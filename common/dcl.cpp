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

#include "common/dcl.h"
#include "common/debug.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "common/textconsole.h"

namespace Common {

class DecompressorDCL {
public:
	bool unpack(SeekableReadStream *sourceStream, WriteStream *targetStream, uint32 targetSize, bool targetFixedSize);

protected:
	/**
	 * Initialize decompressor.
	 * @param sourceStream	source stream to read from
	 * @param targetStream	target memory stream to write to
	 */
	void init(SeekableReadStream *sourceStream, WriteStream *targetStream, uint32 targetSize, bool targetFixedSize);

	/**
	 * Get a number of bits from _src stream, starting with the least
	 * significant unread bit of the current four byte block.
	 * @param n		number of bits to get
	 * @return n-bits number
	 */
	uint32 getBitsLSB(int n);

	/**
	 * Get one byte from _src stream.
	 * @return byte
	 */
	byte getByteLSB();

	void fetchBitsLSB();

	/**
	 * Write one byte into _dest stream
	 * @param b byte to put
	 */
	void putByte(byte b);

	int huffman_lookup(const int *tree);

	uint32 _dwBits;			///< bits buffer
	byte _nBits;			///< number of unread bits in _dwBits
	uint32 _sourceSize;		///< size of the source stream
	uint32 _targetSize;		///< size of the target stream (if fixed)
	bool _targetFixedSize;  ///< if target stream is fixed size or dynamic size
	uint32 _bytesRead;		///< number of bytes read from _sourceStream
	uint32 _bytesWritten;	///< number of bytes written to _targetStream
	SeekableReadStream *_sourceStream;
	WriteStream *_targetStream;
};

void DecompressorDCL::init(SeekableReadStream *sourceStream, WriteStream *targetStream, uint32 targetSize, bool targetFixedSize) {
	_sourceStream = sourceStream;
	_targetStream = targetStream;
	_sourceSize = sourceStream->size();
	_targetSize = targetSize;
	_targetFixedSize = targetFixedSize;
	_nBits = 0;
	_bytesRead = _bytesWritten = 0;
	_dwBits = 0;
}

void DecompressorDCL::fetchBitsLSB() {
	while (_nBits <= 24) {
		_dwBits |= ((uint32)_sourceStream->readByte()) << _nBits;
		_nBits += 8;
		_bytesRead++;
	}
}

uint32 DecompressorDCL::getBitsLSB(int n) {
	// Fetching more data to buffer if needed
	if (_nBits < n)
		fetchBitsLSB();
	uint32 ret = (_dwBits & ~(~0UL << n));
	_dwBits >>= n;
	_nBits -= n;
	return ret;
}

byte DecompressorDCL::getByteLSB() {
	return getBitsLSB(8);
}

void DecompressorDCL::putByte(byte b) {
	_targetStream->writeByte(b);
	_bytesWritten++;
}

#define HUFFMAN_LEAF 0x40000000
// Branch node
#define BN(pos, left, right)  ((left << 12) | (right)),
// Leaf node
#define LN(pos, value)  ((value) | HUFFMAN_LEAF),

static const int length_tree[] = {
	BN(0, 1, 2)
	BN(1, 3, 4)     BN(2, 5, 6)
	BN(3, 7, 8)     BN(4, 9, 10)    BN(5, 11, 12)  LN(6, 1)
	BN(7, 13, 14)   BN(8, 15, 16)   BN(9, 17, 18)  LN(10, 3)  LN(11, 2)  LN(12, 0)
	BN(13, 19, 20)  BN(14, 21, 22)  BN(15, 23, 24) LN(16, 6)  LN(17, 5)  LN(18, 4)
	BN(19, 25, 26)  BN(20, 27, 28)  LN(21, 10)     LN(22, 9)  LN(23, 8)  LN(24, 7)
	BN(25, 29, 30)  LN(26, 13)      LN(27, 12)     LN(28, 11)
	LN(29, 15)      LN(30, 14)
	0 // We need something witout a comma at the end
};

static const int distance_tree[] = {
	BN(0, 1, 2)
	BN(1, 3, 4)       BN(2, 5, 6)
	//
	BN(3, 7, 8)       BN(4, 9, 10)      BN(5, 11, 12)     LN(6, 0)
	BN(7, 13, 14)     BN(8, 15, 16)     BN(9, 17, 18)     BN(10, 19, 20)
	BN(11, 21, 22)    BN(12, 23, 24)
	//
	BN(13, 25, 26)    BN(14, 27, 28)    BN(15, 29, 30)    BN(16, 31, 32)
	BN(17, 33, 34)    BN(18, 35, 36)    BN(19, 37, 38)    BN(20, 39, 40)
	BN(21, 41, 42)    BN(22, 43, 44)    LN(23, 2)         LN(24, 1)
	//
	BN(25, 45, 46)    BN(26, 47, 48)    BN(27, 49, 50)    BN(28, 51, 52)
	BN(29, 53, 54)    BN(30, 55, 56)    BN(31, 57, 58)    BN(32, 59, 60)
	BN(33, 61, 62)	  BN(34, 63, 64)    BN(35, 65, 66)    BN(36, 67, 68)
	BN(37, 69, 70)    BN(38, 71, 72)    BN(39, 73, 74)    BN(40, 75, 76)
	LN(41, 6)         LN(42, 5)         LN(43, 4)         LN(44, 3)
	//
	BN(45, 77, 78)    BN(46, 79, 80)    BN(47, 81, 82)    BN(48, 83, 84)
	BN(49, 85, 86)    BN(50, 87, 88)    BN(51, 89, 90)    BN(52, 91, 92)
	BN(53, 93, 94)    BN(54, 95, 96)    BN(55, 97, 98)    BN(56, 99, 100)
	BN(57, 101, 102)  BN(58, 103, 104)  BN(59, 105, 106)  BN(60, 107, 108)
	BN(61, 109, 110)  LN(62, 21)        LN(63, 20)        LN(64, 19)
	LN(65, 18)        LN(66, 17)        LN(67, 16)        LN(68, 15)
	LN(69, 14)        LN(70, 13)        LN(71, 12)        LN(72, 11)
	LN(73, 10)        LN(74, 9)         LN(75, 8)         LN(76, 7)
	//
	BN(77, 111, 112)  BN(78, 113, 114)  BN(79, 115, 116)  BN(80, 117, 118)
	BN(81, 119, 120)  BN(82, 121, 122)  BN(83, 123, 124)  BN(84, 125, 126)
	LN(85, 47)        LN(86, 46)        LN(87, 45)        LN(88, 44)
	LN(89, 43)        LN(90, 42)        LN(91, 41)        LN(92, 40)
	LN(93, 39)        LN(94, 38)        LN(95, 37)        LN(96, 36)
	LN(97, 35)        LN(98, 34)        LN(99, 33)        LN(100, 32)
	LN(101, 31)       LN(102, 30)       LN(103, 29)       LN(104, 28)
	LN(105, 27)       LN(106, 26)       LN(107, 25)       LN(108, 24)
	LN(109, 23)       LN(110, 22)       LN(111, 63)       LN(112, 62)
	LN(113, 61)       LN(114, 60)       LN(115, 59)       LN(116, 58)
	LN(117, 57)       LN(118, 56)       LN(119, 55)       LN(120, 54)
	LN(121, 53)       LN(122, 52)       LN(123, 51)       LN(124, 50)
	LN(125, 49)       LN(126, 48)
	0 // We need something witout a comma at the end
};

static const int ascii_tree[] = {
	BN(0, 1, 2)       BN(1, 3, 4)       BN(2, 5, 6)       BN(3, 7, 8)
	BN(4, 9, 10)      BN(5, 11, 12)     BN(6, 13, 14)     BN(7, 15, 16)
	BN(8, 17, 18)     BN(9, 19, 20)     BN(10, 21, 22)    BN(11, 23, 24)
	BN(12, 25, 26)    BN(13, 27, 28)    BN(14, 29, 30)    BN(15, 31, 32)
	BN(16, 33, 34)    BN(17, 35, 36)    BN(18, 37, 38)    BN(19, 39, 40)
	BN(20, 41, 42)    BN(21, 43, 44)    BN(22, 45, 46)    BN(23, 47, 48)
	BN(24, 49, 50)    BN(25, 51, 52)    BN(26, 53, 54)    BN(27, 55, 56)
	BN(28, 57, 58)    BN(29, 59, 60)    LN(30, 32)
	//
	BN(31, 61, 62)    BN(32, 63, 64)    BN(33, 65, 66)    BN(34, 67, 68)
	BN(35, 69, 70)    BN(36, 71, 72)    BN(37, 73, 74)    BN(38, 75, 76)
	BN(39, 77, 78)    BN(40, 79, 80)    BN(41, 81, 82)    BN(42, 83, 84)
	BN(43, 85, 86)    BN(44, 87, 88)    BN(45, 89, 90)    BN(46, 91, 92)
	BN(47, 93, 94)    BN(48, 95, 96)    BN(49, 97, 98)    LN(50, 117)
	LN(51, 116)       LN(52, 115)       LN(53, 114)       LN(54, 111)
	LN(55, 110)       LN(56, 108)       LN(57, 105)       LN(58, 101)
	LN(59, 97)        LN(60, 69)
	//
	BN(61, 99, 100)   BN(62, 101, 102)  BN(63, 103, 104)  BN(64, 105, 106)
	BN(65, 107, 108)  BN(66, 109, 110)	BN(67, 111, 112)  BN(68, 113, 114)
	BN(69, 115, 116)  BN(70, 117, 118)  BN(71, 119, 120)  BN(72, 121, 122)
	BN(73, 123, 124)  BN(74, 125, 126)  BN(75, 127, 128)  BN(76, 129, 130)
	BN(77, 131, 132)  BN(78, 133, 134)  LN(79, 112)       LN(80, 109)
	LN(81, 104)       LN(82, 103)       LN(83, 102)       LN(84, 100)
	LN(85, 99)        LN(86, 98)        LN(87, 84)        LN(88, 83)
	LN(89, 82)        LN(90, 79)        LN(91, 78)        LN(92, 76)
	LN(93, 73)        LN(94, 68)        LN(95, 67)        LN(96, 65)
	LN(97, 49)        LN(98, 45)
	//
	BN(99, 135, 136)  BN(100, 137, 138) BN(101, 139, 140) BN(102, 141, 142)
	BN(103, 143, 144) BN(104, 145, 146)	BN(105, 147, 148) BN(106, 149, 150)
	BN(107, 151, 152) BN(108, 153, 154) BN(109, 155, 156) BN(110, 157, 158)
	BN(111, 159, 160) BN(112, 161, 162) BN(113, 163, 164) LN(114, 119)
	LN(115, 107)      LN(116, 85)       LN(117, 80)       LN(118, 77)
	LN(119, 70)       LN(120, 66)       LN(121, 61)       LN(122, 56)
	LN(123, 55)       LN(124, 53)       LN(125, 52)       LN(126, 51)
	LN(127, 50)       LN(128, 48)       LN(129, 46)       LN(130, 44)
	LN(131, 41)       LN(132, 40)       LN(133, 13)       LN(134, 10)
	//
	BN(135, 165, 166) BN(136, 167, 168) BN(137, 169, 170) BN(138, 171, 172)
	BN(139, 173, 174) BN(140, 175, 176) BN(141, 177, 178) BN(142, 179, 180)
	BN(143, 181, 182) BN(144, 183, 184) BN(145, 185, 186) BN(146, 187, 188)
	BN(147, 189, 190) BN(148, 191, 192) LN(149, 121)      LN(150, 120)
	LN(151, 118)      LN(152, 95)       LN(153, 91)       LN(154, 87)
	LN(155, 72)       LN(156, 71)       LN(157, 58)       LN(158, 57)
	LN(159, 54)       LN(160, 47)       LN(161, 42)       LN(162, 39)
	LN(163, 34)       LN(164, 9)
	//
	BN(165, 193, 194) BN(166, 195, 196) BN(167, 197, 198) BN(168, 199, 200)
	BN(169, 201, 202) BN(170, 203, 204) BN(171, 205, 206) BN(172, 207, 208)
	BN(173, 209, 210) BN(174, 211, 212) BN(175, 213, 214) BN(176, 215, 216)
	BN(177, 217, 218) BN(178, 219, 220) BN(179, 221, 222) BN(180, 223, 224)
	BN(181, 225, 226) BN(182, 227, 228)	BN(183, 229, 230) BN(184, 231, 232)
	BN(185, 233, 234) LN(186, 93)       LN(187, 89)       LN(188, 88)
	LN(189, 86)       LN(190, 75)       LN(191, 62)       LN(192, 43)
	//
	BN(193, 235, 236) BN(194, 237, 238) BN(195, 239, 240) BN(196, 241, 242)
	BN(197, 243, 244) BN(198, 245, 246)	BN(199, 247, 248) BN(200, 249, 250)
	BN(201, 251, 252) BN(202, 253, 254) BN(203, 255, 256) BN(204, 257, 258)
	BN(205, 259, 260) BN(206, 261, 262) BN(207, 263, 264) BN(208, 265, 266)
	BN(209, 267, 268) BN(210, 269, 270)	BN(211, 271, 272) BN(212, 273, 274)
	BN(213, 275, 276) BN(214, 277, 278) BN(215, 279, 280) BN(216, 281, 282)
	BN(217, 283, 284) BN(218, 285, 286) BN(219, 287, 288) BN(220, 289, 290)
	BN(221, 291, 292) BN(222, 293, 294) BN(223, 295, 296) BN(224, 297, 298)
	BN(225, 299, 300) BN(226, 301, 302) BN(227, 303, 304) BN(228, 305, 306)
	BN(229, 307, 308) LN(230, 122)      LN(231, 113)      LN(232, 38)
	LN(233, 36)       LN(234, 33)
	//
	BN(235, 309, 310) BN(236, 311, 312) BN(237, 313, 314) BN(238, 315, 316)
	BN(239, 317, 318) BN(240, 319, 320) BN(241, 321, 322) BN(242, 323, 324)
	BN(243, 325, 326) BN(244, 327, 328) BN(245, 329, 330) BN(246, 331, 332)
	BN(247, 333, 334) BN(248, 335, 336) BN(249, 337, 338) BN(250, 339, 340)
	BN(251, 341, 342) BN(252, 343, 344)	BN(253, 345, 346) BN(254, 347, 348)
	BN(255, 349, 350) BN(256, 351, 352) BN(257, 353, 354) BN(258, 355, 356)
	BN(259, 357, 358) BN(260, 359, 360) BN(261, 361, 362) BN(262, 363, 364)
	BN(263, 365, 366) BN(264, 367, 368) BN(265, 369, 370) BN(266, 371, 372)
	BN(267, 373, 374) BN(268, 375, 376) BN(269, 377, 378) BN(270, 379, 380)
	BN(271, 381, 382) BN(272, 383, 384) BN(273, 385, 386) BN(274, 387, 388)
	BN(275, 389, 390) BN(276, 391, 392) BN(277, 393, 394) BN(278, 395, 396)
	BN(279, 397, 398) BN(280, 399, 400) BN(281, 401, 402) BN(282, 403, 404)
	BN(283, 405, 406) BN(284, 407, 408) BN(285, 409, 410) BN(286, 411, 412)
	BN(287, 413, 414) BN(288, 415, 416) BN(289, 417, 418) BN(290, 419, 420)
	BN(291, 421, 422) BN(292, 423, 424) BN(293, 425, 426) BN(294, 427, 428)
	BN(295, 429, 430) BN(296, 431, 432) BN(297, 433, 434) BN(298, 435, 436)
	LN(299, 124)      LN(300, 123)      LN(301, 106)      LN(302, 92)
	LN(303, 90)       LN(304, 81)       LN(305, 74)       LN(306, 63)
	LN(307, 60)       LN(308, 0)
	//
	BN(309, 437, 438) BN(310, 439, 440) BN(311, 441, 442) BN(312, 443, 444)
	BN(313, 445, 446) BN(314, 447, 448) BN(315, 449, 450) BN(316, 451, 452)
	BN(317, 453, 454) BN(318, 455, 456) BN(319, 457, 458) BN(320, 459, 460)
	BN(321, 461, 462) BN(322, 463, 464) BN(323, 465, 466) BN(324, 467, 468)
	BN(325, 469, 470) BN(326, 471, 472)	BN(327, 473, 474) BN(328, 475, 476)
	BN(329, 477, 478) BN(330, 479, 480) BN(331, 481, 482) BN(332, 483, 484)
	BN(333, 485, 486) BN(334, 487, 488) BN(335, 489, 490) BN(336, 491, 492)
	BN(337, 493, 494) BN(338, 495, 496) BN(339, 497, 498) BN(340, 499, 500)
	BN(341, 501, 502) BN(342, 503, 504) BN(343, 505, 506) BN(344, 507, 508)
	BN(345, 509, 510) LN(346, 244)      LN(347, 243)      LN(348, 242)
	LN(349, 238)      LN(350, 233)      LN(351, 229)      LN(352, 225)
	LN(353, 223)      LN(354, 222)      LN(355, 221)      LN(356, 220)
	LN(357, 219)      LN(358, 218)      LN(359, 217)      LN(360, 216)
	LN(361, 215)      LN(362, 214)      LN(363, 213)      LN(364, 212)
	LN(365, 211)      LN(366, 210)      LN(367, 209)      LN(368, 208)
	LN(369, 207)      LN(370, 206)      LN(371, 205)      LN(372, 204)
	LN(373, 203)      LN(374, 202)      LN(375, 201)      LN(376, 200)
	LN(377, 199)      LN(378, 198)      LN(379, 197)      LN(380, 196)
	LN(381, 195)      LN(382, 194)      LN(383, 193)      LN(384, 192)
	LN(385, 191)      LN(386, 190)      LN(387, 189)      LN(388, 188)
	LN(389, 187)      LN(390, 186)      LN(391, 185)      LN(392, 184)
	LN(393, 183)      LN(394, 182)      LN(395, 181)      LN(396, 180)
	LN(397, 179)      LN(398, 178)      LN(399, 177)      LN(400, 176)
	LN(401, 127)      LN(402, 126)      LN(403, 125)      LN(404, 96)
	LN(405, 94)       LN(406, 64)       LN(407, 59)       LN(408, 37)
	LN(409, 35)       LN(410, 31)       LN(411, 30)       LN(412, 29)
	LN(413, 28)       LN(414, 27)       LN(415, 25)       LN(416, 24)
	LN(417, 23)       LN(418, 22)       LN(419, 21)       LN(420, 20)
	LN(421, 19)       LN(422, 18)       LN(423, 17)       LN(424, 16)
	LN(425, 15)       LN(426, 14)       LN(427, 12)       LN(428, 11)
	LN(429, 8)        LN(430, 7)        LN(431, 6)        LN(432, 5)
	LN(433, 4)        LN(434, 3)        LN(435, 2)        LN(436, 1)
	LN(437, 255)      LN(438, 254)      LN(439, 253)      LN(440, 252)
	LN(441, 251)      LN(442, 250)      LN(443, 249)      LN(444, 248)
	LN(445, 247)      LN(446, 246)      LN(447, 245)      LN(448, 241)
	LN(449, 240)      LN(450, 239)      LN(451, 237)      LN(452, 236)
	LN(453, 235)      LN(454, 234)      LN(455, 232)      LN(456, 231)
	LN(457, 230)      LN(458, 228)      LN(459, 227)      LN(460, 226)
	LN(461, 224)      LN(462, 175)      LN(463, 174)      LN(464, 173)
	LN(465, 172)      LN(466, 171)      LN(467, 170)      LN(468, 169)
	LN(469, 168)      LN(470, 167)      LN(471, 166)      LN(472, 165)
	LN(473, 164)      LN(474, 163)      LN(475, 162)      LN(476, 161)
	LN(477, 160)      LN(478, 159)      LN(479, 158)      LN(480, 157)
	LN(481, 156)      LN(482, 155)      LN(483, 154)      LN(484, 153)
	LN(485, 152)      LN(486, 151)      LN(487, 150)      LN(488, 149)
	LN(489, 148)      LN(490, 147)      LN(491, 146)      LN(492, 145)
	LN(493, 144)      LN(494, 143)      LN(495, 142)      LN(496, 141)
	LN(497, 140)      LN(498, 139)      LN(499, 138)      LN(500, 137)
	LN(501, 136)      LN(502, 135)      LN(503, 134)      LN(504, 133)
	LN(505, 132)      LN(506, 131)      LN(507, 130)      LN(508, 129)
	LN(509, 128)      LN(510, 26)
};

int DecompressorDCL::huffman_lookup(const int *tree) {
	int pos = 0;

	while (!(tree[pos] & HUFFMAN_LEAF)) {
		int bit = getBitsLSB(1);
		debug(8, "[%d]:%d->", pos, bit);
		pos = bit ? tree[pos] & 0xFFF : tree[pos] >> 12;
	}

	debug(8, "=%02x\n", tree[pos] & 0xffff);
	return tree[pos] & 0xFFFF;
}

#define DCL_BINARY_MODE 0
#define DCL_ASCII_MODE 1

#define MIDI_SETUP_BUNDLE_FILE_MAXIMUM_DICTIONARY_SIZE 4096

bool DecompressorDCL::unpack(SeekableReadStream *sourceStream, WriteStream *targetStream, uint32 targetSize, bool targetFixedSize) {
	byte   dictionary[MIDI_SETUP_BUNDLE_FILE_MAXIMUM_DICTIONARY_SIZE];
	uint16 dictionaryPos = 0;
	uint16 dictionarySize = 0;
	uint16 dictionaryMask = 0;
	int value;
	uint16 tokenOffset = 0;
	uint16 tokenLength = 0;

	init(sourceStream, targetStream, targetSize, targetFixedSize);

	byte mode = getByteLSB();
	byte dictionaryType = getByteLSB();

	if (mode != DCL_BINARY_MODE && mode != DCL_ASCII_MODE) {
		warning("DCL-INFLATE: Error: Encountered mode %02x, expected 00 or 01", mode);
		return false;
	}

	// TODO: original code supported 3 as well???
	// Was this an accident or on purpose? And the original code did just give out a warning
	// and didn't error out at all
	switch (dictionaryType) {
	case 4:
		dictionarySize = 1024;
		break;
	case 5:
		dictionarySize = 2048;
		break;
	case 6:
		dictionarySize = 4096;
		break;
	default:
		warning("DCL-INFLATE: Error: unsupported dictionary type %02x", dictionaryType);
		return false;
	}
	dictionaryMask = dictionarySize - 1;

	while ((!targetFixedSize) || (_bytesWritten < _targetSize)) {
		if (getBitsLSB(1)) { // (length,distance) pair
			value = huffman_lookup(length_tree);

			if (value < 8)
				tokenLength = value + 2;
			else
				tokenLength = 8 + (1 << (value - 7)) + getBitsLSB(value - 7);

			if (tokenLength == 519)
				break; // End of stream signal

			debug(8, " | ");

			value = huffman_lookup(distance_tree);

			if (tokenLength == 2)
				tokenOffset = (value << 2) | getBitsLSB(2);
			else
				tokenOffset = (value << dictionaryType) | getBitsLSB(dictionaryType);
			tokenOffset++;

			debug(8, "\nCOPY(%d from %d)\n", tokenLength, tokenOffset);

			if (_targetFixedSize) {
				if (tokenLength + _bytesWritten > _targetSize) {
					warning("DCL-INFLATE Error: Write out of bounds while copying %d bytes (declared unpacked size is %d bytes, current is %d + %d bytes)",
							tokenLength, _targetSize, _bytesWritten, tokenLength);
					return false;
				}
			}

			if (_bytesWritten < tokenOffset) {
				warning("DCL-INFLATE Error: Attempt to copy from before beginning of input stream (declared unpacked size is %d bytes, current is %d bytes)",
						_targetSize, _bytesWritten);
				return false;
			}

			uint16 dictionaryBaseIndex = (dictionaryPos - tokenOffset) & dictionaryMask;
			uint16 dictionaryIndex = dictionaryBaseIndex;
			uint16 dictionaryNextIndex = dictionaryPos;

			while (tokenLength) {
				// Write byte from dictionary
				putByte(dictionary[dictionaryIndex]);
				debug(9, "\33[32;31m%02x\33[37;37m ", dictionary[dictionaryIndex]);

				dictionary[dictionaryNextIndex] = dictionary[dictionaryIndex];

				dictionaryNextIndex = (dictionaryNextIndex + 1) & dictionaryMask;
				dictionaryIndex = (dictionaryIndex + 1) & dictionaryMask;

				if (dictionaryIndex == dictionaryPos)
					dictionaryIndex = dictionaryBaseIndex;
				if (dictionaryNextIndex == dictionarySize)
					dictionaryNextIndex = 0;

				tokenLength--;
			}
			dictionaryPos = dictionaryNextIndex;
			debug(9, "\n");

		} else { // Copy byte verbatim
			value = (mode == DCL_ASCII_MODE) ? huffman_lookup(ascii_tree) : getByteLSB();
			putByte(value);

			// Also remember it inside dictionary
			dictionary[dictionaryPos] = value;
			dictionaryPos++;
			if (dictionaryPos >= dictionarySize)
				dictionaryPos = 0;

			debug(9, "\33[32;31m%02x \33[37;37m", value);
		}
	}

	if (_targetFixedSize) {
		if (_bytesWritten != _targetSize)
			warning("DCL-INFLATE Error: Inconsistent bytes written (%d) and target buffer size (%d)", _bytesWritten, _targetSize);
		return _bytesWritten == _targetSize;
	}
	return true; // For targets featuring dynamic size we always succeed
}

bool decompressDCL(ReadStream *src, byte *dest, uint32 packedSize, uint32 unpackedSize) {
	bool success = false;
	DecompressorDCL dcl;

	if (!src || !dest)
		return false;

	byte *sourceBufferPtr = (byte *)malloc(packedSize);
	if (!sourceBufferPtr)
		return false;

	// Read source into memory
	src->read(sourceBufferPtr, packedSize);

	Common::MemoryReadStream  *sourceStream = new MemoryReadStream(sourceBufferPtr, packedSize, DisposeAfterUse::YES);
	Common::MemoryWriteStream *targetStream = new MemoryWriteStream(dest, unpackedSize);

	success = dcl.unpack(sourceStream, targetStream, unpackedSize, true);
	delete sourceStream;
	delete targetStream;
	return success;
}

SeekableReadStream *decompressDCL(SeekableReadStream *sourceStream, uint32 packedSize, uint32 unpackedSize) {
	bool success = false;
	byte *targetPtr = nullptr;
	Common::MemoryWriteStream *targetStream;
	DecompressorDCL dcl;

	targetPtr = (byte *)malloc(unpackedSize);
	if (!targetPtr)
		return nullptr;

	targetStream = new MemoryWriteStream(targetPtr, unpackedSize);

	success = dcl.unpack(sourceStream, targetStream, unpackedSize, true);
	delete targetStream;

	if (!success) {
		free(targetPtr);
		return nullptr;
	}
	return new MemoryReadStream(targetPtr, unpackedSize, DisposeAfterUse::YES);
}

// This one figures out the unpacked size by itself
// Needed for at least Simon 2, because the unpacked size is not stored anywhere
SeekableReadStream *decompressDCL(SeekableReadStream *sourceStream) {
	Common::MemoryWriteStreamDynamic *targetStream;
	DecompressorDCL dcl;

	targetStream = new MemoryWriteStreamDynamic(DisposeAfterUse::NO);

	if (dcl.unpack(sourceStream, targetStream, 0, false)) {
		byte *targetPtr = targetStream->getData();
		uint32 unpackedSize = targetStream->size();
		delete targetStream;
		return new MemoryReadStream(targetPtr, unpackedSize, DisposeAfterUse::YES);
	}
	delete targetStream;
	return nullptr;
}

} // End of namespace Common
