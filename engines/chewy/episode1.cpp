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

#include "chewy/defines.h"
#include "chewy/events.h"
#include "chewy/global.h"
#include "chewy/ani_dat.h"
#include "chewy/episode1.h"
#include "chewy/rooms/rooms.h"

namespace Chewy {

int16 e_streifen;

void load_chewy_taf(int16 taf_nr) {
	taf_dateiheader *tafheader;
	const char *fname_;
	if (AkChewyTaf != taf_nr) {
		if (chewy) {
			free((char *)chewy);
			chewy = nullptr;
		}
		spieler_mi[P_CHEWY].HotY = CH_HOT_Y;
		switch (taf_nr) {
		case CHEWY_NORMAL:
			fname_ = CHEWY_TAF;
			chewy_ph_anz = chewy_phasen_anz;
			chewy_ph = (uint8 *)chewy_phasen;
			break;

		case CHEWY_BORK:
			fname_ = CHEWY_BO_TAF;
			chewy_ph_anz = chewy_bo_phasen_anz;
			chewy_ph = (uint8 *)chewy_bo_phasen;
			break;

		case CHEWY_MINI:
			fname_ = CHEWY_MI_TAF;
			chewy_ph_anz = chewy_mi_phasen_anz;
			chewy_ph = (uint8 *)chewy_mi_phasen;
			break;

		case CHEWY_PUMPKIN:
			fname_ = CHEWY_PUMP_TAF;
			chewy_ph_anz = chewy_mi_phasen_anz;
			chewy_ph = (uint8 *)chewy_mi_phasen;
			break;

		case CHEWY_ROCKER:
			fname_ = CHEWY_ROCK_TAF;
			chewy_ph_anz = chewy_ro_phasen_anz;
			chewy_ph = (uint8 *)chewy_ro_phasen;
			break;

		case CHEWY_JMANS:
			fname_ = CHEWY_JMAN_TAF;
			chewy_ph_anz = chewy_ro_phasen_anz;
			chewy_ph = (uint8 *)chewy_jm_phasen;
			spieler_mi[P_CHEWY].HotY = 68;
			break;

		default:
			fname_ = NULL;
			break;

		}
		if (fname_ != NULL) {
			_G(spieler).ChewyAni = taf_nr;
			AkChewyTaf = taf_nr;
			chewy = mem->taf_adr(fname_);
			ERROR
			mem->file->get_tafinfo(fname_, &tafheader);
			if (!modul) {
				chewy_kor = chewy->korrektur;
			} else {
				error();
			}
		}
	}
}

void switch_room(int16 nr) {
	fx_blend = BLEND1;
	exit_room(-1);
	_G(spieler).PersonRoomNr[P_CHEWY] = nr;
	room->load_room(&room_blk, _G(spieler).PersonRoomNr[P_CHEWY], &_G(spieler));
	ERROR

	enter_room(-1);
	set_up_screen(DO_SETUP);
}

} // namespace Chewy
