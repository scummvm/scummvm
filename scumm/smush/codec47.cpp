/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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

#include <stdafx.h>
#include "scumm/scumm.h"
#include "engine.h"
#include "codec47.h"
#include "chunk.h"
#include "blitter.h"

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

static byte smush_buf_big[99328];
static byte smush_buf_small[32768];
static int16 smush_table[256];

void Codec47Decoder::makeTables37(int32 param) {
	int32 variable1, variable2, variable4, count_1, count_2;
	int32 tmp_param, tmp_param_tmp, tmp_value1, tmp_value2, b1, b2;
	int32 * tmp_table37_1_2, * tmp_table37_1_1, * tmp_table37_2_2, * tmp_table37_2_1;
	int32 value_table37_1_1, value_table37_1_2;
	int32 table[64], tmp_a, tmp_c, tmp_d, s, p, d, tmp_ib, tmp;
	int32 * table37_1 = 0, * table37_2 = 0, * table_ptr, l;
	byte * ptr;

	if (param == 8) {
		table37_1 = &codec37_table[32];
		table37_2 = &codec37_table[48];
		ptr = smush_buf_big + 384;
		for (l = 0; l < 256; l++) {
			*ptr = 0;
			ptr += 388;
		}
		ptr = smush_buf_big + 385;
		for (l = 0; l < 256; l++) {
			*ptr = 0;
			ptr += 388;
		}
	}

	if (param == 4) {
		table37_1 = &codec37_table[0];
		table37_2 = &codec37_table[16];
		ptr = smush_buf_small + 96;
		for (l = 0; l < 256; l++) {
			*ptr = 0;
			ptr += 128;
		}
		ptr = smush_buf_small + 97;
		for (l = 0; l < 256; l++) {
			*ptr = 0;
			ptr += 128;
		}
	}

	s = 0;
	p = 0;
	tmp_param = param * param;
	tmp_table37_1_1 = table37_1;
	tmp_table37_2_1 = table37_2;
	count_1 = 16;
	do {
		tmp_table37_1_2 = table37_1;
		tmp_table37_2_2 = table37_2;
		count_2 = 16;
		do {
			tmp = *(tmp_table37_2_1);
			tmp_value1 = tmp;
			if (tmp == 0) {
				b1 = 0;
			} else if ((tmp - param) == -1) {
				b1 = 1;
			} else {
				b1 = 2;
				tmp = *(tmp_table37_1_1);
				if (tmp != 0) {
					b1 = 0;
					tmp -= param;
					if (tmp == -1)
						b1 = 3;
					else
						b1 = 4;
				}
			}
			tmp = *(tmp_table37_2_2);
			tmp_value2 = tmp;
			if (tmp == 0) {
				b2 = 0;
			} else if ((tmp - param) == -1) {
				b2 = 1;
			} else {
				tmp = *(tmp_table37_1_2);
				if (tmp == 0) {
					b2 = 2;
				} else {
					tmp -= param;
					if (tmp == -1)
						b2 = 3;
					else
						b2 = 4;
				}
			}
			tmp_param_tmp = tmp_param - 1;
			d = tmp_param_tmp;
			if(tmp_param != 0) {
				do {
					table[d] = 0;
				} while (d-- != 0);
			}
			value_table37_1_1 = *(tmp_table37_1_1);
			value_table37_1_2 = *(tmp_table37_1_2);
			tmp_c = abs(tmp_value2 - tmp_value1);
			tmp_a = abs(value_table37_1_2 - value_table37_1_1);
			if (tmp_c <= tmp_a) {
				tmp_c = tmp_a;
			}

			variable2 = tmp_c + 1;
			for (variable1 = 0; variable1 < variable2; variable1++) {
				if (variable2 > 1) {
					int32 variable3;

					tmp_c = variable2 - variable1 - 1;
					tmp_ib = variable2 - 1;
					tmp_d = value_table37_1_1;
					tmp_ib >>= 1;
					variable3 = variable2 - 1;
					tmp_d *= variable1;
					tmp_a = value_table37_1_2 * tmp_c + tmp_d + tmp_ib;
					variable4 = tmp_a / variable3;
					tmp_d = tmp_value2 * tmp_c + tmp_value1 * variable1 + tmp_ib;
					tmp_a = tmp_d / variable3;
				} else {
					variable4 = value_table37_1_1;
					tmp_a = tmp_value1;
				}
				tmp_c = param;
				tmp_d = variable4;
				tmp_c *= tmp_a;
				tmp_c += tmp_d;
				table_ptr = &table[tmp_c];
				*(table_ptr) = 1;

				if ((b1 == 2 && b2 == 3) ||
				    (b2 == 2 && b1 == 3) ||
				    (b1 == 0 && b2 != 1) ||
				    (b2 == 0 && b1 != 1)) {
					if (tmp_a < 0)
						continue;
					tmp_c = param;
					tmp_c <<= 2;
					do {
						*(table_ptr) = 1;
						table_ptr -= tmp_c / 4;
					} while (--tmp_a >= 0);
					continue;
				}
				
				if ((b2 != 0 && b1 == 1) ||
				    (b1 != 0 && b2 == 1)) {
					tmp_c = param;
					if (tmp_c <= tmp_a)
						continue;
					tmp_c <<= 2;
					tmp_d = param;
					tmp_d -= tmp_a;
					do {
						*(table_ptr) = 1;
						table_ptr += tmp_c / 4;
					} while (--tmp_d != 0);
					continue;
				}

				if ((b1 == 0 && b2 == 1) ||
				    (b2 == 0 && b1 == 1) ||
				    (b1 == 3 && b2 != 2) ||
				    (b2 == 3 && b1 != 2)) {
					tmp_c = param;
					if (tmp_c <= variable4)
						continue;
					tmp_c -= variable4;
					d = tmp_c;
					do {
						*(table_ptr++) = 1;
					} while (--d != 0);
					continue;
				}
				
				if ((b1 == 2 && b2 != 3) ||
				    (b2 == 2 && b1 != 3)) {
					if (variable4 >= 0) {
						tmp_c = variable4 + 1;
						d = tmp_c;
						do {
							*(table_ptr--) = 1;
						} while (--d != 0);
					}
				}
			}

			if (tmp_param != 0) {
				if (param == 8) {
					tmp_c = tmp_param_tmp;
					do {
						if (table[tmp_c] != 0) {
							smush_buf_big[256 + s + smush_buf_big[384 + s]] = (byte)tmp_c;
							smush_buf_big[384 + s]++;
						} else {
							smush_buf_big[320 + s + smush_buf_big[385 + s]] = (byte)tmp_c;
							smush_buf_big[385 + s]++;
						}
					} while (tmp_c-- != 0);
				}
				if (param == 4) {
					tmp_c = tmp_param_tmp;
					do {
						if (table[tmp_c] != 0) {
							smush_buf_small[64 + p + smush_buf_small[96 + p]] = (byte)tmp_c;
							smush_buf_small[96 + p]++;
						} else {
							smush_buf_small[80 + p + smush_buf_small[97 + p]] = (byte)tmp_c;
							smush_buf_small[97 + p]++;
						}
					} while (tmp_c-- != 0);
				}
			}
			p += 128;
			s += 388;
			tmp_table37_1_2++;
			tmp_table37_2_2++;
		}	while (--count_2 != 0);
		tmp_table37_1_1++;
		tmp_table37_2_1++;
	} while (--count_1!= 0);
}

