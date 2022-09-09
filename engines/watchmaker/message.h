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

#ifndef WATCHMAKER_MESSAGE_H
#define WATCHMAKER_MESSAGE_H

namespace Watchmaker {

// DEFINIZIONI MESSAGGI

// PRIORITA'
#define MP_DEFAULT  0                       // Deafault
#define MP_CLEARA   1                       // Fa partire tutti gli eventi che aspettano A
#define MP_WAITA    2                       // Aspetta A

#define MP_WAIT_LINK    4                   // Apetta di ricevere un link dall'animazione che da il tempo
#define MP_WAIT_ACT     8                   // Apetta la fine di una camminata o corsa o azione
#define MP_WAIT_ANIM    16                  // Apetta la fine di una animazione
#define MP_WAIT_CAMERA  32                  // Aspetta la fine del percorso della camera
#define MP_WAIT_PORTAL  64                  // Aspetta il passaggio del portale
#define MP_WAIT_RETRACE 128                 // Aspetta il Refresh del video

// PARAMETRI EXTRA
#define MPX_START_T2D_SAVE      1
#define MPX_START_T2D_LOAD      2
#define MPX_START_T2D_OPTIONS   3

// IDLE
#define ME_CURCHAR              254
#define ME_ALL                  255

// MOUSE
#define ME_MOUSEUPDATE          1
#define ME_MLEFT                2
#define ME_MRIGHT               3
#define ME_MOUSEHIDE            4
#define ME_MOUSEUNHIDE          5
#define ME_MLEFTPRESSED         6
#define ME_MRIGHTPRESSED        7

// SYSTEM
enum class EventClass {
	MC_IDLE = 0,
	MC_MOUSE = 1,
	MC_SYSTEM = 2,
	MC_CAMERA = 3,
	MC_STRING = 4,
	MC_ACTION = 5,
	MC_PLAYER = 6,
	MC_INVENTORY = 7,
	MC_DIALOG = 8,
	MC_ANIM = 9,
	MC_T2D = 10
};

enum SystemEvent {
	ME_QUIT =                1,
	ME_START =               2,
	ME_CHANGEROOM =          3,
	ME_CHANGEPLAYER =        4,
	ME_STARTEFFECT =         5,
	ME_CONTINUEEFFECT =      6,
	ME_STOPEFFECT =          7,
	ME_PLAYERTIMER =         8
};

// CAMERAS
#define ME_SETCAMERA            1
#define ME_CAMERAUPDATE         2
#define ME_SOURCEAT             3
#define ME_TARGETAT             4
#define ME_CHECKDISTANCE        5
#define ME_MOVECAMERATO         6
#define ME_INVALIDATECAMERA     7
#define ME_CAMERAGOTO           8
#define ME_SAVECAMERAPOS        9
#define ME_RESTORECAMERAPOS     10
#define ME_CAMERA3TO1           11
#define ME_CAMERA1TO3           12
#define ME_CAMERAPLAYER         13

// STRINGS
#define ME_PLAYERSPEAK          1
#define ME_PLAYERCONTINUESPEAK  2           //aspetta che finisca il tempo della frase
#define ME_PLAYERCONTINUESPEAK_WAITWAVE 3   //aspetta che finisca il suono associato alla frase

// ACTION
#define ME_MOUSEOPERATE         1
#define ME_MOUSEEXAMINE         2
#define ME_INVOPERATE           3
#define ME_INVEXAMINE           4
#define ME_USEWITH              5

// MAINC
#define ME_PLAYERGOTO           1
#define ME_PLAYERGOTOEXAMINE    2
#define ME_PLAYERGOTOACTION     3
#define ME_PLAYERGOTOEXIT       4
#define ME_PLAYERIDLE           5
#define ME_PLAYERGOTONOSKIP     6

// INVENTORY
#define ME_EXAMINEICON          1
#define ME_OPERATEICON          2
#define ME_INVOFF               3
#define ME_INVMODE1             4
#define ME_INVMODE2             5
#define ME_INVMODE3             6
#define ME_INVMODE4             7
#define ME_INVSWITCH            8

// DIALOG
#define ME_DIALOGSTART          1
#define ME_DIALOGCONTINUE       2
#define ME_DIALOGEND            3
#define ME_DIALOGEND_SKIPINTRO  4

// ANIM
#define ME_STARTANIM            1
#define ME_STOPANIM             2
#define ME_PAUSEANIM            3
#define ME_CONTINUEANIM         4

// T2D
#define ME_T2DSTART             1
#define ME_T2DSTOP              2
#define ME_T2DCONTINUE          3
#define ME_T2DSETBITMAP         4
#define ME_T2DSETBUTTON         5
#define ME_T2DSETWINDOW         6
#define ME_T2DSWAPWINDOWS       7

} // End of namespace Watchmaker

#endif // WATCHMAKER_MESSAGE_H
