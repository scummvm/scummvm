/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "voyeur/voyeur.h"
#include "voyeur/animation.h"
#include "voyeur/screen.h"
#include "voyeur/staticres.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "graphics/palette.h"
#include "graphics/scaler.h"
#include "graphics/thumbnail.h"

namespace Voyeur {

VoyeurEngine::VoyeurEngine(OSystem *syst, const VoyeurGameDescription *gameDesc) : Engine(syst),
		_gameDescription(gameDesc), _randomSource("Voyeur"),
		_defaultFontInfo(3, 0xff, 0xff, 0, 0, ALIGN_LEFT, 0, Common::Point(), 1, 1,
			Common::Point(1, 1), 1, 0, 0) {
	_debugger = nullptr;
	_eventsManager = nullptr;
	_filesManager = nullptr;
	_screen = nullptr;
	_soundManager = nullptr;
	_voy = nullptr;
	_bVoy = NULL;

	_iForceDeath = ConfMan.getInt("boot_param");
	if (_iForceDeath < 1 || _iForceDeath > 4)
		_iForceDeath = -1;

	_controlPtr = NULL;
	_stampFlags = 0;
	_playStampGroupId = _currentVocId = 0;
	_audioVideoId = -1;
	_checkTransitionId = -1;
	_gameHour = 0;
	_gameMinute = 0;
	_flashTimeVal = 0;
	_flashTimeFlag = false;
	_timeBarVal = -1;
	_checkPhoneVal = 0;
	_voyeurArea = AREA_NONE;
	_loadGameSlot = -1;

	DebugMan.addDebugChannel(kDebugScripts, "scripts", "Game scripts");

	_stampLibPtr = nullptr;
	_controlGroupPtr = nullptr;
	_stampData = nullptr;
	_stackGroupPtr = nullptr;
	_glGoState = -1;
	_glGoStack = -1;
	_resolvePtr = nullptr;
	_mainThread = nullptr;

	centerMansionView();
}

VoyeurEngine::~VoyeurEngine() {
	delete _bVoy;
	delete _voy;
	delete _soundManager;
	delete _screen;
	delete _filesManager;
	delete _eventsManager;
	//_debugger is deleted by Engine
}

Common::Error VoyeurEngine::run() {
	ESP_Init();
	globalInitBolt();

	if (doHeadTitle()) {
		// The original allows the victim to be explicitly specified via the command line.
		// This is possible in ScummVM by using a boot parameter.
		if (_iForceDeath >= 1 && _iForceDeath <= 4)
			_voy->_eventFlags |= EVTFLAG_VICTIM_PRESET;


		playStamp();
		if (!shouldQuit())
			doTailTitle();
	}

	return Common::kNoError;
}


int VoyeurEngine::getRandomNumber(int maxNumber) {
	return _randomSource.getRandomNumber(maxNumber);
}

void VoyeurEngine::ESP_Init() {
	ThreadResource::init();

	if (ConfMan.hasKey("save_slot"))
		_loadGameSlot = ConfMan.getInt("save_slot");
}

void VoyeurEngine::globalInitBolt() {
	_debugger = new Debugger(this);
	setDebugger(_debugger);
	_eventsManager = new EventsManager(this);
	_filesManager = new FilesManager(this);
	_screen = new Screen(this);
	_soundManager = new SoundManager(_mixer);
	_voy = new SVoy(this);

	initBolt();

	_filesManager->openBoltLib("bvoy.blt", _bVoy);
	_bVoy->getBoltGroup(0x000);
	_bVoy->getBoltGroup(0x100);

	_screen->_fontPtr = &_defaultFontInfo;
	_screen->_fontPtr->_curFont = _bVoy->boltEntry(0x101)._fontResource;
	assert(_screen->_fontPtr->_curFont);

	// Setup default flags
	_voy->_viewBounds = nullptr;

	_eventsManager->addFadeInt();
}

void VoyeurEngine::initBolt() {
	vInitInterrupts();
	_screen->sInitGraphics();
	_eventsManager->vInitColor();
	initInput();
}

void VoyeurEngine::vInitInterrupts() {
	_eventsManager->_intPtr._palette = &_screen->_VGAColors[0];
}

void VoyeurEngine::initInput() {
}

bool VoyeurEngine::doHeadTitle() {
//	char dest[144];

	_eventsManager->startMainClockInt();

	if (_loadGameSlot == -1) {
		// Show starting screen
		if (!getIsDemo() && _bVoy->getBoltGroup(0x500)) {
			showConversionScreen();
			_bVoy->freeBoltGroup(0x500);

			if (shouldQuit())
				return false;
		}

		if (ConfMan.getBool("copy_protection")) {
			// Display lock screen
			bool result = doLock();
			if (!result || shouldQuit())
				return false;
		}

		if (!getIsDemo()) {
			// Show the title screen
			_eventsManager->getMouseInfo();
			showTitleScreen();
			if (shouldQuit())
				return false;
		}

		// Opening
		_eventsManager->getMouseInfo();
		doOpening();
		if (shouldQuit())
			return false;

		_eventsManager->getMouseInfo();
		doTransitionCard("Saturday Afternoon", "Player's Apartment");
		_eventsManager->delayClick(90);

		if (_voy->_eventFlags & EVTFLAG_VICTIM_PRESET) {
			// Preset victim turned on, so add a default set of incriminating videos
			_voy->addEvent(18, 1, EVTYPE_VIDEO, 33, 0, 998, -1);
			_voy->addEvent(18, 2, EVTYPE_VIDEO, 41, 0, 998, -1);
			_voy->addEvent(18, 3, EVTYPE_VIDEO, 47, 0, 998, -1);
			_voy->addEvent(18, 4, EVTYPE_VIDEO, 53, 0, 998, -1);
			_voy->addEvent(18, 5, EVTYPE_VIDEO, 46, 0, 998, -1);
			_voy->addEvent(18, 6, EVTYPE_VIDEO, 50, 0, 998, -1);
			_voy->addEvent(18, 7, EVTYPE_VIDEO, 40, 0, 998, -1);
			_voy->addEvent(18, 8, EVTYPE_VIDEO, 43, 0, 998, -1);
			_voy->addEvent(19, 1, EVTYPE_AUDIO, 20, 0, 998, -1);
		}
	}

	_voy->_aptLoadMode = 140;
	return true;
}

void VoyeurEngine::showConversionScreen() {
	_screen->_backgroundPage = _bVoy->boltEntry(0x502)._picResource;
	_screen->_vPort->setupViewPort();
	flipPageAndWait();

	// Immediate palette load to show the initial screen
	CMapResource *cMap = _bVoy->getCMapResource(0x503);
	assert(cMap);
	cMap->_steps = 0;
	cMap->startFade();

	// Wait briefly
	_eventsManager->delayClick(150);
	if (shouldQuit())
		return;

	// Fade out the screen
	cMap = _bVoy->getCMapResource(0x504);
	cMap->_steps = 30;
	cMap->startFade();
	if (shouldQuit())
		return;

	flipPageAndWaitForFade();

	_screen->screenReset();
}

bool VoyeurEngine::doLock() {
	bool result = true, setPassword = false;
	int buttonVocSize, wrongVocSize;
	byte *buttonVoc = _filesManager->fload("button.voc", &buttonVocSize);
	byte *wrongVoc = _filesManager->fload("wrong.voc", &wrongVocSize);

	if (_bVoy->getBoltGroup(0x700)) {
		Common::String password = ConfMan.hasKey("lockCode") ? ConfMan.get("lockCode") : "3333";

		_screen->_backgroundPage = _bVoy->getPictureResource(0x700);
		_screen->_backColors = _bVoy->getCMapResource(0x701);
		PictureResource *cursorPic = _bVoy->getPictureResource(0x702);
		_voy->_viewBounds = _bVoy->boltEntry(0x704)._rectResource;
		Common::Array<RectEntry> &hotspots = _bVoy->boltEntry(0x705)._rectResource->_entries;

		assert(cursorPic);
		_screen->_vPort->setupViewPort();

		_screen->_backColors->startFade();
		_screen->_vPort->_parent->_flags |= DISPFLAG_8;
		_screen->flipPage();
		_eventsManager->sWaitFlip();

		while (!shouldQuit() && (_eventsManager->_fadeStatus & 1))
			_eventsManager->delay(1);

		_eventsManager->setCursorColor(127, 0);
		_screen->setColor(1, 64, 64, 64);
		_screen->setColor(2, 96, 96, 96);
		_screen->setColor(3, 160, 160, 160);
		_screen->setColor(4, 224, 224, 224);

		// Set up the cursor
		_eventsManager->setCursor(cursorPic);
		_eventsManager->showCursor();

		_eventsManager->_intPtr._hasPalette = true;

		_screen->_fontPtr->_curFont = _bVoy->boltEntry(0x708)._fontResource;
		_screen->_fontPtr->_fontSaveBack = 0;
		_screen->_fontPtr->_fontFlags = DISPFLAG_NONE;

		Common::String dateString = "ScummVM";
 		Common::String displayString = Common::String::format("Last Play %s", dateString.c_str());

		bool firstLoop = true;
		bool breakFlag = false;
		while (!breakFlag && !shouldQuit()) {
			_screen->_vPort->setupViewPort();
			flipPageAndWait();

			// Display the last play time
			_screen->_fontPtr->_pos = Common::Point(0, 97);
			_screen->_fontPtr->_justify = ALIGN_CENTER;
			_screen->_fontPtr->_justifyWidth = 384;
			_screen->_fontPtr->_justifyHeight = 97;

			_screen->_vPort->drawText(displayString);
			flipPageAndWait();

			if (firstLoop) {
				firstLoop = false;
				displayString = "";
			}

			// Loop for getting key presses
			int key;
			do {
				do {
					// Scan through the list of key rects to check if a keypad key is highlighted
					key = -1;
					Common::Point mousePos = _eventsManager->getMousePos() + Common::Point(20, 10);

					int keyCount = hotspots.size();
					for (int keyIndex = 0; keyIndex < keyCount; ++keyIndex) {
						if (hotspots[keyIndex].contains(mousePos)) {
							key = keyIndex;
							break;
						}
					}

					_eventsManager->setCursorColor(127, (key == -1) ? 0 : 1);
					_eventsManager->_intPtr._hasPalette = true;

					_eventsManager->delay(1);
					_eventsManager->getMouseInfo();
				} while (!shouldQuit() && !_eventsManager->_mouseClicked);
				_eventsManager->_mouseClicked = false;
			} while (!shouldQuit() && key == -1);

			_soundManager->abortVOCMap();
			_soundManager->playVOCMap(buttonVoc, buttonVocSize);

			while (_soundManager->getVOCStatus()) {
				if (shouldQuit())
					break;
				_eventsManager->delay(1);
			}

			// Process the key
			if (key < 10) {
				// Numeric key
				if (displayString.size() < 10) {
					displayString += '0' + key;
					continue;
				}
			} else if (key == 10) {
				// Accept key
				if (setPassword) {
					// Set a new password
					password = displayString;
					ConfMan.setAndFlush("lockCode", password);
				}

				if (password == displayString) {
					breakFlag = true;
					result = true;
					break;
				}
			} else if (key == 11) {
				// New code
				if (password == displayString) {
					_screen->_vPort->setupViewPort();
					password = displayString;
					displayString = "";
					setPassword = true;
					continue;
				}
			} else if (key == 12) {
				// Exit keyword
				breakFlag = true;
				result = false;
				break;
			} else {
				continue;
			}

			_screen->_vPort->setupViewPort();
			displayString = "";
			_soundManager->playVOCMap(wrongVoc, wrongVocSize);
		}

		_screen->fillPic(_screen->_vPort, 0);
		flipPageAndWait();
		_screen->resetPalette();

		_voy->_viewBounds = nullptr;
		_bVoy->freeBoltGroup(0x700);
	}

	_eventsManager->hideCursor();

	delete[] buttonVoc;
	delete[] wrongVoc;

	return result;
}

void VoyeurEngine::showTitleScreen() {
	if (!_bVoy->getBoltGroup(0x500))
		return;

	_screen->_backgroundPage = _bVoy->getPictureResource(0x500);

	_screen->_vPort->setupViewPort();
	flipPageAndWait();

	// Immediate palette load to show the initial screen
	CMapResource *cMap = _bVoy->getCMapResource(0x501);
	assert(cMap);
	cMap->_steps = 60;
	cMap->startFade();

	// Wait briefly
	_eventsManager->delayClick(200);
	if (shouldQuit()) {
		_bVoy->freeBoltGroup(0x500);
		return;
	}

	// Fade out the screen
	cMap = _bVoy->getCMapResource(0x504);
	cMap->_steps = 30;
	cMap->startFade();

	flipPageAndWaitForFade();
	if (shouldQuit()) {
		_bVoy->freeBoltGroup(0x500);
		return;
	}

	_screen->screenReset();
	_eventsManager->delayClick(200);

	// Voyeur title
	playRL2Video("a1100100.rl2");
	_screen->screenReset();

	_bVoy->freeBoltGroup(0x500);
}

void VoyeurEngine::doOpening() {
	_screen->screenReset();

	if (!_bVoy->getBoltGroup(0x200))
		return;

	byte *frameTable = _bVoy->memberAddr(0x215);
	byte *xyTable = _bVoy->memberAddr(0x216);
//	byte *whTable = _bVoy->memberAddr(0x217);
	int frameIndex = 0;
	bool creditShow = true;
	PictureResource *textPic = nullptr;
	Common::Point textPos;

	_voy->_vocSecondsOffset = 0;
	_voy->_RTVNum = 0;
	_voy->_audioVisualStartTime = _voy->_RTVNum;
	_voy->_eventFlags |= EVTFLAG_RECORDING;
	_gameHour = 4;
	_gameMinute  = 0;
	_audioVideoId = 1;
	_eventsManager->_videoDead = -1;
	_voy->addVideoEventStart();

	_voy->_eventFlags &= ~EVTFLAG_TIME_DISABLED;

	for (int i = 0; i < 256; ++i)
		_screen->setColor(i, 8, 8, 8);

	_eventsManager->_intPtr._hasPalette = true;
	_screen->_vPort->setupViewPort();
	flipPageAndWait();

	RL2Decoder decoder;
	decoder.loadRL2File("a2300100.rl2", false);
	decoder.start();

	while (!shouldQuit() && !decoder.endOfVideo() && !_eventsManager->_mouseClicked) {
		if (decoder.hasDirtyPalette()) {
			const byte *palette = decoder.getPalette();
			_screen->setPalette(palette, 0, 256);
		}

		if (decoder.needsUpdate()) {
			const Graphics::Surface *frame = decoder.decodeNextFrame();
			_screen->blitFrom(*frame);

			if (decoder.getCurFrame() >= (int32)READ_LE_UINT32(frameTable + frameIndex * 4)) {
				if (creditShow) {
					// Show a credit
					textPic = _bVoy->boltEntry(frameIndex / 2 + 0x202)._picResource;
					textPos = Common::Point(READ_LE_UINT16(xyTable + frameIndex * 2),
						READ_LE_UINT16(xyTable + (frameIndex + 1) * 2));

					creditShow = false;
				} else {
					textPic = nullptr;

					creditShow = true;
				}

				++frameIndex;
			}

			if (textPic) {
				_screen->sDrawPic(textPic, _screen->_vPort, textPos);
			}

			flipPageAndWait();
		}

		_eventsManager->getMouseInfo();
		g_system->delayMillis(10);
	}

	if ((_voy->_RTVNum - _voy->_audioVisualStartTime) < 2)
		_eventsManager->delay(60);

	_voy->_eventFlags |= EVTFLAG_TIME_DISABLED;
	_voy->addVideoEventEnd();
	_voy->_eventFlags &= ~EVTFLAG_RECORDING;

	_bVoy->freeBoltGroup(0x200);
}

void VoyeurEngine::playRL2Video(const Common::String &filename) {
	RL2Decoder decoder;
	decoder.loadRL2File(filename, false);
	decoder.start();

	while (!shouldQuit() && !decoder.endOfVideo() && !_eventsManager->_mouseClicked) {
		if (decoder.hasDirtyPalette()) {
			const byte *palette = decoder.getPalette();
			_screen->setPalette(palette, 0, 256);
		}

		if (decoder.needsUpdate()) {
			const Graphics::Surface *frame = decoder.decodeNextFrame();
			_screen->blitFrom(*frame);
		}

		_eventsManager->getMouseInfo();
		g_system->delayMillis(10);
	}
}

void VoyeurEngine::playAVideo(int videoId) {
	playAVideoDuration(videoId, 9999);
}

void VoyeurEngine::playAVideoDuration(int videoId, int duration) {
	int totalFrames = duration * 10;

	if (videoId == -1)
		return;

	PictureResource *pic = NULL;
	if (videoId == 42) {
		_bVoy->getBoltGroup(0xE00);
		_eventsManager->_videoDead = 0;
		pic = _bVoy->boltEntry(0xE00 + _eventsManager->_videoDead)._picResource;
	}

	RL2Decoder decoder;
	decoder.loadVideo(videoId);

	decoder.seek(Audio::Timestamp(_voy->_vocSecondsOffset * 1000));
	decoder.start();
	int endFrame = decoder.getCurFrame() + totalFrames;

	_eventsManager->getMouseInfo();
	_eventsManager->startCursorBlink();

	while (!shouldQuit() && !decoder.endOfVideo() && !_eventsManager->_mouseClicked &&
			(decoder.getCurFrame() < endFrame)) {
		if (decoder.needsUpdate()) {
			const Graphics::Surface *frame = decoder.decodeNextFrame();
			_screen->blitFrom(*frame);

			if (_voy->_eventFlags & EVTFLAG_RECORDING)
				_screen->drawDot();
		}

		if (decoder.hasDirtyPalette()) {
			const byte *palette = decoder.getPalette();
			_screen->setPalette(palette, 0, decoder.getPaletteCount());
			_screen->setOneColor(128, 220, 20, 20);
		}

		_eventsManager->getMouseInfo();
		g_system->delayMillis(10);
	}

	// RL2 finished
	_screen->screenReset();
	_voy->_eventFlags &= ~EVTFLAG_RECORDING;

	if (_voy->_eventFlags & EVTFLAG_8) {
		assert(pic);
		byte *imgData = _screen->_vPort->_currentPic->_imgData;
		_screen->_vPort->_currentPic->_imgData = pic->_imgData;
		pic->_imgData = imgData;
		_voy->_eventFlags &= ~EVTFLAG_8;
	}

	if (videoId == 42)
		_bVoy->freeBoltGroup(0xE00);
}

void VoyeurEngine::playAudio(int audioId) {
	_bVoy->getBoltGroup(0x7F00);
	_screen->_backgroundPage = _bVoy->boltEntry(0x7F00 +
		BLIND_TABLE[audioId] * 2)._picResource;
	_screen->_backColors = _bVoy->boltEntry(0x7F01 +
		BLIND_TABLE[audioId] * 2)._cMapResource;

	_screen->_vPort->setupViewPort();
	_screen->_backColors->startFade();
	flipPageAndWaitForFade();

	_voy->_eventFlags &= ~EVTFLAG_TIME_DISABLED;
	_soundManager->setVOCOffset(_voy->_vocSecondsOffset);
	Common::String filename = _soundManager->getVOCFileName(
		audioId + 159);
	_soundManager->startVOCPlay(filename);
	_voy->_eventFlags |= EVTFLAG_RECORDING;
	_eventsManager->startCursorBlink();

	while (!shouldQuit() && !_eventsManager->_mouseClicked &&
			_soundManager->getVOCStatus())
		_eventsManager->delayClick(1);

	_voy->_eventFlags |= EVTFLAG_TIME_DISABLED;
	_soundManager->stopVOCPlay();

	_bVoy->freeBoltGroup(0x7F00);
	_screen->_vPort->setupViewPort(NULL);

	_voy->_eventFlags &= ~EVTFLAG_RECORDING;
	_voy->_playStampMode = 129;
}

void VoyeurEngine::doTransitionCard(const Common::String &time, const Common::String &location) {
	_screen->setColor(128, 16, 16, 16);
	_screen->setColor(224, 220, 220, 220);
	_eventsManager->_intPtr._hasPalette = true;

	_screen->_vPort->setupViewPort(NULL);
	_screen->_vPort->fillPic(0x80);
	_screen->flipPage();
	_eventsManager->sWaitFlip();

	flipPageAndWait();
	_screen->_vPort->fillPic(0x80);

	FontInfoResource &fi = *_screen->_fontPtr;
	fi._curFont = _bVoy->boltEntry(257)._fontResource;
	fi._foreColor = 224;
	fi._fontSaveBack = 0;
	fi._pos = Common::Point(0, 116);
	fi._justify = ALIGN_CENTER;
	fi._justifyWidth = 384;
	fi._justifyHeight = 120;

	_screen->_vPort->drawText(time);

	if (!location.empty()) {
		fi._pos = Common::Point(0, 138);
		fi._justify = ALIGN_CENTER;
		fi._justifyWidth = 384;
		fi._justifyHeight = 140;

		_screen->_vPort->drawText(location);
	}

	flipPageAndWait();
}

void VoyeurEngine::flipPageAndWait() {
	_screen->_vPort->_flags |= DISPFLAG_8;
	_screen->flipPage();
	_eventsManager->sWaitFlip();
}

void VoyeurEngine::flipPageAndWaitForFade() {
	flipPageAndWait();

	while (!shouldQuit() && (_eventsManager->_fadeStatus & 1))
		_eventsManager->delay(1);
}

void VoyeurEngine::showEndingNews() {
	_playStampGroupId = (_voy->_incriminatedVictimNumber - 1) * 256 + 0x7700;
	_voy->_boltGroupId2 = (_controlPtr->_state->_victimIndex - 1) * 256 + 0x7B00;

	_bVoy->getBoltGroup(_playStampGroupId);
	_bVoy->getBoltGroup(_voy->_boltGroupId2);

	PictureResource *pic = _bVoy->boltEntry(_playStampGroupId)._picResource;
	CMapResource *pal = _bVoy->boltEntry(_playStampGroupId + 1)._cMapResource;

	_screen->_vPort->setupViewPort(pic);
	pal->startFade();
	flipPageAndWaitForFade();

	_eventsManager->getMouseInfo();

	for (int idx = 1; idx < 4; ++idx) {
		if (idx == 3) {
			pic = _bVoy->boltEntry(_voy->_boltGroupId2)._picResource;
			pal = _bVoy->boltEntry(_voy->_boltGroupId2 + 1)._cMapResource;
		} else {
			pic = _bVoy->boltEntry(_playStampGroupId + idx * 2)._picResource;
			pal = _bVoy->boltEntry(_playStampGroupId + idx * 2 + 1)._cMapResource;
		}

		_screen->_vPort->setupViewPort(pic);
		pal->startFade();
		flipPageAndWaitForFade();

		_bVoy->freeBoltMember(_playStampGroupId + (idx - 1) * 2);
		_bVoy->freeBoltMember(_playStampGroupId + (idx - 1) * 2 + 1);

		Common::String fname = Common::String::format("news%d.voc", idx);
		_soundManager->startVOCPlay(fname);

		_eventsManager->getMouseInfo();
		while (!shouldQuit() && !_eventsManager->_mouseClicked &&
				_soundManager->getVOCStatus()) {
			_eventsManager->delay(1);
			_eventsManager->getMouseInfo();
		}

		_soundManager->stopVOCPlay();
		if (idx == 3)
			_eventsManager->delay(3);

		if (shouldQuit() || _eventsManager->_mouseClicked)
			break;
	}

	_bVoy->freeBoltGroup(_playStampGroupId);
	_bVoy->freeBoltGroup(_voy->_boltGroupId2);
	_playStampGroupId = -1;
	_voy->_boltGroupId2 = -1;
}

/*------------------------------------------------------------------------*/

/**
 * Returns true if it is currently okay to restore a game
 */
bool VoyeurEngine::canLoadGameStateCurrently() {
	return _voyeurArea == AREA_APARTMENT;
}

/**
 * Returns true if it is currently okay to save the game
 */
bool VoyeurEngine::canSaveGameStateCurrently() {
	return _voyeurArea == AREA_APARTMENT;
}

/**
 * Load the savegame at the specified slot index
 */
Common::Error VoyeurEngine::loadGameState(int slot) {
	_loadGameSlot = slot;
	return Common::kNoError;
}

void VoyeurEngine::loadGame(int slot) {
	// Open up the save file
	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(getSaveStateName(slot));
	if (!saveFile)
		return;

	Common::Serializer serializer(saveFile, NULL);

	// Store the current time index before the game is loaded
	_checkTransitionId = _voy->_transitionId;

	// Stop any playing sound
	_soundManager->stopVOCPlay();

	// Read in the savegame header
	VoyeurSavegameHeader header;
	if (!header.read(saveFile))
		return;

	serializer.setVersion(header._version);
	synchronize(serializer);

	delete saveFile;

	// Show a transition card if the time index has changed
	checkTransition();

	// Load the apartment
	_mainThread->loadTheApt();
}

/**
 * Save the game to the given slot index, and with the given name
 */
Common::Error VoyeurEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	// Open the save file for writing
	Common::OutSaveFile *saveFile = g_system->getSavefileManager()->openForSaving(getSaveStateName(slot));
	if (!saveFile)
		return Common::kCreatingFileFailed;

