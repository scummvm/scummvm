/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "scumm.h"
#include "bomp.h"


static void bompScaleFuncX(byte *line_buffer, byte *scaling_x_ptr, byte skip, int32 size);

static void bompDecodeLineReverse(byte *dst, const byte *src, int size);

static void bompApplyShadow0(const byte *line_buffer, byte *dst, int32 size, byte transparency);
static void bompApplyShadow1(const byte *shadowPalette, const byte *line_buffer, byte *dst, int32 size, byte transparency);
static void bompApplyShadow3(const byte *shadowPalette, const byte *line_buffer, byte *dst, int32 size, byte transparency);
static void bompApplyActorPalette(byte *actorPalette, byte *line_buffer, int32 size);



void decompressBomp(byte *dst, const byte *src, int w, int h) {
	assert(w > 0);
	assert(h > 0);

	do {
		bompDecodeLine(dst, src + 2, w);
		src += READ_LE_UINT16(src) + 2;
		dst += w;
	} while (--h);
}

void bompDecodeLine(byte *dst, const byte *src, int size) {
	assert(size > 0);

	int len, num;
	byte code, color;

	len = size;
	while (len) {
		code = *src++;
		num = (code >> 1) + 1;
		if (num > len)
			num = len;
		len -= num;
		if (code & 1) {
			color = *src++;
			memset(dst, color, num);
		} else {
			memcpy(dst, src, num);
			src += num;
		}
		dst += num;
	}
}

void bompDecodeLineReverse(byte *dst, const byte *src, int size) {
	assert(size > 0);

	dst += size;
	
	int len, num;
	byte code, color;

	len = size;
	while (len) {
		code = *src++;
		num = (code >> 1) + 1;
		if (num > len)
			num = len;
		len -= num;
		dst -= num;
		if (code & 1) {
			color = *src++;
			memset(dst, color, num);
		} else {
			memcpy(dst, src, num);
			src += num;
		}
	}
}

void bompApplyMask(byte *line_buffer, byte *mask, byte maskbit, int32 size, byte transparency) {
	while(1) {
		do {
			if (size-- == 0) 
				return;
			if (*mask & maskbit) {
				*line_buffer = transparency;
			}
			line_buffer++;
			maskbit >>= 1;
		} while	(maskbit);
		mask++;
		maskbit = 128;
	}
}

void bompApplyShadow(int shadowMode, const byte *shadowPalette, const byte *line_buffer, byte *dst, int32 size, byte transparency) {
	assert(size > 0);
	switch(shadowMode) {
	case 0:
		bompApplyShadow0(line_buffer, dst, size, transparency);
		break;
	case 1:
		bompApplyShadow1(shadowPalette, line_buffer, dst, size, transparency);
		break;
	case 3:
		bompApplyShadow3(shadowPalette, line_buffer, dst, size, transparency);
		break;
	default:
		error("Unknown shadow mode %d", shadowMode);
	}
}
void bompApplyShadow0(const byte *line_buffer, byte *dst, int32 size, byte transparency) {
	while(size-- > 0) {
		byte tmp = *line_buffer++;
		if (tmp != transparency) {
			*dst = tmp;
		}
		dst++;
	}
}

void bompApplyShadow1(const byte *shadowPalette, const byte *line_buffer, byte *dst, int32 size, byte transparency) {
	while(size-- > 0) {
		byte tmp = *line_buffer++;
		if (tmp != transparency) {
			if (tmp == 13) {
				tmp = shadowPalette[*dst];
			}
			*dst = tmp;
		}
		dst++;
	}
}

void bompApplyShadow3(const byte *shadowPalette, const byte *line_buffer, byte *dst, int32 size, byte transparency) {
	while(size-- > 0) {
		byte tmp = *line_buffer++;
		if (tmp != transparency) {
			if (tmp < 8) {
				tmp = shadowPalette[*dst + (tmp << 8)];
			}
			*dst = tmp;
		}
		dst++;
	}
}

void bompApplyActorPalette(byte *actorPalette, byte *line_buffer, int32 size) {
	if (actorPalette != 0) {
		actorPalette[255] = 255;
		while (size-- > 0) {
			*line_buffer = actorPalette[*line_buffer];
			line_buffer++;
		}
	}
}

