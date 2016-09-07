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

/* Common structures, macros, and base class shared by both Indeo4 and 
 * Indeo5 decoders, derived from ffmpeg. We don't currently support Indeo5 
 * decoding, but just in case we eventually need it, this is kept as a separate
 * file like it is in ffmpeg.
 *
 * Original copyright note: * Intel Indeo 4 (IV41, IV42, etc.) video decoder for ffmpeg
 * written, produced, and directed by Alan Smithee
 */

#include "image/codecs/indeo/indeo.h"
#include "image/codecs/indeo/indeo_dsp.h"
#include "image/codecs/indeo/mem.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"

namespace Image {
namespace Indeo {
	
/**
 * These are 2x8 predefined Huffman codebooks for coding macroblock/block
 * signals. They are specified using "huffman descriptors" in order to
 * avoid huge static tables. The decoding tables will be generated at
 * startup from these descriptors.
 */

 /**
  * Static macroblock huffman tables
  */
static const IVIHuffDesc ivi_mb_huff_desc[8] = {
    {8,  {0, 4, 5, 4, 4, 4, 6, 6}},
    {12, {0, 2, 2, 3, 3, 3, 3, 5, 3, 2, 2, 2}},
    {12, {0, 2, 3, 4, 3, 3, 3, 3, 4, 3, 2, 2}},
    {12, {0, 3, 4, 4, 3, 3, 3, 3, 3, 2, 2, 2}},
    {13, {0, 4, 4, 3, 3, 3, 3, 2, 3, 3, 2, 1, 1}},
    {9,  {0, 4, 4, 4, 4, 3, 3, 3, 2}},
    {10, {0, 4, 4, 4, 4, 3, 3, 2, 2, 2}},
    {12, {0, 4, 4, 4, 3, 3, 2, 3, 2, 2, 2, 2}}
};

/**
 * static block huffman tables
 */
static const IVIHuffDesc ivi_blk_huff_desc[8] = {
    {10, {1, 2, 3, 4, 4, 7, 5, 5, 4, 1}},
    {11, {2, 3, 4, 4, 4, 7, 5, 4, 3, 3, 2}},
    {12, {2, 4, 5, 5, 5, 5, 6, 4, 4, 3, 1, 1}},
    {13, {3, 3, 4, 4, 5, 6, 6, 4, 4, 3, 2, 1, 1}},
    {11, {3, 4, 4, 5, 5, 5, 6, 5, 4, 2, 2}},
    {13, {3, 4, 5, 5, 5, 5, 6, 4, 3, 3, 2, 1, 1}},
    {13, {3, 4, 5, 5, 5, 6, 5, 4, 3, 3, 2, 1, 1}},
    {9,  {3, 4, 4, 5, 5, 5, 6, 5, 5}}
};

/*------------------------------------------------------------------------*/

/**
 * calculate number of tiles in a stride
 */
#define IVI_NUM_TILES(stride, tile_size) (((stride) + (tile_size) - 1) / (tile_size))


/**
 * calculate number of macroblocks in a tile
 */
#define IVI_MBs_PER_TILE(tile_width, tile_height, mb_size) \
    ((((tile_width) + (mb_size) - 1) / (mb_size)) * (((tile_height) + (mb_size) - 1) / (mb_size)))

/*------------------------------------------------------------------------*/

int IVIHuffDesc::ivi_create_huff_from_desc(VLC *vlc, int flag) const {
    int         pos, i, j, codes_per_row, prefix, not_last_row;
    uint16      codewords[256];
    uint8       bits[256];

    pos = 0; // current position = 0

    for (i = 0; i < num_rows; i++) {
        codes_per_row = 1 << xbits[i];
        not_last_row  = (i != num_rows - 1);
        prefix        = ((1 << i) - 1) << (xbits[i] + not_last_row);

        for (j = 0; j < codes_per_row; j++) {
            if (pos >= 256) // Some Indeo5 codebooks can have more than 256
                break;      // elements, but only 256 codes are allowed!

            bits[pos] = i + xbits[i] + not_last_row;
            if (bits[pos] > IVI_VLC_BITS)
                return -1; // invalid descriptor

            codewords[pos] = inv_bits((prefix | j), bits[pos]);
            if (!bits[pos])
                bits[pos] = 1;

            pos++;
        }//for j
    }//for i

    // number of codewords = pos
	return vlc->init_vlc(IVI_VLC_BITS, pos, bits, 1, 1, codewords, 2, 2,
                    (flag ? INIT_VLC_USE_NEW_STATIC : 0) | INIT_VLC_LE);
}

/*------------------------------------------------------------------------*/

bool IVIHuffDesc::ivi_huff_desc_cmp(const IVIHuffDesc *desc2) const {
	return num_rows != desc2->num_rows ||
		memcmp(xbits, desc2->xbits, num_rows);
}

void IVIHuffDesc::ivi_huff_desc_copy(const IVIHuffDesc *src) {
	num_rows = src->num_rows;
	memcpy(xbits, src->xbits, src->num_rows);
}

/*------------------------------------------------------------------------*/

IVIHuffTab::IVIHuffTab() {
	tab = nullptr;

	for (int i = 0; i < 8; i++) {
		ivi_mb_vlc_tabs[i].table = table_data + i * 2 * 8192;
		ivi_mb_vlc_tabs[i].table_allocated = 8192;
		ivi_mb_huff_desc[i].ivi_create_huff_from_desc(&ivi_mb_vlc_tabs[i], 1);
		ivi_blk_vlc_tabs[i].table = table_data + (i * 2 + 1) * 8192;
		ivi_blk_vlc_tabs[i].table_allocated = 8192;
		ivi_blk_huff_desc[i].ivi_create_huff_from_desc(&ivi_blk_vlc_tabs[i], 1);
	}
}

int IVIHuffTab::ff_ivi_dec_huff_desc(GetBits *gb, int desc_coded, int which_tab) {
	int i, result;
	IVIHuffDesc new_huff;

	if (!desc_coded) {
		// select default table
		tab = (which_tab) ? &ivi_blk_vlc_tabs[7]
			: &ivi_mb_vlc_tabs[7];
		return 0;
	}

	tab_sel = gb->getBits(3);
	if (tab_sel == 7) {
		// custom huffman table (explicitly encoded)
		new_huff.num_rows = gb->getBits(4);
		if (!new_huff.num_rows) {
			warning("Empty custom Huffman table!");
			return -1;
		}

		for (i = 0; i < new_huff.num_rows; i++)
			new_huff.xbits[i] = gb->getBits(4);

		// Have we got the same custom table? Rebuild if not.
		if (new_huff.ivi_huff_desc_cmp(&cust_desc) || !cust_tab.table) {
			cust_desc.ivi_huff_desc_copy(&new_huff);

			if (cust_tab.table)
				cust_tab.ff_free_vlc();
			result = cust_desc.ivi_create_huff_from_desc(&cust_tab, 0);
			if (result) {
				// reset faulty description
				cust_desc.num_rows = 0;
				warning("Error while initializing custom vlc table!");
				return result;
			}
		}
		tab = &cust_tab;
	} else {
		// select one of predefined tables
		tab = (which_tab) ? &ivi_blk_vlc_tabs[tab_sel]
			: &ivi_mb_vlc_tabs[tab_sel];
	}

	return 0;
}

/*------------------------------------------------------------------------*/

bool IVIPicConfig::ivi_pic_config_cmp(const IVIPicConfig &cfg2) {
	return pic_width != cfg2.pic_width || pic_height != cfg2.pic_height ||
		chroma_width != cfg2.chroma_width || chroma_height != cfg2.chroma_height ||
		tile_width != cfg2.tile_width || tile_height != cfg2.tile_height ||
		luma_bands != cfg2.luma_bands || chroma_bands != cfg2.chroma_bands;
}

/*------------------------------------------------------------------------*/

int IVIPlaneDesc::ff_ivi_init_planes(IVIPlaneDesc *planes, const IVIPicConfig *cfg, bool is_indeo4) {
	int p, b;
	uint32 b_width, b_height, align_fac, width_aligned,
		height_aligned, buf_size;
	IVIBandDesc *band;

	ivi_free_buffers(planes);

	if (av_image_check_size(cfg->pic_width, cfg->pic_height, 0, NULL) < 0 ||
		cfg->luma_bands < 1 || cfg->chroma_bands < 1)
		return -1;

	// fill in the descriptor of the luminance plane
	planes[0].width = cfg->pic_width;
	planes[0].height = cfg->pic_height;
	planes[0].num_bands = cfg->luma_bands;

	// fill in the descriptors of the chrominance planes
	planes[1].width = planes[2].width = (cfg->pic_width + 3) >> 2;
	planes[1].height = planes[2].height = (cfg->pic_height + 3) >> 2;
	planes[1].num_bands = planes[2].num_bands = cfg->chroma_bands;

	for (p = 0; p < 3; p++) {
		planes[p].bands = (IVIBandDesc *)av_mallocz_array(planes[p].num_bands, sizeof(IVIBandDesc));
		if (!planes[p].bands)
			return -2;

		// select band dimensions: if there is only one band then it
		// has the full size, if there are several bands each of them
		// has only half size
		b_width = planes[p].num_bands == 1 ? planes[p].width
			: (planes[p].width + 1) >> 1;
		b_height = planes[p].num_bands == 1 ? planes[p].height
			: (planes[p].height + 1) >> 1;

		// luma   band buffers will be aligned on 16x16 (max macroblock size)
		// chroma band buffers will be aligned on   8x8 (max macroblock size)
		align_fac = p ? 8 : 16;
		width_aligned = FFALIGN(b_width, align_fac);
		height_aligned = FFALIGN(b_height, align_fac);
		buf_size = width_aligned * height_aligned * sizeof(int16);

		for (b = 0; b < planes[p].num_bands; b++) {
			band = &planes[p].bands[b]; // select appropriate plane/band
			band->plane = p;
			band->band_num = b;
			band->width = b_width;
			band->height = b_height;
			band->pitch = width_aligned;
			band->aheight = height_aligned;
			band->bufs[0] = (int16 *)av_mallocz(buf_size);
			band->bufs[1] = (int16 *)av_mallocz(buf_size);
			band->bufsize = buf_size / 2;
			if (!band->bufs[0] || !band->bufs[1])
				return -2;

			// allocate the 3rd band buffer for scalability mode
			if (cfg->luma_bands > 1) {
				band->bufs[2] = (int16 *)av_mallocz(buf_size);
				if (!band->bufs[2])
					return -2;
			}
			if (is_indeo4) {
				band->bufs[3] = (int16 *)av_mallocz(buf_size);
				if (!band->bufs[3])
					return -2;
			}
			// reset custom vlc
			planes[p].bands[0].blk_vlc.cust_desc.num_rows = 0;
		}
	}

	return 0;
}

int IVIPlaneDesc::ff_ivi_init_tiles(IVIPlaneDesc *planes,
		int tile_width, int tile_height) {
	int p, b, x_tiles, y_tiles, t_width, t_height, ret;
	IVIBandDesc *band;

	for (p = 0; p < 3; p++) {
		t_width = !p ? tile_width : (tile_width + 3) >> 2;
		t_height = !p ? tile_height : (tile_height + 3) >> 2;

		if (!p && planes[0].num_bands == 4) {
			t_width >>= 1;
			t_height >>= 1;
		}
		if (t_width <= 0 || t_height <= 0)
			return -3;

		for (b = 0; b < planes[p].num_bands; b++) {
			band = &planes[p].bands[b];
			x_tiles = IVI_NUM_TILES(band->width, t_width);
			y_tiles = IVI_NUM_TILES(band->height, t_height);
			band->num_tiles = x_tiles * y_tiles;

			av_freep(&band->tiles);
			band->tiles = (IVITile *)av_mallocz_array(band->num_tiles, sizeof(IVITile));
			if (!band->tiles)
				return -2;

			// use the first luma band as reference for motion vectors
			// and quant
			ret = band->ivi_init_tiles(planes[0].bands[0].tiles,
				p, b, t_height, t_width);
			if (ret < 0)
				return ret;
		}
	}

	return 0;
}

void IVIPlaneDesc::ivi_free_buffers(IVIPlaneDesc *planes) {
	int p, b, t;

	for (p = 0; p < 3; p++) {
		if (planes[p].bands)
			for (b = 0; b < planes[p].num_bands; b++) {
				av_freep(&planes[p].bands[b].bufs[0]);
				av_freep(&planes[p].bands[b].bufs[1]);
				av_freep(&planes[p].bands[b].bufs[2]);
				av_freep(&planes[p].bands[b].bufs[3]);

				if (planes[p].bands[b].blk_vlc.cust_tab.table)
					planes[p].bands[b].blk_vlc.cust_tab.ff_free_vlc();
				for (t = 0; t < planes[p].bands[b].num_tiles; t++)
					av_freep(&planes[p].bands[b].tiles[t].mbs);
				av_freep(&planes[p].bands[b].tiles);
			}
		av_freep(&planes[p].bands);
		planes[p].num_bands = 0;
	}
}

/*------------------------------------------------------------------------*/

int IVIBandDesc::ivi_init_tiles(IVITile *ref_tile, int p, int b, int t_height, int t_width) {
	int x, y;
	IVITile *tile = tiles;

	for (y = 0; y < height; y += t_height) {
		for (x = 0; x < width; x += t_width) {
			tile->xpos = x;
			tile->ypos = y;
			tile->mb_size = mb_size;
			tile->width = MIN(width - x, t_width);
			tile->height = MIN(height - y, t_height);
			tile->is_empty = tile->data_size = 0;
			// calculate number of macroblocks
			tile->num_MBs = IVI_MBs_PER_TILE(tile->width, tile->height,
				mb_size);

			av_freep(&tile->mbs);
			tile->mbs = (IVIMbInfo *)av_mallocz_array(tile->num_MBs, sizeof(IVIMbInfo));
			if (!tile->mbs)
				return -2;

			tile->ref_mbs = 0;
			if (p || b) {
				if (tile->num_MBs != ref_tile->num_MBs) {
					warning("ref_tile mismatch");
					return -1;
				}
				tile->ref_mbs = ref_tile->mbs;
				ref_tile++;
			}
			tile++;
		}
	}

	return 0;
}

/*------------------------------------------------------------------------*/

IndeoDecoderBase::IndeoDecoderBase(uint16 width, uint16 height) : Codec() {
	_pixelFormat = g_system->getScreenFormat();
	_surface = new Graphics::ManagedSurface();
	_surface->create(width, height, _pixelFormat);
	_ctx.gb = nullptr;
	_ctx.pic_conf.pic_width = _ctx.pic_conf.pic_height = 0;
	_ctx.b_ref_buf = 3; // buffer 2 is used for scalability mode
}

IndeoDecoderBase::~IndeoDecoderBase() {
	delete _surface;
	IVIPlaneDesc::ivi_free_buffers(_ctx.planes);
	if (_ctx.mb_vlc.cust_tab.table)
		_ctx.mb_vlc.cust_tab.ff_free_vlc();

	delete _ctx.p_frame;
}

int IndeoDecoderBase::decodeIndeoFrame() {
	int result, p, b;
	AVFrame frameData;
	AVFrame *frame = &frameData;

	// Decode the header
	if (decodePictureHeader() < 0)
		return -1;

	if (_ctx.gop_invalid)
		return -1;

	if (_ctx.frame_type == IVI4_FRAMETYPE_NULL_LAST) {
		// Returning the previous frame, so exit wth success
		return 0;
	}

	if (_ctx.gop_flags & IVI5_IS_PROTECTED) {
		warning("Password-protected clip");
		return -1;
	}

	if (!_ctx.planes[0].bands) {
		warning("Color planes not initialized yet");
		return -1;
	}

	switch_buffers();

	//{ START_TIMER;

	if (is_nonnull_frame()) {
		_ctx.buf_invalid[_ctx.dst_buf] = 1;
		for (p = 0; p < 3; p++) {
			for (b = 0; b < _ctx.planes[p].num_bands; b++) {
				result = decode_band(&_ctx.planes[p].bands[b]);
				if (result < 0) {
					warning("Error while decoding band: %d, plane: %d", b, p);
					return result;
				}
			}
		}
		_ctx.buf_invalid[_ctx.dst_buf] = 0;
	} else {
		if (_ctx.is_scalable)
			return -1;

		for (p = 0; p < 3; p++) {
			if (!_ctx.planes[p].bands[0].buf)
				return -1;
		}
	}
	if (_ctx.buf_invalid[_ctx.dst_buf])
		return -1;

	//STOP_TIMER("decode_planes"); }

	if (!is_nonnull_frame())
		return 0;

	result = ff_set_dimensions(_ctx.planes[0].width, _ctx.planes[0].height);
	if (result < 0)
		return result;

	if ((result = ff_get_buffer(frame, 0)) < 0)
		return result;

	if (_ctx.is_scalable) {
		if (_ctx.is_indeo4)
			ff_ivi_recompose_haar(&_ctx.planes[0], frame->data[0], frame->linesize[0]);
		else
			ff_ivi_recompose53(&_ctx.planes[0], frame->data[0], frame->linesize[0]);
	} else {
		ivi_output_plane(&_ctx.planes[0], frame->data[0], frame->linesize[0]);
	}

	ivi_output_plane(&_ctx.planes[2], frame->data[1], frame->linesize[1]);
	ivi_output_plane(&_ctx.planes[1], frame->data[2], frame->linesize[2]);

	// If the bidirectional mode is enabled, next I and the following P
	// frame will be sent together. Unfortunately the approach below seems
	// to be the only way to handle the B-frames mode.
	// That's exactly the same Intel decoders do.
	if (_ctx.is_indeo4 && _ctx.frame_type == IVI4_FRAMETYPE_INTRA) {
		int left;

		// skip version string
		while (_ctx.gb->getBits(8)) {
			if (_ctx.gb->getBitsLeft() < 8)
				return -1;
		}
		left = _ctx.gb->getBitsCount() & 0x18;
		_ctx.gb->skipBitsLong(64 - left);
		if (_ctx.gb->getBitsLeft() > 18 &&
			_ctx.gb->showBitsLong(21) == 0xBFFF8) { // syncheader + inter type
			error("Indeo decoder: Mode not currently implemented in ScummVM");
		}
	}

	return 0;
}

int IndeoDecoderBase::decode_band(IVIBandDesc *band) {
	int         result, i, t, idx1, idx2, pos;
	IVITile *	tile;

	band->buf = band->bufs[_ctx.dst_buf];
	if (!band->buf) {
		warning("Band buffer points to no data!");
		return -1;
	}
	if (_ctx.is_indeo4 && _ctx.frame_type == IVI4_FRAMETYPE_BIDIR) {
		band->ref_buf = band->bufs[_ctx.b_ref_buf];
		band->b_ref_buf = band->bufs[_ctx.ref_buf];
	} else {
		band->ref_buf = band->bufs[_ctx.ref_buf];
		band->b_ref_buf = 0;
	}
	band->data_ptr = _ctx.frame_data + (_ctx.gb->getBitsCount() >> 3);

	result = decode_band_hdr(band);
	if (result) {
		warning("Error while decoding band header: %d",
			result);
		return result;
	}

	if (band->is_empty) {
		warning("Empty band encountered!");
		return -1;
	}

	band->rv_map = &_ctx.rvmap_tabs[band->rvmap_sel];

	// apply corrections to the selected rvmap table if present
	for (i = 0; i < band->num_corr; i++) {
		idx1 = band->corr[i * 2];
		idx2 = band->corr[i * 2 + 1];
		FFSWAP(uint8, band->rv_map->runtab[idx1], band->rv_map->runtab[idx2]);
		FFSWAP(int16, band->rv_map->valtab[idx1], band->rv_map->valtab[idx2]);
		if (idx1 == band->rv_map->eob_sym || idx2 == band->rv_map->eob_sym)
			band->rv_map->eob_sym ^= idx1 ^ idx2;
		if (idx1 == band->rv_map->esc_sym || idx2 == band->rv_map->esc_sym)
			band->rv_map->esc_sym ^= idx1 ^ idx2;
	}

	pos = _ctx.gb->getBitsCount();

	for (t = 0; t < band->num_tiles; t++) {
		tile = &band->tiles[t];

		if (tile->mb_size != band->mb_size) {
			warning("MB sizes mismatch: %d vs. %d",
				band->mb_size, tile->mb_size);
			return -1;
		}
		tile->is_empty = _ctx.gb->getBits1();
		if (tile->is_empty) {
			result = ivi_process_empty_tile(band, tile,
				(_ctx.planes[0].bands[0].mb_size >> 3) - (band->mb_size >> 3));
			if (result < 0)
				break;
			warning("Empty tile encountered!");
		} else {
			tile->data_size = ivi_dec_tile_data_size(_ctx.gb);
			if (!tile->data_size) {
				warning("Tile data size is zero!");
				result = -1;
				break;
			}

			result = decode_mb_info(band, tile);
			if (result < 0)
				break;

			result = ivi_decode_blocks(_ctx.gb, band, tile);
			if (result < 0) {
				warning("Corrupted tile data encountered!");
				break;
			}

			if ((((int)_ctx.gb->getBitsCount() - pos) >> 3) != tile->data_size) {
				warning("Tile data_size mismatch!");
				result = -1;
				break;
			}

			pos += tile->data_size << 3; // skip to next tile
		}
	}

	// restore the selected rvmap table by applying its corrections in
	// reverse order
	for (i = band->num_corr - 1; i >= 0; i--) {
		idx1 = band->corr[i * 2];
		idx2 = band->corr[i * 2 + 1];
		FFSWAP(uint8, band->rv_map->runtab[idx1], band->rv_map->runtab[idx2]);
		FFSWAP(int16, band->rv_map->valtab[idx1], band->rv_map->valtab[idx2]);
		if (idx1 == band->rv_map->eob_sym || idx2 == band->rv_map->eob_sym)
			band->rv_map->eob_sym ^= idx1 ^ idx2;
		if (idx1 == band->rv_map->esc_sym || idx2 == band->rv_map->esc_sym)
			band->rv_map->esc_sym ^= idx1 ^ idx2;
	}

	_ctx.gb->alignGetBits();

	return result;
}

int IndeoDecoderBase::ff_set_dimensions(uint16 width, uint16 height) {
	if (_surface->w != width || _surface->h != height)
		_surface->create(width, height);

	return 0;
}

int IndeoDecoderBase::ff_get_buffer(AVFrame *frame, int flags) {
	frame->data[0] = (uint8 *)_surface->getBasePtr(0, 0);
	return 0;
}

void IndeoDecoderBase::ff_ivi_recompose_haar(const IVIPlaneDesc *plane,
		uint8 *dst, const int dst_pitch) {
	int           x, y, indx, b0, b1, b2, b3, p0, p1, p2, p3;
	const short * b0_ptr, *b1_ptr, *b2_ptr, *b3_ptr;
	int32         pitch;

	// all bands should have the same pitch
	pitch = plane->bands[0].pitch;

	// get pointers to the wavelet bands
	b0_ptr = plane->bands[0].buf;
	b1_ptr = plane->bands[1].buf;
	b2_ptr = plane->bands[2].buf;
	b3_ptr = plane->bands[3].buf;

	for (y = 0; y < plane->height; y += 2) {
		for (x = 0, indx = 0; x < plane->width; x += 2, indx++) {
			// load coefficients
			b0 = b0_ptr[indx]; //should be: b0 = (num_bands > 0) ? b0_ptr[indx] : 0;
			b1 = b1_ptr[indx]; //should be: b1 = (num_bands > 1) ? b1_ptr[indx] : 0;
			b2 = b2_ptr[indx]; //should be: b2 = (num_bands > 2) ? b2_ptr[indx] : 0;
			b3 = b3_ptr[indx]; //should be: b3 = (num_bands > 3) ? b3_ptr[indx] : 0;

							   // haar wavelet recomposition
			p0 = (b0 + b1 + b2 + b3 + 2) >> 2;
			p1 = (b0 + b1 - b2 - b3 + 2) >> 2;
			p2 = (b0 - b1 + b2 - b3 + 2) >> 2;
			p3 = (b0 - b1 - b2 + b3 + 2) >> 2;

			// bias, convert and output four pixels
			dst[x] = av_clip_uint8(p0 + 128);
			dst[x + 1] = av_clip_uint8(p1 + 128);
			dst[dst_pitch + x] = av_clip_uint8(p2 + 128);
			dst[dst_pitch + x + 1] = av_clip_uint8(p3 + 128);
		}// for x

		dst += dst_pitch << 1;

		b0_ptr += pitch;
		b1_ptr += pitch;
		b2_ptr += pitch;
		b3_ptr += pitch;
	}// for y
}


void IndeoDecoderBase::ff_ivi_recompose53(const IVIPlaneDesc *plane,
		uint8 *dst, const int dst_pitch) {
	int           x, y, indx;
	int32         p0, p1, p2, p3, tmp0, tmp1, tmp2;
	int32         b0_1, b0_2, b1_1, b1_2, b1_3, b2_1, b2_2, b2_3, b2_4, b2_5, b2_6;
	int32         b3_1, b3_2, b3_3, b3_4, b3_5, b3_6, b3_7, b3_8, b3_9;
	int32         pitch, back_pitch;
	const short * b0_ptr, *b1_ptr, *b2_ptr, *b3_ptr;
	const int     num_bands = 4;

	// all bands should have the same pitch
	pitch = plane->bands[0].pitch;

	// pixels at the position "y-1" will be set to pixels at the "y" for the 1st iteration
	back_pitch = 0;

	// get pointers to the wavelet bands
	b0_ptr = plane->bands[0].buf;
	b1_ptr = plane->bands[1].buf;
	b2_ptr = plane->bands[2].buf;
	b3_ptr = plane->bands[3].buf;

	for (y = 0; y < plane->height; y += 2) {

		if (y + 2 >= plane->height)
			pitch = 0;
		// load storage variables with values
		if (num_bands > 0) {
			b0_1 = b0_ptr[0];
			b0_2 = b0_ptr[pitch];
		}

		if (num_bands > 1) {
			b1_1 = b1_ptr[back_pitch];
			b1_2 = b1_ptr[0];
			b1_3 = b1_1 - b1_2 * 6 + b1_ptr[pitch];
		}

		if (num_bands > 2) {
			b2_2 = b2_ptr[0];     // b2[x,  y  ]
			b2_3 = b2_2;          // b2[x+1,y  ] = b2[x,y]
			b2_5 = b2_ptr[pitch]; // b2[x  ,y+1]
			b2_6 = b2_5;          // b2[x+1,y+1] = b2[x,y+1]
		}

		if (num_bands > 3) {
			b3_2 = b3_ptr[back_pitch]; // b3[x  ,y-1]
			b3_3 = b3_2;               // b3[x+1,y-1] = b3[x  ,y-1]
			b3_5 = b3_ptr[0];          // b3[x  ,y  ]
			b3_6 = b3_5;               // b3[x+1,y  ] = b3[x  ,y  ]
			b3_8 = b3_2 - b3_5 * 6 + b3_ptr[pitch];
			b3_9 = b3_8;
		}

		for (x = 0, indx = 0; x < plane->width; x += 2, indx++) {
			if (x + 2 >= plane->width) {
				b0_ptr--;
				b1_ptr--;
				b2_ptr--;
				b3_ptr--;
			}

			// some values calculated in the previous iterations can
			// be reused in the next ones, so do appropriate copying
			b2_1 = b2_2; // b2[x-1,y  ] = b2[x,  y  ]
			b2_2 = b2_3; // b2[x  ,y  ] = b2[x+1,y  ]
			b2_4 = b2_5; // b2[x-1,y+1] = b2[x  ,y+1]
			b2_5 = b2_6; // b2[x  ,y+1] = b2[x+1,y+1]
			b3_1 = b3_2; // b3[x-1,y-1] = b3[x  ,y-1]
			b3_2 = b3_3; // b3[x  ,y-1] = b3[x+1,y-1]
			b3_4 = b3_5; // b3[x-1,y  ] = b3[x  ,y  ]
			b3_5 = b3_6; // b3[x  ,y  ] = b3[x+1,y  ]
			b3_7 = b3_8; // vert_HPF(x-1)
			b3_8 = b3_9; // vert_HPF(x  )

			p0 = p1 = p2 = p3 = 0;

			// process the LL-band by applying LPF both vertically and horizontally
			if (num_bands > 0) {
				tmp0 = b0_1;
				tmp2 = b0_2;
				b0_1 = b0_ptr[indx + 1];
				b0_2 = b0_ptr[pitch + indx + 1];
				tmp1 = tmp0 + b0_1;

				p0 = tmp0 << 4;
				p1 = tmp1 << 3;
				p2 = (tmp0 + tmp2) << 3;
				p3 = (tmp1 + tmp2 + b0_2) << 2;
			}

			// process the HL-band by applying HPF vertically and LPF horizontally
			if (num_bands > 1) {
				tmp0 = b1_2;
				tmp1 = b1_1;
				b1_2 = b1_ptr[indx + 1];
				b1_1 = b1_ptr[back_pitch + indx + 1];

				tmp2 = tmp1 - tmp0 * 6 + b1_3;
				b1_3 = b1_1 - b1_2 * 6 + b1_ptr[pitch + indx + 1];

				p0 += (tmp0 + tmp1) << 3;
				p1 += (tmp0 + tmp1 + b1_1 + b1_2) << 2;
				p2 += tmp2 << 2;
				p3 += (tmp2 + b1_3) << 1;
			}

			// process the LH-band by applying LPF vertically and HPF horizontally
			if (num_bands > 2) {
				b2_3 = b2_ptr[indx + 1];
				b2_6 = b2_ptr[pitch + indx + 1];

				tmp0 = b2_1 + b2_2;
				tmp1 = b2_1 - b2_2 * 6 + b2_3;

				p0 += tmp0 << 3;
				p1 += tmp1 << 2;
				p2 += (tmp0 + b2_4 + b2_5) << 2;
				p3 += (tmp1 + b2_4 - b2_5 * 6 + b2_6) << 1;
			}

			// process the HH-band by applying HPF both vertically and horizontally
			if (num_bands > 3) {
				b3_6 = b3_ptr[indx + 1];            // b3[x+1,y  ]
				b3_3 = b3_ptr[back_pitch + indx + 1]; // b3[x+1,y-1]

				tmp0 = b3_1 + b3_4;
				tmp1 = b3_2 + b3_5;
				tmp2 = b3_3 + b3_6;

				b3_9 = b3_3 - b3_6 * 6 + b3_ptr[pitch + indx + 1];

				p0 += (tmp0 + tmp1) << 2;
				p1 += (tmp0 - tmp1 * 6 + tmp2) << 1;
				p2 += (b3_7 + b3_8) << 1;
				p3 += b3_7 - b3_8 * 6 + b3_9;
			}

			// output four pixels
			dst[x] = av_clip_uint8((p0 >> 6) + 128);
			dst[x + 1] = av_clip_uint8((p1 >> 6) + 128);
			dst[dst_pitch + x] = av_clip_uint8((p2 >> 6) + 128);
			dst[dst_pitch + x + 1] = av_clip_uint8((p3 >> 6) + 128);
		}// for x

		dst += dst_pitch << 1;

		back_pitch = -pitch;

		b0_ptr += pitch + 1;
		b1_ptr += pitch + 1;
		b2_ptr += pitch + 1;
		b3_ptr += pitch + 1;
	}
}

void IndeoDecoderBase::ivi_output_plane(IVIPlaneDesc *plane, uint8 *dst, int dst_pitch) {
	int           x, y;
	const int16 * src = plane->bands[0].buf;
	uint32        pitch = plane->bands[0].pitch;

	if (!src)
		return;

	for (y = 0; y < plane->height; y++) {
		for (x = 0; x < plane->width; x++)
			dst[x] = av_clip_uint8(src[x] + 128);
		src += pitch;
		dst += dst_pitch;
	}
}

int IndeoDecoderBase::ivi_process_empty_tile(IVIBandDesc *band,
			IVITile *tile, int32 mv_scale) {
	int             x, y, need_mc, mbn, blk, num_blocks, mv_x, mv_y, mc_type;
	int             offs, mb_offset, row_offset, ret;
	IVIMbInfo       *mb, *ref_mb;
	const int16     *src;
	int16           *dst;
	ivi_mc_func     mc_no_delta_func;

	if (tile->num_MBs != IVI_MBs_PER_TILE(tile->width, tile->height, band->mb_size)) {
		warning("Allocated tile size %d mismatches "
			"parameters %d in ivi_process_empty_tile()",
			tile->num_MBs, IVI_MBs_PER_TILE(tile->width, tile->height, band->mb_size));
		return -1;
	}

	offs = tile->ypos * band->pitch + tile->xpos;
	mb = tile->mbs;
	ref_mb = tile->ref_mbs;
	row_offset = band->mb_size * band->pitch;
	need_mc = 0; // reset the mc tracking flag

	for (y = tile->ypos; y < (tile->ypos + tile->height); y += band->mb_size) {
		mb_offset = offs;

		for (x = tile->xpos; x < (tile->xpos + tile->width); x += band->mb_size) {
			mb->xpos = x;
			mb->ypos = y;
			mb->buf_offs = mb_offset;

			mb->type = 1; // set the macroblocks type = INTER
			mb->cbp = 0; // all blocks are empty

			if (!band->qdelta_present && !band->plane && !band->band_num) {
				mb->q_delta = band->glob_quant;
				mb->mv_x = 0;
				mb->mv_y = 0;
			}

			if (band->inherit_qdelta && ref_mb)
				mb->q_delta = ref_mb->q_delta;

			if (band->inherit_mv && ref_mb) {
				// motion vector inheritance
				if (mv_scale) {
					mb->mv_x = ivi_scale_mv(ref_mb->mv_x, mv_scale);
					mb->mv_y = ivi_scale_mv(ref_mb->mv_y, mv_scale);
				} else {
					mb->mv_x = ref_mb->mv_x;
					mb->mv_y = ref_mb->mv_y;
				}
				need_mc |= mb->mv_x || mb->mv_y; // tracking non-zero motion vectors
				{
					int dmv_x, dmv_y, cx, cy;

					dmv_x = mb->mv_x >> band->is_halfpel;
					dmv_y = mb->mv_y >> band->is_halfpel;
					cx = mb->mv_x &  band->is_halfpel;
					cy = mb->mv_y &  band->is_halfpel;

					if (mb->xpos + dmv_x < 0
						|| mb->xpos + dmv_x + band->mb_size + cx > band->pitch
						|| mb->ypos + dmv_y < 0
						|| mb->ypos + dmv_y + band->mb_size + cy > band->aheight) {
						warning("MV out of bounds");
						return -1;
					}
				}
			}

			mb++;
			if (ref_mb)
				ref_mb++;
			mb_offset += band->mb_size;
		} // for x
		offs += row_offset;
	} // for y

	if (band->inherit_mv && need_mc) { // apply motion compensation if there is at least one non-zero motion vector
		num_blocks = (band->mb_size != band->blk_size) ? 4 : 1; // number of blocks per mb
		mc_no_delta_func = (band->blk_size == 8) ? IndeoDSP::ff_ivi_mc_8x8_no_delta
			: IndeoDSP::ff_ivi_mc_4x4_no_delta;

		for (mbn = 0, mb = tile->mbs; mbn < tile->num_MBs; mb++, mbn++) {
			mv_x = mb->mv_x;
			mv_y = mb->mv_y;
			if (!band->is_halfpel) {
				mc_type = 0; // we have only fullpel vectors
			} else {
				mc_type = ((mv_y & 1) << 1) | (mv_x & 1);
				mv_x >>= 1;
				mv_y >>= 1; // convert halfpel vectors into fullpel ones
			}

			for (blk = 0; blk < num_blocks; blk++) {
				// adjust block position in the buffer according with its number
				offs = mb->buf_offs + band->blk_size * ((blk & 1) + !!(blk & 2) * band->pitch);
				ret = ivi_mc(band, mc_no_delta_func, nullptr, offs,
					mv_x, mv_y, 0, 0, mc_type, -1);
				if (ret < 0)
					return ret;
			}
		}
	} else {
		// copy data from the reference tile into the current one
		src = band->ref_buf + tile->ypos * band->pitch + tile->xpos;
		dst = band->buf + tile->ypos * band->pitch + tile->xpos;
		for (y = 0; y < tile->height; y++) {
			memcpy(dst, src, tile->width*sizeof(band->buf[0]));
			src += band->pitch;
			dst += band->pitch;
		}
	}

	return 0;
}

int IndeoDecoderBase::ivi_dec_tile_data_size(GetBits *gb) {
	int len = 0;

	if (gb->getBits1()) {
		len = gb->getBits(8);
		if (len == 255)
			len = gb->getBitsLong(24);
	}

	// align the bitstream reader on the byte boundary
	gb->alignGetBits();

	return len;
}

int IndeoDecoderBase::ivi_decode_blocks(GetBits *gb, IVIBandDesc *band, IVITile *tile) {
   int mbn, blk, num_blocks, blk_size, ret, is_intra;
    int mc_type = 0, mc_type2 = -1;
    int mv_x = 0, mv_y = 0, mv_x2 = 0, mv_y2 = 0;
    int32 prev_dc;
    uint32 cbp, quant, buf_offs;
    IVIMbInfo *mb;
    ivi_mc_func mc_with_delta_func, mc_no_delta_func;
    ivi_mc_avg_func mc_avg_with_delta_func, mc_avg_no_delta_func;
    const uint8 *scale_tab;

    // init intra prediction for the DC coefficient
    prev_dc    = 0;
    blk_size   = band->blk_size;
    // number of blocks per mb
    num_blocks = (band->mb_size != blk_size) ? 4 : 1;
    if (blk_size == 8) {
        mc_with_delta_func     = IndeoDSP::ff_ivi_mc_8x8_delta;
        mc_no_delta_func       = IndeoDSP::ff_ivi_mc_8x8_no_delta;
        mc_avg_with_delta_func = IndeoDSP::ff_ivi_mc_avg_8x8_delta;
        mc_avg_no_delta_func   = IndeoDSP::ff_ivi_mc_avg_8x8_no_delta;
    } else {
        mc_with_delta_func     = IndeoDSP::ff_ivi_mc_4x4_delta;
        mc_no_delta_func       = IndeoDSP::ff_ivi_mc_4x4_no_delta;
        mc_avg_with_delta_func = IndeoDSP::ff_ivi_mc_avg_4x4_delta;
        mc_avg_no_delta_func   = IndeoDSP::ff_ivi_mc_avg_4x4_no_delta;
    }

    for (mbn = 0, mb = tile->mbs; mbn < tile->num_MBs; mb++, mbn++) {
        is_intra = !mb->type;
        cbp      = mb->cbp;
        buf_offs = mb->buf_offs;

        quant = band->glob_quant + mb->q_delta;
        if (_ctx.is_indeo4)
            quant = av_clip_uintp2(quant, 5);
        else
            quant = av_clip((int)quant, 0, 23);

        scale_tab = is_intra ? band->intra_scale : band->inter_scale;
        if (scale_tab)
            quant = scale_tab[quant];

        if (!is_intra) {
            mv_x  = mb->mv_x;
            mv_y  = mb->mv_y;
            mv_x2 = mb->b_mv_x;
            mv_y2 = mb->b_mv_y;
            if (band->is_halfpel) {
                mc_type  = ((mv_y  & 1) << 1) | (mv_x  & 1);
                mc_type2 = ((mv_y2 & 1) << 1) | (mv_x2 & 1);
                mv_x  >>= 1;
                mv_y  >>= 1;
                mv_x2 >>= 1;
                mv_y2 >>= 1; // convert halfpel vectors into fullpel ones
            }
            if (mb->type == 2)
                mc_type = -1;
            if (mb->type != 2 && mb->type != 3)
                mc_type2 = -1;
            if (mb->type) {
                int dmv_x, dmv_y, cx, cy;

                dmv_x = mb->mv_x >> band->is_halfpel;
                dmv_y = mb->mv_y >> band->is_halfpel;
                cx    = mb->mv_x &  band->is_halfpel;
                cy    = mb->mv_y &  band->is_halfpel;

                if (mb->xpos + dmv_x < 0 ||
                    mb->xpos + dmv_x + band->mb_size + cx > band->pitch ||
                    mb->ypos + dmv_y < 0 ||
                    mb->ypos + dmv_y + band->mb_size + cy > band->aheight) {
                    return -1;
                }
            }
            if (mb->type == 2 || mb->type == 3) {
                int dmv_x, dmv_y, cx, cy;

                dmv_x = mb->b_mv_x >> band->is_halfpel;
                dmv_y = mb->b_mv_y >> band->is_halfpel;
                cx    = mb->b_mv_x &  band->is_halfpel;
                cy    = mb->b_mv_y &  band->is_halfpel;

                if (mb->xpos + dmv_x < 0 ||
                    mb->xpos + dmv_x + band->mb_size + cx > band->pitch ||
                    mb->ypos + dmv_y < 0 ||
                    mb->ypos + dmv_y + band->mb_size + cy > band->aheight) {
                    return -1;
                }
            }
        }

        for (blk = 0; blk < num_blocks; blk++) {
            // adjust block position in the buffer according to its number
            if (blk & 1) {
                buf_offs += blk_size;
            } else if (blk == 2) {
                buf_offs -= blk_size;
                buf_offs += blk_size * band->pitch;
            }

            if (cbp & 1) { // block coded ?
                ret = ivi_decode_coded_blocks(gb, band, mc_with_delta_func,
                                              mc_avg_with_delta_func,
                                              mv_x, mv_y, mv_x2, mv_y2,
                                              &prev_dc, is_intra,
                                              mc_type, mc_type2, quant,
                                              buf_offs);
                if (ret < 0)
                    return ret;
            } else {
                // block not coded
                // for intra blocks apply the dc slant transform
                // for inter - perform the motion compensation without delta
                if (is_intra) {
                    ret = ivi_dc_transform(band, &prev_dc, buf_offs, blk_size);
                    if (ret < 0)
                        return ret;
                } else {
                    ret = ivi_mc(band, mc_no_delta_func, mc_avg_no_delta_func,
                                 buf_offs, mv_x, mv_y, mv_x2, mv_y2,
                                 mc_type, mc_type2);
                    if (ret < 0)
                        return ret;
                }
            }

            cbp >>= 1;
        }// for blk
    }// for mbn

	gb->alignGetBits();
    return 0;
}

int IndeoDecoderBase::ivi_scale_mv(int mv, int mv_scale){
	return (mv + (mv > 0) + (mv_scale - 1)) >> mv_scale;
}

int IndeoDecoderBase::ivi_mc(IVIBandDesc *band, ivi_mc_func mc, ivi_mc_avg_func mc_avg,
                  int offs, int mv_x, int mv_y, int mv_x2, int mv_y2,
                  int mc_type, int mc_type2){
    int ref_offs = offs + mv_y * band->pitch + mv_x;
    int buf_size = band->pitch * band->aheight;
    int min_size = band->pitch * (band->blk_size - 1) + band->blk_size;
    int ref_size = (mc_type > 1) * band->pitch + (mc_type & 1);

    if (mc_type != -1) {
        assert(offs >= 0 && ref_offs >= 0 && band->ref_buf);
        assert(buf_size - min_size >= offs);
        assert(buf_size - min_size - ref_size >= ref_offs);
    }

    if (mc_type2 == -1) {
        mc(band->buf + offs, band->ref_buf + ref_offs, band->pitch, mc_type);
    } else {
        int ref_offs2 = offs + mv_y2 * band->pitch + mv_x2;
        int ref_size2 = (mc_type2 > 1) * band->pitch + (mc_type2 & 1);
        if (offs < 0 || ref_offs2 < 0 || !band->b_ref_buf)
            return -1;
        if (buf_size - min_size - ref_size2 < ref_offs2)
            return -1;

        if (mc_type == -1)
            mc(band->buf + offs, band->b_ref_buf + ref_offs2,
               band->pitch, mc_type2);
        else
            mc_avg(band->buf + offs, band->ref_buf + ref_offs,
                   band->b_ref_buf + ref_offs2, band->pitch,
                   mc_type, mc_type2);
    }

    return 0;
}

int IndeoDecoderBase::ivi_decode_coded_blocks(GetBits *gb, IVIBandDesc *band,
		ivi_mc_func mc, ivi_mc_avg_func mc_avg, int mv_x, int mv_y,
		int mv_x2, int mv_y2, int *prev_dc, int is_intra,
		int mc_type, int mc_type2, uint32 quant, int offs) {
	const uint16 *base_tab = is_intra ? band->intra_base : band->inter_base;
	RVMapDesc *rvmap = band->rv_map;
	uint8 col_flags[8];
	int32 trvec[64];
	uint32 sym = 0, q;
	int lo, hi;
	int pos, run, val;
	int blk_size = band->blk_size;
	int num_coeffs = blk_size * blk_size;
	int col_mask = blk_size - 1;
	int scan_pos = -1;
	int min_size = band->pitch * (band->transform_size - 1) +
		band->transform_size;
	int buf_size = band->pitch * band->aheight - offs;

	if (min_size > buf_size)
		return -1;

	if (!band->scan) {
		warning("Scan pattern is not set.");
		return -1;
	}

	// zero transform vector
	memset(trvec, 0, num_coeffs * sizeof(trvec[0]));
	// zero column flags
	memset(col_flags, 0, sizeof(col_flags));
	while (scan_pos <= num_coeffs) {
		sym = gb->getVLC2(band->blk_vlc.tab->table,
			IVI_VLC_BITS, 1);
		if (sym == rvmap->eob_sym)
			break; // End of block

				   // Escape - run/val explicitly coded using 3 vlc codes
		if (sym == rvmap->esc_sym) {
			run = gb->getVLC2(band->blk_vlc.tab->table, IVI_VLC_BITS, 1) + 1;
			lo = gb->getVLC2(band->blk_vlc.tab->table, IVI_VLC_BITS, 1);
			hi = gb->getVLC2(band->blk_vlc.tab->table, IVI_VLC_BITS, 1);
			// merge them and convert into signed val
			val = IVI_TOSIGNED((hi << 6) | lo);
		} else {
			if (sym >= 256U) {
				warning("Invalid sym encountered");
				return -1;
			}
			run = rvmap->runtab[sym];
			val = rvmap->valtab[sym];
		}

		// de-zigzag and dequantize
		scan_pos += run;
		if (scan_pos >= num_coeffs || scan_pos < 0)
			break;
		pos = band->scan[scan_pos];

		if (!val)
			warning("Val = 0 encountered!");

		q = (base_tab[pos] * quant) >> 9;
		if (q > 1)
			val = val * q + FFSIGN(val) * (((q ^ 1) - 1) >> 1);
		trvec[pos] = val;
		// track columns containing non-zero coeffs
		col_flags[pos & col_mask] |= !!val;
	}

	if (scan_pos < 0 || scan_pos >= num_coeffs && sym != rvmap->eob_sym)
		return -1; // corrupt block data

	// undoing DC coeff prediction for intra-blocks
	if (is_intra && band->is_2d_trans) {
		*prev_dc += trvec[0];
		trvec[0] = *prev_dc;
		col_flags[0] |= !!*prev_dc;
	}

	if (band->transform_size > band->blk_size) {
		warning("Too large transform");
		return -1;
	}

	// apply inverse transform
	band->inv_transform(trvec, band->buf + offs,
		band->pitch, col_flags);

	// apply motion compensation
	if (!is_intra)
		return ivi_mc(band, mc, mc_avg, offs, mv_x, mv_y, mv_x2, mv_y2,
			mc_type, mc_type2);

	return 0;
}

int IndeoDecoderBase::ivi_dc_transform(IVIBandDesc *band, int *prev_dc,
		int buf_offs, int blk_size) {
	int buf_size = band->pitch * band->aheight - buf_offs;
	int min_size = (blk_size - 1) * band->pitch + blk_size;

	if (min_size > buf_size)
		return -1;

	band->dc_transform(prev_dc, band->buf + buf_offs,
		band->pitch, blk_size);

	return 0;
}


/**
 *  Scan patterns shared between indeo4 and indeo5
 */
const uint8 IndeoDecoderBase::_ff_ivi_vertical_scan_8x8[64] = {
	0,  8, 16, 24, 32, 40, 48, 56,
	1,  9, 17, 25, 33, 41, 49, 57,
	2, 10, 18, 26, 34, 42, 50, 58,
	3, 11, 19, 27, 35, 43, 51, 59,
	4, 12, 20, 28, 36, 44, 52, 60,
	5, 13, 21, 29, 37, 45, 53, 61,
	6, 14, 22, 30, 38, 46, 54, 62,
	7, 15, 23, 31, 39, 47, 55, 63
};

const uint8 IndeoDecoderBase::_ff_ivi_horizontal_scan_8x8[64] = {
	0,  1,  2,  3,  4,  5,  6,  7,
	8,  9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23,
	24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39,
	40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55,
	56, 57, 58, 59, 60, 61, 62, 63
};

const uint8 IndeoDecoderBase::_ff_ivi_direct_scan_4x4[16] = {
	0, 1, 4, 8, 5, 2, 3, 6, 9, 12, 13, 10, 7, 11, 14, 15
};

/*------------------------------------------------------------------------*/

int av_image_check_size(unsigned int w, unsigned int h, int log_offset, void *log_ctx) {
	if (((w + 128) * (uint64)(h + 128)) < (INT_MAX / 8))
		return 0;

	error("Picture size %ux%u is invalid", w, h);
}

} // End of namespace Indeo
} // End of namespace Image
