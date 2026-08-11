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
#include "bagel/boflib/misc.h"
#include "bagel/hodjnpodj/barbershop/main.h"
#include "bagel/hodjnpodj/barbershop/animate.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Barbershop {

// globals!!
//
//
extern CPalette     *pGamePalette;
extern LPGAMESTRUCT pGameParams;

CAnimate::CAnimate(CSound *pSound) {
	m_pSprite       = nullptr;
	m_cClownRect    = CRect(CLOWN_LEFT, CLOWN_TOP, CLOWN_RIG, CLOWN_BOT);
	m_cUFORect      = CRect(UFO_LEFT, UFO_TOP, UFO_RIG, UFO_BOT);
	m_cCarRect      = CRect(CAR_LEFT, CAR_TOP, CAR_RIG, CAR_BOT);
	m_cBratRect     = CRect(BRAT_LEFT, BRAT_TOP, BRAT_RIG, BRAT_BOT);
	m_cLolliRect    = CRect(LOLLI_LEFT, LOLLI_TOP, LOLLI_RIG, LOLLI_BOT);
	m_cCutRect      = CRect(CUT_LEFT, CUT_TOP, CUT_RIG, CUT_BOT);
	m_pSound        = pSound;

	//srand((unsigned) time(nullptr));     // seed the random number generator
} // CAnimate

CAnimate::~CAnimate() {
} // ~CAnimate

bool CAnimate::Clown(CDC *pDC, CPoint point) {
	bool    bSuccess;
	int     i;

	if (m_cClownRect.PtInRect(point) == false)
		return false;

	m_pSprite = new CSprite;
	(*m_pSprite).SharePalette(pGamePalette);
	bSuccess = (*m_pSprite).LoadCels(pDC, CLOWN_BMP, CLOWN_FRAMES);
	ASSERT(bSuccess);
	(*m_pSprite).SetMasked(false);
	(*m_pSprite).SetMobile(false);

	if (pGameParams->bSoundEffectsEnabled != false) {
		m_pSound->initialize(
		    CLOWN_WAV,
		    SOUND_WAVE | SOUND_ASYNCH | SOUND_QUEUE
		);
		m_pSound->play();
	} // end if

	if (bSuccess == true) {
		(*m_pSprite).SetCel(CLOWN_FRAMES);
		for (i = 0; i < CLOWN_FRAMES - 1; i++) {
			(*m_pSprite).PaintSprite(
			    pDC,
			    m_cClownRect.TopLeft()
			);
			Sleep(CLOWN_TIME_SLICE);
		} // end for
	} // end if

	m_pSprite->EraseSprite(pDC);

	delete m_pSprite;
	m_pSprite = nullptr;

	return true;
}

