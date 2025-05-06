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

#ifndef HODJNPODJ_HNPLIBS_BUTTON_H
#define HODJNPODJ_HNPLIBS_BUTTON_H

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/dibdoc.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"
#include "bagel/hodjnpodj/hnplibs/button.h"

namespace Bagel {
namespace HodjNPodj {

#define BUTTON_EDGE_WIDTH		3
#define	FOCUS_RECT_DX			2
#define FOCUS_RECT_DY			1
#define SELECTED_TEXT_DX		1
#define SELECTED_TEXT_DY		1
#define CHECK_BOX_SIZE			13
#define CHECK_BOX_DX			4
#define CHECK_TEXT_DX			22
#define RADIO_BOX_SIZE			13
#define RADIO_BOX_DX			4
#define RADIO_TEXT_DX			22

#define	RGB_BUTTON_FACE         PALETTERGB(199,167,139) // PALETTERGB(207,159,115)
#define	RGB_BUTTON_HIGHLIGHT	PALETTERGB(223,199,175)
#define	RGB_BUTTON_SHADOW       PALETTERGB(155,123,95)
#define	RGB_BUTTON_TEXT         PALETTERGB(51,31,19) // PALETTERGB(83,59,1)
#define RGB_BUTTON_TEXT_DISABLE PALETTERGB(155,139,123)
#define	RGB_BUTTON_OUTLINE      PALETTERGB(83,59,51) // PALETTERGB(83,59,1)

#define	RGB_CHECK_FACE         	PALETTERGB(199,167,139) // PALETTERGB(207,159,115)
#define	RGB_CHECK_CONTROL      	PALETTERGB(99,15,7)
#define	RGB_CHECK_TEXT         	PALETTERGB(51,31,19) // PALETTERGB(83,59,1)
#define RGB_CHECK_TEXT_DISABLE 	PALETTERGB(155,139,123)
#define	RGB_CHECK_OUTLINE      	PALETTERGB(83,59,51) // PALETTERGB(207,159,115)

#define	RGB_RADIO_FACE         	PALETTERGB(199,167,139) // PALETTERGB(207,159,115)
#define	RGB_RADIO_CONTROL      	PALETTERGB(99,15,7)
#define	RGB_RADIO_TEXT         	PALETTERGB(51,31,19) // PALETTERGB(83,59,1)
#define RGB_RADIO_TEXT_DISABLE 	PALETTERGB(155,139,123)
#define	RGB_RADIO_OUTLINE      	PALETTERGB(83,59,51) // PALETTERGB(207,159,115)


class CBmpButton : public CBitmapButton {
	DECLARE_DYNCREATE(CBmpButton)

// Constructors
public:
	CBmpButton();	// use "new" operator to create buttons, then LoadButton

// Destructors
public:
	~CBmpButton();

// Implementation
public:
	BOOL SetControl(UINT nID, CWnd * pParent);
	BOOL AutoLoad(UINT nID, CWnd * pParent);

