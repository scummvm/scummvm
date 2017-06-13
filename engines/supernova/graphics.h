#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "common/scummsys.h"
#include "image/image_decoder.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct Surface;
}

namespace Supernova {

class MSNImageDecoder : public Image::ImageDecoder {
public:
	MSNImageDecoder();
	virtual ~MSNImageDecoder();

	virtual void destroy();
	virtual bool loadStream(Common::SeekableReadStream &stream);
	virtual const Graphics::Surface *getSurface() const { return _surface; }
	virtual const byte *getPalette() const { return _palette; }
	
	bool loadSection(int _section);

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
