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
		if (ABS(_floatIcon[i].framesLeft) < kEpsilon) {
			_floatIcon[i].framesLeft = 32;
			_floatIcon[i].x = xloc;
			_floatIcon[i].y = yloc;
			_floatIcon[i].ico = ico;
			return;
		}
	}
}

void GriffonEngine::addFloatText(const char *stri, float xloc, float yloc, int col) {
	for (int i = 0; i < kMaxFloat; i++) {
		if (ABS(_floatText[i].framesLeft) < kEpsilon) {
			_floatText[i].framesLeft = 32;
			_floatText[i].x = xloc;
			_floatText[i].y = yloc;
			_floatText[i].col = col;
			strcpy(_floatText[i].text, stri);
			return;
		}
	}
}

void GriffonEngine::eventText(const char *stri) {
	_videoBuffer2->fillRect(Common::Rect(0, 0, _videoBuffer2->w, _videoBuffer2->h), 0);
	_videoBuffer3->fillRect(Common::Rect(0, 0, _videoBuffer3->w, _videoBuffer3->h), 0);

	int x = 160 - 4 * strlen(stri);

	_ticks = g_system->getMillis();
	int pause_ticks = _ticks + 500;
	int b_ticks = _ticks;

	_videoBuffer->blit(*_videoBuffer3);
	_videoBuffer->blit(*_videoBuffer2);

	do {
		g_system->getEventManager()->pollEvent(_event);

		if ((_event.type == Common::EVENT_KEYDOWN || _event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START) && pause_ticks < _ticks)
			break;
		_videoBuffer2->blit(*_videoBuffer);

		int fr = 192;

		if (pause_ticks > _ticks)
			fr = 192 * (_ticks - b_ticks) / 500;
		if (fr > 192)
			fr = 192;

		_windowImg->setAlpha(fr, true);

		_windowImg->blit(*_videoBuffer);
		if (pause_ticks < _ticks)
			drawString(_videoBuffer, stri, x, 15, 0);

		g_system->copyRectToScreen(_videoBuffer->getPixels(), _videoBuffer->pitch, 0, 0, _videoBuffer->w, _videoBuffer->h);
		g_system->updateScreen();

		g_system->getEventManager()->pollEvent(_event);
		g_system->delayMillis(10);

		_ticksPassed = _ticks;
		_ticks = g_system->getMillis();

		_ticksPassed = _ticks - _ticksPassed;
		_fpsr = (float)_ticksPassed / 24.0;

		_fp++;
		if (_ticks > _nextTicks) {
			_nextTicks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}

		g_system->delayMillis(10);
	} while (1);

	_videoBuffer3->blit(*_videoBuffer);

	_itemTicks = _ticks + 210;
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

		_fontChr[stri[i] - 32][col]->blit(*buffer, rcDest.left, rcDest.top);
	}
}

void GriffonEngine::drawProgress(int w, int wm) {
	long ccc = _videoBuffer->format.RGBToColor(0, 255, 0);

	rcDest.setWidth(w * 74 / wm);
	_videoBuffer->fillRect(rcDest, ccc);

	g_system->copyRectToScreen(_videoBuffer->getPixels(), _videoBuffer->pitch, 0, 0, _videoBuffer->w, _videoBuffer->h);
	g_system->updateScreen();

	g_system->getEventManager()->pollEvent(_event);
}


} // end of namespace Griffon
