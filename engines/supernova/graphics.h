#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "common/scummsys.h"
#include "image/image_decoder.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
class Surface;
}

namespace Supernova {

const byte initPalette[48] = {
//   r   g   b
	 0,  0,  0,  16, 16, 16,  22, 22, 22,
	28, 28, 28,  63, 63, 63,   0, 52,  0,
	 0, 63,  0,  54,  0,  0,  63,  0,  0,
	 0,  0, 30,   0,  0, 45,  40, 40, 40,
	20, 50, 63,  10, 63, 10,  60, 60,  0,
	63, 10, 10
};

// TODO
const byte defaultVGAPalette[] = {
	0, 0, 0
};

class MSNImageDecoder : public Image::ImageDecoder {
public:
	MSNImageDecoder();
	virtual ~MSNImageDecoder();

	virtual void destroy();
	virtual bool loadStream(Common::SeekableReadStream &stream);
	virtual const Graphics::Surface *getSurface() const { return _surface; }
	virtual const byte *getPalette() const { return _palette; }
	
	bool loadSection(int _section);

private:
	static const int kMaxSections = 50;
	static const int kMaxClickFields = 80;
	
	Graphics::Surface *_surface;
	byte *_palette;
	byte *_encodedImage;
	
	struct Section {
		int16  x1;
		int16  x2;
		byte   y1;
		byte   y2;
		byte   next;
		uint16 addressLow;
		byte   addressHigh;
	} _section[kMaxSections];

	struct ClickField {
		int16  x1;
		int16  x2;
		byte   y1;
		byte   y2;
		byte   next;
	} _clickField[kMaxClickFields];
};

}
#endif
