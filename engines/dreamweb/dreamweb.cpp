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
#include "common/events.h"
#include "common/file.h"
#include "common/func.h"
#include "common/system.h"
#include "common/timer.h"
#include "common/util.h"
#include "common/concatstream.h"

#include "engines/advancedDetector.h"

#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "dreamweb/detection.h"
#include "dreamweb/sound.h"
#include "dreamweb/dreamweb.h"
#include "dreamweb/rnca_archive.h"

#include "common/text-to-speech.h"

namespace DreamWeb {

DreamWebEngine::DreamWebEngine(OSystem *syst, const DreamWebGameDescription *gameDesc) :
	Engine(syst), _gameDescription(gameDesc), _rnd("dreamweb"),
	_exText(kNumExTexts),
	_setDesc(kNumSetTexts), _blockDesc(kNumBlockTexts),
	_roomDesc(kNumRoomTexts), _freeDesc(kNumFreeTexts),
	_personText(kNumPersonTexts) {

	_vSyncPrevTick = 0;
	_sound = nullptr;
	_speed = 1;
	_turbo = false;
	_oldMouseState = 0;

	_ttsMan = g_system->getTextToSpeechManager();

	_datafilePrefix = "DREAMWEB.";
	_speechDirName = "SPEECH";
	// ES and FR CD release use a different data file prefix
	// and speech directory naming.
	if (isCD()) {
		switch(getLanguage()) {
		case Common::ES_ESP:
			_datafilePrefix = "DREAMWSP.";
			_speechDirName = "SPANISH";
			break;
		case Common::FR_FRA:
			_datafilePrefix = "DREAMWFR.";
			_speechDirName = "FRENCH";
			break;
		default:
			// Nothing to do
			break;
		}
	}

	_openChangeSize = kInventx+(4*kItempicsize);
	_quitRequested = false;

	_speechLoaded = false;

	_backdropBlocks = nullptr;
	_reelList = nullptr;

	_oldSubject._type = 0;
	_oldSubject._index = 0;

	// misc variables
	_speechCount = 0;
	_charShift = 0;
	_brightPalette = false;
	_roomLoaded = 0;
	_didZoom = 0;
	_lineSpacing = 10;
	_textAddressX = 13;
	_textAddressY = 182;
	_textLen = 0;
	_lastXPos = 0;
	_itemFrame = 0;
	_withObject = 0;
	_withType = 0;
	_lookCounter = 0;
	_command = 0;
	_commandType = 0;
	_objectType = 0;
	_getBack = 0;
	_invOpen = 0;
	_mainMode = 0;
	_pickUp = 0;
	_lastInvPos = 0;
	_examAgain = 0;
	_newTextLine = 0;
	_openedOb = 0;
	_openedType = 0;
	_mapAdX = 0;
	_mapAdY = 0;
	_mapOffsetX = 104;
	_mapOffsetY = 38;
	_mapXStart = 0;
	_mapYStart = 0;
	_mapXSize = 0;
	_mapYSize = 0;
	_haveDoneObs = 0;
	_manIsOffScreen = 0;
	_facing = 0;
	_leaveDirection = 0;
	_turnToFace = 0;
	_turnDirection = 0;
	_mainTimer = 0;
	_introCount = 0;
	_currentKey = 0;
	_timerCount = 0;
	_mapX = 0;
	_mapY = 0;
	_ryanX = 0;
	_ryanY = 0;
	_lastFlag = 0;
	_destPos = 0;
	_realLocation = 0;
	_roomNum = 0;
	_nowInNewRoom = 0;
	_resetManXY = 0;
	_newLocation = 0xFF;
	_autoLocation = 0xFF;
	_mouseX = 0;
	_mouseY = 0;
	_mouseButton = 0;
	_oldButton = 0;
	_oldX = 0;
	_oldY = 0;
	_oldPointerX = 0;
	_oldPointerY = 0;
	_delHereX = 0;
	_delHereY = 0;
	_pointerXS = 32;
	_pointerYS = 32;
	_delXS = 0;
	_delYS = 0;
	_pointerFrame = 0;
	_pointerPower = 0;
	_pointerMode = 0;
	_pointerSpeed = 0;
	_pointerCount = 0;
	_inMapArea = 0;
	_talkMode = 0;
	_talkPos = 0;
	_character = 0;
	_watchDump = 0;
	_logoNum = 0;
	_oldLogoNum = 0;
	_pressed = 0;
	_pressPointer = 0;
	_graphicPress = 0;
	_pressCount = 0;
	_lightCount = 0;
	_folderPage = 0;
	_diaryPage = 0;
	_menuCount = 0;
	_symbolTopX = 0;
	_symbolTopNum = 0;
	_symbolTopDir = 0;
	_symbolBotX = 0;
	_symbolBotNum = 0;
	_symbolBotDir = 0;
	_walkAndExam = 0;
	_walkExamType = 0;
	_walkExamNum = 0;
	_cursLocX = 0;
	_cursLocY = 0;
	_curPos = 0;
	_monAdX = 0;
	_monAdY = 0;
	_timeCount = 0;
	_needToDumpTimed = 0;
	_loadingOrSave = 0;
	_saveLoadPage = 0;
	_currentSlot = 0;
	_cursorPos = 0;
	_colorPos = 0;
	_fadeDirection = 0;
	_numToFade = 0;
	_fadeCount = 0;
	_addToGreen = 0;
	_addToRed = 0;
	_addToBlue = 0;
	_lastSoundReel = 0;
	_lastHardKey = Common::KEYCODE_INVALID;
	_bufferIn = 0;
	_bufferOut = 0;
	_blinkFrame = 23;
	_blinkCount = 0;
	_reAssesChanges = 0;
	_pointersPath = 0;
	_mansPath = 0;
	_pointerFirstPath = 0;
	_finalDest = 0;
	_destination = 0;
	_lineStartX = 0;
	_lineStartY = 0;
	_lineEndX = 0;
	_lineEndY = 0;
	_linePointer = 0;
	_lineDirection = 0;
	_lineLength = 0;

	_subtitles = 0;
	_foreignRelease = 0;
	_wonGame = 0;
	_hasSpeech = 0;
	_roomsSample = 0;
	_copyProtection = 0;

	for (uint i = 0; i < 128; i++)
		memset(&_setDat[i], 0, sizeof(SetObject));

	for (uint i = 0; i < 80; i++)
		memset(&_freeDat[i], 0, sizeof(DynObject));

	for (uint i = 0; i < kNumExObjects; i++)
		memset(&_exData[i], 0, sizeof(DynObject));

	memset(&_vars, 0, sizeof(GameVars));

	for (uint i = 0; i < 96; i++)
		memset(&_backdropFlags[i], 0, sizeof(BackdropMapFlag));

	for (uint i = 0; i < kNumReelRoutines+1; i++)
		memset(&_reelRoutines[i], 0, sizeof(ReelRoutine));

	_personData = nullptr;

	for (uint i = 0; i < 16; i++)
		memset(&_openInvList[i], 0, sizeof(ObjectRef));

	for (uint i = 0; i < 30; i++)
		memset(&_ryanInvList[i], 0, sizeof(ObjectRef));

	for (uint i = 0; i < 11*10; i++)
		memset(&_mapFlags[i], 0, sizeof(MapFlag));

	for (uint i = 0; i < kNumChanges; i++)
		memset(&_listOfChanges[i], 0, sizeof(Change));

	_currentCharset = nullptr;

	for (uint i = 0; i < 36; i++)
		memset(&_pathData[i], 0, sizeof(RoomPaths));
}

DreamWebEngine::~DreamWebEngine() {
	delete _sound;
	if (_thumbnail.getPixels())
		_thumbnail.free();
}

void DreamWebEngine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);
	if (!pause)
		_vSyncPrevTick = _system->getMillis();
}

