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

#include "bagel/boflib/gui/window.h"
#include "bagel/boflib/gfx/bitmap.h"
#include "bagel/boflib/gfx/palette.h"

namespace Bagel {

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

	virtual void onPaint(CBofRect *pRect);
	virtual void onClose();
	virtual void onInitDialog();

public:
	/**
	 * Constructor
	 */
	CBofDialog();

	/**
	 * Constructor
	 */
	CBofDialog(const char *pszFileName, CBofWindow *pParent, const uint32 nID = 0, const uint32 lFlags = BOFDLG_DEFAULT);

	/**
	 * Destructor
	 */
	virtual ~CBofDialog();

	/**
	 * Creates the dialog
	 * @param pszName		Dialog name
	 * @param x				Top-left X position
	 * @param y				Top-left Y position
	 * @param nWidth		Width
	 * @param nHeight		Height
	 * @param pParent		Parent window
	 * @param nControlID	Control Id
	 * @return				Error return code
	 */
	ErrorCode create(const char *pszName, int x = 0, int y = 0, int nWidth = USE_DEFAULT, int nHeight = USE_DEFAULT, CBofWindow *pParent = nullptr, uint32 nControlID = 0);

	/**
	 * Creates the dialog
	 * @param pszName		Dialog name
	 * @param pRect			Dialog bounds
	 * @param pParent		Parent window
	 * @param nControlID	Control Id
	 * @return				Error return code
	 */
	ErrorCode create(const char *pszName, CBofRect *pRect = nullptr, CBofWindow *pParent = nullptr, uint32 nControlID = 0);

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

} // namespace Bagel

#endif
