/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "common/endian.h"
#include "common/util.h"
#include "common/textconsole.h"

#include "engines/grim/movie/codecs/blocky16.h"

namespace Grim {

#if defined(SCUMM_NEED_ALIGNMENT)

#define COPY_4X1_LINE(dst, src)			\
	do {					\
		(dst)[0] = (src)[0];	\
		(dst)[1] = (src)[1];	\
		(dst)[2] = (src)[2];	\
		(dst)[3] = (src)[3];	\
	} while (0)

#if defined(SCUMM_BIG_ENDIAN)

#define WRITE_2X1_LINE(dst, v)		\
	do {				\
		(dst)[0] = (byte)((v >> 8) & 0xFF);	\
		(dst)[1] = (byte)((v >> 0) & 0xFF);	\
	} while (0)

#define WRITE_4X1_LINE(dst, v)		\
	do {				\
		(dst)[0] = (byte)((v >> 24) & 0xFF);	\
		(dst)[1] = (byte)((v >> 16) & 0XFF);	\
		(dst)[2] = (byte)((v >>  8) & 0xFF);	\
		(dst)[3] = (byte)((v >>  0) & 0xFF);	\
	} while (0)

#else /* SCUMM_BIG_ENDIAN */

#define WRITE_2X1_LINE(dst, v)		\
	do {				\
		(dst)[0] = (byte)((v >> 0) & 0xFF);	\
		(dst)[1] = (byte)((v >> 8) & 0xFF);	\
	} while (0)

#define WRITE_4X1_LINE(dst, v)		\
	do {				\
		(dst)[0] = (byte)((v >>  0) & 0xFF);	\
		(dst)[1] = (byte)((v >>  8) & 0XFF);	\
		(dst)[2] = (byte)((v >> 16) & 0xFF);	\
		(dst)[3] = (byte)((v >> 24) & 0xFF);	\
	} while (0)

#endif

#else /* SCUMM_NEED_ALIGNMENT */

#define COPY_4X1_LINE(dst, src)			\
	*(uint32 *)(dst) = *(const uint32 *)(src);

#define WRITE_2X1_LINE(dst, v)		\
	*(uint16 *)(dst) = v;

#define WRITE_4X1_LINE(dst, v)		\
	*(uint32 *)(dst) = v;

#endif

static int8 blocky16_table_small1[] = {
	0, 1, 2, 3, 3, 3, 3, 2, 1, 0, 0, 0, 1, 2, 2, 1,
};

static int8 blocky16_table_small2[] = {
	0, 0, 0, 0, 1, 2, 3, 3, 3, 3, 2, 1, 1, 1, 2, 2,
};

static int8 blocky16_table_big1[] = {
	0, 2, 5, 7, 7, 7, 7, 7, 7, 5, 2, 0, 0, 0, 0, 0,
};

static int8 blocky16_table_big2[] = {
	0, 0, 0, 0, 1, 3, 4, 6, 7, 7, 7, 7, 6, 4, 3, 1,
};

static int8 blocky16_table[] = {
	  0,   0,  -1, -43,   6, -43,  -9, -42,  13, -41,
	-16, -40,  19, -39, -23, -36,  26, -34,  -2, -33,
	  4, -33, -29, -32,  -9, -32,  11, -31, -16, -29,
	 32, -29,  18, -28, -34, -26, -22, -25,  -1, -25,
	  3, -25,  -7, -24,   8, -24,  24, -23,  36, -23,
	-12, -22,  13, -21, -38, -20,   0, -20, -27, -19,
	 -4, -19,   4, -19, -17, -18,  -8, -17,   8, -17,
	 18, -17,  28, -17,  39, -17, -12, -15,  12, -15,
	-21, -14,  -1, -14,   1, -14, -41, -13,  -5, -13,
	  5, -13,  21, -13, -31, -12, -15, -11,  -8, -11,
	  8, -11,  15, -11,  -2, -10,   1, -10,  31, -10,
	-23,  -9, -11,  -9,  -5,  -9,   4,  -9,  11,  -9,
	 42,  -9,   6,  -8,  24,  -8, -18,  -7,  -7,  -7,
	 -3,  -7,  -1,  -7,   2,  -7,  18,  -7, -43,  -6,
	-13,  -6,  -4,  -6,   4,  -6,   8,  -6, -33,  -5,
	 -9,  -5,  -2,  -5,   0,  -5,   2,  -5,   5,  -5,
	 13,  -5, -25,  -4,  -6,  -4,  -3,  -4,   3,  -4,
	  9,  -4, -19,  -3,  -7,  -3,  -4,  -3,  -2,  -3,
	 -1,  -3,   0,  -3,   1,  -3,   2,  -3,   4,  -3,
	  6,  -3,  33,  -3, -14,  -2, -10,  -2,  -5,  -2,
	 -3,  -2,  -2,  -2,  -1,  -2,   0,  -2,   1,  -2,
	  2,  -2,   3,  -2,   5,  -2,   7,  -2,  14,  -2,
	 19,  -2,  25,  -2,  43,  -2,  -7,  -1,  -3,  -1,
	 -2,  -1,  -1,  -1,   0,  -1,   1,  -1,   2,  -1,
	  3,  -1,  10,  -1,  -5,   0,  -3,   0,  -2,   0,
	 -1,   0,   1,   0,   2,   0,   3,   0,   5,   0,
	  7,   0, -10,   1,  -7,   1,  -3,   1,  -2,   1,
	 -1,   1,   0,   1,   1,   1,   2,   1,   3,   1,
	-43,   2, -25,   2, -19,   2, -14,   2,  -5,   2,
	 -3,   2,  -2,   2,  -1,   2,   0,   2,   1,   2,
	  2,   2,   3,   2,   5,   2,   7,   2,  10,   2,
	 14,   2, -33,   3,  -6,   3,  -4,   3,  -2,   3,
	 -1,   3,   0,   3,   1,   3,   2,   3,   4,   3,
	 19,   3,  -9,   4,  -3,   4,   3,   4,   7,   4,
	 25,   4, -13,   5,  -5,   5,  -2,   5,   0,   5,
	  2,   5,   5,   5,   9,   5,  33,   5,  -8,   6,
	 -4,   6,   4,   6,  13,   6,  43,   6, -18,   7,
	 -2,   7,   0,   7,   2,   7,   7,   7,  18,   7,
	-24,   8,  -6,   8, -42,   9, -11,   9,  -4,   9,
	  5,   9,  11,   9,  23,   9, -31,  10,  -1,  10,
	  2,  10, -15,  11,  -8,  11,   8,  11,  15,  11,
	 31,  12, -21,  13,  -5,  13,   5,  13,  41,  13,
	 -1,  14,   1,  14,  21,  14, -12,  15,  12,  15,
	-39,  17, -28,  17, -18,  17,  -8,  17,   8,  17,
	 17,  18,  -4,  19,   0,  19,   4,  19,  27,  19,
	 38,  20, -13,  21,  12,  22, -36,  23, -24,  23,
	 -8,  24,   7,  24,  -3,  25,   1,  25,  22,  25,
	 34,  26, -18,  28, -32,  29,  16,  29, -11,  31,
	  9,  32,  29,  32,  -4,  33,   2,  33, -26,  34,
	 23,  36, -19,  39,  16,  40, -13,  41,   9,  42,
	 -6,  43,   1,  43,   0,   0,   0,   0,   0,   0,
	  0,   0,   0
};

void Blocky16::makeTablesInterpolation(int param) {
	int32 variable1, variable2;
	int32 b1, b2;
	int32 value_table47_1_2, value_table47_1_1, value_table47_2_2, value_table47_2_1;
	int32 tableSmallBig[64], tmp, s;
	int8 *table47_1 = nullptr, *table47_2 = nullptr;
	int32 *ptr_small_big;
	byte *ptr;
	int i, x, y;

	if (param == 8) {
		table47_1 = blocky16_table_big1;
		table47_2 = blocky16_table_big2;
		ptr = _tableBig + 384;
		for (i = 0; i < 256; i++) {
			*ptr = 0;
			ptr += 388;
		}
		ptr = _tableBig + 385;
		for (i = 0; i < 256; i++) {
			*ptr = 0;
			ptr += 388;
		}
	} else if (param == 4) {
		table47_1 = blocky16_table_small1;
		table47_2 = blocky16_table_small2;
		ptr = _tableSmall + 96;
		for (i = 0; i < 256; i++) {
			*ptr = 0;
			ptr += 128;
		}
		ptr = _tableSmall + 97;
		for (i = 0; i < 256; i++) {
			*ptr = 0;
			ptr += 128;
		}
	} else {
		error("Blocky16::makeTablesInterpolation: unknown param %d", param);
	}

	s = 0;
	for (x = 0; x < 16; x++) {
		value_table47_1_1 = table47_1[x];
		value_table47_2_1 = table47_2[x];
		for (y = 0; y < 16; y++) {
			value_table47_1_2 = table47_1[y];
			value_table47_2_2 = table47_2[y];

			if (value_table47_2_1 == 0) {
				b1 = 0;
			} else if (value_table47_2_1 == param - 1) {
				b1 = 1;
			} else if (value_table47_1_1 == 0) {
				b1 = 2;
			} else if (value_table47_1_1 == param - 1) {
				b1 = 3;
			} else {
				b1 = 4;
			}

			if (value_table47_2_2 == 0) {
				b2 = 0;
			} else if (value_table47_2_2 == param - 1) {
				b2 = 1;
			} else if (value_table47_1_2 == 0) {
				b2 = 2;
			} else if (value_table47_1_2 == param - 1) {
				b2 = 3;
			} else {
				b2 = 4;
			}

			memset(tableSmallBig, 0, param * param * 4);

			variable2 = abs(value_table47_2_2 - value_table47_2_1);
			tmp = abs(value_table47_1_2 - value_table47_1_1);
			if (variable2 <= tmp) {
				variable2 = tmp;
			}

			for (variable1 = 0; variable1 <= variable2; variable1++) {
				int32 variable3, variable4;

				if (variable2 > 0) {
					// Linearly interpolate between value_table47_1_1 and value_table47_1_2
					// respectively value_table47_2_1 and value_table47_2_2.
					variable4 = (value_table47_1_1 * variable1 + value_table47_1_2 * (variable2 - variable1) + variable2 / 2) / variable2;
					variable3 = (value_table47_2_1 * variable1 + value_table47_2_2 * (variable2 - variable1) + variable2 / 2) / variable2;
				} else {
					variable4 = value_table47_1_1;
					variable3 = value_table47_2_1;
				}
				ptr_small_big = &tableSmallBig[param * variable3 + variable4];
				*ptr_small_big = 1;

				if ((b1 == 2 && b2 == 3) || (b2 == 2 && b1 == 3) ||
				    (b1 == 0 && b2 != 1) || (b2 == 0 && b1 != 1)) {
					if (variable3 >= 0) {
						i = variable3 + 1;
						while (i--) {
							*ptr_small_big = 1;
							ptr_small_big -= param;
						}
					}
				} else if ((b2 != 0 && b1 == 1) || (b1 != 0 && b2 == 1)) {
					if (param > variable3) {
						i = param - variable3;
						while (i--) {
							*ptr_small_big = 1;
							ptr_small_big += param;
						}
					}
				} else if ((b1 == 2 && b2 != 3) || (b2 == 2 && b1 != 3)) {
					if (variable4 >= 0) {
						i = variable4 + 1;
						while (i--) {
							*(ptr_small_big--) = 1;
						}
					}
				} else if ((b1 == 0 && b2 == 1) || (b2 == 0 && b1 == 1) ||
						   (b1 == 3 && b2 != 2) || (b2 == 3 && b1 != 2)) {
					if (param > variable4) {
						i = param - variable4;
						while (i--) {
							*(ptr_small_big++) = 1;
						}
					}
				}
			}

			if (param == 8) {
				for (i = 64 - 1; i >= 0; i--) {
					if (tableSmallBig[i] != 0) {
						_tableBig[256 + s + _tableBig[384 + s]] = (byte)i;
						_tableBig[384 + s]++;
					} else {
						_tableBig[320 + s + _tableBig[385 + s]] = (byte)i;
						_tableBig[385 + s]++;
					}
				}
				s += 388;
			}
			if (param == 4) {
				for (i = 16 - 1; i >= 0; i--) {
					if (tableSmallBig[i] != 0) {
						_tableSmall[64 + s + _tableSmall[96 + s]] = (byte)i;
						_tableSmall[96 + s]++;
					} else {
						_tableSmall[80 + s + _tableSmall[97 + s]] = (byte)i;
						_tableSmall[97 + s]++;
					}
				}
				s += 128;
			}
		}
	}
}

void Blocky16::makeTables47(int width) {
	if (_lastTableWidth == width)
		return;

	_lastTableWidth = width;

	int32 a, c, d;
	int16 tmp;

	for (int l = 0; l < 512; l += 2) {
		_table[l / 2] = (int16)(blocky16_table[l + 1] * width + blocky16_table[l]);
	}

	a = 0;
	c = 0;
	do {
		for (d = 0; d < _tableSmall[96 + c]; d++) {
			tmp = _tableSmall[64 + c + d];
			tmp = (int16)((byte)(tmp >> 2) * width + (tmp & 3));
			_tableSmall[c + d * 2] = (byte)tmp;
			_tableSmall[c + d * 2 + 1] = tmp >> 8;
		}
		for (d = 0; d < _tableSmall[97 + c]; d++) {
			tmp = _tableSmall[80 + c + d];
			tmp = (int16)((byte)(tmp >> 2) * width + (tmp & 3));
			_tableSmall[32 + c + d * 2] = (byte)tmp;
			_tableSmall[32 + c + d * 2 + 1] = tmp >> 8;
		}
		for (d = 0; d < _tableBig[384 + a]; d++) {
			tmp = _tableBig[256 + a + d];
			tmp = (int16)((byte)(tmp >> 3) * width + (tmp & 7));
			_tableBig[a + d * 2] = (byte)tmp;
			_tableBig[a + d * 2 + 1] = tmp >> 8;
		}
		for (d = 0; d < _tableBig[385 + a]; d++) {
			tmp = _tableBig[320 + a + d];
			tmp = (int16)((byte)(tmp >> 3) * width + (tmp & 7));
			_tableBig[128 + a + d * 2] = (byte)tmp;
			_tableBig[128 + a + d * 2 + 1] = tmp >> 8;
		}

		a += 388;
		c += 128;
	} while (c < 32768);
}

void Blocky16::level3(byte *d_dst) {
	int32 tmp2;
	uint32 t;
	byte code = *_d_src++;
	int i;

	if (code <= 0xF5) {
		if (code == 0xF5) {
			int16 tmp = READ_LE_UINT16(_d_src);
			tmp2 = tmp * 2;
			_d_src += 2;
		} else {
			tmp2 = _table[code] * 2;
		}
		tmp2 += _offset1;
		for (i = 0; i < 2; i++) {
			COPY_4X1_LINE(d_dst + 0, d_dst + tmp2 + 0);
			d_dst += _d_pitch;
		}
	} else if ((code == 0xFF) || (code == 0xF8)) {
		WRITE_2X1_LINE(d_dst + 0, READ_LE_UINT16(_d_src + 0));
		WRITE_2X1_LINE(d_dst + 2, READ_LE_UINT16(_d_src + 2));
		d_dst += _d_pitch;
		WRITE_2X1_LINE(d_dst + 0, READ_LE_UINT16(_d_src + 4));
		WRITE_2X1_LINE(d_dst + 2, READ_LE_UINT16(_d_src + 6));
		_d_src += 8;
	} else if (code == 0xFD) {
		t = *_d_src++;
		t = READ_LE_UINT16(_param6_7Ptr + t * 2);
		t = (t << 16) | t;
		for (i = 0; i < 2; i++) {
			WRITE_4X1_LINE(d_dst + 0, t);
			d_dst += _d_pitch;
		}
	} else if (code == 0xFE) {
		t = READ_LE_UINT16(_d_src);
		_d_src += 2;
		t = (t << 16) | t;
		for (i = 0; i < 2; i++) {
			WRITE_4X1_LINE(d_dst + 0, t);
			d_dst += _d_pitch;
		}
	} else if (code == 0xF6) {
		tmp2 = _offset2;
		for (i = 0; i < 2; i++) {
			COPY_4X1_LINE(d_dst + 0, d_dst + tmp2 + 0);
			d_dst += _d_pitch;
		}
	} else if (code == 0xF7) {
		tmp2 = READ_LE_UINT32(_d_src);
		_d_src += 4;
		WRITE_2X1_LINE(d_dst + 0, READ_LE_UINT16(_param6_7Ptr + (byte)tmp2 * 2));
		WRITE_2X1_LINE(d_dst + 2, READ_LE_UINT16(_param6_7Ptr + (byte)(tmp2 >> 8) * 2));
		tmp2 >>= 16;
		d_dst += _d_pitch;
		WRITE_2X1_LINE(d_dst + 0, READ_LE_UINT16(_param6_7Ptr + (byte)tmp2 * 2));
		WRITE_2X1_LINE(d_dst + 2, READ_LE_UINT16(_param6_7Ptr + (byte)(tmp2 >> 8) * 2));
	} else if ((code >= 0xF9) && (code <= 0xFC))  {
		t = READ_LE_UINT16(_paramPtr + code * 2);
		t = (t << 16) | t;
		for (i = 0; i < 2; i++) {
			WRITE_4X1_LINE(d_dst + 0, t);
			d_dst += _d_pitch;
		}
	}
}

void Blocky16::level2(byte *d_dst) {
	int32 tmp2;
	uint32 t = 0, val;
	byte code = *_d_src++;
	int i;

	if (code <= 0xF5) {
		if (code == 0xF5) {
			int16 tmp = READ_LE_UINT16(_d_src);
			tmp2 = tmp * 2;
			_d_src += 2;
		} else {
			tmp2 = _table[code] * 2;
		}
		tmp2 += _offset1;
		for (i = 0; i < 4; i++) {
			COPY_4X1_LINE(d_dst +  0, d_dst + tmp2 +  0);
			COPY_4X1_LINE(d_dst +  4, d_dst + tmp2 +  4);
			d_dst += _d_pitch;
		}
	} else if (code == 0xFF) {
		level3(d_dst);
		d_dst += 4;
		level3(d_dst);
		d_dst += _d_pitch * 2 - 4;
		level3(d_dst);
		d_dst += 4;
		level3(d_dst);
	} else if (code == 0xF6) {
		tmp2 = _offset2;
		for (i = 0; i < 4; i++) {
			COPY_4X1_LINE(d_dst +  0, d_dst + tmp2 +  0);
			COPY_4X1_LINE(d_dst +  4, d_dst + tmp2 +  4);
			d_dst += _d_pitch;
		}
	} else if ((code == 0xF7) || (code == 0xF8)) {
		byte tmp = *_d_src++;
		if (code == 0xF8) {
			val = READ_LE_UINT32(_d_src);
			_d_src += 4;
		} else {
			tmp2 = READ_LE_UINT16(_d_src);
			val = READ_LE_UINT16(_param6_7Ptr + (byte)(tmp2 >> 8) * 2) << 16;
			val |= READ_LE_UINT16(_param6_7Ptr + (byte)tmp2 * 2);
			_d_src += 2;
		}
		byte *tmp_ptr = _tableSmall + (tmp * 128);
		byte l = tmp_ptr[96];
		int16 *tmp_ptr2 = (int16 *)tmp_ptr;
		while (l--) {
			WRITE_2X1_LINE(d_dst + READ_LE_UINT16(tmp_ptr2) * 2, val);
			tmp_ptr2++;
		}
		l = tmp_ptr[97];
		val >>= 16;
		tmp_ptr2 = (int16 *)(tmp_ptr + 32);
		while (l--) {
			WRITE_2X1_LINE(d_dst + READ_LE_UINT16(tmp_ptr2) * 2, val);
			tmp_ptr2++;
		}
	} else if (code >= 0xF9) {
		if (code == 0xFD) {
			t = *_d_src++;
			t = READ_LE_UINT16(_param6_7Ptr + t * 2);
			t = (t << 16) | t;
		} else if (code == 0xFE) {
			t = READ_LE_UINT16(_d_src);
			t = (t << 16) | t;
			_d_src += 2;
		} else if ((code >= 0xF9) && (code <= 0xFC))  {
			t = READ_LE_UINT16(_paramPtr + code * 2);
			t = (t << 16) | t;
		}
		for (i = 0; i < 4; i++) {
			WRITE_4X1_LINE(d_dst + 0, t);
			WRITE_4X1_LINE(d_dst + 4, t);
			d_dst += _d_pitch;
		}
	}
}

void Blocky16::level1(byte *d_dst) {
	int32 tmp2;
	uint32 t = 0, val;
	byte code = *_d_src++;
	int i;

	if (code <= 0xF5) {
		if (code == 0xF5) {
			int16 tmp = READ_LE_UINT16(_d_src);
			tmp2 = tmp * 2;
			_d_src += 2;
		} else {
			tmp2 = _table[code] * 2;
		}
		tmp2 += _offset1;
		for (i = 0; i < 8; i++) {
			COPY_4X1_LINE(d_dst +  0, d_dst + tmp2 +  0);
			COPY_4X1_LINE(d_dst +  4, d_dst + tmp2 +  4);
			COPY_4X1_LINE(d_dst +  8, d_dst + tmp2 +  8);
			COPY_4X1_LINE(d_dst + 12, d_dst + tmp2 + 12);
			d_dst += _d_pitch;
		}
	} else if (code == 0xFF) {
		level2(d_dst);
		d_dst += 8;
		level2(d_dst);
		d_dst += _d_pitch * 4 - 8;
		level2(d_dst);
		d_dst += 8;
		level2(d_dst);
	} else if (code == 0xF6) {
		tmp2 = _offset2;
		for (i = 0; i < 8; i++) {
			COPY_4X1_LINE(d_dst +  0, d_dst + tmp2 +  0);
			COPY_4X1_LINE(d_dst +  4, d_dst + tmp2 +  4);
			COPY_4X1_LINE(d_dst +  8, d_dst + tmp2 +  8);
			COPY_4X1_LINE(d_dst + 12, d_dst + tmp2 + 12);
			d_dst += _d_pitch;
		}
	} else if ((code == 0xF7) || (code == 0xF8)) {
		byte tmp = *_d_src++;
		if (code == 0xF8) {
			val = READ_LE_UINT32(_d_src);
			_d_src += 4;
		} else {
			tmp2 = READ_LE_UINT16(_d_src);
			val = READ_LE_UINT16(_param6_7Ptr + (byte)(tmp2 >> 8) * 2) << 16;
			val |= READ_LE_UINT16(_param6_7Ptr + (byte)tmp2 * 2);
			_d_src += 2;
		}
		byte *tmp_ptr = _tableBig + (tmp * 388);
		byte l = tmp_ptr[384];
		int16 *tmp_ptr2 = (int16 *)tmp_ptr;
		while (l--) {
			WRITE_2X1_LINE(d_dst + READ_LE_UINT16(tmp_ptr2) * 2, val);
			tmp_ptr2++;
		}
		l = tmp_ptr[385];
		val >>= 16;
		tmp_ptr2 = (int16 *)(tmp_ptr + 128);
		while (l--) {
			WRITE_2X1_LINE(d_dst + READ_LE_UINT16(tmp_ptr2) * 2, val);
			tmp_ptr2++;
		}
	} else if (code >= 0xF9) {
		if (code == 0xFD) {
			t = *_d_src++;
			t = READ_LE_UINT16(_param6_7Ptr + t * 2);
			t = (t << 16) | t;
		} else if (code == 0xFE) {
			t = READ_LE_UINT16(_d_src);
			t = (t << 16) | t;
			_d_src += 2;
		} else if ((code >= 0xF9) && (code <= 0xFC))  {
			t = READ_LE_UINT16(_paramPtr + code * 2);
			t = (t << 16) | t;
		}
		for (i = 0; i < 8; i++) {
			WRITE_4X1_LINE(d_dst +  0, t);
			WRITE_4X1_LINE(d_dst +  4, t);
			WRITE_4X1_LINE(d_dst +  8, t);
			WRITE_4X1_LINE(d_dst + 12, t);
			d_dst += _d_pitch;
		}
	}
}

void Blocky16::decode2(byte *dst, const byte *src, int width, int height, const byte *param_ptr, const byte *param6_7_ptr) {
	_d_src = src;
	_paramPtr = param_ptr - 0xf9 - 0xf9;
	_param6_7Ptr = param6_7_ptr;
	int bh = _blocksHeight;
	int next_line = width * 2 * 7;
	_d_pitch = width * 2;

	do {
		int tmp_bw = _blocksWidth;
		do {
			level1(dst);
			dst += 16;
		} while (--tmp_bw);
		dst += next_line;
	} while (--bh);
}

void Blocky16::init(int width, int height) {
	deinit();
	_width = width;
	_height = height;
	makeTablesInterpolation(4);
	makeTablesInterpolation(8);

	_blocksWidth = (width + 7) / 8;
	_blocksHeight = (height + 7) / 8;

	_frameSize = _width * _height * 2;

	// some animations, like tb_kitty.snm don't have a multiple of 8 width or height,
	// so set the size of the buffer in _blocksWidth * 8 * _blocksHeight * 8, instead
	// of using _frameSize.
	int size = _blocksWidth * 8 * _blocksHeight * 8 * 2;
	_offset = size - _frameSize;
	// workaround for read over buffer by increasing buffer
	// 200 bytes is enough for smush anims:
	// lol, byeruba, crushed, eldepot, heltrain, hostage
	uint32 deltaSize = size * 3 + 200;
	_deltaBuf = new byte[deltaSize];
	memset(_deltaBuf, 0, deltaSize);
	_deltaBufs[0] = _deltaBuf;
	_deltaBufs[1] = _deltaBuf + _frameSize;
	_curBuf = _deltaBuf + _frameSize * 2;
}

Blocky16::Blocky16() {
	_tableBig = new byte[99328];
	_tableSmall = new byte[32768];
	memset(_tableBig, 0, 99328);
	memset(_tableSmall, 0, 32768);
	_deltaBuf = nullptr;
	_deltaBufs[0] = nullptr;
	_deltaBufs[1] = nullptr;
	_curBuf = nullptr;
	_prevSeqNb = 0;
	_lastTableWidth = 0;
	_d_src = nullptr;
	_paramPtr = nullptr;
	_param6_7Ptr = nullptr;
	_blocksHeight = _blocksWidth = 0;
	_height = _width = 0;
	_offset = _offset1 = _offset2 = 0;
	_frameSize = 0;
	_d_pitch = 0;
}

void Blocky16::deinit() {
	_lastTableWidth = -1;
	if (_deltaBuf) {
		delete[] _deltaBuf;
		_deltaBuf = nullptr;
		_deltaBufs[0] = nullptr;
		_deltaBufs[1] = nullptr;
	}
}

Blocky16::~Blocky16() {
	deinit();
	if (_tableBig) {
		delete[] _tableBig;
		_tableBig = nullptr;
	}
	if (_tableSmall) {
		delete[] _tableSmall;
		_tableSmall = nullptr;
	}
}

static int bomp_left;
static int bomp_num;
static int bomp_color;
static const byte *bomp_src;

static byte bompDecode() {
	byte code, result;
	const byte *src;

	if (bomp_left == 2) {
		src = bomp_src;
		bomp_num = (*src >> 1) + 1;
		code = *(src++) & 1;
		bomp_src = src;
		if (code != 0) {
			bomp_left = 1;
			bomp_color = *src++;
			bomp_src = src;
		} else {
			bomp_left = 0;
		}
	} else {
		src = bomp_src;
	}
	if (bomp_left != 0) {
		if (bomp_left - 1 == 0) {
			result = bomp_color;
		} else {
			result = 255;
		}
	} else {
		result = *(src++);
		bomp_src = src;
	}

	bomp_num--;
	if (bomp_num == 0) {
		bomp_left = 2;
	}

	return result;
}

static void bompInit(const byte *src) {
	bomp_left = 2;
	bomp_src = src;
}

static void bompDecodeMain(byte *dst, const byte *src, int size) {
	size /= 2;
	bompInit(src);
	while (size--) {
#ifdef SCUMM_BIG_ENDIAN
		*(dst + 1) = bompDecode();
		*(dst + 0) = bompDecode();
#else
		*(dst + 0) = bompDecode();
		*(dst + 1) = bompDecode();
#endif
		dst += 2;
	}
}

void Blocky16::decode(byte *dst, const byte *src) {
	_offset1 = ((_deltaBufs[1] - _curBuf) / 2) * 2;
	_offset2 = ((_deltaBufs[0] - _curBuf) / 2) * 2;

	int32 seq_nb = READ_LE_UINT16(src + 16);

	const byte *gfx_data = src + 560;

	if (seq_nb == 0) {
		makeTables47(_width);
		if (src[32] == src[33]) {
			memset(_deltaBufs[0], src[32], _frameSize);
			memset(_deltaBufs[1], src[32], _frameSize);
		} else {
			int count = _frameSize / 2;
			uint16 *ptr1 = (uint16 *)_deltaBufs[0];
			uint16 *ptr2 = (uint16 *)_deltaBufs[1];
			uint16 val = READ_LE_UINT16(src + 32);
			while (count--) {
				*(uint16 *)(ptr1++) = val;
				*(uint16 *)(ptr2++) = val;
			};

		}
		_prevSeqNb = -1;
	}

	switch (src[18]) {
	case 0:
#if defined(SCUMM_BIG_ENDIAN)
		for (int i = 0; i < _width * _height; i++) {
			((uint16 *)_curBuf)[i] = READ_LE_UINT16(gfx_data + i * 2);
		}
#else
		memcpy(_curBuf, gfx_data, _frameSize);
#endif
		break;
	case 1:
		error("blocky16: not implemented decode1 proc");
		break;
	case 2:
		if (seq_nb == _prevSeqNb + 1) {
			decode2(_curBuf, gfx_data, _width, _height, src + 24, src + 40);
		}

		break;
	case 3:
		memcpy(_curBuf, _deltaBufs[1], _frameSize);
		break;
	case 4:
		memcpy(_curBuf, _deltaBufs[0], _frameSize);
		break;
	case 5:
		bompDecodeMain(_curBuf, gfx_data, READ_LE_UINT32(src + 36));
		break;
	case 6:
		{
			int count = _frameSize / 2;
			uint16 *ptr = (uint16 *)_curBuf;
			while (count--) {
				int offset = *gfx_data++ * 2;
				*(uint16 *)ptr++ = READ_LE_UINT16(src + 40 + offset);
			};
			break;
		}
	case 7:
		error("blocky16: not implemented decode7 proc");
		break;
	case 8:
		{
			bompInit(gfx_data);
			int count = _frameSize / 2;
			uint16 *ptr = (uint16 *)_curBuf;
			while (count--) {
				int offset = bompDecode() * 2;
				*(uint16 *)ptr++ = READ_LE_UINT16(src + 40 + offset);
			};
			break;
		}
	}

	memcpy(dst, _curBuf, _frameSize);

	if (seq_nb == _prevSeqNb + 1) {
		byte *tmp_ptr = nullptr;
		if (src[19] == 1) {
			tmp_ptr = _curBuf;
			_curBuf = _deltaBufs[1];
			_deltaBufs[1] = tmp_ptr;
		} else if (src[19] == 2) {
			tmp_ptr = _deltaBufs[0];
			_deltaBufs[0] = _deltaBufs[1];
			_deltaBufs[1] = _curBuf - _offset; // subtract the offset here or else a black bar will appear
			_curBuf = tmp_ptr;                 // on top of tb_kitty.snm.
		}
	}
	_prevSeqNb = seq_nb;
}

} // end of namespace Grim
