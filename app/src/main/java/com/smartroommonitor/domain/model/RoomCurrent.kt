package com.smartroommonitor.domain.model

/**
 * Cermin `room1/current` — firebase/schema-v0.9.json.
 * Semua field ditulis ESP32 (`Device`); aplikasi hanya membaca.
 */
data class RoomCurrent(
    val temperature: Double,
    val humidity: Double,
    val humidityLabel: String,
    val airDelta: Int,
    val airLabel: String,
    val airAqi: Int,
    val airTvoc: Int,
    val airEco2: Int,
    val airWarmingUp: Boolean,
    val lightLux: Double,
    val lightLabel: String,
    val presence: String,
    val presenceDetected: Boolean,
    val fanStatus: Boolean,
    val lampStatus: Boolean,
    val nightModeActive: Boolean,
    val timestampMs: Long,
)
