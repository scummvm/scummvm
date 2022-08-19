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

// Based on Phantasma code by Thomas Harte (2013)

#include "common/config-manager.h"
#include "common/rect.h"
#include "math/glmath.h"
#include "graphics/tinygl/tinygl.h"

#include "engines/freescape/gfx_tinygl.h"
#include "engines/freescape/gfx_tinygl_texture.h"

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
}

void TinyGLRenderer::setViewport(const Common::Rect &rect) {
	tglViewport(rect.left, g_system->getHeight() - rect.bottom, rect.width(), rect.height());
}

void TinyGLRenderer::clear() {
	tglClear(TGL_COLOR_BUFFER_BIT | TGL_DEPTH_BUFFER_BIT);
	tglColor3f(1.0f, 1.0f, 1.0f);
}

void TinyGLRenderer::drawTexturedRect2D(const Common::Rect &screenRect, const Common::Rect &textureRect, Texture *texture) {
	const float sLeft = screenRect.left;
	const float sTop = screenRect.top;
	const float sWidth = screenRect.width();
	const float sHeight = screenRect.height();

	// HACK: tglBlit is not affected by the viewport, so we offset the draw coordinates here
	int viewPort[4];
	tglGetIntegerv(TGL_VIEWPORT, viewPort);

	TinyGL::BlitTransform transform(sLeft + viewPort[0], sTop + viewPort[1]);
	transform.sourceRectangle(textureRect.left, textureRect.top, sWidth, sHeight);
	tglBlit(((TinyGLTexture *)texture)->getBlitTexture(), transform);
}

void TinyGLRenderer::updateProjectionMatrix(float fov, float nearClipPlane, float farClipPlane) {
	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();

	float aspectRatio = kOriginalWidth / (float) kFrameHeight;

	float xmaxValue = nearClipPlane * tan(fov * M_PI / 360.0);
	float ymaxValue = xmaxValue / aspectRatio;
	//debug("max values: %f %f", xmaxValue, ymaxValue);

	tglFrustum(xmaxValue, -xmaxValue, -ymaxValue, ymaxValue, nearClipPlane, farClipPlane);
	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();
}

void TinyGLRenderer::positionCamera(const Math::Vector3d &pos, const Math::Vector3d &interest) {
	Math::Vector3d up_vec(0, 1, 0);

	Math::Matrix4 lookMatrix = Math::makeLookAtMatrix(pos, interest, up_vec);
	tglMultMatrixf(lookMatrix.getData());
	tglTranslatef(-pos.x(), -pos.y(), -pos.z());
}

