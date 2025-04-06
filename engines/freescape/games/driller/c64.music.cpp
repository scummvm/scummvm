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

// --- Driller Music Data (Embedded from Disassembly) ---
namespace Freescape {

// Frequency Tables (0x0D53 - 0x0E12)
const uint8_t frq_lo[192] = {
	0x0C, 0x1C, 0x2D, 0x3E, 0x51, 0x66, 0x7B, 0x91, 0xA9, 0xC3, 0xDD, 0xFA, 0x18, 0x38, 0x5A, 0x7D, // 0D53
	0xA3, 0xCC, 0xF6, 0x23, 0x53, 0x86, 0xBB, 0xF4, 0x30, 0x70, 0xB4, 0xFB, 0x47, 0x98, 0xED, 0x47, // 0D63
	0xA7, 0x0C, 0x77, 0xE9, 0x61, 0xE1, 0x68, 0xF7, 0x8F, 0x30, 0xDA, 0x8F, 0x4E, 0x18, 0xEF, 0xD2, // 0D73
	0xC3, 0xC3, 0xD1, 0xEF, 0x1F, 0x60, 0xB5, 0x1E, 0x9C, 0x31, 0xDF, 0xA5, 0x87, 0x86, 0xA2, 0xDF, // 0D83
	0x3E, 0xC1, 0x6B, 0x3C, 0x39, 0x63, 0xBE, 0x4B, 0x0F, 0x0C, 0x45, 0xBF, 0x7D, 0x83, 0xD6, 0x79, // 0D93
	0x73, 0xC7, 0x7C, 0x97, 0x1E, 0x18, 0x8B, 0x7E, 0xFA, 0x06, 0xAC, 0xF3, 0xE6, 0x8F, 0xF8, 0x2E, // 0DA3
	// Fill remaining based on 96 notes if needed, assuming 96 notes.
	// Replicate last 96 bytes if table is expected to be 192
	0x0C, 0x1C, 0x2D, 0x3E, 0x51, 0x66, 0x7B, 0x91, 0xA9, 0xC3, 0xDD, 0xFA, 0x18, 0x38, 0x5A, 0x7D,
	0xA3, 0xCC, 0xF6, 0x23, 0x53, 0x86, 0xBB, 0xF4, 0x30, 0x70, 0xB4, 0xFB, 0x47, 0x98, 0xED, 0x47,
	0xA7, 0x0C, 0x77, 0xE9, 0x61, 0xE1, 0x68, 0xF7, 0x8F, 0x30, 0xDA, 0x8F, 0x4E, 0x18, 0xEF, 0xD2,
	0xC3, 0xC3, 0xD1, 0xEF, 0x1F, 0x60, 0xB5, 0x1E, 0x9C, 0x31, 0xDF, 0xA5, 0x87, 0x86, 0xA2, 0xDF,
	0x3E, 0xC1, 0x6B, 0x3C, 0x39, 0x63, 0xBE, 0x4B, 0x0F, 0x0C, 0x45, 0xBF, 0x7D, 0x83, 0xD6, 0x79,
	0x73, 0xC7, 0x7C, 0x97, 0x1E, 0x18, 0x8B, 0x7E, 0xFA, 0x06, 0xAC, 0xF3, 0xE6, 0x8F, 0xF8, 0x2E};
const uint8_t frq_hi[192] = {
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, // 0DB3
	0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x06, // 0DC3
	0x06, 0x07, 0x07, 0x07, 0x08, 0x08, 0x09, 0x09, 0x0A, 0x0B, 0x0B, 0x0C, 0x0D, 0x0E, 0x0E, 0x0F, // 0DD3
	0x10, 0x11, 0x12, 0x13, 0x15, 0x16, 0x17, 0x19, 0x1A, 0x1C, 0x1D, 0x1F, 0x21, 0x23, 0x25, 0x27, // 0DE3
	0x2A, 0x2C, 0x2F, 0x32, 0x35, 0x38, 0x3B, 0x3F, 0x43, 0x47, 0x4B, 0x4F, 0x54, 0x59, 0x5E, 0x64, // 0DF3
	0x6A, 0x70, 0x77, 0x7E, 0x86, 0x8E, 0x96, 0x9F, 0xA8, 0xB3, 0xBD, 0xC8, 0xD4, 0xE1, 0xEE, 0xFD, // 0E03
	// Fill remaining based on 96 notes if needed
	// Replicate last 96 bytes if table is expected to be 192
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02,
	0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x06,
	0x06, 0x07, 0x07, 0x07, 0x08, 0x08, 0x09, 0x09, 0x0A, 0x0B, 0x0B, 0x0C, 0x0D, 0x0E, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13, 0x15, 0x16, 0x17, 0x19, 0x1A, 0x1C, 0x1D, 0x1F, 0x21, 0x23, 0x25, 0x27,
	0x2A, 0x2C, 0x2F, 0x32, 0x35, 0x38, 0x3B, 0x3F, 0x43, 0x47, 0x4B, 0x4F, 0x54, 0x59, 0x5E, 0x64,
	0x6A, 0x70, 0x77, 0x7E, 0x86, 0x8E, 0x96, 0x9F, 0xA8, 0xB3, 0xBD, 0xC8, 0xD4, 0xE1, 0xEE, 0xFD};

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
const uint16_t arpeggio_addr[] = {0x157C}; // Address of arpeggio_0 data (relative offset for index lookup)
const int NUM_ARPEGGIOS = sizeof(arpeggio_addr) / sizeof(arpeggio_addr[0]);

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
const int NUM_PATTERNS = sizeof(pattern_addresses) / sizeof(pattern_addresses[0]);

// Tune Data (0x1054, 0x15D5 - 0x15E5)
const uint8_t tune_tempo_data[] = {0x00, 0x03, 0x03}; // tempos for tune 0, 1, 2
const uint8_t *const tune_track_data[][3] = {
	{nullptr, nullptr, nullptr},                               // Tune 0 (no data specified, likely silent/unused)
	{voice1_track_data, voice2_track_data, voice3_track_data}, // Tune 1
	{voice1_track_data, voice2_track_data, voice3_track_data}  // Tune 2 (Assume same as tune 1 for now if needed)
};
const int NUM_TUNES = sizeof(tune_tempo_data) / sizeof(tune_tempo_data[0]);

// SID Base Addresses for Voices
const int voice_sid_offset[] = {0, 7, 14};

// Debug log levels
#define DEBUG_LEVEL 4 // 0: Minimal, 1: Basic Flow, 2: Detailed State

DrillerSIDPlayer::DrillerSIDPlayer(Audio::Mixer *mixer) : _sid(nullptr),
														  _mixer(mixer),
														  _soundHandle(), // Default initialize
														  _sampleRate(mixer->getOutputRate()),
														  _cyclesPerSample(0.0f),
														  _cycleCounter(0.0),
														  _playState(STOPPED),
														  _targetTuneIndex(0),
														  _globalTempo(3),       // Default tempo
														  _globalTempoCounter(1) // Start immediately
{
	initSID();

	// Calculate cycles per sample for timing in readBuffer
	// Using PAL clock rate for C64 SID
	const double PAL_CLOCK_FREQ = 985248.0; // Use PAL C64 clock
	_cyclesPerSample = PAL_CLOCK_FREQ / _sampleRate;

	// Start the stream via the mixer
	// Pass address of _soundHandle for it to be filled by playStream
	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
	debug(DEBUG_LEVEL >= 1, "Driller SID Player Initialized (Sample Rate: %d Hz)", _sampleRate);
}

DrillerSIDPlayer::~DrillerSIDPlayer() {
	// Check if sound handle is valid before stopping (might not be if playStream failed)
	// A better check might involve a dedicated flag or checking if handle is non-zero/default
	if (_mixer) {                         // Ensure mixer exists
		_mixer->stopHandle(_soundHandle); // Pass handle by value
	}
	delete _sid;
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

// --- AudioStream API ---
int DrillerSIDPlayer::readBuffer(int16 *buffer, const int numSamples) {
	if (!_sid) { // Safety check if SID initialization failed
		memset(buffer, 0, numSamples * sizeof(int16));
		return numSamples;
	}

	int samplesGenerated = 0;
	while (samplesGenerated < numSamples) {
		// Determine how many SID cycles until the next C64 frame tick (approx 50Hz for PAL)
		const double CYCLES_PER_FRAME = 985248.0 / 50.0; // PAL C64 clock / 50Hz VSync

		// How many cycles to run SID for this iteration?
		double cyclesToRun = CYCLES_PER_FRAME - _cycleCounter;
		int samplesToGenerate = MIN((int)ceil(cyclesToRun / _cyclesPerSample), numSamples - samplesGenerated);
		if (samplesToGenerate <= 0)
			samplesToGenerate = 1; // Ensure progress

		// Prevent requesting more samples than the buffer has space for
		samplesToGenerate = MIN(samplesToGenerate, numSamples - samplesGenerated);

		double cyclesForThisStep = samplesToGenerate * _cyclesPerSample;

		// Run the SID emulation
		Resid::cycle_count x = static_cast<Resid::cycle_count>(cyclesForThisStep);
		// Use the standard reSID clock method
		_sid->updateClock(x, buffer + samplesGenerated, samplesToGenerate);

		_cycleCounter += cyclesForThisStep;
		samplesGenerated += samplesToGenerate;

		// If a frame boundary is crossed, run the player logic
		if (_cycleCounter >= CYCLES_PER_FRAME) {
			_cycleCounter -= CYCLES_PER_FRAME; // Keep track of remainder cycles
			playFrame();
		}
	}
	return numSamples; // We always fill the buffer requested
}

// --- SID Interaction ---
void DrillerSIDPlayer::SID_Write(int reg, uint8_t data) {
	if (_sid) {
		debug(DEBUG_LEVEL >= 3, "SID Write: Reg $%02X = $%02X", reg, data);
		_sid->write(reg, data);
	}
}

void DrillerSIDPlayer::initSID() {
	delete _sid; // Delete previous instance if any
	_sid = new Resid::SID();
	// Use PAL clock rate
	_sid->set_sampling_parameters(985248.0, _sampleRate);
	_sid->enable_filter(true); // Enable filter emulation
	_sid->reset();

	// Reset SID registers (like 0x0910 - reset_voices)
	SID_Write(0x04, 0);    // V1 Ctrl = 0
	SID_Write(0x0B, 0);    // V2 Ctrl = 0
	SID_Write(0x12, 0);    // V3 Ctrl = 0
	SID_Write(0x15, 0);    // Filter Cutoff Lo = 0
	SID_Write(0x16, 0);    // Filter Cutoff Hi = 0
	SID_Write(0x17, 0);    // Filter Res/Ctrl = 0
	SID_Write(0x18, 0x0F); // Volume & Filter Mode = Max Volume
}

// --- Player Logic (Called once per C64 frame) ---
void DrillerSIDPlayer::playFrame() {
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

	// Update global tempo counter (0x09A5)
	bool tempoTick = false;
	if (_globalTempoCounter > 0) { // Only decrement if positive
		_globalTempoCounter--;
	}

	if (_globalTempoCounter == 0) {
		tempoTick = true;
		_globalTempoCounter = _globalTempo; // Reload counter (0x09AA)
		if (_globalTempoCounter == 0)
			_globalTempoCounter = 1; // Avoid getting stuck if tempo is 0
		debug(DEBUG_LEVEL >= 2, "Driller: Tempo Tick! Reloading counter to %d", _globalTempoCounter);
	}

	// Process each voice (0x0E46 - 0x0E55)
	for (int voiceIndex = 0; voiceIndex < 3; ++voiceIndex) {
		playVoice(voiceIndex, tempoTick);
	}

	// Update master volume after processing voices (Maybe not needed if set elsewhere)
	// The original sets it in reset_voices and potentially instrument data
	// SID_Write(0x18, 0x0F); // Ensure volume is max - Done in init/reset
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
			debug(DEBUG_LEVEL >= 1, "Driller: V%d - Got track pointer %p from currentTuneTracks[%d]", i, trackPtr, i);
		} else {
			debug(DEBUG_LEVEL >= 0, "Driller: V%d - Cannot get track pointer because currentTuneTracks is NULL", i);
		}

		// Assign the pointer
		_voiceState[i].trackDataPtr = trackPtr;

		// *** ADD THIS LOG - AFTER ASSIGNING TO voiceState ***
		debug(DEBUG_LEVEL >= 1, "Driller: V%d - Assigned _voiceState[%d].trackDataPtr = %p", i, i, _voiceState[i].trackDataPtr);

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

		debug(DEBUG_LEVEL >= 1, "Driller: Reset Voice %d - Checking _voiceState[%d].trackDataPtr (%p)...", i, i, _voiceState[i].trackDataPtr);

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
void DrillerSIDPlayer::playVoice(int voiceIndex, bool tempoTick) {
	// debug(DEBUG_LEVEL >= 2, "Driller: Processing Voice %d (Tempo Tick: %d)", voiceIndex, tempoTick);
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

	// Hard Restart / Buzz Effect Check (Inst A0[7] & 0x01) - Apply if active
	// This check was previously in applyNote, moved here to match L1005 check location relative to effects
	if (v.hardRestartActive) {
		applyHardRestart(v, sidOffset, instA0, instA1);
	}

	// Glide down effect? (L094E) - Inst A0[7] & 0x04
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

	// --- Tempo Tick: Process note/delay ---
	if (tempoTick) { // Added braces for clarity
		// Corresponds to L0964 -> L096E path when tempo_ctr hits 0

		// Decrement voice delay counter (0x0969 dec voice1_ctrl2,x)
		if (v.delayCounter >= 0) { // If positive or zero, decrement
			v.delayCounter--;
			// --- Fix 4: Add Logging ---
			debug(DEBUG_LEVEL >= 2, "Driller V1: Tempo Tick - Delay Counter decremented to %d", v.delayCounter);
			// --- End Fix 4 ---
		}

		// If counter is still non-negative (was >= 0 before decrement), note holds
		if (v.delayCounter >= 0) {
			// bmi L09B6 is false
			// Apply continuous effects for this frame (original jumps to L0B33 via L096E)
			applyContinuousEffects(v, sidOffset, instA0, instA1);
			return; // Return AFTER applying effects for the holding note
		}

		// --- Delay Counter Expired (was 0, now -1): Read New Note/Command (L09B6 onwards) ---
		debug(DEBUG_LEVEL >= 1, "Driller V1: Delay Counter Expired - Reading new pattern data"); // Add this log

		// ... rest of pattern reading logic ...

	} else { // Not a tempo tick
		applyContinuousEffects(v, sidOffset, instA0, instA1);
		return; // No note processing on non-tempo ticks
	}

	// If tempoTick was true AND delayCounter became < 0, pattern processing happened above.
	// If we reach here, it means a new note/command was processed.
	// Do we need to call applyContinuousEffects *again*?
	// The original assembly jumps to voice_done (0B30, 0CCB etc) after effects or note setting.
	// Let's assume effects are applied either during hold (in the delayCounter >= 0 block)
	// or implicitly handled as part of the new note setup (e.g. frequency set directly).
	// Avoid calling applyContinuousEffects twice per tick.
	// The structure now correctly handles this:
	// - If !tempoTick -> applyEffects -> return
	// - If tempoTick:
	//    - Decrement delay
	//    - If delay >= 0 -> applyEffects -> return
	//    - If delay < 0 -> processPattern -> (applyNote potentially called) -> implicit return (end of function)

	// --- Delay Counter Expired (was 0, now -1): Read New Note/Command (L09B6 onwards) ---
	// Reset delay counter - will be set by FD command later if needed. Stays -1 for now.

	// Store track/pattern pointers locally (like 09B6-09BE)
	// Already have v.trackDataPtr, v.patternDataPtr

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
	_tempControl3 = 0xFF; // Reset gate mask (0x09D0) - Currently unused in C++ code
	v.whatever0 = 0;      // Reset effect states (0x09D5 onwards)
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

			// Effect FD: Set Note Duration (0x09E5 + 0x09ED)
			if (cmd == 0xFD) {
				v.noteDuration = dataByte; // Store duration (0x09EF)
				debug(DEBUG_LEVEL >= 2, "Driller V%d: Cmd FD, Set Duration = %d", voiceIndex, v.noteDuration);
			}
			// Effect FC: Portamento Up (0x0A17) / FE in disassembly comment? Check logic.
			// Original checks FD, then FB, then FA. FE is not checked explicitly.
			// Assuming FE should behave like FC based on command range >= FD.
			else if (cmd == 0xFE) { // FC in disassembly checks cmp #$FB, bne @effect_fc_2
				debug(DEBUG_LEVEL >= 2, "Driller V%d: Cmd FE/FC, Porta Up Param = $%02X", voiceIndex, dataByte);
				if (v.currentNote > 0) {                      // Only apply if a note is playing
					v.whatever2 = (instA0[7] & 0x02) ? 4 : 2; // Porta Up Type
					v.portaStepRaw = dataByte;
					v.whatever0 = 0;
					v.whatever1 = 0;  // Reset other effects
					v.portaSpeed = 0; // Force recalc
				}
			}
			// Effect FB: Portamento Down (0x09FB)
			else { // Must be FB (This case unreachable if cmd == 0xFE handled above?)
				// Correction: Original logic is cmp $FD -> bcc check_fb_fc -> cmp $FB -> bcc check_fa -> cmp $FB -> bne effect_fc -> effect_fb
				// So if >= FD, it *is* FD. If not FD, then check FB. If FB, do FB. If not FB, do FC (lda #2, bne do_effect).
				// Let's fix the logic:
				/* Handled above for FD */
				debug(DEBUG_LEVEL >= 0, "Driller V%d: Unexpected path for Cmd $%02X", voiceIndex, cmd);
			}
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

			if (v.currentNote > 0) {
				// Set porta type (1=Down(FB), 2=Up(FC)) or (3=DownH, 4=UpH)
				if (cmd == 0xFB) {                            // effect_fb_1
					v.whatever2 = (instA0[7] & 0x02) ? 3 : 1; // (0A01)
					debug(DEBUG_LEVEL >= 2, "Driller V%d: Cmd FB, Porta Down Param = $%02X (Type %d)", voiceIndex, portaParam, v.whatever2);
				} else {                                      // FC (effect_fc_2)
					v.whatever2 = (instA0[7] & 0x02) ? 4 : 2; // (0A17 -> 0A01)
					debug(DEBUG_LEVEL >= 2, "Driller V%d: Cmd FC, Porta Up Param = $%02X (Type %d)", voiceIndex, portaParam, v.whatever2);
				}

				v.portaStepRaw = portaParam; // Store raw porta speed (0A0A / 0A19->0A0A)
				v.whatever0 = 0;             // Reset vibrato state (0A0D)
				v.whatever1 = 0;             // Reset arpeggio state (0A0F)
				v.portaSpeed = 0;            // Force recalc
			} else {
				debug(DEBUG_LEVEL >= 2, "Driller V%d: Ignoring FB/FC command, no note playing.", voiceIndex);
			}
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
			v.something_else[0] = v.attackDecay; // Map to something_else array
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
			v.currentNote = cmd; // Store note value (0A44)
			debug(DEBUG_LEVEL >= 2, "Driller V%d: Note Cmd = $%02X (%d)", voiceIndex, v.currentNote, v.currentNote);
			// Set delay counter based on previously read duration (FD command)
			// If no FD command, duration is 0, so delayCounter is set to 0
			// The counter is checked *after* decrementing. So if duration is N, it lasts N ticks.
			// If duration is 1, counter=1 -> dec=0 -> hold -> dec=-1 -> new note. Lasts 1 tick.
			// If duration is 0, counter=0 -> dec=-1 -> new note. Lasts 0 ticks (effectively ignored?).
			// Let's set counter = duration.
			v.delayCounter = v.noteDuration; // (0A47 -> 0A4A)
			v.noteDuration = 0;              // Reset duration for next note

			// Reset hard restart counters (0A4D)
			v.whatever3 = 0;
			v.whatever4 = 0;

			// Reset glide down timer (0A55)
			v.glideDownTimer = 2; // voice1_two_ctr = 2

			// Handle legato/slide (Instrument A0[7] & 0x02) (0A5D)
			if (instA0[7] & 0x02) { // Check legato bit
				// Copy AD high nibble again? (0A64) - Seems redundant
				// v.something_else[0] = v.attackDecay; // If something_else maps to PW, this overwrites PW?
				// Copy SR low nibble again? (0A6A)
				// v.sustainRelease = v.ctrl0; // Ensure SR matches instrument
				// Store in something_else[2]? Original stores ctrl0 to [2] (0A6D)
				// v.something_else[2] = v.ctrl0; // Map ADSR to structure? No, assume PW.
				// This block in assembly seems to just reload ADSR values into temp locations? Ignore for C++ struct model.
				debug(DEBUG_LEVEL >= 3, "Driller V%d: Legato instrument flag set.", voiceIndex);
			}

			// Apply Note Data
			applyNote(v, sidOffset, instA0, instA1, voiceIndex);

			// Continue reading pattern (but we are done with this note)
			v.patternIndex++;
			noteProcessed = true; // Exit the pattern reading loop for this frame
		}

	} // End while(!noteProcessed)

