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

#include "episode_opening_sequence.h"

#include "common/algorithm.h"
#include "common/scummsys.h"

#include "common/array.h"

#include "scooby/compression/byte_rle_decoder.h"
#include "scooby/compression/ring_lz_decoder.h"
#include "scooby/graphics/genesis_asset_decoder.h"
#include "scooby/graphics/palette_color.h"

namespace Scooby {

const Common::Array<int8> EpisodeOpeningSequence::kScrollDelays{0};
const Common::Array<int16> EpisodeOpeningSequence::kVerticalPhases{0, 1, 1, 1, 2, 2, 2, 1, 1, 1, 0};

bool EpisodeOpeningSequence::runEpisodeOpeningSequence() {
	// Ghidra 0x000013BA-0x0000140D: preserve machine state, install callback entry 0xA4A4, and retain the
	// startup-selected 256-pixel H32 viewport over a 128-cell-wide logical plane without retaining VDP state.
	int episodeAssetRecord = kEpisodeAssetTableOffset + _state.EpisodeIndex * kEpisodeAssetRecordSize;

	// Ghidra 0x0000140E-0x00001475: decode the complete pattern set and both 128x28 plane maps; VRAM
	// destinations E000 and C000 become logical background and foreground ownership.
	_scene.loadTiles(MakeSpan(
		decompressRingLz(
			_rom, static_cast<int>(_rom.readUInt32(episodeAssetRecord)))));
	Common::Array<uint8> backgroundCells = decompressRingLz(
		_rom, static_cast<int>(_rom.readUInt32(episodeAssetRecord + kBackgroundAssetField)));
	Common::Array<uint8> foregroundCells = decompressRingLz(
		_rom, static_cast<int>(_rom.readUInt32(episodeAssetRecord + kForegroundAssetField)));
	_scene.replaceCompleteLayers(MakeSpan(backgroundCells), MakeSpan(foregroundCells),
								 kLayerColumns,
								 kLayerRows);

	// Ghidra 0x00001476-0x000014B7: follow six relative descriptors and publish each exact 60-tile
	// byte-RLE animation bank at its authored pattern index.
	int spriteOffsetTable =
		kOpeningSpriteDescriptorOffset + _rom.readUInt16(
											 kOpeningSpriteDescriptorOffset + kSpriteDescriptorOffsetTableField);
	for (int streamIndex = 0; streamIndex < kOpeningSpriteStreamCount; streamIndex++) {
		int descriptorOffset =
			kOpeningSpriteDescriptorOffset + _rom.readUInt16(
												 spriteOffsetTable + streamIndex * static_cast<int>(sizeof(uint16)));
		int streamOffset =
			kOpeningSpriteDescriptorOffset + static_cast<int>(_rom.readUInt32(
												 descriptorOffset + kSpriteDescriptorDataField));
		Common::Array<uint8> streamTiles = decompressByteRle(
			_rom, streamOffset);
		_scene.loadTilesAt(MakeSpan(streamTiles),
						   kSpriteTileBankStart + streamIndex * kAnimationTileBankStride);
	}

	// Ghidra 0x000014B8-0x000014E1: publish the first sprite table, clear the two observable next-frame
	// line-scroll projections within the larger workspace, and reset both plane displacements instead of VSRAM.
	writeEpisodeOpeningSpriteTable();
	Common::fill(_sequenceState.ForegroundLineHorizontalOffsets.begin(),
				 _sequenceState.ForegroundLineHorizontalOffsets.end(), 0);
	Common::fill(_sequenceState.BackgroundLineHorizontalOffsets.begin(),
				 _sequenceState.BackgroundLineHorizontalOffsets.end(), 0);
	_scene.setVerticalOffsets(0, 0);

	// Ghidra 0x000014E2-0x0000152D: initialize every opening-only scroll and sprite global before the first
	// logical per-line scroll update. Process-zeroed low words remain zero where the original writes only a word.
	_sequenceState.ScrollBandBase = 0;
	_sequenceState.ScrollAccumulators[1] = 0;
	_sequenceState.ScrollAccumulators[3] = 0;
	_sequenceState.ScrollAccumulators[5] = 0;
	_sequenceState.ScrollAccumulators[7] &= 0xFFFF;
	_sequenceState.SpriteYBase = 0x9F;
	_sequenceState.HorizontalOffset = -0x9E;
	_sequenceState.AnimationCountdown = 1;
	_sequenceState.AnimationTileOffset = 0;
	_sequenceState.LeadingSpriteXOffset = 0;
	_sequenceState.VerticalPhase = 0;
	updateEpisodeOpeningScene();

	// Ghidra 0x0000152E-0x00001575: assemble the 64-color target, overlay default shared colors 1-30,
	// force episode one's color zero black, and replace the hardware interrupt-level handover.
	Common::Array<PaletteColor> openingPalette = readPalette(
		_rom, static_cast<int>(_rom.readUInt32(episodeAssetRecord + kOpeningPaletteField)));
	Common::copy(openingPalette.begin(), openingPalette.end(), _state.TargetPalette.begin());
	Common::Array<PaletteColor> sharedOverlay =
		readPalette(_rom, kDefaultSharedPaletteOffset,
					kSharedPaletteOverlayCount);
	Common::copy(sharedOverlay.begin(), sharedOverlay.end(), _state.TargetPalette.begin() + 1);
	if (_state.EpisodeIndex == 0) {
		_state.TargetPalette[0] = PaletteColor();
	}

	// Ghidra 0x00001576-0x000015A5: publish initial scroll and sprites, preserve both silent audio commands,
	// and fade through callback-aware retraces.
	updateEpisodeOpeningScene();
	writeEpisodeOpeningSpriteTable();
	playAudioCommand(0x7C);
	playAudioCommand(0x1E);
	if (!_presenter.fadePaletteIn(MakeSpan(_state.TargetPalette), &_invokeOpeningVBlankFunctor)) {
		return false;
	}

	// Ghidra 0x000015A6-0x00001629, including inline data at 0x0000160C-0x00001623: reset and advance
	// the shared phase-table cursors and their one-entry zero-delay sequence.
	_state.ScrollAnimationPhaseOffset = 0;
	_state.ScrollAnimationDelayCursor = 0;
	_state.ScrollAnimationDelay = 0;
	while (true) {
		if (!waitForOpeningVerticalBlank()) {
			return false;
		}

		advanceVerticalPhase();

		// Ghidra 0x0000162A-0x0000167B: update scroll and sprites, alternate 60-tile animation banks every
		// two frames, advance horizontal travel, and exit on a changed active-low action or offset 0x10E.
		updateEpisodeOpeningScene();
		writeEpisodeOpeningSpriteTable();
		_sequenceState.AnimationCountdown--;
		if (_sequenceState.AnimationCountdown < 0) {
			_sequenceState.AnimationTileOffset ^= kAnimationTileBankStride;
			_sequenceState.AnimationCountdown = 1;
			_sequenceState.HorizontalOffset++;
		}

		uint8 currentActionInput = static_cast<uint8>(_state.ControllerOneInput & 0xF0);
		uint8 previousActionInput = static_cast<uint8>(_state.PreviousControllerOneInput & 0xF0);
		if ((currentActionInput != 0xF0 && currentActionInput != previousActionInput) ||
			_sequenceState.HorizontalOffset == 0x10E) {
			break;
		}
	}

	// Ghidra 0x0000167C-0x000016C3: fade to black, restore shared palette color zero, and collapse VDP and
	// callback restoration into the next managed scene owner's handover.
	if (!_presenter.fadePaletteOut(&_invokeOpeningVBlankFunctor)) {
		return false;
	}

	_state.TargetPalette[0] = readPalette(_rom, 0x32480, 1)[0];
	return true;
}

void EpisodeOpeningSequence::advanceVerticalPhase() {
	if (_state.ScrollAnimationDelay != 0) {
		_state.ScrollAnimationDelay--;
		return;
	}

	_sequenceState.VerticalPhase =
		kVerticalPhases[static_cast<std::size_t>(_state.ScrollAnimationPhaseOffset / static_cast<int>(sizeof(
																						 int16)))];
	_state.ScrollAnimationPhaseOffset += static_cast<int>(sizeof(int16));
	if (_state.ScrollAnimationPhaseOffset ==
		static_cast<int>(kVerticalPhases.size()) * static_cast<int>(sizeof(int16))) {
		_state.ScrollAnimationPhaseOffset = 0;
	}

	if (_state.ScrollAnimationPhaseOffset != 0) {
		return;
	}

	_state.ScrollAnimationDelay = kScrollDelays[static_cast<std::size_t>(_state.ScrollAnimationDelayCursor)];
	_state.ScrollAnimationDelayCursor++;
	if (_state.ScrollAnimationDelayCursor == static_cast<int>(kScrollDelays.size())) {
		_state.ScrollAnimationDelayCursor = 0;
	}
}

void EpisodeOpeningSequence::invokeOpeningVBlank() {
	_state.DisplayFlags |= 0x01;
	_inputVBlank.handleInputVBlankTail();
}

void EpisodeOpeningSequence::updateEpisodeOpeningScene() {
	// Ghidra 0x000017AE-0x000017E9: publish 224 prepared plane-A and plane-B words to the interleaved
	// horizontal-scroll table; interrupt, VDP increment, DMA, and VRAM addresses become one logical copy.
	_scene.setLineHorizontalOffsets(MakeSpan(_sequenceState.ForegroundLineHorizontalOffsets),
									MakeSpan(_sequenceState.BackgroundLineHorizontalOffsets));

	// Ghidra 0x000017EA-0x00001803: switch the background's static upper band to -256 once horizontal
	// travel is nonnegative, then select the episode-specific accumulator and raster-band layout.
	if (_sequenceState.HorizontalOffset >= 0) {
		_sequenceState.ScrollBandBase = -0x100;
	}

	Common::Array<int32> &scrollAccumulators = _sequenceState.ScrollAccumulators;
	Span<int16> foreground = MakeSpan(_sequenceState.ForegroundLineHorizontalOffsets);
	Span<int16> background = MakeSpan(_sequenceState.BackgroundLineHorizontalOffsets);
	if (_state.EpisodeIndex != 0) {
		// Ghidra 0x00001804-0x000018F1: advance eight signed 16.16 values and fill all 224 episode-two
		// lines through the exact inclusive DBF extents before returning.
		scrollAccumulators[7] -= 0x8000;
		scrollAccumulators[0] -= 0xC000;
		scrollAccumulators[1] -= 0x10000;
		scrollAccumulators[2] -= 0x10000;
		scrollAccumulators[3] -= 0x18000;
		scrollAccumulators[4] -= 0x20000;
		scrollAccumulators[5] -= 0x30000;
		scrollAccumulators[6] -= 0x40000;
		background.slice(0, 104).fill(_sequenceState.ScrollBandBase);
		background.slice(104, 32).fill(static_cast<int16>(scrollAccumulators[7] >> 16));
		background.slice(136, 25).fill(static_cast<int16>(scrollAccumulators[1] >> 16));
		background.slice(161, 15).fill(static_cast<int16>(scrollAccumulators[4] >> 16));
		background.slice(176).fill(static_cast<int16>(scrollAccumulators[5] >> 16));
		foreground.slice(0, 40).fill(static_cast<int16>(scrollAccumulators[0] >> 16));
		foreground.slice(40, 16).fill(static_cast<int16>(scrollAccumulators[2] >> 16));
		foreground.slice(56, 16).fill(static_cast<int16>(scrollAccumulators[3] >> 16));
		foreground.slice(72).fill(static_cast<int16>(scrollAccumulators[6] >> 16));
		return;
	}

	// Ghidra 0x000018F2-0x0000197D: advance four signed 16.16 values and fill all 224 episode-one lines
	// through its two background and three foreground inclusive DBF extents.
	scrollAccumulators[0] -= 0x8000;
	scrollAccumulators[1] -= 0x10000;
	scrollAccumulators[2] -= 0x20000;
	scrollAccumulators[3] -= 0x40000;
	background.slice(0, 80).fill(_sequenceState.ScrollBandBase);
	background.slice(80).fill(static_cast<int16>(scrollAccumulators[2] >> 16));
	foreground.slice(0, 96).fill(static_cast<int16>(scrollAccumulators[0] >> 16));
	foreground.slice(96, 48).fill(static_cast<int16>(scrollAccumulators[1] >> 16));
	foreground.slice(144).fill(static_cast<int16>(scrollAccumulators[3] >> 16));
}

bool EpisodeOpeningSequence::waitForOpeningVerticalBlank() {
	invokeOpeningVBlank();
	return _presenter.waitForVerticalBlank();
}

void EpisodeOpeningSequence::writeEpisodeOpeningSpriteTable() {
	// Ghidra 0x000016C4-0x0000175B: replace interrupt masking and the FC00 VDP write cursor with one
	// complete logical table, retain each packed word calculation, and terminate through the fixed extent.
	Common::Array<SpriteInstance> sprites(kSpriteTableSize);
	Span<SpriteInstance> spriteSpan = MakeSpan(sprites);
	writeEpisodeOpeningSpriteGroup(
		spriteSpan.slice(0, kSpriteGroupSize),
		static_cast<int16>(_sequenceState.LeadingSpriteXOffset + 0x7C + _sequenceState.HorizontalOffset),
		static_cast<int16>(_sequenceState.VerticalPhase + 0x6F + _sequenceState.SpriteYBase),
		kSpriteTileBankStart);
	writeEpisodeOpeningSpriteGroup(
		spriteSpan.slice(static_cast<std::size_t>(kSpriteGroupSize), kSpriteGroupSize),
		static_cast<int16>(_sequenceState.HorizontalOffset + 0x80),
		static_cast<int16>(_sequenceState.SpriteYBase + 0x80),
		kSpriteTileBankStart + kAnimationTileBankStride + _sequenceState.AnimationTileOffset);
	writeEpisodeOpeningSpriteGroup(
		spriteSpan.slice(static_cast<std::size_t>(kSpriteGroupSize) * 2, kSpriteGroupSize),
		static_cast<int16>(_sequenceState.HorizontalOffset + 0xCA),
		static_cast<int16>(_sequenceState.SpriteYBase + 0x79),
		kSpriteTileBankStart + kAnimationTileBankStride * (3 + _sequenceState.VerticalPhase));
	_scene.setSpriteVerticalOffset(0);
	_scene.replaceSprites(spriteSpan);
}

void EpisodeOpeningSequence::writeEpisodeOpeningSpriteGroup(Span<SpriteInstance> destination,
															int16 packedX, int16 packedY,
															int tileIndex) {
	// Ghidra 0x0000175C-0x000017AD: replace four ROM cursors, linked-entry writes, and the fixed DBF loop
	// with six decoded logical sprites while preserving cumulative word arithmetic and the packed-X zero case.
	for (int index = 0; index < kSpriteGroupSize; index++) {
		int sourceOffset = index * static_cast<int>(sizeof(int16));
		packedY += _rom.readInt16(kSpriteGroupYOffsetTableOffset + sourceOffset);
		uint16 shape = _rom.readUInt16(kSpriteGroupShapeTableOffset + sourceOffset);
		uint16 attributes =
			static_cast<uint16>(tileIndex + _rom.readUInt16(
												kSpriteGroupTileOffsetTableOffset + sourceOffset));
		packedX += _rom.readInt16(kSpriteGroupXOffsetTableOffset + sourceOffset);
		uint16 publishedX = packedX == 0
								? static_cast<uint16>(0xFFFF)
								: static_cast<uint16>(packedX);
		destination[static_cast<std::size_t>(index)] = SpriteInstance(
			(publishedX & 0x01FF) - kSpriteCoordinateBias,
			(static_cast<uint16>(packedY) & 0x03FF) - kSpriteCoordinateBias, ((shape >> 10) & 0x03) + 1,
			((shape >> 8) & 0x03) + 1, attributes & 0x07FF,
			static_cast<uint8>((attributes >> 13) & 0x03),
			(attributes & 0x0800) != 0, (attributes & 0x1000) != 0, (attributes & 0x8000) != 0);
	}
}
} // namespace Scooby