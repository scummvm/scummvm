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
 */

#ifndef ASYLUM_MENU_H
#define ASYLUM_MENU_H

#include "asylum/screen.h"
#include "asylum/resource.h"

#define MENU_PAL_ENTRY		17

namespace Asylum {

/** Game main menu routines */
class Menu {
public:
    Menu(Screen *screen);
    ~Menu();
	
	/** Initialize menu (music, palette, gamma level,etc.) */
	void init();
	
	/** run menu (blit bkg image, eye orietation, icons display) */
	void run();

private:
	Screen *_screen;

	static const uint32 eyesTable[8];
    
	/** Background image buffer */
	//uint8 _bkgImageBuf[SCREEN_WIDTH*SCREEN_DEPTH];
	
	/** Mouse icon resource image buffer */
	uint8 *_mouseIconBuf;

	/** Menu palette */
	uint8 _palette[PAL_SIZE];

	// FIXME: this resource shouldn't be here. Must be deleted when a single 
	// entry could be extracted from resource withou reading entire file.
	Resource *_res1;
};

} // namespace Asylum

#endif
