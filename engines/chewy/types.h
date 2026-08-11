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

#ifndef CHEWY_TYPES_H
#define CHEWY_TYPES_H

#include "common/serializer.h"
#include "chewy/defines.h"
#include "chewy/object_extra.h"

namespace Chewy {

#include "common/pack-start.h"	// START STRUCT PACKING

struct GameFlags {
	bool R0SlimeUsed : 1;
	bool R0PillowThrow : 1;
	bool R0Monocle : 1;
	bool R0Noteboook : 1;
	bool R2ElectrocutedBork : 1;
	bool R2FussSchleim : 1;
	bool R5Terminal : 1;
	bool R5Door : 1;

	bool R6BolaSchild : 1;
	bool R6BolaOk : 1;
	bool R6BolaBecher : 1;
	bool R6DoorLeftF : 1;
	bool R6DoorLeftB : 1;
	bool R6DoorRightB : 1;
	bool R7DoorRight : 1;
	bool R7Lever : 1;

	bool R7RopeLeft : 1;
	bool R7RopeOk : 1;
	bool R7RHaken : 1;
	bool R7BorkFlug : 1;
	bool R7ChewyFlug : 1;
	bool R8Folter : 1;
	bool R8Stein : 1;
	bool R8GTuer : 1;

	bool R8Kohle : 1;
	bool R8Abkuehlen : 1;
	bool R8GipsWurf : 1;
	bool R9Grid : 1;
	bool R9Surimy : 1;
	bool R10Surimy : 1;
	bool R10SurimyOk : 1;
	bool R11CardOk : 1;

	bool R11TerminalOk : 1;
	bool R11DoorRightB : 1;
	bool R11DoorRightF : 1;
	bool R12Betreten : 1;
	bool R12ChewyBork : 1;
	bool R12Talisman : 1;
	bool R12TalismanOk : 1;
	bool R12RaumOk : 1;

	bool R12BorkTalk : 1;
	bool R12BorkInRohr : 1;
	bool R12TransOn : 1;
	bool R12ChainLeft : 1;
	bool R13BorkOk : 1;
	bool R13Band : 1;
	bool R13Bandlauf : 1;
	bool R13Surf : 1;

	bool R14Feuer : 1;
	bool R14Translator : 1;
	bool R14FluxoFlex : 1;
	bool R14GleiterAuf : 1;
	bool R14Sicherung : 1;
	bool R14Waffe : 1;
	bool R16F5Exit : 1;
	bool R17GridWeg : 1;

	bool R17Rope : 1;
	bool R17LeverOk : 1;
	bool R17EnergyOut : 1;
	bool R17DoorCommand : 1;
	bool R18DoorBridge : 1;
	bool R18CartridgeInSlot : 1;
	bool R18CartTerminal : 1;
	bool R18CartSave : 1;

	bool R18SurimyThrown : 1;
	bool R18SondeMoni : 1;
	bool R18MonitorSwitch : 1;
	bool R18Crown : 1;
	bool R18FirstEntry : 1;
	bool R18Grid : 1;
	bool R21Salto : 1;
	bool R21EnergyGrid : 1;

	bool R21GarbageGrid : 1;
	bool R21Lever1 : 1;
	bool R21Lever2 : 1;
	bool R21Lever3 : 1;
	bool R21Laser1Weg : 1;
	bool R21Laser2Weg : 1;
	bool R21Gang : 1;
	bool R22Paint : 1;

	bool R22ChewyPlatt : 1;
	bool R22BorkPlatt : 1;
	bool R22GetBork : 1;
	bool R23FluxoFlex : 1;
	bool R23Cartridge : 1;
	bool R24FirstEntry : 1;
	bool R25FirstEntry : 1;
	bool R25GliderFlamesExtinguished : 1;

	bool R25SurimyLauf : 1;
	bool R25GliderExit : 1;
	bool R27SurimyOk : 1;
	bool R28SurimyCar : 1;
	bool R28ChewyPump : 1;
	bool R28LetterBox : 1;
	bool R28EntryHaus : 1;
	bool R28Manuskript : 1;

	bool R28RKuerbis : 1;
	bool R28ExitTown : 1;
	bool R28PostCar : 1;
	bool R29Pumpe : 1;
	bool R29Schlauch1 : 1;
	bool R29WaterHose : 1;
	bool R29AutoSitz : 1;
	bool R31PflanzeWeg : 1;

