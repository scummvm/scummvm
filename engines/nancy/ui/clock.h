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

#include "engines/nancy/ui/animatedbutton.h"

namespace Nancy {

struct NancyInput;

namespace UI {

class Clock : public RenderObject {
	friend class ClockGlobe;
public:
	Clock();
	virtual ~Clock() = default;

	void init() override;
	void registerGraphics() override;
	void updateGraphics() override;
	void handleInput(NancyInput &input);

	void drawClockHands();

protected:
	class ClockGlobe : public AnimatedButton {
	public:
		ClockGlobe(uint zOrder, Clock *owner) : AnimatedButton(zOrder), _owner(owner), _closeTime(0) {}
		virtual ~ClockGlobe() = default;

		void init() override;
		void updateGraphics() override;
		void onClick() override;
		void onTrigger() override;

	private:
		Clock *_owner;

		uint32 _closeTime;
		uint32 _timeToKeepOpen;
	};

	RenderObject _gargoyleEyes;
	ClockGlobe _globe;

	Common::Array<Common::Rect> _globeSrcRects;
	Common::Array<Common::Rect> _hoursHandSrcRects;
	Common::Array<Common::Rect> _minutesHandSrcRects;
	Common::Array<Common::Rect> _hoursHandDestRects;
	Common::Array<Common::Rect> _minutesHandDestRects;

	Time _playerTime;
};

} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_CLOCK_H
