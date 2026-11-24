package com.smartroommonitor.ui.navigation

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.layout.navigationBarsPadding
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.statusBarsPadding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.outlined.FormatListBulleted
import androidx.compose.material.icons.outlined.Home
import androidx.compose.material.icons.outlined.PersonOutline
import androidx.compose.material.icons.outlined.ShowChart
import androidx.compose.material.icons.outlined.Tune
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import com.smartroommonitor.domain.model.Role
import com.smartroommonitor.ui.chart.ChartScreen
import com.smartroommonitor.ui.chart.ChartViewModel
import com.smartroommonitor.ui.control.ControlScreen
import com.smartroommonitor.ui.control.ControlViewModel
import com.smartroommonitor.ui.dashboard.DashboardScreen
import com.smartroommonitor.ui.dashboard.DashboardViewModel
import com.smartroommonitor.ui.events.EventListScreen
import com.smartroommonitor.ui.events.EventViewModel
import com.smartroommonitor.ui.profile.ProfileScreen
import com.smartroommonitor.ui.profile.ThemeMode

private enum class Tab(val label: String, val icon: ImageVector) {
    Dashboard("Dashboard", Icons.Outlined.Home),
    Grafik("Grafik", Icons.Outlined.ShowChart),
    Kejadian("Kejadian", Icons.Outlined.FormatListBulleted),
    Kontrol("Kontrol", Icons.Outlined.Tune),
    Profil("Profil", Icons.Outlined.PersonOutline),
}

@Composable
fun AppShell(
    role: Role,
    email: String,
    themeMode: ThemeMode,
    dashboard: DashboardViewModel,
    chart: ChartViewModel,
    events: EventViewModel,
    control: ControlViewModel,
    onThemeChange: (ThemeMode) -> Unit,
    onSignOut: () -> Unit,
) {
    var tab by remember { mutableStateOf(Tab.Dashboard) }
    var logoutDialog by remember { mutableStateOf(false) }

    val dashState by dashboard.state.collectAsStateWithLifecycle()
    val chartState by chart.state.collectAsStateWithLifecycle()
    val eventState by events.state.collectAsStateWithLifecycle()
    val controlState by control.state.collectAsStateWithLifecycle()

    Column(
        Modifier
            .fillMaxSize()
            .background(MaterialTheme.colorScheme.background)
            .statusBarsPadding(),
    ) {
        Box(Modifier.weight(1f)) {
            when (tab) {
                Tab.Dashboard -> DashboardScreen(state = dashState, onMute = dashboard::mute)
                Tab.Grafik -> ChartScreen(
                    state = chartState,
                    onSelectMetric = chart::selectMetric,
                    onToggleExpanded = chart::toggleExpanded,
                )
                Tab.Kejadian -> EventListScreen(
                    state = eventState,
                    onFilter = events::selectFilter,
                )
                Tab.Kontrol -> ControlScreen(
                    state = controlState,
                    onFanMode = control::setFanMode,
                    onLampMode = control::setLampMode,
                    onMessageShown = control::consumeMessage,
                )
                Tab.Profil -> ProfileScreen(
                    role = role,
                    email = email,
                    themeMode = themeMode,
                    showLogoutDialog = logoutDialog,
                    onThemeChange = onThemeChange,
                    onLogoutRequest = { logoutDialog = true },
                    onLogoutDismiss = { logoutDialog = false },
                    onLogoutConfirm = {
                        logoutDialog = false
                        onSignOut()
                    },
                )
            }
        }
        BottomNav(current = tab, onSelect = { tab = it })
    }
}

@Composable
private fun BottomNav(current: Tab, onSelect: (Tab) -> Unit) {
    Column(Modifier.navigationBarsPadding()) {
        HorizontalDivider(color = MaterialTheme.colorScheme.outline)
        Row(
            Modifier
                .fillMaxWidth()
                .background(MaterialTheme.colorScheme.surface)
                .padding(vertical = 8.dp, horizontal = 6.dp),
            horizontalArrangement = Arrangement.SpaceEvenly,
        ) {
            Tab.entries.forEach { item ->
                val active = item == current
                Column(
                    Modifier
                        .weight(1f)
                        .heightIn(min = 48.dp)
                        .clickable { onSelect(item) }
                        .padding(vertical = 4.dp),
                    horizontalAlignment = Alignment.CenterHorizontally,
                    verticalArrangement = Arrangement.Center,
                ) {
                    Icon(
                        item.icon,
                        contentDescription = null,
                        tint = if (active) {
                            MaterialTheme.colorScheme.onSurface
                        } else {
                            MaterialTheme.colorScheme.onSurfaceVariant
                        },
                        modifier = Modifier.size(21.dp),
                    )
                    Spacer(Modifier.height(4.dp))
                    Text(
                        item.label,
                        style = MaterialTheme.typography.labelSmall.copy(
                            fontSize = 10.sp,
                            letterSpacing = 0.sp,
                            fontWeight = if (active) FontWeight.Bold else FontWeight.Medium,
                        ),
                        color = if (active) {
                            MaterialTheme.colorScheme.onSurface
                        } else {
                            MaterialTheme.colorScheme.onSurfaceVariant
                        },
                    )
                }
            }
        }
    }
}
