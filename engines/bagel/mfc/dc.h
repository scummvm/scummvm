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

#ifndef BAGEL_HODJNPODJ_MFC_DC_H
#define BAGEL_HODJNPODJ_MFC_DC_H

#include "bagel/spacebar/boflib/gfx/palette.h"

namespace Bagel {
namespace HodjNPodj {

enum DeviceCaps {
	HORZRES, VERTRES
};
enum {
	SRCCOPY
};

class CDC;
typedef CDC *HDC;

class CDC {
private:
	CPalette _palette;

public:
	HDC m_hDC;

public:
	CDC() : m_hDC(this) {}

	int GetDeviceCaps(int field) const;
	CPalette *SelectPalette(CPalette *pPalette, bool bForceBackground);
	void RealizePalette();

	/**
	 * Performs a bit-block transfer of bitmaps
	 * @param x			Destination x
	 * @param y			Destination y
	 * @param nWidth	Width of area
	 * @param nHeight	Height of area
	 * @param pSrcDC	Source bitmap
	 * @param xSrc		Source x co-ordinate
	 * @param ySrc		Source y co-ordinate
	 * @param dwRop		Raster operation code
	 * @return		Returns true if successful
	 */
	bool BitBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
		int xSrc, int ySrc, uint32 dwRop);
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
