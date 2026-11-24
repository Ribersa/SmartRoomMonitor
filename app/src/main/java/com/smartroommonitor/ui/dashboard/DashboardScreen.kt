package com.smartroommonitor.ui.dashboard

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.outlined.Air
import androidx.compose.material.icons.outlined.ErrorOutline
import androidx.compose.material.icons.outlined.Lightbulb
import androidx.compose.material.icons.outlined.Person
import androidx.compose.material.icons.outlined.Thermostat
import androidx.compose.material.icons.outlined.WaterDrop
import androidx.compose.material.icons.outlined.WbSunny
import androidx.compose.material.icons.outlined.WifiOff
import androidx.compose.material.icons.outlined.WindPower
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.smartroommonitor.domain.model.RoomCurrent
import com.smartroommonitor.domain.usecase.Freshness
import com.smartroommonitor.ui.theme.LocalAppColors
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

private val ID = Locale("id", "ID")

private fun decimal(value: Double, digits: Int = 1): String =
    String.format(ID, "%.${digits}f", value)

private fun clock(timestampMs: Long): String =
    SimpleDateFormat("HH:mm:ss", ID).format(Date(timestampMs))

@Composable
fun DashboardScreen(
    state: DashboardUiState,
    onMute: () -> Unit,
    modifier: Modifier = Modifier,
) {
    val app = LocalAppColors.current
    val current = state.current
    val showOffline = state.freshness != Freshness.Fresh

    Column(
        modifier
            .fillMaxSize()
            .background(MaterialTheme.colorScheme.background),
    ) {
        Column(
            Modifier
                .weight(1f)
                .verticalScroll(rememberScrollState())
                .padding(horizontal = 20.dp),
        ) {
            Spacer(Modifier.height(8.dp))
            TopBar(subtitle = "Ruang 1 · Selasa, 24 November 2026", avatar = state.role.name.take(2).uppercase())
            StatusLine(freshness = state.freshness, timestampMs = current?.timestampMs ?: 0L)
            if (showOffline) {
                OfflineBanner()
                Spacer(Modifier.height(12.dp))
            }

            AlarmBanner(
                alarm = state.alarm,
                canControl = state.role.canControl,
                muteSending = state.muteSending,
                muteError = state.muteError,
                onMute = onMute,
            )
            if (state.alarm.active || state.alarm.isMuted) Spacer(Modifier.height(12.dp))

            if (state.loading || current == null) {
                LoadingCards()
            } else {
                TemperatureCard(current, state.tempAlert)
                Spacer(Modifier.height(10.dp))
                HumidityCard(current)
                Spacer(Modifier.height(10.dp))
                AirCard(current)
                Spacer(Modifier.height(10.dp))
                LightCard(current)
                Spacer(Modifier.height(10.dp))
                PresenceCard(current)
                Spacer(Modifier.height(20.dp))
                SectionHead("Perangkat", if (state.role.canControl) "Mode diatur di Kontrol" else "Hanya Staff yang dapat mengubah")
                DeviceCard(current)
                if (!state.role.canControl) {
                    Spacer(Modifier.height(12.dp))
                    Text(
                        "Anda masuk sebagai Pengunjung. Kontrol kipas, lampu, dan tombol MUTE tidak tersedia.",
                        style = MaterialTheme.typography.bodyMedium.copy(fontSize = 11.5.sp, lineHeight = 18.sp),
                        color = app.faint,
                    )
                }
            }
            Spacer(Modifier.height(24.dp))
        }
    }
}

@Composable
private fun TopBar(subtitle: String, avatar: String) {
    Row(
        Modifier
            .fillMaxWidth()
            .padding(top = 8.dp, bottom = 4.dp),
        verticalAlignment = Alignment.Top,
        horizontalArrangement = Arrangement.SpaceBetween,
    ) {
        Column(Modifier.weight(1f)) {
            Text(
                "Dashboard",
                style = MaterialTheme.typography.headlineMedium,
                color = MaterialTheme.colorScheme.onBackground,
            )
            Spacer(Modifier.height(4.dp))
            Text(
                subtitle,
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
            )
        }
        Box(
            Modifier
                .size(40.dp)
                .background(MaterialTheme.colorScheme.surfaceVariant, CircleShape)
                .border(1.dp, MaterialTheme.colorScheme.outline, CircleShape),
            contentAlignment = Alignment.Center,
        ) {
            Text(
                avatar,
                style = MaterialTheme.typography.labelLarge,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
            )
        }
    }
}

