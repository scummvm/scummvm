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

#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/hnplibs/button.h"

namespace Bagel {
namespace HodjNPodj {

extern  CPalette    *pGamePalette;

IMPLEMENT_DYNCREATE(CBmpButton, CBitmapButton)


/*************************************************************************
 *
 * CBmpButton()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Constructor for button class.  Initialize all fields
 *                  to logical nullptr.  Calls should then be made to other
 *                  button routines to load palettes and bitmaps.
 *
 ************************************************************************/

CBmpButton::CBmpButton() {
	m_pPalette = nullptr;                      // no initial palette
}


/*************************************************************************
 *
 * ~CBmpButton()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Destructor for button class.  It is important to tear
 *                  things down in the order we built them; map out bitmap,
 *                  map out the palette, then discard the device context -
 *                  failure to release key resources will cause application
 *                  or system crashes.
 *
 ************************************************************************/

CBmpButton::~CBmpButton() {
	if (m_pPalette != nullptr) {               // release the palette resource
		(*m_pPalette).DeleteObject();
		delete m_pPalette;
		m_pPalette = nullptr;
	}
}


/*************************************************************************
 *
 * OnEraseBkgnd()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Do not let Windows automatically erase our window
 *                  to white.
 *
 ************************************************************************/

bool CBmpButton::OnEraseBkgnd(CDC *pDC) {
	return true;                                      // do not automatically erase background to white
}


/*************************************************************************
 *
 * DrawItem()
 *
 * Parameters:      pointer to button structure information
 *
 * Return Value:    none
 *
 * Description:     Paint a button appropriately, based on its current state.
 *
 ************************************************************************/

void CBmpButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	CDC         *pDC;
	CPalette    *pPalOld = nullptr;
//bool      bActiveWindow = false;
//CWnd      *pActiveWindow;
	CBitmap     *pBitmap;
	CWnd        *pParentWnd;

	pParentWnd = GetParent();
	if (!IsWindowVisible() ||                       // punt if not visible
	        (pParentWnd == nullptr) ||
	        !(*pParentWnd).IsWindowVisible())
		return;
	/*
	    pActiveWindow = GetActiveWindow();              // get the active window and if it is
	    if ((this == pActiveWindow) ||                  // ... us, then we will use the palette
	        (pActiveWindow == nullptr))
	        bActiveWindow = true;                       // ... in a foreground mode
	*/
	pDC = new CDC;                                  // setup the device context given to us
	(*pDC).Attach((*lpDrawItemStruct).hDC);         // ... as a pointer to a CDC object

	if (m_pPalette != nullptr) {                       // map the palette into the context
		pPalOld = (*pDC).SelectPalette(m_pPalette, true);
		(*pDC).RealizePalette();              // ... and tell the system to use it
	}

	if (((*lpDrawItemStruct).itemState & ODS_GRAYED) || // display the correct bitmap based on state
	        ((*lpDrawItemStruct).itemState & ODS_DISABLED))
		pBitmap = &m_bitmapDisabled;
	else if ((*lpDrawItemStruct).itemState & ODS_SELECTED)
		pBitmap = &m_bitmapSel;
	else if ((*lpDrawItemStruct).itemState & ODS_FOCUS)
		pBitmap = &m_bitmapFocus;
	else
		pBitmap = &m_bitmap;

	PaintBitmap(pDC, nullptr, pBitmap, (*lpDrawItemStruct).rcItem.left, (*lpDrawItemStruct).rcItem.top);

	if (pPalOld != nullptr)
		(*pDC).SelectPalette(pPalOld, false);

