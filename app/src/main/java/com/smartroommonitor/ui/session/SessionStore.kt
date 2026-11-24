package com.smartroommonitor.ui.session

import android.content.Context
import com.smartroommonitor.domain.model.Role

/**
 * Cache peran agar dashboard bisa dirender tanpa menunggu jaringan.
 * Token Firebase tetap diurus FirebaseAuth; di sini hanya uid + role.
 */
class SessionStore(context: Context) {

    private val prefs = context.applicationContext
        .getSharedPreferences("session", Context.MODE_PRIVATE)

    var uid: String?
        get() = prefs.getString(KEY_UID, null)
        private set(value) = prefs.edit().putString(KEY_UID, value).apply()

    var role: Role
        get() = Role.from(prefs.getString(KEY_ROLE, null))
        private set(value) = prefs.edit().putString(KEY_ROLE, value.name).apply()

    fun save(uid: String, role: Role) {
        prefs.edit().putString(KEY_UID, uid).putString(KEY_ROLE, role.name).apply()
    }

    fun clear() {
        prefs.edit().clear().apply()
    }

    private companion object {
        const val KEY_UID = "uid"
        const val KEY_ROLE = "role"
    }
}
