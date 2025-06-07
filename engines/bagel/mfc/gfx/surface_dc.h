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

#ifndef BAGEL_MFC_GFX_SURFACE_DC_H
#define BAGEL_MFC_GFX_SURFACE_DC_H

#include "graphics/managed_surface.h"
#include "bagel/mfc/minwindef.h"

namespace Bagel {
namespace MFC {

class CDC;
class CWnd;
class CPalette;

namespace Gfx {

class SurfaceDC {
public:
	HBITMAP _bitmap = nullptr;
	HPALETTE _palette = nullptr;
	CPalette *_cPalette = nullptr;

public:
	SurfaceDC();
	~SurfaceDC();

	HGDIOBJ Attach(HGDIOBJ gdiObj);
	Graphics::ManagedSurface *getSurface() const;

	HPALETTE selectPalette(HPALETTE pal);
	CPalette *selectPalette(CPalette *pal);
	UINT realizePalette();
	COLORREF GetNearestColor(COLORREF crColor) const;

	void fillRect(const Common::Rect &r, COLORREF crColor);
	void bitBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
		int xSrc, int ySrc, DWORD dwRop);
	void stretchBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
		int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, DWORD dwRop);
};

} // namespace Gfx
} // namespace MFC
} // namespace Bagel

#endif