	(*pDC).Detach();                          // dismantle the temporary CDC we built
	delete pDC;
}


/*************************************************************************
 *
 * SetControl()
 *
 * Parameters:
 *  unsigned int nID        control identifier for the dialog button
 *  CWnd * pParent  pointer to parent window owning the button
 *
 * Return Value:
 *  bool            success / failure condition
 *
 * Description:     associate this button with a dialog control.
 *
 ************************************************************************/

bool CBmpButton::SetControl(unsigned int nID, CWnd * pParent) {
	if (!CBitmapButton::SubclassDlgItem(nID, pParent))      // attach the Button to the dialog control
		return false;

	return true;
}


/*************************************************************************
 *
 * AutoLoad()
 *
 * Parameters:
 *  unsigned int nID        control identifier for the dialog button
 *  CWnd * pParent  pointer to parent window owning the button
 *
 * Return Value:
 *  bool            success / failure condition
 *
 * Description:     associates this button with a dialog control and
 *                  automatically load bitmaps for a dialog button.
 *
 ************************************************************************/

bool CBmpButton::AutoLoad(unsigned int nID, CWnd * pParent) {
	CString buttonName;
	bool    bSuccess;

	if (!SetControl(nID, pParent))                          // attach the Button to the dialog control
		return false;

	GetWindowText(buttonName);                              // get the button's title
	ASSERT(!buttonName.IsEmpty());
	// load the bitmaps
	bSuccess = LoadBitmaps(buttonName + "U", buttonName + "D", buttonName + "F", buttonName + "X");

	if (bSuccess)
		CBitmapButton::SizeToContent();                     // resize button to its content

	return bSuccess;
}


/*************************************************************************
 *
 * LoadBitmaps()
 *
 * Parameters:
 *  CPalette *pPalatte  palette for buttons
 *  CBitmap *pBase      bitmap for UP state
 *  CBitmap *pSelected  bitmap for SELECTED state
 *  CBitmap *pFocus     bitmap for FOCUS state
 *  CBitmap *pDisabled  bitmap for DISABLED state
 *
 * Return Value:
 *  bool            success / failure condition
 *
 * Description:     Setup the CBitmaps for the four button states of a CBmpButton.
 *
 ************************************************************************/

bool CBmpButton::LoadBitmaps(CPalette *pPalette, CBitmap *pBase, CBitmap *pSelected, CBitmap *pFocus, CBitmap *pDisabled) {
	HBITMAP     hBitmap = nullptr;

	if (pBase == nullptr)
		return false;

	if (pPalette == nullptr)
		m_pPalette = nullptr;
	else {
		if (m_pPalette != nullptr)
			delete m_pPalette;
		m_pPalette = DuplicatePalette(pPalette);
	}

	m_bitmap.DeleteObject();
	hBitmap = (HBITMAP)(*pBase).Detach();
	m_bitmap.Attach(hBitmap);
	delete pBase;

	m_bitmapSel.DeleteObject();
	if (pSelected != nullptr) {
		hBitmap = (HBITMAP)(*pSelected).Detach();
		m_bitmapSel.Attach(hBitmap);
		delete pSelected;
	}

	m_bitmapFocus.DeleteObject();
	if (pFocus != nullptr) {
		hBitmap = (HBITMAP)(*pFocus).Detach();
		m_bitmapFocus.Attach(hBitmap);
		delete pFocus;
	}

	m_bitmapDisabled.DeleteObject();
	if (pDisabled != nullptr) {
		hBitmap = (HBITMAP)(*pDisabled).Detach();
		m_bitmapDisabled.Attach(hBitmap);
		delete pDisabled;
	}

	ensureSize();

	return true;
}


/*************************************************************************
 *
 * LoadBitmaps()
 *
 * Parameters:
 *  const int nBase         resource number for UP state
 *  const int nSelected     resource number for SELECTED state
 *  const int nFocus        resource number for FOCUS state
 *  const int nDisabled     resource number for DISABLED state
 *
 * Return Value:
 *  bool            success / failure condition
 *
 * Description:     Read in the DDB bitmaps for the four button states of a CBmpButton.
 *
 ************************************************************************/

bool CBmpButton::LoadBitmaps(const int nBase, const int nSelected, const int nFocus, const int nDisabled) {
	CDC         *pDC = nullptr;
	CBitmap     *pBitmap = nullptr;
	HBITMAP     hBitmap = nullptr;
	bool        bSuccess = true;

	if (m_pPalette != nullptr) {                       // release any existing palette resource
		(*m_pPalette).DeleteObject();
		delete m_pPalette;
		m_pPalette = nullptr;
	}

	pDC = GetDC();                                  // fetch a device context we can use
	if (pDC == nullptr)
		bSuccess = false;

	if (bSuccess) {                                 // get a CBitmap for the UP button
		pBitmap = FetchResourceBitmap(pDC, &m_pPalette, nBase);
		if (pBitmap != nullptr) {                      // ... and plug it into our instance
			m_bitmap.DeleteObject();                // release what ever was already there
			hBitmap = (HBITMAP)(*pBitmap).Detach();
			m_bitmap.Attach(hBitmap);
			delete pBitmap;
			if (nSelected != 0) {               // get the SELECTED bitmap and plug it in
				pBitmap = FetchResourceBitmap(pDC, nullptr, nSelected);
				if (pBitmap == nullptr)
					bSuccess = false;
				else {
					m_bitmapSel.DeleteObject();     // release what ever was already there
					hBitmap = (HBITMAP)(*pBitmap).Detach();
					m_bitmapSel.Attach(hBitmap);
					delete pBitmap;
				}
			}
			if (bSuccess &&                         // get the FOCUS bitmap and plug it in
			        (nFocus != 0)) {
				pBitmap = FetchResourceBitmap(pDC, nullptr, nFocus);
				if (pBitmap == nullptr)
					bSuccess = false;
				else {
					m_bitmapFocus.DeleteObject();   // release what ever was already there
					hBitmap = (HBITMAP)(*pBitmap).Detach();
					m_bitmapFocus.Attach(hBitmap);
					delete pBitmap;
				}
			}
			if (bSuccess &&                         // get the DISABLED bitmap and plug it in
			        (nDisabled != 0)) {
				pBitmap = FetchResourceBitmap(pDC, nullptr, nDisabled);
				if (pBitmap == nullptr)
					bSuccess = false;
				else {
					m_bitmapDisabled.DeleteObject();// release what ever was already there
					hBitmap = (HBITMAP)(*pBitmap).Detach();
					m_bitmapDisabled.Attach(hBitmap);
					delete pBitmap;
				}
			}
		} else
			bSuccess = false;
	}

	if (pDC != nullptr)
		ReleaseDC(pDC);

	if (bSuccess)
		ensureSize();

	return bSuccess;
}


/*************************************************************************
 *
 * LoadBitmaps()
 *
 * Parameters:
 *  const char *lpszBase         pointer to resource name string for UP state
 *  const char *lpszSelected     pointer to resource name string for SELECTED state
 *  const char *lpszFocus        pointer to resource name string for FOCUS state
 *  const char *lpszDisabled     pointer to resource name string for DISABLED state
 *
 * Return Value:
 *  bool            success / failure condition
 *
 * Description:     Read in the DDB bitmaps for the four button states of a CBmpButton.
 *
 ************************************************************************/

bool CBmpButton::LoadBitmaps(const char *lpszBase, const char *lpszSelected, const char *lpszFocus, const char *lpszDisabled) {
	CDC         *pDC = nullptr;
	CBitmap     *pBitmap = nullptr;
	HBITMAP     hBitmap = nullptr;
	bool        bSuccess = true;

	if (m_pPalette != nullptr) {                       // release any existing palette resource
		(*m_pPalette).DeleteObject();
		delete m_pPalette;
		m_pPalette = nullptr;
	}

	pDC = GetDC();                                  // fetch a device context we can use
	if (pDC == nullptr)
		bSuccess = false;

	if (bSuccess) {                                 // get a CBitmap for the UP button
		pBitmap = FetchResourceBitmap(pDC, &m_pPalette, lpszBase);
		if (pBitmap != nullptr) {                      // ... and plug it into our instance
			m_bitmap.DeleteObject();                // release what ever was already there
			hBitmap = (HBITMAP)(*pBitmap).Detach();
			m_bitmap.Attach(hBitmap);
			delete pBitmap;
			if (lpszSelected != nullptr) {             // get the SELECTED bitmap and plug it in
				pBitmap = FetchResourceBitmap(pDC, nullptr, lpszSelected);
				if (pBitmap == nullptr)
					bSuccess = false;
				else {
					m_bitmapSel.DeleteObject();     // release what ever was already there
					hBitmap = (HBITMAP)(*pBitmap).Detach();
					m_bitmapSel.Attach(hBitmap);
					delete pBitmap;
				}
			}
			if (bSuccess &&                         // get the FOCUS bitmap and plug it in
			        (lpszFocus != nullptr)) {
				pBitmap = FetchResourceBitmap(pDC, nullptr, lpszFocus);
				if (pBitmap == nullptr)
					bSuccess = false;
				else {
					m_bitmapFocus.DeleteObject();   // release what ever was already there
					hBitmap = (HBITMAP)(*pBitmap).Detach();
					m_bitmapFocus.Attach(hBitmap);
					delete pBitmap;
				}
			}
			if (bSuccess &&                         // get the DISABLED bitmap and plug it in
			        (lpszDisabled != nullptr)) {
				pBitmap = FetchResourceBitmap(pDC, nullptr, lpszDisabled);
				if (pBitmap == nullptr)
					bSuccess = false;
				else {
					m_bitmapDisabled.DeleteObject();// release what ever was already there
					hBitmap = (HBITMAP)(*pBitmap).Detach();
					m_bitmapDisabled.Attach(hBitmap);
					delete pBitmap;
				}
			}
		} else
			bSuccess = false;
	}

	if (pDC != nullptr)
		ReleaseDC(pDC);

	if (bSuccess)
		ensureSize();

	return bSuccess;
}


/*************************************************************************
 *
 * LoadBmpBitmaps()
 *
 * Parameters:
 *  const char *lpszBase         pointer to .BMP path string for UP state
 *  const char *lpszSelected     pointer to .BMP path string for SELECTED state
 *  const char *lpszFocus        pointer to .BMP path string for FOCUS state
 *  const char *lpszDisabled     pointer to .BMP path string for DISABLED state
 *
 * Return Value:
 *  bool            success / failure condition
 *
 * Description:     Read in the .BMP files for the four button states of a CBmpButton.
 *
 ************************************************************************/

bool CBmpButton::LoadBmpBitmaps(const char *lpszBase, const char *lpszSelected, const char *lpszFocus, const char *lpszDisabled) {
	CDC         *pDC = nullptr;
	CBitmap     *pBitmap = nullptr;
	HBITMAP     hBitmap = nullptr;
	bool        bSuccess = true;

	if (m_pPalette != nullptr) {                       // release any existing palette resource
		(*m_pPalette).DeleteObject();
		delete m_pPalette;
		m_pPalette = nullptr;
	}

	pDC = GetDC();                                  // fetch a device context we can use
	if (pDC == nullptr)
		bSuccess = false;

	if (bSuccess) {                                 // get a CBitmap for the UP button
		pBitmap = FetchBitmap(pDC, &m_pPalette, lpszBase);
		if (pBitmap != nullptr) {                      // ... and plug it into our instance
			m_bitmap.DeleteObject();                // release what ever was already there
			hBitmap = (HBITMAP)(*pBitmap).Detach();
			m_bitmap.Attach(hBitmap);
			delete pBitmap;
			if (lpszSelected != nullptr) {             // get the SELECTED bitmap and plug it in
				pBitmap = FetchBitmap(pDC, nullptr, lpszSelected);
				if (pBitmap == nullptr)
					bSuccess = false;
				else {
					m_bitmapSel.DeleteObject();     // release what ever was already there
					hBitmap = (HBITMAP)(*pBitmap).Detach();
					m_bitmapSel.Attach(hBitmap);
					delete pBitmap;
				}
			}
			if (bSuccess &&                         // get the FOCUS bitmap and plug it in
			        (lpszFocus != nullptr)) {
				pBitmap = FetchBitmap(pDC, nullptr, lpszFocus);
				if (pBitmap == nullptr)
					bSuccess = false;
				else {
					m_bitmapFocus.DeleteObject();   // release what ever was already there
					hBitmap = (HBITMAP)(*pBitmap).Detach();
					m_bitmapFocus.Attach(hBitmap);
					delete pBitmap;
				}
			}
			if (bSuccess &&                         // get the DISABLED bitmap and plug it in
			        (lpszDisabled != nullptr)) {
				pBitmap = FetchBitmap(pDC, nullptr, lpszDisabled);
				if (pBitmap == nullptr)
					bSuccess = false;
				else {
					m_bitmapDisabled.DeleteObject();// release what ever was already there
					hBitmap = (HBITMAP)(*pBitmap).Detach();
					m_bitmapDisabled.Attach(hBitmap);
					delete pBitmap;
				}
			}
		} else
			bSuccess = false;
	}

	if (pDC != nullptr)
		ReleaseDC(pDC);

	if (bSuccess)
		ensureSize();

	return bSuccess;
}

void CBmpButton::ensureSize() {
	CRect clientRect;
	GetClientRect(&clientRect);
	BITMAP bmpInfo;
	m_bitmap.GetBitmap(&bmpInfo);

	if (clientRect.right < bmpInfo.bmWidth ||
		clientRect.bottom < bmpInfo.bmHeight) {
		clientRect.right = MAX(clientRect.right, bmpInfo.bmWidth);
		clientRect.bottom = MAX(clientRect.bottom, bmpInfo.bmHeight);
		ClientToScreen(&clientRect);
		m_pParentWnd->ScreenToClient(&clientRect);
		MoveWindow(&clientRect);
	}

	// Flag for the control to redraw itself
	Invalidate();
}

// CBmpButton message map:
// Associate messages with member functions.
//
BEGIN_MESSAGE_MAP(CBmpButton, CBitmapButton)
	//{{AFX_MSG_MAP( CBmpButton )
	ON_WM_ERASEBKGND()
	ON_WM_DRAWITEM()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CMaskedButton, CBmpButton)


