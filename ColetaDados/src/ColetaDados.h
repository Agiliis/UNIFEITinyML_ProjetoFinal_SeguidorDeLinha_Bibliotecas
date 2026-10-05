#ifndef COLETA_DADOS_H
#define COLETA_DADOS_H

#include <Arduino.h>
#include <LittleFS.h>

class ColetaDados {
public:
    static constexpr uint8_t SENSOR_COUNT = 6;
    static constexpr uint32_t SAMPLE_PERIOD_MS = 20;
    static constexpr uint32_t MAX_SAMPLES = 2000;

    ColetaDados(const char *filePath = "/dados.csv");

    void beginRun();
    bool addSample(const int16_t *sensors,
                   uint32_t timeMs,
                   int16_t error,
                   int16_t pwmLeft,
                   int16_t pwmRight);
    bool finishRun();

    bool isActive() const;
    bool isFull() const;
    uint32_t sampleCount() const;
    uint32_t runId() const;

    bool dump(Stream &output) const;
    bool clearFile();
    void printSpaceInfo(Stream &output) const;

private:
    struct Sample {
        uint32_t timeMs;
        int16_t sensors[SENSOR_COUNT];
        int16_t error;
        int16_t pwmLeft;
        int16_t pwmRight;
    };

    const char *filePath;
    Sample samples[MAX_SAMPLES];
    bool active;
    bool full;
    uint32_t sampleCountValue;
    uint32_t runIdValue;
    uint32_t runStartMs;
    uint32_t lastSampleMs;

    bool saveToFlash();
};

#endif
