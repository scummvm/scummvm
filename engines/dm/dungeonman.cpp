#include "dungeonman.h"
#include "gfx.h"
#include "common/file.h"
#include "common/memstream.h"



namespace DM {
// TODO: refactor direction into a class
int8 dirIntoStepCountEast[4] = {0 /* North */, 1 /* East */, 0 /* West */, -1 /* South */};
int8 dirIntoStepCountNorth[4] = {-1 /* North */, 0 /* East */, 1 /* West */, 0 /* South */};

void turnDirRight(direction &dir) { dir = (direction)((dir + 1) & 3); }


}

using namespace DM;

CreatureInfo gCreatureInfo[kCreatureTypeCount] = { // @ G0243_as_Graphic559_CreatureInfo
												   /* { CreatureAspectIndex, AttackSoundOrdinal, Attributes, GraphicInfo,
												   MovementTicks, AttackTicks, Defense, BaseHealth, Attack, PoisonAttack,
												   Dexterity, Ranges, Properties, Resistances, AnimationTicks, WoundProbabilities, AttackType } */
	{0,  4, 0x0482, 0x623D,   8, 20,  55, 150, 150, 240,  55, 0x1153, 0x299B, 0x0876, 0x0254, 0xFD40, 4},
	{1,  0, 0x0480, 0xA625,  15, 32,  20, 110,  80,  15,  20, 0x3132, 0x33A9, 0x0E42, 0x0384, 0xFC41, 3},
	{2,  6, 0x0510, 0x6198,   3,  5,  50,  10,  10,   0, 110, 0x1376, 0x710A, 0x0235, 0x0222, 0xFD20, 0},
	{3,  0, 0x04B4, 0xB225,  10, 21,  30,  40,  58,   0,  80, 0x320A, 0x96AA, 0x0B3C, 0x0113, 0xF910, 5},
	{4,  1, 0x0701, 0xA3B8,   9,  8,  45, 101,  90,   0,  65, 0x1554, 0x58FF, 0x0A34, 0x0143, 0xFE93, 4},
	{5,  0, 0x0581, 0x539D,  20, 18, 100,  60,  30,   0,  30, 0x1232, 0x4338, 0x0583, 0x0265, 0xFFD6, 3},
	{6,  3, 0x070C, 0x0020, 120, 10,   5, 165,   5,   0,   5, 0x1111, 0x10F1, 0x0764, 0x02F2, 0xFC84, 6},
	{7,  7, 0x0300, 0x0220, 185, 15, 170,  50,  40,   5,  10, 0x1463, 0x25C4, 0x06E3, 0x01F4, 0xFD93, 4}, /* Atari ST: AttackSoundOrdinal = 0 */
	{8,  2, 0x1864, 0x5225,  11, 16,  15,  30,  55,   0,  80, 0x1423, 0x4664, 0x0FC8, 0x0116, 0xFB30, 6},
	{9, 10, 0x0282, 0x71B8,  21, 14, 240, 120, 219,   0,  35, 0x1023, 0x3BFF, 0x0FF7, 0x04F3, 0xF920, 3}, /* Atari ST: AttackSoundOrdinal = 7 */
	{10,  2, 0x1480, 0x11B8,  17, 12,  25,  33,  20,   0,  40, 0x1224, 0x5497, 0x0F15, 0x0483, 0xFB20, 3},
	{11,  0, 0x18C6, 0x0225, 255,  8,  45,  80, 105,   0,  60, 0x1314, 0x55A5, 0x0FF9, 0x0114, 0xFD95, 1},
	{12, 11, 0x1280, 0x6038,   7,  7,  22,  20,  22,   0,  80, 0x1013, 0x6596, 0x0F63, 0x0132, 0xFA30, 4}, /* Atari ST: AttackSoundOrdinal = 8 */
	{13,  9, 0x14A2, 0xB23D,   5, 10,  42,  39,  90, 100,  88, 0x1343, 0x5734, 0x0638, 0x0112, 0xFA30, 4}, /* Atari ST: AttackSoundOrdinal = 0 */
	{14,  0, 0x05B8, 0x1638,  10, 20,  47,  44,  75,   0,  90, 0x4335, 0xD952, 0x035B, 0x0664, 0xFD60, 5},
	{15,  5, 0x0381, 0x523D,  18, 19,  72,  70,  45,  35,  35, 0x1AA1, 0x15AB, 0x0B93, 0x0253, 0xFFC5, 4},
	{16, 10, 0x0680, 0xA038,  13,  8,  28,  20,  25,   0,  41, 0x1343, 0x2148, 0x0321, 0x0332, 0xFC30, 3}, /* Atari ST: AttackSoundOrdinal = 7 */
	{17,  0, 0x04A0, 0xF23D,   1, 16, 180,   8,  28,  20, 150, 0x1432, 0x19FD, 0x0004, 0x0112, 0xF710, 4},
	{18, 11, 0x0280, 0xA3BD,  14,  6, 140,  60, 105,   0,  70, 0x1005, 0x7AFF, 0x0FFA, 0x0143, 0xFA30, 4}, /* Atari ST: AttackSoundOrdinal = 8 */
	{19,  0, 0x0060, 0xE23D,   5, 18,  15,  33,  61,   0,  65, 0x3258, 0xAC77, 0x0F56, 0x0117, 0xFC40, 5},
	{20,  8, 0x10DE, 0x0225,  25, 25,  75, 144,  66,   0,  50, 0x1381, 0x7679, 0x0EA7, 0x0345, 0xFD93, 3}, /* Atari ST: AttackSoundOrdinal = 0 */
	{21,  3, 0x0082, 0xA3BD,   7, 15,  33,  77, 130,   0,  60, 0x1592, 0x696A, 0x0859, 0x0224, 0xFC30, 4},
	{22,  0, 0x1480, 0x53BD,  10, 14,  68, 100, 100,   0,  75, 0x4344, 0xBDF9, 0x0A5D, 0x0124, 0xF920, 3},
	{23,  0, 0x38AA, 0x0038,  12, 22, 255, 180, 210,   0, 130, 0x6369, 0xFF37, 0x0FBF, 0x0564, 0xFB52, 5},
	{24,  1, 0x068A, 0x97BD,  13, 28, 110, 255, 255,   0,  70, 0x3645, 0xBF7C, 0x06CD, 0x0445, 0xFC30, 4}, /* Atari ST Version 1.0 1987-12-08 1987-12-11: Ranges = 0x2645 */
	{25,  0, 0x38AA, 0x0000,  12, 22, 255, 180, 210,   0, 130, 0x6369, 0xFF37, 0x0FBF, 0x0564, 0xFB52, 5},
	{26,  0, 0x38AA, 0x0000,  12, 22, 255, 180, 210,   0, 130, 0x6369, 0xFF37, 0x0FBF, 0x0564, 0xFB52, 5}};

