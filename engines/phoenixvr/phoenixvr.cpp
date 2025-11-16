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

#include "phoenixvr/phoenixvr.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/file.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/framelimiter.h"
#include "graphics/paletteman.h"
#include "phoenixvr/console.h"
#include "phoenixvr/detection.h"
#include "phoenixvr/pakf.h"
#include "phoenixvr/region_set.h"
#include "phoenixvr/script.h"

namespace PhoenixVR {

PhoenixVREngine *g_engine;

PhoenixVREngine::PhoenixVREngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
																					 _gameDescription(gameDesc),
																					 _randomSource("PhoenixVR"),
																					 _pixelFormat(Graphics::PixelFormat::createFormatRGB24()) {
	g_engine = this;
	auto path = Common::FSNode(ConfMan.getPath("path"));
	SearchMan.addSubDirectoryMatching(path, "NecroES/Data", 1, 1, true);
	SearchMan.addSubDirectoryMatching(path, "NecroES/Data2", 2, 1, true);
}

PhoenixVREngine::~PhoenixVREngine() {
	delete _screen;
}

uint32 PhoenixVREngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String PhoenixVREngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::String PhoenixVREngine::resolvePath(const Common::String &path) {
	auto resolved = removeDrive(path);
	resolved.replace('\\', '/');
	return resolved;
}

Common::String PhoenixVREngine::removeDrive(const Common::String &path) {
	if (path.size() < 2 || path[1] != ':')
		return path;
	else
		return path.substr(2);
}

void PhoenixVREngine::setNextScript(const Common::String &path) {
	_nextScript = resolvePath(path);
	debug("setNextScript %s", _nextScript.c_str());
}

void PhoenixVREngine::goToWarp(const Common::String &warp) {
	debug("gotowarp %s", warp.c_str());
	_nextWarp = warp;
}

void PhoenixVREngine::setCursorDefault(uint idx, const Common::String &path) {
	debug("setCursorDefault %u: %s", idx, path.c_str());
}

void PhoenixVREngine::declareVariable(const Common::String &name) {
	_variables.setVal(name, 0);
}

void PhoenixVREngine::setVariable(const Common::String &name, int value) {
	debug("set %s %d", name.c_str(), value);
	_variables.setVal(name, value);
}

int PhoenixVREngine::getVariable(const Common::String &name) const {
	return _variables.getVal(name);
}

Common::Error PhoenixVREngine::run() {
	initGraphics(640, 480, &_pixelFormat);
	_screen = new Graphics::Screen();
	{
		Common::File vars;
		if (vars.open(Common::Path("variable.txt"))) {
			while (!vars.eos()) {
				auto var = vars.readLine();
				declareVariable(var);
			}
		}
	}
	setNextScript("script.lst");

	// Set the engine's debugger console
	setDebugger(new Console());

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	Common::Event e;

	Graphics::FrameLimiter limiter(g_system, 60);
	while (!shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
		}
		if (!_nextScript.empty()) {
			debug("loading script from %s", _nextScript.c_str());
			auto nextScript = Common::move(_nextScript);
			_nextScript.clear();

			nextScript = removeDrive(nextScript);

			Common::File file;
			Common::Path nextPath(nextScript);
			if (file.open(nextPath)) {
				_script.reset(new Script(file));
			} else {
				auto pakFile = nextPath;
				pakFile = pakFile.removeExtension().append(".pak");
				file.open(pakFile);
				if (!file.isOpen())
					error("can't open script file %s", nextScript.c_str());
				Common::ScopedPtr<Common::SeekableReadStream> scriptStream(unpack(file));
				_script.reset(new Script(*scriptStream));
			}
			goToWarp(_script->getInitScript()->vrFile);
		}
		if (!_nextWarp.empty()) {
			_warp = _script->getWarp(_nextWarp);
			_nextWarp.clear();
			debug("warp %s %s", _warp->vrFile.c_str(), _warp->testFile.c_str());
			_regSet.reset(new RegionSet(_warp->testFile));

			Script::ExecutionContext ctx{this, true};
			debug("execute warp script %s", _warp->vrFile.c_str());
			auto &test = _warp->getDefaultTest();
			test->scope.exec(ctx);
		}

		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		limiter.delayBeforeSwap();
		_screen->update();
		limiter.startFrame();
	}

	return Common::kNoError;
}

Common::Error PhoenixVREngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

} // End of namespace PhoenixVR
