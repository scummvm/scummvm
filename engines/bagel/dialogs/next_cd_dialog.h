
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

#ifndef BAGEL_DIALOGS_NEXT_CD_DIALOG_H
#define BAGEL_DIALOGS_NEXT_CD_DIALOG_H

#include "bagel/boflib/gui/dialog.h"
#include "bagel/boflib/gui/button.h"
#include "bagel/boflib/gfx/text.h"

namespace Bagel {

#define NUM_QUIT_BUTTONS 3

class CBagQuitDialog : public CBofDialog {
public:
	CBagQuitDialog();

	virtual void onInitDialog();

protected:
	virtual void onPaint(CBofRect *pRect);
	virtual void onClose();
	virtual void onBofButton(CBofObject *pObject, int nState);

	virtual void onKeyHit(uint32 lKey, uint32 lRepCount);

	// Data
	//
	CBofBmpButton *_pButtons[NUM_QUIT_BUTTONS];
};

class CBagNextCDDialog : public CBofDialog {
public:
	CBagNextCDDialog();

	virtual void onInitDialog();

protected:
	virtual void onPaint(CBofRect *pRect);
	virtual void onClose();
	virtual void onBofButton(CBofObject *pObject, int nState);

	virtual void onKeyHit(uint32 lKey, uint32 lRepCount);

	// Data
	CBofBmpButton *_pButton;
};

class CBagCreditsDialog : public CBofDialog {
public:
	CBagCreditsDialog();

	virtual void onInitDialog();

protected:
	virtual void onPaint(CBofRect *pRect);
	virtual void onClose();

	virtual void onKeyHit(uint32 lKey, uint32 lRepCount);
	virtual void onLButtonDown(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);

	virtual void onMainLoop();

	ErrorCode nextScreen();
	ErrorCode displayCredits();
	ErrorCode loadNextTextFile();
	int linesPerPage();
	void nextLine();
	ErrorCode paintLine(int nLine, char *pszText);

	CBofBitmap *_pCreditsBmp;
	CBofBitmap *_pSaveBmp;

	char *_pszNextLine;
	char *_pszEnd;
	char *_pszText;

	int _nLines;
	int _nNumPixels;
	int _iScreen;

	bool _bDisplay;
};

} // namespace Bagel

#endif
