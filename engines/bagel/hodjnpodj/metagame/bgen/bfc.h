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
#include "bagel/hodjnpodj/metagame/saves/savegame_info.h"
#include "bagel/hodjnpodj/hnplibs/stdinc.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#define RETAIN_META_DLL             false       // whether Meta DLL is kept in memory
#define ANIMATION_TIMER_ID          10          // animation timer identifier
#define ANIMATION_TIMER_INTERVAL    200         // animation timer interval

#define SHORT_GAME      0
#define MEDIUM_GAME     1
#define LONG_GAME       2
#define ITEMS_IN_BLACK_MARKET 12

struct NOTE_LIST {
	CNote *pNote;
	bool   bUsed;
};

// CHodjPodj class -- information about Hodj or Podj
class CHodjPodj {
public:
	bool m_bMoving = false;					// flag: this player moving
	bool m_bComputer = false;				// flag: played by computer
	bool m_bHaveMishMosh = false;

	int m_iSectorCode = 0;					// MG_SECTOR_xxxx
	int m_iNode = 0;						// node # location
	int m_iSkillLevel = 0;
	int m_iTargetNode = 0;					// current destinations node #

	// list of clue numbers for clues given by winning mini-game
	int m_iWinInfoWon = 0;
	int m_iWinInfoNeed = 0;
	int m_iWinInfoTable[MAX_GAME_TABLE] = {};

	// list of clue numbers for clues given by farmer, etc.
	int m_iSecondaryInfoWon = 0;
	int m_iSecondaryInfoNeed = 0;
	int m_iSecondaryInfoTable[MAX_GAME_TABLE] = {};

	// list of objects required to get Mish/Mosh
	int m_iRequiredObjectsCount = 0;
	int m_iRequiredObjectsTable[MAX_GAME_TABLE] = {};
	int m_iRequiredMoney = 0;				// money needed for Mish/Mosh

	// list of secondary information location we still have to visit
	int m_iSecondaryLoc[MAX_GAME_TABLE] = {};

	int m_iGameHistory[20] = {};			// last 20 mini-games played
	int m_iTargetLocation = 0;				// target location for computer play
	int m_iSpecialTravelCode = 0;
	int m_iNumberBoatTries = 0;
	int m_iFurlongs = 0;					// number of furlongs left for players turn
	int m_nTurns = 0;						// number of turns (not furlongs)

	// inventory information
	// these fields are valid if m_bInventories is true, even if
	// m_bRestart is false
	CInventory  *m_pInventory = nullptr;	// inventory player owns
	CInventory  *m_pGenStore = nullptr;		// player's inventory in general store
	CInventory  *m_pBlackMarket = nullptr;	// player's inventory in pawn shop
	CInventory  *m_pTradingPost = nullptr;	// player's inventory in trading post

	NOTE_LIST   m_aClueArray[NUMBER_OF_CLUES] = {};	// array of clues for the notebook
} ;

// CBfcMgr -- boffo games interface manager class
class CBfcMgr {
public:
	bool m_bTraps[240] = {};				// used booby-trap/narration flags

	bool m_bLowMemory = false;				// low memory environment flag

	bool m_bSlowCPU = false;				// 80386 processor flag

	bool m_bInventories = false;			// use the inventory pointers even
	// if m_bRestart is false to indicate that some
	// fields are still value (as noted)
	int m_iGameTime = 0;					// SHORT_GAME, MEDIUM_GAME or LONG_GAME
	// Note: This value is valid if
	// m_bInventories is true
	bool m_bRestart = false;				// (set by EXE for DLL) restarting game
	// (otherwise, remaining data is invalid, unless
	// otherwise noted)
	bool m_bGameOver  = false;				// if set, the meta-game is over (someone
	// has won MishMosh and is in castle)
	bool m_bScrolling = false;				// true if scrolling map while walking

	bool m_bAnimations = false;				// true if showing animations

	bool m_bRestoredGame = false;			// true if restoring a saved game

	bool m_bVisitedStore = false;			// true if computer Hodj/Podj visited a store

	int m_iFunctionCode = 0;				// (set by DLL for EXE) MG_GAME_xxxx to
	// indicate that a game is to be played,
	// or MG_DLLX_xxxx to indicate other DLL
	// exit action

	bool m_bChanged = false;				// Save Game check to determine if we need to
	// ask about saving users game.
	// This field does NOT need to be save in the
	// .SAV file.

	uint32   m_dwFreeSpaceMargin = 0;
	uint32   m_dwFreePhysicalMargin = 0;

	int m_nInstallationCode = 0;
	char m_chHomePath[PATHSPECSIZE] = {};
	char m_chCDPath[PATHSPECSIZE] = {};
	char m_chMiniPath[PATHSPECSIZE] = {};

	struct GAMESTRUCT m_stGameStruct ;
	int m_iMishMoshLoc = 0;    // MG_LOC_xxxx -- Mish/Mosh location

	// ... need color narration information from Barbara

	class CHodjPodj m_cHodj, m_cPodj ;  // Hodj/Podj info

	// these items must be created in the Frame EXE
	CItem *m_pMishItem = nullptr;
	CItem *m_pMoshItem = nullptr;
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
