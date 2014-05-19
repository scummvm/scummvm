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
//#include "cge2/game.h"
//#include "cge2/events.h"
#include "cge2/cge2_main.h"

namespace CGE2 {

Font::Font(CGE2Engine *vm, const char *name) : _vm(vm) {
	warning("STUB: Font::Font()");
}

Font::~Font() {
	warning("STUB: Font::~Font()");
}

void Font::load() {
	warning("STUB: Font::load()");
}

uint16 Font::width(const char *text) {
	warning("STUB: Font::width()");
	return 0;
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
