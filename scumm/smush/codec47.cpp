/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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
#include "codec47.h"
#include "scumm/bomp.h"

#include "common/engine.h"

#if defined(SCUMM_NEED_ALIGNMENT)

#define COPY_4X1_LINE(dst, src)			\
	do {					\
		(dst)[0] = (src)[0];	\
		(dst)[1] = (src)[1];	\
		(dst)[2] = (src)[2];	\
		(dst)[3] = (src)[3];	\
	} while (0)

#define COPY_2X1_LINE(dst, src)			\
	do {					\
		(dst)[0] = (src)[0];	\
		(dst)[1] = (src)[1];	\
	} while (0)


#else /* SCUMM_NEED_ALIGNMENT */

#define COPY_4X1_LINE(dst, src)			\
	*(uint32 *)(dst) = *(const uint32 *)(src)

#define COPY_2X1_LINE(dst, src)			\
	*(uint16 *)(dst) = *(const uint16 *)(src)

#endif

#define FILL_4X1_LINE(dst, val)			\
	do {					\
		(dst)[0] = val;	\
		(dst)[1] = val;	\
		(dst)[2] = val;	\
		(dst)[3] = val;	\
	} while (0)

#define FILL_2X1_LINE(dst, val)			\
	do {					\
		(dst)[0] = val;	\
		(dst)[1] = val;	\
	} while (0)

