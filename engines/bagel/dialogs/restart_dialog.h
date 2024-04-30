
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

#ifndef BAGEL_DIALOGS_RESTART_DIALOG_H
#define BAGEL_DIALOGS_RESTART_DIALOG_H

#include "bagel/boflib/gui/button.h"
#include "bagel/boflib/gui/text_box.h"
#include "bagel/dialogs/save_dialog.h"

namespace Bagel {

#define NUM_RESTART_BTNS 2

class CBagRestartDialog : public CBofDialog {
public:
	CBagRestartDialog(const char *pszFileName, CBofWindow *pWin);

	virtual void onInitDialog();

protected:
	virtual void onPaint(CBofRect *pRect);
	virtual void onClose();
	virtual void onBofButton(CBofObject *pObject, int nState);

	virtual void onKeyHit(uint32 lKey, uint32 lRepCount);

	// Data
	CBofBmpButton *_pButtons[NUM_RESTART_BTNS];
	CBofPalette *_pSavePalette;
};

} // namespace Bagel

#endif
