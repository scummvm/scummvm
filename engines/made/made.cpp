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

#include "made/made.h"
#include "made/console.h"
#include "made/pmvplayer.h"
#include "made/resource.h"
#include "made/screen.h"
#include "made/database.h"
#include "made/script.h"
#include "made/music.h"

#include "common/config-manager.h"
#include "common/events.h"
#include "common/formats/winexe_ne.h"
#include "common/system.h"
#include "common/error.h"

#include "engines/util.h"

#include "graphics/wincursor.h"

#include "backends/audiocd/audiocd.h"

namespace Made {

MadeEngine::MadeEngine(OSystem *syst, const MadeGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {

	_eventNum = 0;
	_eventMouseX = _eventMouseY = 0;
	_eventKey = 0;

	_useWinCursors = false;

	_autoStopSound = false;
	_soundEnergyIndex = 0;
	_soundEnergyArray = nullptr;
	_musicBeatStart = 0;
	_cdTimeStart = 0;
	_introMusicDigital = true;
	if (ConfMan.hasKey("intro_music_digital"))
		_introMusicDigital = ConfMan.getBool("intro_music_digital");

	_rnd = new Common::RandomSource("made");

	setDebugger(new MadeConsole(this));

	_system->getAudioCDManager()->open();

	_pmvPlayer = new PmvPlayer(this, _mixer);
	_res = new ResourceReader();
	_screen = new Screen(this);

	if (getGameID() == GID_LGOP2 || getGameID() == GID_MANHOLE || getGameID() == GID_RODNEY) {
		_dat = new GameDatabaseV2(this);
	} else if (getGameID() == GID_RTZ) {
		_dat = new GameDatabaseV3(this);
	} else {
		error("Unknown GameID");
	}

	_script = new ScriptInterpreter(this);

	_music = nullptr;

	_soundRate = 0;

	_saveLoadScreenOpen = false;
	_openingCreditsOpen = true;
	_tapeRecorderOpen = false;
	_previousRect = -1;
	_previousTextBox = -1;
	_voiceText = true;
	_forceVoiceText = false;
	_forceQueueText = false;

#ifdef USE_TTS
	_rtzSaveLoadIndex = ARRAYSIZE(_rtzSaveLoadButtonText);
	_rtzFirstSaveSlot = 0;
	_tapeRecorderIndex = 0;
	_playOMaticButtonIndex = ARRAYSIZE(_playOMaticButtonText);
#endif

	// Set default sound frequency
	switch (getGameID()) {
	case GID_RODNEY:
		_soundRate = 11025;
		break;
	case GID_MANHOLE:
		_soundRate = 11025;
		break;
	case GID_LGOP2:
		_soundRate = 8000;
		break;
	case GID_RTZ:
		// Return to Zork sets it itself via a script function
		break;
	default:
		break;
	}
}

MadeEngine::~MadeEngine() {
	_system->getAudioCDManager()->stop();

	delete _rnd;
	delete _pmvPlayer;
	delete _res;
	delete _screen;
	delete _dat;
	delete _script;
	delete _music;
}

void MadeEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	if (_music)
		_music->syncSoundSettings();
}

int16 MadeEngine::getTicks() {
	return g_system->getMillis() * 30 / 1000;
}

int16 MadeEngine::getTimer(int16 timerNum) {
	if (timerNum > 0 && timerNum <= ARRAYSIZE(_timers) && _timers[timerNum - 1] != -1) {
		Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
		if (getGameID() == GID_LGOP2 && ttsMan && ttsMan->isSpeaking()) {
			return 1;
		}

		return (getTicks() - _timers[timerNum - 1]);
	} else
		return 32000;
}

void MadeEngine::setTimer(int16 timerNum, int16 value) {
	if (timerNum > 0 && timerNum <= ARRAYSIZE(_timers))
		_timers[timerNum - 1] = value;
}

void MadeEngine::resetTimer(int16 timerNum) {
	if (timerNum > 0 && timerNum <= ARRAYSIZE(_timers))
		_timers[timerNum - 1] = getTicks();
}

int16 MadeEngine::allocTimer() {
	for (int i = 0; i < ARRAYSIZE(_timers); i++) {
		if (_timers[i] == -1) {
			_timers[i] = getTicks();
			return i + 1;
		}
	}
	return 0;
}

void MadeEngine::freeTimer(int16 timerNum) {
	if (timerNum > 0 && timerNum <= ARRAYSIZE(_timers))
		_timers[timerNum - 1] = -1;
}

void MadeEngine::resetAllTimers() {
	for (int i = 0; i < ARRAYSIZE(_timers); i++)
		_timers[i] = -1;
}

void MadeEngine::sayText(const Common::String &text, Common::TextToSpeechManager::Action action) const {
	if (text.empty()) {
		return;
	}

	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (ttsMan != nullptr && ConfMan.getBool("tts_enabled")) {
		ttsMan->say(text, action, _ttsTextEncoding);
	}
}

void MadeEngine::stopTextToSpeech() const {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (ttsMan != nullptr && ConfMan.getBool("tts_enabled") && ttsMan->isSpeaking()) {
		ttsMan->stop();
	}
}

#ifdef USE_TTS

void MadeEngine::checkHoveringSaveLoadScreen() {
	static const Common::Rect rtzSaveLoadScreenButtons[] = {
		Common::Rect(184, 174, 241, 189),	// Cancel button
		Common::Rect(109, 174, 166, 189),	// Save/load button
		Common::Rect(25, 20, 297, 158)		// Text entry box
	};

	static const uint8 kRtzSaveLoadButtonCount = ARRAYSIZE(rtzSaveLoadScreenButtons);
	static const uint8 kRtzSaveBoxHeight = 14;

	enum RtzSaveLoadScreenIndex {
		kCancel = 0,
		kSaveOrLoad = 1,
		kTextBox = 2
	};

	if (_saveLoadScreenOpen && getGameID() == GID_RTZ) {
		bool hoveringOverButton = false;
		for (uint8 i = 0; i < kRtzSaveLoadButtonCount; ++i) {
			if (rtzSaveLoadScreenButtons[i].contains(_eventMouseX, _eventMouseY)) {
				if (_previousRect != i) {
					if (i == kTextBox) {
						int index = MIN((_eventMouseY - 20) / kRtzSaveBoxHeight, 9);

						if (index != _previousTextBox) {
							sayText(Common::String::format("%d", _rtzFirstSaveSlot + index));
							_previousTextBox = index;
						}
					} else {
						sayText(_rtzSaveLoadButtonText[i]);
						_previousRect = i;
					}
				}

				hoveringOverButton = true;
				break;
			}
		}

		if (!hoveringOverButton) {
			_previousRect = -1;
			_previousTextBox = -1;
		}
	}
}

void MadeEngine::checkHoveringPlayOMatic(int16 spriteY) {
	static const Common::Rect lgop2PlayOMaticButtons[] = {
		Common::Rect(105, 102, 225, 122),
		Common::Rect(105, 127, 225, 147),
		Common::Rect(105, 152, 225, 172),
		Common::Rect(105, 177, 225, 197)
	};

	static const uint8 kLgop2PlayOMaticButtonCount = ARRAYSIZE(lgop2PlayOMaticButtons);

	if (_saveLoadScreenOpen && getGameID() == GID_LGOP2) {
		bool hoveringOverButton = false;
		for (uint8 i = 0; i < kLgop2PlayOMaticButtonCount; ++i) {
			if (lgop2PlayOMaticButtons[i].contains(_eventMouseX, _eventMouseY) || spriteY == lgop2PlayOMaticButtons[i].top) {
				if (_previousRect != i || spriteY != -1) {
					sayText(_playOMaticButtonText[i], Common::TextToSpeechManager::INTERRUPT);
					_previousRect = i;
				}

				hoveringOverButton = true;
				break;
			}
		}

		if (!hoveringOverButton) {
			_previousRect = -1;
		}
	}
}

#endif

Common::String MadeEngine::getSavegameFilename(int16 saveNum) {
	return Common::String::format("%s.%03d", getTargetName().c_str(), saveNum);
}

void MadeEngine::handleEvents() {

	Common::Event event;
	Common::EventManager *eventMan = _system->getEventManager();

	// NOTE: Don't reset _eventNum to 0 here or no events will get through to the scripts.

	while (eventMan->pollEvent(event)) {
		switch (event.type) {

		case Common::EVENT_MOUSEMOVE:
			_eventMouseX = event.mouse.x;
			_eventMouseY = event.mouse.y;

#ifdef USE_TTS
			checkHoveringSaveLoadScreen();
			checkHoveringPlayOMatic();
#endif

			break;

		case Common::EVENT_LBUTTONDOWN:
			_eventNum = 2;

			if (_openingCreditsOpen) {
				_openingCreditsOpen = false;
				stopTextToSpeech();
			}

			break;

		case Common::EVENT_LBUTTONUP:
			_eventNum = 1;
			break;

		case Common::EVENT_RBUTTONDOWN:
			_eventNum = 4;
			break;

		case Common::EVENT_RBUTTONUP:
			_eventNum = 3;
			break;
		case Common::EVENT_KEYDOWN:
			// Handle any special keys here
			// Supported keys taken from https://web.archive.org/web/20141114142447/http://www.allgame.com/game.php?id=13542&tab=controls
			if (event.kbd.keycode == Common::KEYCODE_BACKSPACE) {
				_eventNum = 5;
				_eventKey = 9;
			} else {
				_eventNum = 5;
				_eventKey = event.kbd.ascii;
			}
			break;
		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			switch (event.customType) {
			case kActionCursorUp:
				_eventMouseY = MAX<int16>(0, _eventMouseY - 1);
				g_system->warpMouse(_eventMouseX, _eventMouseY);
				break;
			case kActionCursorDown:
				_eventMouseY = MIN<int16>(199, _eventMouseY + 1);
				g_system->warpMouse(_eventMouseX, _eventMouseY);
				break;
			case kActionCursorLeft:
				_eventMouseX = MAX<int16>(0, _eventMouseX - 1);
				g_system->warpMouse(_eventMouseX, _eventMouseY);
				break;
			case kActionCursorRight:
				_eventMouseX = MIN<int16>(319, _eventMouseX + 1);
				g_system->warpMouse(_eventMouseX, _eventMouseY);
				break;
			case kActionMenu:
				_eventNum = 5;
				_eventKey = 21; //KEYCODE F1
				break;
			case kActionSaveGame:
				_eventNum = 5;
				_eventKey = 22; //KEYCODE F2
				break;
			case kActionLoadGame:
				_eventNum = 5;
				_eventKey = 23; //KEYCODE F3
				break;
			case kActionRepeatMessage:
				_eventNum = 5;
				_eventKey = 24; //KEYCODE F4
				break;
			default:
				break;
			}
			break;
		default:
			break;

		}
	}

	_system->getAudioCDManager()->update();

}

Common::Error MadeEngine::run() {
	if (getPlatform() == Common::kPlatformMacintosh)
		_music = nullptr; // TODO: Macintosh music player
	else
		_music = new DOSMusicPlayer(this, getGameID() == GID_RTZ);
	syncSoundSettings();

	// Initialize backend
	initGraphics(320, 200);

	resetAllTimers();

	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (ttsMan != nullptr) {
		ttsMan->enable(ConfMan.getBool("tts_enabled"));

		if (getLanguage() == Common::KO_KOR) {	// Korean version doesn't translate any text
			ttsMan->setLanguage("en");
		} else {
			ttsMan->setLanguage(ConfMan.get("language"));
		}

		if (getLanguage() == Common::JA_JPN) {
			_ttsTextEncoding = Common::CodePage::kWindows932;
		} else {
			_ttsTextEncoding = Common::CodePage::kDos850;
		}
	}

	if (getGameID() == GID_RTZ) {
		if (getFeatures() & GF_DEMO) {
			_dat->open("demo.dat");
			_res->open("demo.prj");
		} else if (getFeatures() & GF_CD) {
			_dat->open("rtzcd.dat");
			_res->open("rtzcd.prj");
		} else if (getFeatures() & GF_CD_COMPRESSED) {
			_dat->openFromRed("rtzcd.red", "rtzcd.dat");
			_res->open("rtzcd.prj");
		} else if (getFeatures() & GF_FLOPPY) {
			_dat->open("rtz.dat");
			_res->open("rtz.prj");
		} else {
			error("Unknown RTZ game features");
		}
	} else if (getGameID() == GID_MANHOLE) {
		_dat->open("manhole.dat");

		if (getVersion() == 2) {
			_res->open("manhole.prj");
		} else {
			_res->openResourceBlocks();
		}
	} else if (getGameID() == GID_LGOP2) {
		_dat->open("lgop2.dat");
		_res->open("lgop2.prj");
	} else if (getGameID() == GID_RODNEY) {
		_dat->open("rodneys.dat");
		_res->open("rodneys.prj");

		if (ConfMan.hasKey("windows_cursors") && ConfMan.getBool("windows_cursors")) {
			// Try to open the EXE and get the hand cursor out
			Common::WinResources *exe = Common::WinResources::createFromEXE("rodneysw.exe"); // Win16 executable
			if (!exe)
				exe = Common::WinResources::createFromEXE("rodneysv.exe"); // Tandy VIS executable

			if (exe) {
				Graphics::WinCursorGroup *_winCursor = Graphics::WinCursorGroup::createCursorGroup(exe, Common::WinResourceID("HANDCURSOR"));
				if (_winCursor) {
					if (_winCursor->cursors.size() > 0) {
						_screen->setMouseCursor(_winCursor->cursors[0].cursor);
						_useWinCursors = true;
					}
					delete _winCursor;
				}

				delete exe;
			}
		}
	} else {
		error ("Unknown MADE game");
	}

	if ((getFeatures() & GF_CD) || (getFeatures() & GF_CD_COMPRESSED)) {
		if (!existExtractedCDAudioFiles()
		    && !isDataAndCDAudioReadFromSameCD()) {
			warnMissingExtractedCDAudio();
		}
	}

	_autoStopSound = false;
	_eventNum = _eventKey = _eventMouseX = _eventMouseY = 0;

#ifdef DUMP_SCRIPTS
	_script->dumpAllScripts();
#else
	if (! _useWinCursors)
		_screen->setDefaultMouseCursor();

	_script->runScript(_dat->getMainCodeObjectIndex());
#endif

	if (_music)
		_music->close();

	return Common::kNoError;
}

void MadeEngine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);

	if (pause) {
		if (_music)
			_music->pause();
	} else {
		if (_music)
			_music->resume();
	}
}

} // End of namespace Made
