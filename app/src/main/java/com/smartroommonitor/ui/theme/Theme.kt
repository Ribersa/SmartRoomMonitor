package com.smartroommonitor.ui.theme

import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.CompositionLocalProvider
import androidx.compose.runtime.Immutable
import androidx.compose.runtime.staticCompositionLocalOf
import androidx.compose.ui.graphics.Color

/** Token yang tidak ada di ColorScheme Material 3. */
@Immutable
data class AppColors(
    val faint: Color,
    val borderSoft: Color,
    val surface2: Color,
    val danger: Color,
    val dangerBg: Color,
    val dangerBorder: Color,
    val success: Color,
    val warning: Color,
)

val LocalAppColors = staticCompositionLocalOf {
    AppColors(
        faint = LightFaint, borderSoft = LightBorderSoft, surface2 = LightSurface2,
        danger = LightDanger, dangerBg = LightDangerBg, dangerBorder = LightDangerBorder,
        success = LightSuccess, warning = LightWarning,
    )
}

private val LightScheme = lightColorScheme(
    primary = LightText,
    onPrimary = LightBg,
    secondary = LightMuted,
    onSecondary = LightBg,
    background = LightBg,
    onBackground = LightText,
    surface = LightSurface,
    onSurface = LightText,
    surfaceVariant = LightSurface2,
    onSurfaceVariant = LightMuted,
    outline = LightBorder,
    outlineVariant = LightBorderSoft,
    error = LightDanger,
    onError = Color.White,
)

private val DarkScheme = darkColorScheme(
    primary = DarkText,
    onPrimary = DarkBg,
    secondary = DarkMuted,
    onSecondary = DarkBg,
    background = DarkBg,
    onBackground = DarkText,
    surface = DarkSurface,
    onSurface = DarkText,
    surfaceVariant = DarkSurface2,
    onSurfaceVariant = DarkMuted,
    outline = DarkBorder,
    outlineVariant = DarkBorderSoft,
    error = DarkDanger,
    onError = DarkBg,
)

@Composable
fun SmartRoomTheme(
    darkTheme: Boolean = isSystemInDarkTheme(),
    content: @Composable () -> Unit,
) {
    val appColors = if (darkTheme) {
        AppColors(
            faint = DarkFaint, borderSoft = DarkBorderSoft, surface2 = DarkSurface2,
            danger = DarkDanger, dangerBg = DarkDangerBg, dangerBorder = DarkDangerBorder,
            success = DarkSuccess, warning = DarkWarning,
        )
    } else {
        AppColors(
            faint = LightFaint, borderSoft = LightBorderSoft, surface2 = LightSurface2,
            danger = LightDanger, dangerBg = LightDangerBg, dangerBorder = LightDangerBorder,
            success = LightSuccess, warning = LightWarning,
        )
    }
    CompositionLocalProvider(LocalAppColors provides appColors) {
        MaterialTheme(
            colorScheme = if (darkTheme) DarkScheme else LightScheme,
            typography = AppTypography,
            shapes = AppShapes,
            content = content,
        )
    }
}
