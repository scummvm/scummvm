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

#include "pelrock/actions.h"
#include "pelrock/computer.h"
#include "pelrock/console.h"
#include "pelrock/detection.h"
#include "pelrock/fonts/small_font.h"
#include "pelrock/offsets.h"
#include "pelrock/pathfinding.h"
#include "pelrock/pelrock.h"
#include "pelrock/slidingpuzzle.h"
#include "pelrock/util.h"

namespace Pelrock {

static const uint32 kInventoryArrowsOffset = 3186048; // ALFRED.7 — inventory scroll arrows

PelrockEngine *g_engine;

PelrockEngine::PelrockEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
																				 _gameDescription(gameDesc), _randomSource("Pelrock") {
	g_engine = this;
	_chrono = new ChronoManager();
	_events = new PelrockEventManager();
}

PelrockEngine::~PelrockEngine() {
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
	delete _state;
	delete[] _alfredSprite;
	delete[] _inventoryOverlayState.arrows[0];
	delete[] _inventoryOverlayState.arrows[1];
	// Free path-finding buffers (allocated via malloc in findPath)
	if(_currentContext.pathBuffer) {
		free(_currentContext.pathBuffer);
	}
	if(_currentContext.movementBuffer) {
		free(_currentContext.movementBuffer);
	}
}

uint32 PelrockEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String PelrockEngine::getGameId() const {
	return _gameDescription->gameId;
}

bool PelrockEngine::isAlternateTiming() const {
	return ConfMan.getBool("alternate_timing");
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

	bool playIntroFromConfig = ConfMan.getBool("play_intro");
	bool hasLaunchedBefore = ConfMan.getBool("launched_before");
	shouldPlayIntro = playIntroFromConfig || !hasLaunchedBefore;
	// Set the engine's debugger console
	setDebugger(new PelrockConsole(this));

	_state->stateGame = shouldPlayIntro ? INTRO : GAME;

	init();

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	while (!shouldQuit()) {
		if (_state->stateGame == SETTINGS) {
			_graphics->clearScreen();
			_menu->menuLoop();
			_state->stateGame = GAME;
		} else if (_state->stateGame == GAME) {
			maybePlayPostIntro();
			gameLoop();
		} else if (_state->stateGame == INTRO) {
			CursorMan.showMouse(false);
			_videoManager->playIntro();
			CursorMan.showMouse(true);
			ConfMan.setBool("launched_before", true);
			ConfMan.flushToDisk();
			_state->setFlag(FLAG_FROM_INTRO, true);
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

	_graphics->calculateScalingMasks();
	_compositeBuffer.create(640, 400, Graphics::PixelFormat::createFormatCLUT8());
	_currentBackground.create(640, 400, Graphics::PixelFormat::createFormatCLUT8());

	changeCursor(DEFAULT);
	CursorMan.showMouse(true);

	if (gameInitialized == false) {
		gameInitialized = true;
		loadAnims();
		setScreenAndPrepare(0, ALFRED_DOWN);
		// setScreenAndPrepare(36, ALFRED_LEFT);

		// setScreen(3, ALFRED_RIGHT);
		// setScreen(22, ALFRED_DOWN);
		// setScreen(41, ALFRED_DOWN);
		// setScreen(43, ALFRED_DOWN);
		// setScreen(46, ALFRED_RIGHT);
		// setScreen(0, ALFRED_DOWN);
		// setScreen(52, ALFRED_DOWN);
		// setScreen(15, ALFRED_DOWN);
		// setScreen(2, ALFRED_LEFT);
		// _alfredState.x = 576;
		// alfredState.y = 374;
	}

	loadInventoryArrows();
}

void PelrockEngine::loadInventoryArrows() {
	Common::File alfred7;
	if (!alfred7.open("ALFRED.7")) {
		error("Failed to open ALFRED.7 to load inventory arrows");
		return;
	}
	alfred7.seek(kInventoryArrowsOffset, SEEK_SET); // Inventory arrows in ALFRED.7
	_inventoryOverlayState.arrows[0] = new byte[20 * 60];
	_inventoryOverlayState.arrows[1] = new byte[20 * 60];
	alfred7.read(_inventoryOverlayState.arrows[0], 20 * 60);
	alfred7.read(_inventoryOverlayState.arrows[1], 20 * 60);
	alfred7.close();
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

	if (hotspot->actionFlags & kActionMaskOpen) {
		verbs.push_back(OPEN);
	}
	if (hotspot->actionFlags & kActionMaskClose) {
		verbs.push_back(CLOSE);
	}
	if (hotspot->actionFlags & kActionMaskUnknown) {
		verbs.push_back(UNKNOWN);
	}
	if (hotspot->actionFlags & kActionMaskPickup) {
		verbs.push_back(PICKUP);
	}
	if (hotspot->actionFlags & kActionMaskTalk) {
		verbs.push_back(TALK);
	}
	if (hotspot->actionFlags & kActionMaskPush) {
		verbs.push_back(PUSH);
	}
	if (hotspot->actionFlags & kActionMaskPull) {
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
	if (_disableAmbientSounds)
		return;
	// Get ambient slot offset (0-3) or -1 if no sound this frame
	int ambientSlotOffset = _sound->tickAmbientSound(_chrono->getFrameCount());
	if (ambientSlotOffset >= 0) {
		// Convert to room sound index: slots 12-15 = room indices 4-7
		byte roomSoundIndex = kAmbientSoundSlotBase + ambientSlotOffset;
		if (roomSoundIndex < _room->_roomSfx.size()) {
			byte soundFileIndex = _room->_roomSfx[roomSoundIndex];
			if (soundFileIndex != 0) { // 0 = NO_SOUND.SMP (disabled)
				// Don't play the same sound twice at the same time
				if (!_sound->isSoundIndexPlaying(soundFileIndex)) {
					_sound->playSound(soundFileIndex);
				}
			}
		}
	}
}

void PelrockEngine::travelToEgypt() {
	_graphics->fadeToBlack(10);

	_sound->playMusicTrack(26, false);
	byte *palette = new byte[768];
	if (!_bgScreen.getPixels()) {
		_bgScreen.create(640, 400, Graphics::PixelFormat::createFormatCLUT8());
	}
	_res->getExtraScreen(6, (byte *)_bgScreen.getPixels(), palette);
	CursorMan.showMouse(false);
	g_system->getPaletteManager()->setPalette(palette, 0, 256);

	_screen->blitFrom(_bgScreen);
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

		_screen->markAllDirty();
		_screen->update();
	}
	_graphics->clearScreen();

	g_system->getPaletteManager()->setPalette(_room->_roomPalette, 0, 256);
	_bgScreen.free();
	CursorMan.showMouse(true);
	delete[] palette;
	_screen->markAllDirty();
	_screen->update();

	_alfredState.x = 575;
	_alfredState.y = 210;
	setScreenAndPrepare(21, ALFRED_DOWN);

	// Original gives 4 items after room load (items 17, 64, 24, 59)
	_state->inventoryItems.clear();
	_state->selectedInventoryItem = -1;
	// we dont want a flashing animation in this case!
	_state->addInventoryItem(17);
	_state->addInventoryItem(64);
	_state->addInventoryItem(24);
	_state->addInventoryItem(59);
}

bool PelrockEngine::shouldSkipFrame() {
	if (isAlternateTiming()) {
		return false; // never skip frames in alternate timing mode
	}
	int skipAmount = 0;
	if (_alfredState.animState == ALFRED_WALKING) {
		skipAmount = 1;
	} else if (_dialog->_dialogActive) {
		if (_alfredState.animState == ALFRED_TALKING) {
			skipAmount = 1; // Alfred speaking
		} else if (_dialog->_isNPCTalking && _npcTalkSpeedByte > 0) {
			skipAmount = _npcTalkSpeedByte; // NPC speaking
		}
	}

	if (skipAmount > 0) {
		if (_renderSkipCounter < skipAmount) {
			_renderSkipCounter++;
			return true; // skip this tick
		}
		_renderSkipCounter = 0; // render this tick, reset
	} else {
		_renderSkipCounter = 0;
	}
	return false;
}

bool PelrockEngine::renderScene(int overlayMode) {

	_chrono->updateChrono();
	if (_chrono->_gameTick) {
		if (shouldSkipFrame()) {
			return false;
		}

		frameTriggers();

		playSoundIfNeeded();

		_graphics->copyBackgroundToBuffer();

		_graphics->placeStickersFirstPass();

		updateAnimations();

		_graphics->placeStickersSecondPass();

		renderOverlay(overlayMode);

		mouseHoverForMap();

		_graphics->presentFrame();

		_graphics->updatePaletteAnimations();

		// Execute deferred actions AFTER renderScene, so any scene changes
		// (addSticker, disableSprite, etc.) are in place before the next frame's
		// placeStickersFirstPass + presentFrame.
		if (_queuedAction.readyToExecute) {
			_queuedAction.readyToExecute = false;
			doAction(_queuedAction.verb, &_room->_currentRoomHotspots[_queuedAction.hotspotIndex]);
		}

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
					_dialog->say(_res->_ingameTexts[kTextYLosCondones]);
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
		drawText(_compositeBuffer, _largeFont, _hoveredMapLocation.c_str(), _events->_mouseX - r.width() / 2, _events->_mouseY - r.height(), 640, kAlfredColor);
	}
}

void PelrockEngine::frameTriggers() {
	uint32 frameCount = _chrono->getFrameCount();
	passerByAnim(frameCount);
	handleFightRoomFrame();
	shakeEffect();
	maybeHaveDogPee();
}

void PelrockEngine::maybeHaveDogPee() {

	if (_room->_currentRoomNumber != 19) {
		return;
	}
	Sprite * dog = _room->findSpriteByIndex(2);

	if(_alfredState.x < 146 && !_isDogPeeing) {
		_isDogPeeing = true;
		dog->animData[0].nframes = 24;
		while(!shouldQuit() && dog->animData[0].curFrame < 23) {
			_events->pollEvent();
			renderScene(OVERLAY_NONE);

			_screen->update();
			g_system->delayMillis(10);
		}
		dog->animData[0].nframes = 9;
		dog->animData[0].curFrame = 0;

		_dialog->say(_res->_ingameTexts[kTextQueAscoCasiMeMea]);
		_currentHotspot = nullptr; // Clear so arrival direction isn't overridden by dog hotspot
		walkTo(152, _alfredState.y);
		_isDogPeeing = false;
	}
}

void PelrockEngine::maybePlayPostIntro() {
	if (_state->getFlag(FLAG_FROM_INTRO) == true) {
		setScreenAndPrepare(0, ALFRED_LEFT);
		_alfredState.x = 396;
		_alfredState.y = 267;

		_res->loadAlfredSpecialAnim(16, false);
		_alfredState.animState = ALFRED_SPECIAL_ANIM;
		_dialog->_disableClickToAdvance = true;
		_dialog->say(_res->_ingameTexts[kTextVayaSuenho]);
		_dialog->_disableClickToAdvance = false;
		waitForSpecialAnimation();
		_graphics->fadeToBlack(20);
		g_system->getPaletteManager()->setPalette(_room->_roomPalette, 0, 256);
		_state->setFlag(FLAG_FROM_INTRO, false);
		_alfredState.direction = ALFRED_DOWN;
		_alfredState.x = kAlfredInitialPosX;
		_alfredState.y = kAlfredInitialPosY;
		// setScreenAndPrepare(0, ALFRED_DOWN);
		_dialog->say(_res->_ingameTexts[kTextMensajeOtraEpoca]);
	}
}

void PelrockEngine::shakeEffect() {
	if (!_shakeEffectState.enabled) {
		return;
	}

	_shakeEffectState.shakeX = (_chrono->getFrameCount() % 4 < 2) ? 2 : -2;
	g_system->setShakePos(_shakeEffectState.shakeX, _shakeEffectState.shakeY);
	_alfredState.x += (_shakeEffectState.shakeX / 2); // Adjust Alfred's position to counteract shake for better readability
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
			if (sprite && sprite->zOrder == 255) {
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
		// debug("Checking passerby anim %d for sprite %d, direction %d", _room->_passerByAnims->currentAnimIndex, spriteIndex, direction);
		Sprite *sprite = _room->findSpriteByIndex(spriteIndex);
		if (direction == kPasserbyRight) {
			// debug("Checking passerby anim for sprite %d moving RIGHT, curpos is %d", spriteIndex, sprite->x);
			if (sprite->x >= anim.resetCoord) {
				sprite->x = startX;
				sprite->y = startY;
				sprite->zOrder = 255;
				sprite->curAnimIndex = 0;
				sprite->animData[0].curFrame = 0;
				_room->_passerByAnims->latch = false;
			}
		} else if (direction == kPasserbyLeft) {
			// debug("Checking passerby anim for sprite %d moving LEFT, curpos is %d", spriteIndex, sprite->x);

			if (sprite->x <= anim.resetCoord) {
				sprite->x = startX;
				sprite->y = startY;
				sprite->zOrder = 255;
				sprite->curAnimIndex = 0;
				sprite->animData[0].curFrame = 0;
				_room->_passerByAnims->latch = false;
			}
		} else if (direction == kPasserbyDown) {
			// debug("Checking passerby anim for sprite %d moving DOWN, curpos is %d, reset %d", spriteIndex, sprite->y, anim.resetCoord);
			if (sprite->y >= anim.resetCoord) {
				sprite->x = startX;
				sprite->y = startY;
				sprite->zOrder = 255;
				sprite->curAnimIndex = 0;
				sprite->animData[0].curFrame = 0;
				_room->_passerByAnims->latch = false;
			}
		}
	}
}

void PelrockEngine::executeAction(VerbIcon action, HotSpot *hotspot) {
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

	if (_disableAction) {
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
		if (actionClicked != NO_ACTION && _currentHotspot != nullptr) {
			// Action was selected - queue it
			walkAndAction(_currentHotspot, actionClicked);
		} else if (_actionPopupState.isAlfredUnder && actionClicked != NO_ACTION) {
			debug("Using item on Alfred");
			useOnAlfred(_state->selectedInventoryItem);
		} else if (_inventoryOverlayState.isActive && _inventoryOverlayState.posInInventorySelectionArea(_events->_releaseX, _events->_releaseY)) {
			int item = checkMouseClickInventoryOverlay(_events->_releaseX, _events->_releaseY);
			_state->selectedInventoryItem = item;
			if(_actionPopupState.isAlfredUnder) {
				useOnAlfred(item);
			} else if (_currentHotspot != nullptr) {
				walkAndAction(_currentHotspot, ITEM);
			}
		} else {
			// Released outside popup - just close it
			_queuedAction = QueuedAction{NO_ACTION, -1, false, false};
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
		if (_room->_currentRoomAnims[i].zOrder > alfredZOrder || _room->_currentRoomAnims[i].zOrder == 255) {
			// debug("Drawing anim %d with zOrder %d in first pass (behind Alfred)", i, _room->_currentRoomAnims[i].zOrder);
			drawNextFrame(&_room->_currentRoomAnims[i]);
		}
	}

	// Draw Alfred
	chooseAlfredStateAndDraw();

	// Second pass: sprites in front of Alfred (sprite zOrder <= alfredZOrder)
	for (uint i = 0; i < _room->_currentRoomAnims.size(); i++) {
		if (_room->_currentRoomAnims[i].zOrder <= alfredZOrder && _room->_currentRoomAnims[i].zOrder != 255) {
			// debug("Drawing anim %d with zOrder %d in second pass (in front of Alfred)", i, _room->_currentRoomAnims[i].zOrder);
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

void PelrockEngine::renderOverlay(int overlayMode) {
	if (overlayMode == OVERLAY_CHOICES) {
		_dialog->displayChoices(_dialog->_currentChoices, _compositeBuffer);
	} else if (overlayMode == OVERLAY_PICKUP_ICON) {
		pickupIconFlash();
	} else if (overlayMode == OVERLAY_ACTION) {
		showActionBalloon(_actionPopupState.x, _actionPopupState.y, _actionPopupState.curFrame);
	}
}

void PelrockEngine::doAction(VerbIcon action, HotSpot *hotspot) {
	if (action == NO_ACTION) {
		return;
	}
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

	// Set NPC talk speed byte for original timing
	TalkingAnims *th = &_room->_talkingAnims;
	_npcTalkSpeedByte = animSet->talkingAnimIndex ? th->speedByteB : th->speedByteA;

	_dialog->startConversation(_room->_conversationData, _room->_conversationDataSize, animSet->talkingAnimIndex, animSet);

	// Conversation ended — clear NPC talk speed byte
	_npcTalkSpeedByte = 0;
	_renderSkipCounter = 0;
}

void PelrockEngine::lookAt(HotSpot *hotspot) {
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
		drawIdleFrame();
		break;
	}
	case ALFRED_WALKING: {
		MovementStep step = _currentContext.movementBuffer[_currentStep];
		if (step.distanceX > 0) {
			if (step.flags & kMoveRight) {
				_alfredState.direction = ALFRED_RIGHT;
				_alfredState.x += MIN(_alfredState.movementSpeedX, step.distanceX);
			}
			if (step.flags & kMoveLeft) {
				_alfredState.direction = ALFRED_LEFT;
				_alfredState.x -= MIN(_alfredState.movementSpeedX, step.distanceX);
			}
		}
		if (step.distanceY > 0) {
			if (step.flags & kMoveDown) {
				_alfredState.direction = ALFRED_DOWN;
				_alfredState.y += MIN(_alfredState.movementSpeedY, step.distanceY);
			}
			if (step.flags & kMoveUp) {
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
				_alfredState.setState(ALFRED_IDLE);
				_alfredState.isWalkingCancelable = true;
				_disableAction = false;
				if (_queuedAction.isQueued) {
					// When a queued action exists, face toward its target hotspot
					// (not _currentHotspot which is the mouse-hovered one)
					HotSpot *targetHotspot = &_room->_currentRoomHotspots[_queuedAction.hotspotIndex];
					_alfredState.direction = calculateAlfredsDirection(targetHotspot);
				} else if (_currentHotspot != nullptr) {
					_alfredState.direction = calculateAlfredsDirection(_currentHotspot);
				}
				drawIdleFrame();
				if (_queuedAction.isQueued) {
					// look and talk execute immediately, others need interaction animation first
					if (_queuedAction.verb == TALK || _queuedAction.verb == LOOK) {
						// Defer to after renderScene so any scene changes (stickers,
						// sprites) take effect before the next presentFrame.
						_queuedAction.isQueued = false;
						_queuedAction.readyToExecute = true;
						break;
					}
					_alfredState.setState(ALFRED_INTERACTING);
					// Don't check exits when there's a queued action - action executes first
					break;
				}

				// Only check exits after walking is complete AND no queued action
				Exit *exit = isExitUnder(_alfredState.x, _alfredState.y);
				if (exit != nullptr) {
					exitTriggers(exit);
					_alfredState.x = exit->targetX;
					_alfredState.y = exit->targetY;
					setScreenAndPrepare(exit->targetRoom, exit->dir);
					// setScreen() resets the composite buffer to the bare background;
					// place first-pass stickers now so they are present in the very
					// first presentFrame() for the new room (avoids a one-frame flash
					// without stickers).
					_graphics->placeStickersFirstPass();
				}
			}
		} else {
			_currentContext.movementBuffer[_currentStep] = step;
		}
		if (_room->_currentRoomNumber == 55) {
			if (_alfredState.curFrame >= 9) {
				_alfredState.curFrame = 0;
			}
			if (_alfredState.animState == ALFRED_WALKING) { // in case it changed to idle above
				debug("Drawing crawl frame %d for direction %d", _alfredState.curFrame, _alfredState.direction);
				drawSpriteToBuffer(_compositeBuffer, _res->alfredCrawlFrames[_alfredState.direction][_alfredState.curFrame], _alfredState.x, _alfredState.y - 55, 130, 55, 255);
				_alfredState.curFrame++;
			}
		} else {
			if (_alfredState.curFrame >= walkingAnimLengths[_alfredState.direction]) {
				_alfredState.curFrame = 0;
			}
			if (_alfredState.animState == ALFRED_WALKING) { // in case it changed to idle above

				drawAlfred(_res->alfredWalkFrames[_alfredState.direction][_alfredState.curFrame]);
				_alfredState.curFrame++;
			}
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
		drawAlfred(_res->alfredInteractFrames[_alfredState.direction][_alfredState.curFrame]);
		_alfredState.curFrame++;
		if (_alfredState.curFrame >= interactingAnimLength) {
			if (_queuedAction.isQueued) {
				// Defer to after renderScene so any scene changes (stickers,
				// sprites) take effect before the next presentFrame.
				_queuedAction.isQueued = false;
				_queuedAction.readyToExecute = true;
				_alfredState.setState(ALFRED_IDLE);
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
			drawSpriteToBuffer(_compositeBuffer, frame, _alfredState.x, _alfredState.y - _res->_currentSpecialAnim->h, _res->_currentSpecialAnim->w, _res->_currentSpecialAnim->h, 255);
		}
		if (_chrono->getFrameCount() % _res->_currentSpecialAnim->speed == 0) {
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

void PelrockEngine::exitTriggers(Pelrock::Exit *exit) {
	if (exit->targetRoom == 31 && _room->_currentRoomNumber == 32) {
		_res->loadAlfredSpecialAnim(8);
		_alfredState.setState(ALFRED_SPECIAL_ANIM);
		waitForSpecialAnimation();
	} else if (exit->targetRoom == 55 && _room->_currentRoomNumber == 44) {
		uint16 x = _alfredState.x;
		uint16 y = _alfredState.y;
		_alfredState.x -= 20;
		_alfredState.y += 5;
		_res->loadAlfredSpecialAnim(15);
		_alfredState.setState(ALFRED_SPECIAL_ANIM);
		waitForSpecialAnimation();
		_alfredState.x = x;
		_alfredState.y = y;
	} else if (exit->targetRoom == 48 && _room->_currentRoomNumber == 46) {
		smokeAnimation(-1);
		uint16 x = _alfredState.x;
		if (x < 282) {
			if (_state->getFlag(FLAG_PUERTA_BUENA) == 1) {
				_state->setFlag(FLAG_CORRECT_DOOR_CHOSEN, true);
			}
		} else {
			if (_state->getFlag(FLAG_PUERTA_BUENA) == 2) {
				_state->setFlag(FLAG_CORRECT_DOOR_CHOSEN, true);
			}
		}
	}
}

void PelrockEngine::drawIdleFrame() {
	if (_room->_currentRoomNumber == 55) {
		drawSpriteToBuffer(_compositeBuffer, _res->alfredCrawlFrames[_alfredState.direction][0], _alfredState.x, _alfredState.y - 55, 130, 55, 255);
	} else {
		drawAlfred(_res->alfredIdle[_alfredState.direction]);
	}
}

/**
 * Scales and shades alfred sprite and draws it to the composite buffer
 */
void PelrockEngine::drawAlfred(byte *buf) {

	ScaleCalculation scaleCalc = _graphics->calculateScaling(_alfredState.y, _room->_scaleParams);

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

	byte *scaledBuf = _graphics->scale(scaleCalc.scaleY, finalWidth, finalHeight, buf);
	delete[] _alfredSprite;
	_alfredSprite = scaledBuf;

	// Shadow detection: scan across Alfred's width at feet line.
	// Original game scans shadow buffer
	// at (topY + 0x66) * 640 + X + col for col = 0..width, where topY + 0x66 = feetY.
	// The shadow map value (0-3) indexes into the palette remap tables.
	if (_room->_pixelsShadows != nullptr) {
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
					_alfredSprite[i] = _room->_paletteRemaps[shadowLevel][_alfredSprite[i]];
					// _alfredSprite[i] = _room->_paletteRemaps[0][_alfredSprite[i]];
				}
			}
		}
	}

	drawSpriteToBuffer(_compositeBuffer, _alfredSprite, _alfredState.x, _alfredState.y - finalHeight, finalWidth, finalHeight, 255);

	// Water reflection (rooms 25 and 45 only)
	if ((_room->_currentRoomNumber == 25 || _room->_currentRoomNumber == 45) && _alfredState.y >= 299) {
		// Offset from Alfred's feet to start of reflection
		int yOffset = (_room->_currentRoomNumber == 45) ? 25 : 13;
		_graphics->reflectionEffect(_alfredSprite, _alfredState.x, _alfredState.y + yOffset, finalWidth, finalHeight);
	}
}

void applyMovement(int16 *x, int16 *y, byte *z, uint16 flags) {
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
	if (sprite->zOrder == 255) {
		// Skip disabled sprites (zOrder 0xFF = disabled in original game)
		return;
	}

	applyMovement(&(sprite->x), &(sprite->y), &(sprite->zOrder), animData.movementFlags);
	int x = sprite->x;
	int y = sprite->y;
	int w = sprite->w;
	int h = sprite->h;
	if (sprite->isTalking) {
		animateTalkingNPC(sprite);
		return;
	}

	int curFrame = animData.curFrame;
	if (curFrame >= animData.nframes) {
		debug("Warning: curFrame %d exceeds nframes %d for sprite %d anim %d", curFrame, animData.nframes, sprite->index, sprite->curAnimIndex);
		curFrame = 0;
	}
	drawSpriteToBuffer(_compositeBuffer, animData.animData[curFrame], x, y, w, h, 255);

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
				if (sprite->disableAfterSequence && sprite->curAnimIndex == sprite->numAnims - 1) {
					sprite->zOrder = 255;
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

void PelrockEngine::paintDebugLayer() {
	bool showWalkboxes = true;

	if (showWalkboxes) {
		for (uint i = 0; i < _room->_currentRoomWalkboxes.size(); i++) {
			WalkBox box = _room->_currentRoomWalkboxes[i];
			drawRect(_screen, box.x, box.y, box.w, box.h, 13);
		}
	}

	bool showSprites = true;
	if (showSprites) {
		for (uint i = 0; i < _room->_currentRoomAnims.size(); i++) {
			Sprite sprite = _room->_currentRoomAnims[i];
			if (sprite.zOrder == 255) {
				continue;
			}
			drawRect(_screen, sprite.x, sprite.y, sprite.w, sprite.h, 14);
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
		_screen->copyRectToSurface(_room->_pixelsShadows, 640, 0, 0, 640, 400);
	}
	_smallFont->drawString(_screen, Common::String::format("Room number: %d", _room->_currentRoomNumber), 0, 4, 640, 13);
	_smallFont->drawString(_screen, Common::String::format("Alfred pos: %d, %d (%d)", _alfredState.x, _alfredState.y, _alfredState.y - kAlfredFrameHeight), 0, 18, 640, 13);
	_smallFont->drawString(_screen, Common::String::format("Frame number: %d", _chrono->getFrameCount()), 0, 30, 640, 13);
}

void PelrockEngine::checkLongMouseClick(int x, int y) {
	_alfredState.idleFrameCounter = 0;
	int hotspotIndex = isHotspotUnder(x, y);
	bool alfredUnder = isAlfredUnder(x, y);
	if ((hotspotIndex != -1 || alfredUnder) && !_actionPopupState.isActive) {

		_actionPopupState.x = CLIP((int)_alfredState.x - 70, 1, 390);

		_actionPopupState.y = MAX(10, (int)_alfredState.y - (int)kAlfredFrameHeight - 102);
		_actionPopupState.isActive = true;
		_inventoryOverlayState.invStartingPos = -1;
		_actionPopupState.curFrame = 0;
		_actionPopupState.isAlfredUnder = alfredUnder;
		if (hotspotIndex != -1) {
			_currentHotspot = &_room->_currentRoomHotspots[hotspotIndex];
		} else {
			_currentHotspot = nullptr;
		}
	}
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

	int localX = x - sprite->x;
	int localY = y - sprite->y;
	if (localX >= 0 && localX < sprite->w && localY >= 0 && localY < sprite->h) {
		byte pixel = animData.animData[curFrame][localY * sprite->w + localX];
		if (pixel != 255) {
			return true;
		}
	}
	return false;
}

Common::Point getPositionInBalloonForIndex(int i, int x, int y) {
	return Common::Point(4 + x + (i * kVerbIconWidth), y + 18);
}

Common::Rect getActionArea(int x, int y) {
	Common::Point p1 = getPositionInBalloonForIndex(0, x, y);
	Common::Point p2 = getPositionInBalloonForIndex(4, x, y);
	return Common::Rect(p1.x, p1.y, p2.x + kVerbIconWidth, p2.y + kVerbIconHeight);
}

void PelrockEngine::showActionBalloon(int posx, int posy, int curFrame) {

	drawSpriteToBuffer(_compositeBuffer, _res->_popUpBalloon + (curFrame * kBalloonHeight * kBalloonWidth), posx, posy, kBalloonWidth, kBalloonHeight, 255);
	Common::Array<VerbIcon> actions = availableActions(_currentHotspot);

	VerbIcon icon = isActionUnder(_events->_mouseX, _events->_mouseY);

	bool shouldBlink = _chrono->getFrameCount() % kIconBlinkPeriod == 0;

	for (uint i = 0; i < actions.size(); i++) {
		if (icon == actions[i] && shouldBlink) {
			continue;
		}
		Common::Point p = getPositionInBalloonForIndex(i, posx, posy);
		drawSpriteToBuffer(_compositeBuffer, _res->_verbIcons[actions[i]], p.x, p.y, kVerbIconWidth, kVerbIconHeight, 1);
	}

	Common::Rect actionArea = getActionArea(posx, posy);
	// moving mouse over action area outside of the item closes the inventoryoverlay
	if (actionArea.contains(_events->_mouseX, _events->_mouseY)) {
		_inventoryOverlayState.isActive = icon == ITEM;
		_inventoryOverlayState.flashingIconIndex = -1;
	}

	if (_inventoryOverlayState.isActive) {
		// find selectedInventoryItem index in inventoryItems, set invStartingPos to that index if found, otherwise 0
		int scrollPos = getScrollPositionForItem(_state->selectedInventoryItem);
		if(_inventoryOverlayState.invStartingPos == -1) {
			_inventoryOverlayState.invStartingPos = scrollPos != -1 ? scrollPos : 0;
		}
		showInventoryOverlay();
		if (_inventoryOverlayState.posInInventorySelectionArea(_events->_mouseX, _events->_mouseY)) {
			_inventoryOverlayState.flashingIconIndex = -1;
			checkMouseOverInventoryOverlay(_events->_mouseX, _events->_mouseY);
		}
	}

	if (_state->selectedInventoryItem >= 0 && !_state->inventoryItems.empty()) {
		if (icon != ITEM || !shouldBlink) {
			Common::Point p = getPositionInBalloonForIndex(actions.size(), posx, posy);
			drawSpriteToBuffer(_compositeBuffer, _res->getIconForObject(_state->selectedInventoryItem).iconData, p.x, p.y, kVerbIconWidth, kVerbIconHeight, 1);
		}
	}

	if (_actionPopupState.curFrame < 3) {
		_actionPopupState.curFrame++;
	} else {
		_actionPopupState.curFrame = 0;
	}
}

int PelrockEngine::getScrollPositionForItem(int item) {
	int selectedIndex = -1;
	// assign selectedIndex to the index of the item
	for (size_t i = 0; i < _state->inventoryItems.size(); i++) {
		if (_state->inventoryItems[i] == item) {
			selectedIndex = i;
			break;
		}
	}
	// take the selectedIndex as the starting pos for calculatiom, it should ALWAYS be visible at the end of the overlay, if enough items
	if (selectedIndex != -1) {
		if (selectedIndex < _inventoryOverlayState.invStartingPos) {
			// do nothing
		} else if (selectedIndex >= _inventoryOverlayState.invStartingPos + kInventoryPageSize) {
			selectedIndex = selectedIndex - kInventoryPageSize + 1; // scroll right to show the selected item
		} else {
			selectedIndex = _inventoryOverlayState.invStartingPos; // no scrolling needed, keep current starting pos
		}
	}

	return selectedIndex;
}

void PelrockEngine::animateTalkingNPC(Sprite *animSet) {
	// Change with the right index

	int index = animSet->talkingAnimIndex;
	TalkingAnims *animHeader = &_room->_talkingAnims;

	int x = animSet->x + (index ? animHeader->offsetXAnimB : animHeader->offsetXAnimA);
	int y = animSet->y + (index ? animHeader->offsetYAnimB : animHeader->offsetYAnimA);

	int w = index ? animHeader->wAnimB : animHeader->wAnimA;
	int h = index ? animHeader->hAnimB : animHeader->hAnimA;
	int numFrames = index ? animHeader->numFramesAnimB : animHeader->numFramesAnimA;

	// In the original game, the render rate itself is throttled to match kTalkAnimationSpeed.
	// When in "Alternate timing" mode, we use kTalkAnimationSpeed to avoid slowing down the game loop.
	int npcTalkSpeed = isAlternateTiming() ? kTalkAnimationSpeed : 1;
	if (_chrono->getFrameCount() % npcTalkSpeed == 0) {
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

	drawSpriteToBuffer(_compositeBuffer, frame, x, y, w, h, 255);
}

Common::Point getPositionInOverlayForIndex(uint index) {
	return Common::Point(20 + index * (60), 340);
}

void PelrockEngine::pickupIconFlash() {
	uint invSize = _state->inventoryItems.size();
	// focus on the last positionin the inventory, where the newly picked up item would be, if there is at least 1 item in the inventory
	_inventoryOverlayState.invStartingPos = getScrollPositionForItem(_state->inventoryItems[invSize - 1]);
	_inventoryOverlayState.flashingIconIndex = invSize - 1;
	showInventoryOverlay();
}

void PelrockEngine::showInventoryOverlay() {
	_graphics->showOverlay(60, _compositeBuffer);
	uint invSize = _state->inventoryItems.size();
	// invStartingPos is an ITEM index (not a page number).
	// The original game scrolls 1 item at a time, not 1 page at a time.
	if(_inventoryOverlayState.invStartingPos == -1) {
		_inventoryOverlayState.invStartingPos = getScrollPositionForItem(_state->selectedInventoryItem);
		if(_inventoryOverlayState.invStartingPos == -1) {
			_inventoryOverlayState.invStartingPos = 0;
		}
	}
	int firstItem = _inventoryOverlayState.invStartingPos;

	for (int i = firstItem; i < (int)invSize && i < firstItem + kInventoryPageSize; i++) {
		Common::Point p = getPositionInOverlayForIndex(i - firstItem);
		if (i == _inventoryOverlayState.flashingIconIndex && _chrono->getFrameCount() % kIconBlinkPeriod == 0) {
			continue;
		}
		drawSpriteToBuffer(_compositeBuffer, _res->getIconForObject(_state->inventoryItems[i]).iconData, p.x, p.y, 60, 60, 1);
	}

	// draw arrows if there are more items to show in either direction
	if (_inventoryOverlayState.invStartingPos > 0) {
		drawSpriteToBuffer(_compositeBuffer, _inventoryOverlayState.arrows[0], 0, 340, 20, 60, 255);
	}
	if (firstItem + kInventoryPageSize < (int)invSize) {
		drawSpriteToBuffer(_compositeBuffer, _inventoryOverlayState.arrows[1], 620, 340, 20, 60, 255);
	}
}

void PelrockEngine::checkMouseOverInventoryOverlay(int x, int y) {
	// Original game: invStartingPos is an item index, scrolls 1 item per frame
	// with no frame throttling (scrolls every game tick = ~55ms).
	if (x < 20) {
		if (_inventoryOverlayState.invStartingPos > 0) {
			_inventoryOverlayState.invStartingPos--;
		}
		debug("Mouse at x=%d triggers scroll left, new invStartingPos=%d", x, _inventoryOverlayState.invStartingPos);
	} else if (x >= 620) {
		if (_inventoryOverlayState.invStartingPos + kInventoryPageSize < (int)_state->inventoryItems.size()) {
			_inventoryOverlayState.invStartingPos++;
		}
		debug("Mouse at x=%d triggers scroll right, new invStartingPos=%d", x, _inventoryOverlayState.invStartingPos);
	} else {
		// mouse hover over inventory item, laid out horizontally, y coordinate is not relevant
		int index = (x - 20) / 60 + _inventoryOverlayState.invStartingPos;
		if (index < (int)_state->inventoryItems.size()) {
			debug("hovering over inventory item %d at index %d", _state->inventoryItems[index], index);
			_inventoryOverlayState.flashingIconIndex = index;
		} else {
			debug("hovering over empty slot in inventory overlay, no item at index %d", index);
			_inventoryOverlayState.flashingIconIndex = -1;
		}
	}
}

int PelrockEngine::checkMouseClickInventoryOverlay(int x, int y) {

	if (x < 20) {
		return -1;
	} else if (x >= 620) {
		return -1;
	} else {
		// mouse hover over inventory item, laid out horizontally, y coordinate is not relevant
		int index = (x - 20) / 60 + _inventoryOverlayState.invStartingPos;
		if (index < (int)_state->inventoryItems.size()) {
			return _state->inventoryItems[index];
		}
		return -1;
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

	if (_events->_lastKeyEvent == Common::KeyCode::KEYCODE_w) {
		Computer computer(_events);

		computer.run();
		_events->_lastKeyEvent = Common::KeyCode::KEYCODE_INVALID;
	}

	if (_events->_lastKeyEvent == Common::KeyCode::KEYCODE_e && _room->_currentRoomNumber == 52) {
		teleportToPrincess();
		_events->_lastKeyEvent = Common::KeyCode::KEYCODE_INVALID;
	}

	if (_events->_lastKeyEvent == Common::KeyCode::KEYCODE_q) {
		endingScene();
		_events->_lastKeyEvent = Common::KeyCode::KEYCODE_INVALID;
	}
	if (_events->_lastKeyEvent == Common::KeyCode::KEYCODE_u) {
		if (_room->_currentRoomNumber == 36) {
			pyramidCollapse();
		}
		_events->_lastKeyEvent = Common::KeyCode::KEYCODE_INVALID;
	}
	if (_events->_lastKeyEvent == Common::KeyCode::KEYCODE_k) {
		_events->_lastKeyEvent = Common::KeyCode::KEYCODE_INVALID;
		credits();
	}
	if (_events->_lastKeyEvent == Common::KeyCode::KEYCODE_v) {
		_events->_lastKeyEvent = Common::KeyCode::KEYCODE_INVALID;
		SlidingPuzzle puzzle(_events, _sound);
		puzzle.run();
	}

	if (_events->_lastKeyEvent == Common::KeyCode::KEYCODE_h) {
		_events->_lastKeyEvent = Common::KeyCode::KEYCODE_INVALID;
		antiPiracyEffect();
	}

	renderScene();
	_screen->update();
}

void PelrockEngine::computerLoop() {
	Computer computer(_events);
	computer.run();
	if (_state->selectedBookIndex != -1 && _state->bookLetter != '\0') {
		Common::StringArray lines;
		lines.push_back(computer._memorizedMsg + _state->bookLetter);
		_dialog->sayAlfred(lines);
	}
}

void PelrockEngine::extraScreenLoop() {
	_screen->blitFrom(_bgScreen);
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
	_bgScreen.free();
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
	PathContext context = {nullptr, nullptr, 0, 0, 0};
	findPath(_alfredState.x, _alfredState.y, x, y, _room->_currentRoomWalkboxes, &context, _currentHotspot);
	_currentContext = context;
	_alfredState.setState(ALFRED_WALKING);
}

void PelrockEngine::walkAndAction(HotSpot *hotspot, VerbIcon action) {
	if(hotspot == nullptr) {
		return;
	}
	_disableAction = true;
	walkTo(hotspot->x + hotspot->w / 2, hotspot->y + hotspot->h);
	_queuedAction = QueuedAction{action, hotspot->index, true, false};
	waitForActionEnd();
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
	uint loopEnd = _state->selectedInventoryItem != -1 ? actions.size() + 1 : actions.size();
	for (uint i = 0; i < loopEnd; i++) {
		Common::Point p = getPositionInBalloonForIndex(i, _actionPopupState.x, _actionPopupState.y);
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
	_queuedAction = QueuedAction{NO_ACTION, -1, false, false};
	_actionPopupState.isActive = false;
	_currentHotspot = nullptr;
	_alfredState.idleFrameCounter = 0;
	int hotspotIndex = isHotspotUnder(_events->_mouseX, _events->_mouseY);
	bool isHotspotUnder = false;
	if (hotspotIndex != -1) {
		isHotspotUnder = true;
	}
	_currentHotspot = isHotspotUnder ? &_room->_currentRoomHotspots[hotspotIndex] : nullptr;
	Common::Point walkTarget = calculateWalkTarget(_room->_currentRoomWalkboxes, _events->_mouseX, _events->_mouseY, _currentHotspot);
	_curWalkTarget = walkTarget;

	walkTo(walkTarget.x, walkTarget.y);
}

void PelrockEngine::changeCursor(Cursor cursor) {
	CursorMan.replaceCursor(_res->_cursorMasks[cursor], kCursorWidth, kCursorHeight, 0, 0, 255);
}

void PelrockEngine::checkMouseHover() {
	if(_actionPopupState.isActive) {
		return;
	}

	bool hotspotDetected = false;
	int hotspotIndex = isHotspotUnder(_events->_mouseX, _events->_mouseY);

	bool alfredDetected = false;
	if (isAlfredUnder(_events->_mouseX, _events->_mouseY)) {
		alfredDetected = true;
		_hoveredMapLocation = "Alfred";
	}

	if (hotspotIndex != -1) {
		hotspotDetected = true;
		if (hotspotIndex < (int)_room->_currentRoomDescriptions.size())
			_hoveredMapLocation = _room->_currentRoomDescriptions[hotspotIndex].text;
	} else if (!alfredDetected) {
		_hoveredMapLocation = "";
	}

	if (_room->_currentRoomNumber == 21) {
		return;
	}

	// Calculate walk target first (before checking anything else)
	Common::Point walkTarget = calculateWalkTarget(_room->_currentRoomWalkboxes, _events->_mouseX, _events->_mouseY, hotspotDetected ? &_room->_currentRoomHotspots[hotspotIndex] : nullptr);

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

void PelrockEngine::setScreen(int roomNumber) {
	Common::File roomFile;
	if (!roomFile.open(Common::Path("ALFRED.1"))) {
		error("Could not open ALFRED.1");
		return;
	}
	changeCursor(DEFAULT);
	_sound->stopAllSounds();
	_currentHotspot = nullptr;
	_currentStep = 0;
	int roomOffset = roomNumber * kRoomStructSize;
	_alfredState.curFrame = 0;

	byte *palette = new byte[256 * 3];
	_room->getPalette(&roomFile, roomOffset, palette);
	memcpy(_room->_roomPalette, palette, 768);
	_currentBackground.create(640, 400, Graphics::PixelFormat::createFormatCLUT8());
	_room->getBackground(&roomFile, roomOffset, (byte *)_currentBackground.getPixels());

	_screen->clear();

	_graphics->copyBackgroundToBuffer();
	g_system->getPaletteManager()->setPalette(palette, 0, 256);

	_room->loadRoomMetadata(&roomFile, roomNumber);

	roomFile.close();
	delete[] palette;
}

void PelrockEngine::setScreenAndPrepare(int roomNumber, AlfredDirection dir) {
	setScreen(roomNumber);
	_alfredState.direction = dir;
	_alfredState.setState(ALFRED_IDLE);
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

	doExtraActions(roomNumber);
}

void PelrockEngine::loadExtraScreenAndPresent(int screenIndex) {

	byte *palette = new byte[768];
	if (!_bgScreen.getPixels()) {
		_bgScreen.create(640, 400, Graphics::PixelFormat::createFormatCLUT8());
	}
	_res->getExtraScreen(screenIndex, (byte *)_bgScreen.getPixels(), palette);
	CursorMan.showMouse(false);
	_graphics->clearScreen();
	g_system->getPaletteManager()->setPalette(palette, 0, 256);
	extraScreenLoop();
	CursorMan.showMouse(true);
	delete[] palette;
	_screen->markAllDirty();
	_screen->update();
}

void PelrockEngine::waitForSpecialAnimation() {
	while (!g_engine->shouldQuit() && !_res->_isSpecialAnimFinished) {
		_events->pollEvent();
		renderScene(OVERLAY_NONE);
		_screen->update();
		g_system->delayMillis(10);
	}
}

void PelrockEngine::doExtraActions(int roomNumber) {
	// Reset goodbye-disabled flag each room load. Specific rooms set it below.
	_dialog->_goodbyeDisabled = false;

	switch (roomNumber) {
	case 4:
	case 0:
		if (_state->getFlag(FLAG_PUESTA_SALSA_PICANTE) && !_state->getFlag(FLAG_JEFE_ENCARCELADO)) {
			_state->setFlag(FLAG_JEFE_ENCARCELADO, true);
			_room->disableSprite(13, 0, PERSIST_BOTH);
			loadExtraScreenAndPresent(4);
		}
		break;
	case 7: { // not in the original but it was clearly a bug
		_dialog->_goodbyeDisabled = true;
		break;
	}
	case 15:
		if (_state->getFlag(FLAG_ENTRA_EN_TIENDA_PRIMERA_VEZ)) {
			_state->setFlag(FLAG_ENTRA_EN_TIENDA_PRIMERA_VEZ, false);
			_dialog->say(_res->_ingameTexts[kTextGamberros]);
			_dialog->say(_res->_ingameTexts[kTextQuienYo]);
			_dialog->say(_res->_ingameTexts[kTextPintaBuenaPersona]);
		}
		break;
	case 19: {
		Sprite *dog = _room->findSpriteByIndex(2);
		dog->animData[0].nframes = 9;
		break;
	}
	case 24: {
		_room->findSpriteByIndex(1)->numAnims = 1;

		if(_state->hasInventoryItem(88) && _state->getFlag(FLAG_PIGEON_DEAD) == false) {
			_dialog->say(_res->_ingameTexts[kTextProbarLibro]);
			playAlfredSpecialAnim(0);
			Sprite *pigeons = _room->findSpriteByIndex(1);
			pigeons->numAnims = 4;
			pigeons->curAnimIndex = 0;
			pigeons->disableAfterSequence = true;
			pigeons->animData[0].curFrame = 0;
			while(!g_engine->shouldQuit() && pigeons->zOrder != 255) {
				_events->pollEvent();
				renderScene();
				_screen->update();
				g_system->delayMillis(10);
			}
			_dialog->say(_res->_ingameTexts[kTextPracticarMas]);
			_state->setFlag(FLAG_PIGEON_DEAD, true);
		}
		break;
	}
	case 38: {
		if (_room->_prevRoomNumber == 30) {
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
		}
		break;
	}
	case 32: {
		if (_room->_prevRoomNumber == 31) {
			int x = _alfredState.x;
			int y = _alfredState.y;
			_res->loadAlfredSpecialAnim(7);
			_alfredState.setState(ALFRED_SPECIAL_ANIM);
			waitForSpecialAnimation();
			_alfredState.x = x;
			_alfredState.y = y;
		}
		break;
	}
	case 27: {
		if (_room->_prevRoomNumber == 33) {
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
		if (_state->getFlag(FLAG_CROCODILLO_ENCENDIDO) == true) {
			byte palette[768];
			_res->getPaletteForRoom28(palette);
			g_system->getPaletteManager()->setPalette(palette, 0, 256);
			Common::copy(palette, palette + 768, _room->_roomPalette);
		}
		break;
	}
	case 26: {
		if (_state->getFlag(FLAG_A_LA_CARCEL) == true) {
			_dialog->_goodbyeDisabled = true;
			if (_state->getFlag(FLAG_SE_HA_PUESTO_EL_MUNECO) == true) {
				_state->setCurrentRoot(26, 2, 1);
			} else {
				_dialog->say(_res->_ingameTexts[kTextOigaUsted], 1);
				_dialog->say(_res->_ingameTexts[kTextEsAMi]);
				_dialog->say(_res->_ingameTexts[kTextVengaAhoraMismo], 1);
				_state->setCurrentRoot(26, 1, 1);
				walkAndAction(_room->findHotspotByExtra(421), TALK);
			}
		}
		break;
	}
	case 30: {
		if (_state->getFlag(FLAG_ROBA_PELO_PRINCESA) == true) {
			_dialog->_goodbyeDisabled = true;
			_state->setFlag(FLAG_ROBA_PELO_PRINCESA, false);
			_room->enableSprite(0, 200, PERSIST_TEMP);
			_dialog->say(_res->_ingameTexts[kTextOigaUsted]);
			walkAndAction(_room->findHotspotByExtra(0), TALK);
		}
		break;
	}
	case 36: {
		if (_shakeEffectState.enabled) {
			_shakeEffectState.disable();
		}
		if (_state->getFlag(FLAG_PIRAMIDE_JODIDA) == true &&
			// _state->getFlag(FLAG_VIGILANTE_MEANDO) == true &&
			_state->getFlag(FLAG_PIRAMIDE_JODIDA2) == false) {
			_state->setFlag(FLAG_VIGILANTE_MEANDO, false);
			_state->setFlag(FLAG_PIRAMIDE_JODIDA2, true);
			debug("Pyramid is now active!");
			pyramidCollapse();
		}
		break;
	}
	case 39:
	case 40:
		// Rooms 39/40 are the pharaoh's guard and throne room — all conversation
		// paths lead to capture, no voluntary exit allowed.
		_dialog->_goodbyeDisabled = true;
		break;
	case 10: {
		// _events->waitForKey();
		// while(!shouldQuit()) {
		// 	playSpecialAnim(212915, true, 287, 152, 62, 58, 10);
		// 	playSpecialAnim(236645, true, 287, 152, 62, 58, 10);
		// 	// setScreen(11, ALFRED_DOWN);
		// 	playSpecialAnim(261449, true, 0, 223, 64, 97, 8);
		// }
		break;
	}
	case 48: {
		_dialog->_goodbyeDisabled = true;

		if (_state->getFlag(FLAG_END_OF_GAME) == true) {

			_dialog->say(_res->_ingameTexts[kTextOhMiSalvador]);
			_dialog->say(_res->_ingameTexts[kTextVoyPoriPrincesa]);
			_state->setCurrentRoot(48, 1, 0);
			walkAndAction(_room->findHotspotByExtra(634), TALK);

			endingScene();

		} else if (_state->getFlag(FLAG_CORRECT_DOOR_CHOSEN) == true) {
			_dialog->say(_res->_ingameTexts[kTextOhMiSalvador]);
			_dialog->say(_res->_ingameTexts[kTextVoyPoriPrincesa]);
			_state->setFlag(FLAG_TRAMPILLA_ABIERTA, true);
			walkAndAction(_room->findHotspotByExtra(634), TALK);
			_room->addSticker(134);
			// wait a few frames
			int framesToWait = 0;
			while (!shouldQuit() && framesToWait < 10) {
				_events->pollEvent();

				bool didRender = renderScene(OVERLAY_NONE);
				if (didRender)
					framesToWait++;
				_screen->update();
				g_system->delayMillis(10);
			}
			_alfredState.x = 294;
			_alfredState.y = 387;
			_room->addSticker(136);
			setScreenAndPrepare(49, ALFRED_UP);

		} else {
			_dialog->say(_res->_ingameTexts[kTextOhMiSalvador]);
			_dialog->say(_res->_ingameTexts[kTextVoyPoriPrincesa]);
			// _state->setCurrentRoot(48, 0, 1);
			HotSpot *fatMummy = nullptr;
			for (uint i = 0; i < _room->_currentRoomHotspots.size(); i++) {
				if (_room->_currentRoomHotspots[i].isSprite && _room->_currentRoomHotspots[i].index == 1) {
					fatMummy = &_room->_currentRoomHotspots[i];
				}
			}
			walkAndAction(fatMummy, NO_ACTION);
			smokeAnimation(0);
			smokeAnimation(1, false);

			walkAndAction(fatMummy, TALK);
			_state->clear();
			_alfredState.x = kAlfredInitialPosX;
			_alfredState.y = kAlfredInitialPosY;
			_graphics->fadeToBlack(20);
			_state->setFlag(FLAG_FROM_INTRO, true);
			setScreenAndPrepare(0, ALFRED_LEFT);
		}
		break;
	}
	case 51:
	case 52:
	case 53:
	case 54:
		initGodsSequences(_room->_currentRoomNumber);
		if(roomNumber == 52) {
			_room->addStickerToRoom(52, 105);
		}
		break;
	default:
		break;
	}
}

void PelrockEngine::pyramidCollapse() {
	// === Pyramid Collapse Sequence (Room 36 per-frame handler at 0x1098F) ===
	// Binary: sprite index 2 = collapse animation, sprite index 0 = NPC guard.
	// Original sprite table indices are offset by 2 from ScummVM indices due
	// to the 2 header sprite slots in the room data.

	// Hide NPC initially — binary sets sprite_2 field 0x21 = 0xFF (zOrder = 255)
	Sprite *npc = _room->findSpriteByIndex(0);
	if (npc)
		npc->zOrder = 255;

	// Start collapse animation — binary sets sprite_4 field 0x21 = 0xFE (zOrder = 254)
	Sprite *collapseSprite = _room->findSpriteByIndex(2);
	if (collapseSprite)
		collapseSprite->zOrder = 254;

	// Play collapse sound
	_sound->playSound("QUAKE1ZZ.SMP", 0);

	// ----- PHASE 1: Wait for collapse animation frame 5 -----
	// Binary: loop sleep(0x69) + tick + render, check sprite_4 field 0x20 == 5
	while (!shouldQuit()) {
		_events->pollEvent();
		renderScene(OVERLAY_NONE);
		_screen->update();
		g_system->delayMillis(10);
		collapseSprite = _room->findSpriteByIndex(2);
		if (!collapseSprite || collapseSprite->animData[collapseSprite->curAnimIndex].curFrame >= 5) {
			collapseSprite->zOrder = 255; // Hide collapse animation sprite after frame 5
			break;
		}
	}
	debug("Collapse animation reached frame 5, applying background changes");

	// ----- PHASE 2: Background tile copies (hide pyramid top) -----
	// Copy 1: 99×45 from secondary buffer to front buffer (fills collapsed area)
	{
		static const int srcX = 240, srcY = 145;
		// static const int dstX = 510, dstY = 33;
		static const int copyW = 99, copyH = 45;
		Common::Rect copyRect(srcX, srcY, srcX + copyW, srcY + copyH);
		_currentBackground.blitFrom(_compositeBuffer, copyRect, Common::Point(srcX, srcY));
	}
	_room->findSpriteByIndex(2)->zOrder = 255;

	_dialog->say(_res->_ingameTexts[kTextYaNoSeHaceOnComoAntes]);
	npc = _room->findSpriteByIndex(0);
	if (npc)
		npc->zOrder = 254;

	npc = _room->findSpriteByIndex(0);
	npc->animData[0].nframes = 5;
	if (npc) {
		npc->animData[npc->curAnimIndex].movementFlags = 0x1C;
		npc->y -= 25; // One-time nudge upward to emerge from behind pyramid
	}
	while (!shouldQuit()) {
		_events->pollEvent();
		renderScene(OVERLAY_NONE);
		_screen->update();
		g_system->delayMillis(10);
		npc = _room->findSpriteByIndex(0);
		if (!npc || npc->x >= 339)
			break;
	}

	npc = _room->findSpriteByIndex(0);
	if (npc)
		npc->animData[npc->curAnimIndex].movementFlags = 0x340;
	while (!shouldQuit()) {
		_events->pollEvent();
		renderScene(OVERLAY_NONE);
		_screen->update();
		g_system->delayMillis(10);
		npc = _room->findSpriteByIndex(0);
		if (!npc || npc->y >= 206)
			break;
	}

	npc = _room->findSpriteByIndex(0);
	if (npc)
		npc->animData[npc->curAnimIndex].movementFlags = 0x14;
	while (!shouldQuit()) {
		_events->pollEvent();
		renderScene(OVERLAY_NONE);
		_screen->update();
		g_system->delayMillis(10);
		npc = _room->findSpriteByIndex(0);
		if (!npc || npc->x <= 307)
			break;
	}

	// Stop NPC movement
	npc = _room->findSpriteByIndex(0);
	npc->animData[0].nframes = 1;
	if (npc)
		npc->animData[npc->curAnimIndex].movementFlags = 0;

	_dialog->say(_res->_ingameTexts[kTextPor5Minutos], 0);

	_room->disableExit(36, 0);

	_room->addStickerToRoom(21, 79);
	_room->disableExit(21, 2, PERSIST_BOTH);
	HotSpot *pyramidHotspot = new HotSpot();
	pyramidHotspot->x = 510;
	pyramidHotspot->y = 33;
	pyramidHotspot->w = 99;
	pyramidHotspot->h = 45;
	pyramidHotspot->extra = 411;
	pyramidHotspot->isEnabled = false;
	pyramidHotspot->innerIndex = 2;
	pyramidHotspot->index = 7;
	_room->disableHotspot(21, pyramidHotspot, PERSIST_BOTH);

	_dialog->say(_res->_ingameTexts[kTextTaLuegoLucas]);

	// Walk Alfred to right edge exit -> room 21
	walkLoop(603, 212, ALFRED_RIGHT);
}

void PelrockEngine::endingScene() {
	byte *palette = new byte[768];
	if (!_bgScreen.getPixels()) {
		_bgScreen.create(640, 400, Graphics::PixelFormat::createFormatCLUT8());
	}
	_res->getExtraScreen(14, (byte *)_bgScreen.getPixels(), palette);
	CursorMan.showMouse(false);
	_graphics->clearScreen();
	g_system->getPaletteManager()->setPalette(palette, 0, 256);
	Common::File alfred7;
	if (!alfred7.open(Common::Path("ALFRED.7"))) {
		error("Could not open ALFRED.7");
		return;
	}
	byte *decompressedBuf = nullptr;
	size_t decompressedSize = 0;
	rleDecompressSingleBuda(&alfred7, 3222250, decompressedBuf, decompressedSize);
	alfred7.close();

	int animValues[4][8] = {
		{426, 211, 114, 189, 2, 2, 0, 0}, // Legs anim values (2 frames)
		{287, 68, 42, 26, 3, 1, 1, 15},   // Eyes anim values (3 frames)
		{172, 173, 93, 71, 3, 1, 3, 17},  // Alfred hand anim values (3 frames)
		{241, 334, 55, 66, 2, 2, 0, 0}    // Hand anim values (2 frames)
	};

	uint32 pos = 0;
	Common::Array<Sprite *> sprites;
	for (int i = 0; i < 4; i++) {
		Sprite *sprite = new Sprite();
		sprite->x = animValues[i][0];
		sprite->y = animValues[i][1];
		sprite->w = animValues[i][2];
		sprite->h = animValues[i][3];
		sprite->stride = animValues[i][2] * animValues[i][3];
		bool isIdleAnim = animValues[i][7] > 0;
		if (isIdleAnim) {
			sprite->numAnims = 2;
		} else {
			sprite->numAnims = 1;
		}

		sprite->animData = new Anim[sprite->numAnims];
		Anim mainAnim;
		mainAnim.nframes = animValues[i][4];
		mainAnim.loopCount = animValues[i][6];
		mainAnim.speed = animValues[i][5];

		byte *legsAnimData = new byte[sprite->stride * mainAnim.nframes];
		Common::copy(decompressedBuf + pos, decompressedBuf + pos + sprite->stride * mainAnim.nframes, legsAnimData);
		mainAnim.animData = new byte *[mainAnim.nframes];
		for (int j = 0; j < mainAnim.nframes; j++) {
			mainAnim.animData[j] = new byte[sprite->stride];
			extractSingleFrame(legsAnimData, mainAnim.animData[j], j, sprite->w, sprite->h);
		}

		if (isIdleAnim) {
			Anim idleAnim;
			idleAnim.nframes = 1;
			idleAnim.loopCount = 1;
			idleAnim.speed = animValues[i][7];
			idleAnim.animData = new byte *[1];
			idleAnim.animData[0] = new byte[sprite->stride];
			extractSingleFrame(legsAnimData, idleAnim.animData[0], 0, sprite->w, sprite->h);
			sprite->animData[0] = idleAnim;
			sprite->animData[1] = mainAnim;
		} else {
			sprite->animData[0] = mainAnim;
		}

		pos += sprite->stride * mainAnim.nframes;
		delete[] legsAnimData;
		sprites.push_back(sprite);
	}

	Common::Rect bbox1 = _largeFont->getBoundingBox("ALFRED PELROCK");
	int y1 = 400 / 2 - bbox1.height() / 2;
	int y2 = 400 / 2 + bbox1.height() / 2;
	int ticks = 0;
	_sound->playMusicTrack(3);
	// Loop runs indefinitely — original game waits for any keypress before advancing to credits.
	_events->_lastKeyEvent = Common::KEYCODE_INVALID;
	while (!shouldQuit()) {
		_events->pollEvent();

		_chrono->updateChrono();

		if (_chrono->_gameTick) {
			_compositeBuffer.blitFrom(_bgScreen);

			for (Sprite *sprite : sprites) {
				drawNextFrame(sprite);
			}

			_screen->blitFrom(_compositeBuffer);
			// Title text visible frames 21–149 (original: frame > 0x14 && frame < 0x96)
			if (ticks > 20 && ticks < 150) {
				drawText(_largeFont, "ALFRED PELROCK", 0, y1, 640, 255);
				drawText(_largeFont, "En busca de un sue\x80o", 0, y2, 640, 255);
			}
			ticks++;
		}

		// Any keypress advances to credits (original: infinite loop until input)
		if (_events->_lastKeyEvent != Common::KEYCODE_INVALID) {
			_events->_lastKeyEvent = Common::KEYCODE_INVALID;
			break;
		}

		g_system->delayMillis(10);
		_screen->markAllDirty();
		_screen->update();
	}

	_screen->clear(0);
	g_system->getPaletteManager()->setPalette(_room->_roomPalette, 0, 256);
	_bgScreen.free();

	CursorMan.showMouse(true);
	delete[] palette;
	_screen->markAllDirty();
	_screen->update();
	if (shouldQuit())
		return;
	credits();
}

void PelrockEngine::credits() {
	_sound->playMusicTrack(3);
	// 25-page room slideshow: each page loads a game room and overlays credit texts.
	static const int kNumCreditPages = 25;
	static const int kCreditRooms[kNumCreditPages] = {
		22, 27, 36, 23, 24, 37, 25, 26, 49, 43, 35, 52, 29,
		39, 40, 41, 45, 47, 21, 50, 46, 42, 34, 30, 14};
	static const int kFramesPerPage = 45;

	Common::Array<Common::StringArray> creditTexts = _res->getCredits();
	Common::Array<int> creditsSpeakerId;
	// Preprocess credit texts: extract speaker IDs and apply word wrapping
	for(uint i = 0; i < creditTexts.size(); i++) {
		byte speakerId;
		_dialog->processColorAndTrim(creditTexts[i], speakerId);
		creditsSpeakerId.push_back(speakerId);
		// Text is already encoded for new lines but should also be wrapped to respect the max chars per line!
		creditTexts[i] = _dialog->wordWrap(creditTexts[i])[0];
		// all lines start with a space but the first one contains the trailing space of the speakerId
		creditTexts[i][0] = creditTexts[i][0].substr(1, creditTexts[i][0].size() - 1);
	}

	CursorMan.showMouse(false);

	// Outer restart loop — keypress during display restarts from page 0

	_alfredState.setState(ALFRED_SKIP_DRAWING);
	_disableAmbientSounds = true;
	_disableAction = true;
	bool keyPressed = false;
	do {
		for (int page = 0; page < kNumCreditPages && !shouldQuit(); page++) {
			// loads screen
			setScreen(kCreditRooms[page]);
			if(kCreditRooms[page] == 24) {
				Sprite *pigeons = _room->findSpriteByIndex(1);
				pigeons->disableAfterSequence = true;
			}

			int height = creditTexts[page].size() * 25; // Add some padding

			Graphics::Surface s;
			s.create(640, height + 1, Graphics::PixelFormat::createFormatCLUT8());
			s.fillRect(s.getRect(), 255); // Clear surface

			int maxWidth = 0;
			/**
			 * Last line is less indented for some reason, so skip that for calculation
			 */
			for (uint i = 0; i < creditTexts[page].size(); i++) {
				maxWidth = MAX(maxWidth, g_engine->_largeFont->getStringWidth(creditTexts[page][i]));
			}

			int startX = 320 - (maxWidth / 2);
			int startY = (400 - s.getRect().height()) / 2 - 10;

			for (uint i = 0; i < creditTexts[page].size(); i++) {
				// subtract that extra negative identation
				int xPos = i == creditTexts[page].size() - 1 ? startX - 10 : startX;
				int yPos = i * 25; // Above sprite, adjust for line
				g_engine->_largeFont->drawString(&s, creditTexts[page][i], xPos, yPos, 640, creditsSpeakerId[page], Graphics::kTextAlignLeft);
			}

			int frames = 0;
			_events->_lastKeyEvent = Common::KEYCODE_INVALID;
			while (!shouldQuit() && frames < kFramesPerPage) {
				_events->pollEvent();
				bool didRender = renderScene(OVERLAY_NONE);

				if (didRender) {
					_screen->transBlitFrom(s, s.getRect(), Common::Point(0, startY), 255);
					frames++;
				}
				_screen->markAllDirty();
				_screen->update();
				g_system->delayMillis(10);
				if (_events->_lastKeyEvent != Common::KEYCODE_INVALID) {
					// Any key press exits credits
					keyPressed = true;
					break;
				}
			}
			if (keyPressed) {
				break;
			}
		}
	} while (!shouldQuit() && !keyPressed);

	g_engine->quitGame();
}

void PelrockEngine::initGodsSequences(int roomNumber) {
	int idx = roomNumber - 51;
	_fightFrameCounter = 0;
	_fightSorcererSpriteIdx = kFightRooms[idx].spriteIdx;
	_fightSorcererAppeared = false;
	_fightSpellCast = false;
	_fightSpellFrameCounter = 0;
	_fightInBlockingAnim = false;

	_room->disableSprite(_fightSorcererSpriteIdx);
}

void PelrockEngine::handleFightRoomFrame() {
	int room = _room->_currentRoomNumber;
	if (room < 51 || room > 54)
		return;
	int idx = room - 51;

	if (_state->getFlag(FLAG_COMO_ESTAN_LOS_DIOSES) & (1 << idx)) {
		// Gods are already defeated in this room, no need to run sequence
		return;
	}

	// Guard against reentrance from blocking animation loops calling renderScene
	if (_fightInBlockingAnim)
		return;

	if (_alfredState.animState != ALFRED_IDLE || _actionPopupState.isActive) {
		return;
	}
	_fightFrameCounter++;

	// Phase 1: NPC appearance at tick 64
	if (!_fightSorcererAppeared && _fightFrameCounter >= 64) {
		_sound->playSound(_room->_roomSfx[0]);
		_fightSorcererAppeared = true;
		_fightInBlockingAnim = true;
		_room->findSpriteByIndex(_fightSorcererSpriteIdx)->animData[0].nframes = 1;
		smokeAnimation(_fightSorcererSpriteIdx, false);
		_fightInBlockingAnim = false;
		return;
	}

	// Phase 2: spell trigger at tick 104 (64 + 40)
	if (_fightSorcererAppeared && !_fightSpellCast && _fightFrameCounter >= 104) {
		debug("spell cast triggered for room %d at frame %d", room, _fightFrameCounter);
		_fightSpellCast = true;
		_fightSpellFrameCounter = 0;
	}

	// Phase 3: wait 40 ticks after spell trigger, then cast
	if (_fightSpellCast) {
		debug("in spell cast phase for room %d at frame %d", room, _fightFrameCounter);
		_fightSpellFrameCounter++;
		if (_fightSpellFrameCounter >= 40 && !(_state->getFlag(FLAG_COMO_ESTAN_LOS_DIOSES) & (1 << idx))) {
			debug("spell cast animation starting for room %d at frame %d, flag is %d", room, _fightFrameCounter, _state->getFlag(FLAG_COMO_ESTAN_LOS_DIOSES));
			_fightInBlockingAnim = true;

			int spellFrames = kFightRooms[idx].spellFrames;
			int framesDone = 0;
			_room->findSpriteByIndex(_fightSorcererSpriteIdx)->animData[0].nframes = kFightRooms[idx].spellFrames;
			_room->findSpriteByIndex(_fightSorcererSpriteIdx)->animData[0].speed = 1;
			_sound->playSound(_room->_roomSfx[1]);
			while (!shouldQuit() && framesDone < spellFrames) {
				_events->pollEvent();
				if (renderScene(OVERLAY_NONE))
					framesDone++;
				_screen->update();
				g_system->delayMillis(10);
			}
			_room->findSpriteByIndex(_fightSorcererSpriteIdx)->animData[0].nframes = 1;
			smokeAnimation(-1, true);

			_fightInBlockingAnim = false;

			_alfredState.x = 294;
			_alfredState.y = 387;
			_alfredState.direction = ALFRED_UP;
			setScreenAndPrepare(49, ALFRED_UP);
		}
	}
}

} // End of namespace Pelrock
