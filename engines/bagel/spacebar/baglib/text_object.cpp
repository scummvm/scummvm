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

#include "bagel/spacebar/baglib/text_object.h"
#include "bagel/spacebar/baglib/master_win.h"
#include "bagel/spacebar/baglib/pan_window.h"
#include "bagel/spacebar/baglib/zoom_pda.h"
#include "bagel/spacebar/baglib/base_pda.h"
#include "bagel/spacebar/baglib/menu_dlg.h"
#include "bagel/spacebar/baglib/bagel.h"
#include "bagel/spacebar/baglib/rp_object.h"
#include "bagel/spacebar/boflib/gfx/text.h"

namespace Bagel {
namespace SpaceBar {

extern bool g_pauseTimerFl;

CBagTextObject::CBagTextObject() : CBagObject() {
	_xObjType = TEXT_OBJ;
	_nDX = 80;
	_nDY = 20;
	_psText = nullptr;
	CBagObject::setOverCursor(1); // Switch to cursor 1, 4 doesn't exist.

	_nPointSize = 16;
	_nFGColor = CTEXT_COLOR;
	_psInitInfo = nullptr;
	_bCaption = false;
	_bTitle = false;
	_bReAttach = false;
	_nTextFont = FONT_DEFAULT;

	setRPObject(nullptr);
}

CBagTextObject::~CBagTextObject() {
	delete _psInitInfo;
	_psInitInfo = nullptr;

	CBagTextObject::detach();
}

CBofRect CBagTextObject::getRect() {
	CBofPoint p = getPosition();
	CBofSize s = getSize();
	CBofRect r = CBofRect(p, s);
	return r;
}

ErrorCode CBagTextObject::update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect, int) {
	assert(isValidObject(this));
	assert(pBmp != nullptr);
	assert(pSrcRect != nullptr);

	// assume no error
	ErrorCode errorCode = ERR_NONE;

	if ((pBmp != nullptr) && isAttached() && !(getText().isEmpty())) {

		if (pBmp->getRect().ptInRect(pt)) {

			CBofRect r(pt, pSrcRect->size());

			int nPointSize = _nPointSize;
			int nFormat = FORMAT_CENTER_LEFT;
			if (!_bTitle) {
				byte c1 = 3;
				byte c2 = 9;

				CBofRect cBevel;
				cBevel.intersectRect(pBmp->getRect(), r);

				int left = cBevel.left;
				int top = cBevel.top;
				int right = cBevel.right;
				int bottom = cBevel.bottom;

				r.left += 6;
				r.top += 3;
				r.right -= 5;
				r.bottom -= 5;

				int i;
				for (i = 1; i <= 3; i++) {
					pBmp->line(left + i, bottom - i, right - i, bottom - i, c1);
					pBmp->line(right - i, bottom - i, right - i, top + i - 1, c1);
				}

				for (i = 1; i <= 3; i++) {
					pBmp->line(left + i, bottom - i, left + i, top + i - 1, c2);
					pBmp->line(left + i, top + i - 1, right - i, top + i - 1, c2);
				}
				nPointSize = 16;
				nFormat = FORMAT_TOP_LEFT;

			} else {
				r.left += 1;
			}

			errorCode = paintText(pBmp, &r, getText(), mapWindowsPointSize(nPointSize), TEXT_NORMAL, _nFGColor, JUSTIFY_WRAP, nFormat, _nTextFont);

			// This object does not need to be updated now...
			setDirty(false);
		}
	}

	return errorCode;
}

ErrorCode CBagTextObject::attach() {
	assert(isValidObject(this));

	if (!getFileName().right(4).find(".TXT") || !getFileName().right(4).find(".txt")) {
		// Prevent memory leak
		delete _psText;
		_psText = nullptr;

		// Allocate a new string
		_psText = new CBofString;

		CBofFile fpTextFile(getFileName());

		if (!fpTextFile.errorOccurred()) {
			// Allocate the buffers
			uint32 nFileLen = fpTextFile.getLength();
			char *pTextBuff = (char *)bofCleanAlloc(nFileLen + 1);

			// Read the text file into buffers
			fpTextFile.read(pTextBuff, nFileLen);
			fpTextFile.close();

			*_psText += pTextBuff;

			if (_psInitInfo != nullptr) {
				CBagVar *pVar = g_VarManager->getVariable(*_psInitInfo);

				if (pVar != nullptr) {
					_bReAttach = true;
					_psText->replaceStr("%s", pVar->getValue());
				}
			}

			bofFree(pTextBuff);

		} else {
			reportError(ERR_FOPEN, "Failed to create a CBofFile for %s", getFileName().getBuffer());
		}

		if (isCaption()) {
			recalcTextRect(true);
		}

	} else {
		// The Text is in the Bagel script, rather than a .txt file
		// Prevent memory leak
		delete _psText;
		_psText = nullptr;

		// Allocate a new string
		_psText = new CBofString;
		*_psText = getFileName();

		// Replace any underscores with spaces
		_psText->replaceChar('_', ' ');

		recalcTextRect(false);
	}

	// If this guy is linked to a residue printing object, make sure he knows
	// we've been attached.
	CBagRPObject *pRPObj = (CBagRPObject *)getRPObject();

	if (pRPObj != nullptr) {
		pRPObj->setTimeSet(false);
	}

	return CBagObject::attach();
}

ErrorCode CBagTextObject::detach() {
	assert(isValidObject(this));

	delete _psText;
	_psText = nullptr;

	return CBagObject::detach();
}

const CBofString &CBagTextObject::getText() {
	if (_psText)
		return *_psText;

	return getFileName();
}

void CBagTextObject::setText(const CBofString &s) {
	if (_psText) {
		*_psText = s;
	} else {
		setFileName(s);
	}

	recalcTextRect(!getFileName().right(4).find(".TXT") || !getFileName().right(4).find(".txt"));
}

//   Takes in info and then removes the relative information and returns the info
//   without the relevant info.
//
ParseCodes CBagTextObject::setInfo(CBagIfstream &istr) {
	bool nObjectUpdated = false;

	while (!istr.eof()) {
		istr.eatWhite();

		char ch = (char)istr.peek();
		switch (ch) {

		//
		//  VAR var - var is a BAGEL CBagVar variable (replaces all %s in text)
		//
		case 'V': {
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);

			// Need to use this field, so no one else can
			assert(_psInitInfo == nullptr);

			getAlphaNumFromStream(istr, sStr);

			if (!sStr.find("VAR")) {
				istr.eatWhite();
				getAlphaNumFromStream(istr, sStr);
				setInitInfo(sStr);
				nObjectUpdated = true;
			} else {
				putbackStringOnStream(istr, sStr);
			}
			break;
		}

		//
		//  SIZE n - n point size of the txt
		//
		case 'S': {
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);

			getAlphaNumFromStream(istr, sStr);

			if (!sStr.find("SIZE")) {
				istr.eatWhite();
				int n;
				getIntFromStream(istr, n);
				_nPointSize = (byte)n;
				nObjectUpdated = true;

				// WORKAROUND: Reduce the font size of Cilia's full-screen log
				// in ScummVM so that it fits on the screen
				if (_nPointSize == 28 && istr.getSize() == 359105)
					_nPointSize = 26;

			} else {
				putbackStringOnStream(istr, sStr);
			}
			break;
		}

		//
		//  FONT MONO or DEFAULT
		//
		case 'F': {
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);

			getAlphaNumFromStream(istr, sStr);

			if (!sStr.find("FONT")) {
				istr.eatWhite();
				int n;
				getIntFromStream(istr, n);
				_nTextFont = MapFont(n);
				nObjectUpdated = true;
			} else {
				putbackStringOnStream(istr, sStr);
			}
			// FIXME: Missing break?
		}
		// fallthrough

