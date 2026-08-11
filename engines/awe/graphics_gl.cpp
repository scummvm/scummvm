/* ScummVM - Graphic Adventure AweEngine
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

#include "awe/gfx.h"
#include "awe/system_stub.h"

namespace Awe {

#ifdef TODO
static struct {
	PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
	PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
	PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
	PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
} _fptr;

static void setupFboFuncs() {
#ifdef _WIN32
	_fptr.glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBindFramebuffer");
	_fptr.glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glGenFramebuffers");
	_fptr.glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)SDL_GL_GetProcAddress("glFramebufferTexture2D");
	_fptr.glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)SDL_GL_GetProcAddress("glCheckFramebufferStatus");
#else
	_fptr.glBindFramebuffer = glBindFramebuffer;
	_fptr.glGenFramebuffers = glGenFramebuffers;
	_fptr.glFramebufferTexture2D = glFramebufferTexture2D;
	_fptr.glCheckFramebufferStatus = glCheckFramebufferStatus;
#endif
}

static GLuint kNoTextureId = (GLuint)-1;

static bool hasExtension(const char *exts, const char *name) {
	const char *p = strstr(exts, name);
	if (p) {
		p += strlen(name);
		return *p == ' ' || *p == 0;
	}
	return false;
}

static int roundPow2(int sz) {
	if (sz != 0 && (sz & (sz - 1)) == 0) {
		return sz;
	}
	int textureSize = 1;
	while (textureSize < sz) {
		textureSize <<= 1;
	}
	return textureSize;
}

struct Texture {
	bool _npotTex;
	GLuint _id;
	int _w, _h;
	float _u, _v;
	uint8 *_rgbData;
	const uint8 *_raw16Data;
	int _fmt;

	void init();
	void uploadDataCLUT(const uint8 *data, int srcPitch, int w, int h, const Color *pal);
	void uploadDataRGB(const void *data, int srcPitch, int w, int h, int fmt, int type);
	void draw(int w, int h);
	void clear();
	void readRaw16(const uint8 *src, const Color *pal, int w, int h);
	void readFont(const uint8 *src);
	void readRGB555(const uint16 *src, int w, int h);
};

void Texture::init() {
	_npotTex = false;
	_id = kNoTextureId;
	_w = _h = 0;
	_u = _v = 0.f;
	_rgbData = 0;
	_raw16Data = 0;
	_fmt = -1;
}

static void convertTextureCLUT(const uint8 *src, const int srcPitch, int w, int h, uint8 *dst, int dstPitch, const Color *pal, bool alpha) {
	for (int y = 0; y < h; ++y) {
		int offset = 0;
		for (int x = 0; x < w; ++x) {
			const uint8 color = src[x];
			dst[offset++] = pal[color].r;
			dst[offset++] = pal[color].g;
			dst[offset++] = pal[color].b;
			if (alpha) {
				if (color == 0) {
					dst[offset++] = 0;
				} else {
					dst[offset++] = 255;
				}
			}
		}
		dst += dstPitch;
		src += srcPitch;
	}
}

void Texture::uploadDataCLUT(const uint8 *data, int srcPitch, int w, int h, const Color *pal) {
	if (w != _w || h != _h) {
		free(_rgbData);
		_rgbData = 0;
	}
	int depth = 1;
	int fmt = GL_RGB;
	int type = GL_UNSIGNED_BYTE;
	switch (_fmt) {
	case FMT_CLUT:
		depth = 3;
		fmt = GL_RGB;
		break;
	case FMT_RGB:
		depth = 3;
		fmt = GL_RGB;
		break;
	case FMT_RGBA:
		depth = 4;
		fmt = GL_RGBA;
		break;
	default:
		return;
	}
	const bool alpha = (_fmt == FMT_RGBA);
	if (!_rgbData) {
		_w = _npotTex ? w : roundPow2(w);
		_h = _npotTex ? h : roundPow2(h);
		_rgbData = (uint8 *)malloc(_w * _h * depth);
		if (!_rgbData) {
			return;
		}
		_u = w / (float)_w;
		_v = h / (float)_h;
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glGenTextures(1, &_id);
		glBindTexture(GL_TEXTURE_2D, _id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (fmt == GL_RED) ? GL_NEAREST : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (fmt == GL_RED) ? GL_NEAREST : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		convertTextureCLUT(data, srcPitch, w, h, _rgbData, _w * depth, pal, alpha);
		glTexImage2D(GL_TEXTURE_2D, 0, fmt, _w, _h, 0, fmt, type, _rgbData);
	} else {
		glBindTexture(GL_TEXTURE_2D, _id);
		convertTextureCLUT(data, srcPitch, w, h, _rgbData, _w * depth, pal, alpha);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _w, _h, fmt, type, _rgbData);
	}
}

void Texture::uploadDataRGB(const void *data, int srcPitch, int w, int h, int fmt, int type) {
	_w = w;
	_h = h;
	_u = 1.f;
	_v = 1.f;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &_id);
	glBindTexture(GL_TEXTURE_2D, _id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, fmt, _w, _h, 0, fmt, type, data);
}

void Texture::draw(int w, int h) {
	if (_id != kNoTextureId) {
		glEnable(GL_TEXTURE_2D);
		glColor4ub(255, 255, 255, 255);
		glBindTexture(GL_TEXTURE_2D, _id);
		glBegin(GL_QUADS);
		glTexCoord2f(0.f, 0.f);
		glVertex2i(0, 0);
		glTexCoord2f(_u, 0.f);
		glVertex2i(w, 0);
		glTexCoord2f(_u, _v);
		glVertex2i(w, h);
		glTexCoord2f(0.f, _v);
		glVertex2i(0, h);
		glEnd();
		glDisable(GL_TEXTURE_2D);
	}
}

void Texture::clear() {
	if (_id != kNoTextureId) {
		glDeleteTextures(1, &_id);
		_id = kNoTextureId;
	}
	free(_rgbData);
	_rgbData = 0;
	_raw16Data = 0;
}

void Texture::readRaw16(const uint8 *src, const Color *pal, int w, int h) {
	_raw16Data = src;
	uploadDataCLUT(_raw16Data, w, w, h, pal);
}

void Texture::readFont(const uint8 *src) {
	_fmt = FMT_RGBA;
	const int W = 96 * 8 * 2;
	const int H = 8;
	uint8 *out = (uint8 *)calloc(1, W * H);
	if (out) {
		for (int i = 0; i < 96; ++i) {
			for (int y = 0; y < 8; ++y) {
				uint8 mask = *src++;
				for (int x = 0; x < 8; ++x) {
					out[y * W + i * 16 + x] = (mask >> (7 - x)) & 1;
				}
			}
		}
		Color pal[2];
		pal[0].r = pal[0].g = pal[0].b = 0;
		pal[1].r = pal[1].g = pal[1].b = 255;
		uploadDataCLUT(out, W, W, H, pal);
		free(out);
	}
}

static uint16 rgb555_to_565(const uint16 color) {
	const int r = (color >> 10) & 31;
	const int g = (color >> 5) & 31;
	const int b = color & 31;
	return (r << 11) | (g << 6) | b;
}

void Texture::readRGB555(const uint16 *src, int w, int h) {
	_rgbData = (uint8 *)malloc(w * h * sizeof(uint16));
	if (!_rgbData) {
		return;
	}
	for (int i = 0; i < w * h; ++i) {
		((uint16 *)_rgbData)[i] = rgb555_to_565(src[i]);
	}
	uploadDataRGB(_rgbData, w * sizeof(uint16), w, h, GL_RGB, GL_UNSIGNED_SHORT_5_6_5);
}

struct DrawListEntry {
	static const int NUM_VERTICES = 1024;

	int color;
	int numVertices;
	Point vertices[NUM_VERTICES];
};

struct DrawList {
	typedef std::vector<DrawListEntry> Entries;

	int fillColor;
	Entries entries;
	int yOffset;

	DrawList()
		: fillColor(0), yOffset(0) {
	}

	void clear(uint8 color) {
		fillColor = color;
		entries.clear();
	}

	void append(uint8 color, int count, const Point *vertices) {
		if (count <= DrawListEntry::NUM_VERTICES) {
			DrawListEntry e;
			e.color = color;
			e.numVertices = count;
			memcpy(e.vertices, vertices, count * sizeof(Point));
			entries.push_back(e);
		}
	}
};

static const int SCREEN_W = 320;
static const int SCREEN_H = 200;

static const int DEFAULT_FB_W = 1280;
static const int DEFAULT_FB_H = 800;

static const int NUM_LISTS = 4;

struct GraphicsGL : Graphics {
	int _w, _h;
	int _fbW, _fbH;
	Color _pal[16];
	Color *_alphaColor;
	Texture _backgroundTex;
	Texture _fontTex;
	Texture _spritesTex;
	int _spritesSizeX, _spritesSizeY;
	GLuint _fbPage0;
	GLuint _pageTex[NUM_LISTS];
	DrawList _drawLists[NUM_LISTS];
	struct {
		int num;
		Point pos;
	} _sprite;

	GraphicsGL();
	virtual ~GraphicsGL() {
	}

	virtual void init(int targetW, int targetH);
	virtual void setFont(const uint8 *src, int w, int h);
	virtual void setPalette(const Color *colors, int count);
	virtual void setSpriteAtlas(const uint8 *src, int w, int h, int xSize, int ySize);
	virtual void drawSprite(int listNum, int num, const Point *pt, uint8 color);
	virtual void drawBitmap(int listNum, const uint8 *data, int w, int h, int fmt);
	virtual void drawPoint(int listNum, uint8 color, const Point *pt);
	virtual void drawQuadStrip(int listNum, uint8 color, const QuadStrip *qs);
	virtual void drawStringChar(int listNum, uint8 color, char c, const Point *pt);
	virtual void clearBuffer(int listNum, uint8 color);
	virtual void copyBuffer(int dstListNum, int srcListNum, int vscroll = 0);
	virtual void drawBuffer(int listNum, SystemStub *stub);
	virtual void drawRect(int num, uint8 color, const Point *pt, int w, int h);
	virtual void drawBitmapOverlay(const Graphics::Surface &src, int fmt, SystemStub *stub);

	void initFbo();
	void drawVerticesFlat(int count, const Point *vertices);
	void drawVerticesTex(int count, const Point *vertices);
	void drawVerticesToFb(uint8 color, int count, const Point *vertices);
};

GraphicsGL::GraphicsGL() {
	_fixUpPalette = FIXUP_PALETTE_NONE;
	memset(_pal, 0, sizeof(_pal));
	_alphaColor = &_pal[ALPHA_COLOR_INDEX];
	_spritesSizeX = _spritesSizeY = 0;
	_sprite.num = -1;
}

void GraphicsGL::init(int targetW, int targetH) {
	Graphics::init(targetW, targetH);
	_fbW = (targetW <= 0) ? DEFAULT_FB_W : targetW;
	_fbH = (targetH <= 0) ? DEFAULT_FB_H : targetH;
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	const char *exts = (const char *)glGetString(GL_EXTENSIONS);
	const bool npotTex = hasExtension(exts, "GL_ARB_texture_non_power_of_two");
	const bool hasFbo = hasExtension(exts, "GL_ARB_framebuffer_object");
	_backgroundTex.init();
	_backgroundTex._npotTex = npotTex;
	_fontTex.init();
	_fontTex._npotTex = npotTex;
	_spritesTex.init();
	_spritesTex._npotTex = npotTex;
	if (hasFbo) {
		setupFboFuncs();
		initFbo();
		_fptr.glBindFramebuffer(GL_FRAMEBUFFER, 0);
	} else {
		error("GL_ARB_framebuffer_object is not supported");
	}
}

void GraphicsGL::initFbo() {
	GLint buffersCount;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &buffersCount);
	if (buffersCount < NUM_LISTS) {
		error("GL_MAX_COLOR_ATTACHMENTS is %d", buffersCount);
		return;
	}

	_fptr.glGenFramebuffers(1, &_fbPage0);
	_fptr.glBindFramebuffer(GL_FRAMEBUFFER, _fbPage0);

	glGenTextures(NUM_LISTS, _pageTex);
	for (int i = 0; i < NUM_LISTS; ++i) {
		glBindTexture(GL_TEXTURE_2D, _pageTex[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _fbW, _fbH, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);
		_fptr.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, _pageTex[i], 0);
		int status = _fptr.glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			error("glCheckFramebufferStatus failed, ret %d", status);
			return;
		}
	}

	glViewport(0, 0, _fbW, _fbH);

	const float r = (float)_fbW / SCREEN_W;
	glLineWidth(r);
	glPointSize(r);
}

void GraphicsGL::setFont(const uint8 *src, int w, int h) {
	if (src == 0) {
		_fontTex.readFont(_font);
	} else {
		_fontTex.uploadDataRGB(src, w * 4, w, h, GL_RGBA, GL_UNSIGNED_BYTE);
	}
}

void GraphicsGL::setPalette(const Color *colors, int n) {
	assert(n <= 16);
	for (int i = 0; i < n; ++i) {
		_pal[i] = colors[i];
	}
	if (_fixUpPalette == FIXUP_PALETTE_REDRAW) {
		for (int i = 0; i < NUM_LISTS; ++i) {
			_fptr.glBindFramebuffer(GL_FRAMEBUFFER, _fbPage0);
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0, _fbW, 0, _fbH, 0, 1);

			const int color = _drawLists[i].fillColor;
			if (color != COL_BMP) {
				assert(color < 16);
				glClearColor(_pal[color].r / 255.f, _pal[color].g / 255.f, _pal[color].b / 255.f, 1.f);
				glClear(GL_COLOR_BUFFER_BIT);
			}

			glScalef((float)_fbW / SCREEN_W, (float)_fbH / SCREEN_H, 1);

			DrawList::Entries::const_iterator it = _drawLists[i].entries.begin();
			for (; it != _drawLists[i].entries.end(); ++it) {
				DrawListEntry e = *it;
				if (e.color < 16) {
					glColor4ub(_pal[e.color].r, _pal[e.color].g, _pal[e.color].b, 255);
					drawVerticesFlat(e.numVertices, e.vertices);
				} else if (e.color == COL_ALPHA) {
					glColor4ub(_alphaColor->r, _alphaColor->g, _alphaColor->b, 192);
					drawVerticesFlat(e.numVertices, e.vertices);
				}
			}

			glLoadIdentity();
			glScalef(1., 1., 1.);
		}
	}
}

void GraphicsGL::setSpriteAtlas(const uint8 *src, int w, int h, int xSize, int ySize) {
	_spritesTex.uploadDataRGB(src, w * 4, w, h, GL_RGBA, GL_UNSIGNED_BYTE);
	_spritesSizeX = xSize;
	_spritesSizeY = ySize;
}

static void drawTexQuad(const int *pos, const float *uv, GLuint tex) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);
	glBegin(GL_QUADS);
	glTexCoord2f(uv[0], uv[1]);
	glVertex2i(pos[0], pos[1]);
	glTexCoord2f(uv[2], uv[1]);
	glVertex2i(pos[2], pos[1]);
	glTexCoord2f(uv[2], uv[3]);
	glVertex2i(pos[2], pos[3]);
	glTexCoord2f(uv[0], uv[3]);
	glVertex2i(pos[0], pos[3]);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

static void drawSpriteHelper(const Point *pt, int num, int xSize, int ySize, int texId) {
	const int wSize = 18;
	const int hSize = 18;
	const int pos[4] = {
		pt->x, pt->y,
		pt->x + wSize, pt->y + hSize
	};
	int u = num % xSize;
	int v = num / ySize;
	const float uv[4] = {
		u * 1.f / xSize, v * 1.f / ySize,
		(u + 1) * 1.f / xSize, (v + 1) * 1.f / ySize
	};
	glColor4ub(255, 255, 255, 255);
	drawTexQuad(pos, uv, texId);
}

void GraphicsGL::drawSprite(int listNum, int num, const Point *pt, uint8 color) {
	assert(listNum < NUM_LISTS);
	_fptr.glBindFramebuffer(GL_FRAMEBUFFER, _fbPage0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0 + listNum);

	glViewport(0, 0, _fbW, _fbH);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _fbW, 0, _fbH, 0, 1);

	glScalef((float)_fbW / SCREEN_W, (float)_fbH / SCREEN_H, 1);

	drawSpriteHelper(pt, num, _spritesSizeX, _spritesSizeY, _spritesTex._id);

	glLoadIdentity();
	glScalef(1., 1., 1.);
}

void GraphicsGL::drawBitmap(int listNum, const uint8 *data, int w, int h, int fmt) {
	_backgroundTex._fmt = fmt;
	switch (fmt) {
	case FMT_CLUT:
		_backgroundTex.readRaw16(data, _pal, w, h);
		break;
	case FMT_RGB:
		_backgroundTex.clear();
		_backgroundTex.uploadDataRGB(data, w * 3, w, h, GL_RGB, GL_UNSIGNED_BYTE);
		break;
	case FMT_RGB555:
		_backgroundTex.clear();
		_backgroundTex.readRGB555((const uint16 *)data, w, h);
		break;
	default:
		break;
	}
	_fptr.glBindFramebuffer(GL_FRAMEBUFFER, _fbPage0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0 + listNum);

	glViewport(0, 0, _fbW, _fbH);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _fbW, 0, _fbH, 0, 1);

	_backgroundTex.draw(_fbW, _fbH);

	_drawLists[listNum].clear(COL_BMP);
}

void GraphicsGL::drawVerticesToFb(uint8 color, int count, const Point *vertices) {
	glScalef((float)_fbW / SCREEN_W, (float)_fbH / SCREEN_H, 1);

	if (color == COL_PAGE) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, _pageTex[0]);
		glColor4f(1., 1., 1., 1.);
		drawVerticesTex(count, vertices);
		glDisable(GL_TEXTURE_2D);
	} else {
		if (color == COL_ALPHA) {
			glColor4ub(_alphaColor->r, _alphaColor->g, _alphaColor->b, 192);
		} else {
			assert(color < 16);
			glColor4ub(_pal[color].r, _pal[color].g, _pal[color].b, 255);
		}
		drawVerticesFlat(count, vertices);
	}

	glLoadIdentity();
	glScalef(1., 1., 1.);
}

void GraphicsGL::drawPoint(int listNum, uint8 color, const Point *pt) {
	assert(listNum < NUM_LISTS);
	_fptr.glBindFramebuffer(GL_FRAMEBUFFER, _fbPage0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0 + listNum);

	glViewport(0, 0, _fbW, _fbH);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _fbW, 0, _fbH, 0, 1);

	drawVerticesToFb(color, 1, pt);
	if (_fixUpPalette != FIXUP_PALETTE_NONE) {
		_drawLists[listNum].append(color, 1, pt);
	}
}

void GraphicsGL::drawQuadStrip(int listNum, uint8 color, const QuadStrip *qs) {
	assert(listNum < NUM_LISTS);
	_fptr.glBindFramebuffer(GL_FRAMEBUFFER, _fbPage0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0 + listNum);

	glViewport(0, 0, _fbW, _fbH);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _fbW, 0, _fbH, 0, 1);

	drawVerticesToFb(color, qs->numVertices, qs->vertices);
	if (_fixUpPalette != FIXUP_PALETTE_NONE) {
		_drawLists[listNum].append(color, qs->numVertices, qs->vertices);
	}
}

void GraphicsGL::drawStringChar(int listNum, uint8 color, char c, const Point *pt) {
	assert(listNum < NUM_LISTS);
	_fptr.glBindFramebuffer(GL_FRAMEBUFFER, _fbPage0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0 + listNum);

	glViewport(0, 0, _fbW, _fbH);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _fbW, 0, _fbH, 0, 1);

	glScalef((float)_fbW / SCREEN_W, (float)_fbH / SCREEN_H, 1);

	glColor4ub(_pal[color].r, _pal[color].g, _pal[color].b, 255);
	if (_fontTex._h == 8) {
		const int pos[4] = {
			pt->x, pt->y,
			pt->x + 8, pt->y + 8
		};
		const float uv[4] = {
			(c - 0x20) * 16.f / _fontTex._w, 0.f,
			(c - 0x20) * 16.f / _fontTex._w + 1 * 8.f / _fontTex._w, 1.f
		};
		drawTexQuad(pos, uv, _fontTex._id);
	} else {
		const int pos[4] = {
			pt->x - 8, pt->y,
			pt->x, pt->y + 8
		};
		float uv[4];
		uv[0] = (c % 16) * 16 / 256.f;
		uv[2] = uv[0] + 16 / 256.f;
		uv[1] = (c / 16) * 16 / 256.f;
		uv[3] = uv[1] + 16 / 256.f;
		drawTexQuad(pos, uv, _fontTex._id);
	}

	glLoadIdentity();
	glScalef(1., 1., 1.);
}

void GraphicsGL::drawVerticesFlat(int count, const Point *vertices) {
	switch (count) {
	case 1:
		glBegin(GL_POINTS);
		glVertex2i(vertices[0].x, vertices[0].y);
		glEnd();
		break;
	case 2:
		glBegin(GL_LINES);
		if (vertices[1].x > vertices[0].x) {
			glVertex2i(vertices[0].x, vertices[0].y);
			glVertex2i(vertices[1].x + 1, vertices[1].y);
		} else {
			glVertex2i(vertices[1].x, vertices[1].y);
			glVertex2i(vertices[0].x + 1, vertices[0].y);
		}
		glEnd();
		break;
	default:
		glBegin(GL_QUAD_STRIP);
		for (int i = 0; i < count / 2; ++i) {
			const int j = count - 1 - i;
			if (vertices[j].x > vertices[i].x) {
				glVertex2i(vertices[i].x, vertices[i].y);
				glVertex2i(vertices[j].x + 1, vertices[j].y);
			} else {
				glVertex2i(vertices[j].x, vertices[j].y);
				glVertex2i(vertices[i].x + 1, vertices[i].y);
			}
		}
		glEnd();
		break;
	}
}

void GraphicsGL::drawVerticesTex(int count, const Point *vertices) {
	if (count < 4) {
		warning("Invalid vertices count for drawing mode 0x11", count);
		return;
	}
	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i < count / 2; ++i) {
		const int j = count - 1 - i;
		if (vertices[j].x > vertices[i].y) {
			glTexCoord2f(vertices[i].x / 320., vertices[i].y / 200.);
			glVertex2i(vertices[i].x, vertices[i].y);
			glTexCoord2f((vertices[j].x + 1) / 320., vertices[j].y / 200.);
			glVertex2i((vertices[j].x + 1), vertices[j].y);
		} else {
			glTexCoord2f(vertices[j].x / 320., vertices[j].y / 200.);
			glVertex2i(vertices[j].x, vertices[j].y);
			glTexCoord2f((vertices[i].x + 1) / 320., vertices[i].y / 200.);
			glVertex2i((vertices[i].x + 1), vertices[i].y);
		}
	}
	glEnd();
}

void GraphicsGL::clearBuffer(int listNum, uint8 color) {
	assert(listNum < NUM_LISTS);
	_fptr.glBindFramebuffer(GL_FRAMEBUFFER, _fbPage0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0 + listNum);

	glViewport(0, 0, _fbW, _fbH);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _fbW, 0, _fbH, 0, 1);

	assert(color < 16);
	glClearColor(_pal[color].r / 255.f, _pal[color].g / 255.f, _pal[color].b / 255.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	_drawLists[listNum].clear(color);
}

static void drawTextureFb(GLuint tex, int w, int h, int vscroll) {
	glColor4ub(255, 255, 255, 255);
	const int pos[] = {
		0, vscroll,
		w, h + vscroll
	};
	const float uv[] = {
		0., 0.,
		1., 1.
	};
	drawTexQuad(pos, uv, tex);
}

void GraphicsGL::copyBuffer(int dstListNum, int srcListNum, int vscroll) {
	assert(dstListNum < NUM_LISTS && srcListNum < NUM_LISTS);

	_fptr.glBindFramebuffer(GL_FRAMEBUFFER, _fbPage0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0 + dstListNum);

	glViewport(0, 0, _fbW, _fbH);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _fbW, 0, _fbH, 0, 1);

	const int yoffset = vscroll * _fbH / (SCREEN_H - 1);
	drawTextureFb(_pageTex[srcListNum], _fbW, _fbH, yoffset);

	_drawLists[dstListNum] = _drawLists[srcListNum];
	_drawLists[dstListNum].yOffset = vscroll;
}

static void dumpPalette(const Color *pal) {
	static const int SZ = 32;
	int x2, x1 = 0;
	for (int i = 0; i < 16; ++i) {
		x2 = x1 + SZ;
		glColor4ub(pal[i].r, pal[i].g, pal[i].b, 255);
		glBegin(GL_QUADS);
		glVertex2i(x1, 0);
		glVertex2i(x2, 0);
		glVertex2i(x2, SZ);
		glVertex2i(x1, SZ);
		glEnd();
		x1 = x2;
	}
}

void GraphicsGL::drawBuffer(int listNum, SystemStub *stub) {
	assert(listNum < NUM_LISTS);

	_fptr.glBindFramebuffer(GL_FRAMEBUFFER, 0);

	float ar[4];
	stub->prepareScreen(_w, _h, ar);

	glViewport(0, 0, _w, _h);

	glClearColor(0., 0., 0., 1.);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _w, _h, 0, 0, 1);

	glPushMatrix();
	glTranslatef(ar[0] * _w, ar[1] * _h, 0.);
	glScalef(ar[2], ar[3], 1.);

	drawTextureFb(_pageTex[listNum], _w, _h, 0);
	if (0) {
		glDisable(GL_TEXTURE_2D);
		dumpPalette(_pal);
	}

	glPopMatrix();
	stub->updateScreen();
}

void GraphicsGL::drawRect(int num, uint8 color, const Point *pt, int w, int h) {

	// ignore 'num' target framebuffer as this is only used for the title screen with the 3DO version
	assert(color < 16);
	glColor4ub(_pal[color].r, _pal[color].g, _pal[color].b, 255);

	glScalef((float)_fbW / SCREEN_W, (float)_fbH / SCREEN_H, 1);
	const int x1 = pt->x;
	const int y1 = pt->y;
	const int x2 = x1 + w - 1;
	const int y2 = y1 + h - 1;
	glBegin(GL_LINES);
	// horizontal
	glVertex2i(x1, y1);
	glVertex2i(x2, y1);
	glVertex2i(x1, y2);
	glVertex2i(x2, y2);
	// vertical
	glVertex2i(x1, y1);
	glVertex2i(x1, y2);
	glVertex2i(x2, y1);
	glVertex2i(x2, y2);
	glEnd();
}

void GraphicsGL::drawBitmapOverlay(const Graphics::Surface &src, int fmt, SystemStub *stub) {
	if (fmt == FMT_RGB555) {
	}
}

Graphics *GraphicsGL_create() {
	return new GraphicsGL();
}

#endif

} // namespace Awe
