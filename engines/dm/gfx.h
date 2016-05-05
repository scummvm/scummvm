#ifndef GFX_H
#define GFX_H

#include "common/scummsys.h"
#include "dm/dm.h"

namespace DM {

struct Frame;

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
	void drawFrame(Frame &f);
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
								  int16 srcWidth, uint16 destX, uint16 destY, byte *destBitmap, uint16 destWidth);
	inline void DisplayMan::blitToScreen(byte *srcBitmap, uint16 srcFromX, uint16 srcToX, uint16 srcFromY, uint16 srcToY,
								  int16 srcWidth, uint16 destX, uint16 destY);
	byte *getCurrentVgaBuffer();
	void updateScreen();
	void drawDungeon();
};

}



#endif
