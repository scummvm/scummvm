#pragma once

#include <os_types.h>

enum DDB_Machine;

bool SCR_GetScreen (const char* fileName, DDB_Machine target, 
                    uint8_t* buffer, size_t bufferSize, 
                    uint8_t* output, int width, int height, uint32_t* palette,
                    bool* isHAM);