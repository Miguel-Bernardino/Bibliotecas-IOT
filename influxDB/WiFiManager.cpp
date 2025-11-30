#include "WiFiManager.h"
#include <Arduino.h> // Necessário para Serial, delay e millis()

WiFiManager::WiFiManager() { 

}

bool WiFiManager::isConnected() {
    // Atualiza e retorna o estado real do Wi-Fi
    isConnectedToTheNetwork = (WiFi.status() == WL_CONNECTED);
    return isConnectedToTheNetwork;
}

int WiFiManager::scanLocalNetworks() {
    Serial.println("Starting network scan...");
    
    if (isConnected()) {
        WiFi.disconnect(); 
        isConnectedToTheNetwork = false;
    }
    
    // Inicia a varredura
    numSsid = WiFi.scanNetworks();

    Serial.print("Lista de SSID: ");
    Serial.println(numSsid);
    
    for (int thisNet = 0; thisNet < numSsid; thisNet++) {
        Serial.print(thisNet);
        Serial.print(") Rede: ");
        Serial.print(WiFi.SSID(thisNet));
        Serial.print(" | RSSI: ");
        Serial.println(WiFi.RSSI(thisNet));
    }

    return numSsid;
}

int WiFiManager::findSsidIndex(const char* targetSsid) {
    if (numSsid <= 0) {
        Serial.println("Warning: Scan networks first or numSsid is 0.");
        return -1;
    }

    for (int i = 0; i < numSsid; i++) {
        // Usa strcmp para comparar const char* com a string interna do WiFi.SSID(i)
        if (strcmp(targetSsid, WiFi.SSID(i).c_str()) == 0) {
            return i;
        }
    }

    return -1;
}

// 🔑 Conexão com Senha Opcional e Timeout
bool WiFiManager::connectToTheNetwork(const char* ssid, const char* password) {
    if (isConnected()) {
        WiFi.disconnect();
    }
    
    Serial.print("Attempting to connect to: ");
    Serial.println(ssid);

    // ✅ MELHORIA: Lógica da senha opcional
    if (password != NULL && strlen(password) > 0) {
        WiFi.begin(ssid, password); 
        Serial.println("Using password.");
    } else {
        WiFi.begin(ssid); 
        Serial.println("Assuming Open Network.");
    }

    // --- Checagem de Conexão com TIMEOUT (Não Bloqueia Indefinidamente) ---
    unsigned long startTime = millis();

    Serial.print("Conectando...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        
        // Verifica o Timeout
        if (millis() - startTime > CONNECTION_TIMEOUT_MS) {
            Serial.println("\n❌ Connection Failed (Timeout).");
            WiFi.disconnect(true);
            isConnectedToTheNetwork = false;
            return false;
        }
    }

    Serial.println("\n✅ Sucesso");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    isConnectedToTheNetwork = true;
    return true;
}

// Conecta a Rede Por Meio Do Indice (usa a função principal)
bool WiFiManager::connectToTheNetworkBySSIDIndex(int ssidIndex, const char* password) {
    if (ssidIndex < 0 || ssidIndex >= numSsid) {
        Serial.println("Error: Invalid SSID index or no scan performed.");
        return false;
    }
    
    const char* ssid = WiFi.SSID(ssidIndex).c_str();
    
    // Reutiliza a função principal que já tem o tratamento de timeout e senha
    return connectToTheNetwork(ssid, password);
}