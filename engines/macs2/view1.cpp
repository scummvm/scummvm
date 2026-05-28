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
#include "common/debug.h"
#include "common/system.h"
#include "graphics/palette.h"
#include "graphics/paletteman.h"
#include "macs2/gameobjects.h"
#include "macs2/macs2.h"
#include "macs2/adlib.h"
#include <graphics/cursorman.h>
#include <math/angle.h>
#include <math/vector2d.h>

namespace Macs2 {
namespace {
constexpr int kNumLoadedCursors = 5;

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
	debug("%s text at (%d,%d): %s", kind, x, y, text.c_str());
}

void buildFadedPalette(byte *colors, const byte *sourcePalette, int fadeValue) {
	memcpy(colors, sourcePalette, 256 * 3);
	for (uint i = 0; i < 256 * 3; ++i) {
		if (colors[i] < fadeValue) {
			colors[i] = 0;
		} else {
			colors[i] -= fadeValue;
		}
		colors[i] = (colors[i] * 259 + 33) >> 6;
	}
}

void setViewPaletteSafely(View1 *view, const byte *colors) {
	const bool shouldTouchCursor = view != nullptr && !view->isCursorSuppressedForFade();
	const bool cursorWasVisible = shouldTouchCursor && CursorMan.isVisible();
	if (cursorWasVisible)
		CursorMan.showMouse(false);

	if (shouldTouchCursor)
		view->UpdateCursor(colors);

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

void View1::OpenInventory(GameObject *newInventorySource) {
	if (newInventorySource == nullptr) {
		warning("Tried to open inventory for a null source");
		return;
	}

	SetInventorySource(newInventorySource);
	_isShowingInventory = true;
	inventoryPage = 0;
	activeInventoryItem = nullptr;
	g_engine->_scriptExecutor->global1040 = false;
	g_engine->_scriptExecutor->global1042 = false;
	if (g_engine->_scriptExecutor->_mouseMode == Script::MouseMode::UseInventory) {
		g_engine->SetCursorMode(Script::MouseMode::Use);
		UpdateCursor();
	}
}

void View1::CloseInventory() {
	if (!_isShowingInventory) {
		return;
	}

	const bool shouldResumeExternalInventory = !IsInventorySourceProtagonist() && g_engine->_scriptExecutor->hasPendingExternalInventoryResume;
	_isShowingInventory = false;
	inventoryPage = 0;
	activeInventoryItem = nullptr;
	g_engine->_scriptExecutor->global1040 = false;
	g_engine->_scriptExecutor->global1042 = false;

	if (shouldResumeExternalInventory) {
		g_engine->SetCursorMode(g_engine->_scriptExecutor->savedExternalInventoryMouseMode);
		UpdateCursor();
		SetInventorySource(GameObjects::instance().GetProtagonistObject());
		g_engine->_scriptExecutor->hasPendingExternalInventoryResume = false;
		g_engine->_scriptExecutor->externalInventorySourceObjectID = 0;
		g_engine->_scriptExecutor->SetCurrentSceneScriptAt(g_engine->_scriptExecutor->secondaryInventoryLocation);
		g_engine->RunScriptExecutor();
		return;
	}

	if (!IsInventorySourceProtagonist()) {
		SetInventorySource(GameObjects::instance().GetProtagonistObject());
	}
}

void View1::SetInventorySource(GameObject *newInventorySource) {
	inventorySource = newInventorySource;
	// TODO: Make sure the assignment per object is saved correctly
	inventoryItems.clear();

	for (GameObject *currentObject : GameObjects::instance().Objects) {
		if (currentObject->SceneIndex == inventorySource->Index) {
			inventoryItems.push_back(currentObject);
		}
	}
}
bool View1::IsInventorySourceProtagonist() const {
	return inventorySource->Index == 1;
}
void View1::TransferInventoryItem(GameObject *item, GameObject *targetContainer) {
	int index = FindInventoryItem(item);
	inventoryItems.remove_at(index);
	item->SceneIndex = targetContainer->Index;
}

int View1::FindInventoryItem(GameObject *item) {
	for (uint i = 0; i != inventoryItems.size(); i++) {
		if (inventoryItems[i] == item) {
			return i;
		}
	}
	return -1;
}

Character *View1::GetCharacterByIndex(uint16 index) {
	// TODO: Consider a map
	for (Character *c : characters) {
		if (c->GameObject->Index == index) {
			return c;
		}
	}
	return nullptr;
}
void View1::UpdateCursor(const byte *palette) {
	// Handle special cursor modes
	if (g_engine->_scriptExecutor->_mouseMode == Script::MouseMode::Disabled) {
		CursorMan.showMouse(false);
		return;
	}
	CursorMan.showMouse(true);

	// For PanelUse/PanelCursor, fall back to Use cursor visually
	int mode = (int)g_engine->_scriptExecutor->_mouseMode - (int)Script::MouseMode::Talk;
	if (mode >= kNumLoadedCursors) {
		mode = (int)Script::MouseMode::Use - (int)Script::MouseMode::Talk;
	}
	if (mode < 0) {
		warning("Invalid cursor mode %d, falling back to Use cursor", mode);
		mode = (int)Script::MouseMode::Use - (int)Script::MouseMode::Talk;
	}

	if (g_engine->_cursorData[mode] == nullptr || g_engine->_cursorWidths[mode] == 0 || g_engine->_cursorHeights[mode] == 0) {
		warning("Cursor data for mode %d is invalid", mode);
		return;
	}

	const uint16 width = g_engine->_cursorWidths[mode];
	const uint16 height = g_engine->_cursorHeights[mode];
	const byte *cursorData = g_engine->_cursorData[mode];
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
	_backgroundSurface.copyFrom(g_engine->_bgImageShip);
	currentSpeechActData.onRightSide = false;
	UpdateCursor();
	setViewPaletteSafely(this, g_engine->_pal);
	_paletteDirty = false;
	CursorMan.showMouse(true);

	// TODO: Check if this works like this
	Character *protagonist = new Character();
	// TODO: Need to properly handle the offset
	// TODO: Remember that the game starts enumerating objects at 1 and not at 0
	protagonist->GameObject = GameObjects::instance().Objects[0x0];
	characters.push_back(protagonist);
	inventorySource = protagonist->GameObject;

	// inventoryItems.push_back(GameObjects::instance().Objects[0x8 - 1]);

	inventoryButtonLocations.resize(6);
}

AnimFrame *View1::GetInventoryIcon(GameObject *gameObject) {
	AnimFrame *result = new AnimFrame();
	int index = 5 - 1;
	if (is_in_list<uint16, 0x10, 0x11, 0x17, 0x18, 0x1B, 0x22, 0x23, 0x19, 0x1A, 0x14, 0x1C, 0x1D, 0x3C, 0x98>(gameObject->Index)) {
		// gameObject->Index == 0x23 || gameObject->Index == 0x22) {
		// TODO Figure out these - the mug has a different blob
		index = 0x13;
	}
	index = 0x13;
	Common::MemoryReadStream stream(gameObject->Blobs[index].data(), gameObject->Blobs[index].size());
	// TODO: Need to check how the offset really is calculated by the game code, this will not hold
	stream.seek(23, SEEK_SET);

	uint16 offset = Macs2::BackgroundAnimationBlob::Func1480(gameObject->Blobs[index], true, 0);
	offset += 6;
	stream.seek(offset, SEEK_SET);
	offset += 6;
	result->ReadFromStream(&stream);
	return result;
	// TODO: Think about proper memory management
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

void View1::drawStringBackground(uint16 x, uint16 y, uint16 width, uint16 height) {
	Graphics::ManagedSurface s = getSurface();

	// TODO: Look up how we determine the width of the right border
	constexpr int borderWidth = 10;
	constexpr int highlightWidth = 2;
	// Draw the background
	// Draw the border segments
	drawDarkRectangle(x, y, width, height);

	// TODO: Is this the same calculation?
	uint16 xSegments = (width / g_engine->_borderWidth) + 1;
	uint16 ySegments = (height / g_engine->_borderHeight) + 1;

	// First the left side
	Common::Rect clippingRect(x, y, x + borderWidth, y + height);
	int currentX = x;
	int currentY = y;
	for (int iy = 0; iy < ySegments; iy++) {
		// DrawSprite(currentX, currentY, g_engine->_borderWidth, g_engine->_borderHeight, g_engine->_borderData, s);
		DrawSpriteClipped(currentX, currentY, clippingRect, g_engine->_borderWidth, g_engine->_borderHeight, g_engine->_borderData, s);
		currentY += g_engine->_borderHeight;
	}

	// Top
	clippingRect = Common::Rect(x, y, x + width, y + borderWidth);
	currentX = x;
	currentY = y;
	for (int ix = 0; ix < xSegments; ix++) {
		DrawSpriteClipped(currentX, currentY, clippingRect, g_engine->_borderWidth, g_engine->_borderHeight, g_engine->_borderData, s);
		currentX += g_engine->_borderWidth;
	}

	// Right
	// TODO: Need to figure out the margin here
	currentX = x + width - borderWidth;
	currentY = y;
	clippingRect = Common::Rect(currentX, y, x + width, y + height);
	for (int iy = 0; iy < ySegments; iy++) {
		DrawSpriteClipped(currentX, currentY, clippingRect, g_engine->_borderWidth, g_engine->_borderHeight, g_engine->_borderData, s);
		currentY += g_engine->_borderHeight;
	}

	// Bottom
	currentX = x;
	currentY = y + height - borderWidth;
	clippingRect = Common::Rect(x, currentY, x + width, y + height);
	for (int ix = 0; ix < xSegments; ix++) {
		DrawSpriteClipped(currentX, currentY, clippingRect, g_engine->_borderWidth, g_engine->_borderHeight, g_engine->_borderData, s);
		currentX += g_engine->_borderWidth;
	}

	// Highlight on the top and left
	// TODO: Check if it's also done to the inside of the frame
	// TODO: Refactor code to have less copy paste
	// First the left side
	clippingRect = Common::Rect(x, y, x + highlightWidth, y + height);
	currentX = x;
	currentY = y;
	for (int iy = 0; iy < ySegments; iy++) {
		// DrawSprite(currentX, currentY, g_engine->_borderWidth, g_engine->_borderHeight, g_engine->_borderData, s);
		DrawSpriteClipped(currentX, currentY, clippingRect, g_engine->_borderWidth, g_engine->_borderHeight, g_engine->_borderHighlightData, s);
		currentY += g_engine->_borderHeight;
	}

	// Top
	clippingRect = Common::Rect(x, y, x + width, y + highlightWidth);
	currentX = x;
	currentY = y;
	for (int ix = 0; ix < xSegments; ix++) {
		DrawSpriteClipped(currentX, currentY, clippingRect, g_engine->_borderWidth, g_engine->_borderHeight, g_engine->_borderHighlightData, s);
		currentX += g_engine->_borderWidth;
	}
}

void View1::drawBackgroundAnimations(Graphics::ManagedSurface &s) {
	for (int i = 0; i < g_engine->_numBackgroundAnimations; i++) {
		/*  if (i != 7) {
			continue;
		} */
		BackgroundAnimation &current = g_engine->_backgroundAnimations[i];
		BackgroundAnimationBlob &currentBlob = g_engine->_backgroundAnimationsBlobs[i];
		// AnimFrame &currentFrame = current.Frames[current.FrameIndex];
		// AnimFrame currentFrame = currentBlob.GetFrame(currentBlob.FrameIndex);
		AnimFrame currentFrame = currentBlob.GetCurrentFrame();
		DrawSprite(current.X, current.Y, currentFrame.Width, currentFrame.Height, currentFrame.Data, s, false);
	}
}

void View1::drawBackgroundAnimationNumbers(Graphics::ManagedSurface &s) {
	for (int i = 0; i < g_engine->_numBackgroundAnimations; i++) {
		BackgroundAnimation &current = g_engine->_backgroundAnimations[i];
		renderString(current.X, current.Y, Common::String::format("%u", i));
	}
}

void View1::drawCurrentSpeaker(Graphics::ManagedSurface &s) {
	// TODO: Draw the border

	AnimFrame *frame = currentSpeechActData.speaker->GetCurrentPortrait(currentSpeechActData.onRightSide);
	AnimFrame *leftPortrait = currentSpeechActData.speaker->GetCurrentPortrait(false);
	AnimFrame *rightPortrait = currentSpeechActData.speaker->GetCurrentPortrait(true);
	if (frame == nullptr) {
		return;
	}

	// See l0037_B462: for the calculations below
	// Draw the border
	const int portraitWidth = MAX<int>(leftPortrait ? leftPortrait->Width : 0, rightPortrait ? rightPortrait->Width : 0);
	const int portraitHeight = MAX<int>(leftPortrait ? leftPortrait->Height : 0, rightPortrait ? rightPortrait->Height : 0);
	const Common::Point borderSize(portraitWidth + 0xD, portraitHeight + 0xD);
	DrawBorder(currentSpeechActData.position, borderSize, s);

	// Draw the portrait over the border
	Common::Point pos = currentSpeechActData.position + Common::Point(7, 7);
	DrawSprite(pos, frame->Width, frame->Height, frame->Data, s, false);
}

void View1::renderString(uint16 x, uint16 y, Common::String s) {
	Graphics::ManagedSurface surf = getSurface();
	uint16 currentX = x;
	uint16 currentY = y;
	for (auto iter = s.begin(); iter != s.end(); iter++) {
		GlyphData data;
		bool found = g_engine->FindGlyph(*iter, data);
		if (found) {
			DrawSprite(currentX, currentY, data.Width, data.Height, data.Data, surf, false);
			currentX += data.Width + 1;
			// TODO: Add reference to where this is defined
		} else {
			if ((byte)*iter != ' ') {
				warning("Missing glyph for character 0x%02x while rendering \"%s\" at (%u,%u)", (byte)*iter, s.c_str(), x, y);
			}
			// TODO: Different character for not found?
			currentX += 10;
		}
	}
}

void View1::renderString(const Common::Point pos, const Common::String &s) {
	renderString(pos.x, pos.y, s);
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
		if (entry.alignment == 1) {
			x -= g_engine->MeasureString(text);
		} else if (entry.alignment == 2) {
			x -= g_engine->MeasureString(text) / 2;
		}

		if (x < 0)
			x = 0;

		logRenderedText("Overlay", x, entry.position.y, text);
		renderString(x, entry.position.y, text);
	}
}

void View1::showStringBox(const Common::StringArray &sa) {
	// This calculation can be found at l0037_B368:
	// int borderWidth = 10;
	// int padding = 3;
	int totalWidth = g_engine->MeasureStrings(sa) + 0x12;
	int totalHeight = g_engine->MeasureStringsVertically(sa) + 0x10;
	debug("Render text box: lines=%u borderPos=(%d,%d) borderSize=(%d,%d) text=\"%s\"",
		  sa.size(), stringBoxPosition.x, stringBoxPosition.y, totalWidth, totalHeight, joinDebugStrings(sa).c_str());
	debugC(kDebugScript,
		   "Render text box: lines=%u borderPos=(%d,%d) borderSize=(%d,%d) text=\"%s\"",
		   sa.size(), stringBoxPosition.x, stringBoxPosition.y, totalWidth, totalHeight, joinDebugStrings(sa).c_str());

	// drawStringBackground(x, y, totalWidth, totalHeight);
	Graphics::ManagedSurface s = getSurface();
	DrawBorder(stringBoxPosition, Common::Point(totalWidth, totalHeight), s);
	// TODO range based
	int lineOffset = stringBoxPosition.y + 0x9;
	for (auto iter = sa.begin(); iter < sa.end(); iter++) {
		logRenderedText("TextBox", stringBoxPosition.x + 0x9, lineOffset, *iter);
		renderString(stringBoxPosition.x + 0x9, lineOffset, *iter);
		lineOffset += g_engine->maxGlyphHeight + 2;
	}
}

void View1::drawGlyphs(Macs2::GlyphData *data, int count, uint16 x, uint16 y, Graphics::ManagedSurface &s) {
	uint16 currentX = x;
	uint16 currentY = y;
	for (int i = 0; i < count; i++) {
		const Macs2::GlyphData &currentData = data[i];
		if (currentX + currentData.Width > s.w) {
			currentY += currentData.Height;
			currentX = x;
		}
		DrawSprite(currentX, currentY, currentData.Width, currentData.Height, currentData.Data, s, false);
		currentX += currentData.Width;
	}
}

void View1::handleFading() {
	if (fadeMode == FadeMode::None) {
		return;
	}

	if (fadeMode == FadeMode::FromBlack) {
		currentFadeValue -= fadeDelta;
		if (currentFadeValue <= 0) {
			currentFadeValue = -1;
			fadeMode = FadeMode::None;
			setViewPaletteSafely(this, g_engine->_pal);
			endFadeCursorSuppression(g_engine->_pal);
			_paletteDirty = false;
			return;
		}
	} else {
		currentFadeValue += fadeDelta;
		if (currentFadeValue >= 0x40) {
			byte colors[256 * 3];
			currentFadeValue = 0x40;
			fadeMode = FadeMode::None;
			buildFadedPalette(colors, g_engine->_palVanilla, currentFadeValue);
			setViewPaletteSafely(this, colors);
			endFadeCursorSuppression(colors);
			return;
		}
	}

	applyPaletteWithFade(this, g_engine->_palVanilla, currentFadeValue);
}

void View1::drawPathfindingPoints(Graphics::ManagedSurface &s) {

	constexpr bool drawNodes = false;
	if (drawNodes) {

		GlyphData xData;
		g_engine->FindGlyph('x', xData);
		int numLines = 0;
		for (int i = 0; i < 16; i++) {
			PathfindingPoint &current = g_engine->pathfindingPoints[i];
			renderString(current.Position.x - xData.Width * 0.5, current.Position.y - xData.Height * 0.5, "x");

			Common::String number = Common::String::format("%u", i);
			renderString(current.Position.x - xData.Width * 0.5 + 10, current.Position.y - xData.Height * 0.5 + 10, number.c_str());

			for (uint8 adjacentIndex : current.adjacentPoints) {
				if (adjacentIndex >= g_engine->pathfindingPoints.size()) {
					continue;
				}
				PathfindingPoint &other = g_engine->pathfindingPoints[adjacentIndex - 1];
				s.drawLine(current.Position.x, current.Position.y, other.Position.x, other.Position.y, 0xFFFFFFFF);
				numLines++;
			}
		}
	}

	// Draw the test results
	Macs2::Character *c = GetCharacterByIndex(1);
	// Handle the protagonist not being in the scene
	if (c == nullptr) {
		return;
	}
	Common::Array<uint8> &overlay = c->PathfindingOverlay;
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
	for (const Common::String &current : g_engine->debugOutput) {
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

	isShowingMainMenu = true;
	// Let's prototype the icon size first
	Common::Point iconMaxSize(20, 20);
	iconMaxSize += Common::Point(6, 6);
	Common::Point inventorySize(iconMaxSize.x * 3 + 0x10, iconMaxSize.y * 3 + 0x10);
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
	mainMenuRect = Common::Rect(upperLeft, lowerRight);
	assert(mainMenuRect.width() == inventorySize.x && mainMenuRect.height() == inventorySize.y);
	isShowingMainMenu = true;
}

void View1::drawMainMenu(Graphics::ManagedSurface &s) {
	DrawBorder(Common::Point(mainMenuRect.left, mainMenuRect.top), Common::Point(mainMenuRect.width(), mainMenuRect.height()), s);
	uint16 currentX = mainMenuRect.left;
	uint16 currentY = mainMenuRect.top;
	mainMenuButtonLocations.resize(9);
	for (int i = 0; i < 9; i++) {
		AnimFrame &currentFrame = g_engine->imageResources[i];
		DrawSprite(currentX, currentY, currentFrame.Width, currentFrame.Height, currentFrame.Data, s, false);
		mainMenuButtonLocations[i] = Common::Rect(Common::Point(currentX, currentY), currentFrame.Width, currentFrame.Height);
		currentX += currentFrame.Width + 4;
		if (i > 0 && i % 3 == 0) {
			currentX = mainMenuRect.left;
			currentY += currentFrame.Height;
		}
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
	stringBoxPosition = pos;
	setStringBox(sa);
}

void View1::clearStringBox(bool continueScript) {
	_isShowingStringBox = false;
	_isShowingDialogueChoice = false;
	_dialogueChoiceCount = 0;
	currentSpeechActData.speaker = nullptr;
	redraw();
	if (continueScript && _continueScriptAfterUI) {
		_continueScriptAfterUI = false;
		// TODO: Check which one it should be
		g_engine->RunScriptExecutor(false);
	} else if (!continueScript) {
		_continueScriptAfterUI = false;
	}
}

int View1::GetCharacterArrayIndex(const Character *c) const {
	// TODO: Check if there is a find function somewhere
	for (uint i = 0; i < characters.size(); i++) {
		if (characters[i] == c) {
			return i;
		}
	}
	return -1;
}

bool View1::HasDuplicateCharacters() const {
	Common::Array<uint16> uniqueIDs;
	for (Macs2::Character *current : characters) {
		for (uint16 currentID : uniqueIDs) {
			if (currentID == current->GameObject->Index) {
				return true;
			}
		}
		uniqueIDs.push_back(current->GameObject->Index);
	}
	return false;
}

void View1::startFading() {
	beginFadeCursorSuppression();
	currentFadeValue = 0x40;
	fadeMode = FadeMode::FromBlack;
}

void View1::startFadeToBlack() {
	beginFadeCursorSuppression();
	currentFadeValue = 0;
	fadeMode = FadeMode::ToBlack;
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
	UpdateCursor(palette);
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
		// Handle string boxes
		if (_isShowingStringBox) {
			clearStringBox();
			return true;
		}

		if (_isShowingInventory) {

			for (int i = 0; i < 6; i++) {
				const Common::Rect &current = inventoryButtonLocations[i];
				if (current.contains(msg._pos)) {
					switch (i) {
					case static_cast<int>(InventoryButtonIndex::Look): {
						g_engine->SetCursorMode(Script::MouseMode::Look);
						UpdateCursor();
					} break;
					case static_cast<int>(InventoryButtonIndex::Hand): {
						g_engine->SetCursorMode(Script::MouseMode::Use);
						UpdateCursor();
					} break;
					case static_cast<int>(InventoryButtonIndex::Up): {
						if (inventoryPage > 0) {
							inventoryPage--;
						}
					} break;
					case static_cast<int>(InventoryButtonIndex::Down): {
						// Check how many pages we have
						uint16 numPages = (uint16)ceil((double)inventoryItems.size() / 5.0);
						if (inventoryPage < numPages - 2) {
							inventoryPage++;
						}

					} break;
					case static_cast<int>(InventoryButtonIndex::Close): {
						CloseInventory();
						return true;
					}
					}
				}
			}

			// Check if we hit an item
			// TODO: Skipping this for now while we only have one item
			GameObject *clickedObject = getClickedInventoryItem2(msg._pos);
			// TODO: Reminder that we need to highlight the clicked button for a moment

			// TODO: Maybe handled better elsewhere - examining inventory items
			if (clickedObject != nullptr && g_engine->_scriptExecutor->_mouseMode == Script::MouseMode::Look) {
				// TODO: Does the scripting engine expect always the objects with the
				// right number prefix like here 419 instead of 19?
				g_engine->_scriptExecutor->_interactedObjectID = 0x400 + clickedObject->Index;
				g_engine->_scriptExecutor->_interactedOtherObjectID = 0;
				g_engine->RunScriptExecutor(false);
				return true;
			}
			if (clickedObject != nullptr && g_engine->_scriptExecutor->_mouseMode == Script::MouseMode::Use) {
				activeInventoryItem = clickedObject;
				AnimFrame *icon = GetInventoryIcon(activeInventoryItem);
				CursorMan.replaceCursor((void *)icon->Data, icon->Width, icon->Height, icon->Width / 2, icon->Height / 2, 0);
				g_engine->_scriptExecutor->_mouseMode = Script::MouseMode::UseInventory;
				return true;
			}
			if (activeInventoryItem != nullptr && clickedObject != nullptr) {
				// Trigger a use item on item
				GameObject *firstObject = activeInventoryItem;
				// TODO: We should check if the active object still exists afterwards and only
				// then deactivate it
				// TODO: Does the scripting engine expect always the objects with the
				// right number prefix like here 419 instead of 19?
				g_engine->_scriptExecutor->_interactedObjectID = 0x400 + firstObject->Index;
				g_engine->_scriptExecutor->_interactedOtherObjectID = 0x400 + clickedObject->Index;
				g_engine->_scriptExecutor->global1042 = true;
				g_engine->RunScriptExecutor(false);
			}

			return true;
		}

		if (isShowingMainMenu) {
			for (int i = 0; i < 9; i++) {
				const Common::Rect &current = mainMenuButtonLocations[i];
				if (current.contains(msg._pos)) {
					switch (i) {
					case static_cast<int>(MainMenuButtonIndex::Talk): {
						g_engine->SetCursorMode(Script::MouseMode::Talk);
						isShowingMainMenu = false;
					} break;
					case static_cast<int>(MainMenuButtonIndex::Look): {
						g_engine->SetCursorMode(Script::MouseMode::Look);
						isShowingMainMenu = false;
					} break;
					case static_cast<int>(MainMenuButtonIndex::Use): {
						g_engine->SetCursorMode(Script::MouseMode::Use);
						isShowingMainMenu = false;
					} break;
					case static_cast<int>(MainMenuButtonIndex::Walk): {
						g_engine->SetCursorMode(Script::MouseMode::Walk);
						isShowingMainMenu = false;
					} break;
					case static_cast<int>(MainMenuButtonIndex::Inventory): {
						isShowingMainMenu = false;
						OpenInventory(GameObjects::instance().GetProtagonistObject());
					} break;

					case static_cast<int>(MainMenuButtonIndex::Close): {
						isShowingMainMenu = false;
					} break;
					}
				}
			}
			UpdateCursor();
		}

		// Handle no other interactions during a script
		if (g_engine->_scriptExecutor->IsExecuting()) {
			return true;
		}

		// uint32 value = getSurface().getPixel(msg._pos.x, msg._pos.y);
		// uint32 value = g_engine->_map.getPixel(msg._pos.x, msg._pos.y);
		// g_system->setWindowCaption(Common::String::format("%u,%u: %u", msg._pos.x, msg._pos.y, value));
		// g_engine->CalculatePath(Common::Point(154, 136), Common::Point(msg._pos.x, msg._pos.y));

		if (g_engine->_scriptExecutor->_mouseMode == Script::MouseMode::Walk) {
			// TODO: Should address the protagonist differently
			// TODO: Sort out the different modes and only define them once
			Character *protagonist = GetCharacterByIndex(1);
			if (protagonist == nullptr) {
				warning("Ignoring walk click without protagonist character in the active scene");
				return true;
			}

			protagonist->StartLerpTo(msg._pos, 1000);
			return true;
		}

		// Check if we hit something
		uint16 index = GetHitObjectID(Common::Point(msg._pos.x, msg._pos.y));
		if (index == 0) {
			// TODO: Check which we should test first in practice, objects or background
			index = g_engine->GetInteractedBackgroundHotspot(msg._pos);
		}
		if (index != 0) {
			debug("*** New interaction started");
			g_engine->_scriptExecutor->_interactedObjectID = index;
			g_engine->_scriptExecutor->_interactedOtherObjectID = activeInventoryItem != nullptr ? activeInventoryItem->Index + 0x0400 : 0x0000;

			// TODO: We need to keep better track of whether the inventory item
			// is actually gone, resetting for now like this
			activeInventoryItem = nullptr;

			// Set the script
			g_engine->_scriptExecutor->SetScript(Scenes::instance().CurrentSceneScript);
			// TODO: Not sure where the original code rewinds the script
			Scenes::instance().CurrentSceneScript->seek(0, SEEK_SET);
			g_engine->RunScriptExecutor(false);
			// TODO: For the case of clicking an object, this reset happens at l0037_EFD3:
			// Not sure where and if it happens for an inventory interaction
			g_engine->_scriptExecutor->_interactedObjectID = 0;
			g_engine->_scriptExecutor->_interactedOtherObjectID = 0;
		}
		return true;
	} else if (msg._button == MouseMessage::MB_RIGHT) {
		// Handle no other interactions during a script
		if (g_engine->_scriptExecutor->IsExecuting()) {
			// From handleInput: right-click during script execution opens the
			// map/save panel if no UI is blocking.
			if (!_isShowingStringBox && !_isShowingDialogueChoice &&
				!g_engine->_scriptExecutor->overlayTextStageActive &&
				!g_engine->_scriptExecutor->waitForSoundPlayback &&
				!g_engine->_scriptExecutor->waitForMusicControl &&
				!g_engine->_scriptExecutor->waitForAdlibReady) {
				// Toggle the save/load panel (ScummVM uses native save dialog)
				// TODO: Implement the original's 10-slot save/load panel
				// For now, open ScummVM's save/load dialog
			}
			return true;
		}

		// From handleInput (1008:e8bf): right-click when not executing
		// opens the action bar at the mouse position.
		if (!isShowingMainMenu) {
			openMainMenu(msg._pos);
		} else {
			isShowingMainMenu = false;
		}
		UpdateCursor();
		return true;
	}
	return false;
}

bool View1::msgMouseMove(const MouseMoveMessage &msg) {
	// TODO: Check what we are hovering over and save this info
	// uint16 areaID = g_engine->_scriptExecutor->Func101D(msg._pos.x, msg._pos.y);
	// g_system->setWindowCaption(Common::String::format("Area ID: %.4x", areaID));
	return true;
}

bool View1::msgKeypress(const KeypressMessage &msg) {
	// Any keypress to close the view
	// close();
	if (_isShowingStringBox && !_isShowingDialogueChoice) {
		clearStringBox();
		return true;
	}

	if (msg.ascii == (uint16)'t') {
		if (_isShowingInventory && activeInventoryItem != nullptr) {
			if (inventorySource->Index == 1) {
				// TODO: Need to handle this case, the game can figure out that there is a container
				// in the current room as seen in room 3 of the boat
			} else {
				TransferInventoryItem(activeInventoryItem, GameObjects::instance().GetProtagonistObject());
				activeInventoryItem = nullptr;
			}
		}
	}
	if (msg.ascii == (uint16)'c') {
		g_engine->changeScene(0x6);
	}
	if (msg.ascii == (uint16)'d') {
		_backgroundSurface.copyFrom(g_engine->_depthMap);
		redraw();
	}
	if (msg.ascii == (uint16)'m') {
		// _backgroundSurface = g_engine->_map;
		_backgroundSurface.copyFrom(g_engine->_pathfindingMap);
		redraw();
	} else if (msg.ascii == (uint16)'x') {
		CalculateCharacterScaling(0x79);
	} else if (msg.ascii == (uint16)'b') {
		_backgroundSurface.copyFrom(g_engine->_bgImageShip);
		startFading();
		redraw();
	} else if (msg.ascii == (uint16)'s') {
		// g_engine->ExecuteScript(g_engine->_scriptStream);
		g_engine->RunScriptExecutor(true);
		// Also test the lerping
		GetCharacterByIndex(1)->StartLerpTo(Common::Point(200, 100), 5000);
	} else if (msg.ascii == (uint16)'i') {
		if (!_isShowingInventory) {
			OpenInventory(GameObjects::instance().GetProtagonistObject());
		} else {
			CloseInventory();
		}
	} else if (msg.ascii >= '1' && msg.ascii <= '9') {
		// Register a dialogue choice and act upon it
		uint8 numberPressed = msg.ascii - '1' + 1;
		TriggerDialogueChoice(numberPressed);
	} else if (msg.ascii == 'p') {

		/*  characters[0]->IsFollowingPath = true;
		characters[0]->CurrentPathIndex = -1;
		characters[0]->Path.clear();
		characters[0]->Path.push_back(8);
		characters[0]->Path.push_back(11);
		characters[0]->Path.push_back(9); */
		const Common::Point mousePos = g_system->getEventManager()->getMousePos();
		GetCharacterByIndex(1)->PathFinalDestination = mousePos;
		GetCharacterByIndex(1)->Path.clear();
		// g_engine->_path.clear();
		bool pathfindingResult = characters[0]->FindPath(mousePos);
		GetCharacterByIndex(1)->IsFollowingPath = pathfindingResult;
		GetCharacterByIndex(1)->CurrentPathIndex = -1;

	} else if (msg.ascii == 'n') {
		Common::Point mousePos = g_system->getEventManager()->getMousePos();
		openMainMenu(mousePos);
	}

	return true;
}

void View1::draw() {
	if (_paletteDirty && currentFadeValue < 0) {
		setViewPaletteSafely(this, g_engine->_pal);
		_paletteDirty = false;
	}

	handleFading();

	Graphics::ManagedSurface s = getSurface();

	s.blitFrom(_backgroundSurface);
	// Handle highlighting

	/*
	for (int x = 0; x < s.w; x++) {
		for (int y = 0; y < s.h; y++) {
			if (g_engine->_map.getPixel(x, y) == 0x2) {
				s.setPixel(x, y, 0xFF);
			}
		}
	}
	*/

	drawBackgroundAnimations(s);
	DrawCharacters(s);
	drawOverlayTextEntries();

	// Draw the character

	// uint16 charX = 50;
	// uint16 charY = 100;
	// TODO: I don't have the right offset yet plus there must be some trick to reading sequential frames, probl. need
	// to seek in between frames
	// AnimFrame &f = g_engine->_animFrames[_guyFrameIndex];
	// DrawSprite(charX, charY, f.Width, f.Height, f.Data, s);
	// DrawSpriteAdvanced(charX, charY, f.Width, f.Height, 26, f.Data, s);
	/* for (int x = 0; x < g_engine->_charWidth; x++) {
		for (int y = 0; y < g_engine->_charHeight; y++) {
			uint8 val = g_engine->_charData[y * g_engine->_charWidth + x];
			if (val != 0) {
				s.setPixel(charX + x, charY + y, val);
			}
		}
	} */

	// Draw the border part
	/* uint16 borderX = 100;
	uint16 borderY = 50;
	for (int x = 0; x < g_engine->_borderWidth; x++) {
		for (int y = 0; y < g_engine->_borderHeight; y++) {
			uint8 val = g_engine->_borderData[y * g_engine->_borderWidth + x];
			if (val != 0) {
				s.setPixel(borderX + x, borderY + y, val);
			}
		}
	} */

	// And the highlight part
	/* borderX = 150;
	borderY = 100;
	for (int x = 0; x < g_engine->_borderHighlightWidth; x++) {
		for (int y = 0; y < g_engine->_borderHighlightHeight; y++) {
			uint8 val = g_engine->_borderHighlightData[y * g_engine->_borderHighlightWidth + x];
			if (val != 0) {
				s.setPixel(borderX + x, borderY + y, val);
			}
		}
	}
	*/

	// DrawSprite(200, 100, g_engine->_flagWidths[1], g_engine->_flagHeights[1], g_engine->_flagData[1], s);
	// DrawSprite(200, 150, g_engine->_flagWidths[2], g_engine->_flagHeights[2], g_engine->_flagData[2], s);

	// Draw the mouse cursor
	// DrawSprite(100, 100, g_engine->_cursorWidth, g_engine->_cursorHeight, g_engine->_cursorData, s);

	// Draw the animation frame
	// DrawSprite(180, 80, g_engine->_guyWidth, g_engine->_guyHeight, g_engine->_guyData, s);

	// for (int i = 0; i < 100; ++i)
	//	s.frameRect(Common::Rect(i, i, 320 - i, 200 - i), i);

	// Draw a shaded rectangle
	// drawDarkRectangle(50, 50, 100, 50);
	// drawStringBackground(50, 50, 100, 50);
	if (_isShowingStringBox) {
		showStringBox(_drawnStringBox);
		if (currentSpeechActData.speaker != nullptr) {
			// TODO: Improve addressing of the memory
			drawCurrentSpeaker(s);
		}
	}

	// Draw all glyphs
	// drawGlyphs(g_engine->_glyphs, g_engine->numGlyphs, 10, 10, s);

	// DrawSprite(108, 14, g_engine->_flagWidths[_flagFrameIndex], g_engine->_flagHeights[_flagFrameIndex], g_engine->_flagData[_flagFrameIndex], s);
	;
	// renderString(200, 100, "Hello, world!");

	// DrawSprite(100, 100, g_engine->_stick.Width, g_engine->_stick.Height, g_engine->_stick.Data, s);

	// We keep the inventory on but don't draw it in case we display a string
	// i.e. a description of an item
	if (_isShowingInventory && !_isShowingStringBox) {
		// drawInventory(s);
		drawInventory2(s);
	}

	if (isShowingMainMenu) {
		drawMainMenu(s);
	}

	if (activeInventoryItem != nullptr) {
		AnimFrame *icon = GetInventoryIcon(activeInventoryItem);
		DrawSprite(0x00, 0x00, icon->Width, icon->Height, icon->Data, s, false);
	}

	drawPathfindingPoints(s);
	drawPath(s);
	// drawBackgroundAnimationNumbers(s);
	drawDebugOutput(s);

	// Get mouse position
	Common::Point mousePos = g_system->getEventManager()->getMousePos();

	if (_isShowingInventory) {
		// Show the ID of the hovered item
		GameObject *hoveredObject = getClickedInventoryItem2(mousePos);
		if (hoveredObject != nullptr) {
			Common::String name = GameObjects::instance().ObjectNames[hoveredObject->Index];
			if (!name.empty()) {
				renderString(mousePos.x + 20, mousePos.y + 20, name);
			} else {
				renderString(mousePos.x + 20, mousePos.y + 20, Common::String::format("%2.x", hoveredObject->Index));
			}
		}
	} else {
		// Draw the position next to it
		renderString(mousePos.x + 20, mousePos.y + 20, Common::String::format("%u %u", mousePos.x, mousePos.y));
	}

	// Render the scaling factors
	renderString(0, 0, Common::String::format("%u %u", scalingValues.characterY, scalingValues.scalingFactor));

	// DrawImageResources(s);
}

bool View1::tick() {
	// TODO: Check if this pattern works or it would be better different
	// TODO: Check if loading also works with this pattern
	if (!started) {
		g_engine->changeScene(Scenes::instance().CurrentSceneIndex);
		started = true;
	}
	// Cycle the palette
	++_offset;
	// for (int i = 0; i < 256; ++i)
	//	_pal[i * 3 + 1] = (i + _offset) % 256;
	//  g_system->getPaletteManager()->setPalette(_pal, 0, 256);

	// Music fade tick from gameTick (1008:e556).
	// Processes volume fade in/out each frame when active.
	Script::ScriptExecutor *se = g_engine->_scriptExecutor;
	if (se->activeMusicSlot != 0 && se->musicControlMode != 0) {
		if (se->musicControlMode == 1) {
			// Fade out: volume -= step
			int vol = (int)se->musicControlVolume - (int)se->musicControlParam;
			if (vol < 1) {
				se->musicControlMode = 0;
				se->musicControlVolume = 0;
				vol = 0;
			} else {
				se->musicControlVolume = vol;
			}
			g_engine->getAdlib()->SetVolume(se->musicControlVolume);
		} else {
			// Fade in: volume += step. When >= 63: stop music.
			int vol = (int)se->musicControlVolume + (int)se->musicControlParam;
			if (vol >= 0x3F) {
				se->musicControlMode = 0;
				se->activeMusicSlot = 0;
				g_engine->getAdlib()->StopMusic();
			} else {
				se->musicControlVolume = vol;
				g_engine->getAdlib()->SetVolume(se->musicControlVolume);
			}
		}
	}

	// Below is redundant since we're only cycling the palette, but it demonstrates
	// how to trigger the view to do further draws after the first time, since views
	// don't automatically keep redrawing unless you tell it to
	// if ((_offset % 256) == 0)
	//	redraw();

	// Update the flag
	// TODO: Think about all these and compare other implementations, e.g. if we should rather update anims in draw
	// TODO: Consider wraparout
	uint32 tick_time = g_events->currentMillis;
	uint32 delta = tick_time - _lastMillis;
	_nextFrameFlag -= delta;

	// Background animation advance - original gameTick uses a tick counter
	// that resets when exceeding the mode-dependent threshold (mode 2: 39 ticks,
	// mode 3: scene-specific value from word5205).
	if (_nextFrameFlag <= 0) {
		_flagFrameIndex++;
		if (_flagFrameIndex == 3) {
			_flagFrameIndex = 0;
		}
		// TODO: Handle cleaner
		_nextFrameFlag = _frameDelayFlag;
		// TODO: Check if this is necessary

		// Proper update of the background anims
		// TODO: Hardcoding start to 2 to have the manually flipped animations not change automatically
		for (int i = 2; i < g_engine->_numBackgroundAnimations; i++) {
			BackgroundAnimation &current = g_engine->_backgroundAnimations[i];
			current.FrameIndex++;
			current.FrameIndex = current.FrameIndex % current.numFrames;
		}

		// TODO: Piggybacking the guy on this
		_guyFrameIndex++;
		_guyFrameIndex = _guyFrameIndex % 6;

		// And the animations overall
		for (Character *currentCharacter : characters) {
			currentCharacter->animationIndex++;
		}
		redraw();
	}

	_lastMillis = tick_time;

	int i = 0;
	for (auto currentCharacter : characters) {
		currentCharacter->Update();
		i++;
	}

	return true;
}

void View1::drawInventory(Graphics::ManagedSurface &s) {
	Common::Rect inventoryRect(0x36, 0x2C, 0x10A, 0x82);
	drawDarkRectangle(0x36, 0x2c, 0x10A - 0x36, 0x82 - 0x2c);
	// TODO: Add proper grid, add y as well
	int x = 0;
	int y = 0;
	int rowHeight = 0;
	for (GameObject *currentItem : inventoryItems) {
		AnimFrame *icon = GetInventoryIcon(currentItem);
		DrawSprite(0x36 + x, 0x2c + y, icon->Width, icon->Height, icon->Data, s, false);
		x += icon->Width;
		rowHeight = MAX<int>(icon->Height, rowHeight);
		if (x > inventoryRect.width()) {
			x = 0;
			y += rowHeight;
		}
	}
}

void View1::drawInventory2(Graphics::ManagedSurface &s) {
	// First, draw the whole background
	// Happens around l0037_47A1:

	uint16 maxWidthButtonIcon = 0;  // [0FE0h]
	uint16 maxHeightButtonIcon = 0; // [0FE2h]
	for (uint16 index : g_engine->inventoryIconIndices) {
		AnimFrame &currentFrame = g_engine->imageResources[index - 1];
		maxWidthButtonIcon = MAX(maxWidthButtonIcon, currentFrame.Width);
		// TODO: Not sure if this one is needed
		maxHeightButtonIcon = MAX(maxHeightButtonIcon, currentFrame.Height);
	}

	uint16 maxWidthInventoryIcon = 0x20;  // [0FDCh]
	uint16 maxHeightInventoryIcon = 0x20; // [0FDEh]

	for (GameObject *currentInventoryObject : inventoryItems) {
		AnimFrame *icon = GetInventoryIcon(currentInventoryObject);
		if (icon->Width < 250) {
			// TODO: One of the items taken in chapter 4 during one of the first three
			// screens seems to have a faulty width which
			// throws off the calculation
			maxWidthInventoryIcon = MAX(maxWidthInventoryIcon, icon->Width);
		}

		// TODO: Not sure if this one is needed
		maxHeightInventoryIcon = MAX(maxHeightInventoryIcon, icon->Height);
	}

	// TODO: Verify these in emulator
	uint16 widthCandidate1 = (maxWidthButtonIcon + 4) * 6 + 4;
	uint16 widthCandidate2 = (maxWidthInventoryIcon + 6 + 4) * 5 + 0xC;
	uint16 width = MAX(widthCandidate1, widthCandidate2); // [0FD8h]

	// Height calculation
	uint16 height = (maxHeightInventoryIcon + 6 + 4) * 2 + maxHeightButtonIcon + 0x6 + 0x10; // [0FDAh]

	// Position calculation - TODO: Proper position
	uint16 x = s.w / 2 - width / 2;  // [0FD4h]
	uint16 y = s.h / 2 - height / 2; // [0FD6h]

	Graphics::ManagedSurface *buffer = new Graphics::ManagedSurface(s.w, s.h, s.format);
	buffer->rawBlitFrom(s, Common::Rect(0, 0, s.w, s.h), Common::Point(0, 0));

	DrawBorderSide(Common::Point(x, y), Common::Point(width, height), s);
	DrawBorderOuterHighlights(Common::Point(x, y), Common::Point(width, height), s);

	uint16 buttonX = (s.w / 2) - (maxWidthButtonIcon + 4) * 3 + 2;
	uint16 buttonY = y + height - 4 - maxHeightButtonIcon;

	// Draw the buttons at the bottom
	for (int i = 0; i < 6; i++) {
		uint16 index = g_engine->inventoryIconIndices[i];
		AnimFrame &currentFrame = g_engine->imageResources[index - 1];
		DrawPressedBorderOuterHighlights(Common::Point(buttonX, buttonY), Common::Point(maxWidthButtonIcon, maxHeightButtonIcon), s);
		uint16 iconX = (maxWidthButtonIcon / 2 + buttonX) - currentFrame.Width / 2;
		uint16 iconY = (maxHeightButtonIcon / 2 + buttonY) - currentFrame.Height / 2;
		inventoryButtonLocations[i] = Common::Rect(Common::Point(buttonX, buttonY), maxWidthButtonIcon, maxHeightButtonIcon);
		DrawSprite(iconX, iconY, currentFrame.Width, currentFrame.Height, currentFrame.Data, s, false);
		buttonX += maxWidthButtonIcon + 4;
	}
	Common::Rect sourceRect(Common::Point((s.w / 2) - ((maxWidthInventoryIcon + 4) * 5 + 4) / 2 + 1, y + 5),
							(maxWidthInventoryIcon + 4) * 5 + 2, (maxHeightInventoryIcon + 4) * 2 + 2);
	s.rawBlitFrom(*buffer, sourceRect, Common::Point(sourceRect.left, sourceRect.top));

	DrawBorderOuterHighlights(Common::Point(
								  (s.w / 2) - ((maxWidthInventoryIcon + 4) * 5 + 4) / 2,
								  y + 4),
							  Common::Point(
								  (maxWidthInventoryIcon + 4) * 5 + 4,
								  (maxHeightInventoryIcon + 4) * 2 + 4),
							  s);

	uint16 itemX = x + 8;
	uint16 itemY = x + 8;
	inventoryGridUpperLeft.x = itemX;
	inventoryGridUpperLeft.y = itemY;
	inventorySlotSize.x = maxWidthInventoryIcon;
	inventorySlotSize.y = maxHeightInventoryIcon;
	// TODO: Align with original code's pagingation and ordering logic
	uint16 itemIndex = inventoryPage * 5;
	// Now the inventory icons themselves
	for (int iy = 0; iy < 2; iy++) {
		for (int ix = 0; ix < 5; ix++) {
			if (itemIndex > inventoryItems.size() - 1) {
				break;
			}
			itemIndex++;
			// TODO: Seems like this can happen if we take all items out
			// of another inventory
			if (itemIndex >= inventoryItems.size()) {
				break;
			}
			AnimFrame *icon = GetInventoryIcon(inventoryItems[itemIndex]);
			// ;; x = slotWidth / 2 + currentX - imageWidth / 2
			//  ;; y = slotHeight / 2 + currentY - imageHeight / 2
			DrawSprite(maxWidthInventoryIcon / 2 + itemX - icon->Width / 2,
					   maxHeightInventoryIcon / 2 + itemY - icon->Height / 2,
					   icon->Width, icon->Height, icon->Data, s, false);
			itemX += maxWidthInventoryIcon + 4;
		}
		itemX = x + 8;
		itemY += maxHeightInventoryIcon + 4;
	}
}

GameObject *View1::getClickedInventoryItem(const Common::Point &p) {
	// TODO: Add proper grid, add y as well
	Common::Rect inventoryRect(0x36, 0x2C, 0x10A, 0x82);
	int x = 0;
	int y = 0;
	int rowHeight = 0;
	for (GameObject *currentItem : inventoryItems) {
		AnimFrame *icon = GetInventoryIcon(currentItem);
		Common::Rect currentRect(Common::Point(0x36 + x, 0x2c + y), icon->Width, icon->Height);
		if (currentRect.contains(p)) {
			return currentItem;
		}
		x += icon->Width;
		rowHeight = MAX<int>(rowHeight, icon->Height);
		if (x > inventoryRect.width()) {
			x = 0;
			y += rowHeight;
		}
	}
	return nullptr;
}

GameObject *View1::getClickedInventoryItem2(const Common::Point &p) {

	Common::Rect currentInventorySlot(inventoryGridUpperLeft, inventoryGridUpperLeft + inventorySlotSize);

	uint16 itemIndex = inventoryPage * 5;
	for (int iy = 0; iy < 2; iy++) {
		for (int ix = 0; ix < 5; ix++) {
			if (itemIndex >= inventoryItems.size()) {
				break;
			}
			if (currentInventorySlot.contains(p)) {
				return inventoryItems[itemIndex];
			}
			itemIndex++;
			currentInventorySlot.moveTo(currentInventorySlot.left + inventorySlotSize.x, currentInventorySlot.top);
		}
		currentInventorySlot.moveTo(inventoryGridUpperLeft.x, currentInventorySlot.top + inventorySlotSize.y + 4);
	}
	return nullptr;
}

void View1::DrawSprite(int16 x, int16 y, uint16 width, uint16 height, byte *data, Graphics::ManagedSurface &s, bool mirrored, bool useDepth, uint8 depth) {
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
					// TODO: Check which relation has to hold
					if (!useDepth || bgDepth < depth) {
						s.setPixel(x + actualX, y + currentY, val);
					}
				}
			}
		}
	}
}

