#include "gfx.h"
#include "engines/util.h"
#include "common/system.h"
#include "common/file.h"
#include "graphics/palette.h"
#include "common/endian.h"


namespace DM {

// this is for the Amiga version, later when we add support for more versions, this will have to be renamed
uint16 dmPalettes[10][16] = {
	{0x000, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0x000, 0xFFF, 0xAAA, 0xFFF, 0xAAA, 0x444, 0xFF0, 0xFF0},
	{0x000, 0x666, 0x888, 0x620, 0x0CC, 0x840, 0x080, 0x0C0, 0xF00, 0xFA0, 0xC86, 0xFF0, 0x000, 0xAAA, 0x00F, 0xFFF},
	{0x006, 0x0AA, 0xFF6, 0x840, 0xFF8, 0x000, 0x080, 0xA00, 0xC84, 0xFFA, 0xF84, 0xFC0, 0xFA0, 0x000, 0x620, 0xFFC},
	{0x000, 0x666, 0x888, 0x840, 0xCA8, 0x0C0, 0x080, 0x0A0, 0x864, 0xF00, 0xA86, 0x642, 0x444, 0xAAA, 0x620, 0xFFF},
	{0x000, 0x666, 0x888, 0x620, 0x0CC, 0x840, 0x080, 0x0C0, 0xF00, 0xFA0, 0xC86, 0xFF0, 0x444, 0xAAA, 0x00F, 0xFFF},
	{0x000, 0x444, 0x666, 0x620, 0x0CC, 0x820, 0x060, 0x0A0, 0xC00, 0x000, 0x000, 0xFC0, 0x222, 0x888, 0x00C, 0xCCC},
	{0x000, 0x222, 0x444, 0x420, 0x0CC, 0x620, 0x040, 0x080, 0xA00, 0x000, 0x000, 0xFA0, 0x000, 0x666, 0x00A, 0xAAA},
	{0x000, 0x000, 0x222, 0x200, 0x0CC, 0x420, 0x020, 0x060, 0x800, 0x000, 0x000, 0xC80, 0x000, 0x444, 0x008, 0x888},
	{0x000, 0x000, 0x000, 0x000, 0x0CC, 0x200, 0x000, 0x040, 0x600, 0x000, 0x000, 0xA60, 0x000, 0x222, 0x006, 0x666},
	{0x000, 0x000, 0x000, 0x000, 0x0CC, 0x000, 0x000, 0x020, 0x400, 0x000, 0x000, 0x640, 0x000, 0x000, 0x004, 0x444}
};


enum GraphicIndice {
	FloorGraphIndice = 75,
	CeilingGraphIndice = 76
};

struct Frame {
	// srcWidth and srcHeight (present in the original sources) is redundant here, can be deduced from gaphicsIndice
	// these coorinates are inclusive boundaries, when blitting you gotta add +1 to srcTo fields
	uint16 srcFromX, srcToX, srcFromY, srcToY;
	uint16 srcWidth, srcHeight;
	uint16 destX, destY;
};

Frame ceilingFrame = {0, 223, 0, 28, 224, 29, 0, 0};
Frame floorFrame = {0, 223, 66, 135, 224, 70, 0, 0};

}

using namespace DM;

DisplayMan::DisplayMan(DMEngine *dmEngine) :
	_vm(dmEngine), _currPalette(palSwoosh), _screenWidth(0), _screenHeight(0),
	_vgaBuffer(NULL), _itemCount(0), _packedItemPos(NULL), _packedBitmaps(NULL),
	_unpackedBitmaps(NULL) {}

DisplayMan::~DisplayMan() {
	delete[] _packedBitmaps;
	delete[] _packedItemPos;
	delete[] _vgaBuffer;
	delete[] _unpackedBitmaps;
}

void DisplayMan::setUpScreens(uint16 width, uint16 height) {
	_screenWidth = width;
	_screenHeight = height;
	loadPalette(palSwoosh);
	_vgaBuffer = new byte[_screenWidth * _screenHeight];
	clearScreen(colorBlack);
}

