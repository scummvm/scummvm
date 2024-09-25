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

#include "common/scummsys.h"

#if defined(NINTENDO_SWITCH)

#include <math.h>

#include "backends/platform/sdl/switch/switch.h"
#include "backends/events/switchsdl/switchsdl-events.h"
#include "backends/timer/sdl/sdl-timer.h"
#include "backends/platform/sdl/sdl.h"
#include "engines/engine.h"

#include "common/util.h"
#include "common/events.h"
#include "common/config-manager.h"

Common::Point SwitchEventSource::getTouchscreenSize() {
	return Common::Point(1280, 720);
}

bool SwitchEventSource::pollEvent(Common::Event &event) {
	((DefaultTimerManager *) g_system->getTimerManager())->handler();
	return SdlEventSource::pollEvent(event);
}

void SwitchEventSource::convertTouchXYToGameXY(float touchX, float touchY, int *gameX, int *gameY) {
	int screenH = _graphicsManager->getWindowHeight();
	int screenW = _graphicsManager->getWindowWidth();
	Common::Point touchscreenSize = getTouchscreenSize();

	const int dispW = touchscreenSize.x;
	const int dispH = touchscreenSize.y;

	int x, y, w, h;
	float sx, sy;
	float ratio = (float)screenW / (float)screenH;

	h = dispH;
	w = h * ratio;

	x = (dispW - w) / 2;
	y = (dispH - h) / 2;

	sy = (float)h / (float)screenH;
	sx = (float)w / (float)screenW;

	// Find touch coordinates in terms of screen pixels
	float dispTouchX = (touchX * (float)dispW);
	float dispTouchY = (touchY * (float)dispH);

	*gameX = CLIP((int)((dispTouchX - x) / sx), 0, screenW - 1);
	*gameY = CLIP((int)((dispTouchY - y) / sy), 0, screenH - 1);
}

#endif
