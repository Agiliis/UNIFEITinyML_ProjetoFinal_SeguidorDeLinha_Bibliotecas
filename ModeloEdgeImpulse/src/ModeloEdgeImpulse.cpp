#include "ModeloEdgeImpulse.h"
#ifndef EI_MODEL_FLOAT32
#define EI_MODEL_FLOAT32 0
#endif

#if EI_MODEL_FLOAT32
#include <SeguidorDeLinha_IA_float32_2.h>
#else
#include <SeguidorDeLinha_IA_int8_2.h>
#endif

bool ModeloEdgeImpulse::inferirSensores(const int *sensorPins,
                                        uint8_t sensorCount,
                                        int &pwmLeft,
                                        int &pwmRight,
                                        Stream *telemetry) {
    float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = { 0.0f };
    uint8_t count = min(sensorCount, (uint8_t)EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);

    for (uint8_t i = 0; i < count; i++) {
        features[i] = (float)analogRead(sensorPins[i]);
    }

    return inferir(features, pwmLeft, pwmRight, telemetry);
}

bool ModeloEdgeImpulse::inferir(const float *features,
                                int &pwmLeft,
                                int &pwmRight,
                                Stream *telemetry) {
    pwmLeft = 0;
    pwmRight = 0;

    signal_t signal;
    int signalError = numpy::signal_from_buffer(
        features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
    if (signalError != 0) {
        if (telemetry) telemetry->printf("IA: falha ao criar sinal (%d)\n", signalError);
        return false;
    }

    ei_impulse_result_t result = { 0 };
    EI_IMPULSE_ERROR inferenceError = run_classifier(&signal, &result, false);
    if (inferenceError != EI_IMPULSE_OK) {
        if (telemetry) telemetry->printf("IA: falha na inferencia (%d)\n", inferenceError);
        return false;
    }

    float left = (result.classification[0].value * OUTPUT_SCALE + OUTPUT_OFFSET)
               * PWM_FACTOR;
    float right = (result.classification[1].value * OUTPUT_SCALE + OUTPUT_OFFSET)
                * PWM_FACTOR;

    pwmLeft = constrain((int)lroundf(left), -100, 100);
    pwmRight = constrain((int)lroundf(right), -100, 100);
    printTelemetry(telemetry, features,
                   result.classification[0].value,
                   result.classification[1].value,
                   pwmLeft, pwmRight);
    return true;
}

void ModeloEdgeImpulse::printTelemetry(Stream *telemetry,
                                       const float *features,
                                       float leftOutput,
                                       float rightOutput,
                                       int pwmLeft,
                                       int pwmRight) {
    if (!telemetry) return;

    uint32_t now = millis();
    if (now - lastTelemetryMs < 500) return;

    telemetry->printf("IA: sensores=");
    for (uint8_t i = 0; i < EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME; i++) {
        if (i > 0) telemetry->print(',');
        telemetry->print((int)features[i]);
    }
    telemetry->printf(" saidas=%.4f,%.4f pwm=%d,%d\n",
                      leftOutput,
                      rightOutput,
                      pwmLeft,
                      pwmRight);
    lastTelemetryMs = now;
}
