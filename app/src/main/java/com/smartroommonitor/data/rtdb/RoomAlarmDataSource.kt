package com.smartroommonitor.data.rtdb

import android.content.Context
import com.google.firebase.FirebaseApp
import com.google.firebase.database.DataSnapshot
import com.google.firebase.database.DatabaseError
import com.google.firebase.database.FirebaseDatabase
import com.google.firebase.database.ValueEventListener
import com.smartroommonitor.domain.model.RoomAlarm
import kotlinx.coroutines.channels.awaitClose
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.callbackFlow
import kotlinx.coroutines.flow.emptyFlow
import kotlinx.coroutines.tasks.await

class RoomAlarmDataSource(private val context: Context) {

    private fun reference() = FirebaseDatabase.getInstance().getReference("room1/alarm")

    fun observe(): Flow<RoomAlarm> {
        if (FirebaseApp.getApps(context).isEmpty()) return emptyFlow()
        return callbackFlow {
            val ref = reference()
            val listener = object : ValueEventListener {
                override fun onDataChange(snapshot: DataSnapshot) {
                    trySend(snapshot.toRoomAlarm())
                }

                override fun onCancelled(error: DatabaseError) {
                    close(error.toException())
                }
            }
            ref.addValueEventListener(listener)
            awaitClose { ref.removeEventListener(listener) }
        }
    }

    /** Menulis `room1/alarm/mute`. Rules mengizinkan Staff dan Device saja. */
    suspend fun setMute(muted: Boolean) {
        if (FirebaseApp.getApps(context).isEmpty()) return
        reference().child("mute").setValue(muted).await()
    }
}

internal fun DataSnapshot.toRoomAlarm() = RoomAlarm(
    active = child("active").getValue(Boolean::class.java) ?: false,
    buzzer = child("buzzer").getValue(Boolean::class.java) ?: false,
    status = child("status").getValue(String::class.java) ?: "normal",
    reason = child("reason").getValue(String::class.java) ?: "none",
    mute = child("mute").getValue(Boolean::class.java) ?: false,
)
