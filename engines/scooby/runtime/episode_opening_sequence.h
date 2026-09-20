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

#ifndef SCOOBY_EPISODE_OPENING_SEQUENCE_H
#define SCOOBY_EPISODE_OPENING_SEQUENCE_H

#include "common/scummsys.h"

#include "common/array.h"
#include "common/func.h"

#include "runtime_state.h"
#include "scooby/assets/rom.h"
#include "scooby/common/span.h"
#include "scooby/graphics/sprite_instance.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/main_menu/vblank/main_menu_vblank_handler.h"
#include "scooby/presentation/frame_presenter.h"

// Owns the selected episode's animated handover from menu state to room startup.

namespace Scooby {

class EpisodeOpeningSequence {
public:
	// Binds the episode handover to its authored assets, callback tail, and shared presentation.
	// rom: verified cartridge asset source.
	// scene: logical scene receiving complete opening assets.
	// presenter: shared retrace and palette-transition owner.
	// inputVBlank: recovered callback containing the opening's raw 0x0000A4A4 entry.
	// state: shared episode, input, palette, and scroll-animation state.
	EpisodeOpeningSequence(const ScoobyDooRom &rom, TileScene &scene,
						   FramePresenter &presenter,
						   MainMenuVBlankHandler &inputVBlank,
						   RuntimeState &state)
		: _rom(rom), _scene(scene), _presenter(presenter), _inputVBlank(inputVBlank), _state(state),
		  _invokeOpeningVBlankFunctor(this, &EpisodeOpeningSequence::invokeOpeningVBlank) {
	}

	// Publishes and runs the selected episode's opening until an action edge or its authored
	// endpoint.
	// Returns false when the host requests shutdown during a recovered retrace.
	//
	// Ghidra: runEpisodeOpeningSequence (0x000013BA-0x0000160B, 0x00001624-0x000016C3). Episode-one
	// patterns occupy compressed source 0x000193DC-0x0001C088 and expand to 894 tiles; episode-two
	// patterns occupy 0x0001D60A-0x0002066D and expand to 948 tiles. Each episode supplies two
	// exact 7,168-byte, 128x28 maps while H32 exposes a centered 256-pixel viewport over those
	// complete 1024-pixel rows. Six byte-RLE sprite banks each expand to 60 tiles at indices 0x458,
	// 0x494, 0x4D0, 0x50C, 0x548, and 0x584. Episode palettes at 0x0001D58A/0x00021940 receive
	// shared colors 1-30 from 0x00032482; only episode one forces color zero black. Raw callback
	// entry 0x0000A4A4 preserves input and countdown work while skipping menu-only lightning and
	// animation.
	bool runEpisodeOpeningSequence();

private:
	struct OpeningState {
		// Replaces Ghidra g_sbEpisodeOpeningAnimationCountdown at 0xFF0AD0. No image-backed value is
		// consumed; the opening writes one before its first signed-byte decrement.
		int8 AnimationCountdown{};

		// Replaces the first 448 bytes of Ghidra g_abWideMenuAndOpeningWorkspace at
		// 0xFF3800-0xFF39BF. The parent clears all 224 signed words before the first update; each
		// update publishes the current values, then builds the next call's values.
		Common::Array<int16> BackgroundLineHorizontalOffsets = Common::Array<int16>(224);

		// Replaces Ghidra g_wEpisodeOpeningAnimationTileOffset at 0xFF0A1C. No image-backed value is
		// consumed; the opening writes zero before alternating it with 0x3C.
		int16 AnimationTileOffset{};

		// Replaces Ghidra g_wEpisodeOpeningHorizontalOffset at 0xFF0A18. No image-backed value is
		// consumed; the opening starts it at -158 and advances it through 270.
		int16 HorizontalOffset{};

		// Replaces the Ghidra g_bSharedDecodeBufferStart projection at 0xFF3000-0xFF31BF. The parent
		// clears all 224 signed words before the first update; each update publishes the current
		// values, then builds the next call's values.
		Common::Array<int16> ForegroundLineHorizontalOffsets = Common::Array<int16>(224);

		// Replaces Ghidra g_wEpisodeOpeningLeadingSpriteXOffset at 0xFF0A14. No image-backed value is
		// consumed; the opening clears it before sprite publication.
		int16 LeadingSpriteXOffset{};

		// Replaces Ghidra g_wEpisodeOpeningScrollBandBase at 0xFF09F0. No image-backed value is
		// consumed; the opening clears it before the first scroll update.
		int16 ScrollBandBase{};

		// Replaces Ghidra g_alEpisodeOpeningScrollAccumulators at 0xFF09F4-0xFF0A13. Process
		// initialization supplies zero for untouched low words; the opening then preserves the
		// original selective clears before its scroll child advances signed 16.16 values.
		Common::Array<int32> ScrollAccumulators = Common::Array<int32>(8);

		// Replaces Ghidra g_wEpisodeOpeningSpriteYBase at 0xFF0A1A. No image-backed value is
		// consumed; the opening writes 159 before sprite publication.
		int16 SpriteYBase{};

		// Replaces Ghidra g_wEpisodeOpeningVerticalPhase at 0xFF0A16. No image-backed value is
		// consumed; the opening clears it before loading the inline phase sequence.
		int16 VerticalPhase{};
	};

	static const int kAnimationTileBankStride = 0x3C;
	static const int kBackgroundAssetField = 0x04;
	static const int kDefaultSharedPaletteOffset = 0x32482;
	static const int kEpisodeAssetRecordSize = 0x10;
	static const int kEpisodeAssetTableOffset = 0x18FEC;
	static const int kForegroundAssetField = 0x08;
	static const int kLayerColumns = 128;
	static const int kLayerRows = 28;
	static const int kOpeningPaletteField = 0x0C;
	static const int kOpeningSpriteDescriptorOffset = 0x10B14;
	static const int kOpeningSpriteStreamCount = 6;
	static const int kSharedPaletteOverlayCount = 30;
	static const int kSpriteCoordinateBias = 128;
	static const int kSpriteDescriptorDataField = 0x06;
	static const int kSpriteDescriptorOffsetTableField = 0x04;
	static const int kSpriteGroupShapeTableOffset = 0xFC99C;
	static const int kSpriteGroupSize = 6;
	static const int kSpriteGroupTileOffsetTableOffset = 0xFC944;
	static const int kSpriteGroupXOffsetTableOffset = 0xFC89C;
	static const int kSpriteGroupYOffsetTableOffset = 0xFC8B4;
	static const int kSpriteTableSize = 18;
	static const int kSpriteTileBankStart = 0x458;
	static const Common::Array<int8> kScrollDelays;
	static const Common::Array<int16> kVerticalPhases;

	void advanceVerticalPhase();
	void invokeOpeningVBlank();
	static void playAudioCommand(int command) { (void)command; }
	void updateEpisodeOpeningScene();
	bool waitForOpeningVerticalBlank();
	void writeEpisodeOpeningSpriteTable();
	void writeEpisodeOpeningSpriteGroup(Span<SpriteInstance> destination,
										int16 packedX,
										int16 packedY, int tileIndex);

	const ScoobyDooRom &_rom;
	TileScene &_scene;
	FramePresenter &_presenter;
	MainMenuVBlankHandler &_inputVBlank;
	RuntimeState &_state;
	OpeningState _sequenceState;
	Common::Functor0Mem<void, EpisodeOpeningSequence> _invokeOpeningVBlankFunctor;
};
} // namespace Scooby

#endif // SCOOBY_EPISODE_OPENING_SEQUENCE_H
