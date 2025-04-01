#include "input_manager.h"

InputManager::InputManager(int pinUp, int pinDown, int pinLeft, int pinRight, int pinSelect) {
    buttonPins[BUTTON_UP_PIN] = pinUp;
    buttonPins[BUTTON_DOWN_PIN] = pinDown;
    buttonPins[BUTTON_LEFT_PIN] = pinLeft;
    buttonPins[BUTTON_RIGHT_PIN] = pinRight;
    buttonPins[BUTTON_SELECT_PIN] = pinSelect;
}

void InputManager::begin() {
    for (int i = 0; i < NUM_BUTTONS; i++) {
        pinMode(buttonPins[i], INPUT_PULLUP); // Usa pull-up interno come da documentazione
        lastButtonState[i] = digitalRead(buttonPins[i]);
        currentButtonState[i] = lastButtonState[i];
        lastDebounceTime[i] = 0;
    }
}

void InputManager::update() {
    unsigned long currentTime = millis();

    for (int i = 0; i < NUM_BUTTONS; i++) {
        bool reading = digitalRead(buttonPins[i]);

        // Controlla se lo stato è cambiato (ignorando il rumore)
        if (reading != lastButtonState[i]) {
            lastDebounceTime[i] = currentTime;
        }

        if ((currentTime - lastDebounceTime[i]) > debounceDelay) {
            // Se lo stato si è stabilizzato dopo il debounce
            if (reading != currentButtonState[i]) {
                currentButtonState[i] = reading;

                // Chiama il callback se impostato
                if (eventCallback != nullptr) {
                    // I pulsanti sono attivi BASSI a causa del pull-up
                    if (currentButtonState[i] == LOW) {
                        eventCallback((Button)i, BUTTON_PRESSED);
                    } else {
                        eventCallback((Button)i, BUTTON_RELEASED);
                    }
                }
            }
        }
        lastButtonState[i] = reading;
    }
}

void InputManager::setCallback(ButtonCallback callback) {
    eventCallback = callback;
}

bool InputManager::anyButtonPressed() {
     for (int i = 0; i < NUM_BUTTONS; i++) {
         // Stato LOW significa premuto a causa del PULLUP
         if (currentButtonState[i] == LOW) {
             return true;
         }
     }
     return false;
}