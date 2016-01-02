/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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

#include "backends/graphics/opengl/texture.h"
#include "backends/graphics/opengl/shader.h"

#include "common/rect.h"
#include "common/textconsole.h"

namespace OpenGL {

static GLuint nextHigher2(GLuint v) {
	if (v == 0)
		return 1;
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	return ++v;
}


GLTexture::GLTexture(GLenum glIntFormat, GLenum glFormat, GLenum glType)
    : _glIntFormat(glIntFormat), _glFormat(glFormat), _glType(glType),
      _width(0), _height(0), _texCoords(), _glFilter(GL_NEAREST),
      _glTexture(0) {
	create();
}

GLTexture::~GLTexture() {
	GL_CALL_SAFE(glDeleteTextures, (1, &_glTexture));
}

void GLTexture::enableLinearFiltering(bool enable) {
	if (enable) {
		_glFilter = GL_LINEAR;
	} else {
		_glFilter = GL_NEAREST;
	}

	bind();

	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _glFilter));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _glFilter));
}

void GLTexture::destroy() {
	GL_CALL(glDeleteTextures(1, &_glTexture));
	_glTexture = 0;
}

void GLTexture::create() {
	// Release old texture name in case it exists.
	destroy();

	// Get a new texture name.
	GL_CALL(glGenTextures(1, &_glTexture));

	// Set up all texture parameters.
	bind();
	GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _glFilter));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _glFilter));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	// If a size is specified, allocate memory for it.
	if (_width != 0 && _height != 0) {
		// Allocate storage for OpenGL texture.
		GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, _glIntFormat, _width, _height,
		                     0, _glFormat, _glType, NULL));
	}
}

void GLTexture::bind() {
	GL_CALL(glBindTexture(GL_TEXTURE_2D, _glTexture));
}

void GLTexture::setSize(uint width, uint height) {
	const uint oldWidth  = _width;
	const uint oldHeight = _height;

	if (!g_context.NPOTSupported) {
		_width  = nextHigher2(width);
		_height = nextHigher2(height);
	} else {
		_width  = width;
		_height = height;
	}

	// If a size is specified, allocate memory for it.
	if (width != 0 && height != 0) {
		const GLfloat texWidth = (GLfloat)width / _width;
		const GLfloat texHeight = (GLfloat)height / _height;

		_texCoords[0] = 0;
		_texCoords[1] = 0;

		_texCoords[2] = texWidth;
		_texCoords[3] = 0;

		_texCoords[4] = 0;
		_texCoords[5] = texHeight;

		_texCoords[6] = texWidth;
		_texCoords[7] = texHeight;

		// Allocate storage for OpenGL texture if necessary.
		if (oldWidth != _width || oldHeight != _height) {
			bind();
			GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, _glIntFormat, _width,
			                     _height, 0, _glFormat, _glType, NULL));
		}
	}
}

void GLTexture::updateArea(const Common::Rect &area, const Graphics::Surface &src) {
	// Set the texture on the active texture unit.
	bind();

	// Update the actual texture.
	// Although we have the area of the texture buffer we want to update we
	// cannot take advantage of the left/right boundries here because it is
	// not possible to specify a pitch to glTexSubImage2D. To be precise, with
	// plain OpenGL we could set GL_UNPACK_ROW_LENGTH to achieve this. However,
	// OpenGL ES 1.0 does not support GL_UNPACK_ROW_LENGTH. Thus, we are left
	// with the following options:
	//
	// 1) (As we do right now) Simply always update the whole texture lines of
	//    rect changed. This is simplest to implement. In case performance is
	//    really an issue we can think of switching to another method.
	//
	// 2) Copy the dirty rect to a temporary buffer and upload that by using
	//    glTexSubImage2D. This is what the Android backend does. It is more
	//    complicated though.
	//
	// 3) Use glTexSubImage2D per line changed. This is what the old OpenGL
	//    graphics manager did but it is much slower! Thus, we do not use it.
	GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, area.top, src.w, area.height(),
	                       _glFormat, _glType, src.getBasePtr(0, area.top)));
}

