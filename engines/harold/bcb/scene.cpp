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

#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/surface.h"
#include "harold/bcb/bcb.h"
#include "harold/bcb/script.h"
#include "harold/bcb/static_data.h"
#include "video/avi_decoder.h"

#include <math.h>

namespace BCB {

void BCBEngine::notifyGameClockTicksChanged() {
	const int ticks = stateDword(kStateGameClockTicks);
	if (_lastLoggedGameClockTicks == ticks)
		return;

	_lastLoggedGameClockTicks = ticks;
	const int clockTicks = ticks % 1440;
	const int dayOfMonth = 10 + CLIP<int>(stateDword(kStateRequiredDiscIndex), 0, 4);
	const int hour = clockTicks / 60;
	const int minute = clockTicks % 60;
	const Common::String message = Common::String::format("BCB clock: 11/%02d %02d:%02d (ticks=%d / 0x%04x)\n", dayOfMonth, hour, minute, ticks, ticks);
	g_system->logMessage(LogMessageType::kInfo, message.c_str());
}

void BCBEngine::applyPendingClockTickDelta() {
	error("BCB: applyPendingClockTickDelta is not implemented");
}

void BCBEngine::drawClockAndChipIndicator(bool present) {
	error("BCB: drawClockAndChipIndicator is not implemented");
}

bool BCBEngine::drawMainSceneScreen(bool lineReveal) {
	error("BCB: drawMainSceneScreen is not implemented");
}

void BCBEngine::renderClockAndChipToSurface(Graphics::Surface &surface, int baseHourOffset, int gameClockTicks) {
	error("BCB: renderClockAndChipToSurface is not implemented");
}

} // End of namespace BCB
