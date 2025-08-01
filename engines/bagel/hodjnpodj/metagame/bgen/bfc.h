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

#include "common/serializer.h"
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
	CHodjPodj(void) {
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
	CBfcMgr(void) {
		memset(&m_cStartData, 0, &m_cEndData - &m_cStartData) ;
	}
};

//
// Save/Restore structures
//

#define MAX_SAVEGAMES   6               // max number of saved game slots
#define MAX_PLAYERS     2
#define BOFFO_GAME_ID   0x01464F42      // "BOF1" <--- our first game

struct CLUE_ARRAY {
	BOOL bUsed = false;
	INT iNoteID = 0;
	INT iPlaceID = 0;
	INT iPersonID = 0;

	void sync(Common::Serializer &s);
};

struct INVENTORY {
	CHAR m_szTitle[40] = { '\0' };
	INT m_aItemList[MG_OBJ_COUNT] = { 0 };
	INT m_nItems = 0;

	void sync(Common::Serializer &s);
};

struct PLAYER_INFO {
	// These items are directly translated from CHodjPodj
	BOOL m_bMoving = false;			// flag: this player moving
	BOOL m_bComputer = false;		// flag: played by computer
	BOOL m_bHaveMishMosh = false;
	INT  m_iSectorCode = 0;			// MG_SECTOR_xxxx
	INT  m_iNode = 0;				// node # location
	INT  m_iSkillLevel = 0;

	// list of clue numbers for clues given by winning mini-game
	INT m_iWinInfoWon = 0;
	INT m_iWinInfoNeed = 0;
	INT m_iWinInfoTable[MAX_GAME_TABLE] = { 0 };

	// list of clue numbers for clues given by farmer, etc.
	INT m_iSecondaryInfoWon = 0;
	INT m_iSecondaryInfoNeed = 0;
	INT m_iSecondaryInfoTable[MAX_GAME_TABLE] = { 0 };

	// list of objects required to get Mish/Mosh
	INT m_iRequiredObjectsCount = 0;
	INT m_iRequiredObjectsTable[MAX_GAME_TABLE] = { 0 };
	INT m_iRequiredMoney = 0;		// money needed for Mish/Mosh

	// list of secondary information location we still have to visit
	INT m_iSecondaryLoc[MAX_GAME_TABLE] = { 0 };

	INT m_iGameHistory[20] = { 0 };		// last 20 mini-games played
	INT m_iTargetLocation = 0;			// target location for computer play
	INT m_iSpecialTravelCode = 0;
	INT m_iNumberBoatTries = 0;
	INT m_iFurlongs = 0;				// players remaining furlongs
	INT m_nTurns = 0;					// players remaining turns

	// these items require special attention when saving or restoring a game
	LONG m_lCrowns = 0;					// from inventory

	CLUE_ARRAY m_bClueArray[NUMBER_OF_CLUES]; // array of clues for the notebook
	INVENTORY m_stInventory;
	INVENTORY m_stGenStore;
	INVENTORY m_stBlackMarket;
	INVENTORY m_stTradingPost;

	void sync(Common::Serializer &s);
};

/**
 * Structure for the save game file format
 */
struct SAVEGAME_INFO {
	ULONG   m_lBoffoGameID = 0;
	USHORT  m_nFixedRecordSize = 0;
	BOOL    m_bUsed = false;
//	CHAR    m_szSaveGameDescription[40];    // description of this saved game

	PLAYER_INFO m_stPlayerInfo[MAX_PLAYERS];

	BOOL    m_bTraps[240] = { false };		// table of used booby traps and narrations

	INT     m_iGameTime = 0;		// SHORT_GAME, MEDIUM_GAME or LONG_GAME

	INT     m_iMishMoshLoc = 0;		// MG_LOC_xxxx -- Mish/Mosh location

	UBYTE   m_chJunk[80] = { 0 };	// for future use so the .SAV file size
	// does not have to change

	BOOL    m_bSoundEffectsEnabled = false;
	BOOL    m_bMusicEnabled = false;

	BOOL    m_bScrolling = false;

	BOOL    m_bNewMishMosh = false;     // TRUE if frame should allocate new Mish/Mosh

	void sync(Common::Serializer &s);
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
