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
 * $URL$
 * $Id$
 *
 */

#include "common/file.h"

#include "sound/audiostream.h"
#include "sound/flac.h"
#include "sound/mp3.h"
#include "sound/vorbis.h"
#include "sound/wave.h"

#include "tucker/tucker.h"
#include "tucker/graphics.h"

namespace Tucker {

enum {
	kCurrentCompressedSoundDataVersion = 1,
	kCompressedSoundDataFileHeaderSize = 4
};

struct CompressedSoundFile {
	const char *filename;
	Audio::AudioStream *(*makeStream)(Common::SeekableReadStream *stream, bool disposeAfterUse, uint32 startTime, uint32 duration, uint numLoops);
};

static const CompressedSoundFile compressedSoundFilesTable[] = {
#ifdef USE_FLAC
	{ "TUCKER.SOF", Audio::makeFlacStream },
#endif
#ifdef USE_VORBIS
	{ "TUCKER.SOG", Audio::makeVorbisStream },
#endif
#ifdef USE_MAD
	{ "TUCKER.SO3", Audio::makeMP3Stream },
#endif
	{ 0, 0 }
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
			}
			++_pos;
		}
		return negate ? -num : num;
	}

	bool findNextToken(DataToken t) {
		const char *token = 0;
		switch (t) {
		case kDataTokenDw:
			token = "dw";
			break;
		case kDataTokenEx:
			token = "!!";
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

uint8 *TuckerEngine::loadFile(uint8 *p) {
	_fileLoadSize = 0;
	bool decode = false;
	if (_useEnc) {
		char *ext = strrchr(_fileToLoad, '.');
		if (ext && strcmp(ext + 1, "c") == 0) {
			strcpy(ext + 1, "enc");
			decode = true;
		}
	}
	Common::File f;
	if (!f.open(_fileToLoad)) {
		return 0;
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

void TuckerEngine::openCompressedSoundFile() {
	_compressedSoundType = -1;
	for (int i = 0; compressedSoundFilesTable[i].filename; ++i) {
		if (_fCompressedSound.open(compressedSoundFilesTable[i].filename)) {
			int version = _fCompressedSound.readUint16LE();
			if (version == kCurrentCompressedSoundDataVersion) {
				_compressedSoundType = i;
				return;
			}
			warning("Unhandled version %d for compressed sound file '%s'", version, compressedSoundFilesTable[i].filename);
		}
	}
}

void TuckerEngine::loadImage(uint8 *dst, int type) {
	int count = 0;
	Common::File f;
	if (!f.open(_fileToLoad)) {
		return;
	}
	f.seek(128, SEEK_SET);
	int size = 0;
	while (size < 64000) {
		if (type == 2) {
			++count;
			if (count > 500) {
				count = 0;
				isSpeechSoundPlaying();
			}
		}
		int code = f.readByte();
		if (code >= 0xC0) {
			const int sz = code - 0xC0;
			code = f.readByte();
			memset(dst + size, code, sz);
			size += sz;
		} else {
			dst[size++] = code;
		}
	}
	if (type != 0) {
		f.seek(-768, SEEK_END);
		for (int i = 0; i < 256; ++i) {
			if (type == 2) {
				isSpeechSoundPlaying();
			}
			for (int c = 0; c < 3; ++c) {
				_currentPalette[3 * i + c] = f.readByte();
			}
		}
		setBlackPalette();
	}
}

void TuckerEngine::loadCursor() {
	strcpy(_fileToLoad, "pointer.pcx");
	loadImage(_loadTempBuf, 0);
	for (int cursor = 0; cursor < 7; ++cursor) {
		Graphics::encodeRAW(_loadTempBuf + cursor * 320 * 16, _cursorGfxBuf + cursor * 256, 16, 16);
	}
}

void TuckerEngine::loadCharset() {
	strcpy(_fileToLoad, "charset.pcx");
	loadImage(_loadTempBuf, 0);
	loadCharsetHelper(kCharSet1CharW, kCharSet1CharH, 32, 7);
}

void TuckerEngine::loadCharset2() {
	_charWidthTable[58] = 7;
	_charWidthTable[32] = 15;
	memcpy(_charWidthTable + 65, _charWidthCharset2, 58);
	strcpy(_fileToLoad, "char2.pcx");
	loadImage(_loadTempBuf, 0);
	loadCharsetHelper(kCharSet2CharW, kCharSet2CharH, 16, 6);
}

void TuckerEngine::loadCharsetHelper(int charW, int charH, int xSize, int ySize) {
	int offset = 0;
	for (int y = 0; y < ySize; ++y) {
		for (int x = 0; x < xSize; ++x) {
			offset += Graphics::encodeRAW(_loadTempBuf + (y * 320) * charH + x * charW, _charsetGfxBuf + offset, charW, charH);
		}
	}
}

void TuckerEngine::loadCharSizeDta() {
	strcpy(_fileToLoad, "charsize.dta");
	loadFile(_loadTempBuf);
	if (_fileLoadSize != 0) {
		DataTokenizer t(_loadTempBuf, _fileLoadSize, true);
		for (int i = 0; i < 256; ++i) {
			_charWidthTable[i] = t.getNextInteger();
		}
		_charWidthTable[225] = 0;
	} else {
		memcpy(_charWidthTable + 32, _charWidthCharset1, 224);
	}
}

void TuckerEngine::loadPanel() {
	strcpy(_fileToLoad, (_panelNum == 0) ? "panel1.pcx" : "panel2.pcx");
	loadImage(_panelGfxBuf, 0);
}

void TuckerEngine::loadBudSpr(int startOffset) {
	int endOffset;
	loadCTable01(0, startOffset, endOffset);
	loadCTable02(0);
	int frame = 0;
	int spriteOffset = 0;
	for (int i = startOffset; i < endOffset; ++i) {
		if (_ctable01Table_sprite[frame] == i) {
			switch (_flagsTable[137]) {
			case 0:
				sprintf(_fileToLoad, "bud_%d.pcx", frame + 1);
				break;
			case 1:
				sprintf(_fileToLoad, "peg_%d.pcx", frame + 1);
				break;
			default:
				sprintf(_fileToLoad, "mac_%d.pcx", frame + 1);
				break;
			}
			loadImage(_loadTempBuf, 0);
			++frame;
		}
		int sz = Graphics::encodeRLE(_loadTempBuf + _spriteFramesTable[i].sourceOffset, _spritesGfxBuf + spriteOffset, _spriteFramesTable[i].xSize, _spriteFramesTable[i].ySize);
		_spriteFramesTable[i].sourceOffset = spriteOffset;
		spriteOffset += sz;
	}
}

void TuckerEngine::loadCTable01(int locationNum, int firstSpriteNum, int &lastSpriteNum) {
	strcpy(_fileToLoad, "ctable01.c");
	loadFile(_loadTempBuf);
	DataTokenizer t(_loadTempBuf,  _fileLoadSize);
	lastSpriteNum = firstSpriteNum;
	int count = 0;
	if (t.findIndex(_locationNum)) {
		while (t.findNextToken(kDataTokenDw)) {
			const int x = t.getNextInteger();
			if (x < 0) {
				break;
			} else if (x == 999) {
				_ctable01Table_sprite[count] = lastSpriteNum;
				++count;
				continue;
			}
			const int y = t.getNextInteger();
			SpriteFrame *c = &_spriteFramesTable[lastSpriteNum++];
			c->sourceOffset = y * 320 + x;
			c->xSize = t.getNextInteger();
			c->ySize = t.getNextInteger();
			c->xOffset = t.getNextInteger();
			if (c->xOffset > 300) {
				c->xOffset -= 500;
			}
			c->yOffset = t.getNextInteger();
			if (c->yOffset > 300) {
				c->yOffset -= 500;
			}
		}
	}
	_ctable01Table_sprite[count] = -1;
}

void TuckerEngine::loadCTable02(int fl) {
	assert(fl == 0);
	int entry = 0;
	int i = 0;
	strcpy(_fileToLoad, "ctable02.c");
	loadFile(_loadTempBuf);
	DataTokenizer t(_loadTempBuf, _fileLoadSize);
	while (t.findNextToken(kDataTokenDw)) {
		_spriteAnimationsTable[entry].numParts = t.getNextInteger();
		if (_spriteAnimationsTable[entry].numParts < 1) {
			return;
		}
		_spriteAnimationsTable[entry].rotateFlag = t.getNextInteger();
		int num = t.getNextInteger();
		if (num != fl) {
			continue;
		}
		int start = 0;
		_spriteAnimationsTable[entry].firstFrameIndex = i;
		while (start != 999) {
			start = t.getNextInteger();
			_spriteAnimationFramesTable[i] = start;
			++i;
		}
		++entry;
	}
}

void TuckerEngine::loadLoc() {
	int i = _locationWidthTable[_locationNum];
	_locationHeight = (_locationNum < 73) ? 140 : 200;
	strcpy(_fileToLoad, (i == 1) ? "loc00.pcx" : "loc00a.pcx");
	copyLocBitmap(0, 0);
	Graphics::copyFrom640(_locationBackgroundGfxBuf, _quadBackgroundGfxBuf, 320, _locationHeight);
	if (_locationHeight == 200) {
		return;
	}
	strcpy(_fileToLoad, (i != 2) ? "path00.pcx" : "path00a.pcx");
	copyLocBitmap(0, 1);
	if (i > 1) {
		strcpy(_fileToLoad, "loc00b.pcx");
		copyLocBitmap(320, 0);
		Graphics::copyFrom640(_locationBackgroundGfxBuf + 320, _quadBackgroundGfxBuf + 44800, 320, 140);
		if (i == 2) {
			strcpy(_fileToLoad, "path00b.pcx");
			copyLocBitmap(320, 1);
		}
	}
	if (i > 2) {
		strcpy(_fileToLoad, "loc00c.pcx");
		copyLocBitmap(0, 0);
		Graphics::copyFrom640(_locationBackgroundGfxBuf, _quadBackgroundGfxBuf + 89600, 320, 140);
	}
	if (_locationNum == 1) {
		strcpy(_fileToLoad, "rochpath.pcx");
		_loadLocBufPtr = _quadBackgroundGfxBuf + 89600;
		loadImage(_loadLocBufPtr, 0);
	}
	if (i > 3) {
		strcpy(_fileToLoad, "loc00d.pcx");
		copyLocBitmap(0, 0);
		Graphics::copyFrom640(_locationBackgroundGfxBuf + 320, _quadBackgroundGfxBuf + 134400, 320, 140);
	}
}

void TuckerEngine::loadObj() {
	if (_locationNum == 99) {
		return;
	}
	if (_locationNum < 24) {
		_partNum = 1;
		_speechSoundBaseNum = 2639;
	} else if (_locationNum < 41 || (_locationNum >= 69 && _locationNum < 73) || (_locationNum > 79 && _locationNum < 83)) {
		_partNum = 2;
		_speechSoundBaseNum = 2679;
	} else {
		_partNum = 3;
		_speechSoundBaseNum = 2719;
	}
	if (_partNum == _currentPartNum) {
		return;
	}
	handleNewPartSequence();
	_currentPartNum = _partNum;

	sprintf(_fileToLoad, "objtxt%d.c", _partNum);
	free(_objTxtBuf);
	_objTxtBuf = loadFile();
	sprintf(_fileToLoad, "pt%dtext.c", _partNum);
	free(_ptTextBuf);
	_ptTextBuf = loadFile();

	loadData();
	loadPanObj();
}

void TuckerEngine::loadData() {
	int flag = 0;
	int objNum = _partNum * 10;
	strcpy(_fileToLoad, "data.c");
	loadFile(_loadTempBuf);
	DataTokenizer t(_loadTempBuf, _fileLoadSize);
	_dataCount = 0;
	int count = 0;
	int maxCount = 0;
	while (flag < 2) {
		flag = 0;
		if (!t.findIndex(objNum)) {
			flag = 2;
		}
		while (flag == 0) {
			if (!t.findNextToken(kDataTokenDw)) {
				flag = 1;
				continue;
			}
			_dataCount = t.getNextInteger();
			if (_dataCount < 0) {
				flag = 1;
				continue;
			}
			if (_dataCount > maxCount) {
				maxCount = _dataCount;
			}
			const int x = t.getNextInteger();
			const int y = t.getNextInteger();
			_dataTable[_dataCount].sourceOffset = y * 320 + x;
			_dataTable[_dataCount].xSize = t.getNextInteger();
			_dataTable[_dataCount].ySize = t.getNextInteger();
			_dataTable[_dataCount].xDest = t.getNextInteger();
			_dataTable[_dataCount].yDest = t.getNextInteger();
			_dataTable[_dataCount].index = count;
		}
		if (flag < 2) {
			++objNum;
			++count;
		}
	}
	_dataCount = maxCount;
	int offset = 0;
	for (int i = 0; i < count; ++i) {
		sprintf(_fileToLoad, "scrobj%d%d.pcx", _partNum, i);
		loadImage(_loadTempBuf, 0);
		offset = loadDataHelper(offset, i);
	}
}

int TuckerEngine::loadDataHelper(int offset, int index) {
	for (int i = 0; i < _dataCount + 1; ++i) {
		if (_dataTable[i].index == index) {
			int sz = Graphics::encodeRLE(_loadTempBuf + _dataTable[i].sourceOffset, _data3GfxBuf + offset, _dataTable[i].xSize, _dataTable[i].ySize);
			_dataTable[i].sourceOffset = offset;
			offset += sz;
		}
	}
	return offset;
}

void TuckerEngine::loadPanObj() {
	sprintf(_fileToLoad, "panobjs%d.pcx", _partNum);
	loadImage(_loadTempBuf, 0);
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
	strcpy(_fileToLoad, "data3.c");
	loadFile(_loadTempBuf);
	DataTokenizer t(_loadTempBuf, _fileLoadSize);
	_locationAnimationsCount = 0;
	if (t.findIndex(_locationNum)) {
		while (t.findNextToken(kDataTokenDw)) {
			int num = t.getNextInteger();
			if (num < 0) {
				break;
			}
			assert(_locationAnimationsCount < kLocationAnimationsTableSize);
			LocationAnimation *d = &_locationAnimationsTable[_locationAnimationsCount++];
			d->graphicNum = num;
			const int seqNum = t.getNextInteger();
			int i = 0;
			int j = 1;
			if (seqNum > 0) {
				while (j < seqNum) {
					while (_staticData3Table[i] != 999) {
						++i;
					}
					++i;
					++j;
				}
				d->animCurrentCounter = d->animInitCounter = i;
				while (_staticData3Table[i + 1] != 999) {
					++i;
				}
				d->animLastCounter = i;
			} else {
				d->animLastCounter = 0;
			}
			d->getFlag = t.getNextInteger();
			d->inventoryNum = t.getNextInteger();
			d->flagNum = t.getNextInteger();
			d->flagValue = t.getNextInteger();
			d->selectable = t.getNextInteger();
			d->standX = t.getNextInteger();
			d->standY = t.getNextInteger();
			d->drawFlag = 0;
		}
	}
}

void TuckerEngine::loadData4() {
	strcpy(_fileToLoad,"data4.c");
	loadFile(_loadTempBuf);
	DataTokenizer t(_loadTempBuf, _fileLoadSize);
	t.findNextToken(kDataTokenDw);
	_data4FlagDebug = t.getNextInteger();
	_displayGameHints = t.getNextInteger();
	// forces game hints feature
//	_displayGameHints = 1;
	_locationObjectsCount = 0;
	if (t.findIndex(_locationNum)) {
		while (t.findNextToken(kDataTokenDw)) {
			int i = t.getNextInteger();
			if (i < 0) {
				break;
			}
			assert(_locationObjectsCount < kLocationObjectsTableSize);
			LocationObject *d = &_locationObjectsTable[_locationObjectsCount++];
			d->xPos = i;
			d->yPos = t.getNextInteger();
			d->xSize = t.getNextInteger();
			d->ySize = t.getNextInteger();
			d->standX = t.getNextInteger();
			d->standY = t.getNextInteger();
			d->textNum = t.getNextInteger();
			d->cursorNum = t.getNextInteger();
			d->locationNum = t.getNextInteger();
			if (d->locationNum > 0) {
				d->toX = t.getNextInteger();
				d->toY = t.getNextInteger();
				d->toX2 = t.getNextInteger();
				d->toY2 = t.getNextInteger();
				d->toWalkX2 = t.getNextInteger();
				d->toWalkY2 = t.getNextInteger();
			}
		}
	}
}

void TuckerEngine::loadActionFile() {
	switch (_partNum) {
	case 1:
		strcpy(_fileToLoad, "action1.c");
		break;
	case 2:
		strcpy(_fileToLoad, "action2.c");
		break;
	default:
		strcpy(_fileToLoad, "action3.c");
		break;
	}
	loadFile(_loadTempBuf);
	DataTokenizer t(_loadTempBuf, _fileLoadSize);
	_actionsCount = 0;
	if (t.findIndex(_locationNum)) {
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
			action->key = keyE * 1000000 + keyD * 100000 + keyA * 10000 + keyB * 1000 + keyC;
			action->testFlag1Num = t.getNextInteger();
			action->testFlag1Value = t.getNextInteger();
			action->testFlag2Num = t.getNextInteger();
			action->testFlag2Value = t.getNextInteger();
			action->speech = t.getNextInteger();
			action->flipX = t.getNextInteger();
			action->index = t.getNextInteger();
			action->delay = t.getNextInteger();
			action->setFlagNum = t.getNextInteger();
			action->setFlagValue = t.getNextInteger();
			action->fxNum = t.getNextInteger();
			action->fxDelay = t.getNextInteger();
		}
	}
}

void TuckerEngine::loadCharPos() {
	strcpy(_fileToLoad, "charpos.c");
	loadFile(_loadTempBuf);
	DataTokenizer t(_loadTempBuf, _fileLoadSize);
	_charPosCount = 0;
	if (t.findIndex(_locationNum)) {
		while (t.findNextToken(kDataTokenDw)) {
			const int i = t.getNextInteger();
			if (i < 0) {
				break;
			}
			assert(_charPosCount < 4);
			CharPos *charPos = &_charPosTable[_charPosCount++];
			charPos->xPos = i;
			charPos->yPos = t.getNextInteger();
			charPos->xSize = t.getNextInteger();
			charPos->ySize = t.getNextInteger();
			charPos->xWalkTo = t.getNextInteger();
			charPos->yWalkTo = t.getNextInteger();
			charPos->flagNum = t.getNextInteger();
			charPos->flagValue = t.getNextInteger();
			charPos->direction = t.getNextInteger();
			charPos->name = t.getNextInteger();
			charPos->description = t.getNextInteger();
		}
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

void TuckerEngine::loadSprA02_01() {
	for (int i = 1; i < kSprA02TableSize; ++i) {
		free(_sprA02Table[i]);
		_sprA02Table[i] = 0;
	}
	const int count = _sprA02LookupTable[_locationNum];
	for (int i = 1; i < count + 1; ++i) {
		sprintf(_fileToLoad, "sprites/a%02d_%02d.spr", _locationNum, i);
		_sprA02Table[i] = loadFile();
	}
	_sprA02Table[0] = _sprA02Table[1];
}

void TuckerEngine::loadSprC02_01() {
	for (int i = 1; i < kSprC02TableSize; ++i) {
		free(_sprC02Table[i]);
		_sprC02Table[i] = 0;
	}
	const int count = _sprC02LookupTable[_locationNum];
	for (int i = 1; i < count + 1; ++i) {
		sprintf(_fileToLoad, "sprites/c%02d_%02d.spr", _locationNum, i);
		_sprC02Table[i] = loadFile();
	}
	_sprC02Table[0] = _sprC02Table[1];
	_spritesCount = _sprC02LookupTable2[_locationNum];
	for (int i = 0; i < kMaxCharacters; ++i) {
		memset(&_spritesTable[i], 0, sizeof(Sprite));
		_spritesTable[i].state = -1;
		_spritesTable[i].stateIndex = -1;
	}
}

void TuckerEngine::loadFx() {
	strcpy(_fileToLoad, "fx.c");
	loadFile(_loadTempBuf);
	DataTokenizer t(_loadTempBuf, _fileLoadSize);
	t.findIndex(_locationNum);
	t.findNextToken(kDataTokenDw);
	_locationSoundsCount = t.getNextInteger();
	_locationSoundsTable[0].offset = 0;
	_currentFxSet = 0;
	for (int i = 0; i < _locationSoundsCount; ++i) {
		_locationSoundsTable[i].num = t.getNextInteger();
		_locationSoundsTable[i].volume = t.getNextInteger();
		_locationSoundsTable[i].type = t.getNextInteger();
		if (_locationSoundsTable[i].type == 5) {
			_currentFxSet = 1;
			_currentFxIndex = i;
			_currentFxVolume = _locationSoundsTable[i].volume;
			_currentFxDist = t.getNextInteger();
			_currentFxScale = t.getNextInteger();
		} else if (_locationSoundsTable[i].type == 6 || _locationSoundsTable[i].type == 7 || _locationSoundsTable[i].type == 8) {
			_locationSoundsTable[i].startFxSpriteState = t.getNextInteger();
			_locationSoundsTable[i].startFxSpriteNum = t.getNextInteger();
			_locationSoundsTable[i].updateType = t.getNextInteger();
			if (_locationSoundsTable[i].type == 7) {
				_locationSoundsTable[i].flagNum = t.getNextInteger();
				_locationSoundsTable[i].flagValueStartFx = t.getNextInteger();
				_locationSoundsTable[i].stopFxSpriteState = t.getNextInteger();
				_locationSoundsTable[i].stopFxSpriteNum = t.getNextInteger();
				_locationSoundsTable[i].flagValueStopFx = t.getNextInteger();
			}
		}
		if (_locationSoundsTable[i].type == 8) {
			_locationSoundsTable[i].type = 6;
		}
	}
	t.findNextToken(kDataTokenDw);
	int count = t.getNextInteger();
	_locationMusicsTable[0].offset = _locationSoundsTable[_locationSoundsCount].offset;
	_locationMusicsCount = 0;
	for (int i = 0; i < count; ++i) {
		int flagNum = t.getNextInteger();
		int flagValue = t.getNextInteger();
		if (flagValue == _flagsTable[flagNum]) {
			_locationMusicsTable[_locationMusicsCount].num = t.getNextInteger();
			_locationMusicsTable[_locationMusicsCount].volume = t.getNextInteger();
			_locationMusicsTable[_locationMusicsCount].flag = t.getNextInteger();
			++_locationMusicsCount;
		} else {
			for (int j = 0; j < 3; ++j) {
				t.getNextInteger();
			}
		}
	}
}

void TuckerEngine::loadSound(Audio::Mixer::SoundType type, int num, int volume, bool loop, Audio::SoundHandle *handle) {
	Audio::AudioStream *stream = 0;
	if (_compressedSoundType < 0) {
		const char *fmt = 0;
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
		char fileName[64];
		snprintf(fileName, sizeof(fileName), fmt, num);
		Common::File f;
		if (f.open(fileName)) {
			int size, rate;
			uint8 flags = 0;
			if (Audio::loadWAVFromStream(f, size, rate, flags)) {
				uint8 *data = (uint8 *)malloc(size);
				if (data) {
					f.read(data, size);
					flags |= Audio::Mixer::FLAG_AUTOFREE;
					if (loop) {
						flags |= Audio::Mixer::FLAG_LOOP;
					}
					stream = Audio::makeLinearInputStream(data, size, rate, flags, 0, 0);
				}
			}
		}
	} else {
		int offset = 0;
		switch (type) {
		case Audio::Mixer::kSFXSoundType:
			offset = kCompressedSoundDataFileHeaderSize;
			break;
		case Audio::Mixer::kMusicSoundType:
			offset = kCompressedSoundDataFileHeaderSize + 8;
			break;
		case Audio::Mixer::kSpeechSoundType:
			offset = kCompressedSoundDataFileHeaderSize + 16;
			break;
		default:
			return;
		}
		_fCompressedSound.seek(offset);
		int dirOffset = _fCompressedSound.readUint32LE();
		int dirSize = _fCompressedSound.readUint32LE();
		if (num < dirSize) {
			dirOffset += kCompressedSoundDataFileHeaderSize + 3 * 8;
			_fCompressedSound.seek(dirOffset + num * 8);
			int soundOffset = _fCompressedSound.readUint32LE();
			int soundSize = _fCompressedSound.readUint32LE();
			if (soundSize != 0) {
				_fCompressedSound.seek(dirOffset + dirSize * 8 + soundOffset);
				Common::MemoryReadStream *tmp = _fCompressedSound.readStream(soundSize);
				if (tmp) {
					stream = (compressedSoundFilesTable[_compressedSoundType].makeStream)(tmp, true, 0, 0, 1);
				}
			}
		}
	}
	if (stream) {
		_mixer->playInputStream(type, handle, stream, -1, volume * Audio::Mixer::kMaxChannelVolume / kMaxSoundVolume);
	}
}

void TuckerEngine::loadActionsTable() {
	int table = 0;
	do {
		if (!_csDataLoaded) {
			DataTokenizer t(_csDataBuf, _csDataSize);
			bool found = t.findIndex(_locationNum);
			assert(found);
			for (int i = 0; i < _nextAction; ++i) {
				found = t.findNextToken(kDataTokenDw);
				assert(found);
			}
			_forceRedrawPanelItems = 1;
			_panelState = 1;
			setCursorType(2);
			_tableInstructionsPtr = _csDataBuf + t._pos + 1;
			_csDataLoaded = true;
			_csDataHandled = 1;
		}
		if (_csDataTableFlag2 == 1 && _charSpeechSoundCounter > 0) {
			break;
		}
		_csDataTableFlag2 = 0;
		if (_stopActionOnPanelLock == 1) {
			if (_panelLockedFlag) {
				break;
			}
			_stopActionOnPanelLock = 0;
		}
		if (_stopActionCounter > 0) {
			--_stopActionCounter;
			break;
		}
		if (_stopActionOnSoundFlag != 0) {
			if (isSoundPlaying(_soundInstructionIndex)) {
				break;
			}
			_stopActionOnSoundFlag = 0;
		}
		if (_csDataTableCount != 0) {
			if (_csDataTableCount == 99) {
				if (_backgroundSpriteCurrentAnimation > -1) {
					if (_backgroundSpriteCurrentFrame != _backgroundSpriteLastFrame) {
						break;
					}
					_csDataTableCount = 0;
				} else {
					if (_spriteAnimationFramesTable[_spriteAnimationFrameIndex] != 999) {
						break;
					}
					_csDataTableCount = 0;
				}
			} else {
				if (_spritesTable[_csDataTableCount - 1].firstFrame - 1 != _spritesTable[_csDataTableCount - 1].animationFrame) {
					break;
				}
				_csDataTableCount = 0;
			}
		}
		if (_conversationOptionsCount != 0) {
			if (_mouseButtons != 0 && _nextTableToLoadIndex != -1) {
				_nextAction = _nextTableToLoadTable[_nextTableToLoadIndex];
				_csDataLoaded = false;
				 _conversationOptionsCount = 0;
				setCursorType(2);
			}
			break;
		}
		table = 0;
		while (table == 0) {
			table = parseTableInstruction();
		}
	} while (table == 3);
	if (table == 2) {
		_nextAction = 0;
		_csDataLoaded = false;
		_forceRedrawPanelItems = 1;
		_panelState = 0;
		setCursorType(0);
		_csDataHandled = 0;
		_skipPanelObjectUnderCursor = 0;
		_mouseClick = 1;
	}
}

} // namespace Tucker
