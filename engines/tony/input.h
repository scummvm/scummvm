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
 */
/**************************************************************************
 *                                     様様様様様様様様様様様様様様様様様 *
 *                                             Nayma Software srl         *
 *                    e                -= We create much MORE than ALL =- *
 *        u-        z$$$c        '.    様様様様様様様様様様様様様様様様様 *
 *      .d"        d$$$$$b        "b.                                     *
 *   .z$*         d$$$$$$$L        ^*$c.                                  *
 *  #$$$.         $$$$$$$$$         .$$$" Project: Roasted Moths........  *
 *    ^*$b       4$$$$$$$$$F      .d$*"                                   *
 *      ^$$.     4$$$$$$$$$F     .$P"     Module:  Input.CPP............  *
 *        *$.    '$$$$$$$$$     4$P 4                                     *
 *     J   *$     "$$$$$$$"     $P   r    Author:  Giovanni Bajo........  *
 *    z$   '$$$P*4c.*$$$*.z@*R$$$    $.                                   *
 *   z$"    ""       #$F^      ""    '$c                                  *
 *  z$$beu     .ue="  $  "=e..    .zed$$c                                 *
 *      "#$e z$*"   .  `.   ^*Nc e$""                                     *
 *         "$$".  .r"   ^4.  .^$$"                                        *
 *          ^.@*"6L=\ebu^+C$"*b."                                         *
 *        "**$.  "c 4$$$  J"  J$P*"    OS:  [ ] DOS  [X] WIN95  [ ] PORT  *
 *            ^"--.^ 9$"  .--""      COMP:  [ ] WATCOM  [X] VISUAL C++    *
 *                    "                     [ ] EIFFEL  [ ] GCC/GXX/DJGPP *
 *                                                                        *
 * This source code is Copyright (C) Nayma Software.  ALL RIGHTS RESERVED *
 *                                                                        *
 **************************************************************************/

#ifndef TONY_INPUT_H
#define TONY_INPUT_H

#include "tony/utils.h"

namespace Tony {

class RMInput {
private:
//	LPDIRECTINPUT m_DI;
//	LPDIRECTINPUTDEVICE m_DIDKeyboard, m_DIDMouse;

//	DIMOUSESTATE m_mState;
	int m_mX, m_mY;
	bool m_bClampMouse;

	bool m_bLeftClickMouse, m_bLeftReleaseMouse, m_bRightClickMouse, m_bRightReleaseMouse;

private:
	// Inizializza DirectInput
	void DIInit(uint32 hInst);

	// Deinizializza DirectInput
	void DIClose(void);
	
public:
	RMInput();
	~RMInput();

	// Class initialisation
	void Init(uint32 hInst);

	// Closes the class
	void Close(void);

	// Polling (must be performed once per frame)
	void Poll(void);

	// Aquire the DirectInput device
	bool Acquire(void);

	// Deacquires the device
	void Unacquire(void);

	// Reading of the mouse
	RMPoint MousePos() { return RMPoint(m_mX, m_mY); }

	// Current status of the mouse buttons
	bool MouseLeft();
	bool MouseRight();

	// Events of mouse clicks
	bool MouseLeftClicked() { return m_bLeftClickMouse; }
	bool MouseRightClicked() { return m_bRightClickMouse; }
	bool MouseBothClicked() { return m_bLeftClickMouse&&m_bRightClickMouse; }
	bool MouseLeftReleased() { return m_bLeftReleaseMouse; }
	bool MouseRightReleased() { return m_bRightReleaseMouse; }
	bool MouseBothReleased() { return m_bLeftReleaseMouse&&m_bRightReleaseMouse; }

	// Warns when changing from full screen to windowed
	void SwitchFullscreen(bool bFull);

	// Warns when we are in the GDI loop
	void GDIControl(bool bCon);
};

} // End of namespace Tony

#endif
