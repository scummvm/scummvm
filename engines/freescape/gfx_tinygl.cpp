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

#include "common/config-manager.h"
#include "common/rect.h"
#include "common/textconsole.h"

#include "graphics/colormasks.h"
#include "graphics/surface.h"

#include "math/vector2d.h"
#include "math/glmath.h"

#include "engines/freescape/gfx.h"
#include "engines/freescape/gfx_tinygl.h"
#include "engines/freescape/gfx_tinygl_texture.h"
#include "graphics/tinygl/tinygl.h"

namespace Freescape {

Renderer *CreateGfxTinyGL(OSystem *system) {
	return new TinyGLRenderer(system);
}

TinyGLRenderer::TinyGLRenderer(OSystem *system) : Renderer(system) {
}

TinyGLRenderer::~TinyGLRenderer() {
	TinyGL::destroyContext();
}

Texture *TinyGLRenderer::createTexture(const Graphics::Surface *surface) {
	return new TinyGLTexture(surface);
}

void TinyGLRenderer::freeTexture(Texture *texture) {
	TinyGLTexture *glTexture = static_cast<TinyGLTexture *>(texture);
	delete glTexture;
}

void TinyGLRenderer::init() {
	debug("Initializing Software 3D Renderer");

	computeScreenViewport();

	TinyGL::createContext(kOriginalWidth, kOriginalHeight, g_system->getScreenFormat(), 512, true, ConfMan.getBool("dirtyrects"));

	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();

	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();

	tglDisable(TGL_LIGHTING);
	tglDisable(TGL_TEXTURE_2D);
	tglEnable(TGL_DEPTH_TEST);

	//tglColorMask(TGL_FALSE, TGL_FALSE, TGL_FALSE, TGL_FALSE);
	//tglDisable(TGL_POLYGON_OFFSET_FILL);

}

void TinyGLRenderer::clear() {
	tglClear(TGL_COLOR_BUFFER_BIT | TGL_DEPTH_BUFFER_BIT);
	tglColor3f(1.0f, 1.0f, 1.0f);
}

void TinyGLRenderer::drawRect2D(const Common::Rect &rect, uint8 a, uint8 r, uint8 g, uint8 b) {
	tglDisable(TGL_TEXTURE_2D);
	tglColor3ub(r, g, b);

	if (a != 255) {
		tglEnable(TGL_BLEND);
		tglBlendFunc(TGL_SRC_ALPHA, TGL_ONE_MINUS_SRC_ALPHA);
	}

	tglBegin(TGL_TRIANGLE_STRIP);
		tglVertex3f(rect.left, rect.bottom, 0.0f);
		tglVertex3f(rect.right, rect.bottom, 0.0f);
		tglVertex3f(rect.left, rect.top, 0.0f);
		tglVertex3f(rect.right, rect.top, 0.0f);
	tglEnd();

	tglDisable(TGL_BLEND);
}

void TinyGLRenderer::drawTexturedRect2D(const Common::Rect &screenRect, const Common::Rect &textureRect,
		Texture *texture, float transparency, bool additiveBlending) {
	const float sLeft = screenRect.left;
	const float sTop = screenRect.top;
	const float sWidth = screenRect.width();
	const float sHeight = screenRect.height();

	if (transparency >= 0.0) {
		if (additiveBlending) {
			tglBlendFunc(TGL_SRC_ALPHA, TGL_ONE);
		} else {
			tglBlendFunc(TGL_SRC_ALPHA, TGL_ONE_MINUS_SRC_ALPHA);
		}
		tglEnable(TGL_BLEND);
	} else {
		transparency = 1.0;
	}

	// HACK: tglBlit is not affected by the viewport, so we offset the draw coordinates here
	int viewPort[4];
	tglGetIntegerv(TGL_VIEWPORT, viewPort);

	tglEnable(TGL_TEXTURE_2D);
	tglDepthMask(TGL_FALSE);

	TinyGL::BlitTransform transform(sLeft + viewPort[0], sTop + viewPort[1]);
	transform.sourceRectangle(textureRect.left, textureRect.top, sWidth, sHeight);
	transform.tint(transparency);
	tglBlit(((TinyGLTexture *)texture)->getBlitTexture(), transform);

	tglDisable(TGL_BLEND);
	tglDepthMask(TGL_TRUE);
}

void TinyGLRenderer::draw2DText(const Common::String &text, const Common::Point &position) {
	TinyGLTexture *glFont = static_cast<TinyGLTexture *>(_font);

	// The font only has uppercase letters
	Common::String textToDraw = text;
	textToDraw.toUppercase();

	tglEnable(TGL_BLEND);
	tglBlendFunc(TGL_SRC_ALPHA, TGL_ONE_MINUS_SRC_ALPHA);

	tglEnable(TGL_TEXTURE_2D);
	tglDepthMask(TGL_FALSE);

	tglColor3f(1.0f, 1.0f, 1.0f);
	tglBindTexture(TGL_TEXTURE_2D, glFont->id);

	int x = position.x;
	int y = position.y;

	for (uint i = 0; i < textToDraw.size(); i++) {
		Common::Rect textureRect = getFontCharacterRect(textToDraw[i]);
		int w = textureRect.width();
		int h = textureRect.height();

		TinyGL::BlitTransform transform(x, y);
		transform.sourceRectangle(textureRect.left, textureRect.top, w, h);
		transform.flip(true, false);
		//TinyGL::tglBlit(glFont->getBlitTexture(), transform);

		x += textureRect.width() - 3;
	}

	tglDisable(TGL_TEXTURE_2D);
	tglDisable(TGL_BLEND);
	tglDepthMask(TGL_TRUE);
}

void TinyGLRenderer::scale(const Math::Vector3d &scale) {
	tglScalef(-1, 1, 1);
}

void TinyGLRenderer::updateProjectionMatrix(float fov, float nearClipPlane, float farClipPlane) {
	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();

	float aspectRatio = kOriginalWidth / (float) kFrameHeight;

	float xmaxValue = nearClipPlane * tan(fov * M_PI / 360.0);
	float ymaxValue = xmaxValue / aspectRatio;
	//debug("max values: %f %f", xmaxValue, ymaxValue);

	tglFrustum(-xmaxValue, xmaxValue, -ymaxValue, ymaxValue, nearClipPlane, farClipPlane);
	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();
}

void TinyGLRenderer::positionCamera(const Math::Vector3d &pos, const Math::Vector3d &interest) {
	Math::Vector3d up_vec(0, 1, 0);

	Math::Matrix4 lookMatrix = Math::makeLookAtMatrix(pos, interest, up_vec);
	tglMultMatrixf(lookMatrix.getData());
	tglTranslatef(-pos.x(), -pos.y(), -pos.z());
}


void TinyGLRenderer::renderFace(const Common::Array<Math::Vector3d> &vertices) {
	assert(vertices.size() >= 2);
	const Math::Vector3d &v0 = vertices[0];

	if (vertices.size() == 2) {
		const Math::Vector3d &v1 = vertices[1];
		if (v0 == v1)
			return;
		tglBegin(TGL_LINES);
		tglVertex3f(v0.x(), v0.y(),	v0.z());
		tglVertex3f(v1.x(), v1.y(),	v1.z());
		tglEnd();
		return;
	}

	tglBegin(TGL_TRIANGLES);
	for (int i = 1; i < vertices.size() - 1; i++) {
		const Math::Vector3d &v1 = vertices[i];
		const Math::Vector3d &v2 = vertices[i+1];
		tglVertex3f(v0.x(), v0.y(),	v0.z());
		tglVertex3f(v1.x(), v1.y(),	v1.z());
		tglVertex3f(v2.x(), v2.y(),	v2.z());
	}
	tglEnd();
}

void TinyGLRenderer::renderPolygon(const Math::Vector3d &origin, const Math::Vector3d &size, const Common::Array<uint16> *ordinates, Common::Array<uint8> *colours) {
	//assert(size.x() == 0 || size.y() == 0 || size.z() == 0);
	uint8 r, g, b;
	float dx, dy, dz;
	dx = dy = dz = 0;

	if (size.x() == 0)
		dx = 2;
	else if (size.y() == 0)
		dy = 2;
	else if (size.z() == 0)
		dz = 2;
	else {
		if (ordinates->size() != 6)
			error("Invalid polygon: %f %f %f", size.x(), size.y(), size.z());
	}

	Common::Array<Math::Vector3d> vertices;
	tglEnable(TGL_POLYGON_OFFSET_FILL);
	// tglEnable(TGL_POLYGON_OFFSET_LINE);
	// tglEnable(TGL_POLYGON_OFFSET_POINT);
	tglPolygonOffset(1.f, 1.f);
	if ((*colours)[0] != _keyColor) {
		_palette->getRGBAt((*colours)[0], r, g, b);
		tglColor3ub(r, g, b);
		for (int i = 0; i < ordinates->size(); i = i + 3) {
			vertices.push_back(Math::Vector3d(origin.x() + (*ordinates)[i] + dx, origin.y() + (*ordinates)[i + 1] + dy,	origin.z() + (*ordinates)[i + 2] + dz));
		}
		renderFace(vertices);
	}
	vertices.clear();
	if ((*colours)[1] != _keyColor) {
		_palette->getRGBAt((*colours)[1], r, g, b);
		tglColor3ub(r, g, b);
		for (int i = ordinates->size(); i > 0; i = i - 3) {
			vertices.push_back(Math::Vector3d(origin.x() + (*ordinates)[i-3] - dx, origin.y() + (*ordinates)[i-2] - dy,	origin.z() + (*ordinates)[i-1] - dz));
		}
		renderFace(vertices);
	}
	tglPolygonOffset(0, 0);
	tglDisable(TGL_POLYGON_OFFSET_FILL);
}

void TinyGLRenderer::renderRectangle(const Math::Vector3d &origin, const Math::Vector3d &size, Common::Array<uint8> *colours) {

	assert(size.x() == 0 || size.y() == 0 || size.z() == 0);

	tglPolygonOffset(250, 250);
	//debug("origin: %f, %f, %f", origin.x(), origin.y(), origin.z());
	//debug("size: %f, %f, %f", size.x(), size.y(), size.z());

	float dx, dy, dz;
	float offset = 0;
	uint8 r, g, b;
	Common::Array<Math::Vector3d> vertices;
	for (int i = 0; i < 2; i++) {

		//debug("rec color: %d", (*colours)[i]);
		if ((*colours)[i] != _keyColor) {
			_palette->getRGBAt((*colours)[i], r, g, b);
			tglColor3ub(r, g, b);
			vertices.clear();
			vertices.push_back(Math::Vector3d(origin.x(), origin.y(), origin.z()));

			dx = dy = dz = 0;
			if (size.x() == 0) {
				dy = size.y();
			} else if (size.y() == 0) {
				dx = size.x();
			} else if (size.z() == 0) {
				dx = size.x();
			}

			vertices.push_back(Math::Vector3d(origin.x() + dx,	origin.y() + dy, origin.z() + dz));
			vertices.push_back(Math::Vector3d(origin.x() + size.x(), origin.y() + size.y(), origin.z() + size.z()));
			renderFace(vertices);

			vertices.clear();
			vertices.push_back(Math::Vector3d(origin.x(), origin.y(), origin.z()));

			dx = dy = dz = 0;
			if (size.x() == 0) {
				dz = size.z();
			} else if (size.y() == 0) {
				dz = size.z();
			} else if (size.z() == 0) {
				dy = size.y();
			}

			vertices.push_back(Math::Vector3d(origin.x() + dx,	origin.y() + dy, origin.z() + dz));
			vertices.push_back(Math::Vector3d(origin.x() + size.x(), origin.y() + size.y(), origin.z() + size.z()));
			renderFace(vertices);

			// tglBegin(TGL_TRIANGLES);
			// tglVertex3f(origin.x(),	origin.y(),	origin.z());
			// if (size.x() == 0)
			// 	tglVertex3f(origin.x(),	origin.y() + size.y(), origin.z());
			// else if (size.y() == 0)
			// 	tglVertex3f(origin.x() + size.x(),	origin.y(), origin.z());
			// else
			// 	tglVertex3f(origin.x() + size.x(),	origin.y(), origin.z());
			// tglVertex3f(origin.x() + size.x(), origin.y() + size.y(), origin.z() + size.z());

			// tglVertex3f(origin.x(),	origin.y(),	origin.z());
			// if (size.x() == 0)
			// 	tglVertex3f(origin.x(),	origin.y(), origin.z()  + size.z());
			// else if (size.y() == 0)
			// 	tglVertex3f(origin.x() + size.x(),	origin.y(), origin.z() + size.y());
			// else
			// 	tglVertex3f(origin.x(),	origin.y() + size.y(), origin.z());
			// tglVertex3f(origin.x() + size.x(), origin.y() + size.y(), origin.z() + size.z());
			// tglEnd();
		}
	}

	tglPolygonOffset(0, 0);
	//tglDepthMask(TGL_TRUE);
	//tglColorMask(TGL_FALSE, TGL_FALSE, TGL_FALSE, TGL_FALSE);
	//tglDisable(TGL_POLYGON_OFFSET_FILL);
}


void TinyGLRenderer::renderCube(const Math::Vector3d &origin, const Math::Vector3d &size, Common::Array<uint8> *colours) {
	assert(size.x() > 0);
	assert(size.y() > 0);
	assert(size.z() > 0);

	//debug("Rendering cube at %f, %f, %f", origin.x(), origin.y(), origin.z());
	//debug("with size %f, %f, %f", size.x(), size.y(), size.z());
	uint8 r, g, b;

	// Face 0
	if ((*colours)[0] != _keyColor) {
		_palette->getRGBAt((*colours)[0], r, g, b);
		tglColor3ub(r, g, b);
		tglBegin(TGL_TRIANGLES);
		tglVertex3f(origin.x(),	origin.y(),				origin.z());
		tglVertex3f(origin.x(),	origin.y(),				origin.z() + size.z());
		tglVertex3f(origin.x(),	origin.y() + size.y(),	origin.z() + size.z());

		tglVertex3f(origin.x(),	origin.y(),				origin.z());
		tglVertex3f(origin.x(),	origin.y() + size.y(),	origin.z() + size.z());
		tglVertex3f(origin.x(),	origin.y() + size.y(),	origin.z());
		tglEnd();
	}

	// Face 1
	if ((*colours)[1] != _keyColor) {
		_palette->getRGBAt((*colours)[1], r, g, b);
		tglColor3ub(r, g, b);
		tglBegin(TGL_TRIANGLES);
		tglVertex3f(origin.x() + size.x(),	origin.y() + size.y(),	origin.z());
		tglVertex3f(origin.x() + size.x(),	origin.y() + size.y(),	origin.z() + size.z());
		tglVertex3f(origin.x() + size.x(),	origin.y(),				origin.z() + size.z());

		tglVertex3f(origin.x() + size.x(),	origin.y() + size.y(),	origin.z());
		tglVertex3f(origin.x() + size.x(),	origin.y(),				origin.z() + size.z());
		tglVertex3f(origin.x() + size.x(),	origin.y(),				origin.z());
		tglEnd();
	}

	// Face 2
	if ((*colours)[2] != _keyColor) {
		_palette->getRGBAt((*colours)[2], r, g, b);
		tglColor3ub(r, g, b);
		tglBegin(TGL_TRIANGLES);
		tglVertex3f(origin.x() + size.x(),	origin.y(),	origin.z());
		tglVertex3f(origin.x() + size.x(),	origin.y(),	origin.z() + size.z());
		tglVertex3f(origin.x(),			origin.y(),		origin.z() + size.z());

		tglVertex3f(origin.x() + size.x(),	origin.y(),	origin.z());
		tglVertex3f(origin.x(),			origin.y(),		origin.z() + size.z());
		tglVertex3f(origin.x(),			origin.y(),		origin.z());
		tglEnd();
	}

	// Face 3
	if ((*colours)[3] != _keyColor) {
		_palette->getRGBAt((*colours)[3], r, g, b);
		tglColor3ub(r, g, b);
		tglBegin(TGL_TRIANGLES);
		tglVertex3f(origin.x(),				origin.y() + size.y(),	origin.z());
		tglVertex3f(origin.x(),				origin.y() + size.y(),	origin.z() + size.z());
		tglVertex3f(origin.x() + size.x(),	origin.y() + size.y(),	origin.z() + size.z());

		tglVertex3f(origin.x(),				origin.y() + size.y(),	origin.z());
		tglVertex3f(origin.x() + size.x(),	origin.y() + size.y(),	origin.z() + size.z());
		tglVertex3f(origin.x() + size.x(),	origin.y() + size.y(),	origin.z());
		tglEnd();
	}

	// Face 1
	if ((*colours)[4] != _keyColor) {
		_palette->getRGBAt((*colours)[4], r, g, b);
		tglColor3ub(r, g, b);
		tglBegin(TGL_TRIANGLES);
		tglVertex3f(origin.x(),				origin.y() + size.y(),	origin.z());
		tglVertex3f(origin.x() + size.x(),	origin.y() + size.y(),	origin.z());
		tglVertex3f(origin.x() + size.x(),	origin.y(),				origin.z());

		tglVertex3f(origin.x(),				origin.y() + size.y(),	origin.z());
		tglVertex3f(origin.x() + size.x(),	origin.y(),				origin.z());
		tglVertex3f(origin.x(),				origin.y(),				origin.z());
		tglEnd();
	}

	// Face 0
	if ((*colours)[5] != _keyColor) {
		_palette->getRGBAt((*colours)[5], r, g, b);
		tglColor3ub(r, g, b);
		tglBegin(TGL_TRIANGLES);
		tglVertex3f(origin.x(),		        origin.y(),				origin.z() + size.z());
		tglVertex3f(origin.x() + size.x(),	origin.y(),				origin.z() + size.z());
		tglVertex3f(origin.x() + size.x(),	origin.y() + size.y(),	origin.z() + size.z());

		tglVertex3f(origin.x(),		        origin.y(),				origin.z() + size.z());
		tglVertex3f(origin.x() + size.x(),	origin.y() + size.y(),	origin.z() + size.z());
		tglVertex3f(origin.x(),		        origin.y() + size.y(),	origin.z() + size.z());
		tglEnd();
	}
}

void TinyGLRenderer::drawSky(uint8 color) {
	uint8 r, g, b;
	_palette->getRGBAt(color, r, g, b);
	tglClearColor(r / 255., g / 255., b / 255., 1.0);
	tglClear(TGL_COLOR_BUFFER_BIT | TGL_DEPTH_BUFFER_BIT);
}

void TinyGLRenderer::drawFloor(uint8 color) {
	uint8 r, g, b;
	_palette->getRGBAt(color, r, g, b);
	tglColor3ub(r, g, b);
	tglBegin(TGL_QUADS);
	tglVertex3f(-100000.f, 0.f, -100000.f);
	tglVertex3f(100000.f, 0.f, -100000.f);
	tglVertex3f(100000.f, 0.f, 100000.f);
	tglVertex3f(-100000.f, 0.f, 100000.f);
	tglEnd();
}

void TinyGLRenderer::flipBuffer() {
	Common::List<Common::Rect> dirtyAreas;
	TinyGL::presentBuffer(dirtyAreas);

	Graphics::Surface glBuffer;
	TinyGL::getSurfaceRef(glBuffer);

	if (!dirtyAreas.empty()) {
		for (Common::List<Common::Rect>::iterator itRect = dirtyAreas.begin(); itRect != dirtyAreas.end(); ++itRect) {
			g_system->copyRectToScreen(glBuffer.getBasePtr((*itRect).left, (*itRect).top), glBuffer.pitch,
			                           (*itRect).left, (*itRect).top, (*itRect).width(), (*itRect).height());
		}
	}
}

} // End of namespace Myst3
