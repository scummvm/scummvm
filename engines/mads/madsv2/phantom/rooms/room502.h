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

#ifndef MADS_PHANTOM_ROOM502_H
#define MADS_PHANTOM_ROOM502_H

#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/anim.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

#define local ((Scratch *)(&game.scratch[0]))
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define num_cycle_stages        9       /* 9 stages of color cycling */

typedef struct {        /* Room local variables */

	byte puzzle_picture[16];  /* one of 4                   */
	byte puzzle_sprite[16];  /* which sprite in ss         */

	byte fire_1_on;     /* T if fire 1 is shooting from floor */
	byte fire_2_on;
	byte fire_3_on;
	byte fire_4_on;
	int8 panel_pushed;        /* will hold panel #0-15 where mouse clicked */
	byte turning_panel;       /* T if Raoul is moving (jumping, etc)       */
	byte yippie;              /* T if puzzle is solved                     */
	byte message_stage;       /* # is next message to display (1 to 3)     */
	byte room_getting_hotter; /* Set if room getting hot                   */
	byte interim_y;

	int16 interim_x;            /* will hold interim coordinates for panel_pushed */

	int16 sprite[17];           /* Sprite series handles */
	int16 sequence[17];         /* Sequence handles      */
	int16 animation[1];         /* Animation handles     */

	int16 puzzle_sequence[16];  /* sequence handles for stamps */

	CycleList cycle_list[num_cycle_stages];   /* Cycling lists           */
	RGBcolor *cycle_color[num_cycle_stages];  /* Cycling colors          */

	byte *cycle_pointer;                      /* Master cycle pointer    */
	int16 cycle_bookkeep;                     /* Cycling bookkeeping     */
	int16 cycle_stage;                        /* Set desired value (0-6) */

	uint32 hot_clock;    /* Last kernel clock       */
	uint32 hot_timer;    /* Timer count             */
	uint32 death_timer;  /* when this reaches LENGTH_OF_LIFE, die */

} Scratch;


/* ========================= Sprite Series =================== */

#define fx_fire_1               0       /* rm502x2  */
#define fx_fire_2               1       /* rm502x3  */
#define fx_fire_3               2       /* rm502x4  */
#define fx_fire_4               3       /* rm502x5  */
#define fx_death                4       /* rm502a2  */
#define fx_door                 5       /* rm502x0  */
#define fx_trap_door            6       /* rm502x1  */
#define fx_rope_thrower         7       /* rm502a1  */
#define fx_rope_climber         8       /* rm502a3  */
#define fx_rope                 9       /* rm502x6  */
#define fx_pusher               10      /* rm502a0  */

#define fx_row_1                11      /* rm502j0  */
#define fx_row_2                12      /* rm502k0  */
#define fx_row_3                13      /* rm502l0  */
#define fx_row_4                14      /* rm502m0  */
#define fx_panel                15      
#define fx_interim              16      /* rm502h0  */

/* ========================= Triggers ======================== */

#define ROOM_502_FIRE_BURST_1   60
#define ROOM_502_FIRE_BURST_2   63
#define ROOM_502_FIRE_BURST_3   66
#define ROOM_502_FIRE_BURST_4   69
#define ROOM_502_DEATH          71
#define ROOM_502_ENTER          77
#define ROOM_502_TRAP_DOOR_OPEN 80
#define ROOM_502_THROW_UP       82

#define ROOM_502_ROW_1          90
#define ROOM_502_ROW_2          95
#define ROOM_502_ROW_3          100
#define ROOM_502_ROW_4          105
#define ROOM_502_ANIMATE_PANELS 110


/* ========================= Other Macros ==================== */

#define PLAYER_X_FROM_501       43 
#define PLAYER_Y_FROM_501       154

#define WALK_TO_X_FROM_501      87 
#define WALK_TO_Y_FROM_501      153 

#define CYCLE_SWITCH_TICKS      300    /* How fast room gets hot  */
#define LENGTH_OF_LIFE          7200   /* (2 minutes ) */

#define DEATH_X                 160
#define DEATH_Y                 148

#define COLUMN_1_X              107
#define COLUMN_2_X              127
#define COLUMN_3_X              147
#define COLUMN_4_X              167
#define COLUMN_Y                148

#define TRAP_DOOR_X             211
#define TRAP_DOOR_Y             149

#define TRAP_DOOR_HS_X          225
#define TRAP_DOOR_HS_Y          28

#define THROW_UP_X              200
#define THROW_UP_Y              149

#define FIFTEEN_SECONDS         900
#define SIXTY_SECONDS           3600
#define NINETY_SECONDS          5400

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
