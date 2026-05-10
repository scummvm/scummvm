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
 * Based on the original sources
 *   https://github.com/Croquetx/thecolony
 * Copyright (C) 1988, David A. Smith
 *
 * Distributed under Apache Version 2.0 License
 *
 */

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/font.h"
#include "graphics/surface.h"
#include "math/matrix4.h"

#include "colony/renderer.h"
#include "colony/renderer_opengl_shaders.h"

#ifdef USE_OPENGL_SHADERS

#include "graphics/opengl/shader.h"
#include "graphics/opengl/system_headers.h"

namespace Colony {

// Phase 3: 2D primitives + the 3D corridor draw path are programmable.
// XOR mode and polygon stipple are intentionally left stubbed — see the
// renderer audit for why those are deferred.
class OpenGLShaderRenderer : public Renderer {
public:
	OpenGLShaderRenderer(OSystem *system, int width, int height);
	~OpenGLShaderRenderer() override;

	void clear(uint32 color) override;
	void drawLine(int x1, int y1, int x2, int y2, uint32 color) override;
	void drawRect(const Common::Rect &rect, uint32 color) override;
	void fillRect(const Common::Rect &rect, uint32 color) override;
	void drawString(const Graphics::Font *font, const Common::String &str, int x, int y,
			uint32 color, Graphics::TextAlign align) override;
	void drawEllipse(int x, int y, int rx, int ry, uint32 color) override;
	void fillEllipse(int x, int y, int rx, int ry, uint32 color) override;
	void fillDitherRect(const Common::Rect &rect, uint32 c1, uint32 c2) override;
	void setPixel(int x, int y, uint32 color) override;
	void drawQuad(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, uint32 color) override;
	void drawPolygon(const int *x, const int *y, int count, uint32 color) override;

	void setPalette(const byte *palette, uint start, uint count) override;

	void begin3D(int camX, int camY, int camZ, int angle, int angleY, const Common::Rect &viewport) override;
	void draw3DWall(int x1, int y1, int x2, int y2, uint32 color) override;
	void draw3DQuad(float x1, float y1, float z1, float x2, float y2, float z2,
			float x3, float y3, float z3, float x4, float y4, float z4, uint32 color) override;
	void draw3DPolygon(const float *x, const float *y, const float *z, int count, uint32 color) override;
	void draw3DLine(float x1, float y1, float z1, float x2, float y2, float z2, uint32 color) override;
	void end3D() override;

	void copyToScreen() override;
	void setWireframe(bool enable, int64_t fillColor) override;
	void setXorMode(bool enable) override {}
	void setStippleData(const byte *data) override;
	void setMacColors(uint32 fg, uint32 bg) override;
	void setDepthState(bool testEnabled, bool writeEnabled) override;
	void setDepthRange(float nearVal, float farVal) override;
	void computeScreenViewport() override;

	void drawSurface(const Graphics::Surface *surf, int x, int y) override;
	Graphics::Surface *getScreenshot() override;

private:
	void resolveColor(uint32 color, float rgba[4]) const;
	void rebuildProjection();
	// Push _projection to the 2D shaders. The matrix is constant between
	// resolution changes, so this only needs to run from the constructor
	// and computeScreenViewport — not per draw.
	void uploadProjectionUniform();
	void uploadSolid(const float *positions, int vertCount);
	void drawSolid(GLenum mode, const float *positions, int vertCount, const float rgba[4]);
	void drawTexturedQuad(int x, int y, int w, int h);
	// Set glLineWidth to scale with the window size (Freescape pattern,
	// gfx_opengl_shaders.cpp:692). No-op when mode isn't a line primitive.
	// Cached to avoid redundant state changes across same-width draws.
	void applyLineWidth(GLenum mode);

	void uploadSolid3D(const float *positions, int vertCount);
	// allowStipple=true on the fill pass picks up _stippleActive; lines
	// must always render unstippled (matches the fixed-function path,
	// which only stipples GL_QUADS / GL_POLYGON, never lines).
	void drawSolid3D(GLenum mode, const float *positions, int vertCount,
			const float rgba[4], bool allowStipple = false);
	// Renders a filled+wireframe 3D primitive. Honors _wireframe,
	// _wireframeFillColor, and _stippleActive — same semantics as
	// OpenGLRenderer::draw3DWall/Quad/Polygon.
	void drawWireframeable3D(const float *positions, int vertCount, uint32 color);

	OSystem *_system = nullptr;
	int _width = 0;
	int _height = 0;
	byte _palette[256 * 3] = {};
	Common::Rect _screenViewport;

	OpenGL::Shader *_solidShader = nullptr;
	OpenGL::Shader *_bitmapShader = nullptr;
	OpenGL::Shader *_solid3dShader = nullptr;
	GLuint _solidVBO = 0;
	GLuint _bitmapVBO = 0;
	GLuint _solid3dVBO = 0;
	GLuint _bitmapTexture = 0;

	Math::Matrix4 _projection;
	Math::Matrix4 _mvpMatrix;

