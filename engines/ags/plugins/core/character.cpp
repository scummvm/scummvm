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

#include "ags/plugins/core/character.h"
#include "ags/engine/ac/character.h"
#include "ags/engine/ac/global_character.h"
#include "ags/shared/ac/game_struct_defines.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Character::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(Character::AddInventory^2, AddInventory);
	SCRIPT_METHOD_EXT(Character::AddWaypoint^2, AddWaypoint);
	SCRIPT_METHOD_EXT(Character::Animate^5, Animate);
	SCRIPT_METHOD_EXT(Character::ChangeRoom^3, ChangeRoom);
	SCRIPT_METHOD_EXT(Character::ChangeRoomAutoPosition^2, ChangeRoomAutoPosition);
	SCRIPT_METHOD_EXT(Character::ChangeView^1, ChangeView);
	SCRIPT_METHOD_EXT(Character::FaceCharacter^2, FaceCharacter);
	SCRIPT_METHOD_EXT(Character::FaceDirection^2, FaceDirection);
	SCRIPT_METHOD_EXT(Character::FaceLocation^3, FaceLocation);
	SCRIPT_METHOD_EXT(Character::FaceObject^2, FaceObject);
	SCRIPT_METHOD_EXT(Character::FollowCharacter^3, FollowCharacter);
	SCRIPT_METHOD_EXT(Character::GetProperty^1, GetProperty);
	SCRIPT_METHOD_EXT(Character::GetPropertyText^2, GetPropertyText);
	SCRIPT_METHOD_EXT(Character::GetTextProperty^1, GetTextProperty);
	SCRIPT_METHOD_EXT(Character::HasInventory^1, HasInventory);
	SCRIPT_METHOD_EXT(Character::IsCollidingWithChar^1, IsCollidingWithChar);
	SCRIPT_METHOD_EXT(Character::IsCollidingWithObject^1, IsCollidingWithObject);
	SCRIPT_METHOD_EXT(Character::LockView^1, LockView);
	SCRIPT_METHOD_EXT(Character::LockView^2, LockViewEx);
	if (engine->version < kScriptAPI_v341) {
		SCRIPT_METHOD_EXT(Character::LockViewAligned^3, LockViewAligned_Old);
		SCRIPT_METHOD_EXT(Character::LockViewAligned^4, LockViewAlignedEx_Old);
	} else {
		SCRIPT_METHOD_EXT(Character::LockViewAligned^3, LockViewAligned);
		SCRIPT_METHOD_EXT(Character::LockViewAligned^4, LockViewAlignedEx);
	}
	SCRIPT_METHOD_EXT(Character::LockViewFrame^3, LockViewFrame);
	SCRIPT_METHOD_EXT(Character::LockViewFrame^4, LockViewFrameEx);
	SCRIPT_METHOD_EXT(Character::LockViewOffset^3, LockViewOffset);
	SCRIPT_METHOD_EXT(Character::LockViewOffset^4, LockViewOffset);
	SCRIPT_METHOD_EXT(Character::LoseInventory^1, LoseInventory);
	SCRIPT_METHOD_EXT(Character::Move^4, Move);
	SCRIPT_METHOD_EXT(Character::PlaceOnWalkableArea^0, PlaceOnWalkableArea);
	SCRIPT_METHOD_EXT(Character::RemoveTint^0, RemoveTint);
	SCRIPT_METHOD_EXT(Character::RunInteraction^1, RunInteraction);
	SCRIPT_METHOD_EXT(Character::Say^101, ScPl_Say);
	SCRIPT_METHOD_EXT(Character::SayAt^4, SayAt);
	SCRIPT_METHOD_EXT(Character::SayBackground^1, SayBackground);
	SCRIPT_METHOD_EXT(Character::SetAsPlayer^0, SetAsPlayer);
	SCRIPT_METHOD_EXT(Character::SetIdleView^2, SetIdleView);
	//SCRIPT_METHOD_EXT(Character::SetOption^2",             (void*)SetOption);
	SCRIPT_METHOD_EXT(Character::SetWalkSpeed^2, SetSpeed);
	SCRIPT_METHOD_EXT(Character::StopMoving^0, StopMoving);
	SCRIPT_METHOD_EXT(Character::Think^101, ScPl_Think);
	SCRIPT_METHOD_EXT(Character::Tint^5, Tint);
	SCRIPT_METHOD_EXT(Character::UnlockView^0, UnlockView);
	SCRIPT_METHOD_EXT(Character::UnlockView^1, UnlockViewEx);
	SCRIPT_METHOD_EXT(Character::Walk^4, Walk);
	SCRIPT_METHOD_EXT(Character::WalkStraight^3, WalkStraight);
	SCRIPT_METHOD_EXT(Character::GetAtRoomXY^2, GetCharacterAtRoom);
	SCRIPT_METHOD_EXT(Character::GetAtScreenXY^2, GetCharacterAtScreen);
	SCRIPT_METHOD_EXT(Character::get_ActiveInventory, GetActiveInventory);
	SCRIPT_METHOD_EXT(Character::set_ActiveInventory, SetActiveInventory);
	SCRIPT_METHOD_EXT(Character::get_Animating, GetAnimating);
	SCRIPT_METHOD_EXT(Character::get_AnimationSpeed, GetAnimationSpeed);
	SCRIPT_METHOD_EXT(Character::set_AnimationSpeed, SetAnimationSpeed);
	SCRIPT_METHOD_EXT(Character::get_Baseline, GetBaseline);
	SCRIPT_METHOD_EXT(Character::set_Baseline, SetBaseline);
	SCRIPT_METHOD_EXT(Character::get_BlinkInterval, GetBlinkInterval);
	SCRIPT_METHOD_EXT(Character::set_BlinkInterval, SetBlinkInterval);
	SCRIPT_METHOD_EXT(Character::get_BlinkView, GetBlinkView);
	SCRIPT_METHOD_EXT(Character::set_BlinkView, SetBlinkView);
	SCRIPT_METHOD_EXT(Character::get_BlinkWhileThinking, GetBlinkWhileThinking);
	SCRIPT_METHOD_EXT(Character::set_BlinkWhileThinking, SetBlinkWhileThinking);
	SCRIPT_METHOD_EXT(Character::get_BlockingHeight, GetBlockingHeight);
	SCRIPT_METHOD_EXT(Character::set_BlockingHeight, SetBlockingHeight);
	SCRIPT_METHOD_EXT(Character::get_BlockingWidth, GetBlockingWidth);
	SCRIPT_METHOD_EXT(Character::set_BlockingWidth, SetBlockingWidth);
	SCRIPT_METHOD_EXT(Character::get_Clickable, GetClickable);
	SCRIPT_METHOD_EXT(Character::set_Clickable, SetClickable);
	SCRIPT_METHOD_EXT(Character::get_DestinationX, GetDestinationX);
	SCRIPT_METHOD_EXT(Character::get_DestinationY, GetDestinationY);
	SCRIPT_METHOD_EXT(Character::get_DiagonalLoops, GetDiagonalWalking);
	SCRIPT_METHOD_EXT(Character::set_DiagonalLoops, SetDiagonalWalking);
	SCRIPT_METHOD_EXT(Character::get_Frame, GetFrame);
	SCRIPT_METHOD_EXT(Character::set_Frame, SetFrame);
	if (engine->version < kScriptAPI_v341)
		SCRIPT_METHOD_EXT(Character::get_HasExplicitTint, GetHasExplicitTint_Old);
	else
		SCRIPT_METHOD_EXT(Character::get_HasExplicitTint, GetHasExplicitTint);
	SCRIPT_METHOD_EXT(Character::get_ID, GetID);
	SCRIPT_METHOD_EXT(Character::get_IdleView, GetIdleView);
	SCRIPT_METHOD_EXT(Character::geti_InventoryQuantity, GetIInventoryQuantity);
	SCRIPT_METHOD_EXT(Character::seti_InventoryQuantity, SetIInventoryQuantity);
	SCRIPT_METHOD_EXT(Character::get_IgnoreLighting, GetIgnoreLighting);
	SCRIPT_METHOD_EXT(Character::set_IgnoreLighting, SetIgnoreLighting);
	SCRIPT_METHOD_EXT(Character::get_IgnoreScaling, GetIgnoreScaling);
	SCRIPT_METHOD_EXT(Character::set_IgnoreScaling, SetIgnoreScaling);
	SCRIPT_METHOD_EXT(Character::get_IgnoreWalkbehinds, GetIgnoreWalkbehinds);
	SCRIPT_METHOD_EXT(Character::set_IgnoreWalkbehinds, SetIgnoreWalkbehinds);
	SCRIPT_METHOD_EXT(Character::get_Loop, GetLoop);
	SCRIPT_METHOD_EXT(Character::set_Loop, SetLoop);
	SCRIPT_METHOD_EXT(Character::get_ManualScaling, GetIgnoreScaling);
	SCRIPT_METHOD_EXT(Character::set_ManualScaling, SetManualScaling);
	SCRIPT_METHOD_EXT(Character::get_MovementLinkedToAnimation, GetMovementLinkedToAnimation);
	SCRIPT_METHOD_EXT(Character::set_MovementLinkedToAnimation, SetMovementLinkedToAnimation);
	SCRIPT_METHOD_EXT(Character::get_Moving, GetMoving);
	SCRIPT_METHOD_EXT(Character::get_Name, GetName);
	SCRIPT_METHOD_EXT(Character::set_Name, SetName);
	SCRIPT_METHOD_EXT(Character::get_NormalView, GetNormalView);
	SCRIPT_METHOD_EXT(Character::get_PreviousRoom, GetPreviousRoom);
	SCRIPT_METHOD_EXT(Character::get_Room, GetRoom);
	SCRIPT_METHOD_EXT(Character::get_ScaleMoveSpeed, GetScaleMoveSpeed);
	SCRIPT_METHOD_EXT(Character::set_ScaleMoveSpeed, SetScaleMoveSpeed);
	SCRIPT_METHOD_EXT(Character::get_ScaleVolume, GetScaleVolume);
	SCRIPT_METHOD_EXT(Character::set_ScaleVolume, SetScaleVolume);
	SCRIPT_METHOD_EXT(Character::get_Scaling, GetScaling);
	SCRIPT_METHOD_EXT(Character::set_Scaling, SetScaling);
	SCRIPT_METHOD_EXT(Character::get_Solid, GetSolid);
	SCRIPT_METHOD_EXT(Character::set_Solid, SetSolid);
	SCRIPT_METHOD_EXT(Character::get_Speaking, GetSpeaking);
	SCRIPT_METHOD_EXT(Character::get_SpeakingFrame, GetSpeakingFrame);
	SCRIPT_METHOD_EXT(Character::get_SpeechAnimationDelay, GetCharacterSpeechAnimationDelay);
	SCRIPT_METHOD_EXT(Character::set_SpeechAnimationDelay, SetSpeechAnimationDelay);
	SCRIPT_METHOD_EXT(Character::get_SpeechColor, GetSpeechColor);
	SCRIPT_METHOD_EXT(Character::set_SpeechColor, SetSpeechColor);
	SCRIPT_METHOD_EXT(Character::get_SpeechView, GetSpeechView);
	SCRIPT_METHOD_EXT(Character::set_SpeechView, SetSpeechView);
	SCRIPT_METHOD_EXT(Character::get_ThinkView, GetThinkView);
	SCRIPT_METHOD_EXT(Character::set_ThinkView, SetThinkView);
	SCRIPT_METHOD_EXT(Character::get_Transparency, GetTransparency);
	SCRIPT_METHOD_EXT(Character::set_Transparency, SetTransparency);
	SCRIPT_METHOD_EXT(Character::get_TurnBeforeWalking, GetTurnBeforeWalking);
	SCRIPT_METHOD_EXT(Character::set_TurnBeforeWalking, SetTurnBeforeWalking);
	SCRIPT_METHOD_EXT(Character::get_View, GetView);
	SCRIPT_METHOD_EXT(Character::get_WalkSpeedX, GetWalkSpeedX);
	SCRIPT_METHOD_EXT(Character::get_WalkSpeedY, GetWalkSpeedY);
	SCRIPT_METHOD_EXT(Character::get_X, GetX);
	SCRIPT_METHOD_EXT(Character::set_X, SetX);
	SCRIPT_METHOD_EXT(Character::get_x, GetX);
	SCRIPT_METHOD_EXT(Character::set_x, SetX);
	SCRIPT_METHOD_EXT(Character::get_Y, GetY);
	SCRIPT_METHOD_EXT(Character::set_Y, SetY);
	SCRIPT_METHOD_EXT(Character::get_y, GetY);
	SCRIPT_METHOD_EXT(Character::set_y, SetY);
	SCRIPT_METHOD_EXT(Character::get_Z, GetZ);
	SCRIPT_METHOD_EXT(Character::set_Z, SetZ);
	SCRIPT_METHOD_EXT(Character::get_z, GetZ);
	SCRIPT_METHOD_EXT(Character::set_z, SetZ);
}

