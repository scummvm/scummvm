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

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/file.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/framelimiter.h"
#include "graphics/paletteman.h"
#include "image/pcx.h"
#include "image/png.h"

#include "pelrock.h"
#include "pelrock/console.h"
#include "pelrock/detection.h"
#include "pelrock/fonts/small_font.h"
#include "pelrock/offsets.h"
#include "pelrock/pelrock.h"
#include "pelrock/util.h"

namespace Pelrock {

PelrockEngine *g_engine;

PelrockEngine::PelrockEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
																				 _gameDescription(gameDesc), _randomSource("Pelrock") {
	g_engine = this;
	_chronoManager = new ChronoManager();
}

PelrockEngine::~PelrockEngine() {
	delete[] _compositeBuffer;
	delete[] _currentBackground;
	delete _largeFont;
	delete _screen;
	delete _chronoManager;
	delete _videoManager;
	delete _soundManager;
	delete _room;
	delete _res;
	// if (_bgPopupBalloon)
	// 	delete[] _bgPopupBalloon;
	delete _smallFont;
}

uint32 PelrockEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String PelrockEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Array<Common::Array<Common::String>> wordWrap(Common::String text);

Common::Error PelrockEngine::run() {
	// Initialize 320x200 paletted graphics mode
	initGraphics(640, 400);
	_screen = new Graphics::Screen();
	_videoManager = new VideoManager(_screen);
	_room = new RoomManager();
	_res = new ResourceManager();
	_soundManager = new SoundManager(_mixer);

	// Set the engine's debugger console
	setDebugger(new PelrockConsole(this));

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	// Simple event handling loop
	Common::Event e;
	Graphics::FrameLimiter limiter(g_system, 60);

	if (shouldPlayIntro == false) {
		stateGame = GAME;
	} else {
		stateGame = INTRO;
		_videoManager->playIntro();
		stateGame = GAME;
	}
	init();

	while (!shouldQuit()) {
		_chronoManager->updateChrono();
		while (g_system->getEventManager()->pollEvent(e)) {
			if (e.type == Common::EVENT_KEYDOWN) {
				switch (e.kbd.keycode) {
				case Common::KEYCODE_w:
					alfredState.animState = ALFRED_WALKING;
					break;
				case Common::KEYCODE_t:
					alfredState.animState = ALFRED_TALKING;
					break;
				case Common::KEYCODE_s:
					alfredState.animState = ALFRED_IDLE;
					break;
				case Common::KEYCODE_c:
					alfredState.animState = ALFRED_COMB;
					break;
				case Common::KEYCODE_i:
					alfredState.animState = ALFRED_INTERACTING;
					break;
				case Common::KEYCODE_z:
					showShadows = !showShadows;
					break;
				case Common::KEYCODE_y:
					alfredState.x = 193;
					alfredState.y = 382;
					walkTo(377, 318);
					break;
				default:
					break;
				}
			} else if (e.type == Common::EVENT_MOUSEMOVE) {
				mouseX = e.mouse.x;
				mouseY = e.mouse.y;
				// debug(3, "Mouse moved to (%d,%d)", mouseX, mouseY);
			} else if (e.type == Common::EVENT_LBUTTONDOWN) {
				if (!_isMouseDown) {
					_mouseClickTime = g_system->getMillis();
					_isMouseDown = true;
				}
			} else if (e.type == Common::EVENT_LBUTTONUP) {
				_isMouseDown = false;
				checkMouseClick(e.mouse.x, e.mouse.y);
				_displayPopup = false;
				_longClick = false;
			} else if (e.type == Common::EVENT_RBUTTONUP) {
				if (stateGame != SETTINGS)
					stateGame = SETTINGS;
				else {
					g_system->getPaletteManager()->setPalette(_room->_roomPalette, 0, 256);
					stateGame = GAME;
				}
			}
		}
		if (_isMouseDown) {
			if (g_system->getMillis() - _mouseClickTime >= kLongClickDuration) {
				debug("long click!");
				_longClick = true;
				_isMouseDown = false;
				checkLongMouseClick(e.mouse.x, e.mouse.y);
			}
		}
		checkMouseHover();
		if (stateGame == SETTINGS) {

			memcpy(_screen->getPixels(), _res->_mainMenu, 640 * 400);
			g_system->getPaletteManager()->setPalette(_res->_mainMenuPalette, 0, 256);
			g_engine->_screen->markAllDirty();
			g_engine->_screen->update();

		} else if (stateGame == GAME) {
			frames();
		}

		_screen->update();
		// limiter.delayBeforeSwap();
		// limiter.startFrame();
	}

	return Common::kNoError;
}

void PelrockEngine::init() {
	_res->loadCursors();
	_res->loadInteractionIcons();
	_res->loadSettingsMenu();
	_soundManager->loadSoundIndex();

	calculateScalingMasks();
	_compositeBuffer = new byte[640 * 400];
	_currentBackground = new byte[640 * 400];

	_smallFont = new SmallFont();
	_smallFont->load("ALFRED.4");
	_largeFont = new LargeFont();
	_largeFont->load("ALFRED.7");

	changeCursor(DEFAULT);
	CursorMan.showMouse(true);

	if (gameInitialized == false) {
		gameInitialized = true;
		loadAnims();
		setScreen(0, ALFRED_DOWN);
		// setScreen(6, 0); // museum entrance
		// setScreen(13, 1); // restaurants kitchen
		// setScreen(2, 2); // hooker
	}
}

void PelrockEngine::loadAnims() {
	_res->loadAlfredAnims();
}

void PelrockEngine::displayChoices(Common::Array<Common::String> choices, byte *compositeBuffer) {
	int overlayHeight = choices.size() * kChoiceHeight + 2;
	int overlayY = 400 - overlayHeight;
	for (int x = 0; x < 640; x++) {
		for (int y = overlayY; y < 400; y++) {
			int index = y * 640 + x;
			compositeBuffer[index] = _room->overlayRemap[compositeBuffer[index]];
		}
	}
	for (int i = 0; i < choices.size(); i++) {
		drawText(choices[i], 10, overlayY + 2 + i * kChoiceHeight, 620, 15);
	}
}

byte *PelrockEngine::grabBackgroundSlice(int x, int y, int w, int h) {
	byte *bg = new byte[w * h];
	for (int j = 0; j < w; j++) {
		for (int i = 0; i < h; i++) {
			int idx = i * w + j;
			if (y + i < 400 && x + j < 640) {
				*(bg + idx) = _currentBackground[(y + i) * 640 + (x + j)];
			}
		}
	}
	return bg;
}

void PelrockEngine::putBackgroundSlice(int x, int y, int w, int h, byte *slice) {
	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {
			int index = (j * w + i);
			if (x + i < 640 && y + j < 400)
				*(byte *)g_engine->_screen->getBasePtr(x + i, y + j) = slice[index];
		}
	}
}

Common::Array<VerbIcon> PelrockEngine::availableActions(HotSpot *hotspot) {
	if (hotspot == nullptr) {
		return Common::Array<VerbIcon>();
	}
	Common::Array<VerbIcon> verbs;
	verbs.push_back(LOOK);

	if (hotspot->type & 1) {
		verbs.push_back(OPEN);
	}
	if (hotspot->type & 2) {
		verbs.push_back(CLOSE);
	}
	if (hotspot->type & 4) {
		verbs.push_back(UNKNOWN);
	}
	if (hotspot->type & 8) {
		verbs.push_back(PICKUP);
	}
	if (hotspot->type & 16) {
		verbs.push_back(TALK);
	}
	if (hotspot->type & 32) {
		verbs.push_back(PUSH);
	}
	if (hotspot->type & 128) {
		verbs.push_back(PULL);
	}
	return verbs;
}

