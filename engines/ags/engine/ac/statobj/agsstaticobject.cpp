
#include <string.h>
#include "ac/statobj/agsstaticobject.h"
#include "ac/game.h"
#include "ac/gamestate.h"

AGSStaticObject GlobalStaticManager;
StaticGame      GameStaticManager;

const char* AGSStaticObject::GetFieldPtr(const char *address, intptr_t offset)
{
    return address + offset;
}

void AGSStaticObject::Read(const char *address, intptr_t offset, void *dest, int size)
{
    memcpy(dest, address + offset, size);
}

uint8_t AGSStaticObject::ReadInt8(const char *address, intptr_t offset)
{
    return *(uint8_t*)(address + offset);
}

int16_t AGSStaticObject::ReadInt16(const char *address, intptr_t offset)
{
    return *(int16_t*)(address + offset);
}

int32_t AGSStaticObject::ReadInt32(const char *address, intptr_t offset)
{
    return *(int32_t*)(address + offset);
}

float AGSStaticObject::ReadFloat(const char *address, intptr_t offset)
{
    return *(float*)(address + offset);
}

void AGSStaticObject::Write(const char *address, intptr_t offset, void *src, int size)
{
    memcpy((void*)(address + offset), src, size);
}

void AGSStaticObject::WriteInt8(const char *address, intptr_t offset, uint8_t val)
{
    *(uint8_t*)(address + offset) = val;
}

void AGSStaticObject::WriteInt16(const char *address, intptr_t offset, int16_t val)
{
    *(int16_t*)(address + offset) = val;
}

void AGSStaticObject::WriteInt32(const char *address, intptr_t offset, int32_t val)
{
    *(int32_t*)(address + offset) = val;
}

void AGSStaticObject::WriteFloat(const char *address, intptr_t offset, float val)
{
    *(float*)(address + offset) = val;
}


void StaticGame::WriteInt32(const char *address, intptr_t offset, int32_t val)
{
    if (offset == 4 * sizeof(int32_t))
    { // game.debug_mode
        set_debug_mode(val != 0);
    }
    else if (offset == 99 * sizeof(int32_t) || offset == 112 * sizeof(int32_t))
    { // game.text_align, game.speech_text_align
        *(int32_t*)(address + offset) = ReadScriptAlignment(val);
    }
    else
    {
        *(int32_t*)(address + offset) = val;
    }
}
