import { requireNativeModule } from 'expo-modules-core';

const HearingAidEngine = requireNativeModule('HearingAidEngine');

export function startHearingAid() {
  HearingAidEngine.start();
}

export function stopHearingAid() {
  HearingAidEngine.stop();
}
