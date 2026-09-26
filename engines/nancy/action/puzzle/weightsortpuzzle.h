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

#ifndef NANCY_ACTION_WEIGHTSORTPUZZLE_H
#define NANCY_ACTION_WEIGHTSORTPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/puzzlerecord.h"
#include "engines/nancy/misc/mousefollow.h"

namespace Nancy {
namespace Action {

// Sorting puzzle, new in Nancy15 (AR 185, the frass sort). Loose objects are carried from
// one container to another; each has a kind and a weight, and a container only takes the
// kinds it declares and can show a running readout of what it holds.
//
// Both an object's sprite and its resting place are picked at random, so the same pile
// never comes out looking the same twice.
//
// A board whose containers do not count towards either outcome never ends on its own and
// is left through the give-up hotspot, which is what the Nancy15 sort screen does.
class WeightSortPuzzle : public PuzzleRecord {
public:
	WeightSortPuzzle() : PuzzleRecord(7) {}
	virtual ~WeightSortPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

	static const uint kNumOutcomes = 2;

	enum PickUpMode : byte {
		kPickUpAny			= 0,
		kPickUpOriginal		= 1,	// only what the container started with, not what was put in
		kPickUpSpare		= 2		// only down to what the container is required to hold
	};

	struct ObjectType {
		int32 kind = 0;
		int32 weight = 0;
		Common::Path imageName;
		Common::Array<Common::Rect> srcs;
	};

	// The open range of totals a container has to sit in for one of the two outcomes.
	struct OutcomeRange {
		int32 maxExclusive = 0;
		int32 minExclusive = 0;
	};

	struct Container {
		byte showTotalWeight = 0;			// readout totals weight instead of counting objects
		bool countsTowardOutcome = false;
		byte pickUpMode = kPickUpAny;
		bool dropInPlace = false;			// false re-scatters the container after a drop
		OutcomeRange ranges[kNumOutcomes];
		Common::Rect area;					// where its contents are scattered
		Common::Rect shakeHotspot;			// clicking it re-scatters the container
		Common::Path readoutImageName;
		Common::Array<Common::Rect> digitSrcs;	// digits 0-9
		Common::Array<Common::Rect> digitDests;	// one slot per readout digit
		int32 acceptedKinds = 0;			// OR of the kind bits it takes
		RandomSoundBlock pickUpSound;
		RandomSoundBlock putDownSound;
		Common::Array<int32> initialContents;	// object type indices, duplicates allowed
		Common::Array<int32> requiredContents;	// what it has to hold for the first outcome
	};

	struct Outcome {
		SceneChangeDescription scene;
		FlagDescription flag;
		RandomSoundBlock sound;
	};

	struct Object {
		uint type = 0;
		int container = -1;			// -1 while it rides the cursor
		bool placedByHand = false;
		Common::Rect src;
		Common::Rect dest;
	};

protected:
	Common::String getRecordTypeName() const override { return "WeightSortPuzzle"; }

	static void readContainer(Common::SeekableReadStream &stream, Container &dst);

	void scatter(Object &object);
	void scatterContainer(uint container);
	int objectAtCursor(const Common::Point &mousePos) const;
	int containerAtCursor(const Common::Point &mousePos) const;
	int shakeHotspotAtCursor(const Common::Point &mousePos) const;
	bool canPickUp(const Object &object) const;
	int countIn(uint container, int32 type) const;
	static int countListed(const Common::Array<int32> &list, int32 type);
	int32 readoutValue(uint container) const;
	bool containerPasses(uint container, uint outcome) const;
	// Which outcome the board has reached, or -1 while it is still being sorted.
	int reachedOutcome() const;

	void carryObject(int object, NancyInput &input);
	void redraw();

	// -- File data --
	byte _firstNeedsAllContainers = 0;	// else a single passing container is enough
	byte _secondNeedsAllContainers = 0;
	byte _firstNeedsEmptyCursor = 0;
	byte _returnOnMiss = 0;				// dropping outside every container puts it back
	uint16 _hoverCursorType = 0;
	uint16 _dragCursorType = 0;
	uint16 _rejectCursorType = 0;

	Common::Array<ObjectType> _objectTypes;
	Common::Array<Container> _containers;

	RandomSoundBlock _rejectSound;		// a container refusing an object
	Outcome _outcomes[kNumOutcomes];

	// -- Runtime state --
	Common::Array<Object> _objects;
	int _carriedObjectID = -1;
	int _carriedFrom = -1;
	Misc::MouseFollowObject _carriedObject;
	int _outcome = -1;
	bool _exitRequested = false;
	SoundDescription _endSound;

	Common::Array<Graphics::ManagedSurface> _typeImages;
	Common::Array<Graphics::ManagedSurface> _readoutImages;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_WEIGHTSORTPUZZLE_H
