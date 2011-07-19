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

#include "cge/config.h"
#include "cge/sound.h"
#include "cge/vmenu.h"
#include "cge/text.h"
#include "cge/cge_main.h"

namespace CGE {

#define MENU_TEXT   56

static Choice *_cho;
static int     _hlp;

void CGEEngine::snSelect() {
	debugC(1, kDebugEngine, "CGEEngine::snSelect()");

	inf(_text->getText(_hlp));
	_talk->gotoxy(_talk->_x, kFontHigh / 2);
	(new Vmenu(this, _cho, SCR_WID / 2, _talk->_y + _talk->_h + kTextVMargin + kFontHigh))->setName(_text->getText(MENU_TEXT));
}

} // End of namespace CGE
