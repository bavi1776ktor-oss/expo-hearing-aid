#include <oboe/Oboe.h>
#include <android/log.h>
#include <cmath>

#define LOG_TAG "HearingAidEngine"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

class HearingAidEngine : public oboe::AudioStreamDataCallback {
public:
    void start() {
        oboe::AudioStreamBuilder builder;
        
        // Настройка параметров для достижения ультра-низкой задержки
        builder.setSharingMode(oboe::SharingMode::Exclusive)
               ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
               ->setFormat(oboe::AudioFormat::Float) // Работаем с Float для точности математики
               ->setChannelCount(oboe::ChannelCount::Mono)
               ->setDataCallback(this);

        oboe::Result result = builder.openStream(stream);
        if (result == oboe::Result::OK) {
            // Настройка оптимального размера буфера
            stream->setBufferSizeInFrames(stream->getFramesPerBurst() * 2);
            stream->requestStart();
        } else {
            LOGE("Не удалось открыть аудиопоток Oboe: %s", oboe::convertToText(result));
        }
    }

    void stop() {
        if (stream) {
            stream->requestStop();
            stream->close();
            stream.reset();
        }
    }

    // Метод вызывается системой в реальном времени при готовности аудио-буфера
    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames) override {
        float *floatData = static_cast<float *>(audioData);
        
        // Коэффициент усиления (Gain). 2.0f означает усиление звука в 2 раза.
        // В будущем сюда мы добавим разделение по частотам (эквалайзер).
        float volumeMultiplier = 2.0f; 

        for (int i = 0; i < numFrames; ++i) {
            // Усиливаем сигнал
            float processedSample = floatData[i] * volumeMultiplier;
            
            // Жесткое ограничение (Hard Limiting) для защиты слуха:
            // Звук не должен превышать безопасный порог амплитуды (-1.0 до 1.0)
            if (processedSample > 1.0f) processedSample = 1.0f;
            if (processedSample < -1.0f) processedSample = -1.0f;
            
            floatData[i] = processedSample;
        }
        
        return oboe::DataCallbackResult::Continue;
    }

private:
    std::shared_ptr<oboe::AudioStream> stream;
};

static HearingAidEngine engine;

// Функции-обертки для вызова из Kotlin
extern "C" void startEngine() {
    engine.start();
}

extern "C" void stopEngine() {
    engine.stop();
}
