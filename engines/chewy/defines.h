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
#define ATS_TAP_OFF ADS_TAP_MAX	// 500
#define AAD_TAP_OFF ATS_TAP_OFF+ATS_TAP_MAX	// 500 + 100
#define INV_TAP_OFF AAD_TAP_OFF+AAD_TAP_MAX	// 500 + 100 + 100
#define USE_TAP_OFF INV_TAP_OFF+INV_TAP_MAX	// 500 + 100 + 100 + 140

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

enum SetupScreenMode {
	NO_SETUP = 0,
	DO_SETUP = 1,
	DO_MAIN_LOOP = 2
};

#define MIN_FAR_MEM 3000000
#define MIN_LOW_MEM 100000
#define SPEECH_HALF_BUF 25000

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
#define CUR_EXIT_LEFT 13
#define CUR_EXIT_RIGHT 14
#define CUR_EXIT_TOP 15
#define CUR_EXIT_BOTTOM 16
#define CUR_USER 17

#define CUR_DISK 18
#define CUR_HOWARD 19
#define CUR_ZEIGE 20
#define CUR_NICHELLE 21
#define CUR_BLASTER 22
#define EXIT_LEFT_SPR 26
#define EXIT_RIGHT_SPR 27
#define EXIT_ABOVE_SPR 28
#define EXIT_BOTTOM_SPR 29

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
#define HOT_LOOK_NR 2
#define MES_RAHMEN 1
#define MES_NO_RAHMEN 2
#define INV_ATS_MODE 0
#define INV_USE_ATS_MODE 1

#define WIN_INVENTORY WIN_INF_X,WIN_INF_Y,18,11,60,1
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

#define ANI_FRONT 0
#define ANI_BACK 1
#define ANI_WAIT 0
#define ANI_GO 1
#define MAX_PROG_ANI 10

#define INVENTORY_NORMAL 0

#define INVENTORY_STATIC 1
#define STATIC_USE 2

#define DETEDIT_REC 3
#define AUTO_OBJ 4
#define DETAIL_OBJ 5
#define SPIELER_OBJ 6

#define EPISODE1_TGP 0
#define GBOOK_TGP 1
#define NOTEBOOK_START 2
#define DIARY_START 3

#define CURSOR_TAF "misc/cursor.taf"
#define INVENTORY_TAF "misc/inventar.taf"
#define INVENTORY_IIB "misc/inventar.iib"
#define INVENTORY_SIB "misc/inventar.sib"
#define EXIT_EIB "misc/exit.eib"
#define CHEWY_TAF "misc/ch_walk.taf"
#define CHEWY_BO_TAF "misc/ch_bork.taf"
#define CHEWY_MI_TAF "misc/ch_mini.taf"
#define CHEWY_PUMP_TAF "misc/ch_pump.taf"
#define CHEWY_ROCK_TAF "misc/ch_rock.taf"
#define CHEWY_JMAN_TAF "misc/ch_mans.taf"
#define CHEWY_HELM_TAF "misc/ch_helm.taf"
#define MENUTAF "misc/menu.taf"
#define CH_SPZ_FILE "misc/ch_spez.taf"
#define OPTION_TAF "misc/option.taf"

#define DETAILTEST "room/test.rdi"
#define GBOOK "back/gbook.tgp"

#define EPISODE1 "back/episode1.tgp"
#define EPISODE1_GEP "back/episode1.gep"

#define FONT6x8 "txt/6x8.tff"
#define FONT8x8 "txt/8x8.tff"

#define ATDS_TXT "txt/atds.tap"
#define INV_ATS_STEUER "txt/inv_st.s"
#define ROOM_ATS_STEUER "txt/room_st.s"
#define ADS_TXT_STEUER "txt/diah.adh"
#define INV_USE_IDX "txt/inv_use.idx"

#define DETAIL_TVP "sound/details.tap"

#define QUIT_MSG_EN "QUIT ?? Y/N "
#define QUIT_MSG_DE "BEENDEN ?? J/N "

#define MAX_PERSON 3

#define P_CHEWY 0
#define P_HOWARD 1
#define P_NICHELLE 2
#define P_3 3
#define P_4 4

