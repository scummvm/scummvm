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

bool CPalette::CreatePalette(LPLOGPALETTE lpLogPalette) {
	DeleteObject();
	m_hObject = new Impl(lpLogPalette);

	// This is where it becomes permanent
	AfxHookObject();

	return true;
}

int CPalette::GetObject(int nCount, void *lpObject) const {
	LOGPALETTE *pal = (LOGPALETTE *)lpObject;
	const Impl *src = static_cast<const Impl *>(m_hObject);
	assert(nCount >= 4);

	pal->palVersion = 0x300;
	nCount -= 2;
	pal->palNumEntries = src->size();
	nCount -= 2;

	uint i;
	for (i = 0; i < src->size() && nCount > 0; ++i, nCount -= 4) {
		auto &entry = pal->palPalEntry[i];
		src->get(i, entry.peRed, entry.peGreen, entry.peBlue);
		entry.peFlags = 0;
	}

	return 4 + (4 * i);
}

unsigned int CPalette::GetEntryCount() const {
	const Impl *src = static_cast<const Impl *>(m_hObject);
	return src->size();
}

unsigned int CPalette::GetPaletteEntries(unsigned int nStartIndex, unsigned int nNumEntries,
		LPPALETTEENTRY lpPaletteColors) const {
	const Impl *src = static_cast<const Impl *>(m_hObject);

	for (uint i = 0; i < nNumEntries; ++i) {
		auto &entry = lpPaletteColors[i];
		src->get(nStartIndex + i, entry.peRed,
			entry.peGreen, entry.peBlue);
		entry.peFlags = 0;
	}

	return src->size();
}

unsigned int CPalette::SetPaletteEntries(unsigned int nStartIndex, unsigned int nNumEntries,
		LPPALETTEENTRY lpPaletteColors) {
	Impl *pal = static_cast<Impl *>(m_hObject);

	for (uint i = 0; i < nNumEntries; ++i) {
		auto &entry = lpPaletteColors[i];
		pal->set(nStartIndex + i, entry.peRed,
			entry.peGreen, entry.peBlue);
	}

	return nNumEntries;
}

unsigned int CPalette::SetPaletteEntries(const Graphics::Palette &pal) {
	Graphics::Palette *impl = static_cast<Impl *>(m_hObject);
	*impl = pal;
	return pal.size();
}

bool CPalette::AnimatePalette(unsigned int nStartIndex, unsigned int nNumEntries,
		const PALETTEENTRY *lpPaletteColors) {
	// Set the new colors
	Graphics::Palette *impl = static_cast<Impl *>(m_hObject);
	for (; nNumEntries > 0; ++nStartIndex, --nNumEntries,
			++lpPaletteColors) {
		impl->set(nStartIndex, lpPaletteColors->peRed,
			lpPaletteColors->peGreen,
			lpPaletteColors->peBlue);
	}

	// Set the new system palette
	AfxGetApp()->setPalette(*impl);

	return true;
}

unsigned int CPalette::GetNearestPaletteIndex(COLORREF crColor) {
	return palette()->findBestColor(
		GetRValue(crColor),
		GetGValue(crColor),
		GetBValue(crColor)
	);
}

/*--------------------------------------------*/

CPalette::Impl::Impl(const LPLOGPALETTE pal) :
	CGdiObjectImpl(), Graphics::Palette(pal->palNumEntries) {
	for (uint i = 0; i < size(); ++i) {
		const auto &e = pal->palPalEntry[i];
		set(i, e.peRed, e.peGreen, e.peBlue);
	}
}

} // namespace MFC
} // namespace Bagel
