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
#include "mtropolis/debug.h"
#include "mtropolis/runtime.h"

#include "mtropolis/plugins.h"
#include "mtropolis/plugin/standard.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/ptr.h"
#include "common/stuffit.h"
#include "common/system.h"
#include "common/winexe.h"

#include "engines/util.h"

#include "graphics/cursorman.h"
#include "graphics/maccursor.h"
#include "graphics/surface.h"
#include "graphics/pixelformat.h"
#include "graphics/wincursor.h"

namespace MTropolis {

static bool loadCursorsFromPE(CursorGraphicCollection &cursorGraphics, Common::SeekableReadStream *stream) {
	Common::SharedPtr<Common::WinResources> winRes(Common::WinResources::createFromEXE(stream));
	if (!winRes) {
		warning("Couldn't load resources from PE file");
		return false;
	}

	Common::Array<Common::WinResourceID> cursorGroupIDs = winRes->getIDList(Common::kWinGroupCursor);
	for (Common::Array<Common::WinResourceID>::const_iterator it = cursorGroupIDs.begin(), itEnd = cursorGroupIDs.end(); it != itEnd; ++it) {
		const Common::WinResourceID &id = *it;

		Common::SharedPtr<Graphics::WinCursorGroup> cursorGroup(Graphics::WinCursorGroup::createCursorGroup(winRes.get(), *it));
		if (!winRes) {
			warning("Couldn't load cursor group");
			return false;
		}

		if (cursorGroup->cursors.size() == 0) {
			// Empty?
			continue;
		}

		cursorGraphics.addWinCursorGroup(id.getID(), cursorGroup);
	}

	return true;
}

static bool loadCursorsFromMacResources(CursorGraphicCollection &cursorGraphics, Common::MacResManager &resMan) {
	const uint32 bwType = MKTAG('C', 'U', 'R', 'S');
	const uint32 colorType = MKTAG('c', 'r', 's', 'r');

	Common::MacResIDArray bwIDs = resMan.getResIDArray(bwType);
	Common::MacResIDArray colorIDs = resMan.getResIDArray(colorType);

	Common::MacResIDArray bwOnlyIDs;
	for (Common::MacResIDArray::const_iterator bwIt = bwIDs.begin(), bwItEnd = bwIDs.end(); bwIt != bwItEnd; ++bwIt) {
		bool hasColor = false;
		for (Common::MacResIDArray::const_iterator colorIt = colorIDs.begin(), colorItEnd = colorIDs.end(); colorIt != colorItEnd; ++colorIt) {
			if ((*colorIt) == (*bwIt)) {
				hasColor = true;
				break;
			}
		}

		if (!hasColor)
			bwOnlyIDs.push_back(*bwIt);
	}

	for (int cti = 0; cti < 2; cti++) {
		const uint32 resType = (cti == 0) ? bwType : colorType;
		const bool isBW = (cti == 0);
		const Common::MacResIDArray &resArray = (cti == 0) ? bwOnlyIDs : colorIDs;

		for (size_t i = 0; i < resArray.size(); i++) {
			Common::SeekableReadStream *resData = resMan.getResource(resType, resArray[i]);
			if (!resData) {
				warning("Failed to open cursor resource");
				return false;
			}

			Common::SharedPtr<Graphics::MacCursor> cursor(new Graphics::MacCursor());
			// Some CURS resources are 72 bytes instead of the expected 68, make sure they load as the correct format
			if (!cursor->readFromStream(*resData, isBW, 0xff, isBW)) {
				warning("Failed to load cursor resource");
				return false;
			}

			cursorGraphics.addMacCursor(resArray[i], cursor);
		}
	}

	return true;
}

struct MacObsidianResources : public ProjectResources {
	MacObsidianResources();
	~MacObsidianResources();

	void setup();
	Common::SeekableReadStream *getSegmentStream(int index) const;
	const Common::SharedPtr<CursorGraphicCollection> &getCursorGraphics() const;

private:
	Common::MacResManager _installerResMan;
	Common::MacResManager _dataFileResMan[5];

