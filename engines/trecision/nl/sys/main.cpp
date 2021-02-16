/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trecision/nl/lib/addtype.h"
#include "trecision/nl/sysdef.h"
#include "trecision/nl/define.h"
#include "trecision/trecision.h"

#include "common/config-manager.h"
#include "common/events.h"
#include "common/system.h"
#include "engines/engine.h"
#include "graphics/colormasks.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

namespace Trecision {

int CurKey, CurAscii;
short wmx = 0, wmy = 0, wmleft = 0, wmright = 0, omx = 0, omy = 0;
bool videoLocked = false;
Graphics::PixelFormat _screenFormat;
bool _linearMode, _gamePaused = false;

extern short mx, my, mleft, mright;
extern uint16		*_video, VideoPitch;
extern uint8		*SoundStartBuffer;
extern uint8		*MemoryArea;
extern const char 		*_sysSentence[];
extern int			ForceQuit;
extern int			RoomReady;
extern int			KeybInput;

void initMain();
void NextMessage();
void longset(void *dest, uint32 value, uint32 len);
void StopAllSmackAnims();
void StopSoundSystem();
void RefreshAllAnimations();
void FastFileFinish();
void AnimFileFinish();
void SpeechFileFinish();
void VMouseON();
void VMouseOFF();
void clearScreen();
void UnlockVideo();
char waitKey();
void ShowScreen(int px, int py, int dx, int dy);
void wordset(void *dest, uint16 value, uint32 len);

void GetColorMask(const Graphics::PixelFormat &format) {
	extern unsigned short _bitMask[3];

	_bitMask[0] = format.rMax() << format.rShift;
	_bitMask[1] = format.gMax() << format.gShift;
	_bitMask[2] = format.bMax() << format.bShift;
}

void EventLoop() {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_MOUSEMOVE:
			wmx = event.mouse.x;
			wmy = event.mouse.y;
			break;

		case Common::EVENT_LBUTTONDOWN:
			wmleft = 1;
			break;

		case Common::EVENT_LBUTTONUP:
			wmleft = 0;
			break;

		case Common::EVENT_RBUTTONDOWN:
			wmright = 1;
			break;

		case Common::EVENT_RBUTTONUP:
			wmright = 0;
			break;

		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_CAPSLOCK) {
				if (!g_vm->_fastWalkLocked)
					g_vm->_fastWalk ^= true;
				g_vm->_fastWalkLocked = true;
			}
			break;

		case Common::EVENT_KEYUP:
			CurKey = event.kbd.keycode;
			CurAscii = event.kbd.ascii;
			switch (event.kbd.keycode) {
			case Common::KEYCODE_p:
				if (!_gamePaused && !KeybInput) {
					CurKey = 0;
					_gamePaused = true;
					waitKey();
				}
				_gamePaused = false;
				break;

			case Common::KEYCODE_CAPSLOCK:
				g_vm->_fastWalkLocked = false;
				break;
			}
			break;

		default:
			break;
		}
	}
	//g_system->delayMillis(10);
	g_system->updateScreen();
}

void NlInit() {
	clearScreen();

	_screenFormat = g_system->getScreenFormat();
	GetColorMask(_screenFormat);

	_linearMode = true;
	initMain();

	while (!g_engine->shouldQuit()) {
		EventLoop();
		NextMessage();
	}
}

/* -----------------25/10/97 15.12-------------------
					CheckSystem
 --------------------------------------------------*/
void CheckSystem() {
	//for ( int a=0; a<5; a++ )
	{
		RefreshAllAnimations();
		EventLoop();
	}
}

/*-----------------16/01/97 16.21-------------------
					lockVideo
--------------------------------------------------*/
void lockVideo() {
	if (!videoLocked) {
		Graphics::Surface *surface = g_system->lockScreen();
		_video = (uint16 *)surface->getPixels();
		VideoPitch = surface->pitch;

		videoLocked = true;
	}
}