#define CHEWY_NORMAL 1
#define CHEWY_BORK 2
#define CHEWY_MINI 3
#define CHEWY_PUMPKIN 4
#define CHEWY_ROCKER 5
#define CHEWY_JMANS 6
#define CHEWY_ANI7 7

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
#define MONOCLE_INV 4
#define NOTEBOOK_INV 5
#define NOTEBOOK_OPEN_INV 6
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
#define MACHETE_INV 99
#define RUBBER_INV 100
#define FOOD_CERTIFICATE_INV 101
#define DIARY_INV 109

#define SIB_SLIME 3

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

#define FCUT_000 0	// Chewy escapes the prison cell
#define FCUT_001 1	// Burger
#define FCUT_002 2	// Chewy mocks an alien and he trips on a rope
#define FCUT_003 3	// Jumping rat-line creature
#define FCUT_004 4	// Chewy sticks his hand in the rat's hole (custom callback: Room6::cut_serv1)
#define FCUT_005 5	// Chewy sticks the burger in the rat's hole and catches it (custom callback: Room6::cut_serv2)
#define FCUT_006 6	// Chewy pushes the big red button and a huge anvil drops on him
#define FCUT_007 7	// Chewy paints "don't" in front of "push this"
#define FCUT_008 8	// Alien pushes the big red button and a huge anvil drops on him
#define FCUT_009 9	// Chewy tries the transporter (custom callback: Room11::cut_serv)
#define FCUT_010 10	// Chewy uses a deflated alien in the transporter (custom callback: Room12::cut_serv)
#define FCUT_011 11	// Four panel video, where Chewy flies in a spaceship
#define FCUT_012 12	// Chewy messes with a conveyor belt
#define FCUT_013 13	// Chewy is transported in place of an alien
#define FCUT_014 14	// Chewy jumps on a conveyor belt
#define FCUT_015 15	// Transporter
#define FCUT_016 16	// Transporter
#define FCUT_017 17	// Chewy drops from a conveyor belt into a dumpster
#define FCUT_SPACECHASE_18 18	// Space chase: Chewy escapes F5 with spaceship
#define FCUT_SPACECHASE_19 19	// Space chase: Spaceship flight
#define FCUT_SPACECHASE_20 20  // Space chase: Spaceship flight
#define FCUT_SPACECHASE_21 21	// Space chase: Aliens chasing Chewy's spaceship
#define FCUT_SPACECHASE_22 22  // Space chase: Dogfight with Chewy and alien spaceship
#define FCUT_SPACECHASE_23 23  // Space chase: Chewy's spaceship enters bumper field and black hole
#define FCUT_SPACECHASE_24 24  // Space chase: Chewy's spaceship travels through black hole
#define FCUT_SPACECHASE_25 25  // Space chase: Chewy's spaceship approaches earth
#define FCUT_SPACECHASE_26 26  // Space chase: Chewy's spaceship starts descending
#define FCUT_SPACECHASE_27 27  // Space chase: Chewy's spaceship approaches farm
#define FCUT_SPACECHASE_28 28	// Space chase: Chewy's spaceship performs crash landing near farm
#define FCUT_029 29 // Chewy wakes up after crash landing
#define FCUT_030 30	// Chewy douses shaceship fire
#define FCUT_031 31	// Chewy uses kitchen appliance
#define FCUT_032 32	// TV: Farmer speaking (custom callback: Room39::setup_func)
#define FCUT_033 33	// TV: Gasp ghost show intro
#define FCUT_034 34 // TV: Gasp ghost show (custom callback: Room39::setup_func)
#define FCUT_035 35 // TV: Book sale show (custom callback: Room39::setup_func)
#define FCUT_036 36	// TV: Laura letter - head chopping block
#define FCUT_037 37 // TV: Tied girl and aliens (custom callback: Room39::setup_func)
#define FCUT_038 38 // TV: Flap news (custom callback: Room39::setup_func)
#define FCUT_039 39 // TV: Flap news with alien (custom callback: Room39::setup_func)
#define FCUT_040 40 // TV: Cooking show (custom callback: Room39::setup_func)
#define FCUT_041 41	// Using translator on TV to learn English
#define FCUT_042 42	// Turning on TV
#define FCUT_043 43	// Chewy trying to wake up Howard
#define FCUT_044 44	// Chewy using head band with typewriter
#define FCUT_045 45	// Using translator on cat
#define FCUT_046 46	// Using rat in the attic
#define FCUT_047 47	// Using chicken feed on seesaw 
#define FCUT_048 48	// Using fishing pole and bone to grab the dog dentures
#define FCUT_049 49	// Closeup of Chewy having taken the dog dentures
#define FCUT_050 50	// Barking dog
#define FCUT_051 51	// Barking dog without dentures
#define FCUT_052 52 // Using translator on chicken
#define FCUT_053 53	// Chewy watching a fight
#define FCUT_054 54	// Chewy mocking animal
#define FCUT_055 55	// Using rat on old car
#define FCUT_056 56	// Using rat in front of old car, flower appears
#define FCUT_057 57	// Chewy with pumpkin head in the mail office using the vending machine
#define FCUT_058 58	// Zombie girl jumping on train tracks
#define FCUT_059 59	// Zombie girl talking to the pumpkin dummy, causing its head to explode
#define FCUT_060 60	// Zombie girl talking to the pumpkin dummy and crashing into the store
#define FCUT_061 61 // Zombie girl getting scared, shrinks and is placed in bottle by Howard (custom callback: Room43::setup_func)
#define FCUT_062 62	// Zombie girl talks from inside the bottle
#define FCUT_063 63	// Howard and Chewy walking beside a moving truck
#define FCUT_064 64	// Giant pumpkin bursts through the house
#define FCUT_065 65	// Policeman fighting Howard and Chewy
#define FCUT_066 66 // Policeman loses from Howard and Chewy
#define FCUT_067 67	// Chewy throws smoke at kid, so it spits a chewing gum at Howard
#define FCUT_068 68	// Man runs out of room full of spiders
#define FCUT_069 69	// Chewy enters phone booth, transforms into blonde woman
#define FCUT_070 70	// Woman runs out of room full of spiders
#define FCUT_071 71	// Best seller book presentation
#define FCUT_072 72	// Chewy removes his woman disguise
#define FCUT_073 73	// Train moving
#define FCUT_074 74	// Howard and Chewy at docks, another man appears
#define FCUT_075 75	// Howard and Chewy at docks, get thrown out of store
#define FCUT_076 76	// Howard inside box, lid closes
#define FCUT_077 77	// Chewy turns TV host into alien
#define FCUT_078 78	// Fire alarm in TV console room
#define FCUT_079 79	// Running away in the sewers
#define FCUT_080 80	// Gasp ghost show TV set, rain starts
#define FCUT_081 81	// Gasp ghost show TV set, Chewy and alien
#define FCUT_082 82	// Gasp ghost show TV set, large monster prop is dropped on alien
#define FCUT_083 83	// Lady screaming at piranha in cocktail glass
#define FCUT_084 84 // Dropping inside trap in jungle
#define FCUT_085 85 // Dropping inside trap in jungle (with Howard)
#define FCUT_086 86 // King kong chasing hunters
#define FCUT_087 87 // King kong fights alien spaceships
#define FCUT_088 88 // King kong shouting
#define FCUT_089 89	// Lasers shot at heroes outside temple (custom callback: Room87::proc5)
#define FCUT_090 90	// Aliens abduct Nichelle
#define FCUT_091 91 // Alien ship flies away with Nichelle
#define FCUT_092 92 // Chewy taunts aliens opposite a cave
#define FCUT_093 93 // Chewy, pink ball and statue turning into pink alien
#define FCUT_094 94 // Pink alien talking (custom callback: Room87::proc3)
#define FCUT_095 95 // Pink alien talking (custom callback: Room87::proc5)
#define FCUT_096 96	// Pink alien jumping down
#define FCUT_097 97 // Aliens coming out of door and shooting
#define FCUT_098 98	// Chewy and pink alien running away from aliens
#define FCUT_099 99	// Pink alien taking pink ball
#define FCUT_100 100 // Heroes running away from aliens
#define FCUT_101 101 // Heroes falling off waterfall
#define FCUT_102 102 // Spaceship flying away from jungle and King Kong
#define FCUT_103 103 // Spaceship flying
#define FCUT_104 104 // Howard and Chewy beaming down
#define FCUT_105 105 // Spaceship beams up the heroes and flies away
#define FCUT_106 106 // Pink alien beams down the heroes
#define FCUT_107 107 // Chewy shooting at slime police zeppelin (custom callback: Room90::proc5)
#define FCUT_108 108 // Aliens entering hotel
#define FCUT_109 109 // Rat jumping closeup
#define FCUT_110 110 // Aliens at docks
#define FCUT_111 111 // Rat closeup
#define FCUT_112 112 // Alien band playing music (custom callback: Room56::proc1)
#define FCUT_113 113 // Howard and Chewy beaming up
#define FCUT_114 114 // Howard and Chewy opening a door leading outside building
#define FCUT_115 115 // Aliens falling off building
#define FCUT_116 116 // Ship in ocean
#define FCUT_117 117 // Volcano erupting, F5 emerges from inside
#define FCUT_118 118 // Ship flying above town
#define FCUT_119 119 // Ship flying above town
#define FCUT_120 120 // Howard and Chewy beaming down, shooting starts
#define FCUT_121 121 // Zombie sheriff shoots small winding pink aliens
#define FCUT_122 122 // Alien chases small winding pink alien
#define FCUT_123 123 // Howard and Chewy on a wagon jumping a gap on train track
#define FCUT_124 124 // Howard and Chewy on wagon
#define FCUT_125 125 // Wagon crashes into alien
#define FCUT_126 126 // Wagon running next to shooting alien
#define FCUT_127 127 // Wagon reaching the end of the tracks
#define FCUT_128 128 // Wagon reaching bridge
#define FCUT_129 129 // Wagon crossing bridge
#define FCUT_130 130 // Wagon crossing bridge
#define FCUT_131 131 // Wagon moving
#define FCUT_132 132 // Wagon moving
#define FCUT_133 133 // Howard and Chewy on wagon
#define FCUT_134 134 // Red spaceships (custom callback: flic_user_function)
#define FCUT_135 135 // Intro - red spaceships (custom callback: flic_user_function)
#define FCUT_136 136 // Red spaceship shooting spaceship (custom callback: flic_user_function)
#define FCUT_137 137 // Dogfight between red spaceship and spaceship (custom callback: flic_user_function)
#define FCUT_138 138 // Spaceships flying from F5 (custom callback: flic_user_function)
#define FCUT_139 139 // Chewy's spaceship enters bumper field (custom callback: flic_user_function)
#define FCUT_140 140 // Purple aliens talking from inside spaceships (custom callback: flic_user_function)
#define FCUT_141 141 // Purple alien talking from inside spaceship (custom callback: flic_user_function)
#define FCUT_142 142 // Purple alien talking from inside spaceship (custom callback: flic_user_function)
#define FCUT_143 143 // Purple alien talking from inside spaceship (custom callback: flic_user_function)
#define FCUT_144 144 // Purple alien talking from inside spaceship (custom callback: flic_user_function)
#define FCUT_145 145 // Purple alien talking from inside spaceship
#define FCUT_146 146 // Purple alien talking from inside spaceship (custom callback: flic_user_function)
#define FCUT_147 147 // Red space ship approaching Earth (custom callback: flic_user_function)
#define FCUT_148 148 // Purple alien entering F5 and getting pink orb (custom callback: flic_user_function)
#define FCUT_149 149 // Space ship shooting green bullet (custom callback: flic_user_function)
#define FCUT_150 150 // Space ship generating force field (custom callback: flic_user_function)
#define FCUT_151 151 // Alien talking from inside spaceship (custom callback: flic_user_function)
#define FCUT_152 152 // Purple alien crash landing outside temple, natives appear (custom callback: flic_user_function)
#define FCUT_153 153 // Red space ship crash landing in jungle (custom callback: flic_user_function)
#define FCUT_154 154 // Red space ship being chased (custom callback: flic_user_function)
#define FCUT_155 155 // Intro logos (German)
#define FCUT_156 156 // Red space ship entering black hole (custom callback: flic_user_function)
#define FCUT_157 157 // Red space ship traveling inside black hole (custom callback: flic_user_function)
#define FCUT_158 158 // Howard and Chewy waking up
#define FCUT_159 159 // Credits
#define FCUT_160 160 // Intro logos (English)

#define EXIT_LEFT 1
#define EXIT_RIGHT 2
#define EXIT_TOP 3
#define EXIT_BOTTOM 4
#define WAIT_TASTE_LOS while (_G(in)->getSwitchCode() != 0 \
	&& !SHOULD_QUIT) { g_events->update(); }

} // namespace Chewy

#endif
