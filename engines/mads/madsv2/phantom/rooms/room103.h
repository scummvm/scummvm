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

#ifndef MADS_PHANTOM_ROOMS_103_H
#define MADS_PHANTOM_ROOMS_103_H

#include "mads/madsv2/core/general.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

#define local ((Scratch *)(&game.scratch[0]))
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

typedef struct {        /* Room local variables */
	int sprite[20];       /* Sprite series handles  */
	int sequence[20];     /* Sequence handles       */
	int animation[7];     /* Animation handles      */
	int prompt_1;         /* Dynamic var for prompter's stand */
	int prompt_2;         /* Dynamic var for prompter's stand */
	int prompt_3;         /* Dynamic var for prompter's stand */
	int prompt_4;         /* Dynamic var for prompter's stand */
	int prompt_5;         /* Dynamic var for prompter's stand */
	int floor_r_1;        /* Dynamic var for floor  */
	int floor_r_2;        /* Dynamic var for floor  */
	int floor_l_1;        /* Dynamic var for floor  */
	int floor_l_2;        /* Dynamic var for floor  */
	int man;              /* Dynamic var for man / Jacques */

	int jacques_frame;
	int jacques_action;
	int jacques_talk_count;

	int just_did_option;
	int anim_0_running;
	int anim_1_running;
	int anim_2_running;
	int anim_3_running;
	int anim_4_running;
	int anim_5_running;
	int anim_6_running;

	int top_of_stand;     /* Either NO, YES_LEFT, or YES_RIGHT */

	int converse_counter; /* counter for talking displacements */
	int frame_guard;      /* guards against running certain daemon code more than once */

	int stairs_frame;
	int climb_thru;
	int sit_on_it;
} Scratch;


/* ========================= Sprite Series =================== */

#define fx_trap_door                0       /* rm103x0  */
#define fx_door                     1       /* rm103x1  */
#define fx_take_6                   2       /* rdr_6    */
#define fx_steps                    3       /* rm103f0  */
#define fx_take_9                   4       /* rdr_9    */
#define fx_key                      5       /* rm103x2  */
#define fx_jacques_body             6       /* rm103c1  */
#define fx_broken_promp             7       /* rm103f1  */
#define fx_bend_down_9              8       /* rrd_9    */
#define fx_lever                    9       /* rm103x3  */
#define fx_on_seat                  10      /* rm103a3  */
#define fx_up_trap_door             11      /* rm103a2  */
#define fx_rail_pieces              12      /* rm103f2  */


/* ======================== Jacques actions ================== */

#define CONV12_JAC_TALK             0
#define CONV12_JAC_SHUT_UP          1
#define CONV12_JAC_POINT            2
#define CONV12_JAC_I_DUNNO_1        3
#define CONV12_JAC_I_DUNNO_2        4


/* ========================= Triggers ======================== */

#define ROOM_103_TRAP_DOOR_OPENS    60
#define ROOM_103_TRAP_DOOR_CLOSES   62
#define ROOM_103_DOOR_CLOSES        65
#define ROOM_103_DOOR_OPENS         70
#define ROOM_103_DONE_MOVING_PROMPT 75
#define ROOM_103_DIED               80
#define ROOM_103_JACQUES_TALK       90
#define ROOM_103_JACQUES_SHUT_UP    92
#define ROOM_103_JACQUES_POINT      94
#define ROOM_103_JACQUES_I_DUNNO_1  96
#define ROOM_103_JACQUES_I_DUNNO_2  98

#define ROOM_103_GET_OFF_SEAT       100

#define ROOM_103_SWITCH_DOWN        105
#define ROOM_103_SWITCH_UP          110
#define ROOM_103_LOOK_THRU_BOX      115
#define ROOM_103_CLIMB_OUT_TRAP     120


/* ======================= Other Macros ====================== */

#define CONV_JACQUES                12

#define PLAYER_X_FROM_102           15
#define PLAYER_Y_FROM_102           147

