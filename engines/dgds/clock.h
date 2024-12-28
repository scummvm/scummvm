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

#ifndef DGDS_CLOCK_H
#define DGDS_CLOCK_H

#include "common/types.h"
#include "common/rect.h"
#include "common/error.h"
#include "common/serializer.h"

namespace Graphics {
class ManagedSurface;
}

namespace Dgds {

class Global;

/* The game clock - drawn to the screen in Rise of the Dragon */
class Clock {
public:
	Clock();
	void addGameTime(int mins);

	void setTime(int16 month, int16 day, int16 hour, int16 minute);

	Common::String getTimeStr() const;
	void draw(Graphics::ManagedSurface &surf);
	void toggleVisibleUser() { _visibleUser = !_visibleUser; }
	void setVisibleScript(bool val) { _visibleScript = val; }

	void update(bool gameRunning);

	int16 getMins() const { return _mins; }

	Global *getMinsGlobal(uint16 num);
	Global *getHoursGlobal(uint16 num);
	Global *getDaysGlobal(uint16 num);
	Global *getDays2Global(uint16 num);
	Global *getGameMinsAddedGlobal(uint16 num);
	Global *getGameTicksUpGlobal(uint16 num);
	Global *getGameTicksDownGlobal(uint16 num);

	const Common::Rect &getPos() const { return _drawPos; }

	Common::Error syncState(Common::Serializer &s);

	Common::String dump() const;

private:
	uint32 _lastPlayTime;
	uint32 _millis;

	int16 _gameMinsAdded;
	int16 _gameTicksUp;
	int16 _gameTicksDown;

	int16 _days;
	int16 _days2;
	int16 _hours;
	int16 _mins;

	Common::Rect _drawPos;

	// Clock is shown if both are true;
	bool _visibleScript;
	bool _visibleUser;
};

} // end namespace Dgds

#endif // DGDS_CLOCK_H
