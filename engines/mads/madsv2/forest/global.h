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

#ifndef MADS_FOREST_GLOBAL_H
#define MADS_FOREST_GLOBAL_H

#include "common/serializer.h"
#include "mads/madsv2/core/vocabh.h"
#include "mads/madsv2/forest/extra.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {

enum {	
	walker_timing             =   0,   /* Stop walker timing (long) */
	walker_timing_2           =   1,
	inventory_is_displayed    =   2,   /* T if inventory is being displayed */
	player_hyperwalked        =   3,
	player_selected_object    =   4,
	walker_converse_state     =   5,
	walker_converse_now       =   6,
	g007                      =   7,
	g008                      =   8,
	g009                      =   9,
	play_background_sounds    =  10,  /* Play environmental sounds */
	player_score_flags        =  11,  /* 16 flags for checking if player increased score */
	dont_load_walker          =  12,  /* if T, will not load walker in section code */
	perform_displacements     =  13,  /* if T, will do displacements */
	intro                     =  14,  /* if T, Intro is in progress */
	outro                     =  15,  /* if T, Outro is in progress */
	g016                      =  16,
	g017                      =  17,
	g018                      =  18,
	g019                      =  19,
	g020                      =  20,
	tunnel_exit_1_opened      =  21,
	g022                      =  22,
	g023                      =  23,
	g024                      =  24,
	g025                      =  25,
	g026                      =  26,
	g027                      =  27,
	g028                      =  28,
	g029                      =  29,
	g030                      =  30,
	g031                      =  31,
	g032                      =  32,
	g033                      =  33,
	g034                      =  34,
	g035                      =  35,
	g036                      =  36,
	g037                      =  37,
	g038                      =  38,
	g039                      =  39,
	tunnel_exit_2_opened      =  40,
	tunnel_1_room             =  41,
	tunnel_2_room             =  42,
	tunnel_3_room             =  43,
	tunnel_4_room             =  44,
	tunnel_5_room             =  45,
	g046                      =  46,
	g047                      =  47,
	g048                      =  48,
	g049                      =  49,
	g050                      =  50,
	g051                      =  51,
	g052                      =  52,
	g053                      =  53,
	g054                      =  54,
	g055                      =  55,
	g056                      =  56,
	g057                      =  57,
	g058                      =  58,
	g059                      =  59,
	tunnel_exit_4_opened      =  60,
	tunnel_exit_5_opened      =  61,
	g062                      =  62,
	tunnel_exit_3_opened      =  63,
	g064                      =  64,
	g065                      =  65,
	phineas_status            =  66,
	g067                      =  67,
	g068                      =  68,
	g069                      =  69,
	g070                      =  70,
	g071                      =  71,
	g072                      =  72,
	g073                      =  73,
	g074                      =  74,
	g075                      =  75,
	g076                      =  76,
	g077                      =  77,
	g078                      =  78,
	g079                      =  79,
	g080                      =  80,
	g081                      =  81,
	g082                      =  82,
	g083                      =  83,
	g084                      =  84,
	g085                      =  85,
	g086                      =  86,
	g087                      =  87,
	g088                      =  88,
	g089                      =  89,
	g090                      =  90,
	g091                      =  91,
	g092                      =  92,
	g093                      =  93,
	g094                      =  94,
	g095                      =  95,
	g096                      =  96,
	g097                      =  97,
	g098                      =  98,
	g099                      =  99,
	g100                      = 100,
	g101                      = 101,
	g102                      = 102,
	g103                      = 103,
	g104                      = 104,
	g105                      = 105,
	g106                      = 106,
	g107                      = 107,
	g108                      = 108,
	g109                      = 109,
	g110                      = 110,
	g111                      = 111,
	g112                      = 112,
	g113                      = 113,
	g114                      = 114,
	g115                      = 115,
	g116                      = 116,
	g117                      = 117,
	g118                      = 118,
	g119                      = 119,
	g120                      = 120,
	g121                      = 121,
	g122                      = 122,
	g123                      = 123,
	g124                      = 124,
	g125                      = 125,
	g126                      = 126,
	g127                      = 127,
	g128                      = 128,
	g129                      = 129,
	g130                      = 130,
	g131                      = 131,
	g132                      = 132,
	g133                      = 133,
	g134                      = 134,
	g135                      = 135,
	g136                      = 136,
	g137                      = 137,
	g138                      = 138,
	g139                      = 139,
	g140                      = 140,
	g141                      = 141,
	g142                      = 142,
	g143                      = 143,
	g144                      = 144,
	g145                      = 145,
	g146                      = 146,
	g147                      = 147,
	g148                      = 148,
	g149                      = 149,
	g150                      = 150,
	g151                      = 151,
	g152                      = 152,
	g153                      = 153,
	g154                      = 154,
	g155                      = 155,
	g156                      = 156,
	g157                      = 157,
	g158                      = 158,
	g159                      = 159,
	g160                      = 160,
	g161                      = 161,
	g162                      = 162,
	g163                      = 163,
	g164                      = 164,
	g165                      = 165,
	g166                      = 166,
	g167                      = 167,
	g168                      = 168,
	g169                      = 169,
	g170                      = 170,
	g171                      = 171,
	g172                      = 172,
	g173                      = 173,
	g174                      = 174,
	g175                      = 175,
	g176                      = 176,
	g177                      = 177,
	g178                      = 178,
	g179                      = 179,
	g180                      = 180,
	g181                      = 181,
	g182                      = 182,
	g183                      = 183,
	g184                      = 184,
	g185                      = 185,
	g186                      = 186,
	g187                      = 187,
	g188                      = 188,
	g189                      = 189,
	g190                      = 190,
	g191                      = 191,
	g192                      = 192,
	g193                      = 193,
	g194                      = 194,
	g195                      = 195,
	g196                      = 196,
	g197                      = 197,
	g198                      = 198,
	g199                      = 199,
	g200                      = 200,
	g201                      = 201,
	g202                      = 202,
	g203                      = 203,
	g204                      = 204,
	g205                      = 205,
	g206                      = 206,
	g207                      = 207,
	g208                      = 208,
	g209                      = 209,
	g210                      = 210
};

// phineas_status
#define PHIN_IS_IN_CONTROL_AGAIN 2

struct AnimationInfo {
	int16 _active;
	int16 _frame;
	int16 _val3;
	int16 _val4;

	void synchronize(Common::Serializer &s) {
		s.syncMultipleLE(_active, _frame, _val3, _val4);
	}
};

extern int16 flags[40];
extern bool lets_get_a_move_on_anim;
extern bool inv_enable_command;

extern void global_init();
extern void global_section_constructor();
extern void sync_room(Common::Serializer &s);
extern void global_section_walker();
extern void global_section_interface();

extern void global_room_init();
extern void global_error_code();
extern void global_anim1(int arg_0, int arg_2, int arg_4, int16 *arg_6);
extern void global_anim2(int arg_0, int arg_2, int arg_4, int16 *arg_6);
extern void global_anim3(int handle, int16 *frame);
extern void global_midi_play(int num);
extern void global_daemon_code();
extern void global_game_main_loop();

} // namespace Forest
} // namespace MADSV2
} // namespace MADS

#endif
