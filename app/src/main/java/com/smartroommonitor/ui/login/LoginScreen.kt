package com.smartroommonitor.ui.login

import androidx.compose.foundation.background
import androidx.compose.foundation.border
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
import androidx.compose.foundation.text.KeyboardActions
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Home
import androidx.compose.material.icons.filled.Visibility
import androidx.compose.material.icons.filled.VisibilityOff
import androidx.compose.material.icons.outlined.ErrorOutline
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.OutlinedTextFieldDefaults
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.focus.FocusRequester
import androidx.compose.ui.focus.focusRequester
import androidx.compose.ui.semantics.liveRegion
import androidx.compose.ui.semantics.LiveRegionMode
import androidx.compose.ui.semantics.semantics
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.input.PasswordVisualTransformation
import androidx.compose.ui.text.input.VisualTransformation
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.smartroommonitor.ui.theme.LocalAppColors

@Composable
fun LoginScreen(
    state: LoginUiState,
    onEmailChange: (String) -> Unit,
    onPasswordChange: (String) -> Unit,
    onTogglePassword: () -> Unit,
    onSubmit: () -> Unit,
    modifier: Modifier = Modifier,
) {
    val app = LocalAppColors.current
    val summaryFocus = remember { FocusRequester() }

    LaunchedEffect(state.errorSummary) {
        if (state.errorSummary != null) summaryFocus.requestFocus()
    }

    Column(
        modifier = modifier
            .fillMaxSize()
            .background(MaterialTheme.colorScheme.background)
            .verticalScroll(rememberScrollState())
            .padding(horizontal = 20.dp),
    ) {
        Spacer(Modifier.height(28.dp))

        Box(
            Modifier
                .size(52.dp)
                .background(MaterialTheme.colorScheme.primary, RoundedCornerShape(16.dp)),
            contentAlignment = Alignment.Center,
        ) {
            Icon(
                Icons.Filled.Home,
                contentDescription = null,
                tint = MaterialTheme.colorScheme.onPrimary,
                modifier = Modifier.size(28.dp),
            )
        }

        Spacer(Modifier.height(22.dp))
        Text(
            "SmartRoomMonitor",
            style = MaterialTheme.typography.headlineMedium.copy(fontSize = 28.sp, lineHeight = 35.sp),
            color = MaterialTheme.colorScheme.onBackground,
        )
        Spacer(Modifier.height(10.dp))
        Text(
            "Pantau kondisi ruangan, terima peringatan, dan atur kipas serta lampu dari satu layar.",
            style = MaterialTheme.typography.bodyMedium,
            color = MaterialTheme.colorScheme.onSurfaceVariant,
        )

        if (state.errorSummary != null) {
            Spacer(Modifier.height(18.dp))
            Row(
                Modifier
                    .fillMaxWidth()
                    .focusRequester(summaryFocus)
                    .semantics { liveRegion = LiveRegionMode.Polite }
                    .background(app.dangerBg, RoundedCornerShape(14.dp))
                    .border(1.dp, app.dangerBorder, RoundedCornerShape(14.dp))
                    .padding(13.dp),
            ) {
                Box(
                    Modifier
                        .width(3.dp)
                        .height(38.dp)
                        .background(app.danger, RoundedCornerShape(2.dp)),
                )
                Spacer(Modifier.width(11.dp))
                Column {
                    Row(verticalAlignment = Alignment.CenterVertically) {
                        Icon(
                            Icons.Outlined.ErrorOutline,
                            contentDescription = null,
                            tint = app.danger,
                            modifier = Modifier.size(16.dp),
                        )
                        Spacer(Modifier.width(7.dp))
                        Text(
                            "Tidak bisa masuk",
                            style = MaterialTheme.typography.labelLarge,
                            color = app.danger,
                        )
                    }
                    Spacer(Modifier.height(5.dp))
                    Text(
                        state.errorSummary,
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onBackground,
                    )
                }
            }
        }

        Spacer(Modifier.height(18.dp))
        FieldLabel("Email")
        Spacer(Modifier.height(8.dp))
        OutlinedTextField(
            value = state.email,
            onValueChange = onEmailChange,
            modifier = Modifier
                .fillMaxWidth()
                .heightIn(min = 50.dp),
            placeholder = { Text("nama@contoh.com", color = app.faint) },
            singleLine = true,
            isError = state.emailError != null,
            enabled = !state.loading,
            shape = RoundedCornerShape(14.dp),
            colors = fieldColors(),
            keyboardOptions = KeyboardOptions(
                keyboardType = KeyboardType.Email,
                imeAction = ImeAction.Next,
            ),
        )
        FieldError(state.emailError)

        Spacer(Modifier.height(16.dp))
        FieldLabel("Kata sandi")
        Spacer(Modifier.height(8.dp))
        OutlinedTextField(
            value = state.password,
            onValueChange = onPasswordChange,
            modifier = Modifier
                .fillMaxWidth()
                .heightIn(min = 50.dp),
            placeholder = { Text("••••••••", color = app.faint) },
            singleLine = true,
            isError = state.passwordError != null,
            enabled = !state.loading,
            shape = RoundedCornerShape(14.dp),
            colors = fieldColors(),
            visualTransformation = if (state.passwordVisible) {
                VisualTransformation.None
            } else {
                PasswordVisualTransformation()
            },
            trailingIcon = {
                IconButton(onClick = onTogglePassword, enabled = !state.loading) {
                    Icon(
                        if (state.passwordVisible) Icons.Filled.Visibility else Icons.Filled.VisibilityOff,
                        contentDescription = if (state.passwordVisible) {
                            "Sembunyikan kata sandi"
                        } else {
                            "Tampilkan kata sandi"
                        },
                        tint = MaterialTheme.colorScheme.onSurfaceVariant,
                        modifier = Modifier.size(18.dp),
                    )
                }
            },
            keyboardOptions = KeyboardOptions(
                keyboardType = KeyboardType.Password,
                imeAction = ImeAction.Done,
            ),
            keyboardActions = KeyboardActions(onDone = { onSubmit() }),
        )
        FieldError(state.passwordError)

        Spacer(Modifier.height(10.dp))
        Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.End) {
            TextButton(onClick = { }) {
                Text(
                    "Lupa kata sandi?",
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                )
            }
        }

        Spacer(Modifier.height(4.dp))
        Button(
            onClick = onSubmit,
            enabled = !state.loading,
            modifier = Modifier
                .fillMaxWidth()
                .heightIn(min = 50.dp),
            shape = RoundedCornerShape(14.dp),
            colors = ButtonDefaults.buttonColors(
                containerColor = MaterialTheme.colorScheme.primary,
                contentColor = MaterialTheme.colorScheme.background,
                disabledContainerColor = MaterialTheme.colorScheme.primary.copy(alpha = 0.45f),
                disabledContentColor = MaterialTheme.colorScheme.background,
            ),
        ) {
            if (state.loading) {
                CircularProgressIndicator(
                    modifier = Modifier.size(16.dp),
                    strokeWidth = 2.dp,
                    color = MaterialTheme.colorScheme.background,
                )
                Spacer(Modifier.width(10.dp))
                Text("Memeriksa akun…", fontWeight = FontWeight.Bold, fontSize = 15.sp)
            } else {
                Text("Masuk", fontWeight = FontWeight.Bold, fontSize = 15.sp)
            }
        }

        Spacer(Modifier.height(26.dp))
        Text(
            "Akses tanpa login ditolak.\nHubungi pengelola ruangan untuk mendapatkan akun.",
            style = MaterialTheme.typography.bodyMedium.copy(fontSize = 11.5.sp, lineHeight = 18.sp),
            color = app.faint,
            modifier = Modifier.fillMaxWidth(),
        )
        Spacer(Modifier.height(24.dp))
    }
}

