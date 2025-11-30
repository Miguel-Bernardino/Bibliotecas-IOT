#include "WiFiManager.h"
#include "InfluxDBManager.h"
#include <Arduino.h> 
#include <InfluxDbClient.h> // Adiciona include aqui também por segurança.

// --- CONFIGURAÇÕES DE REDE ---
// ⚠️ ATENÇÃO: Preencha com suas credenciais de WiFi
const char* WIFI_SSID = "SUA_REDE_WIFI";
const char* WIFI_PASS = "SUA_SENHA_WIFI";

// --- CONFIGURAÇÕES DO INFLUXDB (Preencha com suas credenciais) ---
// Ex: "https://us-east-1-1.aws.cloud2.influxdata.com"
const char* INFLUXDB_URL = "SEU_URL_DO_INFLUXDB"; 
const char* INFLUXDB_ORG = "SUA_ORG_DO_INFLUXDB";
const char* INFLUXDB_BUCKET = "SEU_BUCKET_DO_INFLUXDB";
// Token gerado pelo InfluxDB com permissão de escrita e leitura
const char* INFLUXDB_TOKEN = "SEU_TOKEN_DO_INFLUXDB"; 
const char* INFLUXDB_MEASUREMENT = "dados_esp32";

// --- INSTÂNCIAS DAS CLASSES ---
WiFiManager wifiManager;
// Alocação dinâmica, inicializado no setup()
InfluxDBManager* dbManager; 

// Variáveis de Exemplo (simulando leituras de sensores)
float temperatura = 25.5;
int umidade = 60;
int contador = 0;

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n--- INÍCIO DO PROJETO INFLUXDB/ESP32 ---");

    // 1. Conexão WiFi (Usando o gerenciador de WiFi)
    if (!wifiManager.connectToTheNetwork(WIFI_SSID, WIFI_PASS)) {
        Serial.println("🛑 Falha Crítica: Não foi possível conectar ao WiFi. Parando.");
        // Não retorna para permitir depuração, mas o loop pode falhar se depender da conexão
    }

    // 2. Inicializa o InfluxDBManager (passando o endereço do WiFiManager)
    dbManager = new InfluxDBManager(
        &wifiManager, 
        INFLUXDB_URL, 
        INFLUXDB_ORG, 
        INFLUXDB_BUCKET, 
        INFLUXDB_TOKEN, 
        INFLUXDB_MEASUREMENT
    );

    // 3. Valida a conexão com o InfluxDB
    if (wifiManager.isConnected() && !dbManager->setupInfluxDB()) {
        Serial.println("🛑 Falha Crítica: Não foi possível conectar ao InfluxDB. Verifique as credenciais.");
        // A execução continua, mas a escrita de dados irá falhar
    }

    Serial.println("✅ Inicialização completa. Iniciando loop de medições.");
}

void loop() {
    // Simula a leitura de sensores
    contador++;
    temperatura += sin(contador / 10.0) * 0.5; // Variação senoidal simples
    umidade = 60 + (contador % 5); // Variação simples

    Serial.print("\n--- Medição Sequencial #");
    Serial.print(contador);
    Serial.print(" ---\n  Temp: ");
    Serial.print(temperatura);
    Serial.print("C, Umid: ");
    Serial.print(umidade);
    Serial.println("%");

    // Adiciona os dados ao Ponto de Medição
    dbManager->addFloatField("temperatura_c", temperatura);
    dbManager->addIntField("umidade_relativa", umidade);
    dbManager->addIntField("leitura_sequencial", contador);

    // Envia os dados
    if (dbManager->writeData()) {
        Serial.println("✅ Dados enviados com sucesso!");
    } else {
        Serial.println("❌ Falha ao enviar dados!");
    }

    // Limpa o ponto para a próxima medição
    dbManager->clearPoint();

    // Demonstração da consulta de dados a cada 5 leituras
    if (contador % 5 == 0) { 
        Serial.println("\n*** INICIANDO LEITURA DE DADOS (FLUX QUERY) ***");
        
        // Exemplo de Flux Query para buscar os últimos 10 minutos de dados
        String fluxQuery_recent = String("from(bucket: \"") + INFLUXDB_BUCKET + "\")" 
                        + " |> range(start: -10m)" 
                        + " |> filter(fn: (r) => r[\"_measurement\"] == \"" + INFLUXDB_MEASUREMENT + "\")";

        Serial.println("\n--- RESULTADO DA QUERY RECENTE ---");
        String jsonResult = dbManager->queryAndParseJSON(fluxQuery_recent);
        Serial.print("Resultado JSON completo: ");
        Serial.println(jsonResult);
        Serial.println("------------------------------------");
    }

    // Intervalo entre as medições
    delay(10000); // 10 segundos
}