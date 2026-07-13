package com.hearingaid

import expo.modules.kotlin.modules.Module
import expo.modules.kotlin.modules.ModuleDefinition

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

    // Убрали ключевое слово external, так как функции теперь вызываются напрямую внутри JNI
    private fun startEngine() = Java_com_hearingaid_HearingAidEngineModule_startEngine()
    private fun stopEngine() = Java_com_hearingaid_HearingAidEngineModule_stopEngine()

    private external fun Java_com_hearingaid_HearingAidEngineModule_startEngine()
    private external fun Java_com_hearingaid_HearingAidEngineModule_stopEngine()
}
