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
	_ctx.is_indeo4 = true;
	_ctx.ref_buf = 1;
	_ctx.b_ref_buf = 3;
	_ctx.p_frame = new AVFrame();
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
	_ctx.frame_data = frameData;
	_ctx.frame_size = stream.size();

	// Set up the GetBits instance for reading the data
	_ctx.gb = new GetBits(_ctx.frame_data, _ctx.frame_size * 8);

	// Decode the frame
	int err = decodeIndeoFrame();

	// Free the bit reader and frame buffer
	delete _ctx.gb;
	_ctx.gb = nullptr;
	delete[] frameData;
	_ctx.frame_data = nullptr;
	_ctx.frame_size = 0;

	return (err < 0) ? nullptr : &_surface->rawSurface();
}

int Indeo5Decoder::decodePictureHeader() {
	int             pic_size_indx, i, p;
	IVIPicConfig    pic_conf;

	int ret;

	if (_ctx.gb->getBits(5) != 0x1F) {
		warning("Invalid picture start code!");
		return -1;
	}

	_ctx.prev_frame_type = _ctx.frame_type;
	_ctx.frame_type = _ctx.gb->getBits(3);
	if (_ctx.frame_type >= 5) {
		warning("Invalid frame type: %d", _ctx.frame_type);
		return -1;
	}

	_ctx.frame_num = _ctx.gb->getBits(8);

	if (_ctx.frame_type == FRAMETYPE_INTRA) {
		if ((ret = decode_gop_header()) < 0) {
			warning("Invalid GOP header, skipping frames.");
			_ctx.gop_invalid = 1;
			return ret;
		}
		_ctx.gop_invalid = 0;
	}

	if (_ctx.frame_type == FRAMETYPE_INTER_SCAL && !_ctx.is_scalable) {
		warning("Scalable inter frame in non scalable stream");
		_ctx.frame_type = FRAMETYPE_INTER;
		return -1;
	}

	if (_ctx.frame_type != FRAMETYPE_NULL) {
		_ctx.frame_flags = _ctx.gb->getBits(8);

		_ctx.pic_hdr_size = (_ctx.frame_flags & 1) ? _ctx.gb->getBitsLong(24) : 0;

		_ctx.checksum = (_ctx.frame_flags & 0x10) ? _ctx.gb->getBits(16) : 0;

		// skip unknown extension if any
		if (_ctx.frame_flags & 0x20)
			skip_hdr_extension(); // XXX: untested

										  // decode macroblock huffman codebook
		ret = _ctx.mb_vlc.ff_ivi_dec_huff_desc(&_ctx, _ctx.frame_flags & 0x40,
			IVI_MB_HUFF);
		if (ret < 0)
			return ret;

		_ctx.gb->skipBits(3); // FIXME: unknown meaning!
	}

	_ctx.gb->alignGetBits();
	return 0;
}

void Indeo5Decoder::switch_buffers() {
	switch (_ctx.prev_frame_type) {
	case FRAMETYPE_INTRA:
	case FRAMETYPE_INTER:
		_ctx.buf_switch ^= 1;
		_ctx.dst_buf = _ctx.buf_switch;
		_ctx.ref_buf = _ctx.buf_switch ^ 1;
		break;
	case FRAMETYPE_INTER_SCAL:
		if (!_ctx.inter_scal) {
			_ctx.ref2_buf = 2;
			_ctx.inter_scal = 1;
		}
		FFSWAP(int, _ctx.dst_buf, _ctx.ref2_buf);
		_ctx.ref_buf = _ctx.ref2_buf;
		break;
	case FRAMETYPE_INTER_NOREF:
		break;
	}

	switch (_ctx.frame_type) {
	case FRAMETYPE_INTRA:
		_ctx.buf_switch = 0;
		// FALLTHROUGH
	case FRAMETYPE_INTER:
		_ctx.inter_scal = 0;
		_ctx.dst_buf = _ctx.buf_switch;
		_ctx.ref_buf = _ctx.buf_switch ^ 1;
		break;
	case FRAMETYPE_INTER_SCAL:
	case FRAMETYPE_INTER_NOREF:
	case FRAMETYPE_NULL:
		break;
	}
}

