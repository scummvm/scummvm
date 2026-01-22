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

#include "common/macresman.h"
#include "common/memorypool.h"
#include "common/memstream.h"
#include "common/str-enc.h"
#include "common/stream.h"

#include "fool/fool.h"
#include "fool/toolbox.h"
#include "fool/zbasic.h"

namespace Fool {

ZBasic::ZBasic(Toolbox *toolbox) : _toolbox(toolbox) {
	_memPool = new Common::MemoryPool(sizeof(int));
	_window = g_engine->_wm.addWindow(false, false, false);
	_window->disableBorder();
	_window->resize(SCREEN_WIDTH, SCREEN_HEIGHT);
	g_engine->_wm.setBackgroundWindow(_window);
}

ZBasic::~ZBasic() {
	delete _memPool;
	_memPool = nullptr;
}

void ZBasic::loadProgram(const Common::Path &path) {
	_fileId = this->_toolbox->OpenResFile(path);
	if (_fileId == -1) {
		warning("ZBasic::loadProgram: unable to open %s", path.toString(':').c_str());
		return;
	}
	Handle scotRes = this->_toolbox->GetResource(MKTAG('S', 'C', 'O', 'T'), 1);
	if (!scotRes) {
		warning("ZBasic::loadProgram: could not find SCOT chunk");
		return;
	}
	Common::MemoryReadStream *scot = new Common::MemoryReadStream(scotRes->data(), scotRes->size());

	this->_dataTable.clear();
	this->_dataPtr = 0;
	this->_stringTable.clear();
	bool hasStringTable = false;
	while (scot->pos() < scot->size()) {
		uint32 offset = scot->pos();
		uint8 opcode = scot->readByte();
		if (opcode == 0x80) {
			hasStringTable = true;
			break;
		}
		switch (opcode) {
		case kDatumNULL:
			debugN(5, "ZBasic::loadProgram: _dataTable[%d]: offset %d, NULL\n", this->_dataTable.size(), offset);
			this->_dataTable.push_back(ZBasicDatum::newNull(offset));
			break;
		case kDatumINT:
			{
				uint8 size = scot->readByte();
				Common::SharedPtr<ZBasicDatum> result;
				if (size == 0) {
					result = ZBasicDatum::newInt(offset, 0);
					debugN(5, "ZBasic::loadProgram: _dataTable[%d]: offset %d, %d\n", this->_dataTable.size(), offset, result->data.i16);
					this->_dataTable.push_back(result);
				} else if (size == 2) {
					result = ZBasicDatum::newInt(offset, scot->readSint16BE());
					debugN(5, "ZBasic::loadProgram: _dataTable[%d]: offset %d, %d\n", this->_dataTable.size(), offset, result->data.i16);
					this->_dataTable.push_back(result);
				} else {
					warning("ZBasic::loadProgram: unexpected size for int %d", size);
					scot->skip(size);
				}
			}
			break;
		case kDatumDBLINT:
			{
				uint8 size = scot->readByte();
				Common::SharedPtr<ZBasicDatum> result;
				if (size == 0) {
					result = ZBasicDatum::newDblInt(offset, 0);
					debugN(5, "ZBasic::loadProgram: _dataTable[%d]: offset %d, %d\n", this->_dataTable.size(), offset, result->data.i32);
					this->_dataTable.push_back(result);
				} else if (size == 4) {
					result = ZBasicDatum::newDblInt(offset, scot->readSint32BE());
					debugN(5, "ZBasic::loadProgram: _dataTable[%d]: offset %d, %d\n", this->_dataTable.size(), offset, result->data.i32);
					this->_dataTable.push_back(result);
				} else {
					warning("ZBasic::loadProgram: unexpected size for dblint %d", size);
					scot->skip(size);
				}
			}
			break;
		case kDatumSTR:
			{
				uint8 size = scot->readByte();
				char *buf = (char*)calloc(size+1, 1);
				scot->read(buf, size);
				Common::SharedPtr<ZBasicDatum> result = ZBasicDatum::newStr(offset, Common::convertToU32String(buf, Common::kMacRoman));
				debugN(5, "ZBasic::loadProgram: _dataTable[%d]: offset %d, \"%s\"\n", this->_dataTable.size(), offset, result->data.str->encode().c_str());
				this->_dataTable.push_back(result);
				free(buf);
			}
			break;
		case kDatumBCD:
			{
				scot->seek(-1);
				debugN(5, "ZBasic::loadProgram: _dataTable[%d]: offset %d, BCD\n", this->_dataTable.size(), offset);
				this->_dataTable.push_back(ZBasicDatum::newBcd(offset, ZBasicBCD::read(scot)));
			}
			break;
		default:
			break;
		}
		scot->skip(scot->pos() % 2);
	}
	if (hasStringTable) {
		uint32 offset = 0;
		scot->skip(1);
		while (scot->pos() < scot->size()) {
			uint8 size = scot->readByte();
			char *buf = (char*)calloc(size+1, 1);
			scot->read(buf, size);
			Common::SharedPtr<ZBasicDatum> result = ZBasicDatum::newStr(offset, Common::convertToU32String(buf, Common::kMacRoman));
			debugN(5, "ZBasic::loadProgram: _stringTable[%d]: offset %d, \"%s\"\n", this->_stringTable.size(), offset, result->data.str->encode().c_str());
			this->_stringTable.push_back(result);
			offset += size + 1;
			free(buf);
			scot->skip(scot->pos() % 2);
		}
	}

	delete scot;

}

void ZBasic::blockMove(void *srcptr, void *destptr, uint16 size) {
	memmove(destptr, srcptr, size);
}

Common::U32String ZBasic::chr(uint16 code) {
	return Common::U32String(code, Common::kMacRoman);
}

void ZBasic::bufferFlush(const Common::U32String &str) {
	warning("STUB: ZBasic::bufferFlush");
}

void ZBasic::close(int16 fileNo) {
	warning("STUB: ZBasic::close");
}

void ZBasic::coordinateWindow() {
	warning("STUB: ZBasic::coordinateWindow");
}

void ZBasic::get(int16 x1, int16 y1, int16 x2, int16 y2, Graphics::Surface &dest) {
	warning("STUB: ZBasic::get");
}

int16 ZBasic::instr(int16 expression, const Common::U32String &string1, const Common::U32String &string2) {
	uint32 result =  string1.find(string2, (uint32)MAX<int16>(expression - 1, 0));
	if (result == Common::U32String::npos)
		return 0;
	return (int16)(result + 1);
}

int ZBasic::finderInfo(int16 &count, Common::U32String &var, uint32 &type, uint16 volume) {
	warning("STUB: ZBasic::finderInfo");
	return 0;
}

bool ZBasic::maybe() {
	return g_engine->getRandomNumber(1) == 1;
}

uint32 ZBasic::mem(int16 index) {
	if (index == -1)
		return 32*1024*1024;
	return 0;
}

void ZBasic::openR(int16 fileNo, const Common::U32String &fileName, uint32 lineSize, int16 volNo) {
	warning("STUB: ZBasic::openR");
}

void ZBasic::picture(int16 x, int16 y, PicHandle &src) {
	if (!src) {
		warning("ZBasic::picture: Empty handle");
		return;
	}
	if (debugLevelSet(5)) {
		Graphics::Surface *srf = src->surfacePtr();
		Common::hexdump((const byte *)srf->getPixels(), srf->pitch*srf->h);
	}
	Common::Rect srcRect(0, 0, src->w, src->h);
	Common::Rect destRect(x, y, x+src->w, y+src->h);
	_window->getWindowSurface()->blitFrom(*src, srcRect, destRect);
	_window->addDirtyRect(destRect);
	_window->setContentDirty(true);
}

void ZBasic::picture(int16 x1, int16 y1, int16 x2, int16 y2, PicHandle &src) {
	warning("STUB: ZBasic::picture");
}

void ZBasic::put(int16 x, int16 y, Graphics::Surface &src) {
	this->put(x, y, x+src.w, y+src.h, src);
}

void ZBasic::put(int16 x1, int16 y1, int16 x2, int16 y2, Graphics::Surface &src) {
	warning("STUB: ZBasic::put");
}

Common::Array<byte> ZBasic::read(int16 fileNo, uint32 length) {
	warning("STUB: ZBasic::read");
	return Common::Array<byte>();
}

int16 ZBasic::readInt() {
	if (_dataPtr >= _dataTable.size()) {
		warning("ZBasic::readInt: no more data left");
		return 0;
	}
	Common::SharedPtr<ZBasicDatum> &el = _dataTable[_dataPtr];
	_dataPtr++;
	if (el->type != kDatumINT) {
		warning("ZBasic::readInt: entry %d was unexpected type %d", _dataPtr-1, el->type);
		return 0;
	}
	return el->data.i16;
}

Common::U32String ZBasic::readStr() {
	Common::U32String result;
	if (_dataPtr >= _dataTable.size()) {
		warning("ZBasic::readStr: no more data left");
		return result;
	}
	Common::SharedPtr<ZBasicDatum> &el = _dataTable[_dataPtr];
	_dataPtr++;
	if (el->type != kDatumSTR) {
		warning("ZBasic::readStr: entry %d was unexpected type %d", _dataPtr-1, el->type);
		return result;
	}
	result = *el->data.str;
	return result;
}

int16 ZBasic::rndInt(int16 max) {
	if (max == 0)
		return 0;
	max = (int16)abs(MAX<int16>(max, -32767));
	return (int16)(g_engine->getRandomNumber(max - 1) + 1);
}


const Common::U32String &ZBasic::str(size_t index) {
	return *_stringTable[index]->data.str;
}

void ZBasic::text(int16 font, int16 size, int16 face, ZBasicTextMode mode) {
	warning("STUB: ZBasic::text");
}

void ZBasic::window(int16 windowNumber, const Common::String &title, int16 x1, int16 y1, int16 x2, int16 y2, ZBasicWindowType type) {
	warning("STUB:: ZBasic::window");
}

void ZBasic::unk_6(int16 unk1, int32 unk2, int16 unk3, int16 unk4) {
	warning("STUB:: ZBasic::unk_6");
}

void ZBasic::unk_20() {
	warning("STUB: ZBasic::unk_20");
}

void ZBasic::unk_130(int16 unk1) {
	warning("STUB: ZBasic::unk_130");
}


} // namespace Fool
