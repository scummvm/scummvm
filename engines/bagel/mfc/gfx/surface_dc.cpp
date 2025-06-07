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

#include "bagel/mfc/gfx/surface_dc.h"
#include "bagel/mfc/gfx/blitter.h"
#include "bagel/mfc/afxwin.h"

namespace Bagel {
namespace MFC {
namespace Gfx {

SurfaceDC::SurfaceDC() {
}

SurfaceDC::~SurfaceDC() {
	delete _bitmap;
}

HBITMAP SurfaceDC::Attach(HBITMAP bitmap) {
	HBITMAP result = _bitmap;
	_bitmap = bitmap;
	return result;
}

void SurfaceDC::Detach() {
	_bitmap = nullptr;
}

Graphics::ManagedSurface *SurfaceDC::getSurface() const {
	assert(_bitmap);
	return static_cast<CBitmap::Impl *>(_bitmap);
}

HPALETTE SurfaceDC::selectPalette(HPALETTE pal) {
	HPALETTE oldPal = _palette;
	_palette = pal;

	CBitmap::Impl *bitmap = (CBitmap::Impl *)_bitmap;

	if (pal) {
		auto *newPal = static_cast<CPalette::Impl *>(pal);
		if (bitmap)
			bitmap->setPalette(newPal->data(), 0, newPal->size());
	} else {
		if (bitmap)
			bitmap->clearPalette();
	}

	return oldPal;
}

CPalette *SurfaceDC::selectPalette(CPalette *pal) {
	CPalette *oldPal = _cPalette;
	_cPalette = pal;
	selectPalette((HPALETTE)_cPalette->m_hObject);
	return oldPal;
}


UINT SurfaceDC::realizePalette() {
	const auto *pal = static_cast<const CPalette::Impl *>(_palette);
	AfxGetApp()->setPalette(*pal);
	return 256;
}

COLORREF SurfaceDC::GetNearestColor(COLORREF crColor) const {
	if (crColor <= 255)
		return crColor;

	const auto *pal = static_cast<const CPalette::Impl *>(_palette);
	return pal->findBestColor(
		GetRValue(crColor),
		GetGValue(crColor),
		GetBValue(crColor));
}

void SurfaceDC::fillRect(const Common::Rect &r, COLORREF crColor) {
	assert(_bitmap);
	static_cast<CBitmap::Impl *>(_bitmap)->fillRect(r,
		GetNearestColor(crColor));
}

void SurfaceDC::bitBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
		int xSrc, int ySrc, DWORD dwRop) {
	Graphics::ManagedSurface *src = pSrcDC->surface()->getSurface();
	Graphics::ManagedSurface *dest = getSurface();
	const Common::Rect srcRect(xSrc, ySrc, xSrc + nWidth, ySrc + nHeight);
	const Common::Point destPos(x, y);

	if (dwRop == SRCCOPY) {
		dest->blitFrom(*src, srcRect, destPos);
	} else {
		blit(src, dest, srcRect, destPos, dwRop);
	}
}

void SurfaceDC::stretchBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
		int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, DWORD dwRop) {
	Graphics::ManagedSurface *src = pSrcDC->surface()->getSurface();
	Graphics::ManagedSurface *dest = getSurface();
	const Common::Rect srcRect(xSrc, ySrc, xSrc + nSrcWidth, ySrc + nSrcHeight);
	const Common::Rect destRect(x, y, x + nWidth, y + nHeight);

	stretchBlit(src, dest, srcRect, destRect, dwRop);
}

} // namespace Gfx
} // namespace MFC
} // namespace Bagel