void Character::AddInventory(ScriptMethodParams &params) {
	PARAMS3(CharacterInfo *, chaa, ScriptInvItem *, invi, int, addIndex);
	AGS3::Character_AddInventory(chaa, invi, addIndex);
}

void Character::AddWaypoint(ScriptMethodParams &params) {
	PARAMS3(CharacterInfo *, chaa, int, x, int, y);
	AGS3::Character_AddWaypoint(chaa, x, y);
}

void Character::Animate(ScriptMethodParams &params) {
	PARAMS6(CharacterInfo *, chaa, int, loop, int, delay, int, repeat, int, blocking, int, direction);
	AGS3::Character_Animate(chaa, loop, delay, repeat, blocking, direction);
}

void Character::ChangeRoom(ScriptMethodParams &params) {
	PARAMS4(CharacterInfo *, chaa, int, room, int, x, int, y);
	AGS3::Character_ChangeRoom(chaa, room, x, y);
}

void Character::ChangeRoomAutoPosition(ScriptMethodParams &params) {
	PARAMS3(CharacterInfo *, chaa, int, room, int, newPos);
	AGS3::Character_ChangeRoomAutoPosition(chaa, room, newPos);
}

void Character::ChangeView(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chap, int, vii);
	AGS3::Character_ChangeView(chap, vii);
}

