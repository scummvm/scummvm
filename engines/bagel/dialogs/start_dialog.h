
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

#ifndef BAGEL_DIALOGS_START_DIALOG_H
#define BAGEL_DIALOGS_START_DIALOG_H

#include "bagel/boflib/gui/list_box.h"
#include "bagel/boflib/gui/button.h"
#include "bagel/boflib/gui/text_box.h"
#include "bagel/dialogs/save_dialog.h"

namespace Bagel {

#define NUM_START_BTNS 3

class CBagStartDialog : public CBofDialog {
private:
	CBofBmpButton *_buttons[NUM_START_BTNS];
	CBofPalette *_savePalette = nullptr;

protected:
	void onPaint(CBofRect *pRect) override;
	void onClose() override;
	void onBofButton(CBofObject *pObject, int nFlags) override;

	void onKeyHit(uint32 lKey, uint32 nRepCount) override;

public:
	CBagStartDialog(const char *pszFileName, CBofWindow *pWin);

	void onInitDialog() override;
};

} // namespace Bagel

#endif