void DreamWebEngine::waitForVSync() {
	if (isPaused())
		return;

	processEvents();

	if (!_turbo) {
		const uint32 delay =  1000 / 70 / _speed;
		uint32 elapsed = _system->getMillis() - _vSyncPrevTick;
		if (elapsed < delay)
			_system->delayMillis(delay - elapsed);
	}
	_vSyncPrevTick = _system->getMillis();

	doShake();
	doFade();
	_system->updateScreen();
}

void DreamWebEngine::quit() {
	_quitRequested = true;
	_lastHardKey = Common::KEYCODE_ESCAPE;
}

void DreamWebEngine::processEvents(bool processSoundEvents) {
	if (_eventMan->shouldQuit()) {
		quit();
		return;
	}

	if (processSoundEvents)
		_sound->soundHandler();

	Common::Event event;
	int softKey;
	while (_eventMan->pollEvent(event)) {
		switch(event.type) {
		case Common::EVENT_RETURN_TO_LAUNCHER:
			quit();
			break;
		case Common::EVENT_KEYDOWN:
			if (event.kbd.flags & Common::KBD_CTRL) {
				switch (event.kbd.keycode) {

				case Common::KEYCODE_f:
					setSpeed(_speed != 4? 4: 1);
					break;

				case Common::KEYCODE_g:
					_turbo = !_turbo;
					break;

				case Common::KEYCODE_c: //skip statue puzzle
					_symbolBotNum = 3;
					_symbolTopNum = 5;
					break;

				default:
					break;
				}

				return; //do not pass ctrl + key to the engine
			}

			// Some parts of the code uses the hardware key
			// code directly.

			switch (event.kbd.keycode) {
			case Common::KEYCODE_ESCAPE:
				_lastHardKey = Common::KEYCODE_ESCAPE;
				break;
			case Common::KEYCODE_SPACE:
				_lastHardKey = Common::KEYCODE_SPACE;
				break;
			default:
				_lastHardKey = Common::KEYCODE_INVALID;
				break;
			}

			// The rest of the keys are converted to ASCII. This
			// is fairly restrictive, and eventually we may want
			// to let through more keys. I think this is mostly to
			// keep weird glyphs out of savegame names.

			softKey = 0;

			debug(1, "DreamWebEngine::processEvents() KeyDown keycode:%d ascii:0x%02x", event.kbd.keycode, event.kbd.ascii);
			if ((event.kbd.ascii >= 'a' && event.kbd.ascii <= 'z') ||
				(event.kbd.ascii >= 'A' && event.kbd.ascii <= 'Z')) {
				softKey = event.kbd.ascii & ~0x20; // (& ~0x20) forces ascii codes for a-z to map to A-Z
			} else if (event.kbd.ascii == '-' ||
				event.kbd.ascii == ' ' ||
				(event.kbd.ascii >= '0' && event.kbd.ascii <= '9')) {
				softKey = event.kbd.ascii;
			} else if (event.kbd.keycode >= Common::KEYCODE_KP0 && event.kbd.keycode <= Common::KEYCODE_KP9) {
				softKey = event.kbd.keycode - Common::KEYCODE_KP0 + '0';
			} else if (event.kbd.keycode == Common::KEYCODE_KP_MINUS) {
				softKey = '-';
			} else if (event.kbd.keycode == Common::KEYCODE_BACKSPACE ||
				event.kbd.keycode == Common::KEYCODE_DELETE) {
				softKey = 8;
			} else if (event.kbd.keycode == Common::KEYCODE_RETURN
				|| event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
				softKey = 13;
			}

			if (softKey)
				keyPressed(softKey);
			break;
		default:
			break;
		}
	}
}

