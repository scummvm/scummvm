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

#include "image/gif.h"

#include "common/array.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "graphics/surface.h"
#include "graphics/pixelformat.h"

#include <gif_lib.h>

namespace Image {

GIFDecoder::GIFDecoder() : _outputSurface(nullptr), _palette(0) {
}

GIFDecoder::~GIFDecoder() {
	destroy();
}

static int gifReadFromStream(GifFileType *gif, GifByteType *bytes, int size) {
	Common::SeekableReadStream *stream = (Common::SeekableReadStream *)gif->UserData;
	return stream->read(bytes, size);
}

bool GIFDecoder::loadStream(Common::SeekableReadStream &stream) {
	destroy();

	int error = 0;
	GifFileType *gif = DGifOpen(&stream, gifReadFromStream, &error);
	if (!gif) {
		warning("GIF open failed with error %s", GifErrorString(error));
		return false;
	}

	const int errcode = DGifSlurp(gif);
	if (errcode != GIF_OK) {
		warning("GIF failed to load");
		DGifCloseFile(gif, 0);
		return false;
	}

	if (gif->ImageCount <= 0) {
		warning("GIF doesn't contain valid image data");
		DGifCloseFile(gif, 0);
		return false;
	}

	if (gif->ImageCount > 1) {
		warning("GIF contains more than one frame - only loading the first one");
	}

	const SavedImage *gifImage = gif->SavedImages;

	const int width = gif->SWidth;
	const int height = gif->SHeight;

	const ColorMapObject *colorMap = gif->SColorMap;
	_hasTransparentColor = false;
	for (int i = 0; i < gif->ExtensionBlockCount; ++i) {
		const ExtensionBlock &eb = gif->ExtensionBlocks[i];
		GraphicsControlBlock gcb;
		DGifExtensionToGCB(eb.ByteCount, eb.Bytes, &gcb);
		if (gcb.TransparentColor != NO_TRANSPARENT_COLOR) {
			_hasTransparentColor = true;
			_transparentColor = gcb.TransparentColor;
			break;
		}
	}

	int colorCount = colorMap->ColorCount;
	_outputSurface = new Graphics::Surface();
	_palette.resize(colorCount, false);

	const Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();
	for (int i = 0; i < colorCount; ++i) {
		_palette.set(i, colorMap->Colors[i].Red,
						colorMap->Colors[i].Green,
						colorMap->Colors[i].Blue);
	}

	// TODO: support transparency

	_outputSurface->create(width, height, format);
	const uint8 *in = (const uint8 *)gifImage->RasterBits;
	uint8 *pixelPtr = (uint8 *)_outputSurface->getBasePtr(0, 0);
	if (gif->Image.Interlace) {
		const int interlacedOffset[] = {0, 4, 2, 1};
		const int interlacedJumps[] = {8, 8, 4, 2};
		for (int i = 0; i < 4; ++i) {
			for (int row = interlacedOffset[i]; row < height; row += interlacedJumps[i]) {
				memcpy(pixelPtr + width * row, in, width);
				in += width;
			}
		}
	} else {
		memcpy(pixelPtr, in, width * height);
	}

	DGifCloseFile(gif, 0);
	return true;
}

void GIFDecoder::destroy() {
	if (_outputSurface) {
		_outputSurface->free();
		delete _outputSurface;
		_outputSurface = 0;
	}
	_palette.clear();
}

} // End of namespace Image
