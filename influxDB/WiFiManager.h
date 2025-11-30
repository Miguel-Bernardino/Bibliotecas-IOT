#ifndef WiFiManager_H
#define WiFiManager_H

#include <Arduino.h>
#include <WiFi.h> 
#include <string.h> 
#include <stddef.h> // ⬅️ NOVO: Adicione esta linha para definir NULL

class WiFiManager {
private: 
    // Variável para rastrear o estado da conexão.
    bool isConnectedToTheNetwork = false;
    // Variável para armazenar o número de redes escaneadas.
    int numSsid = 0; 
    
    // ⚙️ Constante para definir o tempo limite de conexão (20 segundos).
    const unsigned long CONNECTION_TIMEOUT_MS = 20000; 

public: 
    WiFiManager();

    // Retorna o status de conexão atual.
    bool isConnected();

    // 🔍 Inicia a varredura das redes locais e imprime a lista.
    int scanLocalNetworks(); 

    // 🔎 Encontra o índice de um SSID escaneado.
    // Usamos const char* para maior eficiência.
    int findSsidIndex(const char* targetSsid); 

    // 🔑 Tenta conectar usando o SSID e a senha (opcional). 
    // Adiciona timeout e trata senha NULL ou string vazia.
    bool connectToTheNetwork(const char* ssid, const char* password = NULL);

    // Conecta a Rede Por Meio Do Indice. Também recebe senha opcional.
    bool connectToTheNetworkBySSIDIndex(int ssidIndex, const char* password = NULL);
};

#endif