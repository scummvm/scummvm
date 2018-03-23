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

#ifndef PINK_PAGE_H
#define PINK_PAGE_H

#include <video/flic_decoder.h>
#include "engines/pink/objects/object.h"
#include "engines/pink/objects/module.h"
#include "engines/pink/resource_mgr.h"

namespace Pink {

class Archive;
class Actor;
class LeadActor;

class Page : public NamedObject {
public:

    void load(Archive &archive);
    Actor *findActor(Common::String &name);
    Sound* loadSound(Common::String &fileName);
    CelDecoder *loadCel(Common::String &fileName);

    virtual void toConsole();

protected:
    void init();
    Common::Array<Actor*> _actors;
    ResourceMgr _resMgr;
    LeadActor *_leadActor;

    /*
        int unk_1;
        CString _str;
     */
};

} // End of namespace Pink

#endif