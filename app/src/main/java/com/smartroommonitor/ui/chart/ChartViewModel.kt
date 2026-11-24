package com.smartroommonitor.ui.chart

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.smartroommonitor.data.local.HistoryDao
import com.smartroommonitor.data.local.PreferenceStore
import com.smartroommonitor.data.local.toEntity
import com.smartroommonitor.data.local.toModel
import com.smartroommonitor.data.rtdb.HistoryDataSource
import com.smartroommonitor.domain.model.ChartMetric
import com.smartroommonitor.domain.model.HistoryPoint
import com.smartroommonitor.domain.usecase.MetricStats
import com.smartroommonitor.domain.usecase.statsFor
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch

data class ChartUiState(
    val points: List<HistoryPoint> = emptyList(),
    val metric: ChartMetric = ChartMetric.Temperature,
    val expanded: Boolean = false,
    val loading: Boolean = true,
    val fromCache: Boolean = false,
    val error: String? = null,
) {
    val stats: MetricStats? get() = points.statsFor(metric)
}

class ChartViewModel(
    private val history: HistoryDataSource,
    private val dao: HistoryDao,
    private val prefs: PreferenceStore,
) : ViewModel() {

    private val _state = MutableStateFlow(
        ChartUiState(metric = prefs.chartMetric, expanded = prefs.chartExpanded)
    )
    val state: StateFlow<ChartUiState> = _state.asStateFlow()

    init {
        viewModelScope.launch {
            dao.observe().collect { cached ->
                if (cached.isNotEmpty() && _state.value.points.isEmpty()) {
                    _state.update {
                        it.copy(points = cached.map { e -> e.toModel() }, loading = false, fromCache = true)
                    }
                }
            }
        }
        viewModelScope.launch { refresh() }
    }

    fun refresh() {
        viewModelScope.launch {
            _state.update { it.copy(error = null) }
            try {
                val fresh = history.load()
                if (fresh.isNotEmpty()) {
                    dao.upsertAll(fresh.map { it.toEntity() })
                    dao.pruneBefore(System.currentTimeMillis() - 86_400_000L)
                    _state.update { it.copy(points = fresh, loading = false, fromCache = false) }
                } else {
                    _state.update { it.copy(loading = false) }
                }
            } catch (e: Exception) {
                _state.update {
                    it.copy(
                        loading = false,
                        error = if (it.points.isEmpty()) "Tidak bisa memuat riwayat." else null,
                    )
                }
            }
        }
    }

    fun selectMetric(metric: ChartMetric) {
        prefs.chartMetric = metric
        _state.update { it.copy(metric = metric) }
    }

    fun toggleExpanded() {
        val expanded = !_state.value.expanded
        prefs.chartExpanded = expanded
        _state.update { it.copy(expanded = expanded) }
    }
}
