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

#ifndef HDB_MPC_H
#define HDB_MPC_H

namespace HDB {

#define		TILE_TELEPORT1					"anim_t32_teleporter1"
#define		TILE_TOUCHPLATE_OFF				"t32_touchplateoff"
#define		TILE_TOUCHPLATE_ON				"t32_touchplateon"
#define		TILE_SKY_STARS					"t32_sky_stars"
#define		TILE_HOLDER1					"anim_t32_holder_empty1"
#define		TILE_HOLDERFULL					"t32_holder_full"
#define		TILE_SWITCH1					"anim_t32_switch_off01"
#define		TILE_1SWITCH1					"anim_t32_1switch_off01"
#define		TILE_DOOR_N						"t32_door_n_1"
#define		TILE_DOOR_P						"t32_door_p_1"
#define		TILE_DOOR_S						"t32_door_s_1"
#define		TILE_ASKCOMP1					"anim_t32_askcomp1"
#define		TILE_BRIDGE_U					"t32_bridgeup1"
#define		TILE_SWITCH_ON					"t32_switch_on"
#define		TILE_1SWITCH_ON					"t32_1switch_on"
#define		TILE_BRIDGE_R					"t32_bridgeright1"
#define		TILE_BRIDGE_D					"t32_bridgedown1"
#define		TILE_BRIDGE_L					"t32_bridgeleft1"
#define		TILE_FLOORSHOCK					"t32_floorshock3"
#define		TILE_BEDR_CLOSED				"t32_bedr_r"
#define		TILE_BEDL_CLOSED				"t32_bedl_l"
#define		TILE_BEDR_OPEN					"t32_bedr_open"
#define		TILE_BEDL_OPEN					"t32_bedl_open"
#define		TILE_FERRET						"anim_t32_ferret1"
#define		TILE_DOOR_NV					"t32_door_nv_1"
#define		TILE_DOOR_PV					"t32_door_pv_1"
#define		TILE_DOOR_SV					"t32_door_sv_1"
#define		TILE_MAILSORT					"anim_t32_mailsort1"
#define		TILE_INBOX						"t32_deco_inbox"
#define		TILE_DOOR2_N					"t32_door2_n_1"
#define		TILE_DOOR2_P					"t32_door2_p_1"
#define		TILE_DOOR2_S					"t32_door2_s_1"
#define		TILE_DOOR2_NV					"t32_door2_nv_1"
#define		TILE_DOOR2_PV					"t32_door2_pv_1"
#define		TILE_DOOR2_SV					"t32_door2_sv_1"
#define		TILE_2DOOR_N					"t32_e2_door_n_1"
#define		TILE_2DOOR_P					"t32_e2_door_p_1"
#define		TILE_2DOOR_S					"t32_e2_door_s_1"
#define		TILE_2DOOR_NV					"t32_e2_door_nv_1"
#define		TILE_2DOOR_PV					"t32_e2_door_pv_1"
#define		TILE_2DOOR_SV					"t32_e2_door_sv_1"
#define		TILE_3DOOR_N					"t32_e3_door_n_1"
#define		TILE_3DOOR_P					"t32_e3_door_p_1"
#define		TILE_3DOOR_S					"t32_e3_door_s_1"
#define		TILE_3DOOR_NV					"t32_e3_door_nv_1"
#define		TILE_3DOOR_PV					"t32_e3_door_pv_1"
#define		TILE_3DOOR_SV					"t32_e3_door_sv_1"
#define		TILE_MEMOS						"t32_memos"
#define		TILE_PACKAGE					"t32_package"
#define		TILE_TYRANIUM					"t32_tyranium"
#define		TILE_FOUR_TOOLS					"t32_four_tools"
#define		TILE_GRATING_LR					"t32_mgrate_mid"
#define		TILE_GRATING_UD					"t32_mgrate_tbmid"
#define		TILE_BTUNNEL_BLOCK_B			"t32_btunnel_block_b"
#define		TILE_BTUNNEL_BLOCK_L			"t32_btunnel_block_l"
#define		TILE_BTUNNEL_BLOCK_R			"t32_btunnel_block_r"
#define		TILE_BTUNNEL_BLOCK_T			"t32_btunnel_block_t"
#define		TILE_BTUNNEL_DONE_B				"t32_btunnel_done_b"
#define		TILE_BTUNNEL_DONE_L				"t32_btunnel_done_l"
#define		TILE_BTUNNEL_DONE_R				"t32_btunnel_done_r"
#define		TILE_BTUNNEL_DONE_T				"t32_btunnel_done_t"
#define		TILE_A							"t32_alpha_a"
#define		TILE_B							"t32_alpha_b"
#define		TILE_C							"t32_alpha_c"
#define		TILE_D							"t32_alpha_d"
#define		TILE_E							"t32_alpha_e"
#define		TILE_F							"t32_alpha_f"
#define		TILE_G							"t32_alpha_g"
#define		TILE_H							"t32_alpha_h"
#define		TILE_I							"t32_alpha_i"
#define		TILE_J							"t32_alpha_j"
#define		TILE_K							"t32_alpha_k"
#define		TILE_L							"t32_alpha_l"
#define		TILE_M							"t32_alpha_m"
#define		TILE_N							"t32_alpha_n"
#define		TILE_O							"t32_alpha_o"
#define		TILE_P							"t32_alpha_p"
#define		TILE_Q							"t32_alpha_q"
#define		TILE_R							"t32_alpha_r"
#define		TILE_S							"t32_alpha_s"
#define		TILE_T							"t32_alpha_t"
#define		TILE_U							"t32_alpha_u"
#define		TILE_V							"t32_alpha_v"
#define		TILE_W							"t32_alpha_w"
#define		TILE_X							"t32_alpha_x"
#define		TILE_Y							"t32_alpha_y"
#define		TILE_Z							"t32_alpha_z"
#define		TILE_SKY_STARS_DOWN_SLOW		"t32_sky_stars_down_slow"
#define		TILE_SKY_STARS_UP_SLOW			"t32_sky_stars_up_slow"
#define		TILE_SKY_STARS_RIGHT_SLOW		"t32_sky_stars_right_slow"
#define		TILE_SKY_STARS_LEFT_SLOW		"t32_sky_stars_left_slow"
#define		TILE_SKY_STARS_STATIONARY		"t32_sky_stars_stationary"
#define		TILE_CHAIR						"t32_brownchair_u"
#define		TILE_PIZZA						"t32_deco_pizza"
#define		TILE_CABOPEN_T					"t32_opencabinet_t"
#define		TILE_CABOPEN_B					"t32_opencabinet_b"
#define		TILE_BLOCKPOLE					"t32_blockpole_1"
#define		TILE_SKY_CLOUDS					"t32_sky_clouds"
#define		TILE_TEMPLE_TOUCHPLATE_OFF		"t32_temple_touchplate_off"
#define		TILE_TEMPLE_TOUCHPLATE_ON		"t32_temple_touchplate_on"
#define		TILE_KC_HOLDER_WHITE_OFF		"anim_t32_kcholder_white_off1"
#define		TILE_KC_HOLDER_WHITE_ON			"t32_kcholder_white_on"
#define		TILE_KC_HOLDER_BLUE_OFF			"anim_t32_kcholder_blue_off1"
#define		TILE_KC_HOLDER_BLUE_ON			"t32_kcholder_blue_on"
#define		TILE_KC_HOLDER_RED_OFF			"anim_t32_kcholder_red_off1"
#define		TILE_KC_HOLDER_RED_ON			"t32_kcholder_red_on"
#define		TILE_KC_HOLDER_GREEN_OFF		"anim_t32_kcholder_green_off1"
#define		TILE_KC_HOLDER_GREEN_ON			"t32_kcholder_green_on"
#define		TILE_KC_HOLDER_PURPLE_OFF		"anim_t32_kcholder_purple_off1"
#define		TILE_KC_HOLDER_PURPLE_ON		"t32_kcholder_purple_on"
#define		TILE_KC_HOLDER_BLACK_OFF		"anim_t32_kcholder_black_off1"
#define		TILE_KC_HOLDER_BLACK_ON			"t32_kcholder_black_on"
#define		TILE_SHIP_HANDSWITCH_OFF		"anim_t32_ship_handswitch_off1"
#define		TILE_SHIP_HANDSWITCH_ON			"t32_ship_handswitch_on"
#define		TILE_BLOCKING_VINES				"t32_blocking_vines"
#define		TILE_WASTEBASKET				"t32_deco_wastebasket"
#define		TILE_SNOWDOOR_B					"t32_snow_temple_opendoor_b"
#define		TILE_PHAR_CARPET				"t32_phar_carpet_tb"
#define		TILE_HEADSTATUE					"t32_phar_headstatue1"
#define		TILE_ROUTER_HOLDER				"t32_router_holder"
#define		TILE_WEAPON_EQUIPPED			"t32_weapon_equipped"
#define		FORCEFIELD_LR					"anim_t32_forcefield_lr_"
#define		FORCEFIELD_UD					"anim_t32_forcefield_ud_"
#define		FORCESPLASH_TOP					"anim_t32_ff_splash_top"
#define		FORCESPLASH_BTM					"anim_t32_ff_splash_btm"
#define		FORCESPLASH_LEFT				"anim_t32_ff_splash_left"
#define		FORCESPLASH_RIGHT				"anim_t32_ff_splash_right_"
#define		DEMIGOD_FLOAT_T					"anim_t32_demigod_floats_t01"
#define		DEMIGOD_FLOAT_B					"anim_t32_demigod_floats_t01"
#define		TILE_FFTONGUE_UD_MIDDLE			"t32_tongue_ud_middle"
#define		TILE_FFTONGUE_UD_WIGGLE_L		"t32_tongue_ud_wiggle_left"
#define		TILE_FFTONGUE_UD_WIGGLE_M		"t32_tongue_ud_wiggle_mid"
#define		TILE_FFTONGUE_UD_WIGGLE_R		"t32_tongue_ud_wiggle_right"
#define		TILE_FFTONGUE_LR_MIDDLE			"t32_tongue_lr_middle"
#define		TILE_FFTONGUE_R_WIGGLE_U		"t32_tongue_right_wiggle_up"
#define		TILE_FFTONGUE_R_WIGGLE_M		"t32_tongue_right_wiggle_mid"
#define		TILE_FFTONGUE_R_WIGGLE_D		"t32_tongue_right_wiggle_down"
#define		TILE_FFTONGUE_L_WIGGLE_U		"t32_tongue_left_wiggle_up"
#define		TILE_FFTONGUE_L_WIGGLE_M		"t32_tongue_left_wiggle_mid"
#define		TILE_FFTONGUE_L_WIGGLE_D		"t32_tongue_left_wiggle_down"
#define		TILE_CONDUCTOR					"t32_conductor"
#define		GEM_WHITE						"t32_gem_white"
#define		GEM_RED_GFX						"t32_gem_red"
#define		GEM_BLUE_GFX					"t32_gem_blue"
#define		GEM_GREEN_GFX					"t32_gem_green"
#define		TILE_HEAD_SCIENTIST				"t32_head_scientist"
#define		TILE_HEAD_SPACEDUDE				"t32_head_spacedude"
#define		TILE_HEAD_WORKER				"t32_head_worker"
#define		TILE_HEAD_ACCOUNTANT			"t32_head_accountant"
#define		TILE_HEAD_DOLLY					"t32_head_dolly"
#define		TILE_HEAD_SARGE					"t32_head_sarge"
#define		TILE_SLOT_GOOD1					"t32_slot_good1"
#define		TILE_SLOT_GOOD2					"t32_slot_good2"
#define		TILE_SLOT_GOOD3					"t32_slot_good3"
#define		TILE_SLOT_GOOD4					"t32_slot_good4"
#define		TILE_SLOT_GOOD5					"t32_slot_good5"
#define		TILE_SLOT_GOOD6					"t32_slot_good6"
#define		TILE_SLOT_BAD					"t32_slot_bad"
#define		GROUP_ENT_GUY					"ent_guy_"
#define		GROUP_ENT_GUY_WALKDOWN			"ent_guy_walkdown"
#define		GROUP_ENT_GUY_WALKUP			"ent_guy_walkup"
#define		GROUP_ENT_GUY_WALKRIGHT			"ent_guy_walkright"
#define		GROUP_ENT_GUY_WALKLEFT			"ent_guy_walkleft"
#define		GROUP_ENT_GUY_DIE				"ent_guy_die"
#define		GROUP_ENT_GUY_BLINK				"ent_guy_blink"
#define		GROUP_ENT_GUY_STANDDOWN			"ent_guy_standdown"
#define		GROUP_ENT_GUY_STANDUP			"ent_guy_standup"
#define		GROUP_ENT_GUY_STANDRIGHT		"ent_guy_standright"
#define		GROUP_ENT_GUY_STANDLEFT			"ent_guy_standleft"
#define		GROUP_ENT_GUY_GOODJOB			"ent_guy_goodjob"
#define		TILE_GUYSLEEPING				"t32_guysleep"
#define		GROUP_ENT_GUY_PLUMMET			"ent_guy_plummet"
#define		GROUP_ENT_GUY_HORRIBLE1			"ent_guy_horrible1"
#define		GROUP_ENT_GUY_HORRIBLE2			"ent_guy_horrible2"
#define		GROUP_ENT_GUY_HORRIBLE3			"ent_guy_horrible3"
#define		GROUP_ENT_GUY_HORRIBLE4			"ent_guy_horrible4"
#define		GROUP_ENT_GUY_PUSHDOWN			"ent_guy_pushdown"
#define		GROUP_ENT_GUY_PUSHUP			"ent_guy_pushup"
#define		GROUP_ENT_GUY_PUSHRIGHT			"ent_guy_pushright"
#define		GROUP_ENT_GUY_PUSHLEFT			"ent_guy_pushleft"
#define		GROUP_ENT_GUY_STUNDOWN			"ent_guy_stundown"
#define		GROUP_ENT_GUY_STUNUP			"ent_guy_stunup"
#define		GROUP_ENT_GUY_STUNRIGHT			"ent_guy_stunright"
#define		GROUP_ENT_GUY_STUNLEFT			"ent_guy_stunleft"
#define		GROUP_ENT_GUY_GETDOWN			"ent_guy_getdown"
#define		GROUP_ENT_GUY_GETUP				"ent_guy_getup"
#define		GROUP_ENT_GUY_GETRIGHT			"ent_guy_getright"
#define		GROUP_ENT_GUY_GETLEFT			"ent_guy_getleft"
#define		GROUP_ENT_GUY_SLINGDOWN			"ent_guy_slingdown"
#define		GROUP_ENT_GUY_SLINGUP			"ent_guy_slingup"
#define		GROUP_ENT_GUY_SLINGRIGHT		"ent_guy_slingright"
#define		GROUP_ENT_GUY_SLINGLEFT			"ent_guy_slingleft"
#define		CLUBLEFT1						"clubleft1"
#define		CLUBLEFT2						"clubleft2"
#define		CLUBLEFT3						"clubleft3"
#define		CLUBRIGHT1						"clubright1"
#define		CLUBRIGHT2						"clubright2"
#define		CLUBRIGHT3						"clubright3"
#define		CLUBDOWN1						"clubdown1"
#define		CLUBDOWN2						"clubdown2"
#define		CLUBDOWN3						"clubdown3"
#define		CLUBUP1							"clubup1"
#define		CLUBUP2							"clubup2"
#define		CLUBUP3							"clubup3"
#define		TILE_ENV_WHITE					"ent_envelope_white_sit01"
#define		GROUP_ENT_ENVELOPE_WHITE        "ent_envelope_white_"
#define		GROUP_ENT_ENVELOPE_WHITE_SIT    "ent_envelope_white_sit"
#define		GROUP_ENT_ENVELOPE_BLUE         "ent_envelope_blue_"
#define		GROUP_ENT_ENVELOPE_BLUE_SIT     "ent_envelope_blue_sit"
#define		GROUP_ENT_ENVELOPE_RED          "ent_envelope_red_"
#define		GROUP_ENT_ENVELOPE_RED_SIT      "ent_envelope_red_sit"
#define		GROUP_ENT_ENVELOPE_GREEN        "ent_envelope_green_"
#define		GROUP_ENT_ENVELOPE_GREEN_SIT    "ent_envelope_green_sit"
#define		GROUP_ENT_SCIENTIST             "ent_scientist_"
#define		GROUP_ENT_SCIENTIST_WALKDOWN    "ent_scientist_walkdown"
#define		GROUP_ENT_SCIENTIST_WALKUP      "ent_scientist_walkup"
#define		GROUP_ENT_SCIENTIST_WALKRIGHT   "ent_scientist_walkright"
#define		GROUP_ENT_SCIENTIST_WALKLEFT    "ent_scientist_walkleft"
#define		GROUP_ENT_SCIENTIST_DIE         "ent_scientist_die"
#define		GROUP_ENT_SCIENTIST_STANDDOWN   "ent_scientist_standdown"
#define		GROUP_ENT_SCIENTIST_STANDUP     "ent_scientist_standup"
#define		GROUP_ENT_SCIENTIST_STANDRIGHT  "ent_scientist_standright"
#define		GROUP_ENT_SCIENTIST_STANDLEFT   "ent_scientist_standleft"
#define		GROUP_ENT_SCIENTIST_BLINK       "ent_scientist_blink"
#define		GROUP_ENT_SCIENTIST_WIN         "ent_scientist_win"
#define		GROUP_ENT_SERGEANT              "ent_sergeant_"
#define		GROUP_ENT_SERGEANT_WALKDOWN     "ent_sergeant_walkdown"
#define		GROUP_ENT_SERGEANT_WALKUP       "ent_sergeant_walkup"
#define		GROUP_ENT_SERGEANT_WALKRIGHT    "ent_sergeant_walkright"
#define		GROUP_ENT_SERGEANT_WALKLEFT     "ent_sergeant_walkleft"
#define		GROUP_ENT_SERGEANT_STANDDOWN    "ent_sergeant_standdown"
#define		GROUP_ENT_SERGEANT_STANDUP      "ent_sergeant_standup"
#define		GROUP_ENT_SERGEANT_STANDRIGHT   "ent_sergeant_standright"
#define		GROUP_ENT_SERGEANT_STANDLEFT    "ent_sergeant_standleft"
#define		GROUP_ENT_SERGEANT_BLINK        "ent_sergeant_blink"
#define		GROUP_ENT_SERGEANT_YELL         "ent_sergeant_yell"
#define		GROUP_ENT_WORKER                "ent_worker_"
#define		GROUP_ENT_WORKER_WALKDOWN       "ent_worker_walkdown"
#define		GROUP_ENT_WORKER_WALKUP         "ent_worker_walkup"
#define		GROUP_ENT_WORKER_WALKRIGHT      "ent_worker_walkright"
#define		GROUP_ENT_WORKER_WALKLEFT       "ent_worker_walkleft"
#define		GROUP_ENT_WORKER_DIE            "ent_worker_die"
#define		GROUP_ENT_WORKER_STANDDOWN      "ent_worker_standdown"
#define		GROUP_ENT_WORKER_STANDUP        "ent_worker_standup"
#define		GROUP_ENT_WORKER_STANDRIGHT     "ent_worker_standright"
#define		GROUP_ENT_WORKER_STANDLEFT      "ent_worker_standleft"
#define		GROUP_ENT_WORKER_BLINK          "ent_worker_blink"
#define		GROUP_ENT_DEAD_WORKER           "ent_dead_worker_"
#define		GROUP_ENT_DEAD_WORKER_SIT       "ent_dead_worker_sit"
#define		GROUP_ENT_SPACEDUDE             "ent_spacedude_"
#define		GROUP_ENT_SPACEDUDE_WALKDOWN    "ent_spacedude_walkdown"
#define		GROUP_ENT_SPACEDUDE_WALKUP      "ent_spacedude_walkup"
#define		GROUP_ENT_SPACEDUDE_WALKRIGHT   "ent_spacedude_walkright"
#define		GROUP_ENT_SPACEDUDE_WALKLEFT    "ent_spacedude_walkleft"
#define		GROUP_ENT_DOLLY                 "ent_dolly_"
#define		GROUP_ENT_DOLLY_WALKDOWN        "ent_dolly_walkdown"
#define		GROUP_ENT_DOLLY_WALKUP          "ent_dolly_walkup"
#define		GROUP_ENT_DOLLY_WALKRIGHT       "ent_dolly_walkright"
#define		GROUP_ENT_DOLLY_WALKLEFT        "ent_dolly_walkleft"
#define		GROUP_ENT_DOLLY_USERIGHT        "ent_dolly_useright"
#define		GROUP_ENT_DOLLY_KISSRIGHT       "ent_dolly_kissright"
#define		GROUP_ENT_DOLLY_KISSLEFT        "ent_dolly_kissleft"
#define		GROUP_ENT_DOLLY_ANGRY           "ent_dolly_angry"
#define		GROUP_ENT_DOLLY_LAUGH           "ent_dolly_laugh"
#define		GROUP_ENT_DOLLY_PANIC           "ent_dolly_panic"
#define		GROUP_ENT_CELL                  "ent_cell_"
#define		GROUP_ENT_CELL_SIT              "ent_cell_sit"
#define		GROUP_ENT_CRATE                 "ent_crate_"
#define		GROUP_ENT_CRATE_SIT             "ent_crate_sit"
#define		GROUP_ENT_CRATE_FLOAT           "ent_crate_float"
#define		GROUP_ENT_BARREL_LIGHT          "ent_barrel_light_"
#define		GROUP_ENT_BARREL_LIGHT_SIT      "ent_barrel_light_sit"
#define		GROUP_ENT_BARREL_LIGHT_FLOAT    "ent_barrel_light_float"
#define		GROUP_ENT_BARREL_LIGHT_MELT     "ent_barrel_light_melt"
#define		GROUP_ENT_BARREL_HEAVY          "ent_barrel_heavy_"
#define		GROUP_ENT_BARREL_HEAVY_SIT      "ent_barrel_heavy_sit"
#define		GROUP_ENT_BARREL_HEAVY_FLOAT    "ent_barrel_heavy_float"
#define		GROUP_ENT_BARREL_HEAVY_MELT     "ent_barrel_heavy_melt"
#define		GROUP_ENT_BARREL_EXPLODE        "ent_barrel_explode_"
#define		GROUP_ENT_BARREL_EXPLODE_SIT    "ent_barrel_explode_sit"
#define		GROUP_ENT_TRANSCEIVER           "ent_transceiver_"
#define		GROUP_ENT_TRANSCEIVER_SIT       "ent_transceiver_sit"
#define		GROUP_ENT_GOO_CUP               "ent_goo_cup_"
#define		GROUP_ENT_GOO_CUP_SIT           "ent_goo_cup_sit"
#define		GROUP_ENT_CLUB                  "ent_club_"
#define		GROUP_ENT_CLUB_SIT              "ent_club_sit"
#define		GROUP_ENT_ROBOSTUNNER           "ent_robostunner_"
#define		GROUP_ENT_ROBOSTUNNER_SIT       "ent_robostunner_sit"
#define		GROUP_ENT_SLUGSLINGER           "ent_slugslinger_"
#define		GROUP_ENT_SLUGSLINGER_SIT       "ent_slugslinger_sit"
#define		GEM_WHITE_GFX					"ent_gem_white_sit01"
#define		GROUP_ENT_GEM_WHITE             "ent_gem_white_"
#define		GROUP_ENT_GEM_WHITE_SIT         "ent_gem_white_sit"
#define		GROUP_ENT_GEM_BLUE              "ent_gem_blue_"
#define		GROUP_ENT_GEM_BLUE_SIT          "ent_gem_blue_sit"
#define		GROUP_ENT_GEM_RED               "ent_gem_red_"
#define		GROUP_ENT_GEM_RED_SIT           "ent_gem_red_sit"
#define		GROUP_ENT_GEM_GREEN             "ent_gem_green_"
#define		GROUP_ENT_GEM_GREEN_SIT         "ent_gem_green_sit"
#define		GROUP_ENT_TEACUP                "ent_teacup_"
#define		GROUP_ENT_TEACUP_SIT            "ent_teacup_sit"
#define		GROUP_ENT_COOKIE                "ent_cookie_"
#define		GROUP_ENT_COOKIE_SIT            "ent_cookie_sit"
#define		GROUP_ENT_BURGER                "ent_burger_"
#define		GROUP_ENT_BURGER_SIT            "ent_burger_sit"
#define		GROUP_ENT_PDA                   "ent_pda_"
#define		GROUP_ENT_PDA_SIT               "ent_pda_sit"
#define		GROUP_ENT_BOOK                  "ent_book_"
#define		GROUP_ENT_BOOK_SIT              "ent_book_sit"
#define		GROUP_ENT_CLIPBOARD             "ent_clipboard_"
#define		GROUP_ENT_CLIPBOARD_SIT         "ent_clipboard_sit"
#define		GROUP_ENT_NOTE                  "ent_note_"
#define		GROUP_ENT_NOTE_SIT              "ent_note_sit"
#define		GROUP_ENT_KEYCARD_WHITE         "ent_keycard_white_"
#define		GROUP_ENT_KEYCARD_WHITE_SIT     "ent_keycard_white_sit"
#define		GROUP_ENT_KEYCARD_BLUE          "ent_keycard_blue_"
#define		GROUP_ENT_KEYCARD_BLUE_SIT      "ent_keycard_blue_sit"
#define		GROUP_ENT_KEYCARD_RED           "ent_keycard_red_"
#define		GROUP_ENT_KEYCARD_RED_SIT       "ent_keycard_red_sit"
#define		GROUP_ENT_KEYCARD_GREEN         "ent_keycard_green_"
#define		GROUP_ENT_KEYCARD_GREEN_SIT     "ent_keycard_green_sit"
#define		GROUP_ENT_KEYCARD_PURPLE        "ent_keycard_purple_"
#define		GROUP_ENT_KEYCARD_PURPLE_SIT    "ent_keycard_purple_sit"
#define		GROUP_ENT_KEYCARD_BLACK         "ent_keycard_black_"
#define		GROUP_ENT_KEYCARD_BLACK_SIT     "ent_keycard_black_sit"
#define		GROUP_ENT_MAGIC_EGG             "ent_magic_egg_"
#define		GROUP_ENT_MAGIC_EGG_SIT         "ent_magic_egg_sit"
#define		GROUP_ENT_ICE_BLOCK             "ent_ice_block_"
#define		GROUP_ENT_ICE_BLOCK_SIT         "ent_ice_block_sit"
#define		GROUP_ENT_CABKEY                "ent_cabkey_"
#define		GROUP_ENT_CABKEY_SIT            "ent_cabkey_sit"
#define		GROUP_ENT_DOLLY_TOOL1           "ent_dolly_tool1_"
#define		GROUP_ENT_DOLLY_TOOL1_SIT       "ent_dolly_tool1_sit"
#define		GROUP_ENT_DOLLY_TOOL2           "ent_dolly_tool2_"
#define		GROUP_ENT_DOLLY_TOOL2_SIT       "ent_dolly_tool2_sit"
#define		GROUP_ENT_DOLLY_TOOL3           "ent_dolly_tool3_"
#define		GROUP_ENT_DOLLY_TOOL3_SIT       "ent_dolly_tool3_sit"
#define		GROUP_ENT_DOLLY_TOOL4           "ent_dolly_tool4_"
#define		GROUP_ENT_DOLLY_TOOL4_SIT       "ent_dolly_tool4_sit"
#define		GROUP_ENT_SEED                  "ent_seed_"
#define		GROUP_ENT_SEED_SIT              "ent_seed_sit"
#define		GROUP_ENT_SODA                  "ent_soda_"
#define		GROUP_ENT_SODA_SIT              "ent_soda_sit"
#define		GROUP_ENT_SLICER                "ent_slicer_"
#define		GROUP_ENT_SLICER_SIT            "ent_slicer_sit"
#define		GROUP_ENT_ROUTER                "ent_router_"
#define		GROUP_ENT_ROUTER_SIT            "ent_router_sit"
#define		GROUP_ENT_FROGSTATUE            "ent_frogstatue_"
#define		GROUP_ENT_FROGSTATUE_SIT        "ent_frogstatue_sit"
#define		GROUP_ENT_PACKAGE               "ent_package_"
#define		GROUP_ENT_PACKAGE_SIT           "ent_package_sit"
#define		GROUP_ENT_PUSHBOT               "ent_pushbot_"
#define		GROUP_ENT_PUSHBOT_WALKDOWN      "ent_pushbot_walkdown"
#define		GROUP_ENT_PUSHBOT_WALKUP        "ent_pushbot_walkup"
#define		GROUP_ENT_PUSHBOT_WALKRIGHT     "ent_pushbot_walkright"
#define		GROUP_ENT_PUSHBOT_WALKLEFT      "ent_pushbot_walkleft"
#define		GROUP_ENT_OMNIBOT               "ent_omnibot_"
#define		GROUP_ENT_OMNIBOT_WALKDOWN      "ent_omnibot_walkdown"
#define		GROUP_ENT_OMNIBOT_WALKUP        "ent_omnibot_walkup"
#define		GROUP_ENT_OMNIBOT_WALKRIGHT     "ent_omnibot_walkright"
#define		GROUP_ENT_OMNIBOT_WALKLEFT      "ent_omnibot_walkleft"
#define		GROUP_ENT_OMNIBOT_MISSILE       "ent_omnibot_missile"
#define		GROUP_ENT_RIGHTBOT              "ent_rightbot_"
#define		GROUP_ENT_RIGHTBOT_WALKDOWN     "ent_rightbot_walkdown"
#define		GROUP_ENT_RIGHTBOT_WALKUP       "ent_rightbot_walkup"
#define		GROUP_ENT_RIGHTBOT_WALKRIGHT    "ent_rightbot_walkright"
#define		GROUP_ENT_RIGHTBOT_WALKLEFT     "ent_rightbot_walkleft"
#define		GROUP_ENT_MAINTBOT              "ent_maintbot_"
#define		GROUP_ENT_MAINTBOT_WALKDOWN     "ent_maintbot_walkdown"
#define		GROUP_ENT_MAINTBOT_WALKUP       "ent_maintbot_walkup"
#define		GROUP_ENT_MAINTBOT_WALKRIGHT    "ent_maintbot_walkright"
#define		GROUP_ENT_MAINTBOT_WALKLEFT     "ent_maintbot_walkleft"
#define		GROUP_ENT_MAINTBOT_USEDOWN      "ent_maintbot_usedown"
#define		GROUP_ENT_MAINTBOT_USEUP        "ent_maintbot_useup"
#define		GROUP_ENT_MAINTBOT_USERIGHT     "ent_maintbot_useright"
#define		GROUP_ENT_MAINTBOT_USELEFT      "ent_maintbot_useleft"
#define		GROUP_ENT_MAINTBOT_STANDDOWN    "ent_maintbot_standdown"
#define		GROUP_ENT_MAINTBOT_STANDUP      "ent_maintbot_standup"
#define		GROUP_ENT_MAINTBOT_STANDRIGHT   "ent_maintbot_standright"
#define		GROUP_ENT_MAINTBOT_STANDLEFT    "ent_maintbot_standleft"
#define		GROUP_ENT_RAILRIDER             "ent_railrider_"
#define		GROUP_ENT_RAILRIDER_STANDDOWN   "ent_railrider_standdown"
#define		GROUP_ENT_RAILRIDER_STANDUP     "ent_railrider_standup"
#define		GROUP_ENT_RAILRIDER_WALKDOWN    "ent_railrider_walkdown"
#define		GROUP_ENT_RAILRIDER_WALKUP      "ent_railrider_walkup"
#define		GROUP_ENT_RAILRIDER_WALKRIGHT   "ent_railrider_walkright"
#define		GROUP_ENT_RAILRIDER_WALKLEFT    "ent_railrider_walkleft"
#define		GROUP_ENT_SHOCKBOT              "ent_shockbot_"
#define		GROUP_ENT_SHOCKBOT_WALK         "ent_shockbot_walk"
#define		GROUP_ENT_LASER                 "ent_laser_"
#define		GROUP_ENT_LASER_UP              "ent_laser_up"
#define		GROUP_ENT_LASER_DOWN            "ent_laser_down"
#define		GROUP_ENT_LASER_LEFT            "ent_laser_left"
#define		GROUP_ENT_LASER_RIGHT           "ent_laser_right"
#define		GROUP_ENT_FOUR_LASER            "ent_four_laser_"
#define		GROUP_ENT_FOUR_LASER_UP         "ent_four_laser_up"
#define		GROUP_ENT_FOUR_LASER_DOWN       "ent_four_laser_down"
#define		GROUP_ENT_FOUR_LASER_LEFT       "ent_four_laser_left"
#define		GROUP_ENT_FOUR_LASER_RIGHT      "ent_four_laser_right"
#define		GROUP_ENT_GREENBEAMLR           "ent_greenbeamlr_"
#define		GROUP_ENT_GREENBEAMLR_ANIM      "ent_greenbeamlr_anim"
#define		GROUP_ENT_GREENBEAMUD           "ent_greenbeamud_"
#define		GROUP_ENT_GREENBEAMUD_ANIM      "ent_greenbeamud_anim"
#define		GROUP_ENT_DEFLECTORBL           "ent_deflectorbl_"
#define		GROUP_ENT_DEFLECTORBL_SIT       "ent_deflectorbl_sit"
#define		GROUP_ENT_DEFLECTORBR           "ent_deflectorbr_"
#define		GROUP_ENT_DEFLECTORBR_SIT       "ent_deflectorbr_sit"
#define		GROUP_ENT_DEFLECTORTR           "ent_deflectortr_"
#define		GROUP_ENT_DEFLECTORTR_SIT       "ent_deflectortr_sit"
#define		GROUP_ENT_DEFLECTORTL           "ent_deflectortl_"
#define		GROUP_ENT_DEFLECTORTL_SIT       "ent_deflectortl_sit"
#define		GROUP_ENT_REDBEAMLR             "ent_redbeamlr_"
#define		GROUP_ENT_REDBEAMLR_ANIM        "ent_redbeamlr_anim"
#define		GROUP_ENT_REDBEAMUD             "ent_redbeamud_"
#define		GROUP_ENT_REDBEAMUD_ANIM        "ent_redbeamud_anim"
#define		GROUP_ENT_TURNBOT               "ent_turnbot_"
#define		GROUP_ENT_TURNBOT_WALKDOWN      "ent_turnbot_walkdown"
#define		GROUP_ENT_TURNBOT_WALKUP        "ent_turnbot_walkup"
#define		GROUP_ENT_TURNBOT_WALKRIGHT     "ent_turnbot_walkright"
#define		GROUP_ENT_TURNBOT_WALKLEFT      "ent_turnbot_walkleft"
#define		GROUP_ENT_VORTEXIAN             "ent_vortexian_"
#define		GROUP_ENT_VORTEXIAN_STANDDOWN   "ent_vortexian_standdown"
#define		GROUP_ENT_CHICKEN               "ent_chicken_"
#define		GROUP_ENT_CHICKEN_WALKDOWN      "ent_chicken_walkdown"
#define		GROUP_ENT_CHICKEN_WALKUP        "ent_chicken_walkup"
#define		GROUP_ENT_CHICKEN_WALKRIGHT     "ent_chicken_walkright"
#define		GROUP_ENT_CHICKEN_WALKLEFT      "ent_chicken_walkleft"
#define		GROUP_ENT_CHICKEN_STANDDOWN     "ent_chicken_standdown"
#define		GROUP_ENT_CHICKEN_STANDUP       "ent_chicken_standup"
#define		GROUP_ENT_CHICKEN_STANDRIGHT    "ent_chicken_standright"
#define		GROUP_ENT_CHICKEN_STANDLEFT     "ent_chicken_standleft"
#define		GROUP_ENT_CHICKEN_DIE           "ent_chicken_die"
#define		GROUP_ENT_DEADEYE               "ent_deadeye_"
#define		GROUP_ENT_DEADEYE_WALKDOWN      "ent_deadeye_walkdown"
#define		GROUP_ENT_DEADEYE_WALKUP        "ent_deadeye_walkup"
#define		GROUP_ENT_DEADEYE_WALKRIGHT     "ent_deadeye_walkright"
#define		GROUP_ENT_DEADEYE_WALKLEFT      "ent_deadeye_walkleft"
#define		GROUP_ENT_ICEPUFF               "ent_icepuff_"
#define		GROUP_ENT_ICEPUFF_PEEK          "ent_icepuff_peek"
#define		GROUP_ENT_ICEPUFF_APPEAR        "ent_icepuff_appear"
#define		GROUP_ENT_ICEPUFF_THROWDOWN     "ent_icepuff_throwdown"
#define		GROUP_ENT_ICEPUFF_THROWRIGHT    "ent_icepuff_throwright"
#define		GROUP_ENT_ICEPUFF_THROWLEFT     "ent_icepuff_throwleft"
#define		GROUP_ENT_ICEPUFF_DISAPPEAR     "ent_icepuff_disappear"
#define		GROUP_ENT_ACCOUNTANT            "ent_accountant_"
#define		GROUP_ENT_ACCOUNTANT_STANDDOWN  "ent_accountant_standdown"
#define		GROUP_ENT_ACCOUNTANT_STANDUP    "ent_accountant_standup"
#define		GROUP_ENT_ACCOUNTANT_STANDRIGHT "ent_accountant_standright"
#define		GROUP_ENT_ACCOUNTANT_STANDLEFT  "ent_accountant_standleft"
#define		GROUP_ENT_ACCOUNTANT_TALKDOWN   "ent_accountant_talkdown"
#define		GROUP_ENT_ACCOUNTANT_TALKUP     "ent_accountant_talkup"
#define		GROUP_ENT_ACCOUNTANT_TALKRIGHT  "ent_accountant_talkright"
#define		GROUP_ENT_ACCOUNTANT_TALKLEFT   "ent_accountant_talkleft"
#define		GROUP_ENT_MEERKAT               "ent_meerkat_"
#define		GROUP_ENT_MEERKAT_DIRTMOVE      "ent_meerkat_dirtmove"
#define		GROUP_ENT_MEERKAT_APPEAR        "ent_meerkat_appear"
#define		GROUP_ENT_MEERKAT_BITE          "ent_meerkat_bite"
#define		GROUP_ENT_MEERKAT_DISAPPEAR     "ent_meerkat_disappear"
#define		GROUP_ENT_MEERKAT_LOOK          "ent_meerkat_look"
#define		GROUP_ENT_GOODFAIRY             "ent_goodfairy_"
#define		GROUP_ENT_GOODFAIRY_WALKDOWN    "ent_goodfairy_walkdown"
#define		GROUP_ENT_GOODFAIRY_WALKUP      "ent_goodfairy_walkup"
#define		GROUP_ENT_GOODFAIRY_WALKRIGHT   "ent_goodfairy_walkright"
#define		GROUP_ENT_GOODFAIRY_WALKLEFT    "ent_goodfairy_walkleft"
#define		GROUP_ENT_BADFAIRY              "ent_badfairy_"
#define		GROUP_ENT_BADFAIRY_WALKDOWN     "ent_badfairy_walkdown"
#define		GROUP_ENT_BADFAIRY_WALKUP       "ent_badfairy_walkup"
#define		GROUP_ENT_BADFAIRY_WALKRIGHT    "ent_badfairy_walkright"
#define		GROUP_ENT_BADFAIRY_WALKLEFT     "ent_badfairy_walkleft"
#define		GROUP_ENT_GATEPUDDLE            "ent_gatepuddle_"
#define		GROUP_ENT_GATEPUDDLE_STANDDOWN  "ent_gatepuddle_standdown"
#define		GROUP_ENT_BUZZFLY               "ent_buzzfly_"
#define		GROUP_ENT_BUZZFLY_WALKDOWN      "ent_buzzfly_walkdown"
#define		GROUP_ENT_BUZZFLY_WALKUP        "ent_buzzfly_walkup"
#define		GROUP_ENT_BUZZFLY_WALKRIGHT     "ent_buzzfly_walkright"
#define		GROUP_ENT_BUZZFLY_WALKLEFT      "ent_buzzfly_walkleft"
#define		GROUP_ENT_FATFROG               "ent_fatfrog_"
#define		GROUP_ENT_FATFROG_STANDDOWN     "ent_fatfrog_standdown"
#define		GROUP_ENT_FATFROG_LICKDOWN      "ent_fatfrog_lickdown"
#define		GROUP_ENT_FATFROG_STANDRIGHT    "ent_fatfrog_standright"
#define		GROUP_ENT_FATFROG_STANDLEFT     "ent_fatfrog_standleft"
#define		GROUP_ENT_FATFROG_LICKRIGHT     "ent_fatfrog_lickright"
#define		GROUP_ENT_FATFROG_LICKLEFT      "ent_fatfrog_lickleft"
#define		GROUP_ENT_MONKEYSTONE           "ent_monkeystone_"
#define		GROUP_ENT_MONKEYSTONE_SIT       "ent_monkeystone_sit"
#define		GEM_FLASH						"get_gem_flash_sit"
#define		GROUP_GET_GEM_FLASH             "get_gem_flash_"
#define		GROUP_GET_GEM_FLASH_SIT         "get_gem_flash_sit"
#define		TELEPORT_FLASH					"teleporter_flash_sit"
#define		GROUP_TELEPORTER_FLASH          "teleporter_flash_"
#define		GROUP_TELEPORTER_FLASH_SIT      "teleporter_flash_sit"
#define		WATER_SPLASH					"water_splash_sit"
#define		GROUP_WATER_SPLASH              "water_splash_"
#define		GROUP_WATER_SPLASH_SIT          "water_splash_sit"
#define		SLIME_SPLASH					"slime_splash_sit"
#define		GROUP_SLIME_SPLASH              "slime_splash_"
#define		GROUP_SLIME_SPLASH_SIT          "slime_splash_sit"
#define		STEAM_PUFF						"steam_puff_sit"
#define		GROUP_STEAM_PUFF                "steam_puff_"
#define		GROUP_STEAM_PUFF_SIT            "steam_puff_sit"
#define		STUNNED_STARS					"starstun_sit"
#define		GROUP_STARSTUN                  "starstun_"
#define		GROUP_STARSTUN_SIT              "starstun_sit"
#define		SHOCK_SPARK						"shock_spark_sit"
#define		GROUP_SHOCK_SPARK               "shock_spark_"
#define		GROUP_SHOCK_SPARK_SIT           "shock_spark_sit"
#define		EXPLOSION_BOOM					"explosion_boom_sit"
#define		GROUP_EXPLOSION_BOOM            "explosion_boom_"
#define		GROUP_EXPLOSION_BOOM_SIT        "explosion_boom_sit"
#define		INVENTORY_NORMAL				"inventory_normal"
#define		INVENTORY_SELECT				"inventory_select"
#define		MINI_MSTONE						"mini_mstone"
#define		TITLESCREEN						"titlescreen"
#define		MENU_NEWGAME					"menu_newgame"
#define		MENU_LOADGAME					"menu_loadgame"
#define		MENU_OPTIONS					"menu_options"
#define		MENU_QUIT						"menu_quit"
#define		MENU_RESUME						"menu_resume"
#define		STAR_1							"star_1"
#define		STAR_2							"star_2"
#define		STAR_3							"star_3"
#define		MENU_ROCKETSHIP1				"menu_rocketship1"
#define		MENU_ROCKETSHIP2				"menu_rocketship2"
#define		MENU_EXHAUST1					"menu_exhaust1"
#define		MENU_EXHAUST2					"menu_exhaust2"
#define		TITLELOGO						"titlelogo"
#define		CTRL_ASSIGN						"ctrl_assign"
#define		CTRL_ARROWUP					"ctrl_arrowup"
#define		CTRL_ARROWDN					"ctrl_arrowdn"
#define		CTRL_ARROWLEFT					"ctrl_arrowleft"
#define		CTRL_ARROWRIGHT					"ctrl_arrowright"
#define		MENU_CONTROLS					"menu_controls"
#define		MENU_BACK						"menu_back"
#define		MENU_WARP						"menu_warp"
#define		SCREEN_DARKEN					"screen_darken"
#define		WARP_PLAQUE						"warp_plaque"
#define		PAUSE_PLAQUE					"pause_plaque"
#define		MONKEYLOGOSCREEN				"monkeylogoscreen"
#define		MONKEYLOGO_OOHOOH				"monkeylogo_oohooh"
#define		MONKEYLOGO_SMILE				"monkeylogo_smile"
#define		BACKSCROLL_PLANET1				"backscroll_planet1"
#define		BACKSCROLL_PLANET2				"backscroll_planet2"
#define		BACKSCROLL_PLANET3				"backscroll_planet3"
#define		BACKSCROLL_PLANET4				"backscroll_planet4"
#define		BACKSCROLL_PLANET5				"backscroll_planet5"
#define		BACKSCROLL_GALAXY1				"backscroll_galaxy1"
#define		BACKSCROLL_GALAXY2				"backscroll_galaxy2"
#define		MENU_BACK_TOPLEFT				"menu_back_topleft"
#define		MENU_BACK_TOP					"menu_back_top"
#define		MENU_BACK_TOPRIGHT				"menu_back_topright"
#define		MENU_BACK_LEFT					"menu_back_left"
#define		MENU_BACK_MIDDLE				"menu_back_middle"
#define		MENU_BACK_RIGHT					"menu_back_right"
#define		MENU_BACK_BTMLEFT				"menu_back_btmleft"
#define		MENU_BACK_BOTTOM				"menu_back_bottom"
#define		MENU_BACK_BTMRIGHT				"menu_back_btmright"
#define		MENU_TITLE_LEFT					"menu_title_left"
#define		MENU_TITLE_MID					"menu_title_mid"
#define		MENU_TITLE_RIGHT				"menu_title_right"
#define		G_MENU_BACK_TOPLEFT				"g_menu_back_topleft"
#define		G_MENU_BACK_TOP					"g_menu_back_top"
#define		G_MENU_BACK_TOPRIGHT			"g_menu_back_topright"
#define		G_MENU_BACK_LEFT				"g_menu_back_left"
#define		G_MENU_BACK_MIDDLE				"g_menu_back_middle"
#define		G_MENU_BACK_RIGHT				"g_menu_back_right"
#define		G_MENU_BACK_BTMLEFT				"g_menu_back_btmleft"
#define		G_MENU_BACK_BOTTOM				"g_menu_back_bottom"
#define		G_MENU_BACK_BTMRIGHT			"g_menu_back_btmright"
#define		G_MENU_TITLE_LEFT				"g_menu_title_left"
#define		G_MENU_TITLE_MID				"g_menu_title_mid"
#define		G_MENU_TITLE_RIGHT				"g_menu_title_right"
#define		G_CHECK_EMPTY					"g_check_empty"
#define		G_CHECK_ON						"g_check_on"
#define		G_CHECK_OFF						"g_check_off"
#define		G_CHECK_LEFT					"g_check_left"
#define		G_CHECK_RIGHT					"g_check_right"
#define		MENU_HAND_POINTRIGHT			"menu_hand_pointright"
#define		MENU_ARROW_DELIVERTO			"menu_arrow_deliverto"
#define		MENU_SLIDER_LEFT				"menu_slider_left"
#define		MENU_HAND_POINTER				"menu_hand_pointer"
#define		MENU_SLIDER_MID					"menu_slider_mid"
#define		MENU_SLIDER_RIGHT				"menu_slider_right"
#define		MENU_SLIDER_KNOB				"menu_slider_knob"
#define		MENU_DELIVERY_INDENTATION		"menu_delivery_indentation"
#define		MENU_TITLE_DELIVERIES			"menu_title_deliveries"
#define		MENU_TITLE_RESOURCES			"menu_title_resources"
#define		MENU_RESTART					"menu_restart"
#define		MODE_PUZZLE						"mode_puzzle"
#define		MODE_ACTION						"mode_action"
#define		GAMEFILE_SLOT					"gamefile_slot"
#define		MENU_REGISTRATION				"menu_registration"
#define		SECRETSTAR_RED1					"secretstar_red1"
#define		SECRETSTAR_RED2					"secretstar_red2"
#define		SECRETSTAR_GREEN1				"secretstar_green1"
#define		SECRETSTAR_GREEN2				"secretstar_green2"
#define		SECRETSTAR_BLUE1				"secretstar_blue1"
#define		SECRETSTAR_BLUE2				"secretstar_blue2"
#define		CINE_INTROSHIP					"cine_introship"
#define		CINE_LITTLE_MBOT				"cine_little_mbot"
#define		CINE_CLOCK						"cine_clock"
#define		GAME_TRY						"game_try"
#define		GAME_AGAIN						"game_again"
#define		GAME_TA_LEVELRESTART			"game_ta_levelrestart"
#define		PIC_LOADBAR		"pic_loadbar"
#define		PIC_LOADSTAR		"pic_loadstar"
#define		MENU_LOAD		"menu_load"
#define		MENU_SAVE		"menu_save"
#define		MENU_BACKSPACE		"menu_backspace"
#define		MENU_VERSION_NUMBER		"menu_version_number"
#define		CLOUDY_SKIES		"cloudy_skies"
#define		TITLE_EPISODE1		"title_episode1"
#define		TITLE_EPISODE2		"title_episode2"
#define		TITLE_EPISODE3		"title_episode3"
#define		PANIC_POINTER1		"panic_pointer1"
#define		PANIC_POINTER2		"panic_pointer2"
#define		PANIC_NUM0		"panic_num0"
#define		PANIC_NUM1		"panic_num1"
#define		PANIC_NUM2		"panic_num2"
#define		PANIC_NUM3		"panic_num3"
#define		PANIC_NUM4		"panic_num4"
#define		PANIC_NUM5		"panic_num5"
#define		PANIC_NUM6		"panic_num6"
#define		PANIC_NUM7		"panic_num7"
#define		PANIC_NUM8		"panic_num8"
#define		PANIC_NUM9		"panic_num9"
#define		PANIC_PANIC		"panic_panic"
#define		PANIC_ZONE		"panic_zone"
#define		PIC_RANK1		"pic_rank1"
#define		PIC_RANK2		"pic_rank2"
#define		PIC_RANK3		"pic_rank3"
#define		PIC_RANK4		"pic_rank4"
#define		PIC_RANK5		"pic_rank5"
#define		DRAGON_ASLEEP		"dragon_asleep"
#define		DRAGON_FLAP1		"dragon_flap1"
#define		DRAGON_FLAP2		"dragon_flap2"
#define		DRAGON_BREATHE_START		"dragon_breathe_start"
#define		DRAGON_BREATHING_1		"dragon_breathing_1"
#define		DRAGON_BREATHING_2		"dragon_breathing_2"
#define		ICEPUFF_SNOWBALL_DOWN		"icepuff_snowball_down"
#define		ICEPUFF_SNOWBALL_RIGHT		"icepuff_snowball_right"
#define		ICEPUFF_SNOWBALL_LEFT		"icepuff_snowball_left"
#define		SLUG_SHOT1		"slug_shot1"
#define		SLUG_SHOT2		"slug_shot2"
#define		SLUG_SHOT3		"slug_shot3"
#define		SLUG_SHOT4		"slug_shot4"
#define		PIC_ENV_WHITE		"pic_env_white"
#define		PIC_FROG_STATUE		"pic_frog_statue"
#define		PIC_JENKINS		"pic_jenkins"
#define		PIC_TEAM_BRIAN		"pic_team_brian"
#define		PIC_TEAM_STEVIE		"pic_team_stevie"
#define		PIC_TEAM_JOSH		"pic_team_josh"
#define		PIC_TEAM_TOM		"pic_team_tom"
#define		PIC_TEAM_JOHN		"pic_team_john"
#define		PIC_TEAM_LUCAS		"pic_team_lucas"
#define		PIC_QUITSCREEN		"pic_quitscreen"
#define		PIC_CONTROLSSCREEN		"pic_controlsscreen"
#define		PIC_MOUSE_CURSOR1		"pic_mouse_cursor1"
#define		PIC_MOUSE_CURSOR2		"pic_mouse_cursor2"
#define		PIC_MOUSE_CURSOR3		"pic_mouse_cursor3"
#define		PIC_MOUSE_CURSOR4		"pic_mouse_cursor4"
#define		PIC_MOUSE_CURSOR5		"pic_mouse_cursor5"
#define		PIC_MOUSE_CURSOR6		"pic_mouse_cursor6"
#define		PIC_MOUSE_CURSOR7		"pic_mouse_cursor7"
#define		PIC_MOUSE_CURSOR8		"pic_mouse_cursor8"
#define		PIC_STAR256		"pic_star256"
#define		PIC_STAR192		"pic_star192"
#define		PIC_STAR128		"pic_star128"
#define		PIC_STAR64		"pic_star64"
#define		PIC_SNOWFLAKE		"pic_snowflake"
#define		PIC_INFOBAR		"pic_infobar"
#define		PIC_LOADSCREEN		"pic_loadscreen"
#define		MENU_ACCEPT_WAV		"menu_accept.wav"

enum IconList {
	ICON_GUY_DOWN,
	ICON_GUY_UP,
	ICON_GUY_RIGHT,
	ICON_GUY_LEFT,
	ICON_ENVELOPE_WHITE,
	ICON_ENVELOPE_BLUE,
	ICON_ENVELOPE_RED,
	ICON_ENVELOPE_GREEN,
	ICON_LASER_RIGHT,
	ICON_LASER_DOWN,
	ICON_LASER_LEFT,
	ICON_LASER_UP,
	ICON_DIVERTER_LEFTDOWN,
	ICON_DIVERTER_RIGHTDOWN,
	ICON_DIVERTER_UPRIGHT,
	ICON_DIVERTER_LEFTUP,
	ICON_FOURFIRER_RIGHT,
	ICON_FOURFIRER_DOWN,
	ICON_FOURFIRER_LEFT,
	ICON_FOURFIRER_UP,
	ICON_ARROW_DOWN,
	ICON_ARROW_UP,
	ICON_ARROW_RIGHT,
	ICON_ARROW_LEFT,
	ICON_STOPARROW_DOWN,
	ICON_STOPARROW_UP,
	ICON_STOPARROW_RIGHT,
	ICON_STOPARROW_LEFT,
	ICON_ITEM_CELL,
	ICON_ENVITEM_CRATE,
	ICON_ENVITEM_LIGHTBARREL,
	ICON_ENVITEM_HEAVYBARREL,
	ICON_ENVITEM_BOOMBARREL,
	ICON_ITEM_TRANSCEIVER,
	ICON_E2_ITEM_CLUB,
	ICON_E3_ITEM_ROBOSTUNNER,
	ICON_E2_ITEM_SLUGSLINGER,
	ICON_SCIENTIST_DOWN,
	ICON_SCIENTIST_UP,
	ICON_SCIENTIST_RIGHT,
	ICON_SCIENTIST_LEFT,
	ICON_WORKER_DOWN,
	ICON_WORKER_UP,
	ICON_WORKER_RIGHT,
	ICON_WORKER_LEFT,
	ICON_SHOCKBOT_DOWN,
	ICON_SHOCKBOT_UP,
	ICON_SHOCKBOT_RIGHT,
	ICON_SHOCKBOT_LEFT,
	ICON_RIGHTBOT_DOWN,
	ICON_RIGHTBOT_UP,
	ICON_RIGHTBOT_RIGHT,
	ICON_RIGHTBOT_LEFT,
	ICON_PUSHBOT_DOWN,
	ICON_PUSHBOT_UP,
	ICON_PUSHBOT_RIGHT,
	ICON_PUSHBOT_LEFT,
	ICON_LISTENBOT_DOWN,
	ICON_LISTENBOT_UP,
	ICON_LISTENBOT_RIGHT,
	ICON_LISTENBOT_LEFT,
	ICON_MONKEYSTONE,
	ICON_TELEPORTER1,
	ICON_TELEPORTER2,
	ICON_TELEPORTER3,
	ICON_TELEPORTER4,
	ICON_TELEPORTER5,
	ICON_TELEPORTER6,
	ICON_TELEPORTER7,
	ICON_TELEPORTER8,
	ICON_TELEPORTER9,
	ICON_TELEPORTER10,
	ICON_TELEPORTER11,
	ICON_TELEPORTER12,
	ICON_TELEPORTER13,
	ICON_TELEPORTER14,
	ICON_TELEPORTER15,
	ICON_TELEPORTER16,
	ICON_TELEPORTER17,
	ICON_TELEPORTER18,
	ICON_TELEPORTER19,
	ICON_TELEPORTER20,
	ICON_LEVELEXIT,
	ICON_ACTION1,
	ICON_ACTION2,
	ICON_ACTION3,
	ICON_ACTION4,
	ICON_ACTION5,
	ICON_ACTION6,
	ICON_ACTION7,
	ICON_ACTION8,
	ICON_ACTION9,
	ICON_ACTION10,
	ICON_ACTION11,
	ICON_ACTION12,
	ICON_ACTION13,
	ICON_ACTION14,
	ICON_ACTION15,
	ICON_ACTION16,
	ICON_ACTION17,
	ICON_ACTION18,
	ICON_ACTION19,
	ICON_ACTION20,
	ICON_SPACEDUDE_DOWN,
	ICON_SPACEDUDE_UP,
	ICON_SPACEDUDE_RIGHT,
	ICON_SPACEDUDE_LEFT,
	ICON_SERGEANT_DOWN,
	ICON_SERGEANT_UP,
	ICON_SERGEANT_RIGHT,
	ICON_SERGEANT_LEFT,
	ICON_MAINTBOT_DOWN,
	ICON_MAINTBOT_UP,
	ICON_MAINTBOT_RIGHT,
	ICON_MAINTBOT_LEFT,
	ICON_ACTION_AUTO,
	ICON_GEM_WHITE,
	ICON_GEM_BLUE,
	ICON_GEM_RED,
	ICON_GEM_GREEN,
	ICON_SET_MUSIC,
	ICON_LUA,
	ICON_HERE,
	ICON_VORTEXIAN,
	ICON_CHICKEN_DOWN,
	ICON_CHICKEN_UP,
	ICON_CHICKEN_RIGHT,
	ICON_CHICKEN_LEFT,
	ICON_GOO_CUP,
	ICON_TEACUP,
	ICON_COOKIE,
	ICON_BURGER,
	ICON_PDA,
	ICON_BOOK,
	ICON_CLIPBOARD,
	ICON_NOTE,
	ICON_KEYCARD_WHITE,
	ICON_KEYCARD_BLUE,
	ICON_KEYCARD_RED,
	ICON_KEYCARD_GREEN,
	ICON_KEYCARD_PURPLE,
	ICON_KEYCARD_BLACK,
	ICON_MAGIC_EGG,
	ICON_ICE_BLOCK,
	ICON_CABKEY,
	ICON_DEAD_WORKER,
	ICON_OMNIBOT_DOWN,
	ICON_OMNIBOT_UP,
	ICON_OMNIBOT_RIGHT,
	ICON_OMNIBOT_LEFT,
	ICON_TURNBOT_DOWN,
	ICON_TURNBOT_UP,
	ICON_TURNBOT_RIGHT,
	ICON_TURNBOT_LEFT,
	ICON_DOLLY_DOWN,
	ICON_DOLLY_UP,
	ICON_DOLLY_RIGHT,
	ICON_DOLLY_LEFT,
	ICON_TRIGGER,
	ICON_DOLLY_TOOL1,
	ICON_DOLLY_TOOL2,
	ICON_DOLLY_TOOL3,
	ICON_DOLLY_TOOL4,
	ICON_RAILRIDER_ON_UP,
	ICON_RAILRIDER_ON_DOWN,
	ICON_RAILRIDER_ON_LEFT,
	ICON_RAILRIDER_ON_RIGHT,
	ICON_RAILRIDER_UP,
	ICON_RAILRIDER_DOWN,
	ICON_RAILRIDER_LEFT,
	ICON_RAILRIDER_RIGHT,
	ICON_SODA,
	ICON_ARROW_4WAY,
	ICON_DEADEYE_DOWN,
	ICON_DEADEYE_UP,
	ICON_DEADEYE_RIGHT,
	ICON_DEADEYE_LEFT,
	ICON_MEERKAT_MOUND,
	ICON_FATFROG_DOWN,
	ICON_FATFROG_RIGHT,
	ICON_FATFROG_LEFT,
	ICON_GOODFAIRY_DOWN,
	ICON_GOODFAIRY_UP,
	ICON_GOODFAIRY_RIGHT,
	ICON_GOODFAIRY_LEFT,
	ICON_BADFAIRY_DOWN,
	ICON_BADFAIRY_UP,
	ICON_BADFAIRY_RIGHT,
	ICON_BADFAIRY_LEFT,
	ICON_ACCOUNTANT_DOWN,
	ICON_ACCOUNTANT_UP,
	ICON_ACCOUNTANT_RIGHT,
	ICON_ACCOUNTANT_LEFT,
	ICON_ICEPUFF_DOWN,
	ICON_DRAGON_DOWN,
	ICON_BUZZFLY_DOWN,
	ICON_BUZZFLY_UP,
	ICON_BUZZFLY_RIGHT,
	ICON_BUZZFLY_LEFT,
	ICON_FROG_STATUE,
	ICON_PIZZA_SLICER,
	ICON_FAIRY_SRC,
	ICON_FAIRY_SRC2,
	ICON_FAIRY_SRC3,
	ICON_FAIRY_SRC4,
	ICON_FAIRY_SRC5,
	ICON_FAIRY_DEST,
	ICON_FAIRY_DEST2,
	ICON_FAIRY_DEST3,
	ICON_FAIRY_DEST4,
	ICON_FAIRY_DEST5,
	ICON_QUESTION_MARK,
	ICON_DEBUG_LOGO,
	ICON_TARGET_X,
	ICON_WAYPOINT_SELECT1,
	ICON_WAYPOINT_SELECT2,
	ICON_WAYPOINT_SELECT3,
	ICON_WAYPOINT_SELECT4,
	ICON_WAYPOINT_1,
	ICON_WAYPOINT_2
};

} // End of Namespace

#endif // !HDB_MPC_H
