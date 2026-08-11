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

#ifndef NANCY_ACTION_SCALEPUZZLE_H
#define NANCY_ACTION_SCALEPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Balance-scale puzzle, new in Nancy13 (AR 174). A row of numbered
// "sampler" figures sits above two scale pans (a "minus" pan on the left
// and a "plus" pan on the right) with a numeric indicator between them.
// Coins of known values start in a tray; each is picked up by clicking it
// and dropping onto an empty slot on either pan or back into the tray.
// The indicator shows the running total sum(right values) - sum(left
// values).
//
// Each figure has to be matched in turn: a figure lights (and its latch
// bar opens) when the number of coins resting on the two pans equals its
// required coin count and the indicator magnitude equals the figure's
// number. Figures must be matched in order, the lights stay on once lit,
// and clearing the pans resets them. The puzzle is solved once every
// figure of the current scene is lit.
class ScalePuzzle : public RenderActionRecord {
public:
	ScalePuzzle() : RenderActionRecord(7) {}
	virtual ~ScalePuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "ScalePuzzle"; }

	static const int16 kNoCoin = -1;

	// Which of the three slot groups a slot belongs to.
	enum SlotRegion { kSourceTray = 0, kLeftPan = 1, kRightPan = 2 };

	// A draggable coin. Its value determines how far it moves the indicator; a coin is only
	// placed if the player is carrying its inventory item (coins are collected in the game).
	struct Coin {
		int16 itemID = -1;			// 0x00 - inventory item id gating this coin
		int32 value = 0;			// 0x02
		Common::Rect src;			// 0x06 - coin sprite in the overlay image
	};

	// A numbered figure to be matched. It lights when exactly "coinCount" coins rest on the
	// pans and the indicator magnitude equals "number"; its latch sprite is then drawn.
	struct Target {
		byte coinCount = 0;			// 0x00 - required number of coins on the pans
		int32 number = 0;			// 0x01 - the figure's number (required |indicator|)
		Common::Rect latchSrc;		// 0x05 - the open-latch sprite in the overlay image
		Common::Rect latchDst;		// 0x15 - where it is drawn (a bar along the top)
		RandomSoundBlock sound;		// the latch-open sound, played once when it lights
		bool lit = false;			// runtime: has it been matched?
	};

	// One group of slots (tray / left pan / right pan): the fixed on-screen positions and,
	// per slot, the index of the coin resting there (or kNoCoin).
	struct SlotGroup {
		Common::Array<Common::Rect> dests;
		Common::Array<int16> coins;
	};

	SlotGroup &group(SlotRegion region);
	// The slot whose rect contains the cursor. When wantEmpty is set only empty slots match
	// (used while carrying), otherwise only occupied slots match (used while picking up).
	bool slotAtCursor(const Common::Point &mousePos, bool wantEmpty, SlotRegion &outRegion, uint &outIndex) const;
	void recomputeBalance();
	// Draws the image region src centred inside slot (the original centres coins in their
	// slots, FUN_004b6660 case 0).
	void blitCentered(const Common::Rect &src, const Common::Rect &slot);
	void redraw();
	void setDataCursor(uint16 cursorType) const;
	SoundDescription playSoundBlock(const RandomSoundBlock &block);

	// -- File data --
	Common::Path _imageName;				// 0x00
	uint16 _hoverCursorType = 0;			// 0x21 - raw Nancy13 cursor type over a coin
	uint16 _dragCursorType = 0;				// 0x23 - raw Nancy13 cursor type while carrying
	SceneChangeDescription _solveScene;		// 0x25 - applied when solved (9999 => none)
	FlagDescription _solveFlag;				// 0x27 - set when solved
	RandomSoundBlock _latchSound;			// the first sound block; played on give-up

	Common::Array<Target> _targets;			// the figures to match in this scene
	Common::Array<Coin> _coins;				// the coin definitions

	SlotGroup _tray;						// container 0xc7 - source tray (35 slots)
	SlotGroup _left;						// container 0xb3 - left ("minus") pan (7 slots)
	SlotGroup _right;						// container 0xbd - right ("plus") pan (7 slots)

	Common::Rect _coinBackSrc;				// 0xd1 - the coin-count token sprite
	Common::Array<Common::Rect> _pileFrames;	// 0xe1 - token positions (drawn at placedCount-1)
	Common::Rect _altSrc;					// 0xef - the lit red-light sprite
	Common::Array<Common::Rect> _lightFrames;	// 0xff - the 10 red-light positions (by coinCount-1)
	Common::Array<Common::Rect> _indicatorFrames;	// 0x85 - the numeric indicator (-N..+N)
	Common::Rect _indicatorDest;			// 0x8f - where the indicator is drawn

	RandomSoundBlock _pickupSound;			// 0x170 - coin picked up
	RandomSoundBlock _dropTraySound;		// 0x218 - coin dropped back into the tray
	RandomSoundBlock _dropPanSound;			// 0x1c4 - coin dropped onto a pan

	// The clickable "give up / exit" hotspot (the base-class hotspot record). Clicking it
	// always jumps to the scene's first frame and sets an event flag.
	Common::Rect _exitHotspot;
	uint16 _exitCursorType = 0;
	SceneChangeDescription _exitScene;
	FlagDescription _exitFlag;

	// -- Runtime state --
	int _indicatorZeroFrame = 0;			// the "0" frame (frames.size() / 2)
	int _tilt = 0;							// the indicator value: sum(right) - sum(left)
	int _placedCount = 0;					// coins resting on the two pans

	int16 _carriedCoin = kNoCoin;			// the coin on the cursor, or kNoCoin
	Common::Point _dragPos;					// cursor position (viewport space) while carrying

	bool _solved = false;
	bool _exitRequested = false;
	SoundDescription _endSound;				// the cue we wait on before changing scene

	Graphics::ManagedSurface _image;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_SCALEPUZZLE_H
