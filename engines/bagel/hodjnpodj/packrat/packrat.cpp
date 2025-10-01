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

#include "bagel/boflib/misc.h"
#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/packrat/packrat.h"
#include "bagel/hodjnpodj/packrat/dialogs.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Packrat {

#define DYNAMIC_OBJ_COUNT

#define PACKRATTIMER    97

#define MAXLIVESPRITE   5
#define MAXLIVES        10

#define MOVEPLAYER  16
#define MOVEBADGUY  16

#define NUMBEROFPLAYERCELLS 8
#define NUMBEROFBADGUYCELLS 7

#define MAZETOP     26
#define MAZEBOTTOM  442
#define MAZELEFT    22
#define MAZERIGHT   598

#define FOLLOWRIGHTWALL     1
#define FOLLOWLEFTWALL      2

#define PLAYER          6
#define WALL            -1
#define OBJECT          1
#define SUPEROBJ1       2
#define SUPEROBJ2       3
#define SUPEROBJ3       4
#define SUPEROBJ4       5
#define BADGUY1         10
#define BADGUY2         20
#define BADGUY3         30
#define BADGUY4         40

#define NUMBEROFROWS    27
#define NUMBEROFCOLS    37

#define EATTURTLE           500
#define RELEASETURTLECOUNT  100

#define IDC_RESETGAME       888

#define MAZE1               "ART\\PKRT1.BMP"
#define MAZE2               "ART\\PKRT2.BMP"
#define MAZE3               "ART\\PKRT3.BMP"
#define MAZE4               "ART\\PKRT4.BMP"

#define OPTIONU             "ART\\OPTIONU.BMP"
#define OPTIONG             "ART\\OPTIONG.BMP"

#define BADGUYSPRITEU       "ART\\TURTLEBU.BMP"
#define BADGUYSPRITED       "ART\\TURTLEBD.BMP"
#define BADGUYSPRITEL       "ART\\TURTLEBL.BMP"
#define BADGUYSPRITER       "ART\\TURTLEBR.BMP"

#define GOODGUYSPRITEU      "ART\\TURTLEGU.BMP"
#define GOODGUYSPRITED      "ART\\TURTLEGD.BMP"
#define GOODGUYSPRITEL      "ART\\TURTLEGL.BMP"
#define GOODGUYSPRITER      "ART\\TURTLEGR.BMP"

#define FLASHSPRITEU        "ART\\TURTLEFU.BMP"
#define FLASHSPRITED        "ART\\TURTLEFD.BMP"
#define FLASHSPRITEL        "ART\\TURTLEFL.BMP"
#define FLASHSPRITER        "ART\\TURTLEFR.BMP"

#define PODJUPSPRITE        "ART\\PODJU.BMP"
#define PODJDOWNSPRITE      "ART\\PODJD.BMP"
#define PODJLEFTSPRITE      "ART\\PODJL.BMP"
#define PODJRIGHTSPRITE     "ART\\PODJR.BMP"

#define HODJUPSPRITE        "ART\\HODJU.BMP"
#define HODJDOWNSPRITE      "ART\\HODJD.BMP"
#define HODJLEFTSPRITE      "ART\\HODJL.BMP"
#define HODJRIGHTSPRITE     "ART\\HODJR.BMP"

#define LIVESPRITE          "ART\\LIVE.BMP"
#define BALLOONSPRITE       "ART\\BALLOONS.BMP"

#define SSPACEBMP           "ART\\SEMPTY.BMP"
#define LSPACEBMP           "ART\\LEMPTY.BMP"

#define BADGUYSOUND         ".\\SOUND\\GULP.WAV"
#define PLAYERSOUND         ".\\SOUND\\SORRY2.WAV"
#define SUPERSOUND          ".\\SOUND\\POWER.WAV"

#define NEWLIFE_WAV         ".\\SOUND\\NEWLIFE.WAV"

#define BG1     1
#define BG2     2
#define BG3     4
#define BG4     8

extern CMainPackRatWindow   *pcwndPackRat;

CPalette    *pGamePalette = nullptr;           // Palette to be used throughout the game
CBmpButton      *pOptionButton = nullptr;      // Option button object for getting to the options dialog

CSprite *pPlayerRightSprite = nullptr;
CSprite *pPlayerLeftSprite = nullptr;
CSprite *pPlayerUpSprite = nullptr;
CSprite *pPlayerDownSprite = nullptr;

CSprite *pBadGuyUpSprite  = nullptr;
CSprite *pBadGuyDownSprite  = nullptr;
CSprite *pBadGuyLeftSprite  = nullptr;
CSprite *pBadGuyRightSprite  = nullptr;

CSprite *pGoodGuyUpSprite  = nullptr;
CSprite *pGoodGuyDownSprite  = nullptr;
CSprite *pGoodGuyLeftSprite  = nullptr;
CSprite *pGoodGuyRightSprite  = nullptr;

CSprite *pFlashUpSprite  = nullptr;
CSprite *pFlashDownSprite  = nullptr;
CSprite *pFlashLeftSprite  = nullptr;
CSprite *pFlashRightSprite  = nullptr;

CSprite *pLiveSprite  = nullptr;
CSprite *pLivesSprite[MAXLIVESPRITE];

CSprite *pBalloonSprite  = nullptr;

CSprite *pPlayerSprite = nullptr;
CSprite *apBadGuySprite[4];

CBitmap *pSSpaceBMP = nullptr;
CBitmap *pLSpaceBMP = nullptr;

static  CSound  *pGameSound = nullptr;                             // Game theme song

POINT   ptNOLocInGrid;
int     nPaintObject;

int nPlayerMove;
int anBadGuyMove[4];

bool    bFirstTimer;
bool    bEatTurtle;
bool    abChangeTurtle[4];
int     nEatTurtle;
bool    bChangeTurtle;
int     nOldTSpeed;

bool    abGoodTurtle[4];

bool    anSuperObj[4];

bool    bEndGame;
bool    bFlashTurtle;

int     anReleaseTurtleCount[4];

bool    bUseJoyStick;
unsigned int    awJoyCenter[2];

int     nNumOfBGKilled;
int     nStartingLives;


int     nKillBadGuy = 0;
bool    bKillPlayer = false;

bool    bResetGame;

bool    bMouseHidden = false;
bool    bInNewGameRect = false;
bool    bStart;

int anCurrentMaze[999];
const int anMaze4[999] = {
	-1, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, -1,
	-1, 0, 0, -1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, -1, 0, 0, -1,
	-1, 0, 0, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, 0, 0, -1,
	-1, -1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1,
	-1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 5, 1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, 1, 2, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, -1,
	-1, 1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, 1, 1, 1, 1, 1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1,
	-1, 1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1,
	-1, 1, -1, -1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, -1, -1, 1, -1,
	-1, 1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1,
	-1, 1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1,
	-1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 1, -1,
	-1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, 1, 1, 1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, 1, 1, 1, -1, -1, -1, -1, -1, -1,
	0, 1, 1, 1, -1, -1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, -1, -1, 1, 1, 1, 0,
	-1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1,
	-1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1,
	-1, 1, 1, 1, 1, 1, 1, -1, -1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, -1, -1, 1, 1, 1, 1, 1, 1, -1,
	-1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1,
	-1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1,
	-1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, -1,
	-1, 1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, 1, -1,
	-1, 1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, 1, -1,
	-1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, -1,
	-1, -1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1,
	-1, 0, 0, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, 0, 0, -1,
	-1, 0, 0, -1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, -1, 0, 0, -1,
	-1, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, -1
};

const int anMaze3[999] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, 0, 0, -1, 1, 1, 1, 1, 1, 1, 1, 4, 1, 1, 1, 1, -1, -1, 0, -1, -1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, -1, 0, 0, -1,
	-1, 0, 0, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 0, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, 0, 0, -1,
	-1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 0, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1,
	-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, -1, -1, 0, -1, -1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1,
	-1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, 0, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1,
	0, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, 0, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, 0,
	-1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, -1, -1, 1, 1, 1, 1, 1, -1, -1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, -1,
	-1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1,
	-1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1,
	-1, 1, -1, -1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, -1, -1, 1, -1,
	-1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1,
	-1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1,
	-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1,
	-1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1,
	-1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1,
	-1, 1, -1, -1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, -1, -1, 1, -1,
	-1, 1, -1, -1, 3, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 5, -1, -1, 1, -1,
	-1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1,
	-1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, -1, -1, 1, 1, 1, 1, 1, -1, -1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, -1,
	0, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, 0, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, 0,
	-1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, 0, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1,
	-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, -1, -1, 0, -1, -1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1,
	-1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 0, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1,
	-1, 0, 0, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 0, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, 0, 0, -1,
	-1, 0, 0, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 0, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, 0, 0, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

const int anMaze2[999] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, 0, 0, -1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, -1, 0, 0, -1,
	-1, 0, 0, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, 0, 0, -1,
	-1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1,
	-1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, -1,
	-1, 1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1,
	-1, 1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1,
	-1, 1, -1, -1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, -1, -1, 1, -1,
	-1, 1, -1, -1, 2, -1, -1, 1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 4, -1, -1, 1, -1,
	-1, 1, 1, 1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, 1, 1, 1, -1,
	-1, 1, -1, -1, -1, -1, -1, 1, -1, -1, -1, 1, -1, -1, -1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, 1, -1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1,
	-1, 1, -1, -1, -1, -1, -1, 1, -1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1,
	-1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, -1,
	-1, 1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1,
	-1, 1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1,
	-1, 1, -1, -1, 1, -1, -1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, -1, -1, 1, -1, -1, 1, -1,
	-1, 1, -1, -1, 1, 1, 1, 1, -1, -1, 3, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, 5, -1, -1, 1, 1, 1, 1, -1, -1, 1, -1,
	-1, 1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1,
	-1, 1, -1, -1, -1, -1, -1, 1, -1, -1, 1, 1, 1, 1, 1, 1, -1, -1, 1, -1, -1, 1, 1, 1, 1, 1, 1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1,
	-1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, 1, 1, 1, 1, 1, 1, 1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	-1, -1, -1, -1, 1, -1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, -1, 1, -1, -1, -1, -1,
	-1, 0, 0, -1, 1, -1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, -1, 1, -1, 0, 0, -1,
	-1, 0, 0, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, 0, 0, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

const int anMaze1[999] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, 0, 0, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, 0, 0, -1,
	-1, 0, 0, -1, 1, -1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, -1, 1, -1, 0, 0, -1,
	-1, -1, -1, -1, 1, -1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, -1, 1, -1, -1, -1, -1,
	-1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, -1, -1, 1, -1, -1, 1, 1, 1, 1, 1, -1, -1, 1, -1, -1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, -1,
	-1, 1, -1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, -1, 1, -1,
	-1, 1, -1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, -1, 1, -1,
	-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, 5, -1, -1, -1, 3, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, -1, -1, 1, 1, 1, 1, 1, -1, -1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1,
	-1, 1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1,
	-1, 1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1,
	0, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 0,
	-1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1,
	-1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1,
	-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1,
	-1, 1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1,
	-1, 1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1,
	-1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, -1,
	-1, 1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1,
	-1, 1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1,
	-1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 1, -1, -1, 1, 1, 1, 1, -1, -1, -1, 1, 1, 1, 1, -1, -1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, -1,
	-1, -1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1,
	-1, 0, 0, -1, 1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, 1, -1, 0, 0, -1,
	-1, 0, 0, -1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, -1, 0, 0, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

int     nExtraLives;
bool    bInLoop;
bool    bBALLOONShown;

/*****************************************************************
 *
 * CMainPackRatWindow
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Create the window with the appropriate style, size, menu, etc.;
 * it will be later revealed by CTheApp::InitInstance().  Then
 * create our splash screen object by opening and loading its DIB.
 *
 * FORMAL PARAMETERS:
 *
 *  lUserAmount = initial amount of money that user starts with
 *                              defaults to zero
 *  nRounds         = the number of rounds to play, if 0 then not playing rounds
 *                          = defaults to zero
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/

CMainPackRatWindow::CMainPackRatWindow(HWND hCallingWnd, LPGAMESTRUCT lpGameStruct) :
		ptBaloon(609, 25),
		ptLastMouseCoord(-1, -1),
		ptCurrMouseCoord(-1, -1),
		rNewGame(21, 3, 227, 20),
		ptScore(MainRect.left + 10, MainRect.top + 10,
			MainRect.left + 110, MainRect.top + 25) {
	CDC     *pDC = nullptr;                        // device context for the screen
	CString WndClass;
	CSize   mySize;
	bool    bTestCreate;                        // bool for testing the creation of each button
	bool    bTestBmp;                       // bool for testing the creation of the splash screen
	int     nLoop1;
	JOYINFO joyInfo;

	CBitmap *pTempBMP = nullptr;
// Define a special window class which traps double-clicks, is byte aligned
// to maximize BITBLT performance, and creates "owned" DCs rather than sharing
// the five system defined DCs which are not guaranteed to be available;
// this adds a bit to our app size but avoids hangs/freezes/lockups.

	BeginWaitCursor();
	initStatics();

	WndClass = AfxRegisterWndClass(CS_DBLCLKS | CS_BYTEALIGNWINDOW | CS_OWNDC,
	                               nullptr, nullptr, nullptr);

// set the seed for the random number generator
//srand( (unsigned)time( nullptr ));

// initialize private members
	m_lpGameStruct = lpGameStruct;

	m_hCallAppWnd = hCallingWnd;

// load splash screen
	pDC = GetDC();                                  // get a device context for our window

	/*
	pDibDoc = new CDibDoc();                                                // create an object to hold our splash screen
	ASSERT(pDibDoc);                                                                // ... and verify we got it
	bTestBmp = pDibDoc->OpenDocument(MAZE1);        // next load in the actual DIB based artwork
	ASSERT( bTestBmp );
	pGamePalette = (*pDibDoc).DetachPalette();      // grab its palette and save it for later use
	delete pDibDoc;                                 // now discard the splash screen
	*/
// set window coordinates to center game on screeen
	MainRect.left = (pDC->GetDeviceCaps(HORZRES) - GAME_WIDTH) >> 1;
	MainRect.top = (pDC->GetDeviceCaps(VERTRES) - GAME_HEIGHT) >> 1;
	MainRect.right = MainRect.left + GAME_WIDTH;    // determine where to place the game window
	MainRect.bottom = MainRect.top + GAME_HEIGHT;   // ... so it is centered on the screen

	ReleaseDC(pDC);
	pDC = nullptr;
// Create the window as a POPUP so that no boarders, title, or menu are present;
// this is because the game's background art will fill the entire 640x40 area.
	Create(WndClass, "Boffo Games -- Packrat", WS_POPUP, MainRect, nullptr, 0);

	pDC = GetDC();
	pTempBMP = FetchBitmap(pDC, &pGamePalette, MAZE1);

	bTestBmp = CSprite::SetBackdrop(pDC, pGamePalette, pTempBMP);
	ASSERT(bTestBmp);
	pTempBMP = nullptr;

//ReleaseDC( pDC );
//pDC = nullptr;

	ShowWindow(SW_SHOWNORMAL);
//UpdateWindow();

