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

#ifndef BAGEL_BOFLIB_GFX_BOF_TEXT_H
#define BAGEL_BOFLIB_GFX_BOF_TEXT_H

#include "bagel/boflib/boffo.h"

#include "bagel/boflib/bof_object.h"
#include "bagel/boflib/gfx/bof_bitmap.h"
#include "bagel/boflib/bof_error.h"
#include "bagel/boflib/bof_string.h"

namespace Bagel {

#define NUM_POINT_SIZES 18

// text color and offset definitions
//
// jwl 08.19.96 rgb's are the same for black and white...

//#if BOF_MAC
//#define CTEXT_COLOR         RGB(255,255,255)
//#define CTEXT_SHADOW_COLOR  RGB(255,255,255)
//#else
#define CTEXT_COLOR         RGB(0,0,0)
#define CTEXT_SHADOW_COLOR  RGB(0,0,0)
//#endif
#define CTEXT_SHADOW_DX     2
#define CTEXT_SHADOW_DY     2

// text justification definitions
//
// jwl 07.15.96 use mac constants
#if BOF_MAC
#define JUSTIFY_CENTER      1
#define JUSTIFY_LEFT        0
#define JUSTIFY_RIGHT       -1
#define JUSTIFY_WRAP        0       // no concept on mac
#else
#define JUSTIFY_CENTER      0
#define JUSTIFY_LEFT        1
#define JUSTIFY_RIGHT       2
#define JUSTIFY_WRAP        3

#endif

// text weight definitions
//
#if 1 // BOF_WINDOWS

enum {
    DT_TOP      = (1 << 0),
    DT_BOTTOM   = (1 << 1),
    DT_LEFT     = (1 << 2),
    DT_RIGHT    = (1 << 3),
    DT_CENTER   = (1 << 4),
    DT_WORDBREAK = (1 << 5),

};

#define FONT_DEFAULT        0
#define FONT_MONO           1

#define TEXT_DONTCARE       0
#define TEXT_THIN           FW_THIN
#define TEXT_EXTRALIGHT     FW_EXTRALIGHT
#define TEXT_ULTRALIGHT     FW_ULTRALIGHT
#define TEXT_LIGHT          FW_LIGHT
#define TEXT_NORMAL         FW_NORMAL
#define TEXT_REGULAR        FW_REGULAR
#define TEXT_MEDIUM         FW_MEDIUM
#define TEXT_SEMIBOLD       FW_SEMIBOLD
#define TEXT_DEMIBOLD       FW_DEMIBOLD
#define TEXT_BOLD           FW_BOLD
#define TEXT_EXTRABOLD      FW_EXTRABOLD
#define TEXT_ULTRABOLD      FW_ULTRABOLD
#define TEXT_BLACK          FW_BLACK
#define TEXT_HEAVY          FW_HEAVY

#define FORMAT_TOP_LEFT         ( DT_TOP | DT_LEFT )
#define FORMAT_TOP_RIGHT        ( DT_TOP | DT_RIGHT )
#define FORMAT_TOP_CENTER       ( DT_TOP | DT_CENTER )
#define FORMAT_BOT_LEFT         ( DT_BOTTOM | DT_LEFT )
#define FORMAT_BOT_RIGHT        ( DT_BOTTOM | DT_RIGHT )
#define FORMAT_BOT_CENTER       ( DT_BOTTOM | DT_CENTER )
#define FORMAT_CENTER_LEFT      ( DT_VCENTER | DT_LEFT )
#define FORMAT_CENTER_RIGHT     ( DT_VCENTER | DT_RIGHT )
#define FORMAT_CENTER_CENTER    ( DT_VCENTER | DT_CENTER )
#define FORMAT_SINGLE_LINE      DT_SINGLELINE
#define FORMAT_MULTI_LINE       DT_WORDBREAK
#define FORMAT_DEFAULT          ( FORMAT_TOP_LEFT | FORMAT_MULTI_LINE )

#elif BOF_MAC
#define FONT_DEFAULT        applFont
#define FONT_MONO           monaco

#define TEXT_DONTCARE       0
#define TEXT_THIN           0
#define TEXT_EXTRALIGHT     0
#define TEXT_ULTRALIGHT     0
#define TEXT_LIGHT          0
#define TEXT_NORMAL         0
#define TEXT_REGULAR        0
#define TEXT_MEDIUM         0
#define TEXT_SEMIBOLD       0
#define TEXT_DEMIBOLD       0
#define TEXT_BOLD           bold
#define TEXT_EXTRABOLD      (bold | outline)
#define TEXT_ULTRABOLD      (bold | shadow)
#define TEXT_BLACK          0
#define TEXT_HEAVY          0

#define FORMAT_TOP_LEFT         teFlushLeft
#define FORMAT_TOP_RIGHT        teFlushRight
#define FORMAT_TOP_CENTER       teCenter
#define FORMAT_BOT_LEFT         0
#define FORMAT_BOT_RIGHT        0
#define FORMAT_BOT_CENTER       0
#define FORMAT_CENTER_LEFT      0
#define FORMAT_CENTER_RIGHT     0
#define FORMAT_CENTER_CENTER    0
#define FORMAT_SINGLE_LINE      0
#define FORMAT_MULTI_LINE       0
#define FORMAT_DEFAULT          ( FORMAT_TOP_LEFT | FORMAT_MULTI_LINE )
#endif

#if BOF_MAC || BOF_WINMAC
#define FONT_DEFAULT_SIZE       10
#define FONT_8POINT             8
#define FONT_10POINT            10
#define FONT_12POINT            10
#define FONT_14POINT            12
#define FONT_15POINT            12
#define FONT_18POINT            16
#define FONT_20POINT            18
#define TEXT_DEFAULT_FACE       TEXT_REGULAR
#else
#define FONT_DEFAULT_SIZE       -14
#define FONT_8POINT             8
#define FONT_10POINT            10
#define FONT_12POINT            12
#define FONT_14POINT            14
#define FONT_15POINT            15
#define FONT_18POINT            18
#define FONT_20POINT            20
#define TEXT_DEFAULT_FACE       TEXT_BOLD
#endif

// jwl 12.24.96 a utility routine for calculating a text rectangle.
CBofRect CalculateTextRect(CBofWindow *pWnd, CBofString *pStr, INT nSize, INT nFont);

class CBofText: public CBofObject, public CBofError {

public:

