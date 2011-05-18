/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "common/endian.h"

#include "engines/grim/grim.h"
#include "engines/grim/material.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/resource.h"

namespace Grim {

Material::Material(const char *filename, const char *data, int len, CMap *cmap) :
		Object(), _fname(filename), _cmap(cmap) {
	if (len < 4 || memcmp(data, "MAT ", 4) != 0)
		error("invalid magic loading texture");

	_numImages = READ_LE_UINT32(data + 12);
	_currImage = 0;
	/* Discovered by diffing orange.mat with pink.mat and blue.mat .
	 * Actual meaning unknown, so I prefer to use it as an enum-ish
	 * at the moment, to detect unexpected values.
	 */
	uint32 offset = READ_LE_UINT32(data + 0x4c);
	if (offset == 0x8)
	    data += 16;
	else if (offset != 0)
	    error("Unknown offset: %d", offset);
	_width = READ_LE_UINT32(data + 60 + _numImages * 40);
	_height = READ_LE_UINT32(data + 64 + _numImages * 40);

	if (_width == 0 || _height == 0) {
		if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("skip load texture: bad texture size (%dx%d) for texture %s", _width, _height, filename);
		return;
	}

	data += 84 + _numImages * 40;

	g_driver->createMaterial(this, data, cmap);
}

void Material::select() const {
	if (_width == 0 || _height == 0)
		return;
	g_driver->selectMaterial(this);
}

Material::~Material() {
	if (g_resourceloader)
		g_resourceloader->uncacheMaterial(this);
	if (_width == 0 || _height == 0)
		return;
	g_driver->destroyMaterial(this);
}

} // end of namespace Grim
