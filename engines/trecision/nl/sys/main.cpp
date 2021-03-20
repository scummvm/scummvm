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

#include "common/scummsys.h"
#include "trecision/nl/define.h"
#include "trecision/trecision.h"

#include "common/events.h"
#include "common/system.h"
#include "trecision/graphics.h"
#include "trecision/video.h"

namespace Trecision {

extern int16 mx, my;
extern bool mleft, mright;
extern uint8		*SoundStartBuffer;
extern uint8		*MemoryArea;
extern const char 		*_sysSentence[];
extern int			ForceQuit;

void NextMessage();
void longset(void *dest, uint32 value, uint32 len);
void StopSoundSystem();
void FastFileFinish();
void AnimFileFinish();
void SpeechFileFinish();
void VMouseON();
void VMouseOFF();
char waitKey();
void wordset(void *dest, uint16 value, uint32 len);

/*-----------------16/01/97 20.53-------------------
					GetKey
--------------------------------------------------*/
char GetKey() {
	Common::KeyCode key = g_vm->_curKey;
	uint16 ascii = g_vm->_curAscii;
	g_vm->_curKey = Common::KEYCODE_INVALID;
	g_vm->_curAscii = 0;

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
		}

		return 0;
	}
}

/*-----------------17/01/97 11.17-------------------
					waitKey
--------------------------------------------------*/
char waitKey() {
	while (g_vm->_curKey == Common::KEYCODE_INVALID)
		g_vm->CheckSystem();

	Common::KeyCode t = g_vm->_curKey;
	g_vm->_curKey = Common::KEYCODE_INVALID;

	return t;
}

/*-----------------17/01/97 11.18-------------------
					FreeKey
--------------------------------------------------*/
void FreeKey() {
	g_vm->_curKey = Common::KEYCODE_INVALID;
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
		g_vm->CheckSystem();
}

/* -----------------24/06/98 15.00-------------------
 * 					NlDissolve
 * --------------------------------------------------*/
void NlDissolve(int val) {
	uint16 CenterX = MAXX / 2, CenterY = MAXY / 2;
	int sv = ReadTime(), cv;

	int lastv = 9000;
	while ((sv + val) > (cv = ReadTime())) {
		g_vm->CheckSystem();
		if (lastv < (sv + val - cv))
			continue;

		lastv = (sv + val - cv);

		float a = (float)(((CenterX + 200) / val) * lastv);
		float b = (float)((CenterY / val) * lastv);

		float x = 0.0f;
		float y = b;

		if ((CenterY - (int)y) > TOP)
			wordset(g_vm->_video2 + (TOP)*MAXX, 0, ((CenterY - (int)y) - TOP)*MAXX);
		if ((AREA + TOP) > (CenterY + (int)y))
			wordset(g_vm->_video2 + (CenterY + (int)y) * MAXX, 0, (AREA + TOP - (CenterY + (int)y)) * MAXX);

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
				wordset(g_vm->_video2 + CenterX + (int)x + (CenterY + (int)y) * MAXX, 0, MAXX - (CenterX + (int)x));
			if ((CenterX + (int)x) < MAXX)
				wordset(g_vm->_video2 + CenterX + (int)x + (CenterY - (int)y) * MAXX, 0, MAXX - (CenterX + (int)x));
			if ((CenterX - (int)x) > 0)
				wordset(g_vm->_video2 + (CenterY + (int)y) * MAXX, 0, (CenterX - (int)x));
			if ((CenterX - (int)x) > 0)
				wordset(g_vm->_video2 + (CenterY - (int)y) * MAXX, 0, (CenterX - (int)x));
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
				wordset(g_vm->_video2 + CenterX + (int)x + (CenterY + (int)y) * MAXX, 0, MAXX - (CenterX + (int)x));
			if ((CenterX + (int)x) < MAXX)
				wordset(g_vm->_video2 + CenterX + (int)x + (CenterY - (int)y) * MAXX, 0, MAXX - (CenterX + (int)x));
			if ((CenterX - (int)x) > 0)
				wordset(g_vm->_video2 + (CenterY + (int)y) * MAXX, 0, (CenterX - (int)x));
			if ((CenterX - (int)x) > 0)
				wordset(g_vm->_video2 + (CenterY - (int)y) * MAXX, 0, (CenterX - (int)x));
		}


		g_vm->_graphicsMgr->showScreen(0, 0, MAXX, MAXY);
	}

	g_vm->_graphicsMgr->clearScreen();
}

/*-----------------10/12/95 15.29-------------------
					Mouse
--------------------------------------------------*/
void Mouse(MouseCmd opt) {
	g_vm->CheckSystem();

	switch (opt) {
	// Update mouse
	case MCMD_UPDT: {
		if (g_vm->_mouseONOFF && !g_vm->_flagMouseEnabled)
			Mouse(MCMD_OFF);

		if (!g_vm->_mouseONOFF && g_vm->_flagMouseEnabled)
			Mouse(MCMD_ON);

		if (g_vm->_mouseONOFF) {
			mleft = g_vm->_mouseLeftBtn;
			mright = g_vm->_mouseRightBtn;

			mx = CLIP<int16>(g_vm->_mouseX, 10, MAXX - 11);
			my = CLIP<int16>(g_vm->_mouseY, 10, MAXY - 11);

			VMouseON();
		}
		break;
	}
	case MCMD_OFF: {
		if (!g_vm->_mouseONOFF)
			break;
		g_vm->_mouseONOFF = false;
		VMouseOFF();
		break;
	}
	case MCMD_ON: {
		if (g_vm->_mouseONOFF)
			break;
		g_vm->_mouseONOFF = true;

		mleft = g_vm->_mouseLeftBtn;
		mright = g_vm->_mouseRightBtn;

		mx = CLIP<int16>(g_vm->_mouseX, 10, MAXX - 11);
		my = CLIP<int16>(g_vm->_mouseY, 10, MAXY - 11);

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
	g_vm->_animMgr->stopAllSmkAnims();
	StopSoundSystem();

	if (str)
		warning(str);

	g_system->quit();
}

} // End of namespace Trecision
