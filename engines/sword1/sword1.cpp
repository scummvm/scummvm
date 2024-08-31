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

#include "sword1/sword1.h"

#include "sword1/resman.h"
#include "sword1/objectman.h"
#include "sword1/mouse.h"
#include "sword1/logic.h"
#include "sword1/sound.h"
#include "sword1/screen.h"
#include "sword1/swordres.h"
#include "sword1/menu.h"
#include "sword1/control.h"

#include "common/config-manager.h"
#include "common/textconsole.h"
#include "common/timer.h"

#include "engines/advancedDetector.h"
#include "engines/util.h"

#include "gui/message.h"

namespace Sword1 {

SystemVars SwordEngine::_systemVars;

SwordEngine::SwordEngine(OSystem *syst, const ADGameDescription *gameDesc)
	: Engine(syst) {

	_features = gameDesc->flags;
	_systemVars.platform = gameDesc->platform;

	// Add default file directories
	const Common::FSNode gameDataDir(ConfMan.getPath("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "music");
	SearchMan.addSubDirectoryMatching(gameDataDir, "speech");
	SearchMan.addSubDirectoryMatching(gameDataDir, "streams"); // PSX videos

	//SearchMan.addSubDirectoryMatching(gameDataDir, "clusters"); // Comes from AD
	SearchMan.addSubDirectoryMatching(gameDataDir, "video"); // Comes from AD
	SearchMan.addSubDirectoryMatching(gameDataDir, "smackshi"); // Comes from AD
	//SearchMan.addSubDirectoryMatching(gameDataDir, "english"); // PSX Demo  // Comes from AD
	//SearchMan.addSubDirectoryMatching(gameDataDir, "italian"); // PSX Demo  // Comes from AD

	setDebugger(new SwordConsole(this));

	_mouseState = 0;
	_resMan = 0;
	_objectMan = 0;
	_screen = 0;
	_mouse = 0;
	_logic = 0;
	_sound = 0;
	_menu = 0;
	_control = 0;
}

SwordEngine::~SwordEngine() {
	delete _control;
	delete _logic;
	delete _menu;
	delete _sound;
	delete _screen;
	delete _mouse;
	delete _objectMan;
	delete _resMan;
}

Common::Error SwordEngine::init() {

	initGraphics(640, 480);

	checkCdFiles();

	debug(5, "Starting resource manager");
	_resMan = new ResMan("swordres.rif", _systemVars.platform == Common::kPlatformMacintosh,
		Common::parseLanguage(ConfMan.get("language")) == Common::KO_KOR);
	debug(5, "Starting object manager");
	_objectMan = new ObjectMan(_resMan);
	_mouse = new Mouse(_system, _resMan, _objectMan);
	_screen = new Screen(_system, this, _resMan, _objectMan);
	_sound = new Sound(_mixer, this, _resMan);
	_menu = new Menu(_screen, _mouse);
	_logic = new Logic(this, _objectMan, _resMan, _screen, _mouse, _sound, _menu, _system, _mixer);
	_mouse->useLogicAndMenu(_logic, _menu);
	_mouse->useScreenMutex(&_screen->_screenAccessMutex);

	// Init the virtual mouse coordinates to be at the center of the screen
	_mouseCoord.x = SCREEN_WIDTH / 2;
	_mouseCoord.y = SCREEN_FULL_DEPTH / 2;

	syncSoundSettings();

	_systemVars.justRestoredGame = 0;
	_systemVars.currentCD = 0;
	_systemVars.controlPanelMode = CP_NEWGAME;
	_systemVars.saveGameFlag = SGF_DONE;
	_systemVars.snrStatus = SNR_BLANK;
	_systemVars.wantFade = true;
	_systemVars.realLanguage = Common::parseLanguage(ConfMan.get("language"));
	_systemVars.isLangRtl = false;
	_systemVars.debugMode = (gDebugLevel >= 0);
	_systemVars.slowMode = false;
	_systemVars.fastMode = false;
	_systemVars.parallaxOn = true;

	Common::Language langOverride = Common::UNK_LANG;

	if (ConfMan.hasKey("subtitles_language_override", _targetName)) {
		langOverride = Common::parseLanguage(ConfMan.get("subtitles_language_override"));

		// Versions with extended language support have seven languages instead of five...
		bool hasExtendedLangSupport = (_systemVars.realLanguage == Common::PT_BRA || _systemVars.realLanguage == Common::CS_CZE);

		// ...but let's avoid setting one of the extra languages in a five-languages edition
		if (!hasExtendedLangSupport && (langOverride == Common::PT_BRA || langOverride == Common::CS_CZE))
			langOverride = Common::UNK_LANG;
	}

	switch (langOverride != Common::UNK_LANG ? langOverride : _systemVars.realLanguage) {
	case Common::DE_DEU:
		_systemVars.language = BS1_GERMAN;
		break;
	case Common::FR_FRA:
		_systemVars.language = BS1_FRENCH;
		break;
	case Common::IT_ITA:
		_systemVars.language = BS1_ITALIAN;
		break;
	case Common::ES_ESP:
		_systemVars.language = BS1_SPANISH;
		break;
	case Common::PT_BRA:
		_systemVars.language = BS1_PORT;
		break;
	case Common::CS_CZE:
		_systemVars.language = BS1_CZECH;
		break;
	case Common::HE_ISR:
		// Hebrew is using "faked" English
		_systemVars.language = BS1_ENGLISH;
		_systemVars.isLangRtl = true;
		break;
	default:
		_systemVars.language = BS1_ENGLISH;
		break;
	}

	_systemVars.showText = ConfMan.getBool("subtitles");
	_systemVars.textNumber = 0;
	_systemVars.playSpeech = true;
	_mouseState = 0;

	_systemVars.gamePaused = false;
	_systemVars.displayDebugText = false;
	_systemVars.displayDebugMouse = false;
	_systemVars.displayDebugGrid = false;
	_systemVars.framesPerSecondCounter = 0;
	_systemVars.gameCycle = 0;

	// Differences between the DOS and Windows audio drivers:
	// * DOS:
	//    - Apparently uses linear volume curves, as per the AIL/Miles drivers;
	//    - Music can fade in and out;
	//    - Fades sound effects in and out at each scene change;
	//    - Lowers the volume of the music each time a speech line is playing;
	// 
	// * Windows:
	//    - Uses DirectSound with volume tables for game volume -> decibel 
	//      volume conversion; the resulting logarithmic curve and slightly
	//      different volume scaling for sound effects ensures audibly
	//      different intensities for some sounds;
	//    - Music can only fade out, not in;
	//    - Does NOT sound effects in and out at each scene change;
	//    - Does NOT lower the volume of the music each time a speech line is playing;

	ConfMan.registerDefault("windows_audio_mode", false);
	if (ConfMan.hasKey("windows_audio_mode", _targetName)) {
		_systemVars.useWindowsAudioMode = !(SwordEngine::isPsx() || SwordEngine::isMac()) && ConfMan.getBool("windows_audio_mode");
	}

	// Some Mac versions use big endian for the speech files but not all of them.
	if (_systemVars.platform == Common::kPlatformMacintosh)
		_sound->checkSpeechFileEndianness();

	_logic->initialize();
	_objectMan->initialize();
	_mouse->initialize();
	_control = new Control(this, _saveFileMan, _resMan, _objectMan, _system, _mouse, _sound, _screen, _logic);
	_logic->setControlPanelObject(_control);

	return Common::kNoError;
}

void SwordEngine::reinitialize() {
	_sound->clearAllFx();
	_resMan->flush(); // free everything that's currently alloced and opened. (*evil*)

	_logic->initialize();     // now reinitialize these objects as they (may) have locked
	_objectMan->initialize(); // resources which have just been wiped.
	_mouse->initialize();
	_system->warpMouse(320, 240);
	_systemVars.wantFade = true;
}

void SwordEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	_sound->getVolumes();
}

void SwordEngine::flagsToBool(bool *dest, uint8 flags) {
	uint8 bitPos = 0;
	while (flags) {
		if (flags & 1)
			dest[bitPos] = true;
		flags >>= 1;
		bitPos++;
	}
}

void SwordEngine::checkKeys() {

	if (_systemVars.gamePaused) {
		_sound->pauseSpeech();
		_sound->pauseMusic();
		_sound->pauseFx();

		while (_customType != kActionPause && !Engine::shouldQuit()) {
			pollInput(0);
			_sound->updateMusicStreaming();
		}

		_sound->unpauseSpeech();
		_sound->unpauseMusic();
		_sound->unpauseFx();

		_systemVars.gamePaused = false;
		_keyPressed.reset();
		_customType = kActionNone;
	}

	switch (_customType) {
	case kActionMainPanel:
	case kActionEscape:
		if ((Logic::_scriptVars[MOUSE_STATUS] & 1) &&
			(Logic::_scriptVars[GEORGE_HOLDING_PIECE] == 0) &&
			(Logic::_scriptVars[SCREEN] != 91)) { // Disable the save screen on the phone envelope room!
			_systemVars.saveGameFlag = SGF_SAVE;
			_systemVars.snrStatus = SNR_MAINPANEL;
		}

		break;
	case kActionQuit:
		Engine::quitGame();

		break;
	case kActionPause:
		_systemVars.gamePaused = true;
		break;
	default:
		break;
	}

	// Debug keys!
	if (!_systemVars.isDemo && _systemVars.debugMode) {
		switch (_keyPressed.keycode) {
		case Common::KEYCODE_t: // CTRL-T: Toggles debug text
			if (_keyPressed.hasFlags(Common::KBD_CTRL))
				_systemVars.displayDebugText = !_systemVars.displayDebugText;
			break;
		case Common::KEYCODE_m: // SHIFT-M: Toggles debug mouse tracking
			// This was originally CTRL-M, but ScummVM steals that event to
			// lock the mouse cursor within the window boundaries.
			if (_keyPressed.hasFlags(Common::KBD_SHIFT))
				_systemVars.displayDebugMouse = !_systemVars.displayDebugMouse;
			_screen->fullRefresh(true);
			break;
		case Common::KEYCODE_g: // CTRL-G: Toggles walkgrid displaying
			if (_keyPressed.hasFlags(Common::KBD_CTRL))
				_systemVars.displayDebugGrid = !_systemVars.displayDebugGrid;
			_screen->fullRefresh(true);
			break;
		case Common::KEYCODE_1: // Slow mode
			{
				if (_systemVars.slowMode) {
					_systemVars.slowMode = false;
					_targetFrameTime = DEFAULT_FRAME_TIME; // 12.5Hz
				} else {
					_systemVars.slowMode = true;
					_targetFrameTime = SLOW_FRAME_TIME; // 2Hz
				}

				_systemVars.fastMode = false; // For good measure...

				_rate = _targetFrameTime / 10;
			}
			break;
		case Common::KEYCODE_4: // Fast mode
			{
				if (_systemVars.fastMode) {
					_systemVars.fastMode = false;
					_targetFrameTime = DEFAULT_FRAME_TIME; // 12.5Hz
				} else {
					_systemVars.fastMode = true;
					_targetFrameTime = FAST_FRAME_TIME; // 100Hz
				}

				_systemVars.slowMode = false; // For good measure...

				_rate = _targetFrameTime / 10;
			}
			break;
		default:
			break;
		}
	}
}

static const char *const errorMsgs[] = {
	"The file \"%s\" is missing and the game doesn't work without it.\n"
	"Please copy it from CD %d and try starting the game again.\n"
	"The Readme file also contains further information.",

	"%d important files are missing, the game can't start without them.\n"
	"Please copy these files from their corresponding CDs:\n",

	"The file \"%s\" is missing.\n"
	"Even though the game may initially seem to\n"
	"work fine, it will crash when it needs the\n"
	"data from this file and you will be thrown back to your last savegame.\n"
	"Please copy the file from CD %d and start the game again.",

	"%d files are missing.\n"
	"Even though the game may initially seem to\n"
	"work fine, it will crash when it needs the\n"
	"data from these files and you will be thrown back to your last savegame.\n"
	"Please copy these files from their corresponding CDs:\n"
};

const CdFile SwordEngine::_pcCdFileList[] = {
	{ "paris2.clu", FLAG_CD1 },
	{ "ireland.clu", FLAG_CD2 },
	{ "paris3.clu", FLAG_CD1 },
	{ "paris4.clu", FLAG_CD1 },
	{ "scotland.clu", FLAG_CD2 },
	{ "spain.clu", FLAG_CD2 },
	{ "syria.clu", FLAG_CD2 },
	{ "train.clu", FLAG_CD2 },
	{ "compacts.clu", FLAG_CD1 | FLAG_DEMO | FLAG_IMMED },
	{ "general.clu", FLAG_CD1 | FLAG_DEMO | FLAG_IMMED },
	{ "maps.clu", FLAG_CD1 | FLAG_DEMO },
	{ "paris1.clu", FLAG_CD1 | FLAG_DEMO },
	{ "scripts.clu", FLAG_CD1 | FLAG_DEMO | FLAG_IMMED },
	{ "swordres.rif", FLAG_CD1 | FLAG_DEMO | FLAG_IMMED },
	{ "text.clu", FLAG_CD1 | FLAG_DEMO },
	{ "1m14a.wav", FLAG_DEMO },
	{ "speech1.clu", FLAG_SPEECH1 },
	{ "speech2.clu", FLAG_SPEECH2 }
#ifdef USE_FLAC
	, { "speech1.clf", FLAG_SPEECH1 },
	{ "speech2.clf", FLAG_SPEECH2 }
#endif
#ifdef USE_VORBIS
	, { "speech1.clv", FLAG_SPEECH1 },
	{ "speech2.clv", FLAG_SPEECH2 }
#endif
#ifdef USE_MAD
	, { "speech1.cl3", FLAG_SPEECH1 },
	{ "speech2.cl3", FLAG_SPEECH2 }
#endif
};

const CdFile SwordEngine::_macCdFileList[] = {
	{ "paris2.clm", FLAG_CD1 },
	{ "ireland.clm", FLAG_CD2 },
	{ "paris3.clm", FLAG_CD1 },
	{ "paris4.clm", FLAG_CD1 },
	{ "scotland.clm", FLAG_CD2 },
	{ "spain.clm", FLAG_CD2 },
	{ "syria.clm", FLAG_CD2 },
	{ "train.clm", FLAG_CD2 },
	{ "compacts.clm", FLAG_CD1 | FLAG_DEMO | FLAG_IMMED },
	{ "general.clm", FLAG_CD1 | FLAG_DEMO | FLAG_IMMED },
	{ "maps.clm", FLAG_CD1 | FLAG_DEMO },
	{ "paris1.clm", FLAG_CD1 | FLAG_DEMO },
	{ "scripts.clm", FLAG_CD1 | FLAG_DEMO | FLAG_IMMED },
	{ "swordres.rif", FLAG_CD1 | FLAG_DEMO | FLAG_IMMED },
	{ "text.clm", FLAG_CD1 | FLAG_DEMO },
	{ "speech1.clu", FLAG_SPEECH1 },
	{ "speech2.clu", FLAG_SPEECH2 }
#ifdef USE_FLAC
	,{ "speech1.clf", FLAG_SPEECH1 },
	{ "speech2.clf", FLAG_SPEECH2 }
#endif
#ifdef USE_VORBIS
	,{ "speech1.clv", FLAG_SPEECH1 },
	{ "speech2.clv", FLAG_SPEECH2 }
#endif
#ifdef USE_MAD
	,{ "speech1.cl3", FLAG_SPEECH1 },
	{ "speech2.cl3", FLAG_SPEECH2 }
#endif
};

const CdFile SwordEngine::_psxCdFileList[] = { // PSX edition has only one cd
	{ "paris2.clu", FLAG_CD1 },
	{ "ireland.clu", FLAG_CD1 },
	{ "paris3.clu", FLAG_CD1 },
	{ "paris4.clu", FLAG_CD1 },
	{ "scotland.clu", FLAG_CD1 },
	{ "spain.clu", FLAG_CD1 },
	{ "syria.clu", FLAG_CD1 },
	{ "train.clu", FLAG_CD1 },
	{ "train.plx", FLAG_CD1 },
	{ "compacts.clu", FLAG_CD1 | FLAG_DEMO | FLAG_IMMED },
	{ "general.clu", FLAG_CD1 | FLAG_DEMO | FLAG_IMMED },
	{ "maps.clu", FLAG_CD1 | FLAG_DEMO },
	{ "paris1.clu", FLAG_CD1 | FLAG_DEMO},
	{ "scripts.clu", FLAG_CD1 | FLAG_DEMO | FLAG_IMMED },
	{ "swordres.rif", FLAG_CD1 | FLAG_DEMO | FLAG_IMMED },
	{ "text.clu", FLAG_CD1 | FLAG_DEMO },
	{ "speech.dat", FLAG_SPEECH1 | FLAG_DEMO },
	{ "speech.tab", FLAG_SPEECH1 | FLAG_DEMO },
	{ "speech.inf", FLAG_SPEECH1 | FLAG_DEMO },
	{ "speech.lis", FLAG_SPEECH1 | FLAG_DEMO }
};

void SwordEngine::showFileErrorMsg(uint8 type, bool *fileExists) {
	char msg[1024];
	int missCnt = 0, missNum = 0;

	if (SwordEngine::isMac()) {
		for (int i = 0; i < ARRAYSIZE(_macCdFileList); i++)
			if (!fileExists[i]) {
				missCnt++;
				missNum = i;
			}
		assert(missCnt > 0); // this function shouldn't get called if there's nothing missing.
		warning("%d files missing", missCnt);
		int msgId = (type == TYPE_IMMED) ? 0 : 2;
		if (missCnt == 1) {
			Common::sprintf_s(msg, errorMsgs[msgId],
			        _macCdFileList[missNum].name, (_macCdFileList[missNum].flags & FLAG_CD2) ? 2 : 1);
			warning("%s", msg);
		} else {
			char *pos = msg + Common::sprintf_s(msg, errorMsgs[msgId + 1], missCnt);
			warning("%s", msg);
			for (int i = 0; i < ARRAYSIZE(_macCdFileList); i++)
				if (!fileExists[i]) {
					warning("\"%s\" (CD %d)", _macCdFileList[i].name, (_macCdFileList[i].flags & FLAG_CD2) ? 2 : 1);
					pos += Common::sprintf_s(pos, sizeof(msg) - (pos - msg),
						"\"%s\" (CD %d)\n", _macCdFileList[i].name, (_macCdFileList[i].flags & FLAG_CD2) ? 2 : 1);
				}
		}
	} else if (SwordEngine::isPsx()) {
		for (int i = 0; i < ARRAYSIZE(_psxCdFileList); i++)
			if (!fileExists[i]) {
				missCnt++;
				missNum = i;
			}
		assert(missCnt > 0); // this function shouldn't get called if there's nothing missing.
		warning("%d files missing", missCnt);
		int msgId = (type == TYPE_IMMED) ? 0 : 2;
		if (missCnt == 1) {
			Common::sprintf_s(msg, errorMsgs[msgId], _psxCdFileList[missNum].name, 1);
			warning("%s", msg);
		} else {
			char *pos = msg + Common::sprintf_s(msg, errorMsgs[msgId + 1], missCnt);
			warning("%s", msg);
			for (int i = 0; i < ARRAYSIZE(_psxCdFileList); i++)
				if (!fileExists[i]) {
					warning("\"%s\"", _macCdFileList[i].name);
					pos += Common::sprintf_s(pos, sizeof(msg) - (pos - msg),
						"\"%s\"\n", _macCdFileList[i].name);
				}
		}
	} else {
		for (int i = 0; i < ARRAYSIZE(_pcCdFileList); i++)
			if (!fileExists[i]) {
				missCnt++;
				missNum = i;
			}
		assert(missCnt > 0); // this function shouldn't get called if there's nothing missing.
		warning("%d files missing", missCnt);
		int msgId = (type == TYPE_IMMED) ? 0 : 2;
		if (missCnt == 1) {
			Common::sprintf_s(msg, errorMsgs[msgId],
			        _pcCdFileList[missNum].name, (_pcCdFileList[missNum].flags & FLAG_CD2) ? 2 : 1);
			warning("%s", msg);
		} else {
			char *pos = msg + Common::sprintf_s(msg, errorMsgs[msgId + 1], missCnt);
			warning("%s", msg);
			for (int i = 0; i < ARRAYSIZE(_pcCdFileList); i++)
				if (!fileExists[i]) {
					warning("\"%s\" (CD %d)", _pcCdFileList[i].name, (_pcCdFileList[i].flags & FLAG_CD2) ? 2 : 1);
					pos += Common::sprintf_s(pos, sizeof(msg) - (pos - msg),
						"\"%s\" (CD %d)\n", _pcCdFileList[i].name, (_pcCdFileList[i].flags & FLAG_CD2) ? 2 : 1);
				}
		}
	}
	GUI::MessageDialog dialog(msg);
	dialog.runModal();
	if (type == TYPE_IMMED) // we can't start without this file, so error() out.
		error("%s", msg);
}

void SwordEngine::checkCdFiles() { // check if we're running from cd, hdd or what...
	bool fileExists[30];
	bool isFullVersion = false; // default to demo version
	bool missingTypes[8] = { false, false, false, false, false, false, false, false };
	bool foundTypes[8] = { false, false, false, false, false, false, false, false };
	bool cd2FilesFound = false;
	_systemVars.runningFromCd = false;
	_systemVars.playSpeech = true;

	// check all files and look out if we can find a file that wouldn't exist if this was the demo version
	if (SwordEngine::isMac()) {
		for (int fcnt = 0; fcnt < ARRAYSIZE(_macCdFileList); fcnt++) {
			if (Common::File::exists(_macCdFileList[fcnt].name)) {
				fileExists[fcnt] = true;
				flagsToBool(foundTypes, _macCdFileList[fcnt].flags);
				if (!(_macCdFileList[fcnt].flags & FLAG_DEMO))
					isFullVersion = true;
				if (_macCdFileList[fcnt].flags & FLAG_CD2)
					cd2FilesFound = true;
			} else {
				flagsToBool(missingTypes, _macCdFileList[fcnt].flags);
				fileExists[fcnt] = false;
			}
		}
	} else if (SwordEngine::isPsx()) {
		for (int fcnt = 0; fcnt < ARRAYSIZE(_psxCdFileList); fcnt++) {
			if (Common::File::exists(_psxCdFileList[fcnt].name)) {
				fileExists[fcnt] = true;
				flagsToBool(foundTypes, _psxCdFileList[fcnt].flags);
				if (!(_psxCdFileList[fcnt].flags & FLAG_DEMO))
					isFullVersion = true;
				cd2FilesFound = true;
			} else {
				flagsToBool(missingTypes, _psxCdFileList[fcnt].flags);
				fileExists[fcnt] = false;
			}
		}
	} else {
		for (int fcnt = 0; fcnt < ARRAYSIZE(_pcCdFileList); fcnt++) {
			if (Common::File::exists(_pcCdFileList[fcnt].name)) {
				fileExists[fcnt] = true;
				flagsToBool(foundTypes, _pcCdFileList[fcnt].flags);
				if (!(_pcCdFileList[fcnt].flags & FLAG_DEMO))
					isFullVersion = true;
				if (_pcCdFileList[fcnt].flags & FLAG_CD2)
					cd2FilesFound = true;
			} else {
				flagsToBool(missingTypes, _pcCdFileList[fcnt].flags);
				fileExists[fcnt] = false;
			}
		}
	}

	if (((_features & ADGF_DEMO) == 0) != isFullVersion) // shouldn't happen...
		warning("Your Broken Sword 1 version looks like a %s version but you are starting it as a %s version", isFullVersion ? "full" : "demo", (_features & ADGF_DEMO) ? "demo" : "full");

	if (foundTypes[TYPE_SPEECH1]) // we found some kind of speech1 file (.clu, .cl3, .clv)
		missingTypes[TYPE_SPEECH1] = false; // so we don't care if there's a different kind missing
	if (foundTypes[TYPE_SPEECH2]) // same for speech2
		missingTypes[TYPE_SPEECH2] = false;

	if (isFullVersion)                   // if this is the full version...
		missingTypes[TYPE_DEMO] = false; // then we don't need demo files...
	else                                 // and vice versa
		missingTypes[TYPE_SPEECH1] = missingTypes[TYPE_SPEECH2] = missingTypes[TYPE_CD1] = missingTypes[TYPE_CD2] = false;

	bool somethingMissing = false;
	for (int i = 0; i < 8; i++)
		somethingMissing |= missingTypes[i];
	if (somethingMissing) { // okay, there *are* files missing
		// first, update the fileExists[] array depending on our changed missingTypes
		if (SwordEngine::isMac()) {
			for (int fileCnt = 0; fileCnt < ARRAYSIZE(_macCdFileList); fileCnt++)
				if (!fileExists[fileCnt]) {
					fileExists[fileCnt] = true;
					for (int flagCnt = 0; flagCnt < 8; flagCnt++)
						if (missingTypes[flagCnt] && ((_macCdFileList[fileCnt].flags & (1 << flagCnt)) != 0))
							fileExists[fileCnt] = false; // this is one of the files we were looking for
				}
		} else if (SwordEngine::isPsx()) {
			for (int fileCnt = 0; fileCnt < ARRAYSIZE(_psxCdFileList); fileCnt++)
				if (!fileExists[fileCnt]) {
					fileExists[fileCnt] = true;
					for (int flagCnt = 0; flagCnt < 8; flagCnt++)
						if (missingTypes[flagCnt] && ((_psxCdFileList[fileCnt].flags & (1 << flagCnt)) != 0))
							fileExists[fileCnt] = false; // this is one of the files we were looking for
				}
		} else {
			for (int fileCnt = 0; fileCnt < ARRAYSIZE(_pcCdFileList); fileCnt++)
				if (!fileExists[fileCnt]) {
					fileExists[fileCnt] = true;
					for (int flagCnt = 0; flagCnt < 8; flagCnt++)
						if (missingTypes[flagCnt] && ((_pcCdFileList[fileCnt].flags & (1 << flagCnt)) != 0))
							fileExists[fileCnt] = false; // this is one of the files we were looking for
				}
		}
		if (missingTypes[TYPE_IMMED]) {
			// important files missing, can't start the game without them
			showFileErrorMsg(TYPE_IMMED, fileExists);
		} else if ((!missingTypes[TYPE_CD1]) && !cd2FilesFound) {
			/* we have all the data from cd one, but not a single one from CD2.
			    I'm not sure how we should handle this, for now I'll just assume that the
			    user has set up the extrapath correctly and copied the necessary files to HDD.
			    A quite optimistic assumption, I'd say. Maybe we should change this for the release
			    to warn the user? */
			warning("CD2 data files not found. I hope you know what you're doing and that\n"
			        "you have set up the extrapath and additional data correctly.\n"
			        "If you didn't, you should better read the ScummVM readme file");
			_systemVars.runningFromCd = true;
			_systemVars.playSpeech = true;
		} else if (missingTypes[TYPE_CD1] || missingTypes[TYPE_CD2]) {
			// several files from CD1 both CDs are missing. we can probably start, but it'll crash sooner or later
			showFileErrorMsg(TYPE_CD1, fileExists);
		} else if (missingTypes[TYPE_SPEECH1] || missingTypes[TYPE_SPEECH2]) {
			// not so important, but there won't be any voices
			if (missingTypes[TYPE_SPEECH1] && missingTypes[TYPE_SPEECH2])
				warning("Unable to find the speech files. The game will work, but you won't hear any voice output.\n"
				        "Please copy the SPEECH.CLU files from both CDs and rename them to SPEECH1.CLU and SPEECH2.CLU,\n"
				        "corresponding to the CD number.\n"
				        "Please read the ScummVM Readme file for more information");
			else
				warning("Unable to find the speech file from CD %d.\n"
				        "You won't hear any voice output in that part of the game.\n"
				        "Please read the ScummVM Readme file for more information", missingTypes[TYPE_SPEECH1] ? 1 : 2);
		} else if (missingTypes[TYPE_DEMO]) {
			// for the demo version, we simply expect to have all files immediately
			showFileErrorMsg(TYPE_IMMED, fileExists);
		}
	} // everything's fine, let's play.
	/*if (!isFullVersion)
		_systemVars.isDemo = true;
	*/
	// make the demo flag depend on the Gamesettings for now, and not on what the datafiles look like
	_systemVars.isDemo = (_features & ADGF_DEMO) != 0;

	// Spanish demo has proper speech.clu and uses normal sound and var mapping
	_systemVars.isSpanishDemo = (_systemVars.isDemo && Common::parseLanguage(ConfMan.get("language")) == Common::ES_ESP) != 0;
}

Common::Error SwordEngine::go() {
	_control->checkForOldSaveGames();
	setTotalPlayTime(0);

	_screen->initFadePaletteServer();
	installTimerRoutines();

	bool startedFromGMM = false;
	uint16 startPos = ConfMan.getInt("boot_param");

	if (startPos) {
		_logic->startPositions(startPos);
	} else {
		int saveSlot = ConfMan.getInt("save_slot");
		// Savegames are numbered starting from 1 in the dialog window,
		// but their filenames are numbered starting from 0.
		if (saveSlot >= 0 && _control->savegamesExist() && _control->restoreGameFromFile(saveSlot)) {
			_control->doRestore();
			startedFromGMM = true;
			_systemVars.controlPanelMode = CP_NORMAL;
		} else if (_control->savegamesExist()) {
			_systemVars.snrStatus = SNR_MAINPANEL;
			_systemVars.controlPanelMode = CP_NEWGAME;
			_control->getPlayerOptions();

			// If player clicked on "Start" (Restart)
			// just ignore it - so game can start from 'startPos'
			// (which will be '0' for normal game anyway)
			if (_systemVars.saveGameFlag == SGF_RESTART)
				_systemVars.saveGameFlag = SGF_DONE;
		}
	}

	while (!shouldQuit()) {
		if (_systemVars.saveGameFlag == SGF_RESTORE) {
			debug(1, "SwordEngine::go(): Restoring game");
			if (!_control->restoreGame())
				warning("SwordEngine::go(): Couldn't restore game");

		} else if (_systemVars.saveGameFlag == SGF_RESTART) {
			debug(1, "SwordEngine::go(): Restarting game");
			startPos = 0;
			_logic->startPositions(startPos);
		} else if (!startedFromGMM) { // START GAME
			_logic->startPositions(startPos);
			startPos = 0;
		}

		mainLoop();

		if (!shouldQuit()) {
			// the mainloop was left, we have to reinitialize.
			reinitialize();
		}
	}

	uninstallTimerRoutines();

	return Common::kNoError;
}

void SwordEngine::showDebugInfo() {
	Object *playerCompact = _objectMan->fetchObject(PLAYER);

	// Screen coordinates for game cycle string
	int32 gameCycleX = Logic::_scriptVars[SCROLL_OFFSET_X] + 130;
	int32 gameCycleY = Logic::_scriptVars[SCROLL_OFFSET_Y] + 125;

	// Screen coordinates for mouse coordinates string
	int32 mouseCoordsX = Logic::_scriptVars[SCROLL_OFFSET_X] + 220;
	int32 mouseCoordsY = Logic::_scriptVars[SCROLL_OFFSET_Y] + 125;

	// Screen coordinates for special item string
	int32 specialItemX = Logic::_scriptVars[SCROLL_OFFSET_X] + 350;
	int32 specialItemY = Logic::_scriptVars[SCROLL_OFFSET_Y] + 125;

	// Screen coordinates for player coordinates string
	int32 playerCoordsX = Logic::_scriptVars[SCROLL_OFFSET_X] + 475;
	int32 playerCoordsY = Logic::_scriptVars[SCROLL_OFFSET_Y] + 125;

	// Screen coordinates for Paris flag string
	int32 parisFlagX = Logic::_scriptVars[SCROLL_OFFSET_X] + 590;
	int32 parisFlagY = Logic::_scriptVars[SCROLL_OFFSET_Y] + 125;

	// Screen coordinates for player's script level string
	int32 scriptLevelX = Logic::_scriptVars[SCROLL_OFFSET_X] + 660;
	int32 scriptLevelY = Logic::_scriptVars[SCROLL_OFFSET_Y] + 125;

	// Screen coordinates for the talk flag string
	int32 talkFlagX = Logic::_scriptVars[SCROLL_OFFSET_X] + 720;
	int32 talkFlagY = Logic::_scriptVars[SCROLL_OFFSET_Y] + 125;

	// Screen coordinates for FPS counter string
	int32 fpsX = Logic::_scriptVars[SCROLL_OFFSET_X] + 130;
	int32 fpsY = Logic::_scriptVars[SCROLL_OFFSET_Y] + 145;

	// Screen coordinates for game speed string
	int32 gameSpeedX = Logic::_scriptVars[SCROLL_OFFSET_X] + 220;
	int32 gameSpeedY = Logic::_scriptVars[SCROLL_OFFSET_Y] + 145;

	// Screen coordinates for screen number string
	int32 screenX = Logic::_scriptVars[SCROLL_OFFSET_X] + 350;
	int32 screenY = Logic::_scriptVars[SCROLL_OFFSET_Y] + 145;

	// Screen coordinates for current CD string
	int32 currentCDX = Logic::_scriptVars[SCROLL_OFFSET_X] + 475;
	int32 currentCDY = Logic::_scriptVars[SCROLL_OFFSET_Y] + 145;

	// Screen coordinates for the end sequence phase string
	int32 endSceneX = Logic::_scriptVars[SCROLL_OFFSET_X] + 590;
	int32 endSceneY = Logic::_scriptVars[SCROLL_OFFSET_Y] + 145;

	// Screen coordinates for the current text line number string
	int32 textNoX = Logic::_scriptVars[SCROLL_OFFSET_X] + 130;
	int32 textNoY = Logic::_scriptVars[SCROLL_OFFSET_Y] + 165;

	// Screen coordinates for debug flags string
	int32 debugFlagsX = Logic::_scriptVars[SCROLL_OFFSET_X] + 130;
	int32 debugFlagsY = Logic::_scriptVars[SCROLL_OFFSET_Y] + 185;

	// Screen coordinates for the paused message string
	int32 pausedX = Logic::_scriptVars[SCROLL_OFFSET_X] + 400;
	int32 pausedY = Logic::_scriptVars[SCROLL_OFFSET_Y] + 315;

	if (SwordEngine::isPsx()) {
		pausedX += 20;
		pausedY -= 16;
	}

	uint8 buf[255];

	if (_systemVars.gamePaused) {
		Common::sprintf_s(buf, "%s", _control->getPauseString());
		_screen->printDebugLine(buf, ' ', pausedX, pausedY);
	}

	if ((_systemVars.displayDebugText) && (!_systemVars.isDemo)) {
		// Game cycle
		Common::sprintf_s(buf, "%d", _systemVars.gameCycle);
		_screen->printDebugLine(buf, ' ', gameCycleX, gameCycleY);

		// Mouse coordinates
		Common::sprintf_s(buf, "m %d,%d", Logic::_scriptVars[MOUSE_X], Logic::_scriptVars[MOUSE_Y]);
		_screen->printDebugLine(buf, ' ', mouseCoordsX, mouseCoordsY);

		// Special item
		Common::sprintf_s(buf, "id %d", Logic::_scriptVars[SPECIAL_ITEM]);
		_screen->printDebugLine(buf, ' ', specialItemX, specialItemY);

		// Player coordinates
		Common::sprintf_s(buf, "G %d,%d", playerCompact->o_xcoord, playerCompact->o_ycoord);
		_screen->printDebugLine(buf, ' ', playerCoordsX, playerCoordsY);

		// Paris status flag
		Common::sprintf_s(buf, "pf %d", Logic::_scriptVars[PARIS_FLAG]);
		_screen->printDebugLine(buf, ' ', parisFlagX, parisFlagY);

		// Player script level
		Common::sprintf_s(buf, "lv %d", playerCompact->o_tree.o_script_level);
		_screen->printDebugLine(buf, ' ', scriptLevelX, scriptLevelY);

		// Talk flag
		Common::sprintf_s(buf, "tf %d", Logic::_scriptVars[TALK_FLAG]);
		_screen->printDebugLine(buf, ' ', talkFlagX, talkFlagY);

		// Frames per second
		Common::sprintf_s(buf, "%u fps", _systemVars.framesPerSecondCounter);
		_screen->printDebugLine(buf, ' ', fpsX, fpsY);

		// Debug game speed (based on pressing keys '1' & '4')
		if (_systemVars.slowMode) {
			Common::sprintf_s(buf, "(slow)");
		} else if (_systemVars.fastMode) {
			Common::sprintf_s(buf, "(fast)");
		} else {
			Common::sprintf_s(buf, "(norm)");
		}

		_screen->printDebugLine(buf, ' ', gameSpeedX, gameSpeedY);

		// Screen number
		Common::sprintf_s(buf, "screen %d", Logic::_scriptVars[SCREEN]);
		_screen->printDebugLine(buf, ' ', screenX, screenY);

		// CD in use
		Common::sprintf_s(buf, "CD-%d", _systemVars.currentCD);
		_screen->printDebugLine(buf, ' ', currentCDX, currentCDY);

		// End sequence scene number
		if (Logic::_scriptVars[END_SCENE]) {
			Common::sprintf_s(buf, "scene %d", Logic::_scriptVars[END_SCENE]);
			_screen->printDebugLine(buf, ' ', endSceneX, endSceneY);
		}

		// Debug flags
		if ((Logic::_scriptVars[DEBUG_FLAG_1] > 0) || (Logic::_scriptVars[DEBUG_FLAG_2] > 0) || (Logic::_scriptVars[DEBUG_FLAG_3] > 0)) {
			Common::sprintf_s(buf, "debug flags: %d, %d, %d",
							  Logic::_scriptVars[DEBUG_FLAG_1],
							  Logic::_scriptVars[DEBUG_FLAG_2],
							  Logic::_scriptVars[DEBUG_FLAG_3]);
			_screen->printDebugLine(buf, ' ', debugFlagsX, debugFlagsY);
		}
	}

	if (_systemVars.displayDebugText) {
		// Text line number
		if (_logic->canShowDebugTextNumber()) {
			Common::sprintf_s(buf, "TEXT %d", _systemVars.textNumber);
			_screen->printDebugLine(buf, ' ', textNoX, textNoY);
		}
	}

	if (_systemVars.displayDebugGrid) {
		_logic->plotRouteGrid(playerCompact);
		_screen->fullRefresh(true);
	}

	if (_systemVars.displayDebugMouse) {
		// Draw a cross shaped cursor under the mouse cursor
		_screen->plotPoint(Logic::_scriptVars[MOUSE_X] - 128, Logic::_scriptVars[MOUSE_Y] - 128, 255);
		_screen->plotPoint(Logic::_scriptVars[MOUSE_X] - 130, Logic::_scriptVars[MOUSE_Y] - 128, 255);
		_screen->plotPoint(Logic::_scriptVars[MOUSE_X] - 128, Logic::_scriptVars[MOUSE_Y] - 130, 255);
		_screen->plotPoint(Logic::_scriptVars[MOUSE_X] - 128, Logic::_scriptVars[MOUSE_Y] - 126, 255);
		_screen->plotPoint(Logic::_scriptVars[MOUSE_X] - 126, Logic::_scriptVars[MOUSE_Y] - 128, 255);

		// Draw a cross shaped cursor on the player coordinates
		_screen->plotPoint(playerCompact->o_xcoord - 128, playerCompact->o_ycoord - 128, 255);
		_screen->plotPoint(playerCompact->o_xcoord - 130, playerCompact->o_ycoord - 128, 255);
		_screen->plotPoint(playerCompact->o_xcoord - 128, playerCompact->o_ycoord - 130, 255);
		_screen->plotPoint(playerCompact->o_xcoord - 128, playerCompact->o_ycoord - 126, 255);
		_screen->plotPoint(playerCompact->o_xcoord - 126, playerCompact->o_ycoord - 128, 255);

		_screen->fullRefresh(true);
	}
}

void SwordEngine::setMenuToTargetState() {
	_menu->setToTargetState();
}

void SwordEngine::checkCd() {
	uint8 needCd = _cdList[Logic::_scriptVars[NEW_SCREEN]];
	if (_systemVars.runningFromCd) { // are we running from cd?
		if (needCd == 0) { // needCd == 0 means we can use either CD1 or CD2.
			if (_systemVars.currentCD == 0) {
				_systemVars.currentCD = 1; // if there is no CD currently inserted, ask for CD1.
				askForCd();
			} // else: there is already a cd inserted and we don't care if it's cd1 or cd2.
		} else if (needCd != _systemVars.currentCD) { // we need a different CD than the one in drive.
			_sound->closeCowSystem(); // close music and sound files before changing CDs
			_systemVars.currentCD = needCd; // askForCd will ask the player to insert _systemVars.currentCd,
			askForCd();           // so it has to be updated before calling it.
		}
	} else {        // we're running from HDD, we don't have to care about music files and Sound will take care of
		if (needCd) // switching sound.clu files on Sound::newScreen by itself, so there's nothing to be done.
			_systemVars.currentCD = needCd;
		else if (_systemVars.currentCD == 0)
			_systemVars.currentCD = 1;
	}
}

void SwordEngine::askForCd() {
	char buf[255];

	_control->askForCdMessage(SwordEngine::_systemVars.currentCD, false);

	_screen->fnSetFadeTargetPalette(0, 1, 0, BORDER_BLACK); // Set colour 0 to black - for screen borders
	_screen->fnSetFadeTargetPalette(193, 1, 0, TEXT_WHITE); // Set colours 193 to white - for letters

	while (!shouldQuit()) {
		_screen->startFadePaletteUp(1);

		uint32 startTime = _system->getMillis();
		while (_screen->stillFading()) {
			if (_vblCount >= _rate)
				_vblCount = 0;

			pollInput(0);

			// In the remote event that this wait cycle gets
			// stuck during debugging, trigger a timeout
			if (_system->getMillis() - startTime > 1000)
				break;
		}

		while (_customType == kActionNone && _keyPressed.keycode == Common::KEYCODE_INVALID && !shouldQuit()) {
			pollInput(0);
		}

		startFadePaletteDown(1);

		startTime = _system->getMillis();
		while (_screen->stillFading()) {
			if (_vblCount >= _rate)
				_vblCount = 0;

			pollInput(0);

			// In the remote event that this wait cycle gets
			// stuck during debugging, trigger a timeout
			if (_system->getMillis() - startTime > 1000)
				break;
		}

		startTime = _system->getMillis();
		while (_system->getMillis() - startTime < 500) {
			pollInput(0);
		};

		_keyPressed.reset();
		_customType = kActionNone;

		// At this point the original code sets colors 1 to 180 to grey;
		// the only visible effect of this is that the screen flashes when
		// loading a save state. It's not clear what the original wanted to do.
		// for (int i = 1; i < 180; i++) {
		//     SetPalette(i, 1, _grey);
		// }

		Common::sprintf_s(buf, "cd%d.id", SwordEngine::_systemVars.currentCD);
		if (Common::File::exists(buf))
			break;

		_control->askForCdMessage(SwordEngine::_systemVars.currentCD, true);
	}
}

uint8 SwordEngine::mainLoop() {
	_keyPressed.reset();
	_customType = kActionNone;
	_systemVars.gameCycle = 1;

	do {
		if (shouldQuit())
			break;

		if (Logic::_scriptVars[NEW_SCREEN] > 50)
			_objectMan->mainLoopPatch();

		checkCd();

		_screen->newScreen(Logic::_scriptVars[NEW_SCREEN]);
		_logic->newScreen(Logic::_scriptVars[NEW_SCREEN]);
		_sound->newScreen(Logic::_scriptVars[NEW_SCREEN]);
		Logic::_scriptVars[SCREEN] = Logic::_scriptVars[NEW_SCREEN];

		do {
			uint32 newTime;
			uint32 frameTime = _system->getMillis();

			bool scrollFrameShown = false;

			_systemVars.saveGameFlag = SGF_DONE;

			_systemVars.gameCycle++;

			_logic->engine();
			_sound->setCrossFadeIncrement();
			_logic->updateScreenParams(); // sets scrolling

			_screen->draw();

			showDebugInfo();

			_mouse->animate();

			if (!Logic::_scriptVars[NEW_PALETTE]) {
				newTime = _system->getMillis();
				if ((int32)(newTime - frameTime) < _targetFrameTime / 2) {
					scrollFrameShown = _screen->showScrollFrame();
					pollInput((_targetFrameTime / 2) - (_system->getMillis() - frameTime));
				}

				_mainLoopFrameCount++;
			}

			_sound->engine();

			newTime = _system->getMillis();
			if (((int32)(newTime - frameTime) < _targetFrameTime) || (!scrollFrameShown))
				_screen->updateScreen();
			_mainLoopFrameCount++;
			pollInput((_targetFrameTime) - (_system->getMillis() - frameTime));

			_vblCount = 0; // Reset the vBlank counter for the other timers...

			// Calculation for the frames per second counter for the debug text
			if (_ticker > 5000)
				_ticker = 0;
			if (_ticker > 1000) {
				_systemVars.framesPerSecondCounter = _mainLoopFrameCount;
				_mainLoopFrameCount = 0;
				_ticker -= 1000;
			}

			_mouse->engine(_mouseCoord.x, _mouseCoord.y, _mouseState);

			checkKeys();

			if (_systemVars.saveGameFlag == SGF_SAVE) {
				_control->getPlayerOptions();
				debug(1, "SwordEngine::mainLoop(): Returned to mainloop() from getPlayerOptions()");
			}

			_mouseState = 0;
			_keyPressed.reset();
			_customType = kActionNone;

		} while ((Logic::_scriptVars[SCREEN] == Logic::_scriptVars[NEW_SCREEN]) &&
			(_systemVars.saveGameFlag == SGF_DONE || _systemVars.saveGameFlag == SGF_SAVE) &&
			(!shouldQuit()));

		if ((Logic::_scriptVars[SCREEN] != 53) && !shouldQuit()) {
			startFadePaletteDown(1);
		}

		_screen->quitScreen(); // Close graphic resources
		waitForFade();

		_sound->clearAllFx(); // Purge the sound AFTER they've been faded

		_objectMan->closeSection(Logic::_scriptVars[SCREEN]); // Close the section that PLAYER has just left, if it's empty now
	} while ((_systemVars.saveGameFlag < SGF_RESTORE) && (!shouldQuit()));

	return 0;
}

void SwordEngine::waitForFade() {
	uint32 startTime = _system->getMillis();
	while (_screen->stillFading()) { // This indirectly also waits for FX to be faded
		if (_vblCount >= _rate)
			_vblCount = 0;

		pollInput(0);

		// In the remote event that this wait cycle gets
		// stuck during debugging, trigger a timeout
		if (_system->getMillis() - startTime > 2000)
			break;
	}
}

void SwordEngine::pollInput(uint32 delay) { //copied and mutilated from sky.cpp

	Common::Event event;
	uint32 start = _system->getMillis();

	do {
		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				_customType = event.customType;
				break;
			case Common::EVENT_KEYDOWN:
				_keyPressed = event.kbd;
				break;
			case Common::EVENT_MOUSEMOVE:
				_mouseCoord = event.mouse;
				break;
			case Common::EVENT_LBUTTONDOWN:
				_mouseState |= BS1L_BUTTON_DOWN;
				_mouseCoord = event.mouse;
				break;
			case Common::EVENT_RBUTTONDOWN:
				_mouseState |= BS1R_BUTTON_DOWN;
				_mouseCoord = event.mouse;
				break;
			case Common::EVENT_LBUTTONUP:
				_mouseState |= BS1L_BUTTON_UP;
				_mouseCoord = event.mouse;
				break;
			case Common::EVENT_RBUTTONUP:
				_mouseState |= BS1R_BUTTON_UP;
				_mouseCoord = event.mouse;
				break;
			default:
				break;
			}
		}

		_sound->updateMusicStreaming();

		_screen->_screenAccessMutex.lock();
		_system->updateScreen();
		_screen->_screenAccessMutex.unlock();

		if (delay > 0)
			_system->delayMillis(10);

	} while (_system->getMillis() < start + delay);
}