// Sort sprites by zOrder in-place using insertion sort (efficient for nearly-sorted data)
void sortAnimsByZOrder(Common::Array<Sprite> &anims) {
	for (size_t i = 1; i < anims.size(); ++i) {
		Sprite key = anims[i];
		int z = key.zOrder;
		size_t j = i;
		while (j > 0 && anims[j - 1].zOrder > z) {
			anims[j] = anims[j - 1];
			--j;
		}
		anims[j] = key;
	}
}

void PelrockEngine::frames() {

	if (_chronoManager->_gameTick) {

		int soundIndex = _soundManager->tick();
		if (soundIndex >= 0 && soundIndex < _room->_roomSfx.size()) {
			// debug("Playing SFX index %d", soundIndex);
			_soundManager->playSound(_room->_roomSfx[3 + soundIndex]);
		}

		// Sort sprites by zOrder (persists in the array)
		sortAnimsByZOrder(_room->_currentRoomAnims);

		memcpy(_compositeBuffer, _currentBackground, 640 * 400);

		// Create temporary render order partitioned by Alfred's Y position
		Common::Array<Sprite *> renderOrder;
		int alfredY = alfredState.y;

		// First pass: sprites behind Alfred (y <= alfredY)
		for (int i = 0; i < _room->_currentRoomAnims.size(); i++) {
			if (_room->_currentRoomAnims[i].y < (alfredY - kAlfredFrameHeight)) {
				debug("Drawing sprite %d in front of Alfred at zOrder %d, pos (%d, %d)", i, _room->_currentRoomAnims[i].zOrder, _room->_currentRoomAnims[i].x, _room->_currentRoomAnims[i].y);

				// renderOrder.push_back(&_room->_currentRoomAnims[i]);
				drawNextFrame(&_room->_currentRoomAnims[i]);
			}
		}

		// Draw Alfred here (you'll need to add this)
		chooseAlfredStateAndDraw();

		// Second pass: sprites in front of Alfred (y > alfredY)
		for (int i = 0; i < _room->_currentRoomAnims.size(); i++) {
			if (_room->_currentRoomAnims[i].y > (alfredY - kAlfredFrameHeight)) {
				debug("Drawing sprite %d behind Alfred at zOrder %d, pos (%d, %d)", i, _room->_currentRoomAnims[i].zOrder, _room->_currentRoomAnims[i].x, _room->_currentRoomAnims[i].y);
				drawNextFrame(&_room->_currentRoomAnims[i]);
				// renderOrder.push_back(&_room->_currentRoomAnims[i]);
			}
		}

		// // Render in the computed order
		// for (int i = 0; i < renderOrder.size(); i++) {
		// 	drawNextFrame(renderOrder[i]);
		// }

		if (_displayPopup) {
			showActionBalloon(_popupX, _popupY, _currentPopupFrame);
			if (_currentPopupFrame < 3) {
				_currentPopupFrame++;
			} else
				_currentPopupFrame = 0;
		}

		Common::Array<Common::String> testChoices;
		testChoices.push_back("First choice");
		testChoices.push_back("Second choice");
		testChoices.push_back("Third choice");
		displayChoices(testChoices, _compositeBuffer);

		memcpy(_screen->getPixels(), _compositeBuffer, 640 * 400);

		if (alfredState.animState != ALFRED_WALKING && !_currentTextPages.empty()) {
			if (_chronoManager->_textTtl > 0) {
				if (alfredState.animState == ALFRED_TALKING) {
					_textPos = Common::Point(alfredState.x, alfredState.y - kAlfredFrameHeight - 10);
				}
				renderText(_currentTextPages[_currentTextPageIndex], _textColor, _textPos.x, _textPos.y);
			} else if (_currentTextPageIndex < _currentTextPages.size() - 1) {
				_currentTextPageIndex++;

				int totalChars = 0;
				for (int i = 0; i < _currentTextPages[_currentTextPageIndex].size(); i++) {
					totalChars += _currentTextPages[_currentTextPageIndex][i].size();
				}
				_chronoManager->_textTtl = totalChars * kTextCharDisplayTime;
			} else {
				_currentTextPages.clear();
				_currentTextPageIndex = 0;
				alfredState.animState = ALFRED_IDLE;
				isNPCATalking = false;
				isNPCBTalking = false;
			}
		}

		if (alfredState.animState == ALFRED_IDLE && alfredState.nextState != ALFRED_IDLE) {
			// debug("Switching Alfred state from IDLE to %d", alfredState.nextState);
			alfredState.animState = alfredState.nextState;
			alfredState.nextState = ALFRED_IDLE;
			alfredState.curFrame = 0;
		}

		// debug("Drawing walkboxes..., %d, _currentRoomWalkboxes.size()=%d",  _currentRoomWalkboxes.size(), _currentRoomWalkboxes.size());
		for (int i = 0; i < _room->_currentRoomWalkboxes.size(); i++) {
			// debug("Drawing walkbox %d", i);
			WalkBox box = _room->_currentRoomWalkboxes[i];
			drawRect(_screen, box.x, box.y, box.w, box.h, 150 + i);
			_smallFont->drawString(_screen, Common::String::format("%d", i), box.x + 2, box.y + 2, 640, 14);
		}

		drawPos(_screen, alfredState.x, alfredState.y, 13);
		drawPos(_screen, alfredState.x, alfredState.y - kAlfredFrameHeight, 13);
		drawPos(_screen, _curWalkTarget.x, _curWalkTarget.y, 100);

		if (showShadows) {
			memcpy(_screen->getPixels(), _room->_pixelsShadows, 640 * 400);
		}
		_smallFont->drawString(_screen, Common::String::format("Room number: %d", _room->_currentRoomNumber), 0, 4, 640, 13);
		_smallFont->drawString(_screen, Common::String::format("Alfred pos: %d, %d (%d)", alfredState.x, alfredState.y, alfredState.y - kAlfredFrameHeight), 0, 18, 640, 13);
		_screen->markAllDirty();

		// _screen->update();
	}
}

void PelrockEngine::doAction(byte action, HotSpot *hotspot) {
	switch (action) {
	case LOOK:
		lookAtHotspot(_currentHotspot);
		break;
	case TALK:
		talkTo(_currentHotspot);
		break;
	default:
		break;
	}
}

