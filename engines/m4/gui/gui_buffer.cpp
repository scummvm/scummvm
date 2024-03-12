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

#include "m4/gui/gui_buffer.h"
#include "m4/gui/gui_vmng.h"
#include "m4/vars.h"

namespace M4 {

bool gui_buffer_system_init() {
	return true;
}

void gui_buffer_system_shutdown() {
}

static void Buffer_Show(void *s, void *r, void *b, int32 destX, int32 destY) {
	ScreenContext *myScreen = (ScreenContext *)s;
	RectList *myRectList = (RectList *)r;
	Buffer *destBuffer = (Buffer *)b;
	Buffer *myBuffer;
	RectList *myRect;

	// Parameter verification
	if (!myScreen) return;
	myBuffer = (Buffer *)(myScreen->scrnContent);
	if (!myBuffer)
		return;

	// If no destBuffer, then draw directly to video
	if (!destBuffer) {
		myRect = myRectList;
		while (myRect) {
			vmng_refresh_video(myRect->x1, myRect->y1, myRect->x1 - myScreen->x1, myRect->y1 - myScreen->y1,
				myRect->x2 - myScreen->x1, myRect->y2 - myScreen->y1, myBuffer);
			myRect = myRect->next;
		}
	} else {
		// Draw to the dest buffer
		myRect = myRectList;
		while (myRect) {
			gr_buffer_rect_copy_2(myBuffer, destBuffer, myRect->x1 - myScreen->x1, myRect->y1 - myScreen->y1,
				destX, destY, myRect->x2 - myRect->x1 + 1, myRect->y2 - myRect->y1 + 1);
			myRect = myRect->next;
		}
	}
}

bool gui_buffer_register(int32 x1, int32 y1, Buffer *myBuf, uint32 scrnFlags, EventHandler evtHandler) {
	int32 x2, y2;

	x2 = x1 + myBuf->w - 1;
	y2 = y1 + myBuf->h - 1;

	if (!vmng_screen_create(x1, y1, x2, y2, SCRN_BUF, scrnFlags | SF_OFFSCRN, (void *)myBuf,
		(RefreshFunc)Buffer_Show, evtHandler))
		return false;

	return true;
}

void GrBuff_Show(void *s, void *r, void *b, int32 destX, int32 destY) {
	ScreenContext *myScreen = (ScreenContext *)s;
	RectList *myRectList = (RectList *)r;
	Buffer *destBuffer = (Buffer *)b;
	GrBuff *myGrBuffer;
	Buffer *myBuffer;
	RectList *myRect;

	// Parameter verification
	if (!myScreen)
		return;

	myGrBuffer = (GrBuff *)myScreen->scrnContent;

	if (!myGrBuffer)
		return;

	myBuffer = myGrBuffer->get_buffer();
	if (!myBuffer)
		return;

	// If no destBuffer, then draw directly to video
	if (!destBuffer) {
		myRect = myRectList;
		while (myRect) {
			myGrBuffer->refresh_video(myRect->x1, myRect->y1, myRect->x1 - myScreen->x1, myRect->y1 - myScreen->y1,
				myRect->x2 - myScreen->x1, myRect->y2 - myScreen->y1);
			myRect = myRect->next;
		}
	} else {
		// Else draw to the dest buffer
		myRect = myRectList;
		while (myRect) {
			gr_buffer_rect_copy_2(myBuffer, destBuffer, myRect->x1 - myScreen->x1, myRect->y1 - myScreen->y1,
				destX, destY, myRect->x2 - myRect->x1 + 1, myRect->y2 - myRect->y1 + 1);
			myRect = myRect->next;
		}
	}

	myGrBuffer->release();
}

bool gui_GrBuff_register(int32 x1, int32 y1, GrBuff *myBuf, uint32 scrnFlags, EventHandler evtHandler) {
	return (vmng_screen_create(x1, y1, x1 + myBuf->w - 1, y1 + myBuf->h - 1,
		SCRN_BUF, scrnFlags | SF_OFFSCRN, (void *)myBuf,
		(RefreshFunc)GrBuff_Show, evtHandler) == nullptr) ? false : true;
}

bool gui_buffer_set_event_handler(void *myBuf, EventHandler evtHandler) {
	ScreenContext *myScreen = vmng_screen_find(myBuf, nullptr);

	if (myScreen == nullptr)
		return false;

	myScreen->evtHandler = evtHandler;
	return true;
}

void gui_buffer_deregister(void *myBuf) {
	vmng_screen_dispose(myBuf);
}

void gui_buffer_activate(Buffer *myBuf) {
	vmng_screen_show((void *)myBuf);
}

bool gui_buffer_add_key(Buffer *myBuf, long myKey, HotkeyCB cb) {
	return AddScreenHotkey((void *)myBuf, myKey, cb);
}

} // End of namespace M4