Common::Error DreamWebEngine::run() {
	if (_gameDescription->desc.flags & GF_INSTALLER) {
		Common::Array<Common::SharedPtr<Common::SeekableReadStream>> volumes;
		for (uint i = 0; _gameDescription->desc.filesDescriptions[i].fileName; i++) {
			Common::File *dw = new Common::File();
			const char *name = _gameDescription->desc.filesDescriptions[i].fileName;
			if (!dw->open(name)) {
				error("Can't open %s", name);
			}
			volumes.push_back(Common::SharedPtr<Common::SeekableReadStream>(dw));
		}
		Common::ConcatReadStream *concat = new Common::ConcatReadStream(volumes);
		SearchMan.add("rnca", RNCAArchive::open(concat, DisposeAfterUse::YES));
	}

	if (_ttsMan != nullptr) {
		Common::String languageString = Common::getLanguageCode(getLanguage());
		_ttsMan->setLanguage(languageString);
		_ttsMan->enable(ConfMan.getBool("tts_enabled_objects") || ConfMan.getBool("tts_enabled_speech"));
		switch (getLanguage()) {
		case Common::RU_RUS:
			_textEncoding = Common::kDos866;
			break;
		case Common::CS_CZE:
			_textEncoding = Common::kWindows1250;
			break;
		default:
			_textEncoding = Common::kDos850;
			break;
		}
	}

	syncSoundSettings();
	setDebugger(new DreamWebConsole(this));
	_sound = new DreamWebSound(this);

	_hasSpeech = Common::File::exists(_speechDirName.appendComponent("r01c0000.raw")) && !ConfMan.getBool("speech_mute");
	_brightPalette = ConfMan.getBool("bright_palette");
	_copyProtection = ConfMan.getBool("copy_protection");

	_vSyncPrevTick = _system->getMillis();
	dreamweb();
	dreamwebFinalize();
	_quitRequested = false;

	return Common::kNoError;
}

void DreamWebEngine::setSpeed(uint speed) {
	debug(0, "setting speed %u", speed);
	_speed = speed;
}

