package com.smartroommonitor

import android.app.Application
import com.smartroommonitor.di.AppContainer

class SmartRoomApp : Application() {
    lateinit var container: AppContainer
        private set

    override fun onCreate() {
        super.onCreate()
        container = AppContainer(this)
    }
}
