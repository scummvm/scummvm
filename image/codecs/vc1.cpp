/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * WMV3 and VC-1 software video decoder.
 *
 * This decoder is a ScummVM port of the FFmpeg WMV3/VC-1 decoder path
 * used by libavcodec revision c23123630e6a7e645c199599b8ade3fe7e9ab3db:
 * https://git.ffmpeg.org/gitweb/ffmpeg.git/commit/c23123630e6a7e645c199599b8ade3fe7e9ab3db
 * The original code was Copyright
 * (c) 2005 Anonymous, Alex Beregszaszi and Michael Niedermayer,
 * (c) 2006-2007 Konstantin Shishkov,
 * (c) 2011 Mashiat Sarker Shakkhar,
 * (c) 2000-2002 Fabrice Bellard,
 * (c) 2001-2003 BERO <bero@geocities.co.jp>,
 * (c) 2001-2012 Michael Niedermayer <michaelni@gmx.at> et al,
 * (c) 2010 Loren Merritt,
 * (c) 2011 Oskar Arvidsson and
 * (c) 2012 Ronald S. Bultje and distributed under the GNU LGPL
 * version 2.1 or later. The migrated path includes sequence and picture
 * headers, bitplanes, VLC tables, prediction, motion compensation, inverse
 * transforms, overlap and loop filtering, IntraX8, error concealment,
 * reference-frame ordering, and YUV420 output.
 */

#include "image/codecs/vc1.h"
#include "common/array.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "common/ptr.h"
#include "common/rational.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "graphics/yuv_to_rgb.h"
#include "image/codecs/msmpeg4_vc1.h"
#include <limits.h>

namespace Image {

typedef void (*ChromaFunc)(uint8 *dst, const uint8 *src, ptrdiff_t stride, int height, int x, int y);

typedef void (*vc1op_pixels_func)(uint8 *block /*align width (8 or 16)*/, const uint8 *pixels /*align 1*/,
								  ptrdiff_t line_size, int round);

typedef struct WMV3DSP {
	/* vc1 functions */
	void (*vc1_inv_trans_8x8)(int16 *b);
	void (*vc1_inv_trans_8x4)(uint8 *dest, ptrdiff_t stride, int16 *block);
	void (*vc1_inv_trans_4x8)(uint8 *dest, ptrdiff_t stride, int16 *block);
	void (*vc1_inv_trans_4x4)(uint8 *dest, ptrdiff_t stride, int16 *block);
	void (*vc1_inv_trans_8x8_dc)(uint8 *dest, ptrdiff_t stride, int16 *block);
	void (*vc1_inv_trans_8x4_dc)(uint8 *dest, ptrdiff_t stride, int16 *block);
	void (*vc1_inv_trans_4x8_dc)(uint8 *dest, ptrdiff_t stride, int16 *block);
	void (*vc1_inv_trans_4x4_dc)(uint8 *dest, ptrdiff_t stride, int16 *block);
	void (*vc1_v_overlap)(uint8 *src, ptrdiff_t stride);
	void (*vc1_h_overlap)(uint8 *src, ptrdiff_t stride);
	void (*vc1_v_s_overlap)(int16 *top, int16 *bottom);
	void (*vc1_h_s_overlap)(int16 *left, int16 *right, ptrdiff_t left_stride, ptrdiff_t right_stride, int flags);
	void (*vc1_v_loop_filter4)(uint8 *src, ptrdiff_t stride, int pq);
	void (*vc1_h_loop_filter4)(uint8 *src, ptrdiff_t stride, int pq);
	void (*vc1_v_loop_filter8)(uint8 *src, ptrdiff_t stride, int pq);
	void (*vc1_h_loop_filter8)(uint8 *src, ptrdiff_t stride, int pq);
	void (*vc1_v_loop_filter16)(uint8 *src, ptrdiff_t stride, int pq);
	void (*vc1_h_loop_filter16)(uint8 *src, ptrdiff_t stride, int pq);

	/* put 8x8 block with bicubic interpolation and quarterpel precision
	 * last argument is actually round value instead of height
	 */
	vc1op_pixels_func put_vc1_mspel_pixels_tab[2][16];
	vc1op_pixels_func avg_vc1_mspel_pixels_tab[2][16];

	/* This is really one func used in VC-1 decoding */
	ChromaFunc put_no_rnd_vc1_chroma_pixels_tab[3];
	ChromaFunc avg_no_rnd_vc1_chroma_pixels_tab[3];

} WMV3DSP;

void initWMV3DSP(WMV3DSP *dsp);

namespace VC1 {
// Bitstream and VLC support

struct Bits {
	Common::BitStream8MSB *stream;
	int size_in_bits;
};
inline unsigned get_bits(Bits *b, int n) {
	return b->stream->getBits(n);
}
inline unsigned get_bits1(Bits *b) {
	return b->stream->getBit();
}
inline int get_bits_count(const Bits *b) {
	return b->stream->pos();
}
inline int get_bits_left(const Bits *b) {
	return b->size_in_bits - get_bits_count(b);
}
inline void skip_bits(Bits *b, int n) {
	b->stream->skip(n);
}
inline int get_unary(Bits *b, int stop, int max) {
	int i = 0;
	while (i < max && get_bits1(b) != (unsigned)stop)
		++i;
	return i;
}

inline int decode012(Bits *b) {
	return get_bits1(b) ? 1 + get_bits1(b) : 0;
}
inline int decode210(Bits *b) {
	return get_bits1(b) ? 0 : 2 - get_bits1(b);
}
struct VLC {
	struct Node {
		int child[2];
		int symbol;
		Node() : child{-1, -1}, symbol(-1) {}
	};
	Common::Array<Node> nodes;
};
const VLC *makeVLC(int count, const void *lengths, int lengthStride, int lengthSize, const void *codes, int codeStride,
				   int codeSize);
int get_vlc2(Bits *b, const VLC *table);
void initTables();
inline int av_clip(int a, int low, int high) {
	return CLIP(a, low, high);
}
inline int av_clip_uint8(int a) {
	return CLIP(a, 0, 255);
}
inline int av_clip_uintp2(int a, int p) {
	return CLIP(a, 0, (1 << p) - 1);
}
struct Crop {
	uint8 operator[](int x) const { return av_clip_uint8(x); }
};
inline int mid_pred(int a, int b, int c) {
	if (a > b)
		SWAP(a, b);
	return MAX(a, MIN(b, c));
}
#define FFALIGN(x, a) (((x) + (a) - 1) & ~((a) - 1))
enum PictureType { AV_PICTURE_TYPE_I = 1, AV_PICTURE_TYPE_P = 2, AV_PICTURE_TYPE_B = 3, AV_PICTURE_TYPE_BI = 7 };

enum Profile { PROFILE_SIMPLE = 0, PROFILE_MAIN = 1, PROFILE_ADVANCED = 3 };

enum MacroblockType { MB_TYPE_INTRA = 1, MB_TYPE_16x16 = 8, MB_TYPE_SKIP = 131072 };

enum ErrorStatus { ER_MB_ERROR = 14, ER_MB_END = 112 };

static const int CONFIG_GRAY = 0;
static const int AV_CODEC_FLAG_GRAY = 8192;
static const int AVDISCARD_NONKEY = 32;

// Decoder state and picture storage

struct Settings {
	int error_concealment = 3;
	int flags;
	int skip_loop_filter;
	int coded_width;
	int coded_height;
};
struct Planes {
	uint8 *data[3];
	int linesize[3];
	int pict_type;
	int width;
	int height;
	int format;
	int decode_error_flags;
};
struct Picture {
	Planes *f;
	uint8 *data[3];
	int linesize[3];
	int16 (*motion_val[2])[2];
	int8 *ref_index[2];
	uint32 *mb_type;
	int8 *qscale_table;
	Picture *ptr;
	int field_picture;
	int interlaced;
};
typedef void (*PixelOp)(uint8 *, const uint8 *, ptrdiff_t, int);
typedef void (*QpelOp)(uint8 *, const uint8 *, ptrdiff_t);
struct QpelDSP {
	QpelOp put_qpel_pixels_tab[2][16], put_no_rnd_qpel_pixels_tab[2][16], avg_qpel_pixels_tab[2][16];
};
void initQpel(QpelDSP *);
struct HalfDSP {
	PixelOp put_pixels_tab[2][4], put_no_rnd_pixels_tab[2][4], avg_pixels_tab[2][4], avg_no_rnd_pixels_tab[4];
};
struct ChromaDSP {
	ChromaFunc put_h264_chroma_pixels_tab[3], avg_h264_chroma_pixels_tab[3];
};
struct BlockDSP {
	void (*clear_block)(int16 *);
	void (*clear_blocks)(int16 *);
};
struct ClampDSP {
	void (*put_pixels_clamped)(const int16 *, uint8 *, ptrdiff_t);
	void (*put_signed_pixels_clamped)(const int16 *, uint8 *, ptrdiff_t);
	void (*add_pixels_clamped)(const int16 *, uint8 *, ptrdiff_t);
};
struct EdgeDSP {
	void (*emulated_edge_mc)(uint8 *, const uint8 *, ptrdiff_t, ptrdiff_t, int, int, int, int, int, int);
};
inline int median4(int a, int b, int c, int d) {
	if (a < b) {
		if (c < d)
			return (MIN(b, d) + MAX(a, c)) / 2;
		return (MIN(b, c) + MAX(a, d)) / 2;
	}
	if (c < d)
		return (MIN(a, d) + MAX(b, c)) / 2;
	return (MIN(a, c) + MAX(b, d)) / 2;
}

typedef struct ERContext {
	Settings *avctx;

	int (*sad)(void *unused, const uint8 *blk1, const uint8 *blk2, ptrdiff_t stride, int h);

	int *mb_index2xy;
	int mb_num;
	int mb_width, mb_height;
	ptrdiff_t mb_stride;
	ptrdiff_t b8_stride;

	int error_count;
	int error_occurred;
	uint8 *error_status_table;
	uint8 *er_temp_buffer;
	int16 *dc_val[3];
	uint8 *mbskip_table;
	uint8 *mbintra_table;
	int mv[2][4][2];

	Picture cur_pic;
	Picture last_pic;
	Picture next_pic;

	int8 *ref_index[2];
	int16 (*motion_val_base[2])[2];

	uint16 pp_time;
	uint16 pb_time;
	int quarter_sample;
	int partitioned_frame;

	void (*decode_mb)(void *opaque, int ref, int mv_dir, int mv_type, int (*mv)[2][4][2], int mb_x, int mb_y,
					  int mb_intra, int mb_skipped);
	void *opaque;
} ERContext;

void ff_er_add_slice(ERContext *, int, int, int, int, int);
void ff_er_frame_start(ERContext *);
void ff_er_frame_end(ERContext *, int *);
struct PictureState;
void initializeER(PictureState *);
struct PictureState {
	Settings *avctx;
	int ac_pred;
	int b8_stride;
	int chroma_x_shift;
	int end_mb_y;
	int first_slice_line;
	int h_edge_pos;
	int height;
	int width;
	int low_delay;
	int linesize;
	int mb_height;
	int mb_intra;
	int mb_stride;
	int mb_width;
	int mb_x;
	int mb_y;
	int mspel;
	int pict_type;
	int quarter_sample;
	int start_mb_y;
	int uvlinesize;
	int v_edge_pos;
	int y_dc_scale;
	int16 (*ac_val)[16];
	int16 *dc_val;
	uint8 *coded_block;
	uint8 *mbskip_table;
	int block_index[6];
	int block_wrap[6];
	int mv[2][4][2];
	uint8 *dest[3];
	Picture cur_pic;
	Picture last_pic;
	Picture next_pic;
	BlockDSP bdsp;
	ClampDSP idsp;
	HalfDSP hdsp;
	QpelDSP qdsp;
	EdgeDSP vdsp;
	struct {
		uint8 *edge_emu_buffer;
	} sc;
	ERContext er;
};
struct IntraX8Context;
int ff_intrax8_decode_picture(IntraX8Context *, Picture *, Bits *, int *, int *, int, int, int, int);
void ff_init_block_index(PictureState *s);
void ff_update_block_index(PictureState *s, int bits, int lowres, int chroma);
#define B_FRACTION_DEN 256
#define AC_VLC_BITS 9

/** Sequence quantizer mode */
//@{
enum QuantMode {
	QUANT_FRAME_IMPLICIT, ///< Implicitly specified at frame level
	QUANT_FRAME_EXPLICIT, ///< Explicitly specified at frame level
	QUANT_NON_UNIFORM,    ///< Non-uniform quant used for all frames
	QUANT_UNIFORM         ///< Uniform quant used for all frames
};
//@}

/** Where quant can be changed */
//@{
enum DQProfile { DQPROFILE_FOUR_EDGES, DQPROFILE_DOUBLE_EDGES, DQPROFILE_SINGLE_EDGE, DQPROFILE_ALL_MBS };
//@}

/** @name Where quant can be changed
 */
//@{
enum DQSingleEdge { DQSINGLE_BEDGE_LEFT, DQSINGLE_BEDGE_TOP, DQSINGLE_BEDGE_RIGHT, DQSINGLE_BEDGE_BOTTOM };
//@}

/** Which pair of edges is quantized with ALTPQUANT */
//@{
enum DQDoubleEdge {
	DQDOUBLE_BEDGE_TOPLEFT,
	DQDOUBLE_BEDGE_TOPRIGHT,
	DQDOUBLE_BEDGE_BOTTOMRIGHT,
	DQDOUBLE_BEDGE_BOTTOMLEFT
};
//@}

/** MV modes for P-frames */
//@{
enum MVModes { MV_PMODE_1MV_HPEL_BILIN, MV_PMODE_1MV, MV_PMODE_1MV_HPEL, MV_PMODE_MIXED_MV, MV_PMODE_INTENSITY_COMP };
//@}

/** MBMODE for interlaced frame P-picture */
//@{
enum MBModesIntfr {
	MV_PMODE_INTFR_1MV,
	MV_PMODE_INTFR_2MV_FIELD,
	MV_PMODE_INTFR_2MV,
	MV_PMODE_INTFR_4MV_FIELD,
	MV_PMODE_INTFR_4MV,
	MV_PMODE_INTFR_INTRA,
};
//@}

/** @name MV types for B-frames */
//@{
enum BMVTypes { BMV_TYPE_BACKWARD, BMV_TYPE_FORWARD, BMV_TYPE_INTERPOLATED, BMV_TYPE_DIRECT };
//@}

/** @name Block types for P/B-frames */
//@{
enum TransformTypes {
	TT_8X8,
	TT_8X4_BOTTOM,
	TT_8X4_TOP,
	TT_8X4, // both halves
	TT_4X8_RIGHT,
	TT_4X8_LEFT,
	TT_4X8, // both halves
	TT_4X4
};
//@}

enum CodingSet {
	CS_HIGH_MOT_INTRA = 0,
	CS_HIGH_MOT_INTER,
	CS_LOW_MOT_INTRA,
	CS_LOW_MOT_INTER,
	CS_MID_RATE_INTRA,
	CS_MID_RATE_INTER,
	CS_HIGH_RATE_INTRA,
	CS_HIGH_RATE_INTER
};

/** @name Overlap conditions for Advanced Profile */
//@{
enum COTypes { CONDOVER_NONE = 0, CONDOVER_ALL, CONDOVER_SELECT };
//@}

/**
 * FCM Frame Coding Mode
 * @note some content might be marked interlaced
 * but have fcm set to 0 as well (e.g. HD-DVD)
 */
enum FrameCodingMode {
	PROGRESSIVE = 0, ///<  in the bitstream is reported as 00b
	ILACE_FRAME,     ///<  in the bitstream is reported as 10b
	ILACE_FIELD      ///<  in the bitstream is reported as 11b
};

/**
 * Imode types
 * @{
 */
enum Imode { IMODE_RAW, IMODE_NORM2, IMODE_DIFF2, IMODE_NORM6, IMODE_DIFF6, IMODE_ROWSKIP, IMODE_COLSKIP };
/** @} */ // imode defines

/** The VC1 Context
 * @todo Change size wherever another size is more efficient
 * Many members are only used for Advanced Profile
 */
// VC-1 picture context

struct VC1Context : VC1Header {
	PictureState s;
	Bits gb;
	IntraX8Context *x8;
	ChromaDSP h264chroma;
	WMV3DSP vc1dsp;

	/** Simple/Main Profile sequence header */
	//@{
	///< at frame level
	int reserved; ///< reserved
	//@}

	/** Advanced Profile */
	//@{
	///< Decoder parameters
	//@}

	/** Sequence header data for all Profiles
	 * TODO: choose between ints, uint8_ts and monobit flags
	 */
	//@{
	//@}

	/** Frame decoding info for all profiles */
	//@{
	uint8 mv_mode;               ///< MV coding mode
	uint8 mv_mode2;              ///< Secondary MV coding mode (B-frames)
	int k_x;                     ///< Number of bits for MVs (depends on MV range)
	int k_y;                     ///< Number of bits for MVs (depends on MV range)
	int range_x, range_y;        ///< MV range
	uint8 pq, altpq;             ///< Current/alternate frame quantizer scale
	uint8 zz_8x8[4][64];         ///< Zigzag table for TT_8x8, permuted for IDCT
	int left_blk_sh, top_blk_sh; ///< Either 3 or 0, positions of l/t in blk[]
	/** pquant parameters */
	//@{
	uint8 dquantfrm;
	uint8 dqprofile;
	uint8 dqsbedge;
	uint8 dqbilevel;
	//@}
	int dc_table_index;
	/** AC coding set indexes
	 * @see 8.1.1.10, p(1)10
	 */
	//@{
	int c_ac_table_index; ///< Chroma index from ACFRM element
	int y_ac_table_index; ///< Luma index from AC2FRM element
	//@}
	int esc3_level_length;
	int esc3_run_length;
	int ttfrm;               ///< Transform type info present at frame level
	uint8 ttmbf;             ///< Transform type flag
	int *ttblk_base, *ttblk; ///< Transform type at the block level
	int codingset;           ///< index of current table set from 11.8 to use for luma block decoding
	int codingset2;          ///< index of current table set from 11.8 to use for chroma block decoding
	int pqindex;             ///< raw pqindex used in coding set selection
	int a_avail, c_avail;
	uint8 *mb_type_base, *mb_type;

	/** Luma compensation parameters */
	//@{
	uint8 lumscale;
	uint8 lumshift;
	//@}
	int16 bfraction;     ///< Relative position % anchors=> how to scale MVs
	uint8 halfpq;        ///< Uniform quant over image and qp+.5
	uint8 respic;        ///< Frame-level flag for resized images
	int buffer_fullness; ///< HRD info
	/** Ranges:
	 * -# 0 -> [-64n 63.f] x [-32, 31.f]
	 * -# 1 -> [-128, 127.f] x [-64, 63.f]
	 * -# 2 -> [-512, 511.f] x [-128, 127.f]
	 * -# 3 -> [-1024, 1023.f] x [-256, 255.f]
	 */
	uint8 mvrange;        ///< Extended MV range flag
	uint8 pquantizer;     ///< Uniform (over sequence) quantizer in use
	const VLC *cbpcy_vlc; ///< CBPCY VLC table
	int tt_index;         ///< Index for Transform Type tables (to decode TTMB)
	int mv_table_index;
	uint8 *mv_type_mb_plane;                     ///< bitplane for mv_type == (4MV)
	uint8 *direct_mb_plane;                      ///< bitplane for "direct" MBs
	uint8 *forward_mb_plane;                     ///< bitplane for "forward" MBs
	int mv_type_is_raw;                          ///< mv type mb plane is not coded
	int dmb_is_raw;                              ///< direct mb plane is raw
	int fmb_is_raw;                              ///< forward mb plane is raw
	int skip_is_raw;                             ///< skip mb plane is not coded
	uint8 last_luty[2][256], last_lutuv[2][256]; ///< lookup tables used for intensity compensation
	uint8 aux_luty[2][256], aux_lutuv[2][256];   ///< lookup tables used for intensity compensation
	uint8 next_luty[2][256], next_lutuv[2][256]; ///< lookup tables used for intensity compensation
	uint8 (*curr_luty)[256], (*curr_lutuv)[256];
	int last_use_ic, *curr_use_ic, next_use_ic, aux_use_ic;
	int last_interlaced, next_interlaced; ///< whether last_pic, next_pic is interlaced
	int rnd;                              ///< rounding control
	int cbptab;

	/** Frame decoding info for S/M profiles only */
	//@{
	uint8 rangeredfrm; ///< out_sample = CLIP((in_sample-128)*2+128)
	uint8 interpfrm;
	//@}

	/** Frame decoding info for Advanced profile */
	//@{
	int fcm;
	uint8 numpanscanwin;
	uint8 tfcntr;
	uint8 rptfrm, tff, rff;
	uint8 uvsamp;
	uint8 postproc;
	uint8 bit_rate_exponent;
	uint8 buffer_size_exponent;
	uint8 *acpred_plane; ///< AC prediction flags bitplane
	int acpred_is_raw;
	uint8 *over_flags_plane; ///< Overflags bitplane
	int overflg_is_raw;
	uint8 condover;
	//@}

	/** Frame decoding info for interlaced picture */
	uint8 dmvrange; ///< Extended differential MV range flag
	int fourmvswitch;
	int intcomp;
	uint8 lumscale2; ///< for interlaced field P picture
	uint8 lumshift2;
	const VLC *mbmode_vlc;
	const VLC *imv_vlc;
	const VLC *twomvbp_vlc;
	const VLC *fourmvbp_vlc;
	uint8 twomvbp;
	uint8 fourmvbp;
	uint8 *fieldtx_plane;
	int fieldtx_is_raw;
	uint8 zzi_8x8[64];
	uint8 *blk_mv_type_base, *blk_mv_type; ///< 0: frame MV, 1: field MV (interlaced frame)
	uint8 *mv_f_base, *mv_f[2];            ///< 0: MV obtained from same field, 1: opposite field
	uint8 *mv_f_next_base, *mv_f_next[2];
	int field_mode; ///< 1 for interlaced field pictures
	int fptype;
	int second_field;
	int refdist;           ///< distance of the current picture from reference
	int numref;            ///< number of past field pictures used as reference
						   // 0 corresponds to 1 and 1 corresponds to 2 references
	int reffield;          ///< if numref = 0 (1 reference) then reffield decides which
						   // field to use among the two fields from previous frame
	int intcompfield;      ///< which of the two fields to be intensity compensated
						   // 0: both fields, 1: bottom field, 2: top field
	int cur_field_type;    ///< 0: top, 1: bottom
	int ref_field_type[2]; ///< forward and backward reference field type (top or bottom)
	int blocks_off, mb_off;
	int bmvtype;
	int frfd, brfd; ///< reference frame distance (forward or backward)
	int first_pic_header_flag;
	int pic_header_flag;
	int mbmodetab;
	int icbptab;
	int imvtab;
	int twomvbptab;
	int fourmvbptab;

	int new_sprite, two_sprites;
	int p_frame_skipped;
	int bi_type;
	int x8_type;

	int16 (*block)[6][64];
	int n_allocated_blks, cur_blk_idx, left_blk_idx, topleft_blk_idx, top_blk_idx;
	uint32 *cbp_base, *cbp;
	uint8 *is_intra_base, *is_intra;
	int16 (*luma_mv_base)[2], (*luma_mv)[2];
	uint8 bfraction_lut_index; ///< Index for BFRACTION value (see Table 40, reproduced into ff_vc1_bfraction_lut[])

	int end_mb_x; ///< Horizontal macroblock limit (used only by mss2)

	int parse_only; ///< Context is used within parser

	int16 blocks[6][64];
};

void ff_vc1_decode_blocks(VC1Context *v);

void ff_vc1_i_overlap_filter(VC1Context *v);
void ff_vc1_p_overlap_filter(VC1Context *v);
void ff_vc1_i_loop_filter(VC1Context *v);
void ff_vc1_p_loop_filter(VC1Context *v);
void ff_vc1_p_intfr_loop_filter(VC1Context *v);
void ff_vc1_b_intfi_loop_filter(VC1Context *v);

void ff_vc1_mc_1mv(VC1Context *v, int dir);
void ff_vc1_mc_4mv_luma(VC1Context *v, int n, int dir, int avg);
void ff_vc1_mc_4mv_chroma(VC1Context *v, int dir);
void ff_vc1_mc_4mv_chroma4(VC1Context *v, int dir, int dir2, int avg);

void ff_vc1_interp_mc(VC1Context *v);

void ff_vc1_pred_mv(VC1Context *v, int n, int dmv_x, int dmv_y, int mv1, int r_x, int r_y, uint8 *is_intra,
					int pred_flag, int dir);
void ff_vc1_pred_mv_intfr(VC1Context *v, int n, int dmv_x, int dmv_y, int mvn, int r_x, int r_y, int dir);
void ff_vc1_pred_b_mv(VC1Context *v, int dmv_x[2], int dmv_y[2], int direct, int mvtype);
void ff_vc1_pred_b_mv_intfi(VC1Context *v, int n, int *dmv_x, int *dmv_y, int mv1, int *pred_flag);

static inline int scale_mv(int value, int bfrac, int inv, int qs) {
	int n = bfrac;

#if B_FRACTION_DEN == 256
	if (inv)
		n -= 256;
	if (!qs)
		return 2 * ((value * n + 255) >> 9);
	return (value * n + 128) >> 8;
#else
	if (inv)
		n -= B_FRACTION_DEN;
	if (!qs)
		return 2 * ((value * n + B_FRACTION_DEN - 1) / (2 * B_FRACTION_DEN));
	return (value * n + B_FRACTION_DEN / 2) / B_FRACTION_DEN;
#endif
}

int ff_vc1_parse_frame_header(VC1Context *, Bits *);
int ff_vc1_parse_frame_header_adv(VC1Context *, Bits *);
void ff_simple_idct_int16_8bit(int16 *);
void ff_simple_idct_add_int16_8bit(uint8 *, ptrdiff_t, int16 *);
void ff_simple_idct84_add(uint8 *, ptrdiff_t, int16 *);
void ff_simple_idct48_add(uint8 *, ptrdiff_t, int16 *);
void ff_simple_idct44_add(uint8 *, ptrdiff_t, int16 *);
void initSupport(PictureState *, ChromaDSP *);

/** Table for conversion between TTBLK and TTMB */
extern const int ff_vc1_ttblk_to_tt[3][8];

extern const int ff_vc1_ttfrm_to_tt[4];

/** MV P mode - the 5th element is only used for mode 1 */
extern const uint8 ff_vc1_mv_pmode_table[2][5];
extern const uint8 ff_vc1_mv_pmode_table2[2][4];

extern const int ff_vc1_fps_nr[7], ff_vc1_fps_dr[2];
extern const uint8 ff_vc1_pquant_table[3][32];

/* MBMODE table for interlaced frame P-picture */
extern const uint8 ff_vc1_mbmode_intfrp[2][15][4];

/** @name VC-1 VLC tables
 *  @todo TODO move this into the context
 */
//@{
extern const VLC *ff_vc1_imode_vlc;
extern const VLC *ff_vc1_norm2_vlc;
extern const VLC *ff_vc1_norm6_vlc;
extern const VLC *ff_vc1_ttmb_vlc[3];
extern const VLC *ff_vc1_mv_diff_vlc[4];
extern const VLC *ff_vc1_cbpcy_p_vlc[4];
extern const VLC *ff_vc1_icbpcy_vlc[8];
extern const VLC *ff_vc1_4mv_block_pattern_vlc[4];
extern const VLC *ff_vc1_2mv_block_pattern_vlc[4];
extern const VLC *ff_vc1_ttblk_vlc[3];
extern const VLC *ff_vc1_subblkpat_vlc[3];
extern const VLC *ff_vc1_intfr_4mv_mbmode_vlc[4];
extern const VLC *ff_vc1_intfr_non4mv_mbmode_vlc[4];
extern const VLC *ff_vc1_if_mmv_mbmode_vlc[8];
extern const VLC *ff_vc1_if_1mv_mbmode_vlc[8];
extern const VLC *ff_vc1_1ref_mvdata_vlc[4];
extern const VLC *ff_vc1_2ref_mvdata_vlc[8];

extern const VLC *ff_vc1_ac_coeff_table[8];

//@}

/* pre-computed scales for all bfractions and base=256 */
extern const int16 ff_vc1_bfraction_lut[23];

// Same as H.264
extern const VC1Ratio ff_vc1_pixel_aspect[16];

extern const uint8 ff_wmv3_dc_scale_table[32];

/* DC differentials low+hi-mo, p217 are the same as in msmpeg4data .h */

/* Scantables/ZZ scan are at 11.9 (p262) and 8.1.1.12 (p10) */
extern const uint8 ff_vc1_simple_progressive_4x4_zz[16];
extern const uint8 ff_vc1_adv_progressive_8x4_zz[32];
extern const uint8 ff_vc1_adv_progressive_4x8_zz[32];
extern const uint8 ff_vc1_adv_interlaced_8x8_zz[64];
extern const uint8 ff_vc1_adv_interlaced_8x4_zz[32];
extern const uint8 ff_vc1_adv_interlaced_4x8_zz[32];
extern const uint8 ff_vc1_adv_interlaced_4x4_zz[16];

/* DQScale as specified in 8.1.3.9 - almost identical to 0x40000/i */
extern const int32 ff_vc1_dqscale[63];

/* P Interlaced field picture MV predictor scaling values (Table 114) */
extern const uint16 ff_vc1_field_mvpred_scales[2][7][4];
/* B Interlaced field picture backward MV predictor scaling values for first field (Table 115) */
extern const uint16 ff_vc1_b_field_mvpred_scales[7][4];

#define AC_MODES 8

extern const int ff_vc1_ac_sizes[AC_MODES];
extern const VLC *ff_msmp4_mb_i_vlc;
extern const VLC *ff_msmp4_dc_vlc[2 /* dc_table_index */][2 /* 0: luma, 1: chroma */];

/* intra picture macroblock coded block pattern */
using MSMPEG4VC1::ff_msmp4_mb_i_table;

#define WMV1_SCANTABLE_COUNT 4

extern const uint8 ff_wmv1_scantable[WMV1_SCANTABLE_COUNT][64];

using MSMPEG4VC1::ff_msmp4_dc_tables;

struct X8IDCT {
	void (*idct_add)(uint8 *, ptrdiff_t, int16 *);
};
typedef struct IntraX8DSPContext {
	void (*v_loop_filter)(uint8 *src, ptrdiff_t stride, int qscale);
	void (*h_loop_filter)(uint8 *src, ptrdiff_t stride, int qscale);

	void (*spatial_compensation[12])(const uint8 *src, uint8 *dst, ptrdiff_t stride);
	void (*setup_spatial_compensation)(const uint8 *src, uint8 *dst, ptrdiff_t stride, int *range, int *sum, int edges);
} IntraX8DSPContext;

void ff_intrax8dsp_init(IntraX8DSPContext *dsp);

typedef struct IntraX8Context {
	const VLC *j_ac_vlc_table[4]; // they point to the static j_mb_vlc.table
	const VLC *j_orient_vlc_table;
	const VLC *j_dc_vlc_table[3];

	int use_quant_matrix;

	// set by ff_intrax8_common_init
	uint8 *prediction_table; // 2 * (mb_w * 2)
	uint8 permutated_scantable[3][64];
	X8IDCT idsp;
	Settings *avctx;
	int16 *block;

	// set by the caller codec
	IntraX8DSPContext dsp;
	BlockDSP bdsp;
	int quant;
	int dquant;
	int qsum;
	int loopfilter;
	Planes *frame;
	Bits *gb;

	// calculated per frame
	int quant_dc_chroma;
	int divide_quant_dc_luma;
	int divide_quant_dc_chroma;
	uint8 *dest[3];
	uint8 scratchpad[42]; // size of the block is fixed (8x8 plus padding)

	// changed per block
	int edges;
	int flat_dc;
	int predicted_dc;
	int raw_orient;
	int chroma_orient;
	int orient;
	int est_run;

	// block props
	int mb_x, mb_y;
	int mb_width, mb_height;
} IntraX8Context;

int initX8(IntraX8Context *, int16 *, int, int);
void endX8(IntraX8Context *);

// Packet decoder lifecycle

class Decoder {
public:
	Decoder(int width, int height, bool advanced);
	~Decoder();
	bool initialize(const uint8 *data, uint32 size);
	const Picture *decode(const uint8 *data, uint32 size);
	const Picture *drain();
	bool failed() const { return _failed; }
	int width() const { return _v->width; }
	int height() const { return _v->height; }

private:
	VC1Context *_v;
	Settings _settings;
	Common::Array<uint8 *> _allocations;
	Picture _pictures[3];
	Planes _planes[3];
	bool _advanced, _ready, _failed, _pending;
	template<class T> T *allocate(uint32 n) {
		uint8 *p = new uint8[sizeof(T) * n]();
		_allocations.push_back(p);
		return reinterpret_cast<T *>(p);
	}
	void allocatePictures();
	void initializePictureLayout();
	void allocatePredictors();
	void allocateReferencePictures();
	void initializeConcealment();
	void initializeTransforms();

	void clearPictures();
	bool readHeader(const uint8 *, uint32, bool);
	bool decodePacket(const uint8 *, uint32, const Picture *&);
};

/* which indexes point to last=1 entries in tables */
static const int vc1_last_decode_table[AC_MODES] = {119, 99, 85, 81, 67, 58, 126, 109};

static const uint8 vc1_index_decode_table[AC_MODES][185][2] = {
	{{0, 1},  {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {0, 7},  {0, 8},  {0, 9},  {0, 10}, {0, 11}, {0, 12},
	 {0, 13}, {0, 14}, {0, 15}, {0, 16}, {0, 17}, {0, 18}, {0, 19}, {1, 1},  {1, 2},  {1, 3},  {1, 4},  {1, 5},
	 {1, 6},  {1, 7},  {1, 8},  {1, 9},  {1, 10}, {1, 11}, {1, 12}, {1, 13}, {1, 14}, {1, 15}, {2, 1},  {2, 2},
	 {2, 3},  {2, 4},  {2, 5},  {2, 6},  {2, 7},  {2, 8},  {2, 9},  {2, 10}, {2, 11}, {2, 12}, {3, 1},  {3, 2},
	 {3, 3},  {3, 4},  {3, 5},  {3, 6},  {3, 7},  {3, 8},  {3, 9},  {3, 10}, {3, 11}, {4, 1},  {4, 2},  {4, 3},
	 {4, 4},  {4, 5},  {4, 6},  {5, 1},  {5, 2},  {5, 3},  {5, 4},  {5, 5},  {6, 1},  {6, 2},  {6, 3},  {6, 4},
	 {7, 1},  {7, 2},  {7, 3},  {7, 4},  {8, 1},  {8, 2},  {8, 3},  {8, 4},  {9, 1},  {9, 2},  {9, 3},  {9, 4},
	 {10, 1}, {10, 2}, {10, 3}, {11, 1}, {11, 2}, {11, 3}, {12, 1}, {12, 2}, {12, 3}, {13, 1}, {13, 2}, {13, 3},
	 {14, 1}, {14, 2}, {14, 3}, {15, 1}, {15, 2}, {15, 3}, {16, 1}, {16, 2}, {17, 1}, {17, 2}, {18, 1}, {19, 1},
	 {20, 1}, {21, 1}, {22, 1}, {23, 1}, {24, 1}, {25, 1}, {26, 1}, {27, 1}, {28, 1}, {29, 1}, {30, 1}, {0, 1},
	 {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {1, 1},  {1, 2},  {1, 3},  {1, 4},  {1, 5},  {2, 1},  {2, 2},
	 {2, 3},  {2, 4},  {3, 1},  {3, 2},  {3, 3},  {3, 4},  {4, 1},  {4, 2},  {4, 3},  {5, 1},  {5, 2},  {6, 1},
	 {6, 2},  {7, 1},  {7, 2},  {8, 1},  {8, 2},  {9, 1},  {9, 2},  {10, 1}, {10, 2}, {11, 1}, {11, 2}, {12, 1},
	 {12, 2}, {13, 1}, {13, 2}, {14, 1}, {14, 2}, {15, 1}, {15, 2}, {16, 1}, {17, 1}, {18, 1}, {19, 1}, {20, 1},
	 {21, 1}, {22, 1}, {23, 1}, {24, 1}, {25, 1}, {26, 1}, {27, 1}, {28, 1}, {29, 1}, {30, 1}, {31, 1}, {32, 1},
	 {33, 1}, {34, 1}, {35, 1}, {36, 1}, {37, 1}},
	{{0, 1},  {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {0, 7},  {0, 8},  {0, 9},  {0, 10}, {0, 11}, {0, 12},
	 {0, 13}, {0, 14}, {0, 15}, {0, 16}, {0, 17}, {0, 18}, {0, 19}, {0, 20}, {0, 21}, {0, 22}, {0, 23}, {1, 1},
	 {1, 2},  {1, 3},  {1, 4},  {1, 5},  {1, 6},  {1, 7},  {1, 8},  {1, 9},  {1, 10}, {1, 11}, {2, 1},  {2, 2},
	 {2, 3},  {2, 4},  {2, 5},  {2, 6},  {2, 7},  {2, 8},  {3, 1},  {3, 2},  {3, 3},  {3, 4},  {3, 5},  {3, 6},
	 {3, 7},  {4, 1},  {4, 2},  {4, 3},  {4, 4},  {4, 5},  {5, 1},  {5, 2},  {5, 3},  {5, 4},  {5, 5},  {6, 1},
	 {6, 2},  {6, 3},  {6, 4},  {7, 1},  {7, 2},  {7, 3},  {7, 4},  {8, 1},  {8, 2},  {8, 3},  {9, 1},  {9, 2},
	 {9, 3},  {10, 1}, {10, 2}, {10, 3}, {11, 1}, {11, 2}, {11, 3}, {12, 1}, {12, 2}, {13, 1}, {13, 2}, {14, 1},
	 {14, 2}, {15, 1}, {15, 2}, {16, 1}, {16, 2}, {17, 1}, {18, 1}, {19, 1}, {20, 1}, {21, 1}, {22, 1}, {23, 1},
	 {24, 1}, {25, 1}, {26, 1}, {0, 1},  {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {0, 7},  {0, 8},  {0, 9},
	 {1, 1},  {1, 2},  {1, 3},  {1, 4},  {1, 5},  {2, 1},  {2, 2},  {2, 3},  {2, 4},  {3, 1},  {3, 2},  {3, 3},
	 {3, 4},  {4, 1},  {4, 2},  {4, 3},  {5, 1},  {5, 2},  {5, 3},  {6, 1},  {6, 2},  {6, 3},  {7, 1},  {7, 2},
	 {8, 1},  {8, 2},  {9, 1},  {9, 2},  {10, 1}, {10, 2}, {11, 1}, {11, 2}, {12, 1}, {12, 2}, {13, 1}, {13, 2},
	 {14, 1}, {14, 2}, {15, 1}, {16, 1}, {17, 1}, {18, 1}, {19, 1}, {20, 1}, {21, 1}, {22, 1}, {23, 1}, {24, 1},
	 {25, 1}, {26, 1}, {27, 1}, {28, 1}, {29, 1}, {30, 1}, {31, 1}, {32, 1}, {33, 1}, {34, 1}, {35, 1}, {36, 1}},
	{{0, 1},  {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {0, 7},  {0, 8},  {0, 9},  {0, 10}, {0, 11}, {0, 12},
	 {0, 13}, {0, 14}, {0, 15}, {0, 16}, {1, 1},  {1, 2},  {1, 3},  {1, 4},  {1, 5},  {1, 6},  {1, 7},  {1, 8},
	 {1, 9},  {1, 10}, {1, 11}, {2, 1},  {2, 2},  {2, 3},  {2, 4},  {2, 5},  {2, 6},  {2, 7},  {2, 8},  {3, 1},
	 {3, 2},  {3, 3},  {3, 4},  {3, 5},  {3, 6},  {3, 7},  {4, 1},  {4, 2},  {4, 3},  {4, 4},  {4, 5},  {5, 1},
	 {5, 2},  {5, 3},  {5, 4},  {6, 1},  {6, 2},  {6, 3},  {6, 4},  {7, 1},  {7, 2},  {7, 3},  {8, 1},  {8, 2},
	 {8, 3},  {9, 1},  {9, 2},  {9, 3},  {10, 1}, {10, 2}, {10, 3}, {11, 1}, {11, 2}, {11, 3}, {12, 1}, {12, 2},
	 {12, 3}, {13, 1}, {13, 2}, {13, 3}, {14, 1}, {14, 2}, {15, 1}, {15, 2}, {16, 1}, {17, 1}, {18, 1}, {19, 1},
	 {20, 1}, {0, 1},  {0, 2},  {0, 3},  {0, 4},  {1, 1},  {1, 2},  {1, 3},  {1, 4},  {2, 1},  {2, 2},  {2, 3},
	 {3, 1},  {3, 2},  {3, 3},  {4, 1},  {4, 2},  {5, 1},  {5, 2},  {6, 1},  {6, 2},  {7, 1},  {7, 2},  {8, 1},
	 {8, 2},  {9, 1},  {9, 2},  {10, 1}, {10, 2}, {11, 1}, {11, 2}, {12, 1}, {12, 2}, {13, 1}, {13, 2}, {14, 1},
	 {15, 1}, {16, 1}, {17, 1}, {18, 1}, {19, 1}, {20, 1}, {21, 1}, {22, 1}, {23, 1}, {24, 1}, {25, 1}, {26, 1}},
	{{0, 1},  {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {0, 7},  {0, 8},  {0, 9},  {0, 10}, {0, 11}, {0, 12},
	 {0, 13}, {0, 14}, {1, 1},  {1, 2},  {1, 3},  {1, 4},  {1, 5},  {1, 6},  {1, 7},  {1, 8},  {1, 9},  {2, 1},
	 {2, 2},  {2, 3},  {2, 4},  {2, 5},  {3, 1},  {3, 2},  {3, 3},  {3, 4},  {4, 1},  {4, 2},  {4, 3},  {4, 4},
	 {5, 1},  {5, 2},  {5, 3},  {5, 4},  {6, 1},  {6, 2},  {6, 3},  {7, 1},  {7, 2},  {7, 3},  {8, 1},  {8, 2},
	 {8, 3},  {9, 1},  {9, 2},  {9, 3},  {10, 1}, {10, 2}, {10, 3}, {11, 1}, {11, 2}, {11, 3}, {12, 1}, {12, 2},
	 {12, 3}, {13, 1}, {13, 2}, {14, 1}, {14, 2}, {15, 1}, {15, 2}, {16, 1}, {17, 1}, {18, 1}, {19, 1}, {20, 1},
	 {21, 1}, {22, 1}, {23, 1}, {24, 1}, {25, 1}, {26, 1}, {27, 1}, {28, 1}, {29, 1}, {0, 1},  {0, 2},  {0, 3},
	 {0, 4},  {0, 5},  {1, 1},  {1, 2},  {1, 3},  {1, 4},  {2, 1},  {2, 2},  {2, 3},  {3, 1},  {3, 2},  {3, 3},
	 {4, 1},  {4, 2},  {5, 1},  {5, 2},  {6, 1},  {6, 2},  {7, 1},  {7, 2},  {8, 1},  {8, 2},  {9, 1},  {9, 2},
	 {10, 1}, {10, 2}, {11, 1}, {11, 2}, {12, 1}, {12, 2}, {13, 1}, {13, 2}, {14, 1}, {14, 2}, {15, 1}, {15, 2},
	 {16, 1}, {17, 1}, {18, 1}, {19, 1}, {20, 1}, {21, 1}, {22, 1}, {23, 1}, {24, 1}, {25, 1}, {26, 1}, {27, 1},
	 {28, 1}, {29, 1}, {30, 1}, {31, 1}, {32, 1}, {33, 1}, {34, 1}, {35, 1}, {36, 1}, {37, 1}, {38, 1}, {39, 1},
	 {40, 1}, {41, 1}, {42, 1}, {43, 1}},
	{{0, 1},  {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {0, 7},  {0, 8},  {0, 9},  {0, 10}, {0, 11}, {0, 12},
	 {0, 13}, {0, 14}, {0, 15}, {0, 16}, {0, 17}, {0, 18}, {0, 19}, {0, 20}, {0, 21}, {0, 22}, {0, 23}, {0, 24},
	 {0, 25}, {0, 26}, {0, 27}, {1, 1},  {1, 2},  {1, 3},  {1, 4},  {1, 5},  {1, 6},  {1, 7},  {1, 8},  {1, 9},
	 {1, 10}, {2, 1},  {2, 2},  {2, 3},  {2, 4},  {2, 5},  {3, 1},  {3, 2},  {3, 3},  {3, 4},  {4, 1},  {4, 2},
	 {4, 3},  {5, 1},  {5, 2},  {5, 3},  {6, 1},  {6, 2},  {6, 3},  {7, 1},  {7, 2},  {7, 3},  {8, 1},  {8, 2},
	 {9, 1},  {9, 2},  {10, 1}, {11, 1}, {12, 1}, {13, 1}, {14, 1}, {0, 1},  {0, 2},  {0, 3},  {0, 4},  {0, 5},
	 {0, 6},  {0, 7},  {0, 8},  {1, 1},  {1, 2},  {1, 3},  {2, 1},  {2, 2},  {3, 1},  {3, 2},  {4, 1},  {4, 2},
	 {5, 1},  {5, 2},  {6, 1},  {6, 2},  {7, 1},  {8, 1},  {9, 1},  {10, 1}, {11, 1}, {12, 1}, {13, 1}, {14, 1},
	 {15, 1}, {16, 1}, {17, 1}, {18, 1}, {19, 1}, {20, 1}},
	{{0, 1},  {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {0, 7},  {0, 8},  {0, 9},  {0, 10}, {0, 11}, {0, 12},
	 {1, 1},  {1, 2},  {1, 3},  {1, 4},  {1, 5},  {1, 6},  {2, 1},  {2, 2},  {2, 3},  {2, 4},  {3, 1},  {3, 2},
	 {3, 3},  {4, 1},  {4, 2},  {4, 3},  {5, 1},  {5, 2},  {5, 3},  {6, 1},  {6, 2},  {6, 3},  {7, 1},  {7, 2},
	 {8, 1},  {8, 2},  {9, 1},  {9, 2},  {10, 1}, {10, 2}, {11, 1}, {12, 1}, {13, 1}, {14, 1}, {15, 1}, {16, 1},
	 {17, 1}, {18, 1}, {19, 1}, {20, 1}, {21, 1}, {22, 1}, {23, 1}, {24, 1}, {25, 1}, {26, 1}, {0, 1},  {0, 2},
	 {0, 3},  {1, 1},  {1, 2},  {2, 1},  {3, 1},  {4, 1},  {5, 1},  {6, 1},  {7, 1},  {8, 1},  {9, 1},  {10, 1},
	 {11, 1}, {12, 1}, {13, 1}, {14, 1}, {15, 1}, {16, 1}, {17, 1}, {18, 1}, {19, 1}, {20, 1}, {21, 1}, {22, 1},
	 {23, 1}, {24, 1}, {25, 1}, {26, 1}, {27, 1}, {28, 1}, {29, 1}, {30, 1}, {31, 1}, {32, 1}, {33, 1}, {34, 1},
	 {35, 1}, {36, 1}, {37, 1}, {38, 1}, {39, 1}, {40, 1}},
	{{0, 1},  {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {0, 7},  {0, 8},  {0, 9},  {0, 10}, {0, 11}, {0, 12},
	 {0, 13}, {0, 14}, {0, 15}, {0, 16}, {0, 17}, {0, 18}, {0, 19}, {0, 20}, {0, 21}, {0, 22}, {0, 23}, {0, 24},
	 {0, 25}, {0, 26}, {0, 27}, {0, 28}, {0, 29}, {0, 30}, {0, 31}, {0, 32}, {0, 33}, {0, 34}, {0, 35}, {0, 36},
	 {0, 37}, {0, 38}, {0, 39}, {0, 40}, {0, 41}, {0, 42}, {0, 43}, {0, 44}, {0, 45}, {0, 46}, {0, 47}, {0, 48},
	 {0, 49}, {0, 50}, {0, 51}, {0, 52}, {0, 53}, {0, 54}, {0, 55}, {0, 56}, {1, 1},  {1, 2},  {1, 3},  {1, 4},
	 {1, 5},  {1, 6},  {1, 7},  {1, 8},  {1, 9},  {1, 10}, {1, 11}, {1, 12}, {1, 13}, {1, 14}, {1, 15}, {1, 16},
	 {1, 17}, {1, 18}, {1, 19}, {1, 20}, {2, 1},  {2, 2},  {2, 3},  {2, 4},  {2, 5},  {2, 6},  {2, 7},  {2, 8},
	 {2, 9},  {2, 10}, {3, 1},  {3, 2},  {3, 3},  {3, 4},  {3, 5},  {3, 6},  {3, 7},  {4, 1},  {4, 2},  {4, 3},
	 {4, 4},  {4, 5},  {4, 6},  {5, 1},  {5, 2},  {5, 3},  {5, 4},  {5, 5},  {6, 1},  {6, 2},  {6, 3},  {6, 4},
	 {7, 1},  {7, 2},  {7, 3},  {8, 1},  {8, 2},  {8, 3},  {9, 1},  {9, 2},  {9, 3},  {10, 1}, {10, 2}, {11, 1},
	 {11, 2}, {12, 1}, {12, 2}, {13, 1}, {13, 2}, {14, 1}, {0, 1},  {0, 2},  {0, 3},  {0, 4},  {1, 1},  {1, 2},
	 {1, 3},  {2, 1},  {2, 2},  {2, 3},  {3, 1},  {3, 2},  {4, 1},  {4, 2},  {5, 1},  {5, 2},  {6, 1},  {6, 2},
	 {7, 1},  {7, 2},  {8, 1},  {8, 2},  {9, 1},  {9, 2},  {10, 1}, {10, 2}, {11, 1}, {11, 2}, {12, 1}, {12, 2},
	 {13, 1}, {13, 2}, {14, 1}, {14, 2}, {15, 1}, {16, 1}},
	{{0, 1},  {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {0, 7},  {0, 8},  {0, 9},  {0, 10}, {0, 11}, {0, 12},
	 {0, 13}, {0, 14}, {0, 15}, {0, 16}, {0, 17}, {0, 18}, {0, 19}, {0, 20}, {0, 21}, {0, 22}, {0, 23}, {0, 24},
	 {0, 25}, {0, 26}, {0, 27}, {0, 28}, {0, 29}, {0, 30}, {0, 31}, {0, 32}, {1, 1},  {1, 2},  {1, 3},  {1, 4},
	 {1, 5},  {1, 6},  {1, 7},  {1, 8},  {1, 9},  {1, 10}, {1, 11}, {1, 12}, {1, 13}, {2, 1},  {2, 2},  {2, 3},
	 {2, 4},  {2, 5},  {2, 6},  {2, 7},  {2, 8},  {3, 1},  {3, 2},  {3, 3},  {3, 4},  {3, 5},  {3, 6},  {4, 1},
	 {4, 2},  {4, 3},  {4, 4},  {4, 5},  {5, 1},  {5, 2},  {5, 3},  {5, 4},  {6, 1},  {6, 2},  {6, 3},  {6, 4},
	 {7, 1},  {7, 2},  {7, 3},  {8, 1},  {8, 2},  {8, 3},  {9, 1},  {9, 2},  {9, 3},  {10, 1}, {10, 2}, {11, 1},
	 {11, 2}, {12, 1}, {12, 2}, {13, 1}, {13, 2}, {14, 1}, {14, 2}, {15, 1}, {15, 2}, {16, 1}, {16, 2}, {17, 1},
	 {17, 2}, {18, 1}, {18, 2}, {19, 1}, {19, 2}, {20, 1}, {20, 2}, {21, 1}, {21, 2}, {22, 1}, {22, 2}, {23, 1},
	 {24, 1}, {0, 1},  {0, 2},  {0, 3},  {0, 4},  {1, 1},  {1, 2},  {1, 3},  {2, 1},  {2, 2},  {2, 3},  {3, 1},
	 {3, 2},  {3, 3},  {4, 1},  {4, 2},  {5, 1},  {5, 2},  {6, 1},  {6, 2},  {7, 1},  {7, 2},  {8, 1},  {8, 2},
	 {9, 1},  {9, 2},  {10, 1}, {10, 2}, {11, 1}, {11, 2}, {12, 1}, {12, 2}, {13, 1}, {13, 2}, {14, 1}, {14, 2},
	 {15, 1}, {15, 2}, {16, 1}, {16, 2}, {17, 1}, {17, 2}, {18, 1}, {18, 2}, {19, 1}, {19, 2}, {20, 1}, {20, 2},
	 {21, 1}, {21, 2}, {22, 1}, {22, 2}, {23, 1}, {23, 2}, {24, 1}, {24, 2}, {25, 1}, {25, 2}, {26, 1}, {26, 2},
	 {27, 1}, {27, 2}, {28, 1}, {28, 2}, {29, 1}, {30, 1}}};

static const uint8 vc1_delta_level_table[AC_MODES][31] = {
	{19, 15, 12, 11, 6, 5, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{23, 11, 8, 7, 5, 5, 4, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{16, 11, 8, 7, 5, 4, 4, 3, 3, 3, 3, 3, 3, 3, 2, 2, 1, 1, 1, 1, 1},
	{14, 9, 5, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{27, 10, 5, 4, 3, 3, 3, 3, 2, 2, 1, 1, 1, 1, 1},
	{12, 6, 4, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{56, 20, 10, 7, 6, 5, 4, 3, 3, 3, 2, 2, 2, 2, 1},
	{32, 13, 8, 6, 5, 4, 4, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1}};

static const uint8 vc1_last_delta_level_table[AC_MODES][44] = {
	{6, 5, 4, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{9, 5, 4, 4, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{4, 4, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{5, 4, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1,
	 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{8, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{4, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1},
	{4, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1}};

static const uint8 vc1_delta_run_table[AC_MODES][57] = {
	{255, 30, 17, 15, 9, 5, 4, 3, 3, 3, 3, 3, 2, 1, 1, 1, 0, 0, 0, 0},
	{255, 26, 16, 11, 7, 5, 3, 3, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{255, 20, 15, 13, 6, 4, 3, 3, 2, 1, 1, 1, 0, 0, 0, 0, 0},
	{255, 29, 15, 12, 5, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0},
	{255, 14, 9, 7, 3, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{255, 26, 10, 6, 2, 1, 1, 0, 0, 0, 0, 0, 0},
	{255, 14, 13, 9, 6, 5, 4, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	 0,   0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{255, 24, 22, 9, 6, 4, 3, 2, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

static const uint8 vc1_last_delta_run_table[AC_MODES][10] = {{255, 37, 15, 4, 3, 1, 0},
															 {255, 36, 14, 6, 3, 1, 0, 0, 0, 0},
															 {255, 26, 13, 3, 1},
															 {255, 43, 15, 3, 1, 0},
															 {255, 20, 6, 1, 0, 0, 0, 0, 0},
															 {255, 40, 1, 0},
															 {255, 16, 14, 2, 0},
															 {255, 30, 28, 3, 0}};

/* BitPlane IMODE - such a small table... */
// Static coding tables

static const uint8 vc1_imode_codes[7] = {0, 2, 1, 3, 1, 2, 3};
static const uint8 vc1_imode_bits[7] = {4, 2, 3, 2, 4, 3, 3};

/* Normal-2 imode */
static const uint8 vc1_norm2_codes[4] = {0, 4, 5, 3};
static const uint8 vc1_norm2_bits[4] = {1, 3, 3, 2};

static const uint16 vc1_norm6_codes[64] = {
	0x001, 0x002, 0x003, 0x000, 0x004, 0x001, 0x002, 0x047, 0x005, 0x003, 0x004, 0x04B, 0x005, 0x04D, 0x04E, 0x30E,
	0x006, 0x006, 0x007, 0x053, 0x008, 0x055, 0x056, 0x30D, 0x009, 0x059, 0x05A, 0x30C, 0x05C, 0x30B, 0x30A, 0x037,
	0x007, 0x00A, 0x00B, 0x043, 0x00C, 0x045, 0x046, 0x309, 0x00D, 0x049, 0x04A, 0x308, 0x04C, 0x307, 0x306, 0x036,
	0x00E, 0x051, 0x052, 0x305, 0x054, 0x304, 0x303, 0x035, 0x058, 0x302, 0x301, 0x034, 0x300, 0x033, 0x032, 0x007,
};

static const uint8 vc1_norm6_bits[64] = {
	1,  4,  4,  8,  4,  8,  8,  10, 4,  8,  8,  10, 8,  10, 10, 13, 4,  8,  8, 10, 8,  10,
	10, 13, 8,  10, 10, 13, 10, 13, 13, 9,  4,  8,  8,  10, 8,  10, 10, 13, 8, 10, 10, 13,
	10, 13, 13, 9,  8,  10, 10, 13, 10, 13, 13, 9,  10, 13, 13, 9,  13, 9,  9, 6,
};

/* 4MV Block pattern VLC tables */
static const uint8 vc1_4mv_block_pattern_codes[4][16] = {{14, 58, 59, 25, 12, 26, 15, 15, 13, 24, 27, 0, 28, 1, 2, 2},
														 {8, 18, 19, 4, 20, 5, 30, 11, 21, 31, 6, 12, 7, 13, 14, 0},
														 {15, 6, 7, 2, 8, 3, 28, 9, 10, 29, 4, 11, 5, 12, 13, 0},
														 {0, 11, 12, 4, 13, 5, 30, 16, 14, 31, 6, 17, 7, 18, 19, 10}};
static const uint8 vc1_4mv_block_pattern_bits[4][16] = {{5, 6, 6, 5, 5, 5, 5, 4, 5, 5, 5, 3, 5, 3, 3, 2},
														{4, 5, 5, 4, 5, 4, 5, 4, 5, 5, 4, 4, 4, 4, 4, 2},
														{4, 4, 4, 4, 4, 4, 5, 4, 4, 5, 4, 4, 4, 4, 4, 3},
														{2, 4, 4, 4, 4, 4, 5, 5, 4, 5, 4, 5, 4, 5, 5, 4}};

/* 2MV Block pattern VLC tables */
static const uint8 vc1_2mv_block_pattern_codes[4][4] = {{2, 1, 0, 3}, {1, 0, 2, 3}, {2, 0, 3, 1}, {1, 3, 2, 0}};

static const uint8 vc1_2mv_block_pattern_bits[4][4] = {{2, 2, 2, 2}, {1, 2, 3, 3}, {3, 2, 3, 1}, {1, 3, 3, 2}};

/* Interlaced frame picture 4MV MBMODE VLC tables (p. 246, p. 360) */
static const uint16 vc1_intfr_4mv_mbmode_codes[4][15] = {
	{22, 17, 0, 47, 32, 10, 1, 3, 67, 133, 132, 92, 19, 93, 18},
	{3, 45, 0, 7, 23, 6, 1, 2, 10, 39, 44, 8, 18, 77, 76},
	{15, 6, 28, 9, 41, 6, 2, 15, 14, 8, 40, 29, 0, 21, 11},
	{7, 198, 1, 2, 193, 13, 25, 0, 97, 1599, 98, 398, 798, 192, 1598}};

static const uint8 vc1_intfr_4mv_mbmode_bits[4][15] = {{5, 5, 2, 6, 6, 4, 2, 2, 7, 8, 8, 7, 5, 7, 5},
													   {3, 6, 3, 3, 5, 3, 3, 3, 4, 6, 6, 4, 5, 7, 7},
													   {4, 3, 5, 5, 7, 4, 2, 5, 5, 5, 7, 5, 2, 6, 5},
													   {4, 9, 1, 3, 9, 5, 6, 2, 8, 12, 8, 10, 11, 9, 12}};

/* Interlaced frame picture NON-4MV MBMODE VLC tables (p. 363) */
static const uint8 vc1_intfr_non4mv_mbmode_codes[4][9] = {{9, 22, 0, 17, 16, 10, 1, 3, 23},
														  {7, 0, 5, 2, 1, 1, 6, 3, 4},
														  {1, 0, 10, 23, 44, 8, 3, 9, 45},
														  {7, 97, 1, 2, 49, 13, 25, 0, 96}};

static const uint8 vc1_intfr_non4mv_mbmode_bits[4][9] = {
	{4, 5, 2, 5, 5, 4, 2, 2, 5}, {3, 4, 6, 2, 3, 2, 3, 5, 6}, {2, 2, 4, 5, 6, 4, 2, 4, 6}, {4, 8, 1, 3, 7, 5, 6, 2, 8}};

/* Interlaced field picture MBMODE VLC tables (p. 356 - 11.4.1, 11.4.2) */
/* mixed-MV */
static const uint8 vc1_if_mmv_mbmode_codes[8][8] = {{16, 17, 3, 3, 0, 5, 9, 2},     {8, 9, 3, 6, 7, 0, 5, 2},
													{16, 17, 5, 3, 0, 3, 9, 2},     {56, 57, 15, 4, 5, 6, 29, 0},
													{52, 53, 27, 14, 15, 2, 12, 0}, {56, 57, 29, 5, 6, 0, 15, 4},
													{16, 17, 6, 7, 0, 1, 9, 5},     {56, 57, 0, 5, 6, 29, 4, 15}};
static const uint8 vc1_if_mmv_mbmode_bits[8][8] = {
	{6, 6, 2, 3, 2, 4, 5, 2}, {5, 5, 3, 3, 3, 2, 4, 2}, {6, 6, 4, 3, 2, 2, 5, 2}, {6, 6, 4, 3, 3, 3, 5, 1},
	{6, 6, 5, 4, 4, 2, 4, 1}, {6, 6, 5, 3, 3, 1, 4, 3}, {5, 5, 3, 3, 2, 2, 4, 3}, {6, 6, 1, 3, 3, 5, 3, 4}};

/* 1MV */
static const uint8 vc1_if_1mv_mbmode_codes[8][6] = {{0, 1, 1, 1, 1, 1},    {0, 1, 1, 1, 1, 1},  {16, 17, 3, 0, 9, 5},
													{20, 21, 3, 11, 0, 4}, {4, 5, 2, 3, 3, 0},  {4, 5, 3, 2, 0, 3},
													{0, 1, 1, 1, 1, 1},    {16, 17, 9, 5, 3, 0}};
static const uint8 vc1_if_1mv_mbmode_bits[8][6] = {{5, 5, 1, 3, 2, 4}, {5, 5, 1, 2, 3, 4}, {5, 5, 2, 1, 4, 3},
												   {5, 5, 2, 4, 1, 3}, {4, 4, 2, 3, 2, 2}, {4, 4, 3, 2, 2, 2},
												   {5, 5, 3, 4, 1, 2}, {5, 5, 4, 3, 2, 1}};

/* Interlaced frame/field picture MVDATA VLC tables */

/* 1-reference tables */
static const uint32 vc1_1ref_mvdata_codes[4][72] =
	{/* uint32 may be too big */
	 {0x00005, 0x0000C, 0x0001E, 0x00012, 0x0000C, 0x00034, 0x00075, 0x00070, 0x00000, 0x00008, 0x0001B, 0x00008,
	  0x0001D, 0x0007C, 0x000D6, 0x001DE, 0x001AF, 0x00005, 0x0001B, 0x00026, 0x0001E, 0x00012, 0x00076, 0x0004D,
	  0x001F6, 0x001F4, 0x00039, 0x0007F, 0x00027, 0x0006A, 0x00071, 0x00035, 0x00071, 0x00068, 0x001DC, 0x00027,
	  0x00073, 0x000FF, 0x000E8, 0x000E9, 0x0007E, 0x001F9, 0x001F5, 0x001FD, 0x0003E, 0x001CA, 0x003F9, 0x0004C,
	  0x00069, 0x001FA, 0x001DF, 0x001F7, 0x00070, 0x001DD, 0x00E4D, 0x00727, 0x00392, 0x001C8, 0x001CB, 0x003F8,
	  0x001AE, 0x001F8, 0x001FB, 0x0E4CE, 0x0E4CF, 0x07260, 0x07261, 0x07262, 0x07263, 0x07264, 0x07265, 0x07266},
	 {0x00007, 0x00001, 0x00007, 0x00016, 0x00001, 0x00045, 0x00018, 0x002B6, 0x00006, 0x00004, 0x00017, 0x00010,
	  0x00029, 0x0002C, 0x0015A, 0x00066, 0x0019E, 0x00009, 0x00028, 0x00017, 0x00000, 0x0002A, 0x00004, 0x0005B,
	  0x000B5, 0x000CE, 0x00006, 0x00044, 0x0000F, 0x00046, 0x0000E, 0x000AC, 0x00032, 0x00037, 0x011EB, 0x0000A,
	  0x0001A, 0x0011F, 0x00016, 0x00014, 0x0002B, 0x00168, 0x00055, 0x023D5, 0x00057, 0x0002F, 0x00036, 0x0002E,
	  0x00169, 0x00054, 0x0047B, 0x0019F, 0x02B7D, 0x0008E, 0x00ADE, 0x00479, 0x0056E, 0x008F4, 0x015BF, 0x00478,
	  0x023D4, 0x0ADF1, 0x056F9, 0xADF0E, 0xADF0F, 0x56F80, 0x56F81, 0x56F82, 0x56F83, 0x56F84, 0x56F85, 0x56F86},
	 {0x00002, 0x00006, 0x00007, 0x0000D, 0x00007, 0x00030, 0x000FF, 0x001F0, 0x00002, 0x00000, 0x00005, 0x00019,
	  0x0001E, 0x00007, 0x00063, 0x000FD, 0x00023, 0x0000E, 0x0001B, 0x0001A, 0x00006, 0x00009, 0x00018, 0x000C5,
	  0x00033, 0x001F1, 0x00002, 0x003FB, 0x001F3, 0x00022, 0x001FC, 0x00042, 0x00623, 0x00083, 0x00620, 0x0007D,
	  0x00040, 0x00043, 0x003E4, 0x003E5, 0x00191, 0x00FE9, 0x00105, 0x00208, 0x000FC, 0x00624, 0x00622, 0x00190,
	  0x00626, 0x007F5, 0x00C4B, 0x01FD0, 0x0104D, 0x00065, 0x00C42, 0x000C9, 0x00627, 0x00C43, 0x00C4A, 0x0104E,
	  0x01FD1, 0x0104F, 0x00412, 0x104CE, 0x104CF, 0x08260, 0x08261, 0x08262, 0x08263, 0x08264, 0x08265, 0x08266},
	 {0x0000D, 0x00001, 0x00004, 0x00000, 0x00017, 0x00005, 0x0007F, 0x0004D, 0x00003, 0x00011, 0x0003E, 0x0003B,
	  0x00017, 0x00067, 0x0004A, 0x000C3, 0x000F2, 0x0000A, 0x0002C, 0x00032, 0x0003D, 0x00015, 0x00028, 0x00093,
	  0x000CC, 0x00096, 0x00003, 0x00075, 0x00020, 0x0002D, 0x00021, 0x00029, 0x00090, 0x001D0, 0x001FB, 0x0001C,
	  0x0004C, 0x00060, 0x00009, 0x00008, 0x0002D, 0x0009F, 0x001FA, 0x0013D, 0x00031, 0x000FC, 0x00058, 0x00092,
	  0x000F0, 0x000F1, 0x000CD, 0x00185, 0x00165, 0x0004E, 0x00091, 0x000E9, 0x00184, 0x001D1, 0x001E6, 0x00097,
	  0x001E7, 0x000B3, 0x0013C, 0x0164E, 0x0164F, 0x00B20, 0x00B21, 0x00B22, 0x00B23, 0x00B24, 0x00B25, 0x00B26}};

static const uint8 vc1_1ref_mvdata_bits[4][72] = {
	{3, 4, 5, 5, 5, 6, 7,  7,  2,  4,  5,  5,  6, 7, 8, 9,  9,  4,  6,  6,  6,  6,  7,  8,
	 9, 9, 6, 8, 7, 7, 7,  7,  8,  8,  9,  6,  8, 8, 8, 8,  8,  9,  9,  9,  7,  10, 10, 8,
	 8, 9, 9, 9, 8, 9, 13, 12, 11, 10, 10, 10, 9, 9, 9, 17, 17, 16, 16, 16, 16, 16, 16, 16},
	{3,  3,  4,  5,  5,  7, 8,  10, 3,  4,  5,  5,  6,  7,  9,  10, 12, 4,  6,  6,  5,  6,  6,  8,
	 9,  11, 4,  7,  7,  7, 7,  8,  9,  9,  13, 5,  8,  9,  8,  8,  9,  10, 10, 14, 7,  9,  9,  9,
	 10, 10, 11, 12, 14, 8, 12, 11, 11, 12, 13, 11, 14, 16, 15, 20, 20, 19, 19, 19, 19, 19, 19, 19},
	{3,  4,  4,  4,  5,  6, 8,  9,  2,  4,  5,  5,  5,  6,  7,  8,  8,  4,  7,  7,  6,  6,  7,  8,
	 8,  9,  5,  10, 9,  8, 9,  9,  11, 10, 11, 7,  9,  9,  10, 10, 11, 12, 11, 12, 8,  11, 11, 11,
	 11, 11, 12, 13, 15, 9, 12, 10, 11, 12, 12, 15, 13, 15, 13, 19, 19, 18, 18, 18, 18, 18, 18, 18},
	{4, 4, 4, 4, 5,  5, 7, 7, 3, 5, 6, 6, 6, 7, 7, 8,  8,  4,  6,  6,  6,  6,  7,  8,
	 8, 8, 4, 7, 6,  6, 6, 7, 8, 9, 9, 5, 7, 7, 6, 6,  7,  8,  9,  9,  6,  8,  8,  8,
	 8, 8, 8, 9, 10, 7, 8, 8, 9, 9, 9, 8, 9, 9, 9, 14, 14, 13, 13, 13, 13, 13, 13, 13}};

/* 2-reference tables */
static const uint32 vc1_2ref_mvdata_codes[8][126] =
	{/* table 132 - table 139 */
	 {0x0000C, 0x0001C, 0x0000B, 0x00000, 0x0000E, 0x0002A, 0x00050, 0x00368, 0x00002, 0x0001A, 0x00004, 0x0003A,
	  0x0001D, 0x0006C, 0x000EF, 0x001BC, 0x0015F, 0x0000F, 0x00003, 0x0001C, 0x0000D, 0x0000B, 0x0003E, 0x000A7,
	  0x00146, 0x00199, 0x00006, 0x0001F, 0x00004, 0x0003C, 0x00007, 0x001BE, 0x0008B, 0x0002C, 0x007B3, 0x00005,
	  0x000DB, 0x00056, 0x000EC, 0x00052, 0x001BD, 0x00078, 0x000CF, 0x00573, 0x00009, 0x00023, 0x000ED, 0x00018,
	  0x00006, 0x00044, 0x000F5, 0x00079, 0x006D2, 0x0006E, 0x0002B, 0x0015D, 0x00017, 0x0037F, 0x00144, 0x000CE,
	  0x00028, 0x000AB, 0x00010, 0x001B5, 0x000F7, 0x000A6, 0x0007B, 0x00028, 0x001ED, 0x001E9, 0x006FD, 0x00004,
	  0x000F5, 0x00029, 0x0028A, 0x0028B, 0x0028F, 0x00DF9, 0x00335, 0x01E85, 0x000EE, 0x002BD, 0x0002B, 0x003D8,
	  0x003D1, 0x00198, 0x001E9, 0x0051D, 0x000B4, 0x0003F, 0x00455, 0x0022B, 0x00229, 0x00451, 0x00578, 0x007B2,
	  0x00570, 0x00155, 0x00032, 0x003D0, 0x00054, 0x006D3, 0x00571, 0x00454, 0x00334, 0x01BF1, 0x000B7, 0x00029,
	  0x01E84, 0x0016C, 0x0019B, 0x01BF0, 0x00579, 0x00F43, 0x000B5, 0x008A1, 0x0002A, 0x0016D, 0x008A0, 0x007A0,
	  0x003D1, 0x00AE5, 0x00154, 0x00AE4, 0x00A39, 0x00A38},
	 {0x00003, 0x00009, 0x00016, 0x00010, 0x000D7, 0x00335, 0x00574, 0x00555, 0x00000, 0x0001D, 0x00009, 0x00017,
	  0x0002C, 0x000AD, 0x00374, 0x006B3, 0x00577, 0x0000F, 0x00018, 0x0000A, 0x0002E, 0x00022, 0x0017C, 0x00E7B,
	  0x01B89, 0x015D8, 0x00008, 0x00034, 0x0006D, 0x00023, 0x001C2, 0x00376, 0x002D3, 0x01C4A, 0x0330A, 0x00014,
	  0x0006A, 0x00072, 0x0006C, 0x000E3, 0x0019B, 0x0073F, 0x01CF0, 0x00B41, 0x00032, 0x000E6, 0x000E0, 0x000CF,
	  0x000AB, 0x0019C, 0x002AB, 0x00E2B, 0x015D9, 0x0006F, 0x001C3, 0x000AF, 0x000BF, 0x000AC, 0x0017D, 0x006E3,
	  0x00E29, 0x01984, 0x00054, 0x000B5, 0x0017A, 0x001AD, 0x00199, 0x00178, 0x00358, 0x002D2, 0x01C4B, 0x0005B,
	  0x002A8, 0x00331, 0x00388, 0x0038B, 0x00370, 0x00713, 0x00CC3, 0x01CF1, 0x001B9, 0x005EF, 0x00738, 0x002F2,
	  0x0033B, 0x002B9, 0x006EB, 0x00570, 0x00E24, 0x0039D, 0x005A2, 0x005A3, 0x00E7D, 0x005EE, 0x00739, 0x00554,
	  0x00AA5, 0x00AA4, 0x00377, 0x01CF5, 0x00BCE, 0x00E79, 0x00660, 0x00674, 0x006EA, 0x00E7C, 0x00D65, 0x002F6,
	  0x015DA, 0x01B88, 0x005A1, 0x01CF4, 0x005E6, 0x00E28, 0x00575, 0x00D64, 0x00334, 0x0330B, 0x015DB, 0x00B40,
	  0x00BCF, 0x00DC5, 0x00E2A, 0x00675, 0x00571, 0x00553},
	 {0x00004, 0x00002, 0x00010, 0x00003, 0x00017, 0x00045, 0x0003E, 0x0007E, 0x00003, 0x00002, 0x00028, 0x0001E,
	  0x00015, 0x00047, 0x00002, 0x0014D, 0x00060, 0x0000B, 0x00026, 0x00024, 0x00014, 0x00032, 0x0006F, 0x000C3,
	  0x00531, 0x006E5, 0x00015, 0x0003F, 0x0002D, 0x00001, 0x0013E, 0x000DD, 0x000F6, 0x00305, 0x00331, 0x0000E,
	  0x00003, 0x00034, 0x00033, 0x0001A, 0x0014A, 0x000C5, 0x000F4, 0x006E4, 0x00001, 0x0003C, 0x0007D, 0x0008D,
	  0x0009D, 0x00031, 0x0006E, 0x00296, 0x000CD, 0x00025, 0x00149, 0x00032, 0x00089, 0x00036, 0x00088, 0x0006F,
	  0x00003, 0x0031D, 0x0000E, 0x001AA, 0x0027E, 0x00061, 0x0014E, 0x0014F, 0x00067, 0x000FF, 0x00183, 0x00036,
	  0x00357, 0x000F5, 0x000C6, 0x000C2, 0x00299, 0x00119, 0x00231, 0x00350, 0x0002C, 0x0018F, 0x00530, 0x00297,
	  0x00004, 0x001B8, 0x000C0, 0x0027A, 0x00311, 0x0009C, 0x00621, 0x00199, 0x0031C, 0x000F7, 0x003E3, 0x00356,
	  0x00189, 0x00005, 0x0006B, 0x008C2, 0x00330, 0x004FF, 0x004F0, 0x00351, 0x004F2, 0x001F2, 0x00373, 0x00000,
	  0x00C41, 0x008C3, 0x009EC, 0x003E2, 0x00304, 0x004F7, 0x004F1, 0x001F0, 0x00148, 0x00C40, 0x009ED, 0x008C0,
	  0x008C1, 0x004F3, 0x004FE, 0x000FE, 0x001F3, 0x001A9},
	 {0x00000, 0x00004, 0x0002F, 0x00052, 0x00010, 0x000AD, 0x0050B, 0x00190, 0x00003, 0x00016, 0x00007, 0x0000D,
	  0x000BB, 0x00173, 0x000C9, 0x0050F, 0x0172C, 0x00003, 0x00011, 0x00005, 0x00043, 0x00023, 0x0004B, 0x0032E,
	  0x02E5B, 0x00482, 0x00009, 0x0002A, 0x00014, 0x0002A, 0x00108, 0x005CA, 0x0065A, 0x02136, 0x02132, 0x0000B,
	  0x00013, 0x00041, 0x000B8, 0x00174, 0x00100, 0x014DA, 0x0404E, 0x01437, 0x0002B, 0x00085, 0x000A7, 0x000A0,
	  0x0014C, 0x0029A, 0x0032C, 0x02133, 0x0142A, 0x00051, 0x00284, 0x000AC, 0x00102, 0x00045, 0x00044, 0x0081B,
	  0x0065E, 0x00CB7, 0x00018, 0x0050C, 0x00212, 0x002E4, 0x00203, 0x00094, 0x00122, 0x0081A, 0x00655, 0x00033,
	  0x002BA, 0x00246, 0x00242, 0x00A6E, 0x0040C, 0x00808, 0x02134, 0x0404F, 0x00175, 0x00405, 0x00247, 0x0012A,
	  0x00A14, 0x002BB, 0x00191, 0x0084F, 0x01438, 0x000AF, 0x00B97, 0x00483, 0x0143B, 0x0032B, 0x00243, 0x0142B,
	  0x00958, 0x029BF, 0x00049, 0x00A6C, 0x014DB, 0x004AD, 0x014DE, 0x0084E, 0x01434, 0x00257, 0x02E5A, 0x00207,
	  0x01435, 0x01439, 0x00CB6, 0x0143A, 0x00194, 0x00654, 0x02135, 0x0537C, 0x0015C, 0x00240, 0x01012, 0x0537D,
	  0x00959, 0x01098, 0x01436, 0x0065F, 0x02026, 0x02137},
	 {0x00005, 0x00019, 0x00016, 0x00011, 0x0003E, 0x0005E, 0x000EF, 0x000E2, 0x00000, 0x00039, 0x0002B, 0x00026,
	  0x00028, 0x00012, 0x000C2, 0x000ED, 0x0011D, 0x0000D, 0x00031, 0x0002A, 0x00025, 0x00020, 0x0005C, 0x001ED,
	  0x0024D, 0x00770, 0x00006, 0x0007A, 0x00060, 0x0004F, 0x00048, 0x00039, 0x00186, 0x00213, 0x00EC6, 0x0000F,
	  0x00026, 0x0005F, 0x00075, 0x00070, 0x00027, 0x001DB, 0x003C6, 0x0078F, 0x0003F, 0x000A6, 0x000F0, 0x0003A,
	  0x00052, 0x0004E, 0x000E3, 0x001D9, 0x0030F, 0x00010, 0x001DD, 0x000A7, 0x000F7, 0x00022, 0x00092, 0x003C4,
	  0x002EF, 0x00762, 0x00079, 0x0008F, 0x001DA, 0x00087, 0x000E8, 0x000BA, 0x00176, 0x000EE, 0x003B0, 0x00085,
	  0x00119, 0x0030E, 0x00108, 0x001D2, 0x0010C, 0x00773, 0x00424, 0x00434, 0x00071, 0x005DD, 0x001C1, 0x003A7,
	  0x00127, 0x0008D, 0x0021B, 0x007B2, 0x001DF, 0x003D8, 0x00764, 0x00EE4, 0x003B3, 0x0074D, 0x001D8, 0x005DC,
	  0x0084A, 0x00499, 0x003C5, 0x01D8E, 0x00765, 0x00435, 0x00771, 0x001C2, 0x00118, 0x003BC, 0x00381, 0x00387,
	  0x07B33, 0x01097, 0x01096, 0x01ECD, 0x00E99, 0x00F1C, 0x00F1D, 0x00EE5, 0x0011C, 0x07B32, 0x03D98, 0x01D8F,
	  0x00E98, 0x00F67, 0x003BD, 0x00380, 0x00498, 0x00386},
	 {0x0000D, 0x00010, 0x0002E, 0x00039, 0x0000D, 0x00074, 0x000ED, 0x000B6, 0x00001, 0x00002, 0x00000, 0x00030,
	  0x00029, 0x00070, 0x000F3, 0x0008C, 0x00166, 0x00009, 0x00033, 0x00078, 0x00006, 0x000C4, 0x0000B, 0x00163,
	  0x000CC, 0x005BE, 0x0001F, 0x0002F, 0x00064, 0x00018, 0x000C6, 0x0000A, 0x00162, 0x002C0, 0x00EF3, 0x00007,
	  0x0000F, 0x000E3, 0x000CA, 0x000B2, 0x0018F, 0x003AE, 0x0075F, 0x00C51, 0x00015, 0x00047, 0x000EE, 0x000E2,
	  0x000EA, 0x00009, 0x0016A, 0x002C3, 0x0059D, 0x0003D, 0x00008, 0x001D9, 0x00032, 0x0000E, 0x0016E, 0x0032C,
	  0x0065B, 0x0196B, 0x00002, 0x0000F, 0x001D8, 0x0008D, 0x000B4, 0x001E4, 0x00067, 0x00317, 0x00794, 0x00022,
	  0x003BE, 0x00315, 0x00034, 0x00037, 0x002DE, 0x0006C, 0x00EFE, 0x0066C, 0x00028, 0x003CB, 0x003AC, 0x00035,
	  0x0016B, 0x003BD, 0x002C1, 0x0062C, 0x01DFE, 0x0000E, 0x0059E, 0x005BF, 0x000DA, 0x00629, 0x00584, 0x00EB7,
	  0x00B0A, 0x0066D, 0x0000C, 0x0077E, 0x0059C, 0x00778, 0x0075E, 0x0075A, 0x0062D, 0x00337, 0x00334, 0x00197,
	  0x01E57, 0x01DE4, 0x0196A, 0x01E56, 0x00C50, 0x00B3F, 0x01E54, 0x00B0B, 0x0018E, 0x001B6, 0x01E55, 0x00CB4,
	  0x00B3E, 0x00EB6, 0x01DE5, 0x01DFF, 0x00335, 0x001B7},
	 {0x00001, 0x0000B, 0x00019, 0x0006F, 0x0002A, 0x00075, 0x007EB, 0x00163, 0x00001, 0x0000E, 0x0001A, 0x0003E,
	  0x0001C, 0x0002D, 0x00164, 0x007EC, 0x00165, 0x00004, 0x00006, 0x00036, 0x0007F, 0x000AE, 0x00158, 0x0015C,
	  0x0056D, 0xFD510, 0x00000, 0x00004, 0x0007B, 0x000F3, 0x0003B, 0x007ED, 0x002B3, 0x002CC, 0x0056E, 0x00018,
	  0x0003E, 0x00017, 0x0001E, 0x000AF, 0x003F7, 0x0056F, 0x002CD, 0xFD511, 0x00014, 0x000AD, 0x000AA, 0x00014,
	  0x000A8, 0x00153, 0x000E8, 0x001FE, 0x00DCF, 0x00078, 0x001B8, 0x00152, 0x000FE, 0x002B1, 0x0015D, 0x00160,
	  0xFD512, 0xFD513, 0x0007A, 0x002B0, 0x001E5, 0x000E9, 0x000FC, 0x006E6, 0x00DC8, 0x00584, 0xFD514, 0x000AB,
	  0x00DDE, 0x00159, 0x003F4, 0x00DC9, 0x00DCA, 0x001FA, 0xFD515, 0xFD516, 0x000FC, 0x001FF, 0x001E4, 0x000AF,
	  0x0015A, 0x00167, 0x00DCB, 0x00585, 0xFD517, 0x003F7, 0x03F55, 0xFD518, 0x00DDC, 0x00586, 0x03F56, 0xFD519,
	  0x03F57, 0xFD51A, 0x001BA, 0x00587, 0x00588, 0x00DDF, 0x002B2, 0xFD51B, 0x00DCE, 0x003F6, 0xFD51C, 0x00FD4,
	  0xFD51D, 0xFD51E, 0xFD51F, 0x7EA80, 0x7EA81, 0x0056C, 0x7EA82, 0x7EA83, 0x00376, 0x00589, 0x0058A, 0x7EA84,
	  0x7EA85, 0x00DDD, 0x7EA86, 0x7EA87, 0x0058B, 0x07EA9},
	 {0x00003, 0x0000E, 0x0000F, 0x0007E, 0x00062, 0x000C6, 0x00CD9, 0x0063E, 0x00002, 0x00002, 0x00000, 0x00018,
	  0x0000C, 0x00069, 0x00039, 0x00707, 0x00C7E, 0x00002, 0x0000D, 0x0001B, 0x0000F, 0x0019A, 0x00647, 0x01A37,
	  0x346C4, 0x0346D, 0x00001, 0x0001E, 0x0007F, 0x0000A, 0x000E1, 0x00661, 0x00CE4, 0x346C5, 0x346C6, 0x0001D,
	  0x00030, 0x0000D, 0x000CB, 0x00199, 0x00320, 0x0008E, 0x0652E, 0x346C7, 0x0003E, 0x00039, 0x00035, 0x00033,
	  0x0019F, 0x001C0, 0x00CDA, 0x346C8, 0x346C9, 0x0000B, 0x000D0, 0x0019E, 0x00022, 0x00038, 0x0018E, 0x0031E,
	  0x03294, 0x0023C, 0x00032, 0x00012, 0x00013, 0x00071, 0x0019D, 0x00020, 0x00C87, 0x00CC0, 0x346CA, 0x00338,
	  0x00653, 0x001A2, 0x0032A, 0x00322, 0x00CE7, 0x00084, 0x0011F, 0x346CB, 0x00325, 0x00649, 0x0032B, 0x00077,
	  0x00648, 0x00642, 0x00C86, 0x00C8C, 0x346CC, 0x0003A, 0x019B7, 0x00043, 0x00327, 0x0008C, 0x0008D, 0x00C8D,
	  0x346CD, 0x346CE, 0x00337, 0x00CE5, 0x00085, 0x00326, 0x00347, 0x00CA4, 0x00C7F, 0x00D1A, 0x346CF, 0x00328,
	  0x1A360, 0x1A361, 0x00CD8, 0x0068C, 0x03295, 0x03296, 0x0652F, 0x066D8, 0x00331, 0x00706, 0x0023D, 0x00076,
	  0x00CC1, 0x00382, 0x00CE6, 0x066D9, 0x066DA, 0x066DB}};

static const uint8 vc1_2ref_mvdata_bits[8][126] = {
	{4,  5,  5,  5,  6,  7,  8,  10, 2,  5,  5,  6,  6,  7,  8,  9,  10, 4,  5,  6,  6,  7,  8,  9,  10, 11,
	 4,  6,  6,  7,  7,  9,  9,  10, 12, 5,  8,  8,  8,  8,  9,  9,  10, 12, 5,  7,  8,  7,  7,  8,  9,  9,
	 11, 7,  9,  10, 9,  10, 10, 10, 10, 12, 6,  9,  9,  9,  9,  9,  10, 10, 11, 7,  10, 10, 11, 11, 11, 12,
	 12, 14, 8,  11, 10, 11, 11, 11, 11, 12, 12, 8,  12, 11, 11, 12, 12, 12, 12, 13, 8,  12, 11, 11, 12, 12,
	 12, 13, 12, 9,  14, 13, 11, 13, 12, 13, 12, 13, 9,  13, 13, 12, 12, 13, 13, 13, 13, 13},
	{3,  4,  5,  6,  8,  10, 11, 11, 2,  5,  5,  6,  7,  8,  10, 11, 11, 4,  5,  5,  6,  7,  9,  12, 13, 13,
	 4,  6,  7,  7,  9,  10, 11, 13, 14, 5,  7,  7,  7,  8,  9,  11, 13, 13, 6,  8,  8,  8,  8,  9,  10, 12,
	 13, 7,  9,  8,  8,  8,  9,  11, 12, 13, 7,  9,  9,  9,  9,  9,  10, 11, 13, 8,  10, 10, 10, 10, 10, 11,
	 12, 13, 9,  11, 11, 10, 10, 10, 11, 11, 12, 10, 12, 12, 12, 11, 11, 11, 12, 12, 10, 13, 12, 12, 11, 11,
	 11, 12, 12, 10, 13, 13, 12, 13, 11, 12, 11, 12, 10, 14, 13, 13, 12, 12, 12, 11, 11, 11},
	{4,  4,  5,  5,  6,  7,  8,  9,  2,  5,  6,  6,  6,  7,  7,  9,  9,  4,  6,  6,  6,  7,  8,  9,  11, 12,
	 5,  7,  7,  7,  9,  9,  10, 11, 12, 5,  7,  7,  7,  7,  9,  9,  10, 12, 5,  8,  8,  8,  8,  8,  9,  10,
	 10, 6,  9,  8,  8,  8,  8,  9,  9,  11, 6,  10, 10, 9,  9,  9,  9,  10, 10, 7,  11, 10, 9,  9,  10, 9,
	 10, 11, 7,  10, 11, 10, 10, 10, 9,  10, 11, 8,  12, 11, 11, 10, 11, 11, 10, 10, 8,  12, 12, 11, 11, 11,
	 11, 10, 11, 8,  13, 12, 12, 11, 11, 11, 11, 10, 9,  13, 12, 12, 12, 11, 11, 10, 10, 10},
	{3,  4,  6,  7,  7,  9,  11, 11, 2,  5,  5,  6,  8,  9,  10, 11, 13, 3,  5,  5,  7,  8,  9,  12, 14, 13,
	 4,  6,  6,  7,  9,  11, 13, 14, 14, 5,  7,  7,  8,  9,  9,  13, 15, 13, 6,  8,  8,  8,  9,  10, 12, 14,
	 13, 7,  10, 9,  9,  9,  9,  12, 13, 14, 7,  11, 10, 10, 10, 10, 11, 12, 13, 8,  11, 12, 12, 12, 11, 12,
	 14, 15, 9,  11, 12, 11, 12, 11, 11, 12, 13, 9,  12, 13, 13, 12, 12, 13, 14, 14, 9,  12, 13, 13, 13, 12,
	 13, 12, 14, 10, 13, 13, 14, 13, 11, 13, 14, 15, 10, 12, 13, 15, 14, 13, 13, 13, 14, 14},
	{4,  5,  5,  5,  6,  7,  8,  8,  2,  6,  6,  6,  6,  6,  8,  9,  10, 4,  6,  6,  6,  6,  7,  9,  10, 11,
	 4,  7,  7,  7,  7,  7,  9,  10, 12, 5,  7,  7,  7,  7,  7,  9,  10, 11, 6,  8,  8,  7,  7,  7,  8,  9,
	 10, 6,  9,  8,  8,  7,  8,  10, 10, 11, 7,  9,  9,  8,  8,  8,  9,  9,  10, 8,  10, 10, 9,  9,  9,  11,
	 11, 11, 8,  11, 10, 10, 9,  9,  10, 11, 10, 10, 12, 12, 11, 11, 10, 11, 12, 11, 10, 13, 12, 11, 11, 10,
	 10, 11, 11, 11, 15, 13, 13, 13, 12, 12, 12, 12, 10, 15, 14, 13, 12, 12, 11, 11, 11, 11},
	{4,  5,  6,  6,  6,  7,  8,  8,  2,  4,  5,  6,  6,  7,  8,  8,  9,  4,  6,  7,  7,  8,  8,  9,  10, 11,
	 5,  6,  7,  7,  8,  8,  9,  10, 12, 5,  7,  8,  8,  8,  9,  10, 11, 12, 5,  7,  8,  8,  8,  8,  9,  10,
	 11, 6,  8,  9,  8,  8,  9,  10, 11, 13, 5,  8,  9,  8,  8,  9,  9,  10, 11, 6,  10, 10, 9,  9,  10, 10,
	 12, 13, 6,  10, 10, 9,  9,  10, 10, 11, 13, 7,  11, 11, 11, 11, 11, 12, 12, 13, 7,  11, 11, 11, 11, 11,
	 11, 12, 12, 9,  13, 13, 13, 13, 12, 12, 13, 12, 9,  12, 13, 12, 12, 12, 13, 13, 12, 12},
	{3,  5,  6,  8,  9,  10, 12, 12, 1,  5,  6,  7,  8,  9,  12, 12, 12, 4,  6,  7,  8,  9,  12, 12, 14, 21,
	 4,  6,  8,  9,  9,  12, 13, 13, 14, 6,  9,  8,  8,  9,  13, 14, 13, 21, 6,  9,  9,  8,  9,  10, 11, 12,
	 13, 8,  10, 10, 11, 11, 12, 12, 21, 21, 8,  11, 10, 11, 11, 12, 13, 14, 21, 9,  13, 10, 11, 13, 13, 12,
	 21, 21, 9,  12, 10, 11, 12, 12, 13, 14, 21, 11, 15, 21, 13, 14, 15, 21, 15, 21, 10, 14, 14, 13, 13, 21,
	 13, 13, 21, 13, 21, 21, 21, 20, 20, 14, 20, 20, 11, 14, 14, 20, 20, 13, 20, 20, 14, 16},
	{2,  5,  6,  8,  9,  10, 13, 13, 2,  4,  5,  6,  8,  9,  10, 13, 14, 3,  5,  7,  8,  10, 12, 15, 20, 16,
	 4,  6,  8,  8,  10, 12, 13, 20, 20, 7,  8,  8,  9,  10, 11, 12, 16, 20, 7,  8,  8,  8,  10, 11, 13, 20,
	 20, 8,  10, 10, 10, 10, 11, 12, 15, 14, 8,  9,  9,  9,  10, 10, 13, 13, 20, 11, 12, 11, 11, 11, 13, 12,
	 13, 20, 11, 12, 11, 11, 12, 12, 13, 13, 20, 10, 14, 11, 11, 12, 12, 13, 20, 20, 11, 13, 12, 11, 12, 13,
	 14, 14, 20, 11, 19, 19, 13, 13, 15, 15, 16, 16, 11, 13, 14, 11, 13, 12, 13, 16, 16, 16}};

/* P-Picture CBPCY VLC tables */
// Looks like original tables are not conforming to standard at all. Are they used for old WMV?
static const uint16 vc1_cbpcy_p_codes[4][64] = {
	{0, 6,  15, 13, 13, 11,  3,  13,  5,  8,  49,  10,  12, 114, 102, 119, 1, 54, 96, 8, 10, 111,
	 5, 15, 12, 10, 2,  12,  13, 115, 53, 63, 1,   7,   1,  7,   14,  12,  4, 14, 1,  9, 97, 11,
	 7, 58, 52, 62, 4,  103, 1,  9,   11, 56, 101, 118, 4,  110, 100, 30,  2, 5,  4,  3},
	{0, 9,  1,  18, 5, 14, 237, 26,  3,   121, 3,   22,  13, 16, 6,   30, 2,  10, 1, 20,  12, 241,
	 5, 28, 16, 12, 3, 24, 28,  124, 239, 247, 1,   240, 1,  19, 18,  15, 4,  27, 1, 122, 2,  23,
	 1, 17, 7,  31, 1, 11, 2,   21,  19,  246, 238, 29,  17, 13, 236, 25, 58, 63, 8, 125},
	{0,  201, 25,  231, 5,  221, 1, 3,   2,   414, 2,  241, 16, 225, 195, 492, 2, 412, 1,  240, 7, 224,
	 98, 245, 1,   220, 96, 5,   9, 230, 101, 247, 1,  102, 1,  415, 24,  3,   2, 244, 3,  54,  3, 484,
	 17, 114, 200, 493, 3,  413, 1, 4,   13,  113, 99, 485, 4,  111, 194, 243, 5, 29,  26, 31},
	{0,  28, 12, 44, 3,  36, 20, 52, 2,  32, 16, 48, 8,  40, 24, 28, 1,  30, 14, 46, 6,  38,
	 22, 54, 3,  34, 18, 50, 10, 42, 26, 30, 1,  29, 13, 45, 5,  37, 21, 53, 2,  33, 17, 49,
	 9,  41, 25, 29, 1,  31, 15, 47, 7,  39, 23, 55, 4,  35, 19, 51, 11, 43, 27, 31}};

static const uint8 vc1_cbpcy_p_bits[4][64] = {
	{13, 13, 7, 13, 7, 13, 13, 12, 6, 13, 7, 12, 6, 8, 8, 8, 5, 7, 8,  12, 6, 8, 13, 12, 7, 13, 13, 12, 6, 8, 7, 7,
	 6,  13, 8, 12, 7, 13, 13, 12, 7, 13, 8, 12, 5, 7, 7, 7, 6, 8, 13, 12, 6, 7, 8,  8,  5, 8,  8,  6,  3, 3, 3, 2},
	{14, 13, 8,  13, 3,  13, 8, 13, 3, 7, 8, 13, 4,  13, 13, 13, 3,  13, 13, 13, 4,  8,
	 13, 13, 5,  13, 13, 13, 5, 7,  8, 8, 3, 8,  14, 13, 5,  13, 13, 13, 4,  7,  13, 13,
	 6,  13, 13, 13, 5,  13, 8, 13, 5, 8, 8, 13, 5,  13, 8,  13, 6,  6,  13, 7},
	{13, 8, 6, 8, 4, 8,  13, 12, 4, 9, 8, 8, 5, 8, 8, 9, 5, 9, 10, 8,  4, 8, 7, 8, 6, 8, 7, 13, 4, 8, 7, 8,
	 5,  7, 8, 9, 6, 13, 13, 8,  4, 6, 8, 9, 5, 7, 8, 9, 5, 9, 9,  13, 5, 7, 7, 9, 4, 7, 8, 8,  3, 5, 5, 5},
	{9, 9, 9, 9, 2, 9, 9, 9, 2, 9, 9, 9, 9, 9, 9, 8, 3, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 8,
	 2, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 8}};

/* Interlaced CBPCY VLC tables (Table 124 - Table 131) */
static const uint16 vc1_icbpcy_p_codes[8][63] = {
	{0x2F1A, 0x2F1B, 0x178C, 0x0090, 0x02A8, 0x02A9, 0x0BC7, 0x0091, 0x02AA, 0x02AB, 0x05E0, 0x004A, 0x0096,
	 0x0097, 0x00BD, 0x0092, 0x02AC, 0x02AD, 0x05E1, 0x0098, 0x0132, 0x0133, 0x0179, 0x0134, 0x026A, 0x026B,
	 0x02FC, 0x004E, 0x0040, 0x0041, 0x002B, 0x0093, 0x02AE, 0x02AF, 0x05E2, 0x0136, 0x026E, 0x026F, 0x02FD,
	 0x009E, 0x013E, 0x013F, 0x017F, 0x0050, 0x0042, 0x0043, 0x002C, 0x0051, 0x00A4, 0x00A5, 0x00BE, 0x0053,
	 0x0044, 0x0045, 0x002D, 0x0054, 0x0046, 0x0047, 0x002E, 0x0003, 0x0000, 0x0001, 0x0001},
	{0x0041, 0x0042, 0x0100, 0x0043, 0x0088, 0x0089, 0x0101, 0x0045, 0x008C, 0x008D, 0x0102, 0x0010, 0x0022,
	 0x0023, 0x0024, 0x0047, 0x0010, 0x0011, 0x0103, 0x0025, 0x0058, 0x0059, 0x005A, 0x005B, 0x005A, 0x005B,
	 0x005C, 0x000C, 0x0030, 0x0031, 0x0019, 0x0009, 0x0014, 0x0015, 0x002C, 0x005C, 0x005D, 0x005E, 0x005F,
	 0x0026, 0x005D, 0x005E, 0x005F, 0x000D, 0x0034, 0x0035, 0x001B, 0x0014, 0x0027, 0x002A, 0x002B, 0x000E,
	 0x0038, 0x0039, 0x001D, 0x000F, 0x003C, 0x003D, 0x001F, 0x0005, 0x0009, 0x0000, 0x0003},
	{0x0032, 0x0033, 0x001A, 0x0026, 0x00E4, 0x00E5, 0x01E6, 0x0027, 0x00E6, 0x00E7, 0x01E7, 0x000E, 0x0063,
	 0x006C, 0x0077, 0x0028, 0x00E8, 0x00E9, 0x01E8, 0x007B, 0x00DA, 0x00DB, 0x00EC, 0x00F5, 0x01B8, 0x01B9,
	 0x01DA, 0x0021, 0x004B, 0x0054, 0x002B, 0x0029, 0x00EA, 0x00EB, 0x01E9, 0x004A, 0x01BA, 0x01BB, 0x01DB,
	 0x0020, 0x00DE, 0x00DF, 0x00F2, 0x0022, 0x0055, 0x0058, 0x002D, 0x000F, 0x0070, 0x0071, 0x0078, 0x0023,
	 0x0059, 0x005C, 0x002F, 0x0024, 0x005D, 0x0062, 0x0030, 0x0002, 0x001F, 0x0006, 0x0000},
	{0x0028, 0x0029, 0x009D, 0x0000, 0x01EA, 0x01EB, 0x01EC, 0x0001, 0x01ED, 0x01EE, 0x01EF, 0x0005, 0x00F0,
	 0x00F1, 0x003B, 0x0002, 0x01F0, 0x01F1, 0x01F2, 0x003F, 0x015C, 0x015D, 0x0099, 0x0010, 0x03D0, 0x03D1,
	 0x0130, 0x000F, 0x009E, 0x009F, 0x00FB, 0x0003, 0x01F3, 0x01F4, 0x01F5, 0x0011, 0x03D2, 0x03D3, 0x0131,
	 0x0009, 0x015E, 0x015F, 0x009C, 0x0010, 0x00A8, 0x00A9, 0x0038, 0x0006, 0x00F2, 0x00F3, 0x004D, 0x0011,
	 0x00AA, 0x00AB, 0x0039, 0x0012, 0x00AC, 0x00AD, 0x003A, 0x0006, 0x0016, 0x0017, 0x000E},
	{0x003C, 0x003D, 0x001F, 0x000A, 0x0061, 0x0062, 0x0002, 0x000B, 0x0063, 0x0064, 0x0003, 0x0007, 0x0003,
	 0x0004, 0x000B, 0x000C, 0x0065, 0x0066, 0x0004, 0x0012, 0x000A, 0x000B, 0x0014, 0x001B, 0x0018, 0x0019,
	 0x0034, 0x002C, 0x0067, 0x0068, 0x0035, 0x000D, 0x0069, 0x006C, 0x0005, 0x0060, 0x001A, 0x001B, 0x0035,
	 0x0013, 0x000E, 0x000F, 0x0015, 0x002D, 0x006D, 0x006E, 0x0038, 0x0008, 0x0008, 0x0009, 0x000C, 0x002E,
	 0x006F, 0x0072, 0x003A, 0x002F, 0x0073, 0x0000, 0x003B, 0x0007, 0x0014, 0x0015, 0x0004},
	{0x0038, 0x0039, 0x009D, 0x000A, 0x0091, 0x0092, 0x0093, 0x000B, 0x0094, 0x0095, 0x0096, 0x0003, 0x00EE,
	 0x00EF, 0x0036, 0x000C, 0x0097, 0x0098, 0x0099, 0x0008, 0x01E4, 0x01E5, 0x006A, 0x0018, 0x03CC, 0x03CD,
	 0x00D6, 0x000E, 0x009E, 0x009F, 0x00F5, 0x000D, 0x009A, 0x009B, 0x009C, 0x0019, 0x03CE, 0x03CF, 0x00D7,
	 0x0009, 0x01E8, 0x01E9, 0x0090, 0x000F, 0x00E8, 0x00E9, 0x00F6, 0x0005, 0x00F0, 0x00F1, 0x0037, 0x0010,
	 0x00EA, 0x00EB, 0x00F7, 0x0011, 0x00EC, 0x00ED, 0x0034, 0x0000, 0x003E, 0x003F, 0x0002},
	{0x003C, 0x003D, 0x01CF, 0x0000, 0x00BF, 0x00E0, 0x01FC, 0x0001, 0x00E1, 0x00E2, 0x01FD, 0x0009, 0x01F1,
	 0x01F2, 0x01F3, 0x0002, 0x00E3, 0x00E4, 0x01FE, 0x0011, 0x03EE, 0x03EF, 0x03F0, 0x0021, 0x07E2, 0x07E3,
	 0x07E4, 0x0018, 0x03F7, 0x03FE, 0x03FF, 0x0003, 0x00E5, 0x00E6, 0x0080, 0x002E, 0x07E5, 0x07E6, 0x07E7,
	 0x0016, 0x03F4, 0x03F5, 0x03F6, 0x0019, 0x0102, 0x0103, 0x0104, 0x000A, 0x01F4, 0x01F5, 0x01F6, 0x001A,
	 0x0105, 0x0106, 0x0107, 0x001B, 0x0178, 0x0179, 0x01CE, 0x001D, 0x00BD, 0x00BE, 0x01F0},
	{0x0003, 0x0004, 0x01B6, 0x0004, 0x002E, 0x002F, 0x000E, 0x0005, 0x0030, 0x0031, 0x000F, 0x0003, 0x000A,
	 0x000B, 0x0014, 0x0006, 0x0032, 0x0033, 0x0010, 0x0005, 0x0030, 0x0031, 0x0032, 0x0009, 0x0066, 0x0067,
	 0x0068, 0x001D, 0x01B7, 0x01B8, 0x01B9, 0x0007, 0x0034, 0x0035, 0x0011, 0x0016, 0x0069, 0x006A, 0x006B,
	 0x000A, 0x0036, 0x0037, 0x00D8, 0x001E, 0x01BA, 0x01BB, 0x01BC, 0x0004, 0x0015, 0x0016, 0x0017, 0x001F,
	 0x01BD, 0x01BE, 0x01BF, 0x0000, 0x0010, 0x0011, 0x0012, 0x001C, 0x00D9, 0x00DA, 0x0013}};

static const uint8 vc1_icbpcy_p_bits[8][63] = {
	{15, 15, 14, 9,  11, 11, 13, 9, 11, 11, 12, 8, 9, 9, 9, 9, 11, 11, 12, 9, 10, 10, 10, 10, 11, 11, 11, 8, 8, 8, 7, 9,
	 11, 11, 12, 10, 11, 11, 11, 9, 10, 10, 10, 8, 8, 8, 7, 8, 9,  9,  9,  8, 8,  8,  7,  8,  8,  8,  7,  3, 3, 3, 1},
	{7, 7, 9, 7, 8, 8, 9, 7, 8, 8, 9, 6, 7, 7, 7, 7, 7, 7, 9, 7, 8, 8, 8, 8, 9, 9, 9, 6, 7, 7, 6, 6,
	 7, 7, 8, 8, 9, 9, 9, 7, 8, 8, 8, 6, 7, 7, 6, 6, 7, 7, 7, 6, 7, 7, 6, 6, 7, 7, 6, 3, 4, 3, 2},
	{6, 6, 5, 6, 8, 8, 9, 6, 8, 8, 9, 5, 7, 7, 7, 6, 8, 8, 9, 7, 8, 8, 8, 8, 9, 9, 9, 6, 7, 7, 6, 6,
	 8, 8, 9, 7, 9, 9, 9, 6, 8, 8, 8, 6, 7, 7, 6, 5, 7, 7, 7, 6, 7, 7, 6, 6, 7, 7, 6, 3, 5, 4, 2},
	{6, 6, 8, 4, 9,  9,  9, 4, 9, 9, 9, 4, 8, 8, 7, 4, 9, 9, 9, 6, 9, 9, 8, 6, 10, 10, 9, 5, 8, 8, 8, 4,
	 9, 9, 9, 6, 10, 10, 9, 5, 9, 9, 8, 5, 8, 8, 7, 4, 8, 8, 7, 5, 8, 8, 7, 5, 8,  8,  7, 3, 5, 5, 4},
	{6, 6, 5, 5, 7, 7, 7, 5, 7, 7, 7, 5, 6, 6, 6, 5, 7, 7, 7, 6, 7, 7, 7, 7, 8, 8, 8, 6, 7, 7, 6, 5,
	 7, 7, 7, 7, 8, 8, 8, 6, 7, 7, 7, 6, 7, 7, 6, 5, 6, 6, 6, 6, 7, 7, 6, 6, 7, 6, 6, 4, 5, 5, 3},
	{6, 6, 8, 4, 8,  8,  8, 4, 8, 8, 8, 4, 8, 8, 7, 4, 8, 8, 8, 5, 9, 9, 8, 6, 10, 10, 9, 5, 8, 8, 8, 4,
	 8, 8, 8, 6, 10, 10, 9, 5, 9, 9, 8, 5, 8, 8, 8, 4, 8, 8, 7, 5, 8, 8, 8, 5, 8,  8,  7, 3, 6, 6, 4},
	{6, 6, 9, 3, 8,  8,  9,  3, 8,  8,  9,  4, 9, 9, 9, 3, 8, 8, 9, 5, 10, 10, 10, 6, 11, 11, 11, 5, 10, 10, 10, 3,
	 8, 8, 8, 6, 11, 11, 11, 5, 10, 10, 10, 5, 9, 9, 9, 4, 9, 9, 9, 5, 9,  9,  9,  5, 9,  9,  9,  5, 8,  8,  9},
	{6, 6, 10, 3, 7,  7,  7,  3, 7, 7, 7, 4, 8,  8,  8,  3, 7, 7, 7, 5, 9,  9,  9,  6, 10, 10, 10, 6, 10, 10, 10, 3,
	 7, 7, 7,  6, 10, 10, 10, 5, 9, 9, 9, 6, 10, 10, 10, 4, 8, 8, 8, 6, 10, 10, 10, 5, 9,  9,  9,  6, 9,  9,  9}};

/* MacroBlock Transform Type: 7.1.3.11, p89
 * 8x8:B
 * 8x4:B:btm  8x4:B:top  8x4:B:both,
 * 4x8:B:right  4x8:B:left  4x8:B:both
 * 4x4:B  8x8:MB
 * 8x4:MB:btm  8x4:MB:top  8x4,MB,both
 * 4x8,MB,right  4x8,MB,left
 * 4x4,MB                               */
static const uint16 vc1_ttmb_codes[3][16] = {{0x0003, 0x002E, 0x005F, 0x0000, 0x0016, 0x0015, 0x0001, 0x0004, 0x0014,
											  0x02F1, 0x0179, 0x017B, 0x0BC0, 0x0BC1, 0x05E1, 0x017A},
											 {0x0006, 0x0006, 0x0003, 0x0007, 0x000F, 0x000E, 0x0000, 0x0002, 0x0002,
											  0x0014, 0x0011, 0x000B, 0x0009, 0x0021, 0x0015, 0x0020},
											 {0x0006, 0x0000, 0x000E, 0x0005, 0x0002, 0x0003, 0x0003, 0x000F, 0x0002,
											  0x0081, 0x0021, 0x0009, 0x0101, 0x0041, 0x0011, 0x0100}};

static const uint8 vc1_ttmb_bits[3][16] = {{2, 6, 7, 2, 5, 5, 2, 3, 5, 10, 9, 9, 12, 12, 11, 9},
										   {3, 4, 4, 4, 4, 4, 3, 3, 2, 7, 7, 6, 6, 8, 7, 8},
										   {3, 3, 4, 5, 3, 3, 4, 4, 2, 10, 8, 6, 11, 9, 7, 11}};

/* TTBLK (Transform Type per Block) tables */
static const uint8 vc1_ttblk_codes[3][8] = {
	{0, 1, 3, 5, 16, 17, 18, 19}, {3, 0, 1, 2, 3, 5, 8, 9}, {1, 0, 1, 4, 6, 7, 10, 11}};
static const uint8 vc1_ttblk_bits[3][8] = {
	{2, 2, 2, 3, 5, 5, 5, 5}, {2, 3, 3, 3, 3, 3, 4, 4}, {2, 3, 3, 3, 3, 3, 4, 4}};

/* SUBBLKPAT tables, p93-94, reordered */
static const uint8 vc1_subblkpat_codes[3][15] = {{14, 12, 7, 11, 9, 26, 2, 10, 27, 8, 0, 6, 1, 15, 1},
												 {14, 0, 8, 15, 10, 4, 23, 13, 5, 9, 25, 3, 24, 22, 1},
												 {5, 6, 2, 2, 8, 0, 28, 3, 1, 3, 29, 1, 19, 18, 15}};
static const uint8 vc1_subblkpat_bits[3][15] = {{5, 5, 5, 5, 5, 6, 4, 5, 6, 5, 4, 5, 4, 5, 1},
												{4, 3, 4, 4, 4, 5, 5, 4, 5, 4, 5, 4, 5, 5, 2},
												{3, 3, 4, 3, 4, 5, 5, 3, 5, 4, 5, 4, 5, 5, 4}};

/* MV differential tables, p265 */
static const uint16 vc1_mv_diff_codes[4][73] =
	{
		{
			0,  2,   3,   8,   576, 3,   2,   6,   5,   577, 578, 7,   8,   9,   40,  19,  37,  82, 21,
			22, 23,  579, 580, 166, 96,  167, 49,  194, 195, 581, 582, 583, 292, 293, 294, 13,  2,  7,
			24, 50,  102, 295, 13,  7,   8,   18,  50,  103, 38,  20,  21,  22,  39,  204, 103, 23, 24,
			25, 104, 410, 105, 106, 107, 108, 109, 220, 411, 442, 222, 443, 446, 447, 7 /* 73 elements */
		},
		{
			0,    4,    5,    3,    4,    3,    4,    5,    20,   6,    21,   44,   45,   46,   3008,
			95,   112,  113,  57,   3009, 3010, 116,  117,  3011, 118,  3012, 3013, 3014, 3015, 3016,
			3017, 3018, 3019, 3020, 3021, 3022, 1,    4,    15,   160,  161,  41,   6,    11,   42,
			162,  43,   119,  56,   57,   58,   163,  236,  237,  3023, 119,  120,  242,  122,  486,
			1512, 487,  246,  494,  1513, 495,  1514, 1515, 1516, 1517, 1518, 1519, 31 /* 73 elements */
		},
		{
			0,   512, 513, 514, 515, 2,   3,   258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269,
			270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 1,   5,
			287, 288, 289, 290, 6,   7,   291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303,
			304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319 /* 73 elements */
		},
		{
			0,  1,   1,   2,  3,   4,  1,  5,  4,   3,   5,   8,   6,   9,   10,  11,  12, 7,  104,
			14, 105, 4,   10, 15,  11, 6,  14, 8,   106, 107, 108, 15,  109, 9,   55,  10, 1,  2,
			1,  2,   3,   12, 6,   2,  6,  7,  28,  7,   15,  8,   5,   18,  29,  152, 77, 24, 25,
			26, 39,  108, 13, 109, 55, 56, 57, 116, 11,  153, 234, 235, 118, 119, 15 /* 73 elements */
		}};
static const uint8 vc1_mv_diff_bits[4][73] = {
	{
		6,  7, 7,  8,  14, 6,  5,  6,  7,  14, 14, 6, 6, 6, 8, 9, 10, 9, 7, 7, 7, 14, 14, 10, 9,
		10, 8, 10, 10, 14, 14, 14, 13, 13, 13, 6,  3, 5, 6, 8, 9, 13, 5, 4, 4, 5, 7,  9,  6,  5,
		5,  5, 6,  9,  8,  5,  5,  5,  7,  10, 7,  7, 7, 7, 7, 8, 10, 9, 8, 9, 9, 9,  3 /* 73 elements */
	},
	{
		5,  7,  7,  6,  6,  5,  5,  6,  7,  5,  7,  8, 8, 8, 14, 9, 9,  9,  8,  14, 14, 9,  9, 14, 9,
		14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 2, 3, 6, 8,  8, 6,  3,  4,  6,  8,  6,  9, 6,  6,
		6,  8,  8,  8,  14, 7,  7,  8,  7,  9,  13, 9, 8, 9, 13, 9, 13, 13, 13, 13, 13, 13, 5 /* 73 elements */

	},
	{
		3,  12, 12, 12, 12, 3,  4,  11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
		11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 1,  5,  11, 11, 11, 11, 4,  4,  11, 11, 11, 11, 11, 11,
		11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11 /* 73 elements */
	},
	{
		15, 11, 15, 15, 15, 15, 12, 15, 12, 11, 12, 12, 15, 12, 12, 12, 12, 15, 15, 12, 15, 10, 11, 12, 11,
		10, 11, 10, 15, 15, 15, 11, 15, 10, 14, 10, 4,  4,  5,  7,  8,  9,  5,  3,  4,  5,  6,  8,  5,  4,
		3,  5,  6,  8,  7,  5,  5,  5,  6,  7,  9,  7,  6,  6,  6,  7,  10, 8,  8,  8,  7,  7,  4 /* 73 elements */
	}};

static const uint32 vc1_ac_tables[AC_MODES][186][2] = {
	{{0x0001, 2},  {0x0005, 3},  {0x000D, 4},  {0x0012, 5},  {0x000E, 6},  {0x0015, 7},  {0x0013, 8},  {0x003F, 8},
	 {0x004B, 9},  {0x011F, 9},  {0x00B8, 10}, {0x03E3, 10}, {0x0172, 11}, {0x024D, 12}, {0x03DA, 12}, {0x02DD, 13},
	 {0x1F55, 13}, {0x05B9, 14}, {0x3EAE, 14}, {0x0000, 4},  {0x0010, 5},  {0x0008, 7},  {0x0020, 8},  {0x0029, 9},
	 {0x01F4, 9},  {0x0233, 10}, {0x01E0, 11}, {0x012A, 12}, {0x03DD, 12}, {0x050A, 13}, {0x1F29, 13}, {0x0A42, 14},
	 {0x1272, 15}, {0x1737, 15}, {0x0003, 5},  {0x0011, 7},  {0x00C4, 8},  {0x004B, 10}, {0x00B4, 11}, {0x07D4, 11},
	 {0x0345, 12}, {0x02D7, 13}, {0x07BF, 13}, {0x0938, 14}, {0x0BBB, 14}, {0x095E, 15}, {0x0013, 5},  {0x0078, 7},
	 {0x0069, 9},  {0x0232, 10}, {0x0461, 11}, {0x03EC, 12}, {0x0520, 13}, {0x1F2A, 13}, {0x3E50, 14}, {0x3E51, 14},
	 {0x1486, 15}, {0x000C, 6},  {0x0024, 9},  {0x0094, 11}, {0x08C0, 12}, {0x0F09, 14}, {0x1EF0, 15}, {0x003D, 6},
	 {0x0053, 9},  {0x01A0, 11}, {0x02D6, 13}, {0x0F08, 14}, {0x0013, 7},  {0x007C, 9},  {0x07C1, 11}, {0x04AC, 14},
	 {0x001B, 7},  {0x00A0, 10}, {0x0344, 12}, {0x0F79, 14}, {0x0079, 7},  {0x03E1, 10}, {0x02D4, 13}, {0x2306, 14},
	 {0x0021, 8},  {0x023C, 10}, {0x0FAE, 12}, {0x23DE, 14}, {0x0035, 8},  {0x0175, 11}, {0x07B3, 13}, {0x00C5, 8},
	 {0x0174, 11}, {0x0785, 13}, {0x0048, 9},  {0x01A3, 11}, {0x049E, 13}, {0x002C, 9},  {0x00FA, 10}, {0x07D6, 11},
	 {0x0092, 10}, {0x05CC, 13}, {0x1EF1, 15}, {0x00A3, 10}, {0x03ED, 12}, {0x093E, 14}, {0x01E2, 11}, {0x1273, 15},
	 {0x07C4, 11}, {0x1487, 15}, {0x0291, 12}, {0x0293, 12}, {0x0F8A, 12}, {0x0509, 13}, {0x0508, 13}, {0x078D, 13},
	 {0x07BE, 13}, {0x078C, 13}, {0x04AE, 14}, {0x0BBA, 14}, {0x2307, 14}, {0x0B9A, 14}, {0x1736, 15}, {0x000E, 4},
	 {0x0045, 7},  {0x01F3, 9},  {0x047A, 11}, {0x05DC, 13}, {0x23DF, 14}, {0x0019, 5},  {0x0028, 9},  {0x0176, 11},
	 {0x049D, 13}, {0x23DD, 14}, {0x0030, 6},  {0x00A2, 10}, {0x02EF, 12}, {0x05B8, 14}, {0x003F, 6},  {0x00A5, 10},
	 {0x03DB, 12}, {0x093F, 14}, {0x0044, 7},  {0x07CB, 11}, {0x095F, 15}, {0x0063, 7},  {0x03C3, 12}, {0x0015, 8},
	 {0x08F6, 12}, {0x0017, 8},  {0x0498, 13}, {0x002C, 8},  {0x07B2, 13}, {0x002F, 8},  {0x1F54, 13}, {0x008D, 8},
	 {0x07BD, 13}, {0x008E, 8},  {0x1182, 13}, {0x00FB, 8},  {0x050B, 13}, {0x002D, 8},  {0x07C0, 11}, {0x0079, 9},
	 {0x1F5F, 13}, {0x007A, 9},  {0x1F56, 13}, {0x0231, 10}, {0x03E4, 10}, {0x01A1, 11}, {0x0143, 11}, {0x01F7, 11},
	 {0x016F, 12}, {0x0292, 12}, {0x02E7, 12}, {0x016C, 12}, {0x016D, 12}, {0x03DC, 12}, {0x0F8B, 12}, {0x0499, 13},
	 {0x03D8, 12}, {0x078E, 13}, {0x02D5, 13}, {0x1F5E, 13}, {0x1F2B, 13}, {0x078F, 13}, {0x04AD, 14}, {0x3EAF, 14},
	 {0x23DC, 14}, {0x004A, 9}},
	{{0x0000, 3},  {0x0003, 4},  {0x000B, 5},  {0x0014, 6},  {0x003F, 6},  {0x005D, 7},  {0x00A2, 8},  {0x00AC, 9},
	 {0x016E, 9},  {0x020A, 10}, {0x02E2, 10}, {0x0432, 11}, {0x05C9, 11}, {0x0827, 12}, {0x0B54, 12}, {0x04E6, 13},
	 {0x105F, 13}, {0x172A, 13}, {0x20B2, 14}, {0x2D4E, 14}, {0x39F0, 14}, {0x4175, 15}, {0x5A9E, 15}, {0x0004, 4},
	 {0x001E, 5},  {0x0042, 7},  {0x00B6, 8},  {0x0173, 9},  {0x0395, 10}, {0x072E, 11}, {0x0B94, 12}, {0x16A4, 13},
	 {0x20B3, 14}, {0x2E45, 14}, {0x0005, 5},  {0x0040, 7},  {0x0049, 9},  {0x028F, 10}, {0x05CB, 11}, {0x048A, 13},
	 {0x09DD, 14}, {0x73E2, 15}, {0x0018, 5},  {0x0025, 8},  {0x008A, 10}, {0x051B, 11}, {0x0E5F, 12}, {0x09C9, 14},
	 {0x139C, 15}, {0x0029, 6},  {0x004F, 9},  {0x0412, 11}, {0x048D, 13}, {0x2E41, 14}, {0x0038, 6},  {0x010E, 9},
	 {0x05A8, 11}, {0x105C, 13}, {0x39F2, 14}, {0x0058, 7},  {0x021F, 10}, {0x0E7E, 12}, {0x39FF, 14}, {0x0023, 8},
	 {0x02E3, 10}, {0x04E5, 13}, {0x2E40, 14}, {0x00A1, 8},  {0x05BE, 11}, {0x09C8, 14}, {0x0083, 8},  {0x013A, 11},
	 {0x1721, 13}, {0x0044, 9},  {0x0276, 12}, {0x39F6, 14}, {0x008B, 10}, {0x04EF, 13}, {0x5A9B, 15}, {0x0208, 10},
	 {0x1CFE, 13}, {0x0399, 10}, {0x1CB4, 13}, {0x039E, 10}, {0x39F3, 14}, {0x05AB, 11}, {0x73E3, 15}, {0x0737, 11},
	 {0x5A9F, 15}, {0x082D, 12}, {0x0E69, 12}, {0x0E68, 12}, {0x0433, 11}, {0x0B7B, 12}, {0x2DF8, 14}, {0x2E56, 14},
	 {0x2E57, 14}, {0x39F7, 14}, {0x51A5, 15}, {0x0003, 3},  {0x002A, 6},  {0x00E4, 8},  {0x028E, 10}, {0x0735, 11},
	 {0x1058, 13}, {0x1CFA, 13}, {0x2DF9, 14}, {0x4174, 15}, {0x0009, 4},  {0x0054, 8},  {0x0398, 10}, {0x048B, 13},
	 {0x139D, 15}, {0x000D, 4},  {0x00AD, 9},  {0x0826, 12}, {0x2D4C, 14}, {0x0011, 5},  {0x016B, 9},  {0x0B7F, 12},
	 {0x51A4, 15}, {0x0019, 5},  {0x021B, 10}, {0x16FD, 13}, {0x001D, 5},  {0x0394, 10}, {0x28D3, 14}, {0x002B, 6},
	 {0x05BC, 11}, {0x5A9A, 15}, {0x002F, 6},  {0x0247, 12}, {0x0010, 7},  {0x0A35, 12}, {0x003E, 6},  {0x0B7A, 12},
	 {0x0059, 7},  {0x105E, 13}, {0x0026, 8},  {0x09CF, 14}, {0x0055, 8},  {0x1CB5, 13}, {0x0057, 8},  {0x0E5B, 12},
	 {0x00A0, 8},  {0x1468, 13}, {0x0170, 9},  {0x0090, 10}, {0x01CE, 9},  {0x021A, 10}, {0x0218, 10}, {0x0168, 9},
	 {0x021E, 10}, {0x0244, 12}, {0x0736, 11}, {0x0138, 11}, {0x0519, 11}, {0x0E5E, 12}, {0x072C, 11}, {0x0B55, 12},
	 {0x09DC, 14}, {0x20BB, 14}, {0x048C, 13}, {0x1723, 13}, {0x2E44, 14}, {0x16A5, 13}, {0x0518, 11}, {0x39FE, 14},
	 {0x0169, 9}},
	{{0x0001, 2},  {0x0006, 3},  {0x000F, 4},  {0x0016, 5},  {0x0020, 6},  {0x0018, 7},  {0x0008, 8},  {0x009A, 8},
	 {0x0056, 9},  {0x013E, 9},  {0x00F0, 10}, {0x03A5, 10}, {0x0077, 11}, {0x01EF, 11}, {0x009A, 12}, {0x005D, 13},
	 {0x0001, 4},  {0x0011, 5},  {0x0002, 7},  {0x000B, 8},  {0x0012, 9},  {0x01D6, 9},  {0x027E, 10}, {0x0191, 11},
	 {0x00EA, 12}, {0x03DC, 12}, {0x013B, 13}, {0x0004, 5},  {0x0014, 7},  {0x009E, 8},  {0x0009, 10}, {0x01AC, 11},
	 {0x01E2, 11}, {0x03CA, 12}, {0x005F, 13}, {0x0017, 5},  {0x004E, 7},  {0x005E, 9},  {0x00F3, 10}, {0x01AD, 11},
	 {0x00EC, 12}, {0x05F0, 13}, {0x000E, 6},  {0x00E1, 8},  {0x03A4, 10}, {0x009C, 12}, {0x013D, 13}, {0x003B, 6},
	 {0x001C, 9},  {0x0014, 11}, {0x09BE, 12}, {0x0006, 7},  {0x007A, 9},  {0x0190, 11}, {0x0137, 13}, {0x001B, 7},
	 {0x0008, 10}, {0x075C, 11}, {0x0071, 7},  {0x00D7, 10}, {0x09BF, 12}, {0x0007, 8},  {0x00AF, 10}, {0x04CC, 11},
	 {0x0034, 8},  {0x0265, 10}, {0x009F, 12}, {0x00E0, 8},  {0x0016, 11}, {0x0327, 12}, {0x0015, 9},  {0x017D, 11},
	 {0x0EBB, 12}, {0x0014, 9},  {0x00F6, 10}, {0x01E4, 11}, {0x00CB, 10}, {0x099D, 12}, {0x00CA, 10}, {0x02FC, 12},
	 {0x017F, 11}, {0x04CD, 11}, {0x02FD, 12}, {0x04FE, 11}, {0x013A, 13}, {0x000A, 4},  {0x0042, 7},  {0x01D3, 9},
	 {0x04DD, 11}, {0x0012, 5},  {0x00E8, 8},  {0x004C, 11}, {0x0136, 13}, {0x0039, 6},  {0x0264, 10}, {0x0EBA, 12},
	 {0x0000, 7},  {0x00AE, 10}, {0x099C, 12}, {0x001F, 7},  {0x04DE, 11}, {0x0043, 7},  {0x04DC, 11}, {0x0003, 8},
	 {0x03CB, 12}, {0x0006, 8},  {0x099E, 12}, {0x002A, 8},  {0x05F1, 13}, {0x000F, 8},  {0x09FE, 12}, {0x0033, 8},
	 {0x09FF, 12}, {0x0098, 8},  {0x099F, 12}, {0x00EA, 8},  {0x013C, 13}, {0x002E, 8},  {0x0192, 11}, {0x0136, 9},
	 {0x006A, 9},  {0x0015, 11}, {0x03AF, 10}, {0x01E3, 11}, {0x0074, 11}, {0x00EB, 12}, {0x02F9, 12}, {0x005C, 13},
	 {0x00ED, 12}, {0x03DD, 12}, {0x0326, 12}, {0x005E, 13}, {0x0016, 7}},
	{{0x0004, 3},  {0x0014, 5},  {0x0017, 7},  {0x007F, 8},  {0x0154, 9},  {0x01F2, 10}, {0x00BF, 11}, {0x0065, 12},
	 {0x0AAA, 12}, {0x0630, 13}, {0x1597, 13}, {0x03B7, 14}, {0x2B22, 14}, {0x0BE6, 15}, {0x000B, 4},  {0x0037, 7},
	 {0x0062, 9},  {0x0007, 11}, {0x0166, 12}, {0x00CE, 13}, {0x1590, 13}, {0x05F6, 14}, {0x0BE7, 15}, {0x0007, 5},
	 {0x006D, 8},  {0x0003, 11}, {0x031F, 12}, {0x05F2, 14}, {0x0002, 6},  {0x0061, 9},  {0x0055, 12}, {0x01DF, 14},
	 {0x001A, 6},  {0x001E, 10}, {0x0AC9, 12}, {0x2B23, 14}, {0x001E, 6},  {0x001F, 10}, {0x0AC3, 12}, {0x2B2B, 14},
	 {0x0006, 7},  {0x0004, 11}, {0x02F8, 13}, {0x0019, 7},  {0x0006, 11}, {0x063D, 13}, {0x0057, 7},  {0x0182, 11},
	 {0x2AA2, 14}, {0x0004, 8},  {0x0180, 11}, {0x059C, 14}, {0x007D, 8},  {0x0164, 12}, {0x076D, 15}, {0x0002, 9},
	 {0x018D, 11}, {0x1581, 13}, {0x00AD, 8},  {0x0060, 12}, {0x0C67, 14}, {0x001C, 9},  {0x00EE, 13}, {0x0003, 9},
	 {0x02CF, 13}, {0x00D9, 9},  {0x1580, 13}, {0x0002, 11}, {0x0183, 11}, {0x0057, 12}, {0x0061, 12}, {0x0031, 11},
	 {0x0066, 12}, {0x0631, 13}, {0x0632, 13}, {0x00AC, 13}, {0x031D, 12}, {0x0076, 12}, {0x003A, 11}, {0x0165, 12},
	 {0x0C66, 14}, {0x0003, 2},  {0x0054, 7},  {0x02AB, 10}, {0x0016, 13}, {0x05F7, 14}, {0x0005, 4},  {0x00F8, 9},
	 {0x0AA9, 12}, {0x005F, 15}, {0x0004, 4},  {0x001C, 10}, {0x1550, 13}, {0x0004, 5},  {0x0077, 11}, {0x076C, 15},
	 {0x000E, 5},  {0x000A, 12}, {0x000C, 5},  {0x0562, 11}, {0x0004, 6},  {0x031C, 12}, {0x0006, 6},  {0x00C8, 13},
	 {0x000D, 6},  {0x01DA, 13}, {0x0007, 6},  {0x00C9, 13}, {0x0001, 7},  {0x002E, 14}, {0x0014, 7},  {0x1596, 13},
	 {0x000A, 7},  {0x0AC2, 12}, {0x0016, 7},  {0x015B, 14}, {0x0015, 7},  {0x015A, 14}, {0x000F, 8},  {0x005E, 15},
	 {0x007E, 8},  {0x00AB, 8},  {0x002D, 9},  {0x00D8, 9},  {0x000B, 9},  {0x0014, 10}, {0x02B3, 10}, {0x01F3, 10},
	 {0x003A, 10}, {0x0000, 10}, {0x0058, 10}, {0x002E, 9},  {0x005E, 10}, {0x0563, 11}, {0x00EC, 12}, {0x0054, 12},
	 {0x0AC1, 12}, {0x1556, 13}, {0x02FA, 13}, {0x0181, 11}, {0x1557, 13}, {0x059D, 14}, {0x2AA3, 14}, {0x2B2A, 14},
	 {0x01DE, 14}, {0x063C, 13}, {0x00CF, 13}, {0x1594, 13}, {0x000D, 9}},
	{{0x0002, 2},  {0x0006, 3},  {0x000F, 4},  {0x000D, 5},  {0x000C, 5},  {0x0015, 6},  {0x0013, 6},  {0x0012, 6},
	 {0x0017, 7},  {0x001F, 8},  {0x001E, 8},  {0x001D, 8},  {0x0025, 9},  {0x0024, 9},  {0x0023, 9},  {0x0021, 9},
	 {0x0021, 10}, {0x0020, 10}, {0x000F, 10}, {0x000E, 10}, {0x0007, 11}, {0x0006, 11}, {0x0020, 11}, {0x0021, 11},
	 {0x0050, 12}, {0x0051, 12}, {0x0052, 12}, {0x000E, 4},  {0x0014, 6},  {0x0016, 7},  {0x001C, 8},  {0x0020, 9},
	 {0x001F, 9},  {0x000D, 10}, {0x0022, 11}, {0x0053, 12}, {0x0055, 12}, {0x000B, 5},  {0x0015, 7},  {0x001E, 9},
	 {0x000C, 10}, {0x0056, 12}, {0x0011, 6},  {0x001B, 8},  {0x001D, 9},  {0x000B, 10}, {0x0010, 6},  {0x0022, 9},
	 {0x000A, 10}, {0x000D, 6},  {0x001C, 9},  {0x0008, 10}, {0x0012, 7},  {0x001B, 9},  {0x0054, 12}, {0x0014, 7},
	 {0x001A, 9},  {0x0057, 12}, {0x0019, 8},  {0x0009, 10}, {0x0018, 8},  {0x0023, 11}, {0x0017, 8},  {0x0019, 9},
	 {0x0018, 9},  {0x0007, 10}, {0x0058, 12}, {0x0007, 4},  {0x000C, 6},  {0x0016, 8},  {0x0017, 9},  {0x0006, 10},
	 {0x0005, 11}, {0x0004, 11}, {0x0059, 12}, {0x000F, 6},  {0x0016, 9},  {0x0005, 10}, {0x000E, 6},  {0x0004, 10},
	 {0x0011, 7},  {0x0024, 11}, {0x0010, 7},  {0x0025, 11}, {0x0013, 7},  {0x005A, 12}, {0x0015, 8},  {0x005B, 12},
	 {0x0014, 8},  {0x0013, 8},  {0x001A, 8},  {0x0015, 9},  {0x0014, 9},  {0x0013, 9},  {0x0012, 9},  {0x0011, 9},
	 {0x0026, 11}, {0x0027, 11}, {0x005C, 12}, {0x005D, 12}, {0x005E, 12}, {0x005F, 12}, {0x0003, 7}},
	{{0x0002, 2},  {0x000F, 4},  {0x0015, 6},  {0x0017, 7},  {0x001F, 8},  {0x0025, 9},  {0x0024, 9},  {0x0021, 10},
	 {0x0020, 10}, {0x0007, 11}, {0x0006, 11}, {0x0020, 11}, {0x0006, 3},  {0x0014, 6},  {0x001E, 8},  {0x000F, 10},
	 {0x0021, 11}, {0x0050, 12}, {0x000E, 4},  {0x001D, 8},  {0x000E, 10}, {0x0051, 12}, {0x000D, 5},  {0x0023, 9},
	 {0x000D, 10}, {0x000C, 5},  {0x0022, 9},  {0x0052, 12}, {0x000B, 5},  {0x000C, 10}, {0x0053, 12}, {0x0013, 6},
	 {0x000B, 10}, {0x0054, 12}, {0x0012, 6},  {0x000A, 10}, {0x0011, 6},  {0x0009, 10}, {0x0010, 6},  {0x0008, 10},
	 {0x0016, 7},  {0x0055, 12}, {0x0015, 7},  {0x0014, 7},  {0x001C, 8},  {0x001B, 8},  {0x0021, 9},  {0x0020, 9},
	 {0x001F, 9},  {0x001E, 9},  {0x001D, 9},  {0x001C, 9},  {0x001B, 9},  {0x001A, 9},  {0x0022, 11}, {0x0023, 11},
	 {0x0056, 12}, {0x0057, 12}, {0x0007, 4},  {0x0019, 9},  {0x0005, 11}, {0x000F, 6},  {0x0004, 11}, {0x000E, 6},
	 {0x000D, 6},  {0x000C, 6},  {0x0013, 7},  {0x0012, 7},  {0x0011, 7},  {0x0010, 7},  {0x001A, 8},  {0x0019, 8},
	 {0x0018, 8},  {0x0017, 8},  {0x0016, 8},  {0x0015, 8},  {0x0014, 8},  {0x0013, 8},  {0x0018, 9},  {0x0017, 9},
	 {0x0016, 9},  {0x0015, 9},  {0x0014, 9},  {0x0013, 9},  {0x0012, 9},  {0x0011, 9},  {0x0007, 10}, {0x0006, 10},
	 {0x0005, 10}, {0x0004, 10}, {0x0024, 11}, {0x0025, 11}, {0x0026, 11}, {0x0027, 11}, {0x0058, 12}, {0x0059, 12},
	 {0x005A, 12}, {0x005B, 12}, {0x005C, 12}, {0x005D, 12}, {0x005E, 12}, {0x005F, 12}, {0x0003, 7}},
	{{0x0000, 2},  {0x0003, 3},  {0x000D, 4},  {0x0005, 4},  {0x001C, 5},  {0x0016, 5},  {0x003F, 6},  {0x003A, 6},
	 {0x002E, 6},  {0x0022, 6},  {0x007B, 7},  {0x0067, 7},  {0x005F, 7},  {0x0047, 7},  {0x0026, 7},  {0x00EF, 8},
	 {0x00CD, 8},  {0x00C1, 8},  {0x00A9, 8},  {0x004F, 8},  {0x01F2, 9},  {0x01DD, 9},  {0x0199, 9},  {0x0185, 9},
	 {0x015D, 9},  {0x011B, 9},  {0x03EF, 10}, {0x03E1, 10}, {0x03C8, 10}, {0x0331, 10}, {0x0303, 10}, {0x02F1, 10},
	 {0x02A0, 10}, {0x0233, 10}, {0x0126, 10}, {0x07C0, 11}, {0x076F, 11}, {0x076C, 11}, {0x0661, 11}, {0x0604, 11},
	 {0x0572, 11}, {0x0551, 11}, {0x046A, 11}, {0x0274, 11}, {0x0F27, 12}, {0x0F24, 12}, {0x0EDB, 12}, {0x0C8E, 12},
	 {0x0C0B, 12}, {0x0C0A, 12}, {0x0AE3, 12}, {0x08D6, 12}, {0x0490, 12}, {0x0495, 12}, {0x1F19, 13}, {0x1DB5, 13},
	 {0x0009, 4},  {0x0010, 5},  {0x0029, 6},  {0x0062, 7},  {0x00F3, 8},  {0x00AD, 8},  {0x01E5, 9},  {0x0179, 9},
	 {0x009C, 9},  {0x03B1, 10}, {0x02AE, 10}, {0x0127, 10}, {0x076E, 11}, {0x0570, 11}, {0x0275, 11}, {0x0F25, 12},
	 {0x0EC0, 12}, {0x0AA0, 12}, {0x08D7, 12}, {0x1E4C, 13}, {0x0008, 5},  {0x0063, 7},  {0x00AF, 8},  {0x017B, 9},
	 {0x03B3, 10}, {0x07DD, 11}, {0x0640, 11}, {0x0F8D, 12}, {0x0BC1, 12}, {0x0491, 12}, {0x0028, 6},  {0x00C3, 8},
	 {0x0151, 9},  {0x02A1, 10}, {0x0573, 11}, {0x0EC3, 12}, {0x1F35, 13}, {0x0065, 7},  {0x01DA, 9},  {0x02AF, 10},
	 {0x0277, 11}, {0x08C9, 12}, {0x1781, 13}, {0x0025, 7},  {0x0118, 9},  {0x0646, 11}, {0x0AA6, 12}, {0x1780, 13},
	 {0x00C9, 8},  {0x0321, 10}, {0x0F9B, 12}, {0x191E, 13}, {0x0048, 8},  {0x07CC, 11}, {0x0AA1, 12}, {0x0180, 9},
	 {0x0465, 11}, {0x1905, 13}, {0x03E2, 10}, {0x0EC1, 12}, {0x3C9B, 14}, {0x02F4, 10}, {0x08C8, 12}, {0x07C1, 11},
	 {0x0928, 13}, {0x05E1, 11}, {0x320D, 14}, {0x0EC2, 12}, {0x6418, 15}, {0x1F34, 13}, {0x0078, 7},  {0x0155, 9},
	 {0x0552, 11}, {0x191F, 13}, {0x00FA, 8},  {0x07DC, 11}, {0x1907, 13}, {0x00AC, 8},  {0x0249, 11}, {0x13B1, 14},
	 {0x01F6, 9},  {0x0AE2, 12}, {0x01DC, 9},  {0x04ED, 12}, {0x0184, 9},  {0x1904, 13}, {0x0156, 9},  {0x09D9, 13},
	 {0x03E7, 10}, {0x0929, 13}, {0x03B2, 10}, {0x3B68, 14}, {0x02F5, 10}, {0x13B0, 14}, {0x0322, 10}, {0x3B69, 14},
	 {0x0234, 10}, {0x7935, 15}, {0x07C7, 11}, {0xC833, 16}, {0x0660, 11}, {0x7934, 15}, {0x024B, 11}, {0xC832, 16},
	 {0x0AA7, 12}, {0x1F18, 13}, {0x007A, 7}},
	{{0x0002, 2},    {0x0000, 3},    {0x001E, 5},    {0x0004, 5},    {0x0012, 6},    {0x0070, 7},   {0x001A, 7},
	 {0x005F, 8},    {0x0047, 8},    {0x01D3, 9},    {0x00B5, 9},    {0x0057, 9},    {0x03B5, 10},  {0x016D, 10},
	 {0x0162, 10},   {0x07CE, 11},   {0x0719, 11},   {0x0691, 11},   {0x02C6, 11},   {0x0156, 11},  {0x0F92, 12},
	 {0x0D2E, 12},   {0x0D20, 12},   {0x059E, 12},   {0x0468, 12},   {0x02A6, 12},   {0x1DA2, 13},  {0x1C60, 13},
	 {0x1A43, 13},   {0x0B1D, 13},   {0x08C0, 13},   {0x055D, 13},   {0x0003, 3},    {0x000A, 5},   {0x0077, 7},
	 {0x00E5, 8},    {0x01D9, 9},    {0x03E5, 10},   {0x0166, 10},   {0x0694, 11},   {0x0152, 11},  {0x059F, 12},
	 {0x1F3C, 13},   {0x1A4B, 13},   {0x055E, 13},   {0x000C, 4},    {0x007D, 7},    {0x0044, 8},   {0x03E0, 10},
	 {0x0769, 11},   {0x0E31, 12},   {0x1F26, 13},   {0x055C, 13},   {0x001B, 5},    {0x00E2, 8},   {0x03A5, 10},
	 {0x02C9, 11},   {0x1F23, 13},   {0x3B47, 14},   {0x0007, 5},    {0x01D8, 9},    {0x02D8, 11},  {0x1F27, 13},
	 {0x3494, 14},   {0x0035, 6},    {0x03E1, 10},   {0x059C, 12},   {0x38C3, 14},   {0x000C, 6},   {0x0165, 10},
	 {0x1D23, 13},   {0x1638, 14},   {0x0068, 7},    {0x0693, 11},   {0x3A45, 14},   {0x0020, 7},   {0x0F90, 12},
	 {0x7CF6, 15},   {0x00E8, 8},    {0x058F, 12},   {0x2CEF, 15},   {0x0045, 8},    {0x0B3A, 13},  {0x01F1, 9},
	 {0x3B46, 14},   {0x01A7, 9},    {0x1676, 14},   {0x0056, 9},    {0x692A, 15},   {0x038D, 10},  {0xE309, 16},
	 {0x00AA, 10},   {0x1C611, 17},  {0x02DF, 11},   {0xB3B9, 17},   {0x02C8, 11},   {0x38C20, 18}, {0x01B0, 11},
	 {0x16390, 18},  {0x0F9F, 12},   {0x16771, 18},  {0x0ED0, 12},   {0x71843, 19},  {0x0D2A, 12},  {0xF9E8C, 20},
	 {0x0461, 12},   {0xF9E8E, 20},  {0x0B67, 13},   {0x055F, 13},   {0x003F, 6},    {0x006D, 9},   {0x0E90, 12},
	 {0x054E, 13},   {0x0013, 6},    {0x0119, 10},   {0x0B66, 13},   {0x000B, 6},    {0x0235, 11},  {0x7CF5, 15},
	 {0x0075, 7},    {0x0D24, 12},   {0xF9E9, 16},   {0x002E, 7},    {0x1F22, 13},   {0x0021, 7},   {0x054F, 13},
	 {0x0014, 7},    {0x3A44, 14},   {0x00E4, 8},    {0x7CF7, 15},   {0x005E, 8},    {0x7185, 15},  {0x0037, 8},
	 {0x2C73, 15},   {0x01DB, 9},    {0x59DD, 16},   {0x01C7, 9},    {0x692B, 15},   {0x01A6, 9},   {0x58E5, 16},
	 {0x00B4, 9},    {0x1F3D0, 17},  {0x00B0, 9},    {0xB1C9, 17},   {0x03E6, 10},   {0x16770, 18}, {0x016E, 10},
	 {0x3E7A2, 18},  {0x011B, 10},   {0xF9E8D, 20},  {0x00D9, 10},   {0xF9E8F, 20},  {0x00A8, 10},  {0x2C723, 19},
	 {0x0749, 11},   {0xE3084, 20},  {0x0696, 11},   {0x58E45, 20},  {0x02DE, 11},   {0xB1C88, 21}, {0x0231, 11},
	 {0x1C610A, 21}, {0x01B1, 11},   {0x71842D, 23}, {0x0D2B, 12},   {0x38C217, 22}, {0x0D2F, 12},  {0x163913, 22},
	 {0x05B2, 12},   {0x163912, 22}, {0x0469, 12},   {0x71842C, 23}, {0x1A42, 13},   {0x08C1, 13},  {0x0073, 7}}};

const int ff_vc1_ac_sizes[AC_MODES] = {186, 169, 133, 149, 103, 103, 163, 175};

} // namespace VC1
static inline uint8 clipByte(int value) {
	return CLIP(value, 0, 255);
}

static inline uint32 roundAvg32(uint32 a, uint32 b) {
	return (a | b) - (((a ^ b) & ~0x01010101U) >> 1);
}

/* Apply overlap transform to horizontal edge */
// Overlap and loop-filter DSP

static void vc1_v_overlap_c(uint8 *src, ptrdiff_t stride) {
	int i;
	int a, b, c, d;
	int d1, d2;
	int rnd = 1;
	for (i = 0; i < 8; i++) {
		a = src[-2 * stride];
		b = src[-stride];
		c = src[0];
		d = src[stride];
		d1 = (a - d + 3 + rnd) >> 3;
		d2 = (a - d + b - c + 4 - rnd) >> 3;

		src[-2 * stride] = a - d1;
		src[-stride] = clipByte(b - d2);
		src[0] = clipByte(c + d2);
		src[stride] = d + d1;
		src++;
		rnd = !rnd;
	}
}

/* Apply overlap transform to vertical edge */
static void vc1_h_overlap_c(uint8 *src, ptrdiff_t stride) {
	int i;
	int a, b, c, d;
	int d1, d2;
	int rnd = 1;
	for (i = 0; i < 8; i++) {
		a = src[-2];
		b = src[-1];
		c = src[0];
		d = src[1];
		d1 = (a - d + 3 + rnd) >> 3;
		d2 = (a - d + b - c + 4 - rnd) >> 3;

		src[-2] = a - d1;
		src[-1] = clipByte(b - d2);
		src[0] = clipByte(c + d2);
		src[1] = d + d1;
		src += stride;
		rnd = !rnd;
	}
}

static void vc1_v_s_overlap_c(int16 *top, int16 *bottom) {
	int i;
	int a, b, c, d;
	int d1, d2;
	int rnd1 = 4, rnd2 = 3;
	for (i = 0; i < 8; i++) {
		a = top[48];
		b = top[56];
		c = bottom[0];
		d = bottom[8];
		d1 = a - d;
		d2 = a - d + b - c;

		top[48] = ((a * 8) - d1 + rnd1) >> 3;
		top[56] = ((b * 8) - d2 + rnd2) >> 3;
		bottom[0] = ((c * 8) + d2 + rnd1) >> 3;
		bottom[8] = ((d * 8) + d1 + rnd2) >> 3;

		bottom++;
		top++;
		rnd2 = 7 - rnd2;
		rnd1 = 7 - rnd1;
	}
}

static void vc1_h_s_overlap_c(int16 *left, int16 *right, ptrdiff_t left_stride, ptrdiff_t right_stride, int flags) {
	int i;
	int a, b, c, d;
	int d1, d2;
	int rnd1 = flags & 2 ? 3 : 4;
	int rnd2 = 7 - rnd1;
	for (i = 0; i < 8; i++) {
		a = left[6];
		b = left[7];
		c = right[0];
		d = right[1];
		d1 = a - d;
		d2 = a - d + b - c;

		left[6] = ((a * 8) - d1 + rnd1) >> 3;
		left[7] = ((b * 8) - d2 + rnd2) >> 3;
		right[0] = ((c * 8) + d2 + rnd1) >> 3;
		right[1] = ((d * 8) + d1 + rnd2) >> 3;

		right += right_stride;
		left += left_stride;
		if (flags & 1) {
			rnd2 = 7 - rnd2;
			rnd1 = 7 - rnd1;
		}
	}
}

/**
 * VC-1 in-loop deblocking filter for one line
 * @param src source block type
 * @param stride block stride
 * @param pq block quantizer
 * @return whether other 3 pairs should be filtered or not
 * @see 8.6
 */
static inline int vc1_filter_line(uint8 *src, ptrdiff_t stride, int pq) {
	int a0 = (2 * (src[-2 * stride] - src[1 * stride]) - 5 * (src[-1 * stride] - src[0 * stride]) + 4) >> 3;
	int a0_sign = a0 >> 31; /* Store sign */

	a0 = (a0 ^ a0_sign) - a0_sign; /* a0 = ABS(a0); */
	if (a0 < pq) {
		int a1 = ABS((2 * (src[-4 * stride] - src[-1 * stride]) - 5 * (src[-3 * stride] - src[-2 * stride]) + 4) >> 3);
		int a2 = ABS((2 * (src[0 * stride] - src[3 * stride]) - 5 * (src[1 * stride] - src[2 * stride]) + 4) >> 3);
		if (a1 < a0 || a2 < a0) {
			int clip = src[-1 * stride] - src[0 * stride];
			int clip_sign = clip >> 31;

			clip = ((clip ^ clip_sign) - clip_sign) >> 1;
			if (clip) {
				int a3 = MIN(a1, a2);
				int d = 5 * (a3 - a0);
				int d_sign = (d >> 31);

				d = ((d ^ d_sign) - d_sign) >> 3;
				d_sign ^= a0_sign;

				if (d_sign ^ clip_sign)
					d = 0;
				else {
					d = MIN(d, clip);
					d = (d ^ d_sign) - d_sign; /* Restore sign */
					src[-1 * stride] = clipByte(src[-1 * stride] - d);
					src[0 * stride] = clipByte(src[0 * stride] + d);
				}
				return 1;
			}
		}
	}
	return 0;
}

/**
 * VC-1 in-loop deblocking filter
 * @param src source block type
 * @param step distance between horizontally adjacent elements
 * @param stride distance between vertically adjacent elements
 * @param len edge length to filter (4 or 8 pixels)
 * @param pq block quantizer
 * @see 8.6
 */
static inline void vc1_loop_filter(uint8 *src, ptrdiff_t step, ptrdiff_t stride, int len, int pq) {
	int i;
	int filt3;

	for (i = 0; i < len; i += 4) {
		filt3 = vc1_filter_line(src + 2 * step, stride, pq);
		if (filt3) {
			vc1_filter_line(src + 0 * step, stride, pq);
			vc1_filter_line(src + 1 * step, stride, pq);
			vc1_filter_line(src + 3 * step, stride, pq);
		}
		src += step * 4;
	}
}

static void vc1_v_loop_filter4_c(uint8 *src, ptrdiff_t stride, int pq) {
	vc1_loop_filter(src, 1, stride, 4, pq);
}

static void vc1_h_loop_filter4_c(uint8 *src, ptrdiff_t stride, int pq) {
	vc1_loop_filter(src, stride, 1, 4, pq);
}

static void vc1_v_loop_filter8_c(uint8 *src, ptrdiff_t stride, int pq) {
	vc1_loop_filter(src, 1, stride, 8, pq);
}

static void vc1_h_loop_filter8_c(uint8 *src, ptrdiff_t stride, int pq) {
	vc1_loop_filter(src, stride, 1, 8, pq);
}

static void vc1_v_loop_filter16_c(uint8 *src, ptrdiff_t stride, int pq) {
	vc1_loop_filter(src, 1, stride, 16, pq);
}

static void vc1_h_loop_filter16_c(uint8 *src, ptrdiff_t stride, int pq) {
	vc1_loop_filter(src, stride, 1, 16, pq);
}

/* Do inverse transform on 8x8 block */
// Inverse-transform DSP

static void vc1_inv_trans_8x8_dc_c(uint8 *dest, ptrdiff_t stride, int16 *block) {
	int i;
	int dc = block[0];

	dc = (3 * dc + 1) >> 1;
	dc = (3 * dc + 16) >> 5;

	for (i = 0; i < 8; i++) {
		dest[0] = clipByte(dest[0] + dc);
		dest[1] = clipByte(dest[1] + dc);
		dest[2] = clipByte(dest[2] + dc);
		dest[3] = clipByte(dest[3] + dc);
		dest[4] = clipByte(dest[4] + dc);
		dest[5] = clipByte(dest[5] + dc);
		dest[6] = clipByte(dest[6] + dc);
		dest[7] = clipByte(dest[7] + dc);
		dest += stride;
	}
}

static void vc1_inv_trans_8x8_c(int16 block[64]) {
	int i;
	int t1, t2, t3, t4, t5, t6, t7, t8;
	int16 *src, *dst, temp[64];

	src = block;
	dst = temp;
	for (i = 0; i < 8; i++) {
		t1 = 12 * (src[0] + src[32]) + 4;
		t2 = 12 * (src[0] - src[32]) + 4;
		t3 = 16 * src[16] + 6 * src[48];
		t4 = 6 * src[16] - 16 * src[48];

		t5 = t1 + t3;
		t6 = t2 + t4;
		t7 = t2 - t4;
		t8 = t1 - t3;

		t1 = 16 * src[8] + 15 * src[24] + 9 * src[40] + 4 * src[56];
		t2 = 15 * src[8] - 4 * src[24] - 16 * src[40] - 9 * src[56];
		t3 = 9 * src[8] - 16 * src[24] + 4 * src[40] + 15 * src[56];
		t4 = 4 * src[8] - 9 * src[24] + 15 * src[40] - 16 * src[56];

		dst[0] = (t5 + t1) >> 3;
		dst[1] = (t6 + t2) >> 3;
		dst[2] = (t7 + t3) >> 3;
		dst[3] = (t8 + t4) >> 3;
		dst[4] = (t8 - t4) >> 3;
		dst[5] = (t7 - t3) >> 3;
		dst[6] = (t6 - t2) >> 3;
		dst[7] = (t5 - t1) >> 3;

		src += 1;
		dst += 8;
	}

	src = temp;
	dst = block;
	for (i = 0; i < 8; i++) {
		t1 = 12 * (src[0] + src[32]) + 64;
		t2 = 12 * (src[0] - src[32]) + 64;
		t3 = 16 * src[16] + 6 * src[48];
		t4 = 6 * src[16] - 16 * src[48];

		t5 = t1 + t3;
		t6 = t2 + t4;
		t7 = t2 - t4;
		t8 = t1 - t3;

		t1 = 16 * src[8] + 15 * src[24] + 9 * src[40] + 4 * src[56];
		t2 = 15 * src[8] - 4 * src[24] - 16 * src[40] - 9 * src[56];
		t3 = 9 * src[8] - 16 * src[24] + 4 * src[40] + 15 * src[56];
		t4 = 4 * src[8] - 9 * src[24] + 15 * src[40] - 16 * src[56];

		dst[0] = (t5 + t1) >> 7;
		dst[8] = (t6 + t2) >> 7;
		dst[16] = (t7 + t3) >> 7;
		dst[24] = (t8 + t4) >> 7;
		dst[32] = (t8 - t4 + 1) >> 7;
		dst[40] = (t7 - t3 + 1) >> 7;
		dst[48] = (t6 - t2 + 1) >> 7;
		dst[56] = (t5 - t1 + 1) >> 7;

		src++;
		dst++;
	}
}

/* Do inverse transform on 8x4 part of block */
static void vc1_inv_trans_8x4_dc_c(uint8 *dest, ptrdiff_t stride, int16 *block) {
	int i;
	int dc = block[0];

	dc = (3 * dc + 1) >> 1;
	dc = (17 * dc + 64) >> 7;

	for (i = 0; i < 4; i++) {
		dest[0] = clipByte(dest[0] + dc);
		dest[1] = clipByte(dest[1] + dc);
		dest[2] = clipByte(dest[2] + dc);
		dest[3] = clipByte(dest[3] + dc);
		dest[4] = clipByte(dest[4] + dc);
		dest[5] = clipByte(dest[5] + dc);
		dest[6] = clipByte(dest[6] + dc);
		dest[7] = clipByte(dest[7] + dc);
		dest += stride;
	}
}

static void vc1_inv_trans_8x4_c(uint8 *dest, ptrdiff_t stride, int16 *block) {
	int i;
	int t1, t2, t3, t4, t5, t6, t7, t8;
	int16 *src, *dst;

	src = block;
	dst = block;

	for (i = 0; i < 4; i++) {
		t1 = 12 * (src[0] + src[4]) + 4;
		t2 = 12 * (src[0] - src[4]) + 4;
		t3 = 16 * src[2] + 6 * src[6];
		t4 = 6 * src[2] - 16 * src[6];

		t5 = t1 + t3;
		t6 = t2 + t4;
		t7 = t2 - t4;
		t8 = t1 - t3;

		t1 = 16 * src[1] + 15 * src[3] + 9 * src[5] + 4 * src[7];
		t2 = 15 * src[1] - 4 * src[3] - 16 * src[5] - 9 * src[7];
		t3 = 9 * src[1] - 16 * src[3] + 4 * src[5] + 15 * src[7];
		t4 = 4 * src[1] - 9 * src[3] + 15 * src[5] - 16 * src[7];

		dst[0] = (t5 + t1) >> 3;
		dst[1] = (t6 + t2) >> 3;
		dst[2] = (t7 + t3) >> 3;
		dst[3] = (t8 + t4) >> 3;
		dst[4] = (t8 - t4) >> 3;
		dst[5] = (t7 - t3) >> 3;
		dst[6] = (t6 - t2) >> 3;
		dst[7] = (t5 - t1) >> 3;

		src += 8;
		dst += 8;
	}

	src = block;
	for (i = 0; i < 8; i++) {
		t1 = 17 * (src[0] + src[16]) + 64;
		t2 = 17 * (src[0] - src[16]) + 64;
		t3 = 22 * src[8] + 10 * src[24];
		t4 = 22 * src[24] - 10 * src[8];

		dest[0 * stride] = clipByte(dest[0 * stride] + ((t1 + t3) >> 7));
		dest[1 * stride] = clipByte(dest[1 * stride] + ((t2 - t4) >> 7));
		dest[2 * stride] = clipByte(dest[2 * stride] + ((t2 + t4) >> 7));
		dest[3 * stride] = clipByte(dest[3 * stride] + ((t1 - t3) >> 7));

		src++;
		dest++;
	}
}

/* Do inverse transform on 4x8 parts of block */
static void vc1_inv_trans_4x8_dc_c(uint8 *dest, ptrdiff_t stride, int16 *block) {
	int i;
	int dc = block[0];

	dc = (17 * dc + 4) >> 3;
	dc = (12 * dc + 64) >> 7;

	for (i = 0; i < 8; i++) {
		dest[0] = clipByte(dest[0] + dc);
		dest[1] = clipByte(dest[1] + dc);
		dest[2] = clipByte(dest[2] + dc);
		dest[3] = clipByte(dest[3] + dc);
		dest += stride;
	}
}

static void vc1_inv_trans_4x8_c(uint8 *dest, ptrdiff_t stride, int16 *block) {
	int i;
	int t1, t2, t3, t4, t5, t6, t7, t8;
	int16 *src, *dst;

	src = block;
	dst = block;

	for (i = 0; i < 8; i++) {
		t1 = 17 * (src[0] + src[2]) + 4;
		t2 = 17 * (src[0] - src[2]) + 4;
		t3 = 22 * src[1] + 10 * src[3];
		t4 = 22 * src[3] - 10 * src[1];

		dst[0] = (t1 + t3) >> 3;
		dst[1] = (t2 - t4) >> 3;
		dst[2] = (t2 + t4) >> 3;
		dst[3] = (t1 - t3) >> 3;

		src += 8;
		dst += 8;
	}

	src = block;
	for (i = 0; i < 4; i++) {
		t1 = 12 * (src[0] + src[32]) + 64;
		t2 = 12 * (src[0] - src[32]) + 64;
		t3 = 16 * src[16] + 6 * src[48];
		t4 = 6 * src[16] - 16 * src[48];

		t5 = t1 + t3;
		t6 = t2 + t4;
		t7 = t2 - t4;
		t8 = t1 - t3;

		t1 = 16 * src[8] + 15 * src[24] + 9 * src[40] + 4 * src[56];
		t2 = 15 * src[8] - 4 * src[24] - 16 * src[40] - 9 * src[56];
		t3 = 9 * src[8] - 16 * src[24] + 4 * src[40] + 15 * src[56];
		t4 = 4 * src[8] - 9 * src[24] + 15 * src[40] - 16 * src[56];

		dest[0 * stride] = clipByte(dest[0 * stride] + ((t5 + t1) >> 7));
		dest[1 * stride] = clipByte(dest[1 * stride] + ((t6 + t2) >> 7));
		dest[2 * stride] = clipByte(dest[2 * stride] + ((t7 + t3) >> 7));
		dest[3 * stride] = clipByte(dest[3 * stride] + ((t8 + t4) >> 7));
		dest[4 * stride] = clipByte(dest[4 * stride] + ((t8 - t4 + 1) >> 7));
		dest[5 * stride] = clipByte(dest[5 * stride] + ((t7 - t3 + 1) >> 7));
		dest[6 * stride] = clipByte(dest[6 * stride] + ((t6 - t2 + 1) >> 7));
		dest[7 * stride] = clipByte(dest[7 * stride] + ((t5 - t1 + 1) >> 7));

		src++;
		dest++;
	}
}

/* Do inverse transform on 4x4 part of block */
static void vc1_inv_trans_4x4_dc_c(uint8 *dest, ptrdiff_t stride, int16 *block) {
	int i;
	int dc = block[0];

	dc = (17 * dc + 4) >> 3;
	dc = (17 * dc + 64) >> 7;

	for (i = 0; i < 4; i++) {
		dest[0] = clipByte(dest[0] + dc);
		dest[1] = clipByte(dest[1] + dc);
		dest[2] = clipByte(dest[2] + dc);
		dest[3] = clipByte(dest[3] + dc);
		dest += stride;
	}
}

static void vc1_inv_trans_4x4_c(uint8 *dest, ptrdiff_t stride, int16 *block) {
	int i;
	int t1, t2, t3, t4;
	int16 *src, *dst;

	src = block;
	dst = block;
	for (i = 0; i < 4; i++) {
		t1 = 17 * (src[0] + src[2]) + 4;
		t2 = 17 * (src[0] - src[2]) + 4;
		t3 = 22 * src[1] + 10 * src[3];
		t4 = 22 * src[3] - 10 * src[1];

		dst[0] = (t1 + t3) >> 3;
		dst[1] = (t2 - t4) >> 3;
		dst[2] = (t2 + t4) >> 3;
		dst[3] = (t1 - t3) >> 3;

		src += 8;
		dst += 8;
	}

	src = block;
	for (i = 0; i < 4; i++) {
		t1 = 17 * (src[0] + src[16]) + 64;
		t2 = 17 * (src[0] - src[16]) + 64;
		t3 = 22 * src[8] + 10 * src[24];
		t4 = 22 * src[24] - 10 * src[8];

		dest[0 * stride] = clipByte(dest[0 * stride] + ((t1 + t3) >> 7));
		dest[1 * stride] = clipByte(dest[1 * stride] + ((t2 - t4) >> 7));
		dest[2 * stride] = clipByte(dest[2 * stride] + ((t2 + t4) >> 7));
		dest[3 * stride] = clipByte(dest[3 * stride] + ((t1 - t3) >> 7));

		src++;
		dest++;
	}
}

/* motion compensation functions */

/* Filter in case of 2 filters */
#define VC1_MSPEL_FILTER_16B(DIR, TYPE)                                                                                \
	static inline int vc1_mspel_##DIR##_filter_16bits(const TYPE *src, ptrdiff_t stride, int mode) {                   \
		switch (mode) {                                                                                                \
		case 0: /* no shift - should not occur */                                                                      \
			return 0;                                                                                                  \
		case 1: /* 1/4 shift */                                                                                        \
			return -4 * src[-stride] + 53 * src[0] + 18 * src[stride] - 3 * src[stride * 2];                           \
		case 2: /* 1/2 shift */                                                                                        \
			return -1 * src[-stride] + 9 * src[0] + 9 * src[stride] - 1 * src[stride * 2];                             \
		case 3: /* 3/4 shift */                                                                                        \
			return -3 * src[-stride] + 18 * src[0] + 53 * src[stride] - 4 * src[stride * 2];                           \
		}                                                                                                              \
		return 0; /* should not occur */                                                                               \
	}

VC1_MSPEL_FILTER_16B(ver, uint8)
VC1_MSPEL_FILTER_16B(hor, int16)

/* Filter used to interpolate fractional pel values */
static inline int vc1_mspel_filter(const uint8 *src, ptrdiff_t stride, int mode, int r) {
	switch (mode) {
	case 0: // no shift
		return src[0];
	case 1: // 1/4 shift
		return (-4 * src[-stride] + 53 * src[0] + 18 * src[stride] - 3 * src[stride * 2] + 32 - r) >> 6;
	case 2: // 1/2 shift
		return (-1 * src[-stride] + 9 * src[0] + 9 * src[stride] - 1 * src[stride * 2] + 8 - r) >> 4;
	case 3: // 3/4 shift
		return (-3 * src[-stride] + 18 * src[0] + 53 * src[stride] - 4 * src[stride * 2] + 32 - r) >> 6;
	}
	return 0; // should not occur
}

/* Function used to do motion compensation with bicubic interpolation */
#define VC1_MSPEL_MC(OP, OP4, OPNAME)                                                                                  \
	static inline void OPNAME##vc1_mspel_mc(uint8 *dst, const uint8 *src, ptrdiff_t stride, int hmode, int vmode,      \
											int rnd) {                                                                 \
		int i, j;                                                                                                      \
                                                                                                                       \
		if (vmode) { /* Horizontal filter to apply */                                                                  \
			int r;                                                                                                     \
                                                                                                                       \
			if (hmode) { /* Vertical filter to apply, output to tmp */                                                 \
				static const int shift_value[] = {0, 5, 1, 5};                                                         \
				int shift = (shift_value[hmode] + shift_value[vmode]) >> 1;                                            \
				int16 tmp[11 * 8], *tptr = tmp;                                                                        \
                                                                                                                       \
				r = (1 << (shift - 1)) + rnd - 1;                                                                      \
                                                                                                                       \
				src -= 1;                                                                                              \
				for (j = 0; j < 8; j++) {                                                                              \
					for (i = 0; i < 11; i++)                                                                           \
						tptr[i] = (vc1_mspel_ver_filter_16bits(src + i, stride, vmode) + r) >> shift;                  \
					src += stride;                                                                                     \
					tptr += 11;                                                                                        \
				}                                                                                                      \
                                                                                                                       \
				r = 64 - rnd;                                                                                          \
				tptr = tmp + 1;                                                                                        \
				for (j = 0; j < 8; j++) {                                                                              \
					for (i = 0; i < 8; i++)                                                                            \
						OP(dst[i], (vc1_mspel_hor_filter_16bits(tptr + i, 1, hmode) + r) >> 7);                        \
					dst += stride;                                                                                     \
					tptr += 11;                                                                                        \
				}                                                                                                      \
                                                                                                                       \
				return;                                                                                                \
			} else { /* No horizontal filter, output 8 lines to dst */                                                 \
				r = 1 - rnd;                                                                                           \
                                                                                                                       \
				for (j = 0; j < 8; j++) {                                                                              \
					for (i = 0; i < 8; i++)                                                                            \
						OP(dst[i], vc1_mspel_filter(src + i, stride, vmode, r));                                       \
					src += stride;                                                                                     \
					dst += stride;                                                                                     \
				}                                                                                                      \
				return;                                                                                                \
			}                                                                                                          \
		}                                                                                                              \
                                                                                                                       \
		/* Horizontal mode with no vertical mode */                                                                    \
		for (j = 0; j < 8; j++) {                                                                                      \
			for (i = 0; i < 8; i++)                                                                                    \
				OP(dst[i], vc1_mspel_filter(src + i, 1, hmode, rnd));                                                  \
			dst += stride;                                                                                             \
			src += stride;                                                                                             \
		}                                                                                                              \
	}                                                                                                                  \
	static inline void OPNAME##vc1_mspel_mc_16(uint8 *dst, const uint8 *src, ptrdiff_t stride, int hmode, int vmode,   \
											   int rnd) {                                                              \
		int i, j;                                                                                                      \
                                                                                                                       \
		if (vmode) { /* Horizontal filter to apply */                                                                  \
			int r;                                                                                                     \
                                                                                                                       \
			if (hmode) { /* Vertical filter to apply, output to tmp */                                                 \
				static const int shift_value[] = {0, 5, 1, 5};                                                         \
				int shift = (shift_value[hmode] + shift_value[vmode]) >> 1;                                            \
				int16 tmp[19 * 16], *tptr = tmp;                                                                       \
                                                                                                                       \
				r = (1 << (shift - 1)) + rnd - 1;                                                                      \
                                                                                                                       \
				src -= 1;                                                                                              \
				for (j = 0; j < 16; j++) {                                                                             \
					for (i = 0; i < 19; i++)                                                                           \
						tptr[i] = (vc1_mspel_ver_filter_16bits(src + i, stride, vmode) + r) >> shift;                  \
					src += stride;                                                                                     \
					tptr += 19;                                                                                        \
				}                                                                                                      \
                                                                                                                       \
				r = 64 - rnd;                                                                                          \
				tptr = tmp + 1;                                                                                        \
				for (j = 0; j < 16; j++) {                                                                             \
					for (i = 0; i < 16; i++)                                                                           \
						OP(dst[i], (vc1_mspel_hor_filter_16bits(tptr + i, 1, hmode) + r) >> 7);                        \
					dst += stride;                                                                                     \
					tptr += 19;                                                                                        \
				}                                                                                                      \
                                                                                                                       \
				return;                                                                                                \
			} else { /* No horizontal filter, output 8 lines to dst */                                                 \
				r = 1 - rnd;                                                                                           \
                                                                                                                       \
				for (j = 0; j < 16; j++) {                                                                             \
					for (i = 0; i < 16; i++)                                                                           \
						OP(dst[i], vc1_mspel_filter(src + i, stride, vmode, r));                                       \
					src += stride;                                                                                     \
					dst += stride;                                                                                     \
				}                                                                                                      \
				return;                                                                                                \
			}                                                                                                          \
		}                                                                                                              \
                                                                                                                       \
		/* Horizontal mode with no vertical mode */                                                                    \
		for (j = 0; j < 16; j++) {                                                                                     \
			for (i = 0; i < 16; i++)                                                                                   \
				OP(dst[i], vc1_mspel_filter(src + i, 1, hmode, rnd));                                                  \
			dst += stride;                                                                                             \
			src += stride;                                                                                             \
		}                                                                                                              \
	}                                                                                                                  \
	static void OPNAME##pixels8x8_c(uint8 *block, const uint8 *pixels, ptrdiff_t line_size, int rnd) {                 \
		int i;                                                                                                         \
		for (i = 0; i < 8; i++) {                                                                                      \
			OP4(block, READ_UINT32(pixels));                                                                           \
			OP4(block + 4, READ_UINT32(pixels + 4));                                                                   \
			pixels += line_size;                                                                                       \
			block += line_size;                                                                                        \
		}                                                                                                              \
	}                                                                                                                  \
	static void OPNAME##pixels16x16_c(uint8 *block, const uint8 *pixels, ptrdiff_t line_size, int rnd) {               \
		int i;                                                                                                         \
		for (i = 0; i < 16; i++) {                                                                                     \
			OP4(block, READ_UINT32(pixels));                                                                           \
			OP4(block + 4, READ_UINT32(pixels + 4));                                                                   \
			OP4(block + 8, READ_UINT32(pixels + 8));                                                                   \
			OP4(block + 12, READ_UINT32(pixels + 12));                                                                 \
			pixels += line_size;                                                                                       \
			block += line_size;                                                                                        \
		}                                                                                                              \
	}

#define op_put(a, b) (a) = clipByte(b)
#define op_avg(a, b) (a) = ((a) + clipByte(b) + 1) >> 1
#define op4_avg(a, b) WRITE_UINT32(a, roundAvg32(READ_UINT32(a), b))
#define op4_put(a, b) WRITE_UINT32(a, b)

VC1_MSPEL_MC(op_put, op4_put, put_)
VC1_MSPEL_MC(op_avg, op4_avg, avg_)

/* pixel functions - really are entry points to vc1_mspel_mc */

#define PUT_VC1_MSPEL(a, b)                                                                                            \
	static void put_vc1_mspel_mc##a##b##_c(uint8 *dst, const uint8 *src, ptrdiff_t stride, int rnd) {                  \
		put_vc1_mspel_mc(dst, src, stride, a, b, rnd);                                                                 \
	}                                                                                                                  \
	static void avg_vc1_mspel_mc##a##b##_c(uint8 *dst, const uint8 *src, ptrdiff_t stride, int rnd) {                  \
		avg_vc1_mspel_mc(dst, src, stride, a, b, rnd);                                                                 \
	}                                                                                                                  \
	static void put_vc1_mspel_mc##a##b##_16_c(uint8 *dst, const uint8 *src, ptrdiff_t stride, int rnd) {               \
		put_vc1_mspel_mc_16(dst, src, stride, a, b, rnd);                                                              \
	}                                                                                                                  \
	static void avg_vc1_mspel_mc##a##b##_16_c(uint8 *dst, const uint8 *src, ptrdiff_t stride, int rnd) {               \
		avg_vc1_mspel_mc_16(dst, src, stride, a, b, rnd);                                                              \
	}

PUT_VC1_MSPEL(1, 0)
PUT_VC1_MSPEL(2, 0)
PUT_VC1_MSPEL(3, 0)

PUT_VC1_MSPEL(0, 1)
PUT_VC1_MSPEL(1, 1)
PUT_VC1_MSPEL(2, 1)
PUT_VC1_MSPEL(3, 1)

PUT_VC1_MSPEL(0, 2)
PUT_VC1_MSPEL(1, 2)
PUT_VC1_MSPEL(2, 2)
PUT_VC1_MSPEL(3, 2)

PUT_VC1_MSPEL(0, 3)
PUT_VC1_MSPEL(1, 3)
PUT_VC1_MSPEL(2, 3)
PUT_VC1_MSPEL(3, 3)

#define chroma_mc(a) ((A * src[a] + B * src[a + 1] + C * src[stride + a] + D * src[stride + a + 1] + 32 - 4) >> 6)
static void put_no_rnd_vc1_chroma_mc8_c(uint8 *dst /* align 8 */, const uint8 *src /* align 1 */, ptrdiff_t stride,
										int h, int x, int y) {
	const int A = (8 - x) * (8 - y);
	const int B = (x) * (8 - y);
	const int C = (8 - x) * (y);
	const int D = (x) * (y);
	int i;

	assert(x < 8 && y < 8 && x >= 0 && y >= 0);

	for (i = 0; i < h; i++) {
		dst[0] = chroma_mc(0);
		dst[1] = chroma_mc(1);
		dst[2] = chroma_mc(2);
		dst[3] = chroma_mc(3);
		dst[4] = chroma_mc(4);
		dst[5] = chroma_mc(5);
		dst[6] = chroma_mc(6);
		dst[7] = chroma_mc(7);
		dst += stride;
		src += stride;
	}
}

static void put_no_rnd_vc1_chroma_mc4_c(uint8 *dst, const uint8 *src, ptrdiff_t stride, int h, int x, int y) {
	const int A = (8 - x) * (8 - y);
	const int B = (x) * (8 - y);
	const int C = (8 - x) * (y);
	const int D = (x) * (y);
	int i;

	assert(x < 8 && y < 8 && x >= 0 && y >= 0);

	for (i = 0; i < h; i++) {
		dst[0] = chroma_mc(0);
		dst[1] = chroma_mc(1);
		dst[2] = chroma_mc(2);
		dst[3] = chroma_mc(3);
		dst += stride;
		src += stride;
	}
}

#define avg2(a, b) (((a) + (b) + 1) >> 1)
static void avg_no_rnd_vc1_chroma_mc8_c(uint8 *dst /* align 8 */, const uint8 *src /* align 1 */, ptrdiff_t stride,
										int h, int x, int y) {
	const int A = (8 - x) * (8 - y);
	const int B = (x) * (8 - y);
	const int C = (8 - x) * (y);
	const int D = (x) * (y);
	int i;

	assert(x < 8 && y < 8 && x >= 0 && y >= 0);

	for (i = 0; i < h; i++) {
		dst[0] = avg2(dst[0], chroma_mc(0));
		dst[1] = avg2(dst[1], chroma_mc(1));
		dst[2] = avg2(dst[2], chroma_mc(2));
		dst[3] = avg2(dst[3], chroma_mc(3));
		dst[4] = avg2(dst[4], chroma_mc(4));
		dst[5] = avg2(dst[5], chroma_mc(5));
		dst[6] = avg2(dst[6], chroma_mc(6));
		dst[7] = avg2(dst[7], chroma_mc(7));
		dst += stride;
		src += stride;
	}
}

static void avg_no_rnd_vc1_chroma_mc4_c(uint8 *dst /* align 8 */, const uint8 *src /* align 1 */, ptrdiff_t stride,
										int h, int x, int y) {
	const int A = (8 - x) * (8 - y);
	const int B = (x) * (8 - y);
	const int C = (8 - x) * (y);
	const int D = (x) * (y);
	int i;

	assert(x < 8 && y < 8 && x >= 0 && y >= 0);

	for (i = 0; i < h; i++) {
		dst[0] = avg2(dst[0], chroma_mc(0));
		dst[1] = avg2(dst[1], chroma_mc(1));
		dst[2] = avg2(dst[2], chroma_mc(2));
		dst[3] = avg2(dst[3], chroma_mc(3));
		dst += stride;
		src += stride;
	}
}

#define FN_ASSIGN(X, Y)                                                                                                \
	dsp->put_vc1_mspel_pixels_tab[1][X + 4 * Y] = put_vc1_mspel_mc##X##Y##_c;                                          \
	dsp->put_vc1_mspel_pixels_tab[0][X + 4 * Y] = put_vc1_mspel_mc##X##Y##_16_c;                                       \
	dsp->avg_vc1_mspel_pixels_tab[1][X + 4 * Y] = avg_vc1_mspel_mc##X##Y##_c;                                          \
	dsp->avg_vc1_mspel_pixels_tab[0][X + 4 * Y] = avg_vc1_mspel_mc##X##Y##_16_c

void initWMV3DSP(WMV3DSP *dsp) {
	*dsp = WMV3DSP();
	dsp->vc1_inv_trans_8x8 = vc1_inv_trans_8x8_c;
	dsp->vc1_inv_trans_4x8 = vc1_inv_trans_4x8_c;
	dsp->vc1_inv_trans_8x4 = vc1_inv_trans_8x4_c;
	dsp->vc1_inv_trans_4x4 = vc1_inv_trans_4x4_c;
	dsp->vc1_inv_trans_8x8_dc = vc1_inv_trans_8x8_dc_c;
	dsp->vc1_inv_trans_4x8_dc = vc1_inv_trans_4x8_dc_c;
	dsp->vc1_inv_trans_8x4_dc = vc1_inv_trans_8x4_dc_c;
	dsp->vc1_inv_trans_4x4_dc = vc1_inv_trans_4x4_dc_c;

	dsp->vc1_h_overlap = vc1_h_overlap_c;
	dsp->vc1_v_overlap = vc1_v_overlap_c;
	dsp->vc1_h_s_overlap = vc1_h_s_overlap_c;
	dsp->vc1_v_s_overlap = vc1_v_s_overlap_c;

	dsp->vc1_v_loop_filter4 = vc1_v_loop_filter4_c;
	dsp->vc1_h_loop_filter4 = vc1_h_loop_filter4_c;
	dsp->vc1_v_loop_filter8 = vc1_v_loop_filter8_c;
	dsp->vc1_h_loop_filter8 = vc1_h_loop_filter8_c;
	dsp->vc1_v_loop_filter16 = vc1_v_loop_filter16_c;
	dsp->vc1_h_loop_filter16 = vc1_h_loop_filter16_c;

	dsp->put_vc1_mspel_pixels_tab[0][0] = put_pixels16x16_c;
	dsp->avg_vc1_mspel_pixels_tab[0][0] = avg_pixels16x16_c;
	dsp->put_vc1_mspel_pixels_tab[1][0] = put_pixels8x8_c;
	dsp->avg_vc1_mspel_pixels_tab[1][0] = avg_pixels8x8_c;
	FN_ASSIGN(0, 1);
	FN_ASSIGN(0, 2);
	FN_ASSIGN(0, 3);

	FN_ASSIGN(1, 0);
	FN_ASSIGN(1, 1);
	FN_ASSIGN(1, 2);
	FN_ASSIGN(1, 3);

	FN_ASSIGN(2, 0);
	FN_ASSIGN(2, 1);
	FN_ASSIGN(2, 2);
	FN_ASSIGN(2, 3);

	FN_ASSIGN(3, 0);
	FN_ASSIGN(3, 1);
	FN_ASSIGN(3, 2);
	FN_ASSIGN(3, 3);

	dsp->put_no_rnd_vc1_chroma_pixels_tab[0] = put_no_rnd_vc1_chroma_mc8_c;
	dsp->avg_no_rnd_vc1_chroma_pixels_tab[0] = avg_no_rnd_vc1_chroma_mc8_c;
	dsp->put_no_rnd_vc1_chroma_pixels_tab[1] = put_no_rnd_vc1_chroma_mc4_c;
	dsp->avg_no_rnd_vc1_chroma_pixels_tab[1] = avg_no_rnd_vc1_chroma_mc4_c;
}

#undef FN_ASSIGN
#undef PUT_VC1_MSPEL
#undef VC1_MSPEL_FILTER_16B
#undef VC1_MSPEL_MC
#undef avg2
#undef chroma_mc
#undef op4_avg
#undef op4_put
#undef op_avg
#undef op_put

enum { PROFILE_SIMPLE, PROFILE_MAIN, PROFILE_COMPLEX, PROFILE_ADVANCED };

static int setDimensions(VC1Header *v, int width, int height) {
	if (width <= 0 || height <= 0) {
		v->width = v->height = 0;
		return -1;
	}
	v->width = width;
	v->height = height;
	return 0;
}

static void validateAspect(VC1Header *v) {
	const VC1Ratio &sar = v->sample_aspect_ratio;
	bool valid = sar.den > 0 && sar.num >= 0;
	if (valid && sar.num && sar.num != sar.den) {
		int64 scaled = sar.num < sar.den ? (int64)v->width * sar.num / sar.den : (int64)v->height * sar.den / sar.num;
		valid = scaled > 0;
	}
	if (!valid)
		v->sample_aspect_ratio = VC1Ratio();
}

static void reduceAspect(VC1Ratio &aspect, int num, int den) {
	if (den <= 0) {
		aspect = VC1Ratio();
		return;
	}
	Common::Rational ratio(num, den);
	aspect.num = ratio.getNumerator();
	aspect.den = ratio.getDenominator();
}

static const uint8 ff_wmv2_scantableA[64] = {
	0x00, 0x01, 0x02, 0x08, 0x03, 0x09, 0x0A, 0x10, 0x04, 0x0B, 0x11, 0x18, 0x12, 0x0C, 0x05, 0x13,
	0x19, 0x0D, 0x14, 0x1A, 0x1B, 0x06, 0x15, 0x1C, 0x0E, 0x16, 0x1D, 0x07, 0x1E, 0x0F, 0x17, 0x1F,
};

static const uint8 ff_wmv2_scantableB[64] = {
	0x00, 0x08, 0x01, 0x10, 0x09, 0x18, 0x11, 0x02, 0x20, 0x0A, 0x19, 0x28, 0x12, 0x30, 0x21, 0x1A,
	0x38, 0x29, 0x22, 0x03, 0x31, 0x39, 0x0B, 0x2A, 0x13, 0x32, 0x1B, 0x3A, 0x23, 0x2B, 0x33, 0x3B,
};

static const uint8 ff_vc1_adv_progressive_8x4_zz[32] = {/* Table 233 */
														0,  8,  1,  16, 2,  9, 10, 3,  24, 17, 4,  11, 18, 12, 5,  19,
														25, 13, 20, 26, 27, 6, 21, 28, 14, 22, 29, 7,  30, 15, 23, 31};

static const uint8 ff_vc1_adv_progressive_4x8_zz[32] = {/* Table 234 */
														0,  1,  8,  2,  9,  16, 17, 24, 10, 32, 25, 18, 40, 3,  33, 26,
														48, 11, 56, 41, 34, 49, 57, 42, 19, 50, 27, 58, 35, 43, 51, 59};

static const VC1Ratio ff_vc1_pixel_aspect[16] = {{0, 1},   {1, 1},    {12, 11}, {10, 11}, {16, 11}, {40, 33},
												 {24, 11}, {20, 11},  {32, 11}, {80, 33}, {18, 11}, {15, 11},
												 {64, 33}, {160, 99}, {0, 1},   {0, 1}};

static const int ff_vc1_fps_nr[7] = {24, 25, 30, 50, 60, 48, 72};
static const int ff_vc1_fps_dr[2] = {1000, 1001};

static int readVC1Advanced(VC1Header *v, Common::BitStream8MSB *gb);

/**
 * Decode Simple/Main Profiles sequence header
 * @see Figure 7-8, p16-17
 * @param v Parsed codec state
 * @param gb Bit stream containing the codec sequence header
 * @return Status
 */
// Sequence and entry-point headers

int readVC1Sequence(VC1Header *v, Common::BitStream8MSB *gb) {

	v->profile = gb->getBits(2);
	if (v->profile == PROFILE_COMPLEX) {
		warning("WMV3 Complex Profile is not fully supported\n");
	}

	if (v->profile == PROFILE_ADVANCED) {
		v->zz_8x4 = ff_vc1_adv_progressive_8x4_zz;
		v->zz_4x8 = ff_vc1_adv_progressive_4x8_zz;
		return readVC1Advanced(v, gb);
	} else {
		v->chromaformat = 1;
		v->zz_8x4 = ff_wmv2_scantableA;
		v->zz_4x8 = ff_wmv2_scantableB;
		v->res_y411 = gb->getBit();
		v->res_sprite = gb->getBit();
		if (v->res_y411) {
			warning("Old interlaced mode is not supported\n");
			return -1;
		}
	}

	// (fps-2)/4 (->30)
	v->frmrtq_postproc = gb->getBits(3); // common
	// (bitrate-32kbps)/64kbps
	v->bitrtq_postproc = gb->getBits(5); // common
	v->loop_filter = gb->getBit();       // common
	if (v->loop_filter == 1 && v->profile == PROFILE_SIMPLE) {
		warning("LOOPFILTER shall not be enabled in Simple Profile\n");
	}

	v->res_x8 = gb->getBit(); // reserved
	v->multires = gb->getBit();
	v->res_fasttx = gb->getBit();

	v->fastuvmc = gb->getBit(); // common
	if (!v->profile && !v->fastuvmc) {
		warning("FASTUVMC unavailable in Simple Profile\n");
		return -1;
	}
	v->extended_mv = gb->getBit(); // common
	if (!v->profile && v->extended_mv) {
		warning("Extended MVs unavailable in Simple Profile\n");
		return -1;
	}
	v->dquant = gb->getBits(2);    // common
	v->vstransform = gb->getBit(); // common

	v->res_transtab = gb->getBit();
	if (v->res_transtab) {
		warning("1 for reserved RES_TRANSTAB is forbidden\n");
		return -1;
	}

	v->overlap = gb->getBit(); // common

	v->resync_marker = gb->getBit();
	v->rangered = gb->getBit();
	if (v->rangered && v->profile == PROFILE_SIMPLE) {
		warning("RANGERED should be set to 0 in Simple Profile\n");
	}

	v->max_b_frames = gb->getBits(3);   // common
	v->quantizer_mode = gb->getBits(2); // common

	v->finterpflag = gb->getBit(); // common

	if (v->res_sprite) {
		int w = gb->getBits(11);
		int h = gb->getBits(11);
		int ret = setDimensions(v, w, h);
		if (ret < 0) {
			warning("Failed to set dimensions %d %d\n", w, h);
			return ret;
		}
		gb->skip(5); // frame rate
		v->res_x8 = gb->getBit();
		if (gb->getBit()) { // something to do with DC VLC selection
			warning("Unsupported sprite feature\n");
			return -1;
		}
		gb->skip(3); // slice code
		v->res_rtm_flag = 0;
	} else {
		v->res_rtm_flag = gb->getBit(); // reserved
	}
	// TODO: figure out what they mean (always 0x402F)
	if (gb->pos() > gb->size())
		return -1;
	if (!v->res_fasttx)
		gb->skip(16);
	return 0;
}

static int readVC1Advanced(VC1Header *v, Common::BitStream8MSB *gb) {
	v->res_rtm_flag = 1;
	v->level = gb->getBits(3);
	if (v->level >= 5) {
		warning("Reserved LEVEL %i\n", v->level);
	}
	v->chromaformat = gb->getBits(2);
	if (v->chromaformat != 1) {
		warning("Only 4:2:0 chroma format supported\n");
		return -1;
	}

	// (fps-2)/4 (->30)
	v->frmrtq_postproc = gb->getBits(3); // common
	// (bitrate-32kbps)/64kbps
	v->bitrtq_postproc = gb->getBits(5); // common
	v->postprocflag = gb->getBit();      // common

	v->max_coded_width = (gb->getBits(12) + 1) << 1;
	v->max_coded_height = (gb->getBits(12) + 1) << 1;
	v->broadcast = gb->getBit();
	v->interlace = gb->getBit();
	v->tfcntrflag = gb->getBit();
	v->finterpflag = gb->getBit();
	gb->skip(1); // reserved

	v->psf = gb->getBit();
	if (v->psf) { // PsF, 6.1.13
		warning("Progressive Segmented Frame mode: not supported (yet)\n");
		return -1;
	}
	v->max_b_frames = 7;
	if (gb->getBit()) { // Display Info - decoding is not affected by it
		int w, h, ar = 0;

		w = gb->getBits(14) + 1;
		h = gb->getBits(14) + 1;

		if (gb->getBit())
			ar = gb->getBits(4);
		if (ar && ar < 14) {
			v->sample_aspect_ratio = ff_vc1_pixel_aspect[ar];
		} else if (ar == 15) {
			w = gb->getBits(8) + 1;
			h = gb->getBits(8) + 1;
			v->sample_aspect_ratio = VC1Ratio(w, h);
		} else {
			if (v->width > v->max_coded_width || v->height > v->max_coded_height) {
				warning("VC-1: Huge resolution");
			} else
				reduceAspect(v->sample_aspect_ratio, v->height * w, v->width * h);
		}

		validateAspect(v);

		if (gb->getBit()) { // framerate stuff
			if (gb->getBit()) {
				v->framerate.den = 32;
				v->framerate.num = gb->getBits(16) + 1;
			} else {
				int nr, dr;
				nr = gb->getBits(8);
				dr = gb->getBits(4);
				if (nr > 0 && nr < 8 && dr > 0 && dr < 3) {
					v->framerate.den = ff_vc1_fps_dr[dr - 1];
					v->framerate.num = ff_vc1_fps_nr[nr - 1] * 1000;
				}
			}
		}

		if (gb->getBit()) {
			v->color_prim = gb->getBits(8);
			v->transfer_char = gb->getBits(8);
			v->matrix_coef = gb->getBits(8);
		}
	}

	v->hrd_param_flag = gb->getBit();
	if (v->hrd_param_flag) {
		int i;
		v->hrd_num_leaky_buckets = gb->getBits(5);
		gb->skip(4); // bitrate exponent
		gb->skip(4); // buffer size exponent
		for (i = 0; i < v->hrd_num_leaky_buckets; i++) {
			gb->skip(16); // hrd_rate[n]
			gb->skip(16); // hrd_buffer[n]
		}
	}
	return gb->pos() <= gb->size() ? 0 : -1;
}

int readVC1Entry(VC1Header *v, Common::BitStream8MSB *gb) {
	int i;
	int w, h;
	int ret;

	v->broken_link = gb->getBit();
	v->closed_entry = gb->getBit();
	v->panscanflag = gb->getBit();
	v->refdist_flag = gb->getBit();
	v->loop_filter = gb->getBit();
	v->fastuvmc = gb->getBit();
	v->extended_mv = gb->getBit();
	v->dquant = gb->getBits(2);
	v->vstransform = gb->getBit();
	v->overlap = gb->getBit();
	v->quantizer_mode = gb->getBits(2);

	if (v->hrd_param_flag) {
		for (i = 0; i < v->hrd_num_leaky_buckets; i++) {
			gb->skip(8); // hrd_full[n]
		}
	}

	if (gb->getBit()) {
		w = (gb->getBits(12) + 1) << 1;
		h = (gb->getBits(12) + 1) << 1;
	} else {
		w = v->max_coded_width;
		h = v->max_coded_height;
	}
	if ((ret = setDimensions(v, w, h)) < 0) {
		warning("Failed to set dimensions %d %d\n", w, h);
		return ret;
	}

	if (v->extended_mv)
		v->extended_dmv = gb->getBit();
	if ((v->range_mapy_flag = gb->getBit())) {
		warning("Luma scaling is not supported, expect wrong picture\n");
		v->range_mapy = gb->getBits(3);
	}
	if ((v->range_mapuv_flag = gb->getBit())) {
		warning("Chroma scaling is not supported, expect wrong picture\n");
		v->range_mapuv = gb->getBits(3);
	}

	return gb->pos() <= gb->size() ? 0 : -1;
}

enum { IMODE_RAW, IMODE_NORM2, IMODE_DIFF2, IMODE_NORM6, IMODE_DIFF6, IMODE_ROWSKIP, IMODE_COLSKIP };

/* BitPlane IMODE - such a small table... */
static const uint32 vc1_imode_codes[7] = {0, 2, 1, 3, 1, 2, 3};
static const uint8 vc1_imode_bits[7] = {4, 2, 3, 2, 4, 3, 3};

/* Normal-2 imode */
static const uint32 vc1_norm2_codes[4] = {0, 4, 5, 3};
static const uint8 vc1_norm2_bits[4] = {1, 3, 3, 2};

static const uint32 vc1_norm6_codes[64] = {
	0x001, 0x002, 0x003, 0x000, 0x004, 0x001, 0x002, 0x047, 0x005, 0x003, 0x004, 0x04B, 0x005, 0x04D, 0x04E, 0x30E,
	0x006, 0x006, 0x007, 0x053, 0x008, 0x055, 0x056, 0x30D, 0x009, 0x059, 0x05A, 0x30C, 0x05C, 0x30B, 0x30A, 0x037,
	0x007, 0x00A, 0x00B, 0x043, 0x00C, 0x045, 0x046, 0x309, 0x00D, 0x049, 0x04A, 0x308, 0x04C, 0x307, 0x306, 0x036,
	0x00E, 0x051, 0x052, 0x305, 0x054, 0x304, 0x303, 0x035, 0x058, 0x302, 0x301, 0x034, 0x300, 0x033, 0x032, 0x007,
};

static const uint8 vc1_norm6_bits[64] = {
	1,  4,  4,  8,  4,  8,  8,  10, 4,  8,  8,  10, 8,  10, 10, 13, 4,  8,  8, 10, 8,  10,
	10, 13, 8,  10, 10, 13, 10, 13, 13, 9,  4,  8,  8,  10, 8,  10, 10, 13, 8, 10, 10, 13,
	10, 13, 13, 9,  8,  10, 10, 13, 10, 13, 13, 9,  10, 13, 13, 9,  13, 9,  9, 6,
};

static void initCodes(uint16 *table, uint maxBits, const uint32 *codes, const uint8 *lengths, uint count) {
	for (uint i = 0; i < (1U << maxBits); ++i)
		table[i] = 0xffff;
	for (uint i = 0; i < count; ++i) {
		uint shift = maxBits - lengths[i];
		uint start = codes[i] << shift;
		for (uint j = 0; j < (1U << shift); ++j)
			table[start + j] = (i << 4) | lengths[i];
	}
}

static int readCode(Common::BitStream8MSB &bits, const uint16 *table, uint maxBits) {
	uint16 entry = table[bits.peekBits(maxBits)];
	if (entry == 0xffff)
		return -1;
	bits.skip(entry & 15);
	if (bits.pos() > bits.size())
		return -1;
	return entry >> 4;
}

// Bitplane decoding

VC1Bitplane::VC1Bitplane() {
	initCodes(_imode, 4, vc1_imode_codes, vc1_imode_bits, 7);
	initCodes(_norm2, 3, vc1_norm2_codes, vc1_norm2_bits, 4);
	initCodes(_norm6, 13, vc1_norm6_codes, vc1_norm6_bits, 64);
}

static void decode_rowskip(uint8 *plane, int width, int height, int stride, Common::BitStream8MSB *gb) {
	int x, y;

	for (y = 0; y < height; y++) {
		if (!gb->getBit()) // rowskip
			memset(plane, 0, width);
		else
			for (x = 0; x < width; x++)
				plane[x] = gb->getBit();
		plane += stride;
	}
}

/** Decode columns by checking if they are skipped
 * @param plane Buffer to store decoded bits
 * @param[in] width Width of this buffer
 * @param[in] height Height of this buffer
 * @param[in] stride of this buffer
 * @todo FIXME: Optimize
 */
static void decode_colskip(uint8 *plane, int width, int height, int stride, Common::BitStream8MSB *gb) {
	int x, y;

	for (x = 0; x < width; x++) {
		if (!gb->getBit()) // colskip
			for (y = 0; y < height; y++)
				plane[y * stride] = 0;
		else
			for (y = 0; y < height; y++)
				plane[y * stride] = gb->getBit();
		plane++;
	}
}

/** Decode a bitplane's bits
 * @param data bitplane where to store the decode bits
 * @param[out] raw_flag pointer to the flag indicating that this bitplane is not coded explicitly
 * @param gb Bit stream containing the coded bitplane
 * @return Status
 * @todo FIXME: Optimize
 */
int VC1Bitplane::decode(uint8 *data, int *raw_flag, int width, int height, int stride,
						Common::BitStream8MSB *gb) const {
	if (!data || !raw_flag || width <= 0 || height <= 0 || stride < width)
		return -1;
	int imode, x, y, code, offset;
	uint8 invert, *planep = data;
	invert = gb->getBit();
	imode = readCode(*gb, _imode, 4);

	if (imode < 0)
		return -1;
	*raw_flag = 0;
	switch (imode) {
	case IMODE_RAW:
		// Data is actually read in the MB layer (same for all tests == "raw")
		*raw_flag = 1; // invert ignored
		return gb->pos() <= gb->size() ? invert : -1;
	case IMODE_DIFF2:
	case IMODE_NORM2:
		if ((height * width) & 1) {
			*planep++ = gb->getBit();
			y = offset = 1;
			if (offset == width) {
				offset = 0;
				planep += stride - width;
			}
		} else
			y = offset = 0;
		// decode bitplane as one long line
		for (; y < height * width; y += 2) {
			code = readCode(*gb, _norm2, 3);
			if (code < 0)
				return -1;
			*planep++ = code & 1;
			offset++;
			if (offset == width) {
				offset = 0;
				planep += stride - width;
			}
			*planep++ = code >> 1;
			offset++;
			if (offset == width) {
				offset = 0;
				planep += stride - width;
			}
		}
		break;
	case IMODE_DIFF6:
	case IMODE_NORM6:
		if (!(height % 3) && (width % 3)) { // use 2x3 decoding
			for (y = 0; y < height; y += 3) {
				for (x = width & 1; x < width; x += 2) {
					code = readCode(*gb, _norm6, 13);
					if (code < 0) {
						return -1;
					}
					planep[x + 0] = (code >> 0) & 1;
					planep[x + 1] = (code >> 1) & 1;
					planep[x + 0 + stride] = (code >> 2) & 1;
					planep[x + 1 + stride] = (code >> 3) & 1;
					planep[x + 0 + stride * 2] = (code >> 4) & 1;
					planep[x + 1 + stride * 2] = (code >> 5) & 1;
				}
				planep += stride * 3;
			}
			if (width & 1)
				decode_colskip(data, 1, height, stride, gb);
		} else { // 3x2
			planep += (height & 1) * stride;
			for (y = height & 1; y < height; y += 2) {
				for (x = width % 3; x < width; x += 3) {
					code = readCode(*gb, _norm6, 13);
					if (code < 0) {
						return -1;
					}
					planep[x + 0] = (code >> 0) & 1;
					planep[x + 1] = (code >> 1) & 1;
					planep[x + 2] = (code >> 2) & 1;
					planep[x + 0 + stride] = (code >> 3) & 1;
					planep[x + 1 + stride] = (code >> 4) & 1;
					planep[x + 2 + stride] = (code >> 5) & 1;
				}
				planep += stride * 2;
			}
			x = width % 3;
			if (x)
				decode_colskip(data, x, height, stride, gb);
			if (height & 1)
				decode_rowskip(data + x, width - x, 1, stride, gb);
		}
		break;
	case IMODE_ROWSKIP:
		decode_rowskip(data, width, height, stride, gb);
		break;
	case IMODE_COLSKIP:
		decode_colskip(data, width, height, stride, gb);
		break;
	default:
		break;
	}

	/* Applying diff operator */
	if (imode == IMODE_DIFF2 || imode == IMODE_DIFF6) {
		planep = data;
		planep[0] ^= invert;
		for (x = 1; x < width; x++)
			planep[x] ^= planep[x - 1];
		for (y = 1; y < height; y++) {
			planep += stride;
			planep[0] ^= planep[-stride];
			for (x = 1; x < width; x++) {
				if (planep[x - 1] != planep[x - stride])
					planep[x] ^= invert;
				else
					planep[x] ^= planep[x - 1];
			}
		}
	} else if (invert) {
		planep = data;
		for (x = 0; x < stride * height; x++)
			planep[x] = !planep[x]; // FIXME stride
	}
	return gb->pos() <= gb->size() ? (imode << 1) + invert : -1;
}

namespace VC1 {
const VLC *ff_msmp4_mb_i_vlc;
const VLC *ff_msmp4_dc_vlc[2][2];
/** Table for conversion between TTBLK and TTMB */
const int ff_vc1_ttblk_to_tt[3][8] = {
	{TT_8X4, TT_4X8, TT_8X8, TT_4X4, TT_8X4_TOP, TT_8X4_BOTTOM, TT_4X8_RIGHT, TT_4X8_LEFT},
	{TT_8X8, TT_4X8_RIGHT, TT_4X8_LEFT, TT_4X4, TT_8X4, TT_4X8, TT_8X4_BOTTOM, TT_8X4_TOP},
	{TT_8X8, TT_4X8, TT_4X4, TT_8X4_BOTTOM, TT_4X8_RIGHT, TT_4X8_LEFT, TT_8X4, TT_8X4_TOP}};

const int ff_vc1_ttfrm_to_tt[4] = {TT_8X8, TT_8X4, TT_4X8, TT_4X4};

/** MV P mode - the 5th element is only used for mode 1 */
const uint8 ff_vc1_mv_pmode_table[2][5] = {
	{MV_PMODE_1MV_HPEL_BILIN, MV_PMODE_1MV, MV_PMODE_1MV_HPEL, MV_PMODE_INTENSITY_COMP, MV_PMODE_MIXED_MV},
	{MV_PMODE_1MV, MV_PMODE_MIXED_MV, MV_PMODE_1MV_HPEL, MV_PMODE_INTENSITY_COMP, MV_PMODE_1MV_HPEL_BILIN}};
const uint8 ff_vc1_mv_pmode_table2[2][4] = {
	{MV_PMODE_1MV_HPEL_BILIN, MV_PMODE_1MV, MV_PMODE_1MV_HPEL, MV_PMODE_MIXED_MV},
	{MV_PMODE_1MV, MV_PMODE_MIXED_MV, MV_PMODE_1MV_HPEL, MV_PMODE_1MV_HPEL_BILIN}};

/* MBMODE table for interlaced frame P-picture */
const uint8 ff_vc1_mbmode_intfrp[2][15][4] = {{/* 1: 4-MV, 0: non-4-MV */
											   /* Type, FIELDTX, 1-MV Differential present, Residuals (CBP) present */
											   /* Table 164 - Table 167 */
											   {MV_PMODE_INTFR_1MV, 0, 1, 1},
											   {MV_PMODE_INTFR_1MV, 1, 1, 1},
											   {MV_PMODE_INTFR_1MV, 0, 1, 0},
											   {MV_PMODE_INTFR_1MV, 0, 0, 1},
											   {MV_PMODE_INTFR_1MV, 1, 0, 1},
											   {MV_PMODE_INTFR_2MV_FIELD, 0, 0, 1},
											   {MV_PMODE_INTFR_2MV_FIELD, 1, 0, 1},
											   {MV_PMODE_INTFR_2MV_FIELD, 1, 0, 0},
											   {MV_PMODE_INTFR_INTRA, 0, 0, 0}},
											  {/* Table 160 - Table 163 */
											   {MV_PMODE_INTFR_1MV, 0, 1, 1},
											   {MV_PMODE_INTFR_1MV, 1, 1, 1},
											   {MV_PMODE_INTFR_1MV, 0, 1, 0},
											   {MV_PMODE_INTFR_1MV, 0, 0, 1},
											   {MV_PMODE_INTFR_1MV, 1, 0, 1},
											   {MV_PMODE_INTFR_2MV_FIELD, 0, 0, 1},
											   {MV_PMODE_INTFR_2MV_FIELD, 1, 0, 1},
											   {MV_PMODE_INTFR_2MV_FIELD, 1, 0, 0},
											   {MV_PMODE_INTFR_4MV, 0, 0, 1},
											   {MV_PMODE_INTFR_4MV, 1, 0, 1},
											   {MV_PMODE_INTFR_4MV, 0, 0, 0},
											   {MV_PMODE_INTFR_4MV_FIELD, 0, 0, 1},
											   {MV_PMODE_INTFR_4MV_FIELD, 1, 0, 1},
											   {MV_PMODE_INTFR_4MV_FIELD, 1, 0, 0},
											   {MV_PMODE_INTFR_INTRA, 0, 0, 0}}};

const int ff_vc1_fps_nr[7] = {24, 25, 30, 50, 60, 48, 72}, ff_vc1_fps_dr[2] = {1000, 1001};
const uint8 ff_vc1_pquant_table[3][32] = {
	/* Implicit quantizer */
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 27, 29, 31},
	/* Explicit quantizer, pquantizer uniform */
	{0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
	 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31},
	/* Explicit quantizer, pquantizer non-uniform */
	{0,  1,  1,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13,
	 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 29, 31}};

/** @name VC-1 VLC tables
 *  @todo TODO move this into the context
 */
//@{
const VLC *ff_vc1_imode_vlc;
const VLC *ff_vc1_norm2_vlc;
const VLC *ff_vc1_norm6_vlc;
const VLC *ff_vc1_ttmb_vlc[3];
const VLC *ff_vc1_mv_diff_vlc[4];
const VLC *ff_vc1_cbpcy_p_vlc[4];
const VLC *ff_vc1_icbpcy_vlc[8];
const VLC *ff_vc1_4mv_block_pattern_vlc[4];
const VLC *ff_vc1_2mv_block_pattern_vlc[4];
const VLC *ff_vc1_ttblk_vlc[3];
const VLC *ff_vc1_subblkpat_vlc[3];
const VLC *ff_vc1_intfr_4mv_mbmode_vlc[4];
const VLC *ff_vc1_intfr_non4mv_mbmode_vlc[4];
const VLC *ff_vc1_if_mmv_mbmode_vlc[8];
const VLC *ff_vc1_if_1mv_mbmode_vlc[8];
const VLC *ff_vc1_1ref_mvdata_vlc[4];
const VLC *ff_vc1_2ref_mvdata_vlc[8];

const VLC *ff_vc1_ac_coeff_table[8];

//@}

#if B_FRACTION_DEN == 840 // original bfraction from vc9data.h, not conforming to standard
/* bfraction is fractional, we scale to the GCD 3*5*7*8 = 840 */
const int16 ff_vc1_bfraction_lut[23] = {
	420 /*1/2*/,
	280 /*1/3*/,
	560 /*2/3*/,
	210 /*1/4*/,
	630 /*3/4*/,
	168 /*1/5*/,
	336 /*2/5*/,
	504 /*3/5*/,
	672 /*4/5*/,
	140 /*1/6*/,
	700 /*5/6*/,
	120 /*1/7*/,
	240 /*2/7*/,
	360 /*3/7*/,
	480 /*4/7*/,
	600 /*5/7*/,
	720 /*6/7*/,
	105 /*1/8*/,
	315 /*3/8*/,
	525 /*5/8*/,
	735 /*7/8*/,
	-1 /*inv.*/,
	0 /*BI fm*/
};
#else
/* pre-computed scales for all bfractions and base=256 */
const int16 ff_vc1_bfraction_lut[23] = {
	128 /*1/2*/,
	85 /*1/3*/,
	170 /*2/3*/,
	64 /*1/4*/,
	192 /*3/4*/,
	51 /*1/5*/,
	102 /*2/5*/,
	153 /*3/5*/,
	204 /*4/5*/,
	43 /*1/6*/,
	215 /*5/6*/,
	37 /*1/7*/,
	74 /*2/7*/,
	111 /*3/7*/,
	148 /*4/7*/,
	185 /*5/7*/,
	222 /*6/7*/,
	32 /*1/8*/,
	96 /*3/8*/,
	160 /*5/8*/,
	224 /*7/8*/,
	-1 /*inv.*/,
	0 /*BI fm*/
};
#endif

// Same as H.264
const VC1Ratio ff_vc1_pixel_aspect[16] = {{0, 1},   {1, 1},    {12, 11}, {10, 11}, {16, 11}, {40, 33},
										  {24, 11}, {20, 11},  {32, 11}, {80, 33}, {18, 11}, {15, 11},
										  {64, 33}, {160, 99}, {0, 1},   {0, 1}};

const uint8 ff_wmv3_dc_scale_table[32] = {0,  2,  4,  8,  8,  8,  9,  9,  10, 10, 11, 11, 12, 12, 13, 13,
										  14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21};

/* DC differentials low+hi-mo, p217 are the same as in msmpeg4data .h */

/* Table 232 */
const uint8 ff_vc1_simple_progressive_4x4_zz[16] = {0, 8, 16, 1, 9, 24, 17, 2, 10, 18, 25, 3, 11, 26, 19, 27};

const uint8 ff_vc1_adv_progressive_8x4_zz[32] = {/* Table 233 */
												 0,  8,  1,  16, 2,  9, 10, 3,  24, 17, 4,  11, 18, 12, 5,  19,
												 25, 13, 20, 26, 27, 6, 21, 28, 14, 22, 29, 7,  30, 15, 23, 31};

const uint8 ff_vc1_adv_progressive_4x8_zz[32] = {/* Table 234 */
												 0,  1,  8,  2,  9,  16, 17, 24, 10, 32, 25, 18, 40, 3,  33, 26,
												 48, 11, 56, 41, 34, 49, 57, 42, 19, 50, 27, 58, 35, 43, 51, 59};

const uint8 ff_vc1_adv_interlaced_8x8_zz[64] = {/* Table 235 */
												0,  8,  1,  16, 24, 9,  2,  32, 40, 48, 56, 17, 10, 3,  25, 18,
												11, 4,  33, 41, 49, 57, 26, 34, 42, 50, 58, 19, 12, 5,  27, 20,
												13, 6,  35, 28, 21, 14, 7,  15, 22, 29, 36, 43, 51, 59, 60, 52,
												44, 37, 30, 23, 31, 38, 45, 53, 61, 62, 54, 46, 39, 47, 55, 63};

const uint8 ff_vc1_adv_interlaced_8x4_zz[32] = {/* Table 236 */
												0,  8, 16, 24, 1,  9, 2,  17, 25, 10, 3,  18, 26, 4,  11, 19,
												12, 5, 13, 20, 27, 6, 21, 28, 14, 22, 29, 7,  30, 15, 23, 31};

const uint8 ff_vc1_adv_interlaced_4x8_zz[32] = {/* Table 237 */
												0,  1,  2,  8,  16, 9,  24, 17, 10, 3,  32, 40, 48, 56, 25, 18,
												33, 26, 41, 34, 49, 57, 11, 42, 19, 50, 27, 58, 35, 43, 51, 59};

const uint8 ff_vc1_adv_interlaced_4x4_zz[16] = {/* Table 238 */
												0, 8, 16, 24, 1, 9, 17, 2, 25, 10, 18, 3, 26, 11, 19, 27};

/* DQScale as specified in 8.1.3.9 - almost identical to 0x40000/i */
const int32 ff_vc1_dqscale[63] = {
	0x40000, 0x20000, 0x15555, 0x10000, 0xCCCD, 0xAAAB, 0x9249, 0x8000, 0x71C7, 0x6666, 0x5D17, 0x5555, 0x4EC5,
	0x4925,  0x4444,  0x4000,  0x3C3C,  0x38E4, 0x35E5, 0x3333, 0x30C3, 0x2E8C, 0x2C86, 0x2AAB, 0x28F6, 0x2762,
	0x25ED,  0x2492,  0x234F,  0x2222,  0x2108, 0x2000, 0x1F08, 0x1E1E, 0x1D42, 0x1C72, 0x1BAD, 0x1AF3, 0x1A42,
	0x199A,  0x18FA,  0x1862,  0x17D0,  0x1746, 0x16C1, 0x1643, 0x15CA, 0x1555, 0x14E6, 0x147B, 0x1414, 0x13B1,
	0x1352,  0x12F7,  0x129E,  0x1249,  0x11F7, 0x11A8, 0x115B, 0x1111, 0x10C9, 0x1084, 0x1041};

/* P Interlaced field picture MV predictor scaling values (Table 114) */
const uint16 ff_vc1_field_mvpred_scales[2][7][4] = {
	// Refdist:
	//      0       1       2       3 or greater
	{
		// current field is first
		{128, 192, 213, 224}, // SCALEOPP
		{512, 341, 307, 293}, // SCALESAME1
		{219, 236, 242, 245}, // SCALESAME2
		{32, 48, 53, 56},     // SCALEZONE1_X
		{8, 12, 13, 14},      // SCALEZONE1_Y
		{37, 20, 14, 11},     // ZONE1OFFSET_X
		{10, 5, 4, 3}         // ZONE1OFFSET_Y
	},
	{
		// current field is second
		{128, 64, 43, 32},       // SCALEOPP
		{512, 1024, 1536, 2048}, // SCALESAME1
		{219, 204, 200, 198},    // SCALESAME2
		{32, 16, 11, 8},         // SCALEZONE1_X
		{8, 4, 3, 2},            // SCALEZONE1_Y
		{37, 52, 56, 58},        // ZONE1OFFSET_X
		{10, 13, 14, 15}         // ZONE1OFFSET_Y
	}};

/* B Interlaced field picture backward MV predictor scaling values for first field (Table 115) */
const uint16 ff_vc1_b_field_mvpred_scales[7][4] = {
	// BRFD:
	//  0       1       2       3 or greater
	{171, 205, 219, 228}, // SCALESAME
	{384, 320, 299, 288}, // SCALEOPP1
	{230, 239, 244, 246}, // SCALEOPP2
	{43, 51, 55, 57},     // SCALEZONE1_X
	{11, 13, 14, 14},     // SCALEZONE1_Y
	{26, 17, 12, 10},     // ZONE1OFFSET_X
	{7, 4, 3, 3}          // ZONE1OFFSET_Y
};

const uint8 ff_wmv1_scantable[WMV1_SCANTABLE_COUNT][64] = {
	{
		0x00, 0x08, 0x01, 0x02, 0x09, 0x10, 0x18, 0x11, 0x0A, 0x03, 0x04, 0x0B, 0x12, 0x19, 0x20, 0x28,
		0x30, 0x38, 0x29, 0x21, 0x1A, 0x13, 0x0C, 0x05, 0x06, 0x0D, 0x14, 0x1B, 0x22, 0x31, 0x39, 0x3A,
		0x32, 0x2A, 0x23, 0x1C, 0x15, 0x0E, 0x07, 0x0F, 0x16, 0x1D, 0x24, 0x2B, 0x33, 0x3B, 0x3C, 0x34,
		0x2C, 0x25, 0x1E, 0x17, 0x1F, 0x26, 0x2D, 0x35, 0x3D, 0x3E, 0x36, 0x2E, 0x27, 0x2F, 0x37, 0x3F,
	},
	{
		0x00, 0x08, 0x01, 0x02, 0x09, 0x10, 0x18, 0x11, 0x0A, 0x03, 0x04, 0x0B, 0x12, 0x19, 0x20, 0x28,
		0x21, 0x30, 0x1A, 0x13, 0x0C, 0x05, 0x06, 0x0D, 0x14, 0x1B, 0x22, 0x29, 0x38, 0x31, 0x39, 0x2A,
		0x23, 0x1C, 0x15, 0x0E, 0x07, 0x0F, 0x16, 0x1D, 0x24, 0x2B, 0x32, 0x3A, 0x33, 0x3B, 0x2C, 0x25,
		0x1E, 0x17, 0x1F, 0x26, 0x2D, 0x34, 0x3C, 0x35, 0x3D, 0x2E, 0x27, 0x2F, 0x36, 0x3E, 0x37, 0x3F,
	},
	{
		0x00, 0x01, 0x08, 0x02, 0x03, 0x09, 0x10, 0x18, 0x11, 0x0A, 0x04, 0x05, 0x0B, 0x12, 0x19, 0x20,
		0x28, 0x30, 0x21, 0x1A, 0x13, 0x0C, 0x06, 0x07, 0x0D, 0x14, 0x1B, 0x22, 0x29, 0x38, 0x31, 0x39,
		0x2A, 0x23, 0x1C, 0x15, 0x0E, 0x0F, 0x16, 0x1D, 0x24, 0x2B, 0x32, 0x3A, 0x33, 0x2C, 0x25, 0x1E,
		0x17, 0x1F, 0x26, 0x2D, 0x34, 0x3B, 0x3C, 0x35, 0x2E, 0x27, 0x2F, 0x36, 0x3D, 0x3E, 0x37, 0x3F,
	},
	{
		0x00, 0x08, 0x10, 0x01, 0x18, 0x20, 0x28, 0x09, 0x02, 0x03, 0x0A, 0x11, 0x19, 0x30, 0x38, 0x29,
		0x21, 0x1A, 0x12, 0x0B, 0x04, 0x05, 0x0C, 0x13, 0x1B, 0x22, 0x31, 0x39, 0x32, 0x2A, 0x23, 0x1C,
		0x14, 0x0D, 0x06, 0x07, 0x0E, 0x15, 0x1D, 0x24, 0x2B, 0x33, 0x3A, 0x3B, 0x34, 0x2C, 0x25, 0x1E,
		0x16, 0x0F, 0x17, 0x1F, 0x26, 0x2D, 0x3C, 0x35, 0x2E, 0x27, 0x2F, 0x36, 0x3D, 0x3E, 0x37, 0x3F,
	}};

// VLC table initialization

void initTables() {

	ff_vc1_norm2_vlc = makeVLC(4, vc1_norm2_bits, 1, 1, vc1_norm2_codes, 1, 1);
	ff_vc1_norm6_vlc = makeVLC(64, vc1_norm6_bits, 1, 1, vc1_norm6_codes, 2, 2);
	ff_vc1_imode_vlc = makeVLC(7, vc1_imode_bits, 1, 1, vc1_imode_codes, 1, 1);
	for (int i = 0; i < 3; i++) {
		ff_vc1_ttmb_vlc[i] = makeVLC(16, vc1_ttmb_bits[i], 1, 1, vc1_ttmb_codes[i], 2, 2);
		ff_vc1_ttblk_vlc[i] = makeVLC(8, vc1_ttblk_bits[i], 1, 1, vc1_ttblk_codes[i], 1, 1);
		ff_vc1_subblkpat_vlc[i] = makeVLC(15, vc1_subblkpat_bits[i], 1, 1, vc1_subblkpat_codes[i], 1, 1);
	}
	for (int i = 0; i < 4; i++) {
		ff_vc1_4mv_block_pattern_vlc[i] =
			makeVLC(16, vc1_4mv_block_pattern_bits[i], 1, 1, vc1_4mv_block_pattern_codes[i], 1, 1);
		ff_vc1_cbpcy_p_vlc[i] = makeVLC(64, vc1_cbpcy_p_bits[i], 1, 1, vc1_cbpcy_p_codes[i], 2, 2);
		ff_vc1_mv_diff_vlc[i] = makeVLC(73, vc1_mv_diff_bits[i], 1, 1, vc1_mv_diff_codes[i], 2, 2);
		/* initialize 4MV MBMODE VLC tables for interlaced frame P picture */
		ff_vc1_intfr_4mv_mbmode_vlc[i] =
			makeVLC(15, vc1_intfr_4mv_mbmode_bits[i], 1, 1, vc1_intfr_4mv_mbmode_codes[i], 2, 2);
		/* initialize NON-4MV MBMODE VLC tables for the same */
		ff_vc1_intfr_non4mv_mbmode_vlc[i] =
			makeVLC(9, vc1_intfr_non4mv_mbmode_bits[i], 1, 1, vc1_intfr_non4mv_mbmode_codes[i], 1, 1);
		/* initialize interlaced MVDATA tables (1-Ref) */
		ff_vc1_1ref_mvdata_vlc[i] = makeVLC(72, vc1_1ref_mvdata_bits[i], 1, 1, vc1_1ref_mvdata_codes[i], 4, 4);
		/* Initialize 2MV Block pattern VLC tables */
		ff_vc1_2mv_block_pattern_vlc[i] =
			makeVLC(4, vc1_2mv_block_pattern_bits[i], 1, 1, vc1_2mv_block_pattern_codes[i], 1, 1);
	}
	for (int i = 0; i < 8; i++) {
		ff_vc1_ac_coeff_table[i] =
			makeVLC(ff_vc1_ac_sizes[i], &vc1_ac_tables[i][0][1], 8, 4, &vc1_ac_tables[i][0][0], 8, 4);
		/* initialize interlaced MVDATA tables (2-Ref) */
		ff_vc1_2ref_mvdata_vlc[i] = makeVLC(126, vc1_2ref_mvdata_bits[i], 1, 1, vc1_2ref_mvdata_codes[i], 4, 4);
		/* Initialize interlaced CBPCY VLC tables (Table 124 - Table 131) */
		ff_vc1_icbpcy_vlc[i] = makeVLC(63, vc1_icbpcy_p_bits[i], 1, 1, vc1_icbpcy_p_codes[i], 2, 2);
		/* Initialize interlaced field picture MBMODE VLC tables */
		ff_vc1_if_mmv_mbmode_vlc[i] = makeVLC(8, vc1_if_mmv_mbmode_bits[i], 1, 1, vc1_if_mmv_mbmode_codes[i], 1, 1);
		ff_vc1_if_1mv_mbmode_vlc[i] = makeVLC(6, vc1_if_1mv_mbmode_bits[i], 1, 1, vc1_if_1mv_mbmode_codes[i], 1, 1);
	}
	for (int i = 0; i < 2; ++i)
		for (int j = 0; j < 2; ++j)
			ff_msmp4_dc_vlc[i][j] =
				makeVLC(120, &ff_msmp4_dc_tables[i][j][0][1], 8, 4, &ff_msmp4_dc_tables[i][j][0][0], 8, 4);
	ff_msmp4_mb_i_vlc = makeVLC(64, &ff_msmp4_mb_i_table[0][1], 4, 2, &ff_msmp4_mb_i_table[0][0], 4, 2);
}

// Scalar reconstruction helpers

static uint32 tableValue(const void *p, int size) {
	if (size == 1)
		return *(const uint8 *)p;
	if (size == 2)
		return READ_UINT16(p);
	return READ_UINT32(p);
}

const VLC *makeVLC(int count, const void *lengths, int ls, int lz, const void *codes, int cs, int cz) {
	VLC *v = new VLC;
	v->nodes.push_back(VLC::Node());
	for (int i = 0; i < count; ++i) {
		int length = tableValue((const uint8 *)lengths + i * ls, lz);
		if (!length)
			continue;
		uint32 code = tableValue((const uint8 *)codes + i * cs, cz);
		int at = 0;
		for (int k = length - 1; k >= 0; --k) {
			int bit = (code >> k) & 1;
			int next = v->nodes[at].child[bit];
			if (next < 0) {
				next = v->nodes.size();
				v->nodes[at].child[bit] = next;
				v->nodes.push_back(VLC::Node());
			}
			at = next;
		}
		assert(v->nodes[at].symbol == -1);
		v->nodes[at].symbol = i;
	}
	return v;
}

int get_vlc2(Bits *b, const VLC *v) {
	int at = 0;
	if (!v)
		return -1;
	for (int k = 0; k < 32; ++k) {
		if (v->nodes[at].symbol >= 0)
			return v->nodes[at].symbol;
		if (get_bits_left(b) <= 0)
			return -1;
		at = v->nodes[at].child[get_bits1(b)];
		if (at < 0)
			return -1;
	}
	return -1;
}

static void clearBlock(int16 *p) {
	memset(p, 0, 64 * sizeof(*p));
}
static void clearBlocks(int16 *p) {
	memset(p, 0, 6 * 64 * sizeof(*p));
}
template<int mode> static void clampBlock(const int16 *b, uint8 *d, ptrdiff_t stride) {
	for (int y = 0; y < 8; ++y) {
		for (int x = 0; x < 8; ++x)
			d[x] = av_clip_uint8(b[y * 8 + x] + (mode == 1 ? 128 : mode == 2 ? d[x] : 0));
		d += stride;
	}
}

template<int width, int xy, bool round, bool average>
static void halfPixels(uint8 *d, const uint8 *s, ptrdiff_t stride, int h) {
	putHalfPixels<width>(d, s, stride, h, xy, !round, average);
}

template<int width, bool average>
static void chromaPixels(uint8 *d, const uint8 *s, ptrdiff_t stride, int h, int x, int y) {
	int a = (8 - x) * (8 - y), b = x * (8 - y), c = (8 - x) * y, e = x * y;
	for (int row = 0; row < h; ++row) {
		for (int col = 0; col < width; ++col) {
			int value = (a * s[col] + b * s[col + 1] + c * s[col + stride] + e * s[col + stride + 1] + 32) >> 6;
			d[col] = average ? (d[col] + value + 1) >> 1 : value;
		}
		d += stride;
		s += stride;
	}
}

static void edgePixels(uint8 *dst, const uint8 *src, ptrdiff_t ds, ptrdiff_t ss, int bw, int bh, int x, int y, int w,
					   int h) {
	for (int row = 0; row < bh; ++row) {
		int sy = CLIP(y + row, 0, h - 1) - y;
		for (int col = 0; col < bw; ++col)
			dst[col] = src[sy * ss + CLIP(x + col, 0, w - 1) - x];
		dst += ds;
	}
}

void initSupport(PictureState *s, ChromaDSP *c) {
	s->bdsp.clear_block = clearBlock;
	s->bdsp.clear_blocks = clearBlocks;
	s->idsp.put_pixels_clamped = clampBlock<0>;
	s->idsp.put_signed_pixels_clamped = clampBlock<1>;
	s->idsp.add_pixels_clamped = clampBlock<2>;
	s->vdsp.emulated_edge_mc = edgePixels;
#define H(w, i, xy)                                                                                                    \
	s->hdsp.put_pixels_tab[i][xy] = halfPixels<w, xy, true, false>;                                                    \
	s->hdsp.put_no_rnd_pixels_tab[i][xy] = halfPixels<w, xy, false, false>;                                            \
	s->hdsp.avg_pixels_tab[i][xy] = halfPixels<w, xy, true, true>;
	H(16, 0, 0)
	H(16, 0, 1)
	H(16, 0, 2)
	H(16, 0, 3)
	H(8, 1, 0)
	H(8, 1, 1)
	H(8, 1, 2)
	H(8, 1, 3)
#undef H
		s->hdsp.avg_no_rnd_pixels_tab[0] = halfPixels<16, 0, false, true>;
	s->hdsp.avg_no_rnd_pixels_tab[1] = halfPixels<16, 1, false, true>;
	s->hdsp.avg_no_rnd_pixels_tab[2] = halfPixels<16, 2, false, true>;
	s->hdsp.avg_no_rnd_pixels_tab[3] = halfPixels<16, 3, false, true>;
	c->put_h264_chroma_pixels_tab[0] = chromaPixels<8, false>;
	c->put_h264_chroma_pixels_tab[1] = chromaPixels<4, false>;
	c->put_h264_chroma_pixels_tab[2] = chromaPixels<2, false>;
	c->avg_h264_chroma_pixels_tab[0] = chromaPixels<8, true>;
	c->avg_h264_chroma_pixels_tab[1] = chromaPixels<4, true>;
	c->avg_h264_chroma_pixels_tab[2] = chromaPixels<2, true>;
}

void ff_init_block_index(PictureState *s) {
	const int linesize = s->cur_pic.linesize[0], uvlinesize = s->cur_pic.linesize[1];
	s->block_index[0] = s->b8_stride * (s->mb_y * 2) - 2 + s->mb_x * 2;
	s->block_index[1] = s->b8_stride * (s->mb_y * 2) - 1 + s->mb_x * 2;
	s->block_index[2] = s->b8_stride * (s->mb_y * 2 + 1) - 2 + s->mb_x * 2;
	s->block_index[3] = s->b8_stride * (s->mb_y * 2 + 1) - 1 + s->mb_x * 2;
	s->block_index[4] = s->mb_stride * (s->mb_y + 1) + s->b8_stride * s->mb_height * 2 + s->mb_x - 1;
	s->block_index[5] = s->mb_stride * (s->mb_y + s->mb_height + 2) + s->b8_stride * s->mb_height * 2 + s->mb_x - 1;
	s->dest[0] = s->cur_pic.data[0] + (s->mb_x - 1) * 16 + s->mb_y * linesize * 16;
	s->dest[1] = s->cur_pic.data[1] + (s->mb_x - 1) * 8 + s->mb_y * uvlinesize * 8;
	s->dest[2] = s->cur_pic.data[2] + (s->mb_x - 1) * 8 + s->mb_y * uvlinesize * 8;
}

void ff_update_block_index(PictureState *s, int, int, int) {
	for (int i = 0; i < 4; ++i)
		s->block_index[i] += 2;
	++s->block_index[4];
	++s->block_index[5];
	s->dest[0] += 16;
	s->dest[1] += 8;
	s->dest[2] += 8;
}

/**
 * @file
 * simpleidct in C.
 */
/* Based upon some commented-out C code from mpeg2dec (idct_mmx.c
 * written by Aaron Holtzman <aholtzma@ess.engr.uvic.ca>). */
static inline void idctRowCondDC_int16_8bit(int16 *row, int extra_shift) {
	unsigned a0, a1, a2, a3, b0, b1, b2, b3;
	// TODO: Add DC-only support for int32 input
	if (!(READ_UINT32(row + 2) | READ_UINT32(row + 4) | READ_UINT32(row + 6) | row[1])) {
		uint32 temp;
		if (3 - extra_shift >= 0) {
			temp = (row[0] * (1 << (3 - extra_shift))) & 0xffff;
		} else {
			temp = ((row[0] + (1 << (extra_shift - 3 - 1))) >> (extra_shift - 3)) & 0xffff;
		}
		temp += temp * (1 << 16);
		WRITE_UINT32(row, temp);
		WRITE_UINT32(row + 2, temp);
		WRITE_UINT32(row + 4, temp);
		WRITE_UINT32(row + 6, temp);
		return;
	}
	a0 = ((unsigned)16383 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * row[0]) + (1 << (11 + extra_shift - 1));
	a1 = a0;
	a2 = a0;
	a3 = a0;
	a0 += (unsigned)21407 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * row[2];
	a1 += (unsigned)8867 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * row[2];
	a2 -= (unsigned)8867 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * row[2];
	a3 -= (unsigned)21407 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * row[2];
	b0 = ((22725 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (row[1]));
	((b0) += (19266 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (row[3]));
	b1 = ((19266 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (row[1]));
	((b1) += (-4520 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (row[3]));
	b2 = ((12873 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (row[1]));
	((b2) += (-22725 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (row[3]));
	b3 = ((4520 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (row[1]));
	((b3) += (-12873 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (row[3]));
	if (READ_UINT64(row + 4)) {
		a0 += (unsigned)16383 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * row[4] +
			  (unsigned)8867 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * row[6];
		a1 += (unsigned)-16383 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * row[4] -
			  (unsigned)21407 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * row[6];
		a2 += (unsigned)-16383 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * row[4] +
			  (unsigned)21407 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * row[6];
		a3 += (unsigned)16383 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * row[4] -
			  (unsigned)8867 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * row[6];
		((b0) += (12873 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (row[5]));
		((b0) += (4520 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (row[7]));
		((b1) += (-22725 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (row[5]));
		((b1) += (-12873 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (row[7]));
		((b2) += (4520 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (row[5]));
		((b2) += (19266 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (row[7]));
		((b3) += (19266 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (row[5]));
		((b3) += (-22725 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (row[7]));
	}
	row[0] = (int)(a0 + b0) >> (11 + extra_shift);
	row[7] = (int)(a0 - b0) >> (11 + extra_shift);
	row[1] = (int)(a1 + b1) >> (11 + extra_shift);
	row[6] = (int)(a1 - b1) >> (11 + extra_shift);
	row[2] = (int)(a2 + b2) >> (11 + extra_shift);
	row[5] = (int)(a2 - b2) >> (11 + extra_shift);
	row[3] = (int)(a3 + b3) >> (11 + extra_shift);
	row[4] = (int)(a3 - b3) >> (11 + extra_shift);
}

static inline void idctSparseCol_int16_8bit(int16 *col) {
	unsigned a0, a1, a2, a3, b0, b1, b2, b3;
	do {
		a0 = (unsigned)16383 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ *
			 (col[8 * 0] + ((1 << (20 - 1)) / 16383 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/));
		a1 = a0;
		a2 = a0;
		a3 = a0;
		a0 += (unsigned)21407 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 2];
		a1 += (unsigned)8867 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 2];
		a2 += (unsigned)-8867 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 2];
		a3 += (unsigned)-21407 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 2];
		b0 = ((22725 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 1]));
		b1 = ((19266 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 1]));
		b2 = ((12873 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 1]));
		b3 = ((4520 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 1]));
		((b0) += (19266 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 3]));
		((b1) += (-4520 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 3]));
		((b2) += (-22725 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 3]));
		((b3) += (-12873 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 3]));
		if (col[8 * 4]) {
			a0 += (unsigned)16383 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 4];
			a1 += (unsigned)-16383 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 4];
			a2 += (unsigned)-16383 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 4];
			a3 += (unsigned)16383 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 4];
		}
		if (col[8 * 5]) {
			((b0) += (12873 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 5]));
			((b1) += (-22725 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 5]));
			((b2) += (4520 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 5]));
			((b3) += (19266 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 5]));
		}
		if (col[8 * 6]) {
			a0 += (unsigned)8867 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 6];
			a1 += (unsigned)-21407 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 6];
			a2 += (unsigned)21407 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 6];
			a3 += (unsigned)-8867 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 6];
		}
		if (col[8 * 7]) {
			((b0) += (4520 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 7]));
			((b1) += (-12873 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 7]));
			((b2) += (19266 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 7]));
			((b3) += (-22725 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 7]));
		}
	} while (0);
	col[0] = ((int)(a0 + b0) >> 20);
	col[8] = ((int)(a1 + b1) >> 20);
	col[16] = ((int)(a2 + b2) >> 20);
	col[24] = ((int)(a3 + b3) >> 20);
	col[32] = ((int)(a3 - b3) >> 20);
	col[40] = ((int)(a2 - b2) >> 20);
	col[48] = ((int)(a1 - b1) >> 20);
	col[56] = ((int)(a0 - b0) >> 20);
}

static inline void idctSparseColPut_int16_8bit(uint8 *dest, ptrdiff_t line_size, int16 *col) {
	unsigned a0, a1, a2, a3, b0, b1, b2, b3;
	do {
		a0 = (unsigned)16383 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ *
			 (col[8 * 0] + ((1 << (20 - 1)) / 16383 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/));
		a1 = a0;
		a2 = a0;
		a3 = a0;
		a0 += (unsigned)21407 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 2];
		a1 += (unsigned)8867 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 2];
		a2 += (unsigned)-8867 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 2];
		a3 += (unsigned)-21407 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 2];
		b0 = ((22725 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 1]));
		b1 = ((19266 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 1]));
		b2 = ((12873 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 1]));
		b3 = ((4520 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 1]));
		((b0) += (19266 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 3]));
		((b1) += (-4520 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 3]));
		((b2) += (-22725 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 3]));
		((b3) += (-12873 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 3]));
		if (col[8 * 4]) {
			a0 += (unsigned)16383 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 4];
			a1 += (unsigned)-16383 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 4];
			a2 += (unsigned)-16383 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 4];
			a3 += (unsigned)16383 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 4];
		}
		if (col[8 * 5]) {
			((b0) += (12873 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 5]));
			((b1) += (-22725 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 5]));
			((b2) += (4520 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 5]));
			((b3) += (19266 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 5]));
		}
		if (col[8 * 6]) {
			a0 += (unsigned)8867 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 6];
			a1 += (unsigned)-21407 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 6];
			a2 += (unsigned)21407 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 6];
			a3 += (unsigned)-8867 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 6];
		}
		if (col[8 * 7]) {
			((b0) += (4520 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 7]));
			((b1) += (-12873 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 7]));
			((b2) += (19266 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 7]));
			((b3) += (-22725 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 7]));
		}
	} while (0);
	dest[0] = av_clip_uint8((int)(a0 + b0) >> 20);
	dest += line_size;
	dest[0] = av_clip_uint8((int)(a1 + b1) >> 20);
	dest += line_size;
	dest[0] = av_clip_uint8((int)(a2 + b2) >> 20);
	dest += line_size;
	dest[0] = av_clip_uint8((int)(a3 + b3) >> 20);
	dest += line_size;
	dest[0] = av_clip_uint8((int)(a3 - b3) >> 20);
	dest += line_size;
	dest[0] = av_clip_uint8((int)(a2 - b2) >> 20);
	dest += line_size;
	dest[0] = av_clip_uint8((int)(a1 - b1) >> 20);
	dest += line_size;
	dest[0] = av_clip_uint8((int)(a0 - b0) >> 20);
}

static inline void idctSparseColAdd_int16_8bit(uint8 *dest, ptrdiff_t line_size, int16 *col) {
	unsigned a0, a1, a2, a3, b0, b1, b2, b3;
	do {
		a0 = (unsigned)16383 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ *
			 (col[8 * 0] + ((1 << (20 - 1)) / 16383 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/));
		a1 = a0;
		a2 = a0;
		a3 = a0;
		a0 += (unsigned)21407 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 2];
		a1 += (unsigned)8867 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 2];
		a2 += (unsigned)-8867 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 2];
		a3 += (unsigned)-21407 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 2];
		b0 = ((22725 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 1]));
		b1 = ((19266 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 1]));
		b2 = ((12873 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 1]));
		b3 = ((4520 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 1]));
		((b0) += (19266 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 3]));
		((b1) += (-4520 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 3]));
		((b2) += (-22725 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 3]));
		((b3) += (-12873 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 3]));
		if (col[8 * 4]) {
			a0 += (unsigned)16383 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 4];
			a1 += (unsigned)-16383 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 4];
			a2 += (unsigned)-16383 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 4];
			a3 += (unsigned)16383 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 4];
		}
		if (col[8 * 5]) {
			((b0) += (12873 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 5]));
			((b1) += (-22725 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 5]));
			((b2) += (4520 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 5]));
			((b3) += (19266 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 5]));
		}
		if (col[8 * 6]) {
			a0 += (unsigned)8867 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 6];
			a1 += (unsigned)-21407 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 6];
			a2 += (unsigned)21407 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 6];
			a3 += (unsigned)-8867 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/ * col[8 * 6];
		}
		if (col[8 * 7]) {
			((b0) += (4520 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 7]));
			((b1) += (-12873 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 7]));
			((b2) += (19266 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 7]));
			((b3) += (-22725 /*cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5*/) * (col[8 * 7]));
		}
	} while (0);
	dest[0] = av_clip_uint8(dest[0] + ((int)(a0 + b0) >> 20));
	dest += line_size;
	dest[0] = av_clip_uint8(dest[0] + ((int)(a1 + b1) >> 20));
	dest += line_size;
	dest[0] = av_clip_uint8(dest[0] + ((int)(a2 + b2) >> 20));
	dest += line_size;
	dest[0] = av_clip_uint8(dest[0] + ((int)(a3 + b3) >> 20));
	dest += line_size;
	dest[0] = av_clip_uint8(dest[0] + ((int)(a3 - b3) >> 20));
	dest += line_size;
	dest[0] = av_clip_uint8(dest[0] + ((int)(a2 - b2) >> 20));
	dest += line_size;
	dest[0] = av_clip_uint8(dest[0] + ((int)(a1 - b1) >> 20));
	dest += line_size;
	dest[0] = av_clip_uint8(dest[0] + ((int)(a0 - b0) >> 20));
}

void ff_simple_idct_put_int16_8bit(uint8 *dest_, ptrdiff_t line_size, int16 *block_) {
	int16 *block = (int16 *)block_;
	uint8 *dest = (uint8 *)dest_;
	int i;
	line_size /= sizeof(uint8);
	for (i = 0; i < 8; i++)
		idctRowCondDC_int16_8bit(block + i * 8, 0);
	for (i = 0; i < 8; i++)
		idctSparseColPut_int16_8bit(dest + i, line_size, block + i);
}

void ff_simple_idct_add_int16_8bit(uint8 *dest_, ptrdiff_t line_size, int16 *block) {
	uint8 *dest = (uint8 *)dest_;
	int i;
	line_size /= sizeof(uint8);
	for (i = 0; i < 8; i++)
		idctRowCondDC_int16_8bit(block + i * 8, 0);
	for (i = 0; i < 8; i++)
		idctSparseColAdd_int16_8bit(dest + i, line_size, block + i);
}

void ff_simple_idct_int16_8bit(int16 *block) {
	int i;
	for (i = 0; i < 8; i++)
		idctRowCondDC_int16_8bit(block + i * 8, 0);
	for (i = 0; i < 8; i++)
		idctSparseCol_int16_8bit(block + i);
}

static inline void idct4col_add(uint8 *dest, ptrdiff_t line_size, const int16 *col) {
	int c0, c1, c2, c3, a0, a1, a2, a3;
	a0 = col[8 * 0];
	a1 = col[8 * 1];
	a2 = col[8 * 2];
	a3 = col[8 * 3];
	c0 = (a0 + a2) * ((int)((0.5) * M_SQRT2 * (1 << 12) + 0.5)) + (1 << ((4 + 1 + 12) - 1));
	c2 = (a0 - a2) * ((int)((0.5) * M_SQRT2 * (1 << 12) + 0.5)) + (1 << ((4 + 1 + 12) - 1));
	c1 = a1 * ((int)((0.6532814824) * M_SQRT2 * (1 << 12) + 0.5)) +
		 a3 * ((int)((0.2705980501) * M_SQRT2 * (1 << 12) + 0.5));
	c3 = a1 * ((int)((0.2705980501) * M_SQRT2 * (1 << 12) + 0.5)) -
		 a3 * ((int)((0.6532814824) * M_SQRT2 * (1 << 12) + 0.5));
	dest[0] = av_clip_uint8(dest[0] + ((c0 + c1) >> (4 + 1 + 12)));
	dest += line_size;
	dest[0] = av_clip_uint8(dest[0] + ((c2 + c3) >> (4 + 1 + 12)));
	dest += line_size;
	dest[0] = av_clip_uint8(dest[0] + ((c2 - c3) >> (4 + 1 + 12)));
	dest += line_size;
	dest[0] = av_clip_uint8(dest[0] + ((c0 - c1) >> (4 + 1 + 12)));
}

static inline void idct4row(int16 *row) {
	unsigned c0, c1, c2, c3;
	int a0, a1, a2, a3;
	a0 = row[0];
	a1 = row[1];
	a2 = row[2];
	a3 = row[3];
	c0 = (a0 + a2) * ((int)((0.5) * M_SQRT2 * (1 << 15) + 0.5)) + (1 << (11 - 1));
	c2 = (a0 - a2) * ((int)((0.5) * M_SQRT2 * (1 << 15) + 0.5)) + (1 << (11 - 1));
	c1 = a1 * ((int)((0.6532814824) * M_SQRT2 * (1 << 15) + 0.5)) +
		 a3 * ((int)((0.2705980501) * M_SQRT2 * (1 << 15) + 0.5));
	c3 = a1 * ((int)((0.2705980501) * M_SQRT2 * (1 << 15) + 0.5)) -
		 a3 * ((int)((0.6532814824) * M_SQRT2 * (1 << 15) + 0.5));
	row[0] = (c0 + c1) >> 11;
	row[1] = (c2 + c3) >> 11;
	row[2] = (c2 - c3) >> 11;
	row[3] = (c0 - c1) >> 11;
}

void ff_simple_idct84_add(uint8 *dest, ptrdiff_t line_size, int16 *block) {
	int i;
	/* IDCT8 on each line */
	for (i = 0; i < 4; i++) {
		idctRowCondDC_int16_8bit(block + i * 8, 0);
	}
	/* IDCT4 and store */
	for (i = 0; i < 8; i++) {
		idct4col_add(dest + i, line_size, block + i);
	}
}

void ff_simple_idct48_add(uint8 *dest, ptrdiff_t line_size, int16 *block) {
	int i;
	/* IDCT4 on each line */
	for (i = 0; i < 8; i++) {
		idct4row(block + i * 8);
	}
	/* IDCT8 and store */
	for (i = 0; i < 4; i++) {
		idctSparseColAdd_int16_8bit(dest + i, line_size, block + i);
	}
}

void ff_simple_idct44_add(uint8 *dest, ptrdiff_t line_size, int16 *block) {
	int i;
	/* IDCT4 on each line */
	for (i = 0; i < 4; i++) {
		idct4row(block + i * 8);
	}
	/* IDCT4 and store */
	for (i = 0; i < 4; i++) {
		idct4col_add(dest + i, line_size, block + i);
	}
}

static void copy_block8(uint8 *d, const uint8 *s, ptrdiff_t ds, ptrdiff_t ss, int h) {
	for (int y = 0; y < h; ++y) {
		memcpy(d, s, 8);
		d += ds;
		s += ss;
	}
}

static void copy_block9(uint8 *d, const uint8 *s, ptrdiff_t ds, ptrdiff_t ss, int h) {
	for (int y = 0; y < h; ++y) {
		memcpy(d, s, 9);
		d += ds;
		s += ss;
	}
}

static void copy_block16(uint8 *d, const uint8 *s, ptrdiff_t ds, ptrdiff_t ss, int h) {
	for (int y = 0; y < h; ++y) {
		memcpy(d, s, 16);
		d += ds;
		s += ss;
	}
}

static void copy_block17(uint8 *d, const uint8 *s, ptrdiff_t ds, ptrdiff_t ss, int h) {
	for (int y = 0; y < h; ++y) {
		memcpy(d, s, 17);
		d += ds;
		s += ss;
	}
}

static void put_pixels8_8_c(uint8 *d, const uint8 *s, ptrdiff_t stride, int h) {
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < 8; ++x) {
			int val = s[x];
			d[x] = val;
		}
		d += stride;
		s += stride;
	}
}

static void put_pixels8_l2_8(uint8 *d, const uint8 *a, const uint8 *b, int ds, int as, int bs, int h) {
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < 8; ++x) {
			int val = (a[x] + b[x] + 1) >> 1;
			d[x] = val;
		}
		d += ds;
		a += as;
		b += bs;
	}
}

static void put_pixels8_l4_8(uint8 *d, const uint8 *a, const uint8 *b, const uint8 *c, const uint8 *e, int ds, int as,
							 int bs, int cs, int es, int h) {
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < 8; ++x) {
			int val = (a[x] + b[x] + c[x] + e[x] + 2) >> 2;
			d[x] = val;
		}
		d += ds;
		a += as;
		b += bs;
		c += cs;
		e += es;
	}
}

static void avg_pixels8_8_c(uint8 *d, const uint8 *s, ptrdiff_t stride, int h) {
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < 8; ++x) {
			int val = s[x];
			d[x] = (d[x] + val + 1) >> 1;
		}
		d += stride;
		s += stride;
	}
}

static void avg_pixels8_l2_8(uint8 *d, const uint8 *a, const uint8 *b, int ds, int as, int bs, int h) {
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < 8; ++x) {
			int val = (a[x] + b[x] + 1) >> 1;
			d[x] = (d[x] + val + 1) >> 1;
		}
		d += ds;
		a += as;
		b += bs;
	}
}

static void avg_pixels8_l4_8(uint8 *d, const uint8 *a, const uint8 *b, const uint8 *c, const uint8 *e, int ds, int as,
							 int bs, int cs, int es, int h) {
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < 8; ++x) {
			int val = (a[x] + b[x] + c[x] + e[x] + 2) >> 2;
			d[x] = (d[x] + val + 1) >> 1;
		}
		d += ds;
		a += as;
		b += bs;
		c += cs;
		e += es;
	}
}

static void put_no_rnd_pixels8_l2_8(uint8 *d, const uint8 *a, const uint8 *b, int ds, int as, int bs, int h) {
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < 8; ++x) {
			int val = (a[x] + b[x] + 0) >> 1;
			d[x] = val;
		}
		d += ds;
		a += as;
		b += bs;
	}
}

static void put_no_rnd_pixels8_l4_8(uint8 *d, const uint8 *a, const uint8 *b, const uint8 *c, const uint8 *e, int ds,
									int as, int bs, int cs, int es, int h) {
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < 8; ++x) {
			int val = (a[x] + b[x] + c[x] + e[x] + 1) >> 2;
			d[x] = val;
		}
		d += ds;
		a += as;
		b += bs;
		c += cs;
		e += es;
	}
}

static void put_pixels16_8_c(uint8 *d, const uint8 *s, ptrdiff_t stride, int h) {
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < 16; ++x) {
			int val = s[x];
			d[x] = val;
		}
		d += stride;
		s += stride;
	}
}

static void put_pixels16_l2_8(uint8 *d, const uint8 *a, const uint8 *b, int ds, int as, int bs, int h) {
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < 16; ++x) {
			int val = (a[x] + b[x] + 1) >> 1;
			d[x] = val;
		}
		d += ds;
		a += as;
		b += bs;
	}
}

static void put_pixels16_l4_8(uint8 *d, const uint8 *a, const uint8 *b, const uint8 *c, const uint8 *e, int ds, int as,
							  int bs, int cs, int es, int h) {
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < 16; ++x) {
			int val = (a[x] + b[x] + c[x] + e[x] + 2) >> 2;
			d[x] = val;
		}
		d += ds;
		a += as;
		b += bs;
		c += cs;
		e += es;
	}
}

static void avg_pixels16_8_c(uint8 *d, const uint8 *s, ptrdiff_t stride, int h) {
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < 16; ++x) {
			int val = s[x];
			d[x] = (d[x] + val + 1) >> 1;
		}
		d += stride;
		s += stride;
	}
}

static void avg_pixels16_l2_8(uint8 *d, const uint8 *a, const uint8 *b, int ds, int as, int bs, int h) {
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < 16; ++x) {
			int val = (a[x] + b[x] + 1) >> 1;
			d[x] = (d[x] + val + 1) >> 1;
		}
		d += ds;
		a += as;
		b += bs;
	}
}

static void avg_pixels16_l4_8(uint8 *d, const uint8 *a, const uint8 *b, const uint8 *c, const uint8 *e, int ds, int as,
							  int bs, int cs, int es, int h) {
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < 16; ++x) {
			int val = (a[x] + b[x] + c[x] + e[x] + 2) >> 2;
			d[x] = (d[x] + val + 1) >> 1;
		}
		d += ds;
		a += as;
		b += bs;
		c += cs;
		e += es;
	}
}

static void put_no_rnd_pixels16_l2_8(uint8 *d, const uint8 *a, const uint8 *b, int ds, int as, int bs, int h) {
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < 16; ++x) {
			int val = (a[x] + b[x] + 0) >> 1;
			d[x] = val;
		}
		d += ds;
		a += as;
		b += bs;
	}
}

static void put_no_rnd_pixels16_l4_8(uint8 *d, const uint8 *a, const uint8 *b, const uint8 *c, const uint8 *e, int ds,
									 int as, int bs, int cs, int es, int h) {
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < 16; ++x) {
			int val = (a[x] + b[x] + c[x] + e[x] + 1) >> 2;
			d[x] = val;
		}
		d += ds;
		a += as;
		b += bs;
		c += cs;
		e += es;
	}
}
#define QPEL_MC(r, OPNAME, RND, OP)                                                                                    \
	static void OPNAME##mpeg4_qpel8_h_lowpass(uint8 *dst, const uint8 *src, int dstStride, int srcStride, int h) {     \
		const Crop cm;                                                                                                 \
		int i;                                                                                                         \
                                                                                                                       \
		for (i = 0; i < h; i++) {                                                                                      \
			OP(dst[0], (src[0] + src[1]) * 20 - (src[0] + src[2]) * 6 + (src[1] + src[3]) * 3 - (src[2] + src[4]));    \
			OP(dst[1], (src[1] + src[2]) * 20 - (src[0] + src[3]) * 6 + (src[0] + src[4]) * 3 - (src[1] + src[5]));    \
			OP(dst[2], (src[2] + src[3]) * 20 - (src[1] + src[4]) * 6 + (src[0] + src[5]) * 3 - (src[0] + src[6]));    \
			OP(dst[3], (src[3] + src[4]) * 20 - (src[2] + src[5]) * 6 + (src[1] + src[6]) * 3 - (src[0] + src[7]));    \
			OP(dst[4], (src[4] + src[5]) * 20 - (src[3] + src[6]) * 6 + (src[2] + src[7]) * 3 - (src[1] + src[8]));    \
			OP(dst[5], (src[5] + src[6]) * 20 - (src[4] + src[7]) * 6 + (src[3] + src[8]) * 3 - (src[2] + src[8]));    \
			OP(dst[6], (src[6] + src[7]) * 20 - (src[5] + src[8]) * 6 + (src[4] + src[8]) * 3 - (src[3] + src[7]));    \
			OP(dst[7], (src[7] + src[8]) * 20 - (src[6] + src[8]) * 6 + (src[5] + src[7]) * 3 - (src[4] + src[6]));    \
			dst += dstStride;                                                                                          \
			src += srcStride;                                                                                          \
		}                                                                                                              \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##mpeg4_qpel8_v_lowpass(uint8 *dst, const uint8 *src, int dstStride, int srcStride) {            \
		const Crop cm;                                                                                                 \
		const int w = 8;                                                                                               \
		int i;                                                                                                         \
                                                                                                                       \
		for (i = 0; i < w; i++) {                                                                                      \
			const int src0 = src[0 * srcStride];                                                                       \
			const int src1 = src[1 * srcStride];                                                                       \
			const int src2 = src[2 * srcStride];                                                                       \
			const int src3 = src[3 * srcStride];                                                                       \
			const int src4 = src[4 * srcStride];                                                                       \
			const int src5 = src[5 * srcStride];                                                                       \
			const int src6 = src[6 * srcStride];                                                                       \
			const int src7 = src[7 * srcStride];                                                                       \
			const int src8 = src[8 * srcStride];                                                                       \
			OP(dst[0 * dstStride], (src0 + src1) * 20 - (src0 + src2) * 6 + (src1 + src3) * 3 - (src2 + src4));        \
			OP(dst[1 * dstStride], (src1 + src2) * 20 - (src0 + src3) * 6 + (src0 + src4) * 3 - (src1 + src5));        \
			OP(dst[2 * dstStride], (src2 + src3) * 20 - (src1 + src4) * 6 + (src0 + src5) * 3 - (src0 + src6));        \
			OP(dst[3 * dstStride], (src3 + src4) * 20 - (src2 + src5) * 6 + (src1 + src6) * 3 - (src0 + src7));        \
			OP(dst[4 * dstStride], (src4 + src5) * 20 - (src3 + src6) * 6 + (src2 + src7) * 3 - (src1 + src8));        \
			OP(dst[5 * dstStride], (src5 + src6) * 20 - (src4 + src7) * 6 + (src3 + src8) * 3 - (src2 + src8));        \
			OP(dst[6 * dstStride], (src6 + src7) * 20 - (src5 + src8) * 6 + (src4 + src8) * 3 - (src3 + src7));        \
			OP(dst[7 * dstStride], (src7 + src8) * 20 - (src6 + src8) * 6 + (src5 + src7) * 3 - (src4 + src6));        \
			dst++;                                                                                                     \
			src++;                                                                                                     \
		}                                                                                                              \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##mpeg4_qpel16_h_lowpass(uint8 *dst, const uint8 *src, int dstStride, int srcStride, int h) {    \
		const Crop cm;                                                                                                 \
		int i;                                                                                                         \
                                                                                                                       \
		for (i = 0; i < h; i++) {                                                                                      \
			OP(dst[0], (src[0] + src[1]) * 20 - (src[0] + src[2]) * 6 + (src[1] + src[3]) * 3 - (src[2] + src[4]));    \
			OP(dst[1], (src[1] + src[2]) * 20 - (src[0] + src[3]) * 6 + (src[0] + src[4]) * 3 - (src[1] + src[5]));    \
			OP(dst[2], (src[2] + src[3]) * 20 - (src[1] + src[4]) * 6 + (src[0] + src[5]) * 3 - (src[0] + src[6]));    \
			OP(dst[3], (src[3] + src[4]) * 20 - (src[2] + src[5]) * 6 + (src[1] + src[6]) * 3 - (src[0] + src[7]));    \
			OP(dst[4], (src[4] + src[5]) * 20 - (src[3] + src[6]) * 6 + (src[2] + src[7]) * 3 - (src[1] + src[8]));    \
			OP(dst[5], (src[5] + src[6]) * 20 - (src[4] + src[7]) * 6 + (src[3] + src[8]) * 3 - (src[2] + src[9]));    \
			OP(dst[6], (src[6] + src[7]) * 20 - (src[5] + src[8]) * 6 + (src[4] + src[9]) * 3 - (src[3] + src[10]));   \
			OP(dst[7], (src[7] + src[8]) * 20 - (src[6] + src[9]) * 6 + (src[5] + src[10]) * 3 - (src[4] + src[11]));  \
			OP(dst[8], (src[8] + src[9]) * 20 - (src[7] + src[10]) * 6 + (src[6] + src[11]) * 3 - (src[5] + src[12])); \
			OP(dst[9],                                                                                                 \
			   (src[9] + src[10]) * 20 - (src[8] + src[11]) * 6 + (src[7] + src[12]) * 3 - (src[6] + src[13]));        \
			OP(dst[10],                                                                                                \
			   (src[10] + src[11]) * 20 - (src[9] + src[12]) * 6 + (src[8] + src[13]) * 3 - (src[7] + src[14]));       \
			OP(dst[11],                                                                                                \
			   (src[11] + src[12]) * 20 - (src[10] + src[13]) * 6 + (src[9] + src[14]) * 3 - (src[8] + src[15]));      \
			OP(dst[12],                                                                                                \
			   (src[12] + src[13]) * 20 - (src[11] + src[14]) * 6 + (src[10] + src[15]) * 3 - (src[9] + src[16]));     \
			OP(dst[13],                                                                                                \
			   (src[13] + src[14]) * 20 - (src[12] + src[15]) * 6 + (src[11] + src[16]) * 3 - (src[10] + src[16]));    \
			OP(dst[14],                                                                                                \
			   (src[14] + src[15]) * 20 - (src[13] + src[16]) * 6 + (src[12] + src[16]) * 3 - (src[11] + src[15]));    \
			OP(dst[15],                                                                                                \
			   (src[15] + src[16]) * 20 - (src[14] + src[16]) * 6 + (src[13] + src[15]) * 3 - (src[12] + src[14]));    \
			dst += dstStride;                                                                                          \
			src += srcStride;                                                                                          \
		}                                                                                                              \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##mpeg4_qpel16_v_lowpass(uint8 *dst, const uint8 *src, int dstStride, int srcStride) {           \
		const Crop cm;                                                                                                 \
		const int w = 16;                                                                                              \
		int i;                                                                                                         \
                                                                                                                       \
		for (i = 0; i < w; i++) {                                                                                      \
			const int src0 = src[0 * srcStride];                                                                       \
			const int src1 = src[1 * srcStride];                                                                       \
			const int src2 = src[2 * srcStride];                                                                       \
			const int src3 = src[3 * srcStride];                                                                       \
			const int src4 = src[4 * srcStride];                                                                       \
			const int src5 = src[5 * srcStride];                                                                       \
			const int src6 = src[6 * srcStride];                                                                       \
			const int src7 = src[7 * srcStride];                                                                       \
			const int src8 = src[8 * srcStride];                                                                       \
			const int src9 = src[9 * srcStride];                                                                       \
			const int src10 = src[10 * srcStride];                                                                     \
			const int src11 = src[11 * srcStride];                                                                     \
			const int src12 = src[12 * srcStride];                                                                     \
			const int src13 = src[13 * srcStride];                                                                     \
			const int src14 = src[14 * srcStride];                                                                     \
			const int src15 = src[15 * srcStride];                                                                     \
			const int src16 = src[16 * srcStride];                                                                     \
			OP(dst[0 * dstStride], (src0 + src1) * 20 - (src0 + src2) * 6 + (src1 + src3) * 3 - (src2 + src4));        \
			OP(dst[1 * dstStride], (src1 + src2) * 20 - (src0 + src3) * 6 + (src0 + src4) * 3 - (src1 + src5));        \
			OP(dst[2 * dstStride], (src2 + src3) * 20 - (src1 + src4) * 6 + (src0 + src5) * 3 - (src0 + src6));        \
			OP(dst[3 * dstStride], (src3 + src4) * 20 - (src2 + src5) * 6 + (src1 + src6) * 3 - (src0 + src7));        \
			OP(dst[4 * dstStride], (src4 + src5) * 20 - (src3 + src6) * 6 + (src2 + src7) * 3 - (src1 + src8));        \
			OP(dst[5 * dstStride], (src5 + src6) * 20 - (src4 + src7) * 6 + (src3 + src8) * 3 - (src2 + src9));        \
			OP(dst[6 * dstStride], (src6 + src7) * 20 - (src5 + src8) * 6 + (src4 + src9) * 3 - (src3 + src10));       \
			OP(dst[7 * dstStride], (src7 + src8) * 20 - (src6 + src9) * 6 + (src5 + src10) * 3 - (src4 + src11));      \
			OP(dst[8 * dstStride], (src8 + src9) * 20 - (src7 + src10) * 6 + (src6 + src11) * 3 - (src5 + src12));     \
			OP(dst[9 * dstStride], (src9 + src10) * 20 - (src8 + src11) * 6 + (src7 + src12) * 3 - (src6 + src13));    \
			OP(dst[10 * dstStride], (src10 + src11) * 20 - (src9 + src12) * 6 + (src8 + src13) * 3 - (src7 + src14));  \
			OP(dst[11 * dstStride], (src11 + src12) * 20 - (src10 + src13) * 6 + (src9 + src14) * 3 - (src8 + src15)); \
			OP(dst[12 * dstStride],                                                                                    \
			   (src12 + src13) * 20 - (src11 + src14) * 6 + (src10 + src15) * 3 - (src9 + src16));                     \
			OP(dst[13 * dstStride],                                                                                    \
			   (src13 + src14) * 20 - (src12 + src15) * 6 + (src11 + src16) * 3 - (src10 + src16));                    \
			OP(dst[14 * dstStride],                                                                                    \
			   (src14 + src15) * 20 - (src13 + src16) * 6 + (src12 + src16) * 3 - (src11 + src15));                    \
			OP(dst[15 * dstStride],                                                                                    \
			   (src15 + src16) * 20 - (src14 + src16) * 6 + (src13 + src15) * 3 - (src12 + src14));                    \
			dst++;                                                                                                     \
			src++;                                                                                                     \
		}                                                                                                              \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel8_mc10_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                 \
		uint8 half[64];                                                                                                \
                                                                                                                       \
		put##RND##mpeg4_qpel8_h_lowpass(half, src, 8, stride, 8);                                                      \
		OPNAME##pixels8_l2_8(dst, src, half, stride, stride, 8, 8);                                                    \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel8_mc20_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                 \
		OPNAME##mpeg4_qpel8_h_lowpass(dst, src, stride, stride, 8);                                                    \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel8_mc30_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                 \
		uint8 half[64];                                                                                                \
                                                                                                                       \
		put##RND##mpeg4_qpel8_h_lowpass(half, src, 8, stride, 8);                                                      \
		OPNAME##pixels8_l2_8(dst, src + 1, half, stride, stride, 8, 8);                                                \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel8_mc01_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                 \
		uint8 full[16 * 9];                                                                                            \
		uint8 half[64];                                                                                                \
                                                                                                                       \
		copy_block8(full, src, 16, stride, 9);                                                                         \
		put##RND##mpeg4_qpel8_v_lowpass(half, full, 8, 16);                                                            \
		OPNAME##pixels8_l2_8(dst, full, half, stride, 16, 8, 8);                                                       \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel8_mc02_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                 \
		uint8 full[16 * 9];                                                                                            \
                                                                                                                       \
		copy_block8(full, src, 16, stride, 9);                                                                         \
		OPNAME##mpeg4_qpel8_v_lowpass(dst, full, stride, 16);                                                          \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel8_mc03_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                 \
		uint8 full[16 * 9];                                                                                            \
		uint8 half[64];                                                                                                \
                                                                                                                       \
		copy_block8(full, src, 16, stride, 9);                                                                         \
		put##RND##mpeg4_qpel8_v_lowpass(half, full, 8, 16);                                                            \
		OPNAME##pixels8_l2_8(dst, full + 16, half, stride, 16, 8, 8);                                                  \
	}                                                                                                                  \
                                                                                                                       \
	void ff_##OPNAME##qpel8_mc11_old_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                               \
		uint8 full[16 * 9];                                                                                            \
		uint8 halfH[72];                                                                                               \
		uint8 halfV[64];                                                                                               \
		uint8 halfHV[64];                                                                                              \
                                                                                                                       \
		copy_block9(full, src, 16, stride, 9);                                                                         \
		put##RND##mpeg4_qpel8_h_lowpass(halfH, full, 8, 16, 9);                                                        \
		put##RND##mpeg4_qpel8_v_lowpass(halfV, full, 8, 16);                                                           \
		put##RND##mpeg4_qpel8_v_lowpass(halfHV, halfH, 8, 8);                                                          \
		OPNAME##pixels8_l4_8(dst, full, halfH, halfV, halfHV, stride, 16, 8, 8, 8, 8);                                 \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel8_mc11_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                 \
		uint8 full[16 * 9];                                                                                            \
		uint8 halfH[72];                                                                                               \
		uint8 halfHV[64];                                                                                              \
                                                                                                                       \
		copy_block9(full, src, 16, stride, 9);                                                                         \
		put##RND##mpeg4_qpel8_h_lowpass(halfH, full, 8, 16, 9);                                                        \
		put##RND##pixels8_l2_8(halfH, halfH, full, 8, 8, 16, 9);                                                       \
		put##RND##mpeg4_qpel8_v_lowpass(halfHV, halfH, 8, 8);                                                          \
		OPNAME##pixels8_l2_8(dst, halfH, halfHV, stride, 8, 8, 8);                                                     \
	}                                                                                                                  \
                                                                                                                       \
	void ff_##OPNAME##qpel8_mc31_old_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                               \
		uint8 full[16 * 9];                                                                                            \
		uint8 halfH[72];                                                                                               \
		uint8 halfV[64];                                                                                               \
		uint8 halfHV[64];                                                                                              \
                                                                                                                       \
		copy_block9(full, src, 16, stride, 9);                                                                         \
		put##RND##mpeg4_qpel8_h_lowpass(halfH, full, 8, 16, 9);                                                        \
		put##RND##mpeg4_qpel8_v_lowpass(halfV, full + 1, 8, 16);                                                       \
		put##RND##mpeg4_qpel8_v_lowpass(halfHV, halfH, 8, 8);                                                          \
		OPNAME##pixels8_l4_8(dst, full + 1, halfH, halfV, halfHV, stride, 16, 8, 8, 8, 8);                             \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel8_mc31_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                 \
		uint8 full[16 * 9];                                                                                            \
		uint8 halfH[72];                                                                                               \
		uint8 halfHV[64];                                                                                              \
                                                                                                                       \
		copy_block9(full, src, 16, stride, 9);                                                                         \
		put##RND##mpeg4_qpel8_h_lowpass(halfH, full, 8, 16, 9);                                                        \
		put##RND##pixels8_l2_8(halfH, halfH, full + 1, 8, 8, 16, 9);                                                   \
		put##RND##mpeg4_qpel8_v_lowpass(halfHV, halfH, 8, 8);                                                          \
		OPNAME##pixels8_l2_8(dst, halfH, halfHV, stride, 8, 8, 8);                                                     \
	}                                                                                                                  \
                                                                                                                       \
	void ff_##OPNAME##qpel8_mc13_old_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                               \
		uint8 full[16 * 9];                                                                                            \
		uint8 halfH[72];                                                                                               \
		uint8 halfV[64];                                                                                               \
		uint8 halfHV[64];                                                                                              \
                                                                                                                       \
		copy_block9(full, src, 16, stride, 9);                                                                         \
		put##RND##mpeg4_qpel8_h_lowpass(halfH, full, 8, 16, 9);                                                        \
		put##RND##mpeg4_qpel8_v_lowpass(halfV, full, 8, 16);                                                           \
		put##RND##mpeg4_qpel8_v_lowpass(halfHV, halfH, 8, 8);                                                          \
		OPNAME##pixels8_l4_8(dst, full + 16, halfH + 8, halfV, halfHV, stride, 16, 8, 8, 8, 8);                        \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel8_mc13_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                 \
		uint8 full[16 * 9];                                                                                            \
		uint8 halfH[72];                                                                                               \
		uint8 halfHV[64];                                                                                              \
                                                                                                                       \
		copy_block9(full, src, 16, stride, 9);                                                                         \
		put##RND##mpeg4_qpel8_h_lowpass(halfH, full, 8, 16, 9);                                                        \
		put##RND##pixels8_l2_8(halfH, halfH, full, 8, 8, 16, 9);                                                       \
		put##RND##mpeg4_qpel8_v_lowpass(halfHV, halfH, 8, 8);                                                          \
		OPNAME##pixels8_l2_8(dst, halfH + 8, halfHV, stride, 8, 8, 8);                                                 \
	}                                                                                                                  \
                                                                                                                       \
	void ff_##OPNAME##qpel8_mc33_old_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                               \
		uint8 full[16 * 9];                                                                                            \
		uint8 halfH[72];                                                                                               \
		uint8 halfV[64];                                                                                               \
		uint8 halfHV[64];                                                                                              \
                                                                                                                       \
		copy_block9(full, src, 16, stride, 9);                                                                         \
		put##RND##mpeg4_qpel8_h_lowpass(halfH, full, 8, 16, 9);                                                        \
		put##RND##mpeg4_qpel8_v_lowpass(halfV, full + 1, 8, 16);                                                       \
		put##RND##mpeg4_qpel8_v_lowpass(halfHV, halfH, 8, 8);                                                          \
		OPNAME##pixels8_l4_8(dst, full + 17, halfH + 8, halfV, halfHV, stride, 16, 8, 8, 8, 8);                        \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel8_mc33_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                 \
		uint8 full[16 * 9];                                                                                            \
		uint8 halfH[72];                                                                                               \
		uint8 halfHV[64];                                                                                              \
                                                                                                                       \
		copy_block9(full, src, 16, stride, 9);                                                                         \
		put##RND##mpeg4_qpel8_h_lowpass(halfH, full, 8, 16, 9);                                                        \
		put##RND##pixels8_l2_8(halfH, halfH, full + 1, 8, 8, 16, 9);                                                   \
		put##RND##mpeg4_qpel8_v_lowpass(halfHV, halfH, 8, 8);                                                          \
		OPNAME##pixels8_l2_8(dst, halfH + 8, halfHV, stride, 8, 8, 8);                                                 \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel8_mc21_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                 \
		uint8 halfH[72];                                                                                               \
		uint8 halfHV[64];                                                                                              \
                                                                                                                       \
		put##RND##mpeg4_qpel8_h_lowpass(halfH, src, 8, stride, 9);                                                     \
		put##RND##mpeg4_qpel8_v_lowpass(halfHV, halfH, 8, 8);                                                          \
		OPNAME##pixels8_l2_8(dst, halfH, halfHV, stride, 8, 8, 8);                                                     \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel8_mc23_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                 \
		uint8 halfH[72];                                                                                               \
		uint8 halfHV[64];                                                                                              \
                                                                                                                       \
		put##RND##mpeg4_qpel8_h_lowpass(halfH, src, 8, stride, 9);                                                     \
		put##RND##mpeg4_qpel8_v_lowpass(halfHV, halfH, 8, 8);                                                          \
		OPNAME##pixels8_l2_8(dst, halfH + 8, halfHV, stride, 8, 8, 8);                                                 \
	}                                                                                                                  \
                                                                                                                       \
	void ff_##OPNAME##qpel8_mc12_old_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                               \
		uint8 full[16 * 9];                                                                                            \
		uint8 halfH[72];                                                                                               \
		uint8 halfV[64];                                                                                               \
		uint8 halfHV[64];                                                                                              \
                                                                                                                       \
		copy_block9(full, src, 16, stride, 9);                                                                         \
		put##RND##mpeg4_qpel8_h_lowpass(halfH, full, 8, 16, 9);                                                        \
		put##RND##mpeg4_qpel8_v_lowpass(halfV, full, 8, 16);                                                           \
		put##RND##mpeg4_qpel8_v_lowpass(halfHV, halfH, 8, 8);                                                          \
		OPNAME##pixels8_l2_8(dst, halfV, halfHV, stride, 8, 8, 8);                                                     \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel8_mc12_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                 \
		uint8 full[16 * 9];                                                                                            \
		uint8 halfH[72];                                                                                               \
                                                                                                                       \
		copy_block9(full, src, 16, stride, 9);                                                                         \
		put##RND##mpeg4_qpel8_h_lowpass(halfH, full, 8, 16, 9);                                                        \
		put##RND##pixels8_l2_8(halfH, halfH, full, 8, 8, 16, 9);                                                       \
		OPNAME##mpeg4_qpel8_v_lowpass(dst, halfH, stride, 8);                                                          \
	}                                                                                                                  \
                                                                                                                       \
	void ff_##OPNAME##qpel8_mc32_old_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                               \
		uint8 full[16 * 9];                                                                                            \
		uint8 halfH[72];                                                                                               \
		uint8 halfV[64];                                                                                               \
		uint8 halfHV[64];                                                                                              \
                                                                                                                       \
		copy_block9(full, src, 16, stride, 9);                                                                         \
		put##RND##mpeg4_qpel8_h_lowpass(halfH, full, 8, 16, 9);                                                        \
		put##RND##mpeg4_qpel8_v_lowpass(halfV, full + 1, 8, 16);                                                       \
		put##RND##mpeg4_qpel8_v_lowpass(halfHV, halfH, 8, 8);                                                          \
		OPNAME##pixels8_l2_8(dst, halfV, halfHV, stride, 8, 8, 8);                                                     \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel8_mc32_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                 \
		uint8 full[16 * 9];                                                                                            \
		uint8 halfH[72];                                                                                               \
                                                                                                                       \
		copy_block9(full, src, 16, stride, 9);                                                                         \
		put##RND##mpeg4_qpel8_h_lowpass(halfH, full, 8, 16, 9);                                                        \
		put##RND##pixels8_l2_8(halfH, halfH, full + 1, 8, 8, 16, 9);                                                   \
		OPNAME##mpeg4_qpel8_v_lowpass(dst, halfH, stride, 8);                                                          \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel8_mc22_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                 \
		uint8 halfH[72];                                                                                               \
                                                                                                                       \
		put##RND##mpeg4_qpel8_h_lowpass(halfH, src, 8, stride, 9);                                                     \
		OPNAME##mpeg4_qpel8_v_lowpass(dst, halfH, stride, 8);                                                          \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel16_mc10_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                \
		uint8 half[256];                                                                                               \
                                                                                                                       \
		put##RND##mpeg4_qpel16_h_lowpass(half, src, 16, stride, 16);                                                   \
		OPNAME##pixels16_l2_8(dst, src, half, stride, stride, 16, 16);                                                 \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel16_mc20_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                \
		OPNAME##mpeg4_qpel16_h_lowpass(dst, src, stride, stride, 16);                                                  \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel16_mc30_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                \
		uint8 half[256];                                                                                               \
                                                                                                                       \
		put##RND##mpeg4_qpel16_h_lowpass(half, src, 16, stride, 16);                                                   \
		OPNAME##pixels16_l2_8(dst, src + 1, half, stride, stride, 16, 16);                                             \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel16_mc01_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                \
		uint8 full[24 * 17];                                                                                           \
		uint8 half[256];                                                                                               \
                                                                                                                       \
		copy_block16(full, src, 24, stride, 17);                                                                       \
		put##RND##mpeg4_qpel16_v_lowpass(half, full, 16, 24);                                                          \
		OPNAME##pixels16_l2_8(dst, full, half, stride, 24, 16, 16);                                                    \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel16_mc02_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                \
		uint8 full[24 * 17];                                                                                           \
                                                                                                                       \
		copy_block16(full, src, 24, stride, 17);                                                                       \
		OPNAME##mpeg4_qpel16_v_lowpass(dst, full, stride, 24);                                                         \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel16_mc03_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                \
		uint8 full[24 * 17];                                                                                           \
		uint8 half[256];                                                                                               \
                                                                                                                       \
		copy_block16(full, src, 24, stride, 17);                                                                       \
		put##RND##mpeg4_qpel16_v_lowpass(half, full, 16, 24);                                                          \
		OPNAME##pixels16_l2_8(dst, full + 24, half, stride, 24, 16, 16);                                               \
	}                                                                                                                  \
                                                                                                                       \
	void ff_##OPNAME##qpel16_mc11_old_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                              \
		uint8 full[24 * 17];                                                                                           \
		uint8 halfH[272];                                                                                              \
		uint8 halfV[256];                                                                                              \
		uint8 halfHV[256];                                                                                             \
                                                                                                                       \
		copy_block17(full, src, 24, stride, 17);                                                                       \
		put##RND##mpeg4_qpel16_h_lowpass(halfH, full, 16, 24, 17);                                                     \
		put##RND##mpeg4_qpel16_v_lowpass(halfV, full, 16, 24);                                                         \
		put##RND##mpeg4_qpel16_v_lowpass(halfHV, halfH, 16, 16);                                                       \
		OPNAME##pixels16_l4_8(dst, full, halfH, halfV, halfHV, stride, 24, 16, 16, 16, 16);                            \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel16_mc11_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                \
		uint8 full[24 * 17];                                                                                           \
		uint8 halfH[272];                                                                                              \
		uint8 halfHV[256];                                                                                             \
                                                                                                                       \
		copy_block17(full, src, 24, stride, 17);                                                                       \
		put##RND##mpeg4_qpel16_h_lowpass(halfH, full, 16, 24, 17);                                                     \
		put##RND##pixels16_l2_8(halfH, halfH, full, 16, 16, 24, 17);                                                   \
		put##RND##mpeg4_qpel16_v_lowpass(halfHV, halfH, 16, 16);                                                       \
		OPNAME##pixels16_l2_8(dst, halfH, halfHV, stride, 16, 16, 16);                                                 \
	}                                                                                                                  \
                                                                                                                       \
	void ff_##OPNAME##qpel16_mc31_old_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                              \
		uint8 full[24 * 17];                                                                                           \
		uint8 halfH[272];                                                                                              \
		uint8 halfV[256];                                                                                              \
		uint8 halfHV[256];                                                                                             \
                                                                                                                       \
		copy_block17(full, src, 24, stride, 17);                                                                       \
		put##RND##mpeg4_qpel16_h_lowpass(halfH, full, 16, 24, 17);                                                     \
		put##RND##mpeg4_qpel16_v_lowpass(halfV, full + 1, 16, 24);                                                     \
		put##RND##mpeg4_qpel16_v_lowpass(halfHV, halfH, 16, 16);                                                       \
		OPNAME##pixels16_l4_8(dst, full + 1, halfH, halfV, halfHV, stride, 24, 16, 16, 16, 16);                        \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel16_mc31_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                \
		uint8 full[24 * 17];                                                                                           \
		uint8 halfH[272];                                                                                              \
		uint8 halfHV[256];                                                                                             \
                                                                                                                       \
		copy_block17(full, src, 24, stride, 17);                                                                       \
		put##RND##mpeg4_qpel16_h_lowpass(halfH, full, 16, 24, 17);                                                     \
		put##RND##pixels16_l2_8(halfH, halfH, full + 1, 16, 16, 24, 17);                                               \
		put##RND##mpeg4_qpel16_v_lowpass(halfHV, halfH, 16, 16);                                                       \
		OPNAME##pixels16_l2_8(dst, halfH, halfHV, stride, 16, 16, 16);                                                 \
	}                                                                                                                  \
                                                                                                                       \
	void ff_##OPNAME##qpel16_mc13_old_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                              \
		uint8 full[24 * 17];                                                                                           \
		uint8 halfH[272];                                                                                              \
		uint8 halfV[256];                                                                                              \
		uint8 halfHV[256];                                                                                             \
                                                                                                                       \
		copy_block17(full, src, 24, stride, 17);                                                                       \
		put##RND##mpeg4_qpel16_h_lowpass(halfH, full, 16, 24, 17);                                                     \
		put##RND##mpeg4_qpel16_v_lowpass(halfV, full, 16, 24);                                                         \
		put##RND##mpeg4_qpel16_v_lowpass(halfHV, halfH, 16, 16);                                                       \
		OPNAME##pixels16_l4_8(dst, full + 24, halfH + 16, halfV, halfHV, stride, 24, 16, 16, 16, 16);                  \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel16_mc13_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                \
		uint8 full[24 * 17];                                                                                           \
		uint8 halfH[272];                                                                                              \
		uint8 halfHV[256];                                                                                             \
                                                                                                                       \
		copy_block17(full, src, 24, stride, 17);                                                                       \
		put##RND##mpeg4_qpel16_h_lowpass(halfH, full, 16, 24, 17);                                                     \
		put##RND##pixels16_l2_8(halfH, halfH, full, 16, 16, 24, 17);                                                   \
		put##RND##mpeg4_qpel16_v_lowpass(halfHV, halfH, 16, 16);                                                       \
		OPNAME##pixels16_l2_8(dst, halfH + 16, halfHV, stride, 16, 16, 16);                                            \
	}                                                                                                                  \
                                                                                                                       \
	void ff_##OPNAME##qpel16_mc33_old_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                              \
		uint8 full[24 * 17];                                                                                           \
		uint8 halfH[272];                                                                                              \
		uint8 halfV[256];                                                                                              \
		uint8 halfHV[256];                                                                                             \
                                                                                                                       \
		copy_block17(full, src, 24, stride, 17);                                                                       \
		put##RND##mpeg4_qpel16_h_lowpass(halfH, full, 16, 24, 17);                                                     \
		put##RND##mpeg4_qpel16_v_lowpass(halfV, full + 1, 16, 24);                                                     \
		put##RND##mpeg4_qpel16_v_lowpass(halfHV, halfH, 16, 16);                                                       \
		OPNAME##pixels16_l4_8(dst, full + 25, halfH + 16, halfV, halfHV, stride, 24, 16, 16, 16, 16);                  \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel16_mc33_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                \
		uint8 full[24 * 17];                                                                                           \
		uint8 halfH[272];                                                                                              \
		uint8 halfHV[256];                                                                                             \
                                                                                                                       \
		copy_block17(full, src, 24, stride, 17);                                                                       \
		put##RND##mpeg4_qpel16_h_lowpass(halfH, full, 16, 24, 17);                                                     \
		put##RND##pixels16_l2_8(halfH, halfH, full + 1, 16, 16, 24, 17);                                               \
		put##RND##mpeg4_qpel16_v_lowpass(halfHV, halfH, 16, 16);                                                       \
		OPNAME##pixels16_l2_8(dst, halfH + 16, halfHV, stride, 16, 16, 16);                                            \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel16_mc21_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                \
		uint8 halfH[272];                                                                                              \
		uint8 halfHV[256];                                                                                             \
                                                                                                                       \
		put##RND##mpeg4_qpel16_h_lowpass(halfH, src, 16, stride, 17);                                                  \
		put##RND##mpeg4_qpel16_v_lowpass(halfHV, halfH, 16, 16);                                                       \
		OPNAME##pixels16_l2_8(dst, halfH, halfHV, stride, 16, 16, 16);                                                 \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel16_mc23_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                \
		uint8 halfH[272];                                                                                              \
		uint8 halfHV[256];                                                                                             \
                                                                                                                       \
		put##RND##mpeg4_qpel16_h_lowpass(halfH, src, 16, stride, 17);                                                  \
		put##RND##mpeg4_qpel16_v_lowpass(halfHV, halfH, 16, 16);                                                       \
		OPNAME##pixels16_l2_8(dst, halfH + 16, halfHV, stride, 16, 16, 16);                                            \
	}                                                                                                                  \
                                                                                                                       \
	void ff_##OPNAME##qpel16_mc12_old_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                              \
		uint8 full[24 * 17];                                                                                           \
		uint8 halfH[272];                                                                                              \
		uint8 halfV[256];                                                                                              \
		uint8 halfHV[256];                                                                                             \
                                                                                                                       \
		copy_block17(full, src, 24, stride, 17);                                                                       \
		put##RND##mpeg4_qpel16_h_lowpass(halfH, full, 16, 24, 17);                                                     \
		put##RND##mpeg4_qpel16_v_lowpass(halfV, full, 16, 24);                                                         \
		put##RND##mpeg4_qpel16_v_lowpass(halfHV, halfH, 16, 16);                                                       \
		OPNAME##pixels16_l2_8(dst, halfV, halfHV, stride, 16, 16, 16);                                                 \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel16_mc12_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                \
		uint8 full[24 * 17];                                                                                           \
		uint8 halfH[272];                                                                                              \
                                                                                                                       \
		copy_block17(full, src, 24, stride, 17);                                                                       \
		put##RND##mpeg4_qpel16_h_lowpass(halfH, full, 16, 24, 17);                                                     \
		put##RND##pixels16_l2_8(halfH, halfH, full, 16, 16, 24, 17);                                                   \
		OPNAME##mpeg4_qpel16_v_lowpass(dst, halfH, stride, 16);                                                        \
	}                                                                                                                  \
                                                                                                                       \
	void ff_##OPNAME##qpel16_mc32_old_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                              \
		uint8 full[24 * 17];                                                                                           \
		uint8 halfH[272];                                                                                              \
		uint8 halfV[256];                                                                                              \
		uint8 halfHV[256];                                                                                             \
                                                                                                                       \
		copy_block17(full, src, 24, stride, 17);                                                                       \
		put##RND##mpeg4_qpel16_h_lowpass(halfH, full, 16, 24, 17);                                                     \
		put##RND##mpeg4_qpel16_v_lowpass(halfV, full + 1, 16, 24);                                                     \
		put##RND##mpeg4_qpel16_v_lowpass(halfHV, halfH, 16, 16);                                                       \
		OPNAME##pixels16_l2_8(dst, halfV, halfHV, stride, 16, 16, 16);                                                 \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel16_mc32_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                \
		uint8 full[24 * 17];                                                                                           \
		uint8 halfH[272];                                                                                              \
                                                                                                                       \
		copy_block17(full, src, 24, stride, 17);                                                                       \
		put##RND##mpeg4_qpel16_h_lowpass(halfH, full, 16, 24, 17);                                                     \
		put##RND##pixels16_l2_8(halfH, halfH, full + 1, 16, 16, 24, 17);                                               \
		OPNAME##mpeg4_qpel16_v_lowpass(dst, halfH, stride, 16);                                                        \
	}                                                                                                                  \
                                                                                                                       \
	static void OPNAME##qpel16_mc22_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {                                \
		uint8 halfH[272];                                                                                              \
                                                                                                                       \
		put##RND##mpeg4_qpel16_h_lowpass(halfH, src, 16, stride, 17);                                                  \
		OPNAME##mpeg4_qpel16_v_lowpass(dst, halfH, stride, 16);                                                        \
	}

#define op_avg(a, b) a = (((a) + cm[((b) + 16) >> 5] + 1) >> 1)
#define op_put(a, b) a = cm[((b) + 16) >> 5]
#define op_put_no_rnd(a, b) a = cm[((b) + 15) >> 5]

QPEL_MC(0, put_, _, op_put)
QPEL_MC(1, put_no_rnd_, _no_rnd_, op_put_no_rnd)
QPEL_MC(0, avg_, _, op_avg)

#undef op_avg
#undef op_put
#undef op_put_no_rnd

void ff_put_pixels8x8_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {
	put_pixels8_8_c(dst, src, stride, 8);
}

void ff_avg_pixels8x8_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {
	avg_pixels8_8_c(dst, src, stride, 8);
}

void ff_put_pixels16x16_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {
	put_pixels16_8_c(dst, src, stride, 16);
}

void ff_avg_pixels16x16_c(uint8 *dst, const uint8 *src, ptrdiff_t stride) {
	avg_pixels16_8_c(dst, src, stride, 16);
}

#define put_qpel8_mc00_c ff_put_pixels8x8_c
#define avg_qpel8_mc00_c ff_avg_pixels8x8_c
#define put_qpel16_mc00_c ff_put_pixels16x16_c
#define avg_qpel16_mc00_c ff_avg_pixels16x16_c
#define put_no_rnd_qpel8_mc00_c ff_put_pixels8x8_c
#define put_no_rnd_qpel16_mc00_c ff_put_pixels16x16_c

void initQpel(QpelDSP *c) {
#define dspfunc(PFX, IDX, NUM)                                                                                         \
	c->PFX##_pixels_tab[IDX][0] = PFX##NUM##_mc00_c;                                                                   \
	c->PFX##_pixels_tab[IDX][1] = PFX##NUM##_mc10_c;                                                                   \
	c->PFX##_pixels_tab[IDX][2] = PFX##NUM##_mc20_c;                                                                   \
	c->PFX##_pixels_tab[IDX][3] = PFX##NUM##_mc30_c;                                                                   \
	c->PFX##_pixels_tab[IDX][4] = PFX##NUM##_mc01_c;                                                                   \
	c->PFX##_pixels_tab[IDX][5] = PFX##NUM##_mc11_c;                                                                   \
	c->PFX##_pixels_tab[IDX][6] = PFX##NUM##_mc21_c;                                                                   \
	c->PFX##_pixels_tab[IDX][7] = PFX##NUM##_mc31_c;                                                                   \
	c->PFX##_pixels_tab[IDX][8] = PFX##NUM##_mc02_c;                                                                   \
	c->PFX##_pixels_tab[IDX][9] = PFX##NUM##_mc12_c;                                                                   \
	c->PFX##_pixels_tab[IDX][10] = PFX##NUM##_mc22_c;                                                                  \
	c->PFX##_pixels_tab[IDX][11] = PFX##NUM##_mc32_c;                                                                  \
	c->PFX##_pixels_tab[IDX][12] = PFX##NUM##_mc03_c;                                                                  \
	c->PFX##_pixels_tab[IDX][13] = PFX##NUM##_mc13_c;                                                                  \
	c->PFX##_pixels_tab[IDX][14] = PFX##NUM##_mc23_c;                                                                  \
	c->PFX##_pixels_tab[IDX][15] = PFX##NUM##_mc33_c

	dspfunc(put_qpel, 0, 16);
	dspfunc(put_qpel, 1, 8);

	dspfunc(put_no_rnd_qpel, 0, 16);
	dspfunc(put_no_rnd_qpel, 1, 8);

	dspfunc(avg_qpel, 0, 16);
	dspfunc(avg_qpel, 1, 8);
}

#undef QPEL_MC
#undef avg_qpel16_mc00_c
#undef avg_qpel8_mc00_c
#undef dspfunc
#undef put_no_rnd_qpel16_mc00_c
#undef put_no_rnd_qpel8_mc00_c
#undef put_qpel16_mc00_c
#undef put_qpel8_mc00_c

/*
 * area positions, #3 is 1 pixel only, other are 8 pixels
 *    |66666666|
 *   3|44444444|55555555|
 * - -+--------+--------+
 * 1 2|XXXXXXXX|
 * 1 2|XXXXXXXX|
 * 1 2|XXXXXXXX|
 * 1 2|XXXXXXXX|
 * 1 2|XXXXXXXX|
 * 1 2|XXXXXXXX|
 * 1 2|XXXXXXXX|
 * 1 2|XXXXXXXX|
 * ^-start
 */

#define area1 (0)
#define area2 (8)
#define area3 (8 + 8)
#define area4 (8 + 8 + 1)
#define area5 (8 + 8 + 1 + 8)
#define area6 (8 + 8 + 1 + 16)

/**
 Collect statistics and prepare the edge pixels required by the other spatial compensation functions.

 * @param src pointer to the beginning of the processed block
 * @param dst pointer to emu_edge, edge pixels are stored the way other compensation routines do.
 * @param linesize byte offset between 2 vertical pixels in the source image
 * @param range pointer to the variable where the edge pixel range is to be stored (max-min values)
 * @param psum  pointer to the variable where the edge pixel sum is to be stored
 * @param edges Informs this routine that the block is on an image border, so it has to interpolate the missing edge
pixels. and some of the edge pixels should be interpolated, the flag has the following meaning: 1   - mb_x==0 - first
block in the row, interpolate area #1,#2,#3; 2   - mb_y==0 - first row, interpolate area #3,#4,#5,#6; note:   1|2 -
mb_x==mb_y==0 - first block, use 0x80 value for all areas; 4   - mb_x>= (mb_width-1) last block in the row, interpolate
area #5;
-*/
static void x8_setup_spatial_compensation(const uint8 *src, uint8 *dst, ptrdiff_t stride, int *range, int *psum,
										  int edges) {
	const uint8 *ptr;
	int sum;
	int i;
	int min_pix, max_pix;
	uint8 c;

	if ((edges & 3) == 3) {
		*psum = 0x80 * (8 + 1 + 8 + 2);
		*range = 0;
		memset(dst, 0x80, 16 + 1 + 16 + 8);
		/* this triggers flat_dc for sure. flat_dc avoids all (other)
		 * prediction modes, but requires dc_level decoding. */
		return;
	}

	min_pix = 256;
	max_pix = -1;

	sum = 0;

	if (!(edges & 1)) { // (mb_x != 0) // there is previous block on this row
		ptr = src - 1;  // left column, area 2
		for (i = 7; i >= 0; i--) {
			c = *(ptr - 1); // area1, same mb as area2, no need to check
			dst[area1 + i] = c;
			c = *ptr;

			sum += c;
			min_pix = MIN(min_pix, int(c));
			max_pix = MAX(max_pix, int(c));
			dst[area2 + i] = c;

			ptr += stride;
		}
	}

	if (!(edges & 2)) {     // (mb_y != 0) // there is row above
		ptr = src - stride; // top line
		for (i = 0; i < 8; i++) {
			c = *(ptr + i);
			sum += c;
			min_pix = MIN(min_pix, int(c));
			max_pix = MAX(max_pix, int(c));
		}
		if (edges & 4) {               // last block on the row?
			memset(dst + area5, c, 8); // set with last pixel fr
			memcpy(dst + area4, ptr, 8);
		} else {
			memcpy(dst + area4, ptr, 16); // both area4 and 5
		}
		// area6 always present in the above block
		memcpy(dst + area6, ptr - stride, 8);
	}
	// now calculate the stuff we need
	if (edges & 3) { // mb_x ==0 || mb_y == 0) {
		int avg = (sum + 4) >> 3;

		if (edges & 1)                            // (mb_x == 0) { // implies mb_y !=0
			memset(dst + area1, avg, 8 + 8 + 1);  // areas 1, 2, 3 are averaged
		else                                      // implies y == 0 x != 0
			memset(dst + area3, avg, 1 + 16 + 8); // areas 3, 4, 5, 6

		sum += avg * 9;
	} else {
		// the edge pixel, in the top line and left column
		c = *(src - 1 - stride);
		dst[area3] = c;
		sum += c;
		// edge pixel is not part of min/max
	}
	*range = max_pix - min_pix;
	sum += *(dst + area5) + *(dst + area5 + 1);
	*psum = sum;
}

static const uint16 zero_prediction_weights[64 * 2] = {
	640, 640, 669, 480,  708, 354,  748, 257,  792, 198, 760, 143, 808, 101, 772, 72,  480, 669, 537, 537, 598, 416,
	661, 316, 719, 250,  707, 185,  768, 134,  745, 97,  354, 708, 416, 598, 488, 488, 564, 388, 634, 317, 642, 241,
	716, 179, 706, 132,  257, 748,  316, 661,  388, 564, 469, 469, 543, 395, 571, 311, 655, 238, 660, 180, 198, 792,
	250, 719, 317, 634,  395, 543,  469, 469,  507, 380, 597, 299, 616, 231, 161, 855, 206, 788, 266, 710, 340, 623,
	411, 548, 455, 455,  548, 366,  576, 288,  122, 972, 159, 914, 211, 842, 276, 758, 341, 682, 389, 584, 483, 483,
	520, 390, 110, 1172, 144, 1107, 193, 1028, 254, 932, 317, 846, 366, 731, 458, 611, 499, 499,
};

static void spatial_compensation_0(const uint8 *src, uint8 *dst, ptrdiff_t stride) {
	int i, j;
	int x, y;
	unsigned int p; // power divided by 2
	int a;
	uint16 left_sum[2][8] = {{0}};
	uint16 top_sum[2][8] = {{0}};

	for (i = 0; i < 8; i++) {
		a = src[area2 + 7 - i] << 4;
		for (j = 0; j < 8; j++) {
			p = abs(i - j);
			left_sum[p & 1][j] += a >> (p >> 1);
		}
	}

	for (i = 0; i < 8; i++) {
		a = src[area4 + i] << 4;
		for (j = 0; j < 8; j++) {
			p = abs(i - j);
			top_sum[p & 1][j] += a >> (p >> 1);
		}
	}
	for (; i < 10; i++) {
		a = src[area4 + i] << 4;
		for (j = 5; j < 8; j++) {
			p = abs(i - j);
			top_sum[p & 1][j] += a >> (p >> 1);
		}
	}
	for (; i < 12; i++) {
		a = src[area4 + i] << 4;
		for (j = 7; j < 8; j++) {
			p = abs(i - j);
			top_sum[p & 1][j] += a >> (p >> 1);
		}
	}

	for (i = 0; i < 8; i++) {
		top_sum[0][i] += (top_sum[1][i] * 181 + 128) >> 8; // 181 is sqrt(2)/2
		left_sum[0][i] += (left_sum[1][i] * 181 + 128) >> 8;
	}
	for (y = 0; y < 8; y++) {
		for (x = 0; x < 8; x++)
			dst[x] = ((uint32)top_sum[0][x] * zero_prediction_weights[y * 16 + x * 2 + 0] +
					  (uint32)left_sum[0][y] * zero_prediction_weights[y * 16 + x * 2 + 1] + 0x8000) >>
					 16;
		dst += stride;
	}
}

static void spatial_compensation_1(const uint8 *src, uint8 *dst, ptrdiff_t stride) {
	int x, y;

	for (y = 0; y < 8; y++) {
		for (x = 0; x < 8; x++)
			dst[x] = src[area4 + MIN(2 * y + x + 2, 15)];
		dst += stride;
	}
}

static void spatial_compensation_2(const uint8 *src, uint8 *dst, ptrdiff_t stride) {
	for (int y = 0; y < 8; y++) {
		memcpy(dst, src + area4 + 1 + y, 8);
		dst += stride;
	}
}

static void spatial_compensation_3(const uint8 *src, uint8 *dst, ptrdiff_t stride) {
	for (int y = 0; y < 8; y++) {
		memcpy(dst, src + area4 + ((y + 1) >> 1), 8);
		dst += stride;
	}
}

static void spatial_compensation_4(const uint8 *src, uint8 *dst, ptrdiff_t stride) {
	int x, y;

	for (y = 0; y < 8; y++) {
		for (x = 0; x < 8; x++)
			dst[x] = (src[area4 + x] + src[area6 + x] + 1) >> 1;
		dst += stride;
	}
}

static void spatial_compensation_5(const uint8 *src, uint8 *dst, ptrdiff_t stride) {
	int x, y;

	for (y = 0; y < 8; y++) {
		for (x = 0; x < 8; x++) {
			if (2 * x - y < 0)
				dst[x] = src[area2 + 9 + 2 * x - y];
			else
				dst[x] = src[area4 + x - ((y + 1) >> 1)];
		}
		dst += stride;
	}
}

static void spatial_compensation_6(const uint8 *src, uint8 *dst, ptrdiff_t stride) {
	for (int y = 0; y < 8; y++) {
		memcpy(dst, src + area3 - y, 8);
		dst += stride;
	}
}

static void spatial_compensation_7(const uint8 *src, uint8 *dst, ptrdiff_t stride) {
	int x, y;

	for (y = 0; y < 8; y++) {
		for (x = 0; x < 8; x++) {
			if (x - 2 * y > 0)
				dst[x] = (src[area3 - 1 + x - 2 * y] + src[area3 + x - 2 * y] + 1) >> 1;
			else
				dst[x] = src[area2 + 8 - y + (x >> 1)];
		}
		dst += stride;
	}
}

static void spatial_compensation_8(const uint8 *src, uint8 *dst, ptrdiff_t stride) {
	int x, y;

	for (y = 0; y < 8; y++) {
		for (x = 0; x < 8; x++)
			dst[x] = (src[area1 + 7 - y] + src[area2 + 7 - y] + 1) >> 1;
		dst += stride;
	}
}

static void spatial_compensation_9(const uint8 *src, uint8 *dst, ptrdiff_t stride) {
	int x, y;

	for (y = 0; y < 8; y++) {
		for (x = 0; x < 8; x++)
			dst[x] = src[area2 + 6 - MIN(x + y, 6)];
		dst += stride;
	}
}

static void spatial_compensation_10(const uint8 *src, uint8 *dst, ptrdiff_t stride) {
	int x, y;

	for (y = 0; y < 8; y++) {
		for (x = 0; x < 8; x++)
			dst[x] = (src[area2 + 7 - y] * (8 - x) + src[area4 + x] * x + 4) >> 3;
		dst += stride;
	}
}

static void spatial_compensation_11(const uint8 *src, uint8 *dst, ptrdiff_t stride) {
	int x, y;

	for (y = 0; y < 8; y++) {
		for (x = 0; x < 8; x++)
			dst[x] = (src[area2 + 7 - y] * y + src[area4 + x] * (8 - y) + 4) >> 3;
		dst += stride;
	}
}

static void x8_loop_filter(uint8 *ptr, const ptrdiff_t a_stride, const ptrdiff_t b_stride, int quant) {
	int i, t;
	int p0, p1, p2, p3, p4, p5, p6, p7, p8, p9;
	int ql = (quant + 10) >> 3;

	for (i = 0; i < 8; i++, ptr += b_stride) {
		p0 = ptr[-5 * a_stride];
		p1 = ptr[-4 * a_stride];
		p2 = ptr[-3 * a_stride];
		p3 = ptr[-2 * a_stride];
		p4 = ptr[-1 * a_stride];
		p5 = ptr[0];
		p6 = ptr[1 * a_stride];
		p7 = ptr[2 * a_stride];
		p8 = ptr[3 * a_stride];
		p9 = ptr[4 * a_stride];

		t = (ABS(p1 - p2) <= ql) + (ABS(p2 - p3) <= ql) + (ABS(p3 - p4) <= ql) + (ABS(p4 - p5) <= ql);

		// You need at least 1 to be able to reach a total score of 6.
		if (t > 0) {
			t += (ABS(p5 - p6) <= ql) + (ABS(p6 - p7) <= ql) + (ABS(p7 - p8) <= ql) + (ABS(p8 - p9) <= ql) +
				 (ABS(p0 - p1) <= ql);
			if (t >= 6) {
				int min, max;

				min = max = p1;
				min = MIN(min, p3);
				max = MAX(max, p3);
				min = MIN(min, p5);
				max = MAX(max, p5);
				min = MIN(min, p8);
				max = MAX(max, p8);
				if (max - min < 2 * quant) { // early stop
					min = MIN(min, p2);
					max = MAX(max, p2);
					min = MIN(min, p4);
					max = MAX(max, p4);
					min = MIN(min, p6);
					max = MAX(max, p6);
					min = MIN(min, p7);
					max = MAX(max, p7);
					if (max - min < 2 * quant) {
						ptr[-2 * a_stride] = (4 * p2 + 3 * p3 + 1 * p7 + 4) >> 3;
						ptr[-1 * a_stride] = (3 * p2 + 3 * p4 + 2 * p7 + 4) >> 3;
						ptr[0] = (2 * p2 + 3 * p5 + 3 * p7 + 4) >> 3;
						ptr[1 * a_stride] = (1 * p2 + 3 * p6 + 4 * p7 + 4) >> 3;
						continue;
					}
				}
			}
		}
		{
			int x, x0, x1, x2;
			int m;

			x0 = (2 * p3 - 5 * p4 + 5 * p5 - 2 * p6 + 4) >> 3;
			if (ABS(x0) < quant) {
				x1 = (2 * p1 - 5 * p2 + 5 * p3 - 2 * p4 + 4) >> 3;
				x2 = (2 * p5 - 5 * p6 + 5 * p7 - 2 * p8 + 4) >> 3;

				x = ABS(x0) - MIN(ABS(x1), ABS(x2));
				m = p4 - p5;

				if (x > 0 && (m ^ x0) < 0) {
					int32 sign;

					sign = m >> 31;
					m = (m ^ sign) - sign; // abs(m)
					m >>= 1;

					x = 5 * x >> 3;

					if (x > m)
						x = m;

					x = (x ^ sign) - sign;

					ptr[-1 * a_stride] -= x;
					ptr[0] += x;
				}
			}
		}
	}
}

static void x8_h_loop_filter(uint8 *src, ptrdiff_t stride, int qscale) {
	x8_loop_filter(src, stride, 1, qscale);
}

static void x8_v_loop_filter(uint8 *src, ptrdiff_t stride, int qscale) {
	x8_loop_filter(src, 1, stride, qscale);
}

void ff_intrax8dsp_init(IntraX8DSPContext *dsp) {
	dsp->h_loop_filter = x8_h_loop_filter;
	dsp->v_loop_filter = x8_v_loop_filter;
	dsp->setup_spatial_compensation = x8_setup_spatial_compensation;
	dsp->spatial_compensation[0] = spatial_compensation_0;
	dsp->spatial_compensation[1] = spatial_compensation_1;
	dsp->spatial_compensation[2] = spatial_compensation_2;
	dsp->spatial_compensation[3] = spatial_compensation_3;
	dsp->spatial_compensation[4] = spatial_compensation_4;
	dsp->spatial_compensation[5] = spatial_compensation_5;
	dsp->spatial_compensation[6] = spatial_compensation_6;
	dsp->spatial_compensation[7] = spatial_compensation_7;
	dsp->spatial_compensation[8] = spatial_compensation_8;
	dsp->spatial_compensation[9] = spatial_compensation_9;
	dsp->spatial_compensation[10] = spatial_compensation_10;
	dsp->spatial_compensation[11] = spatial_compensation_11;
}

#undef area1
#undef area2
#undef area3
#undef area4
#undef area5
#undef area6

static const uint8 x8_orient_lowquant_table[4][12][2] = {
	{
		/* Orient lowquant table 0 */
		{0, 1},
		{1, 3},
		{2, 3},
		{3, 4},
		{4, 4},
		{5, 6},
		{7, 6},
		{6, 5},
		{8, 6},
		{9, 6},
		{10, 6},
		{11, 6},
	},
	{
		/* Orient lowquant table 1 */
		{0, 5},
		{1, 5},
		{2, 5},
		{6, 5},
		{5, 4},
		{8, 4},
		{3, 2},
		{4, 2},
		{7, 3},
		{9, 4},
		{10, 5},
		{11, 5},
	},
	{
		/* Orient lowquant table 2 */
		{0, 2},
		{1, 2},
		{2, 3},
		{3, 3},
		{4, 3},
		{5, 6},
		{6, 6},
		{7, 5},
		{8, 6},
		{9, 6},
		{10, 6},
		{11, 6},
	},
	{
		/* Orient lowquant table 3 */
		{0, 3},
		{1, 4},
		{2, 4},
		{3, 2},
		{4, 2},
		{5, 5},
		{6, 5},
		{7, 4},
		{8, 5},
		{9, 5},
		{10, 5},
		{11, 5},
	},
};

static const uint8 x8_orient_highquant_table[2][12][2] = {
	{
		/* Orient highquant table 0 */
		{0, 2},
		{1, 2},
		{2, 3},
		{3, 3},
		{4, 3},
		{5, 6},
		{7, 6},
		{6, 5},
		{8, 6},
		{9, 6},
		{10, 6},
		{11, 6},
	},
	{
		/* Orient highquant table 1 */
		{0, 1},
		{1, 2},
		{2, 3},
		{3, 5},
		{4, 5},
		{5, 7},
		{7, 7},
		{6, 6},
		{8, 7},
		{9, 7},
		{10, 7},
		{11, 7},
	},
};
#define MAX_OR_VLC_BITS 7

static const uint8 x8_dc_quant_table[2][8][34][2] = {
	{
		{
			/* DC highquant table 0 */
			{0, 5},   {3, 5},   {1, 4},   {2, 4},   {4, 5},   {6, 5},   {5, 4},   {7, 5},   {8, 7},
			{9, 7},   {10, 12}, {11, 12}, {12, 12}, {13, 12}, {14, 12}, {15, 12}, {16, 12}, {25, 12},
			{26, 12}, {27, 12}, {28, 12}, {29, 12}, {30, 12}, {31, 12}, {32, 12}, {33, 12}, {24, 8},
			{23, 7},  {17, 3},  {18, 2},  {19, 3},  {20, 4},  {21, 5},  {22, 5},
		},
		{
			/* DC highquant table 1 */
			{0, 3},   {1, 3},   {2, 3},   {3, 4},   {4, 4},   {5, 3},   {6, 4},   {7, 4},   {8, 6},
			{9, 7},   {10, 7},  {11, 11}, {12, 11}, {13, 11}, {14, 11}, {15, 11}, {16, 11}, {25, 11},
			{27, 11}, {28, 11}, {29, 11}, {30, 11}, {31, 11}, {32, 10}, {33, 10}, {23, 7},  {17, 6},
			{18, 4},  {19, 5},  {20, 5},  {21, 6},  {24, 7},  {26, 7},  {22, 5},
		},
		{
			/* DC highquant table 2 */
			{0, 7},   {1, 7},   {2, 6},   {3, 7},   {5, 7},   {4, 6},   {6, 8},   {7, 8},   {8, 13},
			{9, 13},  {10, 13}, {11, 13}, {12, 13}, {13, 13}, {14, 13}, {15, 13}, {16, 12}, {23, 12},
			{24, 12}, {25, 12}, {26, 12}, {27, 12}, {28, 12}, {29, 12}, {30, 12}, {31, 12}, {32, 12},
			{33, 12}, {22, 8},  {21, 6},  {20, 5},  {19, 3},  {18, 2},  {17, 1},
		},
		{
			/* DC highquant table 3 */
			{0, 4},   {3, 4},   {1, 3},   {2, 3},   {4, 4},   {8, 5},   {9, 5},   {5, 3},   {6, 3},
			{7, 3},   {10, 7},  {11, 11}, {12, 11}, {13, 11}, {14, 11}, {15, 11}, {16, 11}, {25, 11},
			{26, 11}, {27, 11}, {29, 11}, {30, 11}, {31, 11}, {32, 10}, {33, 10}, {18, 7},  {21, 7},
			{17, 5},  {19, 6},  {20, 6},  {22, 7},  {23, 7},  {24, 7},  {28, 7},
		},
		{
			/* DC highquant table 4 */
			{0, 8},   {4, 9},   {6, 9},   {1, 7},   {2, 7},   {3, 7},   {5, 8},   {7, 14},  {8, 14},
			{9, 14},  {10, 14}, {11, 14}, {12, 14}, {13, 14}, {14, 14}, {15, 13}, {16, 13}, {24, 13},
			{25, 13}, {26, 13}, {27, 13}, {28, 13}, {29, 13}, {30, 13}, {31, 13}, {32, 13}, {33, 13},
			{23, 9},  {22, 7},  {21, 6},  {20, 4},  {19, 3},  {17, 2},  {18, 1},
		},
		{
			/* DC highquant table 5 */
			{0, 7},  {1, 7},  {2, 6},  {3, 6},  {4, 6},  {5, 6},  {6, 6},  {7, 6},  {8, 6},  {9, 6},  {10, 6}, {11, 6},
			{12, 6}, {13, 6}, {14, 6}, {15, 6}, {16, 6}, {17, 6}, {18, 6}, {19, 6}, {20, 6}, {21, 6}, {22, 6}, {23, 6},
			{24, 6}, {25, 6}, {26, 6}, {28, 6}, {29, 6}, {30, 6}, {31, 6}, {32, 6}, {33, 6}, {27, 1},
		},
		{
			/* DC highquant table 6 */
			{0, 5},   {2, 5},   {1, 4},   {3, 5},   {4, 6},   {6, 6},   {5, 5},   {7, 6},   {8, 8},
			{9, 8},   {10, 13}, {11, 13}, {12, 13}, {13, 13}, {14, 13}, {15, 13}, {16, 13}, {25, 13},
			{26, 13}, {27, 13}, {28, 13}, {29, 13}, {30, 13}, {31, 13}, {32, 13}, {33, 13}, {24, 9},
			{23, 8},  {17, 2},  {18, 2},  {19, 3},  {20, 4},  {21, 5},  {22, 5},
		},
		{
			/* DC highquant table 7 */
			{0, 7},  {1, 7},  {2, 7},  {3, 7},  {4, 7},  {5, 7},  {6, 7},  {7, 7},  {8, 7},  {9, 7},  {10, 7}, {11, 7},
			{12, 7}, {13, 7}, {14, 7}, {15, 7}, {16, 7}, {19, 7}, {20, 7}, {21, 7}, {22, 7}, {23, 7}, {24, 7}, {25, 7},
			{26, 7}, {27, 7}, {28, 7}, {29, 7}, {30, 7}, {31, 7}, {32, 7}, {33, 7}, {18, 2}, {17, 1},
		},
	},
	{
		{
			/* DC lowquant table 0 */
			{0, 5},   {2, 5},   {1, 4},   {3, 5},   {4, 5},   {5, 5},   {6, 6},   {7, 6},   {8, 8},
			{9, 8},   {10, 13}, {11, 13}, {12, 13}, {13, 13}, {14, 13}, {15, 13}, {16, 13}, {25, 13},
			{26, 13}, {27, 13}, {28, 13}, {29, 13}, {30, 13}, {31, 13}, {32, 13}, {33, 13}, {24, 9},
			{23, 8},  {22, 6},  {21, 5},  {20, 4},  {19, 3},  {17, 2},  {18, 2},
		},
		{
			/* DC lowquant table 1 */
			{0, 4},   {3, 4},   {1, 3},   {2, 3},   {4, 4},   {6, 4},   {5, 3},   {7, 3},   {8, 4},
			{9, 4},   {10, 5},  {11, 6},  {12, 9},  {13, 9},  {14, 13}, {15, 13}, {16, 13}, {25, 13},
			{26, 13}, {27, 13}, {28, 12}, {29, 12}, {31, 12}, {32, 12}, {33, 12}, {21, 9},  {20, 7},
			{17, 6},  {18, 6},  {19, 6},  {22, 7},  {23, 8},  {24, 9},  {30, 9},
		},
		{
			/* DC lowquant table 2 */
			{0, 4},   {3, 4},   {1, 3},   {2, 3},   {4, 4},   {5, 4},   {6, 4},   {7, 4},   {8, 6},
			{9, 6},   {10, 7},  {11, 7},  {12, 12}, {13, 12}, {14, 12}, {15, 12}, {16, 12}, {25, 12},
			{26, 12}, {27, 12}, {28, 12}, {29, 12}, {30, 12}, {31, 12}, {32, 11}, {33, 11}, {24, 8},
			{23, 7},  {17, 4},  {18, 3},  {19, 4},  {20, 5},  {21, 6},  {22, 6},
		},
		{
			/* DC lowquant table 3 */
			{0, 4},   {6, 4},   {2, 3},   {1, 2},   {3, 3},   {4, 3},   {5, 3},   {7, 4},   {8, 6},
			{9, 6},   {10, 7},  {11, 8},  {12, 14}, {13, 14}, {14, 14}, {15, 14}, {16, 13}, {21, 13},
			{22, 13}, {23, 13}, {24, 13}, {25, 13}, {26, 13}, {27, 13}, {28, 13}, {29, 13}, {30, 13},
			{31, 13}, {32, 13}, {33, 13}, {17, 9},  {18, 7},  {19, 8},  {20, 8},
		},
		{
			/* DC lowquant table 4 */
			{0, 7},  {1, 7},  {2, 7},  {3, 7},  {4, 7},  {5, 7},  {6, 7},  {7, 7},  {8, 7},  {9, 7},  {10, 7}, {11, 7},
			{12, 7}, {13, 7}, {14, 7}, {15, 7}, {16, 7}, {19, 7}, {20, 7}, {21, 7}, {22, 7}, {23, 7}, {24, 7}, {25, 7},
			{26, 7}, {27, 7}, {28, 7}, {29, 7}, {30, 7}, {31, 7}, {32, 7}, {33, 7}, {18, 2}, {17, 1},
		},
		{
			/* DC lowquant table 5 */
			{0, 5},   {2, 5},   {1, 4},   {3, 6},   {4, 6},   {5, 6},   {6, 7},   {8, 7},   {7, 6},
			{9, 6},   {10, 8},  {12, 8},  {11, 7},  {13, 13}, {14, 13}, {15, 13}, {16, 13}, {23, 13},
			{24, 13}, {25, 13}, {26, 13}, {27, 13}, {28, 13}, {29, 13}, {30, 13}, {31, 13}, {32, 13},
			{33, 12}, {22, 9},  {21, 8},  {20, 7},  {17, 3},  {19, 3},  {18, 1},
		},
		{
			/* DC lowquant table 6 */
			{0, 4},   {4, 4},   {2, 3},   {1, 2},   {3, 3},   {5, 4},   {6, 5},   {7, 6},   {8, 7},
			{9, 7},   {10, 8},  {11, 14}, {12, 14}, {15, 14}, {16, 14}, {23, 14}, {24, 14}, {25, 14},
			{26, 14}, {27, 14}, {28, 14}, {29, 14}, {30, 14}, {31, 14}, {32, 14}, {33, 13}, {13, 10},
			{14, 9},  {21, 8},  {22, 8},  {20, 6},  {19, 5},  {17, 4},  {18, 3},
		},
		{
			/* DC lowquant table 7 */
			{0, 6},   {1, 6},   {2, 6},   {3, 7},   {4, 7},   {5, 6},   {6, 6},   {7, 6},   {8, 7},
			{9, 8},   {10, 8},  {11, 9},  {12, 10}, {13, 14}, {14, 14}, {15, 14}, {16, 14}, {24, 14},
			{25, 14}, {26, 14}, {27, 14}, {28, 14}, {29, 14}, {30, 14}, {31, 14}, {32, 13}, {33, 13},
			{23, 8},  {22, 7},  {21, 6},  {20, 5},  {19, 4},  {18, 2},  {17, 1},
		},
	},
};
#define MAX_DC_VLC_BITS 14

static const uint8 x8_ac_quant_table[2][2][8][77][2] = {
	{
		{
			{
				/* AC highquant table group 0, table 0 */
				{0, 3},   {1, 4},   {2, 6},   {3, 6},   {4, 7},   {6, 15},  {8, 15},  {10, 15}, {12, 15}, {13, 15},
				{14, 15}, {15, 14}, {34, 14}, {45, 14}, {46, 14}, {47, 14}, {49, 14}, {50, 14}, {51, 14}, {52, 14},
				{53, 14}, {54, 14}, {55, 14}, {56, 14}, {57, 14}, {58, 14}, {60, 14}, {61, 14}, {62, 14}, {63, 14},
				{64, 14}, {65, 14}, {67, 14}, {69, 14}, {70, 14}, {71, 14}, {72, 14}, {74, 14}, {75, 14}, {76, 14},
				{9, 9},   {7, 8},   {5, 6},   {11, 9},  {18, 9},  {19, 9},  {22, 9},  {20, 7},  {17, 6},  {16, 5},
				{21, 7},  {29, 8},  {33, 8},  {28, 6},  {27, 5},  {24, 3},  {23, 2},  {25, 4},  {26, 4},  {30, 7},
				{32, 7},  {31, 6},  {35, 7},  {36, 7},  {37, 7},  {38, 9},  {41, 9},  {42, 8},  {39, 5},  {40, 6},
				{43, 9},  {44, 9},  {48, 8},  {59, 9},  {66, 9},  {68, 9},  {73, 9},
			},
			{
				/* AC highquant table group 0, table 1 */
				{0, 3},   {1, 4},   {2, 4},   {3, 5},   {4, 6},   {5, 6},   {6, 6},   {7, 6},   {8, 7},   {9, 8},
				{10, 8},  {11, 8},  {12, 8},  {13, 14}, {44, 14}, {47, 14}, {50, 14}, {51, 14}, {54, 14}, {55, 14},
				{56, 14}, {63, 14}, {64, 14}, {67, 14}, {68, 14}, {69, 14}, {70, 14}, {72, 14}, {76, 14}, {14, 10},
				{15, 10}, {45, 10}, {32, 8},  {16, 4},  {17, 5},  {18, 6},  {19, 7},  {31, 7},  {20, 5},  {21, 6},
				{26, 6},  {22, 5},  {24, 5},  {23, 4},  {25, 5},  {27, 6},  {28, 6},  {29, 6},  {30, 6},  {33, 8},
				{38, 8},  {34, 7},  {35, 7},  {36, 7},  {37, 7},  {39, 7},  {40, 7},  {41, 8},  {42, 8},  {43, 8},
				{46, 9},  {49, 9},  {53, 8},  {58, 8},  {57, 6},  {48, 5},  {52, 5},  {59, 7},  {60, 8},  {61, 9},
				{66, 9},  {62, 8},  {65, 8},  {71, 9},  {75, 9},  {74, 8},  {73, 5},
			},
			{
				/* AC highquant table group 0, table 2 */
				{0, 4},   {1, 5},   {2, 12},  {3, 12},  {4, 12},  {5, 12},  {6, 12},  {7, 12},  {8, 12},  {9, 12},
				{10, 12}, {11, 12}, {12, 11}, {13, 11}, {14, 11}, {15, 11}, {16, 11}, {17, 11}, {18, 11}, {19, 11},
				{20, 11}, {21, 11}, {22, 11}, {28, 11}, {29, 11}, {30, 11}, {31, 11}, {32, 11}, {33, 11}, {34, 11},
				{35, 11}, {36, 11}, {37, 11}, {38, 11}, {40, 11}, {41, 11}, {42, 11}, {43, 11}, {44, 11}, {45, 11},
				{46, 11}, {47, 11}, {48, 11}, {49, 11}, {50, 11}, {51, 11}, {52, 11}, {53, 11}, {54, 11}, {55, 11},
				{56, 11}, {57, 11}, {58, 11}, {59, 11}, {60, 11}, {61, 11}, {62, 11}, {63, 11}, {64, 11}, {65, 11},
				{66, 11}, {67, 11}, {68, 11}, {69, 11}, {70, 11}, {71, 11}, {72, 11}, {73, 11}, {74, 11}, {75, 11},
				{76, 11}, {25, 5},  {26, 5},  {27, 5},  {39, 5},  {24, 2},  {23, 1},
			},
			{
				/* AC highquant table group 0, table 3 */
				{0, 8},  {1, 8},  {2, 8},  {3, 8},  {4, 8},  {5, 8},  {6, 8},  {7, 8},  {8, 8},  {9, 8},  {10, 8},
				{11, 8}, {12, 8}, {13, 8}, {14, 8}, {15, 8}, {16, 8}, {17, 8}, {18, 8}, {19, 8}, {20, 8}, {21, 8},
				{22, 8}, {24, 8}, {25, 7}, {26, 7}, {27, 7}, {28, 7}, {29, 7}, {30, 7}, {31, 7}, {32, 7}, {33, 7},
				{34, 7}, {35, 7}, {36, 7}, {37, 7}, {38, 7}, {39, 7}, {40, 7}, {41, 7}, {42, 7}, {43, 7}, {44, 7},
				{45, 7}, {46, 7}, {47, 7}, {48, 7}, {49, 7}, {50, 7}, {51, 7}, {52, 7}, {53, 7}, {54, 7}, {55, 7},
				{56, 7}, {57, 7}, {58, 7}, {59, 7}, {60, 7}, {61, 7}, {62, 7}, {63, 7}, {64, 7}, {65, 7}, {66, 7},
				{67, 7}, {68, 7}, {69, 7}, {70, 7}, {71, 7}, {72, 7}, {73, 7}, {74, 7}, {75, 7}, {76, 7}, {23, 1},
			},
			{
				/* AC highquant table group 0, table 4 */
				{0, 9},  {1, 9},  {2, 9},  {3, 9},  {4, 9},  {5, 9},  {6, 9},  {7, 9},  {8, 9},  {9, 9},  {10, 9},
				{11, 9}, {12, 9}, {13, 9}, {14, 9}, {15, 9}, {16, 9}, {17, 9}, {18, 9}, {19, 9}, {20, 9}, {21, 9},
				{22, 8}, {25, 8}, {26, 8}, {27, 8}, {28, 8}, {29, 8}, {30, 8}, {31, 8}, {32, 8}, {33, 8}, {34, 8},
				{35, 8}, {36, 8}, {37, 8}, {38, 8}, {39, 8}, {40, 8}, {41, 8}, {42, 8}, {43, 8}, {44, 8}, {45, 8},
				{46, 8}, {47, 8}, {48, 8}, {49, 8}, {50, 8}, {51, 8}, {52, 8}, {53, 8}, {54, 8}, {55, 8}, {56, 8},
				{57, 8}, {58, 8}, {59, 8}, {60, 8}, {61, 8}, {62, 8}, {63, 8}, {64, 8}, {65, 8}, {66, 8}, {67, 8},
				{68, 8}, {69, 8}, {70, 8}, {71, 8}, {72, 8}, {73, 8}, {74, 8}, {75, 8}, {76, 8}, {23, 2}, {24, 1},
			},
			{
				/* AC highquant table group 0, table 5 */
				{0, 10},  {1, 10},  {2, 10},  {3, 10},  {4, 10},  {5, 10},  {6, 10},  {7, 10},  {8, 10},  {9, 10},
				{10, 10}, {11, 10}, {12, 10}, {13, 10}, {14, 10}, {15, 10}, {16, 10}, {17, 10}, {18, 10}, {19, 10},
				{20, 9},  {21, 9},  {22, 9},  {25, 9},  {26, 9},  {28, 9},  {29, 9},  {30, 9},  {31, 9},  {32, 9},
				{33, 9},  {34, 9},  {35, 9},  {36, 9},  {37, 9},  {38, 9},  {39, 9},  {40, 9},  {41, 9},  {42, 9},
				{43, 9},  {44, 9},  {45, 9},  {46, 9},  {47, 9},  {48, 9},  {49, 9},  {50, 9},  {51, 9},  {52, 9},
				{53, 9},  {54, 9},  {55, 9},  {56, 9},  {57, 9},  {58, 9},  {59, 9},  {60, 9},  {61, 9},  {62, 9},
				{63, 9},  {64, 9},  {65, 9},  {66, 9},  {67, 9},  {68, 9},  {69, 9},  {70, 9},  {71, 9},  {72, 9},
				{73, 9},  {74, 9},  {75, 9},  {76, 9},  {24, 3},  {27, 2},  {23, 1},
			},
			{
				/* AC highquant table group 0, table 6 */
				{0, 2},   {1, 4},   {2, 5},   {3, 5},   {4, 6},   {5, 6},   {6, 7},   {7, 8},   {8, 8},   {9, 8},
				{10, 9},  {11, 9},  {12, 10}, {14, 10}, {13, 9},  {15, 15}, {42, 15}, {46, 15}, {50, 15}, {51, 15},
				{55, 15}, {56, 14}, {59, 14}, {61, 14}, {62, 14}, {63, 14}, {64, 14}, {68, 14}, {69, 14}, {70, 14},
				{71, 14}, {72, 14}, {75, 14}, {76, 14}, {43, 10}, {37, 9},  {17, 5},  {18, 6},  {19, 6},  {16, 3},
				{20, 4},  {21, 7},  {24, 7},  {25, 7},  {26, 7},  {22, 5},  {23, 5},  {27, 8},  {30, 8},  {28, 7},
				{29, 7},  {31, 8},  {32, 8},  {33, 8},  {34, 8},  {35, 8},  {36, 8},  {38, 8},  {39, 8},  {40, 8},
				{41, 9},  {44, 10}, {45, 10}, {47, 10}, {54, 10}, {49, 9},  {58, 8},  {60, 10}, {66, 10}, {67, 10},
				{74, 10}, {65, 8},  {48, 6},  {52, 4},  {53, 6},  {57, 6},  {73, 5},
			},
			{
				/* AC highquant table group 0, table 7 */
				{0, 3},   {1, 4},   {2, 5},   {3, 5},   {4, 6},   {5, 8},   {9, 13},  {12, 13}, {13, 13}, {14, 13},
				{42, 13}, {45, 13}, {46, 13}, {47, 13}, {50, 13}, {51, 13}, {53, 13}, {54, 13}, {55, 13}, {56, 13},
				{58, 13}, {61, 13}, {62, 13}, {63, 13}, {64, 13}, {66, 13}, {67, 13}, {68, 13}, {69, 13}, {70, 13},
				{72, 12}, {74, 12}, {75, 12}, {76, 12}, {6, 7},   {7, 6},   {8, 7},   {10, 8},  {11, 8},  {15, 8},
				{31, 8},  {18, 7},  {19, 6},  {17, 5},  {16, 4},  {20, 6},  {21, 7},  {36, 8},  {37, 8},  {22, 6},
				{27, 6},  {23, 3},  {24, 5},  {25, 5},  {26, 4},  {28, 6},  {29, 6},  {30, 5},  {32, 6},  {33, 6},
				{34, 6},  {35, 6},  {38, 7},  {40, 7},  {41, 7},  {43, 7},  {39, 5},  {44, 8},  {52, 8},  {48, 7},
				{49, 7},  {57, 8},  {60, 8},  {59, 7},  {65, 8},  {71, 8},  {73, 6},
			},
		},
		{
			{
				/* AC highquant table group 1, table 0 */
				{0, 3},   {1, 4},   {2, 5},   {3, 5},   {4, 5},   {5, 5},   {6, 6},   {7, 7},   {9, 7},   {8, 6},
				{10, 8},  {13, 8},  {11, 7},  {12, 7},  {14, 8},  {15, 9},  {41, 9},  {17, 6},  {18, 6},  {19, 7},
				{21, 8},  {42, 9},  {43, 9},  {16, 4},  {20, 5},  {22, 6},  {26, 6},  {23, 4},  {24, 4},  {25, 5},
				{27, 6},  {28, 6},  {29, 6},  {30, 7},  {31, 7},  {32, 7},  {33, 7},  {34, 7},  {35, 7},  {36, 7},
				{37, 7},  {38, 7},  {39, 7},  {40, 7},  {44, 11}, {54, 14}, {55, 14}, {56, 14}, {64, 14}, {69, 14},
				{70, 14}, {72, 14}, {76, 14}, {53, 10}, {45, 9},  {47, 8},  {46, 6},  {48, 5},  {49, 7},  {50, 9},
				{51, 9},  {58, 10}, {67, 10}, {61, 9},  {52, 7},  {57, 7},  {59, 7},  {60, 8},  {62, 8},  {63, 7},
				{65, 7},  {66, 7},  {68, 10}, {71, 10}, {74, 9},  {75, 8},  {73, 6},
			},
			{
				/* AC highquant table group 1, table 1 */
				{0, 2},   {1, 4},   {2, 5},   {3, 5},   {4, 6},   {5, 7},   {6, 7},   {7, 7},   {8, 8},   {9, 9},
				{10, 9},  {11, 9},  {12, 9},  {13, 15}, {42, 15}, {44, 15}, {45, 15}, {46, 15}, {47, 15}, {50, 15},
				{51, 15}, {56, 15}, {61, 15}, {62, 15}, {63, 15}, {64, 15}, {66, 15}, {67, 14}, {15, 11}, {14, 10},
				{30, 9},  {19, 7},  {17, 5},  {18, 6},  {23, 6},  {16, 3},  {20, 4},  {21, 5},  {24, 7},  {25, 7},
				{26, 8},  {27, 8},  {28, 7},  {22, 4},  {29, 8},  {31, 8},  {32, 9},  {33, 9},  {34, 9},  {35, 9},
				{36, 10}, {37, 11}, {38, 11}, {39, 9},  {40, 9},  {41, 11}, {55, 11}, {43, 10}, {49, 8},  {54, 8},
				{48, 6},  {53, 7},  {58, 8},  {59, 11}, {68, 14}, {69, 14}, {70, 14}, {71, 14}, {72, 14}, {74, 14},
				{75, 14}, {76, 14}, {60, 10}, {65, 9},  {52, 4},  {57, 5},  {73, 5},
			},
			{
				/* AC highquant table group 1, table 2 */
				{0, 3},   {1, 4},   {2, 4},   {3, 5},   {4, 6},   {5, 7},   {7, 7},   {6, 6},   {8, 9},   {9, 9},
				{10, 8},  {11, 15}, {13, 15}, {14, 15}, {15, 15}, {46, 15}, {47, 15}, {50, 15}, {51, 15}, {54, 14},
				{55, 14}, {56, 14}, {62, 14}, {63, 14}, {64, 14}, {66, 14}, {67, 14}, {69, 14}, {70, 14}, {72, 14},
				{76, 14}, {34, 10}, {12, 9},  {31, 8},  {17, 5},  {16, 3},  {18, 7},  {19, 7},  {21, 6},  {22, 5},
				{20, 4},  {23, 4},  {24, 4},  {25, 5},  {26, 6},  {27, 6},  {28, 6},  {29, 6},  {30, 7},  {32, 7},
				{33, 8},  {35, 8},  {36, 9},  {37, 10}, {38, 10}, {41, 9},  {42, 9},  {39, 6},  {40, 7},  {43, 8},
				{44, 10}, {60, 10}, {45, 9},  {48, 7},  {49, 9},  {58, 9},  {53, 8},  {57, 6},  {52, 5},  {59, 9},
				{61, 10}, {68, 10}, {65, 8},  {71, 9},  {75, 9},  {74, 8},  {73, 6},
			},
			{
				/* AC highquant table group 1, table 3 */
				{0, 3},   {1, 4},   {2, 5},   {3, 5},   {4, 6},   {5, 6},   {6, 7},   {7, 7},   {8, 6},   {9, 6},
				{10, 7},  {11, 7},  {12, 9},  {13, 9},  {15, 8},  {14, 7},  {17, 6},  {16, 5},  {18, 9},  {42, 9},
				{32, 8},  {19, 7},  {20, 6},  {21, 7},  {22, 7},  {27, 6},  {25, 5},  {23, 3},  {24, 3},  {26, 5},
				{28, 6},  {29, 6},  {30, 7},  {31, 7},  {33, 8},  {34, 8},  {35, 8},  {41, 8},  {36, 7},  {38, 7},
				{37, 6},  {39, 7},  {43, 14}, {44, 14}, {45, 14}, {50, 14}, {51, 14}, {53, 14}, {54, 13}, {55, 13},
				{56, 13}, {58, 13}, {64, 13}, {67, 13}, {68, 13}, {69, 13}, {70, 13}, {71, 13}, {72, 13}, {74, 13},
				{76, 13}, {47, 9},  {49, 8},  {40, 6},  {46, 7},  {48, 7},  {52, 8},  {57, 8},  {60, 7},  {59, 6},
				{61, 9},  {75, 9},  {73, 8},  {63, 7},  {62, 6},  {65, 7},  {66, 7},
			},
			{
				/* AC highquant table group 1, table 4 */
				{0, 2},   {1, 4},   {2, 5},   {3, 5},   {4, 6},   {5, 6},   {6, 7},   {7, 7},   {8, 7},   {9, 8},
				{10, 9},  {11, 9},  {12, 10}, {13, 10}, {14, 10}, {15, 15}, {50, 15}, {51, 15}, {54, 15}, {55, 15},
				{56, 15}, {61, 15}, {64, 15}, {45, 12}, {47, 12}, {58, 12}, {32, 8},  {19, 7},  {18, 6},  {17, 5},
				{16, 3},  {20, 4},  {21, 6},  {22, 6},  {24, 5},  {23, 4},  {25, 6},  {26, 6},  {27, 6},  {28, 7},
				{29, 7},  {30, 7},  {31, 7},  {33, 8},  {34, 8},  {35, 8},  {36, 8},  {37, 8},  {38, 10}, {43, 10},
				{41, 9},  {39, 7},  {40, 8},  {42, 9},  {44, 10}, {46, 10}, {49, 8},  {53, 9},  {59, 10}, {60, 10},
				{48, 6},  {52, 6},  {57, 7},  {62, 12}, {67, 15}, {69, 15}, {70, 15}, {71, 15}, {72, 14}, {76, 14},
				{63, 11}, {68, 12}, {74, 12}, {75, 11}, {66, 9},  {65, 8},  {73, 6},
			},
			{
				/* AC highquant table group 1, table 5 */
				{0, 3},   {1, 4},   {2, 4},   {3, 5},   {4, 5},   {5, 6},   {6, 7},   {7, 8},   {8, 11},  {10, 11},
				{11, 10}, {9, 9},   {12, 11}, {13, 15}, {14, 15}, {15, 15}, {47, 15}, {49, 15}, {50, 15}, {51, 15},
				{54, 15}, {55, 14}, {56, 14}, {58, 14}, {63, 14}, {38, 11}, {53, 11}, {37, 9},  {32, 8},  {19, 7},
				{18, 6},  {16, 4},  {17, 5},  {20, 5},  {21, 7},  {31, 7},  {22, 6},  {25, 5},  {24, 4},  {23, 3},
				{26, 5},  {27, 5},  {28, 5},  {29, 6},  {30, 6},  {33, 8},  {34, 8},  {35, 7},  {36, 8},  {41, 8},
				{40, 7},  {39, 5},  {42, 8},  {44, 10}, {60, 10}, {45, 9},  {43, 7},  {46, 9},  {59, 9},  {61, 10},
				{64, 14}, {67, 14}, {68, 14}, {69, 14}, {70, 14}, {72, 14}, {75, 14}, {76, 14}, {66, 11}, {62, 9},
				{48, 7},  {52, 7},  {57, 7},  {65, 8},  {71, 9},  {74, 9},  {73, 7},
			},
			{
				/* AC highquant table group 1, table 6 */
				{0, 3},   {1, 4},   {2, 4},   {3, 5},   {4, 5},   {5, 5},   {6, 5},   {7, 6},   {8, 7},   {10, 7},
				{9, 5},   {11, 13}, {13, 13}, {15, 13}, {29, 13}, {32, 13}, {33, 13}, {34, 12}, {37, 12}, {38, 12},
				{40, 12}, {41, 12}, {42, 12}, {43, 12}, {44, 12}, {45, 12}, {50, 12}, {53, 12}, {54, 12}, {55, 12},
				{56, 12}, {58, 12}, {60, 12}, {61, 12}, {62, 12}, {63, 12}, {64, 12}, {66, 12}, {67, 12}, {68, 12},
				{69, 12}, {70, 12}, {71, 12}, {72, 12}, {74, 12}, {76, 12}, {12, 7},  {14, 7},  {18, 7},  {17, 6},
				{19, 7},  {21, 7},  {16, 4},  {20, 5},  {22, 5},  {23, 6},  {24, 6},  {25, 7},  {27, 7},  {26, 6},
				{28, 7},  {30, 7},  {31, 7},  {35, 7},  {36, 7},  {39, 7},  {47, 6},  {46, 5},  {49, 5},  {48, 4},
				{51, 7},  {75, 7},  {57, 6},  {52, 5},  {59, 6},  {65, 6},  {73, 5},
			},
			{
				/* AC highquant table group 1, table 7 */
				{0, 3},   {1, 4},   {2, 5},   {3, 6},   {4, 6},   {5, 8},   {6, 8},   {7, 8},   {8, 11},  {9, 15},
				{10, 15}, {11, 15}, {12, 15}, {13, 15}, {14, 15}, {15, 15}, {38, 15}, {46, 15}, {47, 15}, {50, 15},
				{51, 15}, {53, 15}, {54, 15}, {55, 15}, {56, 15}, {37, 11}, {58, 15}, {61, 15}, {62, 15}, {63, 15},
				{64, 15}, {66, 15}, {67, 15}, {68, 15}, {69, 15}, {70, 15}, {72, 14}, {75, 14}, {76, 14}, {19, 9},
				{17, 6},  {18, 7},  {21, 8},  {22, 8},  {28, 6},  {16, 4},  {20, 5},  {25, 5},  {26, 5},  {27, 5},
				{23, 2},  {24, 3},  {29, 6},  {30, 8},  {31, 8},  {32, 9},  {36, 9},  {33, 8},  {34, 7},  {35, 8},
				{41, 8},  {40, 7},  {42, 9},  {44, 9},  {43, 8},  {39, 5},  {45, 9},  {49, 10}, {71, 10}, {48, 8},
				{52, 8},  {57, 9},  {60, 9},  {59, 8},  {65, 9},  {74, 9},  {73, 7},
			},
		},
	},
	{
		{
			{
				/* AC lowquant table group 0, table 0 */
				{0, 2},   {1, 3},   {2, 4},   {3, 5},   {4, 6},   {5, 7},   {6, 7},   {7, 7},   {8, 9},   {12, 10},
				{13, 10}, {9, 8},   {10, 8},  {11, 8},  {14, 10}, {15, 10}, {18, 9},  {19, 10}, {41, 10}, {40, 9},
				{17, 7},  {21, 7},  {20, 6},  {16, 4},  {22, 7},  {32, 7},  {28, 6},  {25, 5},  {23, 4},  {24, 4},
				{26, 5},  {27, 5},  {29, 6},  {30, 6},  {31, 6},  {33, 7},  {34, 7},  {35, 7},  {36, 7},  {37, 7},
				{38, 7},  {39, 8},  {42, 14}, {43, 14}, {44, 14}, {45, 14}, {47, 14}, {49, 14}, {50, 14}, {51, 14},
				{53, 14}, {54, 14}, {55, 14}, {56, 14}, {58, 14}, {63, 14}, {64, 14}, {65, 14}, {46, 10}, {48, 9},
				{52, 8},  {57, 10}, {66, 14}, {67, 14}, {68, 14}, {69, 14}, {70, 14}, {71, 14}, {72, 13}, {73, 13},
				{74, 13}, {75, 13}, {76, 13}, {62, 9},  {59, 7},  {60, 8},  {61, 8},
			},
			{
				/* AC lowquant table group 0, table 1 */
				{0, 3},   {1, 5},   {2, 7},   {5, 8},   {6, 8},   {3, 6},   {4, 6},   {7, 8},   {8, 14},  {9, 14},
				{11, 14}, {12, 14}, {13, 14}, {14, 14}, {15, 14}, {18, 14}, {20, 14}, {21, 14}, {22, 14}, {41, 14},
				{45, 14}, {46, 14}, {47, 14}, {49, 14}, {50, 14}, {51, 14}, {52, 13}, {53, 13}, {54, 13}, {55, 13},
				{56, 13}, {57, 13}, {58, 13}, {60, 13}, {61, 13}, {62, 13}, {63, 13}, {64, 13}, {66, 13}, {67, 13},
				{68, 13}, {69, 13}, {70, 13}, {71, 13}, {72, 13}, {73, 13}, {74, 13}, {75, 13}, {76, 13}, {10, 7},
				{16, 6},  {17, 8},  {19, 8},  {30, 7},  {23, 2},  {24, 3},  {25, 5},  {29, 6},  {31, 7},  {32, 7},
				{26, 4},  {27, 4},  {28, 4},  {33, 7},  {34, 8},  {40, 8},  {35, 7},  {42, 8},  {43, 8},  {36, 6},
				{37, 6},  {38, 6},  {39, 6},  {44, 8},  {48, 8},  {65, 7},  {59, 6},
			},
			{
				/* AC lowquant table group 0, table 2 */
				{0, 2},   {1, 3},   {2, 3},   {3, 4},   {4, 5},   {5, 5},   {6, 6},   {7, 6},   {8, 7},   {9, 7},
				{10, 7},  {11, 8},  {12, 8},  {13, 8},  {14, 9},  {15, 10}, {39, 16}, {40, 16}, {41, 15}, {43, 15},
				{44, 15}, {45, 15}, {50, 15}, {51, 15}, {53, 15}, {42, 12}, {49, 12}, {54, 15}, {55, 15}, {56, 15},
				{58, 15}, {64, 15}, {65, 15}, {66, 15}, {67, 15}, {18, 8},  {19, 9},  {21, 9},  {17, 6},  {20, 6},
				{22, 8},  {38, 8},  {27, 7},  {16, 4},  {23, 5},  {24, 6},  {25, 6},  {26, 6},  {28, 7},  {29, 7},
				{30, 7},  {31, 7},  {32, 7},  {33, 7},  {34, 7},  {35, 7},  {36, 7},  {37, 7},  {46, 8},  {47, 10},
				{62, 11}, {63, 12}, {68, 15}, {69, 15}, {70, 15}, {71, 15}, {72, 15}, {74, 15}, {75, 15}, {76, 15},
				{48, 9},  {52, 8},  {57, 9},  {73, 9},  {59, 7},  {60, 8},  {61, 8},
			},
			{
				/* AC lowquant table group 0, table 3 */
				{0, 2},   {1, 3},   {2, 4},   {3, 5},   {4, 5},   {5, 6},   {6, 6},   {7, 7},   {8, 7},   {9, 8},
				{10, 8},  {11, 9},  {12, 9},  {13, 9},  {14, 10}, {15, 10}, {17, 5},  {18, 7},  {19, 7},  {21, 7},
				{24, 7},  {16, 3},  {20, 4},  {22, 5},  {23, 6},  {25, 7},  {26, 7},  {27, 8},  {28, 8},  {29, 8},
				{30, 8},  {31, 8},  {32, 8},  {33, 9},  {37, 9},  {34, 8},  {35, 8},  {36, 8},  {38, 11}, {40, 11},
				{39, 10}, {41, 15}, {43, 15}, {44, 15}, {45, 15}, {50, 15}, {51, 15}, {64, 15}, {66, 15}, {49, 12},
				{47, 11}, {42, 10}, {46, 9},  {55, 10}, {56, 11}, {63, 12}, {67, 15}, {68, 15}, {69, 15}, {70, 15},
				{71, 15}, {72, 15}, {74, 14}, {48, 7},  {53, 7},  {52, 5},  {54, 8},  {58, 10}, {61, 10}, {62, 10},
				{65, 11}, {75, 12}, {76, 12}, {57, 7},  {59, 8},  {60, 8},  {73, 7},
			},
			{
				/* AC lowquant table group 0, table 4 */
				{0, 3},   {1, 4},   {2, 4},   {3, 5},   {4, 6},   {5, 6},   {6, 6},   {7, 7},   {9, 7},   {8, 6},
				{10, 8},  {11, 8},  {12, 8},  {13, 9},  {14, 9},  {15, 8},  {37, 8},  {31, 7},  {18, 6},  {17, 5},
				{16, 4},  {19, 6},  {21, 6},  {22, 5},  {20, 4},  {23, 5},  {24, 6},  {25, 6},  {26, 6},  {27, 6},
				{28, 6},  {29, 6},  {30, 6},  {32, 7},  {33, 7},  {34, 7},  {35, 7},  {36, 7},  {38, 11}, {50, 15},
				{51, 15}, {55, 15}, {56, 15}, {64, 15}, {69, 15}, {70, 14}, {71, 14}, {72, 14}, {75, 14}, {76, 14},
				{45, 10}, {42, 9},  {41, 8},  {39, 7},  {40, 7},  {43, 9},  {44, 9},  {46, 9},  {47, 10}, {58, 10},
				{49, 8},  {54, 9},  {62, 10}, {66, 10}, {48, 6},  {53, 7},  {59, 7},  {52, 4},  {57, 6},  {60, 8},
				{61, 8},  {63, 9},  {65, 9},  {67, 10}, {68, 10}, {74, 9},  {73, 5},
			},
			{
				/* AC lowquant table group 0, table 5 */
				{0, 2},   {1, 4},   {2, 5},   {3, 5},   {4, 6},   {5, 6},   {6, 7},   {7, 8},   {8, 8},   {9, 8},
				{10, 9},  {11, 11}, {14, 11}, {13, 10}, {12, 8},  {15, 15}, {42, 15}, {50, 15}, {51, 15}, {62, 15},
				{64, 15}, {67, 15}, {68, 15}, {69, 14}, {70, 14}, {71, 14}, {72, 14}, {38, 11}, {40, 10}, {33, 9},
				{17, 5},  {18, 6},  {19, 7},  {24, 7},  {16, 3},  {20, 4},  {21, 6},  {23, 6},  {22, 5},  {25, 8},
				{27, 8},  {26, 7},  {28, 7},  {29, 8},  {31, 8},  {30, 7},  {32, 8},  {34, 9},  {35, 9},  {36, 9},
				{37, 9},  {39, 9},  {41, 11}, {44, 11}, {43, 10}, {45, 11}, {47, 11}, {46, 10}, {56, 9},  {49, 8},
				{48, 6},  {54, 7},  {55, 7},  {53, 5},  {52, 4},  {57, 6},  {58, 8},  {59, 8},  {60, 10}, {61, 11},
				{75, 11}, {63, 10}, {65, 10}, {66, 10}, {74, 10}, {76, 9},  {73, 5},
			},
			{
				/* AC lowquant table group 0, table 6 */
				{0, 2},   {1, 4},   {2, 5},   {3, 6},   {4, 6},   {5, 8},   {6, 9},   {7, 10},  {8, 10},  {9, 9},
				{10, 15}, {11, 15}, {13, 15}, {15, 15}, {37, 15}, {46, 15}, {47, 15}, {49, 15}, {50, 15}, {51, 15},
				{53, 15}, {54, 15}, {55, 15}, {56, 15}, {58, 15}, {60, 15}, {61, 15}, {62, 15}, {63, 15}, {64, 15},
				{67, 15}, {68, 15}, {69, 15}, {70, 15}, {71, 15}, {72, 15}, {74, 14}, {75, 14}, {76, 14}, {12, 10},
				{14, 10}, {36, 10}, {21, 9},  {18, 8},  {19, 8},  {22, 8},  {32, 8},  {17, 5},  {16, 4},  {20, 5},
				{26, 5},  {25, 4},  {23, 3},  {24, 3},  {27, 6},  {28, 6},  {29, 7},  {30, 7},  {31, 7},  {33, 8},
				{34, 9},  {38, 9},  {35, 8},  {41, 9},  {42, 10}, {45, 10}, {43, 8},  {44, 8},  {39, 6},  {40, 6},
				{48, 9},  {57, 9},  {52, 8},  {59, 9},  {65, 10}, {66, 10}, {73, 8},
			},
			{
				/* AC lowquant table group 0, table 7 */
				{0, 4},   {1, 11},  {2, 11},  {3, 11},  {4, 11},  {5, 11},  {6, 11},  {7, 11},  {8, 11},  {9, 11},
				{10, 11}, {11, 11}, {12, 11}, {13, 11}, {14, 11}, {15, 11}, {16, 11}, {17, 10}, {18, 10}, {19, 10},
				{20, 10}, {21, 10}, {22, 10}, {25, 10}, {28, 10}, {29, 10}, {30, 10}, {31, 10}, {32, 10}, {33, 10},
				{34, 10}, {35, 10}, {36, 10}, {37, 10}, {38, 10}, {39, 10}, {40, 10}, {41, 10}, {42, 10}, {43, 10},
				{44, 10}, {45, 10}, {46, 10}, {47, 10}, {48, 10}, {49, 10}, {50, 10}, {51, 10}, {52, 10}, {53, 10},
				{54, 10}, {55, 10}, {56, 10}, {57, 10}, {58, 10}, {59, 10}, {60, 10}, {61, 10}, {62, 10}, {63, 10},
				{64, 10}, {65, 10}, {66, 10}, {67, 10}, {68, 10}, {69, 10}, {70, 10}, {71, 10}, {72, 10}, {73, 10},
				{74, 10}, {75, 10}, {76, 10}, {26, 4},  {27, 4},  {24, 2},  {23, 1},
			},
		},
		{
			{
				/* AC lowquant table group 1, table 0 */
				{0, 3},   {1, 4},   {2, 4},   {3, 5},   {4, 6},   {5, 7},   {7, 8},   {8, 8},   {6, 6},   {9, 9},
				{10, 10}, {14, 15}, {47, 15}, {49, 14}, {50, 14}, {51, 14}, {54, 14}, {55, 14}, {56, 14}, {58, 14},
				{15, 11}, {11, 9},  {12, 9},  {13, 9},  {37, 9},  {32, 8},  {17, 5},  {16, 4},  {18, 6},  {19, 7},
				{21, 7},  {20, 5},  {22, 5},  {25, 5},  {24, 4},  {23, 3},  {26, 5},  {27, 5},  {28, 5},  {29, 6},
				{30, 6},  {31, 7},  {33, 8},  {34, 8},  {35, 8},  {36, 8},  {38, 10}, {44, 11}, {46, 11}, {45, 10},
				{53, 10}, {41, 8},  {39, 6},  {40, 7},  {42, 8},  {43, 8},  {48, 7},  {57, 7},  {52, 6},  {59, 8},
				{60, 9},  {61, 11}, {63, 11}, {62, 10}, {64, 14}, {67, 14}, {68, 14}, {69, 14}, {70, 14}, {71, 14},
				{72, 14}, {76, 14}, {75, 11}, {66, 10}, {74, 9},  {65, 8},  {73, 6},
			},
			{
				/* AC lowquant table group 1, table 1 */
				{0, 2},   {1, 3},   {2, 4},   {3, 5},   {4, 6},   {5, 6},   {6, 7},   {7, 8},   {8, 8},   {9, 8},
				{10, 9},  {11, 9},  {12, 11}, {13, 11}, {14, 11}, {15, 11}, {24, 9},  {23, 8},  {17, 5},  {18, 6},
				{19, 7},  {25, 10}, {27, 10}, {26, 9},  {28, 10}, {29, 12}, {35, 12}, {34, 11}, {30, 10}, {31, 10},
				{21, 6},  {32, 10}, {33, 10}, {36, 10}, {37, 12}, {38, 12}, {39, 15}, {40, 15}, {41, 15}, {42, 15},
				{43, 14}, {44, 14}, {45, 14}, {51, 14}, {61, 14}, {64, 14}, {46, 11}, {47, 11}, {49, 10}, {50, 12},
				{63, 12}, {59, 11}, {56, 10}, {48, 8},  {55, 8},  {16, 3},  {20, 4},  {22, 4},  {52, 4},  {53, 6},
				{54, 7},  {58, 8},  {60, 10}, {62, 11}, {65, 14}, {66, 14}, {67, 14}, {68, 14}, {69, 14}, {70, 14},
				{71, 14}, {72, 14}, {74, 11}, {75, 11}, {76, 10}, {57, 6},  {73, 6},
			},
			{
				/* AC lowquant table group 1, table 2 */
				{0, 3},   {1, 4},   {2, 5},   {3, 6},   {4, 7},   {5, 8},   {6, 8},   {7, 8},   {8, 10},  {10, 10},
				{9, 9},   {11, 10}, {12, 16}, {15, 16}, {38, 16}, {44, 16}, {46, 15}, {47, 15}, {50, 15}, {51, 15},
				{59, 15}, {60, 15}, {61, 15}, {62, 15}, {64, 15}, {68, 15}, {69, 15}, {70, 15}, {71, 15}, {72, 15},
				{37, 11}, {13, 10}, {14, 10}, {19, 8},  {18, 6},  {17, 5},  {21, 5},  {23, 7},  {24, 9},  {25, 9},
				{26, 9},  {27, 9},  {28, 9},  {29, 10}, {30, 10}, {31, 10}, {32, 10}, {33, 10}, {34, 10}, {35, 9},
				{36, 10}, {39, 10}, {40, 11}, {42, 11}, {41, 10}, {43, 10}, {45, 11}, {63, 11}, {16, 3},  {20, 3},
				{22, 4},  {48, 7},  {49, 9},  {65, 11}, {66, 11}, {67, 11}, {74, 11}, {56, 8},  {54, 6},  {55, 6},
				{58, 7},  {75, 8},  {76, 8},  {52, 3},  {53, 4},  {57, 5},  {73, 5},
			},
			{
				/* AC lowquant table group 1, table 3 */
				{0, 2},   {1, 3},   {2, 4},   {3, 4},   {4, 5},   {5, 5},   {6, 6},   {7, 6},   {8, 6},   {9, 6},
				{10, 7},  {11, 7},  {12, 8},  {13, 8},  {14, 8},  {15, 9},  {19, 9},  {17, 6},  {18, 7},  {21, 8},
				{36, 8},  {16, 4},  {20, 6},  {22, 7},  {28, 7},  {23, 6},  {24, 6},  {25, 6},  {26, 6},  {27, 6},
				{29, 7},  {30, 7},  {31, 7},  {32, 7},  {33, 7},  {34, 7},  {35, 7},  {37, 8},  {38, 9},  {39, 11},
				{41, 15}, {42, 15}, {43, 15}, {44, 15}, {45, 15}, {51, 15}, {54, 15}, {55, 15}, {49, 12}, {40, 10},
				{46, 7},  {47, 8},  {48, 8},  {50, 9},  {53, 9},  {56, 15}, {67, 15}, {68, 15}, {69, 15}, {70, 14},
				{71, 14}, {72, 14}, {74, 14}, {75, 14}, {76, 14}, {58, 11}, {65, 11}, {66, 11}, {64, 9},  {52, 7},
				{57, 8},  {61, 8},  {60, 7},  {59, 6},  {62, 8},  {73, 8},  {63, 7},
			},
			{
				/* AC lowquant table group 1, table 4 */
				{0, 2},   {1, 4},   {2, 5},   {3, 6},   {4, 6},   {5, 7},   {6, 7},   {7, 8},   {8, 8},   {9, 8},
				{10, 9},  {11, 9},  {12, 10}, {13, 10}, {14, 14}, {69, 14}, {70, 14}, {72, 14}, {15, 12}, {44, 11},
				{37, 10}, {29, 8},  {19, 7},  {18, 6},  {17, 5},  {21, 6},  {24, 6},  {16, 3},  {20, 4},  {22, 5},
				{23, 5},  {25, 6},  {26, 6},  {27, 7},  {28, 7},  {30, 7},  {31, 8},  {32, 8},  {33, 8},  {34, 8},
				{35, 8},  {36, 9},  {38, 10}, {43, 10}, {39, 7},  {40, 8},  {41, 9},  {42, 9},  {45, 9},  {46, 9},
				{47, 10}, {50, 12}, {51, 12}, {62, 11}, {56, 9},  {48, 7},  {49, 8},  {54, 8},  {55, 8},  {58, 8},
				{52, 4},  {53, 6},  {57, 6},  {59, 9},  {60, 10}, {61, 10}, {63, 10}, {64, 11}, {67, 11}, {65, 9},
				{66, 9},  {68, 11}, {71, 11}, {76, 10}, {74, 9},  {75, 9},  {73, 6},
			},
			{
				/* AC lowquant table group 1, table 5 */
				{0, 2},   {1, 3},   {2, 4},   {3, 5},   {4, 5},   {5, 6},   {6, 6},   {7, 7},   {8, 7},   {9, 7},
				{10, 8},  {11, 8},  {12, 8},  {13, 9},  {14, 10}, {39, 11}, {40, 12}, {41, 12}, {15, 9},  {37, 9},
				{19, 8},  {18, 7},  {21, 7},  {17, 5},  {16, 3},  {20, 5},  {22, 6},  {24, 6},  {23, 5},  {25, 7},
				{26, 7},  {27, 7},  {28, 8},  {29, 8},  {30, 7},  {31, 8},  {32, 8},  {33, 8},  {34, 8},  {35, 8},
				{36, 8},  {38, 9},  {42, 15}, {44, 15}, {45, 15}, {56, 15}, {43, 13}, {51, 13}, {66, 13}, {58, 11},
				{49, 10}, {46, 8},  {47, 9},  {50, 9},  {48, 8},  {53, 7},  {54, 9},  {55, 9},  {59, 8},  {52, 5},
				{57, 7},  {60, 8},  {61, 8},  {62, 9},  {64, 11}, {65, 12}, {67, 15}, {68, 15}, {69, 15}, {70, 15},
				{71, 15}, {72, 15}, {74, 14}, {75, 11}, {76, 11}, {63, 8},  {73, 7},
			},
			{
				/* AC lowquant table group 1, table 6 */
				{0, 3},   {1, 4},   {2, 4},   {3, 5},   {4, 5},   {5, 6},   {6, 6},   {7, 7},   {8, 8},   {9, 8},
				{10, 8},  {11, 9},  {12, 9},  {13, 9},  {14, 11}, {15, 12}, {45, 12}, {38, 10}, {33, 8},  {16, 3},
				{17, 5},  {18, 6},  {19, 7},  {27, 7},  {20, 4},  {21, 6},  {25, 6},  {22, 5},  {23, 4},  {24, 5},
				{26, 6},  {28, 7},  {29, 7},  {30, 7},  {31, 7},  {32, 7},  {34, 8},  {35, 8},  {36, 9},  {41, 9},
				{37, 8},  {39, 7},  {40, 8},  {42, 11}, {47, 15}, {50, 15}, {51, 15}, {56, 15}, {64, 15}, {67, 15},
				{68, 15}, {69, 15}, {70, 14}, {71, 14}, {72, 14}, {76, 14}, {44, 10}, {43, 9},  {46, 9},  {54, 9},
				{55, 11}, {63, 11}, {61, 10}, {58, 9},  {48, 6},  {49, 7},  {53, 7},  {52, 5},  {57, 6},  {59, 8},
				{60, 9},  {62, 10}, {74, 10}, {65, 8},  {66, 9},  {75, 9},  {73, 5},
			},
			{
				/* AC lowquant table group 1, table 7 */
				{0, 2},   {1, 4},   {2, 5},   {3, 6},   {4, 7},   {5, 7},   {6, 8},   {7, 8},   {8, 9},   {9, 9},
				{10, 9},  {11, 10}, {12, 15}, {13, 15}, {14, 15}, {15, 15}, {32, 15}, {36, 15}, {38, 15}, {41, 15},
				{42, 15}, {45, 15}, {47, 15}, {50, 15}, {51, 15}, {61, 15}, {62, 14}, {34, 11}, {18, 6},  {17, 5},
				{19, 7},  {23, 8},  {24, 8},  {21, 6},  {25, 9},  {26, 10}, {28, 10}, {27, 9},  {29, 9},  {30, 10},
				{31, 10}, {33, 10}, {35, 11}, {40, 11}, {37, 10}, {43, 11}, {44, 11}, {39, 9},  {46, 11}, {60, 11},
				{49, 10}, {56, 9},  {59, 9},  {63, 14}, {64, 14}, {67, 14}, {68, 14}, {69, 14}, {70, 14}, {71, 14},
				{72, 14}, {74, 11}, {76, 10}, {48, 7},  {16, 3},  {20, 4},  {22, 4},  {52, 3},  {53, 5},  {54, 7},
				{55, 7},  {58, 7},  {65, 9},  {66, 9},  {75, 8},  {57, 5},  {73, 5},
			},
		},
	},
};
#define MAX_AC_VLC_BITS 16

#define W0 2048
#define W1 2841 /* 2048*sqrt (2)*cos (1*pi/16) */
#define W2 2676 /* 2048*sqrt (2)*cos (2*pi/16) */
#define W3 2408 /* 2048*sqrt (2)*cos (3*pi/16) */
#define W4 2048 /* 2048*sqrt (2)*cos (4*pi/16) */
#define W5 1609 /* 2048*sqrt (2)*cos (5*pi/16) */
#define W6 1108 /* 2048*sqrt (2)*cos (6*pi/16) */
#define W7 565  /* 2048*sqrt (2)*cos (7*pi/16) */

static void wmv2_idct_row(short *b) {
	int s1, s2;
	int a0, a1, a2, a3, a4, a5, a6, a7;

	/* step 1 */
	a1 = W1 * b[1] + W7 * b[7];
	a7 = W7 * b[1] - W1 * b[7];
	a5 = W5 * b[5] + W3 * b[3];
	a3 = W3 * b[5] - W5 * b[3];
	a2 = W2 * b[2] + W6 * b[6];
	a6 = W6 * b[2] - W2 * b[6];
	a0 = W0 * b[0] + W0 * b[4];
	a4 = W0 * b[0] - W0 * b[4];

	/* step 2 */
	s1 = (int)(181U * (a1 - a5 + a7 - a3) + 128) >> 8; // 1, 3, 5, 7
	s2 = (int)(181U * (a1 - a5 - a7 + a3) + 128) >> 8;

	/* step 3 */
	b[0] = (a0 + a2 + a1 + a5 + (1 << 7)) >> 8;
	b[1] = (a4 + a6 + s1 + (1 << 7)) >> 8;
	b[2] = (a4 - a6 + s2 + (1 << 7)) >> 8;
	b[3] = (a0 - a2 + a7 + a3 + (1 << 7)) >> 8;
	b[4] = (a0 - a2 - a7 - a3 + (1 << 7)) >> 8;
	b[5] = (a4 - a6 - s2 + (1 << 7)) >> 8;
	b[6] = (a4 + a6 - s1 + (1 << 7)) >> 8;
	b[7] = (a0 + a2 - a1 - a5 + (1 << 7)) >> 8;
}

static void wmv2_idct_col(short *b) {
	int s1, s2;
	int a0, a1, a2, a3, a4, a5, a6, a7;

	/* step 1, with extended precision */
	a1 = (W1 * b[8 * 1] + W7 * b[8 * 7] + 4) >> 3;
	a7 = (W7 * b[8 * 1] - W1 * b[8 * 7] + 4) >> 3;
	a5 = (W5 * b[8 * 5] + W3 * b[8 * 3] + 4) >> 3;
	a3 = (W3 * b[8 * 5] - W5 * b[8 * 3] + 4) >> 3;
	a2 = (W2 * b[8 * 2] + W6 * b[8 * 6] + 4) >> 3;
	a6 = (W6 * b[8 * 2] - W2 * b[8 * 6] + 4) >> 3;
	a0 = (W0 * b[8 * 0] + W0 * b[8 * 4]) >> 3;
	a4 = (W0 * b[8 * 0] - W0 * b[8 * 4]) >> 3;

	/* step 2 */
	s1 = (int)(181U * (a1 - a5 + a7 - a3) + 128) >> 8;
	s2 = (int)(181U * (a1 - a5 - a7 + a3) + 128) >> 8;

	/* step 3 */
	b[8 * 0] = (a0 + a2 + a1 + a5 + (1 << 13)) >> 14;
	b[8 * 1] = (a4 + a6 + s1 + (1 << 13)) >> 14;
	b[8 * 2] = (a4 - a6 + s2 + (1 << 13)) >> 14;
	b[8 * 3] = (a0 - a2 + a7 + a3 + (1 << 13)) >> 14;

	b[8 * 4] = (a0 - a2 - a7 - a3 + (1 << 13)) >> 14;
	b[8 * 5] = (a4 - a6 - s2 + (1 << 13)) >> 14;
	b[8 * 6] = (a4 + a6 - s1 + (1 << 13)) >> 14;
	b[8 * 7] = (a0 + a2 - a1 - a5 + (1 << 13)) >> 14;
}

static void wmv2_idct_add_c(uint8 *dest, ptrdiff_t line_size, int16 *block) {
	int i;

	for (i = 0; i < 64; i += 8)
		wmv2_idct_row(block + i);
	for (i = 0; i < 8; i++)
		wmv2_idct_col(block + i);

	for (i = 0; i < 8; i++) {
		dest[0] = av_clip_uint8(dest[0] + block[0]);
		dest[1] = av_clip_uint8(dest[1] + block[1]);
		dest[2] = av_clip_uint8(dest[2] + block[2]);
		dest[3] = av_clip_uint8(dest[3] + block[3]);
		dest[4] = av_clip_uint8(dest[4] + block[4]);
		dest[5] = av_clip_uint8(dest[5] + block[5]);
		dest[6] = av_clip_uint8(dest[6] + block[6]);
		dest[7] = av_clip_uint8(dest[7] + block[7]);
		dest += line_size;
		block += 8;
	}
}

static const VLC *makeCanonicalVLC(int count, const uint8 table[][2]) {
	uint32 codes[256], lengths[256];
	uint64 code = 0;
	for (int i = 0; i < count; ++i) {
		int n = table[i][1];
		codes[i] = code >> (32 - n);
		lengths[i] = n;
		code += uint64(1) << (32 - n);
	}
	VLC *v = const_cast<VLC *>(makeVLC(count, lengths, 4, 4, codes, 4, 4));
	for (uint i = 0; i < v->nodes.size(); ++i)
		if (v->nodes[i].symbol >= 0)
			v->nodes[i].symbol = table[v->nodes[i].symbol][0];
	return v;
}
#define VLC_BUFFER_SIZE 28150

#define MAX_TABLE_DEPTH(table_bits, max_bits) ((max_bits + table_bits - 1) / table_bits)

#define DC_VLC_BITS 9
#define OR_VLC_BITS 7

#define DC_VLC_MTD MAX_TABLE_DEPTH(DC_VLC_BITS, MAX_DC_VLC_BITS)
#define AC_VLC_MTD MAX_TABLE_DEPTH(AC_VLC_BITS, MAX_AC_VLC_BITS)
#define OR_VLC_MTD MAX_TABLE_DEPTH(OR_VLC_BITS, MAX_OR_VLC_BITS)

static const VLC *j_ac_vlc[2][2][8];  // [quant < 13], [intra / inter], [select]
static const VLC *j_dc_vlc[2][8];     // [quant], [select]
static const VLC *j_orient_vlc[2][4]; // [quant], [select]

static const VLC *x8_init_vlc(int nb_bits, int nb_codes, const uint8 table[][2]) {
	return makeCanonicalVLC(nb_codes, table);
}

static void x8_vlc_init(void) {

	int i;

	// set ac tables
	for (i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			for (int k = 0; k < 8; k++)
				j_ac_vlc[i][j][k] = x8_init_vlc(AC_VLC_BITS, 77, x8_ac_quant_table[i][j][k]);

	// set dc tables
	for (i = 0; i < 2; i++)
		for (int j = 0; j < 8; j++)
			j_dc_vlc[i][j] = x8_init_vlc(DC_VLC_BITS, 34, x8_dc_quant_table[i][j]);

	// set orient tables
	for (i = 0; i < 2; i++)
		j_orient_vlc[0][i] = x8_init_vlc(OR_VLC_BITS, 12, x8_orient_highquant_table[i]);
	for (i = 0; i < 4; i++)
		j_orient_vlc[1][i] = x8_init_vlc(OR_VLC_BITS, 12, x8_orient_lowquant_table[i]);
}

static void x8_reset_vlc_tables(IntraX8Context *w) {
	memset(w->j_dc_vlc_table, 0, sizeof(w->j_dc_vlc_table));
	memset(w->j_ac_vlc_table, 0, sizeof(w->j_ac_vlc_table));
	w->j_orient_vlc_table = NULL;
}

static inline void x8_select_ac_table(IntraX8Context *const w, int mode) {
	int table_index;

	assert(mode < 4);

	if (w->j_ac_vlc_table[mode])
		return;

	table_index = get_bits(w->gb, 3);
	// 2 modes use same tables
	w->j_ac_vlc_table[mode] = j_ac_vlc[w->quant < 13][mode >> 1][table_index];
	assert(w->j_ac_vlc_table[mode]);
}

static inline int x8_get_orient_vlc(IntraX8Context *w) {
	if (!w->j_orient_vlc_table) {
		int table_index = get_bits(w->gb, 1 + (w->quant < 13));
		w->j_orient_vlc_table = j_orient_vlc[w->quant < 13][table_index];
	}

	return get_vlc2(w->gb, w->j_orient_vlc_table);
}

#define extra_bits(eb) (eb)         // 3 bits
#define extra_run (0xFF << 8)       // 1 bit
#define extra_level (0x00 << 8)     // 1 bit
#define run_offset(r) ((r) << 16)   // 6 bits
#define level_offset(l) ((l) << 24) // 5 bits
static const uint32 ac_decode_table[] = {
	/* 46 */ extra_bits(3) | extra_run | run_offset(16) | level_offset(0),
	/* 47 */ extra_bits(3) | extra_run | run_offset(24) | level_offset(0),
	/* 48 */ extra_bits(2) | extra_run | run_offset(4) | level_offset(1),
	/* 49 */ extra_bits(3) | extra_run | run_offset(8) | level_offset(1),

	/* 50 */ extra_bits(5) | extra_run | run_offset(32) | level_offset(0),
	/* 51 */ extra_bits(4) | extra_run | run_offset(16) | level_offset(1),

	/* 52 */ extra_bits(2) | extra_level | run_offset(0) | level_offset(4),
	/* 53 */ extra_bits(2) | extra_level | run_offset(0) | level_offset(8),
	/* 54 */ extra_bits(2) | extra_level | run_offset(0) | level_offset(12),
	/* 55 */ extra_bits(3) | extra_level | run_offset(0) | level_offset(16),
	/* 56 */ extra_bits(3) | extra_level | run_offset(0) | level_offset(24),

	/* 57 */ extra_bits(2) | extra_level | run_offset(1) | level_offset(3),
	/* 58 */ extra_bits(3) | extra_level | run_offset(1) | level_offset(7),

	/* 59 */ extra_bits(2) | extra_run | run_offset(16) | level_offset(0),
	/* 60 */ extra_bits(2) | extra_run | run_offset(20) | level_offset(0),
	/* 61 */ extra_bits(2) | extra_run | run_offset(24) | level_offset(0),
	/* 62 */ extra_bits(2) | extra_run | run_offset(28) | level_offset(0),
	/* 63 */ extra_bits(4) | extra_run | run_offset(32) | level_offset(0),
	/* 64 */ extra_bits(4) | extra_run | run_offset(48) | level_offset(0),

	/* 65 */ extra_bits(2) | extra_run | run_offset(4) | level_offset(1),
	/* 66 */ extra_bits(3) | extra_run | run_offset(8) | level_offset(1),
	/* 67 */ extra_bits(4) | extra_run | run_offset(16) | level_offset(1),

	/* 68 */ extra_bits(2) | extra_level | run_offset(0) | level_offset(4),
	/* 69 */ extra_bits(3) | extra_level | run_offset(0) | level_offset(8),
	/* 70 */ extra_bits(4) | extra_level | run_offset(0) | level_offset(16),

	/* 71 */ extra_bits(2) | extra_level | run_offset(1) | level_offset(3),
	/* 72 */ extra_bits(3) | extra_level | run_offset(1) | level_offset(7),
};
#undef extra_bits
#undef extra_run
#undef extra_level
#undef run_offset
#undef level_offset

static void x8_get_ac_rlf(IntraX8Context *const w, const int mode, int *const run, int *const level, int *const final) {
	int i, e;

	//    x8_select_ac_table(w, mode);
	i = get_vlc2(w->gb, w->j_ac_vlc_table[mode]);

	if (i < 46) { // [0-45]
		int t, l;
		if (i < 0) {
			*level = *final = // prevent 'may be used uninitialized'
				*run = 64;    // this would cause error exit in the ac loop
			return;
		}

		/*
		 * i == 0-15  r = 0-15 l = 0; r = i & %01111
		 * i == 16-19 r = 0-3  l = 1; r = i & %00011
		 * i == 20-21 r = 0-1  l = 2; r = i & %00001
		 * i == 22    r = 0    l = 3; r = i & %00000
		 */

		*final = t = i > 22;
		i -= 23 * t;

		/* l = lut_l[i / 2] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 3 }[i >> 1];
		 *     11 10'01 01'00 00'00 00'00 00'00 00 => 0xE50000 */
		l = (0xE50000 >> (i & 0x1E)) & 3; // 0x1E or ~1 or (i >> 1 << 1)

		/* t = lut_mask[l] = { 0x0f, 0x03, 0x01, 0x00 }[l];
		 *     as i < 256 the higher bits do not matter */
		t = 0x01030F >> (l << 3);

		*run = i & t;
		*level = l;
	} else if (i < 73) { // [46-72]
		uint32 sm;
		uint32 mask;

		i -= 46;
		sm = ac_decode_table[i];

		e = get_bits(w->gb, sm & 0xF);
		sm >>= 8; // 3 bits
		mask = sm & 0xff;
		sm >>= 8; // 1 bit

		*run = (sm & 0xff) + (e & mask);  // 6 bits
		*level = (sm >> 8) + (e & ~mask); // 5 bits
		*final = i > (58 - 46);
	} else if (i < 75) { // [73-74]
		static const uint8 crazy_mix_runlevel[32] = {
			0x22, 0x32, 0x33, 0x53, 0x23, 0x42, 0x43, 0x63, 0x24, 0x52, 0x34, 0x73, 0x25, 0x62, 0x44, 0x83,
			0x26, 0x72, 0x35, 0x54, 0x27, 0x82, 0x45, 0x64, 0x28, 0x92, 0x36, 0x74, 0x29, 0xa2, 0x46, 0x84,
		};

		*final = !(i & 1);
		e = get_bits(w->gb, 5); // get the extra bits
		*run = crazy_mix_runlevel[e] >> 4;
		*level = crazy_mix_runlevel[e] & 0x0F;
	} else {
		*level = get_bits(w->gb, 7 - 3 * (i & 1));
		*run = get_bits(w->gb, 6);
		*final = get_bits1(w->gb);
	}
	return;
}

/* static const uint8 dc_extra_sbits[] = {
 *     0, 1, 1, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7,
 * }; */
static const uint8 dc_index_offset[] = {
	0, 1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
};

static int x8_get_dc_rlf(IntraX8Context *const w, const int mode, int *const level, int *const final) {
	int i, e, c;

	assert(mode < 3);
	if (!w->j_dc_vlc_table[mode]) {
		int table_index = get_bits(w->gb, 3);
		// 4 modes, same table
		w->j_dc_vlc_table[mode] = j_dc_vlc[w->quant < 13][table_index];
	}

	i = get_vlc2(w->gb, w->j_dc_vlc_table[mode]);

	/* (i >= 17) { i -= 17; final =1; } */
	c = i > 16;
	*final = c;
	i -= 17 * c;

	if (i <= 0) {
		*level = 0;
		return -i;
	}
	c = (i + 1) >> 1; // hackish way to calculate dc_extra_sbits[]
	c -= c > 1;

	e = get_bits(w->gb, c); // get the extra bits
	i = dc_index_offset[i] + (e >> 1);

	e = -(e & 1);         // 0, 0xffffff
	*level = (i ^ e) - e; // (i ^ 0) - 0, (i ^ 0xff) - (-1)
	return 0;
}

// end of huffman

static int x8_setup_spatial_predictor(IntraX8Context *const w, const int chroma) {
	int range;
	int sum;
	int quant;

	w->dsp.setup_spatial_compensation(w->dest[chroma], w->scratchpad, w->frame->linesize[chroma > 0], &range, &sum,
									  w->edges);
	if (chroma) {
		w->orient = w->chroma_orient;
		quant = w->quant_dc_chroma;
	} else {
		quant = w->quant;
	}

	w->flat_dc = 0;
	if (range < quant || range < 3) {
		w->orient = 0;

		// yep you read right, a +-1 idct error may break decoding!
		if (range < 3) {
			w->flat_dc = 1;
			sum += 9;
			// ((1 << 17) + 9) / (8 + 8 + 1 + 2) = 6899
			w->predicted_dc = sum * 6899 >> 17;
		}
	}
	if (chroma)
		return 0;

	assert(w->orient < 3);
	if (range < 2 * w->quant) {
		if ((w->edges & 3) == 0) {
			if (w->orient == 1)
				w->orient = 11;
			if (w->orient == 2)
				w->orient = 10;
		} else {
			w->orient = 0;
		}
		w->raw_orient = 0;
	} else {
		static const uint8 prediction_table[3][12] = {
			{0, 8, 4, 10, 11, 2, 6, 9, 1, 3, 5, 7},
			{4, 0, 8, 11, 10, 3, 5, 2, 6, 9, 1, 7},
			{8, 0, 4, 10, 11, 1, 7, 2, 6, 9, 3, 5},
		};
		w->raw_orient = x8_get_orient_vlc(w);
		if (w->raw_orient < 0)
			return -1;
		assert(w->raw_orient < 12);
		assert(w->orient < 3);
		w->orient = prediction_table[w->orient][w->raw_orient];
	}
	return 0;
}

static void x8_update_predictions(IntraX8Context *const w, const int orient, const int est_run) {
	w->prediction_table[w->mb_x * 2 + (w->mb_y & 1)] = (est_run << 2) + 1 * (orient == 4) + 2 * (orient == 8);
	/*
	 * y = 2n + 0 -> // 0 2 4
	 * y = 2n + 1 -> // 1 3 5
	 */
}

static void x8_get_prediction_chroma(IntraX8Context *const w) {
	w->edges = 1 * !(w->mb_x >> 1);
	w->edges |= 2 * !(w->mb_y >> 1);
	w->edges |= 4 * (w->mb_x >= (2 * w->mb_width - 1)); // mb_x for chroma would always be odd

	w->raw_orient = 0;
	// lut_co[8] = {inv,4,8,8, inv,4,8,8} <- => {1,1,0,0;1,1,0,0} => 0xCC
	if (w->edges & 3) {
		w->chroma_orient = 4 << ((0xCC >> w->edges) & 1);
		return;
	}
	// block[x - 1][y | 1 - 1)]
	w->chroma_orient = (w->prediction_table[2 * w->mb_x - 2] & 0x03) << 2;
}

static void x8_get_prediction(IntraX8Context *const w) {
	int a, b, c, i;

	w->edges = 1 * !w->mb_x;
	w->edges |= 2 * !w->mb_y;
	w->edges |= 4 * (w->mb_x >= (2 * w->mb_width - 1));

	switch (w->edges & 3) {
	case 0:
		break;
	case 1:
		// take the one from the above block[0][y - 1]
		w->est_run = w->prediction_table[!(w->mb_y & 1)] >> 2;
		w->orient = 1;
		return;
	case 2:
		// take the one from the previous block[x - 1][0]
		w->est_run = w->prediction_table[2 * w->mb_x - 2] >> 2;
		w->orient = 2;
		return;
	case 3:
		w->est_run = 16;
		w->orient = 0;
		return;
	}
	// no edge cases
	b = w->prediction_table[2 * w->mb_x + !(w->mb_y & 1)];     // block[x    ][y - 1]
	a = w->prediction_table[2 * w->mb_x - 2 + (w->mb_y & 1)];  // block[x - 1][y    ]
	c = w->prediction_table[2 * w->mb_x - 2 + !(w->mb_y & 1)]; // block[x - 1][y - 1]

	w->est_run = MIN(b, a);
	/* This condition has nothing to do with w->edges, even if it looks
	 * similar it would trigger if e.g. x = 3; y = 2;
	 * I guess somebody wrote something wrong and it became standard. */
	if ((w->mb_x & w->mb_y) != 0)
		w->est_run = MIN(c, w->est_run);
	w->est_run >>= 2;

	a &= 3;
	b &= 3;
	c &= 3;

	i = (0xFFEAF4C4 >> (2 * b + 8 * a)) & 3;
	if (i != 3)
		w->orient = i;
	else
		w->orient = (0xFFEAD8 >> (2 * c + 8 * (w->quant > 12))) & 3;
	/*
	 * lut1[b][a] = {
	 * ->{ 0, 1, 0, pad },
	 *   { 0, 1, X, pad },
	 *   { 2, 2, 2, pad }
	 * }
	 * pad 2  2  2;
	 * pad X  1  0;
	 * pad 0  1  0 <-
	 * -> 11 10 '10 10 '11 11'01 00 '11 00'01 00 => 0xEAF4C4
	 *
	 * lut2[q>12][c] = {
	 * ->{ 0, 2, 1, pad},
	 *   { 2, 2, 2, pad}
	 * }
	 * pad 2  2  2;
	 * pad 1  2  0 <-
	 * -> 11 10'10 10 '11 01'10 00 => 0xEAD8
	 */
}

static void x8_ac_compensation(IntraX8Context *const w, const int direction, const int dc_level) {
	int t;
#define B(x, y) w->block[(x) + (y) * 8]
#define T(x) ((x) * dc_level + 0x8000) >> 16;
	switch (direction) {
	case 0:
		t = T(3811); // h
		B(1, 0) -= t;
		B(0, 1) -= t;

		t = T(487); // e
		B(2, 0) -= t;
		B(0, 2) -= t;

		t = T(506); // f
		B(3, 0) -= t;
		B(0, 3) -= t;

		t = T(135); // c
		B(4, 0) -= t;
		B(0, 4) -= t;
		B(2, 1) += t;
		B(1, 2) += t;
		B(3, 1) += t;
		B(1, 3) += t;

		t = T(173); // d
		B(5, 0) -= t;
		B(0, 5) -= t;

		t = T(61); // b
		B(6, 0) -= t;
		B(0, 6) -= t;
		B(5, 1) += t;
		B(1, 5) += t;

		t = T(42); // a
		B(7, 0) -= t;
		B(0, 7) -= t;
		B(4, 1) += t;
		B(1, 4) += t;
		B(4, 4) += t;

		t = T(1084); // g
		B(1, 1) += t;
		break;
	case 1:
		B(0, 1) -= T(6269);
		B(0, 3) -= T(708);
		B(0, 5) -= T(172);
		B(0, 7) -= T(73);
		break;
	case 2:
		B(1, 0) -= T(6269);
		B(3, 0) -= T(708);
		B(5, 0) -= T(172);
		B(7, 0) -= T(73);
		break;
	}
#undef B
#undef T
}

static void dsp_x8_put_solidcolor(const uint8 pix, uint8 *dst, const ptrdiff_t linesize) {
	int k;
	for (k = 0; k < 8; k++) {
		memset(dst, pix, 8);
		dst += linesize;
	}
}

static const int16 quant_table[64] = {
	256, 256, 256, 256, 256, 256, 259, 262, 265, 269, 272, 275, 278, 282, 285, 288, 292, 295, 299, 303, 306, 310,
	314, 317, 321, 325, 329, 333, 337, 341, 345, 349, 353, 358, 362, 366, 371, 375, 379, 384, 389, 393, 398, 403,
	408, 413, 417, 422, 428, 433, 438, 443, 448, 454, 459, 465, 470, 476, 482, 488, 493, 499, 505, 511,
};

static int x8_decode_intra_mb(IntraX8Context *const w, const int chroma) {
	uint8 *scantable;
	int final, run, level;
	int ac_mode, dc_mode, est_run, dc_level;
	int pos, n;
	int zeros_only;
	int use_quant_matrix;
	int sign;

	assert(w->orient < 12);
	w->bdsp.clear_block(w->block);

	if (chroma)
		dc_mode = 2;
	else
		dc_mode = !!w->est_run; // 0, 1

	if (x8_get_dc_rlf(w, dc_mode, &dc_level, &final))
		return -1;
	n = 0;
	zeros_only = 0;
	if (!final) { // decode ac
		use_quant_matrix = w->use_quant_matrix;
		if (chroma) {
			ac_mode = 1;
			est_run = 64; // not used
		} else {
			if (w->raw_orient < 3)
				use_quant_matrix = 0;

			if (w->raw_orient > 4) {
				ac_mode = 0;
				est_run = 64;
			} else {
				if (w->est_run > 1) {
					ac_mode = 2;
					est_run = w->est_run;
				} else {
					ac_mode = 3;
					est_run = 64;
				}
			}
		}
		x8_select_ac_table(w, ac_mode);
		/* scantable_selector[12] = { 0, 2, 0, 1, 1, 1, 0, 2, 2, 0, 1, 2 }; <-
		 * -> 10'01' 00'10' 10'00' 01'01' 01'00' 10'00 => 0x928548 */
		scantable = w->permutated_scantable[(0x928548 >> (2 * w->orient)) & 3];
		pos = 0;
		do {
			n++;
			if (n >= est_run) {
				ac_mode = 3;
				x8_select_ac_table(w, 3);
			}

			x8_get_ac_rlf(w, ac_mode, &run, &level, &final);

			pos += run + 1;
			if (pos > 63) {
				// this also handles vlc error in x8_get_ac_rlf
				return -1;
			}
			level = (level + 1) * w->dquant;
			level += w->qsum;

			sign = -get_bits1(w->gb);
			level = (level ^ sign) - sign;

			if (use_quant_matrix)
				level = (level * quant_table[pos]) >> 8;

			w->block[scantable[pos]] = level;
		} while (!final);
	} else {                                                // DC only
		if (w->flat_dc && ((unsigned)(dc_level + 1)) < 3) { // [-1; 1]
			int32 divide_quant = !chroma ? w->divide_quant_dc_luma : w->divide_quant_dc_chroma;
			int32 dc_quant = !chroma ? w->quant : w->quant_dc_chroma;

			// original intent dc_level += predicted_dc/quant;
			// but it got lost somewhere in the rounding
			dc_level += (w->predicted_dc * divide_quant + (1 << 12)) >> 13;

			dsp_x8_put_solidcolor(av_clip_uint8((dc_level * dc_quant + 4) >> 3), w->dest[chroma],
								  w->frame->linesize[!!chroma]);

			goto block_placed;
		}
		zeros_only = dc_level == 0;
	}
	if (!chroma)
		w->block[0] = dc_level * w->quant;
	else
		w->block[0] = dc_level * w->quant_dc_chroma;

	// there is !zero_only check in the original, but dc_level check is enough
	if ((unsigned int)(dc_level + 1) >= 3 && (w->edges & 3) != 3) {
		int direction;
		/* ac_comp_direction[orient] = { 0, 3, 3, 1, 1, 0, 0, 0, 2, 2, 2, 1 }; <-
		 * -> 01'10' 10'10' 00'00' 00'01' 01'11' 11'00 => 0x6A017C */
		direction = (0x6A017C >> (w->orient * 2)) & 3;
		if (direction != 3) {
			x8_ac_compensation(w, direction, w->block[0]);
		}
	}

	if (w->flat_dc) {
		dsp_x8_put_solidcolor(w->predicted_dc, w->dest[chroma], w->frame->linesize[!!chroma]);
	} else {
		w->dsp.spatial_compensation[w->orient](w->scratchpad, w->dest[chroma], w->frame->linesize[!!chroma]);
	}
	if (!zeros_only)
		w->idsp.idct_add(w->dest[chroma], w->frame->linesize[!!chroma], w->block);

block_placed:
	if (!chroma)
		x8_update_predictions(w, w->orient, n);

	if (w->loopfilter) {
		uint8 *ptr = w->dest[chroma];
		ptrdiff_t linesize = w->frame->linesize[!!chroma];

		if (!((w->edges & 2) || (zeros_only && (w->orient | 4) == 4)))
			w->dsp.h_loop_filter(ptr, linesize, w->quant);

		if (!((w->edges & 1) || (zeros_only && (w->orient | 8) == 8)))
			w->dsp.v_loop_filter(ptr, linesize, w->quant);
	}
	return 0;
}

// FIXME maybe merge with ff_*
static void x8_init_block_index(IntraX8Context *w, Planes *frame) {
	// not parent codec linesize as this would be wrong for field pics
	// not that IntraX8 has interlacing support ;)
	const ptrdiff_t linesize = frame->linesize[0];
	const ptrdiff_t uvlinesize = frame->linesize[1];

	w->dest[0] = frame->data[0];
	w->dest[1] = frame->data[1];
	w->dest[2] = frame->data[2];

	w->dest[0] += w->mb_y * linesize << 3;
	// chroma blocks are on add rows
	w->dest[1] += (w->mb_y & ~1) * uvlinesize << 2;
	w->dest[2] += (w->mb_y & ~1) * uvlinesize << 2;
}

int initX8(IntraX8Context *w, int16 block[64], int mb_width, int mb_height) {
	w->mb_width = mb_width;
	w->mb_height = mb_height;
	w->block = block;
	w->prediction_table = new uint8[mb_width * 4]();
	w->idsp.idct_add = wmv2_idct_add_c;
	memcpy(w->permutated_scantable[0], ff_wmv1_scantable[0], 64);
	memcpy(w->permutated_scantable[1], ff_wmv1_scantable[2], 64);
	memcpy(w->permutated_scantable[2], ff_wmv1_scantable[3], 64);
	ff_intrax8dsp_init(&w->dsp);
	PictureState state = {};
	ChromaDSP chroma;
	initSupport(&state, &chroma);
	w->bdsp = state.bdsp;
	static const bool initialized = (x8_vlc_init(), true);
	(void)initialized;
	return 0;
}

void endX8(IntraX8Context *w) {
	delete[] w->prediction_table;
}

int ff_intrax8_decode_picture(IntraX8Context *w, Picture *pict, Bits *gb, int *mb_x, int *mb_y, int dquant,
							  int quant_offset, int loopfilter, int lowdelay) {
	int mb_xy;

	w->gb = gb;
	w->dquant = dquant;
	w->quant = dquant >> 1;
	w->qsum = quant_offset;
	w->frame = pict->f;
	w->loopfilter = loopfilter;
	w->use_quant_matrix = get_bits1(w->gb);

	w->mb_x = *mb_x;
	w->mb_y = *mb_y;

	w->divide_quant_dc_luma = ((1 << 16) + (w->quant >> 1)) / w->quant;
	if (w->quant < 5) {
		w->quant_dc_chroma = w->quant;
		w->divide_quant_dc_chroma = w->divide_quant_dc_luma;
	} else {
		w->quant_dc_chroma = w->quant + ((w->quant + 3) >> 3);
		w->divide_quant_dc_chroma = ((1 << 16) + (w->quant_dc_chroma >> 1)) / w->quant_dc_chroma;
	}
	x8_reset_vlc_tables(w);

	for (w->mb_y = 0; w->mb_y < w->mb_height * 2; w->mb_y++) {
		x8_init_block_index(w, w->frame);
		mb_xy = (w->mb_y >> 1) * (w->mb_width + 1);
		if (get_bits_left(gb) < 1)
			goto error;
		for (w->mb_x = 0; w->mb_x < w->mb_width * 2; w->mb_x++) {
			x8_get_prediction(w);
			if (x8_setup_spatial_predictor(w, 0))
				goto error;
			if (x8_decode_intra_mb(w, 0))
				goto error;

			if (w->mb_x & w->mb_y & 1) {
				x8_get_prediction_chroma(w);

				/* when setting up chroma, no vlc is read,
				 * so no error condition can be reached */
				x8_setup_spatial_predictor(w, 1);
				if (x8_decode_intra_mb(w, 1))
					goto error;

				x8_setup_spatial_predictor(w, 2);
				if (x8_decode_intra_mb(w, 2))
					goto error;

				w->dest[1] += 8;
				w->dest[2] += 8;

				pict->qscale_table[mb_xy] = w->quant;
				mb_xy++;
			}
			w->dest[0] += 8;
		}
	}

error:
	*mb_x = w->mb_x;
	*mb_y = w->mb_y;

	return 0;
}

#undef AC_VLC_MTD
#undef DC_VLC_BITS
#undef DC_VLC_MTD
#undef MAX_AC_VLC_BITS
#undef MAX_DC_VLC_BITS
#undef MAX_OR_VLC_BITS
#undef MAX_TABLE_DEPTH
#undef OR_VLC_BITS
#undef OR_VLC_MTD
#undef VLC_BUFFER_SIZE
#undef W0
#undef W1
#undef W2
#undef W3
#undef W4
#undef W5
#undef W6
#undef W7

static int bitplane_decoding(uint8 *data, int *raw, VC1Context *v) {
	VC1Bitplane decoder;
	int result =
		decoder.decode(data, raw, v->s.mb_width, v->s.mb_height >> v->field_mode, v->s.mb_stride, v->gb.stream);
	return result;
}

static int vop_dquant_decoding(VC1Context *v) {
	Bits *const gb = &v->gb;
	int pqdiff;

	// variable size
	if (v->dquant != 2) {
		v->dquantfrm = get_bits1(gb);
		if (!v->dquantfrm)
			return 0;

		v->dqprofile = get_bits(gb, 2);
		switch (v->dqprofile) {
		case DQPROFILE_SINGLE_EDGE:
		case DQPROFILE_DOUBLE_EDGES:
			v->dqsbedge = get_bits(gb, 2);
			break;
		case DQPROFILE_ALL_MBS:
			v->dqbilevel = get_bits1(gb);
			if (!v->dqbilevel) {
				v->halfpq = 0;
				return 0;
			}
			break;
		default:
			break; // Forbidden ?
		}
	}

	pqdiff = get_bits(gb, 3);
	if (pqdiff == 7)
		v->altpq = get_bits(gb, 5);
	else
		v->altpq = v->pq + pqdiff + 1;

	return 0;
}

/* fill lookup tables for intensity compensation */
#define INIT_LUT(lumscale, lumshift, luty, lutuv, chain)                                                               \
	do {                                                                                                               \
		int scale, shift, i;                                                                                           \
		if (!lumscale) {                                                                                               \
			scale = -64;                                                                                               \
			shift = (255 - lumshift * 2) * 64;                                                                         \
			if (lumshift > 31)                                                                                         \
				shift += 128 << 6;                                                                                     \
		} else {                                                                                                       \
			scale = lumscale + 32;                                                                                     \
			if (lumshift > 31)                                                                                         \
				shift = (lumshift - 64) * 64;                                                                          \
			else                                                                                                       \
				shift = lumshift << 6;                                                                                 \
		}                                                                                                              \
		for (i = 0; i < 256; i++) {                                                                                    \
			int iy = chain ? luty[i] : i;                                                                              \
			int iu = chain ? lutuv[i] : i;                                                                             \
			luty[i] = av_clip_uint8((scale * iy + shift + 32) >> 6);                                                   \
			lutuv[i] = av_clip_uint8((scale * (iu - 128) + 128 * 64 + 32) >> 6);                                       \
		}                                                                                                              \
	} while (0)

static void rotate_luts(VC1Context *v) {
	if (v->s.pict_type == AV_PICTURE_TYPE_BI || v->s.pict_type == AV_PICTURE_TYPE_B) {
		v->curr_use_ic = &v->aux_use_ic;
		v->curr_luty = v->aux_luty;
		v->curr_lutuv = v->aux_lutuv;
	} else {
#define ROTATE(DEF, L, N, C)                                                                                           \
	do {                                                                                                               \
		DEF;                                                                                                           \
		memcpy(&tmp, L, sizeof(tmp));                                                                                  \
		memcpy(L, N, sizeof(tmp));                                                                                     \
		memcpy(N, &tmp, sizeof(tmp));                                                                                  \
		C = N;                                                                                                         \
	} while (0)

		ROTATE(int tmp, &v->last_use_ic, &v->next_use_ic, v->curr_use_ic);
		ROTATE(uint8 tmp[2][256], v->last_luty, v->next_luty, v->curr_luty);
		ROTATE(uint8 tmp[2][256], v->last_lutuv, v->next_lutuv, v->curr_lutuv);
	}

	INIT_LUT(32, 0, v->curr_luty[0], v->curr_lutuv[0], 0);
	INIT_LUT(32, 0, v->curr_luty[1], v->curr_lutuv[1], 0);
	*v->curr_use_ic = 0;
}

static int read_bfraction(VC1Context *v, Bits *gb) {
	int bfraction_lut_index = get_bits(gb, 3);

	if (bfraction_lut_index == 7)
		bfraction_lut_index = 7 + get_bits(gb, 4);

	if (bfraction_lut_index == 21) {
		warning("bfraction invalid\n");
		return (-1);
	}
	v->bfraction_lut_index = bfraction_lut_index;
	v->bfraction = ff_vc1_bfraction_lut[v->bfraction_lut_index];
	return 0;
}

int ff_vc1_parse_frame_header(VC1Context *v, Bits *gb) {
	int pqindex, lowquant, status;

	v->field_mode = 0;
	v->fcm = PROGRESSIVE;
	if (v->finterpflag)
		v->interpfrm = get_bits1(gb);
	skip_bits(gb, 2); // framecnt unused
	v->rangeredfrm = 0;
	if (v->rangered)
		v->rangeredfrm = get_bits1(gb);
	if (get_bits1(gb)) {
		v->s.pict_type = AV_PICTURE_TYPE_P;
	} else {
		if (v->max_b_frames && !get_bits1(gb)) {
			v->s.pict_type = AV_PICTURE_TYPE_B;
		} else
			v->s.pict_type = AV_PICTURE_TYPE_I;
	}

	v->bi_type = 0;
	if (v->s.pict_type == AV_PICTURE_TYPE_B) {
		if (read_bfraction(v, gb) < 0)
			return (-1);
		if (v->bfraction == 0) {
			v->s.pict_type = AV_PICTURE_TYPE_BI;
		}
	}
	if (v->s.pict_type == AV_PICTURE_TYPE_I || v->s.pict_type == AV_PICTURE_TYPE_BI)
		skip_bits(gb, 7); // skip buffer fullness

	if (v->parse_only)
		return 0;

	/* calculate RND */
	if (v->s.pict_type == AV_PICTURE_TYPE_I || v->s.pict_type == AV_PICTURE_TYPE_BI)
		v->rnd = 1;
	if (v->s.pict_type == AV_PICTURE_TYPE_P)
		v->rnd ^= 1;

	if (get_bits_left(gb) < 5)
		return (-1);
	/* Quantizer stuff */
	pqindex = get_bits(gb, 5);
	if (!pqindex)
		return -1;
	if (v->quantizer_mode == QUANT_FRAME_IMPLICIT)
		v->pq = ff_vc1_pquant_table[0][pqindex];
	else
		v->pq = ff_vc1_pquant_table[1][pqindex];
	v->pqindex = pqindex;
	if (pqindex < 9)
		v->halfpq = get_bits1(gb);
	else
		v->halfpq = 0;
	switch (v->quantizer_mode) {
	case QUANT_FRAME_IMPLICIT:
		v->pquantizer = pqindex < 9;
		break;
	case QUANT_NON_UNIFORM:
		v->pquantizer = 0;
		break;
	case QUANT_FRAME_EXPLICIT:
		v->pquantizer = get_bits1(gb);
		break;
	default:
		v->pquantizer = 1;
		break;
	}
	v->dquantfrm = 0;
	if (v->extended_mv == 1)
		v->mvrange = get_unary(gb, 0, 3);
	v->k_x = v->mvrange + 9 + (v->mvrange >> 1); // k_x can be 9 10 12 13
	v->k_y = v->mvrange + 8;                     // k_y can be 8 9 10 11
	v->range_x = 1 << (v->k_x - 1);
	v->range_y = 1 << (v->k_y - 1);
	if (v->multires && v->s.pict_type != AV_PICTURE_TYPE_B)
		v->respic = get_bits(gb, 2);

	if (v->res_x8 && (v->s.pict_type == AV_PICTURE_TYPE_I || v->s.pict_type == AV_PICTURE_TYPE_BI)) {
		v->x8_type = get_bits1(gb);
	} else
		v->x8_type = 0;

	if (v->first_pic_header_flag)
		rotate_luts(v);

	switch (v->s.pict_type) {
	case AV_PICTURE_TYPE_P:
		v->tt_index = (v->pq > 4) + (v->pq > 12);

		lowquant = (v->pq > 12) ? 0 : 1;
		v->mv_mode = ff_vc1_mv_pmode_table[lowquant][get_unary(gb, 1, 4)];
		if (v->mv_mode == MV_PMODE_INTENSITY_COMP) {
			v->mv_mode2 = ff_vc1_mv_pmode_table2[lowquant][get_unary(gb, 1, 3)];
			v->lumscale = get_bits(gb, 6);
			v->lumshift = get_bits(gb, 6);
			v->last_use_ic = 1;
			/* fill lookup tables for intensity compensation */
			INIT_LUT(v->lumscale, v->lumshift, v->last_luty[0], v->last_lutuv[0], 1);
			INIT_LUT(v->lumscale, v->lumshift, v->last_luty[1], v->last_lutuv[1], 1);
		}
		if (v->mv_mode == MV_PMODE_INTENSITY_COMP) {
			v->s.quarter_sample = (v->mv_mode2 != MV_PMODE_1MV_HPEL && v->mv_mode2 != MV_PMODE_1MV_HPEL_BILIN);
			v->s.mspel = (v->mv_mode2 != MV_PMODE_1MV_HPEL_BILIN);
		} else {
			v->s.quarter_sample = (v->mv_mode != MV_PMODE_1MV_HPEL && v->mv_mode != MV_PMODE_1MV_HPEL_BILIN);
			v->s.mspel = (v->mv_mode != MV_PMODE_1MV_HPEL_BILIN);
		}

		if ((v->mv_mode == MV_PMODE_INTENSITY_COMP && v->mv_mode2 == MV_PMODE_MIXED_MV) ||
			v->mv_mode == MV_PMODE_MIXED_MV) {
			status = bitplane_decoding(v->mv_type_mb_plane, &v->mv_type_is_raw, v);
			if (status < 0)
				return -1;

		} else {
			v->mv_type_is_raw = 0;
			memset(v->mv_type_mb_plane, 0, v->s.mb_stride * v->s.mb_height);
		}
		status = bitplane_decoding(v->s.mbskip_table, &v->skip_is_raw, v);
		if (status < 0)
			return -1;

		if (get_bits_left(gb) < 4)
			return (-1);

		/* Hopefully this is correct for P-frames */
		v->mv_table_index = get_bits(gb, 2); // but using ff_vc1_ tables
		v->cbptab = get_bits(gb, 2);
		v->cbpcy_vlc = ff_vc1_cbpcy_p_vlc[v->cbptab];

		if (v->dquant) {

			vop_dquant_decoding(v);
		}

		if (v->vstransform) {
			v->ttmbf = get_bits1(gb);
			if (v->ttmbf) {
				v->ttfrm = ff_vc1_ttfrm_to_tt[get_bits(gb, 2)];
			} else
				v->ttfrm = 0; // FIXME Is that so ?
		} else {
			v->ttmbf = 1;
			v->ttfrm = TT_8X8;
		}
		break;
	case AV_PICTURE_TYPE_B:
		v->tt_index = (v->pq > 4) + (v->pq > 12);

		v->mv_mode = get_bits1(gb) ? MV_PMODE_1MV : MV_PMODE_1MV_HPEL_BILIN;
		v->s.quarter_sample = (v->mv_mode == MV_PMODE_1MV);
		v->s.mspel = v->s.quarter_sample;

		status = bitplane_decoding(v->direct_mb_plane, &v->dmb_is_raw, v);
		if (status < 0)
			return -1;

		status = bitplane_decoding(v->s.mbskip_table, &v->skip_is_raw, v);
		if (status < 0)
			return -1;

		v->mv_table_index = get_bits(gb, 2);
		v->cbptab = get_bits(gb, 2);
		v->cbpcy_vlc = ff_vc1_cbpcy_p_vlc[v->cbptab];

		if (v->dquant) {

			vop_dquant_decoding(v);
		}

		if (v->vstransform) {
			v->ttmbf = get_bits1(gb);
			if (v->ttmbf) {
				v->ttfrm = ff_vc1_ttfrm_to_tt[get_bits(gb, 2)];
			} else
				v->ttfrm = 0;
		} else {
			v->ttmbf = 1;
			v->ttfrm = TT_8X8;
		}
		break;
	}

	if (!v->x8_type) {
		/* AC Syntax */
		v->c_ac_table_index = decode012(gb);
		if (v->s.pict_type == AV_PICTURE_TYPE_I || v->s.pict_type == AV_PICTURE_TYPE_BI) {
			v->y_ac_table_index = decode012(gb);
		}
		/* DC Syntax */
		v->dc_table_index = get_bits1(gb);
	}

	if (v->s.pict_type == AV_PICTURE_TYPE_BI) {
		v->s.pict_type = AV_PICTURE_TYPE_B;
		v->bi_type = 1;
	}
	return 0;
}

int ff_vc1_parse_frame_header_adv(VC1Context *v, Bits *gb) {
	int pqindex, lowquant;
	int status;
	int field_mode, fcm;

	v->numref = 0;
	v->p_frame_skipped = 0;
	if (v->second_field) {
		if (v->fcm != ILACE_FIELD || v->field_mode != 1)
			return -1;
		if (v->fptype & 4)
			v->s.pict_type = (v->fptype & 1) ? AV_PICTURE_TYPE_BI : AV_PICTURE_TYPE_B;
		else
			v->s.pict_type = (v->fptype & 1) ? AV_PICTURE_TYPE_P : AV_PICTURE_TYPE_I;
		v->s.cur_pic.ptr->f->pict_type = v->s.pict_type;
		if (!v->pic_header_flag)
			goto parse_common_info;
	}

	field_mode = 0;
	if (v->interlace) {
		fcm = decode012(gb);
		if (fcm) {
			if (fcm == ILACE_FIELD)
				field_mode = 1;
		}
	} else {
		fcm = PROGRESSIVE;
	}
	if (!v->first_pic_header_flag && v->field_mode != field_mode)
		return (-1);
	v->field_mode = field_mode;
	v->fcm = fcm;

	assert(v->s.mb_height == (v->s.height + 15) >> 4 || v->s.mb_height == FFALIGN((v->s.height + 15) >> 4, 2));
	if (v->field_mode) {
		v->s.mb_height = FFALIGN((v->s.height + 15) >> 4, 2);
		v->fptype = get_bits(gb, 3);
		if (v->fptype & 4) // B-picture
			v->s.pict_type = (v->fptype & 2) ? AV_PICTURE_TYPE_BI : AV_PICTURE_TYPE_B;
		else
			v->s.pict_type = (v->fptype & 2) ? AV_PICTURE_TYPE_P : AV_PICTURE_TYPE_I;
	} else {
		v->s.mb_height = (v->s.height + 15) >> 4;
		switch (get_unary(gb, 0, 4)) {
		case 0:
			v->s.pict_type = AV_PICTURE_TYPE_P;
			break;
		case 1:
			v->s.pict_type = AV_PICTURE_TYPE_B;
			break;
		case 2:
			v->s.pict_type = AV_PICTURE_TYPE_I;
			break;
		case 3:
			v->s.pict_type = AV_PICTURE_TYPE_BI;
			break;
		case 4:
			v->s.pict_type = AV_PICTURE_TYPE_P; // skipped pic
			v->p_frame_skipped = 1;
			break;
		}
	}
	if (v->tfcntrflag)
		skip_bits(gb, 8);
	if (v->broadcast) {
		if (!v->interlace || v->psf) {
			v->rptfrm = get_bits(gb, 2);
		} else {
			v->tff = get_bits1(gb);
			v->rff = get_bits1(gb);
		}
	} else {
		v->tff = 1;
	}
	if (v->panscanflag) {
		warning("Pan-scan");
		//...
	}
	if (v->p_frame_skipped) {
		return 0;
	}
	v->rnd = get_bits1(gb);
	if (v->interlace)
		v->uvsamp = get_bits1(gb);
	if (v->field_mode) {
		if (!v->refdist_flag)
			v->refdist = 0;
		else if ((v->s.pict_type != AV_PICTURE_TYPE_B) && (v->s.pict_type != AV_PICTURE_TYPE_BI)) {
			v->refdist = get_bits(gb, 2);
			if (v->refdist == 3)
				v->refdist += get_unary(gb, 0, 14);
			if (v->refdist > 16)
				return (-1);
		}
		if ((v->s.pict_type == AV_PICTURE_TYPE_B) || (v->s.pict_type == AV_PICTURE_TYPE_BI)) {
			if (read_bfraction(v, gb) < 0)
				return (-1);
			v->frfd = (v->bfraction * v->refdist) >> 8;
			v->brfd = v->refdist - v->frfd - 1;
			if (v->brfd < 0)
				v->brfd = 0;
		}
		goto parse_common_info;
	}
	if (v->fcm == PROGRESSIVE) {
		if (v->finterpflag)
			v->interpfrm = get_bits1(gb);
		if (v->s.pict_type == AV_PICTURE_TYPE_B) {
			if (read_bfraction(v, gb) < 0)
				return (-1);
			if (v->bfraction == 0) {
				v->s.pict_type = AV_PICTURE_TYPE_BI; /* XXX: should not happen here */
			}
		}
	}

parse_common_info:
	if (v->field_mode)
		v->cur_field_type = !(v->tff ^ v->second_field);
	pqindex = get_bits(gb, 5);
	if (!pqindex)
		return -1;
	if (v->quantizer_mode == QUANT_FRAME_IMPLICIT)
		v->pq = ff_vc1_pquant_table[0][pqindex];
	else
		v->pq = ff_vc1_pquant_table[1][pqindex];
	v->pqindex = pqindex;
	if (pqindex < 9)
		v->halfpq = get_bits1(gb);
	else
		v->halfpq = 0;
	switch (v->quantizer_mode) {
	case QUANT_FRAME_IMPLICIT:
		v->pquantizer = pqindex < 9;
		break;
	case QUANT_NON_UNIFORM:
		v->pquantizer = 0;
		break;
	case QUANT_FRAME_EXPLICIT:
		v->pquantizer = get_bits1(gb);
		break;
	default:
		v->pquantizer = 1;
		break;
	}
	v->dquantfrm = 0;
	if (v->postprocflag)
		v->postproc = get_bits(gb, 2);

	if (v->parse_only)
		return 0;

	if (v->first_pic_header_flag)
		rotate_luts(v);

	switch (v->s.pict_type) {
	case AV_PICTURE_TYPE_I:
	case AV_PICTURE_TYPE_BI:
		if (v->fcm == ILACE_FRAME) { // interlace frame picture
			status = bitplane_decoding(v->fieldtx_plane, &v->fieldtx_is_raw, v);
			if (status < 0)
				return -1;

		} else
			v->fieldtx_is_raw = 0;
		status = bitplane_decoding(v->acpred_plane, &v->acpred_is_raw, v);
		if (status < 0)
			return -1;

		v->condover = CONDOVER_NONE;
		if (v->overlap && v->pq <= 8) {
			v->condover = decode012(gb);
			if (v->condover == CONDOVER_SELECT) {
				status = bitplane_decoding(v->over_flags_plane, &v->overflg_is_raw, v);
				if (status < 0)
					return -1;
			}
		}
		break;
	case AV_PICTURE_TYPE_P:
		if (v->field_mode) {
			v->numref = get_bits1(gb);
			if (!v->numref) {
				v->reffield = get_bits1(gb);
				v->ref_field_type[0] = v->reffield ^ !v->cur_field_type;
			}
		}
		if (v->extended_mv)
			v->mvrange = get_unary(gb, 0, 3);
		else
			v->mvrange = 0;
		if (v->interlace) {
			if (v->extended_dmv)
				v->dmvrange = get_unary(gb, 0, 3);
			else
				v->dmvrange = 0;
			if (v->fcm == ILACE_FRAME) { // interlaced frame picture
				v->fourmvswitch = get_bits1(gb);
				v->intcomp = get_bits1(gb);
				if (v->intcomp) {
					v->lumscale = get_bits(gb, 6);
					v->lumshift = get_bits(gb, 6);
					INIT_LUT(v->lumscale, v->lumshift, v->last_luty[0], v->last_lutuv[0], 1);
					INIT_LUT(v->lumscale, v->lumshift, v->last_luty[1], v->last_lutuv[1], 1);
					v->last_use_ic = 1;
				}
				status = bitplane_decoding(v->s.mbskip_table, &v->skip_is_raw, v);
				if (status < 0)
					return -1;

				v->mbmodetab = get_bits(gb, 2);
				if (v->fourmvswitch)
					v->mbmode_vlc = ff_vc1_intfr_4mv_mbmode_vlc[v->mbmodetab];
				else
					v->mbmode_vlc = ff_vc1_intfr_non4mv_mbmode_vlc[v->mbmodetab];
				v->imvtab = get_bits(gb, 2);
				v->imv_vlc = ff_vc1_1ref_mvdata_vlc[v->imvtab];
				// interlaced p-picture cbpcy range is [1, 63]
				v->icbptab = get_bits(gb, 3);
				v->cbpcy_vlc = ff_vc1_icbpcy_vlc[v->icbptab];
				v->twomvbptab = get_bits(gb, 2);
				v->twomvbp_vlc = ff_vc1_2mv_block_pattern_vlc[v->twomvbptab];
				if (v->fourmvswitch) {
					v->fourmvbptab = get_bits(gb, 2);
					v->fourmvbp_vlc = ff_vc1_4mv_block_pattern_vlc[v->fourmvbptab];
				}
			}
		}
		v->k_x = v->mvrange + 9 + (v->mvrange >> 1); // k_x can be 9 10 12 13
		v->k_y = v->mvrange + 8;                     // k_y can be 8 9 10 11
		v->range_x = 1 << (v->k_x - 1);
		v->range_y = 1 << (v->k_y - 1);

		v->tt_index = (v->pq > 4) + (v->pq > 12);
		if (v->fcm != ILACE_FRAME) {
			int mvmode;
			mvmode = get_unary(gb, 1, 4);
			lowquant = (v->pq > 12) ? 0 : 1;
			v->mv_mode = ff_vc1_mv_pmode_table[lowquant][mvmode];
			if (v->mv_mode == MV_PMODE_INTENSITY_COMP) {
				int mvmode2;
				mvmode2 = get_unary(gb, 1, 3);
				v->mv_mode2 = ff_vc1_mv_pmode_table2[lowquant][mvmode2];
				if (v->field_mode) {
					v->intcompfield = decode210(gb) ^ 3;
				} else
					v->intcompfield = 3;

				v->lumscale2 = v->lumscale = 32;
				v->lumshift2 = v->lumshift = 0;
				if (v->intcompfield & 1) {
					v->lumscale = get_bits(gb, 6);
					v->lumshift = get_bits(gb, 6);
				}
				if ((v->intcompfield & 2) && v->field_mode) {
					v->lumscale2 = get_bits(gb, 6);
					v->lumshift2 = get_bits(gb, 6);
				} else if (!v->field_mode) {
					v->lumscale2 = v->lumscale;
					v->lumshift2 = v->lumshift;
				}
				if (v->field_mode && v->second_field) {
					if (v->cur_field_type) {
						INIT_LUT(v->lumscale, v->lumshift, v->curr_luty[v->cur_field_type ^ 1],
								 v->curr_lutuv[v->cur_field_type ^ 1], 0);
						INIT_LUT(v->lumscale2, v->lumshift2, v->last_luty[v->cur_field_type],
								 v->last_lutuv[v->cur_field_type], 1);
					} else {
						INIT_LUT(v->lumscale2, v->lumshift2, v->curr_luty[v->cur_field_type ^ 1],
								 v->curr_lutuv[v->cur_field_type ^ 1], 0);
						INIT_LUT(v->lumscale, v->lumshift, v->last_luty[v->cur_field_type],
								 v->last_lutuv[v->cur_field_type], 1);
					}
					v->next_use_ic = *v->curr_use_ic = 1;
				} else {
					INIT_LUT(v->lumscale, v->lumshift, v->last_luty[0], v->last_lutuv[0], 1);
					INIT_LUT(v->lumscale2, v->lumshift2, v->last_luty[1], v->last_lutuv[1], 1);
				}
				v->last_use_ic = 1;
			}
			if (v->mv_mode == MV_PMODE_INTENSITY_COMP) {
				v->s.quarter_sample = (v->mv_mode2 != MV_PMODE_1MV_HPEL && v->mv_mode2 != MV_PMODE_1MV_HPEL_BILIN);
				v->s.mspel = (v->mv_mode2 != MV_PMODE_1MV_HPEL_BILIN);
			} else {
				v->s.quarter_sample = (v->mv_mode != MV_PMODE_1MV_HPEL && v->mv_mode != MV_PMODE_1MV_HPEL_BILIN);
				v->s.mspel = (v->mv_mode != MV_PMODE_1MV_HPEL_BILIN);
			}
		}
		if (v->fcm == PROGRESSIVE) { // progressive
			if ((v->mv_mode == MV_PMODE_INTENSITY_COMP && v->mv_mode2 == MV_PMODE_MIXED_MV) ||
				v->mv_mode == MV_PMODE_MIXED_MV) {
				status = bitplane_decoding(v->mv_type_mb_plane, &v->mv_type_is_raw, v);
				if (status < 0)
					return -1;

			} else {
				v->mv_type_is_raw = 0;
				memset(v->mv_type_mb_plane, 0, v->s.mb_stride * v->s.mb_height);
			}
			status = bitplane_decoding(v->s.mbskip_table, &v->skip_is_raw, v);
			if (status < 0)
				return -1;

			/* Hopefully this is correct for P-frames */
			v->mv_table_index = get_bits(gb, 2); // but using ff_vc1_ tables
			v->cbptab = get_bits(gb, 2);
			v->cbpcy_vlc = ff_vc1_cbpcy_p_vlc[v->cbptab];
		} else if (v->fcm == ILACE_FRAME) { // frame interlaced
			v->s.quarter_sample = 1;
			v->s.mspel = 1;
		} else { // field interlaced
			v->mbmodetab = get_bits(gb, 3);
			v->imvtab = get_bits(gb, 2 + v->numref);
			if (!v->numref)
				v->imv_vlc = ff_vc1_1ref_mvdata_vlc[v->imvtab];
			else
				v->imv_vlc = ff_vc1_2ref_mvdata_vlc[v->imvtab];
			v->icbptab = get_bits(gb, 3);
			v->cbpcy_vlc = ff_vc1_icbpcy_vlc[v->icbptab];
			if ((v->mv_mode == MV_PMODE_INTENSITY_COMP && v->mv_mode2 == MV_PMODE_MIXED_MV) ||
				v->mv_mode == MV_PMODE_MIXED_MV) {
				v->fourmvbptab = get_bits(gb, 2);
				v->fourmvbp_vlc = ff_vc1_4mv_block_pattern_vlc[v->fourmvbptab];
				v->mbmode_vlc = ff_vc1_if_mmv_mbmode_vlc[v->mbmodetab];
			} else {
				v->mbmode_vlc = ff_vc1_if_1mv_mbmode_vlc[v->mbmodetab];
			}
		}
		if (v->dquant) {

			vop_dquant_decoding(v);
		}

		if (v->vstransform) {
			v->ttmbf = get_bits1(gb);
			if (v->ttmbf) {
				v->ttfrm = ff_vc1_ttfrm_to_tt[get_bits(gb, 2)];
			} else
				v->ttfrm = 0; // FIXME Is that so ?
		} else {
			v->ttmbf = 1;
			v->ttfrm = TT_8X8;
		}
		break;
	case AV_PICTURE_TYPE_B:
		if (v->fcm == ILACE_FRAME) {
			if (read_bfraction(v, gb) < 0)
				return (-1);
			if (v->bfraction == 0) {
				return -1;
			}
		}
		if (v->extended_mv)
			v->mvrange = get_unary(gb, 0, 3);
		else
			v->mvrange = 0;
		v->k_x = v->mvrange + 9 + (v->mvrange >> 1); // k_x can be 9 10 12 13
		v->k_y = v->mvrange + 8;                     // k_y can be 8 9 10 11
		v->range_x = 1 << (v->k_x - 1);
		v->range_y = 1 << (v->k_y - 1);

		v->tt_index = (v->pq > 4) + (v->pq > 12);

		if (v->field_mode) {
			int mvmode;

			if (v->extended_dmv)
				v->dmvrange = get_unary(gb, 0, 3);
			mvmode = get_unary(gb, 1, 3);
			lowquant = (v->pq > 12) ? 0 : 1;
			v->mv_mode = ff_vc1_mv_pmode_table2[lowquant][mvmode];
			v->s.quarter_sample = (v->mv_mode == MV_PMODE_1MV || v->mv_mode == MV_PMODE_MIXED_MV);
			v->s.mspel = (v->mv_mode != MV_PMODE_1MV_HPEL_BILIN);
			status = bitplane_decoding(v->forward_mb_plane, &v->fmb_is_raw, v);
			if (status < 0)
				return -1;

			v->mbmodetab = get_bits(gb, 3);
			if (v->mv_mode == MV_PMODE_MIXED_MV)
				v->mbmode_vlc = ff_vc1_if_mmv_mbmode_vlc[v->mbmodetab];
			else
				v->mbmode_vlc = ff_vc1_if_1mv_mbmode_vlc[v->mbmodetab];
			v->imvtab = get_bits(gb, 3);
			v->imv_vlc = ff_vc1_2ref_mvdata_vlc[v->imvtab];
			v->icbptab = get_bits(gb, 3);
			v->cbpcy_vlc = ff_vc1_icbpcy_vlc[v->icbptab];
			if (v->mv_mode == MV_PMODE_MIXED_MV) {
				v->fourmvbptab = get_bits(gb, 2);
				v->fourmvbp_vlc = ff_vc1_4mv_block_pattern_vlc[v->fourmvbptab];
			}
			v->numref = 1; // interlaced field B pictures are always 2-ref
		} else if (v->fcm == ILACE_FRAME) {
			if (v->extended_dmv)
				v->dmvrange = get_unary(gb, 0, 3);
			if (get_bits1(gb)) /* intcomp - present but shall always be 0 */
				warning("Intensity compensation set for B picture\n");
			v->intcomp = 0;
			v->mv_mode = MV_PMODE_1MV;
			v->fourmvswitch = 0;
			v->s.quarter_sample = 1;
			v->s.mspel = 1;
			status = bitplane_decoding(v->direct_mb_plane, &v->dmb_is_raw, v);
			if (status < 0)
				return -1;

			status = bitplane_decoding(v->s.mbskip_table, &v->skip_is_raw, v);
			if (status < 0)
				return -1;

			v->mbmodetab = get_bits(gb, 2);
			v->mbmode_vlc = ff_vc1_intfr_non4mv_mbmode_vlc[v->mbmodetab];
			v->imvtab = get_bits(gb, 2);
			v->imv_vlc = ff_vc1_1ref_mvdata_vlc[v->imvtab];
			// interlaced p/b-picture cbpcy range is [1, 63]
			v->icbptab = get_bits(gb, 3);
			v->cbpcy_vlc = ff_vc1_icbpcy_vlc[v->icbptab];
			v->twomvbptab = get_bits(gb, 2);
			v->twomvbp_vlc = ff_vc1_2mv_block_pattern_vlc[v->twomvbptab];
			v->fourmvbptab = get_bits(gb, 2);
			v->fourmvbp_vlc = ff_vc1_4mv_block_pattern_vlc[v->fourmvbptab];
		} else {
			v->mv_mode = get_bits1(gb) ? MV_PMODE_1MV : MV_PMODE_1MV_HPEL_BILIN;
			v->s.quarter_sample = (v->mv_mode == MV_PMODE_1MV);
			v->s.mspel = v->s.quarter_sample;
			status = bitplane_decoding(v->direct_mb_plane, &v->dmb_is_raw, v);
			if (status < 0)
				return -1;

			status = bitplane_decoding(v->s.mbskip_table, &v->skip_is_raw, v);
			if (status < 0)
				return -1;

			v->mv_table_index = get_bits(gb, 2);
			v->cbptab = get_bits(gb, 2);
			v->cbpcy_vlc = ff_vc1_cbpcy_p_vlc[v->cbptab];
		}

		if (v->dquant) {

			vop_dquant_decoding(v);
		}

		if (v->vstransform) {
			v->ttmbf = get_bits1(gb);
			if (v->ttmbf) {
				v->ttfrm = ff_vc1_ttfrm_to_tt[get_bits(gb, 2)];
			} else
				v->ttfrm = 0;
		} else {
			v->ttmbf = 1;
			v->ttfrm = TT_8X8;
		}
		break;
	}

	/* AC Syntax */
	v->c_ac_table_index = decode012(gb);
	if (v->s.pict_type == AV_PICTURE_TYPE_I || v->s.pict_type == AV_PICTURE_TYPE_BI) {
		v->y_ac_table_index = decode012(gb);
	} else if (v->fcm != PROGRESSIVE && !v->s.quarter_sample) {
		v->range_x <<= 1;
		v->range_y <<= 1;
	}

	/* DC Syntax */
	v->dc_table_index = get_bits1(gb);
	if ((v->s.pict_type == AV_PICTURE_TYPE_I || v->s.pict_type == AV_PICTURE_TYPE_BI) && v->dquant) {

		vop_dquant_decoding(v);
	}

	v->bi_type = (v->s.pict_type == AV_PICTURE_TYPE_BI);
	if (v->bi_type)
		v->s.pict_type = AV_PICTURE_TYPE_B;

	return 0;
}

#undef INIT_LUT
#undef ROTATE

static inline int scaleforsame_x(const VC1Context *v, int n /* MV */, int dir) {
	int scaledvalue, refdist;
	int scalesame1, scalesame2;
	int scalezone1_x, zone1offset_x;
	int table_index = dir ^ v->second_field;

	if (v->s.pict_type != AV_PICTURE_TYPE_B)
		refdist = v->refdist;
	else
		refdist = dir ? v->brfd : v->frfd;
	if (refdist > 3)
		refdist = 3;
	scalesame1 = ff_vc1_field_mvpred_scales[table_index][1][refdist];
	scalesame2 = ff_vc1_field_mvpred_scales[table_index][2][refdist];
	scalezone1_x = ff_vc1_field_mvpred_scales[table_index][3][refdist];
	zone1offset_x = ff_vc1_field_mvpred_scales[table_index][5][refdist];

	if (ABS(n) > 255)
		scaledvalue = n;
	else {
		if (ABS(n) < scalezone1_x)
			scaledvalue = (n * scalesame1) >> 8;
		else {
			if (n < 0)
				scaledvalue = ((n * scalesame2) >> 8) - zone1offset_x;
			else
				scaledvalue = ((n * scalesame2) >> 8) + zone1offset_x;
		}
	}
	return av_clip(scaledvalue, -v->range_x, v->range_x - 1);
}

static inline int scaleforsame_y(const VC1Context *v, int n /* MV */, int dir) {
	int scaledvalue, refdist;
	int scalesame1, scalesame2;
	int scalezone1_y, zone1offset_y;
	int table_index = dir ^ v->second_field;

	if (v->s.pict_type != AV_PICTURE_TYPE_B)
		refdist = v->refdist;
	else
		refdist = dir ? v->brfd : v->frfd;
	if (refdist > 3)
		refdist = 3;
	scalesame1 = ff_vc1_field_mvpred_scales[table_index][1][refdist];
	scalesame2 = ff_vc1_field_mvpred_scales[table_index][2][refdist];
	scalezone1_y = ff_vc1_field_mvpred_scales[table_index][4][refdist];
	zone1offset_y = ff_vc1_field_mvpred_scales[table_index][6][refdist];

	if (ABS(n) > 63)
		scaledvalue = n;
	else {
		if (ABS(n) < scalezone1_y)
			scaledvalue = (n * scalesame1) >> 8;
		else {
			if (n < 0)
				scaledvalue = ((n * scalesame2) >> 8) - zone1offset_y;
			else
				scaledvalue = ((n * scalesame2) >> 8) + zone1offset_y;
		}
	}

	if (v->cur_field_type && !v->ref_field_type[dir])
		return av_clip(scaledvalue, -v->range_y / 2 + 1, v->range_y / 2);
	else
		return av_clip(scaledvalue, -v->range_y / 2, v->range_y / 2 - 1);
}

static inline int scaleforopp_x(const VC1Context *v, int n /* MV */) {
	int scalezone1_x, zone1offset_x;
	int scaleopp1, scaleopp2, brfd;
	int scaledvalue;

	brfd = MIN(v->brfd, 3);
	scalezone1_x = ff_vc1_b_field_mvpred_scales[3][brfd];
	zone1offset_x = ff_vc1_b_field_mvpred_scales[5][brfd];
	scaleopp1 = ff_vc1_b_field_mvpred_scales[1][brfd];
	scaleopp2 = ff_vc1_b_field_mvpred_scales[2][brfd];

	if (ABS(n) > 255)
		scaledvalue = n;
	else {
		if (ABS(n) < scalezone1_x)
			scaledvalue = (n * scaleopp1) >> 8;
		else {
			if (n < 0)
				scaledvalue = ((n * scaleopp2) >> 8) - zone1offset_x;
			else
				scaledvalue = ((n * scaleopp2) >> 8) + zone1offset_x;
		}
	}
	return av_clip(scaledvalue, -v->range_x, v->range_x - 1);
}

static inline int scaleforopp_y(const VC1Context *v, int n /* MV */, int dir) {
	int scalezone1_y, zone1offset_y;
	int scaleopp1, scaleopp2, brfd;
	int scaledvalue;

	brfd = MIN(v->brfd, 3);
	scalezone1_y = ff_vc1_b_field_mvpred_scales[4][brfd];
	zone1offset_y = ff_vc1_b_field_mvpred_scales[6][brfd];
	scaleopp1 = ff_vc1_b_field_mvpred_scales[1][brfd];
	scaleopp2 = ff_vc1_b_field_mvpred_scales[2][brfd];

	if (ABS(n) > 63)
		scaledvalue = n;
	else {
		if (ABS(n) < scalezone1_y)
			scaledvalue = (n * scaleopp1) >> 8;
		else {
			if (n < 0)
				scaledvalue = ((n * scaleopp2) >> 8) - zone1offset_y;
			else
				scaledvalue = ((n * scaleopp2) >> 8) + zone1offset_y;
		}
	}
	if (v->cur_field_type && !v->ref_field_type[dir]) {
		return av_clip(scaledvalue, -v->range_y / 2 + 1, v->range_y / 2);
	} else {
		return av_clip(scaledvalue, -v->range_y / 2, v->range_y / 2 - 1);
	}
}

static inline int scaleforsame(const VC1Context *v, int n /* MV */, int dim, int dir) {
	int brfd, scalesame;
	int hpel = 1 - v->s.quarter_sample;

	n >>= hpel;
	if (v->s.pict_type != AV_PICTURE_TYPE_B || v->second_field || !dir) {
		if (dim)
			n = scaleforsame_y(v, n, dir) * (1 << hpel);
		else
			n = scaleforsame_x(v, n, dir) * (1 << hpel);
		return n;
	}
	brfd = MIN(v->brfd, 3);
	scalesame = ff_vc1_b_field_mvpred_scales[0][brfd];

	n = (n * scalesame >> 8) * (1 << hpel);
	return n;
}

static inline int scaleforopp(const VC1Context *v, int n /* MV */, int dim, int dir) {
	int refdist, scaleopp;
	int hpel = 1 - v->s.quarter_sample;

	n >>= hpel;
	if (v->s.pict_type == AV_PICTURE_TYPE_B && !v->second_field && dir == 1) {
		if (dim)
			n = scaleforopp_y(v, n, dir) * (1 << hpel);
		else
			n = scaleforopp_x(v, n) * (1 << hpel);
		return n;
	}
	if (v->s.pict_type != AV_PICTURE_TYPE_B)
		refdist = v->refdist;
	else
		refdist = dir ? v->brfd : v->frfd;
	refdist = MIN(refdist, 3);
	scaleopp = ff_vc1_field_mvpred_scales[dir ^ v->second_field][0][refdist];

	n = (n * scaleopp >> 8) * (1 << hpel);
	return n;
}

/** Predict and set motion vector
 */
void ff_vc1_pred_mv(VC1Context *v, int n, int dmv_x, int dmv_y, int mv1, int r_x, int r_y, uint8 *is_intra,
					int pred_flag, int dir) {
	PictureState *s = &v->s;
	int xy, wrap, off = 0;
	int px, py;
	int sum;
	int mixedmv_pic, num_samefield = 0, num_oppfield = 0;
	int opposite, a_f, b_f, c_f;
	int16 field_predA[2];
	int16 field_predB[2];
	int16 field_predC[2];
	int a_valid, b_valid, c_valid;
	int hybridmv_thresh, y_bias = 0;

	if (v->mv_mode == MV_PMODE_MIXED_MV ||
		((v->mv_mode == MV_PMODE_INTENSITY_COMP) && (v->mv_mode2 == MV_PMODE_MIXED_MV)))
		mixedmv_pic = 1;
	else
		mixedmv_pic = 0;
	/* scale MV difference to be quad-pel */
	if (!s->quarter_sample) {
		dmv_x *= 2;
		dmv_y *= 2;
	}

	wrap = s->b8_stride;
	xy = s->block_index[n];

	if (s->mb_intra) {
		s->mv[0][n][0] = s->cur_pic.motion_val[0][xy + v->blocks_off][0] = 0;
		s->mv[0][n][1] = s->cur_pic.motion_val[0][xy + v->blocks_off][1] = 0;
		s->cur_pic.motion_val[1][xy + v->blocks_off][0] = 0;
		s->cur_pic.motion_val[1][xy + v->blocks_off][1] = 0;
		if (mv1) { /* duplicate motion data for 1-MV block */
			s->cur_pic.motion_val[0][xy + 1 + v->blocks_off][0] = 0;
			s->cur_pic.motion_val[0][xy + 1 + v->blocks_off][1] = 0;
			s->cur_pic.motion_val[0][xy + wrap + v->blocks_off][0] = 0;
			s->cur_pic.motion_val[0][xy + wrap + v->blocks_off][1] = 0;
			s->cur_pic.motion_val[0][xy + wrap + 1 + v->blocks_off][0] = 0;
			s->cur_pic.motion_val[0][xy + wrap + 1 + v->blocks_off][1] = 0;
			v->luma_mv[s->mb_x][0] = v->luma_mv[s->mb_x][1] = 0;
			s->cur_pic.motion_val[1][xy + 1 + v->blocks_off][0] = 0;
			s->cur_pic.motion_val[1][xy + 1 + v->blocks_off][1] = 0;
			s->cur_pic.motion_val[1][xy + wrap + v->blocks_off][0] = 0;
			s->cur_pic.motion_val[1][xy + wrap + v->blocks_off][1] = 0;
			s->cur_pic.motion_val[1][xy + wrap + 1 + v->blocks_off][0] = 0;
			s->cur_pic.motion_val[1][xy + wrap + 1 + v->blocks_off][1] = 0;
		}
		return;
	}

	a_valid = !s->first_slice_line || (n == 2 || n == 3);
	b_valid = a_valid;
	c_valid = s->mb_x || (n == 1 || n == 3);
	if (mv1) {
		if (v->field_mode && mixedmv_pic)
			off = (s->mb_x == (s->mb_width - 1)) ? -2 : 2;
		else
			off = (s->mb_x == (s->mb_width - 1)) ? -1 : 2;
		b_valid = b_valid && s->mb_width > 1;
	} else {
		// in 4-MV mode different blocks have different B predictor position
		switch (n) {
		case 0:
			if (v->res_rtm_flag)
				off = s->mb_x ? -1 : 1;
			else
				off = s->mb_x ? -1 : 2 * s->mb_width - wrap - 1;
			break;
		case 1:
			off = (s->mb_x == (s->mb_width - 1)) ? -1 : 1;
			break;
		case 2:
			off = 1;
			break;
		case 3:
			off = -1;
		}
		if (v->field_mode && s->mb_width == 1)
			b_valid = b_valid && c_valid;
	}

	if (v->field_mode) {
		a_valid = a_valid && !is_intra[xy - wrap];
		b_valid = b_valid && !is_intra[xy - wrap + off];
		c_valid = c_valid && !is_intra[xy - 1];
	}

	if (a_valid) {
		const int16 *A = s->cur_pic.motion_val[dir][xy - wrap + v->blocks_off];
		a_f = v->mv_f[dir][xy - wrap + v->blocks_off];
		num_oppfield += a_f;
		num_samefield += 1 - a_f;
		field_predA[0] = A[0];
		field_predA[1] = A[1];
	} else {
		field_predA[0] = field_predA[1] = 0;
		a_f = 0;
	}
	if (b_valid) {
		const int16 *B = s->cur_pic.motion_val[dir][xy - wrap + off + v->blocks_off];
		b_f = v->mv_f[dir][xy - wrap + off + v->blocks_off];
		num_oppfield += b_f;
		num_samefield += 1 - b_f;
		field_predB[0] = B[0];
		field_predB[1] = B[1];
	} else {
		field_predB[0] = field_predB[1] = 0;
		b_f = 0;
	}
	if (c_valid) {
		const int16 *C = s->cur_pic.motion_val[dir][xy - 1 + v->blocks_off];
		c_f = v->mv_f[dir][xy - 1 + v->blocks_off];
		num_oppfield += c_f;
		num_samefield += 1 - c_f;
		field_predC[0] = C[0];
		field_predC[1] = C[1];
	} else {
		field_predC[0] = field_predC[1] = 0;
		c_f = 0;
	}

	if (v->field_mode) {
		if (!v->numref)
			// REFFIELD determines if the last field or the second-last field is
			// to be used as reference
			opposite = 1 - v->reffield;
		else {
			if (num_samefield <= num_oppfield)
				opposite = 1 - pred_flag;
			else
				opposite = pred_flag;
		}
	} else
		opposite = 0;
	if (opposite) {
		v->mv_f[dir][xy + v->blocks_off] = 1;
		v->ref_field_type[dir] = !v->cur_field_type;
		if (a_valid && !a_f) {
			field_predA[0] = scaleforopp(v, field_predA[0], 0, dir);
			field_predA[1] = scaleforopp(v, field_predA[1], 1, dir);
		}
		if (b_valid && !b_f) {
			field_predB[0] = scaleforopp(v, field_predB[0], 0, dir);
			field_predB[1] = scaleforopp(v, field_predB[1], 1, dir);
		}
		if (c_valid && !c_f) {
			field_predC[0] = scaleforopp(v, field_predC[0], 0, dir);
			field_predC[1] = scaleforopp(v, field_predC[1], 1, dir);
		}
	} else {
		v->mv_f[dir][xy + v->blocks_off] = 0;
		v->ref_field_type[dir] = v->cur_field_type;
		if (a_valid && a_f) {
			field_predA[0] = scaleforsame(v, field_predA[0], 0, dir);
			field_predA[1] = scaleforsame(v, field_predA[1], 1, dir);
		}
		if (b_valid && b_f) {
			field_predB[0] = scaleforsame(v, field_predB[0], 0, dir);
			field_predB[1] = scaleforsame(v, field_predB[1], 1, dir);
		}
		if (c_valid && c_f) {
			field_predC[0] = scaleforsame(v, field_predC[0], 0, dir);
			field_predC[1] = scaleforsame(v, field_predC[1], 1, dir);
		}
	}

	if (a_valid) {
		px = field_predA[0];
		py = field_predA[1];
	} else if (c_valid) {
		px = field_predC[0];
		py = field_predC[1];
	} else if (b_valid) {
		px = field_predB[0];
		py = field_predB[1];
	} else {
		px = 0;
		py = 0;
	}

	if (num_samefield + num_oppfield > 1) {
		px = mid_pred(field_predA[0], field_predB[0], field_predC[0]);
		py = mid_pred(field_predA[1], field_predB[1], field_predC[1]);
	}

	/* Pullback MV as specified in 8.3.5.3.4 */
	if (!v->field_mode) {
		int qx, qy, X, Y;
		int MV = mv1 ? -60 : -28;
		qx = (s->mb_x << 6) + ((n == 1 || n == 3) ? 32 : 0);
		qy = (s->mb_y << 6) + ((n == 2 || n == 3) ? 32 : 0);
		X = (s->mb_width << 6) - 4;
		Y = (s->mb_height << 6) - 4;
		if (qx + px < MV)
			px = MV - qx;
		if (qy + py < MV)
			py = MV - qy;
		if (qx + px > X)
			px = X - qx;
		if (qy + py > Y)
			py = Y - qy;
	}

	if (!v->field_mode || s->pict_type != AV_PICTURE_TYPE_B) {
		/* Calculate hybrid prediction as specified in 8.3.5.3.5 (also 10.3.5.4.3.5) */
		hybridmv_thresh = 32;
		if (a_valid && c_valid) {
			if (is_intra[xy - wrap])
				sum = ABS(px) + ABS(py);
			else
				sum = ABS(px - field_predA[0]) + ABS(py - field_predA[1]);
			if (sum > hybridmv_thresh) {
				if (get_bits1(&v->gb)) { // read HYBRIDPRED bit
					px = field_predA[0];
					py = field_predA[1];
				} else {
					px = field_predC[0];
					py = field_predC[1];
				}
			} else {
				if (is_intra[xy - 1])
					sum = ABS(px) + ABS(py);
				else
					sum = ABS(px - field_predC[0]) + ABS(py - field_predC[1]);
				if (sum > hybridmv_thresh) {
					if (get_bits1(&v->gb)) {
						px = field_predA[0];
						py = field_predA[1];
					} else {
						px = field_predC[0];
						py = field_predC[1];
					}
				}
			}
		}
	}

	if (v->field_mode && v->numref)
		r_y >>= 1;
	if (v->field_mode && v->cur_field_type && v->ref_field_type[dir] == 0)
		y_bias = 1;
	/* store MV using signed modulus of MV range defined in 4.11 */
	s->mv[dir][n][0] = s->cur_pic.motion_val[dir][xy + v->blocks_off][0] =
		((px + dmv_x + r_x) & ((r_x << 1) - 1)) - r_x;
	s->mv[dir][n][1] = s->cur_pic.motion_val[dir][xy + v->blocks_off][1] =
		((py + dmv_y + r_y - y_bias) & ((r_y << 1) - 1)) - r_y + y_bias;
	if (mv1) { /* duplicate motion data for 1-MV block */
		s->cur_pic.motion_val[dir][xy + 1 + v->blocks_off][0] = s->cur_pic.motion_val[dir][xy + v->blocks_off][0];
		s->cur_pic.motion_val[dir][xy + 1 + v->blocks_off][1] = s->cur_pic.motion_val[dir][xy + v->blocks_off][1];
		s->cur_pic.motion_val[dir][xy + wrap + v->blocks_off][0] = s->cur_pic.motion_val[dir][xy + v->blocks_off][0];
		s->cur_pic.motion_val[dir][xy + wrap + v->blocks_off][1] = s->cur_pic.motion_val[dir][xy + v->blocks_off][1];
		s->cur_pic.motion_val[dir][xy + wrap + 1 + v->blocks_off][0] =
			s->cur_pic.motion_val[dir][xy + v->blocks_off][0];
		s->cur_pic.motion_val[dir][xy + wrap + 1 + v->blocks_off][1] =
			s->cur_pic.motion_val[dir][xy + v->blocks_off][1];
		v->mv_f[dir][xy + 1 + v->blocks_off] = v->mv_f[dir][xy + v->blocks_off];
		v->mv_f[dir][xy + wrap + v->blocks_off] = v->mv_f[dir][xy + wrap + 1 + v->blocks_off] =
			v->mv_f[dir][xy + v->blocks_off];
	}
}

/** Predict and set motion vector for interlaced frame picture MBs
 */
void ff_vc1_pred_mv_intfr(VC1Context *v, int n, int dmv_x, int dmv_y, int mvn, int r_x, int r_y, int dir) {
	PictureState *s = &v->s;
	int xy, wrap, off = 0;
	int A[2], B[2], C[2];
	int px = 0, py = 0;
	int a_valid = 0, b_valid = 0, c_valid = 0;
	int field_a, field_b, field_c; // 0: same, 1: opposite
	int total_valid, num_samefield, num_oppfield;
	int pos_c, pos_b, n_adj;

	wrap = s->b8_stride;
	xy = s->block_index[n];

	if (s->mb_intra) {
		s->mv[0][n][0] = s->cur_pic.motion_val[0][xy][0] = 0;
		s->mv[0][n][1] = s->cur_pic.motion_val[0][xy][1] = 0;
		s->cur_pic.motion_val[1][xy][0] = 0;
		s->cur_pic.motion_val[1][xy][1] = 0;
		if (mvn == 1) { /* duplicate motion data for 1-MV block */
			s->cur_pic.motion_val[0][xy + 1][0] = 0;
			s->cur_pic.motion_val[0][xy + 1][1] = 0;
			s->cur_pic.motion_val[0][xy + wrap][0] = 0;
			s->cur_pic.motion_val[0][xy + wrap][1] = 0;
			s->cur_pic.motion_val[0][xy + wrap + 1][0] = 0;
			s->cur_pic.motion_val[0][xy + wrap + 1][1] = 0;
			v->luma_mv[s->mb_x][0] = v->luma_mv[s->mb_x][1] = 0;
			s->cur_pic.motion_val[1][xy + 1][0] = 0;
			s->cur_pic.motion_val[1][xy + 1][1] = 0;
			s->cur_pic.motion_val[1][xy + wrap][0] = 0;
			s->cur_pic.motion_val[1][xy + wrap][1] = 0;
			s->cur_pic.motion_val[1][xy + wrap + 1][0] = 0;
			s->cur_pic.motion_val[1][xy + wrap + 1][1] = 0;
		}
		return;
	}

	off = ((n == 0) || (n == 1)) ? 1 : -1;
	/* predict A */
	if (s->mb_x || (n == 1) || (n == 3)) {
		if ((v->blk_mv_type[xy])                                   // current block (MB) has a field MV
			|| (!v->blk_mv_type[xy] && !v->blk_mv_type[xy - 1])) { // or both have frame MV
			A[0] = s->cur_pic.motion_val[dir][xy - 1][0];
			A[1] = s->cur_pic.motion_val[dir][xy - 1][1];
			a_valid = 1;
		} else { // current block has frame mv and cand. has field MV (so average)
			A[0] =
				(s->cur_pic.motion_val[dir][xy - 1][0] + s->cur_pic.motion_val[dir][xy - 1 + off * wrap][0] + 1) >> 1;
			A[1] =
				(s->cur_pic.motion_val[dir][xy - 1][1] + s->cur_pic.motion_val[dir][xy - 1 + off * wrap][1] + 1) >> 1;
			a_valid = 1;
		}
		if (!(n & 1) && v->is_intra[s->mb_x - 1]) {
			a_valid = 0;
			A[0] = A[1] = 0;
		}
	} else
		A[0] = A[1] = 0;
	/* Predict B and C */
	B[0] = B[1] = C[0] = C[1] = 0;
	if (n == 0 || n == 1 || v->blk_mv_type[xy]) {
		if (!s->first_slice_line) {
			if (!v->is_intra[s->mb_x - s->mb_stride]) {
				b_valid = 1;
				n_adj = n | 2;
				pos_b = s->block_index[n_adj] - 2 * wrap;
				if (v->blk_mv_type[pos_b] && v->blk_mv_type[xy]) {
					n_adj = (n & 2) | (n & 1);
				}
				B[0] = s->cur_pic.motion_val[dir][s->block_index[n_adj] - 2 * wrap][0];
				B[1] = s->cur_pic.motion_val[dir][s->block_index[n_adj] - 2 * wrap][1];
				if (v->blk_mv_type[pos_b] && !v->blk_mv_type[xy]) {
					B[0] = (B[0] + s->cur_pic.motion_val[dir][s->block_index[n_adj ^ 2] - 2 * wrap][0] + 1) >> 1;
					B[1] = (B[1] + s->cur_pic.motion_val[dir][s->block_index[n_adj ^ 2] - 2 * wrap][1] + 1) >> 1;
				}
			}
			if (s->mb_width > 1) {
				if (!v->is_intra[s->mb_x - s->mb_stride + 1]) {
					c_valid = 1;
					n_adj = 2;
					pos_c = s->block_index[2] - 2 * wrap + 2;
					if (v->blk_mv_type[pos_c] && v->blk_mv_type[xy]) {
						n_adj = n & 2;
					}
					C[0] = s->cur_pic.motion_val[dir][s->block_index[n_adj] - 2 * wrap + 2][0];
					C[1] = s->cur_pic.motion_val[dir][s->block_index[n_adj] - 2 * wrap + 2][1];
					if (v->blk_mv_type[pos_c] && !v->blk_mv_type[xy]) {
						C[0] =
							(1 + C[0] + (s->cur_pic.motion_val[dir][s->block_index[n_adj ^ 2] - 2 * wrap + 2][0])) >> 1;
						C[1] =
							(1 + C[1] + (s->cur_pic.motion_val[dir][s->block_index[n_adj ^ 2] - 2 * wrap + 2][1])) >> 1;
					}
					if (s->mb_x == s->mb_width - 1) {
						if (!v->is_intra[s->mb_x - s->mb_stride - 1]) {
							c_valid = 1;
							n_adj = 3;
							pos_c = s->block_index[3] - 2 * wrap - 2;
							if (v->blk_mv_type[pos_c] && v->blk_mv_type[xy]) {
								n_adj = n | 1;
							}
							C[0] = s->cur_pic.motion_val[dir][s->block_index[n_adj] - 2 * wrap - 2][0];
							C[1] = s->cur_pic.motion_val[dir][s->block_index[n_adj] - 2 * wrap - 2][1];
							if (v->blk_mv_type[pos_c] && !v->blk_mv_type[xy]) {
								C[0] =
									(1 + C[0] + s->cur_pic.motion_val[dir][s->block_index[1] - 2 * wrap - 2][0]) >> 1;
								C[1] =
									(1 + C[1] + s->cur_pic.motion_val[dir][s->block_index[1] - 2 * wrap - 2][1]) >> 1;
							}
						} else
							c_valid = 0;
					}
				}
			}
		}
	} else {
		pos_b = s->block_index[1];
		b_valid = 1;
		B[0] = s->cur_pic.motion_val[dir][pos_b][0];
		B[1] = s->cur_pic.motion_val[dir][pos_b][1];
		pos_c = s->block_index[0];
		c_valid = 1;
		C[0] = s->cur_pic.motion_val[dir][pos_c][0];
		C[1] = s->cur_pic.motion_val[dir][pos_c][1];
	}

	total_valid = a_valid + b_valid + c_valid;
	// check if predictor A is out of bounds
	if (!s->mb_x && !(n == 1 || n == 3)) {
		A[0] = A[1] = 0;
	}
	// check if predictor B is out of bounds
	if ((s->first_slice_line && v->blk_mv_type[xy]) || (s->first_slice_line && !(n & 2))) {
		B[0] = B[1] = C[0] = C[1] = 0;
	}
	if (!v->blk_mv_type[xy]) {
		if (s->mb_width == 1) {
			px = B[0];
			py = B[1];
		} else {
			if (total_valid >= 2) {
				px = mid_pred(A[0], B[0], C[0]);
				py = mid_pred(A[1], B[1], C[1]);
			} else if (total_valid) {
				if (a_valid) {
					px = A[0];
					py = A[1];
				} else if (b_valid) {
					px = B[0];
					py = B[1];
				} else {
					px = C[0];
					py = C[1];
				}
			}
		}
	} else {
		if (a_valid)
			field_a = (A[1] & 4) ? 1 : 0;
		else
			field_a = 0;
		if (b_valid)
			field_b = (B[1] & 4) ? 1 : 0;
		else
			field_b = 0;
		if (c_valid)
			field_c = (C[1] & 4) ? 1 : 0;
		else
			field_c = 0;

		num_oppfield = field_a + field_b + field_c;
		num_samefield = total_valid - num_oppfield;
		if (total_valid == 3) {
			if ((num_samefield == 3) || (num_oppfield == 3)) {
				px = mid_pred(A[0], B[0], C[0]);
				py = mid_pred(A[1], B[1], C[1]);
			} else if (num_samefield >= num_oppfield) {
				/* take one MV from same field set depending on priority
				the check for B may not be necessary */
				px = !field_a ? A[0] : B[0];
				py = !field_a ? A[1] : B[1];
			} else {
				px = field_a ? A[0] : B[0];
				py = field_a ? A[1] : B[1];
			}
		} else if (total_valid == 2) {
			if (num_samefield >= num_oppfield) {
				if (!field_a && a_valid) {
					px = A[0];
					py = A[1];
				} else if (!field_b && b_valid) {
					px = B[0];
					py = B[1];
				} else /*if (c_valid)*/ {
					assert(c_valid);
					px = C[0];
					py = C[1];
				}
			} else {
				if (field_a && a_valid) {
					px = A[0];
					py = A[1];
				} else /*if (field_b && b_valid)*/ {
					assert(field_b && b_valid);
					px = B[0];
					py = B[1];
				}
			}
		} else if (total_valid == 1) {
			px = (a_valid) ? A[0] : ((b_valid) ? B[0] : C[0]);
			py = (a_valid) ? A[1] : ((b_valid) ? B[1] : C[1]);
		}
	}

	/* store MV using signed modulus of MV range defined in 4.11 */
	s->mv[dir][n][0] = s->cur_pic.motion_val[dir][xy][0] = ((px + dmv_x + r_x) & ((r_x << 1) - 1)) - r_x;
	s->mv[dir][n][1] = s->cur_pic.motion_val[dir][xy][1] = ((py + dmv_y + r_y) & ((r_y << 1) - 1)) - r_y;
	if (mvn == 1) { /* duplicate motion data for 1-MV block */
		s->cur_pic.motion_val[dir][xy + 1][0] = s->cur_pic.motion_val[dir][xy][0];
		s->cur_pic.motion_val[dir][xy + 1][1] = s->cur_pic.motion_val[dir][xy][1];
		s->cur_pic.motion_val[dir][xy + wrap][0] = s->cur_pic.motion_val[dir][xy][0];
		s->cur_pic.motion_val[dir][xy + wrap][1] = s->cur_pic.motion_val[dir][xy][1];
		s->cur_pic.motion_val[dir][xy + wrap + 1][0] = s->cur_pic.motion_val[dir][xy][0];
		s->cur_pic.motion_val[dir][xy + wrap + 1][1] = s->cur_pic.motion_val[dir][xy][1];
	} else if (mvn == 2) { /* duplicate motion data for 2-Field MV block */
		s->cur_pic.motion_val[dir][xy + 1][0] = s->cur_pic.motion_val[dir][xy][0];
		s->cur_pic.motion_val[dir][xy + 1][1] = s->cur_pic.motion_val[dir][xy][1];
		s->mv[dir][n + 1][0] = s->mv[dir][n][0];
		s->mv[dir][n + 1][1] = s->mv[dir][n][1];
	}
}

void ff_vc1_pred_b_mv(VC1Context *v, int dmv_x[2], int dmv_y[2], int direct, int mvtype) {
	PictureState *s = &v->s;
	int xy, wrap;
	int px, py;
	int sum;
	int r_x, r_y;
	const uint8 *is_intra = v->mb_type;

	assert(!v->field_mode);

	r_x = v->range_x;
	r_y = v->range_y;
	/* scale MV difference to be quad-pel */
	if (!s->quarter_sample) {
		dmv_x[0] *= 2;
		dmv_y[0] *= 2;
		dmv_x[1] *= 2;
		dmv_y[1] *= 2;
	}

	wrap = s->b8_stride;
	xy = s->block_index[0];

	if (s->mb_intra) {
		s->cur_pic.motion_val[0][xy][0] = s->cur_pic.motion_val[0][xy][1] = s->cur_pic.motion_val[1][xy][0] =
			s->cur_pic.motion_val[1][xy][1] = 0;
		return;
	}
	if (direct && s->next_pic.ptr->field_picture)
		warning("Mixed frame/field direct mode not supported\n");

	s->mv[0][0][0] = scale_mv(s->next_pic.motion_val[1][xy][0], v->bfraction, 0, s->quarter_sample);
	s->mv[0][0][1] = scale_mv(s->next_pic.motion_val[1][xy][1], v->bfraction, 0, s->quarter_sample);
	s->mv[1][0][0] = scale_mv(s->next_pic.motion_val[1][xy][0], v->bfraction, 1, s->quarter_sample);
	s->mv[1][0][1] = scale_mv(s->next_pic.motion_val[1][xy][1], v->bfraction, 1, s->quarter_sample);

	/* Pullback predicted motion vectors as specified in 8.4.5.4 */
	s->mv[0][0][0] = av_clip(s->mv[0][0][0], -60 - (s->mb_x << 6), (s->mb_width << 6) - 4 - (s->mb_x << 6));
	s->mv[0][0][1] = av_clip(s->mv[0][0][1], -60 - (s->mb_y << 6), (s->mb_height << 6) - 4 - (s->mb_y << 6));
	s->mv[1][0][0] = av_clip(s->mv[1][0][0], -60 - (s->mb_x << 6), (s->mb_width << 6) - 4 - (s->mb_x << 6));
	s->mv[1][0][1] = av_clip(s->mv[1][0][1], -60 - (s->mb_y << 6), (s->mb_height << 6) - 4 - (s->mb_y << 6));
	if (direct) {
		s->cur_pic.motion_val[0][xy][0] = s->mv[0][0][0];
		s->cur_pic.motion_val[0][xy][1] = s->mv[0][0][1];
		s->cur_pic.motion_val[1][xy][0] = s->mv[1][0][0];
		s->cur_pic.motion_val[1][xy][1] = s->mv[1][0][1];
		return;
	}

	if ((mvtype == BMV_TYPE_FORWARD) || (mvtype == BMV_TYPE_INTERPOLATED)) {
		int16 *C = s->cur_pic.motion_val[0][xy - 2];
		const int16 *A = s->cur_pic.motion_val[0][xy - wrap * 2];
		int off = (s->mb_x == (s->mb_width - 1)) ? -2 : 2;
		const int16 *B = s->cur_pic.motion_val[0][xy - wrap * 2 + off];

		if (!s->mb_x)
			C[0] = C[1] = 0;
		if (!s->first_slice_line) { // predictor A is not out of bounds
			if (s->mb_width == 1) {
				px = A[0];
				py = A[1];
			} else {
				px = mid_pred(A[0], B[0], C[0]);
				py = mid_pred(A[1], B[1], C[1]);
			}
		} else if (s->mb_x) { // predictor C is not out of bounds
			px = C[0];
			py = C[1];
		} else {
			px = py = 0;
		}
		/* Pullback MV as specified in 8.3.5.3.4 */
		{
			int qx, qy, X, Y;
			int sh = v->profile < PROFILE_ADVANCED ? 5 : 6;
			int MV = 4 - (1 << sh);
			qx = (s->mb_x << sh);
			qy = (s->mb_y << sh);
			X = (s->mb_width << sh) - 4;
			Y = (s->mb_height << sh) - 4;
			if (qx + px < MV)
				px = MV - qx;
			if (qy + py < MV)
				py = MV - qy;
			if (qx + px > X)
				px = X - qx;
			if (qy + py > Y)
				py = Y - qy;
		}
		/* Calculate hybrid prediction as specified in 8.3.5.3.5 */
		if (0 && !s->first_slice_line && s->mb_x) {
			if (is_intra[xy - wrap])
				sum = ABS(px) + ABS(py);
			else
				sum = ABS(px - A[0]) + ABS(py - A[1]);
			if (sum > 32) {
				if (get_bits1(&v->gb)) {
					px = A[0];
					py = A[1];
				} else {
					px = C[0];
					py = C[1];
				}
			} else {
				if (is_intra[xy - 2])
					sum = ABS(px) + ABS(py);
				else
					sum = ABS(px - C[0]) + ABS(py - C[1]);
				if (sum > 32) {
					if (get_bits1(&v->gb)) {
						px = A[0];
						py = A[1];
					} else {
						px = C[0];
						py = C[1];
					}
				}
			}
		}
		/* store MV using signed modulus of MV range defined in 4.11 */
		s->mv[0][0][0] = ((px + dmv_x[0] + r_x) & ((r_x << 1) - 1)) - r_x;
		s->mv[0][0][1] = ((py + dmv_y[0] + r_y) & ((r_y << 1) - 1)) - r_y;
	}
	if ((mvtype == BMV_TYPE_BACKWARD) || (mvtype == BMV_TYPE_INTERPOLATED)) {
		int16 *C = s->cur_pic.motion_val[1][xy - 2];
		const int16 *A = s->cur_pic.motion_val[1][xy - wrap * 2];
		int off = (s->mb_x == (s->mb_width - 1)) ? -2 : 2;
		const int16 *B = s->cur_pic.motion_val[1][xy - wrap * 2 + off];

		if (!s->mb_x)
			C[0] = C[1] = 0;
		if (!s->first_slice_line) { // predictor A is not out of bounds
			if (s->mb_width == 1) {
				px = A[0];
				py = A[1];
			} else {
				px = mid_pred(A[0], B[0], C[0]);
				py = mid_pred(A[1], B[1], C[1]);
			}
		} else if (s->mb_x) { // predictor C is not out of bounds
			px = C[0];
			py = C[1];
		} else {
			px = py = 0;
		}
		/* Pullback MV as specified in 8.3.5.3.4 */
		{
			int qx, qy, X, Y;
			int sh = v->profile < PROFILE_ADVANCED ? 5 : 6;
			int MV = 4 - (1 << sh);
			qx = (s->mb_x << sh);
			qy = (s->mb_y << sh);
			X = (s->mb_width << sh) - 4;
			Y = (s->mb_height << sh) - 4;
			if (qx + px < MV)
				px = MV - qx;
			if (qy + py < MV)
				py = MV - qy;
			if (qx + px > X)
				px = X - qx;
			if (qy + py > Y)
				py = Y - qy;
		}
		/* Calculate hybrid prediction as specified in 8.3.5.3.5 */
		if (0 && !s->first_slice_line && s->mb_x) {
			if (is_intra[xy - wrap])
				sum = ABS(px) + ABS(py);
			else
				sum = ABS(px - A[0]) + ABS(py - A[1]);
			if (sum > 32) {
				if (get_bits1(&v->gb)) {
					px = A[0];
					py = A[1];
				} else {
					px = C[0];
					py = C[1];
				}
			} else {
				if (is_intra[xy - 2])
					sum = ABS(px) + ABS(py);
				else
					sum = ABS(px - C[0]) + ABS(py - C[1]);
				if (sum > 32) {
					if (get_bits1(&v->gb)) {
						px = A[0];
						py = A[1];
					} else {
						px = C[0];
						py = C[1];
					}
				}
			}
		}
		/* store MV using signed modulus of MV range defined in 4.11 */

		s->mv[1][0][0] = ((px + dmv_x[1] + r_x) & ((r_x << 1) - 1)) - r_x;
		s->mv[1][0][1] = ((py + dmv_y[1] + r_y) & ((r_y << 1) - 1)) - r_y;
	}
	s->cur_pic.motion_val[0][xy][0] = s->mv[0][0][0];
	s->cur_pic.motion_val[0][xy][1] = s->mv[0][0][1];
	s->cur_pic.motion_val[1][xy][0] = s->mv[1][0][0];
	s->cur_pic.motion_val[1][xy][1] = s->mv[1][0][1];
}

void ff_vc1_pred_b_mv_intfi(VC1Context *v, int n, int *dmv_x, int *dmv_y, int mv1, int *pred_flag) {
	int dir = (v->bmvtype == BMV_TYPE_BACKWARD) ? 1 : 0;
	PictureState *s = &v->s;
	int mb_pos = s->mb_x + s->mb_y * s->mb_stride;

	if (v->bmvtype == BMV_TYPE_DIRECT) {
		int total_opp, k, f;
		if (s->next_pic.mb_type[mb_pos + v->mb_off] != MB_TYPE_INTRA) {
			s->mv[0][0][0] = scale_mv(s->next_pic.motion_val[1][s->block_index[0] + v->blocks_off][0], v->bfraction, 0,
									  s->quarter_sample);
			s->mv[0][0][1] = scale_mv(s->next_pic.motion_val[1][s->block_index[0] + v->blocks_off][1], v->bfraction, 0,
									  s->quarter_sample);
			s->mv[1][0][0] = scale_mv(s->next_pic.motion_val[1][s->block_index[0] + v->blocks_off][0], v->bfraction, 1,
									  s->quarter_sample);
			s->mv[1][0][1] = scale_mv(s->next_pic.motion_val[1][s->block_index[0] + v->blocks_off][1], v->bfraction, 1,
									  s->quarter_sample);

			total_opp = v->mv_f_next[0][s->block_index[0] + v->blocks_off] +
						v->mv_f_next[0][s->block_index[1] + v->blocks_off] +
						v->mv_f_next[0][s->block_index[2] + v->blocks_off] +
						v->mv_f_next[0][s->block_index[3] + v->blocks_off];
			f = (total_opp > 2) ? 1 : 0;
		} else {
			s->mv[0][0][0] = s->mv[0][0][1] = 0;
			s->mv[1][0][0] = s->mv[1][0][1] = 0;
			f = 0;
		}
		v->ref_field_type[0] = v->ref_field_type[1] = v->cur_field_type ^ f;
		for (k = 0; k < 4; k++) {
			s->cur_pic.motion_val[0][s->block_index[k] + v->blocks_off][0] = s->mv[0][0][0];
			s->cur_pic.motion_val[0][s->block_index[k] + v->blocks_off][1] = s->mv[0][0][1];
			s->cur_pic.motion_val[1][s->block_index[k] + v->blocks_off][0] = s->mv[1][0][0];
			s->cur_pic.motion_val[1][s->block_index[k] + v->blocks_off][1] = s->mv[1][0][1];
			v->mv_f[0][s->block_index[k] + v->blocks_off] = f;
			v->mv_f[1][s->block_index[k] + v->blocks_off] = f;
		}
		return;
	}
	if (v->bmvtype == BMV_TYPE_INTERPOLATED) {
		ff_vc1_pred_mv(v, 0, dmv_x[0], dmv_y[0], 1, v->range_x, v->range_y, v->mb_type, pred_flag[0], 0);
		ff_vc1_pred_mv(v, 0, dmv_x[1], dmv_y[1], 1, v->range_x, v->range_y, v->mb_type, pred_flag[1], 1);
		return;
	}
	if (dir) { // backward
		ff_vc1_pred_mv(v, n, dmv_x[1], dmv_y[1], mv1, v->range_x, v->range_y, v->mb_type, pred_flag[1], 1);
		if (n == 3 || mv1) {
			ff_vc1_pred_mv(v, 0, dmv_x[0], dmv_y[0], 1, v->range_x, v->range_y, v->mb_type, 0, 0);
		}
	} else { // forward
		ff_vc1_pred_mv(v, n, dmv_x[0], dmv_y[0], mv1, v->range_x, v->range_y, v->mb_type, pred_flag[0], 0);
		if (n == 3 || mv1) {
			ff_vc1_pred_mv(v, 0, dmv_x[1], dmv_y[1], 1, v->range_x, v->range_y, v->mb_type, 0, 1);
		}
	}
}

static inline void vc1_h_overlap_filter(VC1Context *v, int16 (*left_block)[64], int16 (*right_block)[64],
										int left_fieldtx, int right_fieldtx, int block_num) {
	switch (block_num) {
	case 0:
		v->vc1dsp.vc1_h_s_overlap(
			left_block[2], right_block[0], left_fieldtx ^ right_fieldtx ? 16 - 8 * left_fieldtx : 8,
			left_fieldtx ^ right_fieldtx ? 16 - 8 * right_fieldtx : 8, left_fieldtx || right_fieldtx ? 0 : 1);
		break;

	case 1:
		v->vc1dsp.vc1_h_s_overlap(right_block[0], right_block[2], 8, 8, right_fieldtx ? 0 : 1);
		break;

	case 2:
		v->vc1dsp.vc1_h_s_overlap(!left_fieldtx && right_fieldtx ? left_block[2] + 8 : left_block[3],
								  left_fieldtx && !right_fieldtx ? right_block[0] + 8 : right_block[1],
								  left_fieldtx ^ right_fieldtx ? 16 - 8 * left_fieldtx : 8,
								  left_fieldtx ^ right_fieldtx ? 16 - 8 * right_fieldtx : 8,
								  left_fieldtx || right_fieldtx ? 2 : 1);
		break;

	case 3:
		v->vc1dsp.vc1_h_s_overlap(right_block[1], right_block[3], 8, 8, right_fieldtx ? 2 : 1);
		break;

	case 4:
	case 5:
		v->vc1dsp.vc1_h_s_overlap(left_block[block_num], right_block[block_num], 8, 8, 1);
		break;
	}
}

static inline void vc1_v_overlap_filter(VC1Context *v, int16 (*top_block)[64], int16 (*bottom_block)[64],
										int block_num) {
	switch (block_num) {
	case 0:
		v->vc1dsp.vc1_v_s_overlap(top_block[1], bottom_block[0]);
		break;

	case 1:
		v->vc1dsp.vc1_v_s_overlap(top_block[3], bottom_block[2]);
		break;

	case 2:
		v->vc1dsp.vc1_v_s_overlap(bottom_block[0], bottom_block[1]);
		break;

	case 3:
		v->vc1dsp.vc1_v_s_overlap(bottom_block[2], bottom_block[3]);
		break;

	case 4:
	case 5:
		v->vc1dsp.vc1_v_s_overlap(top_block[block_num], bottom_block[block_num]);
		break;
	}
}

void ff_vc1_i_overlap_filter(VC1Context *v) {
	PictureState *s = &v->s;
	int16(*topleft_blk)[64], (*top_blk)[64], (*left_blk)[64], (*cur_blk)[64];
	int block_count = CONFIG_GRAY && (s->avctx->flags & AV_CODEC_FLAG_GRAY) ? 4 : 6;
	int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
	int i;

	topleft_blk = v->block[v->topleft_blk_idx];
	top_blk = v->block[v->top_blk_idx];
	left_blk = v->block[v->left_blk_idx];
	cur_blk = v->block[v->cur_blk_idx];

	/* Within a MB, the horizontal overlap always runs before the vertical.
	 * To accomplish that, we run the H on the left and internal vertical
	 * borders of the currently decoded MB. Then, we wait for the next overlap
	 * iteration to do H overlap on the right edge of this MB, before moving
	 * over and running the V overlap on the top and internal horizontal
	 * borders. Therefore, the H overlap trails by one MB col and the
	 * V overlap trails by one MB row. This is reflected in the time at which
	 * we run the put_pixels loop, i.e. delayed by one row and one column. */
	for (i = 0; i < block_count; i++) {
		if (s->mb_x == 0 && (i & 5) != 1)
			continue;

		if (v->pq >= 9 || (v->profile == PROFILE_ADVANCED &&
						   (v->condover == CONDOVER_ALL ||
							(v->over_flags_plane[mb_pos] && ((i & 5) == 1 || v->over_flags_plane[mb_pos - 1])))))
			vc1_h_overlap_filter(v, s->mb_x ? left_blk : cur_blk, cur_blk,
								 v->fcm == ILACE_FRAME && s->mb_x && v->fieldtx_plane[mb_pos - 1],
								 v->fcm == ILACE_FRAME && v->fieldtx_plane[mb_pos], i);
	}

	if (v->fcm != ILACE_FRAME)
		for (i = 0; i < block_count; i++) {
			if (s->first_slice_line && !(i & 2))
				continue;

			if (s->mb_x &&
				(v->pq >= 9 ||
				 (v->profile == PROFILE_ADVANCED &&
				  (v->condover == CONDOVER_ALL ||
				   (v->over_flags_plane[mb_pos - 1] && ((i & 2) || v->over_flags_plane[mb_pos - 1 - s->mb_stride]))))))
				vc1_v_overlap_filter(v, s->first_slice_line ? left_blk : topleft_blk, left_blk, i);
			if (s->mb_x == s->mb_width - 1 &&
				(v->pq >= 9 ||
				 (v->profile == PROFILE_ADVANCED &&
				  (v->condover == CONDOVER_ALL ||
				   (v->over_flags_plane[mb_pos] && ((i & 2) || v->over_flags_plane[mb_pos - s->mb_stride]))))))
				vc1_v_overlap_filter(v, s->first_slice_line ? cur_blk : top_blk, cur_blk, i);
		}
}

void ff_vc1_p_overlap_filter(VC1Context *v) {
	PictureState *s = &v->s;
	int16(*topleft_blk)[64], (*top_blk)[64], (*left_blk)[64], (*cur_blk)[64];
	int block_count = CONFIG_GRAY && (s->avctx->flags & AV_CODEC_FLAG_GRAY) ? 4 : 6;
	int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
	int i;

	topleft_blk = v->block[v->topleft_blk_idx];
	top_blk = v->block[v->top_blk_idx];
	left_blk = v->block[v->left_blk_idx];
	cur_blk = v->block[v->cur_blk_idx];

	for (i = 0; i < block_count; i++) {
		if (s->mb_x == 0 && (i & 5) != 1)
			continue;

		if (v->mb_type[s->block_index[i]] && v->mb_type[s->block_index[i] - 1])
			vc1_h_overlap_filter(v, s->mb_x ? left_blk : cur_blk, cur_blk,
								 v->fcm == ILACE_FRAME && s->mb_x && v->fieldtx_plane[mb_pos - 1],
								 v->fcm == ILACE_FRAME && v->fieldtx_plane[mb_pos], i);
	}

	if (v->fcm != ILACE_FRAME)
		for (i = 0; i < block_count; i++) {
			if (s->first_slice_line && !(i & 2))
				continue;

			if (s->mb_x && v->mb_type[s->block_index[i] - 2 + (i > 3)] &&
				v->mb_type[s->block_index[i] - s->block_wrap[i] - 2 + (i > 3)])
				vc1_v_overlap_filter(v, s->first_slice_line ? left_blk : topleft_blk, left_blk, i);
			if (s->mb_x == s->mb_width - 1)
				if (v->mb_type[s->block_index[i]] && v->mb_type[s->block_index[i] - s->block_wrap[i]])
					vc1_v_overlap_filter(v, s->first_slice_line ? cur_blk : top_blk, cur_blk, i);
		}
}

#define LEFT_EDGE (1 << 0)
#define RIGHT_EDGE (1 << 1)
#define TOP_EDGE (1 << 2)
#define BOTTOM_EDGE (1 << 3)

static inline void vc1_i_h_loop_filter(VC1Context *v, uint8 *dest, uint32 flags, int block_num) {
	PictureState *s = &v->s;
	int pq = v->pq;
	uint8 *dst;

	if (block_num & 2)
		return;

	if (!(flags & LEFT_EDGE) || (block_num & 5) == 1) {
		if (block_num > 3)
			dst = dest;
		else
			dst = dest + (block_num & 2) * 4 * s->linesize + (block_num & 1) * 8;

		if (v->fcm == ILACE_FRAME)
			if (block_num > 3) {
				v->vc1dsp.vc1_h_loop_filter4(dst, 2 * s->uvlinesize, pq);
				v->vc1dsp.vc1_h_loop_filter4(dst + s->uvlinesize, 2 * s->uvlinesize, pq);
			} else {
				v->vc1dsp.vc1_h_loop_filter8(dst, 2 * s->linesize, pq);
				v->vc1dsp.vc1_h_loop_filter8(dst + s->linesize, 2 * s->linesize, pq);
			}
		else if (block_num > 3)
			v->vc1dsp.vc1_h_loop_filter8(dst, s->uvlinesize, pq);
		else
			v->vc1dsp.vc1_h_loop_filter16(dst, s->linesize, pq);
	}
}

static inline void vc1_i_v_loop_filter(VC1Context *v, uint8 *dest, uint32 flags, uint8 fieldtx, int block_num) {
	PictureState *s = &v->s;
	int pq = v->pq;
	uint8 *dst;

	if ((block_num & 5) == 1)
		return;

	if (!(flags & TOP_EDGE) || block_num & 2) {
		if (block_num > 3)
			dst = dest;
		else
			dst = dest + (block_num & 2) * 4 * s->linesize + (block_num & 1) * 8;

		if (v->fcm == ILACE_FRAME) {
			if (block_num > 3) {
				v->vc1dsp.vc1_v_loop_filter8(dst, 2 * s->uvlinesize, pq);
				v->vc1dsp.vc1_v_loop_filter8(dst + s->uvlinesize, 2 * s->uvlinesize, pq);
			} else if (block_num < 2 || !fieldtx) {
				v->vc1dsp.vc1_v_loop_filter16(dst, 2 * s->linesize, pq);
				v->vc1dsp.vc1_v_loop_filter16(dst + s->linesize, 2 * s->linesize, pq);
			}
		} else if (block_num > 3)
			v->vc1dsp.vc1_v_loop_filter8(dst, s->uvlinesize, pq);
		else
			v->vc1dsp.vc1_v_loop_filter16(dst, s->linesize, pq);
	}
}

void ff_vc1_i_loop_filter(VC1Context *v) {
	PictureState *s = &v->s;
	int block_count = CONFIG_GRAY && (s->avctx->flags & AV_CODEC_FLAG_GRAY) ? 4 : 6;
	int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
	uint8 *dest, fieldtx;
	uint32 flags = 0;
	int i;

	/* Within a MB, the vertical loop filter always runs before the horizontal.
	 * To accomplish that, we run the V loop filter on top and internal
	 * horizontal borders of the last overlap filtered MB. Then, we wait for
	 * the loop filter iteration on the next row to do V loop filter on the
	 * bottom edge of this MB, before moving over and running the H loop
	 * filter on the left and internal vertical borders. Therefore, the loop
	 * filter trails by one row and one column relative to the overlap filter
	 * and two rows and two columns relative to the decoding loop. */
	if (!s->first_slice_line) {
		dest = s->dest[0] - 16 * s->linesize - 16;
		flags = s->mb_y == s->start_mb_y + 1 ? TOP_EDGE : 0;
		if (s->mb_x) {
			fieldtx = v->fieldtx_plane[mb_pos - s->mb_stride - 1];
			for (i = 0; i < block_count; i++)
				vc1_i_v_loop_filter(v, i > 3 ? s->dest[i - 3] - 8 * s->uvlinesize - 8 : dest, flags, fieldtx, i);
		}
		if (s->mb_x == v->end_mb_x - 1) {
			dest += 16;
			fieldtx = v->fieldtx_plane[mb_pos - s->mb_stride];
			for (i = 0; i < block_count; i++)
				vc1_i_v_loop_filter(v, i > 3 ? s->dest[i - 3] - 8 * s->uvlinesize : dest, flags, fieldtx, i);
		}
	}
	if (s->mb_y == s->end_mb_y - 1) {
		dest = s->dest[0] - 16;
		flags = s->first_slice_line ? TOP_EDGE | BOTTOM_EDGE : BOTTOM_EDGE;
		if (s->mb_x) {
			fieldtx = v->fieldtx_plane[mb_pos - 1];
			for (i = 0; i < block_count; i++)
				vc1_i_v_loop_filter(v, i > 3 ? s->dest[i - 3] - 8 : dest, flags, fieldtx, i);
		}
		if (s->mb_x == v->end_mb_x - 1) {
			dest += 16;
			fieldtx = v->fieldtx_plane[mb_pos];
			for (i = 0; i < block_count; i++)
				vc1_i_v_loop_filter(v, i > 3 ? s->dest[i - 3] : dest, flags, fieldtx, i);
		}
	}

	if (s->mb_y >= s->start_mb_y + 2) {
		dest = s->dest[0] - 32 * s->linesize - 16;
		if (s->mb_x) {
			flags = s->mb_x == 1 ? LEFT_EDGE : 0;
			for (i = 0; i < block_count; i++)
				vc1_i_h_loop_filter(v, i > 3 ? s->dest[i - 3] - 16 * s->uvlinesize - 8 : dest, flags, i);
		}
		if (s->mb_x == v->end_mb_x - 1) {
			dest += 16;
			flags = s->mb_x == 0 ? LEFT_EDGE | RIGHT_EDGE : RIGHT_EDGE;
			for (i = 0; i < block_count; i++)
				vc1_i_h_loop_filter(v, i > 3 ? s->dest[i - 3] - 16 * s->uvlinesize : dest, flags, i);
		}
	}
	if (s->mb_y == s->end_mb_y - 1) {
		if (s->mb_y >= s->start_mb_y + 1) {
			dest = s->dest[0] - 16 * s->linesize - 16;
			if (s->mb_x) {
				flags = s->mb_x == 1 ? LEFT_EDGE : 0;
				for (i = 0; i < block_count; i++)
					vc1_i_h_loop_filter(v, i > 3 ? s->dest[i - 3] - 8 * s->uvlinesize - 8 : dest, flags, i);
			}
			if (s->mb_x == v->end_mb_x - 1) {
				flags = s->mb_x == 0 ? LEFT_EDGE | RIGHT_EDGE : RIGHT_EDGE;
				dest += 16;
				for (i = 0; i < block_count; i++)
					vc1_i_h_loop_filter(v, i > 3 ? s->dest[i - 3] - 8 * s->uvlinesize : dest, flags, i);
			}
		}
		dest = s->dest[0] - 16;
		if (s->mb_x) {
			flags = s->mb_x == 1 ? LEFT_EDGE : 0;
			for (i = 0; i < block_count; i++)
				vc1_i_h_loop_filter(v, i > 3 ? s->dest[i - 3] - 8 : dest, flags, i);
		}
		if (s->mb_x == v->end_mb_x - 1) {
			dest += 16;
			flags = s->mb_x == 0 ? LEFT_EDGE | RIGHT_EDGE : RIGHT_EDGE;
			for (i = 0; i < block_count; i++)
				vc1_i_h_loop_filter(v, i > 3 ? s->dest[i - 3] : dest, flags, i);
		}
	}
}

static inline void vc1_p_h_loop_filter(VC1Context *v, uint8 *dest, uint32 *cbp, uint8 *is_intra, int16 (*mv)[2],
									   uint8 *mv_f, int *ttblk, uint32 flags, int block_num) {
	PictureState *s = &v->s;
	int pq = v->pq;
	uint32 left_cbp = cbp[0] >> (block_num * 4), right_cbp;
	uint8 left_is_intra, right_is_intra;
	int tt;
	int idx, linesize = block_num > 3 ? s->uvlinesize : s->linesize;
	uint8 *dst;

	if (block_num > 3)
		dst = dest;
	else
		dst = dest + (block_num & 2) * 4 * s->linesize + (block_num & 1) * 8;

	if (!(flags & RIGHT_EDGE) || !(block_num & 5)) {
		left_is_intra = is_intra[0] & (1 << block_num);

		if (block_num > 3) {
			right_is_intra = is_intra[1] & (1 << block_num);
			right_cbp = cbp[1] >> (block_num * 4);
		} else if (block_num & 1) {
			right_is_intra = is_intra[1] & (1 << (block_num - 1));
			right_cbp = cbp[1] >> ((block_num - 1) * 4);
		} else {
			right_is_intra = is_intra[0] & (1 << (block_num + 1));
			right_cbp = cbp[0] >> ((block_num + 1) * 4);
		}

		if (left_is_intra || right_is_intra || mv[0][0] != mv[1][0] || mv[0][1] != mv[1][1] ||
			(v->fcm == ILACE_FIELD && mv_f[0] != mv_f[1]))
			v->vc1dsp.vc1_h_loop_filter8(dst + 8, linesize, pq);
		else {
			idx = (left_cbp | (right_cbp >> 1)) & 5;
			if (idx & 1)
				v->vc1dsp.vc1_h_loop_filter4(dst + 4 * linesize + 8, linesize, pq);
			if (idx & 4)
				v->vc1dsp.vc1_h_loop_filter4(dst + 8, linesize, pq);
		}
	}

	tt = ttblk[0] >> (block_num * 4) & 0xf;
	if (tt == TT_4X4 || tt == TT_4X8) {
		if (left_cbp & 3)
			v->vc1dsp.vc1_h_loop_filter4(dst + 4 * linesize + 4, linesize, pq);
		if (left_cbp & 12)
			v->vc1dsp.vc1_h_loop_filter4(dst + 4, linesize, pq);
	}
}

static inline void vc1_p_v_loop_filter(VC1Context *v, uint8 *dest, const uint32 *cbp, const uint8 *is_intra,
									   int16 (*mv)[2], const uint8 *mv_f, const int *ttblk, uint32 flags,
									   int block_num) {
	PictureState *s = &v->s;
	int pq = v->pq;
	uint32 top_cbp = cbp[0] >> (block_num * 4), bottom_cbp;
	uint8 top_is_intra, bottom_is_intra;
	int tt;
	int idx, linesize = block_num > 3 ? s->uvlinesize : s->linesize;
	uint8 *dst;

	if (block_num > 3)
		dst = dest;
	else
		dst = dest + (block_num & 2) * 4 * s->linesize + (block_num & 1) * 8;

	if (!(flags & BOTTOM_EDGE) || block_num < 2) {
		top_is_intra = is_intra[0] & (1 << block_num);

		if (block_num > 3) {
			bottom_is_intra = is_intra[s->mb_stride] & (1 << block_num);
			bottom_cbp = cbp[s->mb_stride] >> (block_num * 4);
		} else if (block_num < 2) {
			bottom_is_intra = is_intra[0] & (1 << (block_num + 2));
			bottom_cbp = cbp[0] >> ((block_num + 2) * 4);
		} else {
			bottom_is_intra = is_intra[s->mb_stride] & (1 << (block_num - 2));
			bottom_cbp = cbp[s->mb_stride] >> ((block_num - 2) * 4);
		}

		if (top_is_intra || bottom_is_intra || mv[0][0] != mv[block_num > 3 ? s->mb_stride : s->b8_stride][0] ||
			mv[0][1] != mv[block_num > 3 ? s->mb_stride : s->b8_stride][1] ||
			(v->fcm == ILACE_FIELD && mv_f[0] != mv_f[block_num > 3 ? s->mb_stride : s->b8_stride]))
			v->vc1dsp.vc1_v_loop_filter8(dst + 8 * linesize, linesize, pq);
		else {
			idx = (top_cbp | (bottom_cbp >> 2)) & 3;
			if (idx & 1)
				v->vc1dsp.vc1_v_loop_filter4(dst + 8 * linesize + 4, linesize, pq);
			if (idx & 2)
				v->vc1dsp.vc1_v_loop_filter4(dst + 8 * linesize, linesize, pq);
		}
	}

	tt = ttblk[0] >> (block_num * 4) & 0xf;
	if (tt == TT_4X4 || tt == TT_8X4) {
		if (top_cbp & 5)
			v->vc1dsp.vc1_v_loop_filter4(dst + 4 * linesize + 4, linesize, pq);
		if (top_cbp & 10)
			v->vc1dsp.vc1_v_loop_filter4(dst + 4 * linesize, linesize, pq);
	}
}

void ff_vc1_p_loop_filter(VC1Context *v) {
	PictureState *s = &v->s;
	int block_count = CONFIG_GRAY && (s->avctx->flags & AV_CODEC_FLAG_GRAY) ? 4 : 6;
	uint8 *dest;
	uint32 *cbp;
	uint8 *is_intra;
	int16(*uvmv)[2];
	int *ttblk;
	uint32 flags;
	int i;

	/* Within a MB, the vertical loop filter always runs before the horizontal.
	 * To accomplish that, we run the V loop filter on all applicable
	 * horizontal borders of the MB above the last overlap filtered MB. Then,
	 * we wait for the next loop filter iteration to do H loop filter on all
	 * applicable vertical borders of this MB. Therefore, the loop filter
	 * trails by one row and one column relative to the overlap filter and two
	 * rows and two columns relative to the decoding loop. */
	if (s->mb_y >= s->start_mb_y + 2) {
		if (s->mb_x) {
			dest = s->dest[0] - 32 * s->linesize - 16;
			cbp = &v->cbp[s->mb_x - 2 * s->mb_stride - 1];
			is_intra = &v->is_intra[s->mb_x - 2 * s->mb_stride - 1];
			uvmv = &v->luma_mv[s->mb_x - 2 * s->mb_stride - 1];
			ttblk = &v->ttblk[s->mb_x - 2 * s->mb_stride - 1];
			flags = s->mb_y == s->start_mb_y + 2 ? TOP_EDGE : 0;
			for (i = 0; i < block_count; i++)
				vc1_p_v_loop_filter(
					v, i > 3 ? s->dest[i - 3] - 16 * s->uvlinesize - 8 : dest, cbp, is_intra,
					i > 3 ? uvmv : &s->cur_pic.motion_val[0][s->block_index[i] - 4 * s->b8_stride - 2 + v->blocks_off],
					i > 3 ? &v->mv_f[0][s->block_index[i] - 2 * s->mb_stride - 1 + v->mb_off]
						  : &v->mv_f[0][s->block_index[i] - 4 * s->b8_stride - 2 + v->blocks_off],
					ttblk, flags, i);
		}
		if (s->mb_x == s->mb_width - 1) {
			dest = s->dest[0] - 32 * s->linesize;
			cbp = &v->cbp[s->mb_x - 2 * s->mb_stride];
			is_intra = &v->is_intra[s->mb_x - 2 * s->mb_stride];
			uvmv = &v->luma_mv[s->mb_x - 2 * s->mb_stride];
			ttblk = &v->ttblk[s->mb_x - 2 * s->mb_stride];
			flags = s->mb_y == s->start_mb_y + 2 ? TOP_EDGE : 0;
			for (i = 0; i < block_count; i++)
				vc1_p_v_loop_filter(
					v, i > 3 ? s->dest[i - 3] - 16 * s->uvlinesize : dest, cbp, is_intra,
					i > 3 ? uvmv : &s->cur_pic.motion_val[0][s->block_index[i] - 4 * s->b8_stride + v->blocks_off],
					i > 3 ? &v->mv_f[0][s->block_index[i] - 2 * s->mb_stride + v->mb_off]
						  : &v->mv_f[0][s->block_index[i] - 4 * s->b8_stride + v->blocks_off],
					ttblk, flags, i);
		}
	}
	if (s->mb_y == s->end_mb_y - 1) {
		if (s->mb_x) {
			if (s->mb_y >= s->start_mb_y + 1) {
				dest = s->dest[0] - 16 * s->linesize - 16;
				cbp = &v->cbp[s->mb_x - s->mb_stride - 1];
				is_intra = &v->is_intra[s->mb_x - s->mb_stride - 1];
				uvmv = &v->luma_mv[s->mb_x - s->mb_stride - 1];
				ttblk = &v->ttblk[s->mb_x - s->mb_stride - 1];
				flags = s->mb_y == s->start_mb_y + 1 ? TOP_EDGE : 0;
				for (i = 0; i < block_count; i++)
					vc1_p_v_loop_filter(
						v, i > 3 ? s->dest[i - 3] - 8 * s->uvlinesize - 8 : dest, cbp, is_intra,
						i > 3 ? uvmv
							  : &s->cur_pic.motion_val[0][s->block_index[i] - 2 * s->b8_stride - 2 + v->blocks_off],
						i > 3 ? &v->mv_f[0][s->block_index[i] - s->mb_stride - 1 + v->mb_off]
							  : &v->mv_f[0][s->block_index[i] - 2 * s->b8_stride - 2 + v->blocks_off],
						ttblk, flags, i);
			}
			dest = s->dest[0] - 16;
			cbp = &v->cbp[s->mb_x - 1];
			is_intra = &v->is_intra[s->mb_x - 1];
			uvmv = &v->luma_mv[s->mb_x - 1];
			ttblk = &v->ttblk[s->mb_x - 1];
			flags = s->mb_y == s->start_mb_y ? TOP_EDGE | BOTTOM_EDGE : BOTTOM_EDGE;
			for (i = 0; i < block_count; i++)
				vc1_p_v_loop_filter(v, i > 3 ? s->dest[i - 3] - 8 : dest, cbp, is_intra,
									i > 3 ? uvmv : &s->cur_pic.motion_val[0][s->block_index[i] - 2 + v->blocks_off],
									i > 3 ? &v->mv_f[0][s->block_index[i] - 1 + v->mb_off]
										  : &v->mv_f[0][s->block_index[i] - 2 + v->blocks_off],
									ttblk, flags, i);
		}
		if (s->mb_x == s->mb_width - 1) {
			if (s->mb_y >= s->start_mb_y + 1) {
				dest = s->dest[0] - 16 * s->linesize;
				cbp = &v->cbp[s->mb_x - s->mb_stride];
				is_intra = &v->is_intra[s->mb_x - s->mb_stride];
				uvmv = &v->luma_mv[s->mb_x - s->mb_stride];
				ttblk = &v->ttblk[s->mb_x - s->mb_stride];
				flags = s->mb_y == s->start_mb_y + 1 ? TOP_EDGE : 0;
				for (i = 0; i < block_count; i++)
					vc1_p_v_loop_filter(
						v, i > 3 ? s->dest[i - 3] - 8 * s->uvlinesize : dest, cbp, is_intra,
						i > 3 ? uvmv : &s->cur_pic.motion_val[0][s->block_index[i] - 2 * s->b8_stride + v->blocks_off],
						i > 3 ? &v->mv_f[0][s->block_index[i] - s->mb_stride + v->mb_off]
							  : &v->mv_f[0][s->block_index[i] - 2 * s->b8_stride + v->blocks_off],
						ttblk, flags, i);
			}
			dest = s->dest[0];
			cbp = &v->cbp[s->mb_x];
			is_intra = &v->is_intra[s->mb_x];
			uvmv = &v->luma_mv[s->mb_x];
			ttblk = &v->ttblk[s->mb_x];
			flags = s->mb_y == s->start_mb_y ? TOP_EDGE | BOTTOM_EDGE : BOTTOM_EDGE;
			for (i = 0; i < block_count; i++)
				vc1_p_v_loop_filter(v, i > 3 ? s->dest[i - 3] : dest, cbp, is_intra,
									i > 3 ? uvmv : &s->cur_pic.motion_val[0][s->block_index[i] + v->blocks_off],
									i > 3 ? &v->mv_f[0][s->block_index[i] + v->mb_off]
										  : &v->mv_f[0][s->block_index[i] + v->blocks_off],
									ttblk, flags, i);
		}
	}

	if (s->mb_y >= s->start_mb_y + 2) {
		if (s->mb_x >= 2) {
			dest = s->dest[0] - 32 * s->linesize - 32;
			cbp = &v->cbp[s->mb_x - 2 * s->mb_stride - 2];
			is_intra = &v->is_intra[s->mb_x - 2 * s->mb_stride - 2];
			uvmv = &v->luma_mv[s->mb_x - 2 * s->mb_stride - 2];
			ttblk = &v->ttblk[s->mb_x - 2 * s->mb_stride - 2];
			flags = s->mb_x == 2 ? LEFT_EDGE : 0;
			for (i = 0; i < block_count; i++)
				vc1_p_h_loop_filter(
					v, i > 3 ? s->dest[i - 3] - 16 * s->uvlinesize - 16 : dest, cbp, is_intra,
					i > 3 ? uvmv : &s->cur_pic.motion_val[0][s->block_index[i] - 4 * s->b8_stride - 4 + v->blocks_off],
					i > 3 ? &v->mv_f[0][s->block_index[i] - 2 * s->mb_stride - 2 + v->mb_off]
						  : &v->mv_f[0][s->block_index[i] - 4 * s->b8_stride - 4 + v->blocks_off],
					ttblk, flags, i);
		}
		if (s->mb_x == s->mb_width - 1) {
			if (s->mb_x >= 1) {
				dest = s->dest[0] - 32 * s->linesize - 16;
				cbp = &v->cbp[s->mb_x - 2 * s->mb_stride - 1];
				is_intra = &v->is_intra[s->mb_x - 2 * s->mb_stride - 1];
				uvmv = &v->luma_mv[s->mb_x - 2 * s->mb_stride - 1];
				ttblk = &v->ttblk[s->mb_x - 2 * s->mb_stride - 1];
				flags = s->mb_x == 1 ? LEFT_EDGE : 0;
				for (i = 0; i < block_count; i++)
					vc1_p_h_loop_filter(
						v, i > 3 ? s->dest[i - 3] - 16 * s->uvlinesize - 8 : dest, cbp, is_intra,
						i > 3 ? uvmv
							  : &s->cur_pic.motion_val[0][s->block_index[i] - 4 * s->b8_stride - 2 + v->blocks_off],
						i > 3 ? &v->mv_f[0][s->block_index[i] - 2 * s->mb_stride - 1 + v->mb_off]
							  : &v->mv_f[0][s->block_index[i] - 4 * s->b8_stride - 2 + v->blocks_off],
						ttblk, flags, i);
			}
			dest = s->dest[0] - 32 * s->linesize;
			cbp = &v->cbp[s->mb_x - 2 * s->mb_stride];
			is_intra = &v->is_intra[s->mb_x - 2 * s->mb_stride];
			uvmv = &v->luma_mv[s->mb_x - 2 * s->mb_stride];
			ttblk = &v->ttblk[s->mb_x - 2 * s->mb_stride];
			flags = s->mb_x ? RIGHT_EDGE : LEFT_EDGE | RIGHT_EDGE;
			for (i = 0; i < block_count; i++)
				vc1_p_h_loop_filter(
					v, i > 3 ? s->dest[i - 3] - 16 * s->uvlinesize : dest, cbp, is_intra,
					i > 3 ? uvmv : &s->cur_pic.motion_val[0][s->block_index[i] - 4 * s->b8_stride + v->blocks_off],
					i > 3 ? &v->mv_f[0][s->block_index[i] - 2 * s->mb_stride + v->mb_off]
						  : &v->mv_f[0][s->block_index[i] - 4 * s->b8_stride + v->blocks_off],
					ttblk, flags, i);
		}
	}
	if (s->mb_y == s->end_mb_y - 1) {
		if (s->mb_y >= s->start_mb_y + 1) {
			if (s->mb_x >= 2) {
				dest = s->dest[0] - 16 * s->linesize - 32;
				cbp = &v->cbp[s->mb_x - s->mb_stride - 2];
				is_intra = &v->is_intra[s->mb_x - s->mb_stride - 2];
				uvmv = &v->luma_mv[s->mb_x - s->mb_stride - 2];
				ttblk = &v->ttblk[s->mb_x - s->mb_stride - 2];
				flags = s->mb_x == 2 ? LEFT_EDGE : 0;
				for (i = 0; i < block_count; i++)
					vc1_p_h_loop_filter(
						v, i > 3 ? s->dest[i - 3] - 8 * s->uvlinesize - 16 : dest, cbp, is_intra,
						i > 3 ? uvmv
							  : &s->cur_pic.motion_val[0][s->block_index[i] - 2 * s->b8_stride - 4 + v->blocks_off],
						i > 3 ? &v->mv_f[0][s->block_index[i] - s->mb_stride - 2 + v->mb_off]
							  : &v->mv_f[0][s->block_index[i] - 2 * s->b8_stride - 4 + v->blocks_off],
						ttblk, flags, i);
			}
			if (s->mb_x == s->mb_width - 1) {
				if (s->mb_x >= 1) {
					dest = s->dest[0] - 16 * s->linesize - 16;
					cbp = &v->cbp[s->mb_x - s->mb_stride - 1];
					is_intra = &v->is_intra[s->mb_x - s->mb_stride - 1];
					uvmv = &v->luma_mv[s->mb_x - s->mb_stride - 1];
					ttblk = &v->ttblk[s->mb_x - s->mb_stride - 1];
					flags = s->mb_x == 1 ? LEFT_EDGE : 0;
					for (i = 0; i < block_count; i++)
						vc1_p_h_loop_filter(
							v, i > 3 ? s->dest[i - 3] - 8 * s->uvlinesize - 8 : dest, cbp, is_intra,
							i > 3 ? uvmv
								  : &s->cur_pic.motion_val[0][s->block_index[i] - 2 * s->b8_stride - 2 + v->blocks_off],
							i > 3 ? &v->mv_f[0][s->block_index[i] - s->mb_stride - 1 + v->mb_off]
								  : &v->mv_f[0][s->block_index[i] - 2 * s->b8_stride - 2 + v->blocks_off],
							ttblk, flags, i);
				}
				dest = s->dest[0] - 16 * s->linesize;
				cbp = &v->cbp[s->mb_x - s->mb_stride];
				is_intra = &v->is_intra[s->mb_x - s->mb_stride];
				uvmv = &v->luma_mv[s->mb_x - s->mb_stride];
				ttblk = &v->ttblk[s->mb_x - s->mb_stride];
				flags = s->mb_x ? RIGHT_EDGE : LEFT_EDGE | RIGHT_EDGE;
				for (i = 0; i < block_count; i++)
					vc1_p_h_loop_filter(
						v, i > 3 ? s->dest[i - 3] - 8 * s->uvlinesize : dest, cbp, is_intra,
						i > 3 ? uvmv : &s->cur_pic.motion_val[0][s->block_index[i] - 2 * s->b8_stride + v->blocks_off],
						i > 3 ? &v->mv_f[0][s->block_index[i] - s->mb_stride + v->mb_off]
							  : &v->mv_f[0][s->block_index[i] - 2 * s->b8_stride + v->blocks_off],
						ttblk, flags, i);
			}
		}
		if (s->mb_x >= 2) {
			dest = s->dest[0] - 32;
			cbp = &v->cbp[s->mb_x - 2];
			is_intra = &v->is_intra[s->mb_x - 2];
			uvmv = &v->luma_mv[s->mb_x - 2];
			ttblk = &v->ttblk[s->mb_x - 2];
			flags = s->mb_x == 2 ? LEFT_EDGE : 0;
			for (i = 0; i < block_count; i++)
				vc1_p_h_loop_filter(v, i > 3 ? s->dest[i - 3] - 16 : dest, cbp, is_intra,
									i > 3 ? uvmv : &s->cur_pic.motion_val[0][s->block_index[i] - 4 + v->blocks_off],
									i > 3 ? &v->mv_f[0][s->block_index[i] - 2 + v->mb_off]
										  : &v->mv_f[0][s->block_index[i] - 4 + v->blocks_off],
									ttblk, flags, i);
		}
		if (s->mb_x == s->mb_width - 1) {
			if (s->mb_x >= 1) {
				dest = s->dest[0] - 16;
				cbp = &v->cbp[s->mb_x - 1];
				is_intra = &v->is_intra[s->mb_x - 1];
				uvmv = &v->luma_mv[s->mb_x - 1];
				ttblk = &v->ttblk[s->mb_x - 1];
				flags = s->mb_x == 1 ? LEFT_EDGE : 0;
				for (i = 0; i < block_count; i++)
					vc1_p_h_loop_filter(v, i > 3 ? s->dest[i - 3] - 8 : dest, cbp, is_intra,
										i > 3 ? uvmv : &s->cur_pic.motion_val[0][s->block_index[i] - 2 + v->blocks_off],
										i > 3 ? &v->mv_f[0][s->block_index[i] - 1 + v->mb_off]
											  : &v->mv_f[0][s->block_index[i] - 2 + v->blocks_off],
										ttblk, flags, i);
			}
			dest = s->dest[0];
			cbp = &v->cbp[s->mb_x];
			is_intra = &v->is_intra[s->mb_x];
			uvmv = &v->luma_mv[s->mb_x];
			ttblk = &v->ttblk[s->mb_x];
			flags = s->mb_x ? RIGHT_EDGE : LEFT_EDGE | RIGHT_EDGE;
			for (i = 0; i < block_count; i++)
				vc1_p_h_loop_filter(v, i > 3 ? s->dest[i - 3] : dest, cbp, is_intra,
									i > 3 ? uvmv : &s->cur_pic.motion_val[0][s->block_index[i] + v->blocks_off],
									i > 3 ? &v->mv_f[0][s->block_index[i] + v->mb_off]
										  : &v->mv_f[0][s->block_index[i] + v->blocks_off],
									ttblk, flags, i);
		}
	}
}

static inline void vc1_p_h_intfr_loop_filter(VC1Context *v, uint8 *dest, const int *ttblk, uint32 flags, uint8 fieldtx,
											 int block_num) {
	PictureState *s = &v->s;
	int pq = v->pq;
	int tt;
	int linesize = block_num > 3 ? s->uvlinesize : s->linesize;
	uint8 *dst;

	if (block_num > 3)
		dst = dest;
	else
		dst = dest + (block_num & 2) * 4 * s->linesize + (block_num & 1) * 8;

	tt = ttblk[0] >> (block_num * 4) & 0xf;
	if (block_num < 4) {
		if (fieldtx) {
			if (block_num < 2) {
				if (tt == TT_4X4 || tt == TT_4X8)
					v->vc1dsp.vc1_h_loop_filter8(dst + 4, 2 * linesize, pq);
				if (!(flags & RIGHT_EDGE) || block_num == 0)
					v->vc1dsp.vc1_h_loop_filter8(dst + 8, 2 * linesize, pq);
			} else {
				if (tt == TT_4X4 || tt == TT_4X8)
					v->vc1dsp.vc1_h_loop_filter8(dst - 7 * linesize + 4, 2 * linesize, pq);
				if (!(flags & RIGHT_EDGE) || block_num == 2)
					v->vc1dsp.vc1_h_loop_filter8(dst - 7 * linesize + 8, 2 * linesize, pq);
			}
		} else {
			if (tt == TT_4X4 || tt == TT_4X8) {
				v->vc1dsp.vc1_h_loop_filter4(dst + 4, 2 * linesize, pq);
				v->vc1dsp.vc1_h_loop_filter4(dst + linesize + 4, 2 * linesize, pq);
			}
			if (!(flags & RIGHT_EDGE) || !(block_num & 5)) {
				v->vc1dsp.vc1_h_loop_filter4(dst + 8, 2 * linesize, pq);
				v->vc1dsp.vc1_h_loop_filter4(dst + linesize + 8, 2 * linesize, pq);
			}
		}
	} else {
		if (tt == TT_4X4 || tt == TT_4X8) {
			v->vc1dsp.vc1_h_loop_filter4(dst + 4, 2 * linesize, pq);
			v->vc1dsp.vc1_h_loop_filter4(dst + linesize + 4, 2 * linesize, pq);
		}
		if (!(flags & RIGHT_EDGE)) {
			v->vc1dsp.vc1_h_loop_filter4(dst + 8, 2 * linesize, pq);
			v->vc1dsp.vc1_h_loop_filter4(dst + linesize + 8, 2 * linesize, pq);
		}
	}
}

static inline void vc1_p_v_intfr_loop_filter(VC1Context *v, uint8 *dest, const int *ttblk, uint32 flags, uint8 fieldtx,
											 int block_num) {
	PictureState *s = &v->s;
	int pq = v->pq;
	int tt;
	int linesize = block_num > 3 ? s->uvlinesize : s->linesize;
	uint8 *dst;

	if (block_num > 3)
		dst = dest;
	else
		dst = dest + (block_num & 2) * 4 * s->linesize + (block_num & 1) * 8;

	tt = ttblk[0] >> (block_num * 4) & 0xf;
	if (block_num < 4) {
		if (fieldtx) {
			if (block_num < 2) {
				if (tt == TT_4X4 || tt == TT_8X4)
					v->vc1dsp.vc1_v_loop_filter8(dst + 8 * linesize, 2 * linesize, pq);
				if (!(flags & BOTTOM_EDGE))
					v->vc1dsp.vc1_v_loop_filter8(dst + 16 * linesize, 2 * linesize, pq);
			} else {
				if (tt == TT_4X4 || tt == TT_8X4)
					v->vc1dsp.vc1_v_loop_filter8(dst + linesize, 2 * linesize, pq);
				if (!(flags & BOTTOM_EDGE))
					v->vc1dsp.vc1_v_loop_filter8(dst + 9 * linesize, 2 * linesize, pq);
			}
		} else {
			if (block_num < 2) {
				if (!(flags & TOP_EDGE) && (tt == TT_4X4 || tt == TT_8X4)) {
					v->vc1dsp.vc1_v_loop_filter8(dst + 4 * linesize, 2 * linesize, pq);
					v->vc1dsp.vc1_v_loop_filter8(dst + 5 * linesize, 2 * linesize, pq);
				}
				v->vc1dsp.vc1_v_loop_filter8(dst + 8 * linesize, 2 * linesize, pq);
				v->vc1dsp.vc1_v_loop_filter8(dst + 9 * linesize, 2 * linesize, pq);
			} else if (!(flags & BOTTOM_EDGE)) {
				if (tt == TT_4X4 || tt == TT_8X4) {
					v->vc1dsp.vc1_v_loop_filter8(dst + 4 * linesize, 2 * linesize, pq);
					v->vc1dsp.vc1_v_loop_filter8(dst + 5 * linesize, 2 * linesize, pq);
				}
				v->vc1dsp.vc1_v_loop_filter8(dst + 8 * linesize, 2 * linesize, pq);
				v->vc1dsp.vc1_v_loop_filter8(dst + 9 * linesize, 2 * linesize, pq);
			}
		}
	} else {
		if (!(flags & BOTTOM_EDGE)) {
			if (!(flags & TOP_EDGE) && (tt == TT_4X4 || tt == TT_8X4)) {
				v->vc1dsp.vc1_v_loop_filter8(dst + 4 * linesize, 2 * linesize, pq);
				v->vc1dsp.vc1_v_loop_filter8(dst + 5 * linesize, 2 * linesize, pq);
			}
			v->vc1dsp.vc1_v_loop_filter8(dst + 8 * linesize, 2 * linesize, pq);
			v->vc1dsp.vc1_v_loop_filter8(dst + 9 * linesize, 2 * linesize, pq);
		}
	}
}

void ff_vc1_p_intfr_loop_filter(VC1Context *v) {
	PictureState *s = &v->s;
	int block_count = CONFIG_GRAY && (s->avctx->flags & AV_CODEC_FLAG_GRAY) ? 4 : 6;
	int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
	uint8 *dest;
	int *ttblk;
	uint32 flags;
	uint8 fieldtx;
	int i;

	/* Within a MB, the vertical loop filter always runs before the horizontal.
	 * To accomplish that, we run the V loop filter on all applicable
	 * horizontal borders of the MB above the last overlap filtered MB. Then,
	 * we wait for the loop filter iteration on the next row and next column to
	 * do H loop filter on all applicable vertical borders of this MB.
	 * Therefore, the loop filter trails by two rows and one column relative to
	 * the overlap filter and two rows and two columns relative to the decoding
	 * loop. */
	if (s->mb_x) {
		if (s->mb_y >= s->start_mb_y + 1) {
			dest = s->dest[0] - 16 * s->linesize - 16;
			ttblk = &v->ttblk[s->mb_x - s->mb_stride - 1];
			flags = s->mb_y == s->start_mb_y + 1 ? TOP_EDGE : 0;
			fieldtx = v->fieldtx_plane[mb_pos - s->mb_stride - 1];
			for (i = 0; i < block_count; i++)
				vc1_p_v_intfr_loop_filter(v, i > 3 ? s->dest[i - 3] - 8 * s->uvlinesize - 8 : dest, ttblk, flags,
										  fieldtx, i);
		}
	}
	if (s->mb_x == s->mb_width - 1) {
		if (s->mb_y >= s->start_mb_y + 1) {
			dest = s->dest[0] - 16 * s->linesize;
			ttblk = &v->ttblk[s->mb_x - s->mb_stride];
			flags = s->mb_y == s->start_mb_y + 1 ? TOP_EDGE : 0;
			fieldtx = v->fieldtx_plane[mb_pos - s->mb_stride];
			for (i = 0; i < block_count; i++)
				vc1_p_v_intfr_loop_filter(v, i > 3 ? s->dest[i - 3] - 8 * s->uvlinesize : dest, ttblk, flags, fieldtx,
										  i);
		}
	}
	if (s->mb_y == s->end_mb_y - 1) {
		if (s->mb_x) {
			dest = s->dest[0] - 16;
			ttblk = &v->ttblk[s->mb_x - 1];
			flags = s->mb_y == s->start_mb_y ? TOP_EDGE | BOTTOM_EDGE : BOTTOM_EDGE;
			fieldtx = v->fieldtx_plane[mb_pos - 1];
			for (i = 0; i < block_count; i++)
				vc1_p_v_intfr_loop_filter(v, i > 3 ? s->dest[i - 3] - 8 : dest, ttblk, flags, fieldtx, i);
		}
		if (s->mb_x == s->mb_width - 1) {
			dest = s->dest[0];
			ttblk = &v->ttblk[s->mb_x];
			flags = s->mb_y == s->start_mb_y ? TOP_EDGE | BOTTOM_EDGE : BOTTOM_EDGE;
			fieldtx = v->fieldtx_plane[mb_pos];
			for (i = 0; i < block_count; i++)
				vc1_p_v_intfr_loop_filter(v, i > 3 ? s->dest[i - 3] : dest, ttblk, flags, fieldtx, i);
		}
	}

	if (s->mb_y >= s->start_mb_y + 2) {
		if (s->mb_x >= 2) {
			dest = s->dest[0] - 32 * s->linesize - 32;
			ttblk = &v->ttblk[s->mb_x - 2 * s->mb_stride - 2];
			flags = s->mb_x == 2 ? LEFT_EDGE : 0;
			fieldtx = v->fieldtx_plane[mb_pos - 2 * s->mb_stride - 2];
			for (i = 0; i < block_count; i++)
				vc1_p_h_intfr_loop_filter(v, i > 3 ? s->dest[i - 3] - 16 * s->uvlinesize - 16 : dest, ttblk, flags,
										  fieldtx, i);
		}
		if (s->mb_x == s->mb_width - 1) {
			if (s->mb_x >= 1) {
				dest = s->dest[0] - 32 * s->linesize - 16;
				ttblk = &v->ttblk[s->mb_x - 2 * s->mb_stride - 1];
				flags = s->mb_x == 1 ? LEFT_EDGE : 0;
				fieldtx = v->fieldtx_plane[mb_pos - 2 * s->mb_stride - 1];
				for (i = 0; i < block_count; i++)
					vc1_p_h_intfr_loop_filter(v, i > 3 ? s->dest[i - 3] - 16 * s->uvlinesize - 8 : dest, ttblk, flags,
											  fieldtx, i);
			}
			dest = s->dest[0] - 32 * s->linesize;
			ttblk = &v->ttblk[s->mb_x - 2 * s->mb_stride];
			flags = s->mb_x ? RIGHT_EDGE : LEFT_EDGE | RIGHT_EDGE;
			fieldtx = v->fieldtx_plane[mb_pos - 2 * s->mb_stride];
			for (i = 0; i < block_count; i++)
				vc1_p_h_intfr_loop_filter(v, i > 3 ? s->dest[i - 3] - 16 * s->uvlinesize : dest, ttblk, flags, fieldtx,
										  i);
		}
	}
	if (s->mb_y == s->end_mb_y - 1) {
		if (s->mb_y >= s->start_mb_y + 1) {
			if (s->mb_x >= 2) {
				dest = s->dest[0] - 16 * s->linesize - 32;
				ttblk = &v->ttblk[s->mb_x - s->mb_stride - 2];
				flags = s->mb_x == 2 ? LEFT_EDGE : 0;
				fieldtx = v->fieldtx_plane[mb_pos - s->mb_stride - 2];
				for (i = 0; i < block_count; i++)
					vc1_p_h_intfr_loop_filter(v, i > 3 ? s->dest[i - 3] - 8 * s->uvlinesize - 16 : dest, ttblk, flags,
											  fieldtx, i);
			}
			if (s->mb_x == s->mb_width - 1) {
				if (s->mb_x >= 1) {
					dest = s->dest[0] - 16 * s->linesize - 16;
					ttblk = &v->ttblk[s->mb_x - s->mb_stride - 1];
					flags = s->mb_x == 1 ? LEFT_EDGE : 0;
					fieldtx = v->fieldtx_plane[mb_pos - s->mb_stride - 1];
					for (i = 0; i < block_count; i++)
						vc1_p_h_intfr_loop_filter(v, i > 3 ? s->dest[i - 3] - 8 * s->uvlinesize - 8 : dest, ttblk,
												  flags, fieldtx, i);
				}
				dest = s->dest[0] - 16 * s->linesize;
				ttblk = &v->ttblk[s->mb_x - s->mb_stride];
				flags = s->mb_x ? RIGHT_EDGE : LEFT_EDGE | RIGHT_EDGE;
				fieldtx = v->fieldtx_plane[mb_pos - s->mb_stride];
				for (i = 0; i < block_count; i++)
					vc1_p_h_intfr_loop_filter(v, i > 3 ? s->dest[i - 3] - 8 * s->uvlinesize : dest, ttblk, flags,
											  fieldtx, i);
			}
		}
		if (s->mb_x >= 2) {
			dest = s->dest[0] - 32;
			ttblk = &v->ttblk[s->mb_x - 2];
			flags = s->mb_x == 2 ? LEFT_EDGE : 0;
			fieldtx = v->fieldtx_plane[mb_pos - 2];
			for (i = 0; i < block_count; i++)
				vc1_p_h_intfr_loop_filter(v, i > 3 ? s->dest[i - 3] - 16 : dest, ttblk, flags, fieldtx, i);
		}
		if (s->mb_x == s->mb_width - 1) {
			if (s->mb_x >= 1) {
				dest = s->dest[0] - 16;
				ttblk = &v->ttblk[s->mb_x - 1];
				flags = s->mb_x == 1 ? LEFT_EDGE : 0;
				fieldtx = v->fieldtx_plane[mb_pos - 1];
				for (i = 0; i < block_count; i++)
					vc1_p_h_intfr_loop_filter(v, i > 3 ? s->dest[i - 3] - 8 : dest, ttblk, flags, fieldtx, i);
			}
			dest = s->dest[0];
			ttblk = &v->ttblk[s->mb_x];
			flags = s->mb_x ? RIGHT_EDGE : LEFT_EDGE | RIGHT_EDGE;
			fieldtx = v->fieldtx_plane[mb_pos];
			for (i = 0; i < block_count; i++)
				vc1_p_h_intfr_loop_filter(v, i > 3 ? s->dest[i - 3] : dest, ttblk, flags, fieldtx, i);
		}
	}
}

static inline void vc1_b_h_intfi_loop_filter(VC1Context *v, uint8 *dest, const uint32 *cbp, const int *ttblk,
											 uint32 flags, int block_num) {
	PictureState *s = &v->s;
	int pq = v->pq;
	uint8 *dst;
	uint32 block_cbp = cbp[0] >> (block_num * 4);
	int tt;
	int idx, linesize = block_num > 3 ? s->uvlinesize : s->linesize;

	if (block_num > 3)
		dst = dest;
	else
		dst = dest + (block_num & 2) * 4 * s->linesize + (block_num & 1) * 8;

	if (!(flags & RIGHT_EDGE) || !(block_num & 5)) {
		v->vc1dsp.vc1_h_loop_filter8(dst + 8, linesize, pq);
	}

	tt = ttblk[0] >> (block_num * 4) & 0xf;
	if (tt == TT_4X4 || tt == TT_4X8) {
		idx = (block_cbp | (block_cbp >> 1)) & 5;
		if (idx & 1)
			v->vc1dsp.vc1_h_loop_filter4(dst + 4 * linesize + 4, linesize, pq);
		if (idx & 4)
			v->vc1dsp.vc1_h_loop_filter4(dst + 4, linesize, pq);
	}
}

static inline void vc1_b_v_intfi_loop_filter(VC1Context *v, uint8 *dest, const uint32 *cbp, const int *ttblk,
											 uint32 flags, int block_num) {
	PictureState *s = &v->s;
	int pq = v->pq;
	uint8 *dst;
	uint32 block_cbp = cbp[0] >> (block_num * 4);
	int tt;
	int idx, linesize = block_num > 3 ? s->uvlinesize : s->linesize;

	if (block_num > 3)
		dst = dest;
	else
		dst = dest + (block_num & 2) * 4 * s->linesize + (block_num & 1) * 8;

	if (!(flags & BOTTOM_EDGE) || block_num < 2)
		v->vc1dsp.vc1_v_loop_filter8(dst + 8 * linesize, linesize, pq);

	tt = ttblk[0] >> (block_num * 4) & 0xf;
	if (tt == TT_4X4 || tt == TT_8X4) {
		idx = (block_cbp | (block_cbp >> 2)) & 3;
		if (idx & 1)
			v->vc1dsp.vc1_v_loop_filter4(dst + 4 * linesize + 4, linesize, pq);
		if (idx & 2)
			v->vc1dsp.vc1_v_loop_filter4(dst + 4 * linesize, linesize, pq);
	}
}

void ff_vc1_b_intfi_loop_filter(VC1Context *v) {
	PictureState *s = &v->s;
	int block_count = CONFIG_GRAY && (s->avctx->flags & AV_CODEC_FLAG_GRAY) ? 4 : 6;
	uint8 *dest;
	const uint32 *cbp;
	int *ttblk;
	uint32 flags = 0;
	int i;

	/* Within a MB, the vertical loop filter always runs before the horizontal.
	 * To accomplish that, we run the V loop filter on all applicable
	 * horizontal borders of the MB above the currently decoded MB. Then,
	 * we wait for the next loop filter iteration to do H loop filter on all
	 * applicable vertical borders of this MB. Therefore, the loop filter
	 * trails by one row and one column relative to the decoding loop. */
	if (!s->first_slice_line) {
		dest = s->dest[0] - 16 * s->linesize;
		cbp = &v->cbp[s->mb_x - s->mb_stride];
		ttblk = &v->ttblk[s->mb_x - s->mb_stride];
		flags = s->mb_y == s->start_mb_y + 1 ? TOP_EDGE : 0;
		for (i = 0; i < block_count; i++)
			vc1_b_v_intfi_loop_filter(v, i > 3 ? s->dest[i - 3] - 8 * s->uvlinesize : dest, cbp, ttblk, flags, i);
	}
	if (s->mb_y == s->end_mb_y - 1) {
		dest = s->dest[0];
		cbp = &v->cbp[s->mb_x];
		ttblk = &v->ttblk[s->mb_x];
		flags = s->first_slice_line ? TOP_EDGE | BOTTOM_EDGE : BOTTOM_EDGE;
		for (i = 0; i < block_count; i++)
			vc1_b_v_intfi_loop_filter(v, i > 3 ? s->dest[i - 3] : dest, cbp, ttblk, flags, i);
	}

	if (!s->first_slice_line) {
		dest = s->dest[0] - 16 * s->linesize - 16;
		cbp = &v->cbp[s->mb_x - s->mb_stride - 1];
		ttblk = &v->ttblk[s->mb_x - s->mb_stride - 1];
		if (s->mb_x) {
			flags = s->mb_x == 1 ? LEFT_EDGE : 0;
			for (i = 0; i < block_count; i++)
				vc1_b_h_intfi_loop_filter(v, i > 3 ? s->dest[i - 3] - 8 * s->uvlinesize - 8 : dest, cbp, ttblk, flags,
										  i);
		}
		if (s->mb_x == s->mb_width - 1) {
			dest += 16;
			cbp++;
			ttblk++;
			flags = s->mb_x == 0 ? LEFT_EDGE | RIGHT_EDGE : RIGHT_EDGE;
			for (i = 0; i < block_count; i++)
				vc1_b_h_intfi_loop_filter(v, i > 3 ? s->dest[i - 3] - 8 * s->uvlinesize : dest, cbp, ttblk, flags, i);
		}
	}
	if (s->mb_y == s->end_mb_y - 1) {
		dest = s->dest[0] - 16;
		cbp = &v->cbp[s->mb_x - 1];
		ttblk = &v->ttblk[s->mb_x - 1];
		if (s->mb_x) {
			flags = s->mb_x == 1 ? LEFT_EDGE : 0;
			for (i = 0; i < block_count; i++)
				vc1_b_h_intfi_loop_filter(v, i > 3 ? s->dest[i - 3] - 8 : dest, cbp, ttblk, flags, i);
		}
		if (s->mb_x == s->mb_width - 1) {
			dest += 16;
			cbp++;
			ttblk++;
			flags = s->mb_x == 0 ? LEFT_EDGE | RIGHT_EDGE : RIGHT_EDGE;
			for (i = 0; i < block_count; i++)
				vc1_b_h_intfi_loop_filter(v, i > 3 ? s->dest[i - 3] : dest, cbp, ttblk, flags, i);
		}
	}
}

#undef BOTTOM_EDGE
#undef LEFT_EDGE
#undef RIGHT_EDGE
#undef TOP_EDGE

static inline void vc1_scale_luma(uint8 *srcY, int k, int linesize) {
	int i, j;
	for (j = 0; j < k; j++) {
		for (i = 0; i < k; i++)
			srcY[i] = ((srcY[i] - 128) >> 1) + 128;
		srcY += linesize;
	}
}

static inline void vc1_scale_chroma(uint8 *srcU, uint8 *srcV, int k, int uvlinesize) {
	int i, j;
	for (j = 0; j < k; j++) {
		for (i = 0; i < k; i++) {
			srcU[i] = ((srcU[i] - 128) >> 1) + 128;
			srcV[i] = ((srcV[i] - 128) >> 1) + 128;
		}
		srcU += uvlinesize;
		srcV += uvlinesize;
	}
}

static inline void vc1_lut_scale_luma(uint8 *srcY, const uint8 *lut1, const uint8 *lut2, int k, int linesize) {
	int i, j;

	for (j = 0; j < k; j += 2) {
		for (i = 0; i < k; i++)
			srcY[i] = lut1[srcY[i]];
		srcY += linesize;

		if (j + 1 == k)
			break;

		for (i = 0; i < k; i++)
			srcY[i] = lut2[srcY[i]];
		srcY += linesize;
	}
}

static inline void vc1_lut_scale_chroma(uint8 *srcU, uint8 *srcV, const uint8 *lut1, const uint8 *lut2, int k,
										int uvlinesize) {
	int i, j;

	for (j = 0; j < k; j += 2) {
		for (i = 0; i < k; i++) {
			srcU[i] = lut1[srcU[i]];
			srcV[i] = lut1[srcV[i]];
		}
		srcU += uvlinesize;
		srcV += uvlinesize;

		if (j + 1 == k)
			break;

		for (i = 0; i < k; i++) {
			srcU[i] = lut2[srcU[i]];
			srcV[i] = lut2[srcV[i]];
		}
		srcU += uvlinesize;
		srcV += uvlinesize;
	}
}

static const uint8 popcount4[16] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};

static inline int get_luma_mv(VC1Context *v, int dir, int16 *tx, int16 *ty) {
	PictureState *s = &v->s;
	int idx = v->mv_f[dir][s->block_index[0] + v->blocks_off] | (v->mv_f[dir][s->block_index[1] + v->blocks_off] << 1) |
			  (v->mv_f[dir][s->block_index[2] + v->blocks_off] << 2) |
			  (v->mv_f[dir][s->block_index[3] + v->blocks_off] << 3);
	static const uint8 index2[16] = {0, 0, 0, 0x23, 0, 0x13, 0x03, 0, 0, 0x12, 0x02, 0, 0x01, 0, 0, 0};
	int opp_count = popcount4[idx];

	switch (opp_count) {
	case 0:
	case 4:
		*tx = median4(s->mv[dir][0][0], s->mv[dir][1][0], s->mv[dir][2][0], s->mv[dir][3][0]);
		*ty = median4(s->mv[dir][0][1], s->mv[dir][1][1], s->mv[dir][2][1], s->mv[dir][3][1]);
		break;
	case 1:
		*tx = mid_pred(s->mv[dir][idx < 2][0], s->mv[dir][1 + (idx < 4)][0], s->mv[dir][2 + (idx < 8)][0]);
		*ty = mid_pred(s->mv[dir][idx < 2][1], s->mv[dir][1 + (idx < 4)][1], s->mv[dir][2 + (idx < 8)][1]);
		break;
	case 3:
		*tx = mid_pred(s->mv[dir][idx > 0xd][0], s->mv[dir][1 + (idx > 0xb)][0], s->mv[dir][2 + (idx > 0x7)][0]);
		*ty = mid_pred(s->mv[dir][idx > 0xd][1], s->mv[dir][1 + (idx > 0xb)][1], s->mv[dir][2 + (idx > 0x7)][1]);
		break;
	case 2:
		*tx = (s->mv[dir][index2[idx] >> 4][0] + s->mv[dir][index2[idx] & 0xf][0]) / 2;
		*ty = (s->mv[dir][index2[idx] >> 4][1] + s->mv[dir][index2[idx] & 0xf][1]) / 2;
		break;
	}
	return opp_count;
}

static inline int get_chroma_mv(VC1Context *v, int dir, int16 *tx, int16 *ty) {
	PictureState *s = &v->s;
	int idx = !v->mb_type[s->block_index[0]] | (!v->mb_type[s->block_index[1]] << 1) |
			  (!v->mb_type[s->block_index[2]] << 2) | (!v->mb_type[s->block_index[3]] << 3);
	static const uint8 index2[16] = {0, 0, 0, 0x01, 0, 0x02, 0x12, 0, 0, 0x03, 0x13, 0, 0x23, 0, 0, 0};
	int valid_count = popcount4[idx];

	switch (valid_count) {
	case 4:
		*tx = median4(s->mv[dir][0][0], s->mv[dir][1][0], s->mv[dir][2][0], s->mv[dir][3][0]);
		*ty = median4(s->mv[dir][0][1], s->mv[dir][1][1], s->mv[dir][2][1], s->mv[dir][3][1]);
		break;
	case 3:
		*tx = mid_pred(s->mv[dir][idx > 0xd][0], s->mv[dir][1 + (idx > 0xb)][0], s->mv[dir][2 + (idx > 0x7)][0]);
		*ty = mid_pred(s->mv[dir][idx > 0xd][1], s->mv[dir][1 + (idx > 0xb)][1], s->mv[dir][2 + (idx > 0x7)][1]);
		break;
	case 2:
		*tx = (s->mv[dir][index2[idx] >> 4][0] + s->mv[dir][index2[idx] & 0xf][0]) / 2;
		*ty = (s->mv[dir][index2[idx] >> 4][1] + s->mv[dir][index2[idx] & 0xf][1]) / 2;
		break;
	default:
		return 0;
	}
	return valid_count;
}

/** Do motion compensation over 1 macroblock
 * Mostly adapted hpel_motion and qpel_motion from mpegvideo.c
 */
void ff_vc1_mc_1mv(VC1Context *v, int dir) {
	PictureState *s = &v->s;
	ChromaDSP *h264chroma = &v->h264chroma;
	uint8 *srcY, *srcU, *srcV;
	int dxy, mx, my, uvmx, uvmy, src_x, src_y, uvsrc_x, uvsrc_y;
	int v_edge_pos = s->v_edge_pos >> v->field_mode;
	int i;
	const uint8(*luty)[256], (*lutuv)[256];
	int use_ic;
	int interlace;
	int linesize, uvlinesize;

	if ((!v->field_mode || (v->ref_field_type[dir] == 1 && v->cur_field_type == 1)) && !v->s.last_pic.data[0])
		return;

	linesize = s->cur_pic.ptr->f->linesize[0];
	uvlinesize = s->cur_pic.ptr->f->linesize[1];

	mx = s->mv[dir][0][0];
	my = s->mv[dir][0][1];

	// store motion vectors for further use in B-frames
	if (s->pict_type == AV_PICTURE_TYPE_P) {
		for (i = 0; i < 4; i++) {
			s->cur_pic.motion_val[1][s->block_index[i] + v->blocks_off][0] = mx;
			s->cur_pic.motion_val[1][s->block_index[i] + v->blocks_off][1] = my;
		}
	}

	uvmx = (mx + ((mx & 3) == 3)) >> 1;
	uvmy = (my + ((my & 3) == 3)) >> 1;
	v->luma_mv[s->mb_x][0] = uvmx;
	v->luma_mv[s->mb_x][1] = uvmy;

	if (v->field_mode && v->cur_field_type != v->ref_field_type[dir]) {
		my = my - 2 + 4 * v->cur_field_type;
		uvmy = uvmy - 2 + 4 * v->cur_field_type;
	}

	// fastuvmc shall be ignored for interlaced frame picture
	if (v->fastuvmc && (v->fcm != ILACE_FRAME)) {
		uvmx = uvmx + ((uvmx < 0) ? (uvmx & 1) : -(uvmx & 1));
		uvmy = uvmy + ((uvmy < 0) ? (uvmy & 1) : -(uvmy & 1));
	}
	if (!dir) {
		if (v->field_mode && (v->cur_field_type != v->ref_field_type[dir]) && v->second_field) {
			srcY = s->cur_pic.data[0];
			srcU = s->cur_pic.data[1];
			srcV = s->cur_pic.data[2];
			luty = v->curr_luty;
			lutuv = v->curr_lutuv;
			use_ic = *v->curr_use_ic;
			interlace = 1;
		} else {
			srcY = s->last_pic.data[0];
			srcU = s->last_pic.data[1];
			srcV = s->last_pic.data[2];
			luty = v->last_luty;
			lutuv = v->last_lutuv;
			use_ic = v->last_use_ic;
			interlace = v->last_interlaced;
		}
	} else {
		srcY = s->next_pic.data[0];
		srcU = s->next_pic.data[1];
		srcV = s->next_pic.data[2];
		luty = v->next_luty;
		lutuv = v->next_lutuv;
		use_ic = v->next_use_ic;
		interlace = v->next_interlaced;
	}

	if (!srcY || !srcU) {
		warning("Referenced frame missing.\n");
		return;
	}

	src_x = s->mb_x * 16 + (mx >> 2);
	src_y = s->mb_y * 16 + (my >> 2);
	uvsrc_x = s->mb_x * 8 + (uvmx >> 2);
	uvsrc_y = s->mb_y * 8 + (uvmy >> 2);

	if (v->profile != PROFILE_ADVANCED) {
		src_x = av_clip(src_x, -16, s->mb_width * 16);
		src_y = av_clip(src_y, -16, s->mb_height * 16);
		uvsrc_x = av_clip(uvsrc_x, -8, s->mb_width * 8);
		uvsrc_y = av_clip(uvsrc_y, -8, s->mb_height * 8);
	} else {
		src_x = av_clip(src_x, -17, s->avctx->coded_width);
		uvsrc_x = av_clip(uvsrc_x, -8, s->avctx->coded_width >> 1);
		if (v->fcm == ILACE_FRAME) {
			src_y = av_clip(src_y, -18 + (src_y & 1), s->avctx->coded_height + (src_y & 1));
			uvsrc_y = av_clip(uvsrc_y, -8 + (uvsrc_y & 1), (s->avctx->coded_height >> 1) + (uvsrc_y & 1));
		} else {
			src_y = av_clip(src_y, -18, s->avctx->coded_height + 1);
			uvsrc_y = av_clip(uvsrc_y, -8, s->avctx->coded_height >> 1);
		}
	}

	srcY += src_y * s->linesize + src_x;
	srcU += uvsrc_y * s->uvlinesize + uvsrc_x;
	srcV += uvsrc_y * s->uvlinesize + uvsrc_x;

	if (v->field_mode && v->ref_field_type[dir]) {
		srcY += linesize;
		srcU += uvlinesize;
		srcV += uvlinesize;
	}

	/* for grayscale we should not try to read from unknown area */
	if (CONFIG_GRAY && s->avctx->flags & AV_CODEC_FLAG_GRAY) {
		srcU = s->sc.edge_emu_buffer + 18 * s->linesize;
		srcV = s->sc.edge_emu_buffer + 18 * s->linesize;
	}

	if (v->rangeredfrm || use_ic || s->h_edge_pos < 22 || v_edge_pos < 22 ||
		(unsigned)(src_x - s->mspel) > (unsigned)(s->h_edge_pos - (mx & 3) - 16 - s->mspel * 3) ||
		(unsigned)(src_y - 1) > (unsigned)(v_edge_pos - (my & 3) - 16 - 3)) {
		uint8 *ubuf = s->sc.edge_emu_buffer + 19 * s->linesize;
		uint8 *vbuf = ubuf + 9 * s->uvlinesize;
		const int k = 17 + s->mspel * 2;

		srcY -= s->mspel * (1 + s->linesize);
		if (interlace) {
			s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer, srcY, linesize << 1, linesize << 1, k,
									 v->field_mode ? k : (k + 1) >> 1, src_x - s->mspel,
									 (src_y - s->mspel) >> !v->field_mode, s->h_edge_pos, s->v_edge_pos >> 1);
			if (!v->field_mode)
				s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer + linesize, srcY + linesize, linesize << 1,
										 linesize << 1, k, k >> 1, src_x - s->mspel, (src_y - s->mspel + 1) >> 1,
										 s->h_edge_pos, s->v_edge_pos >> 1);
		} else
			s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer, srcY, linesize, linesize, k,
									 v->field_mode ? (k << 1) - 1 : k, src_x - s->mspel,
									 v->field_mode ? 2 * (src_y - s->mspel) + v->ref_field_type[dir] : src_y - s->mspel,
									 s->h_edge_pos, s->v_edge_pos);
		srcY = s->sc.edge_emu_buffer;
		if (interlace) {
			s->vdsp.emulated_edge_mc(ubuf, srcU, uvlinesize << 1, uvlinesize << 1, 9, v->field_mode ? 9 : 5, uvsrc_x,
									 uvsrc_y >> !v->field_mode, s->h_edge_pos >> 1, s->v_edge_pos >> 2);
			s->vdsp.emulated_edge_mc(vbuf, srcV, uvlinesize << 1, uvlinesize << 1, 9, v->field_mode ? 9 : 5, uvsrc_x,
									 uvsrc_y >> !v->field_mode, s->h_edge_pos >> 1, s->v_edge_pos >> 2);
			if (!v->field_mode) {
				s->vdsp.emulated_edge_mc(ubuf + uvlinesize, srcU + uvlinesize, uvlinesize << 1, uvlinesize << 1, 9, 4,
										 uvsrc_x, (uvsrc_y + 1) >> 1, s->h_edge_pos >> 1, s->v_edge_pos >> 2);
				s->vdsp.emulated_edge_mc(vbuf + uvlinesize, srcV + uvlinesize, uvlinesize << 1, uvlinesize << 1, 9, 4,
										 uvsrc_x, (uvsrc_y + 1) >> 1, s->h_edge_pos >> 1, s->v_edge_pos >> 2);
			}
		} else {
			s->vdsp.emulated_edge_mc(ubuf, srcU, uvlinesize, uvlinesize, 9, v->field_mode ? 17 : 9, uvsrc_x,
									 v->field_mode ? 2 * uvsrc_y + v->ref_field_type[dir] : uvsrc_y, s->h_edge_pos >> 1,
									 s->v_edge_pos >> 1);
			s->vdsp.emulated_edge_mc(vbuf, srcV, uvlinesize, uvlinesize, 9, v->field_mode ? 17 : 9, uvsrc_x,
									 v->field_mode ? 2 * uvsrc_y + v->ref_field_type[dir] : uvsrc_y, s->h_edge_pos >> 1,
									 s->v_edge_pos >> 1);
		}
		srcU = ubuf;
		srcV = vbuf;
		/* if we deal with range reduction we need to scale source blocks */
		if (v->rangeredfrm) {
			vc1_scale_luma(srcY, k, s->linesize);
			vc1_scale_chroma(srcU, srcV, 9, s->uvlinesize);
		}
		/* if we deal with intensity compensation we need to scale source blocks */
		if (use_ic) {
			vc1_lut_scale_luma(srcY, luty[v->field_mode ? v->ref_field_type[dir] : ((0 + src_y - s->mspel) & 1)],
							   luty[v->field_mode ? v->ref_field_type[dir] : ((1 + src_y - s->mspel) & 1)], k,
							   s->linesize);
			vc1_lut_scale_chroma(srcU, srcV, lutuv[v->field_mode ? v->ref_field_type[dir] : ((0 + uvsrc_y) & 1)],
								 lutuv[v->field_mode ? v->ref_field_type[dir] : ((1 + uvsrc_y) & 1)], 9, s->uvlinesize);
		}
		srcY += s->mspel * (1 + s->linesize);
	}

	if (s->mspel) {
		dxy = ((my & 3) << 2) | (mx & 3);
		v->vc1dsp.put_vc1_mspel_pixels_tab[0][dxy](s->dest[0], srcY, s->linesize, v->rnd);
	} else { // hpel mc - always used for luma
		dxy = (my & 2) | ((mx & 2) >> 1);
		if (!v->rnd)
			s->hdsp.put_pixels_tab[0][dxy](s->dest[0], srcY, s->linesize, 16);
		else
			s->hdsp.put_no_rnd_pixels_tab[0][dxy](s->dest[0], srcY, s->linesize, 16);
	}

	if (CONFIG_GRAY && s->avctx->flags & AV_CODEC_FLAG_GRAY)
		return;
	/* Chroma MC always uses qpel bilinear */
	uvmx = (uvmx & 3) << 1;
	uvmy = (uvmy & 3) << 1;
	if (!v->rnd) {
		h264chroma->put_h264_chroma_pixels_tab[0](s->dest[1], srcU, s->uvlinesize, 8, uvmx, uvmy);
		h264chroma->put_h264_chroma_pixels_tab[0](s->dest[2], srcV, s->uvlinesize, 8, uvmx, uvmy);
	} else {
		v->vc1dsp.put_no_rnd_vc1_chroma_pixels_tab[0](s->dest[1], srcU, s->uvlinesize, 8, uvmx, uvmy);
		v->vc1dsp.put_no_rnd_vc1_chroma_pixels_tab[0](s->dest[2], srcV, s->uvlinesize, 8, uvmx, uvmy);
	}
	if (v->field_mode) {
		v->mv_f[dir][s->block_index[4] + v->mb_off] = v->cur_field_type != v->ref_field_type[dir];
		v->mv_f[dir][s->block_index[5] + v->mb_off] = v->cur_field_type != v->ref_field_type[dir];
	}
}

/** Do motion compensation for 4-MV macroblock - luminance block
 */
void ff_vc1_mc_4mv_luma(VC1Context *v, int n, int dir, int avg) {
	PictureState *s = &v->s;
	uint8 *srcY;
	int dxy, mx, my, src_x, src_y;
	int off;
	int fieldmv = (v->fcm == ILACE_FRAME) ? v->blk_mv_type[s->block_index[n]] : 0;
	int v_edge_pos = s->v_edge_pos >> v->field_mode;
	const uint8(*luty)[256];
	int use_ic;
	int interlace;
	int linesize;

	if ((!v->field_mode || (v->ref_field_type[dir] == 1 && v->cur_field_type == 1)) && !v->s.last_pic.data[0])
		return;

	linesize = s->cur_pic.ptr->f->linesize[0];

	mx = s->mv[dir][n][0];
	my = s->mv[dir][n][1];

	if (!dir) {
		if (v->field_mode && (v->cur_field_type != v->ref_field_type[dir]) && v->second_field) {
			srcY = s->cur_pic.data[0];
			luty = v->curr_luty;
			use_ic = *v->curr_use_ic;
			interlace = 1;
		} else {
			srcY = s->last_pic.data[0];
			luty = v->last_luty;
			use_ic = v->last_use_ic;
			interlace = v->last_interlaced;
		}
	} else {
		srcY = s->next_pic.data[0];
		luty = v->next_luty;
		use_ic = v->next_use_ic;
		interlace = v->next_interlaced;
	}

	if (!srcY) {
		warning("Referenced frame missing.\n");
		return;
	}

	if (v->field_mode) {
		if (v->cur_field_type != v->ref_field_type[dir])
			my = my - 2 + 4 * v->cur_field_type;
	}

	if (s->pict_type == AV_PICTURE_TYPE_P && n == 3 && v->field_mode) {
		int opp_count = get_luma_mv(v, 0, &s->cur_pic.motion_val[1][s->block_index[0] + v->blocks_off][0],
									&s->cur_pic.motion_val[1][s->block_index[0] + v->blocks_off][1]);
		int k, f = opp_count > 2;
		for (k = 0; k < 4; k++)
			v->mv_f[1][s->block_index[k] + v->blocks_off] = f;
	}

	if (v->fcm == ILACE_FRAME) { // not sure if needed for other types of picture
		int qx, qy;
		int width = s->avctx->coded_width;
		int height = s->avctx->coded_height >> 1;
		if (s->pict_type == AV_PICTURE_TYPE_P) {
			s->cur_pic.motion_val[1][s->block_index[n] + v->blocks_off][0] = mx;
			s->cur_pic.motion_val[1][s->block_index[n] + v->blocks_off][1] = my;
		}
		qx = (s->mb_x * 16) + (mx >> 2);
		qy = (s->mb_y * 8) + (my >> 3);

		if (qx < -17)
			mx -= 4 * (qx + 17);
		else if (qx > width)
			mx -= 4 * (qx - width);
		if (qy < -18)
			my -= 8 * (qy + 18);
		else if (qy > height + 1)
			my -= 8 * (qy - height - 1);
	}

	if ((v->fcm == ILACE_FRAME) && fieldmv)
		off = ((n > 1) ? s->linesize : 0) + (n & 1) * 8;
	else
		off = s->linesize * 4 * (n & 2) + (n & 1) * 8;

	src_x = s->mb_x * 16 + (n & 1) * 8 + (mx >> 2);
	if (!fieldmv)
		src_y = s->mb_y * 16 + (n & 2) * 4 + (my >> 2);
	else
		src_y = s->mb_y * 16 + ((n > 1) ? 1 : 0) + (my >> 2);

	if (v->profile != PROFILE_ADVANCED) {
		src_x = av_clip(src_x, -16, s->mb_width * 16);
		src_y = av_clip(src_y, -16, s->mb_height * 16);
	} else {
		src_x = av_clip(src_x, -17, s->avctx->coded_width);
		if (v->fcm == ILACE_FRAME)
			src_y = av_clip(src_y, -18 + (src_y & 1), s->avctx->coded_height + (src_y & 1));
		else
			src_y = av_clip(src_y, -18, s->avctx->coded_height + 1);
	}

	srcY += src_y * s->linesize + src_x;
	if (v->field_mode && v->ref_field_type[dir])
		srcY += linesize;

	if (v->rangeredfrm || use_ic || s->h_edge_pos < 13 || v_edge_pos < 23 ||
		(unsigned)(src_x - s->mspel) > (unsigned)(s->h_edge_pos - (mx & 3) - 8 - s->mspel * 2) ||
		(unsigned)(src_y - (s->mspel << fieldmv)) > (unsigned)(v_edge_pos - (my & 3) - ((8 + s->mspel * 2) << fieldmv))) {
		const int k = 9 + s->mspel * 2;

		srcY -= s->mspel * (1 + (s->linesize << fieldmv));
		/* check emulate edge stride and offset */
		if (interlace) {
			s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer, srcY, linesize << 1, linesize << 1, k,
									 v->field_mode ? k : ((k << fieldmv) + 1) >> 1, src_x - s->mspel,
									 (src_y - (s->mspel << fieldmv)) >> !v->field_mode, s->h_edge_pos,
									 s->v_edge_pos >> 1);
			if (!v->field_mode && !fieldmv)
				s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer + linesize, srcY + linesize, linesize << 1,
										 linesize << 1, k, k >> 1, src_x - s->mspel, (src_y - s->mspel + 1) >> 1,
										 s->h_edge_pos, s->v_edge_pos >> 1);
		} else
			s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer, srcY, linesize, linesize, k,
									 v->field_mode ? (k << 1) - 1 : k << fieldmv, src_x - s->mspel,
									 v->field_mode ? 2 * (src_y - s->mspel) + v->ref_field_type[dir]
												   : src_y - (s->mspel << fieldmv),
									 s->h_edge_pos, s->v_edge_pos);
		srcY = s->sc.edge_emu_buffer;
		/* if we deal with range reduction we need to scale source blocks */
		if (v->rangeredfrm) {
			vc1_scale_luma(srcY, k, s->linesize << fieldmv);
		}
		/* if we deal with intensity compensation we need to scale source blocks */
		if (use_ic) {
			vc1_lut_scale_luma(
				srcY,
				luty[v->field_mode ? v->ref_field_type[dir] : (((0 << fieldmv) + src_y - (s->mspel << fieldmv)) & 1)],
				luty[v->field_mode ? v->ref_field_type[dir] : (((1 << fieldmv) + src_y - (s->mspel << fieldmv)) & 1)],
				k, s->linesize << fieldmv);
		}
		srcY += s->mspel * (1 + (s->linesize << fieldmv));
	}

	if (s->mspel) {
		dxy = ((my & 3) << 2) | (mx & 3);
		if (avg)
			v->vc1dsp.avg_vc1_mspel_pixels_tab[1][dxy](s->dest[0] + off, srcY, s->linesize << fieldmv, v->rnd);
		else
			v->vc1dsp.put_vc1_mspel_pixels_tab[1][dxy](s->dest[0] + off, srcY, s->linesize << fieldmv, v->rnd);
	} else { // hpel mc - always used for luma
		dxy = (my & 2) | ((mx & 2) >> 1);
		if (!v->rnd)
			s->hdsp.put_pixels_tab[1][dxy](s->dest[0] + off, srcY, s->linesize, 8);
		else
			s->hdsp.put_no_rnd_pixels_tab[1][dxy](s->dest[0] + off, srcY, s->linesize, 8);
	}
}

/** Do motion compensation for 4-MV macroblock - both chroma blocks
 */
void ff_vc1_mc_4mv_chroma(VC1Context *v, int dir) {
	PictureState *s = &v->s;
	ChromaDSP *h264chroma = &v->h264chroma;
	uint8 *srcU, *srcV;
	int uvmx, uvmy, uvsrc_x, uvsrc_y;
	int16 tx, ty;
	int chroma_ref_type;
	int v_edge_pos = s->v_edge_pos >> v->field_mode;
	const uint8(*lutuv)[256];
	int use_ic;
	int interlace;
	int uvlinesize;

	if (!v->field_mode && !v->s.last_pic.data[0])
		return;
	if (CONFIG_GRAY && s->avctx->flags & AV_CODEC_FLAG_GRAY)
		return;

	/* calculate chroma MV vector from four luma MVs */
	if (!v->field_mode || !v->numref) {
		int valid_count = get_chroma_mv(v, dir, &tx, &ty);
		if (!valid_count) {
			s->cur_pic.motion_val[1][s->block_index[0] + v->blocks_off][0] = 0;
			s->cur_pic.motion_val[1][s->block_index[0] + v->blocks_off][1] = 0;
			v->luma_mv[s->mb_x][0] = v->luma_mv[s->mb_x][1] = 0;
			return; // no need to do MC for intra blocks
		}
		chroma_ref_type = v->ref_field_type[dir];
	} else {
		int opp_count = get_luma_mv(v, dir, &tx, &ty);
		chroma_ref_type = v->cur_field_type ^ (opp_count > 2);
	}
	if (v->field_mode && chroma_ref_type == 1 && v->cur_field_type == 1 && !v->s.last_pic.data[0])
		return;
	s->cur_pic.motion_val[1][s->block_index[0] + v->blocks_off][0] = tx;
	s->cur_pic.motion_val[1][s->block_index[0] + v->blocks_off][1] = ty;

	uvlinesize = s->cur_pic.ptr->f->linesize[1];

	uvmx = (tx + ((tx & 3) == 3)) >> 1;
	uvmy = (ty + ((ty & 3) == 3)) >> 1;

	v->luma_mv[s->mb_x][0] = uvmx;
	v->luma_mv[s->mb_x][1] = uvmy;

	if (v->fastuvmc) {
		uvmx = uvmx + ((uvmx < 0) ? (uvmx & 1) : -(uvmx & 1));
		uvmy = uvmy + ((uvmy < 0) ? (uvmy & 1) : -(uvmy & 1));
	}
	// Field conversion bias
	if (v->cur_field_type != chroma_ref_type)
		uvmy += 2 - 4 * chroma_ref_type;

	uvsrc_x = s->mb_x * 8 + (uvmx >> 2);
	uvsrc_y = s->mb_y * 8 + (uvmy >> 2);

	if (v->profile != PROFILE_ADVANCED) {
		uvsrc_x = av_clip(uvsrc_x, -8, s->mb_width * 8);
		uvsrc_y = av_clip(uvsrc_y, -8, s->mb_height * 8);
	} else {
		uvsrc_x = av_clip(uvsrc_x, -8, s->avctx->coded_width >> 1);
		uvsrc_y = av_clip(uvsrc_y, -8, s->avctx->coded_height >> 1);
	}

	if (!dir) {
		if (v->field_mode && (v->cur_field_type != chroma_ref_type) && v->second_field) {
			srcU = s->cur_pic.data[1];
			srcV = s->cur_pic.data[2];
			lutuv = v->curr_lutuv;
			use_ic = *v->curr_use_ic;
			interlace = 1;
		} else {
			srcU = s->last_pic.data[1];
			srcV = s->last_pic.data[2];
			lutuv = v->last_lutuv;
			use_ic = v->last_use_ic;
			interlace = v->last_interlaced;
		}
	} else {
		srcU = s->next_pic.data[1];
		srcV = s->next_pic.data[2];
		lutuv = v->next_lutuv;
		use_ic = v->next_use_ic;
		interlace = v->next_interlaced;
	}

	if (!srcU) {
		warning("Referenced frame missing.\n");
		return;
	}

	srcU += uvsrc_y * s->uvlinesize + uvsrc_x;
	srcV += uvsrc_y * s->uvlinesize + uvsrc_x;

	if (v->field_mode) {
		if (chroma_ref_type) {
			srcU += uvlinesize;
			srcV += uvlinesize;
		}
	}

	if (v->rangeredfrm || use_ic || s->h_edge_pos < 18 || v_edge_pos < 18 ||
		(unsigned)uvsrc_x > (unsigned)((s->h_edge_pos >> 1) - 9) || (unsigned)uvsrc_y > (unsigned)((v_edge_pos >> 1) - 9)) {
		if (interlace) {
			s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer, srcU, uvlinesize << 1, uvlinesize << 1, 9,
									 v->field_mode ? 9 : 5, uvsrc_x, uvsrc_y >> !v->field_mode, s->h_edge_pos >> 1,
									 s->v_edge_pos >> 2);
			s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer + 16, srcV, uvlinesize << 1, uvlinesize << 1, 9,
									 v->field_mode ? 9 : 5, uvsrc_x, uvsrc_y >> !v->field_mode, s->h_edge_pos >> 1,
									 s->v_edge_pos >> 2);
			if (!v->field_mode) {
				s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer + uvlinesize, srcU + uvlinesize, uvlinesize << 1,
										 uvlinesize << 1, 9, 4, uvsrc_x, (uvsrc_y + 1) >> 1, s->h_edge_pos >> 1,
										 s->v_edge_pos >> 2);
				s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer + 16 + uvlinesize, srcV + uvlinesize, uvlinesize << 1,
										 uvlinesize << 1, 9, 4, uvsrc_x, (uvsrc_y + 1) >> 1, s->h_edge_pos >> 1,
										 s->v_edge_pos >> 2);
			}
		} else {
			s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer, srcU, uvlinesize, uvlinesize, 9, v->field_mode ? 17 : 9,
									 uvsrc_x, v->field_mode ? 2 * uvsrc_y + chroma_ref_type : uvsrc_y,
									 s->h_edge_pos >> 1, s->v_edge_pos >> 1);
			s->vdsp.emulated_edge_mc(
				s->sc.edge_emu_buffer + 16, srcV, uvlinesize, uvlinesize, 9, v->field_mode ? 17 : 9, uvsrc_x,
				v->field_mode ? 2 * uvsrc_y + chroma_ref_type : uvsrc_y, s->h_edge_pos >> 1, s->v_edge_pos >> 1);
		}
		srcU = s->sc.edge_emu_buffer;
		srcV = s->sc.edge_emu_buffer + 16;

		/* if we deal with range reduction we need to scale source blocks */
		if (v->rangeredfrm) {
			vc1_scale_chroma(srcU, srcV, 9, s->uvlinesize);
		}
		/* if we deal with intensity compensation we need to scale source blocks */
		if (use_ic) {
			vc1_lut_scale_chroma(srcU, srcV, lutuv[v->field_mode ? chroma_ref_type : ((0 + uvsrc_y) & 1)],
								 lutuv[v->field_mode ? chroma_ref_type : ((1 + uvsrc_y) & 1)], 9, s->uvlinesize);
		}
	}

	/* Chroma MC always uses qpel bilinear */
	uvmx = (uvmx & 3) << 1;
	uvmy = (uvmy & 3) << 1;
	if (!v->rnd) {
		h264chroma->put_h264_chroma_pixels_tab[0](s->dest[1], srcU, s->uvlinesize, 8, uvmx, uvmy);
		h264chroma->put_h264_chroma_pixels_tab[0](s->dest[2], srcV, s->uvlinesize, 8, uvmx, uvmy);
	} else {
		v->vc1dsp.put_no_rnd_vc1_chroma_pixels_tab[0](s->dest[1], srcU, s->uvlinesize, 8, uvmx, uvmy);
		v->vc1dsp.put_no_rnd_vc1_chroma_pixels_tab[0](s->dest[2], srcV, s->uvlinesize, 8, uvmx, uvmy);
	}
	if (v->field_mode) {
		v->mv_f[dir][s->block_index[4] + v->mb_off] = v->cur_field_type != chroma_ref_type;
		v->mv_f[dir][s->block_index[5] + v->mb_off] = v->cur_field_type != chroma_ref_type;
	}
}

/** Do motion compensation for 4-MV interlaced frame chroma macroblock (both U and V)
 */
void ff_vc1_mc_4mv_chroma4(VC1Context *v, int dir, int dir2, int avg) {
	PictureState *s = &v->s;
	ChromaDSP *h264chroma = &v->h264chroma;
	uint8 *srcU, *srcV;
	int uvsrc_x, uvsrc_y;
	int uvmx_field[4], uvmy_field[4];
	int i, off, tx, ty;
	int fieldmv = v->blk_mv_type[s->block_index[0]];
	static const uint8 s_rndtblfield[16] = {0, 0, 1, 2, 4, 4, 5, 6, 2, 2, 3, 8, 6, 6, 7, 12};
	int v_dist = fieldmv ? 1 : 4; // vertical offset for lower sub-blocks
	int v_edge_pos = s->v_edge_pos >> 1;
	int use_ic;
	int interlace;
	int uvlinesize;
	const uint8(*lutuv)[256];

	if (CONFIG_GRAY && s->avctx->flags & AV_CODEC_FLAG_GRAY)
		return;

	uvlinesize = s->cur_pic.ptr->f->linesize[1];

	for (i = 0; i < 4; i++) {
		int d = i < 2 ? dir : dir2;
		tx = s->mv[d][i][0];
		uvmx_field[i] = (tx + ((tx & 3) == 3)) >> 1;
		ty = s->mv[d][i][1];
		if (fieldmv)
			uvmy_field[i] = (ty >> 4) * 8 + s_rndtblfield[ty & 0xF];
		else
			uvmy_field[i] = (ty + ((ty & 3) == 3)) >> 1;
	}

	for (i = 0; i < 4; i++) {
		off = (i & 1) * 4 + ((i & 2) ? v_dist * s->uvlinesize : 0);
		uvsrc_x = s->mb_x * 8 + (i & 1) * 4 + (uvmx_field[i] >> 2);
		uvsrc_y = s->mb_y * 8 + ((i & 2) ? v_dist : 0) + (uvmy_field[i] >> 2);
		// FIXME: implement proper pull-back (see vc1cropmv.c, vc1CROPMV_ChromaPullBack())
		uvsrc_x = av_clip(uvsrc_x, -8, s->avctx->coded_width >> 1);
		if (v->fcm == ILACE_FRAME)
			uvsrc_y = av_clip(uvsrc_y, -8 + (uvsrc_y & 1), (s->avctx->coded_height >> 1) + (uvsrc_y & 1));
		else
			uvsrc_y = av_clip(uvsrc_y, -8, s->avctx->coded_height >> 1);
		if (i < 2 ? dir : dir2) {
			srcU = s->next_pic.data[1];
			srcV = s->next_pic.data[2];
			lutuv = v->next_lutuv;
			use_ic = v->next_use_ic;
			interlace = v->next_interlaced;
		} else {
			srcU = s->last_pic.data[1];
			srcV = s->last_pic.data[2];
			lutuv = v->last_lutuv;
			use_ic = v->last_use_ic;
			interlace = v->last_interlaced;
		}
		if (!srcU)
			return;
		srcU += uvsrc_y * s->uvlinesize + uvsrc_x;
		srcV += uvsrc_y * s->uvlinesize + uvsrc_x;
		uvmx_field[i] = (uvmx_field[i] & 3) << 1;
		uvmy_field[i] = (uvmy_field[i] & 3) << 1;

		if (use_ic || s->h_edge_pos < 10 || v_edge_pos < (5 << fieldmv) ||
			(unsigned)uvsrc_x > (unsigned)((s->h_edge_pos >> 1) - 5) || (unsigned)uvsrc_y > (unsigned)(v_edge_pos - (5 << fieldmv))) {
			if (interlace) {
				s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer, srcU, uvlinesize << 1, uvlinesize << 1, 5,
										 ((5 << fieldmv) + 1) >> 1, uvsrc_x, uvsrc_y >> 1, s->h_edge_pos >> 1,
										 s->v_edge_pos >> 2);
				s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer + 16, srcV, uvlinesize << 1, uvlinesize << 1, 5,
										 ((5 << fieldmv) + 1) >> 1, uvsrc_x, uvsrc_y >> 1, s->h_edge_pos >> 1,
										 s->v_edge_pos >> 2);
				if (!fieldmv) {
					s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer + uvlinesize, srcU + uvlinesize, uvlinesize << 1,
											 uvlinesize << 1, 5, 2, uvsrc_x, (uvsrc_y + 1) >> 1, s->h_edge_pos >> 1,
											 s->v_edge_pos >> 2);
					s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer + 16 + uvlinesize, srcV + uvlinesize,
											 uvlinesize << 1, uvlinesize << 1, 5, 2, uvsrc_x, (uvsrc_y + 1) >> 1,
											 s->h_edge_pos >> 1, s->v_edge_pos >> 2);
				}
			} else {
				s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer, srcU, uvlinesize, uvlinesize, 5, 5 << fieldmv, uvsrc_x,
										 uvsrc_y, s->h_edge_pos >> 1, s->v_edge_pos >> 1);
				s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer + 16, srcV, uvlinesize, uvlinesize, 5, 5 << fieldmv,
										 uvsrc_x, uvsrc_y, s->h_edge_pos >> 1, s->v_edge_pos >> 1);
			}
			srcU = s->sc.edge_emu_buffer;
			srcV = s->sc.edge_emu_buffer + 16;

			/* if we deal with intensity compensation we need to scale source blocks */
			if (use_ic) {
				vc1_lut_scale_chroma(srcU, srcV, lutuv[(uvsrc_y + (0 << fieldmv)) & 1],
									 lutuv[(uvsrc_y + (1 << fieldmv)) & 1], 5, s->uvlinesize << fieldmv);
			}
		}
		if (avg) {
			if (!v->rnd) {
				h264chroma->avg_h264_chroma_pixels_tab[1](s->dest[1] + off, srcU, s->uvlinesize << fieldmv, 4,
														  uvmx_field[i], uvmy_field[i]);
				h264chroma->avg_h264_chroma_pixels_tab[1](s->dest[2] + off, srcV, s->uvlinesize << fieldmv, 4,
														  uvmx_field[i], uvmy_field[i]);
			} else {
				v->vc1dsp.avg_no_rnd_vc1_chroma_pixels_tab[1](s->dest[1] + off, srcU, s->uvlinesize << fieldmv, 4,
															  uvmx_field[i], uvmy_field[i]);
				v->vc1dsp.avg_no_rnd_vc1_chroma_pixels_tab[1](s->dest[2] + off, srcV, s->uvlinesize << fieldmv, 4,
															  uvmx_field[i], uvmy_field[i]);
			}
		} else {
			if (!v->rnd) {
				h264chroma->put_h264_chroma_pixels_tab[1](s->dest[1] + off, srcU, s->uvlinesize << fieldmv, 4,
														  uvmx_field[i], uvmy_field[i]);
				h264chroma->put_h264_chroma_pixels_tab[1](s->dest[2] + off, srcV, s->uvlinesize << fieldmv, 4,
														  uvmx_field[i], uvmy_field[i]);
			} else {
				v->vc1dsp.put_no_rnd_vc1_chroma_pixels_tab[1](s->dest[1] + off, srcU, s->uvlinesize << fieldmv, 4,
															  uvmx_field[i], uvmy_field[i]);
				v->vc1dsp.put_no_rnd_vc1_chroma_pixels_tab[1](s->dest[2] + off, srcV, s->uvlinesize << fieldmv, 4,
															  uvmx_field[i], uvmy_field[i]);
			}
		}
	}
}

/** Motion compensation for direct or interpolated blocks in B-frames
 */
void ff_vc1_interp_mc(VC1Context *v) {
	PictureState *s = &v->s;
	ChromaDSP *h264chroma = &v->h264chroma;
	uint8 *srcY, *srcU, *srcV;
	int dxy, mx, my, uvmx, uvmy, src_x, src_y, uvsrc_x, uvsrc_y;
	int v_edge_pos = s->v_edge_pos >> v->field_mode;
	int use_ic = v->next_use_ic;
	int interlace = v->next_interlaced;
	int linesize, uvlinesize;

	if (!v->field_mode && !v->s.next_pic.data[0])
		return;

	linesize = s->cur_pic.ptr->f->linesize[0];
	uvlinesize = s->cur_pic.ptr->f->linesize[1];

	mx = s->mv[1][0][0];
	my = s->mv[1][0][1];
	uvmx = (mx + ((mx & 3) == 3)) >> 1;
	uvmy = (my + ((my & 3) == 3)) >> 1;
	if (v->field_mode && v->cur_field_type != v->ref_field_type[1]) {
		my = my - 2 + 4 * v->cur_field_type;
		uvmy = uvmy - 2 + 4 * v->cur_field_type;
	}
	if (v->fastuvmc) {
		uvmx = uvmx + ((uvmx < 0) ? -(uvmx & 1) : (uvmx & 1));
		uvmy = uvmy + ((uvmy < 0) ? -(uvmy & 1) : (uvmy & 1));
	}
	srcY = s->next_pic.data[0];
	srcU = s->next_pic.data[1];
	srcV = s->next_pic.data[2];

	src_x = s->mb_x * 16 + (mx >> 2);
	src_y = s->mb_y * 16 + (my >> 2);
	uvsrc_x = s->mb_x * 8 + (uvmx >> 2);
	uvsrc_y = s->mb_y * 8 + (uvmy >> 2);

	if (v->profile != PROFILE_ADVANCED) {
		src_x = av_clip(src_x, -16, s->mb_width * 16);
		src_y = av_clip(src_y, -16, s->mb_height * 16);
		uvsrc_x = av_clip(uvsrc_x, -8, s->mb_width * 8);
		uvsrc_y = av_clip(uvsrc_y, -8, s->mb_height * 8);
	} else {
		src_x = av_clip(src_x, -17, s->avctx->coded_width);
		uvsrc_x = av_clip(uvsrc_x, -8, s->avctx->coded_width >> 1);
		if (v->fcm == ILACE_FRAME) {
			src_y = av_clip(src_y, -18 + (src_y & 1), s->avctx->coded_height + (src_y & 1));
			uvsrc_y = av_clip(uvsrc_y, -8 + (uvsrc_y & 1), (s->avctx->coded_height >> 1) + (uvsrc_y & 1));
		} else {
			src_y = av_clip(src_y, -18, s->avctx->coded_height + 1);
			uvsrc_y = av_clip(uvsrc_y, -8, s->avctx->coded_height >> 1);
		}
	}

	srcY += src_y * s->linesize + src_x;
	srcU += uvsrc_y * s->uvlinesize + uvsrc_x;
	srcV += uvsrc_y * s->uvlinesize + uvsrc_x;

	if (v->field_mode && v->ref_field_type[1]) {
		srcY += linesize;
		srcU += uvlinesize;
		srcV += uvlinesize;
	}

	/* for grayscale we should not try to read from unknown area */
	if (CONFIG_GRAY && s->avctx->flags & AV_CODEC_FLAG_GRAY) {
		srcU = s->sc.edge_emu_buffer + 18 * s->linesize;
		srcV = s->sc.edge_emu_buffer + 18 * s->linesize;
	}

	if (v->rangeredfrm || s->h_edge_pos < 22 || v_edge_pos < 22 || use_ic ||
		(unsigned)(src_x - 1) > (unsigned)(s->h_edge_pos - (mx & 3) - 16 - 3) ||
		(unsigned)(src_y - 1) > (unsigned)(v_edge_pos - (my & 3) - 16 - 3)) {
		uint8 *ubuf = s->sc.edge_emu_buffer + 19 * s->linesize;
		uint8 *vbuf = ubuf + 9 * s->uvlinesize;
		const int k = 17 + s->mspel * 2;

		srcY -= s->mspel * (1 + s->linesize);
		if (interlace) {
			s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer, srcY, linesize << 1, linesize << 1, k,
									 v->field_mode ? k : ((k + 1) >> 1), src_x - s->mspel,
									 (src_y - s->mspel) >> !v->field_mode, s->h_edge_pos, s->v_edge_pos >> 1);
			if (!v->field_mode)
				s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer + linesize, srcY + linesize, linesize << 1,
										 linesize << 1, k, k >> 1, src_x - s->mspel, (src_y - s->mspel + 1) >> 1,
										 s->h_edge_pos, s->v_edge_pos >> 1);
		} else
			s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer, srcY, linesize, linesize, k,
									 v->field_mode ? (k << 1) - 1 : k, src_x - s->mspel,
									 v->field_mode ? 2 * (src_y - s->mspel) + v->ref_field_type[1] : src_y - s->mspel,
									 s->h_edge_pos, s->v_edge_pos);
		srcY = s->sc.edge_emu_buffer;
		if (interlace) {
			s->vdsp.emulated_edge_mc(ubuf, srcU, uvlinesize << 1, uvlinesize << 1, 9, v->field_mode ? 9 : 5, uvsrc_x,
									 uvsrc_y >> !v->field_mode, s->h_edge_pos >> 1, s->v_edge_pos >> 2);
			s->vdsp.emulated_edge_mc(vbuf, srcV, uvlinesize << 1, uvlinesize << 1, 9, v->field_mode ? 9 : 5, uvsrc_x,
									 uvsrc_y >> !v->field_mode, s->h_edge_pos >> 1, s->v_edge_pos >> 2);
			if (!v->field_mode) {
				s->vdsp.emulated_edge_mc(ubuf + uvlinesize, srcU + uvlinesize, uvlinesize << 1, uvlinesize << 1, 9, 4,
										 uvsrc_x, (uvsrc_y + 1) >> 1, s->h_edge_pos >> 1, s->v_edge_pos >> 2);
				s->vdsp.emulated_edge_mc(vbuf + uvlinesize, srcV + uvlinesize, uvlinesize << 1, uvlinesize << 1, 9, 4,
										 uvsrc_x, (uvsrc_y + 1) >> 1, s->h_edge_pos >> 1, s->v_edge_pos >> 2);
			}
		} else {
			s->vdsp.emulated_edge_mc(ubuf, srcU, uvlinesize, uvlinesize, 9, v->field_mode ? 17 : 9, uvsrc_x,
									 v->field_mode ? 2 * uvsrc_y + v->ref_field_type[1] : uvsrc_y, s->h_edge_pos >> 1,
									 s->v_edge_pos >> 1);
			s->vdsp.emulated_edge_mc(vbuf, srcV, uvlinesize, uvlinesize, 9, v->field_mode ? 17 : 9, uvsrc_x,
									 v->field_mode ? 2 * uvsrc_y + v->ref_field_type[1] : uvsrc_y, s->h_edge_pos >> 1,
									 s->v_edge_pos >> 1);
		}
		srcU = ubuf;
		srcV = vbuf;
		/* if we deal with range reduction we need to scale source blocks */
		if (v->rangeredfrm) {
			vc1_scale_luma(srcY, k, s->linesize);
			vc1_scale_chroma(srcU, srcV, 9, s->uvlinesize);
		}

		if (use_ic) {
			const uint8(*luty)[256] = v->next_luty;
			const uint8(*lutuv)[256] = v->next_lutuv;
			vc1_lut_scale_luma(srcY, luty[v->field_mode ? v->ref_field_type[1] : ((0 + src_y - s->mspel) & 1)],
							   luty[v->field_mode ? v->ref_field_type[1] : ((1 + src_y - s->mspel) & 1)], k,
							   s->linesize);
			vc1_lut_scale_chroma(srcU, srcV, lutuv[v->field_mode ? v->ref_field_type[1] : ((0 + uvsrc_y) & 1)],
								 lutuv[v->field_mode ? v->ref_field_type[1] : ((1 + uvsrc_y) & 1)], 9, s->uvlinesize);
		}
		srcY += s->mspel * (1 + s->linesize);
	}

	if (s->mspel) {
		dxy = ((my & 3) << 2) | (mx & 3);
		v->vc1dsp.avg_vc1_mspel_pixels_tab[0][dxy](s->dest[0], srcY, s->linesize, v->rnd);
	} else { // hpel mc
		dxy = (my & 2) | ((mx & 2) >> 1);

		if (!v->rnd)
			s->hdsp.avg_pixels_tab[0][dxy](s->dest[0], srcY, s->linesize, 16);
		else
			s->hdsp.avg_no_rnd_pixels_tab[dxy](s->dest[0], srcY, s->linesize, 16);
	}

	if (CONFIG_GRAY && s->avctx->flags & AV_CODEC_FLAG_GRAY)
		return;
	/* Chroma MC always uses qpel bilinear */
	uvmx = (uvmx & 3) << 1;
	uvmy = (uvmy & 3) << 1;
	if (!v->rnd) {
		h264chroma->avg_h264_chroma_pixels_tab[0](s->dest[1], srcU, s->uvlinesize, 8, uvmx, uvmy);
		h264chroma->avg_h264_chroma_pixels_tab[0](s->dest[2], srcV, s->uvlinesize, 8, uvmx, uvmy);
	} else {
		v->vc1dsp.avg_no_rnd_vc1_chroma_pixels_tab[0](s->dest[1], srcU, s->uvlinesize, 8, uvmx, uvmy);
		v->vc1dsp.avg_no_rnd_vc1_chroma_pixels_tab[0](s->dest[2], srcV, s->uvlinesize, 8, uvmx, uvmy);
	}
}

// offset tables for interlaced picture MVDATA decoding
static const uint8 offset_table[2][9] = {
	{0, 1, 2, 4, 8, 16, 32, 64, 128},
	{0, 1, 3, 7, 15, 31, 63, 127, 255},
};

// mapping table for internal block representation
static const int block_map[6] = {0, 2, 1, 3, 4, 5};

/***********************************************************************/
/**
 * @name VC-1 Bitplane decoding
 * @see 8.7, p56
 * @{
 */

static inline void init_block_index(VC1Context *v) {
	PictureState *s = &v->s;
	ff_init_block_index(s);
	if (v->field_mode && !(v->second_field ^ v->tff)) {
		s->dest[0] += s->cur_pic.ptr->f->linesize[0];
		s->dest[1] += s->cur_pic.ptr->f->linesize[1];
		s->dest[2] += s->cur_pic.ptr->f->linesize[2];
	}
}

static inline void update_block_index(PictureState *s) {
	/* VC1 is always 420 except when using AV_CODEC_FLAG_GRAY
	 * (or a HWAccel). Shall we inline this value? */
	ff_update_block_index(s, 8, 0, s->chroma_x_shift);
}

/** @} */ // Bitplane group

static void vc1_put_blocks_clamped(VC1Context *v, int put_signed) {
	PictureState *s = &v->s;
	uint8 *dest;
	int block_count = CONFIG_GRAY && (s->avctx->flags & AV_CODEC_FLAG_GRAY) ? 4 : 6;
	int fieldtx = 0;
	int i;

	/* The put pixels loop is one MB row and one MB column behind the decoding
	 * loop because we can only put pixels when overlap filtering is done. For
	 * interlaced frame pictures, however, the put pixels loop is only one
	 * column behind the decoding loop as interlaced frame pictures only need
	 * horizontal overlap filtering. */
	if (!s->first_slice_line && v->fcm != ILACE_FRAME) {
		if (s->mb_x) {
			for (i = 0; i < block_count; i++) {
				if (i > 3 ? v->mb_type[s->block_index[i] - s->block_wrap[i] - 1]
						  : v->mb_type[s->block_index[i] - 2 * s->block_wrap[i] - 2]) {
					dest = s->dest[0] + ((i & 2) - 4) * 4 * s->linesize + ((i & 1) - 2) * 8;
					if (put_signed)
						s->idsp.put_signed_pixels_clamped(v->block[v->topleft_blk_idx][block_map[i]],
														  i > 3 ? s->dest[i - 3] - 8 * s->uvlinesize - 8 : dest,
														  i > 3 ? s->uvlinesize : s->linesize);
					else
						s->idsp.put_pixels_clamped(v->block[v->topleft_blk_idx][block_map[i]],
												   i > 3 ? s->dest[i - 3] - 8 * s->uvlinesize - 8 : dest,
												   i > 3 ? s->uvlinesize : s->linesize);
				}
			}
		}
		if (s->mb_x == v->end_mb_x - 1) {
			for (i = 0; i < block_count; i++) {
				if (i > 3 ? v->mb_type[s->block_index[i] - s->block_wrap[i]]
						  : v->mb_type[s->block_index[i] - 2 * s->block_wrap[i]]) {
					dest = s->dest[0] + ((i & 2) - 4) * 4 * s->linesize + (i & 1) * 8;
					if (put_signed)
						s->idsp.put_signed_pixels_clamped(v->block[v->top_blk_idx][block_map[i]],
														  i > 3 ? s->dest[i - 3] - 8 * s->uvlinesize : dest,
														  i > 3 ? s->uvlinesize : s->linesize);
					else
						s->idsp.put_pixels_clamped(v->block[v->top_blk_idx][block_map[i]],
												   i > 3 ? s->dest[i - 3] - 8 * s->uvlinesize : dest,
												   i > 3 ? s->uvlinesize : s->linesize);
				}
			}
		}
	}
	if (s->mb_y == s->end_mb_y - 1 || v->fcm == ILACE_FRAME) {
		if (s->mb_x) {
			if (v->fcm == ILACE_FRAME)
				fieldtx = v->fieldtx_plane[s->mb_y * s->mb_stride + s->mb_x - 1];
			for (i = 0; i < block_count; i++) {
				if (i > 3 ? v->mb_type[s->block_index[i] - 1] : v->mb_type[s->block_index[i] - 2]) {
					if (fieldtx)
						dest = s->dest[0] + ((i & 2) >> 1) * s->linesize + ((i & 1) - 2) * 8;
					else
						dest = s->dest[0] + (i & 2) * 4 * s->linesize + ((i & 1) - 2) * 8;
					if (put_signed)
						s->idsp.put_signed_pixels_clamped(v->block[v->left_blk_idx][block_map[i]],
														  i > 3 ? s->dest[i - 3] - 8 : dest,
														  i > 3 ? s->uvlinesize : s->linesize << fieldtx);
					else
						s->idsp.put_pixels_clamped(v->block[v->left_blk_idx][block_map[i]],
												   i > 3 ? s->dest[i - 3] - 8 : dest,
												   i > 3 ? s->uvlinesize : s->linesize << fieldtx);
				}
			}
		}
		if (s->mb_x == v->end_mb_x - 1) {
			if (v->fcm == ILACE_FRAME)
				fieldtx = v->fieldtx_plane[s->mb_y * s->mb_stride + s->mb_x];
			for (i = 0; i < block_count; i++) {
				if (v->mb_type[s->block_index[i]]) {
					if (fieldtx)
						dest = s->dest[0] + ((i & 2) >> 1) * s->linesize + (i & 1) * 8;
					else
						dest = s->dest[0] + (i & 2) * 4 * s->linesize + (i & 1) * 8;
					if (put_signed)
						s->idsp.put_signed_pixels_clamped(v->block[v->cur_blk_idx][block_map[i]],
														  i > 3 ? s->dest[i - 3] : dest,
														  i > 3 ? s->uvlinesize : s->linesize << fieldtx);
					else
						s->idsp.put_pixels_clamped(v->block[v->cur_blk_idx][block_map[i]],
												   i > 3 ? s->dest[i - 3] : dest,
												   i > 3 ? s->uvlinesize : s->linesize << fieldtx);
				}
			}
		}
	}
}

#define inc_blk_idx(idx)                                                                                               \
	do {                                                                                                               \
		idx++;                                                                                                         \
		if (idx >= v->n_allocated_blks)                                                                                \
			idx = 0;                                                                                                   \
	} while (0)

/***********************************************************************/
/**
 * @name VC-1 Block-level functions
 * @see 7.1.4, p91 and 8.1.1.7, p(1)04
 * @{
 */

/**
 * @def GET_MQUANT
 * @brief Get macroblock-level quantizer scale
 */
#define GET_MQUANT()                                                                                                   \
	if (v->dquantfrm) {                                                                                                \
		int edges = 0;                                                                                                 \
		if (v->dqprofile == DQPROFILE_ALL_MBS) {                                                                       \
			if (v->dqbilevel) {                                                                                        \
				mquant = (get_bits1(gb)) ? -v->altpq : v->pq;                                                          \
			} else {                                                                                                   \
				mqdiff = get_bits(gb, 3);                                                                              \
				if (mqdiff != 7)                                                                                       \
					mquant = -v->pq - mqdiff;                                                                          \
				else                                                                                                   \
					mquant = -get_bits(gb, 5);                                                                         \
			}                                                                                                          \
		}                                                                                                              \
		if (v->dqprofile == DQPROFILE_SINGLE_EDGE)                                                                     \
			edges = 1 << v->dqsbedge;                                                                                  \
		else if (v->dqprofile == DQPROFILE_DOUBLE_EDGES)                                                               \
			edges = (3 << v->dqsbedge) % 15;                                                                           \
		else if (v->dqprofile == DQPROFILE_FOUR_EDGES)                                                                 \
			edges = 15;                                                                                                \
		if ((edges & 1) && !s->mb_x)                                                                                   \
			mquant = -v->altpq;                                                                                        \
		if ((edges & 2) && !s->mb_y)                                                                                   \
			mquant = -v->altpq;                                                                                        \
		if ((edges & 4) && s->mb_x == (s->mb_width - 1))                                                               \
			mquant = -v->altpq;                                                                                        \
		if ((edges & 8) && s->mb_y == ((s->mb_height >> v->field_mode) - 1))                                           \
			mquant = -v->altpq;                                                                                        \
		if (!mquant || mquant > 31 || mquant < -31) {                                                                  \
			warning("Overriding invalid mquant %d\n", mquant);                                                         \
			mquant = 1;                                                                                                \
		}                                                                                                              \
	}

/**
 * @def GET_MVDATA(_dmv_x, _dmv_y)
 * @brief Get MV differentials
 * @see MVDATA decoding from 8.3.5.2, p(1)20
 * @param _dmv_x Horizontal differential for decoded MV
 * @param _dmv_y Vertical differential for decoded MV
 */
#define GET_MVDATA(_dmv_x, _dmv_y)                                                                                     \
	index = 1 + get_vlc2(gb, ff_vc1_mv_diff_vlc[v->mv_table_index]);                                                   \
	if (index > 36) {                                                                                                  \
		mb_has_coeffs = 1;                                                                                             \
		index -= 37;                                                                                                   \
	} else                                                                                                             \
		mb_has_coeffs = 0;                                                                                             \
	s->mb_intra = 0;                                                                                                   \
	if (!index) {                                                                                                      \
		_dmv_x = _dmv_y = 0;                                                                                           \
	} else if (index == 35) {                                                                                          \
		_dmv_x = get_bits(gb, v->k_x - 1 + s->quarter_sample);                                                         \
		_dmv_y = get_bits(gb, v->k_y - 1 + s->quarter_sample);                                                         \
	} else if (index == 36) {                                                                                          \
		_dmv_x = 0;                                                                                                    \
		_dmv_y = 0;                                                                                                    \
		s->mb_intra = 1;                                                                                               \
	} else {                                                                                                           \
		index1 = index % 6;                                                                                            \
		_dmv_x = offset_table[1][index1];                                                                              \
		val = size_table[index1] - (!s->quarter_sample && index1 == 5);                                                \
		if (val > 0) {                                                                                                 \
			val = get_bits(gb, val);                                                                                   \
			sign = 0 - (val & 1);                                                                                      \
			_dmv_x = (sign ^ ((val >> 1) + _dmv_x)) - sign;                                                            \
		}                                                                                                              \
                                                                                                                       \
		index1 = index / 6;                                                                                            \
		_dmv_y = offset_table[1][index1];                                                                              \
		val = size_table[index1] - (!s->quarter_sample && index1 == 5);                                                \
		if (val > 0) {                                                                                                 \
			val = get_bits(gb, val);                                                                                   \
			sign = 0 - (val & 1);                                                                                      \
			_dmv_y = (sign ^ ((val >> 1) + _dmv_y)) - sign;                                                            \
		}                                                                                                              \
	}

static inline void get_mvdata_interlaced(VC1Context *v, int *dmv_x, int *dmv_y, int *pred_flag) {
	int index, index1;
	int extend_x, extend_y;
	Bits *const gb = &v->gb;
	int esc;
	int val, sign;

	if (v->numref) {

		esc = 125;
	} else {

		esc = 71;
	}
	extend_x = v->dmvrange & 1;
	extend_y = (v->dmvrange >> 1) & 1;
	index = get_vlc2(gb, v->imv_vlc);
	if (index == esc) {
		*dmv_x = get_bits(gb, v->k_x);
		*dmv_y = get_bits(gb, v->k_y);
		if (v->numref) {
			if (pred_flag)
				*pred_flag = *dmv_y & 1;
			*dmv_y = (*dmv_y + (*dmv_y & 1)) >> 1;
		}
	} else {
		assert(index < esc);
		index1 = (index + 1) % 9;
		if (index1 != 0) {
			val = get_bits(gb, index1 + extend_x);
			sign = 0 - (val & 1);
			*dmv_x = (sign ^ ((val >> 1) + offset_table[extend_x][index1])) - sign;
		} else
			*dmv_x = 0;
		index1 = (index + 1) / 9;
		if (index1 > v->numref) {
			val = get_bits(gb, (index1 >> v->numref) + extend_y);
			sign = 0 - (val & 1);
			*dmv_y = (sign ^ ((val >> 1) + offset_table[extend_y][index1 >> v->numref])) - sign;
		} else
			*dmv_y = 0;
		if (v->numref && pred_flag)
			*pred_flag = index1 & 1;
	}
}

/** Reconstruct motion vector for B-frame and do motion compensation
 */
static inline void vc1_b_mc(VC1Context *v, int dmv_x[2], int dmv_y[2], int direct, int mode) {
	if (direct) {
		ff_vc1_mc_1mv(v, 0);
		ff_vc1_interp_mc(v);
		return;
	}
	if (mode == BMV_TYPE_INTERPOLATED) {
		ff_vc1_mc_1mv(v, 0);
		ff_vc1_interp_mc(v);
		return;
	}

	ff_vc1_mc_1mv(v, (mode == BMV_TYPE_BACKWARD));
}

/** Get predicted DC value for I-frames only
 * prediction dir: left=0, top=1
 * @param s PictureState
 * @param overlap flag indicating that overlap filtering is used
 * @param pq integer part of picture quantizer
 * @param[in] n block index in the current MB
 * @param dc_val_ptr Pointer to DC predictor
 * @param dir_ptr Prediction direction for use in AC prediction
 */
static inline int vc1_i_pred_dc(PictureState *s, int overlap, int pq, int n, int16 **dc_val_ptr, int *dir_ptr) {
	int a, b, c, wrap, pred, scale;
	int16 *dc_val;
	static const uint16 dcpred[32] = {65535, 1024, 512, 341, 256, 205, 171, 146, 128, 114, 102, 93, 85, 79, 73, 68,
									  64,    60,   57,  54,  51,  49,  47,  45,  43,  41,  39,  38, 37, 35, 34, 33};

	/* find prediction - wmv3_dc_scale always used here in fact */
	scale = s->y_dc_scale;

	wrap = s->block_wrap[n];
	dc_val = s->dc_val + s->block_index[n];

	/* B A
	 * C X
	 */
	c = dc_val[-1];
	b = dc_val[-1 - wrap];
	a = dc_val[-wrap];

	if (pq < 9 || !overlap) {
		/* Set outer values */
		if (s->first_slice_line && (n != 2 && n != 3))
			b = a = dcpred[scale];
		if (s->mb_x == 0 && (n != 1 && n != 3))
			b = c = dcpred[scale];
	} else {
		/* Set outer values */
		if (s->first_slice_line && (n != 2 && n != 3))
			b = a = 0;
		if (s->mb_x == 0 && (n != 1 && n != 3))
			b = c = 0;
	}

	if (abs(a - b) <= abs(b - c)) {
		pred = c;
		*dir_ptr = 1; // left
	} else {
		pred = a;
		*dir_ptr = 0; // top
	}

	/* update predictor */
	*dc_val_ptr = &dc_val[0];
	return pred;
}

/** Get predicted DC value
 * prediction dir: left=0, top=1
 * @param s PictureState
 * @param overlap flag indicating that overlap filtering is used
 * @param pq integer part of picture quantizer
 * @param[in] n block index in the current MB
 * @param a_avail flag indicating top block availability
 * @param c_avail flag indicating left block availability
 * @param dc_val_ptr Pointer to DC predictor
 * @param dir_ptr Prediction direction for use in AC prediction
 */
static inline int ff_vc1_pred_dc(PictureState *s, int overlap, int pq, int n, int a_avail, int c_avail,
								 int16 **dc_val_ptr, int *dir_ptr) {
	int a, b, c, wrap, pred;
	int16 *dc_val;
	int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
	int q1, q2 = 0;
	int dqscale_index;

	/* scale predictors if needed */
	q1 = ABS(s->cur_pic.qscale_table[mb_pos]);
	dqscale_index = ff_wmv3_dc_scale_table[q1] - 1;
	if (dqscale_index < 0)
		return 0;

	wrap = s->block_wrap[n];
	dc_val = s->dc_val + s->block_index[n];

	/* B A
	 * C X
	 */
	c = dc_val[-1];
	b = dc_val[-1 - wrap];
	a = dc_val[-wrap];

	if (c_avail && (n != 1 && n != 3)) {
		q2 = ABS(s->cur_pic.qscale_table[mb_pos - 1]);
		if (q2 && q2 != q1)
			c = (int)((unsigned)c * ff_wmv3_dc_scale_table[q2] * ff_vc1_dqscale[dqscale_index] + 0x20000) >> 18;
	}
	if (a_avail && (n != 2 && n != 3)) {
		q2 = ABS(s->cur_pic.qscale_table[mb_pos - s->mb_stride]);
		if (q2 && q2 != q1)
			a = (int)((unsigned)a * ff_wmv3_dc_scale_table[q2] * ff_vc1_dqscale[dqscale_index] + 0x20000) >> 18;
	}
	if (a_avail && c_avail && (n != 3)) {
		int off = mb_pos;
		if (n != 1)
			off--;
		if (n != 2)
			off -= s->mb_stride;
		q2 = ABS(s->cur_pic.qscale_table[off]);
		if (q2 && q2 != q1)
			b = (int)((unsigned)b * ff_wmv3_dc_scale_table[q2] * ff_vc1_dqscale[dqscale_index] + 0x20000) >> 18;
	}

	if (c_avail && (!a_avail || abs(a - b) <= abs(b - c))) {
		pred = c;
		*dir_ptr = 1; // left
	} else if (a_avail) {
		pred = a;
		*dir_ptr = 0; // top
	} else {
		pred = 0;
		*dir_ptr = 1; // left
	}

	/* update predictor */
	*dc_val_ptr = &dc_val[0];
	return pred;
}

/** @} */ // Block group

/**
 * @name VC1 Macroblock-level functions in Simple/Main Profiles
 * @see 7.1.4, p91 and 8.1.1.7, p(1)04
 * @{
 */

static inline int vc1_coded_block_pred(PictureState *const s, int n, int diff) {
	int xy, wrap, pred, a, b, c;

	xy = s->block_index[n];
	wrap = s->b8_stride;

	/* B C
	 * A X
	 */
	a = s->coded_block[xy - 1];
	b = s->coded_block[xy - 1 - wrap];
	c = s->coded_block[xy - wrap];

	if (b == c) {
		pred = a;
	} else {
		pred = c;
	}

	/* store value */
	s->coded_block[xy] = pred ^ diff;

	return pred ^ diff;
}

/**
 * Decode one AC coefficient
 * @param v The VC1 context
 * @param last Last coefficient
 * @param skip How much zero coefficients to skip
 * @param value Decoded AC coefficient value
 * @param codingset set of VLC to decode data
 * @see 8.1.3.4
 */
static int vc1_decode_ac_coeff(VC1Context *v, int *last, int *skip, int *value, int codingset) {
	Bits *const gb = &v->gb;
	int index, run, level, lst, sign;

	index = get_vlc2(gb, ff_vc1_ac_coeff_table[codingset]);
	if (index < 0)
		return index;
	if (index != ff_vc1_ac_sizes[codingset] - 1) {
		run = vc1_index_decode_table[codingset][index][0];
		level = vc1_index_decode_table[codingset][index][1];
		lst = index >= vc1_last_decode_table[codingset] || get_bits_left(gb) < 0;
		sign = get_bits1(gb);
	} else {
		int escape = decode210(gb);
		if (escape != 2) {
			index = get_vlc2(gb, ff_vc1_ac_coeff_table[codingset]);
			if ((unsigned)index >= ff_vc1_ac_sizes[codingset] - 1U)
				return (-1);
			run = vc1_index_decode_table[codingset][index][0];
			level = vc1_index_decode_table[codingset][index][1];
			lst = index >= vc1_last_decode_table[codingset];
			if (escape == 0) {
				if (lst)
					level += vc1_last_delta_level_table[codingset][run];
				else
					level += vc1_delta_level_table[codingset][run];
			} else {
				if (lst)
					run += vc1_last_delta_run_table[codingset][level] + 1;
				else
					run += vc1_delta_run_table[codingset][level] + 1;
			}
			sign = get_bits1(gb);
		} else {
			lst = get_bits1(gb);
			if (v->esc3_level_length == 0) {
				if (v->pq < 8 || v->dquantfrm) { // table 59
					v->esc3_level_length = get_bits(gb, 3);
					if (!v->esc3_level_length)
						v->esc3_level_length = get_bits(gb, 2) + 8;
				} else { // table 60
					v->esc3_level_length = get_unary(gb, 1, 6) + 2;
				}
				v->esc3_run_length = 3 + get_bits(gb, 2);
			}
			run = get_bits(gb, v->esc3_run_length);
			sign = get_bits1(gb);
			level = get_bits(gb, v->esc3_level_length);
		}
	}

	*last = lst;
	*skip = run;
	*value = (level ^ -sign) + sign;

	return 0;
}

/** Decode intra block in intra frames - should be faster than decode_intra_block
 * @param v VC1Context
 * @param block block to decode
 * @param[in] n subblock index
 * @param coded are AC coeffs present or not
 * @param codingset set of VLC to decode data
 */
static int vc1_decode_i_block(VC1Context *v, int16 block[64], int n, int coded, int codingset) {
	Bits *const gb = &v->gb;
	PictureState *s = &v->s;
	int dc_pred_dir = 0; /* Direction of the DC prediction used */
	int16 *dc_val;
	int16 *ac_val, *ac_val2;
	int dcdiff, scale;

	/* Get DC differential */
	dcdiff = get_vlc2(gb, ff_msmp4_dc_vlc[v->dc_table_index][n >= 4]);
	if (dcdiff) {
		const int m = (v->pq == 1 || v->pq == 2) ? 3 - v->pq : 0;
		if (dcdiff == 119 /* ESC index value */) {
			dcdiff = get_bits(gb, 8 + m);
		} else {
			if (m)
				dcdiff = (dcdiff << m) + get_bits(gb, m) - ((1 << m) - 1);
		}
		if (get_bits1(gb))
			dcdiff = -dcdiff;
	}

	/* Prediction */
	dcdiff += vc1_i_pred_dc(&v->s, v->overlap, v->pq, n, &dc_val, &dc_pred_dir);
	*dc_val = dcdiff;

	/* Store the quantized DC coeff, used for prediction */
	block[0] = dcdiff * s->y_dc_scale;

	ac_val = s->ac_val[s->block_index[n]];
	ac_val2 = ac_val;
	if (dc_pred_dir) // left
		ac_val -= 16;
	else // top
		ac_val -= 16 * s->block_wrap[n];

	scale = v->pq * 2 + v->halfpq;

	// AC Decoding

	if (coded) {
		int last = 0, skip, value;
		const uint8 *zz_table;
		int k;

		if (v->s.ac_pred) {
			if (!dc_pred_dir)
				zz_table = v->zz_8x8[2];
			else
				zz_table = v->zz_8x8[3];
		} else
			zz_table = v->zz_8x8[1];

		for (int i = 1; !last; ++i) {
			int ret = vc1_decode_ac_coeff(v, &last, &skip, &value, codingset);
			if (ret < 0)
				return ret;
			i += skip;
			if (i > 63)
				break;
			block[zz_table[i]] = value;
		}

		/* apply AC prediction if needed */
		if (s->ac_pred) {
			int sh;
			if (dc_pred_dir) { // left
				sh = v->left_blk_sh;
			} else { // top
				sh = v->top_blk_sh;
				ac_val += 8;
			}
			for (k = 1; k < 8; k++)
				block[k << sh] += ac_val[k];
		}
		/* save AC coeffs for further prediction */
		for (k = 1; k < 8; k++) {
			ac_val2[k] = block[k << v->left_blk_sh];
			ac_val2[k + 8] = block[k << v->top_blk_sh];
		}

		/* scale AC coeffs */
		for (k = 1; k < 64; k++)
			if (block[k]) {
				block[k] *= scale;
				if (!v->pquantizer)
					block[k] += (block[k] < 0) ? -v->pq : v->pq;
			}

	} else {
		int k;

		memset(ac_val2, 0, 16 * 2);

		/* apply AC prediction if needed */
		if (s->ac_pred) {
			int sh;
			if (dc_pred_dir) { // left
				sh = v->left_blk_sh;
			} else { // top
				sh = v->top_blk_sh;
				ac_val += 8;
				ac_val2 += 8;
			}
			memcpy(ac_val2, ac_val, 8 * 2);
			for (k = 1; k < 8; k++) {
				block[k << sh] = ac_val[k] * scale;
				if (!v->pquantizer && block[k << sh])
					block[k << sh] += (block[k << sh] < 0) ? -v->pq : v->pq;
			}
		}
	}

	return 0;
}

/** Decode intra block in intra frames - should be faster than decode_intra_block
 * @param v VC1Context
 * @param block block to decode
 * @param[in] n subblock number
 * @param coded are AC coeffs present or not
 * @param codingset set of VLC to decode data
 * @param mquant quantizer value for this macroblock
 */
static int vc1_decode_i_block_adv(VC1Context *v, int16 block[64], int n, int coded, int codingset, int mquant) {
	Bits *const gb = &v->gb;
	PictureState *s = &v->s;
	int dc_pred_dir = 0; /* Direction of the DC prediction used */
	int16 *dc_val = NULL;
	int16 *ac_val, *ac_val2;
	int dcdiff;
	int a_avail = v->a_avail, c_avail = v->c_avail;
	int use_pred = s->ac_pred;
	int scale;
	int q1, q2 = 0;
	int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
	int quant = ABS(mquant);

	/* Get DC differential */
	dcdiff = get_vlc2(gb, ff_msmp4_dc_vlc[v->dc_table_index][n >= 4]);
	if (dcdiff) {
		const int m = (quant == 1 || quant == 2) ? 3 - quant : 0;
		if (dcdiff == 119 /* ESC index value */) {
			dcdiff = get_bits(gb, 8 + m);
		} else {
			if (m)
				dcdiff = (dcdiff << m) + get_bits(gb, m) - ((1 << m) - 1);
		}
		if (get_bits1(gb))
			dcdiff = -dcdiff;
	}

	/* Prediction */
	dcdiff += ff_vc1_pred_dc(&v->s, v->overlap, quant, n, v->a_avail, v->c_avail, &dc_val, &dc_pred_dir);
	*dc_val = dcdiff;

	/* Store the quantized DC coeff, used for prediction */
	block[0] = dcdiff * s->y_dc_scale;

	/* check if AC is needed at all */
	if (!a_avail && !c_avail)
		use_pred = 0;

	scale = quant * 2 + ((mquant < 0) ? 0 : v->halfpq);

	ac_val = s->ac_val[s->block_index[n]];
	ac_val2 = ac_val;
	if (dc_pred_dir) // left
		ac_val -= 16;
	else // top
		ac_val -= 16 * s->block_wrap[n];

	q1 = s->cur_pic.qscale_table[mb_pos];
	if (n == 3)
		q2 = q1;
	else if (dc_pred_dir) {
		if (n == 1)
			q2 = q1;
		else if (c_avail && mb_pos)
			q2 = s->cur_pic.qscale_table[mb_pos - 1];
	} else {
		if (n == 2)
			q2 = q1;
		else if (a_avail && mb_pos >= s->mb_stride)
			q2 = s->cur_pic.qscale_table[mb_pos - s->mb_stride];
	}

	// AC Decoding

	if (coded) {
		int last = 0, skip, value;
		const uint8 *zz_table;
		int k;

		if (v->s.ac_pred) {
			if (!use_pred && v->fcm == ILACE_FRAME) {
				zz_table = v->zzi_8x8;
			} else {
				if (!dc_pred_dir) // top
					zz_table = v->zz_8x8[2];
				else // left
					zz_table = v->zz_8x8[3];
			}
		} else {
			if (v->fcm != ILACE_FRAME)
				zz_table = v->zz_8x8[1];
			else
				zz_table = v->zzi_8x8;
		}

		for (int i = 1; !last; ++i) {
			int ret = vc1_decode_ac_coeff(v, &last, &skip, &value, codingset);
			if (ret < 0)
				return ret;
			i += skip;
			if (i > 63)
				break;
			block[zz_table[i]] = value;
		}

		/* apply AC prediction if needed */
		if (use_pred) {
			int sh;
			if (dc_pred_dir) { // left
				sh = v->left_blk_sh;
			} else { // top
				sh = v->top_blk_sh;
				ac_val += 8;
			}
			/* scale predictors if needed*/
			q1 = ABS(q1) * 2 + ((q1 < 0) ? 0 : v->halfpq) - 1;
			if (q1 < 1)
				return (-1);
			if (q2)
				q2 = ABS(q2) * 2 + ((q2 < 0) ? 0 : v->halfpq) - 1;
			if (q2 && q1 != q2) {
				for (k = 1; k < 8; k++)
					block[k << sh] += (int)(ac_val[k] * (unsigned)q2 * ff_vc1_dqscale[q1 - 1] + 0x20000) >> 18;
			} else {
				for (k = 1; k < 8; k++)
					block[k << sh] += ac_val[k];
			}
		}
		/* save AC coeffs for further prediction */
		for (k = 1; k < 8; k++) {
			ac_val2[k] = block[k << v->left_blk_sh];
			ac_val2[k + 8] = block[k << v->top_blk_sh];
		}

		/* scale AC coeffs */
		for (k = 1; k < 64; k++)
			if (block[k]) {
				block[k] *= scale;
				if (!v->pquantizer)
					block[k] += (block[k] < 0) ? -quant : quant;
			}

	} else { // no AC coeffs
		int k;

		memset(ac_val2, 0, 16 * 2);

		/* apply AC prediction if needed */
		if (use_pred) {
			int sh;
			if (dc_pred_dir) { // left
				sh = v->left_blk_sh;
			} else { // top
				sh = v->top_blk_sh;
				ac_val += 8;
				ac_val2 += 8;
			}
			memcpy(ac_val2, ac_val, 8 * 2);
			q1 = ABS(q1) * 2 + ((q1 < 0) ? 0 : v->halfpq) - 1;
			if (q1 < 1)
				return (-1);
			if (q2)
				q2 = ABS(q2) * 2 + ((q2 < 0) ? 0 : v->halfpq) - 1;
			if (q2 && q1 != q2) {
				for (k = 1; k < 8; k++)
					ac_val2[k] = (int)(ac_val2[k] * q2 * (unsigned)ff_vc1_dqscale[q1 - 1] + 0x20000) >> 18;
			}
			for (k = 1; k < 8; k++) {
				block[k << sh] = ac_val2[k] * scale;
				if (!v->pquantizer && block[k << sh])
					block[k << sh] += (block[k << sh] < 0) ? -quant : quant;
			}
		}
	}

	return 0;
}

/** Decode intra block in inter frames - more generic version than vc1_decode_i_block
 * @param v VC1Context
 * @param block block to decode
 * @param[in] n subblock index
 * @param coded are AC coeffs present or not
 * @param mquant block quantizer
 * @param codingset set of VLC to decode data
 */
static int vc1_decode_intra_block(VC1Context *v, int16 block[64], int n, int coded, int mquant, int codingset) {
	Bits *const gb = &v->gb;
	PictureState *s = &v->s;
	int dc_pred_dir = 0; /* Direction of the DC prediction used */
	int16 *dc_val = NULL;
	int16 *ac_val, *ac_val2;
	int dcdiff;
	int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
	int a_avail = v->a_avail, c_avail = v->c_avail;
	int use_pred = s->ac_pred;
	int scale;
	int q1, q2 = 0;
	int quant = ABS(mquant);

	s->bdsp.clear_block(block);

	/* XXX: Guard against dumb values of mquant */
	quant = av_clip_uintp2(quant, 5);

	/* Set DC scale - y and c use the same so we only set y */
	s->y_dc_scale = ff_wmv3_dc_scale_table[quant];

	/* Get DC differential */
	dcdiff = get_vlc2(gb, ff_msmp4_dc_vlc[v->dc_table_index][n >= 4]);
	if (dcdiff) {
		const int m = (quant == 1 || quant == 2) ? 3 - quant : 0;
		if (dcdiff == 119 /* ESC index value */) {
			dcdiff = get_bits(gb, 8 + m);
		} else {
			if (m)
				dcdiff = (dcdiff << m) + get_bits(gb, m) - ((1 << m) - 1);
		}
		if (get_bits1(gb))
			dcdiff = -dcdiff;
	}

	/* Prediction */
	dcdiff += ff_vc1_pred_dc(&v->s, v->overlap, quant, n, a_avail, c_avail, &dc_val, &dc_pred_dir);
	*dc_val = dcdiff;

	/* Store the quantized DC coeff, used for prediction */
	block[0] = dcdiff * s->y_dc_scale;

	// AC Decoding

	/* check if AC is needed at all and adjust direction if needed */
	if (!a_avail)
		dc_pred_dir = 1;
	if (!c_avail)
		dc_pred_dir = 0;
	if (!a_avail && !c_avail)
		use_pred = 0;
	ac_val = s->ac_val[s->block_index[n]];
	ac_val2 = ac_val;

	scale = quant * 2 + ((mquant < 0) ? 0 : v->halfpq);

	if (dc_pred_dir) // left
		ac_val -= 16;
	else // top
		ac_val -= 16 * s->block_wrap[n];

	q1 = s->cur_pic.qscale_table[mb_pos];
	if (dc_pred_dir && c_avail && mb_pos)
		q2 = s->cur_pic.qscale_table[mb_pos - 1];
	if (!dc_pred_dir && a_avail && mb_pos >= s->mb_stride)
		q2 = s->cur_pic.qscale_table[mb_pos - s->mb_stride];
	if (dc_pred_dir && n == 1)
		q2 = q1;
	if (!dc_pred_dir && n == 2)
		q2 = q1;
	if (n == 3)
		q2 = q1;

	if (coded) {
		int last = 0, skip, value;
		int k;

		for (int i = 1; !last; ++i) {
			int ret = vc1_decode_ac_coeff(v, &last, &skip, &value, codingset);
			if (ret < 0)
				return ret;
			i += skip;
			if (i > 63)
				break;
			if (v->fcm == PROGRESSIVE)
				block[v->zz_8x8[0][i]] = value;
			else {
				if (use_pred && (v->fcm == ILACE_FRAME)) {
					if (!dc_pred_dir) // top
						block[v->zz_8x8[2][i]] = value;
					else // left
						block[v->zz_8x8[3][i]] = value;
				} else {
					block[v->zzi_8x8[i]] = value;
				}
			}
		}

		/* apply AC prediction if needed */
		if (use_pred) {
			/* scale predictors if needed*/
			q1 = ABS(q1) * 2 + ((q1 < 0) ? 0 : v->halfpq) - 1;
			if (q1 < 1)
				return (-1);
			if (q2)
				q2 = ABS(q2) * 2 + ((q2 < 0) ? 0 : v->halfpq) - 1;
			if (q2 && q1 != q2) {
				if (dc_pred_dir) { // left
					for (k = 1; k < 8; k++)
						block[k << v->left_blk_sh] +=
							(int)(ac_val[k] * q2 * (unsigned)ff_vc1_dqscale[q1 - 1] + 0x20000) >> 18;
				} else { // top
					for (k = 1; k < 8; k++)
						block[k << v->top_blk_sh] +=
							(int)(ac_val[k + 8] * q2 * (unsigned)ff_vc1_dqscale[q1 - 1] + 0x20000) >> 18;
				}
			} else {
				if (dc_pred_dir) { // left
					for (k = 1; k < 8; k++)
						block[k << v->left_blk_sh] += ac_val[k];
				} else { // top
					for (k = 1; k < 8; k++)
						block[k << v->top_blk_sh] += ac_val[k + 8];
				}
			}
		}
		/* save AC coeffs for further prediction */
		for (k = 1; k < 8; k++) {
			ac_val2[k] = block[k << v->left_blk_sh];
			ac_val2[k + 8] = block[k << v->top_blk_sh];
		}

		/* scale AC coeffs */
		for (k = 1; k < 64; k++)
			if (block[k]) {
				block[k] *= scale;
				if (!v->pquantizer)
					block[k] += (block[k] < 0) ? -quant : quant;
			}
	} else { // no AC coeffs
		int k;

		memset(ac_val2, 0, 16 * 2);
		if (dc_pred_dir) { // left
			if (use_pred) {
				memcpy(ac_val2, ac_val, 8 * 2);
				q1 = ABS(q1) * 2 + ((q1 < 0) ? 0 : v->halfpq) - 1;
				if (q1 < 1)
					return (-1);
				if (q2)
					q2 = ABS(q2) * 2 + ((q2 < 0) ? 0 : v->halfpq) - 1;
				if (q2 && q1 != q2) {
					for (k = 1; k < 8; k++)
						ac_val2[k] = (int)(ac_val2[k] * (unsigned)q2 * ff_vc1_dqscale[q1 - 1] + 0x20000) >> 18;
				}
			}
		} else { // top
			if (use_pred) {
				memcpy(ac_val2 + 8, ac_val + 8, 8 * 2);
				q1 = ABS(q1) * 2 + ((q1 < 0) ? 0 : v->halfpq) - 1;
				if (q1 < 1)
					return (-1);
				if (q2)
					q2 = ABS(q2) * 2 + ((q2 < 0) ? 0 : v->halfpq) - 1;
				if (q2 && q1 != q2) {
					for (k = 1; k < 8; k++)
						ac_val2[k + 8] = (int)(ac_val2[k + 8] * (unsigned)q2 * ff_vc1_dqscale[q1 - 1] + 0x20000) >> 18;
				}
			}
		}

		/* apply AC prediction if needed */
		if (use_pred) {
			if (dc_pred_dir) { // left
				for (k = 1; k < 8; k++) {
					block[k << v->left_blk_sh] = ac_val2[k] * scale;
					if (!v->pquantizer && block[k << v->left_blk_sh])
						block[k << v->left_blk_sh] += (block[k << v->left_blk_sh] < 0) ? -quant : quant;
				}
			} else { // top
				for (k = 1; k < 8; k++) {
					block[k << v->top_blk_sh] = ac_val2[k + 8] * scale;
					if (!v->pquantizer && block[k << v->top_blk_sh])
						block[k << v->top_blk_sh] += (block[k << v->top_blk_sh] < 0) ? -quant : quant;
				}
			}
		}
	}

	return 0;
}

/** Decode P block
 */
static int vc1_decode_p_block(VC1Context *v, int16 block[64], int n, int mquant, int ttmb, int first_block, uint8 *dst,
							  int linesize, int skip_block, int *ttmb_out) {
	PictureState *s = &v->s;
	Bits *const gb = &v->gb;
	int i, j;
	int subblkpat = 0;
	int scale, off, idx, last, skip, value;
	int ttblk = ttmb & 7;
	int pat = 0;
	int quant = ABS(mquant);

	s->bdsp.clear_block(block);

	if (ttmb == -1) {
		ttblk = ff_vc1_ttblk_to_tt[v->tt_index][get_vlc2(gb, ff_vc1_ttblk_vlc[v->tt_index])];
	}
	if (ttblk == TT_4X4) {
		subblkpat = ~(get_vlc2(gb, ff_vc1_subblkpat_vlc[v->tt_index]) + 1);
	}
	if ((ttblk != TT_8X8 && ttblk != TT_4X4) &&
		((v->ttmbf || (ttmb != -1 && (ttmb & 8) && !first_block)) || (!v->res_rtm_flag && !first_block))) {
		subblkpat = decode012(gb);
		if (subblkpat)
			subblkpat ^= 3; // swap decoded pattern bits
		if (ttblk == TT_8X4_TOP || ttblk == TT_8X4_BOTTOM)
			ttblk = TT_8X4;
		if (ttblk == TT_4X8_RIGHT || ttblk == TT_4X8_LEFT)
			ttblk = TT_4X8;
	}
	scale = quant * 2 + ((mquant < 0) ? 0 : v->halfpq);

	// convert transforms like 8X4_TOP to generic TT and SUBBLKPAT
	if (ttblk == TT_8X4_TOP || ttblk == TT_8X4_BOTTOM) {
		subblkpat = 2 - (ttblk == TT_8X4_TOP);
		ttblk = TT_8X4;
	}
	if (ttblk == TT_4X8_RIGHT || ttblk == TT_4X8_LEFT) {
		subblkpat = 2 - (ttblk == TT_4X8_LEFT);
		ttblk = TT_4X8;
	}
	switch (ttblk) {
	case TT_8X8:
		pat = 0xF;
		i = 0;
		last = 0;
		while (!last) {
			int ret = vc1_decode_ac_coeff(v, &last, &skip, &value, v->codingset2);
			if (ret < 0)
				return ret;
			i += skip;
			if (i > 63)
				break;
			if (!v->fcm)
				idx = v->zz_8x8[0][i++];
			else
				idx = v->zzi_8x8[i++];
			block[idx] = value * scale;
			if (!v->pquantizer)
				block[idx] += (block[idx] < 0) ? -quant : quant;
		}
		if (!skip_block) {
			if (i == 1)
				v->vc1dsp.vc1_inv_trans_8x8_dc(dst, linesize, block);
			else {
				v->vc1dsp.vc1_inv_trans_8x8(block);
				s->idsp.add_pixels_clamped(block, dst, linesize);
			}
		}
		break;
	case TT_4X4:
		pat = ~subblkpat & 0xF;
		for (j = 0; j < 4; j++) {
			last = subblkpat & (1 << (3 - j));
			i = 0;
			off = (j & 1) * 4 + (j & 2) * 16;
			while (!last) {
				int ret = vc1_decode_ac_coeff(v, &last, &skip, &value, v->codingset2);
				if (ret < 0)
					return ret;
				i += skip;
				if (i > 15)
					break;
				if (!v->fcm)
					idx = ff_vc1_simple_progressive_4x4_zz[i++];
				else
					idx = ff_vc1_adv_interlaced_4x4_zz[i++];
				block[idx + off] = value * scale;
				if (!v->pquantizer)
					block[idx + off] += (block[idx + off] < 0) ? -quant : quant;
			}
			if (!(subblkpat & (1 << (3 - j))) && !skip_block) {
				if (i == 1)
					v->vc1dsp.vc1_inv_trans_4x4_dc(dst + (j & 1) * 4 + (j & 2) * 2 * linesize, linesize, block + off);
				else
					v->vc1dsp.vc1_inv_trans_4x4(dst + (j & 1) * 4 + (j & 2) * 2 * linesize, linesize, block + off);
			}
		}
		break;
	case TT_8X4:
		pat = ~((subblkpat & 2) * 6 + (subblkpat & 1) * 3) & 0xF;
		for (j = 0; j < 2; j++) {
			last = subblkpat & (1 << (1 - j));
			i = 0;
			off = j * 32;
			while (!last) {
				int ret = vc1_decode_ac_coeff(v, &last, &skip, &value, v->codingset2);
				if (ret < 0)
					return ret;
				i += skip;
				if (i > 31)
					break;
				if (!v->fcm)
					idx = v->zz_8x4[i++] + off;
				else
					idx = ff_vc1_adv_interlaced_8x4_zz[i++] + off;
				block[idx] = value * scale;
				if (!v->pquantizer)
					block[idx] += (block[idx] < 0) ? -quant : quant;
			}
			if (!(subblkpat & (1 << (1 - j))) && !skip_block) {
				if (i == 1)
					v->vc1dsp.vc1_inv_trans_8x4_dc(dst + j * 4 * linesize, linesize, block + off);
				else
					v->vc1dsp.vc1_inv_trans_8x4(dst + j * 4 * linesize, linesize, block + off);
			}
		}
		break;
	case TT_4X8:
		pat = ~(subblkpat * 5) & 0xF;
		for (j = 0; j < 2; j++) {
			last = subblkpat & (1 << (1 - j));
			i = 0;
			off = j * 4;
			while (!last) {
				int ret = vc1_decode_ac_coeff(v, &last, &skip, &value, v->codingset2);
				if (ret < 0)
					return ret;
				i += skip;
				if (i > 31)
					break;
				if (!v->fcm)
					idx = v->zz_4x8[i++] + off;
				else
					idx = ff_vc1_adv_interlaced_4x8_zz[i++] + off;
				block[idx] = value * scale;
				if (!v->pquantizer)
					block[idx] += (block[idx] < 0) ? -quant : quant;
			}
			if (!(subblkpat & (1 << (1 - j))) && !skip_block) {
				if (i == 1)
					v->vc1dsp.vc1_inv_trans_4x8_dc(dst + j * 4, linesize, block + off);
				else
					v->vc1dsp.vc1_inv_trans_4x8(dst + j * 4, linesize, block + off);
			}
		}
		break;
	}
	if (ttmb_out)
		*ttmb_out |= ttblk << (n * 4);
	return pat;
}

/** @} */ // Macroblock group

static const uint8 size_table[6] = {0, 2, 3, 4, 5, 8};

/** Decode one P-frame MB
 */
static int vc1_decode_p_mb(VC1Context *v) {
	PictureState *s = &v->s;
	Bits *const gb = &v->gb;
	int i, j;
	int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
	int cbp;             /* cbp decoding stuff */
	int mqdiff, mquant;  /* MB quantization */
	int ttmb = v->ttfrm; /* MB Transform type */

	int mb_has_coeffs = 1; /* last_flag */
	int dmv_x, dmv_y;      /* Differential MV components */
	int index, index1;     /* LUT indexes */
	int val, sign;         /* temp values */
	int first_block = 1;
	int dst_idx, off;
	int skipped, fourmv;
	int block_cbp = 0, pat, block_tt = 0, block_intra = 0;
	int ret;

	mquant = v->pq; /* lossy initialization */

	if (v->mv_type_is_raw)
		fourmv = get_bits1(gb);
	else
		fourmv = v->mv_type_mb_plane[mb_pos];
	if (v->skip_is_raw)
		skipped = get_bits1(gb);
	else
		skipped = v->s.mbskip_table[mb_pos];

	if (!fourmv) { /* 1MV mode */
		if (!skipped) {
			GET_MVDATA(dmv_x, dmv_y);

			if (s->mb_intra) {
				s->cur_pic.motion_val[1][s->block_index[0]][0] = 0;
				s->cur_pic.motion_val[1][s->block_index[0]][1] = 0;
			}
			s->cur_pic.mb_type[mb_pos] = s->mb_intra ? MB_TYPE_INTRA : MB_TYPE_16x16;
			ff_vc1_pred_mv(v, 0, dmv_x, dmv_y, 1, v->range_x, v->range_y, v->mb_type, 0, 0);

			/* FIXME Set DC val for inter block ? */
			if (s->mb_intra && !mb_has_coeffs) {
				GET_MQUANT();
				s->ac_pred = get_bits1(gb);
				cbp = 0;
			} else if (mb_has_coeffs) {
				if (s->mb_intra)
					s->ac_pred = get_bits1(gb);
				cbp = get_vlc2(gb, v->cbpcy_vlc);
				GET_MQUANT();
			} else {
				mquant = v->pq;
				cbp = 0;
			}
			s->cur_pic.qscale_table[mb_pos] = mquant;

			if (!v->ttmbf && !s->mb_intra && mb_has_coeffs)
				ttmb = get_vlc2(gb, ff_vc1_ttmb_vlc[v->tt_index]);
			if (!s->mb_intra)
				ff_vc1_mc_1mv(v, 0);
			dst_idx = 0;
			for (i = 0; i < 6; i++) {
				s->dc_val[s->block_index[i]] = 0;
				dst_idx += i >> 2;
				val = ((cbp >> (5 - i)) & 1);
				off = (i & 4) ? 0 : ((i & 1) * 8 + (i & 2) * 4 * s->linesize);
				v->mb_type[s->block_index[i]] = s->mb_intra;
				if (s->mb_intra) {
					/* check if prediction blocks A and C are available */
					v->a_avail = v->c_avail = 0;
					if (i == 2 || i == 3 || !s->first_slice_line)
						v->a_avail = v->mb_type[s->block_index[i] - s->block_wrap[i]];
					if (i == 1 || i == 3 || s->mb_x)
						v->c_avail = v->mb_type[s->block_index[i] - 1];

					ret = vc1_decode_intra_block(v, v->block[v->cur_blk_idx][block_map[i]], i, val, mquant,
												 (i & 4) ? v->codingset2 : v->codingset);
					if (ret < 0)
						return ret;
					if (CONFIG_GRAY && (i > 3) && (s->avctx->flags & AV_CODEC_FLAG_GRAY))
						continue;
					v->vc1dsp.vc1_inv_trans_8x8(v->block[v->cur_blk_idx][block_map[i]]);
					if (v->rangeredfrm)
						for (j = 0; j < 64; j++)
							v->block[v->cur_blk_idx][block_map[i]][j] *= 2;
					block_cbp |= 0xF << (i << 2);
					block_intra |= 1 << i;
				} else if (val) {
					pat =
						vc1_decode_p_block(v, v->block[v->cur_blk_idx][block_map[i]], i, mquant, ttmb, first_block,
										   s->dest[dst_idx] + off, (i & 4) ? s->uvlinesize : s->linesize,
										   CONFIG_GRAY && (i & 4) && (s->avctx->flags & AV_CODEC_FLAG_GRAY), &block_tt);
					if (pat < 0)
						return pat;
					block_cbp |= pat << (i << 2);
					if (!v->ttmbf && ttmb < 8)
						ttmb = -1;
					first_block = 0;
				}
			}
		} else { // skipped
			s->mb_intra = 0;
			for (i = 0; i < 6; i++) {
				v->mb_type[s->block_index[i]] = 0;
				s->dc_val[s->block_index[i]] = 0;
			}
			s->cur_pic.mb_type[mb_pos] = MB_TYPE_SKIP;
			s->cur_pic.qscale_table[mb_pos] = 0;
			ff_vc1_pred_mv(v, 0, 0, 0, 1, v->range_x, v->range_y, v->mb_type, 0, 0);
			ff_vc1_mc_1mv(v, 0);
		}
	} else { // 4MV mode
		if (!skipped /* unskipped MB */) {
			int intra_count = 0, coded_inter = 0;
			int is_intra[6], is_coded[6];
			/* Get CBPCY */
			cbp = get_vlc2(gb, v->cbpcy_vlc);
			for (i = 0; i < 6; i++) {
				val = ((cbp >> (5 - i)) & 1);
				s->dc_val[s->block_index[i]] = 0;
				s->mb_intra = 0;
				if (i < 4) {
					dmv_x = dmv_y = 0;
					s->mb_intra = 0;
					mb_has_coeffs = 0;
					if (val) {
						GET_MVDATA(dmv_x, dmv_y);
					}
					ff_vc1_pred_mv(v, i, dmv_x, dmv_y, 0, v->range_x, v->range_y, v->mb_type, 0, 0);
					if (!s->mb_intra)
						ff_vc1_mc_4mv_luma(v, i, 0, 0);
					intra_count += s->mb_intra;
					is_intra[i] = s->mb_intra;
					is_coded[i] = mb_has_coeffs;
				}
				if (i & 4) {
					is_intra[i] = (intra_count >= 3);
					is_coded[i] = val;
				}
				if (i == 4)
					ff_vc1_mc_4mv_chroma(v, 0);
				v->mb_type[s->block_index[i]] = is_intra[i];
				if (!coded_inter)
					coded_inter = (!is_intra[i]) & is_coded[i];
			}
			// if there are no coded blocks then don't do anything more
			dst_idx = 0;
			if (!intra_count && !coded_inter)
				goto end;
			GET_MQUANT();
			s->cur_pic.qscale_table[mb_pos] = mquant;
			/* test if block is intra and has pred */
			{
				int intrapred = 0;
				for (i = 0; i < 6; i++)
					if (is_intra[i]) {
						if (((!s->first_slice_line || (i == 2 || i == 3)) &&
							 v->mb_type[s->block_index[i] - s->block_wrap[i]]) ||
							((s->mb_x || (i == 1 || i == 3)) && v->mb_type[s->block_index[i] - 1])) {
							intrapred = 1;
							break;
						}
					}
				if (intrapred)
					s->ac_pred = get_bits1(gb);
				else
					s->ac_pred = 0;
			}
			if (!v->ttmbf && coded_inter)
				ttmb = get_vlc2(gb, ff_vc1_ttmb_vlc[v->tt_index]);
			for (i = 0; i < 6; i++) {
				dst_idx += i >> 2;
				off = (i & 4) ? 0 : ((i & 1) * 8 + (i & 2) * 4 * s->linesize);
				s->mb_intra = is_intra[i];
				if (is_intra[i]) {
					/* check if prediction blocks A and C are available */
					v->a_avail = v->c_avail = 0;
					if (i == 2 || i == 3 || !s->first_slice_line)
						v->a_avail = v->mb_type[s->block_index[i] - s->block_wrap[i]];
					if (i == 1 || i == 3 || s->mb_x)
						v->c_avail = v->mb_type[s->block_index[i] - 1];

					ret = vc1_decode_intra_block(v, v->block[v->cur_blk_idx][block_map[i]], i, is_coded[i], mquant,
												 (i & 4) ? v->codingset2 : v->codingset);
					if (ret < 0)
						return ret;
					if (CONFIG_GRAY && (i > 3) && (s->avctx->flags & AV_CODEC_FLAG_GRAY))
						continue;
					v->vc1dsp.vc1_inv_trans_8x8(v->block[v->cur_blk_idx][block_map[i]]);
					if (v->rangeredfrm)
						for (j = 0; j < 64; j++)
							v->block[v->cur_blk_idx][block_map[i]][j] *= 2;
					block_cbp |= 0xF << (i << 2);
					block_intra |= 1 << i;
				} else if (is_coded[i]) {
					pat =
						vc1_decode_p_block(v, v->block[v->cur_blk_idx][block_map[i]], i, mquant, ttmb, first_block,
										   s->dest[dst_idx] + off, (i & 4) ? s->uvlinesize : s->linesize,
										   CONFIG_GRAY && (i & 4) && (s->avctx->flags & AV_CODEC_FLAG_GRAY), &block_tt);
					if (pat < 0)
						return pat;
					block_cbp |= pat << (i << 2);
					if (!v->ttmbf && ttmb < 8)
						ttmb = -1;
					first_block = 0;
				}
			}
		} else { // skipped MB
			s->mb_intra = 0;
			s->cur_pic.qscale_table[mb_pos] = 0;
			for (i = 0; i < 6; i++) {
				v->mb_type[s->block_index[i]] = 0;
				s->dc_val[s->block_index[i]] = 0;
			}
			for (i = 0; i < 4; i++) {
				ff_vc1_pred_mv(v, i, 0, 0, 0, v->range_x, v->range_y, v->mb_type, 0, 0);
				ff_vc1_mc_4mv_luma(v, i, 0, 0);
			}
			ff_vc1_mc_4mv_chroma(v, 0);
			s->cur_pic.qscale_table[mb_pos] = 0;
		}
	}
end:
	if (v->overlap && v->pq >= 9)
		ff_vc1_p_overlap_filter(v);
	vc1_put_blocks_clamped(v, 1);

	v->cbp[s->mb_x] = block_cbp;
	v->ttblk[s->mb_x] = block_tt;
	v->is_intra[s->mb_x] = block_intra;

	return 0;
}

/* Decode one macroblock in an interlaced frame p picture */

static int vc1_decode_p_mb_intfr(VC1Context *v) {
	PictureState *s = &v->s;
	Bits *const gb = &v->gb;
	int i;
	int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
	int cbp = 0;         /* cbp decoding stuff */
	int mqdiff, mquant;  /* MB quantization */
	int ttmb = v->ttfrm; /* MB Transform type */

	int mb_has_coeffs = 1; /* last_flag */
	int dmv_x, dmv_y;      /* Differential MV components */
	int val;               /* temp value */
	int first_block = 1;
	int dst_idx, off;
	int skipped, fourmv = 0, twomv = 0;
	int block_cbp = 0, pat, block_tt = 0;
	int idx_mbmode = 0, mvbp;
	int fieldtx;
	int ret;

	mquant = v->pq; /* Lossy initialization */

	if (v->skip_is_raw)
		skipped = get_bits1(gb);
	else
		skipped = v->s.mbskip_table[mb_pos];
	if (!skipped) {
		if (v->fourmvswitch)
			idx_mbmode = get_vlc2(gb, v->mbmode_vlc); // try getting this done
		else
			idx_mbmode = get_vlc2(gb, v->mbmode_vlc); // in a single line
		switch (ff_vc1_mbmode_intfrp[v->fourmvswitch][idx_mbmode][0]) {
		/* store the motion vector type in a flag (useful later) */
		case MV_PMODE_INTFR_4MV:
			fourmv = 1;
			v->blk_mv_type[s->block_index[0]] = 0;
			v->blk_mv_type[s->block_index[1]] = 0;
			v->blk_mv_type[s->block_index[2]] = 0;
			v->blk_mv_type[s->block_index[3]] = 0;
			break;
		case MV_PMODE_INTFR_4MV_FIELD:
			fourmv = 1;
			v->blk_mv_type[s->block_index[0]] = 1;
			v->blk_mv_type[s->block_index[1]] = 1;
			v->blk_mv_type[s->block_index[2]] = 1;
			v->blk_mv_type[s->block_index[3]] = 1;
			break;
		case MV_PMODE_INTFR_2MV_FIELD:
			twomv = 1;
			v->blk_mv_type[s->block_index[0]] = 1;
			v->blk_mv_type[s->block_index[1]] = 1;
			v->blk_mv_type[s->block_index[2]] = 1;
			v->blk_mv_type[s->block_index[3]] = 1;
			break;
		case MV_PMODE_INTFR_1MV:
			v->blk_mv_type[s->block_index[0]] = 0;
			v->blk_mv_type[s->block_index[1]] = 0;
			v->blk_mv_type[s->block_index[2]] = 0;
			v->blk_mv_type[s->block_index[3]] = 0;
			break;
		}
		if (ff_vc1_mbmode_intfrp[v->fourmvswitch][idx_mbmode][0] == MV_PMODE_INTFR_INTRA) { // intra MB
			for (i = 0; i < 4; i++) {
				s->cur_pic.motion_val[1][s->block_index[i]][0] = 0;
				s->cur_pic.motion_val[1][s->block_index[i]][1] = 0;
			}
			v->is_intra[s->mb_x] = 0x3f; // Set the bitfield to all 1.
			s->mb_intra = 1;
			s->cur_pic.mb_type[mb_pos] = MB_TYPE_INTRA;
			fieldtx = v->fieldtx_plane[mb_pos] = get_bits1(gb);
			mb_has_coeffs = get_bits1(gb);
			if (mb_has_coeffs)
				cbp = 1 + get_vlc2(gb, v->cbpcy_vlc);
			v->s.ac_pred = v->acpred_plane[mb_pos] = get_bits1(gb);
			GET_MQUANT();
			s->cur_pic.qscale_table[mb_pos] = mquant;
			/* Set DC scale - y and c use the same so we only set y */
			s->y_dc_scale = ff_wmv3_dc_scale_table[ABS(mquant)];
			dst_idx = 0;
			for (i = 0; i < 6; i++) {
				v->a_avail = v->c_avail = 0;
				v->mb_type[s->block_index[i]] = 1;
				s->dc_val[s->block_index[i]] = 0;
				dst_idx += i >> 2;
				val = ((cbp >> (5 - i)) & 1);
				if (i == 2 || i == 3 || !s->first_slice_line)
					v->a_avail = v->mb_type[s->block_index[i] - s->block_wrap[i]];
				if (i == 1 || i == 3 || s->mb_x)
					v->c_avail = v->mb_type[s->block_index[i] - 1];

				ret = vc1_decode_intra_block(v, v->block[v->cur_blk_idx][block_map[i]], i, val, mquant,
											 (i & 4) ? v->codingset2 : v->codingset);
				if (ret < 0)
					return ret;
				if (CONFIG_GRAY && (i > 3) && (s->avctx->flags & AV_CODEC_FLAG_GRAY))
					continue;
				v->vc1dsp.vc1_inv_trans_8x8(v->block[v->cur_blk_idx][block_map[i]]);
				block_cbp |= 0xf << (i << 2);
			}

		} else { // inter MB
			mb_has_coeffs = ff_vc1_mbmode_intfrp[v->fourmvswitch][idx_mbmode][3];
			if (mb_has_coeffs)
				cbp = 1 + get_vlc2(gb, v->cbpcy_vlc);
			if (ff_vc1_mbmode_intfrp[v->fourmvswitch][idx_mbmode][0] == MV_PMODE_INTFR_2MV_FIELD) {
				v->twomvbp = get_vlc2(gb, v->twomvbp_vlc);
			} else {
				if ((ff_vc1_mbmode_intfrp[v->fourmvswitch][idx_mbmode][0] == MV_PMODE_INTFR_4MV) ||
					(ff_vc1_mbmode_intfrp[v->fourmvswitch][idx_mbmode][0] == MV_PMODE_INTFR_4MV_FIELD)) {
					v->fourmvbp = get_vlc2(gb, v->fourmvbp_vlc);
				}
			}
			s->mb_intra = v->is_intra[s->mb_x] = 0;
			for (i = 0; i < 6; i++)
				v->mb_type[s->block_index[i]] = 0;
			fieldtx = v->fieldtx_plane[mb_pos] = ff_vc1_mbmode_intfrp[v->fourmvswitch][idx_mbmode][1];
			/* for all motion vector read MVDATA and motion compensate each block */
			dst_idx = 0;
			if (fourmv) {
				mvbp = v->fourmvbp;
				for (i = 0; i < 4; i++) {
					dmv_x = dmv_y = 0;
					if (mvbp & (8 >> i))
						get_mvdata_interlaced(v, &dmv_x, &dmv_y, 0);
					ff_vc1_pred_mv_intfr(v, i, dmv_x, dmv_y, 0, v->range_x, v->range_y, 0);
					ff_vc1_mc_4mv_luma(v, i, 0, 0);
				}
				ff_vc1_mc_4mv_chroma4(v, 0, 0, 0);
			} else if (twomv) {
				mvbp = v->twomvbp;
				dmv_x = dmv_y = 0;
				if (mvbp & 2) {
					get_mvdata_interlaced(v, &dmv_x, &dmv_y, 0);
				}
				ff_vc1_pred_mv_intfr(v, 0, dmv_x, dmv_y, 2, v->range_x, v->range_y, 0);
				ff_vc1_mc_4mv_luma(v, 0, 0, 0);
				ff_vc1_mc_4mv_luma(v, 1, 0, 0);
				dmv_x = dmv_y = 0;
				if (mvbp & 1) {
					get_mvdata_interlaced(v, &dmv_x, &dmv_y, 0);
				}
				ff_vc1_pred_mv_intfr(v, 2, dmv_x, dmv_y, 2, v->range_x, v->range_y, 0);
				ff_vc1_mc_4mv_luma(v, 2, 0, 0);
				ff_vc1_mc_4mv_luma(v, 3, 0, 0);
				ff_vc1_mc_4mv_chroma4(v, 0, 0, 0);
			} else {
				mvbp = ff_vc1_mbmode_intfrp[v->fourmvswitch][idx_mbmode][2];
				dmv_x = dmv_y = 0;
				if (mvbp) {
					get_mvdata_interlaced(v, &dmv_x, &dmv_y, 0);
				}
				ff_vc1_pred_mv_intfr(v, 0, dmv_x, dmv_y, 1, v->range_x, v->range_y, 0);
				ff_vc1_mc_1mv(v, 0);
			}
			if (cbp)
				GET_MQUANT(); // p. 227
			s->cur_pic.qscale_table[mb_pos] = mquant;
			if (!v->ttmbf && cbp)
				ttmb = get_vlc2(gb, ff_vc1_ttmb_vlc[v->tt_index]);
			for (i = 0; i < 6; i++) {
				s->dc_val[s->block_index[i]] = 0;
				dst_idx += i >> 2;
				val = ((cbp >> (5 - i)) & 1);
				if (!fieldtx)
					off = (i & 4) ? 0 : ((i & 1) * 8 + (i & 2) * 4 * s->linesize);
				else
					off = (i & 4) ? 0 : ((i & 1) * 8 + ((i > 1) * s->linesize));
				if (val) {
					pat =
						vc1_decode_p_block(v, v->block[v->cur_blk_idx][block_map[i]], i, mquant, ttmb, first_block,
										   s->dest[dst_idx] + off, (i & 4) ? s->uvlinesize : (s->linesize << fieldtx),
										   CONFIG_GRAY && (i & 4) && (s->avctx->flags & AV_CODEC_FLAG_GRAY), &block_tt);
					if (pat < 0)
						return pat;
					block_cbp |= pat << (i << 2);
					if (!v->ttmbf && ttmb < 8)
						ttmb = -1;
					first_block = 0;
				}
			}
		}
	} else { // skipped
		s->mb_intra = v->is_intra[s->mb_x] = 0;
		for (i = 0; i < 6; i++) {
			v->mb_type[s->block_index[i]] = 0;
			s->dc_val[s->block_index[i]] = 0;
		}
		s->cur_pic.mb_type[mb_pos] = MB_TYPE_SKIP;
		s->cur_pic.qscale_table[mb_pos] = 0;
		v->blk_mv_type[s->block_index[0]] = 0;
		v->blk_mv_type[s->block_index[1]] = 0;
		v->blk_mv_type[s->block_index[2]] = 0;
		v->blk_mv_type[s->block_index[3]] = 0;
		ff_vc1_pred_mv_intfr(v, 0, 0, 0, 1, v->range_x, v->range_y, 0);
		ff_vc1_mc_1mv(v, 0);
		v->fieldtx_plane[mb_pos] = 0;
	}
	if (v->overlap && v->pq >= 9)
		ff_vc1_p_overlap_filter(v);
	vc1_put_blocks_clamped(v, 1);

	v->cbp[s->mb_x] = block_cbp;
	v->ttblk[s->mb_x] = block_tt;

	return 0;
}

static int vc1_decode_p_mb_intfi(VC1Context *v) {
	PictureState *s = &v->s;
	Bits *const gb = &v->gb;
	int i;
	int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
	int cbp = 0;         /* cbp decoding stuff */
	int mqdiff, mquant;  /* MB quantization */
	int ttmb = v->ttfrm; /* MB Transform type */

	int mb_has_coeffs = 1; /* last_flag */
	int dmv_x, dmv_y;      /* Differential MV components */
	int val;               /* temp values */
	int first_block = 1;
	int dst_idx, off;
	int pred_flag = 0;
	int block_cbp = 0, pat, block_tt = 0;
	int idx_mbmode = 0;
	int ret;

	mquant = v->pq; /* Lossy initialization */

	idx_mbmode = get_vlc2(gb, v->mbmode_vlc);
	if (idx_mbmode <= 1) {           // intra MB
		v->is_intra[s->mb_x] = 0x3f; // Set the bitfield to all 1.
		s->mb_intra = 1;
		s->cur_pic.motion_val[1][s->block_index[0] + v->blocks_off][0] = 0;
		s->cur_pic.motion_val[1][s->block_index[0] + v->blocks_off][1] = 0;
		s->cur_pic.mb_type[mb_pos + v->mb_off] = MB_TYPE_INTRA;
		GET_MQUANT();
		s->cur_pic.qscale_table[mb_pos] = mquant;
		/* Set DC scale - y and c use the same so we only set y */
		s->y_dc_scale = ff_wmv3_dc_scale_table[ABS(mquant)];
		v->s.ac_pred = v->acpred_plane[mb_pos] = get_bits1(gb);
		mb_has_coeffs = idx_mbmode & 1;
		if (mb_has_coeffs)
			cbp = 1 + get_vlc2(gb, v->cbpcy_vlc);
		dst_idx = 0;
		for (i = 0; i < 6; i++) {
			v->a_avail = v->c_avail = 0;
			v->mb_type[s->block_index[i]] = 1;
			s->dc_val[s->block_index[i]] = 0;
			dst_idx += i >> 2;
			val = ((cbp >> (5 - i)) & 1);
			if (i == 2 || i == 3 || !s->first_slice_line)
				v->a_avail = v->mb_type[s->block_index[i] - s->block_wrap[i]];
			if (i == 1 || i == 3 || s->mb_x)
				v->c_avail = v->mb_type[s->block_index[i] - 1];

			ret = vc1_decode_intra_block(v, v->block[v->cur_blk_idx][block_map[i]], i, val, mquant,
										 (i & 4) ? v->codingset2 : v->codingset);
			if (ret < 0)
				return ret;
			if (CONFIG_GRAY && (i > 3) && (s->avctx->flags & AV_CODEC_FLAG_GRAY))
				continue;
			v->vc1dsp.vc1_inv_trans_8x8(v->block[v->cur_blk_idx][block_map[i]]);
			block_cbp |= 0xf << (i << 2);
		}
	} else {
		s->mb_intra = v->is_intra[s->mb_x] = 0;
		s->cur_pic.mb_type[mb_pos + v->mb_off] = MB_TYPE_16x16;
		for (i = 0; i < 6; i++)
			v->mb_type[s->block_index[i]] = 0;
		if (idx_mbmode <= 5) { // 1-MV
			dmv_x = dmv_y = pred_flag = 0;
			if (idx_mbmode & 1) {
				get_mvdata_interlaced(v, &dmv_x, &dmv_y, &pred_flag);
			}
			ff_vc1_pred_mv(v, 0, dmv_x, dmv_y, 1, v->range_x, v->range_y, v->mb_type, pred_flag, 0);
			ff_vc1_mc_1mv(v, 0);
			mb_has_coeffs = !(idx_mbmode & 2);
		} else { // 4-MV
			v->fourmvbp = get_vlc2(gb, v->fourmvbp_vlc);
			for (i = 0; i < 4; i++) {
				dmv_x = dmv_y = pred_flag = 0;
				if (v->fourmvbp & (8 >> i))
					get_mvdata_interlaced(v, &dmv_x, &dmv_y, &pred_flag);
				ff_vc1_pred_mv(v, i, dmv_x, dmv_y, 0, v->range_x, v->range_y, v->mb_type, pred_flag, 0);
				ff_vc1_mc_4mv_luma(v, i, 0, 0);
			}
			ff_vc1_mc_4mv_chroma(v, 0);
			mb_has_coeffs = idx_mbmode & 1;
		}
		if (mb_has_coeffs)
			cbp = 1 + get_vlc2(gb, v->cbpcy_vlc);
		if (cbp) {
			GET_MQUANT();
		}
		s->cur_pic.qscale_table[mb_pos] = mquant;
		if (!v->ttmbf && cbp) {
			ttmb = get_vlc2(gb, ff_vc1_ttmb_vlc[v->tt_index]);
		}
		dst_idx = 0;
		for (i = 0; i < 6; i++) {
			s->dc_val[s->block_index[i]] = 0;
			dst_idx += i >> 2;
			val = ((cbp >> (5 - i)) & 1);
			off = (i & 4) ? 0 : (i & 1) * 8 + (i & 2) * 4 * s->linesize;
			if (val) {
				pat = vc1_decode_p_block(v, v->block[v->cur_blk_idx][block_map[i]], i, mquant, ttmb, first_block,
										 s->dest[dst_idx] + off, (i & 4) ? s->uvlinesize : s->linesize,
										 CONFIG_GRAY && (i & 4) && (s->avctx->flags & AV_CODEC_FLAG_GRAY), &block_tt);
				if (pat < 0)
					return pat;
				block_cbp |= pat << (i << 2);
				if (!v->ttmbf && ttmb < 8)
					ttmb = -1;
				first_block = 0;
			}
		}
	}
	if (v->overlap && v->pq >= 9)
		ff_vc1_p_overlap_filter(v);
	vc1_put_blocks_clamped(v, 1);

	v->cbp[s->mb_x] = block_cbp;
	v->ttblk[s->mb_x] = block_tt;

	return 0;
}

/** Decode one B-frame MB (in Main profile)
 */
static int vc1_decode_b_mb(VC1Context *v) {
	PictureState *s = &v->s;
	Bits *const gb = &v->gb;
	int i, j;
	int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
	int cbp = 0;           /* cbp decoding stuff */
	int mqdiff, mquant;    /* MB quantization */
	int ttmb = v->ttfrm;   /* MB Transform type */
	int mb_has_coeffs = 0; /* last_flag */
	int index, index1;     /* LUT indexes */
	int val, sign;         /* temp values */
	int first_block = 1;
	int dst_idx, off;
	int skipped, direct;
	int dmv_x[2], dmv_y[2];
	int bmvtype = BMV_TYPE_BACKWARD;
	int ret;

	mquant = v->pq; /* lossy initialization */
	s->mb_intra = 0;

	if (v->dmb_is_raw)
		direct = get_bits1(gb);
	else
		direct = v->direct_mb_plane[mb_pos];
	if (v->skip_is_raw)
		skipped = get_bits1(gb);
	else
		skipped = v->s.mbskip_table[mb_pos];

	dmv_x[0] = dmv_x[1] = dmv_y[0] = dmv_y[1] = 0;
	for (i = 0; i < 6; i++) {
		v->mb_type[s->block_index[i]] = 0;
		s->dc_val[s->block_index[i]] = 0;
	}
	s->cur_pic.qscale_table[mb_pos] = 0;

	if (!direct) {
		if (!skipped) {
			GET_MVDATA(dmv_x[0], dmv_y[0]);
			dmv_x[1] = dmv_x[0];
			dmv_y[1] = dmv_y[0];
		}
		if (skipped || !s->mb_intra) {
			bmvtype = decode012(gb);
			switch (bmvtype) {
			case 0:
				bmvtype = (v->bfraction >= (B_FRACTION_DEN / 2)) ? BMV_TYPE_BACKWARD : BMV_TYPE_FORWARD;
				break;
			case 1:
				bmvtype = (v->bfraction >= (B_FRACTION_DEN / 2)) ? BMV_TYPE_FORWARD : BMV_TYPE_BACKWARD;
				break;
			case 2:
				bmvtype = BMV_TYPE_INTERPOLATED;
				dmv_x[0] = dmv_y[0] = 0;
			}
		}
	}
	for (i = 0; i < 6; i++)
		v->mb_type[s->block_index[i]] = s->mb_intra;

	if (skipped) {
		if (direct)
			bmvtype = BMV_TYPE_INTERPOLATED;
		ff_vc1_pred_b_mv(v, dmv_x, dmv_y, direct, bmvtype);
		vc1_b_mc(v, dmv_x, dmv_y, direct, bmvtype);
		return 0;
	}
	if (direct) {
		cbp = get_vlc2(gb, v->cbpcy_vlc);
		GET_MQUANT();
		s->mb_intra = 0;
		s->cur_pic.qscale_table[mb_pos] = mquant;
		if (!v->ttmbf)
			ttmb = get_vlc2(gb, ff_vc1_ttmb_vlc[v->tt_index]);
		dmv_x[0] = dmv_y[0] = dmv_x[1] = dmv_y[1] = 0;
		ff_vc1_pred_b_mv(v, dmv_x, dmv_y, direct, bmvtype);
		vc1_b_mc(v, dmv_x, dmv_y, direct, bmvtype);
	} else {
		if (!mb_has_coeffs && !s->mb_intra) {
			/* no coded blocks - effectively skipped */
			ff_vc1_pred_b_mv(v, dmv_x, dmv_y, direct, bmvtype);
			vc1_b_mc(v, dmv_x, dmv_y, direct, bmvtype);
			return 0;
		}
		if (s->mb_intra && !mb_has_coeffs) {
			GET_MQUANT();
			s->cur_pic.qscale_table[mb_pos] = mquant;
			s->ac_pred = get_bits1(gb);
			cbp = 0;
			ff_vc1_pred_b_mv(v, dmv_x, dmv_y, direct, bmvtype);
		} else {
			if (bmvtype == BMV_TYPE_INTERPOLATED) {
				GET_MVDATA(dmv_x[0], dmv_y[0]);
				if (!mb_has_coeffs) {
					/* interpolated skipped block */
					ff_vc1_pred_b_mv(v, dmv_x, dmv_y, direct, bmvtype);
					vc1_b_mc(v, dmv_x, dmv_y, direct, bmvtype);
					return 0;
				}
			}
			ff_vc1_pred_b_mv(v, dmv_x, dmv_y, direct, bmvtype);
			if (!s->mb_intra) {
				vc1_b_mc(v, dmv_x, dmv_y, direct, bmvtype);
			}
			if (s->mb_intra)
				s->ac_pred = get_bits1(gb);
			cbp = get_vlc2(gb, v->cbpcy_vlc);
			GET_MQUANT();
			s->cur_pic.qscale_table[mb_pos] = mquant;
			if (!v->ttmbf && !s->mb_intra && mb_has_coeffs)
				ttmb = get_vlc2(gb, ff_vc1_ttmb_vlc[v->tt_index]);
		}
	}
	dst_idx = 0;
	for (i = 0; i < 6; i++) {
		s->dc_val[s->block_index[i]] = 0;
		dst_idx += i >> 2;
		val = ((cbp >> (5 - i)) & 1);
		off = (i & 4) ? 0 : ((i & 1) * 8 + (i & 2) * 4 * s->linesize);
		v->mb_type[s->block_index[i]] = s->mb_intra;
		if (s->mb_intra) {
			/* check if prediction blocks A and C are available */
			v->a_avail = v->c_avail = 0;
			if (i == 2 || i == 3 || !s->first_slice_line)
				v->a_avail = v->mb_type[s->block_index[i] - s->block_wrap[i]];
			if (i == 1 || i == 3 || s->mb_x)
				v->c_avail = v->mb_type[s->block_index[i] - 1];

			ret = vc1_decode_intra_block(v, v->blocks[i], i, val, mquant, (i & 4) ? v->codingset2 : v->codingset);
			if (ret < 0)
				return ret;
			if (CONFIG_GRAY && (i > 3) && (s->avctx->flags & AV_CODEC_FLAG_GRAY))
				continue;
			v->vc1dsp.vc1_inv_trans_8x8(v->blocks[i]);
			if (v->rangeredfrm)
				for (j = 0; j < 64; j++)
					v->blocks[i][j] *= 2;
			s->idsp.put_signed_pixels_clamped(v->blocks[i], s->dest[dst_idx] + off,
											  i & 4 ? s->uvlinesize : s->linesize);
		} else if (val) {
			int pat = vc1_decode_p_block(v, v->blocks[i], i, mquant, ttmb, first_block, s->dest[dst_idx] + off,
										 (i & 4) ? s->uvlinesize : s->linesize,
										 CONFIG_GRAY && (i & 4) && (s->avctx->flags & AV_CODEC_FLAG_GRAY), NULL);
			if (pat < 0)
				return pat;
			if (!v->ttmbf && ttmb < 8)
				ttmb = -1;
			first_block = 0;
		}
	}
	return 0;
}

/** Decode one B-frame MB (in interlaced field B picture)
 */
static int vc1_decode_b_mb_intfi(VC1Context *v) {
	PictureState *s = &v->s;
	Bits *const gb = &v->gb;
	int i, j;
	int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
	int cbp = 0;           /* cbp decoding stuff */
	int mqdiff, mquant;    /* MB quantization */
	int ttmb = v->ttfrm;   /* MB Transform type */
	int mb_has_coeffs = 0; /* last_flag */
	int val;               /* temp value */
	int first_block = 1;
	int dst_idx, off;
	int fwd;
	int dmv_x[2], dmv_y[2], pred_flag[2];
	int bmvtype = BMV_TYPE_BACKWARD;
	int block_cbp = 0, pat, block_tt = 0;
	int idx_mbmode;
	int ret;

	mquant = v->pq; /* Lossy initialization */
	s->mb_intra = 0;

	idx_mbmode = get_vlc2(gb, v->mbmode_vlc);
	if (idx_mbmode <= 1) {           // intra MB
		v->is_intra[s->mb_x] = 0x3f; // Set the bitfield to all 1.
		s->mb_intra = 1;
		s->cur_pic.motion_val[1][s->block_index[0]][0] = 0;
		s->cur_pic.motion_val[1][s->block_index[0]][1] = 0;
		s->cur_pic.mb_type[mb_pos + v->mb_off] = MB_TYPE_INTRA;
		GET_MQUANT();
		s->cur_pic.qscale_table[mb_pos] = mquant;
		/* Set DC scale - y and c use the same so we only set y */
		s->y_dc_scale = ff_wmv3_dc_scale_table[ABS(mquant)];
		v->s.ac_pred = v->acpred_plane[mb_pos] = get_bits1(gb);
		mb_has_coeffs = idx_mbmode & 1;
		if (mb_has_coeffs)
			cbp = 1 + get_vlc2(gb, v->cbpcy_vlc);
		dst_idx = 0;
		for (i = 0; i < 6; i++) {
			v->a_avail = v->c_avail = 0;
			v->mb_type[s->block_index[i]] = 1;
			s->dc_val[s->block_index[i]] = 0;
			dst_idx += i >> 2;
			val = ((cbp >> (5 - i)) & 1);
			if (i == 2 || i == 3 || !s->first_slice_line)
				v->a_avail = v->mb_type[s->block_index[i] - s->block_wrap[i]];
			if (i == 1 || i == 3 || s->mb_x)
				v->c_avail = v->mb_type[s->block_index[i] - 1];

			ret = vc1_decode_intra_block(v, v->blocks[i], i, val, mquant, (i & 4) ? v->codingset2 : v->codingset);
			if (ret < 0)
				return ret;
			if (CONFIG_GRAY && (i > 3) && (s->avctx->flags & AV_CODEC_FLAG_GRAY))
				continue;
			v->vc1dsp.vc1_inv_trans_8x8(v->blocks[i]);
			if (v->rangeredfrm)
				for (j = 0; j < 64; j++)
					v->blocks[i][j] <<= 1;
			off = (i & 4) ? 0 : ((i & 1) * 8 + (i & 2) * 4 * s->linesize);
			s->idsp.put_signed_pixels_clamped(v->blocks[i], s->dest[dst_idx] + off,
											  (i & 4) ? s->uvlinesize : s->linesize);
		}
	} else {
		s->mb_intra = v->is_intra[s->mb_x] = 0;
		s->cur_pic.mb_type[mb_pos + v->mb_off] = MB_TYPE_16x16;
		for (i = 0; i < 6; i++)
			v->mb_type[s->block_index[i]] = 0;
		if (v->fmb_is_raw)
			fwd = v->forward_mb_plane[mb_pos] = get_bits1(gb);
		else
			fwd = v->forward_mb_plane[mb_pos];
		if (idx_mbmode <= 5) { // 1-MV
			int interpmvp = 0;
			dmv_x[0] = dmv_x[1] = dmv_y[0] = dmv_y[1] = 0;
			pred_flag[0] = pred_flag[1] = 0;
			if (fwd)
				bmvtype = BMV_TYPE_FORWARD;
			else {
				bmvtype = decode012(gb);
				switch (bmvtype) {
				case 0:
					bmvtype = BMV_TYPE_BACKWARD;
					break;
				case 1:
					bmvtype = BMV_TYPE_DIRECT;
					break;
				case 2:
					bmvtype = BMV_TYPE_INTERPOLATED;
					interpmvp = get_bits1(gb);
				}
			}
			v->bmvtype = bmvtype;
			if (bmvtype != BMV_TYPE_DIRECT && idx_mbmode & 1) {
				get_mvdata_interlaced(v, &dmv_x[bmvtype == BMV_TYPE_BACKWARD], &dmv_y[bmvtype == BMV_TYPE_BACKWARD],
									  &pred_flag[bmvtype == BMV_TYPE_BACKWARD]);
			}
			if (interpmvp) {
				get_mvdata_interlaced(v, &dmv_x[1], &dmv_y[1], &pred_flag[1]);
			}
			if (bmvtype == BMV_TYPE_DIRECT) {
				dmv_x[0] = dmv_y[0] = pred_flag[0] = 0;
				dmv_x[1] = dmv_y[1] = pred_flag[0] = 0;
				if (!s->next_pic.ptr->field_picture) {
					warning("Mixed field/frame direct mode not supported\n");
					return (-1);
				}
			}
			ff_vc1_pred_b_mv_intfi(v, 0, dmv_x, dmv_y, 1, pred_flag);
			vc1_b_mc(v, dmv_x, dmv_y, (bmvtype == BMV_TYPE_DIRECT), bmvtype);
			mb_has_coeffs = !(idx_mbmode & 2);
		} else { // 4-MV
			if (fwd)
				bmvtype = BMV_TYPE_FORWARD;
			v->bmvtype = bmvtype;
			v->fourmvbp = get_vlc2(gb, v->fourmvbp_vlc);
			for (i = 0; i < 4; i++) {
				dmv_x[0] = dmv_y[0] = pred_flag[0] = 0;
				dmv_x[1] = dmv_y[1] = pred_flag[1] = 0;
				if (v->fourmvbp & (8 >> i)) {
					get_mvdata_interlaced(v, &dmv_x[bmvtype == BMV_TYPE_BACKWARD], &dmv_y[bmvtype == BMV_TYPE_BACKWARD],
										  &pred_flag[bmvtype == BMV_TYPE_BACKWARD]);
				}
				ff_vc1_pred_b_mv_intfi(v, i, dmv_x, dmv_y, 0, pred_flag);
				ff_vc1_mc_4mv_luma(v, i, bmvtype == BMV_TYPE_BACKWARD, 0);
			}
			ff_vc1_mc_4mv_chroma(v, bmvtype == BMV_TYPE_BACKWARD);
			mb_has_coeffs = idx_mbmode & 1;
		}
		if (mb_has_coeffs)
			cbp = 1 + get_vlc2(gb, v->cbpcy_vlc);
		if (cbp) {
			GET_MQUANT();
		}
		s->cur_pic.qscale_table[mb_pos] = mquant;
		if (!v->ttmbf && cbp) {
			ttmb = get_vlc2(gb, ff_vc1_ttmb_vlc[v->tt_index]);
		}
		dst_idx = 0;
		for (i = 0; i < 6; i++) {
			s->dc_val[s->block_index[i]] = 0;
			dst_idx += i >> 2;
			val = ((cbp >> (5 - i)) & 1);
			off = (i & 4) ? 0 : (i & 1) * 8 + (i & 2) * 4 * s->linesize;
			if (val) {
				pat = vc1_decode_p_block(v, v->blocks[i], i, mquant, ttmb, first_block, s->dest[dst_idx] + off,
										 (i & 4) ? s->uvlinesize : s->linesize,
										 CONFIG_GRAY && (i & 4) && (s->avctx->flags & AV_CODEC_FLAG_GRAY), &block_tt);
				if (pat < 0)
					return pat;
				block_cbp |= pat << (i << 2);
				if (!v->ttmbf && ttmb < 8)
					ttmb = -1;
				first_block = 0;
			}
		}
	}
	v->cbp[s->mb_x] = block_cbp;
	v->ttblk[s->mb_x] = block_tt;

	return 0;
}

/** Decode one B-frame MB (in interlaced frame B picture)
 */
static int vc1_decode_b_mb_intfr(VC1Context *v) {
	PictureState *s = &v->s;
	Bits *const gb = &v->gb;
	int i, j;
	int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
	int cbp = 0;           /* cbp decoding stuff */
	int mqdiff, mquant;    /* MB quantization */
	int ttmb = v->ttfrm;   /* MB Transform type */
	int mvsw = 0;          /* motion vector switch */
	int mb_has_coeffs = 1; /* last_flag */
	int dmv_x, dmv_y;      /* Differential MV components */
	int val;               /* temp value */
	int first_block = 1;
	int dst_idx, off;
	int skipped, direct, twomv = 0;
	int block_cbp = 0, pat, block_tt = 0;
	int idx_mbmode = 0, mvbp;
	int stride_y, fieldtx;
	int bmvtype = BMV_TYPE_BACKWARD;
	int dir, dir2;
	int ret;

	mquant = v->pq; /* Lossy initialization */
	s->mb_intra = 0;
	if (v->skip_is_raw)
		skipped = get_bits1(gb);
	else
		skipped = v->s.mbskip_table[mb_pos];

	if (!skipped) {
		idx_mbmode = get_vlc2(gb, v->mbmode_vlc);
		if (ff_vc1_mbmode_intfrp[0][idx_mbmode][0] == MV_PMODE_INTFR_2MV_FIELD) {
			twomv = 1;
			v->blk_mv_type[s->block_index[0]] = 1;
			v->blk_mv_type[s->block_index[1]] = 1;
			v->blk_mv_type[s->block_index[2]] = 1;
			v->blk_mv_type[s->block_index[3]] = 1;
		} else {
			v->blk_mv_type[s->block_index[0]] = 0;
			v->blk_mv_type[s->block_index[1]] = 0;
			v->blk_mv_type[s->block_index[2]] = 0;
			v->blk_mv_type[s->block_index[3]] = 0;
		}
	}

	if (ff_vc1_mbmode_intfrp[0][idx_mbmode][0] == MV_PMODE_INTFR_INTRA) { // intra MB
		for (i = 0; i < 4; i++) {
			s->mv[0][i][0] = s->cur_pic.motion_val[0][s->block_index[i]][0] = 0;
			s->mv[0][i][1] = s->cur_pic.motion_val[0][s->block_index[i]][1] = 0;
			s->mv[1][i][0] = s->cur_pic.motion_val[1][s->block_index[i]][0] = 0;
			s->mv[1][i][1] = s->cur_pic.motion_val[1][s->block_index[i]][1] = 0;
		}
		v->is_intra[s->mb_x] = 0x3f; // Set the bitfield to all 1.
		s->mb_intra = 1;
		s->cur_pic.mb_type[mb_pos] = MB_TYPE_INTRA;
		fieldtx = v->fieldtx_plane[mb_pos] = get_bits1(gb);
		mb_has_coeffs = get_bits1(gb);
		if (mb_has_coeffs)
			cbp = 1 + get_vlc2(gb, v->cbpcy_vlc);
		v->s.ac_pred = v->acpred_plane[mb_pos] = get_bits1(gb);
		GET_MQUANT();
		s->cur_pic.qscale_table[mb_pos] = mquant;
		/* Set DC scale - y and c use the same so we only set y */
		s->y_dc_scale = ff_wmv3_dc_scale_table[ABS(mquant)];
		dst_idx = 0;
		for (i = 0; i < 6; i++) {
			v->a_avail = v->c_avail = 0;
			v->mb_type[s->block_index[i]] = 1;
			s->dc_val[s->block_index[i]] = 0;
			dst_idx += i >> 2;
			val = ((cbp >> (5 - i)) & 1);
			if (i == 2 || i == 3 || !s->first_slice_line)
				v->a_avail = v->mb_type[s->block_index[i] - s->block_wrap[i]];
			if (i == 1 || i == 3 || s->mb_x)
				v->c_avail = v->mb_type[s->block_index[i] - 1];

			ret = vc1_decode_intra_block(v, v->blocks[i], i, val, mquant, (i & 4) ? v->codingset2 : v->codingset);
			if (ret < 0)
				return ret;
			if (CONFIG_GRAY && i > 3 && (s->avctx->flags & AV_CODEC_FLAG_GRAY))
				continue;
			v->vc1dsp.vc1_inv_trans_8x8(v->blocks[i]);
			if (i < 4) {
				stride_y = s->linesize << fieldtx;
				off =
					(fieldtx) ? ((i & 1) * 8) + ((i & 2) >> 1) * s->linesize : (i & 1) * 8 + 4 * (i & 2) * s->linesize;
			} else {
				stride_y = s->uvlinesize;
				off = 0;
			}
			s->idsp.put_signed_pixels_clamped(v->blocks[i], s->dest[dst_idx] + off, stride_y);
		}
	} else {
		s->mb_intra = v->is_intra[s->mb_x] = 0;

		if (v->dmb_is_raw)
			direct = get_bits1(gb);
		else
			direct = v->direct_mb_plane[mb_pos];

		if (direct) {
			if (s->next_pic.ptr->field_picture)
				warning("Mixed frame/field direct mode not supported\n");
			s->mv[0][0][0] = s->cur_pic.motion_val[0][s->block_index[0]][0] =
				scale_mv(s->next_pic.motion_val[1][s->block_index[0]][0], v->bfraction, 0, s->quarter_sample);
			s->mv[0][0][1] = s->cur_pic.motion_val[0][s->block_index[0]][1] =
				scale_mv(s->next_pic.motion_val[1][s->block_index[0]][1], v->bfraction, 0, s->quarter_sample);
			s->mv[1][0][0] = s->cur_pic.motion_val[1][s->block_index[0]][0] =
				scale_mv(s->next_pic.motion_val[1][s->block_index[0]][0], v->bfraction, 1, s->quarter_sample);
			s->mv[1][0][1] = s->cur_pic.motion_val[1][s->block_index[0]][1] =
				scale_mv(s->next_pic.motion_val[1][s->block_index[0]][1], v->bfraction, 1, s->quarter_sample);

			if (twomv) {
				s->mv[0][2][0] = s->cur_pic.motion_val[0][s->block_index[2]][0] =
					scale_mv(s->next_pic.motion_val[1][s->block_index[2]][0], v->bfraction, 0, s->quarter_sample);
				s->mv[0][2][1] = s->cur_pic.motion_val[0][s->block_index[2]][1] =
					scale_mv(s->next_pic.motion_val[1][s->block_index[2]][1], v->bfraction, 0, s->quarter_sample);
				s->mv[1][2][0] = s->cur_pic.motion_val[1][s->block_index[2]][0] =
					scale_mv(s->next_pic.motion_val[1][s->block_index[2]][0], v->bfraction, 1, s->quarter_sample);
				s->mv[1][2][1] = s->cur_pic.motion_val[1][s->block_index[2]][1] =
					scale_mv(s->next_pic.motion_val[1][s->block_index[2]][1], v->bfraction, 1, s->quarter_sample);

				for (i = 1; i < 4; i += 2) {
					s->mv[0][i][0] = s->cur_pic.motion_val[0][s->block_index[i]][0] = s->mv[0][i - 1][0];
					s->mv[0][i][1] = s->cur_pic.motion_val[0][s->block_index[i]][1] = s->mv[0][i - 1][1];
					s->mv[1][i][0] = s->cur_pic.motion_val[1][s->block_index[i]][0] = s->mv[1][i - 1][0];
					s->mv[1][i][1] = s->cur_pic.motion_val[1][s->block_index[i]][1] = s->mv[1][i - 1][1];
				}
			} else {
				for (i = 1; i < 4; i++) {
					s->mv[0][i][0] = s->cur_pic.motion_val[0][s->block_index[i]][0] = s->mv[0][0][0];
					s->mv[0][i][1] = s->cur_pic.motion_val[0][s->block_index[i]][1] = s->mv[0][0][1];
					s->mv[1][i][0] = s->cur_pic.motion_val[1][s->block_index[i]][0] = s->mv[1][0][0];
					s->mv[1][i][1] = s->cur_pic.motion_val[1][s->block_index[i]][1] = s->mv[1][0][1];
				}
			}
		}

		if (!direct) {
			if (skipped || !s->mb_intra) {
				bmvtype = decode012(gb);
				switch (bmvtype) {
				case 0:
					bmvtype = (v->bfraction >= (B_FRACTION_DEN / 2)) ? BMV_TYPE_BACKWARD : BMV_TYPE_FORWARD;
					break;
				case 1:
					bmvtype = (v->bfraction >= (B_FRACTION_DEN / 2)) ? BMV_TYPE_FORWARD : BMV_TYPE_BACKWARD;
					break;
				case 2:
					bmvtype = BMV_TYPE_INTERPOLATED;
				}
			}

			if (twomv && bmvtype != BMV_TYPE_INTERPOLATED)
				mvsw = get_bits1(gb);
		}

		if (!skipped) { // inter MB
			mb_has_coeffs = ff_vc1_mbmode_intfrp[0][idx_mbmode][3];
			if (mb_has_coeffs)
				cbp = 1 + get_vlc2(gb, v->cbpcy_vlc);
			if (!direct) {
				if (bmvtype == BMV_TYPE_INTERPOLATED && twomv) {
					v->fourmvbp = get_vlc2(gb, v->fourmvbp_vlc);
				} else if (bmvtype == BMV_TYPE_INTERPOLATED || twomv) {
					v->twomvbp = get_vlc2(gb, v->twomvbp_vlc);
				}
			}

			for (i = 0; i < 6; i++)
				v->mb_type[s->block_index[i]] = 0;
			fieldtx = v->fieldtx_plane[mb_pos] = ff_vc1_mbmode_intfrp[0][idx_mbmode][1];
			/* for all motion vector read MVDATA and motion compensate each block */
			dst_idx = 0;
			if (direct) {
				if (twomv) {
					for (i = 0; i < 4; i++) {
						ff_vc1_mc_4mv_luma(v, i, 0, 0);
						ff_vc1_mc_4mv_luma(v, i, 1, 1);
					}
					ff_vc1_mc_4mv_chroma4(v, 0, 0, 0);
					ff_vc1_mc_4mv_chroma4(v, 1, 1, 1);
				} else {
					ff_vc1_mc_1mv(v, 0);
					ff_vc1_interp_mc(v);
				}
			} else if (twomv && bmvtype == BMV_TYPE_INTERPOLATED) {
				mvbp = v->fourmvbp;
				for (i = 0; i < 4; i++) {
					dir = i == 1 || i == 3;
					dmv_x = dmv_y = 0;
					val = ((mvbp >> (3 - i)) & 1);
					if (val)
						get_mvdata_interlaced(v, &dmv_x, &dmv_y, 0);
					j = i > 1 ? 2 : 0;
					ff_vc1_pred_mv_intfr(v, j, dmv_x, dmv_y, 2, v->range_x, v->range_y, dir);
					ff_vc1_mc_4mv_luma(v, j, dir, dir);
					ff_vc1_mc_4mv_luma(v, j + 1, dir, dir);
				}

				ff_vc1_mc_4mv_chroma4(v, 0, 0, 0);
				ff_vc1_mc_4mv_chroma4(v, 1, 1, 1);
			} else if (bmvtype == BMV_TYPE_INTERPOLATED) {
				mvbp = v->twomvbp;
				dmv_x = dmv_y = 0;
				if (mvbp & 2)
					get_mvdata_interlaced(v, &dmv_x, &dmv_y, 0);

				ff_vc1_pred_mv_intfr(v, 0, dmv_x, dmv_y, 1, v->range_x, v->range_y, 0);
				ff_vc1_mc_1mv(v, 0);

				dmv_x = dmv_y = 0;
				if (mvbp & 1)
					get_mvdata_interlaced(v, &dmv_x, &dmv_y, 0);

				ff_vc1_pred_mv_intfr(v, 0, dmv_x, dmv_y, 1, v->range_x, v->range_y, 1);
				ff_vc1_interp_mc(v);
			} else if (twomv) {
				dir = bmvtype == BMV_TYPE_BACKWARD;
				dir2 = dir;
				if (mvsw)
					dir2 = !dir;
				mvbp = v->twomvbp;
				dmv_x = dmv_y = 0;
				if (mvbp & 2)
					get_mvdata_interlaced(v, &dmv_x, &dmv_y, 0);
				ff_vc1_pred_mv_intfr(v, 0, dmv_x, dmv_y, 2, v->range_x, v->range_y, dir);

				dmv_x = dmv_y = 0;
				if (mvbp & 1)
					get_mvdata_interlaced(v, &dmv_x, &dmv_y, 0);
				ff_vc1_pred_mv_intfr(v, 2, dmv_x, dmv_y, 2, v->range_x, v->range_y, dir2);

				if (mvsw) {
					for (i = 0; i < 2; i++) {
						s->mv[dir][i + 2][0] = s->mv[dir][i][0] = s->cur_pic.motion_val[dir][s->block_index[i + 2]][0] =
							s->cur_pic.motion_val[dir][s->block_index[i]][0];
						s->mv[dir][i + 2][1] = s->mv[dir][i][1] = s->cur_pic.motion_val[dir][s->block_index[i + 2]][1] =
							s->cur_pic.motion_val[dir][s->block_index[i]][1];
						s->mv[dir2][i + 2][0] = s->mv[dir2][i][0] = s->cur_pic.motion_val[dir2][s->block_index[i]][0] =
							s->cur_pic.motion_val[dir2][s->block_index[i + 2]][0];
						s->mv[dir2][i + 2][1] = s->mv[dir2][i][1] = s->cur_pic.motion_val[dir2][s->block_index[i]][1] =
							s->cur_pic.motion_val[dir2][s->block_index[i + 2]][1];
					}
				} else {
					ff_vc1_pred_mv_intfr(v, 0, 0, 0, 2, v->range_x, v->range_y, !dir);
					ff_vc1_pred_mv_intfr(v, 2, 0, 0, 2, v->range_x, v->range_y, !dir);
				}

				ff_vc1_mc_4mv_luma(v, 0, dir, 0);
				ff_vc1_mc_4mv_luma(v, 1, dir, 0);
				ff_vc1_mc_4mv_luma(v, 2, dir2, 0);
				ff_vc1_mc_4mv_luma(v, 3, dir2, 0);
				ff_vc1_mc_4mv_chroma4(v, dir, dir2, 0);
			} else {
				dir = bmvtype == BMV_TYPE_BACKWARD;

				mvbp = ff_vc1_mbmode_intfrp[0][idx_mbmode][2];
				dmv_x = dmv_y = 0;
				if (mvbp)
					get_mvdata_interlaced(v, &dmv_x, &dmv_y, 0);

				ff_vc1_pred_mv_intfr(v, 0, dmv_x, dmv_y, 1, v->range_x, v->range_y, dir);
				v->blk_mv_type[s->block_index[0]] = 1;
				v->blk_mv_type[s->block_index[1]] = 1;
				v->blk_mv_type[s->block_index[2]] = 1;
				v->blk_mv_type[s->block_index[3]] = 1;
				ff_vc1_pred_mv_intfr(v, 0, 0, 0, 2, v->range_x, v->range_y, !dir);
				for (i = 0; i < 2; i++) {
					s->mv[!dir][i + 2][0] = s->mv[!dir][i][0] = s->cur_pic.motion_val[!dir][s->block_index[i + 2]][0] =
						s->cur_pic.motion_val[!dir][s->block_index[i]][0];
					s->mv[!dir][i + 2][1] = s->mv[!dir][i][1] = s->cur_pic.motion_val[!dir][s->block_index[i + 2]][1] =
						s->cur_pic.motion_val[!dir][s->block_index[i]][1];
				}
				ff_vc1_mc_1mv(v, dir);
			}

			if (cbp)
				GET_MQUANT(); // p. 227
			s->cur_pic.qscale_table[mb_pos] = mquant;
			if (!v->ttmbf && cbp)
				ttmb = get_vlc2(gb, ff_vc1_ttmb_vlc[v->tt_index]);
			for (i = 0; i < 6; i++) {
				s->dc_val[s->block_index[i]] = 0;
				dst_idx += i >> 2;
				val = ((cbp >> (5 - i)) & 1);
				if (!fieldtx)
					off = (i & 4) ? 0 : ((i & 1) * 8 + (i & 2) * 4 * s->linesize);
				else
					off = (i & 4) ? 0 : ((i & 1) * 8 + ((i > 1) * s->linesize));
				if (val) {
					pat =
						vc1_decode_p_block(v, v->blocks[i], i, mquant, ttmb, first_block, s->dest[dst_idx] + off,
										   (i & 4) ? s->uvlinesize : (s->linesize << fieldtx),
										   CONFIG_GRAY && (i & 4) && (s->avctx->flags & AV_CODEC_FLAG_GRAY), &block_tt);
					if (pat < 0)
						return pat;
					block_cbp |= pat << (i << 2);
					if (!v->ttmbf && ttmb < 8)
						ttmb = -1;
					first_block = 0;
				}
			}

		} else { // skipped
			dir = 0;
			for (i = 0; i < 6; i++) {
				v->mb_type[s->block_index[i]] = 0;
				s->dc_val[s->block_index[i]] = 0;
			}
			s->cur_pic.mb_type[mb_pos] = MB_TYPE_SKIP;
			s->cur_pic.qscale_table[mb_pos] = 0;
			v->blk_mv_type[s->block_index[0]] = 0;
			v->blk_mv_type[s->block_index[1]] = 0;
			v->blk_mv_type[s->block_index[2]] = 0;
			v->blk_mv_type[s->block_index[3]] = 0;

			if (!direct) {
				if (bmvtype == BMV_TYPE_INTERPOLATED) {
					ff_vc1_pred_mv_intfr(v, 0, 0, 0, 1, v->range_x, v->range_y, 0);
					ff_vc1_pred_mv_intfr(v, 0, 0, 0, 1, v->range_x, v->range_y, 1);
				} else {
					dir = bmvtype == BMV_TYPE_BACKWARD;
					ff_vc1_pred_mv_intfr(v, 0, 0, 0, 1, v->range_x, v->range_y, dir);
					if (mvsw) {
						dir2 = dir;
						if (mvsw)
							dir2 = !dir;
						for (i = 0; i < 2; i++) {
							s->mv[dir][i + 2][0] = s->mv[dir][i][0] =
								s->cur_pic.motion_val[dir][s->block_index[i + 2]][0] =
									s->cur_pic.motion_val[dir][s->block_index[i]][0];
							s->mv[dir][i + 2][1] = s->mv[dir][i][1] =
								s->cur_pic.motion_val[dir][s->block_index[i + 2]][1] =
									s->cur_pic.motion_val[dir][s->block_index[i]][1];
							s->mv[dir2][i + 2][0] = s->mv[dir2][i][0] =
								s->cur_pic.motion_val[dir2][s->block_index[i]][0] =
									s->cur_pic.motion_val[dir2][s->block_index[i + 2]][0];
							s->mv[dir2][i + 2][1] = s->mv[dir2][i][1] =
								s->cur_pic.motion_val[dir2][s->block_index[i]][1] =
									s->cur_pic.motion_val[dir2][s->block_index[i + 2]][1];
						}
					} else {
						v->blk_mv_type[s->block_index[0]] = 1;
						v->blk_mv_type[s->block_index[1]] = 1;
						v->blk_mv_type[s->block_index[2]] = 1;
						v->blk_mv_type[s->block_index[3]] = 1;
						ff_vc1_pred_mv_intfr(v, 0, 0, 0, 2, v->range_x, v->range_y, !dir);
						for (i = 0; i < 2; i++) {
							s->mv[!dir][i + 2][0] = s->mv[!dir][i][0] =
								s->cur_pic.motion_val[!dir][s->block_index[i + 2]][0] =
									s->cur_pic.motion_val[!dir][s->block_index[i]][0];
							s->mv[!dir][i + 2][1] = s->mv[!dir][i][1] =
								s->cur_pic.motion_val[!dir][s->block_index[i + 2]][1] =
									s->cur_pic.motion_val[!dir][s->block_index[i]][1];
						}
					}
				}
			}

			ff_vc1_mc_1mv(v, dir);
			if (direct || bmvtype == BMV_TYPE_INTERPOLATED) {
				ff_vc1_interp_mc(v);
			}
			v->fieldtx_plane[mb_pos] = 0;
		}
	}
	v->cbp[s->mb_x] = block_cbp;
	v->ttblk[s->mb_x] = block_tt;

	return 0;
}

/** Decode blocks of I-frame
 */
static void vc1_decode_i_blocks(VC1Context *v) {
	int k, j;
	PictureState *s = &v->s;
	int cbp, val;
	int mb_pos;

	/* select coding mode used for VLC tables selection */
	switch (v->y_ac_table_index) {
	case 0:
		v->codingset = (v->pqindex <= 8) ? CS_HIGH_RATE_INTRA : CS_LOW_MOT_INTRA;
		break;
	case 1:
		v->codingset = CS_HIGH_MOT_INTRA;
		break;
	case 2:
		v->codingset = CS_MID_RATE_INTRA;
		break;
	}

	switch (v->c_ac_table_index) {
	case 0:
		v->codingset2 = (v->pqindex <= 8) ? CS_HIGH_RATE_INTER : CS_LOW_MOT_INTER;
		break;
	case 1:
		v->codingset2 = CS_HIGH_MOT_INTER;
		break;
	case 2:
		v->codingset2 = CS_MID_RATE_INTER;
		break;
	}

	/* Set DC scale - y and c use the same so we only set y */
	s->y_dc_scale = ff_wmv3_dc_scale_table[v->pq];

	// do frame decode
	s->mb_x = s->mb_y = 0;
	s->mb_intra = 1;
	s->first_slice_line = 1;
	for (s->mb_y = s->start_mb_y; s->mb_y < s->end_mb_y; s->mb_y++) {
		s->mb_x = 0;
		init_block_index(v);
		for (; s->mb_x < v->end_mb_x; s->mb_x++) {
			update_block_index(s);
			s->bdsp.clear_blocks(v->block[v->cur_blk_idx][0]);
			mb_pos = s->mb_x + s->mb_y * s->mb_width;
			s->cur_pic.mb_type[mb_pos] = MB_TYPE_INTRA;
			s->cur_pic.qscale_table[mb_pos] = v->pq;
			for (int i = 0; i < 4; i++) {
				s->cur_pic.motion_val[1][s->block_index[i]][0] = 0;
				s->cur_pic.motion_val[1][s->block_index[i]][1] = 0;
			}

			// do actual MB decoding and displaying
			cbp = get_vlc2(&v->gb, ff_msmp4_mb_i_vlc);
			v->s.ac_pred = get_bits1(&v->gb);

			for (k = 0; k < 6; k++) {
				v->mb_type[s->block_index[k]] = 1;

				val = ((cbp >> (5 - k)) & 1);

				if (k < 4)
					val = vc1_coded_block_pred(&v->s, k, val);
				cbp |= val << (5 - k);

				vc1_decode_i_block(v, v->block[v->cur_blk_idx][block_map[k]], k, val,
								   (k < 4) ? v->codingset : v->codingset2);

				if (CONFIG_GRAY && k > 3 && (s->avctx->flags & AV_CODEC_FLAG_GRAY))
					continue;
				v->vc1dsp.vc1_inv_trans_8x8(v->block[v->cur_blk_idx][block_map[k]]);
			}

			if (v->overlap && v->pq >= 9) {
				ff_vc1_i_overlap_filter(v);
				if (v->rangeredfrm)
					for (k = 0; k < 6; k++)
						for (j = 0; j < 64; j++)
							v->block[v->cur_blk_idx][block_map[k]][j] *= 2;
				vc1_put_blocks_clamped(v, 1);
			} else {
				if (v->rangeredfrm)
					for (k = 0; k < 6; k++)
						for (j = 0; j < 64; j++)
							v->block[v->cur_blk_idx][block_map[k]][j] =
								(v->block[v->cur_blk_idx][block_map[k]][j] - 64) * 2;
				vc1_put_blocks_clamped(v, 0);
			}

			if (v->loop_filter)
				ff_vc1_i_loop_filter(v);

			if (get_bits_left(&v->gb) < 0) {
				ff_er_add_slice(&s->er, 0, 0, s->mb_x, s->mb_y, ER_MB_ERROR);
				warning("Bits overconsumption: %i > %i\n", get_bits_count(&v->gb), v->gb.size_in_bits);
				return;
			}

			v->topleft_blk_idx = (v->topleft_blk_idx + 1) % (v->end_mb_x + 2);
			v->top_blk_idx = (v->top_blk_idx + 1) % (v->end_mb_x + 2);
			v->left_blk_idx = (v->left_blk_idx + 1) % (v->end_mb_x + 2);
			v->cur_blk_idx = (v->cur_blk_idx + 1) % (v->end_mb_x + 2);
		}

		s->first_slice_line = 0;
	}

	/* This is intentionally mb_height and not end_mb_y - unlike in advanced
	 * profile, these only differ are when decoding MSS2 rectangles. */
	ff_er_add_slice(&s->er, 0, 0, s->mb_width - 1, s->mb_height - 1, ER_MB_END);
}

/** Decode blocks of I-frame for advanced profile
 */
static int vc1_decode_i_blocks_adv(VC1Context *v) {
	int k;
	PictureState *s = &v->s;
	Bits *const gb = &v->gb;
	int cbp, val;
	int mb_pos;
	int mquant;
	int mqdiff;

	if (get_bits_left(gb) <= 1)
		return (-1);

	/* select coding mode used for VLC tables selection */
	switch (v->y_ac_table_index) {
	case 0:
		v->codingset = (v->pqindex <= 8) ? CS_HIGH_RATE_INTRA : CS_LOW_MOT_INTRA;
		break;
	case 1:
		v->codingset = CS_HIGH_MOT_INTRA;
		break;
	case 2:
		v->codingset = CS_MID_RATE_INTRA;
		break;
	}

	switch (v->c_ac_table_index) {
	case 0:
		v->codingset2 = (v->pqindex <= 8) ? CS_HIGH_RATE_INTER : CS_LOW_MOT_INTER;
		break;
	case 1:
		v->codingset2 = CS_HIGH_MOT_INTER;
		break;
	case 2:
		v->codingset2 = CS_MID_RATE_INTER;
		break;
	}

	// do frame decode
	s->mb_intra = 1;
	s->first_slice_line = 1;
	s->mb_x = 0;
	s->mb_y = s->start_mb_y;
	if (s->start_mb_y) {
		memset(&s->coded_block[(2 * s->mb_y - 1) * s->b8_stride - 2], 0, (1 + s->b8_stride) * sizeof(*s->coded_block));
	}
	for (; s->mb_y < s->end_mb_y; s->mb_y++) {
		s->mb_x = 0;
		init_block_index(v);
		for (; s->mb_x < s->mb_width; s->mb_x++) {
			mquant = v->pq;
			update_block_index(s);
			s->bdsp.clear_blocks(v->block[v->cur_blk_idx][0]);
			mb_pos = s->mb_x + s->mb_y * s->mb_stride;
			s->cur_pic.mb_type[mb_pos + v->mb_off] = MB_TYPE_INTRA;
			for (int i = 0; i < 4; i++) {
				s->cur_pic.motion_val[1][s->block_index[i] + v->blocks_off][0] = 0;
				s->cur_pic.motion_val[1][s->block_index[i] + v->blocks_off][1] = 0;
			}

			// do actual MB decoding and displaying
			if (v->fieldtx_is_raw)
				v->fieldtx_plane[mb_pos] = get_bits1(gb);
			if (get_bits_left(gb) <= 1) {
				ff_er_add_slice(&s->er, 0, s->start_mb_y, s->mb_x, s->mb_y, ER_MB_ERROR);
				return 0;
			}

			cbp = get_vlc2(gb, ff_msmp4_mb_i_vlc);
			if (v->acpred_is_raw)
				v->s.ac_pred = get_bits1(gb);
			else
				v->s.ac_pred = v->acpred_plane[mb_pos];

			if (v->condover == CONDOVER_SELECT && v->overflg_is_raw)
				v->over_flags_plane[mb_pos] = get_bits1(gb);

			GET_MQUANT();

			s->cur_pic.qscale_table[mb_pos] = mquant;
			/* Set DC scale - y and c use the same so we only set y */
			s->y_dc_scale = ff_wmv3_dc_scale_table[ABS(mquant)];

			for (k = 0; k < 6; k++) {
				v->mb_type[s->block_index[k]] = 1;

				val = ((cbp >> (5 - k)) & 1);

				if (k < 4)
					val = vc1_coded_block_pred(&v->s, k, val);
				cbp |= val << (5 - k);

				v->a_avail = !s->first_slice_line || (k == 2 || k == 3);
				v->c_avail = !!s->mb_x || (k == 1 || k == 3);

				vc1_decode_i_block_adv(v, v->block[v->cur_blk_idx][block_map[k]], k, val,
									   (k < 4) ? v->codingset : v->codingset2, mquant);

				if (CONFIG_GRAY && k > 3 && (s->avctx->flags & AV_CODEC_FLAG_GRAY))
					continue;
				v->vc1dsp.vc1_inv_trans_8x8(v->block[v->cur_blk_idx][block_map[k]]);
			}

			if (v->overlap && (v->pq >= 9 || v->condover != CONDOVER_NONE))
				ff_vc1_i_overlap_filter(v);
			vc1_put_blocks_clamped(v, 1);
			if (v->loop_filter)
				ff_vc1_i_loop_filter(v);

			if (get_bits_left(gb) < 0) {
				// TODO: may need modification to handle slice coding
				ff_er_add_slice(&s->er, 0, s->start_mb_y, s->mb_x, s->mb_y, ER_MB_ERROR);
				warning("Bits overconsumption: %i > %i\n", get_bits_count(gb), gb->size_in_bits);
				return 0;
			}
			inc_blk_idx(v->topleft_blk_idx);
			inc_blk_idx(v->top_blk_idx);
			inc_blk_idx(v->left_blk_idx);
			inc_blk_idx(v->cur_blk_idx);
		}
		s->first_slice_line = 0;
	}

	ff_er_add_slice(&s->er, 0, s->start_mb_y << v->field_mode, s->mb_width - 1, (s->end_mb_y << v->field_mode) - 1,
					ER_MB_END);
	return 0;
}

static void vc1_decode_p_blocks(VC1Context *v) {
	PictureState *s = &v->s;
	int apply_loop_filter;
	int ret;

	/* select coding mode used for VLC tables selection */
	switch (v->c_ac_table_index) {
	case 0:
		v->codingset = (v->pqindex <= 8) ? CS_HIGH_RATE_INTRA : CS_LOW_MOT_INTRA;
		break;
	case 1:
		v->codingset = CS_HIGH_MOT_INTRA;
		break;
	case 2:
		v->codingset = CS_MID_RATE_INTRA;
		break;
	}

	switch (v->c_ac_table_index) {
	case 0:
		v->codingset2 = (v->pqindex <= 8) ? CS_HIGH_RATE_INTER : CS_LOW_MOT_INTER;
		break;
	case 1:
		v->codingset2 = CS_HIGH_MOT_INTER;
		break;
	case 2:
		v->codingset2 = CS_MID_RATE_INTER;
		break;
	}

	apply_loop_filter = v->loop_filter && !(s->avctx->skip_loop_filter >= AVDISCARD_NONKEY);
	s->first_slice_line = 1;
	memset(v->cbp_base, 0, sizeof(v->cbp_base[0]) * 3 * s->mb_stride);
	for (s->mb_y = s->start_mb_y; s->mb_y < s->end_mb_y; s->mb_y++) {
		s->mb_x = 0;
		init_block_index(v);
		for (; s->mb_x < s->mb_width; s->mb_x++) {
			update_block_index(s);

			if (v->fcm == ILACE_FIELD || (v->fcm == PROGRESSIVE && v->mv_type_is_raw) || v->skip_is_raw)
				if (get_bits_left(&v->gb) <= 1) {
					ff_er_add_slice(&s->er, 0, s->start_mb_y, s->mb_x, s->mb_y, ER_MB_ERROR);
					return;
				}

			if (v->fcm == ILACE_FIELD) {
				ret = vc1_decode_p_mb_intfi(v);
				if (apply_loop_filter)
					ff_vc1_p_loop_filter(v);
			} else if (v->fcm == ILACE_FRAME) {
				ret = vc1_decode_p_mb_intfr(v);
				if (apply_loop_filter)
					ff_vc1_p_intfr_loop_filter(v);
			} else {
				ret = vc1_decode_p_mb(v);
				if (apply_loop_filter)
					ff_vc1_p_loop_filter(v);
			}
			if (ret < 0 || get_bits_left(&v->gb) < 0 || get_bits_count(&v->gb) < 0) {
				// TODO: may need modification to handle slice coding
				ff_er_add_slice(&s->er, 0, s->start_mb_y, s->mb_x, s->mb_y, ER_MB_ERROR);
				warning("Error or Bits overconsumption: %i > %i at %ix%i\n", get_bits_count(&v->gb), v->gb.size_in_bits,
						s->mb_x, s->mb_y);
				return;
			}
			inc_blk_idx(v->topleft_blk_idx);
			inc_blk_idx(v->top_blk_idx);
			inc_blk_idx(v->left_blk_idx);
			inc_blk_idx(v->cur_blk_idx);
		}
		memmove(v->cbp_base, v->cbp - s->mb_stride, sizeof(v->cbp_base[0]) * 2 * s->mb_stride);
		memmove(v->ttblk_base, v->ttblk - s->mb_stride, sizeof(v->ttblk_base[0]) * 2 * s->mb_stride);
		memmove(v->is_intra_base, v->is_intra - s->mb_stride, sizeof(v->is_intra_base[0]) * 2 * s->mb_stride);
		memmove(v->luma_mv_base, v->luma_mv - s->mb_stride, sizeof(v->luma_mv_base[0]) * 2 * s->mb_stride);
		s->first_slice_line = 0;
	}
	ff_er_add_slice(&s->er, 0, s->start_mb_y << v->field_mode, s->mb_width - 1, (s->end_mb_y << v->field_mode) - 1,
					ER_MB_END);
}

static void vc1_decode_b_blocks(VC1Context *v) {
	PictureState *s = &v->s;

	/* select coding mode used for VLC tables selection */
	switch (v->c_ac_table_index) {
	case 0:
		v->codingset = (v->pqindex <= 8) ? CS_HIGH_RATE_INTRA : CS_LOW_MOT_INTRA;
		break;
	case 1:
		v->codingset = CS_HIGH_MOT_INTRA;
		break;
	case 2:
		v->codingset = CS_MID_RATE_INTRA;
		break;
	}

	switch (v->c_ac_table_index) {
	case 0:
		v->codingset2 = (v->pqindex <= 8) ? CS_HIGH_RATE_INTER : CS_LOW_MOT_INTER;
		break;
	case 1:
		v->codingset2 = CS_HIGH_MOT_INTER;
		break;
	case 2:
		v->codingset2 = CS_MID_RATE_INTER;
		break;
	}

	s->first_slice_line = 1;
	for (s->mb_y = s->start_mb_y; s->mb_y < s->end_mb_y; s->mb_y++) {
		s->mb_x = 0;
		init_block_index(v);
		for (; s->mb_x < s->mb_width; s->mb_x++) {
			update_block_index(s);

			if (v->fcm == ILACE_FIELD || v->skip_is_raw || v->dmb_is_raw)
				if (get_bits_left(&v->gb) <= 1) {
					ff_er_add_slice(&s->er, 0, s->start_mb_y, s->mb_x, s->mb_y, ER_MB_ERROR);
					return;
				}

			if (v->fcm == ILACE_FIELD) {
				vc1_decode_b_mb_intfi(v);
				if (v->loop_filter)
					ff_vc1_b_intfi_loop_filter(v);
			} else if (v->fcm == ILACE_FRAME) {
				vc1_decode_b_mb_intfr(v);
				if (v->loop_filter)
					ff_vc1_p_intfr_loop_filter(v);
			} else {
				vc1_decode_b_mb(v);
				if (v->loop_filter)
					ff_vc1_i_loop_filter(v);
			}
			if (get_bits_left(&v->gb) < 0 || get_bits_count(&v->gb) < 0) {
				// TODO: may need modification to handle slice coding
				ff_er_add_slice(&s->er, 0, s->start_mb_y, s->mb_x, s->mb_y, ER_MB_ERROR);
				warning("Bits overconsumption: %i > %i at %ix%i\n", get_bits_count(&v->gb), v->gb.size_in_bits, s->mb_x,
						s->mb_y);
				return;
			}
		}
		memmove(v->cbp_base, v->cbp - s->mb_stride, sizeof(v->cbp_base[0]) * 2 * s->mb_stride);
		memmove(v->ttblk_base, v->ttblk - s->mb_stride, sizeof(v->ttblk_base[0]) * 2 * s->mb_stride);
		memmove(v->is_intra_base, v->is_intra - s->mb_stride, sizeof(v->is_intra_base[0]) * 2 * s->mb_stride);
		s->first_slice_line = 0;
	}
	ff_er_add_slice(&s->er, 0, s->start_mb_y << v->field_mode, s->mb_width - 1, (s->end_mb_y << v->field_mode) - 1,
					ER_MB_END);
}

static void vc1_decode_skip_blocks(VC1Context *v) {
	PictureState *s = &v->s;

	if (!v->s.last_pic.data[0])
		return;

	ff_er_add_slice(&s->er, 0, s->start_mb_y, s->mb_width - 1, s->end_mb_y - 1, ER_MB_END);
	s->first_slice_line = 1;
	for (s->mb_y = s->start_mb_y; s->mb_y < s->end_mb_y; s->mb_y++) {
		s->mb_x = 0;
		init_block_index(v);
		update_block_index(s);
		memcpy(s->dest[0], s->last_pic.data[0] + s->mb_y * 16 * s->linesize, s->linesize * 16);
		memcpy(s->dest[1], s->last_pic.data[1] + s->mb_y * 8 * s->uvlinesize, s->uvlinesize * 8);
		memcpy(s->dest[2], s->last_pic.data[2] + s->mb_y * 8 * s->uvlinesize, s->uvlinesize * 8);
		s->first_slice_line = 0;
	}
}

void ff_vc1_decode_blocks(VC1Context *v) {

	v->esc3_level_length = 0;
	if (v->x8_type) {
		ff_intrax8_decode_picture(v->x8, v->s.cur_pic.ptr, &v->gb, &v->s.mb_x, &v->s.mb_y, 2 * v->pq + v->halfpq,
								  v->pq * !v->pquantizer, v->loop_filter, v->s.low_delay);

		ff_er_add_slice(&v->s.er, 0, 0, (v->s.mb_x >> 1) - 1, (v->s.mb_y >> 1) - 1, ER_MB_END);
	} else {
		v->cur_blk_idx = 0;
		v->left_blk_idx = -1;
		v->topleft_blk_idx = 1;
		v->top_blk_idx = 2;
		switch (v->s.pict_type) {
		case AV_PICTURE_TYPE_I:
			if (v->profile == PROFILE_ADVANCED)
				vc1_decode_i_blocks_adv(v);
			else
				vc1_decode_i_blocks(v);
			break;
		case AV_PICTURE_TYPE_P:
			if (v->p_frame_skipped)
				vc1_decode_skip_blocks(v);
			else
				vc1_decode_p_blocks(v);
			break;
		case AV_PICTURE_TYPE_B:
			if (v->bi_type) {
				if (v->profile == PROFILE_ADVANCED)
					vc1_decode_i_blocks_adv(v);
				else
					vc1_decode_i_blocks(v);
			} else
				vc1_decode_b_blocks(v);
			break;
		}
	}
}

#undef GET_MQUANT
#undef GET_MVDATA
#undef inc_blk_idx

enum {
	VP_START = 1,
	ER_AC_ERROR = 2,
	ER_DC_ERROR = 4,
	ER_MV_ERROR = 8,
	ER_AC_END = 16,
	ER_DC_END = 32,
	ER_MV_END = 64,
	FF_EC_GUESS_MVS = 1,
	FF_EC_DEBLOCK = 2,
	FF_EC_FAVOR_INTER = 256,
	FF_THREAD_SLICE = 2,
	FF_DEBUG_ER = 1024,
	AV_EF_EXPLODE = 8,
	FF_DECODE_ERROR_CONCEALMENT_ACTIVE = 4,
	MB_TYPE_INTRA4x4 = 1,
	MB_TYPE_L0 = 12288,
	MB_TYPE_FORWARD_MV = 4096,
	MV_DIR_FORWARD = 1,
	MV_DIR_BACKWARD = 2,
	MV_TYPE_16X16 = 0,
	MV_TYPE_8X8 = 1
};
#define IS_INTRA(a) ((a) & 7)
#define IS_INTER(a) ((a) & 120)
#define IS_8X8(a) ((a) & 64)
/**
 * @param stride the number of MVs to get to the next row
 * @param mv_step the number of MVs per row or column in a macroblock
 */
static void set_mv_strides(ERContext *s, ptrdiff_t *mv_step, ptrdiff_t *stride) {
	if (false) {
		assert(s->quarter_sample);
		*mv_step = 4;
		*stride = s->mb_width * 4;
	} else {
		*mv_step = 2;
		*stride = s->b8_stride;
	}
}

/**
 * Replace the current MB with a flat dc-only version.
 */
static void put_dc(ERContext *s, uint8 *dest_y, uint8 *dest_cb, uint8 *dest_cr, int mb_x, int mb_y) {
	int *linesize = s->cur_pic.f->linesize;
	int dc, dcu, dcv, y, i;
	for (i = 0; i < 4; i++) {
		dc = s->dc_val[0][mb_x * 2 + (i & 1) + (mb_y * 2 + (i >> 1)) * s->b8_stride];
		if (dc < 0)
			dc = 0;
		else if (dc > 2040)
			dc = 2040;
		for (y = 0; y < 8; y++) {
			int x;
			for (x = 0; x < 8; x++)
				dest_y[x + (i & 1) * 8 + (y + (i >> 1) * 8) * linesize[0]] = dc / 8;
		}
	}
	dcu = s->dc_val[1][mb_x + mb_y * s->mb_stride];
	dcv = s->dc_val[2][mb_x + mb_y * s->mb_stride];
	if (dcu < 0)
		dcu = 0;
	else if (dcu > 2040)
		dcu = 2040;
	if (dcv < 0)
		dcv = 0;
	else if (dcv > 2040)
		dcv = 2040;

	if (dest_cr)
		for (y = 0; y < 8; y++) {
			int x;
			for (x = 0; x < 8; x++) {
				dest_cb[x + y * linesize[1]] = dcu / 8;
				dest_cr[x + y * linesize[2]] = dcv / 8;
			}
		}
}

static void filter181(int16 *data, int width, int height, ptrdiff_t stride) {
	int x, y;

	/* horizontal filter */
	for (y = 1; y < height - 1; y++) {
		int prev_dc = data[0 + y * stride];

		for (x = 1; x < width - 1; x++) {
			int dc;
			dc = -prev_dc + data[x + y * stride] * 8 - data[x + 1 + y * stride];
			dc = (av_clip(dc, INT_MIN / 10923, INT_MAX / 10923 - 32768) * 10923 + 32768) >> 16;
			prev_dc = data[x + y * stride];
			data[x + y * stride] = dc;
		}
	}

	/* vertical filter */
	for (x = 1; x < width - 1; x++) {
		int prev_dc = data[x];

		for (y = 1; y < height - 1; y++) {
			int dc;

			dc = -prev_dc + data[x + y * stride] * 8 - data[x + (y + 1) * stride];
			dc = (av_clip(dc, INT_MIN / 10923, INT_MAX / 10923 - 32768) * 10923 + 32768) >> 16;
			prev_dc = data[x + y * stride];
			data[x + y * stride] = dc;
		}
	}
}

/**
 * guess the dc of blocks which do not have an undamaged dc
 * @param w     width in 8 pixel blocks
 * @param h     height in 8 pixel blocks
 */
static void guess_dc(ERContext *s, int16 *dc, int w, int h, ptrdiff_t stride, int is_luma) {
	int b_x, b_y;
	int16(*col)[4] = new int16[stride * h][4];
	uint32(*dist)[4] = new uint32[stride * h][4];

	if (!col || !dist) {
		warning("guess_dc() is out of memory\n");
		goto fail;
	}

	for (b_y = 0; b_y < h; b_y++) {
		int color = 1024;
		int distance = -1;
		for (b_x = 0; b_x < w; b_x++) {
			int mb_index_j = (b_x >> is_luma) + (b_y >> is_luma) * s->mb_stride;
			int error_j = s->error_status_table[mb_index_j];
			int intra_j = IS_INTRA(s->cur_pic.mb_type[mb_index_j]);
			if (intra_j == 0 || !(error_j & ER_DC_ERROR)) {
				color = dc[b_x + b_y * stride];
				distance = b_x;
			}
			col[b_x + b_y * stride][1] = color;
			dist[b_x + b_y * stride][1] = distance >= 0 ? b_x - distance : 9999;
		}
		color = 1024;
		distance = -1;
		for (b_x = w - 1; b_x >= 0; b_x--) {
			int mb_index_j = (b_x >> is_luma) + (b_y >> is_luma) * s->mb_stride;
			int error_j = s->error_status_table[mb_index_j];
			int intra_j = IS_INTRA(s->cur_pic.mb_type[mb_index_j]);
			if (intra_j == 0 || !(error_j & ER_DC_ERROR)) {
				color = dc[b_x + b_y * stride];
				distance = b_x;
			}
			col[b_x + b_y * stride][0] = color;
			dist[b_x + b_y * stride][0] = distance >= 0 ? distance - b_x : 9999;
		}
	}
	for (b_x = 0; b_x < w; b_x++) {
		int color = 1024;
		int distance = -1;
		for (b_y = 0; b_y < h; b_y++) {
			int mb_index_j = (b_x >> is_luma) + (b_y >> is_luma) * s->mb_stride;
			int error_j = s->error_status_table[mb_index_j];
			int intra_j = IS_INTRA(s->cur_pic.mb_type[mb_index_j]);
			if (intra_j == 0 || !(error_j & ER_DC_ERROR)) {
				color = dc[b_x + b_y * stride];
				distance = b_y;
			}
			col[b_x + b_y * stride][3] = color;
			dist[b_x + b_y * stride][3] = distance >= 0 ? b_y - distance : 9999;
		}
		color = 1024;
		distance = -1;
		for (b_y = h - 1; b_y >= 0; b_y--) {
			int mb_index_j = (b_x >> is_luma) + (b_y >> is_luma) * s->mb_stride;
			int error_j = s->error_status_table[mb_index_j];
			int intra_j = IS_INTRA(s->cur_pic.mb_type[mb_index_j]);
			if (intra_j == 0 || !(error_j & ER_DC_ERROR)) {
				color = dc[b_x + b_y * stride];
				distance = b_y;
			}
			col[b_x + b_y * stride][2] = color;
			dist[b_x + b_y * stride][2] = distance >= 0 ? distance - b_y : 9999;
		}
	}

	for (b_y = 0; b_y < h; b_y++) {
		for (b_x = 0; b_x < w; b_x++) {
			int mb_index, error, j;
			int64 guess, weight_sum;
			mb_index = (b_x >> is_luma) + (b_y >> is_luma) * s->mb_stride;
			error = s->error_status_table[mb_index];

			if (IS_INTER(s->cur_pic.mb_type[mb_index]))
				continue; // inter
			if (!(error & ER_DC_ERROR))
				continue; // dc-ok

			weight_sum = 0;
			guess = 0;
			for (j = 0; j < 4; j++) {
				int64 weight = 256 * 256 * 256 * 16 / MAX<uint32>(dist[b_x + b_y * stride][j], 1);
				guess += weight * (int64)col[b_x + b_y * stride][j];
				weight_sum += weight;
			}
			guess = (guess + weight_sum / 2) / weight_sum;
			dc[b_x + b_y * stride] = guess;
		}
	}

fail:
	delete[] col;
	delete[] dist;
}

/**
 * simple horizontal deblocking filter used for error resilience
 * @param w     width in 8 pixel blocks
 * @param h     height in 8 pixel blocks
 */
static void h_block_filter(ERContext *s, uint8 *dst, int w, int h, ptrdiff_t stride, int is_luma) {
	int b_x, b_y;
	ptrdiff_t mvx_stride, mvy_stride;
	const Crop cm;
	set_mv_strides(s, &mvx_stride, &mvy_stride);
	mvx_stride >>= is_luma;
	mvy_stride *= mvx_stride;

	for (b_y = 0; b_y < h; b_y++) {
		for (b_x = 0; b_x < w - 1; b_x++) {
			int y;
			int left_status = s->error_status_table[(b_x >> is_luma) + (b_y >> is_luma) * s->mb_stride];
			int right_status = s->error_status_table[((b_x + 1) >> is_luma) + (b_y >> is_luma) * s->mb_stride];
			int left_intra = IS_INTRA(s->cur_pic.mb_type[(b_x >> is_luma) + (b_y >> is_luma) * s->mb_stride]);
			int right_intra = IS_INTRA(s->cur_pic.mb_type[((b_x + 1) >> is_luma) + (b_y >> is_luma) * s->mb_stride]);
			int left_damage = left_status & ER_MB_ERROR;
			int right_damage = right_status & ER_MB_ERROR;
			int offset = b_x * 8 + b_y * stride * 8;
			int16 *left_mv = s->cur_pic.motion_val[0][mvy_stride * b_y + mvx_stride * b_x];
			int16 *right_mv = s->cur_pic.motion_val[0][mvy_stride * b_y + mvx_stride * (b_x + 1)];
			if (!(left_damage || right_damage))
				continue; // both undamaged
			if ((!left_intra) && (!right_intra) && ABS(left_mv[0] - right_mv[0]) + ABS(left_mv[1] + right_mv[1]) < 2)
				continue;

			for (y = 0; y < 8; y++) {
				int a, b, c, d;

				a = dst[offset + 7 + y * stride] - dst[offset + 6 + y * stride];
				b = dst[offset + 8 + y * stride] - dst[offset + 7 + y * stride];
				c = dst[offset + 9 + y * stride] - dst[offset + 8 + y * stride];

				d = ABS(b) - ((ABS(a) + ABS(c) + 1) >> 1);
				d = MAX(d, 0);
				if (b < 0)
					d = -d;

				if (d == 0)
					continue;

				if (!(left_damage && right_damage))
					d = d * 16 / 9;

				if (left_damage) {
					dst[offset + 7 + y * stride] = cm[dst[offset + 7 + y * stride] + ((d * 7) >> 4)];
					dst[offset + 6 + y * stride] = cm[dst[offset + 6 + y * stride] + ((d * 5) >> 4)];
					dst[offset + 5 + y * stride] = cm[dst[offset + 5 + y * stride] + ((d * 3) >> 4)];
					dst[offset + 4 + y * stride] = cm[dst[offset + 4 + y * stride] + ((d * 1) >> 4)];
				}
				if (right_damage) {
					dst[offset + 8 + y * stride] = cm[dst[offset + 8 + y * stride] - ((d * 7) >> 4)];
					dst[offset + 9 + y * stride] = cm[dst[offset + 9 + y * stride] - ((d * 5) >> 4)];
					dst[offset + 10 + y * stride] = cm[dst[offset + 10 + y * stride] - ((d * 3) >> 4)];
					dst[offset + 11 + y * stride] = cm[dst[offset + 11 + y * stride] - ((d * 1) >> 4)];
				}
			}
		}
	}
}

/**
 * simple vertical deblocking filter used for error resilience
 * @param w     width in 8 pixel blocks
 * @param h     height in 8 pixel blocks
 */
static void v_block_filter(ERContext *s, uint8 *dst, int w, int h, ptrdiff_t stride, int is_luma) {
	int b_x, b_y;
	ptrdiff_t mvx_stride, mvy_stride;
	const Crop cm;
	set_mv_strides(s, &mvx_stride, &mvy_stride);
	mvx_stride >>= is_luma;
	mvy_stride *= mvx_stride;

	for (b_y = 0; b_y < h - 1; b_y++) {
		for (b_x = 0; b_x < w; b_x++) {
			int x;
			int top_status = s->error_status_table[(b_x >> is_luma) + (b_y >> is_luma) * s->mb_stride];
			int bottom_status = s->error_status_table[(b_x >> is_luma) + ((b_y + 1) >> is_luma) * s->mb_stride];
			int top_intra = IS_INTRA(s->cur_pic.mb_type[(b_x >> is_luma) + (b_y >> is_luma) * s->mb_stride]);
			int bottom_intra = IS_INTRA(s->cur_pic.mb_type[(b_x >> is_luma) + ((b_y + 1) >> is_luma) * s->mb_stride]);
			int top_damage = top_status & ER_MB_ERROR;
			int bottom_damage = bottom_status & ER_MB_ERROR;
			int offset = b_x * 8 + b_y * stride * 8;

			int16 *top_mv = s->cur_pic.motion_val[0][mvy_stride * b_y + mvx_stride * b_x];
			int16 *bottom_mv = s->cur_pic.motion_val[0][mvy_stride * (b_y + 1) + mvx_stride * b_x];

			if (!(top_damage || bottom_damage))
				continue; // both undamaged

			if ((!top_intra) && (!bottom_intra) && ABS(top_mv[0] - bottom_mv[0]) + ABS(top_mv[1] + bottom_mv[1]) < 2)
				continue;

			for (x = 0; x < 8; x++) {
				int a, b, c, d;

				a = dst[offset + x + 7 * stride] - dst[offset + x + 6 * stride];
				b = dst[offset + x + 8 * stride] - dst[offset + x + 7 * stride];
				c = dst[offset + x + 9 * stride] - dst[offset + x + 8 * stride];

				d = ABS(b) - ((ABS(a) + ABS(c) + 1) >> 1);
				d = MAX(d, 0);
				if (b < 0)
					d = -d;

				if (d == 0)
					continue;

				if (!(top_damage && bottom_damage))
					d = d * 16 / 9;

				if (top_damage) {
					dst[offset + x + 7 * stride] = cm[dst[offset + x + 7 * stride] + ((d * 7) >> 4)];
					dst[offset + x + 6 * stride] = cm[dst[offset + x + 6 * stride] + ((d * 5) >> 4)];
					dst[offset + x + 5 * stride] = cm[dst[offset + x + 5 * stride] + ((d * 3) >> 4)];
					dst[offset + x + 4 * stride] = cm[dst[offset + x + 4 * stride] + ((d * 1) >> 4)];
				}
				if (bottom_damage) {
					dst[offset + x + 8 * stride] = cm[dst[offset + x + 8 * stride] - ((d * 7) >> 4)];
					dst[offset + x + 9 * stride] = cm[dst[offset + x + 9 * stride] - ((d * 5) >> 4)];
					dst[offset + x + 10 * stride] = cm[dst[offset + x + 10 * stride] - ((d * 3) >> 4)];
					dst[offset + x + 11 * stride] = cm[dst[offset + x + 11 * stride] - ((d * 1) >> 4)];
				}
			}
		}
	}
}

#define MV_FROZEN 8
#define MV_CHANGED 4
#define MV_UNCHANGED 2
#define MV_LISTED 1
static inline void add_blocklist(int (*blocklist)[2], int *blocklist_length, uint8 *fixed, int mb_x, int mb_y,
								 int mb_xy) {
	if (fixed[mb_xy])
		return;
	fixed[mb_xy] = MV_LISTED;
	blocklist[*blocklist_length][0] = mb_x;
	blocklist[(*blocklist_length)++][1] = mb_y;
}

static void guess_mv(ERContext *s) {
	int (*blocklist)[2], (*next_blocklist)[2];
	uint8 *fixed;
	const ptrdiff_t mb_stride = s->mb_stride;
	const int mb_width = s->mb_width;
	int mb_height = s->mb_height;
	int i, num_avail;
	int mb_x, mb_y;
	ptrdiff_t mot_step, mot_stride;
	int blocklist_length, next_blocklist_length;

	if (s->last_pic.f && s->last_pic.f->data[0])
		mb_height = MIN(mb_height, (s->last_pic.f->height + 15) >> 4);
	if (s->next_pic.f && s->next_pic.f->data[0])
		mb_height = MIN(mb_height, (s->next_pic.f->height + 15) >> 4);

	blocklist = (int (*)[2])s->er_temp_buffer;
	next_blocklist = blocklist + s->mb_stride * s->mb_height;
	fixed = (uint8 *)(next_blocklist + s->mb_stride * s->mb_height);

	set_mv_strides(s, &mot_step, &mot_stride);

	num_avail = 0;
	if (s->last_pic.motion_val[0]) {
		if (false)
			(void)0;
		else
			(void)0;
	}
	for (i = 0; i < mb_width * mb_height; i++) {
		const int mb_xy = s->mb_index2xy[i];
		int f = 0;
		int error = s->error_status_table[mb_xy];

		if (IS_INTRA(s->cur_pic.mb_type[mb_xy]))
			f = MV_FROZEN; // intra // FIXME check
		if (!(error & ER_MV_ERROR))
			f = MV_FROZEN; // inter with undamaged MV

		fixed[mb_xy] = f;
		if (f == MV_FROZEN)
			num_avail++;
		else if (s->last_pic.f->data[0] && s->last_pic.motion_val[0]) {
			mb_y = mb_xy / s->mb_stride;
			mb_x = mb_xy % s->mb_stride;
			const int mot_index = (mb_x + mb_y * mot_stride) * mot_step;
			s->cur_pic.motion_val[0][mot_index][0] = s->last_pic.motion_val[0][mot_index][0];
			s->cur_pic.motion_val[0][mot_index][1] = s->last_pic.motion_val[0][mot_index][1];
			s->cur_pic.ref_index[0][4 * mb_xy] = s->last_pic.ref_index[0][4 * mb_xy];
		}
	}

	if ((!(s->avctx->error_concealment & FF_EC_GUESS_MVS)) || num_avail <= MAX(mb_width, mb_height) / 2) {
		for (mb_y = 0; mb_y < mb_height; mb_y++) {
			for (mb_x = 0; mb_x < s->mb_width; mb_x++) {
				const int mb_xy = mb_x + mb_y * s->mb_stride;
				int mv_dir = (s->last_pic.f && s->last_pic.f->data[0]) ? MV_DIR_FORWARD : MV_DIR_BACKWARD;

				if (IS_INTRA(s->cur_pic.mb_type[mb_xy]))
					continue;
				if (!(s->error_status_table[mb_xy] & ER_MV_ERROR))
					continue;

				s->mv[0][0][0] = 0;
				s->mv[0][0][1] = 0;
				s->decode_mb(s->opaque, 0, mv_dir, MV_TYPE_16X16, &s->mv, mb_x, mb_y, 0, 0);
			}
		}
		return;
	}

	blocklist_length = 0;
	for (mb_y = 0; mb_y < mb_height; mb_y++) {
		for (mb_x = 0; mb_x < mb_width; mb_x++) {
			const int mb_xy = mb_x + mb_y * mb_stride;
			if (fixed[mb_xy] == MV_FROZEN) {
				if (mb_x)
					add_blocklist(blocklist, &blocklist_length, fixed, mb_x - 1, mb_y, mb_xy - 1);
				if (mb_y)
					add_blocklist(blocklist, &blocklist_length, fixed, mb_x, mb_y - 1, mb_xy - mb_stride);
				if (mb_x + 1 < mb_width)
					add_blocklist(blocklist, &blocklist_length, fixed, mb_x + 1, mb_y, mb_xy + 1);
				if (mb_y + 1 < mb_height)
					add_blocklist(blocklist, &blocklist_length, fixed, mb_x, mb_y + 1, mb_xy + mb_stride);
			}
		}
	}

	for (;;) {
		int changed, pass, none_left;
		int blocklist_index;

		none_left = 1;
		changed = 1;
		for (pass = 0; (changed || pass < 2) && pass < 10; pass++) {
			changed = 0;
			for (blocklist_index = 0; blocklist_index < blocklist_length; blocklist_index++) {
				mb_x = blocklist[blocklist_index][0];
				mb_y = blocklist[blocklist_index][1];
				const int mb_xy = mb_x + mb_y * mb_stride;
				int mv_predictor[8][2];
				int ref[8];
				int pred_count;
				int j;
				int best_score;
				int best_pred;
				int mot_index;
				int prev_x, prev_y, prev_ref;

				if ((mb_x ^ mb_y ^ pass) & 1)
					continue;
				assert(fixed[mb_xy] != MV_FROZEN);

				assert(!IS_INTRA(s->cur_pic.mb_type[mb_xy]));
				assert(s->last_pic.f && s->last_pic.f->data[0]);

				j = 0;
				if (mb_x > 0)
					j |= fixed[mb_xy - 1];
				if (mb_x + 1 < mb_width)
					j |= fixed[mb_xy + 1];
				if (mb_y > 0)
					j |= fixed[mb_xy - mb_stride];
				if (mb_y + 1 < mb_height)
					j |= fixed[mb_xy + mb_stride];

				assert(j & MV_FROZEN);

				if (!(j & MV_CHANGED) && pass > 1)
					continue;

				none_left = 0;
				pred_count = 0;
				mot_index = (mb_x + mb_y * mot_stride) * mot_step;

				if (mb_x > 0 && fixed[mb_xy - 1] > 1) {
					mv_predictor[pred_count][0] = s->cur_pic.motion_val[0][mot_index - mot_step][0];
					mv_predictor[pred_count][1] = s->cur_pic.motion_val[0][mot_index - mot_step][1];
					ref[pred_count] = s->cur_pic.ref_index[0][4 * (mb_xy - 1)];
					pred_count++;
				}
				if (mb_x + 1 < mb_width && fixed[mb_xy + 1] > 1) {
					mv_predictor[pred_count][0] = s->cur_pic.motion_val[0][mot_index + mot_step][0];
					mv_predictor[pred_count][1] = s->cur_pic.motion_val[0][mot_index + mot_step][1];
					ref[pred_count] = s->cur_pic.ref_index[0][4 * (mb_xy + 1)];
					pred_count++;
				}
				if (mb_y > 0 && fixed[mb_xy - mb_stride] > 1) {
					mv_predictor[pred_count][0] = s->cur_pic.motion_val[0][mot_index - mot_stride * mot_step][0];
					mv_predictor[pred_count][1] = s->cur_pic.motion_val[0][mot_index - mot_stride * mot_step][1];
					ref[pred_count] = s->cur_pic.ref_index[0][4 * (mb_xy - s->mb_stride)];
					pred_count++;
				}
				if (mb_y + 1 < mb_height && fixed[mb_xy + mb_stride] > 1) {
					mv_predictor[pred_count][0] = s->cur_pic.motion_val[0][mot_index + mot_stride * mot_step][0];
					mv_predictor[pred_count][1] = s->cur_pic.motion_val[0][mot_index + mot_stride * mot_step][1];
					ref[pred_count] = s->cur_pic.ref_index[0][4 * (mb_xy + s->mb_stride)];
					pred_count++;
				}
				if (pred_count == 0)
					continue;

				if (pred_count > 1) {
					int sum_x = 0, sum_y = 0, sum_r = 0;
					int max_x, max_y, min_x, min_y, max_r, min_r;

					for (j = 0; j < pred_count; j++) {
						sum_x += mv_predictor[j][0];
						sum_y += mv_predictor[j][1];
						sum_r += ref[j];
						if (j && ref[j] != ref[j - 1])
							goto skip_mean_and_median;
					}

					/* mean */
					mv_predictor[pred_count][0] = sum_x / j;
					mv_predictor[pred_count][1] = sum_y / j;
					ref[pred_count] = sum_r / j;

					/* median */
					if (pred_count >= 3) {
						min_y = min_x = min_r = 99999;
						max_y = max_x = max_r = -99999;
					} else {
						min_x = min_y = max_x = max_y = min_r = max_r = 0;
					}
					for (j = 0; j < pred_count; j++) {
						max_x = MAX(max_x, mv_predictor[j][0]);
						max_y = MAX(max_y, mv_predictor[j][1]);
						max_r = MAX(max_r, ref[j]);
						min_x = MIN(min_x, mv_predictor[j][0]);
						min_y = MIN(min_y, mv_predictor[j][1]);
						min_r = MIN(min_r, ref[j]);
					}
					mv_predictor[pred_count + 1][0] = sum_x - max_x - min_x;
					mv_predictor[pred_count + 1][1] = sum_y - max_y - min_y;
					ref[pred_count + 1] = sum_r - max_r - min_r;

					if (pred_count == 4) {
						mv_predictor[pred_count + 1][0] /= 2;
						mv_predictor[pred_count + 1][1] /= 2;
						ref[pred_count + 1] /= 2;
					}
					pred_count += 2;
				}

			skip_mean_and_median:
				/* zero MV */
				mv_predictor[pred_count][0] = mv_predictor[pred_count][1] = ref[pred_count] = 0;
				pred_count++;

				prev_x = s->cur_pic.motion_val[0][mot_index][0];
				prev_y = s->cur_pic.motion_val[0][mot_index][1];
				prev_ref = s->cur_pic.ref_index[0][4 * mb_xy];

				/* last MV */
				mv_predictor[pred_count][0] = prev_x;
				mv_predictor[pred_count][1] = prev_y;
				ref[pred_count] = prev_ref;
				pred_count++;

				best_pred = 0;
				best_score = 256 * 256 * 256 * 64;
				for (j = 0; j < pred_count; j++) {
					int *linesize = s->cur_pic.f->linesize;
					int score = 0;
					uint8 *src = s->cur_pic.f->data[0] + mb_x * 16 + mb_y * 16 * linesize[0];

					s->cur_pic.motion_val[0][mot_index][0] = s->mv[0][0][0] = mv_predictor[j][0];
					s->cur_pic.motion_val[0][mot_index][1] = s->mv[0][0][1] = mv_predictor[j][1];

					// predictor intra or otherwise not available
					if (ref[j] < 0)
						continue;

					s->decode_mb(s->opaque, ref[j], MV_DIR_FORWARD, MV_TYPE_16X16, &s->mv, mb_x, mb_y, 0, 0);

					if (mb_x > 0 && fixed[mb_xy - 1] > 1) {
						int k;
						for (k = 0; k < 16; k++)
							score += ABS(src[k * linesize[0] - 1] - src[k * linesize[0]]);
					}
					if (mb_x + 1 < mb_width && fixed[mb_xy + 1] > 1) {
						int k;
						for (k = 0; k < 16; k++)
							score += ABS(src[k * linesize[0] + 15] - src[k * linesize[0] + 16]);
					}
					if (mb_y > 0 && fixed[mb_xy - mb_stride] > 1) {
						int k;
						for (k = 0; k < 16; k++)
							score += ABS(src[k - linesize[0]] - src[k]);
					}
					if (mb_y + 1 < mb_height && fixed[mb_xy + mb_stride] > 1) {
						int k;
						for (k = 0; k < 16; k++)
							score += ABS(src[k + linesize[0] * 15] - src[k + linesize[0] * 16]);
					}

					if (score <= best_score) { // <= will favor the last MV
						best_score = score;
						best_pred = j;
					}
				}
				s->mv[0][0][0] = mv_predictor[best_pred][0];
				s->mv[0][0][1] = mv_predictor[best_pred][1];

				for (i = 0; i < mot_step; i++)
					for (j = 0; j < mot_step; j++) {
						s->cur_pic.motion_val[0][mot_index + i + j * mot_stride][0] = s->mv[0][0][0];
						s->cur_pic.motion_val[0][mot_index + i + j * mot_stride][1] = s->mv[0][0][1];
					}

				s->decode_mb(s->opaque, ref[best_pred], MV_DIR_FORWARD, MV_TYPE_16X16, &s->mv, mb_x, mb_y, 0, 0);

				if (s->mv[0][0][0] != prev_x || s->mv[0][0][1] != prev_y) {
					fixed[mb_xy] = MV_CHANGED;
					changed++;
				} else
					fixed[mb_xy] = MV_UNCHANGED;
			}
		}

		if (none_left)
			return;

		next_blocklist_length = 0;

		for (blocklist_index = 0; blocklist_index < blocklist_length; blocklist_index++) {
			mb_x = blocklist[blocklist_index][0];
			mb_y = blocklist[blocklist_index][1];
			const int mb_xy = mb_x + mb_y * mb_stride;

			if (fixed[mb_xy] & (MV_CHANGED | MV_UNCHANGED | MV_FROZEN)) {
				fixed[mb_xy] = MV_FROZEN;
				if (mb_x > 0)
					add_blocklist(next_blocklist, &next_blocklist_length, fixed, mb_x - 1, mb_y, mb_xy - 1);
				if (mb_y > 0)
					add_blocklist(next_blocklist, &next_blocklist_length, fixed, mb_x, mb_y - 1, mb_xy - mb_stride);
				if (mb_x + 1 < mb_width)
					add_blocklist(next_blocklist, &next_blocklist_length, fixed, mb_x + 1, mb_y, mb_xy + 1);
				if (mb_y + 1 < mb_height)
					add_blocklist(next_blocklist, &next_blocklist_length, fixed, mb_x, mb_y + 1, mb_xy + mb_stride);
			}
		}
		assert(next_blocklist_length <= mb_height * mb_width);
		SWAP(blocklist_length, next_blocklist_length);
		SWAP(blocklist, next_blocklist);
	}
}

static int is_intra_more_likely(ERContext *s) {
	int is_intra_likely, i, j, undamaged_count, skip_amount, mb_x, mb_y;

	if (!s->last_pic.f || !s->last_pic.f->data[0])
		return 1; // no previous frame available -> use spatial prediction

	if (s->avctx->error_concealment & FF_EC_FAVOR_INTER)
		return 0;

	undamaged_count = 0;
	for (i = 0; i < s->mb_num; i++) {
		const int mb_xy = s->mb_index2xy[i];
		const int error = s->error_status_table[mb_xy];
		if (!((error & ER_DC_ERROR) && (error & ER_MV_ERROR)))
			undamaged_count++;
	}

	if (undamaged_count < 5)
		return 0; // almost all MBs damaged -> use temporal prediction

	skip_amount = MAX(undamaged_count / 50, 1); // check only up to 50 MBs
	is_intra_likely = 0;

	j = 0;
	for (mb_y = 0; mb_y < s->mb_height - 1; mb_y++) {
		for (mb_x = 0; mb_x < s->mb_width; mb_x++) {
			int error;
			const int mb_xy = mb_x + mb_y * s->mb_stride;

			error = s->error_status_table[mb_xy];
			if ((error & ER_DC_ERROR) && (error & ER_MV_ERROR))
				continue; // skip damaged

			j++;
			// skip a few to speed things up
			if ((j % skip_amount) != 0)
				continue;

			if (s->cur_pic.f->pict_type == AV_PICTURE_TYPE_I) {
				int *linesize = s->cur_pic.f->linesize;
				uint8 *mb_ptr = s->cur_pic.f->data[0] + mb_x * 16 + mb_y * 16 * linesize[0];
				uint8 *last_mb_ptr = s->last_pic.f->data[0] + mb_x * 16 + mb_y * 16 * linesize[0];

				if (false) {
					// FIXME
				} else {
					(void)0;
				}
				is_intra_likely += s->sad(NULL, last_mb_ptr, mb_ptr, linesize[0], 16);
				// FIXME need await_progress() here
				is_intra_likely -= s->sad(NULL, last_mb_ptr, last_mb_ptr + linesize[0] * 16, linesize[0], 16);
			} else {
				if (IS_INTRA(s->cur_pic.mb_type[mb_xy]))
					is_intra_likely++;
				else
					is_intra_likely--;
			}
		}
	}
	//      warning( "is_intra_likely: %d type:%d\n", is_intra_likely, s->pict_type);
	return is_intra_likely > 0;
}

void ff_er_frame_start(ERContext *s) {
	if (!s->avctx->error_concealment)
		return;

	memset(s->error_status_table, ER_MB_ERROR | VP_START | ER_MB_END, s->mb_stride * s->mb_height * sizeof(uint8));
	s->error_count = 3 * s->mb_num;
	s->error_occurred = 0;
}

static int er_supported(ERContext *s) {
	if (false || !s->cur_pic.f || s->cur_pic.field_picture)
		return 0;
	return 1;
}

/**
 * Add a slice.
 * @param endx   x component of the last macroblock, can be -1
 *               for the last of the previous line
 * @param status the status at the end (ER_MV_END, ER_AC_ERROR, ...), it is
 *               assumed that no earlier end or error of the same type occurred
 */
void ff_er_add_slice(ERContext *s, int startx, int starty, int endx, int endy, int status) {
	const int start_i = av_clip(startx + starty * s->mb_width, 0, s->mb_num - 1);
	const int end_i = av_clip(endx + endy * s->mb_width, 0, s->mb_num);
	const int start_xy = s->mb_index2xy[start_i];
	const int end_xy = s->mb_index2xy[end_i];
	int mask = -1;

	if (false)
		return;

	if (start_i > end_i || start_xy > end_xy) {
		warning("internal error, slice end before start\n");
		return;
	}

	if (!s->avctx->error_concealment)
		return;

	mask &= ~VP_START;
	if (status & (ER_AC_ERROR | ER_AC_END)) {
		mask &= ~(ER_AC_ERROR | ER_AC_END);
		s->error_count += start_i - end_i - 1;
	}
	if (status & (ER_DC_ERROR | ER_DC_END)) {
		mask &= ~(ER_DC_ERROR | ER_DC_END);
		s->error_count += start_i - end_i - 1;
	}
	if (status & (ER_MV_ERROR | ER_MV_END)) {
		mask &= ~(ER_MV_ERROR | ER_MV_END);
		s->error_count += start_i - end_i - 1;
	}

	if (status & ER_MB_ERROR) {
		s->error_occurred = 1;
		s->error_count = INT_MAX;
	}

	if (mask == ~0x7F) {
		memset(&s->error_status_table[start_xy], 0, (end_xy - start_xy) * sizeof(uint8));
	} else {
		int i;
		for (i = start_xy; i < end_xy; i++)
			s->error_status_table[i] &= mask;
	}

	if (end_i == s->mb_num)
		s->error_count = INT_MAX;
	else {
		s->error_status_table[end_xy] &= mask;
		s->error_status_table[end_xy] |= status;
	}

	s->error_status_table[start_xy] |= VP_START;

	if (start_xy > 0 && !(0 & FF_THREAD_SLICE) && er_supported(s) && 0 * s->mb_width < start_i) {
		int prev_status = s->error_status_table[s->mb_index2xy[start_i - 1]];

		prev_status &= ~VP_START;
		if (prev_status != (ER_MV_END | ER_DC_END | ER_AC_END)) {
			s->error_occurred = 1;
			s->error_count = INT_MAX;
		}
	}
}

void ff_er_frame_end(ERContext *s, int *decode_error_flags) {
	int *linesize = NULL;
	int i, mb_x, mb_y, error, error_type, dc_error, mv_error, ac_error;
	int distance;
	int threshold_part[4] = {100, 100, 100};
	int threshold = 50;
	int is_intra_likely;
	int guessed_mb_type;

	/* We do not support ER of field pictures yet,
	 * though it should not crash if enabled. */
	if (!s->avctx->error_concealment || !s->error_count || !er_supported(s)) {
		return;
	}
	linesize = s->cur_pic.f->linesize;

	if (s->last_pic.f) {
		if (s->last_pic.f->width != s->cur_pic.f->width || s->last_pic.f->height != s->cur_pic.f->height ||
			s->last_pic.f->format != s->cur_pic.f->format) {
			warning("Cannot use previous picture in error concealment\n");
			memset(&s->last_pic, 0, sizeof(s->last_pic));
		}
	}
	if (s->next_pic.f) {
		if (s->next_pic.f->width != s->cur_pic.f->width || s->next_pic.f->height != s->cur_pic.f->height ||
			s->next_pic.f->format != s->cur_pic.f->format) {
			warning("Cannot use next picture in error concealment\n");
			memset(&s->next_pic, 0, sizeof(s->next_pic));
		}
	}

	/* handle overlapping slices */
	for (error_type = 1; error_type <= 3; error_type++) {
		int end_ok = 0;

		for (i = s->mb_num - 1; i >= 0; i--) {
			const int mb_xy = s->mb_index2xy[i];
			error = s->error_status_table[mb_xy];

			if (error & (1 << error_type))
				end_ok = 1;
			if (error & (8 << error_type))
				end_ok = 1;

			if (!end_ok)
				s->error_status_table[mb_xy] |= 1 << error_type;

			if (error & VP_START)
				end_ok = 0;
		}
	}
	/* handle slices with partitions of different length */
	if (s->partitioned_frame) {
		int end_ok = 0;

		for (i = s->mb_num - 1; i >= 0; i--) {
			const int mb_xy = s->mb_index2xy[i];
			error = s->error_status_table[mb_xy];

			if (error & ER_AC_END)
				end_ok = 0;
			if ((error & ER_MV_END) || (error & ER_DC_END) || (error & ER_AC_ERROR))
				end_ok = 1;

			if (!end_ok)
				s->error_status_table[mb_xy] |= ER_AC_ERROR;

			if (error & VP_START)
				end_ok = 0;
		}
	}
	/* handle missing slices */
	if (1 & AV_EF_EXPLODE) {
		int end_ok = 1;

		// FIXME + 100 hack
		for (i = s->mb_num - 2; i >= s->mb_width + 100; i--) {
			const int mb_xy = s->mb_index2xy[i];
			int error1 = s->error_status_table[mb_xy];
			int error2 = s->error_status_table[s->mb_index2xy[i + 1]];

			if (error1 & VP_START)
				end_ok = 1;

			if (error2 == (VP_START | ER_MB_ERROR | ER_MB_END) && error1 != (VP_START | ER_MB_ERROR | ER_MB_END) &&
				((error1 & ER_AC_END) || (error1 & ER_DC_END) || (error1 & ER_MV_END))) {
				// end & uninit
				end_ok = 0;
			}

			if (!end_ok)
				s->error_status_table[mb_xy] |= ER_MB_ERROR;
		}
	}

	/* backward mark errors */
	distance = 9999999;
	for (error_type = 1; error_type <= 3; error_type++) {
		for (i = s->mb_num - 1; i >= 0; i--) {
			const int mb_xy = s->mb_index2xy[i];
			error = s->error_status_table[mb_xy];

			if (!s->mbskip_table || !s->mbskip_table[mb_xy]) // FIXME partition specific
				distance++;
			if (error & (1 << error_type))
				distance = 0;

			if (s->partitioned_frame) {
				if (distance < threshold_part[error_type - 1])
					s->error_status_table[mb_xy] |= 1 << error_type;
			} else {
				if (distance < threshold)
					s->error_status_table[mb_xy] |= 1 << error_type;
			}

			if (error & VP_START)
				distance = 9999999;
		}
	}

	/* forward mark errors */
	error = 0;
	for (i = 0; i < s->mb_num; i++) {
		const int mb_xy = s->mb_index2xy[i];
		int old_error = s->error_status_table[mb_xy];

		if (old_error & VP_START) {
			error = old_error & ER_MB_ERROR;
		} else {
			error |= old_error & ER_MB_ERROR;
			s->error_status_table[mb_xy] |= error;
		}
	}
	/* handle not partitioned case */
	if (!s->partitioned_frame) {
		for (i = 0; i < s->mb_num; i++) {
			const int mb_xy = s->mb_index2xy[i];
			error = s->error_status_table[mb_xy];
			if (error & ER_MB_ERROR)
				error |= ER_MB_ERROR;
			s->error_status_table[mb_xy] = error;
		}
	}

	dc_error = ac_error = mv_error = 0;
	for (i = 0; i < s->mb_num; i++) {
		const int mb_xy = s->mb_index2xy[i];
		error = s->error_status_table[mb_xy];
		if (error & ER_DC_ERROR)
			dc_error++;
		if (error & ER_AC_ERROR)
			ac_error++;
		if (error & ER_MV_ERROR)
			mv_error++;
	}
	warning("concealing %d DC, %d AC, %d MV errors in %c frame\n", dc_error, ac_error, mv_error,
			"?IPB???i"[s->cur_pic.f->pict_type]);

	if (decode_error_flags)
		*decode_error_flags |= FF_DECODE_ERROR_CONCEALMENT_ACTIVE;
	else
		s->cur_pic.f->decode_error_flags |= FF_DECODE_ERROR_CONCEALMENT_ACTIVE;

	is_intra_likely = is_intra_more_likely(s);

	/* set unknown mb-type to most likely */
	guessed_mb_type = is_intra_likely ? MB_TYPE_INTRA4x4 : (MB_TYPE_16x16 | (false ? MB_TYPE_L0 : MB_TYPE_FORWARD_MV));
	for (i = 0; i < s->mb_num; i++) {
		const int mb_xy = s->mb_index2xy[i];
		error = s->error_status_table[mb_xy];
		if (!((error & ER_DC_ERROR) && (error & ER_MV_ERROR)))
			continue;

		s->cur_pic.mb_type[mb_xy] = guessed_mb_type;
	}

	// change inter to intra blocks if no reference frames are available
	if (!(s->last_pic.f && s->last_pic.f->data[0]) && !(s->next_pic.f && s->next_pic.f->data[0]))
		for (i = 0; i < s->mb_num; i++) {
			const int mb_xy = s->mb_index2xy[i];
			if (!IS_INTRA(s->cur_pic.mb_type[mb_xy]))
				s->cur_pic.mb_type[mb_xy] = MB_TYPE_INTRA4x4;
		}

	/* handle inter blocks with damaged AC */
	for (mb_y = 0; mb_y < s->mb_height; mb_y++) {
		for (mb_x = 0; mb_x < s->mb_width; mb_x++) {
			const int mb_xy = mb_x + mb_y * s->mb_stride;
			const int mb_type = s->cur_pic.mb_type[mb_xy];
			const int dir = !(s->last_pic.f && s->last_pic.f->data[0]);
			const int mv_dir = dir ? MV_DIR_BACKWARD : MV_DIR_FORWARD;
			int mv_type;

			error = s->error_status_table[mb_xy];

			if (IS_INTRA(mb_type))
				continue; // intra
			if (error & ER_MV_ERROR)
				continue; // inter with damaged MV
			if (!(error & ER_AC_ERROR))
				continue; // undamaged inter

			if (IS_8X8(mb_type)) {
				int mb_index = mb_x * 2 + mb_y * 2 * s->b8_stride;
				int j;
				mv_type = MV_TYPE_8X8;
				for (j = 0; j < 4; j++) {
					s->mv[0][j][0] = s->cur_pic.motion_val[dir][mb_index + (j & 1) + (j >> 1) * s->b8_stride][0];
					s->mv[0][j][1] = s->cur_pic.motion_val[dir][mb_index + (j & 1) + (j >> 1) * s->b8_stride][1];
				}
			} else {
				mv_type = MV_TYPE_16X16;
				s->mv[0][0][0] = s->cur_pic.motion_val[dir][mb_x * 2 + mb_y * 2 * s->b8_stride][0];
				s->mv[0][0][1] = s->cur_pic.motion_val[dir][mb_x * 2 + mb_y * 2 * s->b8_stride][1];
			}

			s->decode_mb(s->opaque, 0 /* FIXME H.264 partitioned slices need this set */, mv_dir, mv_type, &s->mv, mb_x,
						 mb_y, 0, 0);
		}
	}

	/* guess MVs */
	if (s->cur_pic.f->pict_type == AV_PICTURE_TYPE_B) {
		for (mb_y = 0; mb_y < s->mb_height; mb_y++) {
			for (mb_x = 0; mb_x < s->mb_width; mb_x++) {
				int xy = mb_x * 2 + mb_y * 2 * s->b8_stride;
				const int mb_xy = mb_x + mb_y * s->mb_stride;
				const int mb_type = s->cur_pic.mb_type[mb_xy];
				int mv_dir = MV_DIR_FORWARD | MV_DIR_BACKWARD;

				error = s->error_status_table[mb_xy];

				if (IS_INTRA(mb_type))
					continue;
				if (!(error & ER_MV_ERROR))
					continue; // inter with undamaged MV
				if (!(error & ER_AC_ERROR))
					continue; // undamaged inter

				if (!(s->last_pic.f && s->last_pic.f->data[0]))
					mv_dir &= ~MV_DIR_FORWARD;
				if (!(s->next_pic.f && s->next_pic.f->data[0]))
					mv_dir &= ~MV_DIR_BACKWARD;

				if (s->pp_time) {
					int time_pp = s->pp_time;
					int time_pb = s->pb_time;

					assert(true);
					(void)0;

					s->mv[0][0][0] = s->next_pic.motion_val[0][xy][0] * time_pb / time_pp;
					s->mv[0][0][1] = s->next_pic.motion_val[0][xy][1] * time_pb / time_pp;
					s->mv[1][0][0] = s->next_pic.motion_val[0][xy][0] * (time_pb - time_pp) / time_pp;
					s->mv[1][0][1] = s->next_pic.motion_val[0][xy][1] * (time_pb - time_pp) / time_pp;
				} else {
					s->mv[0][0][0] = 0;
					s->mv[0][0][1] = 0;
					s->mv[1][0][0] = 0;
					s->mv[1][0][1] = 0;
				}

				s->decode_mb(s->opaque, 0, mv_dir, MV_TYPE_16X16, &s->mv, mb_x, mb_y, 0, 0);
			}
		}
	} else
		guess_mv(s);

	/* fill DC for inter blocks */
	for (mb_y = 0; mb_y < s->mb_height; mb_y++) {
		for (mb_x = 0; mb_x < s->mb_width; mb_x++) {
			int dc, dcu, dcv, y, n;
			int16 *dc_ptr;
			uint8 *dest_y, *dest_cb, *dest_cr;
			const int mb_xy = mb_x + mb_y * s->mb_stride;
			const int mb_type = s->cur_pic.mb_type[mb_xy];

			// error = s->error_status_table[mb_xy];

			if (IS_INTRA(mb_type) && s->partitioned_frame)
				continue;
			// if (error & ER_MV_ERROR)
			//     continue; // inter data damaged FIXME is this good?

			dest_y = s->cur_pic.f->data[0] + mb_x * 16 + mb_y * 16 * linesize[0];
			dest_cb = s->cur_pic.f->data[1] + mb_x * 8 + mb_y * 8 * linesize[1];
			dest_cr = s->cur_pic.f->data[2] + mb_x * 8 + mb_y * 8 * linesize[2];

			dc_ptr = &s->dc_val[0][mb_x * 2 + mb_y * 2 * s->b8_stride];
			for (n = 0; n < 4; n++) {
				dc = 0;
				for (y = 0; y < 8; y++) {
					int x;
					for (x = 0; x < 8; x++)
						dc += dest_y[x + (n & 1) * 8 + (y + (n >> 1) * 8) * linesize[0]];
				}
				dc_ptr[(n & 1) + (n >> 1) * s->b8_stride] = (dc + 4) >> 3;
			}

			if (!s->cur_pic.f->data[2])
				continue;

			dcu = dcv = 0;
			for (y = 0; y < 8; y++) {
				int x;
				for (x = 0; x < 8; x++) {
					dcu += dest_cb[x + y * linesize[1]];
					dcv += dest_cr[x + y * linesize[2]];
				}
			}
			s->dc_val[1][mb_x + mb_y * s->mb_stride] = (dcu + 4) >> 3;
			s->dc_val[2][mb_x + mb_y * s->mb_stride] = (dcv + 4) >> 3;
		}
	}
	/* guess DC for damaged blocks */
	guess_dc(s, s->dc_val[0], s->mb_width * 2, s->mb_height * 2, s->b8_stride, 1);
	guess_dc(s, s->dc_val[1], s->mb_width, s->mb_height, s->mb_stride, 0);
	guess_dc(s, s->dc_val[2], s->mb_width, s->mb_height, s->mb_stride, 0);

	/* filter luma DC */
	filter181(s->dc_val[0], s->mb_width * 2, s->mb_height * 2, s->b8_stride);

	/* render DC only intra */
	for (mb_y = 0; mb_y < s->mb_height; mb_y++) {
		for (mb_x = 0; mb_x < s->mb_width; mb_x++) {
			uint8 *dest_y, *dest_cb, *dest_cr;
			const int mb_xy = mb_x + mb_y * s->mb_stride;
			const int mb_type = s->cur_pic.mb_type[mb_xy];

			error = s->error_status_table[mb_xy];

			if (IS_INTER(mb_type))
				continue;
			if (!(error & ER_AC_ERROR))
				continue; // undamaged

			dest_y = s->cur_pic.f->data[0] + mb_x * 16 + mb_y * 16 * linesize[0];
			dest_cb = s->cur_pic.f->data[1] + mb_x * 8 + mb_y * 8 * linesize[1];
			dest_cr = s->cur_pic.f->data[2] + mb_x * 8 + mb_y * 8 * linesize[2];
			if (!s->cur_pic.f->data[2])
				dest_cb = dest_cr = NULL;

			put_dc(s, dest_y, dest_cb, dest_cr, mb_x, mb_y);
		}
	}

	if (s->avctx->error_concealment & FF_EC_DEBLOCK) {
		/* filter horizontal block boundaries */
		h_block_filter(s, s->cur_pic.f->data[0], s->mb_width * 2, s->mb_height * 2, linesize[0], 1);

		/* filter vertical block boundaries */
		v_block_filter(s, s->cur_pic.f->data[0], s->mb_width * 2, s->mb_height * 2, linesize[0], 1);

		if (s->cur_pic.f->data[2]) {
			h_block_filter(s, s->cur_pic.f->data[1], s->mb_width, s->mb_height, linesize[1], 0);
			h_block_filter(s, s->cur_pic.f->data[2], s->mb_width, s->mb_height, linesize[2], 0);
			v_block_filter(s, s->cur_pic.f->data[1], s->mb_width, s->mb_height, linesize[1], 0);
			v_block_filter(s, s->cur_pic.f->data[2], s->mb_width, s->mb_height, linesize[2], 0);
		}
	}

	/* clean a few tables */
	for (i = 0; i < s->mb_num; i++) {
		const int mb_xy = s->mb_index2xy[i];
		error = s->error_status_table[mb_xy];

		if (s->mbskip_table && s->cur_pic.f->pict_type != AV_PICTURE_TYPE_B &&
			(error & (ER_DC_ERROR | ER_MV_ERROR | ER_AC_ERROR))) {
			s->mbskip_table[mb_xy] = 0;
		}
		if (s->mbintra_table)
			s->mbintra_table[mb_xy] = 1;
	}

	memset(&s->cur_pic, 0, sizeof(Picture));
	memset(&s->last_pic, 0, sizeof(Picture));
	memset(&s->next_pic, 0, sizeof(Picture));
}

#undef IS_8X8
#undef IS_INTER
#undef IS_INTRA
#undef MV_CHANGED
#undef MV_FROZEN
#undef MV_LISTED
#undef MV_UNCHANGED

enum { FMT_H263 = 2, FMT_H261 = 1, FMT_MPEG1 = 0, CONFIG_SMALL = 0 };
static inline int ff_h263_round_chroma(int x) {
	// FIXME static or not?
	static const uint8 h263_chroma_roundtab[16] = {
		//  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
		0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1,
	};
	return h263_chroma_roundtab[x & 0xf] + (x >> 3);
}

static inline int hpel_motion(PictureState *s, uint8 *dest, uint8 *src, int src_x, int src_y, const PixelOp *pix_op,
							  int motion_x, int motion_y) {
	int dxy = 0;
	int emu = 0;

	src_x += motion_x >> 1;
	src_y += motion_y >> 1;

	/* WARNING: do no forget half pels */
	src_x = av_clip(src_x, -16, s->width); // FIXME unneeded for emu?
	if (src_x != s->width)
		dxy |= motion_x & 1;
	src_y = av_clip(src_y, -16, s->height);
	if (src_y != s->height)
		dxy |= (motion_y & 1) << 1;
	src += src_y * s->linesize + src_x;

	if ((unsigned)src_x >= (unsigned)MAX(s->h_edge_pos - (motion_x & 1) - 7, 0) ||
		(unsigned)src_y >= (unsigned)MAX(s->v_edge_pos - (motion_y & 1) - 7, 0)) {
		s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer, src, s->linesize, s->linesize, 9, 9, src_x, src_y,
								 s->h_edge_pos, s->v_edge_pos);
		src = s->sc.edge_emu_buffer;
		emu = 1;
	}
	pix_op[dxy](dest, src, s->linesize, 8);
	return emu;
}

static inline void mpeg_motion_internal(PictureState *s, uint8 *dest_y, uint8 *dest_cb, uint8 *dest_cr, int field_based,
										int bottom_field, int field_select, uint8 *const *ref_picture,
										const PixelOp (*pix_op)[4], int motion_x, int motion_y, int h, int is_mpeg12,
										int is_16x8, int mb_y) {
	const uint8 *ptr_y, *ptr_cb, *ptr_cr;
	int dxy, uvdxy, mx, my, src_x, src_y, uvsrc_x, uvsrc_y, v_edge_pos, block_y_half;
	ptrdiff_t uvlinesize, linesize;

	v_edge_pos = s->v_edge_pos >> field_based;
	linesize = s->cur_pic.linesize[0] << field_based;
	uvlinesize = s->cur_pic.linesize[1] << field_based;
	block_y_half = (field_based | is_16x8);

	dxy = ((motion_y & 1) << 1) | (motion_x & 1);
	src_x = s->mb_x * 16 + (motion_x >> 1);
	src_y = (mb_y << (4 - block_y_half)) + (motion_y >> 1);

	if (!is_mpeg12) {
		if ((0) && field_based) {
			mx = (motion_x >> 1) | (motion_x & 1);
			my = motion_y >> 1;
			uvdxy = ((my & 1) << 1) | (mx & 1);
			uvsrc_x = s->mb_x * 8 + (mx >> 1);
			uvsrc_y = (mb_y << (3 - block_y_half)) + (my >> 1);
		} else {
			uvdxy = dxy | (motion_y & 2) | ((motion_x & 2) >> 1);
			uvsrc_x = src_x >> 1;
			uvsrc_y = src_y >> 1;
		}
		// Even chroma mv's are full pel in H261
	} else if ((!CONFIG_SMALL && !is_mpeg12) || (CONFIG_SMALL && FMT_H263 == FMT_H261)) {
		assert(FMT_H263 == FMT_H261);
		mx = motion_x / 4;
		my = motion_y / 4;
		uvdxy = 0;
		uvsrc_x = s->mb_x * 8 + mx;
		uvsrc_y = mb_y * 8 + my;
	} else {
		assert(FMT_H263 == FMT_MPEG1);
		if (1) {
			mx = motion_x / 2;
			my = motion_y / 2;
			uvdxy = ((my & 1) << 1) | (mx & 1);
			uvsrc_x = s->mb_x * 8 + (mx >> 1);
			uvsrc_y = (mb_y << (3 - block_y_half)) + (my >> 1);
		} else {
			if (s->chroma_x_shift) {
				// Chroma422
				mx = motion_x / 2;
				uvdxy = ((motion_y & 1) << 1) | (mx & 1);
				uvsrc_x = s->mb_x * 8 + (mx >> 1);
				uvsrc_y = src_y;
			} else {
				// Chroma444
				uvdxy = dxy;
				uvsrc_x = src_x;
				uvsrc_y = src_y;
			}
		}
	}

	ptr_y = ref_picture[0] + src_y * linesize + src_x;
	ptr_cb = ref_picture[1] + uvsrc_y * uvlinesize + uvsrc_x;
	ptr_cr = ref_picture[2] + uvsrc_y * uvlinesize + uvsrc_x;

	if ((unsigned)src_x >= (unsigned)MAX(s->h_edge_pos - (motion_x & 1) - 15, 0) ||
		(unsigned)src_y >= (unsigned)MAX(v_edge_pos - (motion_y & 1) - h + 1, 0)) {
		if (is_mpeg12 || (CONFIG_SMALL && (false || false))) {

			return;
		}
		src_y = (unsigned)src_y << field_based;
		s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer, ptr_y, s->linesize, s->linesize, 17, 17 + field_based, src_x,
								 src_y, s->h_edge_pos, s->v_edge_pos);
		ptr_y = s->sc.edge_emu_buffer;
		if (!CONFIG_GRAY || !(s->avctx->flags & AV_CODEC_FLAG_GRAY)) {
			uint8 *ubuf = s->sc.edge_emu_buffer + 18 * s->linesize;
			uint8 *vbuf = ubuf + 10 * s->uvlinesize;
			if (0)
				vbuf -= s->uvlinesize;
			uvsrc_y = (unsigned)uvsrc_y << field_based;
			s->vdsp.emulated_edge_mc(ubuf, ptr_cb, s->uvlinesize, s->uvlinesize, 9, 9 + field_based, uvsrc_x, uvsrc_y,
									 s->h_edge_pos >> 1, s->v_edge_pos >> 1);
			s->vdsp.emulated_edge_mc(vbuf, ptr_cr, s->uvlinesize, s->uvlinesize, 9, 9 + field_based, uvsrc_x, uvsrc_y,
									 s->h_edge_pos >> 1, s->v_edge_pos >> 1);
			ptr_cb = ubuf;
			ptr_cr = vbuf;
		}
	}

	/* FIXME use this for field pix too instead of the obnoxious hack which
	 * changes picture.data */
	if (bottom_field) {
		dest_y += s->linesize;
		dest_cb += s->uvlinesize;
		dest_cr += s->uvlinesize;
	}

	if (field_select) {
		ptr_y += s->linesize;
		ptr_cb += s->uvlinesize;
		ptr_cr += s->uvlinesize;
	}

	pix_op[0][dxy](dest_y, ptr_y, linesize, h);

	if (!CONFIG_GRAY || !(s->avctx->flags & AV_CODEC_FLAG_GRAY)) {
		pix_op[s->chroma_x_shift][uvdxy](dest_cb, ptr_cb, uvlinesize, h >> 1);
		pix_op[s->chroma_x_shift][uvdxy](dest_cr, ptr_cr, uvlinesize, h >> 1);
	}
}

static inline void qpel_motion(PictureState *s, uint8 *dest_y, uint8 *dest_cb, uint8 *dest_cr, int field_based,
							   int bottom_field, int field_select, uint8 *const *ref_picture,
							   const PixelOp (*pix_op)[4], const QpelOp (*qpix_op)[16], int motion_x, int motion_y,
							   int h) {
	const uint8 *ptr_y, *ptr_cb, *ptr_cr;
	int dxy, uvdxy, mx, my, src_x, src_y, uvsrc_x, uvsrc_y, v_edge_pos;
	ptrdiff_t linesize, uvlinesize;

	dxy = ((motion_y & 3) << 2) | (motion_x & 3);

	src_x = s->mb_x * 16 + (motion_x >> 2);
	src_y = s->mb_y * (16 >> field_based) + (motion_y >> 2);

	v_edge_pos = s->v_edge_pos >> field_based;
	linesize = s->linesize << field_based;
	uvlinesize = s->uvlinesize << field_based;

	if (field_based) {
		mx = motion_x / 2;
		my = motion_y >> 1;
	} else if (0) {
		static const int rtab[8] = {0, 0, 1, 1, 0, 0, 0, 1};
		mx = (motion_x >> 1) + rtab[motion_x & 7];
		my = (motion_y >> 1) + rtab[motion_y & 7];
	} else if (0) {
		mx = (motion_x >> 1) | (motion_x & 1);
		my = (motion_y >> 1) | (motion_y & 1);
	} else {
		mx = motion_x / 2;
		my = motion_y / 2;
	}
	mx = (mx >> 1) | (mx & 1);
	my = (my >> 1) | (my & 1);

	uvdxy = (mx & 1) | ((my & 1) << 1);
	mx >>= 1;
	my >>= 1;

	uvsrc_x = s->mb_x * 8 + mx;
	uvsrc_y = s->mb_y * (8 >> field_based) + my;

	ptr_y = ref_picture[0] + src_y * linesize + src_x;
	ptr_cb = ref_picture[1] + uvsrc_y * uvlinesize + uvsrc_x;
	ptr_cr = ref_picture[2] + uvsrc_y * uvlinesize + uvsrc_x;

	if ((unsigned)src_x >= (unsigned)MAX(s->h_edge_pos - (motion_x & 3) - 15, 0) ||
		(unsigned)src_y >= (unsigned)MAX(v_edge_pos - (motion_y & 3) - h + 1, 0)) {
		s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer, ptr_y, s->linesize, s->linesize, 17, 17 + field_based, src_x,
								 src_y * (1 << field_based), s->h_edge_pos, s->v_edge_pos);
		ptr_y = s->sc.edge_emu_buffer;
		if (!CONFIG_GRAY || !(s->avctx->flags & AV_CODEC_FLAG_GRAY)) {
			uint8 *ubuf = s->sc.edge_emu_buffer + 18 * s->linesize;
			uint8 *vbuf = ubuf + 10 * s->uvlinesize;
			if (0)
				vbuf -= s->uvlinesize;
			s->vdsp.emulated_edge_mc(ubuf, ptr_cb, s->uvlinesize, s->uvlinesize, 9, 9 + field_based, uvsrc_x,
									 uvsrc_y * (1 << field_based), s->h_edge_pos >> 1, s->v_edge_pos >> 1);
			s->vdsp.emulated_edge_mc(vbuf, ptr_cr, s->uvlinesize, s->uvlinesize, 9, 9 + field_based, uvsrc_x,
									 uvsrc_y * (1 << field_based), s->h_edge_pos >> 1, s->v_edge_pos >> 1);
			ptr_cb = ubuf;
			ptr_cr = vbuf;
		}
	}

	if (!field_based)
		qpix_op[0][dxy](dest_y, ptr_y, linesize);
	else {
		if (bottom_field) {
			dest_y += s->linesize;
			dest_cb += s->uvlinesize;
			dest_cr += s->uvlinesize;
		}

		if (field_select) {
			ptr_y += s->linesize;
			ptr_cb += s->uvlinesize;
			ptr_cr += s->uvlinesize;
		}
		// damn interlaced mode
		// FIXME boundary mirroring is not exactly correct here
		qpix_op[1][dxy](dest_y, ptr_y, linesize);
		qpix_op[1][dxy](dest_y + 8, ptr_y + 8, linesize);
	}
	if (!CONFIG_GRAY || !(s->avctx->flags & AV_CODEC_FLAG_GRAY)) {
		pix_op[1][uvdxy](dest_cr, ptr_cr, uvlinesize, h >> 1);
		pix_op[1][uvdxy](dest_cb, ptr_cb, uvlinesize, h >> 1);
	}
}

/**
 * H.263 chroma 4mv motion compensation.
 */
static void chroma_4mv_motion(PictureState *s, uint8 *dest_cb, uint8 *dest_cr, uint8 *const *ref_picture,
							  const PixelOp *pix_op, int mx, int my) {
	const uint8 *ptr;
	int src_x, src_y, dxy, emu = 0;
	ptrdiff_t offset;

	/* In case of 8X8, we construct a single chroma motion vector
	 * with a special rounding */
	mx = ff_h263_round_chroma(mx);
	my = ff_h263_round_chroma(my);

	dxy = ((my & 1) << 1) | (mx & 1);
	mx >>= 1;
	my >>= 1;

	src_x = s->mb_x * 8 + mx;
	src_y = s->mb_y * 8 + my;
	src_x = av_clip(src_x, -8, (s->width >> 1));
	if (src_x == (s->width >> 1))
		dxy &= ~1;
	src_y = av_clip(src_y, -8, (s->height >> 1));
	if (src_y == (s->height >> 1))
		dxy &= ~2;

	offset = src_y * s->uvlinesize + src_x;
	ptr = ref_picture[1] + offset;
	if ((unsigned)src_x >= (unsigned)MAX((s->h_edge_pos >> 1) - (dxy & 1) - 7, 0) ||
		(unsigned)src_y >= (unsigned)MAX((s->v_edge_pos >> 1) - (dxy >> 1) - 7, 0)) {
		s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer, ptr, s->uvlinesize, s->uvlinesize, 9, 9, src_x, src_y,
								 s->h_edge_pos >> 1, s->v_edge_pos >> 1);
		ptr = s->sc.edge_emu_buffer;
		emu = 1;
	}
	pix_op[dxy](dest_cb, ptr, s->uvlinesize, 8);

	ptr = ref_picture[2] + offset;
	if (emu) {
		s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer, ptr, s->uvlinesize, s->uvlinesize, 9, 9, src_x, src_y,
								 s->h_edge_pos >> 1, s->v_edge_pos >> 1);
		ptr = s->sc.edge_emu_buffer;
	}
	pix_op[dxy](dest_cr, ptr, s->uvlinesize, 8);
}

static inline void apply_8x8(PictureState *s, uint8 *dest_y, uint8 *dest_cb, uint8 *dest_cr, int dir,
							 uint8 *const *ref_picture, const QpelOp (*qpix_op)[16], const PixelOp (*pix_op)[4]) {
	int dxy, mx, my, src_x, src_y;
	int i;
	int mb_x = s->mb_x;
	int mb_y = s->mb_y;
	uint8 *dest;
	const uint8 *ptr;

	mx = 0;
	my = 0;
	if (s->quarter_sample) {
		for (i = 0; i < 4; i++) {
			int motion_x = s->mv[dir][i][0];
			int motion_y = s->mv[dir][i][1];

			dxy = ((motion_y & 3) << 2) | (motion_x & 3);
			src_x = mb_x * 16 + (motion_x >> 2) + (i & 1) * 8;
			src_y = mb_y * 16 + (motion_y >> 2) + (i >> 1) * 8;

			/* WARNING: do no forget half pels */
			src_x = av_clip(src_x, -16, s->width);
			if (src_x == s->width)
				dxy &= ~3;
			src_y = av_clip(src_y, -16, s->height);
			if (src_y == s->height)
				dxy &= ~12;

			ptr = ref_picture[0] + (src_y * s->linesize) + (src_x);
			if ((unsigned)src_x >= (unsigned)MAX(s->h_edge_pos - (motion_x & 3) - 7, 0) ||
				(unsigned)src_y >= (unsigned)MAX(s->v_edge_pos - (motion_y & 3) - 7, 0)) {
				s->vdsp.emulated_edge_mc(s->sc.edge_emu_buffer, ptr, s->linesize, s->linesize, 9, 9, src_x, src_y,
										 s->h_edge_pos, s->v_edge_pos);
				ptr = s->sc.edge_emu_buffer;
			}
			dest = dest_y + ((i & 1) * 8) + (i >> 1) * 8 * s->linesize;
			qpix_op[1][dxy](dest, ptr, s->linesize);

			mx += s->mv[dir][i][0] / 2;
			my += s->mv[dir][i][1] / 2;
		}
	} else {
		for (i = 0; i < 4; i++) {
			hpel_motion(s, dest_y + ((i & 1) * 8) + (i >> 1) * 8 * s->linesize, ref_picture[0], mb_x * 16 + (i & 1) * 8,
						mb_y * 16 + (i >> 1) * 8, pix_op[1], s->mv[dir][i][0], s->mv[dir][i][1]);

			mx += s->mv[dir][i][0];
			my += s->mv[dir][i][1];
		}
	}

	if (!CONFIG_GRAY || !(s->avctx->flags & AV_CODEC_FLAG_GRAY))
		chroma_4mv_motion(s, dest_cb, dest_cr, ref_picture, pix_op[1], mx, my);
}

static void concealMB(void *opaque, int ref, int dir, int type, int (*mv)[2][4][2], int x, int y, int intra,
					  int skipped) {
	PictureState *s = static_cast<PictureState *>(opaque);
	assert(!intra);
	s->mb_x = x;
	s->mb_y = y;
	s->mb_intra = intra;
	memcpy(s->mv, mv, sizeof(*mv));
	uint8 *dest[3] = {s->cur_pic.data[0] + y * 16 * s->linesize + x * 16,
					  s->cur_pic.data[1] + y * 8 * s->uvlinesize + x * 8,
					  s->cur_pic.data[2] + y * 8 * s->uvlinesize + x * 8};
	const PixelOp(*pixels)[4] = s->hdsp.put_pixels_tab;
	const QpelOp(*qpel)[16] = s->qdsp.put_qpel_pixels_tab;
	for (int d = 0; d < 2; ++d) {
		if (!(dir & (1 << d)))
			continue;
		Picture &p = d ? s->next_pic : s->last_pic;
		if (type == 1)
			apply_8x8(s, dest[0], dest[1], dest[2], d, p.data, qpel, pixels);
		else if (s->quarter_sample)
			qpel_motion(s, dest[0], dest[1], dest[2], 0, 0, 0, p.data, pixels, qpel, s->mv[d][0][0], s->mv[d][0][1],
						16);
		else
			mpeg_motion_internal(s, dest[0], dest[1], dest[2], 0, 0, 0, p.data, pixels, s->mv[d][0][0], s->mv[d][0][1],
								 16, 0, 0, y);
		pixels = s->hdsp.avg_pixels_tab;
		qpel = s->qdsp.avg_qpel_pixels_tab;
	}
}

static int sad16(void *, const uint8 *a, const uint8 *b, ptrdiff_t stride, int h) {
	int sum = 0;
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < 16; ++x)
			sum += ABS(int(a[x]) - int(b[x]));
		a += stride;
		b += stride;
	}
	return sum;
}

void initializeER(PictureState *s) {
	s->er.avctx = s->avctx;
	s->er.sad = sad16;
	s->er.decode_mb = concealMB;
	s->er.opaque = s;
	s->er.mb_num = s->mb_width * s->mb_height;
	s->er.mb_width = s->mb_width;
	s->er.mb_height = s->mb_height;
	s->er.mb_stride = s->mb_stride;
	s->er.b8_stride = s->b8_stride;
	s->er.dc_val[0] = s->dc_val;
	s->er.dc_val[1] = s->dc_val + s->b8_stride * 2 * FFALIGN(s->mb_height, 2) + s->mb_stride;
	s->er.dc_val[2] = s->er.dc_val[1] + s->mb_stride * (FFALIGN(s->mb_height, 2) + 1);
	s->er.mbskip_table = s->mbskip_table;
	initQpel(&s->qdsp);
}

// Packet and slice storage

// Owns padded, unescaped data and its readers for one packet segment.
class Slice {
public:
	Common::Array<uint8> data;
	Common::ScopedPtr<Common::MemoryReadStream> input;
	Common::ScopedPtr<Common::BitStream8MSB> stream;
	Bits bits;
	int row;
	Slice(const uint8 *src, uint32 size, bool unescape) : input(nullptr), stream(nullptr), row(0) {
		for (uint32 i = 0; i < size; ++i) {
			if (unescape && i >= 2 && i + 1 < size && src[i] == 3 && src[i - 1] == 0 && src[i - 2] == 0 &&
				src[i + 1] < 4)
				continue;
			data.push_back(src[i]);
		}
		int bytes = data.size();
		data.resize(bytes + 64);
		memset(data.data() + bytes, 0, 64);
		input.reset(new Common::MemoryReadStream(data.data(), data.size()));
		stream.reset(new Common::BitStream8MSB(input.get()));
		bits.stream = stream.get();
		bits.size_in_bits = bytes * 8;
	}

private:
	Slice(const Slice &) = delete;
	Slice &operator=(const Slice &) = delete;
};
static uint32 nextMarker(const uint8 *p, uint32 size, uint32 from) {
	for (uint32 i = from; i + 3 < size; ++i)
		if (p[i] == 0 && p[i + 1] == 0 && p[i + 2] == 1)
			return i;
	return size;
}

Decoder::Decoder(int w, int h, bool advanced)
	: _v(new VC1Context()), _settings{}, _pictures{}, _planes{}, _advanced(advanced), _ready(false), _failed(false),
	  _pending(false) {
	_v->width = w;
	_v->height = h;
	_v->s.avctx = &_settings;
	static const bool initialized = (initTables(), true);
	(void)initialized;
	_v->pq = 255;
	initWMV3DSP(&_v->vc1dsp);
	initSupport(&_v->s, &_v->h264chroma);
}

Decoder::~Decoder() {
	clearPictures();
	delete _v;
}

void Decoder::clearPictures() {
	if (_v->x8) {
		endX8(_v->x8);
		delete _v->x8;
		_v->x8 = nullptr;
	}
	for (uint i = 0; i < _allocations.size(); ++i)
		delete[] _allocations[i];
	_allocations.clear();
	memset(_pictures, 0, sizeof(_pictures));
	memset(_planes, 0, sizeof(_planes));
	_v->s.cur_pic = {};
	_v->s.last_pic = {};
	_v->s.next_pic = {};
	_pending = false;
}

bool Decoder::readHeader(const uint8 *data, uint32 size, bool entry) {
	Common::MemoryReadStream input(data, size);
	Common::BitStream8MSB bits(input);
	return (entry ? readVC1Entry(_v, &bits) : readVC1Sequence(_v, &bits)) == 0;
}

bool Decoder::initialize(const uint8 *data, uint32 size) {
	if (_ready || !size || size > 1024 * 1024)
		return false;
	if (!_advanced) {
		if (!readHeader(data, size, false))
			return false;
	} else {
		bool sequence = false, entry = false;
		uint32 start = nextMarker(data, size, 0);
		while (start < size) {
			uint32 end = nextMarker(data, size, start + 4);
			Slice chunk(data + start + 4, end - start - 4, true);
			if (data[start + 3] == 15) {
				if (!readHeader(chunk.data.data(), chunk.bits.size_in_bits / 8, false))
					return false;
				sequence = true;
			}
			if (data[start + 3] == 14) {
				if (!readHeader(chunk.data.data(), chunk.bits.size_in_bits / 8, true))
					return false;
				entry = true;
			}
			start = end;
		}
		if (!sequence || !entry)
			return false;
	}
	if (_v->width <= 0 || _v->height <= 0 || _v->width > 8192 || _v->height > 8192)
		return false;
	allocatePictures();
	_ready = true;
	return true;
}
// Picture allocation and initialization

// Keep allocation order explicit: predictors, reference pictures, concealment, transforms.
void Decoder::allocatePictures() {
	initializePictureLayout();
	allocatePredictors();
	allocateReferencePictures();
	initializeConcealment();
	initializeTransforms();
}

void Decoder::initializePictureLayout() {
	VC1Context *v = _v;
	PictureState *s = &v->s;
	s->height = v->height;
	s->width = v->width;
	_settings.coded_width = v->width;
	_settings.coded_height = v->height;
	s->mb_width = (v->width + 15) >> 4;
	s->mb_height = (v->height + 15) >> 4;
	s->mb_stride = s->mb_width + 1;
	s->b8_stride = s->mb_width * 2 + 1;
	s->chroma_x_shift = 1;
	s->low_delay = !v->max_b_frames || v->res_sprite;
	s->h_edge_pos = v->profile == PROFILE_ADVANCED ? v->width : s->mb_width * 16;
	s->v_edge_pos = v->profile == PROFILE_ADVANCED ? v->height : s->mb_height * 16;
	s->linesize = FFALIGN(s->mb_width * 16 + 64, 32);
	s->uvlinesize = s->linesize / 2;
	for (int i = 0; i < 6; ++i)
		s->block_wrap[i] = i < 4 ? s->b8_stride : s->mb_stride;
}

void Decoder::allocatePredictors() {
	VC1Context *v = _v;
	PictureState *s = &v->s;
	const int alignedMacroblockHeight = FFALIGN(s->mb_height, 2);
	const int lumaPredictorCount = s->b8_stride * (2 * alignedMacroblockHeight + 1);
	const int chromaPredictorCount = s->mb_stride * (alignedMacroblockHeight + 1);
	const int predictorCount = lumaPredictorCount + 2 * chromaPredictorCount;
	s->coded_block = allocate<uint8>(lumaPredictorCount) + s->b8_stride + 1;
	s->dc_val = allocate<int16>(predictorCount) + s->b8_stride + 1;
	for (int i = 0; i < predictorCount; ++i)
		(s->dc_val - s->b8_stride - 1)[i] = 1024;
	s->ac_val = allocate<int16[16]>(predictorCount) + s->b8_stride + 1;
	s->mbskip_table = allocate<uint8>(s->mb_stride * alignedMacroblockHeight + 2);
	s->sc.edge_emu_buffer = allocate<uint8>(s->linesize * 128);
	v->mv_type_mb_plane = allocate<uint8>(s->mb_stride * alignedMacroblockHeight);
	v->direct_mb_plane = allocate<uint8>(s->mb_stride * alignedMacroblockHeight);
	v->forward_mb_plane = allocate<uint8>(s->mb_stride * alignedMacroblockHeight);
	v->fieldtx_plane = allocate<uint8>(s->mb_stride * alignedMacroblockHeight);
	v->acpred_plane = allocate<uint8>(s->mb_stride * alignedMacroblockHeight);
	v->over_flags_plane = allocate<uint8>(s->mb_stride * alignedMacroblockHeight);
	v->n_allocated_blks = s->mb_width + 2;
	v->block = allocate<int16[6][64]>(v->n_allocated_blks);
	v->cbp_base = allocate<uint32>(3 * s->mb_stride);
	v->cbp = v->cbp_base + 2 * s->mb_stride;
	v->ttblk_base = allocate<int>(3 * s->mb_stride);
	v->ttblk = v->ttblk_base + 2 * s->mb_stride;
	v->is_intra_base = allocate<uint8>(3 * s->mb_stride);
	v->is_intra = v->is_intra_base + 2 * s->mb_stride;
	v->luma_mv_base = allocate<int16[2]>(3 * s->mb_stride);
	v->luma_mv = v->luma_mv_base + 2 * s->mb_stride;
	v->mb_type_base = allocate<uint8>(predictorCount);
	v->mb_type = v->mb_type_base + s->b8_stride + 1;
	v->blk_mv_type_base = allocate<uint8>(lumaPredictorCount);
	v->blk_mv_type = v->blk_mv_type_base + s->b8_stride + 1;
	v->mv_f_base = allocate<uint8>(2 * predictorCount);
	v->mv_f[0] = v->mv_f_base + s->b8_stride + 1;
	v->mv_f[1] = v->mv_f[0] + predictorCount;
	v->mv_f_next_base = allocate<uint8>(2 * predictorCount);
	v->mv_f_next[0] = v->mv_f_next_base + s->b8_stride + 1;
	v->mv_f_next[1] = v->mv_f_next[0] + predictorCount;
}

void Decoder::allocateReferencePictures() {
	VC1Context *v = _v;
	PictureState *s = &v->s;
	const int alignedMacroblockHeight = FFALIGN(s->mb_height, 2);
	const int lumaPredictorCount = s->b8_stride * (2 * alignedMacroblockHeight + 1);
	const int chromaPredictorCount = s->mb_stride * (alignedMacroblockHeight + 1);
	for (int n = 0; n < 3; ++n) {
		Picture &p = _pictures[n];
		p.f = &_planes[n];
		p.f->width = v->width;
		p.f->height = v->height;
		p.ptr = &p;
		for (int i = 0; i < 3; ++i) {
			int stride = i ? s->uvlinesize : s->linesize;
			int height = alignedMacroblockHeight * (i ? 8 : 16);
			p.data[i] = allocate<uint8>(stride * (height + 64)) + stride * 32 + 32;
			p.linesize[i] = stride;
			p.f->data[i] = p.data[i];
			p.f->linesize[i] = stride;
		}
		p.qscale_table = allocate<int8>(chromaPredictorCount);
		p.mb_type = allocate<uint32>(chromaPredictorCount);
		for (int i = 0; i < 2; ++i)
			p.ref_index[i] = allocate<int8>(4 * chromaPredictorCount);
		for (int i = 0; i < 2; ++i)
			p.motion_val[i] = allocate<int16[2]>(lumaPredictorCount + 2 * s->b8_stride) + s->b8_stride + 1;
	}
}

void Decoder::initializeConcealment() {
	VC1Context *v = _v;
	PictureState *s = &v->s;
	const int alignedMacroblockHeight = FFALIGN(s->mb_height, 2);
	initializeER(s);
	s->er.error_status_table = allocate<uint8>(s->mb_stride * alignedMacroblockHeight);
	s->er.er_temp_buffer = allocate<uint8>(s->mb_stride * alignedMacroblockHeight * (4 * sizeof(int) + 1));
	s->er.mbintra_table = allocate<uint8>(s->mb_stride * alignedMacroblockHeight);
	memset(s->er.mbintra_table, 1, s->mb_stride * alignedMacroblockHeight);
	s->er.mb_index2xy = allocate<int>(s->mb_width * alignedMacroblockHeight + 1);
	for (int y = 0; y < alignedMacroblockHeight; ++y)
		for (int x = 0; x < s->mb_width; ++x)
			s->er.mb_index2xy[x + y * s->mb_width] = x + y * s->mb_stride;
	s->er.mb_index2xy[s->er.mb_num] = (s->mb_height - 1) * s->mb_stride + s->mb_width;
}

void Decoder::initializeTransforms() {
	VC1Context *v = _v;
	PictureState *s = &v->s;
	v->x8 = new IntraX8Context();
	initX8(v->x8, v->blocks[0], s->mb_width, s->mb_height);
	for (int i = 0; i < 64; ++i) {
		for (int j = 0; j < 4; ++j) {
			int x = ff_wmv1_scantable[j][i];
			v->zz_8x8[j][i] = v->profile == PROFILE_ADVANCED || v->res_fasttx ? ((x >> 3) | ((x & 7) << 3)) : x;
		}
		int x = ff_vc1_adv_interlaced_8x8_zz[i];
		v->zzi_8x8[i] = (x >> 3) | ((x & 7) << 3);
	}
	v->left_blk_sh = 0;
	v->top_blk_sh = 3;
	if (v->profile != PROFILE_ADVANCED && !v->res_fasttx) {
		v->left_blk_sh = 3;
		v->top_blk_sh = 0;
		v->vc1dsp.vc1_inv_trans_8x8 = ff_simple_idct_int16_8bit;
		v->vc1dsp.vc1_inv_trans_8x8_dc = ff_simple_idct_add_int16_8bit;
		v->vc1dsp.vc1_inv_trans_8x4 = v->vc1dsp.vc1_inv_trans_8x4_dc = ff_simple_idct84_add;
		v->vc1dsp.vc1_inv_trans_4x8 = v->vc1dsp.vc1_inv_trans_4x8_dc = ff_simple_idct48_add;
		v->vc1dsp.vc1_inv_trans_4x4 = v->vc1dsp.vc1_inv_trans_4x4_dc = ff_simple_idct44_add;
	}
}

const Picture *Decoder::decode(const uint8 *data, uint32 size) {
	_failed = false;
	if (!_ready || size > 64 * 1024 * 1024) {
		_failed = true;
		return nullptr;
	}
	if (!size)
		return drain();
	const Picture *output = nullptr;
	if (!decodePacket(data, size, output)) {
		_failed = true;
		return nullptr;
	}
	return output;
}

const Picture *Decoder::drain() {
	if (!_pending)
		return nullptr;
	_pending = false;
	return _v->s.next_pic.ptr;
}
// Packet decoding and reference-frame ordering

bool Decoder::decodePacket(const uint8 *data, uint32 size, const Picture *&output) {
	VC1Context *v = _v;
	PictureState *s = &v->s;
	Common::Array<Slice *> chunks;
	struct Cleanup {
		Common::Array<Slice *> &a;
		~Cleanup() {
			for (uint i = 0; i < a.size(); ++i)
				delete a[i];
		}
	} cleanup{chunks};
	int firstFieldLast = -1;
	if (_advanced) {
		uint32 start = nextMarker(data, size, 0);
		if (start > 0) {
			chunks.push_back(new Slice(data, start, true));
		}
		while (start < size) {
			uint32 end = nextMarker(data, size, start + 4);
			uint8 type = data[start + 3];
			if (type == 14) {
				Slice chunk(data + start + 4, end - start - 4, true);
				if (!readHeader(chunk.data.data(), chunk.bits.size_in_bits / 8, true))
					return false;
			} else if (type == 13 || type == 12 || type == 11) {
				Slice *chunk = new Slice(data + start + 4, end - start - 4, true);
				if (type == 12) {
					chunk->row = (v->height + 31) >> 5;
					firstFieldLast = int(chunks.size()) - 2;
				} else if (type == 11)
					chunk->row = get_bits(&chunk->bits, 9);
				chunks.push_back(chunk);
			}
			start = end;
		}
	} else
		chunks.push_back(new Slice(data, size, false));
	if (chunks.empty())
		return false;
	if (_settings.coded_width != v->width || _settings.coded_height != v->height) {
		clearPictures();
		allocatePictures();
	}
	v->gb = chunks[0]->bits;
	v->second_field = 0;
	if (v->res_sprite) {
		v->new_sprite = !get_bits1(&v->gb);
		v->two_sprites = get_bits1(&v->gb);
	}
	v->pic_header_flag = 0;
	v->first_pic_header_flag = 1;
	int result =
		v->profile < PROFILE_ADVANCED ? ff_vc1_parse_frame_header(v, &v->gb) : ff_vc1_parse_frame_header_adv(v, &v->gb);
	v->first_pic_header_flag = 0;
	if (result < 0 || get_bits_left(&v->gb) < 0)
		return false;
	if (s->pict_type == AV_PICTURE_TYPE_B && !s->last_pic.ptr)
		return true;
	Picture *current = nullptr;
	for (int i = 0; i < 3; ++i)
		if (&_pictures[i] != s->last_pic.ptr && &_pictures[i] != s->next_pic.ptr) {
			current = &_pictures[i];
			break;
		}
	if (!current)
		return false;
	s->cur_pic = *current;
	if (s->pict_type != AV_PICTURE_TYPE_B) {
		s->last_pic = s->next_pic;
		s->next_pic = s->cur_pic;
	}
	if (s->pict_type != AV_PICTURE_TYPE_I && !s->last_pic.ptr)
		return false;
	current->field_picture = v->field_mode;
	current->f->pict_type = s->pict_type;
	current->interlaced = v->fcm != PROGRESSIVE;
	v->last_interlaced = s->last_pic.ptr ? s->last_pic.ptr->interlaced : 0;
	v->next_interlaced = s->next_pic.ptr ? s->next_pic.ptr->interlaced : 0;
	s->er.cur_pic = s->cur_pic;
	s->er.last_pic = s->last_pic;
	s->er.next_pic = s->next_pic;
	s->er.quarter_sample = s->quarter_sample;
	ff_er_frame_start(&s->er);
	v->end_mb_x = s->mb_width;
	if (v->field_mode) {
		for (int i = 0; i < 3; ++i)
			s->cur_pic.linesize[i] *= 2;
		s->linesize *= 2;
		s->uvlinesize *= 2;
	}
	int mh = s->mb_height >> v->field_mode;
	bool ok = mh > 0;
	for (uint i = 0; ok && i < chunks.size(); ++i) {
		v->second_field = i > 0 && chunks[i]->row >= mh;
		if (v->second_field && !v->field_mode) {
			ok = false;
			break;
		}
		v->blocks_off = v->second_field ? s->b8_stride * (s->mb_height & ~1) : 0;
		v->mb_off = v->second_field ? (s->mb_stride * s->mb_height) >> 1 : 0;
		if (i) {
			v->gb = chunks[i]->bits;
			v->pic_header_flag = 0;
			if ((v->field_mode && int(i) == firstFieldLast + 2) || (v->pic_header_flag = get_bits1(&v->gb))) {
				if (ff_vc1_parse_frame_header_adv(v, &v->gb) < 0) {
					ok = false;
					break;
				}
			}
		}
		s->start_mb_y = i ? MAX(0, chunks[i]->row % mh) : 0;
		if (!v->field_mode || v->second_field)
			s->end_mb_y = i + 1 == chunks.size() ? mh : MIN(mh, chunks[i + 1]->row % mh);
		else {
			if (i + 1 >= chunks.size()) {
				ok = false;
				break;
			}
			s->end_mb_y = int(i) == firstFieldLast + 1 ? mh : MIN(mh, chunks[i + 1]->row % mh);
		}
		if (s->end_mb_y <= s->start_mb_y) {
			ok = false;
			break;
		}
		if (((s->pict_type == AV_PICTURE_TYPE_P && !v->p_frame_skipped) ||
			 (s->pict_type == AV_PICTURE_TYPE_B && !v->bi_type)) &&
			!v->cbpcy_vlc) {
			ok = false;
			break;
		}
		ff_vc1_decode_blocks(v);
	}
	if (v->field_mode) {
		v->second_field = 0;
		for (int i = 0; i < 3; ++i)
			s->cur_pic.linesize[i] /= 2;
		s->linesize /= 2;
		s->uvlinesize /= 2;
		if (s->pict_type != AV_PICTURE_TYPE_B) {
			SWAP(v->mv_f_next[0], v->mv_f[0]);
			SWAP(v->mv_f_next[1], v->mv_f[1]);
		}
	}
	if (!ok)
		return false;
	if (s->er.error_occurred && s->pict_type == AV_PICTURE_TYPE_B)
		return false;
	if (!v->field_mode)
		ff_er_frame_end(&s->er, nullptr);
	if (s->pict_type == AV_PICTURE_TYPE_B || s->low_delay)
		output = current;
	else {
		output = s->last_pic.ptr;
		_pending = true;
	}
	return true;
}
} // namespace VC1
// ScummVM codec interface

WMV3Decoder::WMV3Decoder(int width, int height, bool advanced)
	: _decoder(new VC1::Decoder(width, height, advanced)), _format(Codec::getDefaultYUVFormat()) {
}
WMV3Decoder::~WMV3Decoder() {
	_surface.free();
	delete _decoder;
}

bool WMV3Decoder::setExtraData(Common::SeekableReadStream &stream) {
	int64 size = stream.size() - stream.pos();
	if (size <= 0 || size > 1024 * 1024)
		return false;
	Common::Array<uint8> data;
	data.resize(size);
	if (stream.read(data.data(), size) != size)
		return false;
	return _decoder->initialize(data.data(), size);
}

const Graphics::Surface *WMV3Decoder::decodeFrame(Common::SeekableReadStream &stream) {
	int64 size = stream.size() - stream.pos();
	if (size < 0 || size > 64 * 1024 * 1024)
		return nullptr;
	Common::Array<uint8> data;
	data.resize(size);
	if (size && stream.read(data.data(), size) != size)
		return nullptr;
	return convert(_decoder->decode(data.data(), size));
}

const Graphics::Surface *WMV3Decoder::drainFrame() {
	return convert(_decoder->drain());
}
bool WMV3Decoder::hasError() const {
	return _decoder->failed();
}
bool WMV3Decoder::setOutputPixelFormat(const Graphics::PixelFormat &format) {
	if (format.bytesPerPixel != 2 && format.bytesPerPixel != 4)
		return false;
	if (format != _format) {
		_surface.free();
		_format = format;
	}
	return true;
}

const Graphics::Surface *WMV3Decoder::convert(const VC1::Picture *picture) {
	if (!picture)
		return nullptr;
	int w = _decoder->width(), h = _decoder->height();
	if (!_surface.getPixels() || _surface.w != w || _surface.h != h) {
		_surface.free();
		_surface.create((w + 1) & ~1, (h + 1) & ~1, _format);
	}
	YUVToRGBMan.convert420(&_surface, Graphics::YUVToRGBManager::kScaleITU, picture->data[0], picture->data[1],
						   picture->data[2], (w + 1) & ~1, (h + 1) & ~1, picture->linesize[0], picture->linesize[1]);
	_surface.w = w;
	_surface.h = h;
	return &_surface;
}

} // namespace Image