		//
		//  AS [CAPTION]  - how to run the link
		//
		case 'A': {
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);
			getAlphaNumFromStream(istr, sStr);

			if (!sStr.find("AS")) {
				istr.eatWhite();
				getAlphaNumFromStream(istr, sStr);
				if (!sStr.find("CAPTION")) {
					_bCaption = true;
					nObjectUpdated = true;

				} else if (!sStr.find("TITLE")) {
					_bTitle = true;
					nObjectUpdated = true;

				} else {
					putbackStringOnStream(istr, sStr);
					putbackStringOnStream(istr, "AS ");
				}
			} else {
				putbackStringOnStream(istr, sStr);
			}
			break;
		}

		//
		//  COLOR n - n color index
		//
		case 'C': {
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);

			getAlphaNumFromStream(istr, sStr);

			if (!sStr.find("COLOR")) {
				int nColor;
				istr.eatWhite();
				getIntFromStream(istr, nColor);
				setColor(nColor);
				nObjectUpdated = true;
			} else {
				putbackStringOnStream(istr, sStr);
			}
			break;
		}

		//
		// No match return from function
		//
		default: {
			ParseCodes parseCode = CBagObject::setInfo(istr);
			if (parseCode == PARSING_DONE) {
				return PARSING_DONE;
			}

			if (parseCode == UPDATED_OBJECT) {
				nObjectUpdated = true;
			} else { // rc==UNKNOWN_TOKEN
				if (nObjectUpdated)
					return UPDATED_OBJECT;

				return UNKNOWN_TOKEN;
			}
			break;
		}
		}
	}

	return PARSING_DONE;
}

