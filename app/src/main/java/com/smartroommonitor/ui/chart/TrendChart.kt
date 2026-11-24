package com.smartroommonitor.ui.chart

import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Path
import androidx.compose.ui.graphics.StrokeCap
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.smartroommonitor.domain.model.ChartMetric
import com.smartroommonitor.domain.model.HistoryPoint
import com.smartroommonitor.domain.usecase.valueOf
import com.smartroommonitor.ui.theme.LocalAppColors
import kotlin.math.roundToInt

private fun tickLabel(value: Double, metric: ChartMetric): String = when (metric) {
    ChartMetric.AirDelta -> value.roundToInt().toString()
    else -> String.format(java.util.Locale("id", "ID"), "%.0f", value)
}

@Composable
fun TrendChart(
    points: List<HistoryPoint>,
    metric: ChartMetric,
    limit: Double,
    limitLabel: String,
    modifier: Modifier = Modifier,
) {
    val app = LocalAppColors.current
    val lineColor = MaterialTheme.colorScheme.onSurface
    val values = points.map { it.valueOf(metric) }
    val rawMin = values.minOrNull() ?: 0.0
    val rawMax = values.maxOrNull() ?: 1.0
    val span = (rawMax - rawMin).coerceAtLeast(1.0)
    val yMin = (rawMin - span * 0.15).coerceAtLeast(0.0)
    val yMax = rawMax + span * 0.15
    val yRange = (yMax - yMin).coerceAtLeast(1.0)
    val ticks = listOf(yMin, yMin + yRange / 2, yMax)

    Column(modifier.fillMaxWidth()) {
        Row(Modifier.fillMaxWidth()) {
            Column(
                Modifier
                    .width(34.dp)
                    .height(150.dp),
                verticalArrangement = androidx.compose.foundation.layout.Arrangement.SpaceBetween,
            ) {
                ticks.reversed().forEach { t ->
                    Text(
                        tickLabel(t, metric),
                        style = MaterialTheme.typography.labelSmall.copy(fontSize = 9.sp, letterSpacing = 0.sp),
                        color = app.faint,
                    )
                }
            }
            Canvas(
                Modifier
                    .weight(1f)
                    .height(150.dp),
            ) {
                val w = size.width
                val h = size.height
                fun xAt(index: Int): Float =
                    if (values.size <= 1) w / 2f else index.toFloat() / (values.size - 1) * w
                fun yAt(value: Double): Float =
                    (h - ((value - yMin) / yRange * h)).toFloat().coerceIn(0f, h)

                ticks.forEach { t ->
                    drawLine(
                        color = app.borderSoft,
                        start = Offset(0f, yAt(t)),
                        end = Offset(w, yAt(t)),
                        strokeWidth = 1f,
                    )
                }

                if (limit in yMin..yMax) {
                    var startX = 0f
                    while (startX < w) {
                        drawLine(
                            color = app.danger,
                            start = Offset(startX, yAt(limit)),
                            end = Offset((startX + 12f).coerceAtMost(w), yAt(limit)),
                            strokeWidth = 2.4f,
                        )
                        startX += 22f
                    }
                }

                if (values.size >= 2) {
                    val path = Path().apply {
                        moveTo(xAt(0), yAt(values[0]))
                        for (i in 1 until values.size) lineTo(xAt(i), yAt(values[i]))
                    }
                    drawPath(
                        path = path,
                        color = lineColor,
                        style = Stroke(width = 4f, cap = StrokeCap.Round),
                    )
                    values.indices.forEach { i ->
                        drawCircle(
                            color = app.danger,
                            radius = 3.2f,
                            center = Offset(xAt(i), yAt(values[i])),
                        )
                    }
                }
            }
        }

        Spacer(Modifier.height(8.dp))
        Row(
            Modifier
                .fillMaxWidth()
                .padding(start = 34.dp),
            horizontalArrangement = androidx.compose.foundation.layout.Arrangement.SpaceBetween,
        ) {
            listOf("00.00", "12.00", "23.00").forEach { label ->
                Text(
                    label,
                    style = MaterialTheme.typography.labelSmall.copy(fontSize = 9.sp, letterSpacing = 0.sp),
                    color = app.faint,
                )
            }
        }

        Spacer(Modifier.height(10.dp))
        Row(Modifier.padding(start = 34.dp), verticalAlignment = Alignment.CenterVertically) {
            Box(
                Modifier
                    .width(16.dp)
                    .height(2.dp)
                    .background(MaterialTheme.colorScheme.onSurface),
            )
            Spacer(Modifier.width(6.dp))
            Text(
                "${metric.label} ruangan",
                style = MaterialTheme.typography.bodyMedium.copy(fontSize = 11.sp),
                color = MaterialTheme.colorScheme.onSurfaceVariant,
            )
            Spacer(Modifier.width(14.dp))
            Box(
                Modifier
                    .width(16.dp)
                    .height(2.dp)
                    .background(app.danger),
            )
            Spacer(Modifier.width(6.dp))
            Text(
                limitLabel,
                style = MaterialTheme.typography.bodyMedium.copy(fontSize = 11.sp),
                color = MaterialTheme.colorScheme.onSurfaceVariant,
            )
        }
    }
}
