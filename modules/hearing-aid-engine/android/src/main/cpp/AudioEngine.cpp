#include <oboe/Oboe.h>
#include <android/log.h>
#include <vector>
#include <cmath>

#define LOG_TAG "HearingAidEngine"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Структура простейшего биквадратного фильтра для обработки звука
struct BiQuadFilter {
    float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
    float x1 = 0, x2 = 0, y1 = 0, y2 = 0;

    // Расчет коэффициентов для фильтра высоких частот (усиление четкости речи)
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
        x2 = x1;
        x1 = in;
        y2 = y1;
        y1 = out;
        return out;
    }
};

class HearingAidEngine : public oboe::AudioStreamDataCallback {
public:
    void start() {
        oboe::AudioStreamBuilder builder;
        
        builder.setSharingMode(oboe::SharingMode::Exclusive)
               ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
               ->setFormat(oboe::AudioFormat::Float)
               ->setChannelCount(oboe::ChannelCount::Mono)
               ->setDataCallback(this);

        oboe::Result result = builder.openStream(stream);
        if (result == oboe::Result::OK) {
            sampleRate = stream->getSampleRate();
            
            // Настраиваем фильтр: частота 3000 Гц, усиливаем высокие частоты на +12 Дб для разборчивости речи
            highSpeechFilter.configureHighShelf(sampleRate, 3000.0f, 12.0f);

            stream->setBufferSizeInFrames(stream->getFramesPerBurst() * 2);
            stream->requestStart();
        } else {
            LOGE("Ошибка запуска Oboe: %s", oboe::convertToText(result));
        }
    }

    void stop() {
        if (stream) {
            stream->requestStop();
            stream->close();
            stream.reset();
        }
    }

    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames) override {
        float *floatData = static_cast<float *>(audioData);
        
        // Общее усиление (сделать погромче тихие звуки)
        float masterGain = 1.5f; 

        for (int i = 0; i < numFrames; ++i) {
            float sample = floatData[i];

            // 1. Пропускаем звук через фильтр высоких частот (выделяем согласные звуки)
            sample = highSpeechFilter.process(sample);

            // 2. Применяем общий уровень громкости
            float processedSample = sample * masterGain;
            
            // 3. Автоматическое ограничение (Лимитер), чтобы резкий грохот не бил по ушам
            if (processedSample > 0.8f) processedSample = 0.8f;
            if (processedSample < -0.8f) processedSample = -0.8f;
            
            floatData[i] = processedSample;
        }
        
        return oboe::DataCallbackResult::Continue;
    }

private:
    std::shared_ptr<oboe::AudioStream> stream;
    BiQuadFilter highSpeechFilter;
    int32_t sampleRate = 44100;
};

static HearingAidEngine engine;

extern "C" void startEngine() {
    engine.start();
}

extern "C" void stopEngine() {
    engine.stop();
}
