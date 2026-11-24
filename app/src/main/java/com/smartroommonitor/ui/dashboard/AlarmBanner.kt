package com.smartroommonitor.ui.dashboard

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.outlined.VolumeOff
import androidx.compose.material.icons.outlined.WarningAmber
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.smartroommonitor.domain.model.RoomAlarm
import com.smartroommonitor.ui.theme.LocalAppColors

/**
 * Banner `room1/alarm`. Tombol MUTE hanya untuk Staff; Pengunjung melihat banner saja.
 * Re-arm 60 detik dikerjakan firmware, aplikasi hanya menampilkan status.
 */
@Composable
fun AlarmBanner(
    alarm: RoomAlarm,
    canControl: Boolean,
    muteSending: Boolean,
    muteError: String?,
    onMute: () -> Unit,
    modifier: Modifier = Modifier,
) {
    val app = LocalAppColors.current

    if (alarm.isMuted) {
        Row(
            modifier
                .fillMaxWidth()
                .background(MaterialTheme.colorScheme.surface, RoundedCornerShape(16.dp))
                .border(1.dp, MaterialTheme.colorScheme.outline, RoundedCornerShape(16.dp))
                .padding(14.dp),
        ) {
            Icon(
                Icons.Outlined.VolumeOff,
                contentDescription = null,
                tint = MaterialTheme.colorScheme.onSurfaceVariant,
                modifier = Modifier.size(20.dp),
            )
            Spacer(Modifier.width(12.dp))
            Column {
                Text(
                    "Alarm dibungkam",
                    style = MaterialTheme.typography.labelLarge,
                    color = MaterialTheme.colorScheme.onSurface,
                )
                Spacer(Modifier.height(3.dp))
                Text(
                    "Bunyi peringatan dimatikan. Alarm aktif lagi dalam 60 detik bila kondisi bahaya belum mereda.",
                    style = MaterialTheme.typography.bodyMedium.copy(fontSize = 12.sp, lineHeight = 18.sp),
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                )
            }
        }
        return
    }

    if (!alarm.active) return

    Column(modifier.fillMaxWidth()) {
        Row(
            Modifier
                .fillMaxWidth()
                .background(app.dangerBg, RoundedCornerShape(16.dp))
                .border(1.dp, app.dangerBorder, RoundedCornerShape(16.dp))
                .padding(14.dp),
        ) {
            Icon(
                Icons.Outlined.WarningAmber,
                contentDescription = null,
                tint = app.danger,
                modifier = Modifier.size(20.dp),
            )
            Spacer(Modifier.width(12.dp))
            Column {
                Text(
                    "Peringatan aktif",
                    style = MaterialTheme.typography.labelLarge.copy(fontWeight = FontWeight.Bold),
                    color = app.danger,
                )
                Spacer(Modifier.height(3.dp))
                Text(
                    "${alarm.reasonLabel}. Perangkat membunyikan alarm.",
                    style = MaterialTheme.typography.bodyMedium.copy(fontSize = 12.sp, lineHeight = 18.sp),
                    color = MaterialTheme.colorScheme.onBackground,
                )
            }
        }

        if (canControl) {
            Spacer(Modifier.height(10.dp))
            Button(
                onClick = onMute,
                enabled = !muteSending,
                modifier = Modifier
                    .fillMaxWidth()
                    .heightIn(min = 52.dp),
                shape = RoundedCornerShape(14.dp),
                colors = ButtonDefaults.buttonColors(
                    containerColor = app.danger,
                    contentColor = MaterialTheme.colorScheme.background,
                ),
            ) {
                if (muteSending) {
                    CircularProgressIndicator(
                        modifier = Modifier.size(16.dp),
                        strokeWidth = 2.dp,
                        color = MaterialTheme.colorScheme.background,
                    )
                    Spacer(Modifier.width(10.dp))
                } else {
                    Icon(
                        Icons.Outlined.VolumeOff,
                        contentDescription = null,
                        modifier = Modifier.size(18.dp),
                    )
                    Spacer(Modifier.width(9.dp))
                }
                Text("MUTE Darurat", fontWeight = FontWeight.Bold, fontSize = 15.sp)
            }
        } else {
            Spacer(Modifier.height(8.dp))
            Text(
                "Hanya Staff yang dapat membungkam alarm.",
                style = MaterialTheme.typography.bodyMedium.copy(fontSize = 11.5.sp),
                color = app.faint,
            )
        }

        if (muteError != null) {
            Spacer(Modifier.height(8.dp))
            Text(
                muteError,
                style = MaterialTheme.typography.bodyMedium,
                color = app.danger,
            )
        }
    }
}