void PelrockEngine::talkTo(HotSpot *hotspot) {
	debug("Talking to object %d", hotspot->index);
	if (_room->_currentRoomConversations.size() == 0)
		return;

	Sprite *animSet;
	for (int i = 0; i < _room->_currentRoomAnims.size(); i++) {
		if (i == hotspot->index) {
			animSet = &_room->_currentRoomAnims[i];
		}
	}

	ConversationNode selectedNode = _room->_currentRoomConversations[0];

	bool isNPC = selectedNode.speakerId != 13;
	if (isNPC) {
		sayNPC(animSet, selectedNode.text, selectedNode.speakerId);
	}
	// for(int i= 0; i< _currentRoomConversations.size(); i++) {
	// _currentRoomConversations
	// }

	// showDescription(_currentRoomConversations[0].text, x, y, _currentRoomConversations[0].speakerId);
	// for(int i = 0; i < _currentRoomConversations[0].choices.size(); i++) {
	// 	int idx = _currentRoomConversations.size() - 1 - i;
	// 	_smallFont->drawString(_screen, _currentRoomConversations[0].choices[idx].text.c_str(), 0, 400 - ((i + 1) * 12), 640, 14);
	// }
}

void PelrockEngine::lookAtHotspot(HotSpot *hotspot) {
	debug("Look action clicked");
	walkTo(_currentHotspot->x, _currentHotspot->y);
	sayAlfred(_room->_currentRoomDescriptions[_currentHotspot->index].text);
	_displayPopup = false;
}

void PelrockEngine::renderText(Common::Array<Common::String> lines, int color, int baseX, int baseY) {

	int maxW = 0;
	for (size_t i = 0; i < lines.size(); i++) {
		Common::Rect r = _largeFont->getBoundingBox(lines[i]);
		if (r.width() > maxW) {
			maxW = r.width();
		}
	}
	int lineSize = lines.size();
	for (size_t i = 0; i < lines.size(); i++) {
		int textX = baseX - (maxW / 2);
		int textY = baseY - (lineSize * 25) + (i * 25);
		drawText(lines[i], textX, textY, maxW, color);
	}
}

void PelrockEngine::chooseAlfredStateAndDraw() {
	switch (alfredState.animState) {
	case ALFRED_WALKING: {

		MovementStep step = _currentContext.movementBuffer[_currentStep];

		if (step.distance_x > 0) {
			if (step.flags & MOVE_RIGHT) {
				alfredState.direction = ALFRED_RIGHT;
				alfredState.x += MIN(alfredState.movementSpeed, step.distance_x);
			}
			if (step.flags & MOVE_LEFT) {
				alfredState.direction = ALFRED_LEFT;
				alfredState.x -= MIN(alfredState.movementSpeed, step.distance_x);
			}
		}
		if (step.distance_y > 0) {
			if (step.flags & MOVE_DOWN) {
				alfredState.direction = ALFRED_DOWN;
				alfredState.y += MIN(alfredState.movementSpeed, step.distance_y);
			}
			if (step.flags & MOVE_UP) {
				alfredState.direction = ALFRED_UP;
				alfredState.y -= MIN(alfredState.movementSpeed, step.distance_y);
			}
		}

		if (step.distance_x > 0)
			step.distance_x -= MIN(alfredState.movementSpeed, step.distance_x);

		if (step.distance_y > 0)
			step.distance_y -= MIN(alfredState.movementSpeed, step.distance_y);

		if (step.distance_x <= 0 && step.distance_y <= 0) {
			_currentStep++;
			if (_currentStep >= _currentContext.movementCount) {
				_currentStep = 0;
				alfredState.animState = ALFRED_IDLE;
			}
		} else {
			_currentContext.movementBuffer[_currentStep] = step;
		}

		Exit *exit = isExitUnder(alfredState.x, alfredState.y);

		if (exit != nullptr) {
			alfredState.x = exit->targetX;
			alfredState.y = exit->targetY;
			setScreen(exit->targetRoom, exit->dir);
		}

		if (alfredState.curFrame >= walkingAnimLengths[alfredState.direction]) {
			alfredState.curFrame = 0;
		}

		drawAlfred(_res->alfredWalkFrames[alfredState.direction][alfredState.curFrame]);
		// if(alfredFrameSkip) alfredState.curFrame++;
		// alfredFrameSkip = !alfredFrameSkip;
		alfredState.curFrame++;
		break;
	}
	case ALFRED_TALKING:
		if (alfredState.curFrame >= talkingAnimLengths[alfredState.direction] - 1) {
			alfredState.curFrame = 0;
		}
		drawAlfred(_res->alfredTalkFrames[alfredState.direction][alfredState.curFrame]);
		alfredState.curFrame++;
		break;
	case ALFRED_COMB:
		if (alfredState.curFrame >= 11) {
			alfredState.curFrame = 0;
		}
		drawSpriteToBuffer(_compositeBuffer, 640, _res->alfredCombFrames[0][alfredState.curFrame], alfredState.x, alfredState.y - kAlfredFrameHeight, 51, 102, 255);
		alfredState.curFrame++;
		break;
	case ALFRED_INTERACTING:
		if (alfredState.curFrame >= interactingAnimLength) {
			alfredState.curFrame = 0;
		}
		drawAlfred(_res->alfredInteractFrames[alfredState.direction][alfredState.curFrame]);
		alfredState.curFrame++;
		break;
	default:
		drawAlfred(_res->alfredIdle[alfredState.direction]);
		break;
	}
}

void PelrockEngine::drawAlfred(byte *buf) {

	ScaleCalculation scale = calculateScaling(alfredState.y, _room->_scaleParams);

	int finalHeight = kAlfredFrameHeight - scale.scaleDown + scale.scaleUp;
	if (finalHeight <= 0) {
		finalHeight = 1;
	}
	float scaleFactor = static_cast<float>(finalHeight) / static_cast<float>(kAlfredFrameHeight);
	int finalWidth = static_cast<int>(kAlfredFrameWidth * scaleFactor);
	if (finalWidth <= 0) {
		finalWidth = 1;
	}
	int scaleIndex = finalHeight - 1;
	if (scaleIndex >= _heightScalingTable.size()) {
		scaleIndex = _heightScalingTable.size() - 1;
	}
	if (scaleIndex < 0) {
		scaleIndex = 0;
	}
	// debug("Scaling Alfred frame to final size (%d x %d) from scale factor %.2f", finalWidth, finalHeight, scaleFactor);
	int linesToSkip = kAlfredFrameHeight - finalHeight;

	// debug("lines to skip = %d, finalHeight = %d, finalWidth = %d for position (%d, %d)", linesToSkip, finalHeight, finalWidth, xAlfred, yAlfred);

	int shadowPos = alfredState.y; // - finalHeight;
	bool shadeCharacter = _room->_pixelsShadows[shadowPos * 640 + alfredState.x] != 0xFF;

	byte *finalBuf = new byte[finalWidth * finalHeight];

	if (linesToSkip > 0) {
		int skipInterval = kAlfredFrameHeight / linesToSkip;
		Common::Array<float> idealSkipPositions;
		for (int i = 0; i < linesToSkip; i++) {
			float idealPos = (i + 0.5f) * skipInterval;
			idealSkipPositions.push_back(idealPos);
		}

		// debug("Height scaling table size =%d", _heightScalingTable.size());
		Common::Array<int> tableSkipPositions;
		for (int scanline = 0; scanline < kAlfredFrameHeight; scanline++) {
			if (_heightScalingTable[scaleIndex][scanline] != 0) {
				tableSkipPositions.push_back(scanline);
			}
		}

		// debug("Table skip positions:");
		// for (size_t i = 0; i < tableSkipPositions.size(); i++) {
		// 	debug("  %d", tableSkipPositions[i]);
		// }

		Common::Array<int> skipTheseLines;
		for (size_t i = 0; i < idealSkipPositions.size(); i++) {
			float idealPos = idealSkipPositions[i];
			int closest = -1;
			int minDiff = INT32_MAX;
			for (size_t j = 0; j < tableSkipPositions.size(); j++) {
				int candidate = tableSkipPositions[j];
				int diff = static_cast<int>(abs(candidate - idealPos));
				if (diff < minDiff) {
					minDiff = diff;
					closest = candidate;
				}
			}
			if (closest != -1) {
				skipTheseLines.push_back(closest);
			}
			if (skipTheseLines.size() >= static_cast<size_t>(linesToSkip)) {
				break;
			}
		}

		int outY = 0;
		for (int srcY = 0; srcY < kAlfredFrameHeight; srcY++) {
			bool skipLine = false;
			for (size_t skipIdx = 0; skipIdx < skipTheseLines.size(); ++skipIdx) {
				if (skipTheseLines[skipIdx] == srcY) {
					skipLine = true;
					break;
				}
			}
			if (!skipLine) {
				for (int outX = 0; outX < finalWidth; outX++) {
					int srcX = static_cast<int>(outX * kAlfredFrameWidth / finalWidth);
					if (srcX >= kAlfredFrameWidth) {
						srcX = kAlfredFrameWidth - 1;
					}
					int srcIndex = srcY * kAlfredFrameWidth + srcX;
					int outIndex = outY * finalWidth + outX;
					finalBuf[outIndex] = buf[srcIndex];
				}
				outY++;
			}
		}
	} else {
		Common::copy(buf, buf + (kAlfredFrameWidth * kAlfredFrameHeight), finalBuf);
	}

	if (shadeCharacter) {
		for (int i = 0; i < finalWidth * finalHeight; i++) {
			if (finalBuf[i] != 255) {
				finalBuf[i] = _room->alfredRemap[finalBuf[i]];
			}
		}
	}

	drawSpriteToBuffer(_compositeBuffer, 640, finalBuf, alfredState.x, alfredState.y - finalHeight, finalWidth, finalHeight, 255);
	delete[] finalBuf;
}

