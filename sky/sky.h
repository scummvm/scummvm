/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef SKYMAIN_H
#define SKYMAIN_H

#include <stdio.h>
#include "common/engine.h"
#include "common/util.h"

class SkyState : public Engine {

protected:
	byte _game;
	bool _isCDVersion;
	byte _key_pressed;

	uint16 _debugMode;
	uint16 _debugLevel;
	uint16 _language;
	uint _mouse_x, _mouse_y;
	uint _mouse_x_old, _mouse_y_old;
	bool _mouse_pos_changed;
	uint _left_button_down;

	uint8 _palette[1024];

	int _num_screen_updates;

//	int _timer_id;

	FILE *_dump_file;

	int _number_of_savegames;

	int _sdl_mouse_x, _sdl_mouse_y;
	
	byte *_work_screen;
	byte *_backscreen;
	byte *_temp_pal;
	byte *_work_palette;
	byte *_half_palette;

	byte *_game_grid;

public:
	SkyState(GameDetector *detector, OSystem *syst);
	virtual ~SkyState();

protected:
	void delay(uint amount);
	void pollMouseXY();
	void go();
	void convert_palette(uint8 *inpal, uint8* outpal);

	void initialise();
	void initialise_disk();
	void initialise_screen();
	void set_palette(uint8 *pal);
	uint16 *load_file(uint16 file_nr, uint8 *dest);
	uint16 *get_file_info(uint16 file_nr);
	void init_virgin();
	void show_screen();

	static int CDECL game_thread_proc(void *param);

	void shutdown();

	RandomSource _rnd;
};

#endif
