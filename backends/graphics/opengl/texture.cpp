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

GLint Texture::_maxTextureSize = 0;

void Texture::queryTextureInformation() {
	GL_CALL(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &_maxTextureSize));
	debug(5, "OpenGL maximum texture size: %d", _maxTextureSize);
}

Texture::Texture(GLenum glIntFormat, GLenum glFormat, GLenum glType, const Graphics::PixelFormat &format)
    : _glIntFormat(glIntFormat), _glFormat(glFormat), _glType(glType), _format(format), _glFilter(GL_NEAREST),
      _glTexture(0), _textureData(), _userPixelData(), _allDirty(false) {
	recreateInternalTexture();
}

Texture::~Texture() {
	GL_CALL_SAFE(glDeleteTextures, (1, &_glTexture));
	_textureData.free();
}

void Texture::releaseInternalTexture() {
	GL_CALL(glDeleteTextures(1, &_glTexture));
	_glTexture = 0;
}

void Texture::recreateInternalTexture() {
	// Release old texture name in case it exists.
	releaseInternalTexture();

	// Get a new texture name.
	GL_CALL(glGenTextures(1, &_glTexture));

	// Set up all texture parameters.
	GL_CALL(glBindTexture(GL_TEXTURE_2D, _glTexture));
	GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _glFilter));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _glFilter));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	// In case there is an actual texture setup we reinitialize it.
	if (_textureData.getPixels()) {
		// Allocate storage for OpenGL texture.
		GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, _glIntFormat, _textureData.w,
		       _textureData.h, 0, _glFormat, _glType, NULL));

		// Mark dirts such that it will be completely refreshed the next time.
		flagDirty();
	}
}

void Texture::enableLinearFiltering(bool enable) {
	if (enable) {
		_glFilter = GL_LINEAR;
	} else {
		_glFilter = GL_NEAREST;
	}

	GL_CALL(glBindTexture(GL_TEXTURE_2D, _glTexture));

	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _glFilter));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _glFilter));
}

void Texture::allocate(uint width, uint height) {
	uint texWidth = width, texHeight = height;
	if (!g_context.NPOTSupported) {
		texWidth  = nextHigher2(texWidth);
		texHeight = nextHigher2(texHeight);
	}

	// In case the needed texture dimension changed we will reinitialize the
	// texture.
	if (texWidth != _textureData.w || texHeight != _textureData.h) {
		// Create a buffer for the texture data.
		_textureData.create(texWidth, texHeight, _format);

		// Set the texture.
		GL_CALL(glBindTexture(GL_TEXTURE_2D, _glTexture));

		// Allocate storage for OpenGL texture.
		GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, _glIntFormat, _textureData.w,
		       _textureData.h, 0, _glFormat, _glType, NULL));
	}

	// Create a sub-buffer for raw access.
	_userPixelData = _textureData.getSubArea(Common::Rect(width, height));
}

void Texture::copyRectToTexture(uint x, uint y, uint w, uint h, const void *srcPtr, uint srcPitch) {
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

void Texture::fill(uint32 color) {
	Graphics::Surface *dst = getSurface();
	dst->fillRect(Common::Rect(dst->w, dst->h), color);

	flagDirty();
}

void Texture::draw(GLfloat x, GLfloat y, GLfloat w, GLfloat h) {
	// Only do any processing when the Texture is initialized.
	if (!_textureData.getPixels()) {
		return;
	}

	// First update any potentional changes.
	updateTexture();

	// Set the texture.
	GL_CALL(glBindTexture(GL_TEXTURE_2D, _glTexture));

	// Calculate the texture rect that will be drawn.
	const GLfloat texWidth = (GLfloat)_userPixelData.w / _textureData.w;
	const GLfloat texHeight = (GLfloat)_userPixelData.h / _textureData.h;
	const GLfloat texcoords[4*2] = {
		0,        0,
		texWidth, 0,
		0,        texHeight,
		texWidth, texHeight
	};

	// Calculate the screen rect where the texture will be drawn.
	const GLfloat vertices[4*2] = {
		x,     y,
		x + w, y,
		x,     y + h,
		x + w, y + h
	};

#if !USE_FORCED_GL && !USE_FORCED_GLES && !USE_FORCED_GLES2
	if (g_context.type == kContextGLES2) {
#endif
#if !USE_FORCED_GL && !USE_FORCED_GLES
		GL_CALL(glVertexAttribPointer(kTexCoordAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, texcoords));
		GL_CALL(glVertexAttribPointer(kPositionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, vertices));
#endif
#if !USE_FORCED_GL && !USE_FORCED_GLES && !USE_FORCED_GLES2
	} else {
#endif
#if !USE_FORCED_GLES2
#if !USE_FORCED_GLES
		if (g_context.shadersSupported) {
			GL_CALL(glVertexAttribPointerARB(kTexCoordAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, texcoords));
			GL_CALL(glVertexAttribPointerARB(kPositionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, vertices));
		} else {
#endif
			GL_CALL(glTexCoordPointer(2, GL_FLOAT, 0, texcoords));
			GL_CALL(glVertexPointer(2, GL_FLOAT, 0, vertices));
#if !USE_FORCED_GLES
		}
#endif
#endif
#if !USE_FORCED_GL && !USE_FORCED_GLES && !USE_FORCED_GLES2
	}
#endif

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
	if (_glFilter == GL_LINEAR) {
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

	// Set the texture.
	GL_CALL(glBindTexture(GL_TEXTURE_2D, _glTexture));

	// Update the actual texture.
	// Although we keep track of the dirty part of the texture buffer we
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
	GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, dirtyArea.top, _textureData.w, dirtyArea.height(),
	                       _glFormat, _glType, _textureData.getBasePtr(0, dirtyArea.top)));

	// We should have handled everything, thus not dirty anymore.
	clearDirty();
}

Common::Rect Texture::getDirtyArea() const {
	if (_allDirty) {
		return Common::Rect(_userPixelData.w, _userPixelData.h);
	} else {
		return _dirtyArea;
	}
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
	const Graphics::PixelFormat &hardwareFormat = getHardwareFormat();

	if (hardwareFormat.bytesPerPixel == 2) {
		convertPalette<uint16>((uint16 *)_palette + start, palData, colors, hardwareFormat);
	} else if (hardwareFormat.bytesPerPixel == 4) {
		convertPalette<uint32>((uint32 *)_palette + start, palData, colors, hardwareFormat);
	} else {
		warning("TextureCLUT8::setPalette: Unsupported pixel depth: %d", hardwareFormat.bytesPerPixel);
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