void applyMovement(int16_t *x, int16_t *y, /*int8_t *z,*/ uint16_t flags) {
	// X-axis movement
	if (flags & 0x10) {            // Bit 4: X movement enabled
		int amount = flags & 0x07; // Bits 0-2: pixels per frame
		if (flags & 0x08) {        // Bit 3: direction
			*x += amount;          // 1 = right (add)
		} else {
			*x -= amount; // 0 = left (subtract)
		}
	}

	// Y-axis movement
	if (flags & 0x200) {                  // Bit 9: Y movement enabled
		int amount = (flags >> 5) & 0x07; // Bits 5-7: pixels per frame
		if (flags & 0x100) {              // Bit 8: direction
			*y += amount;                 // 1 = down (add)
		} else {
			*y -= amount; // 0 = up (subtract)
		}
	}

	// // Z-axis movement
	// if (flags & 0x4000) {  // Bit 14: Z movement enabled
	//     int amount = (flags >> 10) & 0x07;  // Bits 10-12: amount
	//     if (flags & 0x2000) {  // Bit 13: direction
	//         *z += amount;   // 1 = forward (add)
	//     } else {
	//         *z -= amount;   // 0 = back (subtract)
	//     }
	// }
}

void PelrockEngine::drawNextFrame(Sprite *sprite) {
	Anim &animData = sprite->animData[sprite->curAnimIndex];
	if (sprite->zOrder == -1) {
		// skips z0rder -1 sprites
		return;
	}

	applyMovement(&(sprite->x), &(sprite->y), animData.movementFlags);
	int x = sprite->x;
	int y = sprite->y;
	int w = animData.w;
	int h = animData.h;
	int extra = sprite->extra;
	if (whichNPCTalking == extra) {
		drawTalkNPC(sprite);
		return;
	}

	int frameSize = animData.w * animData.h;
	int curFrame = animData.curFrame;
	byte *frame = new byte[frameSize];
	extractSingleFrame(animData.animData, frame, curFrame, animData.w, animData.h);

	drawSpriteToBuffer(_compositeBuffer, 640, frame, sprite->x, sprite->y, sprite->w, sprite->h, 255);

	if (animData.elpapsedFrames == animData.speed) {
		animData.elpapsedFrames = 0;
		if (animData.curFrame < animData.nframes - 1) {
			animData.curFrame++;
		} else {
			if (animData.curLoop < animData.loopCount - 1) {
				animData.curFrame = 0;
				animData.curLoop++;
			} else {
				animData.curFrame = 0;
				animData.curLoop = 0;
				if (sprite->curAnimIndex < sprite->numAnims - 1) {
					sprite->curAnimIndex++;
				} else {
					sprite->curAnimIndex = 0;
				}
			}
		}
	} else {
		animData.elpapsedFrames++;
	}
}

void PelrockEngine::checkLongMouseClick(int x, int y) {
	int hotspotIndex = isHotspotUnder(mouseX, mouseY);
	if (hotspotIndex != -1) {

		_popupX = x - kBalloonWidth / 2;
		if (_popupX < 0)
			_popupX = 0;
		if (_popupX + kBalloonWidth > 640) {
			_popupX -= 640 - (_popupX + kBalloonWidth);
		}

		_popupY = y - kBalloonHeight;
		if (_popupY < 0) {
			_popupY = 0;
		}
		_displayPopup = true;
		_currentPopupFrame = 0;
		_currentHotspot = &_room->_currentRoomHotspots[hotspotIndex];
		debug("Current hotspot type: %d", _currentHotspot->type);
	}
}

void PelrockEngine::calculateScalingMasks() {

	//    for scale_factor in range(CHAR_WIDTH):
	//     step = CHAR_WIDTH / (scale_factor + 1.0)
	//     row = []
	//     index = 0.0
	//     source_pixel = 0

	//     while index < CHAR_WIDTH:
	//         row.append(source_pixel)
	//         index += step
	//         source_pixel += 1
	//         if source_pixel >= CHAR_WIDTH:
	//             source_pixel = CHAR_WIDTH - 1

	//     # Pad to exactly CHAR_WIDTH entries
	//     while len(row) < CHAR_WIDTH:
	//         row.append(row[-1] if row else 0)
	//     width_table.append(row[:CHAR_WIDTH])

	for (int scaleFactor = 0; scaleFactor < kAlfredFrameWidth; scaleFactor++) {
		float step = kAlfredFrameWidth / (scaleFactor + 1.0f);
		Common::Array<int> row;
		float index = 0.0f;
		int sourcePixel = 0;

		while (index < kAlfredFrameWidth) {
			row.push_back(sourcePixel);
			index += step;
			sourcePixel += 1;
			if (sourcePixel >= kAlfredFrameWidth) {
				sourcePixel = kAlfredFrameWidth - 1;
			}
		}

		// Pad to exactly CHAR_WIDTH entries
		while (row.size() < kAlfredFrameWidth) {
			row.push_back(row.empty() ? 0 : row[row.size() - 1]);
		}

		_widthScalingTable.push_back(row);
	}

	//  height_table = []
	// for scale_factor in range(CHAR_HEIGHT):
	//     step = CHAR_HEIGHT / (scale_factor + 1.0)
	//     row = [0] * CHAR_HEIGHT  # Initialize all to 0

	//     # Mark positions where we should keep/duplicate the scanline
	//     position = step
	//     counter = 1
	//     while position < CHAR_HEIGHT:
	//         idx = round(position)
	//         if idx < CHAR_HEIGHT:
	//             row[idx] = counter
	//             counter += 1
	//         position += step

	//     height_table.append(row)
	for (int scaleFactor = 0; scaleFactor < kAlfredFrameHeight; scaleFactor++) {
		float step = kAlfredFrameHeight / (scaleFactor + 1.0f);
		Common::Array<int> row;
		row.resize(kAlfredFrameHeight, 0);
		float position = step;
		int counter = 1;
		while (position < kAlfredFrameHeight) {
			int idx = static_cast<int>(round(position));
			if (idx < kAlfredFrameHeight) {
				row[idx] = counter;
				counter++;
			}
			position += step;
		}
		_heightScalingTable.push_back(row);
	}
}

