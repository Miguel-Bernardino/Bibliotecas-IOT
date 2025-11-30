#include "WiFiManager.h"
#include "Http.h" // Inclui o novo cabeçalho
#include <ArduinoJson.h> // Inclui a biblioteca ArduinoJson

// 1. Defina as credenciais da sua rede
const char* ssid = "POCO F5";
const char* password = "miguelFBI22";

// 2. Instancia o Gerenciador de Wi-Fi e o Cliente HTTP
WiFiManager wifiManager;
// O cliente HTTP precisa de uma referência ao gerenciador de Wi-Fi
Http httpClient(wifiManager); // Instancia a nova classe Http

void setup() {
    Serial.begin(115200);

    // 3. Conecta ao Wi-Fi usando o WiFiManager
    if (wifiManager.connectToTheNetwork(ssid, password)) {
        Serial.println("\nSetup: Wi-Fi conectado com sucesso!");
    } else {
        Serial.println("\nSetup: Falha ao conectar ao Wi-Fi. Verifique as credenciais.");
    }
}

void loop() {
    // A. Exemplo de requisição GET SIMPLES
    Serial.println("\n--- Executando GET Simples e Parseando JSON ---");
    Http::Response getSimpleResponse = httpClient.get("http://worldtimeapi.org/api/ip"); // Http::Response
    
    if (getSimpleResponse.httpCode == 200) {
        // ... (lógica de parsing JSON) ...
        const size_t capacity = JSON_OBJECT_SIZE(15) + 300; 
        DynamicJsonDocument doc(capacity);
        DeserializationError error = deserializeJson(doc, getSimpleResponse.payload);

        if (!error) {
            const char* dateTime = doc["datetime"]; 
            Serial.print("✅ Data e Hora extraída: ");
            Serial.println(dateTime);
        } else {
            Serial.print("❌ Falha ao parsear JSON GET Simples: ");
            Serial.println(error.f_str());
        }
    } else {
        Serial.printf("GET Simples Falhou. Código: %d\n", getSimpleResponse.httpCode);
    }
    
    delay(5000); // Espera 5 segundos

    // 🔴 B. Exemplo de requisição GET ENVIANDO JSON (NÃO-PADRÃO HTTP)
    Serial.println("\n--- Executando GET ENVIANDO JSON ---");
    
    // Usamos 'httpbin.org' para testar o envio de corpo em GET
    const char* getJsonUrl = "http://httpbin.org/get"; 
    
    // 1. Criação do JSON (Serialização)
    const size_t get_capacity = JSON_OBJECT_SIZE(2); 
    DynamicJsonDocument getDoc(get_capacity);
    getDoc["query"] = "status";
    getDoc["value"] = "pending";
    
    String getBodyJson;
    serializeJson(getDoc, getBodyJson);
    
    Serial.print("JSON para GET: ");
    Serial.println(getBodyJson);
    
    // 2. Chamada do GET passando o corpo JSON e o Content-Type: "application/json"
    Http::Response getJsonResponse = httpClient.get(getJsonUrl, getBodyJson, "application/json"); // Http::Response

    if (getJsonResponse.httpCode == 200) {
        Serial.println("GET c/ JSON Payload (Resposta do Servidor - httpbin):");
        Serial.println(getJsonResponse.payload);
        
        // Exemplo de como parsear a resposta do httpbin para ver seu JSON enviado:
        DynamicJsonDocument responseDoc(1024); // Novo buffer para a resposta
        DeserializationError error = deserializeJson(responseDoc, getJsonResponse.payload);
        
        if (!error) {
            Serial.print("✅ JSON enviado (extraído da resposta): ");
            Serial.println(responseDoc["data"].as<String>()); 
        }

    } else {
        Serial.printf("GET c/ JSON Falhou. Código: %d\n", getJsonResponse.httpCode);
    }

    delay(5000); 

    // C. Exemplo de requisição POST
    Serial.println("\n--- Executando POST com JSON Serializado ---");
    
    const char* postUrl = "http://httpbin.org/post"; 
    
    const size_t post_capacity = JSON_OBJECT_SIZE(3); 
    DynamicJsonDocument postDoc(post_capacity);

    postDoc["device"] = "ESP32-S3-JSON";
    postDoc["status"] = "online";
    postDoc["temperature"] = 26.1;
    
    String postBody;
    serializeJson(postDoc, postBody);
    
    Serial.print("JSON para POST: ");
    Serial.println(postBody);
    
    Http::Response postResponse = httpClient.post(postUrl, postBody);

    if (postResponse.httpCode == 200 || postResponse.httpCode == 201) {
        Serial.println("POST Payload (Response from Server):");
        Serial.println(postResponse.payload);
    } else {
        Serial.printf("POST Falhou. Código: %d\n", postResponse.httpCode);
    }

    delay(10000); 
}