@Composable
private fun StatusLine(freshness: Freshness, timestampMs: Long) {
    val app = LocalAppColors.current
    val (dotColor, label) = when (freshness) {
        Freshness.Fresh -> app.success to "Tersambung · Diperbarui ${clock(timestampMs)}"
        Freshness.Stale -> app.danger to "Perangkat offline · data terakhir ${clock(timestampMs)}"
        Freshness.NoData -> app.warning to "Menunggu data pertama dari perangkat"
    }
    Row(
        Modifier.padding(vertical = 14.dp),
        verticalAlignment = Alignment.CenterVertically,
    ) {
        Box(
            Modifier
                .size(8.dp)
                .background(dotColor, CircleShape),
        )
        Spacer(Modifier.width(8.dp))
        Text(
            label,
            style = MaterialTheme.typography.bodyMedium.copy(fontWeight = FontWeight.SemiBold, fontSize = 12.sp),
            color = MaterialTheme.colorScheme.onSurfaceVariant,
        )
    }
}

@Composable
private fun OfflineBanner() {
    val app = LocalAppColors.current
    Row(
        Modifier
            .fillMaxWidth()
            .background(app.dangerBg, RoundedCornerShape(16.dp))
            .border(1.dp, app.dangerBorder, RoundedCornerShape(16.dp))
            .padding(14.dp),
    ) {
        Icon(
            Icons.Outlined.WifiOff,
            contentDescription = null,
            tint = app.danger,
            modifier = Modifier
                .size(20.dp)
                .padding(top = 1.dp),
        )
        Spacer(Modifier.width(12.dp))
        Column {
            Text(
                "Perangkat offline",
                style = MaterialTheme.typography.labelLarge,
                color = app.danger,
            )
            Spacer(Modifier.height(3.dp))
            Text(
                "Data terhenti lebih dari 15 detik. Data terakhir tetap ditampilkan.",
                style = MaterialTheme.typography.bodyMedium.copy(fontSize = 12.sp, lineHeight = 18.sp),
                color = MaterialTheme.colorScheme.onBackground,
            )
        }
    }
}

@Composable
private fun SectionHead(title: String, hint: String) {
    Row(
        Modifier
            .fillMaxWidth()
            .padding(top = 4.dp, bottom = 10.dp),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.Bottom,
    ) {
        Text(
            title,
            style = MaterialTheme.typography.titleMedium,
            color = MaterialTheme.colorScheme.onBackground,
        )
        Text(
            hint,
            style = MaterialTheme.typography.bodyMedium.copy(fontSize = 11.sp),
            color = LocalAppColors.current.faint,
        )
    }
}

@Composable
private fun MetricCard(
    icon: ImageVector,
    label: String,
    value: String,
    unit: String,
    badge: String,
    badgeTone: BadgeTone,
    footLeft: String,
    footRight: String,
    aux: String? = null,
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
                    modifier = Modifier.size(15.dp),
                )
                Spacer(Modifier.width(7.dp))
                Text(
                    label.uppercase(),
                    style = MaterialTheme.typography.labelSmall,
                    color = app.faint,
                )
            }
            Badge(badge, badgeTone)
        }
        Spacer(Modifier.height(10.dp))
        Row(verticalAlignment = Alignment.Bottom) {
            Text(
                value,
                style = MaterialTheme.typography.headlineMedium.copy(
                    fontSize = 30.sp, lineHeight = 32.sp, fontWeight = FontWeight.Bold,
                ),
                color = MaterialTheme.colorScheme.onSurface,
            )
            if (unit.isNotEmpty()) {
                Spacer(Modifier.width(6.dp))
                Text(
                    unit,
                    style = MaterialTheme.typography.labelLarge.copy(fontSize = 14.sp),
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                )
            }
            if (aux != null) {
                Spacer(Modifier.weight(1f))
                Text(
                    aux,
                    style = MaterialTheme.typography.bodyMedium.copy(fontSize = 12.sp, fontWeight = FontWeight.SemiBold),
                    color = app.faint,
                )
            }
        }
        Spacer(Modifier.height(12.dp))
        Box(
            Modifier
                .fillMaxWidth()
                .height(1.dp)
                .background(app.borderSoft),
        )
        Spacer(Modifier.height(11.dp))
        Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
            Text(
                footLeft,
                style = MaterialTheme.typography.bodyMedium.copy(fontSize = 11.5.sp),
                color = MaterialTheme.colorScheme.onSurfaceVariant,
            )
            Text(
                footRight,
                style = MaterialTheme.typography.bodyMedium.copy(
                    fontSize = 11.5.sp, fontWeight = FontWeight.Bold,
                ),
                color = MaterialTheme.colorScheme.onSurface,
                textAlign = TextAlign.End,
            )
        }
    }
}

