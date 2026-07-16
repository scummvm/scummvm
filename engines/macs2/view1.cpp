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

#include "macs2/view1.h"
#include "common/util.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/debug.h"
#include "common/system.h"
#include "engines/enhancements.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/paletteman.h"
#include "macs2/debugtools.h"
#include "macs2/detection.h"
#include "macs2/gameobjects.h"
#include "macs2/macs2.h"
#include "macs2/music.h"
#include "macs2/scummui.h"

namespace Macs2 {
namespace {
constexpr int kNumLoadedCursors = 33;

// Button lookup table at data segment offset 0x26, 1-indexed
// Binary: local_4 = 1..7, each iteration reads *(local_4 * 2 + 0x26) as index into cursor array
const uint16 kLookupTable[8] = {9, 15, 14, 27, 29, 16, 17, 9}; // index 0 unused

// drawAllCharacters (1008:90a2) local_14: animation slot for current orientation.
uint16 resolveAnimSlotIndex(const GameObject *obj) {
	if ((int16)obj->_overloadAnimTriggerDirection < 0 ||
		obj->_overloadAnimTriggerDirection != obj->_orientation) {
		return obj->_orientation;
	}
	return 0x15;
}

Common::String joinDebugStrings(const Common::StringArray &strings) {
	Common::String result;
	for (uint i = 0; i < strings.size(); ++i) {
		if (i != 0)
			result += " | ";
		result += strings[i];
	}
	return result;
}

void logRenderedText(const char *kind, int x, int y, const Common::String &text) {
	Common::U32String u32text(text.c_str(), Common::kDos850);
	Common::String utf8text(u32text);
	g_engine->_textLog.push_back(Common::String::format("%s text at (%d,%d): ", kind, x, y) + utf8text);
}

void resetObjectDrawBounds(GameObject *obj) {
	if (obj != nullptr)
		obj->resetDrawBounds();
}

// Build a screen-clipped erase rect from the previous frame's sprite bounds.
// Returns false when there is nothing on-screen to erase.
bool buildClippedEraseRect(int32 left, int32 top, uint16 width, uint16 height,
						   int32 screenW, int32 screenH, Common::Rect &out) {
	if (width == 0 && height == 0)
		return false;

	// drawAllCharacters @ 1008:90a2: dirty right/bottom are inclusive (+1 padding),
	// Common::Rect uses exclusive right/bottom (+1 more).
	const int32 exclRight = left + (int32)width + 2;
	const int32 exclBottom = top + (int32)height + 2;
	if (exclRight <= 0 || exclBottom <= 0 || left >= screenW || top >= screenH)
		return false;

	const int32 clipLeft = MAX(left, (int32)0);
	const int32 clipTop = MAX(top, (int32)0);
	const int32 clipRight = MIN(exclRight, screenW);
	const int32 clipBottom = MIN(exclBottom, screenH);
	if (clipRight <= clipLeft || clipBottom <= clipTop)
		return false;

	if (clipLeft < -32768 || clipTop < -32768 || clipRight > 32767 || clipBottom > 32767)
		return false;

	// Avoid Common::Rect(x1,y1,x2,y2) constructor (asserts on invalid input).
	out.left = (int16)clipLeft;
	out.top = (int16)clipTop;
	out.right = (int16)clipRight;
	out.bottom = (int16)clipBottom;
	return out.isValidRect() && !out.isEmpty();
}

void buildFadedPalette(byte *colors, const byte *sourcePalette, int fadeValue) {
	// Original fadePaletteToBlack/FromBlack: subtracts fadeValue from raw 6-bit VGA
	// palette values (0-63), clamping to 0. Then scales to 8-bit for ScummVM.
	// Apply palette darkening if active (scenes with _scenePaletteMode != 1).
	uint16 darkenPercent = (g_engine->_scenePaletteMode == 1) ? 0 : g_engine->_paletteDarkenPercent;
	if (darkenPercent > 100)
		darkenPercent = 100;
	uint16 brightnessFactor = 100 - darkenPercent;
	for (uint i = 0; i < 256 * 3; ++i) {
		int raw = (sourcePalette[i] * brightnessFactor) / 100; // darken first
		int faded = raw - fadeValue;
		if (faded < 0)
			faded = 0;
		colors[i] = (faded * 259 + 33) >> 6; // 6-bit to 8-bit
	}
}

} // namespace

const View1::BorderStyle View1::kBorderRaised = {0x1010, 0x1012, 0x1011};
const View1::BorderStyle View1::kBorderPressed = {0x1010, 0x1011, 0x1012};

View1::View1() : UIElement("View1") {
	_backgroundSurface.copyFrom(g_engine->_sceneBackground);
	currentSpeechActData.onRightSide = false;
	updateCursor();
	setViewPaletteSafely(g_engine->_pal);
	_paletteDirty = false;
	CursorMan.showMouse(true);

	// TODO: Check if this works like this
	Character *protagonist = new Character();
	// TODO: Need to properly handle the offset
	// TODO: Remember that the game starts enumerating objects at 1 and not at 0
	protagonist->_gameObject = GameObjects::instance()._objects[0x0];
	_characters.push_back(protagonist);
	rebuildCharacterLookupTable();
	_inventorySource = protagonist->_gameObject;
	_inventoryButtonLocations.resize(6);

	if (hasScummVerbUI()) {
		_scummUI = new ScummUI(this);
		_bounds = Common::Rect(0, 0, kScreenWidth, kScreenHeight);
		_innerBounds = _bounds;
		setInventorySource(_inventorySource);
	}
}

void View1::ensureScummVerbUI() {
	if (!hasScummVerbUI())
		return;
	if (!_scummUI) {
		_scummUI = new ScummUI(this);
		if (_inventorySource)
			setInventorySource(_inventorySource);
	}
	if (_bounds.height() != kScreenHeight) {
		_bounds = Common::Rect(0, 0, kScreenWidth, kScreenHeight);
		_innerBounds = _bounds;
		::initGraphics(kScreenWidth, kScreenHeight);
	}
}

bool View1::hasScummVerbUI() const {
	return g_engine->enhancementEnabled(kEnhUIUX);
}

bool View1::shouldShowScummVerbUI() const {
	if (!hasScummVerbUI())
		return false;
	if (_currentMode == ViewMode::VM_HELP)
		return false;
	if (_isShowingTextBox || _isShowingDialoguePanel)
		return false;
	if (_uiPanelState == kUiPanelContainerInventory || _uiPanelState == kUiPanelSaveLoad)
		return false;
	if (g_engine->_scriptExecutor->_cursorMode == Script::MouseMode::Disabled)
		return false;

	// Use the actor object table directly; Character lookup can lag behind scene changes.
	const GameObject *actor = GameObjects::getObjectByIndex(Scenes::instance()._currentActorIndex);
	if (!actor)
		return false;

	const uint16 scene = (uint16)Scenes::instance()._currentSceneIndex;
	return actor->_sceneIndex == scene;
}

View1::~View1() {
	delete _scummUI;
	for (Character *c : _characters) {
		delete c;
	}
}

void View1::applyPaletteWithFade(const byte *sourcePalette, int fadeValue) {
	byte colors[256 * 3];
	buildFadedPalette(colors, sourcePalette, fadeValue);
	setViewPaletteSafely(colors);
}

void View1::setViewPaletteSafely(const byte *colors) {
	const bool shouldTouchCursor = _cursorSuppressedForFade;
	const bool cursorWasVisible = shouldTouchCursor && CursorMan.isVisible();
	if (cursorWasVisible)
		CursorMan.showMouse(false);

	if (shouldTouchCursor)
		updateCursor(colors);

	g_system->getPaletteManager()->setPalette(colors, 0, 256);

	if (cursorWasVisible)
		CursorMan.showMouse(true);
}

void View1::restoreUiPaletteEntries() {
	// Binary setPaletteRange(0xF0, 0x10, palette+0x30) after applyScenePaletteEffect:
	// push VGA indices 0xF0..0xFF from palette color slots 0x10..0x1F.
	g_system->getPaletteManager()->setPalette(g_engine->_pal + 16 * 3, 0xF0, 16);
}

void View1::openInventory(GameObject *newInventorySource) {
	if (newInventorySource == nullptr) {
		warning("Tried to open inventory for a null source");
		return;
	}

	setInventorySource(newInventorySource);
	_pendingPanelRequest = kPanelRequestNone; // Binary: g_wPendingPanelRequest = 0

	// SCUMM verb UI: protagonist inventory is always visible in the strip.
	if (hasScummVerbUI() && newInventorySource->_index == Scenes::instance()._currentActorIndex) {
		if (_scummUI)
			_scummUI->syncInventory();
		return;
	}

	// Binary: g_wUiPanelState = 2 for protagonist, 3 for container
	_uiPanelState = (newInventorySource->_index == Scenes::instance()._currentActorIndex)
						? kUiPanelInventory
						: kUiPanelContainerInventory;
	_inventoryScrollOffset = 0;
	_activeInventoryItem = nullptr;
	g_engine->_scriptExecutor->_inventoryActionFlag = false;
	g_engine->_scriptExecutor->_inventoryCombineFlag = false;
	// Binary drawProtagonistInventoryPanel (1008:45aa): unconditionally calls setCursorMode(0x15)
	g_engine->setCursorMode(Script::MouseMode::Use);
	updateCursor();
	redraw();
}

void View1::closeInventory() {
	if (_uiPanelState != kUiPanelInventory && _uiPanelState != kUiPanelContainerInventory) {
		return;
	}

	const bool wasContainerPanel = _uiPanelState == kUiPanelContainerInventory;
	_uiPanelState = kUiPanelNone;
	_inventoryScrollOffset = 0;
	_activeInventoryItem = nullptr;
	g_engine->_scriptExecutor->_inventoryActionFlag = false;
	g_engine->_scriptExecutor->_inventoryCombineFlag = false;

	if (!isInventorySourceProtagonist()) {
		setInventorySource(GameObjects::instance().getProtagonistObject());
	}

	g_engine->setCursorMode(_savedCursorMode);
	updateCursor();

	if (wasContainerPanel) {
		// Binary handleInput, panel state 3 + close button (6): restore the
		// script context saved by scriptOpenInventory (g_wScriptIsExecuting = 1),
		// then runScriptExecutor resumes right after the openInventory opcode.
		g_engine->setCursorMode(g_engine->_scriptExecutor->_savedExternalInventoryMouseMode);
		updateCursor();
		setInventorySource(GameObjects::instance().getProtagonistObject());
		g_engine->_scriptExecutor->_hasPendingExternalInventoryResume = false;
		g_engine->_scriptExecutor->_externalInventorySourceObjectID = 0;
		g_engine->_scriptExecutor->restoreOpenInventoryScriptContext();
		g_engine->runScriptExecutor();
	}
}

void View1::setInventorySource(GameObject *newInventorySource) {
	if (newInventorySource == nullptr) {
		warning("Tried to set inventory source to a null object");
		return;
	}
	_inventorySource = newInventorySource;
	// Rebuild inventory list from all objects whose SceneIndex matches.
	// Binary (syncInventoryObjectList at 1008:071e) checks: object.sceneIndex == actorIndex + 0x400.
	const uint16 inventorySceneId = (_inventorySource->_index == Scenes::instance()._currentActorIndex)
										? (uint16)(Scenes::instance()._currentActorIndex + 0x400)
										: (uint16)(_inventorySource->_index + 0x400);
	_inventoryItems.clear();

	for (GameObject *currentObject : GameObjects::instance()._objects) {
		if (currentObject != nullptr && currentObject->_sceneIndex == inventorySceneId) {
			_inventoryItems.push_back(currentObject);
		}
	}
	if (hasScummVerbUI() && _scummUI)
		_scummUI->syncInventory();
}

void View1::refreshProtagonistInventoryAfterLoad(uint16 actorIndex) {
	_inventorySource = GameObjects::instance().getProtagonistObject();
	const uint16 invScene = actorIndex + 0x400;

	Common::Array<GameObject *> validated;
	for (GameObject *obj : _inventoryItems) {
		if (obj && obj->_sceneIndex == invScene)
			validated.push_back(obj);
	}

	for (GameObject *obj : GameObjects::instance()._objects) {
		if (!obj || obj->_sceneIndex != invScene)
			continue;

		bool found = false;
		for (GameObject *listed : validated) {
			if (listed == obj) {
				found = true;
				break;
			}
		}
		if (!found)
			validated.push_back(obj);
	}

	_inventoryItems = validated;

	if (hasScummVerbUI() && _scummUI)
		_scummUI->resetInventoryAfterLoad();
}

bool View1::isInventorySourceProtagonist() const {
	return _inventorySource->_index == 1;
}

void View1::transferInventoryItem(GameObject *item, GameObject *targetContainer) {
	int index = findInventoryItem(item);
	_inventoryItems.remove_at(index);
	item->_sceneIndex = targetContainer->_index + 0x400;
	if (hasScummVerbUI() && _scummUI)
		_scummUI->syncInventory();
}

int View1::findInventoryItem(const GameObject *item) {
	for (uint i = 0; i != _inventoryItems.size(); i++) {
		if (_inventoryItems[i] == item) {
			return (int)i;
		}
	}
	return -1;
}

Character *View1::getCharacterByIndex(uint16 index) const {
	if (index > 0 && index <= kMaxSceneObjects) {
		Character *c = _characterByObjectIndex[index];
		if (c != nullptr && c->_gameObject != nullptr && c->_gameObject->_index == index)
			return c;
	}
	return nullptr;
}

void View1::rebuildCharacterLookupTable() const {
	memset(_characterByObjectIndex, 0, sizeof(_characterByObjectIndex));
	for (Character *c : _characters) {
		if (c != nullptr && c->_gameObject != nullptr && c->_gameObject->_index > 0 &&
			c->_gameObject->_index <= kMaxSceneObjects)
			_characterByObjectIndex[c->_gameObject->_index] = c;
	}
}

void View1::sortObjectListByY() const {
	// sortObjectListByY @ 1008:8cf2 - 1-based FAC table @ 0xFAC
	_sortedObjectCount = 0;
	const uint16 sceneIndex = (uint16)Scenes::instance()._currentSceneIndex;
	for (uint16 objectIndex = 1; objectIndex <= kMaxSceneObjects; objectIndex++) {
		GameObject *obj = GameObjects::getObjectByIndex(objectIndex);
		if (obj == nullptr || obj->_dataOffset == 0)
			continue;
		if ((int16)obj->_sceneIndex < 0 || obj->_sceneIndex != sceneIndex)
			continue;
		_sortedObjectIndices[++_sortedObjectCount] = objectIndex;
	}
	if (_sortedObjectCount > 1)
		buildSortedObjectList(1, (int)_sortedObjectCount);
}

void View1::buildSortedObjectList(int low, int high) const {
	// buildSortedObjectList @ 1008:8c5a - quicksort slots [low..high] by object Y (+0x02)
	if (low >= high)
		return;

	auto objectY = [](uint16 objectIndex) -> int {
		GameObject *obj = GameObjects::getObjectByIndex(objectIndex);
		return obj ? obj->_position.y : 0;
	};

	const int pivotY = objectY(_sortedObjectIndices[(low + high) / 2]);
	int i = low;
	int j = high;
	while (i <= j) {
		while (objectY(_sortedObjectIndices[i]) < pivotY)
			i++;
		while (pivotY < objectY(_sortedObjectIndices[j]))
			j--;
		if (i <= j) {
			SWAP(_sortedObjectIndices[i], _sortedObjectIndices[j]);
			i++;
			j--;
		}
	}
	if (low < j)
		buildSortedObjectList(low, j);
	if (i < high)
		buildSortedObjectList(i, high);
}

void View1::updateCursor(const byte *palette) {
	CursorMan.showMouse(true);

	// Original indexes cursor array as: base + mode * 16 - 16, i.e. 0-based index = mode - 1.
	// The array has 33 entries (indices 0-32). Cursor modes 0x13-0x1A map to entries 18-25.
	int mode = (int)g_engine->_scriptExecutor->_cursorMode - 1;

	// SCUMM-style UI: gameplay verbs share the walk cursor; the sentence line shows the active verb.
	if (hasScummVerbUI()) {
		const Script::MouseMode cursorMode = g_engine->_scriptExecutor->_cursorMode;
		switch (cursorMode) {
		case Script::MouseMode::Talk:
		case Script::MouseMode::Look:
		case Script::MouseMode::Use:
		case Script::MouseMode::Walk:
			mode = (int)Script::MouseMode::Walk - 1;
			break;
		default:
			break;
		}
	}

	if (mode < 0 || mode >= kNumLoadedCursors) {
		warning("Invalid cursor mode %d, falling back to Walk cursor", mode);
		mode = (int)Script::MouseMode::Walk - 1;
	}

	if (mode >= (int)g_engine->_imageResources.size() || g_engine->_imageResources[mode]._data.empty() || g_engine->_imageResources[mode]._width == 0) {
		warning("Cursor data for mode %d is invalid", mode);
		return;
	}

	const uint16 width = g_engine->_imageResources[mode]._width;
	const uint16 height = g_engine->_imageResources[mode]._height;
	const byte *cursorData = g_engine->_imageResources[mode]._data.data();
	const Graphics::PixelFormat rgbaCursorFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
	Common::Array<uint32> rgbaCursor;
	rgbaCursor.resize(width * height);

	for (uint i = 0; i < rgbaCursor.size(); ++i) {
		const byte colorIndex = cursorData[i];
		if (colorIndex == 0) {
			rgbaCursor[i] = 0;
			continue;
		}

		const byte *activePalette = palette ? palette : g_engine->_pal;
		const byte *paletteEntry = &activePalette[colorIndex * 3];
		rgbaCursor[i] = rgbaCursorFormat.RGBToColor(paletteEntry[0], paletteEntry[1], paletteEntry[2]);
	}

	CursorMan.replaceCursor(rgbaCursor.data(), width, height, width >> 1, height >> 1, 0, &rgbaCursorFormat);
	// Enable a cursor palette so the backend won't re-blit the cursor on
	// every screen palette change. The macs2 engine uses RGBA cursors with
	// baked-in palette colors, so the cursor palette content is irrelevant -
	// it just needs to exist to prevent the backend's setPalette() from
	// triggering blitCursor() which can corrupt the RLE-accelerated surface.
	byte dummyPalette[256 * 3] = {};
	CursorMan.replaceCursorPalette(dummyPalette, 0, 256);
}

AnimFrame *View1::getInventoryIcon(GameObject *gameObject) {
	AnimFrame *result = new AnimFrame();
	// Inventory icon is always in blob slot 0x14 (zero-based index 0x13)
	// Original: runtime offset +0x16c/+0x16e, validity at +0x173
	int index = 0x13;
	if (index >= (int)gameObject->_blobs.size() || gameObject->_blobs[index].empty()) {
		warning("GetInventoryIcon: no icon blob for object %u", gameObject->_index);
		delete result;
		return nullptr;
	}
	Common::Array<uint8> &blob = gameObject->_blobs[index];

	// Original calls getAnimFrameWidth(1, ...) with mode=1 to reset to frame 1
	uint16 offset = Macs2::BackgroundAnimationBlob::advanceAnimFrame(blob, true, 1);
	// offset points to per-frame: offsetX(2), offsetY(2), unknown(2), width(2), height(2), pixels
	offset += 6;
	result->_width = READ_LE_UINT16(&blob[offset]);
	result->_height = READ_LE_UINT16(&blob[offset + 2]);
	result->_data.resize(result->_width * result->_height);
	memcpy(result->_data.data(), &blob[offset + 4], result->_width * result->_height);
	// TODO: Think about proper memory management
	return result;
}

void View1::drawDarkRectangle(uint16 x, uint16 y, uint16 width, uint16 height) {
	// drawAnimFrameScaled @ 1010:1399: remap each background pixel through per-scene 256-byte table
	Graphics::ManagedSurface s = getSurface();
	for (uint16 xOffset = 0; xOffset < width; xOffset++) {
		for (uint16 yOffset = 0; yOffset < height; yOffset++) {
			const uint16 currentX = x + xOffset;
			const uint16 currentY = y + yOffset;
			const uint8 currentValue = (uint8)s.getPixel(currentX, currentY);
			const uint8 newValue = g_engine->_panelRemapTable[currentValue];
			if (currentX < kScreenWidth && currentY < kGameHeight)
				s.setPixel(currentX, currentY, newValue);
		}
	}
}

void View1::drawBackgroundAnimations(Graphics::ManagedSurface &s) {
	for (int i = 0; i < (int)g_engine->_backgroundAnimations.size(); i++) {
		BackgroundAnimation &current = g_engine->_backgroundAnimations[i];
		BackgroundAnimationBlob &currentBlob = g_engine->_backgroundAnimationsBlobs[i];
		// Binary drawAllCharacters (1008:90a2): null bg-anim blob -> error 0x08;
		// zero frame count -> error 0x0B; aborts entire draw pass.
		if (currentBlob._blob.empty()) {
			g_engine->_scriptExecutor->setScriptError(8);
			return;
		}
		AnimBlobView view(currentBlob._blob);
		if (!view.isValid() || view.frameCount() == 0) {
			g_engine->_scriptExecutor->setScriptError(view.frameCount() == 0 ? 0x0B : 8);
			return;
		}
		// Binary drawAllCharacters (1008:929c): drawAnimFrame(2, y, x+1, blob) - one
		// advanceAnimFrame(save=1, mode=2) per frame, not a separate tick advance.
		uint16 frameStart = BackgroundAnimationBlob::advanceAnimFrame(currentBlob._blob, true, 2);
		int16 frameOffsetX = (int16)READ_LE_UINT16(&currentBlob._blob[frameStart]);
		int16 frameOffsetY = (int16)READ_LE_UINT16(&currentBlob._blob[frameStart + 2]);
		AnimFrame currentFrame;
		currentFrame._width = READ_LE_UINT16(&currentBlob._blob[frameStart + 6]);
		currentFrame._height = READ_LE_UINT16(&currentBlob._blob[frameStart + 8]);
		currentFrame._data.resize(currentFrame._width * currentFrame._height);
		memcpy(currentFrame._data.data(), &currentBlob._blob[frameStart + 10],
			   currentFrame._width * currentFrame._height);
		drawSprite(current._x + 1 + frameOffsetX, current._y + frameOffsetY, currentFrame, s, false);
	}
}

void View1::drawCurrentSpeaker(Graphics::ManagedSurface &s) {
	// Mouth animation from handleTimerCallback (1008:d38b).
	// Cycles between frame 1 (mouth open) and frame 2 (mouth closed)
	// based on a decrementing counter, creating a talking animation.
	bool useAlternateBlob = false;
	if (currentSpeechActData.mouthAnimActive) {
		if (currentSpeechActData.mouthAnimCounter <= 0) {
			useAlternateBlob = true;
		}
	}

	// Select portrait blob: primary (Blobs[17]) during countdown, alternate (Blobs[18]) after
	// Mode 0: render current frame without advancing (advance happens in tick())
	AnimFrame *frame = currentSpeechActData.speaker->getCurrentPortrait(useAlternateBlob, 0);
	if (frame == nullptr) {
		return;
	}
	AnimFrame *leftPortrait = currentSpeechActData.speaker->getCurrentPortrait(false, 0);
	AnimFrame *rightPortrait = currentSpeechActData.speaker->getCurrentPortrait(true, 0);

	// See l0037_B462: for the calculations below
	// Draw the border
	const int portraitWidth = MAX<int>(leftPortrait ? leftPortrait->_width : 0, rightPortrait ? rightPortrait->_width : 0);
	const int portraitHeight = MAX<int>(leftPortrait ? leftPortrait->_height : 0, rightPortrait ? rightPortrait->_height : 0);
	const Common::Point borderSize(portraitWidth + 0xD, portraitHeight + 0xD);
	drawBorder(currentSpeechActData.position, borderSize, s);

	// Draw the portrait over the border
	Common::Point pos = currentSpeechActData.position + Common::Point(7, 7);
	drawSprite(pos, frame->_width, frame->_height, frame->_data.data(), s, false);
	delete frame;
	delete leftPortrait;
	delete rightPortrait;
}

void View1::renderString(uint16 x, uint16 y, const Common::String &s) {
	Graphics::ManagedSurface surf = getSurface();
	uint16 currentX = x;
	uint16 currentY = y;

	// First pass: find widest glyph (drawTextString at 1010:21b1 does this)
	uint16 widestGlyph = 1;
	for (auto iter = s.begin(); iter != s.end(); iter++) {
		GlyphData data;
		if (g_engine->findGlyph(*iter, data)) {
			widestGlyph = MAX(widestGlyph, data._width);
		}
	}

	// Second pass: render with correct spacing
	for (auto iter = s.begin(); iter != s.end(); iter++) {
		GlyphData data;
		bool found = g_engine->findGlyph(*iter, data);
		if (found) {
			drawSprite(currentX, currentY, data, surf, false);
			currentX += data._width + 1;
		} else {
			if ((byte)*iter != ' ') {
				warning("Missing glyph for character 0x%02x while rendering \"%s\" at (%u,%u)", (byte)*iter, s.c_str(), x, y);
			}
			currentX += widestGlyph;
		}
	}
}

void View1::renderString(const Common::Point pos, const Common::String &s) {
	renderString(pos.x, pos.y, s);
}

void View1::renderStringTo(uint16 x, uint16 y, const Common::String &s, Graphics::ManagedSurface &surf) {
	uint16 currentX = x;
	uint16 currentY = y;

	uint16 widestGlyph = 1;
	for (auto iter = s.begin(); iter != s.end(); iter++) {
		GlyphData data;
		if (g_engine->findGlyph(*iter, data)) {
			widestGlyph = MAX(widestGlyph, data._width);
		}
	}

	for (auto iter = s.begin(); iter != s.end(); iter++) {
		GlyphData data;
		const bool found = g_engine->findGlyph(*iter, data);
		if (found) {
			drawSprite(currentX, currentY, data, surf, false);
			currentX += data._width + 1;
		} else {
			currentX += widestGlyph;
		}
	}
}

int View1::measureStringWithFont(const Common::String &s, const GlyphData *glyphs, uint16 numGlyphs) {
	int width = 0;
	uint16 widestGlyph = 1;
	for (uint i = 0; i < numGlyphs; i++) {
		widestGlyph = MAX(widestGlyph, glyphs[i]._width);
	}
	for (auto iter = s.begin(); iter != s.end(); iter++) {
		bool found = false;
		for (uint i = 0; i < numGlyphs; i++) {
			if (glyphs[i]._ascii == *iter) {
				width += glyphs[i]._width + 1;
				found = true;
				break;
			}
		}
		if (!found)
			width += widestGlyph;
	}
	return width;
}

void View1::renderStringWithFont(uint16 x, uint16 y, const Common::String &s, const GlyphData *glyphs, uint16 numGlyphs) {
	Graphics::ManagedSurface surf = getSurface();
	renderStringWithFontTo(x, y, s, glyphs, numGlyphs, surf);
}

void View1::renderStringWithFontTo(uint16 x, uint16 y, const Common::String &s, const GlyphData *glyphs,
								   uint16 numGlyphs, Graphics::ManagedSurface &surf) {
	uint16 currentX = x;
	uint16 widestGlyph = 1;
	for (uint i = 0; i < numGlyphs; i++) {
		widestGlyph = MAX(widestGlyph, glyphs[i]._width);
	}
	for (auto iter = s.begin(); iter != s.end(); iter++) {
		bool found = false;
		for (uint i = 0; i < numGlyphs; i++) {
			if (glyphs[i]._ascii == *iter) {
				drawSprite(currentX, y, glyphs[i], surf, false);
				currentX += glyphs[i]._width + 1;
				found = true;
				break;
			}
		}
		if (!found)
			currentX += widestGlyph;
	}
}

void View1::addOverlayTextEntry(const OverlayTextEntry &entry) {
	_overlayTextEntries.push_back(entry);
	redraw();
}

void View1::clearOverlayTextEntries() {
	if (_overlayTextEntries.empty()) {
		return;
	}
	_overlayTextEntries.clear();
	redraw();
}

void View1::drawOverlayTextEntries() {
	for (const OverlayTextEntry &entry : _overlayTextEntries) {
		int x = entry.position.x;
		Common::String text = entry.text;
		// Use overlay font if loaded, otherwise fall back to main font
		const GlyphData *font = g_engine->numOverlayGlyphs > 0 ? g_engine->_overlayGlyphs : g_engine->_glyphs;
		uint16 fontCount = g_engine->numOverlayGlyphs > 0 ? g_engine->numOverlayGlyphs : g_engine->numGlyphs;

		if (entry.alignment == 1) {
			x -= measureStringWithFont(text, font, fontCount);
		} else if (entry.alignment == 2) {
			x -= measureStringWithFont(text, font, fontCount) / 2;
		}

		if (x < 0)
			x = 0;

		logRenderedText("Overlay", x, entry.position.y, text);
		renderStringWithFont(x, entry.position.y, text, font, fontCount);
	}
}

void View1::showStringBox(const Common::StringArray &sa) {
	// This calculation can be found at l0037_B368:
	// int borderWidth = 10;
	// int padding = 3;
	int totalWidth = g_engine->measureStrings(sa) + 0x12;
	int totalHeight = g_engine->measureStringsVertically(sa) + 0x10;
	g_engine->_textLog.push_back(Common::String::format(
									 "Render text box: lines=%u pos=(%d,%d) size=(%d,%d) text=\"", sa.size(),
									 _stringBoxPosition.x, _stringBoxPosition.y, totalWidth, totalHeight) +
								 Common::String(Common::U32String(joinDebugStrings(sa).c_str(), Common::kDos850)) + "\"");

	Graphics::ManagedSurface s = getSurface();
	drawBorder(_stringBoxPosition, Common::Point(totalWidth, totalHeight), s);
	// TODO range based
	int lineOffset = _stringBoxPosition.y + 0x9;
	for (auto iter = sa.begin(); iter < sa.end(); iter++) {
		logRenderedText("TextBox", _stringBoxPosition.x + 0x9, lineOffset, *iter);
		renderString(_stringBoxPosition.x + 0x9, lineOffset, *iter);
		lineOffset += g_engine->maxGlyphHeight + 2;
	}
}

void View1::drawPathfindingPoints(Graphics::ManagedSurface &s) {
	GlyphData xData;
	g_engine->findGlyph('x', xData);
	for (int i = 0; i < 16; i++) {
		PathfindingPoint &current = g_engine->pathfindingPoints[i];
		renderString(current._position.x - xData._width * 0.5, current._position.y - xData._height * 0.5, "x");

		Common::String number = Common::String::format("%u", i);
		renderString(current._position.x - xData._width * 0.5 + 10, current._position.y - xData._height * 0.5 + 10, number.c_str());

		for (uint8 adjacentIndex : current._adjacentPoints) {
			if (adjacentIndex >= g_engine->pathfindingPoints.size()) {
				continue;
			}
			PathfindingPoint &other = g_engine->pathfindingPoints[adjacentIndex - 1];
			s.drawLine(current._position.x, current._position.y, other._position.x, other._position.y, 0xFFFFFFFF);
		}
	}

	// Draw the test results
	Macs2::Character *c = getCharacterByIndex(Scenes::instance()._currentActorIndex);
	// Handle the active actor not being in the scene
	if (c == nullptr) {
		return;
	}
	const Common::Array<uint8> &overlay = c->_pathfindingOverlay;
	for (int y = 0; y < kGameHeight; y++) {
		for (int x = 0; x < kScreenWidth; x++) {
			const uint8 currentValue = overlay[y * kScreenWidth + x];
			if (currentValue != 0) {
				s.setPixel(x, y, currentValue);
			}
		}
	}
}

void View1::drawDebugOutput(Graphics::ManagedSurface &s) {
	uint16 x = 0;
	uint16 y = 0;
	constexpr uint16 deltaY = 20;
	for (const Common::String &current : g_engine->_debugOutput) {
		renderString(x, y, current);
		y += deltaY;
	}
}

void View1::drawPath(Graphics::ManagedSurface &s) {
	if (g_engine->_path.size() < 2) {
		return;
	}
	for (uint i = 0; i < g_engine->_path.size() - 1; i++) {
		s.drawLine(g_engine->_path[i].x, g_engine->_path[i].y, g_engine->_path[i + 1].x, g_engine->_path[i + 1].y, 0xFF);
	}
}

void View1::layoutActionBarButtons() {
	uint16 maxW = 0, maxH = 0;
	for (int i = 0; i < 9 && i < (int)g_engine->_imageResources.size(); i++) {
		maxW = MAX(maxW, g_engine->_imageResources[i]._width);
		maxH = MAX(maxH, g_engine->_imageResources[i]._height);
	}
	const uint16 btnW = maxW + 6;
	const uint16 btnH = maxH + 6;

	_mainMenuButtonLocations.resize(9);
	for (int i = 0; i < 9; i++) {
		const int col = i % 3;
		const int row = i / 3;
		const uint16 cellX = _mainMenuRect.left + 4 + col * (btnW + 4);
		const uint16 cellY = _mainMenuRect.top + 4 + row * (btnH + 4);
		_mainMenuButtonLocations[i] = Common::Rect(cellX, cellY, cellX + btnW, cellY + btnH);
	}
}

void View1::openMainMenu(Common::Point clickedPosition) {
	if (hasScummVerbUI())
		return;

	// Binary handleInput: save cursor and set to PanelCursor (0x19)
	_savedCursorMode = g_engine->_scriptExecutor->_cursorMode;
	g_engine->setCursorMode(Script::MouseMode::PanelCursor);
	_clickedButtonIndex = 0;

	uint16 maxW = 0, maxH = 0;
	for (int i = 0; i < 9 && i < (int)g_engine->_imageResources.size(); i++) {
		maxW = MAX(maxW, g_engine->_imageResources[i]._width);
		maxH = MAX(maxH, g_engine->_imageResources[i]._height);
	}
	const uint16 btnW = maxW + 6;
	const uint16 btnH = maxH + 6;
	const Common::Point panelSize(btnW * 3 + 0x10, btnH * 3 + 0x10);

	Common::Point upperLeft = clickedPosition - panelSize / 2;
	if (upperLeft.x < 0) {
		upperLeft.x = 0;
	}
	if (upperLeft.y < 0) {
		upperLeft.y = 0;
	}
	// Binary openActionBarAtPosition (1008:3fba): clamp to screen bounds.
	if ((int)(upperLeft.x + panelSize.x) >= kScreenWidth) {
		upperLeft.x = kScreenWidth - panelSize.x - 1;
	}
	if ((int)(upperLeft.y + panelSize.y) >= kGameHeight) {
		upperLeft.y = kGameHeight - panelSize.y - 1;
	}

	_mainMenuRect = Common::Rect(upperLeft, upperLeft + panelSize);
	_uiPanelState = kUiPanelActionBar;
	layoutActionBarButtons();
	redraw();
}

void View1::enterMapMode() {
	// Binary handleInput end-block when scene+0x61db != 0 (1008:e8bf): fade, load map
	// from scene+0x5DDB (_mapSceneOffsets[0]), set cursor 0x18 (PanelUse).
	uint32 helpOffset = g_engine->_mapSceneOffsets[0];
	if (helpOffset == 0 || helpOffset >= (uint32)g_engine->_fileStream->size()) {
		return;
	}
	memcpy(_savedPalVanilla, g_engine->_palVanilla, 256 * 3);
	_savedDepthMap.copyFrom(g_engine->_depthMap);
	startFadeToBlack(8);
	Graphics::ManagedSurface mapBg = g_engine->readRLEImage(helpOffset, g_engine->_fileStream);
	_backgroundSurface.copyFrom(mapBg);
	g_engine->_fileStream->read(g_engine->_palVanilla, 0x300);
	g_engine->applyPaletteDarkening();
	Graphics::ManagedSurface mapDepth = g_engine->readRLEImage(g_engine->_fileStream->pos(), g_engine->_fileStream);
	g_engine->_depthMap.blitFrom(mapDepth);
	g_engine->_mapSubSceneTableFilePos = g_engine->_fileStream->pos();
	_currentMode = ViewMode::VM_HELP;
	g_engine->setCursorMode(Script::MouseMode::PanelUse);
	updateCursor();
	startFading(8);
	redraw();
}

void View1::drawMainMenu(Graphics::ManagedSurface &s) {
	layoutActionBarButtons();

	drawBorderSide(Common::Point(_mainMenuRect.left, _mainMenuRect.top), Common::Point(_mainMenuRect.width(), _mainMenuRect.height()), s);
	drawNinePatchBorder(Common::Point(_mainMenuRect.left, _mainMenuRect.top), Common::Point(_mainMenuRect.width(), _mainMenuRect.height()), kBorderRaised, false, false, s);

	for (int i = 0; i < 9 && i < (int)g_engine->_imageResources.size(); i++) {
		const Common::Rect &cell = _mainMenuButtonLocations[i];
		const bool pressed = (_clickedButtonIndex == (uint16)(i + 1));
		const BorderStyle &border = pressed ? kBorderPressed : kBorderRaised;
		drawNinePatchBorder(Common::Point(cell.left, cell.top), Common::Point(cell.width(), cell.height()), border, false, false, s);

		AnimFrame &frame = g_engine->_imageResources[i];
		const int pressOffset = pressed ? 1 : 0;
		const uint16 iconX = cell.left + (cell.width() - frame._width) / 2 + pressOffset;
		const uint16 iconY = cell.top + (cell.height() - frame._height) / 2 + pressOffset;
		drawSprite(iconX, iconY, frame, s, false);
	}
}

void View1::handleTextBoxInput() {
	// Binary handleTextBoxInput (1008:a8b2): redraws background over text box,
	// then sets g_wIsShowingTextBox = 0. Nothing else.
	_isShowingTextBox = false;
	g_engine->_scriptExecutor->_waitingForUiClick = false;
	redraw();
}

void View1::dismissDialoguePanel() {
	// Binary dismissDialoguePanel (1008:b66d): redraws background over dialogue rect,
	// then sets g_wIsShowingDialoguePanel = 0. Does NOT touch scene+0x53B9.
	_isShowingDialoguePanel = false;
	g_engine->_scriptExecutor->_waitingForUiClick = false;
	redraw();
}

bool View1::handleDialogueChoiceClick(int clickY, int clickX) {
	// Binary handleDialogueChoiceClick (1008:d53b):
	// Checks if click is within text box bounds (X+9..X+W-9, Y+9..Y+H-9).
	// Iterates choice entries to find which line was clicked.
	// Stores script index at scene+0x53B7 and clears scene+0x53B9.
	const int boxW = g_engine->measureStrings(_drawnStringBox) + 0x12;
	const int boxH = g_engine->measureStringsVertically(_drawnStringBox) + 0x10;
	if (clickX < _stringBoxPosition.x + 9 || clickY < _stringBoxPosition.y + 9 ||
		clickX > _stringBoxPosition.x + boxW - 9 || clickY > _stringBoxPosition.y + boxH - 9) {
		return false;
	}

	int lineHeight = g_engine->maxGlyphHeight + 2;
	int firstLineY = _stringBoxPosition.y + 9;
	int relY = clickY - firstLineY;
	debug("handleDialogueChoiceClick: clickY=%d firstLineY=%d relY=%d lineHeight=%d clickedLine=%d",
		  clickY, firstLineY, relY, lineHeight, relY >= 0 ? relY / lineHeight : -1);
	if (relY >= 0) {
		int clickedLine = relY / lineHeight;
		int cumulativeLines = 0;
		for (uint i = 0; i < _dialogueChoiceLineCounts.size(); i++) {
			cumulativeLines += _dialogueChoiceLineCounts[i];
			if (clickedLine < cumulativeLines) {
				// Binary: clears scene+0x53B9 before storing choice
				_isDialogueChoiceInputActive = false;
				triggerDialogueChoice(i + 1);
				return true;
			}
		}
	}
	return false;
}

int View1::getCharacterArrayIndex(const Character *c) const {
	// TODO: Check if there is a find function somewhere
	for (uint i = 0; i < _characters.size(); i++) {
		if (_characters[i] == c) {
			return (int)i;
		}
	}
	return -1;
}

// Effective world position for pickup walk targets and bounds-attached props.
// Inventory items use their holder's position; attached objects use parent + offset.
static Common::Point getObjectEffectivePosition(const GameObject *object) {
	if (object == nullptr) {
		return Common::Point();
	}
	if (object->_hasBoundsAttachment) {
		const GameObject *parent = GameObjects::getObjectByIndex(object->_boundsAttachmentObjectID);
		if (parent != nullptr) {
			return Common::Point(
				parent->_position.x + (int16)object->_boundsAttachmentValue1,
				parent->_position.y + (int16)object->_boundsAttachmentValue2);
		}
	}
	if (object->_sceneIndex > 0x400) {
		const GameObject *holder = GameObjects::getObjectByIndex(object->_sceneIndex - 0x400);
		if (holder != nullptr) {
			return holder->_position;
		}
	}
	return object->_position;
}

void View1::transferPickupTarget(GameObject *targetObject) {
	if (targetObject == nullptr) {
		return;
	}

	Script::ScriptExecutor *executor = g_engine->_scriptExecutor;
	const uint16 actorIndex = executor->_pickupActorObjectID;
	if (actorIndex == 0) {
		return;
	}

	// Binary drawAllCharacters (1008:90a2): target.sceneIndex = g_wPickupActorObjectId + 0x400
	targetObject->_sceneIndex = actorIndex + 0x400;
	targetObject->_hasBoundsAttachment = false;
	targetObject->_boundsAttachmentObjectID = 0;
	targetObject->_boundsAttachmentValue1 = 0;
	targetObject->_boundsAttachmentValue2 = 0;
	targetObject->_boundsAttachmentValue3 = 0;

	Character *itemCharacter = getCharacterByIndex(targetObject->_index);
	if (itemCharacter != nullptr) {
		executor->saveWalkRuntime(itemCharacter, targetObject);
		const int index = getCharacterArrayIndex(itemCharacter);
		if (index >= 0) {
			itemCharacter->_markedForDeletion = true;
			_characters.remove_at(index);
			_pendingCharacterDeletes.push_back(itemCharacter);
			rebuildCharacterLookupTable();
		}
	}

	if (_inventorySource != nullptr && _inventorySource->_index == actorIndex) {
		bool alreadyListed = false;
		for (const GameObject *item : _inventoryItems) {
			if (item->_index == targetObject->_index) {
				alreadyListed = true;
				break;
			}
		}
		if (!alreadyListed) {
			_inventoryItems.push_back(targetObject);
		}
	} else {
		for (uint i = 0; i < _inventoryItems.size(); i++) {
			if (_inventoryItems[i]->_index == targetObject->_index) {
				_inventoryItems.remove_at(i);
				break;
			}
		}
	}

	if (_activeInventoryItem != nullptr && _activeInventoryItem->_index == targetObject->_index) {
		_activeInventoryItem = nullptr;
		if (executor->_cursorMode == Script::MouseMode::UseInventory) {
			g_engine->setCursorMode(Script::MouseMode::Use);
			updateCursor();
		}
	}

	if (hasScummVerbUI() && _scummUI)
		_scummUI->syncInventory();

	// Binary sets g_wNeedsRedraw and restores scene background over the panel area.
	redraw();
}

void View1::startFading(uint16 speed) {
	startFadingWithSpeed(speed);
}

void View1::fadePaletteToBlack(uint16 speed, const byte *sourcePalette) {
	// Blocking fade to black matching DOS fadePaletteToBlack (1010:00ba).
	if (speed == 0)
		speed = 4;
	if (sourcePalette == nullptr)
		sourcePalette = g_engine->_palVanilla;
	beginFadeCursorSuppression();

	// Ensure current frame is on screen before fading
	Graphics::ManagedSurface *screen = g_events->getScreen();
	g_system->copyRectToScreen((const byte *)screen->getPixels(),
							   screen->pitch, 0, 0, screen->w, screen->h);

	uint fadeValue = 0;
	while (fadeValue <= 0x40 && !g_system->getEventManager()->shouldQuit()) {
		uint32 frameStart = g_system->getMillis();

		byte colors[256 * 3];
		buildFadedPalette(colors, sourcePalette, fadeValue);
		setViewPaletteSafely(colors);
		g_system->copyRectToScreen((const byte *)g_events->getScreen()->getPixels(),
								   g_events->getScreen()->pitch, 0, 0, g_events->getScreen()->w, g_events->getScreen()->h);
		g_system->updateScreen();

		Common::Event evt;
		while (g_system->getEventManager()->pollEvent(evt)) {
			if (evt.type == Common::EVENT_QUIT)
				break;
		}

		// Original syncs to VGA vsync during palette writes. On real hardware
		// writing 768 bytes to the DAC takes most of one frame period.
		uint32 elapsed = g_system->getMillis() - frameStart;
		if (elapsed < 16)
			g_system->delayMillis(16 - elapsed);
		fadeValue += speed;
	}

	// Final: set all black
	byte colors[256 * 3];
	memset(colors, 0, sizeof(colors));
	setViewPaletteSafely(colors);
	g_system->updateScreen();

	_currentFadeValue = 0x40;
	_fadeMode = FadeMode::None;
	endFadeCursorSuppression(colors);
}

void View1::startFadeToBlack(uint16 speed) {
	fadePaletteToBlack(speed, g_engine->_palVanilla);
}

void View1::instantSceneCut() {
	// Binary scriptChangeScene mode 1 (1008:ad6e): clearScreen + setPaletteRange(0x100, 0).
	// applyScenePaletteEffect is only used on the help-disabled path, not here.
	byte blackPal[256 * 3];
	memset(blackPal, 0, sizeof(blackPal));
	setViewPaletteSafely(blackPal);
	Graphics::ManagedSurface s = getSurface();
	s.fillRect(Common::Rect(s.w, s.h), 0);
	g_system->copyRectToScreen((const byte *)s.getPixels(), s.pitch, 0, 0, s.w, s.h);
	setViewPaletteSafely(g_engine->_pal);
	_paletteDirty = false;
	g_system->updateScreen();
}

void View1::presentFrame() {
	draw();
	_needsRedraw = false;
	g_events->getScreen()->makeAllDirty();
	g_events->getScreen()->update();
}

void View1::startFadingWithSpeed(uint16 speed) {
	// Blocking fade from black matching DOS fadePaletteFromBlack (1010:012f):
	// Original starts at fadeValue = fadeSpeed + 0x40, subtracts fadeSpeed each
	// iteration until underflow or zero, then writes the full target palette.
	// Each iteration waits for VGA vsync (~14ms at 70Hz).
	if (speed == 0)
		speed = 4;
	beginFadeCursorSuppression();

	// Set palette to black before blitting new scene pixels
	byte blackPal[256 * 3];
	memset(blackPal, 0, sizeof(blackPal));
	g_system->getPaletteManager()->setPalette(blackPal, 0, 256);

	// Draw the new scene to the screen surface (invisible because palette is black)
	Graphics::ManagedSurface s = getSurface();
	s.blitFrom(_backgroundSurface);
	if (_currentMode != ViewMode::VM_HELP) {
		drawAllCharacters(&s, false);
	}

	// Copy pixels to the system screen
	g_system->copyRectToScreen((const byte *)g_events->getScreen()->getPixels(),
							   g_events->getScreen()->pitch, 0, 0, g_events->getScreen()->w, g_events->getScreen()->h);
	g_system->updateScreen();

	// Fade from black: original starts at speed + 0x40 to guarantee first frame
	// is fully black (max 6-bit value is 0x3F, so subtracting 0x44 always clamps to 0)
	int fadeValue = speed + 0x40;
	while (!g_system->getEventManager()->shouldQuit()) {
		uint32 frameStart = g_system->getMillis();

		applyPaletteWithFade(g_engine->_palVanilla, fadeValue);
		// Re-copy pixels so the backend redraws with the new palette
		g_system->copyRectToScreen((const byte *)g_events->getScreen()->getPixels(),
								   g_events->getScreen()->pitch, 0, 0, g_events->getScreen()->w, g_events->getScreen()->h);
		g_system->updateScreen();

		Common::Event evt;
		while (g_system->getEventManager()->pollEvent(evt)) {
			if (evt.type == Common::EVENT_QUIT)
				break;
		}

		uint32 elapsed = g_system->getMillis() - frameStart;
		if (elapsed < 16)
			g_system->delayMillis(16 - elapsed);

		// Check exit: original exits when subtraction underflows or reaches 0
		if (fadeValue < (int)speed) {
			break;
		}
		fadeValue -= speed;
		if (fadeValue == 0) {
			break;
		}
	}

	// Final: write the full target palette (matches original's exit path)
	setViewPaletteSafely(g_engine->_pal);
	g_system->updateScreen();

	_currentFadeValue = -1;
	_fadeMode = FadeMode::None;
	_paletteDirty = false;
	endFadeCursorSuppression(g_engine->_pal);
	redraw();
}

void View1::beginFadeCursorSuppression() {
	if (_cursorSuppressedForFade) {
		return;
	}

	_cursorWasVisibleBeforeFade = CursorMan.isVisible();
	if (_cursorWasVisibleBeforeFade) {
		CursorMan.showMouse(false);
	}
	_cursorSuppressedForFade = true;
}

void View1::endFadeCursorSuppression(const byte *palette) {
	if (!_cursorSuppressedForFade) {
		return;
	}

	_cursorSuppressedForFade = false;
	updateCursor(palette);
	if (_cursorWasVisibleBeforeFade) {
		CursorMan.showMouse(true);
	}
	_cursorWasVisibleBeforeFade = false;
}

bool View1::msgFocus(const FocusMessage &msg) {
	// Common::fill(&_pal[0], &_pal[256 * 3], 0);
	//  _offset = 128;
	return true;
}

void View1::runInventoryPanelScriptIfPending(bool excludeCloseButton) {
	if (_pendingPanelRequest != kPanelRequestNone) {
		_uiBackgroundRestorePending = true;
	}
	redraw();
	if (_pendingPanelRequest == kPanelRequestNone) {
		return;
	}
	if (excludeCloseButton && _clickedButtonIndex == 6) {
		return;
	}
	g_engine->runScriptExecutor(false);
	_pendingPanelRequest = kPanelRequestNone;
}

bool View1::handleInventoryClick(const MouseDownMessage &msg) {
	// Binary handleInventoryClick (1008:4d07): only when g_wClickedButtonIndex == 0.
	if (_clickedButtonIndex != 0) {
		return true;
	}

	for (int i = 0; i < 6; i++) {
		const Common::Rect &current = _inventoryButtonLocations[i];
		if (!current.contains(msg._pos)) {
			continue;
		}

		_clickedButtonIndex = (uint16)(i + 1);
		InventoryButtonIndex buttonIndex = (InventoryButtonIndex)i;
		switch (buttonIndex) {
		case InventoryButtonIndex::Look: {
			g_engine->setCursorMode(Script::MouseMode::Look);
			updateCursor();
			break;
		}
		case InventoryButtonIndex::Hand: {
			g_engine->setCursorMode(Script::MouseMode::Use);
			updateCursor();
			break;
		}
		case InventoryButtonIndex::Up: {
			if (_inventoryScrollOffset > 0) {
				_inventoryScrollOffset -= 5;
			}
			break;
		}
		case InventoryButtonIndex::Down: {
			// Binary (1008:4d07): scroll down when offset+10 <= item count.
			if (_inventoryScrollOffset + 10 <= (uint16)_inventoryItems.size()) {
				_inventoryScrollOffset += 5;
			}
			break;
		}
		case InventoryButtonIndex::Drop: {
			// Binary handleInventoryClick button 5 / handleDialogueClick button 5.
			// Only active when mode == 0x17 (UseInventory) and an item is held.
			if (g_engine->_scriptExecutor->_cursorMode == Script::MouseMode::UseInventory && _activeInventoryItem != nullptr) {
				if (isInventorySourceProtagonist()) {
					// Protagonist's inventory: find a container in the current scene.
					// Binary iterates objects 1..0x200, finds first with:
					//   SceneIndex >= 0, SceneIndex == currentScene, hasInventoryIcon (+0x184)
					const uint16 currentScene = Scenes::instance()._currentSceneIndex;
					GameObject *container = nullptr;
					for (GameObject *obj : GameObjects::instance()._objects) {
						if (obj == nullptr)
							continue;
						if ((int16)obj->_sceneIndex < 0)
							continue;
						if (obj->_sceneIndex != currentScene)
							continue;
						// +0x184 = hasInventoryIcon: blob slot 0x13 is loaded
						if (0x13 >= obj->_blobs.size() || obj->_blobs[0x13].empty())
							continue;
						container = obj;
						break;
					}
					if (container != nullptr) {
						transferInventoryItem(_activeInventoryItem, container);
						_activeInventoryItem = nullptr;
						g_engine->setCursorMode(Script::MouseMode::Use);
						updateCursor();
						g_engine->_scriptExecutor->_inventoryCombineFlag = true;
						setInventorySource(_inventorySource);
					}
				} else {
					// External inventory (another character): take item to protagonist.
					// Binary: item.SceneIndex = g_wCurrentActorIndex + 0x400
					transferInventoryItem(_activeInventoryItem, GameObjects::instance().getProtagonistObject());
					_activeInventoryItem = nullptr;
					g_engine->setCursorMode(Script::MouseMode::Use);
					updateCursor();
					g_engine->_scriptExecutor->_inventoryActionFlag = true;
					setInventorySource(_inventorySource);
				}
			}
			break;
		}
		case InventoryButtonIndex::Close: {
			// Binary handleInventoryClick (1008:4d07) button 6: prepare close on release.
			if (g_engine->_scriptExecutor->_cursorMode == Script::MouseMode::UseInventory) {
				_savedCursorMode = Script::MouseMode::UseInventory;
				g_engine->_scriptExecutor->_interactedInventoryItemId = 0x400 + _activeInventoryItem->_index;
			} else {
				if (_savedCursorMode == Script::MouseMode::UseInventory) {
					_savedCursorMode = Script::MouseMode::Use;
				}
				g_engine->_scriptExecutor->_interactedInventoryItemId = 0;
			}
			g_engine->_scriptExecutor->_interactedObjectID = 0;
			redraw();
			return true;
		}
		}
		redraw();
		return true;
	}

	// Check if we hit an inventory item
	GameObject *clickedObject = getClickedInventoryItem(msg._pos);

	if (clickedObject != nullptr && g_engine->_scriptExecutor->_cursorMode == Script::MouseMode::Look) {
		g_engine->_scriptExecutor->_interactedObjectID = 0x400 + clickedObject->_index;
		g_engine->_scriptExecutor->_interactedInventoryItemId = 0;
		_clickedButtonIndex = 5;
		_pendingPanelRequest = kPanelRequestInventory;
		runInventoryPanelScriptIfPending(true);
		return true;
	}
	if (clickedObject != nullptr && g_engine->_scriptExecutor->_cursorMode == Script::MouseMode::Use) {
		_activeInventoryItem = clickedObject;
		g_engine->_scriptExecutor->_interactedObjectID = 0x400 + clickedObject->_index;
		AnimFrame *icon = getInventoryIcon(_activeInventoryItem);
		if (icon != nullptr) {
			// Original copies item icon frame into cursor array slot 0x17 (UseInventory)
			// so the cursor shows the picked-up item
			int cursorSlot = (int)Script::MouseMode::UseInventory - 1;
			g_engine->_imageResources[cursorSlot] = *icon;
			delete icon;
		}
		g_engine->setCursorMode(Script::MouseMode::UseInventory);
		updateCursor();
		return true;
	}
	if (_activeInventoryItem != nullptr && clickedObject != nullptr) {
		// Use item on item (combine): from handleInventoryClick grid hit-test, mode 0x17.
		// Binary sets interactedObjectId (source) + interactedInventoryItemId (target),
		// g_wPendingPanelRequest=1, then epilogue runScriptExecutor (clears pending after return).
		// Does NOT set g_wInventoryCombineFlag here (that's only in the Drop button path).
		g_engine->_scriptExecutor->_interactedObjectID = 0x400 + _activeInventoryItem->_index;
		g_engine->_scriptExecutor->_interactedInventoryItemId = 0x400 + clickedObject->_index;
		_clickedButtonIndex = 5;
		_pendingPanelRequest = kPanelRequestInventory;
		runInventoryPanelScriptIfPending(true);
	}

	return true;
}

// Binary: handleContainerInventoryClick (1008:5b0a)
// Container inventory uses the same 6-button layout as protagonist inventory but
// button 5 (Take) and button 6 (Close) have different semantics:
// - Button 5: Always transfers held item to protagonist (no container-in-scene search)
// - Button 6: Simple close - clears interaction IDs, no cursor mode save/restore
// Item clicks also differ: Look triggers runScriptExecutor immediately; no combine path.
bool View1::handleContainerInventoryClick(const MouseDownMessage &msg) {
	if (_clickedButtonIndex != 0) {
		return true;
	}

	for (int i = 0; i < 6; i++) {
		const Common::Rect &current = _inventoryButtonLocations[i];
		if (!current.contains(msg._pos)) {
			continue;
		}

		_clickedButtonIndex = (uint16)(i + 1);
		InventoryButtonIndex buttonIndex = (InventoryButtonIndex)i;
		switch (buttonIndex) {
		case InventoryButtonIndex::Look: {
			g_engine->setCursorMode(Script::MouseMode::Look);
			updateCursor();
			break;
		}
		case InventoryButtonIndex::Hand: {
			g_engine->setCursorMode(Script::MouseMode::Use);
			updateCursor();
			break;
		}
		case InventoryButtonIndex::Up: {
			if (_inventoryScrollOffset > 0) {
				_inventoryScrollOffset -= 5;
			}
			break;
		}
		case InventoryButtonIndex::Down: {
			if (_inventoryScrollOffset + 10 <= (uint16)_inventoryItems.size()) {
				_inventoryScrollOffset += 5;
			}
			break;
		}
		case InventoryButtonIndex::Drop: {
			// Binary button 5 (Take): transfers held item to protagonist.
			// Only active when mode == UseInventory (0x17) and an item is held.
			if (g_engine->_scriptExecutor->_cursorMode == Script::MouseMode::UseInventory && _activeInventoryItem != nullptr) {
				transferInventoryItem(_activeInventoryItem, GameObjects::instance().getProtagonistObject());
				_activeInventoryItem = nullptr;
				g_engine->setCursorMode(Script::MouseMode::Use);
				updateCursor();
				g_engine->_scriptExecutor->_inventoryActionFlag = true;
				setInventorySource(_inventorySource);
				if (hasScummVerbUI() && _scummUI)
					_scummUI->syncInventory();
			}
			break;
		}
		case InventoryButtonIndex::Close: {
			// Binary button 6: clear IDs on press; script resume on release
			// (handleInput state==3, clickedButtonIndex==6).
			g_engine->_scriptExecutor->_interactedInventoryItemId = 0;
			g_engine->_scriptExecutor->_interactedObjectID = 0;
			redraw();
			return true;
		}
		}
		redraw();
		return true;
	}

	// Item click handling - container has Look and Use but NO combine path.
	GameObject *clickedObject = getClickedInventoryItem(msg._pos);

	if (clickedObject != nullptr && g_engine->_scriptExecutor->_cursorMode == Script::MouseMode::Look) {
		g_engine->_scriptExecutor->_interactedObjectID = 0x400 + clickedObject->_index;
		g_engine->_scriptExecutor->_interactedInventoryItemId = 0;
		_clickedButtonIndex = 5;
		_pendingPanelRequest = kPanelRequestInventory;
		runInventoryPanelScriptIfPending(false);
		return true;
	}
	if (clickedObject != nullptr && g_engine->_scriptExecutor->_cursorMode == Script::MouseMode::Use) {
		// Binary: Use on container item picks up item as UseInventory cursor
		_activeInventoryItem = clickedObject;
		g_engine->_scriptExecutor->_interactedObjectID = 0x400 + clickedObject->_index;
		AnimFrame *icon = getInventoryIcon(_activeInventoryItem);
		if (icon != nullptr) {
			int cursorSlot = (int)Script::MouseMode::UseInventory - 1;
			g_engine->_imageResources[cursorSlot] = *icon;
			delete icon;
		}
		g_engine->setCursorMode(Script::MouseMode::UseInventory);
		updateCursor();
		if (hasScummVerbUI() && _scummUI)
			_scummUI->syncInventory();
		return true;
	}

	return true;
}

bool View1::handleActionBarClick(const MouseDownMessage &msg) {
	if (_clickedButtonIndex != 0) {
		return true;
	}

	for (int i = 0; i < 9; i++) {
		const Common::Rect &current = _mainMenuButtonLocations[i];
		if (!current.contains(msg._pos)) {
			continue;
		}

		_clickedButtonIndex = (uint16)(i + 1);
		MainMenuButtonIndex buttonIndex = (MainMenuButtonIndex)i;
		switch (buttonIndex) {
		case MainMenuButtonIndex::Talk: {
			_savedCursorMode = Script::MouseMode::Talk;
			g_engine->setCursorMode(Script::MouseMode::Talk);
			break;
		}
		case MainMenuButtonIndex::Look: {
			_savedCursorMode = Script::MouseMode::Look;
			g_engine->setCursorMode(Script::MouseMode::Look);
			break;
		}
		case MainMenuButtonIndex::Use: {
			_savedCursorMode = Script::MouseMode::Use;
			g_engine->setCursorMode(Script::MouseMode::Use);
			break;
		}
		case MainMenuButtonIndex::Walk: {
			_savedCursorMode = Script::MouseMode::Walk;
			g_engine->setCursorMode(Script::MouseMode::Walk);
			break;
		}
		case MainMenuButtonIndex::Inventory: {
			// Binary: handleActionBarClick button 5 sets g_wPendingPanelRequest = 1.
			// Panel closes on release; gameTick opens inventory when state returns to 0.
			_pendingPanelRequest = kPanelRequestInventory;
			break;
		}
		case MainMenuButtonIndex::InventoryUse: {
			if (_activeInventoryItem != nullptr) {
				_savedCursorMode = Script::MouseMode::UseInventory;
				g_engine->_scriptExecutor->_interactedInventoryItemId = 0x400 + _activeInventoryItem->_index;
				g_engine->setCursorMode(Script::MouseMode::UseInventory);
			}
			break;
		}
		case MainMenuButtonIndex::Map: {
			// Binary handleActionBarClick (1008:42dc) button 7: sets scene+0x61db=1
			// and saved cursor Walk; map load happens after action bar closes on release.
			if (!_helpButtonDisabled) {
				_pendingMapOpen = true;
				_savedCursorMode = Script::MouseMode::Walk;
			}
			break;
		}
		case MainMenuButtonIndex::SaveLoad: {
			_pendingPanelRequest = kPanelRequestSaveLoad;
			if (!ConfMan.getBool("original_menus")) {
				g_engine->openMainMenuDialog();
			}
			break;
		}
		case MainMenuButtonIndex::Close: {
			break;
		}
		}
		updateCursor();
		redraw();
		return true;
	}
	return true;
}

bool View1::handleHelpClick(const MouseDownMessage &msg) {
	Common::Rect screenRect(kScreenWidth, kGameHeight);
	if (screenRect.contains(msg._pos)) {
		uint8 depth = g_engine->_depthMap.getPixel(msg._pos.x, msg._pos.y);
		if (depth > 0 && depth < 0xFA) {
			// Binary: fileSeek(scene + 0x5DD7 + depth*4) = _mapSceneOffsets[depth-1]
			uint32 subSceneOffset = g_engine->_mapSceneOffsets[depth - 1];
			if (subSceneOffset != 0 && subSceneOffset < (uint32)g_engine->_fileStream->size()) {
				startFadeToBlack(8);
				Graphics::ManagedSurface preview = g_engine->readRLEImage(subSceneOffset, g_engine->_fileStream);
				_backgroundSurface.copyFrom(preview);
				// Read sub-scene palette
				g_engine->_fileStream->read(g_engine->_palVanilla, 0x300);
				g_engine->applyPaletteDarkening();
				// Read sub-scene depth map
				Graphics::ManagedSurface subDepth = g_engine->readRLEImage(g_engine->_fileStream->pos(), g_engine->_fileStream);
				g_engine->_depthMap.blitFrom(subDepth);
				startFading(8);
				redraw();
			}
		} else if (depth == 0xFF) {
			// Return to normal mode: restore scene visuals without resetting characters
			_currentMode = ViewMode::VM_GAME;
			g_engine->setCursorMode(_savedCursorMode);
			updateCursor();
			startFadeToBlack(8);
			_backgroundSurface.copyFrom(g_engine->_sceneBackground);
			memcpy(g_engine->_palVanilla, _savedPalVanilla, 256 * 3);
			g_engine->applyPaletteDarkening();
			g_engine->_depthMap.copyFrom(_savedDepthMap);
			startFading(8);
			redraw();
		}
	}
	return true;
}

bool View1::handleInput(const MouseDownMessage &msg) {
	if (msg._button == MouseMessage::MB_LEFT) {
		// Help mode (depth-based scene preview) from handleInput (1008:e8bf).
		// When currentMode == VM_HELP, clicking on the depth map previews scenes.
		if (_currentMode == ViewMode::VM_HELP) {
			return handleHelpClick(msg);
		}

		if (shouldShowScummVerbUI() && _scummUI && _scummUI->isPointInUI(msg._pos)) {
			if (g_engine->_scriptExecutor->_cursorMode != Script::MouseMode::Disabled) {
				_scummUI->handleClick(msg._pos, g_engine->_scriptExecutor->isExecuting());
				presentFrame();
			}
			return true;
		}

		// Handle original save/load panel clicks
		if (_uiPanelState == kUiPanelSaveLoad) {
			handleOriginalSaveLoadClick(msg._pos);
			return true;
		}

		// Binary (handleInput 1008:e8bf): when g_wScriptIsExecuting==0 there is NO
		// text-box-dismiss gate before the interaction check. The text box (if any)
		// is cleared as a side-effect of the script rerunning. Clear it here so the
		// UI updates immediately, but do NOT consume the click.
		if (_isShowingTextBox && !g_engine->_scriptExecutor->isExecuting()) {
			handleTextBoxInput();
		}

		if (_uiPanelState == kUiPanelInventory && !g_engine->_scriptExecutor->isExecuting()) {
			return handleInventoryClick(msg);
		}

		if (_uiPanelState == kUiPanelContainerInventory) {
			return handleContainerInventoryClick(msg);
		}

		if (_uiPanelState == kUiPanelActionBar && !g_engine->_scriptExecutor->isExecuting()) {
			return handleActionBarClick(msg);
		}

		// Handle interactions during script execution
		// From handleInput (1008:f1d4): clicks during script execution are ONLY processed
		// if cursor is not Disabled (0x1A). When cursor is Disabled (walk/wait in progress),
		// clicks are completely ignored.
		if (g_engine->_scriptExecutor->isExecuting() &&
			g_engine->_scriptExecutor->_cursorMode != Script::MouseMode::Disabled) {
			// Binary handleInput (1008:f1d4-f225): exact sequence of unconditional checks
			// 1. if g_wIsShowingTextBox != 0: handleTextBoxInput()
			// 2. if g_wIsShowingDialoguePanel != 0: dismissDialoguePanel()
			// 3. if scene+0x53B9 != 0: handleDialogueChoiceClick(mouseY, mouseX)
			// 4. if scene+0x53B9 == 0: set click state + runScriptExecutor()
			//
			// NOTE: Binary calls 1+2 unconditionally, then redraws the choice panel
			// every frame via animateDialogueChoicePortrait when scene+0x53B9 is still set.
			// ScummVM doesn't re-render per frame, so we only dismiss when the choice
			// click succeeds (clears _isDialogueChoiceInputActive) or no choice is active.
			if (_isDialogueChoiceInputActive) {
				handleDialogueChoiceClick(msg._pos.y, msg._pos.x);
				if (!_isDialogueChoiceInputActive) {
					// Choice was made - now dismiss visuals (binary does this before the click)
					handleTextBoxInput();
					dismissDialoguePanel();
				}
			} else {
				if (_isShowingTextBox) {
					handleTextBoxInput();
				}
				if (_isShowingDialoguePanel) {
					dismissDialoguePanel();
				}
			}
			if (!_isDialogueChoiceInputActive) {
				g_engine->_scriptExecutor->_scriptClickFlag = 0;
				g_engine->_scriptExecutor->_scriptClickX = (uint16)msg._pos.x;
				g_engine->_scriptExecutor->_scriptClickY = (uint16)msg._pos.y;
				g_engine->_scriptExecutor->_scriptClickResult = 1;
				g_engine->runScriptExecutor();
			}
			return true;
		}

		// Binary handleInput (1008:e8bf): when g_wScriptIsExecuting != 0 and cursor
		// is Disabled (0x1A), ALL input is ignored. Only the section above (for
		// text box/dialogue clicks with non-disabled cursor) processes clicks.
		if (g_engine->_scriptExecutor->isExecuting()) {
			return true;
		}

		if (g_engine->_scriptExecutor->_cursorMode == Script::MouseMode::Walk) {
			if (shouldShowScummVerbUI() && msg._pos.y >= kGameHeight)
				return true;

			Character *protagonist = getCharacterByIndex(Scenes::instance()._currentActorIndex);
			if (protagonist == nullptr) {
				debugC(kDebugScript, "Ignoring walk click without active actor character in the scene");
				return true;
			}

			Common::Point target = msg._pos;
			Common::Point charPos = protagonist->getPosition();

			// Snap target to nearest walkable position (1008:9be2)
			int16 targetY = target.y;
			int16 targetX = target.x;
			g_engine->snapToWalkablePosition(&targetY, &targetX, charPos.y, charPos.x);
			target.x = targetX;
			target.y = targetY;

			// handleInput (1008:e8bf): isPathWalkable(targetY, targetX, charY, charX).
			// calculatePath only when direct line fails AND target tile is walkable (< 0xC8).
			protagonist->_pathFinalDestination = target;
			protagonist->_currentPathIndex = 0;
			protagonist->_path.clear();

			const bool directPath = g_engine->isPathWalkable(target.y, target.x, charPos.y, charPos.x);
			if (directPath || Macs2Engine::isWalkabilityBlocking(g_engine->getWalkabilityAt(target.y, target.x))) {
				protagonist->_targetPosition = target;
			} else {
				const bool found = protagonist->calculatePath(target);
				if (!found) {
					protagonist->_targetPosition = target;
				}
			}
			protagonist->_stepDeltaX = abs(protagonist->_targetPosition.x - charPos.x);
			protagonist->_stepDeltaY = abs(protagonist->_targetPosition.y - charPos.y);
			protagonist->_stepError = 0;
			protagonist->_stepDirectionSet = false;
			g_engine->_scriptExecutor->saveWalkRuntime(protagonist, protagonist->_gameObject);
			return true;
		}

		// Check if we hit something
		if (shouldShowScummVerbUI() && msg._pos.y >= kGameHeight)
			return true;

		// Original order: getHotspotAtPoint first, then drawCharactersAndHitTest overrides.
		// Our order (objects first, fallback to background) produces the same result.
		uint16 index = getHitObjectID(Common::Point(msg._pos.x, msg._pos.y));
		if (index == 0) {
			index = g_engine->getHotspotAtPoint(msg._pos);
		}
		if (index != 0) {
			debugC(kDebugScript, "*** New interaction started");

			// Binary (handleInput 1008:ef2d): stop character movement before interaction.
			// Sets runtime target/finalDest to current position, clears path state.
			Character *protagonist = getCharacterByIndex(Scenes::instance()._currentActorIndex);
			if (protagonist != nullptr) {
				Common::Point pos = protagonist->getPosition();
				protagonist->_targetPosition = pos;
				protagonist->_pathFinalDestination = pos;
				protagonist->_path.clear();
				protagonist->_currentPathIndex = 0;
			}

			// Binary (handleInput 1008:ef8f): if mode != 0x17, clear inventory item ID.
			// Note: the binary does NOT touch g_wInventoryActionFlag here.
			if (g_engine->_scriptExecutor->_cursorMode != Script::MouseMode::UseInventory) {
				g_engine->_scriptExecutor->_interactedInventoryItemId = 0;
				_activeInventoryItem = nullptr;
			}

			g_engine->_scriptExecutor->_interactedObjectID = index;

			// Binary: runScriptExecutor() - internally rewinds scene script when
			// g_wScriptIsExecuting==0 (which it is here, since we're in the
			// "not executing" branch of handleInput).
			g_engine->runScriptExecutor(false);

			// Binary: only g_wInteractedObjectId is cleared after runScriptExecutor.
			g_engine->_scriptExecutor->_interactedObjectID = 0;
		}
		return true;
	} else if (msg._button == MouseMessage::MB_RIGHT) {
		// Map mode: right-click does nothing (binary: only left-click processed in map mode)
		if (_currentMode == ViewMode::VM_HELP) {
			return true;
		}
		// Handle no other interactions during a script
		if (g_engine->_scriptExecutor->isExecuting()) {
			// From handleInput: right-click during script execution opens the
			// map/save panel ONLY if none of these are active:
			// Binary: g_wIsShowingDialoguePanel, g_wIsSceneInitRun, scene+0x53B9,
			//         g_wIsShowingTextBox, overlay, sound, music, adlib
			if (!_isShowingDialoguePanel && !_isDialogueChoiceInputActive &&
				!_isShowingTextBox &&
				!g_engine->_scriptExecutor->_overlayTextStageActive &&
				!g_engine->_scriptExecutor->_waitForPcmSound &&
				!g_engine->_scriptExecutor->_waitForMusicControl &&
				!g_engine->_scriptExecutor->_waitForAdlibReady &&
				g_engine->_scriptExecutor->canOpenSaveMenu()) {
				if (ConfMan.getBool("original_menus")) {
					// Binary handleInput (1008:f2af): saves cursor mode before opening panel
					_savedCursorMode = g_engine->_scriptExecutor->_cursorMode;
					openOriginalSaveLoadPanel();
				} else {
					// Binary save/load path saves cursor then sets PanelCursor (0x19).
					_savedCursorMode = g_engine->_scriptExecutor->_cursorMode;
					g_engine->setCursorMode(Script::MouseMode::PanelCursor);
					g_engine->openMainMenuDialog();
					updateCursor();
				}
			}
			return true;
		}

		// From handleInput (1008:e8bf): right-click when not executing and cursor != Disabled
		// opens the action bar at the mouse position (or cycles verbs with SCUMM UI).
		if (g_engine->_scriptExecutor->_cursorMode == Script::MouseMode::Disabled) {
			return true;
		}
		if (hasScummVerbUI()) {
			if (shouldShowScummVerbUI()) {
				g_engine->nextCursorMode();
				_activeInventoryItem = nullptr;
				g_engine->_scriptExecutor->_interactedInventoryItemId = 0;
				if (_scummUI)
					_scummUI->syncActiveVerbFromCursorMode();
				updateCursor();
				presentFrame();
			}
			return true;
		}
		if (_uiPanelState != kUiPanelActionBar) {
			openMainMenu(msg._pos);
		} else {
			_uiPanelState = kUiPanelNone;
			_clickedButtonIndex = 0;
			_uiBackgroundRestorePending = false;
			g_engine->setCursorMode(_savedCursorMode);
			redraw();
		}
		updateCursor();
		return true;
	}
	return false;
}
bool View1::msgMouseDown(const MouseDownMessage &msg) {
	return handleInput(msg);
}

void View1::finishPanelCloseAfterRelease(UiPanelState closedFromState) {
	if (_pendingPanelRequest == kPanelRequestNone) {
		g_engine->setCursorMode(_savedCursorMode);
		updateCursor();
	}
	_uiBackgroundRestorePending = false;
	redraw();

	// Binary handleInput (1008:e8bf): runScriptExecutor after close unless state was 1 or 4.
	if (closedFromState != kUiPanelActionBar && closedFromState != kUiPanelSaveLoad) {
		g_engine->runScriptExecutor();
	}
	g_engine->_scriptExecutor->_inventoryCombineFlag = false;
	g_engine->_scriptExecutor->_inventoryActionFlag = false;
}

bool View1::handlePanelRelease(const MouseUpMessage &msg) {
	(void)msg;

	if (_uiPanelState == kUiPanelNone || _clickedButtonIndex == 0) {
		return false;
	}

	// Binary handleInput (1008:e8bf): action bar/inventory panel release is only handled
	// when g_wScriptIsExecuting==0 and g_wCursorMode!=0x1A. Save/load (state 4) still
	// closes during script execution.
	if (g_engine->_scriptExecutor->isExecuting() &&
		_uiPanelState != kUiPanelSaveLoad) {
		return true;
	}

	const UiPanelState previousState = _uiPanelState;
	bool shouldClose = false;

	switch (_uiPanelState) {
	case kUiPanelActionBar:
		shouldClose = true;
		break;
	case kUiPanelInventory:
	case kUiPanelContainerInventory:
		shouldClose = (_clickedButtonIndex == 6);
		break;
	case kUiPanelSaveLoad:
		shouldClose = (_clickedButtonIndex == 7);
		break;
	default:
		break;
	}

	if (!shouldClose) {
		// Binary handleInput (1008:e8bf): always clears g_wClickedButtonIndex on release.
		_clickedButtonIndex = 0;
		redraw();
		return true;
	}

	const uint16 releasedButton = _clickedButtonIndex;
	_clickedButtonIndex = 0;

	if (previousState == kUiPanelContainerInventory && releasedButton == 6) {
		_uiPanelState = kUiPanelNone;
		_inventoryScrollOffset = 0;
		_activeInventoryItem = nullptr;
		g_engine->_scriptExecutor->_inventoryActionFlag = false;
		g_engine->_scriptExecutor->_inventoryCombineFlag = false;
		if (!isInventorySourceProtagonist()) {
			setInventorySource(GameObjects::instance().getProtagonistObject());
		}
		g_engine->setCursorMode(g_engine->_scriptExecutor->_savedExternalInventoryMouseMode);
		updateCursor();
		g_engine->_scriptExecutor->_hasPendingExternalInventoryResume = false;
		g_engine->_scriptExecutor->_externalInventorySourceObjectID = 0;
		g_engine->_scriptExecutor->restoreOpenInventoryScriptContext();
		finishPanelCloseAfterRelease(previousState);
		return true;
	}

	if (previousState == kUiPanelInventory && releasedButton == 6) {
		_uiPanelState = kUiPanelNone;
		_inventoryScrollOffset = 0;
		g_engine->setCursorMode(_savedCursorMode);
		updateCursor();
		finishPanelCloseAfterRelease(previousState);
		return true;
	}

	if (previousState == kUiPanelActionBar) {
		_uiPanelState = kUiPanelNone;
		const bool openMap = _pendingMapOpen;
		_pendingMapOpen = false;
		finishPanelCloseAfterRelease(previousState);
		if (openMap) {
			enterMapMode();
		}
		return true;
	}

	if (previousState == kUiPanelSaveLoad && releasedButton == 7) {
		closeOriginalSaveLoadPanel();
		_uiBackgroundRestorePending = false;
		g_engine->_scriptExecutor->_inventoryCombineFlag = false;
		g_engine->_scriptExecutor->_inventoryActionFlag = false;
		redraw();
		return true;
	}

	return false;
}

bool View1::msgMouseUp(const MouseUpMessage &msg) {
	return handlePanelRelease(msg);
}

bool View1::msgMouseMove(const MouseMoveMessage &msg) {
	_hoverAreaId = g_engine->_scriptExecutor->getAreaAtPoint(msg._pos.x, msg._pos.y);
	_hoverHotspotId = g_engine->getHotspotAtPoint(msg._pos);

	if (shouldShowScummVerbUI() && _scummUI) {
		if (_scummUI->isPointInUI(msg._pos)) {
			_scummUI->handleMouseMove(msg._pos);
		} else if (msg._pos.y < kGameHeight) {
			_scummUI->clearSentenceObject();
			uint16 index = getHitObjectID(msg._pos);
			if (index == 0)
				index = g_engine->getHotspotAtPoint(msg._pos);
			if (index != 0 && index >= 0x400) {
				const uint16 objIndex = index - 0x400;
				if (objIndex < GameObjects::instance()._objectNames.size()) {
					const Common::String &name = GameObjects::instance()._objectNames[objIndex];
					if (!name.empty())
						_scummUI->updateSentenceLine(name);
				}
			}
		}
	}

	return true;
}

bool View1::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case Macs2::kMacs2ActionInteract: {
		Common::Point pos = g_system->getEventManager()->getMousePos();
		return msgMouseDown(MouseDownMessage(MouseDownMessage::MB_LEFT, pos));
	}
	case Macs2::kMacs2ActionCursorMode: {
		Common::Point pos = g_system->getEventManager()->getMousePos();
		return msgMouseDown(MouseDownMessage(MouseDownMessage::MB_RIGHT, pos));
	}
	case Macs2::kMacs2ActionSkip:
		return msgKeypress(KeypressMessage(Common::KeyState(Common::KEYCODE_ESCAPE)));
	case Macs2::kMacs2ActionInventory:
		return msgKeypress(KeypressMessage(Common::KeyState(Common::KEYCODE_i, 'i')));
	case Macs2::kMacs2ActionMenu:
		return msgKeypress(KeypressMessage(Common::KeyState(Common::KEYCODE_n, 'n')));
	case Macs2::kMacs2ActionGameSpeed:
		g_engine->_gameSpeedMode = (g_engine->_gameSpeedMode + 1) % 3;
		debug("Game speed mode: %u", g_engine->_gameSpeedMode);
		return true;
	case Macs2::kMacs2ActionOpenGMM:
		// Binary has no GMM; mirror save/load panel cursor handling when a script wait is active.
		if (g_engine->_scriptExecutor->isExecuting()) {
			if (g_engine->_scriptExecutor->_cursorMode == Script::MouseMode::Disabled) {
				_savedCursorMode = g_engine->_scriptExecutor->_cursorMode;
				g_engine->setCursorMode(Script::MouseMode::PanelCursor);
			}
		} else if (g_engine->_scriptExecutor->_cursorMode == Script::MouseMode::Disabled) {
			return true;
		}
		g_engine->openMainMenuDialog();
		updateCursor();
		return true;
	case Macs2::kMacs2ActionHelp:
		if (!_helpButtonDisabled && _currentMode != ViewMode::VM_HELP &&
			!g_engine->_scriptExecutor->isExecuting() &&
			g_engine->_scriptExecutor->_cursorMode != Script::MouseMode::Disabled) {
			enterMapMode();
		}
		return true;
	default:
		break;
	}
	return false;
}

