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
	 *  @param[in]  in		Pointer to the vector of transform coefficients
	 *  @param[out] out		Pointer to the output buffer (frame)
	 *  @param[in]  pitch	Pitch to move to the next y line
	 *  @param[in]  flags	Pointer to the array of column flags:
	 *						!= 0 - non_empty column, 0 - empty one
	 *						(this array must be filled by caller)
	 */
	static void ffIviInverseHaar8x8(const int32 *in, int16 *out, uint32 pitch,
		const uint8 *flags);
	static void ffIviInverseHaar8x1(const int32 *in, int16 *out, uint32 pitch,
		const uint8 *flags);
	static void ffIviInverseHaar1x8(const int32 *in, int16 *out, uint32 pitch,
		const uint8 *flags);

	/**
	 *  one-dimensional inverse 8-point Haar transform on rows for Indeo 4
	 *
	 *  @param[in]  in		Pointer to the vector of transform coefficients
	 *  @param[out] out		Pointer to the output buffer (frame)
	 *  @param[in]  pitch	Pitch to move to the next y line
	 *  @param[in]  flags	Pointer to the array of column flags:
	 *						!= 0 - non_empty column, 0 - empty one
	 *						(this array must be filled by caller)
	 */
	static void ffIviRowHaar8(const int32 *in, int16 *out, uint32 pitch,
		const uint8 *flags);

	/**
	 *  one-dimensional inverse 8-point Haar transform on columns for Indeo 4
	 *
	 *  @param[in]  in		Pointer to the vector of transform coefficients
	 *  @param[out] out		Pointer to the output buffer (frame)
	 *  @param[in]  pitch	Pitch to move to the next y line
	 *  @param[in]  flags	Pointer to the array of column flags:
	 *						!= 0 - non_empty column, 0 - empty one
	 *						(this array must be filled by caller)
	 */
	static void ffIviColHaar8(const int32 *in, int16 *out, uint32 pitch,
		const uint8 *flags);

	/**
	 *  two-dimensional inverse Haar 4x4 transform for Indeo 4
	 *
	 *  @param[in]  in		Pointer to the vector of transform coefficients
	 *  @param[out] out		Pointer to the output buffer (frame)
	 *  @param[in]  pitch	Pitch to move to the next y line
	 *  @param[in]  flags	Pointer to the array of column flags:
	 *						!= 0 - non_empty column, 0 - empty one
	 *						(this array must be filled by caller)
	 */
	static void ffIviInverseHaar4x4(const int32 *in, int16 *out, uint32 pitch,
		const uint8 *flags);

	/**
	 *  one-dimensional inverse 4-point Haar transform on rows for Indeo 4
	 *
	 *  @param[in]  in		Pointer to the vector of transform coefficients
	 *  @param[out] out		Pointer to the output buffer (frame)
	 *  @param[in]  pitch	Pitch to move to the next y line
	 *  @param[in]  flags	Pointer to the array of column flags:
	 *						!= 0 - non_empty column, 0 - empty one
	 *						(this array must be filled by caller)
	 */
	static void ffIviRowHaar4(const int32 *in, int16 *out, uint32 pitch,
		const uint8 *flags);

	/**
	 *  one-dimensional inverse 4-point Haar transform on columns for Indeo 4
	 *
	 *  @param[in]  in		Pointer to the vector of transform coefficients
	 *  @param[out] out		Pointer to the output buffer (frame)
	 *  @param[in]  pitch	Pitch to move to the next y line
	 *  @param[in]  flags	Pointer to the array of column flags:
	 *						!= 0 - non_empty column, 0 - empty one
	 *						(this array must be filled by caller)
	 */
	static void ffIviColHaar4(const int32 *in, int16 *out, uint32 pitch,
						  const uint8 *flags);

	/**
	 *  DC-only two-dimensional inverse Haar transform for Indeo 4.
	 *  Performing the inverse transform in this case is equivalent to
	 *  spreading dcCoeff >> 3 over the whole block.
	 *
	 *  @param[in]  in			Pointer to the dc coefficient
	 *  @param[out] out			Pointer to the output buffer (frame)
	 *  @param[in]  pitch		Pitch to move to the next y line
	 *  @param[in]  blkSize		Transform block size
	 */
	static void ffIviDcHaar2d(const int32 *in, int16 *out, uint32 pitch,
		int blkSize);

	/**
	 *  two-dimensional inverse slant 8x8 transform
	 *
	 *  @param[in]  in		Pointer to the vector of transform coefficients
	 *  @param[out] out		Pointer to the output buffer (frame)
	 *  @param[in]  pitch	Pitch to move to the next y line
	 *  @param[in]  flags	Pointer to the array of column flags:
	 *						!= 0 - non_empty column, 0 - empty one
	 *						(this array must be filled by caller)
	 */
	static void ffIviInverseSlant8x8(const int32 *in, int16 *out, uint32 pitch,
		const uint8 *flags);

	/**
	 *  two-dimensional inverse slant 4x4 transform
	 *
	 *  @param[in]  in		Pointer to the vector of transform coefficients
	 *  @param[out] out		Pointer to the output buffer (frame)
	 *  @param[in]  pitch	Pitch to move to the next y line
	 *  @param[in]  flags	Pointer to the array of column flags:
	 *						!= 0 - non_empty column, 0 - empty one
	 *						(this array must be filled by caller)
	 */
	static void ffIviInverseSlant4x4(const int32 *in, int16 *out, uint32 pitch,
		const uint8 *flags);

	/**
	 *  DC-only two-dimensional inverse slant transform.
	 *  Performing the inverse slant transform in this case is equivalent to
	 *  spreading (dcCoeff + 1)/2 over the whole block.
	 *  It works much faster than performing the slant transform on a vector of zeroes.
	 *
	 *  @param[in]  in			Pointer to the dc coefficient
	 *  @param[out] out			Pointer to the output buffer (frame)
	 *  @param[in]  pitch		Pitch to move to the next y line
	 *  @param[in]  blkSize		Transform block size
	 */
	static void ffIviDcSlant2d(const int32 *in, int16 *out, uint32 pitch, int blkSize);

	/**
	 *  inverse 1D row slant transform
	 *
	 *  @param[in]    in		Pointer to the vector of transform coefficients
	 *  @param[out]   out		Pointer to the output buffer (frame)
	 *  @param[in]    pitch		Pitch to move to the next y line
	 *  @param[in]    flags		Pointer to the array of column flags (unused here)
	 */
	static void ffIviRowSlant8(const int32 *in, int16 *out, uint32 pitch,
		const uint8 *flags);

	/**
	 *  inverse 1D column slant transform
	 *
	 *  @param[in]  in		Pointer to the vector of transform coefficients
	 *  @param[out] out		Pointer to the output buffer (frame)
	 *  @param[in]  pitch	Pitch to move to the next y line
	 *  @param[in]  flags	Pointer to the array of column flags:
	 *						!= 0 - non_empty column, 0 - empty one
	 *						(this array must be filled by caller)
	 */
	static void ffIviColSlant8(const int32 *in, int16 *out, uint32 pitch,
		const uint8 *flags);

	/**
	 *  inverse 1D row slant transform
	 *
	 *  @param[in]    in		Pointer to the vector of transform coefficients
	 *  @param[out]   out		Pointer to the output buffer (frame)
	 *  @param[in]    pitch		Pitch to move to the next y line
	 *  @param[in]    flags		Pointer to the array of column flags (unused here)
	 */
	static void ffIviRowSlant4(const int32 *in, int16 *out, uint32 pitch,
		const uint8 *flags);

	/**
	 *  inverse 1D column slant transform
	 *
	 *  @param[in]  in		Pointer to the vector of transform coefficients
	 *  @param[out] out		Pointer to the output buffer (frame)
	 *  @param[in]  pitch	Pitch to move to the next y line
	 *  @param[in]  flags	Pointer to the array of column flags:
	 *						!= 0 - non_empty column, 0 - empty one
	 *						(this array must be filled by caller)
	 */
	static void ffIviColSlant4(const int32 *in, int16 *out, uint32 pitch,
		const uint8 *flags);

	/**
	 *  DC-only inverse row slant transform
	 */
	static void ffIviDcRowSlant(const int32 *in, int16 *out, uint32 pitch, int blkSize);

	/**
	 *  DC-only inverse column slant transform
	 */
	static void ffIviDcColSlant(const int32 *in, int16 *out, uint32 pitch, int blkSize);

	/**
	 *  Copy the pixels into the frame buffer.
	 */
	static void ffIviPutPixels8x8(const int32 *in, int16 *out, uint32 pitch, const uint8 *flags);

	/**
	 *  Copy the DC coefficient into the first pixel of the block and
	 *  zero all others.
	 */
	static void ffIviPutDcPixel8x8(const int32 *in, int16 *out, uint32 pitch, int blkSize);

	/**
	 *  8x8 block motion compensation with adding delta
	 *
	 *  @param[in,out]   buf		Pointer to the block in the current frame buffer containing delta
	 *  @param[in]       refBuf		Pointer to the corresponding block in the reference frame
	 *  @param[in]       pitch		Pitch for moving to the next y line
	 *  @param[in]       mcType		Interpolation type
	 */
	static void ffIviMc8x8Delta(int16 *buf, const int16 *refBuf, uint32 pitch, int mcType);

	/**
	 *  4x4 block motion compensation with adding delta
	 *
	 *  @param[in,out]   buf		Pointer to the block in the current frame buffer containing delta
	 *  @param[in]       refBuf		Pointer to the corresponding block in the reference frame
	 *  @param[in]       pitch		Pitch for moving to the next y line
	 *  @param[in]       mcType		Interpolation type
	 */
	static void ffIviMc4x4Delta(int16 *buf, const int16 *refBuf, uint32 pitch, int mcType);

	/**
	 *  motion compensation without adding delta
	 *
	 *  @param[in,out]   buf		Pointer to the block in the current frame buffer containing delta
	 *  @param[in]       refBuf		Pointer to the corresponding block in the reference frame
	 *  @param[in]       pitch		Pitch for moving to the next y line
	 *  @param[in]       mcType		Interpolation type
	 */
	static void ffIviMc8x8NoDelta(int16 *buf, const int16 *refBuf, uint32 pitch, int mcType);

	/**
	 *  4x4 block motion compensation without adding delta
	 *
	 *  @param[in,out]  buf			Pointer to the block in the current frame receiving the result
	 *  @param[in]      refBuf		Pointer to the corresponding block in the reference frame
	 *  @param[in]      pitch		Pitch for moving to the next y line
	 *  @param[in]      mcType		Interpolation type
	 */
	static void ffIviMc4x4NoDelta(int16 *buf, const int16 *refBuf, uint32 pitch, int mcType);

	/**
	 *  8x8 block motion compensation with adding delta
	 *
	 *  @param[in,out]  buf			Pointer to the block in the current frame buffer containing delta
	 *  @param[in]      refBuf		Pointer to the corresponding block in the backward reference frame
	 *  @param[in]      refBuf2		Pointer to the corresponding block in the forward reference frame
	 *  @param[in]      pitch		Pitch for moving to the next y line
	 *  @param[in]      mcType		Interpolation type for backward reference
	 *  @param[in]      mcType2		Interpolation type for forward reference
	 */
	static void ffIviMcAvg8x8Delta(int16 *buf, const int16 *refBuf, const int16 *refBuf2, uint32 pitch, int mcType, int mcType2);

	/**
	 *  4x4 block motion compensation with adding delta
	 *
	 *  @param[in,out]  buf			Pointer to the block in the current frame buffer containing delta
	 *  @param[in]      refBuf		Pointer to the corresponding block in the backward reference frame
	 *  @param[in]      refBuf2		Pointer to the corresponding block in the forward reference frame
	 *  @param[in]      pitch		Pitch for moving to the next y line
	 *  @param[in]      mcType		Interpolation type for backward reference
	 *  @param[in]      mcType2		Interpolation type for forward reference
	 */
	static void ffIviMcAvg4x4Delta(int16 *buf, const int16 *refBuf, const int16 *refBuf2, uint32 pitch, int mcType, int mcType2);

	/**
	 *  motion compensation without adding delta for B-frames
	 *
	 *  @param[in,out]  buf			Pointer to the block in the current frame buffer containing delta
	 *  @param[in]      refBuf		Pointer to the corresponding block in the backward reference frame
	 *  @param[in]      refBuf2		Pointer to the corresponding block in the forward reference frame
	 *  @param[in]      pitch		Pitch for moving to the next y line
	 *  @param[in]      mcType		Interpolation type for backward reference
	 *  @param[in]      mcType2		Interpolation type for forward reference
	 */
	static void ffIviMcAvg8x8NoDelta(int16 *buf, const int16 *refBuf, const int16 *refBuf2, uint32 pitch, int mcType, int mcType2);

	/**
	 *  4x4 block motion compensation without adding delta for B-frames
	 *
	 *  @param[in,out]  buf			Pointer to the block in the current frame buffer containing delta
	 *  @param[in]      refBuf		Pointer to the corresponding block in the backward reference frame
	 *  @param[in]      refBuf2		Pointer to the corresponding block in the forward reference frame
	 *  @param[in]      pitch		Pitch for moving to the next y line
	 *  @param[in]      mcType		Interpolation type for backward reference
	 *  @param[in]      mcType2		Interpolation type for forward reference
	 */
	static void ffIviMcAvg4x4NoDelta(int16 *buf, const int16 *refBuf, const int16 *refBuf2, uint32 pitch, int mcType, int mcType2);
};

} // End of namespace Indeo
} // End of namespace Image

#endif
