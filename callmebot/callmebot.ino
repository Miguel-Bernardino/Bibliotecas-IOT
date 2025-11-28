// Exemplo no arquivo principal .ino

#include "Network.h"
#include "CallMeBot.h"

// ⚠️ SUAS CREDENCIAIS ⚠️
const char* ssid = "SEU_SSID_AQUI";
const char* password = "SUA_SENHA_AQUI";

const String MEU_TELEFONE = "SEU_NUMERO_AQUI"; // Exemplo: "5511999999999"
const String MINHA_API_KEY = "SUA_API_KEY_AQUI"; 

// 1. Cria o objeto Network
WiFiManager myNetwork;

// 2. Cria o objeto CallMeBot, passando a referência do objeto Network
CallMeBot bot(MEU_TELEFONE, MINHA_API_KEY, myNetwork);

void setup() {
    Serial.begin(115200);
    delay(100);

    // Conecta a rede usando o método da SUA classe Network
    if (myNetwork.connectToTheNetwork(WIFI_SSID, WIFI_PASS)) {
        Serial.println("\n✅ Rede conectada. Iniciando serviço CallMeBot.");
        
        // Tenta enviar a mensagem - a classe CallMeBot usará 'myNetwork' para checar o status
        if (bot.sendWhatsAppMessage("Alerta: Conexão bem sucedida via classe Network!")) {
            Serial.println("Finalizado com sucesso.");
        } else {
            Serial.println("Falha no processo de envio CallMeBot.");
        }
    } else {
        Serial.println("\n❌ Falha ao conectar ao WiFi. O CallMeBot não tentará enviar a mensagem.");
    }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    // Tenta enviar a mensagem apenas se estiver conectado
    bot.sendWhatsAppMessage("Alerta do meu ESP32 S3!");
    
    // Coloque um delay para evitar spam
    delay(10000); 
  } else {
    Serial.println("Aguardando conexão WiFi...");
    delay(5000);
  }
}