/*-----------------16/01/97 16.21-------------------
					UnlockVideo
--------------------------------------------------*/
void UnlockVideo() {
	if (videoLocked) {
		g_system->unlockScreen();
		videoLocked = false;
		VideoPitch = 0;
		_video = nullptr;
	}
}

/*-----------------16/01/97 20.53-------------------
					GetKey
--------------------------------------------------*/
char GetKey() {
	int key = CurKey;
	int ascii = CurAscii;
	CurKey = 0;
	CurAscii = 0;

	switch (key) {
	case Common::KEYCODE_SPACE:
	case Common::KEYCODE_ESCAPE:
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_CLEAR:
	case Common::KEYCODE_BACKSPACE:
		return key;
	case Common::KEYCODE_F1:
	case Common::KEYCODE_F2:
	case Common::KEYCODE_F3:
	case Common::KEYCODE_F4:
	case Common::KEYCODE_F5:
	case Common::KEYCODE_F6:
		return 0x3B + key - Common::KEYCODE_F1;
	default:
		if (ascii) {
			return ascii;
		} else {
			return 0;
		}
	}
}

/*-----------------17/01/97 11.17-------------------
					waitKey
--------------------------------------------------*/
char waitKey() {
	while (CurKey == 0)
		CheckSystem();

	int t = CurKey;
	CurKey = 0;

	return t;
}

/*-----------------17/01/97 11.18-------------------
					FreeKey
--------------------------------------------------*/
void FreeKey() {
	CurKey = 0;
}

/*-----------------10/12/95 15.52-------------------
					clearScreen
--------------------------------------------------*/
void clearScreen() {
	lockVideo();
	if (_video == nullptr)
		return ;
	if ((VideoPitch == 0) || (VideoPitch == SCREENLEN * 2))
		longset(_video, 0x00000000, 320 * 480);
	else
		longset(_video, 0x00000000, (VideoPitch / 4) * 480);
	UnlockVideo();
}

/*-----------------10/12/95 15.52-------------------
					ReadTime
--------------------------------------------------*/
uint32 ReadTime() {
	return ((g_system->getMillis() * 3) / 50);
}

/* -----------------26/01/98 10.26-------------------
 * 					NlDelay
 * --------------------------------------------------*/
void NlDelay(uint32 val) {
	uint32 sv = ReadTime();

	while ((sv + val) > ReadTime())
		CheckSystem();
}

/* -----------------24/06/98 15.00-------------------
 * 					NlDissolve
 * --------------------------------------------------*/
