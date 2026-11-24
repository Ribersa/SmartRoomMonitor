package com.smartroommonitor.domain.usecase

import com.smartroommonitor.domain.model.ChartMetric
import com.smartroommonitor.domain.model.HistoryPoint

data class MetricStats(val min: Double, val avg: Double, val max: Double)

fun List<HistoryPoint>.statsFor(metric: ChartMetric): MetricStats? {
    if (isEmpty()) return null
    val values = map {
        when (metric) {
            ChartMetric.Temperature -> it.temperature
            ChartMetric.Humidity -> it.humidity
            ChartMetric.AirDelta -> it.airDelta.toDouble()
        }
    }
    return MetricStats(
        min = values.min(),
        avg = values.average(),
        max = values.max(),
    )
}

fun HistoryPoint.valueOf(metric: ChartMetric): Double = when (metric) {
    ChartMetric.Temperature -> temperature
    ChartMetric.Humidity -> humidity
    ChartMetric.AirDelta -> airDelta.toDouble()
}
