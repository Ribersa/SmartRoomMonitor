package com.smartroommonitor.domain.model

/** Cermin `room1/alarm`. `active/buzzer/status/reason` ditulis Device; `mute` ditulis Staff. */
data class RoomAlarm(
    val active: Boolean = false,
    val buzzer: Boolean = false,
    val status: String = "normal",
    val reason: String = "none",
    val mute: Boolean = false,
) {
    val isMuted: Boolean get() = status == "dibungkam" || mute

    val reasonLabel: String
        get() = when (reason) {
            "temp_high" -> "Suhu tinggi"
            "air_bad" -> "Kualitas udara buruk"
            "night_presence" -> "Ada orang di jam malam"
            else -> "Peringatan"
        }
}
