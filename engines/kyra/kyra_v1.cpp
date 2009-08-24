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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/config-manager.h"
#include "common/EventRecorder.h"

#include "sound/mididrv.h"
#include "sound/mixer.h"

#include "kyra/kyra_v1.h"
#include "kyra/sound_intern.h"
#include "kyra/resource.h"
#include "kyra/screen.h"
#include "kyra/text.h"
#include "kyra/timer.h"
#include "kyra/script.h"
#include "kyra/debugger.h"

namespace Kyra {

KyraEngine_v1::KyraEngine_v1(OSystem *system, const GameFlags &flags)
	: Engine(system), _flags(flags) {
	_res = 0;
	_sound = 0;
	_text = 0;
	_staticres = 0;
	_timer = 0;
	_emc = 0;
	_debugger = 0;

	if (_flags.platform == Common::kPlatformAmiga)
		_gameSpeed = 50;
	else
		_gameSpeed = 60;
	_tickLength = (uint8)(1000.0 / _gameSpeed);

	_trackMap = 0;
	_trackMapSize = 0;
	_lastMusicCommand = -1;
	_curSfxFile = _curMusicTheme = -1;

	_gameToLoad = -1;

	_mouseState = -1;
	_deathHandler = -1;

	memset(_flagsTable, 0, sizeof(_flagsTable));

	_isSaveAllowed = false;

	_mouseX = _mouseY = 0;

	// sets up all engine specific debug levels
	Common::addDebugChannel(kDebugLevelScriptFuncs, "ScriptFuncs", "Script function debug level");
	Common::addDebugChannel(kDebugLevelScript, "Script", "Script interpreter debug level");
	Common::addDebugChannel(kDebugLevelSprites, "Sprites", "Sprite debug level");
	Common::addDebugChannel(kDebugLevelScreen, "Screen", "Screen debug level");
	Common::addDebugChannel(kDebugLevelSound, "Sound", "Sound debug level");
	Common::addDebugChannel(kDebugLevelAnimator, "Animator", "Animator debug level");
	Common::addDebugChannel(kDebugLevelMain, "Main", "Generic debug level");
	Common::addDebugChannel(kDebugLevelGUI, "GUI", "GUI debug level");
	Common::addDebugChannel(kDebugLevelSequence, "Sequence", "Sequence debug level");
	Common::addDebugChannel(kDebugLevelMovie, "Movie", "Movie debug level");
	Common::addDebugChannel(kDebugLevelTimer, "Timer", "Timer debug level");

	g_eventRec.registerRandomSource(_rnd, "kyra");
}

::GUI::Debugger *KyraEngine_v1::getDebugger() {
	return _debugger;
}

void KyraEngine_v1::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);
	_timer->pause(pause);
}

