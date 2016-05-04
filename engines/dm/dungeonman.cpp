#include "dungeonman.h"
#include "common/file.h"


namespace DM {
DungeonMan::DungeonMan(DMEngine *dmEngine) : _vm(dmEngine), _dungeonDataSize(0), _dungeonData(NULL) {}

DungeonMan::~DungeonMan() {
	delete[] _dungeonData;
}

void DungeonMan::loadDungeonFile() {
	Common::File f;
	f.open("Dungeon.dat");
	if (f.readUint16BE() == 0x8104) {
		_dungeonDataSize = f.readUint32BE();
		_dungeonData = new byte[_dungeonDataSize];
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
		while (uncompIndex < _dungeonDataSize) {
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
				_dungeonData[uncompIndex++] = common[(wordBuff >> 13) & 3];
				bitsUsedInWord += 3;
			} else if (((wordBuff >> 14) & 3) == 2) {
				_dungeonData[uncompIndex++] = lessCommon[(wordBuff >> 10) & 15];
				bitsUsedInWord += 6;
			} else if (((wordBuff >> 14) & 3) == 3) {
				_dungeonData[uncompIndex++] = (wordBuff >> 6) & 255;
				bitsUsedInWord += 10;
			}
		}
	}
	f.close();
}

}