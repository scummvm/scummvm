#include "dungeonman.h"
#include "common/file.h"
#include "common/memstream.h"



namespace DM {
// TODO: refactor direction into a class
int8 dirIntoStepCountEast[4] = {0 /* North */, 1 /* East */, 0 /* West */, -1 /* South */};
int8 dirIntoStepCountNorth[4] = {-1 /* North */, 0 /* East */, 1 /* West */, 0 /* South */};

void turnDirRight(direction &dir) { dir = (direction)((dir + 1) & 3); }

}

using namespace DM;


void DungeonMan::mapCoordsAfterRelMovement(direction dir, int16 stepsForward, int16 stepsRight, int16 &posX, int16 &posY) {
	posX += dirIntoStepCountEast[dir] * stepsForward;
	posY += dirIntoStepCountNorth[dir] * stepsForward;
	turnDirRight(dir);
	posX += dirIntoStepCountEast[dir] * stepsRight;
	posY += dirIntoStepCountNorth[dir] * stepsRight;
}

DungeonMan::DungeonMan(DMEngine *dmEngine) : _vm(dmEngine), _rawDunFileData(NULL), _maps(NULL), _rawMapData(NULL) {}

DungeonMan::~DungeonMan() {
	delete[] _rawDunFileData;
	delete[] _maps;
	delete[] _dunData.mapsFirstColumnIndex;
	delete[] _dunData.columnsCumulativeSquareThingCount;
	delete[] _dunData.squareFirstThings;
	delete[] _dunData.textData;
	delete[] _dunData.mapData;
}

void DungeonMan::decompressDungeonFile() {
	Common::File f;
	f.open("Dungeon.dat");
	if (f.readUint16BE() == 0x8104) {
		_rawDunFileDataSize = f.readUint32BE();
		if (_rawDunFileData) delete[] _rawDunFileData;
		_rawDunFileData = new byte[_rawDunFileDataSize];
		f.readUint16BE();
		byte common[4];
		for (uint16 i = 0; i < 4; ++i)
			common[i] = f.readByte();
		byte lessCommon[16];
		for (uint16 i = 0; i < 16; ++i)
			lessCommon[i] = f.readByte();

		// start unpacking
		uint32 uncompIndex = 0;
		uint8 bitsUsedInWord = 0;
		uint16 wordBuff = f.readUint16BE();
		uint8 bitsLeftInByte = 8;
		byte byteBuff = f.readByte();
		while (uncompIndex < _rawDunFileDataSize) {
			while (bitsUsedInWord != 0) {
				uint8 shiftVal;
				if (f.eos()) {
					shiftVal = bitsUsedInWord;
					wordBuff <<= shiftVal;
				} else {
					shiftVal = MIN(bitsLeftInByte, bitsUsedInWord);
					wordBuff <<= shiftVal;
					wordBuff |= (byteBuff >> (8 - shiftVal));
					byteBuff <<= shiftVal;
					bitsLeftInByte -= shiftVal;
					if (!bitsLeftInByte) {
						byteBuff = f.readByte();
						bitsLeftInByte = 8;
					}
				}
				bitsUsedInWord -= shiftVal;
			}
			if (((wordBuff >> 15) & 1) == 0) {
				_rawDunFileData[uncompIndex++] = common[(wordBuff >> 13) & 3];
				bitsUsedInWord += 3;
			} else if (((wordBuff >> 14) & 3) == 2) {
				_rawDunFileData[uncompIndex++] = lessCommon[(wordBuff >> 10) & 15];
				bitsUsedInWord += 6;
			} else if (((wordBuff >> 14) & 3) == 3) {
				_rawDunFileData[uncompIndex++] = (wordBuff >> 6) & 255;
				bitsUsedInWord += 10;
			}
		}
	} else {
		// TODO: if the dungeon is uncompressed, read it here
	}
	f.close();
}


uint8 gAdditionalThingCounts[16] = {
	0,    /* Door */
	0,    /* Teleporter */
	0,    /* Text String */
	0,    /* Sensor */
	75,   /* Group */
	100,  /* Weapon */
	120,  /* Armour */
	0,    /* Scroll */
	5,    /* Potion */
	0,    /* Container */
	140,  /* Junk */
	0,    /* Unused */
	0,    /* Unused */
	0,    /* Unused */
	60,   /* Projectile */
	50    /* Explosion */
}; // @ G0236_auc_Graphic559_AdditionalThingCounts