void CBagTextObject::setColor(int nColor) {
	switch (nColor) {
	case 0:
		_nFGColor = RGB(0, 0, 0);
		break; // Black
	case 1:
		_nFGColor = RGB(226, 50, 51);
		break; // Red
	case 2:
		_nFGColor = RGB(255, 255, 255);
		break; // Yellow
	case 3:
		_nFGColor = RGB(255, 255, 255);
		break; // Green
	case 4:
		_nFGColor = RGB(255, 255, 255);
		break; // Green
	case 5:
		_nFGColor = RGB(0, 0, 255);
		break; // Blue
	case 6:
		_nFGColor = RGB(255, 0, 255);
		break;
	case 7:
		_nFGColor = CTEXT_WHITE;
		break; // White
	case 8:
		_nFGColor = CTEXT_YELLOW;
		break; // Yellow (chat highlight)
	default:
		break;
	}
}

void CBagTextObject::setProperty(const CBofString &sProp, int nVal) {
	if (!sProp.find("SIZE"))
		setPointSize(nVal);
	else if (!sProp.find("FONT"))
		setFont(MapFont(nVal));
	else if (!sProp.find("COLOR"))
		setColor(nVal);
	else
		CBagObject::setProperty(sProp, nVal);
}

int CBagTextObject::getProperty(const CBofString &sProp) {
	if (!sProp.find("SIZE"))
		return getPointSize();
	if (!sProp.find("FONT"))
		return getFont();
	if (!sProp.find("COLOR"))
		return getColor();

	return CBagObject::getProperty(sProp);
}

bool CBagTextObject::runObject() {
	char szLocalBuff[256];
	CBofString sStr(szLocalBuff, 256);

	if (_bCaption && isImmediateRun()) {

		// Re-attach this object to get any change in a variable
		// (Must be using the VAR token).
		if (_bReAttach) {
			attach();
		}

		CBagel *pApp = CBagel::getBagApp();
		if (pApp != nullptr) {
			CBagMasterWin *pWin = pApp->getMasterWnd();
			if (pWin != nullptr) {
				CBagStorageDevWnd *pParent = pWin->getCurrentStorageDev();

				CBofRect cRect(80, 10, 80 + 480 /*- 1 */, 10 + getRect().height() - 1 + 5);
				CBofPoint cPoint(0, 0);

				CBofPalette *pPal = pApp->getPalette();
				CBofBitmap cBmp(cRect.width(), cRect.height(), pPal);

				cBmp.fillRect(nullptr, pPal->getNearestIndex(RGB(92, 92, 92)));

				CBagMenuDlg cDlg;
				cDlg.createDlg(pParent, pPal, &cRect);

				update(cDlg.getBackdrop(), cPoint, &cRect);

				sStr = "BPDA_WLD";
				CBagPDA *pPDA = (CBagPDA *)g_SDevManager->getStorageDevice(sStr);

				// If we're in the zoom pda then put this box at the
				// bottom of the zoom rect.
				sStr = "BPDAZ_WLD";
				SBZoomPda *pPDAZ = (SBZoomPda *)g_SDevManager->getStorageDevice(sStr);

				if (pPDAZ && pPDAZ->getZoomed() == true) {
					CBofRect zRect = pPDAZ->getViewRect();
					assert(zRect.height() > 0 && zRect.height() < 480);
					assert(zRect.width() > 0 && zRect.width() < 640);
					cDlg.move(80, zRect.bottom - cRect.height(), true); // xxx
				} else if ((pPDA != nullptr) && (pPDA->isActivated() || pPDA->isActivating())) {
					cDlg.move(80, 10, true);

				} else {
					int x = 80;
					int y = 360 + 10 - cRect.height();
					cDlg.move(x, y, true);
				}
				g_pauseTimerFl = true;
				cDlg.doModal();
				g_pauseTimerFl = false;
			}
		}
	}

	return false;
}