	// Constructors
	//
	CBofText();
	CBofText(CBofRect *pRect, INT nJustify = JUSTIFY_CENTER, UINT nFormatFlags = FORMAT_DEFAULT);

	virtual ~CBofText();

	// Implementation
	//
	ERROR_CODE  SetupText(CBofRect *pRect, INT nJustify = JUSTIFY_CENTER, UINT nFormatFlags = FORMAT_DEFAULT);
	ERROR_CODE  SetupTextOpt(CBofRect *pRect, INT nJustify = JUSTIFY_CENTER, UINT nFormatFlags = FORMAT_DEFAULT);

	VOID        SetText(const CBofString &cString)  {
		m_cCurString = cString;
	}
	VOID        SetColor(const RGBCOLOR cColor) {
		m_cTextColor = cColor;
	}
	VOID        SetSize(const INT nSize)        {
		m_nCurSize = nSize;
	}
	VOID        SetWeight(const INT nWeight)    {
		m_nCurWeight = nWeight;
	}

	VOID        SetShadowColor(const RGBCOLOR cColor) {
		m_cShadowColor = cColor;
	}
	VOID        SetShadowSize(INT nDX, INT nDY) {
		m_nShadow_DX = nDX;
		m_nShadow_DY = nDY;
	}

	CBofString  GetText(VOID)                   {
		return (m_cCurString);
	}
	RGBCOLOR    GetColor(VOID)                  {
		return (m_cTextColor);
	}
	INT         GetSize(VOID)                   {
		return (m_nCurSize);
	}
	INT         GetWeight(VOID)                 {
		return (m_nCurWeight);
	}

	ERROR_CODE  Erase(CBofWindow *pWnd);
	ERROR_CODE  Erase(CBofBitmap *pBmp);

	ERROR_CODE  Display(CBofWindow *pWnd);
	ERROR_CODE  Display(CBofBitmap *pBmp);

	ERROR_CODE  Display(CBofWindow *pWnd, const CHAR *pszText, const INT nSize, const INT nWeight, const RGBCOLOR cColor = CTEXT_COLOR, INT nFont = FONT_DEFAULT);
	ERROR_CODE  Display(CBofBitmap *pBmp, const CHAR *pszText, const INT nSize, const INT nWeight, const RGBCOLOR cColor = CTEXT_COLOR, INT nFont = FONT_DEFAULT);

