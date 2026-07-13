package com.hearingaid

import expo.modules.kotlin.modules.Module
import expo.modules.kotlin.modules.ModuleDefinition

// Добавляем обязательное имя для Expo
class HearingAidEngineModule : Module() {
    companion object {
        init {
            System.loadLibrary("hearing_aid_engine")
        }
    }

    override fun definition() = ModuleDefinition {
        Name("HearingAidEngine")

        Function("start") {
            startEngine()
        }

        Function("stop") {
            stopEngine()
        }
    }

    private external fun startEngine()
    private external fun stopEngine()
}
