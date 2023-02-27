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

#ifndef AGS_PLUGINS_CORE_CHARACTER_H
#define AGS_PLUGINS_CORE_CHARACTER_H

#include "ags/plugins/ags_plugin.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class Character : public ScriptContainer {
	BUILT_IN_HASH(Character)
public:
	virtual ~Character() {}
	void AGS_EngineStartup(IAGSEngine *engine) override;

	void AddInventory(ScriptMethodParams &params);
	void AddWaypoint(ScriptMethodParams &params);
	void Animate(ScriptMethodParams &params);
	void ChangeRoom(ScriptMethodParams &params);
	void ChangeRoomAutoPosition(ScriptMethodParams &params);
	void ChangeView(ScriptMethodParams &params);
	void FaceCharacter(ScriptMethodParams &params);
	void FaceDirection(ScriptMethodParams &params);
	void FaceLocation(ScriptMethodParams &params);
	void FaceObject(ScriptMethodParams &params);
	void FollowCharacter(ScriptMethodParams &params);
	void GetProperty(ScriptMethodParams &params);
	void GetPropertyText(ScriptMethodParams &params);
	void GetTextProperty(ScriptMethodParams &params);
	void HasInventory(ScriptMethodParams &params);
	void IsCollidingWithChar(ScriptMethodParams &params);
	void IsCollidingWithObject(ScriptMethodParams &params);
	void LockView(ScriptMethodParams &params);
	void LockViewEx(ScriptMethodParams &params);
	void LockViewAligned_Old(ScriptMethodParams &params);
	void LockViewAlignedEx_Old(ScriptMethodParams &params);
	void LockViewAligned(ScriptMethodParams &params);
	void LockViewAlignedEx(ScriptMethodParams &params);
	void LockViewFrame(ScriptMethodParams &params);
	void LockViewFrameEx(ScriptMethodParams &params);
	void LockViewOffset(ScriptMethodParams &params);
	void LoseInventory(ScriptMethodParams &params);
	void Move(ScriptMethodParams &params);
	void PlaceOnWalkableArea(ScriptMethodParams &params);
	void RemoveTint(ScriptMethodParams &params);
	void RunInteraction(ScriptMethodParams &params);
	void ScPl_Say(ScriptMethodParams &params);
	void SayAt(ScriptMethodParams &params);
	void SayBackground(ScriptMethodParams &params);
	void SetAsPlayer(ScriptMethodParams &params);
	void SetIdleView(ScriptMethodParams &params);
	void SetProperty(ScriptMethodParams &params);
	void SetTextProperty(ScriptMethodParams &params);
	void SetSpeed(ScriptMethodParams &params);
	void StopMoving(ScriptMethodParams &params);
	void ScPl_Think(ScriptMethodParams &params);
	void Tint(ScriptMethodParams &params);
	void UnlockView(ScriptMethodParams &params);
	void UnlockViewEx(ScriptMethodParams &params);
	void Walk(ScriptMethodParams &params);
	void WalkStraight(ScriptMethodParams &params);
	void GetCharacterAtRoom(ScriptMethodParams &params);
	void GetCharacterAtScreen(ScriptMethodParams &params);
	void GetActiveInventory(ScriptMethodParams &params);
	void SetActiveInventory(ScriptMethodParams &params);
	void GetAnimating(ScriptMethodParams &params);
	void GetAnimationSpeed(ScriptMethodParams &params);
	void SetAnimationSpeed(ScriptMethodParams &params);
	void GetBaseline(ScriptMethodParams &params);
	void SetBaseline(ScriptMethodParams &params);
	void GetBlinkInterval(ScriptMethodParams &params);
	void SetBlinkInterval(ScriptMethodParams &params);
	void GetBlinkView(ScriptMethodParams &params);
	void SetBlinkView(ScriptMethodParams &params);
	void GetBlinkWhileThinking(ScriptMethodParams &params);
	void SetBlinkWhileThinking(ScriptMethodParams &params);
	void GetBlockingHeight(ScriptMethodParams &params);
	void SetBlockingHeight(ScriptMethodParams &params);
	void GetBlockingWidth(ScriptMethodParams &params);
	void SetBlockingWidth(ScriptMethodParams &params);
	void GetClickable(ScriptMethodParams &params);
	void SetClickable(ScriptMethodParams &params);
	void GetDestinationX(ScriptMethodParams &params);
	void GetDestinationY(ScriptMethodParams &params);
	void GetDiagonalWalking(ScriptMethodParams &params);
	void SetDiagonalWalking(ScriptMethodParams &params);
	void GetFrame(ScriptMethodParams &params);
	void SetFrame(ScriptMethodParams &params);
	void GetHasExplicitTint_Old(ScriptMethodParams &params);
	void GetHasExplicitTint(ScriptMethodParams &params);
	void GetID(ScriptMethodParams &params);
	void GetIdleView(ScriptMethodParams &params);
	void GetIInventoryQuantity(ScriptMethodParams &params);
	void SetIInventoryQuantity(ScriptMethodParams &params);
	void GetIgnoreLighting(ScriptMethodParams &params);
	void SetIgnoreLighting(ScriptMethodParams &params);
	void GetIgnoreScaling(ScriptMethodParams &params);
	void SetIgnoreScaling(ScriptMethodParams &params);
	void GetIgnoreWalkbehinds(ScriptMethodParams &params);
	void SetIgnoreWalkbehinds(ScriptMethodParams &params);
	void GetLoop(ScriptMethodParams &params);
	void SetLoop(ScriptMethodParams &params);
	void SetManualScaling(ScriptMethodParams &params);
	void GetMovementLinkedToAnimation(ScriptMethodParams &params);
	void SetMovementLinkedToAnimation(ScriptMethodParams &params);
	void GetMoving(ScriptMethodParams &params);
	void GetName(ScriptMethodParams &params);
	void SetName(ScriptMethodParams &params);
	void GetNormalView(ScriptMethodParams &params);
	void GetPreviousRoom(ScriptMethodParams &params);
	void GetRoom(ScriptMethodParams &params);
	void GetScaleMoveSpeed(ScriptMethodParams &params);
	void SetScaleMoveSpeed(ScriptMethodParams &params);
	void GetScaleVolume(ScriptMethodParams &params);
	void SetScaleVolume(ScriptMethodParams &params);
	void GetScaling(ScriptMethodParams &params);
	void SetScaling(ScriptMethodParams &params);
	void GetSolid(ScriptMethodParams &params);
	void SetSolid(ScriptMethodParams &params);
	void GetSpeaking(ScriptMethodParams &params);
	void GetSpeakingFrame(ScriptMethodParams &params);
	void GetCharacterSpeechAnimationDelay(ScriptMethodParams &params);
	void SetSpeechAnimationDelay(ScriptMethodParams &params);
	void GetSpeechColor(ScriptMethodParams &params);
	void SetSpeechColor(ScriptMethodParams &params);
	void GetSpeechView(ScriptMethodParams &params);
	void SetSpeechView(ScriptMethodParams &params);
	void GetThinkView(ScriptMethodParams &params);
	void SetThinkView(ScriptMethodParams &params);
	void GetTransparency(ScriptMethodParams &params);
	void SetTransparency(ScriptMethodParams &params);
	void GetTurnBeforeWalking(ScriptMethodParams &params);
	void SetTurnBeforeWalking(ScriptMethodParams &params);
	void GetView(ScriptMethodParams &params);
	void GetWalkSpeedX(ScriptMethodParams &params);
	void GetWalkSpeedY(ScriptMethodParams &params);
	void GetX(ScriptMethodParams &params);
	void SetX(ScriptMethodParams &params);
	void GetY(ScriptMethodParams &params);
	void SetY(ScriptMethodParams &params);
	void GetZ(ScriptMethodParams &params);
	void SetZ(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
