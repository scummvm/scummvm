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
#include "asylum/graphics.h"
#include "asylum/palette.h"

namespace Asylum {

// eyes animation index table
const uint32 Menu::eyesTable[8] = {3, 5, 1, 7, 4, 8, 2, 6};

Menu::Menu(Screen *screen){
	_screen = screen;
}

Menu::~Menu() {
	delete _res1;
}

void Menu::init(){
	printf("Menu: init()\n");
	
	_res1 = new Resource(1);

	// TODO: from address 0041A500 (init background music, setup menu environment like palette, font, gamma level, etc.)

	Palette *tmpPal = new Palette(_res1->getResource(MENU_PAL_ENTRY));
	memcpy(_palette, tmpPal->_buffer, sizeof(uint8)*256*3);

	// TESTING...
	GraphicResource *gres = new GraphicResource(_res1->getResource(0));
	_screen->setFrontBuffer(0, 0, SCREEN_WIDTH, SCREEN_DEPTH, gres->getGraphicAsset(0).data);
}

void Menu::run(){
	//printf("Menu: running...\n");

	_screen->setPalette(_palette);

	// TODO: get background image
	// blit it into back buffer and front buffer
	
	_screen->updateScreen();
}

} // namespace Asylum
