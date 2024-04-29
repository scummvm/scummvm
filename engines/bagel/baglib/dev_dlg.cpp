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

#include "bagel/baglib/dev_dlg.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/pan_window.h"
#include "bagel/baglib/bagel.h"
#include "bagel/boflib/std_keys.h"

namespace Bagel {

#define MENU_DFLT_HEIGHT 20

CDevDlg::CDevDlg(int nButtonX, int buttonY) {
	_titleText = nullptr;
	_buttonX = nButtonX;
	_buttonY = buttonY;
	_achGuess[0] = '\0';
	_guessCount = 0;
	_useExtraFl = false;
}

CDevDlg::~CDevDlg() {
	if (_titleText != nullptr) {
		delete _titleText;
		_titleText = nullptr;
	}
}

ErrorCode CDevDlg::create(const char *bmp, CBofWindow *wnd, CBofPalette *pal, CBofRect *rect, bool useExtraFl) {
	Assert(IsValidObject(this));
	Assert(bmp != nullptr);
	Assert(wnd != nullptr);
	Assert(pal != nullptr);
	Assert(rect != nullptr);

	_useExtraFl = useExtraFl;

	CBofRect r;
	if (!rect) {
		r = wnd->GetWindowRect();
		r.OffsetRect(-r.left, -r.top);
		r.bottom = r.top + MENU_DFLT_HEIGHT;
	} else {
		r = *rect;
	}

	_guessText = new CBofText();
	if (_guessText != nullptr) {
		_guessText->SetupText(rect, JUSTIFY_LEFT);
	}

	_guessCount = 0;
	Common::fill(_achGuess, _achGuess + ACH_GUESS_MAX_CHARS, 0);

	CBofBitmap *bitmap = nullptr;

	if (bmp != nullptr) {
		bitmap = new CBofBitmap(bmp, pal);
		if (bitmap == nullptr) {
			ReportError(ERR_MEMORY);
		}
	}

	// Fall back to original dialog on failure
	if (bitmap == nullptr) {
		bitmap = new CBofBitmap(200, 100, pal);
		if (bitmap != nullptr) {
			Assert(pal != nullptr);

			bitmap->FillRect(nullptr, pal->GetNearestIndex(RGB(92, 92, 92)));

			CBofRect bmpRect(bitmap->GetRect());
			bitmap->DrawRect(&bmpRect, pal->GetNearestIndex(RGB(0, 0, 0)));
			bitmap->FillRect(rect, pal->GetNearestIndex(RGB(255, 255, 255)));

		} else {
			ReportError(ERR_MEMORY);
		}
	}

	Assert(bitmap != nullptr);
	CBofRect bmpRect(bitmap->GetRect());
	CBofString className = "CDevDlg";
	CBagStorageDevDlg::create(className, &bmpRect, wnd, 0);
	Center();
	SetBackdrop(bitmap);

	return _errCode;
}

void CDevDlg::onLButtonUp(uint32 /*nFlags*/, CBofPoint * /*pPoint*/, void *) {
}

void CDevDlg::onMouseMove(uint32 /*nFlags*/, CBofPoint * /*pPoint*/, void *) {
}

void CDevDlg::onClose() {
	Assert(IsValidObject(this));

	CBagVar *varDialogReturn = VARMNGR->GetVariable("DIALOGRETURN");
	if (varDialogReturn != nullptr) {
		// If we need to parse the input for 2 words (Deven-7 Code words)
		if (_useExtraFl) {
			// Find the break
			char *p = strchr(_achGuess, ',');
			if ((p != nullptr) || ((p = strchr(_achGuess, ' ')) != nullptr)) {
				*p = '\0';
				p++;

				// Set variable 2 (DIALOGRETURN2)
				CBagVar *varDialogReturn2 = VARMNGR->GetVariable("DIALOGRETURN2");
				if (varDialogReturn2 != nullptr) {
					varDialogReturn2->SetValue(p);
				}
			}
		}

		// Set variable 1 (DIALOGRETURN)
		varDialogReturn->SetValue(_achGuess);
	}

	delete _guessText;
	_guessText = nullptr;

	CBagStorageDevDlg::onClose();
}

void CDevDlg::onKeyHit(uint32 keyCode, uint32 repCount) {
	Assert(IsValidObject(this));

	if (_guessCount < ACH_GUESS_MAX_CHARS) {
		bool paintGuessFl = false;
		const char *const MISC_KEYS = " \'$,-%.";

		if (Common::isAlnum(keyCode) || strchr(MISC_KEYS, keyCode)) {
			_achGuess[_guessCount] = toupper(keyCode);
			_guessCount++;
			paintGuessFl = true;

		} else if (keyCode == BKEY_BACK && (_guessCount - 1) >= 0) {
			_guessCount--;
			_achGuess[_guessCount] = 0;
			paintGuessFl = true;
		}

		if (paintGuessFl) {
			paintText();
		}

		// Close dialog box when enter key is hit
		if (keyCode ==  BKEY_ENTER) {
			onClose();
		}
	}

	// If it maxes out, clear it
	if (_guessCount >= ACH_GUESS_MAX_CHARS) {
		Common::fill(_achGuess, _achGuess + ACH_GUESS_MAX_CHARS, 0);
		_guessCount = 0;
	}

	CBagStorageDevDlg::onKeyHit(keyCode, repCount);
}

void CDevDlg::paintText() {
	Assert(IsValidObject(this));

	char achTemp[ACH_GUESS_MAX_CHARS];
	snprintf(achTemp, ACH_GUESS_MAX_CHARS, "%s", _achGuess);

	_guessText->display(GetBackdrop(), achTemp, 16, TEXT_MEDIUM);
}

void CDevDlg::setText(CBofString &text, CBofRect *textRect) {
	Assert(IsValidObject(this));

	_titleText = new CBofText;
	if (_titleText != nullptr) {
		_titleText->SetupText(textRect, JUSTIFY_CENTER, FORMAT_DEFAULT);
		_titleText->setColor(RGB(255, 255, 255));
		_titleText->SetSize(FONT_14POINT);
		_titleText->setWeight(TEXT_BOLD);
		_titleText->setText(text);
	}
}

// Override on render to do the painting, but call the default anyway.
ErrorCode CDevDlg::onRender(CBofBitmap *bmp, CBofRect *rect) {
	Assert(IsValidObject(this));

	ErrorCode errCode = CBagStorageDevDlg::onRender(bmp, rect);

	if (_titleText != nullptr) {
		_titleText->display(GetBackdrop());
	}

	return errCode;
}

} // namespace Bagel
