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
#include "bagel/mfc/afxwin.h"

namespace Bagel {
namespace MFC {
namespace Gfx {

SurfaceDC::SurfaceDC(CWnd *owner) : _owner(owner) {
	Graphics::Screen *screen = AfxGetApp()->getScreen();
	create(*screen, Common::Rect(0, 0, screen->w, screen->h));
}

HPALETTE SurfaceDC::selectPalette(HPALETTE pal) {
	HPALETTE oldPal = _palette;
	_palette = pal;

	if (pal) {
		auto *newPal = static_cast<CPalette::Impl *>(pal);
		setPalette(newPal->data(), 0, newPal->size());
	} else {
		clearPalette();
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
	const auto *pal = static_cast<const CPalette::Impl *>(_palette);
	return pal->findBestColor(
		GetRValue(crColor),
		GetGValue(crColor),
		GetBValue(crColor));
}


} // namespace Gfx
} // namespace MFC
} // namespace Bagel
