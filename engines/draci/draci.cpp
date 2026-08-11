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

#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/file.h"
#include "common/keyboard.h"
#include "common/text-to-speech.h"

#include "engines/util.h"
#include "engines/advancedDetector.h"

#include "graphics/cursorman.h"
#include "graphics/font.h"

#include "draci/draci.h"
#include "draci/animation.h"
#include "draci/barchive.h"
#include "draci/font.h"
#include "draci/game.h"
#include "draci/mouse.h"
#include "draci/music.h"
#include "draci/saveload.h"
#include "draci/screen.h"
#include "draci/script.h"
#include "draci/sound.h"
#include "draci/sprite.h"

namespace Draci {

#ifdef USE_TTS

// Used by all languages but Polish
static const uint16 kamenickyEncodingTable[] = {
	0xc48c, 0xc3bc, 0xc3a9, 0xc48f,	// Č, ü, é, ď
	0xc3a4, 0xc48e, 0xc5a4, 0xc48d,	// ä, Ď, Ť, č
	0xc49b, 0xc494, 0xc4b9, 0xc38d,	// ě, Ě, Ĺ, Í
	0xc4be, 0xc4ba, 0xc384, 0xc381,	// ľ, ĺ, Ä, Á
	0xc389, 0xc5be, 0xc5bd, 0xc3b4,	// É, ž, Ž, ô
	0xc3b6, 0xc393, 0xc5af, 0xc39a,	// ö, Ó, ů, Ú
	0xc3bd, 0xc396, 0xc39c, 0xc5a0,	// ý, Ö, Ü, Š
	0xc4bd, 0xc39d, 0xc598, 0xc5a5,	// Ľ, Ý, Ř, ť
	0xc3a1, 0xc3ad, 0xc3b3, 0xc3ba,	// á, í, ó, ú
	0xc588, 0xc587, 0xc5ae, 0xc394,	// ň, Ň, Ů, Ô
	0xc5a1, 0xc599, 0xc595, 0xc594	// š, ř, ŕ, Ŕ
};

// Name of encoding unknown (described by the Draci website as "some ridiculous proprietary encoding")
// After 0x9b (0xc5bb/Ż), it matches Kamenický encoding (though some of these Czech characters are replaced
// for TTS because it struggles to pronounce them)
static const uint16 polishEncodingTable[] = {
	0xc485, 0xc487, 0xc499, 0xc582,	// ą, ć, ę, ł
	0xc584, 0xc3b3, 0xc59b, 0xc5ba,	// ń, ó, ś, ź
	0xc5bc, 0xc484, 0xc486, 0xc498, // ż, Ą, Ć, Ę
	0xc581, 0xc583, 0xc393, 0xc59a,	// Ł, Ń, Ó, Ś
	0xc5b9, 0xc5bb, 0x5a, 0x6f,		// Ź, Ż, Z, o
	0xc3b6, 0xc393, 0xc5af, 0xc39a,	// ö, Ó, ů, Ú
	0xc3bd, 0xc396, 0xc39c, 0x53,	// ý, Ö, Ü, S
	0xc4bd, 0xc39d, 0x52, 0x74,		// Ľ, Ý, R, t
	0xc3a1, 0xc3ad, 0xc3b3, 0xc3ba,	// á, í, ó, ú
	0x6e, 0x4e, 0xc5ae, 0xc394,		// n, N, Ů, Ô
	0x73, 0x72, 0x72, 0x52			// s, r, r, R
};

// TTS for all languages but Czech struggles to voice a lot of Czech characters in the credits, 
// and oftentimes skips them entirely (i.e. "Špalek" is pronounced as "Palek")
// To more closely resemble how the names are supposed to be pronounced,
// this table replaces certain Czech characters with an alternative
static const uint16 czechCharacterConversionTable[] = {
	0x43, 0xc3bc, 0xc3a9, 0x64,		// C, ü, é, d
	0xc3a4, 0x44, 0x54, 0x63,		// ä, D, T, c
	0x65, 0x45, 0x4c, 0xc38d,		// e, E, L, Í
	0x6c, 0xc4ba, 0xc384, 0xc381,	// l, ĺ, Ä, Á
	0xc389, 0x7a, 0x5a, 0x6f,		// É, z, Z, o
	0xc3b6, 0xc393, 0x75, 0xc39a,	// ö, Ó, u, Ú
	0x0079, 0xc396, 0xc39c, 0x53,	// y, Ö, Ü, S
	0x4c, 0x59, 0x52, 0x74,			// L, Y, R, t
	0xc3a1, 0xc3ad, 0xc3b3, 0xc3ba,	// á, í, ó, ú
	0x6e, 0x4e, 0x55, 0x4f,			// n, N, U, O
	0x73, 0x72, 0x72, 0x52			// s, r, r, R
};

#endif

// Data file paths

const char *objectsPath = "OBJEKTY.DFW";
const char *palettePath = "PALETY.DFW";
const char *spritesPath = "OBR_AN.DFW";
const char *overlaysPath = "OBR_MAS.DFW";
const char *roomsPath = "MIST.DFW";
const char *animationsPath = "ANIM.DFW";
const char *iconsPath = "HRA.DFW";
const char *walkingMapsPath = "MAPY.DFW";
const char *itemsPath = "IKONY.DFW";
const char *itemImagesPath = "OBR_IK.DFW";
const char *initPath = "INIT.DFW";
const char *stringsPath = "RETEZCE.DFW";
const char *soundsPath = "CD2.SAM";
const char *dubbingPath = "CD.SAM";
const char *musicPathMask = "HUDBA%d.MID";

const uint kSoundsFrequency = 13000;
const uint kDubbingFrequency = 22050;

DraciEngine::DraciEngine(OSystem *syst, const ADGameDescription *gameDesc)
 : Engine(syst), _gameDescription(gameDesc), _rnd("draci") {

	setDebugger(new DraciConsole(this));

	_screen = nullptr;
	_mouse = nullptr;
	_game = nullptr;
	_script = nullptr;
	_anims = nullptr;
	_sound = nullptr;
	_music = nullptr;
	_smallFont = nullptr;
	_bigFont = nullptr;
	_iconsArchive = nullptr;
	_objectsArchive = nullptr;
	_spritesArchive = nullptr;
	_paletteArchive = nullptr;
	_roomsArchive = nullptr;
	_overlaysArchive = nullptr;
	_animationsArchive = nullptr;
	_walkingMapsArchive = nullptr;
	_itemsArchive = nullptr;
	_itemImagesArchive = nullptr;
	_initArchive = nullptr;
	_stringsArchive = nullptr;
	_soundsArchive = nullptr;
	_dubbingArchive = nullptr;
	_showWalkingMap = 0;
	_pauseStartTime = 0;
}

bool DraciEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsSubtitleOptions) ||
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

