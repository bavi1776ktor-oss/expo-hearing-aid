package com.hearingaid

import android.content.Context
import android.content.Intent
import expo.modules.kotlin.modules.Module
import expo.modules.kotlin.modules.ModuleDefinition

class HearingAidEngineModule : Module() {
    private val context: Context
        get() = appContext.reactContext ?: throw IllegalStateException("Android context not available")

    companion object {
        init {
            System.loadLibrary("hearing_aid_engine")
        }
    }

    override fun definition() = ModuleDefinition {
        Name("HearingAidEngine")

        Function("start") {
            // Запускаем C++ движок
            startEngine()
            // Запускаем фоновый сервис
            val intent = Intent(context, HearingAidService::class.java).apply { action = "START" }
            context.startService(intent)
        }

        Function("stop") {
            // Останавливаем C++ движок
            stopEngine()
            // Останавливаем фоновый сервис
            val intent = Intent(context, HearingAidService::class.java).apply { action = "STOP" }
            context.startService(intent)
        }
    }

    private external fun startEngine()
    private external fun stopEngine()
}
