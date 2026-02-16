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

#include "common/system.h"
#include "common/config-manager.h"
#include "graphics/surface.h"
#include "graphics/font.h"
#include <math.h>
#include "colony/renderer.h"

#ifdef USE_OPENGL_GAME
#include "graphics/opengl/system_headers.h"
#include "graphics/paletteman.h"

namespace Colony {

class OpenGLRenderer : public Renderer {
public:
	OpenGLRenderer(OSystem *system, int width, int height);
	~OpenGLRenderer();

	void clear(uint32 color) override;
	void drawLine(int x1, int y1, int x2, int y2, uint32 color) override;
	void drawRect(const Common::Rect &rect, uint32 color) override;
	void fillRect(const Common::Rect &rect, uint32 color) override;
	void drawString(const Graphics::Font *font, const Common::String &str, int x, int y, uint32 color, Graphics::TextAlign align) override;
	void scroll(int dx, int dy, uint32 background) override;
	void drawEllipse(int x, int y, int rx, int ry, uint32 color) override;
	void fillEllipse(int x, int y, int rx, int ry, uint32 color) override;
	void fillDitherRect(const Common::Rect &rect, uint32 color1, uint32 color2) override;
	void setPixel(int x, int y, uint32 color) override;
	void setPalette(const byte *palette, uint start, uint count) override;

	void begin3D(int camX, int camY, int camZ, int angle, int angleY, const Common::Rect &viewport) override;
	void draw3DWall(int x1, int y1, int x2, int y2, uint32 color) override;
	void draw3DQuad(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4, uint32 color) override;
	void draw3DPolygon(const float *x, const float *y, const float *z, int count, uint32 color) override;
	void draw3DLine(float x1, float y1, float z1, float x2, float y2, float z2, uint32 color) override;
	void end3D() override;

	void drawQuad(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, uint32 color) override;
	void drawPolygon(const int *x, const int *y, int count, uint32 color) override;
	void copyToScreen() override;
	void setWireframe(bool enable, int fillColor = -1) override { 
		_wireframe = enable; 
		if (fillColor != -1) _wireframeFillColor = (uint32)fillColor;
	}
	void computeScreenViewport() override;

private:
	void useColor(uint32 color);

