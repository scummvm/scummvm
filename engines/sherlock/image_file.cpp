/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "sherlock/image_file.h"
#include "sherlock/screen.h"
#include "sherlock/sherlock.h"
#include "common/debug.h"
#include "common/memstream.h"

namespace Sherlock {

SherlockEngine *ImageFile::_vm;

void ImageFile::setVm(SherlockEngine *vm) {
	_vm = vm;
}

ImageFile::ImageFile() {
}

ImageFile::ImageFile(const Common::String &name, bool skipPal, bool animImages) {
	Common::SeekableReadStream *stream = _vm->_res->load(name);

	Common::fill(&_palette[0], &_palette[PALETTE_SIZE], 0);
	load(*stream, skipPal, animImages);

	delete stream;
}

ImageFile::ImageFile(Common::SeekableReadStream &stream, bool skipPal) {
	Common::fill(&_palette[0], &_palette[PALETTE_SIZE], 0);
	load(stream, skipPal, false);
}

ImageFile::~ImageFile() {
	for (uint idx = 0; idx < size(); ++idx)
		(*this)[idx]._frame.free();
}

void ImageFile::load(Common::SeekableReadStream &stream, bool skipPalette, bool animImages) {
	loadPalette(stream);

	int streamSize = stream.size();
	while (stream.pos() < streamSize) {
		ImageFrame frame;
		frame._width = stream.readUint16LE() + 1;
		frame._height = stream.readUint16LE() + 1;
		frame._paletteBase = stream.readByte();

		if (animImages) {
			// Animation cutscene image files use a 16-bit x offset
			frame._offset.x = stream.readUint16LE();
			frame._rleEncoded = (frame._offset.x & 0xff) == 1;
			frame._offset.y = stream.readByte();
		} else {
			// Standard image files have a separate byte for the RLE flag, and an 8-bit X offset
			frame._rleEncoded = stream.readByte() == 1;
			frame._offset.x = stream.readByte();
			frame._offset.y = stream.readByte();
		}

		frame._rleEncoded = !skipPalette && frame._rleEncoded;

		if (frame._paletteBase) {
			// Nibble packed frame data
			frame._size = (frame._width * frame._height) / 2;
		} else if (frame._rleEncoded) {
			// This size includes the header size, which we subtract
			frame._size = stream.readUint16LE() - 11;
			frame._rleMarker = stream.readByte();
		} else {
			// Uncompressed data
			frame._size = frame._width * frame._height;
		}

		// Load data for frame and decompress it
		byte *data1 = new byte[frame._size + 4];
		stream.read(data1, frame._size);
		Common::fill(data1 + frame._size, data1 + frame._size + 4, 0);
		frame.decompressFrame(data1, IS_ROSE_TATTOO);
		delete[] data1;

		push_back(frame);
	}
}

void ImageFile::loadPalette(Common::SeekableReadStream &stream) {
	// Check for palette
	uint16 width        = stream.readUint16LE() + 1;
	uint16 height       = stream.readUint16LE() + 1;
	byte   paletteBase  = stream.readByte();
	byte   rleEncoded   = stream.readByte();
	byte   offsetX      = stream.readByte();
	byte   offsetY      = stream.readByte();
	uint32 palSignature = 0;

	if ((width == 390) && (height == 2) && (!paletteBase) && (!rleEncoded) && (!offsetX) && (!offsetY)) {
		// We check for these specific values
		// We can't do "width * height", because at least the first German+Spanish menu bar is 60 x 13
		// which is 780, which is the size of the palette. We obviously don't want to detect it as palette.

		// As another security measure, we also check for the signature text
		palSignature = stream.readUint32BE();
		if (palSignature != MKTAG('V', 'G', 'A', ' ')) {
			// signature mismatch, rewind
			stream.seek(-12, SEEK_CUR);
			return;
		}
		// Found palette, so read it in
		stream.seek(8, SEEK_CUR); // Skip over the rest of the signature text "VGA palette"
		for (int idx = 0; idx < PALETTE_SIZE; ++idx)
			_palette[idx] = VGA_COLOR_TRANS(stream.readByte());
	} else {
		// Not a palette, so rewind to start of frame data for normal frame processing
		stream.seek(-8, SEEK_CUR);
	}
}

void ImageFrame::decompressFrame(const byte *src, bool isRoseTattoo) {
	_frame.create(_width, _height, Graphics::PixelFormat::createFormatCLUT8());
	byte *dest = (byte *)_frame.getPixels();
	Common::fill(dest, dest + _width * _height, 0xff);

	if (_paletteBase) {
		// Nibble-packed
		for (uint idx = 0; idx < _size; ++idx, ++src) {
			*dest++ = *src & 0xF;
			*dest++ = (*src >> 4);
		}
	} else if (_rleEncoded && isRoseTattoo) {
		// Rose Tattoo run length encoding doesn't use the RLE marker byte
		for (int yp = 0; yp < _height; ++yp) {
			int xSize = _width;
			while (xSize > 0) {
				// Skip a given number of pixels
				byte skip = *src++;
				dest += skip;
				xSize -= skip;
				if (!xSize)
					break;

				// Get a run length, and copy the following number of pixels
				int rleCount = *src++;
				xSize -= rleCount;
				while (rleCount-- > 0)
					*dest++ = *src++;
			}
			assert(xSize == 0);
		}
	} else if (_rleEncoded) {
		// RLE encoded
		int frameSize = _width * _height;
		while (frameSize > 0) {
			if (*src == _rleMarker) {
				byte rleColor = src[1];
				byte rleCount = MIN((int)src[2], frameSize);
				src += 3;
				frameSize -= rleCount;
				while (rleCount--)
					*dest++ = rleColor;
			} else {
				*dest++ = *src++;
				--frameSize;
			}
		}
	} else {
		// Uncompressed frame
		Common::copy(src, src + _width * _height, dest);
	}
}

/*----------------------------------------------------------------*/

int ImageFrame::sDrawXSize(int scaleVal) const {
	int width = _width;
	int scale = scaleVal == 0 ? 1 : scaleVal;

	if (scaleVal >= SCALE_THRESHOLD)
		--width;

	int result = width * SCALE_THRESHOLD / scale;
	if (scaleVal >= SCALE_THRESHOLD)
		++result;

	return result;
}

int ImageFrame::sDrawYSize(int scaleVal) const {
	int height = _height;
	int scale = scaleVal == 0 ? 1 : scaleVal;

	if (scaleVal >= SCALE_THRESHOLD)
		--height;

	int result = height * SCALE_THRESHOLD / scale;
	if (scaleVal >= SCALE_THRESHOLD)
		++result;

	return result;
}

int ImageFrame::sDrawXOffset(int scaleVal) const {
	if (scaleVal == SCALE_THRESHOLD)
		return _offset.x;

	int width = _offset.x;
	int scale = scaleVal == 0 ? 1 : scaleVal;

	if (scaleVal >= SCALE_THRESHOLD)
		--width;

	int result = width * SCALE_THRESHOLD / scale;
	if (scaleVal > SCALE_THRESHOLD)
		++result;

	return result;
}

int ImageFrame::sDrawYOffset(int scaleVal) const {
	if (scaleVal == SCALE_THRESHOLD)
		return _offset.y;

	int height = _offset.y;
	int scale = scaleVal == 0 ? 1 : scaleVal;

	if (scaleVal >= SCALE_THRESHOLD)
		--height;

	int result = height * SCALE_THRESHOLD / scale;
	if (scaleVal > SCALE_THRESHOLD)
		++result;

	return result;
}

// *******************************************************

/*----------------------------------------------------------------*/

SherlockEngine *ImageFile3DO::_vm;

void ImageFile3DO::setVm(SherlockEngine *vm) {
	_vm = vm;
}

ImageFile3DO::ImageFile3DO(const Common::String &name, ImageFile3DOType imageFile3DOType) {
#if 0
	Common::File *dataStream = new Common::File();

	if (!dataStream->open(name)) {
		error("unable to open %s\n", name.c_str());
	}
#endif
	Common::SeekableReadStream *dataStream = _vm->_res->load(name);

	switch(imageFile3DOType) {
	case kImageFile3DOType_Animation:
		loadAnimationFile(*dataStream);
		break;
	case kImageFile3DOType_Cel:
	case kImageFile3DOType_CelAnimation:
		load3DOCelFile(*dataStream);
		break;
	case kImageFile3DOType_RoomFormat:
		load3DOCelRoomData(*dataStream);
		break;
	case kImageFile3DOType_Font:
		loadFont(*dataStream);
		break;
	default:
		error("unknown Imagefile-3DO-Type");
		break;
	}

	delete dataStream;
}

ImageFile3DO::ImageFile3DO(Common::SeekableReadStream &stream, bool isRoomData) {
	if (!isRoomData) {
		load(stream, isRoomData);
	} else {
		load3DOCelRoomData(stream);
	}
}

void ImageFile3DO::load(Common::SeekableReadStream &stream, bool isRoomData) {
	uint32 headerId = 0;

	if (isRoomData) {
		load3DOCelRoomData(stream);
		return;
	}

	headerId = stream.readUint32BE();
	assert(!stream.eos());

	// Seek back to the start
	stream.seek(-4, SEEK_CUR);

	// Identify type of file
	switch (headerId) {
	case MKTAG('C', 'C', 'B', ' '):
	case MKTAG('A', 'N', 'I', 'M'):
	case MKTAG('O', 'F', 'S', 'T'): // 3DOSplash.cel
		// 3DO .cel (title1a.cel, etc.) or animation file (walk.anim)
		load3DOCelFile(stream);
		break;

	default:
		// Sherlock animation file (.3da files)
		loadAnimationFile(stream);
		break;
	}
}

// 3DO uses RGB555, we use RGB565 internally so that more platforms are able to run us
inline uint16 ImageFile3DO::convertPixel(uint16 pixel3DO) {
	byte red   = (pixel3DO >> 10) & 0x1F;
	byte green = (pixel3DO >> 5) & 0x1F;
	byte blue  = pixel3DO & 0x1F;

	return ((red << 11) | (green << 6) | (blue));
}

void ImageFile3DO::loadAnimationFile(Common::SeekableReadStream &stream) {
	uint32 streamLeft = stream.size() - stream.pos();
	uint32 celDataSize = 0;

	while (streamLeft > 0) {
		ImageFrame frame;

		// We expect a basic header of 8 bytes
		if (streamLeft < 8)
			error("load3DOAnimationFile: expected animation header, not enough bytes");

		celDataSize = stream.readUint16BE();

		frame._width = stream.readUint16BE() + 1; // 2 bytes BE width
		frame._height = stream.readByte() + 1; // 1 byte BE height
		frame._paletteBase = 0;

		frame._rleEncoded = true; // always compressed
		if (frame._width & 0x8000) {
			frame._width &= 0x7FFF;
			celDataSize += 0x10000;
		}

		frame._offset.x = stream.readUint16BE();
		frame._offset.y = stream.readByte();
		frame._size = 0;
		// Got header
		streamLeft -= 8;

		// cel data follows
		if (streamLeft < celDataSize)
			error("load3DOAnimationFile: expected cel data, not enough bytes");

		//
		// Load data for frame and decompress it
		byte *data_ = new byte[celDataSize];
		stream.read(data_, celDataSize);
		streamLeft -= celDataSize;

		// always 16 bits per pixel (RGB555)
		decompress3DOCelFrame(frame, data_, celDataSize, 16, NULL);

		delete[] data_;

		push_back(frame);
	}
}

static byte imagefile3DO_cel_bitsPerPixelLookupTable[8] = {
	0, 1, 2, 4, 6, 8, 16, 0
};

// Reads a 3DO .cel/.anim file
void ImageFile3DO::load3DOCelFile(Common::SeekableReadStream &stream) {
	int32  streamSize = stream.size();
	int32  chunkStartPos = 0;
	uint32 chunkTag = 0;
	uint32 chunkSize = 0;
	byte  *chunkDataPtr = NULL;

	// ANIM chunk (animation header for animation files)
	bool   animFound = false;
	uint32 animVersion = 0;
	uint32 animType = 0;
	uint32 animFrameCount = 1; // we expect 1 frame without an ANIM header
	// CCB chunk (cel control block)
	bool   ccbFound = false;
	uint32 ccbVersion = 0;
	uint32 ccbFlags = 0;
	bool   ccbFlags_compressed = false;
	uint16 ccbPPMP0 = 0;
	uint16 ccbPPMP1 = 0;
	uint32 ccbPRE0 = 0;
	uint16 ccbPRE0_height = 0;
	byte   ccbPRE0_bitsPerPixel = 0;
	uint32 ccbPRE1 = 0;
	uint16 ccbPRE1_width = 0;
	uint32 ccbWidth = 0;
	uint32 ccbHeight = 0;
	// pixel lookup table
	bool   plutFound = false;
	uint32 plutCount = 0;
	ImageFile3DOPixelLookupTable plutRGBlookupTable;

	memset(&plutRGBlookupTable, 0, sizeof(plutRGBlookupTable));

	while (!stream.err() && (stream.pos() < streamSize)) {
		chunkStartPos = stream.pos();
		chunkTag = stream.readUint32BE();
		chunkSize = stream.readUint32BE();

		if (stream.eos() || stream.err())
			break;

		if (chunkSize < 8)
			error("load3DOCelFile: Invalid chunk size");

		uint32 dataSize = chunkSize - 8;

		switch (chunkTag) {
		case MKTAG('A', 'N', 'I', 'M'):
			// animation header
			assert(dataSize >= 24);

			if (animFound)
				error("load3DOCelFile: multiple ANIM chunks not supported");

			animFound   = true;
			animVersion = stream.readUint32BE();
			animType = stream.readUint32BE();
			animFrameCount = stream.readUint32BE();
			// UINT32 - framerate (0x2000 in walk.anim???)
			// UINT32 - starting frame (0 for walk.anim)
			// UINT32 - number of loops (0 for walk.anim)

			if (animVersion != 0)
				error("load3DOCelFile: Unsupported animation file version");
			if (animType != 1)
				error("load3DOCelFile: Only single CCB animation files are supported");
			break;

		case MKTAG('C', 'C', 'B', ' '):
			// CEL control block
			assert(dataSize >= 72);

			if (ccbFound)
				error("load3DOCelFile: multiple CCB chunks not supported");

			ccbFound   = true;
			ccbVersion = stream.readUint32BE();
			ccbFlags   = stream.readUint32BE();
			stream.skip(3 * 4); // skip over 3 pointer fields, which are used in memory only by 3DO hardware
			stream.skip(8 * 4); // skip over 8 offset fields
			ccbPPMP0   = stream.readUint16BE();
			ccbPPMP1   = stream.readUint16BE();
			ccbPRE0    = stream.readUint32BE();
			ccbPRE1    = stream.readUint32BE();
			ccbWidth   = stream.readUint32BE();
			ccbHeight  = stream.readUint32BE();

			if (ccbVersion != 0)
				error("load3DOCelFile: Unsupported CCB version");

			if (ccbFlags & 0x200) // bit 9
				ccbFlags_compressed = true;

			// bit 5 of ccbFlags defines how RGB-black (0, 0, 0) will get treated
			// = false -> RGB-black is treated as transparent
			// = true  -> RGB-black is treated as actual black
			// atm we are always treating it as transparent
			// it seems this bit is not set for any data of Sherlock Holmes

			// PRE0 first 3 bits define how many bits per encoded pixel are used
			ccbPRE0_bitsPerPixel = imagefile3DO_cel_bitsPerPixelLookupTable[ccbPRE0 & 0x07];
			if (!ccbPRE0_bitsPerPixel)
				error("load3DOCelFile: Invalid CCB PRE0 bits per pixel");

			ccbPRE0_height = ((ccbPRE0 >> 6) & 0x03FF) + 1;
			ccbPRE1_width  = (ccbPRE1 & 0x03FF) + 1;
			assert(ccbPRE0_height == ccbHeight);
			assert(ccbPRE1_width == ccbWidth);
			break;

		case MKTAG('P', 'L', 'U', 'T'):
			// pixel lookup table
			// optional, not required for at least 16-bit pixel data
			assert(dataSize >= 6);

			if (!ccbFound)
				error("load3DOCelFile: PLUT chunk found without CCB chunk");
			if (plutFound)
				error("load3DOCelFile: multiple PLUT chunks currently not supported");

			plutFound = true;
			plutCount = stream.readUint32BE();
			// table follows, each entry is 16bit RGB555
			assert(dataSize >= 4 + (plutCount * 2)); // security check
			assert(plutCount <= 256); // security check

			assert(plutCount <= 32); // PLUT should never contain more than 32 entries

			for (uint32 plutColorNr = 0; plutColorNr < plutCount; plutColorNr++) {
				plutRGBlookupTable.pixelColor[plutColorNr] = stream.readUint16BE();
			}

			if (ccbPRE0_bitsPerPixel == 8) {
				// In case we are getting 8-bits per pixel, we calculate the shades accordingly
				// I'm not 100% sure if the calculation is correct. It's difficult to find information
				// on this topic.
				// The map uses this type of cel
				assert(plutCount == 32); // And we expect 32 entries inside PLUT chunk

				uint16 plutColorRGB = 0;
				for (uint32 plutColorNr = 0; plutColorNr < plutCount; plutColorNr++) {
					plutColorRGB = plutRGBlookupTable.pixelColor[plutColorNr];

					// Extract RGB values
					byte plutColorRed   = (plutColorRGB >> 10) & 0x1F;
					byte plutColorGreen = (plutColorRGB >> 5) & 0x1F;
					byte plutColorBlue  = plutColorRGB & 0x1F;

					byte shadeMultiplier = 2;
					for (uint32 plutShadeNr = 1; plutShadeNr < 8; plutShadeNr++) {
						uint16 shadedColorRGB;
						byte   shadedColorRed   = (plutColorRed * shadeMultiplier) >> 3;
						byte   shadedColorGreen = (plutColorGreen * shadeMultiplier) >> 3;
						byte   shadedColorBlue  = (plutColorBlue * shadeMultiplier) >> 3;

						shadedColorRed = CLIP<byte>(shadedColorRed, 0, 0x1F);
						shadedColorGreen = CLIP<byte>(shadedColorGreen, 0, 0x1F);
						shadedColorBlue = CLIP<byte>(shadedColorBlue, 0, 0x1F);
						shadedColorRGB = (shadedColorRed << 10) | (shadedColorGreen << 5) | shadedColorBlue;

						plutRGBlookupTable.pixelColor[plutColorNr + (plutShadeNr << 5)] = shadedColorRGB;
						shadeMultiplier++;
					}
				}
			}
			break;

		case MKTAG('X', 'T', 'R', 'A'):
			// Unknown contents, occurs right before PDAT
			break;

		case MKTAG('P', 'D', 'A', 'T'): {
			// pixel data for one frame
			// may be compressed or uncompressed pixels

			if (ccbPRE0_bitsPerPixel != 16) {
				// We require a pixel lookup table in case bits-per-pixel is lower than 16
				if (!plutFound)
					error("load3DOCelFile: bits per pixel < 16, but no pixel lookup table was found");
			} else {
				// But we don't like it in case bits-per-pixel is 16 and we find one
				if (plutFound)
					error("load3DOCelFile: bits per pixel == 16, but pixel lookup table was found as well");
			}
			// read data into memory
			chunkDataPtr = new byte[dataSize];

			stream.read(chunkDataPtr, dataSize);

			// Set up frame
			ImageFrame imageFrame;

			imageFrame._width = ccbWidth;
			imageFrame._height = ccbHeight;
			imageFrame._paletteBase = 0;
			imageFrame._offset.x = 0;
			imageFrame._offset.y = 0;
			imageFrame._rleEncoded = ccbFlags_compressed;
			imageFrame._size = 0;

			// Decompress/copy this frame
			if (!plutFound) {
				decompress3DOCelFrame(imageFrame, chunkDataPtr, dataSize, ccbPRE0_bitsPerPixel, NULL);
			} else {
				decompress3DOCelFrame(imageFrame, chunkDataPtr, dataSize, ccbPRE0_bitsPerPixel, &plutRGBlookupTable);
			}

			delete[] chunkDataPtr;

			push_back(imageFrame);
			break;
		}

		case MKTAG('O', 'F', 'S', 'T'): // 3DOSplash.cel
			// unknown contents
			break;

		default:
			error("Unsupported '%s' chunk in 3DO cel file", tag2str(chunkTag));
		}

		// Seek to end of chunk
		stream.seek(chunkStartPos + chunkSize);
	}

	// Warning below being used to silence unused variable warnings for now
	warning("TODO: Remove %d %d %d", animFrameCount, ccbPPMP0, ccbPPMP1);
}

// Reads 3DO .cel data (room file format)
void ImageFile3DO::load3DOCelRoomData(Common::SeekableReadStream &stream) {
	uint32 streamLeft = stream.size() - stream.pos();
	uint16 roomDataHeader_size = 0;
	byte   roomDataHeader_offsetX = 0;
	byte   roomDataHeader_offsetY = 0;

	// CCB chunk (cel control block)
	uint32 ccbFlags = 0;
	bool   ccbFlags_compressed = false;
	uint16 ccbPPMP0 = 0;
	uint16 ccbPPMP1 = 0;
	uint32 ccbPRE0 = 0;
	uint16 ccbPRE0_height = 0;
	byte   ccbPRE0_bitsPerPixel = 0;
	uint32 ccbPRE1 = 0;
	uint16 ccbPRE1_width = 0;
	uint32 ccbWidth = 0;
	uint32 ccbHeight = 0;
	// cel data
	uint32 celDataSize = 0;

	while (streamLeft > 0) {
		// We expect at least 8 bytes basic header
		if (streamLeft < 8)
			error("load3DOCelRoomData: expected room data header, not enough bytes");

		// 3DO sherlock holmes room data header
		stream.skip(4); // Possibly UINT16 width, UINT16 height?!?!
		roomDataHeader_size = stream.readUint16BE();
		roomDataHeader_offsetX = stream.readByte();
		roomDataHeader_offsetY = stream.readByte();
		streamLeft -= 8;

		// We expect the header size specified in the basic header to be at least a raw CCB
		if (roomDataHeader_size < 68)
			error("load3DOCelRoomData: header size is too small");
		// Check, that enough bytes for CCB are available
		if (streamLeft < 68)
			error("load3DOCelRoomData: expected raw cel control block, not enough bytes");

		// 3DO raw cel control block
		ccbFlags   = stream.readUint32BE();
		stream.skip(3 * 4); // skip over 3 pointer fields, which are used in memory only by 3DO hardware
		stream.skip(8 * 4); // skip over 8 offset fields
		ccbPPMP0   = stream.readUint16BE();
		ccbPPMP1   = stream.readUint16BE();
		ccbPRE0    = stream.readUint32BE();
		ccbPRE1    = stream.readUint32BE();
		ccbWidth   = stream.readUint32BE();
		ccbHeight  = stream.readUint32BE();

		if (ccbFlags & 0x200) // bit 9
			ccbFlags_compressed = true;

		// PRE0 first 3 bits define how many bits per encoded pixel are used
		ccbPRE0_bitsPerPixel = imagefile3DO_cel_bitsPerPixelLookupTable[ccbPRE0 & 0x07];
		if (!ccbPRE0_bitsPerPixel)
			error("load3DOCelRoomData: Invalid CCB PRE0 bits per pixel");

		ccbPRE0_height = ((ccbPRE0 >> 6) & 0x03FF) + 1;
		ccbPRE1_width  = (ccbPRE1 & 0x03FF) + 1;
		assert(ccbPRE0_height == ccbHeight);
		assert(ccbPRE1_width == ccbWidth);

		if (ccbPRE0_bitsPerPixel != 16) {
			// We currently support 16-bits per pixel in here
			error("load3DOCelRoomData: bits per pixel < 16?!?!?");
		}
		// Got the raw CCB
		streamLeft -= 68;

		// cel data follows
		// size field does not include the 8 byte header
		celDataSize = roomDataHeader_size - 68;

		if (streamLeft < celDataSize)
			error("load3DOCelRoomData: expected cel data, not enough bytes");

		// read data into memory
		byte *celDataPtr = new byte[celDataSize];

		stream.read(celDataPtr, celDataSize);
		streamLeft -= celDataSize;

		// Set up frame
		{
			ImageFrame imageFrame;

			imageFrame._width = ccbWidth;
			imageFrame._height = ccbHeight;
			imageFrame._paletteBase = 0;
			imageFrame._offset.x = roomDataHeader_offsetX;
			imageFrame._offset.y = roomDataHeader_offsetY;
			imageFrame._rleEncoded = ccbFlags_compressed;
			imageFrame._size = 0;

			// Decompress/copy this frame
			decompress3DOCelFrame(imageFrame, celDataPtr, celDataSize, ccbPRE0_bitsPerPixel, NULL);

			delete[] celDataPtr;

			push_back(imageFrame);
		}
	}

	// Suppress compiler warning
	warning("ccbPPMP0 = %d, ccbPPMP1 = %d", ccbPPMP0, ccbPPMP1);
}

static uint16 imagefile3DO_cel_bitsMask[17] = {
	0,
	0x0001, 0x0003, 0x0007, 0x000F, 0x001F, 0x003F, 0x007F, 0x00FF,
	0x01FF, 0x03FF, 0x07FF, 0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF
};

// gets [bitCount] bits from dataPtr, going from MSB to LSB
inline uint16 ImageFile3DO::celGetBits(const byte *&dataPtr, byte bitCount, byte &dataBitsLeft) {
	byte   resultBitsLeft = bitCount;
	uint16 result = 0;
	byte   currentByte = *dataPtr;

	// Get bits of current byte
	while (resultBitsLeft) {
		if (resultBitsLeft < dataBitsLeft) {
			// we need less than we have left
			result |= (currentByte >> (dataBitsLeft - resultBitsLeft)) & imagefile3DO_cel_bitsMask[resultBitsLeft];
			dataBitsLeft -= resultBitsLeft;
			resultBitsLeft = 0;

		} else {
			// we need as much as we have left or more
			resultBitsLeft -= dataBitsLeft;
			result |= (currentByte & imagefile3DO_cel_bitsMask[dataBitsLeft]) << resultBitsLeft;

			// Go to next byte
			dataPtr++;
			dataBitsLeft = 8;
			if (resultBitsLeft) {
				currentByte = *dataPtr;
			}
		}
	}
	return result;
}

// decompress/copy 3DO cel data
void ImageFile3DO::decompress3DOCelFrame(ImageFrame &frame, const byte *dataPtr, uint32 dataSize, byte bitsPerPixel, ImageFile3DOPixelLookupTable *pixelLookupTable) {
	frame._frame.create(frame._width, frame._height, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
	uint16 *dest = (uint16 *)frame._frame.getPixels();
	Common::fill(dest, dest + frame._width * frame._height, 0);

	int frameHeightLeft = frame._height;
	int frameWidthLeft = frame._width;
	uint16 pixelCount = 0;
	uint16 pixel = 0;

	const  byte *srcLineStart = dataPtr;
	const  byte *srcLineData = dataPtr;
	byte   srcLineDataBitsLeft = 0;
	uint16 lineDWordSize = 0;
	uint16 lineByteSize = 0;

	if (bitsPerPixel == 16) {
		// Must not use pixel lookup table on 16-bits-per-pixel data
		assert(!pixelLookupTable);
	}

	if (frame._rleEncoded) {
		// compressed
		byte   compressionType = 0;
		byte   compressionPixels = 0;

		while (frameHeightLeft > 0) {
			frameWidthLeft = frame._width;

			if (bitsPerPixel >= 8) {
				lineDWordSize = READ_BE_UINT16(srcLineStart);
				srcLineData = srcLineStart + 2;
			} else {
				lineDWordSize = *srcLineStart;
				srcLineData = srcLineStart + 1;
			}
			srcLineDataBitsLeft = 8;

			lineDWordSize += 2;
			lineByteSize = lineDWordSize * 4; // calculate compressed data size in bytes for current line

			// debug
			//warning("offset %d: decoding line, size %d, bytesize %d", srcSeeker - src, dwordSize, lineByteSize);

			while (frameWidthLeft > 0) {
				// get 2 bits -> compressionType
				// get 6 bits -> pixel count (0 = 1 pixel)
				compressionType = celGetBits(srcLineData, 2, srcLineDataBitsLeft);
				// 6 bits == length (0 = 1 pixel)
				compressionPixels = celGetBits(srcLineData, 6, srcLineDataBitsLeft) + 1;

				if (!compressionType) // end of line
					break;

				switch(compressionType) {
				case 1: // simple copy
					for (pixelCount = 0; pixelCount < compressionPixels; pixelCount++) {
						pixel = celGetBits(srcLineData, bitsPerPixel, srcLineDataBitsLeft);
						if (pixelLookupTable) {
							pixel = pixelLookupTable->pixelColor[pixel];
						}
						*dest++ = convertPixel(pixel);
					}
					break;
				case 2: // transparent
					for (pixelCount = 0; pixelCount < compressionPixels; pixelCount++) {
						*dest++ = 0;
					}
					break;
				case 3: // duplicate pixels
					pixel = celGetBits(srcLineData, bitsPerPixel, srcLineDataBitsLeft);
					if (pixelLookupTable) {
						pixel = pixelLookupTable->pixelColor[pixel];
					}
					pixel = convertPixel(pixel);
					for (pixelCount = 0; pixelCount < compressionPixels; pixelCount++) {
						*dest++ = pixel;
					}
					break;
				default:
					break;
				}
				frameWidthLeft -= compressionPixels;
			}

			assert(frameWidthLeft >= 0);

			if (frameWidthLeft > 0) {
				// still pixels left? skip them
				dest += frameWidthLeft;
			}

			frameHeightLeft--;

			// Seek to next line start
			srcLineStart += lineByteSize;
		}
	} else {
		// uncompressed
		srcLineDataBitsLeft = 8;
		lineDWordSize = ((frame._width * bitsPerPixel) + 31) >> 5;
		lineByteSize = lineDWordSize * 4;
		uint32 totalExpectedSize = lineByteSize * frame._height;

		assert(totalExpectedSize <= dataSize); // security check

		while (frameHeightLeft > 0) {
			srcLineData = srcLineStart;
			frameWidthLeft = frame._width;

			while (frameWidthLeft > 0) {
				pixel = celGetBits(srcLineData, bitsPerPixel, srcLineDataBitsLeft);
				if (pixelLookupTable) {
					pixel = pixelLookupTable->pixelColor[pixel];
				}
				*dest++ = convertPixel(pixel);

				frameWidthLeft--;
			}
			frameHeightLeft--;

			// Seek to next line start
			srcLineStart += lineByteSize;
		}
	}
}

// Reads Sherlock Holmes 3DO font file
void ImageFile3DO::loadFont(Common::SeekableReadStream &stream) {
	uint32 streamSize = stream.size();
	uint32 header_offsetWidthTable = 0;
	uint32 header_offsetBitsTable = 0;
	uint32 header_fontHeight = 0;
	uint32 header_bytesPerLine = 0;
	uint32 header_maxChar = 0;
	uint32 header_charCount = 0;

	byte  *widthTablePtr = NULL;
	uint32 bitsTableSize = 0;
	byte  *bitsTablePtr = NULL;

	stream.skip(2); // Unknown bytes
	stream.skip(2); // Unknown bytes (0x000E)
	header_offsetWidthTable = stream.readUint32BE();
	header_offsetBitsTable = stream.readUint32BE();
	stream.skip(4); // Unknown bytes (0x00000004)
	header_fontHeight = stream.readUint32BE();
	header_bytesPerLine = stream.readUint32BE();
	header_maxChar = stream.readUint32BE();

	assert(header_maxChar <= 255);
	header_charCount = header_maxChar + 1;

	// Allocate memory for width table
	widthTablePtr = new byte[header_charCount];

	stream.seek(header_offsetWidthTable);
	stream.read(widthTablePtr, header_charCount);

	// Allocate memory for the bits
	assert(header_offsetBitsTable < streamSize); // Security check
	bitsTableSize = streamSize - header_offsetBitsTable;
	bitsTablePtr  = new byte[bitsTableSize];
	stream.read(bitsTablePtr, bitsTableSize);

	// Now extract all characters
	uint16      curChar = 0;
	const byte *curBitsLinePtr = bitsTablePtr;
	const byte *curBitsPtr = NULL;
	byte        curBitsLeft = 0;
	uint32      curCharHeightLeft = 0;
	uint32      curCharWidthLeft = 0;
	byte        curBits = 0;
	byte        curBitsReversed = 0;
	byte        curPosX = 0;

	assert(bitsTableSize >= (header_maxChar * header_fontHeight * header_bytesPerLine)); // Security

	// first frame needs to be "!" (33 decimal)
	// our font code is subtracting 33 from the actual character code
	curBitsLinePtr += (33 * (header_fontHeight * header_bytesPerLine));

	for (curChar = 33; curChar < header_charCount; curChar++) {
		// create frame
		ImageFrame imageFrame;

		imageFrame._width = widthTablePtr[curChar];
		imageFrame._height = header_fontHeight;
		imageFrame._paletteBase = 0;
		imageFrame._offset.x = 0;
		imageFrame._offset.y = 0;
		imageFrame._rleEncoded = false;
		imageFrame._size = 0;

		// Extract pixels
		imageFrame._frame.create(imageFrame._width, imageFrame._height, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
		uint16 *dest = (uint16 *)imageFrame._frame.getPixels();
		Common::fill(dest, dest + imageFrame._width * imageFrame._height, 0);

		curCharHeightLeft = header_fontHeight;
		while (curCharHeightLeft) {
			curCharWidthLeft = widthTablePtr[curChar];
			curBitsPtr  = curBitsLinePtr;
			curBitsLeft = 8;
			curPosX     = 0;

			while (curCharWidthLeft) {
				if (!(curPosX & 1)) {
					curBits = *curBitsPtr >> 4;
				} else {
					curBits = *curBitsPtr & 0x0F;
					curBitsPtr++;
				}
				// doing this properly is complicated
				// the 3DO has built-in anti-aliasing
				// this here at least results in somewhat readable text
				// TODO: make it better
				if (curBits) {
					curBitsReversed = (curBits >> 3) | ((curBits & 0x04) >> 1) | ((curBits & 0x02) << 1) | ((curBits & 0x01) << 3);
					curBits = 20 - curBits;
				}

				byte curIntensity = curBits;
				*dest = (curIntensity << 11) | (curIntensity << 6) | curIntensity;
				dest++;

				curCharWidthLeft--;
				curPosX++;
			}

			curCharHeightLeft--;
			curBitsLinePtr += header_bytesPerLine;
		}

		push_back(imageFrame);
	}

	// Warning below being used to silence unused variable warnings for now
	warning("TODO: Remove %d %d", curBitsLeft, curBitsReversed);

	delete[] bitsTablePtr;
	delete[] widthTablePtr;
}

/*----------------------------------------------------------------*/

StreamingImageFile::StreamingImageFile() {
	_frameNumber = 0;
	_stream = nullptr;
	_flags = 0;
	_scaleVal = 0;
	_zPlacement = 0;
	_compressed = false;
	_active = false;
}

StreamingImageFile::~StreamingImageFile() {
	close();
}

void StreamingImageFile::load(Common::SeekableReadStream *stream, bool compressed) {
	_stream = stream;
	_compressed = compressed;
	_frameNumber = -1;
	_active = true;
}

void StreamingImageFile::close() {
	delete _stream;
	_stream = nullptr;
	_frameNumber = -1;
	_active = false;
	_imageFrame._frame.free();
}

bool StreamingImageFile::getNextFrame() {
	// Don't proceed if we're already at the end of the stream
	assert(_stream);
	if (_stream->pos() >= _stream->size()) {
		_active = false;
		return false;
	}

	// Increment frame number
	++_frameNumber;

	// If necessary, decompress the next frame
	Common::SeekableReadStream *frameStream = _stream;
	if (_compressed) {
		uint32 inSize = _stream->readUint32LE();
		Resources::decompressLZ(*_stream, _buffer, STREAMING_BUFFER_SIZE, inSize);
		frameStream = new Common::MemoryReadStream(_buffer, 11, DisposeAfterUse::NO);
	}

	// Load the data for the frame
	_imageFrame._width = frameStream->readUint16LE() + 1;
	_imageFrame._height = frameStream->readUint16LE() + 1;
	_imageFrame._paletteBase = frameStream->readByte();
	_imageFrame._rleEncoded = frameStream->readByte() == 1;
	_imageFrame._offset.x = frameStream->readByte();
	_imageFrame._offset.y = frameStream->readByte();
	_imageFrame._size = frameStream->readUint16LE() - 11;
	_imageFrame._rleMarker = frameStream->readByte();

	// Free the previous frame
	_imageFrame._frame.free();

	// Decode the frame
	if (_compressed) {
		delete frameStream;
		_imageFrame.decompressFrame(_buffer + 11, true);
	} else {
		byte *data = new byte[_imageFrame._size];
		_stream->read(data, _imageFrame._size);
		_imageFrame.decompressFrame(_buffer + 11, true);
		delete[] data;
	}

	return true;
}

} // End of namespace Sherlock
