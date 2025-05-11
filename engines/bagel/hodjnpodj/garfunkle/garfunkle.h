/*****************************************************************
 *
 *  Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *
 *  game.h
 *
 *  HISTORY
 *
 *      1.00      04/04/94     BAR     Initial creation
 *
 *  MODULE DESCRIPTION:
 *
 *      [Describe the function of the module]
 *   
 *  RELEVANT DOCUMENTATION:
 *
 *      [Specifications, documents, test plans, etc.]
 *   
 ****************************************************************/

#ifndef __game_H__
#define __game_H__

#include <sound.h>

#define PLAY_FACTOR		150		// Multiply MilliSeconds by SpeedFactor for tenths of Seconds
#define ANIM_SLEEP		200		// Milliseconds

// Button Identifier codes (BIDs)
#define IDC_A		100
#define IDC_B		101
#define IDC_C		102
#define IDC_D		103
#define IDC_E		104
#define IDC_F		105 
#define IDC_START	106

// Win condition values
#define	LOW_WIN          8		// Minimum number of notes in completed
#define	MEDIUM_WIN      10		//...sequence required to receive info
#define	HIGH_WIN		12		//...when playing from the metagame

// Backdrop bitmaps
#define MAINSCREEN		".\\ART\\GARFEMPT.BMP"
#define	RIBBON			".\\ART\\RIBBON.BMP"

#define	RIBBON_X		139
#define	RIBBON_Y		332

#define	SIGN_COLOR			RGB( 0, 0, 0 )			// Color of the sign text
#define SIGN_LOCATION_X		117
#define SIGN_LOCATION_Y		310
#define SIGN_WIDTH			 33
#define SIGN_HEIGHT			 30

#define VIOLIN_LOCATION_X		199
#define VIOLIN_LOCATION_Y		210
#define VIOLIN_WIDTH			 57
#define VIOLIN_HEIGHT			 96
                       
#define CELLO_LOCATION_X		257
#define CELLO_LOCATION_Y		206
#define CELLO_WIDTH			 	 63
#define CELLO_HEIGHT			101
                       
#define DRUM_LOCATION_X		321
#define DRUM_LOCATION_Y		208
#define DRUM_WIDTH			 57
#define DRUM_HEIGHT			 92
                       
#define SAX_LOCATION_X		380
#define SAX_LOCATION_Y		208
#define SAX_WIDTH			 55
#define SAX_HEIGHT			 99
                       
#define HARP_LOCATION_X		141
#define HARP_LOCATION_Y		194
#define HARP_WIDTH			 56
#define HARP_HEIGHT			107
                       
#define CLARINET_LOCATION_X		444
#define CLARINET_LOCATION_Y		180
#define CLARINET_WIDTH			 46
#define CLARINET_HEIGHT			127

#define	NEWGAME_LOCATION_X	 15
#define	NEWGAME_LOCATION_Y	  0
#define	NEWGAME_WIDTH		217
#define NEWGAME_HEIGHT		 20
                       
#define SIGN_LOCATION_X		117
#define SIGN_LOCATION_Y		310
#define SIGN_WIDTH			 33
#define SIGN_HEIGHT			 30

#define WOODRIGHT_LOCATION_X	460
#define WOODRIGHT_LOCATION_Y	25
#define WOODRIGHT_WIDTH			164
#define WOODRIGHT_HEIGHT		137
						
#define WOODLEFT_LOCATION_X 	17
#define WOODLEFT_LOCATION_Y		25
#define WOODLEFT_WIDTH        	137
#define WOODLEFT_HEIGHT			148
		
#define BENCH_LOCATION_X		17
#define BENCH_LOCATION_Y		404
#define BENCH_WIDTH				610
#define BENCH_HEIGHT			62
 
// Simon Button constants
#define MIN_BUTTONS		3
#define MAX_BUTTONS		6
#define MAX_SEQUENCE	25
         
// Playback speed constants         
#define MIN_SPEED		 0
#define MAX_SPEED		11
#define NUM_SPEEDS		12
#define	SLOW_DOWN		 1		// take off this amount from time between notes on playback

