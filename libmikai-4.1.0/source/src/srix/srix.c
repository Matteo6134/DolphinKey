/*
 * @author      Lilz <https://telegram.me/Lilz73>
 * @copyright   2020-2021 Lilz <https://telegram.me/Lilz73>
 * @license     MIKAI LICENSE
 *
 * This file is part of MIKAI.
 *
 * MIKAI is free software: you can redistribute it and/or modify
 * it under the terms of the MIKAI License, as published by
 * Lilz along with this program and available on "MIKAI Download" Telegram channel
 * <https://telegram.me/mikaidownload>.
 *
 * MIKAI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY.
 *
 * You should have received a copy of the MIKAI License along
 * with MIKAI.
 * If not, see <https://telegram.me/mikaidownload>.
 */

#include <string.h>
#include <mikai-error.h>
#include "srix.h"
#include <reader/reader.h>

/**
 * Generic SRIX4K tag
 */
struct Srix {
    union {
        struct {
            uint32_t otp[5];            /* 0-4 Resettable OTP bits */
            uint32_t counter[2];        /* 5-6 Count down counter */
            uint32_t lockable[9];       /* 7-15 Lockable EEPROM */
            uint32_t generic[112];      /* 16-127 EEPROM */
        };
        uint32_t eeprom[SRIX4K_BLOCKS]; /* SRIX4K EEPROM */
    };
    uint64_t uid;                       /* SRIX UID */
    SrixFlag blockFlags;                /* Modified block flags */
    NfcReader *reader;                  /* NFC Reader */
};

/**
 * Get UID from a SRIX4K.
 * @param target pointer to Srix instance where save uid.
 * @return MikaiError.
 */
static MikaiError getUid(Srix *target) {
    /* Get UID as byte array */
    uint8_t uidBytes[SRIX_UID_LENGTH];

    MikaiError error = NfcGetUid(target->reader, uidBytes);
    if (MIKAI_IS_ERROR(error)) {
        return error;
    }

    /* Check manufacturer code (datasheet of SRIX4K and ST25TB04K) */
    if (uidBytes[7] != 0xD0 || uidBytes[6] != 0x02) {
        return MIKAI_ERROR(MIKAI_NFC_ERROR, "invalid tag manufacturer code");
    }

    /* Convert UID to uint64 */
    target->uid = (uint64_t) uidBytes[7] << 56U | (uint64_t) uidBytes[6] << 48U | (uint64_t) uidBytes[5] << 40U |
                  (uint64_t) uidBytes[4] << 32U | (uint64_t) uidBytes[3] << 24U | (uint64_t) uidBytes[2] << 16U |
                  (uint64_t) uidBytes[1] << 8U | (uint64_t) uidBytes[0];

    return MIKAI_NO_ERROR;
}

/**
 * Read all blocka from a SRIX4K.
 * @param target pointer to Srix instance where save EEPROM content.
 * @return MikaiError.
 */
static MikaiError readBlocks(Srix *target) {
    if (!target->reader) {
        return MIKAI_ERROR(MIKAI_SRIX_ERROR, "nfc reader hasn't been initialized");
    }

    for (uint8_t i = 0; i < SRIX4K_BLOCKS; i++) {
        uint8_t readBlock[SRIX_BLOCK_LENGTH];

        MikaiError error = NfcReadBlock(target->reader, (SrixBlock *) readBlock, i);
        if (MIKAI_IS_ERROR(error)) {
            return error;
        }

        target->eeprom[i] = readBlock[0] << 24 | readBlock[1] << 16 | readBlock[2] << 8 | readBlock[3];
    }

    return MIKAI_NO_ERROR;
}

/**
 * Write a selected group of blocks on mykey
 * @param target pointer to Srix instance to take the blocks to write.
 * @param groupPointer pointer to array to write.
 * @param groupSize size of array to write.
 * @return MikaiError
 */
