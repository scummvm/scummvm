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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*

DEFINIZIONI MESSAGGI

*/

/*************************************************************************
* PRIORITA'                                                             *
*************************************************************************/
#define MP_SYSTEM  255                  /* PRIORITA' ASSOLUTA */

#define MP_LOW       1
#define MP_MED      64
#define MP_HIGH    128
#define MP_DEFAULT  MP_MED

/*************************************************************************
* CLASSI ED EVENTI                                                      *
*                                                                       *
* MC Message Classes Notify                                             *
* ME Message Events  Notify                                             *
*************************************************************************/
#define CLASS_GAME  63                  //   0- 63 --> coda GAME
#define CLASS_ANIM 127                  //  64-127 --> coda ANIM
#define CLASS_HOMO 255                  // 128-255 --> coda HOMO


/*************************************************************************
* IDLE                                                                  *
*************************************************************************/
#define MC_IDLE          0

/*************************************************************************
* MOUSE                                                                 *
*************************************************************************/
/* Mouse events */
#define MC_MOUSE         1

#define ME_MLEFT         0              // (mx,my,...)
#define ME_MRIGHT        1              // (mx,my,...)
#define ME_MTOGHETHER    2              // (mx,my,...)
#define ME_MMOVE         3              // (mx,my,direction,...)



/*************************************************************************
* SYSTEM                                                                *
*************************************************************************/
/* System events */
#define MC_SYSTEM        2

#define ME_QUIT          0
#define ME_START         1
#define ME_SAVEGAME      2
#define ME_LOADGAME      3
#define ME_CHANGEROOM    4              // (room,door,...)
#define ME_WAITANIMDEL   5
#define ME_WAITANIMADD   6
#define ME_REDRAWROOM	 7



/*************************************************************************
* Inventory                                                              *
*************************************************************************/
/* Inventory events */
#define MC_INVENTORY     3

#define ME_OPEN          0
#define ME_CLOSE         1
#define ME_PAINT         2
#define ME_DEPAINT       3
#define ME_ONELEFT       4
#define ME_ONERIGHT      5
#define ME_OPERATEICON   6              // (mx,my,0,0)
#define ME_EXAMINEICON   7              // (mx,my,0,0)
#define ME_SHOWICONNAME  8              // (mx,my,0,0)
#define ME_INVRESET      9

/*************************************************************************
* STRINGHE                                                              *
*************************************************************************/
/* String names */
#define MC_STRING           4

#define ME_HOMOSPEAK         2          // (...)
#define ME_HOMOSPEAKING      3          // (someone,)
#define ME_SOMEONESPEAKING   5          // (...)
#define ME_SOMEONEWAIT2SPEAK 7
#define ME_SOMEONEWAIT2MUTE  8

/*************************************************************************
* OPERAZIONI CON IL MOUSE                                               *
*************************************************************************/
/* Notification of mouse action */
#define MC_ACTION        5

#define ME_MOUSEOPERATE  0
#define ME_MOUSEEXAMINE  1
#define ME_INVOPERATE    2
#define ME_INVEXAMINE    3
#define ME_USEWITH       4

/*************************************************************************
* DIALOGHI                                                              *
*************************************************************************/
/* Notification of dialog event */
#define MC_DIALOG        6

#define ME_ENDSCELTA	 0
#define ME_STARTDIALOG	 1
/*
#define ME_RESETARG         0
#define ME_CLEARARG         1
#define ME_SHOWARG          2
#define ME_UPDATEARG        3
#define ME_STARTNEWDLG      4
#define ME_CLICKONARG      10
#define ME_WAITBATTUTA     23
#define ME_BATTUTA         20           //(battuta,...)
#define ME_FINEBATTUTA     21
#define ME_WAITFINEBATTUTA 22
#define ME_STARTBATTUTA    24
#define ME_BATTUTA2        25           //(battuta,...)
*/
/*************************************************************************
* PORTE                                                              *
*************************************************************************/
/* Notification of dialog event */
#define ME_INITOPENDOOR  0
#define ME_OPENDOOR      1
#define ME_WAITOPENDOOR  2

