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
//
// A stub class for "managing" static global objects exported to script.
// This may be temporary solution (oh no, not again :P) that could be
// replaced by the use of dynamic objects in the future.
//
//=============================================================================
#ifndef __AGS_EE_STATOBJ__STATICOBJECT_H
#define __AGS_EE_STATOBJ__STATICOBJECT_H

#include "core/types.h"

struct ICCStaticObject {
    virtual ~ICCStaticObject() = default;

    // Legacy support for reading and writing object values by their relative offset
    virtual const char* GetFieldPtr(const char *address, intptr_t offset)           = 0;
    virtual void    Read(const char *address, intptr_t offset, void *dest, int size)= 0;
    virtual uint8_t ReadInt8(const char *address, intptr_t offset)                  = 0;
    virtual int16_t ReadInt16(const char *address, intptr_t offset)                 = 0;
    virtual int32_t ReadInt32(const char *address, intptr_t offset)                 = 0;
    virtual float   ReadFloat(const char *address, intptr_t offset)                 = 0;
    virtual void    Write(const char *address, intptr_t offset, void *src, int size)= 0;
    virtual void    WriteInt8(const char *address, intptr_t offset, uint8_t val)    = 0;
    virtual void    WriteInt16(const char *address, intptr_t offset, int16_t val)   = 0;
    virtual void    WriteInt32(const char *address, intptr_t offset, int32_t val)   = 0;
    virtual void    WriteFloat(const char *address, intptr_t offset, float val)     = 0;
};

#endif // __AGS_EE_STATOBJ__STATICOBJECT_H
