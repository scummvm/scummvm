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

#ifndef BAGEL_METAGAME_BGEN_BFC_H
#define BAGEL_METAGAME_BGEN_BFC_H

#include "bagel/hodjnpodj/metagame/bgen/mgstat.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/hodjnpodj/metagame/bgen/item.h"
#include "bagel/hodjnpodj/metagame/bgen/invent.h"
#include "bagel/hodjnpodj/hnplibs/stdinc.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#define RETAIN_META_DLL             FALSE       // whether Meta DLL is kept in memory
#define ANIMATION_TIMER_ID          10          // animation timer identifier
#define ANIMATION_TIMER_INTERVAL    200         // animation timer interval

#define SHORT_GAME      0
#define MEDIUM_GAME     1
#define LONG_GAME       2
#define ITEMS_IN_BLACK_MARKET 12
#define MAX_GAME_TABLE 12   /* meta game execution table */
#define NUMBER_OF_CLUES 65

struct NOTE_LIST {
	CNote *pNote;
	BOOL   bUsed;
};

// CHodjPodj class -- information about Hodj or Podj
class CHodjPodj {
public:
	char m_cStartData ;

	BOOL m_bMoving ;        // flag: this player moving
	BOOL m_bComputer ;      // flag: played by computer
	BOOL m_bHaveMishMosh;

	int m_iSectorCode ;     // MG_SECTOR_xxxx
	int m_iNode ;           // node # location
	int m_iSkillLevel;
	int m_iTargetNode;      // current destinations node #

	// list of clue numbers for clues given by winning mini-game
	int m_iWinInfoWon ;
	int m_iWinInfoNeed ;
	int m_iWinInfoTable[MAX_GAME_TABLE] ;

	// list of clue numbers for clues given by farmer, etc.
	int m_iSecondaryInfoWon ;
	int m_iSecondaryInfoNeed ;
	int m_iSecondaryInfoTable[MAX_GAME_TABLE] ;

	// list of objects required to get Mish/Mosh
	int m_iRequiredObjectsCount ;
	int m_iRequiredObjectsTable[MAX_GAME_TABLE] ;
	int m_iRequiredMoney ;      // money needed for Mish/Mosh

	// list of secondary information location we still have to visit
	int m_iSecondaryLoc[MAX_GAME_TABLE] ;

	int m_iGameHistory[20] ;    // last 20 mini-games played
	int m_iTargetLocation ;     // target location for computer play
	int m_iSpecialTravelCode;
	int m_iNumberBoatTries;
	int m_iFurlongs;            // number of furlongs left for players turn
	int m_nTurns;               // number of turns (not furlongs)

	// inventory information
	// these fields are valid if m_bInventories is TRUE, even if
	// m_bRestart is FALSE
	CInventory  *m_pInventory;  // inventory player owns
	CInventory  *m_pGenStore;   // player's inventory in general store
	CInventory  *m_pBlackMarket;   // player's inventory in pawn shop
	CInventory  *m_pTradingPost;   // player's inventory in trading post

	NOTE_LIST   m_aClueArray[NUMBER_OF_CLUES]; // array of clues for the notebook

	char m_cEndData ;

	// constructor
	CHodjPodj::CHodjPodj(void) {
		memset(&m_cStartData, 0, &m_cEndData - &m_cStartData);
	}

} ;

// CBfcMgr -- boffo games interface manager class
class CBfcMgr {
public:
	char m_cStartData;

	BOOL m_bTraps[240];     // used booby-trap/narration flags

	BOOL m_bLowMemory;      // low memory environment flag

	BOOL m_bSlowCPU;        // 80386 processor flag

	BOOL m_bInventories;    // use the inventory pointers even
	// if m_bRestart is FALSE to indicate that some
	// fields are still value (as noted)
	int m_iGameTime;        // SHORT_GAME, MEDIUM_GAME or LONG_GAME
	// Note: This value is valid if
	// m_bInventories is TRUE
	BOOL m_bRestart ;       // (set by EXE for DLL) restarting game
	// (otherwise, remaining data is invalid, unless
	// otherwise noted)
	BOOL m_bGameOver ;      // if set, the meta-game is over (someone
	// has won MishMosh and is in castle)
	BOOL m_bScrolling;      // TRUE if scrolling map while walking

	BOOL m_bAnimations;     // TRUE if showing animations

	BOOL m_bRestoredGame;   // TRUE if restoring a saved game

	BOOL m_bVisitedStore;   // TRUE if computer Hodj/Podj visited a store

	int m_iFunctionCode ;   // (set by DLL for EXE) MG_GAME_xxxx to
	// indicate that a game is to be played,
	// or MG_DLLX_xxxx to indicate other DLL
	// exit action

