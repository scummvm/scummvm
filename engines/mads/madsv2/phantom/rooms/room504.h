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

#ifndef MADS_PHANTOM_ROOM504_H
#define MADS_PHANTOM_ROOM504_H

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

	int16 sprite[17];       /* Sprite series handles */
	int16 sequence[17];     /* Sequence handles      */
	int16 animation[4];     /* Animation handles     */

	int16 music_choice;
	int16 input_count;   /* counter for raoul taking sword */

	int16 anim_0_running;
	int16 anim_1_running;
	int16 anim_2_running;
	int16 anim_3_running;
	int16 anim_4_running;
	int16 anim_5_running;
	int16 music_is_playing;
	int16 play_count;
	int16 fire_breath;

	int16 listen_action;    /* action Raoul is going to perform   */
	int16 listen_frame;

	int16 chair_action;     /* action Raoul is going to perform while in chair  */
	int16 chair_frame;

	int16 play_action;      /* action Raoul is going to perform at organ */
	int16 play_frame;
	int16 prevent;

	int16 phan_action;      /* action fight scene is going to perform at organ */
	int16 phan_frame;

	int16 chris_talk_count;

	int16 death_count;

} Scratch;


/* ========================= Sprite Series =================== */

#define fx_trap_door            0       /* rm504x2  */
#define fx_left_door            1       /* rm504x1  */
#define fx_right_door           2       /* rm504x0  */
#define fx_ramhead              3       /* rm504x3  */
#define fx_fire_left            4       /* rm504x4  */
#define fx_fire_right           5       /* rm504x5  */
#define fx_test                 6       /* rm504x5  */
#define fx_a_6                  7
#define fx_a_7                  8
#define fx_a_0                  9
#define fx_a_3                  10
#define fx_a_5                  11
#define fx_a_8                  12
#define fx_burn                 13      /* rm504x7  */
#define fx_take_9               14      /* *rdr_9   */
#define fx_music                15      /* rm504x8  */
#define fx_phan                 15      /* facephn  */


/* 'test' is used many times to load a series so that I can
   remap it.  'test' series are used in anims. */
   /* fx'letters' are loaded in and dumped from memory later */


   /* ========================= Triggers ======================== */

#define ROOM_504_FROM_502        60 
#define ROOM_504_SIT             64 
#define ROOM_504_END_LISTEN      67 
#define ROOM_504_END_CONV        70 
#define ROOM_504_OPEN_RIGHT_DOOR 80
#define ROOM_504_NEXT_FIRE       90
#define ROOM_504_LISTEN          95
#define ROOM_504_DONE_PLAYING    100
#define ROOM_504_LOAD_NEW_PART   110
#define ROOM_504_COMPLETE_CONV   120 /* (after sword battle) */
#define ROOM_504_RUN_PART_3      130
#define ROOM_504_EXIT_INTO_506   136
#define ROOM_504_CHRIS_TALK      145


/* ========================= Other Macros ==================== */

#define CONV_LISTEN             19
#define CONV_MUSIC              27
#define CONV_FIGHT              21

#define CONV19_LISTEN           0
#define CONV19_OTHER            1

#define PLAYER_X_FROM_502       147
#define PLAYER_Y_FROM_502       131

#define PLAYER_X_FROM_506       0
#define PLAYER_Y_FROM_506       109

#define WALK_TO_X_FROM_506      39
#define WALK_TO_Y_FROM_506      118

#define PLAYER_X_FROM_505       317
#define PLAYER_Y_FROM_505       115

#define WALK_TO_X_FROM_505      279
#define WALK_TO_Y_FROM_505      121

#define RIGHT_DOOR_X            286
#define RIGHT_DOOR_Y            120

#define BEHIND_RIGHT_DOOR_X     317              
#define BEHIND_RIGHT_DOOR_Y     115             

#define LEFT_DOOR_X             33
#define LEFT_DOOR_Y             116

#define CONV27_SELECT           0
#define CONV27_PLAY             1

#define AFTER_PLAY_X            156
#define AFTER_PLAY_Y            114

#define FIGHT_ENTER_ROOM        0
#define FIGHT_BEFORE_DODGE      1
#define FIGHT_CHOICE            2
#define FIGHT_DIE               3
#define FIGHT_SWORD             4
#define FIGHT_CHRIS_TO_DOOR     5
#define FIGHT_CHRIS_LEAVE       6
#define FIGHT_CHRIS_TALK        7

#define AFTER_FIGHT_X           114
#define AFTER_FIGHT_Y           137

#define WALK_TO_AFTER_FIGHT_X   130
#define WALK_TO_AFTER_FIGHT_Y   135

#define DYNAMIC_CHR_WALK_TO_X   66
#define DYNAMIC_CHR_WALK_TO_Y   119

#define EXIT_LEFT_DOOR_X        0
#define EXIT_LEFT_DOOR_Y        109

#define CONV_MISC               26

#define CHAIR_SIT               0
#define CHAIR_GET_UP            1

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
