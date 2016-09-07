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

/* VLC code
 *
 * Original copyright note:
 * DSP functions (inverse transforms, motion compensation, wavelet recompositions)
 * for Indeo Video Interactive codecs.
 */

#ifndef IMAGE_CODECS_INDEO_INDEO_DSP_H
#define IMAGE_CODECS_INDEO_INDEO_DSP_H

#include "image/codecs/indeo/mem.h"
#include "image/codecs/indeo/indeo.h"

namespace Image {
namespace Indeo {

class IndeoDSP {
public:
	/**
	 *  two-dimensional inverse Haar 8x8 transform for Indeo 4
	 *
	 *  @param[in]  in        pointer to the vector of transform coefficients
	 *  @param[out] out       pointer to the output buffer (frame)
	 *  @param[in]  pitch     pitch to move to the next y line
	 *  @param[in]  flags     pointer to the array of column flags:
	 *                        != 0 - non_empty column, 0 - empty one
	 *                        (this array must be filled by caller)
	 */
	static void ff_ivi_inverse_haar_8x8(const int32 *in, int16 *out, uint32 pitch,
								 const uint8 *flags);
	static void ff_ivi_inverse_haar_8x1(const int32 *in, int16 *out, uint32 pitch,
								 const uint8 *flags);
	static void ff_ivi_inverse_haar_1x8(const int32 *in, int16 *out, uint32 pitch,
								 const uint8 *flags);

	/**
	 *  one-dimensional inverse 8-point Haar transform on rows for Indeo 4
	 *
	 *  @param[in]  in        pointer to the vector of transform coefficients
	 *  @param[out] out       pointer to the output buffer (frame)
	 *  @param[in]  pitch     pitch to move to the next y line
	 *  @param[in]  flags     pointer to the array of column flags:
	 *                        != 0 - non_empty column, 0 - empty one
	 *                        (this array must be filled by caller)
	 */
	static void ff_ivi_row_haar8(const int32 *in, int16 *out, uint32 pitch,
						  const uint8 *flags);

	/**
	 *  one-dimensional inverse 8-point Haar transform on columns for Indeo 4
	 *
	 *  @param[in]  in        pointer to the vector of transform coefficients
	 *  @param[out] out       pointer to the output buffer (frame)
	 *  @param[in]  pitch     pitch to move to the next y line
	 *  @param[in]  flags     pointer to the array of column flags:
	 *                        != 0 - non_empty column, 0 - empty one
	 *                        (this array must be filled by caller)
	 */
	static void ff_ivi_col_haar8(const int32 *in, int16 *out, uint32 pitch,
						  const uint8 *flags);

	/**
	 *  two-dimensional inverse Haar 4x4 transform for Indeo 4
	 *
	 *  @param[in]  in        pointer to the vector of transform coefficients
	 *  @param[out] out       pointer to the output buffer (frame)
	 *  @param[in]  pitch     pitch to move to the next y line
	 *  @param[in]  flags     pointer to the array of column flags:
	 *                        != 0 - non_empty column, 0 - empty one
	 *                        (this array must be filled by caller)
	 */
	static void ff_ivi_inverse_haar_4x4(const int32 *in, int16 *out, uint32 pitch,
								 const uint8 *flags);

	/**
	 *  one-dimensional inverse 4-point Haar transform on rows for Indeo 4
	 *
	 *  @param[in]  in        pointer to the vector of transform coefficients
	 *  @param[out] out       pointer to the output buffer (frame)
	 *  @param[in]  pitch     pitch to move to the next y line
	 *  @param[in]  flags     pointer to the array of column flags:
	 *                        != 0 - non_empty column, 0 - empty one
	 *                        (this array must be filled by caller)
	 */
	static void ff_ivi_row_haar4(const int32 *in, int16 *out, uint32 pitch,
						  const uint8 *flags);

	/**
	 *  one-dimensional inverse 4-point Haar transform on columns for Indeo 4
	 *
	 *  @param[in]  in        pointer to the vector of transform coefficients
	 *  @param[out] out       pointer to the output buffer (frame)
	 *  @param[in]  pitch     pitch to move to the next y line
	 *  @param[in]  flags     pointer to the array of column flags:
	 *                        != 0 - non_empty column, 0 - empty one
	 *                        (this array must be filled by caller)
	 */
	static void ff_ivi_col_haar4(const int32 *in, int16 *out, uint32 pitch,
						  const uint8 *flags);

	/**
	 *  DC-only two-dimensional inverse Haar transform for Indeo 4.
	 *  Performing the inverse transform in this case is equivalent to
	 *  spreading DC_coeff >> 3 over the whole block.
	 *
	 *  @param[in]  in          pointer to the dc coefficient
	 *  @param[out] out         pointer to the output buffer (frame)
	 *  @param[in]  pitch       pitch to move to the next y line
	 *  @param[in]  blk_size    transform block size
	 */
	static void ff_ivi_dc_haar_2d(const int32 *in, int16 *out, uint32 pitch,
						   int blk_size);

