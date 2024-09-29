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

#include "freescape/freescape.h"

namespace Freescape {

void FreescapeEngine::renderPixels8bitBinImage(Graphics::ManagedSurface *surface, int row, int column, int pixels, int bit) {
	int mask = 0x80;
	for (int i = 0; i < 8; i++) {
		if (column + i >= _screenW)
			continue;
		if ((pixels & mask) > 0) {
			int sample = surface->getPixel(column + i, row) | bit;
			surface->setPixel(column + i, row, sample);
		}
		mask >>= 1;
	}
}

int FreescapeEngine::execute8bitBinImageSingleCommand(Common::SeekableReadStream *file, Graphics::ManagedSurface *surface, int row, int column, int bit, int count) {
	int pixel = file->readByte();
	for (int i = 0; i < count; i++) {
		renderPixels8bitBinImage(surface, row, column + 8 * i, pixel, bit);
	}
	return 8 * count;
}

int FreescapeEngine::execute8bitBinImageMultiCommand(Common::SeekableReadStream *file, Graphics::ManagedSurface *surface, int row, int column, int bit, int count) {
	for (int i = 0; i < count; i++) {
		int pixel = file->readByte();
		renderPixels8bitBinImage(surface, row, column + 8 * i, pixel, bit);
	}
	return 8 * count;
}

int FreescapeEngine::execute8bitBinImageCommand(Common::SeekableReadStream *file, Graphics::ManagedSurface *surface, int row, int column, int bit) {
	int code = file->readByte();
	if (code >= 0x80) {
		int count = 257 - code;
		return execute8bitBinImageSingleCommand(file, surface, row, column, bit, count);
	} else {
		int count = code + 1;
		return execute8bitBinImageMultiCommand(file, surface, row, column, bit, count);
	}
}

void FreescapeEngine::load8bitBinImageRowIteration(Common::SeekableReadStream *file, Graphics::ManagedSurface *surface, int row, int bit) {
	int pixels = 0;
	while (pixels < surface->w) {
		pixels += execute8bitBinImageCommand(file, surface, row, pixels, bit);
	}
}

void FreescapeEngine::load8bitBinImageRow(Common::SeekableReadStream *file, Graphics::ManagedSurface *surface, int row) {
	int bit = 1;
	int nBits = 0;
	if (_renderMode == Common::kRenderCGA)
		nBits = 2;
	else if (_renderMode == Common::kRenderEGA)
		nBits = 4;
	else if (_renderMode == Common::kRenderHercG)
		nBits = 1;
	else
		error("Unimplemented render mode for reading images");

	for (int i = 0; i < nBits; i++) {
		load8bitBinImageRowIteration(file, surface, row, bit);
		bit <<= 1;
	}
}

Graphics::ManagedSurface *FreescapeEngine::load8bitBinImage(Common::SeekableReadStream *file, int offset) {
	Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
	surface->create(_screenW, _screenH, Graphics::PixelFormat::createFormatCLUT8());
	surface->fillRect(Common::Rect(0, 0, _screenW, _screenH), 0);

	file->seek(offset);
	int imageSize = file->readUint16BE();
	int startImage = file->pos();

	for (int row = 0; row < surface->h; row++)
		load8bitBinImageRow(file, surface, row);

	assert(startImage + imageSize == file->pos());
	debugC(1, kFreescapeDebugParser, "Last position: %" PRIx64, file->pos());
	return surface;
}


} // namespace Freescape
