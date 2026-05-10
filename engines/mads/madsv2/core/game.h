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

#ifndef MADS_CORE_GAME_H
#define MADS_CORE_GAME_H

#include "common/serializer.h"
#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/global.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/mads.h"
#include "mads/madsv2/core/heap.h"
#include "mads/madsv2/core/popup.h"

namespace MADS {
namespace MADSV2 {

#define game_version    "1.22"          /* Version of game control code */
#define game_date       "27-Jul-92"

#define HARD_MODE               0       /* Difficulty levels  */
#define EASY_MODE               1

#define TENTH_SECOND            6       /* Timing definitions */
#define QUARTER_SECOND          15
#define THIRD_SECOND            20
#define HALF_SECOND             30

#define ONE_SECOND              60
#define TWO_SECONDS             120
#define THREE_SECONDS           180
#define FOUR_SECONDS            240
#define FIVE_SECONDS            300
#define SIX_SECONDS             360
#define SEVEN_SECONDS           420
#define EIGHT_SECONDS           480
#define NINE_SECONDS            540
#define TEN_SECONDS             600
#define FIFTEEN_SECONDS         900
#define THIRTY_SECONDS          1800
#define ONE_MINUTE              3600
#define TWO_MINUTES             7200
#define THREE_MINUTES           10800
#define UNTIL_THE_COW_EXPLODES  9999999

#define CURSOR_NORMAL           1       /* Cursor types */
#define CURSOR_WAIT             2
#define CURSOR_DOWN             3
#define CURSOR_UP               4
#define CURSOR_LEFT             5
#define CURSOR_RIGHT            6

#define FACING_NORTH            8       /* Player facings */
#define FACING_SOUTH            2
#define FACING_EAST             6
#define FACING_WEST             4
#define FACING_NORTHEAST        9
#define FACING_SOUTHEAST        3
#define FACING_SOUTHWEST        1
#define FACING_NORTHWEST        7
#define FACING_NONE             5


#define DEBUGGER_NONE           -1
#define DEBUGGER_MAIN           0
#define DEBUGGER_HELP           1
#define DEBUGGER_PALETTE        2
#define DEBUGGER_SCRATCH        3
#define DEBUGGER_GLOBAL         4
#define DEBUGGER_CONVERSATION   5
#define DEBUGGER_MATTE          6
#define DEBUGGER_MEMORY         7
#define DEBUGGER_STATE          8
#define DEBUGGER_PAL_LIST_1     9
#define DEBUGGER_PAL_LIST_2     10

#define DEBUGGER_MAX_WATCH      12


extern int int_sprite[6];

extern int selected_intro;

#define fx_int_journal          0
#define fx_int_backpack         1
#define fx_int_candle           2
#define fx_int_exit             3
#define fx_int_dooropen         4
#define fx_int_candle_on        5


#define EXTRA_MAX_INV_OBJECTS 18   /* 16, + background, + open backpack */

/* int inven_ss[EXTRA_MAX_INV_OBJECTS]; */
/* int inven_seq[EXTRA_MAX_INV_OBJECTS]; */


/* Declared by Taranjeet for OUAF Journal */
extern Player journal_player;
extern int journal_room;

/**
 * Game menus
 */
enum {
	GAME_NO_MENU          = 0,
	GAME_MAIN_MENU        = 1,
	GAME_SAVE_MENU        = 2,
	GAME_RESTORE_MENU     = 3,
	GAME_OPTIONS_MENU     = 4,
	GAME_DIFFICULTY_MENU  = 5,
	GAME_ALERT_MENU       = 6,
	GAME_SCORE_MENU       = 7,
	GAME_CD_MENU          = 8
};

#define GAME_MENU_MAX_ITEMS   20      /* Max number of menu items    */
#define GAME_MAX_SAVE_SLOTS   99      /* Max number of save slots    */
#define GAME_MAX_SAVE_LENGTH  63      /* Max length of save name     */
#define GAME_MAX_SAVE_WIDTH   255     /* Max pixel width of save name*/
#define GAME_MAX_SAVES_ON_SCREEN 7    /* Max saves on screen at once */

#define GAME_SAVE_SLOT_MEMORY   (long)(GAME_MAX_SAVE_SLOTS * (GAME_MAX_SAVE_LENGTH + 1))
#define GAME_DIALOG_HEAP        (long)(4096 + sizeof(PopupItem) * GAME_MENU_MAX_ITEMS)
#define GAME_FUDGE_FACTOR       256
#define GAME_MENU_HEAP          (long)(GAME_SAVE_SLOT_MEMORY + GAME_DIALOG_HEAP + GAME_FUDGE_FACTOR)


#define GAME_MENU_SCROLL_FIRST  20    /* Scrolling delays            */
#define GAME_MENU_SCROLL_SECOND 8

#define GAME_MENU_CENTER      -1      /* Center item on menu         */
#define GAME_MENU_SPECIAL     -2      /* Center on special character */
#define GAME_MENU_RIGHT       -3      /* Right-justify text          */

#define WIN_NOTHING           0
#define WIN_QUICK_DEATH       1
#define WIN_SLOW_DEATH        2
#define WIN_ALL_THE_MONEY     3
#define WIN_A_HEAD_POW        4

extern byte game_restore_flag;         /* Flag if restoring game */
extern byte game_autosaved;            /* Flag if autosaved      */

extern byte game_mouse_cursor_fix;     /* Mouse cursor fix       */

extern word abort_value;
extern long abort_clock;

extern int   logfile_enabled;
extern Common::Stream *logfile_handle;

extern int  sound_board_address;
extern int  sound_board_type;
extern int  sound_board_irq;

extern char chain_line[80];
extern int  chain_flag;
extern int  force_chain;
extern int key_abort_level;

extern int game_keystroke;
extern int game_any_keystroke;

extern int win_status;

extern long correction_clock;


extern char config_file_name[20];
extern char save_game_key[8];
extern char restart_game_key[40];
extern char save_game_buf[20];

extern void (*game_menu_routine)();   /* Game Menu routines      */
extern void (*game_menu_init)();
extern void (*game_menu_exit)();
extern void (*game_emergency_save)();


extern int debugger;
extern int debugger_state;
extern int debugger_matte_before;
extern int debugger_memory_skip;
extern int debugger_memory_all;
extern int debugger_memory_keywait;
extern void (*debugger_reset)();      /* Debugger reset routine  */
extern void (*debugger_update)();     /* Debugger update routine */


extern void (*section_preload_code_pointer)();
extern void (*section_init_code_pointer)();
extern void (*section_room_constructor)();
extern void (*section_daemon_code_pointer)();
extern void (*section_pre_parser_code_pointer)();
extern void (*section_parser_code_pointer)();
extern void (*section_error_code_pointer)();
extern void (*section_music_reset_pointer)();


extern void (*room_preload_code_pointer)();
extern void (*room_init_code_pointer)();
extern void (*room_daemon_code_pointer)();
extern void (*room_pre_parser_code_pointer)();
extern void (*room_parser_code_pointer)();
extern void (*room_error_code_pointer)();
extern void (*room_shutdown_code_pointer)();

extern char game_save_file[13];     /* Save directory file      */
extern char *game_save_directory;   /* Save directory pointer   */

extern int  game_preserve_handle;       /* scr_depth preserve       */

extern Heap game_menu_heap;             /* Custom heap for menu     */
extern Popup *game_menu_popup;      /* Popup structure for menu */
extern int  report_version;
extern int art_hags_are_on_hd;

extern void game_save_name(int id);
extern void game_set_camera_speed(void);

/**
 * Execute a function-pointer routine.
 */
extern void game_exec_function(void (*(target))());

extern void game_debugger_reset(void);
extern void game_debugger(void);

/**
 * Sets up for the main menu routines to run--this mainly involves
 * getting enough memory to hold the save directory and menu heap.
 */

extern void game_menu_setup(void);

/**
 * Tells the menu structures to bite down hard.
 */
extern void game_menu_shutdown(void);

extern int  main_cheating_key(int mykey);
extern int  main_normal_key(int mykey);
extern int  main_copy_verify(void);
extern void game_cold_data_init();

/**
 * This is the main outer control structure for the game, that
 * determines which section and room level information should be
 * loaded into and out of memory.
 */
extern void game_control();

/**
 * Routine to parse command line flags.
 */
extern void flag_parse(const char **myscan);

extern void show_logo();
extern void show_version();

extern void init_game();

} // namespace MADSV2
} // namespace MADS

#endif
