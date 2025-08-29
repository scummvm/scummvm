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

#ifndef ALCACHOFA_SCRIPT_DEBUG_H
#define ALCACHOFA_SCRIPT_DEBUG_H

namespace Alcachofa {

static const char *const ScriptOpNames[] = {
	"Nop",
	"Dup",
	"PushAddr",
	"PushValue",
	"Deref",
	"PopN",
	"Store",
	"LoadString",
	"ScriptCall",
	"KernelCall",
	"JumpIfFalse",
	"JumpIfTrue",
	"Jump",
	"Negate",
	"BooleanNot",
	"Mul",
	"Add",
	"Sub",
	"Less",
	"Greater",
	"LessEquals",
	"GreaterEquals",
	"Equals",
	"NotEquals",
	"BitAnd",
	"BitOr",
	"ReturnValue",
	"ReturnVoid",
	"Crash"
};

static const char *const KernelCallNames[] = {
	"Nop",
	"PlayVideo",
	"PlaySound",
	"PlayMusic",
	"StopMusic",
	"WaitForMusicToEnd",
	"ShowCenterBottomText",
	"StopAndTurn",
	"StopAndTurnMe",
	"ChangeCharacter",
	"SayText",
	"Go",
	"Put",
	"ChangeCharacterRoom",
	"KillProcesses",
	"LerpCharacterLodBias",
	"On",
	"Off",
	"Pickup",
	"CharacterPickup",
	"Drop",
	"CharacterDrop",
	"Delay",
	"HadNoMousePressFor",
	"Fork",
	"Animate",
	"AnimateCharacter",
	"AnimateTalking",
	"ChangeRoom",
	"ToggleRoomFloor",
	"SetDialogLineReturn",
	"DialogMenu",
	"ClearInventory",
	"FadeType0",
	"FadeType1",
	"LerpWorldLodBias",
	"FadeType2",
	"SetActiveTextureSet",
	"SetMaxCamSpeedFactor",
	"WaitCamStopping",
	"CamFollow",
	"CamShake",
	"LerpCamXY",
	"LerpCamZ",
	"LerpCamScale",
	"LerpCamToObjectWithScale",
	"LerpCamToObjectResettingZ",
	"LerpCamRotation",
	"FadeIn",
	"FadeOut",
	"FadeIn2",
	"FadeOut2",
	"LerpCamXYZ",
	"LerpCamToObjectKeepingZ"
};

}

#endif // ALCACHOFA_SCRIPT_DEBUG_H
