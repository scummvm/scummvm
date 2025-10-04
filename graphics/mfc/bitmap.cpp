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

bool CBitmap::Attach(HBITMAP hObject) {
	return CGdiObject::Attach(hObject);
}

HBITMAP CBitmap::Detach() {
	return (HBITMAP)CGdiObject::Detach();
}

bool CBitmap::CreateCompatibleBitmap(CDC *pDC, int nWidth, int nHeight) {
	const CDC::Impl *dc = static_cast<CDC::Impl *>(pDC->m_hDC);
	Gfx::Surface *src = dc->getSurface();

	BITMAPINFOHEADER h;
	h.biSize = 40;
	h.biWidth = nWidth;
	h.biHeight = nHeight;
	h.biPlanes = 1;
	h.biBitCount = src->format.bytesPerPixel * 8;
	h.biCompression = BI_RGB;
	h.biSizeImage = 0;
	h.biXPelsPerMeter = 0;
	h.biYPelsPerMeter = 0;
	h.biClrUsed = 0;
	h.biClrImportant = 0;

	m_hObject = CreateDIBitmap(nullptr, &h,
		0, nullptr, nullptr, DIB_RGB_COLORS);

	// This is where it becomes permanent
	AfxHookObject();
	return true;
}

bool CBitmap::CreateBitmap(int nWidth, int nHeight, unsigned int nPlanes,
		unsigned int nBitcount, const void *lpBits) {
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
		lpBits ? CBM_INIT : 0, lpBits, nullptr, DIB_RGB_COLORS);

	// This is where it becomes permanent
	AfxHookObject();
	return true;
}

int CBitmap::GetObject(int nCount, void *lpObject) const {
	CBitmap::Impl *src = static_cast<CBitmap::Impl *>(m_hObject);
	BITMAP *dest = (BITMAP *)lpObject;
	assert(src && nCount == sizeof(BITMAP));

	dest->bmType = 0;
	dest->bmWidth = src->w;
	dest->bmHeight = src->h;
	dest->bmWidthBytes = src->pitch * src->format.bytesPerPixel;
	dest->bmPlanes = 1;
	dest->bmBitsPixel = src->format.bpp();
	dest->bmBits = src->getPixels();

	return sizeof(BITMAP);
}

long CBitmap::GetBitmapBits(long dwCount, void *lpBits) const {
	const CBitmap::Impl *src = static_cast<CBitmap::Impl *>(m_hObject);
	dwCount = MIN<int32>((int32)dwCount, src->pitch * src->h * src->format.bytesPerPixel);

	Common::copy((const byte *)src->getPixels(),
		(const byte *)src->getPixels() + dwCount,
		(byte *)lpBits);
	return dwCount;
}

} // namespace MFC
} // namespace Graphics