bool View1::msgKeypress(const KeypressMessage &msg) {
	// Button 8 skip from handleInput (1008:e8bf):
	// ESC during a skippable script section fast-forwards through opcodes
	// until opcode 0x1D is found (which clears the skippable flag).
	if (msg.keycode == Common::KEYCODE_ESCAPE &&
		g_engine->_scriptExecutor->_scriptSkippable &&
		g_engine->_scriptExecutor->isExecuting()) {
		if (g_engine->_scriptExecutor->skipToEndOfSkippableSection()) {
			g_engine->_scriptExecutor->run();
		}
		return true;
	}

	if ((_isShowingTextBox || _isShowingDialoguePanel) && !_isDialogueChoiceInputActive) {
		handleTextBoxInput();
		dismissDialoguePanel();
		g_engine->runScriptExecutor();
		return true;
	}

	// Binary (handleInput 1008:edff): UI panels only open when not executing and cursor != Disabled.
	if (!g_engine->_scriptExecutor->isExecuting() && g_engine->_scriptExecutor->_cursorMode != Script::MouseMode::Disabled) {
		if (msg.ascii == (uint16)'i') {
			if (hasScummVerbUI()) {
				if (_uiPanelState == kUiPanelContainerInventory)
					closeInventory();
			} else if (_uiPanelState != kUiPanelInventory) {
				openInventory(GameObjects::instance().getProtagonistObject());
			} else {
				closeInventory();
			}
		} else if (msg.ascii == 'n') {
			if (hasScummVerbUI()) {
				if (shouldShowScummVerbUI()) {
					g_engine->nextCursorMode();
					_activeInventoryItem = nullptr;
					g_engine->_scriptExecutor->_interactedInventoryItemId = 0;
					if (_scummUI)
						_scummUI->syncActiveVerbFromCursorMode();
					updateCursor();
					presentFrame();
				}
			} else {
				Common::Point mousePos = g_system->getEventManager()->getMousePos();
				openMainMenu(mousePos);
			}
		}
	}

	if (msg.ascii >= '1' && msg.ascii <= '9') {
		// Select a visible dialogue option by number key.
		// Register a dialogue choice and act upon it
		uint8 numberPressed = msg.ascii - '1' + 1;
		if (numberPressed <= _dialogueChoiceCount && _isDialogueChoiceInputActive) {
			handleTextBoxInput();
			dismissDialoguePanel();
			_isDialogueChoiceInputActive = false;
			triggerDialogueChoice(numberPressed);
			g_engine->_scriptExecutor->_scriptClickFlag = 0;
			g_engine->_scriptExecutor->_scriptClickResult = 1;
			g_engine->runScriptExecutor();
		}
	}

	return true;
}

