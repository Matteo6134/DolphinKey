#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <Arduino.h>

// Enum per i pulsanti
enum Button {
    BUTTON_UP_PIN,
    BUTTON_DOWN_PIN,
    BUTTON_LEFT_PIN,
    BUTTON_RIGHT_PIN,
    BUTTON_SELECT_PIN,
    NUM_BUTTONS,
    BUTTON_NONE // Aggiunto per indicare nessun pulsante premuto
};

// Enum per gli eventi dei pulsanti
enum ButtonEvent {
    BUTTON_PRESSED,
    BUTTON_RELEASED
    // Si potrebbe aggiungere BUTTON_HELD per pressioni prolungate in futuro
};

// Definizione del tipo per la funzione di callback
typedef void (*ButtonCallback)(Button button, ButtonEvent event);

class InputManager {
public:
    // Costruttore: prende i pin per ogni pulsante
    InputManager(int pinUp, int pinDown, int pinLeft, int pinRight, int pinSelect);

    // Inizializza i pin e gli stati
    void begin();

    // Aggiorna lo stato dei pulsanti (da chiamare nel loop principale)
    void update();

    // Imposta la funzione di callback da chiamare quando si verifica un evento
    void setCallback(ButtonCallback callback);

    // Verifica se un qualsiasi pulsante è attualmente premuto (utile per power saving)
    bool anyButtonPressed();

private:
    int buttonPins[NUM_BUTTONS];
    bool lastButtonState[NUM_BUTTONS];
    bool currentButtonState[NUM_BUTTONS];
    unsigned long lastDebounceTime[NUM_BUTTONS];
    unsigned long debounceDelay = 50; // ms
    ButtonCallback eventCallback = nullptr;
};

#endif // INPUT_MANAGER_H