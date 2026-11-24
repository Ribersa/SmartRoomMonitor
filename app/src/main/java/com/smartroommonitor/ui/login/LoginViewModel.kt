package com.smartroommonitor.ui.login

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.smartroommonitor.data.auth.AuthRepository
import com.smartroommonitor.data.auth.AuthResult
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch

data class LoginUiState(
    val email: String = "",
    val password: String = "",
    val passwordVisible: Boolean = false,
    val loading: Boolean = false,
    val errorSummary: String? = null,
    val emailError: String? = null,
    val passwordError: String? = null,
    val signedInUid: String? = null,
)

class LoginViewModel(private val auth: AuthRepository) : ViewModel() {

    private val _state = MutableStateFlow(LoginUiState())
    val state: StateFlow<LoginUiState> = _state.asStateFlow()

    fun onEmailChange(value: String) = _state.update {
        it.copy(email = value, emailError = null, errorSummary = null)
    }

    fun onPasswordChange(value: String) = _state.update {
        it.copy(password = value, passwordError = null, errorSummary = null)
    }

    fun togglePasswordVisibility() = _state.update { it.copy(passwordVisible = !it.passwordVisible) }

    fun submit() {
        val current = _state.value
        if (current.loading) return

        val emailError = when {
            current.email.isBlank() -> "Masukkan alamat email"
            !current.email.contains("@") -> "Format email belum benar"
            else -> null
        }
        val passwordError = if (current.password.isBlank()) "Masukkan kata sandi" else null

        if (emailError != null || passwordError != null) {
            _state.update {
                it.copy(
                    emailError = emailError,
                    passwordError = passwordError,
                    errorSummary = "Periksa kolom yang ditandai sebelum melanjutkan.",
                )
            }
            return
        }

        _state.update { it.copy(loading = true, errorSummary = null) }
        viewModelScope.launch {
            when (val result = auth.signIn(current.email, current.password)) {
                is AuthResult.Success -> _state.update {
                    it.copy(loading = false, signedInUid = result.uid)
                }
                AuthResult.InvalidCredentials -> _state.update {
                    it.copy(
                        loading = false,
                        emailError = "Periksa alamat email",
                        passwordError = "Periksa kata sandi",
                        errorSummary = "Email atau kata sandi salah. Periksa kembali lalu coba lagi.",
                    )
                }
                AuthResult.NoConnection -> _state.update {
                    it.copy(
                        loading = false,
                        errorSummary = "Tidak tersambung ke internet. Periksa koneksi lalu coba lagi.",
                    )
                }
            }
        }
    }
}