//
// Surface
//

Surface::Surface()
    : _allDirty(false), _dirtyArea() {
}

void Surface::copyRectToTexture(uint x, uint y, uint w, uint h, const void *srcPtr, uint srcPitch) {
	Graphics::Surface *dstSurf = getSurface();
	assert(x + w <= dstSurf->w);
	assert(y + h <= dstSurf->h);

	// *sigh* Common::Rect::extend behaves unexpected whenever one of the two
	// parameters is an empty rect. Thus, we check whether the current dirty
	// area is valid. In case it is not we simply use the parameters as new
	// dirty area. Otherwise, we simply call extend.
	if (_dirtyArea.isEmpty()) {
		_dirtyArea = Common::Rect(x, y, x + w, y + h);
	} else {
		_dirtyArea.extend(Common::Rect(x, y, x + w, y + h));
	}

	const byte *src = (const byte *)srcPtr;
	byte *dst = (byte *)dstSurf->getBasePtr(x, y);
	const uint pitch = dstSurf->pitch;
	const uint bytesPerPixel = dstSurf->format.bytesPerPixel;

	if (srcPitch == pitch && x == 0 && w == dstSurf->w) {
		memcpy(dst, src, h * pitch);
	} else {
		while (h-- > 0) {
			memcpy(dst, src, w * bytesPerPixel);
			dst += pitch;
			src += srcPitch;
		}
	}
}

void Surface::fill(uint32 color) {
	Graphics::Surface *dst = getSurface();
	dst->fillRect(Common::Rect(dst->w, dst->h), color);

	flagDirty();
}

Common::Rect Surface::getDirtyArea() const {
	if (_allDirty) {
		return Common::Rect(getWidth(), getHeight());
	} else {
		return _dirtyArea;
	}
}

//
// Surface implementations
//

Texture::Texture(GLenum glIntFormat, GLenum glFormat, GLenum glType, const Graphics::PixelFormat &format)
    : Surface(), _format(format), _glTexture(glIntFormat, glFormat, glType),
      _textureData(), _userPixelData() {
}

Texture::~Texture() {
	_textureData.free();
}

void Texture::destroy() {
	_glTexture.destroy();
}

void Texture::recreate() {
	_glTexture.create();

	// In case image date exists assure it will be completely refreshed next
	// time.
	if (_textureData.getPixels()) {
		flagDirty();
	}
}

void Texture::enableLinearFiltering(bool enable) {
	_glTexture.enableLinearFiltering(enable);
}

void Texture::allocate(uint width, uint height) {
	// Assure the texture can contain our user data.
	_glTexture.setSize(width, height);

	// In case the needed texture dimension changed we will reinitialize the
	// texture data buffer.
	if (_glTexture.getWidth() != _textureData.w || _glTexture.getHeight() != _textureData.h) {
		// Create a buffer for the texture data.
		_textureData.create(_glTexture.getWidth(), _glTexture.getHeight(), _format);
	}

	// Create a sub-buffer for raw access.
	_userPixelData = _textureData.getSubArea(Common::Rect(width, height));
}

void Texture::draw(GLfloat x, GLfloat y, GLfloat w, GLfloat h) {
	// Only do any processing when the Texture is initialized.
	if (!_textureData.getPixels()) {
		return;
	}

	// First update any potentional changes.
	updateTexture();

	// Set the texture.
	_glTexture.bind();

	// Calculate the screen rect where the texture will be drawn.
	const GLfloat vertices[4*2] = {
		x,     y,
		x + w, y,
		x,     y + h,
		x + w, y + h
	};

	// Setup coordinates for drawing.
	g_context.setDrawCoordinates(vertices, _glTexture.getTexCoords());

	// Draw the texture to the screen buffer.
	GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
}

