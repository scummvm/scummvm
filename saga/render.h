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

// Main rendering loop - private header

#ifndef SAGA_RENDER_H_
#define SAGA_RENDER_H_

namespace Saga {

// Render module CVAR defaults
#define R_FULLSCREEN_DEFAULT 0
#define R_DOUBLERES_DEFAULT  1
#define R_HICOLOR_DEFAULT    1
#define R_SOFTCURSOR_DEFAULT 1

#define R_PAUSEGAME_MSG "PAWS GAME"

struct R_RENDER_MODULE {
	int initialized;

	// Init cvars
	int r_fullscreen;
	int r_softcursor;

	// Module data
	R_SURFACE *r_screen_surface;
	R_SURFACE *r_display_surface;
	R_SURFACE *r_backbuf_surface;

	byte *r_bg_buf;
	int r_bg_buf_w;
	int r_bg_buf_h;
	byte *r_tmp_buf;
	int r_tmp_buf_w;
	int r_tmp_buf_h;

	R_SYSTIMER *r_fps_timer;
	R_SPRITELIST *r_test_sprite;

	unsigned int r_fps;
	unsigned int r_framecount;
	unsigned int r_flags;
	int r_mode;
};

void RENDER_FpsTimer(unsigned long interval, void *param);

} // End of namespace Saga

#endif
