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

#include "engines/stark/resources/layer.h"

#include "engines/stark/resources/item.h"
#include "engines/stark/xrcreader.h"

#include "common/debug.h"

namespace Stark {

Resource *Layer::construct(Resource *parent, byte subType, uint16 index, const Common::String &name) {
	switch (subType) {
	case kLayer2D:
		return new Layer2D(parent, subType, index, name);
	case kLayer3D:
		return new Layer3D(parent, subType, index, name);
	default:
		error("Unknown layer subtype %d", subType);
	}
}

Layer::~Layer() {
}

Layer::Layer(Resource *parent, byte subType, uint16 index, const Common::String &name) :
		Resource(parent, subType, index, name),
		_field_44(1.0),
		_field_50(1) {
	_type = TYPE;
}

void Layer::readData(XRCReadStream *stream) {
	_field_44 = stream->readFloat();
	if (_field_44 > 10.0 || _field_44 < -1.0)
		_field_44 = 0;
}

void Layer::printData() {
	debug("field_44: %f", _field_44);
	debug("field_50: %d", _field_50);
}

Layer2D::~Layer2D() {
}

Layer2D::Layer2D(Resource *parent, byte subType, uint16 index, const Common::String &name) :
		Layer(parent, subType, index, name) {
}

void Layer2D::readData(XRCReadStream *stream) {
	Layer::readData(stream);

	uint32 itemsCount = stream->readUint32LE();
	for (uint i = 0; i < itemsCount; i++) {
		uint32 itemIndex = stream->readUint32LE();
		_itemIndices.push_back(itemIndex);
	}

	_field_50 = stream->readUint32LE();
}

RenderEntryArray Layer2D::listRenderEntries() {
	// TODO
	return RenderEntryArray();
}

void Layer2D::printData() {
	Layer::printData();
}

Layer3D::~Layer3D() {
}

Layer3D::Layer3D(Resource *parent, byte subType, uint16 index, const Common::String &name) :
		Layer(parent, subType, index, name),
		_field_54(1),
		_field_58(75),
		_nearClipPlane(100.0),
		_farClipPlane(64000.0) {
}

void Layer3D::readData(XRCReadStream *stream) {
	Layer::readData(stream);

	_field_54 = stream->readUint32LE();
	_nearClipPlane = stream->readFloat();
	_farClipPlane = stream->readFloat();
	if (stream->isDataLeft()) {
		_field_58 = stream->readUint32LE();
	}
}

void Layer3D::onAllLoaded() {
	Layer::onAllLoaded();

	_items = listChildren<Item>();
}

RenderEntryArray Layer3D::listRenderEntries() {
	RenderEntryArray renderEntries;

	for (uint i = 0; i < _items.size(); i++) {
		Item *item = _items[i];

		if (item->getSubType() != Item::kItemSub8) {
			RenderEntry *renderEntry = item->getRenderEntry();

			if (!renderEntry) {
				// warning("No render entry for item '%s'", item->getName().c_str());
				continue;
			}

			renderEntries.push_back(renderEntry);
		}
	}

	return renderEntries;
}

void Layer3D::printData() {
	Layer::printData();

	debug("field_54: %d", _field_54);
	debug("field_58: %d", _field_58);
	debug("nearClipPlane: %f", _nearClipPlane);
	debug("farClipPlane: %f", _farClipPlane);
}

} // End of namespace Stark
