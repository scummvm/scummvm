/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GRAPHICS_PICT_H
#define GRAPHICS_PICT_H

#include "common/array.h"
#include "common/rect.h"
#include "common/scummsys.h"

#include "graphics/pixelformat.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {

class JPEG;
struct Surface;

#define DECLARE_OPCODE(x) void x(Common::SeekableReadStream *stream)

class PictDecoder {
public:
	PictDecoder(Graphics::PixelFormat pixelFormat);
	~PictDecoder();
	Surface *decodeImage(Common::SeekableReadStream *stream, byte *palette = 0);

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

	static PixMap readPixMap(Common::SeekableReadStream *stream, bool hasBaseAddr = true);

private:
	Common::Rect _imageRect;
	PixelFormat _pixelFormat;
	JPEG *_jpeg;
	byte _palette[256 * 3];
	bool _isPaletted;
	Graphics::Surface *_outputSurface;
	bool _continueParsing;

	// Utility Functions
	void unpackBitsRect(Common::SeekableReadStream *stream, bool hasPalette);
	void unpackBitsLine(byte *out, uint32 length, Common::SeekableReadStream *data, byte bitsPerPixel, byte bytesPerPixel);
	void skipBitsRect(Common::SeekableReadStream *stream, bool hasPalette);
	void decodeCompressedQuickTime(Common::SeekableReadStream *stream);
	void outputPixelBuffer(byte *&out, byte value, byte bitsPerPixel);

	// Opcodes
	typedef void (PictDecoder::*OpcodeProcPICT)(Common::SeekableReadStream *stream);
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
	DECLARE_OPCODE(o_txSize);
	DECLARE_OPCODE(o_txRatio);
	DECLARE_OPCODE(o_versionOp);
	DECLARE_OPCODE(o_longText);
	DECLARE_OPCODE(o_longComment);
	DECLARE_OPCODE(o_opEndPic);
	DECLARE_OPCODE(o_headerOp);

	// Regular-mode Opcodes
	void setupOpcodesNormal();
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

} // End of namespace Graphics

#endif
