package com.smartroommonitor.data.local

import android.content.Context
import com.smartroommonitor.domain.model.ChartMetric

/** Preferensi ringan: metrik grafik terakhir, state expand, dan mode tema. */
class PreferenceStore(context: Context) {

    private val prefs = context.applicationContext
        .getSharedPreferences("preferences", Context.MODE_PRIVATE)

    var chartMetric: ChartMetric
        get() = runCatching {
            ChartMetric.valueOf(prefs.getString(KEY_METRIC, ChartMetric.Temperature.name)!!)
        }.getOrDefault(ChartMetric.Temperature)
        set(value) = prefs.edit().putString(KEY_METRIC, value.name).apply()

    var chartExpanded: Boolean
        get() = prefs.getBoolean(KEY_EXPANDED, false)
        set(value) = prefs.edit().putBoolean(KEY_EXPANDED, value).apply()

    /** "system" | "light" | "dark" */
    var themeMode: String
        get() = prefs.getString(KEY_THEME, "system") ?: "system"
        set(value) = prefs.edit().putString(KEY_THEME, value).apply()

    private companion object {
        const val KEY_METRIC = "chart_metric"
        const val KEY_EXPANDED = "chart_expanded"
        const val KEY_THEME = "theme_mode"
    }
}
