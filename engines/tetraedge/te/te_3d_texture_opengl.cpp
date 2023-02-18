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
#include "tetraedge/te/te_3d_texture_opengl.h"
#include "tetraedge/te/te_resource_manager.h"
#include "tetraedge/te/te_renderer.h"

namespace Tetraedge {

static const uint NO_TEXTURE = 0xffffffff;

Te3DTextureOpenGL::Te3DTextureOpenGL() : _glTexture(NO_TEXTURE)/*, _glPixelFormat(GL_INVALID_ENUM)*/ {
	create();
}

Te3DTextureOpenGL::~Te3DTextureOpenGL() {
	destroy();
}

void Te3DTextureOpenGL::bind() const {
	TeRenderer *renderer = g_engine->getRenderer();
	glBindTexture(GL_TEXTURE_2D, _glTexture);
	renderer->setMatrixMode(TeRenderer::MM_GL_TEXTURE);
	renderer->loadMatrix(_matrix);
	renderer->loadCurrentMatrixToGL();
	renderer->setMatrixMode(TeRenderer::MM_GL_MODELVIEW);
}

void Te3DTextureOpenGL::copyCurrentRender(uint xoffset, uint yoffset, uint x, uint y) {
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
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, x, y, _texWidth, _texHeight);
}

void Te3DTextureOpenGL::writeTo(Graphics::Surface &surf) {
	Graphics::Surface fullTex;
	fullTex.create(_texWidth, _texHeight, Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, fullTex.getPixels());
	surf.create(_width, _height, Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));
	surf.copyRectToSurface(fullTex, 0, 0, Common::Rect(_width, _height));
	fullTex.free();
}

void Te3DTextureOpenGL::create() {
	_flipY = false;
	_leftBorder = _btmBorder = _texWidth = _texHeight = 0;
	_rightBorder = _topBorder = _width = _height = 0;
	_format = TeImage::INVALID;
	_loaded = false;
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

void Te3DTextureOpenGL::destroy() {
	if (_createdTexture) {
		glDeleteTextures(1, &_glTexture);
	}
	_createdTexture = false;
	_loaded = false;
	_glTexture = NO_TEXTURE;
}

void Te3DTextureOpenGL::forceTexData(uint gltexture, uint xsize, uint ysize) {
	if (_glTexture != 0xffffffff)
		destroy();
	_glTexture = gltexture;
	_width = xsize;
	_height = ysize;
	_texWidth = xsize;
	_texHeight = ysize;
}

bool Te3DTextureOpenGL::load(const TeImage &img) {
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
		if (_alphaOnly)
			warning("Te3DTexture::load can't load RGB as alpha-only");
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, _texWidth, _texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img.pitch / 3, img.h, GL_RGB, GL_UNSIGNED_BYTE, imgdata);
	} else if (_format == TeImage::RGBA8) {
		Graphics::Surface surf;
		if (_alphaOnly) {
			surf.copyFrom(img);
			// Slight hack: Move R data to A channel.  Our image reader
			// only reads data as RGB, so use red for alpha-only values.
			uint32 *p = (uint32 *)surf.getPixels();
			for (int y = 0; y < img.h; y++) {
				for (int x = 0; x < img.w; x++) {
					byte a, r, g, b;
					img.format.colorToARGB(p[x], a, r, g ,b);
					p[x] = img.format.ARGBToColor(r, r, r, r);
				}
				p += img.pitch / 4;
			}
			imgdata = surf.getPixels();
		}
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _texWidth, _texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img.w, img.h, GL_RGBA, GL_UNSIGNED_BYTE, imgdata);
		// FIXME: Slight hack.. sometimes artifacts appear because we draw
		// a (half?)pixel outside the original texture. Clear one more row
		// of the new texture with 0s to avoid artifacts.
		if ((int)_texHeight > img.h) {
			byte *buf = new byte[img.w * 4];
			memset(buf, 0, img.w * 4);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, img.h, img.w, 1, GL_RGBA, GL_UNSIGNED_BYTE, buf);
			delete [] buf;
		}
		if (_alphaOnly)
			surf.free();
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
void Te3DTextureOpenGL::unbind() {
	TeRenderer *renderer = g_engine->getRenderer();
	renderer->setMatrixMode(TeRenderer::MM_GL_TEXTURE);
	renderer->loadIdentityMatrix();
	renderer->loadCurrentMatrixToGL();
	glBindTexture(GL_TEXTURE_2D, 0);
	renderer->setMatrixMode(TeRenderer::MM_GL_MODELVIEW);
}

bool Te3DTextureOpenGL::unload() {
	glBindTexture(GL_TEXTURE_2D, _glTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	_loaded = false;
	return true;
}

void Te3DTextureOpenGL::update(const TeImage &img, uint xoff, uint yoff) {
	if (!img.w || !img.h)
		return;

	setAccessName(img.getAccessName() + ".3dtex");
	glBindTexture(GL_TEXTURE_2D, _glTexture);
	glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
	glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
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

} // end namespace Tetraedge
