package com.hearingaid

import android.content.Context
import expo.modules.core.interfaces.Package
import expo.modules.core.interfaces.InternalModule

class HearingAidEnginePackage : Package {
    override fun createInternalModules(context: Context): List<InternalModule> {
        return emptyList()
    }
}
