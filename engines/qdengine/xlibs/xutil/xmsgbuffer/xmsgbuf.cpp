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


/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qdengine/xlibs/xutil/xglobal.h"

namespace QDEngine {

/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */

void xtPostMessage(HANDLE hWnd, int msg, int wp, int lp);

/* --------------------------- DEFINITION SECTION --------------------------- */

XMessageBuffer* XMsgBuf = NULL;

XMessageBuffer::XMessageBuffer(void) {
	int i;
	char* heap = new char[XMSG_BUFFER_SIZE * sizeof(MSG)];

	table = new MSG*[XMSG_BUFFER_SIZE];
	for (i = 0; i < XMSG_BUFFER_SIZE; i ++)
		table[i] = (MSG *)(heap + i * sizeof(MSG));

	clear();
}

void XMessageBuffer::put(MSG* p) {
	if (Size < XMSG_BUFFER_SIZE) {
		memcpy((char*)table[LastIndex], (char*)p, sizeof(MSG));
		LastIndex ++;
		if (LastIndex >= XMSG_BUFFER_SIZE)
			LastIndex = 0;
		Size ++;
	} else
		ErrH.Abort("XMessageBuffer overflow...");
}

void XMessageBuffer::put(HANDLE hWnd, int msg, int wParam, int lParam) {
	if (Size < XMSG_BUFFER_SIZE) {
		table[LastIndex] -> hwnd = (HWND)hWnd;
		table[LastIndex] -> message = msg;
		table[LastIndex] -> wParam = wParam;
		table[LastIndex] -> lParam = lParam;

		LastIndex ++;
		if (LastIndex >= XMSG_BUFFER_SIZE)
			LastIndex = 0;
		Size ++;
	} else
		ErrH.Abort("XMessageBuffer overflow...");
}

int XMessageBuffer::get(MSG* p) {
	if (Size) {
		memcpy((char*)p, (char*)table[FirstIndex], sizeof(MSG));

		FirstIndex ++;
		if (FirstIndex >= XMSG_BUFFER_SIZE)
			FirstIndex = 0;

		Size --;

		return 1;
	}
	return 0;
}

void XMessageBuffer::clear(void) {
	Size = 0;
	FirstIndex = LastIndex = 0;
}

} // namespace QDEngine
