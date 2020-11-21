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

#ifndef __AC_CCDYNAMICOBJECT_H
#define __AC_CCDYNAMICOBJECT_H

#include "ac/dynobj/cc_dynamicobject.h"

struct AGSCCDynamicObject : ICCDynamicObject {
protected:
    virtual ~AGSCCDynamicObject() = default;
public:
    // default implementation
    int Dispose(const char *address, bool force) override;

    // TODO: pass savegame format version
    virtual void Unserialize(int index, const char *serializedData, int dataSize) = 0;

    // Legacy support for reading and writing object values by their relative offset
    const char* GetFieldPtr(const char *address, intptr_t offset) override;
    void    Read(const char *address, intptr_t offset, void *dest, int size) override;
    uint8_t ReadInt8(const char *address, intptr_t offset) override;
    int16_t ReadInt16(const char *address, intptr_t offset) override;
    int32_t ReadInt32(const char *address, intptr_t offset) override;
    float   ReadFloat(const char *address, intptr_t offset) override;
    void    Write(const char *address, intptr_t offset, void *src, int size) override;
    void    WriteInt8(const char *address, intptr_t offset, uint8_t val) override;
    void    WriteInt16(const char *address, intptr_t offset, int16_t val) override;
    void    WriteInt32(const char *address, intptr_t offset, int32_t val) override;
    void    WriteFloat(const char *address, intptr_t offset, float val) override;

protected:
    // Savegame serialization
    // TODO: reimplement with the proper memory stream?!
    int bytesSoFar;
    int totalBytes;
    char *serbuffer;

    void StartSerialize(char *sbuffer);
    void SerializeInt(int val);
    void SerializeFloat(float val);
    int  EndSerialize();
    void StartUnserialize(const char *sbuffer, int pTotalBytes);
    int  UnserializeInt();
    float UnserializeFloat();

};

#endif // __AC_CCDYNAMICOBJECT_H