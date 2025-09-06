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

#include "bagel/hodjnpodj/metagame/bgen/stdafx.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"
#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/metagame/gtl/spinner.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

// ordered table of spinner values (will be shuffled)
static int8 SpinnerValues[SPINNER_COUNT] = {
	6,
	7,
	7,
	8,
	8,
	8,
	9,
	9,
	9,
	9,
	10,
	10,
	10,
	10,
	10,
	11,
	11,
	11,
	11,
	11,
	11,
	12,
	12,
	12,
	12,
	12,
	12,
	12,
	13,
	13,
	13,
	13,
	13,
	13,
	13,
	13,
	14,
	14,
	14,
	14,
	14,
	14,
	14,
	15,
	15,
	15,
	15,
	15,
	15,
	16,
	16,
	16,
	16,
	16,
	17,
	17,
	17,
	17,
	18,
	18,
	18,
	19,
	19,
	20
};


IMPLEMENT_DYNCREATE(CSpinner, CObject)

/*************************************************************************
 *
 * CSpinner()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Constructor for animation class.  Initialize all fields
 *                  to logical nullptr.  A call should then be made to the
 *                  Initialize routine to setup the spinner sprite.
 *
 ************************************************************************/

CSpinner::CSpinner() {
	m_nValue = -1;                                  // no defined value
	m_bVisible = false;                             // not yet visible
	m_pSprite = nullptr;                               // no loaded sprite
	m_nX = m_nY = 0;                                // upper left hand corner
	SetupSpinner();                                 // shuffle spinner values
}


/*************************************************************************
 *
 * CSpinner()
 *
 * Parameters:
 *  CWnd * pWnd     window for messages
 *  CDC * pDC       context for display
 *  int nX, nY      upper lefthand corner position for spinner
 *
 * Return Value:    none
 *
 * Description:     Constructor for animation class.  Initialize all fields
 *                  and load spinner spite.  Calls should then be made to the
 *                  Show / Spin / Hide routines.
 *
 ************************************************************************/

CSpinner::CSpinner(CWnd *pWnd, CDC *pDC, int nX, int nY, bool bHodj) {
	Initialize(pWnd, pDC, nX, nY, bHodj);       // go initialize things
	SetupSpinner();                                 // shuffle spinner values
}


/*************************************************************************
 *
 * Initialize()
 *
 * Parameters:
 *  CWnd * pWnd     window for messages
 *  CDC * pDC       context for display
 *  int nX, nY      upper lefthand corner position for spinner
 *
 * Return Value:
 *  bool            success / failure
 *
 * Description:     Initialize all fields and load spinner spite.
 *
 ************************************************************************/

bool CSpinner::Initialize(CWnd *pWnd, CDC *pDC, int nX, int nY, bool bHodj) {
	bool    bSuccess = false;

	m_pWnd = pWnd;                                  // window for messages
	m_pDC = pDC;                                    // context for display
	m_nValue = -1;                                  // no initial value
	m_bVisible = false;                             // not yet visible
	m_nX = nX;                                      // set the position
	m_nY = nY;
	m_bHodj = bHodj;

	m_pSprite = new CSprite();                      // load the sprite image
	if ((m_pDC != nullptr) &&
	        (m_pSprite != nullptr)) {
		bSuccess = (*m_pSprite).LoadSprite(m_pDC, SPINNER_SPEC);
		if (bSuccess)
			(*m_pSprite).SetMasked(true);
		else {
			delete m_pSprite;                       // failed so release it
			m_pSprite = nullptr;
		}
	}

	return bSuccess;
}


/*************************************************************************
 *
 * ~CSpinner()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Destructor for spinner class.  The spinner sprite is
 *                  purged.
 *
 ************************************************************************/

CSpinner::~CSpinner() {
	if (m_pSprite != nullptr)                          // delete the sprite
		delete m_pSprite;
}


/*************************************************************************
 *
 * SetupSpinner()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     double-shuffle spinner values to help ensure randomness.
 *
 ************************************************************************/

