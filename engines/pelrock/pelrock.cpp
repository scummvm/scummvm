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

#include "backends/audiocd/audiocd.h"

#include "pelrock.h"
#include "pelrock/actions.h"
#include "pelrock/computer.h"
#include "pelrock/console.h"
#include "pelrock/detection.h"
#include "pelrock/extrascreens.h"
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
	if (_compositeBuffer)
		delete[] _compositeBuffer;
	if (_currentBackground)
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
	delete _graphics;
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
	_graphics = new GraphicsManager();
	_room = new RoomManager();
	_res = new ResourceManager();
	_sound = new SoundManager(_mixer);
	_dialog = new DialogManager(_screen, _events, _graphics);
	_menu = new MenuManager(_screen, _events, _res, _sound);
	_smallFont = new SmallFont();
	_smallFont->load("ALFRED.4");
	_largeFont = new LargeFont();
	_largeFont->load("ALFRED.7");
	_doubleSmallFont = new DoubleSmallFont();
	_doubleSmallFont->load("ALFRED.4");
	_videoManager = new VideoManager(_screen, _events, _chrono, _largeFont, _dialog, _sound);

	// Set the engine's debugger console
	setDebugger(new PelrockConsole(this));

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	_state->stateGame = shouldPlayIntro ? INTRO : GAME;

	init();
	while (!shouldQuit()) {
		if (_state->stateGame == SETTINGS) {
			_graphics->clearScreen();
			_menu->menuLoop();
			_state->stateGame = GAME;
		} else if (_state->stateGame == GAME) {
			gameLoop();
		} else if (_state->stateGame == INTRO) {
			_videoManager->playIntro();
			_state->stateGame = GAME;
		} else if (_state->stateGame == COMPUTER) {
			computerLoop();
			_state->stateGame = GAME;
		}
	}

	return Common::kNoError;
}

void PelrockEngine::init() {
	_res->loadCursors();
	_res->loadInteractionIcons();
	_res->loadInventoryItems();
	_res->loadHardcodedText();

	_sound->loadSoundIndex();
	_menu->loadMenu();

	calculateScalingMasks();
	_compositeBuffer = new byte[640 * 400];
	_currentBackground = new byte[640 * 400];

	changeCursor(DEFAULT);
	CursorMan.showMouse(true);

	if (gameInitialized == false) {
		gameInitialized = true;
		loadAnims();
		// setScreen(0, ALFRED_DOWN);
		// setScreen(3, ALFRED_RIGHT);
		// setScreen(22, ALFRED_DOWN);
		setScreen(39, ALFRED_DOWN);
		// setScreen(15, ALFRED_DOWN);
		// setScreen(2, ALFRED_LEFT);
		// alfredState.x = 576;
		// alfredState.y = 374;
	}
}

void PelrockEngine::loadAnims() {
	_res->loadAlfredAnims();
}

Common::Array<VerbIcon> PelrockEngine::availableActions(HotSpot *hotspot) {
	if (hotspot == nullptr) {
		return Common::Array<VerbIcon>();
	}
	Common::Array<VerbIcon> verbs;
	verbs.push_back(LOOK);

	if (hotspot->actionFlags & ACTION_MASK_OPEN) {
		verbs.push_back(OPEN);
	}
	if (hotspot->actionFlags & ACTION_MASK_CLOSE) {
		verbs.push_back(CLOSE);
	}
	if (hotspot->actionFlags & ACTION_MASK_UNKNOWN) {
		verbs.push_back(UNKNOWN);
	}
	if (hotspot->actionFlags & ACTION_MASK_PICKUP) {
		verbs.push_back(PICKUP);
	}
	if (hotspot->actionFlags & ACTION_MASK_TALK) {
		verbs.push_back(TALK);
	}
	if (hotspot->actionFlags & ACTION_MASK_PUSH) {
		verbs.push_back(PUSH);
	}
	if (hotspot->actionFlags & ACTION_MASK_PULL) {
		verbs.push_back(PULL);
	}
	return verbs;
}

Common::Point getPositionInBallonForIndex(int i);

// Sort sprites by zOrder in-place using insertion sort (efficient for nearly-sorted data)
void sortAnimsByZOrder(Common::Array<Sprite> &anims) {
	for (size_t i = 0; i < anims.size(); ++i) {
		Sprite key = anims[i];
		int z = key.zOrder;
		size_t j = i;
		while (j > 0 && anims[j - 1].zOrder < z) {
			anims[j] = anims[j - 1];
			--j;
		}
		anims[j] = key;
	}
	// debug("Sorted anims by zOrder");
	// for (size_t i = 0; i < anims.size(); i++) {
	// debug("Anim %d, extra = %d: zOrder=%d", i, anims[i].extra, anims[i].zOrder);
	// }
}

void PelrockEngine::playSoundIfNeeded() {
	// Get ambient slot offset (0-3) or -1 if no sound this frame
	int ambientSlotOffset = _sound->tickAmbientSound(_chrono->getFrameCount());
	if (ambientSlotOffset >= 0) {
		// Convert to room sound index: slots 12-15 = room indices 4-7
		int roomSoundIndex = kAmbientSoundSlotBase + ambientSlotOffset;
		if (roomSoundIndex < _room->_roomSfx.size()) {
			byte soundFileIndex = _room->_roomSfx[roomSoundIndex];
			if (soundFileIndex != 0) { // 0 = NO_SOUND.SMP (disabled)
				_sound->playSound(soundFileIndex);
			}
		}
	}
}

void PelrockEngine::travelToEgypt() {
	_graphics->fadeToBlack(10);
	_sound->playMusicTrack(26, false);
	byte *palette = new byte[768];
	if (_extraScreen == nullptr) {
		_extraScreen = new byte[640 * 400];
	}
	_res->getExtraScreen(6, _extraScreen, palette);
	CursorMan.showMouse(false);
	g_system->getPaletteManager()->setPalette(palette, 0, 256);

	memcpy(_screen->getPixels(), _extraScreen, 640 * 400);
	int frameCount = 0;
	while (!shouldQuit() && frameCount < 96) {
		_events->pollEvent();
		g_system->delayMillis(10);
		_chrono->updateChrono();
		if (_chrono->_gameTick && _chrono->getFrameCount() % 2 == 0) {
			int colorIndex = 160 + frameCount;
			int index = colorIndex * 3;
			palette[index] = 255;   // Red
			palette[index + 1] = 0; // Green
			palette[index + 2] = 0; // Blue
			g_system->getPaletteManager()->setPalette(palette, 0, 256);
			frameCount++;
		}
		drawPaletteSquares((byte *)_screen->getPixels(), palette);

		_screen->markAllDirty();
		_screen->update();
	}
	_graphics->clearScreen();

	g_system->getPaletteManager()->setPalette(_room->_roomPalette, 0, 256);
	free(_extraScreen);
	_extraScreen = nullptr;
	CursorMan.showMouse(true);
	delete[] _extraScreen;
	delete[] palette;
	_screen->markAllDirty();
	_screen->update();
}

bool PelrockEngine::renderScene(int overlayMode) {

	_chrono->updateChrono();
	if (_chrono->_gameTick) {
		frameTriggers();

		playSoundIfNeeded();

		copyBackgroundToBuffer();

		placeStickersFirstPass();

		updateAnimations();

		placeStickersSecondPass();

		renderOverlay(overlayMode);

		mouseHoverForMap();

		presentFrame();

		updatePaletteAnimations();

		_screen->markAllDirty();
		return true;
	}

	switch (_room->_currentRoomNumber) {
	case 2: {
		if (_events->_lastKeyEvent == Common::KEYCODE_x) {
			_events->_lastKeyEvent = Common::KEYCODE_INVALID;
			debug("Pressed X in room 2, numPressedX is now %d, flag is %d", _numPressedX + 1, _state->getFlag(FLAG_PUTA_250_VECES));
			if (_state->getFlag(FLAG_PUTA_250_VECES) == true) {
				_numPressedX++;
				if (_numPressedX == 250) {
					_dialog->say(_res->_ingameTexts[YLOSCONDONES]);
				}
			}
		}
		break;
	}
	}

	return false;
}

