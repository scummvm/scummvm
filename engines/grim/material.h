/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 * $URL$
 * $Id$
 *
 */

#ifndef GRIM_MATERIAL_H
#define GRIM_MATERIAL_H

#include "engines/grim/resource.h"

namespace Grim {

class Material {
public:
	Material() {}
	// Load a texture from the given data.
	Material(const char *filename, const char *data, int len, const CMap *cmap);

	// Load this texture into the GL context
	void select() const;

	// Set which image in an animated texture to use
	void setNumber(int n) { _currImage = n; }

	int numImages() const { return _numImages; }
	int currentImage() const { return _currImage; }

	~Material();

	Common::String _fname;

	const CMap *_cmap;
	int _numImages, _currImage;
	int _width, _height;
	void *_textures;
};

} // end of namespace Grim

#endif
