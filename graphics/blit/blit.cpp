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

#include "graphics/blit.h"
#include "graphics/pixelformat.h"
#include "common/endian.h"

namespace Graphics {

// see graphics/blit/blit-atari.cpp
#ifndef ATARI
// Function to blit a rect
void copyBlit(byte *dst, const byte *src,
			   const uint dstPitch, const uint srcPitch,
			   const uint w, const uint h,
			   const uint bytesPerPixel) {
	if (dst == src)
		return;

	if (dstPitch == srcPitch && ((w * bytesPerPixel) == dstPitch)) {
		memcpy(dst, src, dstPitch * h);
	} else {
		for (uint i = 0; i < h; ++i) {
			memcpy(dst, src, w * bytesPerPixel);
			dst += dstPitch;
			src += srcPitch;
		}
	}
}
#endif

namespace {

template<typename Color, int Size>
inline void keyBlitLogic(byte *dst, const byte *src, const uint w, const uint h,
						 const uint srcDelta, const uint dstDelta, const uint32 key) {
	const uint8 *col = (const uint8 *)&key;
#ifdef SCUMM_BIG_ENDIAN
	if (Size == 3)
		col++;
#endif

	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
			if (Size == sizeof(Color)) {
				const uint32 color = *(const Color *)src;
				if (color != key)
					*(Color *)dst = color;
			} else {
				if (memcmp(src, col, Size))
					memcpy(dst, src, Size);
			}

			src += Size;
			dst += Size;
		}

		src += srcDelta;
		dst += dstDelta;
	}
}

} // End of anonymous namespace

// Function to blit a rect with a transparent color key
bool keyBlit(byte *dst, const byte *src,
			   const uint dstPitch, const uint srcPitch,
			   const uint w, const uint h,
			   const uint bytesPerPixel, const uint32 key) {
	if (dst == src)
		return true;

	// Faster, but larger, to provide optimized handling for each case.
	const uint srcDelta = (srcPitch - w * bytesPerPixel);
	const uint dstDelta = (dstPitch - w * bytesPerPixel);

	if (bytesPerPixel == 1) {
		keyBlitLogic<uint8, 1>(dst, src, w, h, srcDelta, dstDelta, key);
	} else if (bytesPerPixel == 2) {
		keyBlitLogic<uint16, 2>(dst, src, w, h, srcDelta, dstDelta, key);
	} else if (bytesPerPixel == 3) {
		keyBlitLogic<uint8, 3>(dst, src, w, h, srcDelta, dstDelta, key);
	} else if (bytesPerPixel == 4) {
		keyBlitLogic<uint32, 4>(dst, src, w, h, srcDelta, dstDelta, key);
	} else {
		return false;
	}

	return true;
}

namespace {

template<typename Color, int Size>
inline void maskBlitLogic(byte *dst, const byte *src, const byte *mask, const uint w, const uint h,
						 const uint srcDelta, const uint dstDelta, const uint maskDelta) {
	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
			if (*mask) {
				if (Size == sizeof(Color)) {
					*(Color *)dst = *(const Color *)src;
				} else {
					memcpy(dst, src, Size);
				}
			}

			src  += Size;
			dst  += Size;
			mask += 1;
		}

		src  += srcDelta;
		dst  += dstDelta;
		mask += maskDelta;
	}
}

} // End of anonymous namespace

