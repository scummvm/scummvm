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

// Based on the Bink 2 decoder written for FFmpeg by
// Konstantin Shishkov and Paul B Mahol, which was never merged there,
// with what it lacked taken from RAD's own decoder.

#include "common/scummsys.h"

#ifdef USE_BINK

#include "common/compression/huffman.h"
#include "common/endian.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "graphics/surface.h"
#include "graphics/yuv_to_rgb.h"

#include "image/codecs/codec.h"

#include "video/bink2_decoder.h"

namespace Video {

static constexpr uint32 kKB2fID = MKTAG('K', 'B', '2', 'f');
static constexpr uint32 kKB2gID = MKTAG('K', 'B', '2', 'g');

static constexpr uint32 kVideoFlagAlpha = 0x00100000;

// Frame flags: block flags for deblocking follow the slice offsets. The ones
// for rows and columns of blocks, when left out, also leave their edges
// unfiltered. The low byte is the strength of the blur.
static constexpr uint32 kFrameFlagBlockFlags = 0x10000;
static constexpr uint32 kFrameFlagNoRowFlags = 0x8000;
static constexpr uint32 kFrameFlagNoColFlags = 0x4000;

// Revision 'g': a luma CBP without blocks is not followed by the bit that
// says which set of codes they use
static constexpr uint32 kFrameFlagNoEmptyCBPFlag = 0x40000;

// Border around the luma planes; half of it around the chroma ones
static constexpr int kBorder = 32;

// The original rounds with a hair under a half
static constexpr float kAlmostHalf = 0.49999991f;

static constexpr uint32 kQuantCodes[16] = {
	0x01, 0x02, 0x04, 0x08, 0x10, 0x30, 0x50, 0x70,
	0x00, 0x20, 0x40, 0x60, 0x80, 0xA0, 0xC0, 0xE0
};

static constexpr uint8 kQuantLengths[16] = {
	1, 2, 3, 4, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8
};

static constexpr uint32 kACValueCodes[2][13] = {
	{ 0x04, 0x01, 0x02, 0x00, 0x08, 0x18, 0xF8, 0x178, 0x138, 0x38, 0x1B8, 0x78, 0xB8 },
	{ 0x0A, 0x01, 0x04, 0x08, 0x06, 0x00, 0x02, 0x1A, 0x2A, 0x16A, 0x1EA, 0x6A, 0xEA }
};

static constexpr uint8 kACValueLengths[2][13] = {
	{ 3, 1, 2, 4, 5, 6, 8, 9, 9, 9, 9, 9, 9 },
	{ 6, 1, 3, 4, 3, 4, 4, 5, 7, 9, 9, 9, 9 }
};

static constexpr uint32 kACSkipCodes[2][14] = {
	{ 0x00, 0x01, 0x0D, 0x15, 0x45, 0x85, 0xA5, 0x165, 0x65, 0x1E5, 0xE5, 0x25, 0x03, 0x05 },
	{ 0x00, 0x01, 0x03, 0x07, 0x1F, 0x1B, 0x0F, 0x2F, 0x5B, 0xDB, 0x1DB, 0x3B, 0x05, 0x0B }
};

static constexpr uint8 kACSkipLengths[2][14] = {
	{ 1, 3, 4, 5, 7, 8, 8, 9, 9, 9, 9, 8, 2, 8 },
	{ 1, 3, 4, 4, 5, 7, 6, 6, 8, 9, 9, 6, 3, 5 }
};

// How many coefficients a skip symbol jumps over (11: read 6 bits; 62: to the end)
static constexpr byte kACSkips[14] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 62, 0
};

// How many coefficients follow a skip symbol before the next one
static constexpr byte kACNextSkips[14] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7
};

static constexpr float kDCQuant[16] = {
	4, 4, 4, 4, 4, 6, 7, 8, 10, 12, 16, 24, 32, 48, 64, 128
};

static constexpr byte kDCQuantBits[16] = {
	1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 4, 4, 5, 6
};

static constexpr float kACQuant[16] = {
	1.0f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f, 6.0f, 7.0f, 8.0f, 12.0f, 16.0f, 24.0f, 32.0f, 48.0f, 64.0f, 128.0f
};

static constexpr float kLumaIntraQmat[64] = {
	0.125f,    0.190718f, 0.16332f,  0.235175f, 0.3f,      0.392847f, 0.345013f, 0.210373f,
	0.208056f, 0.288582f, 0.317145f, 0.387359f, 0.450788f, 0.790098f, 0.562995f, 0.263095f,
	0.228649f, 0.294491f, 0.341421f, 0.460907f, 0.653281f, 0.731424f, 0.60988f,  0.252336f,
	0.205778f, 0.346585f, 0.422498f, 0.501223f, 0.749621f, 1.004719f, 0.636379f, 0.251428f,
	0.225f,    0.381436f, 0.604285f, 0.823113f, 0.85f,     1.070509f, 0.69679f,  0.265553f,
	0.235708f, 0.476783f, 0.70576f,  0.739104f, 0.795516f, 0.802512f, 0.600616f, 0.249289f,
	0.331483f, 0.600528f, 0.689429f, 0.692062f, 0.69679f,  0.643138f, 0.43934f,  0.188511f,
	0.248309f, 0.440086f, 0.42807f,  0.397419f, 0.386259f, 0.270966f, 0.192244f, 0.094199f
};

static constexpr float kLumaInterQmat[64] = {
	0.125f,    0.17338f,  0.16332f,  0.146984f, 0.128475f, 0.106393f, 0.077046f, 0.043109f,
	0.17338f,  0.240485f, 0.226532f, 0.203873f, 0.1782f,   0.147571f, 0.109474f, 0.062454f,
	0.16332f,  0.226532f, 0.219321f, 0.202722f, 0.181465f, 0.149711f, 0.112943f, 0.062584f,
	0.146984f, 0.203873f, 0.202722f, 0.201647f, 0.183731f, 0.153976f, 0.11711f,  0.065335f,
	0.128475f, 0.1782f,   0.181465f, 0.183731f, 0.177088f, 0.155499f, 0.120267f, 0.068016f,
	0.106393f, 0.147571f, 0.149711f, 0.153976f, 0.155499f, 0.145756f, 0.116636f, 0.068495f,
	0.077046f, 0.109474f, 0.112943f, 0.11711f,  0.120267f, 0.116636f, 0.098646f, 0.060141f,
	0.043109f, 0.062454f, 0.062584f, 0.065335f, 0.068016f, 0.068495f, 0.060141f, 0.038853f
};

static constexpr float kChromaQmat[64] = {
	0.125f,      0.17338f,    0.217761f,   0.383793f,   0.6875f,     0.54016501f, 0.37207201f, 0.18968099f,
	0.17338f,    0.28056601f, 0.32721299f, 0.74753499f, 0.95358998f, 0.74923098f, 0.51607901f, 0.26309499f,
	0.217761f,   0.32721299f, 0.66387498f, 1.056244f,   0.89826202f, 0.70576f,    0.48613599f, 0.24783f,
	0.383793f,   0.74753499f, 1.056244f,   0.95059502f, 0.80841398f, 0.635167f,   0.437511f,   0.223041f,
	0.6875f,     0.95358998f, 0.89826202f, 0.80841398f, 0.6875f,     0.54016501f, 0.37207201f, 0.18968099f,
	0.54016501f, 0.74923098f, 0.70576f,    0.635167f,   0.54016501f, 0.42440501f, 0.292335f,   0.149031f,
	0.37207201f, 0.51607901f, 0.48613599f, 0.437511f,   0.37207201f, 0.292335f,   0.201364f,   0.102655f,
	0.18968099f, 0.26309499f, 0.24783f,    0.223041f,   0.18968099f, 0.149031f,   0.102655f,   0.052333001f
};

static constexpr byte kLumaScan[64] = {
	 0,  2,  1,  8,  9, 17, 10, 16,
	24,  3, 18, 25, 32, 11, 33, 26,
	 4, 40, 19, 12, 27, 41, 34,  5,
	20, 48,  6, 28, 15, 42, 23, 35,
	21, 13, 14,  7, 31, 43, 49, 36,
	22, 56, 39, 50, 30, 44, 29, 51,
	57, 47, 58, 59, 63, 61, 55, 38,
	52, 62, 45, 37, 60, 46, 54, 53
};

static constexpr byte kChromaScan[64] = {
	 0,  1,  8,  2,  9, 16, 10, 17,
	 3, 24, 11, 18, 25, 13, 14,  4,
	15,  5,  6,  7, 12, 19, 20, 21,
	22, 23, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39,
	40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55,
	56, 57, 58, 59, 60, 61, 62, 63
};

// The 8x8 blocks of a 32x32 luma macroblock in coding order, as x + 4 * y
static constexpr byte kLumaBlockPos[16] = {
	0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15
};

// Revision 'g'

static constexpr byte kScanG[64] = {
	 0,  8,  1,  2,  9, 16, 24, 17,
	10,  3,  4, 11, 18, 25, 32, 40,
	33, 26, 19, 12,  5,  6, 13, 20,
	27, 34, 41, 48, 56, 49, 42, 35,
	28, 21, 14,  7, 15, 22, 29, 36,
	43, 50, 57, 58, 51, 44, 37, 30,
	23, 31, 38, 45, 52, 59, 60, 53,
	46, 39, 47, 54, 61, 62, 55, 63
};

static constexpr uint32 kACSkipCodesG[2][14] = {
	{ 0x01, 0x00, 0x04, 0x2C, 0x6C, 0x0C, 0x4C, 0xAC, 0xEC, 0x12C, 0x16C, 0x1AC, 0x02, 0x1C },
	{ 0x01, 0x04, 0x00, 0x08, 0x02, 0x32, 0x0A, 0x12, 0x3A, 0x7A, 0xFA, 0x72, 0x06, 0x1A }
};

static constexpr uint8 kACSkipLengthsG[2][14] = {
	{ 1, 3, 4, 9, 9, 7, 7, 9, 8, 9, 9, 9, 2, 5 },
	{ 1, 3, 4, 4, 5, 7, 5, 6, 7, 8, 8, 7, 3, 6 }
};

static constexpr uint32 kMotionCodesG[16] = {
	0x01, 0x06, 0x0C, 0x1C, 0x18, 0x38, 0x58, 0x78, 0x68, 0x48, 0x28, 0x08, 0x14, 0x04, 0x02, 0x00
};

static constexpr uint8 kMotionLengthsG[16] = {
	1, 3, 5, 5, 7, 7, 7, 7, 7, 7, 7, 7, 5, 5, 3, 4
};

// How many coefficients a skip symbol jumps over (11: read 6 bits; 64: to the end)
static constexpr byte kACSkipsG[14] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 64, 0
};

// The chroma blocks a macroblock codes when it repeats the previous ones, by them
static constexpr byte kChromaCBPG[16] = {
	0x00, 0x00, 0x00, 0x0F, 0x00, 0x0F, 0x0F, 0x0F, 0x00, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F
};

// The step of the DC differences by quantizer, in 1/1024
static constexpr int kDCStepG[37] = {
	1024, 1218, 1448, 1722, 2048, 2435, 2896, 3444,
	4096, 4871, 5793, 6889, 8192, 9742, 11585, 13777,
	16384, 19484, 23170, 27555, 32768, 38968, 46341, 55109,
	65536, 77936, 92682, 110218, 131072, 155872, 185364, 220436,
	262144, 311744, 370728, 440872, 524288
};

// The dequantization matrices: the weight that each of the 64 coefficients
// of a block is multiplied by, coarser for the higher frequencies, in fixed
// point with 7 fractional bits. Quantizer q picks the matrix by q & 3 and
// doubles the weights every 4 steps (1 << (q >> 2)). The DC doesn't use them.
static constexpr uint16 kLumaIntraQmatG[4][64] = {
	{
		 1024,  1432,  1506,  1181,  1843,  2025,  5271,  8592,
		 1313,  1669,  1630,  1672,  2625,  3442,  8023, 12794,
		 1076,  1755,  1808,  1950,  3980,  4875,  8813, 11909,
		 1350,  1868,  2127,  2016,  4725,  4450,  7712,  9637,
		 2458,  3103,  4303,  4303,  6963,  6835, 11079, 13365,
		 3375,  5704,  5052,  6049,  9198,  7232, 10725,  9834,
		 5486,  7521,  7797,  7091, 11079, 10016, 13559, 12912,
		 7279,  7649,  7020,  6097,  9189,  9047, 12661, 13768
	},
	{
		 1218,  1703,  1791,  1405,  2192,  2408,  6268, 10218,
		 1561,  1985,  1938,  1988,  3122,  4093,  9541, 15215,
		 1279,  2087,  2150,  2319,  4733,  5798, 10481, 14162,
		 1606,  2222,  2530,  2398,  5619,  5292,  9171, 11460,
		 2923,  3690,  5117,  5118,  8281,  8128, 13176, 15894,
		 4014,  6783,  6008,  7194, 10938,  8600, 12755, 11694,
		 6524,  8944,  9272,  8433, 13176, 11911, 16125, 15354,
		 8657,  9096,  8348,  7250, 10927, 10759, 15056, 16373
	},
	{
		 1448,  2025,  2130,  1671,  2607,  2864,  7454, 12151,
		 1856,  2360,  2305,  2364,  3713,  4867, 11346, 18094,
		 1521,  2482,  2557,  2758,  5628,  6894, 12464, 16841,
		 1909,  2642,  3008,  2852,  6683,  6293, 10906, 13629,
		 3476,  4388,  6085,  6086,  9847,  9666, 15668, 18901,
		 4773,  8066,  7145,  8555, 13007, 10227, 15168, 13907,
		 7758, 10637, 11026, 10028, 15668, 14165, 19175, 18259,
		10294, 10817,  9927,  8622, 12995, 12794, 17905, 19470
	},
	{
		 1722,  2408,  2533,  1987,  3100,  3406,  8864, 14450,
		 2208,  2807,  2741,  2811,  4415,  5788, 13493, 21517,
		 1809,  2951,  3041,  3280,  6693,  8199, 14822, 20028,
		 2271,  3142,  3578,  3391,  7947,  7484, 12969, 16207,
		 4133,  5218,  7236,  7238, 11711, 11495, 18633, 22478,
		 5677,  9592,  8497, 10174, 15469, 12162, 18038, 16538,
		 9226, 12649, 13112, 11926, 18633, 16845, 22804, 21715,
		12242, 12864, 11806, 10254, 15454, 15215, 21293, 23155
	}
};

