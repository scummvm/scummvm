/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved.
 *
 *
 * animation.cpp
 *
 * HISTORY
 *
 *		1.0      08/03/94     Josquin     Creation
 *
 * MODULE DESCRIPTION:
 *
 *      [Describe the function of the module]
 *
 * CONSTRUCTORS:
 *
 *      [list constructors with one line descriptions]
 *
 * DESTRUCTORS:
 *
 *      [list destructors with one line descriptions]
 *
 * PUBLIC:
 *
 *      [list public routines with one line descriptions]
 *
 * PUBLIC GLOBAL:
 *
 *      [list global public routines with one line descriptions]
 *
 * PROTECTED:
 *
 *      [list protected routines with one line descriptions]
 *
 * PRIVATE:
 *
 *      [list private routines with one line descriptions]
 *
 * MEMBERS:
 *
 *      [list members of the class with one line descriptions]
 *
 * RELEVANT DOCUMENTATION:
 *
 *      [Specifications, documents, test plans, etc.]
 *
 ****************************************************************/

#include "stdafx.h"
#include <misc.h>
#include <mmsystem.h>
#include "main.h"
#include "animate.h"

// globals!!
//
//
extern CPalette		*pGamePalette;
extern LPGAMESTRUCT pGameParams;

CAnimate::CAnimate(CSound *pSound)
{
	m_pSprite 		= NULL;
	m_cClownRect 	= CRect(CLOWN_LEFT, CLOWN_TOP, CLOWN_RIG, CLOWN_BOT);
	m_cUFORect 		= CRect(UFO_LEFT, UFO_TOP, UFO_RIG, UFO_BOT);
	m_cCarRect 		= CRect(CAR_LEFT, CAR_TOP, CAR_RIG, CAR_BOT);
	m_cBratRect 	= CRect(BRAT_LEFT, BRAT_TOP, BRAT_RIG, BRAT_BOT);
	m_cLolliRect	= CRect(LOLLI_LEFT, LOLLI_TOP, LOLLI_RIG, LOLLI_BOT);
	m_cCutRect		= CRect(CUT_LEFT, CUT_TOP, CUT_RIG, CUT_BOT);
	m_pSound		= pSound;

	srand((unsigned) time(NULL));		// seed the random number generator
} // CAnimate

CAnimate::~CAnimate()
{
} // ~CAnimate

BOOL CAnimate::Clown(CDC *pDC, CPoint point)
{
	BOOL	bSuccess;
	int		i;

    if ( m_cClownRect.PtInRect(point) == FALSE )
		return FALSE;

	m_pSprite = new CSprite;
	(*m_pSprite).SharePalette(pGamePalette);
	bSuccess = (*m_pSprite).LoadCels(pDC, CLOWN_BMP, CLOWN_FRAMES);
	ASSERT(bSuccess);
	(*m_pSprite).SetMasked(FALSE);
	(*m_pSprite).SetMobile(FALSE);

	if ( pGameParams->bSoundEffectsEnabled != FALSE ) {
		m_pSound->Initialize(
			  	CLOWN_WAV,
                SOUND_WAVE | SOUND_ASYNCH | SOUND_QUEUE
			  	);
		m_pSound->Play();
	} // end if

	if ( bSuccess == TRUE ) {
		(*m_pSprite).SetCel( CLOWN_FRAMES );
		for( i = 0; i < CLOWN_FRAMES - 1; i++ ) {
			(*m_pSprite).PaintSprite(
						pDC,
						m_cClownRect.TopLeft()
						);
			Sleep(CLOWN_TIME_SLICE);
		} // end for
	} // end if

	m_pSprite->EraseSprite(pDC);
	if ( m_pSprite != NULL ) {
		delete m_pSprite;
		m_pSprite = NULL;
	} // end if

	return TRUE;
} // clown