void Codec47Decoder::makeTables47(int32 width) {
	if (_lastTableWidth == width)
		return;

	_lastTableWidth = width;

	int32 a, c, d, s, tmp_value, tmp_offset;
	int16 tmp, tmp2;

	int16 * tmp_ptr = smush_table;
	int16 * ptr_table = &codec47_table[1];
	do {
		int16 tmp_word = *ptr_table;
		tmp_word *= (int16)width;
		tmp_word += *(ptr_table - 1);
		*tmp_ptr = tmp_word;
		ptr_table += 2;
		tmp_ptr++;
	} while (tmp_ptr < &smush_table[255]);
	a = 0;
	c = 0;
	s = 0;
	tmp_value = 0;
	do {
		d = 0;
		tmp = smush_buf_small[96 + c];
		if (tmp != 0) {
			do {
				tmp = smush_buf_small[64 + c + d];
				tmp2 = tmp;
				tmp2 >>= 2;
				tmp &= 3;
				tmp2 &= 0xFFFF00FF;
				tmp2 *= (int16)width;
				tmp2 += tmp;
				smush_buf_small[(s + d) * 2] = (byte)tmp2;
				smush_buf_small[(s + d) * 2 + 1] = tmp2 >> 8;
				d++;
				tmp = smush_buf_small[96 + c];
			} while (tmp > d);
		}
		d = 0;
		tmp = smush_buf_small[97 + c];
		if (tmp != 0) {
			do {
				tmp = smush_buf_small[80 + c + d];
				tmp2 = tmp;
				tmp2 >>= 2;
				tmp &= 3;
				tmp2 &= 0xFFFF00FF;
				tmp2 *= (int16)width;
				tmp2 += tmp;
				smush_buf_small[32 + (s + d) * 2] = (byte)tmp2;
				smush_buf_small[32 + (s + d) * 2 + 1] = tmp2 >> 8;
				d++;
				tmp = smush_buf_small[97 + c];
			} while (tmp > d);
		}
		d = 0;
		tmp = smush_buf_big[384 + a];
		if (tmp != 0) {
			do {
				tmp = smush_buf_big[256 + a + d];
				tmp2 = tmp >> 3;
				tmp = tmp & 7;
				tmp2 &= 0xFFFF00FF;
				tmp2 *= (int16)width;
				tmp2 += tmp;
				tmp_offset = tmp_value;
				tmp_offset += d;
				d++;
				smush_buf_big[tmp_offset * 2] = (byte)tmp2;
				smush_buf_big[tmp_offset * 2 + 1] = tmp2 >> 8;
				tmp = smush_buf_big[384 + a];
			} while (tmp > d);
		}
		d = 0;
		tmp = smush_buf_big[385 + a];
		if (tmp != 0) {
			do {
				tmp = smush_buf_big[320 + a + d];
				tmp2 = tmp >> 3;
				tmp = tmp & 7;
				tmp2 &= 0xFFFF00FF;
				tmp2 *= (int16)width;
				tmp2 += tmp;
				tmp_offset = tmp_value;
				tmp_offset += d;
				d++;
				smush_buf_big[128 + (tmp_offset * 2)] = (byte)tmp2;
				smush_buf_big[128 + (tmp_offset * 2) + 1] = tmp2 >> 8;
				tmp = smush_buf_big[385 + a];
			} while (tmp > d);
		}
		
		a += 388;
		tmp_value += 194;
		c += 128;
		s += 64;
	} while (s < 16384);
}
	
