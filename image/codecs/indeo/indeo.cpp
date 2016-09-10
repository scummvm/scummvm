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
#include "graphics/yuv_to_rgb.h"
#include "common/system.h"
#include "common/algorithm.h"
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

IVIHuffTab::IVIHuffTab() : tab(nullptr) {
}

int IVIHuffTab::ff_ivi_dec_huff_desc(IVI45DecContext *ctx, int desc_coded, int which_tab) {
	int i, result;
	IVIHuffDesc new_huff;

	if (!desc_coded) {
		// select default table
		tab = (which_tab) ? &ctx->ivi_blk_vlc_tabs[7]
			: &ctx->ivi_mb_vlc_tabs[7];
		return 0;
	}

	tab_sel = ctx->gb->getBits(3);
	if (tab_sel == 7) {
		// custom huffman table (explicitly encoded)
		new_huff.num_rows = ctx->gb->getBits(4);
		if (!new_huff.num_rows) {
			warning("Empty custom Huffman table!");
			return -1;
		}

		for (i = 0; i < new_huff.num_rows; i++)
			new_huff.xbits[i] = ctx->gb->getBits(4);

		// Have we got the same custom table? Rebuild if not.
		if (new_huff.ivi_huff_desc_cmp(&cust_desc) || !cust_tab._table) {
			cust_desc.ivi_huff_desc_copy(&new_huff);

			if (cust_tab._table)
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
		tab = (which_tab) ? &ctx->ivi_blk_vlc_tabs[tab_sel]
			: &ctx->ivi_mb_vlc_tabs[tab_sel];
	}

	return 0;
}

/*------------------------------------------------------------------------*/

IVIMbInfo::IVIMbInfo() : xpos(0), ypos(0), buf_offs(0), type(0), cbp(0),
		q_delta(0), mv_x(0), mv_y(0), b_mv_x(0), b_mv_y(0) {
}

/*------------------------------------------------------------------------*/

IVITile::IVITile() : xpos(0), ypos(0), width(0), height(0), mb_size(0),
		is_empty(0), data_size(0), num_MBs(0), mbs(nullptr), ref_mbs(nullptr) {
}

/*------------------------------------------------------------------------*/

IVIBandDesc::IVIBandDesc() : plane(0), band_num(0), width(0), height(0),
		aheight(0), data_ptr(nullptr), data_size(0), buf(nullptr),
		ref_buf(nullptr), b_ref_buf(nullptr), pitch(0), is_empty(0),
		mb_size(0), blk_size(0), is_halfpel(0), inherit_mv(0), bufsize(0),
		inherit_qdelta(0), qdelta_present(0), quant_mat(0), glob_quant(0),
		scan(nullptr), scan_size(0), num_corr(0), rvmap_sel(0), rv_map(nullptr),
		num_tiles(0), tiles(nullptr), inv_transform(nullptr), transform_size(0),
		dc_transform(nullptr), is_2d_trans(0), checksum(0), checksum_present(0), 
		intra_base(nullptr), inter_base(nullptr), intra_scale(nullptr),
		inter_scale(nullptr) {
	Common::fill(&bufs[0], &bufs[4], (int16 *)nullptr);
	Common::fill(&corr[0], &corr[61 * 2], 0);
}

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

IVIPicConfig::IVIPicConfig() : pic_width(0), pic_height(0), chroma_width(0),
		chroma_height(0), tile_width(0), tile_height(0), luma_bands(0), chroma_bands(0) {
}

bool IVIPicConfig::ivi_pic_config_cmp(const IVIPicConfig &cfg2) {
	return pic_width != cfg2.pic_width || pic_height != cfg2.pic_height ||
		chroma_width != cfg2.chroma_width || chroma_height != cfg2.chroma_height ||
		tile_width != cfg2.tile_width || tile_height != cfg2.tile_height ||
		luma_bands != cfg2.luma_bands || chroma_bands != cfg2.chroma_bands;
}

/*------------------------------------------------------------------------*/

IVIPlaneDesc::IVIPlaneDesc() : width(0), height(0), num_bands(0), bands(nullptr) {
}

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

				if (planes[p].bands[b].blk_vlc.cust_tab._table)
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

AVFrame::AVFrame() {
	Common::fill(&_data[0], &_data[AV_NUM_DATA_POINTERS], (uint8 *)nullptr);
	Common::fill(&_linesize[0], &_linesize[AV_NUM_DATA_POINTERS], 0);
}

int AVFrame::ff_set_dimensions(uint16 width, uint16 height) {
	_width = width;
	_height = height;
	_linesize[0] = _linesize[1] = _linesize[2] = width;

	return 0;
}

int AVFrame::ff_get_buffer(int flags) {
	av_frame_free();

	// Luminance channel
	_data[0] = (uint8 *)av_mallocz(_width * _height);
	
	// UV Chroma Channels
	_data[1] = (uint8 *)av_malloc(_width * _height);
	_data[2] = (uint8 *)av_malloc(_width * _height);
	Common::fill(_data[1], _data[1] + _width * _height, 0x80);
	Common::fill(_data[2], _data[2] + _width * _height, 0x80);

	return 0;
}

void AVFrame::av_frame_free() {
	av_freep(&_data[0]);
	av_freep(&_data[1]);
	av_freep(&_data[2]);
}

/*------------------------------------------------------------------------*/

IVI45DecContext::IVI45DecContext() : gb(nullptr), frame_num(0), frame_type(0),
		prev_frame_type(0), data_size(0), is_scalable(0), frame_data(0),
		inter_scal(0), frame_size(0), pic_hdr_size(0), frame_flags(0),
		checksum(0), buf_switch(0), dst_buf(0), ref_buf(0), ref2_buf(0),
		b_ref_buf(0), rvmap_sel(0), in_imf(0), in_q(0), pic_glob_quant(0),
		unknown1(0), gop_hdr_size(0), gop_flags(0), lock_word(0), has_b_frames(0),
		has_transp(0), uses_tiling(0), uses_haar(0), uses_fullpel(0), gop_invalid(0),
		is_indeo4(0), p_frame(nullptr), got_p_frame(0) {
	Common::fill(&buf_invalid[0], &buf_invalid[4], 0);
	Common::copy(&_ff_ivi_rvmap_tabs[0], &_ff_ivi_rvmap_tabs[9], &rvmap_tabs[0]);

	for (int idx = 0; idx < (8192 * 16); ++idx)
		table_data[idx][0] = table_data[idx][1] = 0;

	for (int i = 0; i < 8; i++) {
		ivi_mb_vlc_tabs[i]._table = table_data + i * 2 * 8192;
		ivi_mb_vlc_tabs[i]._table_allocated = 8192;
		ivi_mb_huff_desc[i].ivi_create_huff_from_desc(&ivi_mb_vlc_tabs[i], 1);
		ivi_blk_vlc_tabs[i]._table = table_data + (i * 2 + 1) * 8192;
		ivi_blk_vlc_tabs[i]._table_allocated = 8192;
		ivi_blk_huff_desc[i].ivi_create_huff_from_desc(&ivi_blk_vlc_tabs[i], 1);
	}
}

/*------------------------------------------------------------------------*/

IndeoDecoderBase::IndeoDecoderBase(uint16 width, uint16 height) : Codec() {
	_pixelFormat = g_system->getScreenFormat();
	assert(_pixelFormat.bytesPerPixel > 1);
	_surface = new Graphics::ManagedSurface();
	_surface->create(width, height, _pixelFormat);
	_surface->fillRect(Common::Rect(0, 0, width, height), 0);
	_ctx.b_ref_buf = 3; // buffer 2 is used for scalability mode
}

IndeoDecoderBase::~IndeoDecoderBase() {
	delete _surface;
	IVIPlaneDesc::ivi_free_buffers(_ctx.planes);
	if (_ctx.mb_vlc.cust_tab._table)
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

	assert(_ctx.planes[0].width <= _surface->w && _ctx.planes[0].height <= _surface->h);
	result = frame->ff_set_dimensions(_ctx.planes[0].width, _ctx.planes[0].height);
	if (result < 0)
		return result;

	if ((result = frame->ff_get_buffer(0)) < 0)
		return result;

	if (_ctx.is_scalable) {
		if (_ctx.is_indeo4)
			ff_ivi_recompose_haar(&_ctx.planes[0], frame->_data[0], frame->_linesize[0]);
		else
			ff_ivi_recompose53(&_ctx.planes[0], frame->_data[0], frame->_linesize[0]);
	} else {
		ivi_output_plane(&_ctx.planes[0], frame->_data[0], frame->_linesize[0]);
	}

	ivi_output_plane(&_ctx.planes[2], frame->_data[1], frame->_linesize[1]);
	ivi_output_plane(&_ctx.planes[1], frame->_data[2], frame->_linesize[2]);

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

	// Merge the planes into the final surface
	Graphics::Surface s = _surface->getSubArea(Common::Rect(0, 0, _surface->w, _surface->h));
	YUVToRGBMan.convert410(&s, Graphics::YUVToRGBManager::kScaleITU,		
		frame->_data[0], frame->_data[1], frame->_data[2], frame->_width, frame->_height,
		frame->_width, frame->_width);

	// Free the now un-needed frame data
	frame->av_frame_free();

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
		sym = gb->getVLC2(band->blk_vlc.tab->_table,
			IVI_VLC_BITS, 1);
		if (sym == rvmap->eob_sym)
			break; // End of block

				   // Escape - run/val explicitly coded using 3 vlc codes
		if (sym == rvmap->esc_sym) {
			run = gb->getVLC2(band->blk_vlc.tab->_table, IVI_VLC_BITS, 1) + 1;
			lo = gb->getVLC2(band->blk_vlc.tab->_table, IVI_VLC_BITS, 1);
			hi = gb->getVLC2(band->blk_vlc.tab->_table, IVI_VLC_BITS, 1);
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

/*------------------------------------------------------------------------*/

int av_image_check_size(unsigned int w, unsigned int h, int log_offset, void *log_ctx) {
	if (((w + 128) * (uint64)(h + 128)) < (INT_MAX / 8))
		return 0;

	error("Picture size %ux%u is invalid", w, h);
}

/*------------------------------------------------------------------------*/

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

const RVMapDesc IVI45DecContext::_ff_ivi_rvmap_tabs[9] = {
{   // MapTab0
    5, // eob_sym
    2, // esc_sym
    // run table
    {1,  1,  0,  1,  1,  0,  1,  1,  2,  2,  1,  1,  1,  1,  3,  3,
     1,  1,  2,  2,  1,  1,  4,  4,  1,  1,  1,  1,  2,  2,  5,  5,
     1,  1,  3,  3,  1,  1,  6,  6,  1,  2,  1,  2,  7,  7,  1,  1,
     8,  8,  1,  1,  4,  2,  1,  4,  2,  1,  3,  3,  1,  1,  1,  9,
     9,  1,  2,  1,  2,  1,  5,  5,  1,  1, 10, 10,  1,  1,  3,  3,
     2,  2,  1,  1, 11, 11,  6,  4,  4,  1,  6,  1,  2,  1,  2, 12,
     8,  1, 12,  7,  8,  7,  1, 16,  1, 16,  1,  3,  3, 13,  1, 13,
     2,  2,  1, 15,  1,  5, 14, 15,  1,  5, 14,  1, 17,  8, 17,  8,
     1,  4,  4,  2,  2,  1, 25, 25, 24, 24,  1,  3,  1,  3,  1,  8,
     6,  7,  6,  1, 18,  8, 18,  1,  7, 23,  2,  2, 23,  1,  1, 21,
    22,  9,  9, 22, 19,  1, 21,  5, 19,  5,  1, 33, 20, 33, 20,  8,
     4,  4,  1, 32,  2,  2,  8,  3, 32, 26,  3,  1,  7,  7, 26,  6,
     1,  6,  1,  1, 16,  1, 10,  1, 10,  2, 16, 29, 28,  2, 29, 28,
     1, 27,  5,  8,  5, 27,  1,  8,  3,  7,  3, 31, 41, 31,  1, 41,
     6,  1,  6,  7,  4,  4,  1,  1,  2,  1,  2, 11, 34, 30, 11,  1,
    30, 15, 15, 34, 36, 40, 36, 40, 35, 35, 37, 37, 39, 39, 38, 38},

    // value table
    { 1,  -1,   0,   2,  -2,   0,   3,  -3,   1,  -1,   4,  -4,   5,  -5,   1,  -1,
      6,  -6,   2,  -2,   7,  -7,   1,  -1,   8,  -8,   9,  -9,   3,  -3,   1,  -1,
     10, -10,   2,  -2,  11, -11,   1,  -1,  12,   4, -12,  -4,   1,  -1,  13, -13,
      1,  -1,  14, -14,   2,   5,  15,  -2,  -5, -15,  -3,   3,  16, -16,  17,   1,
     -1, -17,   6,  18,  -6, -18,   2,  -2,  19, -19,   1,  -1,  20, -20,   4,  -4,
      7,  -7,  21, -21,   1,  -1,   2,   3,  -3,  22,  -2, -22,   8,  23,  -8,   1,
      2, -23,  -1,   2,  -2,  -2,  24,   1, -24,  -1,  25,   5,  -5,   1, -25,  -1,
      9,  -9,  26,   1, -26,   3,   1,  -1,  27,  -3,  -1, -27,   1,   3,  -1,  -3,
     28,  -4,   4,  10, -10, -28,   1,  -1,   1,  -1,  29,   6, -29,  -6,  30,  -4,
      3,   3,  -3, -30,   1,   4,  -1,  31,  -3,   1,  11, -11,  -1, -31,  32,  -1,
     -1,   2,  -2,   1,   1, -32,   1,   4,  -1,  -4,  33,  -1,   1,   1,  -1,   5,
      5,  -5, -33,  -1, -12,  12,  -5,  -7,   1,   1,   7,  34,   4,  -4,  -1,   4,
    -34,  -4,  35,  36,  -2, -35,  -2, -36,   2,  13,   2,  -1,   1, -13,   1,  -1,
     37,   1,  -5,   6,   5,  -1,  38,  -6,  -8,   5,   8,  -1,   1,   1, -37,  -1,
      5,  39,  -5,  -5,   6,  -6, -38, -39, -14,  40,  14,   2,   1,   1,  -2, -40,
     -1,  -2,   2,  -1,  -1,  -1,   1,   1,   1,  -1,   1,  -1,   1,  -1,   1,  -1}
},{
    // MapTab1
    0,  // eob_sym
    38, // esc_sym
    // run table
    {0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  8,  6,  8,  7,
     7,  9,  9, 10, 10, 11, 11,  1, 12,  1, 12, 13, 13, 16, 14, 16,
    14, 15, 15, 17, 17, 18,  0, 18, 19, 20, 21, 19, 22, 21, 20, 22,
    25, 24,  2, 25, 24, 23, 23,  2, 26, 28, 26, 28, 29, 27, 29, 27,
    33, 33,  1, 32,  1,  3, 32, 30, 36,  3, 36, 30, 31, 31, 35, 34,
    37, 41, 34, 35, 37,  4, 41,  4, 49,  8,  8, 49, 40, 38,  5, 38,
    40, 39,  5, 39, 42, 43, 42,  7, 57,  6, 43, 44,  6, 50,  7, 44,
    57, 48, 50, 48, 45, 45, 46, 47, 51, 46, 47, 58,  1, 51, 58,  1,
    52, 59, 53,  9, 52, 55, 55, 59, 53, 56, 54, 56, 54,  9, 64, 64,
    60, 63, 60, 63, 61, 62, 61, 62,  2, 10,  2, 10, 11,  1, 11, 13,
    12,  1, 12, 13, 16, 16,  8,  8, 14,  3,  3, 15, 14, 15,  4,  4,
     1, 17, 17,  5,  1,  7,  7,  5,  6,  1,  2,  2,  6, 22,  1, 25,
    21, 22,  8, 24,  1, 21, 25, 24,  8, 18, 18, 23,  9, 20, 23, 33,
    29, 33, 20,  1, 19,  1, 29, 36,  9, 36, 19, 41, 28, 57, 32,  3,
    28,  3,  1, 27, 49, 49,  1, 32, 26, 26,  2,  4,  4,  7, 57, 41,
     2,  7, 10,  5, 37, 16, 10, 27,  8,  8, 13, 16, 37, 13,  1,  5},

    // value table
    {0,   1,  -1,   1,  -1,   1,  -1,   1,  -1,   1,  -1,   1,   1,  -1,  -1,   1,
    -1,   1,  -1,   1,  -1,   1,  -1,   2,   1,  -2,  -1,   1,  -1,   1,   1,  -1,
    -1,   1,  -1,   1,  -1,   1,   0,  -1,   1,   1,   1,  -1,   1,  -1,  -1,  -1,
     1,   1,   2,  -1,  -1,   1,  -1,  -2,   1,   1,  -1,  -1,   1,   1,  -1,  -1,
     1,  -1,   3,   1,  -3,   2,  -1,   1,   1,  -2,  -1,  -1,  -1,   1,   1,   1,
     1,   1,  -1,  -1,  -1,   2,  -1,  -2,   1,   2,  -2,  -1,   1,   1,   2,  -1,
    -1,   1,  -2,  -1,   1,   1,  -1,   2,   1,   2,  -1,   1,  -2,  -1,  -2,  -1,
    -1,   1,   1,  -1,   1,  -1,   1,   1,   1,  -1,  -1,   1,   4,  -1,  -1,  -4,
     1,   1,   1,   2,  -1,  -1,   1,  -1,  -1,   1,  -1,  -1,   1,  -2,   1,  -1,
     1,   1,  -1,  -1,   1,   1,  -1,  -1,   3,   2,  -3,  -2,   2,   5,  -2,   2,
     2,  -5,  -2,  -2,  -2,   2,  -3,   3,   2,   3,  -3,   2,  -2,  -2,   3,  -3,
     6,   2,  -2,   3,  -6,   3,  -3,  -3,   3,   7,  -4,   4,  -3,   2,  -7,   2,
     2,  -2,  -4,   2,   8,  -2,  -2,  -2,   4,   2,  -2,   2,   3,   2,  -2,  -2,
     2,   2,  -2,  -8,  -2,   9,  -2,   2,  -3,  -2,   2,  -2,   2,   2,   2,   4,
    -2,  -4,  10,   2,   2,  -2,  -9,  -2,   2,  -2,   5,   4,  -4,   4,  -2,   2,
    -5,  -4,  -3,   4,   2,  -3,   3,  -2,  -5,   5,   3,   3,  -2,  -3, -10,  -4}
},{
    // MapTab2
    2,  // eob_sym
    11, // esc_sym
    // run table
    {1,  1,  0,  2,  2,  1,  1,  3,  3,  4,  4,  0,  1,  1,  5,  5,
     2,  2,  6,  6,  7,  7,  1,  8,  1,  8,  3,  3,  9,  9,  1,  2,
     2,  1,  4, 10,  4, 10, 11, 11,  1,  5, 12, 12,  1,  5, 13, 13,
     3,  3,  6,  6,  2,  2, 14, 14, 16, 16, 15,  7, 15,  8,  8,  7,
     1,  1, 17, 17,  4,  4,  1,  1, 18, 18,  2,  2,  5,  5, 25,  3,
     9,  3, 25,  9, 19, 24, 19, 24,  1, 21, 20,  1, 21, 22, 20, 22,
    23, 23,  8,  6, 33,  6,  8, 33,  7,  7, 26, 26,  1, 32,  1, 32,
    28,  4, 28, 10, 29, 27, 27, 10, 41,  4, 29,  2,  2, 41, 36, 31,
    49, 31, 34, 30, 34, 36, 30, 35,  1, 49, 11,  5, 35, 11,  1,  3,
     3,  5, 37, 37,  8, 40,  8, 40, 12, 12, 42, 42,  1, 38, 16, 57,
     1,  6, 16, 39, 38,  6,  7,  7, 13, 13, 39, 43,  2, 43, 57,  2,
    50,  9, 44,  9, 50,  4, 15, 48, 44,  4,  1, 15, 48, 14, 14,  1,
    45, 45,  8,  3,  5,  8, 51, 47,  3, 46, 46, 47,  5, 51,  1, 17,
    17, 58,  1, 58,  2, 52, 52,  2, 53,  7, 59,  6,  6, 56, 53, 55,
     7, 55,  1, 54, 59, 56, 54, 10,  1, 10,  4, 60,  1, 60,  8,  4,
     8, 64, 64, 61,  1, 63,  3, 63, 62, 61,  5, 11,  5,  3, 11, 62},

    // value table
    { 1,  -1,   0,   1,  -1,   2,  -2,   1,  -1,   1,  -1,   0,   3,  -3,   1,  -1,
      2,  -2,   1,  -1,   1,  -1,   4,   1,  -4,  -1,   2,  -2,   1,  -1,   5,   3,
     -3,  -5,   2,   1,  -2,  -1,   1,  -1,   6,   2,   1,  -1,  -6,  -2,   1,  -1,
      3,  -3,   2,  -2,   4,  -4,   1,  -1,   1,  -1,   1,   2,  -1,   2,  -2,  -2,
      7,  -7,   1,  -1,   3,  -3,   8,  -8,   1,  -1,   5,  -5,   3,  -3,   1,   4,
      2,  -4,  -1,  -2,   1,   1,  -1,  -1,   9,   1,   1,  -9,  -1,   1,  -1,  -1,
      1,  -1,   3,  -3,   1,   3,  -3,  -1,   3,  -3,   1,  -1,  10,   1, -10,  -1,
      1,   4,  -1,   2,   1,  -1,   1,  -2,   1,  -4,  -1,   6,  -6,  -1,   1,   1,
      1,  -1,   1,   1,  -1,  -1,  -1,   1,  11,  -1,  -2,   4,  -1,   2, -11,   5,
     -5,  -4,  -1,   1,   4,   1,  -4,  -1,  -2,   2,   1,  -1,  12,   1,  -2,   1,
    -12,   4,   2,   1,  -1,  -4,   4,  -4,   2,  -2,  -1,   1,   7,  -1,  -1,  -7,
     -1,  -3,   1,   3,   1,   5,   2,   1,  -1,  -5,  13,  -2,  -1,   2,  -2, -13,
      1,  -1,   5,   6,   5,  -5,   1,   1,  -6,   1,  -1,  -1,  -5,  -1,  14,   2,
     -2,   1, -14,  -1,   8,   1,  -1,  -8,   1,   5,   1,   5,  -5,   1,  -1,   1,
     -5,  -1,  15,   1,  -1,  -1,  -1,   3, -15,  -3,   6,   1,  16,  -1,   6,  -6,
     -6,   1,  -1,   1, -16,   1,   7,  -1,   1,  -1,  -6,  -3,   6,  -7,   3,  -1}
},{
    // MapTab3
    0,  // eob_sym
    35, // esc_sym
    // run table
    {0,  1,  1,  2,  2,  3,  3,  4,  4,  1,  1,  5,  5,  6,  6,  7,
     7,  8,  8,  9,  9,  2,  2, 10, 10,  1,  1, 11, 11, 12, 12,  3,
     3, 13, 13,  0, 14, 14, 16, 15, 16, 15,  4,  4, 17,  1, 17,  1,
     5,  5, 18, 18,  2,  2,  6,  6,  8, 19,  7,  8,  7, 19, 20, 20,
    21, 21, 22, 24, 22, 24, 23, 23,  1,  1, 25, 25,  3,  3, 26, 26,
     9,  9, 27, 27, 28, 28, 33, 29,  4, 33, 29,  1,  4,  1, 32, 32,
     2,  2, 31, 10, 30, 10, 30, 31, 34, 34,  5,  5, 36, 36, 35, 41,
    35, 11, 41, 11, 37,  1,  8,  8, 37,  6,  1,  6, 40,  7,  7, 40,
    12, 38, 12, 39, 39, 38, 49, 13, 49, 13,  3, 42,  3, 42, 16, 16,
    43, 43, 14, 14,  1,  1, 44, 15, 44, 15,  2,  2, 57, 48, 50, 48,
    57, 50,  4, 45, 45,  4, 46, 47, 47, 46,  1, 51,  1, 17, 17, 51,
     8,  9,  9,  5, 58,  8, 58,  5, 52, 52, 55, 56, 53, 56, 55, 59,
    59, 53, 54,  1,  6, 54,  7,  7,  6,  1,  2,  3,  2,  3, 64, 60,
    60, 10, 10, 64, 61, 62, 61, 63,  1, 63, 62,  1, 18, 24, 18,  4,
    25,  4,  8, 21, 21,  1, 24, 22, 25, 22,  8, 11, 19, 11, 23,  1,
    20, 23, 19, 20,  5, 12,  5,  1, 16,  2, 12, 13,  2, 13,  1, 16},

    // value table
    { 0,   1,  -1,   1,  -1,   1,  -1,   1,  -1,   2,  -2,   1,  -1,   1,  -1,   1,
     -1,   1,  -1,   1,  -1,   2,  -2,   1,  -1,   3,  -3,   1,  -1,   1,  -1,   2,
     -2,   1,  -1,   0,   1,  -1,   1,   1,  -1,  -1,   2,  -2,   1,   4,  -1,  -4,
      2,  -2,   1,  -1,  -3,   3,   2,  -2,   2,   1,   2,  -2,  -2,  -1,   1,  -1,
      1,  -1,   1,   1,  -1,  -1,   1,  -1,   5,  -5,   1,  -1,   3,  -3,   1,  -1,
      2,  -2,   1,  -1,   1,  -1,   1,   1,   3,  -1,  -1,   6,  -3,  -6,  -1,   1,
      4,  -4,   1,   2,   1,  -2,  -1,  -1,   1,  -1,   3,  -3,   1,  -1,   1,   1,
     -1,   2,  -1,  -2,   1,   7,  -3,   3,  -1,   3,  -7,  -3,   1,  -3,   3,  -1,
      2,   1,  -2,   1,  -1,  -1,   1,   2,  -1,  -2,  -4,  -1,   4,   1,   2,  -2,
      1,  -1,  -2,   2,   8,  -8,  -1,   2,   1,  -2,  -5,   5,   1,  -1,  -1,   1,
     -1,   1,   4,  -1,   1,  -4,  -1,  -1,   1,   1,   9,   1,  -9,   2,  -2,  -1,
     -4,   3,  -3,  -4,  -1,   4,   1,   4,   1,  -1,   1,  -1,   1,   1,  -1,   1,
     -1,  -1,  -1,  10,   4,   1,   4,  -4,  -4, -10,   6,   5,  -6,  -5,   1,  -1,
      1,   3,  -3,  -1,   1,  -1,  -1,  -1,  11,   1,   1, -11,  -2,  -2,   2,   5,
     -2,  -5,  -5,   2,  -2,  12,   2,  -2,   2,   2,   5,  -3,  -2,   3,  -2, -12,
     -2,   2,   2,   2,  -5,   3,   5,  13,  -3,   7,  -3,  -3,  -7,   3, -13,   3}
},{
    // MapTab4
    0,  // eob_sym
    34, // esc_sym
    // run table
    {0,  1,  1,  1,  2,  2,  1,  3,  3,  1,  1,  1,  4,  4,  1,  5,
     2,  1,  5,  2,  1,  1,  6,  6,  1,  1,  1,  1,  1,  7,  3,  1,
     2,  3,  0,  1,  2,  7,  1,  1,  1,  8,  1,  1,  8,  1,  1,  1,
     9,  1,  9,  1,  2,  1,  1,  2,  1,  1, 10,  4,  1, 10,  1,  4,
     1,  1,  1,  1,  1,  3,  1,  1,  1,  3,  2,  1,  5,  1,  1,  1,
     2,  5,  1, 11,  1, 11,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
     2,  1,  6,  1,  6,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1, 12,
     3,  1, 12,  1,  1,  1,  2,  1,  1,  3,  1,  1,  1,  1,  1,  1,
     4,  1,  1,  1,  2,  1,  1,  4,  1,  1,  1,  1,  1,  1,  2,  1,
     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  3,  1,  2,  1,  1,  5,
     1,  1,  1,  1,  1,  7,  1,  7,  1,  1,  2,  3,  1,  1,  1,  1,
     5,  1,  1,  1,  1,  1,  1,  2, 13,  1,  1,  1,  1,  1,  1,  1,
     1,  1,  1,  1,  1,  1,  1,  1, 13,  2,  1,  1,  4,  1,  1,  1,
     3,  1,  6,  1,  1,  1, 14,  1,  1,  1,  1,  1, 14,  6,  1,  1,
     1,  1, 15,  2,  4,  1,  2,  3, 15,  1,  1,  1,  8,  1,  1,  8,
     1,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1},

    // value table
    { 0,   1,  -1,   2,   1,  -1,  -2,   1,  -1,   3,  -3,   4,   1,  -1,  -4,   1,
      2,   5,  -1,  -2,  -5,   6,   1,  -1,  -6,   7,  -7,   8,  -8,   1,   2,   9,
      3,  -2,   0,  -9,  -3,  -1,  10, -10,  11,   1, -11,  12,  -1, -12,  13, -13,
      1,  14,  -1, -14,   4,  15, -15,  -4,  16, -16,   1,   2,  17,  -1, -17,  -2,
     18, -18,  19, -19,  20,   3, -20,  21, -21,  -3,   5,  22,   2, -22, -23,  23,
     -5,  -2,  24,   1, -24,  -1,  25, -25,  26, -26, -27,  27,  28,  29, -28, -29,
      6,  30,   2, -31,  -2, -30,  31,  -6, -32,  32,  33, -33,  34, -35, -34,   1,
      4, -36,  -1,  35,  37,  36,   7, -37,  38,  -4, -38,  39,  41,  40, -40, -39,
      3,  42, -43, -41,  -7, -42,  43,  -3,  44, -44,  45, -45,  46,  47,   8, -47,
    -48, -46,  50, -50,  48,  49,  51, -49,  52, -52,   5, -51,  -8, -53,  53,   3,
    -56,  56,  55,  54, -54,   2,  60,  -2, -55,  58,   9,  -5,  59,  57, -57, -63,
     -3, -58, -60, -61,  61, -59, -62,  -9,   1,  64,  62,  69, -64,  63,  65, -67,
    -68,  66, -65,  68, -66, -69,  67, -70,  -1,  10,  71, -71,   4,  73,  72,  70,
      6, -76,  -3,  74, -78, -74,   1,  78,  80, -72, -75,  76,  -1,   3, -73,  79,
     75,  77,   1,  11,  -4, -79, -10,  -6,  -1, -77, -83, -80,   2,  81, -84,  -2,
     83, -81,  82, -82,  84, -87, -86,  85, -11, -85,  86, -89,  87, -88,  88,  89}
},{
    // MapTab5
    2,  // eob_sym
    33, // esc_sym
    // run table
    {1,  1,  0,  2,  1,  2,  1,  3,  3,  1,  1,  4,  4,  2,  2,  1,
     1,  5,  5,  6,  1,  6,  1,  7,  7,  3,  3,  2,  8,  2,  8,  1,
     1,  0,  9,  9,  1,  1, 10,  4, 10,  4, 11, 11,  2,  1,  2,  1,
    12, 12,  3,  3,  1,  1, 13,  5,  5, 13, 14,  1,  1, 14,  2,  2,
     6,  6, 15,  1,  1, 15, 16,  4,  7, 16,  4,  7,  1,  1,  3,  3,
     8,  8,  2,  2,  1,  1, 17, 17,  1,  1, 18, 18,  5,  5,  2,  2,
     1,  1,  9, 19,  9, 19, 20,  3,  3, 20,  1, 10, 21,  1, 10,  4,
     4, 21, 22,  6,  6, 22,  1,  1, 23, 24,  2,  2, 23, 24, 11,  1,
     1, 11,  7, 25,  7,  1,  1, 25,  8,  8,  3, 26,  3,  1, 12,  2,
     2, 26,  1, 12,  5,  5, 27,  4,  1,  4,  1, 27, 28,  1, 28, 13,
     1, 13,  2, 29,  2,  1, 32,  6,  1, 30, 14, 29, 14,  6,  3, 31,
     3,  1, 30,  1, 32, 31, 33,  9, 33,  1,  1,  7,  9,  7,  2,  2,
     1,  1,  4, 36, 34,  4,  5, 10, 10,  5, 34,  1,  1, 35,  8,  8,
    36,  3, 35,  1, 15,  3,  2,  1, 16, 15, 16,  2, 37,  1, 37,  1,
     1,  1,  6,  6, 38,  1, 38, 11,  1, 39, 39, 40, 11,  2, 41,  4,
    40,  1,  2,  4,  1,  1,  1, 41,  3,  1,  3,  1,  5,  7,  5,  7},

    // value table
    { 1,  -1,   0,   1,   2,  -1,  -2,   1,  -1,   3,  -3,   1,  -1,   2,  -2,   4,
     -4,   1,  -1,   1,   5,  -1,  -5,   1,  -1,   2,  -2,   3,   1,  -3,  -1,   6,
     -6,   0,   1,  -1,   7,  -7,   1,   2,  -1,  -2,   1,  -1,   4,   8,  -4,  -8,
      1,  -1,   3,  -3,   9,  -9,   1,   2,  -2,  -1,   1,  10, -10,  -1,   5,  -5,
      2,  -2,   1,  11, -11,  -1,   1,   3,   2,  -1,  -3,  -2,  12, -12,   4,  -4,
      2,  -2,  -6,   6,  13, -13,   1,  -1,  14, -14,   1,  -1,   3,  -3,   7,  -7,
     15, -15,   2,   1,  -2,  -1,   1,   5,  -5,  -1, -16,   2,   1,  16,  -2,   4,
     -4,  -1,   1,   3,  -3,  -1,  17, -17,   1,   1,  -8,   8,  -1,  -1,   2,  18,
    -18,  -2,   3,   1,  -3,  19, -19,  -1,   3,  -3,   6,   1,  -6,  20,   2,   9,
     -9,  -1, -20,  -2,   4,  -4,   1,  -5,  21,   5, -21,  -1,   1, -22,  -1,   2,
     22,  -2,  10,   1, -10,  23,   1,   4, -23,   1,   2,  -1,  -2,  -4,  -7,   1,
      7, -24,  -1,  24,  -1,  -1,   1,   3,  -1, -25,  25,   4,  -3,  -4,  11, -11,
     26, -26,   6,   1,   1,  -6,  -5,  -3,   3,   5,  -1, -27,  27,   1,   4,  -4,
     -1,  -8,  -1,  28,   2,   8, -12, -28,  -2,  -2,   2,  12,  -1,  29,   1, -29,
     30, -30,   5,  -5,   1, -31,  -1,   3,  31,  -1,   1,   1,  -3, -13,   1,  -7,
     -1, -32,  13,   7,  32,  33, -33,  -1,  -9, -34,   9,  34,  -6,   5,   6,  -5}
},{
    // MapTab6
    2,  // eob_sym
    13, // esc_sym
    // run table
    {1,  1,  0,  1,  1,  2,  2,  1,  1,  3,  3,  1,  1,  0,  2,  2,
     4,  1,  4,  1,  1,  1,  5,  5,  1,  1,  6,  6,  2,  2,  1,  1,
     3,  3,  7,  7,  1,  1,  8,  8,  1,  1,  2,  2,  1,  9,  1,  9,
     4,  4, 10,  1,  1, 10,  1,  1, 11, 11,  3,  3,  1,  2,  1,  2,
     1,  1, 12, 12,  5,  5,  1,  1, 13,  1,  1, 13,  2,  2,  1,  1,
     6,  6,  1,  1,  4, 14,  4, 14,  3,  1,  3,  1,  1,  1, 15,  7,
    15,  2,  2,  7,  1,  1,  1,  8,  1,  8, 16, 16,  1,  1,  1,  1,
     2,  1,  1,  2,  1,  1,  3,  5,  5,  3,  4,  1,  1,  4,  1,  1,
    17, 17,  9,  1,  1,  9,  2,  2,  1,  1, 10, 10,  1,  6,  1,  1,
     6, 18,  1,  1, 18,  1,  1,  1,  2,  2,  3,  1,  3,  1,  1,  1,
     4,  1, 19,  1, 19,  7,  1,  1, 20,  1,  4, 20,  1,  7, 11,  2,
     1, 11, 21,  2,  8,  5,  1,  8,  1,  5, 21,  1,  1,  1, 22,  1,
     1, 22,  1,  1,  3,  3,  1, 23,  2, 12, 24,  1,  1,  2,  1,  1,
    12, 23,  1,  1, 24,  1,  1,  1,  4,  1,  1,  1,  2,  1,  6,  6,
     4,  2,  1,  1,  1,  1,  1,  1,  1, 14, 13,  3,  1, 25,  9, 25,
    14,  1,  9,  3, 13,  1,  1,  1,  1,  1, 10,  1,  1,  2, 10,  2},

    // value table
    {-20,  -1,   0,   2,  -2,   1,  -1,   3,  -3,   1,  -1,   4,  -4,   0,   2,  -2,
       1,   5,  -1,  -5,   6,  -6,   1,  -1,   7,  -7,   1,  -1,   3,  -3,   8,  -8,
       2,  -2,   1,  -1,   9,  -9,   1,  -1,  10, -10,   4,  -4,  11,   1, -11,  -1,
       2,  -2,   1,  12, -12,  -1,  13, -13,   1,  -1,   3,  -3,  14,   5, -14,  -5,
     -15,  15,  -1,   1,   2,  -2,  16, -16,   1,  17, -17,  -1,   6,  -6,  18, -18,
       2,  -2, -19,  19,  -3,   1,   3,  -1,   4,  20,  -4,   1, -21,  21,   1,   2,
      -1,  -7,   7,  -2,  22, -22,  23,   2, -23,  -2,   1,  -1, -24,  24, -25,  25,
      -8, -26,  26,   8, -27,  27,   5,   3,  -3,  -5,  -4,  28, -28,   4,  29, -29,
       1,  -1,  -2, -30,  30,   2,   9,  -9, -31,  31,   2,  -2, -32,   3,  32, -33,
      -3,   1,  33, -34,  -1,  34, -35,  35, -10,  10,  -6,  36,   6, -36,  37, -37,
      -5,  38,   1, -38,  -1,   3,  39, -39,  -1,  40,   5,   1, -40,  -3,   2, -11,
     -41,  -2,   1,  11,  -3,  -4,  41,   3,  42,   4,  -1, -43, -42,  43,   1, -44,
      45,  -1,  44, -45,  -7,   7, -46,   1, -12,   2,   1, -47,  46,  12,  47,  48,
      -2,  -1, -48,  49,  -1, -50, -49,  50,  -6, -51,  51,  52, -13,  53,  -4,   4,
       6,  13, -53, -52, -54,  55,  54, -55, -56,  -2,   2,  -8,  56,   1,  -3,  -1,
       2,  58,   3,   8,  -2,  57, -58, -60, -59, -57,  -3,  60,  59, -14,   3,  14}
},{
    // MapTab7
    2,  // eob_sym
    38, // esc_sym
    // run table
    {1,  1,  0,  2,  2,  1,  1,  3,  3,  4,  4,  5,  5,  1,  1,  6,
     6,  2,  2,  7,  7,  8,  8,  1,  1,  3,  3,  9,  9, 10, 10,  1,
     1,  2,  2,  4,  4, 11,  0, 11, 12, 12, 13, 13,  1,  1,  5,  5,
    14, 14, 15, 16, 15, 16,  3,  3,  1,  6,  1,  6,  2,  2,  7,  7,
     8,  8, 17, 17,  1,  1,  4,  4, 18, 18,  2,  2,  1, 19,  1, 20,
    19, 20, 21, 21,  3,  3, 22, 22,  5,  5, 24,  1,  1, 23,  9, 23,
    24,  9,  2,  2, 10,  1,  1, 10,  6,  6, 25,  4,  4, 25,  7,  7,
    26,  8,  1,  8,  3,  1, 26,  3, 11, 11, 27, 27,  2, 28,  1,  2,
    28,  1, 12, 12,  5,  5, 29, 13, 13, 29, 32,  1,  1, 33, 31, 30,
    32,  4, 30, 33,  4, 31,  3, 14,  1,  1,  3, 34, 34,  2,  2, 14,
     6,  6, 35, 36, 35, 36,  1, 15,  1, 16, 16, 15,  7,  9,  7,  9,
    37,  8,  8, 37,  1,  1, 39,  2, 38, 39,  2, 40,  5, 38, 40,  5,
     3,  3,  4,  4, 10, 10,  1,  1,  1,  1, 41,  2, 41,  2,  6,  6,
     1,  1, 11, 42, 11, 43,  3, 42,  3, 17,  4, 43,  1, 17,  7,  1,
     8, 44,  4,  7, 44,  5,  8,  2,  5,  1,  2, 48, 45,  1, 12, 45,
    12, 48, 13, 13,  1,  9,  9, 46,  1, 46, 47, 47, 49, 18, 18, 49},

    // value table
    { 1,  -1,   0,   1,  -1,   2,  -2,   1,  -1,   1,  -1,   1,  -1,   3,  -3,   1,
     -1,  -2,   2,   1,  -1,   1,  -1,   4,  -4,  -2,   2,   1,  -1,   1,  -1,   5,
     -5,  -3,   3,   2,  -2,   1,   0,  -1,   1,  -1,   1,  -1,   6,  -6,   2,  -2,
      1,  -1,   1,   1,  -1,  -1,  -3,   3,   7,   2,  -7,  -2,  -4,   4,   2,  -2,
      2,  -2,   1,  -1,   8,  -8,   3,  -3,   1,  -1,  -5,   5,   9,   1,  -9,   1,
     -1,  -1,   1,  -1,  -4,   4,   1,  -1,   3,  -3,   1, -10,  10,   1,   2,  -1,
     -1,  -2,   6,  -6,   2,  11, -11,  -2,   3,  -3,   1,  -4,   4,  -1,   3,  -3,
      1,   3,  12,  -3,  -5, -12,  -1,   5,   2,  -2,   1,  -1,  -7,   1,  13,   7,
     -1, -13,   2,  -2,   4,  -4,   1,   2,  -2,  -1,   1,  14, -14,   1,   1,   1,
     -1,  -5,  -1,  -1,   5,  -1,  -6,   2, -15,  15,   6,   1,  -1,  -8,   8,  -2,
     -4,   4,   1,   1,  -1,  -1,  16,   2, -16,  -2,   2,  -2,   4,   3,  -4,  -3,
     -1,  -4,   4,   1, -17,  17,  -1,  -9,   1,   1,   9,   1,  -5,  -1,  -1,   5,
     -7,   7,   6,  -6,   3,  -3,  18, -18,  19, -19,   1, -10,  -1,  10,  -5,   5,
     20, -20,  -3,   1,   3,   1,   8,  -1,  -8,   2,   7,  -1, -21,  -2,   5,  21,
      5,  -1,  -7,  -5,   1,  -6,  -5, -11,   6,  22,  11,   1,   1, -22,  -3,  -1,
      3,  -1,   3,  -3, -23,   4,  -4,   1,  23,  -1,   1,  -1,   1,  -2,   2,  -1}
},{
    // MapTab8
    4,  // eob_sym
    11, // esc_sym
    // run table
    {1,  1,  1,  1,  0,  2,  2,  1,  1,  3,  3,  0,  1,  1,  2,  2,
     4,  4,  1,  1,  5,  5,  1,  1,  2,  2,  3,  3,  6,  6,  1,  1,
     7,  7,  8,  1,  8,  2,  2,  1,  4,  4,  1,  3,  1,  3,  9,  9,
     2,  2,  1,  5,  1,  5, 10, 10,  1,  1, 11, 11,  3,  6,  3,  4,
     4,  6,  2,  2,  1, 12,  1, 12,  7, 13,  7, 13,  1,  1,  8,  8,
     2,  2, 14, 14, 16, 15, 16,  5,  5,  1,  3, 15,  1,  3,  4,  4,
     1,  1, 17, 17,  2,  2,  6,  6,  1, 18,  1, 18, 22, 21, 22, 21,
    25, 24, 25, 19,  9, 20,  9, 23, 19, 24, 20,  3, 23,  7,  3,  1,
     1,  7, 28, 26, 29,  5, 28, 26,  5,  8, 29,  4,  8, 27,  2,  2,
     4, 27,  1,  1, 10, 36, 10, 33, 33, 36, 30,  1, 32, 32,  1, 30,
     6, 31, 31, 35,  3,  6, 11, 11,  3,  2, 35,  2, 34,  1, 34,  1,
    37, 37, 12,  7, 12,  5, 41,  5,  4,  7,  1,  8, 13,  4,  1, 41,
    13, 38,  8, 38,  9,  1, 40, 40,  9,  1, 39,  2,  2, 49, 39, 42,
     3,  3, 14, 16, 49, 14, 16, 42, 43, 43,  6,  6, 15,  1,  1, 15,
    44, 44,  1,  1, 50, 48,  4,  5,  4,  7,  5,  2, 10, 10, 48,  7,
    50, 45,  2,  1, 45,  8,  8,  1, 46, 46,  3, 47, 47,  3,  1,  1},

    // value table
    { 1,  -1,   2,  -2,   0,   1,  -1,   3,  -3,   1,  -1,   0,   4,  -4,   2,  -2,
      1,  -1,   5,  -5,   1,  -1,   6,  -6,   3,  -3,   2,  -2,   1,  -1,   7,  -7,
      1,  -1,   1,   8,  -1,   4,  -4,  -8,   2,  -2,   9,   3,  -9,  -3,   1,  -1,
      5,  -5,  10,   2, -10,  -2,   1,  -1,  11, -11,   1,  -1,  -4,   2,   4,   3,
     -3,  -2,   6,  -6,  12,   1, -12,  -1,   2,   1,  -2,  -1,  13, -13,   2,  -2,
      7,  -7,   1,  -1,   1,   1,  -1,   3,  -3,  14,   5,  -1, -14,  -5,   4,  -4,
     15, -15,   1,  -1,   8,  -8,  -3,   3,  16,   1, -16,  -1,   1,   1,  -1,  -1,
      1,   1,  -1,   1,   2,   1,  -2,   1,  -1,  -1,  -1,   6,  -1,   3,  -6,  17,
    -17,  -3,   1,   1,   1,   4,  -1,  -1,  -4,   3,  -1,   5,  -3,  -1,  -9,   9,
     -5,   1,  18, -18,   2,   1,  -2,   1,  -1,  -1,   1,  19,  -1,   1, -19,  -1,
      4,   1,  -1,   1,   7,  -4,  -2,   2,  -7,  10,  -1, -10,   1,  20,  -1, -20,
      1,  -1,   2,   4,  -2,   5,   1,  -5,   6,  -4,  21,   4,   2,  -6, -21,  -1,
     -2,   1,  -4,  -1,  -3,  22,  -1,   1,   3, -22,  -1,  11, -11,   1,   1,   1,
      8,  -8,   2,   2,  -1,  -2,  -2,  -1,   1,  -1,  -5,   5,   2,  23, -23,  -2,
      1,  -1,  24, -24,  -1,  -1,   7,   6,  -7,   5,  -6,  12,  -3,   3,   1,  -5,
      1,   1, -12,  25,  -1,  -5,   5, -25,  -1,   1,   9,   1,  -1,  -9,  26, -26}
}
};


} // End of namespace Indeo
} // End of namespace Image