	/**
	 *  two-dimensional inverse slant 8x8 transform
	 *
	 *  @param[in]    in      pointer to the vector of transform coefficients
	 *  @param[out]   out     pointer to the output buffer (frame)
	 *  @param[in]    pitch   pitch to move to the next y line
	 *  @param[in]    flags   pointer to the array of column flags:
	 *                        != 0 - non_empty column, 0 - empty one
	 *                        (this array must be filled by caller)
	 */
	static void ff_ivi_inverse_slant_8x8(const int32 *in, int16 *out, uint32 pitch,
								  const uint8 *flags);

	/**
	 *  two-dimensional inverse slant 4x4 transform
	 *
	 *  @param[in]    in      pointer to the vector of transform coefficients
	 *  @param[out]   out     pointer to the output buffer (frame)
	 *  @param[in]    pitch   pitch to move to the next y line
	 *  @param[in]    flags   pointer to the array of column flags:
	 *                        != 0 - non_empty column, 0 - empty one
	 *                        (this array must be filled by caller)
	 */
	static void ff_ivi_inverse_slant_4x4(const int32 *in, int16 *out, uint32 pitch,
								  const uint8 *flags);

	/**
	 *  DC-only two-dimensional inverse slant transform.
	 *  Performing the inverse slant transform in this case is equivalent to
	 *  spreading (DC_coeff + 1)/2 over the whole block.
	 *  It works much faster than performing the slant transform on a vector of zeroes.
	 *
	 *  @param[in]    in          pointer to the dc coefficient
	 *  @param[out]   out         pointer to the output buffer (frame)
	 *  @param[in]    pitch       pitch to move to the next y line
	 *  @param[in]    blk_size    transform block size
	 */
	static void ff_ivi_dc_slant_2d(const int32 *in, int16 *out, uint32 pitch, int blk_size);

	/**
	 *  inverse 1D row slant transform
	 *
	 *  @param[in]    in      pointer to the vector of transform coefficients
	 *  @param[out]   out     pointer to the output buffer (frame)
	 *  @param[in]    pitch   pitch to move to the next y line
	 *  @param[in]    flags   pointer to the array of column flags (unused here)
	 */
	static void ff_ivi_row_slant8(const int32 *in, int16 *out, uint32 pitch,
						   const uint8 *flags);

	/**
	 *  inverse 1D column slant transform
	 *
	 *  @param[in]    in      pointer to the vector of transform coefficients
	 *  @param[out]   out     pointer to the output buffer (frame)
	 *  @param[in]    pitch   pitch to move to the next y line
	 *  @param[in]    flags   pointer to the array of column flags:
	 *                        != 0 - non_empty column, 0 - empty one
	 *                        (this array must be filled by caller)
	 */
	static void ff_ivi_col_slant8(const int32 *in, int16 *out, uint32 pitch,
						   const uint8 *flags);

	/**
	 *  inverse 1D row slant transform
	 *
	 *  @param[in]    in      pointer to the vector of transform coefficients
	 *  @param[out]   out     pointer to the output buffer (frame)
	 *  @param[in]    pitch   pitch to move to the next y line
	 *  @param[in]    flags   pointer to the array of column flags (unused here)
	 */
	static void ff_ivi_row_slant4(const int32 *in, int16 *out, uint32 pitch,
						   const uint8 *flags);

	/**
	 *  inverse 1D column slant transform
	 *
	 *  @param[in]    in      pointer to the vector of transform coefficients
	 *  @param[out]   out     pointer to the output buffer (frame)
	 *  @param[in]    pitch   pitch to move to the next y line
	 *  @param[in]    flags   pointer to the array of column flags:
	 *                        != 0 - non_empty column, 0 - empty one
	 *                        (this array must be filled by caller)
	 */
	static void ff_ivi_col_slant4(const int32 *in, int16 *out, uint32 pitch,
						   const uint8 *flags);

	/**
	 *  DC-only inverse row slant transform
	 */
	static void ff_ivi_dc_row_slant(const int32 *in, int16 *out, uint32 pitch, int blk_size);

	/**
	 *  DC-only inverse column slant transform
	 */
	static void ff_ivi_dc_col_slant(const int32 *in, int16 *out, uint32 pitch, int blk_size);

	/**
	 *  Copy the pixels into the frame buffer.
	 */
	static void ff_ivi_put_pixels_8x8(const int32 *in, int16 *out, uint32 pitch, const uint8 *flags);

	/**
	 *  Copy the DC coefficient into the first pixel of the block and
	 *  zero all others.
	 */
	static void ff_ivi_put_dc_pixel_8x8(const int32 *in, int16 *out, uint32 pitch, int blk_size);