void Codec47Decoder::bompDecode(byte *dst, byte *src, int32 len) {
	byte code;
	byte color;
	int32 num;

	do {
		code = *src++;
		num = (code >> 1) + 1;
		if (code & 1) {
			color = *src++;
			memset(dst, color, num);
		} else {
			memcpy(dst, src, num);
			src += num;
		}
		dst += num;
	} while (len -= num);
}

static int32 codec47_decode2_offset1;
static int32 codec47_decode2_offset2;
static int32 d_pitch;
static byte * codec47_decode2_param_ptr;
static int32 codec47_subgfx_width_blocks;
static int32 codec47_subgfx_height_blocks;
static int32 codec47_subgfx_width_pixels;
static byte * d_src;

static void codec47_subgfx_lev4(byte * d_dst) {
	int32 tmp;
	byte code = *d_src++;

	if (code < 0xF8) {
		tmp = smush_table[code] + codec47_decode2_offset1;
		*(uint16*)(d_dst + (d_pitch * 0)) = *(uint16*)(d_dst + (d_pitch * 0) + tmp);
		*(uint16*)(d_dst + (d_pitch * 1)) = *(uint16*)(d_dst + (d_pitch * 1) + tmp);
	} else if (code == 0xFF) {
		*(uint16*)(d_dst + (d_pitch * 0)) = *(uint16*)(d_src + 0);
		*(uint16*)(d_dst + (d_pitch * 1)) = *(uint16*)(d_src + 2);
		d_src += 4;
	} else if (code == 0xFE) {
		byte t = *d_src++;
		tmp = t | t << 8;
		*(uint16*)(d_dst + (d_pitch * 0)) = tmp;
		*(uint16*)(d_dst + (d_pitch * 1)) = tmp;
	} else if (code == 0xFC) {
		tmp = codec47_decode2_offset2;
		*(uint16*)(d_dst + (d_pitch * 0)) = *(uint16*)(d_dst + (d_pitch * 0) + tmp);
		*(uint16*)(d_dst + (d_pitch * 1)) = *(uint16*)(d_dst + (d_pitch * 1) + tmp);
	} else {
		byte t = codec47_decode2_param_ptr[code];
		tmp = t | t << 8;
		*(uint16*)(d_dst + (d_pitch * 0)) = tmp;
		*(uint16*)(d_dst + (d_pitch * 1)) = tmp;
	}
}

