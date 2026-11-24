package com.smartroommonitor.domain.model

enum class Role {
    Staff,
    Pengunjung,
    Unknown;

    val canControl: Boolean get() = this == Staff

    companion object {
        fun from(raw: String?): Role = when (raw) {
            "Staff" -> Staff
            "Pengunjung" -> Pengunjung
            else -> Unknown
        }
    }
}
