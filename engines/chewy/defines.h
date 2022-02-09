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

#ifndef CHEWY_DEFINES_H
#define CHEWY_DEFINES_H

namespace Chewy {

#define VERSION1 0
#define VERSION2 2

#define ADS_TAP_MAX 500
#define ATS_TAP_MAX 100
#define AAD_TAP_MAX 100
#define INV_TAP_MAX 140
#define USE_TAP_MAX 140

#define ADS_TAP_OFF 0
#define ATS_TAP_OFF ADS_TAP_MAX
#define AAD_TAP_OFF ATS_TAP_OFF+ATS_TAP_MAX
#define INV_TAP_OFF AAD_TAP_OFF+AAD_TAP_MAX
#define USE_TAP_OFF INV_TAP_OFF+INV_TAP_MAX

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

enum SetupScreenMode {
	NO_SETUP = 0,
	DO_SETUP = 1,
	DO_MAIN_LOOP = 2
};

#define MIN_FAR_MEM 3000000
#define MIN_LOW_MEM 100000
#define MUSIC_SLOT_SIZE 500000
#define SOUND_SLOT_SIZE 500000
#define SPEECH_HALF_BUF 25000

#define ERROR if(modul) {error();}

#define JXDIFF 180
#define JYDIFF 150

#define SAVEIO 1
#define LOADIO 2
#define ALLIO 3
#define PAL1 1

#define CUR_WALK 0
#define CUR_USE 1
#define CUR_LOOK 2
#define CUR_TALK 3
#define CUR_SAVE 4
#define CUR_INVENT 5
#define CUR_NO_WALK 6
#define CUR_NOPE 7
#define CUR_AK_INVENT 8
#define CUR_NO_USE 10
#define CUR_NO_LOOK 11
#define CUR_NO_TALK 12
#define CUR_AUSGANG_LINKS 13
#define CUR_AUSGANG_RECHTS 14
#define CUR_AUSGANG_OBEN 15
#define CUR_AUSGANG_UNTEN 16
#define CUR_USER 17

#define CUR_DISK 18
#define CUR_HOWARD 19
#define CUR_ZEIGE 20
#define CUR_NICHELLE 21
#define AUSGANG_LINKS_SPR 26
#define AUSGANG_RECHTS_SPR 27
#define AUSGANG_OBEN_SPR 28
#define AUSGANG_UNTEN_SPR 29

#define MENU_X 26
#define MENU_Y 21
#define MENU_CLIPX1 18
#define MENU_CLIPY1 5
#define MENU_CLIPX2 245
#define MENU_CLIPY2 30
#define MAX_MENU_SPRITE 13
#define MENU_START_SPRITE 0
#define WIN_INF_X 15
#define WIN_INF_Y 15
#define WIN_LOOK_X 30
#define WIN_LOOK_Y 150
#define PFEIL_UP 26
#define INVENTORY_HOTSPOTS_COUNT 8
#define HOT_LOOK_ANZ 2
#define MES_RAHMEN 1
#define MES_NO_RAHMEN 2
#define INV_ATS_MODE 0
#define INV_USE_ATS_MODE 1

#define WIN_INVENTAR WIN_INF_X,WIN_INF_Y,18,11,60,1
#define WIN_TEST 24,36,2,8,60,0
#define WIN_LOOK WIN_LOOK_X,WIN_LOOK_Y,10,6,60,1
#define WIN_LOOK2 WIN_LOOK_X+4,WIN_LOOK_Y+4,3,3,60,0

#define MAX_RAUM 70
#define MAX_MOV_OBJ 140

#define MAX_FEST_OBJ 140

#define MAX_EXIT 150
#define MAX_AUTO_MOV 20
#define MAX_TIMER_OBJ 10
#define MAX_ROOM_TIMER 10

#define ANI_VOR 0
#define ANI_RUECK 1
#define ANI_WAIT 0
#define ANI_GO 1
#define MAX_PROG_ANI 10

#define INVENTAR_NORMAL 0

#define INVENTAR_STATIC 1
#define STATIC_USE 2

#define DETEDIT_REC 3
#define AUTO_OBJ 4
#define DETAIL_OBJ 5
#define SPIELER_OBJ 6

#define CURSOR "misc/cursor.taf"
#define INVENTAR "misc/inventar.taf"
#define INVENTAR_IIB "misc/inventar.iib"
#define INVENTAR_SIB "misc/inventar.sib"
#define EXIT_EIB "misc/exit.eib"
#define CHEWY_TAF "misc/ch_walk.taf"
#define CHEWY_BO_TAF "misc/ch_bork.taf"
#define CHEWY_MI_TAF "misc/ch_mini.taf"
#define CHEWY_PUMP_TAF "misc/ch_pump.taf"
#define CHEWY_ROCK_TAF "misc/ch_rock.taf"
#define CHEWY_JMAN_TAF "misc/ch_mans.taf"
#define MENUTAF "misc/menu.taf"
#define CH_SPZ_FILE "misc/ch_spez.taf"
#define OPTION_TAF "MISC/OPTION.TAF"

#define SAVEDIR "save/savegame.\0"

#define DETAILTEST "room/test.rdi"

#define EPISODE1_TGP 0
#define GBOOK_TGP 1
#define EPISODE1 "back/episode1.tgp\0"
#define BUCH_START 2

#define EPISODE1_GEP "back/episode1.gep\0"

#define FONT6x8 "txt/6x8.tff\0"
#define FONT8x8 "txt/8x8.tff\0"

#define ATDS_TXT "txt/atds.tap"
#define INV_ATS_STEUER "txt/inv_st.s"
#define ROOM_ATS_STEUER "txt/room_st.s"
#define ADS_TXT_STEUER "txt/diah.adh"
#define INV_USE_IDX "txt/inv_use.idx"

#define DETAIL_TVP "sound/details.tap"
#define SPEECH_TVP "sound/speech.tvp"

#define ADSH_TMP "adsh.tmp"

#define MAX_PERSON 3

#define P_CHEWY 0
#define P_HOWARD 1
#define P_NICHELLE 2

#define CHEWY_NORMAL 1
#define CHEWY_BORK 2
#define CHEWY_MINI 3
#define CHEWY_PUMPKIN 4
#define CHEWY_ROCKER 5
#define CHEWY_JMANS 6

#define P_LEFT 0
#define P_RIGHT 1
#define LEFT 0
#define RIGHT 1
#define UP 2
#define DOWN 3

#define CH_UP 0
#define CH_DOWN 1
#define CH_LEFT_GE 2
#define CH_LEFT_NO 3
#define CH_LEFT_ZW 4
#define CH_RIGHT_GE 5
#define CH_RIGHT_NO 6
#define CH_RIGHT_ZW 7
#define CH_L_STEHEN 4

#define CH_R_STEHEN 7
#define CH_X_PIX 8
#define CH_Y_PIX 8
#define CH_HOT_MOV_X 8
#define CH_HOT_MOV_Y 25
#define CH_HOT_Y 34
#define CH_HOT_X 25

#define HO_HOT_MOV_X 8
#define HO_HOT_MOV_Y 59
#define HO_HOT_X 25
#define HO_HOT_Y 68
#define HO_X_PIX 8

#define NI_HOT_MOV_X 8
#define NI_HOT_MOV_Y 59
#define NI_HOT_X 25
#define NI_HOT_Y 68
#define NI_X_PIX 8

#define CH_SPZ_OFFSET 345
#define MAX_SPZ_PHASEN 30

#define CH_TALK1 0
#define CH_TALK2 1
#define CH_TALK3 2
#define CH_TALK4 3
#define CH_TALK5 4
#define CH_TALK6 5
#define CH_TALK7 6
#define CH_TALK8 7
#define CH_TALK9 8
#define CH_TALK10 9
#define CH_BAD1 10
#define CH_WONDER1 11
#define CH_EKEL 12
#define CH_LGET_O 13
#define CH_LGET_U 14
#define CH_TALK11 15
#define CH_TALK12 16
#define CH_TRANS 17
#define HO_WALK_L 18
#define HO_STAND_L 19
#define HO_TALK_L 20
#define HO_FRONT 21
#define HO_WALK_R 22
#define HO_STAND_R 23
#define HO_TALK_R 24
#define HO_BACK 25
#define CH_PUMP_TALK 26
#define CH_PUMP_GET1 27
#define CH_ROCK_GET1 28
#define CH_ROCK_HAND 29
#define CH_ROCK_TALK1 30
#define CH_ROCK_GET2 31
#define HO_BRILL_JMP 32
#define CH_JM_TITS 33
#define CH_JM_TALK 34
#define NI_WALK_L 35
#define NI_STAND_L 36
#define NI_TALK_L 37
#define NI_FRONT 38
#define NI_WALK_R 39
#define NI_STAND_R 40
#define NI_TALK_R 41
#define NI_BACK 42

#define ROOM_0_3 3

#define AUGE_TIMER 2
#define FUETTER_POS 3
#define KISSEN_POS 4
#define BLITZ_POS 5
#define VERSTECK_POS 6

#define PILLOW_INV 0
#define SLIME_INV 1
#define RED_CARD_INV 2
#define KABEL_INV 3
#define MONOKEL_INV 4
#define GBUCH_INV 5
#define GBUCH_OPEN_INV 6
#define DEFEKT_INV 7
#define STAHLSEIL_INV 8
#define BECHER_LEER_INV 9
#define BECHER_VOLL_INV 10
#define GIPS_EIMER_INV 11
#define SCHLOTT_INV 12
#define ZANGE_INV 13
#define MASKE_INV 14
#define KOHLE_HEISS_INV 15
#define KOHLE_KALT_INV 16
#define DEE_PAINT_INV 17
#define SURIMY_INV 18
#define BORK_INV 19
#define SPINAT_INV 20
#define YEL_CARD_INV 21
#define BWAFFE_INV 22
#define SICHERUNG_INV 23
#define FLUXO_INV 24
#define TRANSLATOR_INV 25
#define CARTRIDGE_INV 26
#define TALISMAN_INV 27
#define SEIL_INV 28
#define CYB_KRONE_INV 29
#define SCHLAUCH_INV 30
#define TRICHTER_INV 31
#define ANGEL_INV 32
#define MILCH_INV 33
#define MESSER_INV 34
#define PUMPE_INV 35
#define CENT_INV 36
#define ANGEL2_INV 37
#define GEBISS_INV 38
#define H_FUTTER_INV 39
#define KUERBIS1_INV 40
#define K_MASKE_INV 41
#define K_FLEISCH_INV 42
#define K_KERNE_INV 43
#define EIER_INV 44
#define KAFFEE_INV 45
#define PUTENKEULE_INV 46
#define KNOCHEN_INV 47
#define MANUSKRIPT_INV 48
#define RECORDER_INV 49
#define ZAPPER_INV 50
#define MUNTER_INV 51
#define MILCH_LEER_INV 52
#define MILCH_WAS_INV 53
#define PAPIER_INV 54
#define BRIEF_INV 55
#define TICKET_INV 56
#define ZEITUNG_INV 57
#define BMARKE_INV 58
#define BRIEF2_INV 59
#define DUENGER_INV 60
#define LIKOER_INV 61
#define LIKOER2_INV 62
#define DOLLAR175_INV 63
#define CASSETTE_INV 64
#define HOTEL_INV 65
#define KAPPE_INV 66
#define FLASCHE_INV 67
#define GUM_INV 68
#define CIGAR_INV 69
#define KEY_INV 70
#define BURGER_INV 71
#define WOLLE_INV 72
#define UHR_INV 73
#define RING_INV 74
#define SACKGELD_INV 75
#define KILLER_INV 76
#define CUTMAG_INV 77
#define JMKOST_INV 78
#define LEDER_INV 79
#define SPARK_INV 80
#define BESTELL_INV 81
#define VCARD_INV 82
#define EINLAD_INV 83
#define GERAET_INV 84
#define ASCHE_INV 85
#define PIRANHA_INV 86
#define SCHALL_INV 87
#define ARTE_INV 88
#define GALA_INV 89
#define PAPAGEI_INV 90
#define BAR_GUT_INV 91
#define CLINT_500_INV 92
#define CLINT_1500_INV 93
#define CLINT_3000_INV 94
#define B_MARY_INV 95
#define VIDEO_INV 96
#define B_MARY2_INV 97
#define KARTE_INV 98

#define SIB_SCHLEIM 3

#define NO_MOV_OBJ 127
#define CHEWY_OBJ 255
#define HOWARD_OBJ 254
#define NICHELLE_OBJ 253
#define AUTO_OBJ0 0
#define AUTO_OBJ1 1
#define AUTO_OBJ2 2
#define AUTO_OBJ3 3
#define AUTO_OBJ4 4
#define MAX_OBJ_MOV 3

#define ROOM_ATS_MAX 1000
#define INV_USE_ATS_MAX 500
#define ATS_ACTION_VOR 0
#define ATS_ACTION_NACH 1

#define CFO_MODE 0
#define FLC_MODE 1
#define ASF_MODE 2
#define FCUT_001 0
#define FCUT_002 1
#define FCUT_003 2
#define FCUT_004 3
#define FCUT_005 4
#define FCUT_006 5
#define FCUT_007 6
#define FCUT_008 7
#define FCUT_009 8
#define FCUT_010 9
#define FCUT_011 10
#define FCUT_012 11
#define FCUT_013 12
#define FCUT_014 13
#define FCUT_015 14
#define FCUT_016 15
#define FCUT_017 16
#define FCUT_018 17
#define FCUT_019 18
#define FCUT_029 29
#define FCUT_030 30
#define FCUT_031 31
#define FCUT_032 32
#define FCUT_033 33
#define FCUT_034 34
#define FCUT_035 35
#define FCUT_036 36
#define FCUT_037 37
#define FCUT_038 38
#define FCUT_039 39
#define FCUT_040 40
#define FCUT_041 41
#define FCUT_042 42
#define FCUT_043 43
#define FCUT_044 44
#define FCUT_045 45
#define FCUT_046 46
#define FCUT_047 47
#define FCUT_048 48
#define FCUT_049 49
#define FCUT_050 50
#define FCUT_051 51
#define FCUT_052 52
#define FCUT_053 53
#define FCUT_054 54
#define FCUT_055 55
#define FCUT_056 56
#define FCUT_057 57
#define FCUT_058 58
#define FCUT_059 59
#define FCUT_060 60
#define FCUT_061 61
#define FCUT_062 62
#define FCUT_063 63
#define FCUT_064 64
#define FCUT_065 65
#define FCUT_066 66
#define FCUT_067 67
#define FCUT_068 68
#define FCUT_069 69
#define FCUT_070 70
#define FCUT_071 71
#define FCUT_072 72
#define FCUT_073 73
#define FCUT_074 74
#define FCUT_075 75
#define FCUT_076 76
#define FCUT_077 77
#define FCUT_078 78
#define FCUT_079 79
#define FCUT_080 80
#define FCUT_081 81
#define FCUT_082 82
#define FCUT_083 83

#define AUSGANG_LINKS 1
#define AUSGANG_RECHTS 2
#define AUSGANG_OBEN 3
#define AUSGANG_UNTEN 4
#define WAIT_TASTE_LOS while (in->get_switch_code() != 0 \
	&& !SHOULD_QUIT) { g_events->update(); }

#define FILEMENUE 1

} // namespace Chewy

#endif
