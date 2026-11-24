package com.smartroommonitor.domain.model

data class HistoryPoint(
    val keyEpochSec: Long,
    val temperature: Double,
    val humidity: Double,
    val airDelta: Int,
    val airAqi: Int,
    val lightLux: Double,
    val timestampMs: Long,
)

data class RoomEvent(
    val id: String,
    val timestampMs: Long,
    val type: String,
    val severity: String,
    val message: String,
) {
    val typeLabel: String
        get() = when (type) {
            "temp_high" -> "Suhu tinggi"
            "air_bad" -> "Udara buruk"
            "night_presence" -> "Jam malam"
            else -> type
        }

    val severityLabel: String
        get() = when (severity) {
            "danger" -> "BAHAYA"
            "warning" -> "PERINGATAN"
            else -> severity.uppercase()
        }
}

enum class ChartMetric(val label: String, val unit: String) {
    Temperature("Suhu", "°C"),
    Humidity("Kelembapan", "%RH"),
    AirDelta("Udara", "ppb"),
}
