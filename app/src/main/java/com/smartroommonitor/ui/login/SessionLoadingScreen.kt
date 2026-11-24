package com.smartroommonitor.ui.login

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.smartroommonitor.ui.theme.LocalAppColors

/** Layar 03: sesi sedang dibaca. Tanpa status "Tersambung" palsu. */
@Composable
fun SessionLoadingScreen(modifier: Modifier = Modifier) {
    val app = LocalAppColors.current
    Column(
        modifier
            .fillMaxSize()
            .background(MaterialTheme.colorScheme.background)
            .padding(horizontal = 20.dp),
    ) {
        Spacer(Modifier.height(28.dp))
        Spacer(
            Modifier
                .size(52.dp)
                .background(MaterialTheme.colorScheme.primary, RoundedCornerShape(16.dp)),
        )
        Spacer(Modifier.height(22.dp))
        Text(
            "SmartRoomMonitor",
            style = MaterialTheme.typography.headlineMedium,
            color = MaterialTheme.colorScheme.onBackground,
        )
        Spacer(Modifier.height(28.dp))
        SkeletonBar(widthFraction = 0.6f, height = 16)
        Spacer(Modifier.height(10.dp))
        SkeletonBar(widthFraction = 0.85f, height = 12)
        Spacer(Modifier.height(20.dp))
        SkeletonBar(widthFraction = 1f, height = 92, radius = 20)
        Spacer(Modifier.height(20.dp))
        Text(
            "Memuat sesi tersimpan…",
            style = MaterialTheme.typography.bodyMedium,
            color = app.faint,
        )
    }
}

@Composable
private fun SkeletonBar(widthFraction: Float, height: Int, radius: Int = 8) {
    Spacer(
        Modifier
            .fillMaxWidth(widthFraction)
            .height(height.dp)
            .background(LocalAppColors.current.surface2, RoundedCornerShape(radius.dp)),
    )
}
