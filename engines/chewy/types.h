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

#ifndef CHEWY_TYPES_H
#define CHEWY_TYPES_H

#include "chewy/defines.h"
#include "chewy/objekt.h"
#include "chewy/detail.h"

namespace Chewy {

struct Spieler {
	uint8 Ats[ROOM_ATS_MAX * 3];
	uint8 InvAts[MAX_MOV_OBJ * 3];
	uint8 InvUse[INV_USE_ATS_MAX * 3];
	uint8 InvUseDef[40 * 3];

	int16 MainMenuY;
	int16 InvDisp;
	int16 DispZx;
	int16 DispZy;
	int16 DispFlag;
	int16 InventY;
	int16 InventSlot[MAX_MOV_OBJ];

	int16 AkInvent;
	bool inv_cur;
	int16 CurBreite;
	int16 CurHoehe;

	RoomMovObjekt room_m_obj[MAX_MOV_OBJ];
	RoomStaticInventar room_s_obj[MAX_FEST_OBJ];
	RoomExit room_e_obj[MAX_EXIT];

	int16 X[MAX_PERSON];
	int16 Y[MAX_PERSON];
	int16 Phase[MAX_PERSON];
	int16 PersonHide[MAX_PERSON];
	int16 PersonRoomNr[MAX_PERSON];
	int16 ZoomXy[MAX_PERSON][2];

	int16 ChewyAni;

	int16 PersonGlobalDia[MAX_PERSON];
	int16 PersonDia[MAX_PERSON];

	int16 PersonDiaRoom[MAX_PERSON];

	int16 PersonDiaTmpRoom[MAX_PERSON];

	int16 DiaAMov;

	int16 scrollx;
	int16 scrolly;
	int16 ScrollxStep;
	int16 ScrollyStep;

	int16 MausSpeed;
	int16 DelaySpeed;
	int16 AadDelay;
	int16 AadSilent;

	uint16 R0SchleimWurf : 1;
	uint16 R0KissenWurf : 1;
	uint16 R0Monokel : 1;
	uint16 R0GBuch : 1;
	uint16 R2KabelBork : 1;
	uint16 R2FussSchleim : 1;
	uint16 R5Terminal : 1;
	uint16 R5Tuer : 1;
	uint16 R6BolaSchild : 1;
	uint16 R6BolaOk : 1;
	uint16 R6BolaBecher : 1;
	uint16 R6DoorLeftF : 1;
	uint16 R6DoorLeftB : 1;
	uint16 R6DoorRightB : 1;
	uint16 R7DoorRight : 1;
	uint16 R7Hebel : 1;
	uint16 R7SeilLeft : 1;
	uint16 R7SeilOk : 1;
	uint16 R7RHaken : 1;
	uint16 R7BorkFlug : 1;
	uint16 R7ChewyFlug : 1;
	uint16 R8Folter : 1;
	uint16 R8Stein : 1;
	uint16 R8GTuer : 1;
	uint16 R8Kohle : 1;
	uint16 R8Abkuehlen : 1;
	uint16 R8GipsWurf : 1;
	uint16 R9Gitter : 1;
	uint16 R9Surimy : 1;
	uint16 R10Surimy : 1;
	uint16 R10SurimyOk : 1;
	uint16 R11CardOk : 1;
	uint16 R11TerminalOk : 1;
	uint16 R11DoorRightB : 1;
	uint16 R11DoorRightF : 1;
	uint16 R12Betreten : 1;
	uint16 R12ChewyBork : 1;
	uint16 R12Talisman : 1;
	uint16 R12TalismanOk : 1;
	uint16 R12RaumOk : 1;
	uint16 R12BorkTalk : 1;
	uint16 R12BorkInRohr : 1;
	uint16 R12TransOn : 1;
	uint16 R12KetteLinks : 1;
	uint16 R13BorkOk : 1;
	uint16 R13Band : 1;
	uint16 R13Bandlauf : 1;
	uint16 R13Surf : 1;
	uint16 R14Feuer : 1;
	uint16 R14Translator : 1;
	uint16 R14FluxoFlex : 1;
	uint16 R14GleiterAuf : 1;
	uint16 R14Sicherung : 1;
	uint16 R14Waffe : 1;
	uint16 R16F5Exit : 1;
	uint16 R17GitterWeg : 1;
	uint16 R17Seil : 1;
	uint16 R17HebelOk : 1;
	uint16 R17EnergieOut : 1;
	uint16 R17DoorKommand : 1;
	uint16 R18DoorBruecke : 1;
	uint16 R18CartFach : 1;
	uint16 R18CartTerminal : 1;
	uint16 R18CartSave : 1;
	uint16 R18SurimyWurf : 1;
	uint16 R18SondeMoni : 1;
	uint16 R18MoniSwitch : 1;
	uint16 R18Krone : 1;
	uint16 R18FirstEntry : 1;
	uint16 R18Gitter : 1;
	uint16 R21Salto : 1;
	uint16 R21GitterEnergie : 1;
	uint16 R21GitterMuell : 1;
	uint16 R21Hebel1 : 1;
	uint16 R21Hebel2 : 1;
	uint16 R21Hebel3 : 1;
	uint16 R21Laser1Weg : 1;
	uint16 R21Laser2Weg : 1;
	uint16 R21Gang : 1;
	uint16 R22Paint : 1;
	uint16 R22ChewyPlatt : 1;
	uint16 R22BorkPlatt : 1;
	uint16 R22GetBork : 1;
	uint16 R23FluxoFlex : 1;
	uint16 R23Cartridge : 1;
	uint16 R24FirstEntry : 1;
	uint16 R25FirstEntry : 1;
	uint16 R25GleiteLoesch : 1;
	uint16 R25SurimyLauf : 1;
	uint16 R25GleiterExit: 1;
	uint16 R27SurimyOk : 1;
	uint16 R28SurimyCar : 1;
	uint16 R28ChewyPump : 1;
	uint16 R28Briefkasten: 1;
	uint16 R28EntryHaus : 1;
	uint16 R28Manuskript : 1;

