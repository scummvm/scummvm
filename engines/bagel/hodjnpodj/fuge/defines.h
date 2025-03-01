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

#ifndef HODJNPODJ_FUGE_DEFINES_H
#define HODJNPODJ_FUGE_DEFINES_H

namespace Bagel {
namespace HodjNPodj {
namespace Fuge {

#define N_ROWS           6
#define BRICKS_PER_ROW  16
#define N_BRICKS        (N_ROWS * BRICKS_PER_ROW)

#define IDD_USERCFG 100

#define BALLS_MIN     1
#define BALLS_DEF     5
#define BALLS_MAX     5

#define LEVEL_MIN     1
#define LEVEL_DEF     6
#define LEVEL_MAX     6

#define SPEED_MIN     1
#define SPEED_DEF     6
#define SPEED_MAX     10

#define PSIZE_MIN     0
#define PSIZE_DEF     2
#define PSIZE_MAX     2

#define GFORCE_MIN     0
#define GFORCE_DEF     0
#define GFORCE_MAX     20

//
// This mini-game's main screen bitmap
//
#define MINI_GAME_MAP   "ART/FUGE6.BMP"

// Fuge sprite bitmaps
//
#define IDB_BALL        103

#define N_PADDLE_CELS    31
#define PADDLE_START_X  285
#define PADDLE_START_Y  205
#define PADDLE_CEL_JUMP   1
#define FACE_ANGLE      (DOUBLE)0.977322
#define PADDLE0_ANGLE   (PI/4)      // 45 degrees
#define PADDLE1_ANGLE   (PI/3)      // 60 degrees
#define PADDLE2_ANGLE   (PI/2)      // 90 degrees

#define MOUSE_SENS      32

#define BLACKHOLE_RADIUS     14
#define PADDLE_RADIUS        34
#define INNER_BRICK_RADIUS  102
#define ROW6_RADIUS         114
#define ROW5_RADIUS         129
#define ROW4_RADIUS         145
#define ROW3_RADIUS         160
#define ROW2_RADIUS         175
#define ROW1_RADIUS         190
#define WHEEL_RADIUS        191

#define BRICK0_ANGLE        0.000000 
#define BRICK1_ANGLE        0.392699 
#define BRICK2_ANGLE        0.785398 
#define BRICK3_ANGLE        1.178097 
#define BRICK4_ANGLE        1.570796 
#define BRICK5_ANGLE        1.963495 
#define BRICK6_ANGLE        2.356194 
#define BRICK7_ANGLE        2.748894 
#define BRICK8_ANGLE        3.141593 
#define BRICK9_ANGLE        3.534291
#define BRICK10_ANGLE       3.926990
#define BRICK11_ANGLE       4.319690 
#define BRICK12_ANGLE       4.712389 
#define BRICK13_ANGLE       5.105088 
#define BRICK14_ANGLE       5.497787 
#define BRICK15_ANGLE       5.890486


#define N_BLACKHOLE_CELS  5

#define N_BRICK_POINTS   21
#define N_BALL_MOVES      3

#define BALL_START_X    309
#define BALL_START_Y    180
#define BALL_SIZE_X      20
#define BALL_SIZE_Y      20
#define BALL_RADIUS     (BALL_SIZE_X/2)
#define CENTER_X        319
#define CENTER_Y        239
#define G_FORCE         0.009000

#define TURBO_MIN       0
#define TURBO_DEF      10
#define TURBO_MAX      10

#define EXTRA_LIFE_SCORE 100

//
// Button ID constants
//
#define IDC_MENU     100

#define TIMER_ID 10
#define TIMER_INTERVAL  50

// Sounds
//
#define WAV_PADDLE      ".\\SOUND\\PADDLE.WAV"
#define WAV_WALL        ".\\SOUND\\WALL.WAV"
#define WAV_WINWAVE     ".\\SOUND\\WINWAVE.WAV"
#define WAV_GAMEOVER    ".\\SOUND\\SOSORRY.WAV"     //GAMEOVER.WAV"
#define WAV_LOSEBALL    ".\\SOUND\\TOILET.WAV"      //LOSEBALL.WAV"

#define WAV_NARRATION   ".\\SOUND\\FUGE.WAV"
#define MID_SOUNDTRACK  ".\\SOUND\\FUGE.MID"

#define WAV_CAR1        ".\\SOUND\\SICK.WAV"
#define WAV_CAR2        ".\\SOUND\\FERRIS.WAV"
#define WAV_TENT        ".\\SOUND\\LAWYER.WAV"
#define WAV_BOOTH       ".\\SOUND\\STEPUP.WAV"
#define WAV_PEOPLE1     ".\\SOUND\\BALLOON.WAV"
#define WAV_PEOPLE2     ".\\SOUND\\AUNTEDNA.WAV"

#define NUM_WAVS        2                           //There are two people and two car sounds

// Audio Easter Egg areas
//
#define TENT_X          16
#define TENT_Y          320
#define TENT_DX         75
#define TENT_DY         47

#define BOOTH_X         503
#define BOOTH_Y         377
#define BOOTH_DX        93  
#define BOOTH_DY        53

#define PEOPLE_X        27  
#define PEOPLE_Y        370 
#define PEOPLE_DX       102 
#define PEOPLE_DY       60  

#define CAR1_X          131
#define CAR1_Y          385
#define CAR_DX          41
#define CAR_DY          49

#define CAR2_X          84
#define CAR2_Y          231

#define CAR3_X          95
#define CAR3_Y          149

#define CAR4_X          133
#define CAR4_Y          73

#define CAR5_X          465
#define CAR5_Y          75

#define CAR6_X          503
#define CAR6_Y          150

#define CAR7_X          514
#define CAR7_Y          230

#define CAR8_X          500
#define CAR8_Y          315

#define CAR9_X          218
#define CAR9_Y          23
#define CAR9_DX         37
#define CAR9_DY         39

#define CAR10_X         384
#define CAR10_Y         23
#define CAR10_DX        39
#define CAR10_DY        39

#define N_ROWS           6
#define BRICKS_PER_ROW  16
#define N_BRICKS        (N_ROWS * BRICKS_PER_ROW)

} // namespace Fuge
} // namespace HodjNPodj
} // namespace Bagel

#endif
