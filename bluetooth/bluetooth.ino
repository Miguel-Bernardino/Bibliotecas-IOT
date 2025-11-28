#include "BluetoothManager.h"
#include <BLEDevice.h>
#include <vector>
#include <string>

// --- Definição das Características Personalizadas ---
// A chave (UUID) deve ser mantida como std::string aqui
std::vector<CharacteristicConfig> configs = {
    // UUID "1001": Status do LED (WRITE/READ/NOTIFY)
    {"1001", BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE, "LED: OFF"},
    // UUID "1002": Status de Temperatura (READ/NOTIFY)
    {"1002", BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY, "Temp: 0.0 C"},
    // UUID "1003": Contador Simples (READ)
    {"1003", BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY, "Contador: 0"} 
};

// Instancia um objeto da nossa classe BLE
BLEManager bleManager("ESP32-S3_BLE_Service", configs);

void setup() {
  Serial.begin(115200);
  // Inicialização padrão do LED
  neopixelWrite(RGB_BUILTIN, 0,0,0); 
  
  // Inicia o serviço BLE
  bleManager.begin(); 
}

void loop() {
  // --- 1. Lógica de Recepção (Comando Genérico) ---
  ResultadoComando comando = bleManager.lerMensagem();
  
  if (comando.mensagem.length() > 0) {
    String mensagemUpper = comando.mensagem;
    mensagemUpper.toUpperCase();

    Serial.println(comando.uuid);
    // Processamento do comando baseado no CANAL (UUID) de onde veio o comando
    if (comando.uuid == String(CHARACTERISTIC_RX_UUID) || comando.uuid == "00001001-0000-1000-8000-00805f9b34fb") {
      
      if (mensagemUpper == "LIGAR") {
        neopixelWrite(RGB_BUILTIN, 10, 10, 10);
        bleManager.update("1001", "LED: LIGADO!");
        
      } else if (mensagemUpper == "DESLIGAR") {
        neopixelWrite(RGB_BUILTIN, 0,0,0);
        bleManager.update("1001", "LED: DESLIGADO.");
        
      } else {
        // CORREÇÃO: Converte a concatenação de String para C-string (.c_str()) para garantir a compatibilidade com std::string na função update.
        String feedbackMsg = "Comando '" + comando.mensagem + "' desconhecido.";
        bleManager.update("1001", feedbackMsg.c_str());
      }
    }
  }

  // --- 2. Lógica de Atualização Periódica dos Status (Sensores/Contadores) ---
  static unsigned long ultimoUpdate = 0;
  static long contador = 0;

  if (millis() - ultimoUpdate > 5000) { // Atualiza a cada 5 segundos
    
    // A) Atualiza Temperatura (UUID "1002" - READ | NOTIFY)
    float temperatura = random(200, 350) / 10.0;
    // CORREÇÃO: Converte a concatenação de String para C-string para a chamada update().
    String tempMsg = "Temp: " + String(temperatura, 1) + " C"; 
    bleManager.update("1002", tempMsg.c_str()); 

    // B) Atualiza Contador (UUID "1003" - Somente READ)
    contador++;
    // CORREÇÃO: Converte a concatenação de String para C-string para a chamada update().
    String countMsg = "Contador: " + String(contador);
    bleManager.update("1003", countMsg.c_str()); 

    ultimoUpdate = millis();
  }
  
  delay(10); 
}