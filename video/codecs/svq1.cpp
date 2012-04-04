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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// Sorenson Video 1 Codec
// Based off ffmpeg's SVQ1 decoder (written by Mike Melanson)

#include "video/codecs/svq1.h"
#include "video/codecs/svq1_cb.h"
#include "video/codecs/svq1_vlc.h"

#include "common/stream.h"
#include "common/bitstream.h"
#include "common/rect.h"
#include "common/system.h"
#include "common/debug.h"
#include "common/textconsole.h"

#include "graphics/yuv_to_rgb.h"

namespace Video {

#define SVQ1_BLOCK_SKIP     0
#define SVQ1_BLOCK_INTER    1
#define SVQ1_BLOCK_INTER_4V 2
#define SVQ1_BLOCK_INTRA    3

struct VLC {
	int32 bits;
	int16 (*table)[2]; // code, bits
	int32 table_size;
	int32 table_allocated;
};

/**
 * parses a vlc code, faster then get_vlc()
 * @param bits is the number of bits which will be read at once, must be
 *             identical to nb_bits in init_vlc()
 * @param max_depth is the number of times bits bits must be read to completely
 *                  read the longest vlc code
 *                  = (max_vlc_length + bits - 1) / bits
 */
static int getVlc2(Common::BitStream *s, int16 (*table)[2], int bits, int maxDepth) {
	int index = s->getBits(bits);
	int code = table[index][0];
	int n = table[index][1];

	if (maxDepth > 1 && n < 0) {
		index = s->getBits(-n) + code;
		code = table[index][0];
		n = table[index][1];

		if(maxDepth > 2 && n < 0) {
			index = s->getBits(-n) + code;
			code = table[index][0];
			n = table[index][1];
		}
	}

	return code;
}

static int allocTable(VLC *vlc, int size, int use_static) {
	int index;
	int16 (*temp)[2] = NULL;
	index = vlc->table_size;
	vlc->table_size += size;
	if (vlc->table_size > vlc->table_allocated) {
		if(use_static)
			error("SVQ1 cant do anything, init_vlc() is used with too little memory");
		vlc->table_allocated += (1 << vlc->bits);
		temp = (int16 (*)[2])realloc(vlc->table, sizeof(int16 *) * 2 * vlc->table_allocated);
		if (!temp) {
			free(vlc->table);
			vlc->table = NULL;
			return -1;
		}
		vlc->table = temp;
	}
	return index;
}

static VLC svq1_block_type;
static VLC svq1_motion_component;
static VLC svq1_intra_multistage[6];
static VLC svq1_inter_multistage[6];
static VLC svq1_intra_mean;
static VLC svq1_inter_mean;

static int svq1DecodeBlockIntra(Common::BitStream *s, uint8 *pixels, int pitch) {
	uint8 *list[63];
	uint32 *dst;
	int entries[6];
	int i, j, m, n;
	int mean, stages;
	unsigned int x, y, width, height, level;
	uint32 n1, n2, n3, n4;

	// initialize list for breadth first processing of vectors
	list[0] = pixels;

	// recursively process vector
	for (i=0, m=1, n=1, level=5; i < n; i++) {
		// SVQ1_PROCESS_VECTOR()
		for (; level > 0; i++) {
			// process next depth
			if (i == m) {
				m = n;
				if (--level == 0)
					break;
			}
			// divide block if next bit set
			if (s->getBit() == 0)
				break;
			// add child nodes
			list[n++] = list[i];
			list[n++] = list[i] + (((level & 1) ? pitch : 1) << ((level / 2) + 1));
		}

		// destination address and vector size
		dst = (uint32 *) list[i];
		width = 1 << ((4 + level) /2);
		height = 1 << ((3 + level) /2);

		// get number of stages (-1 skips vector, 0 for mean only)
		stages = getVlc2(s, svq1_intra_multistage[level].table, 3, 3) - 1;

		if (stages == -1) {
			for (y=0; y < height; y++) {
				memset (&dst[y*(pitch / 4)], 0, width);
			}
		continue; // skip vector
		}

		if ((stages > 0) && (level >= 4)) {
			warning("Error (svq1_decode_block_intra): invalid vector: stages=%i level=%i", stages, level);
		return -1; // invalid vector
		}

		mean = getVlc2(s, svq1_intra_mean.table, 8, 3);

		if (stages == 0) {
			for (y=0; y < height; y++) {
				memset (&dst[y*(pitch / 4)], mean, width);
			}
		} else {
			// SVQ1_CALC_CODEBOOK_ENTRIES(svq1_intra_codebooks);
			const uint32 *codebook = (const uint32 *) svq1_intra_codebooks[level];
			uint32 bit_cache = s->getBits(4*stages);
			// calculate codebook entries for this vector
			for (j=0; j < stages; j++) {
				entries[j] = (((bit_cache >> (4*(stages - j - 1))) & 0xF) + 16*j) << (level + 1);
			}
			mean -= (stages * 128);
			n4    = ((mean + (mean >> 31)) << 16) | (mean & 0xFFFF);

			// SVQ1_DO_CODEBOOK_INTRA()
			for (y=0; y < height; y++) {
				for (x=0; x < (width / 4); x++, codebook++) {
					n1 = n4;
					n2 = n4;
					// SVQ1_ADD_CODEBOOK()
					// add codebook entries to vector
					for (j=0; j < stages; j++) {
						n3  = codebook[entries[j]] ^ 0x80808080;
						n1 += ((n3 & 0xFF00FF00) >> 8);
						n2 +=  (n3 & 0x00FF00FF);
					}

					// clip to [0..255]
					if (n1 & 0xFF00FF00) {
						n3  = ((( n1 >> 15) & 0x00010001) | 0x01000100) - 0x00010001;
						n1 += 0x7F007F00;
						n1 |= (((~n1 >> 15) & 0x00010001) | 0x01000100) - 0x00010001;
						n1 &= (n3 & 0x00FF00FF);
					}

					if (n2 & 0xFF00FF00) {
						n3  = ((( n2 >> 15) & 0x00010001) | 0x01000100) - 0x00010001;
						n2 += 0x7F007F00;
						n2 |= (((~n2 >> 15) & 0x00010001) | 0x01000100) - 0x00010001;
						n2 &= (n3 & 0x00FF00FF);
					}

					// store result
					dst[x] = (n1 << 8) | n2;
				}
				dst += (pitch / 4);
			}
		}
	}

	return 0;
}

static int svq1DecodeBlockNonIntra(Common::BitStream *s, uint8 *pixels, int pitch) {
	uint8 *list[63];
	uint32 *dst;
	int entries[6];
	int i, j, m, n;
	int mean, stages;
	int x, y, width, height, level;
	uint32 n1, n2, n3, n4;

	// initialize list for breadth first processing of vectors
	list[0] = pixels;

	// recursively process vector
	for (i=0, m=1, n=1, level=5; i < n; i++) {
		// SVQ1_PROCESS_VECTOR()
		for (; level > 0; i++) {
			// process next depth
			if (i == m) {
				m = n;
				if (--level == 0)
					break;
			}
			// divide block if next bit set
			if (s->getBit() == 0)
				break;
			// add child nodes
			list[n++] = list[i];
			list[n++] = list[i] + (((level & 1) ? pitch : 1) << ((level / 2) + 1));
		}

		// destination address and vector size
		dst = (uint32 *) list[i];
		width = 1 << ((4 + level) /2);
		height = 1 << ((3 + level) /2);

		// get number of stages (-1 skips vector, 0 for mean only)
		stages = getVlc2(s, svq1_inter_multistage[level].table, 3, 2) - 1;

		if (stages == -1) continue; // skip vector

		if ((stages > 0) && (level >= 4)) {
			warning("Error (svq1_decode_block_non_intra): invalid vector: stages=%i level=%i", stages, level);
			return -1;        // invalid vector
		}

		mean = getVlc2(s, svq1_inter_mean.table, 9, 3) - 256;

		// SVQ1_CALC_CODEBOOK_ENTRIES(svq1_inter_codebooks);
		const uint32 *codebook = (const uint32 *) svq1_inter_codebooks[level];
		uint32 bit_cache = s->getBits(4*stages);
		// calculate codebook entries for this vector
		for (j=0; j < stages; j++) {
			entries[j] = (((bit_cache >> (4*(stages - j - 1))) & 0xF) + 16*j) << (level + 1);
		}
		mean -= (stages * 128);
		n4 = ((mean + (mean >> 31)) << 16) | (mean & 0xFFFF);

		// SVQ1_DO_CODEBOOK_NONINTRA()
		for (y=0; y < height; y++) {
			for (x=0; x < (width / 4); x++, codebook++) {
				n3 = dst[x];
				// add mean value to vector
				n1 = ((n3 & 0xFF00FF00) >> 8) + n4;
				n2 =  (n3 & 0x00FF00FF)          + n4;
				//SVQ1_ADD_CODEBOOK()
				// add codebook entries to vector
				for (j=0; j < stages; j++) {
					n3  = codebook[entries[j]] ^ 0x80808080;
					n1 += ((n3 & 0xFF00FF00) >> 8);
					n2 +=  (n3 & 0x00FF00FF);
				}

				// clip to [0..255]
				if (n1 & 0xFF00FF00) {
					n3  = ((( n1 >> 15) & 0x00010001) | 0x01000100) - 0x00010001;
					n1 += 0x7F007F00;
					n1 |= (((~n1 >> 15) & 0x00010001) | 0x01000100) - 0x00010001;
					n1 &= (n3 & 0x00FF00FF);
				}

				if (n2 & 0xFF00FF00) {
					n3  = ((( n2 >> 15) & 0x00010001) | 0x01000100) - 0x00010001;
					n2 += 0x7F007F00;
					n2 |= (((~n2 >> 15) & 0x00010001) | 0x01000100) - 0x00010001;
					n2 &= (n3 & 0x00FF00FF);
				}

				// store result
				dst[x] = (n1 << 8) | n2;
			}
			dst += (pitch / 4);
		}
	}
	return 0;
}

// median of 3
static inline int mid_pred(int a, int b, int c) {
	if (a > b) {
		if (c > b) {
			if (c > a) b = a;
			else b = c;
		}
	} else {
		if (b > c) {
			if (c > a) b = c;
			else b = a;
		}
	}
	return b;
}

static int svq1DecodeMotionVector(Common::BitStream *s, Common::Point *mv, Common::Point **pmv) {
	for (int i=0; i < 2; i++) {
		// get motion code
		int diff = getVlc2(s, svq1_motion_component.table, 7, 2);
		if (diff < 0)
			return -1;
		else if (diff) {
			if (s->getBit()) diff= -diff;
		}

		// add median of motion vector predictors and clip result
		if (i == 1)
			mv->y = ((diff + mid_pred(pmv[0]->y, pmv[1]->y, pmv[2]->y)) << 26) >> 26;
		else
			mv->x = ((diff + mid_pred(pmv[0]->x, pmv[1]->x, pmv[2]->x)) << 26) >> 26;
	}

	return 0;
}

static void svq1SkipBlock(uint8 *current, uint8 *previous, int pitch, int x, int y) {
	uint8 *src;
	uint8 *dst;

	src = &previous[x + y*pitch];
	dst = current;

	for (int i = 0; i < 16; i++) {
		memcpy(dst, src, 16);
		src += pitch;
		dst += pitch;
	}
}

static int svq1MotionInterBlock(Common::BitStream *ss,
                                uint8 *current, uint8 *previous, int pitch,
                                Common::Point *motion, int x, int y) {
	uint8 *src;
	uint8 *dst;
	Common::Point mv;
	Common::Point *pmv[3];
	int result;

	// predict and decode motion vector
	pmv[0] = &motion[0];
	if (y == 0) {
		pmv[1] = pmv[2] = pmv[0];
	} else {
		pmv[1] = &motion[(x / 8) + 2];
		pmv[2] = &motion[(x / 8) + 4];
	}

	result = svq1DecodeMotionVector(ss, &mv, pmv);

	if (result != 0)
		return result;

	motion[0].x                =
	motion[(x / 8) + 2].x      =
	motion[(x / 8) + 3].x      = mv.x;
	motion[0].y                =
	motion[(x / 8) + 2].y      =
	motion[(x / 8) + 3].y      = mv.y;

	if(y + (mv.y >> 1)<0)
		mv.y= 0;
	if(x + (mv.x >> 1)<0)
		mv.x= 0;

#if 0
	int w = (s->width+15)&~15;
	int h = (s->height+15)&~15;
	if(x + (mv.x >> 1)<0 || y + (mv.y >> 1)<0 || x + (mv.x >> 1) + 16 > w || y + (mv.y >> 1) + 16> h)
		debug(1, "%d %d %d %d", x, y, x + (mv.x >> 1), y + (mv.y >> 1));
#endif

	src = &previous[(x + (mv.x >> 1)) + (y + (mv.y >> 1))*pitch];
	dst = current;

	// FIXME
	//MpegEncContext *s
	//s->dsp.put_pixels_tab[0][((mv.y & 1) << 1) | (mv.x & 1)](dst,src,pitch,16);

	return 0;
}

static int svq1MotionInter4vBlock(Common::BitStream *ss,
                                  uint8 *current, uint8 *previous, int pitch,
                                  Common::Point *motion, int x, int y) {
	uint8 *src;
	uint8 *dst;
	Common::Point mv;
	Common::Point *pmv[4];
	int i, result;

	// predict and decode motion vector (0)
	pmv[0] = &motion[0];
	if (y == 0) {
		pmv[1] = pmv[2] = pmv[0];
	} else {
		pmv[1] = &motion[(x / 8) + 2];
		pmv[2] = &motion[(x / 8) + 4];
	}

	result = svq1DecodeMotionVector(ss, &mv, pmv);

	if (result != 0)
		return result;

	// predict and decode motion vector (1)
	pmv[0] = &mv;
	if (y == 0) {
		pmv[1] = pmv[2] = pmv[0];
	} else {
		pmv[1] = &motion[(x / 8) + 3];
	}
	result = svq1DecodeMotionVector(ss, &motion[0], pmv);

	if (result != 0)
		return result;

	// predict and decode motion vector (2)
	pmv[1] = &motion[0];
	pmv[2] = &motion[(x / 8) + 1];

	result = svq1DecodeMotionVector(ss, &motion[(x / 8) + 2], pmv);

	if (result != 0)
		return result;

	// predict and decode motion vector (3)
	pmv[2] = &motion[(x / 8) + 2];
	pmv[3] = &motion[(x / 8) + 3];

	result = svq1DecodeMotionVector(ss, pmv[3], pmv);

	if (result != 0)
		return result;

	// form predictions
	for (i=0; i < 4; i++) {
		int mvx = pmv[i]->x + (i&1)*16;
		int mvy = pmv[i]->y + (i>>1)*16;

		///XXX /FIXME clipping or padding?
		if(y + (mvy >> 1)<0)
			mvy = 0;
		if(x + (mvx >> 1)<0)
			mvx = 0;

#if 0
		int w = (s->width+15)&~15;
		int h = (s->height+15)&~15;
		if(x + (mvx >> 1)<0 || y + (mvy >> 1)<0 || x + (mvx >> 1) + 8 > w || y + (mvy >> 1) + 8> h)
			debug(1, "%d %d %d %d", x, y, x + (mvx >> 1), y + (mvy >> 1));
#endif
		src = &previous[(x + (mvx >> 1)) + (y + (mvy >> 1))*pitch];
		dst = current;

		// FIXME
		//MpegEncContext *s
		//s->dsp.put_pixels_tab[1][((mvy & 1) << 1) | (mvx & 1)](dst,src,pitch,8);

		// select next block
		if (i & 1) {
			current  += 8*(pitch - 1);
		} else {
			current  += 8;
		}
	}

	return 0;
}

static int svq1DecodeDeltaBlock(Common::BitStream *ss,
                        uint8 *current, uint8 *previous, int pitch,
                        Common::Point *motion, int x, int y) {
	uint32 block_type;
	int result = 0;

	// get block type
	block_type = getVlc2(ss, svq1_block_type.table, 2, 2);

	// reset motion vectors
	if (block_type == SVQ1_BLOCK_SKIP || block_type == SVQ1_BLOCK_INTRA) {
		motion[0].x                 =
		motion[0].y                 =
		motion[(x / 8) + 2].x =
		motion[(x / 8) + 2].y =
		motion[(x / 8) + 3].x =
		motion[(x / 8) + 3].y = 0;
	}

	switch (block_type) {
	case SVQ1_BLOCK_SKIP:
		svq1SkipBlock(current, previous, pitch, x, y);
		break;

	case SVQ1_BLOCK_INTER:
		result = svq1MotionInterBlock(ss, current, previous, pitch, motion, x, y);
		if (result != 0) {
			warning("Error in svq1MotionInterBlock %i", result);
			break;
		}
		result = svq1DecodeBlockNonIntra(ss, current, pitch);
		break;

	case SVQ1_BLOCK_INTER_4V:
		result = svq1MotionInter4vBlock(ss, current, previous, pitch, motion, x, y);
		if (result != 0) {
			warning("Error in svq1MotionInter4vBlock %i", result);
			break;
		}
		result = svq1DecodeBlockNonIntra(ss, current, pitch);
		break;

	case SVQ1_BLOCK_INTRA:
		result = svq1DecodeBlockIntra(ss, current, pitch);
		break;
	}

	return result;
}

#define GET_DATA(v, table, i, wrap, size)\
{\
	const uint8 *ptr = (const uint8 *)table + i * wrap;\
	switch(size) {\
		case 1:\
			v = *(const uint8 *)ptr;\
			break;\
		case 2:\
			v = *(const uint16 *)ptr;\
			break;\
		default:\
			v = *(const uint32 *)ptr;\
			break;\
	}\
}

static int build_table(VLC *vlc, int table_nb_bits,
                       int nb_codes,
                       const void *bits, int bits_wrap, int bits_size,
                       const void *codes, int codes_wrap, int codes_size,
                       const void *symbols, int symbols_wrap, int symbols_size,
                       int code_prefix, int n_prefix, int flags)
{
	int i, j, k, n, table_size, table_index, nb, n1, index, code_prefix2, symbol;
	uint32 code;
	int16 (*table)[2];

	table_size = 1 << table_nb_bits;
	table_index = allocTable(vlc, table_size, flags & 4);
	if (table_index < 0)
		return -1;
	table = &vlc->table[table_index];

	for(i = 0; i < table_size; i++) {
		table[i][1] = 0; //bits
		table[i][0] = -1; //codes
	}

	// first pass: map codes and compute auxillary table sizes
	for(i = 0; i < nb_codes; i++) {
		GET_DATA(n, bits, i, bits_wrap, bits_size);
		GET_DATA(code, codes, i, codes_wrap, codes_size);
		// we accept tables with holes
		if (n <= 0)
			continue;
		if (!symbols)
			symbol = i;
		else
			GET_DATA(symbol, symbols, i, symbols_wrap, symbols_size);
		// if code matches the prefix, it is in the table
		n -= n_prefix;
		if(flags & 2)
			code_prefix2= code & (n_prefix>=32 ? 0xffffffff : (1 << n_prefix)-1);
		else
			code_prefix2= code >> n;
		if (n > 0 && code_prefix2 == code_prefix) {
			if (n <= table_nb_bits) {
				// no need to add another table
				j = (code << (table_nb_bits - n)) & (table_size - 1);
				nb = 1 << (table_nb_bits - n);
				for(k = 0; k < nb; k++) {
					if(flags & 2)
						j = (code >> n_prefix) + (k<<n);
					if (table[j][1] /*bits*/ != 0) {
						error("SVQ1 incorrect codes");
						return -1;
					}
					table[j][1] = n; //bits
					table[j][0] = symbol;
					j++;
				}
			} else {
				n -= table_nb_bits;
				j = (code >> ((flags & 2) ? n_prefix : n)) & ((1 << table_nb_bits) - 1);
				// compute table size
				n1 = -table[j][1]; //bits
				if (n > n1)
					n1 = n;
				table[j][1] = -n1; //bits
			}
		}
	}

	// second pass : fill auxillary tables recursively
	for(i = 0;i < table_size; i++) {
		n = table[i][1]; //bits
		if (n < 0) {
			n = -n;
			if (n > table_nb_bits) {
				n = table_nb_bits;
				table[i][1] = -n; //bits
			}
			index = build_table(vlc, n, nb_codes,
			                    bits, bits_wrap, bits_size,
			                    codes, codes_wrap, codes_size,
			                    symbols, symbols_wrap, symbols_size,
			                    (flags & 2) ? (code_prefix | (i << n_prefix)) : ((code_prefix << table_nb_bits) | i),
			                    n_prefix + table_nb_bits, flags);
 			if (index < 0)
				return -1;
			// note: realloc has been done, so reload tables
			table = &vlc->table[table_index];
			table[i][0] = index; //code
		}
	}
	return table_index;
}

/* Build VLC decoding tables suitable for use with get_vlc().

   'nb_bits' set thee decoding table size (2^nb_bits) entries. The
   bigger it is, the faster is the decoding. But it should not be too
   big to save memory and L1 cache. '9' is a good compromise.

   'nb_codes' : number of vlcs codes

   'bits' : table which gives the size (in bits) of each vlc code.

   'codes' : table which gives the bit pattern of of each vlc code.

   'symbols' : table which gives the values to be returned from get_vlc().

   'xxx_wrap' : give the number of bytes between each entry of the
   'bits' or 'codes' tables.

   'xxx_size' : gives the number of bytes of each entry of the 'bits'
   or 'codes' tables.

   'wrap' and 'size' allows to use any memory configuration and types
   (byte/word/long) to store the 'bits', 'codes', and 'symbols' tables.

   'use_static' should be set to 1 for tables, which should be freed
   with av_free_static(), 0 if free_vlc() will be used.
*/
void initVlcSparse(VLC *vlc, int nb_bits, int nb_codes,
		const void *bits, int bits_wrap, int bits_size,
		const void *codes, int codes_wrap, int codes_size,
		const void *symbols, int symbols_wrap, int symbols_size) {
	vlc->bits = nb_bits;

	if(vlc->table_size && vlc->table_size == vlc->table_allocated) {
		return;
	} else if(vlc->table_size) {
		error("called on a partially initialized table");
	}

	if (build_table(vlc, nb_bits, nb_codes,
	                bits, bits_wrap, bits_size,
	                codes, codes_wrap, codes_size,
	                symbols, symbols_wrap, symbols_size,
	                0, 0, 4 | 2) < 0) {
		free(&vlc->table);
		return; // Error
	}

	if(vlc->table_size != vlc->table_allocated)
		error("SVQ1 needed %d had %d", vlc->table_size, vlc->table_allocated);
}

SVQ1Decoder::SVQ1Decoder(uint16 width, uint16 height) {
	_surface = new Graphics::Surface();
	_surface->create(width, height, g_system->getScreenFormat());

	_current[0] = new byte[width*height];
	_current[1] = new byte[(width/4)*(height/4)];
	_current[2] = new byte[(width/4)*(height/4)];

	_last[0] = 0;
	_last[1] = 0;
	_last[2] = 0;

	// Setup Variable Length Code Tables
	static int16 tableA[6][2];
	svq1_block_type.table = tableA;
	svq1_block_type.table_allocated = 6;
	svq1_block_type.table_size = 0;
	initVlcSparse(&svq1_block_type, 2, 4, 
	        &svq1_block_type_vlc[0][1], 2, 1, 
	        &svq1_block_type_vlc[0][0], 2, 1, NULL, 0, 0);

	static int16 tableB[176][2];
	svq1_motion_component.table = tableB;
	svq1_motion_component.table_allocated = 176;
	svq1_motion_component.table_size = 0;
	initVlcSparse(&svq1_motion_component, 7, 33, 
	        &mvtab[0][1], 2, 1, 
	        &mvtab[0][0], 2, 1, NULL, 0, 0);

	uint16 offset = 0;
	for (uint8 i = 0; i < 6; i++) {
		static const uint8 sizes[2][6] = {{14, 10, 14, 18, 16, 18}, {10, 10, 14, 14, 14, 16}};
		static int16 tableC[168][2];

		svq1_intra_multistage[i].table = &tableC[offset];
		svq1_intra_multistage[i].table_allocated = sizes[0][i];
		svq1_intra_multistage[i].table_size = 0;
		offset += sizes[0][i];
		initVlcSparse(&svq1_intra_multistage[i], 3, 8, 
		         &svq1_intra_multistage_vlc[i][0][1], 2, 1,
		         &svq1_intra_multistage_vlc[i][0][0], 2, 1, NULL, 0, 0);

		svq1_inter_multistage[i].table = &tableC[offset];
		svq1_inter_multistage[i].table_allocated = sizes[1][i];
		svq1_inter_multistage[i].table_size = 0;
		offset += sizes[1][i];
		initVlcSparse(&svq1_inter_multistage[i], 3, 8,
		         &svq1_inter_multistage_vlc[i][0][1], 2, 1,
		         &svq1_inter_multistage_vlc[i][0][0], 2, 1, NULL, 0, 0);
	}

	static int16 tableD[632][2];
	svq1_intra_mean.table = tableD;
	svq1_intra_mean.table_allocated = 632;
	svq1_intra_mean.table_size = 0;
	initVlcSparse(&svq1_intra_mean, 8, 256, 
	        &svq1_intra_mean_vlc[0][1], 4, 2, 
	        &svq1_intra_mean_vlc[0][0], 4, 2, NULL, 0, 0);

	static int16 tableE[1434][2];
	svq1_inter_mean.table = tableE;
	svq1_inter_mean.table_allocated = 1434;
	svq1_inter_mean.table_size = 0;
	initVlcSparse(&svq1_inter_mean, 9, 512, 
	        &svq1_inter_mean_vlc[0][1], 4, 2, 
	        &svq1_inter_mean_vlc[0][0], 4, 2, NULL, 0, 0);
}

SVQ1Decoder::~SVQ1Decoder() {
	_surface->free();
	delete _surface;

	delete[] _current[0];
	delete[] _current[1];
	delete[] _current[2];

	delete[] _last[0];
	delete[] _last[1];
	delete[] _last[2];
}

const Graphics::Surface *SVQ1Decoder::decodeImage(Common::SeekableReadStream *stream) {
	debug(1, "SVQ1Decoder::decodeImage()");

	Common::BitStream32BEMSB frameData(*stream);

	uint32 frameCode = frameData.getBits(22);
	debug(1, " frameCode: %d", frameCode);

	if ((frameCode & ~0x70) || !(frameCode & 0x60)) { // Invalid
		warning("Invalid Image at frameCode");
		return _surface;
	}

	// swap some header bytes (why?)
	//if (frameCode != 0x20) {
	//  uint32 *src = stream;
	//
	//  for (i = 4; i < 8; i++) {
	//    src[i] = ((src[i] << 16) | (src[i] >> 16)) ^ src[7 - i];
	// }
	//}

#if 0
	static const uint16 checksum_table[256] = {
		0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
		0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
		0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
		0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
		0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
		0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
		0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
		0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
		0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
		0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
		0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
		0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
		0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
		0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
		0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
		0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
		0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
		0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
		0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
		0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
		0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
		0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
		0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
		0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
		0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
		0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
		0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
		0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
		0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
		0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
		0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
		0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
	};
#endif

	byte temporalReference = frameData.getBits(8);
	debug(1, " temporalReference: %d", temporalReference);
	const char* types[4] = { "I (Key)", "P (Delta from Previous)", "B (Delta from Next)", "Invalid" };
	byte frameType = frameData.getBits(2);
	debug(1, " frameType: %d = %s Frame", frameType, types[frameType]);
	if (frameType == 0) { // I Frame
		// TODO: Validate checksum if present
		if (frameCode == 0x50 || frameCode == 0x60) {
			uint32 checksum = frameData.getBits(16);
			debug(1, " checksum:0x%02x", checksum);
			//uint16 calculate_packet_checksum (const uint8 *data, const int length) {
			//  int value;
			//for (int i = 0; i < length; i++)
				//	value = checksum_table[data[i] ^ (value >> 8)] ^ ((value & 0xFF) << 8);
		}
	} else if (frameType == 2) { // B Frame
		warning("B Frames not supported by SVQ1 decoder");
		return _surface;
	} else if (frameType == 3) { // Invalid
		warning("Invalid Frame Type");
		return _surface;
	}

	static const uint8 stringXORTable[256] = {
		0x00, 0xD5, 0x7F, 0xAA, 0xFE, 0x2B, 0x81, 0x54,
		0x29, 0xFC, 0x56, 0x83, 0xD7, 0x02, 0xA8, 0x7D,
		0x52, 0x87, 0x2D, 0xF8, 0xAC, 0x79, 0xD3, 0x06,
		0x7B, 0xAE, 0x04, 0xD1, 0x85, 0x50, 0xFA, 0x2F,
		0xA4, 0x71, 0xDB, 0x0E, 0x5A, 0x8F, 0x25, 0xF0,
		0x8D, 0x58, 0xF2, 0x27, 0x73, 0xA6, 0x0C, 0xD9,
		0xF6, 0x23, 0x89, 0x5C, 0x08, 0xDD, 0x77, 0xA2,
		0xDF, 0x0A, 0xA0, 0x75, 0x21, 0xF4, 0x5E, 0x8B,
		0x9D, 0x48, 0xE2, 0x37, 0x63, 0xB6, 0x1C, 0xC9,
		0xB4, 0x61, 0xCB, 0x1E, 0x4A, 0x9F, 0x35, 0xE0,
		0xCF, 0x1A, 0xB0, 0x65, 0x31, 0xE4, 0x4E, 0x9B,
		0xE6, 0x33, 0x99, 0x4C, 0x18, 0xCD, 0x67, 0xB2,
		0x39, 0xEC, 0x46, 0x93, 0xC7, 0x12, 0xB8, 0x6D,
		0x10, 0xC5, 0x6F, 0xBA, 0xEE, 0x3B, 0x91, 0x44,
		0x6B, 0xBE, 0x14, 0xC1, 0x95, 0x40, 0xEA, 0x3F,
		0x42, 0x97, 0x3D, 0xE8, 0xBC, 0x69, 0xC3, 0x16,
		0xEF, 0x3A, 0x90, 0x45, 0x11, 0xC4, 0x6E, 0xBB,
		0xC6, 0x13, 0xB9, 0x6C, 0x38, 0xED, 0x47, 0x92,
		0xBD, 0x68, 0xC2, 0x17, 0x43, 0x96, 0x3C, 0xE9,
		0x94, 0x41, 0xEB, 0x3E, 0x6A, 0xBF, 0x15, 0xC0,
		0x4B, 0x9E, 0x34, 0xE1, 0xB5, 0x60, 0xCA, 0x1F,
		0x62, 0xB7, 0x1D, 0xC8, 0x9C, 0x49, 0xE3, 0x36,
		0x19, 0xCC, 0x66, 0xB3, 0xE7, 0x32, 0x98, 0x4D,
		0x30, 0xE5, 0x4F, 0x9A, 0xCE, 0x1B, 0xB1, 0x64,
		0x72, 0xA7, 0x0D, 0xD8, 0x8C, 0x59, 0xF3, 0x26,
		0x5B, 0x8E, 0x24, 0xF1, 0xA5, 0x70, 0xDA, 0x0F,
		0x20, 0xF5, 0x5F, 0x8A, 0xDE, 0x0B, 0xA1, 0x74,
		0x09, 0xDC, 0x76, 0xA3, 0xF7, 0x22, 0x88, 0x5D,
		0xD6, 0x03, 0xA9, 0x7C, 0x28, 0xFD, 0x57, 0x82,
		0xFF, 0x2A, 0x80, 0x55, 0x01, 0xD4, 0x7E, 0xAB,
		0x84, 0x51, 0xFB, 0x2E, 0x7A, 0xAF, 0x05, 0xD0,
		0xAD, 0x78, 0xD2, 0x07, 0x53, 0x86, 0x2C, 0xF9
	};

	if ((frameCode ^ 0x10) >= 0x50) {
		// Decode embedded string
		Common::String str;
		uint8 stringLen = frameData.getBits(8);
		byte xorVal = stringXORTable[stringLen];

		for (uint16 i = 0; i < stringLen-1; i++) {
			byte data = frameData.getBits(8);
			str += data ^ xorVal;
			xorVal = stringXORTable[data];
		}
		debug(1, " Embedded String of %d Characters: \"%s\"", stringLen, str.c_str());
	}

	byte unk1 = frameData.getBits(2); // Unknown
	debug(1, " unk1: %d", unk1);
	byte unk2 = frameData.getBits(2); // Unknown
	debug(1, " unk2: %d", unk2);
	bool unk3 = frameData.getBit(); // Unknown
	debug(1, " unk3: %d", unk3);

	static const struct { uint w, h; } standardFrameSizes[7] = {
		{ 160, 120 }, // 0
		{ 128,  96 }, // 1
		{ 176, 144 }, // 2
		{ 352, 288 }, // 3
		{ 704, 576 }, // 4
		{ 240, 180 }, // 5
		{ 320, 240 }  // 6
	};

	byte frameSizeCode = frameData.getBits(3);
	debug(1, " frameSizeCode: %d", frameSizeCode);
	uint16 frameWidth, frameHeight;
	if (frameSizeCode == 7) {
		frameWidth = frameData.getBits(12);
		frameHeight = frameData.getBits(12);
	} else {
		frameWidth = standardFrameSizes[frameSizeCode].w;
		frameHeight = standardFrameSizes[frameSizeCode].h;
	}
	debug(1, " frameWidth: %d", frameWidth);
	debug(1, " frameHeight: %d", frameHeight);
	if (frameWidth == 0 || frameHeight == 0) { // Invalid
		warning("Invalid Frame Size");
		return _surface;
	}
	bool checksumPresent = frameData.getBit();
	debug(1, " checksumPresent: %d", checksumPresent);
	if (checksumPresent) {
		bool usePacketChecksum = frameData.getBit();
		debug(1, " usePacketChecksum: %d", usePacketChecksum);
		bool componentChecksumsAfterImageData = frameData.getBit();
		debug(1, " componentChecksumsAfterImageData: %d", componentChecksumsAfterImageData);
		byte unk4 = frameData.getBits(2);
		debug(1, " unk4: %d", unk4);
		if (unk4 != 0)
			warning("Invalid Frame Header in SVQ1 Frame Decode");
	}

	bool unk5 = frameData.getBit();
	debug(1, " unk5: %d", unk5);
	if (unk5) {
		bool unk6 = frameData.getBit();
		debug(1, " unk6: %d", unk6);
		byte unk7 = frameData.getBits(4);
		debug(1, " unk7: %d", unk7);
		bool unk8 = frameData.getBit();
		debug(1, " unk8: %d", unk8);
		byte unk9 = frameData.getBits(2);
		debug(1, " unk9: %d", unk9);
		while (frameData.getBit()) {
			byte unk10 = frameData.getBits(8);
			debug(1, " unk10: %d", unk10);
		}
	}

	if (frameWidth == _surface->w && frameHeight == _surface->h) {
		// Decode Y, U and V component planes
		for (int i = 0; i < 3; i++) {
			int linesize, width, height;
			if (i == 0) {
				// Y Size is width * height
				width  = frameWidth;
				if (width % 16) {
					width /= 16;
					width++;
					width *= 16;
				}
				assert(width % 16 == 0);
				height = frameHeight;
				if (height % 16) {
					height /= 16;
					height++;
					height *= 16;
				}
				assert(height % 16 == 0);
				linesize = width;
			} else {
				// U and V size is width/4 * height/4
				width  = frameWidth/4;
				if (width % 16) {
					width /= 16;
					width++;
					width *= 16;
				}
				assert(width % 16 == 0);
				height = frameHeight/4;
				if (height % 16) {
					height /= 16;
					height++;
					height *= 16;
				}
				assert(height % 16 == 0);
				linesize = width;
			}

			if (frameType == 0) { // I Frame
				// Keyframe (I)
				byte *current = _current[i];
				for (uint16 y = 0; y < height; y += 16) {
					for (uint16 x = 0; x < width; x += 16) {
						if (int result = svq1DecodeBlockIntra(&frameData, &current[x], linesize) != 0) {
							warning("Error in svq1DecodeBlock %i (keyframe)", result);
							return _surface;
						}
					}
					current += 16 * linesize;
				}
			} else {
				// Delta frame (P or B)

				// Prediction Motion Vector
				Common::Point *pmv = new Common::Point[(width/8) + 3];

				byte *previous;
				if(frameType == 2) { // B Frame
					warning("B Frame not supported currently");
					//previous = _next[i];
				} else
					previous = _last[i];

				byte *current = _current[i];
				for (uint16 y = 0; y < height; y += 16) {
					for (uint16 x = 0; x < width; x += 16) {
						if (int result = svq1DecodeDeltaBlock(&frameData, &current[x], previous, linesize, pmv, x, y) != 0) {
							warning("Error in svq1DecodeDeltaBlock %i", result);
							return _surface;
						}
					}

					pmv[0].x = pmv[0].y = 0;

					current += 16*linesize;
				}
				delete[] pmv;
			}
		}

		convertYUV410ToRGB(_surface, _current[0], _current[1], _current[2], frameWidth, frameHeight, frameWidth, frameWidth/2);
	} else
		warning("FrameWidth/Height Sanity Check Failed!");

	return _surface;
}

} // End of namespace Video
