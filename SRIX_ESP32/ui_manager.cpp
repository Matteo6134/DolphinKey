/*
 * ui_manager.cpp - Implementazione della gestione dell'interfaccia utente
 * 
 * Questo file implementa la classe UiManager per la gestione dell'interfaccia
 * utente grafica sul display.
 */

#include "ui_manager.h"

UiManager::UiManager(TFT_eSPI* display) {
    tft = display;
    currentScreen = SCREEN_HOME;
    selectedItem = MENU_NONE;
    currentBlockIndex = 0;
    tagPresent = false;
}

void UiManager::begin() {
    tft->init();
    tft->setRotation(0); // Potrebbe essere necessario modificare in base all'orientamento del display
    tft->fillScreen(TFT_BLACK);
    
    showScreen(SCREEN_HOME);
}

void UiManager::setTagPresent(bool present) {
    tagPresent = present;
    updateScreen();
}

void UiManager::showScreen(Screen screen) {
    currentScreen = screen;
    selectedItem = MENU_NONE;
    tft->fillScreen(TFT_BLACK);
    updateScreen();
}

void UiManager::updateScreen(Srix* srix) {
    drawStatusBar();
    
    switch (currentScreen) {
        case SCREEN_HOME:
            drawHomeScreen();
            break;
        case SCREEN_TAG_INFO:
            drawTagInfoScreen(srix);
            break;
        case SCREEN_READ_BLOCKS:
            drawReadBlocksScreen(srix);
            break;
        case SCREEN_BLOCK_DETAIL:
            drawBlockDetailScreen(srix);
            break;
        case SCREEN_WRITE_BLOCK:
            drawWriteBlockScreen(srix);
            break;
        case SCREEN_SETTINGS:
            drawSettingsScreen();
            break;
    }
    
    drawNavigationBar();
}

void UiManager::drawStatusBar() {
    tft->fillRect(0, 0, tft->width(), 20, TFT_NAVY);
    tft->setTextColor(TFT_WHITE, TFT_NAVY);
    tft->setTextSize(1);
    
    // NFC status
    tft->setCursor(5, 5);
    tft->print("NFC ");
    if (tagPresent) {
        tft->fillCircle(30, 10, 5, TFT_GREEN);
    } else {
        tft->drawCircle(30, 10, 5, TFT_RED);
    }
    
    // Screen title
    tft->setCursor(tft->width() / 2 - 30, 5);
    switch (currentScreen) {
        case SCREEN_HOME:
            tft->print("Home");
            break;
        case SCREEN_TAG_INFO:
            tft->print("Info Tag");
            break;
        case SCREEN_READ_BLOCKS:
            tft->print("Blocchi");
            break;
        case SCREEN_BLOCK_DETAIL:
            tft->print("Dettaglio");
            break;
        case SCREEN_WRITE_BLOCK:
            tft->print("Scrittura");
            break;
        case SCREEN_SETTINGS:
            tft->print("Impostazioni");
            break;
    }
    
    // Battery status (placeholder - will be updated by updateBatteryStatus)
    tft->setCursor(tft->width() - 50, 5);
    tft->print("Batt ");
    tft->fillRect(tft->width() - 20, 5, 15, 10, TFT_DARKGREY);
}

void UiManager::drawNavigationBar() {
    tft->fillRect(0, tft->height() - 20, tft->width(), 20, TFT_NAVY);
    tft->setTextColor(TFT_WHITE, TFT_NAVY);
    tft->setTextSize(1);
    
    // Navigation hints based on current screen
    switch (currentScreen) {
        case SCREEN_HOME:
            tft->setCursor(5, tft->height() - 15);
            tft->print("[Menu]");
            tft->setCursor(tft->width() - 60, tft->height() - 15);
            tft->print("[Impostaz.]");
            break;
        case SCREEN_TAG_INFO:
        case SCREEN_READ_BLOCKS:
        case SCREEN_SETTINGS:
            tft->setCursor(5, tft->height() - 15);
            tft->print("[Sel]");
            tft->setCursor(tft->width() - 50, tft->height() - 15);
            tft->print("[Su/Giu]");
            break;
        case SCREEN_BLOCK_DETAIL:
            tft->setCursor(5, tft->height() - 15);
            tft->print("[Det]");
            tft->setCursor(tft->width() - 50, tft->height() - 15);
            tft->print("[Su/Giu]");
            break;
        case SCREEN_WRITE_BLOCK:
            tft->setCursor(5, tft->height() - 15);
            tft->print("[Conferma]");
            tft->setCursor(tft->width() - 50, tft->height() - 15);
            tft->print("[Annulla]");
            break;
    }
}