void View1::DrawSprite(const Common::Point &pos, uint16 width, uint16 height, byte *data, Graphics::ManagedSurface &s, bool mirrored, bool useDepth, uint8 depth) {
	DrawSprite(pos.x, pos.y, width, height, data, s, mirrored, useDepth, depth);
}

void View1::DrawSpriteClipped(uint16 x, uint16 y, Common::Rect &clippingRect, uint16 width, uint16 height, const byte *const data, Graphics::ManagedSurface &s) {
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

void View1::DrawSpriteClipped(uint16 x, uint16 y, Common::Rect &clippingRect, const Sprite &sprite, Graphics::ManagedSurface &s) {
	DrawSpriteClipped(x, y, clippingRect, sprite.Width, sprite.Height, sprite.Data.data(), s);
}

void View1::DrawSpriteAdvanced(uint16 x, uint16 y, uint16 width, uint16 height, uint16 scaling, const byte *data, Graphics::ManagedSurface &s) {
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

void View1::DrawSpriteAdvanced(const Common::Point &pos, uint16 width, uint16 height, uint16 scaling, const Sprite &sprite, Graphics::ManagedSurface &s) {
	DrawSpriteAdvanced(pos.x, pos.y, width, height, scaling, sprite.Data.data(), s);
}

void View1::DrawSpriteSuperAdvanced(const Common::Point &pos, const Sprite &sprite, uint16 scaling, bool mirrored, bool useDepth, uint8 depth, Graphics::ManagedSurface &s) {
	const uint16 &x = pos.x;
	const uint16 &y = pos.y;
	const uint16 &width = sprite.Width;
	const uint16 &height = sprite.Height;
	const Common::Array<uint8> data = sprite.Data;
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
					// Check for depth
					uint8 bgDepth = g_engine->_depthMap.getPixel(finalX, finalY);
					// TODO: Check which relation has to hold
					if (!useDepth || bgDepth < depth) {
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

void View1::DrawCharacters(Graphics::ManagedSurface &s) {
	// Y-sort characters (quicksort by Y position, ascending = back to front)
	// from sortObjectListByY / buildSortedObjectList (1008:8cf2)
	Common::Array<Character *> sorted(characters);
	Common::sort(sorted.begin(), sorted.end(), [](Character *a, Character *b) {
		return a->GetPosition().y < b->GetPosition().y;
	});

	for (auto current : sorted) {
		int index = current->GameObject->Index;
		if (!current->GameObject->IsVisible) {
			continue;
		}
		// TODO: Object 50h is a special one, it is the invisible object that moves along the
		// ground during the stick throw. Need to check how this is handled it the game
		// TODO: I'm kind of guessing that nr. 10 also is not visible, it does not appear
		// to have a lot of data to it. Random guess maybe this is the cup which is static?
		// TODO: Check what objects 17 and 18 and 23 in the machine room scene might be
		if (is_in_list<uint16, 0x50, 0x17, 0x18, 0x23>(index)) { // || index == 0x10) {
			continue;
		}

		AnimFrame *frame = current->GetCurrentAnimationFrame();
		bool mirror = current->shouldMirrorCurrentAnimation;

		// AnimFrame *frame = current->GetCurrentPortrait();
		uint8 depth = current->GetPosition().y;
		if (depth == 0) {
			// TODO: This is a quick fix for the issue of the gangster at the beginning not being removed properly
			continue;
		}
		uint8 bgDepth = g_engine->_depthMap.getPixel(current->GetPosition().x, current->GetPosition().y);
		g_system->setWindowCaption(Common::String::format("Depth %u vs. %u", depth, bgDepth));
		// Only output debug values for the character
		uint16 scalingFactor = CalculateCharacterScaling(depth, index == 1);
		// Adjust the position based on the scale
		// TODO: Search where this is done in the game code
		// DrawSprite(current->GetPosition() - frame->GetBottomMiddleOffset(), frame->Width, frame->Height, frame->Data, s, mirror, true, depth);
		// DrawSpriteAdvanced(current->GetPosition() - frame->GetBottomMiddleOffset(scalingFactor), frame->Width, frame->Height, scalingFactor, frame->AsSprite(), s);
		Common::Point actualPosition = current->GetPosition() - Common::Point(0, current->GetVerticalOffset());
		DrawSpriteSuperAdvanced(actualPosition - frame->GetBottomMiddleOffset(scalingFactor), frame->AsSprite(), scalingFactor, mirror, true, depth, s);

		Common::String number = Common::String::format("%u", scalingFactor);
		// number = Common::String::format("%u", scalingFactor);
		// number = Common::String::format("%u", current->GameObject->Index);
		number = Common::String::format("%u", current->GameObject->Orientation);
		renderString(current->GetPosition(), number.c_str());
		// Draw the white dot
		// TODO: Why does it not work for the others apart from the player?
		Common::Rect screenRect(0, 0, 320, 200);
		if (screenRect.contains(current->GetPosition())) {
			s.setPixel(current->GetPosition().x, current->GetPosition().y, 0xFF);
		}
		// DrawSprite(Common::Point(50, 50), frame->Width, frame->Height, frame->Data, s);
	}
}

void View1::ShowSpeechAct(uint16 characterIndex, const Common::Array<Common::String> &strings, const Common::Point &position, bool onRightSide) {
	setStringBox(strings);
	_isShowingDialogueChoice = false;
	_dialogueChoiceCount = 0;
	_continueScriptAfterUI = true;

	currentSpeechActData.speaker = GetCharacterByIndex(characterIndex);
	currentSpeechActData.strings = strings;
	currentSpeechActData.position = position;
	currentSpeechActData.onRightSide = onRightSide;

	const int totalWidth = g_engine->MeasureStrings(strings) + 0x12;
	const int totalHeight = g_engine->MeasureStringsVertically(strings) + 0x10;
	int stringBoxX = position.x;
	int stringBoxY = position.y;
	Common::Point portraitBoxPosition = position;

	if (currentSpeechActData.speaker != nullptr) {
		AnimFrame *leftPortrait = currentSpeechActData.speaker->GetCurrentPortrait(false);
		AnimFrame *rightPortrait = currentSpeechActData.speaker->GetCurrentPortrait(true);
		const int portraitWidth = MAX<int>(leftPortrait ? leftPortrait->Width : 0, rightPortrait ? rightPortrait->Width : 0);
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
	stringBoxPosition = Common::Point(stringBoxX, stringBoxY);
	debug("Layout speech act: speaker=%u rawPos=(%d,%d) rightSide=%u portraitBorderPos=(%d,%d) textBorderPos=(%d,%d) textBorderSize=(%d,%d) text=\"%s\"",
		  characterIndex, position.x, position.y, onRightSide ? 1 : 0,
		  currentSpeechActData.position.x, currentSpeechActData.position.y,
		  stringBoxPosition.x, stringBoxPosition.y, totalWidth, totalHeight, joinDebugStrings(strings).c_str());
	debugC(kDebugScript,
		   "Layout speech act: speaker=%u rawPos=(%d,%d) rightSide=%u portraitBorderPos=(%d,%d) textBorderPos=(%d,%d) textBorderSize=(%d,%d) text=\"%s\"",
		   characterIndex, position.x, position.y, onRightSide ? 1 : 0,
		   currentSpeechActData.position.x, currentSpeechActData.position.y,
		   stringBoxPosition.x, stringBoxPosition.y, totalWidth, totalHeight, joinDebugStrings(strings).c_str());

	if (autoclickActive) {
		clearStringBox();
	}
}

void View1::DrawBorder(const Common::Point &pos, const Common::Point &size, Graphics::ManagedSurface &s) {
	// fn0037_A65D proc
	constexpr uint16 width = 6;
	debugC(kDebugScript, "Render border: pos=(%d,%d) size=(%d,%d)", pos.x, pos.y, size.x, size.y);

	// TODO: Not sure what cmp	word ptr [2026h],1h does
	// Draw the background
	drawDarkRectangle(pos.x + 1, pos.y + 1, size.x - 1, size.y - 1);

	// Left side
	DrawBorderSide(pos, Common::Point(width, size.y), s);

	// Right side
	// TODO: Check if we have the right offset on the right, I missed the part about adding
	// the width originally
	DrawBorderSide(pos + Common::Point(size.x - width, 0), Common::Point(width, size.y), s);

	// Top side
	DrawBorderSide(pos, Common::Point(size.x, width), s);

	// Bottom side
	DrawBorderSide(pos + Common::Point(0, size.y - width), Common::Point(size.x, width), s);

	// Add the function for filling a side of the border
	// Algorithm
	// Set up clipping rect on one side
	// Draw the texture enough times in x and y to fill the clipping rect

	// Draw the highlights and shadows
	// TODO: Compare with the code at l0037_A6FA: especially what
	// the skipped (image argument 0) calls do or if I took care of them
	//
	// Top side highlight
	DrawHorizontalBorderHighlight(pos + Common::Point(1, 1), size.x - 1, 0x1012, s);

	// Left side highlight
	DrawVerticalBorderHighlight(pos + Common::Point(1, 1), size.y - 1, 0x1012, s);

	// Bottom shadow
	DrawHorizontalBorderHighlight(pos + Common::Point(1, size.y - 1), size.x - 1, 0x1011, s);

	// Right side shadow
	DrawVerticalBorderHighlight(pos + Common::Point(size.x - 1, 1), size.y - 1, 0x1011, s);

	// Top shadow
	DrawHorizontalBorderHighlight(pos + Common::Point(6, 6), size.x - 0xB, 0x1011, s);

	// Left shadow
	DrawVerticalBorderHighlight(pos + Common::Point(6, 6), size.y - 0xB, 0x1011, s);

	// Bottom highlight
	DrawHorizontalBorderHighlight(pos + Common::Point(6, size.y - width), size.x - 0xB, 0x1012, s);

	// Right highlight
	DrawVerticalBorderHighlight(pos + Common::Point(size.x - width, width), size.y - 0xB, 0x1012, s);

	/*

	;; These here should be the shadow parts
	;; Args are x and y (+C, +A), width (+8) and height (+6)
	;; Arguments pushed: X+6, Y + 6, W - Bh, Lowlight color
	x + 6, y + 6, w-Bh
	call	far 0037h:3737h
	mov	ax,[bp+0Ch]
	add	ax,6h
	push	ax
	mov	ax,[bp+0Ah]
	add	ax,6h
	push	ax
	mov	ax,[bp+6h]
	sub	ax,0Bh
	push	ax
	les	di,[bp-4h]
	mov	al,es:[di+1011h]
	xor	ah,ah
	push	ax
	call	far 0037h:3876h
	mov	ax,[bp+0Ch]
	add	ax,6h
	push	ax
	mov	ax,[bp+0Ah]
	add	ax,[bp+6h]
	sub	ax,6h
	push	ax
	mov	ax,[bp+8h]
	sub	ax,0Bh
	push	ax
	les	di,[bp-4h]
	mov	al,es:[di+1012h]
	xor	ah,ah
	push	ax
	call	far 0037h:3737h
	mov	ax,[bp+0Ch]
	add	ax,[bp+8h]
	sub	ax,6h
	push	ax
	mov	ax,[bp+0Ah]
	add	ax,6h
	push	ax
	mov	ax,[bp+6h]
	sub	ax,0Bh
	push	ax
	les	di,[bp-4h]
	mov	al,es:[di+1012h]
	xor	ah,ah
	push	ax
	call	far 0037h:3876h*/
}

// TODO: Probably a misnomer
void View1::DrawBorderSide(const Common::Point &pos, const Common::Point &size, Graphics::ManagedSurface &s) {
	// 0037h:39B5h

	// Clipping rectangle setup at l0037_39FE:
	Common::Rect clippingRect(pos + Common::Point(1, 1), pos + size);
	// TODO: Should check which texture we actually use at the moment

	// TODO: Check which area we actually fill
	uint16 currentX = clippingRect.left;
	uint16 currentY = clippingRect.top;
	const Sprite &sprite = g_engine->_borderSprite;

	while (currentY < clippingRect.bottom) {
		while (currentX < clippingRect.right) {
			DrawSpriteClipped(currentX, currentY, clippingRect, sprite, s);
			currentX += sprite.Width;
		}
		currentX = clippingRect.left;
		currentY += sprite.Height;
	}
}

void View1::DrawBorderOuterHighlights(const Common::Point &pos, const Common::Point &size, Graphics::ManagedSurface &s) {

	DrawHorizontalBorderHighlight(pos, size.x + 1, 0x1010, s);
	DrawVerticalBorderHighlight(pos, size.y + 1, 0x1010, s);
	DrawHorizontalBorderHighlight(pos + Common::Point(0, size.y), size.x + 1, 0x1010, s);
	DrawVerticalBorderHighlight(pos + Common::Point(size.x, 0), size.y + 1, 0x1010, s);
	DrawHorizontalBorderHighlight(pos + Common::Point(1, 1), size.x - 1, 0x1012, s);
	DrawVerticalBorderHighlight(pos + Common::Point(1, 1), size.y - 1, 0x1012, s);
	DrawHorizontalBorderHighlight(pos + Common::Point(1, size.y - 1), size.x - 1, 0x1011, s);
	DrawVerticalBorderHighlight(pos + Common::Point(size.x - 1, 1), size.y - 1, 0x1011, s);
}

void View1::DrawPressedBorderOuterHighlights(const Common::Point &pos, const Common::Point &size, Graphics::ManagedSurface &s) {
	DrawHorizontalBorderHighlight(pos, size.x + 1, 0x1010, s);
	DrawVerticalBorderHighlight(pos, size.y + 1, 0x1010, s);
	DrawHorizontalBorderHighlight(pos + Common::Point(0, size.y), size.x + 1, 0x1010, s);
	DrawVerticalBorderHighlight(pos + Common::Point(size.x, 0), size.y + 1, 0x1010, s);
	DrawHorizontalBorderHighlight(pos + Common::Point(1, 1), size.x - 1, 0x1011, s);
	DrawVerticalBorderHighlight(pos + Common::Point(1, 1), size.y - 1, 0x1011, s);
	DrawHorizontalBorderHighlight(pos + Common::Point(1, size.y - 1), size.x - 1, 0x1011, s);
	DrawVerticalBorderHighlight(pos + Common::Point(size.x - 1, 1), size.y - 1, 0x1011, s);
}

Macs2::Sprite *View1::GetUISprite(uint32 offset) {
	if (offset == 0x1012) {
		return &g_engine->_borderHighlightSprite;
	} else if (offset == 0x1011) {
		return &g_engine->_borderShadowSprite;
	} else if (offset == 0x1010) {
		return nullptr;
	}
	// We should not get here
	assert(false);
	return nullptr;
}

void View1::DrawHorizontalBorderHighlight(const Common::Point &pos, int16 width, uint32 spriteAddress, Graphics::ManagedSurface &s) {

	// 0037:3AF5 (in fn0037_3AD4)

	// TODO: There is quite some setup going on in this function before we get to the drawing

	Common::Rect clippingRect(pos, pos + Common::Point(width, 1));
	// TODO: Should check which texture we actually use at the moment

	// TODO: Check which area we actually fill
	uint16 currentX = clippingRect.left;
	uint16 currentY = clippingRect.top;

	const Sprite *sprite = GetUISprite(spriteAddress);
	if (sprite == nullptr) {
		return;
	}
	while (currentX < clippingRect.right) {
		DrawSpriteClipped(currentX, currentY, clippingRect, *sprite, s);
		currentX += sprite->Width;
	}
}

void View1::DrawVerticalBorderHighlight(const Common::Point &pos, int16 height, uint32 spriteAddress, Graphics::ManagedSurface &s) {
	// TODO: Only copy&paste from horizontal so far, need to check original code
	// TODO: Add the assembly location we are at

	// TODO: There is quite some setup going on in this function before we get to the drawing

	Common::Rect clippingRect(pos, pos + Common::Point(1, height));
	// TODO: Should check which texture we actually use at the moment

	// TODO: Check which area we actually fill
	uint16 currentX = clippingRect.left;
	uint16 currentY = clippingRect.top;

	const Sprite *sprite = GetUISprite(spriteAddress);
	if (sprite == nullptr) {
		return;
	}

	while (currentY < clippingRect.bottom) {
		DrawSpriteClipped(currentX, currentY, clippingRect, *sprite, s);
		currentY += sprite->Height;
	}
}

void View1::DrawImageResources(Graphics::ManagedSurface &s) {
	uint16 x = 0;
	uint16 y = 0;
	uint16 currentMaxHeight = 0;
	for (AnimFrame &current : g_engine->imageResources) {
		if (x + current.Width > 320) {
			y += currentMaxHeight;
			x = 0;
			currentMaxHeight = 0;
		}
		DrawSprite(Common::Point(x, y), current.Width, current.Height, current.Data, s, false);
		x += current.Width;
		currentMaxHeight = MAX(current.Height, currentMaxHeight);
	}
}

void View1::ShowDialogueChoice(uint16 speakerObjectID, const Common::Array<Common::StringArray> &choices, const Common::Point &position, bool onRightSide) {
	Common::StringArray joinedLines;
	for (auto &currentLines : choices) {
		for (auto &currentLine : currentLines) {
			joinedLines.push_back(currentLine);
		}
	}

	ShowSpeechAct(speakerObjectID, joinedLines, position, onRightSide);
	_isShowingDialogueChoice = true;
	_dialogueChoiceCount = choices.size();
}

void View1::TriggerDialogueChoice(uint8 index) {
	if (!_isShowingDialogueChoice || index < 1 || index > _dialogueChoiceCount) {
		warning("Ignoring dialogue choice %u without an active matching choice UI", index);
		return;
	}

	// TODO: Confirm that these two are really set accordingly
	g_engine->_scriptExecutor->SetVariableValue(0x0d, index, 0);
	g_engine->_scriptExecutor->chosenDialogueOption = index;

	// TODO: Should check where this happens, but seems like we need to close the
	// options ourselves
	// TODO: Check if the run script after UI needs to be overridden here to not
	// schedule an unnecessary run
	clearStringBox(false);
	// TODO: Not sure about the first run variable here
	g_engine->RunScriptExecutor();
}

uint16 View1::CalculateCharacterScaling(uint16 characterY, bool updateDebugValues) {
	// l0037_93F4: 	scummvm.exe!Macs2::View1::msgKeypress(const Macs2::KeypressMessage & msg) Line 542	C++

	int32 eax = g_engine->word51FD;
	int32 edx = 0;
	int32 ecx = eax;
	int32 ebx = edx;
	eax = characterY;
	if (updateDebugValues) {
		scalingValues.characterY = characterY;
	}
	// TODO: Check this case when it happens
	// assert(eax >= ecx);
	eax -= ecx;
	ebx = eax;
	eax = g_engine->word51FF;
	// TODO: Check this case when it happens
	// assert(eax == 0);
	edx = 0;
	eax *= ebx;
	ebx = 0x64;
	eax /= ebx;
	ebx = eax;
	eax = g_engine->word5201;
	edx = 0;
	eax += ebx;
	if (updateDebugValues) {
		scalingValues.scalingFactor = eax;
	}
	return eax;
}

uint16 View1::GetHitObjectID(const Common::Point &pos) const {
	// TODO: Naive implementation for now
	for (auto currentCharacter : characters) {
		if (!currentCharacter->GameObject->IsVisible || !currentCharacter->GameObject->IsClickable) {
			continue;
		}
		auto animFrame = currentCharacter->GetCurrentAnimationFrame();

		// Saved point of the object is at the bottom in the middle, frame local space starts
		// at top left
		Common::Point localPoint = pos - (currentCharacter->GetPosition() - animFrame->GetBottomMiddleOffset());
		bool isHit = animFrame->PixelHit(localPoint);
		if (isHit) {
			return 0x0400 + currentCharacter->GameObject->Index;
		}
	}
	// TODO: Ignore background image lookup for now
	return 0x0000;
}

bool Character::HandleWalkability(Character *c) {
	// Wall-sliding obstacle avoidance from walkAlongPath (1008:1b8f).
	// When the character steps into a non-walkable pixel (walkability >= 200),
	// the original code samples walkability at +/-1 and +/-2 pixels in each
	// axis to build a gradient vector, then slides the character along that
	// vector until it reaches a walkable position.
	if (c->GameObject->Index != 1) {
		return false;
	}
	if (g_engine->_scriptExecutor->IsExecuting()) {
		return false;
	}

	Common::Point pos = c->GetPosition();
	if (IsWalkable(pos)) {
		return false;
	}

	// Build a push vector by sampling the walkability map around the current
	// position. Non-walkable neighbors push us away from them.
	int pushX = 0;
	int pushY = 0;

	// Sample at distance 1
	if (!IsWalkable(Common::Point(pos.x + 1, pos.y)))
		pushX -= 1;
	if (!IsWalkable(Common::Point(pos.x - 1, pos.y)))
		pushX += 1;
	if (!IsWalkable(Common::Point(pos.x, pos.y + 1)))
		pushY -= 1;
	if (!IsWalkable(Common::Point(pos.x, pos.y - 1)))
		pushY += 1;

	// Sample at distance 2 for stronger gradient
	if (!IsWalkable(Common::Point(pos.x + 2, pos.y)))
		pushX -= 1;
	if (!IsWalkable(Common::Point(pos.x - 2, pos.y)))
		pushX += 1;
	if (!IsWalkable(Common::Point(pos.x, pos.y + 2)))
		pushY -= 1;
	if (!IsWalkable(Common::Point(pos.x, pos.y - 2)))
		pushY += 1;

	// Slide along the push vector
	while (pushX != 0 || pushY != 0) {
		if (pushX < 0) {
			if (IsWalkable(Common::Point(pos.x - 1, pos.y)))
				pos.x -= 1;
			pushX += 1;
		}
		if (pushX > 0) {
			if (IsWalkable(Common::Point(pos.x + 1, pos.y)))
				pos.x += 1;
			pushX -= 1;
		}
		if (pushY < 0) {
			if (IsWalkable(Common::Point(pos.x, pos.y - 1)))
				pos.y -= 1;
			pushY += 1;
		}
		if (pushY > 0) {
			if (IsWalkable(Common::Point(pos.x, pos.y + 1)))
				pos.y += 1;
			pushY -= 1;
		}
	}

	c->SetPosition(pos);
	return true;
}

uint8 Character::LookupWalkability(const Common::Point &p) const {
	Common::Rect screenRect(320, 200);
	if (!screenRect.contains(p)) {
		return 0x00;
	}
	uint32 value = g_engine->_pathfindingMap.getPixel(p.x, p.y);
	// Values 0xC8..0xEF use the pathfinding override table (opcode 0x12)
	if (value >= 0xC8 && value <= 0xEF) {
		uint16 overrideResult;
		if (g_engine->GetPathfindingOverride(value, overrideResult)) {
			return (uint8)overrideResult;
		}
		// Override not active - return non-walkable
		return 0xFF;
	}
	return (uint8)value;
}

bool Character::IsWalkable(const Common::Point &p) const {
	uint8 walkability = LookupWalkability(p);
	return walkability < 0xC8;
}

bool Character::IsLineSegmentWalkable(const Common::Point &p1, const Common::Point &p2, bool print) {
	int x1 = p1.x;
	int y1 = p1.y;
	int x2 = p2.x;
	int y2 = p2.y;

	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);

	int sx = (x1 < x2) ? 1 : -1;
	int sy = (y1 < y2) ? 1 : -1;

	int err = dx - dy;

	bool result = true;
	while (true) {
		Common::Point currentPoint(x1, y1);
		bool isCurrentWalkable = IsWalkable(currentPoint);
		if (print)
			PathfindingOverlay[y1 * 320 + x1] = isCurrentWalkable ? 100 : 50;
		if (!isCurrentWalkable) { // If the point is not walkable, save for later
			result = false;
		}

		if (x1 == x2 && y1 == y2)
			break; // Reached the end point

		int e2 = 2 * err;
		if (e2 > -dy) {
			err -= dy;
			x1 += sx;
		}
		if (e2 < dx) {
			err += dx;
			y1 += sy;
		}
	}

	return result;
}

Character::Character() {
	PathfindingOverlay = Common::Array<uint8>(320 * 200, 0);
	ExecuteScriptOnFinishLerp = false;
}

bool Character::FindPath(Common::Point target) {
	// First naive implementation
	// Find the closest reachable start point
	// Check if we can reach the target
	// If not: Do a recusrive search using the other points

	// TODO: Assume we have to use the net, usually we would do a path trace
	constexpr uint16 numPoints = 16;
	uint minLength = std::numeric_limits<uint>::max();
	int minIndex = -1;
	const Common::Point &charPosition = GameObjects::instance().GetProtagonistObject()->Position;
	for (int i = 0; i < numPoints; i++) {
		PathfindingPoint &current = g_engine->pathfindingPoints[i];
		if (IsLineSegmentWalkable(charPosition, current.Position)) {
			uint dist = charPosition.sqrDist(current.Position);
			if (dist < minLength) {
				minLength = dist;
				minIndex = i;
			}
		}
	}

	// TODO: Handle not finding a start point
	// g_engine->_path.push_back(g_engine->pathfindingPoints[minIndex].Position);
	if (minIndex == -1) {
		debug("No walkable entry point found!");
		return false;
	}

	Common::Array<bool> visited;
	visited.resize(16);
	debug("Best entry point: %u at distance %u", minIndex, minLength);
	bool result = VisitPathfindingNode(minIndex, visited, target);
	PathfindingPoint &entryPoint = g_engine->pathfindingPoints[minIndex];
	IsLineSegmentWalkable(charPosition, entryPoint.Position, true);
	// Now handle searching for the end point, for this, keep track of nodes we already visited
	// Args:
	// Target position
	// Current path
	// Array of points already visited

	return result;
}

bool Character::VisitPathfindingNode(uint16 index, Common::Array<bool> &visited, const Common::Point &target) {
	if (visited[index] == true) {
		// We have visited this node before
		return false;
	}
	visited[index] = true;
	const PathfindingPoint &currentPoint = g_engine->pathfindingPoints[index];
	const Common::Point &currentPosition = currentPoint.Position;
	Path.push_back(index);
	g_engine->_path.push_back(currentPosition);

	// Check if we can reach the target from here
	if (IsLineSegmentWalkable(currentPosition, target)) {
		IsLineSegmentWalkable(currentPosition, target, true);
		return true;
	}

	// See if the adjacent points are good
	for (uint i = 0; i < currentPoint.adjacentPoints.size(); i++) {
		const uint16 currentAdjacentIndex = currentPoint.adjacentPoints[i];
		if (VisitPathfindingNode(currentAdjacentIndex - 1, visited, target)) {
			const PathfindingPoint &adjacentPoint = g_engine->pathfindingPoints[currentAdjacentIndex - 1];
			IsLineSegmentWalkable(currentPoint.Position, adjacentPoint.Position, true);
			return true;
		}
	}
	// None we good, remove us from the path and return
	g_engine->_path.remove_at(g_engine->_path.size() - 1);
	Path.remove_at(Path.size() - 1);
	return false;
}

Common::Point Character::GetPosition() const {
	return GameObject->Position;
}

void Character::SetPosition(const Common::Point &newPosition) {
	GameObject->Position = newPosition;
}

uint16 Character::GetVerticalOffset() const {
	// See for example l0037_8F1F for this calculation
	// l0037_8F1F:
	uint16 result = g_engine->Func0E8C(GetPosition()); // [bp-0Ch]
	if (result >= 0xC8) {
		// l0037_8F38:
		result = 0;
	}

	if (GameObject->Unknown != 0) {
		result = (result * GameObject->Unknown) / 100;
	}

	return result;
}

bool Character::TryFollowPath() {
	CurrentPathIndex++;
	if ((uint)CurrentPathIndex == Path.size()) {
		// This means we now need to move to the final destination
		StartLerpTo(PathFinalDestination, 1000);
		return true;
	}
	if ((uint)CurrentPathIndex == Path.size() + 1) {
		return false;
	}
	const uint16 currentPathPointIndex = Path[CurrentPathIndex]; // -1;
	// Set up a lerp
	Common::String output = Common::String::format("%u - %u", CurrentPathIndex, currentPathPointIndex);
	g_engine->debugOutput.push_back(output);
	PathfindingPoint &current = g_engine->pathfindingPoints[currentPathPointIndex];
	StartLerpTo(current.Position, 1000);
	return true;
}

bool Character::isAnimationMirrored() const {
	return is_in_list<uint16, 6, 7, 8, 14, 15, 16>(GameObject->Orientation);
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

Macs2::AnimFrame *Character::GetCurrentAnimationFrame() {
	// We choose looking towards the screen first
	int blobIndex = GameObject->Orientation - 1;
	bool mirror = false;

	if (isAnimationMirrored()) {
		const int mirroredBlobIndex = getMirroredAnimation(GameObject->Orientation) - 1;
		const bool hasOriginalBlob = blobIndex >= 0 && blobIndex < (int)GameObject->Blobs.size() && !GameObject->Blobs[blobIndex].empty();
		const bool hasMirroredSourceBlob = mirroredBlobIndex >= 0 && mirroredBlobIndex < (int)GameObject->Blobs.size() &&
										   !GameObject->Blobs[mirroredBlobIndex].empty();
		const bool hasDistinctOriginalFacing = hasOriginalBlob &&
											   (!hasMirroredSourceBlob || GameObject->Blobs[blobIndex] != GameObject->Blobs[mirroredBlobIndex]);

		if (!hasDistinctOriginalFacing && hasMirroredSourceBlob) {
			blobIndex = mirroredBlobIndex;
			mirror = true;
		}
	}
	/* if (IsLerping) {
		// We are walking
		blobIndex = GameObject->Orientation + 1;
	} else {
		// We are standing
		blobIndex = GameObject->Orientation + 9;
	} */
	// TODO: The game saves the orientation already adjusted for animation state
	/* if (blobIndex > 8 + 9) {
		blobIndex = 9;
	} */
	// If we don't have this direction, try others until we find one that we have
	// TODO: Log this properly or even assert
	if (GameObject->Blobs[blobIndex].size() == 0) {
		// TODO: Consider a placeholder or an assert to figure out these cases
		debug("No animation blob found for object %.4x with orientation %.4x", GameObject->Index, GameObject->Orientation);
		for (int i = 0; i < 0x11; i++) {
			if (GameObject->Blobs[i].size() != 0) {
				blobIndex = i;
				break;
			}
		}
	}

	if (GameObject->useOverloadAnimation) {
		mirror ^= GameObject->overloadAnimationMirrored;
	} else if (blobIndex >= 0 && blobIndex < (int)GameObject->BlobMirrorFlags.size()) {
		mirror ^= GameObject->BlobMirrorFlags[blobIndex];
	}
	shouldMirrorCurrentAnimation = mirror;
	/*
	// int offset = 0x1C;

	// TODO: Need to figure out the access pattern more systematically
	if (GameObject->Index == 0x8) {
		blobIndex = 4;
	//	offset = 23;
	} else if (GameObject->Index == 0x0a) {
		// TODO: Figure out how we find these
		blobIndex = 6;
	} else if (GameObject->Index == 0x21) {
		blobIndex = 0x11;
	} else if (GameObject->Index == 0x10) {
		blobIndex = 0x0c;
	}
	*/

	// Old code from before 1480 implementation here
	/* AnimationReader testReader(this->GameObject->Blobs[blobIndex]);
	uint16 numAnimations = testReader.readNumAnimations();
	debug("Number of animation frames: %.4", numAnimations);

	Common::MemoryReadStream stream(this->GameObject->Blobs[blobIndex].data(), this->GameObject->Blobs[blobIndex].size());
	stream.seek(0xA, SEEK_SET);
	uint16 offset = stream.readUint16LE();
	offset += 0x8;
	stream.seek(offset, SEEK_CUR);

	AnimFrame* result = new AnimFrame();

	// TODO: Handle properly
	// Skip ahead to the right frame in the animation
	// TODO: No hardcoded number of animations
	// TODO: Check for one-off errors
	testReader.SeekToAnimation((animationIndex - 1) % numAnimations);
	// testReader.SeekToAnimation(0);
	// Skip ahead to the width and height
	testReader.readStream->seek(6, SEEK_CUR
	*/

	Common::Array<uint8> &blob = GameObject->useOverloadAnimation ? GameObject->overloadAnimation : GameObject->Blobs[blobIndex];

	// Update pass
	BackgroundAnimationBlob::Func1480(blob, true, 2);
	// Retrieval pass
	uint16 offset = BackgroundAnimationBlob::Func1480(blob, false, 0x0);
	// My remaining code expects to get dialed to the width and height directly - TODO make uniform
	offset += 6;
	AnimFrame *result = new AnimFrame();
	Common::MemoryReadStream stream(blob.data(), blob.size());
	stream.seek(offset);
	result->ReadFromStream(&stream);
	return result;
	// TODO: Think about proper memory management
}

Macs2::AnimFrame *Character::GetCurrentPortrait(bool onRightSide) {
	if (GameObject->Blobs.size() <= 17) {
		return nullptr;
	}

	uint portraitBlobIndex = 17;
	if (onRightSide && GameObject->Blobs.size() > 18 && !GameObject->Blobs[18].empty()) {
		portraitBlobIndex = 18;
	} else if (GameObject->Blobs[portraitBlobIndex].empty() && GameObject->Blobs.size() > 18 && !GameObject->Blobs[18].empty()) {
		portraitBlobIndex = 18;
	}

	if (GameObject->Blobs[portraitBlobIndex].empty()) {
		return nullptr;
	}

	uint16 offset = BackgroundAnimationBlob::Func1480(GameObject->Blobs[portraitBlobIndex], true, 2);
	// My remaining code expects to get dialed to the width and height directly - TODO make uniform
	offset += 6;
	AnimFrame *result = new AnimFrame();
	Common::MemoryReadStream stream(GameObject->Blobs[portraitBlobIndex].data(), GameObject->Blobs[portraitBlobIndex].size());
	stream.seek(offset);
	result->ReadFromStream(&stream);
	return result;
	// TODO: Think about proper memory management

	// Old code below from before 1480 implementation
	/* AnimFrame *result = new AnimFrame();
	Common::MemoryReadStream stream(this->GameObject->Blobs[17].data(), this->GameObject->Blobs[17].size());
	// TODO: Need to check how the offset really is calculated by the game code, this will not hold
	if (is_in_list<uint16, 2, 4, 6, 0xd, 0xf, 0x12, 0x16, 0x4D>(GameObject->Index)) {
		// GameObject->Index == 2 || GameObject->Index == 4 || GameObject->Index == 6 || GameObject->Index == 0xd || GameObject ->Index == 0xf) {
		stream.seek(35, SEEK_SET);
	} else {
		stream.seek(36, SEEK_SET);
	}

	result->ReadFromStream(&stream);
	return result;
	// TODO: Think about proper memory management
	*/
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
void Character::StartLerpTo(const Common::Point &target, uint32 duration, bool ignoreObstacles) {
	StartPosition = GetPosition();
	EndPosition = target;
	StartTime = g_events->currentMillis;
	Duration = duration;
	IsLerping = true;
	LerpIgnoresObstacles = ignoreObstacles;

	// Reset Bresenham state - direction will be calculated on first Update()
	_stepDirectionSet = false;
	_stepDeltaX = abs(EndPosition.x - StartPosition.x);
	_stepDeltaY = abs(EndPosition.y - StartPosition.y);
	_stepError = 0;
}

void Character::StartPickup(Macs2::GameObject *object) {
	pickedUpObject = object;
	ExecuteScriptOnFinishLerp = true;
	StartLerpTo(pickedUpObject->Position, 1000);
}

void Character::RegisterWaitForMovementFinishedEvent() {

	// For now, we are treating this one as a flag to send an event
	// even if we are not lerping, so that we have a delay between action 0x11
	// and the new execution
	ExecuteScriptOnFinishLerp = true;
}

void Character::Update() {
	if (!IsLerping && !IsFollowingPath) {
		if (pickupAnimationEndTime > 0.0f && pickupAnimationEndTime < g_engine->currentMillis) {
			// Finish picking up
			GameObject->Orientation = previousOrientation;
			pickupAnimationEndTime = -1.0f;
			View1 *currentView = (View1 *)g_engine->findView("View1");
			Character *pickedUpCharacter = currentView->GetCharacterByIndex(pickedUpObject->Index);
			if (pickedUpCharacter != nullptr) {
				int index = currentView->GetCharacterArrayIndex(pickedUpCharacter);
				if (index >= 0) {
					currentView->characters.remove_at(index);
				}
			}
			pickedUpObject->SceneIndex = GameObject->Index;
			if (currentView->inventorySource != nullptr && currentView->inventorySource->Index == GameObject->Index) {
				currentView->inventoryItems.push_back(pickedUpObject);
			}
			if (g_engine->_scriptExecutor->pickupInProgress) {
				g_engine->_scriptExecutor->pickupInProgress = false;
				g_engine->_scriptExecutor->pickupActorObjectID = 0;
				g_engine->_scriptExecutor->pickupTargetObjectID = 0;
				g_engine->SetCursorMode(g_engine->_scriptExecutor->savedPickupMouseMode);
				currentView->UpdateCursor();
			}
			pickedUpObject = nullptr;
			// From here on the interacted object should become 0
			g_engine->_scriptExecutor->_interactedObjectID = 0x0000;
			g_engine->_scriptExecutor->_interactedOtherObjectID = 0x0000;
			if (ExecuteScriptOnFinishLerp) {
				ExecuteScriptOnFinishLerp = false;
				g_engine->_scriptExecutor->global1032 = true;
				g_engine->ScheduleRun();
			}
			return;
		}
		if (pickedUpObject != nullptr && pickupAnimationEndTime < 0.0f) {
			View1 *currentView = (View1 *)g_engine->findView("View1");
			if (g_engine->_scriptExecutor->pickupInProgress) {
				g_engine->_scriptExecutor->pickupInProgress = false;
				g_engine->_scriptExecutor->pickupActorObjectID = 0;
				g_engine->_scriptExecutor->pickupTargetObjectID = 0;
				g_engine->SetCursorMode(g_engine->_scriptExecutor->savedPickupMouseMode);
				currentView->UpdateCursor();
			}
			pickedUpObject = nullptr;
			g_engine->_scriptExecutor->_interactedObjectID = 0x0000;
			g_engine->_scriptExecutor->_interactedOtherObjectID = 0x0000;
		}

		// We might have gotten the 0x11 command after we stopped moving
		// TODO: Check if the code handles this similarly
		// TODO: Consider which run function to use
		if (ExecuteScriptOnFinishLerp) {
			ExecuteScriptOnFinishLerp = false;
			g_engine->_scriptExecutor->global1032 = true;
			g_engine->ScheduleRun();
		}
		return;
	}
	// Bresenham pixel-stepping from walkAlongPath (1008:1b8f).
	// Each frame: calculate walk speed from depth, step that many pixels.
	Common::Point pos = GetPosition();
	// Depth-scaled walk speed from walkAlongPath (1008:1b8f):
	// speed = animSpeed * (word5201 + depthAtPos) / 100
	int depthAtPos = 0;
	Common::Rect screenRect(320, 200);
	if (screenRect.contains(pos)) {
		depthAtPos = g_engine->_depthMap.getPixel(pos.x, pos.y);
	}
	int walkSpeed = (2 * ((int)g_engine->word5201 + depthAtPos)) / 100;
	if (walkSpeed < 1) walkSpeed = 1;


	// Check if we have arrived at the target
	if (pos == EndPosition) {
		if (IsFollowingPath) {
			IsFollowingPath = TryFollowPath();
			if (IsFollowingPath) return;
		}
		IsLerping = false;
		if (hasMotionVerticalOffset) {
			GameObject->Unknown = motionTargetVerticalOffset;
			motionProgress = motionDistanceUnits;
			hasMotionVerticalOffset = false;
		}
		// Standing orientation = walking direction + 8
		if (GameObject->Orientation < 9) GameObject->Orientation += 8;
		if (pickedUpObject != nullptr) {
			pickupAnimationEndTime = g_events->currentMillis + 1000.0f;
			previousOrientation = GameObject->Orientation;
			GameObject->Orientation = 0x11;
			return;
		}
		if (ExecuteScriptOnFinishLerp) {
			ExecuteScriptOnFinishLerp = false;
			g_engine->_scriptExecutor->global1032 = true;
			g_engine->ScheduleRun();
		}
		return;
	}


	// Calculate direction if not yet set (first frame of movement)
	if (!_stepDirectionSet) {
		_stepDirectionSet = true;
		uint16 absDx = abs(EndPosition.x - pos.x);
		uint16 absDy = abs(EndPosition.y - pos.y);
		// Determine 8-directional orientation from walkAlongPath
		uint8 dir = GameObject->Orientation;
		if (dir > 8 && dir < 17) dir -= 8;
		if (pos.y > EndPosition.y && absDx <= absDy) dir = 1; // North
		if (pos.x < EndPosition.x && absDy <= absDx) dir = 3; // East
		if (pos.y < EndPosition.y && absDx <= absDy) dir = 5; // South
		if (pos.x > EndPosition.x && absDy <= absDx) dir = 7; // West
		// Diagonals: deltaX/4 < deltaY && deltaY/2 < deltaX
		if (absDx > (absDy >> 2) && absDy > (absDx >> 1)) {
			if (pos.y > EndPosition.y && pos.x < EndPosition.x) dir = 2;
			if (pos.y < EndPosition.y && pos.x < EndPosition.x) dir = 4;
			if (pos.y < EndPosition.y && pos.x > EndPosition.x) dir = 6;
			if (pos.y > EndPosition.y && pos.x > EndPosition.x) dir = 8;
		}
		GameObject->Orientation = dir;
		_stepDeltaX = absDx;
		_stepDeltaY = absDy;
		_stepError = 0;
	}


	// Step pixels (Bresenham line algorithm)
	int pixelsMoved = 0;
	for (int step = 0; step < walkSpeed; step++) {
		Common::Point prevPos = pos;
		if (_stepError < _stepDeltaX) {
			// Step along X axis
			if (EndPosition.x != pos.x) pixelsMoved++;
			if (EndPosition.x < pos.x) pos.x--;
			else if (EndPosition.x > pos.x) pos.x++;
			_stepError += _stepDeltaY;
		} else {
			// Step along Y axis
			if (EndPosition.y != pos.y) pixelsMoved++;
			if (EndPosition.y < pos.y) pos.y--;
			else if (EndPosition.y > pos.y) pos.y++;
			_stepError -= _stepDeltaX;
		}
		// Check walkability after each step
		if (!IsWalkable(pos)) {
			pos = prevPos;
			// Stop movement - hit a wall
			EndPosition = pos;
			break;
		}
		// Check if we reached the target
		if (pos == EndPosition) break;
	}


	// Update motion vertical offset if active
	if (hasMotionVerticalOffset && motionDistanceUnits != 0) {
		uint16 totalDist = _stepDeltaX + _stepDeltaY;
		if (totalDist > 0) {
			int32 distWalked = abs(pos.x - StartPosition.x) + abs(pos.y - StartPosition.y);
			float progress = (float)distWalked / (float)totalDist;
			if (progress > 1.0f) progress = 1.0f;
			int32 vDelta = (int32)motionTargetVerticalOffset - (int32)motionStartVerticalOffset;
			GameObject->Unknown = (uint16)((int32)motionStartVerticalOffset + (int32)(vDelta * progress));
		}
	}


	SetPosition(pos);
}

bool Button::IsPointInside(const Common::Point &p) const {
	return false;
}

void Button::Render(Graphics::ManagedSurface &s) {
}

} // namespace Macs2
