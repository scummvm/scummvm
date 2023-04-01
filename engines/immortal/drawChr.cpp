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

#include "immortal/immortal.h"

/* -- How does tile image construction work --
 * Tiles in the source are quite complex, and the way they are drawn
 * to the screen makes use of some very low level opcode trickery.
 * To that end, here is the data structure in the source:
 * Files: .CNM (Character Number Map), .UNV (Universe)
 * .CNM = Logical CNM, uncompressed
 * .UNV = Universe parameters, uncompressed + CNM/CBM, compressed
 * Logical CNM = Draw type info about the CNM (indexes into a table used when constructing the complete CNM)
 * Universe parameters = Bounds, animations (unused), Columns, Rows, Chrs, Cells
 * CNM = Map of all cells in the level, with each cell being an index to the Chr in the CBM
 * CBM = All gfx data for the level, stored in 'Chrs' which are 4x2 tiles, with each tile being 2x2 blocks of 8x8 pixel gfx data,
 * stored in Linear Reversed Chunk gfx, *not* bitplane data.
 *
 * Data Structures: CNM, Solid, Left, Right, Draw
 * CNM = Unchanged from the uncompressed file data
 * Solid/Left/Right = [2 bytes] index for screen clipping, [2 bytes] index to position in Draw where the chr gfx start
 * Draw = Series of microroutines that use PEA to move pixel data directly to screen buffer. For Left/Right versions,
 * the routine draws only half of the tile data, divided by a diagonal in one of ULHC/LLHC/URHC/LRHC
 *
 * Tile Structures:
 * Chr = This term is used for multiple things throughout the source, but in this context it is the entire 8x4 set of 8x8 pixel gfx data
 * Block = Not a term used in the source, but is what describes the 8x8 pixel gfx data
 *
 * So the way this works is that after uncompressing the CNM and CBM data,
 * the game converts the pixel data into the microroutines (mungeX()) and builds the Solid/Left/Right routines.
 * Then, when tile drawing needs to happen, the drawing routines (drawX()) will take the chr as an index into Solid/Left/Right,
 * and use the clip data to find a starting scanline, which it can use to determine where the stack will point to.
 * With the stack pointing to the screen buffer, the game will jump to the microroutine (called Linear Coded Chr Routines in the source)
 * and execute the code, moving the pixel data to the screen.
 *
 * So how does it change for this port?
 * Well we can't replicate that PEA trick, so we'll have to ditch the microroutines. However,
 * we will keep the same general structure. Instead of converting the pixel data to routines,
 * we instead convert it into the type of pixel data needed by the ScummVM screen buffer,
 * ie. change 1 pixel per nyble to 1 pixel per byte. However, it gets more complicated in how
 * it has to be stored. In the source, reading the pixel data doesn't have to account for how
 * many pixels to read per scanline, because it is executing code which will end the scanline
 * whenever it has been written to do so. In our case, we must instead rely on the pixel reading
 * function to read the same number of pixels per scanline that we put into the data structure
 * when converting it from the raw pixel data.
 * So now we have:
 * Draw: A vector of Chrs
 * Chr: A set of 32 scan lines
 * Scanline: A byte buffer containing anywhere from 1 to 32 bytes of pixel data
 */

