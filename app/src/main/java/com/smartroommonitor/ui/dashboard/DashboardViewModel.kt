package com.smartroommonitor.ui.dashboard

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.smartroommonitor.data.local.CurrentSnapshotDao
import com.smartroommonitor.data.local.toEntity
import com.smartroommonitor.data.local.toModel
import com.smartroommonitor.data.rtdb.RoomAlarmDataSource
import com.smartroommonitor.data.rtdb.RoomCurrentDataSource
import com.smartroommonitor.domain.model.Role
import com.smartroommonitor.domain.model.RoomAlarm
import com.smartroommonitor.domain.model.RoomCurrent
import com.smartroommonitor.domain.usecase.Freshness
import com.smartroommonitor.domain.usecase.freshnessOf
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch

/** Ambang default sampai `room1/thresholds` dimuat pada F4.4. */
const val DEFAULT_TEMP_ALERT = 35.0

data class DashboardUiState(
    val loading: Boolean = true,
    val current: RoomCurrent? = null,
    val freshness: Freshness = Freshness.NoData,
    val role: Role = Role.Unknown,
    val tempAlert: Double = DEFAULT_TEMP_ALERT,
    val alarm: RoomAlarm = RoomAlarm(),
    val muteSending: Boolean = false,
    val muteError: String? = null,
    val fromCache: Boolean = false,
)

class DashboardViewModel(
    private val source: RoomCurrentDataSource,
    private val alarmSource: RoomAlarmDataSource,
    private val dao: CurrentSnapshotDao,
    role: Role,
) : ViewModel() {

    private val _state = MutableStateFlow(DashboardUiState(role = role))
    val state: StateFlow<DashboardUiState> = _state.asStateFlow()

    init {
        // Cache Room tampil lebih dulu agar layar tidak kosong saat offline.
        viewModelScope.launch {
            dao.observe().collect { entity ->
                if (entity != null && _state.value.current == null) {
                    _state.update {
                        it.copy(
                            loading = false,
                            current = entity.toModel(),
                            fromCache = true,
                            freshness = freshnessOf(entity.timestampMs, System.currentTimeMillis()),
                        )
                    }
                }
            }
        }

        viewModelScope.launch {
            source.observe().collect { current ->
                _state.update {
                    it.copy(
                        loading = false,
                        current = current,
                        fromCache = false,
                        freshness = freshnessOf(current.timestampMs, System.currentTimeMillis()),
                    )
                }
                dao.upsert(current.toEntity())
            }
        }

        viewModelScope.launch {
            alarmSource.observe().collect { alarm ->
                _state.update { it.copy(alarm = alarm) }
            }
        }

        // Penanda kesegaran diperbarui tiap detik agar banner offline muncul tanpa data baru.
        viewModelScope.launch {
            while (true) {
                delay(1_000)
                _state.update {
                    val ts = it.current?.timestampMs ?: 0L
                    it.copy(freshness = freshnessOf(ts, System.currentTimeMillis()))
                }
            }
        }
    }

    fun mute() {
        if (_state.value.muteSending) return
        _state.update { it.copy(muteSending = true, muteError = null) }
        viewModelScope.launch {
            try {
                alarmSource.setMute(true)
                _state.update { it.copy(muteSending = false) }
            } catch (e: Exception) {
                _state.update {
                    it.copy(muteSending = false, muteError = "Perintah MUTE gagal dikirim. Coba lagi.")
                }
            }
        }
    }
}
