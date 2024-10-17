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

#ifndef IMAGE_PICT_H
#define IMAGE_PICT_H

#include "common/array.h"
#include "common/rect.h"
#include "common/scummsys.h"

#include "image/image_decoder.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct Surface;
}

namespace Image {

/**
 * @defgroup image_pict PICT decoder
 * @ingroup image
 *
 * @brief Decoder for PICT images.
 *
 * Used in engines:
 * - Mohawk
 * - Pegasus
 * - SCI
 * @{
 */

#define DECLARE_OPCODE(x) void x(Common::SeekableReadStream &stream)

class PICTDecoder : public ImageDecoder {
public:
	PICTDecoder();
	~PICTDecoder();

	// ImageDecoder API
	bool loadStream(Common::SeekableReadStream &stream);
	void destroy();
	const Graphics::Surface *getSurface() const { return _outputSurface; }
	const byte *getPalette() const { return _palette; }
	int getPaletteSize() const { return 256; }
	uint16 getPaletteColorCount() const { return _paletteColorCount; }

	struct PixMap {
		uint32 baseAddr;
		uint16 rowBytes;
		Common::Rect bounds;
		uint16 pmVersion;
		uint16 packType;
		uint32 packSize;
		uint32 hRes;
		uint32 vRes;
		uint16 pixelType;
		uint16 pixelSize;
		uint16 cmpCount;
		uint16 cmpSize;
		uint32 planeBytes;
		uint32 pmTable;
		uint32 pmReserved;
	};

	static PixMap readRowBytes(Common::SeekableReadStream &stream, bool hasBaseAddr = true);
	static PixMap readPixMap(Common::SeekableReadStream &stream, bool hasBaseAddr = true, bool hasRowBytes = true);

private:
	Common::Rect _imageRect;
	byte _palette[256 * 3];
	uint16 _paletteColorCount;
	byte _penPattern[8];
	Common::Point _currentPenPosition;
	Graphics::Surface *_outputSurface;
	bool _continueParsing;
	int _version;

	// Utility Functions
	void unpackBitsRectOrRgn(Common::SeekableReadStream &stream, bool compressed, bool hasRegion);
	void unpackBits(Common::SeekableReadStream &stream, bool compressed, bool hasRegion);
	void unpackBitsRect(Common::SeekableReadStream &stream, bool withPalette, PixMap pixMap);
	void unpackBitsLine(byte *out, uint32 length, Common::SeekableReadStream *stream, byte bitsPerPixel, byte bytesPerPixel);
	void skipBitsRect(Common::SeekableReadStream &stream, bool withPalette);
	void decodeCompressedQuickTime(Common::SeekableReadStream &stream);
	void outputPixelBuffer(byte *&out, byte value, byte bitsPerPixel);

	// Opcodes
	typedef void (PICTDecoder::*OpcodeProcPICT)(Common::SeekableReadStream &stream);
	struct PICTOpcode {
		PICTOpcode() { op = 0; proc = 0; desc = 0; }
		PICTOpcode(uint16 o, OpcodeProcPICT p, const char *d) { op = o; proc = p; desc = d; }
		uint16 op;
		OpcodeProcPICT proc;
		const char *desc;
	};
	Common::Array<PICTOpcode> _opcodes;

	// Common Opcodes
	void setupOpcodesCommon();
	DECLARE_OPCODE(o_nop);
	DECLARE_OPCODE(o_clip);
	DECLARE_OPCODE(o_txFont);
	DECLARE_OPCODE(o_txFace);
	DECLARE_OPCODE(o_pnSize);
	DECLARE_OPCODE(o_pnPat);
	DECLARE_OPCODE(o_txSize);
	DECLARE_OPCODE(o_txRatio);
	DECLARE_OPCODE(o_versionOp);
	DECLARE_OPCODE(o_shortLine);
	DECLARE_OPCODE(o_shortLineFrom);
	DECLARE_OPCODE(o_longText);
	DECLARE_OPCODE(o_bitsRgn);
	DECLARE_OPCODE(o_packBitsRgn);
	DECLARE_OPCODE(o_shortComment);
	DECLARE_OPCODE(o_longComment);
	DECLARE_OPCODE(o_opEndPic);
	DECLARE_OPCODE(o_headerOp);
	DECLARE_OPCODE(o_versionOp1);

	// Regular-mode Opcodes
	void setupOpcodesNormal();
	DECLARE_OPCODE(on_bitsRect);
	DECLARE_OPCODE(on_packBitsRect);
	DECLARE_OPCODE(on_directBitsRect);
	DECLARE_OPCODE(on_compressedQuickTime);

	// QuickTime-mode Opcodes
	void setupOpcodesQuickTime();
	DECLARE_OPCODE(oq_packBitsRect);
	DECLARE_OPCODE(oq_directBitsRect);
	DECLARE_OPCODE(oq_compressedQuickTime);
};

#undef DECLARE_OPCODE
/** @} */
} // End of namespace Image

#endif