static void codec47_subgfx_lev3(byte * d_dst) {
	int32 tmp;
	byte code = *d_src++;
	int i;

	if (code < 0xF8) {
		tmp = smush_table[code] + codec47_decode2_offset1;
		for (i = 0; i < 4; i++)
			*(uint32*)(d_dst + (d_pitch * i)) = *(uint32*)(d_dst + (d_pitch * i) + tmp);
	} else if (code == 0xFF) {
		byte * tmp_dst = d_dst;
		codec47_subgfx_lev4(d_dst);
		d_dst += 2;
		codec47_subgfx_lev4(d_dst);
		d_dst += d_pitch * 2 - 2;
		codec47_subgfx_lev4(d_dst);
		d_dst += 2;
		codec47_subgfx_lev4(d_dst);
		d_dst = tmp_dst;
	} else if (code == 0xFE) {
		byte t = *d_src++;
		uint32 val = t << 24 | t << 16 | t << 8 | t;
		for (i = 0; i < 4; i++)
			*(uint32*)(d_dst + (d_pitch * i)) = val;
	} else if (code == 0xFD) {
		byte * tmp_ptr = (*d_src++ << 7) + smush_buf_small;
		int32 l = *(tmp_ptr + 96);
		byte val = *d_src++;
		int16 * tmp_ptr2 = (int16*)tmp_ptr;
		while(l--) {
			*(d_dst + READ_LE_UINT16(tmp_ptr2)) = val;
			tmp_ptr2++;
		}
		l = *(tmp_ptr + 97);
		val = *d_src++;
		tmp_ptr2 = (int16*)(tmp_ptr + 32);
		while(l--) {
			*(d_dst + READ_LE_UINT16(tmp_ptr2)) = val;
			tmp_ptr2++;
		}
	} else if (code == 0xFC) {
		tmp = codec47_decode2_offset2;
		for (i = 0; i < 4; i++)
			*(uint32*)(d_dst + (d_pitch * i)) = *(uint32*)(d_dst + (d_pitch * i) + tmp);
	} else {
		byte t = codec47_decode2_param_ptr[code];
		uint32 val = t << 24 | t << 16 | t << 8 | t;
		for (i = 0; i < 4; i++)
			*(uint32*)(d_dst + (d_pitch * i)) = val;
	}
}