void View1::draw() {
	if (_paletteDirty && _currentFadeValue < 0) {
		setViewPaletteSafely(g_engine->_pal);
		_paletteDirty = false;
	}

	Graphics::ManagedSurface s = getSurface();

	s.blitFrom(_backgroundSurface);

	// In map mode, only the background (map image) is shown - no characters/animations/UI.
	if (_currentMode == ViewMode::VM_HELP) {
		return;
	}

	// Handle highlighting

	drawAllCharacters(&s, true);
	drawOverlayTextEntries();
	if (shouldDrawPathfindingOverlay()) {
		drawPathfindingPoints(s);
	}
	if (shouldDrawPathOverlay()) {
		drawPath(s);
	}
	if (shouldDrawDebugOutputOverlay()) {
		drawDebugOutput(s);
	}

	// Get mouse position
	Common::Point mousePos = g_system->getEventManager()->getMousePos();

	if (_isShowingTextBox || _isShowingDialoguePanel) {
		showStringBox(_drawnStringBox);
		if (_isDialogueChoiceInputActive && g_engine->enhancementEnabled(kEnhUIUX)) {
			int lineHeight = g_engine->maxGlyphHeight + 2;
			int firstLineY = _stringBoxPosition.y + 9;
			int relY = mousePos.y - firstLineY;
			if (relY >= 0) {
				int hoveredLine = relY / lineHeight;
				int cumulativeLines = 0;
				for (uint i = 0; i < _dialogueChoiceLineCounts.size(); i++) {
					if (hoveredLine < cumulativeLines + _dialogueChoiceLineCounts[i]) {
						int highlightY = firstLineY + cumulativeLines * lineHeight;
						renderString(_stringBoxPosition.x + 2, highlightY, ".");
						break;
					}
					cumulativeLines += _dialogueChoiceLineCounts[i];
				}
			}
		}
		if (currentSpeechActData.speaker != nullptr) {
			drawCurrentSpeaker(s);
		}
	}

	// We keep the inventory on but don't draw it in case we display a string
	// i.e. a description of an item
	const bool showProtagonistInventory = _uiPanelState == kUiPanelInventory && !hasScummVerbUI();
	if ((showProtagonistInventory || _uiPanelState == kUiPanelContainerInventory) && !_isShowingTextBox && !_isShowingDialoguePanel) {
		drawInventory(s);
	}

	if (_uiPanelState == kUiPanelActionBar) {
		drawMainMenu(s);
	}

	if (_uiPanelState == kUiPanelSaveLoad) {
		drawOriginalSaveLoadPanel(s);
	}

	// Active inventory item is now shown via the cursor (UpdateCursor uses _cursorData slot 0x16)

	if (_uiPanelState == kUiPanelActionBar && g_engine->enhancementEnabled(kEnhUIUX)) {
		for (int i = 0; i < (int)_mainMenuButtonLocations.size(); i++) {
			if (_mainMenuButtonLocations[i].contains(mousePos)) {
				static const char *const buttonNames[] = {
					"Reden", "Schauen", "Benutzen", "Gehen", "Inventar",
					"Gegenstand benutzen", "Hilfe", "Speichern/Laden", "Schliessen"};
				renderString(mousePos.x + 20, mousePos.y + 20, buttonNames[i]);
				break;
			}
		}
	}

	if (_uiPanelState == kUiPanelInventory && g_engine->enhancementEnabled(kEnhUIUX)) {
		for (int i = 0; i < 6; i++) {
			if (_inventoryButtonLocations[i].contains(mousePos)) {
				static const char *const buttonNames[] = {
					"Schauen", "Benutzen", "Hoch", "Runter", "Ablegen", "Schliessen"};
				renderString(mousePos.x + 20, mousePos.y + 20, buttonNames[i]);
				break;
			}
		}

		GameObject *hoveredObject = getClickedInventoryItem(mousePos);
		if (hoveredObject != nullptr) {
			Common::String name = GameObjects::instance()._objectNames[hoveredObject->_index];
			if (!name.empty()) {
				renderString(mousePos.x + 20, mousePos.y + 20, name);
			} else {
				renderString(mousePos.x + 20, mousePos.y + 20, Common::String::format("%2.x", hoveredObject->_index));
			}
		}
	}

	if (_uiPanelState == kUiPanelSaveLoad && g_engine->enhancementEnabled(kEnhUIUX)) {
		for (int i = 0; i < 7; i++) {
			if (_saveLoadButtonRects[i].contains(mousePos)) {
				static const char *const buttonNames[] = {
					"Laden", "Speichern", "Musik an/aus",
					"Textur Men\x81 \x84ndern", ("Zur\x81" "ck zum Spielbeginn"),
					"Exit to DOS", "Schliessen"};
				renderString(mousePos.x + 20, mousePos.y + 20, buttonNames[i]);
				break;
			}
		}
	}

	if (hasScummVerbUI()) {
		ensureScummVerbUI();
		if (_scummUI && !_isShowingTextBox && !_isShowingDialoguePanel) {
			Graphics::ManagedSurface fullScreen(*g_events->getScreen(), Common::Rect(0, 0, kScreenWidth, kScreenHeight));
			if (shouldShowScummVerbUI()) {
				_scummUI->draw(fullScreen);
			} else {
				fullScreen.fillRect(Common::Rect(0, kGameHeight, kScreenWidth, kScreenHeight), 0);
			}
		}
	}
}

