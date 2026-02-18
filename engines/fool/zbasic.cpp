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
#include "graphics/managed_surface.h"

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

	_toolbox->_defaultWindow = _window;
	_toolbox->_defaultBits = BitMap(new Graphics::ManagedSurface());
	_toolbox->_defaultBits->copyFrom(*_window->getWindowSurface());

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
	if (this->_fileStreams.contains(fileNo)) {
		this->_fileStreams.erase(fileNo);
	}
	if (this->_fileLineSize.contains(fileNo)) {
		this->_fileLineSize.erase(fileNo);
	}
}

void ZBasic::coordinateWindow() {
	warning("STUB: ZBasic::coordinateWindow");
}

void ZBasic::get(int16 x1, int16 y1, int16 x2, int16 y2, BitMap &dest, bool preserveDims) {
	GrafPtr port;
	_toolbox->GetPort(port);
	Common::Rect srcRect(x1, y1, x2, y2);
	// Real ZBasic uses a raw array for storage.
	// Some graphical effects expect there to be a full screen page that only has part
	// of it copied over, so we have this flag.
	if (preserveDims) {
		if (srcRect.width() != dest->w) {
			warning("ZBasic::get: differing row lengths, surface not going to draw correctly");
		}
	} else {
		dest->create(srcRect.width(), srcRect.height());
	}
	_toolbox->CopyBits(port->portBits, dest, srcRect, dest->getBounds(), kSrcCopy, nullptr);
}

int16 ZBasic::instr(int16 expression, const Common::U32String &string1, const Common::U32String &string2) {
	uint32 result =  string1.find(string2, (uint32)MAX<int16>(expression - 1, 0));
	if (result == Common::U32String::npos)
		return 0;
	return (int16)(result + 1);
}