void TinyGLRenderer::drawRect2D(const Common::Rect &rect, uint8 a, uint8 r, uint8 g, uint8 b) {
	tglDisable(TGL_TEXTURE_2D);
	tglColor4ub(r, g, b, a);

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

void TinyGLRenderer::renderCrossair(byte color) {
	uint8 r, g, b;
	_palette->getRGBAt(color, r, g, b);

	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();
	tglOrtho(0, kOriginalWidth, kOriginalHeight, 0, 0, 1);
	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();

	tglDisable(TGL_DEPTH_TEST);
	tglDepthMask(TGL_FALSE);

	tglColor3ub(r, g, b);

	tglBegin(TGL_LINES);
	tglVertex2f(kOriginalWidth / 2 - 2, kOriginalHeight / 2);
	tglVertex2f(kOriginalWidth / 2 + 3, kOriginalHeight / 2);

	tglVertex2f(kOriginalWidth / 2, kOriginalHeight / 2 - 2);
	tglVertex2f(kOriginalWidth / 2, kOriginalHeight / 2 + 3);
	tglEnd();

	tglDepthMask(TGL_TRUE);
	tglEnable(TGL_DEPTH_TEST);
}

void TinyGLRenderer::renderShoot(byte color) {
	uint8 r, g, b;
	_palette->getRGBAt(color, r, g, b);

	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();
	tglOrtho(0, kOriginalWidth, kOriginalHeight, 0, 0, 1);
	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();

	tglDisable(TGL_DEPTH_TEST);
	tglDepthMask(TGL_FALSE);

	tglColor3ub(r, g, b);

	tglBegin(TGL_LINES);
	tglVertex2f(0, kOriginalHeight - 2);
	tglVertex2f(kOriginalWidth / 2, kOriginalHeight / 2);

	tglVertex2f(0, kOriginalHeight);
	tglVertex2f(kOriginalWidth / 2, kOriginalHeight / 2);

	tglVertex2f(kOriginalWidth, kOriginalHeight - 2);
	tglVertex2f(kOriginalWidth / 2, kOriginalHeight / 2);

	tglVertex2f(kOriginalWidth, kOriginalHeight);
	tglVertex2f(kOriginalWidth / 2, kOriginalHeight / 2);

	tglEnd();

	tglEnable(TGL_DEPTH_TEST);
	tglDepthMask(TGL_TRUE);
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
	for (int i = 1; i < int(vertices.size()) - 1; i++) {
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
	if (ordinates->size() % 3 > 0 && ordinates->size() > 0)
		error("Invalid polygon with size %f %f %f and ordinates %d", size.x(), size.y(), size.z(), ordinates->size());

	Common::Array<Math::Vector3d> vertices;
	tglEnable(TGL_POLYGON_OFFSET_FILL);
	tglPolygonOffset(-2.0f, 1.f);

	if (ordinates->size() == 6) { // Line
		_palette->getRGBAt((*colours)[0], r, g, b);
		tglColor3ub(r, g, b);
		for (int i = 0; i < int(ordinates->size()); i = i + 3)
			vertices.push_back(Math::Vector3d((*ordinates)[i], (*ordinates)[i + 1],	(*ordinates)[i + 2]));
		renderFace(vertices);

		vertices.clear();
		_palette->getRGBAt((*colours)[1], r, g, b);
		tglColor3ub(r, g, b);
		for (int i = ordinates->size(); i > 0; i = i - 3)
			vertices.push_back(Math::Vector3d((*ordinates)[i-3], (*ordinates)[i-2],	(*ordinates)[i-1]));
		renderFace(vertices);

	} else {
		if ((*colours)[0] != _keyColor) {
			_palette->getRGBAt((*colours)[0], r, g, b);
			tglColor3ub(r, g, b);
			for (int i = 0; i < int(ordinates->size()); i = i + 3) {
				vertices.push_back(Math::Vector3d((*ordinates)[i], (*ordinates)[i + 1], (*ordinates)[i + 2]));
			}
			renderFace(vertices);
		}
		vertices.clear();
		if ((*colours)[1] != _keyColor) {
			_palette->getRGBAt((*colours)[1], r, g, b);
			tglColor3ub(r, g, b);
			for (int i = ordinates->size(); i > 0; i = i - 3) {
				vertices.push_back(Math::Vector3d((*ordinates)[i-3], (*ordinates)[i-2], (*ordinates)[i-1]));
			}
			renderFace(vertices);
		}
	}

	tglPolygonOffset(0, 0);
	tglDisable(TGL_POLYGON_OFFSET_FILL);
}

void TinyGLRenderer::renderRectangle(const Math::Vector3d &origin, const Math::Vector3d &size, Common::Array<uint8> *colours) {

	assert(size.x() == 0 || size.y() == 0 || size.z() == 0);
	tglEnable(TGL_POLYGON_OFFSET_FILL);
	tglPolygonOffset(-2.0f, 1.0f);

	float dx, dy, dz;
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
		}
	}

	tglPolygonOffset(0, 0);
	tglDisable(TGL_POLYGON_OFFSET_FILL);
}

