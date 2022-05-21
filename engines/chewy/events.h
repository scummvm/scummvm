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

#ifndef CHEWY_EVENTS_H
#define CHEWY_EVENTS_H

#include "common/events.h"
#include "common/queue.h"
#include "graphics/screen.h"

namespace Chewy {

struct KbdInfo {
	char _keyCode = '\0';
	int _scanCode = Common::KEYCODE_INVALID;
};

typedef void (*TimerProc)();

class EventsManager {
	struct TimerRecord {
		TimerProc _proc;
		uint32 _interval;
		uint32 _nextFrameTime;
		TimerRecord(TimerProc proc, uint32 interval) : _proc(proc), _interval(interval), _nextFrameTime(0) {
		}
	};
	typedef Common::List<TimerRecord> TimerList;

private:
	void init_timer_handler();
	static void timer_handler();

	void handleMouseEvent(const Common::Event &event);
	void handleKbdEvent(const Common::Event &event);

	TimerList _timers;
	Common::Queue<Common::Event> _pendingEvents;
	Common::Queue<Common::Event> _pendingKeyEvents;

	/**
	 * Checks for timer expiries
	 */
	void checkTimers();

	/**
	 * Timer proc for regularly updating the screen
	 */
	static void updateScreen();

	/**
	 * Adds a new timer method
	 * @param proc		Timer method to execute
	 * @param interval	Interval in milliseconds between calls
	 */
	void addTimer(TimerProc proc, uint interval) {
		_timers.push_back(TimerRecord(proc, interval));
	}

	/**
	 * Process any pending events
	 */
	void processEvents();

	/**
	 * Handles pending event
	 */
	void handleEvent(const Common::Event &event);

public:
	KbdInfo _kbInfo;
	Common::Point _mousePos;
	bool _flag1 = false;
	bool _flag2 = false;

	EventsManager(Graphics::Screen *screen, uint refreshRate = 1000 / 50);
	virtual ~EventsManager();

	void delay(size_t time);

	Graphics::Screen *_screen;

	/**
	 * Handles doing a brief delay, checking for timer updates,
	 * and polling events
	 */
	void update();

	/**
	 * Returns true if any unprocessed keyboard events are pending
	 */
	bool keyEventPending() {
		processEvents();
		return !_pendingKeyEvents.empty();
	}

	/**
	 * Returns true if any unprocessed event other than key events
	 * are pending
	 */
	bool eventPending() {
		processEvents();
		return !_pendingEvents.empty();
	}

	/**
	 * Returns the next pending unprocessed keyboard event
	 */
	Common::Event getPendingKeyEvent() {
		processEvents();
		return _pendingKeyEvents.empty() ? Common::Event() : _pendingKeyEvents.pop();
	}

	/**
	 * Returns the next event, if any
	 */
	Common::Event getPendingEvent() {
		processEvents();
		return _pendingEvents.empty() ? Common::Event() : _pendingEvents.pop();
	}

	/**
	 * Sets the mouse position
	 */
	void warpMouse(const Common::Point &newPos);

	/**
	 * Clear any pending events
	 */
	void clearEvents();
};

extern EventsManager *g_events;

extern void delay(size_t time);

extern bool kbhit();
extern char getch();

#define EVENTS_UPDATE g_events->update()
#define EVENTS_CLEAR g_events->clearEvents()
#define ALT 0x1000

} // namespace Chewy

#endif