#define PLAYER_X_FROM_105           287
#define PLAYER_Y_FROM_105           135

#define WALK_TO_X_FROM_105          252
#define WALK_TO_Y_FROM_105          134

#define WALK_TO_X_OPEN_DOOR         320
#define WALK_TO_Y_OPEN_DOOR         132

#define OFF_SCREEN_X_FROM_102       -20
#define OFF_SCREEN_Y_FROM_102       140

#define WALKER_LEFT_PROMPT_X        2
#define WALKER_LEFT_PROMPT_Y        138

#define WALKER_RIGHT_PROMPT_X       176
#define WALKER_RIGHT_PROMPT_Y       142

#define PROMPT_LEFT_X               37
#define PROMPT_LEFT_Y               139

#define PROMPT_RIGHT_X              154
#define PROMPT_RIGHT_Y              139

#define DYN_PROMPT_LEFT_1_X         2
#define DYN_PROMPT_LEFT_1_Y         79
#define DYN_PROMPT_X_SIZE_1         40
#define DYN_PROMPT_Y_SIZE_1         63

#define DYN_PROMPT_LEFT_2_X         42
#define DYN_PROMPT_LEFT_2_Y         67
#define DYN_PROMPT_X_SIZE_2         16
#define DYN_PROMPT_Y_SIZE_2         75

#define DYN_PROMPT_LEFT_3_X         58
#define DYN_PROMPT_LEFT_3_Y         90
#define DYN_PROMPT_X_SIZE_3         18
#define DYN_PROMPT_Y_SIZE_3         52

#define DYN_PROMPT_LEFT_5_X         2
#define DYN_PROMPT_LEFT_5_Y         49
#define DYN_PROMPT_X_SIZE_5         40
#define DYN_PROMPT_Y_SIZE_5         30

#define DYN_PROMPT_L_WALK_TO_X      59
#define DYN_PROMPT_L_WALK_TO_Y      140

#define DYN_PROMPT_RIGHT_1_X        121
#define DYN_PROMPT_RIGHT_1_Y        79

#define DYN_PROMPT_RIGHT_2_X        161
#define DYN_PROMPT_RIGHT_2_Y        67

#define DYN_PROMPT_RIGHT_3_X        177
#define DYN_PROMPT_RIGHT_3_Y        90

#define DYN_PROMPT_RIGHT_4_X        114
#define DYN_PROMPT_RIGHT_4_Y        100
#define DYN_PROMPT_X_SIZE_4         7
#define DYN_PROMPT_Y_SIZE_4         38

#define DYN_PROMPT_RIGHT_5_X        121

#define DYN_PROMPT_R_WALK_TO_X      171
#define DYN_PROMPT_R_WALK_TO_Y      142

#define DYN_PROMPT_R_WALK_TO_X_JAC  115
#define DYN_PROMPT_R_WALK_TO_Y_JAC  142

#define AFTER_STEPS_FROM_LEFT_X     117
#define AFTER_STEPS_FROM_LEFT_Y     139

#define AFTER_STEPS_FROM_RIGHT_X    62
#define AFTER_STEPS_FROM_RIGHT_Y    142

#define DYN_FLOOR_R_1_X             154
#define DYN_FLOOR_R_1_Y             136
#define DYN_FLOOR_R_1_X_SIZE        41
#define DYN_FLOOR_R_1_Y_SIZE        6
#define DYN_FLOOR_R_1_WALK_TO_X     171
#define DYN_FLOOR_R_1_WALK_TO_Y     142

#define DYN_FLOOR_R_2_X             114
#define DYN_FLOOR_R_2_Y             136
#define DYN_FLOOR_R_2_X_SIZE        32
#define DYN_FLOOR_R_2_Y_SIZE        6
#define DYN_FLOOR_R_2_WALK_TO_X     127
#define DYN_FLOOR_R_2_WALK_TO_Y     140

