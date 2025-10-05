#ifndef IMAGE_DOODLE_DECODER_H
#define IMAGE_DOODLE_DECODER_H

#include "image/image_decoder.h"
#include "common/stream.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

namespace Image {

class DoodleDecoder : public ImageDecoder {
public:
	/**
	 * Constructor for the DoodleDecoder
	 * @param palette Pointer to RGB palette data (16 colors * 3 components)
	 */
	DoodleDecoder(const byte *palette);
	~DoodleDecoder() override;

	// ImageDecoder interface
	bool loadStream(Common::SeekableReadStream &stream) override;
	void destroy() override;
	const Graphics::Surface *getSurface() const override { return _surface; }
	const Graphics::Palette &getPalette() const override { return _palette; }

	/**
	 * Load a C64 doodle image from its component streams
	 * @param highresStream Stream containing high-resolution pixel data
	 * @param colorStream1 Stream containing first color data file
	 * @param colorStream2 Stream containing second color data file
	 * @return Whether loading succeeded
	 */
	bool loadStreams(Common::SeekableReadStream &highresStream,
	                 Common::SeekableReadStream &colorStream1,
	                 Common::SeekableReadStream &colorStream2);

private:
	static const int kWidth = 320;
	static const int kHeight = 200;
	static const int kHeaderSize = 192;
	static const int kColorDataSize = 1000;  // 40x25 color cells

	Graphics::Surface *_surface;
	Graphics::Palette _palette;

	/**
	 * Process an 8x8 cell of the image
	 * @param cellIdx Cell index (0-999)
	 * @param highresData Pointer to high-resolution pixel data
	 * @param colorData1 First color data array
	 * @param colorData2 Second color data array
	 */
	void processDoodleCell(int cellIdx, const byte *highresData,
	                       const byte *colorData1, const byte *colorData2);
};

} // End of namespace Image

#endif
