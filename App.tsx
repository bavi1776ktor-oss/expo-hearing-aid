import React, { useState, useEffect } from 'react';
import { StyleSheet, Text, View, TouchableOpacity, Alert } from 'react-native';
import { Audio } from 'expo-av';
// @ts-ignore - игнорируем отсутствие декларации типов для кастомного нативного модуля
import HearingAidEngine from './modules/hearing-aid-engine';

export default function App() {
  const [isActive, setIsActive] = useState<boolean>(false);
  const [volume, setVolume] = useState<number>(1.0); // 1.0 = 100% громкости по умолчанию

  useEffect(() => {
    // При изменении стейта громкости отправляем её в C++ движок
    HearingAidEngine.setVolume(volume);
  }, [volume]);

  const toggleEngine = async (): Promise<void> => {
    if (!isActive) {
      const { status } = await Audio.requestPermissionsAsync();
      if (status !== 'granted') {
        Alert.alert('Ошибка', 'Необходим доступ к микрофону для работы слухового аппарата.');
        return;
      }
      HearingAidEngine.start();
      setIsActive(true);
    } else {
      HearingAidEngine.stop();
      setIsActive(false);
    }
  };

  // Изменение громкости кнопками на экране
  const increaseVolume = (): void => {
    setVolume((prev: number) => Math.min(prev + 0.2, 2.0)); // Максимум 200%
  };

  const decreaseVolume = (): void => {
    setVolume((prev: number) => Math.max(prev - 0.2, 0.0)); // Минимум 0% (тишина)
  };

  return (
    <View style={styles.container}>
      <Text style={styles.title}>Слуховой Аппарат v2</Text>
      
      <TouchableOpacity 
        style={[styles.button, isActive ? styles.buttonActive : styles.buttonInactive]} 
        onPress={toggleEngine}
      >
        <Text style={styles.buttonText}>{isActive ? 'ВЫКЛ' : 'ВКЛ'}</Text>
      </TouchableOpacity>

      <Text style={styles.statusText}>
        Статус: {isActive ? 'Работает в фоне' : 'Остановлен'}
      </Text>

      <View style={styles.volumeContainer}>
        <Text style={styles.volumeLabel}>
          Собственная громкость: {Math.round(volume * 100)}%
        </Text>
        
        <View style={styles.controlsRow}>
          <TouchableOpacity style={styles.volButton} onPress={decreaseVolume}>
            <Text style={styles.volButtonText}>-</Text>
          </TouchableOpacity>
          
          <View style={styles.progressBarBg}>
            <View style={[styles.progressBarFill, { width: `${(volume / 2.0) * 100}%` }]} />
          </View>

          <TouchableOpacity style={styles.volButton} onPress={increaseVolume}>
            <Text style={styles.volButtonText}>+</Text>
          </TouchableOpacity>
        </View>
      </View>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#121212',
    alignItems: 'center',
    justifyContent: 'center',
    padding: 20,
  },
  title: {
    fontSize: 26,
    fontWeight: 'bold',
    color: '#ffffff',
    marginBottom: 40,
  },
  button: {
    width: 150,
    height: 150,
    borderRadius: 75,
    justifyContent: 'center',
    alignItems: 'center',
    elevation: 5,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.25,
    shadowRadius: 3.84,
  },
  buttonActive: {
    backgroundColor: '#cf6679',
  },
  buttonInactive: {
    backgroundColor: '#03dac6',
  },
  buttonText: {
    fontSize: 28,
    fontWeight: 'bold',
    color: '#000000',
  },
  statusText: {
    marginTop: 20,
    fontSize: 16,
    color: '#bbbbbb',
    marginBottom: 40,
  },
  volumeContainer: {
    width: '100%',
    paddingHorizontal: 20,
    alignItems: 'center',
  },
  volumeLabel: {
    fontSize: 18,
    color: '#ffffff',
    marginBottom: 15,
  },
  controlsRow: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'space-between',
    width: '100%',
  },
  volButton: {
    width: 50,
    height: 50,
    borderRadius: 25,
    backgroundColor: '#333333',
    justifyContent: 'center',
    alignItems: 'center',
  },
  volButtonText: {
    color: '#ffffff',
    fontSize: 24,
    fontWeight: 'bold',
  },
  progressBarBg: {
    flex: 1,
    height: 12,
    backgroundColor: '#222222',
    borderRadius: 6,
    marginHorizontal: 15,
    overflow: 'hidden',
  },
  progressBarFill: {
    height: '100%',
    backgroundColor: '#03dac6',
    borderRadius: 6,
  },
});