static MikaiError srixWriteGroup(Srix *target, uint32_t *groupPointer, uint8_t groupSize) {
    for (uint64_t i = 0; i < groupSize; i++) {
        if (srixFlagGet(&target->blockFlags, groupPointer + i - target->eeprom)) {
            const uint8_t writeBlock[] = {
                    groupPointer[i] >> 24,
                    groupPointer[i] >> 16,
                    groupPointer[i] >> 8,
                    groupPointer[i]
            };

            MikaiError error = NfcWriteBlock(target->reader, (SrixBlock *) writeBlock,
                                             groupPointer + i - target->eeprom);
            if (MIKAI_IS_ERROR(error)) {
                return error;
            }
        }
    }

    return MIKAI_NO_ERROR;
}

Srix *SrixNew() {
    Srix *created = malloc(sizeof(Srix));
    if (!created) {
        return (void *) 0;
    }

    created->reader = NfcReaderNew();

    return created;
}

void SrixDelete(Srix target[static 1]) {
    NfcCloseReader(target->reader);
    free(target->reader);
    free(target);
}

size_t NfcGetReadersCount(Srix target[static 1]) {
    return NfcUpdateReaders(target->reader);
}

char *NfcGetDescription(Srix *target, int reader) {
    return NfcGetReaderDescription(target->reader, reader);
}

MikaiError SrixNfcInit(Srix target[static 1], int reader) {
    target->blockFlags = SRIX_FLAG_INIT;
    NfcCloseReader(target->reader);
    NfcInitReader(target->reader, reader);

    /* Get SRIX4K UID & EEPROM */
    MikaiError error = getUid(target);
    if (MIKAI_IS_ERROR(error)) {
        NfcCloseReader(target->reader);
        return error;
    }

    error = readBlocks(target);
    if (MIKAI_IS_ERROR(error)) {
        NfcCloseReader(target->reader);
    }

    return error;
}

void SrixMemoryInit(Srix target[static 1], uint32_t eeprom[const static SRIX4K_BLOCKS], uint64_t uid) {
    if (target->lockable[0] != 0) {
        /* If memory has been already initialized (Key ID not null), copy only generic blocks */
        memcpy(target->generic, eeprom + 16, SRIX4K_BYTES - 16 * SRIX_BLOCK_LENGTH);

        /* If already initialized, flag all generic blocks */
        for (uint8_t i = 16; i < 128; i++) {
            srixFlagAdd(&target->blockFlags, i);
        }
    } else {
        memcpy(target->eeprom, eeprom, SRIX4K_BLOCKS * SRIX_BLOCK_LENGTH);
    }

    target->uid = uid;
}

uint64_t SrixGetUid(Srix target[static 1]) {
    return target->uid;
}

uint32_t *SrixGetBlock(Srix target[static 1], uint8_t blockNum) {
    return (blockNum < SRIX4K_BLOCKS) ? (target->eeprom + blockNum) : 0;
}

void SrixModifyBlock(Srix target[static 1], const uint32_t block, const uint8_t blockNum) {
    target->eeprom[blockNum] = block;
    srixFlagAdd(&target->blockFlags, blockNum);
}

MikaiError SrixWriteBlocks(Srix target[static 1]) {
    if (!target->reader) {
        return MIKAI_ERROR(MIKAI_SRIX_ERROR, "NFC reader hasn't been initialized");
    }

    /* Counter blocks */
    MikaiError error = srixWriteGroup(target, target->counter, sizeof(target->counter) / sizeof(uint32_t));
    if (MIKAI_IS_ERROR(error)) {
        return error;
    }

    /* OTP blocks */
    error = srixWriteGroup(target, target->otp, sizeof(target->otp) / sizeof(uint32_t));
    if (MIKAI_IS_ERROR(error)) {
        return error;
    }

    /* Lockable blocks */
    error = srixWriteGroup(target, target->lockable, sizeof(target->lockable) / sizeof(uint32_t));
    if (MIKAI_IS_ERROR(error)) {
        return error;
    }

    /* Generic blocks */
    error = srixWriteGroup(target, target->generic, sizeof(target->generic) / sizeof(uint32_t));
    if (MIKAI_IS_ERROR(error)) {
        return error;
    }

    target->blockFlags = SRIX_FLAG_INIT;
    return MIKAI_NO_ERROR;
}