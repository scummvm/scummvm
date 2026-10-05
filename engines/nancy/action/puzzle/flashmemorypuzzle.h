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

#ifndef NANCY_ACTION_FLASHMEMORYPUZZLE_H
#define NANCY_ACTION_FLASHMEMORYPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/puzzlerecord.h"

namespace Nancy {
namespace Action {

// Nancy17 pairs game in the dark. The pictures can only really be seen while
// lightning flashes. A click turns a picture over; two that belong together go
// away, two that don't turn back after a moment. Clearing the board wins.
class FlashMemoryPuzzle : public PuzzleRecord {
public:
	FlashMemoryPuzzle() : PuzzleRecord(7) {}
	virtual ~FlashMemoryPuzzle();

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "FlashMemoryPuzzle"; }

	// A window the lightning comes through
	struct Light {
		Common::Path backgroundName;	// what the flash adds to the whole room
		Common::Path picturesName;		// what it adds to the pictures
		double minWait = 0;				// in seconds
		double maxWait = 0;
		RandomSoundBlock thunder;

		double waited = 0;
		double nextFlash = 0;
	};

	struct Keyframe {
		double time = 0;
		double brightness = 0;
	};

	struct Flash {
		uint lightID = 0;
		double time = 0;
		double duration = 0;
		double brightness = 0;
		Common::Array<Keyframe> keyframes;	// by time
		double flickerInterval = 0;
		double flickerTime = 0;
		double flicker = 0;

		void addKeyframe(double time, double brightness);
		// Returns whether the flash is over
		bool update(double seconds);
	};

	struct Card {
		bool isShown = true;
		bool isMatched = false;
		int pairID = 0;
		Common::Rect src;
		Common::Rect dest;
	};

	static double getRandom();
	void startFlash(uint lightID);
	void clickCard(int cardID);
	void checkPair();
	// Adds a picture, dimmed by a brightness, to what is drawn
	void addLight(const Graphics::ManagedSurface &source, const Common::Rect &src, const Common::Point &dest, double brightness);
	void redraw();

	// -- File data --
	Common::Array<Light> _lights;
	Common::Array<Card> _cards;
	uint16 _cardCursor = 0;
	double _turnBackTime = 0;	// in seconds
	RandomSoundBlock _clickSound;
	RandomSoundBlock _matchSound;
	RandomSoundBlock _noMatchSound;
	RandomSoundBlock _solveSoundBlock;

	// -- Runtime --
	Common::HashMap<Common::String, Graphics::ManagedSurface *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _lightImages;
	Common::Array<Flash> _flashes;
	int _firstCard = -1;
	int _secondCard = -1;
	double _pairTime = 0;
	uint32 _lastUpdateTime = 0;
	bool _isSolved = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_FLASHMEMORYPUZZLE_H
