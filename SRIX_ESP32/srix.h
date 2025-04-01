/*
 * srix.h - Definizioni per tag SRIX
 * 
 * Questo file definisce la classe Srix per la gestione dei tag SRIX4K.
 */

#ifndef SRIX_H
#define SRIX_H

#include <Arduino.h>
#include "nfc_reader.h"
#include "srix_flag.h"
#include "mikai_error.h"

#define SRIX4K_BLOCKS 128
#define SRIX4K_BYTES (SRIX4K_BLOCKS * SRIX_BLOCK_LENGTH)

class Srix {
private:
    union {
        struct {
            uint32_t otp[5];            // 0-4 Resettable OTP bits
            uint32_t counter[2];        // 5-6 Count down counter
            uint32_t lockable[9];       // 7-15 Lockable EEPROM
            uint32_t generic[112];      // 16-127 EEPROM
        };
        uint32_t eeprom[SRIX4K_BLOCKS]; // SRIX4K EEPROM
    };
    
    uint64_t uid;                       // SRIX UID
    SrixFlag blockFlags;                // Modified block flags
    NfcReader* reader;                  // NFC Reader
    
    MikaiError getUid();
    MikaiError readBlocks();
    MikaiError writeGroup(uint32_t* groupPointer, uint8_t groupSize);

public:
    Srix(NfcReader* nfcReader);
    ~Srix();
    
    MikaiError init();
    void memoryInit(uint32_t eeprom[SRIX4K_BLOCKS], uint64_t uid);
    
    uint64_t getUid();
    uint32_t* getBlock(uint8_t blockNum);
    void modifyBlock(uint32_t block, uint8_t blockNum);
    MikaiError writeBlocks();
};

#endif // SRIX_H
