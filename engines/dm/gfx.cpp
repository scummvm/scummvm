#include "gfx.h"
#include "engines/util.h"
#include "common/system.h"
#include "common/file.h"
#include "graphics/palette.h"
#include "common/endian.h"

using namespace DM;

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


DisplayMan::DisplayMan(DMEngine *dmEngine) :
	_vm(dmEngine), _currPalette(palSwoosh), _screenWidth(0), _screenHeight(0),
	_vgaBuffer(0), _itemCount(0), _indexBytePos(NULL), _compressedData(NULL) {}

DisplayMan::~DisplayMan() {
	delete[] _compressedData;
	delete[] _indexBytePos;
	delete[] _vgaBuffer;
}

void DisplayMan::setUpScreens(uint16 width, uint16 height) {
	_screenWidth = width;
	_screenHeight = height;
	loadPalette(palSwoosh);
	_vgaBuffer = new byte[_screenWidth * _screenHeight];
	memset(_vgaBuffer, 0, width * height);
}

void DisplayMan::loadGraphics() {
	Common::File f;
	f.open("graphics.dat");

	_itemCount = f.readUint16BE();
	_indexBytePos = new uint32[_itemCount + 1];
	_indexBytePos[0] = 0;
	for (uint16 i = 1; i < _itemCount + 1; ++i)
		_indexBytePos[i] = f.readUint16BE() + _indexBytePos[i - 1];

	_compressedData = new uint8[_indexBytePos[_itemCount]];

	f.seek(2 + _itemCount * 4);
	for (uint32 i = 0; i < _indexBytePos[_itemCount]; ++i)
		_compressedData[i] = f.readByte();

	f.close();
}

void DisplayMan::loadPalette(dmPaletteEnum palette) {
	byte colorPalette[16 * 3];
	for (int i = 0; i < 16; ++i) {
		colorPalette[i * 3] = (dmPalettes[palette][i] >> 8) * (256 / 16);
		colorPalette[i * 3 + 1] = (dmPalettes[palette][i] >> 4) * (256 / 16);
		colorPalette[i * 3 + 2] = dmPalettes[palette][i] * (256 / 16);
	}
	_vm->_system->getPaletteManager()->setPalette(colorPalette, 0, 16);
}

#define TOBE2(byte1, byte2) ((((uint16)(byte1)) << 8) | (uint16)(byte2))

void DisplayMan::loadIntoBitmap(uint16 index, byte *destBitmap) {
	uint8 *data = _compressedData + _indexBytePos[index];
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

void DisplayMan::blitToScreen(byte *srcBitmap, uint16 srcWidth, uint16 srcHeight, uint16 destX, uint16 destY) {
	for (uint16 y = 0; y < srcHeight; ++y)
		memcpy(getCurrentVgaBuffer() + ((y + destY) * _screenWidth + destX), srcBitmap + y * srcWidth, srcWidth * sizeof(byte));
}

void DisplayMan::updateScreen() {
	_vm->_system->copyRectToScreen(_vgaBuffer, _screenWidth, 0, 0, _screenWidth, _screenHeight);
	_vm->_system->updateScreen();
}

byte *DisplayMan::getCurrentVgaBuffer() {
	return _vgaBuffer;
}

uint16 DisplayMan::getImageWidth(uint16 index) {
	uint8 *data = _compressedData + _indexBytePos[index];
	return TOBE2(data[0], data[1]);
}

uint16 DisplayMan::getImageHeight(uint16 index) {
	uint8 *data = _compressedData + _indexBytePos[index];
	return TOBE2(data[2], data[3]);
}