// Function to blit a rect with a transparent color mask
bool maskBlit(byte *dst, const byte *src, const byte *mask,
			   const uint dstPitch, const uint srcPitch, const uint maskPitch,
			   const uint w, const uint h,
			   const uint bytesPerPixel) {
	if (dst == src)
		return true;

	// Faster, but larger, to provide optimized handling for each case.
	const uint srcDelta  = (srcPitch  - w * bytesPerPixel);
	const uint dstDelta  = (dstPitch  - w * bytesPerPixel);
	const uint maskDelta = (maskPitch - w);

	if (bytesPerPixel == 1) {
		maskBlitLogic<uint8, 1>(dst, src, mask, w, h, srcDelta, dstDelta, maskDelta);
	} else if (bytesPerPixel == 2) {
		maskBlitLogic<uint16, 2>(dst, src, mask, w, h, srcDelta, dstDelta, maskDelta);
	} else if (bytesPerPixel == 3) {
		maskBlitLogic<uint8, 3>(dst, src, mask, w, h, srcDelta, dstDelta, maskDelta);
	} else if (bytesPerPixel == 4) {
		maskBlitLogic<uint32, 4>(dst, src, mask, w, h, srcDelta, dstDelta, maskDelta);
	} else {
		return false;
	}

	return true;
}

namespace {

template<typename SrcColor, int SrcSize, typename DstColor, int DstSize, bool backward, bool hasKey, bool hasMask>
inline void crossBlitLogic(byte *dst, const byte *src, const byte *mask, const uint w, const uint h,
						   const PixelFormat &srcFmt, const PixelFormat &dstFmt,
						   const uint srcDelta, const uint dstDelta, const uint maskDelta,
						   const uint32 key) {
	uint32 color;
	byte a, r, g, b;
	uint8 *col = (uint8 *)&color;
#ifdef SCUMM_BIG_ENDIAN
	if (SrcSize == 3 || DstSize == 3)
		col++;
#endif

	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
			if (SrcSize == sizeof(SrcColor))
				color = *(const SrcColor *)src;
			else
				memcpy(col, src, SrcSize);

			if ((!hasKey || color != key) && (!hasMask || *mask != 0)) {
				srcFmt.colorToARGB(color, a, r, g, b);
				color = dstFmt.ARGBToColor(a, r, g, b);

				if (DstSize == sizeof(DstColor))
					*(DstColor *)dst = color;
				else
					memcpy(dst, col, DstSize);
			}

			if (backward) {
				src -= SrcSize;
				dst -= DstSize;
				if (hasMask)
					mask -= 1;
			} else {
				src += SrcSize;
				dst += DstSize;
				if (hasMask)
					mask += 1;
			}
		}

		if (backward) {
			src -= srcDelta;
			dst -= dstDelta;
			if (hasMask)
				mask -= maskDelta;
		} else {
			src += srcDelta;
			dst += dstDelta;
			if (hasMask)
				mask += maskDelta;
		}
	}
}

