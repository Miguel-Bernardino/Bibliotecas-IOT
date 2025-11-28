#include "WiFiManager.h" // 🛠️ ALTERADO: De Network.h para WiFiManager.h
#include "MqttClient.h"

// 🌐 Configurações de Rede
const char* ssid = "SEU_SSID_AQUI";
const char* password = "SUA_SENHA_AQUI";

// ⚙️ Configurações do MQTT
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;
const char* mqtt_clientId = "MIOT-Esp32";

// --- Instância da Classe WiFiManager ---
WiFiManager* wifiManager; // 🛠️ ALTERADO: O tipo da variável é agora WiFiManager*

// ➡️ Definição das Funções de Tratamento
void testeTopic (String msg) {
  Serial.println("topico executado com sucesso");
  Serial.println("Mensagem Recebida: " + msg);
}

// 🗺️ Mapeamento de Tópicos
TopicHandler topicMap[] = {
  {"miot/teste", testeTopic},
  {"miot/arrow", [](String msg){
    Serial.println("arrow");
  }},
  // Adicione mais tópicos aqui, se necessário:
  // {"miot/comando", suaOutraFuncao},
};

const int numOfTopics = sizeof(topicMap) / sizeof(TopicHandler);

// 🚀 Instância da Biblioteca MQTT
// Lembre-se: O construtor do MqttClient assume que o Wi-Fi será conectado
MqttClient mqtt(
  mqtt_server, 
  mqtt_port, 
  mqtt_clientId, 
  topicMap, 
  numOfTopics
);

// 🆕 NOVO: Função Wrapper Global (no seu .ino)
// Esta função tem a assinatura correta esperada pela PubSubClient.
void mqttCallbackWrapper(char* topic, byte* message, unsigned int length) {
  // Redireciona a chamada para o método de instância na sua classe
  mqtt.handleCallback(topic, message, length);
}


void setup() {
  Serial.begin(115200);
  delay(100);

  // 📶 Conexão Wi-Fi USANDO A CLASSE WiFiManager
  Serial.println("Iniciando conexão Wi-Fi através do WiFiManager..."); // 🛠️ ALTERADO: Texto para refletir a nova classe
  WiFi.mode(WIFI_STA);
  wifiManager = new WiFiManager(); // 🛠️ ALTERADO: Instanciando WiFiManager

  bool connected = wifiManager->connectToTheNetwork(ssid, password);

  if (!connected) {
    
    // ❌ Lógica de Falha: Reinicia o ESP após uma falha de conexão Wi-Fi com timeout.
    Serial.println("❌ Erro: Falha fatal ao conectar ao Wi-Fi. Reiniciando em 5s...");
    delay(5000);
    ESP.restart(); 
    return;
  } else {

    Serial.println("✅ Wi-Fi conectado pelo WiFiManager."); // 🛠️ ALTERADO: Texto para refletir a nova classe
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    // ⚙️ O restante do setup (Configuração do MQTT)

    mqtt.begin(); 
    mqtt.setCallbackMethod(mqttCallbackWrapper);
  }
}

void loop() {
  
  if (!wifiManager->isConnected()) {
    // Se perder a conexão no loop, o `connectToTheNetwork` tentará reconectar,
    // mas não travará devido ao timeout da classe WiFiManager.
    wifiManager->connectToTheNetwork(ssid, password);
  }
  
  mqtt.loop(); 
  
// 🆕 NOVO: Exemplo de Envio de Mensagens
  static unsigned long lastMsg = 0;
  const long interval = 10000; // 10 segundos
  
  if (millis() - lastMsg > interval) {
    lastMsg = millis();
    // Converte millis() para uma string (payload)
    String payload = "Uptime: " + String(millis() / 1000) + "s";
    char payloadChar[30];
    payload.toCharArray(payloadChar, 30);
    
    // Envia a mensagem para o tópico "miot/status"
    mqtt.publishMessage("miot/status", payloadChar);
  }

  // Seu código principal aqui
  delay(10);
}