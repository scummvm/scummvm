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

#include "common/system.h"
#include "graphics/paletteman.h"
#include "bagel/mfc/dc.h"
#include "bagel/bagel.h"

namespace Bagel {
namespace MFC {

BOOL CPen::CreatePen(int nPenStyle, int nWidth, COLORREF crColor) {
	error("TODO: CPen::CreatePen");
}


void CBrush::CreateSolidBrush(COLORREF color) {
	error("TODO: CBrush::CreateSolidBrush");
}

void CBrush::CreateStockObject(int brush) {
	error("TODO: CBrush::CreateStockObject");
}


CDC::~CDC() {
	delete _pen;
	delete _brush;
}

int CDC::GetDeviceCaps(int field) const {
	switch (field) {
	case HORZRES:
		return 640;
	case VERTRES:
		return 480;
	default:
		return 0;
	}
}

CPalette *CDC::SelectPalette(CPalette *pPalette, bool bForceBackground) {
	assert(!bForceBackground);
	_palette = *pPalette;
	return &_palette;
}

void CDC::RealizePalette() {
	g_system->getPaletteManager()->setPalette(_palette.data(), 0, _palette.size());
}

void CDC::Attach(HDC dc) {
	m_hDC = dc;
}

void CDC::Detach() {
	m_hDC = nullptr;
}

bool CDC::BitBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
		int xSrc, int ySrc, uint32 dwRop) {
#ifdef TODO
	Graphics::Screen &screen = *g_engine->_screen;
	const Common::Rect srcRect(xSrc, ySrc, xSrc + nWidth, ySrc + nHeight);
	const Common::Rect destRect(x, y, x + nWidth, y + nHeight);

	screen.blitFrom(*pSrcDC, srcRect, destRect);
#endif
	return true;
}

CPen *CDC::SelectObject(CPen *pen) {
	SWAP(pen, _pen);
	return pen;
}

CBrush *CDC::SelectObject(CBrush *brush) {
	SWAP(brush, _brush);
	return brush;
}

void CDC::Rectangle(int x1, int y1, int x2, int y2) {
	error("TODO: CDC::Rectangle");
}

void CDC::Rectangle(const LPRECT rect) {
	error("TODO: CDC::Rectangle");
}

void CDC::MoveTo(int x, int y) {
	error("TODO: CDC::MoveTo");
}

void CDC::LineTo(int x, int y) {
	error("TODO: CDC::LineTo");
}

int CDC::FrameRect(LPCRECT lpRect, CBrush *pBrush) {
	error("TODO: CDC::FrameRect");
	return 0;
}

void CDC::Ellipse(LPCRECT lpRect) {
	error("TODO: CDC::Ellipse");
}

int CDC::SetBkMode(int nBkMode) {
	error("TODO: CDC::SetBkMode");
	return 0;
}

BOOL CDC::GetTextMetrics(LPTEXTMETRIC lptm) {
	error("TODO: CDC::GetTextMetrics");
	return 0;
}

CSize CDC::GetTextExtent(const char *lpszString) const {
	error("TODO: CDC::GetTextExtent");
	return CSize();
}

CSize CDC::GetTextExtent(const char *lpszString, int nCount) const {
	error("TODO: CDC::GetTextExtent");
	return CSize();
}

CSize CDC::GetTextExtent(const CString &str) const {
	error("TODO: CDC::GetTextExtent");
	return CSize();
}

CSize CDC::GetTextExtent(const wchar_t *lpszString, int nCount) const {
	error("TODO: CDC::GetTextExtent");
	return CSize();
}

int CDC::SetTextColor(int nTextColor) {
	error("TODO: CDC::SetTextColor");
	return 0;
}

BOOL CDC::TextOut(int x, int y, const char *lpszString) {
	error("TODO: CDC::TextOut");
	return true;
}

BOOL CDC::TextOut(int x, int y, const char *lpszString, int nCount) {
	error("TODO: CDC::TextOut");
	return true;
}

} // namespace MFC
} // namespace Bagel
