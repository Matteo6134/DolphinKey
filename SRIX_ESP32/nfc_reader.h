/*
 * nfc_reader.h - Interfaccia per il lettore NFC
 * 
 * Questo file definisce l'interfaccia per la comunicazione con il modulo NFC PN532
 * e l'implementazione delle funzionalità specifiche per i tag SRIX4K.
 */

#ifndef NFC_READER_H
#define NFC_READER_H

#include <Arduino.h>
#include <Adafruit_PN532.h>
#include "mikai_error.h"

#define SRIX_BLOCK_LENGTH 4
#define SRIX_UID_LENGTH 8

typedef struct SrixBlock {
    uint8_t block[SRIX_BLOCK_LENGTH];
} SrixBlock;

class NfcReader {
private:
    Adafruit_PN532* pn532;
    bool initialized;

public:
    NfcReader(uint8_t irq, uint8_t reset);
    ~NfcReader();
    
    bool begin();
    bool isTagPresent();
    
    MikaiError getUid(uint8_t uid[SRIX_UID_LENGTH]);
    MikaiError readBlock(SrixBlock* block, uint8_t blockNum);
    MikaiError writeBlock(SrixBlock* block, uint8_t blockNum);
};

#endif // NFC_READER_H
