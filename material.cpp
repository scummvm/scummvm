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
#include "material.h"
#include "colormap.h"
#include "bits.h"
#include "debug.h"
#include "driver_gl.h"

Material::Material(const char *filename, const char *data, int len, const CMap &cmap) :
		Resource(filename) {
	if (len < 4 || memcmp(data, "MAT ", 4) != 0)
		error("invalid magic loading texture\n");

	num_images_ = READ_LE_UINT32(data + 12);
	curr_image_ = 0;
	width_ = READ_LE_UINT32(data + 76 + num_images_ * 40);
	height_ = READ_LE_UINT32(data + 80 + num_images_ * 40);

	if ((width_ == 0) || (height_ == 0)) {
		warning("bad texture size (%dx%d) for texture %s\n", width_, height_, filename);
	}

	data += 100 + num_images_ * 40;

	g_driver->createMaterial(this, data, &cmap);
}

void Material::select() const {
	g_driver->selectMaterial(this);
}

Material::~Material() {
	g_driver->destroyMaterial(this);
}
