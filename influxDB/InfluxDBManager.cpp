#include "InfluxDBManager.h"
#include <Arduino.h>
#include <InfluxDbClient.h> 
#include <ArduinoJson.h> // Incluído aqui por boas práticas, embora já esteja no .h

// Construtor
InfluxDBManager::InfluxDBManager(
    WiFiManager* manager,
    const char* url,
    const char* org,
    const char* bucket,
    const char* token,
    const char* measurement
) : 
    wifiManager(manager),
    INFLUXDB_URL(url),
    INFLUXDB_ORG(org),
    INFLUXDB_BUCKET(bucket),
    INFLUXDB_TOKEN(token),
    INFLUXDB_MEASUREMENT(measurement)
{
    // Inicializa o cliente e o ponto de medição (alocação dinâmica).
    // O token é passado diretamente no construtor, eliminando a necessidade de setToken().
    client = new InfluxDBClient(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
    sensorPoint = new Point(INFLUXDB_MEASUREMENT);

    // Adiciona uma tag padrão
    sensorPoint->addTag("sensor_id", "esp32-001");
}

// Destrutor
InfluxDBManager::~InfluxDBManager() {
    delete client;
    delete sensorPoint;
}

// Inicialização e Checagem do InfluxDB
bool InfluxDBManager::setupInfluxDB() {
    // O WiFi deve ser conectado ANTES de chamar este método.
    if (!wifiManager->isConnected()) {
        Serial.println("❌ ERRO: O WiFi não está conectado. Conecte o WiFi primeiro.");
        return false;
    }
    
    // Checa a conexão com o servidor
    Serial.print("Checando a conexão com o InfluxDB em: ");
    Serial.println(INFLUXDB_URL);
    if (!client->validateConnection()) {
        Serial.print("❌ ERRO: A conexão com o InfluxDB falhou: ");
        Serial.println(client->getLastErrorMessage());
        return false;
    }

    Serial.println("✅ Conexão com o InfluxDB validada!");
    return true;
}

// Adiciona um campo inteiro
void InfluxDBManager::addIntField(const char* name, int value) {
    sensorPoint->addField(name, value);
}

// Adiciona um campo float
void InfluxDBManager::addFloatField(const char* name, float value) {
    sensorPoint->addField(name, value);
}

// Adiciona uma tag
void InfluxDBManager::addTag(const char* name, const char* value) {
    sensorPoint->addTag(name, value);
}

// Envia os dados para o InfluxDB
bool InfluxDBManager::writeData() {
    if (!wifiManager->isConnected()) {
        Serial.println("❌ ERRO: WiFi desconectado. Não é possível enviar dados.");
        return false;
    }

    // Escreve o ponto (Point)
    if (!client->writePoint(*sensorPoint)) {
        Serial.print("❌ ERRO ao escrever dados no InfluxDB: ");
        Serial.println(client->getLastErrorMessage());
        return false;
    }

    return true;
}

// Limpa o ponto para a próxima medição
void InfluxDBManager::clearPoint() {
    sensorPoint->clearFields();
    // As tags são mantidas, mas você pode usar sensorPoint->clearTags() se necessário
}

// Consulta o InfluxDB e retorna resultados em JSON
String InfluxDBManager::queryAndParseJSON(const String& fluxQuery, size_t jsonCapacity) {
    // 1. Checa o status do WiFi
    if (!wifiManager->isConnected()) {
        Serial.println("❌ ERRO: WiFi desconectado. Não é possível consultar dados.");
        return "{\"error\":\"WiFi desconectado\"}";
    }
    
    // 2. Executa a query
    Serial.print("Executando Flux Query: ");
    Serial.println(fluxQuery);

    FluxQueryResult result = client->query(fluxQuery);

    // 3. Verifica por erros iniciais
    if (!result.getError().isEmpty()) {
        Serial.print("❌ ERRO ao executar Flux Query: ");
        Serial.println(result.getError());
        return "{\"error\":\"" + result.getError() + "\"}";
    }

    // 4. Prepara o JSON (ArduinoJson)
    DynamicJsonDocument doc(jsonCapacity);
    JsonArray recordsArray = doc.to<JsonArray>();

    // 5. Itera sobre os registros (linhas) da tabela de resultados
    while (result.next()) {
        // CORREÇÃO: A linha abaixo foi removida.
        // Serial.println(result.getRow()); 
        
        JsonObject record = recordsArray.add<JsonObject>();
        
        // As chamadas getString() são as mais seguras para extrair dados brutos
        // do tipo timestamp (_time) e tags/fields de medição.
        record["tempo"]        = result.getValueByName("_time").getString();
        record["valor"]        = result.getValueByName("_value").getString();
        record["campo"]        = result.getValueByName("_field").getString(); 
        record["medicao"]      = result.getValueByName("_measurement").getString();
        
        // Exemplo: Tag sensor_id 
        String sensorIdValue = result.getValueByName("sensor_id").getString();
        if (!sensorIdValue.isEmpty()) { 
            record["sensor_id"] = sensorIdValue;
        }
    }

    // Verifica se houve erro após o último registro (pode ocorrer durante a leitura)
    if (!result.getError().isEmpty()) {
        Serial.print("⚠️ Aviso: Erro após o último registro: ");
        Serial.println(result.getError());
    }

    // 6. Serializa o JSON Array para uma String (retornando o resultado final)
    String jsonString;
    serializeJson(doc, jsonString);
    
    Serial.print("JSON gerado: ");
    Serial.println(jsonString);

    return jsonString;
}