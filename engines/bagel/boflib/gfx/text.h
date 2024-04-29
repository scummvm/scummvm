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
#define DT_intERNAL 0x00001000


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
	CBofText(const CBofRect *pRect, int nJustify = JUSTIFY_CENTER, uint32 nFormatFlags = FORMAT_DEFAULT);
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
	ErrorCode SetupText(const CBofRect *pRect, int nJustify = JUSTIFY_CENTER, uint32 nFormatFlags = FORMAT_DEFAULT);
	ErrorCode SetupTextOpt(const CBofRect *pRect, int nJustify = JUSTIFY_CENTER, uint32 nFormatFlags = FORMAT_DEFAULT);

	void setText(const CBofString &cString) {
		m_cCurString = cString;
	}
	void SetColor(const RGBCOLOR cColor) {
		m_cTextColor = cColor;
	}
	void SetSize(const int nSize) {
		m_nCurSize = nSize;
	}
	void SetWeight(const int nWeight) {
		m_nCurWeight = nWeight;
	}

	void SetShadowColor(const RGBCOLOR cColor) {
		m_cShadowColor = cColor;
	}
	void SetShadowSize(int nDX, int nDY) {
		m_nShadow_DX = nDX;
		m_nShadow_DY = nDY;
	}

	CBofString getText() const {
		return m_cCurString;
	}
	RGBCOLOR GetColor() const {
		return m_cTextColor;
	}
	int GetSize() const {
		return m_nCurSize;
	}
	int GetWeight() const {
		return m_nCurWeight;
	}

	/**
	 * Restores the background behind current text on screen
	 * @param pWnd		Window to erase text from
	 * @return			Error return Code
	 */
	ErrorCode Erase(CBofWindow *pWnd);

	/**
	 * Restores the background behind current text offscreen
	 * @param pBmp		Offscreen bitmap to erase text from
	 * @return			Error return Code
	 */
	ErrorCode Erase(CBofBitmap *pBmp);

	/**
	 * Re-displays current text, formatted with current attribs
	 * @param pWnd		Window to paint into
	 * @return			Error return Code
	 */
	ErrorCode Display(CBofWindow *pWnd);

	/**
	 * Re-displays current text, formatted with current attribs
	 * @param pBmp		Bitmap to paint into
	 * @return			Error return Code
	 */
	ErrorCode Display(CBofBitmap *pBmp);

	/**
	 * Display a text string, formatted in the current text area
	 * @param pWnd			Window to paint into
	 * @param pszText		Point to text string to be displayed
	 * @param nSize			Point size of the text to be used
	 * @param nWeight		Weighting of the font (FW_ identifier)
	 * @param cColor		Color that the text will be
	 * @return				Error return Code
	 */
	ErrorCode Display(CBofWindow *pWnd, const char *pszText, const int nSize, const int nWeight, const RGBCOLOR cColor = CTEXT_COLOR, int nFont = FONT_DEFAULT);

	/**
	 * Display a text string, formatted in the current text area
	 * @param pBmp			Bitmap to paint into
	 * @param pszText		Point to text string to be displayed
	 * @param nSize			Point size of the text to be used
	 * @param nWeight		Weighting of the font (FW_ identifier)
	 * @param cColor		Color that the text will be
	 * @return				Error return Code
	 */
	ErrorCode Display(CBofBitmap *pBmp, const char *pszText, const int nSize, const int nWeight, const RGBCOLOR cColor = CTEXT_COLOR, int nFont = FONT_DEFAULT);

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
	ErrorCode DisplayShadowed(CBofWindow *pWnd, const char *pszText, int nSize,
		int nWeight, RGBCOLOR cColor, RGBCOLOR cShadow = CTEXT_SHADOW_COLOR,
		int nDX = CTEXT_SHADOW_DX, int nDY = CTEXT_SHADOW_DY, int n = FONT_DEFAULT);

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
	ErrorCode DisplayShadowed(CBofBitmap *, const char *, const int, const int, const RGBCOLOR cColor, const RGBCOLOR cShadow = CTEXT_SHADOW_COLOR, const int nDX = CTEXT_SHADOW_DX, const int nDY = CTEXT_SHADOW_DY, int n = FONT_DEFAULT);

	void FlushBackground()       {
		m_bSaved = false;
	}

	static ErrorCode initialize();
	static ErrorCode shutdown();

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
	ErrorCode DisplayTextEx(CBofBitmap *pBmp, const char *pszText, CBofRect *pRect, const int nSize, const int nWeight, const bool bShadowed, int nFont = FONT_DEFAULT);

	static Graphics::Font *getFont(int nFont, int nSize, int nWeight);

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
	ErrorCode DisplayText(CBofWindow *pWnd, const char *pszText, CBofRect *pRect, const int nSize, const int nWeight, const bool bShadowed, int nFont = FONT_DEFAULT);
	ErrorCode DisplayText(CBofBitmap *pBmp, const char *pszText, CBofRect *pRect, const int nSize, const int nWeight, const bool bShadowed, int nFont = FONT_DEFAULT);

	void displayLine(Graphics::Font *font, Graphics::ManagedSurface &surface, const Common::String &line,
		int left, int top, int width, int color, Graphics::TextAlign align);