private enum class BadgeTone { Neutral, Ok, Warn, Bad }

@Composable
private fun Badge(text: String, tone: BadgeTone) {
    val app = LocalAppColors.current
    val bg: Color
    val fg: Color
    val border: Color
    when (tone) {
        BadgeTone.Bad -> {
            bg = app.dangerBg; fg = app.danger; border = app.dangerBorder
        }
        BadgeTone.Ok -> {
            bg = MaterialTheme.colorScheme.surfaceVariant; fg = app.success; border = MaterialTheme.colorScheme.outline
        }
        BadgeTone.Warn -> {
            bg = MaterialTheme.colorScheme.surfaceVariant; fg = app.warning; border = MaterialTheme.colorScheme.outline
        }
        BadgeTone.Neutral -> {
            bg = MaterialTheme.colorScheme.surfaceVariant; fg = MaterialTheme.colorScheme.onSurface
            border = MaterialTheme.colorScheme.outline
        }
    }
    Box(
        Modifier
            .background(bg, RoundedCornerShape(999.dp))
            .border(1.dp, border, RoundedCornerShape(999.dp))
            .padding(horizontal = 10.dp, vertical = 5.dp),
    ) {
        Text(
            text,
            style = MaterialTheme.typography.labelLarge.copy(fontSize = 12.sp),
            color = fg,
        )
    }
}

@Composable
private fun TemperatureCard(current: RoomCurrent, tempAlert: Double) {
    val over = current.temperature > tempAlert
    MetricCard(
        icon = Icons.Outlined.Thermostat,
        label = "Suhu",
        value = decimal(current.temperature),
        unit = "°C",
        badge = if (over) "Di atas batas" else "Normal",
        badgeTone = if (over) BadgeTone.Bad else BadgeTone.Ok,
        footLeft = "Batas peringatan",
        footRight = "${decimal(tempAlert)} °C",
    )
}

@Composable
private fun HumidityCard(current: RoomCurrent) {
    MetricCard(
        icon = Icons.Outlined.WaterDrop,
        label = "Kelembapan",
        value = decimal(current.humidity),
        unit = "%RH",
        badge = current.humidityLabel,
        badgeTone = if (current.humidityLabel == "Nyaman") BadgeTone.Ok else BadgeTone.Warn,
        footLeft = "Rentang nyaman",
        footRight = "40–60 %RH",
    )
}

@Composable
private fun AirCard(current: RoomCurrent) {
    val warming = current.airWarmingUp || current.airLabel == "Memanaskan"
    MetricCard(
        icon = Icons.Outlined.Air,
        label = "Kualitas udara",
        value = current.airDelta.toString(),
        unit = "ppb",
        badge = current.airLabel,
        badgeTone = when {
            warming -> BadgeTone.Warn
            current.airLabel == "Buruk" -> BadgeTone.Bad
            current.airLabel == "Sedang" -> BadgeTone.Warn
            else -> BadgeTone.Ok
        },
        aux = if (warming) null else "AQI ${current.airAqi}",
        footLeft = if (warming) "Masa pemanasan 3 menit" else "Delta dari baseline",
        footRight = if (warming) "Tanpa peringatan" else "Buruk ≥ 160 ppb",
    )
}

