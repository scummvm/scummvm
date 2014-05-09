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

/*
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janus B. Wisniewski and L.K. Avalon
 */

#ifndef CGE2_SNAIL_H
#define CGE2_SNAIL_H

#include "cge2/cge2_main.h"

namespace CGE2 {

enum Action { kNear, kMTake, kFTake, kActions };

enum SNCom {
	kSNCom0 = 128,
	kSNNop,     // NOP                  instrukcja "pusta"                        :: do nothing
	kSNUse,     // USE <spr> <cav>|<lab> hint for using
	kSNPause,   // PAUSE -1 <dly>       oczekiwanie <dly>/72 sekund               :: delay <dly>/72 seconds
	kSNInf,     // INF -1 <ref>         prezentacja tekstu o numerze <ref>        :: show text referrenced by <ref>
	kSNCave,    // CAVE -1 <cav>        przejœcie na planszê <cav>                :: go to board <cav>
	kSNSlave,   // SLAVE
	kSNFocus,   // FOCUS                zmiana aktywnego bohatera (-1 cyklicznie) :: change active hero
	kSNSetX,    // SETX <x> <idx>       ustawienie przesuniêcia sprajtu           :: set sprite shift in x axis
	kSNSetY,    // SETX <y> <idx>       ustawienie przesuniêcia sprajtu           :: set sprite shift in y axis
	kSNSetZ,    // SETX <z> <idx>       ustawienie przesuniêcia sprajtu           :: set sprite shift in z axis
	kSNAdd,     // ADD <idx1> <idx2>    suma dwóch wektorów                       :: sum vectors
	kSNSub,     // SUB <idx1> <idx2>    ró¿nica dwóch wektorów                    :: subtract vectors
	kSNMul,     // MUL <idx> <nr>       iloczyn wektora przez liczbê              :: multiply vector by number
	kSNDiv,     // DIV <idx> <nr>       iloraz wektora przez liczbê               :: divide vector by number
	kSNIf,      // IF
	kSNFlag,    // FLAG <nr> <val>      nadanie wartoœci znacznikowi <nr>         :: set flag <nr> to <val>
	kSNFlash,   // FLASH -1 0|1         rozjaœnienie ca³ego obrazu (tak/nie)      :: lighten whole image (on/off)
	kSNLight,   // LIGHT
	kSNCycle,   // CYCLE <cnt>          cykliczne przesuwanie <cnt> kolorow od 1  :: rotate <cnt> colors from 1
	kSNClear,   // CLEAR -1 0           wyzerowanie kolejki kSNAIL                 :: clear kSNAIL queue
	kSNTalk,    // TALK -1 0|1          zezwolenie na dialogi (tak/nie)           :: enable speach (on/off)
	kSNMouse,   // MOUSE -1 0|1         zezwolenie na mysz (tak/nie)              :: enable mouse (on/off)
	kSNMap,     // MAP  0|1 0           chwilowe wylaczenie mapy dla bohatera     :: temporarily turn off map for hero
	kSNCount,   // COUNT
	kSNMidi,    // MIDI -1 <midi>       muzyka MIDI nr <midi> (-1 = cisza)        :: play MIDI referenced by <midi> (-1 = off)
	kSNSetDlg,  // SETDLG 0..3 0..3     przelaczenie trybu mowy                   :: switch of speach mode
	kSNMskDlg,  // MSKDLG 0..3 0..3     przelaczenie maski trybu mowy             :: switch of speach mode mask

	kSNSpr,

