/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
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

#include "stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"


#if defined(USE_NULL_DRIVER)

/* NULL video driver */
class OSystem_NULL : public OSystem {
public:
	void set_palette(const byte *colors, uint start, uint num) {}
	void init_size(uint w, uint h);
	void copy_rect(const byte *buf, int pitch, int x, int y, int w, int h) {}
	void move_screen(int dx, int dy) {}
	void update_screen() {}
	bool show_mouse(bool visible) { return false; }
	void set_mouse_pos(int x, int y) {}
	void set_mouse_cursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y) {}
	void set_shake_pos(int shake_pos) {}
	uint32 get_msecs();
	void delay_msecs(uint msecs);
	bool poll_event(Event *event) { return false; }
	bool set_sound_proc(SoundProc proc, void *param, SoundFormat format) {}
	void quit() { exit(1); }
	uint32 property(int param, Property *value) { return 0; }
	static OSystem *create(int gfx_mode, bool full_screen);
private:

	uint msec_start;

	uint32 get_ticks();
};

void OSystem_NULL::init_size(uint w, uint h, byte sound) {
	msec_start = get_ticks();
}

uint32 OSystem_NULL::get_ticks() {
	uint a = 0;
#ifdef WIN32
	a = GetTickCount();
#endif
#ifdef UNIX
	struct timeval tv;
	gettimeofday(&tv, NULL);
	a = tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif

	return a;
}

void OSystem_NULL::delay_msecs(uint msecs) {
#ifdef WIN32
	Sleep(msecs);
#endif
#ifdef UNIX
	usleep(msecs * 1000);
#endif
}

uint32 OSystem_NULL::get_msecs() {
	return get_ticks() - msec_start;
}

OSystem *OSystem_NULL_create() {
	return new OSystem_NULL();
}
#else /* USE_NULL_DRIVER */

OSystem *OSystem_NULL_create() {
	return NULL;
}

#endif
