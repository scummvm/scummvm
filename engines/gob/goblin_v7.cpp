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

#include "gob/global.h"
#include "gob/gob.h"
#include "gob/goblin.h"
#include "gob/inter.h"
#include "gob/map.h"
#include "gob/mult.h"
#include "gob/scenery.h"
#include "gob/videoplayer.h"

namespace Gob {

Goblin_v7::Goblin_v7(GobEngine *vm) : Goblin_v4(vm) {
}

static void sub_4309D3(Draw *draw, Mult::Mult_Object &obj) {
	if (obj.field_38 != nullptr) {
		Mult::Mult_AnimData animData = *obj.pAnimData;
		draw->freeSprite(50 - animData.animation - 1);
		// sub_473261()

		if (obj.field_50)
			obj.field_3C = nullptr;
		else
			obj.field_38[10] = 0;

		if (obj.field_38[40] != 0) {

		}
	}

	obj.field_38 = nullptr;
	obj.animName[0] = '\0';
}

void Goblin_v7::changeDirection(Mult::Mult_Object *obj, int16 animState) {
	char str[128];
	obj->pAnimData->layer &= 3;
	int32 var_4 = 0;
	Common::strlcpy(str, obj->animName, 128);
	if (obj->pAnimData->curLookDir >= 10) {
		if (obj->pAnimData->curLookDir == 26 ||
			obj->pAnimData->curLookDir == 36) {
			if (str[strlen(str) - 1] == 'U') {
				str[strlen(str) - 3] = '\0';
			} else
				str[strlen(str) - 2] = '\0';
		} else {
			str[strlen(str) - 3] = '\0';
		}
	} else {
		str[strlen(str) - 2] = '\0';
	}

	if (animState < 100)
		var_4 = 1;
	else
		var_4 = 0;

	animState %= 100;
	obj->pAnimData->curLookDir = animState;

	int16 var_98 = 0;
	int16 var_9C = 0;
	while (true) {
		if (animState <= 0 || animState > obj->animVariables[2]) {
			obj->pAnimData->animType = 11;
			return;
		} else {
			int16 *var_94 = obj->animVariables + animState * obj->animVariables[1];
			if (*var_94 == 0) {
				var_98 = var_94[1];
				var_9C = var_94[2];
			} else {
				if ((int8)*var_94 == -2) {
					switch (animState) {
					case 1:
						animState = 5;
						break;
					case 2:
						animState = 4;
						break;
					case 4:
						animState = 2;
						break;
					case 5:
						animState = 1;
						break;
					case 6:
						animState = 8;
						break;
					case 8:
						animState = 6;
						break;
					case 31:
					case 32:
					case 33:
					case 34:
					case 35:
					case 36:
						animState -= 10;
						break;
					default: // 3, 7, 9-30, > 36
						obj->pAnimData->animType = 11;
						return;
					}

					obj->pAnimData->layer |= 0x80;
					var_98 = var_94[1];
					var_9C = var_94[2];
				} else if ((int8)*var_94 == -1) {
					obj->pAnimData->animType = 11;
					return;
				} else {
					animState = *var_94;
					continue;
				}
			}
		}

		break;
	}

	int32 var_A4 = 0;
	if (obj->pAnimData->stateType == 1) {
		var_A4 = animState;
		if (var_A4 == 22)
			animState = 27;
		else if (var_A4 == 25)
			animState = 26;
	}

	switch (animState) {
	case 1:
		Common::strlcat(str, "GG", 128);
		break;
	case 2:
		Common::strlcat(str, "GH", 128);
		break;
	case 3:
		Common::strlcat(str, "HH", 128);
		break;
	case 4:
		Common::strlcat(str, "DH", 128);
		break;
	case 5:
		Common::strlcat(str, "DD", 128);
		break;
	case 6:
		Common::strlcat(str, "DB", 128);
		break;
	case 7:
		Common::strlcat(str, "BB", 128);
		break;
	case 8:
		Common::strlcat(str, "GB", 128);
		break;
	case 21:
		Common::strlcat(str, "COG", 128);
		break;
	case 22:
		warning("Goblin_v7::changeDirection: animState == 22 not implemented");
		break;
	case 23:
		Common::strlcat(str, "EXP", 128);
		break;
	case 24:
		Common::strlcat(str, "FRA", 128);
		break;
	case 25:
		warning("Goblin_v7::changeDirection: animState == 25 not implemented");
		break;
	case 26:
		warning("Goblin_v7::changeDirection: animState == 26 not implemented");
		break;
	case 27:
		warning("Goblin_v7::changeDirection: animState == 27 not implemented");
		break;

	default:
		Common::strlcat(str, Common::String::format("%02d", animState).c_str(), 128);
	}

	if (strcmp(str, obj->animName) != 0) {
		Common::strlcpy(obj->animName, str, 16);
	}

	int32 var_88 = 0;
	int32 var_90 = 0;
	if (_vm->_map->_mapUnknownBool) {
		var_88 = (_vm->_map->getTilesWidth() / 2) * obj->pAnimData->destX +
				 (_vm->_map->getTilesWidth() / 2) * obj->pAnimData->destY -
				 (_vm->_map->getTilesWidth() * 39) / 2;

		var_90 = (_vm->_map->getTilesHeight() / 2) * obj->pAnimData->destY -
				 (_vm->_map->getTilesHeight() / 2) * obj->pAnimData->destX +
				 (_vm->_map->getTilesHeight() * 20);

		if (animState > 10) {
			obj->destX = obj->pAnimData->destX;
			obj->goblinX = obj->destX;
			obj->destY = obj->pAnimData->destY;
			obj->goblinY = obj->destY;
		}
	} else {
		var_90 = obj->pAnimData->destY * _vm->_map->getTilesHeight();
		var_88 = obj->pAnimData->destX * _vm->_map->getTilesWidth();
	}

	*obj->pPosX = var_88 + var_98;
	*obj->pPosY = var_90 + var_9C;
	obj->pAnimData->frame = 0;
	if (var_4 == 0) {
		sub_4309D3(_vm->_draw, *obj);

		VideoPlayer::Properties props;
		props.x          = -1;
		props.y          = -1;
		props.startFrame = 0;
		props.lastFrame  = 0;
		props.breakKey   = 0;
		props.flags      = 0x1201;
		props.palStart   = 0;
		props.palEnd     = 0;

		Common::strlcpy(obj->animName, str, 16);
		if (obj->videoSlot > 0)
			_vm->_vidPlayer->closeVideo(obj->videoSlot - 1);

		int slot = _vm->_vidPlayer->openVideo(false, str, props);
		obj->videoSlot = slot + 1;
	} else {
		if (obj->field_38 == nullptr ||
			strcmp(obj->animName, str) != 0 ||
			obj->field_38[28] & 0x800) {

			VideoPlayer::Properties props;
			props.x          = -1;
			props.y          = -1;
			props.startFrame = 0;
			props.lastFrame  = 0;
			props.breakKey   = 0;
			props.flags      = 0x1601;
			props.palStart   = 0;
			props.palEnd     = 0;

			if (obj->videoSlot > 0)
				_vm->_vidPlayer->closeVideo(obj->videoSlot - 1);

			int slot = _vm->_vidPlayer->openVideo(false, str, props);
			obj->videoSlot = slot + 1;
		}
	}
}

void Goblin_v7::initiateMove(Mult::Mult_Object *obj) {
	int32 animState = 0;
	if (obj->destX != obj->gobDestX || obj->destY != obj->gobDestY) {
		animState = 1;
	}

	if (animState == 0) {
		obj->pAnimData->newState = animState;
		changeDirection(obj, animState);
	} else {
		if (obj->pAnimData->destX != obj->pAnimData->field_1D || obj->pAnimData->destY != obj->pAnimData->field_1E)
			obj->pAnimData->pathExistence = 2;
		else
			obj->pAnimData->pathExistence = 1;

		obj->pAnimData->animType = 12;
		if (obj->pAnimData->curLookDir >= 20) {
			if (obj->pAnimData->curLookDir >= 30) {
				if (obj->pAnimData->curLookDir >= 40)
					return;
				else {
					changeDirection(obj, 105);
					obj->pAnimData->pathExistence = 3;
				}
			} else {
				changeDirection(obj, 101);
				obj->pAnimData->pathExistence = 3;
			}
		} else {
			changeDirection(obj, obj->pAnimData->curLookDir + 100);
		}
	}
}

} // End of namespace Gob