BOOL CAnimate::UFO(CDC *pDC, CPoint point)
{
	BOOL	bSuccess;
	int		i;

    if ( m_cCarRect.PtInRect(point) == FALSE )
		return FALSE;

	m_pSprite = new CSprite;
	(*m_pSprite).SharePalette(pGamePalette);
	bSuccess = (*m_pSprite).LoadCels(pDC, UFOA_BMP, UFOA_FRAMES);
	ASSERT(bSuccess);
	(*m_pSprite).SetMasked(FALSE);
	(*m_pSprite).SetMobile(FALSE);

	if ( pGameParams->bSoundEffectsEnabled != FALSE ) {
		m_pSound->Initialize(
					SPACESHIP_WAV,
                    SOUND_WAVE | SOUND_ASYNCH | SOUND_QUEUE
					);
		m_pSound->Play();
	} // end if

	if ( bSuccess == TRUE ) {
		int j;
		(*m_pSprite).SetCel( UFOA_FRAMES );

		for( i = 0; i < 8; i++ ) {
			(*m_pSprite).PaintSprite(
						pDC,
						m_cUFORect.TopLeft()
						);
			Sleep(UFOA_TIME_SLICE);
		} // end for

		if ( pGameParams->bSoundEffectsEnabled != FALSE ) {
			m_pSound->Initialize(
						SPACERAY_WAV,
						SOUND_WAVE
						);
			m_pSound->Play();

			m_pSound->Initialize(
						SPACESHIP_WAV,
                        SOUND_WAVE | SOUND_ASYNCH | SOUND_QUEUE
						);
			m_pSound->Play();
		} // end if

		for( j = i; j < UFOA_FRAMES - 1; j++ ) {
			(*m_pSprite).PaintSprite(
						pDC,
						m_cUFORect.TopLeft()
						);
			Sleep(UFOA_TIME_SLICE);
		} // end for
	} // end if

	Sleep( UFO_PAUSE );									// pause for a few seconds between animations

	bSuccess = (*m_pSprite).LoadCels(pDC, UFOB_BMP, UFOB_FRAMES);
	ASSERT(bSuccess);
	(*m_pSprite).SetMasked(FALSE);
	(*m_pSprite).SetMobile(FALSE);

	if ( pGameParams->bSoundEffectsEnabled != FALSE ) {
		m_pSound->Initialize(
					SPACESHIP_WAV,
                    SOUND_WAVE | SOUND_ASYNCH | SOUND_QUEUE
					);
		m_pSound->Play();
	} // end if

	if ( bSuccess == TRUE ) {
		int j;
		(*m_pSprite).SetCel( UFOB_FRAMES );

		for( i = 0; i < 9; i++ ) {
			(*m_pSprite).PaintSprite(
						pDC,
						m_cUFORect.TopLeft()
						);
			Sleep( UFOB_TIME_SLICE );
		} // end for

		if ( pGameParams->bSoundEffectsEnabled != FALSE ) {
			m_pSound->Initialize(		// play synchronous beam up sound
						SPACERAY_WAV,
						SOUND_WAVE
						);
			m_pSound->Play();

			m_pSound->Initialize(		// async space ship sound
						SPACESHIP_WAV,
						SOUND_WAVE | SOUND_QUEUE | SOUND_ASYNCH
						);
			m_pSound->Play();
		} // end if

		for( j = i; j < UFOB_FRAMES - 1; j++ ) {
			(*m_pSprite).PaintSprite(
						pDC,
						m_cUFORect.TopLeft()
						);
			Sleep(UFOB_TIME_SLICE);
		} // end for
	} // end if

	m_pSprite->EraseSprite(pDC);
	if (m_pSprite != NULL) {
		delete m_pSprite;
		m_pSprite = NULL;
	} // end if

	return TRUE;
} // UFO

BOOL CAnimate::Brat(CPoint point)
{
    if ( m_cBratRect.PtInRect(point) == FALSE )
		return FALSE;

	if ( pGameParams->bSoundEffectsEnabled != FALSE ) {
		m_pSound->Initialize(
			  	BRAT_WAV,
                SOUND_WAVE | SOUND_ASYNCH | SOUND_QUEUE
			  	);
		m_pSound->Play();
	}  // end if

	return TRUE;
 } // Brat

BOOL CAnimate::Lollipop(CPoint point)
{
	if ( m_cLolliRect.PtInRect(point) == FALSE )
		return FALSE;

	if ( pGameParams->bSoundEffectsEnabled == FALSE )
		return TRUE;

	if ( (rand() % 2) == 0 ) {		// randomly chose sound wave to play
		m_pSound->Initialize(
				LOLLI_A_WAV,
                SOUND_WAVE | SOUND_ASYNCH | SOUND_QUEUE
				);
	} else {
		m_pSound->Initialize(
				LOLLI_B_WAV,
                SOUND_WAVE | SOUND_ASYNCH | SOUND_QUEUE
				);
	} // end if

	m_pSound->Play();
	return TRUE;
} // Lollipop

BOOL CAnimate::Haircut(CPoint point)
{
	if ( m_cCutRect.PtInRect(point) == FALSE )
		return FALSE;

	if ( pGameParams->bSoundEffectsEnabled == FALSE )
		return TRUE;

	m_pSound->Initialize(
			CUT_WAV,
            SOUND_WAVE | SOUND_ASYNCH | SOUND_QUEUE
			);

	m_pSound->Play();
	return TRUE;
} // Haircut