int MapFont(int nFont) {
	if (nFont == 0)
		return FONT_MONO;

	return FONT_DEFAULT;
}

void CBagTextObject::onLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *pv) {
	// If there's a residue printing object, then hand this guy off to
	// him, otherwise, call back to Cbagobj.
	CBagRPObject *pRPObj = (CBagRPObject *)getRPObject();
	if (pRPObj) {
		pRPObj->onLButtonUp(nFlags, xPoint, pv);
		return;
	}

	CBagObject::onLButtonUp(nFlags, xPoint, pv);
}

void CBagTextObject::recalcTextRect(bool bTextFromFile) {
	CBofRect ViewRect;      // The rect of the area where objects are displayed
	CBofSize cDisplaySize;  // Size of rect needed to display font
	CBofSize cSize;         // Size of rect needed to display font

	assert(_psText != nullptr);

	// The window where the object are displayed
	CBagPanWindow *pPanWin = (CBagPanWindow *)(CBagel::getBagApp()->getMasterWnd()->getCurrentGameWindow());
	if (bTextFromFile) {
		if (pPanWin->getDeviceType() == SDEV_GAMEWIN) {
			ViewRect = pPanWin->getViewPort();
		} else {
			ViewRect = pPanWin->getClientRect();
		}
	}

	if (ViewRect.isRectEmpty()) {
		ViewRect.setRect(80, 10, 480 + 80 - 1, 360 + 10 - 1);
	}

	// Get the area spanned by the text (i.e. Get the pixel width and
	// height of the text string).
	CBofRect tmpRect = ViewRect;
	if (!_bTitle) {
		// Exactly match the width used in displayTextEx
		tmpRect.left += 5;
		tmpRect.right = (ViewRect.right == 640 ? PAN_AREA_WIDTH : ViewRect.right) - 5;
	}

	CBofRect textRect = calculateTextRect(tmpRect, _psText, _nPointSize, getFont());
	CBofSize stTextSize(textRect.right, textRect.bottom);

	if (bTextFromFile) {
		// Add fudge factor to make sure that all the text will fit, and not
		// get cut off.  This may cause an extra blank line of text in some
		// captions, but tough diddles, it's still better than truncating
		// some text.

		cSize.cx = stTextSize.cx;
		cSize.cy = stTextSize.cy;
	} else {
		cSize.cx = stTextSize.cx + 9;
		cSize.cy = stTextSize.cy + (_bTitle ? 0 : 7);
		setSize(cSize);
	}

	if (bTextFromFile) {
		cDisplaySize.cx = ViewRect.width();
		cDisplaySize.cy = cSize.cy;

		// If for some reason (CIC, CHAT) we got too large
		// a viewrect, cut it back to the correct width
		if (cDisplaySize.cx > PAN_AREA_WIDTH)
			cDisplaySize.cx = PAN_AREA_WIDTH;

		// Buffer the size a little for spacing etc.
		cDisplaySize.cx -= 5;

		// While the text is wider then the view area
		while (cSize.cx > cDisplaySize.cx) {
			// Increment Display Height to account for another line
			cDisplaySize.cy += cSize.cy;

			// Decrement the size of text by the width of one line
			cSize.cx -= cDisplaySize.cx;
		}

		// Add a little space at the bottom
		cDisplaySize.cy += 5;

		setSize(cDisplaySize);
	}
}

void CBagTextObject::setPSText(CBofString *p) {
	assert(isValidObject(this));

	delete _psText;
	_psText = nullptr;

	if (p != nullptr) {
		_psText = new CBofString(*p);
	}
}

} // namespace SpaceBar
} // namespace Bagel
