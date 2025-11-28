#ifndef CALLMEBOT_H
#define CALLMEBOT_H

#include <Arduino.h>
#include <HTTPClient.h> 
#include "WiFiManager.h" // ⬅️ AGORA INCLUI SUA CLASSE NETWORK!

class CallMeBot {
private:
    // Credenciais
    String phoneNumber;
    String apiKey;
    
    // Referência para o objeto Network. Usaremos esta referência para checar a conexão.
    WiFiManager& networkManager; // ⬅️ Referência obrigatória

    // URL base da API
    const char* CALLMEBOT_API_URL = "https://api.callmebot.com/whatsapp.php"; 

    // Utilitário para formatar o texto para a URL
    String urlEncode(const String& message);

    // Função que executa a requisição GET
    int executeHttpGet(const String& url);

public:
    // ⚠️ Construtor agora recebe o objeto Network por referência
    CallMeBot(const String& phone, const String& key, WiFiManager& manager);

    // 📩 FUNÇÃO PRINCIPAL: Envia a mensagem de WhatsApp
    bool sendWhatsAppMessage(const String& message);
};

#endif