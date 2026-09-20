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

#include "studio_logo_sequence.h"

#include "common/array.h"

#include "scooby/compression/ring_lz_decoder.h"
#include "scooby/graphics/genesis_asset_decoder.h"

namespace Scooby {

StudioLogoSequence::StudioLogoSequence(const ScoobyDooRom &rom, TileScene &scene,
									   FramePresenter &presenter, RaylibHost &host,
									   Common::Functor0<void> &handleRoomVBlank)
	: _handleRoomVBlank(handleRoomVBlank), _presenter(presenter), _host(host), _rom(rom),
	  _scene(scene) {
}

bool StudioLogoSequence::run() {
	_scene.reset(HorizontalDisplayMode::H32);
	Common::Array<uint8> initialTiles = decompressRingLz(_rom, 0x287C8);
	_scene.loadTiles(MakeSpan(initialTiles));
	Common::Array<uint8> stagedTilemap = decompressRingLz(_rom, 0x28220);
	Common::Array<uint8> finalTilemap = decompressRingLz(_rom, 0x2864C);
	Common::Array<PaletteColor> targetPalette = readPalette(_rom, 0x2967A);

	if (runAnimatedStep(MakeSpan(stagedTilemap), MakeSpan(targetPalette)) ==
		StepResult::HostClosed) {
		return false;
	}

	_scene.replacePaletteRange(MakeSpan(targetPalette), 0);
	_scene.loadLayerRows(MakeSpan(finalTilemap), TileLayer::Foreground, 4, 5, 25, 15);
	if (runFinalStep() == StepResult::HostClosed) {
		return false;
	}

	_scene.clearContent();
	return true;
}

StudioLogoSequence::StepResult StudioLogoSequence::runAnimatedStep(
	Span<const uint8> stagedTilemap, Span<const PaletteColor> targetPalette) {
	uploadStagedTilemapBand(stagedTilemap, 0);
	StepResult result = fadeIn(targetPalette);
	if (result != StepResult::Completed) {
		return result;
	}

	for (int cycle = 0; cycle < 4; ++cycle) {
		for (int band = 0; band < 15; ++band) {
			result = waitForFrames(2);
			if (result != StepResult::Completed) {
				return result;
			}

			uploadStagedTilemapBand(stagedTilemap, band);
		}
	}

	return StepResult::Completed;
}

StudioLogoSequence::StepResult StudioLogoSequence::runFinalStep() {
	StepResult result = waitForFrames(0xC8);
	return result == StepResult::Completed ? fadeOut() : result;
}

StudioLogoSequence::StepResult StudioLogoSequence::fadeIn(
	Span<const PaletteColor> targetPalette) {
	for (int step = 0; step < 8; ++step) {
		_scene.advancePaletteToward(targetPalette);
		StepResult result = waitForFrame();
		if (result != StepResult::Completed) {
			return result;
		}
	}

	return StepResult::Completed;
}

StudioLogoSequence::StepResult StudioLogoSequence::fadeOut() {
	StepResult result = waitForFrame();
	if (result != StepResult::Completed) {
		return result;
	}

	for (int step = 0; step < 8; ++step) {
		_scene.advancePaletteTowardBlack();
		result = waitForFrame();
		if (result != StepResult::Completed) {
			return result;
		}
	}

	return StepResult::Completed;
}

StudioLogoSequence::StepResult StudioLogoSequence::waitForFrames(int frameCounter) {
	for (int frame = 0; frame <= frameCounter; ++frame) {
		StepResult result = waitForFrame();
		if (result != StepResult::Completed) {
			return result;
		}
	}

	return StepResult::Completed;
}

StudioLogoSequence::StepResult StudioLogoSequence::waitForFrame() {
	// RunStartupSequence installs HandleRoomVBlank at 0x000008CE before
	// every logo wait reached from 0x000009AC-0x00000BB5; service that
	// still-installed callback before presenting the frame.
	_handleRoomVBlank();
	if (!_presenter.waitForVerticalBlank()) {
		return StepResult::HostClosed;
	}

	return _host.isIntroSkipPressed() ? StepResult::Skipped : StepResult::Completed;
}

void StudioLogoSequence::uploadStagedTilemapBand(Span<const uint8> source, int band) {
	const int bandByteCount = 20 * 5 * static_cast<int>(sizeof(uint16));
	_scene.loadLayerRows(source.slice(static_cast<std::size_t>(band) * static_cast<std::size_t>(bandByteCount),
									  static_cast<std::size_t>(bandByteCount)),
						 TileLayer::Foreground, 8, 8, 20, 5);
}
} // namespace Scooby