void View1::drawSceneUpdate() {
	draw();
	_needsRedraw = false;
}

bool View1::tick() {
	// TODO: Check if this pattern works or it would be better different
	// TODO: Check if loading also works with this pattern
	if (!_started) {
		g_engine->changeScene(Scenes::instance()._currentSceneIndex);
		_started = true;
	}

	// Map mode: no game logic runs (binary: handleInput skips everything when 0x61db set)
	if (_currentMode == ViewMode::VM_HELP) {
		redraw();
		return true;
	}
	// Cycle the palette
	++_offset;

	// Music fade tick from gameTick (1008:e556).
	// Processes volume fade in/out each frame when active.
	Script::ScriptExecutor *se = g_engine->_scriptExecutor;
	if (se->_activeMusicSlot != 0 && se->_musicControlMode != 0) {
		const uint16 musicStep = MAX<uint16>(se->_musicControlStep, 1);
		if (se->_musicControlMode == 1) {
			// Fade out: volume -= step
			int vol = (int)se->_musicControlVolume - (int)musicStep;
			if (vol < 1) {
				se->_musicControlMode = 0;
				se->_musicControlVolume = 0;
			} else {
				se->_musicControlVolume = vol;
			}
			g_engine->getAdlib()->setVolume(g_engine->scaledMusicVolume(se->_musicControlVolume));
		} else {
			// Fade in: volume += step. When >= 63: stop music.
			int vol = (int)se->_musicControlVolume + (int)musicStep;
			if (vol >= 0x3F) {
				se->_musicControlMode = 0;
				se->_activeMusicSlot = 0;
				g_engine->getAdlib()->stopMusic();
			} else {
				se->_musicControlVolume = vol;
				g_engine->getAdlib()->setVolume(g_engine->scaledMusicVolume(se->_musicControlVolume));
			}
		}
	}

	// Below is redundant since we're only cycling the palette, but it demonstrates
	// how to trigger the view to do further draws after the first time, since views
	// don't automatically keep redrawing unless you tell it to
	// if ((_offset % 256) == 0)
	//	redraw();

	// Background animation sequencing happens in drawBackgroundAnimations via
	// drawAnimFrame(2, ...) semantics (1008:929c). Do not advance here - a
	// separate tick advance ran before draw and caused scriptChangeAnimation
	// (1008:b6be) door/state frames to be skipped on the first visible frame.

	// Mode-dependent palette brighten effect, matching gameTick (1008:e556):
	//   g_wBgAnimTickCounter is incremented every tick.
	//   Mode 3: when counter > 1  -> reset and call updateBackgroundAnimations.
	//   Mode 2: when counter > 0x27 -> reset and call updateBackgroundAnimations.
	// The called routine (updateBackgroundAnimationPalette) only does work for
	// mode 2 (gradual brighten); the mode-3 trigger is a no-op in the binary too.
	_bgAnimTickCounter++;
	if (_bgAnimTickCounter > 1 && g_engine->_scenePaletteMode == 3) {
		_bgAnimTickCounter = 0;
		g_engine->updateBackgroundAnimationPalette();
	}
	if (_bgAnimTickCounter > 0x27 && g_engine->_scenePaletteMode == 2) {
		_bgAnimTickCounter = 0;
		g_engine->updateBackgroundAnimationPalette();
	}

	// Advance portrait animation once per tick (matching handleDialogueInput 1008:b4bd)
	if (_isShowingDialoguePanel && currentSpeechActData.speaker != nullptr && currentSpeechActData.mouthAnimActive) {
		Character *speaker = currentSpeechActData.speaker;
		if (currentSpeechActData.mouthAnimCounter < 1) {
			// counter < 1: advance alternate blob (Blobs[18]) with mode 2
			if (speaker->_gameObject->_blobs.size() > 18 && !speaker->_gameObject->_blobs[18].empty()) {
				BackgroundAnimationBlob::advanceAnimFrame(speaker->_gameObject->_blobs[18], true, 2);
			}
		} else {
			currentSpeechActData.mouthAnimCounter--;
			if (currentSpeechActData.mouthAnimCounter < 1) {
				// just hit 0: reset alternate blob (Blobs[18]) with mode 1
				if (speaker->_gameObject->_blobs.size() > 18 && !speaker->_gameObject->_blobs[18].empty()) {
					BackgroundAnimationBlob::advanceAnimFrame(speaker->_gameObject->_blobs[18], true, 1);
				}
			} else {
				// counter > 0: advance primary blob (Blobs[17]) with mode 2
				if (speaker->_gameObject->_blobs.size() > 17 && !speaker->_gameObject->_blobs[17].empty()) {
					BackgroundAnimationBlob::advanceAnimFrame(speaker->_gameObject->_blobs[17], true, 2);
				}
			}
		}
	}

	// Binary gameTick (1008:e556): process pending panel requests when state is idle.
	if (_uiPanelState == kUiPanelNone && _pendingPanelRequest != kPanelRequestNone) {
		switch (_pendingPanelRequest) {
		case kPanelRequestInventory:
			_pendingPanelRequest = kPanelRequestNone;
			openInventory(GameObjects::instance().getProtagonistObject());
			break;
		case kPanelRequestContainerInventory:
			_pendingPanelRequest = kPanelRequestNone;
			openInventory(_inventorySource);
			break;
		case kPanelRequestSaveLoad:
			_pendingPanelRequest = kPanelRequestNone;
			openOriginalSaveLoadPanel();
			break;
		default:
			_pendingPanelRequest = kPanelRequestNone;
			break;
		}
	}

	// Binary gameTick: drawScene(1) (which calls walkAlongPath for all characters)
	// is only called when g_wUiPanelState == 0 and no dialogue panel is showing.
	// Binary gate: g_wUiPanelState == 0 && g_wIsShowingDialoguePanel == 0
	// Binary gameTick: drawScene during dialogue/text wait is gated; if movement
	// finished fires while paused on a clickable wait, don't resume the script.
	if (_uiPanelState == kUiPanelNone && !_isShowingDialoguePanel && !_isShowingTextBox) {
		Script::ScriptExecutor *executor = g_engine->_scriptExecutor;

		// Binary gameTick cascading if/else structure:
		// if (frameWaitCounter == 0) { walkTarget / sound / music / adlib }
		// else { drawScene(1); if counter==0 runScriptExecutor(); }
		// Binary gameTick (1008:e556): each wait branch calls drawScene(1) before
		// checking its completion flag and optionally resuming the script.
		if (!executor->isFrameWaitActive()) {
			// Binary gameTick (1008:e752) walk-wait polling:
			// When g_wWalkTargetObjectIndex > 0, check each frame if the character
			// has reached its target position AND vertical offset matches.
			// Binary uses exact equality: charPos == runtime.finalDest.
			// This works because walkAlongPath snaps pos/finalDest on arrival.
			uint16 walkTarget = executor->_walkTargetObjectIndex;
			if (walkTarget > 0) {
				drawSceneUpdate();
				GameObject *walkObject = GameObjects::getObjectByIndex(walkTarget);
				if (walkObject == nullptr) {
					executor->setScriptError(0x19);
					executor->_walkTargetObjectIndex = 0;
				} else if (walkObject->_dataOffset == 0) {
					executor->setScriptError(2);
					executor->_walkTargetObjectIndex = 0;
				} else {
					Character *c = getCharacterByIndex(walkTarget);
					bool walkComplete = false;
					if (c != nullptr) {
						Common::Point pos = c->getPosition();
						// Binary: walkAlongPath guarantees pos == finalDest on arrival.
						if (pos.x == c->_pathFinalDestination.x && pos.y == c->_pathFinalDestination.y) {
							// Binary gameTick (1008:e752): also requires vertical offset match when
							// runtime+0x21D >= 0. Recover if motion target was never seeded.
							if (c->hasPendingVerticalMotion() &&
								c->_motionVerticalOffsetDelta == 0 && c->_motionDistanceUnits == 0 &&
								c->_motionProgress == 0) {
								c->_motionTargetVerticalOffset = walkObject->_verticalOffsetScale;
							}
							walkComplete = !c->hasPendingVerticalMotion();
						}
					} else if (walkObject->_storedWalkRuntime.valid) {
						// Binary gameTick (1008:e752): polls object table pos vs runtime finalDest;
						// no on-screen Character is required (e.g. after moveObject to another scene).
						const GameObject::StoredWalkRuntime &rt = walkObject->_storedWalkRuntime;
						if (walkObject->_position.x == rt.pathFinalDestination.x &&
							walkObject->_position.y == rt.pathFinalDestination.y) {
							if ((int16)rt.motionTargetVerticalOffset < 0 ||
								rt.motionTargetVerticalOffset == walkObject->_verticalOffsetScale) {
								walkComplete = true;
							}
						}
					}
					if (walkComplete) {
						if (!executor->_pickupInProgress) {
							debugC(kDebugScript, "waitForWalk complete obj=%u", walkTarget);
							executor->debugLogActorWalkState("waitForWalk complete");
							executor->_walkTargetObjectIndex = 0;
							g_engine->runScriptExecutor();
						} else if (c != nullptr && c->_gameObject->_orientation != 0x11) {
							// Binary: pickup in progress, trigger pickup animation.
							// Save current orientation so it can be restored after pickup.
							c->_previousOrientation = c->_gameObject->_orientation;
							c->_gameObject->_orientation = 0x11;
						}
					}
				}
			} else if (executor->_waitForPcmSound) {
				drawSceneUpdate();
				if (!g_engine->isCurrentSoundPlaying()) {
					debugC(kDebugScript, "waitForSound complete");
					executor->debugLogActorWalkState("waitForSound complete");
					executor->_waitForPcmSound = false;
					g_engine->runScriptExecutor();
				}
			} else if (executor->_waitForMusicControl) {
				drawSceneUpdate();
				if (executor->_musicControlMode == 0) {
					executor->_waitForMusicControl = false;
					g_engine->runScriptExecutor();
				}
			} else if (executor->_waitForAdlibReady) {
				drawSceneUpdate();
				if (g_engine->getAdlib()->isPlaybackReady()) {
					executor->_waitForAdlibReady = false;
					g_engine->runScriptExecutor();
				}
			}
		} else {
			drawSceneUpdate();
			if (executor->getFrameWaitCounter() == 0) {
				debugC(kDebugScript, "frameWait complete");
				executor->debugLogActorWalkState("frameWait complete");
				executor->endFrameWait();
				g_engine->runScriptExecutor();
			}
		}

		// Binary gameTick (1008:e556): drawScene(1) when not executing is handled by redraw().
	}

	redraw();
	return true;
}

