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

#include "room_script_action_05_or_0c_executor.h"

#include "common/scummsys.h"

#include "scooby/common/span.h"
#include "scooby/graphics/genesis_asset_decoder.h"

namespace Scooby {

const Common::Array<PaletteColor> RoomScriptAction05Or0CExecutor::kBlankPalette(
	RoomScriptAction05Or0CExecutor::kPaletteColorCount);

void RoomScriptAction05Or0CExecutor::executeRoomScriptAction05Or0C() {
	// Ghidra 0x000045A4-0x000045B1: preserve the progress-bit-one early-return branch.
	if ((_state.ProgressStateBytes[0] & 0x02) != 0) {
		return;
	}

	// Ghidra 0x000045B2-0x000045C3: set progress bit one, then preserve Action0C's direct branch to the
	// separate Action1D owner instead of executing any Action05 presentation work.
	_state.ProgressStateBytes[0] |= 0x02;
	if (_rom.readUInt16(_state.RoomScriptStartOffset + static_cast<int>(sizeof(uint16))) ==
		kShortcutAction) {
		_executeRoomScriptAction1D();
		return;
	}

	// Ghidra 0x000045C4-0x000045D1: managed locals replace the saved register set; retain the full-byte
	// actor-upload wait rather than narrowing it to an authored actor mask.
	while (_state.ActorTileUploadPendingFlags != 0) {
		if (!_waitForRoomVerticalBlank()) {
			return;
		}
	}

	// Ghidra 0x000045D2-0x000045E7: close the room-update gate, wait one retrace, and replace the original
	// 128 hardware writes wrapping across 64-word CRAM with their logical all-colors-black result.
	_state.DisplayFlags &= static_cast<uint8>(~kRoomUpdateEnabledMask);
	if (!_waitForRoomVerticalBlank()) {
		return;
	}

	_scene.replacePaletteRange(MakeSpan(kBlankPalette), 0);
	int16 savedCameraX = _state.CameraX;
	int16 savedCameraY = _state.CameraY;
	{
		// Ghidra 0x000045E8-0x0000461D: the scoped logical-content snapshot replaces the complete 0x2200-word
		// VRAM readback and restore. Construction/destruction order below reproduces the C# "using (...) {
		// try { } finally { } }" nesting exactly: the finally-equivalent camera restore below is constructed
		// after this content guard, so it destructs first (matching finally running before Dispose) for the
		// fall-through path and for every early "return" inside this scope.
		TileScene::ContentReplacementGuard contentGuard = _scene.beginTemporaryContentReplacement();

		struct CameraRestoreGuard {
			RuntimeState &State;
			int16 SavedX;
			int16 SavedY;

			~CameraRestoreGuard() {
				// Ghidra 0x00004720-0x0000472F: restore camera Y then X while native interrupts are masked.
				State.CameraY = SavedY;
				State.CameraX = SavedX;
			}
		} cameraRestoreGuard{_state, savedCameraX, savedCameraY};

		// Ghidra 0x0000461E-0x0000462D: overwrite exactly 15,422 packed bytes. The absent final two bytes of
		// tile 0x1E1 retain their prior pixels even though the authored maps stop at tile 0x1E0.
		_scene.overwritePackedPatternBytes(_rom.readBytes(kPatternBytesOffset, kPatternByteCount), 0);

		// Ghidra 0x0000462E-0x00004651: save and zero both cameras; new complete layer arrays replace the two
		// hardware plane clears.
		_state.CameraX = 0;
		_state.CameraY = 0;
		Common::Array<uint8> foregroundPlane(static_cast<std::size_t>(kPlaneColumns) *
											 static_cast<std::size_t>(kPlaneRows) * sizeof(uint16));
		Common::Array<uint8> backgroundPlane(foregroundPlane.size());

		// Ghidra 0x00004652-0x0000468D: copy the 64x19 foreground map into rows 1-19. For each of 19
		// background rows, preserve both original writes by copying its 32-cell source into each half.
		Span<const uint8> foregroundMap = _rom.readBytes(
			kForegroundMapOffset, kPlaneColumns * kForegroundMapRows * static_cast<int>(sizeof(uint16)));
		for (std::size_t i = 0; i < foregroundMap.size(); ++i) {
			foregroundPlane[static_cast<std::size_t>(kPlaneRowBytes) + i] = foregroundMap[i];
		}

		Span<const uint8> backgroundHalfMap =
			_rom.readBytes(kBackgroundHalfMapOffset, kBackgroundHalfRowBytes * kBackgroundMapRows);
		for (int row = 0; row < kBackgroundMapRows; row++) {
			Span<const uint8> sourceRow =
				backgroundHalfMap.slice(static_cast<std::size_t>(row * kBackgroundHalfRowBytes),
										static_cast<std::size_t>(kBackgroundHalfRowBytes));
			std::size_t destinationOffset = static_cast<std::size_t>((row + 1) * kPlaneRowBytes);
			for (std::size_t i = 0; i < sourceRow.size(); ++i) {
				backgroundPlane[destinationOffset + i] = sourceRow[i];
				backgroundPlane[destinationOffset + static_cast<std::size_t>(kBackgroundHalfRowBytes) + i] =
					sourceRow[i];
			}
		}

		// Plane publication preserves the inherited H32 mode; the 64-column backing extent exists for the
		// original 256-pixel camera toggle and does not widen the visible viewport to H40.
		_scene.replaceCompleteLayers(MakeSpan(backgroundPlane), MakeSpan(foregroundPlane),
									 kPlaneColumns, kPlaneRows);

		// Ghidra 0x0000468E-0x000046B7: stage all 64 authored colors. The content scope suppresses sprites
		// while replacing the original zeroed first sprite-table entry.
		Common::Array<PaletteColor> animatedPalette =
			readPalette(_rom, kPaletteOffset, kPaletteColorCount);

		// Ghidra 0x000046B8-0x00004715: retain both independent active-low button exits, all 201 DBF
		// iterations, the every-fourth-iteration camera bit toggle, every retrace, and each 8-word rotate.
		int cameraToggleCounter = 3;
		for (int frameCounter = 0xC8; frameCounter >= 0; frameCounter--) {
			if ((_state.ControllerOneInput & kAButtonMask) == 0) {
				break;
			}

			if ((_state.ControllerOneInput & kBButtonMask) == 0) {
				break;
			}

			cameraToggleCounter--;
			if (cameraToggleCounter < 0) {
				cameraToggleCounter = 3;
				_state.CameraX = static_cast<int16>(_state.CameraX ^ 0x0100);
			}

			if (!_waitForRoomVerticalBlank()) {
				return;
			}

			_scene.replacePaletteRange(MakeSpan(animatedPalette), 0);
			PaletteColor firstAnimatedColor = animatedPalette[40];
			for (int color = 40; color < 47; color++) {
				animatedPalette[static_cast<std::size_t>(color)] =
					animatedPalette[static_cast<std::size_t>(color + 1)];
			}

			animatedPalette[47] = firstAnimatedColor;
		}

		// Ghidra 0x00004716-0x0000471F: blank the temporary palette before room restoration.
		_scene.replacePaletteRange(MakeSpan(kBlankPalette), 0);

		// cameraRestoreGuard and contentGuard fall out of scope here, in that order, reproducing the C#
		// finally-then-dispose sequence.
	}

	// Ghidra 0x00004730-0x0000475B: scope disposal replaces the pattern snapshot write, logical plane,
	// scrolling, and sprite restoration; retain the explicit viewport redraw before native register return.
	_tileStreamer.drawRoomViewport();

	// Ghidra 0x0000475C-0x0000476B: preserve both consecutive writes that reopen room updates.
	_state.DisplayFlags |= kRoomUpdateEnabledMask;
	_state.DisplayFlags |= kRoomUpdateEnabledMask;

	// Ghidra 0x0000476C-0x0000477B: update actor animation, request interaction refresh through bit one, and
	// call its separate canonical function boundary.
	_updateActorAnimationFrames();
	_state.InteractionFlags |= kInteractionRefreshMask;
	_refreshInteractionDisplay();

	// Ghidra 0x0000477C-0x00004781: D0=5 gives the original WaitForFrames six retraces.
	if (!waitForFrames(5)) {
		return;
	}

	// Ghidra 0x00004782-0x000047A5: close room updates for one final retrace, republish all 64 room palette
	// colors, and reopen the gate.
	_state.DisplayFlags &= static_cast<uint8>(~kRoomUpdateEnabledMask);
	if (!_waitForRoomVerticalBlank()) {
		return;
	}

	_scene.replacePaletteRange(MakeSpan(_state.TargetPalette), 0);
	_state.DisplayFlags |= kRoomUpdateEnabledMask;

	// Ghidra 0x000047A6-0x000047AB: both completed branches converge on the separate Action1D owner.
	_executeRoomScriptAction1D();
}

bool RoomScriptAction05Or0CExecutor::waitForFrames(int frameCounter) {
	for (int frame = 0; frame <= frameCounter; frame++) {
		if (!_waitForRoomVerticalBlank()) {
			return false;
		}
	}

	return true;
}
} // namespace Scooby