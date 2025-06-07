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
#include "common/textconsole.h"
#include "bagel/mfc/afxwin.h"
#include "bagel/mfc/gfx/surface_dc.h"

namespace Bagel {
namespace MFC {

IMPLEMENT_DYNAMIC(CDC, CObject)

CDC *CDC::FromHandle(HDC hDC) {
//	Gfx::SurfaceDC *surf = static_cast<Gfx::SurfaceDC *>(hDC);
//	return &wnd->_dc;
	error("TODO: CDC::FromHandle");
}

BOOL CDC::CreateDC(LPCSTR lpszDriverName, LPCSTR lpszDeviceName,
                   LPCSTR lpszOutput, const void *lpInitData) {
	error("TODO: CDC::CreateDC");
}

BOOL CDC::CreateCompatibleDC(CDC *pDC) {
	m_hDC = pDC->m_hDC;
	return true;
}

BOOL CDC::DeleteDC() {
	delete m_hDC;
	m_hDC = nullptr;
	return true;
}

BOOL CDC::Attach(HDC hDC) {
	m_hDC = hDC;
	return true;
}

void CDC::Detach() {
	error("TODO: CDC::Detach");
}

int CDC::SetStretchBltMode(int nStretchMode) {
	error("TODO: CDC::SetStretchBltMode");
}

int CDC::GetDeviceCaps(int nIndex) const {
	const Graphics::PixelFormat format =
	    g_system->getScreenFormat();

	switch (nIndex) {
	case BITSPIXEL:
		return format.bytesPerPixel * 8;
	case RASTERCAPS:
		return (format.bytesPerPixel == 1 ? RC_PALETTE : 0) |
		       RC_BITBLT |
		       RC_BITMAP64 |
		       RC_DI_BITMAP |
		       RC_DIBTODEV |
		       RC_PALETTE |
		       RC_STRETCHBLT;

	default:
		break;
	}

	error("TODO: CDC::GetDeviceCaps");
	return 0;
}

int CDC::GetMapMode() const {
	error("TODO: CDC::GetMapMode");
}

CPoint CDC::GetViewportOrg() const {
	error("TODO: CDC::GetViewportOrg");
}

int CDC::SetMapMode(int nMapMode) {
	error("TODO: CDC::SetMapMode");
}

CPoint CDC::SetViewportOrg(int x, int y) {
	error("TODO: CDC::SetViewportOrg");
}

CPoint CDC::SetViewportOrg(POINT point) {
	error("TODO: CDC::SetViewportOrg");
}

CPoint CDC::OffsetViewportOrg(int nWidth, int nHeight) {
	error("TODO: CDC::OffsetViewportOrg");
}

int CDC::GetClipBox(LPRECT lpRect) const {
	error("TODO: CDC::GetClipBox");
}

BOOL CDC::PtVisible(int x, int y) const {
	error("TODO: CDC::PtVisible");
}

BOOL CDC::PtVisible(POINT point) const {
	error("TODO: CDC::PtVisible");
}

BOOL CDC::RectVisible(LPCRECT lpRect) const {
	error("TODO: CDC::RectVisible");
}

int CDC::SelectClipRgn(CRgn *pRgn) {
	error("TODO: CDC::SelectClipRgn");
}

int CDC::ExcludeClipRect(int x1, int y1, int x2, int y2) {
	error("TODO: CDC::ExcludeClipRect");
}

int CDC::ExcludeClipRect(LPCRECT lpRect) {
	error("TODO: CDC::ExcludeClipRect");
}

int CDC::ExcludeUpdateRgn(CWnd *pWnd) {
	error("TODO: CDC::ExcludeUpdateRgn");
}

int CDC::IntersectClipRect(int x1, int y1, int x2, int y2) {
	error("TODO: CDC::IntersectClipRect");
}

int CDC::IntersectClipRect(LPCRECT lpRect) {
	error("TODO: CDC::IntersectClipRect");
}

int CDC::OffsetClipRgn(int x, int y) {
	error("TODO: CDC::OffsetClipRgn");
}

int CDC::OffsetClipRgn(SIZE size) {
	error("TODO: CDC::OffsetClipRgn");
}

int CDC::SelectClipRgn(CRgn *pRgn, int nMode) {
	error("TODO: CDC::SelectClipRgn");
}

int CDC::SetROP2(int nDrawMode) {
	error("TODO: CDC::SetROP2");
}

BOOL CDC::DPtoLP(LPPOINT lpPoints, int nCount) {
	error("TODO: CDC::DPtoLP");
}

BOOL CDC::DPtoLP(RECT *lpRect) {
	error("TODO: CDC::DPtoLP");
}

BOOL CDC::LPtoDP(RECT *lpRect) {
	error("TODO: CDC::LPtoDP");
}

BOOL CDC::BitBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
                 int xSrc, int ySrc, DWORD dwRop) {
	error("TODO: CDC::BitBlt");
}

BOOL CDC::StretchBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
                     int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, DWORD dwRop) {
	error("TODO: CDC::StretchBlt");
}