#ifdef __PALM_OS__
static int32 *codec37_table;
static int16 *codec47_table;
#else
static int32 codec37_table[] = {
       0,       1,       2,       3,       3,       3,
       3,       2,       1,       0,       0,       0,
       1,       2,       2,       1,       0,       0,
       0,       0,       1,       2,       3,       3,
       3,       3,       2,       1,       1,       1,
       2,       2,       0,       2,       5,       7,
       7,       7,       7,       7,       7,       5,
       2,       0,       0,       0,       0,       0,
       0,       0,       0,       0,       1,       3,
       4,       6,       7,       7,       7,       7,
       6,       4,       3,       1,      -1,       0,
       0,       0,       0,       0,       0,       1,
       2,       3,       5,       8,      13,      21,
   65535,   65534,   65533,   65531,   65528,   65523,
   65519,   65515,   65536,   65537,   65538,   65539,
   65541,   65544,   65549,   65557,  131071,  131070,
  131069,  131067,  131064,  131059,  131055,  131051,
  131072,  131073,  131074,  131075,  131077,  131080,
  131085,  131093,  196607,  196606,  196605,  196603,
  196600,  196595,  196591,  196587,  196608,  196609,
  196610,  196611,  196613,  196616,  196621,  196629,
  262143,  262142,  262141,  262139,  262136,  262131,
  262127,  262123,  327680,  327681,  327682,  327683,
  327685,  327688,  327693,  327701,  393215,  393214,
  393213,  393211,  393208,  393203,  393199,  393195,
  524288,  524289,  524290,  524291,  524293,  524296,
  524301,  524309,  589823,  589822,  589821,  589819,
  589816,  589811,  589807,  589803,  851968,  851969,
  851970,  851971,  851973,  851976,  851981,  851989,
  917503,  917502,  917501,  917499,  917496,  917491,
  917487,  917483, 1376256, 1376257, 1376258, 1376259,
 1376261, 1376264, 1376269, 1376277, 1441791, 1441790,
 1441789, 1441787, 1441784, 1441779, 1441775, 1441771,
  -65536,  -65535,  -65534,  -65533,  -65531,  -65528,
  -65523,  -65515,      -1,      -2,      -3,      -5,
      -8,     -13,     -17,     -21, -131072, -131071,
 -131070, -131069, -131067, -131064, -131059, -131051,
  -65537,  -65538,  -65539,  -65541,  -65544,  -65549,
  -65553,  -65557, -196608, -196607, -196606, -196605,
 -196603, -196600, -196595, -196587, -131073, -131074,
 -131075, -131077, -131080, -131085, -131089, -131093,
 -327680, -327679, -327678, -327677, -327675, -327672,
 -327667, -327659, -262145, -262146, -262147, -262149,
 -262152, -262157, -262161, -262165, -524288, -524287,
 -524286, -524285, -524283, -524280, -524275, -524267,
 -458753, -458754, -458755, -458757, -458760, -458765,
 -458769, -458773, -851968, -851967, -851966, -851965,
 -851963, -851960, -851955, -851947, -786433, -786434,
 -786435, -786437, -786440, -786445, -786449, -786453,
-1114112,-1114111,-1114110,-1114109,-1114107,-1114104,
-1114099,-1114091,-1048577,-1048578,-1048579,-1048581,
-1048584,-1048589,-1048593,-1048597,-1376256,-1376255,
-1376254,-1376253,-1376251,-1376248,-1376243,-1376235,
-1310721,-1310722,-1310723,-1310725,-1310728,-1310733,
-1310737,       0,-1835016,-1900536,-1572882,-1638383,
-1507328,-1376262,-1441786,-1179661,-1245172,-1179648,
-1179623,-1048601,-1048581,-1114107, -917514, -983030,
 -917504, -786436, -851964, -851949, -720915, -655368,
 -655362, -720896, -720894, -720888, -589839, -589828,
 -655356, -655345, -524294, -524289, -589823, -589818,
 -458781, -458763, -458760, -458755, -524285, -524280,
 -524277, -524259, -393221, -393218, -458752, -458750,
 -458747, -327702, -327689, -327686, -327683, -327681,
 -393215, -393213, -393210, -393207, -393194, -262161,
 -262151, -262148, -262146, -327680, -327678, -327676,
 -327673, -327663, -196621, -196618, -196613, -196611,
 -196609, -262144, -262143, -262141, -262139, -262134,
 -262131, -131080, -131078, -131076, -131075, -131074,
 -131073, -196608, -196607, -196606, -196604, -196602,
 -196600,  -65547,  -65543,  -65541,  -65539,  -65538,
  -65537, -131072, -131071, -131070, -131069, -131067,
 -131065, -131061,      -9,      -6,      -4,      -3,
      -2,      -1,  -65536,  -65535,  -65534,  -65533,
  -65532,  -65530,  -65527,   65505,   65513,   65518,
   65522,   65525,   65529,   65531,   65532,   65533,
   65534,   65535,-2031616,       1,       2,       3,
       4,       5,       7,      11,      14,      18,
      23,      31,  131063,  131066,  131068,  131069,
  131070,  131071,   65536,   65537,   65538,   65539,
   65540,   65542,   65545,  196597,  196601,  196603,
  196605,  196606,  196607,  131072,  131073,  131074,
  131075,  131077,  131079,  131083,  262136,  262138,
  262140,  262142,  262143,  196608,  196609,  196610,
  196611,  196612,  196614,  196616,  327667,  327670,
  327675,  327677,  327679,  262144,  262145,  262147,
  262149,  262154,  262157,  393199,  393209,  393212,
  393214,  327680,  327682,  327684,  327687,  327697,
  458730,  458743,  458746,  458749,  458751,  393217,
  393219,  393222,  393225,  393238,  524283,  524286,
  458752,  458754,  458757,  589795,  589813,  589816,
  589821,  524291,  524296,  524299,  524317,  655354,
  655359,  589825,  589830,  720881,  720892,  655364,
  655375,  786424,  786430,  720896,  720898,  720904,
  786451,  917485,  917500,  851972,  917504, 1048566,
  983050, 1179643, 1114117, 1114137, 1245159, 1179648,
 1310708, 1245197, 1507322, 1441798, 1507328, 1703919,
 1638418, 1966072, 1900552, 2031616,       0,-1376262,
-1441786,-1179661,-1245172,-1179648,-1048581,-1114107,
 -917514, -983030, -917504, -786436, -851964, -851949,
 -720915, -655368, -655362, -720896, -720894, -720888,
 -589839, -589828, -655356, -655345, -524294, -524289,
 -589823, -589818, -458763, -458760, -458755, -524288,
 -524285, -524280, -524277, -393221, -393218, -458752,
 -458750, -458747, -327702, -327689, -327686, -327683,
 -327681, -393215, -393213, -393210, -393207, -393194,
 -262161, -262151, -262148, -262146, -262145, -327680,
 -327679, -327678, -327676, -327673, -327663, -196621,
 -196618, -196613, -196611, -196610, -196609, -262144,
 -262143, -262142, -262141, -262139, -262134, -262131,
 -131080, -131078, -131076, -131075, -131074, -131073,
 -196608, -196607, -196606, -196605, -196604, -196602,
 -196600,  -65547,  -65543,  -65541,  -65540,  -65539,
  -65538,  -65537, -131072, -131071, -131070, -131069,
 -131068, -131067, -131065, -131061,      -9,      -6,
      -5,      -4,      -3,      -2,      -1,  -65536,
  -65535,  -65534,  -65533,  -65532,  -65531,  -65530,
  -65527,   65513,   65518,   65522,   65525,   65529,
   65531,   65532,   65533,   65534,   65535,-1507328,
       1,       2,       3,       4,       5,       7,
      11,      14,      18,      23,  131063,  131066,
  131067,  131068,  131069,  131070,  131071,   65536,
   65537,   65538,   65539,   65540,   65541,   65542,
   65545,  196597,  196601,  196603,  196604,  196605,
  196606,  196607,  131072,  131073,  131074,  131075,
  131076,  131077,  131079,  131083,  262136,  262138,
  262140,  262141,  262142,  262143,  196608,  196609,
  196610,  196611,  196612,  196614,  196616,  327667,
  327670,  327675,  327677,  327678,  327679,  262144,
  262145,  262146,  262147,  262149,  262154,  262157,
  393199,  393209,  393212,  393214,  393215,  327680,
  327681,  327682,  327684,  327687,  327697,  458730,
  458743,  458746,  458749,  458751,  393217,  393219,
  393222,  393225,  393238,  524283,  524286,  458752,
  458754,  458757,  589813,  589816,  589821,  524288,
  524291,  524296,  524299,  655354,  655359,  589825,
  589830,  720881,  720892,  655364,  655375,  786424,
  786430,  720896,  720898,  720904,  786451,  917485,
  917500,  851972,  917504, 1048566,  983050, 1179643,
 1114117, 1179648, 1310708, 1245197, 1507322, 1441798,
 1507328
};

