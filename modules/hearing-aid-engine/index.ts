import { requireNativeModule } from 'expo-modules-core';

// Явно объявляем интерфейс нашего C++/Kotlin модуля
interface HearingAidEngineModule {
  start(): void;
  stop(): void;
  setVolume(volume: number): void;
}

const HearingAidEngine = requireNativeModule<HearingAidEngineModule>('HearingAidEngine');

export default HearingAidEngine;