@Composable
private fun FieldLabel(text: String) {
    Text(
        text,
        style = MaterialTheme.typography.labelLarge,
        color = MaterialTheme.colorScheme.onBackground,
    )
}

@Composable
private fun FieldError(message: String?) {
    if (message == null) return
    val app = LocalAppColors.current
    Row(Modifier.padding(top = 7.dp), verticalAlignment = Alignment.CenterVertically) {
        Icon(
            Icons.Outlined.ErrorOutline,
            contentDescription = null,
            tint = app.danger,
            modifier = Modifier.size(14.dp),
        )
        Spacer(Modifier.width(6.dp))
        Text(message, style = MaterialTheme.typography.bodyMedium, color = app.danger)
    }
}

@Composable
private fun fieldColors() = OutlinedTextFieldDefaults.colors(
    focusedBorderColor = MaterialTheme.colorScheme.primary,
    unfocusedBorderColor = MaterialTheme.colorScheme.outline,
    errorBorderColor = LocalAppColors.current.danger,
    focusedContainerColor = MaterialTheme.colorScheme.surface,
    unfocusedContainerColor = MaterialTheme.colorScheme.surface,
    errorContainerColor = MaterialTheme.colorScheme.surface,
    cursorColor = MaterialTheme.colorScheme.primary,
    focusedTextColor = MaterialTheme.colorScheme.onSurface,
    unfocusedTextColor = MaterialTheme.colorScheme.onSurface,
)
