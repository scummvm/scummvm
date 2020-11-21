//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#include <string.h>
#include "core/types.h"
#include "ac/dynobj/cc_agsdynamicobject.h"
#include "ac/common.h"               // quit()
#include "util/bbop.h"

using namespace AGS::Common;

// *** The script serialization routines for built-in types

int AGSCCDynamicObject::Dispose(const char *address, bool force) {
    // cannot be removed from memory
    return 0;
}

void AGSCCDynamicObject::StartSerialize(char *sbuffer) {
    bytesSoFar = 0;
    serbuffer = sbuffer;
}

void AGSCCDynamicObject::SerializeInt(int val) {
    char *chptr = &serbuffer[bytesSoFar];
    int *iptr = (int*)chptr;
    *iptr = BBOp::Int32FromLE(val);
    bytesSoFar += 4;
}

void AGSCCDynamicObject::SerializeFloat(float val) {
    char *chptr = &serbuffer[bytesSoFar];
    float *fptr = (float*)chptr;
    *fptr = BBOp::FloatFromLE(val);
    bytesSoFar += 4;
}

int AGSCCDynamicObject::EndSerialize() {
    return bytesSoFar;
}

void AGSCCDynamicObject::StartUnserialize(const char *sbuffer, int pTotalBytes) {
    bytesSoFar = 0;
    totalBytes = pTotalBytes;
    serbuffer = (char*)sbuffer;
}

int AGSCCDynamicObject::UnserializeInt() {
    if (bytesSoFar >= totalBytes)
        quit("Unserialise: internal error: read past EOF");

    char *chptr = &serbuffer[bytesSoFar];
    bytesSoFar += 4;
    return BBOp::Int32FromLE(*((int*)chptr));
}

float AGSCCDynamicObject::UnserializeFloat() {
    if (bytesSoFar >= totalBytes)
        quit("Unserialise: internal error: read past EOF");

    char *chptr = &serbuffer[bytesSoFar];
    bytesSoFar += 4;
    return BBOp::FloatFromLE(*((float*)chptr));
}

const char* AGSCCDynamicObject::GetFieldPtr(const char *address, intptr_t offset)
{
    return address + offset;
}

void AGSCCDynamicObject::Read(const char *address, intptr_t offset, void *dest, int size)
{
    memcpy(dest, address + offset, size);
}

uint8_t AGSCCDynamicObject::ReadInt8(const char *address, intptr_t offset)
{
    return *(uint8_t*)(address + offset);
}

int16_t AGSCCDynamicObject::ReadInt16(const char *address, intptr_t offset)
{
    return *(int16_t*)(address + offset);
}

int32_t AGSCCDynamicObject::ReadInt32(const char *address, intptr_t offset)
{
    return *(int32_t*)(address + offset);
}

float AGSCCDynamicObject::ReadFloat(const char *address, intptr_t offset)
{
    return *(float*)(address + offset);
}

void AGSCCDynamicObject::Write(const char *address, intptr_t offset, void *src, int size)
{
    memcpy((void*)(address + offset), src, size);
}

void AGSCCDynamicObject::WriteInt8(const char *address, intptr_t offset, uint8_t val)
{
    *(uint8_t*)(address + offset) = val;
}

void AGSCCDynamicObject::WriteInt16(const char *address, intptr_t offset, int16_t val)
{
    *(int16_t*)(address + offset) = val;
}

void AGSCCDynamicObject::WriteInt32(const char *address, intptr_t offset, int32_t val)
{
    *(int32_t*)(address + offset) = val;
}

void AGSCCDynamicObject::WriteFloat(const char *address, intptr_t offset, float val)
{
    *(float*)(address + offset) = val;
}