void NlDissolve(int val) {
	extern uint16 *Video2;
	uint16 CenterX = MAXX / 2, CenterY = MAXY / 2;
	int sv = ReadTime(), cv;

	int lastv = 9000;
	while ((sv + val) > (cv = ReadTime())) {
		CheckSystem();
		if (lastv < (sv + val - cv))
			continue;

		lastv = (sv + val - cv);

		float a = (float)(((CenterX + 200) / val) * lastv);
		float b = (float)((CenterY / val) * lastv);

		float x = 0.0f;
		float y = b;

		if ((CenterY - (int)y) > TOP)
			wordset(Video2 + (TOP)*MAXX, 0, ((CenterY - (int)y) - TOP)*MAXX);
		if ((AREA + TOP) > (CenterY + (int)y))
			wordset(Video2 + (CenterY + (int)y)*MAXX, 0, (AREA + TOP - (CenterY + (int)y))*MAXX);

		float d1 = b * b - a * a * b + a * a / 4.0f;
		while (a * a * (y - 0.5f) > b * b * (x + 1.0f)) {
			if (d1 < 0.0f)
				d1 += b * b * (2.0f * x + 3.0f);
			else {
				d1 += b * b * (2.0f * x + 3.0f) + a * a * (-2.0f * y + 2.0f);
				y -= 1.0f;
			}
			x += 1.0f;

			if ((CenterX + (int)x) < MAXX)
				wordset(Video2 + CenterX + (int)x + (CenterY + (int)y)*MAXX, 0, MAXX - (CenterX + (int)x));
			if ((CenterX + (int)x) < MAXX)
				wordset(Video2 + CenterX + (int)x + (CenterY - (int)y)*MAXX, 0, MAXX - (CenterX + (int)x));
			if ((CenterX - (int)x) > 0)
				wordset(Video2 + (CenterY + (int)y)*MAXX, 0, (CenterX - (int)x));
			if ((CenterX - (int)x) > 0)
				wordset(Video2 + (CenterY - (int)y)*MAXX, 0, (CenterX - (int)x));
		}

		float d2 = b * b * (x + 0.5f) * (x + 0.5f) + a * a * (y - 1.0f) * (y - 1.0f) - a * a * b * b;
		while (y > 0.0f) {
			if (d2 < 0.0f) {
				d2 += b * b * (2.0f * x + 2.0f) + a * a * (-2.0f * y + 3.0f);
				x += 1.0f;
			} else
				d2 += a * a * (-2.0f * y + 3.0f);
			y -= 1.0f;

			if ((CenterX + (int)x) < MAXX)
				wordset(Video2 + CenterX + (int)x + (CenterY + (int)y)*MAXX, 0, MAXX - (CenterX + (int)x));
			if ((CenterX + (int)x) < MAXX)
				wordset(Video2 + CenterX + (int)x + (CenterY - (int)y)*MAXX, 0, MAXX - (CenterX + (int)x));
			if ((CenterX - (int)x) > 0)
				wordset(Video2 + (CenterY + (int)y)*MAXX, 0, (CenterX - (int)x));
			if ((CenterX - (int)x) > 0)
				wordset(Video2 + (CenterY - (int)y)*MAXX, 0, (CenterX - (int)x));
		}


		ShowScreen(0, 0, MAXX, MAXY);
	}

	clearScreen();
}

/*-----------------10/12/95 15.25-------------------
				click_handler
--------------------------------------------------*/

/*-----------------10/12/95 15.29-------------------
					Mouse
--------------------------------------------------*/
void Mouse(uint8 opt) {
	extern short SemMouseEnabled;

	CheckSystem();

	switch (opt) {
	// Update mouse
	case 3: {
		if (g_vm->_mouseONOFF && !SemMouseEnabled)
			Mouse(2);

		if (!g_vm->_mouseONOFF && SemMouseEnabled)
			Mouse(1);

		if (g_vm->_mouseONOFF) {
			mx = wmx & 0xFFFF;
			my = wmy & 0xFFFF;
			mleft = wmleft & 0xFFFF;
			mright = wmright & 0xFFFF;

			mx = CLIP<int16>(mx, 10, MAXX - 11);
			my = CLIP<int16>(my, 10, MAXY - 11);

			VMouseON();
		}
		break;
	}
	// Turn off mouse
	case 2: {
		if (!g_vm->_mouseONOFF)
			break;
		g_vm->_mouseONOFF = false;
		VMouseOFF();
		break;
	}
	// Turn on mouse
	case 1: {
		if (g_vm->_mouseONOFF)
			break;
		g_vm->_mouseONOFF = true;
		mx = wmx & 0xFFFF;
		my = wmy & 0xFFFF;
		mleft = wmleft & 0xFFFF;
		mright = wmright & 0xFFFF;

		mx = CLIP<int16>(mx, 10, MAXX - 11);
		my = CLIP<int16>(my, 10, MAXY - 11);

		VMouseON();
		break;
	}
	}
}

/* -----------------17/02/95 10.18-------------------
						CloseSys
--------------------------------------------------*/
void CloseSys(const char *str) { // close all
	if (MemoryArea)
		free(MemoryArea);

	FastFileFinish();
	SpeechFileFinish();
	AnimFileFinish();
	StopAllSmackAnims();
	StopSoundSystem();

	if (str)
		warning(str);

	g_system->quit();
}

} // End of namespace Trecision