template<bool hasKey, bool hasMask>
inline bool crossBlitHelper(byte *dst, const byte *src, const byte *mask, const uint w, const uint h,
						   const PixelFormat &srcFmt, const PixelFormat &dstFmt,
						   const uint srcPitch, const uint dstPitch, const uint maskPitch,
						   const uint32 key) {
	// Faster, but larger, to provide optimized handling for each case.
	const uint srcDelta = (srcPitch - w * srcFmt.bytesPerPixel);
	const uint dstDelta = (dstPitch - w * dstFmt.bytesPerPixel);
	const uint maskDelta = hasMask ? (maskPitch - w) : 0;

	// TODO: optimized cases for dstDelta of 0
	if (dstFmt.bytesPerPixel == 2) {
		if (srcFmt.bytesPerPixel == 2) {
			crossBlitLogic<uint16, 2, uint16, 2, false, hasKey, hasMask>(dst, src, mask, w, h, srcFmt, dstFmt, srcDelta, dstDelta, maskDelta, key);
		} else if (srcFmt.bytesPerPixel == 3) {
			crossBlitLogic<uint8, 3, uint16, 2, false, hasKey, hasMask>(dst, src, mask, w, h, srcFmt, dstFmt, srcDelta, dstDelta, maskDelta, key);
		} else {
			crossBlitLogic<uint32, 4, uint16, 2, false, hasKey, hasMask>(dst, src, mask, w, h, srcFmt, dstFmt, srcDelta, dstDelta, maskDelta, key);
		}
	} else if (dstFmt.bytesPerPixel == 3) {
		if (srcFmt.bytesPerPixel == 2) {
			// We need to blit the surface from bottom right to top left here.
			// This is needed, because when we convert to the same memory
			// buffer copying the surface from top left to bottom right would
			// overwrite the source, since we have more bits per destination
			// color than per source color.
			dst += h * dstPitch - dstDelta - dstFmt.bytesPerPixel;
			src += h * srcPitch - srcDelta - srcFmt.bytesPerPixel;
			if (hasMask) mask += h * maskPitch - maskDelta - 1;
			crossBlitLogic<uint16, 2, uint8, 3, true, hasKey, hasMask>(dst, src, mask, w, h, srcFmt, dstFmt, srcDelta, dstDelta, maskDelta, key);
		} else if (srcFmt.bytesPerPixel == 3) {
			crossBlitLogic<uint8, 3, uint8, 3, false, hasKey, hasMask>(dst, src, mask, w, h, srcFmt, dstFmt, srcDelta, dstDelta, maskDelta, key);
		} else {
			crossBlitLogic<uint32, 4, uint8, 3, false, hasKey, hasMask>(dst, src, mask, w, h, srcFmt, dstFmt, srcDelta, dstDelta, maskDelta, key);
		}
	} else if (dstFmt.bytesPerPixel == 4) {
		if (srcFmt.bytesPerPixel == 2) {
			// We need to blit the surface from bottom right to top left here.
			// This is neeeded, because when we convert to the same memory
			// buffer copying the surface from top left to bottom right would
			// overwrite the source, since we have more bits per destination
			// color than per source color.
			dst += h * dstPitch - dstDelta - dstFmt.bytesPerPixel;
			src += h * srcPitch - srcDelta - srcFmt.bytesPerPixel;
			if (hasMask) mask += h * maskPitch - maskDelta - 1;
			crossBlitLogic<uint16, 2, uint32, 4, true, hasKey, hasMask>(dst, src, mask, w, h, srcFmt, dstFmt, srcDelta, dstDelta, maskDelta, key);
		} else if (srcFmt.bytesPerPixel == 3) {
			// We need to blit the surface from bottom right to top left here.
			// This is neeeded, because when we convert to the same memory
			// buffer copying the surface from top left to bottom right would
			// overwrite the source, since we have more bits per destination
			// color than per source color.
			dst += h * dstPitch - dstDelta - dstFmt.bytesPerPixel;
			src += h * srcPitch - srcDelta - srcFmt.bytesPerPixel;
			if (hasMask) mask += h * maskPitch - maskDelta - 1;
			crossBlitLogic<uint8, 3, uint32, 4, true, hasKey, hasMask>(dst, src, mask, w, h, srcFmt, dstFmt, srcDelta, dstDelta, maskDelta, key);
		} else {
			crossBlitLogic<uint32, 4, uint32, 4, false, hasKey, hasMask>(dst, src, mask, w, h, srcFmt, dstFmt, srcDelta, dstDelta, maskDelta, key);
		}
	} else {
		return false;
	}
	return true;
}

} // End of anonymous namespace

// Function to blit a rect from one color format to another
bool crossBlit(byte *dst, const byte *src,
			   const uint dstPitch, const uint srcPitch,
			   const uint w, const uint h,
			   const Graphics::PixelFormat &dstFmt, const Graphics::PixelFormat &srcFmt) {
	// Error out if conversion is impossible
	if ((srcFmt.bytesPerPixel == 1) || (dstFmt.bytesPerPixel == 1)
			 || (!srcFmt.bytesPerPixel) || (!dstFmt.bytesPerPixel))
		return false;

	// Don't perform unnecessary conversion
	if (srcFmt == dstFmt) {
		copyBlit(dst, src, dstPitch, srcPitch, w, h, dstFmt.bytesPerPixel);
		return true;
	}

	return crossBlitHelper<false, false>(dst, src, nullptr, w, h, srcFmt, dstFmt, srcPitch, dstPitch, 0, 0);
}

