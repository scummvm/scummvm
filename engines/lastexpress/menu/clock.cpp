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

#include "lastexpress/menu/clock.h"

#include "lastexpress/data/sequence.h"

#include "lastexpress/game/scenes.h"
#include "lastexpress/game/state.h"

#include "lastexpress/helpers.h"
#include "lastexpress/lastexpress.h"
#include "lastexpress/resource.h"

namespace LastExpress {

ClockOld::ClockOld(LastExpressEngine *engine) : _engine(engine), _frameMinutes(nullptr), _frameHour(nullptr), _frameSun(nullptr), _frameDate(nullptr) {
	_frameMinutes = new SequenceFrame(loadSequence("eggmin.seq"), 0, true);
	_frameHour = new SequenceFrame(loadSequence("egghour.seq"), 0, true);
	if (!_engine->isDemo()) {
		_frameSun = new SequenceFrame(loadSequence("sun.seq"), 0, true);
		_frameDate = new SequenceFrame(loadSequence("datenew.seq"), 0, true);
	}
}

ClockOld::~ClockOld() {
	SAFE_DELETE(_frameMinutes);
	SAFE_DELETE(_frameHour);
	if (!_engine->isDemo()) {
		SAFE_DELETE(_frameSun);
		SAFE_DELETE(_frameDate);
	}

	// Zero passed pointers
	_engine = nullptr;
}

void ClockOld::clear() {
	getScenes()->removeFromQueue(_frameMinutes);
	getScenes()->removeFromQueue(_frameHour);
	if (!_engine->isDemo()) {
		getScenes()->removeFromQueue(_frameSun);
		getScenes()->removeFromQueue(_frameDate);
	}
}

void ClockOld::draw(uint32 time) {
	assert(time >= kTimeCityParis && time <= kTimeCityConstantinople);

	// Check that sequences have been loaded
	if (!_frameMinutes || !_frameHour || (!_engine->isDemo() && !_frameSun) || (!_engine->isDemo() && !_frameDate))
		error("[Clock::draw] Clock sequences have not been loaded correctly");

	// Clear existing frames
	clear();

	// Game starts at: 1037700 = 7:13 p.m. on July 24, 1914
	// Game ends at:   4941000 = 7:30 p.m. on July 26, 1914
	// Game lasts for: 3903300 = 2 days + 17 mins = 2897 mins

	// 15 = 1 second
	// 15 * 60 = 900 = 1 minute
	// 900 * 60 = 54000 = 1 hour
	// 54000 * 24 = 1296000 = 1 day

	// Calculate each sequence index from the current time

	uint8 hour = 0;
	uint8 minute = 0;
	State::getHourMinutes(time, &hour, &minute);
	uint32 index_date = 18 * time / 1296000;
	if (hour == 23)
		index_date += 18 * minute / 60;

	// Set sequences frames
	_frameMinutes->setFrame(minute);
	_frameHour->setFrame((5 * hour + minute / 12) % 60);
	if (!_engine->isDemo()) {
		_frameSun->setFrame((5 * hour + minute / 12) % 120);
		_frameDate->setFrame((uint16)index_date);
	}

	// Adjust z-order and queue
	_frameMinutes->getInfo()->location = 1;
	_frameHour->getInfo()->location = 1;
	if (!_engine->isDemo()) {
		_frameSun->getInfo()->location = 1;
		_frameDate->getInfo()->location = 1;
	}

	getScenes()->addToQueue(_frameMinutes);
	getScenes()->addToQueue(_frameHour);
	if (!_engine->isDemo()) {
		getScenes()->addToQueue(_frameSun);
		getScenes()->addToQueue(_frameDate);
	}
}

Clock::Clock(LastExpressEngine *engine) {
	_engine = engine;
}

Clock::~Clock() {

}

void Clock::startClock(int32 time) {
	_timeTo = time;
	_timeShowing = time;

	if (!_seqMinutes)
		_seqMinutes = _engine->getArchiveManager()->loadSeq("eggmin.seq", 15, 0);

	if (!_seqHour)
		_seqHour = _engine->getArchiveManager()->loadSeq("egghour.seq", 15, 0);

	if (!_seqSun)
		_seqSun = _engine->getArchiveManager()->loadSeq("sun.seq", 15, 0);

	if (!_seqDate)
		_seqDate = _engine->getArchiveManager()->loadSeq("datenew.seq", 15, 0);

	if (!_seqLine1)
		_seqLine1 = _engine->getArchiveManager()->loadSeq("line1.seq", 15, 0);

	if (!_seqLine2)
		_seqLine2 = _engine->getArchiveManager()->loadSeq("line2.seq", 15, 0);

	drawHands(_timeShowing);
	drawTrainPosition(_timeShowing);
}

void Clock::endClock() {
	eraseAllSprites();
	_engine->getSpriteManager()->resetEraseQueue();

	if (_seqMinutes) {
		_engine->getMemoryManager()->freeMem(_seqMinutes->rawSeqData);
		delete _seqMinutes;
		_seqMinutes = nullptr;
	}

	if (_seqHour) {
		_engine->getMemoryManager()->freeMem(_seqHour->rawSeqData);
		delete _seqHour;
		_seqHour = nullptr;
	}

	if (_seqSun) {
		_engine->getMemoryManager()->freeMem(_seqSun->rawSeqData);
		delete _seqSun;
		_seqSun = nullptr;
	}

	if (_seqDate) {
		_engine->getMemoryManager()->freeMem(_seqDate->rawSeqData);
		delete _seqDate;
		_seqDate = nullptr;
	}

	if (_seqLine1) {
		_engine->getMemoryManager()->freeMem(_seqLine1->rawSeqData);
		delete _seqLine1;
		_seqLine1 = nullptr;
	}

	if (_seqLine2) {
		_engine->getMemoryManager()->freeMem(_seqLine2->rawSeqData);
		delete _seqLine2;
		_seqLine2 = nullptr;
	}
}

void Clock::eraseAllSprites() {
	_engine->getSpriteManager()->destroySprite(&_clockHour, false);
	_engine->getSpriteManager()->destroySprite(&_clockMinutes, false);
	_engine->getSpriteManager()->destroySprite(&_clockSun, false);
	_engine->getSpriteManager()->destroySprite(&_clockDate, false);
	_engine->getSpriteManager()->destroySprite(&_trainLine1, false);
	_engine->getSpriteManager()->destroySprite(&_trainLine2, false);
}

void Clock::setClock(int32 time) {
	if (_timeTo == _timeShowing)
		_clockTickDelta = 0;

	_timeTo = time;

	if (_timeShowing != time) {
		if (_engine->getLogicManager()->whoRunningDialog(kCharacterMaster))
			_engine->getLogicManager()->endDialog(kCharacterMaster);

		if (_timeTo >= _timeShowing) {
			_engine->getSoundManager()->playSoundFile("LIB042.SND", kSoundTypeMenu | kSoundFlagFixedVolume | kVolumeFull, kCharacterMaster, 0);
		} else {
			_engine->getSoundManager()->playSoundFile("LIB041.SND", kSoundTypeMenu | kSoundFlagFixedVolume | kVolumeFull, kCharacterMaster, 0);
		}
		
		_engine->getVCR()->updateCurGame(_timeTo, _timeShowing, false);
	}
}

void Clock::stopClock(int32 time) {
	_timeTo = time;
	if (_timeShowing == time)
		tickClock();
}

void Clock::turnOnClock(bool draw) {
	if (draw) {
		drawHands(_timeShowing);
		drawTrainPosition(_timeShowing);
	} else {
		eraseAllSprites();
	}
}

void Clock::drawHands(int32 time) {
	int hours = time % 1296000 / 54000;
	int minutes = time % 54000 / 900 % 60 % 60;
	int date = 18 * (time / 1296000);

	if (hours == 23)
		date += 18 * minutes / 60;

	int minuteFraction = 5 * minutes / 60;
	int hourHandPosition = (minuteFraction + 5 * (time % 1296000 / 54000 % 12)) % 60;
	int sunSpriteIndex = (minuteFraction + 5 * hours) % 120;

	_engine->getSpriteManager()->destroySprite(&_clockHour, false);
	_engine->getSpriteManager()->destroySprite(&_clockMinutes, false);
	_engine->getSpriteManager()->destroySprite(&_clockSun, false);
	_engine->getSpriteManager()->destroySprite(&_clockDate, false);

	if (_seqHour) {
		_clockHour = &_seqHour->sprites[hourHandPosition];
		_clockHour->hotspotPriority = 1;
		_engine->getSpriteManager()->drawSprite(_clockHour);
	}

	if (_seqMinutes) {
		_clockMinutes = &_seqMinutes->sprites[minutes];
		_clockMinutes->hotspotPriority = 1;
		_engine->getSpriteManager()->drawSprite(_clockMinutes);
	}

	if (_seqSun) {
		_clockSun = &_seqSun->sprites[sunSpriteIndex];
		_clockSun->hotspotPriority = 1;
		_engine->getSpriteManager()->drawSprite(_clockSun);
	}

	if (_seqDate) {
		_clockDate = &_seqDate->sprites[date];
		_clockDate->hotspotPriority = 1;
		_engine->getSpriteManager()->drawSprite(_clockDate);
	}
}

void Clock::drawTrainPosition(int32 time) {
	if (_seqLine1 && _seqLine2) {
		int i;
		for (i = 1; _trainLineTimes[i] < time && i < 31; i++);

		int cityTime = _trainLineTimes[i - 1];
		int cityIndex = _trainCitiesIndex[i - 1];

		if (time != cityTime) {
			if (_trainLineTimes[i] != _trainLineTimes[i - 1])
				cityIndex =
				_trainCitiesIndex[i - 1] +
				(int)(
					_trainCitiesIndex[i] - _trainCitiesIndex[i - 1] +
					(_trainCitiesIndex[i] - _trainCitiesIndex[i - 1]) *
					(time - cityTime) - 1
				) / (_trainLineTimes[i] - _trainLineTimes[i - 1]);
		}

		Sprite *oldTrainLine1 = _trainLine1;
		Sprite *oldTrainLine2 = _trainLine2;

		if (cityIndex >= _seqLine1->numFrames) {
			_trainLine1 = &_seqLine1->sprites[_seqLine1->numFrames - 4];
			_trainLine2 = &_seqLine2->sprites[cityIndex - _seqLine1->numFrames];
		} else {
			_trainLine1 = &_seqLine1->sprites[cityIndex];
			_trainLine2 = 0;
		}

		if (_trainLine1 != oldTrainLine1) {
			_engine->getSpriteManager()->destroySprite(&oldTrainLine1, false);
			_trainLine1->hotspotPriority = 1;
			_engine->getSpriteManager()->drawSprite(_trainLine1);
		}

		if (_trainLine2 != oldTrainLine2) {
			_engine->getSpriteManager()->destroySprite(&oldTrainLine2, false);

			if (_trainLine2) {
				_trainLine2->hotspotPriority = 1;
				_engine->getSpriteManager()->drawSprite(_trainLine2);
			}
		}
	}
}

void Clock::tickClock() {
	int32 newTimeShowing;
	int32 oldTimeShowing;
	int32 incrementFactor;

	newTimeShowing = _timeShowing;
	oldTimeShowing = _timeShowing;

	if (_clockTickDelta < 90) {
		incrementFactor = (9 * _clockTickDelta + 89) / 90;
	} else {
		incrementFactor = 9;
	}

	if (_timeTo >= _timeShowing) {
		if (_timeTo > _timeShowing) {
			newTimeShowing = 900 * incrementFactor + _timeShowing;

			_timeShowing = newTimeShowing;
			if (_timeTo < newTimeShowing)
				newTimeShowing = _timeTo;
		}
	} else {
		newTimeShowing = -900 * incrementFactor + _timeShowing;

		_timeShowing = newTimeShowing;
		if (_timeTo > newTimeShowing)
			newTimeShowing = _timeTo;
	}

	_timeShowing = newTimeShowing;
	if (_timeTo == newTimeShowing) {
		newTimeShowing = _timeShowing;

		if (_engine->getLogicManager()->whoRunningDialog(kCharacterMaster)) {
			_engine->getLogicManager()->endDialog(kCharacterMaster);
			newTimeShowing = _timeShowing;
		}
	}

	_timeShowing = newTimeShowing;
	drawHands(newTimeShowing);
	drawTrainPosition(_timeShowing);
	_engine->getSpriteManager()->drawCycle();
	_engine->getVCR()->updateCurGame(_timeShowing, oldTimeShowing, true);
	_clockTickDelta++;
}

bool Clock::statusClock() {
	return _timeTo != _timeShowing;
}

int32 Clock::getTimeShowing() {
	return _timeShowing;
}

int32 Clock::getTimeTo() {
	return _timeTo;
}

} // End of namespace LastExpress