void UiManager::drawHomeScreen() {
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextSize(1);
    
    tft->setCursor(10, 40);
    tft->println("Avvicina un tag");
    tft->setCursor(10, 60);
    tft->println("SRIX4K");
    
    // Logo o immagine (opzionale)
    tft->drawRect(tft->width()/2 - 30, 90, 60, 60, TFT_WHITE);
    tft->drawLine(tft->width()/2 - 30, 90, tft->width()/2 + 30, 150, TFT_WHITE);
    tft->drawLine(tft->width()/2 + 30, 90, tft->width()/2 - 30, 150, TFT_WHITE);
}

void UiManager::drawTagInfoScreen(Srix* srix) {
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextSize(1);
    
    if (srix) {
        char uidBuffer[20];
        sprintf(uidBuffer, "0x%016llX", srix->getUid());
        
        tft->setCursor(10, 30);
        tft->print("UID: ");
        tft->println(uidBuffer);
        
        tft->setCursor(10, 50);
        tft->println("Tipo: SRIX4K");
        
        tft->setCursor(10, 70);
        tft->println("Stato: Valido");
    } else {
        tft->setCursor(10, 40);
        tft->println("Nessun tag rilevato");
    }
    
    // Menu options
    int menuY = 100;
    int menuSpacing = 20;
    
    // Leggi Blocchi
    tft->setCursor(10, menuY);
    if (selectedItem == MENU_FIRST) {
        tft->setTextColor(TFT_BLACK, TFT_WHITE);
    } else {
        tft->setTextColor(TFT_WHITE, TFT_BLACK);
    }
    tft->println("> Leggi Blocchi");
    
    // Scrivi Blocchi
    tft->setCursor(10, menuY + menuSpacing);
    if (selectedItem == MENU_SECOND) {
        tft->setTextColor(TFT_BLACK, TFT_WHITE);
    } else {
        tft->setTextColor(TFT_WHITE, TFT_BLACK);
    }
    tft->println("  Scrivi Blocchi");
    
    // Indietro
    tft->setCursor(10, menuY + menuSpacing * 2);
    if (selectedItem == MENU_THIRD) {
        tft->setTextColor(TFT_BLACK, TFT_WHITE);
    } else {
        tft->setTextColor(TFT_WHITE, TFT_BLACK);
    }
    tft->println("  Indietro");
}

void UiManager::drawReadBlocksScreen(Srix* srix) {
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextSize(1);
    
    if (srix) {
        int startBlock = currentBlockIndex;
        int endBlock = min(startBlock + 3, SRIX4K_BLOCKS - 1);
        
        for (int i = startBlock; i <= endBlock; i++) {
            int y = 30 + (i - startBlock) * 40;
            
            char blockNumBuffer[10];
            sprintf(blockNumBuffer, "%02X", i);
            
            tft->setCursor(10, y);
            tft->print("Blocco: ");
            tft->println(blockNumBuffer);
            
            uint32_t* blockData = srix->getBlock(i);
            if (blockData) {
                char blockDataBuffer[20];
                sprintf(blockDataBuffer, "0x%08lX", *blockData);
                
                tft->setCursor(10, y + 20);
                tft->println(blockDataBuffer);
            }
        }
        
        // Indicatore di scorrimento
        int totalBlocks = SRIX4K_BLOCKS;
        int scrollBarHeight = tft->height() - 60;
        int scrollBarY = 30 + (scrollBarHeight * startBlock) / totalBlocks;
        int scrollBarSize = max(5, (scrollBarHeight * 4) / totalBlocks);
        
        tft->drawRect(tft->width() - 10, 30, 5, scrollBarHeight, TFT_DARKGREY);
        tft->fillRect(tft->width() - 10, scrollBarY, 5, scrollBarSize, TFT_WHITE);
    } else {
        tft->setCursor(10, 40);
        tft->println("Nessun tag rilevato");
    }
}