Common::String DreamWebEngine::getSavegameFilename(int slot) const {
	// TODO: Are saves from all versions of Dreamweb compatible with each other?
	// Then we can can consider keeping the filenames as DREAMWEB.Dnn.
	// Otherwise, this must be changed to be target dependent.
	//Common::String filename = _targetName + Common::String::format(".d%02d", savegameId);
	Common::String filename = Common::String::format("DREAMWEB.D%02d", slot);
	return filename;
}

void DreamWebEngine::keyPressed(uint16 ascii) {
	debug(2, "key pressed = %04x", ascii);
	uint16 in = (_bufferIn + 1) % ARRAYSIZE(_keyBuffer);
	uint16 out = _bufferOut;
	if (in == out) {
		warning("keyboard buffer is full");
		return;
	}
	_bufferIn = in;
	_keyBuffer[in] = ascii;
}

void DreamWebEngine::getPalette(uint8 *data, uint start, uint count) {
	_system->getPaletteManager()->grabPalette(data, start, count);
	while (count--)
		*data++ >>= 2;
}

void DreamWebEngine::setPalette(const uint8 *data, uint start, uint count) {
	assert(start + count <= 256);
	uint8 fixed[3*256];
	for (uint i = 0; i < count * 3; ++i) {
		fixed[i] = data[i] << 2;
	}
	_system->getPaletteManager()->setPalette(fixed, start, count);
}

void DreamWebEngine::blit(const uint8 *src, int pitch, int x, int y, int w, int h) {
	if (y + h > (int)kScreenheight)
		h = kScreenheight - y;
	if (x + w > (int)kScreenwidth)
		w = kScreenwidth - x;
	if (h <= 0 || w <= 0)
		return;
	_system->copyRectToScreen(src, pitch, x, y, w, h);
}

void DreamWebEngine::printUnderMonitor() {
	uint8 *dst = workspace() + kScreenwidth * 43 + 76;

	Graphics::Surface *s = _system->lockScreen();
	if (!s)
		error("lockScreen failed");

	for (uint y = 0; y < 104; ++y) {
		uint8 *src = (uint8 *)s->getBasePtr(76, 43 + 8 + y);
		for (uint x = 0; x < 170; ++x) {
			if (*src < 231)
				*dst++ = *src++;
			else {
				++dst; ++src;
			}
		}
		dst += kScreenwidth - 170;
	}
	_system->unlockScreen();
}

void DreamWebEngine::cls() {
	_system->fillScreen(0);
}

uint8 DreamWebEngine::modifyChar(uint8 c) const {
	switch (getLanguage()) {
	case Common::DE_DEU:
		switch (c) {
		case 129:
			return 'Z' + 3;
		case 132:
			return 'Z' + 1;
		case 142:
			return 'Z' + 4;
		case 154:
			return 'Z' + 6;
		case 225:
			return 'A' - 1;
		case 153:
			return 'Z' + 5;
		case 148:
			return 'Z' + 2;
		default:
			return c;
		}
	case Common::ES_ESP:
		switch(c) {
		case 160:
			return 'Z' + 1;
		case 130:
			return 'Z' + 2;
		case 161:
			return 'Z' + 3;
		case 162:
			return 'Z' + 4;
		case 163:
			return 'Z' + 5;
		case 164:
			return 'Z' + 6;
		case 165:
			return ',' - 1;
		case 168:
			return 'A' - 1;
		case 173:
			return 'A' - 4;
		case 129:
			return 'A' - 5;
		default:
			return c;
		}
	case Common::FR_FRA:
	case Common::IT_ITA:
		switch(c) {
		case 133:
			return 'Z' + 1;
		case 130:
			return 'Z' + 2;
		case 138:
			return 'Z' + 3;
		case 136:
			return 'Z' + 4;
		case 140:
			return 'Z' + 5;
		case 135:
			return 'Z' + 6;
		case 149:
			return ',' - 1;
		case 131:
			return ',' - 2;
		case 141:
			return ',' - 3;
		case 139:
			return ',' - 4;
		case 151:
			return 'A' - 1;
		case 147:
			return 'A' - 3;
		case 150:
			return 'A' - 4;
		default:
			return c;
		}
	case Common::RU_RUS:
		if (c >= 224)
			c -= 48;
		// fall through
	default:
		return c;
	}
}

Common::Path DreamWebEngine::modifyFileName(const char *name) {
	Common::String fileName(name);

	// Sanity check
	if (!fileName.hasPrefix("DREAMWEB."))
		return Common::Path(fileName);

	// Make sure we use the correct file name as it differs depending on the game variant
	fileName = _datafilePrefix;
	fileName += name + 9;
	return Common::Path(fileName);
}

bool DreamWebEngine::hasSpeech() {
	return isCD() && _hasSpeech;
}

} // End of namespace DreamWeb