//pDC = GetDC();
//pOldPalette = pDC->SelectPalette( pGamePalette, false );
//pDC->RealizePalette();

	pSSpaceBMP = FetchResourceBitmap(pDC, nullptr, IDB_SEMPTY);
	pLSpaceBMP = FetchResourceBitmap(pDC, nullptr, IDB_LEMPTY);

	CSprite::RefreshBackdrop(pDC, pGamePalette);

// create buttons

	if (joySetCapture(m_hWnd, JOYSTICKID1, 5000, true) == JOYERR_NOERROR) {
		bUseJoyStick = true;
		joySetThreshold(JOYSTICKID1, 2000);
		joyGetPos(JOYSTICKID1, &joyInfo);
		awJoyCenter[0] = (unsigned int)joyInfo.wXpos;
		awJoyCenter[1] = (unsigned int)joyInfo.wYpos;
	} else {
		bUseJoyStick = false;
	}

	if (m_lpGameStruct->bPlayingMetagame) {

		pBalloonSprite  = new CSprite;
		pBalloonSprite->SharePalette(pGamePalette);
		bTestCreate = pBalloonSprite->LoadCels(pDC, BALLOONSPRITE, 1);
		//bTestCreate = pLiveSprite->LoadResourceCels( pDC, IDB_LIVE, 1 );
		pBalloonSprite->SetMasked(true);
		pBalloonSprite->SetMobile(false);
		pBalloonSprite->SetOptimizeSpeed(false);
		ASSERT(bTestCreate != 0);                // test for sprite's creation
		bBALLOONShown = false;
	}

	pOptionButton = new CBmpButton;         // create the Options button
	ASSERT(pOptionButton);
	OptionRect.SetRect(OPTION_LEFT,
	                   OPTION_TOP,
	                   OPTION_LEFT + OPTION_WIDTH,
	                   OPTION_TOP + OPTION_HEIGHT);
	bTestCreate = pOptionButton->Create("Options", BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, OptionRect, this, IDC_OPTION);
	ASSERT(bTestCreate != 0);                // test for button's creation
	bTestCreate = pOptionButton->LoadBitmaps(IDB_OPTIONU, IDB_OPTIONU, IDB_OPTIONU, IDB_OPTIONG);
//bTestCreate = pOptionButton->LoadBitmaps( OPTIONU, nullptr, nullptr, OPTIONG );
	ASSERT(bTestCreate != 0);                // test for button's creation

	if (m_lpGameStruct->bPlayingHodj) {
		pPlayerRightSprite = new CSprite;
		pPlayerRightSprite->SharePalette(pGamePalette);
		//bTestCreate = pPlayerRightSprite->LoadResourceCels( pDC, IDB_HODJR, 8 );
		bTestCreate = pPlayerRightSprite->LoadCels(pDC, HODJRIGHTSPRITE, 8);
		ASSERT(bTestCreate);                // test for sprite's creation
		pPlayerRightSprite->SetMasked(true);
		pPlayerRightSprite->SetMobile(true);
		pPlayerRightSprite->SetOptimizeSpeed(true);

		pPlayerLeftSprite = new CSprite;
		pPlayerLeftSprite->SharePalette(pGamePalette);
		//bTestCreate = pPlayerLeftSprite->LoadResourceCels( pDC, IDB_HODJL, 8 );
		bTestCreate = pPlayerLeftSprite->LoadCels(pDC, HODJLEFTSPRITE, 8);
		ASSERT(bTestCreate);                // test for sprite's creation
		pPlayerLeftSprite->SetMasked(true);
		pPlayerLeftSprite->SetMobile(true);
		pPlayerLeftSprite->SetOptimizeSpeed(true);

		pPlayerUpSprite = new CSprite;
		pPlayerUpSprite->SharePalette(pGamePalette);
		//bTestCreate = pPlayerUpSprite->LoadResourceCels( pDC, IDB_HODJU, 8 );
		bTestCreate = pPlayerUpSprite->LoadCels(pDC, HODJUPSPRITE, 8);
		ASSERT(bTestCreate);                // test for sprite's creation
		pPlayerUpSprite->SetMasked(true);
		pPlayerUpSprite->SetMobile(true);
		pPlayerUpSprite->SetOptimizeSpeed(true);

		pPlayerDownSprite = new CSprite;
		pPlayerDownSprite->SharePalette(pGamePalette);
		//bTestCreate = pPlayerDownSprite->LoadResourceCels( pDC, IDB_HODJD, 8 );
		bTestCreate = pPlayerDownSprite->LoadCels(pDC, HODJDOWNSPRITE, 8);
		ASSERT(bTestCreate);                // test for sprite's creation
		pPlayerDownSprite->SetMasked(true);
		pPlayerDownSprite->SetMobile(true);
		pPlayerDownSprite->SetOptimizeSpeed(true);
	} else {
		pPlayerRightSprite = new CSprite;
		pPlayerRightSprite->SharePalette(pGamePalette);
		//bTestCreate = pPlayerRightSprite->LoadResourceCels( pDC, IDB_PODJR, 8 );
		bTestCreate = pPlayerRightSprite->LoadCels(pDC, PODJRIGHTSPRITE, 8);
		ASSERT(bTestCreate);                // test for sprite's creation
		pPlayerRightSprite->SetMasked(true);
		pPlayerRightSprite->SetMobile(true);
		pPlayerRightSprite->SetOptimizeSpeed(true);

		pPlayerLeftSprite = new CSprite;
		pPlayerLeftSprite->SharePalette(pGamePalette);
		//bTestCreate = pPlayerLeftSprite->LoadResourceCels( pDC, IDB_PODJL, 8 );
		bTestCreate = pPlayerLeftSprite->LoadCels(pDC, PODJLEFTSPRITE, 8);
		ASSERT(bTestCreate);                // test for sprite's creation
		pPlayerLeftSprite->SetMasked(true);
		pPlayerLeftSprite->SetMobile(true);
		pPlayerLeftSprite->SetOptimizeSpeed(true);

		pPlayerUpSprite = new CSprite;
		pPlayerUpSprite->SharePalette(pGamePalette);
		//bTestCreate = pPlayerUpSprite->LoadResourceCels( pDC, IDB_PODJU, 8 );
		bTestCreate = pPlayerUpSprite->LoadCels(pDC, PODJUPSPRITE, 8);
		ASSERT(bTestCreate);                // test for sprite's creation
		pPlayerUpSprite->SetMasked(true);
		pPlayerUpSprite->SetMobile(true);
		pPlayerUpSprite->SetOptimizeSpeed(true);

		pPlayerDownSprite = new CSprite;
		pPlayerDownSprite->SharePalette(pGamePalette);
		//bTestCreate = pPlayerDownSprite->LoadResourceCels( pDC, IDB_PODJD, 8 );
		bTestCreate = pPlayerDownSprite->LoadCels(pDC, PODJDOWNSPRITE, 8);
		ASSERT(bTestCreate);                // test for sprite's creation
		pPlayerDownSprite->SetMasked(true);
		pPlayerDownSprite->SetMobile(true);
		pPlayerDownSprite->SetOptimizeSpeed(true);
	}

	pPlayerSprite = pPlayerUpSprite->DuplicateSprite(pDC);
	pPlayerSprite->LinkSprite();

	m_ptCurrentPPos.x = 0;
	m_ptCurrentPPos.y = 0;

	pBadGuyUpSprite  = new CSprite;
	pBadGuyUpSprite->SharePalette(pGamePalette);
	bTestCreate = pBadGuyUpSprite->LoadCels(pDC, BADGUYSPRITEU, 7);
//bTestCreate = pBadGuyUpSprite->LoadResourceCels( pDC, IDB_TURTLEBU, 7 );
	pBadGuyUpSprite->SetMasked(true);
	pBadGuyUpSprite->SetMobile(true);
	pBadGuyUpSprite->SetOptimizeSpeed(true);
	ASSERT(bTestCreate != 0);                // test for sprite's creation

	pBadGuyDownSprite  = new CSprite;
	pBadGuyDownSprite->SharePalette(pGamePalette);
	bTestCreate = pBadGuyDownSprite->LoadCels(pDC, BADGUYSPRITED, 7);
//bTestCreate = pBadGuyDownSprite->LoadResourceCels( pDC, IDB_TURTLEBD, 7 );
	pBadGuyDownSprite->SetMasked(true);
	pBadGuyDownSprite->SetMobile(true);
	pBadGuyDownSprite->SetOptimizeSpeed(true);
	ASSERT(bTestCreate != 0);                // test for sprite's creation

	pBadGuyLeftSprite  = new CSprite;
	pBadGuyLeftSprite->SharePalette(pGamePalette);
	bTestCreate = pBadGuyLeftSprite->LoadCels(pDC, BADGUYSPRITEL, 7);
//bTestCreate = pBadGuyLeftSprite->LoadResourceCels( pDC, IDB_TURTLEBL, 7 );
	pBadGuyLeftSprite->SetMasked(true);
	pBadGuyLeftSprite->SetMobile(true);
	pBadGuyLeftSprite->SetOptimizeSpeed(true);
	ASSERT(bTestCreate != 0);                // test for sprite's creation

	pBadGuyRightSprite  = new CSprite;
	pBadGuyRightSprite->SharePalette(pGamePalette);
	bTestCreate = pBadGuyRightSprite->LoadCels(pDC, BADGUYSPRITER, 7);
//bTestCreate = pBadGuyRightSprite->LoadResourceCels( pDC, IDB_TURTLEBR, 7 );
	pBadGuyRightSprite->SetMasked(true);
	pBadGuyRightSprite->SetMobile(true);
	pBadGuyRightSprite->SetOptimizeSpeed(true);
	ASSERT(bTestCreate != 0);                // test for sprite's creation

	pGoodGuyUpSprite  = new CSprite;
	pGoodGuyUpSprite->SharePalette(pGamePalette);
	bTestCreate = pGoodGuyUpSprite->LoadCels(pDC, GOODGUYSPRITEU, 7);
//bTestCreate = pGoodGuyUpSprite->LoadResourceCels( pDC, IDB_TURTLEGU, 7 );
	pGoodGuyUpSprite->SetMasked(true);
	pGoodGuyUpSprite->SetMobile(true);
	pGoodGuyUpSprite->SetOptimizeSpeed(true);
	ASSERT(bTestCreate != 0);                // test for sprite's creation

	pGoodGuyDownSprite  = new CSprite;
	pGoodGuyDownSprite->SharePalette(pGamePalette);
	bTestCreate = pGoodGuyDownSprite->LoadCels(pDC, GOODGUYSPRITED, 7);
//bTestCreate = pGoodGuyDownSprite->LoadResourceCels( pDC, IDB_TURTLEGD, 7 );
	pGoodGuyDownSprite->SetMasked(true);
	pGoodGuyDownSprite->SetMobile(true);
	pGoodGuyDownSprite->SetOptimizeSpeed(true);
	ASSERT(bTestCreate != 0);                // test for sprite's creation

	pGoodGuyLeftSprite  = new CSprite;
	pGoodGuyLeftSprite->SharePalette(pGamePalette);
	bTestCreate = pGoodGuyLeftSprite->LoadCels(pDC, GOODGUYSPRITEL, 7);
//bTestCreate = pGoodGuyLeftSprite->LoadResourceCels( pDC, IDB_TURTLEGL, 7 );
	pGoodGuyLeftSprite->SetMasked(true);
	pGoodGuyLeftSprite->SetMobile(true);
	pGoodGuyLeftSprite->SetOptimizeSpeed(true);
	ASSERT(bTestCreate != 0);                // test for sprite's creation

	pGoodGuyRightSprite  = new CSprite;
	pGoodGuyRightSprite->SharePalette(pGamePalette);
	bTestCreate = pGoodGuyRightSprite->LoadCels(pDC, GOODGUYSPRITER, 7);
//bTestCreate = pGoodGuyRightSprite->LoadResourceCels( pDC, IDB_TURTLEGR, 7 );
	pGoodGuyRightSprite->SetMasked(true);
	pGoodGuyRightSprite->SetMobile(true);
	pGoodGuyRightSprite->SetOptimizeSpeed(true);
	ASSERT(bTestCreate != 0);                // test for sprite's creation

	pFlashUpSprite  = new CSprite;
	pFlashUpSprite->SharePalette(pGamePalette);
	bTestCreate = pFlashUpSprite->LoadCels(pDC, FLASHSPRITEU, 7);
//bTestCreate = pFlashUpSprite->LoadResourceCels( pDC, IDB_TURTLEFU, 7 );
	pFlashUpSprite->SetMasked(true);
	pFlashUpSprite->SetMobile(true);
	pFlashUpSprite->SetOptimizeSpeed(true);
	ASSERT(bTestCreate != 0);                // test for sprite's creation

	pFlashDownSprite  = new CSprite;
	pFlashDownSprite->SharePalette(pGamePalette);
	bTestCreate = pFlashDownSprite->LoadCels(pDC, FLASHSPRITED, 7);
//bTestCreate = pFlashDownSprite->LoadResourceCels( pDC, IDB_TURTLEFD, 7 );
	pFlashDownSprite->SetMasked(true);
	pFlashDownSprite->SetMobile(true);
	pFlashDownSprite->SetOptimizeSpeed(true);
	ASSERT(bTestCreate != 0);                // test for sprite's creation

	pFlashLeftSprite  = new CSprite;
	pFlashLeftSprite->SharePalette(pGamePalette);
	bTestCreate = pFlashLeftSprite->LoadCels(pDC, FLASHSPRITEL, 7);
//bTestCreate = pFlashLeftSprite->LoadResourceCels( pDC, IDB_TURTLEFL, 7 );
	pFlashLeftSprite->SetMasked(true);
	pFlashLeftSprite->SetMobile(true);
	pFlashLeftSprite->SetOptimizeSpeed(true);
	ASSERT(bTestCreate != 0);                // test for sprite's creation

	pFlashRightSprite  = new CSprite;
	pFlashRightSprite->SharePalette(pGamePalette);
	bTestCreate = pFlashRightSprite->LoadCels(pDC, FLASHSPRITER, 7);
//bTestCreate = pFlashRightSprite->LoadResourceCels( pDC, IDB_TURTLEFR, 7 );
	pFlashRightSprite->SetMasked(true);
	pFlashRightSprite->SetMobile(true);
	pFlashRightSprite->SetOptimizeSpeed(true);
	ASSERT(bTestCreate != 0);                // test for sprite's creation

	for (nLoop1 = 0; nLoop1 < 4; nLoop1++) {
		apBadGuySprite[nLoop1] = nullptr;
		apBadGuySprite[nLoop1] = new CSprite;
		apBadGuySprite[nLoop1] = pBadGuyUpSprite->DuplicateSprite(pDC);
		apBadGuySprite[nLoop1]->LinkSprite();
	}

	pLiveSprite  = new CSprite;
	pLiveSprite->SharePalette(pGamePalette);
	bTestCreate = pLiveSprite->LoadCels(pDC, LIVESPRITE, 1);
