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

#ifndef BAGEL_MFC_JOYSTICKAPI_H
#define BAGEL_MFC_JOYSTICKAPI_H

#include "bagel/mfc/minwindef.h"
#include "bagel/mfc/winnt.h"

namespace Bagel {
namespace MFC {

typedef int MMRESULT;

/* joystick ID constants */
#define JOYSTICKID1         0
#define JOYSTICKID2         1

/* constants used with JOYINFO and JOYINFOEX structures and MM_JOY* messages */
#define JOY_BUTTON1         0x0001
#define JOY_BUTTON2         0x0002
#define JOY_BUTTON3         0x0004
#define JOY_BUTTON4         0x0008
#define JOY_BUTTON1CHG      0x0100
#define JOY_BUTTON2CHG      0x0200
#define JOY_BUTTON3CHG      0x0400
#define JOY_BUTTON4CHG      0x0800

/* joystick error return values */
#define JOYERR_NOERROR        (0)                  /* no error */
#define JOYERR_PARMS          (JOYERR_BASE+5)      /* bad parameters */
#define JOYERR_NOCANDO        (JOYERR_BASE+6)      /* request not completed */
#define JOYERR_UNPLUGGED      (JOYERR_BASE+7)      /* joystick is unplugged */

/* joystick information data structure */
typedef struct joyinfo_tag {
	unsigned int wXpos;                 /* x position */
	unsigned int wYpos;                 /* y position */
	unsigned int wZpos;                 /* z position */
	unsigned int wButtons;              /* button states */
} JOYINFO, *PJOYINFO, NEAR *NPJOYINFO, FAR *LPJOYINFO;

extern MMRESULT joySetCapture(HWND hwnd,
                              unsigned int uJoyID, unsigned int uPeriod, bool fChanged);
extern MMRESULT joySetThreshold(unsigned int uJoyID, unsigned int uThreshold);
extern MMRESULT joyGetPos(unsigned int uJoyID, LPJOYINFO pji);
extern MMRESULT joyReleaseCapture(unsigned int uJoyID);

} // namespace MFC
} // namespace Bagel

#endif
