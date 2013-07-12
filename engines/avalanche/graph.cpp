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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* Replacement class for the Graph unit from Pascal. */

#include "avalanche/avalanche.h"
#include "avalanche/graph.h"

#include "common/system.h"

#include "engines/util.h"

#include "graphics/palette.h"

namespace Avalanche {

const byte Graph::_egaPaletteIndex[16] = {0, 1, 2, 3, 4, 5, 20, 7, 56, 57, 58, 59, 60, 61, 62, 63};



void Graph::setParent(AvalancheEngine *vm) {
	_vm = vm;
}

void Graph::init() {
	initGraphics(_screenWidth, _screenHeight, true);

	for (int i = 0; i < 64; ++i) {
		_egaPalette[i][0] = (i >> 2 & 1) * 0xaa + (i >> 5 & 1) * 0x55;
		_egaPalette[i][1] = (i >> 1 & 1) * 0xaa + (i >> 4 & 1) * 0x55;
		_egaPalette[i][2] = (i      & 1) * 0xaa + (i >> 3 & 1) * 0x55;
	}

	for (byte i = 0; i < 16; i++)
		g_system->getPaletteManager()->setPalette(_egaPalette[_egaPaletteIndex[i]], i, 1);

	_surface.create(_screenWidth, _screenHeight, Graphics::PixelFormat::createFormatCLUT8());
}

Graph::~Graph() {
	_surface.free();
}

byte *Graph::getPixel(int16 x, int16 y) {
	return (byte *)_surface.getBasePtr(x, y);
}

void Graph::setPixel(byte *pixel, byte color) {
	*pixel = color;
}

void Graph::drawToScreen() {
	g_system->copyRectToScreen(_surface.pixels, _surface.pitch , 0, 0, _screenWidth, _screenHeight);
	g_system->updateScreen();
}

} // End of namespace Avalanche