void CDC::Ellipse(LPCRECT lpRect) {
	error("TODO: CDC::Ellipse");
}

void CDC::Ellipse(int x1, int y1, int x2, int y2) {
	error("TODO: CDC::Ellipse");
}

void CDC::FrameRect(LPCRECT lpRect, CBrush *pBrush) {
	error("TODO: CDC::FrameRect");
}

void CDC::FillRect(LPCRECT lpRect, CBrush *pBrush) {
	auto *surf = surface();
	CBrush::Impl *brush = static_cast<CBrush::Impl *>(
	                          pBrush->m_hObject);
	assert(brush->_type == HS_HORIZONTAL ||
	       brush->_type == HS_VERTICAL);

	surf->fillRect(*lpRect, brush->getColor());
}

BOOL CDC::FloodFill(int x, int y, COLORREF crColor) {
	error("TODO: CDC::FloodFill");
}

BOOL CDC::FloodFill(int x, int y, COLORREF crColor,
                    UINT nFillType) {
	error("TODO: CDC::FloodFill");
}

void CDC::Rectangle(LPCRECT lpRect) {
	error("TODO: CDC::Rectangle");
}

void CDC::Rectangle(int x1, int y1, int x2, int y2) {
	error("TODO: CDC::Rectangle");
}

BOOL CDC::Pie(int x1, int y1, int x2, int y2,
              int x3, int y3, int x4, int y4) {
	error("TODO: CDC::Pie");
}

BOOL CDC::Pie(LPCRECT lpRect, const POINT &ptStart, const POINT &ptEnd) {
	error("TODO: CDC::Pie");
}

BOOL CDC::FrameRgn(CRgn *pRgn, CBrush *pBrush,
                   int nWidth, int nHeight) {
	error("TODO: CDC::FrameRgn");
}

void CDC::MoveTo(int x, int y) {
	error("TODO: CDC::MoveTo");
}

void CDC::LineTo(int x, int y) {
	error("TODO: CDC::LineTo");
}

COLORREF CDC::GetPixel(int x, int y) const {
	error("TODO: CDC::GetPixel");
}

COLORREF CDC::GetPixel(const POINT &point) const {
	error("TODO: CDC::GetPixel");
}

CGdiObject *CDC::SelectStockObject(int nIndex) {
	error("TODO: CDC::SelectStockObject");
}

CPen *CDC::SelectObject(CPen *pPen) {
	error("TODO: CDC::SelectObject");
}

CBrush *CDC::SelectObject(CBrush *pBrush) {
	error("TODO: CDC::SelectObject");
}

CFont *CDC::SelectObject(CFont *pFont) {
	error("TODO: CDC::SelectObject");
}

CBitmap *CDC::SelectObject(CBitmap *pBitmap) {
	// Attach new bitmap
	HBITMAP oldBitmap = surface()->Attach((HBITMAP)pBitmap->m_hObject);

	if (pBitmap->_isTemporary) {
		delete pBitmap;
		return nullptr;
	}

	if (!oldBitmap)
		return nullptr;

	// Create a new temporary CBitmap to
	// encapsulate the returned HBITMAP
	CBitmap *result = new CBitmap();
	result->Attach(oldBitmap);
	result->_isTemporary = true;

	return result;
}

int CDC::SelectObject(CRgn *pRgn) {
	error("TODO: CDC::SelectObject");
}

CGdiObject *CDC::SelectObject(CGdiObject *pObject) {
	error("TODO: CDC::SelectObject");
}

COLORREF CDC::GetNearestColor(COLORREF crColor) const {
	return surface()->GetNearestColor(crColor);
}