void CSpinner::SetupSpinner() {
	int i, j, n;
	int Values[SPINNER_COUNT];

	for (j = 0; j < 10; j++) {                      // randomly place spinner values
		for (i = 0; i < SPINNER_COUNT; i++)         // ... into the buffer, skipping
			Values[i] = 0;                          // ... ahead for an empty spot as needed

		for (i = 0; i < SPINNER_COUNT; i++) {
			n = brand() % SPINNER_COUNT;
			while (Values[n] != 0) {
				n += 1;
				if (n >= SPINNER_COUNT)
					n = 0;
			}
			Values[n] = SpinnerValues[i];
		}

		for (i = 0; i < SPINNER_COUNT; i++)         // now similarly randomly place them
			SpinnerValues[i] = 0;                   // ... back where they came from, thus
		// ... double-shuffling the entries
		for (i = 0; i < SPINNER_COUNT; i++) {
			n = brand() % SPINNER_COUNT;
			while (SpinnerValues[n] != 0) {
				n += 1;
				if (n >= SPINNER_COUNT)
					n = 0;
			}
			SpinnerValues[n] = Values[i];
		}
	}

}


/*************************************************************************
 *
 * Animate()
 *
 * Parameters:
 *  int nX, nY      upper left hand position for spinner
 *
 * Return Value:
 *  int             spinner value
 *
 * Description:     reveal, animate, and hide the spinner sprite.
 *
 ************************************************************************/

int CSpinner::Animate(int nX, int nY) {
	int     nValue = -1;
	uint32   goal;

	m_nX = nX;                                      // establish position
	m_nY = nY;

	AfxGetApp()->DoWaitCursor(1);

	nValue = Spin();                               // spin it and get a result
	if (nValue > 0) {                              // pause for a moment
		goal = GetTickCount() + SPINNER_WAIT * 1000L;
		while (goal > GetTickCount()) {             // handle non-input messages
			if (HandleMessages())                   // ... and terminate loop if urgent
				break;
		}
		Hide();                              // hide the spinner
	}

	AfxGetApp()->DoWaitCursor(-1);

	return (nValue);
}


/*************************************************************************
 *
 * Show()
 *
 * Parameters:
 *  int nX, nY      upper left hand position for spinner
 *
 * Return Value:
 *  bool            success /failure
 *
 * Description:     reveal the spinner sprite, without any digits.
 *
 ************************************************************************/

bool CSpinner::Show(int nX, int nY) {
	bool    bSuccess = false;

	if (m_pSprite == nullptr)                              // punt if no spinner sprite
		return false;

	m_nX = nX;
	m_nY = nY;

	bSuccess = (*m_pSprite).PaintSprite(m_pDC, nX, nY);

	if (bSuccess)
		m_bVisible = true;
	return bSuccess;
}


/*************************************************************************
 *
 * Hide()
 *
 * Parameters:      none
 *
 * Return Value:
 *  bool            success /failure
 *
 * Description:     hide the spinner sprite.
 *
 ************************************************************************/

bool CSpinner::Hide() {
	bool    bSuccess = false;

	if (m_pSprite == nullptr)                              // punt if no spinner sprite
		return false;

	if (m_bVisible) {                                   // if visible ...
		bSuccess = (*m_pSprite).EraseSprite(m_pDC);     // ... just erase the sprite
		if (bSuccess)
			m_bVisible = false;                         // ... and mark spinner as invisible
	} else
		bSuccess = true;

	return bSuccess;
}


/*************************************************************************
 *
 * Spin()
 *
 * Parameters:      none
 *
 * Return Value:
 *  int             spinner value (-1 for error)
 *
 * Description:     animate the spinner and return a value.
 *
 ************************************************************************/

