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

#include "asylum/menu.h"
#include "asylum/resource.h"


namespace Asylum {

// eyes animation index table
const uint32 Menu::eyesTable[8] = {3, 5, 1, 7, 4, 8, 2, 6};

Menu::Menu(Screen *screen){
	_screen = screen;
}

Menu::~Menu() {
	if(_bkgImageBuf){
		free(_bkgImageBuf);
	}
	if(_mouseIconBuf){
		free(_mouseIconBuf);
	}
}

void Menu::init(){
	printf("Menu: init()\n");
	
	// TODO: from address 0041A500 (init background music, setup menu environment like palette, font, gamma level, etc.)
}

void Menu::run(){
	//printf("Menu: running...\n");

	// TODO: get background image
	// blit it into back buffer and front buffer
	
	_screen->updateScreen();
}

} // namespace Asylum
