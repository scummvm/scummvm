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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/file.h"
#include "common/textconsole.h"

#include "audio/audiostream.h"
#include "audio/decoders/flac.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/wave.h"

#include "graphics/surface.h"

#include "image/pcx.h"

#include "tucker/tucker.h"
#include "tucker/graphics.h"

namespace Tucker {

enum {
	kCurrentCompressedSoundDataVersion = 1,
	kCompressedSoundDataFileHeaderSize = 4
};

struct CompressedSoundFile {
	const char *filename;
	Audio::SeekableAudioStream *(*makeStream)(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse);
};

static const CompressedSoundFile compressedSoundFilesTable[] = {
#ifdef USE_FLAC
	{ "TUCKER.SOF", Audio::makeFLACStream },
#endif
#ifdef USE_VORBIS
	{ "TUCKER.SOG", Audio::makeVorbisStream },
#endif
#ifdef USE_MAD
	{ "TUCKER.SO3", Audio::makeMP3Stream },
#endif
	{ nullptr,      nullptr }
};

static void decodeData(uint8 *data, int dataSize) {
	for (int i = 0; i < dataSize; ++i) {
		data[i] -= 0x80;
	}
}

static void stripData(uint8 *data, int dataSize) {
	bool clearChr = false;
	for (int i = 0; i < dataSize; ++i) {
		if (!clearChr && data[i] == '/') {
			clearChr = true;
		}
		if (clearChr) {
			if (data[i] == '\n') {
				clearChr = false;
			}
			data[i] = ' ';
		}
	}
}

enum DataToken {
	kDataTokenDw,
	kDataTokenEx
};

class DataTokenizer {
public:

	DataTokenizer(uint8 *data, int dataSize, bool stripComments = false)
		: _data(data), _dataSize(dataSize), _pos(0) {
		if (stripComments) {
			stripData(_data, _dataSize);
		}
	}

	int getNextInteger() {
		bool negate = false;
		int state = 0;
		int num = 0;
		while (_pos < _dataSize && state != 2) {
			switch (state) {
			case 0:
				if (_data[_pos] == '-') {
					state = 1;
					negate = true;
				} else if (_data[_pos] >= '0' && _data[_pos] <= '9') {
					state = 1;
					num = _data[_pos] - '0';
				}
				break;
			case 1:
				if (_data[_pos] >= '0' && _data[_pos] <= '9') {
					num *= 10;
					num += _data[_pos] - '0';
				} else {
					state = 2;
				}
				break;
			default:
				break;
			}
			++_pos;
		}
		return negate ? -num : num;
	}

	bool findNextToken(DataToken t) {
		const char *token = nullptr;
		switch (t) {
		case kDataTokenDw:
			token = "dw";
			break;
		case kDataTokenEx:
			token = "!!";
			break;
		default:
			break;
		}
		int tokenLen = strlen(token);
		while (_pos < _dataSize - tokenLen) {
			if (memcmp(_data + _pos, token, tokenLen) == 0) {
				_pos += tokenLen;
				return true;
			}
			++_pos;
		}
		return false;
	}

	bool findIndex(int num) {
		int i = -1;
		while (findNextToken(kDataTokenEx)) {
			i = getNextInteger();
			if (i >= num) {
				break;
			}
		}
		return i == num;
	}

