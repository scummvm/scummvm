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

/*
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#include "cge/general.h"
#include "cge/text.h"
#include "cge/talk.h"
#include "cge/vol.h"
#include "cge/game.h"
#include "cge/snail.h"
#include "cge/cge_main.h"

namespace CGE {

Text *_text;
Talk *_talk = NULL;

Text::Text(CGEEngine *vm, const char *fname, int size) : _vm(vm) {
	_cache = new Han[size];
	mergeExt(_fileName, fname, SAY_EXT);
	if (!INI_FILE::exist(_fileName))
		error("No talk (%s)\n", _fileName);

	for (_size = 0; _size < size; _size++) {
		_cache[_size]._ref = 0;
		_cache[_size]._txt = NULL;
	}
}


Text::~Text() {
	clear();
	delete[] _cache;
}


void Text::clear(int from, int upto) {
	Han *p, * q;
	for (p = _cache, q = p + _size; p < q; p++) {
		if (p->_ref && p->_ref >= from && p->_ref < upto) {
			p->_ref = 0;
			delete[] p->_txt;
			p->_txt = NULL;
		}
	}
}


int Text::find(int ref) {
	Han *p, * q;
	int i = 0;
	for (p = _cache, q = p + _size; p < q; p++) {
		if (p->_ref == ref)
			break;
		else
			i++;
	}
	return i;
}


void Text::preload(int from, int upto) {
	INI_FILE tf = _fileName;
	if (!tf._error) {
		Han *CacheLim = _cache + _size;
		char line[LINE_MAX + 1];
		int n;

		while ((n = tf.read((uint8 *)line)) != 0) {
			char *s;
			int ref;

			if (line[n - 1] == '\n')
				line[-- n] = '\0';
			if ((s = strtok(line, " =,;/\t\n")) == NULL)
				continue;
			if (! IsDigit(*s))
				continue;
			ref = atoi(s);
			if (ref && ref >= from && ref < upto) {
				Han *p;

				p = &_cache[find(ref)];
				if (p < CacheLim) {
					delete[] p->_txt;
					p->_txt = NULL;
				} else
					p = &_cache[find(0)];
				if (p >= CacheLim)
					break;
				s += strlen(s);
				if (s < line + n)
					++s;
				if ((p->_txt = new char[strlen(s) + 1]) == NULL)
					break;
				p->_ref = ref;
				strcpy(p->_txt, s);
			}
		}
	}
}


char *Text::load(int idx, int ref) {
	INI_FILE tf = _fileName;
	if (!tf._error) {
		Han *p = &_cache[idx];
		char line[LINE_MAX + 1];
		int n;

		while ((n = tf.read((uint8 *)line)) != 0) {
			char *s;

			if (line[n - 1] == '\n')
				line[-- n] = '\0';
			if ((s = strtok(line, " =,;/\t\n")) == NULL)
				continue;
			if (! IsDigit(*s))
				continue;

			int r = atoi(s);
			if (r < ref)
				continue;
			if (r > ref)
				break;
			// (r == ref)
			s += strlen(s);
			if (s < line + n)
				++s;
			p->_ref = ref;
			if ((p->_txt = new char[strlen(s) + 1]) == NULL)
				return NULL;
			return strcpy(p->_txt, s);
		}
	}
	return NULL;
}


char *Text::getText(int ref) {
	int i;
	if ((i = find(ref)) < _size)
		return _cache[i]._txt;

	if ((i = find(0)) >= _size) {
		clear(SYSTXT_MAX);            // clear non-system
		if ((i = find(0)) >= _size) {
			clear();              // clear all
			i = 0;
		}
	}
	return load(i, ref);
}


void Text::say(const char *txt, Sprite *spr) {
	killText();
	_talk = new Talk(_vm, txt, ROUND);
	if (_talk) {
		bool east = spr->_flags._east;
		int x = (east) ? (spr->_x + spr->_w - 2) : (spr->_x + 2);
		int y = spr->_y + 2;
		Sprite *spike = new Spike(_vm);
		uint16 sw = spike->_w;

		if (east) {
			if (x + sw + TEXT_RD + 5 >= SCR_WID)
				east = false;
		} else {
			if (x <= 5 + TEXT_RD + sw)
				east = true;
		}
		x = (east) ? (spr->_x + spr->_w - 2) : (spr->_x + 2 - sw);
		if (spr->_ref == 1)
			x += ((east) ? -10 : 10); // Hero

		_talk->_flags._kill = true;
		_talk->_flags._bDel = true;
		_talk->setName(_text->getText(SAY_NAME));
		_talk->gotoxy(x - (_talk->_w - sw) / 2 - 3 + 6 * east, y - spike->_h - _talk->_h + 1);
		_talk->_z = 125;
		_talk->_ref = SAY_REF;

		spike->gotoxy(x, _talk->_y + _talk->_h - 1);
		spike->_z = 126;
		spike->_flags._slav = true;
		spike->_flags._kill = true;
		spike->setName(_text->getText(SAY_NAME));
		spike->step(east);
		spike->_ref = SAY_REF;

		_vga->_showQ->insert(_talk, _vga->_showQ->last());
		_vga->_showQ->insert(spike, _vga->_showQ->last());
	}
}

void CGEEngine::inf(const char *txt) {
	debugC(1, kDebugEngine, "CGEEngine::inf(%s)", txt);

	killText();
	_talk = new Talk(this, txt, RECT);
	if (_talk) {
		_talk->_flags._kill = true;
		_talk->_flags._bDel = true;
		_talk->setName(_text->getText(INF_NAME));
		_talk->center();
		_talk->gotoxy(_talk->_x, _talk->_y - 20);
		_talk->_z = 126;
		_talk->_ref = INF_REF;
		_vga->_showQ->insert(_talk, _vga->_showQ->last());
	}
}


void sayTime(Sprite *spr) {
	/*
	  static char t[] = "00:00";
	  struct time ti;
	  gettime(&ti);
	  wtom(ti.ti_hour, t+0, 10, 2);
	  wtom(ti.ti_min,  t+3, 10, 2);
	  Say((*t == '0') ? (t+1) : t, spr);
	  */
	warning("STUB: sayTime");
}

void killText() {
	if (_talk) {
		SNPOST_(SNKILL, -1, 0, _talk);
		_talk = NULL;
	}
}

} // End of namespace CGE
