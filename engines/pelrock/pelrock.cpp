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
#include "pelrock/pathfinding.h"
#include "pelrock/pelrock.h"
#include "pelrock/util.h"

namespace Pelrock {

PelrockEngine *g_engine;

PelrockEngine::PelrockEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
																				 _gameDescription(gameDesc), _randomSource("Pelrock") {
	g_engine = this;
	_chrono = new ChronoManager();
	_events = new PelrockEventManager();
}

PelrockEngine::~PelrockEngine() {
	delete[] _compositeBuffer;
	delete[] _currentBackground;
	delete _largeFont;
	delete _smallFont;
	delete _doubleSmallFont;
	delete _screen;
	delete _chrono;
	delete _videoManager;
	delete _sound;
	delete _room;
	delete _res;
	delete _events;
	delete _dialog;
	delete _menu;
	// if (_bgPopupBalloon)
	// 	delete[] _bgPopupBalloon;
}

uint32 PelrockEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String PelrockEngine::getGameId() const {
	return _gameDescription->gameId;
}

// Common::Array<Common::Array<Common::String>> wordWrap(Common::String text);

Common::Error PelrockEngine::run() {
	// Initialize 320x200 paletted graphics mode
	initGraphics(640, 400);
	_screen = new Graphics::Screen();
	_videoManager = new VideoManager(_screen, _events);
	_room = new RoomManager();
	_res = new ResourceManager();
	_sound = new SoundManager(_mixer);
	_dialog = new DialogManager(_screen, _events);
	_menu = new MenuManager(_screen, _events, _res);

	// Set the engine's debugger console
	setDebugger(new PelrockConsole(this));

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	// Simple event handling loop
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

		if (stateGame == SETTINGS) {
			changeCursor(DEFAULT);
			_menu->menuLoop();
		} else if (stateGame == GAME) {
			gameLoop();
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
	_res->loadInventoryItems();
	_res->loadAlfredResponses();

	_sound->loadSoundIndex();
	_menu->loadMenu();

	calculateScalingMasks();
	_compositeBuffer = new byte[640 * 400];
	_currentBackground = new byte[640 * 400];

	_smallFont = new SmallFont();
	_smallFont->load("ALFRED.4");
	_largeFont = new LargeFont();
	_largeFont->load("ALFRED.7");
	_doubleSmallFont = new DoubleSmallFont();
	_doubleSmallFont->load("ALFRED.4");

	changeCursor(DEFAULT);
	CursorMan.showMouse(true);

	if (gameInitialized == false) {
		gameInitialized = true;
		loadAnims();
		// setScreen(0, ALFRED_DOWN);
		setScreen(2, ALFRED_LEFT);
		alfredState.x = 576;
		alfredState.y = 374;
	}
}

void PelrockEngine::loadAnims() {
	_res->loadAlfredAnims();
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
				*(byte *)_screen->getBasePtr(x + i, y + j) = slice[index];
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

void PelrockEngine::playSoundIfNeeded() {

	int soundIndex = _sound->tick(_chrono->getFrameCount());
	if (soundIndex >= 0 && soundIndex < _room->_roomSfx.size()) {
		_sound->playSound(_room->_roomSfx[3 + soundIndex]);
	}
}

void PelrockEngine::renderScene(bool showTextOverlay) {

	_chrono->updateChrono();
	if (_chrono->_gameTick) {
		playSoundIfNeeded();

		copyBackgroundToBuffer();

		placeStickers();
		updateAnimations();

		if (showTextOverlay) {
			_dialog->displayChoices(_dialog->_currentChoices, _compositeBuffer);
		}

		presentFrame();
		updatePaletteAnimations();

		// if (alfredState.animState != ALFRED_WALKING && !_currentTextPages.empty()) {
		// 	_chronoManager->countTextDown = true;
		// 	if (_textDurationFrames-- > 0) {
		// 		if (alfredState.animState == ALFRED_TALKING) {
		// 			_textPos = Common::Point(alfredState.x, alfredState.y - kAlfredFrameHeight - 10);
		// 		}
		// 		renderText(_currentTextPages[_currentTextPageIndex], _textColor, _textPos.x, _textPos.y);
		// 	} else if (_currentTextPageIndex < _currentTextPages.size() - 1) {
		// 		_currentTextPageIndex++;

		// 		int totalChars = 0;
		// 		for (int i = 0; i < _currentTextPages[_currentTextPageIndex].size(); i++) {
		// 			totalChars += _currentTextPages[_currentTextPageIndex][i].size();
		// 		}
		// 		_textDurationFrames = totalChars / 2;
		// 	} else {
		// 		_currentTextPages.clear();
		// 		_currentTextPageIndex = 0;
		// 		alfredState.animState = ALFRED_IDLE;
		// 		isNPCATalking = false;
		// 		isNPCBTalking = false;
		// 		_chronoManager->countTextDown = false;
		// 	}
		// }

		// if (alfredState.animState == ALFRED_IDLE && alfredState.nextState != ALFRED_IDLE) {
		// 	alfredState.animState = alfredState.nextState;
		// 	alfredState.nextState = ALFRED_IDLE;
		// 	alfredState.curFrame = 0;
		// }

		_screen->markAllDirty();
	}
}

void PelrockEngine::performActionTrigger(uint16 actionTrigger) {
	debug("Performing action trigger: %d", actionTrigger);
	switch (actionTrigger) {
	case 257:
		byte *palette = new byte[768];
		if (_extraScreen == nullptr) {
			_extraScreen = new byte[640 * 400];
		}
		_res->getExtraScreen(9, _extraScreen, palette);

		g_system->getPaletteManager()->setPalette(palette, 0, 256);
		showExtraScreen();

		_dialog->sayAlfred(_res->_alfredResponses[0][0]); // "I found something interesting!");

		delete[] palette;
		break;
	}
}

void PelrockEngine::checkMouse() {
	if (_events->_leftMouseClicked) {
		checkMouseClick(_events->_mouseClickX, _events->_mouseClickY);
		_events->_leftMouseClicked = false;
		_actionPopupState.isActive = false;
	} else if (_events->_longClicked) {
		checkLongMouseClick(_events->_mouseClickX, _events->_mouseClickY);
		_events->_longClicked = false;
	} else if (_events->_rightMouseClicked) {

		g_system->getPaletteManager()->setPalette(_menu->_mainMenuPalette, 0, 256);
		_events->_rightMouseClicked = false;
		stateGame = SETTINGS;
	}
	checkMouseHover();
}

void PelrockEngine::copyBackgroundToBuffer() {
	// copy background to buffer
	memcpy(_compositeBuffer, _currentBackground, 640 * 400);
}

void PelrockEngine::updateAnimations() {
	// Sort sprites by zOrder (persists in the array)

	sortAnimsByZOrder(_room->_currentRoomAnims);
	// First pass: sprites behind Alfred (y <= alfredY)
	for (int i = 0; i < _room->_currentRoomAnims.size(); i++) {
		if (_room->_currentRoomAnims[i].zOrder > 10 || _room->_currentRoomAnims[i].zOrder < 0) {
			drawNextFrame(&_room->_currentRoomAnims[i]);
		}
	}

	// Draw Alfred here (you'll need to add this)
	chooseAlfredStateAndDraw();

	// Second pass: sprites in front of Alfred (y > alfredY)
	for (int i = 0; i < _room->_currentRoomAnims.size(); i++) {
		if (_room->_currentRoomAnims[i].zOrder <= 10 && _room->_currentRoomAnims[i].zOrder >= 0) {
			drawNextFrame(&_room->_currentRoomAnims[i]);
		}
	}

	if (_actionPopupState.isActive) {
		showActionBalloon(_actionPopupState.x, _actionPopupState.y, _actionPopupState.curFrame);
		if (_actionPopupState.curFrame < 3) {
			_actionPopupState.curFrame++;
		} else
			_actionPopupState.curFrame = 0;
	}
}

void PelrockEngine::presentFrame() {
	memcpy(_screen->getPixels(), _compositeBuffer, 640 * 400);
	paintDebugLayer();
	_screen->markAllDirty();
}

void PelrockEngine::updatePaletteAnimations() {
	if (_room->_currentPaletteAnim != nullptr) {
		if (_room->_currentPaletteAnim->paletteMode == 1) {
			animateFadePalette(_room->_currentPaletteAnim);
		} else {
			animateRotatePalette(_room->_currentPaletteAnim);
		}
	}
}

void PelrockEngine::paintDebugLayer() {
	for (int i = 0; i < _room->_currentRoomWalkboxes.size(); i++) {
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
	_smallFont->drawString(_screen, Common::String::format("Frame number: %d", _chrono->getFrameCount()), 0, 30, 640, 13);
}

void PelrockEngine::placeStickers() {
	for (int i = 0; i < _room->_currentRoomStickers.size(); i++) {
		Sticker sticker = _room->_currentRoomStickers[i];
		if (sticker.roomNumber == _room->_currentRoomNumber) {
			placeSticker(sticker);
		}
	}
}

void PelrockEngine::placeSticker(Sticker sticker) {
	for (int y = 0; y < sticker.h; y++) {
		for (int x = 0; x < sticker.w; x++) {
			byte pixel = sticker.stickerData[y * sticker.w + x];
			if (pixel != 0) {
				int bgX = sticker.x + x;
				int bgY = sticker.y + y;
				if (bgX >= 0 && bgX < 640 && bgY >= 0 && bgY < 400) {
					_compositeBuffer[bgY * 640 + bgX] = pixel;
				}
			}
		}
	}
}

void PelrockEngine::animateFadePalette(PaletteAnim *anim) {

	if (anim->data[0] >= anim->data[6] &&
		anim->data[1] >= anim->data[7] &&
		anim->data[2] >= anim->data[8]) {
		anim->data[10] = 0;
	} else if (anim->data[0] <= anim->data[3] &&
			   anim->data[1] <= anim->data[4] &&
			   anim->data[2] <= anim->data[5]) {
		anim->data[10] = 1;
	}

	if (anim->data[10]) {
		if (anim->data[0] < anim->data[6]) {
			anim->data[0] += anim->data[9];
		}
		if (anim->data[1] < anim->data[7]) {
			anim->data[1] += anim->data[9];
		}
		if (anim->data[2] < anim->data[8]) {
			anim->data[2] += anim->data[9];
		}
	} else {
		if (anim->data[0] > anim->data[3]) {
			anim->data[0] -= anim->data[9];
		}
		if (anim->data[1] > anim->data[4]) {
			anim->data[1] -= anim->data[9];
		}
		if (anim->data[2] > anim->data[5]) {
			anim->data[2] -= anim->data[9];
		}
	}

	_room->_roomPalette[anim->startIndex * 3] = anim->data[0];
	_room->_roomPalette[anim->startIndex * 3 + 1] = anim->data[1];
	_room->_roomPalette[anim->startIndex * 3 + 2] = anim->data[2];
	g_system->getPaletteManager()->setPalette(_room->_roomPalette, 0, 256);
}

void PelrockEngine::animateRotatePalette(PaletteAnim *anim) {
	if (anim->curFrameCount >= anim->data[1]) {
		anim->curFrameCount = 0;
		int colors = anim->paletteMode;
		byte *paletteValues = new byte[colors * 3];
		for (int i = 0; i < colors; i++) {
			paletteValues[i * 3] = _room->_roomPalette[(anim->startIndex + i) * 3];
			paletteValues[i * 3 + 1] = _room->_roomPalette[(anim->startIndex + i) * 3 + 1];
			paletteValues[i * 3 + 2] = _room->_roomPalette[(anim->startIndex + i) * 3 + 2];
		}
		for (int i = 0; i < colors; i++) {
			int srcIndex = (i + 1) % colors;
			_room->_roomPalette[(anim->startIndex + i) * 3] = paletteValues[srcIndex * 3];
			_room->_roomPalette[(anim->startIndex + i) * 3 + 1] = paletteValues[srcIndex * 3 + 1];
			_room->_roomPalette[(anim->startIndex + i) * 3 + 2] = paletteValues[srcIndex * 3 + 2];
		}

		g_system->getPaletteManager()->setPalette(_room->_roomPalette, 0, 256);

	} else {
		anim->curFrameCount++;
	}
}

void PelrockEngine::doAction(byte action, HotSpot *hotspot) {
	switch (action) {
	case LOOK:
		lookAt(hotspot);
		break;
	case TALK:
		talkTo(hotspot);
		break;
	case OPEN:
		open(hotspot);
		break;
	case CLOSE:
		close(hotspot);
		break;
	default:
		break;
	}
}

void PelrockEngine::talkTo(HotSpot *hotspot) {
	Sprite *animSet;
	for (int i = 0; i < _room->_currentRoomAnims.size(); i++) {
		if (_room->_currentRoomAnims[i].index == hotspot->index) {
			animSet = &_room->_currentRoomAnims[i];
			animSet->isTalking = true;
			break;
		}
	}
	_dialog->startConversation(_room->_conversationData, _room->_conversationDataSize, animSet);
}

void PelrockEngine::lookAt(HotSpot *hotspot) {
	_dialog->sayAlfred(_room->_currentRoomDescriptions[_currentHotspot->index]);
	_actionPopupState.isActive = false;
}

void PelrockEngine::open(HotSpot *hotspot) {
	switch (hotspot->extra) {
	case 261:
		_room->addSticker(_res->getSticker(91));
		break;
	case 268:
		_room->addSticker(_res->getSticker(93));
		_room->_currentRoomExits[0].isEnabled = true;
		break;
	default:

		break;
	}
}

void PelrockEngine::close(HotSpot *hotspot) {
	switch (hotspot->extra) {
	case 261:
		_room->removeSticker(91);
		break;
	case 268:
		_room->removeSticker(93);
		_room->_currentRoomExits[0].isEnabled = false;
		break;
	default:

		break;
	}
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
		drawText(_largeFont, lines[i], textX, textY, maxW, color);
	}
}

void PelrockEngine::chooseAlfredStateAndDraw() {
	switch (alfredState.animState) {
	case ALFRED_WALKING: {
		MovementStep step = _currentContext.movementBuffer[_currentStep];
		if (step.distanceX > 0) {
			if (step.flags & MOVE_RIGHT) {
				alfredState.direction = ALFRED_RIGHT;
				alfredState.x += MIN(alfredState.movementSpeed, step.distanceX);
			}
			if (step.flags & MOVE_LEFT) {
				alfredState.direction = ALFRED_LEFT;
				alfredState.x -= MIN(alfredState.movementSpeed, step.distanceX);
			}
		}
		if (step.distanceY > 0) {
			if (step.flags & MOVE_DOWN) {
				alfredState.direction = ALFRED_DOWN;
				alfredState.y += MIN(alfredState.movementSpeed, step.distanceY);
			}
			if (step.flags & MOVE_UP) {
				alfredState.direction = ALFRED_UP;
				alfredState.y -= MIN(alfredState.movementSpeed, step.distanceY);
			}
		}

		if (step.distanceX > 0)
			step.distanceX -= MIN(alfredState.movementSpeed, step.distanceX);

		if (step.distanceY > 0)
			step.distanceY -= MIN(alfredState.movementSpeed, step.distanceY);

		if (step.distanceX <= 0 && step.distanceY <= 0) {
			_currentStep++;
			if (_currentStep >= _currentContext.movementCount) {
				_currentStep = 0;
				if(_queuedAction.isQueued) {
					doAction(_queuedAction.verb, &_room->_currentRoomHotspots[_queuedAction.hotspotIndex]);
					_queuedAction.isQueued = false;
				}
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
		alfredState.curFrame++;
		break;
	}
	case ALFRED_TALKING:
		if (alfredState.curFrame >= talkingAnimLengths[alfredState.direction] - 1) {
			alfredState.curFrame = 0;
		}
		drawAlfred(_res->alfredTalkFrames[alfredState.direction][alfredState.curFrame]);
		if (_chrono->getFrameCount() % kAlfredAnimationSpeed == 0) {
			alfredState.curFrame++;
		}
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
	int linesToSkip = kAlfredFrameHeight - finalHeight;

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

		Common::Array<int> tableSkipPositions;
		for (int scanline = 0; scanline < kAlfredFrameHeight; scanline++) {
			if (_heightScalingTable[scaleIndex][scanline] != 0) {
				tableSkipPositions.push_back(scanline);
			}
		}

		// for (size_t i = 0; i < tableSkipPositions.size(); i++) {
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
					if (outIndex >= finalWidth * finalHeight || srcIndex >= kAlfredFrameWidth * kAlfredFrameHeight) {
					} else
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

void applyMovement(int16_t *x, int16_t *y, int8_t *z, uint16_t flags) {
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

	// Z-axis movement
	if (flags & 0x4000) {                  // Bit 14: Z movement enabled
		int amount = (flags >> 10) & 0x07; // Bits 10-12: amount
		if (flags & 0x2000) {              // Bit 13: direction
			*z += amount;                  // 1 = forward (add)
		} else {
			*z -= amount; // 0 = back (subtract)
		}
	}
}

void PelrockEngine::drawNextFrame(Sprite *sprite) {
	Anim &animData = sprite->animData[sprite->curAnimIndex];
	if (sprite->zOrder == -1) {
		// skips z0rder -1 sprites
		return;
	}

	applyMovement(&(sprite->x), &(sprite->y), &(sprite->zOrder), animData.movementFlags);
	int x = sprite->x;
	int y = sprite->y;
	int w = animData.w;
	int h = animData.h;
	if (sprite->isTalking) {
		animateTalkingNPC(sprite);
		return;
	}

	int frameSize = animData.w * animData.h;
	int curFrame = animData.curFrame;
	byte *frame = new byte[frameSize];
	extractSingleFrame(animData.animData, frame, curFrame, animData.w, animData.h);
	drawSpriteToBuffer(_compositeBuffer, 640, frame, sprite->x, sprite->y, sprite->w, sprite->h, 255);

	// if (animData.elpapsedFrames == animData.speed) {
	if (_chrono->getFrameCount() % animData.speed == 0) {
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
	int hotspotIndex = isHotspotUnder(_events->_mouseX, _events->_mouseY);

	if (hotspotIndex != -1) {

		_actionPopupState.x = alfredState.x - kBalloonWidth / 2;
		if (_actionPopupState.x < 0)
			_actionPopupState.x = 0;
		if (_actionPopupState.x + kBalloonWidth > 640) {
			_actionPopupState.x = 640 - kBalloonWidth;
		}

		_actionPopupState.y = alfredState.y - kAlfredFrameHeight - kBalloonHeight;
		if (_actionPopupState.y < 0) {
			_actionPopupState.y = 0;
		}
		_actionPopupState.isActive = true;
		_actionPopupState.curFrame = 0;
		_currentHotspot = &_room->_currentRoomHotspots[hotspotIndex];
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
			x >= hotspot.x && x <= (hotspot.x + hotspot.w) &&
			y >= hotspot.y && y <= (hotspot.y + hotspot.h)) {
			return i;
		}
	}
	return -1;
}

Exit *PelrockEngine::isExitUnder(int x, int y) {
	for (int i = 0; i < _room->_currentRoomExits.size(); i++) {
		Exit exit = _room->_currentRoomExits[i];
		if (x >= exit.x && x <= (exit.x + exit.w) &&
			y >= exit.y && y <= (exit.y + exit.h)
			// && exit.isEnabled
		) {
			return &(_room->_currentRoomExits[i]);
		}
	}
	return nullptr;
}

void PelrockEngine::showActionBalloon(int posx, int posy, int curFrame) {

	drawSpriteToBuffer(_compositeBuffer, 640, _res->_popUpBalloon + (curFrame * kBalloonHeight * kBalloonWidth), posx, posy, kBalloonWidth, kBalloonHeight, 255);
	Common::Array<VerbIcon> actions = availableActions(_currentHotspot);

	VerbIcon icon = isActionUnder(_events->_mouseX, _events->_mouseY);
	for (int i = 0; i < actions.size(); i++) {
		if (icon == actions[i] && _chrono->getFrameCount() % kIconBlinkPeriod == 0) {
			continue;
		}
		drawSpriteToBuffer(_compositeBuffer, 640, _res->_verbIcons[actions[i]], posx + 20 + (i * (kVerbIconWidth + 2)), posy + 20, kVerbIconWidth, kVerbIconHeight, 1);
	}
}

void PelrockEngine::animateTalkingNPC(Sprite *animSet) {
	// Change with the right index

	int index = animSet->index;
	TalkingAnimHeader *animHeader = &_room->_talkingAnimHeader;

	int x = animSet->x + (index ? animHeader->offsetXAnimB : animHeader->offsetXAnimA);
	int y = animSet->y + (index ? animHeader->offsetYAnimB : animHeader->offsetYAnimA);

	int w = index ? animHeader->wAnimB : animHeader->wAnimA;
	int h = index ? animHeader->hAnimB : animHeader->hAnimA;
	int numFrames = index ? animHeader->numFramesAnimB : animHeader->numFramesAnimA;

	if (_chrono->getFrameCount() % kTalkAnimationSpeed == 0) {
		if (index) {
			animHeader->currentFrameAnimB++;
		} else {
			animHeader->currentFrameAnimA++;
		}
	}

	byte curFrame = index ? animHeader->currentFrameAnimB : animHeader->currentFrameAnimA;

	if (curFrame >= numFrames) {
		if (index) {
			animHeader->currentFrameAnimB = 0;
		} else {
			animHeader->currentFrameAnimA = 0;
		}
		curFrame = 0;
	}
	byte *frame = index ? animHeader->animB[curFrame] : animHeader->animA[curFrame];

	drawSpriteToBuffer(_compositeBuffer, 640, frame, x, y, w, h, 255);
}

void PelrockEngine::gameLoop() {
	_events->pollEvent();
	// while (g_system->getEventManager()->pollEvent(e)) {
	// 	if (e.type == Common::EVENT_KEYDOWN) {
	// 		switch (e.kbd.keycode) {
	// 		case Common::KEYCODE_w:
	// 			alfredState.animState = ALFRED_WALKING;
	// 			break;
	// 		case Common::KEYCODE_t:
	// 			alfredState.animState = ALFRED_TALKING;
	// 			break;
	// 		case Common::KEYCODE_s:
	// 			alfredState.animState = ALFRED_IDLE;
	// 			break;
	// 		case Common::KEYCODE_c:
	// 			alfredState.animState = ALFRED_COMB;
	// 			break;
	// 		case Common::KEYCODE_i:
	// 			alfredState.animState = ALFRED_INTERACTING;
	// 			break;
	// 		case Common::KEYCODE_z:
	// 			showShadows = !showShadows;
	// 			break;
	// 		default:
	// 			break;
	// 		}
	// 	}

	if (inConversation) {
		// TODO: Pass actual conversation data from room
		// For now, using nullptr to disable - actual data needs to be loaded
		_dialog->startConversation(nullptr, 0);
		inConversation = false;
	} else {
		checkMouse();
		renderScene();
	}
}

void PelrockEngine::showExtraScreen() {
	memcpy(_screen->getPixels(), _extraScreen, 640 * 400);
	_screen->markAllDirty();
	_screen->update();
	while (!shouldQuit()) {
		_events->pollEvent();

		if (_events->_leftMouseClicked) {
			_events->_leftMouseClicked = false;
			break;
		}
		g_system->delayMillis(10);
	}

	g_system->getPaletteManager()->setPalette(_room->_roomPalette, 0, 256);
	free(_extraScreen);
	_extraScreen = nullptr;
}

void PelrockEngine::walkTo(int x, int y) {
	_currentStep = 0;
	PathContext context = {nullptr, nullptr, nullptr, 0, 0, 0};
	findPath(alfredState.x, alfredState.y, x, y, _room->_currentRoomWalkboxes, &context);
	_currentContext = context;
	alfredState.animState = ALFRED_WALKING;
	alfredState.curFrame = 0;
}

VerbIcon PelrockEngine::isActionUnder(int x, int y) {
	Common::Array<VerbIcon> actions = availableActions(_currentHotspot);
	for (int i = 0; i < actions.size(); i++) {
		int actionX = _actionPopupState.x + 20 + (i * (kVerbIconWidth + 2));
		int actionY = _actionPopupState.y + 20;
		Common::Rect actionRect = Common::Rect(actionX, actionY, actionX + kVerbIconWidth, actionY + kVerbIconHeight);
		if (actionRect.contains(x, y)) {
			return actions[i];
		}
	}
	return NO_ACTION;
}

bool PelrockEngine::isAlfredUnder(int x, int y) {
	// TODO: Account for scaling
	int alfredX = alfredState.x;
	int alfredY = alfredState.y;
	int alfredW = kAlfredFrameWidth;
	int alfredH = kAlfredFrameHeight;

	if (alfredY - alfredH > y || alfredY < y || alfredX > x || alfredX + alfredW < x) {
		return false;
	}
	return true;
}

void PelrockEngine::checkMouseClick(int x, int y) {

	if (_actionPopupState.isActive) {
		// Common::Array<VerbIcon> actions = availableActions(_currentHotspot);
		VerbIcon actionClicked = isActionUnder(x, y);
		if (actionClicked != NO_ACTION) {
			_actionPopupState.isActive = false;
			if(_currentHotspot != nullptr) {
				walkTo(_currentHotspot->x + _currentHotspot->w / 2, _currentHotspot->y + _currentHotspot->h);
				_queuedAction = QueuedAction{actionClicked, _currentHotspot->index, true};
				return;
			}
		}
	}

	_queuedAction = QueuedAction{NO_ACTION, -1, false};

	_actionPopupState.isActive = false;
	_currentHotspot = nullptr;

	Common::Point walkTarget = calculateWalkTarget(_room->_currentRoomWalkboxes, _events->_mouseX, _events->_mouseY);
	_curWalkTarget = walkTarget;

	{ // For quick room navigation
		Exit *exit = isExitUnder(walkTarget.x, walkTarget.y);

		if (exit != nullptr) {
			alfredState.x = exit->targetX;
			alfredState.y = exit->targetY;

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
	bool hotspotDetected = false;

	// Calculate walk target first (before checking anything else)
	Common::Point walkTarget = calculateWalkTarget(_room->_currentRoomWalkboxes, _events->_mouseX, _events->_mouseY);

	// Check if walk target hits any exit
	bool exitDetected = false;
	Exit *exit = isExitUnder(walkTarget.x, walkTarget.y);
	if (exit != nullptr) {
		exitDetected = true;
	}

	int hotspotIndex = isHotspotUnder(_events->_mouseX, _events->_mouseY);
	if (hotspotIndex != -1) {
		hotspotDetected = true;
	}

	if (isActionUnder(_events->_mouseX, _events->_mouseY) != NO_ACTION) {
		hotspotDetected = false;
	}

	bool alfredDetected = false;
	if (isAlfredUnder(_events->_mouseX, _events->_mouseY)) {
		alfredDetected = true;
	}

	if (alfredDetected) {
		changeCursor(ALFRED);
	} else if (hotspotDetected && exitDetected) {
		changeCursor(COMBINATION);
	} else if (hotspotDetected) {
		changeCursor(HOTSPOT);
	} else if (exitDetected) {
		changeCursor(EXIT);
	} else {
		changeCursor(DEFAULT);
	}
}

// void PelrockEngine::sayNPC(Sprite *anim, Common::String text, byte color) {
// 	isNPCATalking = true;
// 	whichNPCTalking = anim->extra;
// 	_currentTextPages = wordWrap(text);
// 	_textColor = color;
// 	int totalChars = 0;
// 	for (int i = 0; i < _currentTextPages[0].size(); i++) {
// 		totalChars += _currentTextPages[0][i].size();
// 	}
// 	_textPos = Common::Point(anim->x, anim->y - 10);
// 	_textDurationFrames = totalChars / 2;
// }

// void PelrockEngine::sayAlfred(Common::String text) {
// 	alfredState.nextState = ALFRED_TALKING;
// 	alfredState.curFrame = 0;
// 	_currentTextPages = wordWrap(text);
// 	_textColor = 13;
// 	int totalChars = 0;
// 	for (int i = 0; i < _currentTextPages[0].size(); i++) {
// 		totalChars += _currentTextPages[0][i].size();
// 	}
// 	_textDurationFrames = totalChars / 2;
// }

// int calculateWordLength(Common::String text, int startPos, bool &isEnd) {
// 	// return word_length, is_end
// 	int wordLength = 0;
// 	int pos = startPos;
// 	while (pos < text.size()) {
// 		char char_byte = text[pos];
// 		if (char_byte == CHAR_SPACE || isEndMarker(char_byte)) {
// 			break;
// 		}
// 		wordLength++;
// 		pos++;
// 	}
// 	// Check if we hit an end marker
// 	if (pos < text.size() && isEndMarker(text[pos])) {
// 		isEnd = true;
// 	}
// 	// Count ALL trailing spaces as part of this word
// 	if (pos < text.size() && !isEnd) {
// 		if (text[pos] == CHAR_END_MARKER_3) { // 0xF8 (-8) special case
// 			wordLength += 3;
// 		} else {
// 			// Count all consecutive spaces
// 			while (pos < text.size() && text[pos] == CHAR_SPACE) {
// 				wordLength++;
// 				pos++;
// 			}
// 		}
// 	}
// 	return wordLength;
// }

// Common::Array<Common::Array<Common::String>> wordWrap(Common::String text) {

// 	Common::Array<Common::Array<Common::String>> pages;
// 	Common::Array<Common::String> currentPage;
// 	Common::Array<Common::String> currentLine;
// 	int charsRemaining = MAX_CHARS_PER_LINE;
// 	int position = 0;
// 	int currentLineNum = 0;
// 	while (position < text.size()) {
// 		bool isEnd = false;
// 		int wordLength = calculateWordLength(text, position, isEnd);
// 		// # Extract the word (including trailing spaces)
// 		// word = text[position:position + word_length].decode('latin-1', errors='replace')
// 		Common::String word = text.substr(position, wordLength).decode(Common::kLatin1);
// 		// # Key decision: if word_length > chars_remaining, wrap to next line
// 		if (wordLength > charsRemaining) {
// 			// Word is longer than the entire line - need to split
// 			currentPage.push_back(joinStrings(currentLine, ""));
// 			currentLine.clear();
// 			charsRemaining = MAX_CHARS_PER_LINE;
// 			currentLineNum++;

// 			if (currentLineNum >= MAX_LINES) {
// 				pages.push_back(currentPage);
// 				currentPage.clear();
// 				currentLineNum = 0;
// 			}
// 		}
// 		// Add word to current line
// 		currentLine.push_back(word);
// 		charsRemaining -= wordLength;

// 		if (charsRemaining == 0 && isEnd) {
// 			Common::String lineText = joinStrings(currentLine, "");
// 			while (lineText.lastChar() == CHAR_SPACE) {
// 				lineText = lineText.substr(0, lineText.size() - 1);
// 			}
// 			int trailingSpaces = currentLine.size() - lineText.size();
// 			if (trailingSpaces > 0) {
// 				currentPage.push_back(lineText);
// 				//  current_line = [' ' * trailing_spaces]
// 				Common::String currentLine(trailingSpaces, ' ');
// 				charsRemaining = MAX_CHARS_PER_LINE - trailingSpaces;
// 				currentLineNum += 1;

// 				if (currentLineNum >= MAX_LINES) {
// 					pages.push_back(currentPage);
// 					currentPage.clear();
// 					currentLineNum = 0;
// 				}
// 			}
// 		}

// 		position += wordLength;
// 		if (isEnd) {
// 			// End of sentence/paragraph/page
// 			break;
// 		}
// 	}
// 	if (currentLine.empty() == false) {
// 		Common::String lineText = joinStrings(currentLine, "");
// 		while (lineText.lastChar() == CHAR_SPACE) {
// 			lineText = lineText.substr(0, lineText.size() - 1);
// 		}
// 		currentPage.push_back(lineText);
// 	}
// 	if (currentPage.empty() == false) {
// 		pages.push_back(currentPage);
// 	}
// 	for (int i = 0; i < pages.size(); i++) {
// 		for (int j = 0; j < pages[i].size(); j++) {
// 		}
// 	}
// 	return pages;
// }

void PelrockEngine::setScreen(int number, AlfredDirection dir) {
	_sound->stopAllSounds();
	Common::File roomFile;
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
		_sound->playMusicTrack(_room->_musicTrack);
	else {
		_sound->stopMusic();
	}

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
