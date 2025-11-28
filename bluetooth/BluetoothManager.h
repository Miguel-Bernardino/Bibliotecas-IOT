#include <map>
#include <vector>
#include <string> 
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <WString.h> // Para a classe String do Arduino

// --- Definições de UUIDs Base ---
#define SERVICE_UUID           "4fafc201-1fb5-459e-8ac2-c423c7c8fe01"
#define CHARACTERISTIC_RX_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8" // Canal principal de Comandos (WRITE)
#define CHARACTERISTIC_TX_UUID "d0121101-1402-4211-9213-3c224b73b224" // Canal principal de Notificações (NOTIFY)

// --- Estrutura para Configuração de Características Personalizadas ---
struct CharacteristicConfig {
  std::string uuid;
  uint32_t properties; // Ex: BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  std::string initialValue;
};

// --- Estruturas para Recebimento de Comandos ---
struct ComandoRecebido {
    String uuid; // String do Arduino para fácil uso na loop()
    String mensagem;
    bool recebido = false; 

    // NOVO: Construtor para permitir ComandoRecebido{"", "", false} (const char* -> String)
    ComandoRecebido(const char* u, const char* m, bool r) : uuid(u), mensagem(m), recebido(r) {}
    
    // Construtor padrão é necessário já que um construtor customizado foi definido
    ComandoRecebido() = default; 
};

struct ResultadoComando {
    String uuid = "";
    String mensagem = "";

    // NOVO: Construtor para permitir ResultadoComando{"", ""} (const char* -> String)
    ResultadoComando(const char* u, const char* m) : uuid(u), mensagem(m) {}
    
    // Construtor padrão é necessário já que um construtor customizado foi definido
    ResultadoComando() = default; 
};

// Variável estática (Global) para armazenar o comando mais recente
static ComandoRecebido ultimoComando; 

// Declaração forward para evitar erros de referência circular
class BLEManager; 

// Classe para lidar com eventos de escrita (Recepção de Dados)
class BLECallbacks : public BLECharacteristicCallbacks {
private:
    BLEManager* pManager; 
public:
    BLECallbacks(BLEManager* manager) : pManager(manager) {} 

    void onWrite(BLECharacteristic *pCharacteristic) {
        // CORREÇÃO 1: Converte o std::string retornado para String do Arduino.
        String charUuidArduino = String(pCharacteristic->getUUID().toString().c_str());
        
        // Pega o dado bruto
        uint8_t *dataPtr = pCharacteristic->getData();
        size_t dataLen = pCharacteristic->getLength();

        if (dataLen > 0) {
            // Cria std::string a partir dos bytes brutos
            std::string rxValue(reinterpret_cast<const char*>(dataPtr), dataLen);
            
            // Armazenamento genérico na variável estática global
            ultimoComando.uuid = charUuidArduino;
            // Converte a std::string para String do Arduino antes de armazenar no struct
            ultimoComando.mensagem = String(rxValue.c_str()); 
            ultimoComando.recebido = true;
            
            Serial.print("Comando recebido em [");
            Serial.print(ultimoComando.uuid);
            Serial.print("]: ");
            Serial.println(ultimoComando.mensagem);
        }
    }
};

// --- ESTRUTURA PRINCIPAL: BLEManager ---
class BLEManager {
private:
  const char* nomeDispositivo;
  BLECharacteristic *pRxCharacteristic;
  BLECharacteristic *pTxCharacteristic; 
  BLEServer *pServer;
  BLEService *pService; 

  // Mapa para armazenar e acessar todas as características criadas dinamicamente
  std::map<std::string, BLECharacteristic*> customCharacteristics; 
  
  std::vector<CharacteristicConfig> extraConfigs;

  // CORREÇÃO 4 (Parte 1): Mapa para armazenar as propriedades e contornar o erro 'getProperties' privado.
  std::map<std::string, uint32_t> characteristicProperties; 

public:
  // --- CONSTRUTOR ---
  BLEManager(const char* name, std::vector<CharacteristicConfig> configs) 
    : nomeDispositivo(name), extraConfigs(configs) {
      // CORREÇÃO 2 & NOVO: Construtor explícito para ComandoRecebido.
      ultimoComando = ComandoRecebido{"", "", false}; 
    }

  // Função auxiliar para associar o callback de escrita, passando a referência do Manager
  void associarCallbackDeEscrita(BLECharacteristic* pChar) {
      pChar->setCallbacks(new BLECallbacks(this));
  }

