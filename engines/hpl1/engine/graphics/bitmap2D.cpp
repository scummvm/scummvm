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

#include "hpl1/engine/graphics/bitmap2D.h"
#include "common/file.h"
#include "common/rect.h"
#include "hpl1/debug.h"
#include "image/bmp.h"
#include "image/gif.h"
#include "image/jpeg.h"
#include "image/png.h"
#include "image/tga.h"

namespace hpl {

static Image::ImageDecoder *loadImage(const tString &filepath, Image::ImageDecoder *decoder) {
	Common::File imgFile;
	if (!imgFile.open(Common::Path(filepath)))
		error("Could not open file: %s", filepath.c_str());
	if (!decoder->loadStream(imgFile))
		error("Could not load image at %s", filepath.c_str());
	return decoder;
}

Image::JPEGDecoder *setupJPEGDecoder(Image::JPEGDecoder *jpeg) {
#ifdef SCUMM_BIG_ENDIAN
	jpeg->setOutputPixelFormat(Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
#else
	jpeg->setOutputPixelFormat(Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));
#endif
	return jpeg;
}

Bitmap2D::Bitmap2D(const tString &filepath, const tString &type, const Graphics::PixelFormat &desiredFormat)
	: LowLevelPicture(type), _isSurfaceActive(false) {
	if (type == "png")
		_decoder.reset(loadImage(filepath, new Image::PNGDecoder));
	else if (type == "bmp")
		_decoder.reset(loadImage(filepath, new Image::BitmapDecoder));
	else if (type == "tga")
		_decoder.reset(loadImage(filepath, new Image::TGADecoder));
	else if (type == "jpg" || type == "jpeg")
		_decoder.reset(loadImage(filepath, setupJPEGDecoder(new Image::JPEGDecoder)));
#ifdef USE_GIF
	else if (type == "gif")
		_decoder.reset(loadImage(filepath, new Image::GIFDecoder));
#endif
	else
		error("trying to load unsupported image format %s", type.c_str());
	_width = _decoder->getSurface()->w;
	_height = _decoder->getSurface()->h;
	if (desiredFormat.bytesPerPixel != 0 && desiredFormat != _decoder->getSurface()->format)
		copyDecoder(desiredFormat);
}

Bitmap2D::Bitmap2D(const cVector2l &size, const Graphics::PixelFormat &format)
	: LowLevelPicture("none"), _isSurfaceActive(true) {
	create(size, format);
}

void Bitmap2D::drawToBitmap(Bitmap2D &dest, const cVector2l &at, Common::Rect srcSubrect) {
	if (!dest._isSurfaceActive)
		dest.copyDecoder();
	if (dest._surface.w == 0 || dest._surface.h == 0 || activeSurface().w == 0 || activeSurface().h == 0)
		return; // font loading can use bitmaps with 0 width
	if (srcSubrect.right == 0 && srcSubrect.bottom == 0)
		srcSubrect = Common::Rect(activeSurface().w, activeSurface().h);

	if (activeSurface().format != dest._surface.format)
		error("call to Bitmap2D::drawToBitmap with different pixel formats");
	if (srcSubrect.width() > dest._surface.w || srcSubrect.height() > dest._surface.h)
		error("call to Bitmap2D::drawToBitmap would go out of bounds");

	dest._surface.copyRectToSurface(activeSurface(), at.x, at.y, srcSubrect);
}

bool Bitmap2D::create(const cVector2l &size, const Graphics::PixelFormat &format) {
	_surface.create(size.x, size.y, format);
	_isSurfaceActive = true;
	_decoder.release();
	_width = size.x;
	_height = size.y;
	return true;
}

static uint32 quantizeColor(const cColor &col, const Graphics::PixelFormat &format) {
	uint8 r = static_cast<uint8>(col.r * 255);
	uint8 g = static_cast<uint8>(col.g * 255);
	uint8 b = static_cast<uint8>(col.b * 255);
	uint8 a = static_cast<uint8>(col.a * 255);
	return format.ARGBToColor(a, r, g, b);
}

void Bitmap2D::fillRect(const cRect2l &rect, const cColor &color) {
	if (!_isSurfaceActive)
		copyDecoder();

	const uint32 qcol = quantizeColor(color, _surface.format);
	const Common::Rect surfaceRect(
		rect.x,
		rect.y,
		rect.w <= 0 ? rect.x + _surface.w : rect.x + rect.w,
		rect.h <= 0 ? rect.y + _surface.w : rect.y + rect.h);
	_surface.fillRect(surfaceRect, qcol);
}

const void *Bitmap2D::getRawData() const {
	return activeSurface().getPixels();
}

int Bitmap2D::getNumChannels() {
	const Graphics::PixelFormat &format = activeSurface().format;
	return (format.rBits() > 0) + (format.gBits() > 0) + (format.bBits() > 0) + (format.aBits() > 0);
}

const Graphics::PixelFormat &Bitmap2D::format() const {
	return activeSurface().format;
}

bool Bitmap2D::hasAlpha() {
	return activeSurface().format.aBits() > 0;
}

uint32 Bitmap2D::getBpp() const {
	return format().bytesPerPixel;
}

static uint32 alphaToRed(const uint32 pixel, const Graphics::PixelFormat &format) {
	byte r, g, b;
	format.colorToRGB(pixel, r, g, b);
	byte a = r;
	return format.ARGBToColor(a, r, g, b);
}

void Bitmap2D::copyRedToAlpha() {
	if (!_isSurfaceActive)
		copyDecoder();
	for (int16 y = 0; y < _surface.h; y++) {
		uint32 *rowPtr = (uint32 *)_surface.getBasePtr(0, y);
		for (int16 x = 0; x < _surface.w; ++x, ++rowPtr) {
			*rowPtr = alphaToRed(*rowPtr, _surface.format);
		}
	}
}

void Bitmap2D::copyDecoder(const Graphics::PixelFormat &format) {
	_surface.copyFrom(*(_decoder->getSurface()));
	if (format.bytesPerPixel != 0)
		_surface.convertToInPlace(format);
	_isSurfaceActive = true;
	_decoder.reset(nullptr);
}

const Graphics::Surface &Bitmap2D::activeSurface() const {
	if (_isSurfaceActive)
		return _surface;
	return *(_decoder->getSurface());
}

Bitmap2D::~Bitmap2D() {
	if (_isSurfaceActive)
		_surface.free();
}

} // namespace hpl
