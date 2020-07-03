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

#ifndef ULTIMA8_WORLD_CONTAINER_H
#define ULTIMA8_WORLD_CONTAINER_H

#include "ultima/ultima8/world/item.h"
#include "ultima/shared/std/containers.h"

#include "ultima/ultima8/usecode/intrinsics.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class UCList;

class Container : public Item {
	friend class ItemFactory;
	friend class ContainerGump;
	friend class PaperdollGump;
public:
	Container();
	~Container() override;

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE()

	//! Check if an item can be added to the container
	//! \param item The item to check
	//! \param checkwghtvol Need to check weight and volume?
	//! \return true if item can be added, false if not
	virtual bool CanAddItem(Item *item, bool checkwghtvol = false);

	//! Add an item to the container. This does NOT update item.
	//! \param item The item to add
	//! \param checkwghtvol Need to check weight and volume?
	//! \return true if item was added, false if failed
	virtual bool addItem(Item *item, bool checkwghtvol = false);

	//! Remove an item from the container. This does NOT update item.
	//! \param item The item to remove
	//! \return true if succesful, false if item wasn't in container
	virtual bool removeItem(Item *item);

	//! Move an item to the end of the contents list
	//! \param item The item to move
	//! \return true if succesful, false if item isn't in this container
	virtual bool moveItemToEnd(Item *item);

	//! Remove all contents, moving them to this container's
	//! parent. (Or into the world if this container has no parent.)
	//! Note: not yet implemented
	void removeContents();

	//! Destroy all contents.
	void destroyContents();

	//! Set flag on container and all its contents recursively
	void setFlagRecursively(uint32 mask) override;

	//! Search the container for items matching the given loopscript.
	//! \param itemlist The matching items are appended to this list
	//! \param loopscript The loopscript to match items against
	//! \param scriptsize The size (in bytes) of the loopscript
	//! \param recurse If true, search through child-containers too
	void containerSearch(UCList *itemlist, const uint8 *loopscript,
	                     uint32 scriptsize, bool recurse) const;

	//! A simpler search of the container which just gets the
	//! first item with a given shape number, optionally recursively.
	//! \return The first item with that shape, or nullptr if nothing found.
	Item *getFirstItemWithShape(uint16 shapeno, bool recurse);

	//! A simpler search of the container which just gets the
	//! items with a given shape family, optionally recursively.
	//! \return The first item with that shape, or nullptr if nothing found.
	void getItemsWithShapeFamily(Std::vector<Item *> &itemlist, uint16 family, bool recurse);

	//! Get the weight of the container and its contents
	//! \return weight
	uint32 getTotalWeight() const override;

	//! Get the container's capacity
	virtual uint32 getCapacity() const;

	//! Get the total volume used up by the container's current contents
	virtual uint32 getContentVolume() const;

	//! Assign self and contents an objID
	//! \return the assiged ID
	ObjId assignObjId() override;

	//! Clear objIDs of self and contents
	void clearObjId() override;

	//! Destroy self
	void destroy(bool delnow = false) override;

	void dumpInfo() const override;

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

	INTRINSIC(I_removeContents);
	INTRINSIC(I_destroyContents);

protected:
	Std::list<Item *> _contents;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