void DisplayMan::loadGraphics() {
	Common::File f;
	f.open("graphics.dat");

	_itemCount = f.readUint16BE();
	_packedItemPos = new uint32[_itemCount + 1];
	_packedItemPos[0] = 0;
	for (uint16 i = 1; i < _itemCount + 1; ++i)
		_packedItemPos[i] = f.readUint16BE() + _packedItemPos[i - 1];

	_packedBitmaps = new uint8[_packedItemPos[_itemCount]];

	f.seek(2 + _itemCount * 4);
	for (uint32 i = 0; i < _packedItemPos[_itemCount]; ++i)
		_packedBitmaps[i] = f.readByte();

	f.close();

	unpackGraphics();
}

void DisplayMan::unpackGraphics() {
	uint32 unpackedBitmapsSize = 0;
	for (uint16 i = 0; i <= 20; ++i)
		unpackedBitmapsSize += getImageWidth(i) * getImageHeight(i);
	for (uint16 i = 22; i <= 532; ++i)
		unpackedBitmapsSize += getImageWidth(i) * getImageHeight(i);
	_unpackedBitmaps = new byte*[533];
	// graphics items go from 0-20 and 22-532 inclusive, _unpackedItemPos 21 and 22 are there for indexing convenience
	_unpackedBitmaps[0] = new byte[unpackedBitmapsSize];
	loadIntoBitmap(0, _unpackedBitmaps[0]);
	for (uint16 i = 1; i <= 20; ++i) {
		_unpackedBitmaps[i] = _unpackedBitmaps[i - 1] + getImageWidth(i - 1) * getImageHeight(i - 1);
		loadIntoBitmap(i, _unpackedBitmaps[i]);
	}
	_unpackedBitmaps[22] = _unpackedBitmaps[20] + getImageWidth(20) * getImageHeight(20);
	for (uint16 i = 23; i < 533; ++i) {
		_unpackedBitmaps[i] = _unpackedBitmaps[i - 1] + getImageWidth(i - 1) * getImageHeight(i - 1);
		loadIntoBitmap(i, _unpackedBitmaps[i]);
	}
}

void DisplayMan::loadPalette(dmPaletteEnum palette) {
	if (_currPalette == palette)
		return;

	byte colorPalette[16 * 3];
	for (int i = 0; i < 16; ++i) {
		colorPalette[i * 3] = (dmPalettes[palette][i] >> 8) * (256 / 16);
		colorPalette[i * 3 + 1] = (dmPalettes[palette][i] >> 4) * (256 / 16);
		colorPalette[i * 3 + 2] = dmPalettes[palette][i] * (256 / 16);
	}
	_vm->_system->getPaletteManager()->setPalette(colorPalette, 0, 16);
	_currPalette = palette;
}

#define TOBE2(byte1, byte2) ((((uint16)(byte1)) << 8) | (uint16)(byte2))

