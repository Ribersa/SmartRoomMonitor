package com.smartroommonitor.data.rtdb

import android.content.Context
import com.google.firebase.FirebaseApp
import com.google.firebase.database.DataSnapshot
import com.google.firebase.database.DatabaseError
import com.google.firebase.database.FirebaseDatabase
import com.google.firebase.database.ValueEventListener
import kotlinx.coroutines.channels.awaitClose
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.callbackFlow
import kotlinx.coroutines.flow.emptyFlow

data class RoomThresholds(
    val tempFanOn: Double = 31.0,
    val tempFanHysteresis: Double = 1.0,
    val tempAlert: Double = 35.0,
    val airDeltaModerate: Int = 80,
    val airDeltaBad: Int = 160,
    val lightDarkLux: Double = 40.0,
    val lightExternalHysteresisLux: Double = 200.0,
    val lampEmptyTimeoutSec: Int = 10,
)

class ThresholdDataSource(private val context: Context) {

    fun observe(): Flow<RoomThresholds> {
        if (FirebaseApp.getApps(context).isEmpty()) return emptyFlow()
        return callbackFlow {
            val ref = FirebaseDatabase.getInstance().getReference("room1/thresholds")
            val listener = object : ValueEventListener {
                override fun onDataChange(snapshot: DataSnapshot) {
                    trySend(
                        RoomThresholds(
                            tempFanOn = snapshot.child("tempFanOn").asDouble(31.0),
                            tempFanHysteresis = snapshot.child("tempFanHysteresis").asDouble(1.0),
                            tempAlert = snapshot.child("tempAlert").asDouble(35.0),
                            airDeltaModerate = snapshot.child("airDeltaModerate").asInt(80),
                            airDeltaBad = snapshot.child("airDeltaBad").asInt(160),
                            lightDarkLux = snapshot.child("lightDarkLux").asDouble(40.0),
                            lightExternalHysteresisLux = snapshot.child("lightExternalHysteresisLux").asDouble(200.0),
                            lampEmptyTimeoutSec = snapshot.child("lampEmptyTimeoutSec").asInt(10),
                        )
                    )
                }

                override fun onCancelled(error: DatabaseError) { close(error.toException()) }
            }
            ref.addValueEventListener(listener)
            awaitClose { ref.removeEventListener(listener) }
        }
    }
}

private fun DataSnapshot.asDouble(fallback: Double): Double =
    getValue(Double::class.java) ?: getValue(Long::class.java)?.toDouble() ?: fallback

private fun DataSnapshot.asInt(fallback: Int): Int =
    getValue(Int::class.java) ?: getValue(Long::class.java)?.toInt() ?: fallback
