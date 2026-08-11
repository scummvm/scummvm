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

#ifndef IMAGE_CODECS_MSMPEG4_H
#define IMAGE_CODECS_MSMPEG4_H

#include "image/codecs/codec.h"

namespace Image {

class MSMPEG4Decoder : public Codec {
public:
	struct Context;

	MSMPEG4Decoder(int width, int height, int bitsPerPixel);
	~MSMPEG4Decoder() override;

	const Graphics::Surface *decodeFrame(Common::SeekableReadStream &stream) override;
	Graphics::PixelFormat getPixelFormat() const override { return _pixelFormat; }
	bool setOutputPixelFormat(const Graphics::PixelFormat &format) override;

private:
	bool initialize();
	bool decodePictureHeader();
	bool decodeMacroblock(int16 block[6][64]);
	bool decodeBlock(int16 *block, int blockIndex, bool coded);
	int decodeDc(int blockIndex, int *direction);
	bool decodeMotion(int *motionX, int *motionY);
	void decodeMacroblockPixels(int16 block[6][64]);
	void frameStart();
	void frameEnd();
	void resetPredictorsForSlice();
	void drawEdges(byte *plane, int pitch, int width, int height, int edge);
	void convertToSurface();

	Context *_ctx;
	Graphics::Surface *_surface;
	Graphics::PixelFormat _pixelFormat;
	int _width;
	int _height;
	int _bitsPerPixel;
	bool _tablesInitialized;
};

} // End of namespace Image

#endif
