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

#ifndef HODJNPODJ_METAGAME_BGEN_BFC_H
#define HODJNPODJ_METAGAME_BGEN_BFC_H

#include "common/scummsys.h"
#include "bagel/hodjnpodj/metagame/bgen/invent.h"
#include "bagel/hodjnpodj/metagame/bgen/note.h"
#include "bagel/hodjnpodj/globals.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#define RETAIN_META_DLL             false       // Whether Meta DLL is kept in memory
#define ANIMATION_TIMER_ID          10          // Animation timer identifier
#define ANIMATION_TIMER_INTERVAL    200         // Animation timer interval

#define SHORT_GAME      0
#define MEDIUM_GAME     1
#define LONG_GAME       2
#define ITEMS_IN_BLACK_MARKET 12
// Meta game execution table
#define MAX_GAME_TABLE 12
#define NUMBER_OF_CLUES 65

enum SkillLevel {
	SKILLLEVEL_LOW = 0,
	SKILLLEVEL_MEDIUM = 1,
	SKILLLEVEL_HIGH = 2
};

struct GAMESTRUCT {
	long lCrowns;
	long lScore;
	SkillLevel nSkillLevel;
	bool bSoundEffectsEnabled;
	bool bMusicEnabled;
	bool bPlayingMetagame;
	bool bPlayingHodj;

	void clear() {
		lCrowns = lScore = 0;
		nSkillLevel = SKILLLEVEL_LOW;
		bSoundEffectsEnabled = bMusicEnabled = true;
		bPlayingMetagame = false;
		bPlayingHodj = true;
	}
	GAMESTRUCT() {
		clear();
	}
};

struct NOTE_LIST {
	CNote *pNote;
	bool   bUsed;
};

// CHodjPodj class -- information about Hodj or Podj
class CHodjPodj {
public:
	char m_cStartData;

	bool m_bMoving;			// Flag: this player moving
	bool m_bComputer;		// Flag: played by computer
	bool m_bHaveMishMosh;

	int m_iSectorCode;		// MG_SECTOR_xxxx
	int m_iNode;			// Node # location
	int m_iSkillLevel;
	int	m_iTargetNode;		// Current destinations node #

	// List of clue numbers for clues given by winning mini-game
	int m_iWinInfoWon;
	int m_iWinInfoNeed;
	int m_iWinInfoTable[MAX_GAME_TABLE];

	// List of clue numbers for clues given by farmer, etc.
	int m_iSecondaryInfoWon;
	int m_iSecondaryInfoNeed;
	int m_iSecondaryInfoTable[MAX_GAME_TABLE];

	// List of objects required to get Mish/Mosh
	int m_iRequiredObjectsCount;
	int m_iRequiredObjectsTable[MAX_GAME_TABLE];
	int m_iRequiredMoney;		// Money needed for Mish/Mosh

	// List of secondary information location we still have to visit
	int m_iSecondaryLoc[MAX_GAME_TABLE];

	int m_iGameHistory[20];		// Last 20 mini-games played
	int m_iTargetLocation;		// Target location for computer play
	int m_iSpecialTravelCode;
	int m_iNumberBoatTries;
	int m_iFurlongs;			// Number of furlongs left for players turn
	int m_nTurns;				// Number of turns (not furlongs)

	// Inventory information
	// These fields are valid if m_bInventories is TRUE, even if
	// m_bRestart is false
	CInventory *m_pInventory;	// Inventory player owns
	CInventory *m_pGenStore;	// Player's inventory in general store
	CInventory *m_pBlackMarket;	// Player's inventory in pawn shop
	CInventory *m_pTradingPost;	// Player's inventory in trading post

	NOTE_LIST   m_aClueArray[NUMBER_OF_CLUES]; // Array of clues for the notebook

	char m_cEndData;

	// constructor
	CHodjPodj() {
		memset(&m_cStartData, 0, &m_cEndData - &m_cStartData);
	}
};

// CBfcMgr -- Boffo games interface manager class
class CBfcMgr {
public:
	char m_cStartData;

	bool m_bTraps[240];		// Used booby-trap/narration flags

	bool m_bLowMemory;		// Low memory environment flag

	bool m_bSlowCPU;		// 80386 processor flag

	bool m_bInventories;	// Use the inventory pointers even
							// if m_bRestart is false to indicate that some
							// fields are still value (as noted)
	int m_iGameTime;        // SHORT_GAME, MEDIUM_GAME or LONG_GAME
							// Note: This value is valid if
							// m_bInventories is TRUE
	bool m_bRestart;		// (set by EXE for DLL) restarting game
							// (otherwise, remaining data is invalid, unless
							// otherwise noted)
	bool m_bGameOver;		// If set, the meta-game is over (someone
							// has won MishMosh and is in castle)
	bool m_bScrolling;      // TRUE if scrolling map while walking

