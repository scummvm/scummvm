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

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */

int xtDispatchMessage(MSG* msg);
void xtProcessMessageBuffer(void);
void xtSysQuant(void);

/* --------------------------- DEFINITION SECTION --------------------------- */

//#define _XRECORDER_LOG_

#define XREC_MAX_CMESSAGE   7
#define XREC_MAX_IMESSAGE   7

// Captured events...
static int XREC_CMESSAGES[XREC_MAX_CMESSAGE] = {
	WM_KEYDOWN,
	WM_LBUTTONDOWN,
	WM_RBUTTONDOWN,
	WM_MBUTTONDOWN,
	WM_LBUTTONDBLCLK,
	WM_RBUTTONDBLCLK,
	WM_MBUTTONDBLCLK
};

// Ignored events...
static int XREC_IMESSAGES[XREC_MAX_IMESSAGE] = {
	WM_MOUSEMOVE,
	WM_KEYUP,
	WM_SYSKEYUP,
	WM_SYSKEYDOWN,
	WM_LBUTTONUP,
	WM_RBUTTONUP,
	WM_MBUTTONUP
};

XRecorder XRec;

#ifdef  _XRECORDER_LOG_
XStream XRecLog;
#endif

XRecorder::XRecorder(void) {
	flags = 0;
	frameCount = controlCount = 0;

	hFile = NULL;
	nextMsg = NULL;
	hWnd = NULL;
}

void XRecorder::Open(char* fname, int mode) {
	int fmode;
	if (flags & (XRC_RECORD_MODE | XRC_PLAY_MODE))
		Close();

	if (!nextMsg) {
		nextMsg = new XRecorderMessage;
		nextMsg -> data = new int[XRC_BUFFER_SIZE];
		nextMsgDataSize = XRC_BUFFER_SIZE;
	}
	if (!hFile)
		hFile = new XStream;

	fmode = (mode == XRC_RECORD_MODE) ? XS_OUT : XS_IN;
	hFile -> open(fname, fmode);
#ifdef  _XRECORDER_LOG_
	if (mode == XRC_RECORD_MODE)
		XRecLog.open("xrec_wr.log", XS_OUT);
	else
		XRecLog.open("xrec_pl.log", XS_OUT);
#endif
	frameCount = 0;
	flags |= mode;
}

void XRecorder::Close(void) {
	if (flags & (XRC_RECORD_MODE | XRC_PLAY_MODE)) {
		hFile -> close();
		flags &= ~(XRC_RECORD_MODE | XRC_PLAY_MODE | XRC_MESSAGE_READ);
#ifdef  _XRECORDER_LOG_
		XRecLog.close();
#endif
	}
}

void XRecorder::GetMessage(void) {
	if (flags & XRC_PLAY_MODE) {
		if (hFile -> eof()) {
			Close();
			return;
		}
		*hFile > nextMsg -> Type > nextMsg -> Frame > nextMsg -> DataSize;
		if (nextMsg -> DataSize) {
			if (nextMsg -> DataSize > nextMsgDataSize) {
				delete nextMsg -> data;
				nextMsg -> data = new int[nextMsg -> DataSize];
				nextMsgDataSize = nextMsg -> DataSize;
			}
			hFile -> read((char*)nextMsg -> data, nextMsg -> DataSize * sizeof(int));
		}
		flags |= XRC_MESSAGE_READ;
	}
}

void XRecorder::DispatchMessage(void) {
	MSG msg;
	if ((flags & XRC_MESSAGE_READ) && frameCount == nextMsg -> Frame) {
		switch (nextMsg -> Type) {
		case XRC_USER_MESSAGE:
			break;
		case XRC_XMOUSE_MESSAGE:
		case XRC_SYSTEM_MESSAGE:
			msg.message = nextMsg -> data[0];
			msg.wParam = nextMsg -> data[1];
			msg.lParam = nextMsg -> data[2];
			xtDispatchMessage(&msg);
			break;
		}
#ifdef  _XRECORDER_LOG_
		XRecLog < "\r\nEvent -> " <= nextMsg -> Type < "(" <= nextMsg -> data[0] < "), frame = " <= frameCount;
#endif
	}
}

void XRecorder::Flush(void) {
#ifdef  _XRECORDER_LOG_
	XRecLog < "\r\nEvent -> " <= nextMsg -> Type < "(" <= nextMsg -> data[0] < "), frame = " <= frameCount;
#endif
	*hFile < nextMsg -> Type < nextMsg -> Frame < nextMsg -> DataSize;
	if (nextMsg -> DataSize) {
		hFile -> write((char*)nextMsg -> data, nextMsg -> DataSize * sizeof(int));
	}
}

void XRecorder::Quant(void) {
	frameCount ++;

	if (flags & XRC_PLAY_MODE) {
		if (!(flags & XRC_MESSAGE_READ))
			GetMessage();

		while (flags & XRC_MESSAGE_READ && frameCount == nextMsg -> Frame) {
			DispatchMessage();
			GetMessage();
		}
	}

	xtSysQuant();

	xtClearMessageQueue();
	xtProcessMessageBuffer();
}

void XRecorder::PutSysMessage(int id, int msg, int wp, int lp) {
	nextMsg -> Type = id;
	nextMsg -> Frame = frameCount;
	nextMsg -> DataSize = 3;

	nextMsg -> data[0] = msg;
	nextMsg -> data[1] = wp;
	nextMsg -> data[2] = lp;

	Flush();
}

void XRecorder::PutMessage(int msg, int sz, void* p) {
	int i;
	int* ptr = (int*)p;

	nextMsg -> Type = msg;
	nextMsg -> Frame = frameCount;
	nextMsg -> DataSize = sz;

	if (nextMsgDataSize < nextMsg -> DataSize) {
		delete nextMsg -> data;
		nextMsg -> data = new int[nextMsg -> DataSize];
		nextMsgDataSize = nextMsg -> DataSize;
	}

	for (i = 0; i < sz; i ++)
		nextMsg -> data[i] = ptr[i];

	Flush();
}

int XRecorder::CheckMessage(int code) {
	int i;
	for (i = 0; i < XREC_MAX_CMESSAGE; i ++) {
		if (XREC_CMESSAGES[i] == code) {
			if (flags & XRC_PLAY_MODE)
				Close();
			return 0;
		}
	}
	for (i = 0; i < XREC_MAX_IMESSAGE; i ++) {
		if (XREC_IMESSAGES[i] == code) return 0;
	}
	return 1;
}
} // namespace QDEngine
