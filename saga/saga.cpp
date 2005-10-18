/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */
#include "common/stdafx.h"

#include "base/gameDetector.h"
#include "base/plugins.h"
#include "backends/fs/fs.h"

#include "common/file.h"
#include "common/config-manager.h"
#include "common/system.h"

#include "sound/mixer.h"

#include "saga/saga.h"

#include "saga/rscfile.h"
#include "saga/gfx.h"
#include "saga/render.h"
#include "saga/actor.h"
#include "saga/animation.h"
#include "saga/console.h"
#include "saga/events.h"
#include "saga/font.h"
#include "saga/interface.h"
#include "saga/isomap.h"
#include "saga/puzzle.h"
#include "saga/script.h"
#include "saga/scene.h"
#include "saga/sndres.h"
#include "saga/sprite.h"
#include "saga/sound.h"
#include "saga/music.h"
#include "saga/palanim.h"
#include "saga/objectmap.h"
#include "saga/resnames.h"

static const GameSettings saga_games[] = {
	{"ite", "Inherit the Earth", 0},
	{"ihnm", "I Have No Mouth and I Must Scream", GF_DEFAULT_TO_1X_SCALER },
	{0, 0, 0}
};

GameList Engine_SAGA_gameList() {
	GameList games;
	const GameSettings *g = saga_games;

	while (g->name) {
		games.push_back(*g);
		g++;
	}

	return games;
}

DetectedGameList Engine_SAGA_detectGames(const FSList &fslist) {
	return Saga::GAME_ProbeGame(fslist);
}

Engine *Engine_SAGA_create(GameDetector *detector, OSystem *syst) {
	return new Saga::SagaEngine(detector, syst);
}

REGISTER_PLUGIN(SAGA, "SAGA Engine")

namespace Saga {

#define MAX_TIME_DELTA 100

SagaEngine::SagaEngine(GameDetector *detector, OSystem *syst)
	: Engine(syst),
	_targetName(detector->_targetName) {

	_leftMouseButtonPressed = _rightMouseButtonPressed = false;

	_console = NULL;
	_quit = false;

	_resource = NULL;
	_sndRes = NULL;
	_events = NULL;
	_font = NULL;
	_sprite = NULL;
	_anim = NULL;
	_script = NULL;
	_interface = NULL;
	_actor = NULL;
	_palanim = NULL;
	_scene = NULL;
	_isoMap = NULL;
	_gfx = NULL;
	_console = NULL;
	_render = NULL;
	_music = NULL;
	_sound = NULL;
	_puzzle = NULL;

	_frameCount = 0;
	_globalFlags = 0;
	memset(_ethicsPoints, 0, sizeof(_ethicsPoints));

	// The Linux version of Inherit the Earth puts all data files in an
	// 'itedata' sub-directory, except for voices.rsc
	Common::File::addDefaultDirectory(_gameDataPath + "itedata/");

	// The Windows version of Inherit the Earth puts various data files in
	// other subdirectories.
	Common::File::addDefaultDirectory(_gameDataPath + "graphics/");
	Common::File::addDefaultDirectory(_gameDataPath + "music/");
	Common::File::addDefaultDirectory(_gameDataPath + "sound/");

	// The Multi-OS version puts the voices file in the root directory of
	// the CD. The rest of the data files are in game/itedata
	Common::File::addDefaultDirectory(_gameDataPath + "game/itedata/");

	// Mac CD Wyrmkeep
	Common::File::addDefaultDirectory(_gameDataPath + "patch/");

	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_displayClip.left = _displayClip.top = 0;
}

SagaEngine::~SagaEngine() {
	if (_scene != NULL) {
		if (_scene->isSceneLoaded()) {
			_scene->endScene();
		}
	}

	delete _puzzle;
	delete _sndRes;
	delete _events;
	delete _font;
	delete _sprite;
	delete _anim;
	delete _script;
	delete _interface;
	delete _actor;
	delete _palanim;
	delete _scene;
	delete _isoMap;
	delete _render;
	delete _music;
	delete _sound;
	delete _gfx;
	delete _console;

	delete _resource;
}

void SagaEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

int SagaEngine::init(GameDetector &detector) {
	_soundVolume = ConfMan.getInt("sfx_volume") / 25;
	_musicVolume = ConfMan.getInt("music_volume") / 25;
	_subtitlesEnabled = ConfMan.getBool("subtitles");
	_readingSpeed = ConfMan.getInt("talkspeed");
	_copyProtection = ConfMan.getBool("copy_protection");

	if (_readingSpeed > 3)
		_readingSpeed = 0;

	_resource = new Resource(this);

	// Add some default directories
	// Win32 demo & full game
	Common::File::addDefaultDirectory("graphics");
	Common::File::addDefaultDirectory("music");
	Common::File::addDefaultDirectory("sound");

	// Linux demo
	Common::File::addDefaultDirectory("itedata");

	// Mac demos & full game
	Common::File::addDefaultDirectory("patch");

	// Process command line

	// Detect game and open resource files
	if (!initGame()) {
		return FAILURE;
	}

	// Initialize engine modules
	_sndRes = new SndRes(this);
	_events = new Events(this);
	_font = new Font(this);
	_sprite = new Sprite(this);
	_anim = new Anim(this);
	_script = new Script(this);
	_interface = new Interface(this); // requires script module
	_scene = new Scene(this);
	_actor = new Actor(this);
	_palanim = new PalAnim(this);
	_isoMap = new IsoMap(this);
	_puzzle = new Puzzle(this);

	// System initialization

	_previousTicks = _system->getMillis();

	// Initialize graphics
	_gfx = new Gfx(this, _system, getDisplayWidth(), getDisplayHeight(), detector);

	// Graphics driver should be initialized before console
	_console = new Console(this);

	// Graphics should be initialized before music
	int midiDriver = MidiDriver::detectMusicDriver(MDT_NATIVE | MDT_ADLIB | MDT_PREFER_NATIVE);
	bool native_mt32 = (ConfMan.getBool("native_mt32") || (midiDriver == MD_MT32));

	bool adlib = false;

	MidiDriver *driver = MidiDriver::createMidi(midiDriver);
	if (!driver) {
		driver = MidiDriver_ADLIB_create(_mixer);
		adlib = true;
	} else if (native_mt32)
		driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);