Common::Error KyraEngine_v1::init() {
	// Setup mixer
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));

	if (!_flags.useDigSound) {
		// We prefer AdLib over MIDI, since generally AdLib is better supported
		int midiDriver = MidiDriver::detectMusicDriver(MDT_PCSPK | MDT_MIDI | MDT_ADLIB);

		if (_flags.platform == Common::kPlatformFMTowns) {
			if (_flags.gameID == GI_KYRA1)
				_sound = new SoundTowns(this, _mixer);
			else
				_sound = new SoundTownsPC98_v2(this, _mixer);
		} else if (_flags.platform == Common::kPlatformPC98) {
			if (_flags.gameID == GI_KYRA1)
				_sound = new SoundPC98(this, _mixer);
			else
				_sound = new SoundTownsPC98_v2(this, _mixer);
		} else if (_flags.platform == Common::kPlatformAmiga) {
			_sound = new SoundAmiga(this, _mixer);
		} else if (midiDriver == MD_ADLIB) {
			_sound = new SoundAdlibPC(this, _mixer);
		} else {
			Sound::kType type;

			if (midiDriver == MD_PCSPK)
				type = Sound::kPCSpkr;
			else if (midiDriver == MD_MT32 || ConfMan.getBool("native_mt32"))
				type = Sound::kMidiMT32;
			else
				type = Sound::kMidiGM;

			MidiDriver *driver = 0;

			if (midiDriver == MD_PCSPK) {
				driver = new MidiDriver_PCSpeaker(_mixer);
			} else {
				driver = MidiDriver::createMidi(midiDriver);
				if (type == Sound::kMidiMT32)
					driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);
			}

			assert(driver);

			SoundMidiPC *soundMidiPc = new SoundMidiPC(this, _mixer, driver, type);
			_sound = soundMidiPc;
			assert(_sound);

			// Unlike some SCUMM games, it's not that the MIDI sounds are
			// missing. It's just that at least at the time of writing they
			// are decidedly inferior to the Adlib ones.
			if (ConfMan.getBool("multi_midi")) {
				SoundAdlibPC *adlib = new SoundAdlibPC(this, _mixer);
				assert(adlib);

				_sound = new MixedSoundDriver(this, _mixer, soundMidiPc, adlib);
			}
		}

		assert(_sound);
	}

	if (_sound)
		_sound->updateVolumeSettings();

	_res = new Resource(this);
	assert(_res);
	_res->reset();

	if (_flags.isDemo) {
		// HACK: check whether this is the HOF demo or the LOL demo.
		// The LOL demo needs to be detected and run as KyraEngine_HoF,
		// but the static resource loader and the sequence player will
		// need correct IDs.
		if (_res->exists("scene1.cps"))
#ifdef ENABLE_LOL
			_flags.gameID = GI_LOL;
#else
			error("Lands of Lore demo is not supported in this build.");
#endif // !ENABLE_LOL
	}

	_staticres = new StaticResource(this);
	assert(_staticres);
	if (!_staticres->init())
		error("_staticres->init() failed");
	if (!screen()->init())
		error("screen()->init() failed");
	_timer = new TimerManager(this, _system);
	assert(_timer);
	setupTimers();
	_emc = new EMCInterpreter(this);
	assert(_emc);

	setupOpcodeTable();
	readSettings();

	if (ConfMan.hasKey("save_slot")) {
		_gameToLoad = ConfMan.getInt("save_slot");
		if (!saveFileLoadable(_gameToLoad))
			_gameToLoad = -1;
	}

	// Prevent autosave on game startup
	_lastAutosave = _system->getMillis();

	return Common::kNoError;
}

KyraEngine_v1::~KyraEngine_v1() {
	for (Common::Array<const Opcode*>::iterator i = _opcodes.begin(); i != _opcodes.end(); ++i)
		delete *i;
	_opcodes.clear();

	delete _res;
	delete _staticres;
	delete _sound;
	delete _text;
	delete _timer;
	delete _emc;
	delete _debugger;
}

Common::Point KyraEngine_v1::getMousePos() const {
	Common::Point mouse = _eventMan->getMousePos();

	if (_flags.useHiResOverlay) {
		mouse.x >>= 1;
		mouse.y >>= 1;
	}

	return mouse;
}

void KyraEngine_v1::setMousePos(int x, int y) {
	if (_flags.useHiResOverlay) {
		x <<= 1;
		y <<= 1;
	}
	_system->warpMouse(x, y);
}

