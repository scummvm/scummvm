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
	// TODO: will probably be redundant
	uint32 *_packedItemPos;
	// TODO: will probably be reundant
	byte *_packedBitmaps; // TODO: this doesn't not contaion graphics exclusively, will have to be moved
	DisplayMan(const DisplayMan &other); // no implementation on purpose
	void operator=(const DisplayMan &rhs); // no implementation on purpose


	byte **_unpackedBitmaps;
	void unpackGraphics();
	void drawFrameToScreen(byte *bitmap, Frame &f, Color transparent);
	void drawFrameToBitMap(byte *bitmap, Frame &f, Color transparent, byte *destBitmap, uint16 destWidth);
public:
	DisplayMan(DMEngine *dmEngine);
	~DisplayMan();
	void setUpScreens(uint16 width, uint16 height);
	void loadGraphics();
	void loadPalette(dmPaletteEnum palette);
	// TODO: will probably be redundant with public visibility
	void loadIntoBitmap(uint16 index, byte *destBitmap);
	uint16 getImageWidth(uint16 index);
	uint16 getImageHeight(uint16 index);
	void DisplayMan::blitToBitmap(byte *srcBitmap, uint16 srcFromX, uint16 srcToX, uint16 srcFromY, uint16 srcToY,
								  int16 srcWidth, uint16 destX, uint16 destY, byte *destBitmap, uint16 destWidth, Color transparent = colorNoTransparency);
	void DisplayMan::blitToScreen(byte *srcBitmap, uint16 srcFromX, uint16 srcToX, uint16 srcFromY, uint16 srcToY,
								  int16 srcWidth, uint16 destX, uint16 destY, Color transparent = colorNoTransparency);
	void flipBitmapVertical(byte *bitmap, uint16 width, uint16 height);
	void flipBitmapHorizontal(byte *bitmap, uint16 width, uint16 height);
	void clearBitmap(byte *bitmap, uint16 width, uint16 height, Color color);
	byte *getCurrentVgaBuffer();
	void updateScreen();
	void drawDungeon();
	void clearScreen(Color color);
};

}



#endif
