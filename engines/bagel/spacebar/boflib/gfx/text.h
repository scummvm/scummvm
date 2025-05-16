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
#include "bagel/spacebar/boflib/gfx/bitmap.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/string.h"

namespace Bagel {
namespace SpaceBar {

#define NUM_POINT_SIZES 32

// Text color and offset definitions
#define CTEXT_COLOR         RGB(0,0,0)
#define CTEXT_SHADOW_COLOR  RGB(0,0,0)
#define CTEXT_YELLOW        RGB(255, 255, 0)
#define CTEXT_WHITE         RGB(255, 255, 255)

#define CTEXT_SHADOW_DX     2
#define CTEXT_SHADOW_DY     2

// Text justification definitions
enum {
	JUSTIFY_CENTER      = 0,
	JUSTIFY_LEFT        = 1,
	JUSTIFY_RIGHT       = 2,
	JUSTIFY_WRAP        = 3
};

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

#define FONT_DEFAULT_SIZE       (-14)
#define FONT_8POINT             8
#define FONT_10POINT            10
#define FONT_12POINT            12
#define FONT_14POINT            14
#define FONT_15POINT            15
#define FONT_18POINT            18
#define FONT_20POINT            20
#define TEXT_DEFAULT_FACE       TEXT_BOLD


class CBofText : public CBofObject, public CBofError {
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
	 * @param pRect         Rectangular area encompassed by the text object
	 * @param nJustify      Alignment of text in the rectangle
	 * @param nFormatFlags  Format flag
	 */
	ErrorCode setupText(const CBofRect *pRect, int nJustify = JUSTIFY_CENTER, uint32 nFormatFlags = FORMAT_DEFAULT);
	ErrorCode setupTextOpt(const CBofRect *pRect, int nJustify = JUSTIFY_CENTER, uint32 nFormatFlags = FORMAT_DEFAULT);

	void setText(const CBofString &cString) {
		_cCurString = cString;
	}
	void setColor(const COLORREF cColor) {
		_cTextColor = cColor;
	}
	void SetSize(const int nSize) {
		_nCurSize = nSize;
	}
	void setWeight(const int nWeight) {
		_nCurWeight = nWeight;
	}

	void setShadowColor(const COLORREF cColor) {
		_cShadowColor = cColor;
	}
	void setShadowSize(int nDX, int nDY) {
		_nShadow_DX = nDX;
		_nShadow_DY = nDY;
	}

	CBofString getText() const {
		return _cCurString;
	}
	COLORREF getColor() const {
		return _cTextColor;
	}
	int getSize() const {
		return _nCurSize;
	}
	int getWeight() const {
		return _nCurWeight;
	}

	/**
	 * Restores the background behind current text on screen
	 * @param pWnd      Window to erase text from
	 * @return          Error return Code
	 */
	ErrorCode erase(CBofWindow *pWnd);

	/**
	 * Restores the background behind current text offscreen
	 * @param pBmp      Offscreen bitmap to erase text from
	 * @return          Error return Code
	 */
	ErrorCode erase(CBofBitmap *pBmp);

	/**
	 * Re-displays current text, formatted with current attribs
	 * @param pWnd      Window to paint into
	 * @return          Error return Code
	 */
	ErrorCode display(CBofWindow *pWnd);

	/**
	 * Re-displays current text, formatted with current attribs
	 * @param pBmp      Bitmap to paint into
	 * @return          Error return Code
	 */
	ErrorCode display(CBofBitmap *pBmp);

	/**
	 * Display a text string, formatted in the current text area
	 * @param pWnd          Window to paint into
	 * @param pszText       Point to text string to be displayed
	 * @param nSize         Point size of the text to be used
	 * @param nWeight       Weighting of the font (FW_ identifier)
	 * @param cColor        Color that the text will be
	 * @param nFont         Font used (default or mono)
	 * @return              Error return Code
	 */
	ErrorCode display(CBofWindow *pWnd, const char *pszText, int nSize, int nWeight, COLORREF cColor = CTEXT_COLOR, int nFont = FONT_DEFAULT);

	/**
	 * Display a text string, formatted in the current text area
	 * @param pBmp          Bitmap to paint into
	 * @param pszText       Point to text string to be displayed
	 * @param nSize         Point size of the text to be used
	 * @param nWeight       Weighting of the font (FW_ identifier)
	 * @param cColor        Color that the text will be
	 * @param nFont         Font used (default or mono)
	 * @return              Error return Code
	 */
	ErrorCode display(CBofBitmap *pBmp, const char *pszText, int nSize, int nWeight, COLORREF cColor = CTEXT_COLOR, int nFont = FONT_DEFAULT);

