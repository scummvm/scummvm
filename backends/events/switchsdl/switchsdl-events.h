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

#if !defined(DISABLE_DEFAULT_EVENTMANAGER)
#define BACKEND_EVENTS_SWITCH_H

#include "backends/events/sdl/sdl-events.h"

#define TOUCHSCREEN_WIDTH 1280
#define TOUCHSCREEN_HEIGHT 720

/**
 * SDL Events manager for the SWITCH.
 */
class SwitchEventSource : public SdlEventSource {
public:
	SwitchEventSource();
	bool pollEvent(Common::Event &event) override;
protected:
	void preprocessEvents(SDL_Event *event) override;
private:

	enum {
		MAX_NUM_FINGERS = 3, // number of fingers to track per panel
		MAX_TAP_TIME = 250, // taps longer than this will not result in mouse click events
		MAX_TAP_MOTION_DISTANCE = 10, // max distance finger motion in Vita screen pixels to be considered a tap
		SIMULATED_CLICK_DURATION = 50, // time in ms how long simulated mouse clicks should be
		MULTIPLIER = 16 // multiplier for sub-pixel resolution
	};

	typedef struct {
		int id; // -1: no touch
		uint32 timeLastDown;
		int lastX; // last known screen coordinates
		int lastY; // last known screen coordinates
		float lastDownX; // SDL touch coordinates when last pressed down
		float lastDownY; // SDL touch coordinates when last pressed down
	} Touch;

	Touch _finger[MAX_NUM_FINGERS]; // keep track of finger status

	typedef enum DraggingType {
		DRAG_NONE = 0,
		DRAG_TWO_FINGER,
		DRAG_THREE_FINGER,
	} DraggingType;

	DraggingType _multiFingerDragging; // keep track whether we are currently drag-and-dropping

	unsigned int _simulatedClickStartTime[2]; // initiation time of last simulated left or right click (zero if no click)

	void preprocessFingerDown(SDL_Event *event);
	void preprocessFingerUp(SDL_Event *event);
	void preprocessFingerMotion(SDL_Event *event);
	void convertTouchXYToGameXY(float touchX, float touchY, int *gameX, int *gameY);
	void finishSimulatedMouseClicks(void);
};

#endif /* BACKEND_EVENTS_SWITCH_H */
