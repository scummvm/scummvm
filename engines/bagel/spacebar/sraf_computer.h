
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
	const char *_pItem;
};

enum SELLERS {
	IRK4 = 0, YEEF8, QUOSH23
};

class DealSummarySellerItem {
public:
	SELLERS _eSellerID;
	int _nSellerOffer;
	const char *_pSellerName;
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
	BUYERS _eBuyerID;
	int _nBuyerOffer;
	const char *_pBuyerName;
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
	int _nFlags = 0;
	int _nMeetWithID = 0;
	int _nDispatchTime = 0;
	int _nTeamCaptain = 0;
	int _nMeetingTime = 0;
};

class SrafTextScreen : public CBofDialog {
private:
	static CBofWindow *_pSaveActiveWin;

	CBofTextBox *_pTextBox = nullptr;          // Bof text box for all kinds of cool info
	CBofBmpButton *_pOKButton = nullptr;
	CBofString _text;

	CBofRect gCompDisplay;
	CBofRect gTextWindow;
	CBofRect gStafferDisplay;
	CBofRect gSrafTextWindow;

public:
	SrafTextScreen(const CBofString &str, bool isText = false);
	virtual ~SrafTextScreen();
	int createTextScreen(CBofWindow *pParent = nullptr);
	void displayTextScreen();

	void onBofButton(CBofObject *, int) override;
	void onClose() override;
	void onPaint(CBofRect * pRect) override;
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
	void onKeyHit(uint32 lKey, uint32 nRepCount) override;
	ErrorCode attach() override;
	ErrorCode detach() override;
	void onLButtonDown(uint32 nFlags, CBofPoint *xPoint, void * = nullptr) override;
	void onLButtonUp(uint32 nFlags, CBofPoint *xPoint, void * = nullptr) override;
	void onMainLoop() override;
	void onPaint(CBofRect *pRect) override;
	void onBofButton(CBofObject *, int) override;
	void onBofListBox(CBofObject *pListBox, int nItemIndex) override;
	void setOn();
	void setOff();
	void setQuit();
	void alignAtColumn(CBofString &sStr, const char *szRightText, int nAlignAt);
	CBofString &buildBidString(int index);

	void deleteListBox();

	ErrorCode createListBox();

	void fillMain();

	void hideAllButtons();

	void activateDealSummary();
	void activateBuyerBids();
	void activateDealBackground();
	void activateSellerBios();
	void activateOtherBios();
	void activateStaffBios();
	void activateDispatchTeam();
	void activateCurrentEMail();
	void activateAudioSettings();
	void activateRoboButler();
	void activateCheckTeams();
	void activateCodeWords();

	void deactivateMainScreen();
	void activateMainScreen();

	void initDealSummary();
	void initBuyerBids();
	void initDealBackground();
	void initSellerBios();
	void initOtherBios();
	void initStaffBios();
	void initDispatchTeam();
	void initCurrentEMail();
	void initAudioSettings();
	void initRoboButler();

	void onListMainScreen();
	void onListDealSummary();
	void onListBuyerBids();
	void onListDealBackground();
	void onListSellerBios();
	void onListOtherBios();
	void onListStaffBios();
	void onListDispatchTeam();
	void onListCurrentEMail();
	void onListAudioSettings();
	void onListRoboButler();
	void onListCheckTeams();
	void onListCodeWords();

	void onButtonMainScreen(CBofButton *pButton, int nState);
	void onButtonDealSummary(CBofButton *pButton, int nState);
	void onButtonBuyerBids(CBofButton *pButton, int nState);
	void onButtonDealBackground(CBofButton *pButton, int nState);
	void onButtonSellerBios(CBofButton *pButton, int nState);
	void onButtonOtherBios(CBofButton *pButton, int nState);
	void onButtonStaffBios(CBofButton *pButton, int nState);
	void onButtonDispatchTeam(CBofButton *pButton, int nState);
	void onButtonCurrentEMail(CBofButton *pButton, int nState);
	void onButtonAudioSettings(CBofButton *pButton, int nState);
	void onButtonRoboButler(CBofButton *pButton, int nState);
	void onButtonCheckTeams(CBofButton *pButton, int nState);
	void onButtonCodeWords(CBofButton *pButton, int nState);

