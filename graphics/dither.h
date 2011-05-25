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
 */

#ifndef GRAPHICS_DITHER_H
#define GRAPHICS_DITHER_H

#include "common/util.h"

namespace Common {
class SeekableReadStream;
class WriteStream;
}

namespace Graphics {

/** A palette lookup table to find the nearest matching entry of a fixed palette to a true color.
 *
 *  The table can be build up in slices, one slice consisting of all entries for
 *  one value of the first color component.
 */
class PaletteLUT {
public:
	/** Palette format. */
	enum PaletteFormat {
		kPaletteRGB, ///< Palette in RGB colorspace
		kPaletteYUV  ///< Palette in YUV colorspace
	};

	/** Converting a color from YUV to RGB colorspace. */
	inline static void YUV2RGB(byte y, byte u, byte v, byte &r, byte &g, byte &b) {
		r = CLIP<int>(y + ((1357 * (v - 128)) >> 10), 0, 255);
		g = CLIP<int>(y - (( 691 * (v - 128)) >> 10) - ((333 * (u - 128)) >> 10), 0, 255);
		b = CLIP<int>(y + ((1715 * (u - 128)) >> 10), 0, 255);
	}
	/** Converting a color from RGB to YUV colorspace. */
	inline static void RGB2YUV(byte r, byte g, byte b, byte &y, byte &u, byte &v) {
		y = CLIP<int>( ((r * 306) >> 10) + ((g * 601) >> 10) + ((b * 117) >> 10)      , 0, 255);
		u = CLIP<int>(-((r * 172) >> 10) - ((g * 340) >> 10) + ((b * 512) >> 10) + 128, 0, 255);
		v = CLIP<int>( ((r * 512) >> 10) - ((g * 429) >> 10) - ((b *  83) >> 10) + 128, 0, 255);
	}

	/** Create a lookup table of a given depth and palette format.
	 *
	 *  @param depth How many bits of each color component to consider.
	 *  @param format The format the palette should be in.
	 */
	PaletteLUT(byte depth, PaletteFormat format);
	~PaletteLUT();

	/** Setting a palette.
	 *
	 *  Any already built slices will be purged.
	 *
	 *  @param palette The palette, plain 256 * 3 color components.
	 *  @param format The format the palette is in.
	 *  @param depth The number of significant bits in each color component.
	 *  @param transp An index that's seen as transparent and therefore ignored.
	 */
	void setPalette(const byte *palette, PaletteFormat format, byte depth, int transp = -1);

	/** Build the next slice.
	 *
	 *  This will build the next slice, if any.
	 */
	void buildNext();

	/** Querying the color components to a given palette entry index. */
	void getEntry(byte index, byte &c1, byte &c2, byte &c3) const;
	/** Finding the nearest matching entry.
	 *
	 *  @param c1 The first component of the wanted color.
	 *  @param c2 The second component of the wanted color.
	 *  @param c3 The third component of the wanted color.
	 *  @return The palette entry matching the wanted color best.
	 */
	byte findNearest(byte c1, byte c2, byte c3);
	/** Finding the nearest matching entry, together with its color components.
	 *
	 *  @param c1 The first component of the wanted color.
	 *  @param c2 The second component of the wanted color.
	 *  @param c3 The third component of the wanted color.
	 *  @paran nC1 The first component of the found color.
	 *  @paran nC2 The second component of the found color.
	 *  @paran nC3 The third component of the found color.
	 *  @return The palette entry matching the wanted color best.
	 */
	byte findNearest(byte c1, byte c2, byte c3, byte &nC1, byte &nC2, byte &nC3);

	/** Save the table to a stream.
	 *
	 *  This will build the whole table first.
	 */
	bool save(Common::WriteStream &stream);
	/** Load the table from a stream. */
	bool load(Common::SeekableReadStream &stream);

private:
	static const uint32 kVersion = 1;

	byte _depth1; ///< The table's depth for one dimension.
	byte _depth2; ///< The table's depth for two dimensions.
	byte _shift;  ///< Amount to shift to adjust for the table's depth.

	uint32 _dim1; ///< The table's entry offset for one dimension.
	uint32 _dim2; ///< The table's entry offset for two dimensions.
	uint32 _dim3; ///< The table's entry offset for three dimensions.

	int _transp;  ///< The transparent palette index.

	PaletteFormat _format; ///< The table's palette format.
	byte _lutPal[768];     ///< The palette used for looking up a color.
	byte _realPal[768];    ///< The original palette.

	uint32 _got; ///< Number of slices generated.
	byte *_gots; ///< Map of generated slices.
	byte *_lut;  ///< The lookup table.

	/** Building a specified slice. */
	void build(int d1);
	/** Calculates the index into the lookup table for a given color. */
	inline int getIndex(byte c1, byte c2, byte c3) const;
};

/** The Sierra-2-4A ("Filter Light") error distribution dithering algorithm.
 *
 *  The image will be dithered line by line and pixel by pixel, without earlier
 *  values having to be changed.
*/
class SierraLight {
public:
	/** Constructor.
	 *
	 *  @param width The width of the image to dither.
	 *  @param palLUT The palette to which to dither.
	 */
	SierraLight(int16 width, PaletteLUT *palLUT);
	~SierraLight();

	/** Signals that a new frame or image is about to be dithered.
	 *
	 *  This clears all collected errors, so that a new image (of the same
	 *  height and with the same palette) can be dithered.
	 */
	void newFrame();
	/** Signals that a new line is about the begin.
	 *
	 *  The current line's errors will be forgotten and values collected for the
	 *  next line will now count as the current line's.
	 */
	void nextLine();
	/** Dither a pixel.
	 *
	 *  @param c1 The first color component of the pixel.
	 *  @param c2 The second color component of the pixel.
	 *  @param c3 The third color component of the pixel.
	 *  @param x The pixel's x coordinate within the image.
	 */
	byte dither(byte c1, byte c2, byte c3, uint32 x);

protected:
	int16 _width; ///< The image's width.

	PaletteLUT *_palLUT; ///< The palette against which to dither.

	int32 *_errorBuf;  ///< Big buffer for all collected errors.
	int32 *_errors[2]; ///< Pointers into the error buffer for two lines.
	int _curLine;      ///< Which one is the current line?

	/** Querying a pixel's errors. */
	inline void getErrors(uint32 x, int32 &eC1, int32 &eC2, int32 &eC3);
	/** Adding a pixel's errors. */
	inline void addErrors(uint32 x, int32 eC1, int32 eC2, int32 eC3);
};

} // End of namespace Graphics

#endif