//bTestCreate = pLiveSprite->LoadResourceCels( pDC, IDB_LIVE, 1 );
	pLiveSprite->SetMasked(true);
	pLiveSprite->SetMobile(true);
	pLiveSprite->SetOptimizeSpeed(false);
	ASSERT(bTestCreate != 0);                // test for sprite's creation

	for (nLoop1 = 0; nLoop1 < MAXLIVESPRITE; nLoop1++) {
		pLivesSprite[nLoop1] = new CSprite;
		pLivesSprite[nLoop1] = pLiveSprite->DuplicateSprite(pDC);
	}

	ReleaseDC(pDC);                                   // release our window context

	m_ptCurrPLocInGrid.x = 0;
	m_ptCurrPLocInGrid.y = 0;
	m_aptCurrBLocInGrid[0].x = 0;
	m_aptCurrBLocInGrid[0].y = 0;
	m_aptCurrBLocInGrid[1].x = 0;
	m_aptCurrBLocInGrid[1].y = 0;
	m_aptCurrBLocInGrid[2].x = 0;
	m_aptCurrBLocInGrid[2].y = 0;
	m_aptCurrBLocInGrid[3].x = 0;
	m_aptCurrBLocInGrid[3].y = 0;

	m_bMouseCaptured = false;
	m_nPDirection = 0;
	m_nNextDir = m_nPDirection;
	m_nBDirection = FOLLOWRIGHTWALL;
	m_anBDirection[0] = 1;
	m_anBDirection[1] = 3;
	m_anBDirection[2] = 2;
	m_anBDirection[3] = 4;
	m_nNumberOfMoves = 0;
	m_lScore = 0;
	m_bSuspend = true;

	bEatTurtle = false;
	nEatTurtle = 0;
	bFirstTimer = true;
	bChangeTurtle = false;
	bFlashTurtle = false;
	nOldTSpeed = 0;

	nPaintObject = 0;
	ptNOLocInGrid.x = 0;
	ptNOLocInGrid.y = 0;

	nNumOfBGKilled = 0;

	bEndGame = false;


	if (m_lpGameStruct->bPlayingMetagame) {

		nStartingLives = 1;
		switch (m_lpGameStruct->nSkillLevel) {
		case SKILLLEVEL_LOW:
			m_nGameLevel = 1;
			break;
		case SKILLLEVEL_MEDIUM:
			m_nGameLevel = 6;
			break;
		case SKILLLEVEL_HIGH:
			m_nGameLevel = 12;
			break;
		}
	} else {
		m_nGameLevel = 1;
		nStartingLives = 3;
	}

	bResetGame = false;

	ResetGame();

	anBadGuyMove[0] = NUMBEROFBADGUYCELLS + 1;
	anBadGuyMove[1] = NUMBEROFBADGUYCELLS + 1;
	anBadGuyMove[2] = NUMBEROFBADGUYCELLS + 1;
	anBadGuyMove[3] = NUMBEROFBADGUYCELLS + 1;
	nPlayerMove = NUMBEROFPLAYERCELLS;

	if (m_lpGameStruct->bMusicEnabled) {
		pGameSound = new CSound(this, GAME_THEME,
		                        SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
		if (pGameSound != nullptr)
			(*pGameSound).midiLoopPlaySegment(2310, 29400, 0, FMT_MILLISEC);
	} // end if pGameSound

	if (m_lpGameStruct->bPlayingMetagame) {
		m_bSuspend = false;
		bFirstTimer = false;
		SetTimer(PACKRATTIMER, 50, nullptr);
	}

	bStart = false;
	bInLoop = false;
	EndWaitCursor();
	(*this).SetFocus();
}

void CMainPackRatWindow::initStatics() {
	pGamePalette = nullptr;
	pOptionButton = nullptr;
	pPlayerRightSprite = nullptr;
	pPlayerLeftSprite = nullptr;
	pPlayerUpSprite = nullptr;
	pPlayerDownSprite = nullptr;

	pBadGuyUpSprite = nullptr;
	pBadGuyDownSprite = nullptr;
	pBadGuyLeftSprite = nullptr;
	pBadGuyRightSprite = nullptr;

	pGoodGuyUpSprite = nullptr;
	pGoodGuyDownSprite = nullptr;
	pGoodGuyLeftSprite = nullptr;
	pGoodGuyRightSprite = nullptr;

	pFlashUpSprite = nullptr;
	pFlashDownSprite = nullptr;
	pFlashLeftSprite = nullptr;
	pFlashRightSprite = nullptr;

	pLiveSprite = nullptr;
	Common::fill(pLivesSprite, pLivesSprite + MAXLIVESPRITE, nullptr);

	pBalloonSprite = nullptr;

	pPlayerSprite = nullptr;
	Common::fill(apBadGuySprite, apBadGuySprite + 4, nullptr);

	pSSpaceBMP = nullptr;
	pLSpaceBMP = nullptr;
	pGameSound = nullptr;                             // Game theme song

	ptNOLocInGrid.x = ptNOLocInGrid.y = 0;
	nPaintObject = 0;

	nPlayerMove = 0;
	Common::fill(anBadGuyMove, anBadGuyMove + 4, 0);

	bFirstTimer = false;
	bEatTurtle = false;
	Common::fill(abChangeTurtle, abChangeTurtle + 4, false);
	nEatTurtle = 0;
	bChangeTurtle = false;
	nOldTSpeed = 0;
	Common::fill(abGoodTurtle, abGoodTurtle + 4, false);
	Common::fill(anSuperObj, anSuperObj + 4, false);

	bEndGame = false;
	bFlashTurtle = false;
	Common::fill(anReleaseTurtleCount, anReleaseTurtleCount + 4, 0);

	bUseJoyStick = false;
	awJoyCenter[0] = awJoyCenter[1] = 0;

	nNumOfBGKilled = 0;
	nStartingLives = 0;

	nKillBadGuy = 0;
	bKillPlayer = false;
	bResetGame = false;
	bMouseHidden = false;
	bInNewGameRect = false;
	bStart = false;
}