/*************************************************************************
 *
 * CMaskedButton()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Constructor for button class.  Initialize all fields
 *                  to logical nullptr.  Calls should then be made to other
 *                  button routines to load palettes and bitmaps.
 *
 ************************************************************************/

CMaskedButton::CMaskedButton() {
	m_pPalette = nullptr;                      // no initial palette
	m_pBackground = nullptr;                   // no initial background
}


/*************************************************************************
 *
 * ~CMaskedButton()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Destructor for button class.  It is important to tear
 *                  things down in the order we built them; map out bitmap,
 *                  map out the palette, then discard the device context -
 *                  failure to release key resources will cause application
 *                  or system crashes.
 *
 ************************************************************************/

CMaskedButton::~CMaskedButton() {
	if (m_pPalette != nullptr) {               // release the palette resource
		(*m_pPalette).DeleteObject();
		delete m_pPalette;
		m_pPalette = nullptr;
	}

	if (m_pBackground != nullptr) {            // release the background resource
		delete m_pBackground;
		m_pBackground = nullptr;
	}
}


/*************************************************************************
 *
 * OnEraseBkgnd()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Do not let Windows automatically erase our window
 *                  to white.
 *
 ************************************************************************/

bool CMaskedButton::OnEraseBkgnd(CDC *pDC) {
	return true;                                      // do not automatically erase background to white
}


/*************************************************************************
 *
 * DrawItem()
 *
 * Parameters:      pointer to button structure information
 *
 * Return Value:    none
 *
 * Description:     Paint a button appropriately, based on its current state.
 *
 ************************************************************************/

void CMaskedButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	CDC         *pDC;
	CPalette    *pPalOld = nullptr;
	CBitmap     *pBitmap;
	CWnd        *pParentWnd;

	pParentWnd = GetParent();
	if (!IsWindowVisible() ||                       // punt if not visible
	        (pParentWnd == nullptr) ||
	        !(*pParentWnd).IsWindowVisible())
		return;

	pDC = new CDC;                                  // setup the device context given to us
	(*pDC).Attach((*lpDrawItemStruct).hDC);         // ... as a pointer to a CDC object

	if (m_pPalette != nullptr) {                       // map the palette into the context
		pPalOld = (*pDC).SelectPalette(m_pPalette, true);
		(*pDC).RealizePalette();              // ... and tell the system to use it
	}

	if (m_pBackground == nullptr)
		m_pBackground = FetchScreenBitmap(pDC, m_pPalette,
		                                  (*lpDrawItemStruct).rcItem.left,
		                                  (*lpDrawItemStruct).rcItem.top,
		                                  (*lpDrawItemStruct).rcItem.right - (*lpDrawItemStruct).rcItem.left,
		                                  (*lpDrawItemStruct).rcItem.bottom - (*lpDrawItemStruct).rcItem.top);
	else
		PaintBitmap(pDC, m_pPalette, m_pBackground,
		            (*lpDrawItemStruct).rcItem.left,
		            (*lpDrawItemStruct).rcItem.top);

	if (((*lpDrawItemStruct).itemState & ODS_GRAYED) || // display the correct bitmap based on state
	        ((*lpDrawItemStruct).itemState & ODS_DISABLED))
		pBitmap = &m_bitmapDisabled;
	else if ((*lpDrawItemStruct).itemState & ODS_SELECTED)
		pBitmap = &m_bitmapSel;
	else if ((*lpDrawItemStruct).itemState & ODS_FOCUS)
		pBitmap = &m_bitmapFocus;
	else
		pBitmap = &m_bitmap;

	PaintMaskedBitmap(pDC, nullptr, pBitmap, (*lpDrawItemStruct).rcItem.left, (*lpDrawItemStruct).rcItem.top);

	if (pPalOld != nullptr)
		(*pDC).SelectPalette(pPalOld, false);

	(*pDC).Detach();                          // dismantle the temporary CDC we built
	delete pDC;
}