void DisplayMan::loadIntoBitmap(uint16 index, byte *destBitmap) {
	uint8 *data = _packedBitmaps + _packedItemPos[index];
	uint16 width = TOBE2(data[0], data[1]);
	uint16 height = TOBE2(data[2], data[3]);
	uint16 nextByteIndex = 4;
	for (uint16 k = 0; k < width * height;) {
		uint8 nextByte = data[nextByteIndex++];
		uint8 nibble1 = (nextByte & 0xF0) >> 4;
		uint8 nibble2 = (nextByte & 0x0F);
		if (nibble1 <= 7) {
			for (int j = 0; j < nibble1 + 1; ++j)
				destBitmap[k++] = nibble2;
		} else if (nibble1 == 0x8) {
			uint8 byte1 = data[nextByteIndex++];
			for (int j = 0; j < byte1 + 1; ++j)
				destBitmap[k++] = nibble2;
		} else if (nibble1 == 0xC) {
			uint16 word1 = TOBE2(data[nextByteIndex], data[nextByteIndex + 1]);
			nextByteIndex += 2;
			for (int j = 0; j < word1 + 1; ++j)
				destBitmap[k++] = nibble2;
		} else if (nibble1 == 0xB) {
			uint8 byte1 = data[nextByteIndex++];
			for (int j = 0; j < byte1 + 1; ++j, ++k)
				destBitmap[k] = destBitmap[k - width];
			destBitmap[k++] = nibble2;
		} else if (nibble1 == 0xF) {
			uint16 word1 = TOBE2(data[nextByteIndex], data[nextByteIndex + 1]);
			nextByteIndex += 2;
			for (int j = 0; j < word1 + 1; ++j, ++k)
				destBitmap[k] = destBitmap[k - width];
			destBitmap[k++] = nibble2;
		} else if (nibble1 == 9) {
			uint8 byte1 = data[nextByteIndex++];
			if (byte1 % 2)
				byte1++;
			else
				destBitmap[k++] = nibble2;

			for (int j = 0; j < byte1 / 2; ++j) {
				uint8 byte2 = data[nextByteIndex++];
				destBitmap[k++] = byte2 & 0x0F;
				destBitmap[k++] = (byte2 & 0xF0) >> 4;
			}
		}
	}
}

void DisplayMan::blitToBitmap(byte *srcBitmap, uint16 srcFromX, uint16 srcToX, uint16 srcFromY, uint16 srcToY,
							  int16 srcWidth, uint16 destX, uint16 destY,
							  byte *destBitmap, uint16 destWidth, Color transparent) {
	for (uint16 y = 0; y < srcToY - srcFromY; ++y)
		for (uint16 x = 0; x < srcToX - srcFromX; ++x) {
			byte srcPixel = srcBitmap[srcWidth*(y + srcFromY) + srcFromX + x];
			if (srcPixel != transparent)
				destBitmap[destWidth * (y + destY) + destX + x] = srcPixel;
		}
}

void DisplayMan::blitToScreen(byte *srcBitmap, uint16 srcFromX, uint16 srcToX, uint16 srcFromY, uint16 srcToY,
							  int16 srcWidth, uint16 destX, uint16 destY, Color transparent) {
	blitToBitmap(srcBitmap, srcFromX, srcToX, srcFromY, srcToY, srcWidth, destX, destY, getCurrentVgaBuffer(), _screenWidth, transparent);
}

void DisplayMan::updateScreen() {
	_vm->_system->copyRectToScreen(_vgaBuffer, _screenWidth, 0, 0, _screenWidth, _screenHeight);
	_vm->_system->updateScreen();
}

byte *DisplayMan::getCurrentVgaBuffer() {
	return _vgaBuffer;
}

uint16 DisplayMan::getImageWidth(uint16 index) {
	byte *data = _packedBitmaps + _packedItemPos[index];
	return TOBE2(data[0], data[1]);
}

uint16 DisplayMan::getImageHeight(uint16 index) {
	uint8 *data = _packedBitmaps + _packedItemPos[index];
	return TOBE2(data[2], data[3]);
}

void DisplayMan::drawFrameToScreen(byte *bitmap, Frame &f, Color transparent) {
	blitToScreen(bitmap, f.srcFromX, f.srcToX + 1, f.srcFromY, f.srcToY + 1, f.srcWidth, f.destX, f.destY, transparent);
}

void DisplayMan::drawFrameToBitMap(byte *bitmap, Frame &f, Color transparent, byte *destBitmap, uint16 destWidth) {
	blitToBitmap(bitmap, f.srcFromX, f.srcToX + 1, f.srcFromY, f.srcToY + 1, f.srcWidth, f.destX, f.destY, destBitmap, destWidth, transparent);
}

void DisplayMan::drawDungeon() {
	loadPalette(palDungeonView0);
	drawFrameToScreen(_unpackedBitmaps[CeilingGraphIndice], ceilingFrame, colorFlesh);
}

void DisplayMan::clearScreen(Color color) {
	memset(getCurrentVgaBuffer(), color, sizeof(byte) * _screenWidth * _screenHeight);
}