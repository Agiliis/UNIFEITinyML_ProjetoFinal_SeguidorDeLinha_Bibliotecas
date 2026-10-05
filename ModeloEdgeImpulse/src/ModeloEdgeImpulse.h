#ifndef MODELO_EDGE_IMPULSE_H
#define MODELO_EDGE_IMPULSE_H

#include <Arduino.h>

class ModeloEdgeImpulse {
public:
    static constexpr float OUTPUT_SCALE = 160.0f;
    static constexpr float OUTPUT_OFFSET = -60.0f;
    static constexpr float PWM_FACTOR = 1.0f;

    bool inferirSensores(const int *sensorPins,
                         uint8_t sensorCount,
                         int &pwmLeft,
                         int &pwmRight,
                         Stream *telemetry = nullptr);

    bool inferir(const float *features,
                 int &pwmLeft,
                 int &pwmRight,
                 Stream *telemetry = nullptr);

private:
    uint32_t lastTelemetryMs = 0;

    void printTelemetry(Stream *telemetry,
                        const float *features,
                        float leftOutput,
                        float rightOutput,
                        int pwmLeft,
                        int pwmRight);
};

#endif