static int16 codec47_table[] = {
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
	 -6,  43,   1,  43,   0,   0,   0,   0,   0,   0
};
#endif

void Codec47Decoder::makeTables37(int32 param) {
	int32 variable1, variable2;
	int32 b1, b2;
	int32 value_table37_1_2, value_table37_1_1, value_table37_2_2, value_table37_2_1;
	int32 tableSmallBig[64], tmp, s;
	int32 *table37_1 = 0, *table37_2 = 0, *ptr_small_big;
	byte *ptr;
	int i, x, y;

	if (param == 8) {
		table37_1 = &codec37_table[32];
		table37_2 = &codec37_table[48];
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
		table37_1 = &codec37_table[0];
		table37_2 = &codec37_table[16];
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
		error("makeTables37: unknown param %d", param);
	}

	s = 0;
	for (x = 0; x < 16; x++) {
		value_table37_1_1 = table37_1[x];
		value_table37_2_1 = table37_2[x];
		for (y = 0; y < 16; y++) {
			value_table37_1_2 = table37_1[y];
			value_table37_2_2 = table37_2[y];

			if (value_table37_2_1 == 0) {
				b1 = 0;
			} else if (value_table37_2_1 == param - 1) {
				b1 = 1;
			} else if (value_table37_1_1 == 0) {
				b1 = 2;
			} else if (value_table37_1_1 == param - 1) {
				b1 = 3;
			} else {
				b1 = 4;
			}

			if (value_table37_2_2 == 0) {
				b2 = 0;
			} else if (value_table37_2_2 == param - 1) {
				b2 = 1;
			} else if (value_table37_1_2 == 0) {
				b2 = 2;
			} else if (value_table37_1_2 == param - 1) {
				b2 = 3;
			} else {
				b2 = 4;
			}
			
			memset(tableSmallBig, 0, param * param * 4);

			variable2 = abs(value_table37_2_2 - value_table37_2_1);
			tmp = abs(value_table37_1_2 - value_table37_1_1);
			if (variable2 <= tmp) {
				variable2 = tmp;
			}

			for (variable1 = 0; variable1 <= variable2; variable1++) {
				int32 variable3, variable4;

				if (variable2 > 0) {
					// Linearly interpolate between value_table37_1_1 and value_table37_1_2
					// respectively value_table37_2_1 and value_table37_2_2.
					variable4 = (value_table37_1_1 * variable1 + value_table37_1_2 * (variable2 - variable1) + variable2 / 2) / variable2;
					variable3 = (value_table37_2_1 * variable1 + value_table37_2_2 * (variable2 - variable1) + variable2 / 2) / variable2;
				} else {
					variable4 = value_table37_1_1;
					variable3 = value_table37_2_1;
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

void Codec47Decoder::makeTables47(int32 width) {
	if (_lastTableWidth == width)
		return;

	_lastTableWidth = width;

	int32 a, c, d;
	int16 tmp;

	int16 *tmp_ptr = _table;
	int16 *ptr_table = codec47_table;
	do {
		*tmp_ptr++ = (int16)(ptr_table[1] * width + ptr_table[0]);
		ptr_table += 2;
	} while (tmp_ptr < &_table[255]);
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

void Codec47Decoder::level3(byte *d_dst) {
	int32 tmp;
	byte code = *_d_src++;

	if (code < 0xF8) {
		tmp = _table[code] + _offset1;
		COPY_2X1_LINE(d_dst, d_dst + tmp);
		COPY_2X1_LINE(d_dst + _d_pitch, d_dst + _d_pitch + tmp);
	} else if (code == 0xFF) {
		COPY_2X1_LINE(d_dst, _d_src + 0);
		COPY_2X1_LINE(d_dst + _d_pitch, _d_src + 2);
		_d_src += 4;
	} else if (code == 0xFE) {
		byte t = *_d_src++;
		FILL_2X1_LINE(d_dst, t);
		FILL_2X1_LINE(d_dst + _d_pitch, t);
	} else if (code == 0xFC) {
		tmp = _offset2;
		COPY_2X1_LINE(d_dst, d_dst + tmp);
		COPY_2X1_LINE(d_dst + _d_pitch, d_dst + _d_pitch + tmp);
	} else {
		byte t = _paramPtr[code];
		FILL_2X1_LINE(d_dst, t);
		FILL_2X1_LINE(d_dst + _d_pitch, t);
	}
}

void Codec47Decoder::level2(byte *d_dst) {
	int32 tmp;
	byte code = *_d_src++;
	int i;

	if (code < 0xF8) {
		tmp = _table[code] + _offset1;
		for (i = 0; i < 4; i++) {
			COPY_4X1_LINE(d_dst, d_dst + tmp);
			d_dst += _d_pitch;
		}
	} else if (code == 0xFF) {
		byte *tmp_dst = d_dst;
		level3(d_dst);
		d_dst += 2;
		level3(d_dst);
		d_dst += _d_pitch * 2 - 2;
		level3(d_dst);
		d_dst += 2;
		level3(d_dst);
		d_dst = tmp_dst;
	} else if (code == 0xFE) {
		byte t = *_d_src++;
		for (i = 0; i < 4; i++) {
			FILL_4X1_LINE(d_dst, t);
			d_dst += _d_pitch;
		}
	} else if (code == 0xFD) {
		byte *tmp_ptr = _tableSmall + (*_d_src++ << 7);
		int32 l = tmp_ptr[96];
		byte val = *_d_src++;
		int16 *tmp_ptr2 = (int16 *)tmp_ptr;
		while (l--) {
			*(d_dst + READ_LE_UINT16(tmp_ptr2)) = val;
			tmp_ptr2++;
		}
		l = tmp_ptr[97];
		val = *_d_src++;
		tmp_ptr2 = (int16 *)(tmp_ptr + 32);
		while (l--) {
			*(d_dst + READ_LE_UINT16(tmp_ptr2)) = val;
			tmp_ptr2++;
		}
	} else if (code == 0xFC) {
		tmp = _offset2;
		for (i = 0; i < 4; i++) {
			COPY_4X1_LINE(d_dst, d_dst + tmp);
			d_dst += _d_pitch;
		}
	} else {
		byte t = _paramPtr[code];
		for (i = 0; i < 4; i++) {
			FILL_4X1_LINE(d_dst, t);
			d_dst += _d_pitch;
		}
	}
}

void Codec47Decoder::level1(byte *d_dst) {
	int32 tmp, tmp2;
	byte code = *_d_src++;
	int i;

	if (code < 0xF8) {
		tmp2 = _table[code] + _offset1;
		for (i = 0; i < 8; i++) {
			COPY_4X1_LINE(d_dst + 0, d_dst + tmp2);
			COPY_4X1_LINE(d_dst + 4, d_dst + tmp2 + 4);
			d_dst += _d_pitch;
		}
	} else if (code == 0xFF) {
		byte *tmp_dst = d_dst;
		level2(d_dst);
		d_dst += 4;
		level2(d_dst);
		d_dst += _d_pitch * 4 - 4;
		level2(d_dst);
		d_dst += 4;
		level2(d_dst);
		d_dst = tmp_dst;
	} else if (code == 0xFE) {
		byte t = *_d_src++;
		for (i = 0; i < 8; i++) {
			FILL_4X1_LINE(d_dst, t);
			FILL_4X1_LINE(d_dst + 4, t);
			d_dst += _d_pitch;
		}
	} else if (code == 0xFD) {
		tmp = *_d_src++;
		byte *tmp_ptr = _tableBig + (tmp << 2) + (tmp << 7) + (tmp << 8);
		byte l = tmp_ptr[384];
		byte val = *_d_src++;
		int16 *tmp_ptr2 = (int16 *)tmp_ptr;
		while (l--) {
			*(d_dst + READ_LE_UINT16(tmp_ptr2)) = val;
			tmp_ptr2++;
		}
		l = tmp_ptr[385];
		val = *_d_src++;
		tmp_ptr2 = (int16 *)(tmp_ptr + 128);
		while (l--) {
			*(d_dst + READ_LE_UINT16(tmp_ptr2)) = val;
			tmp_ptr2++;
		}
	} else if (code == 0xFC) {
		tmp2 = _offset2;
		for (i = 0; i < 8; i++) {
			COPY_4X1_LINE(d_dst + 0, d_dst + tmp2);
			COPY_4X1_LINE(d_dst + 4, d_dst + tmp2 + 4);
			d_dst += _d_pitch;
		}
	} else {
		byte t = _paramPtr[code];
		for (i = 0; i < 8; i++) {
			FILL_4X1_LINE(d_dst, t);
			FILL_4X1_LINE(d_dst + 4, t);
			d_dst += _d_pitch;
		}
	}
}

void Codec47Decoder::decode2(byte *dst, const byte *src, int width, int height, const byte *param_ptr) {
	_d_src = src;
	_paramPtr = param_ptr - 0xf8;
	int bw = (width + 7) >> 3;
	int bh = (height + 7) >> 3;
	int next_line = width * 7;
	_d_pitch = width;

	do {
		int tmp_bw = bw;
		do {
			level1(dst);
			dst += 8;
		} while (--tmp_bw);
		dst += next_line;
	} while (--bh);
}

void Codec47Decoder::init(int width, int height) {
	deinit();
	_width = width;
	_height = height;
	makeTables37(4);
	makeTables37(8);

	_frameSize = _width * _height;
	_deltaSize = _frameSize * 3;
	_deltaBuf = new byte[_deltaSize];
	_deltaBufs[0] = _deltaBuf;
	_deltaBufs[1] = _deltaBuf + _frameSize;
	_curBuf = _deltaBuf + _frameSize * 2;
}

Codec47Decoder::Codec47Decoder() {
#ifdef __PALM_OS__
	_tableBig = (byte *)calloc(99328, sizeof(byte));
	_tableSmall = (byte *)calloc(32768, sizeof(byte));
#endif
	_deltaBuf = 0;
}

void Codec47Decoder::deinit() {
	_lastTableWidth = -1;
	if (_deltaBuf) {
		delete []_deltaBuf;
		_deltaSize = 0;
		_deltaBuf = 0;
		_deltaBufs[0] = 0;
		_deltaBufs[1] = 0;
	}
}

Codec47Decoder::~Codec47Decoder() {
	deinit();
#ifdef __PALM_OS__
	free(_tableBig);
	free(_tableSmall);
#endif
}

bool Codec47Decoder::decode(byte *dst, const byte *src) {
	_offset1 = _deltaBufs[1] - _curBuf;
	_offset2 = _deltaBufs[0] - _curBuf;

	int32 seq_nb = READ_LE_UINT16(src + 0);

	const byte *gfx_data = src + 26;
	byte *tmp_ptr;

	if (seq_nb == 0) {
		makeTables47(_width);
		memset(_deltaBufs[0], src[12], _frameSize);
		memset(_deltaBufs[1], src[13], _frameSize);
		_prevSeqNb = -1;
	}

	if ((src[4] & 1) != 0) {
		gfx_data += 32896;
	}

	switch(src[2]) {
	case 0:
		memcpy(_curBuf, gfx_data, _frameSize);
		break;
	case 1:
		warning("codec47: not implemented decode1 proc");
		break;
	case 2:
		if (seq_nb == _prevSeqNb + 1) {
			decode2(_curBuf, gfx_data, _width, _height, src + 8);
		}
		break;
	case 3:
		memcpy(_curBuf, _deltaBufs[1], _frameSize);
		break;
	case 4:
		memcpy(_curBuf, _deltaBufs[0], _frameSize);
		break;
	case 5:
		bompDecodeLine(_curBuf, gfx_data, READ_LE_UINT32(src + 14));
		break;
	}

	memcpy(dst, _curBuf, _frameSize);

	if (seq_nb == _prevSeqNb + 1) {
		if (src[3] == 1) {
			tmp_ptr = _curBuf;
			_curBuf = _deltaBufs[1];
			_deltaBufs[1] = tmp_ptr;
		} else if (src[3] == 2) {
			tmp_ptr = _deltaBufs[0];
			_deltaBufs[0] = _deltaBufs[1];
			_deltaBufs[1] = _curBuf;
			_curBuf = tmp_ptr;
		}
	}
	_prevSeqNb = seq_nb;

	return true;
}

#ifdef __PALM_OS__
#include "scumm_globals.h" // init globals
void Codec47_initGlobals() {
	GSETPTR(codec37_table,	GBVARS_CODEC37TABLE_INDEX,	int32	, GBVARS_SCUMM)
	GSETPTR(codec47_table,	GBVARS_CODEC47TABLE_INDEX,	int16	, GBVARS_SCUMM)
}
void Codec47_releaseGlobals() {
	GRELEASEPTR(GBVARS_CODEC37TABLE_INDEX		, GBVARS_SCUMM)
	GRELEASEPTR(GBVARS_CODEC47TABLE_INDEX		, GBVARS_SCUMM)
}
#endif