protected:
	CBofString  m_cCurString;       // text to be displayed
	CBofRect    _cRect;            // bounding rectangle of text area
	CBofPoint   m_cPosition;        // upper left corner of text displayed
	CBofSize    m_cSize;            // dx/dy size of the text bitmap
	RGBCOLOR    m_cTextColor;       // color to use for the text itself
	RGBCOLOR    m_cShadowColor;     // color to use for the text's shadow
	CBofBitmap *m_pBackground;      // bitmap for the text's background
	CBofBitmap *m_pWork;            // bitmap for the work area

	int         m_nCurSize;         // point size of current text
	int         m_nCurWeight;       // style of current text

	int         m_nJustify;         // positioning within the rectangle
	int         m_nShadow_DX;       // horizontal offset for shadow
	int         m_nShadow_DY;       // vertical offset for shadow
	uint32        m_nFormatFlags;     // multi line formatting flags
	bool        m_bMultiLine;       // mutli vs single line formatting
	bool        m_bSaved;

	static Graphics::Font *_defaultFonts[NUM_POINT_SIZES];
	static Graphics::Font *_fixedFonts[NUM_POINT_SIZES];

	static bool  _initialized;

	static int   _tabStop;     // tabstop table
};


// Global text functions
//
ErrorCode PaintText(CBofWindow *pWnd, CBofRect *pRect, const char *, const int nSize, const int nWeight, const RGBCOLOR cColor = CTEXT_COLOR, int nJustify = JUSTIFY_CENTER, uint32 nFormat = FORMAT_DEFAULT, int nFont = FONT_DEFAULT);
ErrorCode PaintText(CBofBitmap *pBmp, CBofRect *pRect, const char *, const int nSize, const int nWeight, const RGBCOLOR cColor = CTEXT_COLOR, int nJustify = JUSTIFY_CENTER, uint32 nFormat = FORMAT_DEFAULT, int nFont = FONT_DEFAULT);

ErrorCode PaintShadowedText(CBofWindow *, CBofRect *pRect, const char *, const int nSize, const int nWeight, const RGBCOLOR cColor = CTEXT_COLOR, int nJustify = JUSTIFY_CENTER, uint32 n = FORMAT_DEFAULT, int nFont = FONT_DEFAULT);
ErrorCode PaintShadowedText(CBofBitmap *, CBofRect *pRect, const char *, const int nSize, const int nWeight, const RGBCOLOR cColor = CTEXT_COLOR, int nJustify = JUSTIFY_CENTER, uint32 n = FORMAT_DEFAULT, int nFont = FONT_DEFAULT);

/**
 * Utility routine that will calculate the rectangle that a text string
 * will fit in, given point size and font.
 */
CBofRect CalculateTextRect(CBofWindow *pWnd, const CBofString *pStr, int nSize, int nFont);
CBofRect CalculateTextRect(CBofRect rect, const CBofString *pStr, int nSize, int nFont);

} // namespace Bagel

#endif