	// Cached "color" uniform values per shader. Uniforms persist with the
	// program object across glUseProgram cycles, so once we've uploaded a
	// color, subsequent draws can skip glUniform4fv when the color matches.
	// Adjacent walls / dashboard primitives commonly share colors, so this
	// elides a lot of uniform writes per frame.
	float _solidLastColor[4] = { -1.0f, -1.0f, -1.0f, -1.0f };
	float _solid3dLastColor[4] = { -1.0f, -1.0f, -1.0f, -1.0f };

	// Cached glLineWidth so applyLineWidth() can short-circuit repeats.
	float _lineWidth = 1.0f;

	bool _wireframe = true;
	int64_t _wireframeFillColor = 0; // -1 = no fill, else color (palette idx or ARGB)

	// Stipple state. The shader emulates glPolygonStipple via a 128-int
	// uniform array (Freescape pattern, GLES2-safe). _stippleActive is
	// true when setStippleData() received a non-null pattern. fg/bg are
	// the engine's encoded colors (high byte 0xFF = direct ARGB, else
	// palette index); they are resolved to vec4 before upload.
	//
	// Defaults match OpenGLRenderer (fg = palette index 0 = black,
	// bg = palette index 255 = white). B&W Mac mode never calls
	// setMacColors and relies on these defaults.
	int _stippleShaderArray[128] = {};
	bool _stippleActive = false;
	uint32 _stippleFg = 0;
	uint32 _stippleBg = 255;
	// Per-shader dirty flags. Track what the program object currently has
	// so we can skip redundant uniform uploads (same idea as the color
	// cache). _solid3dStippleEnabled = the value of "useStipple" most
	// recently uploaded to _solid3dShader. _stippleColorsDirty starts true
	// so the first stipple draw uploads the resolved colors even if the
	// engine never calls setMacColors (the B&W Mac path).
	bool _solid3dStippleEnabled = false;
	bool _stipplePatternDirty = false;
	bool _stippleColorsDirty = true;

	// Solid VBO holds vec2 position only. Sized for the worst-case 2D
	// primitive — the dither overlay can stream up to width*height/2 dots,
	// so leave room for typical screens (≈170k floats for an 800×600 split).
	enum { kSolidVertexCapacity = 320 * 1024 };
	// 3D VBO holds vec3 positions. Corridor polygons are typically <16
	// vertices but a few features (sprite billboards, stairs) push higher.
	enum { kSolid3DVertexCapacity = 1024 };
};

// ---------------------------------------------------------------------------
// Construction / teardown
// ---------------------------------------------------------------------------

OpenGLShaderRenderer::OpenGLShaderRenderer(OSystem *system, int width, int height)
	: _system(system), _width(width), _height(height) {
	debug(1, "Colony: using OpenGL shader renderer");
	for (int i = 0; i < 256 * 3; i++)
		_palette[i] = 255;

	static const char *solidAttribs[] = { "position", nullptr };
	_solidShader = OpenGL::Shader::fromFiles("colony_solid", solidAttribs);
	_solidVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER,
		sizeof(float) * 2 * kSolidVertexCapacity, nullptr, GL_DYNAMIC_DRAW);
	_solidShader->enableVertexAttribute("position", _solidVBO, 2, GL_FLOAT, GL_FALSE,
		2 * sizeof(float), 0);