// CMaskedButton message map:
// Associate messages with member functions.
//
BEGIN_MESSAGE_MAP(CMaskedButton, CBmpButton)
	//{{AFX_MSG_MAP( CMaskedButton )
	ON_WM_ERASEBKGND()
	ON_WM_DRAWITEM()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////



IMPLEMENT_DYNCREATE(CColorButton, CButton)


/*************************************************************************
 *
 * CColorButton()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Constructor for button class.  Initialize all fields
 *                  to logical nullptr.  Calls should then be made to other
 *                  button routines to load palettes and bitmaps.
 *
 ************************************************************************/

CColorButton::CColorButton() {
	m_pPalette = nullptr;              // no initial palette
	m_cButtonFace = RGB_BUTTON_FACE;
	m_cButtonHighlight = RGB_BUTTON_HIGHLIGHT;
	m_cButtonShadow = RGB_BUTTON_SHADOW;
	m_cButtonText = RGB_BUTTON_TEXT;
	m_cButtonTextDisabled = RGB_BUTTON_TEXT_DISABLE;
	m_cButtonOutline = RGB_BUTTON_OUTLINE;
}


/*************************************************************************
 *
 * ~CColorButton()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Destructor for button class.  It is important to tear
 *                  things down in the order we built them; map out bitmap,
 *                  map out the palette, then discard the device context -
 *                  failure to release key resources will cause application
 *                  or system crashes.
 *
 ************************************************************************/

CColorButton::~CColorButton() {
	m_pPalette = nullptr;
}


/*************************************************************************
 *
 * OnEraseBkgnd()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Do not let Windows automatically erase our window
 *                  to white.
 *
 ************************************************************************/

bool CColorButton::OnEraseBkgnd(CDC *pDC) {
	return true;                                      // do not automatically erase background to white
}


/*************************************************************************
 *
 * SetPalette()
 *
 * Parameters:
 *  CPalette *      pointer to palette to be used when painting the button
 *
 * Return Value:    none
 *
 * Description:     registers the palette to be used when painting the button;
 *                  it is assumed to be a shared resource and hence is NOT
 *                  deleted when the button is destroyed.
 *
 ************************************************************************/

void CColorButton::SetPalette(CPalette *pPalette) {
	m_pPalette = pPalette;                          // save the palette pointer
}


/*************************************************************************
 *
 * SetColors()
 *
 * Parameters:
 *  CPalette *              pointer to palette to be used when painting the button
 *  COLORREF cFace          PALETTERGB color value for the face of the button
 *  COLORREF cHighlight     PALETTERGB color value for the upper highlighted edge
 *  COLORREF cShadow        PALETTERGB color value for the lower shadowed edge
 *  COLORREF cText          PALETTERGB color value for the text in the button
 *  COLORREF cTextDisabled  PALETTERGB color value for the text when disabled
 *  COLORREF cOutline       PALETTERGB color value for the bounding rectangle
 *
 * Return Value:    none
 *
 * Description:     establishes the colors to use when painting the button;
 *                  note that cHighlight and CShadow are reversed when the
 *                  button is selected (i.e. down).
 *
 ************************************************************************/

void CColorButton::SetColors(CPalette *pPalette, COLORREF cFace, COLORREF cHighlight, COLORREF cShadow, COLORREF cText, COLORREF cTextDisabled, COLORREF cOutline) {
	m_pPalette = pPalette;
	m_cButtonFace = cFace;
	m_cButtonHighlight = cHighlight;
	m_cButtonShadow = cShadow;
	m_cButtonText = cText;
	m_cButtonTextDisabled = cTextDisabled;
	m_cButtonOutline = cOutline;
}


/*************************************************************************
 *
 * SetControl()
 *
 * Parameters:
 *  unsigned int nID        control identifier for the dialog button
 *  CWnd * pParent  pointer to parent window owning the button
 *
 * Return Value:
 *  bool            success / failure condition
 *
 * Description:     associate this button with a dialog control.
 *
 ************************************************************************/

bool CColorButton::SetControl(unsigned int nID, CWnd * pParent) {
	if (!CColorButton::SubclassDlgItem(nID, pParent))      // attach the Button to the dialog control
		return false;

	return true;
}


/*************************************************************************
 *
 * DrawItem()
 *
 * Parameters:      pointer to button structure information
 *
 * Return Value:    none
 *
 * Description:     Paint a button appropriately, based on its current state.
 *
 ************************************************************************/

void CColorButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	CDC         *pDC;
	CPalette    *pPalOld = nullptr;
	CPen        myPen, myInversePen, *pOldPen;
	CBrush      myBrush, *pOldBrush, myQuill, myFrame;
	COLORREF    myTextColor, oldTextColor;
	int         nUnderscore = -1;
	int         nMyTextLength,
	            i, j, x, y, dx, dy;
	char        chMyText[128];
	TEXTMETRIC  fontMetrics;
	CSize       textInfo,
	            letterInfo,
	            underscoreInfo;
	CRect       focusRect;
	CWnd        *pParentWnd;

	pParentWnd = GetParent();
	if (!IsWindowVisible() ||                       // punt if not visible
	        (pParentWnd == nullptr) ||
	        !(*pParentWnd).IsWindowVisible())
		return;

	nMyTextLength = GetWindowText((char *) &chMyText, 127);
	chMyText[nMyTextLength] = '\0';                 // fetch the button label and strip
	for (i = 0, j = 0; i <= nMyTextLength; i++) {   // ... out any apersand characters
		if (chMyText[i] == '&')                     // ... which are used to force underscoring
			nUnderscore = j;                        // ... of the next letter, which in turn
		else {                                      // ... indicates the keyboard key equivalent
			chMyText[j] = chMyText[i];
			j += 1;
		}
	}
	nMyTextLength = j - 1;
	x = 0;
	y = 0;

	pDC = new CDC;                                  // setup the device context given to us
	(*pDC).Attach((*lpDrawItemStruct).hDC);         // ... as a pointer to a CDC object

	if (m_pPalette != nullptr) {                       // map the palette into the context
		pPalOld = (*pDC).SelectPalette(m_pPalette, true);
		(*pDC).RealizePalette();              // ... and tell the system to use it
	}

	if (((*lpDrawItemStruct).itemState & ODS_GRAYED) ||
	        ((*lpDrawItemStruct).itemState & ODS_DISABLED)) {
		myPen.CreatePen(PS_INSIDEFRAME, BUTTON_EDGE_WIDTH, m_cButtonHighlight);
		myInversePen.CreatePen(PS_INSIDEFRAME, 1, m_cButtonShadow);
		myBrush.CreateSolidBrush(m_cButtonFace);
		myTextColor = m_cButtonTextDisabled;
	} else if ((*lpDrawItemStruct).itemState & ODS_SELECTED) {
		myPen.CreatePen(PS_INSIDEFRAME, BUTTON_EDGE_WIDTH, m_cButtonShadow);
		myInversePen.CreatePen(PS_SOLID, 1, m_cButtonHighlight);
		myBrush.CreateSolidBrush(m_cButtonFace);
		myTextColor = m_cButtonText;
		x = SELECTED_TEXT_DX;
		y = SELECTED_TEXT_DY;
	} else {
		myPen.CreatePen(PS_INSIDEFRAME, BUTTON_EDGE_WIDTH, m_cButtonHighlight);
		myInversePen.CreatePen(PS_SOLID, 1, m_cButtonShadow);
		myBrush.CreateSolidBrush(m_cButtonFace);
		myTextColor = m_cButtonText;
	}

	pOldPen = (*pDC).SelectObject(&myPen);                  // select in the pens and brushes
	pOldBrush = (*pDC).SelectObject(&myBrush);
	(*pDC).Rectangle(&(*lpDrawItemStruct).rcItem);
	(*pDC).SelectObject(pOldPen);
	pOldPen = (*pDC).SelectObject(&myInversePen);

	for (i = 1; i <= BUTTON_EDGE_WIDTH; i++) {              // draw the button edges
		(*pDC).MoveTo((*lpDrawItemStruct).rcItem.left + i, (*lpDrawItemStruct).rcItem.bottom - i);
		(*pDC).LineTo((*lpDrawItemStruct).rcItem.right - i, (*lpDrawItemStruct).rcItem.bottom - i);
		(*pDC).LineTo((*lpDrawItemStruct).rcItem.right - i, (*lpDrawItemStruct).rcItem.top + i - 1);
	}

	(*pDC).SelectObject(pOldPen);
	(*pDC).SelectObject(pOldBrush);

	myFrame.CreateSolidBrush(m_cButtonOutline);             // outline the button
	(*pDC).FrameRect(&(*lpDrawItemStruct).rcItem, &myFrame);

	(*pDC).GetTextMetrics(&fontMetrics);                    // get some info about the font
	textInfo = (*pDC).GetTextExtent(chMyText, nMyTextLength); // get the area spanned by the text
	dx = (*lpDrawItemStruct).rcItem.right - (*lpDrawItemStruct).rcItem.left;
	dy = (*lpDrawItemStruct).rcItem.bottom - (*lpDrawItemStruct).rcItem.top;
	x += (dx - textInfo.cx) >> 1;
	y += (dy - textInfo.cy) >> 1;

	(*pDC).SetBkMode(TRANSPARENT);                          // make the text overlay transparently
	oldTextColor = (*pDC).SetTextColor(myTextColor);         // set the color of the text

	(*pDC).TextOut(x, y, (const char *) &chMyText, nMyTextLength);

	if (nUnderscore >= 0) {
		underscoreInfo = (*pDC).GetTextExtent(chMyText, nUnderscore);
		dx = x + underscoreInfo.cx;
		letterInfo = (*pDC).GetTextExtent(&chMyText[nUnderscore], 1);
		underscoreInfo = (*pDC).GetTextExtent((const char *) "_", 1);
		dx += (letterInfo.cx - underscoreInfo.cx) >> 1;
		(*pDC).TextOut(dx, y, (const char *) "_", 1);
	}

	(*pDC).SetTextColor(oldTextColor);                // set the color of the text

	if ((*lpDrawItemStruct).itemState & ODS_FOCUS) {
		focusRect.SetRect(x - FOCUS_RECT_DX,
		                  y - FOCUS_RECT_DY,
		                  x + textInfo.cx + FOCUS_RECT_DX,
		                  y + textInfo.cy + FOCUS_RECT_DY + 1);
		myQuill.CreateStockObject(DKGRAY_BRUSH);
		(*pDC).FrameRect(&focusRect, &myQuill);
	}

	if (pPalOld != nullptr)
		(*pDC).SelectPalette(pPalOld, false);

	(*pDC).Detach();                          // dismantle the temporary CDC we built
	delete pDC;
}


// CColorButton message map:
// Associate messages with member functions.
//
BEGIN_MESSAGE_MAP(CColorButton, CButton)
	//{{AFX_MSG_MAP( CColorButton )
	ON_WM_ERASEBKGND()
	ON_WM_DRAWITEM()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////


IMPLEMENT_DYNCREATE(CCheckButton, CButton)


/*************************************************************************
 *
 * CCheckButton()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Constructor for button class.  Initialize all fields
 *                  to logical nullptr.  Calls should then be made to other
 *                  button routines to load palettes and bitmaps.
 *
 ************************************************************************/

CCheckButton::CCheckButton() {
	m_bCheckState = false;
	m_pPalette = nullptr;
	m_cButtonFace = RGB_CHECK_FACE;
	m_cButtonControl = RGB_CHECK_CONTROL;
	m_cButtonText = RGB_CHECK_TEXT;
	m_cButtonTextDisabled = RGB_CHECK_TEXT_DISABLE;
	m_cButtonOutline = RGB_CHECK_OUTLINE;
}


/*************************************************************************
 *
 * ~CCheckButton()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Destructor for button class.  It is important to tear
 *                  things down in the order we built them; map out bitmap,
 *                  map out the palette, then discard the device context -
 *                  failure to release key resources will cause application
 *                  or system crashes.
 *
 ************************************************************************/

CCheckButton::~CCheckButton() {
	m_pPalette = nullptr;
}


/*************************************************************************
 *
 * OnEraseBkgnd()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Do not let Windows automatically erase our window
 *                  to white.
 *
 ************************************************************************/

bool CCheckButton::OnEraseBkgnd(CDC *pDC) {
	return true;                                      // do not automatically erase background to white
}


/*************************************************************************
 *
 * SetPalette()
 *
 * Parameters:
 *  CPalette *      pointer to palette to be used when painting the button
 *
 * Return Value:    none
 *
 * Description:     registers the palette to be used when painting the button;
 *                  it is assumed to be a shared resource and hence is NOT
 *                  deleted when the button is destroyed.
 *
 ************************************************************************/

void CCheckButton::SetPalette(CPalette *pPalette) {
	m_pPalette = pPalette;                          // save the palette pointer
}


/*************************************************************************
 *
 * SetColors()
 *
 * Parameters:
 *  CPalette *              pointer to palette to be used when painting the button
 *  COLORREF cFace          PALETTERGB color value for the face of the button
 *  COLORREF cControl       PALETTERGB color value for the control box
 *  COLORREF cText          PALETTERGB color value for the text in the button
 *  COLORREF cTextDisabled  PALETTERGB color value for the text when disabled
 *  COLORREF cOutline       PALETTERGB color value for the bounding rectangle
 *
 * Return Value:    none
 *
 * Description:     establishes the colors to use when painting the button;
 *                  note that cHighlight and CShadow are reversed when the
 *                  button is selected (i.e. down).
 *
 ************************************************************************/

void CCheckButton::SetColors(CPalette *pPalette, COLORREF cFace, COLORREF cControl, COLORREF cText, COLORREF cTextDisabled, COLORREF cOutline) {
	m_pPalette = pPalette;
	m_cButtonFace = cFace;
	m_cButtonControl = cControl;
	m_cButtonText = cText;
	m_cButtonTextDisabled = cTextDisabled;
	m_cButtonOutline = cOutline;
}


/*************************************************************************
 *
 * SetControl()
 *
 * Parameters:
 *  unsigned int nID        control identifier for the dialog button
 *  CWnd * pParent  pointer to parent window owning the button
 *
 * Return Value:
 *  bool            success / failure condition
 *
 * Description:     associate this button with a dialog control.
 *
 ************************************************************************/

bool CCheckButton::SetControl(unsigned int nID, CWnd * pParent) {
	if (!CCheckButton::SubclassDlgItem(nID, pParent))      // attach the Button to the dialog control
		return false;

	return true;
}


