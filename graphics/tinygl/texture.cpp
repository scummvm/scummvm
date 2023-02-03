/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This file is based on, or a modified version of code from TinyGL (C) 1997-2022 Fabrice Bellard,
 * which is licensed under the MIT license (see LICENSE).
 * It also has modifications by the ResidualVM-team, which are covered under the GPLv2 (or later).
 */

// Texture Manager

#include "common/endian.h"

#include "graphics/tinygl/zgl.h"

namespace TinyGL {

GLTexture *GLContext::find_texture(uint h) {
	GLTexture *t;

	t = shared_state.texture_hash_table[h % TEXTURE_HASH_TABLE_SIZE];
	while (t) {
		if (t->handle == h)
			return t;
		t = t->next;
	}
	return nullptr;
}

void GLContext::free_texture(GLTexture *t) {
	GLTexture **ht;
	GLImage *im;

	assert(t);

	if (!t->prev) {
		ht = &shared_state.texture_hash_table[t->handle % TEXTURE_HASH_TABLE_SIZE];
		*ht = t->next;
	} else {
		t->prev->next = t->next;
	}
	if (t->next)
		t->next->prev = t->prev;

	for (int i = 0; i < MAX_TEXTURE_LEVELS; i++) {
		im = &t->images[i];
		if (im->pixmap) {
			delete im->pixmap;
			im->pixmap = nullptr;
		}
	}

	gl_free(t);
}

GLTexture *GLContext::alloc_texture(uint h) {
	GLTexture *t, **ht;

	t = (GLTexture *)gl_zalloc(sizeof(GLTexture));

	ht = &shared_state.texture_hash_table[h % TEXTURE_HASH_TABLE_SIZE];

	t->next = *ht;
	t->prev = nullptr;
	if (t->next)
		t->next->prev = t;
	*ht = t;

	t->handle = h;
	t->disposed = false;
	t->versionNumber = 0;

	return t;
}

void GLContext::glopBindTexture(GLParam *p) {
	int target = p[1].i;
	uint texture = p[2].ui;
	GLTexture *t;

	assert(target == TGL_TEXTURE_2D);

	t = find_texture(texture);
	if (!t) {
		t = alloc_texture(texture);
	}
	current_texture = t;
}

void GLContext::glopTexImage2D(GLParam *p) {
	int target = p[1].i;
	int level = p[2].i;
	int internalformat = p[3].i;
	int width = p[4].i;
	int height = p[5].i;
	int border = p[6].i;
	uint format = (uint)p[7].i;
	uint type = (uint)p[8].i;
	byte *pixels = (byte *)p[9].p;
	GLImage *im;

	if (target != TGL_TEXTURE_2D)
		error("tglTexImage2D: target not handled");
	if (level < 0 || level >= MAX_TEXTURE_LEVELS)
		error("tglTexImage2D: invalid level");
	if (internalformat != TGL_RGBA && internalformat != TGL_RGB)
		error("tglTexImage2D: invalid internalformat");
	if (border != 0)
		error("tglTexImage2D: invalid border");

	assert (current_texture);

	current_texture->versionNumber++;
	im = &current_texture->images[level];
	im->xsize = _textureSize;
	im->ysize = _textureSize;
	if (im->pixmap) {
		delete im->pixmap;
		im->pixmap = nullptr;
	}
	if (pixels) {
		uint filter;
		Graphics::PixelFormat pf;
		bool found = false;
		Common::Array<struct tglColorAssociation>::const_iterator it = colorAssociationList.begin();
		for (; it != colorAssociationList.end(); it++) {
			if (it->format == format &&
			    it->type == type) {
				pf = it->pf;
				found = true;
				break;
			}
		}
		if (!found)
			error("TinyGL texture: format 0x%04x and type 0x%04x combination not supported", format, type);
		Graphics::PixelBuffer src(pf, pixels);
		Graphics::PixelFormat internalPf;
#if defined(SCUMM_LITTLE_ENDIAN)
		if (internalformat == TGL_RGBA)
			internalPf = Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
		else if (internalformat == TGL_RGB)
			internalPf = Graphics::PixelFormat(3, 8, 8, 8, 0, 0, 8, 16, 0);
#else
		if (internalformat == TGL_RGBA)
			internalPf = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
		else if (internalformat == TGL_RGB)
			internalPf = Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0);
#endif
		Graphics::PixelBuffer srcInternal(internalPf, width * height, DisposeAfterUse::YES);
		srcInternal.copyBuffer(0, width * height, src);
		if (width > _textureSize || height > _textureSize)
			filter = texture_mag_filter;
		else
			filter = texture_min_filter;
		switch (filter) {
		case TGL_LINEAR_MIPMAP_NEAREST:
		case TGL_LINEAR_MIPMAP_LINEAR:
		case TGL_LINEAR:
			im->pixmap = new BilinearTexelBuffer(
				srcInternal,
				width, height,
				_textureSize
			);
			break;
		default:
			im->pixmap = new NearestTexelBuffer(
				srcInternal,
				width, height,
				_textureSize
			);
			break;
		}
	}
}

// TODO: not all tests are done
void GLContext::glopTexEnv(GLParam *p) {
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
void GLContext::glopTexParameter(GLParam *p) {
	int target = p[1].i;
	int pname = p[2].i;
	int param = p[3].i;

	if (target != TGL_TEXTURE_2D) {
error:
		error("tglTexParameter: unsupported option");
	}

	switch (pname) {
	case TGL_TEXTURE_WRAP_S:
		texture_wrap_s = param;
		break;
	case TGL_TEXTURE_WRAP_T:
		texture_wrap_t = param;
		break;
	case TGL_TEXTURE_MAG_FILTER:
		switch (param) {
		case TGL_NEAREST:
		case TGL_LINEAR:
			texture_mag_filter = param;
			break;
		default:
			goto error;
		}
		break;
	case TGL_TEXTURE_MIN_FILTER:
		switch (param) {
		case TGL_LINEAR_MIPMAP_NEAREST:
		case TGL_LINEAR_MIPMAP_LINEAR:
		case TGL_NEAREST_MIPMAP_NEAREST:
		case TGL_NEAREST_MIPMAP_LINEAR:
		case TGL_NEAREST:
		case TGL_LINEAR:
			texture_min_filter = param;
			break;
		default:
			goto error;
		}
		break;
	default:
		;
	}
}

void GLContext::glopPixelStore(GLParam *p) {
	int pname = p[1].i;
	int param = p[2].i;

	if (pname != TGL_UNPACK_ALIGNMENT || param != 1) {
		error("tglPixelStore: unsupported option");
	}
}

void GLContext::gl_GenTextures(TGLsizei n, TGLuint *textures) {
	for (int i = 0; i < n; i++) {
		textures[i] = maxTextureName + i + 1;
	}
	maxTextureName += n;
}

void GLContext::gl_DeleteTextures(TGLsizei n, const TGLuint *textures) {
	for (int i = 0; i < n; i++) {
		TinyGL::GLTexture *t = find_texture(textures[i]);
		if (t) {
			if (t == current_texture) {
				current_texture = default_texture;
			}
			t->disposed = true;
		}
	}
}

} // end of namespace TinyGL

