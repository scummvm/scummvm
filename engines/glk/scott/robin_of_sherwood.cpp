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

/*
 * Based on ScottFree interpreter version 1.14 developed by Swansea
 * University Computer Society without disassembly of any other game
 * drivers, only of game databases as permitted by EEC law (for purposes
 * of compatibility).
 *
 * Licensed under GPLv2
 *
 * https://github.com/angstsmurf/spatterlight/tree/master/terps/scott
 */

#include "glk/scott/scott.h"
#include "glk/scott/types.h"
#include "glk/scott/globals.h"
#include "glk/scott/resource.h"
#include "glk/scott/saga_draw.h"
#include "glk/scott/decompress_text.h"
#include "glk/scott/robin_of_sherwood.h"

namespace Glk {
namespace Scott {

#define WATERFALL_ANIMATION_RATE 15

void animateLightning(int stage);

void sherwoodAction(int p) {
	event_t ev;

	switch (p) {
	case 0:
		// Flash animation
		_G(_animationFlag) = 1;
		g_scott->glk_request_timer_events(15);

		while (_G(_animationFlag) < 11) {
			g_scott->glk_select(&ev);
			if (ev.type == evtype_Timer) {
				_G(_animationFlag)++;
				animateLightning(_G(_animationFlag));
			}
		}
		break;
	case 1:
		g_scott->drawImage(0); /* Herne */
		g_scott->display(_G(_bottomWindow), "\n%s\n", _G(_sys)[HIT_ENTER].c_str());
		g_scott->hitEnter();
		_G(_items)[39]._location = 79;
		g_scott->look();
		break;
	case 2:
		// Climbing tree in forest
		_G(_savedRoom) = MY_LOC;
		MY_LOC = 93;
		g_scott->look();
		break;
	default:
		error("sherwoodAction: Unhandled SherwoodAction %d!", p);
		break;
	}
}

int isForestLocation(void) { return (MY_LOC >= 11 && MY_LOC <= 73); }

#define TREES 0
#define BUSHES 1

void drawSherwood(int loc) {
	g_scott->glk_window_clear(_G(_graphics));
	int subimage_index = 0;

	for (int i = 0; i < loc - 11; i++) {
		// BUSHES type images are made up of 5 smaller images
		int skip = 5;
		if (_G(_forestImages)[subimage_index] < 128)
			// Those with bit 7 unset have 10 (trees only) or 11 (trees with path)
			skip = 11;
		subimage_index += skip;
	}

	int forest_type = TREES;
	int subimages;

	// if bit 7 of the image index is set then this image is BUSHES
	if (_G(_forestImages)[subimage_index] >= 128) {
		forest_type = BUSHES;
		subimages = 5;
		// if the last subimage value is 255, there is no path
	} else if (_G(_forestImages)[subimage_index + 10] == 0xff) {
		// Trees only
		subimages = 10;
	} else {
		// Trees with path
		subimages = 11;
	}

	int xpos = 0, ypos = 0, image_number;

	for (int i = 0; i < subimages; i++) {
		if (forest_type == TREES) {
			if (i >= 8) {
				if (i == 8) { // Undergrowth
					ypos = 7;
					xpos = 0;
				} else if (i == 9) { // _G(_bottomWindow) path
					ypos = 10;
					xpos = 0;
				} else { // Forward path
					ypos = 7;
					xpos = 12;
				}
			} else { // Trees (every tree image is 4 characters wide)
				ypos = 0;
				xpos = i * 4;
			}
		}

		image_number = _G(_forestImages)[subimage_index++] & 127;

		drawSagaPictureAtPos(image_number, xpos, ypos);

		if (forest_type == BUSHES) {
			xpos += _G(_images)[image_number]._width;
		}
	}
}

void animateWaterfall(int stage) {
	rectFill(88, 16, 48, 64, _G(_whiteColour));
	for (int line = 2; line < 10; line++) {
		for (int col = 11; col < 17; col++) {
			for (int i = 0; i < 8; i++)
				for (int j = 0; j < 8; j++)
					if ((_G(_screenchars)[col + line * 32][i] & (1 << j)) != 0) {
						int ypos = line * 8 + i + stage;
						if (ypos > 79)
							ypos = ypos - 64;
						putPixel(col * 8 + j, ypos, _G(_blueColour));
					}
		}
	}
}

void animateWaterfallCave(int stage) {
	rectFill(248, 24, 8, 64, _G(_whiteColour));
	for (int line = 3; line < 11; line++) {
		for (int i = 0; i < 8; i++)
			for (int j = 0; j < 8; j++)
				if ((_G(_screenchars)[31 + line * 32][i] & (1 << j)) != 0) {
					int ypos = line * 8 + i + stage;
					if (ypos > 87)
						ypos = ypos - 64;
					putPixel(248 + j, ypos, _G(_blueColour));
				}
	}
}

void animateLightning(int stage) {
	// swich blue and bright yellow
	if (_G(_palChosen) == C64B)
		switchPalettes(6, 7);
	else {
		switchPalettes(1, 14);
		switchPalettes(9, 6);
	}
	drawSagaPictureNumber(77);
	if (stage == 11) {
		g_scott->glk_request_timer_events(0);
	} else if (stage == 3) {
		g_scott->glk_request_timer_events(700);
	} else {
		g_scott->glk_request_timer_events(40);
	}
}

void robinOfSherwoodLook(void) {
	if (!isForestLocation()) {
		if (_G(_rooms)[MY_LOC]._image == 255) {
			g_scott->closeGraphicsWindow();
		} else {
			g_scott->drawImage(_G(_rooms)[MY_LOC]._image);
			for (int ct = 0; ct <= _G(_gameHeader)->_numItems; ct++)
				if (_G(_items)[ct]._image) {
					if ((_G(_items)[ct]._flag & 127) == MY_LOC && _G(_items)[ct]._location == MY_LOC) {
						g_scott->drawImage(_G(_items)[ct]._image);
					}
				}
		}
	}

	if (MY_LOC == 82) // Dummy room where exit from Up a tree goes
		MY_LOC = _G(_savedRoom);
	if (MY_LOC == 93) // Up a tree
		for (int i = 0; i < _G(_gameHeader)->_numItems; i++)
			if (_G(_items)[i]._location == 93)
				_G(_items)[i]._location = _G(_savedRoom);
	if (MY_LOC == 7 && _G(_items)[62]._location == 7) // Left bedroom, open treasure chest
		g_scott->drawImage(70);
	if (isForestLocation()) {
		g_scott->openGraphicsWindow();
		drawSherwood(MY_LOC);

		if (_G(_items)[36]._location == MY_LOC) {
			//"Gregory the tax collector with his horse and cart"
			g_scott->drawImage(15); // Horse and cart
			g_scott->drawImage(3);  // Sacks of grain
		}
		if (_G(_items)[60]._location == MY_LOC || _G(_items)[77]._location == MY_LOC) {
			// "A serf driving a horse and cart"
			g_scott->drawImage(15); // Horse and cart
			g_scott->drawImage(12); // Hay
		}
		if (MY_LOC == 73) {
			// Outlaws camp
			g_scott->drawImage(36); // Campfire
		}
	}

	if (MY_LOC == 86 || MY_LOC == 79) {
		g_scott->glk_request_timer_events(WATERFALL_ANIMATION_RATE);
	}
}

void updateRobinOfSherwoodAnimations(void) {
	_G(_animationFlag)++;
	if (_G(_animationFlag) > 63)
		_G(_animationFlag) = 0;
	if (MY_LOC == 86 || MY_LOC == 79 || MY_LOC == 84) {
		/* If we're in room 84, the stone circle, we just */
		/* want the timer to not switch off */
		if (MY_LOC == 86) {
			animateWaterfall(_G(_animationFlag));
		} else if (MY_LOC == 79) {
			animateWaterfallCave(_G(_animationFlag));
		}
	} else {
		g_scott->glk_request_timer_events(0);
	}
}

GameIDType loadExtraSherwoodData(void) {
	int offset = 0x3d99 + _G(_fileBaselineOffset);
	uint8_t *ptr;
	/* Load the room images */

	ptr = seekToPos(_G(_entireFile), offset);
	if (ptr == 0)
		return UNKNOWN_GAME;

	int ct;
	Room *rp = &_G(_rooms)[0];

	for (ct = 0; ct <= _G(_gameHeader)->_numRooms; ct++) {
		rp->_image = *(ptr++);
		rp++;
		if (ct == 10) {
			for (int i = 0; i < 63; i++) {
				rp++;
				ct++;
			}
		}
	}

	ct = 0;
	rp = &_G(_rooms)[0];

	//int actual_room_number = 0;

	ptr = seekToPos(_G(_entireFile), 0x5b7e + _G(_fileBaselineOffset));
	if (ptr == 0)
		return UNKNOWN_GAME;

	do {
		rp->_text = decompressText(ptr, ct);
		rp->_text.toLowercase();
		ct++;
		//actual_room_number++;
		if (ct == 11) {
			for (int i = 0; i < 61; i++) {
				rp++;
				rp->_text = "in Sherwood Forest";
				//actual_room_number++;
			}
		}
		rp++;
	} while (ct < 33);

	for (int i = I_DONT_UNDERSTAND; i <= THATS_BEYOND_MY_POWER; i++)
		_G(_sys)[i] = _G(_systemMessages)[4 - I_DONT_UNDERSTAND + i];

	for (int i = YOU_SEE; i <= HIT_ENTER; i++)
		_G(_sys)[i] = _G(_systemMessages)[15 - YOU_SEE + i];

	_G(_sys)[OK] = _G(_systemMessages)[2];
	_G(_sys)[TAKEN] = _G(_systemMessages)[2];
	_G(_sys)[DROPPED] = _G(_systemMessages)[2];
	_G(_sys)[PLAY_AGAIN] = _G(_systemMessages)[3];
	_G(_sys)[YOURE_CARRYING_TOO_MUCH] = _G(_systemMessages)[21];
	_G(_sys)[YOU_CANT_GO_THAT_WAY] = _G(_systemMessages)[12];
	_G(_sys)[YOU_ARE] = _G(_systemMessages)[13];
	_G(_sys)[EXITS_DELIMITER] = " ";
	_G(_sys)[MESSAGE_DELIMITER] = ". ";

	ptr = seekToPos(_G(_entireFile), 0x3b6e + _G(_fileBaselineOffset));
	if (ptr == 0)
		return UNKNOWN_GAME;

	int cells = 555;
	_G(_forestImages) = new uint8_t[cells];

	for (int i = 0; i < cells; i++)
		_G(_forestImages)[i] = *(ptr++);

	return ROBIN_OF_SHERWOOD;
}

GameIDType loadExtraSherwoodData64(void) {

	//    white_colour = 1;
	//    blue = 6;

	int offset = 0x1ffd + _G(_fileBaselineOffset);
	uint8_t *ptr;
	/* Load the room images */

	ptr = seekToPos(_G(_entireFile), offset);
	if (ptr == 0)
		return UNKNOWN_GAME;

	int ct;
	Room *rp = &_G(_rooms)[0];

	for (ct = 0; ct <= _G(_gameHeader)->_numRooms; ct++) {
		rp->_image = *(ptr++);
		rp++;

		if (ct == 10) {
			for (int i = 0; i < 63; i++) {
				rp++;
				ct++;
			}
		}
	}

	ct = 0;
	rp = &_G(_rooms)[0];

	//int actual_room_number = 0;

	offset = 0x402e + _G(_fileBaselineOffset);

	ptr = seekToPos(_G(_entireFile), offset);
	if (ptr == 0)
		return UNKNOWN_GAME;

	do {
		rp->_text = decompressText(ptr, ct);
		rp->_text.toLowercase();
		ct++;
		//actual_room_number++;
		if (ct == 11) {
			for (int i = 0; i < 61; i++) {
				rp++;
				rp->_text = "in Sherwood Forest";
				//actual_room_number++;
			}
		}
		rp++;
	} while (ct < 33);

	SysMessageType messagekey[] = {NORTH,
								   SOUTH,
								   EAST,
								   WEST,
								   UP,
								   DOWN,
								   EXITS,
								   YOU_SEE,
								   YOU_ARE,
								   HIT_ENTER,
								   YOU_CANT_GO_THAT_WAY,
								   OK,
								   WHAT_NOW,
								   HUH,
								   YOU_HAVE_IT,
								   TAKEN,
								   DROPPED,
								   YOU_HAVENT_GOT_IT,
								   INVENTORY,
								   YOU_DONT_SEE_IT,
								   THATS_BEYOND_MY_POWER,
								   DIRECTION,
								   YOURE_CARRYING_TOO_MUCH,
								   PLAY_AGAIN,
								   RESUME_A_SAVED_GAME,
								   YOU_CANT_DO_THAT_YET,
								   I_DONT_UNDERSTAND,
								   NOTHING};

	for (int i = 0; i < 26; i++) {
		_G(_sys)[messagekey[i]] = _G(_systemMessages)[i];
	}

	_G(_sys)[HIT_ENTER] = _G(_systemMessages)[30];
	_G(_sys)[WHAT] = _G(_systemMessages)[13];

	_G(_sys)[EXITS_DELIMITER] = " ";
	_G(_sys)[MESSAGE_DELIMITER] = ". ";

	offset = 0x2300 + _G(_fileBaselineOffset);

	ptr = seekToPos(_G(_entireFile), offset);
	if (ptr == 0)
		return UNKNOWN_GAME;

	int cells = 555;
	_G(_forestImages) = new uint8_t[cells];

	for (int i = 0; i < cells; i++) {
		_G(_forestImages)[i] = *(ptr++);
	}

	return ROBIN_OF_SHERWOOD_C64;
}

} // End of namespace Scott
} // End of namespace Glk