#define PLAYER_TIMER	3		// can't wait forever for a note!
#define ANIM_TIMER		4		// time to change frames
#define TIME_LIMIT		10000	// 10 seconds for player response
#define PAUSE_TIME		50		// in milliseconds, so = 1/2 sec
#define INCREMENT_RATE	5		// rate by which speed is increased		

#define	RULES_TEXT		"GARFUNK.TXT"				// rules file
                          
#define WIN_SOUND	  	".\\SOUND\\FANFARE1.WAV"
#define WRONG_SOUND	   	".\\SOUND\\SOSORRY.WAV"
#define SLOW_SOUND		".\\SOUND\\ALARM.WAV"
#define SIGN_1_SOUND	".\\SOUND\\SIGN1.WAV"
#define SIGN_2_SOUND	".\\SOUND\\SIGN2.WAV"
#define SIGN_3_SOUND	".\\SOUND\\SIGN3.WAV"
#define SIGN_4_SOUND	".\\SOUND\\SIGN4.WAV"
#define SIGN_5_SOUND	".\\SOUND\\SIGN5.WAV"
#define BENCH_SOUND		".\\SOUND\\MYSEAT.WAV"
#define TREES_SOUND		".\\SOUND\\TIMBER.WAV"
#define RULES_SOUND		".\\SOUND\\GARFUNK.WAV"

#define	NUM_SIGN_SOUNDS	5

#define VIOLIN_SOUND	".\\SOUND\\viol2.mid"
#define CELLO_SOUND		".\\SOUND\\cello2.mid"
#define DRUM_SOUND		".\\SOUND\\DRUM2.MID"	//drum.mid"		//DRUM.WAV"
#define SAX_SOUND		".\\SOUND\\sax2.mid"	
#define HARP_SOUND		".\\SOUND\\harp2.mid"
#define CLARINET_SOUND	".\\SOUND\\clar2.mid"

#define VIOLIN_ANIM		".\\ART\\violin2.bmp"
#define CELLO_ANIM		".\\ART\\cello2.bmp"
#define DRUM_ANIM		".\\ART\\drums2.bmp"
#define SAX_ANIM		".\\ART\\sax2.bmp"	
#define HARP_ANIM		".\\ART\\harp2.bmp"	
#define CLARINET_ANIM	".\\ART\\clari2.bmp"

#define VIOLIN_CELS		14
#define CELLO_CELS		16
#define DRUM_CELS		20
#define SAX_CELS		14	
#define HARP_CELS		17
#define CLARINET_CELS	18	

#define VIOLIN_OFFSET_X		2
#define CELLO_OFFSET_X		4
#define DRUM_OFFSET_X		0
#define SAX_OFFSET_X		2	
#define HARP_OFFSET_X		2
#define CLARINET_OFFSET_X	5	

#define VIOLIN_OFFSET_Y		16
#define CELLO_OFFSET_Y		5
#define DRUM_OFFSET_Y		3
#define SAX_OFFSET_Y		4	
#define HARP_OFFSET_Y		0
#define CLARINET_OFFSET_Y	4	

//BOOL wait_awhile(int);

///////////////////////////////////////////////////////////////////////////////////

// CMainWindow:
// See game.cpp for the code to the member functions and the message map.
//
class CMainWindow : public CFrameWnd
{
public:
	CMainWindow();
	BOOL GetNewSequence (const char* pszFileName);
	BOOL GetNewSequence (int nLength); 
	void ActivateButtons (UINT nNumActive, BOOL bState);
	void PlayBackSeries(int nNumNotes);
	void NewGame();
	void StartAnimation();
	void StopAnimation();
	BOOL wait_awhile(int nHundSecs);
		
//added data members:
virtual void SplashScreen();

private:
  	void OnSoundNotify(CSound *pSound);

protected:
virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	void OnDestroy();

	//{{AFX_MSG( CMainWindow )
	afx_msg void OnPaint();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
	afx_msg void OnSysKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
//	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point); 
	afx_msg void OnActivate(UINT, CWnd*, BOOL );
	afx_msg	void OnClose();
    afx_msg long OnMCINotify( WPARAM, LPARAM);
    afx_msg long OnMMIONotify( WPARAM, LPARAM);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif // __game_H__