namespace Immortal {

void ImmortalEngine::drawSolid(int chr, int x, int y) {
	/* Okay, this routine is quite different here compared to
	 * the source, so I'll explain:
	 * At this point in the source, you had an array of linear
	 * coded chr routines that draw pixels by directly storing
	 * byte values to the screen buffer. These routines would
	 * get executed from these drawX() routines. It didn't need
	 * to worry about how many pixels per line for example,
	 * because the code would simply stop sending pixels to the
	 * screen when it reached the end of the scanline (this is
	 * important for the other drawX() routines). In our case,
	 * we instead have a table of structs that contain an array
	 * of scan lines, each with 1 - 32 bytes of pixel data
	 * that is already formatted for use by ScummVM's screen buffer.
	 * These drawX() functions must now draw the pixels from that
	 * data.
	 */

	// This will need clipping later
	int index = _Solid[chr];
	int point = (y * kResH) + x;

	// For every scanline, draw the pixels and move down by the size of the screen
	for (int cy = 0; cy < 32; cy++, point += kResH) {

		// For solid, we always have 64 pixels in each scanline
		for (int cx = 0; cx < 64; cx++) {
			_screenBuff[point + cx] = _Draw[index]._scanlines[cy][cx];
		}
	}
}

void ImmortalEngine::drawLRHC(int chr, int x, int y) {
	// This will need clipping later
	int index = _Right[chr];
	int point = (y * kResH) + x;

	for (int cy = 0; cy < 32; cy++, point += kResH) {

		// We only want to draw the amount of pixels based on the number of lines down the tile
		for (int cx = 0; cx < (2 * (cy + 1)); cx++) {
			_screenBuff[point + cx + (64 - (2 * (cy + 1)))] = _Draw[index]._scanlines[cy][cx];
		}
	}
}

void ImmortalEngine::drawLLHC(int chr, int x, int y) {
	// This will need clipping later
	int index = _Left[chr];
	int point = (y * kResH) + x;

	for (int cy = 0; cy < 32; cy++, point += kResH) {
		for (int cx = 0; cx < (2 * (cy + 1)); cx++) {
			_screenBuff[point + cx] = _Draw[index]._scanlines[cy][cx];
		}
	}
}

void ImmortalEngine::drawULHC(int chr, int x, int y) {
	// This will need clipping later
	int index = _Right[chr];
	int point = (y * kResH) + x;

	for (int cy = 0; cy < 32; cy++, point += kResH) {
		for (int cx = 0; cx < (64 - (cy * 2)); cx++) {
			_screenBuff[point + cx] = _Draw[index]._scanlines[cy][cx];
		}
	}
}

void ImmortalEngine::drawURHC(int chr, int x, int y) {
	// This will need clipping later
	int index = _Left[chr];
	int point = (y * kResH) + x;

	for (int cy = 0; cy < 32; cy++, point += kResH) {
		for (int cx = 0; cx < (64 - (cy * 2)); cx++) {
			_screenBuff[point + cx + (cy * 2)] = _Draw[index]._scanlines[cy][cx];
		}
	}	
}

int ImmortalEngine::mungeCBM(uint16 num2Chrs) {
	const uint16 kTBlisterCorners[60] = {7, 1, 1, 1, 1, 1, 5, 3, 1, 1, 1, 1, 1, 3, 5, 3, 5, 1, 1, 1,
		                                 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 8, 8, 8, 8, 16, 16, 16, 16, 8,
		                                 8, 8, 8, 16, 16, 16, 16, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
		                                 };

	// Is this missing an entry? There should be 20 columns, and this is only 19...
	const uint16 kTLogicalCorners[19] = {1, 1, 1, 1, 16, 8, 1, 8,
		                                 16, 1, 1, 8, 1, 16, 8, 16,
		                                 1, 16, 8
		                                 };

	// Each tile is 1024 bytes, so the oldCBM is 1024 * number of tiles
	int lCBM = k1K * _univ->_numChrs;
	_oldCBM = (byte *)malloc(lCBM);

	//debug("Length of CBM: %d", lCBM);

	// Now we get the CBM from the file
	_dataBuffer->seek(_univ->_num2Cells);
	_dataBuffer->read(_oldCBM, lCBM);

	// And now we need to set up the data structures that will be used to expand the CNM/CBM
	// Each Chr needs a 'solid' function, but some also need a 'left' and 'right' function as well
	// So each one needs to be the size of as many Chrs as you have
	_Solid = (uint16 *)malloc(num2Chrs);
	_Right = (uint16 *)malloc(num2Chrs);
	_Left  = (uint16 *)malloc(num2Chrs);

	// _Draw is actually going to be a length that depends on the CNM, so we need it to be a vector

	// In the source, this does all 3 at once, but we have them in separate variables
	uint16 *lists[3] = {_Solid, _Right, _Left};
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < _univ->_numChrs; j++) {
			lists[i][j] = 0;
		}
	}

	uint16 cell = 0;
	uint16 chr  = 0;
	uint16 corners = 0;
	int oldChr = 0;
	uint16 drawIndex = 0;

	// Loop over every cell in the CNM, and set up the draw routines for each draw type needed
	do {
		// The CNM has the chr number in each cell
		chr = _CNM[cell];

		// Have we already done it?
		if (_Solid[chr] == 0) {
			
			// Mark it as done even if not solid presumably for future checks on the solid chr list?
			_Solid[chr] = 1;

			// If cell is within the first 3 rows, and cell 17 is 0
			if ((cell >= (_univ->_numCols)) && (cell < (_univ->_numCols * 3)) && (_CNM[17] != 0)) {
				// Then we get it from a table because this is the water level
				corners = kTBlisterCorners[cell];

			} else {
				// Otherwise we get it from a table indexed by the entry in the logical CNM
				corners = kTLogicalCorners[_logicalCNM[cell]];
			}

			// In the source this is actually asl 5 : asl + rol 5, but we can just use an int instead of 2 uint16s
			oldChr = chr * 1024;

			// Corners determines whether we create a _Draw entry for just solid, or diagonals as well
			if ((corners & 1) != 0) {
				storeAddr(_Solid, chr, drawIndex);
				mungeSolid(oldChr, drawIndex);
			}

			if ((corners & 2) != 0) {
				storeAddr(_Left, chr, drawIndex);
				mungeLLHC(oldChr, drawIndex);
			}

			if ((corners & 4) != 0) {
				storeAddr(_Right, chr, drawIndex);
				mungeLRHC(oldChr, drawIndex);
			}

			if ((corners & 8) != 0) {
				storeAddr(_Left, chr, drawIndex);
				mungeURHC(oldChr, drawIndex);
			}

			if ((corners & 16) != 0) {
				storeAddr(_Right, chr, drawIndex);
				mungeULHC(oldChr, drawIndex);
			}
		}

		cell++;
	} while (cell != (_univ->_num2Cells / 2));
	
	// Finally just return the size of the draw table, which is essentially the expanded CBM
	return _Draw.size();
}

