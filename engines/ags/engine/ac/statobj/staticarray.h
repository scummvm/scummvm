/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef _AGS_ENGINE_AC_STATICOBJ_STATICARRAY_H
#define _AGS_ENGINE_AC_STATICOBJ_STATICARRAY_H

#include "ac/statobj/staticobject.h"

struct ICCDynamicObject;

struct StaticArray : public ICCStaticObject {
public:
    ~StaticArray() override = default;

    void Create(int elem_legacy_size, int elem_real_size, int elem_count = -1 /*unknown*/);
    void Create(ICCStaticObject *stcmgr, int elem_legacy_size, int elem_real_size, int elem_count = -1 /*unknown*/);
    void Create(ICCDynamicObject *dynmgr, int elem_legacy_size, int elem_real_size, int elem_count = -1 /*unknown*/);

    inline ICCStaticObject *GetStaticManager() const
    {
        return _staticMgr;
    }
    inline ICCDynamicObject *GetDynamicManager() const
    {
        return _dynamicMgr;
    }
    // Legacy support for reading and writing object values by their relative offset
    virtual const char *GetElementPtr(const char *address, intptr_t legacy_offset);

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

private:
    ICCStaticObject     *_staticMgr;
    ICCDynamicObject    *_dynamicMgr;
    int                 _elemLegacySize;
    int                 _elemRealSize;
    int                 _elemCount;
};

#endif
