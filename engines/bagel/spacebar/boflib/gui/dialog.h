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
 * along with this program.  If not, see <http://www.gnu.org/licenses/".
 *
 */

#ifndef BAGEL_BOFLIB_GUI_DIALOG_H
#define BAGEL_BOFLIB_GUI_DIALOG_H

#include "bagel/spacebar/boflib/gui/window.h"
#include "bagel/spacebar/boflib/gfx/bitmap.h"

namespace Bagel {
namespace SpaceBar {

#define IDOK        1
#define IDCANCEL    2

#define BOFDLG_TRANSPARENT  0x00000001
#define BOFDLG_SAVEBACKGND  0x00000002

#define BOFDLG_DEFAULT (BOFDLG_TRANSPARENT /* | BOFDLG_SAVEBACKGND*/)

class CBofDialog : public CBofWindow {
protected:
	CBofBitmap *_pDlgBackground = nullptr;
	uint32 _lFlags = 0;
	int _nReturnValue = 0;

	bool _bFirstTime = false;
	bool _bTempBitmap = false;
	bool _bEndDialog = false;
	bool _bHavePainted = false;

protected:
	virtual ErrorCode paint(CBofRect *pRect);
	virtual ErrorCode paintBackground();
	virtual ErrorCode saveBackground();
	virtual ErrorCode killBackground();

	void onPaint(CBofRect *pRect) override;
	void onClose() override;
	virtual void onInitDialog();

public:
	/**
	 * Constructor
	 */
	CBofDialog();

	/**
	 * Constructor
	 */
	CBofDialog(const char *pszFileName, CBofWindow *pParent, uint32 nID = 0, uint32 lFlags = BOFDLG_DEFAULT);

	/**
	 * Destructor
	 */
	virtual ~CBofDialog();

	/**
	 * Creates the dialog
	 * @param pszName       Dialog name
	 * @param x             Top-left X position
	 * @param y             Top-left Y position
	 * @param nWidth        Width
	 * @param nHeight       Height
	 * @param pParent       Parent window
	 * @param nControlID    Control Id
	 * @return              Error return code
	 */
	ErrorCode create(const char *pszName, int x, int y, int nWidth, int nHeight, CBofWindow *pParent, uint32 nControlID = 0) override;

	/**
	 * Creates the dialog
	 * @param pszName       Dialog name
	 * @param pRect         Dialog bounds
	 * @param pParent       Parent window
	 * @param nControlID    Control Id
	 * @return              Error return code
	 */
	ErrorCode create(const char *pszName, CBofRect *pRect, CBofWindow *pParent, uint32 nControlID = 0) override;

	/**
	 * Set the dialog flags
	 */
	void setFlags(uint32 lFlags) {
		_lFlags = lFlags;
	}

	/**
	 * Return the dialog's flags
	 */
	uint32 getFlags() const {
		return _lFlags;
	}

	/**
	 * Show the dialog as a modal
	 */
	int doModal();

	/**
	 * End the dialog modal display
	 */
	void endModal() {
		_bEndDialog = true;
	}

	/**
	 * Set the dialog's return value
	 */
	void setReturnValue(int nValue) {
		_nReturnValue = nValue;
	}

	/**
	 * Get the dialog's return value
	 * @return
	 */
	int getReturnValue() const {
		return _nReturnValue;
	}
};

} // namespace SpaceBar
} // namespace Bagel

#endif
