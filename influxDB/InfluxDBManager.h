#ifndef INFLUXDBMANAGER_H
#define INFLUXDBMANAGER_H

#include <Arduino.h>
#include <InfluxDbClient.h> // Biblioteca oficial do InfluxDB
#include <string.h>
#include "WiFiManager.h" // Inclui seu gerenciador de WiFi
#include <ArduinoJson.h>

class InfluxDBManager {
private: 
    WiFiManager* wifiManager;
    InfluxDBClient* client;
    Point* sensorPoint;

    // --- Configurações InfluxDB (Constantes) ---
    const char* INFLUXDB_URL;
    const char* INFLUXDB_ORG;
    const char* INFLUXDB_BUCKET;
    const char* INFLUXDB_TOKEN;
    const char* INFLUXDB_MEASUREMENT; 

public: 
    // Construtor
    InfluxDBManager(
        WiFiManager* manager,
        const char* url,
        const char* org,
        const char* bucket,
        const char* token,
        const char* measurement
    );
    // Destrutor
    ~InfluxDBManager();

    // Inicializa a conexão com o InfluxDB e checa o servidor.
    bool setupInfluxDB();

    // Adiciona um campo inteiro (int) ao ponto de medição.
    void addIntField(const char* name, int value);

    // Adiciona um campo float (ponto flutuante) ao ponto de medição.
    void addFloatField(const char* name, float value);

    // Adiciona uma tag (chave-valor para metadados) ao ponto de medição.
    void addTag(const char* name, const char* value);
    
    // Envia os dados acumulados no ponto de medição para o InfluxDB.
    bool writeData();

    // Limpa todos os campos e tags do ponto de medição atual para uma nova medição.
    void clearPoint();

    // Consulta o InfluxDB (Flux Query) e retorna os resultados em formato JSON (String).
    String queryAndParseJSON(const String& fluxQuery, size_t jsonCapacity = 5000);
};

#endif // INFLUXDBMANAGER_H