Common::U32String ZBasic::files(int16 expression, const Common::U32String &prompt, const Common::U32String &defaultFilename, int16 &volume) {
	warning("STUB: ZBasic::files");
	return Common::U32String();
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

void ZBasic::menu(int16 menuNo, int16 itemNo, int16 state, const Common::U32String &title) {
	warning("STUB: ZBasic::menu");
}


void ZBasic::openR(int16 fileNo, const Common::U32String &fileName, uint32 lineSize, int16 volNo) {
	if (_fileStreams.contains(fileNo)) {
		this->close(fileNo);
	}
	Common::MacResManager resMan;
	Common::SeekableReadStream *result = resMan.openFileOrDataFork(Common::Path(fileName, ':'));
	if (!result) {
		error("ZBasic::openR: couldn't open %s", fileName.encode().c_str());
	}
	_fileStreams[fileNo] = Common::SharedPtr<Common::SeekableReadStream>(result);
	_fileLineSize[fileNo] = lineSize;
}

void ZBasic::picture(int16 x, int16 y, PicHandle &src) {
	if (!src) {
		warning("ZBasic::picture: Empty handle");
		return;
	}

	Common::Rect destRect(x, y, x+src->getSurface()->w, y+src->getSurface()->h);
	_toolbox->DrawPicture(src, destRect);

	//Common::Rect badnews = src->getBounds();
	//_window->getWindowSurface()->blitFrom(*src, badnews, badnews);
	//Common::Rect badnews = port->portBits->getBounds();
	//_window->getWindowSurface()->blitFrom(*port->portBits, badnews, badnews);
	//_window->addDirtyRect(badnews);
	//_window->setContentDirty(true);
	//_toolbox->Delay(0);
	//warning("did something stupid");
}

void ZBasic::picture(int16 x1, int16 y1, int16 x2, int16 y2, PicHandle &src) {
	warning("STUB: ZBasic::picture");
}

void ZBasic::put(int16 x, int16 y, BitMap &src, ZBasicPutMode mode) {
	this->put(x, y, x+src->w, y+src->h, src, mode);
}

void ZBasic::put(int16 x1, int16 y1, int16 x2, int16 y2, BitMap &src, ZBasicPutMode mode) {
	Common::Rect destRect(x1, y1, x2, y2);
	SourceMode sm = kSrcCopy;
	if (mode == kPutXOR) {
		sm = kSrcXor;
	} else if (mode == kPutAND) {
		sm = kSrcBic;
	} else if (mode == kPutOR) {
		sm = kSrcOr;
	}
	GrafPtr port;
	_toolbox->GetPort(port);
	_toolbox->CopyBits(src, port->portBits, src->getBounds(), destRect, sm, nullptr);
}

int32 ZBasic::readDataDblInt() {
	if (_dataPtr >= _dataTable.size()) {
		warning("ZBasic::readDataDblInt: no more data left");
		return 0;
	}
	Common::SharedPtr<ZBasicDatum> &el = _dataTable[_dataPtr];
	_dataPtr++;
	if (el->type != kDatumDBLINT) {
		warning("ZBasic::readDataDblInt: entry %d was unexpected type %d", _dataPtr-1, el->type);
		return 0;
	}
	return el->data.i32;
}

int16 ZBasic::readDataInt() {
	if (_dataPtr >= _dataTable.size()) {
		warning("ZBasic::readDataInt: no more data left");
		return 0;
	}
	Common::SharedPtr<ZBasicDatum> &el = _dataTable[_dataPtr];
	_dataPtr++;
	if (el->type != kDatumINT) {
		warning("ZBasic::readDataInt: entry %d was unexpected type %d", _dataPtr-1, el->type);
		return 0;
	}
	return el->data.i16;
}

Common::U32String ZBasic::readDataStr() {
	Common::U32String result;
	if (_dataPtr >= _dataTable.size()) {
		warning("ZBasic::readDataStr: no more data left");
		return result;
	}
	Common::SharedPtr<ZBasicDatum> &el = _dataTable[_dataPtr];
	_dataPtr++;
	if (el->type != kDatumSTR) {
		warning("ZBasic::readDataStr: entry %d was unexpected type %d", _dataPtr-1, el->type);
		return result;
	}
	result = *el->data.str;
	return result;
}

Handle ZBasic::readFile(int16 fileNo, uint32 length) {
	if (!_fileStreams.contains(fileNo)) {
		error("ZBasic::readFile: unknown fileNo %d", fileNo);
	}
	Handle result(new Common::Array<byte>());
	result->resize(length);
	uint32 newLen = _fileStreams[fileNo]->read(result->data(), length);
	if (newLen != length) {
		result->resize(newLen);
	}
	return result;
}

uint32 ZBasic::readFile(int16 fileNo, byte *dest, uint32 length) {
	if (!_fileStreams.contains(fileNo)) {
		error("ZBasic::readFile: unknown fileNo %d", fileNo);
	}
	length = _fileStreams[fileNo]->read(dest, length);
	return length;
}

int32 ZBasic::readFileDblInt(int16 fileNo) {
	if (!_fileStreams.contains(fileNo)) {
		error("ZBasic::readFileDblInt: unknown fileNo %d", fileNo);
	}
	return _fileStreams[fileNo]->readSint32BE();
}

int16 ZBasic::readFileInt(int16 fileNo) {
	if (!_fileStreams.contains(fileNo)) {
		error("ZBasic::readFileInt: unknown fileNo %d", fileNo);
	}
	return _fileStreams[fileNo]->readSint16BE();
}

Common::U32String ZBasic::readFileStr(int16 fileNo, int16 length) {
	if (!_fileStreams.contains(fileNo)) {
		error("ZBasic::readFileStr: unknown fileNo %d", fileNo);
	}
	warning("STUB: ZBasic::readFileStr");
	return Common::U32String();
}

void ZBasic::record(int16 fileNo, int16 recordNo, int16 location) {
	if (!_fileStreams.contains(fileNo)) {
		error("ZBasic::record: unknown fileNo %d", fileNo);
	}
	_fileStreams[fileNo]->seek(_fileLineSize[fileNo]*recordNo + location);
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

Common::U32String ZBasic::space(int16 count) {
	Common::U32String result;
	for (int i = 0; i < count; i++) {
		result += Common::U32String(" ");
	}
	return result;
}

void ZBasic::swapInt(int16 &a, int16 &b) {
	int16 tmp = a;
	a = b;
	b = tmp;
}

void ZBasic::text(uint16 font, uint16 size, uint16 face, SourceMode mode) {
	_toolbox->TextFont(font);
	_toolbox->TextSize(size);
	_toolbox->TextFace(face);
	_toolbox->TextMode(mode);
}

void ZBasic::window(int16 windowNumber, const Common::String &title, int16 x1, int16 y1, int16 x2, int16 y2, ZBasicWindowType type) {
	warning("STUB: ZBasic::window");
}

int16 ZBasic::unk_5() {
	warning("STUB: ZBasic::unk_5");
	return 0;
}

void ZBasic::unk_4() {
	warning("STUB: ZBasic::unk_4");
}

void ZBasic::unk_6(int16 unk1, int32 unk2, int16 unk3, int16 unk4) {
	warning("STUB: ZBasic::unk_6");
}

void ZBasic::unk_20() {
	warning("STUB: ZBasic::unk_20");
}

bool ZBasic::incrAndCheck(int16 &a0, int16 d1, int16 d0) {
	a0 += d0;
	return (d0 < 0) ? (a0 < d1) : (d1 < a0);
}

void ZBasic::unk_44(int16 unk1) {
	warning("STUB: ZBasic::unk_44");
}

void ZBasic::unk_110(Common::U32String &unk1, Common::U32String &unk2) {
	warning("STUB: ZBasic::unk_110");
}

void ZBasic::unk_130(int16 unk1) {
	warning("STUB: ZBasic::unk_130");
}

void ZBasic::unk_158() {
	warning("STUB: ZBasic::unk_158");
}

void ZBasic::unk_331(uint16 unk1, int16 unk2) {
	warning("STUB: ZBasic::unk_331");
}

void ZBasic::unk_333(uint16 unk1) {
	warning("STUB: ZBasic::unk_333");
}

void ZBasic::indexSet(const Common::U32String &value, int16 table, int16 index) {
	if (index < 0) {
		warning("ZBasic::indexSet: index must be positive, not %d", index);
		return;
	}
	if (!_index.contains(table)) {
		_index[table] = Common::Array<Common::U32String>();
	}
	if (_index[table].size() <= index) {
		_index[table].resize(index+1);
	}
	_index[table][index] = value;
}

Common::U32String ZBasic::index(int16 table, int16 index) {
	if (index < 0) {
		warning("ZBasic::index: index must be positive, not %d", index);
		return Common::U32String();
	}
	if (!_index.contains(table)) {
		warning("ZBasic::index: table %d not found", table);
		return Common::U32String();
	}
	if (_index[table].size() <= index) {
		warning("ZBasic::index: asked for index %d but only %d entries in table %d", index, _index[table].size(), table);
		return Common::U32String();
	}
	return _index[table][index];
}

} // namespace Fool
