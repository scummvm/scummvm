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

#include "common/random.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/util.h"
#include "engines/nancy/puzzledata.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/ui/ndui/manager.h"
#include "engines/nancy/action/puzzle/stakeoutpuzzle.h"

namespace Nancy {
namespace Action {

static const double kFramesPerSecond = 15.0;
static const uint kMaxRandom = 32767;

// How far into looking at the target's place the one who looks says there is nobody
static const double kNotThereFraction = 0.5;

static const uint16 kAnnouncementVolume = 100;
static const uint16 kWaitCursor = 23;

StakeOutPuzzle::~StakeOutPuzzle() {
	UI::NDUI::Manager *ndui = NancySceneState.getNDUI();
	if (!_isDialogShown || !ndui) {
		return;
	}

	ndui->takeAction(UI::NDUI::kActionHide, _dialogName, 1.0);
	removeTexts();
}

void StakeOutPuzzle::readData(Common::SeekableReadStream &stream) {
	_rightCallsTableIndex = stream.readSint16LE();
	_wrongCallsTableIndex = stream.readSint16LE();
	readFilename(stream, _dialogName);
	readFilename(stream, _textTemplate);
	_hoverCursor = stream.readUint16LE();

	_peekChance = stream.readDoubleLE();
	_lookTime = stream.readDoubleLE();
	_targetHideFactor = stream.readDoubleLE();
	_minPeekWindow = stream.readDoubleLE();
	_maxPeekWindow = stream.readDoubleLE();
	_minAnnouncePause = stream.readDoubleLE();
	_maxAnnouncePause = stream.readDoubleLE();
	_minPeekDelay = stream.readDoubleLE();
	_maxPeekDelay = stream.readDoubleLE();
	_minRoundPause = stream.readDoubleLE();
	_maxRoundPause = stream.readDoubleLE();

	_places.resize(MAX<int16>(stream.readSint16LE(), 0));
	for (Place &place : _places) {
		readFilename(stream, place.name);
		readExclusiveRect(stream, place.dest);
		readFilename(stream, place.imageName);

		place.showSrcs.resize(MAX<int16>(stream.readSint16LE(), 0));
		for (Common::Rect &src : place.showSrcs) {
			readExclusiveRect(stream, src);
		}

		place.hideSrcs.resize(MAX<int16>(stream.readSint16LE(), 0));
		for (Common::Rect &src : place.hideSrcs) {
			readExclusiveRect(stream, src);
		}

		place.sound.readData(stream);
	}

	_persons.resize(MAX<int16>(stream.readSint16LE(), 0));
	for (Person &person : _persons) {
		readFilename(stream, person.name);
		person.isTarget = stream.readByte();
		person.channel = stream.readUint16LE();
		person.wrongCallSound.readData(stream);
		person.notThereSound.readData(stream);
		person.stopSound.readData(stream);
	}
}

void StakeOutPuzzle::init() {
	initViewportSurface();

	for (Place &place : _places) {
		g_nancy->_resource->loadImage(place.imageName, place.image);
		place.image.setTransparentColor(_drawSurface.getTransparentColor());
	}

	_shownFrames.resize(_persons.size(), -1);

	UI::NDUI::Manager *ndui = NancySceneState.getNDUI();
	if (ndui) {
		ndui->takeAction(UI::NDUI::kActionShow, _dialogName, 1.0);
		_isDialogShown = true;
	}

	shuffleAnnouncements();
	_lastUpdateTime = g_nancy->getTotalPlayTime();
}

StakeOutPuzzle::Phase StakeOutPuzzle::getPhase(const Person &person, double &fraction) const {
	if (person.time < person.showStart) {
		fraction = person.time / person.showStart;
		return kWaiting;
	}

	if (person.time < person.shownTime) {
		fraction = (person.time - person.showStart) / (person.shownTime - person.showStart);
		return kShowing;
	}

	if (person.time < person.hideStart) {
		fraction = (person.time - person.shownTime) / (person.hideStart - person.shownTime);
		return kShown;
	}

	if (person.time < person.goneTime) {
		fraction = (person.time - person.hideStart) / (person.goneTime - person.hideStart);
		return kHiding;
	}

	fraction = 1.0;
	return kGone;
}

// Puts the person's time at a point of a phase
void StakeOutPuzzle::setPhase(Person &person, Phase phase, double fraction) {
	switch (phase) {
	case kWaiting:
		person.time = person.showStart * fraction;
		break;
	case kShowing:
		person.time = person.showStart + (person.shownTime - person.showStart) * fraction;
		break;
	case kShown:
		person.time = person.shownTime + (person.hideStart - person.shownTime) * fraction;
		break;
	case kHiding:
		person.time = person.hideStart + (person.goneTime - person.hideStart) * fraction;
		break;
	case kGone:
		person.time = person.goneTime;
		break;
	}
}

// A person who is coming out goes back in from as far as they got
void StakeOutPuzzle::hideNow(Person &person, Phase phase, double fraction) {
	if (phase == kShowing) {
		setPhase(person, kHiding, 1.0 - fraction);
	} else if (phase == kShown) {
		setPhase(person, kHiding, 0.0);
	}
}

double StakeOutPuzzle::getRandomTime(double min, double max) const {
	return min + g_nancy->_randomSource->getRandomNumber(kMaxRandom) / (double)kMaxRandom * (max - min);
}

// The time it takes to come out of a place, or to go back in
double StakeOutPuzzle::getPeekTime(const Place &place) const {
	return place.showSrcs.size() / kFramesPerSecond;
}

int StakeOutPuzzle::getRandomFreePlace() const {
	Common::Array<int> freePlaces;
	for (uint i = 0; i < _places.size(); ++i) {
		if (!_places[i].isTaken) {
			freePlaces.push_back(i);
		}
	}

	if (freePlaces.empty()) {
		return -1;
	}

	return freePlaces[g_nancy->_randomSource->getRandomNumber(freePlaces.size() - 1)];
}

// Everybody says where they are at the start of a round, in a new order every time
void StakeOutPuzzle::shuffleAnnouncements() {
	_announcements.clear();
	for (uint i = 0; i < _persons.size(); ++i) {
		_announcements.push_back(i);
	}

	for (uint left = _announcements.size(); left > 1; --left) {
		const uint index = g_nancy->_randomSource->getRandomNumber(left - 1);
		_announcements.push_back(_announcements.remove_at(index));
	}
}

// The person picks a place and says so over the radio, which the dialog also
// shows. The target does it without a word. Returns true once the person is done.
bool StakeOutPuzzle::announce(Person &person) {
	if (person.isAnnounced) {
		return !g_nancy->_sound->isSoundPlaying(person.channel);
	}

	person.isAnnounced = true;
	person.place = getRandomFreePlace();
	if (person.place == -1) {
		return true;
	}

	_places[person.place].isTaken = true;
	if (person.isTarget) {
		return true;
	}

	// There is a line for every person in every place
	const Common::String key = person.name + _places[person.place].name;

	SoundDescription sound;
	sound.name = key;
	sound.channelID = person.channel;
	sound.numLoops = 1;
	sound.volume = kAnnouncementVolume;
	g_nancy->_sound->loadSound(sound);
	g_nancy->_sound->playSound(sound);

	addText(key);
	return false;
}

void StakeOutPuzzle::addText(const Common::String &key) {
	// The lines of the agents are kept with the conversation texts
	Common::String text = resolveSubtitleText(key);
	if (text.empty()) {
		text = resolveSubtitleText(key, Common::String(), "CONVO");
	}

	UI::NDUI::Manager *ndui = NancySceneState.getNDUI();
	if (text.empty() || !ndui) {
		return;
	}

	++_numTexts;
	const Common::String name = Common::String::format("StakeOut%i", _numTexts);
	ndui->createObject(name, _textTemplate, _dialogName);
	ndui->takeAction(UI::NDUI::kActionSetText, name, text);
}

void StakeOutPuzzle::removeTexts() {
	UI::NDUI::Manager *ndui = NancySceneState.getNDUI();

	for (; _numTexts > 0; --_numTexts) {
		if (ndui) {
			ndui->destroyObject(Common::String::format("StakeOut%i", _numTexts));
		}
	}
}

// Takes the caption of a line off the screen, unless another one has replaced it
void StakeOutPuzzle::clearCaption(const SoundDescription &line) {
	UI::NDUI::Manager *ndui = NancySceneState.getNDUI();
	if (!ndui || ndui->getCaption().empty()) {
		return;
	}

	Common::String caption = resolveSubtitleText(line.name);
	if (caption.empty()) {
		caption = resolveSubtitleText(line.name, Common::String(), "CONVO");
	}

	if (caption == ndui->getCaption()) {
		NancySceneState.getTextbox().clear();
	}
}

// Everybody who peeks out in this round does it once, within the same span of time
void StakeOutPuzzle::startPeeking() {
	_roundState = kPeeking;
	_clock = 0.0;
	_peekStart = getRandomTime(_minPeekDelay, _maxPeekDelay);
	_peekEnd = getRandomTime(_minPeekWindow, _maxPeekWindow) + _peekStart;
	_nextTime = getRandomTime(_minRoundPause, _maxRoundPause) + _peekEnd;

	const double peekTime = _places.empty() ? 0.0 : getPeekTime(_places[0]);

	for (Person &person : _persons) {
		person.time = 0.0;

		if (getRandomTime(0.0, 1.0) >= _peekChance) {
			person.showStart = person.shownTime = person.hideStart = person.goneTime = 0.0;
		} else {
			person.showStart = getRandomTime(_peekStart, _peekEnd - 2 * peekTime);
			person.shownTime = person.showStart + peekTime;
			person.hideStart = getRandomTime(person.shownTime, _peekEnd - peekTime);
			person.goneTime = person.hideStart + peekTime;
		}
	}
}

// The target has left by the time somebody gets there. It hides in a new place,
// and the one who went to look shows up where it was for a moment.
void StakeOutPuzzle::moveToTargetPlace(Person &person) {
	Person *target = nullptr;
	for (Person &other : _persons) {
		if (other.isTarget) {
			target = &other;
			break;
		}
	}

	if (!target || target->time < target->goneTime) {
		return;
	}

	const int seenPlace = target->place;
	const int newPlace = getRandomFreePlace();
	if (newPlace == -1) {
		return;
	}

	target->time = target->goneTime;
	target->place = newPlace;
	_places[newPlace].isTaken = true;
	_places[person.place].isTaken = false;
	person.place = seenPlace;

	const double peekTime = getPeekTime(_places[seenPlace]);
	person.showStart = 0.0;
	person.shownTime = peekTime;
	person.hideStart = peekTime + _lookTime;
	person.goneTime = person.hideStart + peekTime;
	person.time = 0.0;
	person.hasMoved = true;
}

// Returns true once the person is out of sight for the rest of the round
bool StakeOutPuzzle::updatePerson(Person &person, double elapsed) {
	double fraction = 0.0;
	Phase phase = getPhase(person, fraction);

	if (person.wasCalledOut) {
		// Stays in sight to complain, and hides after that
		const bool isComplaining = isSoundBlockPlaying(person.wrongCallSound);
		if (!isComplaining) {
			clearCaption(person.wrongCallLine);

			if (phase == kShowing || phase == kShown) {
				hideNow(person, phase, fraction);
			} else {
				person.time += elapsed;
			}
		}

		return !isComplaining && phase == kGone;
	}

	person.time += elapsed;
	phase = getPhase(person, fraction);

	if (!person.isCatching) {
		return phase == kGone;
	}

	if (phase == kShown) {
		if (fraction > kNotThereFraction && person.hasMoved && !_hasSaidNotThere) {
			person.notThereLine = playSoundBlock(person.notThereSound);
			_hasSaidNotThere = true;
		}
	} else {
		_hasSaidNotThere = false;
	}

	if (phase == kGone) {
		if (!person.hasMoved) {
			moveToTargetPlace(person);
		} else if (_roundState == kCatching) {
			// Back from looking: what is left of the round is the pause after it
			person.isCatching = false;
			_clock = _peekEnd;
			_roundState = kPeeking;
		}
	}

	return phase == kGone;
}

// The player is done saying where the person is
void StakeOutPuzzle::resolveCall(uint personID) {
	Person &called = _persons[personID];
	called.wrongCallLine = playSoundBlock(called.wrongCallSound);

	if (!called.isTarget) {
		_isInputEnabled = true;
		called.wasCalledOut = true;
		return;
	}

	if (_persons.size() < 2) {
		return;
	}

	// Somebody goes after the target, and everybody gets out of sight
	uint catcherID = personID;
	while (catcherID == personID) {
		catcherID = g_nancy->_randomSource->getRandomNumber(_persons.size() - 1);
	}

	Person &catcher = _persons[catcherID];
	catcher.isCatching = true;
	catcher.wasCalledOut = false;
	playSoundBlock(catcher.stopSound);
	_roundState = kCatching;

	for (Person &person : _persons) {
		double fraction = 0.0;
		const Phase phase = getPhase(person, fraction);

		if (&person == &called) {
			person.goneTime = person.hideStart + (person.goneTime - person.hideStart) * _targetHideFactor;
			setPhase(person, phase, fraction);
		}

		if (phase == kWaiting) {
			person.time = person.goneTime;
		} else {
			hideNow(person, phase, fraction);
		}
	}
}

void StakeOutPuzzle::endRound() {
	_roundState = kAnnouncing;
	_clock = 0.0;
	_peekStart = 0.0;
	_peekEnd = 0.0;
	_nextTime = 0.0;
	_isInputEnabled = true;

	removeTexts();

	for (Person &person : _persons) {
		if (person.place != -1) {
			_places[person.place].isTaken = false;
		}

		clearCaption(person.notThereLine);

		person.place = -1;
		person.isAnnounced = false;
		person.wasCalledOut = false;
		person.hasMoved = false;
	}

	shuffleAnnouncements();
}

void StakeOutPuzzle::update(double elapsed) {
	if (_calledPerson != -1 && !isSoundBlockPlaying(_places[_persons[_calledPerson].place].sound)) {
		resolveCall(_calledPerson);
		_calledPerson = -1;
	}

	if (_roundState == kAnnouncing) {
		if (_clock > 0.0) {
			// The pause between two persons
			_clock += elapsed;
			if (_clock >= _nextTime) {
				_clock = 0.0;
			}
		} else if (!_announcements.empty() && announce(_persons[_announcements[0]])) {
			_announcements.remove_at(0);

			if (_announcements.empty()) {
				startPeeking();
			} else {
				_clock += elapsed;
				_nextTime = getRandomTime(_minAnnouncePause, _maxAnnouncePause) + _clock;
			}
		}

		return;
	}

	// Everything stops while the player speaks
	bool isEverybodyGone = true;
	if (_isInputEnabled || _calledPerson == -1) {
		_clock += elapsed;

		for (Person &person : _persons) {
			if (!updatePerson(person, elapsed)) {
				isEverybodyGone = false;
			}
		}
	}

	if (_clock >= _nextTime && _roundState == kPeeking && isEverybodyGone) {
		endRound();
	}
}

bool StakeOutPuzzle::isClickable(const Person &person, const Common::Point &pos) const {
	if (person.place == -1 || person.wasCalledOut) {
		return false;
	}

	double fraction = 0.0;
	const Phase phase = getPhase(person, fraction);
	return (phase == kShowing || phase == kShown || phase == kHiding) && _places[person.place].dest.contains(pos);
}

// The frame to draw of the person's place: the ones for coming out,
// followed by the ones for going back in. -1 while out of sight.
int StakeOutPuzzle::getFrame(const Person &person) const {
	if (_roundState == kAnnouncing || person.place == -1) {
		return -1;
	}

	const Place &place = _places[person.place];
	double fraction = 0.0;
	const Phase phase = getPhase(person, fraction);

	if (phase == kShown) {
		fraction = 1.0;
	} else if (phase != kShowing && phase != kHiding) {
		return -1;
	}

	const int numFrames = phase == kHiding ? place.hideSrcs.size() : place.showSrcs.size();
	if (numFrames == 0) {
		return -1;
	}

	const int frame = MIN((int)(numFrames * CLIP(fraction, 0.0, 1.0)), numFrames - 1);
	return phase == kHiding ? frame + place.showSrcs.size() : frame;
}

void StakeOutPuzzle::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	for (uint i = 0; i < _persons.size(); ++i) {
		const int frame = _shownFrames[i];
		if (frame == -1) {
			continue;
		}

		const Place &place = _places[_persons[i].place];
		const int numShowFrames = place.showSrcs.size();
		const Common::Rect &src = frame < numShowFrames ? place.showSrcs[frame] : place.hideSrcs[frame - numShowFrames];
		_drawSurface.blitFrom(place.image, src, Common::Point(place.dest.left, place.dest.top));
	}