	// After processing note or commands for this tick, if a note wasn't fully processed (e.g. pattern end)
	// we might need to apply effects. But if noteProcessed = true, applyNote was called which handles final writes.
	// If noteProcessed = false (e.g. loop break), effects might need applying.
	// Let's assume effects are only applied when a note holds or on non-tempo ticks.
	// The call to applyContinuousEffects happens *outside* this loop if the delay counter held.
}

// --- Note Application ---
// --- Note Application ---
void DrillerSIDPlayer::applyNote(VoiceState &v, int sidOffset, const uint8_t *instA0, const uint8_t *instA1, int voiceIndex) {
	// Corresponds to 0xA70 onwards

	uint8_t note = v.currentNote;
	// uint16_t pulse = 0; // Remove - PW now uses something_else
	uint16_t newPulseWidth = 0; // New PW value
	uint8_t pwLoByte = 0;       // PW Lo Byte
	uint8_t pwHiNibble = 0;     // PW Hi Byte
	uint8_t writeAD = 0;
	uint8_t writeSR = 0;
	int currentInstNum = 0;
	bool isRest = (note == 0);
	// bool wasRestOrSlide = isRest; // Track if we started with note 0 (UNUSED?)

	// --- MOVED EFFECT INITIALIZATION HERE ---
	// Initialize effect states based on instrument flags (0xAFC onwards)
	// Reset previous effect states only if it wasn't a slide? Maybe always reset on new note cmd?
	// Let's reset effect triggers on *any* new note command (even 0), but keep porta/vib state if sliding?
	// For now, reset triggers based on original:
	// The original code resets whatever0/1/2 at 0x09D5 *before* the pattern loop.
	// FB/FC/FA commands also reset whatever0/1.
	// So, they should be reset unless a slide is happening.
	// Let's rely on the reset before the loop and specific command resets.
	// BUT, we need to SET the flags based on the instrument HERE.

	// Clear effect selection flags before setting based on instrument
	// We might refine this later if effects should persist across rests
	v.whatever0 = 0;
	v.whatever1 = 0;
	v.whatever2 = 0;
	v.portaSpeed = 0; // Reset calculated porta speed
	// Don't reset things_vib_state or stuff_arp counters here? Let effects manage their state.

	// Check Instrument A1[4] for Arpeggio? (L0B09) - Maps to 0xE67
	if (instA1[4] != 0) {
		debug(DEBUG_LEVEL >= 2, "Driller V%d: Activating Arpeggio from InstA1[4]=%02X", voiceIndex, instA1[4]);
		uint8_t arpData = instA1[4];
		v.arpTableIndex = arpData & 0x0F;
		v.arpSpeedHiNibble = (arpData & 0xF0) >> 4;
		if (v.arpTableIndex >= NUM_ARPEGGIOS)
			v.arpTableIndex = 0;
		v.stuff_arp_counter = 0; // Reset counter on activation
		v.stuff_arp_note_index = 0;
		v.whatever1 = 1; // Enable Arp
	}
	// Check Instrument A1[0] for Vibrato? (L0B25) - Maps to 0xE89
	else if (instA1[0] != 0) {
		debug(DEBUG_LEVEL >= 2, "Driller V%d: Activating Vibrato from InstA1[0]=%02X, A1[1]=%02X", voiceIndex, instA1[0], instA1[1]);
		v.things_vib_depth = instA1[0];                     // Depth
		v.things_vib_delay_reload = instA1[1];              // Delay/Speed
		v.things_vib_delay_ctr = v.things_vib_delay_reload; // Init counter
		v.things_vib_state = 0;                             // Init state
		v.whatever0 = 1;                                    // Enable Vib
	}
	// Check Instrument A0[5] for Arpeggio alternative? (L0B1A) - Also maps to 0xE67
	else if (instA0[5] != 0) {
		debug(DEBUG_LEVEL >= 2, "Driller V%d: Activating Arpeggio from InstA0[5]=%02X", voiceIndex, instA0[5]);
		uint8_t arpData = instA0[5];
		v.arpTableIndex = arpData & 0x0F;
		v.arpSpeedHiNibble = (arpData & 0xF0) >> 4;
		if (v.arpTableIndex >= NUM_ARPEGGIOS)
			v.arpTableIndex = 0;
		v.stuff_arp_counter = 0; // Reset counter on activation
		v.stuff_arp_note_index = 0;
		v.whatever1 = 1; // Enable Arp
	}
	// --- END MOVED EFFECT INITIALIZATION ---

	// Check for note 0 = Key Off / Rest (0A70)
	if (isRest) {
		// Slide/Porta to last note? (0A75)
		note = v.currentNoteSlideTarget; // Use last played note for slide target
		v.currentNoteSlideTarget = 0;    // Clear slide target

		if (note == 0) { // Still note 0, true rest
			debug(DEBUG_LEVEL >= 2, "Driller V%d: Applying Rest", voiceIndex);
			v.keyOn = false;           // Ensure gate is off for rest
			goto WriteFinalControlReg; // Skip freq/ADSR/PW writes, just turn off gate
		} else {
			// It's a slide/porta to the previous note 'note'
			debug(DEBUG_LEVEL >= 2, "Driller V%d: Applying Slide to Note %d", voiceIndex, note);
			v.keyOn = true; // Keep Gate ON for slide
			// DO NOT reset effect flags (whatever0/1/2) during slide? This needs verification.
			// The original code structure implies they *are* reset before the pattern loop,
			// and only FB/FC explicitly clear them again. Let's stick with the moved init block above for now.
		}
	} else {
		// Store current note for slide target (0A88)
		debug(DEBUG_LEVEL >= 2, "Driller V%d: Applying Note %d", voiceIndex, note);
		v.currentNoteSlideTarget = note;
		v.keyOn = true; // Key On for a normal note
	}

	// Calculate Frequency (0A8C)
	// ... (rest of frequency code is likely okay) ...
	if (note >= 96)
		note = 95; // Clamp note value
	v.baseFreq = frq_lo[note] | (frq_hi[note] << 8);

	// Store base frequency (0A8F - 0AA1)
	v.stuff_freq_base = v.baseFreq;
	v.stuff_freq_porta_vib = v.baseFreq;    // Initial freq includes effects
	v.stuff_freq_hard_restart = v.baseFreq; // Store for hard restart

	// Set initial frequency on SID
	v.currentFreq = v.baseFreq;
	SID_Write(sidOffset + 0, frq_lo[note]);
	SID_Write(sidOffset + 1, frq_hi[note]);
	debug(DEBUG_LEVEL >= 3, "Driller V%d: Set Freq = %d ($%04X)", voiceIndex, v.currentFreq, v.currentFreq);

	// Set Waveform from Instrument (0AA7)
	v.waveform = instA0[6]; // possibly_instrument_a0+6,y
	debug(DEBUG_LEVEL >= 3, "Driller V%d: Set Waveform = $%02X", voiceIndex, v.waveform);
	// KeyOn is set above based on note value/slide

	// Check if hard restart should start (Instrument A0[7] & 0x01) (0xCC1)
	// ... (hard restart check is likely okay) ...
	if (instA0[7] & 0x01) {
		debug(DEBUG_LEVEL >= 2, "Driller V%d: Activating Hard Restart", voiceIndex);
		v.hardRestartActive = true;
		v.hardRestartDelay = 0; // Reset counters for hard restart
		v.hardRestartCounter = 0;
		v.hardRestartValue = instA1[5]; // possibly_instrument_a1+5,y (0x103D)
	} else {
		v.hardRestartActive = false;
	}

	// Set ADSR (0xAB6)
	// Use values set by FA command (stored in v.attackDecay, v.sustainRelease)
	// SID_Write(sidOffset + 5, v.attackDecay); // Use state variable set by FA
	// SID_Write(sidOffset + 6, v.sustainRelease); // Use state variable set by FA
	// debug(DEBUG_LEVEL >= 3, "Driller V%d: Set ADSR = $%02X / $%02X", voiceIndex, v.attackDecay, v.sustainRelease);
	// In applyNote, right before writing ADSR to SID:

	// Set ADSR (0xAB6)
	writeAD = v.attackDecay;
	writeSR = v.sustainRelease;

	// --- TEMPORARY TEST: Override ADSR for Inst 1 and 4 ---
	currentInstNum = v.instrumentIndex / 8;
	if (currentInstNum == 1 || currentInstNum == 4) {
		debug(1, "Driller V%d: !!! ADSR OVERRIDE TEST !!! Inst %d -> Using AD=A, SR=F", voiceIndex, currentInstNum);
		writeAD = 0xA0; // Attack 10, Decay 0
		writeSR = 0x0F; // Sustain 15, Release 0 (Use F for sustain)
						// Note: The SR register is Sustain Hi Nibble / Release Lo Nibble
						// So 0x0F should be S=0, R=15. Let's try 0xF0 (S=15, R=0) for sustain.
		writeSR = 0xF0;
	}
	// --- END TEMPORARY TEST ---

	// Use values set by FA command (stored in v.attackDecay, v.sustainRelease) OR the override
	SID_Write(sidOffset + 5, writeAD); // Use potentially overridden value
	SID_Write(sidOffset + 6, writeSR); // Use potentially overridden value
	debug(DEBUG_LEVEL >= 3, "Driller V%d: Set ADSR = $%02X / $%02X", voiceIndex, writeAD, writeSR);

	// --- Fix 2: Correct Pulse Width Setting ---
	// Set Pulse Width (0xAC2)
	// Use voice1_something_else[0] and voice1_something_else[2] based on disassembly writes L0AC2/L0AC8
	// These were set by FA command based on instA0[0] (ADSR byte)
	pwLoByte = v.something_else[0];          // From AD hi nibble
	pwHiNibble = v.something_else[2] & 0x0F; // From SR lo nibble, masked to 4 bits

	// Combine for 12-bit PW
	newPulseWidth = pwLoByte | (pwHiNibble << 8);

	v.pulseWidth = newPulseWidth;                         // Update state variable *before* writing for LFO consistency
	SID_Write(sidOffset + 2, v.pulseWidth & 0xFF);        // Write Lo byte (0xAC5)
	SID_Write(sidOffset + 3, (v.pulseWidth >> 8) & 0x0F); // Write Hi nibble (0xACB)
	debug(DEBUG_LEVEL >= 3, "Driller V%d: Set PW = %d ($%03X) based on something_else[0/2] (AD/SR nibbles)", voiceIndex, v.pulseWidth, v.pulseWidth);
	// --- End Fix 2 ---

	// Effect initialization was moved higher up

WriteFinalControlReg: // Target for true rests
					  // --- Final SID Write for Gate/Waveform ---
					  // ... (final control reg write is likely okay) ...
	uint8_t ctrl = v.waveform;
	if (v.keyOn) {
		ctrl |= 0x01; // Gate On
	} else {
		ctrl &= 0xFE; // Gate Off
	}
	// Apply sync/ring mod from waveform byte if included
	SID_Write(sidOffset + 4, ctrl);
	debug(DEBUG_LEVEL >= 2, "Driller V%d: Final Control Reg Write = $%02X (Wave=$%02X, Gate=%d)", voiceIndex, ctrl, v.waveform, v.keyOn);
}

