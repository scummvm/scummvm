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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/backends/platform/psp/osys_psp.cpp $
 * $Id: osys_psp.cpp 43618 2009-08-21 22:44:49Z joostp $
 *
 */

#ifndef PSP_INPUT_H
#define PSP_INPUT_H

#include "common/scummsys.h"
#include "common/events.h"
#include "backends/platform/psp/display_client.h"
#include "backends/platform/psp/default_display_client.h"
#include "backends/platform/psp/pspkeyboard.h"
#include "backends/platform/psp/cursor.h"
#include <pspctrl.h>

class InputHandler {
public:
	InputHandler() : _cursor(0), _keyboard(0), _prevButtons(0), _lastPadCheckTime(0), _buttonsChanged(0), _dpadX(0), _dpadY(0) {}

	void init();
	bool getAllInputs(Common::Event &event);
	void setKeyboard(PSPKeyboard *keyboard) { _keyboard = keyboard; }
	void setCursor(Cursor *cursor) { _cursor = cursor; }

private:
	Cursor *_cursor;
	PSPKeyboard *_keyboard;
	uint32	_prevButtons;
	uint32	_lastPadCheckTime;
	uint32 _buttonsChanged;
	int32 _dpadX, _dpadY;
	int32 _accelX, _accelY;

	bool getEvent(Common::Event &event, SceCtrlData &pad);
	bool getDpadEvent(Common::Event &event, SceCtrlData &pad);
	bool getButtonEvent(Common::Event &event, SceCtrlData &pad);
	bool getNubEvent(Common::Event &event, SceCtrlData &pad);
	int32 modifyNubAxisMotion(int32 input);
	Common::KeyCode translateDpad(int x, int y);
};

#endif /* PSP_INPUT_H */
