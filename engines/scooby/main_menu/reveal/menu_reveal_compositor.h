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

#ifndef SCOOBY_MENU_REVEAL_COMPOSITOR_H
#define SCOOBY_MENU_REVEAL_COMPOSITOR_H

#include "common/scummsys.h"

#include "common/array.h"

#include "scooby/assets/rom.h"
#include "scooby/common/span.h"

// Reconstructs packed sprite tiles from the two masked pixel planes consumed by the main-menu
// reveal. The menu keeps row shifting enabled while its mask axes and wave phase change
// independently.
//
// The 39-word row-shift cycle begins at 0x00008828. Wide output uses the complete 600-entry
// permutation at 0x0000B5C0; narrow output uses the complete 256-entry permutation at 0x0000BA70.
// These tables define packed tile order, not hardware workspace ownership.

namespace Scooby {

// Reimplements Ghidra ComposeWideMenuRevealTiles at 0x0000E40C for its decoded 120x40 pixel
// plane, subtractive row wave, and seventy-five-tile output.
// rom: verified cartridge address space containing the masks, shifts, and output order.
// source: the 120x40 one-byte-per-pixel source selected by RunMainMenu.
// rowMaskOffset: mask selected by original word 0xFF0626.
// columnMaskOffset: mask selected by original word 0xFF0624.
// rowShiftStartIndex: first of the 39 wave words selected by original 0xFF000C.
// Returns seventy-five packed 4-bpp tiles in the original sprite-table order.
//
// Equal reveal offsets select 5/15 through 40/120 rows/columns. Once reveal completes, the
// column mask remains fully selected while the row mask follows the menu triangle phase. The
// 2,400-byte result is published at tile indices 0x780-0x7CA.
Common::Array<uint8> ComposeWide(const ScoobyDooRom &rom,
								 Span<const uint8> source,
								 int16 rowMaskOffset, int16 columnMaskOffset,
								 int rowShiftStartIndex);

// Reimplements Ghidra ComposeNarrowMenuRevealTiles at 0x0000F272 for its decoded 64x32 pixel
// plane, additive row wave, and thirty-two-tile output.
// rom: verified cartridge address space containing the masks, shifts, and output order.
// source: the 64x32 one-byte-per-pixel source selected by RunMainMenu.
// rowMaskOffset: mask selected by original word 0xFF0626.
// columnMaskOffset: mask selected by original word 0xFF0624.
// rowShiftStartIndex: first of the 39 wave words selected by original 0xFF000C.
// Returns thirty-two packed 4-bpp tiles in the original sprite-table order.
//
// Equal reveal offsets select 4/8 through 32/64 rows/columns. Reads that cross the source ends
// use the zero guards originally supplied by cleared Ghidra g_abWideMenuAndOpeningWorkspace and
// g_abActorGraphicsAndNarrowMenuWorkspace storage around 0xFF3800 and 0xFF5800. The 1,024-byte
// result is published at tile indices 0x640-0x65F only after wide reveal reaches zero.
Common::Array<uint8> ComposeNarrow(const ScoobyDooRom &rom,
								   Span<const uint8> source,
								   int16 rowMaskOffset, int16 columnMaskOffset,
								   int rowShiftStartIndex);
} // namespace Scooby

#endif // SCOOBY_MENU_REVEAL_COMPOSITOR_H
