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

#include <memory.h>
#include "scriptuserobject.h"

// return the type name of the object
const char *ScriptUserObject::GetType()
{
    return "UserObject";
}

ScriptUserObject::ScriptUserObject()
    : _size(0)
    , _data(nullptr)
{
}

ScriptUserObject::~ScriptUserObject()
{
    delete [] _data;
}

/* static */ ScriptUserObject *ScriptUserObject::CreateManaged(size_t size)
{
    ScriptUserObject *suo = new ScriptUserObject();
    suo->Create(nullptr, size);
    ccRegisterManagedObject(suo, suo);
    return suo;
}

void ScriptUserObject::Create(const char *data, size_t size)
{
    delete [] _data;
    _data = nullptr;

    _size = size;
    if (_size > 0)
    {
        _data = new char[size];
        if (data)
            memcpy(_data, data, _size);
        else
            memset(_data, 0, _size);
    }
}

int ScriptUserObject::Dispose(const char *address, bool force)
{
    delete this;
    return 1;
}

int ScriptUserObject::Serialize(const char *address, char *buffer, int bufsize)
{
    if (_size > bufsize)
        // buffer not big enough, ask for a bigger one
        return -_size;

    memcpy(buffer, _data, _size);
    return _size;
}

void ScriptUserObject::Unserialize(int index, const char *serializedData, int dataSize)
{
    Create(serializedData, dataSize);
    ccRegisterUnserializedObject(index, this, this);
}

const char* ScriptUserObject::GetFieldPtr(const char *address, intptr_t offset)
{
    return _data + offset;
}

void ScriptUserObject::Read(const char *address, intptr_t offset, void *dest, int size)
{
    memcpy(dest, _data + offset, size);
}

uint8_t ScriptUserObject::ReadInt8(const char *address, intptr_t offset)
{
    return *(uint8_t*)(_data + offset);
}

int16_t ScriptUserObject::ReadInt16(const char *address, intptr_t offset)
{
    return *(int16_t*)(_data + offset);
}

int32_t ScriptUserObject::ReadInt32(const char *address, intptr_t offset)
{
    return *(int32_t*)(_data + offset);
}

float ScriptUserObject::ReadFloat(const char *address, intptr_t offset)
{
    return *(float*)(_data + offset);
}

void ScriptUserObject::Write(const char *address, intptr_t offset, void *src, int size)
{
    memcpy((void*)(_data + offset), src, size);
}

void ScriptUserObject::WriteInt8(const char *address, intptr_t offset, uint8_t val)
{
    *(uint8_t*)(_data + offset) = val;
}

void ScriptUserObject::WriteInt16(const char *address, intptr_t offset, int16_t val)
{
    *(int16_t*)(_data + offset) = val;
}

void ScriptUserObject::WriteInt32(const char *address, intptr_t offset, int32_t val)
{
    *(int32_t*)(_data + offset) = val;
}

void ScriptUserObject::WriteFloat(const char *address, intptr_t offset, float val)
{
    *(float*)(_data + offset) = val;
}


// Allocates managed struct containing two ints: X and Y
ScriptUserObject *ScriptStructHelpers::CreatePoint(int x, int y)
{
    ScriptUserObject *suo = ScriptUserObject::CreateManaged(sizeof(int32_t) * 2);
    suo->WriteInt32((const char*)suo, 0, x);
    suo->WriteInt32((const char*)suo, sizeof(int32_t), y);
    return suo;
}
