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

#include "backends/graphics/opengl/texture.h"
#include "backends/graphics/opengl/shader.h"
#include "backends/graphics/opengl/pipelines/pipeline.h"
#include "backends/graphics/opengl/pipelines/clut8.h"
#include "backends/graphics/opengl/framebuffer.h"
#include "graphics/opengl/debug.h"

#include "common/algorithm.h"
#include "common/endian.h"
#include "common/rect.h"
#include "common/textconsole.h"

#include "graphics/blit.h"

#ifdef USE_SCALERS
#include "graphics/scalerplugin.h"
#endif

namespace OpenGL {

//
// Surface
//

Surface::Surface()
	: _allDirty(false), _dirtyArea() {
}

void Surface::copyRectToTexture(uint x, uint y, uint w, uint h, const void *srcPtr, uint srcPitch) {
	Graphics::Surface *dstSurf = getSurface();
	assert(x + w <= (uint)dstSurf->w);
	assert(y + h <= (uint)dstSurf->h);

	addDirtyArea(Common::Rect(x, y, x + w, y + h));

	const byte *src = (const byte *)srcPtr;
	byte *dst = (byte *)dstSurf->getBasePtr(x, y);
	const uint pitch = dstSurf->pitch;
	const uint bytesPerPixel = dstSurf->format.bytesPerPixel;

	if (srcPitch == pitch && x == 0 && w == (uint)dstSurf->w) {
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

void Surface::fill(const Common::Rect &r, uint32 color) {
	Graphics::Surface *dst = getSurface();
	dst->fillRect(r, color);

	addDirtyArea(r);
}

void Surface::addDirtyArea(const Common::Rect &r) {
	// *sigh* Common::Rect::extend behaves unexpected whenever one of the two
	// parameters is an empty rect. Thus, we check whether the current dirty
	// area is valid. In case it is not we simply use the parameters as new
	// dirty area. Otherwise, we simply call extend.
	if (_dirtyArea.isEmpty()) {
		_dirtyArea = r;
	} else {
		_dirtyArea.extend(r);
	}
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

TextureSurface::TextureSurface(GLenum glIntFormat, GLenum glFormat, GLenum glType, const Graphics::PixelFormat &format)
	: Surface(), _format(format), _glTexture(glIntFormat, glFormat, glType),
	  _textureData(), _userPixelData() {
}

TextureSurface::~TextureSurface() {
	_textureData.free();
}

void TextureSurface::destroy() {
	_glTexture.destroy();
}

void TextureSurface::recreate() {
	_glTexture.create();

	// In case image date exists assure it will be completely refreshed next
	// time.
	if (_textureData.getPixels()) {
		flagDirty();
	}
}

void TextureSurface::enableLinearFiltering(bool enable) {
	_glTexture.enableLinearFiltering(enable);
}

void TextureSurface::allocate(uint width, uint height) {
	// Assure the texture can contain our user data.
	_glTexture.setSize(width, height);

	// In case the needed texture dimension changed we will reinitialize the
	// texture data buffer.
	if (_glTexture.getWidth() != (uint)_textureData.w || _glTexture.getHeight() != (uint)_textureData.h) {
		// Create a buffer for the texture data.
		_textureData.create(_glTexture.getWidth(), _glTexture.getHeight(), _format);
	}

	// Create a sub-buffer for raw access.
	_userPixelData = _textureData.getSubArea(Common::Rect(width, height));

	// The whole texture is dirty after we changed the size. This fixes
	// multiple texture size changes without any actual update in between.
	// Without this we might try to write a too big texture into the GL
	// texture.
	flagDirty();
}

void TextureSurface::updateGLTexture() {
	if (!isDirty()) {
		return;
	}

	Common::Rect dirtyArea = getDirtyArea();

	updateGLTexture(dirtyArea);
}

void TextureSurface::updateGLTexture(Common::Rect &dirtyArea) {
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

FakeTextureSurface::FakeTextureSurface(GLenum glIntFormat, GLenum glFormat, GLenum glType, const Graphics::PixelFormat &format, const Graphics::PixelFormat &fakeFormat)
	: TextureSurface(glIntFormat, glFormat, glType, format),
	  _fakeFormat(fakeFormat),
	  _rgbData(),
	  _palette(nullptr),
	  _mask(nullptr) {
	if (_fakeFormat.isCLUT8()) {
		_palette = new uint32[256]();
	}
}

FakeTextureSurface::~FakeTextureSurface() {
	delete[] _palette;
	delete[] _mask;
	_palette = nullptr;
	_rgbData.free();
}

void FakeTextureSurface::allocate(uint width, uint height) {
	TextureSurface::allocate(width, height);

	// We only need to reinitialize our surface when the output size
	// changed.
	if (width == (uint)_rgbData.w && height == (uint)_rgbData.h) {
		return;
	}

	_rgbData.create(width, height, getFormat());
}

void FakeTextureSurface::setMask(const byte *mask) {
	if (mask) {
		const uint numPixels = _rgbData.w * _rgbData.h;

		if (!_mask)
			_mask = new byte[numPixels];

		memcpy(_mask, mask, numPixels);
	} else {
		delete[] _mask;
		_mask = nullptr;
	}

	flagDirty();
}

void FakeTextureSurface::setColorKey(uint colorKey) {
	if (!_palette)
		return;

	// The key color is set to black so the color value is pre-multiplied with the alpha value
	// to avoid color fringes due to filtering.
	// Erasing the color data is not a problem as the palette is always fully re-initialized
	// before setting the key color.
	uint32 *palette = _palette + colorKey;
	*palette = 0;

	// A palette changes means we need to refresh the whole surface.
	flagDirty();
}

void FakeTextureSurface::setPalette(uint start, uint colors, const byte *palData) {
	if (!_palette)
		return;

	Graphics::convertPaletteToMap(_palette + start, palData, colors, _format);

	// A palette changes means we need to refresh the whole surface.
	flagDirty();
}

void FakeTextureSurface::updateGLTexture() {
	if (!isDirty()) {
		return;
	}

	// Convert color space.
	Graphics::Surface *outSurf = TextureSurface::getSurface();

	const Common::Rect dirtyArea = getDirtyArea();

	byte *dst = (byte *)outSurf->getBasePtr(dirtyArea.left, dirtyArea.top);
	const byte *src = (const byte *)_rgbData.getBasePtr(dirtyArea.left, dirtyArea.top);

	applyPaletteAndMask(dst, src, outSurf->pitch, _rgbData.pitch, _rgbData.w, dirtyArea, outSurf->format, _rgbData.format);

	// Do generic handling of updating the texture.
	TextureSurface::updateGLTexture();
}

void FakeTextureSurface::applyPaletteAndMask(byte *dst, const byte *src, uint dstPitch, uint srcPitch, uint srcWidth, const Common::Rect &dirtyArea, const Graphics::PixelFormat &dstFormat, const Graphics::PixelFormat &srcFormat) const {
	if (_palette) {
		Graphics::crossBlitMap(dst, src, dstPitch, srcPitch, dirtyArea.width(), dirtyArea.height(), dstFormat.bytesPerPixel, _palette);
	} else {
		Graphics::crossBlit(dst, src, dstPitch, srcPitch, dirtyArea.width(), dirtyArea.height(), dstFormat, srcFormat);
	}

	if (_mask) {
		uint maskPitch = srcWidth;
		uint dirtyWidth = dirtyArea.width();
		byte destBPP = dstFormat.bytesPerPixel;

		const byte *maskRowStart = (_mask + dirtyArea.top * maskPitch + dirtyArea.left);
		byte *dstRowStart = dst;

		for (uint y = dirtyArea.top; y < static_cast<uint>(dirtyArea.bottom); y++) {
			if (destBPP == 2) {
				for (uint x = 0; x < dirtyWidth; x++) {
					if (!maskRowStart[x])
						reinterpret_cast<uint16 *>(dstRowStart)[x] = 0;
				}
			} else if (destBPP == 4) {
				for (uint x = 0; x < dirtyWidth; x++) {
					if (!maskRowStart[x])
						reinterpret_cast<uint32 *>(dstRowStart)[x] = 0;
				}
			}

			dstRowStart += dstPitch;
			maskRowStart += maskPitch;
		}
	}
}

TextureSurfaceRGB555::TextureSurfaceRGB555()
	: FakeTextureSurface(GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0), Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0)) {
}

void TextureSurfaceRGB555::updateGLTexture() {
	if (!isDirty()) {
		return;
	}

	// Convert color space.
	Graphics::Surface *outSurf = TextureSurface::getSurface();

	const Common::Rect dirtyArea = getDirtyArea();

	uint16 *dst = (uint16 *)outSurf->getBasePtr(dirtyArea.left, dirtyArea.top);
	const uint dstAdd = outSurf->pitch - 2 * dirtyArea.width();

	const uint16 *src = (const uint16 *)_rgbData.getBasePtr(dirtyArea.left, dirtyArea.top);
	const uint srcAdd = _rgbData.pitch - 2 * dirtyArea.width();

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
	TextureSurface::updateGLTexture();
}

TextureSurfaceRGBA8888Swap::TextureSurfaceRGBA8888Swap()
#ifdef SCUMM_LITTLE_ENDIAN
	: FakeTextureSurface(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24), Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0)) // RGBA8888 -> ABGR8888
#else
	: FakeTextureSurface(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0), Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24)) // ABGR8888 -> RGBA8888
