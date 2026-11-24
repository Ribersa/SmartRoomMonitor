package com.smartroommonitor.ui.events

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.smartroommonitor.data.rtdb.EventDataSource
import com.smartroommonitor.domain.model.RoomEvent
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch

enum class EventFilter(val label: String, val type: String?) {
    All("Semua", null),
    Temperature("Suhu tinggi", "temp_high"),
    Air("Udara buruk", "air_bad"),
    Night("Jam malam", "night_presence"),
}

data class EventUiState(
    val all: List<RoomEvent> = emptyList(),
    val filter: EventFilter = EventFilter.All,
    val loading: Boolean = true,
    val error: String? = null,
) {
    val visible: List<RoomEvent>
        get() = filter.type?.let { type -> all.filter { it.type == type } } ?: all
}

class EventViewModel(private val source: EventDataSource) : ViewModel() {

    private val _state = MutableStateFlow(EventUiState())
    val state: StateFlow<EventUiState> = _state.asStateFlow()

    init {
        refresh()
    }

    fun refresh() {
        viewModelScope.launch {
            _state.update { it.copy(loading = true, error = null) }
            try {
                val events = source.loadLast24h()
                _state.update { it.copy(all = events, loading = false) }
            } catch (e: Exception) {
                _state.update {
                    it.copy(loading = false, error = "Tidak bisa memuat daftar kejadian.")
                }
            }
        }
    }

    fun selectFilter(filter: EventFilter) = _state.update { it.copy(filter = filter) }
}
