#include "ColetaDados.h"

ColetaDados::ColetaDados(const char *filePath)
    : filePath(filePath),
      active(false),
      full(false),
      sampleCountValue(0),
      runIdValue(0),
      runStartMs(0),
      lastSampleMs(0) {
}

void ColetaDados::beginRun() {
    if (active) return;

    runIdValue++;
    sampleCountValue = 0;
    runStartMs = millis();
    lastSampleMs = runStartMs - SAMPLE_PERIOD_MS;
    full = false;
    active = true;
}

bool ColetaDados::addSample(const int16_t *sensors,
                            uint32_t timeMs,
                            int16_t error,
                            int16_t pwmLeft,
                            int16_t pwmRight) {
    if (!active || full) return false;
    if (timeMs - lastSampleMs < SAMPLE_PERIOD_MS) return false;

    lastSampleMs = timeMs;
    if (sampleCountValue >= MAX_SAMPLES) {
        full = true;
        return false;
    }

    Sample &sample = samples[sampleCountValue];
    sample.timeMs = timeMs - runStartMs;
    sample.error = error;
    sample.pwmLeft = pwmLeft;
    sample.pwmRight = pwmRight;

    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        sample.sensors[i] = sensors[i];
    }

    sampleCountValue++;
    return true;
}

bool ColetaDados::finishRun() {
    if (!active && sampleCountValue == 0) return true;

    bool saved = saveToFlash();
    if (saved) {
        active = false;
        full = false;
        sampleCountValue = 0;
    }
    return saved;
}

bool ColetaDados::saveToFlash() {
    if (sampleCountValue == 0) {
        active = false;
        return true;
    }

    File file = LittleFS.open(filePath, FILE_APPEND);
    if (!file) return false;

    if (file.size() == 0) {
        file.println("corrida,t_ms,s0,s1,s2,s3,s4,s5,erro,pwm_esquerdo,pwm_direito");
    }

    for (uint32_t i = 0; i < sampleCountValue; i++) {
        const Sample &sample = samples[i];
        file.printf("%lu,%lu", (unsigned long)runIdValue,
                    (unsigned long)sample.timeMs);
        for (uint8_t sensor = 0; sensor < SENSOR_COUNT; sensor++) {
            file.printf(",%d", sample.sensors[sensor]);
        }
        file.printf(",%d,%d,%d\n", sample.error,
                    sample.pwmLeft, sample.pwmRight);
    }

    file.close();
    return true;
}

bool ColetaDados::isActive() const {
    return active;
}

bool ColetaDados::isFull() const {
    return full;
}

uint32_t ColetaDados::sampleCount() const {
    return sampleCountValue;
}

uint32_t ColetaDados::runId() const {
    return runIdValue;
}

bool ColetaDados::dump(Stream &output) const {
    File file = LittleFS.open(filePath, FILE_READ);
    if (!file) return false;

    output.println("#INICIO_DUMP");
    uint8_t buffer[128];
    while (file.available()) {
        size_t bytesRead = file.read(buffer, sizeof(buffer));
        if (bytesRead == 0) break;
        output.write(buffer, bytesRead);
    }
    file.close();
    output.println("#FIM_DUMP");
    return true;
}

bool ColetaDados::clearFile() {
    return LittleFS.remove(filePath);
}

void ColetaDados::printSpaceInfo(Stream &output) const {
    int totalBytes = LittleFS.totalBytes();
    int usedBytes = LittleFS.usedBytes();

    output.print("Espaco total: ");
    output.println(totalBytes);
    output.print("Espaco usado: ");
    output.println(usedBytes);
    output.print("Espaco restante: ");
    output.println(totalBytes - usedBytes);
}
