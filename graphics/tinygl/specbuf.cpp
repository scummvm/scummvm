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

static void calc_buf(GLSpecBuf *buf, const float shininess) {
	float val, inc;
	val = 0.0f;
	inc = 1.0f / SPECULAR_BUFFER_SIZE;
	for (int i = 0; i <= SPECULAR_BUFFER_SIZE; i++) {
		buf->buf[i] = pow(val, shininess);
		val += inc;
	}
}

GLSpecBuf *specbuf_get_buffer(GLContext *c, const int shininess_i, const float shininess) {
	GLSpecBuf *found, *oldest;
	found = oldest = c->specbuf_first;
	while (found && found->shininess_i != shininess_i) {
		if (found->last_used < oldest->last_used) {
			oldest = found;
		}
		found = found->next;
	}
	if (found) {
		found->last_used = c->specbuf_used_counter++;
		return found;
	}
	if (!oldest || c->specbuf_num_buffers < MAX_SPECULAR_BUFFERS) {
		// create new buffer
		GLSpecBuf *buf = (GLSpecBuf *)gl_malloc(sizeof(GLSpecBuf));
		if (!buf)
			error("could not allocate specular buffer");
		c->specbuf_num_buffers++;
		buf->next = c->specbuf_first;
		c->specbuf_first = buf;
		buf->last_used = c->specbuf_used_counter++;
		buf->shininess_i = shininess_i;
		calc_buf(buf, shininess);
		return buf;
	}
	//overwrite the lru buffer
	oldest->shininess_i = shininess_i;
	oldest->last_used = c->specbuf_used_counter++;
	calc_buf(oldest, shininess);
	return oldest;
}

void specbuf_cleanup(GLContext *c) {
	GLSpecBuf *buf, *next;
	buf = c->specbuf_first;
	for (int i = 0; i < c->specbuf_num_buffers; ++i) {
		next = buf->next;
		gl_free(buf);
		buf = next;
	}
}

} // end of namespace TinyGL
