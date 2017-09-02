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

/* Intel Indeo 4 decompressor, derived from ffmpeg.
 *
 * Original copyright note:
 * Intel Indeo 4 (IV31, IV32, etc.) video decoder for ffmpeg
 * written, produced, and directed by Alan Smithee
 */

#ifndef IMAGE_CODECS_INDEO4_H
#define IMAGE_CODECS_INDEO4_H

#include "image/codecs/indeo/get_bits.h"
#include "image/codecs/indeo/indeo.h"

namespace Image {

using namespace Indeo;

/**
 * Intel Indeo 4 decoder.
 *
 * Used by AVI.
 *
 * Used in video:
 *  - AVIDecoder
 */
class Indeo4Decoder : public IndeoDecoderBase {
	struct Transform {
		InvTransformPtr *_invTrans;
		DCTransformPtr *_dcTrans;
		bool _is2dTrans;
	};
public:
	Indeo4Decoder(uint16 width, uint16 height, uint bitsPerPixel = 16);
	virtual ~Indeo4Decoder() {}

	virtual const Graphics::Surface *decodeFrame(Common::SeekableReadStream &stream);

	static bool isIndeo4(Common::SeekableReadStream &stream);
protected:
	/**
	 * Decode the Indeo 4 picture header.
	 * @returns		0 = Ok, negative number = error
	 */
	virtual int decodePictureHeader();

	/**
	 *  Rearrange decoding and reference buffers.
	 */
	virtual void switchBuffers();

	virtual bool isNonNullFrame() const;

	/**
	 *  Decode Indeo 4 band header.
	 *
	 *  @param[in,out] band      pointer to the band descriptor
	 *  @returns       result code: 0 = OK, negative number = error
	 */
	virtual int decodeBandHeader(IVIBandDesc *band);

	/**
	 *  Decode information (block type, cbp, quant delta, motion vector)
	 *  for all macroblocks in the current tile.
	 *
	 *  @param[in,out] band      pointer to the band descriptor
	 *  @param[in,out] tile      pointer to the tile descriptor
	 *  @returns       result code: 0 = OK, negative number = error
	 */
	virtual int decodeMbInfo(IVIBandDesc *band, IVITile *tile);

	/**
	 * Decodes huffman + RLE-coded transparency data within Indeo4 frames
	 */
	int decodeRLETransparency(VLC_TYPE (*table)[2]);

	/**
	 * Decodes optional transparency data within Indeo4 frames
	 */
	virtual int decodeTransparency();
private:
	int scaleTileSize(int defSize, int sizeFactor);

	/**
	 *  Decode subdivision of a plane.
	 *  This is a simplified version that checks for two supported subdivisions:
	 *  - 1 wavelet band  per plane, size factor 1:1, code pattern: 3
	 *  - 4 wavelet bands per plane, size factor 1:4, code pattern: 2,3,3,3,3
	 *  Anything else is either unsupported or corrupt.
	 *
	 *  @param[in,out] gb	The GetBit context
	 *  @returns		Number of wavelet bands or 0 on error
	 */
	int decodePlaneSubdivision();

private:
	/**
	 * Standard picture dimensions
	 */
	static const uint _ivi4_common_pic_sizes[14];

	/**
	 * Transformations list
	 */
	static Transform _transforms[18];

	static const uint8 *const _scan_index_to_tab[15];

	/**
	 *  Indeo 4 dequant tables
	 */
	static const uint16 _ivi4_quant_8x8_intra[9][64];

	static const uint16 _ivi4_quant_8x8_inter[9][64];

	static const uint16 _ivi4_quant_4x4_intra[5][16];

	static const uint16 _ivi4_quant_4x4_inter[5][16];

	/**
	 *  Table for mapping quant matrix index from the bitstream
	 *  into internal quant table number.
	 */
	static const uint8 _quant_index_to_tab[22];
};

} // End of namespace Image

#endif