	kSNWait,    // WAIT <spr> <seq>|-1  oczekiwanie na SEQ <seq> (-1 = stoi)      :: wait for SEQ <seq> (-1 = freeze)
	kSNHide,    // HIDE <spr> 0|1       widzialnoœæ sprajtu                       :: visibility of sprite
	kSNRoom,    // ROOM <hero> 0|1      dodatkowe miejsce w kieszeni (nie/tak)    :: additional room in pocket (no/yes)
	kSNSay,     // SAY <spr> <ref>      wyg³oszenie tekstu o numerze <ref>        :: say text referenced by <ref>
	kSNSound,   // SOUND <spr> <ref>    wyemitowanie efektu <ref>                 :: play sound effect referenced by <ref>
	kSNTime,    // TIME <spr> 0         wyg³oszenie bie¿¹cego czasu               :: say current time
	kSNKill,    // KILL <spr> 0         usuniêcie sprajtu                         :: remove sprite
	kSNRSeq,    // RSEQ <spr> <nr>      wzglêdny skok SEQ o <nr>                  :: relative jump SEQ <nr> lines
	kSNSeq,     // SEQ <spr> <seq>      skok do SEQ <seq>                         :: jump to certain SEQ
	kSNSend,    // SEND <spr> <cav>     przeniesienie sprajtu na planszê <cav>    :: move sprite to board <cav>
	kSNSwap,    // SWAP <spr1> spr2>    zamiana sprajtów                          :: sprite exchange
	kSNKeep,    // KEEP <spr> <seq>     sprajt do kieszeni ze skokiem do <seq>    :: take sprite into pocket and jump to <seq>
	kSNGive,    // GIVE <spr> <seq>     sprajt z kieszeni ze skokiem do <seq>     :: remove sprite from pocket and jump to <seq>
	kSNGetPos,  // GETPOS <spr> <idx>   pobranie pozycji sprajtu                  :: take sprite's position
	kSNGoto,    // GOTO <spr> <idx>     przesuniêcie sprajtu na dana pozycje      :: move sprite to position
	kSNMoveX,   // MOVEX <spr> <dx>     przesuniêcie wzglêdne po osi X            :: relative move along X axis
	kSNMoveY,   // MOVEY <spr> <dy>     przesuniêcie wzglêdne po osi Y            :: relative move along Y axis
	kSNMoveZ,   // MOVEZ <spr> <dz>     przesuniêcie wzglêdne po osi Z            :: relative move along Z axis
	kSNTrans,   // TRANS <spr> 0|1      ustalenie przezroczystoœci logicznej      :: clear/set logical transparency
	kSNPort,    // PORT <spr> 0|1       ustalenie "bralnoœci" sprajtu             :: clear/set "takeability" of sprite
	kSNNext,    // NEXT <spr> <nr>      skok do <nr> - NEAR lub TAKE              :: jump to <nr> - NEAR or TAKE
	kSNNNext,   // NNEXT <spr> <nr>     skok do <nr> - NEAR                       :: jump to <nr> - NEAR
	kSNMTNext,  // MTNEXT <spr> <nr>    skok do <nr> - TAKE                       :: jump to <nr> - TAKE
	kSNFTNext,  // FTNEXT <spr> <nr>    skok do <nr> - TAKE                       :: jump to <nr> - TAKE
	kSNRNNext,  // RNNEXT <spr> <nr>    skok wzglêdny do <nr> - NEAR              :: relative jump to <nr> - NEAR
	kSNRMTNext, // RMTNEXT <spr> <nr>   skok wzglêdny do <nr> - TAKE              :: relative jump to <nr> - TAKE
	kSNRFTNext, // RFTNEXT <spr> <nr>   skok wzglêdny do <nr> - TAKE              :: relative jump to <nr> - TAKE
	kSNRMNear,  // RMNEAR <spr> 0       usuniêcie listy NEAR                      :: remove NEAR list
	kSNRMMTake, // RMMTAKE <spr> 0      usuniêcie listy TAKE                      :: remove TAKE list
	kSNRMFTake, // RMFTAKE <spr> 0      usuniêcie listy TAKE                      :: remove TAKE list
	kSNSetRef,  // RETREF <spr> <ref>   zmiana numeru <ref> sprajta <spr>         :: change reference of sprite <spr> to <ref> 
	kSNBackPt,  // BACKPT <spr> 0       wmalowanie sprajtu w t³o                  :: paint sprite onto the background
	kSNWalk,    // WALK <hero> <ref>|<point> podejd¿ do sprajtu lub punktu        :: go close to the sprite or point
	kSNReach,   // REACH <hero> <ref>|<m> siêgnij do <ref> lub sposobem <m>       :: reach the sprite or point with <m> method
	kSNCover,   // COVER <sp1> <sp2>    przykrycie sprajtu <sp1> sprajtem <sp2>   :: cover sprite <sp1> with sprite <sp2>
	kSNUncover, // UNCOVER <sp1> <sp2>  odtworzenie sytuacji sprzed COVER         :: restore the state before COVER

	kSNDim,
	kSNExec,
	kSNStep,
	kSNGhost,
	
	kSNNOne = kNoByte
};

class Snail {
	static const char *comTxt[];
public:
	static int com(const char *com);
};

} // End of namespace CGE2

#endif