void View1::flushPendingCharacterDeletes() {
	for (Character *c : _pendingCharacterDeletes) {
		delete c;
	}
	_pendingCharacterDeletes.clear();
}

void View1::drawAllCharacters(Graphics::ManagedSurface *surface, bool fullUpdate) {
	// drawAllCharacters @ 1008:90a2
	g_engine->_movementFinishedFlag = false;
	sortObjectListByY();
	rebuildCharacterLookupTable();

	const uint16 sortedCount = _sortedObjectCount;
	Script::ScriptExecutor *executor = g_engine->_scriptExecutor;

	// --- Pass 1 (1008:90a2): erase previous sprite rects, walkAlongPath, pickup ---
	if (fullUpdate && sortedCount > 0) {
		for (uint16 local_c = 1; local_c <= sortedCount; local_c++) {
			const uint16 objectIndex = _sortedObjectIndices[local_c];
			GameObject *obj = GameObjects::getObjectByIndex(objectIndex);
			if (obj == nullptr || obj->_index != objectIndex)
				continue;

			// Binary pass 1: runtime+0x20D..+0x213 from object runtime+0x225..+0x22B.
			const int32 eraseLeft = obj->_lastDrawX;
			const int32 eraseTop = obj->_lastDrawY;
			const int32 eraseRight = eraseLeft + (int32)obj->_lastDrawWidth + 1;
			const int32 eraseBottom = eraseTop + (int32)obj->_lastDrawHeight + 1;
			obj->_dirtyLeft = (int16)eraseLeft;
			obj->_dirtyTop = (int16)eraseTop;
			obj->_dirtyRight = (int16)eraseRight;
			obj->_dirtyBottom = (int16)eraseBottom;

			if (surface != nullptr) {
				Common::Rect eraseRect;
				if (buildClippedEraseRect(eraseLeft, eraseTop, obj->_lastDrawWidth,
										  obj->_lastDrawHeight,
										  _backgroundSurface.w, _backgroundSurface.h, eraseRect)) {
					surface->blitFrom(_backgroundSurface, eraseRect,
									  Common::Point(eraseRect.left, eraseRect.top));
				} else if (eraseTop >= _backgroundSurface.h || eraseBottom < 0 ||
						   eraseLeft >= _backgroundSurface.w || eraseRight < 0 ||
						   obj->_lastDrawWidth > 400 || obj->_lastDrawHeight > 300) {
					resetObjectDrawBounds(obj);
				}
			}

			Character *current = _characterByObjectIndex[objectIndex];
			if (current != nullptr &&
				(current->_markedForDeletion || current->_gameObject != obj))
				current = nullptr;

			// walkAlongPath(objectIndex) when orientation != 0x11; pickup at 0x11.
			if (current != nullptr) {
				if (obj->_orientation != 0x11)
					current->update();
				else if (executor->_pickupInProgress)
					current->update();
			}
		}
		flushPendingCharacterDeletes();
	}

	// --- Background animations (1008:929c, before LAB_1008_92d4) ---
	if (surface != nullptr && _currentMode != ViewMode::VM_HELP)
		drawBackgroundAnimations(*surface);

	// --- Pass 2 (LAB_1008_92d4): draw sorted scene objects back -> front ---
	if (surface != nullptr && !executor->hasScriptError() && sortedCount > 0) {
		const uint16 animAdvanceMode = (fullUpdate && _uiPanelState == kUiPanelNone) ? 2 : 0;

		for (uint16 local_c = 1; local_c <= sortedCount; local_c++) {
			const uint16 objectIndex = _sortedObjectIndices[local_c];
			GameObject *obj = GameObjects::getObjectByIndex(objectIndex);
			if (obj == nullptr || obj->_index != objectIndex)
				continue;
			Character *current = _characterByObjectIndex[objectIndex];
			if (current != nullptr &&
				(current->_markedForDeletion || current->_gameObject != obj))
				current = nullptr;

			if (obj->_hasBoundsAttachment) {
				GameObject *parent = GameObjects::getObjectByIndex(obj->_boundsAttachmentObjectID);
				if (parent != nullptr) {
					obj->_position.x = parent->_position.x + (int16)obj->_boundsAttachmentValue1;
					obj->_position.y = parent->_position.y + (int16)obj->_boundsAttachmentValue2;
					obj->_verticalOffsetScale =
						parent->_verticalOffsetScale + (int16)obj->_boundsAttachmentValue3;
					if (current != nullptr) {
						current->_targetPosition = obj->_position;
						current->_pathFinalDestination = obj->_position;
						current->_motionTargetVerticalOffset = obj->_verticalOffsetScale;
					}
				}
			}

			const uint16 animSlot = resolveAnimSlotIndex(obj);

			if (!obj->isAnimSlotLoaded(animSlot)) {
				executor->setScriptError(10);
				return;
			}

			Common::Array<uint8> *blob = obj->getAnimSlotBlob(animSlot);
			if (blob == nullptr || blob->empty()) {
				executor->setScriptError(8);
				return;
			}

			AnimBlobView blobView(*blob);
			if (!blobView.isValid() || blobView.frameCount() == 0) {
				executor->setScriptError(blobView.frameCount() == 0 ? 0x0B : 8);
				return;
			}

			AnimFrame frame;
			if (current != nullptr) {
				if (!current->fillCurrentAnimationFrame(animAdvanceMode, frame)) {
					executor->setScriptError(8);
					return;
				}
			} else {
				uint16 frameStart = BackgroundAnimationBlob::advanceAnimFrame(*blob, true, animAdvanceMode);
				frame._offsetX = (int16)READ_LE_UINT16(&(*blob)[frameStart]);
				frame._offsetY = (int16)READ_LE_UINT16(&(*blob)[frameStart + 2]);
				const uint16 offset = frameStart + 6;
				frame._width = READ_LE_UINT16(&(*blob)[offset]);
				frame._height = READ_LE_UINT16(&(*blob)[offset + 2]);
				frame._data.resize(frame._width * frame._height);
				memcpy(frame._data.data(), &(*blob)[offset + 4], frame._width * frame._height);
			}

			const int16 charX = obj->_position.x;
			const int16 charY = obj->_position.y;

			// drawAllCharacters @ 1008:93f8-9440 (inlined; not a separate EXE function)
			int32 depthOffset = ((int32)charY - (int32)g_engine->_walkDepthThresholdY) *
								(int32)g_engine->_walkDepthScaleFactor / 100;
			const uint16 scalingFactor = (uint16)((int32)g_engine->_walkBaseSpeedPct + depthOffset);
			if (obj->_index == 1) {
				_scalingValues.characterY = (uint16)charY;
				_scalingValues.scalingFactor = scalingFactor;
			}

			int16 walkabilityOffset = 0;
			if (g_engine->_pathfindingMap.w > 0) {
				walkabilityOffset = g_engine->getWalkabilityAt(charY, charX);
				if (Macs2Engine::isWalkabilityBlocking((uint16)walkabilityOffset))
					walkabilityOffset = 0;
			}
			if (obj->_verticalOffsetScale != 0)
				walkabilityOffset = (scalingFactor * obj->_verticalOffsetScale) / 100;

			int shadingTableOffset = 0;
			if (g_engine->_shadowMap.w > 0) {
				const int sx = CLIP<int>(charX, 0, kScreenWidthLast);
				const int sy = CLIP<int>(charY, 0, kGameHeightLast);
				shadingTableOffset = MIN<int>(g_engine->_shadowMap.getPixel(sx, sy), 0x20);
			}

			uint16 frameWidth;
			uint16 frameHeight;
			if (obj->_hasScaling) {
				frameWidth = (frame._width * scalingFactor) / 100;
				frameHeight = (frame._height * scalingFactor) / 100;
			} else {
				frameWidth = frame._width;
				frameHeight = frame._height;
			}

			const int16 drawX = charX - (frameWidth >> 1) + frame._offsetX;
			const int16 drawY = (charY - frameHeight) - walkabilityOffset + frame._offsetY;
			const uint8 depthThreshold = (uint8)charY;
			const byte *pixelData = frame._data.data();

			// drawAllCharacters @ 1008:9573-9754: drawAnimFrame / drawAnimFrameShaded / drawAnimFrameDepth
			const bool clipGameArea = hasScummVerbUI();
			if (obj->_hasScaling) {
				drawSpriteTransparent(shadingTableOffset, depthThreshold, scalingFactor,
									  drawX, drawY, frame._width, frame._height, pixelData, *surface);
			} else if (obj->_hasShading) {
				drawSpriteScaled(shadingTableOffset, depthThreshold, drawX, drawY,
								 frame._width, frame._height, pixelData, *surface);
			} else {
				drawSprite(drawX, drawY, frame._width, frame._height,
						   const_cast<byte *>(pixelData), *surface, false, false, 0, clipGameArea);
			}

			// drawAllCharacters @ 1008:9759: wLastDrawX/Y exclude per-frame offsetX/offsetY
			// (offsets applied inside drawAnimFrameDepth @ 1010:1753-1759 only)
			obj->_lastDrawX = charX - (frameWidth >> 1);
			obj->_lastDrawY = (charY - frameHeight) - walkabilityOffset;
			obj->_lastDrawWidth = frameWidth;
			obj->_lastDrawHeight = frameHeight;

			int16 newLeft = obj->_lastDrawX - 1;
			int16 newTop = obj->_lastDrawY - 1;
			int16 newRight = obj->_lastDrawX + 2 * (frameWidth >> 1) + 1;
			int16 newBottom = obj->_lastDrawY + frameHeight + 1;

			if (newLeft < obj->_dirtyLeft)
				obj->_dirtyLeft = newLeft;
			if (newTop < obj->_dirtyTop)
				obj->_dirtyTop = newTop;
			if (obj->_dirtyRight < newRight)
				obj->_dirtyRight = newRight;
			if (obj->_dirtyBottom < newBottom)
				obj->_dirtyBottom = newBottom;

			if (obj->_dirtyTop < 0)
				obj->_dirtyLeft = 0;
			if (obj->_dirtyBottom < 0)
				obj->_dirtyBottom = 0;

			if (current != nullptr && DebugMan.isDebugChannelEnabled(kDebugGraphics)) {
				Common::String number = Common::String::format("%u", obj->_orientation);
				renderString(current->getPosition(), number.c_str());
				Common::Rect screenRect(0, 0, kScreenWidth, kGameHeight);
				if (screenRect.contains(current->getPosition()))
					surface->setPixel(current->getPosition().x, current->getPosition().y, 0xFF);
			}
		}
	}

	// Binary drawAllCharacters tail: movement-finished repeat run (opcode 0x27 area checks).
	if (fullUpdate && g_engine->_movementFinishedFlag) {
		if (executor->isScriptWaitDeferred()) {
			debugC(kDebugScript,
				   "repeatRun deferred: walkWait=%u frameWait=%u soundWait=%d musicWait=%d adlibWait=%d",
				   executor->_walkTargetObjectIndex, executor->getFrameWaitCounter(),
				   executor->_waitForPcmSound ? 1 : 0, executor->_waitForMusicControl ? 1 : 0,
				   executor->_waitForAdlibReady ? 1 : 0);
		} else {
			const Common::Point actorPos = executor->getCharPosition();
			const uint16 area = executor->getAreaAtPoint(actorPos.x, actorPos.y);
			debugC(kDebugScript, "repeatRun start: actor=(%d,%d) areaRepeatRun=%u var[122]=%u",
				   actorPos.x, actorPos.y, area, executor->getVariableValue(122));
			executor->debugLogActorWalkState("repeatRun start");
			executor->_isRepeatRun = true;
			g_engine->runScriptExecutor();
			executor->_isRepeatRun = false;
			executor->debugLogActorWalkState("repeatRun end");
		}
	}
}

void View1::drawInventory(Graphics::ManagedSurface &s) {
	// First, draw the whole background
	// Happens around l0037_47A1:

	// Original uses two separate icon index tables:
	// DS:0x0E for protagonist inventory, DS:0x1A for container inventory
	const Common::Array<uint16> &iconIndices = isInventorySourceProtagonist()
												   ? g_engine->inventoryIconIndices
												   : g_engine->containerInventoryIconIndices;

	uint16 maxWidthButtonIcon = 0;  // [0FE0h]
	uint16 maxHeightButtonIcon = 0; // [0FE2h]
	for (uint16 index : iconIndices) {
		AnimFrame &currentFrame = g_engine->_imageResources[index - 1];
		maxWidthButtonIcon = MAX(maxWidthButtonIcon, currentFrame._width);
		// TODO: Not sure if this one is needed
		maxHeightButtonIcon = MAX(maxHeightButtonIcon, currentFrame._height);
	}

	uint16 maxWidthInventoryIcon = 0x20;  // [0FDCh]
	uint16 maxHeightInventoryIcon = 0x20; // [0FDEh]

	for (GameObject *currentInventoryObject : _inventoryItems) {
		AnimFrame *icon = getInventoryIcon(currentInventoryObject);
		if (icon == nullptr) {
			continue;
		}
		if (icon->_width > 0 && icon->_width < 250) {
			maxWidthInventoryIcon = MAX(maxWidthInventoryIcon, icon->_width);
		}
		if (icon->_height > 0 && icon->_height < 250) {
			maxHeightInventoryIcon = MAX(maxHeightInventoryIcon, icon->_height);
		}
		delete icon;
	}

	// Original adds +6 to button dimensions before using them (g_wActionBarButtonWidth += 6)
	uint16 buttonW = maxWidthButtonIcon + 6;
	uint16 buttonH = maxHeightButtonIcon + 6;

	// TODO: Verify these in emulator
	uint16 widthCandidate1 = (buttonW + 4) * 6 + 4;
	uint16 widthCandidate2 = (maxWidthInventoryIcon + 6 + 4) * 5 + 0xC;
	uint16 width = MAX(widthCandidate1, widthCandidate2); // [0FD8h]

	// Height calculation
	uint16 height = (maxHeightInventoryIcon + 6 + 4) * 2 + buttonH + 0x10; // [0FDAh]

	// Position calculation - TODO: Proper position
	uint16 x = s.w / 2 - width / 2;  // [0FD4h]
	uint16 y = s.h / 2 - height / 2; // [0FD6h]

	// Original: g_wInventorySlotWidth += 6 happens here
	uint16 slotW = maxWidthInventoryIcon + 6;
	uint16 slotH = maxHeightInventoryIcon + 6;

	Graphics::ManagedSurface *buffer = new Graphics::ManagedSurface(s.w, s.h, s.format);
	buffer->rawBlitFrom(s, Common::Rect(0, 0, s.w, s.h), Common::Point(0, 0));

	drawBorderSide(Common::Point(x, y), Common::Point(width, height), s);
	drawNinePatchBorder(Common::Point(x, y), Common::Point(width, height), kBorderRaised, false, false, s);

	uint16 buttonX = (s.w / 2) - (buttonW + 4) * 3 + 2;
	uint16 buttonY = y + height - 4 - buttonH;

	// Draw the buttons at the bottom
	for (int i = 0; i < 6; i++) {
		if (hasScummVerbUI() && !isInventorySourceProtagonist() && i == (int)InventoryButtonIndex::Drop)
			continue;

		uint16 index = iconIndices[i];
		AnimFrame &currentFrame = g_engine->_imageResources[index - 1];
		drawNinePatchBorder(Common::Point(buttonX, buttonY), Common::Point(buttonW, buttonH), kBorderRaised, false, false, s);
		uint16 iconX = (buttonW / 2 + buttonX) - currentFrame._width / 2;
		uint16 iconY = (buttonH / 2 + buttonY) - currentFrame._height / 2;
		_inventoryButtonLocations[i] = Common::Rect(Common::Point(buttonX, buttonY), buttonW, buttonH);
		drawSprite(iconX, iconY, currentFrame, s, false);
		buttonX += buttonW + 4;
	}
	Common::Rect sourceRect(Common::Point((s.w / 2) - ((slotW + 4) * 5 + 4) / 2 + 1, y + 5),
							(slotW + 4) * 5 + 2, (slotH + 4) * 2 + 2);
	// Restore the scene background in the items area, darkened through the shading table
	for (int dy = sourceRect.top; dy < sourceRect.bottom; dy++) {
		for (int dx = sourceRect.left; dx < sourceRect.right; dx++) {
			uint8 pixel = g_engine->_panelRemapTable[((const byte *)buffer->getBasePtr(dx, dy))[0]];
			s.setPixel(dx, dy, pixel);
		}
	}

	drawNinePatchBorder(Common::Point(
							(s.w / 2) - ((slotW + 4) * 5 + 4) / 2,
							y + 4),
						Common::Point(
							(slotW + 4) * 5 + 4,
							(slotH + 4) * 2 + 4),
						kBorderPressed, false, false, s);

	// Original: slotWidth = maxWidth + 6, slotHeight = maxHeight + 6 (already added above for hit testing)
	// Original X start: centered on screen based on slot grid, NOT panel-relative
	// local_e = ((screenWidth/2) - ((slotWidth+4)*5 + 4)/2) + 4
	// local_10 = panelY + 8
	uint16 itemX = (s.w / 2) - ((slotW + 4) * 5 + 4) / 2 + 4;
	uint16 itemY = y + 8;
	_inventoryGridUpperLeft.x = itemX;
	_inventoryGridUpperLeft.y = itemY;
	// Original adds +6 to slot dimensions for hit testing (g_wInventorySlotWidth += 6)
	_inventorySlotSize.x = slotW;
	_inventorySlotSize.y = slotH;
	// Original: local_12 counts from 10 down, showing up to 10 items (5 per row, 2 rows)
	uint16 itemIndex = _inventoryScrollOffset;
	uint16 itemXStart = itemX;
	// Now the inventory icons themselves
	for (int iy = 0; iy < 2; iy++) {
		for (int ix = 0; ix < 5; ix++) {
			if (itemIndex >= _inventoryItems.size()) {
				break;
			}
			AnimFrame *icon = getInventoryIcon(_inventoryItems[itemIndex]);
			if (icon != nullptr) {
				// Original: (slotWidth/2 + local_e) - (frameWidth/2)
				drawSprite(slotW / 2 + itemX - icon->_width / 2,
						   slotH / 2 + itemY - icon->_height / 2,
						   *icon, s, false);
				delete icon;
			}
			itemIndex++;
			itemX += slotW + 4;
		}
		itemX = itemXStart;
		itemY += slotH + 4;
	}
	delete buffer;
}

