/*
 * ui_manager.h - Gestione dell'interfaccia utente
 * 
 * Questo file definisce la classe UiManager per la gestione dell'interfaccia
 * utente grafica sul display.
 */

#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "srix.h"

// Definizione delle schermate
enum Screen {
    SCREEN_HOME,
    SCREEN_TAG_INFO,
    SCREEN_READ_BLOCKS,
    SCREEN_BLOCK_DETAIL,
    SCREEN_WRITE_BLOCK,
    SCREEN_SETTINGS
};

// Definizione degli elementi di menu
enum MenuItem {
    MENU_NONE = -1,
    MENU_FIRST = 0,
    MENU_SECOND = 1,
    MENU_THIRD = 2,
    MENU_FOURTH = 3
};

class UiManager {
private:
    TFT_eSPI* tft;
    Screen currentScreen;
    MenuItem selectedItem;
    uint8_t currentBlockIndex;
    bool tagPresent;
    
    void drawStatusBar();
    void drawNavigationBar();
    
    void drawHomeScreen();
    void drawTagInfoScreen(Srix* srix);
    void drawReadBlocksScreen(Srix* srix);
    void drawBlockDetailScreen(Srix* srix);
    void drawWriteBlockScreen(Srix* srix);
    void drawSettingsScreen();

public:
    UiManager(TFT_eSPI* display);
    
    void begin();
    void setTagPresent(bool present);
    
    void showScreen(Screen screen);
    void updateScreen(Srix* srix = NULL);
    
    void selectNextItem();
    void selectPreviousItem();
    void selectItem(MenuItem item);
    MenuItem getSelectedItem();
    
    void nextBlock();
    void previousBlock();
    uint8_t getCurrentBlock();
    
    Screen getCurrentScreen();
    
    void showErrorMessage(const char* title, const char* message);
    void showLowBatteryWarning();
    void showCriticalBatteryWarning();
    void showWelcomeScreen();
    void updateBatteryStatus(int percentage, bool charging);
    void highlightButton(int button);
};

#endif // UI_MANAGER_H