/*****************************************************************
 *
 * SetMaze
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *
 * FORMAL PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/
void CMainPackRatWindow::SetMaze() {
	CDC     *pDC = GetDC();
	CPalette    *pOldPal = nullptr;
	int     nLoop1;
	CBitmap *pTempBmp = nullptr;
	bool    bTestBmp;

	SetLevel();

//    if ( pGamePalette != nullptr ) {
//      delete pGamePalette;
//      pGamePalette = nullptr;
//  }

	pOldPal = pDC->SelectPalette(pGamePalette, false);
	pDC->RealizePalette();

	switch (m_nMaze) {
	case 2:
		pTempBmp = FetchBitmap(pDC, nullptr, MAZE2);

		#ifndef DYNAMIC_OBJ_COUNT
		m_nNumberOfObjectsLeft = 0;
		#endif
		for (nLoop1 = 0; nLoop1 < 999; nLoop1++) {
			anCurrentMaze[nLoop1] = anMaze2[nLoop1];
		}
		#ifndef DYNAMIC_OBJ_COUNT
		m_nNumberOfObjectsLeft = 365;
		#endif
		break;
	case 3:
		pTempBmp = FetchBitmap(pDC, nullptr, MAZE3);

		#ifndef DYNAMIC_OBJ_COUNT
		m_nNumberOfObjectsLeft = 0;
		#endif
		for (nLoop1 = 0; nLoop1 < 999; nLoop1++) {
			anCurrentMaze[nLoop1] = anMaze3[nLoop1];
		}
		#ifndef DYNAMIC_OBJ_COUNT
		m_nNumberOfObjectsLeft = 342;
		#endif
		break;
	case 4:
		pTempBmp = FetchBitmap(pDC, nullptr, MAZE4);

		#ifndef DYNAMIC_OBJ_COUNT
		m_nNumberOfObjectsLeft = 0;
		#endif
		for (nLoop1 = 0; nLoop1 < 999; nLoop1++) {
			anCurrentMaze[nLoop1] = anMaze4[nLoop1];
		}
		#ifndef DYNAMIC_OBJ_COUNT
		m_nNumberOfObjectsLeft = 366;
		#endif
		break;
	default:
		pTempBmp = FetchBitmap(pDC, nullptr, MAZE1);

		#ifndef DYNAMIC_OBJ_COUNT
		m_nNumberOfObjectsLeft = 0;
		#endif
		for (nLoop1 = 0; nLoop1 < 999; nLoop1++) {
			anCurrentMaze[nLoop1] = anMaze1[nLoop1];
			#ifndef DYNAMIC_OBJ_COUNT
			m_nNumberOfObjectsLeft = 358;
			#endif
		}
		break;
	}

	bTestBmp = CSprite::SetBackdrop(pDC, pGamePalette, pTempBmp);
	ASSERT(bTestBmp);
	pTempBmp = nullptr;

	m_anMazeArray = (int *) &anCurrentMaze[0];
#ifdef DYNAMIC_OBJ_COUNT
	m_nNumberOfObjectsLeft = 0;
	for (int nLoop = 0; nLoop < 999; nLoop++) {
		if (m_anMazeArray[nLoop] > 0) {
			m_nNumberOfObjectsLeft++;
		}
	}
#endif

	anSuperObj[0] = false;
	anSuperObj[1] = false;
	anSuperObj[2] = false;
	anSuperObj[3] = false;
	nNumOfBGKilled = 0;
	bChangeTurtle = true;
	bEatTurtle = false;
	nEatTurtle = 0;
	KillPlayer(true);

	pDC->SelectPalette(pOldPal, false);
	ReleaseDC(pDC);
	RedrawWindow();
	m_bSuspend = false;
}

/*****************************************************************
 *
 * SetLevel
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *
 * FORMAL PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/
void CMainPackRatWindow::SetLevel() {
	switch (m_nGameLevel) {
	case 1:
		m_nBadGuySpeed = 9;
		m_nPlayerSpeed = 8;
		m_nMaze = 1;
		break;
	case 2:
		m_nBadGuySpeed = 9;
		m_nPlayerSpeed = 8;
		m_nMaze = 2;
		break;
	case 3:
		m_nBadGuySpeed = 9;
		m_nPlayerSpeed = 8;
		m_nMaze = 3;
		break;
	case 4:
		m_nBadGuySpeed = 9;
		m_nPlayerSpeed = 8;
		m_nMaze = 4;
		break;
	case 5:
		m_nBadGuySpeed = 8;
		m_nPlayerSpeed = 7;
		m_nMaze = 1;
		break;
	case 6:
		m_nBadGuySpeed = 8;
		m_nPlayerSpeed = 7;
		m_nMaze = 2;
		break;
	case 7:
		m_nBadGuySpeed = 8;
		m_nPlayerSpeed = 7;
		m_nMaze = 3;
		break;
	case 8:
		m_nBadGuySpeed = 8;
		m_nPlayerSpeed = 7;
		m_nMaze = 4;
		break;
	case 9:
		m_nBadGuySpeed = 7;
		m_nPlayerSpeed = 6;
		m_nMaze = 1;
		break;
	case 10:
		m_nBadGuySpeed = 7;
		m_nPlayerSpeed = 6;
		m_nMaze = 2;
		break;
	case 11:
		m_nBadGuySpeed = 7;
		m_nPlayerSpeed = 6;
		m_nMaze = 3;
		break;
	case 12:
		m_nBadGuySpeed = 7;
		m_nPlayerSpeed = 6;
		m_nMaze = 4;
		break;
	case 13:
		m_nBadGuySpeed = 6;
		m_nPlayerSpeed = 5;
		m_nMaze = 1;
		break;
	case 14:
		m_nBadGuySpeed = 6;
		m_nPlayerSpeed = 5;
		m_nMaze = 2;
		break;
	case 15:
		m_nBadGuySpeed = 6;
		m_nPlayerSpeed = 5;
		m_nMaze = 3;
		break;
	case 16:
		m_nBadGuySpeed = 6;
		m_nPlayerSpeed = 5;
		m_nMaze = 4;
		break;
	case 17:
		m_nBadGuySpeed = 5;
		m_nPlayerSpeed = 4;
		m_nMaze = 1;
		break;
	case 18:
		m_nBadGuySpeed = 5;
		m_nPlayerSpeed = 4;
		m_nMaze = 2;
		break;
	case 19:
		m_nBadGuySpeed = 5;
		m_nPlayerSpeed = 4;
		m_nMaze = 3;
		break;
	case 20:
		m_nBadGuySpeed = 5;
		m_nPlayerSpeed = 4;
		m_nMaze = 4;
		break;
	case 21:
		m_nBadGuySpeed = 4;
		m_nPlayerSpeed = 3;
		m_nMaze = 1;
		break;
	case 22:
		m_nBadGuySpeed = 4;
		m_nPlayerSpeed = 3;
		m_nMaze = 2;
		break;
	case 23:
		m_nBadGuySpeed = 4;
		m_nPlayerSpeed = 3;
		m_nMaze = 3;
		break;
	case 24:
		m_nBadGuySpeed = 4;
		m_nPlayerSpeed = 3;
		m_nMaze = 4;
		break;
	case 25:
		m_nBadGuySpeed = 3;
		m_nPlayerSpeed = 2;
		m_nMaze = 1;
		break;
	case 26:
		m_nBadGuySpeed = 3;
		m_nPlayerSpeed = 2;
		m_nMaze = 2;
		break;
	case 27:
		m_nBadGuySpeed = 3;
		m_nPlayerSpeed = 2;
		m_nMaze = 3;
		break;
	case 28:
		m_nBadGuySpeed = 3;
		m_nPlayerSpeed = 2;
		m_nMaze = 4;
		break;
	case 29:
		m_nBadGuySpeed = 2;
		m_nPlayerSpeed = 1;
		m_nMaze = 1;
		break;
	case 30:
		m_nBadGuySpeed = 2;
		m_nPlayerSpeed = 1;
		m_nMaze = 2;
		break;
	case 31:
		m_nBadGuySpeed = 2;
		m_nPlayerSpeed = 1;
		m_nMaze = 3;
		break;
	case 32:
		m_nBadGuySpeed = 2;
		m_nPlayerSpeed = 1;
		m_nMaze = 4;
		break;
	case 33:
		m_nBadGuySpeed = 1;
		m_nPlayerSpeed = 1;
		m_nMaze = 1;
		break;
	case 34:
		m_nBadGuySpeed = 1;
		m_nPlayerSpeed = 1;
		m_nMaze = 2;
		break;
	case 35:
		m_nBadGuySpeed = 1;
		m_nPlayerSpeed = 1;
		m_nMaze = 3;
		break;
	case 36:
		m_nBadGuySpeed = 1;
		m_nPlayerSpeed = 1;
		m_nMaze = 4;
		break;
	}
	return;
}

/*****************************************************************
 *
 * KillBadGuy
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *
 * FORMAL PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/
void CMainPackRatWindow::KillBadGuy(int nWhichBadGuy, bool bFirstTime) {
	CDC         *pDC = GetDC();

	nKillBadGuy = 0;

	m_bSuspend = true;

	if (nWhichBadGuy >= BG4) {
		if (apBadGuySprite[3] != nullptr)
			apBadGuySprite[3]->EraseSprite(pDC);

		m_aptCurrBLocInGrid[3].x = 34;
		m_aptCurrBLocInGrid[3].y = 25;

		anReleaseTurtleCount[3] = 1;
		m_aptCurrentBPos[3].x = (m_aptCurrBLocInGrid[3].x * MOVEBADGUY) + MAZELEFT;
		m_aptCurrentBPos[3].y = (m_aptCurrBLocInGrid[3].y * MOVEBADGUY) + MAZETOP;

		apBadGuySprite[3]->PaintSprite(pDC, m_aptCurrentBPos[3]);
		nWhichBadGuy -= BG4;
		if (bFirstTime == false) {
			nNumOfBGKilled++;
			m_lScore += ((Power(2, (nNumOfBGKilled - 1)) * 25) * m_nGameLevel);
		}
	}

	if (nWhichBadGuy >= BG3) {
		if (apBadGuySprite[2] != nullptr)
			apBadGuySprite[2]->EraseSprite(pDC);

		m_aptCurrBLocInGrid[2].x = 2;
		m_aptCurrBLocInGrid[2].y = 25;

		anReleaseTurtleCount[2] = 1;
		m_aptCurrentBPos[2].x = (m_aptCurrBLocInGrid[2].x * MOVEBADGUY) + MAZELEFT;
		m_aptCurrentBPos[2].y = (m_aptCurrBLocInGrid[2].y * MOVEBADGUY) + MAZETOP;

		apBadGuySprite[2]->PaintSprite(pDC, m_aptCurrentBPos[2]);
		nWhichBadGuy -= BG3;
		if (bFirstTime == false) {
			nNumOfBGKilled++;
			m_lScore += ((Power(2, (nNumOfBGKilled - 1)) * 25) * m_nGameLevel);
		}
	}

	if (nWhichBadGuy >= BG2) {
		if (apBadGuySprite[1] != nullptr)
			apBadGuySprite[1]->EraseSprite(pDC);

		m_aptCurrBLocInGrid[1].x = 34;
		m_aptCurrBLocInGrid[1].y = 2;

		anReleaseTurtleCount[1] = 1;
		m_aptCurrentBPos[1].x = (m_aptCurrBLocInGrid[1].x * MOVEBADGUY) + MAZELEFT;
		m_aptCurrentBPos[1].y = (m_aptCurrBLocInGrid[1].y * MOVEBADGUY) + MAZETOP;

		apBadGuySprite[1]->PaintSprite(pDC, m_aptCurrentBPos[1]);
		nWhichBadGuy -= BG2;
		if (bFirstTime == false) {
			nNumOfBGKilled++;
			m_lScore += ((Power(2, (nNumOfBGKilled - 1)) * 25) * m_nGameLevel);
		}
	}

	if (nWhichBadGuy >= BG1) {
		if (apBadGuySprite[0] != nullptr)
			apBadGuySprite[0]->EraseSprite(pDC);

		m_aptCurrBLocInGrid[0].x = 2;
		m_aptCurrBLocInGrid[0].y = 2;

		anReleaseTurtleCount[0] = 1;
		m_aptCurrentBPos[0].x = (m_aptCurrBLocInGrid[0].x * MOVEBADGUY) + MAZELEFT;
		m_aptCurrentBPos[0].y = (m_aptCurrBLocInGrid[0].y * MOVEBADGUY) + MAZETOP;

		apBadGuySprite[0]->PaintSprite(pDC, m_aptCurrentBPos[0]);
		nWhichBadGuy -= BG1;
		if (bFirstTime == false) {
			nNumOfBGKilled++;
			m_lScore += ((Power(2, (nNumOfBGKilled - 1)) * 25) * m_nGameLevel);
		}
	}

	ReleaseDC(pDC);
	m_bSuspend = false;
	return;
}

/*****************************************************************
 *
 * KillPlayer
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *
 * FORMAL PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/
void CMainPackRatWindow::KillPlayer(bool bFirstTime) {
	CDC         *pDC = GetDC();
	CMsgDlg     cMessageBoxDlg((CWnd *)this, pGamePalette);

	bKillPlayer = false;

	m_bSuspend = true;

	pPlayerSprite->EraseSprite(pDC);

	KillBadGuy((BG1 + BG2 + BG3 + BG4), true);

	m_bSuspend = true;
//  m_anMazeArray[m_ptCurrPLocInGrid.x + ( m_ptCurrPLocInGrid.y * NUMBEROFCOLS )] = 0;
	switch (m_nMaze) {
	case 2:
		m_ptCurrPLocInGrid.x = 18;
		m_ptCurrPLocInGrid.y = 15;
		break;
	case 3:
		m_ptCurrPLocInGrid.x = 18;
		m_ptCurrPLocInGrid.y = 13;
		break;
	case 4:
		m_ptCurrPLocInGrid.x = 18;
		m_ptCurrPLocInGrid.y = 13;
		break;
	case 5:
	default:
		m_ptCurrPLocInGrid.x = 18;
		m_ptCurrPLocInGrid.y = 14;
		break;
	}
	m_ptCurrentPPos.x = (m_ptCurrPLocInGrid.x * MOVEPLAYER) + MAZELEFT;
	m_ptCurrentPPos.y = (m_ptCurrPLocInGrid.y * MOVEPLAYER) + MAZETOP;

	pPlayerSprite->PaintSprite(pDC, m_ptCurrentPPos);

	ReleaseDC(pDC);

	if (bFirstTime == false) {
		m_nLives--;
		pOptionButton->EnableWindow(true);
		if (m_nLives == 0) {
			if (m_lpGameStruct->bSoundEffectsEnabled) {
				sndPlaySound(nullptr, SND_ASYNC);
				sndPlaySound(DEAD_WAV, SND_ASYNC);
			}
			if (m_bMouseCaptured) {
				ReleaseCapture();
				m_bMouseCaptured = false;
				while (MFC::ShowCursor(true) < 0)
					;
			}
			cMessageBoxDlg.SetInitialOptions(4, m_lScore);
			cMessageBoxDlg.DoModal();
			(*this).SetFocus();
			m_nPDirection = 0;
			m_nNextDir = 0;
			m_bSuspend = true;
			if (m_lpGameStruct->bPlayingMetagame) {
				PostMessage(WM_CLOSE, 0, 0);
				bEndGame = true;
			}

			if (m_lpGameStruct->bSoundEffectsEnabled) {
				sndPlaySound(nullptr, SND_ASYNC);
			}

			RedrawWindow();
			return;
		} else {
			if (m_bMouseCaptured) {
				ReleaseCapture();
				m_bMouseCaptured = false;
				while (MFC::ShowCursor(true) < 0)
					;
			}
			cMessageBoxDlg.SetInitialOptions(3, m_lScore, m_nLives);
			cMessageBoxDlg.DoModal();
			if (m_lpGameStruct->bSoundEffectsEnabled) {
				sndPlaySound(nullptr, SND_ASYNC);
			}
			(*this).SetFocus();
		}
	}

	RedrawWindow();
	m_nPDirection = 0;
	m_nNextDir = 0;
	m_bSuspend = false;
	return;
}

/*****************************************************************
 *
 * OnPaint
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Repaint the screen whenever needed; e.g. when uncovered by an
 * overlapping window, when maximized from an icon, and when it the
 * window is initially created.  Ensures that the entire client area
 * of the main screen window is repainted, not just the portion in the
 * update region; see SplashScreen();
 *
 * This routine is called whenever Windows sends a WM_PAINT message.
 * Note that creating a CPaintDC automatically does a BeginPaint and
 * an EndPaint call is done when it is destroyed at the end of this
 * function.  CPaintDC's constructor needs the window (this).
 *
 * FORMAL PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/
void CMainPackRatWindow::OnPaint() {
	PAINTSTRUCT lpPaint;

	BeginPaint(&lpPaint);                           // bracket start of window update
	SplashScreen();                                 // repaint our window's content
	EndPaint(&lpPaint);                             // bracket end of window update
}

/*****************************************************************
 *
 * PaintObjects
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *
 * FORMAL PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/
void CMainPackRatWindow::PaintObjects(int nSize) {
	CDC *pDC = GetDC();
	CPalette  *pOldPal = pDC->SelectPalette(pGamePalette, false);
	int x, y;

	pDC->RealizePalette();

	if (nSize == 1) {
		if (m_nMaze != 1) {
			x = (ptNOLocInGrid.x * MOVEPLAYER) + MAZELEFT + 6;
			y = (ptNOLocInGrid.y * MOVEPLAYER) + MAZETOP + 6;
		} else {
			x = (ptNOLocInGrid.x * MOVEPLAYER) + MAZELEFT + 6;
			y = (ptNOLocInGrid.y * MOVEPLAYER) + MAZETOP + 7;
		}

		PaintMaskedBitmap(pDC, pGamePalette, pSSpaceBMP, x, y, 9, 9);
		PaintMaskedBitmap(CSprite::GetBackdropDC(pDC), pGamePalette, pSSpaceBMP, x, y, 9, 9);
		CSprite::ReleaseBackdropDC();
	} else {
		x = (ptNOLocInGrid.x * MOVEPLAYER) + MAZELEFT + 1;
		y = (ptNOLocInGrid.y * MOVEPLAYER) + MAZETOP + 1;

//      PaintBitmap( pDC, pGamePalette, pLSpaceBMP, x, y, 17, 17 );
//      PaintBitmap( CSprite::GetBackdropDC(), pGamePalette, pLSpaceBMP, x, y, 17, 17 );
		PaintMaskedBitmap(pDC, pGamePalette, pLSpaceBMP, x, y, 19, 19);
		PaintMaskedBitmap(CSprite::GetBackdropDC(pDC), pGamePalette, pLSpaceBMP, x, y, 19, 19);
		CSprite::ReleaseBackdropDC();
	}

	pDC->SelectPalette(pOldPal, false);
	ReleaseDC(pDC);
	nPaintObject = 0;
	ptNOLocInGrid.x = 0;
	ptNOLocInGrid.y = 0;
	return;
}

/*****************************************************************
 *
 * PickUpSuperObject
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *
 *
 * FORMAL PARAMETERS:
 *
 *
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/
void CMainPackRatWindow::PickUpSuperObject() {
	int     y = 0;
	int     x = 0;
	bool    bInSequence = false;

	if (((m_anMazeArray[m_ptCurrPLocInGrid.x + (m_ptCurrPLocInGrid.y * NUMBEROFCOLS)] % 10) >= SUPEROBJ1) &&
	        ((m_anMazeArray[m_ptCurrPLocInGrid.x + (m_ptCurrPLocInGrid.y * NUMBEROFCOLS)] % 10) <= SUPEROBJ4)) {

		y = (m_anMazeArray[m_ptCurrPLocInGrid.x + (m_ptCurrPLocInGrid.y * NUMBEROFCOLS)] % 10);
		m_anMazeArray[m_ptCurrPLocInGrid.x + (m_ptCurrPLocInGrid.y * NUMBEROFCOLS)] = 0;
		bEatTurtle = true;
		nEatTurtle = 1;
		nEatTurtle = 1;
		nOldTSpeed = m_nBadGuySpeed;
		m_nBadGuySpeed *= 2;
		if (m_lpGameStruct->bSoundEffectsEnabled) {
			sndPlaySound(nullptr, SND_ASYNC);
			sndPlaySound(SUPERSOUND, SND_ASYNC);
		}
		bChangeTurtle = true;
		m_nNumberOfObjectsLeft--;
		m_lScore += (5 * m_nGameLevel);
		nPaintObject = 2;
		ptNOLocInGrid.x = m_ptCurrPLocInGrid.x;
		ptNOLocInGrid.y = m_ptCurrPLocInGrid.y;
		bInSequence = true;
		anSuperObj[y - 2] = true;
		for (x = 0; x < (y - 1); x++) {
			if (anSuperObj[x] != true)
				bInSequence = false;
		}
		if (bInSequence) {
			if (y == 5) {
				m_lScore += (500 * m_nGameLevel);
			}
		}
	}
	return;
}

/*****************************************************************
 *
 * PickUpObject
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *
 *
 * FORMAL PARAMETERS:
 *
 *
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/
void CMainPackRatWindow::PickUpObject() {
	if ((m_anMazeArray[m_ptCurrPLocInGrid.x + (m_ptCurrPLocInGrid.y * NUMBEROFCOLS)] % 10)  == OBJECT) {
		m_anMazeArray[m_ptCurrPLocInGrid.x + (m_ptCurrPLocInGrid.y * NUMBEROFCOLS)] = 0;
		m_nNumberOfObjectsLeft--;
		m_lScore += m_nGameLevel;
		nPaintObject = OBJECT;
		ptNOLocInGrid.x = m_ptCurrPLocInGrid.x;
		ptNOLocInGrid.y = m_ptCurrPLocInGrid.y;

	}
	return;
}

/*****************************************************************
 *
 * PlayerKilled
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  checkes whether a player / turtle have been killed
 *
 * FORMAL PARAMETERS:
 *
 *  which turtle to check against
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/

bool CMainPackRatWindow::PlayerKilled(int nWhichBadGuy) {
	bool    bReturn = false;

	if ((m_aptCurrBLocInGrid[nWhichBadGuy].x == m_ptCurrPLocInGrid.x) &&
	        (m_aptCurrBLocInGrid[nWhichBadGuy].y == m_ptCurrPLocInGrid.y)) {

		if (bEatTurtle) {
			if (m_lpGameStruct->bSoundEffectsEnabled) {
				sndPlaySound(nullptr, SND_ASYNC);
				sndPlaySound(BADGUYSOUND, SND_ASYNC);
			}
			nKillBadGuy += Power(2, nWhichBadGuy);
		} else {
			if (m_lpGameStruct->bSoundEffectsEnabled) {
				sndPlaySound(nullptr, SND_ASYNC);
				sndPlaySound(PLAYERSOUND, SND_ASYNC);
			}
			bReturn = true;
		}
	}
	return (bReturn);
}

/*****************************************************************
 *
 * SetNewPlayerPos
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *
 * FORMAL PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/
void CMainPackRatWindow::SetNewPlayerPos() {
	CDC *pDC = GetDC();
	int nWhichBadGuy;
	int nOldDir;
	CSprite     *pTempSprite = nullptr;
	JOYINFO joyInfo;
	CMsgDlg cMessageBoxDlg((CWnd *)this, pGamePalette);


	if (bEndGame) {
		ReleaseDC(pDC);
		return;
	}

	nOldDir = m_nPDirection;

	if ((bUseJoyStick) && (m_nNextDir == 0)) {
		joyGetPos(JOYSTICKID1, &joyInfo);
		if ((unsigned int)(joyInfo.wXpos) > (unsigned int)(awJoyCenter[0] + 10000))
			m_nNextDir = 2;
		else {
			if ((unsigned int)(joyInfo.wXpos) < (unsigned int)(awJoyCenter[0] - 10000))
				m_nNextDir = 1;
			else {
				if ((unsigned int)(joyInfo.wYpos) > (unsigned int)(awJoyCenter[1] + 10000))
					m_nNextDir = 4;
				else {
					if ((unsigned int)(joyInfo.wYpos) < (unsigned int)(awJoyCenter[1] - 10000))
						m_nNextDir = 3;
				}
			}
		}
	}
	switch (m_nNextDir) {
	case 1:
		if (m_anMazeArray[m_ptCurrPLocInGrid.x - 1 + (m_ptCurrPLocInGrid.y * NUMBEROFCOLS)] != WALL) {
			m_nPDirection = m_nNextDir;
			m_nNextDir = 0;
		}
		break;
	case 2:
		if (m_anMazeArray[m_ptCurrPLocInGrid.x + 1 + (m_ptCurrPLocInGrid.y * NUMBEROFCOLS)] != WALL) {
			m_nPDirection = m_nNextDir;
			m_nNextDir = 0;
		}
		break;
	case 3:
		if (m_anMazeArray[m_ptCurrPLocInGrid.x + ((m_ptCurrPLocInGrid.y - 1) * NUMBEROFCOLS)] != WALL) {
			m_nPDirection = m_nNextDir;
			m_nNextDir = 0;
		}
		break;
	case 4:
		if (m_anMazeArray[m_ptCurrPLocInGrid.x + ((m_ptCurrPLocInGrid.y + 1) * NUMBEROFCOLS)] != WALL) {
			m_nPDirection = m_nNextDir;
			m_nNextDir = 0;
		}
		break;
	}
	switch (m_nPDirection) {
	case 1: // move left
		if (m_ptCurrPLocInGrid.x <= 0) {
			m_ptCurrentPPos.x = MAZERIGHT;
//                m_anMazeArray[m_ptCurrPLocInGrid.x + ( m_ptCurrPLocInGrid.y * NUMBEROFCOLS )] = 0;
			m_ptCurrPLocInGrid.x = 36;
		} else {
			if (m_anMazeArray[m_ptCurrPLocInGrid.x - 1 + (m_ptCurrPLocInGrid.y * NUMBEROFCOLS)] != WALL) {

				if (nOldDir != m_nPDirection) {
					pTempSprite = pPlayerLeftSprite->DuplicateSprite(pDC);
				}
				m_ptCurrentPPos.x = (m_ptCurrPLocInGrid.x * MOVEPLAYER) + MAZELEFT;
				m_ptCurrentPPos.y = (m_ptCurrPLocInGrid.y * MOVEPLAYER) + MAZETOP;
				m_ptCurrPLocInGrid.x--;
				PickUpObject();
				PickUpSuperObject();

			} else {
				m_nPDirection = 0;
			}
		}
		break;
	case 2: // move right
		if (m_ptCurrPLocInGrid.x >= 36) {
			m_ptCurrentPPos.x = MAZELEFT;
//                m_anMazeArray[m_ptCurrPLocInGrid.x + ( m_ptCurrPLocInGrid.y * NUMBEROFCOLS )] = 0;
			m_ptCurrPLocInGrid.x = 0;
		} else {
			if (m_anMazeArray[m_ptCurrPLocInGrid.x + 1 + (m_ptCurrPLocInGrid.y * NUMBEROFCOLS)] != WALL) {

				if (nOldDir != m_nPDirection) {
					pTempSprite = new CSprite;
					pTempSprite = pPlayerRightSprite->DuplicateSprite(pDC);
				}
				m_ptCurrentPPos.x = (m_ptCurrPLocInGrid.x * MOVEPLAYER) + MAZELEFT;
				m_ptCurrentPPos.y = (m_ptCurrPLocInGrid.y * MOVEPLAYER) + MAZETOP;
				m_ptCurrPLocInGrid.x++;

				PickUpObject();
				PickUpSuperObject();

			} else {
				m_nPDirection = 0;
			}
		}
		break;
	case 3: // move up
		if (m_ptCurrPLocInGrid.y <= 0) {
			m_ptCurrentPPos.y = MAZEBOTTOM;
//                m_anMazeArray[m_ptCurrPLocInGrid.y + ( m_ptCurrPLocInGrid.x * NUMBEROFCOLS )]=0;
			m_ptCurrPLocInGrid.y = 26;
		} else {
			if (m_anMazeArray[m_ptCurrPLocInGrid.x + ((m_ptCurrPLocInGrid.y - 1) * NUMBEROFCOLS)] != WALL) {

				if (nOldDir != m_nPDirection) {
					pTempSprite = new CSprite;
					pTempSprite = pPlayerUpSprite->DuplicateSprite(pDC);
				}
				m_ptCurrentPPos.x = (m_ptCurrPLocInGrid.x * MOVEPLAYER) + MAZELEFT;
				m_ptCurrentPPos.y = (m_ptCurrPLocInGrid.y * MOVEPLAYER) + MAZETOP;
				m_ptCurrPLocInGrid.y--;

				PickUpObject();
				PickUpSuperObject();

			} else {
				m_nPDirection = 0;
			}
		}

		break;
	case 4: // move down
		if (m_ptCurrPLocInGrid.y >= 26) {
			m_ptCurrentPPos.y = MAZETOP;
//                m_anMazeArray[m_ptCurrPLocInGrid.x + ( m_ptCurrPLocInGrid.y * NUMBEROFCOLS )]=0;
			m_ptCurrPLocInGrid.y = 0;
		} else {
			if (m_anMazeArray[m_ptCurrPLocInGrid.x + ((m_ptCurrPLocInGrid.y + 1) * NUMBEROFCOLS)] != WALL) {
				if (nOldDir != m_nPDirection) {
					pTempSprite = new CSprite;
					pTempSprite = pPlayerDownSprite->DuplicateSprite(pDC);
				}
				m_ptCurrentPPos.x = (m_ptCurrPLocInGrid.x * MOVEPLAYER) + MAZELEFT;
				m_ptCurrentPPos.y = (m_ptCurrPLocInGrid.y * MOVEPLAYER) + MAZETOP;
				m_ptCurrPLocInGrid.y++;

				PickUpObject();
				PickUpSuperObject();

			} else {
				m_nPDirection = 0;
			}
		}
		break;
	}

	if (pTempSprite != nullptr) {
		pTempSprite->LinkSprite();
		pTempSprite->PaintSprite(pDC, m_ptCurrentPPos);
		pPlayerSprite->EraseSprite(pDC);
		pPlayerSprite->UnlinkSprite();
		delete pPlayerSprite;
		pPlayerSprite = pTempSprite;
	}

	pPlayerSprite->PaintSprite(pDC, m_ptCurrentPPos);

//	m_anMazeArray[m_ptCurrPLocInGrid.x + ( m_ptCurrPLocInGrid.y * NUMBEROFCOLS )]=0;

	for (nWhichBadGuy = 0; nWhichBadGuy < 4; nWhichBadGuy++) {
		if (PlayerKilled(nWhichBadGuy)) {
			bKillPlayer = true;
			if (pDC != nullptr)
				ReleaseDC(pDC);
			return;
		}
	}

	if (CheckMessages() == true) {
		PostMessage(WM_CLOSE);
		bEndGame = true;
	}

	if (m_nNumberOfObjectsLeft < 0) {

		if (m_lpGameStruct->bPlayingMetagame) {
			pOptionButton->EnableWindow(true);
			PaintObjects();
			if (m_bMouseCaptured) {
				ReleaseCapture();
				m_bMouseCaptured = false;
				while (MFC::ShowCursor(true) < 0)
					;
			}
			cMessageBoxDlg.SetInitialOptions(5, m_lScore);
			cMessageBoxDlg.DoModal();
			(*this).SetFocus();
			m_nPDirection = 0;
			m_nNextDir = 0;
			m_bSuspend = true;
			PostMessage(WM_CLOSE, 0, 0);
			bEndGame = true;
//          return;
		} else {
			m_nGameLevel++;
			if (m_nGameLevel > LEVELMAX) {
				if (m_lpGameStruct->bSoundEffectsEnabled) {
					sndPlaySound(nullptr, SND_ASYNC);
					sndPlaySound(LASTLEV_WAV, SND_ASYNC);
				}
				pOptionButton->EnableWindow(true);
				if (m_bMouseCaptured) {
					ReleaseCapture();
					m_bMouseCaptured = false;
					while (MFC::ShowCursor(true) < 0)
						;
				}
				cMessageBoxDlg.SetInitialOptions(2, m_lScore);
				cMessageBoxDlg.DoModal();
				(*this).SetFocus();
				m_bSuspend = true;
				if (m_lpGameStruct->bPlayingMetagame) {
					sndPlaySound(nullptr, SND_ASYNC);
					PostMessage(WM_CLOSE, 0, 0);
					bEndGame = true;
				}
			} else {
				pOptionButton->EnableWindow(true);
				if (m_lpGameStruct->bSoundEffectsEnabled) {
					sndPlaySound(LEVEL_WAV, SND_ASYNC);
				}
				if (m_bMouseCaptured) {
					ReleaseCapture();
					m_bMouseCaptured = false;
					while (MFC::ShowCursor(true) < 0)
						;
				}
				cMessageBoxDlg.SetInitialOptions(1, m_lScore, (m_nGameLevel - 1));
				cMessageBoxDlg.DoModal();
				(*this).SetFocus();
				SetMaze();
				m_bSuspend = false;
			}
		}
	}

	if (m_nNumberOfObjectsLeft == 0)
		m_nNumberOfObjectsLeft--;

	if (pDC != nullptr) {
		ReleaseDC(pDC);
	}
	return;
}

/*****************************************************************
 *
 * MovePlayer
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *
 * FORMAL PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/
void CMainPackRatWindow::MovePlayer() {
	CDC *pDC = GetDC();

	if (bEndGame) {
		ReleaseDC(pDC);
		return;
	}

	if (nPlayerMove > m_nPlayerSpeed) {

		if (bKillPlayer) {
			KillPlayer();
		}

		if (nKillBadGuy > 0) {
			KillBadGuy(nKillBadGuy);
		}

		nPlayerMove = 1;
		ReleaseDC(pDC);
		SetNewPlayerPos();
		return;
	}
	switch (m_nPDirection) {
	case 1: // move left
		m_ptCurrentPPos.x = ((m_ptCurrPLocInGrid.x + 1) * MOVEPLAYER) + MAZELEFT - ((nPlayerMove * MOVEPLAYER) / m_nPlayerSpeed);
		m_ptCurrentPPos.y = (m_ptCurrPLocInGrid.y * MOVEPLAYER) + MAZETOP;
		pPlayerSprite->PaintSprite(pDC, m_ptCurrentPPos);
		break;
	case 2: // move right
		m_ptCurrentPPos.x = ((m_ptCurrPLocInGrid.x - 1) * MOVEPLAYER) + MAZELEFT + ((nPlayerMove * MOVEPLAYER) / m_nPlayerSpeed);
		m_ptCurrentPPos.y = (m_ptCurrPLocInGrid.y * MOVEPLAYER) + MAZETOP;
		pPlayerSprite->PaintSprite(pDC, m_ptCurrentPPos);
		break;
	case 3: // move up
		m_ptCurrentPPos.x = (m_ptCurrPLocInGrid.x * MOVEPLAYER) + MAZELEFT;
		m_ptCurrentPPos.y = ((m_ptCurrPLocInGrid.y + 1) * MOVEPLAYER) + MAZETOP - ((nPlayerMove * MOVEPLAYER) / m_nPlayerSpeed);
		pPlayerSprite->PaintSprite(pDC, m_ptCurrentPPos);
		break;
	case 4: // move down
		m_ptCurrentPPos.x = (m_ptCurrPLocInGrid.x * MOVEPLAYER) + MAZELEFT;
		m_ptCurrentPPos.y = ((m_ptCurrPLocInGrid.y - 1) * MOVEPLAYER) + MAZETOP + ((nPlayerMove * MOVEPLAYER) / m_nPlayerSpeed);
		pPlayerSprite->PaintSprite(pDC, m_ptCurrentPPos);
		break;
	default:
		nPlayerMove = m_nPlayerSpeed;
		break;
	}
	if (CheckMessages() == true) {
		PostMessage(WM_CLOSE);
		bEndGame = true;
	}
	nPlayerMove++;
	//pDC->SelectPalette( pOldPal, false );
	ReleaseDC(pDC);
	return;
}

/*****************************************************************
 *
 * SetNewBadGuyPos
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *
 * FORMAL PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/
bool CMainPackRatWindow::SetNewBadGuyPos(int nWhichBadGuy, bool bForceRepaint) {
	bool    bReturn = true;
	CDC     *pDC = GetDC();
	int     nOldDir;
//bool    bNewDirOK = false;
	CSprite *pTempSprite = nullptr;
	bool    bSeePlayer = false;
	bool    bNearPlayer = false;
	int     nOldDirection;
	int     nDirectionToGo = 0;
	int     nDist1 = abs(m_aptCurrBLocInGrid[nWhichBadGuy].x - m_ptCurrPLocInGrid.x);
	int     nDist2 = abs(m_aptCurrBLocInGrid[nWhichBadGuy].y - m_ptCurrPLocInGrid.y);

	if (bEndGame) {
		ReleaseDC(pDC);
		return (bReturn);
	}

	if (m_aptCurrBLocInGrid[nWhichBadGuy].x == m_ptCurrPLocInGrid.x) {
		int     i, j, nSmaller, nLarger;
		bool    bWall = false;

		if (m_aptCurrBLocInGrid[nWhichBadGuy].y > m_ptCurrPLocInGrid.y) {
			nDirectionToGo = 1;
			nSmaller = m_ptCurrPLocInGrid.y;
			nLarger = m_aptCurrBLocInGrid[nWhichBadGuy].y;
		} else {
			nDirectionToGo = 3;
			nSmaller = m_aptCurrBLocInGrid[nWhichBadGuy].y;
			nLarger = m_ptCurrPLocInGrid.y;
		}

		j = m_ptCurrPLocInGrid.x;

		for (i = nSmaller; i < nLarger; i ++) {
			if (m_anMazeArray[j + (i * NUMBEROFCOLS)] == WALL) {
				bWall = true;
				break;
			}
		}

		if (bWall == false) {
			bSeePlayer = true;
		}

	} else {
		if (m_aptCurrBLocInGrid[nWhichBadGuy].y == m_ptCurrPLocInGrid.y) {
			int     i, j, nSmaller, nLarger;
			bool    bWall = false;

			if (m_aptCurrBLocInGrid[nWhichBadGuy].x > m_ptCurrPLocInGrid.x) {
				nDirectionToGo = 4;
				nSmaller = m_ptCurrPLocInGrid.x;
				nLarger = m_aptCurrBLocInGrid[nWhichBadGuy].x;
			} else {
				nDirectionToGo = 2;
				nSmaller = m_aptCurrBLocInGrid[nWhichBadGuy].x;
				nLarger = m_ptCurrPLocInGrid.x;
			}

			j = m_ptCurrPLocInGrid.y;

			for (i = nSmaller; i < nLarger; i ++) {
				if (m_anMazeArray[i + (j * NUMBEROFCOLS)] == WALL) {
					bWall = true;
					break;
				}
			}

			if (bWall == false) {
				bSeePlayer = true;
			}

		}
	}

	if ((bSeePlayer == false) && (nDist1 < 10) && (nDist2 < 10)) {
		nOldDirection = m_anBDirection[nWhichBadGuy];
		if (nDist1 <= nDist2) {
			if (m_aptCurrBLocInGrid[nWhichBadGuy].x > m_ptCurrPLocInGrid.x) {
				nDirectionToGo = 4;
				if ((m_anMazeArray[m_aptCurrBLocInGrid[nWhichBadGuy].x - 1 + (m_aptCurrBLocInGrid[nWhichBadGuy].y * NUMBEROFCOLS)] != WALL) && (abs(nOldDirection - nDirectionToGo) != 2))
					bNearPlayer = true;
			} else {
				nDirectionToGo = 2;
				if ((m_anMazeArray[m_aptCurrBLocInGrid[nWhichBadGuy].x + 1 + (m_aptCurrBLocInGrid[nWhichBadGuy].y * NUMBEROFCOLS)] != WALL) && (abs(nOldDirection - nDirectionToGo) != 2))
					bNearPlayer = true;
			}
		}

		if (bNearPlayer == false) {
			if (m_aptCurrBLocInGrid[nWhichBadGuy].y > m_ptCurrPLocInGrid.y) {
				nDirectionToGo = 1;
				if ((m_anMazeArray[m_aptCurrBLocInGrid[nWhichBadGuy].x + ((m_aptCurrBLocInGrid[nWhichBadGuy].y - 1) * NUMBEROFCOLS)] != WALL) && (abs(nOldDirection - nDirectionToGo) != 2))
					bNearPlayer = true;
			} else {
				nDirectionToGo = 3;
				if ((m_anMazeArray[m_aptCurrBLocInGrid[nWhichBadGuy].x + ((m_aptCurrBLocInGrid[nWhichBadGuy].y + 1) * NUMBEROFCOLS)] != WALL) && (abs(nOldDirection - nDirectionToGo) != 2))
					bNearPlayer = true;
			}
		}

		if (bNearPlayer == false) {
			if (m_aptCurrBLocInGrid[nWhichBadGuy].x > m_ptCurrPLocInGrid.x) {
				nDirectionToGo = 4;
				if ((m_anMazeArray[m_aptCurrBLocInGrid[nWhichBadGuy].x - 1 + (m_aptCurrBLocInGrid[nWhichBadGuy].y * NUMBEROFCOLS)] != WALL) && (abs(nOldDirection - nDirectionToGo) != 2))
					bNearPlayer = true;
			} else {
				nDirectionToGo = 2;
				if ((m_anMazeArray[m_aptCurrBLocInGrid[nWhichBadGuy].x + 1 + (m_aptCurrBLocInGrid[nWhichBadGuy].y * NUMBEROFCOLS)] != WALL) && (abs(nOldDirection - nDirectionToGo) != 2))
					bNearPlayer = true;
			}
		}

	}

	if (m_nNumberOfMoves > 100) {
		m_nNumberOfMoves = 0;

		if (m_nBDirection == FOLLOWRIGHTWALL)
			m_nBDirection = FOLLOWLEFTWALL;
		else
			m_nBDirection = FOLLOWRIGHTWALL;
	} else {
		m_nNumberOfMoves++;
	}

	if (bForceRepaint)
		nOldDir = 0;
	else
		nOldDir = m_anBDirection[nWhichBadGuy];

	if (m_nBDirection == FOLLOWRIGHTWALL) {
		switch (m_anBDirection[nWhichBadGuy]) {
		case 0:
			m_anBDirection[nWhichBadGuy] = 1;
			break;
		case 1: // move up
// these were != -1 instead of > -1
			if (m_anMazeArray[m_aptCurrBLocInGrid[nWhichBadGuy].x + 1 + (m_aptCurrBLocInGrid[nWhichBadGuy].y * NUMBEROFCOLS)] > WALL)
				m_anBDirection[nWhichBadGuy]++;
			break;
		case 2: // move right
			if (m_anMazeArray[m_aptCurrBLocInGrid[nWhichBadGuy].x + ((m_aptCurrBLocInGrid[nWhichBadGuy].y + 1) * NUMBEROFCOLS)] > WALL)
				m_anBDirection[nWhichBadGuy]++;
			break;
		case 3: // move down
			if (m_anMazeArray[m_aptCurrBLocInGrid[nWhichBadGuy].x - 1 + (m_aptCurrBLocInGrid[nWhichBadGuy].y * NUMBEROFCOLS)] > WALL)
				m_anBDirection[nWhichBadGuy]++;
			break;
		case 4: // move left
			if (m_anMazeArray[m_aptCurrBLocInGrid[nWhichBadGuy].x + ((m_aptCurrBLocInGrid[nWhichBadGuy].y - 1) * NUMBEROFCOLS)] > WALL)
				m_anBDirection[nWhichBadGuy] = 1;
			break;
		}
	} else {
		switch (m_anBDirection[nWhichBadGuy]) {
		case 0:
			m_anBDirection[nWhichBadGuy] = 1;
			break;
		case 1: // move up
			if (m_anMazeArray[m_aptCurrBLocInGrid[nWhichBadGuy].x - 1 + (m_aptCurrBLocInGrid[nWhichBadGuy].y * NUMBEROFCOLS)] > WALL)
				m_anBDirection[nWhichBadGuy] = 4;
			break;
		case 2: // move right
			if (m_anMazeArray[m_aptCurrBLocInGrid[nWhichBadGuy].x + ((m_aptCurrBLocInGrid[nWhichBadGuy].y - 1) * NUMBEROFCOLS)] > WALL)
				m_anBDirection[nWhichBadGuy]--;
			break;
		case 3: // move down
			if (m_anMazeArray[m_aptCurrBLocInGrid[nWhichBadGuy].x + 1 + (m_aptCurrBLocInGrid[nWhichBadGuy].y * NUMBEROFCOLS)] > WALL)
				m_anBDirection[nWhichBadGuy]--;
			break;
		case 4: // move left
			if (m_anMazeArray[m_aptCurrBLocInGrid[nWhichBadGuy].x + ((m_aptCurrBLocInGrid[nWhichBadGuy].y + 1) * NUMBEROFCOLS)] > WALL)
				m_anBDirection[nWhichBadGuy]--;
			break;
		}
	}

	if ((bSeePlayer || bNearPlayer) && (bEatTurtle == false)) {
		m_anBDirection[nWhichBadGuy] = nDirectionToGo;
	}

	switch (m_anBDirection[nWhichBadGuy]) {
	case 1: // move up
		if (m_aptCurrBLocInGrid[nWhichBadGuy].y <= 0) {
			m_aptCurrentBPos[nWhichBadGuy].y = MAZEBOTTOM;
			m_aptCurrBLocInGrid[nWhichBadGuy].y = 26;
		} else {
			if (m_anMazeArray[m_aptCurrBLocInGrid[nWhichBadGuy].x + ((m_aptCurrBLocInGrid[nWhichBadGuy].y - 1) * NUMBEROFCOLS)] != WALL) {
				m_aptCurrentBPos[nWhichBadGuy].x = (m_aptCurrBLocInGrid[nWhichBadGuy].x * MOVEBADGUY) + MAZELEFT;
				m_aptCurrentBPos[nWhichBadGuy].y = (m_aptCurrBLocInGrid[nWhichBadGuy].y * MOVEBADGUY) + MAZETOP;

				if (nOldDir != m_anBDirection[nWhichBadGuy]) {
					pTempSprite = new CSprite;

					if (bEatTurtle) {
						if (bFlashTurtle)
							pTempSprite = pFlashUpSprite->DuplicateSprite(pDC);
						else
							pTempSprite = pGoodGuyUpSprite->DuplicateSprite(pDC);
					} else
						pTempSprite = pBadGuyUpSprite->DuplicateSprite(pDC);
				} else {
					if (bEatTurtle) {
						pTempSprite = new CSprite;
						if (bFlashTurtle)
							pTempSprite = pFlashUpSprite->DuplicateSprite(pDC);
						else
							pTempSprite = pGoodGuyUpSprite->DuplicateSprite(pDC);
						abGoodTurtle[nWhichBadGuy] = true;
						abChangeTurtle[nWhichBadGuy] = false;
					}
				}

				m_aptCurrBLocInGrid[nWhichBadGuy].y--;

			} else {
				// change direction
				m_anBDirection[nWhichBadGuy] = brand() % 4;
				bReturn = false;
			}

		}
		break;
	case 2: // move right
		if (m_aptCurrBLocInGrid[nWhichBadGuy].x >= 36) {
			m_aptCurrentBPos[nWhichBadGuy].x = MAZELEFT;
			m_aptCurrBLocInGrid[nWhichBadGuy].x = 0;
		} else {
			if (m_anMazeArray[m_aptCurrBLocInGrid[nWhichBadGuy].x + 1 + (m_aptCurrBLocInGrid[nWhichBadGuy].y * NUMBEROFCOLS)] != WALL) {
				m_aptCurrentBPos[nWhichBadGuy].x = (m_aptCurrBLocInGrid[nWhichBadGuy].x * MOVEBADGUY) + MAZELEFT;
				m_aptCurrentBPos[nWhichBadGuy].y = (m_aptCurrBLocInGrid[nWhichBadGuy].y * MOVEBADGUY) + MAZETOP;

				if (nOldDir != m_anBDirection[nWhichBadGuy]) {
					if (bEatTurtle) {
						if (bFlashTurtle)
							pTempSprite = pFlashRightSprite->DuplicateSprite(pDC);
						else
							pTempSprite = pGoodGuyRightSprite->DuplicateSprite(pDC);
					} else
						pTempSprite = pBadGuyRightSprite->DuplicateSprite(pDC);
				} else {
					if (bEatTurtle) {
						if (bFlashTurtle)
							pTempSprite = pFlashRightSprite->DuplicateSprite(pDC);
						else
							pTempSprite = pGoodGuyRightSprite->DuplicateSprite(pDC);
						abGoodTurtle[nWhichBadGuy] = true;
						abChangeTurtle[nWhichBadGuy] = false;
					}
				}

				m_aptCurrBLocInGrid[nWhichBadGuy].x++;

			} else {
				// change direction
				m_anBDirection[nWhichBadGuy] = brand() % 4;
				bReturn = false;
			}
		}
		break;
	case 3: // move down
		if (m_aptCurrBLocInGrid[nWhichBadGuy].y >= 26) {
			m_aptCurrentBPos[nWhichBadGuy].y = MAZETOP;
			m_aptCurrBLocInGrid[nWhichBadGuy].y = 0;
		} else {
			if (m_anMazeArray[m_aptCurrBLocInGrid[nWhichBadGuy].x + ((m_aptCurrBLocInGrid[nWhichBadGuy].y + 1) * NUMBEROFCOLS)] != WALL) {
				m_aptCurrentBPos[nWhichBadGuy].x = (m_aptCurrBLocInGrid[nWhichBadGuy].x * MOVEBADGUY) + MAZELEFT;
				m_aptCurrentBPos[nWhichBadGuy].y = (m_aptCurrBLocInGrid[nWhichBadGuy].y * MOVEBADGUY) + MAZETOP;

				if (nOldDir != m_anBDirection[nWhichBadGuy]) {
					if (bEatTurtle) {
						if (bFlashTurtle)
							pTempSprite = pFlashDownSprite->DuplicateSprite(pDC);
						else
							pTempSprite = pGoodGuyDownSprite->DuplicateSprite(pDC);
					} else
						pTempSprite = pBadGuyDownSprite->DuplicateSprite(pDC);
				} else {
					if (bEatTurtle) {
						if (bFlashTurtle)
							pTempSprite = pFlashDownSprite->DuplicateSprite(pDC);
						else
							pTempSprite = pGoodGuyDownSprite->DuplicateSprite(pDC);
						abGoodTurtle[nWhichBadGuy] = true;
						abChangeTurtle[nWhichBadGuy] = false;
					}
				}

				m_aptCurrBLocInGrid[nWhichBadGuy].y++;

			} else {
				// change direction
				m_anBDirection[nWhichBadGuy] = brand() % 4;
				bReturn = false;
			}
		}
		break;
	case 4: // move left
		if (m_aptCurrBLocInGrid[nWhichBadGuy].x <= 0) {
			m_aptCurrentBPos[nWhichBadGuy].x = MAZERIGHT;
			m_aptCurrBLocInGrid[nWhichBadGuy].x = 36;
		} else {
			if (m_anMazeArray[m_aptCurrBLocInGrid[nWhichBadGuy].x - 1 + (m_aptCurrBLocInGrid[nWhichBadGuy].y * NUMBEROFCOLS)] != WALL) {
				m_aptCurrentBPos[nWhichBadGuy].x = (m_aptCurrBLocInGrid[nWhichBadGuy].x * MOVEBADGUY) + MAZELEFT;
				m_aptCurrentBPos[nWhichBadGuy].y = (m_aptCurrBLocInGrid[nWhichBadGuy].y * MOVEBADGUY) + MAZETOP;

				if (nOldDir != m_anBDirection[nWhichBadGuy]) {
					if (bEatTurtle) {
						if (bFlashTurtle)
							pTempSprite = pFlashLeftSprite->DuplicateSprite(pDC);
						else
							pTempSprite = pGoodGuyLeftSprite->DuplicateSprite(pDC);
					} else
						pTempSprite = pBadGuyLeftSprite->DuplicateSprite(pDC);
				} else {
					if (bEatTurtle) {
						if (bFlashTurtle)
							pTempSprite = pFlashLeftSprite->DuplicateSprite(pDC);
						else
							pTempSprite = pGoodGuyLeftSprite->DuplicateSprite(pDC);
						abGoodTurtle[nWhichBadGuy] = true;
						abChangeTurtle[nWhichBadGuy] = false;
					}
				}

				m_aptCurrBLocInGrid[nWhichBadGuy].x--;

			} else {
				// change direction
				m_anBDirection[nWhichBadGuy] = brand() % 4;
				bReturn = false;
			}
		}
		break;
	}

	if (pTempSprite != nullptr) {
		pTempSprite->LinkSprite();
		pTempSprite->PaintSprite(pDC, m_aptCurrentBPos[nWhichBadGuy]);
		apBadGuySprite[nWhichBadGuy]->EraseSprite(pDC);
		apBadGuySprite[nWhichBadGuy]->UnlinkSprite();
		delete apBadGuySprite[nWhichBadGuy];
		apBadGuySprite[nWhichBadGuy] = pTempSprite;
	}

	apBadGuySprite[nWhichBadGuy]->PaintSprite(pDC, m_aptCurrentBPos[nWhichBadGuy]);
	pTempSprite = nullptr;

	if (PlayerKilled(nWhichBadGuy)) {
		bKillPlayer = true;
	} else {
		if (nKillBadGuy != 0) {
			if (pDC != nullptr)
				ReleaseDC(pDC);
			return bReturn;
		}
	}

	if (CheckMessages() == true) {
		PostMessage(WM_CLOSE);
		bEndGame = true;
	}
	if (m_anBDirection[nWhichBadGuy] > 4)
		m_anBDirection[nWhichBadGuy] = brand() % 4;
	bChangeTurtle = false;
	if (pDC != nullptr)
		ReleaseDC(pDC);
	return bReturn;
}

/*****************************************************************
 *
 * MoveBadGuys
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *
 * FORMAL PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/
void CMainPackRatWindow::MoveBadGuys() {
	CDC *pDC = GetDC();
	int nWhichBadGuy;
	bool    bTempBool1;
	bool    bTempBool2;


	for (nWhichBadGuy = 0; nWhichBadGuy < 4; nWhichBadGuy++) {

		if (bEndGame) {
			ReleaseDC(pDC);
			return;
		}

		if (anBadGuyMove[nWhichBadGuy] > m_nBadGuySpeed) {

			if (bKillPlayer)
				KillPlayer();

			if (nKillBadGuy >= Power(2, nWhichBadGuy)) {
				KillBadGuy(nKillBadGuy);
			}

			anBadGuyMove[nWhichBadGuy] = 1;
			bTempBool1 = false;
			bTempBool2 = false;
			while (bTempBool1 == false) {
				bTempBool1 = SetNewBadGuyPos(nWhichBadGuy, bTempBool2);
				bTempBool2 = true;
			}
			if (bEndGame) {
				ReleaseDC(pDC);
				return;
			}

		} else {
			switch (m_anBDirection[nWhichBadGuy]) {
			case 1: // move up
				m_aptCurrentBPos[nWhichBadGuy].x = (m_aptCurrBLocInGrid[nWhichBadGuy].x * MOVEBADGUY) + MAZELEFT;
				m_aptCurrentBPos[nWhichBadGuy].y = ((m_aptCurrBLocInGrid[nWhichBadGuy].y + 1) * MOVEBADGUY) + MAZETOP - ((anBadGuyMove[nWhichBadGuy] * MOVEBADGUY) / m_nBadGuySpeed);
				apBadGuySprite[nWhichBadGuy]->PaintSprite(pDC, m_aptCurrentBPos[nWhichBadGuy]);
				break;
			case 2: // move right
				m_aptCurrentBPos[nWhichBadGuy].x = ((m_aptCurrBLocInGrid[nWhichBadGuy].x - 1) * MOVEBADGUY) + MAZELEFT + ((anBadGuyMove[nWhichBadGuy] * MOVEBADGUY) / m_nBadGuySpeed);
				m_aptCurrentBPos[nWhichBadGuy].y = (m_aptCurrBLocInGrid[nWhichBadGuy].y * MOVEBADGUY) + MAZETOP;
				apBadGuySprite[nWhichBadGuy]->PaintSprite(pDC, m_aptCurrentBPos[nWhichBadGuy]);
				break;
			case 3: // move down
				m_aptCurrentBPos[nWhichBadGuy].x = (m_aptCurrBLocInGrid[nWhichBadGuy].x * MOVEBADGUY) + MAZELEFT;
				m_aptCurrentBPos[nWhichBadGuy].y = ((m_aptCurrBLocInGrid[nWhichBadGuy].y - 1) * MOVEBADGUY) + MAZETOP + ((anBadGuyMove[nWhichBadGuy] * MOVEBADGUY) / m_nBadGuySpeed);
				apBadGuySprite[nWhichBadGuy]->PaintSprite(pDC, m_aptCurrentBPos[nWhichBadGuy]);
				break;
			case 4: // move left
				m_aptCurrentBPos[nWhichBadGuy].x = ((m_aptCurrBLocInGrid[nWhichBadGuy].x + 1) * MOVEBADGUY) + MAZELEFT - ((anBadGuyMove[nWhichBadGuy] * MOVEBADGUY) / m_nBadGuySpeed);
				m_aptCurrentBPos[nWhichBadGuy].y = (m_aptCurrBLocInGrid[nWhichBadGuy].y * MOVEBADGUY) + MAZETOP;
				apBadGuySprite[nWhichBadGuy]->PaintSprite(pDC, m_aptCurrentBPos[nWhichBadGuy]);
				break;
			}

			anBadGuyMove[nWhichBadGuy]++;
		}
	}

	ReleaseDC(pDC);
	return;
}


/*****************************************************************
 *
 * CheckMessages
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *
 * FORMAL PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/
bool CMainPackRatWindow::CheckMessages() {
	MSG msg;

	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {

		if (msg.message == WM_CLOSE || msg.message == WM_QUIT) {
			bEndGame = true;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (bEndGame);
}

/*****************************************************************
 *
 * SplashScreen
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Repaint the background artwork, together with all sprites in the
 * sprite chain queue.  The entire window is redrawn, rather than just
 * the updated area, to ensure that the sprites end up with the correct
 * background bitmaps saved for their image areas.
 *
 * FORMAL PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/

void CMainPackRatWindow::SplashScreen() {
	CDC         *pDC = GetDC();                                     // pointer to the window device context
	CSprite     *pSprite = nullptr;
	POINT       ptLive;
	int         nLoop1;

	CSprite::RefreshBackdrop(pDC, pGamePalette);

	pSprite = CSprite::GetSpriteChain();
	if (pSprite != nullptr) {
		while (pSprite !=  nullptr) {
			pSprite->ClearBackground();
			pSprite->RefreshSprite(pDC);
			pSprite = pSprite->GetNextSprite();
		}

		/*
		    for ( nLoop1 = 0; nLoop1 < MAXLIVES; nLoop1++ ) {
		        if ( pLivesSprite[nLoop1] != nullptr )
		            pLivesSprite[nLoop1]->EraseSprite( pDC );
		    }
		*/
	}

	for (nLoop1 = 0; nLoop1 < MAXLIVESPRITE; nLoop1++) {
		if (pLivesSprite[nLoop1] != nullptr) {
			pLivesSprite[nLoop1]->EraseSprite(pDC);
			if (nLoop1 < m_nLives) {
				ptLive.x = 15 + (17 * nLoop1);
				ptLive.y = 25;
				pLivesSprite[nLoop1]->PaintSprite(pDC, ptLive);
			}
		}
	}

	/*
	for ( nLoop1 = 0; nLoop1 < m_nLives; nLoop1++ ) {
	    ptLive.x = 15 + ( 17 * nLoop1);
	    ptLive.y = 25;
	    if ( pLivesSprite[nLoop1] != nullptr )
	        pLivesSprite[nLoop1]->PaintSprite( pDC, ptLive );
	}
	*/

	if ((bBALLOONShown) && (pBalloonSprite != nullptr)) {
		pBalloonSprite->PaintSprite(pDC, ptBaloon.x, ptBaloon.y);
	}


	ReleaseDC(pDC);                                     // release the window's context
}


