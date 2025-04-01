/*
 * srix_flag.h - Definizioni per i flag SRIX
 * 
 * Questo file definisce la struttura SrixFlag e le funzioni per la gestione
 * dei flag di modifica dei blocchi SRIX.
 */

#ifndef SRIX_FLAG_H
#define SRIX_FLAG_H

#include <Arduino.h>

// Definizione della struttura SrixFlag
typedef uint32_t SrixFlag[4];

// Valore di inizializzazione
#define SRIX_FLAG_INIT {0, 0, 0, 0}

// Funzioni per la gestione dei flag
void srixFlagAdd(SrixFlag* flag, uint8_t blockNum);
bool srixFlagGet(SrixFlag* flag, uint8_t blockNum);
void srixFlagClear(SrixFlag* flag, uint8_t blockNum);
void srixFlagReset(SrixFlag* flag);

#endif // SRIX_FLAG_H
