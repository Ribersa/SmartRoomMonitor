package com.smartroommonitor.ui.control

import androidx.compose.foundation.background
import androidx.compose.foundation.border
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
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.outlined.CheckCircle
import androidx.compose.material.icons.outlined.Lightbulb
import androidx.compose.material.icons.outlined.Lock
import androidx.compose.material.icons.outlined.WindPower
import androidx.compose.material.icons.outlined.WifiOff
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Snackbar
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.smartroommonitor.data.rtdb.ActuatorMode
import com.smartroommonitor.domain.model.Role
import com.smartroommonitor.domain.usecase.Freshness
import com.smartroommonitor.ui.theme.LocalAppColors
import kotlinx.coroutines.delay

private fun d1(value: Double): String = String.format(java.util.Locale("id", "ID"), "%.1f", value)

@Composable
fun ControlScreen(
    state: ControlUiState,
    onFanMode: (ActuatorMode) -> Unit,
    onLampMode: (ActuatorMode) -> Unit,
    onMessageShown: () -> Unit,
    modifier: Modifier = Modifier,
) {
    val app = LocalAppColors.current
    val offline = state.freshness == Freshness.Stale || state.freshness == Freshness.NoData

    LaunchedEffect(state.message) {
        if (state.message != null) {
            delay(3_000)
            onMessageShown()
        }
    }

    Box(modifier.fillMaxSize()) {
        Column(
            Modifier
                .fillMaxSize()
                .verticalScroll(rememberScrollState())
                .padding(horizontal = 20.dp),
        ) {
            Spacer(Modifier.height(8.dp))
            Text(
                "Kontrol",
                style = MaterialTheme.typography.headlineMedium,
                color = MaterialTheme.colorScheme.onBackground,
            )
            Spacer(Modifier.height(4.dp))
            Text(
                if (state.role.canControl) "Khusus Staff · Ruang 1" else "Ruang 1",
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
            )
            Spacer(Modifier.height(16.dp))

            if (!state.role.canControl) {
                InfoBanner(
                    icon = Icons.Outlined.Lock,
                    title = "Akses ditolak",
                    body = "Anda masuk sebagai Pengunjung. Hanya Staff yang dapat mengubah mode kipas, lampu, dan membungkam alarm.",
                    tone = Tone.Neutral,
                )
                Spacer(Modifier.height(12.dp))
                Button(
                    onClick = { },
                    modifier = Modifier
                        .fillMaxWidth()
                        .heightIn(min = 48.dp),
                    shape = RoundedCornerShape(14.dp),
                    colors = ButtonDefaults.buttonColors(
                        containerColor = MaterialTheme.colorScheme.surface,
                        contentColor = MaterialTheme.colorScheme.onSurface,
                    ),
                ) { Text("Kembali ke Dashboard") }
                Spacer(Modifier.height(24.dp))
                return@Column
            }

            if (offline) {
                InfoBanner(
                    icon = Icons.Outlined.WifiOff,
                    title = "Perintah tertunda",
                    body = "Perangkat sedang offline. Mode yang dipilih akan diterapkan perangkat setelah tersambung kembali.",
                    tone = Tone.Danger,
                )
                Spacer(Modifier.height(12.dp))
            }

            ModeCard(
                name = "Kipas",
                icon = Icons.Outlined.WindPower,
                statusOn = state.fanOn,
                mode = state.actuator.fanMode,
                enabled = !offline && !state.sending,
                note = "Otomatis: menyala di atas ${d1(state.thresholds.tempFanOn)} °C, mati pada " +
                    "${d1(state.thresholds.tempFanOn - state.thresholds.tempFanHysteresis)} °C atau kurang.",
                onSelect = onFanMode,
            )
            Spacer(Modifier.height(10.dp))
            ModeCard(
                name = "Lampu",
                icon = Icons.Outlined.Lightbulb,
                statusOn = state.lampOn,
                mode = state.actuator.lampMode,
                enabled = !offline && !state.sending,
                note = "Otomatis: menyala saat gelap (< ${d1(state.thresholds.lightDarkLux)} lux) dan ada orang; " +
                    "padam saat cahaya luar naik ${d1(state.thresholds.lightExternalHysteresisLux)} lux " +
                    "atau ruangan kosong ${state.thresholds.lampEmptyTimeoutSec} detik.",
                onSelect = onLampMode,
            )

            Spacer(Modifier.height(12.dp))
            Text(
                "Mode disimpan di room1/actuator. Status fisik dibaca dari room1/current.",
                style = MaterialTheme.typography.bodyMedium.copy(fontSize = 11.5.sp, lineHeight = 18.sp),
                color = app.faint,
            )
            Spacer(Modifier.height(24.dp))
        }

        state.message?.let { message ->
            Snackbar(
                modifier = Modifier
                    .align(Alignment.BottomCenter)
                    .padding(20.dp),
                containerColor = MaterialTheme.colorScheme.primary,
                contentColor = MaterialTheme.colorScheme.background,
                shape = RoundedCornerShape(14.dp),
            ) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Icon(
                        Icons.Outlined.CheckCircle,
                        contentDescription = null,
                        modifier = Modifier.size(18.dp),
                    )
                    Spacer(Modifier.width(10.dp))
                    Text(message, style = MaterialTheme.typography.labelLarge)
                }
            }
        }
    }
}

