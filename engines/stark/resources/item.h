/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_RESOURCES_ITEM_H
#define STARK_RESOURCES_ITEM_H

#include "common/rect.h"
#include "common/str.h"

#include "engines/stark/resources/resource.h"
#include "engines/stark/resourcereference.h"

namespace Stark {

class XRCReadStream;

class Item : public Resource {
public:
	static const ResourceType::Type TYPE = ResourceType::kItem;

	enum SubType {
		kItemSub1 = 1,
		kItemSub2 = 2,
		kItemSub3 = 3,
		kItemSub5 = 5,
		kItemSub6 = 6,
		kItemSub7 = 7,
		kItemSub8 = 8,
		kItemSub10 = 10
	};

	/** Item factory */
	static Resource *construct(Resource *parent, byte subType, uint16 index, const Common::String &name);

	Item(Resource *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Item();

	virtual void readData(XRCReadStream *stream) override;

protected:
	void printData() override;

	bool _field_34;
	int32 _field_38;
};

class ItemVisual : public Item {
public:
	ItemVisual(Resource *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~ItemVisual();

	virtual void readData(XRCReadStream *stream) override;

protected:
	void printData() override;

	uint32 _field_44;
};

class ItemSub5610 : public ItemVisual {
public:
	ItemSub5610(Resource *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~ItemSub5610();
};

class ItemSub56 : public ItemSub5610 {
public:
	ItemSub56(Resource *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~ItemSub56();

	virtual void readData(XRCReadStream *stream) override;

protected:
	void printData() override;

	int32 _field_6C;
	Common::Point _position;
};

class ItemSub78 : public ItemVisual {
public:
	ItemSub78(Resource *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~ItemSub78();

	virtual void readData(XRCReadStream *stream) override;

protected:
	void printData() override;

	ResourceReference _reference;
	Common::Point _position;
};

} // End of namespace Stark

#endif // STARK_RESOURCES_ITEM_H