void UiManager::drawBlockDetailScreen(Srix* srix) {
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextSize(1);
    
    if (srix) {
        char blockNumBuffer[10];
        sprintf(blockNumBuffer, "%02X", currentBlockIndex);
        
        tft->setCursor(10, 30);
        tft->print("Blocco: ");
        tft->println(blockNumBuffer);
        
        uint32_t* blockData = srix->getBlock(currentBlockIndex);
        if (blockData) {
            char blockDataBuffer[20];
            sprintf(blockDataBuffer, "0x%08lX", *blockData);
            
            tft->setCursor(10, 50);
            tft->print("Hex: ");
            tft->println(blockDataBuffer);
            
            // Visualizzazione bit per bit
            tft->setCursor(10, 70);
            tft->println("Bit:");
            
            for (int i = 0; i < 8; i++) {
                char nibbleBuffer[5];
                sprintf(nibbleBuffer, "%X", (*blockData >> (28 - i * 4)) & 0xF);
                
                tft->setCursor(10 + i * 15, 90);
                tft->println(nibbleBuffer);
                
                // Posizioni
                tft->setCursor(10 + i * 15, 110);
                tft->println(i);
            }
            
            // ASCII (se applicabile)
            tft->setCursor(10, 130);
            tft->print("ASCII: ");
            
            for (int i = 0; i < 4; i++) {
                char c = (*blockData >> (24 - i * 8)) & 0xFF;
                if (c >= 32 && c <= 126) {
                    tft->print(c);
                } else {
                    tft->print('.');
                }
            }
        }
    } else {
        tft->setCursor(10, 40);
        tft->println("Nessun tag rilevato");
    }
}

void UiManager::drawWriteBlockScreen(Srix* srix) {
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextSize(1);
    
    if (srix) {
        char blockNumBuffer[10];
        sprintf(blockNumBuffer, "%02X", currentBlockIndex);
        
        tft->setCursor(10, 30);
        tft->print("Modifica Blocco: ");
        tft->println(blockNumBuffer);
        
        uint32_t* blockData = srix->getBlock(currentBlockIndex);
        if (blockData) {
            char blockDataBuffer[20];
            sprintf(blockDataBuffer, "0x%08lX", *blockData);
            
            tft->setCursor(10, 50);
            tft->print("Valore: ");
            tft->println(blockDataBuffer);
            
            // Editor esadecimale
            tft->setCursor(10, 80);
            tft->println("Usa Su/Giu per modificare il valore");
            tft->setCursor(10, 100);
            tft->println("Usa Sx/Dx per spostare il cursore");
            
            // Visualizzazione dei nibble con cursore
            for (int i = 0; i < 8; i++) {
                char nibbleBuffer[5];
                sprintf(nibbleBuffer, "%X", (*blockData >> (28 - i * 4)) & 0xF);
                
                if (selectedItem == (MenuItem)i) {
                    tft->setTextColor(TFT_BLACK, TFT_WHITE);
                } else {
                    tft->setTextColor(TFT_WHITE, TFT_BLACK);
                }
                
                tft->setCursor(10 + i * 15, 130);
                tft->println(nibbleBuffer);
            }
            
            tft->setTextColor(TFT_WHITE, TFT_BLACK);
            tft->setCursor(10, 150);
            tft->println("Premi SELECT per confermare");
        }
    } else {
        tft->setCursor(10, 40);
        tft->println("Nessun tag rilevato");
    }
}

void UiManager::drawSettingsScreen() {
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextSize(1);
    
    tft->setCursor(10, 30);
    tft->println("Impostazioni");
    
    // Menu options
    int menuY = 60;
    int menuSpacing = 20;
    
    // Luminosità
    tft->setCursor(10, menuY);
    if (selectedItem == MENU_FIRST) {
        tft->setTextColor(TFT_BLACK, TFT_WHITE);
    } else {
        tft->setTextColor(TFT_WHITE, TFT_BLACK);
    }
    tft->println("> Luminosita'");
    
    // Timeout
    tft->setCursor(10, menuY + menuSpacing);
    if (selectedItem == MENU_SECOND) {
        tft->setTextColor(TFT_BLACK, TFT_WHITE);
    } else {
        tft->setTextColor(TFT_WHITE, TFT_BLACK);
    }
    tft->println("  Timeout");
    
    // Informazioni
    tft->setCursor(10, menuY + menuSpacing * 2);
    if (selectedItem == MENU_THIRD) {
        tft->setTextColor(TFT_BLACK, TFT_WHITE);
    } else {
        tft->setTextColor(TFT_WHITE, TFT_BLACK);
    }
    tft->println("  Informazioni");
    
    // Indietro
    tft->setCursor(10, menuY + menuSpacing * 3);
    if (selectedItem == MENU_FOURTH) {
        tft->setTextColor(TFT_BLACK, TFT_WHITE);
    } else {
        tft->setTextColor(TFT_WHITE, TFT_BLACK);
    }
    tft->println("  Indietro");
    
    // Versione
    tft->setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft->setCursor(10, tft->height() - 40);
    tft->println("SRIX ESP32-S3 Reader");
    tft->setCursor(10, tft->height() - 30);
    tft->println("v1.0.0");
}

