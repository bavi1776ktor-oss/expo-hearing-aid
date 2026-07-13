import React, { useState } from 'react';
import { StyleSheet, Text, View, TouchableOpacity, PermissionsAndroid, Platform, Alert } from 'react-native';
import { startHearingAid, stopHearingAid } from './modules/hearing-aid-engine';

export default function App() {
  const [isActive, setIsActive] = useState(false);

  const requestMicrophonePermission = async () => {
    if (Platform.OS !== 'android') return true;
    
    try {
      const granted = await PermissionsAndroid.request(
        PermissionsAndroid.PERMISSIONS.RECORD_AUDIO,
        {
          title: "Доступ к микрофону",
          message: "Приложению нужен доступ к микрофону, чтобы усиливать окружающие звуки для наушников.",
          buttonNeutral: "Позже",
          buttonNegative: "Отмена",
          buttonPositive: "Разрешить",
        }
      );
      return granted === PermissionsAndroid.RESULTS.GRANTED;
    } catch (err) {
      console.warn(err);
      return false;
    }
  };

  const toggleHearingAid = async () => {
    if (!isActive) {
      // Проверяем/запрашиваем разрешение перед стартом
      const hasPermission = await requestMicrophonePermission();
      if (!hasPermission) {
        Alert.alert("Ошибка", "Без доступа к микрофону слуховой аппарат не сможет работать.");
        return;
      }
      startHearingAid();
    } else {
      stopHearingAid();
    }
    setIsActive(!isActive);
  };

  return (
    <View style={styles.container}>
      <Text style={styles.title}>Слуховой аппарат MVP</Text>
      <TouchableOpacity 
        style={[styles.button, isActive ? styles.buttonStop : styles.buttonStart]} 
        onPress={toggleHearingAid}
      >
        <Text style={styles.buttonText}>{isActive ? "ВЫКЛ" : "ВКЛ"}</Text>
      </TouchableOpacity>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    backgroundColor: '#f5f5f5',
  },
  title: {
    fontSize: 24,
    fontWeight: 'bold',
    marginBottom: 40,
    color: '#333',
  },
  button: {
    width: 160,
    height: 160,
    borderRadius: 80,
    justifyContent: 'center',
    alignItems: 'center',
    elevation: 5,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.25,
    shadowRadius: 3.84,
  },
  buttonStart: { backgroundColor: '#2e7d32' },
  buttonStop: { backgroundColor: '#c62828' },
  buttonText: { color: 'white', fontSize: 22, fontWeight: 'bold' },
});
