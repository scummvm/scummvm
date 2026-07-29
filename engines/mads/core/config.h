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

#ifndef MADS_CORE_CONFIG_H
#define MADS_CORE_CONFIG_H

#include "common/scummsys.h"

namespace MADS {

enum {
	INTERFACE_MACINTOSH			= 0,      /* Macintosh interface   */
	INTERFACE_BRAINDEAD			= 1       /* The "other" interface */
};

enum {
	INVENTORY_SPINNING			= 0,      /* Inventory objects spin  */
	INVENTORY_SQUAT				= 1       /* Inventory objects squat */
};

enum {
	INTERFACE_ANIMATED			= 0,      /* Interface animations on */
	INTERFACE_STILL				= 1       /* Interface still         */
};

enum {
	SCREEN_FADE_SMOOTH			= 0,       /* Smooth fade (thru black)     */
	SCREEN_FADE_MEDIUM			= 1,       /* Medium fade (Detmar-o-matic) */
	SCREEN_FADE_FAST			= 2        /* Fast fade (thru black)       */
};

enum {
	NAUGHTY                     = 1,       /* Naughtiness levels */
	NICE                        = 2
};

enum Difficulty {
	DIFFICULTY_HARD             = 1,
	DIFFICULTY_MEDIUM           = 2,
	DIFFICULTY_EASY             = 3
};

#define MEMORY_ALL              0       /* Use all available memory */
#define MEMORY_NO_EMS           1       /* Do not use EMS memory    */
#define MEMORY_NO_XMS           2       /* Do not use XMS memory    */
#define MEMORY_CONVENTIONAL     3       /* Use conventional only    */

#define PANNING_SMOOTH          0       /* Panning speeds           */
#define PANNING_MEDIUM          1
#define PANNING_INSTANT         2

#define MOUSE_MICROSOFT         0       /* Mouse cursor fixes       */
#define MOUSE_NOT_MICROSOFT     1

struct ConfigFile {
	int sound_card_type = 0;          /* Sound card configuration  */
	int sound_card_address = 0x220;

	int speech_card_type = 0;         /* Speech card configuration */
	int speech_card_address = 0x220;
	int speech_card_irq = 7;
	int speech_card_drq = 1;

	int music_flag = 1;               /* Music on/off              */
	int sound_flag = 1;               /* Sound on/off              */
	int interface_hotspots = 1;       /* Easy / Standard           */

	int inventory_mode = INVENTORY_SPINNING;           /* Spinning / Still          */
	int animated_interface = INTERFACE_ANIMATED;       /* On / Off                  */
	int naughtiness = NAUGHTY;              /* NAUGHTY / NICE            */
	int quotes_enabled = 1;           /* Quotes option enabled     */

	int high_memory_mode = MEMORY_ALL;         /* High memory mode          */

	int screen_fade = SCREEN_FADE_SMOOTH;              /* Screen fade               */

	int speech_flag = 1;              /* Speech on/off             */

	int panning_speed = PANNING_SMOOTH;            /* Panning speed flag        */

	int mouse_cursor_fix = MOUSE_MICROSOFT;         /* Mouse halfway problem     */

	int cd_version_installed = 0;     /* CD version installed      */
	int cd_drive = 'D';                 /* CD drive letter           */

	int speech_version_installed = 0; /* Version with speech installed */

	int show_speech_boxes = 1;        /* Show text during speech       */

	int sound_card_irq = 7;
	int misc2 = 0;
	int misc3 = 0;
	int misc4 = 0;
	int misc5 = 0;

	bool original_save_load = false;  /* Use original save/load menus */
};

extern ConfigFile config_file;

#define music_off               (!config_file.music_flag)
#define sound_off               (!config_file.sound_flag)

extern void read_config_file();
extern void write_config_file();
//extern void global_load_config_parameters();
//extern void global_unload_config_parameters();

} // namespace MADS

#endif
