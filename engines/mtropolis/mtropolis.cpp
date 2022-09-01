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

#include "mtropolis/mtropolis.h"

#include "mtropolis/actions.h"
#include "mtropolis/boot.h"
#include "mtropolis/debug.h"
#include "mtropolis/runtime.h"

#include "mtropolis/plugins.h"
#include "mtropolis/plugin/standard.h"
#include "mtropolis/plugin/obsidian.h"

namespace MTropolis {

MTropolisEngine::MTropolisEngine(OSystem *syst, const MTropolisGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc), _saveWriter(nullptr), _isTriggeredAutosave(false) {
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "Resource");

	bootAddSearchPaths(gameDataDir, *gameDesc);
}

MTropolisEngine::~MTropolisEngine() {
}

void MTropolisEngine::handleEvents() {
	Common::Event evt;
	Common::EventManager *eventMan = _system->getEventManager();

	while (eventMan->pollEvent(evt)) {
		switch (evt.type) {
		case Common::EVENT_LBUTTONDOWN:
			_runtime->onMouseDown(evt.mouse.x, evt.mouse.y, MTropolis::Actions::kMouseButtonLeft);
			break;
		case Common::EVENT_MBUTTONDOWN:
			_runtime->onMouseDown(evt.mouse.x, evt.mouse.y, MTropolis::Actions::kMouseButtonMiddle);
			break;
		case Common::EVENT_RBUTTONDOWN:
			_runtime->onMouseDown(evt.mouse.x, evt.mouse.y, MTropolis::Actions::kMouseButtonRight);
			break;
		case Common::EVENT_LBUTTONUP:
			_runtime->onMouseUp(evt.mouse.x, evt.mouse.y, MTropolis::Actions::kMouseButtonLeft);
			break;
		case Common::EVENT_MBUTTONUP:
			_runtime->onMouseUp(evt.mouse.x, evt.mouse.y, MTropolis::Actions::kMouseButtonMiddle);
			break;
		case Common::EVENT_RBUTTONUP:
			_runtime->onMouseUp(evt.mouse.x, evt.mouse.y, MTropolis::Actions::kMouseButtonRight);
			break;
		case Common::EVENT_MOUSEMOVE:
			_runtime->onMouseMove(evt.mouse.x, evt.mouse.y);
			break;
		case Common::EVENT_KEYDOWN:
		case Common::EVENT_KEYUP:
			_runtime->onKeyboardEvent(evt.type, evt.kbdRepeat, evt.kbd);
			break;

		default:
			break;
		}
	}
}

Common::Error MTropolisEngine::run() {
	int preferredWidth = 1024;
	int preferredHeight = 768;

	ColorDepthMode preferredColorDepthMode = kColorDepthMode8Bit;
	ColorDepthMode enhancedColorDepthMode = kColorDepthMode8Bit;

	Common::SharedPtr<SubtitleRenderer> subRenderer;

	if (ConfMan.getBool("subtitles"))
		subRenderer.reset(new SubtitleRenderer(ConfMan.getBool("mtropolis_mod_sound_gameplay_subtitles")));

	_runtime.reset(new Runtime(_system, _mixer, this, this, subRenderer));

	subRenderer.reset();

	Common::SharedPtr<ProjectDescription> projectDesc = bootProject(*_gameDescription);

	if (_gameDescription->gameID == GID_OBSIDIAN) {
		preferredWidth = 640;
		preferredHeight = 480;
		preferredColorDepthMode = kColorDepthMode16Bit;
		enhancedColorDepthMode = kColorDepthMode32Bit;

		HackSuites::addObsidianQuirks(*_gameDescription, _runtime->getHacks());
		HackSuites::addObsidianBugFixes(*_gameDescription, _runtime->getHacks());
		HackSuites::addObsidianSaveMechanism(*_gameDescription, _runtime->getHacks());

		if (ConfMan.getBool("mtropolis_mod_auto_save_at_checkpoints"))
			HackSuites::addObsidianAutoSaves(*_gameDescription, _runtime->getHacks(), this);

		if (ConfMan.getBool("mtropolis_mod_obsidian_widescreen")) {
			_runtime->getHacks().reportDisplaySize = Common::Point(640, 480);

			preferredHeight = 360;
			HackSuites::addObsidianImprovedWidescreen(*_gameDescription, _runtime->getHacks());
		}
	}

	if (ConfMan.getBool("mtropolis_mod_minimum_transition_duration"))
		_runtime->getHacks().minTransitionDuration = 75;

	_runtime->queueProject(projectDesc);

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

		for (const Graphics::PixelFormat &candidateFormat : pixelFormats) {
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

	// Figure out a pixel format.  First try to find one that's at least as good or better than the enhanced mode
	ColorDepthMode selectedMode = kColorDepthModeInvalid;

	for (int i = enhancedColorDepthMode; i < kColorDepthModeCount; i++) {
		if (haveExactMode[i] || haveCloseMode[i]) {
			selectedMode = static_cast<ColorDepthMode>(i);
			break;
		}
	}

	// If that fails, find one that's at least as good as the preferred mode
	if (selectedMode == kColorDepthModeInvalid) {
		for (int i = preferredColorDepthMode; i < kColorDepthModeCount; i++) {
			if (haveExactMode[i] || haveCloseMode[i]) {
				selectedMode = static_cast<ColorDepthMode>(i);
				break;
			}
		}
	}

	// If that fails, then try to find the best one available
	if (selectedMode == kColorDepthModeInvalid) {
		for (int i = preferredColorDepthMode - 1; i >= 0; i--) {
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

	ColorDepthMode fakeMode = selectedMode;
	if (selectedMode == enhancedColorDepthMode)
		fakeMode = preferredColorDepthMode;

	// Set active mode
	_runtime->switchDisplayMode(selectedMode, fakeMode);
	_runtime->setDisplayResolution(preferredWidth, preferredHeight);

	initGraphics(preferredWidth, preferredHeight, &modePixelFormats[selectedMode]);

#ifdef MTROPOLIS_DEBUG_ENABLE
	if (ConfMan.getBool("mtropolis_debug_at_start")) {
		_runtime->debugSetEnabled(true);
	}
	if (ConfMan.getBool("mtropolis_pause_at_start")) {
		_runtime->debugBreak();
	}
#endif

	while (!shouldQuit()) {
		handleEvents();

		if (!_runtime->runFrame())
			break;

		_runtime->drawFrame();
		_system->delayMillis(10);
	}

	_runtime.reset();

	return Common::kNoError;
}

void MTropolisEngine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);
}



bool MTropolisEngine::hasFeature(EngineFeature f) const {
	switch (f) {
	case kSupportsReturnToLauncher:
	case kSupportsSavingDuringRuntime:
		return true;
	default:
		return false;
	};
}

} // End of namespace MTropolis