	ERROR_CODE  DisplayShadowed(CBofWindow *, const CHAR *, const INT, const INT, const RGBCOLOR cColor, const RGBCOLOR cShadow = CTEXT_SHADOW_COLOR, const INT nDX = CTEXT_SHADOW_DX, const INT nDY = CTEXT_SHADOW_DY, INT n = FONT_DEFAULT);
	ERROR_CODE  DisplayShadowed(CBofBitmap *, const CHAR *, const INT, const INT, const RGBCOLOR cColor, const RGBCOLOR cShadow = CTEXT_SHADOW_COLOR, const INT nDX = CTEXT_SHADOW_DX, const INT nDY = CTEXT_SHADOW_DY, INT n = FONT_DEFAULT);

	VOID        FlushBackground(VOID)       {
		m_bSaved = FALSE;
	}

	BOOL        WillTextFit(CBofWindow *pWnd, const CHAR *pszText);

	static ERROR_CODE Initialize(VOID);
	static ERROR_CODE ShutDown(VOID);

	ERROR_CODE  DisplayTextEx(CBofBitmap *pBmp, const CHAR *pszText, CBofRect *pRect, const INT nSize, const INT nWeight, const BOOL bShadowed, INT nFont = FONT_DEFAULT);
#if BOF_WINDOWS
	static HFONT GetMonoFont(INT nSize, INT nWeight);
#endif

private:
	VOID        InitializeFields(VOID);
	ERROR_CODE  DisplayText(CBofWindow *pWnd, const CHAR *pszText, CBofRect *pRect, const INT nSize, const INT nWeight, const BOOL bShadowed, INT nFont = FONT_DEFAULT);
	ERROR_CODE  DisplayText(CBofBitmap *pBmp, const CHAR *pszText, CBofRect *pRect, const INT nSize, const INT nWeight, const BOOL bShadowed, INT nFont = FONT_DEFAULT);

protected:

	CBofString  m_cCurString;       // text to be displayed
	CBofRect    m_cRect;            // bounding rectangle of text area
	CBofPoint   m_cPosition;        // upper left corner of text displayed
	CBofSize    m_cSize;            // dx/dy size of the text bitmap
	RGBCOLOR    m_cTextColor;       // color to use for the text itself
	RGBCOLOR    m_cShadowColor;     // color to use for the text's shadow
	CBofBitmap *m_pBackground;      // bitmap for the text's background
	CBofBitmap *m_pWork;            // bitmap for the work area

	INT         m_nCurSize;         // point size of current text
	INT         m_nCurWeight;       // style of current text

	INT         m_nJustify;         // positioning within the rectangle
	INT         m_nShadow_DX;       // horizontal offset for shadow
	INT         m_nShadow_DY;       // vertical offset for shadow
	UINT        m_nFormatFlags;     // multi line formatting flags
	BOOL        m_bMultiLine;       // mutli vs single line formatting
	BOOL        m_bSaved;

#if BOF_WINDOWS
	static HFONT m_hDefaultFont[NUM_POINT_SIZES];
	static HFONT m_hFixedFont[NUM_POINT_SIZES];
	static INT   m_nMonoType;
#endif

	static BOOL  m_bInitialized;

	static INT  m_nTabStop;     // tabstop table
};


// Global text functions
//
ERROR_CODE PaintText(CBofWindow *pWnd, CBofRect *pRect, const CHAR *, const INT nSize, const INT nWeight, const RGBCOLOR cColor = CTEXT_COLOR, INT nJustify = JUSTIFY_CENTER, UINT nFormat = FORMAT_DEFAULT, INT nFont = FONT_DEFAULT);
ERROR_CODE PaintText(CBofBitmap *pBmp, CBofRect *pRect, const CHAR *, const INT nSize, const INT nWeight, const RGBCOLOR cColor = CTEXT_COLOR, INT nJustify = JUSTIFY_CENTER, UINT nFormat = FORMAT_DEFAULT, INT nFont = FONT_DEFAULT);

ERROR_CODE PaintShadowedText(CBofWindow *, CBofRect *pRect, const CHAR *, const INT nSize, const INT nWeight, const RGBCOLOR cColor = CTEXT_COLOR, INT nJustify = JUSTIFY_CENTER, UINT n = FORMAT_DEFAULT, INT nFont = FONT_DEFAULT);
ERROR_CODE PaintShadowedText(CBofBitmap *, CBofRect *pRect, const CHAR *, const INT nSize, const INT nWeight, const RGBCOLOR cColor = CTEXT_COLOR, INT nJustify = JUSTIFY_CENTER, UINT n = FORMAT_DEFAULT, INT nFont = FONT_DEFAULT);

} // namespace Bagel

#endif
