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

#include "harvester/inventory.h"

#include "common/debug.h"
#include "common/endian.h"
#include "graphics/blit.h"
#include "graphics/font.h"
#include "graphics/screen.h"
#include "harvester/detection.h"
#include "harvester/harvester.h"
#include "harvester/resources.h"
#include "harvester/art.h"

namespace Harvester {

namespace {

static const int kInventoryX = 64;
static const int kInventoryY = 48;
static const int kInventoryItemStartX = 73;
static const int kInventoryItemStartY = 115;
static const int kInventoryItemMaxRight = 564;
static const int kInventoryItemSpacing = 5;
static const byte kTransparentPaletteIndex = 0;

struct InventoryCombatLoadoutEntry {
	const char *objectName;
	int loadoutId;
};

struct InventorySecondaryActionEntry {
	const char *objectName;
	const char *actionTag;
	bool closeInventory;
};

static const InventoryCombatLoadoutEntry kInventoryCombatLoadoutMap[] = {
	{ "CLEAVER", 1 },
	{ "NAILGUN", 2 },
	{ "SHOTGUN", 3 },
	{ "9GUN", 4 },
	{ "38GUN", 5 },
	{ "TOMAHAWK", 6 },
	{ "KNIFE", 7 },
	{ "FLAIL", 8 },
	{ "HANDAXE", 9 },
	{ "WRENCH", 10 },
	{ "PITCHFORK", 11 },
	{ "SCYTHE", 12 },
	{ "SWORD", 13 },
	{ "CHAINSAW", 14 },
	{ "HARVEST_BLADE", 15 },
	{ "SHOVEL", 16 },
	{ "FIREAXE", 17 },
	{ "BAT", 18 },
	{ "RAZOR", 19 },
	{ "POOLSTICK", 20 }
};

// Recovered from native run_inventory_screen: right-clicked document/photo items dispatch
// hardcoded closeup action tags instead of using their object records directly.
static const InventorySecondaryActionEntry kInventorySecondaryActionMap[] = {
	{ "NOTE_PHOTOCOPY", "GO_BOYLCOPYCU", true },
	{ "PHOTO_OF_WHALEY_HERRILL_PHOTOCOPY", "GO_BROOMCPYCU", true },
	{ "PHOTO_OF_WHALEY_HERRILL", "GO_BROOMPICCU", true },
	{ "CASKET_PHOTOCOPY", "GO_CASKTCPYCU", true },
	{ "CASKET_PHOTO", "GO_CASKTPICCU", true },
	{ "LODGE_APPLICATION", "GO_LODGAPP1CU", true },
	{ "COMPLETED_LODGE_APPLICATION", "GO_LODGAPP2CU", true },
	{ "MEAT_PERMISSION0", "GO_PERMIT1CU", true },
	{ "MEAT_PERMISSION", "GO_PERMIT2CU", true },
	{ "CHECKBOOK", "GO_REGISTERCU", true },
	{ "CHECKBOOK_PHOTOCOPY", "GO_RGSTRCPYCU", true },
	{ "SAFEBOOK", "GO_SAFEBOOKCU", true },
	{ "PATROL_SCHED", "GO_SCHEDULECU", true },
	{ "TV_DEED", "GO_TVDEED1CU", true },
	{ "TV_DEED_PHOTOCOPY", "GO_TVDEED2CU", true },
	{ "INVITE", "GO_INVITECU", true },
	{ "CLUE", "GOTO_CLUE_CU", true },
	{ "AUTOGRAPH", "GO_AUTOGRPHCU", true }
};

static const char *const kInventoryObjectActionItems[] = {
	"SANDWICH",
	"SANDWICH2",
	"SYRINGE",
	"ST_ASPRIN",
	"ST_COUGHM",
	"ST_VITAMN"
};

static void blitBitmap(Graphics::Screen &screen, const IndexedBitmap &bitmap, int x, int y) {
	if (!bitmap.isValid())
		return;

	int destX = x;
	int destY = y;
	int srcX = 0;
	int srcY = 0;
	int width = (int)bitmap.width;
	int height = (int)bitmap.height;

	if (destX < 0) {
		srcX = -destX;
		width += destX;
		destX = 0;
	}
	if (destY < 0) {
		srcY = -destY;
		height += destY;
		destY = 0;
	}
	if (destX >= screen.w || destY >= screen.h || width <= 0 || height <= 0)
		return;

	width = MIN<int>(width, screen.w - destX);
	height = MIN<int>(height, screen.h - destY);
	if (width <= 0 || height <= 0)
		return;

	const byte *src = bitmap.pixels.data() + srcY * bitmap.width + srcX;
	byte *dst = (byte *)screen.getBasePtr(destX, destY);
	Graphics::keyBlit(dst, src, screen.pitch, bitmap.width, width, height,
		screen.format.bytesPerPixel, kTransparentPaletteIndex);
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

static bool resolveInventoryCombatLoadoutId(const Common::String &objectName, int &loadoutId) {
	loadoutId = 0;
	if (objectName.empty())
		return false;

	for (const InventoryCombatLoadoutEntry &entry : kInventoryCombatLoadoutMap) {
		if (objectName.equalsIgnoreCase(entry.objectName)) {
			loadoutId = entry.loadoutId;
			return true;
		}
	}

	return false;
}

static bool resolveInventorySecondaryActionEntry(const Common::String &objectName,
		InventorySecondaryAction &action) {
	action = InventorySecondaryAction();
	if (objectName.empty())
		return false;

	for (const InventorySecondaryActionEntry &entry : kInventorySecondaryActionMap) {
		if (!objectName.equalsIgnoreCase(entry.objectName))
			continue;

		action.actionTag = entry.actionTag;
		action.closeInventory = entry.closeInventory;
		return true;
	}

	return false;
}

static bool usesObjectActionForInventorySecondaryClick(const Common::String &objectName) {
	if (objectName.empty())
		return false;

	for (const char *itemName : kInventoryObjectActionItems) {
		if (objectName.equalsIgnoreCase(itemName))
			return true;
	}

	return false;
}

static void debugLogInventoryVisual(const StartupInventoryVisual &visual, const Common::String &spritePath) {
	debugC(1, kDebugInventory,
		"Harvester: inventory visual object='%s' sprite='%s' alt='%s' chosen='%s' bounds=(%d,%d)-(%d,%d) action='%s' owner='%s' text='%s'",
		visual.object.objectName.c_str(), visual.object.spritePath.c_str(), visual.object.altSpritePath.c_str(),
		spritePath.c_str(), visual.bounds.left, visual.bounds.top, visual.bounds.right, visual.bounds.bottom,
		visual.object.actionTag.c_str(), visual.object.currentOwnerOrRoom.c_str(),
		visual.object.inventoryTextKey.c_str());
}

} // End of anonymous namespace

InventorySystem::InventorySystem(HarvesterEngine &engine) : _engine(engine) {
}

bool InventorySystem::refresh() {
	_items.clear();

	Script *startupScript = _engine.getStartupScript();
	ResourceManager *resources = _engine.getResources();
	if (!startupScript || !resources)
		return false;

	_lastPlayerHitPoints = startupScript->getPlayerCurrentHitPoints();

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

		const Common::String spritePath = resolveSceneObjectSpritePath(inventoryObject);
		if (!spritePath.empty() && loadBitmapResource(*resources, spritePath, visual.bitmap)) {
			visual.hasBitmap = true;
			if (isStatusObject(inventoryObject)) {
				visual.bounds = Common::Rect(visual.object.currentX, visual.object.currentY,
					visual.object.currentX + visual.bitmap.width, visual.object.currentY + visual.bitmap.height);
				_items.push_back(visual);
				continue;
			}

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

		if (isStatusObject(inventoryObject)) {
			_items.push_back(visual);
			debugLogInventoryVisual(visual, spritePath);
			continue;
		}

		_items.push_back(visual);
		debugLogInventoryVisual(visual, spritePath);
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

bool InventorySystem::open() {
	const bool wasOpen = _open;
	_open = true;
	_promptText.clear();
	return refresh() || !wasOpen;
}

bool InventorySystem::close() {
	if (!_open)
		return false;

	_open = false;
	return true;
}

bool InventorySystem::clearSelection() {
	if (_selectedItemName.empty() && _promptText.empty())
		return false;

	_selectedItemName.clear();
	_promptText.clear();
	return true;
}

bool InventorySystem::refreshIfRuntimeStateChanged() {
	Script *startupScript = _engine.getStartupScript();
	if (!startupScript)
		return false;

	const int currentHitPoints = startupScript->getPlayerCurrentHitPoints();
	if (currentHitPoints == _lastPlayerHitPoints)
		return false;

	return refresh();
}

bool InventorySystem::isOpen() const {
	return _open;
}

bool InventorySystem::hasSelection() const {
	return !_selectedItemName.empty();
}

const Common::String &InventorySystem::getSelectedItemName() const {
	return _selectedItemName;
}

Common::String InventorySystem::resolveSelectedLabel() const {
	Script *startupScript = _engine.getStartupScript();
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

Common::String InventorySystem::buildSelectedPrompt(const Common::String &targetLabel) const {
	return buildUseItemPrompt(resolveSelectedLabel(), targetLabel);
}

void InventorySystem::selectItem(const Common::String &objectName) {
	_selectedItemName = objectName;
}

bool InventorySystem::toggleCombatLoadout(const StartupObjectRecord &object, bool &changed) {
	changed = false;

	int loadoutId = 0;
	if (!resolveInventoryCombatLoadoutId(object.objectName, loadoutId))
		return false;

	Script *startupScript = _engine.getStartupScript();
	if (!startupScript)
		return false;

	const int nextLoadout = startupScript->getPlayerCombatLoadout() == loadoutId ? 0 : loadoutId;
	changed = startupScript->setPlayerCombatLoadout(nextLoadout);
	debugC(1, kDebugInventory,
		"Harvester: inventory combat toggle object='%s' requested_loadout=%d changed=%d resulting_loadout=%d",
		object.objectName.c_str(), nextLoadout, changed, startupScript->getPlayerCombatLoadout());
	return true;
}

bool InventorySystem::resolveSecondaryAction(const StartupObjectRecord &object,
		InventorySecondaryAction &action) const {
	if (resolveInventorySecondaryActionEntry(object.objectName, action)) {
		debugC(1, kDebugInventory,
			"Harvester: inventory secondary action object='%s' resolved hardcoded action='%s' closeInventory=%d",
			object.objectName.c_str(), action.actionTag.c_str(), action.closeInventory);
		return true;
	}

	action = InventorySecondaryAction();
	if (object.actionTag.empty() || !usesObjectActionForInventorySecondaryClick(object.objectName)) {
		debugC(1, kDebugInventory,
			"Harvester: inventory secondary action object='%s' has no supported action (objectAction='%s' supported=%d)",
			object.objectName.c_str(), object.actionTag.c_str(),
			usesObjectActionForInventorySecondaryClick(object.objectName));
		return false;
	}

	action.actionTag = object.actionTag;
	debugC(1, kDebugInventory,
		"Harvester: inventory secondary action object='%s' falling back to object action='%s'",
		object.objectName.c_str(), action.actionTag.c_str());
	return true;
}

void InventorySystem::setPromptText(const Common::String &promptText) {
	_promptText = promptText;
}

const Common::String &InventorySystem::getPromptText() const {
	return _promptText;
}

const StartupInventoryVisual *InventorySystem::findItemAtPoint(const Common::Point &point) const {
	for (int i = (int)_items.size() - 1; i >= 0; --i) {
		if (_items[i].bounds.contains(point))
			return &_items[i];
	}

	return nullptr;
}

Common::Rect InventorySystem::getPanelBounds() const {
	const Art *art = _engine.getStartupArt();
	if (!art)
		return Common::Rect();

	const IndexedBitmap &bitmap = art->getInventoryBitmap();
	return Common::Rect(kInventoryX, kInventoryY,
		kInventoryX + (int)bitmap.width, kInventoryY + (int)bitmap.height);
}

void InventorySystem::drawSelectedDragItem(Graphics::Screen &screen, const Common::Point &point) const {
	if (_selectedItemName.empty())
		return;

	for (const StartupInventoryVisual &item : _items) {
		if (!item.object.objectName.equalsIgnoreCase(_selectedItemName))
			continue;
		if (!item.hasBitmap || !item.bitmap.isValid())
			return;

		const int drawX = point.x - (int)item.bitmap.width / 2;
		const int drawY = point.y - (int)item.bitmap.height / 2;
		blitBitmap(screen, item.bitmap, drawX, drawY);
		return;
	}
}

void InventorySystem::drawOverlay(Graphics::Screen &screen) const {
	const Art *art = _engine.getStartupArt();
	if (!art)
		return;

	blitBitmap(screen, art->getInventoryBitmap(), kInventoryX, kInventoryY);

	for (const StartupInventoryVisual &item : _items) {
		if (!_selectedItemName.empty() && item.object.objectName.equalsIgnoreCase(_selectedItemName))
			continue;
		if (item.hasBitmap && item.bitmap.isValid())
			blitBitmap(screen, item.bitmap, item.object.currentX, item.object.currentY);
	}
}

bool InventorySystem::isExitObject(const StartupObjectRecord &object) {
	return object.objectName.equalsIgnoreCase("INV_EXIT");
}

bool InventorySystem::isStatusObject(const StartupObjectRecord &object) {
	return object.objectName.hasPrefixIgnoreCase("INV_STAT");
}

} // End of namespace Harvester
