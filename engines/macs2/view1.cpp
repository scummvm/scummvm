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
#include "common/algorithm.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/debug.h"
#include "common/system.h"
#include "engines/enhancements.h"
#include "graphics/palette.h"
#include "graphics/paletteman.h"
#include "macs2/adlib.h"
#include "macs2/detection.h"
#include "macs2/gameobjects.h"
#include "macs2/macs2.h"
#include <graphics/cursorman.h>
#include <math/angle.h>
#include <math/vector2d.h>

namespace Macs2 {
namespace {
constexpr int kNumLoadedCursors = 33;

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

void buildFadedPalette(byte *colors, const byte *sourcePalette, int fadeValue) {
	// Original fadePaletteToBlack/FromBlack: subtracts fadeValue from raw 6-bit VGA
	// palette values (0-63), clamping to 0. Then scales to 8-bit for ScummVM.
	for (uint i = 0; i < 256 * 3; ++i) {
		int raw = sourcePalette[i]; // 6-bit value (0-63)
		int faded = raw - fadeValue;
		if (faded < 0)
			faded = 0;
		colors[i] = (faded * 259 + 33) >> 6; // 6-bit to 8-bit
	}
}

void setViewPaletteSafely(View1 *view, const byte *colors) {
	const bool shouldTouchCursor = view != nullptr && !view->isCursorSuppressedForFade();
	const bool cursorWasVisible = shouldTouchCursor && CursorMan.isVisible();
	if (cursorWasVisible)
		CursorMan.showMouse(false);

	if (shouldTouchCursor)
		view->updateCursor(colors);

	g_system->getPaletteManager()->setPalette(colors, 0, 256);

	if (cursorWasVisible)
		CursorMan.showMouse(true);
}

void applyPaletteWithFade(View1 *view, const byte *sourcePalette, int fadeValue) {
	byte colors[256 * 3];
	buildFadedPalette(colors, sourcePalette, fadeValue);
	setViewPaletteSafely(view, colors);
}
} // namespace

void View1::openInventory(GameObject *newInventorySource) {
	if (newInventorySource == nullptr) {
		warning("Tried to open inventory for a null source");
		return;
	}

	setInventorySource(newInventorySource);
	_isShowingInventory = true;
	_inventoryPage = 0;
	_activeInventoryItem = nullptr;
	g_engine->_scriptExecutor->_inventoryActionFlag = false;
	g_engine->_scriptExecutor->_inventoryCombineFlag = false;
	if (g_engine->_scriptExecutor->_mouseMode == Script::MouseMode::UseInventory) {
		g_engine->setCursorMode(Script::MouseMode::Use);
		updateCursor();
	}
}

void View1::closeInventory() {
	if (!_isShowingInventory) {
		return;
	}

	const bool shouldResumeExternalInventory = !isInventorySourceProtagonist() && g_engine->_scriptExecutor->_hasPendingExternalInventoryResume;
	_isShowingInventory = false;
	_inventoryPage = 0;
	_activeInventoryItem = nullptr;
	g_engine->_scriptExecutor->_inventoryActionFlag = false;
	g_engine->_scriptExecutor->_inventoryCombineFlag = false;

	if (shouldResumeExternalInventory) {
		g_engine->setCursorMode(g_engine->_scriptExecutor->_savedExternalInventoryMouseMode);
		updateCursor();
		setInventorySource(GameObjects::instance().getProtagonistObject());
		g_engine->_scriptExecutor->_hasPendingExternalInventoryResume = false;
		g_engine->_scriptExecutor->_externalInventorySourceObjectID = 0;
		// Restore script click state (original: handleInput restores from saved values)
		g_engine->_scriptExecutor->_scriptClickFlag = g_engine->_scriptExecutor->_savedScriptClickFlag;
		g_engine->_scriptExecutor->_scriptClickX = g_engine->_scriptExecutor->_savedScriptClickX;
		g_engine->_scriptExecutor->_scriptClickY = g_engine->_scriptExecutor->_savedScriptClickY;
		g_engine->_scriptExecutor->_scriptClickResult = g_engine->_scriptExecutor->_savedScriptClickResult;
		g_engine->_scriptExecutor->setCurrentSceneScriptAt(g_engine->_scriptExecutor->_secondaryInventoryLocation);
		g_engine->runScriptExecutor();
		return;
	}

	if (!isInventorySourceProtagonist()) {
		setInventorySource(GameObjects::instance().getProtagonistObject());
	}
}

void View1::setInventorySource(GameObject *newInventorySource) {
	_inventorySource = newInventorySource;
	// Rebuild inventory list from all objects whose SceneIndex matches.
	// Binary (readObjectFromFile at 1008:071e) checks: object.sceneIndex == actorIndex + 0x400.
	// The +0x400 offset encodes "inside this container/actor's inventory".
	_inventoryItems.clear();

	const uint16 inventorySceneId = _inventorySource->_index + 0x400;
	for (GameObject *currentObject : GameObjects::instance()._objects) {
		if (currentObject != nullptr && currentObject->_sceneIndex == inventorySceneId) {
			_inventoryItems.push_back(currentObject);
		}
	}
}

bool View1::isInventorySourceProtagonist() const {
	return _inventorySource->_index == 1;
}

void View1::transferInventoryItem(GameObject *item, GameObject *targetContainer) {
	int index = findInventoryItem(item);
	_inventoryItems.remove_at(index);
	item->_sceneIndex = targetContainer->_index + 0x400;
}

int View1::findInventoryItem(GameObject *item) {
	for (uint i = 0; i != _inventoryItems.size(); i++) {
		if (_inventoryItems[i] == item) {
			return i;
		}
	}
	return -1;
}

Character *View1::getCharacterByIndex(uint16 index) {
	// TODO: Consider a map
	for (Character *c : _characters) {
		if (c->_gameObject->_index == index) {
			return c;
		}
	}
	return nullptr;
}
void View1::updateCursor(const byte *palette) {
	CursorMan.showMouse(true);

	// Original indexes cursor array as: base + mode * 16 - 16, i.e. 0-based index = mode - 1.
	// The array has 33 entries (indices 0-32). Cursor modes 0x13-0x1A map to entries 18-25.
	int mode = (int)g_engine->_scriptExecutor->_mouseMode - 1;
	if (mode < 0 || mode >= kNumLoadedCursors) {
		warning("Invalid cursor mode %d, falling back to Walk cursor", mode);
		mode = (int)Script::MouseMode::Walk - 1;
	}

	if (mode >= (int)g_engine->_imageResources.size() || g_engine->_imageResources[mode]._data == nullptr || g_engine->_imageResources[mode]._width == 0) {
		warning("Cursor data for mode %d is invalid", mode);
		return;
	}

	const uint16 width = g_engine->_imageResources[mode]._width;
	const uint16 height = g_engine->_imageResources[mode]._height;
	const byte *cursorData = g_engine->_imageResources[mode]._data;
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

	CursorMan.replaceCursor(rgbaCursor.data(), width, height, width >> 1, height >> 1, 0, false, &rgbaCursorFormat);
	// Enable a cursor palette so the backend won't re-blit the cursor on
	// every screen palette change. The macs2 engine uses RGBA cursors with
	// baked-in palette colors, so the cursor palette content is irrelevant -
	// it just needs to exist to prevent the backend's setPalette() from
	// triggering blitCursor() which can corrupt the RLE-accelerated surface.
	byte dummyPalette[256 * 3] = {};
	CursorMan.replaceCursorPalette(dummyPalette, 0, 256);
}

View1::View1() : UIElement("View1") {
	_backgroundSurface.copyFrom(g_engine->_sceneBackground);
	currentSpeechActData.onRightSide = false;
	updateCursor();
	setViewPaletteSafely(this, g_engine->_pal);
	_paletteDirty = false;
	CursorMan.showMouse(true);

	// TODO: Check if this works like this
	Character *protagonist = new Character();
	// TODO: Need to properly handle the offset
	// TODO: Remember that the game starts enumerating objects at 1 and not at 0
	protagonist->_gameObject = GameObjects::instance()._objects[0x0];
	_characters.push_back(protagonist);
	_inventorySource = protagonist->_gameObject;

	// inventoryItems.push_back(GameObjects::instance().Objects[0x8 - 1]);

	_inventoryButtonLocations.resize(6);
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
	Common::MemoryReadStream stream(gameObject->_blobs[index].data(), gameObject->_blobs[index].size());

	// Original calls getAnimFrameWidth(1, ...) with mode=1 to reset to frame 1
	uint16 offset = Macs2::BackgroundAnimationBlob::advanceAnimFrame(gameObject->_blobs[index], true, 1);
	// Frame data: offsetX(2), offsetY(2), unknown(2), width(2), height(2), pixels
	// Skip the 6-byte header to reach width/height/pixels for ReadFromStream
	stream.seek(offset + 6, SEEK_SET);
	result->readFromStream(&stream);
	// TODO: Think about proper memory management
	return result;
}

void View1::drawDarkRectangle(uint16 x, uint16 y, uint16 width, uint16 height) {
	Graphics::ManagedSurface s = getSurface();
	for (uint16 xOffset = 0; xOffset < width; xOffset++) {
		for (uint16 yOffset = 0; yOffset < height; yOffset++) {
			const uint16 currentX = x + xOffset;
			const uint16 currentY = y + yOffset;
			const uint32 currentValue = s.getPixel(currentX, currentY);
			const uint32 newValue = g_engine->_shadingTable[currentValue];
			if (currentX < 320 && currentY < 200)
				s.setPixel(currentX, currentY, newValue);
		}
	}
}

void View1::drawBackgroundAnimations(Graphics::ManagedSurface &s) {
	for (int i = 0; i < (int)g_engine->_backgroundAnimations.size(); i++) {
		BackgroundAnimation &current = g_engine->_backgroundAnimations[i];
		BackgroundAnimationBlob &currentBlob = g_engine->_backgroundAnimationsBlobs[i];
		AnimFrame currentFrame = currentBlob.getCurrentFrame();
		drawSprite(current._x, current._y, currentFrame._width, currentFrame._height, currentFrame._data, s, false);
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
	AnimFrame *leftPortrait = currentSpeechActData.speaker->getCurrentPortrait(false, 0);
	AnimFrame *rightPortrait = currentSpeechActData.speaker->getCurrentPortrait(true, 0);
	if (frame == nullptr) {
		return;
	}

	// See l0037_B462: for the calculations below
	// Draw the border
	const int portraitWidth = MAX<int>(leftPortrait ? leftPortrait->_width : 0, rightPortrait ? rightPortrait->_width : 0);
	const int portraitHeight = MAX<int>(leftPortrait ? leftPortrait->_height : 0, rightPortrait ? rightPortrait->_height : 0);
	const Common::Point borderSize(portraitWidth + 0xD, portraitHeight + 0xD);
	drawBorder(currentSpeechActData.position, borderSize, s);

	// Draw the portrait over the border
	Common::Point pos = currentSpeechActData.position + Common::Point(7, 7);
	drawSprite(pos, frame->_width, frame->_height, frame->_data, s, false);
}

void View1::renderString(uint16 x, uint16 y, Common::String s) {
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
			drawSprite(currentX, currentY, data._width, data._height, data._data, surf, false);
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
	uint16 currentX = x;
	uint16 widestGlyph = 1;
	for (uint i = 0; i < numGlyphs; i++) {
		widestGlyph = MAX(widestGlyph, glyphs[i]._width);
	}
	for (auto iter = s.begin(); iter != s.end(); iter++) {
		bool found = false;
		for (uint i = 0; i < numGlyphs; i++) {
			if (glyphs[i]._ascii == *iter) {
				drawSprite(currentX, y, glyphs[i]._width, glyphs[i]._height, glyphs[i]._data, surf, false);
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

void View1::drawGlyphs(Macs2::GlyphData *data, int count, uint16 x, uint16 y, Graphics::ManagedSurface &s) {
	uint16 currentX = x;
	uint16 currentY = y;
	for (int i = 0; i < count; i++) {
		const Macs2::GlyphData &currentData = data[i];
		if (currentX + currentData._width > s.w) {
			currentY += currentData._height;
			currentX = x;
		}
		drawSprite(currentX, currentY, currentData._width, currentData._height, currentData._data, s, false);
		currentX += currentData._width;
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
	for (int y = 0; y < 200; y++) {
		for (int x = 0; x < 320; x++) {
			const uint8 currentValue = overlay[y * 320 + x];
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

void View1::openMainMenu(Common::Point clickedPosition) {
	_isShowingMainMenu = true;
	// Calculate button size from actual icon dimensions (matching original)
	uint16 maxW = 0, maxH = 0;
	for (int i = 0; i < 9 && i < (int)g_engine->_imageResources.size(); i++) {
		if (g_engine->_imageResources[i]._width > maxW)
			maxW = g_engine->_imageResources[i]._width;
		if (g_engine->_imageResources[i]._height > maxH)
			maxH = g_engine->_imageResources[i]._height;
	}
	uint16 btnW = maxW + 6;
	uint16 btnH = maxH + 6;
	Common::Point inventorySize(btnW * 3 + 0x10, btnH * 3 + 0x10);
	Common::Point upperLeft = clickedPosition - inventorySize / 2;
	if (upperLeft.x < 0) {
		upperLeft.x += ABS(upperLeft.x);
	}
	if (upperLeft.y < 0) {
		upperLeft.y += ABS(upperLeft.y);
	}
	Common::Point lowerRight = upperLeft + inventorySize;
	// TODO: No hard coding
	if (lowerRight.x > 320) {
		upperLeft.x -= lowerRight.x - 320;
	}
	if (lowerRight.y > 200) {
		upperLeft.y -= lowerRight.y - 200;
	}
	lowerRight = upperLeft + inventorySize;
	_mainMenuRect = Common::Rect(upperLeft, lowerRight);
	assert(_mainMenuRect.width() == inventorySize.x && _mainMenuRect.height() == inventorySize.y);
	_isShowingMainMenu = true;
}

void View1::drawMainMenu(Graphics::ManagedSurface &s) {
	// Original openActionBarAtPosition (1008:3fba) calls:
	// 1. drawBorderSide(height, width, y, x) — tiles border texture over ENTIRE panel
	// 2. drawBorderOuterHighlights(height, width, y, x) — draws edge highlights
	drawBorderSide(Common::Point(_mainMenuRect.left, _mainMenuRect.top), Common::Point(_mainMenuRect.width(), _mainMenuRect.height()), s);
	drawBorderOuterHighlights(Common::Point(_mainMenuRect.left, _mainMenuRect.top), Common::Point(_mainMenuRect.width(), _mainMenuRect.height()), s);

	// 3x3 grid layout matching openActionBarAtPosition (1008:3fba)
	// Each button is sized to the largest icon + 6px padding, centered in cell
	uint16 maxW = 0, maxH = 0;
	for (int i = 0; i < 9 && i < (int)g_engine->_imageResources.size(); i++) {
		if (g_engine->_imageResources[i]._width > maxW)
			maxW = g_engine->_imageResources[i]._width;
		if (g_engine->_imageResources[i]._height > maxH)
			maxH = g_engine->_imageResources[i]._height;
	}
	uint16 btnW = maxW + 6;
	uint16 btnH = maxH + 6;

	_mainMenuButtonLocations.resize(9);
	for (int i = 0; i < 9 && i < (int)g_engine->_imageResources.size(); i++) {
		int col = i % 3;
		int row = i / 3;
		uint16 cellX = _mainMenuRect.left + 8 + col * btnW;
		uint16 cellY = _mainMenuRect.top + 8 + row * btnH;

		// Original draws drawBorderOuterHighlights per button cell
		drawBorderOuterHighlights(Common::Point(cellX, cellY), Common::Point(btnW, btnH), s);

		AnimFrame &frame = g_engine->_imageResources[i];
		// Center icon within cell
		uint16 iconX = cellX + (btnW - frame._width) / 2;
		uint16 iconY = cellY + (btnH - frame._height) / 2;
		drawSprite(iconX, iconY, frame._width, frame._height, frame._data, s, false);
		_mainMenuButtonLocations[i] = Common::Rect(cellX, cellY, cellX + btnW, cellY + btnH);
	}
}

void View1::setStringBox(const Common::StringArray &sa) {
	_drawnStringBox = sa;
	_isShowingStringBox = true;
	_continueScriptAfterUI = true;

	// TODO: Change cursor, stop animations, hide again
	redraw();
}

void View1::setStringBoxAt(const Common::StringArray &sa, const Common::Point &pos) {
	_stringBoxPosition = pos;
	setStringBox(sa);
}

void View1::clearStringBox(bool continueScript) {
	_isShowingStringBox = false;
	_isShowingDialogueChoice = false;
	_dialogueChoiceCount = 0;
	currentSpeechActData.speaker = nullptr;
	currentSpeechActData.mouthAnimActive = false;
	redraw();
	if (continueScript && _continueScriptAfterUI) {
		_continueScriptAfterUI = false;
		// TODO: Check which one it should be
		g_engine->runScriptExecutor(false);
	} else if (!continueScript) {
		_continueScriptAfterUI = false;
	}
}

int View1::getCharacterArrayIndex(const Character *c) const {
	// TODO: Check if there is a find function somewhere
	for (uint i = 0; i < _characters.size(); i++) {
		if (_characters[i] == c) {
			return i;
		}
	}
	return -1;
}

void View1::startFading(uint16 speed) {
	startFadingWithSpeed(speed);
}

void View1::startFadeToBlack(uint16 speed) {
	// Blocking fade to black matching DOS fadePaletteToBlack (1010:00ba):
	// Starts at fadeValue=0, increments by speed each iteration.
	// Exits when fadeValue > 0x40, then writes all-black palette.
	if (speed == 0)
		speed = 4;
	beginFadeCursorSuppression();

	// Ensure current frame is on screen before fading
	Graphics::ManagedSurface *screen = g_events->getScreen();
	g_system->copyRectToScreen((const byte *)screen->getPixels(),
							   screen->pitch, 0, 0, 320, 200);

	uint fadeValue = 0;
	while (fadeValue <= 0x40 && !g_system->getEventManager()->shouldQuit()) {
		uint32 frameStart = g_system->getMillis();

		applyPaletteWithFade(this, g_engine->_palVanilla, fadeValue);
		g_system->copyRectToScreen((const byte *)g_events->getScreen()->getPixels(),
								   g_events->getScreen()->pitch, 0, 0, 320, 200);
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
	setViewPaletteSafely(this, colors);
	g_system->updateScreen();

	_currentFadeValue = 0x40;
	_fadeMode = FadeMode::None;
	endFadeCursorSuppression(colors);
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
	drawBackgroundAnimations(s);
	drawCharacters(s);

	// Copy pixels to the system screen
	g_system->copyRectToScreen((const byte *)g_events->getScreen()->getPixels(),
							   g_events->getScreen()->pitch, 0, 0, 320, 200);
	g_system->updateScreen();

	// Fade from black: original starts at speed + 0x40 to guarantee first frame
	// is fully black (max 6-bit value is 0x3F, so subtracting 0x44 always clamps to 0)
	int fadeValue = speed + 0x40;
	while (!g_system->getEventManager()->shouldQuit()) {
		uint32 frameStart = g_system->getMillis();

		applyPaletteWithFade(this, g_engine->_palVanilla, fadeValue);
		// Re-copy pixels so the backend redraws with the new palette
		g_system->copyRectToScreen((const byte *)g_events->getScreen()->getPixels(),
								   g_events->getScreen()->pitch, 0, 0, 320, 200);
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
	setViewPaletteSafely(this, g_engine->_pal);
	g_system->updateScreen();

	_currentFadeValue = -1;
	_fadeMode = FadeMode::None;
	_paletteDirty = false;
	endFadeCursorSuppression(g_engine->_pal);
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

bool View1::msgMouseDown(const MouseDownMessage &msg) {
	if (msg._button == MouseMessage::MB_LEFT) {
		// Map mode (depth-based scene preview) from handleInput (1008:e8bf).
		// When currentMode == VM_MAP, clicking on the depth map previews scenes.
		if (_currentMode == ViewMode::VM_MAP) {
			Common::Rect screenRect(320, 200);
			if (screenRect.contains(msg._pos)) {
				uint8 depth = g_engine->_depthMap.getPixel(msg._pos.x, msg._pos.y);
				if (depth > 0 && depth < 0xFA) {
					// Preview sub-scene at depth index.
					// Binary: fileSeek(scene + 0x5DD7 + depth*4). Table starts at scene+0x5DD7.
					g_engine->_fileStream->seek(g_engine->_mapSubSceneTableFilePos + depth * 4, SEEK_SET);
					uint32 subSceneOffset = g_engine->_fileStream->readUint32LE();
					if (subSceneOffset != 0 && subSceneOffset < (uint32)g_engine->_fileStream->size()) {
						startFadeToBlack();
						Graphics::ManagedSurface preview = g_engine->readRLEImage(subSceneOffset, g_engine->_fileStream);
						_backgroundSurface.copyFrom(preview);
						// Read sub-scene palette
						g_engine->_fileStream->read(g_engine->_palVanilla, 0x300);
						memcpy(g_engine->_pal, g_engine->_palVanilla, 0x300);
						for (int p = 0; p < 256 * 3; p++) {
							g_engine->_pal[p] = (g_engine->_pal[p] * 259 + 33) >> 6;
						}
						_paletteDirty = true;
						// Read sub-scene depth map
						Graphics::ManagedSurface subDepth = g_engine->readRLEImage(g_engine->_fileStream->pos(), g_engine->_fileStream);
						g_engine->_depthMap.blitFrom(subDepth);
						startFading();
						redraw();
					}
				} else if (depth == 0xFF) {
					// Return to normal mode: reload current scene, clear map flag
					_currentMode = ViewMode::VM_GAME;
					g_engine->setCursorMode(Script::MouseMode::Walk);
					updateCursor();
					g_engine->changeScene(Scenes::instance()._currentSceneIndex, false);
					g_engine->scheduleRun(true);
				}
			}
			return true;
		}

		// Handle original save/load panel clicks
		if (_isMapPanelActive) {
			handleOriginalSaveLoadClick(msg._pos);
			return true;
		}

		// Handle string boxes
		if (_isShowingStringBox) {
			clearStringBox();
			return true;
		}

		if (_isShowingInventory) {

			for (int i = 0; i < 6; i++) {
				const Common::Rect &current = _inventoryButtonLocations[i];
				if (current.contains(msg._pos)) {
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
						if (_inventoryPage > 0) {
							_inventoryPage--;
						}
						break;
					}
					case InventoryButtonIndex::Down: {
						// Check how many pages we have
						uint16 numPages = (uint16)ceil((double)_inventoryItems.size() / 5.0);
						if (_inventoryPage < numPages - 2) {
							_inventoryPage++;
						}
						break;
					}
					case InventoryButtonIndex::Drop: {
						// Binary handleInventoryClick button 5 / handleDialogueClick button 5.
						// Only active when mode == 0x17 (UseInventory) and an item is held.
						if (g_engine->_scriptExecutor->_mouseMode == Script::MouseMode::UseInventory && _activeInventoryItem != nullptr) {
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
						if (g_engine->_scriptExecutor->_mouseMode == Script::MouseMode::UseInventory) {
							// Binary button 6 with mode 0x17: copy objectId to inventoryItemId,
							// then clear objectId. The inventoryItemId persists for scene use.
							g_engine->_scriptExecutor->_interactedOtherObjectID = 0x400 + _activeInventoryItem->_index;
						} else {
							_activeInventoryItem = nullptr;
							g_engine->_scriptExecutor->_interactedOtherObjectID = 0;
						}
						g_engine->_scriptExecutor->_interactedObjectID = 0;
						_isShowingInventory = false;
						_inventoryPage = 0;
						updateCursor();
						return true;
					}
					}
				}
			}

			// Check if we hit an inventory item
			GameObject *clickedObject = getClickedInventoryItem(msg._pos);

			if (clickedObject != nullptr && g_engine->_scriptExecutor->_mouseMode == Script::MouseMode::Look) {
				// Look at item: object ID uses 0x400 prefix (confirmed from original handleInventoryClick)
				g_engine->_scriptExecutor->_interactedObjectID = 0x400 + clickedObject->_index;
				g_engine->_scriptExecutor->_interactedOtherObjectID = 0;
				g_engine->runScriptExecutor(false);
				return true;
			}
			if (clickedObject != nullptr && g_engine->_scriptExecutor->_mouseMode == Script::MouseMode::Use) {
				_activeInventoryItem = clickedObject;
				g_engine->_scriptExecutor->_interactedObjectID = 0x400 + clickedObject->_index;
				AnimFrame *icon = getInventoryIcon(_activeInventoryItem);
				if (icon != nullptr) {
					// Original copies item icon frame into cursor array slot 0x17 (UseInventory)
					// so the cursor shows the picked-up item
					int cursorSlot = (int)Script::MouseMode::UseInventory - 1;
					uint32 pixelSize = icon->_width * icon->_height;
					delete[] g_engine->_imageResources[cursorSlot]._data;
					g_engine->_imageResources[cursorSlot]._data = new byte[pixelSize];
					memcpy(g_engine->_imageResources[cursorSlot]._data, icon->_data, pixelSize);
					g_engine->_imageResources[cursorSlot]._width = icon->_width;
					g_engine->_imageResources[cursorSlot]._height = icon->_height;
				}
				g_engine->setCursorMode(Script::MouseMode::UseInventory);
				updateCursor();
				return true;
			}
			if (_activeInventoryItem != nullptr && clickedObject != nullptr) {
				// Use item on item (combine): from handleInventoryClick grid hit-test, mode 0x17.
				// Binary sets interactedObjectId (source) + interactedInventoryItemId (target),
				// then triggers runScriptExecutor via g_wHasSavedUiBackground. Does NOT set
				// g_wInventoryCombineFlag here (that's only in the Drop button path).
				g_engine->_scriptExecutor->_interactedObjectID = 0x400 + _activeInventoryItem->_index;
				g_engine->_scriptExecutor->_interactedOtherObjectID = 0x400 + clickedObject->_index;
				g_engine->runScriptExecutor(false);
			}

			return true;
		}

		if (_isShowingMainMenu) {
			for (int i = 0; i < 9; i++) {
				const Common::Rect &current = _mainMenuButtonLocations[i];
				if (current.contains(msg._pos)) {
					MainMenuButtonIndex buttonIndex = (MainMenuButtonIndex)i;
					switch (buttonIndex) {
					case MainMenuButtonIndex::Talk: {
						g_engine->setCursorMode(Script::MouseMode::Talk);
						_isShowingMainMenu = false;
						break;
					}
					case MainMenuButtonIndex::Look: {
						g_engine->setCursorMode(Script::MouseMode::Look);
						_isShowingMainMenu = false;
						break;
					}
					case MainMenuButtonIndex::Use: {
						g_engine->setCursorMode(Script::MouseMode::Use);
						_isShowingMainMenu = false;
						break;
					}
					case MainMenuButtonIndex::Walk: {
						g_engine->setCursorMode(Script::MouseMode::Walk);
						_isShowingMainMenu = false;
						break;
					}
					case MainMenuButtonIndex::Inventory: {
						_isShowingMainMenu = false;
						openInventory(GameObjects::instance().getProtagonistObject());
						break;
					}
					case MainMenuButtonIndex::InventoryUse: {
						// Binary button 6: sets cursor mode to 0x17 (UseInventory) and copies
						// active item ID to interacted item, only if an item is selected.
						_isShowingMainMenu = false;
						if (_activeInventoryItem != nullptr) {
							g_engine->setCursorMode(Script::MouseMode::UseInventory);
							g_engine->_scriptExecutor->_interactedObjectID = 0x400 + _activeInventoryItem->_index;
						}
						break;
					}
					case MainMenuButtonIndex::Map: {
						_isShowingMainMenu = false;
						// Enter map mode from handleActionBarClick (1008:42dc) button 7.
						if (g_engine->_mapImageFileOffset == 0 ||
							g_engine->_mapImageFileOffset >= (uint32)g_engine->_fileStream->size()) {
							break;
						}
						// Load map background image, palette, and depth map from file.
						// Binary: fileSeek(scene+0x5DDB), decodeRLERows, fileRead palette,
						// decodeRLERows depth, drawRLEImage, setCursorMode(0x18).
						startFadeToBlack();
						Graphics::ManagedSurface mapBg = g_engine->readRLEImage(g_engine->_mapImageFileOffset, g_engine->_fileStream);
						_backgroundSurface.copyFrom(mapBg);
						// Read map palette (0x300 bytes immediately after the RLE image)
						g_engine->_fileStream->read(g_engine->_palVanilla, 0x300);
						memcpy(g_engine->_pal, g_engine->_palVanilla, 0x300);
						for (int p = 0; p < 256 * 3; p++) {
							g_engine->_pal[p] = (g_engine->_pal[p] * 259 + 33) >> 6;
						}
						_paletteDirty = true;
						// Read map depth map (RLE image after palette)
						Graphics::ManagedSurface mapDepth = g_engine->readRLEImage(g_engine->_fileStream->pos(), g_engine->_fileStream);
						g_engine->_depthMap.blitFrom(mapDepth);
						// Save file position of the sub-scene offset table (follows depth map).
						g_engine->_mapSubSceneTableFilePos = g_engine->_fileStream->pos();
						_currentMode = ViewMode::VM_MAP;
						g_engine->setCursorMode(Script::MouseMode::PanelUse);
						updateCursor();
						startFading();
						redraw();
						break;
					}
					case MainMenuButtonIndex::SaveLoad: {
						_isShowingMainMenu = false;
						g_engine->openMainMenuDialog();
						break;
					}
					case MainMenuButtonIndex::Close: {
						_isShowingMainMenu = false;
						break;
					}
					}
				}
			}
			updateCursor();
			return true;
		}

		// Handle interactions during script execution
		// From handleInput (1008:f0ad): clicks during script execution set the
		// click state variables and resume the script executor.
		if (g_engine->_scriptExecutor->isExecuting()) {
			if (_isShowingStringBox) {
				clearStringBox();
			}
			if (_isShowingDialogueChoice) {
				// Handle dialogue choice click
			}
			// Set script click state (original: g_wScriptClickFlag=0, X=mouseX, Y=mouseY, Result=1)
			g_engine->_scriptExecutor->_scriptClickFlag = 0;
			g_engine->_scriptExecutor->_scriptClickX = (uint16)msg._pos.x;
			g_engine->_scriptExecutor->_scriptClickY = (uint16)msg._pos.y;
			g_engine->_scriptExecutor->_scriptClickResult = 1;
			g_engine->runScriptExecutor();
			return true;
		}

		if (g_engine->_scriptExecutor->_mouseMode == Script::MouseMode::Walk) {
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

			// Original logic from handleInput (1008:e8bf):
			// If direct path is walkable, walk directly. Otherwise use pathfinding network.
			if (g_engine->isPathWalkable(charPos.y, charPos.x, target.y, target.x)) {
				protagonist->_pathFinalDestination = target;
				protagonist->startLerpTo(target, 1000);
			} else {
				protagonist->_pathFinalDestination = target;
				protagonist->_path.clear();
				bool found = protagonist->calculatePath(target);
				if (found) {
					protagonist->_isFollowingPath = true;
					protagonist->_currentPathIndex = -1;
					protagonist->walkAlongPath();
				} else {
					// No path found — walk directly (will get stuck on obstacles)
					protagonist->startLerpTo(target, 1000);
				}
			}
			return true;
		}

		// Check if we hit something
		// Original order: getHotspotAtPoint first, then drawCharactersAndHitTest overrides.
		// Our order (objects first, fallback to background) produces the same result.
		uint16 index = getHitObjectID(Common::Point(msg._pos.x, msg._pos.y));
		if (index == 0) {
			index = g_engine->getHotspotAtPoint(msg._pos);
		}
		if (index != 0) {
			debug("*** New interaction started");

			// Binary (handleInput 1008:ef2d): stop character movement before interaction.
			// Sets runtime target/finalDest to current position, clears path state.
			Character *protagonist = getCharacterByIndex(Scenes::instance()._currentActorIndex);
			if (protagonist != nullptr) {
				Common::Point pos = protagonist->getPosition();
				protagonist->_endPosition = pos;
				protagonist->_pathFinalDestination = pos;
				protagonist->_path.clear();
				protagonist->_currentPathIndex = 0;
				protagonist->_isFollowingPath = false;
				protagonist->_isLerping = false;
			}

			// Binary (handleInput 1008:ef8f): if mode != 0x17, clear inventory item ID.
			// Note: the binary does NOT touch g_wInventoryActionFlag here.
			if (g_engine->_scriptExecutor->_mouseMode != Script::MouseMode::UseInventory) {
				g_engine->_scriptExecutor->_interactedOtherObjectID = 0;
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
		if (_currentMode == ViewMode::VM_MAP) {
			return true;
		}
		// Handle no other interactions during a script
		if (g_engine->_scriptExecutor->isExecuting()) {
			// From handleInput: right-click during script execution opens the
			// map/save panel ONLY if none of these are active:
			// - IsSceneInitRun, text box, dialogue, overlay, sound/music waits
			if (!_isShowingStringBox && !_isShowingDialogueChoice &&
				!g_engine->_scriptExecutor->_overlayTextStageActive &&
				!g_engine->_scriptExecutor->_waitForSoundPlayback &&
				!g_engine->_scriptExecutor->_waitForMusicControl &&
				!g_engine->_scriptExecutor->_waitForAdlibReady &&
				g_engine->_scriptExecutor->canOpenSaveMenu()) {
				if (ConfMan.getBool("original_menus")) {
					openOriginalSaveLoadPanel();
				} else {
					g_engine->openMainMenuDialog();
				}
			}
			return true;
		}

		// From handleInput (1008:e8bf): right-click when not executing
		// opens the action bar at the mouse position.
		if (!_isShowingMainMenu) {
			openMainMenu(msg._pos);
		} else {
			_isShowingMainMenu = false;
		}
		updateCursor();
		return true;
	}
	return false;
}

bool View1::msgMouseMove(const MouseMoveMessage &msg) {
	_hoverAreaId = g_engine->_scriptExecutor->getAreaAtPoint(msg._pos.x, msg._pos.y);
	_hoverHotspotId = g_engine->getHotspotAtPoint(msg._pos);
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

	if (_isShowingStringBox && !_isShowingDialogueChoice) {
		clearStringBox();
		return true;
	}

	// Binary (handleInput 1008:edff): UI panels only open when not executing and cursor != Disabled.
	if (!g_engine->_scriptExecutor->isExecuting() && g_engine->_scriptExecutor->_mouseMode != Script::MouseMode::Disabled) {
		if (msg.ascii == (uint16)'i') {
			if (!_isShowingInventory) {
				openInventory(GameObjects::instance().getProtagonistObject());
			} else {
				closeInventory();
			}
		} else if (msg.ascii == 'n') {
			Common::Point mousePos = g_system->getEventManager()->getMousePos();
			openMainMenu(mousePos);
		}
	}

	if (msg.ascii >= '1' && msg.ascii <= '9') {
		// Select a visible dialogue option by number key.
		// Register a dialogue choice and act upon it
		uint8 numberPressed = msg.ascii - '1' + 1;
		triggerDialogueChoice(numberPressed);
	}

	return true;
}

void View1::draw() {
	if (_paletteDirty && _currentFadeValue < 0) {
		setViewPaletteSafely(this, g_engine->_pal);
		_paletteDirty = false;
	}

	Graphics::ManagedSurface s = getSurface();

	s.blitFrom(_backgroundSurface);

	// In map mode, only the background (map image) is shown — no characters/animations/UI.
	if (_currentMode == ViewMode::VM_MAP) {
		return;
	}

	// Handle highlighting

	drawBackgroundAnimations(s);
	drawCharacters(s);
	drawOverlayTextEntries();

	if (_isShowingStringBox) {
		showStringBox(_drawnStringBox);
		if (currentSpeechActData.speaker != nullptr) {
			drawCurrentSpeaker(s);
		}
	}

	// We keep the inventory on but don't draw it in case we display a string
	// i.e. a description of an item
	if (_isShowingInventory && !_isShowingStringBox) {
		drawInventory(s);
	}

	if (_isShowingMainMenu) {
		drawMainMenu(s);
	}

	if (_isMapPanelActive) {
		drawOriginalSaveLoadPanel(s);
	}

	// Active inventory item is now shown via the cursor (UpdateCursor uses _cursorData slot 0x16)

	// Get mouse position
	Common::Point mousePos = g_system->getEventManager()->getMousePos();

	if (_isShowingMainMenu && g_engine->enhancementEnabled(kEnhUIUX)) {
		for (int i = 0; i < (int)_mainMenuButtonLocations.size(); i++) {
			if (_mainMenuButtonLocations[i].contains(mousePos)) {
				static const char *const buttonNames[] = {
					"Talk", "Look", "Use", "Walk", "Inventory",
					"Use Item", "Map", "Save/Load", "Close"
				};
				renderString(mousePos.x + 20, mousePos.y + 20, buttonNames[i]);
				break;
			}
		}
	}

	if (_isShowingInventory && g_engine->enhancementEnabled(kEnhUIUX)) {
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
}

bool View1::tick() {
	// TODO: Check if this pattern works or it would be better different
	// TODO: Check if loading also works with this pattern
	if (!_started) {
		g_engine->changeScene(Scenes::instance()._currentSceneIndex);
		_started = true;
	}

	// Map mode: no game logic runs (binary: handleInput skips everything when 0x61db set)
	if (_currentMode == ViewMode::VM_MAP) {
		redraw();
		return true;
	}
	// Cycle the palette
	++_offset;

	// Music fade tick from gameTick (1008:e556).
	// Processes volume fade in/out each frame when active.
	Script::ScriptExecutor *se = g_engine->_scriptExecutor;
	if (se->_activeMusicSlot != 0 && se->_musicControlMode != 0) {
		if (se->_musicControlMode == 1) {
			// Fade out: volume -= step
			int vol = (int)se->_musicControlVolume - (int)se->_musicControlParam;
			if (vol < 1) {
				se->_musicControlMode = 0;
				se->_musicControlVolume = 0;
				vol = 0;
			} else {
				se->_musicControlVolume = vol;
			}
			g_engine->getAdlib()->SetVolume(se->_musicControlVolume);
		} else {
			// Fade in: volume += step. When >= 63: stop music.
			int vol = (int)se->_musicControlVolume + (int)se->_musicControlParam;
			if (vol >= 0x3F) {
				se->_musicControlMode = 0;
				se->_activeMusicSlot = 0;
				g_engine->getAdlib()->StopMusic();
			} else {
				se->_musicControlVolume = vol;
				g_engine->getAdlib()->SetVolume(se->_musicControlVolume);
			}
		}
	}

	// Below is redundant since we're only cycling the palette, but it demonstrates
	// how to trigger the view to do further draws after the first time, since views
	// don't automatically keep redrawing unless you tell it to
	// if ((_offset % 256) == 0)
	//	redraw();

	// Update the flag
	// Background animation advance - matching original gameTick (1008:e556):
	// g_wBgAnimTickCounter is incremented once per game frame (~10.8fps).
	// Mode 3 (_bgAnimMode==3): advance when counter > 1 (every 2 game frames ≈ 185ms)
	// Mode 2 (_bgAnimMode==2): advance when counter > 0x27 (every 39 game frames ≈ 3.6s)
	_bgAnimTickCounter++;

	if (_bgAnimTickCounter > 1 && g_engine->_bgAnimMode == 3) {
		_bgAnimTickCounter = 0;
		_flagFrameIndex++;
		if (_flagFrameIndex == 3) {
			_flagFrameIndex = 0;
		}
		_guyFrameIndex++;
		_guyFrameIndex = _guyFrameIndex % 6;
		// Advance background animation blobs (original: updateBackgroundAnimations)
		for (auto &blob : g_engine->_backgroundAnimationsBlobs) {
			BackgroundAnimationBlob::advanceAnimFrame(blob._blob, true, 2);
		}
		redraw();
	}

	if (_bgAnimTickCounter > 0x27 && g_engine->_bgAnimMode == 2) {
		_bgAnimTickCounter = 0;
		_flagFrameIndex++;
		if (_flagFrameIndex == 3) {
			_flagFrameIndex = 0;
		}
		_guyFrameIndex++;
		_guyFrameIndex = _guyFrameIndex % 6;
		// Advance background animation blobs (original: updateBackgroundAnimations)
		for (auto &blob : g_engine->_backgroundAnimationsBlobs) {
			BackgroundAnimationBlob::advanceAnimFrame(blob._blob, true, 2);
		}
		redraw();
	}

	// Advance portrait animation once per tick (matching handleDialogueInput 1008:b4bd)
	if (_isShowingStringBox && currentSpeechActData.speaker != nullptr && currentSpeechActData.mouthAnimActive) {
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

	for (auto currentCharacter : _characters) {
		currentCharacter->update();
	}

	redraw();
	return true;
}

void View1::drawInventory(Graphics::ManagedSurface &s) {
	// First, draw the whole background
	// Happens around l0037_47A1:

	uint16 maxWidthButtonIcon = 0;  // [0FE0h]
	uint16 maxHeightButtonIcon = 0; // [0FE2h]
	for (uint16 index : g_engine->inventoryIconIndices) {
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
	drawBorderOuterHighlights(Common::Point(x, y), Common::Point(width, height), s);

	uint16 buttonX = (s.w / 2) - (buttonW + 4) * 3 + 2;
	uint16 buttonY = y + height - 4 - buttonH;

	// Draw the buttons at the bottom
	for (int i = 0; i < 6; i++) {
		uint16 index = g_engine->inventoryIconIndices[i];
		AnimFrame &currentFrame = g_engine->_imageResources[index - 1];
		drawPressedBorderOuterHighlights(Common::Point(buttonX, buttonY), Common::Point(buttonW, buttonH), s);
		uint16 iconX = (buttonW / 2 + buttonX) - currentFrame._width / 2;
		uint16 iconY = (buttonH / 2 + buttonY) - currentFrame._height / 2;
		_inventoryButtonLocations[i] = Common::Rect(Common::Point(buttonX, buttonY), buttonW, buttonH);
		drawSprite(iconX, iconY, currentFrame._width, currentFrame._height, currentFrame._data, s, false);
		buttonX += buttonW + 4;
	}
	Common::Rect sourceRect(Common::Point((s.w / 2) - ((slotW + 4) * 5 + 4) / 2 + 1, y + 5),
							(slotW + 4) * 5 + 2, (slotH + 4) * 2 + 2);
	s.rawBlitFrom(*buffer, sourceRect, Common::Point(sourceRect.left, sourceRect.top));

	drawBorderOuterHighlights(Common::Point(
								  (s.w / 2) - ((slotW + 4) * 5 + 4) / 2,
								  y + 4),
							  Common::Point(
								  (slotW + 4) * 5 + 4,
								  (slotH + 4) * 2 + 4),
							  s);

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
	uint16 itemIndex = _inventoryPage * 10;
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
						   icon->_width, icon->_height, icon->_data, s, false);
			}
			itemIndex++;
			itemX += slotW + 4;
		}
		itemX = itemXStart;
		itemY += slotH + 4;
	}
}

GameObject *View1::getClickedInventoryItem(const Common::Point &p) {
	Common::Rect currentInventorySlot(_inventoryGridUpperLeft, _inventoryGridUpperLeft + _inventorySlotSize);

	uint16 itemIndex = _inventoryPage * 10;
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

void View1::drawSprite(int16 x, int16 y, uint16 width, uint16 height, byte *data, Graphics::ManagedSurface &s, bool mirrored, bool useDepth, uint8 depth) {
	for (int currentX = 0; currentX < width; currentX++) {
		int actualX = mirrored ? width - currentX - 1 : currentX;
		for (int currentY = 0; currentY < height; currentY++) {
			uint8 val = data[currentY * width + currentX];
			if (val != 0) {
				int finalX = x + actualX;
				int finalY = y + currentY;
				if (finalX >= 0 && finalX < s.w && finalY >= 0 && finalY < s.h) {
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

void View1::drawSprite(const Common::Point &pos, uint16 width, uint16 height, byte *data, Graphics::ManagedSurface &s, bool mirrored, bool useDepth, uint8 depth) {
	drawSprite(pos.x, pos.y, width, height, data, s, mirrored, useDepth, depth);
}

void View1::drawSpriteClipped(uint16 x, uint16 y, Common::Rect &clippingRect, uint16 width, uint16 height, const byte *const data, Graphics::ManagedSurface &s) {
	for (int currentX = 0; currentX < width; currentX++) {
		for (int currentY = 0; currentY < height; currentY++) {
			uint8 val = data[currentY * width + currentX];
			if (val != 0) {
				if (clippingRect.contains(x + currentX, y + currentY)) {
					if (x + currentX < 320 && y + currentY < 200)
						s.setPixel(x + currentX, y + currentY, val);
				}
			}
		}
	}
}

void View1::drawSpriteClipped(uint16 x, uint16 y, Common::Rect &clippingRect, const Sprite &sprite, Graphics::ManagedSurface &s) {
	drawSpriteClipped(x, y, clippingRect, sprite._width, sprite._height, sprite._data.data(), s);
}

void View1::drawSpriteAdvanced(uint16 x, uint16 y, uint16 width, uint16 height, uint16 scaling, const byte *data, Graphics::ManagedSurface &s) {
	int xScaling = 0;
	int yScaling = 0;

	int currentTargetX = 0;
	int currentTargetY = 0;

	// Outer loop: Advance over lines
	// Inner loop: Advance over rows
	for (int currentSourceY = 0; currentSourceY < height; currentSourceY++) {
		currentTargetX = 0;
		xScaling = 0;
		for (int currentSourceX = 0; currentSourceX < width; currentSourceX++) {
			uint8 val = data[currentSourceY * width + currentSourceX];
			if (val != 0) {
				uint16 finalX = x + currentTargetX;
				uint16 finalY = y + currentTargetY;
				if (finalX < s.w && finalY < s.h)
					s.setPixel(finalX, finalY, val);
			}
			xScaling += 0x64;
			currentTargetX++;
			do {
				// Handle x scaling
				if (xScaling <= scaling) {
					// This means we repeat a pixel
					currentSourceX--;
					break;
				}
				xScaling -= scaling;
				currentSourceX++;
			} while (currentSourceX < width);
		}
		yScaling += 0x64;
		currentTargetY++;
		do {
			// Handle y scaling
			if (yScaling <= scaling) {
				// This means we repeat a row
				currentSourceY--;
				break;
			}
			yScaling -= scaling;
			currentSourceY++;
		} while (currentSourceY < height);
	}
}

void View1::drawSpriteAdvanced(const Common::Point &pos, uint16 width, uint16 height, uint16 scaling, const Sprite &sprite, Graphics::ManagedSurface &s) {
	drawSpriteAdvanced(pos.x, pos.y, width, height, scaling, sprite._data.data(), s);
}

void View1::drawSpriteSuperAdvanced(const Common::Point &pos, const Sprite &sprite, uint16 scaling, bool mirrored, bool useDepth, uint8 depth, Graphics::ManagedSurface &s, uint8 shadowIntensity) {
	const uint16 &x = pos.x;
	const uint16 &y = pos.y;
	const uint16 &width = sprite._width;
	const uint16 &height = sprite._height;
	const Common::Array<uint8> data = sprite._data;
	int xScaling = 0;
	int yScaling = 0;

	int currentTargetX = 0;
	int currentTargetY = 0;

	// Outer loop: Advance over lines
	// Inner loop: Advance over rows
	for (int currentSourceY = 0; currentSourceY < height; currentSourceY++) {
		currentTargetX = 0;
		xScaling = 0;
		for (int currentSourceX = 0; currentSourceX < width; currentSourceX++) {
			int actualX = mirrored ? width - currentSourceX - 1 : currentSourceX;
			uint8 val = data[currentSourceY * width + actualX];
			if (val != 0) {
				uint16 finalX = x + currentTargetX;
				uint16 finalY = y + currentTargetY;
				if (finalX < s.w && finalY < s.h) {
					// Depth test: draw pixel only if background depth < character depth
					// Validated against drawSpriteTransparent (1010:0ed1): *pbVar12 < param_4
					uint8 bgDepth = g_engine->_depthMap.getPixel(finalX, finalY);
					if (!useDepth || bgDepth < depth) {
						// Binary drawAnimFrameShaded/drawAnimFrameDepth: apply shading table
						// when shadowIntensity > 0 (from getShadingValueAt, clamped to 32)
						if (shadowIntensity > 0) {
							val = g_engine->_shadingTable[val];
						}
						s.setPixel(finalX, finalY, val);
					}
				}
			}
			xScaling += 0x64;
			currentTargetX++;
			do {
				// Handle x scaling
				if (xScaling <= scaling) {
					// This means we repeat a pixel
					currentSourceX--;
					break;
				}
				xScaling -= scaling;
				currentSourceX++;
			} while (currentSourceX < width);
		}
		yScaling += 0x64;
		currentTargetY++;
		do {
			// Handle y scaling
			if (yScaling <= scaling) {
				// This means we repeat a row
				currentSourceY--;
				break;
			}
			yScaling -= scaling;
			currentSourceY++;
		} while (currentSourceY < height);
	}
}

void View1::drawCharacters(Graphics::ManagedSurface &s) {
	// Y-sort characters (quicksort by Y position, ascending = back to front)
	// from sortObjectListByY / buildSortedObjectList (1008:8cf2)
	Common::Array<Character *> sorted(_characters);
	Common::sort(sorted.begin(), sorted.end(), [](Character *a, Character *b) {
		return a->getPosition().y < b->getPosition().y;
	});

	for (auto current : sorted) {
		if (!current->_gameObject->_isVisible) {
			continue;
		}

		// Bounds attachment from drawAllCharacters (1008:90a2):
		// When HasBoundsAttachment is set, position is relative to parent object.
		if (current->_gameObject->_hasBoundsAttachment) {
			GameObject *parent = GameObjects::getObjectByIndex(current->_gameObject->BoundsAttachmentObjectID);
			if (parent != nullptr) {
				current->_gameObject->_position.x = parent->_position.x + (int16)current->_gameObject->BoundsAttachmentValue1;
				current->_gameObject->_position.y = parent->_position.y + (int16)current->_gameObject->BoundsAttachmentValue2;
				current->_gameObject->_verticalOffsetScale = parent->_verticalOffsetScale + (int16)current->_gameObject->BoundsAttachmentValue3;
			}
		}

		AnimFrame *frame = current->getCurrentAnimationFrame();
		bool mirror = current->_shouldMirrorCurrentAnimation;

		// AnimFrame *frame = current->GetCurrentPortrait();
		uint8 depth = current->getPosition().y;
		if (depth == 0) {
			// TODO: This is a quick fix for the issue of the gangster at the beginning not being removed properly
			continue;
		}
		uint8 bgDepth = g_engine->_depthMap.getPixel(current->getPosition().x, current->getPosition().y);
		if (DebugMan.isDebugChannelEnabled(kDebugGraphics)) {
			g_system->setWindowCaption(Common::String::format("Depth %u vs. %u", depth, bgDepth));
		}
		// Only output debug values for the character
		uint16 scalingFactor = calculateCharacterScaling(depth, current->_gameObject->_index == 1);
		// Adjust the position based on the scale
		Common::Point actualPosition = current->getPosition() - Common::Point(0, current->getVerticalOffset());

		// Binary drawAllCharacters (1008:90a2) three draw modes based on per-object flags:
		//   HasScaling (+0x186) ON  -> depth-tested + scaled + shaded
		//   HasScaling OFF, HasShading (+0x185) ON -> depth-tested + shaded (no scaling)
		//   Both OFF -> plain blit (no depth test, no shading)
		uint8 shadowIntensity = 0;
		if (g_engine->_shadowMap.w > 0) {
			int sx = CLIP<int>(current->getPosition().x, 0, 319);
			int sy = CLIP<int>(current->getPosition().y, 0, 199);
			shadowIntensity = MIN<uint8>(g_engine->_shadowMap.getPixel(sx, sy), 32);
		}

		if (current->_gameObject->_hasScaling) {
			drawSpriteSuperAdvanced(actualPosition - frame->getBottomMiddleOffset(scalingFactor), frame->asSprite(), scalingFactor, mirror, true, depth, s, shadowIntensity);
		} else if (current->_gameObject->_hasShading) {
			drawSpriteSuperAdvanced(actualPosition - frame->getBottomMiddleOffset(100), frame->asSprite(), 100, mirror, true, depth, s, shadowIntensity);
		} else {
			drawSpriteSuperAdvanced(actualPosition - frame->getBottomMiddleOffset(100), frame->asSprite(), 100, mirror, false, depth, s, 0);
		}

		if (DebugMan.isDebugChannelEnabled(kDebugGraphics)) {
			Common::String number = Common::String::format("%u", current->_gameObject->_orientation);
			renderString(current->getPosition(), number.c_str());
			Common::Rect screenRect(0, 0, 320, 200);
			if (screenRect.contains(current->getPosition())) {
				s.setPixel(current->getPosition().x, current->getPosition().y, 0xFF);
			}
		}
	}
}

void View1::showSpeechAct(uint16 characterIndex, const Common::Array<Common::String> &strings, const Common::Point &position, bool onRightSide) {
	setStringBox(strings);
	_isShowingDialogueChoice = false;
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
	debug("Layout speech act: speaker=%u rawPos=(%d,%d) rightSide=%u portraitBorderPos=(%d,%d) textBorderPos=(%d,%d) textBorderSize=(%d,%d) text=\"%s\"",
		  characterIndex, position.x, position.y, onRightSide ? 1 : 0,
		  currentSpeechActData.position.x, currentSpeechActData.position.y,
		  _stringBoxPosition.x, _stringBoxPosition.y, totalWidth, totalHeight, joinDebugStrings(strings).c_str());
	debugC(kDebugScript,
		   "Layout speech act: speaker=%u rawPos=(%d,%d) rightSide=%u portraitBorderPos=(%d,%d) textBorderPos=(%d,%d) textBorderSize=(%d,%d) text=\"%s\"",
		   characterIndex, position.x, position.y, onRightSide ? 1 : 0,
		   currentSpeechActData.position.x, currentSpeechActData.position.y,
		   _stringBoxPosition.x, _stringBoxPosition.y, totalWidth, totalHeight, joinDebugStrings(strings).c_str());

	if (_autoclickActive) {
		clearStringBox();
	}
}

void View1::drawBorder(const Common::Point &pos, const Common::Point &size, Graphics::ManagedSurface &s) {
	// fn0037_A65D proc
	constexpr uint16 width = 6;
	debugC(kDebugScript, "Render border: pos=(%d,%d) size=(%d,%d)", pos.x, pos.y, size.x, size.y);

	// g_wMapPanelPageIndex == 1: draw shaded background + 4 border sides
	// g_wMapPanelPageIndex != 1: fill entire area with border texture (not implemented)
	// Currently always using mode 1 path.
	drawDarkRectangle(pos.x + 1, pos.y + 1, size.x - 1, size.y - 1);

	// Left side
	drawBorderSide(pos, Common::Point(width, size.y), s);

	// Right side (verified: drawBorderSide(6, height, x+width-6, y) in disassembly)
	drawBorderSide(pos + Common::Point(size.x - width, 0), Common::Point(width, size.y), s);

	// Top side
	drawBorderSide(pos, Common::Point(size.x, width), s);

	// Bottom side
	drawBorderSide(pos + Common::Point(0, size.y - width), Common::Point(size.x, width), s);

	// Add the function for filling a side of the border
	// Algorithm
	// Set up clipping rect on one side
	// Draw the texture enough times in x and y to fill the clipping rect

	// Highlights and shadows (outer edge, inner edge, inner border)
	// Top highlight
	drawHorizontalBorderHighlight(pos + Common::Point(1, 1), size.x - 1, 0x1012, s);
	// Left highlight
	drawVerticalBorderHighlight(pos + Common::Point(1, 1), size.y - 1, 0x1012, s);
	// Bottom shadow
	drawHorizontalBorderHighlight(pos + Common::Point(1, size.y - 1), size.x - 1, 0x1011, s);
	// Right shadow
	drawVerticalBorderHighlight(pos + Common::Point(size.x - 1, 1), size.y - 1, 0x1011, s);
	// Inner top shadow
	drawHorizontalBorderHighlight(pos + Common::Point(6, 6), size.x - 0xB, 0x1011, s);
	// Inner left shadow
	drawVerticalBorderHighlight(pos + Common::Point(6, 6), size.y - 0xB, 0x1011, s);
	// Inner bottom highlight
	drawHorizontalBorderHighlight(pos + Common::Point(6, size.y - width), size.x - 0xB, 0x1012, s);
	// Inner right highlight
	drawVerticalBorderHighlight(pos + Common::Point(size.x - width, width), size.y - 0xB, 0x1012, s);
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
			drawSpriteClipped(currentX, currentY, clippingRect, sprite._width, sprite._height, sprite._data, s);
			currentX += sprite._width;
		}
		currentX = clippingRect.left;
		currentY += sprite._height;
	}
}

void View1::drawBorderOuterHighlights(const Common::Point &pos, const Common::Point &size, Graphics::ManagedSurface &s) {
	drawHorizontalBorderHighlight(pos, size.x + 1, 0x1010, s);
	drawVerticalBorderHighlight(pos, size.y + 1, 0x1010, s);
	drawHorizontalBorderHighlight(pos + Common::Point(0, size.y), size.x + 1, 0x1010, s);
	drawVerticalBorderHighlight(pos + Common::Point(size.x, 0), size.y + 1, 0x1010, s);
	drawHorizontalBorderHighlight(pos + Common::Point(1, 1), size.x - 1, 0x1012, s);
	drawVerticalBorderHighlight(pos + Common::Point(1, 1), size.y - 1, 0x1012, s);
	drawHorizontalBorderHighlight(pos + Common::Point(1, size.y - 1), size.x - 1, 0x1011, s);
	drawVerticalBorderHighlight(pos + Common::Point(size.x - 1, 1), size.y - 1, 0x1011, s);
}

void View1::drawPressedBorderOuterHighlights(const Common::Point &pos, const Common::Point &size, Graphics::ManagedSurface &s) {
	drawHorizontalBorderHighlight(pos, size.x + 1, 0x1010, s);
	drawVerticalBorderHighlight(pos, size.y + 1, 0x1010, s);
	drawHorizontalBorderHighlight(pos + Common::Point(0, size.y), size.x + 1, 0x1010, s);
	drawVerticalBorderHighlight(pos + Common::Point(size.x, 0), size.y + 1, 0x1010, s);
	drawHorizontalBorderHighlight(pos + Common::Point(1, 1), size.x - 1, 0x1011, s);
	drawVerticalBorderHighlight(pos + Common::Point(1, 1), size.y - 1, 0x1011, s);
	drawHorizontalBorderHighlight(pos + Common::Point(1, size.y - 1), size.x - 1, 0x1011, s);
	drawVerticalBorderHighlight(pos + Common::Point(size.x - 1, 1), size.y - 1, 0x1011, s);
}

Macs2::AnimFrame *View1::getUISprite(uint32 offset) {
	if (offset == 0x1012) {
		return &g_engine->_imageResources[30];
	} else if (offset == 0x1011) {
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
		drawSpriteClipped(currentX, currentY, clippingRect, sprite->_width, sprite->_height, sprite->_data, s);
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
		drawSpriteClipped(currentX, currentY, clippingRect, sprite->_width, sprite->_height, sprite->_data, s);
		currentY += sprite->_height;
	}
}

void View1::drawImageResources(Graphics::ManagedSurface &s) {
	uint16 x = 0;
	uint16 y = 0;
	uint16 currentMaxHeight = 0;
	for (AnimFrame &current : g_engine->_imageResources) {
		if (x + current._width > 320) {
			y += currentMaxHeight;
			x = 0;
			currentMaxHeight = 0;
		}
		drawSprite(Common::Point(x, y), current._width, current._height, current._data, s, false);
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
	_isShowingDialogueChoice = true;
	_dialogueChoiceCount = choices.size();
}

void View1::triggerDialogueChoice(uint8 index) {
	if (!_isShowingDialogueChoice || index < 1 || index > _dialogueChoiceCount) {
		warning("Ignoring dialogue choice %u without an active matching choice UI", index);
		return;
	}

	// TODO: Confirm that these two are really set accordingly
	g_engine->_scriptExecutor->setVariableValue(0x0d, index, 0);
	g_engine->_scriptExecutor->_chosenDialogueOption = index;

	// TODO: Should check where this happens, but seems like we need to close the
	// options ourselves
	// TODO: Check if the run script after UI needs to be overridden here to not
	// schedule an unnecessary run
	clearStringBox(false);
	// TODO: Not sure about the first run variable here
	g_engine->runScriptExecutor();
}

uint16 View1::calculateCharacterScaling(uint16 characterY, bool updateDebugValues) {
	// Calculates sprite scaling factor based on character Y position and scene depth params.
	// Original at walkAlongPath (1008:1bb8). Formula (signed, no clamping):
	//   depthOffset = (characterY - _walkDepthThresholdY) * _walkDepthScaleFactor / 100
	//   scalingFactor = _walkBaseSpeedPct + depthOffset

	int32 depthOffset = ((int32)characterY - (int32)g_engine->_walkDepthThresholdY) *
						(int32)g_engine->_walkDepthScaleFactor / 100;
	int32 scalingFactor = (int32)g_engine->_walkBaseSpeedPct + depthOffset;

	if (updateDebugValues) {
		_scalingValues.characterY = characterY;
		_scalingValues.scalingFactor = scalingFactor;
	}
	return (uint16)scalingFactor;
}

uint16 View1::getHitObjectID(const Common::Point &pos) const {
	// Matches drawCharactersAndHitTest (1008:8d65): characters are tested back-to-front
	// (sorted by Y ascending), and depth testing ensures clicks don't hit characters
	// hidden behind foreground objects.
	Common::Array<Character *> sorted(_characters);
	Common::sort(sorted.begin(), sorted.end(), [](Character *a, Character *b) {
		return a->getPosition().y < b->getPosition().y;
	});

	uint16 hitResult = 0;
	for (auto currentCharacter : sorted) {
		if (!currentCharacter->_gameObject->_isVisible || !currentCharacter->_gameObject->_isClickable) {
			continue;
		}
		auto animFrame = currentCharacter->getCurrentAnimationFrame();
		if (animFrame == nullptr) {
			continue;
		}

		Common::Point localPoint = pos - (currentCharacter->getPosition() - animFrame->getBottomMiddleOffset());
		if (localPoint.x < 0 || localPoint.x >= animFrame->_width ||
			localPoint.y < 0 || localPoint.y >= animFrame->_height) {
			continue;
		}
		if (animFrame->_data[localPoint.y * animFrame->_width + localPoint.x] == 0) {
			continue;
		}

		// Depth test: character is only clickable if depth map allows drawing at this position
		uint8 characterDepth = currentCharacter->getPosition().y;
		if (pos.x >= 0 && pos.x < 320 && pos.y >= 0 && pos.y < 200) {
			uint8 bgDepth = g_engine->_depthMap.getPixel(pos.x, pos.y);
			if (bgDepth >= characterDepth) {
				continue;
			}
		}

		// Back-to-front: last hit wins (frontmost character)
		hitResult = 0x0400 + currentCharacter->_gameObject->_index;
	}
	return hitResult;
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

uint8 Character::lookupWalkability(const Common::Point &p) const {
	Common::Rect screenRect(320, 200);
	if (!screenRect.contains(p)) {
		return 0x00;
	}
	uint32 value = g_engine->_pathfindingMap.getPixel(p.x, p.y);
	// Values 0xC8..0xEF use the pathfinding override table (opcode 0x12)
	if (value >= 0xC8 && value <= 0xEF) {
		uint16 overrideResult;
		if (g_engine->getPathfindingOverride(value, overrideResult)) {
			return (uint8)overrideResult;
		}
		// Override not active - return non-walkable
		return 0xFF;
	}
	return (uint8)value;
}

bool Character::isWalkable(const Common::Point &p) const {
	uint8 walkability = lookupWalkability(p);
	return walkability < 0xC8;
}

Character::Character() {
	_pathfindingOverlay = Common::Array<uint8>(320 * 200, 0);
	_executeScriptOnFinishLerp = false;
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
			if (canNodeConnectSourceToTarget(i, charPos, target, reachable, nodeCount)) {
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
		_path.clear();
		_endPosition = target;
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
			_endPosition = target;
			return false;
		}
	}

	// Step 5: Skip-forward optimization - skip nodes that can already see the destination
	_currentPathIndex = 0;
	while (_currentPathIndex + 1 < (int16)_path.size()) {
		const Common::Point &nextNodePos = g_engine->pathfindingPoints[_path[_currentPathIndex + 1] - 1]._position;
		if (!g_engine->isPathWalkable(nextNodePos.y, nextNodePos.x, target.y, target.x))
			break;
		_currentPathIndex++;
	}

	// Set immediate target to the current path node
	const Common::Point &firstTarget = g_engine->pathfindingPoints[_path[_currentPathIndex] - 1]._position;
	_endPosition = firstTarget;
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
	if (result >= 0xC8) {
		result = 0;
	}

	if (_gameObject->_verticalOffsetScale != 0) {
		result = (result * _gameObject->_verticalOffsetScale) / 100;
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
		_endPosition = _pathFinalDestination;
		_stepDeltaX = abs(_endPosition.x - _gameObject->_position.x);
		_stepDeltaY = abs(_endPosition.y - _gameObject->_position.y);
		_stepError = 0;
		_stepDirectionSet = false;
		return false; // No more path segments after this
	}
	const uint16 nodeIdx = _path[_currentPathIndex];
	const Common::Point &nodePos = g_engine->pathfindingPoints[nodeIdx - 1]._position;
	_endPosition = nodePos;
	_stepDeltaX = abs(_endPosition.x - _gameObject->_position.x);
	_stepDeltaY = abs(_endPosition.y - _gameObject->_position.y);
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

Macs2::AnimFrame *Character::getCurrentAnimationFrame() {
	// Binary (drawCharactersAndHitTest at 1008:8d65):
	// animSlotIndex = orientation directly (piVar5[3])
	// If runtime+0x22D >= 0 && runtime+0x22D == orientation, use slot 0x15 instead.
	// No mirroring in renderer — blob data is pre-flipped at load time by mirrorAnimBlob.
	// If slot is not loaded (flag +0x33 == 0) or blob ptr is null, original returns error.
	int blobIndex = _gameObject->_orientation - 1;

	// Binary: if (runtime+0x22D >= 0 && runtime+0x22D == orientation) slot = 0x15
	bool useOverload = (_gameObject->overloadAnimTriggerDirection != 0x7FFF &&
						(int16)_gameObject->overloadAnimTriggerDirection >= 0 &&
						_gameObject->overloadAnimTriggerDirection == _gameObject->_orientation);

	_shouldMirrorCurrentAnimation = false;

	if (useOverload) {
		if (_gameObject->overloadAnimation.empty()) {
			return nullptr;
		}
	} else {
		if (blobIndex < 0 || blobIndex >= (int)_gameObject->_blobs.size() || _gameObject->_blobs[blobIndex].empty()) {
			return nullptr;
		}
	}

	Common::Array<uint8> &blob = useOverload ? _gameObject->overloadAnimation : _gameObject->_blobs[blobIndex];

	// Advance and retrieve current frame (called once per draw = once per tick)
	BackgroundAnimationBlob::advanceAnimFrame(blob, true, 2);
	uint16 offset = BackgroundAnimationBlob::advanceAnimFrame(blob, false, 0x0);
	// My remaining code expects to get dialed to the width and height directly - TODO make uniform
	offset += 6;
	AnimFrame *result = new AnimFrame();
	Common::MemoryReadStream stream(blob.data(), blob.size());
	stream.seek(offset);
	result->readFromStream(&stream);
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
	// My remaining code expects to get dialed to the width and height directly - TODO make uniform
	offset += 6;
	AnimFrame *result = new AnimFrame();
	Common::MemoryReadStream stream(_gameObject->_blobs[portraitBlobIndex].data(), _gameObject->_blobs[portraitBlobIndex].size());
	stream.seek(offset);
	result->readFromStream(&stream);
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
	_endPosition = target;
	_startTime = g_events->currentMillis;
	_duration = duration;
	_isLerping = true;
	_lerpIgnoresObstacles = ignoreObstacles;

	// Reset Bresenham state - direction will be calculated on first Update()
	_stepDirectionSet = false;
	_stepDeltaX = abs(_endPosition.x - _startPosition.x);
	_stepDeltaY = abs(_endPosition.y - _startPosition.y);
	_stepError = 0;
}

void Character::startPickup(Macs2::GameObject *object) {
	_pickedUpObject = object;
	_executeScriptOnFinishLerp = true;
	startLerpTo(_pickedUpObject->_position, 1000);
}

void Character::registerWaitForMovementFinishedEvent() {

	// For now, we are treating this one as a flag to send an event
	// even if we are not lerping, so that we have a delay between action 0x11
	// and the new execution
	_executeScriptOnFinishLerp = true;
}

void Character::update() {
	if (!_isLerping && !_isFollowingPath) {
		// Pickup animation frame counter logic from drawAllCharacters (1008:90a2).
		// While orientation == 0x11, counter increments each frame.
		// At _pickupFrameStart: item is transferred to inventory.
		// At _pickupFrameEnd: animation ends, orientation/cursor restored.
		if (_pickedUpObject != nullptr && _gameObject->_orientation == 0x11) {
			View1 *currentView = (View1 *)g_engine->findView("View1");

			// At _pickupFrameStart: transfer item to inventory
			if (!_pickupItemTransferred && _pickupFrameCounter == _gameObject->_pickupFrameStart) {
				_pickupItemTransferred = true;
				Character *pickedUpCharacter = currentView->getCharacterByIndex(_pickedUpObject->_index);
				if (pickedUpCharacter != nullptr) {
					int index = currentView->getCharacterArrayIndex(pickedUpCharacter);
					if (index >= 0) {
						currentView->_characters.remove_at(index);
					}
				}
				_pickedUpObject->_sceneIndex = _gameObject->_index + 0x400;
				if (currentView->_inventorySource != nullptr && currentView->_inventorySource->_index == _gameObject->_index) {
					currentView->_inventoryItems.push_back(_pickedUpObject);
				}
			}

			// At _pickupFrameEnd: end pickup animation
			if (_pickupFrameCounter == _gameObject->_pickupFrameEnd) {
				_gameObject->_orientation = _previousOrientation;
				if (g_engine->_scriptExecutor->_pickupInProgress) {
					g_engine->_scriptExecutor->_pickupInProgress = false;
					g_engine->_scriptExecutor->_pickupActorObjectID = 0;
					g_engine->_scriptExecutor->_pickupTargetObjectID = 0;
					g_engine->setCursorMode(g_engine->_scriptExecutor->_savedPickupMouseMode);
					currentView->updateCursor();
				}
				g_engine->_scriptExecutor->_walkTargetObjectIndex = 0;
				_pickedUpObject = nullptr;
				g_engine->_scriptExecutor->_interactedObjectID = 0x0000;
				g_engine->_scriptExecutor->_interactedOtherObjectID = 0x0000;
				if (_executeScriptOnFinishLerp) {
					_executeScriptOnFinishLerp = false;
					g_engine->_scriptExecutor->_isRepeatRun = true;
					g_engine->scheduleRun();
				}
				return;
			}

			_pickupFrameCounter++;
			return;
		}

		// We might have gotten the 0x11 command after we stopped moving
		// TODO: Check if the code handles this similarly
		// TODO: Consider which run function to use
		if (_executeScriptOnFinishLerp) {
			_executeScriptOnFinishLerp = false;
			g_engine->_scriptExecutor->_isRepeatRun = true;
			g_engine->scheduleRun();
		}
		return;
	}
	// Bresenham pixel-stepping from walkAlongPath (1008:1b8f).
	// Each frame: calculate walk speed from depth, step that many pixels.
	Common::Point pos = getPosition();
	// Depth-scaled walk speed from walkAlongPath (1008:1b8f):
	// speed = animSpeed * (_walkBaseSpeedPct + depthAtPos) / 100
	int depthAtPos = 0;
	Common::Rect screenRect(320, 200);
	if (screenRect.contains(pos)) {
		depthAtPos = g_engine->_depthMap.getPixel(pos.x, pos.y);
	}
	// Per-animation speed from blob data (runtime+orientation*16+0x30)
	uint16 animSpeed = 2; // default fallback
	uint8 orient = _gameObject->_orientation;
	if (orient >= 1 && orient <= 0x15 && (uint)(orient - 1) < _gameObject->_blobSpeeds.size()) {
		animSpeed = _gameObject->_blobSpeeds[orient - 1];
		if (animSpeed == 0)
			animSpeed = 2;
	}
	int walkSpeed = ((int)animSpeed * ((int)g_engine->_walkBaseSpeedPct + depthAtPos)) / 100;
	if (walkSpeed < 1)
		walkSpeed = 1;

	// Proximity arrival check from walkAlongPath (1008:1b8f):
	// Original checks if character is within walkSpeed pixels of target in both axes.
	// Additionally requires vertical offset to have reached target (binary: runtime+0x21D == object+0x08)
	bool arrived = (abs(pos.x - _endPosition.x) <= walkSpeed) &&
				   (abs(pos.y - _endPosition.y) <= walkSpeed);
	// Binary: arrival also requires vertical offset interpolation to be complete
	if (arrived && _hasMotionVerticalOffset &&
		(int16)_motionTargetVerticalOffset >= 0 &&
		_motionTargetVerticalOffset != _gameObject->_verticalOffsetScale) {
		arrived = false;
	}
	if (arrived) {
		// _snapToTarget (runtime+0x22F): when set, snap character to exact target
		// position on arrival. When clear, leave character at last stepped pixel
		// and update the target to match (original: target = currentPos).
		if (_gameObject->_snapToTarget) {
			pos = _endPosition;
			setPosition(pos);
		} else {
			_endPosition = pos;
			_pathFinalDestination = pos;
			if (_hasMotionVerticalOffset && (int16)_motionTargetVerticalOffset >= 0) {
				_motionTargetVerticalOffset = _gameObject->_verticalOffsetScale;
			}
		}
		if (_isFollowingPath) {
			_isFollowingPath = walkAlongPath();
			// walkAlongPath returning true: more nodes, direction reset, return
			// walkAlongPath returning false: target now = finalDest, keep walking
			return;
		}
		_isLerping = false;
		if (_hasMotionVerticalOffset) {
			_gameObject->_verticalOffsetScale = _motionTargetVerticalOffset;
			_motionProgress = _motionDistanceUnits;
			_hasMotionVerticalOffset = false;
		}
		// Standing orientation = walking direction + 8
		if (_gameObject->_orientation < 9)
			_gameObject->_orientation += 8;
		if (_pickedUpObject != nullptr) {
			_pickupFrameCounter = 0;
			_pickupItemTransferred = false;
			_previousOrientation = _gameObject->_orientation;
			_gameObject->_orientation = 0x11;
			return;
		}
		if (_executeScriptOnFinishLerp) {
			_executeScriptOnFinishLerp = false;
			g_engine->_scriptExecutor->_isRepeatRun = true;
			g_engine->scheduleRun();
		}
		return;
	}

	// Binary: if target==current position, skip Phase 0 turn delay (set directionCalculated=1)
	if (!_stepDirectionSet && _endPosition.x == pos.x && _endPosition.y == pos.y) {
		_stepDirectionSet = true;
	}

	// Calculate direction if not yet set (first frame of movement)
	if (!_stepDirectionSet) {
		_stepDirectionSet = true;
		// Phase 0 from walkAlongPath (1008:1b8f): direction calculation.
		// Binary returns after setting direction (1-frame turn delay).
		uint16 absDx = abs(pos.x - _endPosition.x);
		uint16 absDy = abs(pos.y - _endPosition.y);
		uint8 dir = _gameObject->_orientation;
		if (dir > 8 && dir < 17)
			dir -= 8;
		if (dir > 16)
			dir = 1;
		// Cardinal directions (only if animation available for that direction)
		if (_endPosition.y < pos.y && absDx <= absDy &&
			_gameObject->_blobs.size() > 0 && !_gameObject->_blobs[0].empty())
			dir = 1; // North
		if (pos.x < _endPosition.x && absDy <= absDx &&
			_gameObject->_blobs.size() > 2 && !_gameObject->_blobs[2].empty())
			dir = 3; // East
		if (pos.y < _endPosition.y && absDx <= absDy &&
			_gameObject->_blobs.size() > 4 && !_gameObject->_blobs[4].empty())
			dir = 5; // South
		if (_endPosition.x < pos.x && absDy <= absDx &&
			_gameObject->_blobs.size() > 6 && !_gameObject->_blobs[6].empty())
			dir = 7; // West
		// Diagonals: absDx/4 < absDy AND absDy/2 < absDx
		if ((absDx >> 2) < absDy && (absDy >> 1) < absDx) {
			if (_endPosition.y < pos.y && pos.x < _endPosition.x &&
				_gameObject->_blobs.size() > 1 && !_gameObject->_blobs[1].empty())
				dir = 2; // NE
			if (pos.x < _endPosition.x && pos.y < _endPosition.y &&
				_gameObject->_blobs.size() > 3 && !_gameObject->_blobs[3].empty())
				dir = 4; // SE
			if (pos.y < _endPosition.y && _endPosition.x < pos.x &&
				_gameObject->_blobs.size() > 5 && !_gameObject->_blobs[5].empty())
				dir = 6; // SW
			if (_endPosition.x < pos.x && _endPosition.y < pos.y &&
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

	// Step pixels (Bresenham line algorithm)
	int pixelsMoved = 0;
	Common::Point savedPos = pos; // savedX/savedY from binary
	for (int step = 0; step < walkSpeed; step++) {
		savedPos = pos; // Binary: savedX = posX, savedY = posY at start of each iteration
		if (_stepError < _stepDeltaX) {
			// Step along X axis
			if (_endPosition.x != pos.x)
				pixelsMoved++;
			if (_endPosition.x < pos.x)
				pos.x--;
			else if (_endPosition.x > pos.x)
				pos.x++;
			_stepError += _stepDeltaY;
		} else {
			// Step along Y axis
			if (_endPosition.y != pos.y)
				pixelsMoved++;
			if (_endPosition.y < pos.y)
				pos.y--;
			else if (_endPosition.y > pos.y)
				pos.y++;
			_stepError -= _stepDeltaX;
		}
		// Vertical offset Bresenham interpolation (binary runtime+0x21D/21F/221/223)
		// Per-pixel: accum += stepDelta; while accum >= threshold: accum -= threshold, step ±1
		if (_hasMotionVerticalOffset &&
			((int16)_motionTargetVerticalOffset < 0 || _motionTargetVerticalOffset != _gameObject->_verticalOffsetScale)) {
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
		// Check walkability after each step
		if (!isWalkable(pos)) {
			pos = savedPos;
			// Wall-sliding from walkAlongPath (1008:1b8f):
			// Sample ±1 and ±2 pixels to build a push vector, then slide.
			int pushX = 0, pushY = 0;
			if (lookupWalkability(Common::Point(pos.x + 1, pos.y)) >= 200)
				pushX = -1;
			if (lookupWalkability(Common::Point(pos.x - 1, pos.y)) >= 200)
				pushX += 1;
			if (lookupWalkability(Common::Point(pos.x, pos.y + 1)) >= 200)
				pushY = -1;
			if (lookupWalkability(Common::Point(pos.x, pos.y - 1)) >= 200)
				pushY += 1;
			if (lookupWalkability(Common::Point(pos.x + 2, pos.y)) >= 200)
				pushX -= 1;
			if (lookupWalkability(Common::Point(pos.x - 2, pos.y)) >= 200)
				pushX += 1;
			if (lookupWalkability(Common::Point(pos.x, pos.y + 2)) >= 200)
				pushY -= 1;
			if (lookupWalkability(Common::Point(pos.x, pos.y - 2)) >= 200)
				pushY += 1;
			// Apply push vector pixel by pixel
			while (pushX != 0 || pushY != 0) {
				if (pushX < 0) {
					if (lookupWalkability(Common::Point(pos.x - 1, pos.y)) < 200)
						pos.x--;
					pushX++;
				}
				if (pushX > 0) {
					if (lookupWalkability(Common::Point(pos.x + 1, pos.y)) < 200)
						pos.x++;
					pushX--;
				}
				if (pushY < 0) {
					if (lookupWalkability(Common::Point(pos.x, pos.y - 1)) < 200)
						pos.y--;
					pushY++;
				}
				if (pushY > 0) {
					if (lookupWalkability(Common::Point(pos.x, pos.y + 1)) < 200)
						pos.y++;
					pushY--;
				}
			}
			// Binary: cancel path (target=finalDest=currentPos) but DON'T break — continue loop
			_endPosition = pos;
			_pathFinalDestination = pos;
			_isFollowingPath = false;
			_path.clear();
		}
		// Check if we reached the target
		if (pos == _endPosition)
			break;
	}

	// Binary: if pixelsMoved != walkSpeed after step loop, revert position and cancel path
	if (pixelsMoved != walkSpeed) {
		pos = savedPos;
		_endPosition = pos;
		_pathFinalDestination = pos;
		_isFollowingPath = false;
		_path.clear();
	}

	// Binary: walkSpeed==0 special case - still run vertical offset once
	if (walkSpeed == 0 && _hasMotionVerticalOffset &&
		((int16)_motionTargetVerticalOffset < 0 || _motionTargetVerticalOffset != _gameObject->_verticalOffsetScale)) {
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
	_isMapPanelActive = true;
	_mapPanelSubMode = MapPanelSubMode::None;
	_saveConfirmArmed = false;
	_loadConfirmArmed = false;

	// Load existing save slot names
	for (int idx = 0; idx < 30; idx++) {
		SaveStateDescriptor desc = g_engine->getMetaEngine()->querySaveMetaInfos(
			g_engine->getGameId().c_str(), idx);
		if (desc.getSaveSlot() != -1) {
			_saveSlotNames[idx] = desc.getDescription();
		} else {
			_saveSlotNames[idx] = "";
		}
	}
	redraw();
}

void View1::closeOriginalSaveLoadPanel() {
	_isMapPanelActive = false;
	_mapPanelSubMode = MapPanelSubMode::None;
	redraw();
}

void View1::drawOriginalSaveLoadPanel(Graphics::ManagedSurface &s) {
	if (!_isMapPanelActive)
		return;

	// Panel dimensions matching original: centered, 10 slots of 12px + button bar
	const int panelW = 200;
	const int panelH = 156; // 10*12 + 8 padding + 28 button area
	const int panelX = (320 - panelW) / 2;
	const int panelY = (200 - panelH) / 2;

	// Draw panel background
	drawDarkRectangle(panelX, panelY, panelW, panelH);
	drawBorder(Common::Point(panelX, panelY), Common::Point(panelW, panelH), s);

	// Draw 10 save slots for current page
	for (int slot = 0; slot < 10; slot++) {
		int idx = _mapPanelPageIndex * 10 + slot;
		int slotY = panelY + 4 + slot * 12;
		int slotX = panelX + 4;
		int slotW = panelW - 8;

		// Slot background
		drawDarkRectangle(slotX, slotY, slotW, 12);

		// Slot text
		Common::String label;
		if (_saveSlotNames[idx].empty()) {
			label = Common::String::format("- Slot %d -", idx + 1);
		} else {
			label = _saveSlotNames[idx];
		}
		renderString(slotX + 2, slotY + 2, label);
	}

	// Draw bottom button bar
	int btnY = panelY + 4 + 10 * 12 + 4;
	int btnW = 26;
	int btnH = 16;
	int btnX = panelX + 4;

	const char *btnLabels[] = {"Ld", "Sv", "Mu", "Pg", "OK", "OK", "X"};
	for (int i = 0; i < 7; i++) {
		Common::Point bPos(btnX + i * (btnW + 2), btnY);
		Common::Point bSize(btnW, btnH);

		bool pressed = false;
		if (i == 0 && _mapPanelSubMode == MapPanelSubMode::SaveSlots)
			pressed = true;
		if (i == 1 && _mapPanelSubMode == MapPanelSubMode::MapTravel)
			pressed = true;

		if (pressed) {
			drawPressedBorderOuterHighlights(bPos, bSize, s);
		} else {
			drawBorderOuterHighlights(bPos, bSize, s);
		}
		renderString(bPos.x + 4, bPos.y + 4, btnLabels[i]);
	}

	// Page indicator
	Common::String pageStr = Common::String::format("Page %d/3", _mapPanelPageIndex + 1);
	renderString(panelX + panelW - 60, btnY + 4, pageStr);
}

void View1::handleOriginalSaveLoadClick(const Common::Point &pos) {
	const int panelW = 200;
	const int panelH = 156;
	const int panelX = (320 - panelW) / 2;
	const int panelY = (200 - panelH) / 2;

	// Check if click is outside panel → close
	if (pos.x < panelX || pos.x > panelX + panelW ||
		pos.y < panelY || pos.y > panelY + panelH) {
		closeOriginalSaveLoadPanel();
		return;
	}

	// Check slot clicks
	for (int slot = 0; slot < 10; slot++) {
		int slotY = panelY + 4 + slot * 12;
		int slotX = panelX + 4;
		int slotW = panelW - 8;

		if (pos.x >= slotX && pos.x <= slotX + slotW &&
			pos.y >= slotY && pos.y <= slotY + 12) {
			int idx = _mapPanelPageIndex * 10 + slot;

			if (_mapPanelSubMode == MapPanelSubMode::SaveSlots) {
				// Load from this slot
				if (!_saveSlotNames[idx].empty()) {
					if (_loadConfirmArmed) {
						closeOriginalSaveLoadPanel();
						g_engine->loadGameState(idx);
					} else {
						_loadConfirmArmed = true;
						_saveConfirmArmed = false;
					}
				}
			} else if (_mapPanelSubMode == MapPanelSubMode::MapTravel) {
				// Save to this slot
				if (_saveConfirmArmed) {
					Common::String name = _saveSlotNames[idx].empty()
											  ? Common::String::format("Save %d", idx + 1)
											  : _saveSlotNames[idx];
					closeOriginalSaveLoadPanel();
					g_engine->saveGameState(idx, name);
				} else {
					_saveConfirmArmed = true;
					_loadConfirmArmed = false;
				}
			}
			redraw();
			return;
		}
	}

	// Check button bar clicks
	int btnY = panelY + 4 + 10 * 12 + 4;
	int btnW = 26;
	int btnH = 16;
	int btnX = panelX + 4;

	if (pos.y >= btnY && pos.y <= btnY + btnH) {
		for (int i = 0; i < 7; i++) {
			int bx = btnX + i * (btnW + 2);
			if (pos.x >= bx && pos.x <= bx + btnW) {
				switch (i) {
				case 0: // Load mode
					_mapPanelSubMode = MapPanelSubMode::SaveSlots;
					_saveConfirmArmed = false;
					_loadConfirmArmed = false;
					break;
				case 1: // Save mode
					_mapPanelSubMode = MapPanelSubMode::MapTravel;
					_saveConfirmArmed = false;
					_loadConfirmArmed = false;
					break;
				case 2: // Toggle music
					g_engine->_scriptExecutor->_soundSystemActive =
						!g_engine->_scriptExecutor->_soundSystemActive;
					break;
				case 3: // Page scroll
					_mapPanelPageIndex = (_mapPanelPageIndex + 1) % 3;
					_saveConfirmArmed = false;
					_loadConfirmArmed = false;
					break;
				case 4: // Confirm save (double-click)
					if (_mapPanelSubMode == MapPanelSubMode::MapTravel) {
						_saveConfirmArmed = true;
					}
					break;
				case 5: // Confirm load (double-click)
					if (_mapPanelSubMode == MapPanelSubMode::SaveSlots) {
						_loadConfirmArmed = true;
					}
					break;
				case 6: // Close
					closeOriginalSaveLoadPanel();
					return;
				}
				redraw();
				return;
			}
		}
	}
}

} // namespace Macs2