#endif
	  {
}

void TextureSurfaceRGBA8888Swap::updateGLTexture() {
	if (!isDirty()) {
		return;
	}

	// Convert color space.
	Graphics::Surface *outSurf = TextureSurface::getSurface();

	const Common::Rect dirtyArea = getDirtyArea();

	uint32 *dst = (uint32 *)outSurf->getBasePtr(dirtyArea.left, dirtyArea.top);
	const uint dstAdd = outSurf->pitch - 4 * dirtyArea.width();

	const uint32 *src = (const uint32 *)_rgbData.getBasePtr(dirtyArea.left, dirtyArea.top);
	const uint srcAdd = _rgbData.pitch - 4 * dirtyArea.width();

	for (int height = dirtyArea.height(); height > 0; --height) {
		for (int width = dirtyArea.width(); width > 0; --width) {
			const uint32 color = *src++;

			*dst++ = SWAP_BYTES_32(color);
		}

		src = (const uint32 *)((const byte *)src + srcAdd);
		dst = (uint32 *)((byte *)dst + dstAdd);
	}

	// Do generic handling of updating the texture.
	TextureSurface::updateGLTexture();
}

#ifdef USE_SCALERS

ScaledTextureSurface::ScaledTextureSurface(GLenum glIntFormat, GLenum glFormat, GLenum glType, const Graphics::PixelFormat &format, const Graphics::PixelFormat &fakeFormat)
	: FakeTextureSurface(glIntFormat, glFormat, glType, format, fakeFormat), _convData(nullptr), _scaler(nullptr), _scalerIndex(0), _scaleFactor(1), _extraPixels(0) {
}

