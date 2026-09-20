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

#include "sound_test_controller.h"

#include "common/scummsys.h"

namespace Scooby {

const char SoundTestController::kHexDigits[17] = "0123456789ABCDEF";

bool SoundTestController::run() {
	ControllerButtons previousBackButton = _input._backButton;
	_input._backButton = ControllerButtons::C;
	bool result = runLoop();
	_input._backButton = previousBackButton;
	_presentation.setTextPriority(false);
	return result;
}

bool SoundTestController::runLoop() {
	// Ghidra 0x0000940C-0x0000943B: retain the prior input, current animation, audio reset, and scene asset.
	uint8 previousInput = _state.PreviousControllerOneInput;
	composeAnimatedTiles();
	_initializeAudioDriver();
	_presentation.stageByteRleData(kSceneAssetOffset);
	_presentation.setTextPriority(true);
	int group = 0;
	int index = 0;
	int displayedSelection = -1;

	// Ghidra 0x0000943C-0x00009468: drain the prior frame, publish the sound-test plane, and commit tiles.
	if (!waitForPendingCommit()) {
		return false;
	}

	_state.RetraceCountdown = kUpdateCountdown;
	_presentation.commitStagedMenuPlane();
	_presentation.commitMenuSceneBuffers();

	while (true) {
		// Ghidra 0x0000946C-0x0000947B: compose one animated menu frame without consuming selection state.
		composeAnimatedTiles();
		while (_state.RetraceCountdown >= 0) {
			// Ghidra 0x0000947C-0x00009667: update selection, preserve audio call sites, or take C's exit.
			if (processInput(group, index, previousInput, displayedSelection)) {
				_presentation.setTextPriority(false);
				if (!waitForPendingCommit()) {
					return false;
				}

				_presentation.commitMenuSceneBuffers();
				_initializeAudioDriver();
				_playAudioCommand(_state.ResumeAudioCommand);
				return true;
			}

			// Ghidra 0x00009668-0x0000967B: retain this sample while the callback advances the countdown.
			previousInput = _state.ControllerOneInput;
			if (!_waitForVerticalBlank()) {
				return false;
			}
		}

		// Ghidra 0x0000967C-0x00009692: publish the completed animation frame and detect a new selection.
		_state.RetraceCountdown = kUpdateCountdown;
		_presentation.commitMenuSceneBuffers();
		int selection = group * kIndicesPerFullGroup + index;
		if (displayedSelection != selection) {
			displayedSelection = selection;

			// Ghidra 0x00009694-0x000096C3: select the NUL row, synchronize, clear, and center its name.
			if (!_waitForVerticalBlank()) {
				return false;
			}

			_presentation.writeBlankTileRow(1, kDisplayRow);
			_presentation.writeCenteredTileRow(_catalog.getName(selection), kDisplayRow);
		}

		// Ghidra 0x000096C4-0x000096EF: write the two hexadecimal selector cells at C51E and C520.
		_presentation.writeIndexedTile(
			static_cast<uint8>(kHexDigits[static_cast<std::size_t>(group)]),
			kSelectionColumn, kSelectionRow);
		_presentation.writeIndexedTile(
			static_cast<uint8>(kHexDigits[static_cast<std::size_t>(index)]),
			kSelectionColumn + 1, kSelectionRow);
	}
}

bool SoundTestController::processInput(int &group, int &index, uint8 previousInput,
									   int displayedSelection) {
	if (wasPressed(ControllerButtons::Up, previousInput)) {
		consumeInputEdge();
		group++;
	}

	if (wasPressed(ControllerButtons::Down, previousInput)) {
		consumeInputEdge();
		group--;
	}

	if (wasPressed(ControllerButtons::Right, previousInput)) {
		consumeInputEdge();
		index++;
	}

	if (wasPressed(ControllerButtons::Left, previousInput)) {
		consumeInputEdge();
		index--;
	}

	normalizeSelection(group, index);
	if (wasPressed(ControllerButtons::B, previousInput)) {
		consumeInputEdge();
		_initializeAudioDriver();
		_playAudioCommand(_catalog.getAudioCommand(displayedSelection));
	}

	if (wasPressed(ControllerButtons::A, previousInput)) {
		consumeInputEdge();
		_initializeAudioDriver();
	}

	return wasPressed(ControllerButtons::C, previousInput);
}

void SoundTestController::normalizeSelection(int &group, int &index) {
	if (group == kGroupCount - 1) {
		if (index < 0) {
			group--;
			index &= kIndicesPerFullGroup - 1;
		} else if (index == kLastGroupIndexCount) {
			group = 0;
			index = 0;
		}
	} else if (index < 0) {
		group--;
		index &= kIndicesPerFullGroup - 1;
	} else if (index == kIndicesPerFullGroup) {
		index &= kIndicesPerFullGroup - 1;
		group++;
	}

	if (group < 0) {
		group = kGroupCount - 1;
	} else if (group >= kGroupCount) {
		group = 0;
	}

	if (group == kGroupCount - 1 && index >= kLastGroupIndexCount) {
		index = kLastGroupIndexCount - 1;
	}
}

bool SoundTestController::wasPressed(ControllerButtons button, uint8 previousInput) const {
	uint8 mask = static_cast<uint8>(button);
	return (_state.ControllerOneInput & mask) == 0 &&
		   ((previousInput & mask) != 0 || (_state.ControllerOneAccumulatedChanges & mask) != 0);
}

void SoundTestController::consumeInputEdge() {
	_state.ControllerOneAccumulatedChanges = 0;
	_state.ControllerOneChangeBaseline = _state.ControllerOneInput;
}

bool SoundTestController::waitForPendingCommit() {
	while (_state.RetraceCountdown >= 0) {
		if (!_waitForVerticalBlank()) {
			return false;
		}
	}

	return true;
}

void SoundTestController::composeAnimatedTiles() {
	_presentation.composeWideMenuRevealTiles(_presentation.animatedRowMaskOffset(), 0);
	_presentation.composeNarrowMenuRevealTiles(_presentation.animatedRowMaskOffset(), 0);
}
} // namespace Scooby