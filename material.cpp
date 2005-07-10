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
#include "bits.h"
#include "debug.h"
#include "material.h"
#include "colormap.h"
#include "driver.h"

Material::Material(const char *filename, const char *data, int len, const CMap &cmap) :
		Resource(filename), _cmap((CMap *) &cmap) {
	if (len < 4 || memcmp(data, "MAT ", 4) != 0)
		error("invalid magic loading texture\n");

	_numImages = READ_LE_UINT32(data + 12);
	_currImage = 0;
	_width = READ_LE_UINT32(data + 76 + _numImages * 40);
	_height = READ_LE_UINT32(data + 80 + _numImages * 40);

	if ((_width == 0) || (_height == 0)) {
		if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("skip load texture: bad texture size (%dx%d) for texture %s\n", _width, _height, filename);
		return;
	}

	data += 100 + _numImages * 40;

	g_driver->createMaterial(this, data, &cmap);
}

void Material::select() const {
	if ((_width == 0) || (_height == 0))
	    return;
	g_driver->selectMaterial(this);
}

Material::~Material() {
	if ((_width == 0) || (_height == 0))
	    return;
	g_driver->destroyMaterial(this);
}