int CSpinner::Spin() {
	int         i, j, nIdx;
	CBitmap     *pBitmap = nullptr;
	CPalette    *pPalette = nullptr;
	CRect       srcRect, dstARect, dstBRect;
	int         nValue = -1;
	CSound      *pSound;
	bool        bSuccess = false;
	CWinApp *app = AfxGetApp();

	if (m_pSprite == nullptr)                              // punt if no spinner sprite
		return -1;

	CSound::waitWaveSounds();

	pSound = new CSound();                              // create the spinner sound
	(*pSound).initialize(m_pWnd, SPINNER_SOUND, SOUND_WAVE | SOUND_QUEUE | SOUND_BUFFERED | SOUND_ASYNCH | SOUND_NOTIFY | SOUND_LOOP);
	(*pSound).play();                                   // start the spinner sound

	if (!m_bVisible) {                                  // make it visible
		bSuccess = Show(m_nX, m_nY);
		if (!bSuccess)
			goto punt;
	}

	nIdx = brand() % SPINNER_COUNT;                      // generate a spinner value
	nValue = SpinnerValues[nIdx];

	dstARect.SetRect(m_nX + SPINNER_SLOTA_DX,           // calculate rectangles for digits
	                 m_nY + SPINNER_SLOTA_DY,
	                 m_nX + SPINNER_SLOTA_DX + SPINNER_SLOT_DDX,
	                 m_nY + SPINNER_SLOTA_DY + SPINNER_SLOT_DDY);
	dstBRect.SetRect(m_nX + SPINNER_SLOTB_DX,
	                 m_nY + SPINNER_SLOTB_DY,
	                 m_nX + SPINNER_SLOTB_DX + SPINNER_SLOT_DDX,
	                 m_nY + SPINNER_SLOTB_DY + SPINNER_SLOT_DDY);
	// dispaly the spinner digits
	pBitmap = FetchBitmap(m_pDC, &pPalette, (m_bHodj ? HODJ_SPINNER_NUMBERS_SPEC : PODJ_SPINNER_NUMBERS_SPEC));

	i = nValue / 10;                                    // update spinner with actual value
	srcRect.SetRect(i * SPINNER_SLOT_DDX, 0, (i + 1) * SPINNER_SLOT_DDX, SPINNER_SLOT_DDY);
	bSuccess = BltBitmap(m_pDC, pPalette, pBitmap, &srcRect, &dstARect, (uint32) SRCCOPY);
	if (bSuccess) {
		j = nValue % 10;
		srcRect.SetRect(j * SPINNER_SLOT_DDX, 0, (j + 1) * SPINNER_SLOT_DDX, SPINNER_SLOT_DDY);
		bSuccess = BltBitmap(m_pDC, pPalette, pBitmap, &srcRect, &dstBRect, (uint32) SRCCOPY);
	}

	app->pause();

	if (pSound != nullptr) {                               // terminate the sound gracefully
		(*pSound).stop();
		delete pSound;
		pSound = nullptr;
	}

punt:
	if (pBitmap != nullptr)                                // release the resources we used
		delete pBitmap;
	if (pPalette != nullptr)
		delete pPalette;

	if (pSound != nullptr)
		delete pSound;

	if (bSuccess)                                      // return value if all went well
		return (nValue);

	return -1;
}


/*************************************************************************
 *
 * HandleMessages()
 *
 * Parameters:      none
 *
 * Return Value:
 *  bool            whether an urgent message is pending
 *
 * Description:     process pending non-keyboard/mouse messages.
 *
 ************************************************************************/

bool CSpinner::HandleMessages() {
	MSG     msg;

	if (PeekMessage(&msg, nullptr, 0, WM_KEYFIRST - 1, PM_REMOVE)) {
		if (msg.message == WM_CLOSE || msg.message == WM_QUIT)
			return true;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (PeekMessage(&msg, nullptr, WM_KEYLAST + 1, WM_MOUSEMOVE, PM_REMOVE)) {
		if (msg.message == WM_CLOSE || msg.message == WM_QUIT)
			return true;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (PeekMessage(&msg, nullptr, WM_PARENTNOTIFY, 0xFFFF, PM_REMOVE)) {
		if (msg.message == WM_CLOSE || msg.message == WM_QUIT)
			return true;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return false;
}

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