bool Indeo5Decoder::is_nonnull_frame() const {
	return _ctx.frame_type != FRAMETYPE_NULL;
}

int Indeo5Decoder::decode_band_hdr(IVIBandDesc *band) {
	int         i, ret;
	uint8     band_flags;

	band_flags = _ctx.gb->getBits(8);

	if (band_flags & 1) {
		band->is_empty = 1;
		return 0;
	}

	band->data_size = (_ctx.frame_flags & 0x80) ? _ctx.gb->getBitsLong(24) : 0;

	band->inherit_mv = band_flags & 2;
	band->inherit_qdelta = band_flags & 8;
	band->qdelta_present = band_flags & 4;
	if (!band->qdelta_present) band->inherit_qdelta = 1;

	// decode rvmap probability corrections if any
	band->num_corr = 0; // there are no corrections
	if (band_flags & 0x10) {
		band->num_corr = _ctx.gb->getBits(8); // get number of correction pairs
		if (band->num_corr > 61) {
			warning("Too many corrections: %d", band->num_corr);
			return -1;
		}

		// read correction pairs
		for (i = 0; i < band->num_corr * 2; i++)
			band->corr[i] = _ctx.gb->getBits(8);
	}

	// select appropriate rvmap table for this band
	band->rvmap_sel = (band_flags & 0x40) ? _ctx.gb->getBits(3) : 8;

	// decode block huffman codebook
	ret = band->blk_vlc.ff_ivi_dec_huff_desc(&_ctx, band_flags & 0x80, IVI_BLK_HUFF);
	if (ret < 0)
		return ret;

	band->checksum_present = _ctx.gb->getBits1();
	if (band->checksum_present)
		band->checksum = _ctx.gb->getBits(16);

	band->glob_quant = _ctx.gb->getBits(5);

	// skip unknown extension if any
	if (band_flags & 0x20) { // XXX: untested
		_ctx.gb->alignGetBits();
		skip_hdr_extension();
	}

	_ctx.gb->alignGetBits();

	return 0;
}