#define DYN_FLOOR_R_3_X             149
#define DYN_FLOOR_R_3_Y             140
#define DYN_FLOOR_R_3_X_SIZE        13
#define DYN_FLOOR_R_3_Y_SIZE        7
#define DYN_FLOOR_R_3_WALK_TO_X     155
#define DYN_FLOOR_R_3_WALK_TO_Y     144

#define DYN_FLOOR_R_4_X             187
#define DYN_FLOOR_R_4_Y             136
#define DYN_FLOOR_R_4_X_SIZE        8
#define DYN_FLOOR_R_4_Y_SIZE        7
#define DYN_FLOOR_R_4_WALK_TO_X     195
#define DYN_FLOOR_R_4_WALK_TO_Y     139

#define DYN_FLOOR_R_5_X             94
#define DYN_FLOOR_R_5_Y             129
#define DYN_FLOOR_R_5_X_SIZE        18
#define DYN_FLOOR_R_5_Y_SIZE        4
#define DYN_FLOOR_R_5_WALK_TO_X     95
#define DYN_FLOOR_R_5_WALK_TO_Y     133

#define DYN_FLOOR_R_6_X             94
#define DYN_FLOOR_R_6_Y             132
#define DYN_FLOOR_R_6_X_SIZE        3
#define DYN_FLOOR_R_6_Y_SIZE        9
#define DYN_FLOOR_R_6_WALK_TO_X     93
#define DYN_FLOOR_R_6_WALK_TO_Y     135

#define DYN_FLOOR_R_7_X             112
#define DYN_FLOOR_R_7_Y             150
#define DYN_FLOOR_R_7_X_SIZE        21
#define DYN_FLOOR_R_7_Y_SIZE        3
#define DYN_FLOOR_R_7_WALK_TO_X     118
#define DYN_FLOOR_R_7_WALK_TO_Y     154

#define DYN_FLOOR_R_8_X             98
#define DYN_FLOOR_R_8_Y             146
#define DYN_FLOOR_R_8_X_SIZE        21
#define DYN_FLOOR_R_8_Y_SIZE        4
#define DYN_FLOOR_R_8_WALK_TO_X     104
#define DYN_FLOOR_R_8_WALK_TO_Y     148

#define DYN_FLOOR_L_1_X             35
#define DYN_FLOOR_L_1_Y             137
#define DYN_FLOOR_L_1_X_SIZE        40
#define DYN_FLOOR_L_1_Y_SIZE        5
#define DYN_FLOOR_L_1_WALK_TO_X     59
#define DYN_FLOOR_L_1_WALK_TO_Y     140

#define DYN_FLOOR_L_2_X             76
#define DYN_FLOOR_L_2_Y             129
#define DYN_FLOOR_L_2_X_SIZE        6
#define DYN_FLOOR_L_2_Y_SIZE        6
#define DYN_FLOOR_L_2_WALK_TO_X     80
#define DYN_FLOOR_L_2_WALK_TO_Y     135

#define DYN_JAC_X                   156
#define DYN_JAC_Y                   116
#define DYN_JAC_X_SIZE              33
#define DYN_JAC_Y_SIZE              31
#define DYN_JAC_WALK_TO_X           206
#define DYN_JAC_WALK_TO_Y           148

#define DYN_JAC_BODY_X              114
#define DYN_JAC_BODY_Y              132
#define DYN_JAC_BODY_X_SIZE         30
#define DYN_JAC_BODY_Y_SIZE         10
#define DYN_JAC_BODY_WALK_TO_X      95
#define DYN_JAC_BODY_WALK_TO_Y      144

#define PROMPT_UP_LEFT_X            79
#define PROMPT_UP_LEFT_Y            132

#define PROMPT_UP_RIGHT_X           196
#define PROMPT_UP_RIGHT_Y           134

#define DEATH_X                     150
#define DEATH_Y                     147

#define NO                          0
#define YES_LEFT                    1
#define YES_RIGHT                   2

#define LEAVE_LEFT_X                0
#define LEAVE_LEFT_Y                150

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
