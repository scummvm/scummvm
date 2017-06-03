#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "common/stream.h"
#include "image/image_decoder.h"
#include "graphics/surface.h"

class MSNImageDecoder : public Image::ImageDecoder
{
public:
	MSNImageDecoder();
	virtual ~MSNImageDecoder();

	virtual void destroy();
	virtual bool loadStream(Common::SeekableReadStream &stream);
	virtual const Graphics::Surface *getSurface() const { return _surface; }
	virtual const byte *getPalette() const { return _palette; }

private:
	const Graphics::Surface *_surface;
	byte *_palette;
};

#endif
