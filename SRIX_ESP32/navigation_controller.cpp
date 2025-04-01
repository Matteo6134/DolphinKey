#include "navigation_controller.h"
#include "mikai_error.h" // Per MIKAI_OK

NavigationController::NavigationController(UiManager* uiManager, Srix* srixTag)
    : ui(uiManager), srix(srixTag) {}

void NavigationController::handleButtonEvent(Button button, ButtonEvent event) {
    // Gestiamo solo eventi di pressione per ora
    if (event != BUTTON_PRESSED) {
        return;
    }

    // La logica dipende dalla schermata corrente
    Screen currentScreen = ui->getCurrentScreen();

    switch (currentScreen) {
        case SCREEN_HOME:
            handleHomeInput(button);
            break;
        case SCREEN_TAG_INFO:
            handleTagInfoInput(button);
            break;
        case SCREEN_READ_BLOCKS:
            handleReadBlocksInput(button);
            break;
        case SCREEN_BLOCK_DETAIL:
            handleBlockDetailInput(button);
            break;
        case SCREEN_WRITE_BLOCK:
            handleWriteBlockInput(button);
            break;
        case SCREEN_SETTINGS:
            handleSettingsInput(button);
            break;
        default:
            // Schermata sconosciuta o non interattiva
            break;
    }
}

void NavigationController::onTagDetected() {
    Serial.println("NavigationController: Tag Detected");
    ui->showScreen(SCREEN_TAG_INFO); // Mostra subito qualcosa
    ui->setTagPresent(true);
    ui->displayTemporaryMessage("Lettura tag...", 1000);

    MikaiError err = srix->init(); // Legge UID e tutti i blocchi
    if (err == MIKAI_OK) {
        Serial.println("SRIX Init OK");
        ui->updateScreen(srix); // Aggiorna la schermata Tag Info con i dati letti
    } else {
        Serial.print("SRIX Init Error: ");
        Serial.println(err);
        ui->showErrorMessage("Errore Lettura Tag", "Impossibile leggere i dati SRIX");
        // Potremmo tornare a HOME o mostrare Tag Info con errore
        ui->showScreen(SCREEN_TAG_INFO); // Rimaniamo qui ma l'utente vedrà l'errore
        ui->updateScreen(nullptr); // Passa nullptr per indicare dati non validi
    }
}

void NavigationController::onTagLost() {
    Serial.println("NavigationController: Tag Lost");
    // Resetta lo stato interno se necessario (es. se si stava editando)
    if (ui->getCurrentScreen() == SCREEN_WRITE_BLOCK) {
         cancelEdit(); // Funzione ipotetica per annullare modifiche non salvate
    }
     currentEditingBlock = -1;

    // srix->reset(); // Potrebbe essere utile aggiungere un metodo a Srix per pulire i dati
    ui->setTagPresent(false);
    if(ui->getCurrentScreen() != SCREEN_HOME && ui->getCurrentScreen() != SCREEN_SETTINGS) {
        ui->showScreen(SCREEN_HOME);
    }
    ui->updateScreen(); // Aggiorna la UI (rimuove info tag, ecc.)
}

// --- Helper per gestire input per schermata ---

void NavigationController::handleHomeInput(Button button) {
    switch (button) {
        case BUTTON_SELECT_PIN:
            ui->showScreen(SCREEN_SETTINGS);
            ui->updateScreen();
            break;
        default:
            break; // Ignora altri pulsanti
    }
}

void NavigationController::handleTagInfoInput(Button button) {
    switch (button) {
        case BUTTON_UP_PIN:
            ui->selectPreviousItem();
            ui->updateScreen(srix);
            break;
        case BUTTON_DOWN_PIN:
            ui->selectNextItem();
            ui->updateScreen(srix);
            break;
        case BUTTON_LEFT_PIN:
            ui->showScreen(SCREEN_HOME);
            ui->updateScreen();
            break;
        case BUTTON_SELECT_PIN:
            MenuItem selected = ui->getSelectedItem();
            if (selected == MENU_READ_BLOCKS) {
                ui->showScreen(SCREEN_READ_BLOCKS);
                ui->updateScreen(srix);
            } else if (selected == MENU_WRITE_BLOCKS) {
                // Avvia la scrittura dei blocchi modificati
                writeModifiedBlocks();
            } else if (selected == MENU_BACK) {
                ui->showScreen(SCREEN_HOME);
                ui->updateScreen();
            }
            break;
        default:
            break;
    }
}