	OSystem *_system;
	int _width;
	int _height;
	byte _palette[256 * 3];
	bool _wireframe;
	uint32 _wireframeFillColor;
	Common::Rect _screenViewport;
};

OpenGLRenderer::OpenGLRenderer(OSystem *system, int width, int height) : _system(system), _width(width), _height(height) {
	_wireframe = true;
	_wireframeFillColor = 0;
	memset(_palette, 0, sizeof(_palette));
	
	// Default to white for initial colors if setPalette isn't called yet
	for (int i = 0; i < 256 * 3; i++) _palette[i] = 255;

	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _width, _height, 0, -1, 1);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	computeScreenViewport();
	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

OpenGLRenderer::~OpenGLRenderer() {
}

void OpenGLRenderer::setPalette(const byte *palette, uint start, uint count) {
	if (start + count > 256) count = 256 - start;
	memcpy(_palette + start * 3, palette, count * 3);
}

void OpenGLRenderer::useColor(uint32 color) {
	uint32 index = color & 0xFF;
	glColor3ub(_palette[index * 3], _palette[index * 3 + 1], _palette[index * 3 + 2]);
}

void OpenGLRenderer::clear(uint32 color) {
	uint32 index = color & 0xFF;
	glClearColor(_palette[index * 3] / 255.0f, _palette[index * 3 + 1] / 255.0f, _palette[index * 3 + 2] / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::drawLine(int x1, int y1, int x2, int y2, uint32 color) {
	useColor(color);
	glBegin(GL_LINES);
	glVertex2i(x1, y1);
	glVertex2i(x2, y2);
	glEnd();
}

void OpenGLRenderer::drawRect(const Common::Rect &rect, uint32 color) {
	useColor(color);
	glBegin(GL_LINE_LOOP);
	glVertex2i(rect.left, rect.top);
	glVertex2i(rect.right, rect.top);
	glVertex2i(rect.right, rect.bottom);
	glVertex2i(rect.left, rect.bottom);
	glEnd();
}

void OpenGLRenderer::fillRect(const Common::Rect &rect, uint32 color) {
	useColor(color);
	glBegin(GL_QUADS);
	glVertex2i(rect.left, rect.top);
	glVertex2i(rect.right, rect.top);
	glVertex2i(rect.right, rect.bottom);
	glVertex2i(rect.left, rect.bottom);
	glEnd();
}

void OpenGLRenderer::drawString(const Graphics::Font *font, const Common::String &str, int x, int y, uint32 color, Graphics::TextAlign align) {
	if (!font) return;
	int width = font->getStringWidth(str);
	int height = font->getFontHeight();
	if (align == Graphics::kTextAlignCenter) x -= width / 2;
	else if (align == Graphics::kTextAlignRight) x -= width;
	
	Graphics::Surface surface;
	surface.create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	memset(surface.getPixels(), 0, width * height);
	font->drawString(&surface, str, 0, 0, width, 1, Graphics::kTextAlignLeft);
	
	for (int py = 0; py < height; py++) {
		for (int px = 0; px < width; px++) {
			if (*((byte *)surface.getBasePtr(px, py)) == 1) {
				setPixel(x + px, y + py, color);
			}
		}
	}
	surface.free();
}
 
void OpenGLRenderer::begin3D(int camX, int camY, int camZ, int angle, int angleY, const Common::Rect &viewport) {
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);
 
	// Always use filled polygons; wireframe mode draws black fill + colored edges
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
 
	// Scale viewport coordinates to system pixels
	float scaleX = (float)_screenViewport.width() / _width;
	float scaleY = (float)_screenViewport.height() / _height;
	int sysH = _system->getHeight();
 
	int vpX = _screenViewport.left + (int)(viewport.left * scaleX);
	int vpY = sysH - (_screenViewport.top + (int)(viewport.bottom * scaleY));
	int vpW = (int)(viewport.width() * scaleX);
	int vpH = (int)(viewport.height() * scaleY);
 
	glViewport(vpX, vpY, vpW, vpH);
	glScissor(vpX, vpY, vpW, vpH);
	glEnable(GL_SCISSOR_TEST);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
 
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	// Use a proper FOV-based perspective projection (like Freescape).
	// A fixed vertical FOV ensures the forward direction stays centered
	// regardless of viewport dimensions (e.g. when dashboard narrows it).
	float aspectRatio = (float)viewport.width() / (float)viewport.height();
	float fov = 75.0f; // vertical FOV in degrees
	float nearClip = 1.0f;
	float farClip = 10000.0f;
	float ymax = nearClip * tanf(fov * M_PI / 360.0f);
	float xmax = ymax * aspectRatio;
	glFrustum(-xmax, xmax, -ymax, ymax, nearClip, farClip);
 
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
  
	// 1. Vertical look (Pitch) - around camera X axis
	glRotatef((float)angleY * 360.0f / 256.0f, 1.0f, 0.0f, 0.0f);
 
	// 2. Map world to camera orientation (-90 deg X)
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
 
	// 3. Horizontal rotation (Yaw) around World Z axis (Up)
	// +90° compensates for the -90° X rotation above: without it the camera
	// looks along +Y at angle=0, but movement cos/sin uses +X as angle=0.
	glRotatef(-(float)angle * 360.0f / 256.0f + 90.0f, 0.0f, 0.0f, 1.0f);
	
	// 4. Translate camera
	glTranslatef(-(float)camX, -(float)camY, -(float)camZ);
}
 
void OpenGLRenderer::draw3DWall(int x1, int y1, int x2, int y2, uint32 color) {
	float fx1 = x1 * 256.0f;
	float fy1 = y1 * 256.0f;
	float fx2 = x2 * 256.0f;
	float fy2 = y2 * 256.0f;

	if (_wireframe) {
		// Pass 1: Draw background fill (pushed back)
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.1f, 4.0f);
		useColor(_wireframeFillColor);
		glBegin(GL_QUADS);
		glVertex3f(fx1, fy1, -160.0f);
		glVertex3f(fx2, fy2, -160.0f);
		glVertex3f(fx2, fy2, 160.0f);
		glVertex3f(fx1, fy1, 160.0f);
		glEnd();

		// Pass 2: Draw colored wireframe edges pulled forward relative to the fill
		glEnable(GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(0.0f, -1.0f); // Pull lines forward from the fill
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		useColor(color);
		glBegin(GL_QUADS);
		glVertex3f(fx1, fy1, -160.0f);
		glVertex3f(fx2, fy2, -160.0f);
		glVertex3f(fx2, fy2, 160.0f);
		glVertex3f(fx1, fy1, 160.0f);
		glEnd();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_POLYGON_OFFSET_LINE);
		glDisable(GL_POLYGON_OFFSET_FILL);
	} else {
		// Normal mode: push the wall face back slightly.
		// This ensures that wall features drawn later as lines at the same depth correctly win the depth test.
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.1f, 4.0f); // Positive pulls AWAY from camera
		useColor(color);
		glBegin(GL_QUADS);
		glVertex3f(fx1, fy1, -160.0f);
		glVertex3f(fx2, fy2, -160.0f);
		glVertex3f(fx2, fy2, 160.0f);
		glVertex3f(fx1, fy1, 160.0f);
		glEnd();
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
}
 
void OpenGLRenderer::draw3DQuad(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4, uint32 color) {
	if (_wireframe) {
		// Pass 1: Draw background fill (pushed back)
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.1f, 4.0f);
		useColor(_wireframeFillColor);
		glBegin(GL_QUADS);
		glVertex3f(x1, y1, z1);
		glVertex3f(x2, y2, z2);
		glVertex3f(x3, y3, z3);
		glVertex3f(x4, y4, z4);
		glEnd();

		// Pass 2: Draw colored wireframe edges pulled forward
		glEnable(GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(0.0f, -1.0f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		useColor(color);
		glBegin(GL_QUADS);
		glVertex3f(x1, y1, z1);
		glVertex3f(x2, y2, z2);
		glVertex3f(x3, y3, z3);
		glVertex3f(x4, y4, z4);
		glEnd();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_POLYGON_OFFSET_LINE);
		glDisable(GL_POLYGON_OFFSET_FILL);
	} else {
		// Normal mode: push back to allow overlays (like wall features or floor decorations)
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.1f, 4.0f);
		useColor(color);
		glBegin(GL_QUADS);
		glVertex3f(x1, y1, z1);
		glVertex3f(x2, y2, z2);
		glVertex3f(x3, y3, z3);
		glVertex3f(x4, y4, z4);
		glEnd();
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
}
 
void OpenGLRenderer::draw3DPolygon(const float *x, const float *y, const float *z, int count, uint32 color) {
	if (count < 3) return;

	if (_wireframe) {
		// Pass 1: Draw background fill (pushed back)
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.1f, 4.0f);
		useColor(_wireframeFillColor);
		glBegin(GL_POLYGON);
		for (int i = 0; i < count; i++)
			glVertex3f(x[i], y[i], z[i]);
		glEnd();

		// Pass 2: Draw colored wireframe edges pulled forward
		glEnable(GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(0.0f, -1.0f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		useColor(color);
		glBegin(GL_POLYGON);
		for (int i = 0; i < count; i++)
			glVertex3f(x[i], y[i], z[i]);
		glEnd();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_POLYGON_OFFSET_LINE);
		glDisable(GL_POLYGON_OFFSET_FILL);
	} else {
		// Normal mode: push back
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.1f, 4.0f);
		useColor(color);
		glBegin(GL_POLYGON);
		for (int i = 0; i < count; i++)
			glVertex3f(x[i], y[i], z[i]);
		glEnd();
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
}

void OpenGLRenderer::draw3DLine(float x1, float y1, float z1, float x2, float y2, float z2, uint32 color) {
	useColor(color);
	glBegin(GL_LINES);
	glVertex3f(x1, y1, z1);
	glVertex3f(x2, y2, z2);
	glEnd();
}
 
void OpenGLRenderer::end3D() {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_SCISSOR_TEST);
	
