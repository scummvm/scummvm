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

#include "engines/stark/formats/xrc.h"

#include "engines/stark/resources/camera.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/light.h"

#include "engines/stark/services/services.h"
#include "engines/stark/services/stateprovider.h"

#include "engines/stark/scene.h"

#include "common/debug.h"

namespace Stark {
namespace Resources {

Object *Layer::construct(Object *parent, byte subType, uint16 index, const Common::String &name) {
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

Layer::Layer(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name),
		_scrollScale(1.0),
		_enabled(true) {
	_type = TYPE;
}

void Layer::readData(Formats::XRCReadStream *stream) {
	_scrollScale = stream->readFloatLE();
	if (_scrollScale > 10.0 || _scrollScale < -1.0)
		_scrollScale = 0;
}

void Layer::printData() {
	debug("scrollScale: %f", _scrollScale);
	debug("enabled: %d", _enabled);
}

void Layer::setScrollPosition(const Common::Point &position) {
	// The location scroll position is scaled to create a parallax effect
	_scroll.x = (_scrollScale + 1.0) * (float) position.x;
	_scroll.y = (_scrollScale + 1.0) * (float) position.y;
}

Common::Point Layer::getScroll() const {
	return _scroll;
}

void Layer::setScroll(const Common::Point &scroll) {
	_scroll = scroll;
}

bool Layer::isEnabled() const {
	return _enabled;
}

void Layer::enable(bool enabled) {
	_enabled = enabled;
}

Gfx::LightEntryArray Layer::listLightEntries() {
	Common::Array<Light *> lights = listChildren<Light>();

	Gfx::LightEntryArray lightEntries;
	for (uint i = 0; i < lights.size(); i++) {
		lightEntries.push_back(lights[i]->getLightEntry());
	}

	return lightEntries;
}

void Layer::saveLoad(ResourceSerializer *serializer) {
	serializer->syncAsSint32LE(_enabled);
}

void Layer::saveLoadCurrent(ResourceSerializer *serializer) {
	serializer->syncAsSint32LE(_scroll.x);
	serializer->syncAsSint32LE(_scroll.y);
}

Layer2D::~Layer2D() {
}

Layer2D::Layer2D(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Layer(parent, subType, index, name) {
}

void Layer2D::readData(Formats::XRCReadStream *stream) {
	Layer::readData(stream);

	uint32 itemsCount = stream->readUint32LE();
	for (uint i = 0; i < itemsCount; i++) {
		uint32 itemIndex = stream->readUint32LE();
		_itemIndices.push_back(itemIndex);
	}

	_enabled = stream->readBool();
}

void Layer2D::onEnterLocation() {
	Layer::onEnterLocation();

	Common::Array<Item *> items = listChildren<Item>();

	// Build the item list in the appropriate order
	_items.clear();
	for (uint i = 0; i < _itemIndices.size(); i++) {
		for (uint j = 0; j < items.size(); j++) {
			if (items[j]->getIndex() == _itemIndices[i]) {
				_items.push_back(items[j]);
				break;
			}
		}
	}
}

void Layer2D::onExitLocation() {
	Layer::onExitLocation();

	_items.clear();
}

Gfx::RenderEntryArray Layer2D::listRenderEntries() {
	Gfx::RenderEntryArray renderEntries;
	for (uint i = 0; i < _items.size(); i++) {
		Item *item = _items[i];

		Gfx::RenderEntry *renderEntry = item->getRenderEntry(_scroll);

		if (!renderEntry) {
			// warning("No render entry for item '%s'", item->getName().c_str());
			continue;
		}

		renderEntries.push_back(renderEntry);
	}

	return renderEntries;
}

void Layer2D::printData() {
	Layer::printData();
}

Layer3D::~Layer3D() {
}

Layer3D::Layer3D(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Layer(parent, subType, index, name),
		_shouldRenderShadows(true),
		_maxShadowLength(75),
		_nearClipPlane(100.0),
		_farClipPlane(64000.0),
		_backgroundItem(nullptr) {
}

void Layer3D::readData(Formats::XRCReadStream *stream) {
	Layer::readData(stream);

	_shouldRenderShadows = stream->readBool();
	_nearClipPlane = stream->readFloatLE();
	_farClipPlane = stream->readFloatLE();
	if (stream->isDataLeft()) {
		_maxShadowLength = stream->readUint32LE();
	}
}

void Layer3D::onAllLoaded() {
	Layer::onAllLoaded();

	_items = listChildren<Item>();
	_backgroundItem = findChildWithSubtype<Item>(Item::kItemBackground);

	Camera *camera = findChild<Camera>();
	camera->setClipPlanes(_nearClipPlane, _farClipPlane);
}

void Layer3D::onEnterLocation() {
	Layer::onEnterLocation();

	StarkScene->setupShadows(_shouldRenderShadows, _maxShadowLength / 1000.0f);
}

Gfx::RenderEntry *Layer3D::getBackgroundRenderEntry() {
	if (!_backgroundItem) {
		return nullptr;
	}

	return _backgroundItem->getRenderEntry(_scroll);
}

Gfx::RenderEntryArray Layer3D::listRenderEntries() {
	// Sort the items by distance to the camera
	Gfx::RenderEntryArray itemEntries;
	for (uint i = 0; i < _items.size(); i++) {
		Item *item = _items[i];

		if (item->getSubType() != Item::kItemBackground) {
			Gfx::RenderEntry *renderEntry = item->getRenderEntry(_scroll);

			if (!renderEntry) {
				// warning("No render entry for item '%s'", item->getName().c_str());
				continue;
			}

			itemEntries.push_back(renderEntry);
		}
	}
	Common::sort(itemEntries.begin(), itemEntries.end(), Gfx::RenderEntry::compare);

	Gfx::RenderEntryArray renderEntries;

	// Add the background render entry to the list first
	Gfx::RenderEntry *backgroundRenderEntry = getBackgroundRenderEntry();
	if (backgroundRenderEntry) {
		renderEntries.push_back(backgroundRenderEntry);
	}

	// Add the other items
	renderEntries.push_back(itemEntries);

	return renderEntries;
}

void Layer3D::printData() {
	Layer::printData();

	debug("shouldRenderShadows: %d", _shouldRenderShadows);
	debug("maxShadowLength: %d", _maxShadowLength);
	debug("nearClipPlane: %f", _nearClipPlane);
	debug("farClipPlane: %f", _farClipPlane);
}

} // End of namespace Resources
} // End of namespace Stark
