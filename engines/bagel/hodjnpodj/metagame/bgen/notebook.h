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

#ifndef BAGEL_METAGAME_BGEN_NOTEBOOK_H
#define BAGEL_METAGAME_BGEN_NOTEBOOK_H

#include "bagel/hodjnpodj/metagame/bgen/note.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

#define NOTEBOOK_SPEC   ".\\art\\mlscroll.bmp"      // path for notebook's DIB on disk
#define NOTEBOOK_DX             502
#define NOTEBOOK_DY             395
#define NOTEBOOK_CURL_DX        500
#define NOTEBOOK_CURL_DY        50
#define NOTEBOOK_BORDER_DX      60
#define NOTEBOOK_BORDER_DY      60

/*
#define NOTEBOOK_TITLEZONE_DX   75  // 100
#define NOTEBOOK_TITLEZONE_DY   30  // 40
#define NOTEBOOK_TITLEZONE_DDY  -5

#define NOTEBOOK_TEXTZONE_DX    75  // 100
#define NOTEBOOK_TEXTZONE_DY    22  // 30
#define NOTEBOOK_TEXTZONE_DDY   10
*/

#define NOTE_BITMAPS_PER_LINE   6
#define NOTE_BITMAP_DX          59
#define NOTE_BITMAP_DY          59
#define NOTE_BITMAP_DDX         25
#define NOTE_BITMAP_DDY         10
#define NOTE_SMALL_BITMAP_DX    50
#define NOTE_SMALL_BITMAP_DY    50

#define IDD_NOTEBOOK_DIALOG     920
#define IDC_NOTEBOOK_OKAY       920

#define IDC_NOTEBOOK_BOOK       921
#define IDC_NOTEBOOK_SOUND      922

#define NOTE_FONT_SIZE          -12
#define NOTE_TEXT_COLOR         RGB(128,0,128)
#define NOTE_MORE_COLOR         RGB(0,0,0)

#define NOTE_PERSON_DX          NOTEBOOK_BORDER_DX
#define NOTE_PERSON_DY          NOTEBOOK_BORDER_DY - 2
#define NOTE_PLACE_DX           NOTEBOOK_DX - NOTEBOOK_BORDER_DX - NOTE_BITMAP_DX - 5
#define NOTE_PLACE_DY           NOTEBOOK_BORDER_DY - 2
#define NOTE_ICON_DY            NOTEBOOK_DY - NOTEBOOK_BORDER_DY - NOTE_BITMAP_DY + 2
#define NOTE_ICON_DDX           50

#define NOTE_TEXT_DX            NOTEBOOK_BORDER_DX
#define NOTE_TEXT_DY            NOTEBOOK_BORDER_DY + NOTE_BITMAP_DY + NOTE_BITMAP_DDY - 4
#define NOTE_TEXT_DDX           NOTEBOOK_DX - (NOTEBOOK_BORDER_DX << 1)
#define NOTE_TEXT_DDY           NOTEBOOK_DY - (NOTEBOOK_BORDER_DY << 1) - (NOTE_BITMAP_DY << 1) - NOTE_BITMAP_DDY + 4
#define NOTE_TEXT_CHARSPERLINE  30

#define NOTE_BUTTON_DY          15


class CNotebook : public CDialog {
	// Construction
public:
	CNotebook(CWnd *pParent, CPalette *pPalette, CNote *pNoteList, CNote *pNote);

	static  void UpdateNotebook(CDC *pDC);
	static  void UpdateContent(CDC *pDC);
	static  void UpdateNote(CDC *pDC);
	static  void ClearDialogImage();
	static  void RefreshBackground();
	static  void DoWaitCursor();
	static  void DoArrowCursor();

private:
	static  void ShowClue(CNote *pNote);

	bool SetupKeyboardHook();
	void RemoveKeyboardHook();

	bool CreateWorkAreas(CDC *pDC);

private:
	bool m_bKeyboardHook = false;	// whether keyboard hook present
	CRect OkayRect;					// rectangle bounding the OKAY button

	CRect NotebookRect;				// x/y (left/right) and dx/dy (right/bottom) for the notebook window
	CRect PersonRect,				// bounding rectangle for person bitmap
		PlaceRect;					// bounding rectangle for place bitmap
	CRect ScrollTopRect,			// area spanned by upper scroll curl
		ScrollBotRect;				// area spanned by lower scroll curl

	// Dialog Data
		//{{AFX_DATA(CNotebook)
	enum {
		IDD = IDD_NOTEBOOK_DIALOG
	};
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange *pDX) override;    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CNotebook)
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual bool OnInitDialog() override;
	afx_msg bool OnEraseBkgnd(CDC *pDC);
	virtual void OnOK() override;
	virtual void OnCancel() override;
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnShowWindow(bool bShow, unsigned int nStatus);
	afx_msg void OnSize(unsigned int nType, int cx, int cy);
	afx_msg void OnMouseMove(unsigned int nFlags, CPoint point);
	afx_msg bool OnSetCursor(CWnd *pWnd, unsigned int nHitTest, unsigned int message);
	afx_msg void OnLButtonDown(unsigned int nFlags, CPoint point);
	afx_msg void OnActivate(unsigned int nState, CWnd *pWndOther, bool bMinimized) override;
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
