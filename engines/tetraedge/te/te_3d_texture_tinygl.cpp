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

#include "graphics/tinygl/tinygl.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/te/te_3d_texture_tinygl.h"
#include "tetraedge/te/te_resource_manager.h"
#include "tetraedge/te/te_renderer.h"

namespace Tetraedge {

static const uint NO_TEXTURE = 0xffffffff;

Te3DTextureTinyGL::Te3DTextureTinyGL() : _glTexture(NO_TEXTURE)/*, _glPixelFormat(TGL_INVALID_ENUM)*/ {
	create();
}

Te3DTextureTinyGL::~Te3DTextureTinyGL() {
	destroy();
}

void Te3DTextureTinyGL::bind() const {
	TeRenderer *renderer = g_engine->getRenderer();
	tglBindTexture(TGL_TEXTURE_2D, _glTexture);
	renderer->setMatrixMode(TeRenderer::MM_GL_TEXTURE);
	renderer->loadMatrix(_matrix);
	renderer->loadCurrentMatrixToGL();
	renderer->setMatrixMode(TeRenderer::MM_GL_MODELVIEW);
}

void Te3DTextureTinyGL::copyCurrentRender(uint xoffset, uint yoffset, uint x, uint y) {
	_matrix.setToIdentity();
	const TeVector3f32 texScale((float)_width / _texWidth, (float)_height / _texHeight, 1.0);
	_matrix.scale(texScale);
	const TeVector3f32 offset((float)_leftBorder / _width, (float)_btmBorder / _height, 0.0);
	_matrix.translate(offset);
	const TeVector3f32 borderScale(
			1.0 - (float)(_rightBorder + _leftBorder) / (float)_width,
			1.0 - (float)(_topBorder + _btmBorder) / (float)_height, 1.0);
	_matrix.scale(borderScale);
	bind();
	//TODO: Come up with equivalent for TGL.
	//tglCopyTexSubImage2D(TGL_TEXTURE_2D, 0, xoffset, yoffset, x, y, _texWidth, _texHeight);
}

void Te3DTextureTinyGL::writeTo(Graphics::Surface &surf) {
	Graphics::Surface fullTex;
	fullTex.create(_texWidth, _texHeight, Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));
	//TODO: Come up with equivalent for TGL.
	//tglGetTexImage(TGL_TEXTURE_2D, 0, TGL_RGBA, TGL_UNSIGNED_BYTE, fullTex.getPixels());
	surf.create(_width, _height, Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));
	surf.copyRectToSurface(fullTex, 0, 0, Common::Rect(_width, _height));
	fullTex.free();
}

void Te3DTextureTinyGL::create() {
	_flipY = false;
	_leftBorder = _btmBorder = _texWidth = _texHeight = 0;
	_rightBorder = _topBorder = _width = _height = 0;
	_format = TeImage::INVALID;
	_loaded = false;
	if (!_createdTexture)
		tglGenTextures(1, &_glTexture);
	if (_glTexture == NO_TEXTURE) {
		_createdTexture = false;
		return;
	}

	_createdTexture = true;
	tglBindTexture(TGL_TEXTURE_2D, _glTexture);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MAG_FILTER, TGL_LINEAR);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MIN_FILTER, TGL_LINEAR);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_S, TGL_CLAMP_TO_EDGE);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_T, TGL_CLAMP_TO_EDGE);
}

void Te3DTextureTinyGL::destroy() {
	if (_createdTexture) {
		tglDeleteTextures(1, &_glTexture);
	}
	_createdTexture = false;
	_loaded = false;
	_glTexture = NO_TEXTURE;
}

void Te3DTextureTinyGL::forceTexData(uint gltexture, uint xsize, uint ysize) {
	if (_glTexture != 0xffffffff)
		destroy();
	_glTexture = gltexture;
	_width = xsize;
	_height = ysize;
	_texWidth = xsize;
	_texHeight = ysize;
}

