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

#ifndef BAGEL_METAGAME_SAVES_SAVEGAME_INFO_H
#define BAGEL_METAGAME_SAVES_SAVEGAME_INFO_H

#include "common/serializer.h"
#include "bagel/hodjnpodj/metagame/bgen/item.h"
#include "bagel/hodjnpodj/metagame/bgen/invent.h"
#include "bagel/hodjnpodj/hnplibs/stdinc.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Saves {

#define MAX_GAME_TABLE 12   /* meta game execution table */
#define NUMBER_OF_CLUES 65

#define MAX_SAVEGAMES   6               // max number of saved game slots
#define MAX_PLAYERS     2
#define BOFFO_GAME_ID   0x01464F42      // "BOF1" <--- our first game

struct CLUE_ARRAY {
	bool bUsed = false;
	int iNoteID = 0;
	int iPlaceID = 0;
	int iPersonID = 0;

	void sync(Common::Serializer &s);
};

struct INVENTORY {
	char m_szTitle[40] = { '\0' };
	int m_aItemList[MG_OBJ_COUNT] = { 0 };
	int m_nItems = 0;

	void sync(Common::Serializer &s);
};

struct PLAYER_INFO {
	// These items are directly translated from CHodjPodj
	bool m_bMoving = false;			// flag: this player moving
	bool m_bComputer = false;		// flag: played by computer
	bool m_bHaveMishMosh = false;
	int  m_iSectorCode = 0;			// MG_SECTOR_xxxx
	int  m_iNode = 0;				// node # location
	int  m_iSkillLevel = 0;

	// list of clue numbers for clues given by winning mini-game
	int m_iWinInfoWon = 0;
	int m_iWinInfoNeed = 0;
	int m_iWinInfoTable[MAX_GAME_TABLE] = { 0 };

	// list of clue numbers for clues given by farmer, etc.
	int m_iSecondaryInfoWon = 0;
	int m_iSecondaryInfoNeed = 0;
	int m_iSecondaryInfoTable[MAX_GAME_TABLE] = { 0 };

	// list of objects required to get Mish/Mosh
	int m_iRequiredObjectsCount = 0;
	int m_iRequiredObjectsTable[MAX_GAME_TABLE] = { 0 };
	int m_iRequiredMoney = 0;		// money needed for Mish/Mosh

	// list of secondary information location we still have to visit
	int m_iSecondaryLoc[MAX_GAME_TABLE] = { 0 };

	int m_iGameHistory[20] = { 0 };		// last 20 mini-games played
	int m_iTargetLocation = 0;			// target location for computer play
	int m_iSpecialTravelCode = 0;
	int m_iNumberBoatTries = 0;
	int m_iFurlongs = 0;				// players remaining furlongs
	int m_nTurns = 0;					// players remaining turns

	// these items require special attention when saving or restoring a game
	long m_lCrowns = 0;					// from inventory

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
	unsigned long   m_lBoffoGameID = 0;
	uint16  m_nFixedRecordSize = 0;
	bool    m_bUsed = false;
	//	char    m_szSaveGameDescription[40];    // description of this saved game

	PLAYER_INFO m_stPlayerInfo[MAX_PLAYERS];

	bool    m_bTraps[240] = { false };		// table of used booby traps and narrations

	int     m_iGameTime = 0;		// SHORT_GAME, MEDIUM_GAME or LONG_GAME

	int     m_iMishMoshLoc = 0;		// MG_LOC_xxxx -- Mish/Mosh location

	byte   m_chJunk[80] = { 0 };	// for future use so the .SAV file size
	// does not have to change

	bool    m_bSoundEffectsEnabled = false;
	bool    m_bMusicEnabled = false;

	bool    m_bScrolling = false;

	bool    m_bNewMishMosh = false;     // true if frame should allocate new Mish/Mosh

	void sync(Common::Serializer &s);
};

} // namespace Saves
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