void Character::FaceCharacter(ScriptMethodParams &params) {
	PARAMS3(CharacterInfo *, char1, CharacterInfo *, char2, int, blockingStyle);
	AGS3::Character_FaceCharacter(char1, char2, blockingStyle);
}

void Character::FaceDirection(ScriptMethodParams &params) {
	PARAMS3(CharacterInfo *, char1, int, direction, int, blockingStyle);
	AGS3::Character_FaceDirection(char1, direction, blockingStyle);
}

void Character::FaceLocation(ScriptMethodParams &params) {
	PARAMS4(CharacterInfo *, char1, int, xx, int, yy, int, blockingStyle);
	AGS3::Character_FaceLocation(char1, xx, yy, blockingStyle);
}

void Character::FaceObject(ScriptMethodParams &params) {
	PARAMS3(CharacterInfo *, char1, ScriptObject *, obj, int, blockingStyle);
	AGS3::Character_FaceObject(char1, obj, blockingStyle);
}

void Character::FollowCharacter(ScriptMethodParams &params) {
	PARAMS4(CharacterInfo *, chaa, CharacterInfo *, tofollow, int, distaway, int, eagerness);
	AGS3::Character_FollowCharacter(chaa, tofollow, distaway, eagerness);
}

void Character::GetProperty(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, const char *, property);
	params._result = AGS3::Character_GetProperty(chaa, property);
}

