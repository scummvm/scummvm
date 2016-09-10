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

#include "common/scummsys.h"

/* Intel Indeo 5 decompressor, derived from ffmpeg.
 *
 * Original copyright note: * Intel Indeo 3 (IV41, IV42, etc.) video decoder for ffmpeg
 * written, produced, and directed by Alan Smithee
 */

#include "common/endian.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "graphics/yuv_to_rgb.h"
#include "image/codecs/indeo5.h"
#include "image/codecs/indeo/mem.h"

namespace Image {

/**
 *  Indeo5 frame types.
 */
enum {
    FRAMETYPE_INTRA       = 0,
    FRAMETYPE_INTER       = 1,  ///< non-droppable P-frame
    FRAMETYPE_INTER_SCAL  = 2,  ///< droppable P-frame used in the scalability mode
    FRAMETYPE_INTER_NOREF = 3,  ///< droppable P-frame
    FRAMETYPE_NULL        = 4   ///< empty frame with no data
};

#define IVI5_PIC_SIZE_ESC       15

Indeo5Decoder::Indeo5Decoder(uint16 width, uint16 height) : IndeoDecoderBase(width, height) {
	_ctx._isIndeo4 = false;
	_ctx._refBuf = 1;
	_ctx._bRefBuf = 3;
	_ctx._pFrame = new AVFrame();
}

bool Indeo5Decoder::isIndeo5(Common::SeekableReadStream &stream) {
	// Less than 16 bytes? This can't be right
	if (stream.size() < 16)
		return false;

	// Read in the start of the data
	byte buffer[16];
	stream.read(buffer, 16);
	stream.seek(-16, SEEK_CUR);

	// Validate the first 5-bit word has the correct identifier
	Indeo::GetBits gb(buffer, 16 * 8);
	bool isIndeo5 = gb.getBits(5) == 0x1F;

	return isIndeo5;
}

const Graphics::Surface *Indeo5Decoder::decodeFrame(Common::SeekableReadStream &stream) {
	// Not Indeo 5? Fail
	if (!isIndeo5(stream))
		return nullptr;

	// Set up the frame data buffer
	byte *frameData = new byte[stream.size()];
	stream.read(frameData, stream.size());
	_ctx._frameData = frameData;
	_ctx._frameSize = stream.size();

	// Set up the GetBits instance for reading the data
	_ctx._gb = new GetBits(_ctx._frameData, _ctx._frameSize * 8);

	// Decode the frame
	int err = decodeIndeoFrame();

	// Free the bit reader and frame buffer
	delete _ctx._gb;
	_ctx._gb = nullptr;
	delete[] frameData;
	_ctx._frameData = nullptr;
	_ctx._frameSize = 0;

	return (err < 0) ? nullptr : &_surface->rawSurface();
}

int Indeo5Decoder::decodePictureHeader() {
	int             pic_size_indx, i, p;
	IVIPicConfig    pic_conf;

	int ret;

	if (_ctx._gb->getBits(5) != 0x1F) {
		warning("Invalid picture start code!");
		return -1;
	}

	_ctx._prevFrameType = _ctx._frameType;
	_ctx._frameType = _ctx._gb->getBits(3);
	if (_ctx._frameType >= 5) {
		warning("Invalid frame type: %d", _ctx._frameType);
		return -1;
	}

	_ctx._frameNum = _ctx._gb->getBits(8);

	if (_ctx._frameType == FRAMETYPE_INTRA) {
		if ((ret = decode_gop_header()) < 0) {
			warning("Invalid GOP header, skipping frames.");
			_ctx._gopInvalid = true;
			return ret;
		}
		_ctx._gopInvalid = false;
	}

	if (_ctx._frameType == FRAMETYPE_INTER_SCAL && !_ctx._isScalable) {
		warning("Scalable inter frame in non scalable stream");
		_ctx._frameType = FRAMETYPE_INTER;
		return -1;
	}

	if (_ctx._frameType != FRAMETYPE_NULL) {
		_ctx._frameFlags = _ctx._gb->getBits(8);

		_ctx._picHdrSize = (_ctx._frameFlags & 1) ? _ctx._gb->getBitsLong(24) : 0;

		_ctx._checksum = (_ctx._frameFlags & 0x10) ? _ctx._gb->getBits(16) : 0;

		// skip unknown extension if any
		if (_ctx._frameFlags & 0x20)
			skip_hdr_extension(); // XXX: untested

										  // decode macroblock huffman codebook
		ret = _ctx._mbVlc.ff_ivi_dec_huff_desc(&_ctx, _ctx._frameFlags & 0x40,
			IVI_MB_HUFF);
		if (ret < 0)
			return ret;

		_ctx._gb->skipBits(3); // FIXME: unknown meaning!
	}

	_ctx._gb->alignGetBits();
	return 0;
}

void Indeo5Decoder::switch_buffers() {
	switch (_ctx._prevFrameType) {
	case FRAMETYPE_INTRA:
	case FRAMETYPE_INTER:
		_ctx._bufSwitch ^= 1;
		_ctx._dstBuf = _ctx._bufSwitch;
		_ctx._refBuf = _ctx._bufSwitch ^ 1;
		break;
	case FRAMETYPE_INTER_SCAL:
		if (!_ctx._interScal) {
			_ctx._ref2Buf = 2;
			_ctx._interScal = 1;
		}
		FFSWAP(int, _ctx._dstBuf, _ctx._ref2Buf);
		_ctx._refBuf = _ctx._ref2Buf;
		break;
	case FRAMETYPE_INTER_NOREF:
		break;
	}

	switch (_ctx._frameType) {
	case FRAMETYPE_INTRA:
		_ctx._bufSwitch = 0;
		// FALLTHROUGH
	case FRAMETYPE_INTER:
		_ctx._interScal = 0;
		_ctx._dstBuf = _ctx._bufSwitch;
		_ctx._refBuf = _ctx._bufSwitch ^ 1;
		break;
	case FRAMETYPE_INTER_SCAL:
	case FRAMETYPE_INTER_NOREF:
	case FRAMETYPE_NULL:
		break;
	}
}

bool Indeo5Decoder::is_nonnull_frame() const {
	return _ctx._frameType != FRAMETYPE_NULL;
}

int Indeo5Decoder::decode_band_hdr(IVIBandDesc *band) {
	int         i, ret;
	uint8     band_flags;

	band_flags = _ctx._gb->getBits(8);

	if (band_flags & 1) {
		band->_isEmpty = true;
		return 0;
	}

	band->_dataSize = (_ctx._frameFlags & 0x80) ? _ctx._gb->getBitsLong(24) : 0;

	band->_inheritMv = (band_flags & 2) != 0;
	band->_inheritQDelta = (band_flags & 8) != 0;
	band->_qdeltaPresent = (band_flags & 4) != 0;
	if (!band->_qdeltaPresent)
		band->_inheritQDelta = 1;

	// decode rvmap probability corrections if any
	band->_numCorr = 0; // there are no corrections
	if (band_flags & 0x10) {
		band->_numCorr = _ctx._gb->getBits(8); // get number of correction pairs
		if (band->_numCorr > 61) {
			warning("Too many corrections: %d", band->_numCorr);
			return -1;
		}

		// read correction pairs
		for (i = 0; i < band->_numCorr * 2; i++)
			band->_corr[i] = _ctx._gb->getBits(8);
	}

	// select appropriate rvmap table for this band
	band->_rvmapSel = (band_flags & 0x40) ? _ctx._gb->getBits(3) : 8;

	// decode block huffman codebook
	ret = band->_blkVlc.ff_ivi_dec_huff_desc(&_ctx, band_flags & 0x80, IVI_BLK_HUFF);
	if (ret < 0)
		return ret;

	band->_checksumPresent = _ctx._gb->getBits1();
	if (band->_checksumPresent)
		band->_checksum = _ctx._gb->getBits(16);

	band->_globQuant = _ctx._gb->getBits(5);

	// skip unknown extension if any
	if (band_flags & 0x20) { // XXX: untested
		_ctx._gb->alignGetBits();
		skip_hdr_extension();
	}

	_ctx._gb->alignGetBits();

	return 0;
}

int Indeo5Decoder::decode_mb_info(IVIBandDesc *band, IVITile *tile) {
	int         x, y, _mvX, _mvY, mv_delta, offs, mb_offset,
		mv_scale, blks_per_mb, s;
	IVIMbInfo   *mb, *ref_mb;
	int         row_offset = band->_mbSize * band->_pitch;

	mb = tile->_mbs;
	ref_mb = tile->_refMbs;
	offs = tile->_yPos * band->_pitch + tile->_xPos;

	if (!ref_mb &&
		((band->_qdeltaPresent && band->_inheritQDelta) || band->_inheritMv))
		return -1;

	if (tile->_numMBs != IVI_MBs_PER_TILE(tile->_width, tile->_height, band->_mbSize)) {
		warning("Allocated tile size %d mismatches parameters %d",
			tile->_numMBs, IVI_MBs_PER_TILE(tile->_width, tile->_height, band->_mbSize));
		return -1;
	}

	// scale factor for motion vectors
	mv_scale = (_ctx._planes[0]._bands[0]._mbSize >> 3) - (band->_mbSize >> 3);
	_mvX = _mvY = 0;

	for (y = tile->_yPos; y < (tile->_yPos + tile->_height); y += band->_mbSize) {
		mb_offset = offs;

		for (x = tile->_xPos; x < (tile->_xPos + tile->_width); x += band->_mbSize) {
			mb->_xPos = x;
			mb->_yPos = y;
			mb->_bufOffs = mb_offset;

			if (_ctx._gb->getBits1()) {
				if (_ctx._frameType == FRAMETYPE_INTRA) {
					warning("Empty macroblock in an INTRA picture!");
					return -1;
				}
				mb->_type = 1; // empty macroblocks are always INTER
				mb->_cbp = 0; // all blocks are empty

				mb->_qDelta = 0;
				if (!band->_plane && !band->_bandNum && (_ctx._frameFlags & 8)) {
					mb->_qDelta = _ctx._gb->getVLC2(_ctx._mbVlc._tab->_table, IVI_VLC_BITS, 1);
					mb->_qDelta = IVI_TOSIGNED(mb->_qDelta);
				}

				mb->_mvX = mb->_mvY = 0; // no motion vector coded
				if (band->_inheritMv && ref_mb) {
					// motion vector inheritance
					if (mv_scale) {
						mb->_mvX = ivi_scale_mv(ref_mb->_mvX, mv_scale);
						mb->_mvY = ivi_scale_mv(ref_mb->_mvY, mv_scale);
					} else {
						mb->_mvX = ref_mb->_mvX;
						mb->_mvY = ref_mb->_mvY;
					}
				}
			} else {
				if (band->_inheritMv && ref_mb) {
					mb->_type = ref_mb->_type; // copy mb_type from corresponding reference mb
				} else if (_ctx._frameType == FRAMETYPE_INTRA) {
					mb->_type = 0; // mb_type is always INTRA for intra-frames
				} else {
					mb->_type = _ctx._gb->getBits1();
				}

				blks_per_mb = band->_mbSize != band->_blkSize ? 4 : 1;
				mb->_cbp = _ctx._gb->getBits(blks_per_mb);

				mb->_qDelta = 0;
				if (band->_qdeltaPresent) {
					if (band->_inheritQDelta) {
						if (ref_mb) mb->_qDelta = ref_mb->_qDelta;
					} else if (mb->_cbp || (!band->_plane && !band->_bandNum &&
						(_ctx._frameFlags & 8))) {
						mb->_qDelta = _ctx._gb->getVLC2(_ctx._mbVlc._tab->_table, IVI_VLC_BITS, 1);
						mb->_qDelta = IVI_TOSIGNED(mb->_qDelta);
					}
				}

				if (!mb->_type) {
					mb->_mvX = mb->_mvY = 0; // there is no motion vector in intra-macroblocks
				} else {
					if (band->_inheritMv && ref_mb) {
						// motion vector inheritance
						if (mv_scale) {
							mb->_mvX = ivi_scale_mv(ref_mb->_mvX, mv_scale);
							mb->_mvY = ivi_scale_mv(ref_mb->_mvY, mv_scale);
						} else {
							mb->_mvX = ref_mb->_mvX;
							mb->_mvY = ref_mb->_mvY;
						}
					} else {
						// decode motion vector deltas
						mv_delta = _ctx._gb->getVLC2(_ctx._mbVlc._tab->_table, IVI_VLC_BITS, 1);
						_mvY += IVI_TOSIGNED(mv_delta);
						mv_delta = _ctx._gb->getVLC2(_ctx._mbVlc._tab->_table, IVI_VLC_BITS, 1);
						_mvX += IVI_TOSIGNED(mv_delta);
						mb->_mvX = _mvX;
						mb->_mvY = _mvY;
					}
				}
			}

			s = band->_isHalfpel;
			if (mb->_type)
				if (x + (mb->_mvX >> s) + (y + (mb->_mvY >> s))*band->_pitch < 0 ||
					x + ((mb->_mvX + s) >> s) + band->_mbSize - 1
					+ (y + band->_mbSize - 1 + ((mb->_mvY + s) >> s))*band->_pitch > band->_bufSize - 1) {
					warning("motion vector %d %d outside reference", x*s + mb->_mvX, y*s + mb->_mvY);
					return -1;
				}

			mb++;
			if (ref_mb)
				ref_mb++;
			mb_offset += band->_mbSize;
		}

		offs += row_offset;
	}

	_ctx._gb->alignGetBits();

	return 0;
}

int Indeo5Decoder::decode_gop_header() {
	int				result, i, p, tile_size, pic_size_indx, mbSize, blkSize, isScalable;
	int				quantMat;
	bool			blk_size_changed = false;
	IVIBandDesc     *band, *band1, *band2;
	IVIPicConfig    pic_conf;

	_ctx._gopFlags = _ctx._gb->getBits(8);

	_ctx._gopHdrSize = (_ctx._gopFlags & 1) ? _ctx._gb->getBits(16) : 0;

	if (_ctx._gopFlags & IVI5_IS_PROTECTED)
		_ctx._lockWord = _ctx._gb->getBitsLong(32);

	tile_size = (_ctx._gopFlags & 0x40) ? 64 << _ctx._gb->getBits(2) : 0;
	if (tile_size > 256) {
		warning("Invalid tile size: %d", tile_size);
		return -1;
	}

	// decode number of wavelet bands
	// num_levels * 3 + 1
	pic_conf._lumaBands = _ctx._gb->getBits(2) * 3 + 1;
	pic_conf._chromaBands = _ctx._gb->getBits1() * 3 + 1;
	isScalable = pic_conf._lumaBands != 1 || pic_conf._chromaBands != 1;
	if (isScalable && (pic_conf._lumaBands != 4 || pic_conf._chromaBands != 1)) {
		warning("Scalability: unsupported subdivision! Luma bands: %d, chroma bands: %d",
			pic_conf._lumaBands, pic_conf._chromaBands);
		return -1;
	}

	pic_size_indx = _ctx._gb->getBits(4);
	if (pic_size_indx == IVI5_PIC_SIZE_ESC) {
		pic_conf._picHeight = _ctx._gb->getBits(13);
		pic_conf._picWidth = _ctx._gb->getBits(13);
	} else {
		pic_conf._picHeight = _ivi5_common_pic_sizes[pic_size_indx * 2 + 1] << 2;
		pic_conf._picWidth = _ivi5_common_pic_sizes[pic_size_indx * 2] << 2;
	}

	if (_ctx._gopFlags & 2) {
		warning("YV12 picture format");
		return -2;
	}

	pic_conf._chromaHeight = (pic_conf._picHeight + 3) >> 2;
	pic_conf._chromaWidth = (pic_conf._picWidth + 3) >> 2;

	if (!tile_size) {
		pic_conf._tileHeight = pic_conf._picHeight;
		pic_conf._tileWidth = pic_conf._picWidth;
	} else {
		pic_conf._tileHeight = pic_conf._tileWidth = tile_size;
	}

	// check if picture layout was changed and reallocate buffers
	if (pic_conf.ivi_pic_config_cmp(_ctx._picConf) || _ctx._gopInvalid) {
		result = IVIPlaneDesc::ff_ivi_init_planes(_ctx._planes, &pic_conf, 0);
		if (result < 0) {
			warning("Couldn't reallocate color planes!");
			return result;
		}
		_ctx._picConf = pic_conf;
		_ctx._isScalable = isScalable;
		blk_size_changed = 1; // force reallocation of the internal structures
	}

	for (p = 0; p <= 1; p++) {
		for (i = 0; i < (!p ? pic_conf._lumaBands : pic_conf._chromaBands); i++) {
			band = &_ctx._planes[p]._bands[i];

			band->_isHalfpel = _ctx._gb->getBits1();

			mbSize = _ctx._gb->getBits1();
			blkSize = 8 >> _ctx._gb->getBits1();
			mbSize = blkSize << !mbSize;

			if (p == 0 && blkSize == 4) {
				warning("4x4 luma blocks are unsupported!");
				return -2;
			}

			blk_size_changed = mbSize != band->_mbSize || blkSize != band->_blkSize;
			if (blk_size_changed) {
				band->_mbSize = mbSize;
				band->_blkSize = blkSize;
			}

			if (_ctx._gb->getBits1()) {
				warning("Extended transform info");
				return -2;
			}

			// select transform function and scan pattern according to plane and band number
			switch ((p << 2) + i) {
			case 0:
				band->_invTransform = IndeoDSP::ff_ivi_inverse_slant_8x8;
				band->_dcTransform = IndeoDSP::ff_ivi_dc_slant_2d;
				band->_scan = ff_zigzag_direct;
				band->_transformSize = 8;
				break;

			case 1:
				band->_invTransform = IndeoDSP::ff_ivi_row_slant8;
				band->_dcTransform = IndeoDSP::ff_ivi_dc_row_slant;
				band->_scan = _ff_ivi_vertical_scan_8x8;
				band->_transformSize = 8;
				break;

			case 2:
				band->_invTransform = IndeoDSP::ff_ivi_col_slant8;
				band->_dcTransform = IndeoDSP::ff_ivi_dc_col_slant;
				band->_scan = _ff_ivi_horizontal_scan_8x8;
				band->_transformSize = 8;
				break;

			case 3:
				band->_invTransform = IndeoDSP::ff_ivi_put_pixels_8x8;
				band->_dcTransform = IndeoDSP::ff_ivi_put_dc_pixel_8x8;
				band->_scan = _ff_ivi_horizontal_scan_8x8;
				band->_transformSize = 8;
				break;

			case 4:
				band->_invTransform = IndeoDSP::ff_ivi_inverse_slant_4x4;
				band->_dcTransform = IndeoDSP::ff_ivi_dc_slant_2d;
				band->_scan = _ff_ivi_direct_scan_4x4;
				band->_transformSize = 4;
				break;
			}

			band->_is2dTrans = band->_invTransform == IndeoDSP::ff_ivi_inverse_slant_8x8 ||
				band->_invTransform == IndeoDSP::ff_ivi_inverse_slant_4x4;

			if (band->_transformSize != band->_blkSize) {
				warning("transform and block size mismatch (%d != %d)", band->_transformSize, band->_blkSize);
				return -1;
			}

			// select dequant matrix according to plane and band number
			if (!p) {
				quantMat = (pic_conf._lumaBands > 1) ? i + 1 : 0;
			} else {
				quantMat = 5;
			}

			if (band->_blkSize == 8) {
				if (quantMat >= 5) {
					warning("_quantMat %d too large!", quantMat);
					return -1;
				}
				band->_intraBase = &_ivi5_base_quant_8x8_intra[quantMat][0];
				band->_interBase = &_ivi5_base_quant_8x8_inter[quantMat][0];
				band->_intraScale = &_ivi5_scale_quant_8x8_intra[quantMat][0];
				band->_interScale = &_ivi5_scale_quant_8x8_inter[quantMat][0];
			} else {
				band->_intraBase = _ivi5_base_quant_4x4_intra;
				band->_interBase = _ivi5_base_quant_4x4_inter;
				band->_intraScale = _ivi5_scale_quant_4x4_intra;
				band->_interScale = _ivi5_scale_quant_4x4_inter;
			}

			if (_ctx._gb->getBits(2)) {
				warning("End marker missing!");
				return -1;
			}
		}
	}

	// copy chroma parameters into the 2nd chroma plane
	for (i = 0; i < pic_conf._chromaBands; i++) {
		band1 = &_ctx._planes[1]._bands[i];
		band2 = &_ctx._planes[2]._bands[i];

		band2->_width = band1->_width;
		band2->_height = band1->_height;
		band2->_mbSize = band1->_mbSize;
		band2->_blkSize = band1->_blkSize;
		band2->_isHalfpel = band1->_isHalfpel;
		band2->_intraBase = band1->_intraBase;
		band2->_interBase = band1->_interBase;
		band2->_intraScale = band1->_intraScale;
		band2->_interScale = band1->_interScale;
		band2->_scan = band1->_scan;
		band2->_invTransform = band1->_invTransform;
		band2->_dcTransform = band1->_dcTransform;
		band2->_is2dTrans = band1->_is2dTrans;
		band2->_transformSize = band1->_transformSize;
	}

	// reallocate internal structures if needed
	if (blk_size_changed) {
		result = IVIPlaneDesc::ff_ivi_init_tiles(_ctx._planes, pic_conf._tileWidth,
			pic_conf._tileHeight);
		if (result < 0) {
			warning("Couldn't reallocate internal structures!");
			return result;
		}
	}

	if (_ctx._gopFlags & 8) {
		if (_ctx._gb->getBits(3)) {
			warning("Alignment bits are not zero!");
			return -1;
		}

		if (_ctx._gb->getBits1())
			_ctx._gb->skipBitsLong(24); // skip transparency fill color
	}

	_ctx._gb->alignGetBits();

	_ctx._gb->skipBits(23); // FIXME: unknown meaning

							 // skip GOP extension if any
	if (_ctx._gb->getBits1()) {
		do {
			i = _ctx._gb->getBits(16);
		} while (i & 0x8000);
	}

	_ctx._gb->alignGetBits();

	return 0;
}

int Indeo5Decoder::skip_hdr_extension() {
	int i, len;

	do {
		len = _ctx._gb->getBits(8);
		if (8 * len > _ctx._gb->getBitsLeft())
			return -1;
		for (i = 0; i < len; i++)
			_ctx._gb->skipBits(8);
	} while (len);

	return 0;
}

/*------------------------------------------------------------------------*/

const uint8 Indeo5Decoder::_ivi5_common_pic_sizes[30] = {
	160, 120, 80, 60, 40, 30, 176, 120, 88, 60, 88, 72, 44, 36, 60, 45, 160, 60,
	176,  60, 20, 15, 22, 18,   0,   0,  0,  0,  0,  0
};

const uint8 Indeo5Decoder::ivi5_common_pic_sizes[30] = {
    160, 120, 80, 60, 40, 30, 176, 120, 88, 60, 88, 72, 44, 36, 60, 45, 160, 60,
    176,  60, 20, 15, 22, 18,   0,   0,  0,  0,  0,  0
};

const uint16 Indeo5Decoder::_ivi5_base_quant_8x8_inter[5][64] = {
    {0x26, 0x3a, 0x3e, 0x46, 0x4a, 0x4e, 0x52, 0x5a, 0x3a, 0x3e, 0x42, 0x46, 0x4a, 0x4e, 0x56, 0x5e,
     0x3e, 0x42, 0x46, 0x48, 0x4c, 0x52, 0x5a, 0x62, 0x46, 0x46, 0x48, 0x4a, 0x4e, 0x56, 0x5e, 0x66,
     0x4a, 0x4a, 0x4c, 0x4e, 0x52, 0x5a, 0x62, 0x6a, 0x4e, 0x4e, 0x52, 0x56, 0x5a, 0x5e, 0x66, 0x6e,
     0x52, 0x56, 0x5a, 0x5e, 0x62, 0x66, 0x6a, 0x72, 0x5a, 0x5e, 0x62, 0x66, 0x6a, 0x6e, 0x72, 0x76,
    },
    {0x26, 0x3a, 0x3e, 0x46, 0x4a, 0x4e, 0x52, 0x5a, 0x3a, 0x3e, 0x42, 0x46, 0x4a, 0x4e, 0x56, 0x5e,
     0x3e, 0x42, 0x46, 0x48, 0x4c, 0x52, 0x5a, 0x62, 0x46, 0x46, 0x48, 0x4a, 0x4e, 0x56, 0x5e, 0x66,
     0x4a, 0x4a, 0x4c, 0x4e, 0x52, 0x5a, 0x62, 0x6a, 0x4e, 0x4e, 0x52, 0x56, 0x5a, 0x5e, 0x66, 0x6e,
     0x52, 0x56, 0x5a, 0x5e, 0x62, 0x66, 0x6a, 0x72, 0x5a, 0x5e, 0x62, 0x66, 0x6a, 0x6e, 0x72, 0x76,
    },
    {0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2, 0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2,
     0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2, 0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2,
     0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2, 0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2,
     0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2, 0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2,
    },
    {0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
     0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xd4, 0xd4, 0xd4, 0xd4, 0xd4, 0xd4, 0xd4, 0xd4,
     0xde, 0xde, 0xde, 0xde, 0xde, 0xde, 0xde, 0xde, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2,
     0xd6, 0xd6, 0xd6, 0xd6, 0xd6, 0xd6, 0xd6, 0xd6, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2,
    },
    {0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e,
     0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e,
     0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e,
     0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e,
    }
};

const uint16 Indeo5Decoder::_ivi5_base_quant_8x8_intra[5][64] = {
    {0x1a, 0x2e, 0x36, 0x42, 0x46, 0x4a, 0x4e, 0x5a, 0x2e, 0x32, 0x3e, 0x42, 0x46, 0x4e, 0x56, 0x6a,
     0x36, 0x3e, 0x3e, 0x44, 0x4a, 0x54, 0x66, 0x72, 0x42, 0x42, 0x44, 0x4a, 0x52, 0x62, 0x6c, 0x7a,
     0x46, 0x46, 0x4a, 0x52, 0x5e, 0x66, 0x72, 0x8e, 0x4a, 0x4e, 0x54, 0x62, 0x66, 0x6e, 0x86, 0xa6,
     0x4e, 0x56, 0x66, 0x6c, 0x72, 0x86, 0x9a, 0xca, 0x5a, 0x6a, 0x72, 0x7a, 0x8e, 0xa6, 0xca, 0xfe,
    },
    {0x26, 0x3a, 0x3e, 0x46, 0x4a, 0x4e, 0x52, 0x5a, 0x3a, 0x3e, 0x42, 0x46, 0x4a, 0x4e, 0x56, 0x5e,
     0x3e, 0x42, 0x46, 0x48, 0x4c, 0x52, 0x5a, 0x62, 0x46, 0x46, 0x48, 0x4a, 0x4e, 0x56, 0x5e, 0x66,
     0x4a, 0x4a, 0x4c, 0x4e, 0x52, 0x5a, 0x62, 0x6a, 0x4e, 0x4e, 0x52, 0x56, 0x5a, 0x5e, 0x66, 0x6e,
     0x52, 0x56, 0x5a, 0x5e, 0x62, 0x66, 0x6a, 0x72, 0x5a, 0x5e, 0x62, 0x66, 0x6a, 0x6e, 0x72, 0x76,
    },
    {0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2, 0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2,
     0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2, 0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2,
     0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2, 0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2,
     0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2, 0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2,
    },
    {0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
     0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xd4, 0xd4, 0xd4, 0xd4, 0xd4, 0xd4, 0xd4, 0xd4,
     0xde, 0xde, 0xde, 0xde, 0xde, 0xde, 0xde, 0xde, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2,
     0xd6, 0xd6, 0xd6, 0xd6, 0xd6, 0xd6, 0xd6, 0xd6, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2,
    },
    {0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e,
     0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e,
     0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e,
     0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e,
    }
};

const uint16 Indeo5Decoder::_ivi5_base_quant_4x4_inter[16] = {
    0x1e, 0x3e, 0x4a, 0x52, 0x3e, 0x4a, 0x52, 0x56, 0x4a, 0x52, 0x56, 0x5e, 0x52, 0x56, 0x5e, 0x66
};

const uint16 Indeo5Decoder::_ivi5_base_quant_4x4_intra[16] = {
    0x1e, 0x3e, 0x4a, 0x52, 0x3e, 0x4a, 0x52, 0x5e, 0x4a, 0x52, 0x5e, 0x7a, 0x52, 0x5e, 0x7a, 0x92
};


const uint8 Indeo5Decoder::_ivi5_scale_quant_8x8_inter[5][24] = {
    {0x0b, 0x11, 0x13, 0x14, 0x15, 0x16, 0x18, 0x1a, 0x1b, 0x1d, 0x20, 0x22,
     0x23, 0x25, 0x28, 0x2a, 0x2e, 0x32, 0x35, 0x39, 0x3d, 0x41, 0x44, 0x4a,
    },
    {0x07, 0x14, 0x16, 0x18, 0x1b, 0x1e, 0x22, 0x25, 0x29, 0x2d, 0x31, 0x35,
     0x3a, 0x3f, 0x44, 0x4a, 0x50, 0x56, 0x5c, 0x63, 0x6a, 0x71, 0x78, 0x7e,
    },
    {0x15, 0x25, 0x28, 0x2d, 0x30, 0x34, 0x3a, 0x3d, 0x42, 0x48, 0x4c, 0x51,
     0x56, 0x5b, 0x60, 0x65, 0x6b, 0x70, 0x76, 0x7c, 0x82, 0x88, 0x8f, 0x97,
    },
    {0x13, 0x1f, 0x20, 0x22, 0x25, 0x28, 0x2b, 0x2d, 0x30, 0x33, 0x36, 0x39,
     0x3c, 0x3f, 0x42, 0x45, 0x48, 0x4b, 0x4e, 0x52, 0x56, 0x5a, 0x5e, 0x62,
    },
    {0x3c, 0x52, 0x58, 0x5d, 0x63, 0x68, 0x68, 0x6d, 0x73, 0x78, 0x7c, 0x80,
     0x84, 0x89, 0x8e, 0x93, 0x98, 0x9d, 0xa3, 0xa9, 0xad, 0xb1, 0xb5, 0xba,
    },
};

const uint8 Indeo5Decoder::_ivi5_scale_quant_8x8_intra[5][24] = {
    {0x0b, 0x0e, 0x10, 0x12, 0x14, 0x16, 0x17, 0x18, 0x1a, 0x1c, 0x1e, 0x20,
     0x22, 0x24, 0x27, 0x28, 0x2a, 0x2d, 0x2f, 0x31, 0x34, 0x37, 0x39, 0x3c,
    },
    {0x01, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1b, 0x1e, 0x22, 0x25, 0x28, 0x2c,
     0x30, 0x34, 0x38, 0x3d, 0x42, 0x47, 0x4c, 0x52, 0x58, 0x5e, 0x65, 0x6c,
    },
    {0x13, 0x22, 0x27, 0x2a, 0x2d, 0x33, 0x36, 0x3c, 0x41, 0x45, 0x49, 0x4e,
     0x53, 0x58, 0x5d, 0x63, 0x69, 0x6f, 0x75, 0x7c, 0x82, 0x88, 0x8e, 0x95,
    },
    {0x13, 0x1f, 0x21, 0x24, 0x27, 0x29, 0x2d, 0x2f, 0x34, 0x37, 0x3a, 0x3d,
     0x40, 0x44, 0x48, 0x4c, 0x4f, 0x52, 0x56, 0x5a, 0x5e, 0x62, 0x66, 0x6b,
    },
    {0x31, 0x42, 0x47, 0x47, 0x4d, 0x52, 0x58, 0x58, 0x5d, 0x63, 0x67, 0x6b,
     0x6f, 0x73, 0x78, 0x7c, 0x80, 0x84, 0x89, 0x8e, 0x93, 0x98, 0x9d, 0xa4,
    }
};

const uint8 Indeo5Decoder::_ivi5_scale_quant_4x4_inter[24] = {
    0x0b, 0x0d, 0x0d, 0x0e, 0x11, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
};

const uint8 Indeo5Decoder::_ivi5_scale_quant_4x4_intra[24] = {
    0x01, 0x0b, 0x0b, 0x0d, 0x0d, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x13, 0x14,
    0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20
};

} // End of namespace Image