/*****************************************************************
 *
 * ResetGame
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Start a new game, and reset all arrays and buttons
 *
 * FORMAL PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  aDealtArray, apHold
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/

void CMainPackRatWindow::ResetGame() {
	if (m_nGameLevel >= 36)
		m_nGameLevel = 36;

	m_nLives = nStartingLives;
	m_lScore = 0L;
	nExtraLives = 4;
	bEndGame = false;
	SetMaze();
	(*this).SetFocus();
	return;
}

/*****************************************************************
 *
 * OnCommand
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Process the QUIT and OKAY buttons when they are clicked.
 *
 * This function is called when a WM_COMMAND message is issued,
 * typically in order to process control related activities.
 *
 * FORMAL PARAMETERS:
 *
 *  wParam      identifier for the button to be processed
 *  lParam      type of message to be processed
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/

// OnCommand
//

void CALLBACK lpfnOptionCallback(CWnd * pWnd) {
// do the mini options dialog
	int             nOption = 0;       // return from the Options dialog
//unsigned int            x = IDD_MINIOPTIONS_DIALOG;
	CPackRatOptDlg  dlgMiniOptDlg(pWnd, pGamePalette, IDD_MINIOPTIONS_DIALOG);

	bResetGame = false;

	dlgMiniOptDlg.SetInitialOptions(min(pcwndPackRat->m_nGameLevel, 36), nStartingLives);

	nOption = dlgMiniOptDlg.DoModal();
	if (nOption != 0) {
		pcwndPackRat->m_nGameLevel = nOption / 10;
		nStartingLives = nOption % 10;
		bResetGame = true;
	}
	return;
}

bool CMainPackRatWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
	int             nMainOption = 0;       // return from the Options dialog
	bool            bContinue = true;

	if (HIWORD(lParam) == BN_CLICKED)           // only want to look at button clicks
		switch (wParam) {

// Option button clicked, then put up the Options dialog
		case IDC_OPTION:
			m_bSuspend = true;
			pOptionButton->EnableWindow(false);
			bResetGame = false;
			UpdateWindow();
			//CSound::waitWaveSounds();
			sndPlaySound(nullptr, 0);
			if (m_bMouseCaptured) {
				ReleaseCapture();
				m_bMouseCaptured = false;
				while (MFC::ShowCursor(true) < 0)
					;
			}
			if (m_lpGameStruct->bPlayingMetagame) {
				CMainMenu       dlgMainOpts((CWnd *)this, pGamePalette, (NO_NEWGAME | NO_OPTIONS),
				                            lpfnOptionCallback, RULESFILE,
				                            (m_lpGameStruct->bSoundEffectsEnabled ? RULES_WAV : nullptr), m_lpGameStruct);
				nMainOption = dlgMainOpts.DoModal();
				switch (nMainOption) {
				case IDC_OPTIONS_QUIT:
					//      if Quit buttons was hit, quit
					PostMessage(WM_CLOSE, 0, 0);
					bFirstTimer = false;
					bEndGame = true;
					break;
				}
			} else {
				CMainMenu       dlgMainOpts((CWnd *)this, pGamePalette, 0,
				                            lpfnOptionCallback, RULESFILE,
				                            (m_lpGameStruct->bSoundEffectsEnabled ? RULES_WAV : nullptr), m_lpGameStruct);
				nMainOption = dlgMainOpts.DoModal();
				switch (nMainOption) {
				case IDC_OPTIONS_QUIT:
					//      if Quit buttons was hit, quit
					PostMessage(WM_CLOSE, 0, 0);
					bFirstTimer = false;
					bEndGame = true;
					break;
				case IDC_OPTIONS_NEWGAME:
					// reset the game and start a new hand
					bResetGame = true;
					break;
				case IDC_OPTIONS_RETURN:
					if (m_nLives <= 0) {
						bContinue = false;
					}
					break;
				}
			}

			if (m_lpGameStruct->bMusicEnabled) {
				if (pGameSound == nullptr) {
					pGameSound = new CSound(this, GAME_THEME,
					                        SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
					if (pGameSound != nullptr)
						(*pGameSound).midiLoopPlaySegment(2310, 29400, 0, FMT_MILLISEC);
				}
			} // end if pGameSound
			else {
				if (pGameSound != nullptr) {
					pGameSound->stop();
					delete pGameSound;
					pGameSound = nullptr;
				}
			}


			if (bResetGame)
				ResetGame();

			pOptionButton->EnableWindow(true);
			m_bSuspend = false;

			if (bContinue == false) {
				m_bSuspend = true;
				return true;
			}

			if (bFirstTimer) {
				bFirstTimer = false;
				SetTimer(PACKRATTIMER, 50, nullptr);
			}
			break;
		}

// if the Hols buttons are hit then set then accordingly

	(*this).SetFocus();                         // Reset focus back to the main window
	return true;
}

void CMainPackRatWindow::OnMouseMove(unsigned int nFlags, CPoint point) {

	if (m_bMouseCaptured) {

		CRect   rMazeRect(15, 23, 624, 464);
		int     dx, dy;

		if (rMazeRect.PtInRect(point)) {
			if (bMouseHidden == false) {
				while (MFC::ShowCursor(false) >= 0)
					;
				bMouseHidden = true;
			}
		} else {
			if (bMouseHidden) {
				bMouseHidden = false;
				while (MFC::ShowCursor(true) < 0)
					;
			}
		}

		if ((ptLastMouseCoord.x == -1) && (ptLastMouseCoord.y == -1)) {
			ptLastMouseCoord.x = point.x;
			ptLastMouseCoord.y = point.y;
		} else {
			dx = ptLastMouseCoord.x - point.x;
			dy = ptLastMouseCoord.y - point.y;
			if (abs(dx) > abs(dy)) {     // move left/right
				if (dx > 10) {   // move left
					m_nNextDir = 1;
				} else {
					if (abs(dx) > 10) {      // move right
						m_nNextDir = 2;
					}
				}
			} else {
				if (dy > 10) {   // move up
					m_nNextDir = 3;
				} else {
					if (abs(dy) > 10) {      // move down
						m_nNextDir = 4;
					}
				}
			}
		}
	}

	CWnd::OnMouseMove(nFlags, point);
}

void CMainPackRatWindow::OnLButtonDown(unsigned int nFlags, CPoint point) {

	if (m_lpGameStruct->bPlayingMetagame == false) {
		if ((rNewGame.PtInRect(point)) && (m_bMouseCaptured == false))  {
			/*
			            SetCapture();
			            m_bMouseCaptured = true;
			            bInNewGameRect = true;
			*/
			ResetGame();
		}
	}

	CWnd::OnLButtonDown(nFlags, point);
}