	bool R31KoernerDa : 1;
	bool R31Wasser : 1;
	bool R31KlappeZu : 1;
	bool R31SurFurz : 1;
	bool R32HowardWeg : 1;
	bool R32UseSchreib : 1;
	bool R32PapierOk : 1;
	bool R32Script : 1;

	bool R33SchubFirst : 1;
	bool R33Messer : 1;
	bool R33MunterOk : 1;
	bool R33MunterGet : 1;
	bool R35TransCat : 1;
	bool R35Falle : 1;
	bool R35Schublade : 1;
	bool R35CatEat : 1;

	bool R37UsedTranslatorOnRooster : 1;
	bool R37TakenDenturesFromGlass : 1;
	bool R37DogAshamed : 1;
	bool R37RoosterFoughtWithDog : 1;
	bool R37Mes : 1;
	bool R39TranslatorUsed : 1;
	bool R39TvOn : 1;
	bool R39ScriptOk : 1;

	bool R39HowardDa : 1;
	bool R39HowardWach : 1;
	bool R40TeilKarte : 1;
	bool R40TrainMove : 1;
	bool R40TrainOk : 1;
	bool R40Geld : 1;
	bool R40PoliceStart : 1;
	bool R40PoliceAway : 1;

	bool R40PoliceAb : 1;
	bool R40Wettbewerb : 1;
	bool R40HoUse : 1;
	bool R40HaendlerOk : 1;
	bool R40DuengerMit : 1;
	bool R40DuengerTele : 1;
	bool R41FirstTalk : 1;
	bool R41LolaOk : 1;

	bool R41Einbruch : 1;
	bool R41BruchInfo : 1;
	bool R41KuerbisInfo : 1;
	bool R41RepairInfo : 1;
	bool R41HowardDiaOK : 1;
	bool R42FirstEntry : 1;
	bool R42LetterOk : 1;
	bool R42HoToBeamter : 1;

	bool R42StampOk : 1;
	bool R42LetterStamped : 1;
	bool R42StationEmployeeAway : 1;
	bool R43GetPgLady : 1;
	bool R45TaxiOk : 1;
	bool R45MagOk : 1;
	bool R46GetLeder : 1;
	bool R47SchlossOk : 1;

	bool R48TaxiEntry : 1;
	bool R49WegFrei : 1;
	bool R49BoyWeg : 1;
	bool R49BoyAni : 1;
	bool R50Zigarre : 1;
	bool R50KeyOK : 1;
	bool R51FirstEntry : 1;
	bool R51HotelRoom : 1;

	bool R51KillerWeg : 1;
	bool R52HotDogOk : 1;
	bool R52TuerAuf : 1;
	bool R52LichtAn : 1;
	bool R52KakerJoke : 1;
	bool R52KakerWeg : 1;
	bool R53Kostuem : 1;
	bool R53Visit : 1;

	bool R54HotDogOk : 1;
	bool R54FputzerWeg : 1;
	bool R54LiftOk : 1;
	bool R54Schild : 1;
	bool R55SekWeg : 1;
	bool R55Location : 1;
	bool R55Entry : 1;
	bool R55Job : 1;

	bool R55ScriptWeg : 1;
	bool R55EscScriptOk : 1;
	bool R55RaumOk : 1;
	bool R55R54First : 1;
	bool R56GetTabak : 1;
	bool R56AbfahrtOk : 1;
	bool R56Kneipe : 1;
	bool R56WhiskyMix : 1;

	// flags 25
	bool R57StudioAuf : 1;
	bool R59PosterWeg : 1;
	bool R62Flucht : 1;
	bool R62FirstEntry : 1;
	bool R62TShow : 1;
	bool R62LauraTransformation : 1;
	bool R63Fire : 1;
	bool R63FxManAway : 1;

	bool R63Uhr : 1;
	bool R64ManAway : 1;
	bool flags26_4 : 1;
	bool flags26_8 : 1;
	bool changedArtifactOrigin : 1;
	bool flags26_20 : 1;
	bool flags26_40 : 1;
	bool R67SongOk : 1;

	bool R67KommodeAuf : 1;
	bool R67KostuemWeg : 1;
	bool R67PapageiWeg : 1;
	bool R68KarteDa : 1;
	bool R68Parrot : 1;
	bool R68DivaAway : 1;
	bool R68Song : 1;
	bool R68IndigoDia : 1;

