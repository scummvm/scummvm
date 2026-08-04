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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/ripper.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/savefile.h"
#include "common/system.h"

#include "engines/util.h"

#include "ripper/detection.h"
#include "ripper/console.h"
#include "ripper/cursor.h"
#include "ripper/cyber.h"
#include "ripper/input.h"
#include "ripper/inventory.h"
#include "ripper/media.h"
#include "ripper/menu.h"
#include "ripper/milestones.h"
#include "ripper/modal_dialog.h"
#include "ripper/resources.h"
#include "ripper/scene_audio.h"
#include "ripper/script.h"
#include "ripper/settings.h"
#include "ripper/toolbar.h"
#include "ripper/wac/wac.h"
#include "ripper/world_map.h"

namespace Ripper {

RipperEngine::RipperEngine(OSystem *system, const ADGameDescription *gameDescription) :
		Engine(system), _gameDescription(gameDescription), _randomSource("ripper"),
		_cursor(new CursorManager()),
		_cyber(new CyberManager(this)), _input(new InputManager(_eventMan)),
		_inventory(new Inventory(this)),
		_sceneAudio(new SceneAudioManager(this, _mixer)),
		_media(new MediaPlayer(this, _input.get(), _mixer)),
		_milestones(new Milestones()),
		_modalDialog(new ModalDialogManager(this)), _resources(new ResourceManager()),
		_scripts(new ScriptManager(this)), _settings(new RipperSettings(_mixer)),
		_toolbar(new ToolbarManager(this)),
		_wac(new WacManager(this)), _worldMap(new WorldMap(this)),
		_gameplayStarted(false), _puzzleHelpEnabled(false) {
	setDebugger(new Console(this));
}

RipperEngine::~RipperEngine() {
}

bool RipperEngine::hasFeature(EngineFeature feature) const {
	return feature == kSupportsReturnToLauncher ||
		feature == kSupportsLoadingDuringRuntime ||
		feature == kSupportsSavingDuringRuntime;
}

void RipperEngine::pauseEngineIntern(bool pause) {
	// ScummVM's global dialogs suspend this engine while its blocking media loop
	// is still on the stack. Freeze the decoder before its wall clock can advance
	// behind the dialog, then resume audio before releasing that clock.
	if (pause && _media)
		_media->pauseActiveMedia(true);
	Engine::pauseEngineIntern(pause);
	if (!pause && _media)
		_media->pauseActiveMedia(false);
}

void RipperEngine::registerSearchPaths() {
	const Common::FSNode gamePath(ConfMan.getPath("path"));
	static const char *const directories[] = {
		"scene",
		"puzzle",
		"combat",
		"cyber"
	};

	for (uint i = 0; i < ARRAYSIZE(directories); ++i) {
		SearchMan.addSubDirectoryMatching(gamePath, directories[i]);
		debugC(2, kDebugResources, "Ripper: registered search directory '%s'", directories[i]);
	}
}

void RipperEngine::pumpEvents() {
	if (_input->pollEvents()) {
		debugC(1, kDebugGeneral, "Ripper: received quit event");
		quitGame();
	}
}

bool RipperEngine::selectRandomRipperIdentity(const char *source) {
	const uint candidateCount =
		kMilestoneLastRipperIdentity - kMilestoneFirstRipperIdentity + 1;
	return _milestones->selectRipperIdentity(
		_randomSource.getRandomNumber(candidateCount - 1), source);
}

Common::Error RipperEngine::run() {
	registerSearchPaths();
	initGraphics(640, 400);
	_settings->load();
	if (!_resources->initialize())
		return Common::kReadingFailed;
	if (!_milestones->initialize(*_resources))
		return Common::kReadingFailed;
	// Retail initializes a provisional identity before the front end; restoring
	// a save later replaces it with the identity serialized in that game.
	if (!selectRandomRipperIdentity("startup-initialization"))
		return Common::kUnknownError;
	if (!_modalDialog->initialize(*_resources))
		return Common::kReadingFailed;
	if (!_cursor->initialize(*_resources))
		return Common::kReadingFailed;
	if (!_inventory->initialize(*_resources))
		return Common::kReadingFailed;
	if (!_scripts->initialize(*_resources))
		return Common::kReadingFailed;
	if (!_toolbar->initialize(*_resources))
		return Common::kReadingFailed;
	if (!_wac->initialize(*_resources))
		return Common::kReadingFailed;
	if (!_worldMap->initialize(*_resources))
		return Common::kReadingFailed;

	_cursor->setVisible(false);
	const bool skipIntro = ConfMan.hasKey("skip_intro") && ConfMan.getBool("skip_intro");
	if (skipIntro) {
		debugC(1, kDebugVideo,
			"Ripper: skipping startup presentation logo.avi by game option");
	} else {
		debugC(1, kDebugVideo,
			"Ripper: startup presentation logo.avi from RunGameStartupAndMainLoop at 0x100c2");
		if (!_media->play("logo.avi", true))
			return shouldQuit() ? Common::kNoError : Common::kUnknownError;
	}

	bool startGameplay = false;
	const int launcherSaveSlot = ConfMan.getInt("save_slot");
	if (launcherSaveSlot >= 0) {
		debugC(1, kDebugSaveLoad,
			"Ripper: launcher requested restore slot=%d", launcherSaveSlot);
		const Common::Error loadError = loadGameState(launcherSaveSlot);
		if (loadError.getCode() != Common::kNoError)
			return loadError;
		startGameplay = true;
	}

	while (!shouldQuit() && !startGameplay) {
		MainMenu menu(this);
		switch (menu.run()) {
		case kMainMenuNewGame:
			debugC(1, kDebugGeneral, "Ripper: startup menu begins a new game");
			if (!selectRandomRipperIdentity("new-game-initialization"))
				return Common::kUnknownError;
			startGameplay = true;
			break;
		case kMainMenuContinue:
			// RunGameStartupAndMainLoop at 0x100c2 restores its dedicated emergency
			// save directly instead of entering the manual slot chooser.
			if (!_saveFileMan->exists(getSaveStateName(getAutosaveSlot()))) {
				debugC(1, kDebugSaveLoad,
					"Ripper: Continue requested without emergency slot=%d",
					getAutosaveSlot());
				break;
			}
			if (loadGameState(getAutosaveSlot()).getCode() == Common::kNoError) {
				debugC(1, kDebugSaveLoad,
					"Ripper: startup Continue restored emergency slot=%d",
					getAutosaveSlot());
				startGameplay = true;
			}
			break;
		case kMainMenuLoadGame:
			debugC(1, kDebugSaveLoad,
				"Ripper: startup Restore Game entering manual slot chooser");
			if (loadGameDialog()) {
				debugC(1, kDebugSaveLoad,
					"Ripper: startup Restore Game loaded manual slot");
				startGameplay = true;
			}
			break;
		case kMainMenuViewIntro:
			debugC(1, kDebugVideo,
				"Ripper: startup menu plays proint.avi from RunGameStartupAndMainLoop at 0x100c2");
			if (!_media->play("proint.avi", true))
				return shouldQuit() ? Common::kNoError : Common::kUnknownError;
			break;
		case kMainMenuQuit:
			debugC(1, kDebugGeneral, "Ripper: startup menu exits the game");
			quitGame();
			break;
		}
	}

	if (shouldQuit())
		return Common::kNoError;
	if (!_gameplayStarted) {
		if (!_scripts->runStartupPath()) {
			_scripts->logRuntimeFailure("startup scene path failed");
			return Common::kUnknownError;
		}
		_gameplayStarted = true;
	}

	while (!shouldQuit()) {
		pumpEvents();
		if (!_scripts->serviceScene()) {
			_scripts->logRuntimeFailure("scene service failed");
			return Common::kUnknownError;
		}
		_system->updateScreen();
		_system->delayMillis(10);
	}
	if (_scripts->canSaveGame()) {
		debugC(1, kDebugSaveLoad,
			"Ripper: scene loop exit writing emergency Continue slot=%d",
			getAutosaveSlot());
		const Common::Error saveError = saveGameState(
			getAutosaveSlot(), "Continue", true);
		if (saveError.getCode() != Common::kNoError)
			warning("Ripper: unable to write emergency Continue save");
	}

	return Common::kNoError;
}

} // End of namespace Ripper