private enum class Tone { Neutral, Danger }

@Composable
private fun InfoBanner(icon: ImageVector, title: String, body: String, tone: Tone) {
    val app = LocalAppColors.current
    val bg = if (tone == Tone.Danger) app.dangerBg else MaterialTheme.colorScheme.surface
    val border = if (tone == Tone.Danger) app.dangerBorder else MaterialTheme.colorScheme.outline
    val accent = if (tone == Tone.Danger) app.danger else MaterialTheme.colorScheme.onSurfaceVariant
    Row(
        Modifier
            .fillMaxWidth()
            .background(bg, RoundedCornerShape(16.dp))
            .border(1.dp, border, RoundedCornerShape(16.dp))
            .padding(14.dp),
    ) {
        Icon(icon, contentDescription = null, tint = accent, modifier = Modifier.size(20.dp))
        Spacer(Modifier.width(12.dp))
        Column {
            Text(title, style = MaterialTheme.typography.labelLarge, color = accent)
            Spacer(Modifier.height(3.dp))
            Text(
                body,
                style = MaterialTheme.typography.bodyMedium.copy(fontSize = 12.sp, lineHeight = 18.sp),
                color = MaterialTheme.colorScheme.onSurface,
            )
        }
    }
}

@Composable
private fun ModeCard(
    name: String,
    icon: ImageVector,
    statusOn: Boolean,
    mode: ActuatorMode,
    enabled: Boolean,
    note: String,
    onSelect: (ActuatorMode) -> Unit,
) {
    val app = LocalAppColors.current
    Column(
        Modifier
            .fillMaxWidth()
            .background(MaterialTheme.colorScheme.surface, RoundedCornerShape(20.dp))
            .border(1.dp, app.borderSoft, RoundedCornerShape(20.dp))
            .padding(16.dp),
    ) {
        Row(
            Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically,
        ) {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Icon(
                    icon,
                    contentDescription = null,
                    tint = MaterialTheme.colorScheme.onSurfaceVariant,
                    modifier = Modifier.size(16.dp),
                )
                Spacer(Modifier.width(8.dp))
                Text(
                    name.uppercase(),
                    style = MaterialTheme.typography.labelSmall,
                    color = app.faint,
                )
            }
            Text(
                if (statusOn) "Nyala" else "Mati",
                style = MaterialTheme.typography.labelLarge,
                color = if (statusOn) app.success else MaterialTheme.colorScheme.onSurfaceVariant,
            )
        }

        Spacer(Modifier.height(6.dp))
        Row(verticalAlignment = Alignment.CenterVertically) {
            Box(
                Modifier
                    .size(8.dp)
                    .background(
                        if (statusOn) app.success else app.warning,
                        CircleShape,
                    ),
            )
            Spacer(Modifier.width(8.dp))
            Text(
                "Status fisik: ${if (statusOn) "Nyala" else "Mati"}",
                style = MaterialTheme.typography.bodyMedium.copy(fontWeight = FontWeight.SemiBold),
                color = MaterialTheme.colorScheme.onSurfaceVariant,
            )
        }

        Spacer(Modifier.height(14.dp))
        Row(
            Modifier
                .fillMaxWidth()
                .background(MaterialTheme.colorScheme.surfaceVariant, RoundedCornerShape(14.dp))
                .border(1.dp, MaterialTheme.colorScheme.outline, RoundedCornerShape(14.dp))
                .padding(4.dp),
        ) {
            ActuatorMode.entries.forEach { option ->
                val active = option == mode
                Box(
                    Modifier
                        .weight(1f)
                        .heightIn(min = 44.dp)
                        .background(
                            if (active) MaterialTheme.colorScheme.surface else MaterialTheme.colorScheme.surfaceVariant,
                            RoundedCornerShape(10.dp),
                        )
                        .border(
                            if (active) 1.dp else 0.dp,
                            if (active) MaterialTheme.colorScheme.outline else MaterialTheme.colorScheme.surfaceVariant,
                            RoundedCornerShape(10.dp),
                        )
                        .clickable(enabled = enabled) { onSelect(option) },
                    contentAlignment = Alignment.Center,
                ) {
                    Text(
                        option.label,
                        style = MaterialTheme.typography.labelLarge.copy(
                            fontSize = 12.sp,
                            fontWeight = if (active) FontWeight.Bold else FontWeight.SemiBold,
                        ),
                        color = if (active) {
                            MaterialTheme.colorScheme.onSurface
                        } else {
                            MaterialTheme.colorScheme.onSurfaceVariant.copy(alpha = if (enabled) 1f else 0.5f)
                        },
                    )
                }
            }
        }

        Spacer(Modifier.height(14.dp))
        Box(
            Modifier
                .fillMaxWidth()
                .height(1.dp)
                .background(app.borderSoft),
        )
        Spacer(Modifier.height(11.dp))
        Text(
            note,
            style = MaterialTheme.typography.bodyMedium.copy(fontSize = 11.5.sp, lineHeight = 18.sp),
            color = MaterialTheme.colorScheme.onSurfaceVariant,
        )
    }
}
