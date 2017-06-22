/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "kingdom/kingdom.h"

namespace Kingdom {

void KingdomGame::GPLogic3() {
	switch(_StatPlay) {
	case 621:
		switch(_UserInput) {
		case 0x428:
		case 0x429:
		case 0x42A:
			InventoryDel(_UserInput - 0x428);
			SaveAS();
			PlayMovie(178);
			RestoreAS();
			_UserInput = 0;
			break;
		case 0x430:
			PlayMovie(150);
			InventoryAdd(8);
			word_2D7C2 = 1;
			PlaySound(25);
			_CurrMap = 121;
			_UserInput = 0;
			break;
		case 0x43E:
			switch (word_2D7C2) {
			case 0:
				PlayMovie(149);
				break;
			case 1:
				PlayMovie(150);
				break;
			case 9:
				_FrameStop = 24;
				PlayMovie(202);
				_CurrMap = 121;
				break;
			}
			break;
		case 0x445:
			_NodeNum = 56;
			_UserInput = 0;
			_FrameStop = 50;
			EnAll();
			PlayMovie(135);
			word_2D7B6 = 1;
			PlaySound(8);
			_StatPlay = 561;
			_CurrMap = 106;
			if (_Inventory[16] < 1 && _Wizard)
				_CurrMap = 88;
			break;
		case 0x452:
			_StatPlay = 630;
			_LoopFlag = true;
			_UserInput = 0;
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _StatPlay);
			break;
		}
		break;
	case 701:
		switch(_UserInput) {
		case 0x43E:
			_Sound = _LastSound;
			PlayMovie(_PMovie);
			break;
		case 0x445:
			_StatPlay = 650;
			_LoopFlag = true;
			break;
		case 0x446:
			_StatPlay = 780;
			_LoopFlag = true;
			break;
		case 0x447:
			_StatPlay = 660;
			_LoopFlag = true;
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _StatPlay);
			break;
		}
		break;
	case 740:
		_RtnNode = _NodeNum;
		_NodeNum = 74;
		DrawLocation();
		DsAll();
		PlayMovie(185);
		_CurrMap = 10;
		SetATimer();
		_StatPlay = 741;
		_UserInput = 0;
		break;
	case 780:
		_NodeNum = 78;
		DrawLocation();
		_UserInput = 0;
		_Eye = false;
		PlayMovie(171);
		_CurrMap = 124;
		PlaySound(40);
		_StatPlay = 781;
		break;
	case 781:
		switch(_UserInput) {
		case 0x428:
		case 0x42A:
			InventoryDel(_UserInput - 0x428);
			_FrameStop = 64;
			PlayMovie(173);
			ShowPic(471);
			_CurrMap = 124;
			break;
		case 0x429:
			if (_Wizard) {
				_FrameStop = 64;
				PlayMovie(173);
				InventoryDel(1);
			} else {
				_FrameStop = 66;
				_FstFwd = false;
				PlayMovie(172);
				_FstFwd = true;
				PlayMovie(201);
			}

			FShowPic(471);
			_CurrMap = 124;
			break;
		case 0x439:
			_FrameStop = 59;
			_FstFwd = false;
			PlayMovie(174);
			_FstFwd = true;
			PlayMovie(201);
			FShowPic(471);
			_CurrMap = 124;
			break;
		case 0x43E:
			_Sound = _LastSound;
			if (_PMovie == 173)
				_FrameStop = 64;
			if (_PMovie == 201) {
				_FrameStop = 59;
				_FstFwd = false;
				PlayMovie(174);
				_FstFwd = true;
				PlayMovie(201);
			} else {
				PlayMovie(_PMovie);
			}
			if (_PMovie != 171) {
				FShowPic(471);
				_CurrMap = 124;
			}
			break;
		case 0x445:
			_StatPlay = 660;
			_LoopFlag = true;
			break;
		case 0x446:
			_StatPlay = (word_2D7CC == 1) ? 760 : 660;
			_LoopFlag = true;
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _StatPlay);
			break;
		}
		break;
	case 790:
		_NodeNum = 79;
		DrawLocation();
		DsAll();
		PlayMovie(4);
		_CurrMap = 8;
		_StatPlay = 791;
		SetATimer();
		_UserInput = 0;
		break;
	case 791:
		switch(_UserInput) {
		case 0x2F1:
			DsAll();
			PlayMovie(204);
			_StatPlay = 993;
			_LoopFlag = true;
			break;
		case 0x445:
			EnAll();
			PlayMovie(10);
			_StatPlay = 50;
			_LoopFlag = true;
			break;
		case 0x446:
			EnAll();
			PlayMovie(10);
			_StatPlay = 520;
			_LoopFlag = true;
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _StatPlay);
			break;
		}
		break;
	}
}

} // NameSpace