static SoundArchive* openAnyPossibleDubbing() {
	debugC(1, kDraciGeneralDebugLevel, "Trying to find original dubbing");
	LegacySoundArchive *legacy = new LegacySoundArchive(dubbingPath, kDubbingFrequency);
	if (legacy->isOpen() && legacy->size()) {
		debugC(1, kDraciGeneralDebugLevel, "Found original dubbing");
		return legacy;
	}
	delete legacy;

	// The original uncompressed dubbing cannot be found.  Try to open the
	// newer compressed version.
	debugC(1, kDraciGeneralDebugLevel, "Trying to find compressed dubbing");
	ZipSoundArchive *zip = new ZipSoundArchive();

	zip->openArchive("dub-raw.zzz", "buf", RAW80, kDubbingFrequency);
	if (zip->isOpen() && zip->size()) return zip;
#ifdef USE_FLAC
	zip->openArchive("dub-flac.zzz", "flac", FLAC);
	if (zip->isOpen() && zip->size()) return zip;
#endif
#ifdef USE_VORBIS
	zip->openArchive("dub-ogg.zzz", "ogg", OGG);
	if (zip->isOpen() && zip->size()) return zip;
#endif
#ifdef USE_MAD
	zip->openArchive("dub-mp3.zzz", "mp3", MP3);
	if (zip->isOpen() && zip->size()) return zip;
#endif

	// Return an empty (but initialized) archive anyway.
	return zip;
}

