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

#ifndef HODJNPODJ_PEGGLE_GAME2_H
#define HODJNPODJ_PEGGLE_GAME2_H

namespace Bagel {
namespace HodjNPodj {
namespace Peggle {

// Sprite identification types
#define SPRITE_GLASS    0
#define SPRITE_HOLE     1
#define SPRITE_INVALID  2

// midi sound file

#define GAME_THEME      ".\\SOUND\\PEGGLEBO.MID"

// Wave sound files

#define WAV_UNDO    ".\\SOUND\\SOUND208.WAV"
#define WAV_WON     ".\\SOUND\\SOUNDWON.WAV"
#define WAV_DONE    ".\\SOUND\\SOUND216.WAV"
#define WAV_MOVE    ".\\SOUND\\SOUND169.WAV"
#define WAV_NOMOVE  ".\\SOUND\\OLD169.WAV"

// Title display offsets

#define TITLE_X             14
#define TITLE_Y             3
#define TITLE_DX            219
#define TITLE_DY            17

// Bottle display offsets

#define BOTTLE_X            195
#define BOTTLE_Y            84
#define BOTTLE_DX           46
#define BOTTLE_DY           149

// DART display offsets

#define DART_X              268
#define DART_Y              106
#define DART_DX             279
#define DART_DY             63
#define NUM_DART_CELS       21

// DART Easter Egg hotspot offsets

#define EE_DART_X               487
#define EE_DART_Y               108
#define EE_DART_DX              58
#define EE_DART_DY              60

// KEG display offsets

#define KEG_X               15
#define KEG_Y               121
#define KEG_DX              125
#define KEG_DY              40
#define NUM_KEG_CELS        17
#define KEG_CEL_OFFSET      7

// STOOL display offsets

#define STOOL_X             14
#define STOOL_Y             148
#define STOOL_DX            98
#define STOOL_DY            125
#define NUM_STOOL_CELS      25

#define DART_SLEEP           80         // In milliseconds  70
#define KEG_SLEEP           115         // In milliseconds    150  130
#define STOOL_SLEEP          55         // In milliseconds

// Audio easter egg info:
#define NET_X               205
#define NET_Y               35
#define NET_DX              122
#define NET_DY              50
#define NET_SOUND           ".\\SOUND\\ANCHORS.WAV"

#define OAR_X               20
#define OAR_Y               53
#define OAR_DX              185
#define OAR_DY              52
#define OAR_SOUND           ".\\SOUND\\ROWBOAT.WAV"

#define SIGN_X              355
#define SIGN_Y              25
#define SIGN_DX             67
#define SIGN_DY             36
#define SIGN_SOUND          ".\\SOUND\\TAVERN.WAV"

#define TABLE_X             314
#define TABLE_Y             170
#define TABLE_DX            122
#define TABLE_DY            42
#define TABLE_SOUND         ".\\SOUND\\FRINIGHT.WAV"

#define CANDLENR_X          595
#define CANDLENR_Y          208
#define CANDLENR_DX         30
#define CANDLENR_DY         50

#define CANDLEFR_X          530
#define CANDLEFR_Y          183
#define CANDLEFR_DX         20
#define CANDLEFR_DY         20
#define CANDLE_SOUND        ".\\SOUND\\STNICMSG.WAV"

// Board display offsets

#define CROSS_BOARD_DX      210
#define CROSS_BOARD_DY      194
#define CROSS_PLUS_BOARD_DX 210
#define CROSS_PLUS_BOARD_DY 194
#define TRI_BOARD_DX        305
#define TRI_BOARD_DY        224
#define TRI_PLUS_BOARD_DX   304
#define TRI_PLUS_BOARD_DY   197

// Shot glass display offsets

#define CROSS_SHOTGLASS_DX          10
#define CROSS_SHOTGLASS_DDX         31
#define CROSS_SHOTGLASS_DDDX        3
#define CROSS_SHOTGLASS_DY          27
#define CROSS_PLUS_SHOTGLASS_DX     10
#define CROSS_PLUS_SHOTGLASS_DDX    31
#define CROSS_PLUS_SHOTGLASS_DDDX   3
#define CROSS_PLUS_SHOTGLASS_DY     27
#define TRI_SHOTGLASS_DX            24
#define TRI_SHOTGLASS_DY            27
#define TRI_PLUS_SHOTGLASS_DX       24
#define TRI_PLUS_SHOTGLASS_DY       27

// Array Status:

#define EMPTY        0
#define PEGGED       1
#define NO_HOLE      2

// Board Selection:

#define BOARD_COUNT     4
#define BOARD_BASE      10

#define CROSS           10
#define CROSS_PLUS      11
#define TRIANGLE        12
#define TRIANGLE_PLUS   13

#define GRID_SIZE       7         // may be changed later

// Sprite size:
#define SPRITE_SIZE_DX  15
#define SPRITE_SIZE_DY  25

#define HAND_SIZE       48

} // namespace Peggle
} // namespace HodjNPodj
} // namespace Bagel

#endif