ScaledTextureSurface::~ScaledTextureSurface() {
	delete _scaler;

	if (_convData) {
		_convData->free();
		delete _convData;
	}
}

void ScaledTextureSurface::allocate(uint width, uint height) {
	TextureSurface::allocate(width * _scaleFactor, height * _scaleFactor);

	// We only need to reinitialize our surface when the output size
	// changed.
	if (width != (uint)_rgbData.w || height != (uint)_rgbData.h) {
		_rgbData.create(width, height, _fakeFormat);
	}

	if (_format != _fakeFormat || _extraPixels != 0) {
		if (!_convData)
			_convData = new Graphics::Surface();

		_convData->create(width + (_extraPixels * 2), height + (_extraPixels * 2), _format);
	} else if (_convData) {
		_convData->free();
		delete _convData;
		_convData = nullptr;
	}
}

void ScaledTextureSurface::updateGLTexture() {
	if (!isDirty()) {
		return;
	}

	// Convert color space.
	Graphics::Surface *outSurf = TextureSurface::getSurface();

	Common::Rect dirtyArea = getDirtyArea();

	// Extend the dirty region for scalers
	// that "smear" the screen, e.g. 2xSAI
	dirtyArea.grow(_extraPixels);
	dirtyArea.clip(Common::Rect(0, 0, _rgbData.w, _rgbData.h));

	const byte *src = (const byte *)_rgbData.getBasePtr(dirtyArea.left, dirtyArea.top);
	uint srcPitch = _rgbData.pitch;
	byte *dst;
	uint dstPitch;

	if (_convData) {
		dst = (byte *)_convData->getBasePtr(dirtyArea.left + _extraPixels, dirtyArea.top + _extraPixels);
		dstPitch = _convData->pitch;

		applyPaletteAndMask(dst, src, dstPitch, srcPitch, _rgbData.w, dirtyArea, _convData->format, _rgbData.format);

		src = dst;
		srcPitch = dstPitch;
	}

	dst = (byte *)outSurf->getBasePtr(dirtyArea.left * _scaleFactor, dirtyArea.top * _scaleFactor);
	dstPitch = outSurf->pitch;

	if (_scaler && (uint)dirtyArea.height() >= _extraPixels) {
		_scaler->scale(src, srcPitch, dst, dstPitch, dirtyArea.width(), dirtyArea.height(), dirtyArea.left, dirtyArea.top);
	} else {
		Graphics::scaleBlit(dst, src, dstPitch, srcPitch,
		                    dirtyArea.width() * _scaleFactor, dirtyArea.height() * _scaleFactor,
		                    dirtyArea.width(), dirtyArea.height(), outSurf->format);
	}

	dirtyArea.left   *= _scaleFactor;
	dirtyArea.right  *= _scaleFactor;
	dirtyArea.top    *= _scaleFactor;
	dirtyArea.bottom *= _scaleFactor;

	// Do generic handling of updating the texture.
	TextureSurface::updateGLTexture(dirtyArea);
}

