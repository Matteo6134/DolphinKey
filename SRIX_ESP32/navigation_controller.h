#ifndef NAVIGATION_CONTROLLER_H
#define NAVIGATION_CONTROLLER_H

#include "ui_manager.h"
#include "srix.h"
#include "input_manager.h" // Include per Button e ButtonEvent

class NavigationController {
public:
    NavigationController(UiManager* uiManager, Srix* srixTag);

    // Gestisce gli eventi dei pulsanti provenienti da InputManager
    void handleButtonEvent(Button button, ButtonEvent event);

    // Chiamata quando un tag viene rilevato
    void onTagDetected();

    // Chiamata quando un tag viene perso
    void onTagLost();

private:
    UiManager* ui;
    Srix* srix;

    // Stato per la schermata di scrittura
    int currentEditingBlock = -1;
    int cursorPosition = 0; // 0 per nibble alto, 1 per nibble basso
    uint8_t currentEditingValue[SRIX_BLOCK_SIZE]; // Buffer per il valore durante l'edit
    uint8_t originalEditingValue[SRIX_BLOCK_SIZE]; // Per eventuale annullamento

    // Funzioni helper private per gestire logica complessa
    void handleHomeInput(Button button);
    void handleTagInfoInput(Button button);
    void handleReadBlocksInput(Button button);
    void handleBlockDetailInput(Button button);
    void handleWriteBlockInput(Button button);
    void handleSettingsInput(Button button);

    void startEditingBlock(int blockIndex);
    void updateEditingValue(bool increment);
    void confirmEdit();
    void cancelEdit(); // Potrebbe essere utile in futuro
    void writeModifiedBlocks(); // Funzione per avviare la scrittura sul tag
};

#endif // NAVIGATION_CONTROLLER_H