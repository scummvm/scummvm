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

#include "common/debug-channels.h"
#include "common/winexe_pe.h"
#include "common/config-manager.h"

#include "engines/advancedDetector.h"
#include "engines/util.h"

#include "graphics/cursorman.h"
#include "graphics/thumbnail.h"
#include "graphics/surface.h"

#include "pink/pink.h"
#include "pink/console.h"
#include "pink/director.h"
#include "pink/objects/module.h"
#include "pink/objects/actors/lead_actor.h"

namespace Pink {

PinkEngine::PinkEngine(OSystem *system, const ADGameDescription *desc)
	: Engine(system), _console(nullptr), _rnd("pink"),
	_desc(desc), _bro(nullptr), _menu(nullptr), _actor(nullptr),
	_module(nullptr), _director(nullptr), _pdaMgr(this) {

	DebugMan.addDebugChannel(kPinkDebugGeneral, "general", "General issues");
	DebugMan.addDebugChannel(kPinkDebugLoadingResources, "loading_resources", "Loading resources data");
	DebugMan.addDebugChannel(kPinkDebugLoadingObjects, "loading_objects", "Serializing objects from Orb");
	DebugMan.addDebugChannel(kPinkDebugScripts, "scripts", "Sequences");
	DebugMan.addDebugChannel(kPinkDebugActions, "actions", "Actions");

	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "install");
}

PinkEngine::~PinkEngine() {
	delete _console;
	delete _bro;
	_pdaMgr.close();
	for (uint i = 0; i < _modules.size(); ++i) {
		delete _modules[i];
	}
	for (uint j = 0; j < _cursors.size(); ++j) {
		delete _cursors[j];
	}
	delete _director;
	DebugMan.clearAllDebugChannels();
}

Common::Error PinkEngine::init() {
	debugC(10, kPinkDebugGeneral, "PinkEngine init");
	initGraphics(640, 480);

	Common::PEResources exeResources;
	Common::String fileName = isPeril() ? "pptp.exe" : "hpp.exe";
	if (!exeResources.loadFromEXE(fileName)) {
		return Common::kNoGameDataFoundError;
	}

	_console = new Console(this);
	_director = new Director();

	initMenu(exeResources);

	Common::String orbName;
	Common::String broName;
	if (isPeril()) {
		orbName = "PPTP.ORB";
		broName = "PPTP.BRO";
		_bro = new BroFile;
	} else {
		orbName = "HPP.ORB";
	}

	if (!_orb.open(orbName) || (_bro && !_bro->open(broName) && _orb.getTimestamp() == _bro->getTimestamp()))
		return Common::kNoGameDataFoundError;

	if (!loadCursors(exeResources))
		return Common::kNoGameDataFoundError;

	setCursor(kLoadingCursor);

	_orb.loadGame(this);
	debugC(6, kPinkDebugGeneral, "Modules are loaded");

	syncSoundSettings();

	if (ConfMan.hasKey("save_slot"))
		loadGameState(ConfMan.getInt("save_slot"));
	else
		initModule(_modules[0]->getName(), "", nullptr);

	return Common::kNoError;
}

Common::Error Pink::PinkEngine::run() {
	Common::Error error = init();
	if (error.getCode() != Common::kNoError)
		return error;

	while (!shouldQuit()) {
		Common::Event event;
		while (_eventMan->pollEvent(event)) {
			if (_director->processEvent(event))
				continue;

			switch (event.type) {
			case Common::EVENT_QUIT:
			case Common::EVENT_RTL:
				return Common::kNoError;
			case Common::EVENT_MOUSEMOVE:
				_actor->onMouseMove(event.mouse);
				break;
			case Common::EVENT_LBUTTONDOWN:
				_actor->onLeftButtonClick(event.mouse);
				break;
			case Common::EVENT_LBUTTONUP:
				_actor->onLeftButtonUp();
				break;
			case Common::EVENT_RBUTTONDOWN:
				if (isPeril())
					_actor->onRightButtonClick(event.mouse);
				break;
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_d && event.kbd.hasFlags(Common::KBD_CTRL)) {
					_console->attach();
					_console->onFrame();
				} else {
					_actor->onKeyboardButtonClick(event.kbd.keycode);
				}
				break;
			default:
				break;
			}
		}

		_actor->update();
		_director->update();
		_system->delayMillis(10);
	}

	return Common::kNoError;
}

void PinkEngine::pauseEngine(void *engine, bool pause) {
	PinkEngine *vm = (PinkEngine*)engine;
	vm->pauseEngineIntern(pause);
}

void PinkEngine::load(Archive &archive) {
	archive.skipString();
	archive.skipString();
	_modules.deserialize(archive);
}

void PinkEngine::initModule(const Common::String &moduleName, const Common::String &pageName, Archive *saveFile) {
	if (_module)
		removeModule();

	addModule(moduleName);
	if (saveFile)
		_module->loadState(*saveFile);

	debugC(6, kPinkDebugGeneral, "Module added");

	_module->init(saveFile ? kLoadingSave : kLoadingNewGame, pageName);
}

