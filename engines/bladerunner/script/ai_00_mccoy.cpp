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

#include "bladerunner/script/ai_00_mccoy.h"

namespace BladeRunner {

AIScript_McCoy::AIScript_McCoy(BladeRunnerEngine *vm)
	: AIScriptBase(vm)
{}

void AIScript_McCoy::Initialize()
{
	dword_45A0D0_animation_state = 0;
	dword_45A0D4 = 0;
	dword_45A0D8 = 0;
	dword_45A0DC = 30;
	dword_45A0E0 = 1;
	dword_45A0E4 = 0;
	dword_45A0E8 = 3;
}

void AIScript_McCoy::UpdateAnimation(int *animation, int *frame)
{
	*animation = *frame = 0;
	switch (dword_45A0D0_animation_state)
	{
		case 0:
			*animation = 19;
			if (dword_45A0D8 < dword_45A0DC)
			{
				*frame = dword_45A0E8;
				dword_45A0D4 += dword_45A0E0;
				if (dword_45A0D4 > dword_45A0E8)
				{
					dword_45A0D4 = dword_45A0E8;
					dword_45A0E0 = -1;
				}
				else
				{
					*frame = dword_45A0D4;
					if (dword_45A0D4 < dword_45A0E4)
					{
						dword_45A0D4 = dword_45A0E4;
						dword_45A0E0 = 1;
						*frame = dword_45A0E4;
					}
				}
				dword_45A0D8++;
			}
			else
			{
				dword_45A0D4 += dword_45A0E0;
				dword_45A0DC = 0;
				if (dword_45A0D4 == 18 && Random_Query(0, 2))
				{
					dword_45A0E0 = -1;
					dword_45A0D8 = 0;
					dword_45A0E4 = 14;
					dword_45A0E8 = 18;
					dword_45A0DC = Random_Query(0, 30);
				}
				if (dword_45A0D4 == 26)
				{
					if (Random_Query(0, 2))
					{
						dword_45A0E0 = -1;
						dword_45A0D8 = 0;
						dword_45A0E4 = 23;
						dword_45A0E8 = 26;
						dword_45A0DC = Random_Query(0, 30);
						}
				}
				if (dword_45A0D4 >= Slice_Animation_Query_Number_Of_Frames(19))
				{
					dword_45A0D4 = 0;
					if (Random_Query(0, 2))
					{
						dword_45A0D8 = 0;
						dword_45A0E4 = 0;
						dword_45A0E8 = 3;
						dword_45A0DC = Random_Query(0, 45);
					}
				}
				*frame = dword_45A0D4;
				if (dword_45A0D4 < 0)
				{
					*frame = Slice_Animation_Query_Number_Of_Frames(19) - 1;
					dword_45A0D4 = *frame;
				}
			}
			break;

		// Continue walking (follows state 32)
		case 30:
			*animation = 13;
			if (++dword_45A0D4 >= Slice_Animation_Query_Number_Of_Frames(13))
				dword_45A0D4 = 0;
			*frame = dword_45A0D4;

			if (dword_45A0D4 == 2)
				; //Sound_Right_Footstep_Walk(0);
			else if (dword_45A0D4 == 10)
				; //Sound_Left_Footstep_Walk(0);

			break;

		// Start walking
		case 32:
			dword_45A0D4 = 1;
			dword_45A0D0_animation_state = 30;
			*animation = 13;
			*frame = 1;
			break;

		case 58:
			*animation = 47;
			if (dword_45A0D4++ == 5) {
				int stepSound;
				switch (Random_Query(0, 2)) {
				case 0:
					stepSound = 595;
					break;
				case 1:
					stepSound = 594;
					break;
				default:
					stepSound = 593;
				}
				(void)stepSound;
				// Ambient_Sounds_Play_Sound(stepSound, 39, 0, 0, 99);
			}
			if (dword_45A0D4 > Slice_Animation_Query_Number_Of_Frames(*animation) - 2)
				dword_45A0D4 = 0;
			*frame = dword_45A0D4;
			break;
	}
}

void AIScript_McCoy::ChangeAnimationMode(int mode)
{
	switch (mode)
	{
		case 0:
			dword_45A0D0_animation_state = 0;
			dword_45A0D4 = 0;
			break;

		case 1:
			dword_45A0D0_animation_state = 32;
			break;
	}
}

} // End of namespace BladeRunner