bool Te3DTextureTinyGL::load(const TeImage &img) {
	setAccessName(img.getAccessName() + ".3dtex");

	_width = img.w;
	_height = img.h;
	_format = img.teFormat();

	// TODO? set some other fields from the image here.
	// for now just set some good defaults.
	_flipY = true;    //img._flipY;
	_leftBorder = 0;  //img._leftBorder;
	_btmBorder = 0;   //img._btmBorder;
	_rightBorder = 0; //img._rightBorder;
	_topBorder = 0;   //img._topBorder;

	_texWidth = _width;
	_texHeight = _height;

	tglBindTexture(TGL_TEXTURE_2D, _glTexture);
	// Note: these are unsupported in TGL but should be the defaults?
	//tglPixelStorei(TGL_UNPACK_SWAP_BYTES, TGL_FALSE);
	//tglPixelStorei(TGL_UNPACK_LSB_FIRST, TGL_FALSE);
	//tglPixelStorei(TGL_UNPACK_ROW_LENGTH, 0);
	//tglPixelStorei(TGL_UNPACK_SKIP_ROWS, 0);
	//tglPixelStorei(TGL_UNPACK_SKIP_PIXELS, 0);
	tglPixelStorei(TGL_UNPACK_ALIGNMENT, 1);

	const void *imgdata = img.getPixels();
	TGLenum destfmt = _alphaOnly ? TGL_ALPHA : TGL_RGBA;
	if (_format == TeImage::RGB8) {
		tglTexImage2D(TGL_TEXTURE_2D, 0, destfmt, img.pitch / 3, img.h, 0, TGL_RGB, TGL_UNSIGNED_BYTE, imgdata);
	} else if (_format == TeImage::RGBA8) {
		tglTexImage2D(TGL_TEXTURE_2D, 0, destfmt, img.w, img.h, 0, TGL_RGBA, TGL_UNSIGNED_BYTE, imgdata);
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
void Te3DTextureTinyGL::unbind() {
	TeRenderer *renderer = g_engine->getRenderer();
	renderer->setMatrixMode(TeRenderer::MM_GL_TEXTURE);
	renderer->loadIdentityMatrix();
	renderer->loadCurrentMatrixToGL();
	tglBindTexture(TGL_TEXTURE_2D, 0);
	renderer->setMatrixMode(TeRenderer::MM_GL_MODELVIEW);
}

bool Te3DTextureTinyGL::unload() {
	tglBindTexture(TGL_TEXTURE_2D, _glTexture);
	tglTexImage2D(TGL_TEXTURE_2D, 0, TGL_RGB, 0, 0, 0, TGL_RGB, TGL_UNSIGNED_BYTE, NULL);
	_loaded = false;
	return true;
}

void Te3DTextureTinyGL::update(const TeImage &img, uint xoff, uint yoff) {
	if (!img.w || !img.h)
		return;

	setAccessName(img.getAccessName() + ".3dtex");
	tglBindTexture(TGL_TEXTURE_2D, _glTexture);
	// Note: these are unsupported in TGL but should be the defaults?
	//tglPixelStorei(TGL_UNPACK_SWAP_BYTES, TGL_FALSE);
	//tglPixelStorei(TGL_UNPACK_LSB_FIRST, TGL_FALSE);
	//tglPixelStorei(TGL_UNPACK_ROW_LENGTH, 0);
	//tglPixelStorei(TGL_UNPACK_SKIP_ROWS, 0);
	//tglPixelStorei(TGL_UNPACK_SKIP_PIXELS, 0);
	tglPixelStorei(TGL_UNPACK_ALIGNMENT, 1);

	//const void *imgdata = img.getPixels();
	if (_format == TeImage::RGB8) {
		//TODO: Come up with equivalent for TGL.
		//tglTexSubImage2D(TGL_TEXTURE_2D, 0, xoff, yoff, img.w, img.h, TGL_RGB, TGL_UNSIGNED_BYTE, imgdata);
	} else if (_format == TeImage::RGBA8) {
		//TODO: Come up with equivalent for TGL.
		//tglTexSubImage2D(TGL_TEXTURE_2D, 0, xoff, yoff, img.w, img.h, TGL_RGBA, TGL_UNSIGNED_BYTE, imgdata);
	} else {
		warning("Te3DTexture::update can't send image format %d to GL.", _format);
	}
	return;
}

} // end namespace Tetraedge
