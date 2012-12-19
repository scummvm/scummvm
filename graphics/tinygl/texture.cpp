
// Texture Manager

#include "graphics/tinygl/zgl.h"

namespace TinyGL {

static GLTexture *find_texture(GLContext *c, int h) {
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
	GLTexture *t, **ht;
	GLImage *im;
	int i;

	t = find_texture(c, h);
	if (!t->prev) {
		ht = &c->shared_state.texture_hash_table[t->handle % TEXTURE_HASH_TABLE_SIZE];
		*ht = t->next;
	} else {
		t->prev->next = t->next;
	}
	if (t->next)
		t->next->prev = t->prev;

	for (i = 0; i < MAX_TEXTURE_LEVELS; i++) {
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
	int components = p[3].i;
	int width = p[4].i;
	int height = p[5].i;
	int border = p[6].i;
	int format = p[7].i;
	int type = p[8].i;
	void *pixels = p[9].p;
	GLImage *im;
	byte *pixels1;
	bool do_free_after_rgb2rgba = false;

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
			error("glTexImage2D: Pixel format not handled.");
	}

	Graphics::PixelFormat pf;
	switch (format) {
		case TGL_RGBA:
		case TGL_RGB:
#if defined(SCUMM_BIG_ENDIAN)
			pf = Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24);
#elif defined(SCUMM_LITTLE_ENDIAN)
			pf = Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
#endif
			break;
		case TGL_BGRA:
		case TGL_BGR:
#if defined(SCUMM_BIG_ENDIAN)
			pf = Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
#elif defined(SCUMM_LITTLE_ENDIAN)
			pf = Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24);
#endif
			break;
		default:
			break;
	}
	int bytes = pf.bytesPerPixel;

	// Simply unpack RGB into RGBA with 255 for Alpha.
	// FIXME: This will need additional checks when we get around to adding 24/32-bit backend.
	if (target == TGL_TEXTURE_2D && level == 0 && components == 3 && border == 0) {
		if (format == TGL_RGB || format == TGL_BGR) {
			Graphics::PixelBuffer temp(pf, width * height, DisposeAfterUse::NO);
			Graphics::PixelBuffer pixPtr(sourceFormat, (byte *)pixels);

			for (int i = 0; i < width * height; ++i) {
				uint8 r, g, b;
				pixPtr.getRGBAt(i, r, g, b);
				temp.setPixelAt(i, 255, r, g, b);
			}
			format = TGL_RGBA;
			pixels = temp.getRawBuffer();
			do_free_after_rgb2rgba = true;
		}
	} else if (!(target == TGL_TEXTURE_2D && level == 0 && components == 3 && border == 0
				&& format == TGL_RGBA && type == TGL_UNSIGNED_BYTE)) {
		error("glTexImage2D: combination of parameters not handled");
	}

	pixels1 = new byte[256 * 256 * bytes];
	if (width != 256 || height != 256) {
		// no interpolation is done here to respect the original image aliasing !
		//gl_resizeImageNoInterpolate(pixels1, 256, 256, (unsigned char *)pixels, width, height);
		// used interpolation anyway, it look much better :) --- aquadran
		gl_resizeImage(pixels1, 256, 256, (byte *)pixels, width, height);
		width = 256;
		height = 256;
	} else {
		memcpy(pixels1, pixels, 256 * 256 * bytes);
	}

	im = &c->current_texture->images[level];
	im->xsize = width;
	im->ysize = height;
	if (im->pixmap)
		im->pixmap.free();
	im->pixmap = Graphics::PixelBuffer(pf, pixels1);

	if (do_free_after_rgb2rgba)
		gl_free(pixels);
}

// TODO: not all tests are done
void glopTexEnv(GLContext *, GLParam *p) {
	int target = p[1].i;
	int pname = p[2].i;
	int param = p[3].i;

	if (target != TGL_TEXTURE_ENV) {
error:
		error("glTexParameter: unsupported option");
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
		error("glTexParameter: unsupported option");
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
		error("glPixelStore: unsupported option");
	}
}

} // end of namespace TinyGL

void tglGenTextures(int n, unsigned int *textures) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	int max, i;
	TinyGL::GLTexture *t;

	max = 0;
	for (i = 0; i < TEXTURE_HASH_TABLE_SIZE; i++) {
		t = c->shared_state.texture_hash_table[i];
		while (t) {
			if (t->handle > max)
				max = t->handle;
			t = t->next;
		}
	}
	for (i = 0; i < n; i++) {
		textures[i] = max + i + 1;
	}
}

void tglDeleteTextures(int n, const unsigned int *textures) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	int i;
	TinyGL::GLTexture *t;

	for (i = 0; i < n; i++) {
		t = TinyGL::find_texture(c, textures[i]);
		if (t) {
			if (t == c->current_texture) {
				tglBindTexture(TGL_TEXTURE_2D, 0);
			}
			TinyGL::free_texture(c, textures[i]);
		}
	}
}
