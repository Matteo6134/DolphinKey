/*
 * srix.cpp - Implementazione delle funzionalità SRIX
 * 
 * Questo file implementa la classe Srix per la gestione dei tag SRIX4K.
 */

#include "srix.h"

Srix::Srix(NfcReader* nfcReader) {
    reader = nfcReader;
    blockFlags = SRIX_FLAG_INIT;
    memset(eeprom, 0, sizeof(eeprom));
    uid = 0;
}

Srix::~Srix() {
    // Il reader è gestito esternamente
}

MikaiError Srix::getUid() {
    uint8_t uidBytes[SRIX_UID_LENGTH];
    
    MikaiError error = reader->getUid(uidBytes);
    if (MIKAI_IS_ERROR(error)) {
        return error;
    }
    
    // Check manufacturer code (datasheet of SRIX4K and ST25TB04K)
    if (uidBytes[7] != 0xD0 || uidBytes[6] != 0x02) {
        return MIKAI_ERROR(MIKAI_NFC_ERROR, "invalid tag manufacturer code");
    }
    
    // Convert UID to uint64
    uid = (uint64_t) uidBytes[7] << 56U | (uint64_t) uidBytes[6] << 48U | 
          (uint64_t) uidBytes[5] << 40U | (uint64_t) uidBytes[4] << 32U | 
          (uint64_t) uidBytes[3] << 24U | (uint64_t) uidBytes[2] << 16U |
          (uint64_t) uidBytes[1] << 8U  | (uint64_t) uidBytes[0];
    
    return MIKAI_NO_ERROR;
}

MikaiError Srix::readBlocks() {
    for (uint8_t i = 0; i < SRIX4K_BLOCKS; i++) {
        SrixBlock readBlock;
        
        MikaiError error = reader->readBlock(&readBlock, i);
        if (MIKAI_IS_ERROR(error)) {
            return error;
        }
        
        eeprom[i] = readBlock.block[0] << 24 | readBlock.block[1] << 16 | 
                    readBlock.block[2] << 8  | readBlock.block[3];
    }
    
    return MIKAI_NO_ERROR;
}

MikaiError Srix::writeGroup(uint32_t* groupPointer, uint8_t groupSize) {
    for (uint8_t i = 0; i < groupSize; i++) {
        if (srixFlagGet(&blockFlags, groupPointer + i - eeprom)) {
            SrixBlock writeBlock;
            writeBlock.block[0] = (groupPointer[i] >> 24) & 0xFF;
            writeBlock.block[1] = (groupPointer[i] >> 16) & 0xFF;
            writeBlock.block[2] = (groupPointer[i] >> 8) & 0xFF;
            writeBlock.block[3] = groupPointer[i] & 0xFF;
            
            MikaiError error = reader->writeBlock(&writeBlock, groupPointer + i - eeprom);
            if (MIKAI_IS_ERROR(error)) {
                return error;
            }
        }
    }
    
    return MIKAI_NO_ERROR;
}

MikaiError Srix::init() {
    if (!reader) {
        return MIKAI_ERROR(MIKAI_SRIX_ERROR, "NFC reader not initialized");
    }
    
    blockFlags = SRIX_FLAG_INIT;
    
    // Get SRIX4K UID & EEPROM
    MikaiError error = getUid();
    if (MIKAI_IS_ERROR(error)) {
        return error;
    }
    
    error = readBlocks();
    if (MIKAI_IS_ERROR(error)) {
        return error;
    }
    
    return MIKAI_NO_ERROR;
}

void Srix::memoryInit(uint32_t importEeprom[SRIX4K_BLOCKS], uint64_t importUid) {
    if (lockable[0] != 0) {
        // If memory has been already initialized (Key ID not null), copy only generic blocks
        memcpy(generic, importEeprom + 16, sizeof(generic));
        
        // If already initialized, flag all generic blocks
        for (uint8_t i = 16; i < 128; i++) {
            srixFlagAdd(&blockFlags, i);
        }
    } else {
        memcpy(eeprom, importEeprom, sizeof(eeprom));
    }
    
    uid = importUid;
}

uint64_t Srix::getUid() {
    return uid;
}

uint32_t* Srix::getBlock(uint8_t blockNum) {
    return (blockNum < SRIX4K_BLOCKS) ? (eeprom + blockNum) : NULL;
}

void Srix::modifyBlock(uint32_t block, uint8_t blockNum) {
    eeprom[blockNum] = block;
    srixFlagAdd(&blockFlags, blockNum);
}

MikaiError Srix::writeBlocks() {
    if (!reader) {
        return MIKAI_ERROR(MIKAI_SRIX_ERROR, "NFC reader not initialized");
    }
    
    // Counter blocks
    MikaiError error = writeGroup(counter, sizeof(counter) / sizeof(uint32_t));
    if (MIKAI_IS_ERROR(error)) {
        return error;
    }
    
    // OTP blocks
    error = writeGroup(otp, sizeof(otp) / sizeof(uint32_t));
    if (MIKAI_IS_ERROR(error)) {
        return error;
    }
    
    // Lockable blocks
    error = writeGroup(lockable, sizeof(lockable) / sizeof(uint32_t));
    if (MIKAI_IS_ERROR(error)) {
        return error;
    }
    
    // Generic blocks
    error = writeGroup(generic, sizeof(generic) / sizeof(uint32_t));
    if (MIKAI_IS_ERROR(error)) {
        return error;
    }
    
    blockFlags = SRIX_FLAG_INIT;
    return MIKAI_NO_ERROR;
}
