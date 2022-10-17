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

#include "graphics/opengl/system_headers.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/te/te_3d_texture.h"
#include "tetraedge/te/te_resource_manager.h"
#include "tetraedge/te/te_renderer.h"

namespace Tetraedge {

static const uint NO_TEXTURE = 0xffffffff;

Te3DTexture::Te3DTexture() : _glTexture(NO_TEXTURE), _createdTexture(false),
_numFrames(1), _frameRate(0), _format(TeImage::INVALID), _glPixelFormat(GL_INVALID_ENUM) {
	create();
}

Te3DTexture::~Te3DTexture() {
	destroy();
}

void Te3DTexture::bind() {
	TeRenderer *renderer = g_engine->getRenderer();
	glBindTexture(GL_TEXTURE_2D, _glTexture);
	renderer->setMatrixMode(TeRenderer::MM_GL_TEXTURE);
	renderer->loadMatrix(_matrix);
	renderer->loadCurrentMatrixToGL();
	renderer->setMatrixMode(TeRenderer::MM_GL_MODELVIEW);
}

void Te3DTexture::copyCurrentRender(uint xoffset, uint yoffset, uint x, uint y) {
	// TODO: Get some better variable names here.
	_matrix.setToIdentity();
	const TeVector3f32 local_40((float)_width / _texWidth, (float)_height / _texHeight, 1.0);
	_matrix.scale(local_40);
	const TeVector3f32 local_50((float)_leftBorder / _width, (float)_btmBorder / _height, 0.0);
	_matrix.translate(local_50);
	const TeVector3f32 local_60(
			   1.0 - (float)(_rightBorder + _leftBorder) /
					 (float)_width,
			   1.0 - (float)(_topBorder + _btmBorder) /
					 (float)_height, 1.0);
	_matrix.scale(local_60);
	bind();
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, x, y, _texWidth, _texHeight);
}

void Te3DTexture::create() {
	_flipY = true;
	_leftBorder = _btmBorder = _texWidth = _texHeight = 0;
	_rightBorder = _topBorder = _width = _height = 0;
	_format = TeImage::INVALID;
	_loaded = 0;
	if (!_createdTexture)
		glGenTextures(1, &_glTexture);
	if (_glTexture == NO_TEXTURE) {
		_createdTexture = false;
		return;
	}

	_createdTexture = true;
	glBindTexture(GL_TEXTURE_2D, _glTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void Te3DTexture::destroy() {
	if (_createdTexture) {
	  glDeleteTextures(1, &_glTexture);
	}
	_createdTexture = false;
	_loaded = false;
	_glTexture = NO_TEXTURE;
}

void Te3DTexture::forceTexData(uint gltextures, uint xsize, uint ysize) {
	if (_glTexture != 0xffffffff) {
		if (_createdTexture)
			glDeleteTextures(1, &_glTexture);
		_createdTexture = false;
		_loaded = false;
	}
	_glTexture = gltextures;
	_width = xsize;
	_height = ysize;
	_texWidth = xsize;
	_texHeight = ysize;
}

bool Te3DTexture::hasAlpha() const {
	TeImage::Format format = getFormat();
	return (format == TeImage::RGBA8 || format == 9
			|| format == 0xb || format == 1 || format == 0);
}

/*static*/
TeIntrusivePtr<Te3DTexture> Te3DTexture::load2(const Common::Path &path, uint size) {
	Common::Path fullPath = path.append(".3dtex");

	TeResourceManager *resMgr = g_engine->getResourceManager();
	if (!resMgr->exists(fullPath)) {
		TeIntrusivePtr<Te3DTexture> retval(new Te3DTexture());
		retval->load(path);
		retval->setAccessName(fullPath);
		resMgr->addResource(retval.get());
		return retval;
	} else {
		return resMgr->getResource<Te3DTexture>(fullPath);
	}
}

bool Te3DTexture::load(const Common::Path &path) {
	TeResourceManager *resmgr = g_engine->getResourceManager();
	Common::Path resPath = path;
	TeIntrusivePtr<TeImage> img = resmgr->getResource<TeImage>(resPath);
	load(*img);
	setAccessName(resPath.append(".3dtex"));
	return true;
}

bool Te3DTexture::load(const TeImage &img) {
	Common::Path accessName = img.getAccessName();
	setAccessName(accessName.append(".3dtex"));

	_width = img.w;
	_height = img.h;
	_format = img._format;

	// FIXME: set some other fields from the image here.
	// for now just set some good defaults.
	_flipY = true;    //img._flipY;
	_leftBorder = 0;  //img._leftBorder;
	_btmBorder = 0;   //img._btmBorder;
	_rightBorder = 0; //img._rightBorder;
	_topBorder = 0;   //img._topBorder;

	const TeVector2s32 optimizedSz = optimisedSize(img.bufSize());
	_texWidth = optimizedSz._x;
	_texHeight = optimizedSz._y;

	glBindTexture(GL_TEXTURE_2D, _glTexture);
	glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
	glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	const void *imgdata = img.getPixels();
	if (_format == TeImage::RGB8) {
		GLenum glpxformat = GL_RGB;
		if (_glPixelFormat != GL_INVALID_ENUM) {
			glpxformat = _glPixelFormat;
		}
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, _texWidth, _texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img.pitch / 3, img.h, GL_RGB, GL_UNSIGNED_BYTE, imgdata);
	} else if (_format == TeImage::RGBA8) {
		GLenum glpxformat = GL_RGBA8;
		if (_glPixelFormat != GL_INVALID_ENUM) {
			glpxformat = _glPixelFormat;
		}
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _texWidth, _texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img.w, img.h, GL_RGBA, GL_UNSIGNED_BYTE, imgdata);
	} else {
		warning("Te3DTexture::load can't send image format %d to GL.", _format);
	}

	_matrix.setToIdentity();

	_matrix.scale(TeVector3f32((float)_width / _texWidth, (float)_height / _texHeight, 1.0f));
	_matrix.translate(TeVector3f32((float)_leftBorder / _width, (float)_btmBorder / _height, 0.0f));
	_matrix.scale(TeVector3f32(1.0 - (float)(_rightBorder + _leftBorder) / _width,
			   1.0 - (float)(_topBorder + _btmBorder) / _height, 1.0f));
	if (_flipY) {
		_matrix.translate(TeVector3f32(0.0f, 1.0f, 0.0f));
		_matrix.scale(TeVector3f32(1.0f, -1.0f, 1.0f));
	}
	_loaded = true;
	return true;
}

