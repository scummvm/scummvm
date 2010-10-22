#ifndef PRISONER_SCREEN_H
#define PRISONER_SCREEN_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/util.h"

#include "graphics/surface.h"

#include "prisoner/prisoner.h"
#include "prisoner/resource.h"

namespace Prisoner {

class Screen {
public:
	Screen(PrisonerEngine *vm);
	~Screen();

	void update();

	void putPixel(int x, int y, byte color);
	void drawLine(int x1, int y1, int x2, int y2, byte color);
	void hLine(int x, int y, int x2, byte color);
	void vLine(int x, int y, int y2, byte color);
	void fillRect(int x1, int y1, int x2, int y2, byte color);
	void frameRect(int x1, int y1, int x2, int y2, byte color);

	void setPartialPalette(byte *palette, int start, int count);
	void setFullPalette(byte *palette);

	void clear();

	Graphics::Surface *getScreen() const {
		return _workScreen;
	}

	void drawAnimationCelSprite(AnimationCel &cel, int16 x, int16 y, uint16 flags = 0);
	void drawAnimationElement(AnimationResource *animation, int16 elementIndex, int16 x, int16 y, uint16 parentFlags = 0);
	void drawAnimationCommand(AnimationResource *animation, AnimationCommand *cmd, int16 x, int16 y, uint16 parentFlags = 0);

	int16 getTextWidth(FontResource *fontResource, const Common::String &text);
	void drawChar(const FontChar *fontChar, const byte *fontCharData, int16 x, int16 y);
	void drawText(FontResource *fontResource, const Common::String &text, int16 x, int16 y);

	void setClipRect(int clipX1, int clipY1, int clipX2, int clipY2);

	void copyRectFrom(Graphics::Surface *source, int sourceX, int sourceY, int destX, int destY, int width, int height);

	void setFontColorTable(byte index, byte color);
	void initPaletteTransTable(byte colorIncr);
	void buildPaletteTransTable(byte *sourcePalette, byte color);
	void drawTransparentRect(int16 x1, int16 y1, int16 x2, int16 y2);

//protected:

	PrisonerEngine *_vm;

	Graphics::Surface *_workScreen;

	int _clipX1, _clipY1, _clipX2, _clipY2;
	byte _fontColorTable[256];

	/* Palette transparency */
	byte _pttTable1[256], _pttTable2[256];
	byte _pttColorIncr;
	byte _paletteTransTable[256];

protected:
	// FIXME: Remove static vars
	static int *gfxPrimitivesPolyInts;
	static uint gfxPrimitivesPolyAllocated;

};

}

#endif
