#ifndef GFX_H
#define GFX_H

#include "common/scummsys.h"
#include "dm/dm.h"

namespace DM {

struct Frame;
enum Color {
	colorNoTransparency = 255,
	colorBlack = 0,
	colorDarkGary = 1,
	colorLightGray = 2,
	colorDarkBrown = 3,
	colorCyan = 4,
	colorLightBrown = 5,
	colorDarkGreen = 6,
	colorLightGreen = 7,
	colorRed = 8,
	colorGold = 9,
	colorFlesh = 10,
	colorYellow = 11,
	colorDarkestGray = 12,
	colorLightestGray = 13,
	colorBlue = 14,
	colorWhite = 15
};

enum dmPaletteEnum {
	palSwoosh = 0,
	palMousePointer = 1,
	palCredits = 2,
	palEntrance = 3,
	palDungeonView0 = 4,
	palDungeonView1 = 5,
	palDungeonView2 = 6,
	palDungeonView3 = 7,
	palDungeonView4 = 8,
	palDungeonView5 = 9,
};




class DisplayMan {
	DMEngine *_vm;
	dmPaletteEnum _currPalette;
	uint16 _screenWidth;
	uint16 _screenHeight;
	byte *_vgaBuffer;
	uint16 _itemCount;
	uint32 *_packedItemPos;
	byte *_packedBitmaps; // TODO: this doesn't not contaion graphics exclusively, will have to be moved
	DisplayMan(const DisplayMan &other); // no implementation on purpose
	void operator=(const DisplayMan &rhs); // no implementation on purpose

	byte **_bitmaps;
	byte *getCurrentVgaBuffer();
	void loadIntoBitmap(uint16 index, byte *destBitmap);
	void unpackGraphics();
	void drawWallSetBitmap(byte *bitmap, Frame &f, uint16 srcWidth);
public:
	DisplayMan(DMEngine *dmEngine);
	~DisplayMan();
	void setUpScreens(uint16 width, uint16 height);
	void loadGraphics();
	void loadPalette(dmPaletteEnum palette);

	/// Gives the width of an IMG0 type item
	uint16 width(uint16 index);
	/// Gives the height of an IMG1 type item
	uint16 height(uint16 index);

	void blitToBitmap(byte *srcBitmap, uint16 srcWidth, uint16 srcX, uint16 srcY,
					  byte *destBitmap, uint16 destWidth,
					  uint16 destFromX, uint16 destToX, uint16 destFromY, uint16 destToY,
					  Color transparent = colorNoTransparency);
	void blitToBitmap(byte *srcBitmap, uint16 srcWidth, uint16 srcHeight, byte *destBitmap, uint16 destWidth, uint16 destX = 0, uint16 destY = 0);
	void blitToScreen(byte *srcBitmap, uint16 srcWidth, uint16 srcX, uint16 srcY,
					  uint16 destFromX, uint16 destToX, uint16 destFromY, uint16 destToY,
					  Color transparent = colorNoTransparency);

	void flipBitmapHorizontal(byte *bitmap, uint16 width, uint16 height);
	void flipBitmapVertical(byte *bitmap, uint16 width, uint16 height);

	void clearBitmap(byte *bitmap, uint16 width, uint16 height, Color color);
	void clearScreen(Color color);
	void drawDungeon(direction dir, uint16 posX, uint16 posY);
	void updateScreen();
};

}



#endif