	// biography support routines
	void displayTextScreen(CBofString &);

	void recalcSellerSummaryList();
	void recalcBuyerSummaryList();
	void recalcDispatchList(int);

	bool onButtonSubmitOffer();

	//  support routines to return info about a list
	int getMeetMember(int);

	/**
	 * Search the specified list and return the index of a true member, if there
	 * is one, return the element in the list that has "meet with" set to true.
	 */
	int getAdjustedIndex(int, int, bool);

	// Dispatch support routines
	bool verifyDispatchTeam();
	void notifyBoss(CBofString &, int);
	int calculateMeetingTime(int nFlags);
	bool reportMeetingStatus(int);

	/**
	 * Search the specified list and return the index of a true member, if there
	 * is one, return the element in the list that has "meet with" set to true.
	 */
	void setMeetingResult(int, int, bool);

	int getTeamCaptain(int nFlags);
	int getTeamGender(int nFlags);

	// Robobutler service methods

	void doOrderBeverage();
	void doOrderSnack();
	void doShowChowButtons();

	// To activate the main screen from outside the sraffan computer

	static void setMainScreen();
	static CBofBitmap *getComputerBackdrop();

	// User interaction

	void displayMessage(const char *szMsg);

	// Finally, all done!
	void incrementTurnCount();


	/**
	 * This dynamically updates the turncount on the screen,
	 * must know which line to update though.
	 */
	void displayTurnCount(int);
	void onButtonFinished(bool);

	// Save/Restore

	void saveSraffanVars();
	void restoreSraffanVars();

protected:
	enum SrafCompModeC {
		SC_ON, SC_OFF, SC_DONE
	} _eMode;
	
	enum SrafCurScreen {
		SC_MAIN, SC_DEAL, SC_BIDS, SC_BACKGROUND_DATA,
		SC_SELLER_BIOS, SC_OTHER_BIOS, SC_STAFF_BIOS,
		SC_DISPATCH, SC_EMAIL, SC_AUDIO, SC_ORDER,
		SC_CHECK_TEAMS, SC_CODE_WORDS
	} _eCurScreen;
	
	CBofBmpButton *_pButtons[NUM_SRAFCOMPBUTT];
	RGBCOLOR _cTextColor;
	RGBCOLOR _cTextHiliteColor;
	RGBCOLOR _cTextLineColor;

	CBofListBox *_pLBox;               // Main screen list
	CBofList<SrafCompItem> *_pMainList;
	CBofList<DealSummarySellerItem> *_pSellerSummaryList;
	CBofList<DealSummaryBuyerItem> *_pBuyerSummaryList;
	CBofList<DispatchedTeamItem> *_pTeamList;

	CBofBitmap *_pStafferBmp[NUM_STAFFERS + 1];    //  (one extra staffer, durteen)

	int _nSelection;
	int _nListPointSize;
	int _nListItemHeight;
	SrafTextScreen *_pTextOnlyScreen;

	// Code words

	CBofString *_pszGroup1Word;
	CBofString *_pszGroup2Word;
	bool _bSrafAttached;

	static bool _bSwonzaEnlightened;
	static int _nStartingTime;

	static SrafComputer *_pHead;               // Unique pointer to head of list

	static bool _bRandomAudio;
	static int _nRandomTime;

	static bool _bFailureNotified;

private:
	CBofRect gCompDisplay;
	CBofRect gTextWindow;
	CBofRect gStafferDisplay;
	CBofRect gSrafTextWindow;
	CBofString gBidStr;
};

} // namespace SpaceBar
} // namespace Bagel

#endif
