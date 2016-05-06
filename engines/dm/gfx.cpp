#include "gfx.h"
#include "engines/util.h"
#include "common/system.h"
#include "common/file.h"
#include "graphics/palette.h"
#include "common/endian.h"


namespace DM {

// TODO: this is ONLY for the Amiga version, name will have to be refactored
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
	floorIndice = 75,
	ceilingIndice = 76
};

struct Frame {
	// FIXME: these bundaries are inclusive, workaround by adding +1 in the drawFrame methods
	uint16 destFromX, destToX, destFromY, destToY;
	// srcWidth and srcHeight (present in the original sources) is redundant here, can be deduced from gaphicsIndice
	uint16 srcX, srcY;
};

Frame ceilingFrame = {0, 223, 0, 28, 0, 0};
Frame floorFrame = {0, 223, 66, 135, 0, 0};

}

using namespace DM;

DisplayMan::DisplayMan(DMEngine *dmEngine) :
	_vm(dmEngine), _currPalette(palSwoosh), _screenWidth(0), _screenHeight(0),
	_vgaBuffer(NULL), _itemCount(0), _packedItemPos(NULL), _packedBitmaps(NULL),
	_bitmaps(NULL) {}

DisplayMan::~DisplayMan() {
	delete[] _packedBitmaps;
	delete[] _packedItemPos;
	delete[] _vgaBuffer;
	delete[] _bitmaps;
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
		unpackedBitmapsSize += width(i) * height(i);
	for (uint16 i = 22; i <= 532; ++i)
		unpackedBitmapsSize += width(i) * height(i);
	// graphics items go from 0-20 and 22-532 inclusive, _unpackedItemPos 21 and 22 are there for indexing convenience
	_bitmaps = new byte*[533];
	_bitmaps[0] = new byte[unpackedBitmapsSize];
	loadIntoBitmap(0, _bitmaps[0]);
	for (uint16 i = 1; i <= 20; ++i) {
		_bitmaps[i] = _bitmaps[i - 1] + width(i - 1) * height(i - 1);
		loadIntoBitmap(i, _bitmaps[i]);
	}
	_bitmaps[22] = _bitmaps[20] + width(20) * height(20);
	for (uint16 i = 23; i < 533; ++i) {
		_bitmaps[i] = _bitmaps[i - 1] + width(i - 1) * height(i - 1);
		loadIntoBitmap(i, _bitmaps[i]);
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

void DisplayMan::blitToBitmap(byte *srcBitmap, uint16 srcWidth, uint16 srcX, uint16 srcY,
							  byte *destBitmap, uint16 destWidth,
							  uint16 destFromX, uint16 destToX, uint16 destFromY, uint16 destToY,
							  Color transparent) {
	for (uint16 y = 0; y < destToY - destFromY; ++y)
		for (uint16 x = 0; x < destToX - destFromX; ++x) {
			byte srcPixel = srcBitmap[srcWidth * (y + srcY) + srcX + x];
			if (srcPixel != transparent)
				destBitmap[destWidth * (y + destFromY) + destFromX + x] = srcPixel;
		}
}

void DisplayMan::blitToScreen(byte *srcBitmap, uint16 srcWidth, uint16 srcX, uint16 srcY,
							  uint16 destFromX, uint16 destToX, uint16 destFromY, uint16 destToY,
							  Color transparent) {
	blitToBitmap(srcBitmap, srcWidth, srcX, srcY,
				 getCurrentVgaBuffer(), _screenWidth, destFromX, destToX, destFromY, destToY, transparent);
}

void DisplayMan::blitToBitmap(byte *srcBitmap, uint16 srcWidth, uint16 srcHeight, byte *destBitmap, uint16 destWidth, uint16 destX, uint16 destY) {
	for (uint16 y = 0; y < srcHeight; ++y)
		memcpy(destBitmap + destWidth*(y + destY) + destX, srcBitmap + y * srcWidth, sizeof(byte)* srcWidth);
}

void DisplayMan::flipBitmapHorizontal(byte *bitmap, uint16 width, uint16 height) {
	for (uint16 y = 0; y < height; ++y)
		for (uint16 x = 0; x < width / 2; ++x) {
			byte tmp;
			tmp = bitmap[y*width + x];
			bitmap[y*width + x] = bitmap[y*width + width - 1 - x];
			bitmap[y*width + width - 1 - x] = tmp;
		}
}

void DisplayMan::flipBitmapVertical(byte *bitmap, uint16 width, uint16 height) {
	byte *tmp = new byte[width];

	for (uint16 y = 0; y < height / 2; ++y) {
		memcpy(tmp, bitmap + y * width, width);
		memcpy(bitmap + y * width, bitmap + (height - 1 - y) * width, width);
		memcpy(bitmap + (height - 1 - y) * width, tmp, width);
	}

	delete[] tmp;
}



void DisplayMan::updateScreen() {
	_vm->_system->copyRectToScreen(_vgaBuffer, _screenWidth, 0, 0, _screenWidth, _screenHeight);
	_vm->_system->updateScreen();
}

byte *DisplayMan::getCurrentVgaBuffer() {
	return _vgaBuffer;
}

uint16 DisplayMan::width(uint16 index) {
	byte *data = _packedBitmaps + _packedItemPos[index];
	return TOBE2(data[0], data[1]);
}

uint16 DisplayMan::height(uint16 index) {
	uint8 *data = _packedBitmaps + _packedItemPos[index];
	return TOBE2(data[2], data[3]);
}

void DisplayMan::drawWallSetBitmap(byte *bitmap, Frame &f, uint16 srcWidth) {
	blitToScreen(bitmap, srcWidth, f.srcX, f.srcY, f.destFromX, f.destToX + 1, f.destFromY, f.destToY + 1, colorFlesh);
}



void DisplayMan::drawDungeon(direction dir, uint16 posX, uint16 posY) {
	loadPalette(palDungeonView0);
	// TODO: this is a global variable, set from here
	bool flippedWallAndFootprints = (posX + posY + dir) & 1;

	// NOTE: this can hold every bitmap, width and height is "flexible"
	byte  *tmpBitmap = new byte[305 * 111];
	clearBitmap(tmpBitmap, 305, 111, colorBlack);

	if (flippedWallAndFootprints) {
		blitToBitmap(_bitmaps[floorIndice], width(floorIndice), height(floorIndice), tmpBitmap, width(floorIndice));
		flipBitmapHorizontal(tmpBitmap, width(floorIndice), height(floorIndice));
		drawWallSetBitmap(tmpBitmap, floorFrame, width(floorIndice));
		drawWallSetBitmap(_bitmaps[ceilingIndice], ceilingFrame, width(ceilingIndice));
	} else {
		blitToBitmap(_bitmaps[ceilingIndice], width(ceilingIndice), height(ceilingIndice), tmpBitmap, width(ceilingIndice));
		flipBitmapHorizontal(tmpBitmap, width(ceilingIndice), height(ceilingIndice));
		drawWallSetBitmap(tmpBitmap, ceilingFrame, width(ceilingIndice));
		drawWallSetBitmap(_bitmaps[floorIndice], floorFrame, width(floorIndice));
	}


	delete[] tmpBitmap;
}

void DisplayMan::clearScreen(Color color) {
	memset(getCurrentVgaBuffer(), color, sizeof(byte) * _screenWidth * _screenHeight);
}

void DisplayMan::clearBitmap(byte *bitmap, uint16 width, uint16 height, Color color) {
	memset(bitmap, color, sizeof(byte) * width * height);
}