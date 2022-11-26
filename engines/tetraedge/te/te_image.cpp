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

#include "common/rect.h"
#include "tetraedge/te/te_core.h"
#include "tetraedge/te/te_image.h"
#include "tetraedge/te/te_i_codec.h"

namespace Tetraedge {

TeImage::TeImage() : Surface(), _format(INVALID) {
}

TeImage::TeImage(const TeImage &other) {
	error("TODO: TeImage:: copy constructor Implement me.");
}

void TeImage::copy(TeImage &dest, const TeVector2s32 &vec1, const TeVector2s32 &vec2, const TeVector2s32 &vec3) const {
	error("TODO: TeImage::copy Implement me.");
}

unsigned long TeImage::countPixelsOfColor(const TeColor &col) const {
	error("TODO: TeImage: Implement me.");
}

void TeImage::create() {
	_format = INVALID;
	Graphics::Surface::free();
}

void TeImage::create(uint xsize, uint ysize, Common::SharedPtr<TePalette> &pal,
			Format teformat, uint bufxsize, uint bufysize) {
	_format = teformat;
	Graphics::PixelFormat pxformat = ((teformat == TeImage::RGB8) ?
									  Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0) : Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));

	Graphics::Surface::create(xsize, ysize, pxformat);
	Graphics::Surface::fillRect(Common::Rect(0, 0, xsize, ysize), 0);
}

void TeImage::deserialize(Common::ReadStream &stream) {
	error("TODO: TeImage: Implement me.");
}

void TeImage::destroy() {
	Graphics::Surface::free();
	_format = INVALID;
}

void TeImage::drawPlot(void *outbuf, int x, int y, const TeVector2s32 &bufsize, const TeColor &col) {
	error("TODO: TeImage: Implement me.");
}

void TeImage::fill(byte val) {
	error("TODO: TeImage: Implement me.");
}

void TeImage::fill(byte r, byte g, byte b, byte a) {
	Common::Rect wholeSurf(0, 0, w, h);

	uint32 col = ((uint32)r << format.rShift) | ((uint32)g << format.gShift) | ((uint32)b << format.bShift) | (uint32)a << format.aShift;
	Graphics::Surface::fillRect(wholeSurf, col);
}

void TeImage::getBuff(uint x, uint y, byte *pout, uint w_, uint h_) {
	error("TODO: TeImage: Implement me.");
}

bool TeImage::isExtensionSupported(const Common::Path &path) {
	error("TODO: TeImage: Implement me.");
}

bool TeImage::load(const Common::Path &path) {
	TeCore *core = g_engine->getCore();
	TeICodec *codec = core->createVideoCodec(path);
	if (!codec->load(path)) {
		error("TeImage::load: Failed to load %s.", path.toString().c_str());
	}

	Common::SharedPtr<TePalette> nullpal;
	create(codec->width(), codec->height(), nullpal, codec->imageFormat(), codec->width(), codec->height());

	if (!codec->update(0, *this)) {
		error("TeImage::load: Failed to update from %s.", path.toString().c_str());
	}
	return true;
}

bool TeImage::load(Common::ReadStream &stream, const Common::Path &path) {
	error("TODO: TeImage::load Implement me.");
}

bool TeImage::save(const Common::Path &path, enum Type type) {
	error("TODO: TeImage::save Implement me.");
}

int TeImage::serialize(Common::WriteStream &stream) {
	error("TODO: TeImage::serialize Implement me.");
}

} // end namespace Tetraedge