void NavigationController::handleReadBlocksInput(Button button) {
     switch (button) {
        case BUTTON_UP_PIN:
            ui->previousBlock();
            ui->updateScreen(srix);
            break;
        case BUTTON_DOWN_PIN:
            ui->nextBlock();
            ui->updateScreen(srix);
            break;
        case BUTTON_LEFT_PIN:
            ui->showScreen(SCREEN_TAG_INFO);
            ui->updateScreen(srix);
            break;
        case BUTTON_SELECT_PIN:
             ui->showScreen(SCREEN_BLOCK_DETAIL);
             ui->updateScreen(srix);
             break;
        default:
             break;
    }
}

void NavigationController::handleBlockDetailInput(Button button) {
    switch (button) {
        case BUTTON_LEFT_PIN:
            ui->showScreen(SCREEN_READ_BLOCKS);
            ui->updateScreen(srix);
            break;
        case BUTTON_SELECT_PIN:
            startEditingBlock(ui->getCurrentBlockIndex());
            break;
        default:
            break;
    }
}

void NavigationController::handleWriteBlockInput(Button button) {
    if (currentEditingBlock < 0) return; // Non dovrebbe succedere

    switch (button) {
        case BUTTON_UP_PIN:
            updateEditingValue(true); // Incrementa
            ui->updateScreen(srix, currentEditingBlock, cursorPosition, currentEditingValue);
            break;
        case BUTTON_DOWN_PIN:
            updateEditingValue(false); // Decrementa
            ui->updateScreen(srix, currentEditingBlock, cursorPosition, currentEditingValue);
            break;
        case BUTTON_LEFT_PIN:
            cursorPosition = (cursorPosition > 0) ? cursorPosition - 1 : (SRIX_BLOCK_SIZE * 2 - 1); // Muovi a sinistra, wraparound
            //cursorPosition = max(0, cursorPosition - 1); // Versione senza wrap
            ui->updateScreen(srix, currentEditingBlock, cursorPosition, currentEditingValue);
            break;
        case BUTTON_RIGHT_PIN:
             cursorPosition = (cursorPosition < (SRIX_BLOCK_SIZE * 2 - 1)) ? cursorPosition + 1 : 0; // Muovi a destra, wraparound
             //cursorPosition = min(SRIX_BLOCK_SIZE * 2 - 1, cursorPosition + 1); // Versione senza wrap
             ui->updateScreen(srix, currentEditingBlock, cursorPosition, currentEditingValue);
            break;
        case BUTTON_SELECT_PIN:
            confirmEdit(); // Conferma la modifica locale
            ui->showScreen(SCREEN_BLOCK_DETAIL); // Torna alla schermata dettaglio
            ui->updateScreen(srix);
            break;
        default:
            break;
    }
}

void NavigationController::handleSettingsInput(Button button) {
     switch (button) {
        case BUTTON_UP_PIN:
            ui->selectPreviousItem();
            ui->updateScreen();
            break;
        case BUTTON_DOWN_PIN:
            ui->selectNextItem();
            ui->updateScreen();
            break;
        case BUTTON_LEFT_PIN:
             ui->showScreen(SCREEN_HOME);
             ui->updateScreen();
             break;
        case BUTTON_SELECT_PIN:
             MenuItem selected = ui->getSelectedItem();
             if (selected == MENU_BRIGHTNESS) {
                 // TODO: Implementare logica per regolare luminosità
                 ui->displayTemporaryMessage("Luminosita non impl.", 1000);
             } else if (selected == MENU_TIMEOUT) {
                 // TODO: Implementare logica per regolare timeout
                 ui->displayTemporaryMessage("Timeout non impl.", 1000);
             } else if (selected == MENU_BACK) {
                 ui->showScreen(SCREEN_HOME);
                 ui->updateScreen();
             }
             break;
        default:
             break;
    }
}

// --- Funzioni Helper ---