void CMainPackRatWindow::OnLButtonUp(unsigned int nFlags, CPoint point) {

	/*
	    if ( m_bMouseCaptured ) {
	        m_bMouseCaptured = false;
	        ReleaseCapture();

	        if (bInNewGameRect ) {
	            bInNewGameRect = false;
	            ResetGame();
	        }
	    }
	*/
	CWnd::OnLButtonUp(nFlags, point);
}

void CMainPackRatWindow::OnRButtonDown(unsigned int nFlags, CPoint point) {

	if (m_bMouseCaptured) {
		ReleaseCapture();
		m_bMouseCaptured = false;
		while (MFC::ShowCursor(true) < 0)
			;
	} else {
		/*
		this was the old way
		*/
		SetCapture();
		m_bMouseCaptured = true;
		while (MFC::ShowCursor(false) < 0)
			;
	}
	CWnd::OnRButtonDown(nFlags, point);
}

void CMainPackRatWindow::OnRButtonUp(unsigned int nFlags, CPoint point) {
	/*
	    if ( m_bMouseCaptured ) {
	        m_bMouseCaptured = false;
	        ReleaseCapture();
	        while ( MFC::ShowCursor( true ) < 0 )
	            ;
	    }
	*/
	CWnd::OnRButtonUp(nFlags, point);
}