int KyraEngine_v1::checkInput(Button *buttonList, bool mainLoop, int eventFlag) {
	_isSaveAllowed = mainLoop;
	updateInput();
	_isSaveAllowed = false;

	if (mainLoop)
		checkAutosave();

	int keys = 0;
	int8 mouseWheel = 0;

	while (_eventList.size()) {
		Common::Event event = *_eventList.begin();
		bool breakLoop = false;

		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode >= '1' && event.kbd.keycode <= '9' &&
					(event.kbd.flags == Common::KBD_CTRL || event.kbd.flags == Common::KBD_ALT) && mainLoop) {
				int saveLoadSlot = 9 - (event.kbd.keycode - '0') + 990;

				if (event.kbd.flags == Common::KBD_CTRL) {
					loadGameStateCheck(saveLoadSlot);
					_eventList.clear();
					breakLoop = true;
				} else {
					char savegameName[14];
					sprintf(savegameName, "Quicksave %d", event.kbd.keycode - '0');
					saveGameState(saveLoadSlot, savegameName, 0);
				}
			} else if (event.kbd.flags == Common::KBD_CTRL) {
				if (event.kbd.keycode == 'd') {
					if (_debugger)
						_debugger->attach();
				} else if (event.kbd.keycode == 'q') {
					quitGame();
				}
			} else {
				switch(event.kbd.keycode) {
				case Common::KEYCODE_SPACE:
					keys = 61;
					break;
				case Common::KEYCODE_RETURN:
					keys = 43;
					break;
				case Common::KEYCODE_UP:
				case Common::KEYCODE_KP8:
					keys = 96;
					break;
				case Common::KEYCODE_RIGHT:
				case Common::KEYCODE_KP6:
					keys = 102;
					break;
				case Common::KEYCODE_DOWN:
				case Common::KEYCODE_KP2:
					keys = 97;
					break;
				case Common::KEYCODE_LEFT:
				case Common::KEYCODE_KP4:
					keys = 92;
					break;
				case Common::KEYCODE_HOME:
				case Common::KEYCODE_KP7:
					keys = 91;
					break;
				case Common::KEYCODE_PAGEUP:
				case Common::KEYCODE_KP9:
					keys = 101;
					break;
				case Common::KEYCODE_F1:
					keys = 112;
					break;
				case Common::KEYCODE_F2:
					keys = 113;
					break;
				case Common::KEYCODE_F3:
					keys = 114;
					break;
				case Common::KEYCODE_o:
					keys = 25;
					break;
				case Common::KEYCODE_r:
					keys = 20;
					break;
				case Common::KEYCODE_SLASH:
					keys = 55;
					break;
				case Common::KEYCODE_ESCAPE:
					keys = 110;
					break;
				default:
					keys = 0;
				}

				// When we got an keypress, which we might need to handle,
				// break the event loop and pass it to GUI code.
				if (keys)
					breakLoop = true;
			}
			break;

		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_LBUTTONUP: {
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			if (_flags.useHiResOverlay) {
				_mouseX >>= 1;
				_mouseY >>= 1;
			}
			keys = (event.type == Common::EVENT_LBUTTONDOWN ? 199 : (200 | 0x800));
			breakLoop = true;
			} break;

		case Common::EVENT_RBUTTONDOWN:
		case Common::EVENT_RBUTTONUP: {
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			if (_flags.useHiResOverlay) {
				_mouseX >>= 1;
				_mouseY >>= 1;
			}
			keys = (event.type == Common::EVENT_RBUTTONDOWN ? 201 : (202 | 0x800));
			breakLoop = true;
			} break;

		case Common::EVENT_WHEELUP:
			mouseWheel = -1;
			break;

		case Common::EVENT_WHEELDOWN:
			mouseWheel = 1;
			break;

		default:
			break;
		}

		if (_debugger && _debugger->isAttached())
			_debugger->onFrame();

		if (breakLoop)
			break;

		_eventList.erase(_eventList.begin());
	}

	GUI *guiInstance = gui();
	if (guiInstance) {
		if (keys)
			return guiInstance->processButtonList(buttonList, keys | eventFlag, mouseWheel);
		else
			return guiInstance->processButtonList(buttonList, 0, mouseWheel);
	} else {
		return keys;
	}
}

