# ColetaDados

Biblioteca para coletar amostras de sensores e comandos dos motores em RAM e gravar o bloco no LittleFS ao final da corrida.

## Uso

```cpp
#include <ColetaDados.h>

ColetaDados coleta;
int16_t sensores[ColetaDados::SENSOR_COUNT];

coleta.beginRun();
coleta.addSample(sensores, millis(), erro, pwmEsquerdo, pwmDireito);
coleta.finishRun();
```

A biblioteca controla o período de 20 ms, o buffer de 2000 amostras, o CSV `/dados.csv`, o dump serial e a limpeza do arquivo.

```cpp
coleta.dump(Serial);
coleta.printSpaceInfo(Serial);
coleta.clearFile();
```

A partição LittleFS deve ser montada pelo programa antes de usar a biblioteca:

```cpp
LittleFS.begin(true);
```