/*static*/
TeVector2s32 Te3DTexture::optimisedSize(const TeVector2s32 &size) {
	/* The maths here is a bit funky but it just picks the nearest power of 2 (up) */
	int xsize = size._x - 1;
	int ysize = size._y - 1;

	xsize = (int)xsize >> 1 | xsize;
	xsize = (int)xsize >> 2 | xsize;
	xsize = (int)xsize >> 4 | xsize;
	xsize = (int)xsize >> 8 | xsize;
	int v1 = ((int)xsize >> 0x10 | xsize) + 1;
	if (v1 < 8) {
		v1 = 8;
	}

	ysize = (int)ysize >> 1 | ysize;
	ysize = (int)ysize >> 2 | ysize;
	ysize = (int)ysize >> 4 | ysize;
	ysize = (int)ysize >> 8 | ysize;
	int v2 = ((int)ysize >> 0x10 | ysize) + 1;
	if (v2 < 8) {
		v2 = 8;
	}
	return TeVector2s32(v1, v2);
}

/*static*/
void Te3DTexture::unbind() {
	TeRenderer *renderer = g_engine->getRenderer();
	renderer->setMatrixMode(TeRenderer::MM_GL_TEXTURE);
	renderer->loadIdentityMatrix();
	renderer->loadCurrentMatrixToGL();
	glBindTexture(GL_TEXTURE_2D, 0);
	renderer->setMatrixMode(TeRenderer::MM_GL_MODELVIEW);
}

bool Te3DTexture::unload() {
	glBindTexture(GL_TEXTURE_2D, _glTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	_loaded = false;
	return true;
}

void Te3DTexture::update(const TeImage &img, uint xoff, uint yoff) {
	if (!img.w || !img.h)
		return;

	setAccessName(img.getAccessName().append(".3dtex"));
	glBindTexture(GL_TEXTURE_2D, _glTexture);
	glPixelStorei(GL_UNPACK_SWAP_BYTES, 0);
	glPixelStorei(GL_UNPACK_LSB_FIRST, 0);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	const void *imgdata = img.getPixels();
	if (_format == TeImage::RGB8) {
		glTexSubImage2D(GL_TEXTURE_2D, 0, xoff, yoff, img.w, img.h, GL_RGB, GL_UNSIGNED_BYTE, imgdata);
	} else if (_format == TeImage::RGBA8) {
		glTexSubImage2D(GL_TEXTURE_2D, 0, xoff, yoff, img.w, img.h, GL_RGBA, GL_UNSIGNED_BYTE, imgdata);
	} else {
		warning("Te3DTexture::update can't send image format %d to GL.", _format);
	}
	return;
}

// TODO: Add more functions here.

} // end namespace Tetraedge