void NavigationController::startEditingBlock(int blockIndex) {
    if (blockIndex < 0 || blockIndex >= SRIX_NUM_BLOCKS) return;

    MikaiError err;
    const uint8_t* blockData = srix->getBlock(blockIndex, err);

    if (err == MIKAI_OK && blockData != nullptr) {
        currentEditingBlock = blockIndex;
        cursorPosition = 0; // Inizia dal primo nibble
        memcpy(currentEditingValue, blockData, SRIX_BLOCK_SIZE); // Copia valore attuale nel buffer di edit
        memcpy(originalEditingValue, blockData, SRIX_BLOCK_SIZE); // Salva valore originale per eventuale annullamento
        ui->showScreen(SCREEN_WRITE_BLOCK);
        ui->updateScreen(srix, currentEditingBlock, cursorPosition, currentEditingValue);
    } else {
         ui->showErrorMessage("Errore Blocco", "Impossibile leggere dati blocco");
    }
}

void NavigationController::updateEditingValue(bool increment) {
    if (currentEditingBlock < 0) return;

    int byteIndex = cursorPosition / 2;
    int nibbleIndex = cursorPosition % 2; // 0 per alto, 1 per basso

    uint8_t currentByte = currentEditingValue[byteIndex];
    uint8_t currentNibble;
    uint8_t mask;
    int shift;

    if (nibbleIndex == 0) { // Nibble alto
        mask = 0xF0;
        shift = 4;
    } else { // Nibble basso
        mask = 0x0F;
        shift = 0;
    }

    currentNibble = (currentByte & mask) >> shift;

    if (increment) {
        currentNibble = (currentNibble + 1) % 16; // Incrementa con wrap around (0-F)
    } else {
        currentNibble = (currentNibble == 0) ? 15 : currentNibble - 1; // Decrementa con wrap around (F-0)
    }

    // Ricostruisci il byte
    currentEditingValue[byteIndex] = (currentByte & (~mask)) | (currentNibble << shift);
}

void NavigationController::confirmEdit() {
    if (currentEditingBlock < 0) return;

    Serial.printf("Confermato Blocco %d, Nuovo Valore: ", currentEditingBlock);
     for(int i=0; i<SRIX_BLOCK_SIZE; i++) Serial.printf("%02X", currentEditingValue[i]);
     Serial.println();

    // Modifica il blocco nella copia locale in Srix
    MikaiError err = srix->modifyBlock(currentEditingBlock, currentEditingValue);
    if (err != MIKAI_OK) {
         ui->showErrorMessage("Errore Modifica", "Impossibile salvare modifica locale");
         // Potremmo ripristinare currentEditingValue a originalEditingValue
    } else {
         // Modifica locale avvenuta con successo
         ui->displayTemporaryMessage("Modifica salvata localmente", 1000);
    }
    currentEditingBlock = -1; // Esci dalla modalità edit
}

void NavigationController::cancelEdit() {
     // TODO: Implementare se necessario (es. ripristinare da originalEditingValue)
     Serial.println("Edit Cancelled (placeholder)");
     currentEditingBlock = -1;
}

void NavigationController::writeModifiedBlocks() {
     Serial.println("Attempting to write modified blocks to tag...");
     ui->displayTemporaryMessage("Scrittura in corso...", 2000); // Messaggio temporaneo

     MikaiError err = srix->writeBlocks(); // Chiama la funzione di scrittura di Srix

     if (err == MIKAI_OK) {
         Serial.println("Write successful!");
         ui->displayTemporaryMessage("Scrittura Completata!", 1500);
     } else if (err == MIKAI_ERR_NFC_WRITE_NO_CHANGE) {
         Serial.println("No blocks were modified.");
         ui->displayTemporaryMessage("Nessuna modifica da scrivere", 1500);
     }
     else {
         Serial.print("Write failed! Error code: ");
         Serial.println(err);
         char errorMsg[50];
         snprintf(errorMsg, sizeof(errorMsg), "Errore Scrittura (Cod: %d)", err);
         ui->showErrorMessage("Errore Scrittura Tag", errorMsg);
     }
     // Aggiorna la UI nel caso qualcosa sia cambiato (anche solo lo stato "modificato")
     ui->updateScreen(srix);
}