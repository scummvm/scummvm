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

#ifndef SCOOBY_SOUND_TEST_CONTROLLER_H
#define SCOOBY_SOUND_TEST_CONTROLLER_H

#include "common/scummsys.h"

#include "common/func.h"

#include "scooby/input/controller_input.h"
#include "scooby/main_menu/main_menu_presentation.h"
#include "scooby/runtime/runtime_state.h"
#include "sound_test_catalog.h"

// Owns the sound-test selection, presentation, input cadence, and return path recovered from
// RunSoundTestMenu at 0x0000940C. Audio calls remain an explicit silent handover.

namespace Scooby {

class SoundTestController {
public:
	// Binds the recovered loop to its catalog, menu scene, input state, and host handovers.
	// catalog: decoded labels and managed command mapping for all 87 selections.
	// presentation: logical owner of menu composition and indexed text writes.
	// input: controller sampler whose semantic Back binding is scoped to this menu.
	// state: shared controller and signed-retrace state used by the installed callback.
	// waitForVerticalBlank: host wait that invokes the installed main-menu callback once.
	// initializeAudioDriver: silent audio-driver boundary retained at each original call site.
	// playAudioCommand: silent audio-command boundary retained at each original call site.
	SoundTestController(const SoundTestCatalog &catalog, MainMenuPresentation &presentation,
						ControllerInput &input, RuntimeState &state,
						Common::Functor0<bool> &waitForVerticalBlank,
						Common::Functor0<void> &initializeAudioDriver,
						Common::Functor1<int, void> &playAudioCommand)
		: _catalog(catalog), _presentation(presentation), _input(input), _state(state),
		  _waitForVerticalBlank(waitForVerticalBlank), _initializeAudioDriver(initializeAudioDriver),
		  _playAudioCommand(playAudioCommand) {
	}

	// Runs the complete recovered sound-test loop until C returns to the main-menu options.
	// Returns false when the host closes during an original retrace wait.
	//
	// Ghidra: RunSoundTestMenu (0x0000940C-0x000096F3).
	bool run();

private:
	static const int kDisplayRow = 0x11;
	static const int kGroupCount = 6;
	static const int kIndicesPerFullGroup = 0x10;
	static const int kLastGroupIndexCount = 7;
	static const int kSceneAssetOffset = 0x18C72;
	static const int kSelectionColumn = 15;
	static const int kSelectionRow = 20;
	static const int kUpdateCountdown = 4;

	// Original executable character lookup at 0x000093FC-0x0000940B.
	static const char kHexDigits[17];

	bool runLoop();
	bool processInput(int &group, int &index, uint8 previousInput, int displayedSelection);
	static void normalizeSelection(int &group, int &index);
	bool wasPressed(ControllerButtons button, uint8 previousInput) const;
	void consumeInputEdge();
	bool waitForPendingCommit();
	void composeAnimatedTiles();

	const SoundTestCatalog &_catalog;
	MainMenuPresentation &_presentation;
	ControllerInput &_input;
	RuntimeState &_state;
	Common::Functor0<bool> &_waitForVerticalBlank;
	Common::Functor0<void> &_initializeAudioDriver;
	Common::Functor1<int, void> &_playAudioCommand;
};
} // namespace Scooby

#endif // SCOOBY_SOUND_TEST_CONTROLLER_H
