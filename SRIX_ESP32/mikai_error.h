/*
 * mikai_error.h - Definizione della struttura di gestione degli errori
 * 
 * Questo file definisce la struttura MikaiError e le macro per la gestione
 * degli errori nel progetto SRIX_ESP32.
 */

#ifndef MIKAI_ERROR_H
#define MIKAI_ERROR_H

#include <Arduino.h>

// Codici di errore
#define MIKAI_NO_ERROR_CODE 0
#define MIKAI_NFC_ERROR 1
#define MIKAI_SRIX_ERROR 2
#define MIKAI_UI_ERROR 3
#define MIKAI_INPUT_ERROR 4
#define MIKAI_SYSTEM_ERROR 5

// Struttura per la gestione degli errori
typedef struct {
    uint8_t code;
    const char* message;
} MikaiError;

// Macro per la creazione di errori
#define MIKAI_ERROR(code, msg) ((MikaiError){code, msg})
#define MIKAI_NO_ERROR ((MikaiError){MIKAI_NO_ERROR_CODE, NULL})
#define MIKAI_IS_ERROR(error) ((error).code != MIKAI_NO_ERROR_CODE)

#endif // MIKAI_ERROR_H