	static const char *bitmapAttribs[] = { "position", "texcoord", nullptr };
	_bitmapShader = OpenGL::Shader::fromFiles("colony_bitmap", bitmapAttribs);
	// Per-draw vec2 position + vec2 texcoord, 4 vertices for a quad.
	_bitmapVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER,
		sizeof(float) * 16, nullptr, GL_DYNAMIC_DRAW);
	_bitmapShader->enableVertexAttribute("position", _bitmapVBO, 2, GL_FLOAT, GL_FALSE,
		4 * sizeof(float), 0);
	_bitmapShader->enableVertexAttribute("texcoord", _bitmapVBO, 2, GL_FLOAT, GL_FALSE,
		4 * sizeof(float), 2 * sizeof(float));

	// 3D solid: vec3 vertex consuming mvpMatrix; the fragment shader has
	// its own stipple-emulation branch (Freescape pattern, GLES2 safe),
	// so we use a dedicated colony_solid_3d.{vertex,fragment} pair.
	static const char *solid3dAttribs[] = { "position", nullptr };
	_solid3dShader = OpenGL::Shader::fromFiles("colony_solid_3d", solid3dAttribs);
	_solid3dVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER,
		sizeof(float) * 3 * kSolid3DVertexCapacity, nullptr, GL_DYNAMIC_DRAW);
	_solid3dShader->enableVertexAttribute("position", _solid3dVBO, 3, GL_FLOAT, GL_FALSE,
		3 * sizeof(float), 0);
	// Initial stipple state: disabled. The fragment shader takes the
	// non-stippled path until setStippleData(non-null) marks otherwise.
	_solid3dShader->use();
	_solid3dShader->setUniform("useStipple", 0u);

	glGenTextures(1, &_bitmapTexture);
	// Texture parameters are per-texture-object state, so they only need to
	// be set once at creation; they persist across glTexImage2D re-uploads.
	// All blits (drawSurface, drawString) reuse this single texture handle
	// and share the same NEAREST filter + CLAMP_TO_EDGE wrap.
	glBindTexture(GL_TEXTURE_2D, _bitmapTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	computeScreenViewport();
	rebuildProjection();
	uploadProjectionUniform();

	// The bitmap shader's sampler is permanently bound to texture unit 0.
	// Setting it once here removes a per-draw setUniform("tex", 0).
	_bitmapShader->use();
	_bitmapShader->setUniform("tex", 0);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

OpenGLShaderRenderer::~OpenGLShaderRenderer() {
	OpenGL::Shader::freeBuffer(_solidVBO);
	OpenGL::Shader::freeBuffer(_bitmapVBO);
	OpenGL::Shader::freeBuffer(_solid3dVBO);
	delete _solidShader;
	delete _bitmapShader;
	delete _solid3dShader;
	if (_bitmapTexture)
		glDeleteTextures(1, &_bitmapTexture);
}

// ---------------------------------------------------------------------------
// Color resolution and matrix setup
// ---------------------------------------------------------------------------

void OpenGLShaderRenderer::resolveColor(uint32 color, float rgba[4]) const {
	// Same convention as the fixed-function renderer: high byte 0xFF →
	// direct ARGB, otherwise palette index (low byte).
	if (color & 0xFF000000) {
		rgba[0] = ((color >> 16) & 0xFF) / 255.0f;
		rgba[1] = ((color >> 8) & 0xFF) / 255.0f;
		rgba[2] = (color & 0xFF) / 255.0f;
	} else {
		const uint32 idx = color & 0xFF;
		rgba[0] = _palette[idx * 3] / 255.0f;
		rgba[1] = _palette[idx * 3 + 1] / 255.0f;
		rgba[2] = _palette[idx * 3 + 2] / 255.0f;
	}
	rgba[3] = 1.0f;
}

void OpenGLShaderRenderer::rebuildProjection() {
	// Build glOrtho(0, _width, _height, 0, -1, 1) row-major in Math::Matrix4,
	// then transpose so glUniformMatrix4fv (which reads column-major) sees
	// the intended matrix. Y is flipped because our engine puts y=0 at top.
	Math::Matrix4 m;
	for (int r = 0; r < 4; r++)
		for (int c = 0; c < 4; c++)
			m(r, c) = 0.0f;
	m(0, 0) = 2.0f / (float)_width;
	m(0, 3) = -1.0f;
	m(1, 1) = -2.0f / (float)_height;
	m(1, 3) = 1.0f;
	m(2, 2) = -1.0f;
	m(3, 3) = 1.0f;
	m.transpose();
	_projection = m;
}

void OpenGLShaderRenderer::uploadProjectionUniform() {
	// Push the current ortho matrix to both 2D shaders. The values persist
	// in each program object until the next setUniform, so subsequent draws
	// don't need to re-upload it. Color stays per-draw because it varies.
	if (_solidShader) {
		_solidShader->use();
		_solidShader->setUniform("projection", _projection);
	}
	if (_bitmapShader) {
		_bitmapShader->use();
		_bitmapShader->setUniform("projection", _projection);
	}
}

// ---------------------------------------------------------------------------
// Solid-color primitives
// ---------------------------------------------------------------------------

void OpenGLShaderRenderer::uploadSolid(const float *positions, int vertCount) {
	// glBufferData (orphan) instead of glBufferSubData: tells the driver
	// the previous contents are dead, so it can hand us fresh storage
	// without waiting for the GPU to finish reading the old data. This
	// matches Freescape's per-draw pattern (gfx_opengl_shaders.cpp:695,
	// 717) and avoids implicit CPU/GPU sync stalls on Mac drivers.
	glBindBuffer(GL_ARRAY_BUFFER, _solidVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * vertCount, positions, GL_DYNAMIC_DRAW);
}

void OpenGLShaderRenderer::applyLineWidth(GLenum mode) {
	if (mode != GL_LINES && mode != GL_LINE_STRIP && mode != GL_LINE_LOOP)
		return;
	// Scale by window pixel width versus engine logical width — mirrors
	// Freescape (gfx_opengl_shaders.cpp:692), MAX(1, ...) for safety.
	// With kSupportsArbitraryResolutions, _system->getWidth() returns the
	// overlay (window) pixel width; without it, getWidth() == _width and
	// the ratio is 1.
	const int sysW = _system ? _system->getWidth() : 0;
	float w = 1.0f;
	if (sysW > _width)
		w = MAX(1.0f, (float)sysW / (float)_width);
	if (w != _lineWidth) {
		glLineWidth(w);
		_lineWidth = w;
	}
}

void OpenGLShaderRenderer::drawSolid(GLenum mode, const float *positions, int vertCount,
		const float rgba[4]) {
	if (vertCount <= 0)
		return;
	uploadSolid(positions, vertCount);
	// "projection" is set once by uploadProjectionUniform() — it persists in
	// the program object across draws. Shader stays bound between draws so
	// Shader::use()'s _previousShader cache short-circuits the rebind.
	_solidShader->use();
	if (rgba[0] != _solidLastColor[0] || rgba[1] != _solidLastColor[1]
			|| rgba[2] != _solidLastColor[2] || rgba[3] != _solidLastColor[3]) {
		_solidShader->setUniform("color",
			Math::Vector4d(rgba[0], rgba[1], rgba[2], rgba[3]));
		_solidLastColor[0] = rgba[0]; _solidLastColor[1] = rgba[1];
		_solidLastColor[2] = rgba[2]; _solidLastColor[3] = rgba[3];
	}
	applyLineWidth(mode);
	glDrawArrays(mode, 0, vertCount);
}

void OpenGLShaderRenderer::clear(uint32 color) {
	float rgba[4];
	resolveColor(color, rgba);
	glClearColor(rgba[0], rgba[1], rgba[2], 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLShaderRenderer::drawLine(int x1, int y1, int x2, int y2, uint32 color) {
	float rgba[4];
	resolveColor(color, rgba);
	const float verts[] = { (float)x1, (float)y1, (float)x2, (float)y2 };
	drawSolid(GL_LINES, verts, 2, rgba);
}

void OpenGLShaderRenderer::drawRect(const Common::Rect &rect, uint32 color) {
	float rgba[4];
	resolveColor(color, rgba);
	const float verts[] = {
		(float)rect.left,  (float)rect.top,
		(float)rect.right, (float)rect.top,
		(float)rect.right, (float)rect.bottom,
		(float)rect.left,  (float)rect.bottom
	};
	drawSolid(GL_LINE_LOOP, verts, 4, rgba);
}

void OpenGLShaderRenderer::fillRect(const Common::Rect &rect, uint32 color) {
	float rgba[4];
	resolveColor(color, rgba);
	// TRIANGLE_STRIP order: top-left, top-right, bottom-left, bottom-right
	const float verts[] = {
		(float)rect.left,  (float)rect.top,
		(float)rect.right, (float)rect.top,
		(float)rect.left,  (float)rect.bottom,
		(float)rect.right, (float)rect.bottom
	};
	drawSolid(GL_TRIANGLE_STRIP, verts, 4, rgba);
}

void OpenGLShaderRenderer::setPixel(int x, int y, uint32 color) {
	// Same as a 1×1 fillRect — this is rarely called now that animation/PICT
	// blits go through drawSurface.
	fillRect(Common::Rect(x, y, x + 1, y + 1), color);
}

void OpenGLShaderRenderer::drawQuad(int x1, int y1, int x2, int y2,
		int x3, int y3, int x4, int y4, uint32 color) {
	float rgba[4];
	resolveColor(color, rgba);
	// Filled body (TRIANGLE_FAN handles convex quads correctly).
	const float fanVerts[] = {
		(float)x1, (float)y1,
		(float)x2, (float)y2,
		(float)x3, (float)y3,
		(float)x4, (float)y4
	};
	drawSolid(GL_TRIANGLE_FAN, fanVerts, 4, rgba);

	// Match the fixed-function renderer's white outline overlay.
	const float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	drawSolid(GL_LINE_LOOP, fanVerts, 4, white);
}

void OpenGLShaderRenderer::drawPolygon(const int *x, const int *y, int count, uint32 color) {
	if (count < 3)
		return;
	if (count > 1024)
		count = 1024;

	float rgba[4];
	resolveColor(color, rgba);

	float verts[2 * 1024];
	for (int i = 0; i < count; i++) {
		verts[i * 2 + 0] = (float)x[i];
		verts[i * 2 + 1] = (float)y[i];
	}
	drawSolid(GL_TRIANGLE_FAN, verts, count, rgba);

	// Same white outline as drawQuad.
	const float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	drawSolid(GL_LINE_LOOP, verts, count, white);
}

void OpenGLShaderRenderer::drawEllipse(int x, int y, int rx, int ry, uint32 color) {
	float rgba[4];
	resolveColor(color, rgba);
	const int kSegments = 36; // 10° steps, matching the fixed-function path
	float verts[kSegments * 2];
	for (int i = 0; i < kSegments; i++) {
		const float rad = i * 2.0f * (float)M_PI / kSegments;
		verts[i * 2 + 0] = x + cosf(rad) * (float)rx;
		verts[i * 2 + 1] = y + sinf(rad) * (float)ry;
	}
	drawSolid(GL_LINE_LOOP, verts, kSegments, rgba);
}

void OpenGLShaderRenderer::fillEllipse(int x, int y, int rx, int ry, uint32 color) {
	float rgba[4];
	resolveColor(color, rgba);
	const int kSegments = 36;
	float verts[kSegments * 2];
	for (int i = 0; i < kSegments; i++) {
		const float rad = i * 2.0f * (float)M_PI / kSegments;
		verts[i * 2 + 0] = x + cosf(rad) * (float)rx;
		verts[i * 2 + 1] = y + sinf(rad) * (float)ry;
	}
	drawSolid(GL_TRIANGLE_FAN, verts, kSegments, rgba);
}

void OpenGLShaderRenderer::fillDitherRect(const Common::Rect &rect, uint32 c1, uint32 c2) {
	fillRect(rect, c1);
	float rgba[4];
	resolveColor(c2, rgba);

	const int w = rect.width();
	const int h = rect.height();
	if (w <= 0 || h <= 0)
		return;

	// 50% checkerboard: place a dot on every other pixel, alternating per row.
	// Capacity guard — fall back to solid c2 if the rect is larger than our
	// streaming buffer (very rare: only the dashboard background hits this
	// path, and it is much smaller than kSolidVertexCapacity).
	const int maxDots = kSolidVertexCapacity;
	int dots = 0;
	float *verts = new float[maxDots * 2];
	for (int yi = 0; yi < h && dots < maxDots; yi++) {
		const int yy = rect.top + yi;
		for (int xi = (yi & 1); xi < w && dots < maxDots; xi += 2) {
			verts[dots * 2 + 0] = (float)(rect.left + xi);
			verts[dots * 2 + 1] = (float)yy;
			dots++;
		}
	}
	if (dots > 0)
		drawSolid(GL_POINTS, verts, dots, rgba);
	delete[] verts;
}

// ---------------------------------------------------------------------------
// Text rendering
// ---------------------------------------------------------------------------

void OpenGLShaderRenderer::drawString(const Graphics::Font *font, const Common::String &str,
		int x, int y, uint32 color, Graphics::TextAlign align) {
	if (!font)
		return;
	const int w = font->getStringWidth(str);
	const int h = font->getFontHeight();
	if (w <= 0 || h <= 0)
		return;

	if (align == Graphics::kTextAlignCenter)
		x -= w / 2;
	else if (align == Graphics::kTextAlignRight)
		x -= w;

	float rgba[4];
	resolveColor(color, rgba);

	// Render glyphs to a 1-byte-per-pixel mask, then build an RGBA image
	// where set pixels carry the requested color (alpha 1) and unset
	// pixels are transparent. Upload as a texture and draw a single quad.
	Graphics::Surface mask;
	mask.create(w, h, Graphics::PixelFormat::createFormatCLUT8());
	memset(mask.getPixels(), 0, w * h);
	font->drawString(&mask, str, 0, 0, w, 1, Graphics::kTextAlignLeft);

	const byte cr = (byte)(rgba[0] * 255.0f);
	const byte cg = (byte)(rgba[1] * 255.0f);
	const byte cb = (byte)(rgba[2] * 255.0f);

	uint32 *rgbaBuf = new uint32[w * h];
	for (int py = 0; py < h; py++) {
		const byte *src = (const byte *)mask.getBasePtr(0, py);
		uint32 *dst = rgbaBuf + py * w;
		for (int px = 0; px < w; px++) {
			if (src[px] == 1)
				dst[px] = ((uint32)cr) | ((uint32)cg << 8) | ((uint32)cb << 16) | (0xFFu << 24);
			else
				dst[px] = 0;
		}
	}
	mask.free();

	// Texture params are set once at construction. The shared _bitmapTexture
	// keeps NEAREST filter + CLAMP_TO_EDGE for both drawString and drawSurface.
	glBindTexture(GL_TEXTURE_2D, _bitmapTexture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaBuf);
	delete[] rgbaBuf;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	drawTexturedQuad(x, y, w, h);
	glDisable(GL_BLEND);
}

// ---------------------------------------------------------------------------
// Surface blit
// ---------------------------------------------------------------------------

void OpenGLShaderRenderer::drawSurface(const Graphics::Surface *surf, int x, int y) {
	if (!surf || surf->w <= 0 || surf->h <= 0)
		return;
	if (surf->format.bytesPerPixel != 4)
		return;

	// Texture params are set once at construction; just bind here.
	glBindTexture(GL_TEXTURE_2D, _bitmapTexture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	// The engine's surface format is PixelFormat(4,8,8,8,8,24,16,8,0) —
	// R at bit 24, A at bit 0 — so GL_RGBA + GL_UNSIGNED_INT_8_8_8_8 reads
	// each uint32 with the high byte mapping to R, matching the fixed-
	// function path's drawSurface upload (renderer_opengl.cpp:725).
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0,
		GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, surf->getPixels());

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	drawTexturedQuad(x, y, surf->w, surf->h);
	glDisable(GL_BLEND);
}

void OpenGLShaderRenderer::drawTexturedQuad(int x, int y, int w, int h) {
	const float verts[] = {
		// position             texcoord
		(float)x,       (float)y,       0.0f, 0.0f,
		(float)(x + w), (float)y,       1.0f, 0.0f,
		(float)x,       (float)(y + h), 0.0f, 1.0f,
		(float)(x + w), (float)(y + h), 1.0f, 1.0f
	};
	// Orphan + write fresh — see uploadSolid.
	glBindBuffer(GL_ARRAY_BUFFER, _bitmapVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);

	// "projection" and "tex" are set once at init / on resolution change;
	// they persist in the program object so we don't re-upload here.
	_bitmapShader->use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _bitmapTexture);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	// Shader stays bound — see drawSolid for the rationale.
}

// ---------------------------------------------------------------------------
// Buffer / state management
// ---------------------------------------------------------------------------

void OpenGLShaderRenderer::setPalette(const byte *palette, uint start, uint count) {
	if (start + count > 256)
		count = 256 - start;
	memcpy(_palette + start * 3, palette, count * 3);
}

void OpenGLShaderRenderer::computeScreenViewport() {
	const int32 screenWidth = _system->getWidth();
	const int32 screenHeight = _system->getHeight();
	const bool widescreen = ConfMan.getBool("widescreen_mod");

	if (widescreen) {
		_screenViewport = Common::Rect(screenWidth, screenHeight);
	} else if (_system->getFeatureState(OSystem::kFeatureAspectRatioCorrection)) {
		const int32 vpW = MIN<int32>(screenWidth, screenHeight * 4 / 3);
		const int32 vpH = MIN<int32>(screenHeight, screenWidth * 3 / 4);
		_screenViewport = Common::Rect(vpW, vpH);
		_screenViewport.translate((screenWidth - vpW) / 2, (screenHeight - vpH) / 2);
	} else {
		_screenViewport = Common::Rect(screenWidth, screenHeight);
	}

	glViewport(_screenViewport.left, screenHeight - _screenViewport.bottom,
		_screenViewport.width(), _screenViewport.height());
	glScissor(_screenViewport.left, screenHeight - _screenViewport.bottom,
		_screenViewport.width(), _screenViewport.height());
}

void OpenGLShaderRenderer::copyToScreen() {
	glFlush();
	_system->updateScreen();
}

Graphics::Surface *OpenGLShaderRenderer::getScreenshot() {
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(_screenViewport.width(), _screenViewport.height(),
		Graphics::PixelFormat::createFormatRGBA32());
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glReadPixels(_screenViewport.left, _system->getHeight() - _screenViewport.bottom,
		_screenViewport.width(), _screenViewport.height(),
		GL_RGBA, GL_UNSIGNED_BYTE, surface->getPixels());
	surface->flipVertical(Common::Rect(surface->w, surface->h));
	return surface;
}

// ---------------------------------------------------------------------------
// 3D corridor / scene rendering
// ---------------------------------------------------------------------------

void OpenGLShaderRenderer::setWireframe(bool enable, int64_t fillColor) {
	_wireframe = enable;
	_wireframeFillColor = fillColor;
}

void OpenGLShaderRenderer::setStippleData(const byte *data) {
	const bool nowActive = (data != nullptr);
	if (nowActive) {
		// Widen the 128 pattern bytes into ints for the uniform array.
		// Mark the pattern dirty so the next 3D draw uploads it.
		for (int i = 0; i < 128; i++)
			_stippleShaderArray[i] = data[i];
		_stipplePatternDirty = true;
	}
	_stippleActive = nowActive;
}

void OpenGLShaderRenderer::setMacColors(uint32 fg, uint32 bg) {
	if (_stippleFg != fg || _stippleBg != bg) {
		_stippleFg = fg;
		_stippleBg = bg;
		_stippleColorsDirty = true;
	}
}

void OpenGLShaderRenderer::setDepthState(bool testEnabled, bool writeEnabled) {
	if (testEnabled)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
	glDepthMask(writeEnabled ? GL_TRUE : GL_FALSE);
}

void OpenGLShaderRenderer::setDepthRange(float nearVal, float farVal) {
	glDepthRange(nearVal, farVal);
}

void OpenGLShaderRenderer::begin3D(int camX, int camY, int camZ, int angle, int angleY,
		const Common::Rect &viewport) {
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);

	// Map the engine's logical viewport into system pixels (matches
	// OpenGLRenderer::begin3D in renderer_opengl.cpp:246-257).
	const float scaleX = (float)_screenViewport.width() / (float)_width;
	const float scaleY = (float)_screenViewport.height() / (float)_height;
	const int sysH = _system->getHeight();
	const int vpX = _screenViewport.left + (int)(viewport.left * scaleX);
	const int vpY = sysH - (_screenViewport.top + (int)(viewport.bottom * scaleY));
	const int vpW = (int)(viewport.width() * scaleX);
	const int vpH = (int)(viewport.height() * scaleY);
	glViewport(vpX, vpY, vpW, vpH);
	glScissor(vpX, vpY, vpW, vpH);
	glEnable(GL_SCISSOR_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);

	// Perspective: 75° vertical FOV, near=1, far=10000 — matches the
	// fixed-function path so geometry lands at the same screen positions.
	const float aspectRatio = (float)viewport.width() / (float)viewport.height();
	const float fov = 75.0f;
	const float nearClip = 1.0f;
	const float farClip = 10000.0f;
	const float ymax = nearClip * tanf(fov * (float)M_PI / 360.0f);
	const float xmax = ymax * aspectRatio;

	// Build perspective frustum directly. Math::makeFrustumMatrix exists in
	// math/glmath.h, but its sign convention requires a final transpose to
	// match Math::Matrix4's row-major storage; constructing the matrix
	// element-by-element here is clearer and easier to audit.
	Math::Matrix4 proj;
	for (int r = 0; r < 4; r++)
		for (int c = 0; c < 4; c++)
			proj(r, c) = 0.0f;
	proj(0, 0) = 2.0f * nearClip / (xmax - (-xmax));
	proj(1, 1) = 2.0f * nearClip / (ymax - (-ymax));
	proj(0, 2) = (xmax + (-xmax)) / (xmax - (-xmax));
	proj(1, 2) = (ymax + (-ymax)) / (ymax - (-ymax));
	proj(2, 2) = -(farClip + nearClip) / (farClip - nearClip);
	proj(2, 3) = -2.0f * farClip * nearClip / (farClip - nearClip);
	proj(3, 2) = -1.0f;

	// View transform: replicate the fixed-function chain
	//   Rx(pitch) * Rx(-90) * Rz(yaw) * T(-cam)
	// applied to column vectors (renderer_opengl.cpp:280-292).
	Math::Matrix4 pitch;
	pitch.buildAroundX((float)angleY * 360.0f / 256.0f);
	Math::Matrix4 minus90;
	minus90.buildAroundX(-90.0f);
	Math::Matrix4 yaw;
	yaw.buildAroundZ(-(float)angle * 360.0f / 256.0f + 90.0f);
	Math::Matrix4 trans; // identity
	trans.setPosition(Math::Vector3d((float)-camX, (float)-camY, (float)-camZ));

	const Math::Matrix4 view = pitch * minus90 * yaw * trans;
	Math::Matrix4 mvp = proj * view;
	mvp.transpose(); // Math::Matrix4 is row-major; setUniform expects column-major.
	_mvpMatrix = mvp;

	// Push mvpMatrix to the 3D shader once per frame (here in begin3D),
	// instead of on every primitive — it doesn't change between draws.
	_solid3dShader->use();
	_solid3dShader->setUniform("mvpMatrix", _mvpMatrix);
}

void OpenGLShaderRenderer::end3D() {
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthRange(0.0, 1.0);
	glDisable(GL_SCISSOR_TEST);

	// Restore the 2D viewport so subsequent overlay draws (dashboard, menu,
	// crosshair, automap) land in the right spot.
	const int sysW = _system->getWidth();
	const int sysH = _system->getHeight();
	glViewport(0, 0, sysW, sysH);
	glScissor(0, 0, sysW, sysH);
	computeScreenViewport();
}

void OpenGLShaderRenderer::uploadSolid3D(const float *positions, int vertCount) {
	// See uploadSolid for the orphan-via-glBufferData rationale.
	glBindBuffer(GL_ARRAY_BUFFER, _solid3dVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * vertCount, positions, GL_DYNAMIC_DRAW);
}

void OpenGLShaderRenderer::drawSolid3D(GLenum mode, const float *positions, int vertCount,
		const float rgba[4], bool allowStipple) {
	if (vertCount <= 0)
		return;
	uploadSolid3D(positions, vertCount);
	// "mvpMatrix" is set in begin3D and persists in the program object.
	_solid3dShader->use();

	// Toggle the shader's stipple branch only when the effective state
	// changes. allowStipple=false short-circuits stipple for line passes.
	const bool wantStipple = allowStipple && _stippleActive;
	if (wantStipple != _solid3dStippleEnabled) {
		_solid3dShader->setUniform("useStipple", wantStipple ? 1u : 0u);
		_solid3dStippleEnabled = wantStipple;
	}

	if (wantStipple) {
		// Pattern + fg/bg colors are uploaded only when they change.
		if (_stipplePatternDirty) {
			_solid3dShader->setUniform("stipple", 128, _stippleShaderArray);
			_stipplePatternDirty = false;
		}
		if (_stippleColorsDirty) {
			float fgRgba[4], bgRgba[4];
			resolveColor(_stippleFg, fgRgba);
			resolveColor(_stippleBg, bgRgba);
			_solid3dShader->setUniform("stippleFg",
				Math::Vector4d(fgRgba[0], fgRgba[1], fgRgba[2], fgRgba[3]));
			_solid3dShader->setUniform("stippleBg",
				Math::Vector4d(bgRgba[0], bgRgba[1], bgRgba[2], bgRgba[3]));
			_stippleColorsDirty = false;
		}
		// "color" uniform isn't sampled when useStipple is set — skip the
		// upload entirely. Invalidate the cache so a later non-stipple
		// draw with the same rgba still uploads.
		_solid3dLastColor[0] = -1.0f;
	} else {
		// Skip the color upload when it matches the previous draw —
		// adjacent corridor walls/quads commonly share a color.
		if (rgba[0] != _solid3dLastColor[0] || rgba[1] != _solid3dLastColor[1]
				|| rgba[2] != _solid3dLastColor[2] || rgba[3] != _solid3dLastColor[3]) {
			_solid3dShader->setUniform("color",
				Math::Vector4d(rgba[0], rgba[1], rgba[2], rgba[3]));
			_solid3dLastColor[0] = rgba[0]; _solid3dLastColor[1] = rgba[1];
			_solid3dLastColor[2] = rgba[2]; _solid3dLastColor[3] = rgba[3];
		}
	}
	applyLineWidth(mode);
	glDrawArrays(mode, 0, vertCount);
}

void OpenGLShaderRenderer::drawWireframeable3D(const float *positions, int vertCount,
		uint32 color) {
	if (vertCount < 3) {
		// Degenerate — render the line directly so callers don't have to
		// special-case 2-vertex inputs.
		float rgba[4];
		resolveColor(color, rgba);
		drawSolid3D(GL_LINE_STRIP, positions, vertCount, rgba);
		return;
	}

	if (_wireframe) {
		// Fill pass: stipple beats wireframeFillColor when active. Pass
		// allowStipple=true so drawSolid3D picks up _stippleActive and
		// uses the fg/bg uniforms; the fill color is ignored in that case.
		if (_stippleActive || _wireframeFillColor != -1) {
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1.1f, 4.0f);
			float fillRgba[4];
			resolveColor(_stippleActive ? 0u : (uint32)_wireframeFillColor, fillRgba);
			drawSolid3D(GL_TRIANGLE_FAN, positions, vertCount, fillRgba, true);
			glDisable(GL_POLYGON_OFFSET_FILL);
		}
		// Edges: lines are never stippled in the fixed-function path.
		float edgeRgba[4];
		resolveColor(color, edgeRgba);
		drawSolid3D(GL_LINE_LOOP, positions, vertCount, edgeRgba, false);
	} else {
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.1f, 4.0f);
		float rgba[4];
		resolveColor(color, rgba);
		drawSolid3D(GL_TRIANGLE_FAN, positions, vertCount, rgba, true);
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
}

