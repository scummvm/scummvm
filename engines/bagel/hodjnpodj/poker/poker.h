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

#ifndef HODJNPODJ_POKER_POKER_H
#define HODJNPODJ_POKER_POKER_H

#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/hnplibs/dibdoc.h"
#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"            // header for the options library
#include "bagel/hodjnpodj/hnplibs/mainmenu.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/hodjnpodj/poker/resource.h"

namespace Bagel {
namespace HodjNPodj {
namespace Poker {


// Button Identifier codes (BIDs)
#define IDC_STARTAMOUNT                 105
#define IDC_ODDS                                106
#define IDC_OPTION                          119
#define IDC_BET1            120
#define IDC_BET5            121
#define IDC_BET10           122
#define IDC_BET25           123
#define IDC_BET100      124
#define IDC_BET1000     125
#define IDC_BETALL      126
#define IDC_CLEARBET    127
#define IDC_HOLD1           128
#define IDC_HOLD2           129
#define IDC_HOLD3           130
#define IDC_HOLD4           131
#define IDC_HOLD5           132
#define IDC_DEAL            133
#define IDC_DRAW            134
#define IDC_JUNK            999

// Sound File Identifiers
#define CARDSOUND               ".\\SOUND\\CARDDEAL.WAV"    //FWAP.WAV"
#define WAV_PLACE               ".\\SOUND\\COIN.WAV"
#define WAV_CLEAR               ".\\SOUND\\CEARBET.WAV"
#define WAV_HOLD                ".\\SOUND\\CLICK.WAV"
#define WAV_PAY                 ".\\SOUND\\PAYOFF.WAV"
#define WAV_NOPAY               ".\\SOUND\\BUZZER2.WAV"
#define WAV_BROKE               ".\\SOUND\\SOSORRY.WAV"
//
// Easter Egg sound files
#define WAV_CLOCK               ".\\SOUND\\BIGBEN.WAV"
#define WAV_BOOM                ".\\SOUND\\EXPLODE.WAV"
#define WAV_STAR                ".\\SOUND\\TOMTOMS.WAV"
#define WAV_PENCIL              ".\\SOUND\\PENCIL.WAV"
#define WAV_CHAIR               ".\\SOUND\\SQUEAK.WAV"
#define WAV_WINDOW              ".\\SOUND\\WINDOW.WAV"
#define WAV_LIGHT               ".\\SOUND\\LIGHTING.WAV"
//
// Easter Egg art files
#define CLOCK_ANIM              ".\\ART\\CLOCK.BMP"
#define BOOM_ANIM               ".\\ART\\BOOM.BMP"
#define CLOCK_FRAMES            16
#define BOOM_FRAMES             25
#define CLOCK_SLEEP             175
#define BOOM_SLEEP              100
//
// Easter Egg locations
#define CLOCK_X     572
#define CLOCK_Y      74
#define CLOCK_DX     52
#define CLOCK_DY     91

#define BOOM_X      569
#define BOOM_Y      162
#define BOOM_DX      56
#define BOOM_DY      78

#define PENCIL_X     16
#define PENCIL_Y    420
#define PENCIL_DX    55
#define PENCIL_DY    21

#define CHAIR_X     562
#define CHAIR_Y     225
#define CHAIR_DX     40
#define CHAIR_DY    136

#define WINDOW_X     16
#define WINDOW_Y     83
#define WINDOW_DX    55
#define WINDOW_DY   148

#define LIGHT_X      21
#define LIGHT_Y      24
#define LIGHT_DX     47
#define LIGHT_DY     46

// Bitmap Identifiers
#define SPLASHSPEC              "ART\\POKER.BMP"
#define OPTSCROLL               "ART\\OPTSCRL.BMP"
#define CARDBACKBMP             "ART\\CARDBACK.BMP"

// Button positioning constants
#define OPTION_WIDTH                146
#define OPTION_HEIGHT               23
#define OPTION_LEFT                 246
#define OPTION_TOP                  0

#define HOLD_TOP                    202
#define HOLD1_LEFT              120
#define HOLD2_LEFT              206
#define HOLD3_LEFT              288
#define HOLD4_LEFT              369
#define HOLD5_LEFT              454
#define HOLD_WIDTH              66
#define HOLD_HEIGHT             62

#define AMOUNT_WIDTH      135
#define AMOUNT_HEIGHT           18

#define USER_AMOUNT_X           282
#define USER_AMOUNT_Y           375

#define POT_AMOUNT_X            282
#define POT_AMOUNT_Y            300

#define DEAL_LEFT         436
#define DEAL_TOP          270
#define DEAL_WIDTH        92
#define DEAL_HEIGHT             67

#define DRAW_LEFT         436
#define DRAW_TOP          355
#define DRAW_WIDTH        92
#define DRAW_HEIGHT             68

#define CARD_WIDTH              72
#define CARD_HEIGHT             103
#define CARD_TOP                    93
#define CARD_LEFT               112
#define CARD_OFFSET             85

#define BET1_LEFT                   116
#define BET1_TOP                    263
#define BET1_WIDTH              46
#define BET1_HEIGHT             42

#define BET5_LEFT                   116
#define BET5_TOP                    306
#define BET5_WIDTH              46
#define BET5_HEIGHT             41

#define BET10_LEFT              116
#define BET10_TOP                   347
#define BET10_WIDTH             46
#define BET10_HEIGHT            42

#define BET25_LEFT              220
#define BET25_TOP                   263
#define BET25_WIDTH             48
#define BET25_HEIGHT            42

#define BET100_LEFT             220
#define BET100_TOP              305
#define BET100_WIDTH            46
#define BET100_HEIGHT           42

#define BET1000_LEFT            220
#define BET1000_TOP             347
#define BET1000_WIDTH           46
#define BET1000_HEIGHT      41

#define BETALL_LEFT             168
#define BETALL_TOP              304
#define BETALL_WIDTH            48
#define BETALL_HEIGHT           44

#define CLEARBET_LEFT           122
#define CLEARBET_TOP      393
#define CLEARBET_WIDTH      139
#define CLEARBET_HEIGHT     27

#define NUMBEROFROUNDS      4

#define HOLD_BMP_GRID_WITDH     66
#define HOLD_BMP_GRID_TOP1      0
#define HOLD_BMP_GRID_TOP2      62
#define HOLD_BMP_GRID_TOP3      124
#define HOLD_BMP_GRID_TOP4      186

#define BET_BMP_GRID_WITDH  48
#define BET_BMP_GRID_TOP1   0
#define BET_BMP_GRID_TOP2   44
#define BET_BMP_GRID_TOP3   88


void PlayEasterEgg(CDC *, CWnd *, CPalette *, const char *,
                   const char *, int, int, int, int, bool);

/////////////////////////////////////////////////////////////////////////////

// CMainPokerWindow:
// See game.cpp for the code to the member functions and the message map.
//

class CMainPokerWindow : public CFrameWnd {
public:
	bool            m_bPlaySounds = false;		// Bool for should I play sounds
	bool            m_bMiddleOfHand = false;	// Bool for am I in the middle of a hand
	bool            m_bPlayRounds = false;		// Bool for am I playing a certain # of rounds
	long            m_lUserAmount = 0;
	long            m_lStartingAmount = 0;

private:
	bool            abHoldArray[5] = { false };	// Contains the state of the hold buttons
	int             aDealtArray[10][2] = {};	// Contains the list of cards already dealt
	// the second element is 1 if the card is shown
	long            m_lUserBet = 0;				// The amount of money the player currently has
	int             m_nRound = 0;				// the amount of the current bet
	int             m_nPayOffRoyalFlush = 0;	// the pay off ratios
	int             m_nPayOffStraightFlush = 0;
	int             m_nPayOffFourofaKind = 0;
	int             m_nPayOffStraight = 0;
	int             m_nPayOffFullHouse = 0;
	int             m_nPayOffFlush = 0;
	int             m_nPayOffThreeofaKind = 0;
	int             m_nPayOffTwoPair = 0;
	int             m_nPayOffPairJackorHigher = 0;
	int             m_nPayOffPair = 0;
	HWND            m_hCallAppWnd = nullptr;
	LPGAMESTRUCT    m_lpGameStruct = nullptr;
	bool            m_bMouseCaptured = false;
	bool            m_bEndHand = false;
	bool            _flagResetGame = false;

