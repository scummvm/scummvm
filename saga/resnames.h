/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Descriptive names for game resource numbers

#ifndef SAGA_RESOURCENAMES_H_
#define SAGA_RESOURCENAMES_H_

namespace Saga {

// Lookup tables
#define ITE_SCENE_LUT  1806
#define ITE_SCRIPT_LUT 216

	// Hmmm.... as far as I see origginal uses now commented values, but they're wrong
#define IHNM_SCENE_LUT  1272  // 286
#define IHNM_SCRIPT_LUT 0     // 18

// SCENES
#define ITE_DEFAULT_SCENE 32

// FONTS
#define RN_MEDIUM_FONT 0
#define RN_BIG_FONT 1
#define RN_SMALL_FONT 2

// INTERFACE IMAGES
#define ITE_COMMAND_PANEL 3
#define ITE_DIALOGUE_PANEL 4

#define IHNM_COMMAND_PANEL  9
#define IHNM_DIALOGUE_PANEL 10

#define ITE_SETUP_PANEL 5
#define ITE_MAIN_SPRITES 6
#define ITE_COMMAND_BUTTONSPRITES 7
#define ITE_DEFAULT_PORTRAITS 125

// ITE Scene resource numbers
#define ITE_INTRO_ANIM_SCENE 1538
#define ITE_CAVE_SCENE_1 1542
#define ITE_CAVE_SCENE_2 1545
#define ITE_CAVE_SCENE_3 1548
#define ITE_CAVE_SCENE_4 1551

#define ITE_VALLEY_SCENE 1556
#define ITE_TREEHOUSE_SCENE 1560
#define ITE_FAIREPATH_SCENE 1564
#define ITE_FAIRETENT_SCENE 1567

#define ITE_INTRO_ANIM_STARTFRAME 1529

#define INTRO_ANIM_1 1530
#define INTRO_ANIM_2 1531
#define INTRO_ANIM_3 1532
#define INTRO_ANIM_4 1533
#define INTRO_ANIM_5 1534
#define INTRO_ANIM_6 1535
#define INTRO_ANIM_7 1536

#define CAVE_IMG_1 1540
#define CAVE_IMG_2 1543
#define CAVE_IMG_3 1546
#define CAVE_IMG_4 1549

#define INTRO_IMG_1 1552
#define INTRO_IMG_2 1557
#define INTRO_IMG_3 1561
#define INTRO_IMG_4 1565

// ITE_VOICES
#define CAVE_VOICE_0 0
#define CAVE_VOICE_1 1
#define CAVE_VOICE_2 2
#define CAVE_VOICE_3 3
#define CAVE_VOICE_4 4
#define CAVE_VOICE_5 5
#define CAVE_VOICE_6 6
#define CAVE_VOICE_7 7
#define CAVE_VOICE_8 8
#define CAVE_VOICE_9 9
#define CAVE_VOICE_10 10
#define CAVE_VOICE_11 11
#define CAVE_VOICE_12 12
#define CAVE_VOICE_13 13

#define SCENE1_VOICE_009 57
//TODO: fill it
#define SCENE1_VOICE_138 186


// MUSIC
#define MUSIC_1 9
#define MUSIC_2 10

// TODO: If the sound effects are numbered sequentially, we don't really need
//       these constants. But for now they might be useful for debugging.

// SOUND EFFECTS

#define FX_DOOR_OPEN 14
#define FX_DOOR_CLOSE 15
#define FX_RUSH_WATER 16
#define FX_CRICKET 17
#define FX_PORTICULLIS 18
#define FX_CLOCK_1 19
#define FX_CLOCK_2 20
#define FX_DAM_MACHINE 21
#define FX_HUM1 22
#define FX_HUM2 23
#define FX_HUM3 24
#define FX_HUM4 25
#define FX_STREAM 26
#define FX_SURF 27
#define FX_FIRELOOP 28
#define FX_SCRAPING 29
#define FX_BEE_SWARM 30
#define FX_SQUEAKBOARD 31
#define FX_KNOCK 32
#define FX_COINS 33
#define FX_STORM 34
#define FX_DOOR_CLOSE_2 35
#define FX_ARCWELD 36
#define FX_RETRACT_ORB 37
#define FX_DRAGON 38
#define FX_SNORES 39
#define FX_SPLASH 40
#define FX_LOBBY_DOOR 41
#define FX_CHIRP_LOOP 42
#define FX_DOOR_CREAK 43
#define FX_SPOON_DIG 44
#define FX_CROW 45
#define FX_COLDWIND 46
#define FX_TOOL_SND_1 47
#define FX_TOOL_SND_2 48
#define FX_TOOL_SND_3 49
#define FX_DOOR_METAL 50
#define FX_WATER_LOOP_S 51
#define FX_WATER_LOOP_L 52
#define FX_DOOR_OPEN_2 53
#define FX_JAIL_DOOR 54
#define FX_KILN_FIRE 55

// These are only in the CD version

#define FX_CROWD_01 56
#define FX_CROWD_02 57
#define FX_CROWD_03 58
#define FX_CROWD_04 59
#define FX_CROWD_05 60
#define FX_CROWD_06 61
#define FX_CROWD_07 62
#define FX_CROWD_08 63
#define FX_CROWD_09 64
#define FX_CROWD_10 65
#define FX_CROWD_11 66
#define FX_CROWD_12 67
#define FX_CROWD_13 68
#define FX_CROWD_14 69
#define FX_CROWD_15 70
#define FX_CROWD_16 71
#define FX_CROWD_17 72

} // End of namespace Saga

#endif