int16 DM::ordinalToIndex(int16 val) { return val - 1; }
int16 DM::indexToOrdinal(int16 val) { return val + 1; }


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
	for (uint16 i = 0; i < 16; ++i) {
		if (_dunData.thingsData[i])
			delete[] _dunData.thingsData[i][0];
		delete[] _dunData.thingsData[i];
	}
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

// this is the number of uint16s the data has to be stored, not the lenght of the data in dungeon.dat!
unsigned char gThingDataWordCount[16] = {
	2,   /* Door */
	3,   /* Teleporter */
	2,   /* Text String */
	4,   /* Sensor */
	9,  /* Group */
	2,   /* Weapon */
	2,   /* Armour */
	2,   /* Scroll */
	2,   /* Potion */
	4,   /* Container */
	2,   /* Junk */
	0,   /* Unused */
	0,   /* Unused */
	0,   /* Unused */
	5,   /* Projectile */
	2    /* Explosion */
}; // @ G0235_auc_Graphic559_ThingDataByteCount

const Thing Thing::thingNone(0);
const Thing Thing::thingEndOfList(0xFFFE);


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
		_maps[i].wallSet = (WallSet)((tmp >> 4) & 0xF);
		_maps[i].floorSet = (FloorSet)(tmp & 0xF);
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
			_dunData.squareFirstThings[i] = Thing::thingNone;

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


	// load things
	for (uint16 thingType = kDoorThingType; thingType < kThingTypeTotal; ++thingType) {
		uint16 thingCount = _fileHeader.thingCounts[thingType];
		if (_messages.newGame) {
			// _fileHeader.thingCounts[thingType] = 1024; // TODO: what this?? check back later
		}
		uint16 thingStoreWordCount = gThingDataWordCount[thingType];
		if (!thingStoreWordCount || !thingCount)
			continue;

		if (_dunData.thingsData[thingType]) {
			delete[] _dunData.thingsData[thingType][0];
			delete[] _dunData.thingsData[thingType];
		}
		_dunData.thingsData[thingType] = new uint16*[thingCount];
		_dunData.thingsData[thingType][0] = new uint16[thingCount * thingStoreWordCount];
		for (uint16 i = 0; i < thingCount; ++i)
			_dunData.thingsData[thingType][i] = _dunData.thingsData[thingType][0] + i * thingStoreWordCount;

		if (thingType == kGroupThingType) {
			for (uint16 i = 0; i < thingCount; ++i)
				for (uint16 j = 0; j < thingStoreWordCount; ++j) {
					if (j == 2 || j == 3)
						_dunData.thingsData[thingType][i][j] = dunDataStream.readByte();
					else
						_dunData.thingsData[thingType][i][j] = dunDataStream.readUint16BE();
				}
		} else if (thingType == kProjectileThingType) {
			for (uint16 i = 0; i < thingCount; ++i) {
				_dunData.thingsData[thingType][i][0] = dunDataStream.readUint16BE();
				_dunData.thingsData[thingType][i][1] = dunDataStream.readUint16BE();
				_dunData.thingsData[thingType][i][2] = dunDataStream.readByte();
				_dunData.thingsData[thingType][i][3] = dunDataStream.readByte();
				_dunData.thingsData[thingType][i][4] = dunDataStream.readUint16BE();
			}
		} else {
			for (uint16 i = 0; i < thingCount; ++i)
				for (uint16 j = 0; j < thingStoreWordCount; ++j)
					_dunData.thingsData[thingType][i][j] = dunDataStream.readUint16BE();
		}
	}


	// load map data
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
	_currMap.index = mapIndex;
	_currMap.data = _dunData.mapData[mapIndex];
	_currMap.map = _maps + mapIndex;
	_currMap.width = _maps[mapIndex].width + 1;
	_currMap.height = _maps[mapIndex].height + 1;
	_currMap.colCumulativeSquareFirstThingCount
		= &_dunData.columnsCumulativeSquareThingCount[_dunData.mapsFirstColumnIndex[mapIndex]];
}

