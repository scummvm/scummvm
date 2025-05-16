/*****************************************************************
 *
 *  spinner.h
 *
 *  Copyright (c) 1994 by Ledge Multimedia, All Rights Reserved
 *
 *  HISTORY
 *
 *      1.1     07/27/94     EDS     refer to spinner.cpp for details
 *
 *  MODULE DESCRIPTION:
 *
 *      Class definitions for CSpinner.
 *   
 *  RELEVANT DOCUMENTATION:
 *
 *      n/a
 *   
 ****************************************************************/

#ifndef _INC_SPINNER
#define _INC_SPINNER

#include "sprite.h"

#define SPINNER_SPEC				".\\art\\spinner.bmp"
#define HODJ_SPINNER_NUMBERS_SPEC	".\\art\\spinblue.bmp"
#define PODJ_SPINNER_NUMBERS_SPEC	".\\art\\spinred.bmp"
#define SPINNER_SOUND				".\\sound\\spinner.wav"

#define	SPINNER_COUNT		64
#define SPINNER_CYCLE		15

#define SPINNER_WAIT		2

#define SPINNER_SLOT_DDX	30
#define SPINNER_SLOT_DDY	47
#define SPINNER_SLOTA_DX	23
#define	SPINNER_SLOTA_DY	14
#define SPINNER_SLOTB_DX	66
#define	SPINNER_SLOTB_DY	14


class CSpinner : public CObject
{
	DECLARE_DYNCREATE(CSpinner)

// Constructors
public:
	CSpinner();
	CSpinner(CWnd *pWnd, CDC *pDC, int nX = 0, int nY = 0, BOOL bHodj = TRUE);

// Destructors
public:
	~CSpinner();

// Implementation
public:
	int	Animate(void)
		{ return(Animate(m_nX, m_nY)); }
	int	Animate(int nX, int nY);

private:
	BOOL Initialize(CWnd *pWnd, CDC *pDC, int nX = 0, int nY = 0, BOOL bHodj = TRUE);
	int Spin(void);
	BOOL Hide(void);
	BOOL Show(void)
		{ return(Show(m_nX,m_nY)); }
	BOOL Show(int nX,int nY);

private:
	void SetupSpinner(void);

static BOOL HandleMessages(void);

private:

	CWnd	*m_pWnd;			// window for messages
	CDC		*m_pDC;				// context for display
	CSprite	*m_pSprite;			// sprite for spinner
	int		m_nValue;			// current spinner value
	int		m_nX;				// x location of sprite
	int		m_nY;				// y location of sprite
	BOOL	m_bVisible;			// whether spinner is visible
	BOOL	m_bHodj;			// which character is active

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

};


#endif //!_INC_SPINNER