static constexpr uint16 kChromaIntraQmatG[4][64] = {
	{
		 1024,  1193,  1434,  2203,  5632,  4641,  5916,  6563,
		 1193,  1622,  1811,  3606,  6563,  5408,  6894,  7649,
		 1434,  1811,  3515,  4875,  5916,  4875,  6215,  6894,
		 2203,  3606,  4875,  3824,  4641,  3824,  4875,  5408,
		 5632,  6563,  5916,  4641,  5632,  4641,  5916,  6563,
		 4641,  5408,  4875,  3824,  4641,  3824,  4875,  5408,
		 5916,  6894,  6215,  4875,  5916,  4875,  6215,  6894,
		 6563,  7649,  6894,  5408,  6563,  5408,  6894,  7649
	},
	{
		 1218,  1419,  1706,  2620,  6698,  5519,  7035,  7805,
		 1419,  1929,  2153,  4288,  7805,  6432,  8199,  9096,
		 1706,  2153,  4180,  5798,  7035,  5798,  7390,  8199,
		 2620,  4288,  5798,  4548,  5519,  4548,  5798,  6432,
		 6698,  7805,  7035,  5519,  6698,  5519,  7035,  7805,
		 5519,  6432,  5798,  4548,  5519,  4548,  5798,  6432,
		 7035,  8199,  7390,  5798,  7035,  5798,  7390,  8199,
		 7805,  9096,  8199,  6432,  7805,  6432,  8199,  9096
	},
	{
		 1448,  1688,  2028,  3116,  7965,  6563,  8367,  9282,
		 1688,  2294,  2561,  5099,  9282,  7649,  9750, 10817,
		 2028,  2561,  4971,  6894,  8367,  6894,  8789,  9750,
		 3116,  5099,  6894,  5408,  6563,  5408,  6894,  7649,
		 7965,  9282,  8367,  6563,  7965,  6563,  8367,  9282,
		 6563,  7649,  6894,  5408,  6563,  5408,  6894,  7649,
		 8367,  9750,  8789,  6894,  8367,  6894,  8789,  9750,
		 9282, 10817,  9750,  7649,  9282,  7649,  9750, 10817
	},
	{
		 1722,  2007,  2412,  3706,  9472,  7805,  9950, 11038,
		 2007,  2729,  3045,  6064, 11038,  9096, 11595, 12864,
		 2412,  3045,  5912,  8199,  9950,  8199, 10452, 11595,
		 3706,  6064,  8199,  6432,  7805,  6432,  8199,  9096,
		 9472, 11038,  9950,  7805,  9472,  7805,  9950, 11038,
		 7805,  9096,  8199,  6432,  7805,  6432,  8199,  9096,
		 9950, 11595, 10452,  8199,  9950,  8199, 10452, 11595,
		11038, 12864, 11595,  9096, 11038,  9096, 11595, 12864
	}
};

static constexpr uint16 kInterQmatG[4][64] = {
	{
		 1024,  1193,  1076,   844,  1052,   914,  1225,  1492,
		 1193,  1391,  1254,   983,  1227,  1065,  1463,  1816,
		 1076,  1254,  1161,   936,  1195,  1034,  1444,  1741,
		  844,   983,   936,   811,  1055,   927,  1305,  1584,
		 1052,  1227,  1195,  1055,  1451,  1336,  1912,  2354,
		  914,  1065,  1034,   927,  1336,  1313,  1945,  2486,
		 1225,  1463,  1444,  1305,  1912,  1945,  3044,  4039,
		 1492,  1816,  1741,  1584,  2354,  2486,  4039,  5679
	},
	{
		 1218,  1419,  1279,  1003,  1252,  1087,  1457,  1774,
		 1419,  1654,  1491,  1169,  1459,  1267,  1739,  2159,
		 1279,  1491,  1381,  1113,  1421,  1230,  1717,  2070,
		 1003,  1169,  1113,   965,  1254,  1103,  1552,  1884,
		 1252,  1459,  1421,  1254,  1725,  1589,  2274,  2799,
		 1087,  1267,  1230,  1103,  1589,  1562,  2313,  2956,
		 1457,  1739,  1717,  1552,  2274,  2313,  3620,  4803,
		 1774,  2159,  2070,  1884,  2799,  2956,  4803,  6753
	},
	{
		 1448,  1688,  1521,  1193,  1488,  1293,  1732,  2110,
		 1688,  1967,  1773,  1391,  1735,  1507,  2068,  2568,
		 1521,  1773,  1642,  1323,  1690,  1462,  2042,  2462,
		 1193,  1391,  1323,  1147,  1492,  1311,  1845,  2241,
		 1488,  1735,  1690,  1492,  2052,  1889,  2704,  3328,
		 1293,  1507,  1462,  1311,  1889,  1857,  2751,  3515,
		 1732,  2068,  2042,  1845,  2704,  2751,  4306,  5712,
		 2110,  2568,  2462,  2241,  3328,  3515,  5712,  8031
	},
	{
		 1722,  2007,  1809,  1419,  1770,  1537,  2060,  2509,
		 2007,  2339,  2108,  1654,  2063,  1792,  2460,  3054,
		 1809,  2108,  1953,  1574,  2010,  1739,  2428,  2928,
		 1419,  1654,  1574,  1364,  1774,  1559,  2195,  2664,
		 1770,  2063,  2010,  1774,  2440,  2247,  3216,  3958,
		 1537,  1792,  1739,  1559,  2247,  2209,  3271,  4181,
		 2060,  2460,  2428,  2195,  3216,  3271,  5120,  6793,
		 2509,  3054,  2928,  2664,  3958,  4181,  6793,  9550
	}
};

static inline byte clipPixel(int value) {
	return (byte)CLIP(value, 0, 255);
}

// The kind of a block, by its number of AC coefficients: blocks with fewer
// get more of their pixels changed by the deblocking
static int getBlockKind(int coefCount) {
	return coefCount == 0 ? 7 : (coefCount < 4 ? 6 : (coefCount < 8 ? 5 : 4));
}

// The kind of an intra block and its DC
static uint16 makeBlockWord(int coefCount, float dc) {
	return (uint16)((getBlockKind(coefCount) << 13) | ((int)(dc + kAlmostHalf) & 0x1FFF));
}

static inline int countBits4(uint32 value) {
	return (value & 1) + ((value >> 1) & 1) + ((value >> 2) & 1) + ((value >> 3) & 1);
}

static inline int median(int a, int b, int c) {
	return MAX(MIN(a, b), MIN(MAX(a, b), c));
}

// Predicts from three neighbors, as a + b - c bounded by them
static inline float predictMedian(float a, float b, float c) {
	return MIN(MAX(c + b - a, MIN(a, MIN(b, c))), MAX(a, MAX(b, c)));
}

// Predicts from two neighbors
static inline float predictMedian2(float a, float b) {
	return MIN(MAX(a, b), MAX(MIN(a, b), 2 * a - b));
}

static void idct1D(float *block, int step) {
	const float t00 =  block[2 * step] + block[6 * step];
	const float t01 = (block[2 * step] - block[6 * step]) * 1.4142135f - t00;
	const float t02 =  block[0 * step] + block[4 * step];
	const float t03 =  block[0 * step] - block[4 * step];
	const float t04 =  block[3 * step] + block[5 * step];
	const float t05 =  block[3 * step] - block[5 * step];
	const float t06 =  block[1 * step] + block[7 * step];
	const float t07 =  block[1 * step] - block[7 * step];
	const float t08 = t02 + t00;
	const float t09 = t02 - t00;
	const float t10 = t03 + t01;
	const float t11 = t03 - t01;
	const float t12 = t06 + t04;
	const float t13 = (t06 - t04) * 1.4142135f;
	const float t14 = (t07 - t05) * 1.847759f;
	const float t15 = t05 * 2.613126f + t14 - t12;
	const float t16 = t13 - t15;
	const float t17 = t07 * 1.0823922f - t14 + t16;

	block[0 * step] = t08 + t12;
	block[1 * step] = t10 + t15;
	block[2 * step] = t11 + t16;
	block[3 * step] = t09 - t17;
	block[4 * step] = t09 + t17;
	block[5 * step] = t11 - t16;
	block[6 * step] = t10 - t15;
	block[7 * step] = t08 - t12;
}

// Puts the block in place, or adds it to what is there. The DC comes with a
// bias of 512.5, rounding included, as in the original.
static void idct(byte *dst, int pitch, float *block, float dc, bool isAdded) {
	block[0] = (dc + 4100.0f) * 0.125f;

	for (int i = 0; i < 8; i++)
		idct1D(block + i, 8);

	for (int i = 0; i < 8; i++) {
		idct1D(block, 1);

		for (int j = 0; j < 8; j++) {
			const int value = (int)block[j] - 512;
			dst[j] = clipPixel(isAdded ? dst[j] + value : value);
		}

		block += 8;
		dst += pitch;
	}
}

// Adds a residue that has only a DC
static void addFlat(byte *dst, int pitch, float dc) {
	const int value = (int)(dc * 0.125f + kAlmostHalf);
	for (int j = 0; j < 8; j++, dst += pitch)
		for (int i = 0; i < 8; i++)
			dst[i] = clipPixel(dst[i] + value);
}

static void copyBlock(byte *dst, const byte *src, int pitch, int width, int height) {
	for (int i = 0; i < height; i++) {
		memcpy(dst, src, width);
		dst += pitch;
		src += pitch;
	}
}

// Half pixel luma filter, over the pixels step apart around src[0] and src[step]
template<typename T>
static inline int lumaFilter(const T *src, int step) {
	return ((((src[0] + src[step]) * 19) >> 1) - (src[-step] + src[2 * step]) * 2 + ((src[-2 * step] + src[3 * step]) >> 1) + 8) >> 4;
}

// A 16x16 luma block at a half pixel position: mode bit 0 for x, bit 1 for y
static void lumaBlock(byte *dst, const byte *src, int pitch, int mode) {
	switch (mode) {
	case 0:
		copyBlock(dst, src, pitch, 16, 16);
		break;
	case 1:
		for (int j = 0; j < 16; j++) {
			for (int i = 0; i < 16; i++)
				dst[i] = clipPixel(lumaFilter(src + i, 1));
			dst += pitch;
			src += pitch;
		}
		break;
	case 2:
		for (int j = 0; j < 16; j++) {
			for (int i = 0; i < 16; i++)
				dst[i] = clipPixel(lumaFilter(src + i, pitch));
			dst += pitch;
			src += pitch;
		}
		break;
	default: {
		// The first pass is not clipped
		int16 temp[21 * 16];

		src -= 2 * pitch;
		for (int j = 0; j < 21; j++) {
			for (int i = 0; i < 16; i++)
				temp[j * 16 + i] = lumaFilter(src + i, 1);
			src += pitch;
		}

		for (int j = 0; j < 16; j++) {
			for (int i = 0; i < 16; i++)
				dst[i] = clipPixel(lumaFilter(temp + (j + 2) * 16 + i, 16));
			dst += pitch;
		}
		break;
	}
	}
}

// Quarter pixel chroma filter, between a and b
static inline int chromaFilter(int a, int b, int fraction) {
	switch (fraction) {
	case 1:
		return (6 * a + 2 * b + 4) >> 3;
	case 2:
		return (a + b + 1) >> 1;
	default:
		return (2 * a + 6 * b + 4) >> 3;
	}
}

// An 8x8 chroma block at a quarter pixel position: mode bits 0-1 for x, 2-3 for y
static void chromaBlock(byte *dst, const byte *src, int pitch, int mode) {
	const int fractionX = mode & 3;
	const int fractionY = mode >> 2;

	if (!fractionX && !fractionY) {
		copyBlock(dst, src, pitch, 8, 8);
	} else if (!fractionY) {
		for (int j = 0; j < 8; j++) {
			for (int i = 0; i < 8; i++)
				dst[i] = clipPixel(chromaFilter(src[i], src[i + 1], fractionX));
			dst += pitch;
			src += pitch;
		}
	} else if (!fractionX) {
		for (int j = 0; j < 8; j++) {
			for (int i = 0; i < 8; i++)
				dst[i] = clipPixel(chromaFilter(src[i], src[i + pitch], fractionY));
			dst += pitch;
			src += pitch;
		}
	} else {
		// Both ways at once, rounded once
		const int x1 = 2 * fractionX;
		const int x0 = 8 - x1;
		const int y1 = 2 * fractionY;
		const int y0 = 8 - y1;

		for (int j = 0; j < 8; j++) {
			for (int i = 0; i < 8; i++) {
				const int top = x0 * src[i] + x1 * src[i + 1];
				const int bottom = x0 * src[i + pitch] + x1 * src[i + pitch + 1];
				dst[i] = clipPixel((y0 * top + y1 * bottom + 32) >> 6);
			}
			dst += pitch;
			src += pitch;
		}
	}
}

// Eight times the average of an 8x8 block, as the coder takes it
static int averageBlock(const byte *src, int pitch) {
	int sum = 0;

	for (int i = 0; i < 8; i++) {
		const int a = (src[i + 0 * pitch] + src[i + 1 * pitch] + 1) >> 1;
		const int b = (src[i + 2 * pitch] + src[i + 3 * pitch] + 1) >> 1;
		const int c = (src[i + 4 * pitch] + src[i + 5 * pitch] + 1) >> 1;
		const int d = (src[i + 6 * pitch] + src[i + 7 * pitch] + 1) >> 1;
		const int e = (a + b + 1) >> 1;
		const int f = (c + d + 1) >> 1;
		sum += (e + f + 1) >> 1;
	}

	return sum;
}