void DungeonMan::setCurrentMapAndPartyMap(uint16 mapIndex) {
	setCurrentMap(mapIndex);

	byte *metaMapData = _currMap.data[_currMap.width - 1] + _currMap.height;
	_vm->_displayMan->_currMapAllowedCreatureTypes = metaMapData;

	metaMapData += _currMap.map->creatureTypeCount;
	memcpy(_vm->_displayMan->_currMapWallOrnIndices, metaMapData, _currMap.map->wallOrnCount);

	metaMapData += _currMap.map->wallOrnCount;
	memcpy(_vm->_displayMan->_currMapFloorOrnIndices, metaMapData, _currMap.map->floorOrnCount);

	metaMapData += _currMap.map->wallOrnCount;
	memcpy(_vm->_displayMan->_currMapDoorOrnIndices, metaMapData, _currMap.map->doorOrnCount);

	_currMapInscriptionWallOrnIndex = _currMap.map->wallOrnCount;
	_vm->_displayMan->_currMapWallOrnIndices[_currMapInscriptionWallOrnIndex] = kWallOrnInscription;

}


Square DungeonMan::getSquare(int16 mapX, int16 mapY) {
	bool isInXBounds = (mapX >= 0) && (mapX < _currMap.width);
	bool isInYBounds = (mapY >= 0) && (mapY < _currMap.height);

	if (isInXBounds && isInYBounds)
		return _currMap.data[mapX][mapY];


	Square tmpSquare;
	if (isInYBounds) {
		tmpSquare.set(_currMap.data[0][mapY]);
		if (mapX == -1 && (tmpSquare.getType() == kCorridorElemType || tmpSquare.getType() == kPitElemType))
			return Square(kWallElemType).set(kWallEastRandOrnAllowed);

		tmpSquare.set(_currMap.data[_currMap.width - 1][mapY]);
		if (mapX == _currMap.width && (tmpSquare.getType() == kCorridorElemType || tmpSquare.getType() == kPitElemType))
			return Square(kWallElemType).set(kWallWestRandOrnAllowed);
	} else if (isInXBounds) {
		tmpSquare.set(_currMap.data[mapX][0]);
		if (mapY == -1 && (tmpSquare.getType() == kCorridorElemType || tmpSquare.getType() == kPitElemType))
			return Square(kWallElemType).set(kWallSouthRandOrnAllowed);

		tmpSquare.set(_currMap.data[mapX][_currMap.height - 1]);
		if (mapY == _currMap.height && (tmpSquare.getType() == kCorridorElemType || tmpSquare.getType() == kPitElemType))
			return (kWallElemType << 5) | kWallNorthRandOrnAllowed;
	}

	return Square(kWallElemType);
}

