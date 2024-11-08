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

#include "chewy/globals.h"
#include "chewy/cursor.h"
#include "chewy/types.h"
#include "chewy/detail.h"
#include "common/algorithm.h"

namespace Chewy {

void GameFlags::clear() {
	// TODO: Not sure how to do this any better
	Common::fill((byte *)this, (byte *)this + sizeof(GameFlags), 0);
}

void GameState::clear() {
	*this = GameState();
	_flags = this;
	_flags->clear();
}

static void syncArray(Common::Serializer &s, uint8 *arr, size_t count) {
	for (size_t i = 0; i < count; ++i)
		s.syncAsByte(arr[i]);
}

static void syncArray(Common::Serializer &s, int16 *arr, size_t count) {
	for (size_t i = 0; i < count; ++i)
		s.syncAsSint16LE(arr[i]);
}

#define SPIELER_FLAGS_SIZE 38

bool GameState::synchronize(Common::Serializer &s) {
	if (sizeof(GameFlags) != SPIELER_FLAGS_SIZE)
		error("Invalid flags structure size");

	byte dummy = 0;
	int16 dummy16 = 0;
	uint8 InvUseDef[40 * 3] = {0};	// dummy
	int inventoryCursor = _G(cur)->getInventoryCursor();

	// Sync the structure's bitflags
	s.syncBytes((byte *)_flags, SPIELER_FLAGS_SIZE);

	_G(txt)->syncHotspotStrings(s);
	_G(txt)->syncInventoryStrings(s);
	_G(txt)->syncInventoryUseStrings(s);
	syncArray(s, InvUseDef, 40 * 3);

	s.syncAsSint16LE(MainMenuY);
	s.syncAsSint16LE(InvDisp);
	s.syncAsSint16LE(DispZx);
	s.syncAsSint16LE(DispZy);
	s.syncAsSint16LE(DispFlag);
	s.syncAsSint16LE(InventY);
	syncArray(s, InventSlot, MAX_MOV_OBJ);

	s.syncAsSint16LE(inventoryCursor);
	s.syncAsByte(dummy);			// obsolete inventoryCursor flag
	s.syncAsSint16LE(dummy16);	// curWidth
	s.syncAsSint16LE(dummy16);	// curHeight

	for (int i = 0; i < MAX_MOV_OBJ; ++i)
		room_m_obj[i].synchronize(s);
	for (int i = 0; i < MAX_FEST_OBJ; ++i)
		room_s_obj[i].synchronize(s);
	for (int i = 0; i < MAX_EXIT; ++i)
		room_e_obj[i].synchronize(s);

	syncArray(s, X, MAX_PERSON);
	syncArray(s, Y, MAX_PERSON);
	syncArray(s, Phase, MAX_PERSON);
	syncArray(s, _personHide, MAX_PERSON);
	syncArray(s, _personRoomNr, MAX_PERSON);
	syncArray(s, &ZoomXy[0][0], MAX_PERSON * 2);
	s.syncAsSint16LE(ChewyAni);
	syncArray(s, PersonGlobalDia, MAX_PERSON);
	syncArray(s, PersonDia, MAX_PERSON);
	syncArray(s, PersonDiaRoom, MAX_PERSON);
	syncArray(s, PersonDiaTmpRoom, MAX_PERSON);

	s.syncAsSint16LE(DiaAMov);
	s.syncAsSint16LE(scrollx);
	s.syncAsSint16LE(scrolly);
	s.syncAsSint16LE(ScrollxStep);
	s.syncAsSint16LE(ScrollyStep);
	int16 mouseSpeed = 6;
	s.syncAsSint16LE(mouseSpeed);
	s.syncAsSint16LE(DelaySpeed);
	s.syncAsSint16LE(AadDelay);
	s.syncAsSint16LE(AadSilent);
	s.syncAsByte(R0FueterLab);
	s.syncAsByte(R6RaumBetreten);
	s.syncAsByte(R6BolaJoke);
	s.syncAsByte(R7BellCount);
	s.syncAsByte(R11IdCardNr);
	s.syncAsByte(R12BorkCount);
	s.syncAsByte(R13MonitorStatus);
	s.syncAsByte(R17Location);
	s.syncAsSint16LE(R23GliderExit);
	syncArray(s, R24Lever, 3);
	syncArray(s, R24LeverDir, 3);
	syncArray(s, R24CrystalLast, 3);
	s.syncAsByte(R25SurimyGo);
	s.syncAsByte(R27HowardGed);
	s.syncAsByte(R28PumpTxt1);
	s.syncAsByte(R28PumpTxt);
	s.syncAsByte(R31SurimyGo);
	s.syncAsByte(R33SurimyGo);
	syncArray(s, R33Munter, 4);
	s.syncAsSint16LE(R39TvKanal);
	s.syncAsByte(R39TvRecord);
	s.syncAsByte(R39ClintNews);
	s.syncAsByte(R40PoliceAniStatus);
	s.syncAsByte(R41TrainCount);
	syncArray(s, R47Schloss, 3);
	syncArray(s, R48TaxiPerson, MAX_PERSON);
	syncArray(s, R48Auswahl, 5);
	s.syncAsByte(R49BoyAniCount);
	s.syncAsByte(R51DoorCount);
	s.syncAsByte(R54LiftCount);
	s.syncAsByte(R54HowardVorne);
	s.syncAsUint16LE(R55ExitDia);
	s.syncAsSint16LE(R58TmpRoom);
	s.syncAsSint16LE(R58TmpRoom1);
	s.syncAsSint16LE(R58TmpSx);
	s.syncAsSint16LE(R58TmpSy);
	s.syncAsSint16LE(R58TmpX);
	s.syncAsSint16LE(R58TmpY);
	s.syncAsSint16LE(R64Moni1Ani);
	s.syncAsSint16LE(R64Moni2Ani);
	syncArray(s, mi, 8);
	s.syncAsSint16LE(SVal1);
	s.syncAsSint16LE(SVal2);
	s.syncAsSint16LE(SVal3);
	s.syncAsSint16LE(SVal4);
	s.syncAsSint16LE(dummy16);	// sound loop mode
	s.syncAsByte(dummy);	// sound switch
	s.syncAsByte(dummy);	// sound volume
	s.syncAsByte(dummy);	// music switch
	s.syncAsByte(dummy);	// music volume
	s.syncAsByte(dummy);	// speech switch
	s.syncAsByte(FramesPerSecond);
	s.syncAsByte(dummy);	// subtitles switch

	_G(cur)->setInventoryCursor(inventoryCursor);

	return true;
}

void *MALLOC(size_t size) {
	void *ret = malloc(size);
	Common::fill((byte *)ret, (byte *)ret + size, 0);
	return ret;
}

} // namespace Chewy