bool SwordEngine::mouseIsActive() {
	return Logic::_scriptVars[MOUSE_STATUS] & 1;
}

// The following function is needed to restore proper status after GMM load game
void SwordEngine::reinitRes() {
	checkCd(); // Reset currentCD var to correct value
	_screen->newScreen(Logic::_scriptVars[NEW_SCREEN]);
	_logic->newScreen(Logic::_scriptVars[NEW_SCREEN]);
	_sound->newScreen(Logic::_scriptVars[NEW_SCREEN]);
	Logic::_scriptVars[SCREEN] = Logic::_scriptVars[NEW_SCREEN];
	_logic->engine();
	_logic->updateScreenParams();
	_screen->fullRefresh();
	_screen->draw();
}

void SwordEngine::updateTopMenu() {
	_menu->refresh(MENU_TOP);
}

void SwordEngine::updateBottomMenu() {
	_menu->refresh(MENU_BOT);
}

void SwordEngine::fadePaletteStep() {
	_screen->fadePalette();
}

void SwordEngine::startFadePaletteDown(int speed) {
	_screen->startFadePaletteDown(speed);
	_sound->fadeFxDown(speed);
}

void SwordEngine::startFadePaletteUp(int speed) {
	_screen->startFadePaletteUp(speed);
	_sound->fadeFxUp(speed);
}

