#ifndef STARTREK_BITMAP_H
#define STARTREK_BITMAP_H

#include "startrek/filestream.h"

#include "common/ptr.h"
#include "common/stream.h"

namespace StarTrek {

struct Bitmap {
	int16 xoffset;
	int16 yoffset;
	int16 width;
	int16 height;
	byte *pixels;

	Bitmap(Common::SharedPtr<FileStream> stream);
	Bitmap(const Bitmap &bitmap);
	Bitmap(int w, int h);
	~Bitmap();

protected:
	int32 pixelsArraySize;
	Bitmap() : xoffset(0), yoffset(0), width(0), height(0), pixels(nullptr), pixelsArraySize(0) {}
};


/**
 * TextBitmap is the same as Bitmap, except it stores character indices in its "pixels"
 * array instead of actual pixels.
 * A consequence of this is that the pixels array is smaller than otherwise expected
 * (since width/height still reflect the actual size when drawn).
 */
struct TextBitmap : Bitmap {
	TextBitmap(int w, int h);
};


/**
 * StubBitmap is a bitmap without any actual pixel data. Used as a stub for the
 * "starfield" sprite, which is always in draw mode 1 (invisible), so it never gets drawn;
 * however, it does trigger refreshes on the background in that area, so the game can draw
 * on the background layer manually.
 */
struct StubBitmap : Bitmap {
	StubBitmap(int w, int h);
};

}

#endif
