/* Copyright (C) 2023 Giovanni Cascione <ing.cascione@gmail.com>
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

#include <unistd.h>
#include <features/features_cpu.h>

#include "common/list.h"
#include "common/events.h"

#include "backends/platform/libretro/include/libretro-defs.h"
#include "backends/platform/libretro/include/libretro-core.h"
#include "backends/platform/libretro/include/libretro-os.h"
#include "backends/platform/libretro/include/libretro-timer.h"
#include "backends/platform/libretro/include/libretro-graphics-surface.h"
#ifdef USE_OPENGL
#include "backends/platform/libretro/include/libretro-graphics-opengl.h"
#endif

bool OSystem_libretro::pollEvent(Common::Event &event) {
	((LibretroTimerManager *)_timerManager)->checkThread(THREAD_SWITCH_POLL);
	if (!_events.empty()) {
		event = _events.front();
		_events.pop_front();
		return true;
	}

	return false;
}

uint32 OSystem_libretro::getMillis(bool skipRecord) {
	return (uint32)(cpu_features_get_time_usec() / 1000) - _startTime;
}

void OSystem_libretro::delayMillis(uint msecs) {
	uint32 start_time = getMillis();
	uint32 elapsed_time = 0;

	if (retro_setting_get_timing_inaccuracies_enabled()) {
		while (elapsed_time < msecs) {
			/* When remaining delay would take us past the next thread switch time, we switch immediately
			in order to burn as much as possible delay time in the main RetroArch thread as soon as possible. */
			if (msecs - elapsed_time >= ((LibretroTimerManager *)_timerManager)->timeToNextSwitch() && !isOverlayInGUI())
				((LibretroTimerManager *)_timerManager)->checkThread(THREAD_SWITCH_DELAY);
			else
				usleep(1000);

			/* Actual delay provided will be lower than requested: elapsed time is calculated cumulatively.
			i.e. the higher the requested delay, the higher the actual delay reduction */
			elapsed_time += getMillis() - start_time;
		}
	} else {
		while (elapsed_time < msecs) {
			/* if remaining delay is lower than last amount of time spent on main thread, burn it in emu thread
			to avoid exceeding requested delay */
			if (msecs - elapsed_time >= ((LibretroTimerManager *)_timerManager)->spentOnMainThread() && !((LibretroTimerManager *)_timerManager)->timeToNextSwitch() && !isOverlayInGUI())
				((LibretroTimerManager *)_timerManager)->checkThread(THREAD_SWITCH_DELAY);
			else
				usleep(1000);
			elapsed_time = getMillis() - start_time;
		}
	}
}

Common::MutexInternal *OSystem_libretro::createMutex(void) {
	return new LibretroMutexInternal();
}

void OSystem_libretro::requestQuit() {
	Common::Event ev;
	ev.type = Common::EVENT_QUIT;
	LIBRETRO_G_SYSTEM->getEventManager()->pushEvent(ev);
}

void OSystem_libretro::resetQuit() {
	LIBRETRO_G_SYSTEM->getEventManager()->resetQuit();
}

void OSystem_libretro::setMousePosition(int x, int y) {
#ifdef USE_OPENGL
	if (retro_get_video_hw_mode() & VIDEO_GRAPHIC_MODE_REQUEST_HW)
		dynamic_cast<LibretroOpenGLGraphics *>(_graphicsManager)->setMousePosition(x, y);
	else
#endif
		dynamic_cast<LibretroGraphics *>(_graphicsManager)->setMousePosition(x, y);
}
