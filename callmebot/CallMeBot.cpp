#include "CallMeBot.h"

// ----------------------------------------------------
// Construtor (Recebe a Referência da Rede)
// ----------------------------------------------------
// Inicializa a variável networkManager com a referência passada
CallMeBot::CallMeBot(const String& phone, const String& key, WiFiManager& manager) 
    : phoneNumber(phone), apiKey(key), networkManager(manager) {
}

// ----------------------------------------------------
// 🔑 Utilitário: Codificação de URL
// ----------------------------------------------------
String CallMeBot::urlEncode(const String& message) {
    String encoded = "";
    char c;
    char code0;
    char code1;
    
    for (int i = 0; i < message.length(); i++) {
        c = message.charAt(i);
        if (c == ' ') {
            encoded += '+'; 
        } else if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded += c;
        } else {
            // Lógica de codificação percentual (%XX)
            code1 = (c & 0xf) + '0';
            if ((c & 0xf) > 9) { code1 = (c & 0xf) - 10 + 'A'; }
            c = (c >> 4);
            code0 = (c & 0xf) + '0';
            if ((c & 0xf) > 9) { code0 = (c & 0xf) - 10 + 'A'; }
            encoded += '%';
            encoded += code0;
            encoded += code1;
        }
    }
    return encoded;
}

// ----------------------------------------------------
// 🌐 Execução do HTTP GET (VERIFICA A REDE PRIMEIRO)
// ----------------------------------------------------
int CallMeBot::executeHttpGet(const String& url) {
    // ⬅️ USA A SUA CLASSE NETWORK!
    if (!networkManager.isConnected()) {
        Serial.println("❌ Erro HTTP: Rede não conectada. Não é possível requisitar.");
        return 0; // Retorna 0 ou um código de erro personalizado para indicar falha na rede
    }
    
    HTTPClient http;
    
    Serial.print("Requisitando GET: ");
    Serial.println(url);

    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
        Serial.print("HTTP Code: ");
        Serial.println(httpResponseCode);
    } else {
        Serial.printf("❌ GET Falhou, erro: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    http.end();
    return httpResponseCode;
}

// ----------------------------------------------------
// 📞 FUNÇÃO PRINCIPAL: Envio da Mensagem
// ----------------------------------------------------
bool CallMeBot::sendWhatsAppMessage(const String& message) {
    // A checagem de conexão agora está dentro de executeHttpGet(), 
    // mas uma checagem rápida aqui pode economizar processamento.
    if (!networkManager.isConnected()) {
        Serial.println("❌ Não é possível enviar a mensagem: A rede não está ativa.");
        return false;
    }
    
    if (message.length() == 0) {
        Serial.println("Erro: Mensagem vazia.");
        return false;
    }
    
    // 1. Monta o URL completo 
    String url = String(CALLMEBOT_API_URL);
    url += "?phone=" + phoneNumber;
    url += "&apikey=" + apiKey;
    url += "&text=" + urlEncode(message); 
    
    // 2. Executa a requisição GET
    int httpResponseCode = executeHttpGet(url);

    // 3. Verifica o código de sucesso
    if (httpResponseCode == 200) {
        Serial.println("✅ Mensagem CallMeBot enviada com sucesso!");
        return true;
    } else {
        Serial.println("❌ Erro ao enviar mensagem! Código: " + String(httpResponseCode));
        return false;
    }
}