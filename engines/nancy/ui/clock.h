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

#ifndef NANCY_UI_CLOCK_H
#define NANCY_UI_CLOCK_H

#include "engines/nancy/renderobject.h"
#include "engines/nancy/enginedata.h"
#include "engines/nancy/time.h"

#include "engines/nancy/ui/animatedbutton.h"

namespace Nancy {

struct NancyInput;

namespace UI {

class Clock : public RenderObject {
	friend class ClockAnim;
public:
	Clock();
	virtual ~Clock() = default;

	void init() override;
	void registerGraphics() override;
	void updateGraphics() override;
	void handleInput(NancyInput &input);

	// Used to disable the UI clock when a scene can change the in-game time (e.g. SetPlayerClock)
	void lockClock(bool val) { _locked = val; }

	void drawClockHands();

protected:
	class ClockAnim : public AnimatedButton {
	public:
		ClockAnim(uint zOrder, Clock *owner) : AnimatedButton(zOrder), _owner(owner), _closeTime(0), _timeToKeepOpen(0) {}
		virtual ~ClockAnim() = default;

		void init() override;
		void updateGraphics() override;
		void onClick() override;
		void onTrigger() override;

	private:
		Clock *_owner;

		uint32 _closeTime;
		uint32 _timeToKeepOpen;
	};

	const CLOK *_clockData;
	ClockAnim _animation;

	// Used for gargoyle eyes in TVD, inside of watch in nancy2 and up
	RenderObject _staticImage;

	Time _playerTime;
	bool _locked;
};

// Separate class since it's not actually a clock, and is non-interactable. Instead, this shows which
// in-game day it currently is, and also displays a countdown during the endgame
class Nancy5Clock : public RenderObject {
public:
	Nancy5Clock() : RenderObject(10) {}
	virtual ~Nancy5Clock() = default;

	void init() override;
	void updateGraphics() override;

private:
	int32 _currentDay = -1;
	int32 _countdownProgress = -1;

	const CLOK *_clockData = nullptr;
};

} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_CLOCK_H
