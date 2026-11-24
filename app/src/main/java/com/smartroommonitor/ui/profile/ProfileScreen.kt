package com.smartroommonitor.ui.profile

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
import androidx.compose.material.icons.outlined.Check
import androidx.compose.material.icons.outlined.Lock
import androidx.compose.material.icons.outlined.Logout
import androidx.compose.material.icons.outlined.Shield
import androidx.compose.material.icons.outlined.Tune
import androidx.compose.material.icons.outlined.Visibility
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.smartroommonitor.domain.model.Role
import com.smartroommonitor.ui.theme.LocalAppColors

enum class ThemeMode(val key: String, val label: String) {
    System("system", "Ikut sistem"),
    Light("light", "Terang"),
    Dark("dark", "Gelap");

    companion object {
        fun from(key: String?): ThemeMode = entries.firstOrNull { it.key == key } ?: System
    }
}

@Composable
fun ProfileScreen(
    role: Role,
    email: String,
    themeMode: ThemeMode,
    showLogoutDialog: Boolean,
    onThemeChange: (ThemeMode) -> Unit,
    onLogoutRequest: () -> Unit,
    onLogoutDismiss: () -> Unit,
    onLogoutConfirm: () -> Unit,
    modifier: Modifier = Modifier,
) {
    val app = LocalAppColors.current

    Column(
        modifier
            .fillMaxSize()
            .verticalScroll(rememberScrollState())
            .padding(horizontal = 20.dp),
    ) {
        Spacer(Modifier.height(8.dp))
        Text(
            "Profil",
            style = MaterialTheme.typography.headlineMedium,
            color = MaterialTheme.colorScheme.onBackground,
        )
        Spacer(Modifier.height(4.dp))
        Text(
            "Akun & preferensi",
            style = MaterialTheme.typography.bodyMedium,
            color = MaterialTheme.colorScheme.onSurfaceVariant,
        )

        Spacer(Modifier.height(18.dp))
        Row(verticalAlignment = Alignment.CenterVertically) {
            Box(
                Modifier
                    .size(56.dp)
                    .background(app.surface2, CircleShape)
                    .border(1.dp, MaterialTheme.colorScheme.outline, CircleShape),
                contentAlignment = Alignment.Center,
            ) {
                Text(
                    role.name.take(2).uppercase(),
                    style = MaterialTheme.typography.titleLarge,
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                )
            }
            Spacer(Modifier.width(14.dp))
            Column {
                Text(
                    role.name,
                    style = MaterialTheme.typography.titleLarge.copy(fontSize = 18.sp),
                    color = MaterialTheme.colorScheme.onSurface,
                )
                Spacer(Modifier.height(3.dp))
                Text(
                    email.ifBlank { "Tidak ada email" },
                    style = MaterialTheme.typography.bodyMedium.copy(fontSize = 12.sp),
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                )
            }
        }

        Spacer(Modifier.height(20.dp))
        SectionTitle("Tampilan")
        Column(
            Modifier
                .fillMaxWidth()
                .background(MaterialTheme.colorScheme.surfaceVariant, RoundedCornerShape(14.dp))
                .border(1.dp, MaterialTheme.colorScheme.outline, RoundedCornerShape(14.dp))
                .padding(4.dp),
        ) {
            Row {
                ThemeMode.entries.forEach { mode ->
                    val active = mode == themeMode
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
                            .clickable { onThemeChange(mode) },
                        contentAlignment = Alignment.Center,
                    ) {
                        Text(
                            mode.label,
                            style = MaterialTheme.typography.labelLarge.copy(
                                fontSize = 12.sp,
                                fontWeight = if (active) FontWeight.Bold else FontWeight.SemiBold,
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
        Spacer(Modifier.height(8.dp))
        Text(
            "Pilih Terang atau Gelap untuk mengubah tampilan tanpa mengikuti sistem.",
            style = MaterialTheme.typography.bodyMedium.copy(fontSize = 11.5.sp, lineHeight = 18.sp),
            color = app.faint,
        )

        Spacer(Modifier.height(20.dp))
        SectionTitle("Hak akses")
        Column(
            Modifier
                .fillMaxWidth()
                .background(MaterialTheme.colorScheme.surface, RoundedCornerShape(20.dp))
                .border(1.dp, app.borderSoft, RoundedCornerShape(20.dp))
                .padding(horizontal = 16.dp),
        ) {
            AccessRow(
                icon = Icons.Outlined.Visibility,
                label = "Lihat dashboard & grafik",
                granted = true,
            )
            HorizontalDivider(color = app.borderSoft)
            AccessRow(
                icon = Icons.Outlined.Tune,
                label = "Kontrol kipas, lampu & MUTE",
                granted = role.canControl,
            )
            HorizontalDivider(color = app.borderSoft)
            AccessRow(
                icon = Icons.Outlined.Shield,
                label = "Ubah batas peringatan",
                granted = false,
                note = "Lewat Firebase Console",
            )
        }

        Spacer(Modifier.height(12.dp))
        Text(
            "Batas peringatan diubah melalui Firebase Console, bukan dari aplikasi.",
            style = MaterialTheme.typography.bodyMedium.copy(fontSize = 11.5.sp, lineHeight = 18.sp),
            color = app.faint,
        )

        Spacer(Modifier.height(16.dp))
        Button(
            onClick = onLogoutRequest,
            modifier = Modifier
                .fillMaxWidth()
                .heightIn(min = 48.dp),
            shape = RoundedCornerShape(14.dp),
            colors = ButtonDefaults.buttonColors(
                containerColor = MaterialTheme.colorScheme.surface,
                contentColor = MaterialTheme.colorScheme.onSurface,
            ),
        ) {
            Icon(Icons.Outlined.Logout, contentDescription = null, modifier = Modifier.size(18.dp))
            Spacer(Modifier.width(9.dp))
            Text("Keluar", fontWeight = FontWeight.Bold, fontSize = 15.sp)
        }

        Spacer(Modifier.height(20.dp))
        Text(
            "SmartRoomMonitor · SRS v1.0 · ruang room1",
            style = MaterialTheme.typography.bodyMedium.copy(fontSize = 11.sp),
            color = app.faint,
            modifier = Modifier.fillMaxWidth(),
        )
        Spacer(Modifier.height(24.dp))
    }

    if (showLogoutDialog) {
        AlertDialog(
            onDismissRequest = onLogoutDismiss,
            containerColor = MaterialTheme.colorScheme.surface,
            shape = RoundedCornerShape(20.dp),
            title = {
                Text(
                    "Keluar dari akun?",
                    style = MaterialTheme.typography.titleMedium,
                    color = MaterialTheme.colorScheme.onSurface,
                )
            },
            text = {
                Text(
                    "Sesi Anda akan dihapus dan Anda kembali ke layar masuk. Data ruangan tetap tersimpan.",
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                )
            },
            confirmButton = {
                Button(
                    onClick = onLogoutConfirm,
                    shape = RoundedCornerShape(14.dp),
                    colors = ButtonDefaults.buttonColors(
                        containerColor = MaterialTheme.colorScheme.primary,
                        contentColor = MaterialTheme.colorScheme.background,
                    ),
                ) { Text("Keluar") }
            },
            dismissButton = {
                TextButton(onClick = onLogoutDismiss) {
                    Text("Batal", color = MaterialTheme.colorScheme.onSurface)
                }
            },
        )
    }
}

@Composable
private fun SectionTitle(text: String) {
    Text(
        text,
        style = MaterialTheme.typography.titleMedium.copy(fontSize = 15.sp),
        color = MaterialTheme.colorScheme.onSurface,
        modifier = Modifier.padding(bottom = 10.dp),
    )
}

@Composable
private fun AccessRow(
    icon: androidx.compose.ui.graphics.vector.ImageVector,
    label: String,
    granted: Boolean,
    note: String? = null,
) {
    val app = LocalAppColors.current
    Row(
        Modifier
            .fillMaxWidth()
            .heightIn(min = 52.dp)
            .padding(vertical = 12.dp),
        verticalAlignment = Alignment.CenterVertically,
    ) {
        Icon(
            icon,
            contentDescription = null,
            tint = MaterialTheme.colorScheme.onSurfaceVariant,
            modifier = Modifier.size(19.dp),
        )
        Spacer(Modifier.width(12.dp))
        Column(Modifier.weight(1f)) {
            Text(
                label,
                style = MaterialTheme.typography.bodyLarge.copy(fontSize = 13.5.sp, fontWeight = FontWeight.SemiBold),
                color = MaterialTheme.colorScheme.onSurface,
            )
            if (note != null) {
                Text(
                    note,
                    style = MaterialTheme.typography.bodyMedium.copy(fontSize = 11.sp),
                    color = app.faint,
                )
            }
        }
        Icon(
            if (granted) Icons.Outlined.Check else Icons.Outlined.Lock,
            contentDescription = if (granted) "Diizinkan" else "Tidak diizinkan",
            tint = if (granted) app.success else app.faint,
            modifier = Modifier.size(18.dp),
        )
    }
}
