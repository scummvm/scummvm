
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

#ifndef BAGEL_SPACEBAR_SRAF_COMPUTER_H
#define BAGEL_SPACEBAR_SRAF_COMPUTER_H

#include "bagel/boflib/gui/button.h"
#include "bagel/boflib/gui/list_box.h"
#include "bagel/boflib/gui/text_box.h"
#include "bagel/boflib/gui/dialog.h"
#include "bagel/boflib/file.h"
#include "bagel/baglib/storage_dev_win.h"

namespace Bagel {
namespace SpaceBar {

class SrafCompItem {
public:
	const char *m_pItem;
};

enum SELLERS {
	IRK4 = 0, YEEF8, QUOSH23
};

class DealSummarySellerItem {
public:
	SELLERS m_eSellerID;
	int m_nSellerOffer;
	const char *m_pSellerName;
};

enum BUYERS {
	PYLON3 = 0, DIPPIK10, VEBBIL18, GUNG14,
	REYES24, GILD13, HEM20, DORK44, RAWLEY23,
	JELLA37, SINJIN11, HUNDEY42, CHANDRA15, CLANG2,
	MINMARTZN, MINMARTBA, MINMARTRG, MINMARTUT,
	MINMARTPN, MINMARTSZ
};

enum STAFFERS {
	NORG72 = 0, PNURTH81, ZIG64, LENTIL24, VARGAS20, CHURG53, DURTEEN97
};

class DealSummaryBuyerItem {
public:
	BUYERS m_eBuyerID;
	int m_nBuyerOffer;
	const char *m_pBuyerName;
};

#define mOtherParty     0x0001
#define mBuyer          0x0002
#define mSeller         0x0004

// The ordering of these flags must match the ordering of these same
// characters in the array in which they are defined.
#define mNorg72         0x0008
#define mPnurth81       0x0010
#define mZig64          0x0020
#define mLentil24       0x0040
#define mVargas20       0x0080
#define mChurg53        0x0100

#define mStafferMale        0x0200
#define mStafferFemale      0x0400

class DispatchedTeamItem {
public:
	int m_nFlags = 0;
	int m_nMeetWithID = 0;
	int m_nDispatchTime = 0;
	int m_nTeamCaptain = 0;
	int m_nMeetingTime = 0;
};

class SrafTextScreen :
#if BOF_MAC
	public CBofWindow {
#else
	public CBofDialog {
#endif
public:
	SrafTextScreen();
	SrafTextScreen(CBofString &);
	virtual            ~SrafTextScreen();
	INT                 CreateTextScreen(CBofWindow *pParent = nullptr);
	VOID                DisplayTextScreen();

	VOID                OnBofButton(CBofObject *, INT);
	VOID                OnClose();

#if BOF_WINDOWS
	VOID                OnPaint(CBofRect * pRect);
#endif

private:
	CBofTextBox *m_pTextBox = nullptr;          // Bof text box for all kinds of cool info
	CBofBmpButton *m_pOKButton = nullptr;
	CBofFile *m_pTextFile = nullptr;
	CBofString *m_pszFileName = nullptr;
	CBofBitmap *m_pSaveBackground = nullptr;
	static CBofWindow *m_pSaveActiveWin;

#if BOF_MAC
	GrafPtr             m_pSavePort = nullptr;
#endif
};

// Number of constants...
#define NUM_MAIL_MESSAGES    8
#define NUM_MINERALS        14
#define NUM_STAFFERS        6
#define NUM_MAIN_ITEMS      11
#define NUM_SELLERS         3
#define NUM_BUYERS          20
#define NUM_OTHER_PARTYS    3
#define NUM_MUSICAL_SCORES  7
#define NUM_CODE_WORDS      10
#define NUM_SRAFCOMPBUTT    11
#define NUM_OFFERINGS       2

// Constants for get meet member

#define kOthersList     1
#define kSellersList    2
#define kBuyersList     3
#define kStaffersList   4

//	Sraffin text only screens (for stuff like biographies)

class SrafComputer : public CBagStorageDevWnd {

public:
	SrafComputer();
	virtual ~SrafComputer();
	virtual VOID OnKeyHit(uint32 lKey, uint32 nRepCount);
	virtual ERROR_CODE Attach();
	virtual ERROR_CODE Detach();
	VOID OnLButtonDown(uint32 nFlags, CBofPoint *xPoint, void * = nullptr);
	VOID OnLButtonUp(uint32 nFlags, CBofPoint *xPoint, void * = nullptr);
	VOID OnMainLoop();
	ERROR_CODE Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect = nullptr, INT /* nMaskColor */ = -1);
	VOID EraseBackdrop();
	VOID OnPaint(CBofRect *pRect);
	VOID OnBofButton(CBofObject *, INT);
	VOID OnBofListBox(CBofObject *pListBox, INT nItemIndex);
	VOID SetOn();
	VOID SetOff();
	VOID SetQuit();
	VOID AlignAtColumn(CBofString &sStr, const CHAR *szRightText, INT nAlignAt);
	CBofString &BuildBidString(INT index);

	VOID DeleteListBox();

	ERROR_CODE CreateListBox();
	ERROR_CODE CreateTextBox();

	VOID FillMain();

	VOID HideAllButtons();

	VOID ActivateDealSummary();
	VOID ActivateBuyerBids();
	VOID ActivateDealBackground();
	VOID ActivateSellerBios();
	VOID ActivateOtherBios();
	VOID ActivateStaffBios();
	VOID ActivateDispatchTeam();
	VOID ActivateCurrentEMail();
	VOID ActivateAudioSettings();
	VOID ActivateRoboButler();
	VOID ActivateCheckTeams();
	VOID ActivateCodeWords();

	VOID DeactivateMainScreen();
	VOID ActivateMainScreen();

	VOID InitDealSummary();
	VOID InitBuyerBids();
	VOID InitDealBackground();
	VOID InitSellerBios();
	VOID InitOtherBios();
	VOID InitStaffBios();
	VOID InitDispatchTeam();
	VOID InitCurrentEMail();
	VOID InitAudioSettings();
	VOID InitRoboButler();
	VOID InitCheckTeams();
	VOID InitCodeWords();

	VOID OnListMainScreen();
	VOID OnListDealSummary();
	VOID OnListBuyerBids();
	VOID OnListDealBackground();
	VOID OnListSellerBios();
	VOID OnListOtherBios();
	VOID OnListStaffBios();
	VOID OnListDispatchTeam();
	VOID OnListCurrentEMail();
	VOID OnListAudioSettings();
	VOID OnListRoboButler();
	VOID OnListCheckTeams();
	VOID OnListCodeWords();

	VOID OnButtonMainScreen(CBofButton *pButton, INT nState);
	VOID OnButtonDealSummary(CBofButton *pButton, INT nState);
	VOID OnButtonBuyerBids(CBofButton *pButton, INT nState);
	VOID OnButtonDealBackground(CBofButton *pButton, INT nState);
	VOID OnButtonSellerBios(CBofButton *pButton, INT nState);
	VOID OnButtonOtherBios(CBofButton *pButton, INT nState);
	VOID OnButtonStaffBios(CBofButton *pButton, INT nState);
	VOID OnButtonDispatchTeam(CBofButton *pButton, INT nState);
	VOID OnButtonCurrentEMail(CBofButton *pButton, INT nState);
	VOID OnButtonAudioSettings(CBofButton *pButton, INT nState);
	VOID OnButtonRoboButler(CBofButton *pButton, INT nState);
	VOID OnButtonCheckTeams(CBofButton *pButton, INT nState);
	VOID OnButtonCodeWords(CBofButton *pButton, INT nState);

	// biography support routines
	VOID DisplayTextScreen(CBofString &);

	VOID RecalcSellerSummaryList();
	VOID RecalcBuyerSummaryList();
	VOID RecalcDispatchList(INT);

	BOOL OnButtonSubmitOffer();

	//  support routines to return info about a list
	INT GetMeetMember(INT);
	INT GetAdjustedIndex(INT, INT, BOOL);

	// Dispatch support routines
	BOOL VerifyDispatchTeam();
	VOID NotifyBoss(CBofString &, INT);
	INT CalculateMeetingTime(INT nFlags);
	BOOL ReportMeetingStatus(INT);
	VOID SetMeetingResult(INT, INT, BOOL);

	INT GetTeamCaptain(INT nFlags);
	INT GetTeamGender(INT nFlags);

	// Robobutler service methods

	VOID DoOrderBeverage();
	VOID DoOrderSnack();
	VOID DoShowChowButtons();

	// To activate the main screen from outside the sraffan computer

	static VOID SetMainScreen();
	static CBofBitmap *GetComputerBackdrop();

	// User interaction

	VOID DisplayMessage(const CHAR *szMsg);

	// Finally, all done!
	VOID IncrementTurnCount();
	VOID DisplayTurnCount(INT);
	VOID OnButtonFinished(BOOL);

	// Save/Restore

	VOID SaveSraffanVars();
	VOID RestoreSraffanVars();

protected:
	enum SRAFCOMPMODE {
		SCON, SCOFF, SCDONE
	} m_eMode;
	enum SRAFCURSCREEN {
		SCMAIN, SCDEAL, SCBIDS, SCBACKGROUNDDATA,
		SCSELLER_BIOS, SCOTHER_BIOS, SCSTAFF_BIOS,
		SCDISPATCH, SCEMAIL, SCAUDIO, SCORDER,
		SCCHECK_TEAMS, SCCODE_WORDS
	} m_eCurScreen;
	CBofWindow *m_pLastActive;
	CBofBmpButton *m_pButtons[NUM_SRAFCOMPBUTT];
	RGBCOLOR m_cTextColor;
	RGBCOLOR m_cTextHiliteColor;
	RGBCOLOR m_cTextLineColor;

	CBofListBox *m_pLBox;               // Main screen list
	CBofList<SrafCompItem> *m_pMainList;
	CBofList<DealSummarySellerItem> *m_pSellerSummaryList;
	CBofList<DealSummaryBuyerItem> *m_pBuyerSummaryList;
	CBofList<DispatchedTeamItem> *m_pTeamList;

	CBofBitmap *m_pStafferBmp[NUM_STAFFERS + 1];    //  (one extra staffer, durteen)

	int m_nSelection;
	INT m_nListPointSize;
	INT m_nListItemHeight;
	SrafTextScreen *m_pTextOnlyScreen;

	// Code words

	CBofString *m_pszGroup1Word;
	CBofString *m_pszGroup2Word;
	BOOL                m_bSrafAttached;

	static BOOL m_bSwonzaEnlightened;
	static INT m_nStartingTime;

	static SrafComputer *m_pHead;               // Unique pointer to head of list

	static BOOL m_bRandomAudio;
	static INT m_nRandomTime;

	static BOOL m_bFailureNotified;
};

} // namespace SpaceBar
} // namespace Bagel

#endif
