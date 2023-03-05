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

namespace Graphics {

// see graphics/blit-atari.cpp, Atari Falcon's SuperVidel addon allows accelerated blitting
#ifndef USE_SV_BLITTER
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

template<typename Size>
inline void keyBlitLogic(byte *dst, const byte *src, const uint w, const uint h,
						 const uint srcDelta, const uint dstDelta, const uint32 key) {
	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
			uint32 color = *(const Size *)src;
			if (color != key)
				*(Size *)dst = color;

			src += sizeof(Size);
			dst += sizeof(Size);
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
		keyBlitLogic<uint8>(dst, src, w, h, srcDelta, dstDelta, key);
	} else if (bytesPerPixel == 2) {
		keyBlitLogic<uint16>(dst, src, w, h, srcDelta, dstDelta, key);
	} else if (bytesPerPixel == 4) {
		keyBlitLogic<uint32>(dst, src, w, h, srcDelta, dstDelta, key);
	} else {
		return false;
	}

	return true;
}

namespace {

template<typename SrcColor, typename DstColor, bool backward, bool hasKey>
inline void crossBlitLogic(byte *dst, const byte *src, const uint w, const uint h,
						   const PixelFormat &srcFmt, const PixelFormat &dstFmt,
						   const uint srcDelta, const uint dstDelta, const uint32 key) {
	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
			const uint32 color = *(const SrcColor *)src;
			if (!hasKey || color != key) {
				byte a, r, g, b;
				srcFmt.colorToARGB(color, a, r, g, b);
				*(DstColor *)dst = dstFmt.ARGBToColor(a, r, g, b);
			}

			if (backward) {
				src -= sizeof(SrcColor);
				dst -= sizeof(DstColor);
			} else {
				src += sizeof(SrcColor);
				dst += sizeof(DstColor);
			}
		}

		if (backward) {
			src -= srcDelta;
			dst -= dstDelta;
		} else {
			src += srcDelta;
			dst += dstDelta;
		}
	}
}

template<typename DstColor, bool backward, bool hasKey>
inline void crossBlitLogic1BppSource(byte *dst, const byte *src, const uint w, const uint h,
									 const uint srcDelta, const uint dstDelta, const uint32 *map, const uint32 key) {
	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
			const byte color = *src;
			if (!hasKey || color != key) {
				*(DstColor *)dst = map[color];
			}

			if (backward) {
				src -= 1;
				dst -= sizeof(DstColor);
			} else {
				src += 1;
				dst += sizeof(DstColor);
			}
		}

		if (backward) {
			src -= srcDelta;
			dst -= dstDelta;
		} else {
			src += srcDelta;
			dst += dstDelta;
		}
	}
}

template<typename DstColor, bool backward, bool hasKey>
inline void crossBlitLogic3BppSource(byte *dst, const byte *src, const uint w, const uint h,
									 const PixelFormat &srcFmt, const PixelFormat &dstFmt,
									 const uint srcDelta, const uint dstDelta, const uint32 key) {
	uint32 color;
	byte r, g, b, a;
	uint8 *col = (uint8 *)&color;
#ifdef SCUMM_BIG_ENDIAN
	col++;
#endif
	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
			memcpy(col, src, 3);
			if (!hasKey || color != key) {
				srcFmt.colorToARGB(color, a, r, g, b);
				*(DstColor *)dst = dstFmt.ARGBToColor(a, r, g, b);
			}

			if (backward) {
				src -= 3;
				dst -= sizeof(DstColor);
			} else {
				src += 3;
				dst += sizeof(DstColor);
			}
		}

		if (backward) {
			src -= srcDelta;
			dst -= dstDelta;
		} else {
			src += srcDelta;
			dst += dstDelta;
		}
	}
}

} // End of anonymous namespace

