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

namespace TinyGL {

int glRenderMode(int mode) {
	GLContext *c = gl_get_context();
	int result = 0;

	switch (c->render_mode) {
	case TGL_RENDER:
		break;
	case TGL_SELECT:
		if (c->select_overflow) {
			result = -c->select_hits;
		} else {
			result = c->select_hits;
		}
		c->select_overflow = 0;
		c->select_ptr = c->select_buffer;
		c->name_stack_size = 0;
		break;
	default:
		assert(0);
	}
	switch (mode) {
	case TGL_RENDER:
		c->render_mode = TGL_RENDER;
		break;
	case TGL_SELECT:
		c->render_mode = TGL_SELECT;
		assert(c->select_buffer != NULL);
		c->select_ptr = c->select_buffer;
		c->select_hits = 0;
		c->select_overflow = 0;
		c->select_hit = NULL;
		break;
	default:
		assert(0);
	}
	return result;
}

void glSelectBuffer(int size, unsigned int *buf) {
	GLContext *c = gl_get_context();

	assert(c->render_mode != TGL_SELECT);

	c->select_buffer = buf;
	c->select_size = size;
}

void glopInitNames(GLContext *c, GLParam *) {
	if (c->render_mode == TGL_SELECT) {
		c->name_stack_size = 0;
		c->select_hit = NULL;
	}
}

void glopPushName(GLContext *c, GLParam *p) {
	if (c->render_mode == TGL_SELECT) {
		assert(c->name_stack_size < MAX_NAME_STACK_DEPTH);
		c->name_stack[c->name_stack_size++] = p[1].i;
		c->select_hit = NULL;
	}
}

void glopPopName(GLContext *c, GLParam *) {
	if (c->render_mode == TGL_SELECT) {
		assert(c->name_stack_size > 0);
		c->name_stack_size--;
		c->select_hit = NULL;
	}
}

void glopLoadName(GLContext *c, GLParam *p) {
	if (c->render_mode == TGL_SELECT) {
		assert(c->name_stack_size > 0);
		c->name_stack[c->name_stack_size - 1] = p[1].i;
		c->select_hit = NULL;
	}
}

void gl_add_select(GLContext *c, unsigned int zmin, unsigned int zmax) {
	unsigned int *ptr;
	int n;

	if (!c->select_overflow) {
		if (!c->select_hit) {
			n = c->name_stack_size;
			if ((c->select_ptr - c->select_buffer + 3 + n) > c->select_size) {
				c->select_overflow = 1;
			} else {
				ptr = c->select_ptr;
				c->select_hit = ptr;
				*ptr++ = c->name_stack_size;
				*ptr++ = zmin;
				*ptr++ = zmax;
				for (int i = 0; i < n; i++)
					*ptr++ = c->name_stack[i];
				c->select_ptr = ptr;
				c->select_hits++;
			}
		} else {
			if (zmin < c->select_hit[1])
				c->select_hit[1] = zmin;
			if (zmax > c->select_hit[2])
				c->select_hit[2] = zmax;
		}
	}
}

} // end of namespace TinyGL