	glViewport(0, 0, _system->getWidth(), _system->getHeight());
	glScissor(0, 0, _system->getWidth(), _system->getHeight());
 
	// Reset to 2D
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	computeScreenViewport();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _width, _height, 0, -1, 1);
}

void OpenGLRenderer::computeScreenViewport() {
	int32 screenWidth = _system->getWidth();
	int32 screenHeight = _system->getHeight();

	bool widescreen = ConfMan.getBool("widescreen_mod");

	if (widescreen) {
		_screenViewport = Common::Rect(screenWidth, screenHeight);
	} else if (_system->getFeatureState(OSystem::kFeatureAspectRatioCorrection)) {
		// Aspect ratio correction (4:3)
		int32 viewportWidth = MIN<int32>(screenWidth, screenHeight * 4 / 3);
		int32 viewportHeight = MIN<int32>(screenHeight, screenWidth * 3 / 4);
		_screenViewport = Common::Rect(viewportWidth, viewportHeight);

		// Pillarboxing/Letterboxing
		_screenViewport.translate((screenWidth - viewportWidth) / 2,
		                           (screenHeight - viewportHeight) / 2);
	} else {
		_screenViewport = Common::Rect(screenWidth, screenHeight);
	}

	glViewport(_screenViewport.left, screenHeight - _screenViewport.bottom, _screenViewport.width(), _screenViewport.height());
	glScissor(_screenViewport.left, screenHeight - _screenViewport.bottom, _screenViewport.width(), _screenViewport.height());
}