void PinkEngine::changeScene() {
	setCursor(kLoadingCursor);
	_director->clear();

	if (!_nextModule.empty() && _nextModule != _module->getName())
		initModule(_nextModule, _nextPage, nullptr);
	else
		_module->changePage(_nextPage);
}

void PinkEngine::addModule(const Common::String &moduleName) {
	_module = new Module(this, moduleName);

	_orb.loadObject(_module, _module->getName());

	for (uint i = 0; i < _modules.size(); ++i) {
		if (_modules[i]->getName() == moduleName) {
			delete _modules[i];
			_modules[i] = _module;
			break;
		}
	}
}

void PinkEngine::removeModule() {
	for (uint i = 0; i < _modules.size(); ++i) {
		if (_module == _modules[i]) {
			_pdaMgr.close();
			_modules[i] = new ModuleProxy(_module->getName());
			delete _module;
			_module = nullptr;
			break;
		}
	}
}

void PinkEngine::setVariable(Common::String &variable, Common::String &value) {
	_variables[variable] = value;
}

bool PinkEngine::checkValueOfVariable(Common::String &variable, Common::String &value) {
	if (!_variables.contains(variable))
		return value == kUndefinedValue;
	return _variables[variable] == value;
}

bool PinkEngine::loadCursors(Common::PEResources &exeResources) {
	bool isPokus = !isPeril();

	_cursors.reserve(kCursorsCount);

	_cursors.push_back(Graphics::WinCursorGroup::createCursorGroup(exeResources, kPokusLoadingCursorID));
	_cursors.push_back(Graphics::WinCursorGroup::createCursorGroup(exeResources, kPokusClickableFirstCursorID));
	_cursors.push_back(Graphics::WinCursorGroup::createCursorGroup(exeResources, kPokusClickableSecondCursorID));

	if (isPokus) {
		_cursors.push_back(Graphics::WinCursorGroup::createCursorGroup(exeResources, kPokusClickableThirdCursorID));
		_cursors.push_back(Graphics::WinCursorGroup::createCursorGroup(exeResources, kPokusNotClickableCursorID));
		_cursors.push_back(Graphics::WinCursorGroup::createCursorGroup(exeResources, kPokusHoldingItemCursorID));
	} else {
		_cursors.push_back(Graphics::WinCursorGroup::createCursorGroup(exeResources, kPerilClickableThirdCursorID));
		_cursors.push_back(Graphics::WinCursorGroup::createCursorGroup(exeResources, kPerilNotClickableCursorID));
		_cursors.push_back(Graphics::WinCursorGroup::createCursorGroup(exeResources, kPerilHoldingItemCursorID));
	}

	_cursors.push_back(Graphics::WinCursorGroup::createCursorGroup(exeResources, kPokusPDADefaultCursorID));

	if (isPokus) {
		_cursors.push_back(Graphics::WinCursorGroup::createCursorGroup(exeResources, kPokusPDAClickableFirstFrameCursorID));
		_cursors.push_back(Graphics::WinCursorGroup::createCursorGroup(exeResources, kPokusPDAClickableSecondFrameCursorID));
	} else {
		_cursors.push_back(Graphics::WinCursorGroup::createCursorGroup(exeResources, kPerilPDAClickableFirstFrameCursorID));
		_cursors.push_back(Graphics::WinCursorGroup::createCursorGroup(exeResources, kPerilPDAClickableSecondFrameCursorID));
	}

	_cursors.push_back(Graphics::WinCursorGroup::createCursorGroup(exeResources, kPokusExitLeftCursorID));
	_cursors.push_back(Graphics::WinCursorGroup::createCursorGroup(exeResources, kPokusExitRightCursorID));
	_cursors.push_back(Graphics::WinCursorGroup::createCursorGroup(exeResources, kPokusExitForwardCursorID));

	if (isPokus)
		_cursors.push_back(Graphics::WinCursorGroup::createCursorGroup(exeResources, kPokusExitDownCursorID));

	return true;
}

void PinkEngine::setCursor(uint cursorIndex) {
	Graphics::Cursor *cursor = _cursors[cursorIndex]->cursors[0].cursor;
	_system->setCursorPalette(cursor->getPalette(), cursor->getPaletteStartIndex(), cursor->getPaletteCount());
	_system->setMouseCursor(cursor->getSurface(), cursor->getWidth(), cursor->getHeight(),
							cursor->getHotspotX(), cursor->getHotspotY(), cursor->getKeyColor());
	_system->showMouse(true);
}

bool PinkEngine::canLoadGameStateCurrently() {
	return true;
}

bool PinkEngine::canSaveGameStateCurrently() {
	return true;
}

bool PinkEngine::hasFeature(Engine::EngineFeature f) const {
	return
		f == kSupportsRTL ||
		f == kSupportsLoadingDuringRuntime ||
		f == kSupportsSavingDuringRuntime;
}

void PinkEngine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);
	_director->pause(pause);
}

bool PinkEngine::isPeril() {
	return !strcmp(_desc->gameId, kPeril);
}

}