void bompScaleFuncX(byte *line_buffer, byte *scaling_x_ptr, byte skip, int32 size) {
	byte *line_ptr1 = line_buffer;
	byte *line_ptr2 = line_buffer;

	byte tmp = *scaling_x_ptr++;

	while (size--) {
		if ((skip & tmp) == 0) {
			*line_ptr1++ = *line_ptr2;
		}
		line_ptr2++;
		skip >>= 1;
		if (skip == 0) {
			skip = 128;
			tmp = *scaling_x_ptr++;
		}
	}
}

void Scumm::drawBomp(const BompDrawData &bd, bool mirror) {
	const byte *src;
	byte *dst;
	byte maskbit;
	byte *mask = 0;
	int mask_offset;
	byte *charset_mask;
	int clip_left, clip_right, clip_top, clip_bottom;
	byte *scalingYPtr = bd.scalingYPtr;
	byte skip_y_bits = 0x80;
	byte skip_y_new = 0;
	byte tmp;


	if (bd.x < 0) {
		clip_left = -bd.x;
	} else {
		clip_left = 0;
	}

	if (bd.y < 0) {
		clip_top = -bd.y;
	} else {
		clip_top = 0;
	}

	clip_right = bd.srcwidth;
	if (clip_right > bd.outwidth - bd.x) {
		clip_right = bd.outwidth - bd.x;
	}

	clip_bottom = bd.srcheight;
	if (clip_bottom > bd.outheight - bd.y) {
		clip_bottom = bd.outheight - bd.y;
	}

	src = bd.dataptr;
	dst = bd.out + bd.y * bd.outwidth + bd.x + clip_left;

	mask_offset = _screenStartStrip + (bd.y * gdi._numStrips) + ((bd.x + clip_left) >> 3);
	maskbit = revBitMask[(bd.x + clip_left) & 7];

	// Always mask against the charset mask
	charset_mask = getResourceAddress(rtBuffer, 9) + mask_offset;

	// Also mask against any additionally imposed mask
	if (bd.maskPtr) {
		mask = bd.maskPtr + mask_offset;
	}

	// Setup vertical scaling
	if (bd.scale_y != 255) {
		assert(scalingYPtr);

		skip_y_new = *scalingYPtr++;
		skip_y_bits = 0x80;

		if (clip_bottom > bd.scaleBottom) {
			clip_bottom = bd.scaleBottom;
		}
	}

	// Setup horizontal scaling
	if (bd.scale_x != 255) {
		assert(bd.scalingXPtr);
		if (clip_right > bd.scaleRight) {
			clip_right = bd.scaleRight;
		}
	}

	const int width = clip_right - clip_left;

	if (width <= 0)
		return;

	int pos_y = 0;
	byte line_buffer[1024];

	byte *line_ptr = line_buffer + clip_left;

	// Loop over all lines
	while (pos_y < clip_bottom) {
		// Decode a single (bomp encoded) line, reversed if we are in mirror mode
		if (mirror)
			bompDecodeLineReverse(line_buffer, src + 2, bd.srcwidth);
		else
			bompDecodeLine(line_buffer, src + 2, bd.srcwidth);
		src += READ_LE_UINT16(src) + 2;

		// If vertical scaling is enabled, do it
		if (bd.scale_y != 255) {
			// A bit set means we should skip this line...
			tmp = skip_y_new & skip_y_bits;
			
			// Advance the scale-skip bit mask, if it's 0, get the next scale-skip byte
			skip_y_bits >>= 1;
			if (skip_y_bits == 0) {
				skip_y_bits = 0x80;
				skip_y_new = *scalingYPtr++;
			}

			// Skip the current line if the above check tells us to
			if (tmp != 0) 
				continue;
		}

		// Perform horizontal scaling
		if (bd.scale_x != 255) {
			bompScaleFuncX(line_buffer, bd.scalingXPtr, 0x80, bd.srcwidth);
		}

		// The first clip_top lines are to be clipped, i.e. not drawn
		if (clip_top > 0) {
			clip_top--;
		} else {
			// Replace the parts of the line which are masked with the transparency color
			if (bd.maskPtr)
				bompApplyMask(line_ptr, mask, maskbit, width, 255);
			bompApplyMask(line_ptr, charset_mask, maskbit, width, 255);
	
			// Apply custom color map, if available
			if (_bompActorPalettePtr)
				bompApplyActorPalette(_bompActorPalettePtr, line_ptr, width);
			
			// Finally, draw the decoded, scaled, masked and recolored line onto
			// the target surface, using the specified shadow mode
			bompApplyShadow(bd.shadowMode, _shadowPalette, line_ptr, dst, width, 255);
		}

		// Advance to the next line
		pos_y++;
		mask += gdi._numStrips;
		charset_mask += gdi._numStrips;
		dst += bd.outwidth;
	}
}