void CMainPackRatWindow::OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	CRules  dlgRules((CWnd *)this, RULESFILE, pGamePalette,
	                 (m_lpGameStruct->bSoundEffectsEnabled ? RULES_WAV : nullptr));

	switch (nChar) {
	case VK_F1:
		if (bInLoop) {
			bool    bTemp = m_bSuspend;
			m_bSuspend = true;
			sndPlaySound(nullptr, SND_ASYNC);
			if (m_bMouseCaptured) {
				m_bMouseCaptured = false;
				ReleaseCapture();
				while (MFC::ShowCursor(true) < 0)
					;
			}
			pOptionButton->EnableWindow(false);
			UpdateWindow();
			dlgRules.DoModal();      // invoke the help dialog box
			pOptionButton->EnableWindow(true);
			m_bSuspend = bTemp;
		}
		break;

	case VK_F2:
		if (bInLoop) {
			if (m_bMouseCaptured) {
				m_bMouseCaptured = false;
				ReleaseCapture();
				while (MFC::ShowCursor(true) < 0)
					;
			}
			SendMessage(WM_COMMAND, IDC_OPTION, BN_CLICKED);
		}
		break;

	case VK_H:
	case VK_NUMPAD4:
	case VK_LEFT:
		m_nNextDir = 1;
		break;

	case VK_L:
	case VK_NUMPAD6:
	case VK_RIGHT:
		m_nNextDir = 2;
		break;

	case VK_K:
	case VK_NUMPAD8:
	case VK_UP:
		m_nNextDir = 3;
		break;

	case VK_J:
	case VK_NUMPAD2:
	case VK_DOWN:
		m_nNextDir = 4;
		break;

	case VK_SPACE:
	case VK_RETURN:
	case VK_ESCAPE:
		if (m_bMouseCaptured) {
			m_bMouseCaptured = false;
			ReleaseCapture();
			while (MFC::ShowCursor(true) < 0)
				;
		}
		break;

	default:
		CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
		break;
	}
	(*this).SetFocus();
	return;
}

