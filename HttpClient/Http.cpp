#include "Http.h" // Inclui o novo cabeçalho
#include <HTTPClient.h> 
#include <WiFiClient.h>

// O construtor recebe a referência do WiFiManager
Http::Http(WiFiManager& manager) : wifiManager(manager) { // Renomeado
    // Inicializa o WiFiManager na lista de inicialização
}

// 🔒 Função auxiliar para verificar a conexão e tentar reconectar
bool Http::ensureWifiConnected() { // Renomeado
    if (wifiManager.isConnected()) {
        return true;
    }
    
    Serial.println("Warning: Wi-Fi not connected. Attempting automatic reconnection...");
    
    return wifiManager.isConnected();
}

// Implementação do GET
Http::Response Http::get(const char* url, const String& body, const char* contentType) { // Renomeado
    Response result;
    result.httpCode = -1;
    result.payload = "";

    // 1. Garante que o Wi-Fi esteja ativo
    if (!ensureWifiConnected()) {
        Serial.println("❌ GET Failed: Wi-Fi is not connected.");
        return result;
    }

    HTTPClient http;
    
    // Inicia a conexão
    if (http.begin(url)) { 
        
        Serial.print("🌐 HTTP GET: ");
        Serial.println(url);

        int httpCode;
        
        // 2. Verifica se um corpo foi fornecido para usar sendRequest
        if (body.length() > 0) {
            Serial.print("⚠️ WARNING: Sending body with GET. Content-Type: ");
            Serial.println(contentType);
            
            // Adiciona o cabeçalho Content-Type
            http.addHeader("Content-Type", contentType);
            
            // Executa a requisição GET com o corpo usando sendRequest
            httpCode = http.sendRequest("GET", body);
        } else {
            // Se não houver corpo, executa o GET normal
            httpCode = http.GET();
        }

        // httpCode < 0 significa erro, httpCode > 0 significa sucesso (ex: 200, 404)
        if (httpCode > 0) {
            result.httpCode = httpCode;
            
            // Verifica o código de resposta HTTP
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY || httpCode == HTTP_CODE_CREATED) { 
                result.payload = http.getString();
                Serial.printf("✅ Success (Code: %d, Length: %d)\n", httpCode, result.payload.length());
            } else {
                Serial.printf("⚠️ GET Server Error (Code: %d)\n", httpCode);
                result.payload = http.getString(); 
            }
        } else {
            Serial.printf("❌ GET Failed: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end(); // Fecha a conexão
    } else {
        Serial.println("❌ HTTP Begin Failed.");
    }
    
    return result;
}

// Implementação do POST
Http::Response Http::post(const char* url, const String& body, const char* contentType) { // Renomeado
    Response result;
    result.httpCode = -1;
    result.payload = "";
    
    // 1. Garante que o Wi-Fi esteja ativo
    if (!ensureWifiConnected()) {
        Serial.println("❌ POST Failed: Wi-Fi is not connected.");
        return result;
    }

    HTTPClient http;
    
    // Inicia a conexão
    if (http.begin(url)) { // <--- Inicialização da URL aqui
        
        Serial.print("🌐 HTTP POST: ");
        Serial.println(url);
        
        // 2. Define o cabeçalho (Header) para o tipo de conteúdo
        http.addHeader("Content-Type", contentType);

        // 3. Executa o POST
        int httpCode = http.POST(body);
        
        if (httpCode > 0) {
            result.httpCode = httpCode;
            
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
                result.payload = http.getString();
                Serial.printf("✅ Success (Code: %d, Length: %d)\n", httpCode, result.payload.length());
            } else {
                Serial.printf("⚠️ POST Server Error (Code: %d)\n", httpCode);
                result.payload = http.getString();
            }
        } else {
            Serial.printf("❌ POST Failed: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end(); // Fecha a conexão
    } else {
        Serial.println("❌ HTTP Begin Failed.");
    }

    return result;
}