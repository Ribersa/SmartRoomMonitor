package com.smartroommonitor.ui.chart

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
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.outlined.Add
import androidx.compose.material.icons.outlined.Remove
import androidx.compose.material.icons.outlined.ShowChart
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.smartroommonitor.domain.model.ChartMetric
import com.smartroommonitor.domain.model.HistoryPoint
import com.smartroommonitor.domain.usecase.valueOf
import com.smartroommonitor.ui.theme.LocalAppColors
import java.util.Locale

private val ID = Locale("id", "ID")

private fun fmt(value: Double, unit: String): String = when (unit) {
    "ppb" -> "${value.toInt()} ppb"
    else -> String.format(ID, "%.1f %s", value, unit)
}

@Composable
fun ChartScreen(
    state: ChartUiState,
    onSelectMetric: (ChartMetric) -> Unit,
    onToggleExpanded: () -> Unit,
    modifier: Modifier = Modifier,
) {
    val app = LocalAppColors.current
    val limit = when (state.metric) {
        ChartMetric.Temperature -> 35.0
        ChartMetric.Humidity -> 60.0
        ChartMetric.AirDelta -> 160.0
    }
    val limitLabel = when (state.metric) {
        ChartMetric.Temperature -> "Batas 35 °C"
        ChartMetric.Humidity -> "Batas nyaman 60 %RH"
        ChartMetric.AirDelta -> "Buruk ≥ 160 ppb"
    }
    val atLimit = state.points.count { it.valueOf(state.metric) >= limit }

    Column(
        modifier
            .fillMaxSize()
            .verticalScroll(rememberScrollState())
            .padding(horizontal = 20.dp),
    ) {
        Spacer(Modifier.height(8.dp))
        Text(
            "Grafik 24 Jam",
            style = MaterialTheme.typography.headlineMedium,
            color = MaterialTheme.colorScheme.onBackground,
        )
        Spacer(Modifier.height(4.dp))
        Text(
            "Ruang 1 · ${state.metric.label}",
            style = MaterialTheme.typography.bodyMedium,
            color = MaterialTheme.colorScheme.onSurfaceVariant,
        )
        Spacer(Modifier.height(14.dp))

        MetricChips(state, onSelectMetric, onToggleExpanded)
        Spacer(Modifier.height(12.dp))

        if (state.loading && state.points.isEmpty()) {
            Box(
                Modifier
                    .fillMaxWidth()
                    .height(220.dp)
                    .background(app.surface2, RoundedCornerShape(20.dp)),
            )
        } else if (state.points.isEmpty()) {
            EmptyChart()
        } else {
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
                    Text(
                        "${state.metric.label.uppercase()} · 24 JAM TERAKHIR",
                        style = MaterialTheme.typography.labelSmall,
                        color = app.faint,
                    )
                    Text(
                        state.metric.unit,
                        style = MaterialTheme.typography.labelLarge,
                        color = MaterialTheme.colorScheme.onSurface,
                    )
                }
                Spacer(Modifier.height(10.dp))
                TrendChart(
                    points = state.points,
                    metric = state.metric,
                    limit = limit,
                    limitLabel = limitLabel,
                )
                Spacer(Modifier.height(14.dp))
                Stats(state)
                Spacer(Modifier.height(14.dp))
                Box(
                    Modifier
                        .fillMaxWidth()
                        .height(1.dp)
                        .background(app.borderSoft),
                )
                Spacer(Modifier.height(11.dp))
                Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
                    Text(
                        "${state.points.size} titik · interval 10 menit",
                        style = MaterialTheme.typography.bodyMedium.copy(fontSize = 11.5.sp),
                        color = MaterialTheme.colorScheme.onSurfaceVariant,
                    )
                    Text(
                        if (state.fromCache) "Cache lokal" else "room1/history",
                        style = MaterialTheme.typography.bodyMedium.copy(
                            fontSize = 11.5.sp, fontWeight = FontWeight.Bold,
                        ),
                        color = MaterialTheme.colorScheme.onSurface,
                    )
                }
            }

            if (state.points.size < 144) {
                Spacer(Modifier.height(10.dp))
                Text(
                    "Baru ${state.points.size} titik tersimpan. Grafik terisi sampai 144 titik (24 jam).",
                    style = MaterialTheme.typography.bodyMedium.copy(fontSize = 11.5.sp, lineHeight = 18.sp),
                    color = app.faint,
                )
            }
            if (atLimit > 0) {
                Spacer(Modifier.height(10.dp))
                Text(
                    "$atLimit titik melewati $limitLabel.",
                    style = MaterialTheme.typography.bodyMedium.copy(fontSize = 11.5.sp),
                    color = app.danger,
                )
            }
            if (state.metric == ChartMetric.AirDelta) {
                Spacer(Modifier.height(8.dp))
                Text(
                    "Nilai udara adalah airDelta terhadap baseline normal, bukan angka absolut.",
                    style = MaterialTheme.typography.bodyMedium.copy(fontSize = 11.5.sp, lineHeight = 18.sp),
                    color = app.faint,
                )
            }
        }

        if (state.error != null) {
            Spacer(Modifier.height(10.dp))
            Text(state.error, style = MaterialTheme.typography.bodyMedium, color = app.danger)
        }
        Spacer(Modifier.height(24.dp))
    }
}