	_music = new Music(this, _mixer, driver, _musicVolume);
	_music->setNativeMT32(native_mt32);
	_music->setAdlib(adlib);

	if (!_musicVolume) {
		debug(1, "Music disabled.");
	}

	_render = new Render(this, _system);
	if (!_render->initialized()) {
		return FAILURE;
	}

	// Initialize system specific sound
	_sound = new Sound(this, _mixer, _soundVolume);
	if (!_soundVolume) {
		debug(1, "Sound disabled.");
	}

	_interface->converseInit();
	_script->setVerb(kVerbWalkTo);

	_music->setVolume(-1, 1);

	_gfx->initPalette();

	// FIXME: This is the ugly way of reducing redraw overhead. It works
	//        well for 320x200 but it's unclear how well it will work for
	//        640x480.

	if (getGameType() == GType_ITE)
		_system->setFeatureState(OSystem::kFeatureAutoComputeDirtyRects, true);

	return SUCCESS;
}

int SagaEngine::go() {
	int msec = 0;

	_previousTicks = _system->getMillis();

	if (ConfMan.hasKey("start_scene")) {
		_scene->changeScene(ConfMan.getInt("start_scene"), 0, kTransitionNoFade);
	} else if (ConfMan.hasKey("boot_param")) {
		if (getGameType() == GType_ITE)
			_interface->addToInventory(_actor->objIndexToId(ITE_OBJ_MAGIC_HAT));
		_scene->changeScene(ConfMan.getInt("boot_param"), 0, kTransitionNoFade);
	} else if (ConfMan.hasKey("save_slot")) {
		// First scene sets up palette
		_scene->changeScene(getStartSceneNumber(), 0, kTransitionNoFade);
		_events->handleEvents(0); // Process immediate events

		char *fileName;
		fileName = calcSaveFileName(ConfMan.getInt("save_slot"));
		load(fileName);
		_interface->setMode(kPanelMain);
	} else {
		_framesEsc = 0;
		_scene->startScene();
	}

	uint32 currentTicks;

	while (!_quit) {
		if (_console->isAttached())
			_console->onFrame();

		if (_render->getFlags() & RF_RENDERPAUSE) {
			// Freeze time while paused
			_previousTicks = _system->getMillis();
		} else {
			currentTicks = _system->getMillis();
			// Timer has rolled over after 49 days
			if (currentTicks < _previousTicks)
				msec = 0;
			else {
				msec = currentTicks - _previousTicks;
				_previousTicks = currentTicks;
			}
			if (msec > MAX_TIME_DELTA) {
				msec = MAX_TIME_DELTA;
			}

			// Since Puzzle is actorless, we do it here
			if (_puzzle->isActive()) {
				_actor->handleSpeech(msec);
			} else if (!_scene->isInIntro()) {
				if (_interface->getMode() == kPanelMain ||
						_interface->getMode() == kPanelConverse ||
						_interface->getMode() == kPanelCutaway ||
						_interface->getMode() == kPanelNull ||
						_interface->getMode() == kPanelChapterSelection)
					_actor->direct(msec);
			}

			_events->handleEvents(msec);
			_script->executeThreads(msec);
		}
		// Per frame processing
		_render->drawScene();
		_system->delayMillis(10);
	}

	return 0;
}

