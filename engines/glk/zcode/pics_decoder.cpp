/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software{} you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation{} either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program{} if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/zcode/pics_decoder.h"
#include "glk/zcode/pics.h"
#include "common/memstream.h"

namespace Glk {
namespace ZCode {

#define MAX_BIT 512		/* Must be less than or equal to CODE_TABLE_SIZE */
#define CODE_SIZE 8
#define CODE_TABLE_SIZE 4096
#define PREFIX 0
#define PIXEL 1

/**
 * Support class used for picture decompression
 */
class Compress {
private:
	byte _codeBuffer[CODE_TABLE_SIZE];
public:
	short _nextCode;
	short _sLen;
	short _sPtr;
	short _tLen;
	short _tPtr;

	Compress() : _nextCode(0), _sLen(0), _sPtr(0), _tLen(0), _tPtr(0) {}

	/**
	 * Read a code
	 */
	short readCode(Common::ReadStream &src);
};

static short MASK[16] = {
	0x0000, 0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f,
	0x00ff, 0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff
};

short Compress::readCode(Common::ReadStream &src) {
	short code, bsize, tlen, tptr;

	code = 0;
	tlen = _tLen;
	tptr = 0;

	while (tlen) {
		if (_sLen == 0) {
			if ((_sLen = src.read(_codeBuffer, MAX_BIT)) == 0) {
				error("fread");
			}
			_sLen *= 8;
			_sPtr = 0;
		}
		bsize = ((_sPtr + 8) & ~7) - _sPtr;
		bsize = (tlen > bsize) ? bsize : tlen;
		code |= (((uint)_codeBuffer[_sPtr >> 3] >> (_sPtr & 7)) & MASK[bsize]) << tptr;

		tlen -= bsize;
		tptr += bsize;
		_sLen -= bsize;
		_sPtr += bsize;
	}

	if ((_nextCode == MASK[_tLen]) && (_tLen < 12))
		_tLen++;

	return code;
}

/*--------------------------------------------------------------------------*/

PictureDecoder::PictureDecoder() {
	_tableVal = new byte[3 * 3840];
	_tableRef = (uint16 *)(_tableVal + 3840);
}

PictureDecoder::~PictureDecoder() {
	delete[] _tableVal;
}

Common::SeekableReadStream *PictureDecoder::decode(Common::ReadStream &src, uint flags,
		const Common::Array<byte> &palette, uint display, size_t width, size_t height) {
	Common::MemoryWriteStreamDynamic out(DisposeAfterUse::NO);
	short code_table[CODE_TABLE_SIZE][2];
	byte buffer[CODE_TABLE_SIZE];

	// Write out dimensions
	out.writeUint16LE(width);
	out.writeUint16LE(height);

	// Write out palette
	out.writeUint16LE(palette.size() / 3 + 2);
	for (int idx = 0; idx < 6; ++idx)
		out.writeByte((idx < 3) ? 0x77 : 0);
	if (!palette.empty())
		out.write(&palette[0], palette.size());

	byte transparent = 0xff;
	if (flags & 1)
		transparent = flags >> 12;
	out.writeByte(transparent);

	int i;
	short code, old = 0, first, clear_code;
	Compress comp;

	clear_code = 1 << CODE_SIZE;
	comp._nextCode = clear_code + 2;
	comp._tLen = CODE_SIZE + 1;
	comp._tPtr = 0;

	for (i = 0; i < CODE_TABLE_SIZE; i++) {
		code_table[i][PREFIX] = CODE_TABLE_SIZE;
		code_table[i][PIXEL] = i;
	}

	for (;;) {
		if ((code = comp.readCode(src)) == (clear_code + 1))
			break;
		if (code == clear_code) {
			comp._tLen = CODE_SIZE + 1;
			comp._nextCode = clear_code + 2;
			code = comp.readCode(src);
		} else {
			first = (code == comp._nextCode) ? old : code;
			while (code_table[first][PREFIX] != CODE_TABLE_SIZE)
				first = code_table[first][PREFIX];
			code_table[comp._nextCode][PREFIX] = old;
			code_table[comp._nextCode++][PIXEL] = code_table[first][PIXEL];
		}
		old = code;
		i = 0;
		do
			buffer[i++] = (unsigned char)code_table[code][PIXEL];
		while ((code = code_table[code][PREFIX]) != CODE_TABLE_SIZE);
		do
			out.writeByte(buffer[--i]);
		while (i > 0);
	}

	return new Common::MemoryReadStream(out.getData(), out.size(), DisposeAfterUse::YES);
}

} // End of namespace ZCode
} // End of namespace Glk
