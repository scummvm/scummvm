// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2004 The ScummVM-Residual Team (www.scummvm.org)
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
	Engine::instance()->registerTextObject(this);
}

void TextObject::setX(int x) {_x = x; }
void TextObject::setY(int y) {_y = y; }
void TextObject::setColor(Color *newcolor) { _fgColor = newcolor; }

void TextObject::draw() {
	const char *localString = Localizer::instance()->localize(_textID).c_str();
	// This is also used for things like debugging in addition
	// to dialogue so there aren't always translations
	if (strrchr(localString, '/') != NULL) {
		g_driver->drawEmergString(_x, _y, strrchr(localString, '/') + 1, _fgColor);
	} else {
		g_driver->drawEmergString(_x, _y, localString, _fgColor);
	}
}
