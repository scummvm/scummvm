/* ScummVMDS - Scumm Interpreter DS Port
 * Copyright (C) 2002-2004 The ScummVM project and Neil Millstone
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
#include <nds.h>
#include "touchkeyboard.h"
#include "keyboard_raw.h"
#include "keyboard_pal_raw.h"
#include "dsmain.h"
#include "osystem_ds.h"

namespace DS {

struct key_data {
	char keyNum;
	char x, y;
	int character;
	bool pressed;
};

#define DS_NUM_KEYS 60
#define DS_SHIFT 0
#define DS_BACKSPACE 8
#define DS_RETURN 13
#define DS_CAPSLOCK 1


key_data keys[DS_NUM_KEYS] = {
	// Key number		x		y		character
	
	// Numbers
	{28,				3,		0,		'1'},
	{29,				5,		0,		'2'},
	{30,				7,		0,		'3'},
	{31,				9,		0,		'4'},
	{32,				11,		0,		'5'},
	{33,				13,		0,		'6'},
	{34,				15,		0,		'7'},
	{35,				17,		0,		'8'},
	{36,				19,		0,		'9'},
	{27,				21,		0,		'0'},
	{45,				23,		0,		SDLK_MINUS},
	{50,				25,		0,		SDLK_EQUALS},
	{52,				27,		0,		SDLK_BACKSPACE},

	// Top row
	{'Q'-'A' + 1,		4,		2,		'Q'},
	{'W'-'A' + 1,		6,		2,		'W'},
	{'E'-'A' + 1,		8,		2,		'E'},
	{'R'-'A' + 1,		10,		2,		'R'},
	{'T'-'A' + 1,		12,		2,		'T'},
	{'Y'-'A' + 1,		14,		2,		'Y'},
	{'U'-'A' + 1,		16,		2,		'U'},
	{'I'-'A' + 1,		18,		2,		'I'},
	{'O'-'A' + 1,		20,		2,		'O'},
	{'P'-'A' + 1,		22,		2,		'P'},
	{43,				24,		2,		SDLK_LEFTBRACKET},
	{44,				26,		2,		SDLK_RIGHTBRACKET},

	// Middle row
	{55,				3,		4,		DS_CAPSLOCK},
	{'A'-'A' + 1,		5,		4,		'A'},
	{'S'-'A' + 1,		7,		4,		'S'},
	{'D'-'A' + 1,		9,		4,		'D'},
	{'F'-'A' + 1,		11,		4,		'F'},
	{'G'-'A' + 1,		13,		4,		'G'},
	{'H'-'A' + 1,		15,		4,		'H'},
	{'J'-'A' + 1,		17,		4,		'J'},
	{'K'-'A' + 1,		19,		4,		'K'},
	{'L'-'A' + 1,		21,		4,		'L'},
	{42,				23,		4,		SDLK_SEMICOLON},
	{41,				25,		4,		SDLK_QUOTE},
	{46,				27,		4,		SDLK_RETURN},

	// Bottom row
	{51,				4,		6,		DS_SHIFT},
	{'Z'-'A' + 1,		6,		6,		'Z'},
	{'X'-'A' + 1,		8,		6,		'X'},
	{'C'-'A' + 1,		10,		6,		'C'},
	{'V'-'A' + 1,		12,		6,		'V'},
	{'B'-'A' + 1,		14,		6,		'B'},
	{'N'-'A' + 1,		16,		6,		'N'},
	{'M'-'A' + 1,		18,		6,		'M'},
	{38,				20,		6,		SDLK_COMMA},
	{39,				22,		6,		SDLK_PERIOD},
	{40,				24,		6,		SDLK_SLASH},

	// Space bar
	{47,				9,		8,		SDLK_SPACE},
	{48,				11,		8,		SDLK_SPACE},
	{48,				13,		8,		SDLK_SPACE},
	{48,				15,		8,		SDLK_SPACE},
	{48,				17,		8,		SDLK_SPACE},
	{49,				19,		8,		SDLK_SPACE},

	// Cursor arrows
	{52,				27,		8,		SDLK_LEFT},
	{54,				29,		8,		SDLK_DOWN},
	{53,				31,		8,		SDLK_RIGHT},
	{51,				29,		6,		SDLK_UP},
	
	// Close button
	{56,				30,		0,		SDLK_UNKNOWN},
	
};

int keyboardX;
int keyboardY;

int mapBase;
int tileBase;

bool shiftState;
bool capsLockState;

bool closed;

void restoreVRAM(int tileBase, int mapBase, u16* saveSpace) {
	for (int r = 0; r < 32 * 32; r++) {
		((u16 *) SCREEN_BASE_BLOCK_SUB(mapBase))[r] = *saveSpace++;
	}
	
	for (int r = 0; r < 4096; r++) {
		((u16 *) CHAR_BASE_BLOCK_SUB(tileBase))[r]	= *saveSpace++;
	}
}

void drawKeyboard(int tileBase, int mapBase, u16* saveSpace) {


	for (int r = 0; r < 32 * 32; r++) {
//		*saveSpace++ = ((u16 *) SCREEN_BASE_BLOCK_SUB(mapBase))[r];
		((u16 *) SCREEN_BASE_BLOCK_SUB(mapBase))[r] = 127;
	}
	
	for (int r = 0; r < 4096; r++) {
//		*saveSpace++ = ((u16 *) CHAR_BASE_BLOCK_SUB(tileBase))[r];
		((u16 *) CHAR_BASE_BLOCK_SUB(tileBase))[r] = ((u16 *) (keyboard_raw))[r];
	}
	
	for (int r = 0; r < 16; r++) {
		BG_PALETTE_SUB[r] = ((u16 *) (keyboard_pal_raw))[r];
	}

	for (int r = 0; r < 16; r++) {
		int col = ((u16 *) (keyboard_pal_raw))[r];
		
		int red = col & 0x001F;
		int green = (col & 0x03E0) >> 5;
		int blue = (col & 0x7C00) >> 10;
		
		red = (red * 8) / 16;
		green = (green * 8) / 16;
		blue = (blue * 8) / 16;
				
		BG_PALETTE_SUB[16 + r] = red | (green << 5) | (blue << 10);
	}
	
	keyboardX = -2;
	keyboardY = 2;
	
	mapBase = mapBase;
	tileBase = tileBase;
	
	shiftState = false;
	capsLockState = false;
	
	int x = keyboardX;
	int y = keyboardY;
	
	u16* base = ((u16 *) SCREEN_BASE_BLOCK_SUB(mapBase));
	
	for (int r = 0; r < DS_NUM_KEYS; r++) {
		base[(y + keys[r].y) * 32 + x + keys[r].x] = keys[r].keyNum * 2;
		base[(y + keys[r].y) * 32 + x + keys[r].x + 1] = keys[r].keyNum * 2 + 1;
		
		base[(y + keys[r].y + 1) * 32 + x + keys[r].x] = 128 + keys[r].keyNum * 2;
		base[(y + keys[r].y + 1) * 32 + x + keys[r].x + 1] = 128 + keys[r].keyNum * 2 + 1;
		
		keys[r].pressed = false;
	}
	
	closed = false;
}

bool getKeyboardClosed() {
	return closed;
}

void setKeyHighlight(int key, bool highlight) {
	u16* base = ((u16 *) SCREEN_BASE_BLOCK_SUB(DS::mapBase));

	if (highlight) {
		base[(keyboardY + keys[key].y) * 32 + keyboardX + keys[key].x] |= 0x1000;
		base[(keyboardY + keys[key].y) * 32 + keyboardX + keys[key].x + 1] |= 0x1000;
		base[(keyboardY + keys[key].y + 1) * 32 + keyboardX + keys[key].x] |= 0x1000;
		base[(keyboardY + keys[key].y + 1) * 32 + keyboardX + keys[key].x + 1] |= 0x1000;
	} else {
		base[(keyboardY + keys[key].y) * 32 + keyboardX + keys[key].x] &= ~0x1000;
		base[(keyboardY + keys[key].y) * 32 + keyboardX + keys[key].x + 1] &= ~0x1000;
		base[(keyboardY + keys[key].y + 1) * 32 + keyboardX + keys[key].x] &= ~0x1000;
		base[(keyboardY + keys[key].y + 1) * 32 + keyboardX + keys[key].x + 1] &= ~0x1000;
	}
}

void addKeyboardEvents() {
	if (DS::getPenDown()) {
		int x = IPC->touchXpx;
		int y = IPC->touchYpx;
		
		int tx = (x >> 3) - keyboardX;
		int ty = (y >> 3) - keyboardY;
		
//		consolePrintf("x=%d y=%d\n", tx, ty);
		
		for (int r = 0; r < DS_NUM_KEYS; r++) {
			if (( (tx >= keys[r].x) && (tx <= keys[r].x + 1)) && 
				   (ty >= keys[r].y) && (ty <= keys[r].y + 1)) {
			   	OSystem_DS* system = OSystem_DS::instance();
				OSystem::Event event;
				
//				consolePrintf("Key: %d\n", r);
				if ((keys[r].character == SDLK_UNKNOWN)) {
					// Close button
					DS::closed = true;
				} else	if ((keys[r].character >= '0') && (keys[r].character <= '9')) {
					event.kbd.ascii = keys[r].character;
					event.kbd.keycode = 0;
				
				} else if ((keys[r].character >= 'A') && (keys[r].character <= 'Z')) {
					
					if ((!DS::shiftState) && (!DS::capsLockState)) {
						event.kbd.ascii = keys[r].character + 32; // Make key lowercase.
					} else {
						event.kbd.ascii = keys[r].character;
					}
					
					event.kbd.keycode = event.kbd.ascii;
				} else {
					event.kbd.ascii = keys[r].character;
					event.kbd.keycode = keys[r].character;
				}				
			
				
				
				//event.kbd.keycode = keys[r].character;		
				//event.kbd.ascii = keys[r].character;		
				event.type = OSystem::EVENT_KEYDOWN;
				event.kbd.flags = 0;
				system->addEvent(event);

				event.type = OSystem::EVENT_KEYUP;
				system->addEvent(event);
				
				switch (keys[r].character) {
					case DS_SHIFT: {
						DS::shiftState = !DS::shiftState;
						DS::setKeyHighlight(r, DS::shiftState);
						break;
					}
					
					case DS_CAPSLOCK: {
						DS::capsLockState = !DS::capsLockState;
						DS::setKeyHighlight(r, DS::capsLockState);
						break;
					}
					
					default: {
						DS::setKeyHighlight(r, true);
						keys[r].pressed = true;
						
						if (DS::shiftState) {
							DS::shiftState = false;
							for (int t = 0; t < DS_NUM_KEYS; t++) {
								if (keys[t].character == DS_SHIFT) {
									DS::setKeyHighlight(t, false);
								}
							}
						}
						break;
					}
				}
				
			}
		}
	}
	
	if (DS::getPenReleased()) {
		for (int r = 0; r < DS_NUM_KEYS; r++) {
			if (keys[r].pressed) {
				DS::setKeyHighlight(r, false);
				keys[r].pressed = false;
			}
		}	
	}
}

}