void KyraEngine_v1::updateInput() {
	Common::Event event;

	bool updateScreen = false;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == '.' || event.kbd.keycode == Common::KEYCODE_ESCAPE ||
				event.kbd.keycode == Common::KEYCODE_SPACE || event.kbd.keycode == Common::KEYCODE_RETURN ||
				event.kbd.keycode == Common::KEYCODE_UP || event.kbd.keycode == Common::KEYCODE_RIGHT ||
				event.kbd.keycode == Common::KEYCODE_DOWN || event.kbd.keycode == Common::KEYCODE_LEFT)
					_eventList.push_back(Event(event, true));
			else if (event.kbd.keycode == 'q' && event.kbd.flags == Common::KBD_CTRL)
				quitGame();
			else
				_eventList.push_back(event);
			break;

		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN:
			_eventList.push_back(Event(event, true));
			break;

		case Common::EVENT_MOUSEMOVE:
			if (screen()->isMouseVisible())
				updateScreen = true;
			break;

		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP:
		case Common::EVENT_WHEELUP:
		case Common::EVENT_WHEELDOWN:
			_eventList.push_back(event);
			break;

		default:
			break;
		}
	}

	if (updateScreen)
		_system->updateScreen();
}

void KyraEngine_v1::removeInputTop() {
	if (!_eventList.empty())
		_eventList.erase(_eventList.begin());
}

bool KyraEngine_v1::skipFlag() const {
	for (Common::List<Event>::const_iterator i = _eventList.begin(); i != _eventList.end(); ++i) {
		if (i->causedSkip)
			return true;
	}
	return false;
}

void KyraEngine_v1::resetSkipFlag(bool removeEvent) {
	for (Common::List<Event>::iterator i = _eventList.begin(); i != _eventList.end(); ++i) {
		if (i->causedSkip) {
			if (removeEvent)
				_eventList.erase(i);
			else
				i->causedSkip = false;
			return;
		}
	}
}


int KyraEngine_v1::setGameFlag(int flag) {
	assert((flag >> 3) >= 0 && (flag >> 3) <= ARRAYSIZE(_flagsTable));
	_flagsTable[flag >> 3] |= (1 << (flag & 7));
	return 1;
}

int KyraEngine_v1::queryGameFlag(int flag) const {
	assert((flag >> 3) >= 0 && (flag >> 3) <= ARRAYSIZE(_flagsTable));
	return ((_flagsTable[flag >> 3] >> (flag & 7)) & 1);
}

int KyraEngine_v1::resetGameFlag(int flag) {
	assert((flag >> 3) >= 0 && (flag >> 3) <= ARRAYSIZE(_flagsTable));
	_flagsTable[flag >> 3] &= ~(1 << (flag & 7));
	return 0;
}

void KyraEngine_v1::delayUntil(uint32 timestamp, bool updateTimers, bool update, bool isMainLoop) {
	const uint32 curTime = _system->getMillis();
	if (curTime > timestamp)
		return;

	uint32 del = timestamp - curTime;
	while (del && !shouldQuit()) {
		uint32 step = MIN<uint32>(del, _tickLength);
		delay(step, update, isMainLoop);
		del -= step;
	}
}

void KyraEngine_v1::delay(uint32 amount, bool update, bool isMainLoop) {
	_system->delayMillis(amount);
}

void KyraEngine_v1::delayWithTicks(int ticks) {
	delay(ticks * _tickLength);
}

void KyraEngine_v1::registerDefaultSettings() {
	if (_flags.platform == Common::kPlatformFMTowns)
		ConfMan.registerDefault("cdaudio", true);
	if (_flags.fanLang != Common::UNK_LANG) {
		// HACK/WORKAROUND: Since we can't use registerDefault here to overwrite
		// the global subtitles settings, we're using this hack to enable subtitles
		// for fan translations
		const Common::ConfigManager::Domain *cur = ConfMan.getActiveDomain();
		if (!cur || (cur && cur->get("subtitles").empty()))
			ConfMan.setBool("subtitles", true);
	}
}

