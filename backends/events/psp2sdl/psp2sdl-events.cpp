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

#if defined(PSP2)

#include <psp2/kernel/processmgr.h>
#include <psp2/touch.h>
#include "backends/platform/sdl/psp2/psp2.h"
#include "backends/events/psp2sdl/psp2sdl-events.h"
#include "backends/platform/sdl/sdl.h"
#include "engines/engine.h"

#include "common/util.h"
#include "common/events.h"
#include "common/config-manager.h"

#include "math.h"

Common::Point PSP2EventSource::getTouchscreenSize() {
	return Common::Point(960, 544);
}

void PSP2EventSource::preprocessEvents(SDL_Event *event) {

	// prevent suspend (scummvm games contain a lot of cutscenes..)
	sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DISABLE_AUTO_SUSPEND);
	sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DISABLE_OLED_OFF);

	SdlEventSource::preprocessEvents(event);
}

bool PSP2EventSource::isTouchPortTouchpadMode(SDL_TouchID port) {
	return port != 0 || ConfMan.getBool("frontpanel_touchpad_mode");
}

bool PSP2EventSource::isTouchPortActive(SDL_TouchID port) {
	return port == 0 || ConfMan.getBool("touchpad_mouse_mode");
}

void PSP2EventSource::convertTouchXYToGameXY(float touchX, float touchY, int *gameX, int *gameY) {
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
