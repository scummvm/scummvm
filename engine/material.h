/* Residual - Virtual machine to run LucasArts' 3D adventure games
 * Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
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

#ifndef MATERIAL_H
#define MATERIAL_H

#include "resource.h"

#include <cstring>

class CMap;

class Material : public Resource {
public:
	// Load a texture from the given data.
	Material(const char *filename, const char *data, int len, const CMap &cmap);

	// Load this texture into the GL context
	void select() const;

	// Set which image in an animated texture to use
	void setNumber(int n) { _currImage = n; }

	int numImages() const { return _numImages; }
	int currentImage() const { return _currImage; }

	~Material();

//private:
	CMap *_cmap;
	int _numImages, _currImage;
	int _width, _height;
	void *_textures;
};

#endif
