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
	_ctx.show_indeo4_info = false;
	_ctx.b_ref_buf = 3; // buffer 2 is used for scalability mode
}

IndeoDecoderBase::~IndeoDecoderBase() {
	delete _surface;
}

int IndeoDecoderBase::decodeIndeoFrame() {
	// Decode the header
	int err = decodePictureHeader();

	if (!err && _ctx.gop_invalid)
		err = -1;

	if (!err && _ctx.frame_type == IVI4_FRAMETYPE_NULL_LAST) {
		// Returning the previous frame, so exit wth success
		return 0;
	}

	if (!err && _ctx.gop_flags & IVI5_IS_PROTECTED) {
		warning("Password-protected clip");
		err = -1;
	}

	if (!err && !_ctx.planes[0].bands) {
		warning("Color planes not initialized yet");
		err = -1;
	}

	// TODO

	return err;
}

/*------------------------------------------------------------------------*/

int av_image_check_size(unsigned int w, unsigned int h, int log_offset, void *log_ctx) {
	if (((w + 128) * (uint64)(h + 128)) < (INT_MAX / 8))
		return 0;

	error("Picture size %ux%u is invalid", w, h);
}

} // End of namespace Indeo
} // End of namespace Image