// Masks for the edge filter, by code: bit 0 changes p0, bit 1 p1 and p0,
// bit 2 q0, bit 3 q0 and q1. Code 15 (two blocks with only a DC, which the
// blur already smoothed) changes nothing.
static void filterEdge(byte *p, int step, int code) {
	if (code == 15 || code == 0)
		return;

	const int d = p[0] - p[-step];
	const int outer = (2 * d + 8) >> 4;
	const int inner = (4 * d + 8) >> 4;

	if (code & 2)
		p[-2 * step] = clipPixel(p[-2 * step] + outer);
	if (code & 3)
		p[-step] = clipPixel(p[-step] + inner);
	if (code & 12)
		p[0] = clipPixel(p[0] - inner);
	if (code & 8)
		p[step] = clipPixel(p[step] - outer);
}

static inline int16 asInt16(int value) {
	return (int16)(uint16)value;
}

// The weights of the DCs around a block drawn from its DC alone, over its
// pixels: of the top left one, of the top one and of its own. The other
// corners and sides are mirrored.
static constexpr float kBlurCorner[8][8] = {
	{ 0.024111325f, 0.017566822f, 0.011538992f, 0.006372279f, 0.0024111324f, 0.0f, 0.0f, 0.0f },
	{ 0.017566822f, 0.012675668f, 0.008197851f, 0.004408928f, 0.0015844585f, 0.0f, 0.0f, 0.0f },
	{ 0.011538992f, 0.008197851f, 0.0051667127f, 0.0026522458f, 0.0008611188f, 0.0f, 0.0f, 0.0f },
	{ 0.006372279f, 0.004408928f, 0.0026522458f, 0.0012400111f, 0.00031000277f, 0.0f, 0.0f, 0.0f },
	{ 0.0024111324f, 0.0015844585f, 0.0008611188f, 0.00031000277f, 0.0f, 0.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }
};

static constexpr float kBlurEdge[8][8] = {
	{ 0.030828051f, 0.037372556f, 0.043400384f, 0.046155967f, 0.046155967f, 0.043400384f, 0.037372556f, 0.030828051f },
	{ 0.022595756f, 0.027486911f, 0.031964727f, 0.034169193f, 0.034169193f, 0.031964727f, 0.027486911f, 0.022595756f },
	{ 0.014983467f, 0.018324608f, 0.021355746f, 0.023009093f, 0.023009093f, 0.021355746f, 0.018324608f, 0.014983467f },
	{ 0.008404519f, 0.01036787f, 0.012124552f, 0.013226784f, 0.013226784f, 0.012124552f, 0.01036787f, 0.008404519f },
	{ 0.0032722512f, 0.0040989253f, 0.004822265f, 0.005373381f, 0.005373381f, 0.004822265f, 0.0040989253f, 0.0032722512f },
	{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }
};

static constexpr float kBlurCenter[8][8] = {
	{ 0.039267015f, 0.04749931f, 0.055111602f, 0.058418296f, 0.058418296f, 0.055111602f, 0.04749931f, 0.039267015f },
	{ 0.04749931f, 0.057384953f, 0.06654726f, 0.07040507f, 0.07040507f, 0.06654726f, 0.057384953f, 0.04749931f },
	{ 0.055111602f, 0.06654726f, 0.07715624f, 0.08156517f, 0.08156517f, 0.07715624f, 0.06654726f, 0.055111602f },
	{ 0.058418296f, 0.07040507f, 0.08156517f, 0.0859741f, 0.0859741f, 0.08156517f, 0.07040507f, 0.058418296f },
	{ 0.058418296f, 0.07040507f, 0.08156517f, 0.0859741f, 0.0859741f, 0.08156517f, 0.07040507f, 0.058418296f },
	{ 0.055111602f, 0.06654726f, 0.07715624f, 0.08156517f, 0.08156517f, 0.07715624f, 0.06654726f, 0.055111602f },
	{ 0.04749931f, 0.057384953f, 0.06654726f, 0.07040507f, 0.07040507f, 0.06654726f, 0.057384953f, 0.04749931f },
	{ 0.039267015f, 0.04749931f, 0.055111602f, 0.058418296f, 0.058418296f, 0.055111602f, 0.04749931f, 0.039267015f }
};


Bink2Decoder::Bink2Decoder(bool skipOutsideBlocks) : _stream(nullptr), _audioTrackCount(0), _skipOutsideBlocks(skipOutsideBlocks) {
}

Bink2Decoder::~Bink2Decoder() {
	close();
}

bool Bink2Decoder::isBink2(Common::SeekableReadStream &stream) {
	const int64 pos = stream.pos();
	const uint32 id = stream.readUint32BE();
	stream.seek(pos);
	return id == kKB2fID || id == kKB2gID;
}

// The container is the one of Bink videos
bool Bink2Decoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	const uint32 id = stream->readUint32BE();
	if (id != kKB2fID && id != kKB2gID) {
		delete stream;
		return false;
	}

	const uint32 fileSize   = stream->readUint32LE() + 8;
	const uint32 frameCount = stream->readUint32LE();
	const uint32 largestFrameSize = stream->readUint32LE();
	stream->skip(4);
	const uint32 width  = stream->readUint32LE();
	const uint32 height = stream->readUint32LE();
	const uint32 frameRateNum = stream->readUint32LE();
	const uint32 frameRateDen = stream->readUint32LE();
	const uint32 videoFlags = stream->readUint32LE();
	_audioTrackCount = stream->readUint32LE();

	if (frameCount == 0 || largestFrameSize > fileSize || frameRateNum == 0 || frameRateDen == 0) {
		warning("Bink2Decoder: Bad header");
		delete stream;
		return false;
	}

	addTrack(new Bink2VideoTrack((char)(id & 0xFF), _skipOutsideBlocks, width, height, videoFlags, frameCount, Common::Rational(frameRateNum, frameRateDen)));

	// The maximum packet sizes, then the rate and the flags of each track,
	// then their IDs
	stream->skip(_audioTrackCount * 4);
	for (uint32 i = 0; i < _audioTrackCount; i++) {
		const uint16 sampleRate = stream->readUint16LE();
		const uint16 flags = stream->readUint16LE();
		addTrack(new BinkDecoder::BinkAudioTrack(sampleRate, flags, getSoundType()));
	}
	stream->skip(_audioTrackCount * 4);

	_frames.resize(frameCount);
	for (uint32 i = 0; i < frameCount; i++) {
		const uint32 offset = stream->readUint32LE();
		_frames[i].offset = offset & ~1;
		_frames[i].isKeyFrame = (offset & 1) != 0;
		if (i != 0)
			_frames[i - 1].size = _frames[i].offset - _frames[i - 1].offset;
	}
	_frames[frameCount - 1].size = stream->size() - _frames[frameCount - 1].offset;

	_stream = stream;
	return true;
}

void Bink2Decoder::close() {
	VideoDecoder::close();

	delete _stream;
	_stream = nullptr;
	_frames.clear();
	_audioTrackCount = 0;
}

void Bink2Decoder::readNextPacket() {
	Bink2VideoTrack *track = (Bink2VideoTrack *)getTrack(0);
	if (track->endOfTrack())
		return;

	const Frame &frame = _frames[track->getCurFrame() + 1];
	if (!_stream->seek(frame.offset))
		error("Bink2Decoder: Bad seek");

	// The audio packets come first
	uint32 size = frame.size;
	for (uint32 i = 0; i < _audioTrackCount; i++) {
		const uint32 audioSize = _stream->readUint32LE();
		if (audioSize + 4 > size)
			error("Bink2Decoder: Audio packet too big for the frame");

		const int64 audioEnd = _stream->pos() + audioSize;
		if (audioSize >= 4)
			((BinkDecoder::BinkAudioTrack *)getTrack(i + 1))->decodePacket(*_stream, audioSize);
		_stream->seek(audioEnd);
		size -= audioSize + 4;
	}

	track->decodePacket(*_stream, size, frame.isKeyFrame);
}

uint32 Bink2Decoder::findKeyFrame(uint32 frame) const {
	for (int i = frame; i >= 0; i--) {
		if (_frames[i].isKeyFrame)
			return i;
	}

	return frame;
}

// From the key frame before, decoding the frames in between
bool Bink2Decoder::seekIntern(const Audio::Timestamp &time) {
	Bink2VideoTrack *track = (Bink2VideoTrack *)getTrack(0);
	const uint32 frame = track->getFrameAtTime(time);
	const uint32 keyFrame = findKeyFrame(frame);

	track->setCurFrame((int)keyFrame - 1);
	for (uint32 i = 0; i < _audioTrackCount; i++)
		((BinkDecoder::BinkAudioTrack *)getTrack(i + 1))->seek(track->getFrameTime(keyFrame));

	if (frame == keyFrame)
		return true;

	while (track->getCurFrame() < (int)frame - 1)
		readNextPacket();

	// The audio of the frames in between isn't played
	for (uint32 i = 0; i < _audioTrackCount; i++) {
		BinkDecoder::BinkAudioTrack *audioTrack = (BinkDecoder::BinkAudioTrack *)getTrack(i + 1);
		const int rate = audioTrack->getRate();
		audioTrack->skipSamples(track->getFrameTime(frame - 1).convertToFramerate(rate) -
			track->getFrameTime(keyFrame).convertToFramerate(rate));
	}

	return true;
}

VideoDecoder::AudioTrack *Bink2Decoder::getAudioTrack(int index) {
	// Counted from the first audio track, which follows the video one
	Track *track = getTrack(index + 1);
	if (!track || track->getTrackType() != Track::kTrackTypeAudio)
		return nullptr;

	return (AudioTrack *)track;
}

Bink2Decoder::Bink2VideoTrack::Bink2VideoTrack(char revision, bool skipOutsideBlocks, uint32 width, uint32 height, uint32 flags, uint32 frameCount, const Common::Rational &frameRate) :
		_revision(revision), _skipOutsideBlocks(skipOutsideBlocks && revision == 'f'), _width(width), _height(height), _frameCount(frameCount), _curFrameNumber(-1), _frameRate(frameRate),
		_surface(nullptr), _curFrame(0), _comp(0), _mbPos(0) {
	_pixelFormat = Image::Codec::getDefaultYUVFormat();

	_alignedWidth  = (width  + 31) & ~31;
	_alignedHeight = (height + 31) & ~31;
	_hasAlpha = (flags & kVideoFlagAlpha) != 0;
	_numPlanes = _hasAlpha ? 4 : 3;

	for (int plane = 0; plane < 4; plane++) {
		const bool isChroma = plane == 1 || plane == 2;
		const int border = isChroma ? kBorder / 2 : kBorder;
		const int width2  = (isChroma ? _alignedWidth  / 2 : _alignedWidth)  + 2 * border;
		const int height2 = (isChroma ? _alignedHeight / 2 : _alignedHeight) + 2 * border;

		_pitches[plane] = width2;
		_planeSizes[plane] = width2 * height2;

		for (int frame = 0; frame < 2; frame++) {
			if (plane < _numPlanes) {
				_buffers[frame][plane] = new byte[_planeSizes[plane]];
				_planes[frame][plane] = _buffers[frame][plane] + border * width2 + border;
			} else {
				_buffers[frame][plane] = nullptr;
				_planes[frame][plane] = nullptr;
			}
		}
	}

	// One more column than there are macroblocks: the first macroblock of a
	// row looks at the one to its upper right, which a narrow video lacks
	const uint32 columns = _alignedWidth / 32 + 1;
	for (int i = 0; i < 2; i++) {
		_dcRows[i].resize(columns);
		memset(_dcRows[i].data(), 0, columns * sizeof(DCPredict));
		_mvRows[i].resize(columns);
		memset(_mvRows[i].data(), 0, columns * sizeof(MotionVectors));
		_intDCRows[i].resize(columns);
		memset(_intDCRows[i].data(), 0, columns * sizeof(IntDCPredict));
		_quantRows[i].resize(columns);
		memset(_quantRows[i].data(), 0, columns * sizeof(QuantPredict));
	}

	for (int plane = 0; plane < 4; plane++) {
		const int size = (plane == 1 || plane == 2) ? 16 : 8;
		_blockColumns[plane] = _alignedWidth / size;
		_blockWords[plane].resize(_blockColumns[plane] * (_alignedHeight / size));
	}
	_blockVectors.resize((_alignedWidth / 16) * (_alignedHeight / 16) * 2);
	_frameFlags = 0;
	_mbX = _mbY = 0;

	_currentDC = _dcRows[0].data();
	_prevDC = _dcRows[1].data();
	_currentMV = _mvRows[0].data();
	_prevMV = _mvRows[1].data();
	_currentIntDC = _intDCRows[0].data();
	_prevIntDC = _intDCRows[1].data();
	_currentQuant = _quantRows[0].data();
	_prevQuant = _quantRows[1].data();

	_quantHuffman = new Huffman(0, 16, kQuantCodes, kQuantLengths);
	for (int i = 0; i < 2; i++) {
		_acValueHuffman[i] = new Huffman(0, 13, kACValueCodes[i], kACValueLengths[i]);
		_acSkipHuffman[i] = new Huffman(0, 14, kACSkipCodes[i], kACSkipLengths[i]);
		_gACSkipHuffman[i] = new Huffman(0, 14, kACSkipCodesG[i], kACSkipLengthsG[i]);
	}
	_gMotionHuffman = new Huffman(0, 16, kMotionCodesG, kMotionLengthsG);

	memset(_block, 0, sizeof(_block));
	memset(_intBlock, 0, sizeof(_intBlock));

	reset();
}

Bink2Decoder::Bink2VideoTrack::~Bink2VideoTrack() {
	if (_surface) {
		_surface->free();
		delete _surface;
	}

	for (int frame = 0; frame < 2; frame++)
		for (int plane = 0; plane < 4; plane++)
			delete[] _buffers[frame][plane];

	delete _quantHuffman;
	for (int i = 0; i < 2; i++) {
		delete _acValueHuffman[i];
		delete _acSkipHuffman[i];
		delete _gACSkipHuffman[i];
	}
	delete _gMotionHuffman;
}