static byte _bompScaleTable[] = {
    0, 128,  64, 192,  32, 160,  96, 224,
   16, 144,  80, 208,  48, 176, 112, 240,
    8, 136,  72, 200,  40, 168, 104, 232,
   24, 152,  88, 216,  56, 184, 120, 248,
    4, 132,  68, 196,  36, 164, 100, 228,
   20, 148,  84, 212,  52, 180, 116, 244,
   12, 140,  76, 204,  44, 172, 108, 236,
   28, 156,  92, 220,  60, 188, 124, 252,
    2, 130,  66, 194,  34, 162,  98, 226,
   18, 146,  82, 210,  50, 178, 114, 242,
   10, 138,  74, 202,  42, 170, 106, 234,
   26, 154,  90, 218,  58, 186, 122, 250,
    6, 134,  70, 198,  38, 166, 102, 230,
   22, 150,  86, 214,  54, 182, 118, 246,
   14, 142,  78, 206,  46, 174, 110, 238,
   30, 158,  94, 222,  62, 190, 126, 254,
    1, 129,  65, 193,  33, 161,  97, 225,
   17, 145,  81, 209,  49, 177, 113, 241,
    9, 137,  73, 201,  41, 169, 105, 233,
   25, 153,  89, 217,  57, 185, 121, 249,
    5, 133,  69, 197,  37, 165, 101, 229,
   21, 149,  85, 213,  53, 181, 117, 245,
   13, 141,  77, 205,  45, 173, 109, 237,
   29, 157,  93, 221,  61, 189, 125, 253,
    3, 131,  67, 195,  35, 163,  99, 227,
   19, 147,  83, 211,  51, 179, 115, 243,
   11, 139,  75, 203,  43, 171, 107, 235,
   27, 155,  91, 219,  59, 187, 123, 251,
    7, 135,  71, 199,  39, 167, 103, 231,
   23, 151,  87, 215,  55, 183, 119, 247,
   15, 143,  79, 207,  47, 175, 111, 239,
   31, 159,  95, 223,  63, 191, 127, 255,

    0, 128,  64, 192,  32, 160,  96, 224,
   16, 144,  80, 208,  48, 176, 112, 240,
    8, 136,  72, 200,  40, 168, 104, 232,
   24, 152,  88, 216,  56, 184, 120, 248,
    4, 132,  68, 196,  36, 164, 100, 228,
   20, 148,  84, 212,  52, 180, 116, 244,
   12, 140,  76, 204,  44, 172, 108, 236,
   28, 156,  92, 220,  60, 188, 124, 252,
    2, 130,  66, 194,  34, 162,  98, 226,
   18, 146,  82, 210,  50, 178, 114, 242,
   10, 138,  74, 202,  42, 170, 106, 234,
   26, 154,  90, 218,  58, 186, 122, 250,
    6, 134,  70, 198,  38, 166, 102, 230,
   22, 150,  86, 214,  54, 182, 118, 246,
   14, 142,  78, 206,  46, 174, 110, 238,
   30, 158,  94, 222,  62, 190, 126, 254,
    1, 129,  65, 193,  33, 161,  97, 225,
   17, 145,  81, 209,  49, 177, 113, 241,
    9, 137,  73, 201,  41, 169, 105, 233,
   25, 153,  89, 217,  57, 185, 121, 249,
    5, 133,  69, 197,  37, 165, 101, 229,
   21, 149,  85, 213,  53, 181, 117, 245,
   13, 141,  77, 205,  45, 173, 109, 237,
   29, 157,  93, 221,  61, 189, 125, 253,
    3, 131,  67, 195,  35, 163,  99, 227,
   19, 147,  83, 211,  51, 179, 115, 243,
   11, 139,  75, 203,  43, 171, 107, 235,
   27, 155,  91, 219,  59, 187, 123, 251,
    7, 135,  71, 199,  39, 167, 103, 231,
   23, 151,  87, 215,  55, 183, 119, 247,
   15, 143,  79, 207,  47, 175, 111, 239,
   31, 159,  95, 223,  63, 191, 127, 255,
};

