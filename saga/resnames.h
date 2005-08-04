/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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

// Prefix RID_ means Resource Id

// Lookup tables
#define RID_ITE_SCENE_LUT  1806
#define RID_ITE_SCRIPT_LUT 216

#define RID_IHNM_SCENE_LUT  1272
#define RID_IHNM_SCRIPT_LUT 29

#define RID_IHNMDEMO_SCENE_LUT  286
#define RID_IHNMDEMO_SCRIPT_LUT 18

//obj names
#define ITE_OBJ_MAP 14
#define ITE_MAGIC_HAT 0

//actor names
#define ITE_ACTOR_PUZZLE 176

// SCENES
#define ITE_SCENE_INV -1
#define ITE_SCENE_PUZZLE 26
#define ITE_SCENE_LODGE 21
#define ITE_SCENE_ENDCREDIT1 295

#define ITE_DEFAULT_SCENE 32
#define IHNM_DEFAULT_SCENE 151

// FONTS
#define RID_MEDIUM_FONT 0
#define RID_BIG_FONT 1
#define RID_SMALL_FONT 2

// INTERFACE IMAGES
#define RID_ITE_MAIN_PANEL 3
#define RID_ITE_CONVERSE_PANEL 4
#define RID_ITE_OPTION_PANEL 5
#define RID_ITE_MAIN_SPRITES 6
#define RID_ITE_MAIN_PANEL_SPRITES 7
#define RID_ITE_MAIN_STRINGS 35 //main strings
#define RID_ITE_ACTOR_NAMES 36 //actors names
#define RID_ITE_DEFAULT_PORTRAITS 125

#define RID_ITE_TYCHO_MAP 1686
#define RID_ITE_SPR_XHAIR1 (73 + 9)
#define RID_ITE_SPR_XHAIR2 (74 + 9)

#define RID_IHNM_MAIN_PANEL  9
#define RID_IHNM_CONVERSE_PANEL 10
#define RID_IHNM_OPTION_PANEL 11       // TODO: verify this
#define RID_IHNM_MAIN_SPRITES 12       // TODO: verify this
#define RID_IHNM_MAIN_PANEL_SPRITES 13 // TODO: verify this
#define RID_IHNM_MAIN_STRINGS 21

// Puzzle portraits
#define RID_ITE_SAKKA_APPRAISING	6
#define RID_ITE_SAKKA_DENIAL		7
#define RID_ITE_SAKKA_EXCITED		8
#define RID_ITE_JFERRET_SERIOUS		9
#define RID_ITE_JFERRET_GOOFY		10
#define RID_ITE_JFERRET_ALOOF		11

// ITE Scene resource numbers
#define RID_ITE_OVERMAP_SCENE 226
#define RID_ITE_INTRO_ANIM_SCENE 1538
#define RID_ITE_CAVE_SCENE_1 1542
#define RID_ITE_CAVE_SCENE_2 1545
#define RID_ITE_CAVE_SCENE_3 1548
#define RID_ITE_CAVE_SCENE_4 1551

#define RID_ITE_VALLEY_SCENE 1556
#define RID_ITE_TREEHOUSE_SCENE 1560
#define RID_ITE_FAIREPATH_SCENE 1564
#define RID_ITE_FAIRETENT_SCENE 1567

#define RID_ITE_INTRO_ANIM_STARTFRAME 1529

#define RID_ITE_INTRO_ANIM_1 1530
#define RID_ITE_INTRO_ANIM_2 1531
#define RID_ITE_INTRO_ANIM_3 1532
#define RID_ITE_INTRO_ANIM_4 1533
#define RID_ITE_INTRO_ANIM_5 1534
#define RID_ITE_INTRO_ANIM_6 1535
#define RID_ITE_INTRO_ANIM_7 1536

#define RID_ITE_CAVE_IMG_1 1540
#define RID_ITE_CAVE_IMG_2 1543
#define RID_ITE_CAVE_IMG_3 1546
#define RID_ITE_CAVE_IMG_4 1549

#define RID_ITE_INTRO_IMG_1 1552
#define RID_ITE_INTRO_IMG_2 1557
#define RID_ITE_INTRO_IMG_3 1561
#define RID_ITE_INTRO_IMG_4 1565

// ITE_VOICES
#define RID_CAVE_VOICE_0 0
#define RID_CAVE_VOICE_1 1
#define RID_CAVE_VOICE_2 2
#define RID_CAVE_VOICE_3 3
#define RID_CAVE_VOICE_4 4
#define RID_CAVE_VOICE_5 5
#define RID_CAVE_VOICE_6 6
#define RID_CAVE_VOICE_7 7
#define RID_CAVE_VOICE_8 8
#define RID_CAVE_VOICE_9 9
#define RID_CAVE_VOICE_10 10
#define RID_CAVE_VOICE_11 11
#define RID_CAVE_VOICE_12 12
#define RID_CAVE_VOICE_13 13

#define RID_SCENE1_VOICE_009 57
//TODO: fill it
#define RID_SCENE1_VOICE_138 186

#define RID_BOAR_VOICE_000 239
#define RID_BOAR_VOICE_002 241
#define RID_BOAR_VOICE_005 244
#define RID_BOAR_VOICE_006 245
#define RID_BOAR_VOICE_007 246

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
#define FX_DUMMY 56

// These are only in the CD version

#define FX_CROWD_01 57
#define FX_CROWD_02 58
#define FX_CROWD_03 59
#define FX_CROWD_04 60
#define FX_CROWD_05 61
#define FX_CROWD_06 62
#define FX_CROWD_07 63
#define FX_CROWD_08 64
#define FX_CROWD_09 65
#define FX_CROWD_10 66
#define FX_CROWD_11 67
#define FX_CROWD_12 68
#define FX_CROWD_13 69
#define FX_CROWD_14 70
#define FX_CROWD_15 71
#define FX_CROWD_16 72
#define FX_CROWD_17 73

} // End of namespace Saga

#endif