bool Bink2Decoder::Bink2VideoTrack::setOutputPixelFormat(const Graphics::PixelFormat &format) {
	if (format.bytesPerPixel != 2 && format.bytesPerPixel != 4)
		return false;

	_pixelFormat = format;
	return true;
}

bool Bink2Decoder::Bink2VideoTrack::rewind() {
	if (!VideoTrack::rewind())
		return false;

	_curFrameNumber = -1;
	reset();
	return true;
}

void Bink2Decoder::Bink2VideoTrack::decodePacket(Common::SeekableReadStream &stream, uint32 size, bool isKeyFrame) {
	decodeFrame(stream, size, isKeyFrame);
	convertFrame();
	_curFrameNumber++;
}

void Bink2Decoder::Bink2VideoTrack::convertFrame() {
	if (!_surface) {
		// Even-sized, as the conversion goes two pixels at a time
		_surface = new Graphics::Surface();
		_surface->create((_width + 1) & ~1, (_height + 1) & ~1, _pixelFormat);
		_surface->w = _width;
		_surface->h = _height;
	}

	// Bink 2 keeps the full range of luminance: the pictures the games draw
	// over the frames only match them that way
	const int surfaceWidth = (_width + 1) & ~1;
	const int surfaceHeight = (_height + 1) & ~1;
	if (_hasAlpha) {
		YUVToRGBMan.convert420Alpha(_surface, Graphics::YUVToRGBManager::kScaleFull, _planes[_curFrame][0], _planes[_curFrame][1], _planes[_curFrame][2], _planes[_curFrame][3],
				surfaceWidth, surfaceHeight, _pitches[0], _pitches[1]);
	} else {
		YUVToRGBMan.convert420(_surface, Graphics::YUVToRGBManager::kScaleFull, _planes[_curFrame][0], _planes[_curFrame][1], _planes[_curFrame][2],
				surfaceWidth, surfaceHeight, _pitches[0], _pitches[1]);
	}
}

void Bink2Decoder::Bink2VideoTrack::reset() {
	static constexpr byte kBlack[4] = { 0, 128, 128, 255 };

	for (int frame = 0; frame < 2; frame++)
		for (int plane = 0; plane < _numPlanes; plane++)
			memset(_buffers[frame][plane], kBlack[plane], _planeSizes[plane]);
}

void Bink2Decoder::Bink2VideoTrack::decodeFrame(Common::SeekableReadStream &stream, uint32 size, bool isKeyFrame) {
	// The bit streams read whole 32-bit words, so pad the data with zeros
	_packet.resize(size + 8);
	memset(_packet.data() + size, 0, 8);
	if (stream.read(_packet.data(), size) != size) {
		warning("Bink2Decoder: Could not read a frame of %u bytes", size);
		return;
	}

	// Revision 'f' always has two slices, 'g' one for videos below 128 rows
	const int numSlices = (_revision == 'g' && _alignedHeight < 128) ? 1 : 2;
	const uint32 headerSize = 8;

	// The frame flags, then the offset of the second slice, 0 without one
	if (size < headerSize) {
		warning("Bink2Decoder: Frame of %u bytes is too small", size);
		return;
	}

	const uint32 frameFlags  = READ_LE_UINT32(_packet.data());
	const uint32 sliceOffset = numSlices == 2 ? READ_LE_UINT32(_packet.data() + 4) : size;
	if (sliceOffset < headerSize || sliceOffset > size) {
		warning("Bink2Decoder: Bad slice offset %u in a frame of %u bytes", sliceOffset, size);
		return;
	}

	int sliceHeight = (int)_alignedHeight;
	if (numSlices == 2)
		sliceHeight = (int)(_revision == 'g' ? (_height / 2) & ~31 : (_height / 2 + 16) & ~31);
	_frameFlags = frameFlags;

	Common::BitStreamMemoryStream stream1(_packet.data() + headerSize, (sliceOffset - headerSize + 3) & ~3);
	BitStream bits1(stream1);

	// Flags of the rows and columns of blocks, which are not needed
	if (frameFlags & kFrameFlagBlockFlags) {
		if (!(frameFlags & kFrameFlagNoRowFlags))
			skipBlockFlags(bits1, (((_height + 15) & ~15) >> 3) - 1);
		if (!(frameFlags & kFrameFlagNoColFlags))
			skipBlockFlags(bits1, (((_width + 15) & ~15) >> 3) - 1);
	}

	if (_revision == 'g') {
		decodeSliceG(bits1, isKeyFrame, 0, sliceHeight);
		if (numSlices == 2) {
			Common::BitStreamMemoryStream stream2(_packet.data() + sliceOffset, (size - sliceOffset + 3) & ~3);
			BitStream bits2(stream2);
			decodeSliceG(bits2, isKeyFrame, sliceHeight, _alignedHeight);
		}

		deblock();
	} else {
		decodeSlice(bits1, isKeyFrame, 0, sliceHeight);

		Common::BitStreamMemoryStream stream2(_packet.data() + sliceOffset, (size - sliceOffset + 3) & ~3);
		BitStream bits2(stream2);
		decodeSlice(bits2, isKeyFrame, sliceHeight, _alignedHeight);

		deblock();
	}

	_curFrame ^= 1;
}

// Reads past one flag per row or column of 8x8 blocks: stored, or in runs
void Bink2Decoder::Bink2VideoTrack::skipBlockFlags(BitStream &bits, int size) {
	if (!bits.getBit()) {
		bits.skip(size);
		return;
	}

	int flagsLeft = size;
	int mode = 0;

	while (flagsLeft > 0) {
		if (!bits.getBit()) {
			// The flag, unless it flips, and four more stored
			if (mode != 3)
				bits.getBit();
			mode = 2;

			if (flagsLeft < 5) {
				bits.skip(flagsLeft - 1);
				flagsLeft = 0;
			} else {
				bits.skip(4);
				flagsLeft -= 5;
			}
		} else {
			// A run of the flag, unless it flips
			const int lengthBits = flagsLeft < 4 ? 2 : (flagsLeft < 16 ? 4 : 5);
			int count = lengthBits + 1;

			if (mode != 3) {
				count++;
				bits.getBit();
			}

			count = MIN(count, flagsLeft);
			flagsLeft -= count;

			if (flagsLeft > 0) {
				const int extra = bits.getBits(lengthBits);
				flagsLeft -= extra;
				mode = extra == (1 << lengthBits) - 1 ? 1 : 3;
			}
		}
	}
}

void Bink2Decoder::Bink2VideoTrack::decodeSlice(BitStream &bits, bool isKeyFrame, int start, int end) {
	const int dstFrame = _curFrame ^ 1;

	byte *dst[4];
	const byte *src[4];
	for (int plane = 0; plane < 4; plane++) {
		const bool isChroma = plane == 1 || plane == 2;

		if (plane < _numPlanes) {
			dst[plane] = _planes[dstFrame][plane] + (isChroma ? start / 2 : start) * _pitches[plane];
			src[plane] = _planes[_curFrame][plane];
		} else {
			dst[plane] = nullptr;
			src[plane] = nullptr;
		}
	}

	memset(_prevMV, 0, _mvRows[0].size() * sizeof(MotionVectors));

	for (int y = start; y < end; y += 32) {
		// Y, the two chroma planes in the order they are coded, A
		uint32 intraCBP[4] = { 0, 0, 0, 0 };
		uint32 interCBP[4] = { 0, 0, 0, 0 };
		int intraQ[4] = { 8, 8, 8, 8 };
		int interQ[4] = { 8, 8, 8, 8 };

		memset(_currentMV, 0, _mvRows[0].size() * sizeof(MotionVectors));

		for (int x = 0; x < (int)_width; x += 32) {
			MotionVectors mv;
			memset(&mv, 0, sizeof(mv));

			const int type = isKeyFrame ? (int)kBlockIntra : (int)bits.getBits<2>();

			_mbPos = x / 32;
			_mbX = x;
			_mbY = y;
			_currentDC[_mbPos].blockType = type;

			int flags = 0;
			if (y == start)
				flags |= kFlagTop;
			if (x == 0)
				flags |= kFlagLeft;

			bool isValid = true;

			switch (type) {
			case kBlockIntra:
				// The DCs of the neighbors that were not intra come from their pixels
				if (!(flags & (kFlagTop | kFlagLeft)) && _prevDC[_mbPos - 1].blockType != kBlockIntra)
					averageNeighbor(x - 32, -32, dst, _prevDC[_mbPos - 1]);
				if (!(flags & kFlagLeft) && _currentDC[_mbPos - 1].blockType != kBlockIntra)
					averageNeighbor(x - 32, 0, dst, _currentDC[_mbPos - 1]);
				if ((flags & kFlagLeft) && !(flags & kFlagTop) && _prevDC[_mbPos + 1].blockType != kBlockIntra)
					averageNeighbor(x + 32, -32, dst, _prevDC[_mbPos + 1]);
				if (!(flags & kFlagTop) && _prevDC[_mbPos].blockType != kBlockIntra)
					averageNeighbor(x, -32, dst, _prevDC[_mbPos]);

				predictMotionVectors(flags, mv);

				_comp = 0;
				isValid = decodeLuma(bits, true, intraCBP[0], intraQ[0], dst[0] + x, _pitches[0], flags);
				_comp = 1;
				isValid = isValid && decodeChroma(bits, true, intraCBP[1], intraQ[1], dst[2] + x / 2, _pitches[2], flags);
				_comp = 2;
				isValid = isValid && decodeChroma(bits, true, intraCBP[2], intraQ[2], dst[1] + x / 2, _pitches[1], flags);
				if (_hasAlpha) {
					_comp = 3;
					isValid = isValid && decodeLuma(bits, true, intraCBP[3], intraQ[3], dst[3] + x, _pitches[3], flags);
				}
				break;
			case kBlockSkip:
				copyBlock(dst[0] + x, src[0] + x + y * _pitches[0], _pitches[0], 32, 32);
				copyBlock(dst[1] + x / 2, src[1] + x / 2 + y / 2 * _pitches[1], _pitches[1], 16, 16);
				copyBlock(dst[2] + x / 2, src[2] + x / 2 + y / 2 * _pitches[2], _pitches[2], 16, 16);
				if (_hasAlpha)
					copyBlock(dst[3] + x, src[3] + x + y * _pitches[3], _pitches[3], 32, 32);
				break;
			default:
				// Motion, or motion with a residue
				decodeMotionVectors(bits, flags, mv);
				predictMotionVectors(flags, mv);

				compensateLuma(x, y, dst[0], src[0], _pitches[0]);
				compensateChroma(x / 2, y / 2, dst[2], src[2], _pitches[2]);
				compensateChroma(x / 2, y / 2, dst[1], src[1], _pitches[1]);
				if (_hasAlpha)
					compensateLuma(x, y, dst[3], src[3], _pitches[3]);

				if (type == kBlockResidue) {
					_comp = 0;
					isValid = decodeLuma(bits, false, interCBP[0], interQ[0], dst[0] + x, _pitches[0], flags);
					_comp = 1;
					isValid = isValid && decodeChroma(bits, false, interCBP[1], interQ[1], dst[2] + x / 2, _pitches[2], flags);
					_comp = 2;
					isValid = isValid && decodeChroma(bits, false, interCBP[2], interQ[2], dst[1] + x / 2, _pitches[1], flags);
					if (_hasAlpha) {
						_comp = 3;
						isValid = isValid && decodeLuma(bits, false, interCBP[3], interQ[3], dst[3] + x, _pitches[3], flags);
					}
				}
				break;
			}

			storeDeblockInfo(type);

			if (!isValid) {
				warning("Bink2Decoder: Bad quantizer at macroblock %d, %d", x, y);
				return;
			}
		}

		for (int plane = 0; plane < _numPlanes; plane++) {
			const bool isChroma = plane == 1 || plane == 2;
			dst[plane] += (isChroma ? 16 : 32) * _pitches[plane];
		}

		SWAP(_currentMV, _prevMV);
		SWAP(_currentDC, _prevDC);
	}
}

int Bink2Decoder::Bink2VideoTrack::decodeDeltaQ(BitStream &bits) {
	int dq = _quantHuffman->getSymbol(bits);

	if (dq && bits.getBit())
		dq = -dq;

	return dq;
}

// Which 8x8 blocks have coefficients (bits 0-15) and which of those use the
// second set of codes (bits 16-31), from the ones of the previous macroblock
uint32 Bink2Decoder::Bink2VideoTrack::decodeLumaCBP(BitStream &bits, uint32 prevCBP) {
	uint32 low;

	if (bits.getBit()) {
		if (bits.getBit())
			return prevCBP;
		low = prevCBP & 0xFFFF;
	} else {
		low = 0;
		uint32 nibble = (prevCBP >> 4) & 0xF;
		for (int i = 0; i < 4; i++) {
			if (!bits.getBit())
				nibble = bits.getBits<4>();
			low |= nibble << (i * 4);
		}
	}

	uint32 high = 0;
	uint32 cbp = low;
	uint32 nibble = (prevCBP >> 20) & 0xF;
	for (int i = 0; i < 4; i++) {
		const int count = countBits4(cbp);

		if (count == 0) {
			nibble = 0;
		} else if (count == 1 || !bits.getBit()) {
			nibble = 0;
			for (uint32 j = 1; j < 16; j <<= 1) {
				if ((j & cbp) && bits.getBit())
					nibble |= j;
			}
		}

		nibble &= cbp;
		high = (high >> 4) | (nibble << 28);
		cbp >>= 4;
	}

	return high | low;
}

uint32 Bink2Decoder::Bink2VideoTrack::decodeChromaCBP(BitStream &bits, uint32 prevCBP) {
	uint32 low;

	if (bits.getBit()) {
		if (bits.getBit())
			return prevCBP;
		low = prevCBP & 0xF;
	} else {
		low = bits.getBits<4>();
	}

	uint32 high = 0;
	const int count = countBits4(low);
	if (count) {
		if (count != 1 && bits.getBit())
			return low | (((prevCBP >> 16) & low) << 16);

		for (uint32 j = 1; j < 16; j <<= 1) {
			if ((j & low) && bits.getBit())
				high |= j;
		}
	}

	return low | (high << 16);
}