	BOOL m_bChanged;        // Save Game check to determine if we need to
	// ask about saving users game.
	// This field does NOT need to be save in the
	// .SAV file.

	DWORD   m_dwFreeSpaceMargin;
	DWORD   m_dwFreePhysicalMargin;

	int m_nInstallationCode;
	char m_chHomePath[PATHSPECSIZE];
	char m_chCDPath[PATHSPECSIZE];
	char m_chMiniPath[PATHSPECSIZE];

	struct GAMESTRUCT m_stGameStruct ;
	int m_iMishMoshLoc ;    // MG_LOC_xxxx -- Mish/Mosh location

	// ... need color narration information from Barbara

	class CHodjPodj m_cHodj, m_cPodj ;  // Hodj/Podj info

	// these items must be created in the Frame EXE
	CItem *m_pMishItem;
	CItem *m_pMoshItem;

	char m_cEndData ;

	// constructor
	CBfcMgr::CBfcMgr(void) {
		memset(&m_cStartData, 0, &m_cEndData - &m_cStartData) ;
	}
};

//
// Save/Restore structures
//

#define MAX_SAVEGAMES   6               // max number of saved game slots
#define MAX_PLAYERS     2
#define BOFFO_GAME_ID   0x01464F42      // "BOF1" <--- our first game

typedef struct {

	BOOL bUsed;
	INT iNoteID;
	INT iPlaceID;
	INT iPersonID;

} CLUE_ARRAY;

typedef struct {
	CHAR    m_szTitle[40];
	INT     m_aItemList[MG_OBJ_COUNT];
	INT     m_nItems;

} INVENTORY;

typedef struct {

	// these items are directly translated from CHodjPodj
	//

	BOOL m_bMoving;         // flag: this player moving
	BOOL m_bComputer;       // flag: played by computer
	BOOL m_bHaveMishMosh;
	INT  m_iSectorCode;     // MG_SECTOR_xxxx
	INT  m_iNode;           // node # location
	INT  m_iSkillLevel;

	// list of clue numbers for clues given by winning mini-game
	INT m_iWinInfoWon;
	INT m_iWinInfoNeed;
	INT m_iWinInfoTable[MAX_GAME_TABLE];

	// list of clue numbers for clues given by farmer, etc.
	INT m_iSecondaryInfoWon;
	INT m_iSecondaryInfoNeed;
	INT m_iSecondaryInfoTable[MAX_GAME_TABLE];

	// list of objects required to get Mish/Mosh
	INT m_iRequiredObjectsCount;
	INT m_iRequiredObjectsTable[MAX_GAME_TABLE];
	INT m_iRequiredMoney;      // money needed for Mish/Mosh

	// list of secondary information location we still have to visit
	INT m_iSecondaryLoc[MAX_GAME_TABLE];

	INT m_iGameHistory[20];     // last 20 mini-games played
	INT m_iTargetLocation;      // target location for computer play
	INT m_iSpecialTravelCode;
	INT m_iNumberBoatTries;
	INT m_iFurlongs;            // players remaining furlongs
	INT m_nTurns;               // players remaining turns

	// these items require special attention when saving or restoring a game
	//

	LONG m_lCrowns;             // from inventory

	CLUE_ARRAY m_bClueArray[NUMBER_OF_CLUES]; // array of clues for the notebook

	INVENTORY m_stInventory;
	INVENTORY m_stGenStore;
	INVENTORY m_stBlackMarket;
	INVENTORY m_stTradingPost;

	UBYTE     m_chJunk[20];     // for future use so the .SAV file size
	// does not have to change

} PLAYER_INFO;


// structure for the save game file format
//
typedef struct {

	ULONG   m_lBoffoGameID;
	USHORT  m_nFixedRecordSize;
	BOOL    m_bUsed;

	CHAR    m_szSaveGameDescription[40];    // description of this saved game

	PLAYER_INFO m_stPlayerInfo[MAX_PLAYERS];

	BOOL    m_bTraps[240];      // table of used booby traps and narrations

	INT     m_iGameTime;        // SHORT_GAME, MEDIUM_GAME or LONG_GAME

	INT     m_iMishMoshLoc;     // MG_LOC_xxxx -- Mish/Mosh location

	UBYTE   m_chJunk[80];       // for future use so the .SAV file size
	// does not have to change

	BOOL    m_bSoundEffectsEnabled;
	BOOL    m_bMusicEnabled;

	BOOL    m_bScrolling;

	BOOL    m_bNewMishMosh;     // TRUE if frame should allocate new Mish/Mosh

} SAVEGAME_INFO;

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