int DraciEngine::init() {
	// Initialize graphics using following:
	initGraphics(kScreenWidth, kScreenHeight);

	// Open game's archives
	_initArchive = new BArchive(initPath);
	_objectsArchive = new BArchive(objectsPath);
	_spritesArchive = new BArchive(spritesPath);
	_paletteArchive = new BArchive(palettePath);
	_roomsArchive = new BArchive(roomsPath);
	_overlaysArchive = new BArchive(overlaysPath);
	_animationsArchive = new BArchive(animationsPath);
	_iconsArchive = new BArchive(iconsPath);
	_walkingMapsArchive = new BArchive(walkingMapsPath);
	_itemsArchive = new BArchive(itemsPath);
	_itemImagesArchive = new BArchive(itemImagesPath);
	_stringsArchive = new BArchive(stringsPath);

	_soundsArchive = new LegacySoundArchive(soundsPath, kSoundsFrequency);
	_dubbingArchive = openAnyPossibleDubbing();
	_sound = new Sound(_mixer);

	_music = new MusicPlayer(musicPathMask);

	// Setup mixer
	syncSoundSettings();

	// Load the game's fonts
	_smallFont = new Font(kFontSmall);
	_bigFont = new Font(kFontBig);

	_screen = new Screen(this);
	_anims = new AnimationManager(this);
	_mouse = new Mouse(this);
	_script = new Script(this);
	_game = new Game(this);

	if (!_objectsArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening objects archive failed");
		return Common::kUnknownError;
	}

	if (!_spritesArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening sprites archive failed");
		return Common::kUnknownError;
	}

	if (!_paletteArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening palette archive failed");
		return Common::kUnknownError;
	}

	if (!_roomsArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening rooms archive failed");
		return Common::kUnknownError;
	}

	if (!_overlaysArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening overlays archive failed");
		return Common::kUnknownError;
	}

	if (!_animationsArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening animations archive failed");
		return Common::kUnknownError;
	}

	if (!_iconsArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening icons archive failed");
		return Common::kUnknownError;
	}

	if (!_walkingMapsArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening walking maps archive failed");
		return Common::kUnknownError;
	}

	if (!_soundsArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening sounds archive failed");
		return Common::kUnknownError;
	}

	if (!_dubbingArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "WARNING - Opening dubbing archive failed");
	}

	_showWalkingMap = false;

	// Basic archive test
	debugC(2, kDraciGeneralDebugLevel, "Running archive tests...");
	Common::Path path("INIT.DFW");
	BArchive ar(path);
	const BAFile *f;
	debugC(3, kDraciGeneralDebugLevel, "Number of file streams in archive: %d", ar.size());

	if (ar.isOpen()) {
		f = ar.getFile(0);
	} else {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Archive not opened");
		return Common::kUnknownError;
	}

	debugC(3, kDraciGeneralDebugLevel, "First 10 bytes of file %d: ", 0);
	for (uint i = 0; i < 10; ++i) {
		debugC(3, kDraciGeneralDebugLevel, "0x%02x%c", f->_data[i], (i < 9) ? ' ' : '\n');
	}

	return Common::kNoError;
}

void DraciEngine::handleEvents() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_RIGHT:
				if (gDebugLevel >= 0) {
					_game->scheduleEnteringRoomUsingGate(_game->nextRoomNum(), 0);
				}
				break;
			case Common::KEYCODE_LEFT:
				if (gDebugLevel >= 0) {
					_game->scheduleEnteringRoomUsingGate(_game->prevRoomNum(), 0);
				}
				break;
			default:
				break;
			}
			break;
		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			switch (event.customType) {
			case kActionEscape: {
				if (_game->getLoopStatus() == kStatusInventory &&
				   _game->getLoopSubstatus() == kOuterLoop) {
					_game->inventoryDone();
					break;
				}

				const int escRoom = _game->getRoomNum() != _game->getMapRoom()
					? _game->getEscRoom() : _game->getPreviousRoomNum();

				// Check if there is an escape room defined for the current room
				if (escRoom >= 0) {

					// Schedule room change
					// TODO: gate 0 (always present) is not always best for
					// returning from the map, e.g. in the starting location.
					// also, after loading the game, we shouldn't run any gate
					// program, but rather restore the state of all objects.
					_game->scheduleEnteringRoomUsingGate(escRoom, 0);

					// Immediately cancel any running animation or dubbing and
					// end any currently running GPL programs.  In the intro it
					// works as intended---skipping the rest of it.
					//
					// In the map, this causes that animation on newly
					// discovered locations will be re-run next time and
					// cut-scenes won't be played.
					_game->setExitLoop(true);
					_script->endCurrentProgram(true);

					stopTextToSpeech();
				}
				break;
			}
			case kActionMap:
				if (_game->getLoopStatus() == kStatusOrdinary) {
					const int new_room = _game->getRoomNum() != _game->getMapRoom()
						? _game->getMapRoom() : _game->getPreviousRoomNum();
					_game->scheduleEnteringRoomUsingGate(new_room, 0);
				}
				break;
			case kActionShowWalkMap:
				// Show walking map toggle
				_showWalkingMap = !_showWalkingMap;
				_game->switchWalkingAnimations(_showWalkingMap);
				break;
			case kActionToggleWalkSpeed:
				_game->setWantQuickHero(!_game->getWantQuickHero());
				break;
			case kActionInventory:
				if (_game->getRoomNum() == _game->getMapRoom() ||
				    _game->getLoopSubstatus() != kOuterLoop) {
					break;
				}
				if (_game->getLoopStatus() == kStatusInventory) {
					_game->inventoryDone();
				} else if (_game->getLoopStatus() == kStatusOrdinary) {
					_game->inventoryInit();
				}
				break;
			case kActionOpenMainMenu:
				if (event.kbd.hasFlags(0)) {
					openMainMenuDialog();
				}
				break;
			case kActionTogglePointerItem:
			case kActionInvRotatePrevious:
			case kActionInvRotateNext:
				if ((_game->getLoopStatus() == kStatusOrdinary ||
				    _game->getLoopStatus() == kStatusInventory) &&
				   _game->getLoopSubstatus() == kOuterLoop &&
				   _game->getRoomNum() != _game->getMapRoom()) {
					_game->inventorySwitch(event.customType);
				}
				break;
			default:
				break;
			}
			break;
		default:
			_mouse->handleEvent(event);
		}
	}

	// Handle EVENT_QUIT and EVENT_RETURN_TO_LAUNCHER.
	if (shouldQuit()) {
		_game->setQuit(true);
		_script->endCurrentProgram(true);
	}
}

