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

static void bompApplyMask(byte *line_buffer, byte *mask_out, byte bits, int32 size);
static void bompApplyShadow0(const byte *line_buffer, byte *dst, int32 size, byte transparency);
static void bompApplyShadow1(const byte *shadowPalette, const byte *line_buffer, byte *dst, int32 size, byte transparency);
static void bompApplyShadow3(const byte *shadowPalette, const byte *line_buffer, byte *dst, int32 size, byte transparency);
static void bompApplyActorPalette(byte *actorPalette, byte *line_buffer, int32 size);



void decompressBomp(byte *dst, const byte *src, int w, int h) {
	assert(w > 0);
	assert(h > 0);

	do {
#if 0
		bompDecodeLine(dst, src + 2, w);
		src += READ_LE_UINT16(src) + 2;
#else
		int len, num;
		byte code, color;
	
		len = w;
		src += 2;
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
#endif
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

void bompApplyMask(byte *line_buffer, byte *mask_src, byte bits, int32 size) {
	while(1) {
		byte tmp = *(mask_src++);
		do {
			if (size-- == 0) 
				return;
			if (tmp & bits) {
				*(line_buffer) = 255;
			}
			line_buffer++;
			bits >>= 1;
		} while	(bits);
		bits = 128;
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
				tmp = shadowPalette[*(dst)];
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
		while(1) {
			if (size-- == 0)
				break;
			*line_buffer = actorPalette[*line_buffer];
			line_buffer++;
		}
	}
}

void bompScaleFuncX(byte *line_buffer, byte *scaling_x_ptr, byte skip, int32 size) {
	byte * line_ptr1 = line_buffer;
	byte * line_ptr2 = line_buffer;

	byte tmp = *(scaling_x_ptr++);

	while (size--) {
		if ((skip & tmp) == 0) {
			*(line_ptr1++) = *(line_ptr2);
		}
		line_ptr2++;
		skip >>= 1;
		if (skip == 0) {
			skip = 128;
			tmp = *(scaling_x_ptr++);
		}
	}
}

void Scumm::drawBomp(const BompDrawData &bd, int decode_mode, int mask) {
	byte skip_y = 128;
	byte skip_y_new = 0;
	byte bits;
	byte *mask_out = 0;
	byte *charset_mask;
	byte tmp;
	int32 clip_left, clip_right, clip_top, clip_bottom, tmp_x, tmp_y, mask_offset, mask_pitch;
	byte *scalingYPtr = bd.scalingYPtr;

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

	clip_right = bd.srcwidth - clip_left;
	tmp_x = bd.x + bd.srcwidth;
	if (tmp_x > bd.outwidth) {
		clip_right -= tmp_x - bd.outwidth;
	}

	clip_bottom = bd.srcheight;
	tmp_y = bd.y + bd.srcheight;
	if (tmp_y > bd.outheight) {
		clip_bottom -= tmp_y - bd.outheight;
	}

	const byte *src = bd.dataptr;
	byte *dst = bd.out + bd.y * bd.outwidth + bd.x + clip_left;

	mask_pitch = _screenWidth / 8;
	mask_offset = _screenStartStrip + (bd.y * mask_pitch) + ((bd.x + clip_left) >> 3);

	charset_mask = getResourceAddress(rtBuffer, 9) + mask_offset;
	bits = 128 >> ((bd.x + clip_left) & 7);

	if (mask == 1) {
		mask_out = bd.maskPtr + mask_offset;
	}

	if (mask == 3) {
		if (scalingYPtr != NULL) {
			skip_y_new = *(scalingYPtr++);
		}

		if ((clip_right + clip_left) > bd.scaleRight) {
			clip_right = bd.scaleRight - clip_left;
		}

		if (clip_bottom > bd.scaleBottom) {
			clip_bottom = bd.scaleBottom;
		}
	}

	if ((clip_right <= 0) || (clip_bottom <= 0))
		return;

	int32 pos_y = 0;

	byte line_buffer[1024];

	byte *line_ptr = line_buffer + clip_left;

	while(1) {
		switch(decode_mode) {
		case 0:
			memcpy(line_buffer, src, bd.srcwidth);
			src += bd.srcwidth;
			break;
		case 1:
			bompDecodeLine(line_buffer, src + 2, bd.srcwidth);
			src += READ_LE_UINT16(src) + 2;
			break;
		case 3:
			bompDecodeLineReverse(line_buffer, src + 2, bd.srcwidth);
			src += READ_LE_UINT16(src) + 2;
			break;
		default:
			error("Unknown bomp decode_mode %d", decode_mode);
		}

		if (mask == 3) {
			if (bd.scale_y != 255) {
				tmp = skip_y_new & skip_y;
				skip_y >>= 1;
				if (skip_y == 0) {
					skip_y = 128;
					skip_y_new = *(scalingYPtr++);
				}

				if (tmp != 0) 
					continue;
			}

			if (bd.scale_x != 255) {
				bompScaleFuncX(line_buffer, bd.scalingXPtr, 128, bd.srcwidth);
			}
		}

		if (clip_top > 0) {
			clip_top--;
		} else {

			if (mask == 1) {
				bompApplyMask(line_ptr, mask_out, bits, clip_right);
			}
	
			bompApplyMask(line_ptr, charset_mask, bits, clip_right);
			bompApplyActorPalette(_bompActorPalettePtr, line_ptr, clip_right);
			bompApplyShadow(bd.shadowMode, _shadowPalette, line_ptr, dst, clip_right, 255);
		}

		mask_out += mask_pitch;
		charset_mask += mask_pitch;
		pos_y++;
		dst += bd.outwidth;
		if (pos_y >= clip_bottom)
			break;
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

