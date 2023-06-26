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

#ifndef M4_CORE_MOUSE_H
#define M4_CORE_MOUSE_H

#include "m4/m4_types.h"

namespace M4 {

struct MouseInfo {
	uint16 Event;
	uint16 ButtonState;
	uint16 CursorColumn;	// x
	uint16 CursorRow;		// y
	uint16 HorizontalMickeyCount;
	uint16 VerticalMickeyCount;
};

struct MouseDriverInfo {
	byte MajorVersion;
	byte MinorVersion;
	byte MouseType;
	byte MouseIRQ;
};

/*
 *  Define call mask bit fields
 */
enum {
	CursorPositionChanged = 0,
	LeftButtonPressed, LeftButtonReleased, RightButtonPressed,
	RightButtonReleased, OEM_MiddleButtonPressed, OEM_MiddleButtonReleased,
	LeftButtonHold, RightButtonHold, OEM_MiddleButtonHold
};

/*
 *  Define call mask values
 */
#define CPC				((Word)(1 << CursorPositionChanged))
#define LBD				((Word)(1 << LeftButtonPressed))
#define LBU				((Word)(1 << LeftButtonReleased))
#define LBH				((int32)(1 << LeftButtonHold))
#define RBD				((Word)(1 << RightButtonPressed))
#define RBU				((Word)(1 << RightButtonReleased))
#define RBH				((int32)(1 << RightButtonHold))
#define MBD				((Word)(1 << OEM_MiddleButtonPressed))
#define MBU				((Word)(1 << OEM_MiddleButtonReleased))
#define MBHold			((int32)(1 << OEM_MiddleButtonHold))

#define LBC				(LBD + LBU)
#define RBC				(RBD + RBU)
#define MBC				(MBD + MBU)

#define MSA				(LBC + RBC + CPC)
#define OEMA			(LBC + RBC + MBC + CPC)


enum mausEvent {
	_ME_no_event,		// 0
	_ME_move,			// 1
	_ME_L_click,		// 2
	_ME_L_hold,			// 3
	_ME_L_drag,			// 4
	_ME_L_release,		// 5
	_ME_R_click,		// 6
	_ME_R_hold,			// 7
	_ME_R_drag,			// 8
	_ME_R_release,		// 9
	_ME_both_click,		// 10
	_ME_both_hold,		// 11
	_ME_both_drag,		// 12
	_ME_both_release,	// 13
	_ME_doubleclick,	// 14
	_ME_doubleclick_hold, // 15
	_ME_doubleclick_drag, // 16
	_ME_doubleclick_release
};

enum mausState {
	_MS_no_event,		// 0
	_MS_L_clickDown,	// 1
	_MS_R_clickDown,	// 2
	_MS_both_clickDown,	// 3
	_MS_doubleclick_Down // 4
};


extern int   mouse_init();
extern void  mouse_uninstall();
extern byte *GetMousePicture();

} // namespace M4

#endif