DraciEngine::~DraciEngine() {
	// Dispose your resources here

	// If the common library supported Boost's scoped_ptr<>, then wrapping
	// all the following pointers and many more would be appropriate.  So
	// far, there is only SharedPtr, which I feel being an overkill for
	// easy deallocation.
	// TODO: We have ScopedPtr nowadays. Maybe should adapt this code then?
	delete _smallFont;
	delete _bigFont;

	delete _mouse;
	delete _script;
	delete _anims;
	delete _game;
	delete _screen;

	delete _initArchive;
	delete _paletteArchive;
	delete _objectsArchive;
	delete _spritesArchive;
	delete _roomsArchive;
	delete _overlaysArchive;
	delete _animationsArchive;
	delete _iconsArchive;
	delete _walkingMapsArchive;
	delete _itemsArchive;
	delete _itemImagesArchive;
	delete _stringsArchive;

	delete _sound;
	delete _music;
	delete _soundsArchive;
	delete _dubbingArchive;
}

Common::Error DraciEngine::run() {
	init();
	setTotalPlayTime(0);
	_game->init();

	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (ttsMan != nullptr) {
		ttsMan->enable(ConfMan.getBool("tts_enabled_objects") || ConfMan.getBool("tts_enabled_speech") || ConfMan.getBool("tts_enabled_missing_voice"));
		ttsMan->setLanguage(ConfMan.get("language"));
	}

	// Load game from specified slot, if any
	if (ConfMan.hasKey("save_slot")) {
		loadGameState(ConfMan.getInt("save_slot"));
	}

	_game->start();
	return Common::kNoError;
}

void DraciEngine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);
	if (pause) {
		_pauseStartTime = _system->getMillis();

		_anims->pauseAnimations();
		_sound->pauseSound();
		_sound->pauseVoice();
		_music->pause();
	} else {
		_anims->unpauseAnimations();
		_sound->resumeSound();
		_sound->resumeVoice();
		_music->resume();

		// Adjust engine start time
		const int delta = _system->getMillis() - _pauseStartTime;
		_game->shiftSpeechAndFadeTick(delta);
		_pauseStartTime = 0;
	}
}

void DraciEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	_sound->setVolume();
	_music->syncVolume();
}

Common::String DraciEngine::getSavegameFile(int saveGameIdx) {
	return Common::String::format("draci.s%02d", saveGameIdx);
}

Common::Error DraciEngine::loadGameState(int slot) {
	// When called from run() using save_slot, the next operation is the
	// call to start() calling enterNewRoom().
	// When called from handleEvents() in the middle of the game, the next
	// operation after handleEvents() exits from loop(), and returns to
	// start() to the same place as above.
	// In both cases, we are safe to override the data structures right
	// here are now, without waiting for any other code to finish, thanks
	// to our constraint in canLoadGameStateCurrently() and to having
	// enterNewRoom() called right after we exit from here.
	return loadSavegameData(slot, this);
}

bool DraciEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	return (_game->getLoopStatus() == kStatusOrdinary) &&
		(_game->getLoopSubstatus() == kOuterLoop);
}

Common::Error DraciEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	return saveSavegameData(slot, desc, *this);
}

bool DraciEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	return (_game->getLoopStatus() == kStatusOrdinary) &&
		(_game->getLoopSubstatus() == kOuterLoop);
}

