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
 
    Font management and font drawing module public header

 Notes: 
*/

#ifndef SAGA_FONT_MOD_H_
#define SAGA_FONT_MOD_H_

namespace Saga {

enum FONT_ID {

	SMALL_FONT_ID,
	MEDIUM_FONT_ID,
	BIG_FONT_ID
};

enum FONT_EFFECT_FLAGS {

	FONT_NORMAL = 0x00,
	FONT_OUTLINE = 0x01,
	FONT_SHADOW = 0x02,
	FONT_BOLD = 0x04,
	FONT_CENTERED = 0x08
};

int FONT_Init(void);

int FONT_Shutdown(void);

int FONT_Draw(int font_id,
    R_SURFACE * ds,
    const char *draw_str,
    size_t draw_str_len,
    int text_x, int text_y, int color, int effect_color, int flags);

int FONT_GetStringWidth(int font_id,
    const char *test_str, size_t test_str_ct, int flags);

int FONT_GetHeight(int font_id);

} // End of namespace Saga

#endif				/* R_FONT_MOD_H_ */
/* end "r_font_mod.h" */
