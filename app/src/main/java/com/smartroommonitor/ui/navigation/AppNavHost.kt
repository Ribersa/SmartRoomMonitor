package com.smartroommonitor.ui.navigation

import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import androidx.lifecycle.viewmodel.compose.viewModel
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.lifecycle.viewmodel.initializer
import androidx.lifecycle.viewmodel.viewModelFactory
import com.smartroommonitor.data.auth.AuthRepository
import com.smartroommonitor.data.user.UserRepository
import com.smartroommonitor.di.AppContainer
import com.smartroommonitor.domain.model.Role
import com.smartroommonitor.ui.chart.ChartViewModel
import com.smartroommonitor.ui.control.ControlViewModel
import com.smartroommonitor.ui.dashboard.DashboardViewModel
import com.smartroommonitor.ui.events.EventViewModel
import com.smartroommonitor.ui.login.LoginScreen
import com.smartroommonitor.ui.login.LoginViewModel
import com.smartroommonitor.ui.login.SessionLoadingScreen
import com.smartroommonitor.ui.profile.ThemeMode
import com.smartroommonitor.ui.session.SessionStore
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch

sealed interface AppState {
    data object Loading : AppState
    data object SignedOut : AppState
    data class SignedIn(val uid: String, val role: Role) : AppState
}

class AppViewModel(
    private val auth: AuthRepository,
    private val users: UserRepository,
    private val session: SessionStore,
) : ViewModel() {

    private val _state = MutableStateFlow<AppState>(AppState.Loading)
    val state: StateFlow<AppState> = _state.asStateFlow()

    init {
        viewModelScope.launch { restore() }
    }

    private suspend fun restore() {
        val uid = auth.currentUid
        if (uid == null) {
            session.clear()
            _state.value = AppState.SignedOut
            return
        }
        val cached = session.role
        _state.value = AppState.SignedIn(uid, cached)
        val fresh = users.fetchRole(uid)
        if (fresh != Role.Unknown && fresh != cached) {
            session.save(uid, fresh)
            _state.value = AppState.SignedIn(uid, fresh)
        }
    }

    suspend fun onSignedIn(uid: String) {
        val role = users.fetchRole(uid)
        session.save(uid, role)
        _state.value = AppState.SignedIn(uid, role)
    }

    fun signOut() {
        auth.signOut()
        session.clear()
        _state.value = AppState.SignedOut
    }
}

@Composable
fun AppNavHost(
    container: AppContainer,
    themeMode: ThemeMode,
    onThemeChange: (ThemeMode) -> Unit,
) {
    val appVm: AppViewModel = viewModel(
        factory = viewModelFactory {
            initializer { AppViewModel(container.authRepository, container.userRepository, container.sessionStore) }
        }
    )
    val appState by appVm.state.collectAsStateWithLifecycle()
    val loginVm: LoginViewModel = viewModel(
        factory = viewModelFactory { initializer { LoginViewModel(container.authRepository) } }
    )
    val loginState by loginVm.state.collectAsStateWithLifecycle()

    LaunchedEffect(loginState.signedInUid) {
        loginState.signedInUid?.let { appVm.onSignedIn(it) }
    }

    when (val state = appState) {
        AppState.Loading -> SessionLoadingScreen()

        AppState.SignedOut -> LoginScreen(
            state = loginState,
            onEmailChange = loginVm::onEmailChange,
            onPasswordChange = loginVm::onPasswordChange,
            onTogglePassword = loginVm::togglePasswordVisibility,
            onSubmit = loginVm::submit,
        )

        is AppState.SignedIn -> {
            val dashVm: DashboardViewModel = viewModel(
                key = "dash-${state.uid}",
                factory = viewModelFactory {
                    initializer {
                        DashboardViewModel(
                            source = container.roomCurrentDataSource,
                            alarmSource = container.roomAlarmDataSource,
                            dao = container.database.currentSnapshotDao(),
                            role = state.role,
                        )
                    }
                }
            )
            val chartVm: ChartViewModel = viewModel(
                key = "chart-${state.uid}",
                factory = viewModelFactory {
                    initializer {
                        ChartViewModel(
                            history = container.historyDataSource,
                            dao = container.database.historyDao(),
                            prefs = container.preferenceStore,
                        )
                    }
                }
            )
            val eventVm: EventViewModel = viewModel(
                key = "events-${state.uid}",
                factory = viewModelFactory {
                    initializer { EventViewModel(container.eventDataSource) }
                }
            )
            val controlVm: ControlViewModel = viewModel(
                key = "control-${state.uid}",
                factory = viewModelFactory {
                    initializer {
                        ControlViewModel(
                            actuatorSource = container.actuatorDataSource,
                            currentSource = container.roomCurrentDataSource,
                            thresholdSource = container.thresholdDataSource,
                            role = state.role,
                        )
                    }
                }
            )

            AppShell(
                role = state.role,
                email = container.authRepository.currentEmail ?: "",
                themeMode = themeMode,
                dashboard = dashVm,
                chart = chartVm,
                events = eventVm,
                control = controlVm,
                onThemeChange = onThemeChange,
                onSignOut = appVm::signOut,
            )
        }
    }
}
