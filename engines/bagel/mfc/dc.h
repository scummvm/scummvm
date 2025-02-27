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

#ifndef BAGEL_MFC_DC_H
#define BAGEL_MFC_DC_H

#include "bagel/mfc/mfc_types.h"
#include "bagel/mfc/bitmap.h"
#include "bagel/mfc/font.h"
#include "bagel/mfc/palette.h"
#include "bagel/mfc/rect.h"
#include "bagel/mfc/str.h"

namespace Bagel {
namespace MFC {

enum DeviceCaps {
	HORZRES, VERTRES
};
enum {
	SRCCOPY	  = 0x00CC0020,
	SRCPAINT  = 0x00EE0086,
	SRCAND    = 0x008800C6,
	SRCINVERT = 0x00660046,
	SRCERASE  = 0x00440328,
	NOTSRCCOPY  = 0x00330008,
	NOTSRCERASE = 0x001100A6
};
enum {
	PS_INSIDEFRAME,
	PS_SOLID
};
enum {
	OPAQUE,
	TRANSPARENT
};
enum {
	WHITE_BRUSH         =  0,
	LTGRAY_BRUSH        =  1,
	GRAY_BRUSH          =  2,
	DKGRAY_BRUSH        =  3,
	BLACK_BRUSH         =  4,
	NULL_BRUSH          =  5,
	HOLLOW_BRUSH        =  NULL_BRUSH,
	WHITE_PEN           =  6,
	BLACK_PEN           =  7,
	NULL_PEN            =  8,
	OEM_FIXED_FONT      = 10,
	ANSI_FIXED_FONT     = 11,
	ANSI_VAR_FONT       = 12,
	SYSTEM_FONT         = 13,
	DEVICE_DEFAULT_FONT = 14,
	DEFAULT_PALETTE     = 15,
	SYSTEM_FIXED_FONT   = 16
};
enum {
	ODA_DRAWENTIRE = 0x0001,
	ODA_SELECT     = 0x0002,
	ODA_FOCUS      = 0x0004
};

/* Ternary raster operations */
#define SRCCOPY             (DWORD)0x00CC0020 /* dest = source                   */
#define SRCPAINT            (DWORD)0x00EE0086 /* dest = source OR dest           */
#define SRCAND              (DWORD)0x008800C6 /* dest = source AND dest          */
#define SRCINVERT           (DWORD)0x00660046 /* dest = source XOR dest          */
#define SRCERASE            (DWORD)0x00440328 /* dest = source AND (NOT dest )   */
#define NOTSRCCOPY          (DWORD)0x00330008 /* dest = (NOT source)             */
#define NOTSRCERASE         (DWORD)0x001100A6 /* dest = (NOT src) AND (NOT dest) */
#define MERGECOPY           (DWORD)0x00C000CA /* dest = (source AND pattern)     */
#define MERGEPAINT          (DWORD)0x00BB0226 /* dest = (NOT source) OR dest     */
#define PATCOPY             (DWORD)0x00F00021 /* dest = pattern                  */
#define PATPAINT            (DWORD)0x00FB0A09 /* dest = DPSnoo                   */
#define PATINVERT           (DWORD)0x005A0049 /* dest = pattern XOR dest         */
#define DSTINVERT           (DWORD)0x00550009 /* dest = (NOT dest)               */
#define BLACKNESS           (DWORD)0x00000042 /* dest = BLACK                    */
#define WHITENESS           (DWORD)0x00FF0062 /* dest = WHITE                    */
#define NOMIRRORBITMAP      (DWORD)0x80000000 /* Do not Mirror the bitmap in this call */
#define CAPTUREBLT          (DWORD)0x40000000 /* Include layered windows */

class CWnd;
typedef CWnd *HWND;

class CPen {
public:
	BOOL CreatePen(int nPenStyle, int nWidth, COLORREF crColor);
};

class CBrush {
public:
	void CreateSolidBrush(COLORREF color);
	void CreateStockObject(int brush);
};

class CDC;
typedef CDC *HDC;

struct DRAWITEMSTRUCT {
	UINT   CtlType;    // Control type (button, list box, etc.)
	UINT   CtlID;      // Control ID
	UINT   itemID;     // Item ID (for list boxes, combo boxes)
	UINT   itemAction; // Action to perform (e.g., redraw)
	UINT   itemState;  // State of the item (selected, focused, etc.)
	HWND   hwndItem;   // Handle to the control
	HDC    hDC;        // Handle to the device context for drawing
	RECT   rcItem;     // Bounding rectangle for the item
	uint32 *itemData;  // Application-defined data
};
typedef DRAWITEMSTRUCT *LPDRAWITEMSTRUCT;

struct PAINTSTRUCT {
};

class CDC {
private:
	CPalette _palette;
	CPen *_pen = nullptr;
	CBrush *_brush = nullptr;

public:
	HDC m_hDC;

public:
	CDC() : m_hDC(this), _palette(PALETTE_COUNT) {}
	~CDC();

	int GetDeviceCaps(int field) const;
	CPalette *SelectPalette(CPalette *pPalette, bool bForceBackground);
	CBitmap *SelectObject(CBitmap *bitmap);
	CFont *SelectObject(CFont *font);
	void RealizePalette();

	void Attach(HDC dc);
	void Detach();
	void DeleteDC();

	CBitmap *CreateCompatibleBitmap(CDC *pDC, int nWidth, int nHeight);
	BOOL CreateCompatibleDC(CDC *pDC);

	CPen *SelectObject(CPen *pen);
	CBrush *SelectObject(CBrush *brush);

	/**
	 * Performs a bit-block transfer of bitmaps
	 * @param x			Destination x
	 * @param y			Destination y
	 * @param nWidth	Width of area
	 * @param nHeight	Height of area
	 * @param pSrcDC	Source bitmap
	 * @param xSrc		Source x co-ordinate
	 * @param ySrc		Source y co-ordinate
	 * @param dwRop		Raster operation code
	 * @return		Returns true if successful
	 */
	bool BitBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
		int xSrc, int ySrc, uint32 dwRop);

	void Rectangle(int x1, int y1, int x2, int y2);
	void Rectangle(const LPRECT rect);
	void MoveTo(int x, int y);
	void LineTo(int x, int y);
	int FrameRect(LPCRECT lpRect, CBrush *pBrush);
	void FillRect(LPCRECT lpRect, CBrush *pBrush);
	void Ellipse(LPCRECT lpRect);
	int SetBkMode(int nBkMode);

	BOOL GetTextMetrics(LPTEXTMETRIC lptm);
	CSize GetTextExtent(const char *lpszString) const;
	CSize GetTextExtent(const char *lpszString, int nCount) const;
	CSize GetTextExtent(const CString &str) const;
	CSize GetTextExtent(const wchar_t *lpszString, int nCount) const;
	int SetTextColor(int nTextColor);
	BOOL TextOut(int x, int y, const char *lpszString);
	BOOL TextOut(int x, int y, const char *lpszString, int nCount);

	int TabbedTextOut(int x, int y, LPCSTR lpszString,
		int nCount, int nTabPositions,
		const INT *lpnTabStopPositions, int nTabOrigin);
};

class CPaintDC : public CDC {
public:
	explicit CPaintDC(CWnd *pWnd) : m_hWnd(pWnd) {
	}

	// Attributes
protected:
	HWND m_hWnd;
public:
	PAINTSTRUCT m_ps;       // actual paint struct!

public:
	virtual ~CPaintDC() {}
};

} // namespace MFC
} // namespace Bagel

#endif