ScaleCalculation PelrockEngine::calculateScaling(int yPos, ScalingParams scalingParams) {
	int scaleDown = 0;
	int scaleUp = 0;
	if (scalingParams.scaleMode == 0xFF) {
		scaleDown = 0x5e;
		scaleUp = 0x2f;
	} else if (scalingParams.scaleMode == 0xFE) {
		scaleDown = 0;
		scaleUp = 0;
	} else if (scalingParams.scaleMode == 0) {
		if (scalingParams.yThreshold < yPos) {
			scaleDown = 0;
			scaleUp = 0;
		} else {
			if (scalingParams.scaleDivisor != 0) {
				scaleDown = (scalingParams.yThreshold - yPos) / scalingParams.scaleDivisor;
				scaleUp = scaleDown / 2;
			} else {
				scaleDown = 0;
				scaleUp = 0;
			}
		}
	} else {
		scaleDown = 0;
		scaleUp = 0;
	}

	int finalHeight = kAlfredFrameHeight - scaleDown + scaleUp;
	if (finalHeight < 1)
		finalHeight = 1;

	int finalWidth = kAlfredFrameWidth * (finalHeight / kAlfredFrameHeight);
	if (finalWidth < 1)
		finalWidth = 1;

	ScaleCalculation scaleCalc;
	scaleCalc.scaledHeight = finalHeight;
	scaleCalc.scaledWidth = finalWidth;
	scaleCalc.scaleDown = scaleDown;
	scaleCalc.scaleUp = scaleUp;
	return scaleCalc;
}

int PelrockEngine::isHotspotUnder(int x, int y) {

	for (size_t i = 0; i < _room->_currentRoomHotspots.size(); i++) {
		HotSpot hotspot = _room->_currentRoomHotspots[i];
		if (hotspot.isEnabled &&
			mouseX >= hotspot.x && mouseX <= (hotspot.x + hotspot.w) &&
			mouseY >= hotspot.y && mouseY <= (hotspot.y + hotspot.h)) {
			return i;
		}
	}
	return -1;
}

Exit *PelrockEngine::isExitUnder(int x, int y) {
	for (int i = 0; i < _room->_currentRoomExits.size(); i++) {
		Exit exit = _room->_currentRoomExits[i];
		if (x >= exit.x && x <= (exit.x + exit.w) &&
			y >= exit.y && y <= (exit.y + exit.h)) {
			return &(_room->_currentRoomExits[i]);
		}
	}
	return nullptr;
}

void PelrockEngine::showActionBalloon(int posx, int posy, int curFrame) {

	drawSpriteToBuffer(_compositeBuffer, 640, _res->_popUpBalloon + (curFrame * kBalloonHeight * kBalloonWidth), posx, posy, kBalloonWidth, kBalloonHeight, 255);
	Common::Array<VerbIcon> actions = availableActions(_currentHotspot);
	VerbIcon icon = isActionUnder(mouseX, mouseY);
	for (int i = 0; i < actions.size(); i++) {
		if (icon == actions[i] && _iconBlink++ < kIconBlinkPeriod / 2) {
			continue;
		}
		if (_iconBlink > kIconBlinkPeriod) {
			_iconBlink = 0;
		}
		drawSpriteToBuffer(_compositeBuffer, 640, _res->_verbIcons[actions[i]], posx + 20 + (i * (kVerbIconWidth + 2)), posy + 20, kVerbIconWidth, kVerbIconHeight, 1);
	}
}

void PelrockEngine::drawTalkNPC(Sprite *animSet) {
	// Change with the right index

	int index = animSet->index;
	TalkingAnimHeader *animHeader = &_room->_talkingAnimHeader;

	int x = animSet->x + (index ? animHeader->offsetXAnimB : animHeader->offsetXAnimA);
	int y = animSet->y + (index ? animHeader->offsetYAnimB : animHeader->offsetYAnimA);

	int w = index ? animHeader->wAnimB : animHeader->wAnimA;
	int h = index ? animHeader->hAnimB : animHeader->hAnimA;
	int numFrames = index ? animHeader->numFramesAnimB : animHeader->numFramesAnimA;
	int curFrame = index ? animHeader->currentFrameAnimB++ : animHeader->currentFrameAnimA++;
	if (curFrame >= numFrames) {
		if (index) {
			animHeader->currentFrameAnimB = 0;
		} else {
			animHeader->currentFrameAnimA = 0;
		}
		curFrame = 0;
	}
	byte *frame = index ? animHeader->animB[curFrame] : animHeader->animA[curFrame];
	// debug("Talking NPC frame %d/%d, x=%d, y=%d, w=%d, h=%d", curFrame, numFrames, x, y, w, h);

	drawSpriteToBuffer(_compositeBuffer, 640, frame, x, y, w, h, 255);
}

void PelrockEngine::walkTo(int x, int y) {
	_currentStep = 0;
	PathContext context = {nullptr, nullptr, nullptr, 0, 0, 0};
	pathFind(x, y, &context);
	_currentContext = context;
	alfredState.animState = ALFRED_WALKING;
	alfredState.curFrame = 0;
}

