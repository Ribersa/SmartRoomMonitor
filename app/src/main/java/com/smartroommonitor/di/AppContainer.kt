package com.smartroommonitor.di

import android.content.Context
import com.smartroommonitor.data.auth.AuthRepository
import androidx.room.Room
import com.smartroommonitor.data.local.AppDatabase
import com.smartroommonitor.data.local.MIGRATION_1_2
import com.smartroommonitor.data.local.PreferenceStore
import com.smartroommonitor.data.rtdb.ActuatorDataSource
import com.smartroommonitor.data.rtdb.EventDataSource
import com.smartroommonitor.data.rtdb.HistoryDataSource
import com.smartroommonitor.data.rtdb.RoomAlarmDataSource
import com.smartroommonitor.data.rtdb.ThresholdDataSource
import com.smartroommonitor.data.rtdb.RoomCurrentDataSource
import com.smartroommonitor.data.user.UserRepository
import com.smartroommonitor.ui.session.SessionStore

/**
 * Manual dependency container. Ponytail: pindah ke Hilt kalau layar melewati 12.
 */
class AppContainer(context: Context) {
    private val appContext = context.applicationContext

    val authRepository: AuthRepository by lazy { AuthRepository(appContext) }
    val userRepository: UserRepository by lazy { UserRepository(appContext) }
    val sessionStore: SessionStore by lazy { SessionStore(appContext) }
    val roomCurrentDataSource: RoomCurrentDataSource by lazy { RoomCurrentDataSource(appContext) }
    val roomAlarmDataSource: RoomAlarmDataSource by lazy { RoomAlarmDataSource(appContext) }
    val actuatorDataSource: ActuatorDataSource by lazy { ActuatorDataSource(appContext) }
    val thresholdDataSource: ThresholdDataSource by lazy { ThresholdDataSource(appContext) }
    val historyDataSource: HistoryDataSource by lazy { HistoryDataSource(appContext) }
    val eventDataSource: EventDataSource by lazy { EventDataSource(appContext) }
    val preferenceStore: PreferenceStore by lazy { PreferenceStore(appContext) }

    val database: AppDatabase by lazy {
        Room.databaseBuilder(appContext, AppDatabase::class.java, "smartroom.db")
            .addMigrations(MIGRATION_1_2)
            .build()
    }
}
