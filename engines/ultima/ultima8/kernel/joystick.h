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

#ifndef ULTIMA8_KERNEL_JOYSTICK_H
#define ULTIMA8_KERNEL_JOYSTICK_H

#include "ultima/ultima8/kernel/process.h"

namespace Ultima {
namespace Ultima8 {

/*
   For now, we are limiting to one joystick, 16 buttons,
   and we are ignoring anything other than buttons and axes (hats and balls)
*/

enum Joystick {
	JOY1 = 0,
	JOY_LAST
};

void InitJoystick();
void ShutdownJoystick();

class JoystickCursorProcess : public Process {
public:
	JoystickCursorProcess();
	JoystickCursorProcess(Joystick js_, int x_axis_, int y_axis_);
	~JoystickCursorProcess() override;

	ENABLE_RUNTIME_CLASSTYPE()

	void run() override;

	bool loadData(IDataSource *ids, uint32 version);
protected:
	void saveData(ODataSource *ods) override;

	Joystick js;
	int x_axis, y_axis;
	int ticks;
	int accel;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
