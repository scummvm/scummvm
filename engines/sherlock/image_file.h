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

#ifndef SHERLOCK_IMAGE_FILE_H
#define SHERLOCK_IMAGE_FILE_H

#include "common/array.h"
#include "common/file.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/stream.h"
#include "graphics/surface.h"

namespace Sherlock {

class SherlockEngine;

struct ImageFrame {
	uint32 _size;
	uint16 _width, _height;
	int _paletteBase;
	bool _rleEncoded;
	Common::Point _offset;
	byte _rleMarker;
	Graphics::Surface _frame;

	/**
	 * Converts an ImageFrame record to a surface for convenience in passing to drawing methods
	 */
	operator const Graphics::Surface &() { return _frame; }

	/**
	 * Decompress a single frame for the sprite
	 */
	void decompressFrame(const byte *src, bool isRoseTattoo);

	/**
	 * Return the frame width adjusted by a specified scale amount
	 */
	int sDrawXSize(int scaleVal) const;

	/**
	 * Return the frame height adjusted by a specified scale amount
	 */
	int sDrawYSize(int scaleVal) const;

	/**
	 * Return the frame offset x adjusted by a specified scale amount
	 */
	int sDrawXOffset(int scaleVal) const;

	/**
	 * Return the frame offset y adjusted by a specified scale amount
	 */
	int sDrawYOffset(int scaleVal) const;
};

class ImageFile : public Common::Array<ImageFrame> {
private:
	static SherlockEngine *_vm;

	/**
	 * Load the data of the sprite
	 */
	void load(Common::SeekableReadStream &stream, bool skipPalette, bool animImages);

	/**
	 * Gets the palette at the start of the sprite file
	 */
	void loadPalette(Common::SeekableReadStream &stream);
public:
	byte _palette[256 * 3];
public:
	ImageFile();
	ImageFile(const Common::String &name, bool skipPal = false, bool animImages = false);
	ImageFile(Common::SeekableReadStream &stream, bool skipPal = false);
	virtual ~ImageFile();
	static void setVm(SherlockEngine *vm);
};

enum ImageFile3DOType {
	kImageFile3DOType_Animation    = 0,
	kImageFile3DOType_Cel          = 1,
	kImageFile3DOType_CelAnimation = 2,
	kImageFile3DOType_RoomFormat   = 3,
	kImageFile3DOType_Font         = 4
};

struct ImageFile3DOPixelLookupTable {
	uint16 pixelColor[256];
};

class ImageFile3DO : public ImageFile { // Common::Array<ImageFrame> {
private:
	static SherlockEngine *_vm;

	/**
	 * Load the data of the sprite
	 */
	void load(Common::SeekableReadStream &stream, bool isRoomData);

	/**
	 * convert pixel RGB values from RGB555 to RGB565
	 */
	inline uint16 convertPixel(uint16 pixel3DO);

	/**
	 * Load 3DO cel file
	 */
	void load3DOCelFile(Common::SeekableReadStream &stream);

	/**
	 * Load 3DO cel data (room file format)
	 */
	void load3DOCelRoomData(Common::SeekableReadStream &stream);

	inline uint16 celGetBits(const byte *&dataPtr, byte bitCount, byte &dataBitsLeft);

	/**
	 * Decompress a single frame of a 3DO cel file
	 */
	void decompress3DOCelFrame(ImageFrame &frame, const byte *dataPtr, uint32 dataSize, byte bitsPerPixel, ImageFile3DOPixelLookupTable *pixelLookupTable);

	/**
	 * Load animation graphics file
	 */
	void loadAnimationFile(Common::SeekableReadStream &stream);

	/**
	 * Load Sherlock Holmes 3DO font file
	 */
	void loadFont(Common::SeekableReadStream &stream);

public:
	ImageFile3DO(const Common::String &name, ImageFile3DOType imageFile3DOType);
	ImageFile3DO(Common::SeekableReadStream &stream, bool isRoomData = false);
	static void setVm(SherlockEngine *vm);
};

#define STREAMING_BUFFER_SIZE 65536

class StreamingImageFile {
private:
	int _frameNumber;
	Common::SeekableReadStream *_stream;
	byte _buffer[STREAMING_BUFFER_SIZE];
	bool _compressed;
	bool _active;
public:
	ImageFrame _imageFrame;

	Common::Point _position;		// Animation position
	Common::Rect _oldBounds;		// Bounds of previous frame
	Common::Rect _removeBounds;		// Remove area for just drawn frame

	int _flags;						// Flags
	int _scaleVal;					// Specifies the scale amount
	int _zPlacement;				// Used by doBgAnim for determining Z order
public:
	StreamingImageFile();
	~StreamingImageFile();

	/**
	 * Initialize reading of the specified stream
	 */
	void load(Common::SeekableReadStream *stream, bool compressed);

	/**
	 * Close the streamining image file
	 */
	void close();

	/**
	 * Get the next frame of the file
	 */
	bool getNextFrame();

	/**
	 * Returns whether there are any remaining frames or not
	 */
	bool active() const { return _active; }

	/**
	 * Return the current frame number
	 */
	int frameNumber() const { return _frameNumber; }
};

} // End of namespace Sherlock

#endif