	uint8 *_data;
	int _dataSize;
	int _pos;
};

uint8 *TuckerEngine::loadFile(const char *fname, uint8 *p) {
	Common::String filename;
	filename = fname;
	if (_gameLang == Common::DE_DEU || _gameLang == Common::RU_RUS) {
		if (filename == "bgtext.c") {
			filename = "bgtextgr.c";
		} else if (filename == "charname.c") {
			filename = "charnmgr.c";
		} else if (filename == "data5.c") {
			filename = "data5gr.c";
		} else if (filename == "infobar.txt") {
			filename = "infobrgr.txt";
		} else if (filename == "charsize.dta") {
			filename = "charszgr.dta";
		} else if (filename.hasPrefix("objtxt")) {
			filename = Common::String::format("objtx%cgr.c", filename[6]);
		} else if (filename.hasPrefix("pt")) {
			filename = Common::String::format("pt%ctxtgr.c", filename[2]);
		}
	}
	_fileLoadSize = 0;
	bool decode = false;
	if (_gameFlags & kGameFlagEncodedData) {
		if (filename.hasSuffix(".c")) {
			filename.deleteLastChar();
			filename += "enc";
			decode = true;
		}
	}
	Common::File f;
	if (!f.open(filename)) {
		warning("Unable to open '%s'", filename.c_str());
		return nullptr;
	}
	const int sz = f.size();
	if (!p) {
		p = (uint8 *)malloc(sz);
	}
	if (p) {
		f.read(p, sz);
		if (decode) {
			decodeData(p, sz);
		}
		_fileLoadSize = sz;
	}
	return p;
}

void CompressedSound::openFile() {
	_compressedSoundType = -1;
	for (int i = 0; compressedSoundFilesTable[i].filename; ++i) {
		if (_fCompressedSound.open(compressedSoundFilesTable[i].filename)) {
			int version = _fCompressedSound.readUint16LE();
			if (version == kCurrentCompressedSoundDataVersion) {
				_compressedSoundType = i;
				_compressedSoundFlags = _fCompressedSound.readUint16LE();
				debug(1, "Using compressed sound file '%s'", compressedSoundFilesTable[i].filename);
				return;
			}
			warning("Unhandled version %d for compressed sound file '%s'", version, compressedSoundFilesTable[i].filename);
			_fCompressedSound.close();
		}
	}
}

void CompressedSound::closeFile() {
	_fCompressedSound.close();
}

Audio::RewindableAudioStream *CompressedSound::load(CompressedSoundType type, int num) {
	if (_compressedSoundType < 0) {
		return nullptr;
	}
	int offset = 0;
	switch (type) {
	case kSoundTypeFx:
		offset = kCompressedSoundDataFileHeaderSize;
		break;
	case kSoundTypeMusic:
		offset = kCompressedSoundDataFileHeaderSize + 8;
		break;
	case kSoundTypeSpeech:
		offset = kCompressedSoundDataFileHeaderSize + 16;
		break;
	case kSoundTypeIntro:
		if (_compressedSoundFlags & 1) {
			offset = kCompressedSoundDataFileHeaderSize + 24;
		}
		break;
	default:
		break;
	}
	if (offset == 0) {
		return nullptr;
	}
	Audio::SeekableAudioStream *stream = nullptr;
	_fCompressedSound.seek(offset);
	int dirOffset = _fCompressedSound.readUint32LE();
	int dirSize = _fCompressedSound.readUint32LE();
	if (num < dirSize) {
		const int dirHeaderSize = (_compressedSoundFlags & 1) ? 4 * 8 : 3 * 8;
		dirOffset += kCompressedSoundDataFileHeaderSize + dirHeaderSize;
		_fCompressedSound.seek(dirOffset + num * 8);
		int soundOffset = _fCompressedSound.readUint32LE();
		int soundSize = _fCompressedSound.readUint32LE();
		if (soundSize != 0) {
			_fCompressedSound.seek(dirOffset + dirSize * 8 + soundOffset);
			Common::SeekableReadStream *tmp = _fCompressedSound.readStream(soundSize);
			if (tmp) {
				stream = (compressedSoundFilesTable[_compressedSoundType].makeStream)(tmp, DisposeAfterUse::YES);
			}
		}
	}
	return stream;
}

void TuckerEngine::loadImage(const char *fname, uint8 *dst, int type) {
	char filename[80];
	Common::strlcpy(filename, fname, sizeof(filename));

	Common::File f;
	if (!f.open(filename)) {
		// workaround for "paper-3.pcx" / "paper_3.pcx"
		bool tryOpen = false;
		for (char *p = filename; *p; ++p) {
			switch (*p) {
			case '-':
				*p = '_';
				tryOpen = true;
				break;
			case '_':
				*p = '-';
				tryOpen = true;
				break;
			default:
				break;
			}
		}
		if (!tryOpen || !f.open(filename)) {
			warning("Unable to open '%s'", filename);
			return;
		}
	}

	Image::PCXDecoder pcx;
	if (!pcx.loadStream(f))
		error("Error while reading PCX image");

	const ::Graphics::Surface *pcxSurface = pcx.getSurface();
	if (pcxSurface->format.bytesPerPixel != 1)
		error("Invalid bytes per pixel in PCX surface (%d)", pcxSurface->format.bytesPerPixel);
	if (pcxSurface->w != 320 || pcxSurface->h != 200)
		error("Invalid PCX surface size (%d x %d)", pcxSurface->w, pcxSurface->h);
	for (uint16 y = 0; y < pcxSurface->h; y++)
		memcpy(dst + y * 320, pcxSurface->getBasePtr(0, y), pcxSurface->w);

	if (type != 0) {
		memcpy(_currentPalette, pcx.getPalette(), 3 * 256);
		setBlackPalette();
	}
}

void TuckerEngine::loadCursor() {
	loadImage("pointer.pcx", _loadTempBuf, 0);
	for (int cursor = 0; cursor < 7; ++cursor) {
		Graphics::encodeRAW(_loadTempBuf + cursor * 320 * 16, _cursorGfxBuf + cursor * 256, 16, 16);
	}
}

void TuckerEngine::loadCharset() {
	loadImage("charset.pcx", _loadTempBuf, 0);
	switch (_gameLang) {
	case Common::EN_ANY:
		Graphics::setCharset(kCharsetTypeEng);
		break;
	default:
		Graphics::setCharset(kCharsetTypeDefault);
		break;
	}
	loadCharsetHelper();
}

void TuckerEngine::loadCharset2() {
	_charWidthTable[58] = 7;
	_charWidthTable[32] = 15;
	memcpy(_charWidthTable + 65, _charWidthCharset2, 58);
	loadImage("char2.pcx", _loadTempBuf, 0);
	Graphics::setCharset(kCharsetTypeCredits);
	loadCharsetHelper();
}

void TuckerEngine::loadCharsetHelper() {
	const int charW = Graphics::_charset._charW;
	const int charH = Graphics::_charset._charH;
	int offset = 0;
	for (int y = 0; y < Graphics::_charset._yCount; ++y) {
		for (int x = 0; x < Graphics::_charset._xCount; ++x) {
			offset += Graphics::encodeRAW(_loadTempBuf + (y * 320) * charH + x * charW, _charsetGfxBuf + offset, charW, charH);
		}
	}
}

void TuckerEngine::loadCharSizeDta() {
	loadFile("charsize.dta", _loadTempBuf);
	if (_fileLoadSize != 0) {
		DataTokenizer t(_loadTempBuf, _fileLoadSize, true);
		for (int i = 0; i < 256; ++i) {
			_charWidthTable[i] = t.getNextInteger();
		}
		if (_gameLang == Common::FR_FRA) {
			_charWidthTable[225] = 0;
		}
	} else {
		memcpy(_charWidthTable + 32, _charWidthCharset1, 224);
	}
}

void TuckerEngine::loadPanel() {
	loadImage((_panelStyle == kPanelStyleVerbs) ? "panel1.pcx" : "panel2.pcx", _panelGfxBuf, 0);
}

void TuckerEngine::loadBudSpr() {
	int framesCount[20];
	memset(framesCount, 0, sizeof(framesCount));
	int endOffset = loadCTable01(framesCount);
	loadCTable02();
	int frame = 0;
	int spriteOffset = 0;
	for (int i = 0; i < endOffset; ++i) {
		if (framesCount[frame] == i) {
			Common::String filename;
			switch (_flagsTable[137]) {
			case 0:
				if ((_gameFlags & kGameFlagDemo) != 0) {
					filename = Common::String::format("budl00_%d.pcx", frame + 1);
				} else {
					filename = Common::String::format("bud_%d.pcx", frame + 1);
				}
				break;
			case 1:
				filename = Common::String::format("peg_%d.pcx", frame + 1);
				break;
			default:
				filename = Common::String::format("mac_%d.pcx", frame + 1);
				break;
			}
			loadImage(filename.c_str(), _loadTempBuf, 0);
			++frame;
		}
		int sz = Graphics::encodeRLE(_loadTempBuf + _spriteFramesTable[i]._sourceOffset, _spritesGfxBuf + spriteOffset, _spriteFramesTable[i]._xSize, _spriteFramesTable[i]._ySize);
		_spriteFramesTable[i]._sourceOffset = spriteOffset;
		spriteOffset += sz;
	}
}

int TuckerEngine::loadCTable01(int *framesCount) {
	loadFile("ctable01.c", _loadTempBuf);
	DataTokenizer t(_loadTempBuf,  _fileLoadSize);
	int lastSpriteNum = 0;
	int count = 0;
	if (t.findIndex(0)) {
		while (t.findNextToken(kDataTokenDw)) {
			const int x = t.getNextInteger();
			if (x < 0) {
				break;
			} else if (x == 999) {
				framesCount[count] = lastSpriteNum;
				++count;
				continue;
			}
			const int y = t.getNextInteger();
			SpriteFrame *c = &_spriteFramesTable[lastSpriteNum];
			c->_sourceOffset = y * 320 + x;
			c->_xSize = t.getNextInteger();
			c->_ySize = t.getNextInteger();

			// WORKAROUND: original game glitch
			// The sprite grab animation table incorrectly states a height of 24
			// pixels for sprite number 57 while the correct size is 54 pixels.
			// This fixes a glitch in animation sequence 8 (the only to use sprite 57)
			// which gets triggered when picking up the nail in front of the museum.
			// Fixes Trac#10430
			if (lastSpriteNum == 57)
				c->_ySize = 54;

			c->_xOffset = t.getNextInteger();
			if (c->_xOffset > 300) {
				c->_xOffset -= 500;
			}
			c->_yOffset = t.getNextInteger();
			if (c->_yOffset > 300) {
				c->_yOffset -= 500;
			}
			++lastSpriteNum;
		}
	}
	framesCount[count] = -1;
	return lastSpriteNum;
}

void TuckerEngine::loadCTable02() {
	int entry = 0;
	int i = 0;
	loadFile("ctable02.c", _loadTempBuf);
	DataTokenizer t(_loadTempBuf, _fileLoadSize);
	while (t.findNextToken(kDataTokenDw)) {
		_spriteAnimationsTable[entry]._numParts = t.getNextInteger();
		if (_spriteAnimationsTable[entry]._numParts < 1) {
			return;
		}
		_spriteAnimationsTable[entry]._rotateFlag = t.getNextInteger();
		int num = t.getNextInteger();
		if (num != 0) {
			continue;
		}
		int start = 0;
		_spriteAnimationsTable[entry]._firstFrameIndex = i;
		while (start != 999) {
			start = t.getNextInteger();
			if (start == 9999) { // end marker in the demo version
				start = 999;
			}
			_spriteAnimationFramesTable[i] = start;
			++i;
		}
		++entry;
	}
}

void TuckerEngine::loadLoc() {
	Common::String filename;

	int i = _locationWidthTable[_location];
	_locationHeight = (_location < kLocationJesusCutscene1) ? 140 : 200;
	filename = Common::String::format((i == 1) ? "loc%02d.pcx" : "loc%02da.pcx", _location);
	copyLocBitmap(filename.c_str(), 0, false);
	Graphics::copyRect(_quadBackgroundGfxBuf, 320, _locationBackgroundGfxBuf, 640, 320, _locationHeight);
	if (_locationHeight == 200) {
		return;
	}
	filename = Common::String::format((i != 2) ? "path%02d.pcx" : "path%02da.pcx", _location);
	copyLocBitmap(filename.c_str(), 0, true);
	if (i > 1) {
		filename = Common::String::format("loc%02db.pcx", _location);
		copyLocBitmap(filename.c_str(), 320, false);
		Graphics::copyRect(_quadBackgroundGfxBuf + 44800, 320, _locationBackgroundGfxBuf + 320, 640, 320, _locationHeight);
		if (i == 2) {
			filename = Common::String::format("path%02db.pcx", _location);
			copyLocBitmap(filename.c_str(), 320, true);
		}
	}
	if (i > 2) {
		filename = Common::String::format("loc%02dc.pcx", _location);
		copyLocBitmap(filename.c_str(), 0, false);
		Graphics::copyRect(_quadBackgroundGfxBuf + 89600, 320, _locationBackgroundGfxBuf, 640, 320, 140);
	}
	if (_location == kLocationHotelRoom) {
		_loadLocBufPtr = _quadBackgroundGfxBuf + 89600;
		loadImage("rochpath.pcx", _loadLocBufPtr, 0);
	}
	if (i > 3) {
		filename = Common::String::format("loc%02dd.pcx", _location);
		copyLocBitmap(filename.c_str(), 0, false);
		Graphics::copyRect(_quadBackgroundGfxBuf + 134400, 320, _locationBackgroundGfxBuf + 320, 640, 320, 140);
	}
	_fullRedraw = true;
}

void TuckerEngine::loadObj() {
	if (_location == kLocationMap) {
		return;
	}
	if (_location <= kLocationWarehouseCutscene) {
		_part = kPartOne;
		_speechSoundBaseNum = 2639;
	} else if ((                                        _location <= kLocationFarDocks)
	        || (_location >= kLocationComputerScreen && _location <= kLocationSeedyStreetCutscene)
	        || (_location >= kLocationElvisCutscene  && _location <= kLocationJesusCutscene2)) {
		_part = kPartTwo;
		_speechSoundBaseNum = 2679;
	} else {
		_part = kPartThree;
		_speechSoundBaseNum = 2719;
	}
	if (_part == _currentPart) {
		return;
	}
	debug(2, "loadObj() part %d location %d", _part, _location);
	// If a savegame is loaded from the launcher, skip the display chapter
	if (_startSlot != -1)
		_startSlot = -1;
	else if ((_gameFlags & kGameFlagDemo) == 0) {
		handleNewPartSequence();
	}
	_currentPart = _part;

	Common::String filename;
	filename = Common::String::format("objtxt%d.c", _part);
	free(_objTxtBuf);
	_objTxtBuf = loadFile(filename.c_str(), nullptr);
	filename = Common::String::format("pt%dtext.c", _part);
	free(_ptTextBuf);
	_ptTextBuf = loadFile(filename.c_str(), nullptr);
	_characterSpeechDataPtr = _ptTextBuf;
	loadData();
	loadPanObj();
}

void TuckerEngine::loadData() {
	int objNum = _part * 10;
	loadFile("data.c", _loadTempBuf);
	DataTokenizer t(_loadTempBuf, _fileLoadSize);
	_dataCount = 0;
	int count = 0;
	int maxCount = 0;
	while (t.findIndex(objNum)) {
		while (t.findNextToken(kDataTokenDw)) {
			_dataCount = t.getNextInteger();
			if (_dataCount < 0) {
				break;
			}
			if (_dataCount > maxCount) {
				maxCount = _dataCount;
			}
			const int x = t.getNextInteger();
			const int y = t.getNextInteger();
			Data *d = &_dataTable[_dataCount];
			d->_sourceOffset = y * 320 + x;
			d->_xSize = t.getNextInteger();
			d->_ySize = t.getNextInteger();
			d->_xDest = t.getNextInteger();
			d->_yDest = t.getNextInteger();
			d->_index = count;
		}
		++objNum;
		++count;
	}
	_dataCount = maxCount;
	int offset = 0;
	for (int i = 0; i < count; ++i) {
		Common::String filename = Common::String::format("scrobj%d%d.pcx", _part, i);
		loadImage(filename.c_str(), _loadTempBuf, 0);
		offset = loadDataHelper(offset, i);
	}
}

int TuckerEngine::loadDataHelper(int offset, int index) {
	for (int i = 0; i < _dataCount + 1; ++i) {
		if (_dataTable[i]._index == index) {
			int sz = Graphics::encodeRLE(_loadTempBuf + _dataTable[i]._sourceOffset, _data3GfxBuf + offset, _dataTable[i]._xSize, _dataTable[i]._ySize);
			_dataTable[i]._sourceOffset = offset;
			offset += sz;
		}
	}
	return offset;
}

void TuckerEngine::loadPanObj() {
	Common::String filename = Common::String::format("panobjs%d.pcx", _part);
	loadImage(filename.c_str(), _loadTempBuf, 0);
	int offset = 0;
	for (int y = 0; y < 5; ++y) {
		for (int x = 0; x < 10; ++x) {
			const int i = y * 10 + x;
			_panelObjectsOffsetTable[i] = offset;
			offset += Graphics::encodeRLE(_loadTempBuf + (y * 240 + x) * 32, _panelObjectsGfxBuf + offset, 32, 24);
		}
	}
}

void TuckerEngine::loadData3() {
	loadFile("data3.c", _loadTempBuf);
	DataTokenizer t(_loadTempBuf, _fileLoadSize);
	_locationAnimationsCount = 0;
	if (t.findIndex(_location)) {
		while (t.findNextToken(kDataTokenDw)) {
			int num = t.getNextInteger();
			if (num < 0) {
				break;
			}
			assert(_locationAnimationsCount < kLocationAnimationsTableSize);
			LocationAnimation *d = &_locationAnimationsTable[_locationAnimationsCount++];
			d->_graphicNum = num;
			const int seqNum = t.getNextInteger();
			if (seqNum > 0) {
				int anim = 0;
				for (int i = 1; i < seqNum; ++i) {
					while (_staticData3Table[anim] != 999) {
						++anim;
					}
					++anim;
				}
				d->_animCurrentCounter = d->_animInitCounter = anim;
				while (_staticData3Table[anim + 1] != 999) {
					++anim;
				}
				d->_animLastCounter = anim;
			} else {
				d->_animLastCounter = 0;
			}
			d->_getFlag = t.getNextInteger();
			d->_inventoryNum = t.getNextInteger();
			d->_flagNum = t.getNextInteger();
			d->_flagValue = t.getNextInteger();
			d->_selectable = t.getNextInteger();
			d->_standX = t.getNextInteger();
			d->_standY = t.getNextInteger();
			d->_drawFlag = false;
		}
	}
}

void TuckerEngine::loadData4() {
	loadFile("data4.c", _loadTempBuf);
	DataTokenizer t(_loadTempBuf, _fileLoadSize);
	if ((_gameFlags & kGameFlagDemo) == 0) {
		t.findNextToken(kDataTokenDw);
		_gameDebug = t.getNextInteger() != 0;
		_displayGameHints = t.getNextInteger() != 0;
	}
	_locationObjectsCount = 0;
	if (t.findIndex(_location)) {
		while (t.findNextToken(kDataTokenDw)) {
			int i = t.getNextInteger();
			if (i < 0)
				break;

			assert(_locationObjectsCount < kLocationObjectsTableSize);
			LocationObject *d = &_locationObjectsTable[_locationObjectsCount++];
			d->_xPos = i;
			d->_yPos = t.getNextInteger();
			d->_xSize = t.getNextInteger();
			d->_ySize = t.getNextInteger();
			d->_standX = t.getNextInteger();
			d->_standY = t.getNextInteger();
			d->_textNum = t.getNextInteger();
			d->_cursorStyle = (CursorStyle)t.getNextInteger();
			d->_location = (Location)t.getNextInteger();
			if (d->_location != kLocationNone) {
				d->_toX = t.getNextInteger();
				d->_toY = t.getNextInteger();
				d->_toX2 = t.getNextInteger();
				d->_toY2 = t.getNextInteger();
				d->_toWalkX2 = t.getNextInteger();
				d->_toWalkY2 = t.getNextInteger();
			}
		}
	}
}

void TuckerEngine::loadActionFile() {
	assert(_part != kPartInit);

	Common::String filename;
	if (_gameFlags & kGameFlagDemo) {
		filename = "action.c";
	} else {
		filename = Common::String::format("action%d.c", _part);
	}
	loadFile(filename.c_str(), _loadTempBuf);

	DataTokenizer t(_loadTempBuf, _fileLoadSize);
	_actionsCount = 0;
	if (t.findIndex(_location)) {
		while (t.findNextToken(kDataTokenDw)) {
			int keyA = t.getNextInteger();
			if (keyA < 0) {
				break;
			}
			int keyB = t.getNextInteger();
			int keyC = t.getNextInteger();
			int keyD = t.getNextInteger();
			int keyE = t.getNextInteger();
			Action *action = &_actionsTable[_actionsCount++];
			action->_key = keyE * 1000000 + keyD * 100000 + keyA * 10000 + keyB * 1000 + keyC;
			action->_testFlag1Num = t.getNextInteger();
			action->_testFlag1Value = t.getNextInteger();
			action->_testFlag2Num = t.getNextInteger();
			action->_testFlag2Value = t.getNextInteger();
			action->_speech = t.getNextInteger();
			action->_flipX = t.getNextInteger();
			action->_index = t.getNextInteger();
			action->_delay = t.getNextInteger();
			action->_setFlagNum = t.getNextInteger();
			assert(action->_setFlagNum >= 0 && action->_setFlagNum < kFlagsTableSize);
			action->_setFlagValue = t.getNextInteger();
			action->_fxNum = t.getNextInteger();
			action->_fxDelay = t.getNextInteger();
		}
	}
}

void TuckerEngine::loadCharPos() {
	loadFile("charpos.c", _loadTempBuf);
	DataTokenizer t(_loadTempBuf, _fileLoadSize);
	_charPosCount = 0;
	if (t.findIndex(_location)) {
		while (t.findNextToken(kDataTokenDw)) {
			const int i = t.getNextInteger();
			if (i < 0) {
				break;
			}
			assert(_charPosCount < 4);
			CharPos *charPos = &_charPosTable[_charPosCount++];
			charPos->_xPos = i;
			charPos->_yPos = t.getNextInteger();
			charPos->_xSize = t.getNextInteger();
			charPos->_ySize = t.getNextInteger();
			charPos->_xWalkTo = t.getNextInteger();
			charPos->_yWalkTo = t.getNextInteger();
			charPos->_flagNum = t.getNextInteger();
			charPos->_flagValue = t.getNextInteger();
			charPos->_direction = t.getNextInteger();
			charPos->_name = t.getNextInteger();
			charPos->_description = t.getNextInteger();
		}
		int quitLoop = 0;
		size_t count = 0;
		while (quitLoop == 0) {
			t.findNextToken(kDataTokenDw);
			int num = 0;
			while (num != 99) {
				num = t.getNextInteger();
				assert(count < ARRAYSIZE(_characterAnimationsTable));
				_characterAnimationsTable[count] = num;
				if (num < 0) {
					quitLoop = 1;
					break;
				}
				++count;
			}
		}
		quitLoop = 0;
		count = 0;
		while (quitLoop == 0) {
			t.findNextToken(kDataTokenDw);
			int num = 0;
			while (num < 98) {
				num = t.getNextInteger();
				assert(count < ARRAYSIZE(_characterStateTable));
				_characterStateTable[count] = num;
				if (num == 98) {
					--count;
				}
				if (num < 0) {
					quitLoop = 1;
					break;
				}
				++count;
			}
		}
	}
}

void TuckerEngine::loadSprA02_01() {
	unloadSprA02_01();
	const int count = _sprA02LookupTable[_location];
	for (int i = 1; i < count + 1; ++i) {
		Common::String filename = Common::String::format("sprites/a%02d_%02d.spr", _location, i);
		_sprA02Table[i] = loadFile(filename.c_str(), nullptr);
	}
	_sprA02Table[0] = _sprA02Table[1];
}

void TuckerEngine::unloadSprA02_01() {
	for (int i = 1; i < kSprA02TableSize; ++i) {
		free(_sprA02Table[i]);
		_sprA02Table[i] = nullptr;
	}
	_sprA02Table[0] = nullptr;
}

void TuckerEngine::loadSprC02_01() {
	unloadSprC02_01();
	const int count = _sprC02LookupTable[_location];
	for (int i = 1; i < count + 1; ++i) {
		Common::String filename = Common::String::format("sprites/c%02d_%02d.spr", _location, i);
		_sprC02Table[i] = loadFile(filename.c_str(), nullptr);
	}
	_sprC02Table[0] = _sprC02Table[1];
	_spritesCount = _sprC02LookupTable2[_location];
	for (int i = 0; i < kMaxCharacters; ++i) {
		memset(&_spritesTable[i], 0, sizeof(Sprite));
		_spritesTable[i]._state = -1;
		_spritesTable[i]._stateIndex = -1;
	}
}

void TuckerEngine::unloadSprC02_01() {
	for (int i = 1; i < kSprC02TableSize; ++i) {
		free(_sprC02Table[i]);
		_sprC02Table[i] = nullptr;
	}
	_sprC02Table[0] = nullptr;
}

void TuckerEngine::loadFx() {
	loadFile("fx.c", _loadTempBuf);
	DataTokenizer t(_loadTempBuf, _fileLoadSize);
	if (t.findIndex(_location)) {
		t.findNextToken(kDataTokenDw);
		_locationSoundsCount = t.getNextInteger();
		_currentFxSet = 0;
		for (int i = 0; i < _locationSoundsCount; ++i) {
			LocationSound *s = &_locationSoundsTable[i];
			s->_offset = 0;
			s->_num = t.getNextInteger();
			s->_volume = t.getNextInteger();
			s->_type = t.getNextInteger();
			switch (s->_type) {
			case 5:
				_currentFxSet = 1;
				_currentFxIndex = i;
				_currentFxVolume = s->_volume;
				_currentFxDist = t.getNextInteger();
				_currentFxScale = t.getNextInteger();
				break;
			case 6:
			case 7:
			case 8:
				s->_startFxSpriteState = t.getNextInteger();
				s->_startFxSpriteNum = t.getNextInteger();
				s->_updateType = t.getNextInteger();
				if (s->_type == 7) {
					s->_flagNum = t.getNextInteger();
					s->_flagValueStartFx = t.getNextInteger();
					s->_stopFxSpriteState = t.getNextInteger();
					s->_stopFxSpriteNum = t.getNextInteger();
					s->_flagValueStopFx = t.getNextInteger();
				}
				break;
			default:
				break;
			}
			if (s->_type == 8) {
				// type 8 is basically a pointer to another type 6 sample

				// WORKAROUND
				// There is at least one instance (namely in location 40) where the reference
				// is to another sample which has not yet been read in.
				// It seems that the original doesn't properly handle this case which
				// results in the sample not being played at all.
				// We just ignore and hop over these.
				if (s->_num >= i) {
					--i;
					continue;
				}

				assert(s->_num >= 0 && s ->_num < i);
				s->_num  = _locationSoundsTable[s->_num]._num;
				s->_type = 6;
			}
		}
		t.findNextToken(kDataTokenDw);
		int count = t.getNextInteger();
		_locationMusicsCount = 0;
		for (int i = 0; i < count; ++i) {
			int flagNum = t.getNextInteger();
			int flagValue = t.getNextInteger();
			if (flagValue == _flagsTable[flagNum]) {
				LocationMusic *m = &_locationMusicsTable[_locationMusicsCount++];
				m->_offset = 0;
				m->_num = t.getNextInteger();
				m->_volume = t.getNextInteger();
				m->_flag = t.getNextInteger();
			} else {
				for (int j = 0; j < 3; ++j) {
					t.getNextInteger();
				}
			}
		}
	} else {
		error("loadFx() - Index not found for location %d", _location);
	}

}

void TuckerEngine::loadSound(Audio::Mixer::SoundType type, int num, int volume, bool loop, Audio::SoundHandle *handle) {
	Audio::RewindableAudioStream *stream = nullptr;
	switch (type) {
	case Audio::Mixer::kSFXSoundType:
		stream = _compressedSound.load(kSoundTypeFx, num);
		break;
	case Audio::Mixer::kMusicSoundType:
		stream = _compressedSound.load(kSoundTypeMusic, num);
		break;
	case Audio::Mixer::kSpeechSoundType:
		stream = _compressedSound.load(kSoundTypeSpeech, num);
		break;
	default:
		return;
	}
	if (!stream) {
		const char *fmt = nullptr;
		switch (type) {
		case Audio::Mixer::kSFXSoundType:
			fmt = "fx/fx%d.wav";
			break;
		case Audio::Mixer::kMusicSoundType:
			fmt = "music/mus%d.wav";
			break;
		case Audio::Mixer::kSpeechSoundType:
			fmt = "speech/sam%04d.wav";
			break;
		default:
			return;
		}
		Common::String fileName = Common::String::format(fmt, num);
		Common::File *f = new Common::File;
		if (f->open(fileName)) {
			stream = Audio::makeWAVStream(f, DisposeAfterUse::YES);
		} else {
			delete f;
		}
	}

	if (stream) {
		_mixer->stopHandle(*handle);
		_mixer->playStream(type, handle, Audio::makeLoopingAudioStream(stream, loop ? 0 : 1),
		                        -1, scaleMixerVolume(volume, kMaxSoundVolume));
	}
}

void TuckerEngine::loadActionsTable() {
	int table = 0;
	do {
		if (!_csDataLoaded) {
			DataTokenizer t(_csDataBuf, _csDataSize);
			bool found = t.findIndex(_location);
			assert(found);
			for (int i = 0; i < _nextAction; ++i) {
				found = t.findNextToken(kDataTokenDw);
				assert(found);
			}
			_forceRedrawPanelItems = true;
			_panelType = kPanelTypeEmpty;
			setCursorState(kCursorStateDisabledHidden);
			_tableInstructionsPtr = _csDataBuf + t._pos + 1;
			_csDataLoaded = true;
			_csDataHandled = true;
			debug(2, "loadActionsTable() _nextAction %d", _nextAction);
		}
		if (_stopActionOnSpeechFlag && _charSpeechSoundCounter > 0) {
			break;
		}
		_stopActionOnSpeechFlag = false;
		if (_stopActionOnPanelLock) {
			if (_panelLockedFlag) {
				break;
			}
			_stopActionOnPanelLock = false;
		}
		if (_stopActionCounter > 0) {
			--_stopActionCounter;
			break;
		}
		if (_stopActionOnSoundFlag) {
			if (isSoundPlaying(_soundInstructionIndex)) {
				break;
			}
			_stopActionOnSoundFlag = false;
		}
		if (_csDataTableCount != 0) {
			if (_csDataTableCount == 99) {
				if (_backgroundSpriteCurrentAnimation > -1) {
					if (_backgroundSpriteCurrentFrame != _backgroundSpriteLastFrame) {
						break;
					}
				} else {
					if (_spriteAnimationFramesTable[_spriteAnimationFrameIndex] != 999) {
						break;
					}
				}
			} else {
				if (_spritesTable[_csDataTableCount - 1]._firstFrame - 1 != _spritesTable[_csDataTableCount - 1]._animationFrame) {
					break;
				}
			}
			 _csDataTableCount = 0;
		}
		if (_conversationOptionsCount != 0) {
			if (_leftMouseButtonPressed && _nextTableToLoadIndex != -1) {
				_nextAction = _nextTableToLoadTable[_nextTableToLoadIndex];
				_csDataLoaded = false;
				 _conversationOptionsCount = 0;
				setCursorState(kCursorStateDisabledHidden);
			}
			break;
		}
		do {
			table = executeTableInstruction();
		} while (table == 0);
	} while (table == 3);
	if (table == 2) {
		_nextAction = 0;
		_csDataLoaded = false;
		_forceRedrawPanelItems = true;
		_panelType = kPanelTypeNormal;
		setCursorState(kCursorStateNormal);
		_csDataHandled = false;
		_actionVerbLocked = false;
		_mouseClick = 1;
	}
}

} // namespace Tucker
