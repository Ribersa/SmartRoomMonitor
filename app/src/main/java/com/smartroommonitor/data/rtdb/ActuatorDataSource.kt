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
import kotlinx.coroutines.tasks.await

enum class ActuatorMode(val wire: String, val label: String) {
    Auto("auto", "Otomatis"),
    On("on", "Selalu ON"),
    Off("off", "Selalu OFF");

    companion object {
        fun from(raw: String?): ActuatorMode = entries.firstOrNull { it.wire == raw } ?: Auto
    }
}

data class ActuatorState(
    val fanMode: ActuatorMode = ActuatorMode.Auto,
    val lampMode: ActuatorMode = ActuatorMode.Auto,
)

class ActuatorDataSource(private val context: Context) {

    private fun reference() = FirebaseDatabase.getInstance().getReference("room1/actuator")

    fun observe(): Flow<ActuatorState> {
        if (FirebaseApp.getApps(context).isEmpty()) return emptyFlow()
        return callbackFlow {
            val ref = reference()
            val listener = object : ValueEventListener {
                override fun onDataChange(snapshot: DataSnapshot) {
                    trySend(
                        ActuatorState(
                            fanMode = ActuatorMode.from(snapshot.child("fanMode").getValue(String::class.java)),
                            lampMode = ActuatorMode.from(snapshot.child("lampMode").getValue(String::class.java)),
                        )
                    )
                }

                override fun onCancelled(error: DatabaseError) { close(error.toException()) }
            }
            ref.addValueEventListener(listener)
            awaitClose { ref.removeEventListener(listener) }
        }
    }

    suspend fun setFanMode(mode: ActuatorMode) = write("fanMode", mode)
    suspend fun setLampMode(mode: ActuatorMode) = write("lampMode", mode)

    private suspend fun write(key: String, mode: ActuatorMode) {
        if (FirebaseApp.getApps(context).isEmpty()) return
        reference().child(key).setValue(mode.wire).await()
    }
}
