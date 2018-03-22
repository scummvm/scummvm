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

#ifndef PINK_INVENTORY_H
#define PINK_INVENTORY_H


#include <common/array.h>
#include "engines/pink/objects/object.h"

namespace Pink {


class InventoryItem : public NamedObject {
public:
    virtual void deserialize(Archive &archive);

    virtual void toConsole();

    Common::String &getCurrentOwner();

private:
    Common::String _initialOwner;
    Common::String _currentOwner;
};

class LeadActor;

class InventoryMgr : public Object {
public:
    virtual ~InventoryMgr();
    virtual void deserialize(Archive &archive);

    virtual void toConsole();

    void setLeadActor(LeadActor *lead);
    InventoryItem* findInventoryItem(Common::String &name);

private:
    LeadActor *_lead;
    Common::Array<InventoryItem*> _items;
    // other fields. haven't RE them yet
};

} // End of namespace Pink

#endif
