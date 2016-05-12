#ifndef GFX_H
#define GFX_H

#include "common/scummsys.h"
#include "dm/dm.h"

namespace DM {

struct Frame;
enum WallSet {
	kWallSetStone = 0 // @ C0_WALL_SET_STONE
};

enum FloorSet {
	kFloorSetStone = 0 // @ C0_FLOOR_SET_STONE
};

enum Color {
	kColorNoTransparency = 255,
	kColorBlack = 0,
	kColorDarkGary = 1,
	kColorLightGray = 2,
	kColorDarkBrown = 3,
	kColorCyan = 4,
	kColorLightBrown = 5,
	kColorDarkGreen = 6,
	kColorLightGreen = 7,
	kColorRed = 8,
	kColorGold = 9,
	kColorFlesh = 10,
	kColorYellow = 11,
	kColorDarkestGray = 12,
	kColorLightestGray = 13,
	kColorBlue = 14,
	kColorWhite = 15
};

enum dmPaletteEnum {
	kPalSwoosh = 0,
	kPalMousePointer = 1,
	kPalCredits = 2,
	kPalEntrance = 3,
	kPalDungeonView0 = 4,
	kPalDungeonView1 = 5,
	kPalDungeonView2 = 6,
	kPalDungeonView3 = 7,
	kPalDungeonView4 = 8,
	kPalDungeonView5 = 9,
};

struct Viewport {
	// TODO: should probably add width and height, seems redundant right meow
	uint16 posX, posY;
};

extern Viewport gDefultViewPort;
extern Viewport gDungeonViewport;



class DisplayMan {
	DMEngine *_vm;
	dmPaletteEnum _currPalette;
	uint16 _screenWidth;
	uint16 _screenHeight;
	byte *_vgaBuffer;

	uint16 _packedItemCount;
	uint32 *_packedItemPos;
	byte *_packedBitmaps; // TODO: this doesn't not contaion graphics exclusively, will have to be moved

	byte **_bitmaps;


	// the last two pointers are owned by this array
	byte *_wallSetBitMaps[15] = {NULL};	// @G[0696..0710]_puc_Bitmap_WallSet_...

	// pointers are not owned by these fields
	byte *_floorBitmap;
	byte *_ceilingBitmap;

	DisplayMan(const DisplayMan &other); // no implementation on purpose
	void operator=(const DisplayMan &rhs); // no implementation on purpose

	byte *getCurrentVgaBuffer();
	// the original functions has two position parameters, but they are always set to zero
	void loadIntoBitmap(uint16 index, byte *destBitmap); // @ F0466_EXPAND_GraphicToBitmap
	void unpackGraphics();
	void drawWallSetBitmap(byte *bitmap, Frame &f); // @ F0100_DUNGEONVIEW_DrawWallSetBitmap
	void drawSquareD3L(direction dir, int16 posX, int16 posY); // @ F0116_DUNGEONVIEW_DrawSquareD3L
public:
	DisplayMan(DMEngine *dmEngine);
	~DisplayMan();
	void setUpScreens(uint16 width, uint16 height);

	void loadGraphics();
	void loadWallSet(WallSet set); // @ F0095_DUNGEONVIEW_LoadWallSet
	void loadFloorSet(FloorSet set); // @ F0094_DUNGEONVIEW_LoadFloorSet

	void loadPalette(dmPaletteEnum palette);

	/// Gives the width of an IMG0 type item
	uint16 width(uint16 index);
	/// Gives the height of an IMG1 type item
	uint16 height(uint16 index);

	void blitToBitmap(byte *srcBitmap, uint16 srcWidth, uint16 srcX, uint16 srcY,
					  byte *destBitmap, uint16 destWidth,
					  uint16 destFromX, uint16 destToX, uint16 destFromY, uint16 destToY,
					  Color transparent = kColorNoTransparency, Viewport &viewport = gDefultViewPort);
	void blitToBitmap(byte *srcBitmap, uint16 srcWidth, uint16 srcHeight, byte *destBitmap, uint16 destWidth, uint16 destX = 0, uint16 destY = 0);
	void blitToScreen(byte *srcBitmap, uint16 srcWidth, uint16 srcX, uint16 srcY,
					  uint16 destFromX, uint16 destToX, uint16 destFromY, uint16 destToY,
					  Color transparent = kColorNoTransparency, Viewport &viewport = gDefultViewPort);

	void flipBitmapHorizontal(byte *bitmap, uint16 width, uint16 height);
	void flipBitmapVertical(byte *bitmap, uint16 width, uint16 height);

	void clearBitmap(byte *bitmap, uint16 width, uint16 height, Color color);
	void clearScreen(Color color);
	void drawDungeon(direction dir, int16 posX, int16 posY); // @ F0128_DUNGEONVIEW_Draw_CPSF
	void updateScreen();
};

}



#endif
