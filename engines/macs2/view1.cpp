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

#include "common/system.h"
#include "graphics/palette.h"
#include "macs2/view1.h"
#include "macs2/macs2.h"
#include "macs2/gameobjects.h"
#include <graphics/cursorman.h>
#include <math/angle.h>
#include <math/vector2d.h>

namespace Macs2 {
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
void View1::TransferInventoryItem(GameObject *item, GameObject *targetContainer) {
	int index = FindInventoryItem(item);
	inventoryItems.remove_at(index);
	item->SceneIndex = targetContainer->Index;
	
}

int View1::FindInventoryItem(GameObject *item) {
	for (int i = 0; i != inventoryItems.size(); i++) {
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
View1::View1() : UIElement("View1") {
		_backgroundSurface = g_engine->_bgImageShip;
		// TODO: Adjust for final min value
		int mode = (int)g_engine->_scriptExecutor->_mouseMode - (int)Script::MouseMode::Talk;
		g_engine->_cursorData[mode][(g_engine->_cursorWidths[mode] >> 1) + (g_engine->_cursorHeights[0] >> 1) * g_engine->_cursorWidths[mode]] = 0xFF;
		CursorMan.replaceCursor(g_engine->_cursorData[mode], g_engine->_cursorWidths[mode], g_engine->_cursorHeights[mode], g_engine->_cursorWidths[mode] >> 1, g_engine->_cursorHeights[0] >> 1, 0);
		CursorMan.showMouse(true);

		// TODO: Check if this works like this
		Character *protagonist = new Character();
		// TODO: Need to properly handle the offset
		// TODO: Remember that the game starts enumerating objects at 1 and not at 0
		protagonist->GameObject = GameObjects::instance().Objects[0x0];
		characters.push_back(protagonist);

		// inventoryItems.push_back(GameObjects::instance().Objects[0x8 - 1]);
	}

	AnimFrame *View1::GetInventoryIcon(GameObject *gameObject) {
		AnimFrame *result = new AnimFrame();
		int index = 5 - 1;
		if (is_in_list<uint16, 0x10, 0x11, 0x17, 0x18, 0x1B, 0x22, 0x23, 0x19, 0x1A, 0x14, 0x1C, 0x1D, 0x3C>(gameObject->Index)) {
			// gameObject->Index == 0x23 || gameObject->Index == 0x22) {
			// TODO Figure out these - the mug has a different blob
			index = 0x13;
		}
		index = 0x13;
		Common::MemoryReadStream stream(gameObject->Blobs[index].data(), gameObject->Blobs[index].size());
		// TODO: Need to check how the offset really is calculated by the game code, this will not hold
		stream.seek(23, SEEK_SET);
		result->ReadFromStream(&stream);
		return result;
		// TODO: Think about proper memory management
	}

	void View1::drawDarkRectangle(uint16 x, uint16 y, uint16 width, uint16 height)
	{
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
			BackgroundAnimation& current = g_engine->_backgroundAnimations[i];
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

		AnimFrame *frame = currentSpeechActData.speaker->GetCurrentPortrait();
		if (currentSpeechActData.onRightSide) {
			// TODO: Need to measure the string box for this one
			// TODO: Temporary fix by moving it to the left side
			currentSpeechActData.position.x = 10;
		}
		// TODO: Add the else block

		// See l0037_B462: for the calculations below
		// Draw the border
		const Common::Point borderSize(frame->Width + 0xD, frame->Height + 0xD);
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
				// TODO: Different character for not found?
				currentX += 10;
			}
		}
	}