GameObject *View1::getClickedInventoryItem(const Common::Point &p) {
	Common::Rect currentInventorySlot(_inventoryGridUpperLeft, _inventoryGridUpperLeft + _inventorySlotSize);

	uint16 itemIndex = _inventoryScrollOffset;
	for (int iy = 0; iy < 2; iy++) {
		for (int ix = 0; ix < 5; ix++) {
			if (itemIndex >= _inventoryItems.size()) {
				return nullptr;
			}
			if (currentInventorySlot.contains(p)) {
				return _inventoryItems[itemIndex];
			}
			itemIndex++;
			currentInventorySlot.moveTo(currentInventorySlot.left + _inventorySlotSize.x + 4, currentInventorySlot.top);
		}
		currentInventorySlot.moveTo(_inventoryGridUpperLeft.x, currentInventorySlot.top + _inventorySlotSize.y + 4);
	}
	return nullptr;
}

void View1::drawSprite(int16 x, int16 y, uint16 width, uint16 height, byte *data, Graphics::ManagedSurface &s, bool mirrored, bool useDepth, uint8 depth, bool clipToGameArea) {
	for (int currentX = 0; currentX < width; currentX++) {
		int actualX = mirrored ? width - currentX - 1 : currentX;
		for (int currentY = 0; currentY < height; currentY++) {
			uint8 val = data[currentY * width + currentX];
			if (val != 0) {
				int finalX = x + actualX;
				int finalY = y + currentY;
				if (finalX >= 0 && finalX < s.w && finalY >= 0 && finalY < s.h) {
					if (clipToGameArea && finalY >= kGameHeight)
						continue;
					// Check for depth
					uint8 bgDepth = g_engine->_depthMap.getPixel(finalX, finalY);
					// Depth test: draw pixel only if depth map value < character depth
					// (verified: drawSpriteTransparent at 1010:0ed1 uses *depthMap < param_4)
					if (!useDepth || bgDepth < depth) {
						s.setPixel(x + actualX, y + currentY, val);
					}
				}
			}
		}
	}
}

void View1::drawSprite(const Common::Point &pos, uint16 width, uint16 height, byte *data, Graphics::ManagedSurface &s, bool mirrored, bool useDepth, uint8 depth, bool clipToGameArea) {
	drawSprite(pos.x, pos.y, width, height, data, s, mirrored, useDepth, depth, clipToGameArea);
}

void View1::drawSprite(int16 x, int16 y, const Sprite &sprite, Graphics::ManagedSurface &s, bool mirrored, bool useDepth, uint8 depth, bool clipToGameArea) {
	drawSprite(x, y, sprite._width, sprite._height, const_cast<byte *>(sprite._data.data()), s, mirrored, useDepth, depth, clipToGameArea);
}

void View1::drawSpriteClipped(uint16 x, uint16 y, Common::Rect &clippingRect, uint16 width, uint16 height, const byte *const data, Graphics::ManagedSurface &s) {
	for (int currentX = 0; currentX < width; currentX++) {
		for (int currentY = 0; currentY < height; currentY++) {
			uint8 val = data[currentY * width + currentX];
			if (val != 0) {
				const int px = x + currentX;
				const int py = y + currentY;
				if (clippingRect.contains(px, py) && px >= 0 && px < s.w && py >= 0 && py < s.h)
					s.setPixel(px, py, val);
			}
		}
	}
}

void View1::drawSpriteClipped(uint16 x, uint16 y, Common::Rect &clippingRect, const Sprite &sprite, Graphics::ManagedSurface &s) {
	drawSpriteClipped(x, y, clippingRect, sprite._width, sprite._height, sprite._data.data(), s);
}

void View1::drawSpriteFitted(const Common::Rect &bounds, const Sprite &sprite, Graphics::ManagedSurface &s, uint16 inset) {
	if (sprite._width == 0 || sprite._height == 0 || sprite._data.empty())
		return;

	const Common::Rect inner(bounds.left + inset, bounds.top + inset,
							 bounds.right - inset, bounds.bottom - inset);
	if (inner.width() <= 0 || inner.height() <= 0)
		return;

	const int destW = inner.width();
	const int destH = inner.height();
	const int scaleW = (destW * 256) / sprite._width;
	const int scaleH = (destH * 256) / sprite._height;
	const int scale = MIN(scaleW, scaleH);
	if (scale <= 0)
		return;

	const int drawW = MAX(1, (sprite._width * scale) / 256);
	const int drawH = MAX(1, (sprite._height * scale) / 256);
	const int startX = inner.left + (destW - drawW) / 2;
	const int startY = inner.top + (destH - drawH) / 2;

	for (int dy = 0; dy < drawH; dy++) {
		const int srcY = (dy * sprite._height) / drawH;
		const int py = startY + dy;
		if (py < inner.top || py >= inner.bottom)
			continue;

		for (int dx = 0; dx < drawW; dx++) {
			const int srcX = (dx * sprite._width) / drawW;
			const uint8 val = sprite._data[srcY * sprite._width + srcX];
			if (val == 0)
				continue;

			const int px = startX + dx;
			if (px < inner.left || px >= inner.right || px < 0 || px >= s.w || py < 0 || py >= s.h)
				continue;

			s.setPixel(px, py, val);
		}
	}
}

static byte applyShadingTable(byte color, int shadingTableOffset) {
	if (shadingTableOffset == 0)
		return color;
	// drawSpriteTransparent @ 1010:0fba: (color - 0xC0) * 0x20 + shadingTableOffset + scene+0x53D3
	const uint idx = (uint)(color - 0xC0) * 0x20 + (uint)shadingTableOffset;
	return g_engine->_shadingTable[idx];
}

// drawSpriteScaled @ 1010:102b
void View1::drawSpriteScaled(int shadingTableOffset, uint8 depthThreshold, int16 drawX, int16 drawY,
							 uint16 srcWidth, uint16 srcHeight, const byte *srcPixels,
							 Graphics::ManagedSurface &s) {
	int screenY = drawY;
	int srcRow = 0;
	int remainingRows = srcHeight;
	while (remainingRows > 0) {
		if (screenY >= 0 && screenY < s.h && !(hasScummVerbUI() && screenY >= kGameHeight)) {
			int screenX = drawX;
			for (uint16 srcX = 0; srcX < srcWidth; srcX++) {
				if (screenX >= 0 && screenX < s.w) {
					const uint8 bgDepth = g_engine->_depthMap.getPixel(screenX, screenY);
					if (bgDepth < depthThreshold) {
						const uint8 color = srcPixels[srcRow + srcX];
						if (color != 0)
							s.setPixel(screenX, screenY, applyShadingTable(color, shadingTableOffset));
					}
				}
				screenX++;
			}
		}
		screenY++;
		srcRow += srcWidth;
		remainingRows--;
	}
}

// drawSpriteTransparent @ 1010:0ed1
void View1::drawSpriteTransparent(int shadingTableOffset, uint8 depthThreshold, uint16 scalingFactor,
								  int16 drawX, int16 drawY, uint16 srcWidth, uint16 srcHeight,
								  const byte *srcPixels, Graphics::ManagedSurface &s) {
	int screenY = drawY;
	int srcRowOffset = 0;
	int remainingRows = (int)srcHeight;
	uint16 yScaleAccum = 0;

	while (remainingRows > 0) {
		if (screenY >= 0 && screenY < s.h && !(hasScummVerbUI() && screenY >= kGameHeight)) {
			int screenX = drawX;
			const byte *srcPtr = srcPixels + srcRowOffset;
			int remainingSrcPixels = (int)srcWidth;
			uint16 xScaleAccum = 0;

			// Horizontal scaler @ 1010:0f65-0fe6
			while (remainingSrcPixels > 0) {
				const uint8 color = *srcPtr;
				if (color != 0 && screenX >= 0 && screenX < s.w &&
					g_engine->_depthMap.getPixel(screenX, screenY) < depthThreshold) {
					s.setPixel(screenX, screenY, applyShadingTable(color, shadingTableOffset));
				}

				screenX++;
				xScaleAccum += 100;

				if ((int16)xScaleAccum <= (int16)scalingFactor)
					continue;

				while ((int16)xScaleAccum > (int16)scalingFactor) {
					xScaleAccum -= scalingFactor;
					srcPtr++;
					remainingSrcPixels--;
					if (remainingSrcPixels == 0)
						break;

					if ((int16)xScaleAccum <= (int16)scalingFactor)
						break;
				}
			}
		}

		// Vertical scaler @ 1010:0fe7-0x101b
		screenY++;
		yScaleAccum += 100;

		if ((int16)yScaleAccum <= (int16)scalingFactor)
			continue;

		while ((int16)yScaleAccum > (int16)scalingFactor) {
			yScaleAccum -= scalingFactor;
			remainingRows--;
			srcRowOffset += srcWidth;
			if (remainingRows == 0)
				break;
		}
	}
}

void View1::showSpeechAct(uint16 characterIndex, const Common::Array<Common::String> &strings, const Common::Point &position, bool onRightSide) {
	// Binary scriptShowDialogue (1008:b4b6): renders dialogue with portrait,
	// then sets g_wIsShowingDialoguePanel=1 at the very end.
	_drawnStringBox = strings;
	_isShowingDialoguePanel = true;
	_uiPanelState = kUiPanelNone;
	_dialogueChoiceCount = 0;
	_continueScriptAfterUI = true;

	// TTS: speak the dialogue text
	Common::String ttsText;
	for (const Common::String &line : strings) {
		if (!ttsText.empty())
			ttsText += " ";
		ttsText += line;
	}
	g_engine->sayText(ttsText, Common::TextToSpeechManager::INTERRUPT);

	currentSpeechActData.speaker = getCharacterByIndex(characterIndex);
	currentSpeechActData.strings = strings;
	currentSpeechActData.position = position;
	currentSpeechActData.onRightSide = onRightSide;

	const int totalWidth = g_engine->measureStrings(strings) + 0x12;
	const int totalHeight = g_engine->measureStringsVertically(strings) + 0x10;
	int stringBoxX = position.x;
	int stringBoxY = position.y;
	Common::Point portraitBoxPosition = position;

	if (currentSpeechActData.speaker != nullptr) {
		AnimFrame *leftPortrait = currentSpeechActData.speaker->getCurrentPortrait(false);
		AnimFrame *rightPortrait = currentSpeechActData.speaker->getCurrentPortrait(true);
		const int portraitWidth = MAX<int>(leftPortrait ? leftPortrait->_width : 0, rightPortrait ? rightPortrait->_width : 0);
		if (portraitWidth > 0) {
			if (onRightSide) {
				stringBoxX = position.x - portraitWidth - 0x12 - totalWidth;
				portraitBoxPosition.x = stringBoxX + totalWidth + 4;
			} else {
				stringBoxX = position.x + portraitWidth + 0x12;
			}
		}
		delete leftPortrait;
		delete rightPortrait;
	}

	currentSpeechActData.position = portraitBoxPosition;
	// Activate mouth animation (handleTimerCallback 1008:d38b)
	currentSpeechActData.mouthAnimActive = (currentSpeechActData.speaker != nullptr);
	// Original: PTR_LOOP_1020_1004 = sum of all line lengths (total character count)
	int16 totalChars = 0;
	for (const Common::String &line : strings) {
		totalChars += line.size();
	}
	currentSpeechActData.mouthAnimCounter = (totalChars > 0) ? totalChars : 1;
	_stringBoxPosition = Common::Point(stringBoxX, stringBoxY);
	debugC(kDebugScript, "Layout speech act: speaker=%u rawPos=(%d,%d) rightSide=%u portraitBorderPos=(%d,%d) textBorderPos=(%d,%d) textBorderSize=(%d,%d) text=\"%s\"",
		   characterIndex, position.x, position.y, onRightSide ? 1 : 0,
		   currentSpeechActData.position.x, currentSpeechActData.position.y,
		   _stringBoxPosition.x, _stringBoxPosition.y, totalWidth, totalHeight, joinDebugStrings(strings).c_str());

	if (_autoclickActive) {
		handleTextBoxInput();
		g_engine->runScriptExecutor();
	}
}

void View1::drawNinePatchBorder(const Common::Point &pos, const Common::Point &size,
								const BorderStyle &style, bool fillCenter, bool fillSides,
								Graphics::ManagedSurface &s) {
	constexpr uint16 border = 6;

	if (fillCenter) {
		drawDarkRectangle(pos.x + 1, pos.y + 1, size.x - 1, size.y - 1);
	}

	if (fillSides) {
		drawBorderSide(pos, Common::Point(border, size.y), s);
		drawBorderSide(pos + Common::Point(size.x - border, 0), Common::Point(border, size.y), s);
		drawBorderSide(pos, Common::Point(size.x, border), s);
		drawBorderSide(pos + Common::Point(0, size.y - border), Common::Point(size.x, border), s);
	}

	// Outer edge
	drawHorizontalBorderHighlight(pos, size.x + 1, style.outerEdge, s);
	drawVerticalBorderHighlight(pos, size.y + 1, style.outerEdge, s);
	drawHorizontalBorderHighlight(pos + Common::Point(0, size.y), size.x + 1, style.outerEdge, s);
	drawVerticalBorderHighlight(pos + Common::Point(size.x, 0), size.y + 1, style.outerEdge, s);

	// Inner highlights (1px inset)
	drawHorizontalBorderHighlight(pos + Common::Point(1, 1), size.x - 1, style.topLeft, s);
	drawVerticalBorderHighlight(pos + Common::Point(1, 1), size.y - 1, style.topLeft, s);
	drawHorizontalBorderHighlight(pos + Common::Point(1, size.y - 1), size.x - 1, style.bottomRight, s);
	drawVerticalBorderHighlight(pos + Common::Point(size.x - 1, 1), size.y - 1, style.bottomRight, s);
}

void View1::drawBorder(const Common::Point &pos, const Common::Point &size, Graphics::ManagedSurface &s) {
	// fn0037_A65D proc
	constexpr uint16 border = 6;
	debugC(kDebugScript, "Render border: pos=(%d,%d) size=(%d,%d)", pos.x, pos.y, size.x, size.y);

	drawDarkRectangle(pos.x + 1, pos.y + 1, size.x - 1, size.y - 1);

	// Four textured border sides
	drawBorderSide(pos, Common::Point(border, size.y), s);
	drawBorderSide(pos + Common::Point(size.x - border, 0), Common::Point(border, size.y), s);
	drawBorderSide(pos, Common::Point(size.x, border), s);
	drawBorderSide(pos + Common::Point(0, size.y - border), Common::Point(size.x, border), s);

	// Outer bevel (raised: highlight top-left, shadow bottom-right)
	drawHorizontalBorderHighlight(pos + Common::Point(1, 1), size.x - 1, 0x1012, s);
	drawVerticalBorderHighlight(pos + Common::Point(1, 1), size.y - 1, 0x1012, s);
	drawHorizontalBorderHighlight(pos + Common::Point(1, size.y - 1), size.x - 1, 0x1011, s);
	drawVerticalBorderHighlight(pos + Common::Point(size.x - 1, 1), size.y - 1, 0x1011, s);

	// Inner bevel at border/content boundary (inverted: shadow inside top-left, highlight inside bottom-right)
	drawHorizontalBorderHighlight(pos + Common::Point(border, border), size.x - 0xB, 0x1011, s);
	drawVerticalBorderHighlight(pos + Common::Point(border, border), size.y - 0xB, 0x1011, s);
	drawHorizontalBorderHighlight(pos + Common::Point(border, size.y - border), size.x - 0xB, 0x1012, s);
	drawVerticalBorderHighlight(pos + Common::Point(size.x - border, border), size.y - 0xB, 0x1012, s);
}

// drawBorderSide (1008:39b5)
void View1::drawBorderSide(const Common::Point &pos, const Common::Point &size, Graphics::ManagedSurface &s) {
	// Clipping region: (x+1, y+1) to (x+width, y+height) per disassembly
	Common::Rect clippingRect(pos + Common::Point(1, 1), pos + size);
	// Texture: border sprite from cursor image array at offset 0x1f0 (mode 1)
	uint16 currentX = clippingRect.left;
	uint16 currentY = clippingRect.top;
	const AnimFrame &sprite = g_engine->_imageResources[31];

	while (currentY < clippingRect.bottom) {
		while (currentX < clippingRect.right) {
			drawSpriteClipped(currentX, currentY, clippingRect, sprite._width, sprite._height, sprite._data.data(), s);
			currentX += sprite._width;
		}
		currentX = clippingRect.left;
		currentY += sprite._height;
	}
}

Macs2::AnimFrame *View1::getUISprite(uint32 offset) {
	if (offset == 0x1011) {
		return &g_engine->_imageResources[30];
	} else if (offset == 0x1012) {
		return &g_engine->_imageResources[32];
	} else if (offset == 0x1010) {
		return nullptr;
	}
	// We should not get here
	assert(false);
	return nullptr;
}

void View1::drawHorizontalBorderHighlight(const Common::Point &pos, int16 width, uint32 spriteAddress, Graphics::ManagedSurface &s) {
	// drawHorizontalBorderHighlight (1008:3737)
	// Sets clipping region to 1px tall horizontal strip, tiles the highlight/shadow sprite.
	Common::Rect clippingRect(pos, pos + Common::Point(width, 1));
	uint16 currentX = clippingRect.left;
	uint16 currentY = clippingRect.top;

	const AnimFrame *sprite = getUISprite(spriteAddress);
	if (sprite == nullptr) {
		return;
	}
	while (currentX < clippingRect.right) {
		drawSpriteClipped(currentX, currentY, clippingRect, sprite->_width, sprite->_height, sprite->_data.data(), s);
		currentX += sprite->_width;
	}
}

void View1::drawVerticalBorderHighlight(const Common::Point &pos, int16 height, uint32 spriteAddress, Graphics::ManagedSurface &s) {
	// drawVerticalBorderHighlight (1008:3876)
	// Sets clipping region to 1px wide vertical strip, tiles the highlight/shadow sprite.
	Common::Rect clippingRect(pos, pos + Common::Point(1, height));
	uint16 currentX = clippingRect.left;
	uint16 currentY = clippingRect.top;

	const AnimFrame *sprite = getUISprite(spriteAddress);
	if (sprite == nullptr) {
		return;
	}

	while (currentY < clippingRect.bottom) {
		drawSpriteClipped(currentX, currentY, clippingRect, sprite->_width, sprite->_height, sprite->_data.data(), s);
		currentY += sprite->_height;
	}
}

void View1::drawImageResources(Graphics::ManagedSurface &s) {
	uint16 x = 0;
	uint16 y = 0;
	uint16 currentMaxHeight = 0;
	for (AnimFrame &current : g_engine->_imageResources) {
		if (x + current._width > kScreenWidth) {
			y += currentMaxHeight;
			x = 0;
			currentMaxHeight = 0;
		}
		drawSprite(Common::Point(x, y), current._width, current._height, current._data.data(), s, false);
		x += current._width;
		currentMaxHeight = MAX(current._height, currentMaxHeight);
	}
}

void View1::showDialogueChoice(uint16 speakerObjectID, const Common::Array<Common::StringArray> &choices, const Common::Point &position, bool onRightSide) {
	Common::StringArray joinedLines;
	for (auto &currentLines : choices) {
		for (auto &currentLine : currentLines) {
			joinedLines.push_back(currentLine);
		}
	}

	// TTS: speak the dialogue choices
	Common::String ttsText;
	for (uint i = 0; i < choices.size(); i++) {
		if (!ttsText.empty())
			ttsText += ". ";
		ttsText += Common::String::format("%u: ", i + 1);
		for (const Common::String &line : choices[i]) {
			ttsText += line + " ";
		}
	}
	g_engine->sayText(ttsText, Common::TextToSpeechManager::INTERRUPT);

	showSpeechAct(speakerObjectID, joinedLines, position, onRightSide);
	_isDialogueChoiceInputActive = true;
	_dialogueChoiceCount = choices.size();
	_dialogueChoiceLineCounts.clear();
	for (const auto &c : choices) {
		_dialogueChoiceLineCounts.push_back(c.size());
	}
}

void View1::triggerDialogueChoice(uint8 index) {
	if (index < 1 || index > _dialogueChoiceCount) {
		warning("Ignoring dialogue choice %u without an active matching choice UI", index);
		return;
	}

	// Binary handleTimerClick (1008:d53b): stores the script-provided index value
	// from the choice entry (scene+0x5351+choice*6), NOT the 1-based array position.
	// It does NOT resume the script - that happens in handleInput after setting click state.
	uint16 scriptIndex = index;
	if ((uint)(index - 1) < g_engine->_scriptExecutor->_dialogueChoiceScriptIndices.size()) {
		scriptIndex = g_engine->_scriptExecutor->_dialogueChoiceScriptIndices[index - 1];
	}
	g_engine->_scriptExecutor->_chosenDialogueOption = scriptIndex;
	debug("triggerDialogueChoice: index=%u scriptIndex=%u executing=%d", index, scriptIndex, g_engine->_scriptExecutor->isExecuting() ? 1 : 0);
}

uint16 View1::getHitObjectID(const Common::Point &pos) const {
	// drawCharactersAndHitTest (1008:8d65): sortObjectListByY, objectListIndex = count..1.
	sortObjectListByY();
	const_cast<View1 *>(this)->rebuildCharacterLookupTable();

	for (uint16 local_c = _sortedObjectCount; local_c >= 1; local_c--) {
		const uint16 objectIndex = _sortedObjectIndices[local_c];
		Character *currentCharacter = _characterByObjectIndex[objectIndex];
		if (currentCharacter == nullptr || currentCharacter->_markedForDeletion)
			continue;

		AnimFrame frame;
		if (!currentCharacter->fillCurrentAnimationFrame(0, frame))
			continue;

		const Common::Point localPoint = pos - (currentCharacter->getPosition() - frame.getBottomMiddleOffset());
		if (localPoint.x < 0 || localPoint.x >= frame._width ||
			localPoint.y < 0 || localPoint.y >= frame._height)
			continue;
		if (frame._data[localPoint.y * frame._width + localPoint.x] == 0)
			continue;

		const uint8 characterDepth = currentCharacter->getPosition().y;
		if (pos.x >= 0 && pos.x < kScreenWidth && pos.y >= 0 && pos.y < kGameHeight) {
			const uint8 bgDepth = g_engine->_depthMap.getPixel(pos.x, pos.y);
			if (bgDepth >= characterDepth)
				continue;
		}

		return (uint16)(0x0400 + objectIndex);
	}
	return 0;
}

bool Character::HandleWalkability(Character *c) {
	// Wall-sliding obstacle avoidance from walkAlongPath (1008:1b8f).
	// When the character steps into a non-walkable pixel (walkability >= 200),
	// the original code samples walkability at +/-1 and +/-2 pixels in each
	// axis to build a gradient vector, then slides the character along that
	// vector until it reaches a walkable position.
	if (c->_gameObject->_index != 1) {
		return false;
	}
	if (g_engine->_scriptExecutor->isExecuting()) {
		return false;
	}

	Common::Point pos = c->getPosition();
	if (isWalkable(pos)) {
		return false;
	}

	// Build a push vector by sampling the walkability map around the current
	// position. Non-walkable neighbors push us away from them.
	int pushX = 0;
	int pushY = 0;

	// Sample at distance 1
	if (!isWalkable(Common::Point(pos.x + 1, pos.y)))
		pushX -= 1;
	if (!isWalkable(Common::Point(pos.x - 1, pos.y)))
		pushX += 1;
	if (!isWalkable(Common::Point(pos.x, pos.y + 1)))
		pushY -= 1;
	if (!isWalkable(Common::Point(pos.x, pos.y - 1)))
		pushY += 1;

	// Sample at distance 2 for stronger gradient
	if (!isWalkable(Common::Point(pos.x + 2, pos.y)))
		pushX -= 1;
	if (!isWalkable(Common::Point(pos.x - 2, pos.y)))
		pushX += 1;
	if (!isWalkable(Common::Point(pos.x, pos.y + 2)))
		pushY -= 1;
	if (!isWalkable(Common::Point(pos.x, pos.y - 2)))
		pushY += 1;

	// Slide along the push vector
	while (pushX != 0 || pushY != 0) {
		if (pushX < 0) {
			if (isWalkable(Common::Point(pos.x - 1, pos.y)))
				pos.x -= 1;
			pushX += 1;
		}
		if (pushX > 0) {
			if (isWalkable(Common::Point(pos.x + 1, pos.y)))
				pos.x += 1;
			pushX -= 1;
		}
		if (pushY < 0) {
			if (isWalkable(Common::Point(pos.x, pos.y - 1)))
				pos.y -= 1;
			pushY += 1;
		}
		if (pushY > 0) {
			if (isWalkable(Common::Point(pos.x, pos.y + 1)))
				pos.y += 1;
			pushY -= 1;
		}
	}

	c->setPosition(pos);
	return true;
}

