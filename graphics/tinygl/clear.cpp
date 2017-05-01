/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 * This file is based on, or a modified version of code from TinyGL (C) 1997-1998 Fabrice Bellard,
 * which is licensed under the zlib-license (see LICENSE).
 * It also has modifications by the ResidualVM-team, which are covered under the GPLv2 (or later).
 */

#include "graphics/tinygl/zgl.h"
#include "graphics/tinygl/zdirtyrect.h"

namespace TinyGL {

void glopClearColor(GLContext *c, GLParam *p) {
	c->clear_color = Vector4(p[1].f, p[2].f, p[3].f, p[4].f);
}

void glopClearDepth(GLContext *c, GLParam *p) {
	c->clear_depth = p[1].f;
}

void glopClear(GLContext *c, GLParam *p) {
	int mask = p[1].i;
	int z = (int)(c->clear_depth * ((1 << ZB_Z_BITS) - 1));
	int r = (int)(c->clear_color.X * 255);
	int g = (int)(c->clear_color.Y * 255);
	int b = (int)(c->clear_color.Z * 255);

	tglIssueDrawCall(new Graphics::ClearBufferDrawCall(mask & TGL_DEPTH_BUFFER_BIT, z, mask & TGL_COLOR_BUFFER_BIT, r, g, b));
}

} // end of namespace TinyGL