void Character::GetPropertyText(ScriptMethodParams &params) {
	PARAMS3(CharacterInfo *, chaa, const char *, property, char *, buffer);
	AGS3::Character_GetPropertyText(chaa, property, buffer);
}

void Character::GetTextProperty(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, const char *, property);
	params._result = AGS3::Character_GetTextProperty(chaa, property);
}

void Character::HasInventory(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, ScriptInvItem *, invi);
	params._result = AGS3::Character_HasInventory(chaa, invi);
}

void Character::IsCollidingWithChar(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, char1, CharacterInfo *, char2);
	params._result = AGS3::Character_IsCollidingWithChar(char1, char2);
}

void Character::IsCollidingWithObject(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chin, ScriptObject *, objid);
	params._result = AGS3::Character_IsCollidingWithObject(chin, objid);
}

void Character::LockView(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chap, int, vii);
	AGS3::Character_LockView(chap, vii);
}

void Character::LockViewEx(ScriptMethodParams &params) {
	PARAMS3(CharacterInfo *, chap, int, vii, int, stopMoving);
	AGS3::Character_LockViewEx(chap, vii, stopMoving);
}

void Character::LockViewAligned_Old(ScriptMethodParams &params) {
	PARAMS4(CharacterInfo *, chap, int, vii, int, loop, int, align);
	AGS3::Character_LockViewAligned_Old(chap, vii, loop, align);
}

