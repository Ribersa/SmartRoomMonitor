package com.smartroommonitor.ui.theme

import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Shapes
import androidx.compose.material3.Typography
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.font.Font
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.smartroommonitor.R

val PlusJakarta = FontFamily(
    Font(R.font.plus_jakarta_sans_regular, FontWeight.Normal),
    Font(R.font.plus_jakarta_sans_medium, FontWeight.Medium),
    Font(R.font.plus_jakarta_sans_semibold, FontWeight.SemiBold),
    Font(R.font.plus_jakarta_sans_bold, FontWeight.Bold),
)

val AppTypography = Typography(
    headlineMedium = TextStyle(
        fontFamily = PlusJakarta, fontWeight = FontWeight.Bold,
        fontSize = 26.sp, lineHeight = 33.sp, letterSpacing = (-0.4).sp,
    ),
    titleLarge = TextStyle(
        fontFamily = PlusJakarta, fontWeight = FontWeight.Bold,
        fontSize = 22.sp, lineHeight = 28.sp,
    ),
    titleMedium = TextStyle(
        fontFamily = PlusJakarta, fontWeight = FontWeight.SemiBold,
        fontSize = 17.sp, lineHeight = 24.sp,
    ),
    bodyLarge = TextStyle(
        fontFamily = PlusJakarta, fontWeight = FontWeight.Medium,
        fontSize = 15.sp, lineHeight = 23.sp,
    ),
    bodyMedium = TextStyle(
        fontFamily = PlusJakarta, fontWeight = FontWeight.Normal,
        fontSize = 13.sp, lineHeight = 20.sp,
    ),
    labelLarge = TextStyle(
        fontFamily = PlusJakarta, fontWeight = FontWeight.SemiBold,
        fontSize = 13.sp, lineHeight = 18.sp,
    ),
    labelSmall = TextStyle(
        fontFamily = PlusJakarta, fontWeight = FontWeight.Bold,
        fontSize = 11.sp, lineHeight = 15.sp, letterSpacing = 0.8.sp,
    ),
)

val AppShapes = Shapes(
    extraSmall = RoundedCornerShape(10.dp),
    small = RoundedCornerShape(12.dp),
    medium = RoundedCornerShape(14.dp),
    large = RoundedCornerShape(20.dp),
    extraLarge = RoundedCornerShape(22.dp),
)
