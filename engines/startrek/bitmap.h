#ifndef STARTREK_BITMAP_H
#define STARTREK_BITMAP_H

#include "startrek/filestream.h"

#include "common/ptr.h"
#include "common/stream.h"

namespace StarTrek {

struct Bitmap {
	uint16 xoffset;
	uint16 yoffset;
	uint16 width;
	uint16 height;
	byte *pixels;

	Bitmap(Common::SharedPtr<FileStream> stream);
	Bitmap(int w, int h);
	~Bitmap();

protected:
	Bitmap() : xoffset(0),yoffset(0),width(0),height(0),pixels(nullptr) {}
};


// TextBitmap is the same as Bitmap, except it stores character indices in its "pixels"
// array instead of actual pixels.
// A consequence of this is that the pixels array is smaller than otherwise expected
// (since width/height still reflect the actual size when drawn).
struct TextBitmap : Bitmap {
	TextBitmap(int w, int h);
};

}

#endif