// TODO: refactor THINGS into classes
unsigned char gThingDataByteCount[16] = {
	4,   /* Door */
	6,   /* Teleporter */
	4,   /* Text String */
	8,   /* Sensor */
	16,  /* Group */
	4,   /* Weapon */
	4,   /* Armour */
	4,   /* Scroll */
	4,   /* Potion */
	8,   /* Container */
	4,   /* Junk */
	0,   /* Unused */
	0,   /* Unused */
	0,   /* Unused */
	8,   /* Projectile */
	4    /* Explosion */
}; // @ G0235_auc_Graphic559_ThingDataByteCount

const Thing Thing::specThingNone(0, 0, 0);


void DungeonMan::loadDungeonFile() {
	if (_messages.newGame)
		decompressDungeonFile();

	Common::MemoryReadStream dunDataStream(_rawDunFileData, _fileHeader.rawMapDataSize, DisposeAfterUse::NO);

	// initialize _fileHeader
	_fileHeader.dungeonId = _fileHeader.ornamentRandomSeed = dunDataStream.readUint16BE();
	_fileHeader.rawMapDataSize = dunDataStream.readUint16BE();
	_fileHeader.mapCount = dunDataStream.readByte();
	dunDataStream.readByte(); // discard 1 byte
	_fileHeader.textDataWordCount = dunDataStream.readUint16BE();
	uint16 partyPosition = dunDataStream.readUint16BE();
	_fileHeader.partyStartDir = (direction)((partyPosition >> 10) & 3);
	_fileHeader.partyStartPosY = (partyPosition >> 5) & 0x1F;
	_fileHeader.partyStartPosX = (partyPosition >> 0) & 0x1F;
	_fileHeader.squareFirstThingCount = dunDataStream.readUint16BE();
	for (uint16 i = 0; i < kThingTypeTotal; ++i)
		_fileHeader.thingCounts[i] = dunDataStream.readUint16BE();

	// init party position and mapindex
	if (_messages.newGame) {
		_currMap.partyDir = _fileHeader.partyStartDir;
		_currMap.partyPosX = _fileHeader.partyStartPosX;
		_currMap.partyPosY = _fileHeader.partyStartPosY;
		_currMap.currPartyMapIndex = 0;
	}

	// load map data
	if (_maps) delete[] _maps;

	_maps = new Map[_fileHeader.mapCount];
	for (uint16 i = 0; i < _fileHeader.mapCount; ++i) {
		_maps[i].rawDunDataOffset = dunDataStream.readUint16BE();
		dunDataStream.readUint32BE(); // discard 4 bytes
		_maps[i].offsetMapX = dunDataStream.readByte();
		_maps[i].offsetMapY = dunDataStream.readByte();

		uint16 tmp = dunDataStream.readUint16BE();
		_maps[i].height = tmp >> 11;
		_maps[i].width = (tmp >> 6) & 0x1F;
		_maps[i].level = tmp & 0x1F; // Only used in DMII

		tmp = dunDataStream.readUint16BE();
		_maps[i].randFloorOrnCount = tmp >> 12;
		_maps[i].floorOrnCount = (tmp >> 8) & 0xF;
		_maps[i].randWallOrnCount = (tmp >> 4) & 0xF;
		_maps[i].wallOrnCount = tmp & 0xF;

		tmp = dunDataStream.readUint16BE();
		_maps[i].difficulty = tmp >> 12;
		_maps[i].creatureTypeCount = (tmp >> 4) & 0xF;
		_maps[i].doorOrnCount = tmp & 0xF;

		tmp = dunDataStream.readUint16BE();
		_maps[i].doorSet1 = (tmp >> 12) & 0xF;
		_maps[i].doorSet0 = (tmp >> 8) & 0xF;
		_maps[i].wallSet = (tmp >> 4) & 0xF;
		_maps[i].floorSet = tmp & 0xF;
	}

	// TODO: ??? is this - begin
	if (_dunData.mapsFirstColumnIndex) delete[] _dunData.mapsFirstColumnIndex;

	_dunData.mapsFirstColumnIndex = new uint16[_fileHeader.mapCount];
	uint16 columCount = 0;
	for (uint16 i = 0; i < _fileHeader.mapCount; ++i) {
		_dunData.mapsFirstColumnIndex[i] = columCount;
		columCount += _maps[i].width + 1;
	}
	_dunData.columCount = columCount;
	// TODO: ??? is this - end

	if (_messages.newGame) // TODO: what purpose does this serve?
		_fileHeader.squareFirstThingCount += 300;

	// TODO: ??? is this - begin
	if (_dunData.columnsCumulativeSquareThingCount)
		delete[] _dunData.columnsCumulativeSquareThingCount;
	_dunData.columnsCumulativeSquareThingCount = new uint16[columCount];
	for (uint16 i = 0; i < columCount; ++i)
		_dunData.columnsCumulativeSquareThingCount[i] = dunDataStream.readUint16BE();
	// TODO: ??? is this - end

	// TODO: ??? is this - begin
	if (_dunData.squareFirstThings)
		delete[] _dunData.squareFirstThings;
	_dunData.squareFirstThings = new Thing[_fileHeader.squareFirstThingCount];
	for (uint16 i = 0; i < _fileHeader.squareFirstThingCount; ++i)
		_dunData.squareFirstThings[i].set(dunDataStream.readUint16BE());
	if (_messages.newGame)
		for (uint16 i = 0; i < 300; ++i)
			_dunData.squareFirstThings[i] = Thing::specThingNone;

	// TODO: ??? is this - end

	// load text data
	if (_dunData.textData)
		delete[] _dunData.textData;
	_dunData.textData = new uint16[_fileHeader.textDataWordCount];
	for (uint16 i = 0; i < _fileHeader.textDataWordCount; ++i)
		_dunData.textData[i] = dunDataStream.readUint16BE();

	// TODO: ??? what this
	if (_messages.newGame)
		_dunData.eventMaximumCount = 100;

	// load 'Things'
	// TODO: implement load things
	// this is a temporary workaround to seek to raw map data
	for (uint16 i = 0; i < kThingTypeTotal; ++i)
		dunDataStream.skip(_fileHeader.thingCounts[i] * gThingDataByteCount[i]);

	_rawMapData = _rawDunFileData + dunDataStream.pos();

	if (_dunData.mapData) delete[] _dunData.mapData;

	if (!_messages.restartGameRequest) {
		uint8 mapCount = _fileHeader.mapCount;
		_dunData.mapData = new byte**[_dunData.columCount + mapCount];
		byte **colFirstSquares = (byte**)_dunData.mapData + mapCount;
		for (uint8 i = 0; i < mapCount; ++i) {
			_dunData.mapData[i] = colFirstSquares;
			byte *square = _rawMapData + _maps[i].rawDunDataOffset;
			*colFirstSquares++ = square;
			for (uint16 w = 0; w <= _maps[i].width; ++w) {
				square += _maps[w].height + 1;
				*colFirstSquares++ = square;
			}
		}
	}
}