void OpenGLRenderer::scroll(int dx, int dy, uint32 background) {
}

void OpenGLRenderer::drawEllipse(int x, int y, int rx, int ry, uint32 color) {
	useColor(color);
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 360; i += 10) {
		float rad = i * M_PI / 180.0f;
		glVertex2f(x + cos(rad) * (float)rx, y + sin(rad) * (float)ry);
	}
	glEnd();
}

void OpenGLRenderer::fillEllipse(int x, int y, int rx, int ry, uint32 color) {
	useColor(color);
	glBegin(GL_POLYGON);
	for (int i = 0; i < 360; i += 10) {
		float rad = i * M_PI / 180.0f;
		glVertex2f(x + cos(rad) * (float)rx, y + sin(rad) * (float)ry);
	}
	glEnd();
}

void OpenGLRenderer::fillDitherRect(const Common::Rect &rect, uint32 color1, uint32 color2) {
	fillRect(rect, color1);
	useColor(color2);
	glBegin(GL_POINTS);
	for (int y = rect.top; y < rect.bottom; y++) {
		for (int x = rect.left + (y % 2); x < rect.right; x += 2) {
			glVertex2i(x, y);
		}
	}
	glEnd();
}

void OpenGLRenderer::setPixel(int x, int y, uint32 color) {
	useColor(color);
	glBegin(GL_POINTS);
	glVertex2i(x, y);
	glEnd();
}

void OpenGLRenderer::drawQuad(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, uint32 color) {
	useColor(color);
	glBegin(GL_QUADS);
	glVertex2i(x1, y1);
	glVertex2i(x2, y2);
	glVertex2i(x3, y3);
	glVertex2i(x4, y4);
	glEnd();
	
	glColor3ub(255, 255, 255);
	glBegin(GL_LINE_LOOP);
	glVertex2i(x1, y1);
	glVertex2i(x2, y2);
	glVertex2i(x3, y3);
	glVertex2i(x4, y4);
	glEnd();
}

void OpenGLRenderer::drawPolygon(const int *x, const int *y, int count, uint32 color) {
	if (count < 3) return;
	useColor(color);
	glBegin(GL_POLYGON);
	for (int i = 0; i < count; i++) {
		glVertex2i(x[i], y[i]);
	}
	glEnd();
	
	glColor3ub(255, 255, 255);
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < count; i++) {
		glVertex2i(x[i], y[i]);
	}
	glEnd();
}

void OpenGLRenderer::copyToScreen() {
	glFlush();
	_system->updateScreen();
}

Renderer *createOpenGLRenderer(OSystem *system, int width, int height) {
	return new OpenGLRenderer(system, width, height);
}

} // End of namespace Colony

#else

namespace Colony {
Renderer *createOpenGLRenderer(OSystem *system, int width, int height) { return nullptr; }
}

#endif