void SagaEngine::loadStrings(StringsTable &stringsTable, const byte *stringsPointer, size_t stringsLength) {
	uint16 stringsCount;
	size_t offset;
	int i;

	if (stringsLength == 0) {
		error("SagaEngine::loadStrings() Error loading strings list resource");
	}

	stringsTable.stringsPointer = (byte*)malloc(stringsLength);
	memcpy(stringsTable.stringsPointer, stringsPointer, stringsLength);


	MemoryReadStreamEndian scriptS(stringsTable.stringsPointer, stringsLength, isBigEndian()); //TODO: get endianess from context

	offset = scriptS.readUint16();
	stringsCount = offset / 2;
	stringsTable.strings = (const char **)malloc(stringsCount * sizeof(*stringsTable.strings));
	i = 0;
	scriptS.seek(0);
	while (i < stringsCount) {
		offset = scriptS.readUint16();
		if (offset == stringsLength) {
			stringsCount = i;
			stringsTable.strings = (const char **)realloc(stringsTable.strings, stringsCount * sizeof(*stringsTable.strings));
			break;
		}
		if (offset > stringsLength) {
			error("SagaEngine::loadStrings wrong strings table");
		}
		stringsTable.strings[i] = (const char *)stringsTable.stringsPointer + offset;
		debug(9, "string[%i]=%s", i, stringsTable.strings[i]);
		i++;
	}
	stringsTable.stringsCount = stringsCount;
}

const char *SagaEngine::getObjectName(uint16 objectId) {
	ActorData *actor;
	ObjectData *obj;
	const HitZone *hitZone;
	switch (objectTypeId(objectId)) {
	case kGameObjectObject:
		obj = _actor->getObj(objectId);
		if (getGameType() == GType_ITE)
			return _script->_mainStrings.getString(obj->_nameIndex);
		return _actor->_objectsStrings.getString(obj->_nameIndex);
	case kGameObjectActor:
		actor = _actor->getActor(objectId);
		return _actor->_actorsStrings.getString(actor->_nameIndex);
	case kGameObjectHitZone:
		hitZone = _scene->_objectMap->getHitZone(objectIdToIndex(objectId));
		return _scene->_sceneStrings.getString(hitZone->getNameIndex());
	}
	warning("SagaEngine::getObjectName name not found for 0x%X", objectId);
	return NULL;
}

const char *SagaEngine::getTextString(int textStringId) {
	const char *string;
	int lang = (getLanguage() == Common::DE_DEU) ? 1 : 0;

	string = ITEinterfaceTextStrings[lang][textStringId];
	if (!string)
		string = ITEinterfaceTextStrings[0][textStringId];

	return string;
}

void SagaEngine::getExcuseInfo(int verb, const char *&textString, int &soundResourceId) {
	textString = NULL;
	switch (verb) {
	case kVerbPickUp:
		textString = getTextString(kTextICantPickup);
		soundResourceId = RID_BOAR_VOICE_007;
		break;
	case kVerbLookAt:
		textString = getTextString(kTextNothingSpecial);
		soundResourceId = RID_BOAR_VOICE_006;
		break;
	case kVerbOpen:
		textString = getTextString(kTextNoPlaceToOpen);
		soundResourceId = RID_BOAR_VOICE_000;
		break;
	case kVerbClose:
		textString = getTextString(kTextNoOpening);
		soundResourceId = RID_BOAR_VOICE_002;
		break;
	case kVerbUse:
		textString = getTextString(kTextDontKnow);
		soundResourceId = RID_BOAR_VOICE_005;
		break;
	}
}

} // End of namespace Saga