	// Write out the header
	VoyeurSavegameHeader header;
	header.write(saveFile, this, desc);

	// Set up a serializer
	Common::Serializer serializer(NULL, saveFile);

	// Synchronise the data
	serializer.setVersion(VOYEUR_SAVEGAME_VERSION);
	synchronize(serializer);

	saveFile->finalize();
	delete saveFile;

	return Common::kNoError;
}

void VoyeurEngine::synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(_glGoState);
	s.syncAsSint16LE(_glGoStack);
	s.syncAsSint16LE(_stampFlags);
	s.syncAsSint16LE(_playStampGroupId);
	s.syncAsSint16LE(_currentVocId);
	s.syncAsSint16LE(_audioVideoId);

	s.syncAsSint16LE(_iForceDeath);
	s.syncAsSint16LE(_gameHour);
	s.syncAsSint16LE(_gameMinute);
	s.syncAsSint16LE(_flashTimeVal);
	s.syncAsSint16LE(_flashTimeFlag);
	s.syncAsSint16LE(_timeBarVal);
	s.syncAsSint16LE(_checkPhoneVal);

	// Sub-systems
	_voy->synchronize(s);
	_screen->synchronize(s);
	_mainThread->synchronize(s);
	_controlPtr->_state->synchronize(s);
}