/*************************************************************************
 *
 * DrawItem()
 *
 * Parameters:      pointer to button structure information
 *
 * Return Value:    none
 *
 * Description:     Paint a button appropriately, based on its current state.
 *
 ************************************************************************/

void CCheckButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	CDC         *pDC;
	CPalette    *pPalOld = nullptr;
	CPen        outlinePen, controlPen, *pOldPen;
	CBrush      faceBrush, *pOldBrush, myQuill, frameBrush;
	COLORREF    myTextColor, oldTextColor;
	int         nUnderscore = -1;
	int         nMyTextLength,
	            i, j, x, y, dx, dy;
	char        chMyText[128];
	TEXTMETRIC  fontMetrics;
	CSize       textInfo,
	            letterInfo,
	            underscoreInfo;
	CRect       controlRect, focusRect;
	CWnd        *pParentWnd;

	pParentWnd = GetParent();
	if (!IsWindowVisible() ||                       // punt if not visible
	        (pParentWnd == nullptr) ||
	        !(*pParentWnd).IsWindowVisible())
		return;

	nMyTextLength = GetWindowText((char *) &chMyText, 127);
	chMyText[nMyTextLength] = '\0';                 // fetch the button label and strip
	for (i = 0, j = 0; i <= nMyTextLength; i++) {   // ... out any apersand characters
		if (chMyText[i] == '&')                     // ... which are used to force underscoring
			nUnderscore = j;                        // ... of the next letter, which in turn
		else {                                      // ... indicates the keyboard key equivalent
			chMyText[j] = chMyText[i];
			j += 1;
		}
	}
	nMyTextLength = j - 1;

	pDC = new CDC;                                  // setup the device context given to us
	(*pDC).Attach((*lpDrawItemStruct).hDC);         // ... as a pointer to a CDC object

	if (m_pPalette != nullptr) {                       // map the palette into the context
		pPalOld = (*pDC).SelectPalette(m_pPalette, true);
		(*pDC).RealizePalette();              // ... and tell the system to use it
	}

	if (((*lpDrawItemStruct).itemState & ODS_GRAYED) || // setup text color based on enable/disable state
	        ((*lpDrawItemStruct).itemState & ODS_DISABLED))
		myTextColor = m_cButtonTextDisabled;
	else
		myTextColor = m_cButtonText;
	// calculate the button width and height
	dx = (*lpDrawItemStruct).rcItem.right - (*lpDrawItemStruct).rcItem.left;
	dy = (*lpDrawItemStruct).rcItem.bottom - (*lpDrawItemStruct).rcItem.top;
	// create the bounding rectangle for the box
	controlRect.SetRect((*lpDrawItemStruct).rcItem.left + CHECK_BOX_DX,
	                    (*lpDrawItemStruct).rcItem.top + ((dy - CHECK_BOX_SIZE) >> 1),
	                    (*lpDrawItemStruct).rcItem.left + CHECK_BOX_DX + CHECK_BOX_SIZE,
	                    (*lpDrawItemStruct).rcItem.top + ((dy - CHECK_BOX_SIZE) >> 1) + CHECK_BOX_SIZE);
	// create the pens and brushes we need
	outlinePen.CreatePen(PS_INSIDEFRAME, 1, m_cButtonOutline);
	faceBrush.CreateSolidBrush(m_cButtonFace);
	controlPen.CreatePen(PS_INSIDEFRAME, 1, m_cButtonControl);
	frameBrush.CreateSolidBrush(m_cButtonControl);

	pOldBrush = (*pDC).SelectObject(&faceBrush);
	if ((*lpDrawItemStruct).itemAction & ODA_DRAWENTIRE) {
		pOldPen = (*pDC).SelectObject(&outlinePen); // fill in the whole control
		(*pDC).Rectangle(&(*lpDrawItemStruct).rcItem);
	} else {
		pOldPen = (*pDC).SelectObject(&controlPen); // fill in just the box
		(*pDC).Rectangle(&controlRect);
	}
	(*pDC).SelectObject(pOldPen);
	(*pDC).SelectObject(pOldBrush);

	(*pDC).FrameRect(&controlRect, &frameBrush);    // paint the box and indicate selection

	if (m_bCheckState) {                            // indicate that the box is checked
		pOldPen = (*pDC).SelectObject(&controlPen);
		(*pDC).MoveTo(controlRect.left, controlRect.top);
		(*pDC).LineTo(controlRect.right - 1, controlRect.bottom - 1);
		(*pDC).MoveTo(controlRect.left, controlRect.bottom - 1);
		(*pDC).LineTo(controlRect.right - 1, controlRect.top);
		(*pDC).SelectObject(pOldPen);
	}

	if ((*lpDrawItemStruct).itemState & ODS_SELECTED) {
		controlRect.InflateRect(-1, -1);
		(*pDC).FrameRect(&controlRect, &frameBrush);
	}

	(*pDC).GetTextMetrics(&fontMetrics);                    // get some info about the font
	textInfo = (*pDC).GetTextExtent(chMyText, nMyTextLength); // get the area spanned by the text
	x = (*lpDrawItemStruct).rcItem.left + CHECK_TEXT_DX;
	y = (*lpDrawItemStruct).rcItem.top + ((dy - textInfo.cy) >> 1);

	(*pDC).SetBkMode(TRANSPARENT);                      // make the text overlay transparently
	oldTextColor = (*pDC).SetTextColor(myTextColor);    // set the color of the text

	(*pDC).TextOut(x, y, (const char *) &chMyText, nMyTextLength);

	if (nUnderscore >= 0) {                         // put the underscore where it belongs
		underscoreInfo = (*pDC).GetTextExtent(chMyText, nUnderscore);
		dx = x + underscoreInfo.cx;
		letterInfo = (*pDC).GetTextExtent(&chMyText[nUnderscore], 1);
		underscoreInfo = (*pDC).GetTextExtent((const char *) "_", 1);
		dx += (letterInfo.cx - underscoreInfo.cx) >> 1;
		(*pDC).TextOut(dx, y, (const char *) "_", 1);
	}

	(*pDC).SetTextColor(oldTextColor);

	focusRect.SetRect(x - FOCUS_RECT_DX,           // set the focus rectangle
	                  y - FOCUS_RECT_DY,             // ... then paint it or clear it
	                  x + textInfo.cx + FOCUS_RECT_DX,
	                  y + textInfo.cy + FOCUS_RECT_DY + 1);
	if ((*lpDrawItemStruct).itemState & ODS_FOCUS) {
		myQuill.CreateStockObject(DKGRAY_BRUSH);
		(*pDC).FrameRect(&focusRect, &myQuill);
	} else
		(*pDC).FrameRect(&focusRect, &faceBrush);

	if (pPalOld != nullptr)
		(*pDC).SelectPalette(pPalOld, false);

	(*pDC).Detach();                          // dismantle the temporary CDC we built
	delete pDC;
}


/*************************************************************************
 *
 * OnSetCheck()
 *
 * Parameters:
 *  WPARAM wParam   true / false for new check state
 *  LPARAM lParam   0
 *
 * Return Value:    0
 *
 * Description:     set the new state of the check box.
 *
 ************************************************************************/

