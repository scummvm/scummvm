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

#include "mtropolis/mtropolis.h"
#include "mtropolis/console.h"
#include "mtropolis/runtime.h"

#include "common/config-manager.h"

namespace MTropolis {

MTropolisEngine::MTropolisEngine(OSystem *syst, const MTropolisGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {

	if (gameDesc->gameID == GID_OBSIDIAN) {
		const Common::FSNode gameDataDir(ConfMan.get("path"));
		SearchMan.addSubDirectoryMatching(gameDataDir, "Obsidian");
		SearchMan.addSubDirectoryMatching(gameDataDir, "Obsidian/RESOURCE");
	}
}

MTropolisEngine::~MTropolisEngine() {

}

void MTropolisEngine::handleEvents() {

}

Common::Error MTropolisEngine::run() {

	_runtime.reset(new Runtime());

	if (_gameDescription->gameID == GID_OBSIDIAN && _gameDescription->desc.platform == Common::kPlatformWindows) {
		_runtime->addVolume(0, "Installed", true);
		_runtime->addVolume(1, "OBSIDIAN1", true);
		_runtime->addVolume(2, "OBSIDIAN2", true);
		_runtime->addVolume(3, "OBSIDIAN3", true);
		_runtime->addVolume(4, "OBSIDIAN4", true);
		_runtime->addVolume(5, "OBSIDIAN5", true);

		Common::SharedPtr<ProjectDescription> desc(new ProjectDescription());
		desc->addSegment(0, "Obsidian Data 1.MPL");
		desc->addSegment(1, "Obsidian Data 2.MPX");
		desc->addSegment(2, "Obsidian Data 3.MPX");
		desc->addSegment(3, "Obsidian Data 4.MPX");
		desc->addSegment(4, "Obsidian Data 5.MPX");
		desc->addSegment(5, "Obsidian Data 6.MPX");

		_runtime->queueProject(desc);
	}

	while (!shouldQuit()) {
		_runtime->runFrame();
	}

	_runtime.release();

	return Common::kNoError;
}

void MTropolisEngine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);
}

} // End of namespace MTropolis
