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

BOOL CBitmap::Attach(HGDIOBJ hObject) {
	DeleteObject();
	m_hObject = hObject;
	return true;
}

HGDIOBJ CBitmap::Detach() {
	HGDIOBJ result = m_hObject;
	m_hObject = nullptr;
	return result;
}

BOOL CBitmap::CreateCompatibleBitmap(CDC *pDC, int nWidth, int nHeight) {
	error("TODO: CBitmap::CreateCompatibleBitmap");
}

BOOL CBitmap::CreateBitmap(int nWidth, int nHeight, UINT nPlanes,
		UINT nBitcount, const void *lpBits) {
	assert(nPlanes == 1);

	BITMAPINFOHEADER h;
	h.biSize = 40;
	h.biWidth = nWidth;
	h.biHeight = nHeight;
	h.biPlanes = 1;
	h.biBitCount = nBitcount;
	h.biCompression = BI_RGB;
	h.biSizeImage = 0;
	h.biXPelsPerMeter = 0;
	h.biYPelsPerMeter = 0;
	h.biClrUsed = 0;
	h.biClrImportant = 0;

	m_hObject = CreateDIBitmap(nullptr, &h,
		CBM_INIT, lpBits, nullptr, DIB_RGB_COLORS);
	return true;
}

int CBitmap::GetObject(int nCount, LPVOID lpObject) const {
	error("TODO: CBitmap::GetObject");
}

LONG CBitmap::GetBitmapBits(LONG dwCount, LPVOID lpBits) const {
	error("TODO: CBitmap::GetBitmapBits");
}

} // namespace MFC
} // namespace Bagel