// Function to blit a rect from one color format to another with a transparent color key
bool crossKeyBlit(byte *dst, const byte *src,
			   const uint dstPitch, const uint srcPitch,
			   const uint w, const uint h,
			   const Graphics::PixelFormat &dstFmt, const Graphics::PixelFormat &srcFmt, const uint32 key) {
	// Error out if conversion is impossible
	if ((srcFmt.bytesPerPixel == 1) || (dstFmt.bytesPerPixel == 1)
			 || (!srcFmt.bytesPerPixel) || (!dstFmt.bytesPerPixel))
		return false;

	// Don't perform unnecessary conversion
	if (srcFmt == dstFmt) {
		keyBlit(dst, src, dstPitch, srcPitch, w, h, dstFmt.bytesPerPixel, key);
		return true;
	}

	return crossBlitHelper<true, false>(dst, src, nullptr, w, h, srcFmt, dstFmt, srcPitch, dstPitch, 0, key);
}

// Function to blit a rect from one color format to another with a transparent color mask
bool crossMaskBlit(byte *dst, const byte *src, const byte *mask,
			   const uint dstPitch, const uint srcPitch, const uint maskPitch,
			   const uint w, const uint h,
			   const Graphics::PixelFormat &dstFmt, const Graphics::PixelFormat &srcFmt) {
	// Error out if conversion is impossible
	if ((srcFmt.bytesPerPixel == 1) || (dstFmt.bytesPerPixel == 1)
			 || (!srcFmt.bytesPerPixel) || (!dstFmt.bytesPerPixel))
		return false;

	// Don't perform unnecessary conversion
	if (srcFmt == dstFmt) {
		maskBlit(dst, src, mask, dstPitch, srcPitch, maskPitch, w, h, dstFmt.bytesPerPixel);
		return true;
	}

	return crossBlitHelper<false, true>(dst, src, mask, w, h, srcFmt, dstFmt, srcPitch, dstPitch, maskPitch, 0);
}

namespace {

template<typename DstColor, int DstSize, bool backward, bool hasKey, bool hasMask>
inline void crossBlitMapLogic(byte *dst, const byte *src, const byte *mask, const uint w, const uint h,
									 const uint srcDelta, const uint dstDelta, const uint maskDelta, const uint32 *map, const uint32 key) {
	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
			const byte color = *src;
			if ((!hasKey || color != key) && (!hasMask || *mask != 0)) {
				if (DstSize == sizeof(DstColor)) {
					*(DstColor *)dst = map[color];
				} else {
					WRITE_UINT24(dst, map[color]);
				}
			}

			if (backward) {
				src -= 1;
				dst -= DstSize;
				if (hasMask)
					mask -= 1;
			} else {
				src += 1;
				dst += DstSize;
				if (hasMask)
					mask += 1;
			}
		}

		if (backward) {
			src -= srcDelta;
			dst -= dstDelta;
			if (hasMask)
				mask -= maskDelta;
		} else {
			src += srcDelta;
			dst += dstDelta;
			if (hasMask)
				mask += maskDelta;
		}
	}
}