	CRect   MainRect;			// screen area spanned by the game window
	CRect   BetRect1,			// window area spanned by the Bet1 button
		BetRect5,				// window area spanned by the Bet5 button
		BetRect10,				// window area spanned by the Bet10 button
		BetRect25,				// window area spanned by the Bet25 button
		BetRect100,				// window area spanned by the Bet100 button
		BetRect1000,			// window area spanned by the Bet1000 button
		BetRectAll,				// window area spanned by the BetAll button
		ClearBetRect,			// window area spanned by the ClearBet button
		DealRect,				// window area spanned by the Deal button
		DrawRect,				// window area spanned by the Draw button
		OptionRect,				// window area spanned by the Option button
		arCardRect[5];			// window area spanned by the Cards

	CRect   JunkRect;
	CRect   NewGameRect;

public:
	CMainPokerWindow(HWND, LPGAMESTRUCT);
	void initStatics();

	void    SplashScreen();
	void    SetPayOffs(int);
	void    FlagResetGame() {
		_flagResetGame = true;
	}
	void    ResetGame(long = 0L);
	void    SetBet(long);

	static  void ReleaseResources();
	static  void FlushInputEvents();

private:
	int     DealNewCard();
	void    ShowNewCard(int, int);
	void    SetHoldList(int);
	void    EnableBets();
	void    CheckWinningHand();
	int     Mod(int, int);
	void    OnSoundNotify(CSound *pSound);

protected:
	virtual bool OnCommand(WPARAM wParam, LPARAM lParam) override;

	//{{AFX_MSG( CMainPokerWindow )
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(unsigned int, CPoint);
	afx_msg void OnRButtonDown(unsigned int, CPoint);
	afx_msg void OnLButtonUp(unsigned int, CPoint);
	afx_msg void OnKeyDown(unsigned int, unsigned int, unsigned int);
	afx_msg void OnDrawItem(int, LPDRAWITEMSTRUCT);
	afx_msg bool OnEraseBkgnd(CDC *);
	afx_msg void OnActivate(unsigned int nState, CWnd   *pWndOther, bool bMinimized) override;
	afx_msg LRESULT OnMCINotify(WPARAM, LPARAM);
	afx_msg LRESULT OnMMIONotify(WPARAM, LPARAM);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

} // namespace Poker
} // namespace HodjNPodj
} // namespace Bagel

#endif