uint16 Character::lookupWalkability(const Common::Point &p) const {
	return g_engine->getWalkabilityAt((int16)p.y, (int16)p.x);
}

bool Character::isWalkable(const Common::Point &p) const {
	return Macs2Engine::isWalkabilityWalkable(lookupWalkability(p));
}

Character::Character() : _startTime(0), _duration(0) {
	_pathfindingOverlay = Common::Array<uint8>(kScreenWidth * kGameHeight, 0);
}

bool Character::calculatePath(Common::Point target) {
	// Binary calculatePath (1008:1966). Params: charY, charX, finalDestY, finalDestX, actorIndex.
	// The binary operates on the runtime struct directly; we store equivalent state in _path etc.
	constexpr int MAX_NODES = 16;
	const Common::Point &charPos = _gameObject->_position;
	const int nodeCount = g_engine->getPathfindingNodeCount();

	// Step 1: Mark reachability anchored on FINAL DESTINATION (not character)
	// scene[i + 0x50C2] = isPathWalkable(finalDest, node[i])
	bool reachable[MAX_NODES + 1] = {};
	for (int i = 1; i <= nodeCount; i++) {
		const Common::Point &nodePos = g_engine->pathfindingPoints[i - 1]._position;
		reachable[i] = g_engine->isPathWalkable(target.y, target.x, nodePos.y, nodePos.x);
	}

	// Step 2: Find best entry node (lowest combined distance to both source and dest)
	int bestCost = 0x7777;
	int bestNode = 0;
	for (int i = 1; i <= nodeCount; i++) {
		const Common::Point &nodePos = g_engine->pathfindingPoints[i - 1]._position;
		int costToDest = g_engine->euclideanDistance(nodePos, target);
		int costToChar = g_engine->euclideanDistance(nodePos, charPos);
		if (costToDest + costToChar < bestCost) {
			// Verify this node can connect source to target
			// Binary calls canNodeConnectSourceToTarget(destY, destX, charY, charX, i)
			// due to calculatePath being invoked with swapped source/dest params.
			// This means the gate check is "can node see CHARACTER" and the flood-fill
			// checks "any node reachable from DEST" AND "any node visible from CHARACTER".
			if (canNodeConnectSourceToTarget(i, target, charPos, reachable, nodeCount)) {
				// Recompute cost (binary does this twice)
				costToDest = g_engine->euclideanDistance(nodePos, target);
				costToChar = g_engine->euclideanDistance(nodePos, charPos);
				bestCost = costToDest + costToChar;
				bestNode = i;
			}
		}
	}

	if (bestNode == 0) {
		// No path found - go directly to target
		// Binary: pathNodeCount=0, pathIndex=1
		_path.clear();
		_currentPathIndex = 1;
		_targetPosition = target;
		return false;
	}

	// Step 3: smoothPath - build path from bestNode toward a reachable node
	_path.clear();
	_path.push_back(bestNode);
	int currentNode = bestNode;
	while (!reachable[currentNode]) {
		const PathfindingPoint &curPt = g_engine->pathfindingPoints[currentNode - 1];
		int localBestCost = 0x7777;
		int nextNode = currentNode;
		for (uint a = 0; a < curPt._adjacentPoints.size(); a++) {
			int adjIdx = curPt._adjacentPoints[a];
			int cost = g_engine->computeMinCostToReachable(adjIdx, 0x7fff, _gameObject->_index, reachable, nodeCount, target);
			int edgeCost = g_engine->walkableDistance(adjIdx, currentNode);
			if (cost + edgeCost < localBestCost) {
				nextNode = adjIdx;
				localBestCost = cost + edgeCost;
			}
		}
		currentNode = nextNode;
		_path.push_back(currentNode);
		if (_path.size() > MAX_NODES)
			break; // safety
	}

	// Step 4: Validate path - consecutive nodes must be walkable to each other
	for (uint i = 0; i + 1 < _path.size(); i++) {
		const Common::Point &p1 = g_engine->pathfindingPoints[_path[i + 1] - 1]._position;
		const Common::Point &p2 = g_engine->pathfindingPoints[_path[i] - 1]._position;
		if (!g_engine->isPathWalkable(p1.y, p1.x, p2.y, p2.x)) {
			// Path invalid - abort, go directly to target
			_path.clear();
			_targetPosition = target;
			return false;
		}
	}

	// Step 5: Skip-forward optimization - skip nodes the character can already reach directly.
	// Binary: checks isPathWalkable(nextNode, charPos) - "can character see the next node?"
	// Note: binary's calculatePath is called with swapped params, so its 'finalDest' param
	// is actually the character position.
	_currentPathIndex = 0;
	while (_currentPathIndex + 1 < (int16)_path.size()) {
		const Common::Point &nextNodePos = g_engine->pathfindingPoints[_path[_currentPathIndex + 1] - 1]._position;
		if (!g_engine->isPathWalkable(nextNodePos.y, nextNodePos.x, charPos.y, charPos.x))
			break;
		_currentPathIndex++;
	}

	// Set immediate target to the current path node
	const Common::Point &firstTarget = g_engine->pathfindingPoints[_path[_currentPathIndex] - 1]._position;
	_targetPosition = firstTarget;
	return true;
}

bool Character::canNodeConnectSourceToTarget(uint16 nodeIndex, const Common::Point &charPos, const Common::Point &target, const bool *reachable, int nodeCount) {
	// Binary findShortestPath (1008:14d4).
	// Checks if node can connect source (charPos) to target:
	// 1. Node must be able to see the target
	// 2. Flood-fill connected component from node
	// 3. Some node in component must see target AND some node must be seen from source
	const Common::Point &nodePos = g_engine->pathfindingPoints[nodeIndex - 1]._position;
	if (!g_engine->isPathWalkable(nodePos.y, nodePos.x, target.y, target.x))
		return false;

	// Flood-fill connected nodes
	bool visited[17] = {};
	floodFillConnectedNodes(nodeIndex, visited, nodeCount);

	// Check both conditions
	bool anySeesTarget = false;
	bool anySeenFromSource = false;
	for (int i = 1; i <= nodeCount; i++) {
		if (!visited[i])
			continue;
		const Common::Point &p = g_engine->pathfindingPoints[i - 1]._position;
		if (g_engine->isPathWalkable(p.y, p.x, target.y, target.x))
			anySeesTarget = true;
		if (g_engine->isPathWalkable(charPos.y, charPos.x, p.y, p.x))
			anySeenFromSource = true;
	}
	return anySeesTarget && anySeenFromSource;
}

void Character::floodFillConnectedNodes(int nodeIndex, bool *visited, int nodeCount) {
	if (nodeIndex < 1 || nodeIndex > nodeCount)
		return;
	if (visited[nodeIndex])
		return;
	visited[nodeIndex] = true;
	const PathfindingPoint &pt = g_engine->pathfindingPoints[nodeIndex - 1];
	for (uint i = 0; i < pt._adjacentPoints.size(); i++) {
		floodFillConnectedNodes(pt._adjacentPoints[i], visited, nodeCount);
	}
}

Common::Point Character::getPosition() const {
	return _gameObject->_position;
}

void Character::setPosition(const Common::Point &newPosition) {
	_gameObject->_position = newPosition;
}

uint16 Character::getVerticalOffset() const {
	uint16 result = g_engine->getWalkabilityAt(getPosition());
	if (Macs2Engine::isWalkabilityBlocking(result)) {
		result = 0;
	}

	if (_gameObject->_verticalOffsetScale != 0) {
		// drawAllCharacters @ 1008:9549: scalingFactor * verticalOffsetScale / 100
		const int16 charY = getPosition().y;
		int32 depthOffset = ((int32)charY - (int32)g_engine->_walkDepthThresholdY) *
							(int32)g_engine->_walkDepthScaleFactor / 100;
		const uint16 scalingFactor = (uint16)((int32)g_engine->_walkBaseSpeedPct + depthOffset);
		result = (scalingFactor * _gameObject->_verticalOffsetScale) / 100;
	}

	return result;
}

bool Character::walkAlongPath() {
	// Binary walkAlongPath (1008:1b8f) path node advancement:
	// When arrived at current waypoint, snap position to node coords, then advance.
	// Binary: if (pathNodeIndex != 0) posX/Y = nodeCoords[pathNodes[pathNodeIndex]]
	if (_currentPathIndex >= 0 && _currentPathIndex < (int16)_path.size()) {
		const uint16 snapIdx = _path[_currentPathIndex];
		const Common::Point &snapPos = g_engine->pathfindingPoints[snapIdx - 1]._position;
		_gameObject->_position = snapPos;
	}
	_currentPathIndex++;
	if (_currentPathIndex >= (int16)_path.size()) {
		// Past end of path - walk to final destination, then stop
		_targetPosition = _pathFinalDestination;
		_stepDeltaX = abs(_targetPosition.x - _gameObject->_position.x);
		_stepDeltaY = abs(_targetPosition.y - _gameObject->_position.y);
		_stepError = 0;
		_stepDirectionSet = false;
		return false; // No more path segments after this
	}
	const uint16 nodeIdx = _path[_currentPathIndex];
	const Common::Point &nodePos = g_engine->pathfindingPoints[nodeIdx - 1]._position;
	_targetPosition = nodePos;
	_stepDeltaX = abs(_targetPosition.x - _gameObject->_position.x);
	_stepDeltaY = abs(_targetPosition.y - _gameObject->_position.y);
	_stepError = 0;
	_stepDirectionSet = false;
	return true;
}

bool Character::isAnimationMirrored() const {
	return is_in_list<uint16, 6, 7, 8, 14, 15, 16>(_gameObject->_orientation);
}

uint8 Character::getMirroredAnimation(uint8 original) const {
	switch (original) {
	case 6:
		return 4;
	case 7:
		return 3;
	case 8:
		return 2;
	case 14:
		return 12;
	case 15:
		return 11;
	case 16:
		return 10;
	}
	return original;
}

bool Character::fillCurrentAnimationFrame(uint16 advanceMode, Macs2::AnimFrame &out) {
	const uint16 animSlot = resolveAnimSlotIndex(_gameObject);

	_shouldMirrorCurrentAnimation = false;

	Common::Array<uint8> *blobPtr = _gameObject->getAnimSlotBlob(animSlot);
	if (blobPtr == nullptr || blobPtr->empty())
		return false;

	Common::Array<uint8> &blob = *blobPtr;
	const uint16 frameStart = BackgroundAnimationBlob::advanceAnimFrame(blob, true, advanceMode);
	out._offsetX = (int16)READ_LE_UINT16(&blob[frameStart]);
	out._offsetY = (int16)READ_LE_UINT16(&blob[frameStart + 2]);
	const uint16 offset = frameStart + 6;
	out._width = READ_LE_UINT16(&blob[offset]);
	out._height = READ_LE_UINT16(&blob[offset + 2]);
	out._data.resize(out._width * out._height);
	memcpy(out._data.data(), &blob[offset + 4], out._width * out._height);
	return true;
}

Macs2::AnimFrame *Character::getCurrentAnimationFrame(uint16 advanceMode) {
	AnimFrame *result = new AnimFrame();
	if (!fillCurrentAnimationFrame(advanceMode, *result)) {
		delete result;
		return nullptr;
	}
	return result;
}

Macs2::AnimFrame *Character::getCurrentPortrait(bool onRightSide, uint16 frameIndex) {
	if (_gameObject->_blobs.size() <= 17) {
		return nullptr;
	}

	uint portraitBlobIndex = 17;
	if (onRightSide && _gameObject->_blobs.size() > 18 && !_gameObject->_blobs[18].empty()) {
		portraitBlobIndex = 18;
	} else if (_gameObject->_blobs[portraitBlobIndex].empty() && _gameObject->_blobs.size() > 18 && !_gameObject->_blobs[18].empty()) {
		portraitBlobIndex = 18;
	}

	if (_gameObject->_blobs[portraitBlobIndex].empty()) {
		return nullptr;
	}

	uint16 offset = BackgroundAnimationBlob::advanceAnimFrame(_gameObject->_blobs[portraitBlobIndex], true, frameIndex);
	// offset points to per-frame: offsetX(2), offsetY(2), unknown(2), width(2), height(2), pixels
	offset += 6; // skip to width/height/pixels
	Common::Array<uint8> &blob = _gameObject->_blobs[portraitBlobIndex];
	AnimFrame *result = new AnimFrame();
	result->_width = READ_LE_UINT16(&blob[offset]);
	result->_height = READ_LE_UINT16(&blob[offset + 2]);
	result->_data.resize(result->_width * result->_height);
	memcpy(result->_data.data(), &blob[offset + 4], result->_width * result->_height);
	return result;
}

// NOTE: The original game (walkAlongPath at 1008:1b8f) does NOT use time-based
// lerping. Instead it uses pixel-by-pixel Bresenham stepping each frame, with
// speed scaled by depth (perspective). The walk click flow is:
//   1. snapToWalkablePosition() adjusts target to nearest walkable pixel
//   2. isPathWalkable() checks if direct line is clear
//   3. If not: calculatePath() does A* pathfinding through waypoints
//   4. walkAlongPath() steps 1 pixel per axis per frame, scaled by depth
// The current lerp-based approach is a simplification that should eventually
// be replaced with the original pixel-stepping for accurate movement speed.
void Character::startLerpTo(const Common::Point &target, uint32 duration, bool ignoreObstacles) {
	_startPosition = getPosition();
	_targetPosition = target;
	_startTime = g_events->currentMillis;
	_duration = duration;
	_lerpIgnoresObstacles = ignoreObstacles;

	// Reset Bresenham state - direction will be calculated on first Update()
	_stepDirectionSet = false;
	_stepDeltaX = abs(_targetPosition.x - _startPosition.x);
	_stepDeltaY = abs(_targetPosition.y - _startPosition.y);
	_stepError = 0;
}

void Character::startPickup(Macs2::GameObject *object) {
	_pickedUpObject = object;
	// Binary (1008:c475): walk target is the pickup object's effective position.
	_pathFinalDestination = getObjectEffectivePosition(object);
	_pickupFrameCounter = 0;
	_pickupItemTransferred = false;

	Common::Point current = getPosition();
	int16 destX = _pathFinalDestination.x;
	int16 destY = _pathFinalDestination.y;

	_currentPathIndex = 0;
	_path.clear();

	bool directPath = g_engine->isPathWalkable(destY, destX, current.y, current.x);
	if (!directPath && Macs2Engine::isWalkabilityWalkable(g_engine->getWalkabilityAt(destY, destX))) {
		calculatePath(Common::Point(destX, destY));
	}

	if (_path.empty()) {
		_targetPosition = _pathFinalDestination;
	}

	_stepDeltaX = abs(_targetPosition.x - current.x);
	_stepDeltaY = abs(_targetPosition.y - current.y);
	_stepError = 0;
	_stepDirectionSet = false;
}

bool Character::hasPendingVerticalMotion() const {
	return (int16)_motionTargetVerticalOffset >= 0 &&
		   _motionTargetVerticalOffset != _gameObject->_verticalOffsetScale;
}

bool Character::shouldStepVerticalMotion() const {
	return (int16)_motionTargetVerticalOffset < 0 ||
		   _motionTargetVerticalOffset != _gameObject->_verticalOffsetScale;
}

void Character::update() {
	// Binary drawAllCharacters (1008:90a2): calls walkAlongPath for every character
	// every frame, gated ONLY by: frozen flag and orientation != 0x11.

	// Binary: pickup animation handled separately (orientation == 0x11)
	if (_gameObject->_orientation == 0x11) {
		if (_pickedUpObject != nullptr) {
			View1 *currentView = (View1 *)g_engine->findView("View1");

			if (!_pickupItemTransferred && _pickupFrameCounter == _gameObject->_pickupFrameStart) {
				_pickupItemTransferred = true;
				currentView->transferPickupTarget(_pickedUpObject);
			}

			if (_pickupFrameCounter == _gameObject->_pickupFrameEnd) {
				_gameObject->_orientation = _previousOrientation;
				if (g_engine->_scriptExecutor->_pickupInProgress) {
					g_engine->_scriptExecutor->_pickupInProgress = false;
					g_engine->_scriptExecutor->_pickupActorObjectID = 0;
					g_engine->_scriptExecutor->_pickupTargetObjectID = 0;
					g_engine->setCursorMode(g_engine->_scriptExecutor->_cursorModeBeforeWait);
					currentView->updateCursor();
				}
				g_engine->_scriptExecutor->_walkTargetObjectIndex = 0;
				_pickedUpObject = nullptr;
				g_engine->_scriptExecutor->_interactedObjectID = 0x0000;
				g_engine->_scriptExecutor->_interactedInventoryItemId = 0x0000;
				g_engine->_movementFinishedFlag = true;
				return;
			}

			_pickupFrameCounter++;
		}
		return;
	}

	// Binary walkAlongPath (1008:1b8f): runs unconditionally every frame.
	// No _isLerping gate exists in the binary.
	Common::Point pos = getPosition();
	// Walk speed formula from walkAlongPath (1008:1b8f):
	//   depth = (posY - scene[0x51FD]) * scene[0x51FF] / 100
	//   walkSpeed = animSpeed * (scene[0x5201] + depth) / 100
	int32 depthOffset = ((int32)pos.y - (int32)g_engine->_walkDepthThresholdY) *
						(int32)g_engine->_walkDepthScaleFactor / 100;
	// Per-animation speed from blob data (runtime+orientation*16+0x30)
	// Walk speed from binary walkAlongPath (1008:1b8f):
	// pwVar7[orientation * 8 + 0x18] = word at runtime + orientation*16 + 0x30
	// = AnimSlot.wAnimSpeed (slot+0x0C). Stored in _blobWalkSpeeds.
	uint16 animSpeed = 2; // default fallback
	uint8 orient = _gameObject->_orientation;
	if (orient >= 1 && orient <= 0x15 && (uint)(orient - 1) < _gameObject->_blobWalkSpeeds.size()) {
		animSpeed = _gameObject->_blobWalkSpeeds[orient - 1];
		if (animSpeed == 0)
			animSpeed = 2;
	}
	int walkSpeed = ((int)animSpeed * ((int)g_engine->_walkBaseSpeedPct + (int)depthOffset)) / 100;
	if (walkSpeed < 1)
		walkSpeed = 1;

	// Proximity arrival check from walkAlongPath (1008:1b8f):
	// Binary checks if character is within walkSpeed pixels of target in both axes.
	bool arrived = (abs(pos.x - _targetPosition.x) <= walkSpeed) &&
				   (abs(pos.y - _targetPosition.y) <= walkSpeed);
	// Binary: arrival also requires vertical offset interpolation to be complete
	if (arrived && hasPendingVerticalMotion()) {
		arrived = false;
	}
	if (arrived) {
		// Binary (22cd): check if target == finalDest (at final destination)
		bool atFinalDest = (_targetPosition.x == _pathFinalDestination.x &&
							_targetPosition.y == _pathFinalDestination.y);

		if (!atFinalDest && !_path.empty()) {
			// Mid-path waypoint arrival: advance to next node
			// Binary (23b0): snap pos to current path node, advance pathIndex
			walkAlongPath();
			return;
		}

		// Final destination arrival (or direct walk arrival)
		if (_gameObject->_snapToTarget) {
			pos = _targetPosition;
			setPosition(pos);
			_pathFinalDestination = pos;
		} else {
			_targetPosition = pos;
			_pathFinalDestination = pos;
			if ((int16)_motionTargetVerticalOffset >= 0) {
				_motionTargetVerticalOffset = _gameObject->_verticalOffsetScale;
			}
		}
		_path.clear();
		if (hasPendingVerticalMotion()) {
			_gameObject->_verticalOffsetScale = _motionTargetVerticalOffset;
			_motionProgress = _motionDistanceUnits;
		}
		// Walk arrival: orientation changes to standing (walking dir + 8).
		// Script resumption is handled by position polling in View1::tick().
		bool wasWalking = (_gameObject->_orientation < 9);
		if (wasWalking) {
			_gameObject->_orientation += 8;
			// Binary walkAlongPath (1008:1b8f): sets g_bMovementFinishedFlag=1
			// when orientation < 9 at final arrival. This triggers the scene script
			// to check getAreaAtPoint (case 0x27) for scene transitions.
			g_engine->_movementFinishedFlag = true;
		}
		if (_pickedUpObject != nullptr) {
			// Binary: walk completion does NOT set orientation to 0x11 here.
			// gameTick checks position==finalDest each frame, and when matched
			// (with verticalOk), THEN it sets orientation to 0x11.
			// The _pickupFrameCounter and _previousOrientation are already set
			// in startPickup(). View1::tick() handles the orientation trigger.
			return;
		}
		return;
	}

	// Binary: if target==current position, skip Phase 0 turn delay (set directionCalculated=1)
	if (!_stepDirectionSet && _targetPosition.x == pos.x && _targetPosition.y == pos.y) {
		_stepDirectionSet = true;
	}

	// Calculate direction if not yet set (first frame of movement)
	if (!_stepDirectionSet) {
		_stepDirectionSet = true;
		// Phase 0 from walkAlongPath (1008:1b8f): direction calculation.
		// Binary returns after setting direction (1-frame turn delay).
		uint16 absDx = abs(pos.x - _targetPosition.x);
		uint16 absDy = abs(pos.y - _targetPosition.y);
		uint8 dir = _gameObject->_orientation;
		if (dir > 8 && dir < 17)
			dir -= 8;
		if (dir > 16)
			dir = 1;
		// Cardinal directions (only if animation available for that direction)
		if (_targetPosition.y < pos.y && absDx <= absDy &&
			_gameObject->_blobs.size() > 0 && !_gameObject->_blobs[0].empty())
			dir = 1; // North
		if (pos.x < _targetPosition.x && absDy <= absDx &&
			_gameObject->_blobs.size() > 2 && !_gameObject->_blobs[2].empty())
			dir = 3; // East
		if (pos.y < _targetPosition.y && absDx <= absDy &&
			_gameObject->_blobs.size() > 4 && !_gameObject->_blobs[4].empty())
			dir = 5; // South
		if (_targetPosition.x < pos.x && absDy <= absDx &&
			_gameObject->_blobs.size() > 6 && !_gameObject->_blobs[6].empty())
			dir = 7; // West
		// Diagonals: absDx/4 < absDy AND absDy/2 < absDx
		if ((absDx >> 2) < absDy && (absDy >> 1) < absDx) {
			if (_targetPosition.y < pos.y && pos.x < _targetPosition.x &&
				_gameObject->_blobs.size() > 1 && !_gameObject->_blobs[1].empty())
				dir = 2; // NE
			if (pos.x < _targetPosition.x && pos.y < _targetPosition.y &&
				_gameObject->_blobs.size() > 3 && !_gameObject->_blobs[3].empty())
				dir = 4; // SE
			if (pos.y < _targetPosition.y && _targetPosition.x < pos.x &&
				_gameObject->_blobs.size() > 5 && !_gameObject->_blobs[5].empty())
				dir = 6; // SW
			if (_targetPosition.x < pos.x && _targetPosition.y < pos.y &&
				_gameObject->_blobs.size() > 7 && !_gameObject->_blobs[7].empty())
				dir = 8; // NW
		}
		_gameObject->_orientation = dir;
		_stepDeltaX = absDx;
		_stepDeltaY = absDy;
		_stepError = 0;
		// 1-frame turn delay: return after setting direction (binary Phase 0)
		return;
	}

	// Phase 1: Bresenham stepping loop - exact 1:1 match of binary (1008:1ea1..2280)
	// Binary: stepCounter from 1 to walkSpeed, NO early break. Loop always completes.
	// After loop: if pixelsMoved != walkSpeed -> revert pos to savedPos and cancel path.
	int pixelsMoved = 0;
	Common::Point savedPos = pos;
	if (walkSpeed > 0) {
		for (int stepCounter = 1; stepCounter <= walkSpeed; stepCounter++) {
			savedPos = pos; // Binary: savedX/savedY at top of each iteration
			// Bresenham: if error >= deltaX -> step Y, else step X
			if (_stepError >= _stepDeltaX) {
				// Step Y axis
				if (_targetPosition.y != pos.y)
					pixelsMoved++;
				if (_targetPosition.y < pos.y)
					pos.y--;
				else if (_targetPosition.y > pos.y)
					pos.y++;
				_stepError -= _stepDeltaX;
			} else {
				// Step X axis
				if (_targetPosition.x != pos.x)
					pixelsMoved++;
				if (_targetPosition.x < pos.x)
					pos.x--;
				else if (_targetPosition.x > pos.x)
					pos.x++;
				_stepError += _stepDeltaY;
			}
			// Vertical offset interpolation
			if (shouldStepVerticalMotion()) {
				_motionProgress += _motionVerticalOffsetDelta;
				while (_motionProgress >= _motionDistanceUnits && _motionDistanceUnits > 0) {
					_motionProgress -= _motionDistanceUnits;
					if (_motionTargetVerticalOffset < _gameObject->_verticalOffsetScale)
						_gameObject->_verticalOffsetScale--;
					else if (_motionTargetVerticalOffset > _gameObject->_verticalOffsetScale)
						_gameObject->_verticalOffsetScale++;
				}
			}
			// Walkability check - binary uses getWalkabilityAt(posY, posX) >= 0xC8
			if (!isWalkable(pos)) {
				const uint16 tileArea = g_engine->_scriptExecutor->getAreaAtPoint(pos.x, pos.y);
				if (tileArea >= 210 && tileArea <= 215) {
					debugC(kDebugPath,
						   "walk blocked on plate area %u at (%d,%d) walk=%u int16=%d target=(%d,%d)",
						   tileArea, pos.x, pos.y, lookupWalkability(pos), (int16)lookupWalkability(pos),
						   _targetPosition.x, _targetPosition.y);
				}
				// Revert position
				pos = savedPos;
				// Wall-sliding: build push vector from ±1 and ±2 samples
				int pushX = 0, pushY = 0;
				if (Macs2Engine::isWalkabilityBlocking(lookupWalkability(Common::Point(pos.x + 1, pos.y))))
					pushX--;
				if (Macs2Engine::isWalkabilityBlocking(lookupWalkability(Common::Point(pos.x - 1, pos.y))))
					pushX++;
				if (Macs2Engine::isWalkabilityBlocking(lookupWalkability(Common::Point(pos.x, pos.y + 1))))
					pushY--;
				if (Macs2Engine::isWalkabilityBlocking(lookupWalkability(Common::Point(pos.x, pos.y - 1))))
					pushY++;
				if (Macs2Engine::isWalkabilityBlocking(lookupWalkability(Common::Point(pos.x + 2, pos.y))))
					pushX--;
				if (Macs2Engine::isWalkabilityBlocking(lookupWalkability(Common::Point(pos.x - 2, pos.y))))
					pushX++;
				if (Macs2Engine::isWalkabilityBlocking(lookupWalkability(Common::Point(pos.x, pos.y + 2))))
					pushY--;
				if (Macs2Engine::isWalkabilityBlocking(lookupWalkability(Common::Point(pos.x, pos.y - 2))))
					pushY++;
				// Apply push vector
				while (pushX != 0 || pushY != 0) {
					if (pushX < 0) {
						if (Macs2Engine::isWalkabilityWalkable(lookupWalkability(Common::Point(pos.x - 1, pos.y))))
							pos.x--;
						pushX++;
					}
					if (pushX > 0) {
						if (Macs2Engine::isWalkabilityWalkable(lookupWalkability(Common::Point(pos.x + 1, pos.y))))
							pos.x++;
						pushX--;
					}
					if (pushY < 0) {
						if (Macs2Engine::isWalkabilityWalkable(lookupWalkability(Common::Point(pos.x, pos.y - 1))))
							pos.y--;
						pushY++;
					}
					if (pushY > 0) {
						if (Macs2Engine::isWalkabilityWalkable(lookupWalkability(Common::Point(pos.x, pos.y + 1))))
							pos.y++;
						pushY--;
					}
				}
				// Binary: target = finalDest = pos (cancel path, but loop continues)
				_targetPosition = pos;
				_pathFinalDestination = pos;
				_path.clear();
			}
			// Binary: loop continues unconditionally until stepCounter == walkSpeed
		}
	}

	// Binary (2280): if pixelsMoved != walkSpeed -> revert and cancel
	if (pixelsMoved != walkSpeed) {
		const uint16 tileArea = g_engine->_scriptExecutor->getAreaAtPoint(pos.x, pos.y);
		if (tileArea >= 210 && tileArea <= 215) {
			debugC(kDebugPath,
				   "walk cancelled pixelsMoved=%d walkSpeed=%d at (%d,%d) area=%u walk=%u finalDest=(%d,%d)",
				   pixelsMoved, walkSpeed, pos.x, pos.y, tileArea, lookupWalkability(pos),
				   _pathFinalDestination.x, _pathFinalDestination.y);
		} else if (Macs2Engine::isWalkabilityBlocking(lookupWalkability(pos))) {
			debugC(kDebugPath,
				   "walk cancelled (non-walkable) pixelsMoved=%d walkSpeed=%d at (%d,%d) walk=%u",
				   pixelsMoved, walkSpeed, pos.x, pos.y, lookupWalkability(pos));
		}
		pos = savedPos;
		_targetPosition = pos;
		_pathFinalDestination = pos;
		_path.clear();
	}

	// Binary: walkSpeed==0 special case - still run vertical offset once
	if (walkSpeed == 0 && shouldStepVerticalMotion()) {
		_motionProgress += _motionVerticalOffsetDelta;
		while (_motionProgress >= _motionDistanceUnits && _motionDistanceUnits > 0) {
			_motionProgress -= _motionDistanceUnits;
			if (_motionTargetVerticalOffset < _gameObject->_verticalOffsetScale) {
				_gameObject->_verticalOffsetScale--;
			} else if (_motionTargetVerticalOffset > _gameObject->_verticalOffsetScale) {
				_gameObject->_verticalOffsetScale++;
			}
		}
	}

	setPosition(pos);
}

