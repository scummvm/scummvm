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

#ifndef BAGEL_BOFLIB_GFX_TEXT_H
#define BAGEL_BOFLIB_GFX_TEXT_H

#include "graphics/font.h"
#include "bagel/boflib/boffo.h"
#include "bagel/boflib/object.h"
#include "bagel/boflib/gfx/bitmap.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/string.h"

namespace Bagel {

#define NUM_POINT_SIZES 32

// Text color and offset definitions
#define CTEXT_COLOR         RGB(0,0,0)
#define CTEXT_SHADOW_COLOR  RGB(0,0,0)

#define CTEXT_SHADOW_DX     2
#define CTEXT_SHADOW_DY     2

// Text justification definitions
#define JUSTIFY_CENTER      0
#define JUSTIFY_LEFT        1
#define JUSTIFY_RIGHT       2
#define JUSTIFY_WRAP        3

// Text weight definitions
//

/*
 * DrawText Format Flags
 */
#define DT_TOP 0x00000000
#define DT_LEFT 0x00000000
#define DT_CENTER 0x00000001
#define DT_RIGHT 0x00000002
#define DT_VCENTER 0x00000004
#define DT_BOTTOM 0x00000008
#define DT_WORDBREAK 0x00000010
#define DT_SINGLELINE 0x00000020
#define DT_EXPANDTABS 0x00000040
#define DT_TABSTOP 0x00000080
#define DT_NOCLIP 0x00000100
#define DT_EXTERNALLEADING 0x00000200
#define DT_CALCRECT 0x00000400
#define DT_NOPREFIX 0x00000800
#define DT_INTERNAL 0x00001000


enum {
	FW_NORMAL,
	FW_BOLD,
	FW_MEDIUM,
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

#define FONT_DEFAULT_SIZE       -14
#define FONT_8POINT             8
#define FONT_10POINT            10
#define FONT_12POINT            12
#define FONT_14POINT            14
#define FONT_15POINT            15
#define FONT_18POINT            18
#define FONT_20POINT            20
#define TEXT_DEFAULT_FACE       TEXT_BOLD


class CBofText: public CBofObject, public CBofError {
public:
	// Constructors
	CBofText();
	CBofText(const CBofRect *pRect, INT nJustify = JUSTIFY_CENTER, uint32 nFormatFlags = FORMAT_DEFAULT);
	virtual ~CBofText();

	// Implementation
	//

	/**
	 * Build primary data objects and work areas; text will be displayed
	 * centered within the defined rectangular area, hence it is up to
	 * the caller to ensure that the text fits (excess is cropped).
	 * @param pRect			Rectangular area encompassed by the text object
	 * @param nJustify		Alignment of text in the rectangle
	 */
	ERROR_CODE SetupText(const CBofRect *pRect, INT nJustify = JUSTIFY_CENTER, uint32 nFormatFlags = FORMAT_DEFAULT);
	ERROR_CODE SetupTextOpt(const CBofRect *pRect, INT nJustify = JUSTIFY_CENTER, uint32 nFormatFlags = FORMAT_DEFAULT);

	void SetText(const CBofString &cString) {
		m_cCurString = cString;
	}
	void SetColor(const RGBCOLOR cColor) {
		m_cTextColor = cColor;
	}
	void SetSize(const INT nSize) {
		m_nCurSize = nSize;
	}
	void SetWeight(const INT nWeight) {
		m_nCurWeight = nWeight;
	}

	void SetShadowColor(const RGBCOLOR cColor) {
		m_cShadowColor = cColor;
	}
	void SetShadowSize(INT nDX, INT nDY) {
		m_nShadow_DX = nDX;
		m_nShadow_DY = nDY;
	}

	CBofString GetText() const {
		return m_cCurString;
	}
	RGBCOLOR GetColor() const {
		return m_cTextColor;
	}
	INT GetSize() const {
		return m_nCurSize;
	}
	INT GetWeight() const {
		return m_nCurWeight;
	}

