import React, { useState, useEffect, useRef } from 'react';
import { StyleSheet, Text, View, TouchableOpacity, Alert, AppState } from 'react-native';
import { Audio } from 'expo-av';
import Slider from '@react-native-community/slider'; // ← добавь этот пакет

// Импорт нативного модуля
const HearingAidEngine = requireNativeModule('HearingAidEngine');

export default function App() {
  const [isActive, setIsActive] = useState(false);
  const [volume, setVolume] = useState(1.0);        // Выходная громкость (0.0 - 2.0)
  const [sensitivity, setSensitivity] = useState(1.0); // Чувствительность микрофона (0.5 - 3.0)
  const appState = useRef(AppState.currentState);

  // Обновление громкости в нативном движке
  useEffect(() => {
    if (HearingAidEngine?.setVolume) {
      HearingAidEngine.setVolume(volume);
    }
  }, [volume]);

  // TODO: Добавить setSensitivity в нативный модуль позже
  // useEffect(() => {
  //   if (HearingAidEngine?.setSensitivity) {
  //     HearingAidEngine.setSensitivity(sensitivity);
  //   }
  // }, [sensitivity]);

  // Следим за сворачиванием приложения
  useEffect(() => {
    const subscription = AppState.addEventListener('change', nextAppState => {
      if (appState.current.match(/inactive|background/) && nextAppState === 'active') {
        // вернулись в приложение
      } else if (isActive && nextAppState === 'background') {
        // можно оставить работать в фоне
      }
      appState.current = nextAppState;
    });

    return () => subscription.remove();
  }, [isActive]);

  const toggleEngine = async () => {
    if (!HearingAidEngine) {
      Alert.alert('Ошибка', 'Нативный модуль не загружен');
      return;
    }

    if (!isActive) {
      const { status } = await Audio.requestPermissionsAsync();
      if (status !== 'granted') {
        Alert.alert('Доступ запрещён', 'Нужен микрофон для работы приложения');
        return;
      }

      try {
        HearingAidEngine.start();
        setIsActive(true);
      } catch (e) {
        Alert.alert('Ошибка запуска', 'Не удалось запустить движок');
        console.error(e);
      }
    } else {
      try {
        HearingAidEngine.stop();
        setIsActive(false);
      } catch (e) {
        console.error(e);
      }
    }
  };

  return (
    <View style={styles.container}>
      <Text style={styles.title}>Слуховой Аппарат</Text>

      <TouchableOpacity
        style={[styles.powerButton, isActive ? styles.active : styles.inactive]}
        onPress={toggleEngine}
      >
        <Text style={styles.powerText}>{isActive ? 'ВЫКЛЮЧИТЬ' : 'ВКЛЮЧИТЬ'}</Text>
      </TouchableOpacity>

      <Text style={styles.status}>
        Статус: {isActive ? '🟢 Работает' : '⚪ Остановлен'}
      </Text>

      {/* Громкость на выходе */}
      <View style={styles.sliderContainer}>
        <Text style={styles.label}>Громкость: {Math.round(volume * 100)}%</Text>
        <Slider
          style={styles.slider}
          minimumValue={0}
          maximumValue={2}
          value={volume}
          onValueChange={setVolume}
          minimumTrackTintColor="#03dac6"
          maximumTrackTintColor="#555"
          thumbTintColor="#03dac6"
        />
      </View>

      {/* Чувствительность микрофона (пока placeholder) */}
      <View style={styles.sliderContainer}>
        <Text style={styles.label}>Чувствительность микрофона: {Math.round(sensitivity * 100)}%</Text>
        <Slider
          style={styles.slider}
          minimumValue={0.5}
          maximumValue={3.0}
          value={sensitivity}
          onValueChange={setSensitivity}
          minimumTrackTintColor="#ff9800"
          maximumTrackTintColor="#555"
          thumbTintColor="#ff9800"
        />
      </View>
    </View>
  );
}

const styles = StyleSheet.create({
  container: { flex: 1, backgroundColor: '#121212', alignItems: 'center', justifyContent: 'center', padding: 20 },
  title: { fontSize: 28, fontWeight: 'bold', color: '#fff', marginBottom: 40 },
  powerButton: { width: 160, height: 160, borderRadius: 80, justifyContent: 'center', alignItems: 'center' },
  active: { backgroundColor: '#cf6679' },
  inactive: { backgroundColor: '#03dac6' },
  powerText: { fontSize: 24, fontWeight: 'bold', color: '#000' },
  status: { marginTop: 20, fontSize: 18, color: '#bbb' },
  sliderContainer: { width: '100%', marginTop: 30 },
  label: { color: '#fff', fontSize: 16, marginBottom: 10, textAlign: 'center' },
  slider: { width: '100%', height: 40 },
});