template<bool hasKey, bool hasMask>
inline bool crossBlitMapHelperLogic(byte *dst, const byte *src, const byte *mask, const uint w, const uint h,
						   const uint bytesPerPixel, const uint32 *map,
						   const uint srcPitch, const uint dstPitch, const uint maskPitch,
						   const uint32 key) {
	// Faster, but larger, to provide optimized handling for each case.
	const uint srcDelta  = (srcPitch  - w);
	const uint dstDelta  = (dstPitch  - w * bytesPerPixel);
	const uint maskDelta = hasMask ? (maskPitch - w) : 0;

	if (bytesPerPixel == 1) {
		crossBlitMapLogic<uint8, 1, false, hasKey, hasMask>(dst, src, mask, w, h, srcDelta, dstDelta, maskDelta, map, key);
	} else if (bytesPerPixel == 2) {
		// We need to blit the surface from bottom right to top left here.
		// This is neeeded, because when we convert to the same memory
		// buffer copying the surface from top left to bottom right would
		// overwrite the source, since we have more bits per destination
		// color than per source color.
		dst += h * dstPitch - dstDelta - bytesPerPixel;
		src += h * srcPitch - srcDelta - 1;
		if (hasMask) mask += h * maskPitch - maskDelta - 1;
		crossBlitMapLogic<uint16, 2, true, hasKey, hasMask>(dst, src, mask, w, h, srcDelta, dstDelta, maskDelta, map, key);
	} else if (bytesPerPixel == 3) {
		// We need to blit the surface from bottom right to top left here.
		// This is needed, because when we convert to the same memory
		// buffer copying the surface from top left to bottom right would
		// overwrite the source, since we have more bits per destination
		// color than per source color.
		dst += h * dstPitch - dstDelta - bytesPerPixel;
		src += h * srcPitch - srcDelta - 1;
		if (hasMask) mask += h * maskPitch - maskDelta - 1;
		crossBlitMapLogic<uint8, 3, true, hasKey, hasMask>(dst, src, mask, w, h, srcDelta, dstDelta, maskDelta, map, key);
	} else if (bytesPerPixel == 4) {
		// We need to blit the surface from bottom right to top left here.
		// This is needed, because when we convert to the same memory
		// buffer copying the surface from top left to bottom right would
		// overwrite the source, since we have more bits per destination
		// color than per source color.
		dst += h * dstPitch - dstDelta - bytesPerPixel;
		src += h * srcPitch - srcDelta - 1;
		if (hasMask) mask += h * maskPitch - maskDelta - 1;
		crossBlitMapLogic<uint32, 4, true, hasKey, hasMask>(dst, src, mask, w, h, srcDelta, dstDelta, maskDelta, map, key);
	} else {
		return false;
	}
	return true;
}

} // End of anonymous namespace

// Function to blit a rect from one color format to another using a map
bool crossBlitMap(byte *dst, const byte *src,
			   const uint dstPitch, const uint srcPitch,
			   const uint w, const uint h,
			   const uint bytesPerPixel, const uint32 *map) {
	// Error out if conversion is impossible
	if (!bytesPerPixel)
		return false;

	return crossBlitMapHelperLogic<false, false>(dst, src, nullptr, w, h, bytesPerPixel, map, srcPitch, dstPitch, 0, 0);
}

// Function to blit a rect from one color format to another using a map with a transparent color key
bool crossKeyBlitMap(byte *dst, const byte *src,
			   const uint dstPitch, const uint srcPitch,
			   const uint w, const uint h,
			   const uint bytesPerPixel, const uint32 *map, const uint32 key) {
	// Error out if conversion is impossible
	if (!bytesPerPixel)
		return false;

	return crossBlitMapHelperLogic<true, false>(dst, src, nullptr, w, h, bytesPerPixel, map, srcPitch, dstPitch, 0, key);
}

// Function to blit a rect from one color format to another using a map with a transparent color mask
bool crossMaskBlitMap(byte *dst, const byte *src, const byte *mask,
			   const uint dstPitch, const uint srcPitch, const uint maskPitch,
			   const uint w, const uint h,
			   const uint bytesPerPixel, const uint32 *map) {
	// Error out if conversion is impossible
	if (!bytesPerPixel)
		return false;

	return crossBlitMapHelperLogic<false, true>(dst, src, mask, w, h, bytesPerPixel, map, srcPitch, dstPitch, maskPitch, 0);
}

} // End of namespace Graphics