  // --- INICIAR SERVIÇOS BLE ---
  void begin() {
    Serial.println("Iniciando Servidor BLE...");
    
    // 1. Inicializa e Cria o Servidor
    BLEDevice::init(nomeDispositivo);
    pServer = BLEDevice::createServer();
    pService = pServer->createService(SERVICE_UUID); 
    
    // 2. Cria Característica RX (Comandos/Escrita) - Canal Padrão
    pRxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_RX_UUID,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR | BLECharacteristic::PROPERTY_READ
    );
    associarCallbackDeEscrita(pRxCharacteristic); 
    customCharacteristics[CHARACTERISTIC_RX_UUID] = pRxCharacteristic;
    // CORREÇÃO 4 (Parte 2): Armazena a propriedade da característica RX
    characteristicProperties[CHARACTERISTIC_RX_UUID] = BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR | BLECharacteristic::PROPERTY_READ;

    // 3. Cria Característica TX (Notificações Genéricas) - Canal Padrão
    pTxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_TX_UUID,
        BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ 
    );
    pTxCharacteristic->addDescriptor(new BLE2902());
    customCharacteristics[CHARACTERISTIC_TX_UUID] = pTxCharacteristic;
    // CORREÇÃO 4 (Parte 2): Armazena a propriedade da característica TX
    characteristicProperties[CHARACTERISTIC_TX_UUID] = BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ;
    
    // 4. Cria Múltiplas Características a partir da configuração (CUSTOMIZADAS)
    for (const auto& config : extraConfigs) {
      // createCharacteristic espera const char*, então usamos .c_str()
      BLECharacteristic *pChar = pService->createCharacteristic(
          config.uuid.c_str(), 
          config.properties 
      );
      
      // Se a característica tiver NOTIFY ou INDICATE, adiciona o descritor 2902
      if (config.properties & (BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE)) {
        pChar->addDescriptor(new BLE2902());
      }

      // Se a característica customizada tiver a propriedade WRITE, associa o callback genérico
      if (config.properties & (BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR)) {
        associarCallbackDeEscrita(pChar);
      }
      
      // CORREÇÃO 3 (Inicial): Remove a constância do ponteiro para setValue, usando const_cast/reinterpret_cast.
      pChar->setValue(const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(config.initialValue.c_str())), config.initialValue.length());
      
      // Armazena no mapa de customizadas
      customCharacteristics[config.uuid] = pChar;
      // CORREÇÃO 4 (Parte 2): Armazena a propriedade da característica customizada
      characteristicProperties[config.uuid] = config.properties;
      
      Serial.print("Característica Customizada criada: ");
      Serial.println(config.uuid.c_str());
    }
    
    // 5. Inicia o Serviço e a Publicidade
    pService->start();
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    BLEDevice::startAdvertising();
    Serial.println("BLE Publicitando...");
  }

  // --- LÓGICA DE ENVIO E ATUALIZAÇÃO ---

  // Método genérico para atualizar o valor de QUALQUER Característica por UUID e notificar (se permitido)
  void update(std::string uuid, std::string valor) {
    // Tenta encontrar a Característica no mapa
    if (customCharacteristics.count(uuid)) {
        BLECharacteristic* pChar = customCharacteristics.at(uuid);
        
        // CORREÇÃO 3: Remove a constância do ponteiro para setValue, usando const_cast/reinterpret_cast.
        pChar->setValue(const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(valor.c_str())), valor.length());
        
        // CORREÇÃO 4 (Parte 3): Usa o mapa de propriedades para verificar a permissão de NOTIFY, contornando o erro de acesso privado a getProperties().
        if (characteristicProperties.count(uuid) && (characteristicProperties.at(uuid) & BLECharacteristic::PROPERTY_NOTIFY) && pServer->getConnectedCount() > 0) {
            pChar->notify();
            Serial.print("Notificado [");
            Serial.print(uuid.c_str());
            Serial.print("]: ");
            Serial.println(valor.c_str());
        }
    } else {
        Serial.print("Erro: Característica UUID ");
        Serial.print(uuid.c_str());
        Serial.println(" não encontrada.");
    }
  }

  // --- LÓGICA DE RECEPÇÃO ---

  // Coleta o comando mais recente recebido em QUALQUER Característica de Escrita
  ResultadoComando lerMensagem() {
    if (ultimoComando.recebido) {
        ResultadoComando resultado;
        resultado.uuid = ultimoComando.uuid;
        resultado.mensagem = ultimoComando.mensagem;
        
        // Limpa a flag para o próximo comando
        ultimoComando.recebido = false;
        
        return resultado;
    }
    // CORREÇÃO 5 & NOVO: Construtor explícito para ResultadoComando.
    return ResultadoComando{"", ""}; 
  }

  // --- ACESSO DIRETO (AVANÇADO) ---
  BLECharacteristic* getCharacteristic(std::string uuid) {
    if (customCharacteristics.count(uuid)) {
      return customCharacteristics.at(uuid);
    }
    return nullptr;
  }
};