	/**
	 * Restores the background behind current text on screen
	 * @param pWnd		Window to erase text from
	 * @return			Error return Code
	 */
	ERROR_CODE Erase(CBofWindow *pWnd);

	/**
	 * Restores the background behind current text offscreen
	 * @param pBmp		Offscreen bitmap to erase text from
	 * @return			Error return Code
	 */
	ERROR_CODE Erase(CBofBitmap *pBmp);

	/**
	 * Re-displays current text, formatted with current attribs
	 * @param pWnd		Window to paint into
	 * @return			Error return Code
	 */
	ERROR_CODE Display(CBofWindow *pWnd);

	/**
	 * Re-displays current text, formatted with current attribs
	 * @param pBmp		Bitmap to paint into
	 * @return			Error return Code
	 */
	ERROR_CODE Display(CBofBitmap *pBmp);

	/**
	 * Display a text string, formatted in the current text area
	 * @param pWnd			Window to paint into
	 * @param pszText		Point to text string to be displayed
	 * @param nSize			Point size of the text to be used
	 * @param nWeight		Weighting of the font (FW_ identifier)
	 * @param cColor		Color that the text will be
	 * @return				Error return Code
	 */
	ERROR_CODE Display(CBofWindow *pWnd, const CHAR *pszText, const INT nSize, const INT nWeight, const RGBCOLOR cColor = CTEXT_COLOR, INT nFont = FONT_DEFAULT);

	/**
	 * Display a text string, formatted in the current text area
	 * @param pBmp			Bitmap to paint into
	 * @param pszText		Point to text string to be displayed
	 * @param nSize			Point size of the text to be used
	 * @param nWeight		Weighting of the font (FW_ identifier)
	 * @param cColor		Color that the text will be
	 * @return				Error return Code
	 */
	ERROR_CODE Display(CBofBitmap *pBmp, const CHAR *pszText, const INT nSize, const INT nWeight, const RGBCOLOR cColor = CTEXT_COLOR, INT nFont = FONT_DEFAULT);

	/**
	 * Display a shadowed text string into the current text area
	 * @param pWnd			Window to paint into
	 * @param pszText		Point to text string to be displayed
	 * @param nSize			Point size of the text to be used
	 * @param nWeight		Weighting of the font (FW_ identifier)
	 * @param cColor		Color that the text will be
	 * @param cShadow		Color that the text's shadow will be
	 * @return				Error return Code
	 */
	ERROR_CODE DisplayShadowed(CBofWindow *pWnd, const CHAR *pszText, INT nSize,
		INT nWeight, RGBCOLOR cColor, RGBCOLOR cShadow = CTEXT_SHADOW_COLOR,
		INT nDX = CTEXT_SHADOW_DX, INT nDY = CTEXT_SHADOW_DY, INT n = FONT_DEFAULT);

	/**
	 * Display a shadowed text string into the current text area
	 * @param pBmp			Bitmap to paint into
	 * @param pszText		Point to text string to be displayed
	 * @param nSize			Point size of the text to be used
	 * @param nWeight		Weighting of the font (FW_ identifier)
	 * @param cColor		Color that the text will be
	 * @param cShadow		Color that the text's shadow will be
	 * @return				Error return Code
	 */
	ERROR_CODE DisplayShadowed(CBofBitmap *, const CHAR *, const INT, const INT, const RGBCOLOR cColor, const RGBCOLOR cShadow = CTEXT_SHADOW_COLOR, const INT nDX = CTEXT_SHADOW_DX, const INT nDY = CTEXT_SHADOW_DY, INT n = FONT_DEFAULT);

	void FlushBackground()       {
		m_bSaved = FALSE;
	}

	static ERROR_CODE initialize();
	static ERROR_CODE shutdown();