void Texture::updateTexture() {
	if (!isDirty()) {
		return;
	}

	Common::Rect dirtyArea = getDirtyArea();

	// In case we use linear filtering we might need to duplicate the last
	// pixel row/column to avoid glitches with filtering.
	if (_glTexture.isLinearFilteringEnabled()) {
		if (dirtyArea.right == _userPixelData.w && _userPixelData.w != _textureData.w) {
			uint height = dirtyArea.height();

			const byte *src = (const byte *)_textureData.getBasePtr(_userPixelData.w - 1, dirtyArea.top);
			byte *dst = (byte *)_textureData.getBasePtr(_userPixelData.w, dirtyArea.top);

			while (height-- > 0) {
				memcpy(dst, src, _textureData.format.bytesPerPixel);
				dst += _textureData.pitch;
				src += _textureData.pitch;
			}

			// Extend the dirty area.
			++dirtyArea.right;
		}

		if (dirtyArea.bottom == _userPixelData.h && _userPixelData.h != _textureData.h) {
			const byte *src = (const byte *)_textureData.getBasePtr(dirtyArea.left, _userPixelData.h - 1);
			byte *dst = (byte *)_textureData.getBasePtr(dirtyArea.left, _userPixelData.h);
			memcpy(dst, src, dirtyArea.width() * _textureData.format.bytesPerPixel);

			// Extend the dirty area.
			++dirtyArea.bottom;
		}
	}

	_glTexture.updateArea(dirtyArea, _textureData);

	// We should have handled everything, thus not dirty anymore.
	clearDirty();
}

TextureCLUT8::TextureCLUT8(GLenum glIntFormat, GLenum glFormat, GLenum glType, const Graphics::PixelFormat &format)
    : Texture(glIntFormat, glFormat, glType, format), _clut8Data(), _palette(new byte[256 * format.bytesPerPixel]) {
	memset(_palette, 0, sizeof(byte) * format.bytesPerPixel);
}

TextureCLUT8::~TextureCLUT8() {
	delete[] _palette;
	_palette = nullptr;
	_clut8Data.free();
}

void TextureCLUT8::allocate(uint width, uint height) {
	Texture::allocate(width, height);

	// We only need to reinitialize our CLUT8 surface when the output size
	// changed.
	if (width == _clut8Data.w && height == _clut8Data.h) {
		return;
	}

	_clut8Data.create(width, height, Graphics::PixelFormat::createFormatCLUT8());
}

Graphics::PixelFormat TextureCLUT8::getFormat() const {
	return Graphics::PixelFormat::createFormatCLUT8();
}

void TextureCLUT8::setColorKey(uint colorKey) {
	// We remove all alpha bits from the palette entry of the color key.
	// This makes sure its properly handled as color key.
	const uint32 aMask = (0xFF >> _format.aLoss) << _format.aShift;

	if (_format.bytesPerPixel == 2) {
		uint16 *palette = (uint16 *)_palette + colorKey;
		*palette &= ~aMask;
	} else if (_format.bytesPerPixel == 4) {
		uint32 *palette = (uint32 *)_palette + colorKey;
		*palette &= ~aMask;
	} else {
		warning("TextureCLUT8::setColorKey: Unsupported pixel depth %d", _format.bytesPerPixel);
	}

	// A palette changes means we need to refresh the whole surface.
	flagDirty();
}

namespace {
template<typename ColorType>
inline void convertPalette(ColorType *dst, const byte *src, uint colors, const Graphics::PixelFormat &format) {
	while (colors-- > 0) {
		*dst++ = format.RGBToColor(src[0], src[1], src[2]);
		src += 3;
	}
}
} // End of anonymous namespace

void TextureCLUT8::setPalette(uint start, uint colors, const byte *palData) {
	if (_format.bytesPerPixel == 2) {
		convertPalette<uint16>((uint16 *)_palette + start, palData, colors, _format);
	} else if (_format.bytesPerPixel == 4) {
		convertPalette<uint32>((uint32 *)_palette + start, palData, colors, _format);
	} else {
		warning("TextureCLUT8::setPalette: Unsupported pixel depth: %d", _format.bytesPerPixel);
	}

	// A palette changes means we need to refresh the whole surface.
	flagDirty();
}