	bool R68DrinkCoupon : 1;
	bool R71LeopardVined : 1;
	bool flags28_4 : 1;
	bool flags28_8 : 1;
	bool flags28_10 : 1;
	bool flags28_20 : 1;	// bushes moved
	bool flags28_40 : 1;	// machete picked up
	bool R74CutRubberPlant : 1;

	bool flags29_1 : 1;
	bool flags29_2 : 1;
	bool flags29_4 : 1;
	bool flags29_8 : 1;
	bool flags29_10 : 1;
	bool flags29_20 : 1;
	bool flags29_40 : 1;
	bool flags29_80 : 1;

	bool flags30_1 : 1;
	bool flags30_2 : 1;
	bool flags30_4 : 1;
	bool flags30_8 : 1;
	bool flags30_10 : 1;
	bool flags30_20 : 1;
	bool flags30_40 : 1;
	bool flags30_80 : 1;

	bool R88UsedMonkey : 1;
	bool gottenDiary : 1;
	bool R84GoonsPresent : 1;
	bool flags31_8 : 1;
	bool flags31_10 : 1;
	byte flags31_20 : 1;
	byte flags31_40 : 1;
	byte flags31_80 : 1;

	// Further flags
	bool flags32_1 : 1;
	bool flags32_2 : 1;
	bool flags32_4 : 1;
	bool flags32_8 : 1;
	bool flags32_10 : 1;
	bool flags32_20 : 1;
	bool flags32_40 : 1;
	bool flags32_80 : 1;

	bool flags33_1 : 1;
	bool flags33_2 : 1;
	bool flags33_4 : 1;
	bool flags33_8 : 1;
	bool flags33_10 : 1;
	bool flags33_20 : 1;
	bool flags33_40 : 1;
	bool flags33_80 : 1;

	bool flags34_1 : 1;
	bool flags34_2 : 1;
	bool flags34_4 : 1;
	bool flags34_8 : 1;
	bool flags34_10 : 1;
	bool flags34_20 : 1;
	bool flags34_40 : 1;
	bool flags34_80 : 1;

	bool flags35_1 : 1;
	bool flags35_2 : 1;
	bool flags35_4 : 1;
	bool flags35_8 : 1;
	bool flags35_10 : 1;
	bool flags35_20 : 1;
	bool flags35_40 : 1;
	bool flags35_80 : 1;

	bool flags36_1 : 1;
	bool flags36_2 : 1;
	bool flags36_4 : 1;
	bool flags36_8 : 1;
	bool flags36_10 : 1;
	bool flags36_20 : 1;
	bool flags36_40 : 1;
	bool flags36_80 : 1;

	bool flags37_1 : 1;
	bool flags37_2 : 1;
	bool flags37_4 : 1;
	bool flags37_8 : 1;
	bool flags37_10 : 1;
	bool flags37_20 : 1;
	bool flags37_40 : 1;
	bool flags37_80 : 1;

	bool R64AshtrayTaken : 1;	// picked up ashtray in TV room
	bool flags38_2 : 1;
	uint8 flags38_unused : 6;

	GameFlags() { clear(); }
	void clear();
} PACKED_STRUCT;
#include "common/pack-end.h"	// END STRUCT PACKING

struct GameState : public GameFlags {
	GameState() : GameFlags(), _flags(this) {
		_flags->clear();
	}

	/**
	 * Clears the game state
	 */
	void clear();

	/**
	 * For loading or saving the structure data
	 */
	bool synchronize(Common::Serializer &s);

	GameFlags *_flags = nullptr;

	int16 MainMenuY = 0;
	int16 InvDisp = 0;
	int16 DispZx = 0;
	int16 DispZy = 0;
	int16 DispFlag = 0;
	int16 InventY = 0;
	int16 InventSlot[MAX_MOV_OBJ] = { 0 };

	RoomMovObject room_m_obj[MAX_MOV_OBJ];
	RoomStaticInventory room_s_obj[MAX_FEST_OBJ];
	RoomExit room_e_obj[MAX_EXIT];
	int16 X[MAX_PERSON] = { 0 };
	int16 Y[MAX_PERSON] = { 0 };
	int16 Phase[MAX_PERSON] = { 0 };
	int16 _personHide[MAX_PERSON] = { 0 };
	int16 _personRoomNr[MAX_PERSON] = { 0 };
	int16 ZoomXy[MAX_PERSON][2] = {{ 0 }};
	int16 PersonGlobalDia[MAX_PERSON] = { 0 };
	int16 PersonDia[MAX_PERSON] = { 0 };
	int16 PersonDiaRoom[MAX_PERSON] = { 0 };
	int16 PersonDiaTmpRoom[MAX_PERSON] = { 0 };

