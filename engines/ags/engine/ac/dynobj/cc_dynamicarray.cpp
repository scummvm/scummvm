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
#include "cc_dynamicarray.h"

// return the type name of the object
const char *CCDynamicArray::GetType() {
    return CC_DYNAMIC_ARRAY_TYPE_NAME;
}

int CCDynamicArray::Dispose(const char *address, bool force) {
    address -= 8;

    // If it's an array of managed objects, release their ref counts;
    // except if this array is forcefully removed from the managed pool,
    // in which case just ignore these.
    if (!force)
    {
        int *elementCount = (int*)address;
        if (elementCount[0] & ARRAY_MANAGED_TYPE_FLAG)
        {
            elementCount[0] &= ~ARRAY_MANAGED_TYPE_FLAG;
            for (int i = 0; i < elementCount[0]; i++)
            {
                if (elementCount[2 + i] != 0)
                {
                    ccReleaseObjectReference(elementCount[2 + i]);
                }
            }
        }
    }

    delete address;
    return 1;
}

// serialize the object into BUFFER (which is BUFSIZE bytes)
// return number of bytes used
int CCDynamicArray::Serialize(const char *address, char *buffer, int bufsize) {
    int *sizeInBytes = &((int*)address)[-1];
    int sizeToWrite = *sizeInBytes + 8;
    if (sizeToWrite > bufsize)
    {
        // buffer not big enough, ask for a bigger one
        return -sizeToWrite;
    }
    memcpy(buffer, address - 8, sizeToWrite);
    return sizeToWrite;
}

void CCDynamicArray::Unserialize(int index, const char *serializedData, int dataSize) {
    char *newArray = new char[dataSize];
    memcpy(newArray, serializedData, dataSize);
    ccRegisterUnserializedObject(index, &newArray[8], this);
}

DynObjectRef CCDynamicArray::Create(int numElements, int elementSize, bool isManagedType)
{
    char *newArray = new char[numElements * elementSize + 8];
    memset(newArray, 0, numElements * elementSize + 8);
    int *sizePtr = (int*)newArray;
    sizePtr[0] = numElements;
    sizePtr[1] = numElements * elementSize;
    if (isManagedType) 
        sizePtr[0] |= ARRAY_MANAGED_TYPE_FLAG;
    void *obj_ptr = &newArray[8];
    int32_t handle = ccRegisterManagedObject(obj_ptr, this);
    if (handle == 0)
    {
        delete[] newArray;
        return DynObjectRef(0, nullptr);
    }
    return DynObjectRef(handle, obj_ptr);
}


const char* CCDynamicArray::GetFieldPtr(const char *address, intptr_t offset)
{
    return address + offset;
}

void CCDynamicArray::Read(const char *address, intptr_t offset, void *dest, int size)
{
    memcpy(dest, address + offset, size);
}

uint8_t CCDynamicArray::ReadInt8(const char *address, intptr_t offset)
{
    return *(uint8_t*)(address + offset);
}

int16_t CCDynamicArray::ReadInt16(const char *address, intptr_t offset)
{
    return *(int16_t*)(address + offset);
}

int32_t CCDynamicArray::ReadInt32(const char *address, intptr_t offset)
{
    return *(int32_t*)(address + offset);
}

float CCDynamicArray::ReadFloat(const char *address, intptr_t offset)
{
    return *(float*)(address + offset);
}

void CCDynamicArray::Write(const char *address, intptr_t offset, void *src, int size)
{
    memcpy((void*)(address + offset), src, size);
}

void CCDynamicArray::WriteInt8(const char *address, intptr_t offset, uint8_t val)
{
    *(uint8_t*)(address + offset) = val;
}

void CCDynamicArray::WriteInt16(const char *address, intptr_t offset, int16_t val)
{
    *(int16_t*)(address + offset) = val;
}

void CCDynamicArray::WriteInt32(const char *address, intptr_t offset, int32_t val)
{
    *(int32_t*)(address + offset) = val;
}

void CCDynamicArray::WriteFloat(const char *address, intptr_t offset, float val)
{
    *(float*)(address + offset) = val;
}

CCDynamicArray globalDynamicArray;


DynObjectRef DynamicArrayHelpers::CreateStringArray(const std::vector<const char*> items)
{
    // NOTE: we need element size of "handle" for array of managed pointers
    DynObjectRef arr = globalDynamicArray.Create(items.size(), sizeof(int32_t), true);
    if (!arr.second)
        return arr;
    // Create script strings and put handles into array
    int32_t *slots = static_cast<int32_t*>(arr.second);
    for (auto s : items)
    {
        DynObjectRef str = stringClassImpl->CreateString(s);
        *(slots++) = str.first;
    }
    return arr;
}