	uint16 R28RKuerbis : 1;
	uint16 R28ExitTown : 1;

	uint16 R28PostCar : 1;
	uint16 R29Pumpe : 1;
	uint16 R29Schlauch1 : 1;
	uint16 R29Schlauch2 : 1;
	uint16 R29AutoSitz : 1;
	uint16 R31PflanzeWeg : 1;
	uint16 R31KoernerDa : 1;
	uint16 R31Wasser : 1;
	uint16 R31KlappeZu : 1;
	uint16 R31SurFurz : 1;
	uint16 R32HowardWeg : 1;
	uint16 R32UseSchreib : 1;
	uint16 R32PapierOk : 1;
	uint16 R32Script : 1;
	uint16 R33SchubFirst : 1;
	uint16 R33Messer : 1;
	uint16 R33MunterOk : 1;
	uint16 R33MunterGet : 1;
	uint16 R35TransCat : 1;
	uint16 R35Falle : 1;
	uint16 R35Schublade : 1;
	uint16 R35CatEat : 1;
	uint16 R37TransHahn : 1;
	uint16 R37Gebiss : 1;
	uint16 R37HundScham : 1;
	uint16 R37Kloppe : 1;
	uint16 R37Mes : 1;
	uint16 R39TransMensch: 1;
	uint16 R39TvOn : 1;
	uint16 R39ScriptOk : 1;
	uint16 R39HowardDa : 1;
	uint16 R39HowardWach : 1;
	uint16 R40TeilKarte : 1;
	uint16 R40TrainMove : 1;

