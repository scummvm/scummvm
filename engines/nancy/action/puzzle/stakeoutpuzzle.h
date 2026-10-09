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

#ifndef NANCY_ACTION_STAKEOUTPUZZLE_H
#define NANCY_ACTION_STAKEOUTPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/puzzlerecord.h"

namespace Nancy {
namespace Action {

// Nancy18 stakeout. A few people hide in a courtyard, one of them the person to
// catch. A round starts with the others calling out over the radio where they
// are, which a dialog lists; then everybody may peek out of their hiding place
// for a while. Clicking a person who peeks out tells the others where that
// person is. If it is the right one, somebody goes to look there, by which time
// the person has moved on; if not, the one who got pointed out complains.
// Either way a value of the value table counts it, and the scene takes it from there.
class StakeOutPuzzle : public PuzzleRecord {
public:
	StakeOutPuzzle() : PuzzleRecord(7) {}
	virtual ~StakeOutPuzzle();

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "StakeOutPuzzle"; }

	enum RoundState {
		kAnnouncing,
		kPeeking,
		kCatching
	};

	// What a person is doing at a point of their time in a round
	enum Phase {
		kWaiting	= 0,
		kShowing	= 1,
		kShown		= 2,
		kHiding		= 3,
		kGone		= 4
	};

	struct Place {
		// -- File data --
		Common::String name;
		Common::Rect dest;
		Common::Path imageName;
		Common::Array<Common::Rect> showSrcs;
		Common::Array<Common::Rect> hideSrcs;
		RandomSoundBlock sound;	// the player tells the others about this place

		// -- Runtime --
		Graphics::ManagedSurface image;
		bool isTaken = false;
	};

	struct Person {
		// -- File data --
		Common::String name;
		bool isTarget = false;
		uint16 channel = 0;
		RandomSoundBlock wrongCallSound;
		RandomSoundBlock notThereSound;
		RandomSoundBlock stopSound;

		// -- Runtime --
		int place = -1;
		bool isAnnounced = false;
		bool wasCalledOut = false;	// by mistake
		bool isCatching = false;	// goes to look where the target was seen
		bool hasMoved = false;		// to that place
		double time = 0.0;			// all in seconds
		double showStart = 0.0;
		double shownTime = 0.0;
		double hideStart = 0.0;
		double goneTime = 0.0;
		SoundDescription wrongCallLine;	// the ones that got played
		SoundDescription notThereLine;
	};

	Phase getPhase(const Person &person, double &fraction) const;
	void setPhase(Person &person, Phase phase, double fraction);
	void hideNow(Person &person, Phase phase, double fraction);

	double getRandomTime(double min, double max) const;
	double getPeekTime(const Place &place) const;
	int getRandomFreePlace() const;
	void shuffleAnnouncements();

	bool announce(Person &person);
	void addText(const Common::String &key);
	void removeTexts();
	void clearCaption(const SoundDescription &line);

	void update(double elapsed);
	void startPeeking();
	bool updatePerson(Person &person, double elapsed);
	void moveToTargetPlace(Person &person);
	void resolveCall(uint personID);
	void endRound();

	bool isClickable(const Person &person, const Common::Point &pos) const;
	int getFrame(const Person &person) const;
	void redraw();

	// -- File data --
	int16 _rightCallsTableIndex = -1;
	int16 _wrongCallsTableIndex = -1;
	Common::String _dialogName;
	Common::String _textTemplate;
	uint16 _hoverCursor = 0;
	double _peekChance = 0.0;
	double _lookTime = 0.0;				// how long the place of the target gets looked at
	double _targetHideFactor = 0.0;		// of the time it takes to hide, for a target that got seen
	double _minPeekWindow = 0.0;
	double _maxPeekWindow = 0.0;
	double _minAnnouncePause = 0.0;
	double _maxAnnouncePause = 0.0;
	double _minPeekDelay = 0.0;
	double _maxPeekDelay = 0.0;
	double _minRoundPause = 0.0;
	double _maxRoundPause = 0.0;
	Common::Array<Place> _places;
	Common::Array<Person> _persons;

	// -- Runtime --
	RoundState _roundState = kAnnouncing;
	Common::Array<uint> _announcements;	// the persons who still have to say where they are
	double _clock = 0.0;
	double _peekStart = 0.0;
	double _peekEnd = 0.0;
	double _nextTime = 0.0;				// the end of a pause, or of the round
	bool _isInputEnabled = true;
	int _calledPerson = -1;				// the player is saying where this one is
	bool _hasSaidNotThere = false;
	uint _numTexts = 0;
	bool _isDialogShown = false;

	uint32 _lastUpdateTime = 0;
	Common::Array<int> _shownFrames;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_STAKEOUTPUZZLE_H