static void codec47_subgfx_lev2(byte * d_dst) {
	int32 tmp, tmp2;
	byte code = *d_src++;
	int i;

	if (code < 0xF8) {
		tmp2 = smush_table[code] + codec47_decode2_offset1;
		for (i = 0; i < 8; i++) {
			*(uint32*)(d_dst + (d_pitch * i) + 0) = *(uint32*)(d_dst + (d_pitch * i) + tmp2);
			*(uint32*)(d_dst + (d_pitch * i) + 4) = *(uint32*)(d_dst + (d_pitch * i) + tmp2 + 4);
		}
	} else if (code == 0xFF) {
		byte * tmp_dst = d_dst;
		codec47_subgfx_lev3(d_dst);
		d_dst += 4;
		codec47_subgfx_lev3(d_dst);
		d_dst += d_pitch * 4 - 4;
		codec47_subgfx_lev3(d_dst);
		d_dst += 4;
		codec47_subgfx_lev3(d_dst);
		d_dst = tmp_dst;
	} else if (code == 0xFE) {
		byte t = *d_src++;
		int32 val = t << 24 | t << 16 | t << 8 | t;
		for (i = 0; i < 8; i++) {
			*(uint32*)(d_dst + (d_pitch * i)) = val;
			*(uint32*)(d_dst + (d_pitch * i) + 4) = val;
		}
	} else if (code == 0xFD) {
		tmp = *d_src++;
		tmp2 = tmp * 4;
		tmp <<= 7;
		int32 tmp3 = tmp2 + tmp * 2;
		byte * tmp_ptr = tmp + tmp3 + smush_buf_big;
		byte l = *(tmp_ptr + 384);
		byte val = *d_src++;
		int16 * tmp_ptr2 = (int16*)tmp_ptr;
		while(l--) {
			*(d_dst + READ_LE_UINT16(tmp_ptr2)) = val;
			tmp_ptr2++;
		}
		l = *(tmp_ptr + 385);
		val = *d_src++;
		tmp_ptr2 = (int16*)(tmp_ptr + 128);
		while(l--) {
			*(d_dst + READ_LE_UINT16(tmp_ptr2)) = val;
			tmp_ptr2++;
		}
	} else if (code == 0xFC) {
		tmp2 = codec47_decode2_offset2;
		for (i = 0; i < 8; i++) {
			*(uint32*)(d_dst + (d_pitch * i) + 0) = *(uint32*)(d_dst + (d_pitch * i) + tmp2);
			*(uint32*)(d_dst + (d_pitch * i) + 4) = *(uint32*)(d_dst + (d_pitch * i) + tmp2 + 4);
		}
	} else {
		byte t = *(codec47_decode2_param_ptr + code);
		int32 val = t << 24 | t << 16 | t << 8 | t;
		for (i = 0; i < 8; i++) {
			*(uint32*)(d_dst + (d_pitch * i)) = val;
			*(uint32*)(d_dst + (d_pitch * i) + 4) = val;
		}
	}
}

void Codec47Decoder::decode2(byte * dst, byte * src, int32 offset1, int32 offset2, int32 pitch,
						 int16 * tmp_table, byte * param_ptr, int32 height,
						 byte * buf_small, byte * buf_big) {
	d_src = src;
	codec47_decode2_offset1 = offset1;
	codec47_decode2_offset2 = offset2;
	d_pitch = pitch;
	codec47_decode2_param_ptr = param_ptr - 0xf8;
	codec47_subgfx_height_blocks = (height + 7) >> 3;
	codec47_subgfx_width_blocks = (pitch + 7) >> 3;
	codec47_subgfx_width_pixels = pitch << 3;
	codec47_subgfx_width_pixels -= pitch;

	do {
		int32 tmp_codec47_subgfx_width_blocks = codec47_subgfx_width_blocks;
		do {
			codec47_subgfx_lev2(dst);
			dst += 8;
		} while (--tmp_codec47_subgfx_width_blocks);
		dst += codec47_subgfx_width_pixels;
	} while (--codec47_subgfx_height_blocks);
}

