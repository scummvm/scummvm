/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
/*
 Description:	
 
	Intro sequence scene procedures header file

 Notes: 
*/

#ifndef SAGA_ITE_INTRO_H_
#define SAGA_ITE_INTRO_H_

namespace Saga {

#define R_INTRO_STRMAX 256

#define ITE_INTRO_FRAMETIME 90

#define INTRO_CAPTION_Y 170
#define VOICE_PAD 50
#define VOICE_LETTERLEN 90

#define PALETTE_FADE_DURATION 1000
#define DISSOLVE_DURATION 3000

#define CREDIT_DURATION1 4000

enum R_INTRO_SCENE_DIALOGUE_INFO {

	INTRO_CAVE1_START = 0,
	INTRO_CAVE1_END = 4,

	INTRO_CAVE2_START = 4,
	INTRO_CAVE2_END = 7,

	INTRO_CAVE3_START = 7,
	INTRO_CAVE3_END = 10,

	INTRO_CAVE4_START = 10,
	INTRO_CAVE4_END = 14
};

struct R_INTRO_DIALOGUE {

	ulong i_voice_rn;
	const char *i_cvar_name;
	char i_str[R_INTRO_STRMAX];

};

struct INTRO_CAPTION {

	int res_n;
	char *caption;

};

struct INTRO_CREDIT {

	int text_x;
	int text_y;
	int delta_time;
	int duration;
	const char *string;
	int font_id;

};

} // End of namespace Saga

#endif				/* SAGA_ITE_INTRO_H_ */
