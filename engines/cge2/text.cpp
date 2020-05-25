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
 */

/*
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janusz B. Wisniewski and L.K. Avalon
 */

#include "cge2/text.h"
#include "common/str.h"

namespace CGE2 {

Text::Text(CGE2Engine *vm, const char *fname) : _vm(vm) {
	_vm->mergeExt(_fileName, fname, kSayExt);
	if (!_vm->_resman->exist(_fileName))
		error("No talk (%s)", _fileName);
	_txtCount = count();
	if (_txtCount == -1)
		error("Unable to read dialog file %s", _fileName);

	_txtCount += 2;
	_cache = new Handler[_txtCount];
	for (_size = 0; _size < _txtCount; _size++) {
		_cache[_size]._ref = 0;
		_cache[_size]._text = nullptr;
	}
	load();

	_cache[_txtCount - 1]._ref = -1;
	_cache[_txtCount - 1]._text = new char[3];
	strcpy(_cache[_txtCount - 1]._text, "");
}

Text::~Text() {
	clear();
	delete[] _cache;
}

int16 Text::count() {
	EncryptedStream tf(_vm, _fileName);
	if (tf.err())
		return -1;

	Common::String line;
	char tmpStr[kLineMax + 1];

	int counter = 0;

	for (line = tf.readLine(); !tf.eos(); line = tf.readLine()) {
		char *s;
		assert(line.size() <= 513);
		Common::strlcpy(tmpStr, line.c_str(), sizeof(tmpStr));
		if ((s = strtok(tmpStr, " =,;/\t\n")) == nullptr)
			continue;
		if (!Common::isDigit(*s))
			continue;

		counter++;
	}
	return counter;
}

void Text::clear() {
	for (int i = 0; i < _txtCount; i++) {
		if (_cache[i]._ref) {
			_cache[i]._ref = 0;
			delete[] _cache[i]._text;
			_cache[i]._text = nullptr;
		}
	}
}

void Text::load() {
	EncryptedStream tf(_vm, _fileName);
	assert(!tf.err());

	Common::String line;
	char tmpStr[kLineMax + 1];
	int idx;

	for (idx = 0, line = tf.readLine(); !tf.eos(); line = tf.readLine()) {
		int n = line.size();
		char *s;
		Common::strlcpy(tmpStr, line.c_str(), sizeof(tmpStr));
		if ((s = strtok(tmpStr, " =,;/\t\n")) == nullptr)
			continue;
		if (!Common::isDigit(*s))
			continue;

		int r = _vm->number(s);

		s += strlen(s);
		if (s < tmpStr + n)
			++s;

		_cache[idx]._ref = r;
		_cache[idx]._text = new char[strlen(s) + 1];
		strcpy(_cache[idx]._text, s);
		idx++;
	}
}

char *Text::getText(int ref) {
	int i;
	for (i = 0; (i < _size) && (_cache[i]._ref != ref); i++)
		;

	if (i < _size)
		return _cache[i]._text;

	warning("getText: Unable to find ref %d:%d", ref / 256, ref % 256);
	return nullptr;
}

void Text::say(const char *text, Sprite *spr) {
	_vm->killText();

	_vm->_talk = new Talk(_vm, text, kTBRound, kCBSay);

	Speaker *speaker = new Speaker(_vm);

	bool east = spr->_flags._east;
	V2D d(_vm, 20, spr->_siz.y - 2);
	if (!east)
		d.x = -d.x;
	if (_vm->isHero(spr))
		d = d.scale(spr->_pos3D._z.trunc());
	V2D pos = spr->_pos2D + d;
	uint16 sw = (speaker->_siz.x >> 1);
	if (!east)
		sw = -sw;

	if (east) {
		if (pos.x + sw + kTextRoundCorner + kCaptionSide >= kScrWidth)
			east = false;
	} else if (pos.x <= kCaptionSide + kTextRoundCorner - sw)
		east = true;

	if (east != (d.x > 0)) {
		d.x = -d.x;
		sw = -sw;
	}
	pos.x = spr->_pos2D.x + d.x + sw;

	_vm->_talk->_flags._kill = true;
	_vm->_talk->setName(getText(kSayName));
	_vm->_talk->gotoxyz(pos.x, pos.y + speaker->_siz.y - 1, 0);

	speaker->gotoxyz(pos.x, _vm->_talk->_pos3D._y.trunc() - speaker->_siz.y + 1, 0);
	speaker->_flags._slav = true;
	speaker->_flags._kill = true;
	speaker->setName(getText(kSayName));
	speaker->step(east);

	_vm->_vga->_showQ->append(_vm->_talk);
	_vm->_vga->_showQ->append(speaker);
}

void CGE2Engine::inf(const char *text, ColorBank col) {
	killText();
	_talk = new Talk(this, text, kTBRect, col, true);
	_talk->_flags._kill = true;
	_talk->setName(_text->getText(kInfName));
	_talk->center();
	_vga->_showQ->append(_talk);
}

void Text::sayTime(Sprite *spr) {
	TimeDate curTime;
	_vm->_system->getTimeAndDate(curTime);

	char t[6];
	snprintf(t, 6, "%d:%02d", curTime.tm_hour, curTime.tm_min);
	say(t, spr);
}

} // End of namespace CGE2
