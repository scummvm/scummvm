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

#ifndef FOOL_ZBASIC_H
#define FOOL_ZBASIC_H

#include "common/str.h"
#include "common/ustr.h"
#include "graphics/surface.h"

#include "fool/toolbox.h"

namespace Fool {

union DoubleBits {
	uint64 i;
	double d;
};

struct ZBasicBCD {
	uint16 exponent = 0;
	uint64 mantissaBits = 0;
	uint8 mantissaSize = 0;

	ZBasicBCD() {}

	ZBasicBCD(uint16 e, uint64 mb, uint8 ms) {
		this->exponent = e;
		this->mantissaBits = mb;
		this->mantissaSize = ms;
	}

	ZBasicBCD(const ZBasicBCD &obj) {
		this->exponent = obj.exponent;
		this->mantissaBits = obj.mantissaBits;
		this->mantissaSize = obj.mantissaSize;
	}

	static ZBasicBCD read(Common::SeekableReadStream *stream) {
		ZBasicBCD bcd = { 0, 0, 0 };
		uint8 size = stream->readByte();
		if (size == 0) {
			// pass
		} else if (size == 4) {
			bcd.exponent = stream->readUint16BE();
			bcd.mantissaBits = stream->readUint16BE();
			bcd.mantissaSize = 16;
		} else if (size == 6) {
			bcd.exponent = stream->readUint16BE();
			bcd.mantissaBits = stream->readUint32BE();
			bcd.mantissaSize = 32;
		} else if (size == 8) {
			bcd.exponent = stream->readUint16BE();
			bcd.mantissaBits = stream->readUint32BE() << 16;
			bcd.mantissaBits += stream->readUint16BE();
			bcd.mantissaSize = 48;
		} else {
			warning("ZBasicBCD::read: unexpected size for bcd %d", size);
			stream->skip(size);
		}
		return bcd;
	}

	double toDouble() {
		DoubleBits result = { 0 };
		int64 expSigned = this->exponent <= 0x3fff ? (int64)this->exponent : ((int64)this->exponent - 0x8000);
		if (expSigned > 0x7ff) {
			result.i = 0x7ff0000000000000; // +inf
		} else if (expSigned < -0x800) {
			result.i = 0xfff0000000000000; // -inf
		} else {
			int sign = expSigned < 0 ? -1 : 1;
			uint64 mantissaRaw = this->mantissaBits;
			for (int i = 0; i < this->mantissaSize; i += 4) {
				if (i != 0) {
					result.d /= 10;
				}
				result.d += (double)(mantissaRaw & 0xf);
				mantissaRaw >>= 4;
			}
			result.d *= sign * pow((double)10, (double)expSigned);
		}
		return result.d;
	}
};

enum ZBasicTextFont {
	kFontSystem = 0,
	kFontApplication = 1,
	kFontNewYork = 2,
	kFontGeneva = 3,
	kFontMonaco = 4,
	kFontVenice = 5,
	kFontLondon = 6,
	kFontAthens = 7,
	kFontSanFrancisco = 8,
	kFontToronto = 9,
	kFontSeattle = 10,
	kFontCairo = 11,
	kFontLosAngeles = 12
};

enum ZBasicTextFace {
	kFacePlain = 0,
	kFaceBold = 1,
	kFaceItalic = 2,
	kFaceUnderlined = 4,
	kFaceOutlined = 8,
	kFaceShadow = 16,
	kFaceCondensed = 32,
	kFaceExtended = 64
};

enum ZBasicTextMode {
	kModeSourceCOPY = 0,
	kModeSourceOR = 1,
	kModeSourceXOR = 2,
	kModeSourceBIC = 3,
	kModeNOTSourceCOPY = 4,
	kModeNOTSourceOR = 5,
	kModeNOTSourceXOR = 6,
	kModeNOTSourceBIC = 7
};

enum ZBasicDatumType {
	kDatumNULL = 0,
	kDatumINT = 1,
	kDatumDBLINT = 2,
	kDatumBCD = 4,
	kDatumSTR = 8,
};

enum ZBasicWindowType {
	kWindowDocument = 1,
	kWindowDialogTwoLine = 2,
	kWindowDialogOneLine = 3,
	kWindowShadow = 4,
	kWindowDocumentFixed = 5,
};

struct ZBasicDatum {
	ZBasicDatumType type = kDatumNULL;
	uint32 offset = 0;
	union {
		ZBasicBCD *bcd = 0;
		int16 i16;
		int32 i32;
		Common::U32String *str;
	} data;

	~ZBasicDatum() {
		if (this->type == kDatumBCD) {
			delete this->data.bcd;
			this->data.bcd = 0;
		} else if (this->type == kDatumSTR) {
			delete this->data.str;
			this->data.str = 0;
		}
	}

	static ZBasicDatum newNull(uint32 offset) {
		ZBasicDatum result;
		result.type = kDatumNULL;
		result.offset = offset;
		result.data.bcd = 0;
		return result;
	}

	static ZBasicDatum newInt(uint32 offset, int16 value) {
		ZBasicDatum result;
		result.type = kDatumINT;
		result.offset = offset;
		result.data.i16 = value;
		return result;
	}

	static ZBasicDatum newDblInt(uint32 offset, int32 value) {
		ZBasicDatum result;
		result.type = kDatumDBLINT;
		result.offset = offset;
		result.data.i32 = value;
		return result;
	}

	static ZBasicDatum newBcd(uint32 offset, const ZBasicBCD &value) {
		ZBasicDatum result;
		result.type = kDatumBCD;
		result.offset = offset;
		result.data.bcd = new ZBasicBCD(value);
		return result;
	}

	static ZBasicDatum newStr(uint32 offset, const Common::U32String &value) {
		ZBasicDatum result;
		result.type = kDatumSTR;
		result.offset = offset;
		result.data.str = new Common::U32String(value);
		return result;
	}
};

class ZBasic {

private:
	Common::Array<ZBasicDatum> _dataTable;
	uint32 _dataPtr;
	Common::Array<ZBasicDatum> _stringTable;

public:

	void loadProgram(const Common::Path &path);
	void loadSCOT();

	void bufferFlush(const Common::U32String &str);
	void blockMove(void *srcptr, void *destptr, uint16 size) {
		memmove(destptr, srcptr, size);
	}
	void get(int16 x1, int16 y1, int16 x2, int16 y2, Graphics::Surface &dest);
	int16 instr(int16 expression, const Common::U32String &string1, const Common::U32String &string2);
	bool maybe();
	uint32 mem(int16 index);
	void put(int16 x1, int16 y1, int16 x2, int16 y2, Graphics::Surface &src);
	void text(int16 font, int16 size, int16 face, ZBasicTextMode mode);
	void picture(int16 x1, int16 y1, int16 x2, int16 y2, PicHandle &src);
	int16 rndInt(int16 max);
	void window(int16 windowNumber, const Common::String &title, int16 x1, int16 y1, int16 x2, int16 y2, ZBasicWindowType type);

	const Common::U32String &str(size_t index) {
		return *_stringTable[index].data.str;
	}

	int16 readInt();
	Common::U32String readStr();

};

} // namespace Fool

#endif