	void View1::showStringBox(const Common::StringArray &sa) {
		// TODO: Naive and hardcoded implementation
		int contentHeight = sa.size() * 10;
		int contentWidth = g_engine->MeasureStrings(sa);
		int borderWidth = 10;
		int padding = 3;
		int totalWidth = contentWidth + (borderWidth + padding) * 2;
		int totalHeight = contentHeight + (borderWidth + padding) * 2;
		constexpr int x = 80;
		constexpr int y = 20;

		drawStringBackground(x, y, totalWidth, totalHeight);
		// TODO range based
		int lineOffset = borderWidth + padding;
		for (auto iter = sa.begin(); iter < sa.end(); iter++) {
			renderString(x + borderWidth + padding, y + lineOffset, *iter);
			lineOffset += 10;
		}
	}

	void View1::drawGlyphs(Macs2::GlyphData *data, int count, uint16 x, uint16 y, Graphics::ManagedSurface& s) {
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

		currentFadeValue -= fadeDelta;
		if (currentFadeValue < 0) {
			return;
		}

		byte *colors = new byte[256 * 3];
		// g_system->getPaletteManager()->grabPalette(colors, 0, 256);
		// Copy the untouched palette over
		memcpy(colors, g_engine->_palVanilla, 256 * 3);

		for (int i = 0; i < 256 * 3; i++) {
			if (colors[i] < currentFadeValue) {
				colors[i] = 0;
			} else {
				colors[i] -= currentFadeValue;
			}
			colors[i] = (colors[i] * 259 + 33) >> 6;
		}

		g_system->getPaletteManager()->setPalette(colors, 0, 256);

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
		Common::Array<uint8> &overlay = characters[0]->PathfindingOverlay;
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
		for (int i = 0; i < g_engine->_path.size() - 1; i++) {
			s.drawLine(g_engine->_path[i].x, g_engine->_path[i].y, g_engine->_path[i + 1].x, g_engine->_path[i + 1].y, 0xFF);
		}
	}

	void View1::setStringBox(const Common::StringArray& sa) {
		_drawnStringBox = sa;
		_isShowingStringBox = true;
		_continueScriptAfterUI = true;

		// TODO: Change cursor, stop animations, hide again
		redraw();
	}

	void View1::clearStringBox() {
		_isShowingStringBox = false;
		currentSpeechActData.speaker = nullptr;
		redraw();
		if (_continueScriptAfterUI) {
			_continueScriptAfterUI = false;
			// TODO: Check which one it should be
			g_engine->RunScriptExecutor(false);
		}
		
	}

	int View1::GetCharacterArrayIndex(const Character *c) const {
		// TODO: Check if there is a find function somewhere
		for (int i = 0; i < characters.size(); i++) {
			if (characters[i] == c) {
				return i;
			}
		}
		return -1;
	}

	void View1::startFading() {
		currentFadeValue = 0x40;
	}

	bool View1::msgFocus(const FocusMessage &msg) {
	//Common::fill(&_pal[0], &_pal[256 * 3], 0);
	// _offset = 128;
	return true;
}