void DungeonMan::setCurrentMap(uint16 mapIndex) {
	if (_currMap.index == mapIndex)
		return;

	_currMap.index = mapIndex;
	_currMap.data = _dunData.mapData[mapIndex];
	_currMap.map = _maps + mapIndex;
	_currMap.width = _maps[mapIndex].width + 1;
	_currMap.height = _maps[mapIndex].height + 1;
	_currMap.colCumulativeSquareFirstThingCount
		= &_dunData.columnsCumulativeSquareThingCount[_dunData.mapsFirstColumnIndex[mapIndex]];
}

byte DungeonMan::getSquare(int16 mapX, int16 mapY) {
	bool isInXBounds = (mapX >= 0) && (mapX < _currMap.width);
	bool isInYBounds = (mapY >= 0) && (mapY < _currMap.height);

	if (isInXBounds && isInYBounds)
		return _currMap.data[mapX][mapY];

	int16 tmpSquare;
	if (isInYBounds) {
		tmpSquare = getSquareType(_currMap.data[0][mapY]);
		if (mapX == -1 && (tmpSquare == kCorridorElemType || tmpSquare == kPitElemType))
			return (kWallElemType << 5) | kWallEastRandOrnAllowed;

		tmpSquare = getSquareType(_currMap.data[_currMap.width - 1][mapY]);
		if (mapX == _currMap.width && (tmpSquare == kCorridorElemType || tmpSquare == kPitElemType))
			return (kWallElemType << 5) | kWallWestRandOrnAllowed;
	} else if (isInXBounds) {
		tmpSquare = getSquareType(_currMap.data[mapX][0]);
		if (mapY == -1 && (tmpSquare == kCorridorElemType || tmpSquare == kPitElemType))
			return (kWallElemType << 5) | kWallSouthRandOrnAllowed;

		tmpSquare = getSquareType(_currMap.data[mapX][_currMap.height - 1]);
		if (mapY == _currMap.height && (tmpSquare == kCorridorElemType || tmpSquare == kPitElemType))
			return (kWallElemType << 5) | kWallNorthRandOrnAllowed;
	}

	return kWallElemType << 5;
}

byte DungeonMan::getRelSquare(direction dir, int16 stepsForward, int16 stepsRight, int16 posX, int16 posY) {
	mapCoordsAfterRelMovement(dir, stepsForward, stepsForward, posX, posY);
	return getSquare(posX, posY);
}