	/**
	 * Displays specified text onto specified bitmap
	 * @param pBmp			Bitmap to paint text onto
	 * @param pszText		Pointer to text string to be displayed
	 * @param pRect			Area to paint text to
	 * @param nSize			Point size of the text to be used
	 * @param nWeight		Weighting of the font (FW_ identifier)
	 * @param bShadowed		Whether the text is shadowed
	 * @return				Error return Code
	 */
	ERROR_CODE DisplayTextEx(CBofBitmap *pBmp, const CHAR *pszText, CBofRect *pRect, const INT nSize, const INT nWeight, const BOOL bShadowed, INT nFont = FONT_DEFAULT);

	static Graphics::Font *getFont(INT nFont, INT nSize, INT nWeight);

private:
	/**
	 * Initializes key fields to zero or nullptr states.
	 */
	void InitializeFields();

	/**
	 * Displays specified text onto specified bitmap
	 * @param pWnd			Window to paint text onto
	 * @param pszText		Pointer to text string to be displayed
	 * @param pRect			Area to paint text to
	 * @param nSize			Point size of the text to be used
	 * @param nWeight		Weighting of the font (FW_ identifier)
	 * @param bShadowed		Whether the text is shadowed
	 * @return				Error return Code
	 */
	ERROR_CODE DisplayText(CBofWindow *pWnd, const CHAR *pszText, CBofRect *pRect, const INT nSize, const INT nWeight, const BOOL bShadowed, INT nFont = FONT_DEFAULT);
	ERROR_CODE DisplayText(CBofBitmap *pBmp, const CHAR *pszText, CBofRect *pRect, const INT nSize, const INT nWeight, const BOOL bShadowed, INT nFont = FONT_DEFAULT);

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
	uint32        m_nFormatFlags;     // multi line formatting flags
	BOOL        m_bMultiLine;       // mutli vs single line formatting
	BOOL        m_bSaved;

	static Graphics::Font *_defaultFonts[NUM_POINT_SIZES];
	static Graphics::Font *_fixedFonts[NUM_POINT_SIZES];

	static BOOL  _initialized;

	static INT   _tabStop;     // tabstop table
};


// Global text functions
//
ERROR_CODE PaintText(CBofWindow *pWnd, CBofRect *pRect, const CHAR *, const INT nSize, const INT nWeight, const RGBCOLOR cColor = CTEXT_COLOR, INT nJustify = JUSTIFY_CENTER, uint32 nFormat = FORMAT_DEFAULT, INT nFont = FONT_DEFAULT);
ERROR_CODE PaintText(CBofBitmap *pBmp, CBofRect *pRect, const CHAR *, const INT nSize, const INT nWeight, const RGBCOLOR cColor = CTEXT_COLOR, INT nJustify = JUSTIFY_CENTER, uint32 nFormat = FORMAT_DEFAULT, INT nFont = FONT_DEFAULT);

ERROR_CODE PaintShadowedText(CBofWindow *, CBofRect *pRect, const CHAR *, const INT nSize, const INT nWeight, const RGBCOLOR cColor = CTEXT_COLOR, INT nJustify = JUSTIFY_CENTER, uint32 n = FORMAT_DEFAULT, INT nFont = FONT_DEFAULT);
ERROR_CODE PaintShadowedText(CBofBitmap *, CBofRect *pRect, const CHAR *, const INT nSize, const INT nWeight, const RGBCOLOR cColor = CTEXT_COLOR, INT nJustify = JUSTIFY_CENTER, uint32 n = FORMAT_DEFAULT, INT nFont = FONT_DEFAULT);

/**
 * Utility routine that will calculate the rectangle that a text string
 * will fit in, given point size and font.
 */
CBofRect CalculateTextRect(CBofWindow *pWnd, const CBofString *pStr, INT nSize, INT nFont);
CBofRect CalculateTextRect(CBofRect rect, const CBofString *pStr, INT nSize, INT nFont);

} // namespace Bagel

#endif
