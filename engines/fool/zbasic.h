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

#include "common/memorypool.h"
#include "common/memstream.h"
#include "common/ptr.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/ustr.h"
#include "graphics/macgui/macwindow.h"
#include "graphics/mactoolbox/toolbox.h"

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
			result.d = INFINITY;
		} else if (expSigned < -0x800) {
			result.d = -INFINITY;
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

	ZBasicDatum() {}

	~ZBasicDatum() {
		if (this->type == kDatumBCD) {
			delete this->data.bcd;
			this->data.bcd = 0;
		} else if (this->type == kDatumSTR) {
			delete this->data.str;
			this->data.str = 0;
		}
	}

	static Common::SharedPtr<ZBasicDatum> newNull(int32 offset) {
		ZBasicDatum *result = new ZBasicDatum();
		result->type = kDatumNULL;
		result->offset = offset;
		result->data.bcd = 0;
		return Common::SharedPtr<ZBasicDatum>(result);
	}

	static Common::SharedPtr<ZBasicDatum> newInt(uint32 offset, int16 value) {
		ZBasicDatum *result = new ZBasicDatum();
		result->type = kDatumINT;
		result->offset = offset;
		result->data.i16 = value;
		return Common::SharedPtr<ZBasicDatum>(result);
	}

	static Common::SharedPtr<ZBasicDatum> newDblInt(uint32 offset, int32 value) {
		ZBasicDatum *result = new ZBasicDatum();
		result->type = kDatumDBLINT;
		result->offset = offset;
		result->data.i32 = value;
		return Common::SharedPtr<ZBasicDatum>(result);
	}

	static Common::SharedPtr<ZBasicDatum> newBcd(uint32 offset, const ZBasicBCD &value) {
		ZBasicDatum *result = new ZBasicDatum();
		result->type = kDatumBCD;
		result->offset = offset;
		result->data.bcd = new ZBasicBCD(value);
		return Common::SharedPtr<ZBasicDatum>(result);
	}

	static Common::SharedPtr<ZBasicDatum> newStr(uint32 offset, const Common::U32String &value) {
		ZBasicDatum *result = new ZBasicDatum();
		result->type = kDatumSTR;
		result->offset = offset;
		result->data.str = new Common::U32String(value);
		return Common::SharedPtr<ZBasicDatum>(result);
	}
};

class ZBasic {

private:
	Common::Array<Common::SharedPtr<ZBasicDatum>> _dataTable;
	uint32 _dataPtr;
	Common::Array<Common::SharedPtr<ZBasicDatum>> _stringTable;

	Graphics::MacToolbox::Toolbox *_toolbox;
	Common::MemoryPool *_memPool;
	int16 _fileId = -1;
	Graphics::MacWindow *_window;
	Graphics::MacMenu *_menu;

	Graphics::MacToolbox::GrafPtr _defaultPort;

	Common::HashMap<int16, Common::Path> _filePaths;
	Common::HashMap<int16, Common::SharedPtr<Common::SeekableReadStream>> _fileStreams;
	Common::HashMap<int16, Common::SharedPtr<Common::MemoryWriteStreamDynamic>> _fileWriteStreams;
	Common::HashMap<int16, Common::SharedPtr<Common::SeekableWriteStream>> _fileWriteSaveStreams;
	Common::HashMap<int16, uint32> _fileLineSize;
	Common::HashMap<int16, Common::Array<Common::String>> _index;

	uint32 _fileType;
	uint32 _fileCreator;

public:

	ZBasic(Graphics::MacToolbox::Toolbox *toolbox);
	~ZBasic();

	void loadProgram(const Common::Path &path);
	void loadSCOT();
	void loadFonts();

