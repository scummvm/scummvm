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

#include "mads/dragonsphere/sound/gsound_dragonsphere.h"

namespace MADS {
namespace Dragonsphere {
namespace Sound {

#define R(channel, offset) { channel, offset }
#define S(command, mode, flags, counter, period, guardCount, guard1, guard2, guard3, rootCount, ...) \
	{ command, mode, flags, 0, counter, period, guardCount, \
		{ guard1, guard2, guard3 }, rootCount, { __VA_ARGS__ } }
#define N(command) { command, kGSoundNoOp, 0, 0, 0, 0, 0, { 0, 0, 0 }, 0, {} }
#define X(command) { command, kGSoundSpecial, 0, 0, 0, 0, 0, { 0, 0, 0 }, 0, {} }
#define F1(command, a) S(command, kGSoundEffect78, 0, 0, 0, 0, 0, 0, 0, 1, R(0, a))
#define F2(command, a, b) S(command, kGSoundEffect78, 0, 0, 0, 0, 0, 0, 0, 2, R(0, a), R(0, b))
#define F3(command, a, b, c) S(command, kGSoundEffect78, 0, 0, 0, 0, 0, 0, 0, 3, R(0, a), R(0, b), R(0, c))
#define MFLAGS (kGSoundCheckFirstRoot | kGSoundDeferWhileActive | kGSoundStopMusic)
#define DFLAGS (kGSoundCheckFirstRoot | kGSoundStopMusic)
#define M1(command, counter, period, c1, a) S(command, kGSoundMusic, MFLAGS, counter, period, 1, a, 0, 0, 1, R(c1, a))
#define M2(command, counter, period, c1, a, c2, b) S(command, kGSoundMusic, MFLAGS, counter, period, 1, a, 0, 0, 2, R(c1, a), R(c2, b))
#define M3(command, counter, period, c1, a, c2, b, c3, c) S(command, kGSoundMusic, MFLAGS, counter, period, 1, a, 0, 0, 3, R(c1, a), R(c2, b), R(c3, c))
#define M4(command, counter, period, c1, a, c2, b, c3, c, c4, d) S(command, kGSoundMusic, MFLAGS, counter, period, 1, a, 0, 0, 4, R(c1, a), R(c2, b), R(c3, c), R(c4, d))
#define M5(command, counter, period, c1, a, c2, b, c3, c, c4, d, c5, e) S(command, kGSoundMusic, MFLAGS, counter, period, 1, a, 0, 0, 5, R(c1, a), R(c2, b), R(c3, c), R(c4, d), R(c5, e))
#define M6(command, counter, period, c1, a, c2, b, c3, c, c4, d, c5, e, c6, f) S(command, kGSoundMusic, MFLAGS, counter, period, 1, a, 0, 0, 6, R(c1, a), R(c2, b), R(c3, c), R(c4, d), R(c5, e), R(c6, f))
#define M7(command, counter, period, c1, a, c2, b, c3, c, c4, d, c5, e, c6, f, c7, g) S(command, kGSoundMusic, MFLAGS, counter, period, 1, a, 0, 0, 7, R(c1, a), R(c2, b), R(c3, c), R(c4, d), R(c5, e), R(c6, f), R(c7, g))
#define M8(command, counter, period, c1, a, c2, b, c3, c, c4, d, c5, e, c6, f, c7, g, c8, h) S(command, kGSoundMusic, MFLAGS, counter, period, 1, a, 0, 0, 8, R(c1, a), R(c2, b), R(c3, c), R(c4, d), R(c5, e), R(c6, f), R(c7, g), R(c8, h))
#define D4(command, c1, a, c2, b, c3, c, c4, d) S(command, kGSoundDirectMusic, DFLAGS, 0, 0, 1, a, 0, 0, 4, R(c1, a), R(c2, b), R(c3, c), R(c4, d))

static const GSoundDriverData kDriver1 = {
	"GSOUND.DR1", 22172, "9cd1f97006d15d66d7312d492a0f67d4",
	0x021d, 0x34a0, 0x32cc,
	{ 0x1937, 0x1974, 0x1a0b, 0x1a6a, 0x1a88, 0x1a7e,
		0x1a83, 0x1a84, 0x1a85, 0x1a86, 0x1a87 },
	{ 8, 18, 31, 48, 101 }, 1
};
static const GSoundDriverData kDriver2 = {
	"GSOUND.DR2", 12673, "7b0ad8a5cb56993516e0f88dbb86327d",
	0x01d7, 0x13f0, 0x1211,
	{ 0x1923, 0x1960, 0x19f7, 0x1a56, 0x1a74, 0x1a6a,
		0x1a6f, 0x1a70, 0x1a71, 0x1a72, 0x1a73 },
	{ 8, 18, 31, 35, 72 }, 2
};
static const GSoundDriverData kDriver3 = {
	"GSOUND.DR3", 11970, "ce473005db35abd08c440de56b674f6c",
	0x01d3, 0x1170, 0x0f92,
	{ 0x1937, 0x1974, 0x1a0b, 0x1a6a, 0x1a88, 0x1a7e,
		0x1a83, 0x1a84, 0x1a85, 0x1a86, 0x1a87 },
	{ 8, 18, 31, 33, 73 }, 3
};
static const GSoundDriverData kDriver4 = {
	"GSOUND.DR4", 15499, "19c8c5b3eba38a9d028e5ef84e6c91a6",
	0x01ed, 0x1d90, 0x1bbb,
	{ 0x19ed, 0x1a2a, 0x1ac1, 0x1b20, 0x1b3e, 0x1b34,
		0x1b39, 0x1b3a, 0x1b3b, 0x1b3c, 0x1b3d },
	{ 8, 18, 31, 40, 78 }, 4
};
static const GSoundDriverData kDriver5 = {
	"GSOUND.DR5", 15262, "b34a5feed61c725db6043147546c64d2",
	0x01ee, 0x1c90, 0x1abe,
	{ 0x194b, 0x1988, 0x1a1f, 0x1a7e, 0x1a9c, 0x1a92,
		0x1a97, 0x1a98, 0x1a99, 0x1a9a, 0x1a9b },
	{ 8, 18, 31, 38, 78 }, 5
};
static const GSoundDriverData kDriver6 = {
	"GSOUND.DR6", 17325, "ef84204bd776b66e636b809b305d0f55",
	0x020c, 0x22c0, 0x20ed,
	{ 0x1937, 0x1974, 0x1a0b, 0x1a6a, 0x1a88, 0x1a7e,
		0x1a83, 0x1a84, 0x1a85, 0x1a86, 0x1a87 },
	{ 8, 18, 31, 46, 98 }, 6
};
static const GSoundDriverData kDriver9 = {
	"GSOUND.DR9", 30809, "7789a8e19b836876a7fcb60563dbb32e",
	0x021b, 0x5680, 0x54a9,
	{ 0x07c9, 0x0806, 0x0899, 0x08f8, 0x0916, 0x090c,
		0x0911, 0x0912, 0x0913, 0x0914, 0x0915 },
	{ 8, 18, 31, 63, 0 }, 9
};

bool validateDragonsphereGSoundFiles() {
	const GSoundDriverData *drivers[] = {
		&kDriver1, &kDriver2, &kDriver3, &kDriver4,
		&kDriver5, &kDriver6, &kDriver9
	};
	for (uint i = 0; i < ARRAYSIZE(drivers); ++i) {
		if (!GSound::validateOverlay(*drivers[i]))
			return false;
	}
	return true;
}

static const GSoundCommandSpec kCommands1[] = {
	M4(16, 0x90, 0x90, 1, 0x083c, 2, 0x08eb, 3, 0x0968, 4, 0x09e5),
	D4(17, 1, 0x0a40, 2, 0x0a72, 3, 0x0aa5, 4, 0x0abe),
	F2(24, 0x00b1, 0x00c3), F2(25, 0x00d7, 0x00e9),
	F1(26, 0x00fd), F1(27, 0x0107), F2(28, 0x0127, 0x014b),
	N(29), F1(30, 0x016f), F1(31, 0x01a2),
	M4(32, 0xb0, 0xb0, 1, 0x0df6, 2, 0x0e40, 3, 0x0edd, 4, 0x0efc),
	M4(33, 0xb0, 0xb0, 1, 0x0bf8, 2, 0x0ce6, 3, 0x0d4d, 4, 0x0d7e),
	M3(34, 0x50, 0x50, 1, 0x2d54, 2, 0x2dca, 3, 0x2de2),
	M4(35, 0x60, 0x60, 1, 0x2498, 2, 0x2533, 3, 0x25ba, 4, 0x2607),
	M4(36, 0x80, 0x80, 1, 0x21da, 2, 0x22a0, 3, 0x238f, 4, 0x242a),
	M2(37, 0xc0, 0xc0, 1, 0x317c, 2, 0x31d9),
	M4(38, 0x60, 0x60, 1, 0x0f24, 2, 0x0fcb, 3, 0x10a7, 4, 0x1196),
	M3(39, 0xb0, 0xb0, 1, 0x128c, 2, 0x1318, 3, 0x136f),
	M5(40, 0xa8, 0xa8, 1, 0x13d0, 2, 0x1548, 3, 0x16ff, 4, 0x19c4, 5, 0x1bc1),
	M5(41, 0x90, 0x90, 1, 0x1e5c, 2, 0x1ed0, 9, 0x1f49, 4, 0x1fa4, 5, 0x207f),
	M5(42, 0x90, 0x90, 1, 0x0af8, 2, 0x0b26, 3, 0x0b5a, 4, 0x0b9a, 5, 0x0bd4),
	X(43),
	M6(44, 0x60, 0xe0, 1, 0x2dfa, 2, 0x2e42, 3, 0x2e8b, 4, 0x2eb5, 5, 0x303b, 6, 0x3103),
	M5(45, 0x60, 0x60, 1, 0x277a, 2, 0x28e6, 3, 0x29ce, 4, 0x2acb, 5, 0x2bbb),
	M2(46, 0x90, 0x90, 1, 0x3242, 2, 0x3297),
	M4(47, 0x60, 0x60, 1, 0x0f9a, 2, 0x104c, 3, 0x1163, 4, 0x122a),
	X(48),
	F2(64, 0x023e, 0x024d), F1(65, 0x0270), F1(66, 0x0284),
	F1(67, 0x02ca), F1(68, 0x02f6), F1(69, 0x0302),
	F1(70, 0x0324), F1(71, 0x034c), F1(72, 0x035c),
	F1(73, 0x0372), F1(74, 0x0386), F1(75, 0x03aa),
	F1(76, 0x03e0), F2(77, 0x0402, 0x040e), F1(78, 0x041a),
	F1(79, 0x0441), F2(80, 0x0466, 0x048f), F2(81, 0x04ba, 0x04ba),
	F1(82, 0x04e1), F1(83, 0x0545), F1(84, 0x0574),
	F2(85, 0x057e, 0x0592), F2(86, 0x0592, 0x05bb), F1(87, 0x05e4),
	F2(88, 0x05fe, 0x0617), F2(89, 0x0630, 0x0642), F1(90, 0x0654),
	F1(91, 0x067e), N(92), F2(93, 0x01f8, 0x021b), F1(94, 0x02a8),
	F2(95, 0x06b4, 0x0716), F1(96, 0x0763), F1(97, 0x0787), N(98),
	F1(99, 0x07a1), F2(100, 0x03f6, 0x0402), F2(101, 0x07f7, 0x07b3)
};

static const GSoundCommandSpec kCommands2[] = {
	M2(16, 0x60, 0x60, 1, 0x1056, 2, 0x10a7),
	D4(17, 1, 0x0386, 2, 0x03b8, 3, 0x03eb, 4, 0x0404),
	F2(24, 0x00a8, 0x00ba), F2(25, 0x00ce, 0x00e0), F1(26, 0x00f4),
	F1(27, 0x00fe), F2(28, 0x011e, 0x0142), N(29), F1(30, 0x016f), F1(31, 0x01a2),
	M6(32, 0x60, 0x60, 1, 0x043e, 2, 0x0491, 3, 0x052f, 4, 0x05f1, 5, 0x067b, 6, 0x06cb),
	M6(33, 0x60, 0x60, 1, 0x06f4, 2, 0x0795, 3, 0x0831, 4, 0x08d9, 5, 0x09ad, 6, 0x0a31),
	M6(34, 0x60, 0x60, 1, 0x0fb8, 2, 0x1058, 3, 0x10a7, 4, 0x10fc, 5, 0x1151, 6, 0x11b2),
	M6(35, 0xc0, 0x50, 1, 0x0ad8, 2, 0x0c17, 3, 0x0ca5, 4, 0x0d7f, 5, 0x0e57, 6, 0x0ef9),
	F1(64, 0x01f8), F2(65, 0x0202, 0x021e), F1(66, 0x023e), F1(67, 0x025d),
	F1(68, 0x0283), F2(69, 0x0293, 0x02a9), F2(70, 0x02c1, 0x02d1),
	F2(71, 0x02e1, 0x031f), F2(72, 0x0337, 0x035e)
};

static const GSoundCommandSpec kCommands3[] = {
	M6(16, 0x70, 0x70, 1, 0x038e, 2, 0x0458, 3, 0x04b7, 4, 0x0638, 5, 0x06ad, 9, 0x0783),
	D4(17, 1, 0x0874, 2, 0x08a6, 3, 0x08d9, 4, 0x08f2),
	F2(24, 0x00a8, 0x00ba), F2(25, 0x00ce, 0x00e0), F1(26, 0x00f4),
	F1(27, 0x00fe), F2(28, 0x0117, 0x013b), N(29), F1(30, 0x0161), F1(31, 0x018d),
	M6(32, 0x70, 0x70, 1, 0x0bea, 2, 0x0ca6, 3, 0x0d5f, 4, 0x0e1a, 5, 0x0e49, 6, 0x0f14),
	M6(33, 0x70, 0x70, 1, 0x092c, 2, 0x0a02, 3, 0x0a66, 4, 0x0a77, 5, 0x0b74, 9, 0x0ba0),
	F1(64, 0x01e3), F1(65, 0x0235), F1(66, 0x024f), F1(67, 0x0272),
	F2(68, 0x0299, 0x02a5), F1(69, 0x02b3), F2(70, 0x02d1, 0x02fb),
	F1(71, 0x0265), F2(72, 0x0349, 0x0366), F1(73, 0x0383)
};

static const GSoundCommandSpec kCommands4[] = {
	M5(16, 0xc0, 0xc0, 1, 0x156b, 2, 0x15ad, 3, 0x15eb, 4, 0x1557, 9, 0x1644),
	D4(17, 1, 0x03c4, 2, 0x03f6, 3, 0x0429, 4, 0x0442),
	F2(24, 0x00a8, 0x00ba), F2(25, 0x00ce, 0x00e0), F1(26, 0x00f4),
	F1(27, 0x00fe), F2(28, 0x011e, 0x0142), N(29), F1(30, 0x016f), F1(31, 0x01a2),
	M6(32, 0x60, 0x60, 1, 0x092e, 2, 0x0a16, 3, 0x0c2e, 4, 0x0ce4, 5, 0x0d02, 6, 0x0d86),
	M6(33, 0xc0, 0xc0, 1, 0x0e17, 2, 0x0e37, 3, 0x0e69, 4, 0x0eb4, 5, 0x0e06, 9, 0x0f7d),
	M6(34, 0xc0, 0xc0, 1, 0x0e17, 2, 0x0e37, 3, 0x0e69, 4, 0x0eb4, 5, 0x0e06, 9, 0x0f7d),
	M1(35, 0x54, 0x54, 1, 0x1020),
	M5(36, 0x54, 0x54, 1, 0x047c, 2, 0x06e4, 3, 0x0735, 4, 0x0752, 5, 0x0923),
	M5(37, 0x40, 0x40, 1, 0x1096, 2, 0x10d8, 3, 0x1109, 4, 0x1144, 5, 0x1185),
	M5(38, 0x40, 0x40, 1, 0x11be, 2, 0x1220, 3, 0x125b, 4, 0x1290, 9, 0x12d7),
	M6(39, 0x48, 0x48, 1, 0x1724, 2, 0x1798, 3, 0x1805, 4, 0x18c8, 5, 0x1973, 9, 0x1a32),
	M5(40, 0xc0, 0xc0, 1, 0x1566, 2, 0x15a8, 3, 0x15e6, 4, 0x1548, 9, 0x1644),
	F1(64, 0x01f8), F1(65, 0x0262), F1(66, 0x026c), F1(67, 0x02da),
	F2(68, 0x0280, 0x0280), F1(69, 0x022d), F2(70, 0x02a8, 0x02bc),
	F1(71, 0x02d0), F1(72, 0x0304), F1(73, 0x033b), F1(74, 0x0319),
	F1(75, 0x0359), F2(76, 0x036f, 0x037f), F1(77, 0x038f), F1(78, 0x039f)
};

static const GSoundCommandSpec kCommands5[] = {
	M6(16, 0xc0, 0xc0, 1, 0x1494, 2, 0x15f8, 3, 0x1688, 4, 0x1764, 5, 0x17e8, 9, 0x1a08),
	D4(17, 1, 0x0440, 2, 0x0472, 3, 0x04a5, 4, 0x04be),
	F2(24, 0x00a8, 0x00ba), F2(25, 0x00ce, 0x00e0), F1(26, 0x00f4),
	F1(27, 0x00fe), F2(28, 0x011e, 0x0142), N(29), F1(30, 0x016f), F1(31, 0x01a4),
	M5(32, 0x48, 0x48, 1, 0x04f8, 2, 0x053b, 3, 0x0569, 4, 0x0599, 9, 0x0628),
	M6(33, 0x60, 0x60, 1, 0x0db4, 2, 0x0e62, 3, 0x0ef6, 4, 0x1088, 5, 0x1156, 9, 0x11f8),
	M6(34, 0xc0, 0xc0, 1, 0x07dc, 2, 0x089a, 3, 0x095d, 4, 0x0a58, 5, 0x0af9, 9, 0x0c7c),
	M5(35, 0xc0, 0xc0, 1, 0x0cb2, 2, 0x0cc5, 3, 0x0cfa, 4, 0x0d47, 5, 0x0d6e),
	S(36, kGSoundDirectChannels, kGSoundStopMusic, 0, 0, 0, 0, 0, 0, 1, R(4, 0x0d83)),
	M4(37, 0x60, 0x60, 1, 0x0720, 2, 0x075d, 3, 0x0791, 4, 0x07c5),
	M6(38, 0xc0, 0xc0, 1, 0x084e, 2, 0x0919, 3, 0x0994, 4, 0x0ae1, 5, 0x0b6a, 9, 0x0c97),
	F1(64, 0x028d), F1(65, 0x025d), N(66), F1(67, 0x0345),
	F2(68, 0x03b9, 0x03b9), F1(69, 0x0382), F3(70, 0x02f9, 0x030d, 0x02e9),
	F2(71, 0x02b3, 0x02d0), F1(72, 0x0321), F1(73, 0x01fa),
	F2(74, 0x023d, 0x024d), F1(75, 0x0212), F2(76, 0x030d, 0x02e9),
	N(77), F1(78, 0x03e7)
};

static const GSoundCommandSpec kCommands6[] = {
	X(16), D4(17, 1, 0x06da, 2, 0x070c, 3, 0x073f, 4, 0x0758),
	F2(24, 0x00a8, 0x00ba), F2(25, 0x00ce, 0x00e0), F1(26, 0x00f4),
	F1(27, 0x00fe), F2(28, 0x011e, 0x0142), N(29), F1(30, 0x0470), F1(31, 0x016f),
	X(32), X(33),
	M4(34, 0x1e, 0x1e, 1, 0x07be, 2, 0x0802, 3, 0x0841, 4, 0x08d4),
	M1(35, 0x1e, 0x1e, 1, 0x07be),
	M4(36, 0xc8, 0xc8, 1, 0x1f1e, 2, 0x1f73, 3, 0x200f, 4, 0x2082),
	M5(37, 0xc8, 0xc8, 1, 0x1c2a, 2, 0x1c7c, 3, 0x1cda, 4, 0x1dd8, 5, 0x1ed7),
	M6(38, 0xc8, 0xc8, 1, 0x17f6, 2, 0x1850, 3, 0x18aa, 4, 0x1936, 5, 0x19ef, 6, 0x1bda),
	M6(39, 0x64, 0x64, 1, 0x1266, 2, 0x1391, 3, 0x159b, 4, 0x1629, 5, 0x164f, 9, 0x1777),
	M6(40, 0x64, 0x64, 1, 0x0baa, 2, 0x0c94, 3, 0x0d7a, 4, 0x0e6e, 5, 0x0f8e, 6, 0x108c),
	N(41), N(42), N(43),
	M6(44, 0x30, 0x30, 1, 0x10fe, 2, 0x1148, 3, 0x1172, 4, 0x11a6, 5, 0x11d6, 9, 0x122e),
	X(45), N(46),
	N(64), F1(65, 0x01c5), F1(66, 0x03e4), F2(67, 0x01ea, 0x0206),
	F2(68, 0x0222, 0x022e), F1(69, 0x0453), F1(70, 0x02a0), F1(71, 0x042a),
	F2(72, 0x030e, 0x032c), F1(73, 0x02e0), F2(74, 0x02f4, 0x0300),
	F1(75, 0x0290), F1(76, 0x03b6), F1(77, 0x0390), F1(78, 0x02ac),
	F2(79, 0x02bc, 0x02ce), X(80), F1(81, 0x023e), F2(82, 0x025c, 0x0276),
	F2(83, 0x0344, 0x036a), F1(84, 0x03ee), F1(85, 0x040c),
	F2(86, 0x04a3, 0x04c4), F1(87, 0x0533), F2(88, 0x04e5, 0x050c),
	F1(89, 0x053f), F2(90, 0x055d, 0x0560), F1(91, 0x058c),
	F2(92, 0x05b3, 0x05c5), F1(93, 0x05d7), F1(94, 0x05e5),
	F1(95, 0x061b), X(96), F2(97, 0x0651, 0x0654), F2(98, 0x0680, 0x06b6)
};

static const GSoundCommandSpec kCommands9[] = {
	N(16), N(17), N(18), N(24), N(25), N(26), N(27), N(28), N(29), N(30), N(31),
	M6(32, 0x54, 0x54, 1, 0x04a4, 2, 0x04cb, 3, 0x0580, 4, 0x05bb, 5, 0x06f0, 9, 0x07db),
	M7(33, 0x54, 0x54, 1, 0x0826, 2, 0x0888, 3, 0x08c0, 9, 0x0989, 5, 0x09d5, 6, 0x0a35, 7, 0x0b9f),
	M7(34, 0x38, 0x38, 1, 0x0c7e, 2, 0x0e71, 3, 0x107c, 9, 0x127b, 5, 0x13fa, 6, 0x1539, 7, 0x1598),
	M7(35, 0x50, 0x50, 1, 0x1630, 2, 0x169e, 3, 0x16d2, 4, 0x179e, 9, 0x180e, 6, 0x1832, 7, 0x18b0),
	M7(36, 0x28, 0x28, 1, 0x1a14, 2, 0x1a8a, 3, 0x1b06, 4, 0x1b3a, 5, 0x1df2, 6, 0x1e5e, 7, 0x1eca),
	M6(37, 0x50, 0x50, 1, 0x1f0c, 2, 0x1fa2, 3, 0x2038, 4, 0x2082, 9, 0x2156, 6, 0x21ba),
	M7(38, 0x28, 0x28, 1, 0x22f0, 2, 0x236a, 3, 0x23ea, 4, 0x1b3a, 5, 0x241e, 9, 0x2482, 7, 0x24e4),
	M7(39, 0x28, 0x28, 1, 0x2522, 2, 0x25b0, 3, 0x2644, 4, 0x2742, 5, 0x2bbe, 9, 0x2c58, 7, 0x2ca0),
	M7(40, 0x38, 0x38, 1, 0x0c7e, 2, 0x0e71, 3, 0x107c, 9, 0x2d9c, 5, 0x2e92, 6, 0x2f81, 7, 0x2ff3),
	M6(41, 0x54, 0x54, 1, 0x3054, 3, 0x3526, 9, 0x36d7, 5, 0x3772, 6, 0x3857, 7, 0x3ae0),
	M7(42, 0x50, 0x50, 1, 0x309f, 2, 0x30f3, 3, 0x35a7, 9, 0x3708, 5, 0x37ab, 6, 0x393c, 7, 0x3b7f),
	M3(43, 0x60, 0x60, 1, 0x0110, 2, 0x0172, 3, 0x01c2), N(44),
	X(45), X(46),
	M7(47, 0x54, 0x54, 1, 0x0826, 2, 0x0888, 3, 0x08c0, 9, 0x0989, 5, 0x09d5, 6, 0x0a35, 7, 0x0b9f),
	S(48, kGSoundEffectChannel8, 0, 0, 0, 0, 0, 0, 0, 1, R(0, 0x00bc)),
	S(49, kGSoundEffectChannel8, 0, 0, 0, 0, 0, 0, 0, 1, R(0, 0x00de)),
	S(50, kGSoundEffectChannel8, 0, 0, 0, 0, 0, 0, 0, 1, R(0, 0x00d2)),
	M4(51, 0x60, 0x60, 1, 0x4a78, 2, 0x4b15, 3, 0x4b9c, 4, 0x4be9),
	M5(52, 0x54, 0x54, 1, 0x45c6, 2, 0x482e, 3, 0x487f, 4, 0x489c, 5, 0x4a6d),
	X(53),
	M7(54, 0x60, 0x60, 1, 0x3ce0, 2, 0x3cf1, 3, 0x3cfb, 4, 0x3d05, 5, 0x3d2f, 6, 0x3d39, 7, 0x3d63),
	M7(55, 0x60, 0x60, 1, 0x3d72, 2, 0x3e96, 3, 0x4093, 4, 0x413b, 5, 0x4159, 6, 0x41bd, 7, 0x421f),
	N(56),
	M8(57, 0x30, 0x30, 1, 0x422e, 2, 0x42b5, 3, 0x4333, 4, 0x4375, 5, 0x43b1, 6, 0x4423, 7, 0x4451, 9, 0x4487),
	M4(58, 0x90, 0x90, 1, 0x44d6, 2, 0x450f, 3, 0x4529, 4, 0x4563),
	S(59, kGSoundDirectChannels, kGSoundStopAll, 0x54, 0x54, 1, 0x457e, 0, 0, 3, R(1, 0x457e), R(2, 0x4591), R(3, 0x45ab)),
	N(60),
	S(61, kGSoundEffectChannel8, 0, 0, 0, 0, 0, 0, 0, 1, R(0, 0x00f6)),
	M5(62, 0x40, 0x40, 1, 0x4d9e, 2, 0x4f97, 3, 0x51a2, 4, 0x53a1, 5, 0x5412),
	X(63)
};

GSoundDragonsphere::GSoundDragonsphere(Audio::Mixer *mixer,
		const GSoundDriverData &driverData,
		const GSoundCommandSpec *commandSpecs, uint commandSpecCount) :
		GSound(mixer, driverData), _commandSpecs(commandSpecs),
		_commandSpecCount(commandSpecCount), _section(driverData.section) {
}

bool GSoundDragonsphere::validCommand(int commandId) const {
	if (commandId < 0)
		return false;
	if (commandId < 16)
		return commandId <= (_section == 9 ? kDriver9.commandMax[0] :
				_section == 1 ? kDriver1.commandMax[0] : kDriver2.commandMax[0]);

	const GSoundDriverData *data = nullptr;
	switch (_section) {
	case 1: data = &kDriver1; break;
	case 2: data = &kDriver2; break;
	case 3: data = &kDriver3; break;
	case 4: data = &kDriver4; break;
	case 5: data = &kDriver5; break;
	case 6: data = &kDriver6; break;
	case 9: data = &kDriver9; break;
	default: return false;
	}
	if (commandId < 24)
		return commandId <= data->commandMax[1];
	if (commandId < 32)
		return commandId <= data->commandMax[2];
	if (commandId < 64)
		return commandId <= data->commandMax[3];
	return data->commandMax[4] && commandId <= data->commandMax[4];
}

const GSoundCommandSpec *GSoundDragonsphere::findCommandSpec(int commandId) const {
	for (uint i = 0; i < _commandSpecCount; ++i) {
		if (_commandSpecs[i].command == commandId)
			return &_commandSpecs[i];
	}
	return nullptr;
}

int GSoundDragonsphere::command(int commandId, int param) {
	Common::StackLock lock(_driverMutex);
	if (!validCommand(commandId))
		return 0;

	// DR1-DR6 preserve the active 32-bucket selector for command 18.
	// DR9 has a materially different dispatcher and does not perform this store.
	if (_section != 9 && (commandId == 16 ||
			(commandId >= 32 && commandId < 64)))
		setMusicIndex(commandId);
	return executeCommand(commandId, param);
}

bool GSoundDragonsphere::runDeferredMusic(int internalCommand, uint16 guard,
		uint16 counter, uint16 period, const GSoundChannelRoot *roots,
		uint rootCount) {
	if (soundActive(guard))
		return true;
	if (channelsActive()) {
		scheduleSpecial(internalCommand, counter, period);
		return true;
	}
	armNativeTimer(counter, period);
	stopMusic();
	for (uint i = 0; i < rootCount; ++i)
		loadChannel(roots[i].channel, roots[i].offset);
	return true;
}

bool GSoundDragonsphere::executeSpecialCommand(int commandId,
		bool fromDeferred) {
	if (_section == 1 && commandId == 0x101) {
		static const GSoundChannelRoot roots[] = {
			R(1, 0x2630), R(2, 0x266e), R(3, 0x26df)
		};
		armNativeTimer(0x50, 0x50);
		stopMusic();
		for (uint i = 0; i < ARRAYSIZE(roots); ++i)
			loadChannel(roots[i].channel, roots[i].offset);
		return true;
	}
	if (_section == 5 && commandId == 0x105) {
		static const GSoundChannelRoot roots[] = {
			R(1, 0x14a5), R(2, 0x1602), R(3, 0x1692),
			R(4, 0x176e), R(5, 0x17f8), R(9, 0x1a0e)
		};
		armNativeTimer(0xc0, 0xc0);
		stopMusic();
		for (uint i = 0; i < ARRAYSIZE(roots); ++i)
			loadChannel(roots[i].channel, roots[i].offset);
		return true;
	}
	if (_section == 1 && (commandId == 43 || commandId == 48)) {
		static const GSoundChannelRoot roots[] = {
			R(1, 0x20d4), R(2, 0x2120), R(3, 0x2165),
			R(4, 0x218c), R(5, 0x21b5)
		};
		if (soundActive(0x20d4))
			return true;
		if (channelsActive()) {
			deferNativeCommand(commandId);
			return true;
		}
		// The two public entries differ only in this verified stream byte.
		setDataByte(0x2121, commandId == 43 ? 0x3c : 0x30);
		armNativeTimer(0x54, 0x54);
		stopMusic();
		for (uint i = 0; i < ARRAYSIZE(roots); ++i)
			loadChannel(roots[i].channel, roots[i].offset);
		return true;
	}
	if (_section == 6 && commandId == 16) {
		static const GSoundChannelRoot roots[] = {
			R(1, 0x0a7a), R(2, 0x0b04), R(3, 0x0b4d)
		};
		if (!fromDeferred) {
			if (soundActive(0x0a7a) || soundActive(0x1f1e) ||
					soundActive(0x1c2a))
				return true;
			if (channelsActive()) {
				scheduleSpecial(commandId, 0xc8, 0xc8);
				return true;
			}
		}
		armNativeTimer(0xc8, 0xc8);
		setMusicIndex(16);
		stopMusic();
		for (uint i = 0; i < ARRAYSIZE(roots); ++i)
			loadChannel(roots[i].channel, roots[i].offset);
		return true;
	}
	if (_section == 6 && (commandId == 32 || commandId == 33)) {
		static const GSoundChannelRoot roots[] = {
			R(1, 0x0918), R(2, 0x0960), R(3, 0x09a5), R(9, 0x09ca)
		};
		if (!fromDeferred) {
			if (commandId == 33) {
				if (soundActive(0x0a59))
					return true;
				setDataByte(0x0a78, 0xff);
			}
			if (_channels[0]._activeCount && !channelPlays(1, 0x0918)) {
				if (channelPlays(4, 0x0a59))
					_channels[3].enableFade(0xff);
				scheduleSpecial(commandId, 0x3c, 0x3c);
				return true;
			}
		}
		if (!channelPlays(1, 0x0918)) {
			armNativeTimer(0x3c, 0x3c);
			stopMusic();
			for (uint i = 0; i < ARRAYSIZE(roots); ++i)
				loadChannel(roots[i].channel, roots[i].offset);
		}
		if (getDataByte(0x0a78) == 0xff) {
			setDataByte(0x0a78, 0);
			loadChannel(4, 0x0a59);
		}
		return true;
	}
	if (_section == 6 && commandId == 45) {
		if (!fromDeferred) {
			if (soundActive(0x20b0))
				return true;
			if (channelsActive()) {
				scheduleSpecial(commandId, 0x1e, 0x1e);
				return true;
			}
		}
		armNativeTimer(0x1e, 0x1e);
		stopAll();
		playNativeEffectAny(0x20b0);
		return true;
	}
	if (_section == 6 && commandId == 80) {
		executeCommand(4, 0);
		return true;
	}
	if (_section == 6 && commandId == 96) {
		if (!fromDeferred && channelsActive()) {
			scheduleSpecial(commandId, 0x5a, 0x5a);
			return true;
		}
		armNativeTimer(0x5a, 0x5a);
		stopAll();
		for (int channel = 1; channel <= 4; ++channel)
			loadChannel(channel, 0x0792);
		return true;
	}
	if (_section == 9 && commandId == 45) {
		playNativeEffectAny(0x4c12);
		playNativeEffectAny(0x4c74);
		return true;
	}
	if (_section == 9 && commandId == 46) {
		playNativeEffectAny(0x4cc1);
		playNativeEffectAny(0x4d18);
		return true;
	}
	if (_section == 9 && commandId == 53) {
		if (!fromDeferred)
			scheduleSpecial(commandId, 0x04b0, 0x04b0);
		else
			stopMusic();
		return true;
	}
	if (_section == 9 && commandId == 63) {
		playNativeEffectAny(0x4d70);
		return true;
	}

	return false;
}

bool GSoundDragonsphere::executeNativeCallback(uint16 targetOffset,
		GSoundChannel &channel) {
	(void)channel;
	if (_section == 1 && targetOffset == 0x1cae) {
		executeCommand(16, 0);
		return true;
	}
	if (_section == 1 && targetOffset == 0x1ddc) {
		executeCommand(32, 0);
		return true;
	}
	if (_section == 1 && targetOffset == 0x1ee2) {
		executeCommand(40, 0);
		return true;
	}
	if (_section == 1 && targetOffset == 0x1f2a) {
		executeCommand(41, 0);
		return true;
	}
	if (_section == 1 && targetOffset == 0x2053) {
		static const GSoundChannelRoot roots[] = {
			R(1, 0x2630), R(2, 0x266e), R(3, 0x26df)
		};
		return runDeferredMusic(0x101, 0x2630, 0x50, 0x50,
				roots, ARRAYSIZE(roots));
	}
	if (_section == 3 && targetOffset == 0x1b82) {
		// The native callback starts with channel 1. A non-null saved branch
		// target selects channel 3, and a second one selects channel 4.
		GSoundChannel *selected = &_channels[0];
		if (selected->_branchTarget) {
			selected = &_channels[2];
			if (selected->_branchTarget)
				selected = &_channels[3];
		}
		byte note = selected->_note;
		while (note < 0x58)
			note += 12;
		setDataByte(0x026e, note);
		return true;
	}
	if (_section == 5 && targetOffset == 0x1e81) {
		static const GSoundChannelRoot roots[] = {
			R(1, 0x14a5), R(2, 0x1602), R(3, 0x1692),
			R(4, 0x176e), R(5, 0x17f8), R(9, 0x1a0e)
		};
		return runDeferredMusic(0x105, 0x14a5, 0xc0, 0xc0,
				roots, ARRAYSIZE(roots));
	}
	if (_section == 6 && targetOffset == 0x1ffc) {
		executeCommand(37, 0);
		return true;
	}
	if (_section == 9 && targetOffset == 0x1ba6) {
		executeCommand(32, 0);
		return true;
	}
	return false;
}

GSound1::GSound1(Audio::Mixer *mixer) :
		GSoundDragonsphere(mixer, kDriver1, kCommands1,
				ARRAYSIZE(kCommands1)) {
}
GSound2::GSound2(Audio::Mixer *mixer) :
		GSoundDragonsphere(mixer, kDriver2, kCommands2,
				ARRAYSIZE(kCommands2)) {
}
GSound3::GSound3(Audio::Mixer *mixer) :
		GSoundDragonsphere(mixer, kDriver3, kCommands3,
				ARRAYSIZE(kCommands3)) {
}
GSound4::GSound4(Audio::Mixer *mixer) :
		GSoundDragonsphere(mixer, kDriver4, kCommands4,
				ARRAYSIZE(kCommands4)) {
}
GSound5::GSound5(Audio::Mixer *mixer) :
		GSoundDragonsphere(mixer, kDriver5, kCommands5,
				ARRAYSIZE(kCommands5)) {
}
GSound6::GSound6(Audio::Mixer *mixer) :
		GSoundDragonsphere(mixer, kDriver6, kCommands6,
				ARRAYSIZE(kCommands6)) {
}
GSound9::GSound9(Audio::Mixer *mixer) :
		GSoundDragonsphere(mixer, kDriver9, kCommands9,
				ARRAYSIZE(kCommands9)) {
}

#undef D4
#undef M8
#undef M7
#undef M6
#undef M5
#undef M4
#undef M3
#undef M2
#undef M1
#undef DFLAGS
#undef MFLAGS
#undef F3
#undef F2
#undef F1
#undef X
#undef N
#undef S
#undef R

} // namespace Sound
} // namespace Dragonsphere
} // namespace MADS