void UiManager::selectNextItem() {
    if (selectedItem == MENU_NONE) {
        selectedItem = MENU_FIRST;
    } else if (selectedItem < MENU_FOURTH) {
        selectedItem = (MenuItem)((int)selectedItem + 1);
    }
    updateScreen();
}

void UiManager::selectPreviousItem() {
    if (selectedItem == MENU_NONE || selectedItem == MENU_FIRST) {
        selectedItem = MENU_FIRST;
    } else {
        selectedItem = (MenuItem)((int)selectedItem - 1);
    }
    updateScreen();
}

void UiManager::selectItem(MenuItem item) {
    selectedItem = item;
    updateScreen();
}

MenuItem UiManager::getSelectedItem() {
    return selectedItem;
}

void UiManager::nextBlock() {
    if (currentBlockIndex < SRIX4K_BLOCKS - 1) {
        currentBlockIndex++;
        updateScreen();
    }
}

void UiManager::previousBlock() {
    if (currentBlockIndex > 0) {
        currentBlockIndex--;
        updateScreen();
    }
}

uint8_t UiManager::getCurrentBlock() {
    return currentBlockIndex;
}

Screen UiManager::getCurrentScreen() {
    return currentScreen;
}

void UiManager::showErrorMessage(const char* title, const char* message) {
    // Salva lo stato corrente
    Screen previousScreen = currentScreen;
    
    // Mostra il messaggio di errore
    tft->fillRect(20, 60, tft->width() - 40, 80, TFT_RED);
    tft->setTextColor(TFT_WHITE, TFT_RED);
    tft->setTextSize(1);
    
    tft->setCursor(30, 70);
    tft->println(title);
    
    tft->setCursor(30, 90);
    tft->println(message);
    
    tft->setCursor(30, 120);
    tft->println("Premi un tasto per continuare");
    
    // Attendi la pressione di un tasto
    delay(2000);
    
    // Ripristina lo stato precedente
    updateScreen();
}

void UiManager::showLowBatteryWarning() {
    // Mostra avviso
    tft->fillRect(20, 60, tft->width() - 40, 60, TFT_RED);
    tft->setTextColor(TFT_WHITE, TFT_RED);
    tft->setTextSize(1);
    tft->setCursor(30, 70);
    tft->print("Batteria scarica!");
    tft->setCursor(30, 90);
    tft->print("Collegare il caricabatterie");
    
    delay(3000);
    
    // Ripristina lo schermo
    updateScreen();
}

void UiManager::showCriticalBatteryWarning() {
    tft->fillScreen(TFT_BLACK);
    tft->setTextColor(TFT_RED, TFT_BLACK);
    tft->setTextSize(2);
    tft->setCursor(10, 10);
    tft->println("BATTERIA");
    tft->setCursor(10, 40);
    tft->println("CRITICA");
    tft->setTextSize(1);
    tft->setCursor(10, 70);
    tft->println("Spegnimento in corso...");
    
    delay(3000);
    
    // Spegni il display
    tft->setBrightness(0);
}

void UiManager::showWelcomeScreen() {
    tft->fillScreen(TFT_BLACK);
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextSize(2);
    tft->setCursor(10, 40);
    tft->println("SRIX4K");
    tft->setCursor(10, 70);
    tft->println("READER");
    
    tft->setTextSize(1);
    tft->setCursor(10, 110);
    tft->println("ESP32-S3 Edition");
    tft->setCursor(10, 130);
    tft->println("v1.0.0");
    
    delay(2000);
    
    showScreen(SCREEN_HOME);
}

void UiManager::updateBatteryStatus(int percentage, bool charging) {
    // Aggiorna l'icona della batteria nella barra di stato
    tft->fillRect(tft->width() - 30, 5, 25, 10, TFT_NAVY);
    
    // Disegna il contorno della batteria
    tft->drawRect(tft->width() - 28, 5, 20, 10, TFT_WHITE);
    tft->drawRect(tft->width() - 8, 7, 2, 6, TFT_WHITE);
    
    // Riempi in base alla percentuale
    int fillWidth = map(percentage, 0, 100, 0, 18);
    
    // Colore in base al livello o stato di ricarica
    uint16_t color;
    if (charging)
(Content truncated due to size limit. Use line ranges to read in chunks)