	uint16 R40TrainOk : 1;
	uint16 R40Geld : 1;
	uint16 R40PoliceStart: 1;
	uint16 R40PoliceWeg : 1;
	uint16 R40PoliceAb : 1;
	uint16 R40Wettbewerb : 1;
	uint16 R40HoUse : 1;
	uint16 R40HaendlerOk : 1;
	uint16 R40DuengerMit : 1;
	uint16 R40DuengerTele: 1;
	uint16 R41FirstTalk : 1;
	uint16 R41LolaOk : 1;
	uint16 R41Einbruch : 1;
	uint16 R41BruchInfo : 1;
	uint16 R41KuerbisInfo: 1;
	uint16 R41RepairInfo : 1;
	uint16 R41HowardDiaOK: 1;
	uint16 R42FirstEntry : 1;
	uint16 R42BriefOk : 1;
	uint16 R42HoToBeamter: 1;
	uint16 R42MarkeOk : 1;
	uint16 R42BriefMarke : 1;
	uint16 R42BeamterWach: 1;
	uint16 R43GetPgLady : 1;
	uint16 R45TaxiOk : 1;
	uint16 R45MagOk : 1;
	uint16 R46GetLeder : 1;
	uint16 R47SchlossOk : 1;
	uint16 R48TaxiEntry : 1;
	uint16 R49WegFrei : 1;
	uint16 R49BoyWeg : 1;
	uint16 R49BoyAni : 1;
	uint16 R50Zigarre : 1;
	uint16 R50KeyOK : 1;
	uint16 R51FirstEntry : 1;
	uint16 R51HotelRoom : 1;
	uint16 R51KillerWeg : 1;
	uint16 R52HotDogOk : 1;
	uint16 R52TuerAuf : 1;
	uint16 R52LichtAn : 1;
	uint16 R52KakerJoke : 1;
	uint16 R52KakerWeg : 1;
	uint16 R53Kostuem : 1;
	uint16 R53Visit : 1;
	uint16 R54HotDogOk : 1;
	uint16 R54FputzerWeg : 1;
	uint16 R54LiftOk : 1;
	uint16 R54Schild : 1;
	uint16 R55SekWeg : 1;
	uint16 R55Location : 1;

	uint16 R55Entry : 1;
	uint16 R55Job : 1;
	uint16 R55ScriptWeg : 1;
	uint16 R55EscScriptOk: 1;
	uint16 R55RaumOk : 1;
	uint16 R55R54First : 1;

	uint16 R56GetTabak : 1;
	uint16 R56AbfahrtOk : 1;
	uint16 R56Kneipe : 1;
	uint16 R56WhiskyMix : 1;
	uint16 R57StudioAuf : 1;
	uint16 R59PosterWeg : 1;
	uint16 R62Flucht : 1;
	uint16 R62FirstEntry : 1;
	uint16 R62TShow : 1;
	uint16 R62LauraVerwandlung: 1;
	uint16 R63Feuer : 1;
	uint16 R63FxMannWeg : 1;
	uint16 R63Uhr : 1;
	uint16 R64ManWeg : 1;
	uint16 R67LiedOk : 1;
	uint16 R67TuerOk : 1;
	uint16 R67KommodeAuf : 1;
	uint16 R67KostuemWeg : 1;
	uint16 R67PapageiWeg : 1;
	uint16 R68KarteDa : 1;
	uint16 R68Papagei : 1;
	uint16 R68DivaWeg : 1;
	uint16 R68Lied : 1;
	uint16 R68IndigoDia : 1;
	uint16 R68Gutschein : 1;

	uint8 R0FueterLab;
	uint8 R6RaumBetreten;

	uint8 R6BolaJoke;
	uint8 R7BellCount;
	uint8 R11IdCardNr;
	uint8 R12BorkCount;

	uint8 R13MonitorStatus;

	uint8 R17Location;

	int16 R23GleiterExit;

	uint8 R24Hebel[3];

	uint8 R24HebelDir[3];

	uint8 R24KristallLast[3];
	uint8 R25SurimyGo;
	uint8 R27HowardGed;
	uint8 R28PumpTxt1;
	uint8 R28PumpTxt;

	uint8 R31SurimyGo;
	uint8 R33SurimyGo;
	uint8 R33Munter[4];

	int16 R39TvKanal;
	uint8 R39TvRecord;
	uint8 R39ClintNews;

	uint8 R40PoliceAniStatus;
	uint8 R41TrainCount;