static byte _bompBitsTable[] = {
	8, 7, 7, 6, 7, 6, 6, 5, 7, 6, 6, 5, 6, 5, 5, 4,
	7, 6, 6, 5, 6, 5, 5, 4, 6, 5, 5, 4, 5, 4, 4, 3,
	7, 6, 6, 5, 6, 5, 5, 4, 6, 5, 5, 4, 5, 4, 4, 3,
	6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2,
	7, 6, 6, 5, 6, 5, 5, 4, 6, 5, 5, 4, 5, 4, 4, 3,
	6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2,
	6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2,
	5, 4, 4, 3, 4, 3, 3, 2, 4, 3, 3, 2, 3, 2, 2, 1,
	7, 6, 6, 5, 6, 5, 5, 4, 6, 5, 5, 4, 5, 4, 4, 3,
	6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2,
	6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2,
	5, 4, 4, 3, 4, 3, 3, 2, 4, 3, 3, 2, 3, 2, 2, 1,
	6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2,
	5, 4, 4, 3, 4, 3, 3, 2, 4, 3, 3, 2, 3, 2, 2, 1,
	5, 4, 4, 3, 4, 3, 3, 2, 4, 3, 3, 2, 3, 2, 2, 1,
	4, 3, 3, 2, 3, 2, 2, 1, 3, 2, 2, 1, 2, 1, 1, 0,
};

int32 setupBompScale(byte * scaling, int32 size, byte scale) {
	uint32 tmp = (256 - (size >> 1));
	int32 count = (size + 7) >> 3;
	assert(tmp < sizeof(_bompScaleTable));
	byte * tmp_ptr = _bompScaleTable + tmp;
	byte * tmp_scaling = scaling;
	byte a = 0;

	while((count--) != 0) {
		tmp = *(tmp_ptr + 3);
		a <<= 1;
		if (scale < tmp) {
			a |= 1;
		}
		tmp = *(tmp_ptr + 2);
		a <<= 1;
		if (scale < tmp) {
			a |= 1;
		}
		tmp = *(tmp_ptr + 1);
		a <<= 1;
		if (scale < tmp) {
			a |= 1;
		}
		tmp = *(tmp_ptr + 0);
		a <<= 1;
		if (scale < tmp) {
			a |= 1;
		}
		tmp_ptr += 4;

		tmp = *(tmp_ptr + 3);
		a <<= 1;
		if (scale < tmp) {
			a |= 1;
		}
		tmp = *(tmp_ptr + 2);
		a <<= 1;
		if (scale < tmp) {
			a |= 1;
		}
		tmp = *(tmp_ptr + 1);
		a <<= 1;
		if (scale < tmp) {
			a |= 1;
		}
		tmp = *(tmp_ptr + 0);
		a <<= 1;
		if (scale < tmp) {
			a |= 1;
		}
		tmp_ptr += 4;

		*(tmp_scaling++) = a;
	}
	if ((size & 7) != 0) {
		*(tmp_scaling - 1) |= revBitMask[size & 7];
	}

	count = (size + 7) >> 3;
	byte ret_value = 0;
	while(count--) {
		tmp = *scaling++;
		assert(tmp < sizeof(_bompBitsTable));
		ret_value += _bompBitsTable[tmp];
	}

	return ret_value;
}

