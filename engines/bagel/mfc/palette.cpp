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

#include "common/textconsole.h"
#include "bagel/mfc/afxwin.h"

namespace Bagel {
namespace MFC {

BOOL CPalette::CreatePalette(LPLOGPALETTE lpLogPalette) {
	DeleteObject();
	m_hObject = new Impl(lpLogPalette);
	return true;
}

int CPalette::GetObject(int nCount, LPVOID lpObject) const {
	LOGPALETTE *pal = (LOGPALETTE *)lpObject;
	const Impl *src = static_cast<const Impl *>(m_hObject);
	assert((uint)nCount < 4 + (4 * src->size()));

	for (uint i = 0; i < src->size(); ++i) {
		auto &entry = pal->palPalEntry[i];
		src->get(i, entry.peRed, entry.peGreen, entry.peBlue);
		entry.peFlags = 0;
	}

	return 4 + (4 * src->size());
}

UINT CPalette::GetPaletteEntries(UINT nStartIndex, UINT nNumEntries,
		LPPALETTEENTRY lpPaletteColors) const {
	const Impl *src = static_cast<const Impl *>(m_hObject);

	for (uint i = 0; i < nNumEntries; ++i) {
		auto &entry = lpPaletteColors[i];
		src->get(nStartIndex + i, entry.peRed,
			entry.peGreen, entry.peBlue);
		entry.peFlags = 0;
	}

	return nNumEntries;
}

UINT CPalette::SetPaletteEntries(UINT nStartIndex, UINT nNumEntries,
		LPPALETTEENTRY lpPaletteColors) {
	Impl *pal = static_cast<Impl *>(m_hObject);

	for (uint i = 0; i < nNumEntries; ++i) {
		auto &entry = lpPaletteColors[i];
		pal->set(nStartIndex + i, entry.peRed,
			entry.peGreen, entry.peBlue);
	}

	return nNumEntries;
}

BOOL CPalette::AnimatePalette(UINT nStartIndex, UINT nNumEntries,
		const PALETTEENTRY *lpPaletteColors) {
	error("TODO: CPalette::AnimatePalette");
}

UINT CPalette::GetNearestPaletteIndex(COLORREF crColor) {
	const Impl *src = static_cast<const Impl *>(m_hObject);

	return src->findBestColor(
		GetRValue(crColor),
		GetGValue(crColor),
		GetBValue(crColor)
	);
}

/*--------------------------------------------*/

CPalette::Impl::Impl(const LPLOGPALETTE pal) :
	CGdiObjectImpl(), Graphics::Palette(pal->palNumEntries) {
	for (uint i = 0; i < size(); ++i) {
		auto &e = pal->palPalEntry;
		set(i, e->peRed, e->peGreen, e->peBlue);
	}
}

} // namespace MFC
} // namespace Bagel