	_needsRedraw = true;
}

void StakeOutPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun: {
		const uint32 now = g_nancy->getTotalPlayTime();
		const uint32 elapsed = now - _lastUpdateTime;
		_lastUpdateTime = now;

		if (elapsed != 0) {
			update(elapsed / 1000.0);
		}

		bool hasChanged = false;
		for (uint i = 0; i < _persons.size(); ++i) {
			const int frame = getFrame(_persons[i]);
			if (frame != _shownFrames[i]) {
				_shownFrames[i] = frame;
				hasChanged = true;
			}
		}

		if (hasChanged) {
			redraw();
		}

		break;
	}
	default:
		break;
	}
}

void StakeOutPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun) {
		return;
	}

	const Common::Rect viewportBounds = NancySceneState.getViewport().getBounds();
	if (!NancySceneState.getViewport().convertViewportToScreen(viewportBounds).contains(input.mousePos)) {
		return;
	}

	if (!_isInputEnabled) {
		setDataCursor(kWaitCursor, false);
		return;
	}

	Common::Rect mouseRect = NancySceneState.getViewport().convertScreenToViewport(
		Common::Rect(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1));
	Common::Point pos = mouseRect.origin();

	for (uint i = 0; i < _persons.size(); ++i) {
		const Person &person = _persons[i];
		if (!isClickable(person, pos)) {
			continue;
		}

		setDataCursor(_hoverCursor);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			// The player tells the others where the person is; whether that was
			// the one to catch shows once the line is said
			_calledPerson = i;
			_isInputEnabled = false;
			playSoundBlock(_places[person.place].sound);

			TableData *table = (TableData *)NancySceneState.getPuzzleData(TableData::getTag());
			if (table) {
				const int16 index = person.isTarget ? _rightCallsTableIndex : _wrongCallsTableIndex;
				table->setSingleValue(index, table->getValueOrZero(index) + 1);
			}

			input.eatMouseInput();
		}

		return;
	}
}

} // End of namespace Action
} // End of namespace Nancy