/*------------------------------------------------------------------------*/

bool VoyeurSavegameHeader::read(Common::InSaveFile *f, bool skipThumbnail) {
	uint32 signature = f->readUint32BE();
	if (signature != MKTAG('V', 'O', 'Y', 'R')) {
		warning("Invalid savegame");
		return false;
	}

	_version = f->readByte();
	if (_version > VOYEUR_SAVEGAME_VERSION)
		return false;

	char c;
	_saveName = "";
	while ((c = f->readByte()) != 0)
		_saveName += c;

	// Get the thumbnail
	if (!Graphics::loadThumbnail(*f, _thumbnail, skipThumbnail)) {
		return false;
	}

	// Read in the save datet/ime
	_saveYear = f->readSint16LE();
	_saveMonth = f->readSint16LE();
	_saveDay = f->readSint16LE();
	_saveHour = f->readSint16LE();
	_saveMinutes = f->readSint16LE();
	_totalFrames = f->readUint32LE();

	return true;
}

void VoyeurSavegameHeader::write(Common::OutSaveFile *f, VoyeurEngine *vm, const Common::String &saveName) {
	// Write ident string
	f->writeUint32BE(MKTAG('V', 'O', 'Y', 'R'));

	// Write out savegame version
	f->writeByte(VOYEUR_SAVEGAME_VERSION);

	// Write out savegame name
	f->write(saveName.c_str(), saveName.size());
	f->writeByte(0);

	// Create a thumbnail and save it
	Graphics::Surface *thumb = new Graphics::Surface();
	::createThumbnail(thumb, (const byte *)vm->_screen->getPixels(),
		SCREEN_WIDTH, SCREEN_HEIGHT, vm->_screen->_VGAColors);
	Graphics::saveThumbnail(*f, *thumb);
	thumb->free();
	delete thumb;

	// Write the save datet/ime
	TimeDate td;
	g_system->getTimeAndDate(td);
	f->writeSint16LE(td.tm_year + 1900);
	f->writeSint16LE(td.tm_mon + 1);
	f->writeSint16LE(td.tm_mday);
	f->writeSint16LE(td.tm_hour);
	f->writeSint16LE(td.tm_min);
	f->writeUint32LE(vm->_eventsManager->getGameCounter());
}

} // End of namespace Voyeur