void PelrockEngine::mouseHoverForMap() {
	if (_room->_currentRoomNumber == 21 && !_hoveredMapLocation.empty()) {
		Common::Rect r = _largeFont->getBoundingBox(_hoveredMapLocation.c_str());
		drawText(_compositeBuffer, _largeFont, _hoveredMapLocation.c_str(), _events->_mouseX - r.width() / 2, _events->_mouseY - r.height(), 640, ALFRED_COLOR);
	}
}

// const int kPasserbyTriggerFrameInterval = 0x3FF;

const int kPasserbyTriggerFrameInterval = 15;
void PelrockEngine::frameTriggers() {
	uint32 frameCount = _chrono->getFrameCount();
	passerByAnim(frameCount);
}

void PelrockEngine::passerByAnim(uint32 frameCount) {
	if (_room->_passerByAnims == nullptr) {
		return;
	}
	if (_room->_passerByAnims->latch == false) {
		int animIndex = _room->_passerByAnims->numAnims == 1 ? 0 : getRandomNumber(_room->_passerByAnims->numAnims - 1);
		PasserByAnim anim = _room->_passerByAnims->passerByAnims[animIndex];
		if ((frameCount & anim.frameTrigger) == anim.frameTrigger) {
			Sprite *sprite = _room->findSpriteByIndex(anim.spriteIndex);
			if (sprite && sprite->zOrder == -1) {
				debug("Starting passerby anim for sprite %d at index %d", anim.spriteIndex, animIndex);
				sprite->zOrder = anim.targetZIndex;
				sprite->curAnimIndex = 0;
				sprite->animData[0].curFrame = 0;
				sprite->animData[0].curLoop = 0;
				_room->_passerByAnims->latch = true;
				_room->_passerByAnims->currentAnimIndex = animIndex;
			}
		}
	} else {
		PasserByAnim anim = _room->_passerByAnims->passerByAnims[_room->_passerByAnims->currentAnimIndex];
		byte direction = anim.dir;

		int spriteIndex = anim.spriteIndex;
		int startX = anim.startX;
		int startY = anim.startY;
		debug("Checking passerby anim %d for sprite %d, direction %d", _room->_passerByAnims->currentAnimIndex, spriteIndex, direction);
		Sprite *sprite = _room->findSpriteByIndex(spriteIndex);
		if (direction == PASSERBY_RIGHT) {
			debug("Checking passerby anim for sprite %d moving RIGHT, curpos is %d", spriteIndex, sprite->x);
			if (sprite->x >= anim.resetCoord) {
				sprite->x = startX;
				sprite->y = startY;
				sprite->zOrder = -1;
				sprite->curAnimIndex = 0;
				sprite->animData[0].curFrame = 0;
				_room->_passerByAnims->latch = false;
			}
		} else if (direction == PASSERBY_LEFT) {
			debug("Checking passerby anim for sprite %d moving LEFT, curpos is %d", spriteIndex, sprite->x);

			if (sprite->x <= anim.resetCoord) {
				sprite->x = startX;
				sprite->y = startY;
				sprite->zOrder = -1;
				sprite->curAnimIndex = 0;
				sprite->animData[0].curFrame = 0;
				_room->_passerByAnims->latch = false;
			}
		} else if (direction == PASSERBY_DOWN) {
			debug("Checking passerby anim for sprite %d moving DOWN, curpos is %d, reset %d", spriteIndex, sprite->y, anim.resetCoord);
			if (sprite->y >= anim.resetCoord) {
				sprite->x = startX;
				sprite->y = startY;
				sprite->zOrder = -1;
				sprite->curAnimIndex = 0;
				sprite->animData[0].curFrame = 0;
				_room->_passerByAnims->latch = false;
			}
		}
	}
}

void PelrockEngine::reflectionEffect(byte *buf, int x, int y, int width, int height) {

	// ScaleCalculation scaleCalc = calculateScaling(y + 50, _room->_scaleParams);

	// // Update Alfred's scale state for use by other functions
	// _alfredState.scaledX = scaleCalc.scaleX;
	// _alfredState.scaledY = scaleCalc.scaleY;

	// // Use the pre-calculated scaled dimensions from calculateScaling
	// int finalHeight = scaleCalc.scaledHeight;
	// int finalWidth = scaleCalc.scaledWidth;

	// if (finalHeight <= 0) {
	// 	finalHeight = 1;
	// }
	// if (finalWidth <= 0) {
	// 	finalWidth = 1;
	// }

	// byte *finalBuf = scale(scaleCalc.scaleY, finalWidth, finalHeight, buf);
	// height = finalHeight;
	// width = finalWidth;

	// Water reflection - draws mirrored sprite on water pixels
	// Only sprite pixels 0-15 are reflected (checked via pixel & 0xF0 == 0)
	for (int row = 0; row < height; row++) {
		int reflectY = y + row;

		if (reflectY >= 400)
			break; // Screen boundary

		for (int col = 0; col < width; col++) {
			// byte pixel = finalBuf[(height - 1 - row) * width + col]; // Read from bottom up for mirror
			byte pixel = buf[(height - 1 - row) * width + col]; // Read from bottom up for mirror
			// Only reflect pixels 0-15 (high nibble must be 0)
			if (pixel != 255 && (pixel & 0xF0) == 0) {
				byte bgPixel = _compositeBuffer[reflectY * 640 + x + col];
				if (bgPixel >= 223 && bgPixel < 228) { // Is water (0xDF-0xE3)
					_compositeBuffer[reflectY * 640 + x + col] = _room->_paletteRemaps[4][pixel];
				}
			}
		}
	}
}

void PelrockEngine::executeAction(VerbIcon action, HotSpot *hotspot) {
	debug("Executing action %d on hotspot %d", action, hotspot->extra);
	if (action == ITEM) {
		int inventoryObject = _state->selectedInventoryItem;
		for (const CombinationEntry *entry = combinationTable; entry->handler != nullptr; entry++) {
			if (entry->inventoryObject == inventoryObject && entry->hotspotExtra == hotspot->extra) {
				(this->*(entry->handler))(inventoryObject, hotspot);
				return;
			}
		}
		noOpItem(inventoryObject, hotspot);
		warning("No handler for using inventory object %d with hotspot %d", inventoryObject, hotspot->extra);
		return;
	}

	for (const ActionEntry *entry = actionTable; entry->handler != nullptr; entry++) {
		if (entry->action == action && entry->hotspotExtra == hotspot->extra) {
			// Found exact match - call the handler
			(this->*(entry->handler))(hotspot);
			return;
		}
	}

	// Try wildcard match (hotspotExtra == -1 means "any hotspot")
	for (const ActionEntry *entry = actionTable; entry->handler != nullptr; ++entry) {
		if (entry->action == action && entry->hotspotExtra == WILDCARD) {
			(this->*(entry->handler))(hotspot);
			return;
		}
	}

	// No handler found
	warning("No handler for hotspot %d with action %d", hotspot->extra, action);
}