	bool m_bAnimations;		// TRUE if showing animations

	bool m_bRestoredGame;   // TRUE if restoring a saved game

	bool m_bVisitedStore;   // TRUE if computer Hodj/Podj visited a store

	int m_iFunctionCode;	// (set by DLL for EXE) MG_GAME_xxxx to
							// indicate that a game is to be played,
							// or MG_DLLX_xxxx to indicate other DLL
							// exit action

	bool m_bChanged;        // Save Game check to determine if we need to
							// ask about saving users game.
							// This field does NOT need to be save in the
							// .SAV file.

	uint32 m_dwFreeSpaceMargin;
	uint32 m_dwFreePhysicalMargin;

	int	m_nInstallationCode;
	char m_chHomePath[PATHSPECSIZE];
	char m_chCDPath[PATHSPECSIZE];
	char m_chMiniPath[PATHSPECSIZE];

	GAMESTRUCT m_stGameStruct;
	int m_iMishMoshLoc;    // MG_LOC_xxxx -- Mish/Mosh location

	// ... need color narration information from Barbara

	class CHodjPodj m_cHodj, m_cPodj;  // Hodj/Podj info

	// these items must be created in the Frame EXE
	CItem *m_pMishItem;
	CItem *m_pMoshItem;

	char m_cEndData;

	// constructor
	CBfcMgr(void) {
		memset(&m_cStartData, 0, &m_cEndData - &m_cStartData);
	}
};

//
// Save/Restore structures
//

#define MAX_SAVEGAMES   6               // max number of saved game slots
#define MAX_PLAYERS     2
#define BOFFO_GAME_ID   0x01464F42      // "BOF1" <--- our first game

struct CLUE_ARRAY {
	bool bUsed;
	int iNoteID;
	int iPlaceID;
	int iPersonID;
};

struct INVENTORY {
	char m_szTitle[40];
	int  m_aItemList[MG_OBJ_COUNT];
	int  m_nItems;
};

struct PLAYER_INFO {
	// These items are directly translated from CHodjPodj
	bool m_bMoving;         // Flag: this player moving
	bool m_bComputer;       // Flag: played by computer
	bool m_bHaveMishMosh;
	int  m_iSectorCode;     // MG_SECTOR_xxxx
	int  m_iNode;           // Node # location
	int  m_iSkillLevel;

	// List of clue numbers for clues given by winning mini-game
	int m_iWinInfoWon;
	int m_iWinInfoNeed;
	int m_iWinInfoTable[MAX_GAME_TABLE];

	// List of clue numbers for clues given by farmer, etc.
	int m_iSecondaryInfoWon;
	int m_iSecondaryInfoNeed;
	int m_iSecondaryInfoTable[MAX_GAME_TABLE];

	// List of objects required to get Mish/Mosh
	int m_iRequiredObjectsCount;
	int m_iRequiredObjectsTable[MAX_GAME_TABLE];
	int m_iRequiredMoney;      // Money needed for Mish/Mosh

	// List of secondary information location we still have to visit
	int m_iSecondaryLoc[MAX_GAME_TABLE];

	int m_iGameHistory[20];     // Last 20 mini-games played
	int m_iTargetLocation;      // Target location for computer play
	int m_iSpecialTravelCode;
	int m_iNumberBoatTries;
	int m_iFurlongs;            // Players remaining furlongs
	int m_nTurns;               // Players remaining turns

	// These items require special attention when saving or restoring a game

	long m_lCrowns;             // From inventory

	CLUE_ARRAY m_bClueArray[NUMBER_OF_CLUES]; // Array of clues for the notebook

	INVENTORY m_stInventory;
	INVENTORY m_stGenStore;
	INVENTORY m_stBlackMarket;
	INVENTORY m_stTradingPost;

	byte      m_chJunk[20];     // For future use so the .SAV file size
								// does not have to change
};


// Structure for the save game file format
struct SAVEGAME_INFO {
	uint32  m_lBoffoGameID;
	uint16  m_nFixedRecordSize;
	bool    m_bUsed;

	char    m_szSaveGameDescription[40];    // Description of this saved game

	PLAYER_INFO m_stPlayerInfo[MAX_PLAYERS];

	bool    m_bTraps[240];		// Table of used booby traps and narrations

	int     m_iGameTime;		// SHORT_GAME, MEDIUM_GAME or LONG_GAME

	int     m_iMishMoshLoc;		// MG_LOC_xxxx -- Mish/Mosh location

	byte   m_chJunk[80];		// For future use so the .SAV file size
								// does not have to change

	bool    m_bSoundEffectsEnabled;
	bool    m_bMusicEnabled;

	bool    m_bScrolling;

	bool    m_bNewMishMosh;     // true if frame should allocate new Mish/Mosh
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
