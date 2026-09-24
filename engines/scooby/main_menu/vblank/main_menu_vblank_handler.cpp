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

#include "main_menu_vblank_handler.h"

namespace Scooby {
const Common::Array<int> MainMenuVBlankHandler::kLightningPaletteOffsets{0x16CF8, 0xA4F8, 0xA518, 0xA538};

void MainMenuVBlankHandler::handleMainMenuVBlank() {
	// Ghidra 0x0000A38C-0x0000A49F: gate and advance the complete periodic lightning state machine.
	if ((_state.VideoFlags & kMenuLightningEnabledMask) != 0) {
		advanceLightning();
	}

	// Ghidra 0x0000A4A0-0x0000A4A3: advance every independent menu-animation output.
	_presentation.advanceMenuAnimation((_state.InitializationFlags & kMenuAnimationEnabledMask) != 0);

	// Ghidra 0x0000A4A4-0x0000A4E7: continue through the independently installed shared callback tail.
	handleInputVBlankTail();
}

void MainMenuVBlankHandler::handleInputVBlankTail() {
	// Ghidra 0x0000A4A4-0x0000A4A7: publish the next active-low host input sample.
	_input.pollControllers();

	// Ghidra 0x0000A4A8-0x0000A4BB: a held Start button sets room-script flag bit zero.
	if ((_state.ControllerOneInput & static_cast<uint8>(ControllerButtons::Start)) == 0) {
		_state.RoomScriptFlags |= 0x01;
	}

	// Ghidra 0x0000A4BC-0x0000A4CD: decrement the independent signed byte countdown through -1.
	if (_state.AuxiliaryRetraceCountdown >= 0) {
		_state.AuxiliaryRetraceCountdown--;
	}

	// Ghidra 0x0000A4CE-0x0000A4D5: release WaitForVerticalBlank's display-flag handshake.
	_state.DisplayFlags &= 0xFE;

	// Ghidra 0x0000A4D6-0x0000A4E5: decrement the caller-owned signed word countdown through -1.
	if (_state.RetraceCountdown >= 0) {
		_state.RetraceCountdown--;
	}
}

void MainMenuVBlankHandler::advanceLightning() {
	// Ghidra 0x0000A398-0x0000A425: advance an active strike or finish it and schedule the next one.
	if (_lightningDelay < 0) {
		_lightningFrameCountdown--;
		if (_lightningFrameCountdown >= 0) {
			return;
		}

		_lightningFrameCountdown = kLightningFrameCadence;
		_presentation.loadLightningPalette(
			kLightningPaletteOffsets[static_cast<std::size_t>(_lightningFrameByteOffset /
															  static_cast<int>(sizeof(uint32)))]);
		_lightningFrameByteOffset -= static_cast<int>(sizeof(uint32));
		if (_lightningFrameByteOffset >= 0) {
			return;
		}

		_presentation.restoreLightningPatch(_lightningColumn, _lightningRow, _lightningWidth,
											_lightningHeight);
		_lightningDelay = static_cast<int16>(_random.scaleNextRandomValue(kLightningDelayScale));
		return;
	}

	// Ghidra 0x0000A426-0x0000A49F: expire the random delay, publish the flash, and select one region.
	_lightningDelay--;
	if (_lightningDelay >= 0) {
		return;
	}

	_lightningFrameByteOffset = kLightningInitialFrameByteOffset;
	_lightningFrameCountdown = kLightningFrameCadence;
	_presentation.loadLightningPalette(kLightningInitialPaletteOffset);
	switch (_random.scaleNextRandomValue(3)) {
	case 0:
		activateLightningRegion(0xA59C, 22, 7, 4, 10);
		break;
	case 1:
		activateLightningRegion(0xA5EC, 27, 7, 4, 5);
		break;
	case 2:
		activateLightningRegion(0xA614, 5, 6, 4, 5);
		break;
	default:
		break;
	}
}

void MainMenuVBlankHandler::activateLightningRegion(int sourceOffset, int column, int row, int width,
													int height) {
	_lightningColumn = column;
	_lightningRow = row;
	_lightningWidth = width;
	_lightningHeight = height;
	_presentation.applyLightningPatch(sourceOffset, column, row, width, height);
}
} // namespace Scooby