void PelrockEngine::checkMouse() {

	checkMouseHover();

	if(_disableAction) {
		return;
	}

	if (_alfredState.animState == ALFRED_WALKING && !_alfredState.isWalkingCancelable) {
		// Ignore clicks while Alfred is walking
		_events->_leftMouseClicked = false;
		return;
	}

	// Cancel walking animation on mouse click
	if (_events->_leftMouseButton) {
		_alfredState.curFrame = 0;
		_alfredState.setState(ALFRED_IDLE);
	}
	// Handle mouse release after long press (popup selection mode)
	if (_events->_popupSelectionMode && !_events->_leftMouseButton) {
		_events->_leftMouseButton = false;
		_events->_leftMouseClicked = false;
		_events->_popupSelectionMode = false;
		_actionPopupState.isActive = false;
		// Mouse was released while popup is active
		VerbIcon actionClicked = isActionUnder(_events->_releaseX, _events->_releaseY);
		if (actionClicked != NO_ACTION) {
			debug("Popup action clicked: %d, is alfredunder %d", actionClicked, _actionPopupState.isAlfredUnder);
		}
		if (actionClicked != NO_ACTION && _currentHotspot != nullptr) {
			// Action was selected - queue it
			walkAndAction(_currentHotspot, actionClicked);
		} else if (_actionPopupState.isAlfredUnder && actionClicked != NO_ACTION) {
			debug("Using item on Alfred");
			useOnAlfred(_state->selectedInventoryItem);
		} else {
			// Released outside popup - just close it
			_queuedAction = QueuedAction{NO_ACTION, -1, false};
			_currentHotspot = nullptr;
		}
	} else if (_events->_leftMouseClicked) {
		// Regular click (not during popup mode)
		checkMouseClick(_events->_mouseClickX, _events->_mouseClickY);
		_events->_leftMouseClicked = false;
		_actionPopupState.isActive = false;
	} else if (_events->_longClicked && _room->_currentRoomNumber != 21) {
		checkLongMouseClick(_events->_mouseClickX, _events->_mouseClickY);
		_events->_longClicked = false;
	} else if (_events->_rightMouseClicked) {
		_events->_rightMouseClicked = false;
		_state->stateGame = SETTINGS;
	}
}

void PelrockEngine::copyBackgroundToBuffer() {
	// copy background to buffer
	memcpy(_compositeBuffer, _currentBackground, 640 * 400);
}

// Calculate Alfred's z-order based on Y position
// At Y=399 (bottom of screen): z = 10 (foreground)
// At Y=0 (top of screen): z = 209 (background)
static int calculateAlfredZOrder(int alfredY) {
	return ((399 - alfredY) & 0xFFFE) / 2 + 10;
}