bool PelrockEngine::pathFind(int targetX, int targetY, PathContext *context) {

	if (context->pathBuffer == NULL) {
		context->pathBuffer = (uint8_t *)malloc(MAX_PATH_LENGTH);
	}
	if (context->movementBuffer == NULL) {
		context->movementBuffer = (MovementStep *)malloc(MAX_MOVEMENT_STEPS * sizeof(MovementStep));
	}

	int startX = alfredState.x;
	int startY = alfredState.y;
	Common::Point target = calculateWalkTarget(targetX, targetY);
	targetX = target.x;
	targetY = target.y;
	debug("Startx= %d, starty= %d, destx= %d, desty= %d", startX, startY, targetX, targetY);

	uint8_t startBox = findWalkboxForPoint(startX, startY);
	uint8_t destBox = findWalkboxForPoint(targetX, targetY);

	debug("Pathfinding from (%d, %d) in box %d to (%d, %d) in box %d\n", startX, startY, startBox, targetX, targetY, destBox);
	// Check if both points are in valid walkboxes
	if (startBox == 0xFF || destBox == 0xFF) {
		debug("Error: Start or destination not in any walkbox\n");
		return false;
	}
	// Special case: same walkbox
	if (startBox == destBox) {
		// Generate direct movement
		MovementStep direct_step;
		direct_step.flags = 0;
		if (startX < targetX) {
			direct_step.distance_x = targetX - startX;
			direct_step.flags |= MOVE_RIGHT;
		} else {
			direct_step.distance_x = startX - targetX;
			direct_step.flags |= MOVE_LEFT;
		}

		if (startY < targetY) {
			direct_step.distance_y = targetY - startY;
			direct_step.flags |= MOVE_DOWN;
		} else {
			direct_step.distance_y = startY - targetY;
			direct_step.flags |= MOVE_UP;
		}

		context->movementBuffer[0] = direct_step;
		context->movementCount = 1;
	} else {
		// Build walkbox path
		context->pathLength = buildWalkboxPath(startBox, destBox, context->pathBuffer);
		debug("Walkbox path to point");
		for (int i = 0; i < context->pathLength; i++) {
			debug("Walkbox %d: %d", i, context->pathBuffer[i]);
		}
		if (context->pathLength == 0) {
			debug("Error: No path found\n");
			return false;
		}

		// Generate movement steps
		context->movementCount = generateMovementSteps(context->pathBuffer, context->pathLength, startX, startY, targetX, targetY, context->movementBuffer);
		for (int i = 0; i < context->movementCount; i++) {
			debug("Movement step %d: flags=\"%s\", dx=%d, dy=%d", i, printMovementFlags(context->movementBuffer[i].flags).c_str(), context->movementBuffer[i].distance_x, context->movementBuffer[i].distance_y);
		}
	}
	return true;
}

/**
 * Calculate movement needed to reach a target within a walkbox
 */
void calculateMovementToTarget(uint16_t current_x, uint16_t current_y,
							   uint16_t target_x, uint16_t target_y,
							   WalkBox *box,
							   MovementStep *step) {
	step->flags = 0;
	step->distance_x = 0;
	step->distance_y = 0;

	// Calculate horizontal movement
	if (current_x < box->x) {
		// Need to move right to enter walkbox
		step->distance_x = box->x - current_x;
		step->flags |= MOVE_RIGHT;
	} else if (current_x > box->x + box->w) {
		// Need to move left to enter walkbox
		step->distance_x = current_x - (box->x + box->w);
		step->flags |= MOVE_LEFT;
	}

	// Calculate vertical movement
	if (current_y < box->y) {
		// Need to move down to enter walkbox
		step->distance_y = box->y - current_y;
		step->flags |= MOVE_DOWN;
	} else if (current_y > box->y + box->h) {
		// Need to move up to enter walkbox
		step->distance_y = current_y - (box->y + box->h);
		step->flags |= MOVE_UP;
	}
}

/**
 * Generate movement steps from walkbox path
 * Returns: number of movement steps generated
 */
uint16_t PelrockEngine::generateMovementSteps(uint8_t *pathBuffer,
											  uint16_t pathLength,
											  uint16_t startX, uint16_t startY,
											  uint16_t destX, uint16_t destY,
											  MovementStep *movementBuffer) {
	uint16_t currentX = startX;
	uint16_t currentY = startY;
	uint16_t movementIndex = 0;

	// Generate movements for each walkbox in path
	for (uint16_t i = 0; i < pathLength && pathBuffer[i] != PATH_END; i++) {
		uint8_t boxIndex = pathBuffer[i];
		WalkBox *box = &_room->_currentRoomWalkboxes[boxIndex];

		MovementStep step;
		calculateMovementToTarget(currentX, currentY, destX, destY, box, &step);

		if (step.distance_x > 0 || step.distance_y > 0) {
			movementBuffer[movementIndex++] = step;

			// Update current position
			if (step.flags & MOVE_RIGHT) {
				currentX = box->x;
			} else if (step.flags & MOVE_LEFT) {
				currentX = box->x + box->w;
			}

			if (step.flags & MOVE_DOWN) {
				currentY = box->y;
			} else if (step.flags & MOVE_UP) {
				currentY = box->y + box->h;
			}
		}
	}

	// Final movement to exact destination
	MovementStep final_step;
	final_step.flags = 0;

	if (currentX < destX) {
		final_step.distance_x = destX - currentX;
		final_step.flags |= MOVE_RIGHT;
	} else if (currentX > destX) {
		final_step.distance_x = currentX - destX;
		final_step.flags |= MOVE_LEFT;
	} else {
		final_step.distance_x = 0;
	}

	if (currentY < destY) {
		final_step.distance_y = destY - currentY;
		final_step.flags |= MOVE_DOWN;
	} else if (currentY > destY) {
		final_step.distance_y = currentY - destY;
		final_step.flags |= MOVE_UP;
	} else {
		final_step.distance_y = 0;
	}

	if (final_step.distance_x > 0 || final_step.distance_y > 0) {
		movementBuffer[movementIndex++] = final_step;
	}

	return movementIndex;
}

uint16_t PelrockEngine::buildWalkboxPath(uint8_t startBox, uint8_t destBox, uint8_t *pathBuffer) {

	uint16_t pathIndex = 0;
	uint8_t currentBox = startBox;

	// Initialize path with start walkbox
	pathBuffer[pathIndex++] = startBox;

	// Clear visited flags
	clearVisitedFlags();

	// Breadth-first search through walkboxes
	while (currentBox != destBox && pathIndex < MAX_PATH_LENGTH - 1) {
		uint8_t nextBox = getAdjacentWalkbox(currentBox);

		if (nextBox == 0xFF) {
			// Dead end - backtrack
			if (pathIndex > 1) {
				pathIndex--;
				currentBox = pathBuffer[pathIndex - 1];
			} else {
				// No path exists
				return 0;
			}
		} else if (nextBox == destBox) {
			// Found destination
			pathBuffer[pathIndex++] = destBox;
			break;
		} else {
			// Continue searching
			pathBuffer[pathIndex++] = nextBox;
			currentBox = nextBox;
		}
	}

	// Terminate path
	pathBuffer[pathIndex] = PATH_END;

	return pathIndex;
}

void PelrockEngine::clearVisitedFlags() {
	for (int i = 0; i < _room->_currentRoomWalkboxes.size(); i++) {
		_room->_currentRoomWalkboxes[i].flags = 0;
	}
}

/**
 * Check if two walkboxes overlap or touch (are adjacent)
 */
bool areWalkboxesAdjacent(WalkBox *box1, WalkBox *box2) {
	uint16_t box1_x_max = box1->x + box1->w;
	uint16_t box1_y_max = box1->y + box1->h;
	uint16_t box2_x_max = box2->x + box2->w;
	uint16_t box2_y_max = box2->y + box2->h;

	// Check if X ranges overlap
	bool xOverlap = (box1->x <= box2_x_max) && (box2->x <= box1_x_max);

	// Check if Y ranges overlap
	bool yOverlap = (box1->y <= box2_y_max) && (box2->y <= box1_y_max);

	return xOverlap && yOverlap;
}

uint8_t PelrockEngine::getAdjacentWalkbox(uint8_t currentBoxIndex) {
	WalkBox *currentBox = &_room->_currentRoomWalkboxes[currentBoxIndex];

	// Mark current walkbox as visited
	currentBox->flags = 0x01;

	// Search for adjacent unvisited walkbox
	for (uint8_t i = 0; i < _room->_currentRoomWalkboxes.size(); i++) {
		// Skip current walkbox
		if (i == currentBoxIndex) {
			continue;
		}

		// Skip already visited walkboxes
		if (_room->_currentRoomWalkboxes[i].flags == 0x01) {
			continue;
		}

		// Check if walkboxes are adjacent
		if (areWalkboxesAdjacent(currentBox, &_room->_currentRoomWalkboxes[i])) {
			return i;
		}
	}

	return 0xFF; // No adjacent walkbox found
}

