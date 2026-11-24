package com.smartroommonitor.data.user

import android.content.Context
import com.google.firebase.FirebaseApp
import com.google.firebase.database.FirebaseDatabase
import com.smartroommonitor.domain.model.Role
import kotlinx.coroutines.tasks.await

class UserRepository(private val context: Context) {

    /** Membaca `users/{uid}/role` sekali. `Role.Unknown` bila rules menolak atau data tidak ada. */
    suspend fun fetchRole(uid: String): Role {
        if (FirebaseApp.getApps(context).isEmpty()) return Role.Unknown
        return try {
            val snapshot = FirebaseDatabase.getInstance()
                .getReference("users").child(uid).child("role")
                .get().await()
            Role.from(snapshot.getValue(String::class.java))
        } catch (e: Exception) {
            Role.Unknown
        }
    }
}
