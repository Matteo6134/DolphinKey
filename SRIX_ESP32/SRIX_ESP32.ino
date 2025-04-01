/*
 * SRIX_ESP32.ino - File principale per il lettore SRIX4K su ESP32-S3
 * 
 * Questo progetto implementa un lettore portatile per tag SRIX4K (ST25TB04K)
 * utilizzando un ESP32-S3 con display integrato e lettore NFC.
 * 
 * Hardware:
 * - ESP32-S3 (LILYGO T-Display-S3 o simile)
 * - Modulo NFC PN532
 * - Batteria LiPo interna
 * - Porta USB-C per ricarica
 * - Pulsanti direzionali
 * 
 * Autore: Manus AI
 * Data: Marzo 2025
 */

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "nfc_reader.h"
#include "srix.h"
#include "srix_flag.h"
#include "ui_manager.h"
#include "input_manager.h"
#include "navigation_controller.h"

// Pin definitions
#define PN532_IRQ   2
#define PN532_RESET 3
#define BUTTON_UP    4
#define BUTTON_DOWN  5
#define BUTTON_LEFT  6
#define BUTTON_RIGHT 7
#define BUTTON_SELECT 8
#define BATTERY_PIN 34  // Pin ADC per il monitoraggio della batteria
#define CHARGING_PIN 35  // Pin per rilevare lo stato di ricarica

// Global objects
TFT_eSPI tft;
NfcReader nfcReader(PN532_IRQ, PN532_RESET);
Srix srix(&nfcReader);
UiManager ui(&tft);
InputManager input(BUTTON_UP, BUTTON_DOWN, BUTTON_LEFT, BUTTON_RIGHT, BUTTON_SELECT);
NavigationController navigation(&ui, &srix);

// Timing variables
unsigned long lastTagCheck = 0;
unsigned long tagCheckInterval = 500; // Check for tag every 500ms
unsigned long lastBatteryCheck = 0;
unsigned long batteryCheckInterval = 30000; // Check battery every 30 seconds
bool lastTagState = false;
bool lowBatteryWarningShown = false;

// Callback per gli eventi dei pulsanti
void handleButtonEvent(Button button, ButtonEvent event) {
    navigation.handleButtonEvent(button, event);
}

// Funzione per leggere il livello della batteria
float readBatteryLevel() {
  // Leggi il valore analogico
  int rawValue = analogRead(BATTERY_PIN);
  
  // Converti in tensione (dipende dal partitore di tensione utilizzato)
  // Esempio con partitore 100K/100K e riferimento 3.3V
  float voltage = rawValue * 2 * 3.3 / 4095.0;
  
  return voltage;
}

// Funzione per ottenere la percentuale della batteria
int getBatteryPercentage() {
  float voltage = readBatteryLevel();
  
  // Mappa la tensione in percentuale (3.3V-4.2V → 0%-100%)
  int percentage = map(voltage * 100, 330, 420, 0, 100);
  
  // Limita il range
  percentage = constrain(percentage, 0, 100);
  
  return percentage;
}

// Funzione per controllare se il dispositivo è in carica
bool isCharging() {
  // Il pin è LOW quando la ricarica è in corso (dipende dal circuito TP4056)
  return !digitalRead(CHARGING_PIN);
}

// Funzione per controllare lo stato della batteria
void checkBatteryStatus() {
  if (millis() - lastBatteryCheck > batteryCheckInterval) {
    lastBatteryCheck = millis();
    
    int percentage = getBatteryPercentage();
    bool charging = isCharging();
    
    // Aggiorna l'indicatore nella UI
    ui.updateBatteryStatus(percentage, charging);
    
    // Avviso di batteria scarica
    if (percentage < 15 && !lowBatteryWarningShown && !charging) {
      lowBatteryWarningShown = true;
      
      // Mostra avviso
      ui.showLowBatteryWarning();
    } else if (percentage > 20 || charging) {
      lowBatteryWarningShown = false;
    }
    
    // Spegnimento di emergenza con batteria critica
    if (percentage < 5 && !charging) {
      // Salva lo stato se necessario
      ui.showCriticalBatteryWarning();
      
      // Entra in deep sleep
      esp_deep_sleep_start();
    }
  }
}