bool PelrockEngine::isPointInWalkbox(WalkBox *box, uint16_t x, uint16_t y) {
	return (x >= box->x &&
			x <= box->x + box->w &&
			y >= box->y &&
			y <= box->y + box->h);
}

uint8_t PelrockEngine::findWalkboxForPoint(uint16_t x, uint16_t y) {
	for (uint8_t i = 0; i < _room->_currentRoomWalkboxes.size(); i++) {
		if (isPointInWalkbox(&_room->_currentRoomWalkboxes[i], x, y)) {
			return i;
		}
	}
	return 0xFF; // Not found
}

VerbIcon PelrockEngine::isActionUnder(int x, int y) {
	Common::Array<VerbIcon> actions = availableActions(_currentHotspot);
	for (int i = 0; i < actions.size(); i++) {
		int actionX = _popupX + 20 + (i * (kVerbIconWidth + 2));
		int actionY = _popupY + 20;
		Common::Rect actionRect = Common::Rect(actionX, actionY, actionX + kVerbIconWidth, actionY + kVerbIconHeight);
		if (actionRect.contains(x, y)) {

			return actions[i];
		}
	}
	return NO_ACTION;
}

void PelrockEngine::checkMouseClick(int x, int y) {

	if (whichNPCTalking)
		whichNPCTalking = false;

	if (_displayPopup) {
		// Common::Array<VerbIcon> actions = availableActions(_currentHotspot);
		VerbIcon actionClicked = isActionUnder(x, y);
		if (actionClicked != NO_ACTION) {
			debug("Action %d clicked", actionClicked);
			doAction(actionClicked, _currentHotspot);
			_displayPopup = false;
			return;
		}
	}

	_displayPopup = false;
	_currentHotspot = nullptr;

	Common::Point walkTarget = calculateWalkTarget(mouseX, mouseY);
	_curWalkTarget = walkTarget;

	{ // For quick room navigation
		Exit *exit = isExitUnder(walkTarget.x, walkTarget.y);

		if (exit != nullptr) {
			alfredState.x = exit->targetX;
			alfredState.y = exit->targetY;

			debug("Placing character at %d, %d", exit->targetX, exit->targetY);
			setScreen(exit->targetRoom, exit->dir);
		} else {
			walkTo(walkTarget.x, walkTarget.y);
		}
	}
}

void PelrockEngine::changeCursor(Cursor cursor) {
	CursorMan.replaceCursor(_res->_cursorMasks[cursor], kCursorWidth, kCursorHeight, 0, 0, 255);
}

void PelrockEngine::checkMouseHover() {
	bool isSomethingUnder = false;

	// Calculate walk target first (before checking anything else)
	Common::Point walkTarget = calculateWalkTarget(mouseX, mouseY);

	// Check if walk target hits any exit
	bool exitDetected = false;
	Exit *exit = isExitUnder(walkTarget.x, walkTarget.y);
	if (exit != nullptr) {
		exitDetected = true;
	}

	int hotspotIndex = isHotspotUnder(mouseX, mouseY);

	if (hotspotIndex != -1) {
		isSomethingUnder = true;
	}

	if (isActionUnder(mouseX, mouseY) != NO_ACTION) {
		isSomethingUnder = false;
	}

	if (isSomethingUnder && exitDetected) {
		changeCursor(COMBINATION);
	} else if (isSomethingUnder) {
		changeCursor(HOTSPOT);
	} else if (exitDetected) {
		changeCursor(EXIT);
	} else {
		changeCursor(DEFAULT);
	}
}

Common::Point PelrockEngine::calculateWalkTarget(int mouseX, int mouseY) {
	// Starting point for pathfinding
	int sourceX = mouseX;
	int sourceY = mouseY;

	// TODO: If hovering over a sprite/hotspot, adjust source point to sprite center
	// For now, just use mouse position

	// Find nearest walkable point in walkboxes
	uint32 minDistance = 0xFFFFFFFF;
	Common::Point bestTarget(sourceX, sourceY);

	// for (Common::List<WalkBox>::iterator it = _currentRoomWalkboxes.begin();
	//  it != _currentRoomWalkboxes.end(); ++it) {
	for (size_t i = 0; i < _room->_currentRoomWalkboxes.size(); i++) {

		// Calculate distance from source point to this walkbox (Manhattan distance)
		int dx = 0;
		int dy = 0;

		// Calculate horizontal distance
		if (sourceX < _room->_currentRoomWalkboxes[i].x) {
			dx = _room->_currentRoomWalkboxes[i].x - sourceX;
		} else if (sourceX > _room->_currentRoomWalkboxes[i].x + _room->_currentRoomWalkboxes[i].w) {
			dx = sourceX - (_room->_currentRoomWalkboxes[i].x + _room->_currentRoomWalkboxes[i].w);
		}
		// else: sourceX is inside walkbox horizontally, dx = 0

		// Calculate vertical distance
		if (sourceY < _room->_currentRoomWalkboxes[i].y) {
			dy = _room->_currentRoomWalkboxes[i].y - sourceY;
		} else if (sourceY > _room->_currentRoomWalkboxes[i].y + _room->_currentRoomWalkboxes[i].h) {
			dy = sourceY - (_room->_currentRoomWalkboxes[i].y + _room->_currentRoomWalkboxes[i].h);
		}
		// else: sourceY is inside walkbox vertically, dy = 0

		uint32 distance = dx + dy;

		if (distance < minDistance) {
			minDistance = distance;

			// Calculate target point (nearest point on walkbox to source)
			int targetX = sourceX;
			int targetY = sourceY;

			if (sourceX < _room->_currentRoomWalkboxes[i].x) {
				targetX = _room->_currentRoomWalkboxes[i].x;
			} else if (sourceX > _room->_currentRoomWalkboxes[i].x + _room->_currentRoomWalkboxes[i].w) {
				targetX = _room->_currentRoomWalkboxes[i].x + _room->_currentRoomWalkboxes[i].w;
			}

			if (sourceY < _room->_currentRoomWalkboxes[i].y) {
				targetY = _room->_currentRoomWalkboxes[i].y;
			} else if (sourceY > _room->_currentRoomWalkboxes[i].y + _room->_currentRoomWalkboxes[i].h) {
				targetY = _room->_currentRoomWalkboxes[i].y + _room->_currentRoomWalkboxes[i].h;
			}

			bestTarget.x = targetX;
			bestTarget.y = targetY;
		}
	}

	return bestTarget;
}

void PelrockEngine::drawText(Common::String text, int x, int y, int w, byte color) {
	Common::Rect rect = _largeFont->getBoundingBox(text.c_str());
	if (x + rect.width() > 640) {
		x = 640 - rect.width() - 2;
	}
	if (y + rect.height() > 400) {
		y = 400 - rect.height();
	}
	if (x < 0) {
		x = 0;
	}
	if (y < 0) {
		y = 0;
	}
	// Draw main text on top
	_largeFont->drawString(_screen, text.c_str(), x, y, w, color, Graphics::kTextAlignCenter);
}