CPalette *CDC::SelectPalette(CPalette *pPalette, BOOL bForceBackground) {
	return surface()->selectPalette(pPalette);
}

UINT CDC::RealizePalette() {
	return surface()->realizePalette();
}

void CDC::UpdateColors() {
	error("TODO: CDC::UpdateColors");
}

COLORREF CDC::SetBkColor(COLORREF crColor) {
	error("TODO: CDC::SetBkColor");
}

int CDC::SetBkMode(int nBkMode) {
	error("TODO: CDC::SetBkMode");
}

COLORREF CDC::SetTextColor(COLORREF crColor) {
	error("TODO: CDC::SetTextColor");
}

BOOL CDC::TextOut(int x, int y, LPCSTR lpszString, int nCount) {
	error("TODO: CDC::TextOut");
}

BOOL CDC::TextOut(int x, int y, const CString &str) {
	error("TODO: CDC::TextOut");
}

BOOL CDC::ExtTextOut(int x, int y, UINT nOptions, LPCRECT lpRect,
                     LPCSTR lpszString, UINT nCount, LPINT lpDxWidths) {
	error("TODO: CDC::ExtTextOut");
}

BOOL CDC::ExtTextOut(int x, int y, UINT nOptions, LPCRECT lpRect,
                     const CString &str, LPINT lpDxWidths) {
	error("TODO: CDC::ExtTextOut");
}

CSize CDC::TabbedTextOut(int x, int y, LPCSTR lpszString, int nCount,
                         int nTabPositions, LPINT lpnTabStopPositions, int nTabOrigin) {
	error("TODO: CDC::TabbedTextOut");
}

CSize CDC::TabbedTextOut(int x, int y, const CString &str,
                         int nTabPositions, LPINT lpnTabStopPositions, int nTabOrigin) {
	error("TODO: CDC::TabbedTextOut");
}

int CDC::DrawText(LPCSTR lpszString, int nCount,
                  LPRECT lpRect, UINT nFormat) {
	error("TODO: CDC::DrawText");
}

int CDC::DrawText(const CString &str, LPRECT lpRect, UINT nFormat) {
	error("TODO: CDC::DrawText");
}

CSize CDC::GetTextExtent(LPCSTR lpszString, int nCount) const {
	error("TODO: CDC::GetTextExtent");
}

CSize CDC::GetTextExtent(const CString &str) const {
	error("TODO: CDC::GetTextExtent");
}

CSize CDC::GetOutputTextExtent(LPCSTR lpszString, int nCount) const {
	error("TODO: CDC::GetOutputTextExtent");
}

CSize CDC::GetOutputTextExtent(const CString &str) const {
	error("TODO: CDC::GetOutputTextExtent");
}

CSize CDC::GetTabbedTextExtent(LPCSTR lpszString, int nCount,
                               int nTabPositions, LPINT lpnTabStopPositions) const {
	error("TODO: CDC::GetTabbedTextExtent");
}

CSize CDC::GetTabbedTextExtent(const CString &str,
                               int nTabPositions, LPINT lpnTabStopPositions) const {
	error("TODO: CDC::GetTabbedTextExtent");
}

CSize CDC::GetOutputTabbedTextExtent(LPCSTR lpszString, int nCount,
                                     int nTabPositions, LPINT lpnTabStopPositions) const {
	error("TODO: CDC::GetOutputTabbedTextExtent");
}

CSize CDC::GetOutputTabbedTextExtent(const CString &str,
                                     int nTabPositions, LPINT lpnTabStopPositions) const {
	error("TODO: CDC::GetOutputTabbedTextExtent");
}

BOOL CDC::GrayString(CBrush *pBrush,
                     BOOL(CALLBACK *lpfnOutput)(HDC, LPARAM, int), LPARAM lpData,
                     int nCount, int x, int y, int nWidth, int nHeight) {
	error("TODO: CDC::GrayString");
}

UINT CDC::GetTextAlign() const {
	error("TODO: CDC::GetTextAlign");
}

UINT CDC::SetTextAlign(UINT nFlags) {
	error("TODO: CDC::SetTextAlign");
}

BOOL CDC::GetTextMetrics(LPTEXTMETRIC lpMetrics) const {
	error("TODO: CDC::GetTextMetrics");
}

} // namespace MFC
} // namespace Bagel