void Character::LockViewAlignedEx_Old(ScriptMethodParams &params) {
	PARAMS5(CharacterInfo *, chap, int, vii, int, loop, int, align, int, stopMoving);
	AGS3::Character_LockViewAlignedEx_Old(chap, vii, loop, align, stopMoving);
}

void Character::LockViewAligned(ScriptMethodParams &params) {
	PARAMS4(CharacterInfo *, chap, int, vii, int, loop, int, align);
	AGS3::Character_LockViewAligned(chap, vii, loop, align);
}

void Character::LockViewAlignedEx(ScriptMethodParams &params) {
	PARAMS5(CharacterInfo *, chap, int, vii, int, loop, int, align, int, stopMoving);
	AGS3::Character_LockViewAlignedEx(chap, vii, loop, align, stopMoving);
}

void Character::LockViewFrame(ScriptMethodParams &params) {
	PARAMS4(CharacterInfo *, chaa, int, view, int, loop, int, frame);
	AGS3::Character_LockViewFrame(chaa, view, loop, frame);
}

void Character::LockViewFrameEx(ScriptMethodParams &params) {
	PARAMS5(CharacterInfo *, chaa, int, view, int, loop, int, frame, int, stopMoving);
	AGS3::Character_LockViewFrameEx(chaa, view, loop, frame, stopMoving);
}

void Character::LockViewOffset(ScriptMethodParams &params) {
	PARAMS4(CharacterInfo *, chap, int, vii, int, xoffs, int, yoffs);
	AGS3::Character_LockViewOffset(chap, vii, xoffs, yoffs);
}

void Character::LoseInventory(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chap, ScriptInvItem *, invi);
	AGS3::Character_LoseInventory(chap, invi);
}

void Character::Move(ScriptMethodParams &params) {
	PARAMS5(CharacterInfo *, chaa, int, x, int, y, int, blocking, int, direct);
	AGS3::Character_Move(chaa, x, y, blocking, direct);
}

void Character::PlaceOnWalkableArea(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chap);
	AGS3::Character_PlaceOnWalkableArea(chap);
}

void Character::RemoveTint(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	AGS3::Character_RemoveTint(chaa);
}

void Character::RunInteraction(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, mood);
	AGS3::Character_RunInteraction(chaa, mood);
}

void Character::ScPl_Say(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	Common::String texx = params.format(1);
	AGS3::Character_Say(chaa, texx.c_str());
}

void Character::SayAt(ScriptMethodParams &params) {
	PARAMS5(CharacterInfo *, chaa, int, x, int, y, int, width, const char *, texx);
	AGS3::Character_SayAt(chaa, x, y, width, texx);
}

void Character::SayBackground(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, const char *, texx);
	params._result = AGS3::Character_SayBackground(chaa, texx);
}

void Character::SetAsPlayer(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	AGS3::Character_SetAsPlayer(chaa);
}

void Character::SetIdleView(ScriptMethodParams &params) {
	PARAMS3(CharacterInfo *, chaa, int, iview, int, itime);
	AGS3::Character_SetIdleView(chaa, iview, itime);
}

void Character::SetSpeed(ScriptMethodParams &params) {
	PARAMS3(CharacterInfo *, chaa, int, xspeed, int, yspeed);
	AGS3::Character_SetSpeed(chaa, xspeed, yspeed);
}

void Character::StopMoving(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, charp);
	AGS3::Character_StopMoving(charp);
}

void Character::ScPl_Think(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	Common::String texx = params.format(1);
	AGS3::Character_Think(chaa, texx.c_str());
}

void Character::Tint(ScriptMethodParams &params) {
	PARAMS6(CharacterInfo *, chaa, int, red, int, green, int, blue, int, opacity, int, luminance);
	AGS3::Character_Tint(chaa, red, green, blue, opacity, luminance);
}

void Character::UnlockView(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	AGS3::Character_UnlockView(chaa);
}

void Character::UnlockViewEx(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, stopMoving);
	AGS3::Character_UnlockViewEx(chaa, stopMoving);
}

