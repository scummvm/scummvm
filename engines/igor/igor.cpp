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

#include "igor/igor.h"
#include "graphics/framelimiter.h"
#include "igor/detection.h"
#include "igor/console.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/paletteman.h"
#include "graphics/cursorman.h"

#include "audio/mixer.h"

namespace Igor {

IgorEngine *g_engine;

IgorEngine::IgorEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Igor") {
	g_engine = this;


	_screenVGA = (uint8 *)malloc(320 * 200);
	for(int i = 0; i < 4; i++) {
		_facingIgorFrames[i] = (uint8 *)malloc(13500);
	}
	_screenLayer1 = (uint8 *)malloc(320 * 144);
	_screenLayer2 = (uint8 *)malloc(320 * 144);
	_screenTextLayer = (uint8 *)malloc(320 * 144);
	_screenTempLayer = (uint8 *)malloc(9996);
	_igorHeadFrames = (uint8 *)malloc(3696);
	_animFramesBuffer = (uint8 *)malloc(65535);
	_inventoryPanelBuffer = (uint8 *)malloc(9600 * 2);
	_inventoryImagesBuffer = (uint8 *)malloc(48000);
	_verbsPanelBuffer = (uint8 *)malloc(3840);

	{ // hardcoded now
		_game.ovlFileName = "igor.exe";
		_game.sfxFileName = "igor.dat";
		_currentPart = 50;
	}

	// if (_game.flags & kFlagFloppy) {
	// 	_midiPlayer = new Midi(this);
	// } else {
	// 	_midiPlayer = 0;
	// }
}

IgorEngine::~IgorEngine() {
	// free(_resourceEntries);
	// free(_soundOffsets);
	free(_screenVGA);
	for (int i = 0; i < 4; ++i) {
		free(_facingIgorFrames[i]);
	}
	free(_screenLayer1);
	free(_screenLayer2);
	free(_screenTextLayer);
	free(_screenTempLayer);
	free(_igorHeadFrames);
	free(_animFramesBuffer);
	free(_inventoryPanelBuffer);
	free(_inventoryImagesBuffer);
	free(_verbsPanelBuffer);

	// Common::clearAllDebugChannels();

	// delete _midiPlayer;
	delete _screen;
}

uint32 IgorEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String IgorEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error IgorEngine::run() {
	// Initialize 320x200 paletted graphics mode
	initGraphics(320, 200);
	_screen = new Graphics::Screen();

	// Set the engine's debugger console
	setDebugger(new Console());

	restart();
	setupDefaultPalette();

	if (!_ovlFile.open(_game.ovlFileName)) {
		error("Unable to open '%s'", _game.ovlFileName);
	}

	readTableFile();
	loadMainTexts();
	loadIgorFrames();
	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);


	_eventQuitGame = false;
	PART_MAIN();
	_ovlFile.close();
	// _sndFile.close();

	return Common::kNoError;
}

void IgorEngine::restart() {
	_nextTimer = 0;
}

void IgorEngine::setupDefaultPalette() {
	memset(_currentPalette, 0, 255 * 3);
	memset(&_currentPalette[255 * 3], 63, 3);
	g_system->getPaletteManager()->setPalette(_currentPalette, 0, 256);
}