void TinyGLRenderer::renderPyramid(const Math::Vector3d &origin, const Math::Vector3d &size, const Common::Array<uint16> *ordinates, Common::Array<uint8> *colours, int type) {
	Math::Vector3d vertices[8] = {
		origin,		origin,		origin,		origin,
		origin,		origin,		origin,		origin,
	};
	PyramidType pyramidType = (PyramidType)type;
	switch(pyramidType) {
		default: break;
		case EastPyramid:
			vertices[0] += Math::Vector3d(0, 0, size.z());
			vertices[1] += Math::Vector3d(0, size.y(), size.z());
			vertices[2] += Math::Vector3d(0, size.y(), 0);

			vertices[4] += Math::Vector3d(size.x(), (*ordinates)[0], (*ordinates)[3]);
			vertices[5] += Math::Vector3d(size.x(), (*ordinates)[2], (*ordinates)[3]);
			vertices[6] += Math::Vector3d(size.x(), (*ordinates)[2], (*ordinates)[1]);
			vertices[7] += Math::Vector3d(size.x(), (*ordinates)[0], (*ordinates)[1]);
			break;
		case WestPyramid:

			vertices[0] += Math::Vector3d(size.x(), 0, 0);
			vertices[1] += Math::Vector3d(size.x(), size.y(), 0);
			vertices[2] += Math::Vector3d(size.x(), size.y(), size.z());
			vertices[3] += Math::Vector3d(size.x(), 0, size.z());

			vertices[4] += Math::Vector3d(0, (*ordinates)[0], (*ordinates)[1]);
			vertices[5] += Math::Vector3d(0, (*ordinates)[2], (*ordinates)[1]);
			vertices[6] += Math::Vector3d(0, (*ordinates)[2], (*ordinates)[3]);
			vertices[7] += Math::Vector3d(0, (*ordinates)[0], (*ordinates)[3]);
			break;

		case UpPyramid:
			vertices[1] += Math::Vector3d(size.x(), 0, 0);
			vertices[2] += Math::Vector3d(size.x(), 0, size.z());
			vertices[3] += Math::Vector3d(0, 0, size.z());

			vertices[4] += Math::Vector3d((*ordinates)[0], size.y(), (*ordinates)[1]);
			vertices[5] += Math::Vector3d((*ordinates)[2], size.y(), (*ordinates)[1]);
			vertices[6] += Math::Vector3d((*ordinates)[2], size.y(), (*ordinates)[3]);
			vertices[7] += Math::Vector3d((*ordinates)[0], size.y(), (*ordinates)[3]);
			break;

		case DownPyramid:

			vertices[0] += Math::Vector3d(size.x(), size.y(), 0);
			vertices[1] += Math::Vector3d(0, size.y(), 0);
			vertices[2] += Math::Vector3d(0, size.y(), size.z());
			vertices[3] += Math::Vector3d(size.x(), size.y(), size.z());

			vertices[4] += Math::Vector3d((*ordinates)[2], 0, (*ordinates)[1]);
			vertices[5] += Math::Vector3d((*ordinates)[0], 0, (*ordinates)[1]);
			vertices[6] += Math::Vector3d((*ordinates)[0], 0, (*ordinates)[3]);
			vertices[7] += Math::Vector3d((*ordinates)[2], 0, (*ordinates)[3]);
			break;

		case NorthPyramid:
			vertices[0] += Math::Vector3d(0, size.y(), 0);
			vertices[1] += Math::Vector3d(size.x(), size.y(), 0);
			vertices[2] += Math::Vector3d(size.x(), 0, 0);

			vertices[4] += Math::Vector3d((*ordinates)[0], (*ordinates)[3], size.z());
			vertices[5] += Math::Vector3d((*ordinates)[2], (*ordinates)[3], size.z());
			vertices[6] += Math::Vector3d((*ordinates)[2], (*ordinates)[1], size.z());
			vertices[7] += Math::Vector3d((*ordinates)[0], (*ordinates)[1], size.z());
			break;
		case SouthPyramid:
			vertices[0] += Math::Vector3d(0, 0, size.z());
			vertices[1] += Math::Vector3d(size.x(), 0, size.z());
			vertices[2] += Math::Vector3d(size.x(), size.y(), size.z());

			vertices[3] += Math::Vector3d(0, size.y(), size.z());
			vertices[4] += Math::Vector3d((*ordinates)[0], (*ordinates)[1], 0);
			vertices[5] += Math::Vector3d((*ordinates)[2], (*ordinates)[1], 0);
			vertices[6] += Math::Vector3d((*ordinates)[2], (*ordinates)[3], 0);
			vertices[7] += Math::Vector3d((*ordinates)[0], (*ordinates)[3], 0);
			break;
	}

	Common::Array<Math::Vector3d> face;
	uint8 r, g, b;
	if ((*colours)[0] != _keyColor) {
		_palette->getRGBAt((*colours)[0], r, g, b);
		tglColor3ub(r, g, b);
		face.push_back(vertices[6]);
		face.push_back(vertices[7]);
		face.push_back(vertices[3]);
		face.push_back(vertices[2]);

		renderFace(face);
		face.clear();
	}

	if ((*colours)[1] != _keyColor) {
		_palette->getRGBAt((*colours)[1], r, g, b);
		tglColor3ub(r, g, b);

		face.push_back(vertices[7]);
		face.push_back(vertices[4]);
		face.push_back(vertices[0]);
		face.push_back(vertices[3]);
		renderFace(face);
		face.clear();
	}

	if ((*colours)[2] != _keyColor) {
		_palette->getRGBAt((*colours)[2], r, g, b);
		tglColor3ub(r, g, b);

		face.push_back(vertices[4]);
		face.push_back(vertices[5]);
		face.push_back(vertices[1]);
		face.push_back(vertices[0]);
		renderFace(face);
		face.clear();
	}

	if ((*colours)[3] != _keyColor) {
		_palette->getRGBAt((*colours)[3], r, g, b);
		tglColor3ub(r, g, b);

		face.push_back(vertices[5]);
		face.push_back(vertices[6]);
		face.push_back(vertices[2]);
		face.push_back(vertices[1]);
		renderFace(face);
		face.clear();
	}

	if ((*colours)[4] != _keyColor) {
		_palette->getRGBAt((*colours)[4], r, g, b);
		tglColor3ub(r, g, b);

		face.push_back(vertices[0]);
		face.push_back(vertices[1]);
		face.push_back(vertices[2]);
		face.push_back(vertices[3]);
		renderFace(face);
		face.clear();
	}

	if ((*colours)[5] != _keyColor) {
		_palette->getRGBAt((*colours)[5], r, g, b);
		tglColor3ub(r, g, b);

		face.push_back(vertices[7]);
		face.push_back(vertices[6]);
		face.push_back(vertices[5]);
		face.push_back(vertices[4]);
		renderFace(face);
	}
}

void TinyGLRenderer::renderCube(const Math::Vector3d &origin, const Math::Vector3d &size, Common::Array<uint8> *colours) {
	/*assert(size.x() > 0);
	assert(size.y() > 0);
	assert(size.z() > 0);*/

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

} // End of namespace Freescape