LRESULT CCheckButton::OnSetCheck(WPARAM wParam, LPARAM lParam) {
	CRect   controlRect;

	m_bCheckState = wParam;                         // set the new state value

	GetWindowRect(&controlRect);                    // force a repaint of the check box
	controlRect.left = CHECK_BOX_DX;
	controlRect.top = ((controlRect.bottom - controlRect.top) - CHECK_BOX_SIZE) >> 1;
	controlRect.right = CHECK_BOX_DX + CHECK_BOX_SIZE;
	controlRect.bottom = controlRect.top + CHECK_BOX_SIZE;
	InvalidateRect(&controlRect, false);
	UpdateWindow();

	return 0;
}


/*************************************************************************
 *
 * OnGetCheck()
 *
 * Parameters:
 *  WPARAM wParam   0
 *  LPARAM lParam   0
 *
 * Return Value:    true / false for current check state
 *
 * Description:     get the current state of the check box.
 *
 ************************************************************************/

LRESULT CCheckButton::OnGetCheck(WPARAM wParam, LPARAM lParam) {
	return m_bCheckState;                         // return the current state value
}


// CCheckButton message map:
// Associate messages with member functions.
//
BEGIN_MESSAGE_MAP(CCheckButton, CButton)
	//{{AFX_MSG_MAP( CCheckButton )
	ON_WM_ERASEBKGND()
	ON_WM_DRAWITEM()
	ON_MESSAGE(BM_SETCHECK, CCheckButton::OnSetCheck)
	ON_MESSAGE(BM_GETCHECK, CCheckButton::OnGetCheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////


IMPLEMENT_DYNCREATE(CRadioButton, CButton)


/*************************************************************************
 *
 * CRadioButton()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Constructor for button class.  Initialize all fields
 *                  to logical nullptr.  Calls should then be made to other
 *                  button routines to load palettes and bitmaps.
 *
 ************************************************************************/

CRadioButton::CRadioButton() {
	m_bCheckState = false;
	m_pPalette = nullptr;
	m_cButtonFace = RGB_RADIO_FACE;
	m_cButtonControl = RGB_RADIO_CONTROL;
	m_cButtonText = RGB_RADIO_TEXT;
	m_cButtonTextDisabled = RGB_RADIO_TEXT_DISABLE;
	m_cButtonOutline = RGB_RADIO_OUTLINE;
}


/*************************************************************************
 *
 * ~CRadioButton()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Destructor for button class.  It is important to tear
 *                  things down in the order we built them; map out bitmap,
 *                  map out the palette, then discard the device context -
 *                  failure to release key resources will cause application
 *                  or system crashes.
 *
 ************************************************************************/

CRadioButton::~CRadioButton() {
	m_pPalette = nullptr;
}


/*************************************************************************
 *
 * OnEraseBkgnd()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Do not let Windows automatically erase our window
 *                  to white.
 *
 ************************************************************************/

bool CRadioButton::OnEraseBkgnd(CDC *pDC) {
	return true;                                      // do not automatically erase background to white
}


/*************************************************************************
 *
 * SetPalette()
 *
 * Parameters:
 *  CPalette *      pointer to palette to be used when painting the button
 *
 * Return Value:    none
 *
 * Description:     registers the palette to be used when painting the button;
 *                  it is assumed to be a shared resource and hence is NOT
 *                  deleted when the button is destroyed.
 *
 ************************************************************************/

void CRadioButton::SetPalette(CPalette *pPalette) {
	m_pPalette = pPalette;                          // save the palette pointer
}


/*************************************************************************
 *
 * SetColors()
 *
 * Parameters:
 *  CPalette *              pointer to palette to be used when painting the button
 *  COLORREF cFace          PALETTERGB color value for the face of the button
 *  COLORREF cControl       PALETTERGB color value for the control box
 *  COLORREF cText          PALETTERGB color value for the text in the button
 *  COLORREF cTextDisabled  PALETTERGB color value for the text when disabled
 *  COLORREF cOutline       PALETTERGB color value for the bounding rectangle
 *
 * Return Value:    none
 *
 * Description:     establishes the colors to use when painting the button;
 *                  note that cHighlight and CShadow are reversed when the
 *                  button is selected (i.e. down).
 *
 ************************************************************************/

void CRadioButton::SetColors(CPalette *pPalette, COLORREF cFace, COLORREF cControl, COLORREF cText, COLORREF cTextDisabled, COLORREF cOutline) {
	m_pPalette = pPalette;
	m_cButtonFace = cFace;
	m_cButtonControl = cControl;
	m_cButtonText = cText;
	m_cButtonTextDisabled = cTextDisabled;
	m_cButtonOutline = cOutline;
}


/*************************************************************************
 *
 * SetControl()
 *
 * Parameters:
 *  unsigned int nID        control identifier for the dialog button
 *  CWnd * pParent  pointer to parent window owning the button
 *
 * Return Value:
 *  bool            success / failure condition
 *
 * Description:     associate this button with a dialog control.
 *
 ************************************************************************/

bool CRadioButton::SetControl(unsigned int nID, CWnd * pParent) {
	if (!CRadioButton::SubclassDlgItem(nID, pParent))      // attach the Button to the dialog control
		return false;
	SetButtonStyle(BS_OWNERDRAW, false);

	return true;
}


/*************************************************************************
 *
 * DrawItem()
 *
 * Parameters:      pointer to button structure information
 *
 * Return Value:    none
 *
 * Description:     Paint a button appropriately, based on its current state.
 *
 ************************************************************************/

void CRadioButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	CDC         *pDC;
	CPalette    *pPalOld = nullptr;
	CPen        outlinePen, controlPen, framePen, *pOldPen;
	CBrush      faceBrush, *pOldBrush, myQuill, controlBrush;
	COLORREF    myTextColor, oldTextColor;
	int         nUnderscore = -1;
	int         nMyTextLength,
	            i, j, x, y, dx, dy;
	char        chMyText[128];
	TEXTMETRIC  fontMetrics;
	CSize       textInfo,
	            letterInfo,
	            underscoreInfo;
	CRect       controlRect, focusRect, selectRect;
	CWnd        *pParentWnd;

	pParentWnd = GetParent();
	if (!IsWindowVisible() ||                       // punt if not visible
	        (pParentWnd == nullptr) ||
	        !(*pParentWnd).IsWindowVisible())
		return;

	nMyTextLength = GetWindowText((char *) &chMyText, 127);
	chMyText[nMyTextLength] = '\0';                 // fetch the button label and strip
	for (i = 0, j = 0; i <= nMyTextLength; i++) {   // ... out any apersand characters
		if (chMyText[i] == '&')                     // ... which are used to force underscoring
			nUnderscore = j;                        // ... of the next letter, which in turn
		else {                                      // ... indicates the keyboard key equivalent
			chMyText[j] = chMyText[i];
			j += 1;
		}
	}
	nMyTextLength = j - 1;

	pDC = new CDC;                                  // setup the device context given to us
	(*pDC).Attach((*lpDrawItemStruct).hDC);         // ... as a pointer to a CDC object

	if (m_pPalette != nullptr) {                       // map the palette into the context
		pPalOld = (*pDC).SelectPalette(m_pPalette, true);
		(*pDC).RealizePalette();              // ... and tell the system to use it
	}

	if (((*lpDrawItemStruct).itemState & ODS_GRAYED) || // setup text color based on enable/disable state
	        ((*lpDrawItemStruct).itemState & ODS_DISABLED))
		myTextColor = m_cButtonTextDisabled;
	else
		myTextColor = m_cButtonText;
	// calculate the button width and height
	dx = (*lpDrawItemStruct).rcItem.right - (*lpDrawItemStruct).rcItem.left;
	dy = (*lpDrawItemStruct).rcItem.bottom - (*lpDrawItemStruct).rcItem.top;
	// create the bounding rectangle for the box
	controlRect.SetRect((*lpDrawItemStruct).rcItem.left + RADIO_BOX_DX,
	                    (*lpDrawItemStruct).rcItem.top + ((dy - RADIO_BOX_SIZE) >> 1),
	                    (*lpDrawItemStruct).rcItem.left + RADIO_BOX_DX + RADIO_BOX_SIZE,
	                    (*lpDrawItemStruct).rcItem.top + ((dy - RADIO_BOX_SIZE) >> 1) + RADIO_BOX_SIZE);
	selectRect = controlRect;
	// create the pens and brushes we need
	outlinePen.CreatePen(PS_INSIDEFRAME, 1, m_cButtonOutline);
	faceBrush.CreateSolidBrush(m_cButtonFace);
	controlPen.CreatePen(PS_INSIDEFRAME, 1, m_cButtonControl);
	controlBrush.CreateSolidBrush(m_cButtonControl);
	framePen.CreatePen(PS_INSIDEFRAME, 1, m_cButtonControl);

	pOldBrush = (*pDC).SelectObject(&faceBrush);
	if ((*lpDrawItemStruct).itemAction & ODA_DRAWENTIRE) {
		pOldPen = (*pDC).SelectObject(&outlinePen); // fill in the whole control
		(*pDC).Rectangle(&(*lpDrawItemStruct).rcItem);
	} else {
		pOldPen = (*pDC).SelectObject(&controlPen); // fill in just the box
		(*pDC).Ellipse(&controlRect);
	}
	(*pDC).SelectObject(pOldPen);

	pOldPen = (*pDC).SelectObject(&framePen);
	(*pDC).Ellipse(&controlRect);       // paint the box and indicate selection
	if ((*lpDrawItemStruct).itemState & ODS_SELECTED) {
		controlRect.InflateRect(-1, -1);
		(*pDC).Ellipse(&controlRect);
	}
	(*pDC).SelectObject(pOldPen);
	(*pDC).SelectObject(pOldBrush);

	if (m_bCheckState) {                            // indicate that the box is checked
		selectRect.InflateRect(-2, -2);
		pOldPen = (*pDC).SelectObject(&controlPen);
		pOldBrush = (*pDC).SelectObject(&controlBrush);
		(*pDC).Ellipse(&selectRect);
		(*pDC).SelectObject(pOldBrush);
		(*pDC).SelectObject(pOldPen);
	}

	(*pDC).GetTextMetrics(&fontMetrics);                    // get some info about the font
	textInfo = (*pDC).GetTextExtent(chMyText, nMyTextLength); // get the area spanned by the text
	x = (*lpDrawItemStruct).rcItem.left + RADIO_TEXT_DX;
	y = (*lpDrawItemStruct).rcItem.top + ((dy - textInfo.cy) >> 1);

	(*pDC).SetBkMode(TRANSPARENT);                      // make the text overlay transparently
	oldTextColor = (*pDC).SetTextColor(myTextColor);    // set the color of the text

	(*pDC).TextOut(x, y, (const char *) &chMyText, nMyTextLength);

	if (nUnderscore >= 0) {                         // put the underscore where it belongs
		underscoreInfo = (*pDC).GetTextExtent(chMyText, nUnderscore);
		dx = x + underscoreInfo.cx;
		letterInfo = (*pDC).GetTextExtent(&chMyText[nUnderscore], 1);
		underscoreInfo = (*pDC).GetTextExtent((const char *) "_", 1);
		dx += (letterInfo.cx - underscoreInfo.cx) >> 1;
		(*pDC).TextOut(dx, y, (const char *) "_", 1);
	}

	(*pDC).SetTextColor(oldTextColor);

	focusRect.SetRect(x - FOCUS_RECT_DX,                            // set the focus rectangle
	                  y - FOCUS_RECT_DY,              // ... then paint it or clear it
	                  x + textInfo.cx + FOCUS_RECT_DX - 1,
	                  y + textInfo.cy + FOCUS_RECT_DY + 1);
	if ((*lpDrawItemStruct).itemState & ODS_FOCUS) {
		myQuill.CreateStockObject(DKGRAY_BRUSH);
		(*pDC).FrameRect(&focusRect, &myQuill);
	} else {
//		(*pDC).FrameRect(&focusRect, &faceBrush);
	}

	if (pPalOld != nullptr)
		(*pDC).SelectPalette(pPalOld, false);

	(*pDC).Detach();                          // dismantle the temporary CDC we built
	delete pDC;
}


