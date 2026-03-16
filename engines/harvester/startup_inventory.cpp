/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "harvester/startup_inventory.h"

#include "common/endian.h"
#include "graphics/font.h"
#include "harvester/harvester.h"
#include "harvester/resources.h"
#include "harvester/startup_art.h"

namespace Harvester {

namespace {

static const int kInventoryX = 64;
static const int kInventoryY = 48;
static const int kInventoryItemStartX = 73;
static const int kInventoryItemStartY = 115;
static const int kInventoryItemMaxRight = 564;
static const int kInventoryItemSpacing = 5;
static const byte kShadowColor = 0;
static const byte kRoomPromptColor = 0xce;

static void blitBitmap(Graphics::Screen &screen, const IndexedBitmap &bitmap, int x, int y) {
	if (!bitmap.isValid())
		return;

	screen.copyRectToSurface(bitmap.pixels.data(), bitmap.width, x, y, bitmap.width, bitmap.height);
}

static void drawShadowedString(Graphics::Screen &screen, const Graphics::Font &font, const Common::String &text,
		int x, int y, int width, byte color, Graphics::TextAlign align = Graphics::kTextAlignLeft) {
	font.drawString(&screen, text, x + 1, y + 1, width, kShadowColor, align);
	font.drawString(&screen, text, x, y, width, color, align);
}

static Common::String buildUseItemPrompt(const Common::String &itemLabel, const Common::String &targetLabel) {
	if (itemLabel.empty())
		return Common::String();
	if (targetLabel.empty())
		return Common::String::format("Use %s on ...", itemLabel.c_str());

	return Common::String::format("Use %s on %s", itemLabel.c_str(), targetLabel.c_str());
}

static Common::Rect getHotspotBounds(const StartupObjectRecord &object) {
	if (object.boundsX2 > object.currentX && object.boundsY2 > object.currentY)
		return Common::Rect(object.currentX, object.currentY, object.boundsX2 + 1, object.boundsY2 + 1);

	return Common::Rect();
}

static Common::String resolveSceneObjectSpritePath(const StartupObjectRecord &object) {
	const bool atInitialPlacement = object.currentX == object.initialX &&
		object.currentY == object.initialY &&
		object.currentOwnerOrRoom.equalsIgnoreCase(object.initialOwnerOrRoom);
	if (!object.altSpritePath.empty() &&
		(!atInitialPlacement || object.currentOwnerOrRoom.equalsIgnoreCase("INVENTORY")))
		return object.altSpritePath;

	return object.spritePath;
}

static bool loadBitmapResource(ResourceManager &resources, const Common::String &path, IndexedBitmap &bitmap) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 12)
		return false;

	bitmap = IndexedBitmap();
	bitmap.width = READ_LE_UINT32(data.data());
	bitmap.height = READ_LE_UINT32(data.data() + 4);
	const uint32 pixelCount = bitmap.width * bitmap.height;
	if (bitmap.width == 0 || bitmap.height == 0 || data.size() < 12 + pixelCount)
		return false;

	bitmap.pixels.resize(pixelCount);
	memcpy(bitmap.pixels.data(), data.data() + 12, pixelCount);
	return true;
}

} // End of anonymous namespace

StartupInventorySystem::StartupInventorySystem(HarvesterEngine &engine) : _engine(engine) {
}

bool StartupInventorySystem::refresh() {
	_items.clear();

	StartupScript *startupScript = _engine.getStartupScript();
	ResourceManager *resources = _engine.getResources();
	if (!startupScript || !resources)
		return false;

	Common::Array<StartupObjectRecord> inventoryObjects;
	startupScript->getVisibleInventoryObjects(inventoryObjects);
	int nextX = kInventoryItemStartX;
	int nextY = kInventoryItemStartY;
	int rowHeight = 0;

	for (const StartupObjectRecord &inventoryObject : inventoryObjects) {
		StartupInventoryVisual visual;
		visual.object = inventoryObject;

		if (isExitObject(inventoryObject)) {
			visual.bounds = getHotspotBounds(inventoryObject);
			_items.push_back(visual);
			continue;
		}
		if (isStatusObject(inventoryObject))
			continue;

		const Common::String spritePath = resolveSceneObjectSpritePath(inventoryObject);
		if (!spritePath.empty() && loadBitmapResource(*resources, spritePath, visual.bitmap)) {
			visual.hasBitmap = true;
			if (nextX + (int)visual.bitmap.width > kInventoryItemMaxRight) {
				nextX = kInventoryItemStartX;
				nextY += rowHeight + kInventoryItemSpacing;
				rowHeight = 0;
			}

			visual.object.currentX = nextX;
			visual.object.currentY = nextY;
			visual.bounds = Common::Rect(nextX, nextY, nextX + visual.bitmap.width, nextY + visual.bitmap.height);
			nextX += visual.bitmap.width + kInventoryItemSpacing;
			rowHeight = MAX<int>(rowHeight, visual.bitmap.height);
		} else {
			visual.bounds = getHotspotBounds(inventoryObject);
		}

		_items.push_back(visual);
	}

	if (_selectedItemName.empty())
		return true;

	for (const StartupInventoryVisual &item : _items) {
		if (item.object.objectName.equalsIgnoreCase(_selectedItemName))
			return true;
	}

	_selectedItemName.clear();
	_promptText.clear();
	return true;
}