void ScaledTextureSurface::setScaler(uint scalerIndex, int scaleFactor) {
	const PluginList &scalerPlugins = ScalerMan.getPlugins();
	const ScalerPluginObject &scalerPlugin = scalerPlugins[scalerIndex]->get<ScalerPluginObject>();

	// If the scalerIndex has changed, change scaler plugins
	if (_scaler && scalerIndex != _scalerIndex) {
		delete _scaler;
		_scaler = nullptr;
	}

	if (!_scaler) {
		_scaler = scalerPlugin.createInstance(_format);
	}
	_scaler->setFactor(scaleFactor);

	_scalerIndex = scalerIndex;
	_scaleFactor = _scaler->getFactor();
	_extraPixels = scalerPlugin.extraPixels();
}
#endif

#if !USE_FORCED_GLES

// _clut8Texture needs 8 bits internal precision, otherwise graphics glitches
// can occur. GL_ALPHA does not have any internal precision requirements.
// However, in practice (according to fuzzie) it's 8bit. If we run into
// problems, we need to switch to GL_R8 and GL_RED, but that is only supported
// for ARB_texture_rg and GLES3+ (EXT_rexture_rg does not support GL_R8).
TextureSurfaceCLUT8GPU::TextureSurfaceCLUT8GPU()
	: _clut8Texture(GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE),
	  _paletteTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE),
	  _target(new TextureTarget()), _clut8Pipeline(new CLUT8LookUpPipeline()),
	  _clut8Vertices(), _clut8Data(), _userPixelData(), _palette(),
	  _paletteDirty(false) {
	// Allocate space for 256 colors.
	_paletteTexture.setSize(256, 1);

	// Setup pipeline.
	_clut8Pipeline->setFramebuffer(_target);
	_clut8Pipeline->setPaletteTexture(&_paletteTexture);
	_clut8Pipeline->setColor(1.0f, 1.0f, 1.0f, 1.0f);
}

TextureSurfaceCLUT8GPU::~TextureSurfaceCLUT8GPU() {
	delete _clut8Pipeline;
	delete _target;
	_clut8Data.free();
}

void TextureSurfaceCLUT8GPU::destroy() {
	_clut8Texture.destroy();
	_paletteTexture.destroy();
	_target->destroy();
	delete _clut8Pipeline;
	_clut8Pipeline = nullptr;
}

