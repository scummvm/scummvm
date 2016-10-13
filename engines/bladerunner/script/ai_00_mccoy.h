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
class AIScript_McCoy : public AIScriptBase {
	int dword_45A0D0_animation_state;
	int dword_45A0D4_animation_frame;
	int dword_45A0D8;
	int dword_45A0DC;
	int dword_45A0E0;
	int dword_45A0E4;
	float off_45A0EC;
	int dword_45A0E8;
	int dword_45A0F0;
	int dword_45A0F4;
	int dword_45A0F8;
	int dword_45A0FC;
	int dword_462718;
	int dword_46271C;
	float off_45A100;
	float flt_462710;
	float flt_462714;

public:
	AIScript_McCoy(BladeRunnerEngine *vm);

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

private:
	void sub_4053E0();
	void sub_4054F0();
	void sub_405660();
	void sub_405800();
	void sub_4058B0();
	void sub_405920();
	void sub_405940(float a1);
	void sub_4059D0(float a1);
	
};
} // End of namespace BladeRunner