void OpenGLShaderRenderer::draw3DWall(int x1, int y1, int x2, int y2, uint32 color) {
	// 256× scale and ±160 height match renderer_opengl.cpp:295-298.
	const float fx1 = x1 * 256.0f, fy1 = y1 * 256.0f;
	const float fx2 = x2 * 256.0f, fy2 = y2 * 256.0f;
	const float verts[12] = {
		fx1, fy1, -160.0f,
		fx2, fy2, -160.0f,
		fx2, fy2,  160.0f,
		fx1, fy1,  160.0f,
	};
	drawWireframeable3D(verts, 4, color);
}

void OpenGLShaderRenderer::draw3DQuad(float x1, float y1, float z1, float x2, float y2, float z2,
		float x3, float y3, float z3, float x4, float y4, float z4, uint32 color) {
	const float verts[12] = {
		x1, y1, z1,
		x2, y2, z2,
		x3, y3, z3,
		x4, y4, z4,
	};
	drawWireframeable3D(verts, 4, color);
}

void OpenGLShaderRenderer::draw3DPolygon(const float *x, const float *y, const float *z,
		int count, uint32 color) {
	if (count < 3)
		return;
	if (count > kSolid3DVertexCapacity)
		count = kSolid3DVertexCapacity;

	float stack[3 * 32];
	float *verts = (count <= 32) ? stack : new float[3 * count];
	for (int i = 0; i < count; i++) {
		verts[i * 3 + 0] = x[i];
		verts[i * 3 + 1] = y[i];
		verts[i * 3 + 2] = z[i];
	}
	drawWireframeable3D(verts, count, color);
	if (verts != stack)
		delete[] verts;
}

void OpenGLShaderRenderer::draw3DLine(float x1, float y1, float z1, float x2, float y2, float z2,
		uint32 color) {
	const float verts[6] = { x1, y1, z1, x2, y2, z2 };
	float rgba[4];
	resolveColor(color, rgba);
	drawSolid3D(GL_LINES, verts, 2, rgba);
}

// ---------------------------------------------------------------------------
// Factory
// ---------------------------------------------------------------------------

Renderer *createOpenGLShaderRenderer(OSystem *system, int width, int height) {
	return new OpenGLShaderRenderer(system, width, height);
}

} // End of namespace Colony

#else

namespace Colony {
Renderer *createOpenGLShaderRenderer(OSystem *system, int width, int height) { return nullptr; }
}

#endif