	bool View1::msgMouseDown(const MouseDownMessage& msg)
	{
		if (msg._button == MouseMessage::MB_LEFT) {
			// Handle string boxes
			if (_isShowingStringBox) {
				clearStringBox();
				return true;
			}

			if (_isShowingInventory) {
				// Check if we hit an item
				// TODO: Skipping this for now while we only have one item
				GameObject *clickedObject = getClickedInventoryItem(msg._pos);

				// TODO: Maybe handled better elsewhere - examining inventory items
				if (clickedObject != nullptr && g_engine->_scriptExecutor->_mouseMode == Script::MouseMode::Look) {
					// TODO: Does the scripting engine expect always the objects with the
					// right number prefix like here 419 instead of 19?
					g_engine->_scriptExecutor->_interactedObjectID = 0x400 + clickedObject->Index;
					g_engine->_scriptExecutor->_interactedOtherObjectID = 0;
					g_engine->RunScriptExecutor(false);
					return true;
				}
				if (activeInventoryItem != nullptr && clickedObject != nullptr) {
					// Trigger a use item on item
					GameObject *firstObject = activeInventoryItem;
					activeInventoryItem = nullptr;
					// TODO: Does the scripting engine expect always the objects with the
					// right number prefix like here 419 instead of 19?
					g_engine->_scriptExecutor->_interactedObjectID = 0x400 + firstObject->Index;
					g_engine->_scriptExecutor->_interactedOtherObjectID = 0x400 + clickedObject->Index;
					g_engine->RunScriptExecutor(false);
				}
				
				activeInventoryItem = clickedObject;
				return true;
			}

			// Handle no other interactions during a script
			if (g_engine->_scriptExecutor->IsExecuting()) {
				return true;
			}


			// uint32 value = getSurface().getPixel(msg._pos.x, msg._pos.y);
			uint32 value = g_engine->_map.getPixel(msg._pos.x, msg._pos.y);
			// g_system->setWindowCaption(Common::String::format("%u,%u: %u", msg._pos.x, msg._pos.y, value));
			//g_engine->CalculatePath(Common::Point(154, 136), Common::Point(msg._pos.x, msg._pos.y));

			if (g_engine->_scriptExecutor->_mouseMode == Script::MouseMode::Walk) {
				// TODO: Should address the protagonist differently
				// TODO: Sort out the different modes and only define them once
				characters[0]->StartLerpTo(msg._pos, 1000);
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
				return true;
			}

			g_engine->NextCursorMode();
			// TODO: Adjust for actual min value
			int mode = (int)g_engine->_scriptExecutor->_mouseMode - (int)Script::MouseMode::Talk;
			CursorMan.replaceCursor(g_engine->_cursorData[mode], g_engine->_cursorWidths[mode], g_engine->_cursorHeights[mode], g_engine->_cursorWidths[mode] >> 1, g_engine->_cursorHeights[0] >> 1, 0);
			return true;
		}
	}

	bool View1::msgMouseMove(const MouseMoveMessage &msg) {
		// TODO: Check what we are hovering over and save this info
		uint16 areaID = g_engine->_scriptExecutor->Func101D(msg._pos.x, msg._pos.y);
		// g_system->setWindowCaption(Common::String::format("Area ID: %.4x", areaID));
		return true;
	}

bool View1::msgKeypress(const KeypressMessage &msg) {
	// Any keypress to close the view
	// close();
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
		_backgroundSurface = g_engine->_depthMap;
		redraw();
	}
	if (msg.ascii == (uint16)'m') {
		// _backgroundSurface = g_engine->_map;
		_backgroundSurface = g_engine->_pathfindingMap;
		redraw();
	}
	else if (msg.ascii == (uint16)'b') {
		_backgroundSurface = g_engine->_bgImageShip;
		startFading();
		redraw();
	} else if (msg.ascii == (uint16)'s') {
		// g_engine->ExecuteScript(g_engine->_scriptStream);
		g_engine->RunScriptExecutor(true);
		// Also test the lerping
		characters[0]->StartLerpTo(Common::Point(200, 100), 5000);
	} else if (msg.ascii == (uint16)'i') {
		if (!_isShowingInventory) {
			SetInventorySource(GameObjects::instance().GetProtagonistObject());
		}
		_isShowingInventory = !_isShowingInventory;
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
		characters[0]->PathFinalDestination = mousePos;
		characters[0]->Path.clear();
		// g_engine->_path.clear();
		bool pathfindingResult = characters[0]->FindPath(mousePos);
		characters[0]->IsFollowingPath = pathfindingResult;
		characters[0]->CurrentPathIndex = -1;
		
	}
	return true;
}

