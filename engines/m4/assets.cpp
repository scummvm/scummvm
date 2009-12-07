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

#include "m4/assets.h"
#include "m4/globals.h"
#include "m4/compression.h"

namespace M4 {

BaseAsset::BaseAsset(M4Engine *vm, Common::SeekableReadStream* stream, int size, const char *name) : _vm(vm) {
}

BaseAsset::~BaseAsset() {
}

MachineAsset::MachineAsset(M4Engine *vm, Common::SeekableReadStream* stream, int size, const char *name) : BaseAsset(vm, stream, size, name) {
	uint32 stateCount = stream->readUint32LE();
	for (uint32 curState = 0; curState < stateCount; curState++) {
		uint32 stateOffset = stream->readUint32LE();
		_stateTable.push_back(stateOffset);
	}
	_codeSize = size - 4 - 4 * stateCount;
	_code = new byte[_codeSize];
	stream->read(_code, _codeSize);
}

MachineAsset::~MachineAsset() {
	delete[] _code;
}

void MachineAsset::getCode(byte *&code, uint32 &codeSize) {
	code = _code;
	codeSize = _codeSize;
}

uint32 MachineAsset::getStateOffset(uint32 state) {
	assert(state < _stateTable.size());
	return _stateTable[state];
}

SequenceAsset::SequenceAsset(M4Engine *vm, Common::SeekableReadStream* stream, int size, const char *name) : BaseAsset(vm, stream, size, name) {
	_localVarCount = stream->readUint32LE();
	_codeSize = size - 4;
	_code = new byte[_codeSize];
	stream->read(_code, _codeSize);
}

SequenceAsset::~SequenceAsset() {
	delete[] _code;
}

void SequenceAsset::getCode(byte *&code, uint32 &codeSize) {
	code = _code;
	codeSize = _codeSize;
}


DataAsset::DataAsset(M4Engine *vm, Common::SeekableReadStream* stream, int size, const char *name) : BaseAsset(vm, stream, size, name) {

	_recCount = stream->readUint32LE();
	_recSize = stream->readUint32LE();
	_dataSize = _recCount * _recSize;
	_data = new long[_dataSize];
	for (uint32 i = 0; i < _dataSize; i++)
		_data[i] = (long)stream->readUint32LE();

}

DataAsset::~DataAsset() {
	delete _data;
}

long *DataAsset::getRow(int index) {
	assert(index < _recCount);
	return &_data[_recSize * index];
}

SpriteAsset::SpriteAsset(M4Engine *vm, Common::SeekableReadStream* stream, int size, const char *name, bool asStream) : BaseAsset(vm, stream, size, name) {
	_stream = stream;

	if (_vm->isM4()) {
		loadM4SpriteAsset(vm, stream, asStream);
	} else {
		loadMadsSpriteAsset(vm, stream);
	}
}

void SpriteAsset::loadM4SpriteAsset(M4Engine *vm, Common::SeekableReadStream* stream, bool asStream) {
	bool isBigEndian = false;
	uint32 frameOffset;

	uint32 header = _stream->readUint32LE();
	if (header == HEAD_M4SS) {
		printf("LE-encoded sprite\n");
	} else {
		printf("BE-encoded sprite\n");
		isBigEndian = true;
	}

	_srcSize = parseSprite(isBigEndian);

	_stream->readUint32LE();
	_frameRate = (!isBigEndian) ? _stream->readUint32LE() : _stream->readUint32BE();
	_pixelSpeed = (!isBigEndian) ? _stream->readUint32LE() : _stream->readUint32BE();
	_maxWidth = (!isBigEndian) ? _stream->readUint32LE() : _stream->readUint32BE();
	_maxHeight = (!isBigEndian) ? _stream->readUint32LE() : _stream->readUint32BE();
	_stream->skip(6 * 4);
	_frameCount = (!isBigEndian) ? _stream->readUint32LE() : _stream->readUint32BE();

	printf("SpriteAsset::SpriteAsset() srcSize = %d; frameRate = %04X; pixelSpeed = %04X; maxWidth = %d; maxHeight = %d; frameCount = %d\n", _srcSize, _frameRate, _pixelSpeed, _maxWidth, _maxHeight, _frameCount);

	for (int curFrame = 0; curFrame < _frameCount; curFrame++) {
		frameOffset = (!isBigEndian) ? _stream->readUint32LE() : _stream->readUint32BE();
		_frameOffsets.push_back(frameOffset);
	}
	_frameOffsets.push_back(_srcSize - 48 - _frameCount * 4);

	_frameStartOffset = _stream->pos();

	// We don't need to load frames when streaming
	if (asStream)
		return;

	for (int curFrame = 0; curFrame < _frameCount; curFrame++) {
		frameOffset = _frameStartOffset + _frameOffsets[curFrame];
		_stream->seek(frameOffset);

		SpriteAssetFrame frame;
		loadFrameHeader(frame, isBigEndian);

		// Load & unpack RLE data if it's not a streaming animation
		if (frame.stream != 1) {

			frame.frame = new M4Sprite(stream, frame.x, frame.y, frame.w, frame.h, true, frame.comp);
#if 0
			char fn[512];
			sprintf(fn, "%04d.raw", curFrame);
			FILE *h = fopen(fn, "wb");
			fwrite((byte*)frame.frame->getBasePtr(), frame.w * frame.h, 1, h);
			fclose(h);
#endif
		}

		_frames.push_back(frame);

	}

}

void SpriteAsset::loadMadsSpriteAsset(M4Engine *vm, Common::SeekableReadStream* stream) {
	int curFrame = 0;
	uint32 frameOffset = 0;
	MadsPack sprite(stream);
	_frameRate = 0;
	_pixelSpeed = 0;
	_maxWidth = 0;
	_maxHeight = 0;

	Common::SeekableReadStream *spriteStream = sprite.getItemStream(0);
	for (int i = 0; i < 19; i++) {
		spriteStream->readUint16LE();
	}
	_frameCount = spriteStream->readUint16LE();
	// we skip the rest of the data
	delete spriteStream;

	// Get the palette data
	spriteStream = sprite.getItemStream(2);
	int numColors = 0;
	RGB8 *palData = Palette::decodeMadsPalette(spriteStream, &numColors);
	Common::copy(palData, &palData[numColors], &_palette[0]);
	if (numColors < 256)
		Common::set_to((byte *)&_palette[numColors], (byte *)&_palette[256], 0);
	_colorCount = numColors;
	delete[] palData;
	delete spriteStream;

	spriteStream = sprite.getItemStream(1);
	Common::SeekableReadStream *spriteDataStream = sprite.getItemStream(3);
	SpriteAssetFrame frame;
	for (curFrame = 0; curFrame < _frameCount; curFrame++) {
		frame.stream = 0;
		frame.comp = 0;
		frameOffset = spriteStream->readUint32LE();
		_frameOffsets.push_back(frameOffset);
		spriteStream->readUint32LE();	// frame size
		frame.x = spriteStream->readUint16LE();
		frame.y = spriteStream->readUint16LE();
		frame.w = spriteStream->readUint16LE();
		frame.h = spriteStream->readUint16LE();
		if (curFrame == 0)
			printf("%i frames, x = %i, y = %i, w = %i, h = %i\n", _frameCount, frame.x, frame.y, frame.w, frame.h);

		frame.frame = new M4Sprite(spriteDataStream, frame.x, frame.y, frame.w, frame.h, false);
		_frames.push_back(frame);
	}
	delete spriteStream;
	delete spriteDataStream;
}

SpriteAsset::~SpriteAsset() {
	for (Common::Array<SpriteAssetFrame>::iterator it = _frames.begin(); it != _frames.end(); it++) {
		delete (*it).frame;
	}
}

int32 SpriteAsset::parseSprite(bool isBigEndian) {

	uint32 format, chunkType, chunkSize = 0;

	_colorCount = 0;

	format = (!isBigEndian) ? _stream->readUint32LE() : _stream->readUint32BE();

	chunkType = (!isBigEndian) ? _stream->readUint32LE() : _stream->readUint32BE();

	if (chunkType == CELS__PAL) {
		chunkSize = (!isBigEndian) ? _stream->readUint32LE() : _stream->readUint32BE();
		uint32 numColors = (!isBigEndian) ? _stream->readUint32LE() : _stream->readUint32BE();
		// TODO
		//if (palette) {
			// TODO: A sprite set palette specifies the indexes, which need not start at
			// index 0. For now, I'm simply preloading the currently active palette
			// before starting to replace existing entries

			_vm->_palette->grabPalette((byte *) _palette, 0, 256);
			_colorCount = 0;

			for (uint32 i = 0; i < numColors; i++) {
				uint32 paletteEntry = (!isBigEndian) ? _stream->readUint32LE() : _stream->readUint32BE();
				uint index = (paletteEntry >> 24) & 0xFF;

				_palette[index].r = ((paletteEntry >> 16) & 0xFF) << 2;
				_palette[index].g = ((paletteEntry >> 8) & 0xFF) << 2;
				_palette[index].b = (paletteEntry & 0xFF) << 2;

				_colorCount = MAX<int>(_colorCount, index);
			}

		/*
		} else {
			stream.seek(colorCount, )
			data += colorCount * 4;
		}
		*/
		chunkType = (!isBigEndian) ? _stream->readUint32LE() : _stream->readUint32BE();
	}

	if (chunkType == CELS___SS) {
		chunkSize = (!isBigEndian) ? _stream->readUint32LE() : _stream->readUint32BE();
	} else {
		warning("SpriteAsset::parseSprite() Expected chunk type %08X, got %08X", CELS___SS, chunkType);
	}

	return chunkSize;

}

void SpriteAsset::loadFrameHeader(SpriteAssetFrame &frameHeader, bool isBigEndian) {
	_stream->readUint32LE();
	frameHeader.stream = (!isBigEndian) ? _stream->readUint32LE() : _stream->readUint32BE();
	frameHeader.x = (!isBigEndian) ? _stream->readUint32LE() : _stream->readUint32BE();
	frameHeader.y = (!isBigEndian) ? _stream->readUint32LE() : _stream->readUint32BE();
	frameHeader.w = (!isBigEndian) ? _stream->readUint32LE() : _stream->readUint32BE();
	frameHeader.h = (!isBigEndian) ? _stream->readUint32LE() : _stream->readUint32BE();
	frameHeader.comp = (!isBigEndian) ? _stream->readUint32LE() : _stream->readUint32BE();
	frameHeader.frame = NULL;
	_stream->seek(8 * 4, SEEK_CUR);
	//printf("SpriteAsset::loadFrameHeader() stream = %d; x = %d; y = %d; w = %d; h = %d; comp = %d\n", frameHeader.stream, frameHeader.x, frameHeader.y, frameHeader.w, frameHeader.h, frameHeader.comp);
}

M4Sprite *SpriteAsset::getFrame(int frameIndex) {
	return _frames[frameIndex].frame;
}

void SpriteAsset::loadStreamingFrame(M4Sprite *frame, int frameIndex, int destX, int destY) {
	uint32 frameOffset = _frameStartOffset + _frameOffsets[frameIndex];
	_stream->seek(frameOffset);

	SpriteAssetFrame frameHeader;
	loadFrameHeader(frameHeader);

	if (frameHeader.w > 0 && frameHeader.h > 0) {
		Common::MemoryReadStream *frameData = _stream->readStream(getFrameSize(frameIndex));
		if (frameHeader.stream) {
			frame->loadDeltaRle(frameData, destX - frameHeader.x, destY - frameHeader.y);
		} else {
			frame->loadRle(frameData);
		}
		delete frameData;
	}

}

RGBList *SpriteAsset::getRgbList() {
	RGBList *result = new RGBList(_colorCount);
	Common::copy((byte *)&_palette[0], (byte *)&_palette[_colorCount], (byte *)result->data());
	return result;
}

void SpriteAsset::translate(RGBList *list, bool isTransparent) {
	for (int frameIndex = 0; frameIndex < _frameCount; ++frameIndex)
		_frames[frameIndex].frame->translate(list, isTransparent);
}

int32 SpriteAsset::getFrameSize(int index) {
	/*
	if (index + 1 == _frameCount) {
	} else {

	}
	*/
	return _frameOffsets[index + 1] - _frameOffsets[index];
}

AssetManager::AssetManager(M4Engine *vm) {

	_vm = vm;

	/* Initialize asset arrays */
	for (int i = 0; i < 256; i++) {
		_MACH[i] = NULL;
		_SEQU[i] = NULL;
		_DATA[i] = NULL;
		_CELS[i] = NULL;
	}

}

AssetManager::~AssetManager() {
	// unload all remaining assets
	clearAssets(kAssetTypeMACH, 0, 255);
	clearAssets(kAssetTypeSEQU, 0, 255);
	clearAssets(kAssetTypeCELS, 0, 255);
	clearAssets(kAssetTypeDATA, 0, 255);
}

bool AssetManager::clearAssets(AssetType assetType, int32 minHash, int32 maxHash) {

	minHash = MAX<int>(0, minHash);
	maxHash = MIN<int>(maxHash, 255);

	switch (assetType) {
	case kAssetTypeMACH:
		for (int i = minHash; i <= maxHash; i++)
			if (_MACH[i]) {
				delete _MACH[i];
				_MACH[i] = NULL;
			}
		break;
	case kAssetTypeSEQU:
		for (int i = minHash; i <= maxHash; i++)
			if (_SEQU[i]) {
				delete _SEQU[i];
				_SEQU[i] = NULL;
			}
		break;
	case kAssetTypeDATA:
		for (int i = minHash; i <= maxHash; i++)
			if (_DATA[i]) {
				delete _DATA[i];
				_DATA[i] = NULL;
			}
		break;
	case kAssetTypeCELS:
		for (int i = minHash; i <= maxHash; i++)
			if (_CELS[i]) {
				delete _CELS[i];
				_CELS[i] = NULL;
			}
		break;
	}

	// FIXME: no value is returned, returning true for now
	return true;
}

bool AssetManager::loadAsset(const char *assetName, RGB8 *palette) {

	printf("AssetManager::loadAsset() %s\n", assetName);

	// TODO, better use MemoryReadStreamEndian?
	//convertAssetToLE(assetData, assetSize);

	Common::SeekableReadStream *assetS = _vm->res()->get(assetName);

	while (assetS->pos() + 12 < assetS->size()) {
		uint32 chunkType, chunkSize, chunkHash;

		chunkType = assetS->readUint32LE();
		chunkSize = assetS->readUint32LE() - 12; // sub 12 for the chunk header
		chunkHash = assetS->readUint32LE();

		printf("hash = %d\n", chunkHash);

		// Until loading code is complete, so that chunks not fully read are skipped correctly
		uint32 nextOfs = assetS->pos() + chunkSize;

		switch (chunkType) {
		case CHUNK_MACH:
			printf("MACH\n");
			clearAssets(kAssetTypeMACH, chunkHash, chunkHash);
			_MACH[chunkHash] = new MachineAsset(_vm, assetS, chunkSize, assetName);
			break;
		case CHUNK_SEQU:
			printf("SEQU\n");
			clearAssets(kAssetTypeSEQU, chunkHash, chunkHash);
			_SEQU[chunkHash] = new SequenceAsset(_vm, assetS, chunkSize, assetName);
			break;
		case CHUNK_DATA:
			printf("DATA\n");
			clearAssets(kAssetTypeDATA, chunkHash, chunkHash);
			_DATA[chunkHash] = new DataAsset(_vm, assetS, chunkSize, assetName);
			break;
		case CHUNK_CELS:
			printf("CELS\n");
			clearAssets(kAssetTypeCELS, chunkHash, chunkHash);
			_CELS[chunkHash] = new SpriteAsset(_vm, assetS, chunkSize, assetName);
			break;
		default:
			printf("AssetManager::loadAsset() Unknown chunk type %08X\n", chunkType);
		}

		// Until loading code is complete (see above)
		assetS->seek(nextOfs);

	}

	_vm->res()->toss(assetName);

	// FIXME: no value is returned, returning true for now
	return true;
}

int32 AssetManager::addSpriteAsset(const char *assetName, int32 hash, RGB8 *palette) {

	bool alreadyLoaded = false;

	if (hash < 0) {
		for (int i = 0; i <= 255; i++) {
			if (_CELS[i] != NULL) {
				if (_CELS[i]->getName() == assetName) {
					alreadyLoaded = true;
					hash = i;
					break;
				}
			} else {
				hash = i;
				break;
			}
		}
	} else {
		alreadyLoaded = _CELS[hash] != NULL && _CELS[hash]->getName() == assetName;
	}

	/* Not loaded and no empty slots */
	if (hash < 0)
		return -1;

	if (!alreadyLoaded) {

		printf("AssetManager::addSpriteAsset() asset %s not loaded, loading into %d\n", assetName, hash);

		clearAssets(kAssetTypeCELS, hash, hash);

		Common::SeekableReadStream *assetS = _vm->res()->get(assetName);
		_CELS[hash] = new SpriteAsset(_vm, assetS, assetS->size(), assetName);
		_vm->res()->toss(assetName);

	} else {

		printf("AssetManager::addSpriteAsset()  asset %s already loaded in %d\n", assetName, hash);

		/* TODO/FIXME
		if (_CELS[hash].palOffset >= 0 && palette)
			restorePalette(palette, _CELS[hash].data + _CELS[hash].palOffset);
		*/

	}

	return hash;

}

void AssetManager::restorePalette(RGB8 *palette, byte *data) {
	// TODO
}

void AssetManager::convertAssetToLE(byte *assetData, uint32 assetSize) {

}

MachineAsset *AssetManager::getMachine(int32 hash) {
	assert(_MACH[hash] != NULL);
	return _MACH[hash];
}

SequenceAsset *AssetManager::getSequence(int32 hash) {
	assert(_SEQU[hash] != NULL);
	return _SEQU[hash];
}

DataAsset *AssetManager::getData(int32 hash) {
	assert(_DATA[hash] != NULL);
	return _DATA[hash];
}

SpriteAsset *AssetManager::getSprite(int32 hash) {
	assert(_CELS[hash] != NULL);
	return _CELS[hash];
}

M4Sprite *AssetManager::getSpriteFrame(int32 hash, int frameIndex) {
	assert(_CELS[hash] != NULL);
	return _CELS[hash]->getFrame(frameIndex);
}

int32 AssetManager::getSpriteFrameCount(int32 hash) {
	assert(_CELS[hash] != NULL);
	return _CELS[hash]->getCount();
}

} // End of namespace M4