void KyraEngine_v1::readSettings() {
	_configWalkspeed = ConfMan.getInt("walkspeed");
	_configMusic = 0;

	if (!ConfMan.getBool("music_mute")) {
		if (_flags.platform == Common::kPlatformFMTowns)
			_configMusic = ConfMan.getBool("cdaudio") ? 2 : 1;
		else
			_configMusic = 1;
	}
	_configSounds = ConfMan.getBool("sfx_mute") ? 0 : 1;

	if (_sound) {
		_sound->enableMusic(_configMusic);
		_sound->enableSFX(_configSounds);
	}

	bool speechMute = ConfMan.getBool("speech_mute");
	bool subtitles = ConfMan.getBool("subtitles");

	if (!speechMute && subtitles)
		_configVoice = 2;	// Voice & Text
	else if (!speechMute && !subtitles)
		_configVoice = 1;	// Voice only
	else
		_configVoice = 0;	// Text only

	setWalkspeed(_configWalkspeed);
}

void KyraEngine_v1::writeSettings() {
	bool speechMute, subtitles;

	ConfMan.setInt("walkspeed", _configWalkspeed);
	ConfMan.setBool("music_mute", _configMusic == 0);
	if (_flags.platform == Common::kPlatformFMTowns)
		ConfMan.setBool("cdaudio", _configMusic == 2);
	ConfMan.setBool("sfx_mute", _configSounds == 0);

	switch (_configVoice) {
	case 0:		// Text only
		speechMute = true;
		subtitles = true;
		break;
	case 1:		// Voice only
		speechMute = false;
		subtitles = false;
		break;
	default:	// Voice & Text
		speechMute = false;
		subtitles = true;
	}

	if (_sound) {
		if (!_configMusic)
			_sound->beginFadeOut();
		_sound->enableMusic(_configMusic);
		_sound->enableSFX(_configSounds);
	}

	ConfMan.setBool("speech_mute", speechMute);
	ConfMan.setBool("subtitles", subtitles);

	ConfMan.flushToDisk();
}

bool KyraEngine_v1::speechEnabled() {
	return _flags.isTalkie && (_configVoice == 1 || _configVoice == 2);
}

bool KyraEngine_v1::textEnabled() {
	return !_flags.isTalkie || (_configVoice == 0 || _configVoice == 2);
}

int KyraEngine_v1::convertVolumeToMixer(int value) {
	value -= 2;
	return (value * Audio::Mixer::kMaxMixerVolume) / 95;
}

int KyraEngine_v1::convertVolumeFromMixer(int value) {
	return (value * 95) / Audio::Mixer::kMaxMixerVolume + 2;
}

void KyraEngine_v1::setVolume(kVolumeEntry vol, uint8 value) {
	switch (vol) {
	case kVolumeMusic:
		ConfMan.setInt("music_volume", convertVolumeToMixer(value));
		break;

	case kVolumeSfx:
		ConfMan.setInt("sfx_volume", convertVolumeToMixer(value));
		break;

	case kVolumeSpeech:
		ConfMan.setInt("speech_volume", convertVolumeToMixer(value));
		break;
	}

	// Resetup mixer
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));
	if (_sound)
		_sound->updateVolumeSettings();
}

uint8 KyraEngine_v1::getVolume(kVolumeEntry vol) {
	switch (vol) {
	case kVolumeMusic:
		return convertVolumeFromMixer(ConfMan.getInt("music_volume"));
		break;

	case kVolumeSfx:
		return convertVolumeFromMixer(ConfMan.getInt("sfx_volume"));
		break;

	case kVolumeSpeech:
		if (speechEnabled())
			return convertVolumeFromMixer(ConfMan.getInt("speech_volume"));
		else
			return 2;
		break;
	}

	return 2;
}

void KyraEngine_v1::syncSoundSettings() {
	Engine::syncSoundSettings();

	if (_sound)
		_sound->updateVolumeSettings();
}

} // End of namespace Kyra

