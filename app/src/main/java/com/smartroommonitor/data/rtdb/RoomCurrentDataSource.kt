package com.smartroommonitor.data.rtdb

import android.content.Context
import com.google.firebase.FirebaseApp
import com.google.firebase.database.DataSnapshot
import com.google.firebase.database.DatabaseError
import com.google.firebase.database.FirebaseDatabase
import com.google.firebase.database.ValueEventListener
import com.smartroommonitor.domain.model.RoomCurrent
import kotlinx.coroutines.channels.awaitClose
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.callbackFlow
import kotlinx.coroutines.flow.emptyFlow

class RoomCurrentDataSource(private val context: Context) {

    fun observe(): Flow<RoomCurrent> {
        if (FirebaseApp.getApps(context).isEmpty()) return emptyFlow()
        return callbackFlow {
            val ref = FirebaseDatabase.getInstance().getReference("room1/current")
            val listener = object : ValueEventListener {
                override fun onDataChange(snapshot: DataSnapshot) {
                    snapshot.toRoomCurrent()?.let { trySend(it) }
                }

                override fun onCancelled(error: DatabaseError) {
                    close(error.toException())
                }
            }
            ref.addValueEventListener(listener)
            awaitClose { ref.removeEventListener(listener) }
        }
    }
}

internal fun DataSnapshot.toRoomCurrent(): RoomCurrent? {
    if (!hasChildren()) return null
    return RoomCurrent(
        temperature = child("temperature").asDouble(),
        humidity = child("humidity").asDouble(),
        humidityLabel = child("humidityLabel").asString("Nyaman"),
        airDelta = child("airDelta").asInt(),
        airLabel = child("airLabel").asString("Memanaskan"),
        airAqi = child("airAqi").asInt(),
        airTvoc = child("airTvoc").asInt(),
        airEco2 = child("airEco2").asInt(),
        airWarmingUp = child("airWarmingUp").asBoolean(),
        lightLux = child("lightLux").asDouble(),
        lightLabel = child("lightLabel").asString("Terang"),
        presence = child("presence").asString("tidak ada"),
        presenceDetected = child("presenceDetected").asBoolean(),
        fanStatus = child("fanStatus").asBoolean(),
        lampStatus = child("lampStatus").asBoolean(),
        nightModeActive = child("nightModeActive").asBoolean(),
        timestampMs = child("timestamp").asLong(),
    )
}

private fun DataSnapshot.asString(fallback: String): String =
    getValue(String::class.java) ?: fallback

private fun DataSnapshot.asDouble(): Double =
    getValue(Double::class.java) ?: getValue(Long::class.java)?.toDouble() ?: 0.0

private fun DataSnapshot.asInt(): Int =
    getValue(Int::class.java) ?: getValue(Long::class.java)?.toInt() ?: 0

private fun DataSnapshot.asLong(): Long =
    getValue(Long::class.java) ?: getValue(Int::class.java)?.toLong() ?: 0L

private fun DataSnapshot.asBoolean(): Boolean =
    getValue(Boolean::class.java) ?: false