void CMainPackRatWindow::OnTimer(uintptr nWhichTimer) {
	if (nWhichTimer == PACKRATTIMER) {
		KillTimer(PACKRATTIMER);
		bEndGame = false;
		bInLoop = true;
		MainLoop();
		bInLoop = false;
		bFirstTimer = true;
		return;
	}
	return;
}

/*****************************************************************
 *
 * MainLoop
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *
 * FORMAL PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/
void CMainPackRatWindow::MainLoop() {
	int         nLoop1;
	int         nLoop2;
	CDC         *pDC = nullptr;
	POINT       ptLive;
	bool        bRedrawLives = false;

	(*this).SetFocus();
	while (bEndGame == false) {
		if (m_bSuspend == false) {
			if ((nEatTurtle <= (EATTURTLE - (4 * m_nGameLevel))) && (bEatTurtle)) {
				bEatTurtle = true;
				nEatTurtle++;
				if (nEatTurtle > (EATTURTLE - (4 * m_nGameLevel) - 40))
					bFlashTurtle = true;
				else
					bFlashTurtle = false;
			} else {
				if (nEatTurtle > 0)  {
					nNumOfBGKilled = 0;
					bChangeTurtle = true;
					bEatTurtle = false;
					nEatTurtle = 0;
					m_nBadGuySpeed = nOldTSpeed;
				}
			}

			for (nLoop1 = 0; nLoop1 < 4; nLoop1++) {
				if ((anReleaseTurtleCount[nLoop1] <= RELEASETURTLECOUNT) && (anReleaseTurtleCount[nLoop1] > 0)) {
					if (bStart)
						anReleaseTurtleCount[nLoop1]++;
				} else {
					if (anReleaseTurtleCount[nLoop1] > RELEASETURTLECOUNT) {
						anReleaseTurtleCount[nLoop1] = 0;
						switch (nLoop1) {
						case 0:
							m_aptCurrBLocInGrid[0].x = 1;
							m_aptCurrBLocInGrid[0].y = 4;
							break;
						case 1:
							m_aptCurrBLocInGrid[1].x = 35;
							m_aptCurrBLocInGrid[1].y = 4;
							break;
						case 2:
							m_aptCurrBLocInGrid[2].x = 1;
							m_aptCurrBLocInGrid[2].y = 22;
							break;
						case 3:
							m_aptCurrBLocInGrid[3].x = 35;
							m_aptCurrBLocInGrid[3].y = 22;
							break;
						}
					}
				}
			}
			if (bEndGame)
				return;
			if ((m_nPDirection != 0) || (m_nNextDir != 0) || (bUseJoyStick)) {
				bStart = true;
				MovePlayer();
			}

			if ((m_lpGameStruct->bPlayingMetagame) && (m_lScore > (350 * m_nGameLevel))) {
				if (bBALLOONShown == false) {
					pDC = GetDC();
					pBalloonSprite->PaintSprite(pDC, ptBaloon.x, ptBaloon.y);
					ReleaseDC(pDC);
					bBALLOONShown = true;
				}
			}

			if ((m_lpGameStruct->bPlayingMetagame == false) && (m_lScore > 40000)) {
				if (nExtraLives == 1)
					bRedrawLives = true;
			}
			if ((m_lpGameStruct->bPlayingMetagame == false) && (m_lScore > 20000)) {
				if (nExtraLives == 2)
					bRedrawLives = true;
			}
			if ((m_lpGameStruct->bPlayingMetagame == false) && (m_lScore > 10000)) {
				if (nExtraLives == 3)
					bRedrawLives = true;
			}
			if ((m_lpGameStruct->bPlayingMetagame == false) && (m_lScore > 5000)) {
				if (nExtraLives == 4)
					bRedrawLives = true;
			}

			if (bRedrawLives) {
				bRedrawLives = false;
				if (m_nLives < MAXLIVES) {
					if (m_lpGameStruct->bSoundEffectsEnabled) {
						/*
						                    CSound  *pSound = nullptr;
						                        CSound::StopWaveSounds();
						                        sndPlaySound( nullptr, SND_ASYNC);
						                        pSound = new CSound((CWnd *)this, ".\\SOUND\\NEWLIFE.WAV", SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE );
						                        pSound->play();
						*/
						sndPlaySound(nullptr, SND_ASYNC);
						sndPlaySound(NEWLIFE_WAV, SND_ASYNC);
					}

					nExtraLives--;
					m_nLives++;
					pDC = GetDC();
					/*
					                    for ( nLoop2 = 0; nLoop2 < MAXLIVES; nLoop2++ )
					                        pLivesSprite[nLoop2]->EraseSprite( pDC );

					                    for ( nLoop2 = 0; nLoop2 < m_nLives; nLoop2++ ) {
					                        ptLive.x = 15 + ( 17 * nLoop2);
					                        ptLive.y = 25;
					                        pLivesSprite[nLoop2]->PaintSprite( pDC, ptLive );
					                    }
					*/
					for (nLoop2 = 0; nLoop2 < MAXLIVESPRITE; nLoop2++) {
						if (pLivesSprite[nLoop2] != nullptr) {
							pLivesSprite[nLoop2]->EraseSprite(pDC);
							if (nLoop2 < m_nLives) {
								ptLive.x = 15 + (17 * nLoop2);
								ptLive.y = 25;
								pLivesSprite[nLoop2]->PaintSprite(pDC, ptLive);
							}
						}
					}
					ReleaseDC(pDC);
				}
			}

			if (bEndGame)
				return;


			if (nPaintObject == 1) {
				PaintObjects();
			} else {
				if (nPaintObject == 2) {
					PaintObjects(2);
				}
			}

			if (bEndGame)
				return;

			if (m_nGameLevel == 45)  {
				if (bEndGame)
					return;
				MoveBadGuys();
				if (bEndGame)
					return;
				MoveBadGuys();
				if (bEndGame)
					return;
			} else {
				if (bEndGame)
					return;
				MoveBadGuys();
				if (bEndGame)
					return;
			}
		}
		if (CheckMessages() == true) {
			PostMessage(WM_CLOSE);
			bEndGame = true;
			return;
		}
	}
	return;
}

/*****************************************************************
 *
 * OnXXXXXX
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      These functions are called when ever the corresponding WM_
 *      event message is generated for the mouse.
 *
 *      (Add game-specific processing)
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/

bool CMainPackRatWindow::OnEraseBkgnd(CDC *pDC) {
// eat this
	return true;
}

void CMainPackRatWindow::OnActivate(unsigned int nState, CWnd   *pWndOther, bool bMinimized) {
	if (!bMinimized)
		switch (nState) {
		case WA_INACTIVE:
			m_bSuspend = true;
			break;
		case WA_ACTIVE:
		case WA_CLICKACTIVE:
			m_bSuspend = false;
			InvalidateRect(nullptr, false);
			break;
		}
	return;
}


/*****************************************************************
 *
 * OnClose
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  This function is called when a Close event is generated.  For
 *  this sample application we need only kill our event timer;
 *  The ExitInstance will handle releasing resources.
 *
 * FORMAL PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      n/a
 *
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/

void CMainPackRatWindow::OnClose() {
	CDC *pDC = GetDC();
	CRect       rctFillRect(0, 0, 640, 480);
	CBrush  Brush(RGB(0, 0, 0));

	pDC->FillRect(&rctFillRect, &Brush);
	ReleaseDC(pDC);
	ReleaseResources();
	if (bUseJoyStick)
		joyReleaseCapture(JOYSTICKID1);

	if (m_bMouseCaptured) {
		ReleaseCapture();
		m_bMouseCaptured = false;
	}
	while (MFC::ShowCursor(true) < 0)
		;

	CFrameWnd ::OnClose();
}

/*****************************************************************
 *
 * OnDestroy
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  This function is called when after the window has been destroyed.
 *  For poker, we post a message bak to the calling app to tell it
 * that the user has quit the game, and therefore the app can unload
 * this DLLL
 *
 * FORMAL PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      n/a
 *
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/
void CMainPackRatWindow::OnDestroy() {
	//  send a message to the calling app to tell it the user has quit the game
	m_lpGameStruct->lScore = m_lScore;
	MFC::PostMessage(m_hCallAppWnd, WM_PARENTNOTIFY, WM_DESTROY, (LPARAM)m_lpGameStruct);
	m_lpGameStruct = nullptr;
	CFrameWnd::OnDestroy();
}


/*****************************************************************
 *
 * ReleaseResources
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Release all resources that were created and retained during the
 *  course of the game.  This includes sprites in the sprite chain,
 *  the game color palette, and button controls.
 *
 * FORMAL PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      n/a
 *
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/

void CMainPackRatWindow::ReleaseResources() {
	int nLoop1;

	delete pBalloonSprite;
	pBalloonSprite = nullptr;

	delete pGameSound;                      // delete the game theme song
	pGameSound = nullptr;

	CSound::clearSounds();
	CSprite::FlushSpriteChain();

	if (pSSpaceBMP != nullptr) {
		pSSpaceBMP->DeleteObject();
		delete pSSpaceBMP;
		pSSpaceBMP = nullptr;
	}
	if (pLSpaceBMP != nullptr) {
		pLSpaceBMP->DeleteObject();
		delete pLSpaceBMP;
		pLSpaceBMP = nullptr;
	}

	delete pPlayerRightSprite;
	delete pPlayerLeftSprite;
	delete pPlayerUpSprite;
	delete pPlayerDownSprite;
	pPlayerRightSprite = nullptr;
	pPlayerLeftSprite = nullptr;
	pPlayerUpSprite = nullptr;
	pPlayerDownSprite = nullptr;

	delete pBadGuyUpSprite;
	delete pBadGuyDownSprite;
	delete pBadGuyLeftSprite;
	delete pBadGuyRightSprite;
	pBadGuyUpSprite = nullptr;
	pBadGuyDownSprite = nullptr;
	pBadGuyLeftSprite = nullptr;
	pBadGuyRightSprite = nullptr;

	delete pGoodGuyUpSprite;
	delete pGoodGuyDownSprite;
	delete pGoodGuyLeftSprite;
	delete pGoodGuyRightSprite;
	pGoodGuyUpSprite = nullptr;
	pGoodGuyDownSprite = nullptr;
	pGoodGuyLeftSprite = nullptr;
	pGoodGuyRightSprite = nullptr;

	delete pFlashUpSprite;
	delete pFlashDownSprite;
	delete pFlashLeftSprite;
	delete pFlashRightSprite;
	pFlashUpSprite = nullptr;
	pFlashDownSprite = nullptr;
	pFlashLeftSprite = nullptr;
	pFlashRightSprite = nullptr;

	for (nLoop1 = 0; nLoop1 < MAXLIVESPRITE; nLoop1++) {
		delete pLivesSprite[nLoop1];
		pLivesSprite[nLoop1] = nullptr;
	}

	delete pLiveSprite;
	pLiveSprite = nullptr;

	CSprite::ClearBackdrop();

	if (pGamePalette) {
		(*pGamePalette).DeleteObject();         // release the game color palette

		delete pGamePalette;
		pGamePalette = nullptr;
	}

	delete pOptionButton;
	pOptionButton = nullptr;
}


/*****************************************************************
 *
 * Power
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *
 * FORMAL PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      n/a
 *
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/

int CMainPackRatWindow::Power(int nBase, int nPower) {
	int nTemp1 = 1;
	int nTemp2 = 1;
	int nReturn = 1;
	int nLoop = 0;

	for (nLoop = 0; nLoop < nPower; nLoop++) {
		nTemp1 = nTemp2;
		nTemp2 = nTemp1 * nBase;

	}

	nReturn = nTemp2;
	return (nReturn);
}

/*****************************************************************
 *
 * FlushInputEvents
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Remove all keyboard and mouse related events from the message
 *  so that they will not be sent to us for processing; i.e. this
 *  flushes keyboard type ahead and extra mouse clicks and movement.
 *
 * FORMAL PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      n/a
 *
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/

void CMainPackRatWindow::FlushInputEvents() {
	MSG msg;

	while (true) {                                      // find and remove all keyboard events
		if (!PeekMessage(&msg, nullptr, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
			break;
	}

	while (true) {                                      // find and remove all mouse events
		if (!PeekMessage(&msg, nullptr, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
			break;
	}
}

//////////// Additional Sound Notify routines //////////////

LRESULT CMainPackRatWindow::OnMCINotify(WPARAM wParam, LPARAM lParam) {
	CSound  *pSound;

	pSound = CSound::OnMCIStopped(wParam, lParam);
	if (pSound != nullptr)
		OnSoundNotify(pSound);
	return 0;
}


LRESULT CMainPackRatWindow::OnMMIONotify(WPARAM wParam, LPARAM lParam) {
	CSound  *pSound;

	pSound = CSound::OnMMIOStopped(wParam, lParam);
	if (pSound != nullptr)
		OnSoundNotify(pSound);
	return 0;
}

void CMainPackRatWindow::OnSoundNotify(CSound *pSound) {
	//
	// Add your code to process explicit notification of a sound "done" event here.
	// pSound is a pointer to a CSound object for which you requested SOUND_NOTIFY.
	//
}

void CMainPackRatWindow::OnSysKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	switch (nChar) {
	// User has hit ALT_F4 so close down this App
	//
	case VK_F4:
		PostMessage(WM_CLOSE);
		break;

	default:
		CFrameWnd::OnSysKeyDown(nChar, nRepCnt, nFlags);
		break;
	}
}


// CMainPackRatWindow message map:
// Associate messages with member functions.
//
BEGIN_MESSAGE_MAP(CMainPackRatWindow, CFrameWnd)
	//{{AFX_MSG_MAP( CMainPackRatWindow )
	ON_WM_PAINT()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
	ON_WM_SYSKEYDOWN()
	ON_WM_ACTIVATE()
	ON_MESSAGE(MM_MCINOTIFY, CMainPackRatWindow::OnMCINotify)
	ON_MESSAGE(MM_WOM_DONE, CMainPackRatWindow::OnMMIONotify)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

} // namespace Packrat
} // namespace HodjNPodj
} // namespace Bagel
