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

#include "tetraedge/tetraedge.h"

#include "common/endian.h"
#include "common/file.h"
#include "common/rect.h"
#include "tetraedge/te/te_core.h"
#include "tetraedge/te/te_image.h"
#include "tetraedge/te/te_i_codec.h"
#include "tetraedge/te/te_scummvm_codec.h"

namespace Tetraedge {

TeImage::TeImage() : ManagedSurface(), _teFormat(INVALID) {
}

TeImage::TeImage(const TeImage &other) : ManagedSurface(), _teFormat(other._teFormat) {
	copyFrom(other);
	error("TODO: Implement TeImage::TeImage copy constructor");
}

void TeImage::copy(TeImage &dest, const TeVector2s32 &vec1, const TeVector2s32 &vec2, const TeVector2s32 &vec3) const {
	error("TODO: Implement TeImage::copy");
}

uint64 TeImage::countPixelsOfColor(const TeColor &col) const {
	error("TODO: Implement TeImage::countPixelsOfColor");
}

/*
void TeImage::create() {
	// Never used, but in original seems to do the same as destroy??
	destroy();
}*/

void TeImage::createImg(uint xsize, uint ysize, Common::SharedPtr<TePalette> &pal,
			Format teformat, uint bufxsize, uint bufysize) {
	_teFormat = teformat;
#ifdef SCUMM_BIG_ENDIAN
	Graphics::PixelFormat pxformat = ((teformat == TeImage::RGB8) ?
									  Graphics::PixelFormat(3, 8, 8, 8, 0, 0, 8, 16, 0) : Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
#else
	Graphics::PixelFormat pxformat = ((teformat == TeImage::RGB8) ?
									  Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0) : Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));
#endif

	Graphics::ManagedSurface::create(xsize, ysize, pxformat);
	if (teformat == TeImage::RGBA8)
		Graphics::ManagedSurface::fillRect(Common::Rect(0, 0, xsize, ysize), 0);
}

void TeImage::deserialize(Common::ReadStream &stream) {
	error("TODO: Implement TeImage::deserialize");
}

void TeImage::destroy() {
	Graphics::ManagedSurface::free();
	_teFormat = INVALID;
}

void TeImage::drawPlot(void *outbuf, int x, int y, const TeVector2s32 &bufsize, const TeColor &col) {
	error("TODO: Implement TeImage::drawPlot");
}

void TeImage::fill(byte val) {
	error("TODO: Implement TeImage::fill");
}

void TeImage::fill(byte r, byte g, byte b, byte a) {
	Common::Rect wholeSurf(0, 0, w, h);

	uint32 col = ((uint32)r << format.rShift) | ((uint32)g << format.gShift) | ((uint32)b << format.bShift) | ((uint32)a << format.aShift);
	Graphics::ManagedSurface::fillRect(wholeSurf, col);
}

void TeImage::getBuff(uint x, uint y, byte *pout, uint w_, uint h_) {
	error("TODO: Implement TeImage::getBuff");
}

bool TeImage::isExtensionSupported(const Common::Path &path) {
	error("TODO: Implement TeImage::isExtensionSupported");
}

bool TeImage::load(const TetraedgeFSNode &node) {
	TeCore *core = g_engine->getCore();
	TeICodec *codec = core->createVideoCodec(node);
	if (!node.exists() || !codec->load(node)) {
		warning("TeImage::load: Failed to load %s.", node.toString().c_str());
		delete codec;
		return false;
	}

	Common::SharedPtr<TePalette> nullpal;
	createImg(codec->width(), codec->height(), nullpal, codec->imageFormat(), codec->width(), codec->height());

	if (!codec->update(0, *this)) {
		error("TeImage::load: Failed to update from %s.", node.toString().c_str());
	}
	delete codec;
	return true;
}

bool TeImage::load(Common::SeekableReadStream &stream, const Common::String &type) {
	TeCore *core = g_engine->getCore();
	TeScummvmCodec *codec = dynamic_cast<TeScummvmCodec *>(core->createVideoCodec(type));
	if (!codec || !codec->load(stream)) {
		warning("TeImage::load: Failed to load stream");
		delete codec;
		return false;
	}

	Common::SharedPtr<TePalette> nullpal;
	createImg(codec->width(), codec->height(), nullpal, codec->imageFormat(), codec->width(), codec->height());

	if (!codec->update(0, *this)) {
		error("TeImage::load: Failed to update from stream");
	}
	delete codec;
	return true;
}

bool TeImage::save(const Common::Path &path, enum SaveType type) {
	error("TODO: Implement TeImage::save");
}

int TeImage::serialize(Common::WriteStream &stream) {
	error("TODO: Implement TeImage::serialize");
}

} // end namespace Tetraedge