void View1::draw() {
	g_system->getPaletteManager()->setPalette(g_engine->_pal, 0, 256);

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
	
	
	//for (int i = 0; i < 100; ++i)
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

	if (_isShowingInventory) {
		drawInventory(s);
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

	// Draw the position next to it
	renderString(mousePos.x + 20, mousePos.y + 20, Common::String::format("%u %u", mousePos.x, mousePos.y));
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
	//for (int i = 0; i < 256; ++i)
	//	_pal[i * 3 + 1] = (i + _offset) % 256;
	// g_system->getPaletteManager()->setPalette(_pal, 0, 256);

	// Below is redundant since we're only cycling the palette, but it demonstrates
	// how to trigger the view to do further draws after the first time, since views
	// don't automatically keep redrawing unless you tell it to
	//if ((_offset % 256) == 0)
	//	redraw();

	// Update the flag
	// TODO: Think about all these and compare other implementations, e.g. if we should rather update anims in draw
	// TODO: Consider wraparout
	uint32 tick_time = g_events->currentMillis;
	uint32 delta = tick_time - _lastMillis;
	_nextFrameFlag -= delta;

	// TODO: Consider the case of frame skipping
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

void View1::DrawSprite(int16 x, int16 y, uint16 width, uint16 height, byte* data, Graphics::ManagedSurface& s, bool mirrored, bool useDepth, uint8 depth)
{
	for (int currentX = 0; currentX < width; currentX++) {
		int actualX = mirrored ? width - currentX : currentX;
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

void View1::DrawSpriteClipped(uint16 x, uint16 y, Common::Rect &clippingRect, uint16 width, uint16 height, const byte * const data, Graphics::ManagedSurface &s) {
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

void View1::DrawSpriteAdvanced(uint16 x, uint16 y, uint16 width, uint16 height, uint16 scaling, byte *data, Graphics::ManagedSurface &s) {
	int xScaling = 0;
	int yScaling = 0;
	
	int currentTargetX = 0;
	int currentTargetY = 0;

	for (int currentSourceY = 0; currentSourceY < height; currentSourceY++) {
		currentTargetX = 0;
		for (int currentSourceX = 0; currentSourceX < width; currentSourceX++) {
			uint8 val = data[currentSourceY * width + currentSourceX];
			if (val != 0) {
				uint16 finalX = x + currentTargetX;
				uint16 finalY = y + currentTargetY;
				if (finalX >= 0 && finalX < s.w && finalY >= 0 && finalY < s.h)
					s.setPixel(x + currentTargetX, y + currentTargetY, val);
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
	int i = -1;
	for (auto current : characters) {
		int index = current->GameObject->Index;
		// TODO: Object 50h is a special one, it is the invisible object that moves along the
		// ground during the stick throw. Need to check how this is handled it the game
		// TODO: I'm kind of guessing that nr. 10 also is not visible, it does not appear
		// to have a lot of data to it. Random guess maybe this is the cup which is static?
		// TODO: Check what objects 17 and 18 and 23 in the machine room scene might be
		if (is_in_list<uint16, 0x50, 0x17, 0x18, 0x23>(index)) { // || index == 0x10) {
			continue;
		}
		AnimFrame* frame = current->GetCurrentAnimationFrame();
		bool mirror = current->isAnimationMirrored();
		
		// AnimFrame *frame = current->GetCurrentPortrait();
		uint8 depth = current->GetPosition().y;
		uint8 bgDepth = g_engine->_depthMap.getPixel(current->GetPosition().x, current->GetPosition().y);
		g_system->setWindowCaption(Common::String::format("Depth %u vs. %u", depth, bgDepth));

		DrawSprite(current->GetPosition() - frame->GetBottomMiddleOffset(), frame->Width, frame->Height, frame->Data, s, mirror, true, depth);
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
	_continueScriptAfterUI = true;

	currentSpeechActData.speaker = GetCharacterByIndex(characterIndex);
	currentSpeechActData.strings = strings;
	currentSpeechActData.position = position;
	currentSpeechActData.onRightSide = onRightSide;

	if (autoclickActive) {
		clearStringBox();
	}
}

void View1::DrawBorder(const Common::Point &pos, const Common::Point &size, Graphics::ManagedSurface &s) {
	// fn0037_A65D proc
	// TODO: Not sure what cmp	word ptr [2026h],1h does
	// Draw the background
	drawDarkRectangle(pos.x + 1, pos.y + 1, size.x - 1, size.y - 1);
	// TODO: Continue here

	// Left side
	DrawBorderSide(pos, Common::Point(6, size.y), s);

	// Right side
	// TODO: Check if we have the right offset on the right, I missed the part about adding
	// the width originally
	DrawBorderSide(pos + Common::Point(size.x - 6, 0), Common::Point(6, size.y), s);

	// Top side
	DrawBorderSide(pos, Common::Point(size.x, 6), s);

	// Bottom side
	DrawBorderSide(pos + Common::Point(0, size.y - 6), Common::Point(size.x, 6), s);

	// Add the function for filling a side of the border
	// Algorithm
	// Set up clipping rect on one side
	// Draw the texture enough times in x and y to fill the clipping rect

	// Draw the highlights
	// TODO: Check if positions are really correct
	// Top side
	DrawHorizontalBorderHighlight(pos + Common::Point(1, 1), size.x - 1, 0xFF, s);

	// TODO: Left side

	// Bottom highlight
	DrawHorizontalBorderHighlight(pos + Common::Point(1, size.y + 1), size.x - 1, 0xFF, s);

}

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

void View1::DrawHorizontalBorderHighlight(const Common::Point &pos, int16 width, uint8 unknown, Graphics::ManagedSurface &s) {

	// 0037:3AF5 (in fn0037_3AD4)

	// TODO: There is quite some setup going on in this function before we get to the drawing
	
	Common::Rect clippingRect(pos, pos + Common::Point(width, 1));
	// TODO: Should check which texture we actually use at the moment

	// TODO: Check which area we actually fill
	uint16 currentX = clippingRect.left;
	uint16 currentY = clippingRect.top;
	const Sprite &sprite = g_engine->_borderSprite;

	while (currentX < clippingRect.right) {
		DrawSpriteClipped(currentX, currentY, clippingRect, sprite, s);
		currentX += sprite.Width;
	}
}

void View1::ShowDialogueChoice(const Common::Array<Common::StringArray> &choices, const Common::Point &position, bool onRightSide) {
	Common::StringArray joinedLines;
	for (auto &currentLines : choices) {
		for (auto &currentLine : currentLines) {
			joinedLines.push_back(currentLine);
		}
	}

	ShowSpeechAct(1, joinedLines, position, onRightSide);
}

void View1::TriggerDialogueChoice(uint8 index) {
	// TODO: Confirm that these two are really set accordingly
	g_engine->_scriptExecutor->SetVariableValue(0x0d, index, 0);
	g_engine->_scriptExecutor->chosenDialogueOption = index;

	// TODO: Should check where this happens, but seems like we need to close the
	// options ourselves
	// TODO: Check if the run script after UI needs to be overridden here to not
	// schedule an unnecessary run
	clearStringBox();
	// TODO: Not sure about the first run variable here
	g_engine->RunScriptExecutor();
}

uint16 View1::GetHitObjectID(const Common::Point& pos) const {
	// TODO: Naive implementation for now
	for (auto currentCharacter : characters) {
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
	// TODO: Disabling it as it seems like I have it slightly off, it causes us to wrap around
	// when walking off the right of the screen in scene 11
	return false;
	// Read the map to find out if we moved into a non-walkable area
	// TODO: This is where the lerping will be off, since the game does this
	// every time it adjusts by one pixel
	// TODO: To check if the game actually moves by one pixel each frame only or
	// íf it has a loop to do more than one per frame
	if (c->GameObject->Index != 1) {
		// Other characters will always be able to walk normally
		return false;
	}
	if (g_engine->_scriptExecutor->IsExecuting()) {
		// We don't care for walkability
		// TODO: Probably set by some opcode in the game actually
		return false;
	}


	// TODO: For now, only handle walking into the left
	if (!IsWalkable(c->GetPosition())) {
		for (int deltaX = 0; deltaX != 20; deltaX++) {
			if (IsWalkable(Common::Point(c->GetPosition().x + deltaX, c->GetPosition().y))) {
				c->SetPosition(Common::Point(c->GetPosition().x + deltaX, c->GetPosition().y));
				return true;
			}
		}
	}

	return false;
}

uint8 Character::LookupWalkability(const Common::Point &p) const {
	Common::Rect screenRect(320, 200);
	if (!screenRect.contains(p)) {
		return 0x00;
	}
	uint32 value = g_engine->_pathfindingMap.getPixel(p.x, p.y);
	if (value < 0xC8 || value > 0xEF) {
		return value;
	}

	// Look up the value in the structure
	uint16 lookup = value + ((value << 1) << 1);
	// TODO: Handle lookup based on byte ptr es:[di+4EA5h]
	bool lookedUpValue = false;
	if (value == 0xCD) {
		// TODO: Hardcoded test case
		return 0x00;
	}
	if (value == 0xCB)
	{
		// TODO: Hardcoded test, for pathfinding should finally look up exactly how this works
		return 0x00;
	}
	if (!lookedUpValue) {
		return 0xFF;
	} else {
		// TODO: Look up based on es:[di+4EA6h]
		uint8 overrideValue = 0x00;
		return overrideValue;
	}
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

	// return true;
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
	for (int i = 0; i < currentPoint.adjacentPoints.size(); i++) {
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

bool Character::TryFollowPath() {
	CurrentPathIndex++;
	if (CurrentPathIndex == Path.size()) {
		// This means we now need to move to the final destination
		StartLerpTo(PathFinalDestination, 1000);
		return true;
	}
	if (CurrentPathIndex == Path.size() + 1) {
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
}



Macs2::AnimFrame *Character::GetCurrentAnimationFrame() {
	// We choose looking towards the screen first
	int blobIndex = GameObject->Orientation - 1;
	if (GameObject->testOverloadAnimation > -1) {
		blobIndex = GameObject->testOverloadAnimation;
	} else if (isAnimationMirrored()) {
		blobIndex = getMirroredAnimation(GameObject->Orientation) - 1;
		// blobIndex = GameObject->Orientation - 1 -
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
	uint16 offset = BackgroundAnimationBlob::Func1480(GameObject->Blobs[blobIndex],false, 0x0);
	// My remaining code expects to get dialed to the width and height directly - TODO make uniform
	offset += 6;
	AnimFrame *result = new AnimFrame();
	Common::MemoryReadStream stream(GameObject->Blobs[blobIndex].data(), GameObject->Blobs[blobIndex].size());
	stream.seek(offset);
	result->ReadFromStream(&stream);
	return result;
	// TODO: Think about proper memory management
}

Macs2::AnimFrame *Character::GetCurrentPortrait() {
	uint16 offset = BackgroundAnimationBlob::Func1480(GameObject->Blobs[17], true, 2);
	// My remaining code expects to get dialed to the width and height directly - TODO make uniform
	offset += 6;
	AnimFrame *result = new AnimFrame();
	Common::MemoryReadStream stream(GameObject->Blobs[17].data(), GameObject->Blobs[17].size());
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

void Character::StartLerpTo(const Common::Point &target, uint32 duration, bool ignoreObstacles) {
	StartPosition = GetPosition();
	EndPosition = target;
	float Angle = 
	StartTime = g_events->currentMillis;
	Duration = duration;
	IsLerping = true;
	LerpIgnoresObstacles = ignoreObstacles;

	// Calculate orientation
	Common::Point direction = EndPosition - StartPosition;
	Math::Vector2d directionVector = Math::Vector2d(direction.x, direction.y);
	directionVector.normalize();


	Math::Angle angle = directionVector.getAngle();
	// Rotate so that we start at the top and add half of 45 degrees so that we have
	// the right offset for the angles
	float degrees = angle.getDegrees() + 90.0f;
	if (degrees < 0.0f) {
		degrees += 360.0f;
	}
	float degreesAdjusted = degrees + 25.0f;
	if (degreesAdjusted > 360.0f) {
		degreesAdjusted -= 360.0f;
	}
	uint8 segment = degreesAdjusted / (360.0f / 8.0f);
	// TODO: Try out first which values we get
	Common::String message = Common::String::format("Degrees: %f Segment: %u", degrees, segment);
	debug(message.c_str());
	// Need to offset by one as the game handles the first one (straight away from
	// the camera) as index 1, not 0
	GameObject->Orientation = segment + 1;
}

void Character::StartPickup(Character *object) {
	objectToPickUp = object;
	ExecuteScriptOnFinishLerp = true;
	StartLerpTo(objectToPickUp->GetPosition(), 1000);
}

void Character::RegisterWaitForMovementFinishedEvent() {

	// For now, we are treating this one as a flag to send an event
	// even if we are not lerping, so that we have a delay between action 0x11
	// and the new execution
	ExecuteScriptOnFinishLerp = true;
}

void Character::Update() {
	if (!IsLerping && !IsFollowingPath) {
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
	uint32 endTime = StartTime + Duration;
	bool isDone = endTime < g_events->currentMillis;


	if (isDone) {
		if (IsFollowingPath) {
			// Set up a new lerp
			IsFollowingPath = TryFollowPath();
			if (IsFollowingPath) {
				return;
			}
		}


		IsLerping = false;
		// Go to the same orientation but standing
		GameObject->Orientation += 8;

		// Check if we need to pick something up
		if (objectToPickUp != nullptr) {

			View1 *currentView = (View1 *)g_engine->findView("View1");
			int index = currentView->GetCharacterArrayIndex(objectToPickUp);
			currentView->inventoryItems.push_back(objectToPickUp->GameObject);
			currentView->characters.remove_at(index);
			// Give it to the protagonist
			objectToPickUp->GameObject->SceneIndex = 1;
			objectToPickUp = nullptr;
			// From here on the interacted object should become 0
			g_engine->_scriptExecutor->_interactedObjectID = 0x0000;
		}


		// Check if we need to execute the script
		// TODO: Consider which run function to use
		if (ExecuteScriptOnFinishLerp) {
			ExecuteScriptOnFinishLerp = false;
			g_engine->_scriptExecutor->global1032 = true;
			g_engine->ScheduleRun();
		}

		// TODO: This muddles the logic a bit since I also have the executescriptonfinish,
		// should be unified. The game code sets the flag [1020] to true when a move is finished,
		// which in turn unlocks running the script function
		// TODO: Should look at game code how exactly it is handled there
		if (!g_engine->_scriptExecutor->IsExecuting()) {
			g_engine->_scriptExecutor->Rewind();
			// TODO: Get rid of the different copies of the position
			View1 *currentView = (View1 *)g_engine->findView("View1");
			g_engine->ScheduleRun();
		}
		return;
	}

	float progress = (float) (g_events->currentMillis - StartTime) / (float) Duration;
	SetPosition(StartPosition + (EndPosition - StartPosition) * progress);
	if (!LerpIgnoresObstacles && HandleWalkability(this)) {
		IsLerping = false;
		// Go the the same orientation but standing
		GameObject->Orientation += 8;
		// TODO: Copy & paste code
		if (!g_engine->_scriptExecutor->IsExecuting()) {
			g_engine->_scriptExecutor->Rewind();
			// TODO: Get rid of the different copies of the position
			// TODO: Not sure if we should set g_engine->_scriptExecutor->global1032 = false;
			View1 *currentView = (View1 *)g_engine->findView("View1");
			g_engine->ScheduleRun();
		}
	}
}

bool Button::IsPointInside(const Common::Point &p) const {
	return false;
}

void Button::Render(Graphics::ManagedSurface &s) {
}

} // namespace Macs2