// Gestione del risparmio energetico
void checkPowerSaving() {
    static unsigned long lastActivityTime = 0;
    static unsigned long screenTimeout = 30000; // 30 secondi di timeout
    static bool screenOff = false;
    
    // Se c'è attività, resetta il timer
    if (input.anyButtonPressed()) {
        lastActivityTime = millis();
        
        // Se lo schermo era spento, riaccendilo
        if (screenOff) {
            tft.setBrightness(100); // Imposta la luminosità al 100%
            screenOff = false;
            ui.updateScreen(&srix);
        }
    }
    
    // Se è passato il timeout senza attività, spegni lo schermo
    if (!screenOff && millis() - lastActivityTime > screenTimeout) {
        tft.setBrightness(0); // Spegni la retroilluminazione
        screenOff = true;
    }
    
    // Deep sleep dopo 5 minuti di inattività
    if (millis() - lastActivityTime > 300000) { // 5 minuti
        // Salva lo stato se necessario
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextSize(1);
        tft.setCursor(10, 10);
        tft.println("Entrando in modalità sleep...");
        tft.setCursor(10, 30);
        tft.println("Premi un tasto per riattivare");
        delay(2000);
        
        // Spegni il display
        tft.setBrightness(0);
        
        // Configura il wakeup dai pulsanti
        esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_SELECT, 0); // Sveglia con il pulsante SELECT
        
        // Entra in deep sleep
        esp_deep_sleep_start();
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("SRIX ESP32-S3 Reader");
    
    // Controlla se stiamo uscendo dal deep sleep
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
        Serial.println("Riattivazione dal deep sleep");
    }
    
    // Imposta la frequenza della CPU al massimo
    setCpuFrequencyMhz(240);
    
    // Inizializza la PSRAM se disponibile
    if (psramInit()) {
        Serial.println("PSRAM inizializzata");
    }
    
    // Configura i pin per il monitoraggio della batteria e della ricarica
    pinMode(BATTERY_PIN, INPUT);
    pinMode(CHARGING_PIN, INPUT);
    
    // Initialize display
    ui.begin();
    
    // Initialize NFC reader
    if (!nfcReader.begin()) {
        Serial.println("Failed to initialize NFC reader");
        ui.showScreen(SCREEN_HOME);
        ui.setTagPresent(false);
        ui.showErrorMessage("Errore NFC", "Impossibile inizializzare il lettore NFC");
    } else {
        Serial.println("NFC reader initialized");
        ui.showScreen(SCREEN_HOME);
    }
    
    // Initialize input and set callback
    input.begin();
    input.setCallback(handleButtonEvent);
    
    // Mostra lo stato iniziale della batteria
    int percentage = getBatteryPercentage();
    bool charging = isCharging();
    ui.updateBatteryStatus(percentage, charging);
    
    // Mostra messaggio di benvenuto
    ui.showWelcomeScreen();
}

void loop() {
    // Update input state
    input.update();
    
    // Check for tag presence periodically
    if (millis() - lastTagCheck > tagCheckInterval) {
        lastTagCheck = millis();
        
        bool tagPresent = nfcReader.isTagPresent();
        ui.setTagPresent(tagPresent);
        
        // Rileva cambiamenti nello stato del tag
        if (tagPresent && !lastTagState) {
            navigation.onTagDetected();
        } else if (!tagPresent && lastTagState) {
            navigation.onTagLost();
        }
        
        lastTagState = tagPresent;
    }
    
    // Controlla lo stato della batteria
    checkBatteryStatus();
    
    // Gestione del risparmio energetico
    checkPowerSaving();
    
    // Piccola pausa per risparmiare energia
    delay(10);
}