void PelrockEngine::sayNPC(Sprite *anim, Common::String text, byte color) {
	isNPCATalking = true;
	whichNPCTalking = anim->extra;
	debug("NPC says %s, color = %d", text.c_str(), color);
	_currentTextPages = wordWrap(text);
	_textColor = color;
	int totalChars = 0;
	for (int i = 0; i < _currentTextPages[0].size(); i++) {
		totalChars += _currentTextPages[0][i].size();
	}
	debug("Settijng textpos to %d, %d", anim->x, anim->y - 10);
	_textPos = Common::Point(anim->x, anim->y - 10);
	_chronoManager->_textTtl = totalChars * kTextCharDisplayTime;
}

void PelrockEngine::sayAlfred(Common::String text) {
	alfredState.nextState = ALFRED_TALKING;
	alfredState.curFrame = 0;
	debug("Alfred says: %s", text.c_str());
	_currentTextPages = wordWrap(text);
	_textColor = 13;
	int totalChars = 0;
	for (int i = 0; i < _currentTextPages[0].size(); i++) {
		totalChars += _currentTextPages[0][i].size();
	}
	_chronoManager->_textTtl = totalChars * kTextCharDisplayTime;
}

bool isEndMarker(char char_byte) {
	return char_byte == CHAR_END_MARKER_1 || char_byte == CHAR_END_MARKER_2 || char_byte == CHAR_END_MARKER_3 || char_byte == CHAR_END_MARKER_4;
}

int calculateWordLength(Common::String text, int startPos, bool &isEnd) {
	// return word_length, is_end
	int wordLength = 0;
	int pos = startPos;
	while (pos < text.size()) {
		char char_byte = text[pos];
		if (char_byte == CHAR_SPACE || isEndMarker(char_byte)) {
			break;
		}
		wordLength++;
		pos++;
	}
	// Check if we hit an end marker
	if (pos < text.size() && isEndMarker(text[pos])) {
		isEnd = true;
	}
	// Count ALL trailing spaces as part of this word
	if (pos < text.size() && !isEnd) {
		if (text[pos] == CHAR_END_MARKER_3) { // 0xF8 (-8) special case
			wordLength += 3;
		} else {
			// Count all consecutive spaces
			while (pos < text.size() && text[pos] == CHAR_SPACE) {
				wordLength++;
				pos++;
			}
		}
	}
	return wordLength;
}

Common::Array<Common::Array<Common::String>> wordWrap(Common::String text) {

	Common::Array<Common::Array<Common::String>> pages;
	Common::Array<Common::String> currentPage;
	Common::Array<Common::String> currentLine;
	int charsRemaining = MAX_CHARS_PER_LINE;
	int position = 0;
	int currentLineNum = 0;
	while (position < text.size()) {
		bool isEnd = false;
		int wordLength = calculateWordLength(text, position, isEnd);
		// # Extract the word (including trailing spaces)
		// word = text[position:position + word_length].decode('latin-1', errors='replace')
		Common::String word = text.substr(position, wordLength).decode(Common::kLatin1);
		// # Key decision: if word_length > chars_remaining, wrap to next line
		if (wordLength > charsRemaining) {
			// Word is longer than the entire line - need to split
			currentPage.push_back(joinStrings(currentLine, ""));
			currentLine.clear();
			charsRemaining = MAX_CHARS_PER_LINE;
			currentLineNum++;

			if (currentLineNum >= MAX_LINES) {
				pages.push_back(currentPage);
				currentPage.clear();
				currentLineNum = 0;
			}
		}
		// Add word to current line
		currentLine.push_back(word);
		charsRemaining -= wordLength;

		if (charsRemaining == 0 && isEnd) {
			Common::String lineText = joinStrings(currentLine, "");
			while (lineText.lastChar() == CHAR_SPACE) {
				lineText = lineText.substr(0, lineText.size() - 1);
			}
			int trailingSpaces = currentLine.size() - lineText.size();
			if (trailingSpaces > 0) {
				currentPage.push_back(lineText);
				//  current_line = [' ' * trailing_spaces]
				Common::String currentLine(trailingSpaces, ' ');
				charsRemaining = MAX_CHARS_PER_LINE - trailingSpaces;
				currentLineNum += 1;

				if (currentLineNum >= MAX_LINES) {
					pages.push_back(currentPage);
					currentPage.clear();
					currentLineNum = 0;
				}
			}
		}

		position += wordLength;
		if (isEnd) {
			// End of sentence/paragraph/page
			break;
		}
	}
	if (currentLine.empty() == false) {
		Common::String lineText = joinStrings(currentLine, "");
		while (lineText.lastChar() == CHAR_SPACE) {
			lineText = lineText.substr(0, lineText.size() - 1);
		}
		currentPage.push_back(lineText);
	}
	if (currentPage.empty() == false) {
		pages.push_back(currentPage);
	}
	debug("Word wrap produced %d pages", pages.size());
	for (int i = 0; i < pages.size(); i++) {
		debug(" Page %d:", i);
		for (int j = 0; j < pages[i].size(); j++) {
			debug("   Line %d: %s", j, pages[i][j].c_str());
		}
	}
	return pages;
}

void PelrockEngine::setScreen(int number, AlfredDirection dir) {
	_soundManager->stopAllSounds();
	Common::File roomFile;
	debug("Loading room %s number %d", _room->getRoomName(number).c_str(), number);
	if (!roomFile.open(Common::Path("ALFRED.1"))) {
		error("Could not open ALFRED.1");
		return;
	}
	alfredState.direction = dir;
	alfredState.animState = ALFRED_IDLE;
	_currentStep = 0;
	int roomOffset = number * kRoomStructSize;
	alfredState.curFrame = 0;
	byte *palette = new byte[256 * 3];
	_room->getPalette(&roomFile, roomOffset, palette);

	int paletteOffset = roomOffset + (11 * 8);
	roomFile.seek(paletteOffset, SEEK_SET);
	uint32 offset = roomFile.readUint32LE();

	g_system->getPaletteManager()->setPalette(palette, 0, 256);

	byte *background = new byte[640 * 400];
	_room->getBackground(&roomFile, roomOffset, background);
	if (_currentBackground != nullptr)
		delete[] _currentBackground;
	_currentBackground = new byte[640 * 400];
	Common::copy(background, background + 640 * 400, _currentBackground);
	for (int i = 0; i < 640; i++) {
		for (int j = 0; j < 400; j++) {
			_screen->setPixel(i, j, background[j * 640 + i]);
		}
	}

	_room->loadRoomMetadata(&roomFile, number);
	_room->loadRoomTalkingAnimations(number);
	if (_room->_musicTrack > 0)
		_soundManager->playMusicTrack(_room->_musicTrack);
	else {
		_soundManager->stopMusic();
	}
	// for (int i = 0; i < kNumSfxPerRoom; i++) {
	// 	if (_room->_roomSfx[i])
	// 		_soundManager->playSound(_room->_roomSfx[i]);
	// }

	_room->_currentRoomNumber = number;
	_screen->markAllDirty();
	roomFile.close();
	delete[] background;
	delete[] palette;
}

Common::Error PelrockEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

} // End of namespace Pelrock
