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

#ifndef HODJNPODJ_PDQ_GAME_H
#define HODJNPODJ_PDQ_GAME_H

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include "bagel/boflib/error.h"

namespace Bagel {
namespace HodjNPodj {
namespace PDQ {

#define MAX_PLENGTH     25
#define MAX_PLENGTH_S   30

#define MAX_TURNS       3           // Max Turn Count when playing the metagame

// Easter Egg info:
#define BIRD_X              43
#define BIRD_Y              241
#define BIRD_DX             94
#define BIRD_DY             44
#define NUM_BIRD_CELS       20
#define BIRD_SLEEP          40

#define HORSE1_X            158
#define HORSE1_Y            146
#define HORSE1_DX           85
#define HORSE1_DY           126
#define NUM_HORSE1_CELS     16
#define HORSE1_SLEEP        100

#define HORSE2_X            240
#define HORSE2_Y            188
#define HORSE2_DX           127
#define HORSE2_DY           249
#define NUM_HORSE2_CELS     14
#define HORSE2_SLEEP        100

#define FLOWER_X            578
#define FLOWER_Y            246
#define FLOWER_DX           47
#define FLOWER_DY           64
#define NUM_FLOWER_CELS     21
#define FLOWER_SLEEP        200

#define BIRD_ANIM       ".\\ART\\BIRD.BMP"
#define HORSE1_ANIM     ".\\ART\\HORSE1.BMP"
#define HORSE2_ANIM     ".\\ART\\HORSE2.BMP"
#define FLOWER_ANIM     ".\\ART\\FLOWER.BMP"

#define WAV_REVEAL      ".\\SOUND\\REVEAL.WAV"
#define WAV_BADGUESS    ".\\SOUND\\BADGUESS.WAV"
#define WAV_GAMEOVER    ".\\SOUND\\GAMEOVER.WAV"
#define WAV_YOUWIN      ".\\SOUND\\YOUWIN.WAV"
#define WAV_HORSE1      ".\\SOUND\\HORSE1.WAV"
#define WAV_HORSE2      ".\\SOUND\\HORSE2.WAV"
#define WAV_BIRD        ".\\SOUND\\BIRD.WAV"
#define WAV_FLOWER      ".\\SOUND\\WEIRD.WAV"

#define MID_SOUNDTRACK  ".\\SOUND\\THGESNG.MID"

/*
* prototypes
*/
extern ERROR_CODE  InitGame(HWND, CDC *);
extern ERROR_CODE  StartGame(CDC *);
extern void        GameStopTimer();
extern ERROR_CODE  GameStartTimer();
extern void        GamePauseTimer();
extern void        GameResumeTimer();
extern ERROR_CODE  EndGame(CDC *);
extern void        WinGame();
extern ERROR_CODE  RepaintSpriteList(CDC *);
extern bool     CheckUserGuess(const char *);
extern void CALLBACK GetGameParams(CWnd *);
extern void        GameGetScore(unsigned int *, unsigned int *, unsigned int *, unsigned int *);

} // namespace PDQ
} // namespace HodjNPodj
} // namespace Bagel

#endif