int Indeo5Decoder::decode_mb_info(IVIBandDesc *band, IVITile *tile) {
	int         x, y, mv_x, mv_y, mv_delta, offs, mb_offset,
		mv_scale, blks_per_mb, s;
	IVIMbInfo   *mb, *ref_mb;
	int         row_offset = band->mb_size * band->pitch;

	mb = tile->mbs;
	ref_mb = tile->ref_mbs;
	offs = tile->ypos * band->pitch + tile->xpos;

	if (!ref_mb &&
		((band->qdelta_present && band->inherit_qdelta) || band->inherit_mv))
		return -1;

	if (tile->num_MBs != IVI_MBs_PER_TILE(tile->width, tile->height, band->mb_size)) {
		warning("Allocated tile size %d mismatches parameters %d",
			tile->num_MBs, IVI_MBs_PER_TILE(tile->width, tile->height, band->mb_size));
		return -1;
	}

	// scale factor for motion vectors
	mv_scale = (_ctx.planes[0].bands[0].mb_size >> 3) - (band->mb_size >> 3);
	mv_x = mv_y = 0;

	for (y = tile->ypos; y < (tile->ypos + tile->height); y += band->mb_size) {
		mb_offset = offs;

		for (x = tile->xpos; x < (tile->xpos + tile->width); x += band->mb_size) {
			mb->xpos = x;
			mb->ypos = y;
			mb->buf_offs = mb_offset;

			if (_ctx.gb->getBits1()) {
				if (_ctx.frame_type == FRAMETYPE_INTRA) {
					warning("Empty macroblock in an INTRA picture!");
					return -1;
				}
				mb->type = 1; // empty macroblocks are always INTER
				mb->cbp = 0; // all blocks are empty

				mb->q_delta = 0;
				if (!band->plane && !band->band_num && (_ctx.frame_flags & 8)) {
					mb->q_delta = _ctx.gb->getVLC2(_ctx.mb_vlc.tab->_table, IVI_VLC_BITS, 1);
					mb->q_delta = IVI_TOSIGNED(mb->q_delta);
				}

				mb->mv_x = mb->mv_y = 0; // no motion vector coded
				if (band->inherit_mv && ref_mb) {
					// motion vector inheritance
					if (mv_scale) {
						mb->mv_x = ivi_scale_mv(ref_mb->mv_x, mv_scale);
						mb->mv_y = ivi_scale_mv(ref_mb->mv_y, mv_scale);
					} else {
						mb->mv_x = ref_mb->mv_x;
						mb->mv_y = ref_mb->mv_y;
					}
				}
			} else {
				if (band->inherit_mv && ref_mb) {
					mb->type = ref_mb->type; // copy mb_type from corresponding reference mb
				} else if (_ctx.frame_type == FRAMETYPE_INTRA) {
					mb->type = 0; // mb_type is always INTRA for intra-frames
				} else {
					mb->type = _ctx.gb->getBits1();
				}

				blks_per_mb = band->mb_size != band->blk_size ? 4 : 1;
				mb->cbp = _ctx.gb->getBits(blks_per_mb);

				mb->q_delta = 0;
				if (band->qdelta_present) {
					if (band->inherit_qdelta) {
						if (ref_mb) mb->q_delta = ref_mb->q_delta;
					} else if (mb->cbp || (!band->plane && !band->band_num &&
						(_ctx.frame_flags & 8))) {
						mb->q_delta = _ctx.gb->getVLC2(_ctx.mb_vlc.tab->_table, IVI_VLC_BITS, 1);
						mb->q_delta = IVI_TOSIGNED(mb->q_delta);
					}
				}

				if (!mb->type) {
					mb->mv_x = mb->mv_y = 0; // there is no motion vector in intra-macroblocks
				} else {
					if (band->inherit_mv && ref_mb) {
						// motion vector inheritance
						if (mv_scale) {
							mb->mv_x = ivi_scale_mv(ref_mb->mv_x, mv_scale);
							mb->mv_y = ivi_scale_mv(ref_mb->mv_y, mv_scale);
						} else {
							mb->mv_x = ref_mb->mv_x;
							mb->mv_y = ref_mb->mv_y;
						}
					} else {
						// decode motion vector deltas
						mv_delta = _ctx.gb->getVLC2(_ctx.mb_vlc.tab->_table, IVI_VLC_BITS, 1);
						mv_y += IVI_TOSIGNED(mv_delta);
						mv_delta = _ctx.gb->getVLC2(_ctx.mb_vlc.tab->_table, IVI_VLC_BITS, 1);
						mv_x += IVI_TOSIGNED(mv_delta);
						mb->mv_x = mv_x;
						mb->mv_y = mv_y;
					}
				}
			}

			s = band->is_halfpel;
			if (mb->type)
				if (x + (mb->mv_x >> s) + (y + (mb->mv_y >> s))*band->pitch < 0 ||
					x + ((mb->mv_x + s) >> s) + band->mb_size - 1
					+ (y + band->mb_size - 1 + ((mb->mv_y + s) >> s))*band->pitch > band->bufsize - 1) {
					warning("motion vector %d %d outside reference", x*s + mb->mv_x, y*s + mb->mv_y);
					return -1;
				}

			mb++;
			if (ref_mb)
				ref_mb++;
			mb_offset += band->mb_size;
		}

		offs += row_offset;
	}

	_ctx.gb->alignGetBits();

	return 0;
}

