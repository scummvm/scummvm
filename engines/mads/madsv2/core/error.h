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

#ifndef MADS_CORE_ERROR_H
#define MADS_CORE_ERROR_H

#include "mads/madsv2/core/general.h"

namespace MADS {
namespace MADSV2 {

#define NONE            0
#define WARNING         1
#define ERROR           2
#define SEVERE          3

#define ERROR_SERIES_LIST_FULL          -1
#define ERROR_MESSAGE_LIST_FULL         -2
#define ERROR_IMAGE_LIST_FULL           -3
#define ERROR_IMAGE_INTER_LIST_FULL     -4
#define ERROR_NO_MORE_PALETTE_FLAGS     -5
#define ERROR_NO_MORE_COLORS            -6
#define ERROR_SERIES_LOAD_FAILED        -7
#define ERROR_NO_MORE_MEMORY            -8
#define ERROR_WRONG_SERIES_UNLOAD_ORDER -9
#define ERROR_PLAYER_INVENTORY_FULL     -10
#define ERROR_SEQUENCE_LIST_FULL        -11
#define ERROR_VOCAB_ACTIVE_LIST_FULL    -12
#define ERROR_NO_SUCH_OBJECT            -13
#define ERROR_NO_SUCH_MESSAGE           -14
#define ERROR_POPUP_TOO_MANY_LINES      -15
#define ERROR_KERNEL_MESSAGE_LIST_FULL  -16
#define ERROR_MESSAGE_TOO_LONG          -17
#define ERROR_DEMO_PROTECTION           -18
#define ERROR_BEEN_IN_TOO_MANY_ROOMS    -19
#define ERROR_COPY_PROTECTION           -20
#define ERROR_QUOTE_LOAD_FAILED         -21
#define ERROR_TIME_LIMIT_EXPIRED        -22
#define ERROR_QUOTE_DUPLICATE_LOAD      -23
#define ERROR_DYNAMIC_HOTSPOT_OVERFLOW  -24
#define ERROR_SPRITE_DATA_LOAD_FAILED   -25
#define ERROR_CHAIN_FAILURE             -26
#define ERROR_RESTORE_GAME_FAILURE      -27
#define ERROR_EXPLODER_EXPLODED         -28
#define ERROR_EXPLODER_NULL             -29
#define ERROR_ANIMATION_LOAD_FAILURE    -30

#define ERROR_BREAK_POINT               -31

#define ERROR_KERNEL_NO_FONTS           -32
#define ERROR_KERNEL_NO_CURSOR          -33
#define ERROR_KERNEL_NO_OBJECTS         -34
#define ERROR_KERNEL_NO_ROOM            -35
#define ERROR_KERNEL_NO_HOTSPOTS        -36
#define ERROR_KERNEL_NO_VOCAB           -37
#define ERROR_KERNEL_NO_INTERFACE       -38
#define ERROR_KERNEL_NO_ANIMATION       -39
#define ERROR_KERNEL_NO_EMS             -40
#define ERROR_KERNEL_NO_POPUP           -41

#define ERROR_CONV_BAD_PCODE            -50
#define ERROR_CONV_BAD_OPERATOR         -51
#define ERROR_CONV_VARIABLE_RANGE       -52
#define ERROR_POPUP_OVERFLOW            -53
#define ERROR_CONV_FLUSH                -54
#define ERROR_CONV_GET                  -55
#define ERROR_CONV_RUN                  -56
#define ERROR_CONV_MENU                 -57
#define ERROR_CONV_NO_TEXT_LINE         -58
#define ERROR_NO_MORE_EMS               -59
#define ERROR_HEAP_REQUEST_FAILED       -60
#define ERROR_NO_MORE_HEAP              -61
#define ERROR_ORPHANED_TRIGGER          -62
#define ERROR_PEELING_DISABLED          -63
#define ERROR_POPUP_NO_ITEMS            -64
#define ERROR_WRITE_SAVE_DIRECTORY      -65
#define ERROR_MEMORY_CHAIN_CORRUPT      -66
#define ERROR_POPUP_PRESERVE_FAILURE    -67
#define ERROR_TOO_MANY_DAMN_HOTSPOTS    -68
#define ERROR_VARIANT_LOAD_FAILURE      -69


#define MODULE_MATTE                    1
#define MODULE_INTER                    2
#define MODULE_PAL                      3
#define MODULE_ROOM                     4
#define MODULE_KERNEL                   5
#define MODULE_OBJECT                   6
#define MODULE_UNKNOWN                  7
#define MODULE_VOCAB                    8
#define MODULE_TEXT                     9
#define MODULE_POPUP                   10
#define MODULE_MEMORY                  11
#define MODULE_ROOM_LOADER             12
#define MODULE_SPRITE_LOADER           13
#define MODULE_ANIM_LOADER             14
#define MODULE_FONT_LOADER             15
#define MODULE_TEXT_LOADER             16
#define MODULE_VOCAB_LOADER            17
#define MODULE_DEMO                    18
#define MODULE_PLAYER                  19
#define MODULE_LOCK                    20
#define MODULE_QUOTE                   21
#define MODULE_GAME                    22
#define MODULE_GAME_MENU               23
#define MODULE_EXPLODER                24
#define MODULE_ANIM                    25
#define MODULE_CONV                    26
#define MODULE_POPUP_DIALOG            27
#define MODULE_HOTSPOT                 28


extern int error_abort;
extern char error_string[80];

extern void (*error_service_routine)();
extern void (*error_service_routine_2)();


extern void error_report(int error, int severity, int module, long data1, long data2);
extern int error_scan(char *target, const char *name, int number);
extern void error_dump_file(const char *file_name);
extern void error_break_point(int data1, int data2);
extern void error_watch_point(char *message, long data1, long data2);
extern void error_check_memory(void);
extern void error_file_point(char *message, long data1, long data2);
extern void error_entry(char *name, char *message);
extern void error_exit(char *name, char *message);
extern void error_put(char *name, int item);

} // namespace MADSV2
} // namespace MADS

#endif
