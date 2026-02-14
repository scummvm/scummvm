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

#include "engines/freescape/games/driller/c64.music.h"

#include "common/textconsole.h"
#include "common/util.h"

// --- Driller Music Data (Embedded from Disassembly) ---
namespace Freescape {

// Frequency Tables (0x0D53 - 0x0E12)
const uint8_t frq_lo[96] = {
	0x0C, 0x1C, 0x2D, 0x3E, 0x51, 0x66, 0x7B, 0x91, 0xA9, 0xC3, 0xDD, 0xFA, 0x18, 0x38, 0x5A, 0x7D, // 0D53
	0xA3, 0xCC, 0xF6, 0x23, 0x53, 0x86, 0xBB, 0xF4, 0x30, 0x70, 0xB4, 0xFB, 0x47, 0x98, 0xED, 0x47, // 0D63
	0xA7, 0x0C, 0x77, 0xE9, 0x61, 0xE1, 0x68, 0xF7, 0x8F, 0x30, 0xDA, 0x8F, 0x4E, 0x18, 0xEF, 0xD2, // 0D73
	0xC3, 0xC3, 0xD1, 0xEF, 0x1F, 0x60, 0xB5, 0x1E, 0x9C, 0x31, 0xDF, 0xA5, 0x87, 0x86, 0xA2, 0xDF, // 0D83
	0x3E, 0xC1, 0x6B, 0x3C, 0x39, 0x63, 0xBE, 0x4B, 0x0F, 0x0C, 0x45, 0xBF, 0x7D, 0x83, 0xD6, 0x79, // 0D93
	0x73, 0xC7, 0x7C, 0x97, 0x1E, 0x18, 0x8B, 0x7E, 0xFA, 0x06, 0xAC, 0xF3, 0xE6, 0x8F, 0xF8, 0x2E  // 0DA3
};
const uint8_t frq_hi[96] = {
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, // 0DB3
	0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x06, // 0DC3
	0x06, 0x07, 0x07, 0x07, 0x08, 0x08, 0x09, 0x09, 0x0A, 0x0B, 0x0B, 0x0C, 0x0D, 0x0E, 0x0E, 0x0F, // 0DD3
	0x10, 0x11, 0x12, 0x13, 0x15, 0x16, 0x17, 0x19, 0x1A, 0x1C, 0x1D, 0x1F, 0x21, 0x23, 0x25, 0x27, // 0DE3
	0x2A, 0x2C, 0x2F, 0x32, 0x35, 0x38, 0x3B, 0x3F, 0x43, 0x47, 0x4B, 0x4F, 0x54, 0x59, 0x5E, 0x64, // 0DF3
	0x6A, 0x70, 0x77, 0x7E, 0x86, 0x8E, 0x96, 0x9F, 0xA8, 0xB3, 0xBD, 0xC8, 0xD4, 0xE1, 0xEE, 0xFD  // 0E03
};

// Instrument Data (0x0EA5 - 0x1004) - Stored as flat arrays
// possibly_instrument_a0
const uint8_t instrumentDataA0[] = {
	0x00, 0x81, 0x0A, 0x00, 0x00, 0x00, 0x80, 0x01, // Inst 0
	0x90, 0x41, 0xFE, 0x0D, 0x25, 0x00, 0x40, 0x02, // Inst 1
	0x00, 0x81, 0xFD, 0x00, 0x00, 0x00, 0x80, 0x00, // Inst 2
	0x30, 0x41, 0x0E, 0x00, 0x30, 0x00, 0x40, 0x02, // Inst 3
	0x96, 0x41, 0x0E, 0x00, 0xA0, 0x00, 0x40, 0x02, // Inst 4
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Inst 5
	0x32, 0x41, 0x00, 0x40, 0xF0, 0x00, 0x40, 0x02, // Inst 6
	0x00, 0x81, 0x08, 0x00, 0x00, 0x00, 0x80, 0x01, // Inst 7
	0x00, 0x11, 0x0D, 0x00, 0x00, 0x00, 0x10, 0x00, // Inst 8
	0x90, 0x41, 0x0E, 0x00, 0x25, 0x00, 0x40, 0x02, // Inst 9
	0x2E, 0x43, 0x00, 0x60, 0xF5, 0x00, 0x40, 0x04, // Inst 10
	0x70, 0x41, 0x0A, 0x00, 0x40, 0x00, 0x40, 0x02, // Inst 11
	0x00, 0x15, 0x03, 0x00, 0x00, 0x20, 0x14, 0x04, // Inst 12
	0x40, 0x41, 0x00, 0x90, 0x01, 0x00, 0x40, 0x00, // Inst 13
	0x00, 0x15, 0xEE, 0x00, 0x00, 0x00, 0x14, 0x00, // Inst 14
	0x98, 0x41, 0x09, 0x00, 0x00, 0x00, 0x40, 0x01, // Inst 15
	0x21, 0x41, 0x0A, 0x00, 0x30, 0x00, 0x40, 0x06, // Inst 16 ('q')
	0x21, 0x41, 0x0A, 0x00, 0x30, 0x00, 0x40, 0x06, // Inst 17 ('r')
	0x31, 0x41, 0x0E, 0x00, 0x10, 0x00, 0x40, 0x02, // Inst 18 ('s')
	0x23, 0x41, 0x00, 0xA0, 0x50, 0x00, 0x40, 0x00, // Inst 19 ('t')
	0x91, 0x41, 0x0A, 0x00, 0x30, 0x00, 0x40, 0x06, // Inst 20 ('u')
	0xF1, 0x41, 0x0C, 0x00, 0x40, 0x00, 0x40, 0x06, // Inst 21 ('v')
};
// possibly_instrument_a1
const uint8_t instrumentDataA1[] = {
	0x00, 0x00, 0x11, 0x00, 0x00, 0x03, 0x00, 0x00, // Inst 0
	0x00, 0x00, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, // Inst 1
	0x06, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Inst 2
	0x30, 0x02, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, // Inst 3
	0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Inst 4
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Inst 5
	0x00, 0x00, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, // Inst 6
	0x00, 0x00, 0x11, 0x41, 0x01, 0x01, 0x00, 0x00, // Inst 7
	0x50, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Inst 8
	0x00, 0x00, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, // Inst 9
	0x20, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Inst 10
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Inst 11
	0x00, 0x00, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, // Inst 12
	0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Inst 13
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Inst 14
	0x00, 0x00, 0x41, 0xF0, 0x01, 0x01, 0x00, 0x00, // Inst 15
	0x10, 0x02, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00, // Inst 16 ('q')
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Inst 17 ('r')
	0xA0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Inst 18 ('s')
	0x60, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Inst 19 ('t')
	0x00, 0x00, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00, // Inst 20 ('u')
	0x0A, 0x02, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00, // Inst 21 ('v')
};
const int NUM_INSTRUMENTS = sizeof(instrumentDataA0) / 8;

// Arpeggio Data (0x157A - 0x157E)
const uint8_t arpeggio_data[] = {0x00, 0x0C, 0x18};
// Only one arpeggio table is defined (3 entries: +0, +12, +24 semitones)

// Music Data Pointers and Structures
// Need to load the actual PRG file into a buffer (_musicData)
// For simplicity here, we'll define the structures based on the disassembly
// In a real implementation, these would point into _musicData

// Tracks (0x1057 - 0x118A)
const uint8_t voice1_track_data[] = {0x01, 0x01, 0x07, 0x09, 0x09, 0x09, 0x01, 0x07, 0x07, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x03, 0x03, 0x0F, 0x0F, 0x13, 0x13, 0x0F, 0x13, 0x0F, 0x13, 0x0F, 0x13, 0x0F, 0x13, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x1B, 0x1D, 0x1E, 0x0F, 0x1B, 0x1D, 0x1E, 0x0F, 0x1B, 0x1D, 0x1E, 0x12, 0x12, 0x12, 0x12, 0x24, 0x24, 0x21, 0x21, 0x24, 0x24, 0x21, 0x21, 0x24, 0x24, 0x21, 0x21, 0x24, 0x24, 0x21, 0x21, 0x24, 0x24, 0x21, 0x21, 0x24, 0x24, 0x21, 0x21, 0x24, 0x24, 0x21, 0x21, 0x24, 0x24, 0x21, 0x21, 0x08, 0x08, 0x28, 0x00, 0x00, 0x00, 0x00, 0xFF};
const uint8_t voice2_track_data[] = {0x03, 0x03, 0x08, 0x0A, 0x0D, 0x0D, 0x0D, 0x0D, 0x08, 0x07, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x05, 0x12, 0x12, 0x12, 0x12, 0x14, 0x15, 0x14, 0x15, 0x14, 0x15, 0x14, 0x15, 0x08, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x07, 0x07, 0x1F, 0x1F, 0x1F, 0x1F, 0x07, 0x07, 0x00, 0x00, 0x25, 0x25, 0x26, 0x25, 0x27, 0x27, 0x27, 0x27, 0x27, 0x27, 0x27, 0x27, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x28, 0x00, 0x00, 0x00, 0x00, 0xFF};
const uint8_t voice3_track_data[] = {0x00, 0x00, 0x00, 0x00, 0x04, 0x06, 0x06, 0x0C, 0x0B, 0x0C, 0x0B, 0x0C, 0x0B, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x0F, 0x0F, 0x10, 0x11, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x16, 0x07, 0x07, 0x07, 0x18, 0x19, 0x19, 0x1A, 0x1A, 0x08, 0x08, 0x1C, 0x08, 0x08, 0x23, 0x23, 0x22, 0x22, 0x23, 0x23, 0x22, 0x22, 0x23, 0x23, 0x22, 0x22, 0x23, 0x23, 0x22, 0x22, 0x23, 0x23, 0x22, 0x22, 0x23, 0x23, 0x22, 0x22, 0x23, 0x23, 0x22, 0x22, 0x23, 0x23, 0x22, 0x22, 0x07, 0x07, 0x0F, 0x0F, 0x0F, 0x0F, 0x29, 0x00, 0x00, 0x00, 0x00, 0xFF};

// Patterns (0x118B - 0x1579) - Need to define these based on disassembly
const uint8_t pattern_00[] = {0xFD, 0x3F, 0xFA, 0x04, 0x00, 0xFF};
const uint8_t pattern_01[] = {0xFA, 0x01, 0xFD, 0x3F, 0x23, 0x1F, 0x22, 0x1E, 0xFF};
const uint8_t pattern_03[] = {0xFA, 0x01, 0xFD, 0x3F, 0x17, 0x13, 0x16, 0x12, 0xFF};
const uint8_t pattern_02[] = {0xFD, 0x0F, 0xFA, 0x04, 0x00, 0xFF};
const uint8_t pattern_04[] = {0xFA, 0x02, 0xFD, 0x7F, 0x25, 0x25, 0xFF};
const uint8_t pattern_05[] = {0xFA, 0x0E, 0xFD, 0x3F, 0x2F, 0x2B, 0x2E, 0xFC, 0x20, 0x2A, 0xFF};
const uint8_t pattern_06[] = {0xFA, 0x06, 0xFD, 0x01, 0x42, 0x3B, 0x3B, 0x42, 0x3B, 0x3B, 0x43, 0x3B, 0x42, 0x3B, 0x3B, 0x42, 0x3B, 0x3B, 0x43, 0x3B, 0x42, 0x3B, 0x3B, 0x42, 0x3B, 0x3B, 0x43, 0x3B, 0x42, 0x3B, 0x3B, 0x42, 0x3B, 0x3B, 0x43, 0x3B, 0xFF};
const uint8_t pattern_07[] = {0xFA, 0x01, 0xFD, 0x7F, 0x23, 0xFF};
const uint8_t pattern_08[] = {0xFA, 0x01, 0xFD, 0x7F, 0x17, 0x00, 0xFF};
const uint8_t pattern_09[] = {0xFA, 0x09, 0xFD, 0x1F, 0x17, 0x13, 0x12, 0x0F, 0xFF};
const uint8_t pattern_10[] = {0xFA, 0x08, 0xFD, 0x0F, 0x3E, 0x39, 0xFD, 0x1F, 0x3B, 0xFD, 0x0F, 0x3D, 0x3B, 0xFD, 0x1F, 0x3A, 0xFD, 0x7F, 0xFB, 0x01, 0x2F, 0xFF};
const uint8_t pattern_11[] = {0xFA, 0x06, 0xFD, 0x01, 0x3D, 0x36, 0x36, 0x3D, 0x36, 0x36, 0x3E, 0x36, 0x3D, 0x36, 0x36, 0x3D, 0x36, 0x36, 0x3E, 0x36, 0x3A, 0x33, 0x33, 0x3A, 0x33, 0x33, 0x3B, 0x33, 0x3A, 0x33, 0x33, 0x3A, 0x33, 0x33, 0x3B, 0x33, 0xFF};
const uint8_t pattern_12[] = {0xFA, 0x06, 0xFD, 0x01, 0x42, 0x3B, 0x3B, 0x42, 0x3B, 0x3B, 0x43, 0x3B, 0x42, 0x3B, 0x3B, 0x42, 0x3B, 0x3B, 0x43, 0x3B, 0x3E, 0x37, 0x37, 0x3E, 0x37, 0x37, 0x3F, 0x37, 0x3E, 0x37, 0x37, 0x3E, 0x37, 0x37, 0x3F, 0x37, 0xFF};
const uint8_t pattern_13[] = {0xFA, 0x0A, 0xFD, 0x01, 0x3B, 0x3A, 0x39, 0x38, 0x39, 0x3A, 0x3B, 0x3A, 0x39, 0x38, 0x39, 0x3A, 0x3B, 0x3A, 0x39, 0x38, 0x39, 0x3A, 0x3B, 0x3A, 0x39, 0x38, 0x39, 0x3A, 0x3B, 0x3A, 0x39, 0x38, 0x39, 0x3A, 0x3B, 0x3A, 0xFF};
const uint8_t pattern_14[] = {0xFA, 0x07, 0xFD, 0x01, 0x2D, 0xFD, 0x03, 0x2D, 0xFD, 0x0D, 0x2D, 0xFD, 0x03, 0x2D, 0xFD, 0x07, 0xFA, 0x00, 0x2D, 0xFA, 0x07, 0xFD, 0x01, 0x2D, 0xFD, 0x03, 0x2D, 0xFD, 0x0D, 0x2D, 0xFD, 0x03, 0x2D, 0xFD, 0x07, 0xFA, 0x00, 0x2D, 0xFF};
const uint8_t pattern_15[] = {0xFA, 0x0B, 0xFD, 0x01, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0xFF};
const uint8_t pattern_16[] = {0xFA, 0x0B, 0xFD, 0x01, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0xFF};
const uint8_t pattern_17[] = {0xFA, 0x0B, 0xFD, 0x01, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0xFF};
const uint8_t pattern_18[] = {0xFA, 0x0A, 0xFD, 0x01, 0x3B, 0x37, 0x36, 0x34, 0x3B, 0x37, 0x36, 0x34, 0x3B, 0x37, 0x36, 0x34, 0x3B, 0x37, 0x36, 0x34, 0x3B, 0x37, 0x36, 0x34, 0x3B, 0x37, 0x36, 0x34, 0x3B, 0x37, 0x36, 0x34, 0x3B, 0x37, 0x36, 0x34, 0xFF};
const uint8_t pattern_19[] = {0xFA, 0x0B, 0xFD, 0x01, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0xFF};
const uint8_t pattern_20[] = {0xFA, 0x06, 0xFD, 0x01, 0x3F, 0x3B, 0x36, 0x3F, 0x3B, 0x36, 0x3F, 0x3B, 0x3F, 0x3B, 0x36, 0x3F, 0x3B, 0x36, 0x3F, 0x3B, 0x3F, 0x3B, 0x36, 0x3F, 0x3B, 0x36, 0x3F, 0x3B, 0x3F, 0x3B, 0x36, 0x3F, 0x3B, 0x36, 0x3F, 0x3B, 0xFF};
const uint8_t pattern_21[] = {0xFA, 0x06, 0xFD, 0x01, 0x3E, 0x3B, 0x37, 0x3E, 0x3B, 0x37, 0x3E, 0x3B, 0x3E, 0x3B, 0x37, 0x3E, 0x3B, 0x37, 0x3E, 0x3B, 0x3E, 0x3B, 0x37, 0x3E, 0x3B, 0x37, 0x3E, 0x3B, 0x3E, 0x3B, 0x37, 0x3E, 0x3B, 0x37, 0x3E, 0x3B, 0xFF};
const uint8_t pattern_22[] = {0xFA, 0x0D, 0xFD, 0x1F, 0x37, 0x36, 0x39, 0x37, 0x36, 0x2F, 0x2F, 0x32, 0xFF};
const uint8_t pattern_23[] = {0xFA, 0x10, 0xFD, 0x01, 0x23, 0x23, 0x2A, 0x2A, 0x28, 0x28, 0x2A, 0x2A, 0x26, 0x26, 0x2A, 0x2A, 0x28, 0x28, 0x2A, 0x2A, 0x23, 0x23, 0x2A, 0x2A, 0x28, 0x28, 0x2A, 0x2A, 0x26, 0x26, 0x2A, 0x2A, 0x28, 0x28, 0x2A, 0x2A, 0xFF};
const uint8_t pattern_24[] = {0xFA, 0x13, 0xFD, 0x07, 0xFC, 0x37, 0x45, 0xFD, 0x2F, 0x47, 0xFD, 0x07, 0xFB, 0x7F, 0x47, 0xFD, 0x37, 0x42, 0xFD, 0x07, 0xFB, 0x80, 0x42, 0xFF};
const uint8_t pattern_25[] = {0xFA, 0x13, 0xFD, 0x1F, 0x3B, 0xFD, 0x0F, 0x39, 0x37, 0xFD, 0x3F, 0x36, 0xFF};
const uint8_t pattern_26[] = {0xFA, 0x13, 0xFD, 0x1F, 0x34, 0xFD, 0x0F, 0x32, 0x31, 0xFD, 0x3F, 0x2F, 0xFF};
const uint8_t pattern_27[] = {0xFA, 0x0B, 0xFD, 0x01, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0xFF};
const uint8_t pattern_28[] = {0xFA, 0x01, 0xFD, 0x1F, 0x3B, 0xFD, 0x0F, 0x3A, 0x36, 0xFD, 0x2F, 0x36, 0xFD, 0x0F, 0x38, 0xFD, 0x1F, 0x38, 0x2F, 0x31, 0xFD, 0x0F, 0x33, 0x34, 0xFD, 0x7F, 0x36, 0x36, 0xFF};
const uint8_t pattern_29[] = {0xFA, 0x0B, 0xFD, 0x01, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0xFF};
const uint8_t pattern_30[] = {0xFA, 0x0B, 0xFD, 0x01, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0xFF};
const uint8_t pattern_31[] = {0xFA, 0x09, 0xFD, 0x3F, 0x23, 0x1B, 0x1C, 0x1E, 0xFF};
const uint8_t pattern_32[] = {0xFA, 0x01, 0xFD, 0x7F, 0x17, 0x17, 0xFF}; // Note: Simplified, removed data after FF
const uint8_t pattern_33[] = {0xFA, 0x15, 0xFD, 0x01, 0x1F, 0x1F, 0xFD, 0x03, 0x1F, 0xFA, 0x0F, 0xFD, 0x01, 0x2E, 0x27, 0xFA, 0x15, 0x1F, 0xFD, 0x03, 0x1F, 0xFD, 0x01, 0x1F, 0xFD, 0x03, 0x1F, 0xFD, 0x01, 0xFA, 0x0F, 0x2F, 0xFA, 0x15, 0x1A, 0x1D, 0x1F, 0xFF};
const uint8_t pattern_34[] = {0xFA, 0x09, 0xFD, 0x01, 0x13, 0x13, 0xFD, 0x03, 0x13, 0xFD, 0x01, 0xFA, 0x00, 0x2E, 0x27, 0xFA, 0x09, 0x13, 0xFD, 0x03, 0x13, 0xFD, 0x01, 0x13, 0xFD, 0x03, 0x13, 0xFD, 0x01, 0x13, 0x10, 0x11, 0x13, 0xFF};
const uint8_t pattern_35[] = {0xFA, 0x09, 0xFD, 0x01, 0x17, 0x17, 0xFD, 0x03, 0x17, 0xFD, 0x01, 0xFA, 0x00, 0x2E, 0x27, 0xFA, 0x09, 0x17, 0xFD, 0x03, 0x17, 0xFD, 0x01, 0x17, 0xFD, 0x03, 0x17, 0xFD, 0x01, 0x17, 0x12, 0x15, 0x17, 0xFF};
const uint8_t pattern_36[] = {0xFA, 0x15, 0xFD, 0x01, 0x23, 0x23, 0xFD, 0x03, 0x23, 0xFA, 0x0F, 0xFD, 0x01, 0x2E, 0x27, 0xFA, 0x15, 0x23, 0xFD, 0x03, 0x23, 0xFD, 0x01, 0x23, 0xFD, 0x03, 0x23, 0xFD, 0x01, 0xFA, 0x0F, 0x2F, 0xFA, 0x15, 0x1E, 0x21, 0x23, 0xFF};
const uint8_t pattern_37[] = {0xFA, 0x0A, 0xFD, 0x39, 0x47, 0xFD, 0x01, 0x46, 0x45, 0x44, 0xFD, 0x39, 0x43, 0xFD, 0x01, 0x44, 0x45, 0x46, 0xFF};
const uint8_t pattern_38[] = {0xFA, 0x12, 0xFD, 0x3F, 0x3B, 0x43, 0x42, 0x3E, 0x3B, 0x37, 0x36, 0x2F, 0xFF};
const uint8_t pattern_39[] = {0xFA, 0x0C, 0xFD, 0x01, 0x31, 0x3D, 0x49, 0x3D, 0x31, 0x3D, 0x49, 0x3D, 0xFF};
const uint8_t pattern_40[] = {0xFA, 0x01, 0xFD, 0x7F, 0x17, 0x00, 0x00, 0x00, 0xFF};
const uint8_t pattern_41[] = {0xFA, 0x01, 0xFD, 0x7F, 0x23, 0x00, 0x00, 0x00, 0xFF};

// Pattern Address Table (0x157F - 0x15D4)
const uint8_t *const pattern_addresses[] = {
	pattern_00, pattern_01, pattern_03, pattern_02, pattern_04, pattern_05, pattern_06, pattern_07,
	pattern_08, pattern_09, pattern_10, pattern_11, pattern_12, pattern_13, pattern_14, pattern_15,
	pattern_16, pattern_17, pattern_18, pattern_19, pattern_20, pattern_21, pattern_22, pattern_23,
	pattern_24, pattern_25, pattern_26, pattern_27, pattern_28, pattern_29, pattern_30, pattern_31,
	pattern_32, pattern_33, pattern_34, pattern_35, pattern_36, pattern_37, pattern_38, pattern_39,
	pattern_40, pattern_41};
const int NUM_PATTERNS = ARRAYSIZE(pattern_addresses);

// Tune Data (0x1054, 0x15D5 - 0x15E5)
const uint8_t tune_tempo_data[] = {0x00, 0x03, 0x03}; // tempos for tune 0, 1, 2
const uint8_t *const tune_track_data[][3] = {
	{nullptr, nullptr, nullptr},                               // Tune 0 (null pointers = stop)
	{voice1_track_data, voice2_track_data, voice3_track_data}, // Tune 1
};
const int NUM_TUNES = ARRAYSIZE(tune_tempo_data);

// SID Base Addresses for Voices
const int voice_sid_offset[] = {0, 7, 14};

// Debug log levels
#define DEBUG_LEVEL 4 // 0: Minimal, 1: Basic Flow, 2: Detailed State

DrillerSIDPlayer::DrillerSIDPlayer() : _sid(nullptr),
														  _playState(STOPPED),
														  _targetTuneIndex(0),
														  _globalTempo(3),        // Default tempo
														  _globalTempoCounter(1)  // Start immediately
{
	initSID();

	debug(DEBUG_LEVEL >= 1, "Driller SID Player Initialized");
}

DrillerSIDPlayer::~DrillerSIDPlayer() {
	if (_sid) {
		_sid->stop();
		delete _sid;
	}

	debug(DEBUG_LEVEL >= 1, "Driller SID Player Destroyed");
}

// Tune 0 seems unused, Tune 1 is the main theme
void DrillerSIDPlayer::startMusic(int tuneIndex) {
	if (tuneIndex < 0 || tuneIndex >= NUM_TUNES) {
		debug(DEBUG_LEVEL >= 0, "Driller: Invalid tune index %d requested", tuneIndex);
		return;
	}
	debug(DEBUG_LEVEL >= 0, "Driller: Starting Tune %d", tuneIndex);
	_targetTuneIndex = tuneIndex;
	// Signal to change tune on the next frame update
	// If stopped, this will trigger initialization. If playing, triggers change.
	_playState = CHANGING_TUNE;
}

void DrillerSIDPlayer::stopMusic() {
	debug(DEBUG_LEVEL >= 0, "Driller: Stopping Music");
	_playState = STOPPED;
	// Mute SID channels immediately
	if (_sid) {                // Check if SID exists
		SID_Write(0x18, 0x00); // Master volume off
		for (int i = 0; i < 3; ++i) {
			int offset = voice_sid_offset[i];
			SID_Write(offset + 4, 0); // Gate off, keep waveform bits
		}
	}
}

// --- SID Interaction ---
void DrillerSIDPlayer::SID_Write(int reg, uint8_t data) {
	if (_sid) {
		debug(DEBUG_LEVEL >= 3, "SID Write: Reg $%02X = $%02X", reg, data);
		_sid->writeReg(reg, data);
	}
}

void DrillerSIDPlayer::initSID() {
	if (_sid) {
		_sid->stop();
		delete _sid; // Delete previous instance if any
	}

	_sid = SID::Config::create(SID::Config::kSidPAL);
	if (!_sid || !_sid->init())
		error("Failed to initialise SID emulator");

	// Reset SID registers (like 0x0910 - reset_voices)
	SID_Write(0x04, 0);    // V1 Ctrl = 0
	SID_Write(0x0B, 0);    // V2 Ctrl = 0
	SID_Write(0x12, 0);    // V3 Ctrl = 0
	SID_Write(0x15, 0);    // Filter Cutoff Lo = 0
	SID_Write(0x16, 0);    // Filter Cutoff Hi = 0
	SID_Write(0x17, 0);    // Filter Res/Ctrl = 0
	SID_Write(0x18, 0x0F); // Volume & Filter Mode = Max Volume

	_sid->start(new Common::Functor0Mem<void, DrillerSIDPlayer>(this, &DrillerSIDPlayer::onTimer), 50);
}

// --- Player Logic (Called once per C64 frame) ---
void DrillerSIDPlayer::onTimer() {
	// Handle global state changes first (STOPPED, CHANGING_TUNE)
	if (_playState == STOPPED) {
		debug(DEBUG_LEVEL >= 2, "Driller: Frame - Music Stopped");
		// Ensure volume is off if truly stopped (redundant with stopMusic, but safe)
		// SID_Write(0x18, 0x00); // Master volume 0
		return;
	}

	if (_playState == CHANGING_TUNE) {
		debug(DEBUG_LEVEL >= 1, "Driller: Frame - Changing Tune");
		// Corresponds to jmp change_tune
		if (_targetTuneIndex == 0) { // Tune 0 means stop
			stopMusic();
			return;
		}
		handleChangeTune(_targetTuneIndex);
		_playState = PLAYING;
		// Fall through to process the first frame of the new tune
	}
	// debug(DEBUG_LEVEL >= 2, "Driller: Frame - Playing"); // Too verbose

	// Corresponds to the main part of music_play / play_voice loop
	// cmp #$AB; beq continue_playing
	// We are now in the PLAYING state

	// Process each voice (0x0E46 - 0x0E55)
	for (int voiceIndex = 0; voiceIndex < 3; ++voiceIndex) {
		playVoice(voiceIndex);
	}

	// Corresponds to voice_done (0x09A1)
	// After all 3 voices processed (cpx #$0E), handle tempo counter once per frame
	// dec tempo_ctr (0x09A5)
	_globalTempoCounter--;

	// bpl @done (0x09A8)
	if (_globalTempoCounter < 0) {
		// lda tempo; sta tempo_ctr (0x09AA)
		_globalTempoCounter = _globalTempo;
	}
}

// --- Tune Loading ---
void DrillerSIDPlayer::handleChangeTune(int tuneIndex) {
	// Corresponds to change_tune (0x0971)
	debug(DEBUG_LEVEL >= 1, "Driller: Changing to Tune %d", tuneIndex);
	if (tuneIndex < 1 || tuneIndex >= NUM_TUNES) {
		debug(DEBUG_LEVEL >= 0, "Driller: Invalid tune index %d in handleChangeTune, using 1", tuneIndex);
		tuneIndex = 1; // Default to tune 1 if invalid
	}

	// *** ADD THIS LOG - BEFORE ASSIGNMENT ***
	debug(DEBUG_LEVEL >= 1, "Driller: Tune %d - Accessing tune_track_data[%d]...", tuneIndex, tuneIndex);

	const uint8_t *const *currentTuneTracks = tune_track_data[tuneIndex];

	// *** ADD THIS LOG - AFTER ACCESSING THE TUNE'S TRACK ARRAY ***
	// Check if the pointer to the array itself is valid
	if (!currentTuneTracks) {
		debug(DEBUG_LEVEL >= 0, "Driller: FATAL - tune_track_data[%d] is NULL!", tuneIndex);
		// Optional: Handle this error more gracefully, maybe stop music?
	} else {
		debug(DEBUG_LEVEL >= 2, "Driller: tune_track_data[%d] pointer is valid.", tuneIndex);
	}

	for (int i = 0; i < 3; ++i) {
		// *** ADD THIS LOG - BEFORE ASSIGNING TO voiceState ***
		const uint8_t *trackPtr = nullptr; // Temp variable
		if (currentTuneTracks) {           // Check if the tune array pointer is valid
			trackPtr = currentTuneTracks[i];
			debug(DEBUG_LEVEL >= 1, "Driller: V%d - Got track pointer %p from currentTuneTracks[%d]", i, (const void *)trackPtr, i);
		} else {
			debug(DEBUG_LEVEL >= 0, "Driller: V%d - Cannot get track pointer because currentTuneTracks is NULL", i);
		}

		// Assign the pointer
		_voiceState[i].trackDataPtr = trackPtr;

		// *** ADD THIS LOG - AFTER ASSIGNING TO voiceState ***
		debug(DEBUG_LEVEL >= 1, "Driller: V%d - Assigned _voiceState[%d].trackDataPtr = %p", i, i, (const void *)_voiceState[i].trackDataPtr);

		if (!_voiceState[i].trackDataPtr) {
			// This block now just confirms the assignment result
			debug(DEBUG_LEVEL >= 1, "Driller: Voice %d has null track data assigned for tune %d.", i, tuneIndex);
			// Don't reset here, handleResetVoices will do it.
		}
		// Pointers setup in resetVoices below
	}

	_globalTempo = tune_tempo_data[tuneIndex];
	if (_globalTempo == 0)
		_globalTempo = 1; // Avoid infinite loop

	// Corresponds to jmp reset_voices (0x099E)
	handleResetVoices(); // This call should happen AFTER the pointers are assigned
}

void DrillerSIDPlayer::handleResetVoices() {
	// Corresponds to reset_voices (0x0910)
	debug(DEBUG_LEVEL >= 1, "Driller: Resetting Voices");
	SID_Write(0x04, 0);    // V1 Ctrl = 0
	SID_Write(0x0B, 0);    // V2 Ctrl = 0
	SID_Write(0x12, 0);    // V3 Ctrl = 0
	SID_Write(0x18, 0x0F); // Volume Max

	for (int i = 0; i < 3; ++i) {
		// *** DO NOT CALL _voiceState[i].reset() HERE ***
		// The trackDataPtr was just assigned in handleChangeTune.
		// Reset only the playback state relevant for starting a tune/track.

		debug(DEBUG_LEVEL >= 1, "Driller: Reset Voice %d - Checking _voiceState[%d].trackDataPtr (%p)...", i, i, (const void *)_voiceState[i].trackDataPtr);

		if (_voiceState[i].trackDataPtr != nullptr) {
			debug(DEBUG_LEVEL >= 1, "Driller: Reset Voice %d - Track pointer OK. Initializing playback state.", i);

			// Reset playback state, keep trackDataPtr
			_voiceState[i].trackIndex = 0;
			_voiceState[i].patternDataPtr = nullptr; // Will be set by pattern lookup
			_voiceState[i].patternIndex = 0;
			_voiceState[i].instrumentIndex = 0; // Default instrument? Or should tune load set this? Let's keep 0.
			_voiceState[i].delayCounter = -1;   // Ready for first note
			_voiceState[i].noteDuration = 0;
			_voiceState[i].gateMask = 0xFF;
			_voiceState[i].currentNote = 0;
			_voiceState[i].portaTargetNote = 0;
			_voiceState[i].currentFreq = 0;
			_voiceState[i].baseFreq = 0;
			_voiceState[i].targetFreq = 0;
			_voiceState[i].pulseWidth = 0;     // Default PW?
			_voiceState[i].attackDecay = 0x00; // Default ADSR?
			_voiceState[i].sustainRelease = 0x00;
			_voiceState[i].effect = 0;
			_voiceState[i].hardRestartActive = false;
			_voiceState[i].waveform = 0x10; // Default waveform (Triangle)
			_voiceState[i].keyOn = false;
			_voiceState[i].currentNoteSlideTarget = 0;
			_voiceState[i].glideDownTimer = 0; // Reset glide timer

			// Reset other potentially problematic state variables from the struct
			_voiceState[i].whatever0 = 0;
			_voiceState[i].whatever1 = 0;
			_voiceState[i].whatever2 = 0;
			_voiceState[i].whatever3 = 0;
			_voiceState[i].whatever4 = 0;
			_voiceState[i].whatever2_vibDirToggle = 0;
			_voiceState[i].portaStepRaw = 0;
			memset(_voiceState[i].something_else, 0, sizeof(_voiceState[i].something_else));
			_voiceState[i].ctrl0 = 0;
			_voiceState[i].arpTableIndex = 0;
			_voiceState[i].arpSpeedHiNibble = 0;
			_voiceState[i].stuff_freq_porta_vib = 0;
			_voiceState[i].stuff_freq_base = 0;
			_voiceState[i].stuff_freq_hard_restart = 0;
			_voiceState[i].stuff_arp_counter = 0;
			_voiceState[i].stuff_arp_note_index = 0;
			_voiceState[i].things_vib_state = 0;
			_voiceState[i].things_vib_depth = 0;
			_voiceState[i].things_vib_delay_reload = 0;
			_voiceState[i].things_vib_delay_ctr = 0;
			_voiceState[i].portaSpeed = 0;

		} else {
			debug(DEBUG_LEVEL >= 0, "Driller: Reset Voice %d - Check FAILED. trackDataPtr is NULL here!", i);
			// Ensure voice is silent if no track data
			int sidOffset = voice_sid_offset[i];
			SID_Write(sidOffset + 4, 0); // Gate off
		}
	}

	// Reset global tempo counter (0x093D)
	_globalTempoCounter = 1; // Start processing immediately
}

// --- Voice Processing ---
void DrillerSIDPlayer::playVoice(int voiceIndex) {
	// debug(DEBUG_LEVEL >= 2, "Driller: Processing Voice %d", voiceIndex);
	VoiceState &v = _voiceState[voiceIndex];
	int sidOffset = voice_sid_offset[voiceIndex];

	// If track data is null, this voice is inactive for the current tune
	if (!v.trackDataPtr) {
		return;
	}

	// --- Effect application before note processing (Tempo independent) ---
	// Corresponds roughly to L0944 - L0964 (instrument specific effects)
	// And L0B33 onwards (general effects like vibrato, portamento, arpeggio)

	int instBase = v.instrumentIndex; // Already scaled by 8
	// Safety check for instrument index
	if (instBase < 0 || (size_t)instBase >= sizeof(instrumentDataA0)) {
		instBase = 0; // Default to instrument 0 if invalid
		v.instrumentIndex = 0;
	}
	const uint8_t *instA0 = &instrumentDataA0[instBase];
	const uint8_t *instA1 = &instrumentDataA1[instBase];

	// Waveform transition effect (L0944-L095E) - Inst A0[7] & 0x04
	// This logic updates ctrl register $D404, likely wave or gate
	if (instA0[7] & 0x04) {
		if (v.glideDownTimer > 0) { // voice1_two_ctr,x (0xD3E)
			v.glideDownTimer--;
			uint8_t ctrlVal = instA1[2]; // possibly_instrument_a1+2,y
			SID_Write(sidOffset + 4, ctrlVal);
			// bne L0964 - skip waveform reset if timer > 0
		} else {
			// L095E: timer is 0
			uint8_t ctrlVal = instA0[1]; // possibly_instrument_a0+1,y
			SID_Write(sidOffset + 4, ctrlVal);
			// Resets waveform/gate based on inst A0[1]
		}
	}

	// Corresponds to lda tempo_ctr; bne L096E (0x0964)
	// The per-voice processing is gated by the global tempo counter being zero.
	if (_globalTempoCounter == 0) {
		// dec voice1_ctrl2,x (0x0969)
		if (v.delayCounter >= 0) {
			v.delayCounter--;
		}

		// If delay counter has expired, read new data from the pattern.
		if (v.delayCounter < 0) {
			debug(DEBUG_LEVEL >= 1, "Driller V%d: Delay Counter Expired - Reading new pattern data", voiceIndex);

			// --- Start of inlined pattern reading logic ---
			// Get current pattern index from track (09C0-09CE)
			uint8_t patternNum = v.trackDataPtr[v.trackIndex];

			// Handle track end/loop markers (0AE7, 0AF2)
			if (patternNum == 0xFF) { // End of track list
				debug(DEBUG_LEVEL >= 1, "Driller V%d: Track %d end marker (FF), looping.", voiceIndex, v.trackIndex);
				v.trackIndex = 0; // Loop to start
				patternNum = v.trackDataPtr[v.trackIndex];
				if (patternNum == 0xFF || patternNum == 0xFE || !tune_track_data[_targetTuneIndex][voiceIndex]) { // Check again after loop or if track is null initially
					debug(DEBUG_LEVEL >= 0, "Driller V%d: Stopping music after track loop (FF/FE/Null).", voiceIndex);
					stopMusic(); // Stop if loop points to end marker or track is invalid
					return;
				}
			} else if (patternNum == 0xFE) { // Stop playback command
				debug(DEBUG_LEVEL >= 0, "Driller V%d: Stopping music due to track marker FE.", voiceIndex);
				stopMusic();
				return;
			}

			if (patternNum >= NUM_PATTERNS) {
				debug(DEBUG_LEVEL >= 0, "Driller V%d: Invalid pattern number %d at track index %d", voiceIndex, patternNum, v.trackIndex);
				v.trackIndex++; // Skip invalid entry
				// Fetch next pattern number immediately to avoid getting stuck in invalid state for a frame
				size_t trackSize = (voiceIndex == 0) ? sizeof(voice1_track_data) : ((voiceIndex == 1) ? sizeof(voice2_track_data) : sizeof(voice3_track_data));
				if (v.trackIndex >= trackSize) { // Check for track end
					debug(DEBUG_LEVEL >= 0, "Driller V%d: Stopping music, track index out of bounds after skipping invalid pattern.", voiceIndex);
					stopMusic();
					return;
				}
				patternNum = v.trackDataPtr[v.trackIndex];
				if (patternNum == 0xFF || patternNum == 0xFE) {
					debug(DEBUG_LEVEL >= 0, "Driller V%d: Stopping music, encountered FF/FE after skipping invalid pattern.", voiceIndex);
					stopMusic();
					return;
				}
				if (patternNum >= NUM_PATTERNS) { // Still invalid? Stop.
					debug(DEBUG_LEVEL >= 0, "Driller V%d: Stopping music, encountered second invalid pattern.", voiceIndex);
					stopMusic();
					return;
				}
				// Continue with the new valid patternNum
			}

			// Only update pattern pointer if it changed or wasn't set
			if (v.patternDataPtr != pattern_addresses[patternNum]) {
				v.patternDataPtr = pattern_addresses[patternNum];
				v.patternIndex = 0; // Reset index when pattern changes
				debug(DEBUG_LEVEL >= 2, "Driller V%d: Switched to Pattern %d", voiceIndex, patternNum);
			}

			// Reset state related to previous note/effects for gate control
			v.gateMask = 0xFF; // Reset gate mask (0x09D0: lda #$FF; sta control3)
			v.whatever0 = 0;   // Reset effect states (0x09D5 onwards)
			v.whatever1 = 0;
			v.whatever2 = 0;

			// --- Read Pattern Data Loop (0x09E0 read_note_or_ctrl) ---
			bool noteProcessed = false;
			while (!noteProcessed) {
				if (!v.patternDataPtr) { // Safety check
					debug(DEBUG_LEVEL >= 0, "Driller V%d: Pattern pointer is null!", voiceIndex);
					v.trackIndex++;       // Advance track to avoid getting stuck
					noteProcessed = true; // Exit loop, try next track index next frame
					break;
				}

				// Check pattern bounds - Use FF as terminator
				if (v.patternIndex >= 255) { // Sanity check pattern length
					debug(DEBUG_LEVEL >= 0, "Driller V%d: Pattern index overflow (>255), resetting.", voiceIndex);
					v.patternIndex = 0;   // Reset pattern index
					v.trackIndex++;       // Advance track index
					noteProcessed = true; // Exit loop
					break;                // Go to next track entry
				}

				uint8_t cmd = v.patternDataPtr[v.patternIndex];
				debug(DEBUG_LEVEL >= 3, "Driller V%d: Reading Pat %d Idx %d: Cmd $%02X", voiceIndex, patternNum, v.patternIndex, cmd);

				if (cmd == 0xFF) { // End of pattern marker (0x0AD6)
					debug(DEBUG_LEVEL >= 2, "Driller V%d: End of Pattern %d detected.", voiceIndex, patternNum);
					v.patternIndex = 0;   // Reset pattern index
					v.trackIndex++;       // Advance track index (0x0ADF)
					noteProcessed = true; // Exit inner loop, done processing for this tick
					break;                // Exit pattern loop, next tick will get next pattern index from track
				}

				if (cmd >= 0xFD) {                                                       // --- Control Commands ---
					v.patternIndex++;                                                    // Consume command byte
					if (!v.patternDataPtr || v.patternDataPtr[v.patternIndex] == 0xFF) { // Check bounds before reading data
						debug(DEBUG_LEVEL >= 1, "Driller V%d: Pattern ended unexpectedly after Fx command.", voiceIndex);
						noteProcessed = true;
						break;
					}
					uint8_t dataByte = v.patternDataPtr[v.patternIndex]; // Read data byte

					// Effect FD/FE: Set Note Duration (0x09E5 + 0x09ED)
					// Any command >= FD that is not FF (end of pattern) sets the duration.
					v.noteDuration = dataByte; // Store duration (0x09EF)
					debug(DEBUG_LEVEL >= 2, "Driller V%d: Cmd $%02X, Set Duration = %d", voiceIndex, cmd, v.noteDuration);

					// Continue reading pattern (next_note_or_ctrl 09F2/0A15)
					v.patternIndex++;

				} else if (cmd >= 0xFB) {                                                // Effect FB/FC
					v.patternIndex++;                                                    // Consume command byte
					if (!v.patternDataPtr || v.patternDataPtr[v.patternIndex] == 0xFF) { // Check bounds before reading data
						debug(DEBUG_LEVEL >= 1, "Driller V%d: Pattern ended unexpectedly after FB/FC command.", voiceIndex);
						noteProcessed = true;
						break;
					}
					uint8_t portaParam = v.patternDataPtr[v.patternIndex]; // Consume data byte

					// FB = porta type 1 (down lo), FC = porta type 2 (up lo)
					// Assembly: FB -> lda #$01 (0x09FF), FC -> lda #$02 (0x0A17)
					if (cmd == 0xFB) {
						v.whatever2 = 1; // (0x09FF: lda #$01; sta whatever+2)
					} else { // FC
						v.whatever2 = 2; // (0x0A17: lda #$02)
					}
					v.portaStepRaw = portaParam; // sta voice1_something (0x0A0A)
					v.whatever1 = 0;             // sta whatever+1 (0x0A0F)
					v.whatever0 = 0;             // sta whatever (0x0A12)
					v.portaSpeed = 0;            // Force recalc
					v.patternIndex++; // Continue reading pattern (0A15)

				} else if (cmd == 0xFA) { // --- Effect FA: Set Instrument --- (0x0A1B)
					v.patternIndex++;
					if (!v.patternDataPtr || v.patternDataPtr[v.patternIndex] == 0xFF) { // Check bounds before reading data
						debug(DEBUG_LEVEL >= 1, "Driller V%d: Pattern ended unexpectedly after FA command.", voiceIndex);
						noteProcessed = true;
						break;
					}
					uint8_t instNum = v.patternDataPtr[v.patternIndex];
					if (instNum >= NUM_INSTRUMENTS) {
						debug(DEBUG_LEVEL >= 0, "Driller V%d: Invalid instrument number %d, using 0.", voiceIndex, instNum);
						instNum = 0;
					}
					v.instrumentIndex = instNum * 8; // Store base offset (0A28)
					debug(DEBUG_LEVEL >= 2, "Driller V%d: Cmd FA, Set Instrument = %d", voiceIndex, instNum);

					// Update local pointers for instrument data
					instBase = v.instrumentIndex;
					if (instBase < 0 || (size_t)instBase >= sizeof(instrumentDataA0))
						instBase = 0; // Bounds check
					instA0 = &instrumentDataA0[instBase];
					instA1 = &instrumentDataA1[instBase];

					// Set ADSR based on instrument (0A2C - 0A3E)
					uint8_t adsrByte = instA0[0];       // 0A2C
					v.sustainRelease = adsrByte & 0x0F; // Low nibble to SR (0A32) -> ctrl0
					v.attackDecay = adsrByte & 0xF0;    // High nibble to AD (0A3B/0A3E) -> something_else[0/1]
					// Store in voice state for SID write later
					v.ctrl0 = v.sustainRelease;
					v.something_else[0] = v.attackDecay;
					v.something_else[1] = v.attackDecay; // Seems duplicated in disassembly?
					// Also set PW from instA0[0]? Disassembly sets something_else[0] and [1] to AD (hi nibble)
					// Pulse width seems set later from something_else[0] and [2] ? Let's use [0] for AD.
					// Let's assume instA0[2] (often xx) and instA0[3] (often 00) are PW lo/hi nibble?
					// Or maybe something_else[0]/[2] ARE PW and ADSR needs separate vars?
					// Revisit PW setting in applyNote based on L0AC2. It uses something_else[0] and [2].
					// Let's store ADSR in dedicated vars, and use something_else for PW based on instrument.
					// What part of instrument sets PW? L0AC2 uses something_else[0/2]. FA command sets something_else[0/1/2].
					// FA: pla -> and #F0 -> sta something_else[0] / [1]
					// FA: pha -> and #0F -> sta something_else[2] / ctrl0
					// This means: AD Hi Nibble -> PW Lo Byte? AD Hi Nibble -> something_else[1]? SR Lo Nibble -> PW Hi Nibble? SR Lo Nibble -> ctrl0?
					// Let's follow the variable names:
					v.attackDecay = instA0[0] & 0xF0;    // Stored in something_else[0] & [1]
					v.sustainRelease = instA0[0] & 0x0F; // Stored in something_else[2] & ctrl0
					v.something_else[0] = v.attackDecay;
					v.something_else[1] = v.attackDecay;    // ???
					v.something_else[2] = v.sustainRelease; // PW Hi?
					v.ctrl0 = v.sustainRelease;             // SR?

					debug(DEBUG_LEVEL >= 3, "Driller V%d: Inst %d - ADSR Byte: $%02X -> AD: $%02X, SR: $%02X", voiceIndex, instNum, adsrByte, v.attackDecay, v.sustainRelease);

					// Continue reading pattern (0A41 -> 09F2)
					v.patternIndex++;

				} else {                 // --- Plain Note --- (0x0A1D -> 0A44)
					v.currentNote = cmd; // Store note value (0A44: sta stuff+3)
					// Set delay counter based on previously read duration (0A47-0A4A)
					v.delayCounter = v.noteDuration;

					// Reset hard restart counters (0A4D-0A52)
					v.whatever3 = 0;
					v.whatever4 = 0;

					// Reset glide down timer (0A55-0A57)
					v.glideDownTimer = 2; // voice1_two_ctr = 2

					// Apply Note Data (0A5D-0AB3)
					applyNote(v, sidOffset, instA0, instA1, voiceIndex);

					// Continue reading pattern
					v.patternIndex++;
					noteProcessed = true;
				}

			} // End while(!noteProcessed)
			// --- End of inlined pattern reading logic ---

			// L0AFC: Post-note effect setup - determine which continuous effect is active
			postNoteEffectSetup(v, sidOffset, instA0, instA1);
		}
	}

	// ALWAYS apply continuous effects (L0B33+) for the current state of the voice.
	// This runs every frame: on tempo ticks after note processing, and on non-tempo ticks directly.
	applyContinuousEffects(v, sidOffset, instA0, instA1);
}

// --- Note Application ---
// Corresponds to @plain_note (0x0A44) through L0AAD (0x0AB3)
void DrillerSIDPlayer::applyNote(VoiceState &v, int sidOffset, const uint8_t *instA0, const uint8_t *instA1, int voiceIndex) {
	uint8_t note = v.currentNote; // Already stored at @plain_note (0x0A44)
	// v.delayCounter already set from v.noteDuration (0x0A47-0x0A4A)
	// v.whatever3/4 already reset to 0 (0x0A4D-0x0A52)
	// v.glideDownTimer already set to 2 (0x0A55-0x0A57)

	// Check legato bit instA0[7] & 0x02 (0x0A5D-0x0A6D)
	if (instA0[7] & 0x02) {
		// Legato: restore PW values from FA command backup
		v.something_else[0] = v.something_else[1]; // something_else+1 -> something_else+0
		v.something_else[2] = v.ctrl0;              // ctrl0 -> something_else+2
	}

	// Handle note=0 (rest) at L0A70
	if (note == 0) {
		// Load previous note from things+6 (0x0A75)
		note = v.currentNoteSlideTarget;
		v.currentNote = note; // Update stuff+3 equivalent
		v.currentNoteSlideTarget = 0; // Clear things+6 (0x0A7B-0x0A7D)

		// dec control3 (0x0A83)
		v.gateMask--;

		// bne L0AAD - since control3 was 0xFF, now 0xFE, always branches
		// Skip frequency write entirely for rests, jump to L0AAD
	} else {
		// Non-zero note: store as previous note (L0A88)
		v.currentNoteSlideTarget = note; // things+6 = note

		// Set frequency from note (L0A8C-L0AA1)
		if (note >= 96) note = 95;
		SID_Write(sidOffset + 1, frq_hi[note]); // $D401
		SID_Write(sidOffset + 0, frq_lo[note]); // $D400
		// Store in stuff variables: stuff[0]=lo, stuff[1]=lo, stuff[2]=hi, stuff[4]=hi
		uint8_t fLo = frq_lo[note];
		uint8_t fHi = frq_hi[note];
		v.stuff_freq_porta_vib = fLo | (fHi << 8); // stuff[0]/[4]
		v.stuff_freq_base = fLo | (fHi << 8);       // stuff[1]/[2]
		v.stuff_freq_hard_restart = fLo | (fHi << 8);
		v.currentFreq = v.stuff_freq_porta_vib;

		// Write initial waveform (gate-on transient): instA0[6] -> $D404 (L0AA7-L0AAA)
		SID_Write(sidOffset + 4, instA0[6]);
	}

	// L0AAD: Write control register with gate mask
	// lda instA0[1]; AND control3; sta $D404 (0x0AAD-0x0AB3)
	SID_Write(sidOffset + 4, instA0[1] & v.gateMask);

	// Write ADSR from instrument (0x0AB6-0x0ABF)
	SID_Write(sidOffset + 5, instA0[2]); // Attack / Decay
	SID_Write(sidOffset + 6, instA0[3]); // Sustain / Release

	// Write Pulse Width from something_else (0x0AC2-0x0ACB)
	SID_Write(sidOffset + 2, v.something_else[0]); // PW Lo
	SID_Write(sidOffset + 3, v.something_else[2]); // PW Hi

	v.pulseWidth = v.something_else[0] | (v.something_else[2] << 8);
}

// --- Post-Note Effect Setup ---
// Corresponds to L0AFC in the assembly. Runs after each note is applied.
// Determines which continuous effect should be active based on instrument data.
void DrillerSIDPlayer::postNoteEffectSetup(VoiceState &v, int sidOffset, const uint8_t *instA0, const uint8_t *instA1) {
	// L0AFC: lda voice1_things+6,x; beq L0B33
	if (v.currentNoteSlideTarget == 0)
		return; // No note stored, skip to continuous effects (PW LFO etc.)

	// Check if portamento already active from FB/FC pattern command (L0B04)
	// lda voice1_whatever+2,x; bne L0B17
	if (v.whatever2 != 0) {
		// Already have porta from pattern command, go to porta processing
		return; // Porta will run in applyContinuousEffects
	}

	int instBase = v.instrumentIndex;
	if (instBase < 0 || (size_t)instBase >= sizeof(instrumentDataA0))
		instBase = 0;

	// Check instA1[4] for instrument-level portamento (L0B09)
	// lda possibly_instrument_a1+4,y; beq L0B1A
	if (instA1[4] != 0) {
		v.whatever2 = instA1[4];       // Store as porta type (L0B0E)
		v.portaStepRaw = instA1[3];    // Store porta speed (L0B11-L0B14)
		v.portaSpeed = 0;              // Force recalc
		return; // jmp L0C5A - porta will run in applyContinuousEffects
	}

	// Check instA0[5] for arpeggio (L0B1A -> L0E67)
	// lda possibly_instrument_a0+5,y; beq L0B22
	if (instA0[5] != 0) {
		// L0E67: arpeggio setup
		uint8_t arpData = instA0[5];
		v.arpTableIndex = arpData & 0x0F;                    // and #$0F -> ctrl1
		v.arpSpeedHiNibble = (arpData & 0xF0) >> 4;         // and #$F0; lsr*4 -> stuff+5
		v.stuff_arp_counter = 0;                             // sta stuff+6
		v.whatever1 = 1;                                     // sta whatever+1
		v.whatever0 = 0;                                     // sta whatever
		return; // jmp voice_done
	}

	// L0B22: Clear arpeggio flag (A=0 from beq)
	v.whatever1 = 0;

	// Check instA1[0] for vibrato (L0B25 -> L0E89)
	// lda possibly_instrument_a1,y; beq L0B2D
	if (instA1[0] != 0) {
		// L0E89: vibrato setup
		v.things_vib_depth = instA1[0];                      // sta things+1
		v.things_vib_delay_reload = instA1[1];               // sta things+2
		v.things_vib_delay_ctr = v.things_vib_delay_reload;  // sta things+3
		v.things_vib_state = 0;                              // sta things
		v.whatever1 = 0;                                     // sta whatever+1
		v.whatever0 = 1;                                     // sta whatever
		return; // jmp voice_done
	}

	// L0B2D: Clear vibrato flag (A=0 from beq)
	v.whatever0 = 0;
	// jmp voice_done
}

// --- Continuous Effect Application (Vibrato, Porta, Arp) ---
void DrillerSIDPlayer::applyContinuousEffects(VoiceState &v, int sidOffset, const uint8_t *instA0, const uint8_t *instA1) {
	// Corresponds to logic starting around L0B33 / L0B82 / L0BC0 / L0C5A

	uint16_t freq = v.stuff_freq_porta_vib; // Start with base freq + porta/vib from previous step
	bool freqDirty = false;                 // Track if frequency needs writing

	// PW LFO (L0B33-L0B82) - instA0[4] = modulation speed (stored in control1)
	uint8_t lfoSpeed = instA0[4];
	if (lfoSpeed != 0) {
		// Operates on something_else[0] (lo byte) and something_else[2] (hi nibble)
		if (v.whatever2_vibDirToggle == 0) {
			// Add phase (L0B40-L0B5D): clc; adc control1 on lo; adc #0 on hi
			uint16_t sum = (uint16_t)v.something_else[0] + lfoSpeed;
			v.something_else[0] = sum & 0xFF;
			v.something_else[2] = v.something_else[2] + (sum >> 8);
			SID_Write(sidOffset + 2, v.something_else[0]); // $D402
			SID_Write(sidOffset + 3, v.something_else[2]); // $D403
			// clc; cmp #$0E - check hi byte >= 0x0E (L0B59)
			if (v.something_else[2] >= 0x0E) {
				v.whatever2_vibDirToggle = 1; // inc whatever2 (L0B5D)
			}
		} else {
			// Subtract phase (L0B62-L0B7F): sec; sbc control1 on lo; sbc #0 on hi
			uint16_t diff = (uint16_t)v.something_else[0] + 0x100 - lfoSpeed;
			v.something_else[0] = diff & 0xFF;
			if (diff < 0x100) // borrow
				v.something_else[2]--;
			SID_Write(sidOffset + 2, v.something_else[0]); // $D402
			SID_Write(sidOffset + 3, v.something_else[2]); // $D403
			// clc; cmp #$08 - check hi byte < 0x08 (L0B7B)
			if (v.something_else[2] < 0x08) {
				v.whatever2_vibDirToggle = 0; // dec whatever2 (L0B7F)
			}
		}
		v.pulseWidth = v.something_else[0] | (v.something_else[2] << 8);
	}

	// Arpeggio (L0B82) - Check 'whatever1' flag
	if (v.whatever1) {
		// Assembly: single counter (stuff+6) cycles 0..speed-1, used directly as arp table index
		// lda stuff+6; cmp stuff+5; bne L0BA5; lda #0; sta stuff+6
		uint8_t speed = v.arpSpeedHiNibble; // stuff+5: set from instA0[5] hi nibble
		if (v.stuff_arp_counter == speed) {
			v.stuff_arp_counter = 0; // Reset when counter == speed (L0BA0)
		}

		// tay; lda stuff+3; clc; adc arpeggio_0,y (L0BA5-L0BAD)
		uint8_t baseNote = v.currentNote;
		if (baseNote > 0 && baseNote < 96 && v.stuff_arp_counter < 3) {
			uint8_t arpOffset = arpeggio_data[v.stuff_arp_counter]; // Counter IS the table index
			uint8_t arpNote = baseNote + arpOffset;
			if (arpNote >= 96)
				arpNote = 95;

			freq = frq_lo[arpNote] | (frq_hi[arpNote] << 8);
			SID_Write(sidOffset + 0, frq_lo[arpNote]); // sta $D400 (L0BB1)
			SID_Write(sidOffset + 1, frq_hi[arpNote]); // sta $D401 (L0BB7)
			v.currentFreq = freq;
		}

		v.stuff_arp_counter++; // inc stuff+6 (L0BBA)
		// jmp voice_done - arpeggio skips vibrato/porta
		return;
	}

	// Vibrato (L0BC0 / L0BC8) - Check 'whatever0' flag
	// Assembly applies frequency modification EVERY frame.
	// The timer (things+3) only controls when the direction state advances.
	if (v.whatever0) {
		int state = v.things_vib_state;
		uint8_t freqLo = v.stuff_freq_porta_vib & 0xFF;
		uint8_t freqHi = (v.stuff_freq_porta_vib >> 8) & 0xFF;

		// Apply depth based on state (L0C06, L0C2F, L0BD1)
		// States 0, 3, 4: subtract (down). States 1, 2: add (up).
		// State 0: L0C06 (beq). States 1,2: L0C2F (cmp #3; bcc). States 3,4: fall through.
		if (state == 1 || state == 2) {
			// Add (L0C2F): clc; lda stuff,x; adc things+1,x
			uint16_t sum = (uint16_t)freqLo + (v.things_vib_depth & 0xFF);
			freqLo = sum & 0xFF;
			freqHi = freqHi + (sum >> 8);
		} else {
			// Subtract (L0C06/L0BD1): sec; lda stuff,x; sbc things+1,x
			uint16_t diff = (uint16_t)freqLo + 0x100 - (v.things_vib_depth & 0xFF);
			freqLo = diff & 0xFF;
			if (diff < 0x100) // borrow occurred
				freqHi--;
		}

		v.stuff_freq_porta_vib = (uint16_t)freqLo | ((uint16_t)freqHi << 8);
		freq = v.stuff_freq_porta_vib;
		freqDirty = true;

		// Decrement timer, advance state only when expired (dec things+3; bne done)
		v.things_vib_delay_ctr--;
		if (v.things_vib_delay_ctr == 0) {
			v.things_vib_delay_ctr = v.things_vib_delay_reload;
			v.things_vib_state++;
			if (v.things_vib_state >= 5) { // cmp #$05; bcc
				v.things_vib_state = 1;    // Reset to state 1 (0BFE)
			}
		}
	} // end if(v.whatever0)

	// Portamento (L0C5A) - Check 'whatever2' flag
	// 4 distinct types matching assembly:
	// Type 1 (L0C7B): CLC+SBC on lo byte (slide down, borrow to hi)
	// Type 2 (L0CA6): CLC+ADC on lo byte (slide up, carry to hi)
	// Type 3 (L0C96): SEC+SBC on hi byte only (fast slide down)
	// Type >= 4 (L0C6B): CLC+ADC on hi byte only (fast slide up)
	if (v.whatever2) {
		uint8_t freqLo = v.stuff_freq_porta_vib & 0xFF;       // stuff[0]
		uint8_t freqHi = (v.stuff_freq_porta_vib >> 8) & 0xFF; // stuff[4]
		uint8_t speed = v.portaStepRaw & 0xFF;                 // voice1_something

		if (v.whatever2 == 1) {
			// Type 1 (L0C7B): clc; sbc = subtract (speed+1) from lo, borrow to hi
			uint16_t diff = (uint16_t)freqLo - speed; // CLC means borrow, so effectively -(speed+1)
			freqLo = (diff - 1) & 0xFF; // CLC+SBC = subtract with extra borrow
			if ((diff - 1) > 0xFF) freqHi--; // Propagate borrow
			SID_Write(sidOffset + 0, freqLo);
			SID_Write(sidOffset + 1, freqHi);
		} else if (v.whatever2 == 2) {
			// Type 2 (L0CA6): clc; adc on lo, carry to hi
			uint16_t sum = (uint16_t)freqLo + speed;
			freqLo = sum & 0xFF;
			freqHi = freqHi + (sum >> 8);
			SID_Write(sidOffset + 0, freqLo);
			SID_Write(sidOffset + 1, freqHi);
		} else if (v.whatever2 == 3) {
			// Type 3 (L0C96): sec; sbc on hi byte only (fast slide down)
			freqHi = freqHi - speed;
			SID_Write(sidOffset + 1, freqHi);
		} else {
			// Type >= 4 (L0C6B): clc; adc on hi byte only (fast slide up)
			freqHi = freqHi + speed;
			SID_Write(sidOffset + 1, freqHi);
		}

		v.stuff_freq_porta_vib = (uint16_t)freqLo | ((uint16_t)freqHi << 8);
		v.currentFreq = v.stuff_freq_porta_vib;
	}

	// After porta, check for hard restart (L0CBE)
	// lda instA0[7]; and #$01; beq voice_done; jmp L1005
	if (instA0[7] & 0x01) {
		applyHardRestart(v, sidOffset, instA0, instA1);
	}

	// Write final frequency if modified by vibrato (arp and porta write directly)
	if (freqDirty && v.currentFreq != freq) {
		v.currentFreq = freq;
		SID_Write(sidOffset + 0, freq & 0xFF);
		SID_Write(sidOffset + 1, (freq >> 8) & 0xFF);
	}
}

// --- Hard Restart / Buzz Effect ---
void DrillerSIDPlayer::applyHardRestart(VoiceState &v, int sidOffset, const uint8_t *instA0, const uint8_t *instA1) {
	// Corresponds to L1005 onwards
	debug(DEBUG_LEVEL >= 2, "Driller 1: Applying Hard Restart (Delay=%d, Ctr=%d, Val=%d)", v.hardRestartDelay, v.hardRestartCounter, v.hardRestartValue);

	// Check delay phase (L100D)
	if (v.hardRestartDelay > 0) {
		v.hardRestartDelay--;
		// Set high bit of waveform? (L1015)
		SID_Write(sidOffset + 4, 0x81); // Force waveform to noise? Or just toggle sync/ring? Or maybe $80 = Noise, $01 = Gate On
		// Modify frequency slightly (L101A)
		uint16_t freq = v.stuff_freq_hard_restart; // Use stored base freq
		// freq ^= 0x2300; // EOR with #$23 on high byte? (L101D) - Check calculation
		uint8_t hiByte = (freq >> 8) ^ 0x23; // EOR high byte only
		SID_Write(sidOffset + 1, hiByte);    // Write modified high byte
		// Keep low byte as is? Yes, original only writes high byte $D401.
		// Keep current frequency updated? No, use stored base.
		// v.currentFreq = (hiByte << 8) | (freq & 0xFF); // Update internal state if needed
	} else {
		// Delay phase over, check frequency change phase (L103A)
		if (v.hardRestartCounter < v.hardRestartValue) { // Compare with value from inst A1[5] (L103D)
			v.hardRestartCounter++;                      // Increment counter (L1045)
			v.hardRestartDelay++;                        // Also increment delay? Seems odd (L1042) - Maybe reloads delay? Yes, seems to reload.
			// Reset frequency and waveform (L1048 -> L1028)
			uint16_t freq = v.stuff_freq_hard_restart;
			SID_Write(sidOffset + 1, (freq >> 8) & 0xFF); // Restore high byte (L1028)
			SID_Write(sidOffset + 0, freq & 0xFF);        // Restore low byte (L102B implies sta $D401,x AND sta $D400,x ?) No, only $D401. Assume low byte restored too.
			v.currentFreq = freq;                         // Update internal state

			// Restore waveform from instrument? (L1031) - Uses instA1[2]? Needs Gate bit.
			uint8_t ctrl = instA1[2];
			if (v.keyOn)
				ctrl |= 0x01;
			else
				ctrl &= 0xFE; // Add gate state
			SID_Write(sidOffset + 4, ctrl);
		} else {
			// Effect finished (L104A)
			debug(DEBUG_LEVEL >= 2, "Driller 1: Hard Restart Finished");
			v.hardRestartActive = false;
			v.hardRestartCounter = 0; // Reset counters
			v.hardRestartDelay = 0;
			// Restore frequency and waveform (L104A -> L1052 -> L1028)
			uint16_t freq = v.stuff_freq_hard_restart;
			SID_Write(sidOffset + 1, (freq >> 8) & 0xFF);
			SID_Write(sidOffset + 0, freq & 0xFF);
			v.currentFreq = freq; // Update internal state

			uint8_t ctrl = instA1[2]; // Restore waveform from instA1[2]? Needs Gate bit.
			if (v.keyOn)
				ctrl |= 0x01;
			else
				ctrl &= 0xFE; // Add gate state
			SID_Write(sidOffset + 4, ctrl);
		}
	}
}

} // namespace Freescape