// The DCs of the 16 luma or 4 chroma blocks of the macroblock, from the
// differences read and the blocks to the left and above
void Bink2Decoder::Bink2VideoTrack::predictDC(bool isLuma, float minDC, float maxDC, int flags, float base, const float tdc[16]) {
	const float *leftTop = _prevDC[MAX(_mbPos - 1, 0)].dc[_comp];
	const float *top     = _prevDC[_mbPos].dc[_comp];
	const float *left    = _currentDC[MAX(_mbPos - 1, 0)].dc[_comp];
	float *dc = _currentDC[_mbPos].dc[_comp];

	const bool hasTop  = !(flags & kFlagTop);
	const bool hasLeft = !(flags & kFlagLeft);

	float first;
	if (hasTop && hasLeft)
		first = isLuma ? predictMedian(leftTop[15], left[5], top[10]) : predictMedian(leftTop[3], left[1], top[2]);
	else if (hasTop)
		first = isLuma ? predictMedian2(top[10], top[11]) : predictMedian2(top[2], top[3]);
	else if (hasLeft)
		first = isLuma ? predictMedian2(left[5], left[7]) : predictMedian2(left[1], left[3]);
	else
		first = base;

	dc[0] = CLIP(first + tdc[0], minDC, maxDC);

	if (!isLuma) {
		dc[1] = CLIP((hasTop ? predictMedian(top[2], dc[0], top[3]) : dc[0]) + tdc[1], minDC, maxDC);
		dc[2] = CLIP((hasLeft ? predictMedian(left[1], left[3], dc[0]) : predictMedian2(dc[0], dc[1])) + tdc[2], minDC, maxDC);
		dc[3] = CLIP(predictMedian(dc[0], dc[2], dc[1]) + tdc[3], minDC, maxDC);
		return;
	}

	dc[1]  = CLIP((hasTop ? predictMedian(top[10], dc[0], top[11]) : dc[0]) + tdc[1], minDC, maxDC);
	dc[2]  = CLIP((hasLeft ? predictMedian(left[5], left[7], dc[0]) : predictMedian2(dc[0], dc[1])) + tdc[2], minDC, maxDC);
	dc[3]  = CLIP(predictMedian(dc[0], dc[2], dc[1]) + tdc[3], minDC, maxDC);
	dc[4]  = CLIP((hasTop ? predictMedian(top[11], dc[1], top[14]) : predictMedian2(dc[1], dc[3])) + tdc[4], minDC, maxDC);
	dc[5]  = CLIP((hasTop ? predictMedian(top[14], dc[4], top[15]) : dc[4]) + tdc[5], minDC, maxDC);
	dc[6]  = CLIP(predictMedian(dc[1], dc[3], dc[4]) + tdc[6], minDC, maxDC);
	dc[7]  = CLIP(predictMedian(dc[4], dc[6], dc[5]) + tdc[7], minDC, maxDC);
	dc[8]  = CLIP((hasLeft ? predictMedian(left[7], left[13], dc[2]) : predictMedian2(dc[2], dc[3])) + tdc[8], minDC, maxDC);
	dc[9]  = CLIP(predictMedian(dc[2], dc[8], dc[3]) + tdc[9], minDC, maxDC);
	dc[10] = CLIP((hasLeft ? predictMedian(left[13], left[15], dc[8]) : predictMedian2(dc[8], dc[9])) + tdc[10], minDC, maxDC);
	dc[11] = CLIP(predictMedian(dc[8], dc[10], dc[9]) + tdc[11], minDC, maxDC);
	dc[12] = CLIP(predictMedian(dc[3], dc[9], dc[6]) + tdc[12], minDC, maxDC);
	dc[13] = CLIP(predictMedian(dc[6], dc[12], dc[7]) + tdc[13], minDC, maxDC);
	dc[14] = CLIP(predictMedian(dc[9], dc[11], dc[12]) + tdc[14], minDC, maxDC);
	dc[15] = CLIP(predictMedian(dc[12], dc[14], dc[13]) + tdc[15], minDC, maxDC);
}

void Bink2Decoder::Bink2VideoTrack::decodeDC(BitStream &bits, bool isLuma, int q, int minDC, int maxDC, int flags) {
	const int count = isLuma ? 16 : 4;
	float tdc[16] = {};

	int dcBits = bits.getBits<3>();
	if (dcBits == 7)
		dcBits += bits.getBits<2>();

	if (dcBits) {
		for (int j = 0; j < count; j += 4) {
			for (int i = 0; i < 4; i++)
				tdc[i + j] = (float)bits.getBits(dcBits);

			for (int i = 0; i < 4; i++) {
				if (tdc[i + j] != 0.0f && bits.getBit())
					tdc[i + j] = -tdc[i + j];
			}
		}
	}

	// The first intra macroblock of a slice has more bits for its first DC,
	// which starts from 1024 when there are any
	float base = 0.0f;
	if ((flags & kFlagTop) && (flags & kFlagLeft) && minDC >= 0) {
		const int usedBits = kDCQuantBits[q] + dcBits;

		if (usedBits < 10) {
			base = 1024.0f;
			int value = bits.getBits(10 - usedBits);

			if (value) {
				value <<= dcBits;
				if (bits.getBit())
					value = -value;
			}

			tdc[0] += value;
		}
	}

	for (int i = 0; i < count; i++)
		tdc[i] *= kDCQuant[q];

	predictDC(isLuma, minDC, maxDC, flags, base, tdc);
}

// The AC coefficients of four 8x8 blocks, for the ones set in the low bits of cbp
void Bink2Decoder::Bink2VideoTrack::decodeAC(BitStream &bits, const byte *scan, uint32 cbp, float q, const float *qmat) {
	for (int i = 0; i < 4; i++, cbp >>= 1) {
		memset(_block[i], 0, sizeof(_block[i]));
		_coefCounts[i] = 0;

		if (!(cbp & 1))
			continue;

		const int table = (cbp & 0x10000) ? 1 : 0;
		int next = 0;
		int index = 1;

		while (index < 64) {
			int value = _acValueHuffman[table]->getSymbol(bits);

			if (value) {
				if (value >= 4) {
					const int extraBits = value - 3;
					value = bits.getBits(extraBits) + (1 << extraBits) + 2;
				}

				if (bits.getBit())
					value = -value;
				_coefCounts[i]++;
			}

			_block[i][scan[index]] = value * (qmat[scan[index]] * q);

			if (index > 62)
				break;

			index++;
			next--;

			if (next < 1) {
				const int skip = _acSkipHuffman[table]->getSymbol(bits);
				int run = kACSkips[skip];

				next = kACNextSkips[skip];
				if (run == 11)
					run = bits.getBits<6>();

				index += run;
			}
		}
	}
}

bool Bink2Decoder::Bink2VideoTrack::decodeLuma(BitStream &bits, bool isIntra, uint32 &prevCBP, int &prevQ, byte *dst, int pitch, int flags) {
	const uint32 cbp = prevCBP = decodeLumaCBP(bits, prevCBP);

	const int q = prevQ + decodeDeltaQ(bits);
	if (q < 0 || q >= 16)
		return false;
	prevQ = q;

	// The differences of a residue are not predicted from the neighbors
	if (isIntra)
		decodeDC(bits, true, q, 0, 2047, flags);
	else
		decodeDC(bits, true, q, -1023, 1023, kFlagTop | kFlagLeft);

	const float *dc = _currentDC[_mbPos].dc[_comp];

	for (int i = 0; i < 4; i++) {
		decodeAC(bits, kLumaScan, cbp >> (4 * i), kACQuant[q], isIntra ? kLumaIntraQmat : kLumaInterQmat);

		const bool isFlat = !isIntra && !(_coefCounts[0] | _coefCounts[1] | _coefCounts[2] | _coefCounts[3]);
		for (int j = 0; j < 4; j++) {
			const int pos = kLumaBlockPos[i * 4 + j];
			byte *blockDst = dst + (pos & 3) * 8 + (pos >> 2) * 8 * pitch;

			if (isIntra)
				setBlockWord(_comp == 0 ? 0 : 3, _mbX / 8 + (pos & 3), _mbY / 8 + (pos >> 2), makeBlockWord(_coefCounts[j], dc[i * 4 + j]));
			if (isFlat) {
				addFlat(blockDst, pitch, dc[i * 4 + j]);
				continue;
			}
			idct(blockDst, pitch, _block[j], dc[i * 4 + j], !isIntra);
		}
	}

	return true;
}

bool Bink2Decoder::Bink2VideoTrack::decodeChroma(BitStream &bits, bool isIntra, uint32 &prevCBP, int &prevQ, byte *dst, int pitch, int flags) {
	const uint32 cbp = prevCBP = decodeChromaCBP(bits, prevCBP);

	const int q = prevQ + decodeDeltaQ(bits);
	if (q < 0 || q >= 16)
		return false;
	prevQ = q;

	if (isIntra)
		decodeDC(bits, false, q, 0, 2047, flags);
	else
		decodeDC(bits, false, q, -1023, 1023, kFlagTop | kFlagLeft);

	decodeAC(bits, kChromaScan, cbp, kACQuant[q], kChromaQmat);

	const float *dc = _currentDC[_mbPos].dc[_comp];

	const bool isFlat = !isIntra && !(_coefCounts[0] | _coefCounts[1] | _coefCounts[2] | _coefCounts[3]);
	for (int j = 0; j < 4; j++) {
		byte *blockDst = dst + (j & 1) * 8 + (j >> 1) * 8 * pitch;
		if (isIntra)
			setBlockWord(_comp == 1 ? 2 : 1, _mbX / 16 + (j & 1), _mbY / 16 + (j >> 1), makeBlockWord(_coefCounts[j], dc[j]));
		if (isFlat) {
			addFlat(blockDst, pitch, dc[j]);
			continue;
		}
		idct(blockDst, pitch, _block[j], dc[j], !isIntra);
	}

	return true;
}

// The differences of the four motion vectors, in half luma pixels
void Bink2Decoder::Bink2VideoTrack::decodeMotionVectors(BitStream &bits, int flags, MotionVectors &mv) {
	for (int i = 0; i < 2; i++) {
		int valueBits = bits.getBits<3>();
		if (valueBits == 7)
			valueBits += bits.getBits<2>();

		if (valueBits) {
			for (int j = 0; j < 4; j++)
				mv.v[j][i] = bits.getBits(valueBits);

			for (int j = 0; j < 4; j++) {
				if (mv.v[j][i] && bits.getBit())
					mv.v[j][i] = -mv.v[j][i];
			}
		}

		// The first macroblock of a slice has an offset for all four
		int offset = 0;
		if ((flags & kFlagTop) && (flags & kFlagLeft)) {
			offset = bits.getBits<5>() * 16;
			if (offset && bits.getBit())
				offset = -offset;
		}

		for (int j = 0; j < 4; j++)
			mv.v[j][i] += offset;
	}
}

void Bink2Decoder::Bink2VideoTrack::predictMotionVectors(int flags, const MotionVectors &mv) {
	MotionVectors &cur = _currentMV[_mbPos];
	const MotionVectors &left    = _currentMV[MAX(_mbPos - 1, 0)];
	const MotionVectors &leftTop = _prevMV[MAX(_mbPos - 1, 0)];
	const MotionVectors &top     = _prevMV[_mbPos];

	for (int i = 0; i < 2; i++) {
		if (!(flags & kFlagTop)) {
			if (flags & kFlagLeft)
				cur.v[0][i] = mv.v[0][i] + median(top.v[0][i], top.v[2][i], top.v[3][i]);
			else
				cur.v[0][i] = mv.v[0][i] + median(leftTop.v[3][i], top.v[2][i], left.v[1][i]);

			cur.v[1][i] = mv.v[1][i] + median(top.v[2][i], top.v[3][i], cur.v[0][i]);
			cur.v[2][i] = mv.v[2][i] + median(top.v[2][i], cur.v[0][i], cur.v[1][i]);
			cur.v[3][i] = mv.v[3][i] + median(cur.v[0][i], cur.v[1][i], cur.v[2][i]);
		} else if (flags & kFlagLeft) {
			for (int j = 0; j < 4; j++)
				cur.v[j][i] = mv.v[j][i];
		} else {
			cur.v[0][i] = mv.v[0][i] + median(left.v[0][i], left.v[1][i], left.v[3][i]);
			cur.v[2][i] = mv.v[2][i] + median(left.v[1][i], left.v[3][i], cur.v[0][i]);
			cur.v[1][i] = mv.v[1][i] + median(left.v[1][i], cur.v[0][i], cur.v[2][i]);
			cur.v[3][i] = mv.v[3][i] + median(cur.v[0][i], cur.v[1][i], cur.v[2][i]);
		}
	}
}

// A source position in half pixels, moved back into the frame with room for
// the filter taps of a half pixel. As in the original, the limits are
// compared without a sign, so a negative position goes to the far end.
static int clampSourcePosition(int pos, int limit) {
	const bool isHalf = (pos & 1) != 0;
	const uint32 low = isHalf ? 4 : 0;
	const uint32 high = (uint32)limit - (isHalf ? 42 : 32);

	if ((uint32)pos - low > high)
		return (uint32)pos < low ? (int)low : (int)high;
	return pos;
}

// Where the 16x16 luma block of a macroblock is taken from, in half pixels
void Bink2Decoder::Bink2VideoTrack::getSourcePosition(int x, int y, int block, int &posX, int &posY) const {
	const MotionVectors &mv = _currentMV[_mbPos];

	posX = mv.v[block][0] + (x + (block & 1) * 16) * 2;
	posY = mv.v[block][1] + (y + (block >> 1) * 16) * 2;

	if (!_skipOutsideBlocks) {
		posX = clampSourcePosition(posX, (int)_width * 2);
		posY = clampSourcePosition(posY, (int)_height * 2);
	}
}