// Function to blit a rect from one color format to another
bool crossBlit(byte *dst, const byte *src,
			   const uint dstPitch, const uint srcPitch,
			   const uint w, const uint h,
			   const Graphics::PixelFormat &dstFmt, const Graphics::PixelFormat &srcFmt) {
	// Error out if conversion is impossible
	if ((srcFmt.bytesPerPixel == 1) || (dstFmt.bytesPerPixel == 1)
			 || (dstFmt.bytesPerPixel == 3)
			 || (!srcFmt.bytesPerPixel) || (!dstFmt.bytesPerPixel))
		return false;

	// Don't perform unnecessary conversion
	if (srcFmt == dstFmt) {
		copyBlit(dst, src, dstPitch, srcPitch, w, h, dstFmt.bytesPerPixel);
		return true;
	}

	// Faster, but larger, to provide optimized handling for each case.
	const uint srcDelta = (srcPitch - w * srcFmt.bytesPerPixel);
	const uint dstDelta = (dstPitch - w * dstFmt.bytesPerPixel);

	// TODO: optimized cases for dstDelta of 0
	if (dstFmt.bytesPerPixel == 2) {
		if (srcFmt.bytesPerPixel == 2) {
			crossBlitLogic<uint16, uint16, false, false>(dst, src, w, h, srcFmt, dstFmt, srcDelta, dstDelta, 0);
		} else if (srcFmt.bytesPerPixel == 3) {
			crossBlitLogic3BppSource<uint16, false, false>(dst, src, w, h, srcFmt, dstFmt, srcDelta, dstDelta, 0);
		} else {
			crossBlitLogic<uint32, uint16, false, false>(dst, src, w, h, srcFmt, dstFmt, srcDelta, dstDelta, 0);
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
			crossBlitLogic<uint16, uint32, true, false>(dst, src, w, h, srcFmt, dstFmt, srcDelta, dstDelta, 0);
		} else if (srcFmt.bytesPerPixel == 3) {
			// We need to blit the surface from bottom right to top left here.
			// This is neeeded, because when we convert to the same memory
			// buffer copying the surface from top left to bottom right would
			// overwrite the source, since we have more bits per destination
			// color than per source color.
			dst += h * dstPitch - dstDelta - dstFmt.bytesPerPixel;
			src += h * srcPitch - srcDelta - srcFmt.bytesPerPixel;
			crossBlitLogic3BppSource<uint32, true, false>(dst, src, w, h, srcFmt, dstFmt, srcDelta, dstDelta, 0);
		} else {
			crossBlitLogic<uint32, uint32, false, false>(dst, src, w, h, srcFmt, dstFmt, srcDelta, dstDelta, 0);
		}
	} else {
		return false;
	}
	return true;
}

// Function to blit a rect from one color format to another with a transparent color key
bool crossKeyBlit(byte *dst, const byte *src,
			   const uint dstPitch, const uint srcPitch,
			   const uint w, const uint h,
			   const Graphics::PixelFormat &dstFmt, const Graphics::PixelFormat &srcFmt, const uint32 key) {
	// Error out if conversion is impossible
	if ((srcFmt.bytesPerPixel == 1) || (dstFmt.bytesPerPixel == 1)
			 || (dstFmt.bytesPerPixel == 3)
			 || (!srcFmt.bytesPerPixel) || (!dstFmt.bytesPerPixel))
		return false;

	// Don't perform unnecessary conversion
	if (srcFmt == dstFmt) {
		keyBlit(dst, src, dstPitch, srcPitch, w, h, dstFmt.bytesPerPixel, key);
		return true;
	}

	// Faster, but larger, to provide optimized handling for each case.
	const uint srcDelta = (srcPitch - w * srcFmt.bytesPerPixel);
	const uint dstDelta = (dstPitch - w * dstFmt.bytesPerPixel);

	// TODO: optimized cases for dstDelta of 0
	if (dstFmt.bytesPerPixel == 2) {
		if (srcFmt.bytesPerPixel == 2) {
			crossBlitLogic<uint16, uint16, false, true>(dst, src, w, h, srcFmt, dstFmt, srcDelta, dstDelta, key);
		} else if (srcFmt.bytesPerPixel == 3) {
			crossBlitLogic3BppSource<uint16, false, true>(dst, src, w, h, srcFmt, dstFmt, srcDelta, dstDelta, key);
		} else {
			crossBlitLogic<uint32, uint16, false, true>(dst, src, w, h, srcFmt, dstFmt, srcDelta, dstDelta, key);
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
			crossBlitLogic<uint16, uint32, true, true>(dst, src, w, h, srcFmt, dstFmt, srcDelta, dstDelta, key);
		} else if (srcFmt.bytesPerPixel == 3) {
			// We need to blit the surface from bottom right to top left here.
			// This is neeeded, because when we convert to the same memory
			// buffer copying the surface from top left to bottom right would
			// overwrite the source, since we have more bits per destination
			// color than per source color.
			dst += h * dstPitch - dstDelta - dstFmt.bytesPerPixel;
			src += h * srcPitch - srcDelta - srcFmt.bytesPerPixel;
			crossBlitLogic3BppSource<uint32, true, true>(dst, src, w, h, srcFmt, dstFmt, srcDelta, dstDelta, key);
		} else {
			crossBlitLogic<uint32, uint32, false, true>(dst, src, w, h, srcFmt, dstFmt, srcDelta, dstDelta, key);
		}
	} else {
		return false;
	}
	return true;
}

