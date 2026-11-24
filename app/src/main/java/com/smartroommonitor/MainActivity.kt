package com.smartroommonitor

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import com.smartroommonitor.ui.navigation.AppNavHost
import com.smartroommonitor.ui.profile.ThemeMode
import com.smartroommonitor.ui.theme.SmartRoomTheme

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val container = (application as SmartRoomApp).container

        setContent {
            var themeMode by remember {
                mutableStateOf(ThemeMode.from(container.preferenceStore.themeMode))
            }
            val dark = when (themeMode) {
                ThemeMode.Light -> false
                ThemeMode.Dark -> true
                ThemeMode.System -> isSystemInDarkTheme()
            }

            SmartRoomTheme(darkTheme = dark) {
                AppNavHost(
                    container = container,
                    themeMode = themeMode,
                    onThemeChange = { mode ->
                        themeMode = mode
                        container.preferenceStore.themeMode = mode.key
                    },
                )
            }
        }
    }
}
