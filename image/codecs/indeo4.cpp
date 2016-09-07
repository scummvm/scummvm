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

/* Intel Indeo 4 decompressor, derived from ffmpeg.
 *
 * Original copyright note: * Intel Indeo 3 (IV41, IV42, etc.) video decoder for ffmpeg
 * written, produced, and directed by Alan Smithee
 */

#include "common/endian.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "graphics/yuv_to_rgb.h"
#include "image/codecs/indeo4.h"
#include "image/codecs/indeo/mem.h"

namespace Image {

#define IVI4_PIC_SIZE_ESC   7

Indeo4Decoder::Indeo4Decoder(uint16 width, uint16 height) : IndeoDecoderBase(width, height) {
	_ctx.is_indeo4 = true;
}

bool Indeo4Decoder::isIndeo4(Common::SeekableReadStream &stream) {
	// Less than 16 bytes? This can't be right
	if (stream.size() < 16)
		return false;

	// Read in the start of the data
	byte buffer[16];
	stream.read(buffer, 16);
	stream.seek(-16, SEEK_CUR);

	// Validate the first 18-bit word has the correct identifier
	Indeo::GetBits gb(buffer, 16 * 8);
	bool isIndeo4 = gb.getBits(18) == 0x3FFF8;

	return isIndeo4;
}

const Graphics::Surface *Indeo4Decoder::decodeFrame(Common::SeekableReadStream &stream) {
	// Not Indeo 4? Fail
	if (!isIndeo4(stream))
		return nullptr;

	// Set up the GetBits instance for reading the stream
	_ctx.gb = new GetBits(stream);

	// Decode the frame
	int err = decodeIndeoFrame();

	// Free the bit reader
	delete _ctx.gb;
	_ctx.gb = nullptr;

	return (err < 0) ? nullptr : &_surface->rawSurface();
}

int Indeo4Decoder::decodePictureHeader() {
	int             pic_size_indx, i, p;
	IVIPicConfig    pic_conf;

	if (_ctx.gb->getBits(18) != 0x3FFF8) {
		warning("Invalid picture start code!");
		return -1;
	}

	_ctx.prev_frame_type = _ctx.frame_type;
	_ctx.frame_type = _ctx.gb->getBits(3);
	if (_ctx.frame_type == 7) {
		warning("Invalid frame type: %d", _ctx.frame_type);
		return -1;
	}

	if (_ctx.frame_type == IVI4_FRAMETYPE_BIDIR)
		_ctx.has_b_frames = 1;

	_ctx.has_transp = _ctx.gb->getBits1();

	// unknown bit: Mac decoder ignores this bit, XANIM returns error
	if (_ctx.gb->getBits1()) {
		warning("Sync bit is set!");
		return -1;
	}

	_ctx.data_size = _ctx.gb->getBits1() ? _ctx.gb->getBits(24) : 0;

	// null frames don't contain anything else so we just return
	if (_ctx.frame_type >= IVI4_FRAMETYPE_NULL_FIRST) {
		warning("Null frame encountered!");
		return 0;
	}

	// Check key lock status. If enabled - ignore lock word.
	// Usually we have to prompt the user for the password, but
	// we don't do that because Indeo 4 videos can be decoded anyway
	if (_ctx.gb->getBits1()) {
		_ctx.gb->skipBitsLong(32);
		warning("Password-protected clip!");
	}

	pic_size_indx = _ctx.gb->getBits(3);
	if (pic_size_indx == IVI4_PIC_SIZE_ESC) {
		pic_conf.pic_height = _ctx.gb->getBits(16);
		pic_conf.pic_width = _ctx.gb->getBits(16);
	} else {
		pic_conf.pic_height = _ivi4_common_pic_sizes[pic_size_indx * 2 + 1];
		pic_conf.pic_width = _ivi4_common_pic_sizes[pic_size_indx * 2];
	}

	// Decode tile dimensions.
	_ctx.uses_tiling = _ctx.gb->getBits1();
	if (_ctx.uses_tiling) {
		pic_conf.tile_height = scaleTileSize(pic_conf.pic_height, _ctx.gb->getBits(4));
		pic_conf.tile_width = scaleTileSize(pic_conf.pic_width, _ctx.gb->getBits(4));
	} else {
		pic_conf.tile_height = pic_conf.pic_height;
		pic_conf.tile_width = pic_conf.pic_width;
	}

	// Decode chroma subsampling. We support only 4:4 aka YVU9.
	if (_ctx.gb->getBits(2)) {
		warning("Only YVU9 picture format is supported!");
		return -1;
	}
	pic_conf.chroma_height = (pic_conf.pic_height + 3) >> 2;
	pic_conf.chroma_width = (pic_conf.pic_width + 3) >> 2;

	// decode subdivision of the planes
	pic_conf.luma_bands = decodePlaneSubdivision();
	pic_conf.chroma_bands = 0;
	if (pic_conf.luma_bands)
		pic_conf.chroma_bands = decodePlaneSubdivision();
	_ctx.is_scalable = pic_conf.luma_bands != 1 || pic_conf.chroma_bands != 1;
	if (_ctx.is_scalable && (pic_conf.luma_bands != 4 || pic_conf.chroma_bands != 1)) {
		warning("Scalability: unsupported subdivision! Luma bands: %d, chroma bands: %d",
			pic_conf.luma_bands, pic_conf.chroma_bands);
		return -1;
	}

	// check if picture layout was changed and reallocate buffers
	if (pic_conf.ivi_pic_config_cmp(_ctx.pic_conf)) {
		if (IVIPlaneDesc::ff_ivi_init_planes(_ctx.planes, &pic_conf, 1)) {
			warning("Couldn't reallocate color planes!");
			_ctx.pic_conf.luma_bands = 0;
			return -2;
		}

		_ctx.pic_conf = pic_conf;

		// set default macroblock/block dimensions
		for (p = 0; p <= 2; p++) {
			for (i = 0; i < (!p ? pic_conf.luma_bands : pic_conf.chroma_bands); i++) {
				_ctx.planes[p].bands[i].mb_size = !p ? (!_ctx.is_scalable ? 16 : 8) : 4;
				_ctx.planes[p].bands[i].blk_size = !p ? 8 : 4;
			}
		}

		if (IVIPlaneDesc::ff_ivi_init_tiles(_ctx.planes, _ctx.pic_conf.tile_width,
			_ctx.pic_conf.tile_height)) {
			warning("Couldn't reallocate internal structures!");
			return -2;
		}
	}

	_ctx.frame_num = _ctx.gb->getBits1() ? _ctx.gb->getBits(20) : 0;

	// skip decTimeEst field if present
	if (_ctx.gb->getBits1())
		_ctx.gb->skipBits(8);

	// decode macroblock and block huffman codebooks
	if (_ctx.mb_vlc.ff_ivi_dec_huff_desc(_ctx.gb, _ctx.gb->getBits1(), IVI_MB_HUFF) ||
		_ctx.blk_vlc.ff_ivi_dec_huff_desc(_ctx.gb, _ctx.gb->getBits1(), IVI_BLK_HUFF))
		return -1;

	_ctx.rvmap_sel = _ctx.gb->getBits1() ? _ctx.gb->getBits(3) : 8;

	_ctx.in_imf = _ctx.gb->getBits1();
	_ctx.in_q = _ctx.gb->getBits1();

	_ctx.pic_glob_quant = _ctx.gb->getBits(5);

	// TODO: ignore this parameter if unused
	_ctx.unknown1 = _ctx.gb->getBits1() ? _ctx.gb->getBits(3) : 0;

	_ctx.checksum = _ctx.gb->getBits1() ? _ctx.gb->getBits(16) : 0;

	// skip picture header extension if any
	while (_ctx.gb->getBits1()) {
		warning("Pic hdr extension encountered!");
		_ctx.gb->skipBits(8);
	}

	if (_ctx.gb->getBits1()) {
		warning("Bad blocks bits encountered!");
	}

	_ctx.gb->alignGetBits();

	return 0;
}

void Indeo4Decoder::switch_buffers() {
	int is_prev_ref = 0, is_ref = 0;

	switch (_ctx.prev_frame_type) {
	case IVI4_FRAMETYPE_INTRA:
	case IVI4_FRAMETYPE_INTRA1:
	case IVI4_FRAMETYPE_INTER:
		is_prev_ref = 1;
		break;
	}

	switch (_ctx.frame_type) {
	case IVI4_FRAMETYPE_INTRA:
	case IVI4_FRAMETYPE_INTRA1:
	case IVI4_FRAMETYPE_INTER:
		is_ref = 1;
		break;

	default:
		break;
	}

	if (is_prev_ref && is_ref) {
		FFSWAP(int, _ctx.dst_buf, _ctx.ref_buf);
	} else if (is_prev_ref) {
		FFSWAP(int, _ctx.ref_buf, _ctx.b_ref_buf);
		FFSWAP(int, _ctx.dst_buf, _ctx.ref_buf);
	}
}

bool Indeo4Decoder::is_nonnull_frame() const {
	return _ctx.frame_type < IVI4_FRAMETYPE_NULL_FIRST;
}

int Indeo4Decoder::decode_band_hdr(IVIBandDesc *band) {
	int plane, band_num, indx, transform_id, scan_indx;
	int i;
	int quant_mat;

	plane = _ctx.gb->getBits(2);
	band_num = _ctx.gb->getBits(4);
	if (band->plane != plane || band->band_num != band_num) {
		warning("Invalid band header sequence!");
		return -1;
	}

	band->is_empty = _ctx.gb->getBits1();
	if (!band->is_empty) {
		int old_blk_size = band->blk_size;
		// skip header size
		// If header size is not given, header size is 4 bytes.
		if (_ctx.gb->getBits1())
			_ctx.gb->skipBits(16);

		band->is_halfpel = _ctx.gb->getBits(2);
		if (band->is_halfpel >= 2) {
			warning("Invalid/unsupported mv resolution: %d!",
				band->is_halfpel);
			return -1;
		}
		if (!band->is_halfpel)
			_ctx.uses_fullpel = 1;

		band->checksum_present = _ctx.gb->getBits1();
		if (band->checksum_present)
			band->checksum = _ctx.gb->getBits(16);

		indx = _ctx.gb->getBits(2);
		if (indx == 3) {
			warning("Invalid block size!");
			return -1;
		}
		band->mb_size = 16 >> indx;
		band->blk_size = 8 >> (indx >> 1);

		band->inherit_mv = _ctx.gb->getBits1();
		band->inherit_qdelta = _ctx.gb->getBits1();

		band->glob_quant = _ctx.gb->getBits(5);

		if (!_ctx.gb->getBits1() || _ctx.frame_type == IVI4_FRAMETYPE_INTRA) {
			transform_id = _ctx.gb->getBits(5);
			if (transform_id >= FF_ARRAY_ELEMS(_transforms) ||
				!_transforms[transform_id].inv_trans) {
				warning("Transform %d", transform_id);
				return -3;
			}
			if ((transform_id >= 7 && transform_id <= 9) ||
				transform_id == 17) {
				warning("DCT transform");
				return -3;
			}

			if (transform_id < 10 && band->blk_size < 8) {
				warning("wrong transform size!");
				return -1;
			}
			if ((transform_id >= 0 && transform_id <= 2) || transform_id == 10)
				_ctx.uses_haar = 1;

			band->inv_transform = _transforms[transform_id].inv_trans;
			band->dc_transform = _transforms[transform_id].dc_trans;
			band->is_2d_trans = _transforms[transform_id].is_2d_trans;

			if (transform_id < 10)
				band->transform_size = 8;
			else
				band->transform_size = 4;

			if (band->blk_size != band->transform_size) {
				warning("transform and block size mismatch (%d != %d)", band->transform_size, band->blk_size);
				return -1;
			}

			scan_indx = _ctx.gb->getBits(4);
			if (scan_indx == 15) {
				warning("Custom scan pattern encountered!");
				return -1;
			}
			if (scan_indx > 4 && scan_indx < 10) {
				if (band->blk_size != 4) {
					warning("mismatching scan table!");
					return -1;
				}
			} else if (band->blk_size != 8) {
				warning("mismatching scan table!");
				return -1;
			}

			band->scan = _scan_index_to_tab[scan_indx];
			band->scan_size = band->blk_size;

			quant_mat = _ctx.gb->getBits(5);
			if (quant_mat == 31) {
				warning("Custom quant matrix encountered!");
				return -1;
			}
			if (quant_mat >= FF_ARRAY_ELEMS(_quant_index_to_tab)) {
				warning("Quantization matrix %d", quant_mat);
				return -1;
			}
			band->quant_mat = quant_mat;
		} else {
			if (old_blk_size != band->blk_size) {
				warning("The band block size does not match the configuration inherited");
				return -1;
			}
		}
		if (_quant_index_to_tab[band->quant_mat] > 4 && band->blk_size == 4) {
			warning("Invalid quant matrix for 4x4 block encountered!");
			band->quant_mat = 0;
			return -1;
		}
		if (band->scan_size != band->blk_size) {
			warning("mismatching scan table!");
			return -1;
		}
		if (band->transform_size == 8 && band->blk_size < 8) {
			warning("mismatching transform_size!");
			return -1;
		}

		// decode block huffman codebook
		if (!_ctx.gb->getBits1())
			band->blk_vlc.tab = _ctx.blk_vlc.tab;
		else
			if (band->blk_vlc.ff_ivi_dec_huff_desc(_ctx.gb, 1, IVI_BLK_HUFF))
				return -1;

		// select appropriate rvmap table for this band
		band->rvmap_sel = _ctx.gb->getBits1() ? _ctx.gb->getBits(3) : 8;

		// decode rvmap probability corrections if any
		band->num_corr = 0; // there is no corrections
		if (_ctx.gb->getBits1()) {
			band->num_corr = _ctx.gb->getBits(8); // get number of correction pairs
			if (band->num_corr > 61) {
				warning("Too many corrections: %d",
					band->num_corr);
				return -1;
			}

			// read correction pairs
			for (i = 0; i < band->num_corr * 2; i++)
				band->corr[i] = _ctx.gb->getBits(8);
		}
	}

	if (band->blk_size == 8) {
		band->intra_base = &_ivi4_quant_8x8_intra[_quant_index_to_tab[band->quant_mat]][0];
		band->inter_base = &_ivi4_quant_8x8_inter[_quant_index_to_tab[band->quant_mat]][0];
	} else {
		band->intra_base = &_ivi4_quant_4x4_intra[_quant_index_to_tab[band->quant_mat]][0];
		band->inter_base = &_ivi4_quant_4x4_inter[_quant_index_to_tab[band->quant_mat]][0];
	}

	// Indeo 4 doesn't use scale tables
	band->intra_scale = NULL;
	band->inter_scale = NULL;

	_ctx.gb->alignGetBits();

	if (!band->scan) {
		warning("band->scan not set");
		return -1;
	}

	return 0;
}

int Indeo4Decoder::decode_mb_info(IVIBandDesc *band, IVITile *tile) {
	int         x, y, mv_x, mv_y, mv_delta, offs, mb_offset, blks_per_mb,
		mv_scale, mb_type_bits, s;
	IVIMbInfo   *mb, *ref_mb;
	int         row_offset = band->mb_size * band->pitch;

	mb = tile->mbs;
	ref_mb = tile->ref_mbs;
	offs = tile->ypos * band->pitch + tile->xpos;

	blks_per_mb = band->mb_size != band->blk_size ? 4 : 1;
	mb_type_bits = _ctx.frame_type == IVI4_FRAMETYPE_BIDIR ? 2 : 1;

	/* scale factor for motion vectors */
	mv_scale = (_ctx.planes[0].bands[0].mb_size >> 3) - (band->mb_size >> 3);
	mv_x = mv_y = 0;

	if (((tile->width + band->mb_size - 1) / band->mb_size) * ((tile->height + band->mb_size - 1) / band->mb_size) != tile->num_MBs) {
		warning("num_MBs mismatch %d %d %d %d", tile->width, tile->height, band->mb_size, tile->num_MBs);
		return -1;
	}

	for (y = tile->ypos; y < tile->ypos + tile->height; y += band->mb_size) {
		mb_offset = offs;

		for (x = tile->xpos; x < tile->xpos + tile->width; x += band->mb_size) {
			mb->xpos = x;
			mb->ypos = y;
			mb->buf_offs = mb_offset;
			mb->b_mv_x =
				mb->b_mv_y = 0;

			if (_ctx.gb->getBits1()) {
				if (_ctx.frame_type == IVI4_FRAMETYPE_INTRA) {
					warning("Empty macroblock in an INTRA picture!");
					return -1;
				}
				mb->type = 1; // empty macroblocks are always INTER
				mb->cbp = 0;  // all blocks are empty

				mb->q_delta = 0;
				if (!band->plane && !band->band_num && _ctx.in_q) {
					mb->q_delta = _ctx.gb->getVLC2(_ctx.mb_vlc.tab->table,
						IVI_VLC_BITS, 1);
					mb->q_delta = IVI_TOSIGNED(mb->q_delta);
				}

				mb->mv_x = mb->mv_y = 0; /* no motion vector coded */
				if (band->inherit_mv && ref_mb) {
					/* motion vector inheritance */
					if (mv_scale) {
						mb->mv_x = ivi_scale_mv(ref_mb->mv_x, mv_scale);
						mb->mv_y = ivi_scale_mv(ref_mb->mv_y, mv_scale);
					} else {
						mb->mv_x = ref_mb->mv_x;
						mb->mv_y = ref_mb->mv_y;
					}
				}
			} else {
				if (band->inherit_mv) {
					// copy mb_type from corresponding reference mb
					if (!ref_mb) {
						warning("ref_mb unavailable");
						return -1;
					}
					mb->type = ref_mb->type;
				} else if (_ctx.frame_type == IVI4_FRAMETYPE_INTRA ||
					_ctx.frame_type == IVI4_FRAMETYPE_INTRA1) {
					mb->type = 0; // mb_type is always INTRA for intra-frames
				} else {
					mb->type = _ctx.gb->getBits(mb_type_bits);
				}

				mb->cbp = _ctx.gb->getBits(blks_per_mb);

				mb->q_delta = 0;
				if (band->inherit_qdelta) {
					if (ref_mb) mb->q_delta = ref_mb->q_delta;
				}
				else if (mb->cbp || (!band->plane && !band->band_num &&
					_ctx.in_q)) {
					mb->q_delta = _ctx.gb->getVLC2(_ctx.mb_vlc.tab->table,
						IVI_VLC_BITS, 1);
					mb->q_delta = IVI_TOSIGNED(mb->q_delta);
				}

				if (!mb->type) {
					mb->mv_x = mb->mv_y = 0; // there is no motion vector in intra-macroblocks
				} else {
					if (band->inherit_mv) {
						if (ref_mb)
							// motion vector inheritance
							if (mv_scale) {
								mb->mv_x = ivi_scale_mv(ref_mb->mv_x, mv_scale);
								mb->mv_y = ivi_scale_mv(ref_mb->mv_y, mv_scale);
							}
							else {
								mb->mv_x = ref_mb->mv_x;
								mb->mv_y = ref_mb->mv_y;
							}
					} else {
						// decode motion vector deltas
						mv_delta = _ctx.gb->getVLC2(_ctx.mb_vlc.tab->table,
							IVI_VLC_BITS, 1);
						mv_y += IVI_TOSIGNED(mv_delta);
						mv_delta = _ctx.gb->getVLC2(_ctx.mb_vlc.tab->table,
							IVI_VLC_BITS, 1);
						mv_x += IVI_TOSIGNED(mv_delta);
						mb->mv_x = mv_x;
						mb->mv_y = mv_y;
						if (mb->type == 3) {
							mv_delta = _ctx.gb->getVLC2(
								_ctx.mb_vlc.tab->table,
								IVI_VLC_BITS, 1);
							mv_y += IVI_TOSIGNED(mv_delta);
							mv_delta = _ctx.gb->getVLC2(
								_ctx.mb_vlc.tab->table,
								IVI_VLC_BITS, 1);
							mv_x += IVI_TOSIGNED(mv_delta);
							mb->b_mv_x = -mv_x;
							mb->b_mv_y = -mv_y;
						}
					}
					if (mb->type == 2) {
						mb->b_mv_x = -mb->mv_x;
						mb->b_mv_y = -mb->mv_y;
						mb->mv_x = 0;
						mb->mv_y = 0;
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

int Indeo4Decoder::scaleTileSize(int def_size, int size_factor) {
	return size_factor == 15 ? def_size : (size_factor + 1) << 5;
}

int Indeo4Decoder::decodePlaneSubdivision() {
	int i;

	switch (_ctx.gb->getBits(2)) {
	case 3:
		return 1;
	case 2:
		for (i = 0; i < 4; i++)
			if (_ctx.gb->getBits(2) != 3)
				return 0;
		return 4;
	default:
		return 0;
	}
}

/*------------------------------------------------------------------------*/

/**
 *  Indeo 4 8x8 scan (zigzag) patterns
 */
static const uint8 ivi4_alternate_scan_8x8[64] = {
	0,  8,  1,  9, 16, 24,  2,  3, 17, 25, 10, 11, 32, 40, 48, 56,
	4,  5,  6,  7, 33, 41, 49, 57, 18, 19, 26, 27, 12, 13, 14, 15,
	34, 35, 43, 42, 50, 51, 59, 58, 20, 21, 22, 23, 31, 30, 29, 28,
	36, 37, 38, 39, 47, 46, 45, 44, 52, 53, 54, 55, 63, 62, 61, 60
};

static const uint8 ivi4_alternate_scan_4x4[16] = {
	0, 1, 4, 5, 8, 12, 2, 3, 9, 13, 6, 7, 10, 11, 14, 15
};

static const uint8 ivi4_vertical_scan_4x4[16] = {
	0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15
};

static const uint8 ivi4_horizontal_scan_4x4[16] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

const uint Indeo4Decoder::_ivi4_common_pic_sizes[14] = {
	640, 480, 320, 240, 160, 120, 704, 480, 352, 240, 352, 288, 176, 144
};

Indeo4Decoder::Transform Indeo4Decoder::_transforms[18] = {
	{ IndeoDSP::ff_ivi_inverse_haar_8x8,  IndeoDSP::ff_ivi_dc_haar_2d,       1 },
	{ IndeoDSP::ff_ivi_row_haar8,         IndeoDSP::ff_ivi_dc_haar_2d,       0 },
	{ IndeoDSP::ff_ivi_col_haar8,         IndeoDSP::ff_ivi_dc_haar_2d,       0 },
	{ IndeoDSP::ff_ivi_put_pixels_8x8,    IndeoDSP::ff_ivi_put_dc_pixel_8x8, 1 },
	{ IndeoDSP::ff_ivi_inverse_slant_8x8, IndeoDSP::ff_ivi_dc_slant_2d,      1 },
	{ IndeoDSP::ff_ivi_row_slant8,        IndeoDSP::ff_ivi_dc_row_slant,     1 },
	{ IndeoDSP::ff_ivi_col_slant8,        IndeoDSP::ff_ivi_dc_col_slant,     1 },
	{ NULL, NULL, 0 }, // inverse DCT 8x8
	{ NULL, NULL, 0 }, // inverse DCT 8x1
	{ NULL, NULL, 0 }, // inverse DCT 1x8
	{ IndeoDSP::ff_ivi_inverse_haar_4x4,  IndeoDSP::ff_ivi_dc_haar_2d,       1 },
	{ IndeoDSP::ff_ivi_inverse_slant_4x4, IndeoDSP::ff_ivi_dc_slant_2d,      1 },
	{ NULL, NULL, 0 }, // no transform 4x4
	{ IndeoDSP::ff_ivi_row_haar4,         IndeoDSP::ff_ivi_dc_haar_2d,       0 },
	{ IndeoDSP::ff_ivi_col_haar4,         IndeoDSP::ff_ivi_dc_haar_2d,       0 },
	{ IndeoDSP::ff_ivi_row_slant4,        IndeoDSP::ff_ivi_dc_row_slant,     0 },
	{ IndeoDSP::ff_ivi_col_slant4,        IndeoDSP::ff_ivi_dc_col_slant,     0 },
	{ NULL, NULL, 0 }, // inverse DCT 4x4
};

const uint8 *const Indeo4Decoder::_scan_index_to_tab[15] = {
	// for 8x8 transforms
	ff_zigzag_direct,
	ivi4_alternate_scan_8x8,
	_ff_ivi_horizontal_scan_8x8,
	_ff_ivi_vertical_scan_8x8,
	ff_zigzag_direct,

	// for 4x4 transforms
	_ff_ivi_direct_scan_4x4,
	ivi4_alternate_scan_4x4,
	ivi4_vertical_scan_4x4,
	ivi4_horizontal_scan_4x4,
	_ff_ivi_direct_scan_4x4,

	// TODO: check if those are needed
	_ff_ivi_horizontal_scan_8x8,
	_ff_ivi_horizontal_scan_8x8,
	_ff_ivi_horizontal_scan_8x8,
	_ff_ivi_horizontal_scan_8x8,
	_ff_ivi_horizontal_scan_8x8
};

/**
 *  Indeo 4 dequant tables
 */
const uint16 Indeo4Decoder::_ivi4_quant_8x8_intra[9][64] = {
	{
      43,  342,  385,  470,  555,  555,  598,  726,
     342,  342,  470,  513,  555,  598,  726,  769,
     385,  470,  555,  555,  598,  726,  726,  811,
     470,  470,  555,  555,  598,  726,  769,  854,
     470,  555,  555,  598,  683,  726,  854, 1025,
     555,  555,  598,  683,  726,  854, 1025, 1153,
     555,  555,  598,  726,  811,  982, 1195, 1451,
     555,  598,  726,  811,  982, 1195, 1451, 1793
	},
	{
      86, 1195, 2390, 2390, 4865, 4865, 4865, 4865,
    1195, 1195, 2390, 2390, 4865, 4865, 4865, 4865,
    2390, 2390, 4865, 4865, 6827, 6827, 6827, 6827,
    2390, 2390, 4865, 4865, 6827, 6827, 6827, 6827,
    4865, 4865, 6827, 6827, 6827, 6827, 6827, 6827,
    4865, 4865, 6827, 6827, 6827, 6827, 6827, 6827,
    4865, 4865, 6827, 6827, 6827, 6827, 6827, 6827,
    4865, 4865, 6827, 6827, 6827, 6827, 6827, 6827
	},
	{
     235, 1067, 1195, 1323, 1451, 1579, 1707, 1835,
     235, 1067, 1195, 1323, 1451, 1579, 1707, 1835,
     235, 1067, 1195, 1323, 1451, 1579, 1707, 1835,
     235, 1067, 1195, 1323, 1451, 1579, 1707, 1835,
     235, 1067, 1195, 1323, 1451, 1579, 1707, 1835,
     235, 1067, 1195, 1323, 1451, 1579, 1707, 1835,
     235, 1067, 1195, 1323, 1451, 1579, 1707, 1835,
     235, 1067, 1195, 1323, 1451, 1579, 1707, 1835
	},
	{
    1707, 1707, 3414, 3414, 3414, 3414, 3414, 3414,
    1707, 1707, 3414, 3414, 3414, 3414, 3414, 3414,
    1707, 1707, 3414, 3414, 3414, 3414, 3414, 3414,
    1707, 1707, 3414, 3414, 3414, 3414, 3414, 3414,
    1707, 1707, 3414, 3414, 3414, 3414, 3414, 3414,
    1707, 1707, 3414, 3414, 3414, 3414, 3414, 3414,
    1707, 1707, 3414, 3414, 3414, 3414, 3414, 3414,
    1707, 1707, 3414, 3414, 3414, 3414, 3414, 3414
	},
	{
     897,  897,  897,  897,  897,  897,  897,  897,
    1067, 1067, 1067, 1067, 1067, 1067, 1067, 1067,
    1238, 1238, 1238, 1238, 1238, 1238, 1238, 1238,
    1409, 1409, 1409, 1409, 1409, 1409, 1409, 1409,
    1579, 1579, 1579, 1579, 1579, 1579, 1579, 1579,
    1750, 1750, 1750, 1750, 1750, 1750, 1750, 1750,
    1921, 1921, 1921, 1921, 1921, 1921, 1921, 1921,
    2091, 2091, 2091, 2091, 2091, 2091, 2091, 2091
	},
	{
    1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707,
    1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707,
    3414, 3414, 3414, 3414, 3414, 3414, 3414, 3414,
    3414, 3414, 3414, 3414, 3414, 3414, 3414, 3414,
    3414, 3414, 3414, 3414, 3414, 3414, 3414, 3414,
    3414, 3414, 3414, 3414, 3414, 3414, 3414, 3414,
    3414, 3414, 3414, 3414, 3414, 3414, 3414, 3414,
    3414, 3414, 3414, 3414, 3414, 3414, 3414, 3414
	},
	{
    2390, 2390, 2390, 2390, 2390, 2390, 2390, 2390,
    2390, 2390, 2390, 2390, 2390, 2390, 2390, 2390,
    2390, 2390, 2390, 2390, 2390, 2390, 2390, 2390,
    2390, 2390, 2390, 2390, 2390, 2390, 2390, 2390,
    2390, 2390, 2390, 2390, 2390, 2390, 2390, 2390,
    2390, 2390, 2390, 2390, 2390, 2390, 2390, 2390,
    2390, 2390, 2390, 2390, 2390, 2390, 2390, 2390,
    2390, 2390, 2390, 2390, 2390, 2390, 2390, 2390
	},
	{
      22,  171,  214,  257,  257,  299,  299,  342,
     171,  171,  257,  257,  299,  299,  342,  385,
     214,  257,  257,  299,  299,  342,  342,  385,
     257,  257,  257,  299,  299,  342,  385,  427,
     257,  257,  299,  299,  342,  385,  427,  513,
     257,  299,  299,  342,  385,  427,  513,  598,
     299,  299,  299,  385,  385,  470,  598,  726,
     299,  299,  385,  385,  470,  598,  726,  897
	},
	{
      86,  598, 1195, 1195, 2390, 2390, 2390, 2390,
     598,  598, 1195, 1195, 2390, 2390, 2390, 2390,
    1195, 1195, 2390, 2390, 3414, 3414, 3414, 3414,
    1195, 1195, 2390, 2390, 3414, 3414, 3414, 3414,
    2390, 2390, 3414, 3414, 3414, 3414, 3414, 3414,
    2390, 2390, 3414, 3414, 3414, 3414, 3414, 3414,
    2390, 2390, 3414, 3414, 3414, 3414, 3414, 3414,
    2390, 2390, 3414, 3414, 3414, 3414, 3414, 3414
	}
};

const uint16 Indeo4Decoder::_ivi4_quant_8x8_inter[9][64] = {
	{
     427,  427,  470,  427,  427,  427,  470,  470,
     427,  427,  470,  427,  427,  427,  470,  470,
     470,  470,  470,  470,  470,  470,  470,  470,
     427,  427,  470,  470,  427,  427,  470,  470,
     427,  427,  470,  427,  427,  427,  470,  470,
     427,  427,  470,  427,  427,  427,  470,  470,
     470,  470,  470,  470,  470,  470,  470,  470,
     470,  470,  470,  470,  470,  470,  470,  470
	},
	{
    1707, 1707, 2433, 2433, 3414, 3414, 3414, 3414,
    1707, 1707, 2433, 2433, 3414, 3414, 3414, 3414,
    2433, 2433, 3414, 3414, 4822, 4822, 4822, 4822,
    2433, 2433, 3414, 3414, 4822, 4822, 4822, 4822,
    3414, 3414, 4822, 4822, 3414, 3414, 3414, 3414,
    3414, 3414, 4822, 4822, 3414, 3414, 3414, 3414,
    3414, 3414, 4822, 4822, 3414, 3414, 3414, 3414,
    3414, 3414, 4822, 4822, 3414, 3414, 3414, 3414
	},
	{
    1195, 1195, 1281, 1238, 1195, 1195, 1281, 1281,
    1195, 1195, 1281, 1238, 1195, 1195, 1281, 1281,
    1195, 1195, 1281, 1238, 1195, 1195, 1281, 1281,
    1195, 1195, 1281, 1238, 1195, 1195, 1281, 1281,
    1195, 1195, 1281, 1238, 1195, 1195, 1281, 1281,
    1195, 1195, 1281, 1238, 1195, 1195, 1281, 1281,
    1195, 1195, 1281, 1238, 1195, 1195, 1281, 1281,
    1195, 1195, 1281, 1238, 1195, 1195, 1281, 1281
	},
	{
    2433, 2433, 3414, 3414, 2433, 2433, 2433, 2433,
    2433, 2433, 3414, 3414, 2433, 2433, 2433, 2433,
    2433, 2433, 3414, 3414, 2433, 2433, 2433, 2433,
    2433, 2433, 3414, 3414, 2433, 2433, 2433, 2433,
    2433, 2433, 3414, 3414, 2433, 2433, 2433, 2433,
    2433, 2433, 3414, 3414, 2433, 2433, 2433, 2433,
    2433, 2433, 3414, 3414, 2433, 2433, 2433, 2433,
    2433, 2433, 3414, 3414, 2433, 2433, 2433, 2433
	},
	{
    1195, 1195, 1195, 1195, 1195, 1195, 1195, 1195,
    1195, 1195, 1195, 1195, 1195, 1195, 1195, 1195,
    1281, 1281, 1281, 1281, 1281, 1281, 1281, 1281,
    1238, 1238, 1238, 1238, 1238, 1238, 1238, 1238,
    1195, 1195, 1195, 1195, 1195, 1195, 1195, 1195,
    1195, 1195, 1195, 1195, 1195, 1195, 1195, 1195,
    1281, 1281, 1281, 1281, 1281, 1281, 1281, 1281,
    1281, 1281, 1281, 1281, 1281, 1281, 1281, 1281
	},
	{
    2433, 2433, 2433, 2433, 2433, 2433, 2433, 2433,
    2433, 2433, 2433, 2433, 2433, 2433, 2433, 2433,
    3414, 3414, 3414, 3414, 3414, 3414, 3414, 3414,
    3414, 3414, 3414, 3414, 3414, 3414, 3414, 3414,
    2433, 2433, 2433, 2433, 2433, 2433, 2433, 2433,
    2433, 2433, 2433, 2433, 2433, 2433, 2433, 2433,
    2433, 2433, 2433, 2433, 2433, 2433, 2433, 2433,
    2433, 2433, 2433, 2433, 2433, 2433, 2433, 2433
	},
	{
    1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707,
    1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707,
    1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707,
    1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707,
    1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707,
    1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707,
    1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707,
    1707, 1707, 1707, 1707, 1707, 1707, 1707, 1707
	},
	{
      86,  171,  171,  214,  214,  214,  214,  257,
     171,  171,  214,  214,  214,  214,  257,  257,
     171,  214,  214,  214,  214,  257,  257,  257,
     214,  214,  214,  214,  257,  257,  257,  299,
     214,  214,  214,  257,  257,  257,  299,  299,
     214,  214,  257,  257,  257,  299,  299,  299,
     214,  257,  257,  257,  299,  299,  299,  342,
     257,  257,  257,  299,  299,  299,  342,  342
	},
	{
     854,  854, 1195, 1195, 1707, 1707, 1707, 1707,
     854,  854, 1195, 1195, 1707, 1707, 1707, 1707,
    1195, 1195, 1707, 1707, 2390, 2390, 2390, 2390,
    1195, 1195, 1707, 1707, 2390, 2390, 2390, 2390,
    1707, 1707, 2390, 2390, 1707, 1707, 1707, 1707,
    1707, 1707, 2390, 2390, 1707, 1707, 1707, 1707,
    1707, 1707, 2390, 2390, 1707, 1707, 1707, 1707,
    1707, 1707, 2390, 2390, 1707, 1707, 1707, 1707
	}
};

const uint16 Indeo4Decoder::_ivi4_quant_4x4_intra[5][16] = {
	{
      22,  214,  257,  299,
     214,  257,  299,  342,
     257,  299,  342,  427,
     299,  342,  427,  513
	},
	{
     129, 1025, 1451, 1451,
    1025, 1025, 1451, 1451,
    1451, 1451, 2049, 2049,
    1451, 1451, 2049, 2049
	},
	{
      43,  171,  171,  171,
      43,  171,  171,  171,
      43,  171,  171,  171,
      43,  171,  171,  171
	},
	{
      43,   43,   43,   43,
     171,  171,  171,  171,
     171,  171,  171,  171,
     171,  171,  171,  171
	},
	{
      43,   43,   43,   43,
      43,   43,   43,   43,
      43,   43,   43,   43,
      43,   43,   43,   43
	}
};

const uint16 Indeo4Decoder::_ivi4_quant_4x4_inter[5][16] = {
	{
     107,  214,  257,  299,
     214,  257,  299,  299,
     257,  299,  299,  342,
     299,  299,  342,  342
	},
	{
     513, 1025, 1238, 1238,
    1025, 1025, 1238, 1238,
    1238, 1238, 1451, 1451,
    1238, 1238, 1451, 1451
	},
	{
      43,  171,  171,  171,
      43,  171,  171,  171,
      43,  171,  171,  171,
      43,  171,  171,  171
	},
	{
      43,   43,   43,   43,
     171,  171,  171,  171,
     171,  171,  171,  171,
     171,  171,  171,  171
	},
	{
      43,   43,   43,   43,
      43,   43,   43,   43,
      43,   43,   43,   43,
      43,   43,   43,   43
	}
};

const uint8 Indeo4Decoder::_quant_index_to_tab[22] = {
	0, 1, 0, 2, 1, 3, 0, 4, 1, 5, 0, 1, 6, 7, 8, // for 8x8 quant matrixes
	0, 1, 2, 2, 3, 3, 4                          // for 4x4 quant matrixes
};

} // End of namespace Image
