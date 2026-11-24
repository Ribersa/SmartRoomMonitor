package com.smartroommonitor.ui.events

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
import androidx.compose.material.icons.outlined.Inbox
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.smartroommonitor.domain.model.RoomEvent
import com.smartroommonitor.ui.theme.LocalAppColors
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

private val ID = Locale("id", "ID")

@Composable
fun EventListScreen(
    state: EventUiState,
    onFilter: (EventFilter) -> Unit,
    modifier: Modifier = Modifier,
) {
    val app = LocalAppColors.current
    val visible = state.visible

    Column(
        modifier
            .fillMaxSize()
            .verticalScroll(rememberScrollState())
            .padding(horizontal = 20.dp),
    ) {
        Spacer(Modifier.height(8.dp))
        Text(
            "Kejadian",
            style = MaterialTheme.typography.headlineMedium,
            color = MaterialTheme.colorScheme.onBackground,
        )
        Spacer(Modifier.height(4.dp))
        Text(
            "24 jam terakhir · ${state.all.size} catatan",
            style = MaterialTheme.typography.bodyMedium,
            color = MaterialTheme.colorScheme.onSurfaceVariant,
        )
        Spacer(Modifier.height(14.dp))

        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            EventFilter.entries.forEach { filter ->
                FilterChip(
                    label = filter.label,
                    active = filter == state.filter,
                    onClick = { onFilter(filter) },
                )
            }
        }
        Spacer(Modifier.height(14.dp))

        when {
            state.loading -> Box(
                Modifier
                    .fillMaxWidth()
                    .height(180.dp)
                    .background(app.surface2, RoundedCornerShape(20.dp)),
            )

            visible.isEmpty() -> EmptyEvents(state)

            else -> Column(
                Modifier
                    .fillMaxWidth()
                    .background(MaterialTheme.colorScheme.surface, RoundedCornerShape(20.dp))
                    .border(1.dp, app.borderSoft, RoundedCornerShape(20.dp))
                    .padding(horizontal = 16.dp),
            ) {
                visible.forEachIndexed { index, event ->
                    EventRow(event)
                    if (index != visible.lastIndex) {
                        HorizontalDivider(color = app.borderSoft)
                    }
                }
            }
        }

        if (state.error != null) {
            Spacer(Modifier.height(10.dp))
            Text(state.error, style = MaterialTheme.typography.bodyMedium, color = app.danger)
        }
        Spacer(Modifier.height(12.dp))
        Text(
            "Warna bukan satu-satunya penanda. Setiap baris memuat label tipe dan tingkat keparahan.",
            style = MaterialTheme.typography.bodyMedium.copy(fontSize = 11.5.sp, lineHeight = 18.sp),
            color = app.faint,
        )
        Spacer(Modifier.height(24.dp))
    }
}

@Composable
private fun FilterChip(label: String, active: Boolean, onClick: () -> Unit) {
    Row(
        Modifier
            .heightIn(min = 36.dp)
            .background(
                if (active) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.surface,
                RoundedCornerShape(999.dp),
            )
            .border(
                1.dp,
                if (active) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.outline,
                RoundedCornerShape(999.dp),
            )
            .clickable(onClick = onClick)
            .padding(horizontal = 13.dp, vertical = 8.dp),
    ) {
        Text(
            label,
            style = MaterialTheme.typography.labelLarge.copy(fontSize = 12.sp),
            color = if (active) MaterialTheme.colorScheme.background else MaterialTheme.colorScheme.onSurface,
        )
    }
}

@Composable
private fun EventRow(event: RoomEvent) {
    val app = LocalAppColors.current
    val dot = if (event.severity == "danger") app.danger else app.warning
    val time = SimpleDateFormat("HH:mm", ID).format(Date(event.timestampMs))

    Row(Modifier.padding(vertical = 14.dp)) {
        Box(
            Modifier
                .padding(top = 6.dp)
                .size(9.dp)
                .background(dot, CircleShape),
        )
        Spacer(Modifier.width(12.dp))
        Column(Modifier.weight(1f)) {
            Row(
                Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.Bottom,
            ) {
                Text(
                    event.typeLabel,
                    style = MaterialTheme.typography.labelLarge.copy(fontSize = 13.5.sp, fontWeight = FontWeight.Bold),
                    color = MaterialTheme.colorScheme.onSurface,
                )
                Text(
                    time,
                    style = MaterialTheme.typography.bodyMedium.copy(fontSize = 11.sp),
                    color = app.faint,
                )
            }
            Spacer(Modifier.height(4.dp))
            Text(
                event.message,
                style = MaterialTheme.typography.bodyMedium.copy(fontSize = 12.sp, lineHeight = 18.sp),
                color = MaterialTheme.colorScheme.onSurfaceVariant,
            )
            Spacer(Modifier.height(6.dp))
            Text(
                "${event.type.uppercase(ID)} · ${event.severityLabel}",
                style = MaterialTheme.typography.labelSmall.copy(
                    fontSize = 10.5.sp, letterSpacing = 0.3.sp,
                ),
                color = app.faint,
            )
        }
    }
}

@Composable
private fun EmptyEvents(state: EventUiState) {
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
                Icons.Outlined.Inbox,
                contentDescription = null,
                tint = MaterialTheme.colorScheme.onSurfaceVariant,
                modifier = Modifier.size(26.dp),
            )
        }
        Spacer(Modifier.height(18.dp))
        Text(
            if (state.filter == EventFilter.All) "Tidak ada kejadian" else "Tidak ada kejadian ${state.filter.label.lowercase()}",
            style = MaterialTheme.typography.titleMedium,
            color = MaterialTheme.colorScheme.onSurface,
        )
        Spacer(Modifier.height(9.dp))
        Text(
            "Belum ada kondisi bahaya dalam 24 jam terakhir. Kejadian suhu tinggi, udara buruk, dan kehadiran jam malam akan muncul di sini.",
            style = MaterialTheme.typography.bodyMedium.copy(fontSize = 12.5.sp, lineHeight = 20.sp),
            color = MaterialTheme.colorScheme.onSurfaceVariant,
            textAlign = TextAlign.Center,
        )
    }
}
