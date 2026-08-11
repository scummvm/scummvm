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

#include "bagel/hodjnpodj/metagame/bgen/bfc.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Saves {

void CLUE_ARRAY::sync(Common::Serializer &s) {
	s.syncAsByte(bUsed);
	s.syncAsSint16LE(iNoteID);
	s.syncAsSint16LE(iPlaceID);
	s.syncAsSint16LE(iPersonID);
}

void INVENTORY::sync(Common::Serializer &s) {
	s.syncBytes((byte *)m_szTitle, 40);
	for (int i = 0; i < MG_OBJ_COUNT; ++i)
		s.syncAsSint16LE(m_aItemList[i]);
	s.syncAsSint16LE(m_nItems);
}

void PLAYER_INFO::sync(Common::Serializer &s) {

	s.syncAsByte(m_bMoving);
	s.syncAsByte(m_bComputer);
	s.syncAsByte(m_bHaveMishMosh);
	s.syncAsSint16LE(m_iSectorCode);
	s.syncAsSint16LE(m_iNode);
	s.syncAsSint16LE(m_iSkillLevel);

	s.syncAsSint16LE(m_iWinInfoWon);
	s.syncAsSint16LE(m_iWinInfoNeed);
	for (int i = 0; i < MAX_GAME_TABLE; ++i)
		s.syncAsSint16LE(m_iWinInfoTable[i]);

	s.syncAsSint16LE(m_iSecondaryInfoWon);
	s.syncAsSint16LE(m_iSecondaryInfoNeed);
	for (int i = 0; i < MAX_GAME_TABLE; ++i)
		s.syncAsSint16LE(m_iSecondaryInfoTable[i]);

	s.syncAsSint16LE(m_iRequiredObjectsCount);
	for (int i = 0; i < MAX_GAME_TABLE; ++i)
		s.syncAsSint16LE(m_iRequiredObjectsTable[i]);
	s.syncAsSint16LE(m_iRequiredMoney);

	for (int i = 0; i < MAX_GAME_TABLE; ++i)
		s.syncAsSint16LE(m_iSecondaryLoc[i]);

	for (int i = 0; i < 20; ++i)
		s.syncAsSint16LE(m_iGameHistory[i]);
	s.syncAsSint16LE(m_iTargetLocation);
	s.syncAsSint16LE(m_iSpecialTravelCode);
	s.syncAsSint16LE(m_iNumberBoatTries);
	s.syncAsSint16LE(m_iFurlongs);
	s.syncAsSint16LE(m_nTurns);

	s.syncAsSint32LE(m_lCrowns);

	for (int i = 0; i < NUMBER_OF_CLUES; ++i)
		m_bClueArray[i].sync(s);

	m_stInventory.sync(s);
	m_stGenStore.sync(s);
	m_stBlackMarket.sync(s);
	m_stTradingPost.sync(s);
}

void SAVEGAME_INFO::sync(Common::Serializer &s) {
	s.syncAsUint32LE(m_lBoffoGameID);
	s.syncAsUint16LE(m_nFixedRecordSize);
	s.syncAsByte(m_bUsed);

	for (int i = 0; i < MAX_PLAYERS; ++i)
		m_stPlayerInfo[i].sync(s);

	s.syncBytes((byte *)m_bTraps, 240);
	s.syncAsSint16LE(m_iGameTime);
	s.syncAsSint16LE(m_iMishMoshLoc);

	s.syncAsByte(m_bSoundEffectsEnabled);
	s.syncAsByte(m_bMusicEnabled);
	s.syncAsByte(m_bScrolling);
	s.syncAsByte(m_bNewMishMosh);
}

} // namespace Saves
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