void IgorEngine::readTableFile() {
	Common::File tblFile;
	uint32 stringsEntriesOffset = 0, resourcesEntriesOffset = 0, soundEntriesOffset = 0;
	if (tblFile.open("IGOR.TBL") && tblFile.readUint32BE() == 'ITBL' && tblFile.readUint32BE() == 4) {
		stringsEntriesOffset = tblFile.readUint32BE();
		uint32 borlandOverlaySize = _ovlFile.size();
		int gameVersionsCount = tblFile.readByte();
		for (int i = 0; i < gameVersionsCount; ++i) {
			uint32 size = tblFile.readUint32BE();
			if (size == borlandOverlaySize) {
				resourcesEntriesOffset = tblFile.readUint32BE();
				soundEntriesOffset = tblFile.readUint32BE();
				break;
			}
			tblFile.skip(8);
		}
	}
	if (resourcesEntriesOffset != 0 && soundEntriesOffset != 0) {
		tblFile.seek(resourcesEntriesOffset);
		_resourceEntriesCount = tblFile.readUint16BE();
		_resourceEntries = (ResourceEntry *)malloc(sizeof(ResourceEntry) * _resourceEntriesCount);
		for (int i = 0; i < _resourceEntriesCount; ++i) {
			_resourceEntries[i].id = tblFile.readUint16BE();
			_resourceEntries[i].offs = tblFile.readUint32BE();
			_resourceEntries[i].size = tblFile.readUint32BE();
		}
		tblFile.seek(soundEntriesOffset);
		_soundOffsetsCount = tblFile.readUint16BE();
		_soundOffsets = (uint32 *)malloc(sizeof(uint32) * _soundOffsetsCount);
		for (int i = 0; i < _soundOffsetsCount; ++i) {
			_soundOffsets[i] = tblFile.readUint32BE();
		}
		tblFile.seek(stringsEntriesOffset);
		int stringsCount = tblFile.readUint16BE();
		for (int i = 0; i < stringsCount; ++i) {
			int id = tblFile.readUint16BE();
			int lang = tblFile.readByte();
			int len = tblFile.readByte();
			bool skipString = (lang == 1 && _game.language != Common::EN_ANY) || (lang == 2 && _game.language != Common::ES_ESP);
			if (skipString) {
				tblFile.skip(len);
			} else {
				char buf[256];
				tblFile.read(buf, len);
				buf[len] = 0;
				_stringEntries.push_back(StringEntry(id, buf));
			}
		}
		return;
	}
	error("Unable to read 'IGOR.TBL'");
}

void IgorEngine::loadMainTexts() {
}

void IgorEngine::loadIgorFrames() {
}

static int compareResourceEntry(const void *a, const void *b) {
	int id = *(const int *)a;
	const ResourceEntry *entry = (const ResourceEntry *)b;
	return id - entry->id;
}

ResourceEntry *IgorEngine::findData(int id) {
	ResourceEntry *re = (ResourceEntry *)bsearch(&id, _resourceEntries, _resourceEntriesCount, sizeof(ResourceEntry), compareResourceEntry);
	assert(re);
	return re;
}

uint8 *IgorEngine::loadData(int id, uint8 *dst, int *size) {
	// debugC(9, kDebugResource, "loadData() id %d", id);
	ResourceEntry *re = findData(id);
	if (!dst) {
		dst = (uint8 *)malloc(re->size);
		if (!dst) {
			error("Unable to allocate %d bytes", re->size);
		}
	}
	_ovlFile.seek(re->offs);
	_ovlFile.read(dst, re->size);
	if (size) {
		*size = re->size;
	}
	return dst;
}

void IgorEngine::loadRoomData(int pal, int img, int box, int msk, int txt) {
	if (pal != 0) {
		loadData(pal, _paletteBuffer);
	}
	if (img != 0) {
		loadData(img, _screenLayer1);
	}
	// if (box != 0) {
	// 	int sz;
	// 	uint8 *p = loadData(box, 0, &sz);
	// 	memset(_roomObjectAreasTable, 0, sizeof(_roomObjectAreasTable));
	// 	assert((sz % 5) == 0);
	// 	decodeRoomAreas(p, sz / 5);
	// 	free(p);
	// }
	// if (msk != 0) {
	// 	uint8 *p = loadData(msk);
	// 	decodeRoomMask(p);
	// 	free(p);
	// }
	// if (txt != 0) {
	// 	uint8 *p = loadData(txt);
	// 	decodeRoomText(p);
	// 	free(p);
	// }

	{
		g_system->getPaletteManager()->setPalette(_paletteBuffer, 0, 256);
		memcpy(_screenVGA, _screenLayer1, 46080);
	}
}

void IgorEngine::PART_05() {
	loadRoomData(PAL_SpringRock, IMG_SpringRock, BOX_SpringRock, MSK_SpringRock, TXT_SpringRock);



	enterPartLoop();
	while (_currentPart >= 50 && _currentPart <= 52) {
		runPartLoop();
	}
	leavePartLoop();

}


void IgorEngine::enterPartLoop() {
	CursorMan.showMouse(true);
	// if (!_gameState.dialogueTextRunning) {
	// 	showCursor();
	// }
	// _gameState.igorMoving = false;
	// if (_game.version == kIdEngDemo110) {
	// 	CHECK_FOR_END_OF_DEMO();
	// }
}

