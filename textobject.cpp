// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2005 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include "stdafx.h"
#include "textobject.h"
#include "engine.h"
#include "localize.h"
#include "driver_gl.h"

TextObject::TextObject(const char *text, const int x, const int y, const Color& fgColor) :
		_fgColor(fgColor), _x(x), _y(y) {
	strcpy(_textID, text);
	g_engine->registerTextObject(this);
}

void TextObject::setX(int x) {_x = x; }
void TextObject::setY(int y) {_y = y; }
void TextObject::setColor(Color *newcolor) { _fgColor = newcolor; }

std::string parseMsgText(char *msg, char *msgId);

void TextObject::draw() {
	char msgId[32];

	std::string msg = parseMsgText(_textID, msgId);
	g_driver->drawEmergString(_x, _y, msg.c_str(), _fgColor);
}