void Character::Walk(ScriptMethodParams &params) {
	PARAMS5(CharacterInfo *, chaa, int, x, int, y, int, blocking, int, direct);
	AGS3::Character_Walk(chaa, x, y, blocking, direct);
}

void Character::WalkStraight(ScriptMethodParams &params) {
	PARAMS4(CharacterInfo *, chaa, int, xx, int, yy, int, blocking);
	AGS3::Character_WalkStraight(chaa, xx, yy, blocking);
}

void Character::GetCharacterAtRoom(ScriptMethodParams &params) {
	PARAMS2(int, x, int, y);
	params._result = AGS3::GetCharacterAtRoom(x, y);
}

void Character::GetCharacterAtScreen(ScriptMethodParams &params) {
	PARAMS2(int, x, int, y);
	params._result = AGS3::GetCharacterAtScreen(x, y);
}

void Character::GetActiveInventory(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetActiveInventory(chaa);
}

void Character::SetActiveInventory(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, ScriptInvItem *, iit);
	AGS3::Character_SetActiveInventory(chaa, iit);
}

void Character::GetAnimating(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetAnimating(chaa);
}

void Character::GetAnimationSpeed(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetAnimationSpeed(chaa);
}

void Character::SetAnimationSpeed(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, newval);
	AGS3::Character_SetAnimationSpeed(chaa, newval);
}

void Character::GetBaseline(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetBaseline(chaa);
}

void Character::SetBaseline(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, basel);
	AGS3::Character_SetBaseline(chaa, basel);
}

void Character::GetBlinkInterval(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetBlinkInterval(chaa);
}

void Character::SetBlinkInterval(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, interval);
	AGS3::Character_SetBlinkInterval(chaa, interval);
}

void Character::GetBlinkView(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetBlinkView(chaa);
}

void Character::SetBlinkView(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, vii);
	AGS3::Character_SetBlinkView(chaa, vii);
}

void Character::GetBlinkWhileThinking(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetBlinkWhileThinking(chaa);
}

void Character::SetBlinkWhileThinking(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, yesOrNo);
	AGS3::Character_SetBlinkWhileThinking(chaa, yesOrNo);
}

void Character::GetBlockingHeight(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetBlockingHeight(chaa);
}

void Character::SetBlockingHeight(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, hit);
	AGS3::Character_SetBlockingHeight(chaa, hit);
}

void Character::GetBlockingWidth(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetBlockingWidth(chaa);
}

void Character::SetBlockingWidth(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, wid);
	AGS3::Character_SetBlockingWidth(chaa, wid);
}

void Character::GetClickable(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetClickable(chaa);
}

void Character::SetClickable(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, clik);
	AGS3::Character_SetClickable(chaa, clik);
}

void Character::GetDestinationX(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetDestinationX(chaa);
}

void Character::GetDestinationY(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetDestinationX(chaa);
}

void Character::GetDiagonalWalking(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetDiagonalWalking(chaa);
}

void Character::SetDiagonalWalking(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, yesOrNo);
	AGS3::Character_SetDiagonalWalking(chaa, yesOrNo);
}

void Character::GetFrame(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetFrame(chaa);
}

void Character::SetFrame(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, newVal);
	AGS3::Character_SetFrame(chaa, newVal);
}

void Character::GetHasExplicitTint_Old(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetHasExplicitTint_Old(chaa);
}

void Character::GetHasExplicitTint(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetHasExplicitTint(chaa);
}

void Character::GetID(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetID(chaa);
}

void Character::GetIdleView(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetIdleView(chaa);
}

void Character::GetIInventoryQuantity(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, index);
	params._result = AGS3::Character_GetIInventoryQuantity(chaa, index);
}

void Character::SetIInventoryQuantity(ScriptMethodParams &params) {
	PARAMS3(CharacterInfo *, chaa, int, index, int, quant);
	AGS3::Character_SetIInventoryQuantity(chaa, index, quant);
}

void Character::GetIgnoreLighting(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetIgnoreLighting(chaa);
}

void Character::SetIgnoreLighting(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, yesOrNo);
	AGS3::Character_SetIgnoreLighting(chaa, yesOrNo);
}

void Character::GetIgnoreScaling(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetIgnoreScaling(chaa);
}

void Character::SetIgnoreScaling(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, yesOrNo);
	AGS3::Character_SetIgnoreScaling(chaa, yesOrNo);
}

