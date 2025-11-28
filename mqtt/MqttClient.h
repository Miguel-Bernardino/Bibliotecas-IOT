#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include <WiFi.h>
#include <PubSubClient.h>

// Estrutura para mapear Tópico e Função
typedef struct {
  const char* topic;
  void (*method)(String msg); 
} TopicHandler;

// Define o tipo de função de callback que a PubSubClient espera
typedef void (*CallbackFunction)(char*, byte*, unsigned int);

class MqttClient {
private:
  WiFiClient espClient;
  PubSubClient client;
  
  // Membros privados
  const char* mqtt_server;
  const int mqtt_port;
  const char* mqtt_clientId;
  
  TopicHandler* topics; 
  int numOfTopics; 

  // Funções privadas
  bool executeTopicMethod(const String& topic, const String& msg);
  
public:
  // Construtor
  MqttClient(
    const char* server, 
    int port, 
    const char* clientId, 
    TopicHandler topics[], 
    int numTopics
  );
  
  // 🆕 NOVO: Método para enviar mensagens para um tópico MQTT
  bool publishMessage(const char* topic, const char* payload);

  // 🆕 NOVO: Método para configurar a função de callback externamente
  void setCallbackMethod(CallbackFunction callback);

  // Configurações iniciais
  void begin();

  // 🆕 NOVO: Função pública que expõe a lógica de roteamento/callback
  void handleCallback(char* topic, byte* message, unsigned int length);
  
  void reconnect();
  void loop(); 



};

#endif