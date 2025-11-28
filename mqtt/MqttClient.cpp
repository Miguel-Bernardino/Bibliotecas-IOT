#include "MqttClient.h"
#include <Arduino.h> 

// Construtor: Inicializa membros
MqttClient::MqttClient(
  const char* server, 
  int port, 
  const char* clientId, 
  TopicHandler topics[], 
  int numTopics
) : client(espClient), // Inicializa o PubSubClient
    mqtt_server(server),      // Inicializa const char*
    mqtt_port(port),          // Inicializa const int
    mqtt_clientId(clientId)   // Inicializa const char*
{
    // O CORPO DO CONSTRUTOR FICA VAZIO para as variáveis const
    this->topics = topics;
    numOfTopics = numTopics;
}

// IMPLEMENTAÇÃO: Método para enviar mensagens
bool MqttClient::publishMessage(const char* topic, const char* payload) {
    if (!client.connected()) {
        Serial.println("Warning: Cannot publish, MQTT client not connected.");
        return false;
    }

    // Tenta publicar a mensagem (QoS 0)
    bool result = client.publish(topic, payload);

    if (result) {
        Serial.print("Published to: ");
        Serial.print(topic);
        Serial.print(" | Payload: ");
        Serial.println(payload);
    } else {
        Serial.print("❌ Failed to publish to: ");
        Serial.println(topic);
    }

    return result;
}

// 🆕 NOVO: Implementação do setCallbackMethod
void MqttClient::setCallbackMethod(CallbackFunction callback) {
    // Configura a função de callback do PubSubClient
    client.setCallback(callback);
    Serial.println("MQTT Callback method set.");
}

// Configuração inicial (chame no setup())
void MqttClient::begin() {
    // Configura o servidor e a porta
    client.setServer(mqtt_server, mqtt_port);
    Serial.println("MQTT Server configured.");
}

void MqttClient::loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); 
}

// Reconexão e Inscrição (sem mudanças)
void MqttClient::reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(mqtt_clientId)) { 
      Serial.println("connected");
      for(int i = 0; i < numOfTopics; i++) {
        client.subscribe(topics[i].topic);
        Serial.print("Subscribed to: ");
        Serial.println(topics[i].topic);
      }
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// Execução do método mapeado (sem mudanças)
bool MqttClient::executeTopicMethod(const String& topic, const String& msg) {
  for (int i = 0; i < numOfTopics; i++) {
    if (topic.equals(topics[i].topic)) {
      topics[i].method(msg);
      return true;
    }
  }
  return false;
}

// IMPLEMENTAÇÃO: Lógica que era do callback original
void MqttClient::handleCallback(char* topic, byte* message, unsigned int length) {
  String topicString = String(topic);
  String msg = "";

  // Converte payload (byte*) para String
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)message[i];
  }
  
  msg.trim();
  
  Serial.println("--- Mensagem Recebida via Handler ---");
  Serial.println("Tópico: " + topicString);
  
  // Roteia para o método privado de execução
  executeTopicMethod(topicString, msg);
}