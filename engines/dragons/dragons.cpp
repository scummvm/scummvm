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
#include "bigfile.h"
#include "dragonrms.h"
#include "dragonini.h"
#include "dragons.h"
#include "scene.h"
#include "screen.h"
#include "sequenceopcodes.h"

namespace Dragons {

#define DRAGONS_TICK_INTERVAL 17

DragonsEngine::DragonsEngine(OSystem *syst) : Engine(syst) {
	_bigfileArchive = NULL;
	_dragonRMS = NULL;
	_backgroundResourceLoader = NULL;
	_screen = NULL;
	_nextUpdatetime = 0;
	_flags = 0;
	_sequenceOpcodes = new SequenceOpcodes(this);
}

DragonsEngine::~DragonsEngine() {
	delete _sequenceOpcodes;
}

void DragonsEngine::updateEvents() {
	Common::Event event;
	while (_eventMan->pollEvent(event)) {
//		_input->processEvent(event);
		switch (event.type) {
			case Common::EVENT_QUIT:
				quitGame();
				break;
			default:
				break;
		}
	}
}
Common::Error DragonsEngine::run() {
	_screen = new Screen();
	_bigfileArchive = new BigfileArchive("bigfile.dat", Common::Language::EN_ANY);
	_dragonRMS = new DragonRMS(_bigfileArchive);
	_dragonINIResource = new DragonINIResource(_bigfileArchive);
	ActorResourceLoader *actorResourceLoader = new ActorResourceLoader(_bigfileArchive);
	_actorManager = new ActorManager(actorResourceLoader);
	_scene = new Scene(_screen, _bigfileArchive, _actorManager, _dragonRMS, _dragonINIResource);
	_flags = 0x1046;

	_scene->loadScene(0x12, 0x1e);

	_scene->draw();
	_screen->updateScreen();

	gameLoop();

	delete _scene;
	delete _actorManager;
	delete _backgroundResourceLoader;
	delete _dragonRMS;
	delete _bigfileArchive;
	delete _screen;

	debug("Ok");
	return Common::kNoError;
}

void DragonsEngine::gameLoop() {
	while (!shouldQuit()) {
		updateHandler();
		updateEvents();
		wait();
	}
}

void DragonsEngine::updateHandler() {
	updateActorSequences();
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

kReadSaveHeaderError DragonsEngine::readSaveHeader(Common::SeekableReadStream *in, SaveHeader &header, bool skipThumbnail) {

	header.version = in->readUint32LE();
	if (header.version > DRAGONS_SAVEGAME_VERSION)
		return kRSHEInvalidVersion;

	byte descriptionLen = in->readByte();
	header.description = "";
	while (descriptionLen--) {
		header.description += (char)in->readByte();
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

	if ( _nextUpdatetime <= now ) {
		_nextUpdatetime = now + DRAGONS_TICK_INTERVAL;
		return(0);
	}
	uint32 delay = _nextUpdatetime - now;
	_nextUpdatetime += DRAGONS_TICK_INTERVAL;
	return(delay);
}

void DragonsEngine::wait() {
	_system->delayMillis(calulateTimeLeft());
}

void DragonsEngine::updateActorSequences() {
	if (!(_flags & Dragons::ENGINE_FLAG_4)) {
		return;
	}

	//TODO ResetRCnt(0xf2000001);

	int16 actorId = _flags & Dragons::ENGINE_FLAG_80 ? (int16)64 : (int16)23;

	while (actorId >= 0) {
		actorId--;
		Actor *actor = _actorManager->getActor((uint16)actorId);
		if (actorId < 2 && _flags & Dragons::ENGINE_FLAG_40) {
			continue;
		}

		if (actor->flags & Dragons::ACTOR_FLAG_40 &&
				!(actor->flags & Dragons::ACTOR_FLAG_4) &&
				!(actor->flags & Dragons::ACTOR_FLAG_400) &&
				(actor->frameIndex_maybe == 0 || actor->flags & Dragons::ACTOR_FLAG_1)) {
			debug("Actor[%d] execute sequenceOp", actorId);

			if (actor->flags & Dragons::ACTOR_FLAG_1) {
				actor->resetSequenceIP();
				actor->flags &= 0xeff6; //TODO rewrite using ACTOR_FLAG_nnn
				actor->field_7a = 0;
			}
			//TODO execute sequence Opcode here.
			OpCall opCall;
			opCall._op = (byte)READ_LE_UINT16(actor->_seqCodeIp);
			opCall._opSize = (byte)READ_LE_UINT16(actor->_seqCodeIp + 2);
			opCall._code = actor->_seqCodeIp + 4;
			opCall._deltaOfs = opCall._opSize;
			_sequenceOpcodes->execOpcode(actor, opCall);
			return;
		}
	}
}

} // End of namespace Dragons