Square DungeonMan::getRelSquare(direction dir, int16 stepsForward, int16 stepsRight, int16 posX, int16 posY) {
	mapCoordsAfterRelMovement(dir, stepsForward, stepsForward, posX, posY);
	return getSquare(posX, posY);
}

int16 DungeonMan::getSquareFirstThingIndex(int16 mapX, int16 mapY) {
	if (mapX < 0 || mapX >= _currMap.width || mapY < 0 || mapY >= _currMap.height || !Square(_currMap.data[mapX][mapY]).get(kThingListPresent))
		return -1;

	int16 y;
	uint16 index = _currMap.colCumulativeSquareFirstThingCount[mapX];
	byte* square = _currMap.data[mapX];
	while (y++ != mapY)
		if (Square(*square++).get(kThingListPresent))
			index++;

	return index;
}

Thing DungeonMan::getSquareFirstThing(int16 mapX, int16 mapY) {
	int16 index = getSquareFirstThingIndex(mapX, mapY);
	if (index == -1)
		return Thing::thingEndOfList;
	return _dunData.squareFirstThings[index];
}

enum SquareAspectIndice {
	kElemAspect = 0,
	kFirstGroupOrObjectAspect = 1,
	kRightWallOrnOrdAspect = 2,
	kFrontWallOrnOrdAspect = 3,
	kLeftWallOrnOrdAspect = 4,
	kPitInvisibleAspect = 2,
	kTeleporterVisibleAspect = 2,
	kStairsUpAspect = 2,
	kDoorStateAspect = 2,
	kDoorThingIndexAspect = 3,
	kFloorOrnOrdAspect = 4
};