// --- Continuous Effect Application (Vibrato, Porta, Arp) ---
void DrillerSIDPlayer::applyContinuousEffects(VoiceState &v, int sidOffset, const uint8_t *instA0, const uint8_t *instA1) {
	// Corresponds to logic starting around L0B33 / L0B82 / L0BC0 / L0C5A

	uint16_t freq = v.stuff_freq_porta_vib; // Start with base freq + porta/vib from previous step
	bool freqDirty = false;                 // Track if frequency needs writing

	// Instrument A0[4] based frequency LFO (L0B33) - PW LFO?
	uint8_t lfoSpeed = instA0[4];
	if (lfoSpeed != 0) {
		// This LFO modifies 'something_else', which we mapped to PW registers based on FA command logic?
		// Or does it modify PW directly based on current PW? Let's assume it modifies current PW.
		uint16_t currentPW = v.pulseWidth;   // Use the state variable
		if (v.whatever2_vibDirToggle == 0) { // Direction toggle (0B3B)
			currentPW += lfoSpeed;
			if (currentPW > 0x0E00 || currentPW < lfoSpeed) { // Check wrap around too
				currentPW = 0x0E00;                           // Clamp
				v.whatever2_vibDirToggle = 1;                 // Change direction (0B5D)
			}
		} else {
			// Need signed arithmetic potentially if currentPW could go below lfoSpeed
			if (currentPW >= lfoSpeed) {
				currentPW -= lfoSpeed;
			} else {
				currentPW = 0;
			}
			if (currentPW < 0x0800) {         // Limit check (0B7B)
				currentPW = 0x0800;           // Clamp
				v.whatever2_vibDirToggle = 0; // Change direction (0B7F)
			}
		}
		currentPW &= 0x0FFF;
		if (v.pulseWidth != currentPW) {
			v.pulseWidth = currentPW;
			SID_Write(sidOffset + 2, v.pulseWidth & 0xFF);        // Write PW Lo (0B4A / 0B6C)
			SID_Write(sidOffset + 3, (v.pulseWidth >> 8) & 0x0F); // Write PW Hi (0B55 / 0B77)
			debug(1, "Driller 1: PW LFO Updated PW = %d ($%03X)", v.pulseWidth, v.pulseWidth);
		}
	}

	// Arpeggio (L0B82) - Check 'whatever1' flag
	if (v.whatever1) {
		const uint8_t *arpTable = &arpeggio_data[0]; // Only one table defined

		// Speed calculation from 0B98 - checks counter against 'stuff+5' (arpSpeedHiNibble)
		uint8_t speed = v.arpSpeedHiNibble; // This was set from InstA1[4] or InstA0[5] hi nibble
		if (speed == 0)
			speed = 1; // Avoid division by zero or infinite loop

		v.stuff_arp_counter++;
		if (v.stuff_arp_counter >= speed) {
			v.stuff_arp_counter = 0;
			// Advance arpeggio note index (0BA0 / 0BBA)
			v.stuff_arp_note_index = (v.stuff_arp_note_index + 1) % 3; // Cycle 0, 1, 2
			debug(1, "Driller 1: Arp Step -> Note Index %d", v.stuff_arp_note_index);
		}

		// Calculate arpeggio note (0BA6)
		uint8_t baseNote = v.currentNote; // Note from pattern
		if (baseNote > 0 && baseNote < 96) {
			uint8_t arpOffset = arpTable[v.stuff_arp_note_index]; // Offset from table (0BAA)
			uint8_t arpNote = baseNote + arpOffset;
			if (arpNote >= 96)
				arpNote = 95; // Clamp

			// Set frequency based on arpeggio note
			freq = frq_lo[arpNote] | (frq_hi[arpNote] << 8);
			freqDirty = true;
			// Arpeggio overrides other frequency effects for this frame
			goto WriteFrequency;
		} else {
			// If base note is invalid (e.g., 0), maybe use baseFreq? Or just skip arp?
			// Fall through to allow other effects if arp base note is invalid
		}
	}

	// Vibrato (L0BC0 / L0BC8) - Check 'whatever0' flag
	if (v.whatever0) {
		if (v.things_vib_delay_reload > 0) { // Only run if delay is set

			// --- Fix 3a: Simplify Counter Logic ---
			v.things_vib_delay_ctr--;          // Decrement first
			if (v.things_vib_delay_ctr == 0) { // Check if zero AFTER decrementing
											   // --- End Fix 3a ---

				v.things_vib_delay_ctr = v.things_vib_delay_reload; // Reload counter

				int state = v.things_vib_state;
				int32_t current_freq_signed = v.stuff_freq_porta_vib; // Apply vibrato based on current freq (inc. porta)

				// Use level 1 for this crucial debug message
				debug(1, "Driller V1: Vib Step - State %d, Depth %d", state, (int16_t)v.things_vib_depth);

				// Apply depth based on state (L0C06, L0C2F, L0BD1)
				// ... (rest of vibrato logic is likely okay) ...
				// States 0, 2, 3 are down; State 1, 4 are up
				if (state == 1 || state == 4) { // Up sweep
					current_freq_signed += v.things_vib_depth;
				} else { // Down sweep (0, 2, 3)
					current_freq_signed -= v.things_vib_depth;
				}

				// Clamp frequency after modification
				if (current_freq_signed < 0)
					current_freq_signed = 0;
				if (current_freq_signed > 0xFFFF)
					current_freq_signed = 0xFFFF;
				v.stuff_freq_porta_vib = (uint16_t)current_freq_signed; // Store result for next frame's base
				freq = v.stuff_freq_porta_vib;                          // Use vibrato-modified frequency for this frame
				freqDirty = true;

				// Advance state (0BF4 / 0C29 / 0C52)
				v.things_vib_state++;
				if (v.things_vib_state >= 5) { // Cycle states 0..4 (0BFA)
					v.things_vib_state = 1;    // Loop back to state 1 (upward sweep) (0BFE) - Correct based on diss.
				}
				// Use level 1 for this crucial debug message
				debug(1, "Driller V1: Vib Freq Updated = %d, Next State %d", freq, v.things_vib_state);
			}
		}
	} // end if(v.whatever0)

	// Portamento (L0C5A) - Check 'whatever2' flag
	if (v.whatever2) { // Note: 'else if' removed, allow porta+vib? Keep 'else if'.
		// Calculate porta speed if not already done (or if param changed?)
		if (v.portaSpeed == 0) {            // Calculate only once per porta command
			int16_t speed = v.portaStepRaw; // Raw value from FB/FC command (e.g., 0x01 or 0x80)
			// Disassembly L0C7B (type 1) / L0CA6 (type 2) / L0C96 (type 3) / L0C6B (type 4)
			// Types 1 & 3 are down, 2 & 4 are up. Speed seems absolute value?
			// Let's assume portaStepRaw is the step magnitude.
			if (v.whatever2 == 1 || v.whatever2 == 3) { // Down
				v.portaSpeed = -speed;                  // Ensure negative for down
			} else {                                    // Up (2 or 4)
				v.portaSpeed = speed;                   // Ensure positive for up
			}
			debug(1, "Driller 1: Porta Recalc Speed = %d (Raw=%d, Type=%d)", v.portaSpeed, v.portaStepRaw, v.whatever2);
		}

		// Apply portamento step
		int32_t tempFreqSigned = v.stuff_freq_porta_vib; // Apply to current frequency
		tempFreqSigned += v.portaSpeed;                  // Add signed speed

		// Clamp frequency
		if (tempFreqSigned > 0xFFFF)
			tempFreqSigned = 0xFFFF;
		if (tempFreqSigned < 0)
			tempFreqSigned = 0;

		v.stuff_freq_porta_vib = (uint16_t)tempFreqSigned; // Store result for next frame
		freq = v.stuff_freq_porta_vib;                     // Use the porta-modified frequency for this frame
		freqDirty = true;
		debug(DEBUG_LEVEL >= 3, "Driller: Porta Step -> Freq = %d", freq);
	}

WriteFrequency:
	// Write final frequency to SID if it was changed by effects
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