bool StartupInventorySystem::open() {
	const bool wasOpen = _open;
	_open = true;
	_promptText.clear();
	return refresh() || !wasOpen;
}

bool StartupInventorySystem::close() {
	if (!_open)
		return false;

	_open = false;
	return true;
}

bool StartupInventorySystem::clearSelection() {
	if (_selectedItemName.empty() && _promptText.empty())
		return false;

	_selectedItemName.clear();
	_promptText.clear();
	return true;
}

bool StartupInventorySystem::isOpen() const {
	return _open;
}

bool StartupInventorySystem::hasSelection() const {
	return !_selectedItemName.empty();
}

const Common::String &StartupInventorySystem::getSelectedItemName() const {
	return _selectedItemName;
}

Common::String StartupInventorySystem::resolveSelectedLabel() const {
	StartupScript *startupScript = _engine.getStartupScript();
	if (!startupScript || _selectedItemName.empty())
		return Common::String();

	for (const StartupInventoryVisual &item : _items) {
		if (item.object.objectName.equalsIgnoreCase(_selectedItemName))
			return startupScript->resolveObjectLabel(item.object);
	}

	Common::Array<StartupObjectRecord> inventoryObjects;
	startupScript->getVisibleInventoryObjects(inventoryObjects);
	for (const StartupObjectRecord &item : inventoryObjects) {
		if (item.objectName.equalsIgnoreCase(_selectedItemName))
			return startupScript->resolveObjectLabel(item);
	}

	return normalizeHarvesterResourcePath(_selectedItemName);
}

Common::String StartupInventorySystem::buildSelectedPrompt(const Common::String &targetLabel) const {
	return buildUseItemPrompt(resolveSelectedLabel(), targetLabel);
}

void StartupInventorySystem::selectItem(const Common::String &objectName) {
	_selectedItemName = objectName;
}

void StartupInventorySystem::setPromptText(const Common::String &promptText) {
	_promptText = promptText;
}

const Common::String &StartupInventorySystem::getPromptText() const {
	return _promptText;
}

const StartupInventoryVisual *StartupInventorySystem::findItemAtPoint(const Common::Point &point) const {
	for (int i = (int)_items.size() - 1; i >= 0; --i) {
		if (_items[i].bounds.contains(point))
			return &_items[i];
	}

	return nullptr;
}

Common::Rect StartupInventorySystem::getPanelBounds() const {
	const StartupArt *art = _engine.getStartupArt();
	if (!art)
		return Common::Rect();

	const IndexedBitmap &bitmap = art->getInventoryBitmap();
	return Common::Rect(kInventoryX, kInventoryY,
		kInventoryX + (int)bitmap.width, kInventoryY + (int)bitmap.height);
}

void StartupInventorySystem::drawOverlay(Graphics::Screen &screen, const Graphics::Font &font) const {
	const StartupArt *art = _engine.getStartupArt();
	StartupScript *startupScript = _engine.getStartupScript();
	if (!art || !startupScript)
		return;

	blitBitmap(screen, art->getInventoryBitmap(), kInventoryX, kInventoryY);

	for (const StartupInventoryVisual &item : _items) {
		if (item.hasBitmap && item.bitmap.isValid())
			blitBitmap(screen, item.bitmap, item.object.currentX, item.object.currentY);
	}

	Common::String overlayPrompt = _promptText;
	if (overlayPrompt.empty() && !_selectedItemName.empty()) {
		for (const StartupInventoryVisual &item : _items) {
			if (item.object.objectName.equalsIgnoreCase(_selectedItemName)) {
				overlayPrompt = buildUseItemPrompt(startupScript->resolveObjectLabel(item.object), Common::String());
				break;
			}
		}
	}

	if (!overlayPrompt.empty())
		drawShadowedString(screen, font, overlayPrompt, 0, 462, 640, kRoomPromptColor, Graphics::kTextAlignCenter);
}

bool StartupInventorySystem::isExitObject(const StartupObjectRecord &object) {
	return object.objectName.equalsIgnoreCase("INV_EXIT");
}

bool StartupInventorySystem::isStatusObject(const StartupObjectRecord &object) {
	return object.objectName.hasPrefixIgnoreCase("INV_STAT");
}

} // End of namespace Harvester
