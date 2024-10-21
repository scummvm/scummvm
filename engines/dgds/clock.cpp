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

#include "dgds/clock.h"
#include "dgds/globals.h"
#include "dgds/dgds.h"
#include "dgds/font.h"
#include "dgds/includes.h"
#include "dgds/request.h"

namespace Dgds {

class DragonTimeGlobal : public ReadWriteGlobal<int16> {
public:
	DragonTimeGlobal(uint16 num, int16 *val, Clock &clock) : ReadWriteGlobal<int16>(num, val), _clock(clock) {}
	int16 set(int16 val) override {
		if (val != ReadWriteGlobal::get()) {
			ReadWriteGlobal::set(val);
			// notify that the time changed
			_clock.addGameTime(0);
		}
		return val;
	}

private:
	Clock &_clock;
};

Clock::Clock() : _visibleUser(true), _visibleScript(false), _days(0), _days2(0),
	_hours(0), _mins(0), _gameMinsAdded(0), _gameTicksUp(0), _gameTicksDown(0),
	_lastPlayTime(0), _millis(0)
{
}

Global *Clock::getDaysGlobal(uint16 num) {
	return new DragonTimeGlobal(num, &_days, *this);
}

Global *Clock::getDays2Global(uint16 num) {
	return new ReadWriteGlobal<int16>(num, &_days2);
}

Global *Clock::getHoursGlobal(uint16 num) {
	return new DragonTimeGlobal(num, &_hours, *this);
}

Global *Clock::getMinsGlobal(uint16 num) {
	return new DragonTimeGlobal(num, &_mins, *this);
}

Global *Clock::getGameMinsAddedGlobal(uint16 num) {
	return new ReadOnlyGlobal<int16>(num, &_gameMinsAdded);
}

Global *Clock::getGameTicksUpGlobal(uint16 num) {
	return new ReadOnlyGlobal<int16>(num, &_gameTicksUp);
}

Global *Clock::getGameTicksDownGlobal(uint16 num) {
	return new ReadOnlyGlobal<int16>(num, &_gameTicksDown);
}

static int16 DAYS_PER_MONTH[] = {
	31, 28, 31, 30,
	31, 30, 31, 31,
	30, 31, 30, 31
};

void Clock::addGameTime(int mins) {
	_gameMinsAdded += mins;
	int nhours = (_mins + mins) / 60;
	_mins = (_mins + mins) % 60;
	if (_mins < 0) {
		_mins += 0x3c;
		nhours--;
	}
	int ndays = (_hours + nhours) / 24;
	_hours = (_hours + nhours) % 24;
	if (_hours < 0) {
		_hours += 24;
		_days -= 1;
	}
	_days += ndays;
	// TODO: if any change was made to days/hours/mins..
	//if (plusmins + nhours + ndays != 0)
	//  UINT_39e5_0ffa = 0;
}

Common::String Clock::getTimeStr() const {
	int month = 0;
	int day = _days + _days2 + 1;
	while (day > DAYS_PER_MONTH[month]) {
		day -= DAYS_PER_MONTH[month];
		month++;
		if (month == ARRAYSIZE(DAYS_PER_MONTH))
			month = 0;
	}

	return Common::String::format("%2d/%02d %2d:%02d", month + 1, day, _hours, _mins);
}

void Clock::draw(Graphics::ManagedSurface &surf) {
	if (!_visibleUser || !_visibleScript)
		return;

	const Common::String clockStr = getTimeStr();

	const FontManager *fontman = DgdsEngine::getInstance()->getFontMan();
	const DgdsFont *font = fontman->getFont(FontManager::k4x5Font);
	int width = font->getMaxCharWidth() * 12 + 3;
	_drawPos.top = 0;
	_drawPos.bottom = font->getFontHeight() + 6;
	_drawPos.right = SCREEN_WIDTH;
	_drawPos.left = SCREEN_WIDTH - width;

	RequestData::fillBackground(&surf, _drawPos.left + 2, _drawPos.top + 2, _drawPos.width() - 4, _drawPos.height() - 4, 65);
	font->drawString(&surf, clockStr, _drawPos.left + 3, _drawPos.top + 3, _drawPos.width(), 0);
}

// TODO: This is approximate, work out the exact conversion factor here for better game accuracy.
static const int MILLIS_PER_GAME_MIN = 5000;
static const int MILLIS_PER_TIMER_TICK = 60;

void Clock::update(bool gameRunning) {
	uint32 playTimeNow = g_engine->getTotalPlayTime();
	// These timers are updated whether or not the game is running
	_gameTicksUp = playTimeNow / MILLIS_PER_TIMER_TICK;

	// Is there any reason to make this variable anything other than negative the other one??
	// There seems to be no other way to set them as the are RO globals.
	_gameTicksDown = -_gameTicksUp;

	uint32 lastLastPlayTime = _lastPlayTime;
	_lastPlayTime = playTimeNow;

	if (lastLastPlayTime == 0 || !gameRunning)
		return;

	_millis += playTimeNow - lastLastPlayTime;
	int16 mins_to_add = _millis / MILLIS_PER_GAME_MIN;
	_millis = _millis % MILLIS_PER_GAME_MIN;

	if (mins_to_add)
		addGameTime(mins_to_add);
}

Common::String Clock::dump() {
	return Common::String::format("days %d hours %d mins %d", _days, _mins, _hours);
}

Common::Error Clock::syncState(Common::Serializer &s) {
	s.syncAsUint32LE(_lastPlayTime);
	s.syncAsUint32LE(_millis);

	s.syncAsSint16LE(_gameMinsAdded);
	s.syncAsSint16LE(_gameTicksUp);
	s.syncAsSint16LE(_gameTicksDown);
	s.syncAsSint16LE(_days);
	s.syncAsSint16LE(_days2);
	s.syncAsSint16LE(_hours);
	s.syncAsSint16LE(_mins);

	s.syncAsSint16LE(_drawPos.left);
	s.syncAsSint16LE(_drawPos.top);
	s.syncAsSint16LE(_drawPos.right);
	s.syncAsSint16LE(_drawPos.bottom);

	s.syncAsByte(_visibleScript);
	s.syncAsByte(_visibleUser);

	return Common::kNoError;
}

} // end namespace Dgds
