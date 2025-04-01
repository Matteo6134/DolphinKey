/*
 * srix_flag.cpp - Implementazione dei flag SRIX
 * 
 * Questo file implementa le funzioni per la gestione dei flag di modifica
 * dei blocchi SRIX.
 */

#include "srix_flag.h"

// Aggiunge un flag per un blocco specifico
void srixFlagAdd(SrixFlag* flag, uint8_t blockNum) {
    uint8_t flagIndex = blockNum / 32;
    uint8_t bitIndex = blockNum % 32;
    
    if (flagIndex < 4) {
        (*flag)[flagIndex] |= (1UL << bitIndex);
    }
}

// Verifica se un blocco è flaggato
bool srixFlagGet(SrixFlag* flag, uint8_t blockNum) {
    uint8_t flagIndex = blockNum / 32;
    uint8_t bitIndex = blockNum % 32;
    
    if (flagIndex < 4) {
        return ((*flag)[flagIndex] & (1UL << bitIndex)) != 0;
    }
    
    return false;
}

// Rimuove il flag per un blocco specifico
void srixFlagClear(SrixFlag* flag, uint8_t blockNum) {
    uint8_t flagIndex = blockNum / 32;
    uint8_t bitIndex = blockNum % 32;
    
    if (flagIndex < 4) {
        (*flag)[flagIndex] &= ~(1UL << bitIndex);
    }
}

// Resetta tutti i flag
void srixFlagReset(SrixFlag* flag) {
    for (uint8_t i = 0; i < 4; i++) {
        (*flag)[i] = 0;
    }
}
