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

#include "bladerunner/script/script.h"

#include "bladerunner/bladerunner.h"

namespace BladeRunner {

class AIScript_Officer_Leroy : public AIScriptBase {
	int var_45D5B0_animation_state;
	int var_45D5B4_frame;
	int var_45D5B8;
public:
	AIScript_Officer_Leroy(BladeRunnerEngine *vm);

	void Initialize();
	bool Update();
	void TimerExpired(int timer);
	void CompletedMovementTrack();
	void ReceivedClue(int clueId, int fromActorId);
	void ClickedByPlayer();
	void EnteredScene(int sceneId);
	void OtherAgentEnteredThisScene();
	void OtherAgentExitedThisScene();
	void OtherAgentEnteredCombatMode();
	void ShotAtAndMissed();
	void ShotAtAndHit();
	void Retired(int byActorId);
	void GetFriendlinessModifierIfGetsClue();
	bool GoalChanged(int currentGoalNumber, int newGoalNumber);
	bool UpdateAnimation(int *animation, int *frame);
	bool ChangeAnimationMode(int mode);
	void QueryAnimationState(int *animationState, int *a2, int *a3, int *a4);
	void SetAnimationState(int animationState, int a2, int a3, int a4);
	bool ReachedMovementTrackWaypoint();
};

} // End of namespace BladeRunner
