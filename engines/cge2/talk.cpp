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
 * Copyright (c) 1994-1997 Janus B. Wisniewski and L.K. Avalon
 */

#include "cge2/general.h"
#include "cge2/talk.h"
#include "cge2/cge2_main.h"

namespace CGE2 {

Font::Font(CGE2Engine *vm) : _vm(vm) {
	_map = new uint8[kMapSize];
	_pos = new uint16[kPosSize];
	_widthArr = new uint8[kWidSize];

	assert((_map != NULL) && (_pos != NULL) && (_widthArr != NULL));
	load();
}

Font::~Font() {
	delete[] _map;
	delete[] _pos;
	delete[] _widthArr;
}

void Font::load() {
	char *path = "CGE.CFT";
	if (!_vm->_resman->exist(path))
		error("Missing configuration file! %s", path);

	EncryptedStream fontFile(_vm, path);
	assert(!fontFile.err());

	fontFile.read(_widthArr, kWidSize);
	assert(!fontFile.err());

	uint16 p = 0;
	for (uint16 i = 0; i < kPosSize; i++) {
		_pos[i] = p;
		p += _widthArr[i];
	}
	fontFile.read(_map, p);

	path = "CGE.TXC";
	if (!_vm->_resman->exist(path))
		error("Missing configuration file! %s", path);

	// Reading in _colorSet:
	EncryptedStream colorFile(_vm, path);
	assert(!colorFile.err());

	char tmpStr[kLineMax + 1];
	Common::String line;
	int n = 0;

	for (line = colorFile.readLine(); !colorFile.eos(); line = colorFile.readLine()){
		if (line.size() == 0)
			continue;
		Common::strlcpy(tmpStr, line.c_str(), sizeof(tmpStr));
		
		char *p;

		if ((p = _vm->token(tmpStr)) == NULL)
			error("Wrong line! (%d) in %s", colorFile.getLineCount(), path);
		_colorSet[n][0] = _vm->number(p);

		for (int i = 1; i < 4; i++) {
			if ((p = _vm->token(nullptr)) == NULL)
				error("Wrong line! (%d) in %s", colorFile.getLineCount(), path);
			_colorSet[n][i] = _vm->number(p);
		}

		n++;
	}
}

uint16 Font::width(const char *text) {
	uint16 w = 0;
	if (!text)
		return 0;
	while (*text)
		w += _widthArr[(unsigned char)*(text++)];
	return w;
}

Talk::Talk(CGE2Engine *vm, const char *text, TextBoxStyle mode, bool wideSpace)
	: Sprite(vm), _mode(mode), _wideSpace(wideSpace), _vm(vm) {
	warning("STUB: Talk::Talk()");
}

Talk::Talk(CGE2Engine *vm)
	: Sprite(vm), _mode(kTBPure), _vm(vm) {
	warning("STUB: Talk::Talk()");
}

void Talk::update(const char *text) {
	warning("STUB: Talk::update()");
}

Bitmap *Talk::box(uint16 w, uint16 h) {
	warning("STUB: Talk::box()");
	return *_ts;
}

InfoLine::InfoLine(CGE2Engine *vm, uint16 w) : Talk(vm), _oldText(NULL), _vm(vm) {
	warning("STUB: InfoLine::InfoLine()");
}

void InfoLine::update(const char *text) {
	warning("STUB: InfoLine::update()");
}

void InfoLine::update() {
	warning("STUB: InfoLine::update()");
}

void InfoLine::setText(const char *txt) {
	warning("STUB: InfoLine::setText()");
}

} // End of namespace CGE2
