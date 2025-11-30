#ifndef HTTP_H
#define HTTP_H

#include <Arduino.h>
// Inclui o gerenciador de Wi-Fi fornecido
#include "WiFiManager.h" 
// Biblioteca padrão para requisições HTTP em ESP32
#include <HTTPClient.h> 

class Http { // Classe renomeada para Http
private:
    // Uma referência ao gerenciador de Wi-Fi é essencial
    WiFiManager& wifiManager;
    
public:
    // O construtor deve receber uma referência ao WiFiManager
    Http(WiFiManager& manager); // Construtor renomeado

    // Estrutura para o resultado da requisição HTTP
    struct Response {
        int httpCode;
        String payload;
    };

    // 🌐 Requisição GET genérica
    // Aceita body e contentType como parâmetros opcionais.
    Response get(const char* url, const String& body = "", const char* contentType = "application/x-www-form-urlencoded");

    // ✍️ Requisição POST genérica
    Response post(const char* url, const String& body, const char* contentType = "application/json");

private:
    // 🔒 Função auxiliar para garantir que a rede esteja conectada
    bool ensureWifiConnected();
};

#endif