	int16 asc(const Common::U32String &str);
	void bufferFlush(const Common::U32String &str);
	void blockMove(void *srcptr, void *destptr, uint16 size)
;
	int16 castInt(const Common::String &str);
	Common::U32String chr(uint16 code);
	void close(int16 fileNo);
	void coordinateWindow();
	void defOpen(const Common::String &str);
	void get(int16 x1, int16 y1, int16 x2, int16 y2, Graphics::MacToolbox::BitMap &dest, bool preserveDims = false);
	int16 instr(int16 expression, const Common::U32String &string1, const Common::U32String &string2);
	int16 instr(int16 expression, const Common::String &string1, const Common::String &string2);
	Common::U32String files(int16 expression, const Common::U32String &prompt, const Common::U32String &defaultFilename, int16 &volume);
	int finderInfo(int16 &count, Common::U32String &var, Graphics::MacToolbox::OSType &type, uint16 volume);
	bool maybe();
	uint32 mem(int16 index);
	void menu(uint16 menuNo, uint16 itemNo, uint16 state, const Common::U32String &title);
	Common::U32String leftStr(const Common::U32String &str, int16 expression);
	Common::String leftStr(const Common::String &str, int16 expression);
	Common::U32String rightStr(const Common::U32String &str, int16 expression);
	Common::String rightStr(const Common::String &str, int16 expression);
	Common::U32String midStr(const Common::U32String &str, int16 expr1, int16 expr2);
	void midStrSet(Common::U32String &target, int16 expr1, int16 expr2, const Common::U32String &src);
	Common::String midStr(const Common::String &str, int16 expr1, int16 expr2);
	void midStrSet(Common::String &target, int16 expr1, int16 expr2, const Common::String &src);
	void openR(int16 fileNo, const Common::U32String &fileName, uint32 lineSize, int16 volNo);
	void openW(int16 fileNo, const Common::U32String &fileName, uint32 lineSize, int16 volNo);
	void picture(int16 x, int16 y, Graphics::MacToolbox::PicHandle &src);
	void picture(int16 x1, int16 y1, int16 x2, int16 y2, Graphics::MacToolbox::PicHandle &src);
	void put(int16 x, int16 y, Graphics::MacToolbox::BitMap &src, Graphics::MacToolbox::SourceMode mode);
	void put(int16 x1, int16 y1, int16 x2, int16 y2, Graphics::MacToolbox::BitMap &src, Graphics::MacToolbox::SourceMode mode);
	int16 readDataInt();
	int32 readDataDblInt();
	Common::U32String readDataStr();
	Graphics::MacToolbox::Handle readFile(int16 fileNo, uint32 length);
	uint32 readFile(int16 fileNo, byte *dest, uint32 length);
	int32 readFileDblInt(int16 fileNo);
	int16 readFileInt(int16 fileNo);
	Common::String readFileStr(int16 fileNo, int16 length);
	void record(int16 fileNo, int16 recordNo, int16 location);
	void restore(uint32 index);
	int16 rndInt(int16 max);
	Common::U32String space(int16 count);
	void swapInt(int16 &a, int16 &b);
	void swapStr(Common::U32String &a, Common::U32String &b);
	void text(uint16 font, uint16 size, uint16 face, Graphics::MacToolbox::SourceMode mode);
	void window(int16 windowNumber, const Common::String &title, int16 x1, int16 y1, int16 x2, int16 y2, ZBasicWindowType type);
	void writeFileStr(int16 fileNo, const Common::String &str);
	void writeFileInt(int16 fileNo, int16 data);
	void writeFileDblInt(int16 fileNo, int32 data);
	Common::U32String ucase(const Common::U32String &str);

	const Common::U32String str(size_t index);
	const Common::String strRaw(size_t index);


	void unk_4(); // possibly quit?
	int16 soundBusy();
	void sound(int16 frequency, int32 duration, int16 volume, int16 voice);
	void unk_8();
	void unk_10();
	void unk_11(int16 unk1);
	void unk_20();
	bool incrAndCheck(int16 &a0, int16 d1, int16 d0);
	void unk_44(int16 unk1);
	Common::String encodeInt(uint16 data);
	void stringCopy(Common::U32String &dest, const Common::U32String &src);
	void unk_130(int16 unk1);
	void unk_158();
	uint16 decodeInt(const Common::String &data);
	void unk_331(uint16 unk1, int16 unk2);
	void indexClear(int16 table);
	void indexSet(const Common::U32String &value, int16 table, int16 index);
	void indexRawSet(const Common::String &value, int16 table, int16 index);
	Common::U32String index(int16 table, int16 index);
	Common::String indexRaw(int16 table, int16 index);

	int16 getFileId() { return _fileId; }
	void injectFOND(const byte *data, const size_t size, const Common::String &name);
	void setMenuFont(uint16 font, uint16 size);

};

} // namespace Fool

#endif
