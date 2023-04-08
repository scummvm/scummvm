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

#include "watchmaker/classes/do_inv_inv.h"
#include "watchmaker/define.h"
#include "watchmaker/globvar.h"
#include "watchmaker/windows_hacks.h"
#include "watchmaker/message.h"
#include "watchmaker/schedule.h"
#include "watchmaker/classes/do_inv.h"
#include "watchmaker/ll/ll_diary.h"
#include "watchmaker/classes/do_sound.h"
#include "watchmaker/classes/do_string.h"

namespace Watchmaker {

/* -----------------19/05/98 16.40-------------------
 *                  doInvInvUseWith
 * --------------------------------------------------*/
void doInvInvUseWith(WGame &game) {
	uint8 sent = TRUE;
	Init &init = game.init;

	switch (UseWith[USED]) {
	case i00TELEFONO:
	case i00TELEFONOVIC:
		if ((UseWith[WITH] == i28WALKMANOK) && (init.InvObj[i28WALKMANOK].flags & EXTRA)) {
			_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSTART, MP_DEFAULT, 0, 0, tPDA, &UseWith[WITH], nullptr, nullptr);
			sent = FALSE;
		}
		break;

	case i1dAUDIOCASSETTA:
		if (UseWith[WITH] == i28WALKMANVUOTO) {
			ReplaceIcon(init, i28WALKMANVUOTO, i28WALKMANNASTRO);
			KillIcon(init, i1dAUDIOCASSETTA);
			SelectCurBigIcon(i28WALKMANNASTRO);
			UseWith[USED] = i28WALKMANNASTRO;
			sent = false;
		} else if (UseWith[WITH] == i28WALKMANPILE) {
			ReplaceIcon(init, i28WALKMANPILE, i28WALKMANOK);
			KillIcon(init, i1dAUDIOCASSETTA);
			SelectCurBigIcon(i28WALKMANOK);
			UseWith[USED] = i28WALKMANOK;
			sent = false;
		}
		break;

	case i25FIALEABPIENE:
		if (UseWith[WITH] == i34STAMPO) {
			ReplaceIcon(init, i34STAMPO, i25FIALEABUSATE);
			ReplaceIcon(init, i25FIALEABPIENE, i25MEDAGLIONI4);
			IncCurTime(game, 10);
		}
		break;

	case i27PILE:
		if (UseWith[WITH] == i28WALKMANVUOTO) {
			ReplaceIcon(init, i28WALKMANVUOTO, i28WALKMANPILE);
			KillIcon(init, i27PILE);
			SelectCurBigIcon(i28WALKMANPILE);
			UseWith[USED] = i28WALKMANPILE;
			IncCurTime(game, 5);
			sent = false;
		} else if (UseWith[WITH] == i28WALKMANNASTRO) {
			ReplaceIcon(init, i28WALKMANNASTRO, i28WALKMANOK);
			KillIcon(init, i27PILE);
			SelectCurBigIcon(i28WALKMANOK);
			UseWith[USED] = i28WALKMANOK;
			IncCurTime(game, 5);
			sent = false;
		}
		break;

	case i36BUSTA1DOSEA:
	case i36BUSTA2DOSIA:
		if (UseWith[WITH] == i2bSACCHETTOINCENSO)
			ReplaceIcon(init, (uint8)UseWith[USED], i2rBUSTAVUOTAA);
		break;
	case i36BUSTA1DOSEB:
	case i36BUSTA2DOSIB:
		if (UseWith[WITH] == i2bSACCHETTOINCENSO)
			ReplaceIcon(init, (uint8)UseWith[USED], i2rBUSTAVUOTAB);
		break;

	case i3bLASTRANERA:
		if (UseWith[WITH] == i3bLASTRABIANCA) {
			StartSound(game, w3B22);
			ReplaceIcon(init, i3bLASTRABIANCA, i3bLASTRA2VOLTI);
			KillIcon(init, i3bLASTRANERA);
			SelectCurBigIcon(i3bLASTRA2VOLTI);
			sent = false;
		}
		break;
	case i3bLASTRABIANCA:
		if (UseWith[WITH] == i3bLASTRANERA) {
			StartSound(game, w3B22);
			ReplaceIcon(init, i3bLASTRABIANCA, i3bLASTRA2VOLTI);
			KillIcon(init, i3bLASTRANERA);
			SelectCurBigIcon(i3bLASTRA2VOLTI);
			sent = false;
		}
		break;

	default:
		sent = TRUE;
		break;
	}

	if (sent)
		if (!((bUseWith & UW_WITHI) && (UseWith[USED] == UseWith[WITH])))
			PlayerSpeak(game, init.InvObj[UseWith[USED]].action[CurPlayer]);
}

} // End of namespace Watchmaker