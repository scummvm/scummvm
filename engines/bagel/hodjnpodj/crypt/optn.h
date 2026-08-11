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

#ifndef HODJNPODJ_CRYPT_OPTN_H
#define HODJNPODJ_CRYPT_OPTN_H

#include "bagel/hodjnpodj/crypt/resource.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/text.h"

namespace Bagel {
namespace HodjNPodj {
namespace Crypt {

#define MIN_LTTRSSLVD       0               // Preferences
#define MAX_LTTRSSLVD       15

#define TIME_TABLE          19
#define MIN_INDEX_TIME      0
#define MAX_INDEX_TIME      18

#define OPTIONS_COLOR       RGB(0, 0, 0)    // Color of the stats info CText
#define FONT_SIZE           16

#define STAT_BOX_COL        20
#define STAT_BOX_ROW        20
#define STAT_BOX_WIDTH      200
#define STAT_BOX_HEIGHT     20

#define BOX_ROW_OFFSET      48

/////////////////////////////////////////////////////////////////////////////
// COptn dialog

class COptn : public CBmpDialog {
private:
	CString     m_chTime[TIME_TABLE];
	int         m_nTimeIndex[TIME_TABLE] = {};
	CText       *m_pLttrsSlvd = nullptr, *m_pTime = nullptr;
	CPalette    *pGamePalette = nullptr;

	int         TimeToIndex(int nTime);

public:
	COptn(CWnd* pParent = nullptr, CPalette *pPalette = nullptr, int nLttrsSolvd = false, int nTime = false); // standard constructor
	~COptn();
	void ClearDialogImage();

	int         m_nLttrsSlvd = 0, m_nTime = 0;
// Dialog Data
	//{{AFX_DATA(COptn)
	enum { IDD = IDD_USER_OPTIONS };
	CScrollBar  m_LttrsSlvd;
	CScrollBar  m_Time;
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(COptn)
	virtual bool OnInitDialog() override;
	afx_msg void OnPaint();
	afx_msg bool OnEraseBkgnd(CDC *pDC);
	afx_msg void OnHScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar* pScrollBar);
	virtual void OnOK() override;
	virtual void OnCancel() override;
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

} // namespace Crypt
} // namespace HodjNPodj
} // namespace Bagel

#endif