bool CAnimate::UFO(CDC *pDC, CPoint point) {
	bool    bSuccess;
	int     i;

	if (m_cCarRect.PtInRect(point) == false)
		return false;

	m_pSprite = new CSprite;
	(*m_pSprite).SharePalette(pGamePalette);
	bSuccess = (*m_pSprite).LoadCels(pDC, UFOA_BMP, UFOA_FRAMES);
	ASSERT(bSuccess);
	(*m_pSprite).SetMasked(false);
	(*m_pSprite).SetMobile(false);

	if (pGameParams->bSoundEffectsEnabled != false) {
		m_pSound->initialize(
		    SPACESHIP_WAV,
		    SOUND_WAVE | SOUND_ASYNCH | SOUND_QUEUE
		);
		m_pSound->play();
	} // end if

	if (bSuccess == true) {
		int j;
		(*m_pSprite).SetCel(UFOA_FRAMES);

		for (i = 0; i < 8; i++) {
			(*m_pSprite).PaintSprite(
			    pDC,
			    m_cUFORect.TopLeft()
			);
			Sleep(UFOA_TIME_SLICE);
		} // end for

		if (pGameParams->bSoundEffectsEnabled != false) {
			m_pSound->initialize(
			    SPACERAY_WAV,
			    SOUND_WAVE
			);
			m_pSound->play();

			m_pSound->initialize(
			    SPACESHIP_WAV,
			    SOUND_WAVE | SOUND_ASYNCH | SOUND_QUEUE
			);
			m_pSound->play();
		} // end if

		for (j = i; j < UFOA_FRAMES - 1; j++) {
			(*m_pSprite).PaintSprite(
			    pDC,
			    m_cUFORect.TopLeft()
			);
			Sleep(UFOA_TIME_SLICE);
		} // end for
	} // end if

	Sleep(UFO_PAUSE);                                    // pause for a few seconds between animations

	bSuccess = (*m_pSprite).LoadCels(pDC, UFOB_BMP, UFOB_FRAMES);
	ASSERT(bSuccess);
	(*m_pSprite).SetMasked(false);
	(*m_pSprite).SetMobile(false);

	if (pGameParams->bSoundEffectsEnabled != false) {
		m_pSound->initialize(
		    SPACESHIP_WAV,
		    SOUND_WAVE | SOUND_ASYNCH | SOUND_QUEUE
		);
		m_pSound->play();
	} // end if

	if (bSuccess == true) {
		int j;
		(*m_pSprite).SetCel(UFOB_FRAMES);

		for (i = 0; i < 9; i++) {
			(*m_pSprite).PaintSprite(
			    pDC,
			    m_cUFORect.TopLeft()
			);
			Sleep(UFOB_TIME_SLICE);
		} // end for

		if (pGameParams->bSoundEffectsEnabled != false) {
			m_pSound->initialize(  // play synchronous beam up sound
			    SPACERAY_WAV,
			    SOUND_WAVE
			);
			m_pSound->play();

			m_pSound->initialize(  // async space ship sound
			    SPACESHIP_WAV,
			    SOUND_WAVE | SOUND_QUEUE | SOUND_ASYNCH
			);
			m_pSound->play();
		} // end if

		for (j = i; j < UFOB_FRAMES - 1; j++) {
			(*m_pSprite).PaintSprite(
			    pDC,
			    m_cUFORect.TopLeft()
			);
			Sleep(UFOB_TIME_SLICE);
		} // end for
	} // end if

	m_pSprite->EraseSprite(pDC);

	delete m_pSprite;
	m_pSprite = nullptr;

	return true;
}

bool CAnimate::Brat(CPoint point) {
	if (m_cBratRect.PtInRect(point) == false)
		return false;

	if (pGameParams->bSoundEffectsEnabled != false) {
		m_pSound->initialize(
		    BRAT_WAV,
		    SOUND_WAVE | SOUND_ASYNCH | SOUND_QUEUE
		);
		m_pSound->play();
	}  // end if

	return true;
} // Brat

bool CAnimate::Lollipop(CPoint point) {
	if (m_cLolliRect.PtInRect(point) == false)
		return false;

	if (pGameParams->bSoundEffectsEnabled == false)
		return true;

	if ((brand() % 2) == 0) {        // randomly chose sound wave to play
		m_pSound->initialize(
		    LOLLI_A_WAV,
		    SOUND_WAVE | SOUND_ASYNCH | SOUND_QUEUE
		);
	} else {
		m_pSound->initialize(
		    LOLLI_B_WAV,
		    SOUND_WAVE | SOUND_ASYNCH | SOUND_QUEUE
		);
	} // end if

	m_pSound->play();
	return true;
} // Lollipop

bool CAnimate::Haircut(CPoint point) {
	if (m_cCutRect.PtInRect(point) == false)
		return false;

	if (pGameParams->bSoundEffectsEnabled == false)
		return true;

	m_pSound->initialize(
	    CUT_WAV,
	    SOUND_WAVE | SOUND_ASYNCH | SOUND_QUEUE
	);

	m_pSound->play();
	return true;
} // Haircut

} // namespace Barbershop
} // namespace HodjNPodj
} // namespace Bagel