	uint8 R47Schloss[3];
	uint8 R48TaxiPerson[MAX_PERSON];

	uint8 R48Auswahl[5];

	uint8 R49BoyAniCount;
	uint8 R51DoorCount;
	uint8 R54LiftCount;
	uint8 R54HowardVorne;

	short R55ExitDia;

	int16 R58TmpRoom;
	int16 R58TmpRoom1;
	int16 R58TmpSx;
	int16 R58TmpSy;
	int16 R58TmpX;
	int16 R58TmpY;
	int16 R64Moni1Ani;

	int16 R64Moni2Ani;

	uint16 UDummy2 : 1;
	uint16 UDummy3 : 1;
	uint16 UDummy4 : 1;
	uint16 UDummy5 : 1;
	uint16 UDummy6 : 1;
	uint16 UDummy7 : 1;
	uint16 UDummy8 : 1;
	uint16 UDummy9 : 1;
	uint16 UDummy10 : 1;
	uint16 UDummy11 : 1;
	uint16 UDummy12 : 1;
	uint16 UDummy13 : 1;
	uint16 UDummy14 : 1;
	uint16 UDummy15 : 1;
	uint16 UDummy16 : 1;
	uint16 UDummy17 : 1;
	uint16 UDummy18 : 1;
	uint16 UDummy19 : 1;
	uint16 UDummy20 : 1;
	uint8 BDummy1;
	uint8 BDummy2;
	uint8 BDummy3;
	uint8 BDummy4;
	uint8 BDummy5;
	uint8 BDummy6;
	uint8 BDummy7;
	uint8 BDummy8;
	uint8 BDummy9;
	uint8 BDummy10;
	int16 SDummy1;
	int16 SDummy2;
	int16 SDummy3;
	int16 SDummy4;
	int16 SDummy5;
	int16 SDummy6;
	int16 SDummy7;
	int16 SDummy8;
	int16 SDummy9;
	int16 SDummy10;
	bool SoundSwitch;
	uint8 SoundVol;
	bool MusicSwitch;
	uint8 MusicVol;
	bool SpeechSwitch;
	uint8 FramesPerSecond;
	bool DisplayText;
};

struct AutoMov {
	int16 X;
	int16 Y;
};

struct MovPhasen {
	int16 AtsText;
	int16 Phase[4][2];

	int16 Lines;
	int16 Repeat;

	uint8 ZoomFak;
	uint8 Start;
};

struct MovLine {
	int16 EndXyz[3];
	uint8 PhNr;
	uint8 Vorschub;
};

struct AniBlock {
	int16 Nr;
	uint8 Repeat;
	uint8 Dir;

	uint8 Mode;

	uint8 Dummy;
};

struct Flags {
	uint16 StopAutoObj : 1;
	uint16 AniUserAction : 1;
	uint16 AutoAniPlay : 1;
	uint16 StaticUseTxt : 1;
	uint16 GedAction : 1;
	uint16 ChAutoMov : 1;
	uint16 ChAutoMovXy : 1;
	uint16 BreakAMov : 1;
	uint16 AtsAction : 1;
	uint16 AdsAction : 1;
	uint16 AtsText : 1;
	uint16 AdsDialog : 1;
	uint16 ShowAtsInvTxt : 1;
	uint16 MainInput : 1;
	uint16 InventMenu : 1;
	uint16 NoScroll : 1;
	bool InitSound : true;
	uint16 ExitMov : 1;
	uint16 MausTxt : 1;
	uint16 LoadGame : 1;
	uint16 main_maus_flag: 1;
	uint16 ZoomMov : 1;
	uint16 IcmEntry : 1;
	uint16 SaveMenu : 1;
	uint16 SpzAni : 1;
	uint16 MausLinks : 1;
	uint16 ChewyDontGo : 1;
	uint16 NoPalAfterFlc : 1;
	uint16 NoDiaBox : 1;
	uint16 NoEndPosMovObj: 1;
	uint16 NoShad : 1;
	uint16 SavePersonRnr : 1;
	uint16 CursorStatus : 1;
};

} // namespace Chewy

#endif
