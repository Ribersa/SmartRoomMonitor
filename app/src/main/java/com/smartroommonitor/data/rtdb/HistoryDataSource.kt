package com.smartroommonitor.data.rtdb

import android.content.Context
import com.google.firebase.FirebaseApp
import com.google.firebase.database.DataSnapshot
import com.google.firebase.database.FirebaseDatabase
import com.smartroommonitor.domain.model.HistoryPoint
import com.smartroommonitor.domain.model.RoomEvent
import kotlinx.coroutines.tasks.await

class HistoryDataSource(private val context: Context) {

    /** `orderByKey().limitToLast(144)` — 144 titik x 10 menit = 24 jam (SRS v1.0 §7.1). */
    suspend fun load(): List<HistoryPoint> {
        if (FirebaseApp.getApps(context).isEmpty()) return emptyList()
        val snapshot = FirebaseDatabase.getInstance()
            .getReference("room1/history")
            .orderByKey()
            .limitToLast(144)
            .get()
            .await()

        return snapshot.children.mapNotNull { child ->
            child.toHistoryPoint()
        }.sortedBy { it.timestampMs }
    }
}

private fun DataSnapshot.toHistoryPoint(): HistoryPoint? {
    val key = key?.toLongOrNull() ?: return null
    if (!hasChildren()) return null
    return HistoryPoint(
        keyEpochSec = key,
        temperature = child("temperature").number(0.0),
        humidity = child("humidity").number(0.0),
        airDelta = child("airDelta").number(0.0).toInt(),
        airAqi = child("airAqi").number(0.0).toInt(),
        lightLux = child("lightLux").number(0.0),
        timestampMs = child("timestamp").number(key * 1000.0).toLong(),
    )
}

private fun DataSnapshot.number(fallback: Double): Double =
    getValue(Double::class.java) ?: getValue(Long::class.java)?.toDouble() ?: fallback

class EventDataSource(private val context: Context) {

    /** Event 24 jam terakhir, terbaru lebih dulu. Index `timestamp` ada di database.rules.json. */
    suspend fun loadLast24h(limit: Int = 200): List<RoomEvent> {
        if (FirebaseApp.getApps(context).isEmpty()) return emptyList()
        val since = System.currentTimeMillis() - 86_400_000L
        val snapshot = FirebaseDatabase.getInstance()
            .getReference("room1/events")
            .orderByChild("timestamp")
            .limitToLast(limit)
            .get()
            .await()

        return snapshot.children.mapNotNull { child ->
            val ts = child.child("timestamp").getValue(Long::class.java) ?: return@mapNotNull null
            RoomEvent(
                id = child.key ?: return@mapNotNull null,
                timestampMs = ts,
                type = child.child("type").getValue(String::class.java) ?: "unknown",
                severity = child.child("severity").getValue(String::class.java) ?: "info",
                message = child.child("message").getValue(String::class.java) ?: "",
            )
        }.filter { it.timestampMs >= since }.sortedByDescending { it.timestampMs }
    }
}
