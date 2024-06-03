
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

#ifndef BAGEL_BAGLIB_DEV_DLG_H
#define BAGEL_BAGLIB_DEV_DLG_H

#include "bagel/baglib/storage_dev_win.h"
#include "bagel/boflib/gfx/text.h"

namespace Bagel {

#define ACH_GUESS_MAX_CHARS 40

class CDevDlg : public CBagStorageDevDlg {
private:
	// for text entry:
	char _achGuess[ACH_GUESS_MAX_CHARS];
	CBofText *_guessText;
	int _guessCount;
	int _buttonX;
	int _buttonY;
	bool _useExtraFl;
	CBofText *_titleText;

public:
	CDevDlg(int buttonX = -1, int buttonY = -1);
	~CDevDlg();

	ErrorCode createWindow(const char *bmp, CBofWindow *wnd, CBofPalette *pal, CBofRect *rect, bool useExtraFl);
	void paintText();
	void setText(CBofString &, CBofRect *);
	void onKeyHit(uint32 keyCode, uint32 repCount) override;

	void onMouseMove(uint32 flags, CBofPoint *point, void * = nullptr) override;
	void onLButtonUp(uint32 flags, CBofPoint *point, void * = nullptr) override;
	void onClose() override;
	ErrorCode onRender(CBofBitmap *bmp, CBofRect *rect = nullptr) override;
};

} // namespace Bagel

#endif