@Composable
private fun LightCard(current: RoomCurrent) {
    MetricCard(
        icon = Icons.Outlined.WbSunny,
        label = "Cahaya",
        value = decimal(current.lightLux, 0),
        unit = "lux",
        badge = current.lightLabel,
        badgeTone = if (current.lightLabel == "Gelap") BadgeTone.Warn else BadgeTone.Ok,
        footLeft = "Ambang gelap",
        footRight = "40 lux",
    )
}

@Composable
private fun PresenceCard(current: RoomCurrent) {
    MetricCard(
        icon = Icons.Outlined.Person,
        label = "Kehadiran",
        value = when (current.presence) {
            "tidak ada" -> "0"
            else -> "1"
        },
        unit = "orang",
        badge = current.presence,
        badgeTone = if (current.presence == "tidak ada") BadgeTone.Ok else BadgeTone.Neutral,
        footLeft = "Jam malam",
        footRight = if (current.nightModeActive) "Aktif 22:00–06:00" else "22:00–06:00 WIB",
    )
}

@Composable
private fun DeviceCard(current: RoomCurrent) {
    val app = LocalAppColors.current
    Column(
        Modifier
            .fillMaxWidth()
            .background(MaterialTheme.colorScheme.surface, RoundedCornerShape(20.dp))
            .border(1.dp, app.borderSoft, RoundedCornerShape(20.dp))
            .padding(vertical = 6.dp),
    ) {
        DeviceRow("Kipas", Icons.Outlined.WindPower, current.fanStatus)
        Box(
            Modifier
                .fillMaxWidth()
                .padding(start = 52.dp)
                .height(1.dp)
                .background(app.borderSoft),
        )
        DeviceRow("Lampu", Icons.Outlined.Lightbulb, current.lampStatus)
    }
}

@Composable
private fun DeviceRow(name: String, icon: ImageVector, on: Boolean) {
    val app = LocalAppColors.current
    Row(
        Modifier
            .fillMaxWidth()
            .padding(horizontal = 16.dp, vertical = 12.dp),
        verticalAlignment = Alignment.CenterVertically,
    ) {
        Icon(
            icon,
            contentDescription = null,
            tint = MaterialTheme.colorScheme.onSurfaceVariant,
            modifier = Modifier.size(20.dp),
        )
        Spacer(Modifier.width(12.dp))
        Column(Modifier.weight(1f)) {
            Text(
                name,
                style = MaterialTheme.typography.titleMedium.copy(fontSize = 15.sp),
                color = MaterialTheme.colorScheme.onSurface,
            )
            Text(
                if (on) "Sedang menyala" else "Sedang mati",
                style = MaterialTheme.typography.bodyMedium.copy(fontSize = 11.5.sp),
                color = MaterialTheme.colorScheme.onSurfaceVariant,
            )
        }
        Text(
            if (on) "Nyala" else "Mati",
            style = MaterialTheme.typography.labelLarge,
            color = if (on) app.success else MaterialTheme.colorScheme.onSurfaceVariant,
        )
    }
}

@Composable
private fun LoadingCards() {
    val app = LocalAppColors.current
    repeat(4) {
        Box(
            Modifier
                .fillMaxWidth()
                .height(92.dp)
                .background(app.surface2, RoundedCornerShape(20.dp)),
        )
        Spacer(Modifier.height(10.dp))
    }
    Text(
        "Menunggu data pertama dari perangkat…",
        style = MaterialTheme.typography.bodyMedium,
        color = app.faint,
        modifier = Modifier.padding(top = 6.dp),
    )
}

@Composable
private fun ErrorNote(message: String) {
    val app = LocalAppColors.current
    Row(
        Modifier
            .fillMaxWidth()
            .background(app.dangerBg, RoundedCornerShape(14.dp))
            .padding(12.dp),
    ) {
        Icon(
            Icons.Outlined.ErrorOutline,
            contentDescription = null,
            tint = app.danger,
            modifier = Modifier.size(16.dp),
        )
        Spacer(Modifier.width(8.dp))
        Text(message, style = MaterialTheme.typography.bodyMedium, color = MaterialTheme.colorScheme.onSurface)
    }
}