static void vblCallback(void *refCon) {
	SwordEngine *vm = (SwordEngine *)refCon;

	vm->_ticker += 10;
	vm->_inTimer++;

	if (vm->_inTimer == 0) {
		vm->_vblCount++;
		vm->_vbl60HzUSecElapsed += TIMER_USEC;

		if (!vm->screenIsFading()) {
			if ((vm->_vblCount == 1) || (vm->_vblCount == 5)) {
				vm->updateTopMenu();
			}

			if ((vm->_vblCount == 3) || (vm->_vblCount == 7)) {
				vm->updateBottomMenu();
			}
		} else if (vm->fadeDirectionIsUp()) {
			// This is an optimization for all the locks introduced
			// with the fade palette changes: we disable the menu
			// updates whenever the palette is fading, and we bring
			// the menu to its target state.
			vm->setMenuToTargetState();
		}

		if (vm->_vbl60HzUSecElapsed >= PALETTE_FADE_USEC) {
			vm->_vbl60HzUSecElapsed -= PALETTE_FADE_USEC;

			vm->fadePaletteStep();
		}
	}

	vm->_inTimer--;
}

bool SwordEngine::screenIsFading() {
	return _screen->stillFading() != 0;
}

bool SwordEngine::fadeDirectionIsUp() {
	return _screen->stillFading() == 1;
}

void SwordEngine::installTimerRoutines() {
	debug(2, "SwordEngine::installTimerRoutines(): Installing timers...");
	_ticker = 0;
	getTimerManager()->installTimerProc(&vblCallback, 1000000 / TIMER_RATE, this, "AILTimer");
	_sound->installFadeTimer();
}

void SwordEngine::uninstallTimerRoutines() {
	debug(2, "SwordEngine::uninstallTimerRoutines(): Uninstalling timers...");
	getTimerManager()->removeTimerProc(&vblCallback);
	_sound->uninstallFadeTimer();
}

} // End of namespace Sword1