bool Codec47Decoder::initSize(const Point & p, const Rect & r) {
	if(r.width() != getRect().width() && r.height() != getRect().height()) {
		if(
			(r.width() != 640 || r.height() != 480)
			)
			return false;
		Decoder::initSize(p, r);
		clean();

		makeTables37(4);
		makeTables37(8);

		int32 frame_size = getRect().width() * getRect().height();
		_deltaSize = frame_size * 3;
		_deltaBuf = new byte[_deltaSize];
		_deltaBufs[0] = _deltaBuf;
		_deltaBufs[1] = _deltaBuf + frame_size;
		_curBuf = _deltaBuf + frame_size * 2;

		return true;
	}
	return false;
}

Codec47Decoder::Codec47Decoder() {
	_deltaBuf = 0;
}

void Codec47Decoder::clean() {
	_lastTableWidth = -1;
	if(_deltaBuf) {
		delete []_deltaBuf;
		_deltaSize = 0;
		_deltaBuf = 0;
		_deltaBufs[0] = 0;
		_deltaBufs[1] = 0;
	}
}

Codec47Decoder::~Codec47Decoder() {
	clean();
}

bool Codec47Decoder::decode(Blitter & dst, Chunk & src) {
	int32 width = getRect().width();
	int32 height = getRect().height();
	int32 offset1 = _deltaBufs[1] - _curBuf;
	int32 offset2 = _deltaBufs[0] - _curBuf;

	int32 chunk_size = src.getSize() - 14;
	byte * chunk_buffer = (byte*)malloc(chunk_size);
	src.read(chunk_buffer, chunk_size);

	int32 seq_nb = READ_LE_UINT16(chunk_buffer + 0);

	byte * gfx_data = chunk_buffer + 26;
	byte * tmp_ptr;

	if (seq_nb == 0) {
		makeTables47(width);
		memset(_deltaBufs[0], chunk_buffer[12], width * height);
		memset(_deltaBufs[1], chunk_buffer[13], width * height);
		_prevSeqNb = -1;
	}
	
	if ((chunk_buffer[4] & 1) != 0) {
		gfx_data += 32896;
	}

	switch(chunk_buffer[2]) {
	case 0:
		memcpy(_curBuf, gfx_data, width * height);
		break;
	case 1:
		warning("codec47: not implemented decode1 proc");
		break;
	case 2:
		if (seq_nb == _prevSeqNb + 1) {
			decode2(_curBuf, gfx_data, offset1, offset2, width,	smush_table, 
							chunk_buffer + 8, height, smush_buf_small, smush_buf_big);
		}
		break;
	case 3:
		memcpy(_curBuf, _deltaBufs[1], width * height);
		break;
	case 4:
		memcpy(_curBuf, _deltaBufs[0], width * height);
		break;
	case 5:
		bompDecode(_curBuf, gfx_data, READ_LE_UINT32(chunk_buffer + 14));
		break;
	}

	dst.blit(_curBuf, width * height);

	if (seq_nb == _prevSeqNb + 1) {
		if (chunk_buffer[3] == 1) {
			tmp_ptr = _curBuf;
			_curBuf = _deltaBufs[1];
			_deltaBufs[1] = tmp_ptr;
		} else if (chunk_buffer[3] == 2) {
			tmp_ptr = _deltaBufs[0];
			_deltaBufs[0] = _deltaBufs[1];
			_deltaBufs[1] = _curBuf;
			_curBuf = tmp_ptr;
		}
	}
	_prevSeqNb = seq_nb;

	free(chunk_buffer);

	return true;
}