// Four 16x16 blocks. With older decoders, one that starts outside the frame
// is left as it is.
void Bink2Decoder::Bink2VideoTrack::compensateLuma(int x, int y, byte *dst, const byte *src, int pitch) {
	for (int i = 0; i < 4; i++) {
		int posX, posY;
		getSourcePosition(x, y, i, posX, posY);
		const int srcX = posX >> 1;
		const int srcY = posY >> 1;

		if (_skipOutsideBlocks && (srcX < 0 || srcX >= (int)_width || srcY < 0 || srcY >= (int)_height))
			continue;

		const int mode = (posX & 1) | ((posY & 1) << 1);
		lumaBlock(dst + x + (i & 1) * 16 + (i >> 1) * 16 * pitch, src + srcX + srcY * pitch, pitch, mode);
	}
}

// Four 8x8 blocks, with the luma vectors in quarter chroma pixels
void Bink2Decoder::Bink2VideoTrack::compensateChroma(int x, int y, byte *dst, const byte *src, int pitch) {
	for (int i = 0; i < 4; i++) {
		// The luma position in half pixels is the chroma one in quarters
		int posX, posY;
		getSourcePosition(x * 2, y * 2, i, posX, posY);
		const int srcX = posX >> 2;
		const int srcY = posY >> 2;

		if (_skipOutsideBlocks && (srcX < 0 || srcX >= (int)_width / 2 || srcY < 0 || srcY >= (int)_height / 2))
			continue;

		const int mode = (posX & 3) | ((posY & 3) << 2);
		chromaBlock(dst + x + (i & 1) * 8 + (i >> 1) * 8 * pitch, src + srcX + srcY * pitch, pitch, mode);
	}
}

void Bink2Decoder::Bink2VideoTrack::averageNeighbor(int x, int y, byte *const dst[4], DCPredict &predict) {
	for (int i = 0; i < 16; i++) {
		const int pos = kLumaBlockPos[i];
		const int offset = x + (pos & 3) * 8 + (y + (pos >> 2) * 8) * _pitches[0];

		predict.dc[0][i] = (float)averageBlock(dst[0] + offset, _pitches[0]);
		if (_hasAlpha)
			predict.dc[3][i] = (float)averageBlock(dst[3] + offset, _pitches[3]);
	}

	// The chroma planes in the order they are coded
	for (int i = 0; i < 4; i++) {
		const int chromaX = x / 2 + (i & 1) * 8;
		const int chromaY = y / 2 + (i >> 1) * 8;

		predict.dc[1][i] = (float)averageBlock(dst[2] + chromaX + chromaY * _pitches[2], _pitches[2]);
		predict.dc[2][i] = (float)averageBlock(dst[1] + chromaX + chromaY * _pitches[1], _pitches[1]);
	}
}

// Revision 'g'

// Counts the bits that differ from stop, up to len of them
static int getUnary(Common::BitStreamMemory32LELSB &bits, int stop, int len) {
	int count = 0;
	while (count < len && (int)bits.getBit() != stop)
		count++;

	return count;
}

static inline int predictMedianInt(int a, int b, int c) {
	return MIN(MAX(c + b - a, MIN(a, MIN(b, c))), MAX(a, MAX(b, c)));
}

static inline int predictMedian2Int(int a, int b) {
	return MIN(MAX(a, b), MAX(MIN(a, b), 2 * a - b));
}

static void idct1DG(int16 *block, int step, int shift) {
	const int t00 = block[3 * step] + block[5 * step];
	const int t01 = block[3 * step] - block[5 * step];
	const int t02 = block[2 * step] + (block[2 * step] >> 2) + (block[6 * step] >> 1);
	const int t03 = (block[2 * step] >> 1) - (block[6 * step] + (block[6 * step] >> 2));
	const int t0 = block[0 * step] + block[4 * step] + t02;
	const int t1 = block[0 * step] + block[4 * step] - t02;
	const int t2 = block[0 * step] - block[4 * step];
	const int t3 = block[1 * step] + t00;
	const int t4 = block[1 * step] - t00;
	const int t5 = t01 + block[7 * step];
	const int t6 = t01 - block[7 * step];
	const int t7 = t4 + (t6 - (t6 >> 2) - (t6 >> 4));
	const int t8 = (t4 - (t4 >> 2) - (t4 >> 4)) - t6;
	const int t9 = (t3 + (t3 >> 2) - (t3 >> 4)) + (t5 >> 2);
	const int t10 = (t3 >> 2) - (t5 + (t5 >> 2) - (t5 >> 4));
	const int t11 = t2 + t03;
	const int t12 = t2 - t03;

	block[0 * step] = (int16)((t0 + t9) >> shift);
	block[1 * step] = (int16)((t11 + t7) >> shift);
	block[2 * step] = (int16)((t12 + t8) >> shift);
	block[3 * step] = (int16)((t1 + t10) >> shift);
	block[4 * step] = (int16)((t1 - t10) >> shift);
	block[5 * step] = (int16)((t12 - t8) >> shift);
	block[6 * step] = (int16)((t11 - t7) >> shift);
	block[7 * step] = (int16)((t0 - t9) >> shift);
}

// The DC goes in with a bias of 4, rounding included. The block comes out
// transposed.
static void idctG(int16 *block, int dc) {
	block[0] = (int16)(dc * 8 + 32);

	for (int i = 0; i < 8; i++)
		idct1DG(block + i, 8, 0);
	for (int i = 0; i < 8; i++)
		idct1DG(block + i * 8, 1, 6);
}

static void putBlockG(byte *dst, int pitch, const int16 *block) {
	for (int j = 0; j < 8; j++) {
		for (int i = 0; i < 8; i++)
			dst[i] = clipPixel(block[i * 8 + j]);
		dst += pitch;
	}
}

static void copyResidueG(int16 *dst, int pitch, const int16 *block) {
	for (int j = 0; j < 8; j++) {
		for (int i = 0; i < 8; i++)
			dst[i] = block[i * 8 + j];
		dst += pitch;
	}
}

// The edges between the blocks of a residue are smoothed before it is added,
// on the side of each block with few AC coefficients: the nearest of its
// differences when it has fewer than 8, the two nearest with fewer than 4.
// Not between two blocks with only DCs that are close.
static void filterResidueEdgeG(int16 *p, int step, int lineStep, uint16 a, uint16 b) {
	if ((a | b) < 0x2000)
		return;

	if ((a & b) >= 0x6000) {
		const int difference = (int)(a ^ 0x800) - (int)(b ^ 0x800);
		if ((uint32)(difference + 0x20) <= 0x40)
			return;
	}

	const int kindA = a >> 13;
	const int kindB = b >> 13;

	for (int i = 0; i < 8; i++, p += lineStep) {
		const int d = asInt16(p[0] - p[-step]);
		const int outer = asInt16(d + 4) >> 3;
		const int inner = asInt16(d + 2) >> 2;

		if (kindA >= 2)
			p[-2 * step] = asInt16(p[-2 * step] + outer);
		if (kindA >= 1)
			p[-step] = asInt16(p[-step] + inner);
		if (kindB >= 1)
			p[0] = asInt16(p[0] - inner);
		if (kindB >= 2)
			p[step] = asInt16(p[step] - outer);
	}
}

// For blocks of size x size 8x8 blocks, with their words as x + size * y
static void filterResidueG(int16 *residue, int size, const uint16 *words, uint32 frameFlags) {
	const int pitch = size * 8;

	if (!(frameFlags & kFrameFlagNoColFlags))
		for (int y = 0; y < size; y++)
			for (int x = 1; x < size; x++)
				filterResidueEdgeG(residue + y * 8 * pitch + x * 8, 1, pitch, words[y * size + x - 1], words[y * size + x]);

	if (!(frameFlags & kFrameFlagNoRowFlags))
		for (int y = 1; y < size; y++)
			for (int x = 0; x < size; x++)
				filterResidueEdgeG(residue + y * 8 * pitch + x * 8, pitch, 1, words[(y - 1) * size + x], words[y * size + x]);
}

static void addResidueG(byte *dst, int pitch, const int16 *residue, int size) {
	for (int j = 0; j < size; j++) {
		for (int i = 0; i < size; i++)
			dst[i] = clipPixel(dst[i] + residue[j * size + i]);
		dst += pitch;
	}
}

// The kind of a residue block, as its deblocking sees it, and its DC
static uint16 makeResidueWordG(int coefCount, int dc) {
	return (uint16)(((getBlockKind(coefCount) & 3) << 13) | (dc & 0xFFF));
}

void Bink2Decoder::Bink2VideoTrack::decodeSliceG(BitStream &bits, bool isKeyFrame, int start, int end) {
	const int dstFrame = _curFrame ^ 1;
	const uint32 columns = _quantRows[0].size();

	byte *dst[4];
	const byte *src[4];
	for (int plane = 0; plane < 4; plane++) {
		const bool isChroma = plane == 1 || plane == 2;

		if (plane < _numPlanes) {
			dst[plane] = _planes[dstFrame][plane] + (isChroma ? start / 2 : start) * _pitches[plane];
			src[plane] = _planes[_curFrame][plane];
		} else {
			dst[plane] = nullptr;
			src[plane] = nullptr;
		}
	}

	memset(_prevQuant, 0, columns * sizeof(QuantPredict));
	memset(_prevMV, 0, _mvRows[0].size() * sizeof(MotionVectors));

	for (int y = start; y < end; y += 32) {
		// The order in which the kinds of macroblock were last used
		int typeOrder[4] = { kBlockMotion, kBlockResidue, kBlockSkip, kBlockIntra };

		// Y, the two chroma planes in the order they are coded, A
		uint32 intraCBP[4] = { 0, 0, 0, 0 };
		uint32 interCBP[4] = { 0, 0, 0, 0 };

		memset(_currentQuant, 0, columns * sizeof(QuantPredict));
		memset(_currentMV, 0, _mvRows[0].size() * sizeof(MotionVectors));

		for (int x = 0; x < (int)_width; x += 32) {
			const int type = isKeyFrame ? (int)kBlockIntra : decodeBlockTypeG(bits, typeOrder);
			QuantPredict &quant = _currentQuant[x / 32];
			MotionVectors mv;
			memset(&mv, 0, sizeof(mv));
			int mvCount = 0;

			_mbPos = x / 32;
			_mbX = x;
			_mbY = y;
			_currentIntDC[_mbPos].blockType = type;

			int flags = 0;
			if (y == start)
				flags |= kFlagTop;
			if (x == 0)
				flags |= kFlagLeft;

			switch (type) {
			case kBlockIntra:
				// The DCs of the neighbors that were not intra come from their pixels
				if (!(flags & (kFlagTop | kFlagLeft)) && _prevIntDC[_mbPos - 1].blockType != kBlockIntra)
					averageNeighborG(x - 32, -32, dst, _prevIntDC[_mbPos - 1]);
				if (!(flags & kFlagLeft) && _currentIntDC[_mbPos - 1].blockType != kBlockIntra)
					averageNeighborG(x - 32, 0, dst, _currentIntDC[_mbPos - 1]);
				if ((flags & kFlagLeft) && !(flags & kFlagTop) && _prevIntDC[_mbPos + 1].blockType != kBlockIntra)
					averageNeighborG(x + 32, -32, dst, _prevIntDC[_mbPos + 1]);
				if (!(flags & kFlagTop) && _prevIntDC[_mbPos].blockType != kBlockIntra)
					averageNeighborG(x, -32, dst, _prevIntDC[_mbPos]);

				predictMotionVectorsG(flags, mv, mvCount);
				quant.interQ = predictQuant(false, flags);
				quant.intraQ = predictQuant(true, flags) + decodeDeltaQG(bits);
				if (quant.intraQ < 0 || quant.intraQ >= 37) {
					warning("Bink2Decoder: Bad quantizer at macroblock %d, %d", x, y);
					return;
				}

				_comp = 0;
				decodeLumaG(bits, true, intraCBP[0], quant.intraQ, dst[0] + x, _pitches[0], flags);
				_comp = 1;
				decodeChromaG(bits, true, intraCBP[1], quant.intraQ, dst[2] + x / 2, _pitches[2], flags);
				_comp = 2;
				decodeChromaG(bits, true, intraCBP[2], quant.intraQ, dst[1] + x / 2, _pitches[1], flags);
				if (_hasAlpha) {
					_comp = 3;
					decodeLumaG(bits, true, intraCBP[3], quant.intraQ, dst[3] + x, _pitches[3], flags);
				}
				break;
			case kBlockSkip:
				quant.interQ = predictQuant(false, flags);
				quant.intraQ = predictQuant(true, flags);
				copyBlock(dst[0] + x, src[0] + x + y * _pitches[0], _pitches[0], 32, 32);
				copyBlock(dst[1] + x / 2, src[1] + x / 2 + y / 2 * _pitches[1], _pitches[1], 16, 16);
				copyBlock(dst[2] + x / 2, src[2] + x / 2 + y / 2 * _pitches[2], _pitches[2], 16, 16);
				if (_hasAlpha)
					copyBlock(dst[3] + x, src[3] + x + y * _pitches[3], _pitches[3], 32, 32);
				break;
			default:
				// Motion, or motion with a residue
				quant.intraQ = predictQuant(true, flags);
				if (type == kBlockMotion)
					quant.interQ = predictQuant(false, flags);

				mvCount = decodeMotionVectorsG(bits, mv);
				predictMotionVectorsG(flags, mv, mvCount);

				if (type == kBlockResidue) {
					quant.interQ = predictQuant(false, flags) + decodeDeltaQG(bits);
					if (quant.interQ < 0 || quant.interQ >= 37) {
						warning("Bink2Decoder: Bad quantizer at macroblock %d, %d", x, y);
						return;
					}
				}

				compensateLuma(x, y, dst[0], src[0], _pitches[0]);
				compensateChroma(x / 2, y / 2, dst[2], src[2], _pitches[2]);
				compensateChroma(x / 2, y / 2, dst[1], src[1], _pitches[1]);
				if (_hasAlpha)
					compensateLuma(x, y, dst[3], src[3], _pitches[3]);

				if (type == kBlockResidue) {
					_comp = 0;
					decodeLumaG(bits, false, interCBP[0], quant.interQ, dst[0] + x, _pitches[0], flags);

					// The chroma planes can be left out
					if (bits.getBit()) {
						_comp = 1;
						decodeChromaG(bits, false, interCBP[1], quant.interQ, dst[2] + x / 2, _pitches[2], flags);
						_comp = 2;
						decodeChromaG(bits, false, interCBP[2], quant.interQ, dst[1] + x / 2, _pitches[1], flags);
					} else {
						interCBP[1] = 0;
						interCBP[2] = 0;
					}

					if (_hasAlpha) {
						_comp = 3;
						decodeLumaG(bits, false, interCBP[3], quant.interQ, dst[3] + x, _pitches[3], flags);
					}
				}
				break;
			}

			storeDeblockInfo(type);
		}

		for (int plane = 0; plane < _numPlanes; plane++) {
			const bool isChroma = plane == 1 || plane == 2;
			dst[plane] += (isChroma ? 16 : 32) * _pitches[plane];
		}

		SWAP(_currentMV, _prevMV);
		SWAP(_currentQuant, _prevQuant);
		SWAP(_currentIntDC, _prevIntDC);
	}
}