/*************************************************************************
* DOING                                                                 *
*************************************************************************/
/* Notification of doing event */
#define MC_DOING          8

#define ME_INITOPENCLOSE  0
#define ME_OPENCLOSE      1
#define ME_WAITOPENCLOSE  2

/*************************************************************************
* SCRIPT                                                                *
*************************************************************************/
#define C_RANIM   0                     // Room

#define C_ONAME   0                     // Object
#define C_OEXAMINE  1                   // Object
#define C_OACTION  2                    // Object
#define C_OGOROOM  3                    // Object
#define C_OMODE   4                     // Object
#define C_OFLAG   5                     // Object

#define C_INAME   0                     // InvObject
#define C_IEXAMINE  1                   // InvObject
#define C_IACTION  2                    // InvObject
#define C_IFLAG   3                     // InvObject

#define C_AFLAG   0                     // Anim
#define C_ADELAY  1                     // Anim

#define C_WAITASTART 1
#define C_WAITAEND  0

#define C_DDEFANI  0                    // Dialog
#define C_DLINKANI  1
#define C_DSPEAKANI  2
#define C_DLISTENANI 3
#define C_DSCELTADISATTIVATO 4

#define C_DADEFANI  0                   // Dialog Alternativo
#define C_DALINKANI  1
#define C_DASPEAKANI 2
#define C_DALISTENANI 3

#define C_SEMDIAG  0                    // Semafori

#define MC_SCRIPT  9
// (b,w1,w2,l)
#define ME_PAUSE  0                     // (0,time,0,0)
#define ME_SETROOM  1                   // (campo,indice,[bit/indice2c],valore)
#define ME_SETOBJ  2                    // (campo,indice,[bit/indice2c],valore)
#define ME_SETINVOBJ 3                  // (campo,indice,[bit/indice2c],valore)
#define ME_SETANIM  4                   // (campo,indice,[bit/indice2c],valore)
#define ME_ADDICON  5                   // (0,icona,0,0)
#define ME_PLAYDIALOG 6                 // (0,dialogo,0,0)
#define ME_SOMEONETALK 7                // (0,obj,anim,sentence)
#define ME_SETSEM  8                    // (0,indice,valore,0)
#define ME_SETBOX  9                    // (on/off,room,box,0)
#define ME_SETADIALOG 10                // (campo, 0, 0, valore)
#define ME_TORNAORBO 11
#define ME_SETDIALOG 12                 // (campo, indice, 0, valore)
#define ME_SETANIMAZIONESPENTA 13       // (0, 0, 0, valore)
#define ME_PLAYSOUND 14                 // (0,indice,0,0)
#define ME_STOPSOUND 15                 // (0,indice,0,0)
#define ME_HOMOSAY  16                  // (0,0,0,sentence)
#define ME_REGENROOM 17
#define ME_CHANGER  18                  // (0,room,door,0)
#define ME_KILLICON  19                 // (0,icona,0,0)

/*************************************************************************
* ANIMAZIONI                                                            *
*************************************************************************/
/* Notification of mouse action */
#define MC_ANIMATION    64

#define ME_SHOWANIM      0
#define ME_ADDANIM   1
#define ME_DELANIM       2

/*************************************************************************
* HOMO                                                                  *
*************************************************************************/
/* Notification of dialog event */
#define MC_HOMO          128

#define ME_HOMOSHOW        0
#define ME_HOMORESETMOVE   1
#define ME_HOMOGOTO        2
#define ME_HOMOGOTOACTION  3
#define ME_HOMOGOTOEXAMINE 4

#define ME_HOMOSTOP        5
#define ME_HOMOTAKE        6
#define ME_HOMOOPEN        7
#define ME_HOMOCLEARQUEUE  8

#define ME_HOMOACTION          9
#define ME_HOMOCONTINUEACTION 10
#define ME_HOMOGOTOEXIT       11
#define ME_HOMODOACTION		  12
