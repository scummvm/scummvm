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
 */
#include "engines/util.h"
#include "graphics/thumbnail.h"
#include "graphics/surface.h"
#include "common/error.h"
#include "actor.h"
#include "actorresource.h"
#include "background.h"
#include "bigfile.h"
#include "cursor.h"
#include "dragonflg.h"
#include "dragonimg.h"
#include "dragonini.h"
#include "dragonobd.h"
#include "dragonrms.h"
#include "dragonvar.h"
#include "dragons.h"
#include "inventory.h"
#include "scene.h"
#include "screen.h"
#include "sequenceopcodes.h"
#include "scriptopcodes.h"

namespace Dragons {

#define DRAGONS_TICK_INTERVAL 17

static DragonsEngine *_engine = nullptr;

DragonsEngine *getEngine() {
	return _engine;
}

DragonsEngine::DragonsEngine(OSystem *syst) : Engine(syst) {
	_bigfileArchive = NULL;
	_dragonRMS = NULL;
	_backgroundResourceLoader = NULL;
	_screen = NULL;
	_nextUpdatetime = 0;
	_flags = 0;
	_unkFlags1 = 0;
	_sequenceOpcodes = new SequenceOpcodes(this);
	_scriptOpcodes = NULL;
	_engine = this;
	_cursorPosition = Common::Point();
	_cursorSequenceID = 0;
	run_func_ptr_unk_countdown_timer = 0;
	data_8006a3a0_flag = 0;
	data_800633fa = 0;
	_inventory = new Inventory(this);
	_cursor = new Cursor(this);
}

DragonsEngine::~DragonsEngine() {
	delete _sequenceOpcodes;
	delete _scriptOpcodes;
}

void DragonsEngine::updateEvents() {
	Common::Event event;
	while (_eventMan->pollEvent(event)) {
//		_input->processEvent(event);
		switch (event.type) {
			case Common::EVENT_QUIT:
				quitGame();
				break;
			case Common::EVENT_MOUSEMOVE:
				_cursor->updatePosition(event.mouse.x, event.mouse.y);
				break;
			default:
				break;
		}
	}
}

Common::Error DragonsEngine::run() {
	_screen = new Screen();
	_bigfileArchive = new BigfileArchive("bigfile.dat", Common::Language::EN_ANY);
	_dragonFLG = new DragonFLG(_bigfileArchive);
	_dragonIMG = new DragonIMG(_bigfileArchive);
	_dragonOBD = new DragonOBD(_bigfileArchive);
	_dragonRMS = new DragonRMS(_bigfileArchive, _dragonOBD);
	_dragonVAR = new DragonVAR(_bigfileArchive);
	_dragonINIResource = new DragonINIResource(_bigfileArchive);
	ActorResourceLoader *actorResourceLoader = new ActorResourceLoader(_bigfileArchive);
	_actorManager = new ActorManager(actorResourceLoader);
	_scriptOpcodes = new ScriptOpcodes(this, _dragonFLG);
	_scene = new Scene(this, _screen, _scriptOpcodes, _bigfileArchive, _actorManager, _dragonRMS, _dragonINIResource);
	_flags = 0x1046;
	_flags &= 0x1c07040;
	_flags |= 0x26;

	_cursor->init(_actorManager, _dragonINIResource);
	_inventory->init(_actorManager);

	uint16 sceneId = 0x12;
	_dragonINIResource->getFlickerRecord()->sceneId = sceneId; //TODO
	_sceneId1 = sceneId;
	_scene->loadScene(sceneId, 0x1e);

	_scene->draw();
	_screen->updateScreen();

	gameLoop();

	delete _scene;
	delete _actorManager;
	delete _backgroundResourceLoader;
	delete _dragonFLG;
	delete _dragonIMG;
	delete _dragonRMS;
	delete _dragonVAR;
	delete _bigfileArchive;
	delete _screen;

	debug("Ok");
	return Common::kNoError;
}

void DragonsEngine::gameLoop() {
	_counter = 0;
	bit_flags_8006fbd8 = 0;
	while (!shouldQuit()) {
		updateHandler();
		updateEvents();

		if (getCurrentSceneId() != 2) {
			_sceneId1 = getCurrentSceneId();
		}

		_counter++;
		DragonINI *flickerIni = _dragonINIResource->getFlickerRecord();
		if (_counter >= 1200 && flickerIni->actor->resourceID == 0xe) { // 0xe == flicker.act
			Actor *actor = flickerIni->actor;
			actor->_sequenceID2 = 2;
			flickerIni->field_20_actor_field_14 = 2;

			actor->updateSequence(getINI(0xc2)->sceneId == 1 ? 0x30 : 2);
			_counter = 0;
			setFlags(Dragons::ENGINE_FLAG_80000000);
		}

		if (_flags & Dragons::ENGINE_FLAG_80000000) {
			if (flickerIni->actor->flags & Dragons::ACTOR_FLAG_4) {
				_counter = 0;
				clearFlags(Dragons::ENGINE_FLAG_80000000);
			}
		}

		if (bit_flags_8006fbd8 == 0) {
			setFlags(Dragons::ENGINE_FLAG_8);
		}

		if (flickerIni->sceneId == getCurrentSceneId()) {
			uint16 id = getIniFromImg();
			if (id != 0) {
				// 0x80026cac
				error("todo 0x80026cac run script");
			} else {
				// 0x80026d34
				// $s4_1 = 0;
			}
		} else {
			// 0x80026d34
			// $s4_1 = 0;
		}

		// 0x80026d38
		_cursor->updateINIUnderCursor();

		runINIScripts();

		_scene->draw();
		_screen->updateScreen();
		wait();
	}
}

void DragonsEngine::updateHandler() {
	data_8006a3a0_flag |= 0x40;
	//TODO logic here

	updateActorSequences();

	_cursor->updateVisibility();
	_inventory->updateVisibility();

	//TODO logic here
	for (uint16 i = 0; i < 0x17; i++) {
		Actor *actor = _actorManager->getActor(i);
		if (actor->flags & Dragons::ACTOR_FLAG_40) {
			if (!(actor->flags & Dragons::ACTOR_FLAG_100)) {
				int16 priority = _scene->getPriorityAtPosition(Common::Point(actor->x_pos, actor->y_pos));
				DragonINI *flicker = _dragonINIResource->getFlickerRecord();
				if (flicker && _scene->contains(flicker) && flicker->actor->_actorID == i) {
					if (priority < 8 || priority == 0x10) {
						actor->priorityLayer = priority;
					}
				} else {
					if (priority != -1) {
						actor->priorityLayer = priority;
					}
				}

				if (actor->priorityLayer >= 0x11) {
					actor->priorityLayer = 0;
				}

				if (actor->priorityLayer >= 9) {
					actor->priorityLayer -= 8;
				}
			}

			if (actor->sequenceTimer != 0) {
				actor->sequenceTimer--;
			}
		}
	}

	if (_flags & Dragons::ENGINE_FLAG_80) {
		for (uint16 i = 0x17; i < DRAGONS_ENGINE_NUM_ACTORS; i++) {
			Actor *actor = _actorManager->getActor(i);
			if (actor->sequenceTimer != 0) {
				actor->sequenceTimer--;
			}
		}
	}

	if (isFlagSet(ENGINE_FLAG_4)) {
		updatePathfindingActors();
	}

	// TODO 0x8001bed0

	// 0x8001c294
	if (!(_unkFlags1 & ENGINE_UNK1_FLAG_8)) {
		//TODO ReadPad();
	}

	if (isFlagSet(ENGINE_FLAG_20)) {
		engineFlag0x20UpdateFunction();
	}

	//TODO vsync update function

	// TODO data_8006a3a0 logic. @ 0x8001c2f4

	data_8006a3a0_flag &= ~0x40;
}

const char *DragonsEngine::getSavegameFilename(int num) {
	static Common::String filename;
	filename = getSavegameFilename(_targetName, num);
	return filename.c_str();
}

Common::String DragonsEngine::getSavegameFilename(const Common::String &target, int num) {
	assert(num >= 0 && num <= 999);
	return Common::String::format("%s.%03d", target.c_str(), num);
}

#define DRAGONS_SAVEGAME_VERSION 0

kReadSaveHeaderError
DragonsEngine::readSaveHeader(Common::SeekableReadStream *in, SaveHeader &header, bool skipThumbnail) {

	header.version = in->readUint32LE();
	if (header.version > DRAGONS_SAVEGAME_VERSION)
		return kRSHEInvalidVersion;

	byte descriptionLen = in->readByte();
	header.description = "";
	while (descriptionLen--) {
		header.description += (char) in->readByte();
	}

	if (!Graphics::loadThumbnail(*in, header.thumbnail, skipThumbnail)) {
		return kRSHEIoError;
	}

	header.flags = in->readUint32LE();

	header.saveDate = in->readUint32LE();
	header.saveTime = in->readUint32LE();
	header.playTime = in->readUint32LE();

	return ((in->eos() || in->err()) ? kRSHEIoError : kRSHENoError);
}

uint32 DragonsEngine::calulateTimeLeft() {
	uint32 now;

	now = _system->getMillis();

	if (_nextUpdatetime <= now) {
		_nextUpdatetime = now + DRAGONS_TICK_INTERVAL;
		return (0);
	}
	uint32 delay = _nextUpdatetime - now;
	_nextUpdatetime += DRAGONS_TICK_INTERVAL;
	return (delay);
}

void DragonsEngine::wait() {
	_system->delayMillis(calulateTimeLeft());
}

void DragonsEngine::updateActorSequences() {
	if (!(_flags & Dragons::ENGINE_FLAG_4)) {
		return;
	}

	//TODO ResetRCnt(0xf2000001);

	int16 actorId = _flags & Dragons::ENGINE_FLAG_80 ? (int16) 64 : (int16) 23;

	while (actorId > 0) {
		actorId--;
		Actor *actor = _actorManager->getActor((uint16) actorId);
		if (actorId < 2 && _flags & Dragons::ENGINE_FLAG_40) {
			continue;
		}

		if (actor->flags & Dragons::ACTOR_FLAG_40 &&
			!(actor->flags & Dragons::ACTOR_FLAG_4) &&
			!(actor->flags & Dragons::ACTOR_FLAG_400) &&
			(actor->sequenceTimer == 0 || actor->flags & Dragons::ACTOR_FLAG_1)) {
			debug("Actor[%d] execute sequenceOp", actorId);

			if (actor->flags & Dragons::ACTOR_FLAG_1) {
				actor->resetSequenceIP();
				actor->flags &= 0xeff6; //TODO rewrite using ACTOR_FLAG_nnn
				actor->field_7a = 0;
			}
			//TODO execute sequence Opcode here.
			OpCall opCall;
			opCall._result = 1;
			while (opCall._result == 1) {
				opCall._op = (byte) READ_LE_UINT16(actor->_seqCodeIp);
				opCall._code = actor->_seqCodeIp + 2;
				_sequenceOpcodes->execOpcode(actor, opCall);
				actor->_seqCodeIp += opCall._deltaOfs;
			}
		}
	}
}

void DragonsEngine::setFlags(uint32 flags) {
	_flags |= flags;
}

void DragonsEngine::clearFlags(uint32 flags) {
	_flags &= ~flags;
}

void DragonsEngine::setUnkFlags(uint32 flags) {
	_unkFlags1 |= flags;
}

void DragonsEngine::clearUnkFlags(uint32 flags) {
	_unkFlags1 &= ~flags;
}

byte *DragonsEngine::getBackgroundPalette() {
	assert(_scene);
	return _scene->getPalette();
}

bool DragonsEngine::isFlagSet(uint32 flag) {
	return (bool) (_flags & flag);
}

bool DragonsEngine::isUnkFlagSet(uint32 flag) {
	return (bool) (_unkFlags1 & flag);
}

DragonINI *DragonsEngine::getINI(uint32 index) {
	return _dragonINIResource->getRecord(index);
}

uint16 DragonsEngine::getVar(uint16 offset) {
	return _dragonVAR->getVar(offset);
}

uint16 DragonsEngine::getCurrentSceneId() {
	return _scene->getSceneId();
}

void DragonsEngine::setVar(uint16 offset, uint16 value) {
	return _dragonVAR->setVar(offset, value);
}

uint16 DragonsEngine::getIniFromImg() {
	DragonINI *flicker = _dragonINIResource->getFlickerRecord();

	int16 x = flicker->actor->x_pos / 32;
	int16 y = flicker->actor->y_pos / 8;

	uint16 currentSceneId = _scene->getSceneId();

	for (uint16 i = 0; i < _dragonINIResource->totalRecords(); i++) {
		DragonINI *ini = getINI(i);
		if (ini->sceneId == currentSceneId && ini->field_1a_flags_maybe == 0) {
			IMG *img = _dragonIMG->getIMG(ini->field_2);
			if (x >= img->x &&
				img->x + img->w >= x &&
				y >= img->y &&
				img->h + img->y >= y) {
				return i + 1;
			}
		}

	}
	return 0;
}

uint16 DragonsEngine::updateINIUnderCursor() {
	int32 x = (_cursorPosition.x + _scene->_camera.x) / 32;
	int32 y = (_cursorPosition.y + _scene->_camera.y) / 8;

	if (_flags & Dragons::ENGINE_FLAG_10) {

		if (_inventory->getSequenceId() == 0 || _inventory->getSequenceId() == 2) {
//TODO
		} else {

		}
	}

	return 0;
}

void DragonsEngine::runINIScripts() {
	for (uint16 i = 0; i < _dragonINIResource->totalRecords(); i++) {
		DragonINI *ini = getINI(i);
		if (ini->field_1a_flags_maybe & Dragons::INI_FLAG_10) {
			ini->field_1a_flags_maybe &= ~Dragons::INI_FLAG_10;
			byte *data = _dragonOBD->getFromOpt(i);
			ScriptOpCall scriptOpCall;
			scriptOpCall._code = data + 8;
			scriptOpCall._codeEnd = scriptOpCall._code + READ_LE_UINT32(data);
			uint32 currentFlags = _flags;
			clearFlags(Dragons::ENGINE_FLAG_8);
			_scriptOpcodes->runScript3(scriptOpCall);
			_flags = currentFlags;
		}
	}
}

void DragonsEngine::engineFlag0x20UpdateFunction() {
	if (_flags & Dragons::ENGINE_FLAG_20) {
		if ((_flags & (Dragons::ENGINE_FLAG_80000000 | Dragons::ENGINE_FLAG_8)) == 8) {
			_cursor->update();
		}
		//TODO 0x80027be4

		uint16 currentSceneId = _scene->getSceneId();

		// 0x80027db8
		if (!_inventory->isVisible()) {
			for (uint16 i = 0; i < _dragonINIResource->totalRecords(); i++) {
				DragonINI *ini = getINI(i);
				if (ini->field_10 >= 0 && ini->sceneId == currentSceneId) {
					ini->field_10--;
					if (ini->field_10 < 0) {
						ini->field_1a_flags_maybe |= Dragons::INI_FLAG_10;
					}
				}
			}
		}

		if (run_func_ptr_unk_countdown_timer != 0) {
			run_func_ptr_unk_countdown_timer--;
		}
	} else {
		run_func_ptr_unk_countdown_timer = 1;
	}
}

void DragonsEngine::waitForFrames(uint16 numFrames) {
	for (uint16 i = 0; i < numFrames; i++) {
		wait();
		updateHandler();

		_scene->draw();
		_screen->updateScreen();
		updateEvents();
	}
}

void DragonsEngine::playSound(uint16 soundId) {
	debug(3, "TODO: play sound %d", soundId);
}

void DragonsEngine::updatePathfindingActors() {
	for (uint16 i = 0; i < 0x17; i++) {
		Actor *actor = _actorManager->getActor(i);
		actor->walkPath();
	}
}

void DragonsEngine::fade_related(uint32 flags) {
	if (!isFlagSet(ENGINE_FLAG_40)) {
		return;
	}
	setUnkFlags(ENGINE_UNK1_FLAG_2);
	clearFlags(ENGINE_FLAG_40);

	//TODO 0x80015a1c
}

void DragonsEngine::call_fade_related_1f() {
	fade_related(0x1f);
}

} // End of namespace Dragons