// The kinds of macroblock are coded by how recently they were used
int Bink2Decoder::Bink2VideoTrack::decodeBlockTypeG(BitStream &bits, int lru[4]) {
	int type = lru[0];

	switch (getUnary(bits, 1, 3)) {
	case 1:
		type = lru[1];
		SWAP(lru[0], lru[1]);
		break;
	case 2:
		type = lru[3];
		SWAP(lru[2], lru[3]);
		break;
	case 3:
		type = lru[2];
		SWAP(lru[1], lru[2]);
		break;
	default:
		break;
	}

	return type;
}

int Bink2Decoder::Bink2VideoTrack::decodeDeltaQG(BitStream &bits) {
	int dq = getUnary(bits, 1, 4);

	if (dq == 3)
		dq += bits.getBit();
	else if (dq == 4)
		dq += bits.getBits<5>() + 1;

	if (dq && bits.getBit())
		dq = -dq;

	return dq;
}

// From the macroblocks to the left and above; the first one of a slice starts at 16
int Bink2Decoder::Bink2VideoTrack::predictQuant(bool isIntra, int flags) const {
	const QuantPredict &top = _prevQuant[_mbPos];
	const QuantPredict &left = _currentQuant[MAX(_mbPos - 1, 0)];
	const QuantPredict &leftTop = _prevQuant[MAX(_mbPos - 1, 0)];

	if ((flags & kFlagTop) && (flags & kFlagLeft))
		return 16;
	if (flags & kFlagTop)
		return isIntra ? left.intraQ : left.interQ;
	if (flags & kFlagLeft)
		return isIntra ? top.intraQ : top.interQ;

	return isIntra ? median(top.intraQ, left.intraQ, leftTop.intraQ) : median(top.interQ, left.interQ, leftTop.interQ);
}

// Which 8x8 blocks have coefficients (bits 0-15), and whether they use the
// second set of skip codes (bits 16-31)
uint32 Bink2Decoder::Bink2VideoTrack::decodeLumaCBPG(BitStream &bits, uint32 prevCBP) {
	uint32 ones = 0;
	for (int i = 0; i < 16; i++) {
		if (prevCBP & (1 << i))
			ones++;
	}

	// Mostly set ones are coded inverted
	uint32 mask = 0;
	if (ones > 7) {
		ones = 16 - ones;
		mask = 0xFFFF;
	}

	uint32 cbp = 0;
	if (!bits.getBit()) {
		if (ones < 4) {
			for (int j = 0; j < 16; j += 4) {
				if (!bits.getBit())
					cbp |= bits.getBits<4>() << j;
			}
		} else {
			cbp = bits.getBits<16>();
		}
	}

	cbp ^= mask;
	if (!(_frameFlags & kFrameFlagNoEmptyCBPFlag) || cbp) {
		if (bits.getBit())
			cbp |= cbp << 16;
	}

	return cbp;
}

uint32 Bink2Decoder::Bink2VideoTrack::decodeChromaCBPG(BitStream &bits, uint32 prevCBP) {
	uint32 cbp = (prevCBP & 0xF0000) | kChromaCBPG[prevCBP & 0xF];

	if (!bits.getBit()) {
		cbp = bits.getBits<4>();
		if (bits.getBit())
			cbp |= cbp << 16;
	}

	return cbp;
}

void Bink2Decoder::Bink2VideoTrack::predictDCG(bool isLuma, int minDC, int maxDC, int flags, const int tdc[16]) {
	const int *leftTop = _prevIntDC[MAX(_mbPos - 1, 0)].dc[_comp];
	const int *top     = _prevIntDC[_mbPos].dc[_comp];
	const int *left    = _currentIntDC[MAX(_mbPos - 1, 0)].dc[_comp];
	int *dc = _currentIntDC[_mbPos].dc[_comp];

	const bool hasTop  = !(flags & kFlagTop);
	const bool hasLeft = !(flags & kFlagLeft);

	int first;
	if (hasTop && hasLeft)
		first = isLuma ? predictMedianInt(leftTop[15], left[5], top[10]) : predictMedianInt(leftTop[3], left[1], top[2]);
	else if (hasTop)
		first = isLuma ? predictMedian2Int(top[10], top[11]) : predictMedian2Int(top[2], top[3]);
	else if (hasLeft)
		first = isLuma ? predictMedian2Int(left[5], left[7]) : predictMedian2Int(left[1], left[3]);
	else
		first = minDC < 0 ? 0 : 1024;

	dc[0] = CLIP(first + tdc[0], minDC, maxDC);

	if (!isLuma) {
		dc[1] = CLIP((hasTop ? predictMedianInt(top[2], dc[0], top[3]) : dc[0]) + tdc[1], minDC, maxDC);
		dc[2] = CLIP((hasLeft ? predictMedianInt(left[1], left[3], dc[0]) : predictMedian2Int(dc[0], dc[1])) + tdc[2], minDC, maxDC);
		dc[3] = CLIP(predictMedianInt(dc[0], dc[2], dc[1]) + tdc[3], minDC, maxDC);
		return;
	}

	dc[1]  = CLIP((hasTop ? predictMedianInt(top[10], dc[0], top[11]) : dc[0]) + tdc[1], minDC, maxDC);
	dc[2]  = CLIP((hasLeft ? predictMedianInt(left[5], left[7], dc[0]) : predictMedian2Int(dc[0], dc[1])) + tdc[2], minDC, maxDC);
	dc[3]  = CLIP(predictMedianInt(dc[0], dc[2], dc[1]) + tdc[3], minDC, maxDC);
	dc[4]  = CLIP((hasTop ? predictMedianInt(top[11], dc[1], top[14]) : predictMedian2Int(dc[1], dc[3])) + tdc[4], minDC, maxDC);
	dc[5]  = CLIP((hasTop ? predictMedianInt(top[14], dc[4], top[15]) : dc[4]) + tdc[5], minDC, maxDC);
	dc[6]  = CLIP(predictMedianInt(dc[1], dc[3], dc[4]) + tdc[6], minDC, maxDC);
	dc[7]  = CLIP(predictMedianInt(dc[4], dc[6], dc[5]) + tdc[7], minDC, maxDC);
	dc[8]  = CLIP((hasLeft ? predictMedianInt(left[7], left[13], dc[2]) : predictMedian2Int(dc[2], dc[3])) + tdc[8], minDC, maxDC);
	dc[9]  = CLIP(predictMedianInt(dc[2], dc[8], dc[3]) + tdc[9], minDC, maxDC);
	dc[10] = CLIP((hasLeft ? predictMedianInt(left[13], left[15], dc[8]) : predictMedian2Int(dc[8], dc[9])) + tdc[10], minDC, maxDC);
	dc[11] = CLIP(predictMedianInt(dc[8], dc[10], dc[9]) + tdc[11], minDC, maxDC);
	dc[12] = CLIP(predictMedianInt(dc[3], dc[9], dc[6]) + tdc[12], minDC, maxDC);
	dc[13] = CLIP(predictMedianInt(dc[6], dc[12], dc[7]) + tdc[13], minDC, maxDC);
	dc[14] = CLIP(predictMedianInt(dc[9], dc[11], dc[12]) + tdc[14], minDC, maxDC);
	dc[15] = CLIP(predictMedianInt(dc[12], dc[14], dc[13]) + tdc[15], minDC, maxDC);
}

void Bink2Decoder::Bink2VideoTrack::decodeDCG(BitStream &bits, bool isLuma, int q, int minDC, int maxDC, int flags) {
	const int count = isLuma ? 16 : 4;
	const int step = kDCStepG[MAX(q, 8)];
	int tdc[16] = {};

	if (bits.getBit()) {
		for (int i = 0; i < count; i++) {
			int value = getUnary(bits, 0, 12);

			if (value > 3)
				value = (1 << (value - 3)) + bits.getBits(value - 3) + 2;
			if (value && bits.getBit())
				value = -value;

			tdc[i] = (value * step + 0x200) >> 10;
		}
	}

	predictDCG(isLuma, minDC, maxDC, flags, tdc);
}

// The AC coefficients of four 8x8 blocks, for the ones set in the low bits of cbp
void Bink2Decoder::Bink2VideoTrack::decodeACG(BitStream &bits, uint32 cbp, int q, const uint16 qmat[4][64]) {
	memset(_intBlock, 0, sizeof(_intBlock));
	memset(_coefCounts, 0, sizeof(_coefCounts));

	if (!(cbp & 0xF))
		return;

	Huffman *skipHuffman = _gACSkipHuffman[(cbp & 0xFFFF0000) ? 1 : 0];
	const int scale = 1 << (q >> 2);

	for (int i = 0; i < 4; i++, cbp >>= 1) {
		if (!(cbp & 1))
			continue;

		int next = 0;
		int index = 1;
		while (index < 64) {
			next--;
			if (next < 1) {
				const int skip = skipHuffman->getSymbol(bits);
				int run = kACSkipsG[skip];

				next = kACNextSkips[skip];
				if (run == 11)
					run = bits.getBits<6>();

				index += run;
				if (index >= 64)
					break;
			}

			int value = getUnary(bits, 0, 12) + 1;
			if (value > 3)
				value = bits.getBits(value - 3) + (1 << (value - 3)) + 2;
			if (bits.getBit())
				value = -value;

			const byte pos = kScanG[index];
			_intBlock[i][pos] = (int16)((value * qmat[q & 3][pos] * scale + 64) >> 7);
			_coefCounts[i]++;
			index++;
		}
	}
}

void Bink2Decoder::Bink2VideoTrack::decodeLumaG(BitStream &bits, bool isIntra, uint32 &prevCBP, int q, byte *dst, int pitch, int flags) {
	const uint32 cbp = prevCBP = decodeLumaCBPG(bits, prevCBP);

	// The differences of a residue are not predicted from the neighbors
	if (isIntra)
		decodeDCG(bits, true, q, 0, 2047, flags);
	else
		decodeDCG(bits, true, q, -1023, 1023, kFlagTop | kFlagLeft);

	const int *dc = _currentIntDC[_mbPos].dc[_comp];
	uint16 residueWords[16];

	for (int i = 0; i < 4; i++) {
		decodeACG(bits, cbp >> (4 * i), q, isIntra ? kLumaIntraQmatG : kInterQmatG);

		for (int j = 0; j < 4; j++) {
			const int pos = kLumaBlockPos[i * 4 + j];
			idctG(_intBlock[j], dc[i * 4 + j]);

			if (isIntra) {
				const int plane = _comp == 0 ? 0 : 3;
				const int blockIndex = (_mbY / 8 + (pos >> 2)) * _blockColumns[plane] + _mbX / 8 + (pos & 3);
				_blockWords[plane][blockIndex] = makeBlockWord(_coefCounts[j], (float)dc[i * 4 + j]);
				putBlockG(dst + (pos & 3) * 8 + (pos >> 2) * 8 * pitch, pitch, _intBlock[j]);
			} else {
				residueWords[pos] = makeResidueWordG(_coefCounts[j], dc[i * 4 + j]);
				copyResidueG(_residue + (pos & 3) * 8 + (pos >> 2) * 8 * 32, 32, _intBlock[j]);
			}
		}
	}

	if (!isIntra) {
		filterResidueG(_residue, 4, residueWords, _frameFlags);
		addResidueG(dst, pitch, _residue, 32);
	}
}

void Bink2Decoder::Bink2VideoTrack::decodeChromaG(BitStream &bits, bool isIntra, uint32 &prevCBP, int q, byte *dst, int pitch, int flags) {
	const uint32 cbp = prevCBP = decodeChromaCBPG(bits, prevCBP);

	if (isIntra)
		decodeDCG(bits, false, q, 0, 2047, flags);
	else
		decodeDCG(bits, false, q, -1023, 1023, kFlagTop | kFlagLeft);

	decodeACG(bits, cbp, q, isIntra ? kChromaIntraQmatG : kInterQmatG);

	const int *dc = _currentIntDC[_mbPos].dc[_comp];
	uint16 residueWords[4];

	for (int j = 0; j < 4; j++) {
		idctG(_intBlock[j], dc[j]);

		if (isIntra) {
			const int plane = _comp == 1 ? 2 : 1;
			const int blockIndex = (_mbY / 16 + (j >> 1)) * _blockColumns[plane] + _mbX / 16 + (j & 1);
			_blockWords[plane][blockIndex] = makeBlockWord(_coefCounts[j], (float)dc[j]);
			putBlockG(dst + (j & 1) * 8 + (j >> 1) * 8 * pitch, pitch, _intBlock[j]);
		} else {
			residueWords[j] = makeResidueWordG(_coefCounts[j], dc[j]);
			copyResidueG(_residue + (j & 1) * 8 + (j >> 1) * 8 * 16, 16, _intBlock[j]);
		}
	}

	if (!isIntra) {
		filterResidueG(_residue, 2, residueWords, _frameFlags);
		addResidueG(dst, pitch, _residue, 16);
	}
}

