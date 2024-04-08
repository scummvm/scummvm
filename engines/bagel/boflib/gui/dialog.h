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

#if !BOF_WINDOWS
#define IDOK        1
#define IDCANCEL    2
#endif

#define BOFDLG_TRANSPARENT  0x00000001
#define BOFDLG_SAVEBACKGND  0x00000002

#define BOFDLG_DEFAULT (BOFDLG_TRANSPARENT /* | BOFDLG_SAVEBACKGND*/)

class CBofDialog : public CBofWindow {
protected:
	CBofBitmap *_pDlgBackground = nullptr;
	uint32 _lFlags = 0;
	INT _nReturnValue = 0;

	BOOL _bFirstTime = FALSE;
	BOOL _bTempBitmap = FALSE;
	BOOL _bEndDialog = FALSE;
	BOOL _bHavePainted = FALSE;

protected:
	virtual ERROR_CODE Paint(CBofRect *pRect);
	virtual ERROR_CODE PaintBackground();
	virtual ERROR_CODE SaveBackground();
	virtual ERROR_CODE KillBackground();

	virtual void OnPaint(CBofRect *pRect);
	virtual void OnClose();
	virtual void OnInitDialog();

public:
	/**
	 * Constructor
	 */
	CBofDialog();

	/**
	 * Constructor
	 */
	CBofDialog(const CHAR *pszFileName, CBofRect *pRect = nullptr, CBofWindow *pParent = nullptr, const uint32 nID = 0, const uint32 lFlags = BOFDLG_DEFAULT);

	/**
	 * Constructor
	 */
	CBofDialog(CBofBitmap *pImage, CBofRect *pRect = nullptr, CBofWindow *pParent = nullptr, const uint32 nID = 0, const uint32 lFlags = BOFDLG_DEFAULT);

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
	ERROR_CODE Create(const CHAR *pszName, INT x = 0, INT y = 0, INT nWidth = USE_DEFAULT, INT nHeight = USE_DEFAULT, CBofWindow *pParent = nullptr, uint32 nControlID = 0);

	/**
	 * Creates the dialog
	 * @param pszName		Dialog name
	 * @param pRect			Dialog bounds
	 * @param pParent		Parent window
	 * @param nControlID	Control Id
	 * @return				Error return code
	 */
	ERROR_CODE Create(const CHAR *pszName, CBofRect *pRect = nullptr, CBofWindow *pParent = nullptr, uint32 nControlID = 0);

	/**
	 * Set the dialog flags
	 */
	void SetFlags(uint32 lFlags) {
		_lFlags = lFlags;
	}

	/**
	 * Return the dialog's flags
	 */
	uint32 GetFlags() const {
		return _lFlags;
	}

	/**
	 * Show the dialog as a modal
	 */
	INT DoModal();

	/**
	 * End the dialog modal display
	 */
	void EndModal() {
		_bEndDialog = TRUE;
	}

	/**
	 * Set the dialog's return value
	 */
	void SetReturnValue(INT nValue) {
		_nReturnValue = nValue;
	}

	/**
	 * Get the dialog's return value
	 * @return 
	 */
	INT GetReturnValue() const {
		return _nReturnValue;
	}
#if BOF_MAC
	BOOL        HandleMacEvent(EventRecord *pEvent);
#endif
};

} // namespace Bagel

#endif