namespace {
template<typename PixelType>
inline void doPaletteLookUp(PixelType *dst, const byte *src, uint width, uint height, uint dstPitch, uint srcPitch, const PixelType *palette) {
	uint srcAdd = srcPitch - width;
	uint dstAdd = dstPitch - width * sizeof(PixelType);

	while (height-- > 0) {
		for (uint x = width; x > 0; --x) {
			*dst++ = palette[*src++];
		}

		dst = (PixelType *)((byte *)dst + dstAdd);
		src += srcAdd;
	}
}
} // End of anonymous namespace

void TextureCLUT8::updateTexture() {
	if (!isDirty()) {
		return;
	}

	// Do the palette look up
	Graphics::Surface *outSurf = Texture::getSurface();

	Common::Rect dirtyArea = getDirtyArea();

	if (outSurf->format.bytesPerPixel == 2) {
		doPaletteLookUp<uint16>((uint16 *)outSurf->getBasePtr(dirtyArea.left, dirtyArea.top),
		                        (const byte *)_clut8Data.getBasePtr(dirtyArea.left, dirtyArea.top),
		                        dirtyArea.width(), dirtyArea.height(),
		                        outSurf->pitch, _clut8Data.pitch, (const uint16 *)_palette);
	} else if (outSurf->format.bytesPerPixel == 4) {
		doPaletteLookUp<uint32>((uint32 *)outSurf->getBasePtr(dirtyArea.left, dirtyArea.top),
		                        (const byte *)_clut8Data.getBasePtr(dirtyArea.left, dirtyArea.top),
		                        dirtyArea.width(), dirtyArea.height(),
		                        outSurf->pitch, _clut8Data.pitch, (const uint32 *)_palette);
	} else {
		warning("TextureCLUT8::updateTexture: Unsupported pixel depth: %d", outSurf->format.bytesPerPixel);
	}

	// Do generic handling of updating the texture.
	Texture::updateTexture();
}

#if !USE_FORCED_GL
TextureRGB555::TextureRGB555()
    : Texture(GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0)),
      _rgb555Data() {
}

TextureRGB555::~TextureRGB555() {
	_rgb555Data.free();
}

void TextureRGB555::allocate(uint width, uint height) {
	Texture::allocate(width, height);

	// We only need to reinitialize our RGB555 surface when the output size
	// changed.
	if (width == _rgb555Data.w && height == _rgb555Data.h) {
		return;
	}

	_rgb555Data.create(width, height, Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0));
}

Graphics::PixelFormat TextureRGB555::getFormat() const {
	return Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0);
}

void TextureRGB555::updateTexture() {
	if (!isDirty()) {
		return;
	}

	// Convert color space.
	Graphics::Surface *outSurf = Texture::getSurface();

	const Common::Rect dirtyArea = getDirtyArea();

	uint16 *dst = (uint16 *)outSurf->getBasePtr(dirtyArea.left, dirtyArea.top);
	const uint dstAdd = outSurf->pitch - 2 * dirtyArea.width();

	const uint16 *src = (const uint16 *)_rgb555Data.getBasePtr(dirtyArea.left, dirtyArea.top);
	const uint srcAdd = _rgb555Data.pitch - 2 * dirtyArea.width();

	for (int height = dirtyArea.height(); height > 0; --height) {
		for (int width = dirtyArea.width(); width > 0; --width) {
			const uint16 color = *src++;

			*dst++ =   ((color & 0x7C00) << 1)                             // R
			         | (((color & 0x03E0) << 1) | ((color & 0x0200) >> 4)) // G
			         | (color & 0x001F);                                   // B
		}

		src = (const uint16 *)((const byte *)src + srcAdd);
		dst = (uint16 *)((byte *)dst + dstAdd);
	}

	// Do generic handling of updating the texture.
	Texture::updateTexture();
}
#endif // !USE_FORCED_GL

} // End of namespace OpenGL
