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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 *              Originally written by Syn9 in FreeBASIC with SDL
 *              http://syn9.thehideoutgames.com/index_backup.php
 *
 *            Ported to plain C for GCW-Zero handheld by Dmitry Smagin
 *                http://github.com/dmitrysmagin/griffon_legend
 *
 *
 *                 Programming/Graphics: Daniel "Syn9" Kennedy
 *                     Music/Sound effects: David Turner
 *
 *                   Beta testing and gameplay design help:
 *                    Deleter, Cha0s, Aether Fox, and Kiz
 *
 */

#include "common/system.h"

#include "griffon/griffon.h"

namespace Griffon {

void GriffonEngine::addFloatIcon(int ico, float xloc, float yloc) {
	for (int i = 0; i < kMaxFloat; i++) {
		if (ABS(_floaticon[i][0]) < kEpsilon) {
			_floaticon[i][0] = 32;
			_floaticon[i][1] = xloc;
			_floaticon[i][2] = yloc;
			_floaticon[i][3] = ico;
			return;
		}
	}
}

void GriffonEngine::addFloatText(const char *stri, float xloc, float yloc, int col) {
	for (int i = 0; i < kMaxFloat; i++) {
		if (ABS(_floattext[i][0]) < kEpsilon) {
			_floattext[i][0] = 32;
			_floattext[i][1] = xloc;
			_floattext[i][2] = yloc;
			_floattext[i][3] = col;
			strcpy(_floatstri[i], stri);
			return;
		}
	}
}

void GriffonEngine::eventText(const char *stri) {
	_videobuffer2->fillRect(Common::Rect(0, 0, _videobuffer2->w, _videobuffer2->h), 0);
	_videobuffer3->fillRect(Common::Rect(0, 0, _videobuffer3->w, _videobuffer3->h), 0);

	int x = 160 - 4 * strlen(stri);

	_ticks = g_system->getMillis();
	int pause_ticks = _ticks + 500;
	int b_ticks = _ticks;

	_videobuffer->blit(*_videobuffer3);
	_videobuffer->blit(*_videobuffer2);

	do {
		g_system->getEventManager()->pollEvent(_event);

		if (_event.type == Common::EVENT_KEYDOWN && pause_ticks < _ticks)
			break;
		_videobuffer2->blit(*_videobuffer);

		int fr = 192;

		if (pause_ticks > _ticks)
			fr = 192 * (_ticks - b_ticks) / 500;
		if (fr > 192)
			fr = 192;

		_windowimg->setAlpha(fr, true);

		_windowimg->blit(*_videobuffer);
		if (pause_ticks < _ticks)
			drawString(_videobuffer, stri, x, 15, 0);

		g_system->copyRectToScreen(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);
		g_system->updateScreen();

		g_system->getEventManager()->pollEvent(_event);
		g_system->delayMillis(10);

		_tickspassed = _ticks;
		_ticks = g_system->getMillis();

		_tickspassed = _ticks - _tickspassed;
		_fpsr = (float)_tickspassed / 24.0;

		_fp++;
		if (_ticks > _nextticks) {
			_nextticks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}

		g_system->delayMillis(10);
	} while (1);

	_videobuffer3->blit(*_videobuffer);

	_itemticks = _ticks + 210;
}

void GriffonEngine::drawLine(Graphics::TransparentSurface *buffer, int x1, int y1, int x2, int y2, int col) {
	int xdif = x2 - x1;
	int ydif = y2 - y1;

	if (xdif == 0) {
		for (int y = y1; y <= y2; y++) {
			uint32 *temp = (uint32 *)buffer->getBasePtr(x1, y);
			*temp = col;
		}
	}

	if (ydif == 0) {
		for (int x = x1; x <= x2; x++) {
			uint32 *temp = (uint32 *)buffer->getBasePtr(x, y1);
			*temp = col;
		}
	}
}

void GriffonEngine::drawString(Graphics::TransparentSurface *buffer, const char *stri, int xloc, int yloc, int col) {
	int l = strlen(stri);

	for (int i = 0; i < l; i++) {
		rcDest.left = xloc + i * 8;
		rcDest.top = yloc;

		_fontchr[stri[i] - 32][col]->blit(*buffer, rcDest.left, rcDest.top);
	}
}

void GriffonEngine::drawProgress(int w, int wm) {
	long ccc = _videobuffer->format.RGBToColor(0, 255, 0);

	rcDest.setWidth(w * 74 / wm);
	_videobuffer->fillRect(rcDest, ccc);

	g_system->copyRectToScreen(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);
	g_system->updateScreen();

	g_system->getEventManager()->pollEvent(_event);
}


} // end of namespace Griffon