void DungeonMan::setSquareAspect(uint16 *aspectArray, direction dir, int16 mapX, int16 mapY) {
	bool leftOrnAllowed, rightOrnAllowed, frontOrnAllowed;
	bool squareIsFakeWall;

	Thing thing = getSquareFirstThing(mapX, mapY);
	Square square = getSquare(mapX, mapY);

	memset(aspectArray, 0, 5 * sizeof(int16));
	aspectArray[kElemAspect] = square.getType();

	_vm->_displayMan->_championPortraitOrdinal = 0; // BUG0_75
	switch (square.getType()) {
	case kWallElemType:
		switch (dir) {
		case kDirNorth:
			leftOrnAllowed = square.get(kWallEastRandOrnAllowed);
			frontOrnAllowed = square.get(kWallSouthRandOrnAllowed);
			rightOrnAllowed = square.get(kWallWestRandOrnAllowed);
			break;
		case kDirEast:
			leftOrnAllowed = square.get(kWallSouthRandOrnAllowed);
			frontOrnAllowed = square.get(kWallWestRandOrnAllowed);
			rightOrnAllowed = square.get(kWallNorthRandOrnAllowed);
			break;
		case kDirSouth:
			leftOrnAllowed = square.get(kWallWestRandOrnAllowed);
			frontOrnAllowed = square.get(kWallNorthRandOrnAllowed);
			rightOrnAllowed = square.get(kWallEastRandOrnAllowed);
			break;
		case kDirWest:
			leftOrnAllowed = square.get(kWallNorthRandOrnAllowed);
			frontOrnAllowed = square.get(kWallEastRandOrnAllowed);
			rightOrnAllowed = square.get(kWallSouthRandOrnAllowed);
			break;
		}

		squareIsFakeWall = false;
		setSquareAspectOrnOrdinals(aspectArray, leftOrnAllowed, frontOrnAllowed, rightOrnAllowed, dir, mapX, mapY, squareIsFakeWall);

		while ((thing != Thing::thingEndOfList) && (thing.getType() <= kSensorThingType)) {
			int16 sideIndex = (thing.getCell() - dir) & 3;
			if (sideIndex) {
				if (thing.getType() == kTextstringType) {
					if (TextString(getThingData(thing)).isVisible()) {
						aspectArray[sideIndex + 1] = _currMapInscriptionWallOrnIndex + 1;

					} else {
						Sensor sensor(getThingData(thing));
						aspectArray[sideIndex + 1] = sensor.getOrnOrdinal();
						if (sensor.getType() == kSensorWallChampionPortrait) {
							_vm->_displayMan->_championPortraitOrdinal = indexToOrdinal(sensor.getData());
						}
					}
				}
			}
			thing = getNextThing(thing);
		}
		if (squareIsFakeWall && (_currMap.partyPosX != mapX) && (_currMap.partyPosY != mapY)) {
			aspectArray[kFirstGroupOrObjectAspect] = Thing::thingEndOfList.toUint16();
		}
		break;
	}
	aspectArray[kFirstGroupOrObjectAspect] = thing.toUint16();
}

void DungeonMan::setSquareAspectOrnOrdinals(uint16 *aspectArray, bool leftAllowed, bool frontAllowed, bool rightAllowed, direction dir,
											int16 mapX, int16 mapY, bool isFakeWall) {
	int16 ornCount = _currMap.map->randWallOrnCount;

	turnDirRight(dir);
	aspectArray[kRightWallOrnOrdAspect] = getRandomOrnOrdinal(leftAllowed, ornCount, mapX, ++mapY * (dir + 1), 30);
	turnDirRight(dir);
	aspectArray[kFrontWallOrnOrdAspect] = getRandomOrnOrdinal(frontAllowed, ornCount, mapX, ++mapY * (dir + 1), 30);
	turnDirRight(dir);
	aspectArray[kLeftWallOrnOrdAspect] = getRandomOrnOrdinal(rightAllowed, ornCount, mapX, ++mapY * (dir + 1), 30);

	if (isFakeWall || mapX < 0 || mapX >= _currMap.width || mapY < 0 || mapY >= _currMap.height) {
		for (uint16 i = kRightWallOrnOrdAspect; i <= kLeftWallOrnOrdAspect; ++i) {
			if (isWallOrnAnAlcove(ordinalToIndex(aspectArray[i])))
				aspectArray[i] = 0;
		}
	}
}

int16 DungeonMan::getRandomOrnOrdinal(bool allowed, int16 count, int16 mapX, int16 mapY, int16 modulo) {
	int16 index = (((((2000 + (mapX << 5) + mapY) * 31417) >> 1)
					+ (3000 + (_currMap.index << 6) + _currMap.width + _currMap.height) * 11
					+ _fileHeader.ornamentRandomSeed) >> 2) % modulo;
	if (allowed && index < count)
		return indexToOrdinal(index);
	return 0;
}


bool DungeonMan::isWallOrnAnAlcove(int16 wallOrnIndex) {
	if (wallOrnIndex >= 0)
		for (uint16 i = 0; i < kAlcoveOrnCount; ++i)
			if (_vm->_displayMan->_currMapAlcoveOrnIndices[i] == wallOrnIndex)
				return true;
	return false;
}

uint16 *DungeonMan::getThingData(Thing thing) {
	return _dunData.thingsData[thing.getType()][thing.getIndex()];
}

Thing DungeonMan::getNextThing(Thing thing) {
	return getThingData(thing)[0]; // :)
}

