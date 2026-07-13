#include <oboe/Oboe.h>
#include <android/log.h>
#include <jni.h>
#include <cmath>
#include <algorithm>

#define LOG_TAG "HearingAidEngine"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

// Структура цифрового фильтра BiQuad для частотной коррекции
struct BiQuadFilter {
    float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
    float x1 = 0, x2 = 0, y1 = 0, y2 = 0;

    void configureHighShelf(float sampleRate, float cutoffFreq, float dbGain) {
        float w0 = 2.0f * M_PI * cutoffFreq / sampleRate;
        float alpha = sin(w0) / 2.0f * sqrt(2.0f);
        float A = pow(10.0f, dbGain / 40.0f);
        float beta = sqrt(A) * 2.0f * alpha;

        float a0 = (A + 1.0f) - (A - 1.0f) * cos(w0) + beta;
        b0 = (A * ((A + 1.0f) + (A - 1.0f) * cos(w0) + beta)) / a0;
        b1 = (-2.0f * A * ((A - 1.0f) + (A + 1.0f) * cos(w0))) / a0;
        b2 = (A * ((A + 1.0f) + (A - 1.0f) * cos(w0) - beta)) / a0;
        a1 = (-2.0f * ((A - 1.0f) - (A + 1.0f) * cos(w0))) / a0;
        a2 = ((A + 1.0f) - (A - 1.0f) * cos(w0) - beta) / a0;
    }

    float process(float in) {
        float out = b0 * in + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
        x2 = x1; x1 = in; y2 = y1; y1 = out;
        return out;
    }
};

class HearingAidEngine : public oboe::AudioStreamDataCallback {
public:
    void start() {
        // Защита от повторного запуска
        if (playStream || recordStream) return;

        // 1. Настраиваем ВЫХОДНОЙ поток (Воспроизведение в наушники)
        oboe::AudioStreamBuilder outBuilder;
        outBuilder.setDirection(oboe::Direction::Output)
                  ->setSharingMode(oboe::SharingMode::Shared)
                  ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
                  ->setFormat(oboe::AudioFormat::Float)
                  ->setChannelCount(oboe::ChannelCount::Mono)
                  ->setDataCallback(this);

        oboe::Result result = outBuilder.openStream(playStream);
        if (result != oboe::Result::OK) {
            LOGE("Не удалось открыть выходной поток: %s", oboe::convertToText(result));
            return;
        }

        int32_t sampleRate = playStream->getSampleRate();
        // Настройка фильтра: поднимаем частоты выше 3000 Гц на +12 Дб для четкости речи
        highSpeechFilter.configureHighShelf(sampleRate, 3000.0f, 12.0f);

        // 2. Настраиваем ВХОДНОЙ поток (Запись с микрофона)
        oboe::AudioStreamBuilder inBuilder;
        inBuilder.setDirection(oboe::Direction::Input)
                 ->setSharingMode(oboe::SharingMode::Shared)
                 ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
                 ->setFormat(oboe::AudioFormat::Float)
                 ->setChannelCount(oboe::ChannelCount::Mono)
                 ->setSampleRate(sampleRate); // Частоты должны строго совпадать

        result = inBuilder.openStream(recordStream);
        if (result != oboe::Result::OK) {
            LOGE("Не удалось открыть входной поток: %s", oboe::convertToText(result));
            playStream->close();
            playStream.reset();
            return;
        }

        // Запускаем оба потока одновременно
        recordStream->requestStart();
        playStream->requestStart();
        LOGD("Аудио-движок Full Duplex успешно запущен");
    }

    void stop() {
        if (playStream) {
            playStream->requestStop();
            playStream->close();
            playStream.reset();
        }
        if (recordStream) {
            recordStream->requestStop();
            recordStream->close();
            recordStream.reset();
        }
        LOGD("Аудио-движок остановлен");
    }

    // Этот метод вызывается системой Oboe каждый раз, когда наушники готовы принять порцию звука
    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames) override {
        float *floatData = static_cast<float *>(audioData);

        if (!recordStream) {
            std::fill_n(floatData, numFrames, 0.0f);
            return oboe::DataCallbackResult::Continue;
        }

        // Мгновенно читаем сырой звук из микрофона прямо в буфер воспроизведения (таймаут 0)
        auto result = recordStream->read(floatData, numFrames, 0);
        
        if (!result) {
            std::fill_n(floatData, numFrames, 0.0f);
            return oboe::DataCallbackResult::Continue;
        }

        int32_t framesRead = result.value();
        if (framesRead < numFrames) {
            std::fill(floatData + framesRead, floatData + numFrames, 0.0f);
        }

        // Алгоритм DSP обработки звука в реальном времени
        float masterGain = 2.0f; // Коэффициент общего усиления звука
        
        for (int i = 0; i < framesRead; ++i) {
            float sample = floatData[i];

            // Пропускаем через эквалайзер высоких частот (акцент на речь)
            sample = highSpeechFilter.process(sample);

            // Применяем громкость
            float processedSample = sample * masterGain;
            
            // Защитный лимитер: не дает звуку превысить безопасный порог amplitudes
            if (processedSample > 0.8f) processedSample = 0.8f;
            if (processedSample < -0.8f) processedSample = -0.8f;
            
            floatData[i] = processedSample;
        }

        return oboe::DataCallbackResult::Continue;
    }

private:
    std::shared_ptr<oboe::AudioStream> playStream;
    std::shared_ptr<oboe::AudioStream> recordStream;
    BiQuadFilter highSpeechFilter;
};

static HearingAidEngine engine;

// Правильная JNI-склейка функций под архитектуру нашего Kotlin-модуля
extern "C" {
    JNIEXPORT void JNICALL
    Java_com_hearingaid_HearingAidEngineModule_startEngine(JNIEnv *env, jobject thiz) {
        engine.start();
    }

    JNIEXPORT void JNICALL
    Java_com_hearingaid_HearingAidEngineModule_stopEngine(JNIEnv *env, jobject thiz) {
        engine.stop();
    }
}
