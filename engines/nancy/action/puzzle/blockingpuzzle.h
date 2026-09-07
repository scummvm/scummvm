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

#ifndef NANCY_ACTION_BLOCKINGPUZZLE_H
#define NANCY_ACTION_BLOCKINGPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/movieplayer.h"
#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Movie-driven fighting minigame (Nancy14, AR 180). The on-screen opponent attacks
// from one of nine directions in a 3x3 grid; the player blocks by hovering the
// matching cell. Directional movies play through one MoviePlayer, picked by weighted
// random from an idle-dominated sequence, with sound effects layered on top. Each
// fighter's health is a value-table entry (shown by a Meter puzzle, AR 179); a
// ValueTest scene change ends the fight when one is depleted.
class BlockingPuzzle : public RenderActionRecord {
public:
	BlockingPuzzle() : RenderActionRecord(7) {}
	virtual ~BlockingPuzzle();

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "BlockingPuzzle"; }

	// A movie playback descriptor: frame range plus a pauseChance% chance to pause a
	// random [minPauseMs, maxPauseMs] between plays. This port only uses the name.
	struct MovieDescription {
		Common::Path name;
		int16 startFrame = 0;
		int16 lastFrame = 0;
		int32 minPauseMs = 0;
		int32 maxPauseMs = 0;
		byte pauseChance = 0;

		void readData(Common::SeekableReadStream &stream);
	};

	// One movie variant of a weighted random pick.
	struct WeightedMovie {
		Common::Path name;
		int16 weight = 0;
	};

	// One cell of the 3x3 attack grid: a viewport rect, eight tuning values (four
	// recoil-vector pairs, see init()), and the indices of the adjacent cells that
	// count as a partial block.
	struct GridCell {
		byte id = 0;
		Common::Rect rect;
		int16 params[8] = {};
		Common::Array<int16> neighbors;
	};

	// One attack from grid cell cellID (1-9). windup/attack are sound effects; response
	// is the directional movie; strikeFrame is when the block is scored. field_c/d/e
	// are unconfirmed (likely sound timing). idleNames/idleWeight are the weighted
	// return-to-idle movie sequence.
	struct AttackMove {
		int16 cellID = 0;
		int16 field2 = 0;
		RandomSoundBlock windup;
		int16 field_c = 0;
		int16 field_d = 0;
		int16 field_e = 0;
		RandomSoundBlock attack;
		int16 strikeFrame = 0;			// 0xf5
		MovieDescription response;
		Common::Array<Common::Path> idleNames;
		int16 idleWeight = 0;
	};

	// -- File data --
	int16 _opponentHealthIndex = 0;	// 0x17b - value-table slot damaged by recoil.x (Minette)
	int16 _playerHealthIndex = 0;	// 0x17d - value-table slot damaged by recoil.y (the player)
	int16 _field4 = 0;				// 0x183 - unknown
	int16 _field6 = 0;				// 0x154 - unknown
	byte _field8 = 0;				// 0x153 - combat gate flag (gates block input in the original); unused here
	byte _field9 = 0;				// 0x152 - combat gate flag; unused here
	Common::Path _imageName;		// MOU_Fight_OVL sprite sheet
	Common::Rect _controlRects[4];	// [0] = the attack-telegraph box sprite (cell-sized); [1-3] other UI
	Common::Point _point;			// 0x1b3 - unknown
	int16 _field51 = 0;				// 0x51 - unknown
	Common::Rect _movieSrc;			// source rect within the fight movie
	Common::Rect _movieDest;		// on-screen destination for the fight movie
	byte _flag = 0;					// shared across the three reaction-sound blocks; unknown

	// The attacker's reaction sound effects, picked by block outcome and shared
	// across attacks.
	RandomSoundBlock _fullBlockSounds;
	RandomSoundBlock _partBlockSounds;
	RandomSoundBlock _hitSounds;

	Common::Array<GridCell> _grid;

	MovieDescription _introMovie;
	Common::Array<WeightedMovie> _introSequence;

	Common::Array<AttackMove> _moves;

	// -- Runtime state --
	// A grid cell resolved for gameplay: its hit rect, the four recoil/damage
	// vectors (indexed by block outcome), and the adjacent cells that count as a
	// partial block.
	struct RuntimeCell {
		int16 id = 0;
		Common::Rect rect;
		Common::Point base;		// always applied
		Common::Point full;		// exact-direction block
		Common::Point hit;		// unblocked
		Common::Point partial;	// adjacent-direction block
		Common::Array<int16> neighbors;
	};

	// resolveBlock() outcomes.
	enum BlockOutcome { kFullBlock = 1, kPartialBlock = 2, kHit = 3 };

	void redraw();
	void drawTelegraph(const Common::Rect &srcRect, const Common::Point &destPos, byte alpha);
	void playNextMovie();
	bool playMovie(const Common::Path &name);
	void updateRecoil();
	int findMoveByMovie(const Common::Path &name) const;
	const RuntimeCell *cellByID(int id) const;
	int cellAtPoint(const Common::Point &mousePos) const;
	int resolveBlock(int attackCell, int blockCell, Common::Point &recoil) const;
	void applyDamage(const Common::Point &recoil);
	void playSoundBlock(const RandomSoundBlock &block);

	MoviePlayer _moviePlayer;
	Graphics::ManagedSurface _overlayImage;	// MOU_Fight_OVL sprite sheet
	Common::Array<RuntimeCell> _cells;
	int _activeMove = -1;		// index into _moves for the directional clip playing (-1 = milling)
	int _playerBlockCell = 0;	// cell ID the player is guarding (0 = none)
	bool _resolved = false;		// this attack's block already scored

	// Impact screen-shake: decays to zero over its window.
	uint32 _recoilStartMs = 0;	// 0 = no active shake
	Common::Point _recoilAmp;	// initial shake amplitude, in pixels
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_BLOCKINGPUZZLE_H