	/**
	 *  8x8 block motion compensation with adding delta
	 *
	 *  @param[in,out]   buf      pointer to the block in the current frame buffer containing delta
	 *  @param[in]       ref_buf  pointer to the corresponding block in the reference frame
	 *  @param[in]       pitch    pitch for moving to the next y line
	 *  @param[in]       mc_type  interpolation type
	 */
	static void ff_ivi_mc_8x8_delta(int16 *buf, const int16 *ref_buf, uint32 pitch, int mc_type);

	/**
	 *  4x4 block motion compensation with adding delta
	 *
	 *  @param[in,out]   buf      pointer to the block in the current frame buffer containing delta
	 *  @param[in]       ref_buf  pointer to the corresponding block in the reference frame
	 *  @param[in]       pitch    pitch for moving to the next y line
	 *  @param[in]       mc_type  interpolation type
	 */
	static void ff_ivi_mc_4x4_delta(int16 *buf, const int16 *ref_buf, uint32 pitch, int mc_type);

	/**
	 *  motion compensation without adding delta
	 *
	 *  @param[in,out]  buf      pointer to the block in the current frame receiving the result
	 *  @param[in]      ref_buf  pointer to the corresponding block in the reference frame
	 *  @param[in]      pitch    pitch for moving to the next y line
	 *  @param[in]      mc_type  interpolation type
	 */
	static void ff_ivi_mc_8x8_no_delta(int16 *buf, const int16 *ref_buf, uint32 pitch, int mc_type);

	/**
	 *  4x4 block motion compensation without adding delta
	 *
	 *  @param[in,out]  buf      pointer to the block in the current frame receiving the result
	 *  @param[in]      ref_buf  pointer to the corresponding block in the reference frame
	 *  @param[in]      pitch    pitch for moving to the next y line
	 *  @param[in]      mc_type  interpolation type
	 */
	static void ff_ivi_mc_4x4_no_delta(int16 *buf, const int16 *ref_buf, uint32 pitch, int mc_type);

	/**
	 *  8x8 block motion compensation with adding delta
	 *
	 *  @param[in,out]  buf      pointer to the block in the current frame buffer containing delta
	 *  @param[in]      ref_buf  pointer to the corresponding block in the backward reference frame
	 *  @param[in]      ref_buf2 pointer to the corresponding block in the forward reference frame
	 *  @param[in]      pitch    pitch for moving to the next y line
	 *  @param[in]      mc_type  interpolation type for backward reference
	 *  @param[in]      mc_type2 interpolation type for forward reference
	 */
	static void ff_ivi_mc_avg_8x8_delta(int16 *buf, const int16 *ref_buf, const int16 *ref_buf2, uint32 pitch, int mc_type, int mc_type2);

	/**
	 *  4x4 block motion compensation with adding delta
	 *
	 *  @param[in,out]  buf      pointer to the block in the current frame buffer containing delta
	 *  @param[in]      ref_buf  pointer to the corresponding block in the backward reference frame
	 *  @param[in]      ref_buf2 pointer to the corresponding block in the forward reference frame
	 *  @param[in]      pitch    pitch for moving to the next y line
	 *  @param[in]      mc_type  interpolation type for backward reference
	 *  @param[in]      mc_type2 interpolation type for forward reference
	 */
	static void ff_ivi_mc_avg_4x4_delta(int16 *buf, const int16 *ref_buf, const int16 *ref_buf2, uint32 pitch, int mc_type, int mc_type2);

	/**
	 *  motion compensation without adding delta for B-frames
	 *
	 *  @param[in,out]  buf      pointer to the block in the current frame receiving the result
	 *  @param[in]      ref_buf  pointer to the corresponding block in the backward reference frame
	 *  @param[in]      ref_buf2 pointer to the corresponding block in the forward reference frame
	 *  @param[in]      pitch    pitch for moving to the next y line
	 *  @param[in]      mc_type  interpolation type for backward reference
	 *  @param[in]      mc_type2 interpolation type for forward reference
	 */
	static void ff_ivi_mc_avg_8x8_no_delta(int16 *buf, const int16 *ref_buf, const int16 *ref_buf2, uint32 pitch, int mc_type, int mc_type2);

	/**
	 *  4x4 block motion compensation without adding delta for B-frames
	 *
	 *  @param[in,out]  buf      pointer to the block in the current frame receiving the result
	 *  @param[in]      ref_buf  pointer to the corresponding block in the backward reference frame
	 *  @param[in]      ref_buf2 pointer to the corresponding block in the forward reference frame
	 *  @param[in]      pitch    pitch for moving to the next y line
	 *  @param[in]      mc_type  interpolation type for backward reference
	 *  @param[in]      mc_type2 interpolation type for forward reference
	 */
	static void ff_ivi_mc_avg_4x4_no_delta(int16 *buf, const int16 *ref_buf, const int16 *ref_buf2, uint32 pitch, int mc_type, int mc_type2);
};

} // End of namespace Indeo
} // End of namespace Image

#endif