// Function to blit a rect from one color format to another using a map
bool crossBlitMap(byte *dst, const byte *src,
			   const uint dstPitch, const uint srcPitch,
			   const uint w, const uint h,
			   const uint bytesPerPixel, const uint32 *map) {
	// Error out if conversion is impossible
	if ((bytesPerPixel == 3) || (!bytesPerPixel))
		return false;

	// Faster, but larger, to provide optimized handling for each case.
	const uint srcDelta = (srcPitch - w);
	const uint dstDelta = (dstPitch - w * bytesPerPixel);

	if (bytesPerPixel == 1) {
		crossBlitLogic1BppSource<uint8, false, false>(dst, src, w, h, srcDelta, dstDelta, map, 0);
	} else if (bytesPerPixel == 2) {
		// We need to blit the surface from bottom right to top left here.
		// This is neeeded, because when we convert to the same memory
		// buffer copying the surface from top left to bottom right would
		// overwrite the source, since we have more bits per destination
		// color than per source color.
		dst += h * dstPitch - dstDelta - bytesPerPixel;
		src += h * srcPitch - srcDelta - 1;
		crossBlitLogic1BppSource<uint16, true, false>(dst, src, w, h, srcDelta, dstDelta, map, 0);
	} else if (bytesPerPixel == 4) {
		// We need to blit the surface from bottom right to top left here.
		// This is neeeded, because when we convert to the same memory
		// buffer copying the surface from top left to bottom right would
		// overwrite the source, since we have more bits per destination
		// color than per source color.
		dst += h * dstPitch - dstDelta - bytesPerPixel;
		src += h * srcPitch - srcDelta - 1;
		crossBlitLogic1BppSource<uint32, true, false>(dst, src, w, h, srcDelta, dstDelta, map, 0);
	} else {
		return false;
	}
	return true;
}

// Function to blit a rect from one color format to another using a map with a transparent color key
bool crossKeyBlitMap(byte *dst, const byte *src,
			   const uint dstPitch, const uint srcPitch,
			   const uint w, const uint h,
			   const uint bytesPerPixel, const uint32 *map, const uint32 key) {
	// Error out if conversion is impossible
	if ((bytesPerPixel == 3) || (!bytesPerPixel))
		return false;

	// Faster, but larger, to provide optimized handling for each case.
	const uint srcDelta = (srcPitch - w);
	const uint dstDelta = (dstPitch - w * bytesPerPixel);

	if (bytesPerPixel == 1) {
		crossBlitLogic1BppSource<uint8, false, true>(dst, src, w, h, srcDelta, dstDelta, map, key);
	} else if (bytesPerPixel == 2) {
		// We need to blit the surface from bottom right to top left here.
		// This is neeeded, because when we convert to the same memory
		// buffer copying the surface from top left to bottom right would
		// overwrite the source, since we have more bits per destination
		// color than per source color.
		dst += h * dstPitch - dstDelta - bytesPerPixel;
		src += h * srcPitch - srcDelta - 1;
		crossBlitLogic1BppSource<uint16, true, true>(dst, src, w, h, srcDelta, dstDelta, map, key);
	} else if (bytesPerPixel == 4) {
		// We need to blit the surface from bottom right to top left here.
		// This is neeeded, because when we convert to the same memory
		// buffer copying the surface from top left to bottom right would
		// overwrite the source, since we have more bits per destination
		// color than per source color.
		dst += h * dstPitch - dstDelta - bytesPerPixel;
		src += h * srcPitch - srcDelta - 1;
		crossBlitLogic1BppSource<uint32, true, true>(dst, src, w, h, srcDelta, dstDelta, map, key);
	} else {
		return false;
	}
	return true;
}

} // End of namespace Graphics
