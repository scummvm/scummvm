#include "common/file.h"
#include "common/stream.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "engines/freescape/doodle.h"

namespace Image {

DoodleDecoder::DoodleDecoder(const byte *palette) : _surface(nullptr), _palette(palette, 16) {
}

DoodleDecoder::~DoodleDecoder() {
	destroy();
}

void DoodleDecoder::destroy() {
	if (_surface) {
		_surface->free();
		delete _surface;
		_surface = nullptr;
	}
}

bool DoodleDecoder::loadStream(Common::SeekableReadStream &stream) {
	// This method should not be used
	error("DoodleDecoder::loadStream - Use loadStreams instead");
	return false;
}

bool DoodleDecoder::loadStreams(Common::SeekableReadStream &highresStream,
                                Common::SeekableReadStream &colorStream1,
                                Common::SeekableReadStream &colorStream2) {
	destroy();

	// Check stream sizes
	if (highresStream.size() < 8002) {
		error("DoodleDecoder: Invalid high-resolution data size");
		return false;
	}

	if (colorStream1.size() < 1002 || colorStream2.size() < 1002) {
		error("DoodleDecoder: Invalid color data size");
		return false;
	}

	// Skip headers
	highresStream.skip(2);
	colorStream1.skip(2);
	colorStream2.skip(2);

	// Read high-resolution data
	byte *highresData = new byte[8000];
	highresStream.read(highresData, 8000);

	// Skip remaining header bytes in highres file
	highresStream.skip(190);

	// Read color data
	byte *colorData1 = new byte[kColorDataSize];
	byte *colorData2 = new byte[kColorDataSize];

	colorStream1.read(colorData1, kColorDataSize);
	colorStream2.read(colorData2, kColorDataSize);

	int width = 320;
	int height = 200;

	// Create surface
	_surface = new Graphics::Surface();
	_surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());

	// Process each cell
	for (int cellIdx = 0; cellIdx < 1000; ++cellIdx) {
		processDoodleCell(cellIdx, highresData, colorData1, colorData2);
	}

	// Cleanup
	delete[] highresData;
	delete[] colorData1;
	delete[] colorData2;

	return true;
}

void DoodleDecoder::processDoodleCell(int cellIdx, const byte *highresData,
                                      const byte *colorData1, const byte *colorData2) {
	// Calculate cell coordinates
	int cellX = (cellIdx % 40) * 8;
	int cellY = (cellIdx / 40) * 8;

	// Get colors for this cell
	byte color1 = colorData2[cellIdx] & 0xf;
	byte color2 = colorData1[cellIdx] >> 4;
	byte color3 = colorData1[cellIdx] & 0xf;

	// Process each row in the cell
	for (int row = 0; row < 8; ++row) {
		byte pixelByte = highresData[cellIdx * 8 + row];

		// Process 4 pixel pairs (8 pixels total) in multicolor mode
		for (int bit = 0; bit < 4; ++bit) {
			byte pixelPair = (pixelByte >> (6 - bit * 2)) & 0x03;

			// Determine final color
			byte color = 0;
			switch (pixelPair) {
			case 0:
				color = 0;
				break;
			case 1:
				color = color2;
				break;
			case 2:
				color = color3;
				break;
			case 3:
				color = color1;
				break;
			}

			// Set pixel colors (2 pixels for each pair due to multicolor mode)
			_surface->setPixel(cellX + bit * 2, cellY + row, color);
			_surface->setPixel(cellX + bit * 2 + 1, cellY + row, color);
		}
	}
}

} // End of namespace Image