int Indeo5Decoder::decode_gop_header() {
	int             result, i, p, tile_size, pic_size_indx, mb_size, blk_size, is_scalable;
	int             quant_mat, blk_size_changed = 0;
	IVIBandDesc     *band, *band1, *band2;
	IVIPicConfig    pic_conf;

	_ctx.gop_flags = _ctx.gb->getBits(8);

	_ctx.gop_hdr_size = (_ctx.gop_flags & 1) ? _ctx.gb->getBits(16) : 0;

	if (_ctx.gop_flags & IVI5_IS_PROTECTED)
		_ctx.lock_word = _ctx.gb->getBitsLong(32);

	tile_size = (_ctx.gop_flags & 0x40) ? 64 << _ctx.gb->getBits(2) : 0;
	if (tile_size > 256) {
		warning("Invalid tile size: %d", tile_size);
		return -1;
	}

	// decode number of wavelet bands
	// num_levels * 3 + 1
	pic_conf.luma_bands = _ctx.gb->getBits(2) * 3 + 1;
	pic_conf.chroma_bands = _ctx.gb->getBits1() * 3 + 1;
	is_scalable = pic_conf.luma_bands != 1 || pic_conf.chroma_bands != 1;
	if (is_scalable && (pic_conf.luma_bands != 4 || pic_conf.chroma_bands != 1)) {
		warning("Scalability: unsupported subdivision! Luma bands: %d, chroma bands: %d",
			pic_conf.luma_bands, pic_conf.chroma_bands);
		return -1;
	}

	pic_size_indx = _ctx.gb->getBits(4);
	if (pic_size_indx == IVI5_PIC_SIZE_ESC) {
		pic_conf.pic_height = _ctx.gb->getBits(13);
		pic_conf.pic_width = _ctx.gb->getBits(13);
	} else {
		pic_conf.pic_height = _ivi5_common_pic_sizes[pic_size_indx * 2 + 1] << 2;
		pic_conf.pic_width = _ivi5_common_pic_sizes[pic_size_indx * 2] << 2;
	}

	if (_ctx.gop_flags & 2) {
		warning("YV12 picture format");
		return -2;
	}

	pic_conf.chroma_height = (pic_conf.pic_height + 3) >> 2;
	pic_conf.chroma_width = (pic_conf.pic_width + 3) >> 2;

	if (!tile_size) {
		pic_conf.tile_height = pic_conf.pic_height;
		pic_conf.tile_width = pic_conf.pic_width;
	} else {
		pic_conf.tile_height = pic_conf.tile_width = tile_size;
	}

	// check if picture layout was changed and reallocate buffers
	if (pic_conf.ivi_pic_config_cmp(_ctx.pic_conf) || _ctx.gop_invalid) {
		result = IVIPlaneDesc::ff_ivi_init_planes(_ctx.planes, &pic_conf, 0);
		if (result < 0) {
			warning("Couldn't reallocate color planes!");
			return result;
		}
		_ctx.pic_conf = pic_conf;
		_ctx.is_scalable = is_scalable;
		blk_size_changed = 1; // force reallocation of the internal structures
	}

	for (p = 0; p <= 1; p++) {
		for (i = 0; i < (!p ? pic_conf.luma_bands : pic_conf.chroma_bands); i++) {
			band = &_ctx.planes[p].bands[i];

			band->is_halfpel = _ctx.gb->getBits1();

			mb_size = _ctx.gb->getBits1();
			blk_size = 8 >> _ctx.gb->getBits1();
			mb_size = blk_size << !mb_size;

			if (p == 0 && blk_size == 4) {
				warning("4x4 luma blocks are unsupported!");
				return -2;
			}

			blk_size_changed = mb_size != band->mb_size || blk_size != band->blk_size;
			if (blk_size_changed) {
				band->mb_size = mb_size;
				band->blk_size = blk_size;
			}

			if (_ctx.gb->getBits1()) {
				warning("Extended transform info");
				return -2;
			}

			// select transform function and scan pattern according to plane and band number
			switch ((p << 2) + i) {
			case 0:
				band->inv_transform = IndeoDSP::ff_ivi_inverse_slant_8x8;
				band->dc_transform = IndeoDSP::ff_ivi_dc_slant_2d;
				band->scan = ff_zigzag_direct;
				band->transform_size = 8;
				break;

			case 1:
				band->inv_transform = IndeoDSP::ff_ivi_row_slant8;
				band->dc_transform = IndeoDSP::ff_ivi_dc_row_slant;
				band->scan = _ff_ivi_vertical_scan_8x8;
				band->transform_size = 8;
				break;

			case 2:
				band->inv_transform = IndeoDSP::ff_ivi_col_slant8;
				band->dc_transform = IndeoDSP::ff_ivi_dc_col_slant;
				band->scan = _ff_ivi_horizontal_scan_8x8;
				band->transform_size = 8;
				break;

			case 3:
				band->inv_transform = IndeoDSP::ff_ivi_put_pixels_8x8;
				band->dc_transform = IndeoDSP::ff_ivi_put_dc_pixel_8x8;
				band->scan = _ff_ivi_horizontal_scan_8x8;
				band->transform_size = 8;
				break;

			case 4:
				band->inv_transform = IndeoDSP::ff_ivi_inverse_slant_4x4;
				band->dc_transform = IndeoDSP::ff_ivi_dc_slant_2d;
				band->scan = _ff_ivi_direct_scan_4x4;
				band->transform_size = 4;
				break;
			}

			band->is_2d_trans = band->inv_transform == IndeoDSP::ff_ivi_inverse_slant_8x8 ||
				band->inv_transform == IndeoDSP::ff_ivi_inverse_slant_4x4;

			if (band->transform_size != band->blk_size) {
				warning("transform and block size mismatch (%d != %d)", band->transform_size, band->blk_size);
				return -1;
			}

			// select dequant matrix according to plane and band number
			if (!p) {
				quant_mat = (pic_conf.luma_bands > 1) ? i + 1 : 0;
			} else {
				quant_mat = 5;
			}

			if (band->blk_size == 8) {
				if (quant_mat >= 5) {
					warning("quant_mat %d too large!", quant_mat);
					return -1;
				}
				band->intra_base = &_ivi5_base_quant_8x8_intra[quant_mat][0];
				band->inter_base = &_ivi5_base_quant_8x8_inter[quant_mat][0];
				band->intra_scale = &_ivi5_scale_quant_8x8_intra[quant_mat][0];
				band->inter_scale = &_ivi5_scale_quant_8x8_inter[quant_mat][0];
			} else {
				band->intra_base = _ivi5_base_quant_4x4_intra;
				band->inter_base = _ivi5_base_quant_4x4_inter;
				band->intra_scale = _ivi5_scale_quant_4x4_intra;
				band->inter_scale = _ivi5_scale_quant_4x4_inter;
			}

			if (_ctx.gb->getBits(2)) {
				warning("End marker missing!");
				return -1;
			}
		}
	}

	// copy chroma parameters into the 2nd chroma plane
	for (i = 0; i < pic_conf.chroma_bands; i++) {
		band1 = &_ctx.planes[1].bands[i];
		band2 = &_ctx.planes[2].bands[i];

		band2->width = band1->width;
		band2->height = band1->height;
		band2->mb_size = band1->mb_size;
		band2->blk_size = band1->blk_size;
		band2->is_halfpel = band1->is_halfpel;
		band2->intra_base = band1->intra_base;
		band2->inter_base = band1->inter_base;
		band2->intra_scale = band1->intra_scale;
		band2->inter_scale = band1->inter_scale;
		band2->scan = band1->scan;
		band2->inv_transform = band1->inv_transform;
		band2->dc_transform = band1->dc_transform;
		band2->is_2d_trans = band1->is_2d_trans;
		band2->transform_size = band1->transform_size;
	}

	// reallocate internal structures if needed
	if (blk_size_changed) {
		result = IVIPlaneDesc::ff_ivi_init_tiles(_ctx.planes, pic_conf.tile_width,
			pic_conf.tile_height);
		if (result < 0) {
			warning("Couldn't reallocate internal structures!");
			return result;
		}
	}

	if (_ctx.gop_flags & 8) {
		if (_ctx.gb->getBits(3)) {
			warning("Alignment bits are not zero!");
			return -1;
		}

		if (_ctx.gb->getBits1())
			_ctx.gb->skipBitsLong(24); // skip transparency fill color
	}

	_ctx.gb->alignGetBits();

	_ctx.gb->skipBits(23); // FIXME: unknown meaning

							 // skip GOP extension if any
	if (_ctx.gb->getBits1()) {
		do {
			i = _ctx.gb->getBits(16);
		} while (i & 0x8000);
	}

	_ctx.gb->alignGetBits();

	return 0;
}

int Indeo5Decoder::skip_hdr_extension() {
	int i, len;

	do {
		len = _ctx.gb->getBits(8);
		if (8 * len > _ctx.gb->getBitsLeft())
			return -1;
		for (i = 0; i < len; i++)
			_ctx.gb->skipBits(8);
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