void DraciEngine::sayText(const Common::String &text, bool isSubtitle) {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	// _previousSaid is used to prevent the TTS from looping when sayText is called inside a loop,
	// for example when the cursor stays on a dialog option. Without it when the text ends it would speak
	// the same text again.
	// _previousSaid is cleared when appropriate to allow for repeat requests
	bool speak = (!isSubtitle && ConfMan.getBool("tts_enabled_objects") && _previousSaid != text) || 
				 (isSubtitle && (ConfMan.getBool("tts_enabled_speech") || ConfMan.getBool("tts_enabled_missing_voice")));
	if (ttsMan != nullptr && speak) {
#ifdef USE_TTS
		ttsMan->say(convertText(text), Common::TextToSpeechManager::INTERRUPT);
#endif
		_previousSaid = text;
	}
}

void DraciEngine::stopTextToSpeech() {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (ttsMan != nullptr && (ConfMan.getBool("tts_enabled_objects") || ConfMan.getBool("tts_enabled_speech") || ConfMan.getBool("tts_enabled_missing_voice")) && 
			ttsMan->isSpeaking()) {
		ttsMan->stop();
		_previousSaid.clear();
		setTTSVoice(kBertID);
	}
}

void DraciEngine::setTTSVoice(int characterID) const {
#ifdef USE_TTS
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (ttsMan != nullptr && (ConfMan.getBool("tts_enabled_objects") || ConfMan.getBool("tts_enabled_speech") || ConfMan.getBool("tts_enabled_missing_voice"))) {
		Common::Array<int> voices;
		int pitch = 0;
		Common::TTSVoice::Gender gender;

		if (characterDialogData[characterID].male) {
			voices = ttsMan->getVoiceIndicesByGender(Common::TTSVoice::MALE);
			gender = Common::TTSVoice::MALE;
		} else {
			voices = ttsMan->getVoiceIndicesByGender(Common::TTSVoice::FEMALE);
			gender = Common::TTSVoice::FEMALE;
		}

		// If no voice is available for the necessary gender, set the voice to default
		if (voices.empty()) {
			ttsMan->setVoice(0);
		} else {
			int voiceIndex = characterDialogData[characterID].voiceID % voices.size();
			ttsMan->setVoice(voices[voiceIndex]);
		}

		// If no voices are available for this gender, alter the pitch to mimic a voice
		// of the other gender
		if (ttsMan->getVoice().getGender() != gender) {
			if (gender == Common::TTSVoice::MALE) {
				pitch -= 50;
			} else {
				pitch += 50;
			}
		}

		ttsMan->setPitch(pitch);
	}
#endif
}

#ifdef USE_TTS

Common::U32String DraciEngine::convertText(const Common::String &text) const {
	const uint16 *translationTable;

	if (getLanguage() == Common::PL_POL) {
		translationTable = polishEncodingTable;
	} else {
		translationTable = kamenickyEncodingTable;
	}

	const byte *bytes = (const byte *)text.c_str();
	byte *convertedBytes = new byte[text.size() * 2 + 1];

	int i = 0;
	for (const byte *b = bytes; *b; ++b) {
		if (*b == 0x7c) {	// Convert | to a space
			convertedBytes[i] = 0x20;
			i++;
			continue;
		}

		if (*b < 0x80 || *b > 0xab) {
			convertedBytes[i] = *b;
			i++;
			continue;
		}

		bool inTable = false;
		for (int j = 0; translationTable[j]; ++j) {
			if (*b - 0x80 == j) {
				int convertedValue = translationTable[j];

				if (translationTable[j] == 0xc3b4 && getLanguage() == Common::DE_DEU) {
					// German encoding replaces ô with ß
					convertedValue = 0xc39f;
				} else if ((getLanguage() == Common::EN_ANY || getLanguage() == Common::DE_DEU) && 
								translationTable[j] != czechCharacterConversionTable[j]) {
					// Replace certain Czech characters for English and German TTS with close alternatives
					// in those languages, for better TTS
					convertedValue = czechCharacterConversionTable[j];
				}

				if (convertedValue <= 0xff) {
					convertedBytes[i] = convertedValue;
					i++;
				} else {
					convertedBytes[i] = (convertedValue >> 8) & 0xff;
					convertedBytes[i + 1] = convertedValue & 0xff;
					i += 2;
				}
				
				inTable = true;
				break;
			}
		}

		if (!inTable) {
			convertedBytes[i] = *b;
			i++;
		}
	}
	
	convertedBytes[i] = 0;

	Common::U32String result((char *)convertedBytes);
	delete[] convertedBytes;

	return result;
}

#endif


} // End of namespace Draci