void PelrockEngine::updateAnimations() {
	// Sort sprites by zOrder (ascending: low z = back, rendered first)
	sortAnimsByZOrder(_room->_currentRoomAnims);

	int alfredZOrder = calculateAlfredZOrder(_alfredState.y);

	// First pass: sprites behind Alfred (sprite zOrder > alfredZOrder)
	for (uint i = 0; i < _room->_currentRoomAnims.size(); i++) {
		if (_room->_currentRoomAnims[i].zOrder > alfredZOrder || _room->_currentRoomAnims[i].zOrder < 0) {
			drawNextFrame(&_room->_currentRoomAnims[i]);
		}
	}

	// Draw Alfred
	chooseAlfredStateAndDraw();

	// Second pass: sprites in front of Alfred (sprite zOrder <= alfredZOrder)
	for (uint i = 0; i < _room->_currentRoomAnims.size(); i++) {
		if (_room->_currentRoomAnims[i].zOrder <= alfredZOrder && _room->_currentRoomAnims[i].zOrder >= 0) {
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
	bool showWalkboxes = true;

	if (showWalkboxes) {
		for (uint i = 0; i < _room->_currentRoomWalkboxes.size(); i++) {
			WalkBox box = _room->_currentRoomWalkboxes[i];
			drawRect(_screen, box.x, box.y, box.w, box.h, 150 + i);
			// _smallFont->drawString(_screen, Common::String::format("%d", i), box.x + 2, box.y + 2, 640, 14);
		}
	}

	bool showSprites = true;
	if (showSprites) {
		for (uint i = 0; i < _room->_currentRoomAnims.size(); i++) {
			Sprite sprite = _room->_currentRoomAnims[i];
			drawRect(_screen, sprite.x, sprite.y, sprite.animData->w, sprite.animData->h, 14);
			_smallFont->drawString(_screen, Common::String::format("S %d", sprite.index), sprite.x + 2, sprite.y, 640, 14);
		}
	}

	bool showHotspots = true;
	if (showHotspots) {
		for (uint i = 0; i < _room->_currentRoomHotspots.size(); i++) {
			HotSpot hotspot = _room->_currentRoomHotspots[i];
			if (!hotspot.isEnabled || hotspot.isSprite)
				continue;
			drawRect(_screen, hotspot.x, hotspot.y, hotspot.w, hotspot.h, 12);
			_smallFont->drawString(_screen, Common::String::format("HS %d - %d", hotspot.index - _room->_currentRoomAnims.size(), hotspot.extra), hotspot.x + 2, hotspot.y + 2, 640, 12);
			// _smallFont->drawString(_screen, Common::String::format("x=%d", hotspot.extra), hotspot.x + 2, hotspot.y + 2 + 14, 640, 14);
		}
	}

	bool showExits = true;
	if (showExits) {
		for (uint i = 0; i < _room->_currentRoomExits.size(); i++) {
			Exit exit = _room->_currentRoomExits[i];
			drawRect(_screen, exit.x, exit.y, exit.w, exit.h, 200 + i);
			_smallFont->drawString(_screen, Common::String::format("Exit %d -> Room %d", i, exit.targetRoom), exit.x + 2, exit.y + 2, 640, 14);
		}
	}

	drawPos(_screen, _alfredState.x, _alfredState.y, 13);
	drawPos(_screen, _alfredState.x, _alfredState.y - kAlfredFrameHeight, 13);
	drawPos(_screen, _curWalkTarget.x, _curWalkTarget.y, 100);

	if (showShadows) {
		memcpy(_screen->getPixels(), _room->_pixelsShadows, 640 * 400);
	}
	_smallFont->drawString(_screen, Common::String::format("Room number: %d", _room->_currentRoomNumber), 0, 4, 640, 13);
	_smallFont->drawString(_screen, Common::String::format("Alfred pos: %d, %d (%d)", _alfredState.x, _alfredState.y, _alfredState.y - kAlfredFrameHeight), 0, 18, 640, 13);
	_smallFont->drawString(_screen, Common::String::format("Frame number: %d", _chrono->getFrameCount()), 0, 30, 640, 13);
}

void PelrockEngine::placeStickersFirstPass() {
	// also place temporary stickers
	for (uint i = 0; i < _room->_roomStickers.size(); i++) {
		Sticker sticker = _room->_roomStickers[i];
		placeSticker(sticker);
	}
}

void PelrockEngine::placeStickersSecondPass() {
	// Some stickers need to be placed AFTER sprites, hardcoded in the original
	if (_room->_currentRoomNumber == 3) {
		for (uint i = 0; i < _state->stickersPerRoom[3].size(); i++) {
			if (_state->stickersPerRoom[3][i].stickerIndex == 14) {
				placeSticker(_state->stickersPerRoom[3][i]);
				break;
			}
		}
	}
}

void PelrockEngine::placeSticker(Sticker sticker) {
	for (int y = 0; y < sticker.h; y++) {
		for (int x = 0; x < sticker.w; x++) {
			byte pixel = sticker.stickerData[y * sticker.w + x];
			// if (pixel != 0) {
				int bgX = sticker.x + x;
				int bgY = sticker.y + y;
				if (bgX >= 0 && bgX < 640 && bgY >= 0 && bgY < 400) {
					_compositeBuffer[bgY * 640 + bgX] = pixel;
				}
			// }
		}
	}
}

void PelrockEngine::renderOverlay(int overlayMode) {
	if (overlayMode == OVERLAY_CHOICES) {
		_dialog->displayChoices(_dialog->_currentChoices, _compositeBuffer);
	} else if (overlayMode == OVERLAY_PICKUP_ICON) {
		pickupIconFlash();
	} else if (overlayMode == OVERLAY_ACTION) {
		showActionBalloon(_actionPopupState.x, _actionPopupState.y, _actionPopupState.curFrame);
	}
}

void PelrockEngine::animateFadePalette(PaletteAnim *anim) {
	// FADE palette animation - cycles a single palette entry between min/max RGB values
	// Data layout (after loading from EXE):
	//   data[0] = current R
	//   data[1] = current G
	//   data[2] = current B
	//   data[3] = min R
	//   data[4] = min G
	//   data[5] = min B
	//   data[6] = max R
	//   data[7] = max G
	//   data[8] = max B
	//   data[9] = flags byte:
	//             bits 0-1: R increment
	//             bits 2-3: G increment
	//             bits 4-5: B increment
	//             bit 6: direction (0=decreasing toward min, 1=increasing toward max)

	byte flags = anim->data[9];
	// Increments are scaled by 4 (<<2) to match the shifted RGB values
	byte rInc = (flags & 0x03) << 2;
	byte gInc = ((flags >> 2) & 0x03) << 2;
	byte bInc = ((flags >> 4) & 0x03) << 2;
	bool increasing = (flags & 0x40) != 0;

	if (increasing) {
		// Increasing toward max values
		anim->data[0] += rInc;
		anim->data[1] += gInc;
		anim->data[2] += bInc;
		// Check if R reached max, then reverse direction
		if (anim->data[0] >= anim->data[6]) {
			anim->data[9] &= ~0x40; // Clear direction bit
		}
	} else {
		// Decreasing toward min values
		anim->data[0] -= rInc;
		anim->data[1] -= gInc;
		anim->data[2] -= bInc;
		// Check if R reached min, then reverse direction
		if (anim->data[0] <= anim->data[3]) {
			anim->data[9] |= 0x40; // Set direction bit
		}
	}

	_room->_roomPalette[anim->startIndex * 3] = anim->data[0];
	_room->_roomPalette[anim->startIndex * 3 + 1] = anim->data[1];
	_room->_roomPalette[anim->startIndex * 3 + 2] = anim->data[2];
	g_system->getPaletteManager()->setPalette(_room->_roomPalette, 0, 256);
}

void PelrockEngine::animateRotatePalette(PaletteAnim *anim) {

	if (anim->curFrame >= anim->data[1]) {
		anim->curFrame = 0;
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
		delete[] paletteValues;

		g_system->getPaletteManager()->setPalette(_room->_roomPalette, 0, 256);

	} else {
		anim->curFrame++;
	}
}

void PelrockEngine::doAction(VerbIcon action, HotSpot *hotspot) {
	switch (action) {
	case LOOK:
		lookAt(hotspot);
		break;
	case TALK:
		talkTo(hotspot);
		break;
	case PICKUP:
		if (_room->isPickableByExtra(hotspot->extra)) {
			pickUpAndDisable(hotspot);
		}
		executeAction(PICKUP, hotspot);
		break;
	case OPEN:
	case CLOSE:
	default:
		executeAction(action, hotspot);
		break;
	}
}

void PelrockEngine::talkTo(HotSpot *hotspot) {
	Sprite *animSet = nullptr;
	for (uint i = 0; i < _room->_currentRoomAnims.size(); i++) {
		if (_room->_currentRoomAnims[i].index == hotspot->index) {
			animSet = &_room->_currentRoomAnims[i];
			animSet->isTalking = true;
			break;
		}
	}
	changeCursor(DEFAULT);
	debug("Talking to hotspot %d (%d) with extra %d", hotspot->index, hotspot->isSprite ? hotspot->index : hotspot->index - _room->_currentRoomAnims.size(), hotspot->extra);
	_dialog->startConversation(_room->_conversationData, _room->_conversationDataSize, animSet->talkingAnimIndex, animSet);
}

void PelrockEngine::lookAt(HotSpot *hotspot) {
	debug("Looking at hotspot %d (%d) with extra %d", hotspot->index, hotspot->isSprite ? hotspot->index : hotspot->index - _room->_currentRoomAnims.size(), hotspot->extra);
	_dialog->sayAlfred(_room->_currentRoomDescriptions[_currentHotspot->index]);
	_actionPopupState.isActive = false;
}

void PelrockEngine::chooseAlfredStateAndDraw() {
	if (_alfredState.idleFrameCounter++ >= kAlfredIdleAnimationFrameCount &&
		_alfredState.animState == ALFRED_IDLE &&
		(_alfredState.direction == ALFRED_LEFT || _alfredState.direction == ALFRED_RIGHT)) {
		_alfredState.idleFrameCounter = 0;
		_alfredState.setState(ALFRED_COMB);
	}

	switch (_alfredState.animState) {
	case ALFRED_SKIP_DRAWING: {
		break;
	}
	case ALFRED_IDLE: {
		drawAlfred(_res->alfredIdle[_alfredState.direction]);
		break;
	}
	case ALFRED_WALKING: {
		MovementStep step = _currentContext.movementBuffer[_currentStep];
		if (step.distanceX > 0) {
			if (step.flags & MOVE_RIGHT) {
				_alfredState.direction = ALFRED_RIGHT;
				_alfredState.x += MIN(_alfredState.movementSpeedX, step.distanceX);
			}
			if (step.flags & MOVE_LEFT) {
				_alfredState.direction = ALFRED_LEFT;
				_alfredState.x -= MIN(_alfredState.movementSpeedX, step.distanceX);
			}
		}
		if (step.distanceY > 0) {
			if (step.flags & MOVE_DOWN) {
				_alfredState.direction = ALFRED_DOWN;
				_alfredState.y += MIN(_alfredState.movementSpeedY, step.distanceY);
			}
			if (step.flags & MOVE_UP) {
				_alfredState.direction = ALFRED_UP;
				_alfredState.y -= MIN(_alfredState.movementSpeedY, step.distanceY);
			}
		}

		if (step.distanceX > 0)
			step.distanceX -= MIN(_alfredState.movementSpeedX, step.distanceX);

		if (step.distanceY > 0)
			step.distanceY -= MIN(_alfredState.movementSpeedY, step.distanceY);

		if (step.distanceX <= 0 && step.distanceY <= 0) {
			_currentStep++;
			if (_currentStep >= _currentContext.movementCount) {
				_currentStep = 0;
				debug("Finished walking to target");
				_alfredState.setState(ALFRED_IDLE);
				_alfredState.isWalkingCancelable = true;
				_disableAction = false;
				if (_currentHotspot != nullptr)
					_alfredState.direction = calculateAlfredsDirection(_currentHotspot);
				drawAlfred(_res->alfredIdle[_alfredState.direction]);
				if (_queuedAction.isQueued) {
					// look and talk execute immediately, others need interaction animation first
					if (_queuedAction.verb == TALK || _queuedAction.verb == LOOK) {
						_queuedAction.isQueued = false;
						doAction(_queuedAction.verb, &_room->_currentRoomHotspots[_queuedAction.hotspotIndex]);
						break;
					}
					_alfredState.setState(ALFRED_INTERACTING);
					// Don't check exits when there's a queued action - action executes first
					break;
				}

				// Only check exits after walking is complete AND no queued action
				Exit *exit = isExitUnder(_alfredState.x, _alfredState.y);
				if (exit != nullptr) {
					debug("Using exit to room %d", exit->targetRoom);
					if(exit->targetRoom == 31 && _room->_currentRoomNumber == 32) {
						_res->loadAlfredSpecialAnim(8);
						_alfredState.setState(ALFRED_SPECIAL_ANIM);
						waitForSpecialAnimation();
					}
					_alfredState.x = exit->targetX;
					_alfredState.y = exit->targetY;
					setScreen(exit->targetRoom, exit->dir);
				}
			}
		} else {
			_currentContext.movementBuffer[_currentStep] = step;
		}

		if (_alfredState.curFrame >= walkingAnimLengths[_alfredState.direction]) {
			_alfredState.curFrame = 0;
		}
		if (_alfredState.animState == ALFRED_WALKING) { // in case it changed to idle above
			drawAlfred(_res->alfredWalkFrames[_alfredState.direction][_alfredState.curFrame]);
			_alfredState.curFrame++;
		}
		break;
	}
	case ALFRED_TALKING: {
		drawAlfred(_res->alfredTalkFrames[_alfredState.direction][_alfredState.curFrame]);
		if (_chrono->getFrameCount() % kAlfredAnimationSpeed == 0)
			_alfredState.curFrame++;
		if (_alfredState.curFrame >= talkingAnimLengths[_alfredState.direction] - 1) {
			_alfredState.curFrame = 0;
		}
		break;
	}
	case ALFRED_COMB: {

		drawAlfred(_res->alfredCombFrames[_alfredState.direction][_alfredState.curFrame]);
		_alfredState.curFrame++;
		if (_alfredState.curFrame >= 11) {
			_alfredState.setState(ALFRED_IDLE);
		}
		break;
	}
	case ALFRED_INTERACTING: {
		debug("Alfred interacting frame %d/%d, direction %d", _alfredState.curFrame, interactingAnimLength, _alfredState.direction);
		drawAlfred(_res->alfredInteractFrames[_alfredState.direction][_alfredState.curFrame]);
		_alfredState.curFrame++;
		if (_alfredState.curFrame >= interactingAnimLength) {
			if (_queuedAction.isQueued) {
				_queuedAction.isQueued = false;
				_alfredState.setState(ALFRED_IDLE);
				doAction(_queuedAction.verb, &_room->_currentRoomHotspots[_queuedAction.hotspotIndex]);
				break;
			}
		}
		break;
	}
	case ALFRED_SPECIAL_ANIM: {

		byte *frame = new byte[_res->_currentSpecialAnim->stride * _res->_currentSpecialAnim->numFrames];
		extractSingleFrame(_res->_currentSpecialAnim->animData,
						   frame,
						   _res->_currentSpecialAnim->curFrame,
						   _res->_currentSpecialAnim->w,
						   _res->_currentSpecialAnim->h);
		if (_res->_currentSpecialAnim->w == kAlfredFrameWidth && _res->_currentSpecialAnim->h == kAlfredFrameHeight) {
			drawAlfred(frame);
		} else {
			// Scale special anim frame to Alfred size before drawing
			drawSpriteToBuffer(_compositeBuffer, 640, frame, _alfredState.x, _alfredState.y - _res->_currentSpecialAnim->h, _res->_currentSpecialAnim->w, _res->_currentSpecialAnim->h, 255);
		}
		if (_chrono->getFrameCount() % kAlfredAnimationSpeed == 0) {
			_res->_currentSpecialAnim->curFrame++;

			if (_res->_currentSpecialAnim->curFrame >= _res->_currentSpecialAnim->numFrames) {
				if (_res->_currentSpecialAnim->curLoop < _res->_currentSpecialAnim->loopCount - 1) {
					_res->_currentSpecialAnim->curLoop++;
					_res->_currentSpecialAnim->curFrame = 0;
				} else {
					_alfredState.setState(ALFRED_IDLE);
					_res->clearSpecialAnim();
					_res->_isSpecialAnimFinished = true;
				}
			}
		}

		delete[] frame;
		break;
	}
	}
}

byte *PelrockEngine::scale(int scaleY, int finalWidth, int finalHeight, byte *buf) {

	// The scaling table is indexed by how many scanlines to skip (scaleY), not by final height
	int scaleIndex = scaleY;
	if (scaleIndex >= (int)_heightScalingTable.size()) {
		scaleIndex = _heightScalingTable.size() - 1;
	}
	if (scaleIndex < 0) {
		scaleIndex = 0;
	}
	int linesToSkip = kAlfredFrameHeight - finalHeight;

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
	return finalBuf;
}

/**
 * Scales and shades alfred sprite and draws it to the composite buffer
 */
void PelrockEngine::drawAlfred(byte *buf) {

	ScaleCalculation scaleCalc = calculateScaling(_alfredState.y, _room->_scaleParams);

	// Use the pre-calculated scaled dimensions from calculateScaling
	int finalHeight = scaleCalc.scaledHeight;
	int finalWidth = scaleCalc.scaledWidth;

	// Update Alfred's scale state for use by other functions
	_alfredState.w = finalWidth;
	_alfredState.h = finalHeight;

	if (finalHeight <= 0) {
		finalHeight = 1;
	}
	if (finalWidth <= 0) {
		finalWidth = 1;
	}

	byte *scaledBuf = scale(scaleCalc.scaleY, finalWidth, finalHeight, buf);
	delete[] _alfredSprite;
	_alfredSprite = scaledBuf;

	// Shadow detection: scan across Alfred's width at feet line.
	// Original game scans shadow buffer
	// at (topY + 0x66) * 640 + X + col for col = 0..width, where topY + 0x66 = feetY.
	// The shadow map value (0-3) indexes into the palette remap tables.
	byte shadowLevel = 0xFF; // 0xFF = no shadow
	int feetY = _alfredState.y;
	if (feetY >= 0 && feetY < 400 && _room->_pixelsShadows != nullptr) {
		for (int col = 0; col < finalWidth; col++) {
			int checkX = _alfredState.x + col;
			if (checkX >= 0 && checkX < 640) {
				byte shadowVal = _room->_pixelsShadows[feetY * 640 + checkX];
				if (shadowVal != 0xFF) {
					shadowLevel = shadowVal;
					break; // Original breaks on first shadow pixel found
				}
			}
		}
	}

	if (shadowLevel != 0xFF && shadowLevel < 4) {
		for (int i = 0; i < finalWidth * finalHeight; i++) {
			if (_alfredSprite[i] != 255) {
				// _alfredSprite[i] = _room->_paletteRemaps[3 - shadowLevel][_alfredSprite[i]];
				_alfredSprite[i] = _room->_paletteRemaps[0][_alfredSprite[i]];
			}
		}
	}

	drawSpriteToBuffer(_compositeBuffer, 640, _alfredSprite, _alfredState.x, _alfredState.y - finalHeight, finalWidth, finalHeight, 255);

	// Water reflection (rooms 25 and 45 only)
	if ((_room->_currentRoomNumber == 25 || _room->_currentRoomNumber == 45) && _alfredState.y >= 299) {
		// Offset from Alfred's feet to start of reflection
		int yOffset = (_room->_currentRoomNumber == 45) ? 25 : 13;
		reflectionEffect(_alfredSprite, _alfredState.x, _alfredState.y + yOffset, finalWidth, finalHeight);
	}
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

	int frameSize = sprite->stride;
	int curFrame = animData.curFrame;
	drawSpriteToBuffer(_compositeBuffer, 640, animData.animData[curFrame], x, y, w, h, 255);

	// Original in the game: increment FIRST, then check (not check-then-increment)
	animData.elpapsedFrames++;
	if (animData.elpapsedFrames >= animData.speed) {
		animData.elpapsedFrames = 0;
		if (animData.curFrame < animData.nframes - 1) {
			animData.curFrame++;
		} else {
			if (animData.curLoop < animData.loopCount - 1) {
				animData.curFrame = 0;
				animData.curLoop++;
			} else {
				if(sprite->disableAfterSequence) {
					sprite->zOrder = -1;
					return;
				}
				animData.curFrame = 0;
				animData.curLoop = 0;
				if (sprite->curAnimIndex < sprite->numAnims - 1) {
					sprite->curAnimIndex++;

					// Trigger ring on phone on every start of animation 2
					if (_room->_currentRoomNumber == 9 && sprite->index == 3) {
						if (sprite->curAnimIndex == 1 && animData.curLoop == 0) {
							byte soundFileIndex = _room->_roomSfx[2];
							_sound->playSound(soundFileIndex, 2);
						}
					}
				} else {
					sprite->curAnimIndex = 0;
				}
			}
		}
	}
}

void PelrockEngine::checkLongMouseClick(int x, int y) {
	_alfredState.idleFrameCounter = 0;
	int hotspotIndex = isHotspotUnder(x, y);
	bool alfredUnder = isAlfredUnder(x, y);
	if ((hotspotIndex != -1 || alfredUnder) && !_actionPopupState.isActive) {

		_actionPopupState.x = _alfredState.x + kAlfredFrameWidth / 2 - kBalloonWidth / 2;
		if (_actionPopupState.x < 0)
			_actionPopupState.x = 0;
		if (_actionPopupState.x + kBalloonWidth > 640) {
			_actionPopupState.x = 640 - kBalloonWidth;
		}

		_actionPopupState.y = _alfredState.y - kAlfredFrameHeight - kBalloonHeight;
		if (_actionPopupState.y < 0) {
			_actionPopupState.y = 0;
		}
		_actionPopupState.isActive = true;
		_actionPopupState.curFrame = 0;
		debug("Setting alfred under popup: %d", alfredUnder);
		_actionPopupState.isAlfredUnder = alfredUnder;
		if (hotspotIndex != -1) {
			_currentHotspot = &_room->_currentRoomHotspots[hotspotIndex];
		} else {
			_currentHotspot = nullptr;
		}
	}
}

void PelrockEngine::calculateScalingMasks() {

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
	// scaleY = amount to subtract from height (94 max for 0xFF mode)
	// scaleX = amount to subtract from width (47 max for 0xFF mode, = scaleY/2)
	int scaleY = 0;
	int scaleX = 0;
	if (scalingParams.scaleMode == 0xFF) {
		// Maximum scaling - character is very small (used for bird's eye view maps)
		scaleY = 0x5e; // 94
		scaleX = 0x2f; // 47
	} else if (scalingParams.scaleMode == 0xFE) {
		// No scaling - full size character
		scaleY = 0;
		scaleX = 0;
	} else if (scalingParams.scaleMode == 0) {
		// Dynamic scaling based on Y position
		if (scalingParams.yThreshold < yPos) {
			// Below threshold - no scaling
			scaleY = 0;
			scaleX = 0;
		} else {
			if (scalingParams.scaleDivisor != 0) {
				scaleY = (scalingParams.yThreshold - yPos) / scalingParams.scaleDivisor;
				scaleX = scaleY / 2;
			} else {
				scaleY = 0;
				scaleX = 0;
			}
		}
	} else {
		scaleY = 0;
		scaleX = 0;
	}

	// Original game formula: actual dimensions = base - scale amount
	int finalHeight = kAlfredFrameHeight - scaleY;
	if (finalHeight < 1)
		finalHeight = 1;

	int finalWidth = kAlfredFrameWidth - scaleX;
	if (finalWidth < 1)
		finalWidth = 1;

	ScaleCalculation scaleCalc;
	scaleCalc.scaledHeight = finalHeight;
	scaleCalc.scaledWidth = finalWidth;
	scaleCalc.scaleY = scaleY;
	scaleCalc.scaleX = scaleX;
	return scaleCalc;
}

int PelrockEngine::isHotspotUnder(int x, int y) {

	for (size_t i = 0; i < _room->_currentRoomHotspots.size(); i++) {
		HotSpot hotspot = _room->_currentRoomHotspots[i];
		if (hotspot.isEnabled &&
			x >= hotspot.x && x <= (hotspot.x + hotspot.w) &&
			y >= hotspot.y && y <= (hotspot.y + hotspot.h)) {
			// Check against sprite frame
			if (!hotspot.isSprite) {
				return hotspot.index;
			} else {
				Sprite *sprite = nullptr;
				for (size_t j = 0; j < _room->_currentRoomAnims.size(); j++) {
					if (_room->_currentRoomAnims[j].index == hotspot.index) {
						sprite = &(_room->_currentRoomAnims[j]);
						break;
					}
				}
				bool spriteUnder = isSpriteUnder(sprite, x, y);
				if (spriteUnder)
					return i;
				else
					continue;
			}
		}
	}
	return -1;
}

Exit *PelrockEngine::isExitUnder(int x, int y) {
	for (uint i = 0; i < _room->_currentRoomExits.size(); i++) {
		Exit exit = _room->_currentRoomExits[i];
		// Original game uses: x <= exit.x + exit.w - 1 and y <= exit.y + exit.h - 1
		if (x >= exit.x && x <= (exit.x + exit.w - 1) &&
			y >= exit.y && y <= (exit.y + exit.h - 1) && exit.isEnabled) {
			return &(_room->_currentRoomExits[i]);
		}
	}
	return nullptr;
}

/**
 * Checks if the given position is actually frame data or transparent pixel
 */
bool PelrockEngine::isSpriteUnder(Sprite *sprite, int x, int y) {
	if (sprite == nullptr) {
		return false;
	}
	Anim &animData = sprite->animData[sprite->curAnimIndex];
	int curFrame = animData.curFrame;

	int localX = x - animData.x;
	int localY = y - animData.y;
	if (localX >= 0 && localX < animData.w && localY >= 0 && localY < animData.h) {
		byte pixel = animData.animData[curFrame][localY * animData.w + localX];
		if (pixel != 255) {
			return true;
		}
	}
	return false;
}

Common::Point getPositionInBallonForIndex(int i, int x, int y) {
	return Common::Point(x + 20 + (i * (kVerbIconWidth + 2)), y + 20);
}

void PelrockEngine::showActionBalloon(int posx, int posy, int curFrame) {
	drawSpriteToBuffer(_compositeBuffer, 640, _res->_popUpBalloon + (curFrame * kBalloonHeight * kBalloonWidth), posx, posy, kBalloonWidth, kBalloonHeight, 255);
	Common::Array<VerbIcon> actions = availableActions(_currentHotspot);

	VerbIcon icon = isActionUnder(_events->_mouseX, _events->_mouseY);
	bool shouldBlink = _chrono->getFrameCount() % kIconBlinkPeriod == 0;
	for (uint i = 0; i < actions.size(); i++) {
		if (icon == actions[i] && shouldBlink) {
			continue;
		}
		Common::Point p = getPositionInBallonForIndex(i, posx, posy);
		drawSpriteToBuffer(_compositeBuffer, 640, _res->_verbIcons[actions[i]], p.x, p.y, kVerbIconWidth, kVerbIconHeight, 1);
	}
	if (_state->selectedInventoryItem >= 0 && !_state->inventoryItems.empty()) {
		if (icon == ITEM && shouldBlink) {
			return;
		}
		drawSpriteToBuffer(_compositeBuffer, 640, _res->getIconForObject(_state->selectedInventoryItem).iconData, posx + 20 + (actions.size() * (kVerbIconWidth + 2)), posy + 20, kVerbIconWidth, kVerbIconHeight, 1);
	}
	if (_actionPopupState.curFrame < 3) {
		_actionPopupState.curFrame++;
	} else {
		_actionPopupState.curFrame = 0;
	}
}

void PelrockEngine::animateTalkingNPC(Sprite *animSet) {
	// Change with the right index

	int index = animSet->talkingAnimIndex;
	TalkingAnims *animHeader = &_room->_talkingAnimHeader;

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

Common::Point getPositionInOverlayForIndex(uint index) {
	return Common::Point(5 + index * (60 + 2), 400 - 60 - 5);
}

void PelrockEngine::pickupIconFlash() {
	_graphics->showOverlay(65, _compositeBuffer);
	if (_newItem == -1)
		return;
	uint invSize = _state->inventoryItems.size();
	for (int i = 0; i < invSize; i++) {
		Common::Point p = getPositionInOverlayForIndex(i);
		drawSpriteToBuffer(_compositeBuffer, 640, _res->getIconForObject(_state->inventoryItems[i]).iconData, p.x, p.y, 60, 60, 1);
	}

	InventoryObject item = _res->getIconForObject(_newItem);
	if (_chrono->getFrameCount() % kIconBlinkPeriod == 0) {
		Common::Point p = getPositionInOverlayForIndex(invSize);
		debug("Drawing pickup icon for item %d at inventory index %d, position %d,%d", _newItem, invSize, p.x, p.y);
		drawSpriteToBuffer(_compositeBuffer, 640, item.iconData, p.x, p.y, 60, 60, 1);
	}
}

void PelrockEngine::gameLoop() {

	_events->pollEvent();
	checkMouse();
	if (_events->_lastKeyEvent == Common::KeyCode::KEYCODE_m) {
		travelToEgypt();
		_events->_lastKeyEvent = Common::KeyCode::KEYCODE_INVALID;
	}
	if (_events->_lastKeyEvent == Common::KeyCode::KEYCODE_n) {
		loadExtraScreenAndPresent(10);
		_events->_lastKeyEvent = Common::KeyCode::KEYCODE_INVALID;
	}
	if (_events->_lastKeyEvent == Common::KeyCode::KEYCODE_p) {
		antiPiracyEffect();
		_events->_lastKeyEvent = Common::KeyCode::KEYCODE_INVALID;
	}
	if (_events->_lastKeyEvent == Common::KeyCode::KEYCODE_s) {
		SpellBook spellBook(_events, _res);
		Spell *selectedSpell = spellBook.run();
		if(selectedSpell != nullptr) {
			_dialog->sayAlfred(selectedSpell->text);
		}
		_events->_lastKeyEvent = Common::KeyCode::KEYCODE_INVALID;
	}
	renderScene();
	// _events->waitForKey();
	_screen->update();
}

void PelrockEngine::computerLoop() {
	Computer computer(_events);
	computer.run();
	if (_state->selectedBookIndex != -1 && _state->bookLetter != '\0') {
		Common::StringArray lines;
		lines.push_back(Common::String::format(computer._memorizedMsg, _state->bookLetter));
		_dialog->sayAlfred(lines);
	}
}

void PelrockEngine::extraScreenLoop() {
	memcpy(_screen->getPixels(), _extraScreen, 640 * 400);
	while (!shouldQuit()) {
		_events->pollEvent();

		if (_events->_leftMouseClicked) {
			_events->_leftMouseClicked = false;
			break;
		}
		g_system->delayMillis(10);
		_screen->markAllDirty();
		_screen->update();
	}

	g_system->getPaletteManager()->setPalette(_room->_roomPalette, 0, 256);
	free(_extraScreen);
	_extraScreen = nullptr;
}

void PelrockEngine::walkLoop(int16 x, int16 y, AlfredDirection direction) {

	_alfredState.direction = direction;
	walkTo(x, y);
	while (!shouldQuit() && _alfredState.animState == ALFRED_WALKING) {
		_events->pollEvent();
		renderScene();
		_screen->update();
		g_system->delayMillis(10);
	}
	debug("Walk loop ended");
}

void PelrockEngine::walkTo(int x, int y) {
	_currentStep = 0;
	PathContext context = {nullptr, nullptr, nullptr, 0, 0, 0};
	findPath(_alfredState.x, _alfredState.y, x, y, _room->_currentRoomWalkboxes, &context, _currentHotspot);
	_currentContext = context;
	_alfredState.setState(ALFRED_WALKING);
}

void PelrockEngine::walkAndAction(HotSpot *hotspot, VerbIcon action) {
	_disableAction = true;
	walkTo(hotspot->x + hotspot->w / 2, hotspot->y + hotspot->h);
	_queuedAction = QueuedAction{action, hotspot->index, true};
}

AlfredDirection PelrockEngine::calculateAlfredsDirection(HotSpot *hotspot) {

	AlfredDirection calculatedDirection = ALFRED_DOWN;
	if (hotspot->isSprite) {
		// Check if Alfred's left edge is past sprite's right edge
		if (hotspot->x + hotspot->w < _alfredState.x) {
			calculatedDirection = ALFRED_LEFT; // Face LEFT
		}
		// Check if Alfred's right edge is before sprite's left edge
		else if ((_alfredState.x + _alfredState.w) < hotspot->x) {
			calculatedDirection = ALFRED_RIGHT; // Face RIGHT
		}
		// Alfred is horizontally overlapping with sprite
		else {
			// Check if Alfred's top is above sprite's bottom OR Alfred is within sprite's Y range
			if (((_alfredState.y + _alfredState.h) < hotspot->y) ||
				(_alfredState.y <= hotspot->y + hotspot->h &&
				 hotspot->zOrder <= ((399 - _alfredState.y) / 2) + 10)) {
				calculatedDirection = ALFRED_DOWN; // Face DOWN
			} else {
				calculatedDirection = ALFRED_UP; // Face UP
			}
		}
	} else {
		// Check if Alfred's left edge is past hotspot's right edge
		if (hotspot->x + hotspot->w < _alfredState.x) {
			calculatedDirection = ALFRED_LEFT; // Face LEFT
		}
		// Check if Alfred's right edge is before hotspot's left edge
		else if ((_alfredState.x + _alfredState.w) < hotspot->x) {
			calculatedDirection = ALFRED_RIGHT; // Face RIGHT
		}
		// Check vertical positioning
		else if (((_alfredState.y + _alfredState.h) < hotspot->y) ||
				 (_alfredState.y <= hotspot->y + hotspot->h &&
				  (hotspot->actionFlags & 0x80) == 0x80)) {
			calculatedDirection = ALFRED_DOWN; // Face DOWN
		} else {
			calculatedDirection = ALFRED_UP; // Face UP
		}
	}
	return calculatedDirection;
}

VerbIcon PelrockEngine::isActionUnder(int x, int y) {
	/*if (_currentHotspot == nullptr) {
		return NO_ACTION;
	}*/
	Common::Array<VerbIcon> actions = availableActions(_currentHotspot);
	int loopEnd = _state->selectedInventoryItem != -1 ? actions.size() + 1 : actions.size();
	for (int i = 0; i < loopEnd; i++) {
		Common::Point p = getPositionInBallonForIndex(i, _actionPopupState.x, _actionPopupState.y);
		Common::Rect actionRect = Common::Rect(p.x, p.y, p.x + kVerbIconWidth, p.y + kVerbIconHeight);
		if (i == actions.size()) {
			// Check inventory item
			if (actionRect.contains(x, y)) {
				return ITEM;
			}
		} else if (actionRect.contains(x, y)) {
			return actions[i];
		}
	}

	return NO_ACTION;
}

bool PelrockEngine::isAlfredUnder(int x, int y) {
	int localX = x - _alfredState.x;
	int localY = y - _alfredState.y + _alfredState.h; // Adjust for scaling (since Alfred's position is based on his feet, but sprite may be scaled from the top)
	if (_alfredSprite != nullptr && localX >= 0 && localX < _alfredState.w && localY >= 0 && localY < _alfredState.h) {
		byte pixel = _alfredSprite[localY * _alfredState.w + localX];
		if (pixel != 255) {
			return true;
		}
	}
	return false;
}

void PelrockEngine::checkMouseClick(int x, int y) {

	// This handles regular clicks (not popup selection)
	_queuedAction = QueuedAction{NO_ACTION, -1, false};
	_actionPopupState.isActive = false;
	_currentHotspot = nullptr;
	_alfredState.idleFrameCounter = 0;
	int hotspotIndex = isHotspotUnder(_events->_mouseX, _events->_mouseY);
	bool isHotspotUnder = false;
	if (hotspotIndex != -1) {
		isHotspotUnder = true;
	}
	_currentHotspot = isHotspotUnder ? &_room->_currentRoomHotspots[hotspotIndex] : nullptr;
	Common::Point walkTarget = calculateWalkTarget(_room->_currentRoomWalkboxes, _events->_mouseX, _events->_mouseY, isHotspotUnder, _currentHotspot);
	_curWalkTarget = walkTarget;

	walkTo(walkTarget.x, walkTarget.y);

	// { // For quick room navigation
	// 	Exit *exit = isExitUnder(walkTarget.x, walkTarget.y);

	// 	if (exit != nullptr) {
	// 		alfredState.x = exit->targetX;
	// 		alfredState.y = exit->targetY;

	// 		setScreen(exit->targetRoom, exit->dir);
	// 	} else {
	// 	}
	// }
}

void PelrockEngine::changeCursor(Cursor cursor) {
	CursorMan.replaceCursor(_res->_cursorMasks[cursor], kCursorWidth, kCursorHeight, 0, 0, 255);
}

void PelrockEngine::checkMouseHover() {

	bool hotspotDetected = false;
	int hotspotIndex = isHotspotUnder(_events->_mouseX, _events->_mouseY);

	bool alfredDetected = false;
	if (isAlfredUnder(_events->_mouseX, _events->_mouseY)) {
		alfredDetected = true;
		_hoveredMapLocation = "Alfred";
	}

	if (hotspotIndex != -1) {
		hotspotDetected = true;
		_hoveredMapLocation = _room->_currentRoomDescriptions[hotspotIndex].text;
	} else if (!alfredDetected) {
		_hoveredMapLocation = "";
	}

	if (_room->_currentRoomNumber == 21) {
		return;
	}

	if (isActionUnder(_events->_mouseX, _events->_mouseY) != NO_ACTION) {
		hotspotDetected = false;
	}

	// Calculate walk target first (before checking anything else)
	Common::Point walkTarget = calculateWalkTarget(_room->_currentRoomWalkboxes, _events->_mouseX, _events->_mouseY, hotspotDetected, hotspotDetected ? &_room->_currentRoomHotspots[hotspotIndex] : nullptr);

	// Check if walk target hits any exit
	bool exitDetected = false;
	Exit *exit = isExitUnder(walkTarget.x, walkTarget.y);
	if (exit != nullptr) {
		exitDetected = true;
	}

	if (hotspotDetected && exitDetected) {
		changeCursor(COMBINATION);
	} else if (hotspotDetected) {
		changeCursor(HOTSPOT);
	} else if (exitDetected) {
		changeCursor(EXIT);
	} else if (alfredDetected) {
		changeCursor(ALFRED);
	} else {
		changeCursor(DEFAULT);
	}
}

void PelrockEngine::setScreen(int roomNumber, AlfredDirection dir) {
	Common::File roomFile;
	if (!roomFile.open(Common::Path("ALFRED.1"))) {
		error("Could not open ALFRED.1");
		return;
	}
	_sound->stopAllSounds();
	_currentHotspot = nullptr;
	_alfredState.direction = dir;
	_alfredState.setState(ALFRED_IDLE);
	_currentStep = 0;
	int roomOffset = roomNumber * kRoomStructSize;
	_alfredState.curFrame = 0;

	byte *palette = new byte[256 * 3];
	_room->getPalette(&roomFile, roomOffset, palette);

	byte *background = new byte[640 * 400];
	_room->getBackground(&roomFile, roomOffset, background);

	_screen->clear();
	_screen->markAllDirty();
	_screen->update();

	Common::copy(background, background + 640 * 400, _currentBackground);
	copyBackgroundToBuffer();
	g_system->getPaletteManager()->setPalette(palette, 0, 256);

	_room->loadRoomMetadata(&roomFile, roomNumber);
	_room->loadRoomTalkingAnimations(roomNumber);
	if (_room->_musicTrack > 0)
		_sound->playMusicTrack(_room->_musicTrack);
	else {
		_sound->stopMusic();
	}

	if (findWalkboxForPoint(_room->_currentRoomWalkboxes, _alfredState.x, _alfredState.y) == 0xFF) {
		const WalkBox w = _room->_currentRoomWalkboxes[0];
		_alfredState.x = w.x;
		_alfredState.y = w.y;
	}

	_screen->markAllDirty();
	_screen->update();

	doExtraActions(roomNumber);
	roomFile.close();
	delete[] background;
	delete[] palette;
}

void PelrockEngine::loadExtraScreenAndPresent(int screenIndex) {

	byte *palette = new byte[768];
	if (_extraScreen == nullptr) {
		_extraScreen = new byte[640 * 400];
	}
	_res->getExtraScreen(screenIndex, _extraScreen, palette);
	CursorMan.showMouse(false);
	_graphics->clearScreen();
	g_system->getPaletteManager()->setPalette(palette, 0, 256);
	extraScreenLoop();
	CursorMan.showMouse(true);
	delete[] _extraScreen;
	delete[] palette;
	_screen->markAllDirty();
	_screen->update();
}

void PelrockEngine::waitForSpecialAnimation() {
	while (!g_engine->shouldQuit() && !_res->_isSpecialAnimFinished) {
		_events->pollEvent();
		renderScene(OVERLAY_NONE);
		// _events->waitForKey();
		_screen->update();
		g_system->delayMillis(10);
	}
}

void PelrockEngine::doExtraActions(int roomNumber) {
	switch (roomNumber) {
	case 4:
		if (_state->getFlag(FLAG_PUESTA_SALSA_PICANTE) && !_state->getFlag(FLAG_JEFE_ENCARCELADO)) {
			_state->setFlag(FLAG_JEFE_ENCARCELADO, true);
			_room->disableSprite(13, 0, true);
			loadExtraScreenAndPresent(4);
		}
		break;
	case 15:
		if (_state->getFlag(FLAG_ENTRA_EN_TIENDA_PRIMERA_VEZ)) {
			_state->setFlag(FLAG_ENTRA_EN_TIENDA_PRIMERA_VEZ, false);
			_dialog->say(_res->_ingameTexts[GAMBERROS]);
			_dialog->say(_res->_ingameTexts[QUIENYO]);
			_dialog->say(_res->_ingameTexts[PINTA_BUENAPERSONA]);
		}
	case 38: {
		if(_room->_prevRoomNumber == 30) {
			int x = _alfredState.x;
			int y = _alfredState.y;
			_alfredState.x -= 57;
			_alfredState.y += 2;
			_res->loadAlfredSpecialAnim(6);
			_alfredState.setState(ALFRED_SPECIAL_ANIM);
			waitForSpecialAnimation();

			_room->addStickerToRoom(38, 123, PERSIST_TEMP);
			_alfredState.x = x;
			_alfredState.y = y;
			break;
		}
	}
	case 32: {
		if(_room->_prevRoomNumber == 31) {
			int x = _alfredState.x;
			int y = _alfredState.y;
			_res->loadAlfredSpecialAnim(7);
			_alfredState.setState(ALFRED_SPECIAL_ANIM);
			waitForSpecialAnimation();
			_alfredState.x = x;
			_alfredState.y = y;

		}
	}
	case 27: {
		if(_room->_prevRoomNumber == 33) {
			int x = _alfredState.x;
			int y = _alfredState.y;
			_alfredState.x = 12;
			_alfredState.y += 10;
			_res->loadAlfredSpecialAnim(9);
			_alfredState.setState(ALFRED_SPECIAL_ANIM);
			waitForSpecialAnimation();
			_alfredState.x = x;
			_alfredState.y = y;
		}
		break;
	}
	case 28: {
		if(_state->getFlag(FLAG_CROCODILLO_ENCENDIDO) == true) {
			byte palette[768];
			_res->getPaletteForRoom28(palette);
			g_system->getPaletteManager()->setPalette(palette, 0, 256);
			Common::copy(palette, palette + 768, _room->_roomPalette);
			break;
		}
	}
	case 26: {
		if(_state->getFlag(FLAG_A_LA_CARCEL) == true) {
			if(_state->getFlag(FLAG_SE_HA_PUESTO_EL_MUNECO) == true) {
				_state->setCurrentRoot(26, 2, 1);
			} else {
				_dialog->say(_res->_ingameTexts[OIGAUSTED], 1);
				_dialog->say(_res->_ingameTexts[ESAMI]);
				_dialog->say(_res->_ingameTexts[VENGAAHORAMISMO], 1);
				_state->setCurrentRoot(26, 1, 1);
				walkAndAction(_room->findHotspotByExtra(421), TALK);
			}
		}
		break;
	}
	case 30: {
		if(_state->getFlag(FLAG_ROBA_PELO_PRINCESA) == true) {
			_state->setFlag(FLAG_ROBA_PELO_PRINCESA, false);
			_room->enableSprite(0, 200, PERSIST_TEMP);
			_dialog->say(_res->_ingameTexts[OIGAUSTED]);
			walkAndAction(_room->findHotspotByExtra(0), TALK);
		}
		break;
	}
	default:
		break;
	}
}

} // End of namespace Pelrock