bool Button::isPointInside(const Common::Point &p) const {
	return false;
}

void Button::render(Graphics::ManagedSurface &s) {
}

void View1::openOriginalSaveLoadPanel() {
	// Exact translation of initSaveLoadPanel (1008:6184)
	_pendingPanelRequest = kPanelRequestNone; // Binary: g_wPendingPanelRequest = 0
	_uiPanelState = kUiPanelSaveLoad;         // Binary: g_wUiPanelState = 4
	_uiBackgroundRestorePending = true;       // Binary: g_wUiBackgroundRestorePending = 1

	g_engine->setCursorMode(Script::MouseMode::PanelCursor);

	// g_wActionBarButtonWidth = 0; g_wActionBarButtonHeight = 0
	uint16 maxW = 0;
	uint16 maxH = 0;

	// g_wSaveConfirmArmed = 0; g_wLoadConfirmArmed = 0
	_saveConfirmArmed = false;
	_loadConfirmArmed = false;

	// if (g_wMusicEnabled && sceneData[g_wActiveMusicSlot] != 0) adlibStopMusic()
	if (g_engine->_scriptExecutor->_musicEnabled &&
		g_engine->_scriptExecutor->_activeMusicSlot != 0) {
		g_engine->getAdlib()->stopMusic();
	}

	// First loop: calculate max icon width/height from the 7 button images
	for (int i = 1; i <= 7; i++) {
		int imgIdx = kLookupTable[i] - 1; // convert to 0-based
		if (imgIdx >= (int)g_engine->_imageResources.size())
			continue;
		AnimFrame &frame = g_engine->_imageResources[imgIdx];
		if (frame._data.empty() && frame._width == 0) {
			// Binary: if no data, sets width/height fields to 0
			continue;
		}
		// Binary: getFrameWidth/getFrameHeight then updates max
		if (frame._width > maxW)
			maxW = frame._width;
		if (frame._height > maxH)
			maxH = frame._height;
	}

	// g_wUiPanelWidth = (g_wActionBarButtonWidth + 10) * 7 + 4
	uint16 panelWidth = (maxW + 10) * 7 + 4;
	// if (g_wUiPanelWidth < 0xD4) g_wUiPanelWidth = 0xD4
	if (panelWidth < 0xD4)
		panelWidth = 0xD4;
	// g_wUiPanelHeight = g_wActionBarButtonHeight + 0x8A
	uint16 panelHeight = maxH + 0x8A;
	// g_wUiPanelX = (g_wScreenWidth >> 1) - (g_wUiPanelWidth >> 1)
	int panelX = 160 - (panelWidth >> 1);
	// g_wUiPanelY = (g_wScreenHeight >> 1) - (g_wUiPanelHeight >> 1)
	int panelY = 100 - (panelHeight >> 1);

	// g_wActionBarButtonWidth = g_wActionBarButtonWidth + 6
	_saveLoadButtonWidth = maxW + 6;
	// g_wActionBarButtonHeight = g_wActionBarButtonHeight + 6
	_saveLoadButtonHeight = maxH + 6;

	_saveLoadPanelRect = Common::Rect(panelX, panelY, panelX + panelWidth, panelY + panelHeight);

	// local_6 = ((g_wScreenWidth >> 1) - (g_wActionBarButtonWidth + 4) * 7 / 2) + 2
	int buttonRowX = (160 - (int)((_saveLoadButtonWidth + 4) * 7) / 2) + 2;
	// local_8 = (g_wUiPanelY + g_wUiPanelHeight - 4) - g_wActionBarButtonHeight
	int buttonRowY = (panelY + panelHeight - 4) - _saveLoadButtonHeight;

	// Second loop: store button positions and draw them
	for (int i = 1; i <= 7; i++) {
		// Store position into button rect (binary stores into cursor array entry x/y fields)
		_saveLoadButtonRects[i - 1] = Common::Rect(
			buttonRowX, buttonRowY,
			buttonRowX + _saveLoadButtonWidth, buttonRowY + _saveLoadButtonHeight);
		buttonRowX += _saveLoadButtonWidth + 4;
	}

	// Load save slot names (ScummVM equivalent of binary's file reading loop)
	// Convert UTF-8 descriptions to DOS CP850 since the glyph table uses DOS encoding
	for (int idx = 0; idx < 30; idx++) {
		SaveStateDescriptor desc = g_engine->getMetaEngine()->querySaveMetaInfos(
			g_engine->getGameId().c_str(), idx);
		if (desc.getSaveSlot() != -1) {
			Common::String utf8Name = desc.getDescription();
			Common::U32String u32Name = utf8Name.decode(Common::kUtf8);
			_saveSlotNames[idx] = Common::String(u32Name, Common::kDos850);
		} else {
			_saveSlotNames[idx] = "";
		}
	}

	_clickedButtonIndex = 0;
	_saveLoadSubMode = SaveLoadSubMode::None;
	_saveLoadPageIndex = 0;
	redraw();
}

void View1::closeOriginalSaveLoadPanel() {
	_uiPanelState = kUiPanelNone;
	_saveLoadSubMode = SaveLoadSubMode::None;
	g_engine->setCursorMode(_savedCursorMode);
	updateCursor();
	redraw();
}

void View1::drawOriginalSaveLoadPanel(Graphics::ManagedSurface &s) {
	if (_uiPanelState != kUiPanelSaveLoad)
		return;

	// Exact translation of drawSaveLoadPanel (1008:6592)
	const int panelX = _saveLoadPanelRect.left;
	const int panelY = _saveLoadPanelRect.top;
	const int panelW = _saveLoadPanelRect.width();
	const uint16 btnW = _saveLoadButtonWidth;
	const uint16 btnH = _saveLoadButtonHeight;

	// drawBorderSide + drawBorderOuterHighlights for panel
	drawBorderSide(Common::Point(panelX, panelY), Common::Point(panelW, _saveLoadPanelRect.height()), s);
	drawNinePatchBorder(Common::Point(panelX, panelY), Common::Point(panelW, _saveLoadPanelRect.height()), kBorderRaised, false, false, s);

	// Alternate music icon is at cursor array offset 0x1B0 = entry 27 (0-based)
	static const int kAltMusicIconIdx = 27; // 0-based into _imageResources

	uint16 subMode = (uint16)_saveLoadSubMode;

	for (int i = 1; i <= 7; i++) {
		int imgIdx = kLookupTable[i] - 1; // 0-based
		Common::Point btnPos(_saveLoadButtonRects[i - 1].left, _saveLoadButtonRects[i - 1].top);

		// Binary: if (local_4 < 0 || local_4 != g_wSaveLoadSubMode) -> normal border
		// else -> pressed border
		const bool pressed = ((uint16)i == subMode);
		drawNinePatchBorder(btnPos, Common::Point(btnW, btnH), pressed ? kBorderPressed : kBorderRaised, false, false, s);

		// Check if image has valid data (binary: check size fields > 0)
		if (imgIdx >= (int)g_engine->_imageResources.size()) {
			continue;
		}
		AnimFrame &frame = g_engine->_imageResources[imgIdx];
		if (frame._data.empty() || frame._width == 0) {
			continue;
		}

		// Determine which icon to draw
		AnimFrame *iconFrame = &frame;

		// Button 3 with sound off: use alternate icon at index 0x1B0/0x10 = 27
		if (i == 3 && !g_engine->_scriptExecutor->_soundSystemActive) {
			if (kAltMusicIconIdx < (int)g_engine->_imageResources.size()) {
				AnimFrame &altFrame = g_engine->_imageResources[kAltMusicIconIdx];
				if (!altFrame._data.empty() && altFrame._width > 0) {
					iconFrame = &altFrame;
				}
			}
		}

		// Binary icon centering:
		// y = (btnH/2 + buttonY) - (iconH/2)
		// x = (btnW/2 + buttonX) - (iconW/2)
		int iconX = ((btnW >> 1) + btnPos.x) - (iconFrame->_width >> 1);
		int iconY = ((btnH >> 1) + btnPos.y) - (iconFrame->_height >> 1);

		// Pressed: +1 offset
		if (pressed) {
			iconX++;
			iconY++;
		}

		drawSprite(iconX, iconY, *iconFrame, s, false);
	}

	// if (g_wMapPanelPageIndex == 1) drawSaveLoadScrollArrows()
	if (_saveLoadPageIndex == 1) {
		// TODO: drawSaveLoadScrollArrows - draws scaled animation frame over slot area
	}

	// Slot loop: local_4 = 0..9
	for (int slot = 0; slot <= 9; slot++) {
		// drawPanelSlot(0xc, g_wUiPanelWidth - 8, g_wUiPanelY + 4 + slot * 0xc, g_wUiPanelX + 4)
		int slotX = panelX + 4;
		int slotY = panelY + 4 + slot * 0xc;
		int slotW = panelW - 8;
		int slotH = 0xc;
		drawNinePatchBorder(Common::Point(slotX, slotY), Common::Point(slotW, slotH), kBorderPressed, false, false, s);

		// drawText at (g_wUiPanelX + 6, g_wUiPanelY + 6 + slot * 0xc)
		int idx = _saveLoadPageIndex * 10 + slot;
		Common::String label;
		if (idx < 30 && !_saveSlotNames[idx].empty()) {
			label = _saveSlotNames[idx];
		} else {
			label = "NONE";
		}
		const GlyphData *font = g_engine->numPanelGlyphs > 0 ? g_engine->_panelGlyphs : g_engine->_glyphs;
		uint16 fontCount = g_engine->numPanelGlyphs > 0 ? g_engine->numPanelGlyphs : g_engine->numGlyphs;
		label.toUppercase();
		renderStringWithFont(panelX + 6, panelY + 6 + slot * 0xc, label, font, fontCount);
	}
}

void View1::handleOriginalSaveLoadClick(const Common::Point &pos) {
	// Exact translation of handleSaveLoadPanelClick (1008:86a4)
	// Binary takes (clickY, clickX) - note reversed parameter order
	int clickX = pos.x;
	int clickY = pos.y;

	// if (g_wClickedButtonIndex == 0) { ... entire function body }
	if (_clickedButtonIndex != 0) {
		return;
	}

	const int panelX = _saveLoadPanelRect.left;
	const int panelY = _saveLoadPanelRect.top;
	const int panelW = _saveLoadPanelRect.width();
	const uint16 btnW = _saveLoadButtonWidth;
	const uint16 btnH = _saveLoadButtonHeight;

	bool bPageScroll = false;

	// Binary redraws panel inline during click handling
	// (We skip the redraw here since ScummVM handles it via draw() cycle)

	// if (g_wMapPanelPageIndex == 1) drawSaveLoadScrollArrows()
	if (_saveLoadPageIndex == 1) {
		// TODO: drawSaveLoadScrollArrows
	}

	// Slot loop: local_4 = 0..9
	for (int slot = 0; slot <= 9; slot++) {
		// Slot hit test for sub-mode 2 (save): editSaveSlotName
		if (_saveLoadSubMode == SaveLoadSubMode::Save &&
			(int)(panelX + 6) <= clickX &&
			clickX <= (int)(panelX + panelW - 0xc) &&
			(int)(panelY + 6 + slot * 0xc) <= clickY &&
			clickY <= (int)(panelY + slot * 0xc + 0x10)) {
			// editSaveSlotName(slot) - ScummVM: save to slot
			int idx = _saveLoadPageIndex * 10 + slot;
			Common::String name = Common::String::format("Save %d", idx + 1);
			g_engine->saveGameState(idx, name);
			_saveSlotNames[idx] = name;
			redraw();
			return;
		}

		// Slot hit test for sub-mode 1 (load): loadGameFromFile
		if (_saveLoadSubMode == SaveLoadSubMode::Load &&
			(int)(panelX + 6) <= clickX &&
			clickX <= (int)(panelX + panelW - 0xc) &&
			(int)(panelY + 6 + slot * 0xc) <= clickY &&
			clickY <= (int)(panelY + slot * 0xc + 0x10)) {
			int idx = _saveLoadPageIndex * 10 + slot;
			if (idx < 30 && !_saveSlotNames[idx].empty()) {
				// Binary: loadGameFromFile then:
				// g_wUiPanelState = 4; g_wClickedButtonIndex = 0;
				// g_wPendingPanelRequest = 4; g_wSaveLoadSubMode = 0
				g_engine->loadGameState(idx);
				_uiPanelState = kUiPanelSaveLoad;
				_clickedButtonIndex = 0;
				_pendingPanelRequest = kPanelRequestSaveLoadActive;
				_saveLoadSubMode = SaveLoadSubMode::None;
			}
			redraw();
			return;
		}
	}

	for (int i = 1; i <= 7; i++) {
		int imgIdx = kLookupTable[i] - 1; // 0-based
		Common::Point btnPos(_saveLoadButtonRects[i - 1].left, _saveLoadButtonRects[i - 1].top);

		// Hit test: clickX > btnPos.x && clickY > btnPos.y &&
		//           clickX < btnPos.x + btnW && clickY < btnPos.y + btnH
		// AND has valid image data
		// AND (mapDisabledFlag == 0 || i > 2)
		bool hasData = false;
		if (imgIdx < (int)g_engine->_imageResources.size()) {
			AnimFrame &frame = g_engine->_imageResources[imgIdx];
			hasData = (!frame._data.empty() && frame._width > 0);
		}

		bool isHit = (btnPos.x < clickX && btnPos.y < clickY &&
					  clickX < btnPos.x + btnW && clickY < btnPos.y + btnH &&
					  hasData &&
					  (!_helpButtonDisabled || i > 2));

		if (isHit) {
			// Button was clicked - draw pressed and process
			_clickedButtonIndex = i;

			if (i != 5)
				_saveConfirmArmed = false;
			if (i != 6)
				_loadConfirmArmed = false;

			// Process button action
			if (i == 3) {
				// Toggle music, reset clickedButton, redraw
				g_engine->_scriptExecutor->_soundSystemActive =
					!g_engine->_scriptExecutor->_soundSystemActive;
				_clickedButtonIndex = 0;
				redraw();
			} else if (i == 4) {
				bPageScroll = true;
			} else if (i == 5) {
				if (!_saveConfirmArmed) {
					_saveConfirmArmed = true;
				} else {
					// Binary: second click arms error 0x1C and closes via button 7
					g_engine->_scriptExecutor->setScriptError(0x1C);
					_clickedButtonIndex = 7;
				}
			} else if (i == 6) {
				if (!_loadConfirmArmed) {
					_loadConfirmArmed = true;
				} else {
					g_engine->_scriptExecutor->setScriptError(0x1B);
					_clickedButtonIndex = 7;
				}
			} else if (i == 7) {
				// Binary: if music enabled AND sound active, play active music
				if (g_engine->_scriptExecutor->_musicEnabled &&
					g_engine->_scriptExecutor->_soundSystemActive) {
					uint16 slot = g_engine->_scriptExecutor->_activeMusicSlot;
					if (slot != 0 && !g_engine->_scriptExecutor->_musicSlots[slot - 1].empty()) {
						g_engine->getAdlib()->playSongData(g_engine->_scriptExecutor->_musicSlots[slot - 1]);
						// Original's adlibTickHandler resets g_bAdlibMasterVolume=0 (full volume).
						// ScummVM layers user volume on top via scaledMusicVolume, so re-apply it.
						g_engine->_scriptExecutor->_musicControlMode = 0;
						g_engine->_scriptExecutor->_musicControlVolume = 0;
						g_engine->getAdlib()->setVolume(g_engine->scaledMusicVolume(0));
					}
				}
			}
		} else {
			// Button NOT hit - reset confirm flags for buttons 5/6
			if (i == 6)
				_loadConfirmArmed = false;
			if (i == 5)
				_saveConfirmArmed = false;
		}
	}

	// After button loop: set subMode based on clickedButtonIndex
	// Binary: if (g_wClickedButtonIndex < 3) g_wSaveLoadSubMode = g_wClickedButtonIndex
	//         else g_wSaveLoadSubMode = 0
	if (_clickedButtonIndex < 3) {
		_saveLoadSubMode = (SaveLoadSubMode)_clickedButtonIndex;
	} else {
		_saveLoadSubMode = SaveLoadSubMode::None;
	}

	// Binary: if (g_wClickedButtonIndex == 7) g_wPendingPanelRequest = 0 (close)
	//         else g_wPendingPanelRequest = 4 (stay open)
	if (_clickedButtonIndex == 7) {
		_pendingPanelRequest = kPanelRequestNone;
		return;
	} else {
		_pendingPanelRequest = kPanelRequestSaveLoadActive;
	}

	// Binary: if (bPageScroll && ++g_wMapPanelPageIndex == 3) g_wMapPanelPageIndex = 0
	if (bPageScroll) {
		_saveLoadPageIndex++;
		if (_saveLoadPageIndex == 3)
			_saveLoadPageIndex = 0;
	}

	// Reset for next click
	_clickedButtonIndex = 0;
	redraw();
}

} // namespace Macs2