void IgorEngine::leavePartLoop() {
	// hideCursor();
	// SET_EXEC_ACTION_FUNC(1, 0);
	// _updateRoomBackground = 0;
	CursorMan.showMouse(false);
}

void IgorEngine::runPartLoop() {
	handleRoomInput();
	// if (compareGameTick(1, 16)) {
	// 	handleRoomIgorWalk();
	// }
	// if (compareGameTick(19, 32)) {
	// 	handleRoomDialogue();
	// }
	// if (compareGameTick(4, 8)) {
	// 	handleRoomInventoryScroll();
	// }
	// if (compareGameTick(1)) {
	// 	handleRoomLight();
	// }
	// if (_updateRoomBackground) {
	// 	(this->*_updateRoomBackground)();
	// }
	waitForTimer();
}

void IgorEngine::waitForTimer(int ticks) {
	_system->copyRectToScreen(_screenVGA, 320, 0, 0, 320, 200);
	_system->updateScreen();
	debug("Screen updated!");
	uint32 endTicks = (ticks == -1) ? _nextTimer : _system->getMillis() + ticks * 1000 / kTickDelay;
	do {
		Common::Event ev;
		while (_eventMan->pollEvent(ev)) {
			switch (ev.type) {
			case Common::EVENT_QUIT:
				// _inputVars[kInputEscape] = 1;
				_currentPart = kInvalidPart;
				_eventQuitGame = true;
				break;
	// 		case Common::EVENT_KEYDOWN:
	// 			if (ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
	// 				_inputVars[kInputEscape] = 1;
	// 			} else if (ev.kbd.keycode == Common::KEYCODE_SPACE) {
	// 				_inputVars[kInputOptions] = 1;
	// 			} else if (ev.kbd.keycode == Common::KEYCODE_p) {
	// 				_inputVars[kInputPause] = 1;
	// 			} else if (ev.kbd.keycode == Common::KEYCODE_F11) {
	// 				sprintf(_saveStateDescriptions[kQuickSaveSlot], "Quicksave part %d", _currentPart);
	// 				saveGameState(kQuickSaveSlot);
	// 			} else if (ev.kbd.keycode == Common::KEYCODE_F12) {
	// 				loadGameState(kQuickSaveSlot);
	// 			}
	// 			break;
	// 		case Common::EVENT_MOUSEMOVE:
	// 			_inputVars[kInputCursorXPos] = ev.mouse.x;
	// 			_inputVars[kInputCursorYPos] = ev.mouse.y;
	// 			break;
	// 		case Common::EVENT_RBUTTONDOWN:
	// 			_inputVars[kInputSkipDialogue] = 1;
	// 			break;
	// 		case Common::EVENT_LBUTTONDOWN:
	// 			_inputVars[kInputClick] = 1;
	// 			_inputVars[kInputCursorXPos] = ev.mouse.x;
	// 			_inputVars[kInputCursorYPos] = ev.mouse.y;
	// 			break;
			default:
				break;
			}
		}
		_system->delayMillis(10);
		if (_system->getMillis() >= endTicks) {
			break;
		}
		debug("Waiting for timer: %d ms left", endTicks - _system->getMillis());
	} while (true);
	_nextTimer = _system->getMillis() + kTimerTicksCount * 1000 / kTickDelay;
	if (ticks != -1) {
		return;
	}
	_gameTicks += kTimerTicksCount;
	// if ((_gameTicks & 31) == 0) {
	// 	setCursor(_currentCursor);
	// 	_currentCursor = (_currentCursor + 1) & 3;
	// }
	// if (_game.flags & kFlagFloppy) {
	// 	updateMusic();
	// }
	if (_gameTicks == 64) {
		_gameTicks = 0;
	}
}


void IgorEngine::PART_MAIN() {
	 while (!g_engine->shouldQuit() && !_eventQuitGame) {
        switch (_currentPart) {

        case 50:
            PART_05(); // SpringRock
            break;

        default:
            error("Unknown or unhandled room part %d", _currentPart);
            break;
        }
    }
}


void IgorEngine::handleRoomInput() {

}

Common::Error IgorEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

} // End of namespace Igor
