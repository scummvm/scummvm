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

#include "common/debug.h"

#include "engines/stark/resources/item.h"
#include "engines/stark/xrcreader.h"

namespace Stark {

Resource *Item::construct(Resource *parent, byte subType, uint16 index, const Common::String &name) {
	switch (subType) {
	case kItemSub1:
		return new UnimplementedResource(parent, TYPE, subType, index, name);
	case kItemSub2:
		return new UnimplementedResource(parent, TYPE, subType, index, name);
	case kItemSub3:
		return new UnimplementedResource(parent, TYPE, subType, index, name);
	case kItemSub5:
	case kItemSub6:
		return new ItemSub56(parent, subType, index, name);
	case kItemSub7:
	case kItemSub8:
		return new ItemSub78(parent, subType, index, name);
	case kItemSub10:
		return new UnimplementedResource(parent, TYPE, subType, index, name);
	default:
		error("Unknown item subtype %d", subType);
	}
}

Item::~Item() {
}

Item::Item(Resource *parent, byte subType, uint16 index, const Common::String &name) :
				Resource(parent, subType, index, name),
				_field_34(true),
				_field_38(0) {
	_type = TYPE;
}

void Item::readData(XRCReadStream *stream) {
	_field_34 = stream->readBool();
	_field_38 = stream->readSint32LE();
}

void Item::printData() {
	debug("field_34: %d", _field_34);
	debug("field_38: %d", _field_38);
}

ItemVisual::~ItemVisual() {
}

ItemVisual::ItemVisual(Resource *parent, byte subType, uint16 index, const Common::String &name) :
				Item(parent, subType, index, name),
				_field_44(1) {
}

void ItemVisual::readData(XRCReadStream *stream) {
	Item::readData(stream);

	_field_44 = stream->readUint32LE();
}

void ItemVisual::printData() {
	Item::printData();

	debug("field_44: %d", _field_44);
}

ItemSub5610::~ItemSub5610() {
}

ItemSub5610::ItemSub5610(Resource *parent, byte subType, uint16 index, const Common::String &name) :
		ItemVisual(parent, subType, index, name) {
}

ItemSub56::~ItemSub56() {
}

ItemSub56::ItemSub56(Resource *parent, byte subType, uint16 index, const Common::String &name) :
		ItemSub5610(parent, subType, index, name),
		_field_6C(0) {
}

void ItemSub56::readData(XRCReadStream *stream) {
	ItemSub5610::readData(stream);

	_field_6C = stream->readSint32LE();
	_position = stream->readPoint();
}

void ItemSub56::printData() {
	ItemSub5610::printData();

	debug("field_6C: %d", _field_6C);
	debug("position: x %d, y %d", _position.x, _position.y);
}

ItemSub78::~ItemSub78() {
}

ItemSub78::ItemSub78(Resource *parent, byte subType, uint16 index, const Common::String &name) :
		ItemVisual(parent, subType, index, name) {
}

void ItemSub78::readData(XRCReadStream *stream) {
	ItemVisual::readData(stream);

	_reference = stream->readResourceReference();
	_position = stream->readPoint();
}

void ItemSub78::printData() {
	ItemVisual::printData();

	debug("reference: %s", _reference.describe().c_str());
	debug("position: x %d, y %d", _position.x, _position.y);
}

} // End of namespace Stark
