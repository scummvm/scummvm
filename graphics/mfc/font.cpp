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
#include "graphics/mfc/afxwin.h"

namespace Graphics {
namespace MFC {

bool CFont::CreateFont(int nHeight, int nWidth, int nEscapement,
        int nOrientation, int nWeight, byte bItalic, byte bUnderline,
        byte cStrikeOut, byte nCharSet, byte nOutPrecision,
        byte nClipPrecision, byte nQuality, byte nPitchAndFamily,
        const char *lpszFacename) {
	m_hObject = AfxGetApp()->createFont(nHeight, nWidth,
		nEscapement, nOrientation, nWeight, bItalic,
		bUnderline, cStrikeOut, nCharSet, nOutPrecision,
		nClipPrecision, nQuality, nPitchAndFamily,
		lpszFacename);
	assert(m_hObject);

	// This is where it becomes permanent
	AfxHookObject();
	return true;
}

bool CFont::CreateFontIndirect(const LOGFONT *lpLogFont) {
	assert(lpLogFont != nullptr);

	m_hObject = MFC::CreateFontIndirect(lpLogFont);

	if (m_hObject == nullptr)
		return false;

	// This is where it becomes permanent
	AfxHookObject();
	return true;
}

} // namespace MFC
} // namespace Graphics
