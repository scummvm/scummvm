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

#include "mtropolis/plugins.h"
#include "mtropolis/plugin/standard.h"

#include "common/config-manager.h"
#include "common/macresman.h"
#include "common/ptr.h"
#include "common/stuffit.h"

namespace MTropolis {


struct MacObsidianResources : public ProjectResources {
	MacObsidianResources();
	~MacObsidianResources();

	void setup();
	Common::SeekableReadStream *getSegmentStream(int index) const;

private:
	Common::MacResManager _installerResMan;
	Common::MacResManager _dataFileResMan[5];

	Common::SeekableReadStream *_installerDataForkStream;
	Common::Archive *_installerArchive;
	Common::SeekableReadStream *_segmentStreams[6];
};

MacObsidianResources::MacObsidianResources() : _installerArchive(nullptr), _installerDataForkStream(nullptr) {
	for (int i = 0; i < 6; i++)
		_segmentStreams[i] = nullptr;
}

void MacObsidianResources::setup() {
	if (!_installerResMan.open("Obsidian Installer"))
		error("Failed to open Obsidian Installer");

	if (!_installerResMan.hasDataFork())
		error("Obsidian Installer has no data fork");

	_installerDataForkStream = _installerResMan.getDataFork();

	_installerArchive = Common::createStuffItArchive(_installerDataForkStream);
	if (!_installerArchive)
		error("Failed to open Obsidian Installer archive");

	_segmentStreams[0] = _installerArchive->createReadStreamForMember("Obsidian Data 1");

	for (int i = 0; i < 5; i++) {
		char fileName[32];
		sprintf(fileName, "Obsidian Data %i", (i + 2));

		Common::MacResManager &resMan = _dataFileResMan[i];
		if (!resMan.open(fileName))
			error("Failed to open data file %s", fileName);

		if (!resMan.hasDataFork())
			error("Data fork in %s is missing", fileName);

		_segmentStreams[1 + i] = resMan.getDataFork();
	}
}

Common::SeekableReadStream *MacObsidianResources::getSegmentStream(int index) const {
	return _segmentStreams[index];
}

MacObsidianResources::~MacObsidianResources() {
	for (int i = 0; i < 6; i++)
		delete _segmentStreams[i];

	delete _installerArchive;
	delete _installerDataForkStream;
}


MTropolisEngine::MTropolisEngine(OSystem *syst, const MTropolisGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {

	if (gameDesc->gameID == GID_OBSIDIAN && _gameDescription->desc.platform == Common::kPlatformWindows) {
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

		Common::SharedPtr<MTropolis::PlugIn> standardPlugIn = PlugIns::createStandard();
		static_cast<Standard::StandardPlugIn *>(standardPlugIn.get())->getHacks().allowGarbledListModData = true;
		desc->addPlugIn(standardPlugIn);

		desc->addPlugIn(PlugIns::createObsidian());

		_runtime->queueProject(desc);
	} else if (_gameDescription->gameID == GID_OBSIDIAN && _gameDescription->desc.platform == Common::kPlatformMacintosh) {
		MacObsidianResources *resources = new MacObsidianResources();
		Common::SharedPtr<ProjectResources> resPtr(resources);

		resources->setup();

		_runtime->addVolume(0, "Installed", true);
		_runtime->addVolume(1, "OBSIDIAN1", true);
		_runtime->addVolume(2, "OBSIDIAN2", true);
		_runtime->addVolume(3, "OBSIDIAN3", true);
		_runtime->addVolume(4, "OBSIDIAN4", true);
		_runtime->addVolume(5, "OBSIDIAN5", true);

		Common::SharedPtr<ProjectDescription> desc(new ProjectDescription());

		for (int i = 0; i < 6; i++)
			desc->addSegment(i, resources->getSegmentStream(i));

		Common::SharedPtr<MTropolis::PlugIn> standardPlugIn = PlugIns::createStandard();
		static_cast<Standard::StandardPlugIn *>(standardPlugIn.get())->getHacks().allowGarbledListModData = true;
		desc->addPlugIn(standardPlugIn);

		desc->addPlugIn(PlugIns::createObsidian());

		desc->setResources(resPtr);

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