	BOOL LoadBitmaps(CPalette *pPalette, CBitmap *pBase, CBitmap *pSelected, CBitmap *pFocus, CBitmap *pDisabled);
	BOOL LoadBitmaps(LPCSTR lpszBase, LPCSTR lpszSelected = NULL, LPCSTR lpszFocus = NULL, LPCSTR lpszDisabled = NULL);
	BOOL LoadBitmaps(const int nBase, const int nSelected = 0, const int nFocus = 0, const int nDisabled = 0);
	BOOL LoadBmpBitmaps(LPCSTR lpszBase, LPCSTR lpszSelected = NULL, LPCSTR lpszFocus = NULL, LPCSTR lpszDisabled = NULL);

private:
	CPalette	*m_pPalette;
	BOOL		m_bSharedPalette;

private:

virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	//{{AFX_MSG( CBmpButton )
	afx_msg	BOOL OnEraseBkgnd(CDC *pDC);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

};

/////////////////////////////////////////////////////////////////////////////

class CMaskedButton : public CBmpButton
{
	DECLARE_DYNCREATE(CMaskedButton)

// Constructors
public:
	CMaskedButton();	// use "new" operator to create buttons, then LoadButton

// Destructors
public:
	~CMaskedButton();

// Implementation
public:

private:
	CPalette	*m_pPalette;
	CBitmap		*m_pBackground;

private:

virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	//{{AFX_MSG( CMaskedButton )
	afx_msg	BOOL OnEraseBkgnd(CDC *pDC);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

};

/////////////////////////////////////////////////////////////////////////////

class CColorButton : public CButton
{
	DECLARE_DYNCREATE(CColorButton)

// Constructors
public:
	CColorButton();	// use "new" operator to create buttons, then LoadButton

// Destructors
public:
	~CColorButton();

// Implementation
public:
	BOOL SetControl(UINT nID, CWnd * pParent);
	void SetPalette(CPalette *pPalette);
	void SetColors(CPalette *pPalette, COLORREF cFace, COLORREF cHighlight, COLORREF cShadow, COLORREF cText, COLORREF cTextDisabled, COLORREF cOutline);

private:
	CPalette	*m_pPalette;
	COLORREF	m_cButtonFace;
	COLORREF	m_cButtonHighlight;
	COLORREF	m_cButtonShadow;
	COLORREF	m_cButtonText;
	COLORREF	m_cButtonTextDisabled;
	COLORREF	m_cButtonOutline;

private:

virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	//{{AFX_MSG( CColorButton )
	afx_msg	BOOL OnEraseBkgnd(CDC *pDC);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

};

/////////////////////////////////////////////////////////////////////////////

class CCheckButton : public CButton
{
	DECLARE_DYNCREATE(CCheckButton)

// Constructors
public:
	CCheckButton();	// use "new" operator to create buttons, then LoadButton

// Destructors
public:
	~CCheckButton();

// Implementation
public:
	BOOL SetControl(UINT nID, CWnd * pParent);
	void SetPalette(CPalette *pPalette);
	void SetColors(CPalette *pPalette, COLORREF cFace, COLORREF cControl, COLORREF cText, COLORREF cTextDisabled, COLORREF cOutline);

private:
	BOOL		m_bCheckState;
	CPalette	*m_pPalette;
	COLORREF	m_cButtonFace;
	COLORREF	m_cButtonControl;
	COLORREF	m_cButtonText;
	COLORREF	m_cButtonTextDisabled;
	COLORREF	m_cButtonOutline;

private:

virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	//{{AFX_MSG( CCheckButton )
	afx_msg	BOOL OnEraseBkgnd(CDC *pDC);
    afx_msg long OnSetCheck( WPARAM, LPARAM);
    afx_msg long OnGetCheck( WPARAM, LPARAM);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

};

/////////////////////////////////////////////////////////////////////////////

class CRadioButton : public CButton
{
	DECLARE_DYNCREATE(CRadioButton)

// Constructors
public:
	CRadioButton();	// use "new" operator to create buttons, then LoadButton

// Destructors
public:
	~CRadioButton();

// Implementation
public:
	BOOL SetControl(UINT nID, CWnd * pParent);
	void SetPalette(CPalette *pPalette);
	void SetColors(CPalette *pPalette, COLORREF cFace, COLORREF cControl, COLORREF cText, COLORREF cTextDisabled, COLORREF cOutline);

private:
	BOOL		m_bCheckState;
	CPalette	*m_pPalette;
	COLORREF	m_cButtonFace;
	COLORREF	m_cButtonControl;
	COLORREF	m_cButtonText;
	COLORREF	m_cButtonTextDisabled;
	COLORREF	m_cButtonOutline;

private:

virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	//{{AFX_MSG( CRadioButton )
	afx_msg	BOOL OnEraseBkgnd(CDC *pDC);
    afx_msg long OnSetCheck( WPARAM, LPARAM);
    afx_msg long OnGetCheck( WPARAM, LPARAM);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

};

} // namespace HodjNPodj
} // namespace Bagel

#endif
