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

#include "pink/cursor_mgr.h"
#include "pink/pink.h"
#include "pink/objects/actors/lead_actor.h"
#include "pink/objects/handlers/handler.h"
#include "pink/objects/pages/game_page.h"
#include "pink/objects/sequences/sequencer.h"
#include "pink/objects/walk/walk_mgr.h"

namespace Pink {

GamePage::GamePage()
	: _module(nullptr), _cursorMgr(nullptr), _walkMgr(nullptr),
	_sequencer(nullptr), _isLoaded(false), _memFile(nullptr)  {}

GamePage::~GamePage() {
	clear();
	delete _memFile;
}

void GamePage::toConsole() const {
	Page::toConsole();
	_walkMgr->toConsole();
	_sequencer->toConsole();
	for (uint i = 0; i < _handlers.size(); ++i) {
		_handlers[i]->toConsole();
	}
}

void GamePage::deserialize(Archive &archive) {
	Page::deserialize(archive);
	_module = static_cast<Module *>(archive.readObject());
	assert(dynamic_cast<Module *>(_module) != 0);
}

void GamePage::load(Archive &archive) {
	debugC(6, kPinkDebugLoadingObjects, "GamePage load");
	archive.mapObject(_cursorMgr);
	archive.mapObject(_walkMgr);
	archive.mapObject(_sequencer);

	Page::load(archive);

	_leadActor = static_cast<LeadActor *>(archive.readObject());

	_walkMgr->deserialize(archive);
	_sequencer->deserialize(archive);
	_handlers.deserialize(archive);
}

void GamePage::init(bool isLoadingSave) {
	if (!_isLoaded)
		loadManagers();

	toConsole();

	initPalette();

	LeadActor::State state = _leadActor->getState();
	bool paused = (state == LeadActor::kInventory || state == LeadActor::kPDA);
	for (uint i = 0; i < _actors.size(); ++i) {
		_actors[i]->init(paused);
	}

	bool isHandler = false;
	if (!isLoadingSave)
		isHandler = initHandler();

	_leadActor->start(isHandler);
}

bool GamePage::initHandler() {
	for (uint i = 0; i < _handlers.size(); ++i) {
		if (_handlers[i]->isSuitable(_leadActor)) {
			_handlers[i]->handle(_leadActor);
			return true;
		}
	}
	return false;
}

void GamePage::loadManagers() {
	_isLoaded = true;

	_cursorMgr = new CursorMgr(_module->getGame(), this);
	_walkMgr = new WalkMgr;
	_sequencer = new Sequencer(this);

	debugC(6, kPinkDebugGeneral, "ResMgr init");
	_resMgr.init(_module->getGame(), this);

	if (_memFile != nullptr) {
		loadStateFromMem();

		delete _memFile;
		_memFile = nullptr;
	}
}

bool GamePage::checkValueOfVariable(const Common::String &variable, const Common::String &value) const {
	if (!_variables.contains(variable))
		return value == kUndefinedValue;
	return _variables[variable] == value;
}

void GamePage::setVariable(Common::String &variable, Common::String &value) {
	_variables[variable] = value;
	_leadActor->onVariableSet();
}

void GamePage::loadStateFromMem() {
	Archive archive(static_cast<Common::SeekableReadStream *>(_memFile));
	_variables.deserialize(archive);

	for (uint i = 0; i < _actors.size(); ++i) {
		_actors[i]->loadState(archive);
	}
}

void GamePage::saveStateToMem() {
	_memFile = new Common::MemoryReadWriteStream(DisposeAfterUse::YES);
	Archive archive(static_cast<Common::WriteStream *>(_memFile));
	_variables.serialize(archive);

	for (uint i = 0; i < _actors.size(); ++i) {
		_actors[i]->saveState(archive);
	}
}

void GamePage::loadState(Archive &archive) {
	uint size = archive.readDWORD();
	if (size) {
		_memFile = new Common::MemoryReadWriteStream(DisposeAfterUse::YES);
		for (uint i = 0; i < size; ++i) {
			_memFile->writeByte(archive.readByte());
		}
	}
}

void GamePage::saveState(Archive &archive) {
	if (this == _module->getPage()) {
		saveStateToMem();
		archive.writeDWORD(_memFile->size());
		archive.write(_memFile->getData(), _memFile->size());
		delete _memFile;
		_memFile = nullptr;
	} else {
		if (_memFile != nullptr) {
			archive.writeDWORD(_memFile->size());
			archive.write(_memFile->getData(), _memFile->size());
		} else {
			archive.writeDWORD(0);
		}
	}
}

void GamePage::unload() {
	_leadActor->setAction(_leadActor->findAction(kIdleAction));

	saveStateToMem();
	clear();

	_isLoaded = false;
}

void GamePage::clear() {
	Page::clear();
	_variables.clear(1);

	for (uint i = 0; i < _handlers.size(); ++i) {
		delete _handlers[i];
	}

	_handlers.clear();

	delete _cursorMgr; _cursorMgr = nullptr;
	delete _sequencer; _sequencer = nullptr;
	delete _walkMgr; _walkMgr = nullptr;
}

} // End of namespace Pink
