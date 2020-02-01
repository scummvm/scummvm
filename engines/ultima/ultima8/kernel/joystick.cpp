/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/kernel/joystick.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/shared/std/string.h"

namespace Ultima {
namespace Ultima8 {

//static SDL_Joystick *joy[JOY_LAST] = {0};

void InitJoystick() {
#ifdef TODO
	int i, buttons, axes, balls, hats;
	int joys = SDL_NumJoysticks();

	for (i = 0; i < joys; ++i) {
		if (i >= JOY_LAST) {
			perr << "Additional joysticks detected. Cannot initialize more than "
			     << JOY_LAST << "." << Std::endl;
			break;
		}

		joy[i] = 0;

		if (! SDL_JoystickOpened(i)) {
			joy[i] = SDL_JoystickOpen(i);
			if (joy[i]) {
				buttons = SDL_JoystickNumButtons(joy[i]);
				axes = SDL_JoystickNumAxes(joy[i]);
				balls = SDL_JoystickNumBalls(joy[i]);
				hats = SDL_JoystickNumHats(joy[i]);

				pout << "Initialized joystick " << i + 1 << "." << Std::endl;
				pout << "\tButtons: " << buttons << Std::endl;
				pout << "\tAxes: " << axes << Std::endl;
				pout << "\tBalls: " << balls << Std::endl;
				pout << "\tHats: " << hats << Std::endl;
			} else {
				perr << "Error while initializing joystick " << i + 1 << "."
				     << Std::endl;
			}
		}
	}
#endif
}

void ShutdownJoystick() {
#ifdef TODO
	int i;
	for (i = 0; i < JOY_LAST; ++i) {
		if (joy[i] && SDL_JoystickOpened(i)) {
			SDL_JoystickClose(joy[i]);
		}
		joy[i] = 0;
	}
#endif
}

DEFINE_RUNTIME_CLASSTYPE_CODE(JoystickCursorProcess, Process)

JoystickCursorProcess::JoystickCursorProcess()
	: Process(), js(JOY1), x_axis(0), y_axis(1), ticks(0), accel(0) {
}

JoystickCursorProcess::JoystickCursorProcess(Joystick js_, int x_axis_, int y_axis_)
	: Process(), js(js_), x_axis(x_axis_), y_axis(y_axis_), ticks(0), accel(0) {
#ifdef TODO
	flags |= PROC_RUNPAUSED;
	type = 1;

	if (joy[js] && js < JOY_LAST) {
		int axes = SDL_JoystickNumAxes(joy[js]);
		if (x_axis >= axes && y_axis >= axes) {
			perr << "Failed to start JoystickCursorProcess: illegal axis for x (" << x_axis << ") or y (" << y_axis << ")" << Std::endl;
			terminate();
		}
	} else {
		terminate();
	}
#endif
}

JoystickCursorProcess::~JoystickCursorProcess() {
}

#define AXIS_TOLERANCE 1000

//! CONSTANTS - and a lot of guess work
void JoystickCursorProcess::run() {
#ifdef TODO
	int dx = 0, dy = 0;
	int now = g_system->getMillis();

	if (joy[js] && ticks) {
		int tx = now - ticks;
		int r = 350 - accel * 30;
		int16 jx = SDL_JoystickGetAxis(joy[js], x_axis);
		int16 jy = SDL_JoystickGetAxis(joy[js], y_axis);
		if (jx > AXIS_TOLERANCE || jx < -AXIS_TOLERANCE)
			dx = ((jx / 1000) * tx) / r;
		if (jy > AXIS_TOLERANCE || jy < -AXIS_TOLERANCE)
			dy = ((jy / 1000) * tx) / r;
	}

	ticks = now;

	if (dx || dy) {
		int mx, my;
		Ultima8Engine *app = Ultima8Engine::get_instance();
		app->getMouseCoords(mx, my);
		mx += dx;
		my += dy;
		app->setMouseCoords(mx, my);
		++accel;
		if (accel > 10)
			accel = 10;
	} else {
		accel = 0;
	}
#endif
}

bool JoystickCursorProcess::loadData(IDataSource *ids, uint32 version) {
	if (!Process::loadData(ids, version)) return false;

	terminateDeferred(); // Don't allow this process to continue
	return true;
}

void JoystickCursorProcess::saveData(ODataSource *ods) {
	Process::saveData(ods);
	// saves no status
}

} // End of namespace Ultima8
} // End of namespace Ultima
