/*
 * nfc_reader.cpp - Implementazione del lettore NFC
 * 
 * Questo file implementa l'interfaccia per la comunicazione con il modulo NFC PN532
 * e le funzionalità specifiche per i tag SRIX4K.
 */

#include "nfc_reader.h"

NfcReader::NfcReader(uint8_t irq, uint8_t reset) {
    pn532 = new Adafruit_PN532(irq, reset);
    initialized = false;
}

NfcReader::~NfcReader() {
    delete pn532;
}

bool NfcReader::begin() {
    pn532->begin();
    
    uint32_t versiondata = pn532->getFirmwareVersion();
    if (!versiondata) {
        return false;
    }
    
    // Configurazione per ISO14443B
    pn532->SAMConfig();
    
    initialized = true;
    return true;
}

bool NfcReader::isTagPresent() {
    uint8_t success;
    uint8_t uid[7] = { 0 };
    uint8_t uidLength;
    
    success = pn532->readPassiveTargetID(PN532_MIFARE_ISO14443B, uid, &uidLength);
    
    return success;
}

MikaiError NfcReader::getUid(uint8_t uid[SRIX_UID_LENGTH]) {
    if (!initialized) {
        return MIKAI_ERROR(MIKAI_NFC_ERROR, "NFC reader not initialized");
    }
    
    // Comando specifico per SRIX4K (0x0B = GET_UID)
    uint8_t command = 0x0B;
    uint8_t response[SRIX_UID_LENGTH];
    uint8_t responseLength = sizeof(response);
    
    if (!pn532->inDataExchange(&command, 1, response, &responseLength)) {
        return MIKAI_ERROR(MIKAI_NFC_ERROR, "Failed to get UID");
    }
    
    if (responseLength != SRIX_UID_LENGTH) {
        return MIKAI_ERROR(MIKAI_NFC_ERROR, "Invalid UID length");
    }
    
    memcpy(uid, response, SRIX_UID_LENGTH);
    
    return MIKAI_NO_ERROR;
}

MikaiError NfcReader::readBlock(SrixBlock* block, uint8_t blockNum) {
    if (!initialized) {
        return MIKAI_ERROR(MIKAI_NFC_ERROR, "NFC reader not initialized");
    }
    
    // Comando per leggere un blocco SRIX4K (0x08 = READ_BLOCK)
    uint8_t command[2] = { 0x08, blockNum };
    uint8_t response[SRIX_BLOCK_LENGTH];
    uint8_t responseLength = sizeof(response);
    
    if (!pn532->inDataExchange(command, 2, response, &responseLength)) {
        return MIKAI_ERROR(MIKAI_NFC_ERROR, "Failed to read block");
    }
    
    if (responseLength != SRIX_BLOCK_LENGTH) {
        return MIKAI_ERROR(MIKAI_NFC_ERROR, "Invalid block length");
    }
    
    memcpy(block->block, response, SRIX_BLOCK_LENGTH);
    
    return MIKAI_NO_ERROR;
}

MikaiError NfcReader::writeBlock(SrixBlock* block, uint8_t blockNum) {
    if (!initialized) {
        return MIKAI_ERROR(MIKAI_NFC_ERROR, "NFC reader not initialized");
    }
    
    // Comando per scrivere un blocco SRIX4K (0x09 = WRITE_BLOCK)
    uint8_t command[6] = { 
        0x09, 
        blockNum, 
        block->block[0], 
        block->block[1], 
        block->block[2], 
        block->block[3] 
    };
    
    if (!pn532->inDataExchange(command, 6, NULL, 0)) {
        return MIKAI_ERROR(MIKAI_NFC_ERROR, "Failed to write block");
    }
    
    // Verifica che il blocco sia stato scritto correttamente
    SrixBlock checkBlock;
    MikaiError error = readBlock(&checkBlock, blockNum);
    if (MIKAI_IS_ERROR(error)) {
        return error;
    }
    
    if (memcmp(block->block, checkBlock.block, SRIX_BLOCK_LENGTH) != 0) {
        return MIKAI_ERROR(MIKAI_NFC_ERROR, "Block verification failed");
    }
    
    return MIKAI_NO_ERROR;
}