/*************************************************************************
 *
 * OnSetRadio()
 *
 * Parameters:
 *  WPARAM wParam   true / false for new check state
 *  LPARAM lParam   0
 *
 * Return Value:    0
 *
 * Description:     set the new state of the radio button box, and update
 *                  other members of the group as needed.
 *
 ************************************************************************/

LRESULT CRadioButton::OnSetCheck(WPARAM wParam, LPARAM lParam) {
	CRect   controlRect;
	CWnd    *pBase, *pWnd, *pParent;

	m_bCheckState = wParam;                         // set the new state value

	GetWindowRect(&controlRect);                    // force a repaint of the check box
	controlRect.left = RADIO_BOX_DX;
	controlRect.top = ((controlRect.bottom - controlRect.top) - RADIO_BOX_SIZE) >> 1;
	controlRect.right = RADIO_BOX_DX + RADIO_BOX_SIZE;
	controlRect.bottom = controlRect.top + RADIO_BOX_SIZE;
	InvalidateRect(&controlRect, false);
	UpdateWindow();

	if (m_bCheckState == true) {                    // if we are the control in the
		pBase = pWnd = FromHandlePermanent(m_hWnd); // ... group being set, then we
		pParent = (*pWnd).GetParent();              // ... need to clear all of the others
		while (true) {                              // so fetch the next control
			pWnd = (*pParent).GetNextDlgGroupItem(pWnd, true);
			if (pWnd == pBase)                      // ... and clear it if its not us
				break;
			(*pWnd).SendMessage(BM_SETCHECK, false, 0);
		}
	}

	return 0;
}


/*************************************************************************
 *
 * OnGetRadio()
 *
 * Parameters:
 *  WPARAM wParam   0
 *  LPARAM lParam   0
 *
 * Return Value:    true / false for current check state
 *
 * Description:     get the current state of the check box.
 *
 ************************************************************************/

LRESULT CRadioButton::OnGetCheck(WPARAM wParam, LPARAM lParam) {
	return m_bCheckState;                         // return the current state value
}


// CRadioButton message map:
// Associate messages with member functions.
//
BEGIN_MESSAGE_MAP(CRadioButton, CButton)
	//{{AFX_MSG_MAP( CRadioButton )
	ON_WM_ERASEBKGND()
	ON_WM_DRAWITEM()
	ON_MESSAGE(BM_SETCHECK, CRadioButton::OnSetCheck)
	ON_MESSAGE(BM_GETCHECK, CRadioButton::OnGetCheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

} // namespace HodjNPodj
} // namespace Bagel
