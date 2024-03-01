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

#if BOF_MAC
#define BOFDLG_DEFAULT (BOFDLG_TRANSPARENT | BOFDLG_SAVEBACKGND)
#else
#define BOFDLG_DEFAULT (BOFDLG_TRANSPARENT /* | BOFDLG_SAVEBACKGND*/)
#endif

class CBofDialog : public CBofWindow {

	// Construction
public:
	CBofDialog(VOID);
	CBofDialog(const CHAR *pszFileName, CBofRect *pRect = NULL, CBofWindow *pParent = NULL, const UINT nID = 0, const ULONG lFlags = BOFDLG_DEFAULT);
	CBofDialog(CBofBitmap *pImage, CBofRect *pRect = NULL, CBofWindow *pParent = NULL, const UINT nID = 0, const ULONG lFlags = BOFDLG_DEFAULT);
	virtual ~CBofDialog();

	ERROR_CODE  Create(const CHAR *pszName, INT x = 0, INT y = 0, INT nWidth = USE_DEFAULT, INT nHeight = USE_DEFAULT, CBofWindow *pParent = NULL, UINT nControlID = 0);
	ERROR_CODE  Create(const CHAR *pszName, CBofRect *pRect = NULL, CBofWindow *pParent = NULL, UINT nControlID = 0);

	VOID        SetFlags(ULONG lFlags)      {
		m_lFlags = lFlags;
	}
	ULONG       GetFlags(VOID)              {
		return (m_lFlags);
	}

	INT         DoModal(VOID);
	VOID        EndModal(VOID)              {
		m_bEndDialog = TRUE;
	}

	VOID        SetReturnValue(INT nValue)  {
		m_nReturnValue = nValue;
	}
	INT         GetReturnValue(VOID)        {
		return (m_nReturnValue);
	}
#if BOF_MAC
	BOOL        HandleMacEvent(EventRecord *pEvent);
#endif

protected:
	virtual ERROR_CODE Paint(CBofRect *pRect);
	virtual ERROR_CODE PaintBackground(VOID);
	virtual ERROR_CODE SaveBackground(VOID);
	virtual ERROR_CODE KillBackground(VOID);

	virtual VOID OnPaint(CBofRect *pRect);
	virtual VOID OnClose(VOID);
	virtual VOID OnInitDialog(VOID);

	CBofBitmap *m_pDlgBackground;
	ULONG       m_lFlags;
	INT         m_nReturnValue;

	BOOL        m_bFirstTime;
	BOOL        m_bTempBitmap;
	BOOL        m_bEndDialog;
	BOOL        m_bHavePainted;
};

} // namespace Bagel

#endif