void ImmortalEngine::storeAddr(uint16 *drawType, uint16 chr, uint16 drawIndex) {
	// The entry at chr2 is the index into the draw table
	// In the source this required checking bank boundries, luckily that's not relevant here
	drawType[chr] = drawIndex;
}

void ImmortalEngine::mungeSolid(int oldChr, uint16 &drawIndex) {
	// We need a Chr for the entry in the draw table
	Chr chrData;

	// This is a little different from the source, because the source creates the linear coded chr routines
	// So here we are just grabbing the pixel data in the normal way

	// For every line of pixels in the chr
	for (int py = 0; py < 32; py++) {
		// Each scanline needs a byte buffer
		byte *scanline = (byte *)malloc(64);

		// For every pixel in the line, we extract the data from the oldCBM
		for (int px = 0; px < 64; px += 2) {

			/* Pixels are stored in Linear Reversed Chunk format
			 * Which is 2 pixels per byte, stored in reverse order.
			 */
			scanline[px] = (_oldCBM[oldChr] & kMask8High) >> 4;
			scanline[px + 1] = (_oldCBM[oldChr] & kMask8Low);
			oldChr++;
		}
		// Now we add the byte buffer into the chrData
		chrData._scanlines[py] = scanline;
	}
	// And we add the chrData into the draw table
	_Draw.push_back(chrData);
	drawIndex++;
}

void ImmortalEngine::mungeLRHC(int oldChr, uint16 &drawIndex) {
	Chr chrData;

	for (int py = 0; py < 32; py++) {
		byte *scanline = (byte *)malloc(2 * (py + 1));
		oldChr += (32 - (py + 1));

		for (int px = 0; px < (2 * (py + 1)); px += 2) {
			scanline[px] = (_oldCBM[oldChr] & kMask8High) >> 4;
			scanline[px + 1] = (_oldCBM[oldChr] & kMask8Low);
			oldChr++;
		}
		chrData._scanlines[py] = scanline;
	}	
	_Draw.push_back(chrData);
	drawIndex++;
}

void ImmortalEngine::mungeLLHC(int oldChr, uint16 &drawIndex) {
	Chr chrData;

	for (int py = 0; py < 32; py++) {
		byte *scanline = (byte *)malloc(2 * (py + 1));

		for (int px = 0; px < (2 * (py + 1)); px += 2) {
			scanline[px] = (_oldCBM[oldChr] & kMask8High) >> 4;
			scanline[px + 1] = (_oldCBM[oldChr] & kMask8Low);
			oldChr++;
		}
		oldChr += (32 - (py + 1));
		chrData._scanlines[py] = scanline;
	}
	_Draw.push_back(chrData);
	drawIndex++;
}

void ImmortalEngine::mungeULHC(int oldChr, uint16 &drawIndex) {
	Chr chrData;

	for (int py = 0; py < 32; py++) {
		byte *scanline = (byte *)malloc(64 - ((py + 1) * 2));

		for (int px = 0; px < (64 - ((py + 1) * 2)); px += 2) {
			scanline[px] = (_oldCBM[oldChr] & kMask8High) >> 4;
			scanline[px + 1] = (_oldCBM[oldChr] & kMask8Low);
			oldChr++;
		}
		oldChr += (py + 1);
		chrData._scanlines[py] = scanline;
	}
	_Draw.push_back(chrData);
	drawIndex++;
}

void ImmortalEngine::mungeURHC(int oldChr, uint16 &drawIndex) {
	Chr chrData;

	for (int py = 0; py < 32; py++) {
		byte *scanline = (byte *)malloc(64 - (py * 2));

		for (int px = 0; px < (64 - (py * 2)); px += 2) {
			scanline[px] = (_oldCBM[oldChr] & kMask8High) >> 4;
			scanline[px + 1] = (_oldCBM[oldChr] & kMask8Low);
			oldChr++;
		}
		oldChr += (py + 1);
		chrData._scanlines[py] = scanline;
	}
	_Draw.push_back(chrData);
	drawIndex++;
}

} // namespace Immortal
