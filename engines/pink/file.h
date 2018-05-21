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

#ifndef PINK_FILE_H
#define PINK_FILE_H

#include "common/file.h"

namespace Pink {

struct ObjectDescription {
    void load(Common::File &file);

    char name[16];
    uint32 objectsOffset;
    uint32 objectsCount;
    uint32 resourcesOffset;
    uint32 resourcesCount;
};

struct ResourceDescription {
    void load(Common::File &file);

    char name[16];
    uint32 offset;
    uint32 size;
    bool inBro; // in original it is short.
    // Don't know what's better to use.(Perhaps no difference because of padding)
};

class PinkEngine;
class Object;

class OrbFile : public Common::File {
public:
    OrbFile();
    virtual ~OrbFile();
    virtual bool open(const Common::String &name);

    void loadGame(PinkEngine *game);
    void loadObject(Object *obj, const Common::String &name);
    void loadObject(Object *obj, ObjectDescription *objDesc);

    ObjectDescription *getObjDesc(const char *name);
    ResourceDescription *getResDescTable(ObjectDescription *objDesc);

    uint32 getTimestamp();

private:
    void seekToObject(const char *name);

    uint32 _timestamp;
    uint32 _tableOffset;
    uint32 _tableSize;
};

class BroFile : public Common::File {
public:
    virtual bool open(const Common::String &name, uint32 orbTimestamp);
};

} // End of namespace Pink

#endif
