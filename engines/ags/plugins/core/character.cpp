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

#include "ags/plugins/core/character.h"
#include "ags/engine/ac/character.h"
#include "ags/engine/ac/global_character.h"
#include "ags/shared/ac/game_struct_defines.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Character::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(Character::AddInventory^2, Character::AddInventory);
	SCRIPT_METHOD(Character::AddWaypoint^2, Character::AddWaypoint);
	SCRIPT_METHOD(Character::Animate^5, Character::Animate);
	SCRIPT_METHOD(Character::ChangeRoom^3, Character::ChangeRoom);
	SCRIPT_METHOD(Character::ChangeRoomAutoPosition^2, Character::ChangeRoomAutoPosition);
	SCRIPT_METHOD(Character::ChangeView^1, Character::ChangeView);
	SCRIPT_METHOD(Character::FaceCharacter^2, Character::FaceCharacter);
	SCRIPT_METHOD(Character::FaceDirection^2, Character::FaceDirection);
	SCRIPT_METHOD(Character::FaceLocation^3, Character::FaceLocation);
	SCRIPT_METHOD(Character::FaceObject^2, Character::FaceObject);
	SCRIPT_METHOD(Character::FollowCharacter^3, Character::FollowCharacter);
	SCRIPT_METHOD(Character::GetProperty^1, Character::GetProperty);
	SCRIPT_METHOD(Character::GetPropertyText^2, Character::GetPropertyText);
	SCRIPT_METHOD(Character::GetTextProperty^1, Character::GetTextProperty);
	SCRIPT_METHOD(Character::HasInventory^1, Character::HasInventory);
	SCRIPT_METHOD(Character::IsCollidingWithChar^1, Character::IsCollidingWithChar);
	SCRIPT_METHOD(Character::IsCollidingWithObject^1, Character::IsCollidingWithObject);
	SCRIPT_METHOD(Character::LockView^1, Character::LockView);
	SCRIPT_METHOD(Character::LockView^2, Character::LockViewEx);
	if (engine->version < kScriptAPI_v341) {
		SCRIPT_METHOD(Character::LockViewAligned^3, Character::LockViewAligned_Old);
		SCRIPT_METHOD(Character::LockViewAligned^4, Character::LockViewAlignedEx_Old);
	} else {
		SCRIPT_METHOD(Character::LockViewAligned^3, Character::LockViewAligned);
		SCRIPT_METHOD(Character::LockViewAligned^4, Character::LockViewAlignedEx);
	}
	SCRIPT_METHOD(Character::LockViewFrame^3, Character::LockViewFrame);
	SCRIPT_METHOD(Character::LockViewFrame^4, Character::LockViewFrameEx);
	SCRIPT_METHOD(Character::LockViewOffset^3, Character::LockViewOffset);
	SCRIPT_METHOD(Character::LockViewOffset^4, Character::LockViewOffset);
	SCRIPT_METHOD(Character::LoseInventory^1, Character::LoseInventory);
	SCRIPT_METHOD(Character::Move^4, Character::Move);
	SCRIPT_METHOD(Character::PlaceOnWalkableArea^0, Character::PlaceOnWalkableArea);
	SCRIPT_METHOD(Character::RemoveTint^0, Character::RemoveTint);
	SCRIPT_METHOD(Character::RunInteraction^1, Character::RunInteraction);
	SCRIPT_METHOD(Character::Say^101, Character::ScPl_Say);
	SCRIPT_METHOD(Character::SayAt^4, Character::SayAt);
	SCRIPT_METHOD(Character::SayBackground^1, Character::SayBackground);
	SCRIPT_METHOD(Character::SetAsPlayer^0, Character::SetAsPlayer);
	SCRIPT_METHOD(Character::SetIdleView^2, Character::SetIdleView);
	//SCRIPT_METHOD(Character::SetOption^2", Character::            (void*)SetOption);
	SCRIPT_METHOD(Character::SetProperty^2, Character::SetProperty);
	SCRIPT_METHOD(Character::SetTextProperty^2, Character::SetTextProperty);
	SCRIPT_METHOD(Character::SetWalkSpeed^2, Character::SetSpeed);
	SCRIPT_METHOD(Character::StopMoving^0, Character::StopMoving);
	SCRIPT_METHOD(Character::Think^101, Character::ScPl_Think);
	SCRIPT_METHOD(Character::Tint^5, Character::Tint);
	SCRIPT_METHOD(Character::UnlockView^0, Character::UnlockView);
	SCRIPT_METHOD(Character::UnlockView^1, Character::UnlockViewEx);
	SCRIPT_METHOD(Character::Walk^4, Character::Walk);
	SCRIPT_METHOD(Character::WalkStraight^3, Character::WalkStraight);
	SCRIPT_METHOD(Character::GetAtRoomXY^2, Character::GetCharacterAtRoom);
	SCRIPT_METHOD(Character::GetAtScreenXY^2, Character::GetCharacterAtScreen);
	SCRIPT_METHOD(Character::get_ActiveInventory, Character::GetActiveInventory);
	SCRIPT_METHOD(Character::set_ActiveInventory, Character::SetActiveInventory);
	SCRIPT_METHOD(Character::get_Animating, Character::GetAnimating);
	SCRIPT_METHOD(Character::get_AnimationSpeed, Character::GetAnimationSpeed);
	SCRIPT_METHOD(Character::set_AnimationSpeed, Character::SetAnimationSpeed);
	SCRIPT_METHOD(Character::get_Baseline, Character::GetBaseline);
	SCRIPT_METHOD(Character::set_Baseline, Character::SetBaseline);
	SCRIPT_METHOD(Character::get_BlinkInterval, Character::GetBlinkInterval);
	SCRIPT_METHOD(Character::set_BlinkInterval, Character::SetBlinkInterval);
	SCRIPT_METHOD(Character::get_BlinkView, Character::GetBlinkView);
	SCRIPT_METHOD(Character::set_BlinkView, Character::SetBlinkView);
	SCRIPT_METHOD(Character::get_BlinkWhileThinking, Character::GetBlinkWhileThinking);
	SCRIPT_METHOD(Character::set_BlinkWhileThinking, Character::SetBlinkWhileThinking);
	SCRIPT_METHOD(Character::get_BlockingHeight, Character::GetBlockingHeight);
	SCRIPT_METHOD(Character::set_BlockingHeight, Character::SetBlockingHeight);
	SCRIPT_METHOD(Character::get_BlockingWidth, Character::GetBlockingWidth);
	SCRIPT_METHOD(Character::set_BlockingWidth, Character::SetBlockingWidth);
	SCRIPT_METHOD(Character::get_Clickable, Character::GetClickable);
	SCRIPT_METHOD(Character::set_Clickable, Character::SetClickable);
	SCRIPT_METHOD(Character::get_DestinationX, Character::GetDestinationX);
	SCRIPT_METHOD(Character::get_DestinationY, Character::GetDestinationY);
	SCRIPT_METHOD(Character::get_DiagonalLoops, Character::GetDiagonalWalking);
	SCRIPT_METHOD(Character::set_DiagonalLoops, Character::SetDiagonalWalking);
	SCRIPT_METHOD(Character::get_Frame, Character::GetFrame);
	SCRIPT_METHOD(Character::set_Frame, Character::SetFrame);
	if (engine->version < kScriptAPI_v341)
		SCRIPT_METHOD(Character::get_HasExplicitTint, Character::GetHasExplicitTint_Old);
	else
		SCRIPT_METHOD(Character::get_HasExplicitTint, Character::GetHasExplicitTint);
	SCRIPT_METHOD(Character::get_ID, Character::GetID);
	SCRIPT_METHOD(Character::get_IdleView, Character::GetIdleView);
	SCRIPT_METHOD(Character::geti_InventoryQuantity, Character::GetIInventoryQuantity);
	SCRIPT_METHOD(Character::seti_InventoryQuantity, Character::SetIInventoryQuantity);
	SCRIPT_METHOD(Character::get_IgnoreLighting, Character::GetIgnoreLighting);
	SCRIPT_METHOD(Character::set_IgnoreLighting, Character::SetIgnoreLighting);
	SCRIPT_METHOD(Character::get_IgnoreScaling, Character::GetIgnoreScaling);
	SCRIPT_METHOD(Character::set_IgnoreScaling, Character::SetIgnoreScaling);
	SCRIPT_METHOD(Character::get_IgnoreWalkbehinds, Character::GetIgnoreWalkbehinds);
	SCRIPT_METHOD(Character::set_IgnoreWalkbehinds, Character::SetIgnoreWalkbehinds);
	SCRIPT_METHOD(Character::get_Loop, Character::GetLoop);
	SCRIPT_METHOD(Character::set_Loop, Character::SetLoop);
	SCRIPT_METHOD(Character::get_ManualScaling, Character::GetIgnoreScaling);
	SCRIPT_METHOD(Character::set_ManualScaling, Character::SetManualScaling);
	SCRIPT_METHOD(Character::get_MovementLinkedToAnimation, Character::GetMovementLinkedToAnimation);
	SCRIPT_METHOD(Character::set_MovementLinkedToAnimation, Character::SetMovementLinkedToAnimation);
	SCRIPT_METHOD(Character::get_Moving, Character::GetMoving);
	SCRIPT_METHOD(Character::get_Name, Character::GetName);
	SCRIPT_METHOD(Character::set_Name, Character::SetName);
	SCRIPT_METHOD(Character::get_NormalView, Character::GetNormalView);
	SCRIPT_METHOD(Character::get_PreviousRoom, Character::GetPreviousRoom);
	SCRIPT_METHOD(Character::get_Room, Character::GetRoom);
	SCRIPT_METHOD(Character::get_ScaleMoveSpeed, Character::GetScaleMoveSpeed);
	SCRIPT_METHOD(Character::set_ScaleMoveSpeed, Character::SetScaleMoveSpeed);
	SCRIPT_METHOD(Character::get_ScaleVolume, Character::GetScaleVolume);
	SCRIPT_METHOD(Character::set_ScaleVolume, Character::SetScaleVolume);
	SCRIPT_METHOD(Character::get_Scaling, Character::GetScaling);
	SCRIPT_METHOD(Character::set_Scaling, Character::SetScaling);
	SCRIPT_METHOD(Character::get_Solid, Character::GetSolid);
	SCRIPT_METHOD(Character::set_Solid, Character::SetSolid);
	SCRIPT_METHOD(Character::get_Speaking, Character::GetSpeaking);
	SCRIPT_METHOD(Character::get_SpeakingFrame, Character::GetSpeakingFrame);
	SCRIPT_METHOD(Character::get_SpeechAnimationDelay, Character::GetCharacterSpeechAnimationDelay);
	SCRIPT_METHOD(Character::set_SpeechAnimationDelay, Character::SetSpeechAnimationDelay);
	SCRIPT_METHOD(Character::get_SpeechColor, Character::GetSpeechColor);
	SCRIPT_METHOD(Character::set_SpeechColor, Character::SetSpeechColor);
	SCRIPT_METHOD(Character::get_SpeechView, Character::GetSpeechView);
	SCRIPT_METHOD(Character::set_SpeechView, Character::SetSpeechView);
	SCRIPT_METHOD(Character::get_ThinkView, Character::GetThinkView);
	SCRIPT_METHOD(Character::set_ThinkView, Character::SetThinkView);
	SCRIPT_METHOD(Character::get_Transparency, Character::GetTransparency);
	SCRIPT_METHOD(Character::set_Transparency, Character::SetTransparency);
	SCRIPT_METHOD(Character::get_TurnBeforeWalking, Character::GetTurnBeforeWalking);
	SCRIPT_METHOD(Character::set_TurnBeforeWalking, Character::SetTurnBeforeWalking);
	SCRIPT_METHOD(Character::get_View, Character::GetView);
	SCRIPT_METHOD(Character::get_WalkSpeedX, Character::GetWalkSpeedX);
	SCRIPT_METHOD(Character::get_WalkSpeedY, Character::GetWalkSpeedY);
	SCRIPT_METHOD(Character::get_X, Character::GetX);
	SCRIPT_METHOD(Character::set_X, Character::SetX);
	SCRIPT_METHOD(Character::get_x, Character::GetX);
	SCRIPT_METHOD(Character::set_x, Character::SetX);
	SCRIPT_METHOD(Character::get_Y, Character::GetY);
	SCRIPT_METHOD(Character::set_Y, Character::SetY);
	SCRIPT_METHOD(Character::get_y, Character::GetY);
	SCRIPT_METHOD(Character::set_y, Character::SetY);
	SCRIPT_METHOD(Character::get_Z, Character::GetZ);
	SCRIPT_METHOD(Character::set_Z, Character::SetZ);
	SCRIPT_METHOD(Character::get_z, Character::GetZ);
	SCRIPT_METHOD(Character::set_z, Character::SetZ);
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

void Character::SetProperty(ScriptMethodParams &params) {
	PARAMS3(CharacterInfo *, chaa, const char *, property, int, value);
	params._result = AGS3::Character_SetProperty(chaa, property, value);
}

void Character::SetTextProperty(ScriptMethodParams &params) {
	PARAMS3(CharacterInfo *, chaa, const char *, property, const char *, value);
	params._result = AGS3::Character_SetTextProperty(chaa, property, value);
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
