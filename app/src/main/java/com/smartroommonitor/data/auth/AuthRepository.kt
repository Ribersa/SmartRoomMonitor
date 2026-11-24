package com.smartroommonitor.data.auth

import android.content.Context
import com.google.firebase.FirebaseApp
import com.google.firebase.FirebaseNetworkException
import com.google.firebase.auth.FirebaseAuth
import com.google.firebase.auth.FirebaseAuthInvalidCredentialsException
import com.google.firebase.auth.FirebaseAuthInvalidUserException
import kotlinx.coroutines.tasks.await

sealed interface AuthResult {
    data class Success(val uid: String) : AuthResult
    /** Kredensial salah, akun tidak ada, atau `google-services.json` belum tersedia. */
    data object InvalidCredentials : AuthResult
    data object NoConnection : AuthResult
}

class AuthRepository(private val context: Context) {

    private val auth: FirebaseAuth?
        get() = if (FirebaseApp.getApps(context).isEmpty()) null else FirebaseAuth.getInstance()

    val currentUid: String?
        get() = auth?.currentUser?.uid

    val currentEmail: String?
        get() = auth?.currentUser?.email

    suspend fun signIn(email: String, password: String): AuthResult {
        val firebaseAuth = auth ?: return AuthResult.InvalidCredentials
        return try {
            val result = firebaseAuth.signInWithEmailAndPassword(email.trim(), password).await()
            val uid = result.user?.uid ?: return AuthResult.InvalidCredentials
            AuthResult.Success(uid)
        } catch (e: FirebaseAuthInvalidCredentialsException) {
            AuthResult.InvalidCredentials
        } catch (e: FirebaseAuthInvalidUserException) {
            AuthResult.InvalidCredentials
        } catch (e: FirebaseNetworkException) {
            AuthResult.NoConnection
        } catch (e: Exception) {
            if (e.message?.contains("network", ignoreCase = true) == true) {
                AuthResult.NoConnection
            } else {
                AuthResult.InvalidCredentials
            }
        }
    }

    fun signOut() {
        auth?.signOut()
    }
}
