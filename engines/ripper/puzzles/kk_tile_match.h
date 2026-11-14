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

#ifndef RIPPER_PUZZLES_KK_TILE_MATCH_H
#define RIPPER_PUZZLES_KK_TILE_MATCH_H

#include "audio/mixer.h"
#include "common/array.h"
#include "common/hashmap.h"
#include "common/random.h"
#include "common/rect.h"

#include "ripper/display.h"
#include "ripper/puzzles/puzzle.h"
#include "ripper/puzzles/kk_tile_match_model.h"
#include "ripper/resources.h"

namespace Ripper {

class RipperEngine;

class KkTileMatchPuzzle : public Puzzle {
public:
	explicit KkTileMatchPuzzle(RipperEngine *engine);

	Result run(uint completionFlag) override;

private:
	struct FlipAnimation {
		Common::Array<BitmapAssetFrame> frames;
		bool loaded;

		FlipAnimation() : loaded(false) {}
	};

	class DebugHelper {
	public:
		DebugHelper();

		void reset(bool enabled);
		bool sync(const KkTileMatchPuzzle &puzzle);
		void stateChanging();
		void stateChanged(const KkTileMatchPuzzle &puzzle);
		void draw(const KkTileMatchPuzzle &puzzle,
			byte *screen, uint pitch) const;

	private:
		struct SearchNode {
			KkTileMatchModel state;
			uint depth;
			int firstTile;
		};

		struct SearchStateHash {
			uint operator()(const KkTileMatchModel &state) const;
		};

		struct SearchStateEqual {
			bool operator()(const KkTileMatchModel &left,
				const KkTileMatchModel &right) const;
		};

		int findRecommendedTile(const KkTileMatchPuzzle &puzzle,
			uint &solutionDepth, uint &visitedStates) const;

		bool _enabled;
		int _recommendedTile;
		uint _solutionDepth;
	};

	bool loadConfiguration();
	bool loadAssets();
	bool loadFlipAnimation(uint state);
	void drawSlot(byte *screen, uint pitch, const KkTileMatchModel &state,
		uint tile, uint frame, const Common::Point &position) const;
	void renderBoard() const;
	void renderMove(const KkTileMatchModel &snapshot, const uint *frames,
		uint selectedTile, uint step) const;
	bool flashTiles();
	bool applyMove(uint selectedTile);
	bool openTile(uint tile);
	bool closeActiveTiles();
	bool waitMillis(uint32 duration, bool interruptible);
	int findTile(const Common::Point &point) const;
	void updateCursor(const Common::Point &point);
	bool serviceKeyword(uint16 command);
	bool playCue(uint cue, bool loop = false);
	void stopCue(uint cue);
	void stopAudio();
	Common::String slotStateString() const;

	AssetLibrary _library;
	BitmapAssetFrame _background;
	BitmapAssetFrame _closedTile;
	IndexedDisplaySnapshot _incomingDisplay;
	FlipAnimation _flipAnimations[13];
	Audio::SoundHandle _audioHandles[3];
	Common::RandomSource _random;
	int _moveTable[KkTileMatchModel::kTileCount][KkTileMatchModel::kTileCount];
	KkTileMatchModel _model;
	uint _frames[KkTileMatchModel::kTileCount];
	DebugHelper _debugHelper;
	bool _visibleTiles[KkTileMatchModel::kTileCount];
	int _hoveredTile;
	uint _keywordIndex;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_KK_TILE_MATCH_H
