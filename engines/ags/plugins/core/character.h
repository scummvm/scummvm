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

#ifndef AGS_PLUGINS_CORE_CHARACTER_H
#define AGS_PLUGINS_CORE_CHARACTER_H

#include "ags/plugins/plugin_base.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class Character : public ScriptContainer {
public:
	void AGS_EngineStartup(IAGSEngine *engine);

	static void AddInventory(ScriptMethodParams &params);
	static void AddWaypoint(ScriptMethodParams &params);
	static void Animate(ScriptMethodParams &params);
	static void ChangeRoom(ScriptMethodParams &params);
	static void ChangeRoomAutoPosition(ScriptMethodParams &params);
	static void ChangeView(ScriptMethodParams &params);
	static void FaceCharacter(ScriptMethodParams &params);
	static void FaceDirection(ScriptMethodParams &params);
	static void FaceLocation(ScriptMethodParams &params);
	static void FaceObject(ScriptMethodParams &params);
	static void FollowCharacter(ScriptMethodParams &params);
	static void GetProperty(ScriptMethodParams &params);
	static void GetPropertyText(ScriptMethodParams &params);
	static void GetTextProperty(ScriptMethodParams &params);
	static void HasInventory(ScriptMethodParams &params);
	static void IsCollidingWithChar(ScriptMethodParams &params);
	static void IsCollidingWithObject(ScriptMethodParams &params);
	static void LockView(ScriptMethodParams &params);
	static void LockViewEx(ScriptMethodParams &params);
	static void LockViewAligned_Old(ScriptMethodParams &params);
	static void LockViewAlignedEx_Old(ScriptMethodParams &params);
	static void LockViewAligned(ScriptMethodParams &params);
	static void LockViewAlignedEx(ScriptMethodParams &params);
	static void LockViewFrame(ScriptMethodParams &params);
	static void LockViewFrameEx(ScriptMethodParams &params);
	static void LockViewOffset(ScriptMethodParams &params);
	static void LoseInventory(ScriptMethodParams &params);
	static void Move(ScriptMethodParams &params);
	static void PlaceOnWalkableArea(ScriptMethodParams &params);
	static void RemoveTint(ScriptMethodParams &params);
	static void RunInteraction(ScriptMethodParams &params);
	static void ScPl_Say(ScriptMethodParams &params);
	static void SayAt(ScriptMethodParams &params);
	static void SayBackground(ScriptMethodParams &params);
	static void SetAsPlayer(ScriptMethodParams &params);
	static void SetIdleView(ScriptMethodParams &params);
	static void SetSpeed(ScriptMethodParams &params);
	static void StopMoving(ScriptMethodParams &params);
	static void ScPl_Think(ScriptMethodParams &params);
	static void Tint(ScriptMethodParams &params);
	static void UnlockView(ScriptMethodParams &params);
	static void UnlockViewEx(ScriptMethodParams &params);
	static void Walk(ScriptMethodParams &params);
	static void WalkStraight(ScriptMethodParams &params);
	static void GetCharacterAtRoom(ScriptMethodParams &params);
	static void GetCharacterAtScreen(ScriptMethodParams &params);
	static void GetActiveInventory(ScriptMethodParams &params);
	static void SetActiveInventory(ScriptMethodParams &params);
	static void GetAnimating(ScriptMethodParams &params);
	static void GetAnimationSpeed(ScriptMethodParams &params);
	static void SetAnimationSpeed(ScriptMethodParams &params);
	static void GetBaseline(ScriptMethodParams &params);
	static void SetBaseline(ScriptMethodParams &params);
	static void GetBlinkInterval(ScriptMethodParams &params);
	static void SetBlinkInterval(ScriptMethodParams &params);
	static void GetBlinkView(ScriptMethodParams &params);
	static void SetBlinkView(ScriptMethodParams &params);
	static void GetBlinkWhileThinking(ScriptMethodParams &params);
	static void SetBlinkWhileThinking(ScriptMethodParams &params);
	static void GetBlockingHeight(ScriptMethodParams &params);
	static void SetBlockingHeight(ScriptMethodParams &params);
	static void GetBlockingWidth(ScriptMethodParams &params);
	static void SetBlockingWidth(ScriptMethodParams &params);
	static void GetClickable(ScriptMethodParams &params);
	static void SetClickable(ScriptMethodParams &params);
	static void GetDestinationX(ScriptMethodParams &params);
	static void GetDestinationY(ScriptMethodParams &params);
	static void GetDiagonalWalking(ScriptMethodParams &params);
	static void SetDiagonalWalking(ScriptMethodParams &params);
	static void GetFrame(ScriptMethodParams &params);
	static void SetFrame(ScriptMethodParams &params);
	static void GetHasExplicitTint_Old(ScriptMethodParams &params);
	static void GetHasExplicitTint(ScriptMethodParams &params);
	static void GetID(ScriptMethodParams &params);
	static void GetIdleView(ScriptMethodParams &params);
	static void GetIInventoryQuantity(ScriptMethodParams &params);
	static void SetIInventoryQuantity(ScriptMethodParams &params);
	static void GetIgnoreLighting(ScriptMethodParams &params);
	static void SetIgnoreLighting(ScriptMethodParams &params);
	static void GetIgnoreScaling(ScriptMethodParams &params);
	static void SetIgnoreScaling(ScriptMethodParams &params);
	static void GetIgnoreWalkbehinds(ScriptMethodParams &params);
	static void SetIgnoreWalkbehinds(ScriptMethodParams &params);
	static void GetLoop(ScriptMethodParams &params);
	static void SetLoop(ScriptMethodParams &params);
	static void SetManualScaling(ScriptMethodParams &params);
	static void GetMovementLinkedToAnimation(ScriptMethodParams &params);
	static void SetMovementLinkedToAnimation(ScriptMethodParams &params);
	static void GetMoving(ScriptMethodParams &params);
	static void GetName(ScriptMethodParams &params);
	static void SetName(ScriptMethodParams &params);
	static void GetNormalView(ScriptMethodParams &params);
	static void GetPreviousRoom(ScriptMethodParams &params);
	static void GetRoom(ScriptMethodParams &params);
	static void GetScaleMoveSpeed(ScriptMethodParams &params);
	static void SetScaleMoveSpeed(ScriptMethodParams &params);
	static void GetScaleVolume(ScriptMethodParams &params);
	static void SetScaleVolume(ScriptMethodParams &params);
	static void GetScaling(ScriptMethodParams &params);
	static void SetScaling(ScriptMethodParams &params);
	static void GetSolid(ScriptMethodParams &params);
	static void SetSolid(ScriptMethodParams &params);
	static void GetSpeaking(ScriptMethodParams &params);
	static void GetSpeakingFrame(ScriptMethodParams &params);
	static void GetCharacterSpeechAnimationDelay(ScriptMethodParams &params);
	static void SetSpeechAnimationDelay(ScriptMethodParams &params);
	static void GetSpeechColor(ScriptMethodParams &params);
	static void SetSpeechColor(ScriptMethodParams &params);
	static void GetSpeechView(ScriptMethodParams &params);
	static void SetSpeechView(ScriptMethodParams &params);
	static void GetThinkView(ScriptMethodParams &params);
	static void SetThinkView(ScriptMethodParams &params);
	static void GetTransparency(ScriptMethodParams &params);
	static void SetTransparency(ScriptMethodParams &params);
	static void GetTurnBeforeWalking(ScriptMethodParams &params);
	static void SetTurnBeforeWalking(ScriptMethodParams &params);
	static void GetView(ScriptMethodParams &params);
	static void GetWalkSpeedX(ScriptMethodParams &params);
	static void GetWalkSpeedY(ScriptMethodParams &params);
	static void GetX(ScriptMethodParams &params);
	static void SetX(ScriptMethodParams &params);
	static void GetY(ScriptMethodParams &params);
	static void SetY(ScriptMethodParams &params);
	static void GetZ(ScriptMethodParams &params);
	static void SetZ(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
