#ifndef GFX_H
#define GFX_H

#include "common/scummsys.h"
#include "dm/dm.h"

namespace DM {

class DisplayMan {
	DMEngine *_vm;
	byte *_currPalette;
	uint16 _screenWidth;
	uint16 _screenHeight;
	byte *_vgaBuffer;
	uint16 _itemCount;
	uint32 *_indexBytePos;
	uint8 *_compressedData;
	DisplayMan(const DisplayMan &other); // no implementation on purpose
	void operator=(const DisplayMan &rhs); // no implementation on purpose
public:
	DisplayMan(DMEngine *dmEngine);
	~DisplayMan();
	void setUpScreens(uint16 width, uint16 height);
	void loadGraphics();
	void setPalette(byte *buff, uint16 colorCount);
	void loadIntoBitmap(uint16 index, byte *destBitmap);
	uint16 getImageWidth(uint16 index);
	uint16 getImageHeight(uint16 index);
	void blitToScreen(byte *srcBitmap, uint16 srcWidth, uint16 srcHeight, uint16 destX, uint16 destY);
	byte *getCurrentVgaBuffer();
	void updateScreen();
};

}



#endif
