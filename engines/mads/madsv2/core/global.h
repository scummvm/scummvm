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

#ifndef MADS_CORE_GLOBAL_H
#define MADS_CORE_GLOBAL_H

#include "mads/madsv2/core/general.h"

namespace MADS {
namespace MADSV2 {

extern char global_release_name[];
extern char global_release_version[];
extern char global_release_date[];
extern char global_release_copyright[];

#define GLOBAL_LIST_SIZE        210     /* Global variables array size */

#define MESSAGE_COLOR           (((KERNEL_MESSAGE_COLOR_BASE   + 1) << 8) + KERNEL_MESSAGE_COLOR_BASE)
#define MESSAGE_COLOR_2         (((KERNEL_MESSAGE_COLOR_BASE_2 + 1) << 8) + KERNEL_MESSAGE_COLOR_BASE_2)
#define MESSAGE_COLOR_3         (((KERNEL_MESSAGE_COLOR_BASE_3 + 1) << 8) + KERNEL_MESSAGE_COLOR_BASE_3)

/* Global macros */

constexpr int YES = 1;
constexpr int NO  = 0;

#define SCORE_LOOK_SPHERE_106           1
#define SCORE_READ_BOOK_101             2
#define SCORE_SHIFT_INTO_SEAL_113       4   
#define SCORE_PUSH_BUTTON_119           8   
#define SCORE_WEAR_CROWN_119            16  
#define SCORE_MAKE_ACID_DRIP_605        32  
#define SCORE_CROSS_PILLARS             64  
#define SCORE_ENTER_609                 128  
#define SCORE_CROSS_SKY                 256 
#define SCORE_JUMP_INTO_WELL            512 
#define SCORE_MAKE_NOISE_BIRDCALL       1024
#define SCORE_TAKE_BONE                 2048

/* section 1 macros */

#define START_MACROS_HERE               1   /* enter description here */


/* ==================== System variables ===================== */

#define walker_timing                   0   /* Stop walker timing (long) */
#define walker_timing_2                 1
#define inventory_is_displayed          2   /* T if inventory is being displayed */
#define player_hyperwalked              3   /* T if player hit spacebar in room */
#define player_selected_object          4   /* will be >= 0 (to hold selected object) else -1 */
#define candle_is_on                    5   /* T if candle flame is on in interface */


/* ================= Global to all sections ================== */

#if 0
#define player_score                    10  /* player's score of the game   */
#define player_score_flags              11  /* 16 flags for checking if player increased score */
#define dont_load_walker                12  /* if T, will not load walker in section code */
#define perform_displacements           13  /* if T, will do displacements */
#define intro                           14  /* if T, Intro is in progress */
#define outro                           15  /* if T, Outro is in progress */
#endif

/* =================== Section 1  20 - 39 ==================== */

#define section_1_variable              20  /* put description here */

/* =================== Section 2  40 - 59 ==================== */


/* =================== Section 3  60 - 79 ==================== */


/* =================== Section 4  80 - 99 ==================== */


/* ================== Section 5  100 - 119 =================== */


/* ================== Section 6  120 - 139 =================== */


extern char global_release_name[];
extern char global_release_version[];
extern char global_release_date[];
extern char global_release_copyright[];

extern int16 global[];
extern int global_list_size;

void global_init_code(void);
void global_daemon_code(void);
void global_pre_parser_code(void);
void global_parser_code(void);
void global_error_code(void);
void global_room_init(void);
void global_verb_filter(void);
void global_sound_driver(void);
void global_section_constructor(void);
void global_write_config_file(void);
void global_load_config_parameters(void);
void global_unload_config_parameters(void);

} // namespace MADSV2
} // namespace MADS

#endif
