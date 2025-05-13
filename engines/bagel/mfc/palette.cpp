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
	error("TODO: CPalette::CreatePalette");
}

int CPalette::GetObject(int nCount, LPVOID lpObject) const {
	error("TODO: CPalette::GetObject");
}

UINT CPalette::GetPaletteEntries(UINT nStartIndex, UINT nNumEntries,
		LPPALETTEENTRY lpPaletteColors) const {
	error("TODO: CPalette::GetPaletteEntries");
}

UINT CPalette::SetPaletteEntries(UINT nStartIndex, UINT nNumEntries,
		LPPALETTEENTRY lpPaletteColors) {
	error("TODO: CPalette::SetPaletteEntries");
}

BOOL CPalette::AnimatePalette(UINT nStartIndex, UINT nNumEntries,
		const PALETTEENTRY *lpPaletteColors) {
	error("TODO: CPalette::AnimatePalette");
}

UINT CPalette::GetNearestPaletteIndex(COLORREF crColor) {
	error("TODO: CPalette::GetNearestPaletteIndex");
}

} // namespace MFC
} // namespace Bagel
