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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_ACTION_BEADPUZZLE_H
#define NANCY_ACTION_BEADPUZZLE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/commontypes.h"

namespace Nancy {
namespace Action {

// String-of-beads puzzle. The player picks beads from a source area and places
// them onto a thread; when all slots are filled the sequence is scored against
// the solution, yielding perfect / partial / wrong results.
// Called from scene 6251 in Nancy10.
class BeadPuzzle : public RenderActionRecord {
public:
	BeadPuzzle() : RenderActionRecord(7) {}
	virtual ~BeadPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "BeadPuzzle"; }

	static const int kMaxBeadTypes = 10;
	static const int kMaxSlots     = 25;

	// File data

	Common::Path _imageName;

	uint16 _numSlots     = 0;
	uint16 _numBeadTypes = 0;

	Common::Rect _beadSrcRects[kMaxBeadTypes];

	Common::Rect _threadSrc;
	Common::Rect _threadDest;

	Common::Rect _removeHotspot;
	Common::Rect _resultHotspot;

	Common::Rect _pickupHotspots[kMaxBeadTypes];

	Common::Rect _slotDestRects[kMaxSlots];
	Common::Rect _resultDestRects[kMaxSlots];

	uint16 _solution[kMaxSlots] = {}; // expected bead type per slot (file stores type+1; we store the raw value)

	SoundDescription _pickupSound;   // played while a bead is held next to the cursor
	SoundDescription _placeSound;    // played when a bead is placed onto the thread
	SoundDescription _removeSound;   // played when a bead is taken off the thread

	int16 _mistakeThreshold = 0;

	SoundDescription _partialSound;
	SoundDescription _wrongSound;

	FlagDescription _partialFlag;
	SoundDescription _perfectSound;
	FlagDescription _perfectFlag;

	SceneChangeDescription _defaultScene;
	SceneChangeDescription _solvedScene;

	Common::Rect _exitHotspot;

	// Runtime state

	enum SubState {
		kPlaying = 0,
		kDroppingBead,
		kShowingResult,
		kPerfectWaitSound,
		kExitToSolved,
		kExitToDefault
	};

	SubState _subState = kPlaying;

	Common::Array<int16> _placed;
	int16 _heldBead = -1;
	Common::Point _heldDrawPos;

	int16  _dropCurrentSlot = 0;
	uint32 _dropNextTick    = 0;

	enum ResultKind { kNoResult, kPerfect, kPartial, kWrong };
	ResultKind _resultKind = kNoResult;

	uint32 _perfectExitTime = 0;
	bool   _resultSoundPlayed = false;

	Graphics::ManagedSurface _image;

	void redraw();
	void evaluate();
	void persistState();
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_BEADPUZZLE_H
