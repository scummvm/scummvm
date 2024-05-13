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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "graphics/blit.h"
#include "backends/graphics/metal/texture.h"

namespace Metal {

MetalTexture::MetalTexture(MTL::Device *device, MTL::PixelFormat pixelFormat, MTL::TextureUsage usage)
	: _device(device), _pixelFormat(pixelFormat),
	  _width(0), _height(0), _logicalWidth(0), _logicalHeight(0),
	  _texCoords(), _usage(usage), _filter(MTL::SamplerMinMagFilterNearest),
	  _texture(nullptr) {
	create();
}

MetalTexture::~MetalTexture() {
	if (_texture)
		_texture->release();
}

void MetalTexture::enableLinearFiltering(bool enable) {
	if (enable) {
		_filter = MTL::SamplerMinMagFilterLinear;
	} else {
		_filter = MTL::SamplerMinMagFilterNearest;
	}
}

void MetalTexture::setWrapMode(WrapMode wrapMode) {
	switch(wrapMode) {
	case kWrapModeBorder:
		_wrapMode = MTL::SamplerAddressModeClampToBorderColor;
		break;
	case kWrapModeEdge:
		_wrapMode = MTL::SamplerAddressModeClampToEdge;
		break;
	case kWrapModeRepeat:
	default:
		_wrapMode = MTL::SamplerAddressModeRepeat;
		break;
	}
}

void MetalTexture::destroy() {
	_texture->release();
	_texture = nullptr;
}

void MetalTexture::create() {
	// Release old texture name in case it exists.
	destroy();

	// If a size is specified, allocate memory for it.
	if (_width != 0 && _height != 0) {
		// Allocate storage for Metal texture.
		MTL::TextureDescriptor *d = MTL::TextureDescriptor::alloc()->init();
		d->setWidth(_width);
		d->setHeight(_height);
		d->setPixelFormat(_pixelFormat);
		d->setUsage(_usage);
		_texture = _device->newTexture(d);
		d->release();
	}
}

bool MetalTexture::setSize(uint width, uint height) {
	const uint oldWidth  = _width;
	const uint oldHeight = _height;

	_logicalWidth  = width;
	_logicalHeight = height;

	_width = width;
	_height = height;

	// If a size is specified, allocate memory for it.
	if (width != 0 && height != 0) {
		const float texWidth = (float)width / _width;
		const float texHeight = (float)height / _height;

		_texCoords[0] = 0.0f;
		_texCoords[1] = texHeight;

		_texCoords[2] = texWidth;
		_texCoords[3] = texHeight;

		_texCoords[4] = texWidth;
		_texCoords[5] = 0.0f;

		_texCoords[6] = 0.0f;
		_texCoords[7] = 0.0f;

		// Allocate storage for OpenGL texture if necessary.
		if (oldWidth != _width || oldHeight != _height) {
			// Allocate storage for Metal texture.
			MTL::TextureDescriptor *d = MTL::TextureDescriptor::alloc()->init();
			d->setWidth(_width);
			d->setHeight(_height);
			d->setPixelFormat(_pixelFormat);
			d->setUsage(_usage);
			_texture = _device->newTexture(d);
			d->release();
		}
	}
	return true;
}

void MetalTexture::updateArea(const Common::Rect &area, const Graphics::Surface &src) {
	// Update the actual texture.
	_texture->replaceRegion(MTL::Region(area.left, area.top, 0, src.w - area.left, area.height(), 1), 0, src.getBasePtr(area.left, area.top), src.pitch);
}


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

Texture::Texture(MTL::Device *device, const MTL::PixelFormat metalPixelFormat, const Graphics::PixelFormat &format)
	: Surface(), _format(format), _device(device), _metalTexture(MetalTexture(device, metalPixelFormat)),
	  _textureData(), _userPixelData() {
}

Texture::~Texture() {
	_textureData.free();
}

void Texture::destroy() {
	_metalTexture.destroy();
}

void Texture::recreate() {
	// Need to release metal textures
	destroy();
	_metalTexture.create();

	// In case image date exists assure it will be completely refreshed next
	// time.
	if (_textureData.getPixels()) {
		flagDirty();
	}
}

void Texture::enableLinearFiltering(bool enable) {
	_metalTexture.enableLinearFiltering(enable);
}

void Texture::allocate(uint width, uint height) {
	// Assure the texture can contain our user data.
	_metalTexture.setSize(width, height);

	// In case the needed texture dimension changed we will reinitialize the
	// texture data buffer.
	if (_metalTexture.getWidth() != (uint)_textureData.w || _metalTexture.getHeight() != (uint)_textureData.h) {
		// Create a buffer for the texture data.
		_textureData.create(_metalTexture.getWidth(), _metalTexture.getHeight(), _format);
	}

	// Create a sub-buffer for raw access.
	_userPixelData = _textureData.getSubArea(Common::Rect(width, height));

	// The whole texture is dirty after we changed the size. This fixes
	// multiple texture size changes without any actual update in between.
	// Without this we might try to write a too big texture into the GL
	// texture.
	flagDirty();
}

void Texture::updateMetalTexture() {
	if (!isDirty()) {
		return;
	}

	Common::Rect dirtyArea = getDirtyArea();

	updateMetalTexture(dirtyArea);
}

void Texture::updateMetalTexture(Common::Rect &dirtyArea) {
	// In case we use linear filtering we might need to duplicate the last
	// pixel row/column to avoid glitches with filtering.
	if (_metalTexture.isLinearFilteringEnabled()) {
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
	_metalTexture.updateArea(dirtyArea, _textureData);
	// We should have handled everything, thus not dirty anymore.
	clearDirty();
}

} // end namespace Metal
