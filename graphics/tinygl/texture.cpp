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

// Texture Manager

#include "common/endian.h"

#include "graphics/tinygl/zgl.h"

namespace TinyGL {

static GLTexture *find_texture(GLContext *c, unsigned int h) {
	GLTexture *t;

	t = c->shared_state.texture_hash_table[h % TEXTURE_HASH_TABLE_SIZE];
	while (t) {
		if (t->handle == h)
			return t;
		t = t->next;
	}
	return NULL;
}

void free_texture(GLContext *c, int h) {
	free_texture(c, find_texture(c, h));
}

void free_texture(GLContext *c, GLTexture *t) {
	GLTexture **ht;
	GLImage *im;

	if (!t->prev) {
		ht = &c->shared_state.texture_hash_table[t->handle % TEXTURE_HASH_TABLE_SIZE];
		*ht = t->next;
	} else {
		t->prev->next = t->next;
	}
	if (t->next)
		t->next->prev = t->prev;

	for (int i = 0; i < MAX_TEXTURE_LEVELS; i++) {
		im = &t->images[i];
		if (im->pixmap)
			im->pixmap.free();
	}

	gl_free(t);
}

GLTexture *alloc_texture(GLContext *c, int h) {
	GLTexture *t, **ht;

	t = (GLTexture *)gl_zalloc(sizeof(GLTexture));

	ht = &c->shared_state.texture_hash_table[h % TEXTURE_HASH_TABLE_SIZE];

	t->next = *ht;
	t->prev = NULL;
	if (t->next)
		t->next->prev = t;
	*ht = t;

	t->handle = h;
	t->disposed = false;
	t->versionNumber = 0;

	return t;
}

void glInitTextures(GLContext *c) {
	// textures
	c->texture_2d_enabled = 0;
	c->current_texture = find_texture(c, 0);
}

void glopBindTexture(GLContext *c, GLParam *p) {
	int target = p[1].i;
	int texture = p[2].i;
	GLTexture *t;

	assert(target == TGL_TEXTURE_2D && texture >= 0);

	t = find_texture(c, texture);
	if (!t) {
		t = alloc_texture(c, texture);
	}
	c->current_texture = t;
}

void glopTexImage2D(GLContext *c, GLParam *p) {
	int target = p[1].i;
	int level = p[2].i;
// "components" is guessed from "format".
//	int components = p[3].i;
	int width = p[4].i;
	int height = p[5].i;
	int border = p[6].i;
	int format = p[7].i;
	int type = p[8].i;
	byte *pixels = (byte *)p[9].p;
	GLImage *im;

	if (target != TGL_TEXTURE_2D)
		error("tglTexImage2D: target not handled");
	if (level < 0 || level >= MAX_TEXTURE_LEVELS)
		error("tglTexImage2D: invalid level");
	if (border != 0)
		error("tglTexImage2D: invalid border");
	Graphics::PixelFormat sourceFormat;
	switch (format) {
		case TGL_RGBA:
			sourceFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
			break;
		case TGL_RGB:
			sourceFormat = Graphics::PixelFormat(3, 8, 8, 8, 0, 0, 8, 16, 0);
			break;
		case TGL_BGRA:
			sourceFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24);
			break;
		case TGL_BGR:
			sourceFormat = Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0);
			break;
		default:
			error("tglTexImage2D: Pixel format not handled.");
	}

	Graphics::PixelFormat pf;
	switch (format) {
		case TGL_RGBA:
		case TGL_RGB:
#if defined(SCUMM_BIG_ENDIAN)
			pf = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
#elif defined(SCUMM_LITTLE_ENDIAN)
			pf = Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
#endif
			break;
		case TGL_BGRA:
		case TGL_BGR:
#if defined(SCUMM_BIG_ENDIAN)
			pf = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 0, 8, 16);
#elif defined(SCUMM_LITTLE_ENDIAN)
			pf = Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24);
#endif
			break;
		default:
			break;
	}
	Graphics::PixelBuffer internal(
		pf,
		c->_textureSize * c->_textureSize,
		DisposeAfterUse::NO
	);
	if (pixels != NULL) {
		Graphics::PixelBuffer src(sourceFormat, pixels);
		if (width != c->_textureSize || height != c->_textureSize) {
			Graphics::PixelBuffer src_conv(pf, width * height, DisposeAfterUse::YES);
			src_conv.copyBuffer(
				0,
				width * height,
				src
			);
			// we use interpolation for better looking result
			gl_resizeImage(
				internal.getRawBuffer(), c->_textureSize, c->_textureSize,
				src_conv.getRawBuffer(), width, height
			);
		} else {
			internal.copyBuffer(
				0,
				c->_textureSize * c->_textureSize,
				src
			);
		}
	}

	c->current_texture->versionNumber++;
	im = &c->current_texture->images[level];
	im->xsize = c->_textureSize;
	im->ysize = c->_textureSize;
	if (im->pixmap)
		im->pixmap.free();
	im->pixmap = internal;
}

// TODO: not all tests are done
void glopTexEnv(GLContext *, GLParam *p) {
	int target = p[1].i;
	int pname = p[2].i;
	int param = p[3].i;

	if (target != TGL_TEXTURE_ENV) {
error:
		error("tglTexParameter: unsupported option");
	}

	if (pname != TGL_TEXTURE_ENV_MODE)
		goto error;

	if (param != TGL_DECAL)
		goto error;
}

// TODO: not all tests are done
void glopTexParameter(GLContext *, GLParam *p) {
	int target = p[1].i;
	int pname = p[2].i;
	int param = p[3].i;

	if (target != TGL_TEXTURE_2D) {
error:
		error("tglTexParameter: unsupported option");
	}

	switch (pname) {
	case TGL_TEXTURE_WRAP_S:
	case TGL_TEXTURE_WRAP_T:
		if (param != TGL_REPEAT)
			goto error;
		break;
	default:
		;
	}
}

void glopPixelStore(GLContext *, GLParam *p) {
	int pname = p[1].i;
	int param = p[2].i;

	if (pname != TGL_UNPACK_ALIGNMENT || param != 1) {
		error("tglPixelStore: unsupported option");
	}
}

} // end of namespace TinyGL

void tglGenTextures(int n, unsigned int *textures) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	unsigned int max;
	TinyGL::GLTexture *t;

	max = 0;
	for (int i = 0; i < TEXTURE_HASH_TABLE_SIZE; i++) {
		t = c->shared_state.texture_hash_table[i];
		while (t) {
			if (t->handle > max)
				max = t->handle;
			t = t->next;
		}
	}
	for (int i = 0; i < n; i++) {
		textures[i] = max + i + 1;
	}
}

void tglDeleteTextures(int n, const unsigned int *textures) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLTexture *t;

	for (int i = 0; i < n; i++) {
		t = TinyGL::find_texture(c, textures[i]);
		if (t) {
			if (t == c->current_texture) {
				tglBindTexture(TGL_TEXTURE_2D, 0);
			}
			t->disposed = true;
		}
	}
}