@Composable
private fun MetricChips(
    state: ChartUiState,
    onSelectMetric: (ChartMetric) -> Unit,
    onToggleExpanded: () -> Unit,
) {
    val app = LocalAppColors.current
    Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
        if (!state.expanded) {
            Chip(label = "Suhu", active = true, onClick = { })
            Chip(
                label = "Metrik lain",
                active = false,
                icon = Icons.Outlined.Add,
                dashed = true,
                onClick = onToggleExpanded,
            )
        } else {
            ChartMetric.entries.forEach { metric ->
                Chip(
                    label = metric.label,
                    active = metric == state.metric,
                    onClick = { onSelectMetric(metric) },
                )
            }
            Chip(
                label = "Ringkas",
                active = false,
                icon = Icons.Outlined.Remove,
                dashed = true,
                onClick = onToggleExpanded,
            )
        }
    }
}

@Composable
private fun Chip(
    label: String,
    active: Boolean,
    onClick: () -> Unit,
    icon: androidx.compose.ui.graphics.vector.ImageVector? = null,
    dashed: Boolean = false,
) {
    val app = LocalAppColors.current
    Row(
        Modifier
            .heightIn(min = 36.dp)
            .background(
                if (active) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.surface,
                RoundedCornerShape(999.dp),
            )
            .border(
                width = 1.dp,
                color = if (active) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.outline,
                shape = RoundedCornerShape(999.dp),
            )
            .clickable(onClick = onClick)
            .padding(horizontal = 13.dp, vertical = 8.dp),
        verticalAlignment = Alignment.CenterVertically,
    ) {
        if (icon != null) {
            Icon(
                icon,
                contentDescription = null,
                tint = if (active) MaterialTheme.colorScheme.background else MaterialTheme.colorScheme.onSurface,
                modifier = Modifier.size(13.dp),
            )
            Spacer(Modifier.width(5.dp))
        }
        Text(
            label,
            style = MaterialTheme.typography.labelLarge.copy(fontSize = 12.sp),
            color = if (active) MaterialTheme.colorScheme.background else MaterialTheme.colorScheme.onSurface,
        )
    }
}

@Composable
private fun Stats(state: ChartUiState) {
    val app = LocalAppColors.current
    val stats = state.stats ?: return
    Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
        StatCell("Terendah", fmt(stats.min, state.metric.unit), Modifier.weight(1f))
        StatCell("Rata-rata", fmt(stats.avg, state.metric.unit), Modifier.weight(1f))
        StatCell("Tertinggi", fmt(stats.max, state.metric.unit), Modifier.weight(1f))
    }
}

@Composable
private fun StatCell(label: String, value: String, modifier: Modifier = Modifier) {
    val app = LocalAppColors.current
    Column(
        modifier
            .background(app.surface2, RoundedCornerShape(12.dp))
            .padding(10.dp),
    ) {
        Text(
            label.uppercase(),
            style = MaterialTheme.typography.labelSmall.copy(fontSize = 10.5.sp, letterSpacing = 0.5.sp),
            color = app.faint,
        )
        Spacer(Modifier.height(4.dp))
        Text(
            value,
            style = MaterialTheme.typography.labelLarge.copy(fontSize = 15.sp, fontWeight = FontWeight.Bold),
            color = MaterialTheme.colorScheme.onSurface,
        )
    }
}

@Composable
private fun EmptyChart() {
    val app = LocalAppColors.current
    Column(
        Modifier
            .fillMaxWidth()
            .background(MaterialTheme.colorScheme.surface, RoundedCornerShape(20.dp))
            .border(1.dp, app.borderSoft, RoundedCornerShape(20.dp))
            .padding(vertical = 48.dp, horizontal = 18.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
    ) {
        Box(
            Modifier
                .size(56.dp)
                .background(app.surface2, RoundedCornerShape(18.dp))
                .border(1.dp, MaterialTheme.colorScheme.outline, RoundedCornerShape(18.dp)),
            contentAlignment = Alignment.Center,
        ) {
            Icon(
                Icons.Outlined.ShowChart,
                contentDescription = null,
                tint = MaterialTheme.colorScheme.onSurfaceVariant,
                modifier = Modifier.size(26.dp),
            )
        }
        Spacer(Modifier.height(18.dp))
        Text(
            "Belum ada data",
            style = MaterialTheme.typography.titleMedium,
            color = MaterialTheme.colorScheme.onSurface,
        )
        Spacer(Modifier.height(9.dp))
        Text(
            "Perangkat menyimpan satu titik tiap 10 menit. Grafik menampilkan hingga 144 titik untuk 24 jam.",
            style = MaterialTheme.typography.bodyMedium.copy(fontSize = 12.5.sp, lineHeight = 20.sp),
            color = MaterialTheme.colorScheme.onSurfaceVariant,
            textAlign = androidx.compose.ui.text.style.TextAlign.Center,
        )
    }
}