	int16 ChewyAni = 0;
	int16 DiaAMov = 0;
	int16 scrollx = 0;
	int16 scrolly = 0;
	int16 ScrollxStep = 0;
	int16 ScrollyStep = 0;
	int16 DelaySpeed = 0;
	int16 AadDelay = 0;
	int16 AadSilent = 0;

	uint8 R0FueterLab = 0;
	uint8 R6RaumBetreten = 0;
	uint8 R6BolaJoke = 0;
	uint8 R7BellCount = 0;
	uint8 R11IdCardNr = 0;
	uint8 R12BorkCount = 0;

	uint8 R13MonitorStatus = 0;

	uint8 R17Location = 0;

	int16 R23GliderExit = 0;

	uint8 R24Lever[3] = { 0 };
	uint8 R24LeverDir[3] = { 0 };
	uint8 R24CrystalLast[3] = { 0 };
	uint8 R25SurimyGo = 0;
	uint8 R27HowardGed = 0;
	uint8 R28PumpTxt1 = 0;
	uint8 R28PumpTxt = 0;

	uint8 R31SurimyGo = 0;
	uint8 R33SurimyGo = 0;
	uint8 R33Munter[4] = { 0 };

	int16 R39TvKanal = 0;
	uint8 R39TvRecord = 0;
	uint8 R39ClintNews = 0;

	uint8 R40PoliceAniStatus = 0;
	uint8 R41TrainCount = 0;

	uint8 R47Schloss[3] = { 0 };
	uint8 R48TaxiPerson[MAX_PERSON] = { 0 };
	uint8 R48Auswahl[5] = { 0 };

	uint8 R49BoyAniCount = 0;
	uint8 R51DoorCount = 0;
	uint8 R54LiftCount = 0;
	uint8 R54HowardVorne = 0;

	short R55ExitDia = 0;

	int16 R58TmpRoom = 0;
	int16 R58TmpRoom1 = 0;
	int16 R58TmpSx = 0;
	int16 R58TmpSy = 0;
	int16 R58TmpX = 0;
	int16 R58TmpY = 0;
	int16 R64Moni1Ani = 0;

	int16 R64Moni2Ani = 0;

	// ---- //
	int16 R71Val1 = 0;
	int16 R71Val2 = 0;
	// ---- //
	int16 r76State = 0;
	// ---- //
	int16 R79Val[3] = {0, 0, 0};
	// ---- //
	int r90_Array187030[8][4] = {
		{0, 1, 180, 190},
		{0, 1,  60,  60},
		{0, 1, 140, 150},
		{0, 1, 140, 230},
		{0, 1,  40, 340},
		{0, 1,  49,  50},
		{0, 1,  88,  90},
		{0, 1, 170, 190}
	};
	
	// ---- //
	int r94Scrollx = 0;
	// --- //

	uint8 mi[8] = {0};
	int16 SVal1 = 0;
	int16 SVal2 = 0;
	int16 SVal3 = 0;
	int16 SVal4 = 0;
	uint8 FramesPerSecond = 0;
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
	uint16 ChAutoMovXy : 1;		// TODO: Check removal in favor of stopAutoMove
	uint16 BreakAMov : 1;

	uint16 AtsAction : 1;
	uint16 AdsAction : 1;
	uint16 AtsText : 1;
	uint16 DialogCloseup : 1;
	uint16 ShowAtsInvTxt : 1;
	uint16 MainInput : 1;
	uint16 InventMenu : 1;
	uint16 NoScroll : 1;

	uint16 ExitMov : 1;
	uint16 MausTxt : 1;
	uint16 LoadGame : 1;
	uint16 mainMouseFlag: 1;
	uint16 ZoomMov : 1;
	uint16 SaveMenu : 1;

	uint16 SpzAni : 1;
	uint16 MouseLeft : 1;
	uint16 ChewyDontGo : 1;
	uint16 NoPalAfterFlc : 1;
	uint16 NoDiaBox : 1;
	uint16 NoEndPosMovObj: 1;
	uint16 SavePersonRnr : 1;
};

extern void *MALLOC(size_t size);

} // namespace Chewy

#endif