// One motion vector for the whole macroblock, or four. Returns how many.
int Bink2Decoder::Bink2VideoTrack::decodeMotionVectorsG(BitStream &bits, MotionVectors &mv) {
	const int count = bits.getBit() ? 1 : 4;

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < count; j++) {
			int value = _gMotionHuffman->getSymbol(bits);

			if (value == 15) {
				const int valueBits = getUnary(bits, 1, 12) + 4;
				value = bits.getBits(valueBits) + (1 << valueBits) - 1;
				value = (value & 1) ? -(value >> 1) - 1 : value >> 1;
			} else if (value >= 8) {
				value -= 15;
			}

			mv.v[j][i] = value;
		}
	}

	return count;
}

void Bink2Decoder::Bink2VideoTrack::predictMotionVectorsG(int flags, const MotionVectors &mv, int count) {
	MotionVectors &cur = _currentMV[_mbPos];
	const MotionVectors &left    = _currentMV[MAX(_mbPos - 1, 0)];
	const MotionVectors &leftTop = _prevMV[MAX(_mbPos - 1, 0)];
	const MotionVectors &top     = _prevMV[_mbPos];

	for (int i = 0; i < 2; i++) {
		if (count == 1) {
			int value = mv.v[0][i];
			if (!(flags & kFlagTop)) {
				if (flags & kFlagLeft)
					value += median(top.v[0][i], top.v[2][i], top.v[3][i]);
				else
					value += median(leftTop.v[3][i], top.v[2][i], left.v[1][i]);
			} else if (!(flags & kFlagLeft)) {
				value += median(left.v[0][i], left.v[1][i], left.v[3][i]);
			}

			for (int j = 0; j < 4; j++)
				cur.v[j][i] = value;
			continue;
		}

		if (!(flags & kFlagTop)) {
			if (flags & kFlagLeft)
				cur.v[0][i] = mv.v[0][i] + median(top.v[0][i], top.v[2][i], top.v[3][i]);
			else
				cur.v[0][i] = mv.v[0][i] + median(top.v[2][i], leftTop.v[3][i], left.v[1][i]);

			cur.v[1][i] = mv.v[1][i] + median(top.v[2][i], top.v[3][i], cur.v[0][i]);
			if (flags & kFlagLeft)
				cur.v[2][i] = mv.v[2][i] + median(top.v[2][i], cur.v[0][i], cur.v[1][i]);
			else
				cur.v[2][i] = mv.v[2][i] + median(left.v[1][i], left.v[3][i], cur.v[0][i]);
			cur.v[3][i] = mv.v[3][i] + median(cur.v[0][i], cur.v[1][i], cur.v[2][i]);
		} else if (flags & kFlagLeft) {
			cur.v[0][i] = mv.v[0][i];
			cur.v[1][i] = mv.v[1][i] + mv.v[0][i];
			cur.v[2][i] = mv.v[2][i] + mv.v[0][i];
			cur.v[3][i] = mv.v[3][i] + median(cur.v[0][i], cur.v[1][i], cur.v[2][i]);
		} else {
			cur.v[0][i] = mv.v[0][i] + median(left.v[0][i], left.v[1][i], left.v[3][i]);
			cur.v[2][i] = mv.v[2][i] + median(left.v[1][i], left.v[3][i], cur.v[0][i]);
			cur.v[1][i] = mv.v[1][i] + median(left.v[1][i], cur.v[0][i], cur.v[2][i]);
			cur.v[3][i] = mv.v[3][i] + median(cur.v[0][i], cur.v[1][i], cur.v[2][i]);
		}
	}
}

void Bink2Decoder::Bink2VideoTrack::averageNeighborG(int x, int y, byte *const dst[4], IntDCPredict &predict) {
	for (int i = 0; i < 16; i++) {
		const int pos = kLumaBlockPos[i];
		const int offset = x + (pos & 3) * 8 + (y + (pos >> 2) * 8) * _pitches[0];

		predict.dc[0][i] = averageBlock(dst[0] + offset, _pitches[0]);
		if (_hasAlpha)
			predict.dc[3][i] = averageBlock(dst[3] + offset, _pitches[3]);
	}

	// The chroma planes in the order they are coded
	for (int i = 0; i < 4; i++) {
		const int chromaX = x / 2 + (i & 1) * 8;
		const int chromaY = y / 2 + (i >> 1) * 8;

		predict.dc[1][i] = averageBlock(dst[2] + chromaX + chromaY * _pitches[2], _pitches[2]);
		predict.dc[2][i] = averageBlock(dst[1] + chromaX + chromaY * _pitches[1], _pitches[1]);
	}
}

void Bink2Decoder::Bink2VideoTrack::setBlockWord(int plane, int x, int y, uint16 word) {
	_blockWords[plane][y * _blockColumns[plane] + x] = word;
}

// The blocks of a macroblock that is not intra have a word of 0, and the
// vectors of the ones that did not move are 0
void Bink2Decoder::Bink2VideoTrack::storeDeblockInfo(int type) {
	if (type != kBlockIntra) {
		for (int plane = 0; plane < _numPlanes; plane++) {
			const int size = (plane == 1 || plane == 2) ? 2 : 4;
			for (int j = 0; j < size; j++)
				for (int i = 0; i < size; i++)
					setBlockWord(plane, _mbX / (32 / size) + i, _mbY / (32 / size) + j, 0);
		}
	}

	for (int i = 0; i < 4; i++) {
		int *vector = &_blockVectors[((_mbY / 16 + (i >> 1)) * (_alignedWidth / 16) + _mbX / 16 + (i & 1)) * 2];
		const bool isMoved = type == kBlockMotion || type == kBlockResidue;
		vector[0] = isMoved ? _currentMV[_mbPos].v[i][0] : 0;
		vector[1] = isMoved ? _currentMV[_mbPos].v[i][1] : 0;
	}
}

// A block with only a DC is drawn from the DCs around it: each neighbor that
// is such a block too, with a DC closer than the threshold, pulls the pixels
// near it toward its own DC.
void Bink2Decoder::Bink2VideoTrack::blurBlock(int plane, int x, int y, int threshold) {
	const int columns = _blockColumns[plane];
	const int rows = (int)_blockWords[plane].size() / columns;
	const uint16 center = _blockWords[plane][y * columns + x];

	uint16 words[3][3];
	for (int j = 0; j < 3; j++) {
		for (int i = 0; i < 3; i++) {
			const int nx = x + i - 1;
			const int ny = y + j - 1;
			words[j][i] = (nx >= 0 && nx < columns && ny >= 0 && ny < rows) ? _blockWords[plane][ny * columns + nx] : 0;
		}
	}

	if (_revision == 'g') {
		blurBlockG(plane, x, y, words, threshold);
		return;
	}

	// Compared as signed 16-bit words, the kinds of block included
	float values[3][3];
	const int16 c = asInt16(center);
	for (int j = 0; j < 3; j++) {
		for (int i = 0; i < 3; i++) {
			const int16 n = asInt16(words[j][i]);
			const int16 difference = asInt16(MAX(n, c) - MIN(n, c));
			values[j][i] = (threshold > difference) ? (float)(words[j][i] & 0x1FFF) : -1.0f;
		}
	}
	values[1][1] = (float)(center & 0x1FFF);

	// The sides that differ take the center's DC, the corners their side's
	for (int j = 0; j < 3; j += 2)
		if (values[j][1] < 0.0f)
			values[j][1] = values[1][1];
	for (int i = 0; i < 3; i += 2)
		if (values[1][i] < 0.0f)
			values[1][i] = values[1][1];
	for (int j = 0; j < 3; j += 2)
		for (int i = 0; i < 3; i += 2)
			if (values[j][i] < 0.0f)
				values[j][i] = values[1][i];

	const int pitch = _pitches[plane];
	byte *dst = _planes[_curFrame ^ 1][plane] + y * 8 * pitch + x * 8;

	for (int py = 0; py < 8; py++) {
		for (int px = 0; px < 8; px++) {
			// In the original's order
			float sum = values[0][0] * kBlurCorner[py][px];
			sum += values[1][0] * kBlurEdge[px][py];
			sum += values[2][0] * kBlurCorner[7 - py][px];
			sum += values[0][1] * kBlurEdge[py][px];
			sum += values[1][1] * kBlurCenter[py][px];
			sum += values[2][1] * kBlurEdge[7 - py][px];
			sum += values[0][2] * kBlurCorner[py][7 - px];
			sum += values[1][2] * kBlurEdge[7 - px][py];
			sum += values[2][2] * kBlurCorner[7 - py][7 - px];
			sum += kAlmostHalf;
			dst[py * pitch + px] = clipPixel((int)sum);
		}
	}
}

// The weights of revision 'g', in 16-bit fixed point: of the side of a pixel
// toward the edge it is near, and of the other one
static constexpr int kBlurNearWeightsG[4] = { 0x6800, 0x4800, 0x3000, 0x1C00 };
static constexpr int kBlurFarShiftsG[4] = { 0, 7, 5, 4 };

static int blurWeightG(int near, int far, int pos) {
	const int weighted = asInt16((near * kBlurNearWeightsG[pos]) >> 16);
	return pos == 0 ? weighted : weighted + (far >> kBlurFarShiftsG[pos]);
}

// Of the differences to the left (or top) and right (or bottom) DCs
static int blurOffsetG(int first, int second, int pos) {
	return pos < 4 ? blurWeightG(first, second, pos) : blurWeightG(second, first, 7 - pos);
}

// Revision 'g' does the same in 16-bit integers: each row of DCs is spread
// over the 8 columns, then the 3 rows over the 8 rows of pixels, with DCs
// that are 8 times the pixel values
void Bink2Decoder::Bink2VideoTrack::blurBlockG(int plane, int x, int y, const uint16 words[3][3], int threshold) {
	const int16 c = asInt16(words[1][1]);

	int values[3][3];
	for (int j = 0; j < 3; j++) {
		for (int i = 0; i < 3; i++) {
			const int16 difference = MAX(asInt16(c - words[j][i]), asInt16(words[j][i] - c));
			values[j][i] = (threshold > difference) ? (words[j][i] & 0x1FFF) : -1;
		}
	}
	values[1][1] = c & 0x1FFF;

	for (int j = 0; j < 3; j += 2)
		if (values[j][1] < 0)
			values[j][1] = values[1][1];
	for (int i = 0; i < 3; i += 2)
		if (values[1][i] < 0)
			values[1][i] = values[1][1];
	for (int j = 0; j < 3; j += 2)
		for (int i = 0; i < 3; i += 2)
			if (values[j][i] < 0)
				values[j][i] = values[1][i];

	// Rounded by the 4 added before the 3 bits are shifted out
	int rows[3][8];
	for (int j = 0; j < 3; j++) {
		const int left = asInt16(values[j][0] - values[j][1]);
		const int right = asInt16(values[j][2] - values[j][1]);
		for (int i = 0; i < 8; i++)
			rows[j][i] = asInt16(values[j][1] + 4 + blurOffsetG(left, right, i));
	}

	const int pitch = _pitches[plane];
	byte *dst = _planes[_curFrame ^ 1][plane] + y * 8 * pitch + x * 8;

	for (int i = 0; i < 8; i++) {
		const int middle = rows[1][i];
		const int top = asInt16(rows[0][i] - middle);
		const int bottom = asInt16(rows[2][i] - middle);
		for (int j = 0; j < 8; j++)
			dst[j * pitch + i] = clipPixel(asInt16(middle + blurOffsetG(top, bottom, j)) >> 3);
	}
}

// Smooths the edges between the blocks of a plane, the ones between columns
// or between rows, by the kinds of blocks on both sides: blocks with fewer
// AC coefficients get more of their pixels changed. Between two blocks that
// are not intra, it is done when their motion vectors differ.
void Bink2Decoder::Bink2VideoTrack::filterEdges(int plane, bool isVertical) {
	const int columns = _blockColumns[plane];
	const int rows = (int)_blockWords[plane].size() / columns;
	const int pitch = _pitches[plane];
	const int shift = (plane == 1 || plane == 2) ? 0 : 1;
	const int vectorColumns = _alignedWidth / 16;
	byte *base = _planes[_curFrame ^ 1][plane];

	for (int y = isVertical ? 0 : 1; y < rows; y++) {
		for (int x = isVertical ? 1 : 0; x < columns; x++) {
			const int ax = isVertical ? x - 1 : x;
			const int ay = isVertical ? y : y - 1;
			const uint16 a = _blockWords[plane][ay * columns + ax];
			const uint16 b = _blockWords[plane][y * columns + x];

			int code;
			if (((a ^ b) & 0x8000) || (b & 0x8000)) {
				code = ((a >> 13) & 3) | (((b >> 13) & 3) << 2);
			} else {
				const int *va = &_blockVectors[((ay >> shift) * vectorColumns + (ax >> shift)) * 2];
				const int *vb = &_blockVectors[((y >> shift) * vectorColumns + (x >> shift)) * 2];
				if ((uint32)(va[0] - vb[0] + 1) <= 2 && (uint32)(va[1] - vb[1] + 1) <= 2)
					continue;
				code = 0xA;
			}

			byte *p = base + y * 8 * pitch + x * 8;
			for (int i = 0; i < 8; i++) {
				if (isVertical)
					filterEdge(p + i * pitch, 1, code);
				else
					filterEdge(p + i, pitch, code);
			}
		}
	}
}

void Bink2Decoder::Bink2VideoTrack::deblock() {
	const int threshold = (_frameFlags & 0xFF) * 8;

	// The blocks with only a DC, all of them first

	for (int plane = 0; plane < _numPlanes; plane++) {
		const int columns = _blockColumns[plane];
		const int rows = (int)_blockWords[plane].size() / columns;
		for (int y = 0; y < rows; y++)
			for (int x = 0; x < columns; x++)
				if ((_blockWords[plane][y * columns + x] >> 13) == 7)
					blurBlock(plane, x, y, threshold);
	}

	// The alpha plane does not have its edges filtered
	for (int plane = 0; plane < 3; plane++) {
		if (!(_frameFlags & kFrameFlagNoColFlags))
			filterEdges(plane, true);
		if (!(_frameFlags & kFrameFlagNoRowFlags))
			filterEdges(plane, false);
	}
}

} // End of namespace Video

#endif // USE_BINK
