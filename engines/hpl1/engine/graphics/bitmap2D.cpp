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
#include "hpl1/debug.h"
#include "common/rect.h"
#include "image/png.h"
#include "image/jpeg.h"
#include "image/tga.h"
#include "image/bmp.h"
#include "image/gif.h"
#include "common/file.h"

namespace hpl {

template<typename Loader>
static Loader *loadImage(const tString &filepath) {
	//FIXME: string types 
	Common::File imgFile; 
	if (!imgFile.open(filepath.c_str())) {
		error("Could not open file: %s", filepath.c_str());
		return nullptr; 
	}
	Loader *imgLoader = new Loader(); 
	if (!imgLoader->loadStream(imgFile)) {
		error("Could not load image at %s", filepath.c_str());
	}
	return imgLoader; 
}

Bitmap2D::Bitmap2D(const tString &filepath, const tString &type) 
	: iLowLevelPicture(type), isSurfaceActive(false) {
	if (type == "png")
		_decoder.reset(loadImage<Image::PNGDecoder>(filepath)); 
	if (type == "bmp")
		_decoder.reset(loadImage<Image::BitmapDecoder>(filepath));
	if (type == "tga")
		_decoder.reset(loadImage<Image::TGADecoder>(filepath));
	if (type == "jpg" || type == "jpeg")
		_decoder.reset(loadImage<Image::JPEGDecoder>(filepath));
	if (type == "gif")
		_decoder.reset(loadImage<Image::GIFDecoder>(filepath));

	error("trying to load unsupported image format %s", type.c_str());
}

Bitmap2D::Bitmap2D(const tString &type, const cVector2l &size, const Graphics::PixelFormat &format) 
: iLowLevelPicture(type), isSurfaceActive(true) {
	create(size, format);
}


void Bitmap2D::drawToBitmap(Bitmap2D &dest, const cVector2l &pos) {
	if(!dest.isSurfaceActive)
		dest.copyDecoder();
	
	if (activeSurface().format != dest._surface.format)
		error("call to Bitmap2D::drawToBitmap with different pixel formats"); 
	if (activeSurface().w + pos.x > dest._surface.w || activeSurface().h + pos.y > dest._surface.h)
		error("call to Bitmap2D::drawToBitmap would go out of bounds");

	dest._surface.copyRectToSurface(activeSurface(), pos.x, pos.y, Common::Rect(0, 0, activeSurface().w, activeSurface().h));
}

bool Bitmap2D::create(const cVector2l &size, const Graphics::PixelFormat &format) {
	_surface.create(size.x, size.y, format);
	isSurfaceActive = true;
	_decoder.release(); 
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
	if(!isSurfaceActive)
		copyDecoder();
	
	const uint32 qcol = quantizeColor(color, _surface.format);
	const Common::Rect surfaceRect(
		rect.x, 
		rect.y,
		rect.w <= 0 ? rect.x + _surface.w : rect.x + rect.w, 
		rect.h <= 0 ? rect.y + _surface.w : rect.y + rect.h
	);  
	_surface.fillRect(surfaceRect, qcol);
}

const void *Bitmap2D::getRawData() const {
	return activeSurface().getPixels();
}

int Bitmap2D::getNumChannels() {
	return activeSurface().format.bpp() / _surface.format.bytesPerPixel;
}

bool Bitmap2D::HasAlpha() {
	return activeSurface().format.aBits() > 0; 
}

void Bitmap2D::copyDecoder() {
	_surface.copyFrom(*( _decoder->getSurface())); 
	isSurfaceActive = true;
	_decoder.release(); 
}

const Graphics::Surface &Bitmap2D::activeSurface() const {
	if(isSurfaceActive)
		return _surface;
	return *(_decoder->getSurface());
}

Bitmap2D::~Bitmap2D() {
	if(isSurfaceActive)
		_surface.free();
}


} // namespace hpl