	/**
	 * Display a shadowed text string into the current text area
	 * @param pWnd          Window to paint into
	 * @param pszText       Point to text string to be displayed
	 * @param nSize         Point size of the text to be used
	 * @param nWeight       Weighting of the font (FW_ identifier)
	 * @param cColor        Color that the text will be
	 * @param cShadow       Color that the text's shadow will be
	 * @param nDX           Shadow DX
	 * @param nDY           Shadow DY
	 * @param nFont         Font used (default or mono)
	 * @return              Error return Code
	 */
	ErrorCode displayShadowed(CBofWindow *pWnd, const char *pszText, int nSize,
	                          int nWeight, COLORREF cColor, COLORREF cShadow = CTEXT_SHADOW_COLOR,
	                          int nDX = CTEXT_SHADOW_DX, int nDY = CTEXT_SHADOW_DY, int nFont = FONT_DEFAULT);


	void flushBackground() {
		_bSaved = false;
	}

	static ErrorCode initialize();
	static ErrorCode shutdown();

	/**
	 * Displays specified text onto specified bitmap
	 * @param pBmp          Bitmap to paint text onto
	 * @param pszText       Pointer to text string to be displayed
	 * @param pRect         Area to paint text to
	 * @param nSize         Point size of the text to be used
	 * @param nWeight       Weighting of the font (FW_ identifier)
	 * @param bShadowed     Whether the text is shadowed
	 * @param nFont         Font used (default or mono)
	 * @return              Error return Code
	 */
	ErrorCode displayTextEx(CBofBitmap *pBmp, const char *pszText, CBofRect *pRect, int nSize, int nWeight, bool bShadowed, int nFont = FONT_DEFAULT);

	static Graphics::Font *getFont(int nFont, int nSize, int nWeight);

private:
	/**
	 * Initializes key fields to zero or nullptr states.
	 */
	void initializeFields();

	/**
	 * Displays specified text onto specified bitmap
	 * @param pWnd          Window to paint text onto
	 * @param pszText       Pointer to text string to be displayed
	 * @param pRect         Area to paint text to
	 * @param nSize         Point size of the text to be used
	 * @param nWeight       Weighting of the font (FW_ identifier)
	 * @param bShadowed     Whether the text is shadowed
	 * @param nFont         Font used (default or mono)
	 * @return              Error return Code
	 */
	ErrorCode displayText(CBofWindow *pWnd, const char *pszText, CBofRect *pRect, int nSize, int nWeight, bool bShadowed, int nFont = FONT_DEFAULT);
	ErrorCode displayText(CBofBitmap *pBmp, const char *pszText, CBofRect *pRect, int nSize, int nWeight, bool bShadowed, int nFont = FONT_DEFAULT);

	void displayLine(Graphics::Font *font, Graphics::ManagedSurface &surface, const Common::U32String &line,
	                 int left, int top, int width, int color, Graphics::TextAlign align);

protected:
	CBofString  _cCurString;       // text to be displayed
	CBofRect    _cRect;            // bounding rectangle of text area
	CBofPoint   _cPosition;        // upper left corner of text displayed
	CBofSize    _cSize;            // dx/dy size of the text bitmap
	COLORREF    _cTextColor;       // color to use for the text itself
	COLORREF    _cShadowColor;     // color to use for the text's shadow
	CBofBitmap *_pBackground;      // bitmap for the text's background
	CBofBitmap *_pWork;            // bitmap for the work area

	int         _nCurSize;         // point size of current text
	int         _nCurWeight;       // style of current text

	int         _nJustify;         // positioning within the rectangle
	int         _nShadow_DX;       // horizontal offset for shadow
	int         _nShadow_DY;       // vertical offset for shadow
	uint32      _nFormatFlags;     // multi line formatting flags
	bool        _bMultiLine;       // multi vs single line formatting
	bool        _bSaved;

	static Graphics::Font *_defaultFonts[NUM_POINT_SIZES];
	static Graphics::Font *_fixedFonts[NUM_POINT_SIZES];

	static bool  _initialized;

	static int   _tabStop;     // tabstop table
};

// Global text functions
//
ErrorCode paintText(CBofWindow *pWnd, CBofRect *pRect, const char *, int nSize, int nWeight, COLORREF cColor = CTEXT_COLOR, int nJustify = JUSTIFY_CENTER, uint32 nFormat = FORMAT_DEFAULT, int nFont = FONT_DEFAULT);
ErrorCode paintText(CBofBitmap *pBmp, CBofRect *pRect, const char *, int nSize, int nWeight, COLORREF cColor = CTEXT_COLOR, int nJustify = JUSTIFY_CENTER, uint32 nFormat = FORMAT_DEFAULT, int nFont = FONT_DEFAULT);

ErrorCode paintShadowedText(CBofBitmap *, CBofRect *pRect, const char *, int nSize, int nWeight, COLORREF cColor = CTEXT_COLOR, int nJustify = JUSTIFY_CENTER, uint32 n = FORMAT_DEFAULT, int nFont = FONT_DEFAULT);

/**
 * Utility routine that will calculate the rectangle that a text string
 * will fit in, given point size and font.
 */
CBofRect calculateTextRect(CBofWindow *pWnd, const CBofString *pStr, int nSize, int nFont);
CBofRect calculateTextRect(CBofRect rect, const CBofString *pStr, int nSize, int nFont);

} // namespace SpaceBar
} // namespace Bagel

#endif
