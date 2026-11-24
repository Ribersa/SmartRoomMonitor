package com.smartroommonitor.ui.control

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.smartroommonitor.data.rtdb.ActuatorDataSource
import com.smartroommonitor.data.rtdb.ActuatorMode
import com.smartroommonitor.data.rtdb.ActuatorState
import com.smartroommonitor.data.rtdb.RoomCurrentDataSource
import com.smartroommonitor.data.rtdb.RoomThresholds
import com.smartroommonitor.data.rtdb.ThresholdDataSource
import com.smartroommonitor.domain.model.Role
import com.smartroommonitor.domain.usecase.Freshness
import com.smartroommonitor.domain.usecase.freshnessOf
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch

data class ControlUiState(
    val role: Role = Role.Unknown,
    val actuator: ActuatorState = ActuatorState(),
    val thresholds: RoomThresholds = RoomThresholds(),
    val fanOn: Boolean = false,
    val lampOn: Boolean = false,
    val freshness: Freshness = Freshness.NoData,
    val timestampMs: Long = 0L,
    val sending: Boolean = false,
    val message: String? = null,
)

class ControlViewModel(
    private val actuatorSource: ActuatorDataSource,
    private val currentSource: RoomCurrentDataSource,
    private val thresholdSource: ThresholdDataSource,
    role: Role,
) : ViewModel() {

    private val _state = MutableStateFlow(ControlUiState(role = role))
    val state: StateFlow<ControlUiState> = _state.asStateFlow()

    init {
        viewModelScope.launch {
            actuatorSource.observe().collect { actuator ->
                _state.update { it.copy(actuator = actuator) }
            }
        }
        viewModelScope.launch {
            currentSource.observe().collect { current ->
                _state.update {
                    it.copy(
                        fanOn = current.fanStatus,
                        lampOn = current.lampStatus,
                        freshness = freshnessOf(current.timestampMs, System.currentTimeMillis()),
                        timestampMs = current.timestampMs,
                    )
                }
            }
        }
        viewModelScope.launch {
            thresholdSource.observe().collect { thresholds ->
                _state.update { it.copy(thresholds = thresholds) }
            }
        }
        viewModelScope.launch {
            while (true) {
                delay(1_000)
                _state.update {
                    it.copy(freshness = freshnessOf(it.timestampMs, System.currentTimeMillis()))
                }
            }
        }
    }

    fun setFanMode(mode: ActuatorMode) = send("Mode kipas") { actuatorSource.setFanMode(mode) }

    fun setLampMode(mode: ActuatorMode) = send("Mode lampu") { actuatorSource.setLampMode(mode) }

    fun consumeMessage() = _state.update { it.copy(message = null) }

    private fun send(label: String, block: suspend () -> Unit) {
        if (_state.value.sending) return
        if (!_state.value.role.canControl) {
            _state.update { it.copy(message = "Hanya Staff yang dapat mengubah mode.") }
            return
        }
        _state.update { it.copy(sending = true) }
        viewModelScope.launch {
            try {
                block()
                _state.update { it.copy(sending = false, message = "$label diperbarui.") }
            } catch (e: Exception) {
                _state.update {
                    it.copy(sending = false, message = "$label gagal dikirim. Periksa koneksi.")
                }
            }
        }
    }
}