	Common::SeekableReadStream *_installerDataForkStream;
	Common::Archive *_installerArchive;
	Common::SeekableReadStream *_segmentStreams[6];

	Common::SharedPtr<CursorGraphicCollection> _cursorGraphics;
};

MacObsidianResources::MacObsidianResources() : _installerArchive(nullptr), _installerDataForkStream(nullptr) {
	for (int i = 0; i < 6; i++)
		_segmentStreams[i] = nullptr;

	_cursorGraphics.reset(new CursorGraphicCollection());
}

void MacObsidianResources::setup() {
	debug(1, "Opening Obsidian Mac installer package...");

	if (!_installerResMan.open("Obsidian Installer"))
		error("Failed to open Obsidian Installer");

	if (!_installerResMan.hasDataFork())
		error("Obsidian Installer has no data fork");

	_installerDataForkStream = _installerResMan.getDataFork();

	_installerArchive = Common::createStuffItArchive(_installerDataForkStream);
	if (!_installerArchive)
		error("Failed to open Obsidian Installer archive");

	debug(1, "Reading data from installer...");
	_segmentStreams[0] = _installerArchive->createReadStreamForMember("Obsidian Data 1");

	debug(1, "Opening data segments...");
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

	debug(1, "Opening resources...");

	const char *cursorSources[4] = {"Obsidian", "Basic.rPP", "mCursors.cPP", "Obsidian.cPP"};
	for (int i = 0; i < 4; i++) {
		const char *fileName = cursorSources[i];

		Common::MacResManager resMan;
		if (!resMan.open(Common::Path(fileName), *_installerArchive))
			error("Failed to open resources in file '%s'", fileName);

		if (!loadCursorsFromMacResources(*_cursorGraphics, resMan))
			error("Failed to read cursor resources from file '%s'", fileName);
	}

	debug(1, "Finished unpacking installer resources");
}

Common::SeekableReadStream *MacObsidianResources::getSegmentStream(int index) const {
	return _segmentStreams[index];
}

const Common::SharedPtr<CursorGraphicCollection>& MacObsidianResources::getCursorGraphics() const {
	return _cursorGraphics;
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
	Common::Event evt;
	Common::EventManager *eventMan = _system->getEventManager();

	while (eventMan->pollEvent(evt)) {
		switch (evt.type) {
		default:
			break;
		}
	}
}

Common::Error MTropolisEngine::run() {
	int preferredWidth = 1024;
	int preferredHeight = 768;
	bool enableFrameRateLimit = true;
	uint expectedFrameRate = 60;
	ColorDepthMode preferredColorDepthMode = kColorDepthMode8Bit;

	_runtime.reset(new Runtime());

	if (_gameDescription->gameID == GID_OBSIDIAN && _gameDescription->desc.platform == Common::kPlatformWindows) {
		preferredWidth = 640;
		preferredHeight = 480;
		preferredColorDepthMode = kColorDepthMode16Bit;

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

		Common::SharedPtr<CursorGraphicCollection> cursors(new CursorGraphicCollection());
		{
			// Has to be in this order, some resources from MCURSORS will clobber resources from the player executable
			const char *cursorFiles[3] = {"Obsidian.exe", "MCURSORS.C95", "Obsidian.c95"};

			for (int i = 0; i < 3; i++) {
				Common::File f;
				if (!f.open(cursorFiles[i]) || !loadCursorsFromPE(*cursors, &f)) {
					error("Failed to load resources");
				}
			}
		}

		desc->setCursorGraphics(cursors);

		Common::SharedPtr<MTropolis::PlugIn> standardPlugIn = PlugIns::createStandard();
		static_cast<Standard::StandardPlugIn *>(standardPlugIn.get())->getHacks().allowGarbledListModData = true;
		desc->addPlugIn(standardPlugIn);

		desc->addPlugIn(PlugIns::createObsidian());

		_runtime->queueProject(desc);
	} else if (_gameDescription->gameID == GID_OBSIDIAN && _gameDescription->desc.platform == Common::kPlatformMacintosh) {
		preferredWidth = 640;
		preferredHeight = 480;
		preferredColorDepthMode = kColorDepthMode16Bit;

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
		desc->setCursorGraphics(resources->getCursorGraphics());

		_runtime->queueProject(desc);
	}

	// Figure out pixel formats
	Graphics::PixelFormat modePixelFormats[kColorDepthModeCount];
	bool haveExactMode[kColorDepthModeCount];
	bool haveCloseMode[kColorDepthModeCount];

	for (int i = 0; i < kColorDepthModeCount; i++) {
		haveExactMode[i] = false;
		haveCloseMode[i] = false;
	}

	{
		Common::List<Graphics::PixelFormat> pixelFormats = _system->getSupportedFormats();

		Graphics::PixelFormat clut8Format = Graphics::PixelFormat::createFormatCLUT8();

		for (Common::List<Graphics::PixelFormat>::const_iterator it = pixelFormats.begin(), itEnd = pixelFormats.end(); it != itEnd; ++it) {
			const Graphics::PixelFormat &candidateFormat = *it;
			ColorDepthMode thisFormatMode = kColorDepthModeInvalid;
			bool isExactMatch = false;
			if (candidateFormat.rBits() == 8 && candidateFormat.gBits() == 8 && candidateFormat.bBits() == 8) {
				isExactMatch = (candidateFormat.aBits() == 8);
				thisFormatMode = kColorDepthMode32Bit;
			} else if (candidateFormat.rBits() == 5 && candidateFormat.bBits() == 5 && candidateFormat.bytesPerPixel == 2) {
				if (candidateFormat.gBits() == 5) {
					isExactMatch = true;
					thisFormatMode = kColorDepthMode16Bit;
				} else if (candidateFormat.gBits() == 6) {
					isExactMatch = false;
					thisFormatMode = kColorDepthMode16Bit;
				}
			} else if (candidateFormat == clut8Format) {
				isExactMatch = true;
				thisFormatMode = kColorDepthMode8Bit;
			}

			if (thisFormatMode != kColorDepthModeInvalid && !haveExactMode[thisFormatMode]) {
				if (isExactMatch) {
					haveExactMode[thisFormatMode] = true;
					haveCloseMode[thisFormatMode] = true;
					modePixelFormats[thisFormatMode] = candidateFormat;
				} else if (!haveCloseMode[thisFormatMode]) {
					haveCloseMode[thisFormatMode] = true;
					modePixelFormats[thisFormatMode] = candidateFormat;
				}
			}
		}
	}

	// Figure out a pixel format.  First try to find one that's at least as good or better.
	ColorDepthMode selectedMode = kColorDepthModeInvalid;
	for (int i = preferredColorDepthMode; i < kColorDepthModeCount; i++) {
		if (haveExactMode[i] || haveCloseMode[i]) {
			selectedMode = static_cast<ColorDepthMode>(i);
			break;
		}
	}

	// If that fails, then try to find the best one available
	if (selectedMode == kColorDepthModeInvalid) {
		for (int i = preferredColorDepthMode - 1; i >= 0; i++) {
			if (haveExactMode[i] || haveCloseMode[i]) {
				selectedMode = static_cast<ColorDepthMode>(i);
				break;
			}
		}
	}

	if (selectedMode == kColorDepthModeInvalid)
		error("Couldn't resolve a color depth mode");

	// Set up supported pixel modes
	for (int i = 0; i < kColorDepthModeCount; i++) {
		if (haveExactMode[i] || haveCloseMode[i])
			_runtime->setupDisplayMode(static_cast<ColorDepthMode>(i), modePixelFormats[i]);
	}

	// Set active mode
	_runtime->switchDisplayMode(selectedMode, selectedMode);
	_runtime->setDisplayResolution(preferredWidth, preferredHeight);

	initGraphics(preferredWidth, preferredHeight, &modePixelFormats[selectedMode]);
	setDebugger(new Console(this));

#ifdef MTROPOLIS_DEBUG_ENABLE
	if (ConfMan.getBool("mtropolis_debug_at_start")) {
		_runtime->debugSetEnabled(true);
	}
	if (ConfMan.getBool("mtropolis_pause_at_start")) {
		_runtime->debugBreak();
	}
#endif

	uint32 prevTimeStamp = _system->getMillis();

	int32 earliness = 0;	// In thousandths of a frame
	const int32 aheadLimit = 5000;
	const int32 behindLimit = -5000;
	const int32 jitterTolerance = 500;	// Half a frame
	const int skippedFrameLimit = 10;
	uint32 limitRollingAccumulatedThousandthsOfFrames = 0;
	uint32 limitRollingAccumulatedMSec = 0;
	bool paused = false;

	int frameCounter = 0;
	int numSkippedFrames = 0;
	while (!shouldQuit()) {
		handleEvents();

#ifdef MTROPOLIS_DEBUG_ENABLE
		if (_runtime->debugGetDebugger())
			paused = _runtime->debugGetDebugger()->isPaused();
#endif

		// Scheduling is a bit tricky here because the project frame can suspend mid-execution.
		bool skipDraw = false;
		bool forceDraw = false;
		bool frameWasRun = false;
		if (!paused)
			frameWasRun = _runtime->runProjectFrame();

		uint32 frameTime = _system->getMillis();
		const uint32 realElapsedThisFrame = frameTime - prevTimeStamp;
		prevTimeStamp = frameTime;

		if (frameWasRun) {
			frameCounter++;
			uint32 timeAdvance = realElapsedThisFrame;

			if (enableFrameRateLimit) {
				earliness += 1000;
				uint32 durationInThousandsthsOfAFrame = realElapsedThisFrame * expectedFrameRate;
				earliness -= static_cast<int32>(durationInThousandsthsOfAFrame);
				if (earliness > aheadLimit) {
					earliness = aheadLimit;
				} else if (earliness < behindLimit) {
					earliness = behindLimit;
				}

				if (earliness < -1000) {
					if (numSkippedFrames < skippedFrameLimit)
						skipDraw = true; // If we're lagging behind then don't draw
				} else if (earliness > jitterTolerance) {
					// If we're ahead by enough then pause.  Tolerate some slight addition to avoid
					// degenerate cases if the timer wobbles near a frame edge.
					uint millis = earliness / expectedFrameRate;
					if (millis)
						_system->delayMillis(millis);
				}

				limitRollingAccumulatedThousandthsOfFrames += 1000;

				const uint32 prevRollingTime = limitRollingAccumulatedMSec;

				// Resolve the actual msec to increment via rolling timers.
				// Round up so that if time is divided by 60 then to derive tick count then it will always be in sync.
				limitRollingAccumulatedMSec = (limitRollingAccumulatedThousandthsOfFrames + expectedFrameRate - 1) / expectedFrameRate;

				timeAdvance = limitRollingAccumulatedMSec - prevRollingTime;

				if (limitRollingAccumulatedThousandthsOfFrames >= expectedFrameRate * 1000) {
					const int32 secondsToRemove = limitRollingAccumulatedThousandthsOfFrames / (expectedFrameRate * 1000);
					limitRollingAccumulatedThousandthsOfFrames -= secondsToRemove * 1000 * expectedFrameRate;
					limitRollingAccumulatedMSec -= secondsToRemove * 1000;
				}
			}

			_runtime->advanceProjectTime(timeAdvance);
		}

		_runtime->runRealFrame(realElapsedThisFrame);

		if (forceDraw || !skipDraw || !_runtime->mustDraw()) {
			_runtime->drawFrame(_system);
			numSkippedFrames = 0;
		} else {
			if (frameWasRun)
				numSkippedFrames++;
		}
	}

	_runtime.release();

	return Common::kNoError;
}

void MTropolisEngine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);
}

} // End of namespace MTropolis