void Character::GetIgnoreWalkbehinds(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetIgnoreWalkbehinds(chaa);
}

void Character::SetIgnoreWalkbehinds(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, yesOrNo);
	AGS3::Character_SetIgnoreWalkbehinds(chaa, yesOrNo);
}

void Character::GetLoop(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetLoop(chaa);
}

void Character::SetLoop(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, newVal);
	AGS3::Character_SetLoop(chaa, newVal);
}

void Character::SetManualScaling(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, yesOrNo);
	AGS3::Character_SetManualScaling(chaa, yesOrNo);
}

void Character::GetMovementLinkedToAnimation(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetMovementLinkedToAnimation(chaa);
}

void Character::SetMovementLinkedToAnimation(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, yesOrNo);
	AGS3::Character_SetMovementLinkedToAnimation(chaa, yesOrNo);
}

void Character::GetMoving(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetMoving(chaa);
}

void Character::GetName(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetName(chaa);
}

void Character::SetName(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, const char *, newName);
	AGS3::Character_SetName(chaa, newName);
}

void Character::GetNormalView(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetNormalView(chaa);
}

void Character::GetPreviousRoom(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetPreviousRoom(chaa);
}

void Character::GetRoom(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetRoom(chaa);
}

void Character::GetScaleMoveSpeed(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetScaleMoveSpeed(chaa);
}

void Character::SetScaleMoveSpeed(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, yesOrNo);
	AGS3::Character_SetScaleMoveSpeed(chaa, yesOrNo);
}

void Character::GetScaleVolume(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetScaleVolume(chaa);
}

void Character::SetScaleVolume(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, yesOrNo);
	AGS3::Character_SetScaleVolume(chaa, yesOrNo);
}

void Character::GetScaling(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetScaling(chaa);
}

void Character::SetScaling(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, zoomLevel);
	AGS3::Character_SetScaling(chaa, zoomLevel);
}

void Character::GetSolid(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetSolid(chaa);
}

void Character::SetSolid(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, yesOrNo);
	AGS3::Character_SetSolid(chaa, yesOrNo);
}

void Character::GetSpeaking(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetSpeaking(chaa);
}

void Character::GetSpeakingFrame(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetSpeakingFrame(chaa);
}

void Character::GetCharacterSpeechAnimationDelay(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::GetCharacterSpeechAnimationDelay(chaa);
}

void Character::SetSpeechAnimationDelay(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, newDelay);
	AGS3::Character_SetSpeechAnimationDelay(chaa, newDelay);
}

void Character::GetSpeechColor(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetSpeechColor(chaa);
}

void Character::SetSpeechColor(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, ncol);
	AGS3::Character_SetSpeechColor(chaa, ncol);
}

void Character::GetSpeechView(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetSpeechView(chaa);
}

void Character::SetSpeechView(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, vii);
	AGS3::Character_SetSpeechView(chaa, vii);
}

void Character::GetThinkView(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetThinkView(chaa);
}

void Character::SetThinkView(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, vii);
	AGS3::Character_SetThinkView(chaa, vii);
}

void Character::GetTransparency(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetTransparency(chaa);
}

void Character::SetTransparency(ScriptMethodParams &params) {
}

void Character::GetTurnBeforeWalking(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetTurnBeforeWalking(chaa);
}

void Character::SetTurnBeforeWalking(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, yesOrNo);
	AGS3::Character_SetTurnBeforeWalking(chaa, yesOrNo);
}

void Character::GetView(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetView(chaa);
}

void Character::GetWalkSpeedX(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetWalkSpeedX(chaa);
}

void Character::GetWalkSpeedY(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetWalkSpeedY(chaa);
}

void Character::GetX(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetX(chaa);
}

void Character::SetX(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, newVal);
	AGS3::Character_SetX(chaa, newVal);
}

void Character::GetY(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetY(chaa);
}

void Character::SetY(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, newVal);
	AGS3::Character_SetY(chaa, newVal);
}

void Character::GetZ(ScriptMethodParams &params) {
	PARAMS1(CharacterInfo *, chaa);
	params._result = AGS3::Character_GetZ(chaa);
}

void Character::SetZ(ScriptMethodParams &params) {
	PARAMS2(CharacterInfo *, chaa, int, newVal);
	AGS3::Character_SetZ(chaa, newVal);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