void TextureSurfaceCLUT8GPU::recreate() {
	_clut8Texture.create();
	_paletteTexture.create();
	_target->create();

	// In case image date exists assure it will be completely refreshed next
	// time.
	if (_clut8Data.getPixels()) {
		flagDirty();
		_paletteDirty = true;
	}

	if (_clut8Pipeline == nullptr) {
		_clut8Pipeline = new CLUT8LookUpPipeline();
		// Setup pipeline.
		_clut8Pipeline->setFramebuffer(_target);
		_clut8Pipeline->setPaletteTexture(&_paletteTexture);
		_clut8Pipeline->setColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
}

void TextureSurfaceCLUT8GPU::enableLinearFiltering(bool enable) {
	_target->getTexture()->enableLinearFiltering(enable);
}

void TextureSurfaceCLUT8GPU::allocate(uint width, uint height) {
	// Assure the texture can contain our user data.
	_clut8Texture.setSize(width, height);
	_target->setSize(width, height, Common::kRotationNormal);

	// In case the needed texture dimension changed we will reinitialize the
	// texture data buffer.
	if (_clut8Texture.getWidth() != (uint)_clut8Data.w || _clut8Texture.getHeight() != (uint)_clut8Data.h) {
		// Create a buffer for the texture data.
		_clut8Data.create(_clut8Texture.getWidth(), _clut8Texture.getHeight(), Graphics::PixelFormat::createFormatCLUT8());
	}

	// Create a sub-buffer for raw access.
	_userPixelData = _clut8Data.getSubArea(Common::Rect(width, height));

	// Setup structures for internal rendering to _glTexture.
	_clut8Vertices[0] = 0;
	_clut8Vertices[1] = 0;

	_clut8Vertices[2] = width;
	_clut8Vertices[3] = 0;

	_clut8Vertices[4] = 0;
	_clut8Vertices[5] = height;

	_clut8Vertices[6] = width;
	_clut8Vertices[7] = height;

	// The whole texture is dirty after we changed the size. This fixes
	// multiple texture size changes without any actual update in between.
	// Without this we might try to write a too big texture into the GL
	// texture.
	flagDirty();
}

Graphics::PixelFormat TextureSurfaceCLUT8GPU::getFormat() const {
	return Graphics::PixelFormat::createFormatCLUT8();
}

void TextureSurfaceCLUT8GPU::setColorKey(uint colorKey) {
	// The key color is set to black so the color value is pre-multiplied with the alpha value
	// to avoid color fringes due to filtering.
	// Erasing the color data is not a problem as the palette is always fully re-initialized
	// before setting the key color.
	_palette[colorKey * 4    ] = 0x00;
	_palette[colorKey * 4 + 1] = 0x00;
	_palette[colorKey * 4 + 2] = 0x00;
	_palette[colorKey * 4 + 3] = 0x00;

	_paletteDirty = true;
}

void TextureSurfaceCLUT8GPU::setPalette(uint start, uint colors, const byte *palData) {
	byte *dst = _palette + start * 4;

	while (colors-- > 0) {
		memcpy(dst, palData, 3);
		dst[3] = 0xFF;

		dst += 4;
		palData += 3;
	}

	_paletteDirty = true;
}

const Texture &TextureSurfaceCLUT8GPU::getGLTexture() const {
	return *_target->getTexture();
}

void TextureSurfaceCLUT8GPU::updateGLTexture() {
	const bool needLookUp = Surface::isDirty() || _paletteDirty;

	// Update CLUT8 texture if necessary.
	if (Surface::isDirty()) {
		_clut8Texture.updateArea(getDirtyArea(), _clut8Data);
		clearDirty();
	}

	// Update palette if necessary.
	if (_paletteDirty) {
		Graphics::Surface palSurface;
		palSurface.init(256, 1, 256, _palette,
#ifdef SCUMM_LITTLE_ENDIAN
		                Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24) // ABGR8888
#else
		                Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0) // RGBA8888
#endif
		               );

		_paletteTexture.updateArea(Common::Rect(256, 1), palSurface);
		_paletteDirty = false;
	}

	// In case any data changed, do color look up and store result in _target.
	if (needLookUp) {
		lookUpColors();
	}
}

void TextureSurfaceCLUT8GPU::lookUpColors() {
	// Setup pipeline to do color look up.
	_clut8Pipeline->activate();

	// Do color look up.
	_clut8Pipeline->drawTexture(_clut8Texture, _clut8Vertices);

	_clut8Pipeline->deactivate();
}
#endif // !USE_FORCED_GLES

} // End of namespace OpenGL
