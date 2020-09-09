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

#include "bladerunner/script/init_script.h"

#include "bladerunner/bladerunner.h"

namespace BladeRunner {

void InitScript::SCRIPT_Initialize_Game() {
	Assign_Player_Gun_Hit_Sounds(0, kSfxGUNH1A, kSfxGUNH1B, kSfxGUNH1C);
	Assign_Player_Gun_Hit_Sounds(1, kSfxGUNH2A, kSfxGUNH2B, kSfxGUNH2C);
	Assign_Player_Gun_Hit_Sounds(2, kSfxGUNH3A, kSfxGUNH3B, kSfxGUNH3C);

	Assign_Player_Gun_Miss_Sounds(0, kSfxGUNM1A, kSfxGUNM1B, kSfxGUNM1C);
	Assign_Player_Gun_Miss_Sounds(1, kSfxGUNM2A, kSfxGUNM2B, kSfxGUNM2C);
	Assign_Player_Gun_Miss_Sounds(2, kSfxGUNM3A, kSfxGUNM3B, kSfxGUNM3C);

	Init_Globals();
	Init_Game_Flags();
	Init_Clues();  // Inits clues DB entries for McCoy (clues that McCoy can acquire)
	Init_Clues2(); // Inits clues DB for everyone else (clues that the other actors, including the "Mainframe" (kActorVoiceover) can acquire)
	Init_World_Waypoints();
	Init_SDB();
	Init_CDB();
	Init_Spinner();
	Init_Actor_Friendliness();
	Init_Actor_Combat_Aggressiveness();
	Init_Actor_Honesty();
	Init_Actor_Intelligence();
	Init_Actor_Stability();
	Init_Actor_Health();
	Init_Combat_Cover_Waypoints();
	Init_Combat_Flee_Waypoints();
	Init_Shadows();
}

void InitScript::Init_Globals() {
	for (int i = 0; i != 55; ++i)
		Global_Variable_Set(i, 0);

	Global_Variable_Set(kVariableGenericWalkerConfig, 2);
	Global_Variable_Set(kVariableChapter, 1);
	Global_Variable_Set(kVariableChinyen, 100);
	Global_Variable_Set(kVariableGameVersion, _vm->kBladeRunnerScummVMVersion);

	Set_Score(0, 0);
	Set_Score(1, 64);
	Set_Score(2, 10);
	Set_Score(3, 47);
	Set_Score(4, 35);
	Set_Score(5, 23);
	Set_Score(6, 28);
}

void InitScript::Init_Game_Flags() {
	for (int i = 0; i != 730; ++i) {
		Game_Flag_Reset(i);
	}

	if (Random_Query(1, 2) == 1) {
		Game_Flag_Set(kFlagIzoIsReplicant);
	}

	if (Random_Query(1, 2) == 1) {
		Game_Flag_Set(kFlagGordoIsReplicant);
	}

	if (Random_Query(1, 2) == 1) {
		Game_Flag_Set(kFlagLucyIsReplicant);
	}

	if (Random_Query(1, 2) == 1) {
		Game_Flag_Set(kFlagDektoraIsReplicant);
	}

	if (Random_Query(1, 2) == 1) {
		Game_Flag_Set(kFlagSadikIsReplicant);
	}

	if (Random_Query(1, 2) == 1) {
		Game_Flag_Set(kFlagLutherLanceIsReplicant);
	}

	if (!Game_Flag_Query(kFlagGordoIsReplicant)
	 && !Game_Flag_Query(kFlagLucyIsReplicant)
	 && !Game_Flag_Query(kFlagDektoraIsReplicant)
	) {
		Game_Flag_Set(kFlagDektoraIsReplicant);
	}

	if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
		Global_Variable_Set(kVariableHollowayArrest, 1);
	} else if ( Game_Flag_Query(kFlagGordoIsReplicant)
	        && !Game_Flag_Query(kFlagLucyIsReplicant)
	) {
		Global_Variable_Set(kVariableHollowayArrest, 2);
	} else if (!Game_Flag_Query(kFlagGordoIsReplicant)
	        &&  Game_Flag_Query(kFlagLucyIsReplicant)
	) {
		Global_Variable_Set(kVariableHollowayArrest, 3);
	} else if (Random_Query(1, 2) == 1) {
		Global_Variable_Set(kVariableHollowayArrest, 2);
	} else {
		Global_Variable_Set(kVariableHollowayArrest, 3);
	}

	Game_Flag_Set(kFlagMcCoyInRunciters);
	Game_Flag_Set(kFlagSpinnerAtRC01);

	if (_vm->_cutContent) {
		Game_Flag_Set(kFlagGamePlayedInRestoredContentMode);
	}
}

void InitScript::Init_Clues() {
	for (int i = 0; i != 288; ++i) {
		Actor_Clue_Add_To_Database(kActorMcCoy, i, 0, false, false, -1);
	}
}

struct ClueWeight {
	int clue;
	int weight;
};

#if BLADERUNNER_ORIGINAL_BUGS
static ClueWeight ClueWeightsForSteele[44] = {
#else
static ClueWeight ClueWeightsForSteele[45] = {
#endif // BLADERUNNER_ORIGINAL_BUGS
	{kClueMcCoyRetiredZuben, 100},
	{kClueMcCoyShotZubenInTheBack, 100},
	{kClueMcCoyRetiredLucy, 100},
	{kClueMcCoyRetiredDektora, 100},
	{kClueMcCoyRetiredSadik, 100},
	{kClueMcCoyRetiredLutherLance, 100},
	{kClueClovisOrdersMcCoysDeath, 100},
	{kClueVKDektoraReplicant, 100},
	{kClueVKBobGorskyReplicant, 100},
	{kClueVKLutherLanceReplicant, 100},
	{kClueVKGrigorianReplicant, 100},
	{kClueVKIzoReplicant, 100},
	{kClueVKCrazylegsReplicant, 100},
	{kClueVKRunciterReplicant, 100},
	{kClueVKEarlyQReplicant, 100},
	{kClueMcCoyIsStupid, 90},
	{kClueMcCoyIsKind, 90},
	{kClueMcCoyIsInsane, 90},
	{kClueGrigorianInterviewA, 90},
	{kClueGrigorianInterviewB1, 90},
	{kClueGrigorianInterviewB2, 90},
	{kClueChopstickWrapper, 85},
	{kClueMcCoyIsAnnoying, 85},
	{kClueMcCoyWarnedIzo, 85},
	{kClueMcCoyHelpedIzoIzoIsAReplicant, 85},
	{kClueCrimeSceneNotes, 80},
	{kClueShellCasings, 65},
	{kClueSushiMenu, 65},
	{kClueMcCoyLetZubenEscape, 65},
	{kClueMcCoyHelpedDektora, 65},
	{kClueMcCoyHelpedLucy, 65},
	{kClueMcCoyHelpedGordo, 65},
	{kClueMcCoyBetrayal, 65},
	{kClueSightingMcCoyRuncitersShop, 65},
	{kClueWeaponsOrderForm, 65},
	{kClueHollowayInterview, 65},
	{kClueRunciterConfession1, 65},
	{kClueRunciterConfession2, 65},
	{kClueMcCoyKilledRunciter1, 55},
	{kClueMcCoyShotGuzza, 55},
	{kClueMcCoyKilledRunciter2, 55},
	{kClueCandy, 30},
	{kClueToyDog, 30},
	{kClueDogCollar1, 30},
#if BLADERUNNER_ORIGINAL_BUGS
#else
	{kClueOfficersStatement, 30}, // part of resolving Zuben dead end state
#endif // BLADERUNNER_ORIGINAL_BUGS
};

static ClueWeight ClueWeightsForGordo[28] = {
	{kClueMcCoyShotZubenInTheBack, 70},
	{kClueMcCoyIsAnnoying, 65},
	{kClueMcCoyIsKind, 70},
	{kClueMcCoyIsInsane, 95},
	{kClueMcCoyKilledRunciter1, 70},
	{kClueMcCoysDescription, 70},
	{kClueMcCoyIsABladeRunner, 70},
	{kClueMcCoyLetZubenEscape, 70},
	{kClueMcCoyWarnedIzo, 95},
	{kClueMcCoyHelpedIzoIzoIsAReplicant, 70},
	{kClueMcCoyHelpedDektora, 70},
	{kClueMcCoyHelpedLucy, 70},
	{kClueMcCoyHelpedGordo, 70},
	{kClueMcCoyShotGuzza, 65},
	{kClueMcCoyRetiredZuben, 70},
	{kClueMcCoyRetiredLucy, 70},
	{kClueMcCoyRetiredDektora, 70},
	{kClueMcCoyRetiredSadik, 70},
	{kClueMcCoyRetiredLutherLance, 70},
	{kClueMcCoyBetrayal, 70},
	{kClueMcCoyKilledRunciter2, 70},
	{kClueClovisOrdersMcCoysDeath, 70},
	{kClueEarlyAttemptedToSeduceLucy, 70},
	{kClueCrazylegsInterview1, 65},
	{kClueCrazylegsInterview2, 65},
	{kCluePowerSource, 70},
	{kClueCrystalArrestedCrazylegs, 55},
	{kClueSightingMcCoyRuncitersShop, 65}
};

static ClueWeight ClueWeightsForDektora[46] = {
	{kClueMcCoyShotZubenInTheBack, 70},
	{kClueMcCoyIsAnnoying, 45},
	{kClueMcCoyIsKind, 70},
	{kClueMcCoyIsInsane, 65},
	{kClueMcCoyKilledRunciter1, 70},
	{kClueMcCoysDescription, 70},
	{kClueMcCoyIsABladeRunner, 70},
	{kClueMcCoyLetZubenEscape, 70},
	{kClueMcCoyWarnedIzo, 65},
	{kClueMcCoyHelpedIzoIzoIsAReplicant, 70},
	{kClueMcCoyHelpedGordo, 70},
	{kClueMcCoyHelpedLucy, 70},
	{kClueMcCoyHelpedDektora, 70},
	{kClueMcCoyShotGuzza, 45},
	{kClueMcCoyRetiredZuben, 70},
	{kClueMcCoyRetiredLucy, 70},
	{kClueMcCoyRetiredDektora, 70},
	{kClueMcCoyRetiredGordo, 70},
	{kClueMcCoyRetiredSadik, 70},
	{kClueMcCoyRetiredLutherLance, 70},
	{kClueMcCoyBetrayal, 70},
	{kClueMcCoyKilledRunciter2, 70},
	{kClueClovisOrdersMcCoysDeath, 70},
	{kClueSuspectDektora, 70},
	{kClueEarlyAttemptedToSeduceLucy, 70},
	{kClueMcCoyIsStupid, 65},
	{kClueZubenInterview, 65},
	{kClueHowieLeeInterview, 55},
	{kClueIzoInterview, 60},
	{kClueGordoInterview1, 60},
	{kClueGordoInterview2, 60},
	{kClueEarlyQInterview, 70},
	{kClueGordoInterview3, 65},
	{kClueIzosStashRaided, 65},
	{kClueGordoBlabs, 70},
	{kClueDNATyrell, 70},
	{kClueDNASebastian, 65},
	{kClueDNAChew, 65},
	{kClueDNAMoraji, 65},
	{kClueDNALutherLance, 65},
	{kClueDNAMarcus, 65},
	{kClueCrazylegsInterview1, 65},
	{kClueCrazylegsInterview2, 65},
	{kCluePowerSource, 70},
	{kClueCrystalArrestedCrazylegs, 55},
	{kClueSightingMcCoyRuncitersShop, 65}
};

static ClueWeight ClueWeightsForGuzza[23] = {
	{kClueMcCoyIsKind, 90},
	{kClueMcCoyIsInsane, 90},
	{kClueMcCoyIsAnnoying, 70},
	{kClueMcCoyIsABladeRunner, 75},
	{kClueMcCoyWarnedIzo, 75},
	{kClueMcCoyHelpedDektora, 75},
	{kClueMcCoyHelpedLucy, 75},
	{kClueMcCoyHelpedGordo, 75},
	{kClueMcCoyLetZubenEscape, 70},
	{kClueMcCoyHelpedIzoIzoIsAReplicant, 70},
	{kClueMcCoyRetiredZuben, 70},
	{kClueMcCoyRetiredLucy, 70},
	{kClueMcCoyRetiredDektora, 70},
	{kClueMcCoyRetiredSadik, 70},
	{kClueMcCoyRetiredLutherLance, 70},
	{kClueMcCoyKilledRunciter2, 70},
	{kClueSightingSadikBradbury, 65},
	{kClueSightingMcCoyRuncitersShop, 65},
	{kClueWeaponsOrderForm, 65},
	{kClueHollowayInterview, 65},
	{kClueRunciterConfession1, 65},
	{kClueRunciterConfession2, 65},
	{kClueMcCoyKilledRunciter1, 55}
};

static ClueWeight ClueWeightsForClovis[46] = {
	{kClueMcCoyShotZubenInTheBack, 70},
	{kClueMcCoyIsKind, 70},
	{kClueMcCoyKilledRunciter1, 70},
	{kClueMcCoysDescription, 70},
	{kClueMcCoyIsABladeRunner, 70},
	{kClueMcCoyLetZubenEscape, 70},
	{kClueMcCoyHelpedIzoIzoIsAReplicant, 70},
	{kClueMcCoyHelpedGordo, 70},
	{kClueMcCoyHelpedLucy, 70},
	{kClueMcCoyHelpedDektora, 70},
	{kClueMcCoyRetiredZuben, 70},
	{kClueMcCoyRetiredLucy, 70},
	{kClueMcCoyRetiredDektora, 70},
	{kClueMcCoyRetiredSadik, 70},
	{kClueMcCoyRetiredLutherLance, 70},
	{kClueMcCoyBetrayal, 70},
	{kClueMcCoyKilledRunciter2, 70},
	{kClueEarlyAttemptedToSeduceLucy, 70},
	{kClueGordoBlabs, 70},
	{kClueDNATyrell, 70},
	{kCluePowerSource, 70},
	{kClueMcCoyIsInsane, 65},
	{kClueMcCoyWarnedIzo, 65},
	{kClueMcCoyIsStupid, 65},
	{kClueZubenInterview, 65},
	{kClueSuspectDektora, 65},
	{kClueGordoInterview3, 65},
	{kClueDektoraInterview4, 65},
	{kClueIzosStashRaided, 65},
	{kClueDNASebastian, 65},
	{kClueDNAChew, 65},
	{kClueDNAMoraji, 65},
	{kClueDNALutherLance, 65},
	{kClueDNAMarcus, 65},
	{kClueCrazylegsInterview1, 65},
	{kClueCrazylegsInterview2, 65},
	{kClueSightingMcCoyRuncitersShop, 65},
	{kClueIzoInterview, 60},
	{kClueGordoInterview1, 60},
	{kClueGordoInterview2, 60},
	{kClueEarlyQInterview, 60},
	{kClueHowieLeeInterview, 55},
	{kClueDektoraConfession, 55},
	{kClueCrystalArrestedCrazylegs, 55},
	{kClueMcCoyIsAnnoying, 45},
	{kClueMcCoyShotGuzza, 45}
};

static ClueWeight ClueWeightsForLucy[47] = {
	{kClueMcCoyShotZubenInTheBack, 70},
	{kClueMcCoyIsAnnoying, 45},
	{kClueMcCoyIsKind, 70},
	{kClueMcCoyIsInsane, 65},
	{kClueMcCoyKilledRunciter1, 70},
	{kClueMcCoysDescription, 70},
	{kClueMcCoyIsABladeRunner, 70},
	{kClueMcCoyLetZubenEscape, 70},
	{kClueMcCoyWarnedIzo, 65},
	{kClueMcCoyHelpedIzoIzoIsAReplicant, 70},
	{kClueMcCoyHelpedGordo, 70},
	{kClueMcCoyHelpedLucy, 70},
	{kClueMcCoyHelpedDektora, 70},
	{kClueMcCoyShotGuzza, 45},
	{kClueMcCoyRetiredZuben, 70},
	{kClueMcCoyRetiredLucy, 70},
	{kClueMcCoyRetiredDektora, 70},
	{kClueMcCoyRetiredSadik, 70},
	{kClueMcCoyRetiredLutherLance, 70},
	{kClueMcCoyBetrayal, 70},
	{kClueMcCoyKilledRunciter2, 70},
	{kClueClovisOrdersMcCoysDeath, 70},
	{kClueEarlyAttemptedToSeduceLucy, 70},
	{kClueMcCoyIsStupid, 65},
	{kClueZubenInterview, 65},
	{kClueHowieLeeInterview, 55},
	{kClueIzoInterview, 60},
	{kClueGordoInterview1, 60},
	{kClueGordoInterview2, 60},
	{kClueEarlyQInterview, 60},
	{kClueSuspectDektora, 65},
	{kClueGordoInterview3, 65},
	{kClueDektoraInterview4, 65},
	{kClueIzosStashRaided, 55},
	{kClueGordoBlabs, 70},
	{kClueDektoraConfession, 70},
	{kClueDNATyrell, 70},
	{kClueDNASebastian, 65},
	{kClueDNAChew, 65},
	{kClueDNAMoraji, 65},
	{kClueDNALutherLance, 65},
	{kClueDNAMarcus, 65},
	{kClueCrazylegsInterview1, 65},
	{kClueCrazylegsInterview2, 65},
	{kCluePowerSource, 70},
	{kClueCrystalArrestedCrazylegs, 55},
	{kClueSightingMcCoyRuncitersShop, 65}
};

static ClueWeight ClueWeightsForIzoAndSadik[47] = {
	{kClueMcCoyShotZubenInTheBack, 70},
	{kClueMcCoyIsAnnoying, 45},
	{kClueMcCoyIsKind, 70},
	{kClueMcCoyIsInsane, 65},
	{kClueMcCoyKilledRunciter1, 70},
	{kClueMcCoysDescription, 70},
	{kClueMcCoyIsABladeRunner, 70},
	{kClueMcCoyLetZubenEscape, 70},
	{kClueMcCoyWarnedIzo, 65},
	{kClueMcCoyHelpedIzoIzoIsAReplicant, 70},
	{kClueMcCoyHelpedGordo, 70},
	{kClueMcCoyHelpedLucy, 70},
	{kClueMcCoyHelpedDektora, 70},
	{kClueMcCoyShotGuzza, 45},
	{kClueMcCoyRetiredZuben, 70},
	{kClueMcCoyRetiredLucy, 70},
	{kClueMcCoyRetiredDektora, 70},
	{kClueMcCoyRetiredSadik, 70},
	{kClueMcCoyRetiredLutherLance, 70},
	{kClueMcCoyBetrayal, 70},
	{kClueMcCoyKilledRunciter2, 70},
	{kClueClovisOrdersMcCoysDeath, 70},
	{kClueEarlyAttemptedToSeduceLucy, 70},
	{kClueMcCoyIsStupid, 65},
	{kClueZubenInterview, 45},
	{kClueHowieLeeInterview, 45},
	{kClueIzoInterview, 45},
	{kClueGordoInterview1, 45},
	{kClueGordoInterview2, 45},
	{kClueEarlyQInterview, 45},
	{kClueSuspectDektora, 45},
	{kClueGordoInterview3, 45},
	{kClueDektoraInterview4, 45},
	{kClueIzosStashRaided, 45},
	{kClueGordoBlabs, 45},
	{kClueDektoraConfession, 45},
	{kClueDNATyrell, 70},
	{kClueDNASebastian, 70},
	{kClueDNAChew, 70},
	{kClueDNAMoraji, 70},
	{kClueDNALutherLance, 70},
	{kClueDNAMarcus, 70},
	{kClueCrazylegsInterview1, 65},
	{kClueCrazylegsInterview2, 65},
	{kCluePowerSource, 70},
	{kClueCrystalArrestedCrazylegs, 55},
	{kClueSightingMcCoyRuncitersShop, 65}
};

static ClueWeight ClueWeightsForCrazylegs[49] = {
	{kClueMcCoyIsKind, 70},
	{kClueMcCoyKilledRunciter1, 70},
	{kClueMcCoyIsABladeRunner, 70},
	{kClueMcCoyHelpedIzoIzoIsAReplicant, 70},
	{kClueMcCoyHelpedGordo, 70},
	{kClueMcCoyHelpedLucy, 70},
	{kClueMcCoyHelpedDektora, 70},
	{kClueMcCoyRetiredZuben, 70},
	{kClueMcCoyRetiredLucy, 70},
	{kClueMcCoyRetiredDektora, 70},
	{kClueMcCoyRetiredSadik, 70},
	{kClueMcCoyRetiredLutherLance, 70},
	{kClueMcCoyBetrayal, 70},
	{kClueMcCoyKilledRunciter2, 70},
	{kClueClovisOrdersMcCoysDeath, 70},
	{kClueGordoBlabs, 70},
	{kClueDektoraConfession, 70},
	{kClueDNATyrell, 70},
	{kClueDNASebastian, 70},
	{kClueDNAChew, 70},
	{kClueDNAMoraji, 70},
	{kClueDNALutherLance, 70},
	{kClueDNAMarcus, 70},
	{kCluePowerSource, 70},
	{kClueMcCoyShotZubenInTheBack, 65},
	{kClueMcCoyIsAnnoying, 65},
	{kClueMcCoyIsInsane, 65},
	{kClueMcCoysDescription, 65},
	{kClueMcCoyLetZubenEscape, 65},
	{kClueMcCoyWarnedIzo, 65},
	{kClueMcCoyShotGuzza, 65},
	{kClueMcCoyIsStupid, 65},
	{kClueSuspectDektora, 65},
	{kClueGordoInterview3, 65},
	{kClueDektoraInterview4, 65},
	{kClueIzosStashRaided, 65},
	{kClueCrazylegsInterview1, 65},
	{kClueCrazylegsInterview2, 65},
	{kClueSightingMcCoyRuncitersShop, 65},
	{kClueGrigoriansNote, 65},
	{kClueCrazysInvolvement, 65},
	{kClueIzoInterview, 60},
	{kClueGordoInterview1, 60},
	{kClueGordoInterview2, 60},
	{kClueEarlyAttemptedToSeduceLucy, 55},
	{kClueEarlyQInterview, 55},
	{kClueCrystalArrestedCrazylegs, 55},
	{kClueZubenInterview, 50},
	{kClueHowieLeeInterview, 40}
};

static ClueWeight ClueWeightsForLuther[44] = {
	{kClueMcCoyIsKind, 70},
	{kClueGordoBlabs, 70},
	{kClueDNATyrell, 70},
	{kCluePowerSource, 70},
	{kClueMcCoyIsAnnoying, 65},
	{kClueMcCoyWarnedIzo, 65},
	{kClueMcCoyHelpedIzoIzoIsAReplicant, 65},
	{kClueMcCoyHelpedLucy, 65},
	{kClueMcCoyHelpedDektora, 65},
	{kClueMcCoyShotGuzza, 65},
	{kClueMcCoyRetiredLucy, 65},
	{kClueMcCoyRetiredDektora, 65},
	{kClueMcCoyRetiredSadik, 65},
	{kClueMcCoyBetrayal, 65},
	{kClueMcCoyIsStupid, 65},
	{kClueSuspectDektora, 65},
	{kClueIzosStashRaided, 65},
	{kClueDNASebastian, 65},
	{kClueDNAChew, 65},
	{kClueDNAMoraji, 65},
	{kClueDNAMarcus, 65},
	{kClueCrazylegsInterview1, 65},
	{kClueCrazylegsInterview2, 65},
	{kClueMcCoyIsABladeRunner, 60},
	{kClueMcCoyLetZubenEscape, 60},
	{kClueGordoInterview1, 60},
	{kClueGordoInterview2, 60},
	{kClueSightingMcCoyRuncitersShop, 60},
	{kClueMcCoyIsInsane, 55},
	{kClueMcCoysDescription, 55},
	{kClueMcCoyHelpedGordo, 55},
	{kClueMcCoyRetiredZuben, 55},
	{kClueIzoInterview, 55},
	{kClueDektoraInterview4, 55},
	{kClueDektoraConfession, 55},
	{kClueGordoInterview3, 50},
	{kClueEarlyQInterview, 45},
	{kClueCrystalArrestedCrazylegs, 45},
	{kClueMcCoyShotZubenInTheBack, 35},
	{kClueMcCoyKilledRunciter1, 35},
	{kClueMcCoyKilledRunciter2, 35},
	{kClueEarlyAttemptedToSeduceLucy, 35},
	{kClueZubenInterview, 30},
	{kClueHowieLeeInterview, 30}
};

void InitScript::Init_Clues2() {
	for (int i = 0; i != 288; ++i) {
		Actor_Clue_Add_To_Database(kActorVoiceOver, i, 100, false, false, -1);
	}

#define IMPORT_CLUE_TABLE(a, arr) \
	for (int i = 0; i != ARRAYSIZE(arr); ++i) {\
		Actor_Clue_Add_To_Database( a, arr[i].clue, arr[i].weight, false, false, -1);\
	}

	IMPORT_CLUE_TABLE(kActorSteele, ClueWeightsForSteele);
	IMPORT_CLUE_TABLE(kActorGordo, ClueWeightsForGordo);
	IMPORT_CLUE_TABLE(kActorDektora, ClueWeightsForDektora);
	IMPORT_CLUE_TABLE(kActorGuzza, ClueWeightsForGuzza);
	IMPORT_CLUE_TABLE(kActorClovis, ClueWeightsForClovis);
	IMPORT_CLUE_TABLE(kActorLucy, ClueWeightsForLucy);
	IMPORT_CLUE_TABLE(kActorIzo, ClueWeightsForIzoAndSadik);
	IMPORT_CLUE_TABLE(kActorSadik, ClueWeightsForIzoAndSadik);
	IMPORT_CLUE_TABLE(kActorCrazylegs, ClueWeightsForCrazylegs);
	IMPORT_CLUE_TABLE(kActorLuther, ClueWeightsForLuther);

#undef IMPORT_CLUE_TABLE

	//note: this might look a bit weird, but it is implemented in this way in the original game - few weights are in table, others by direct call
	Actor_Clue_Add_To_Database(kActorGrigorian, kClueBobShotInColdBlood, 85, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGrigorian, kClueMcCoysDescription, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGrigorian, kClueMcCoyIsABladeRunner, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGrigorian, kClueMcCoyIsKind, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGrigorian, kClueMcCoyKilledRunciter1, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGrigorian, kClueMcCoyLetZubenEscape, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGrigorian, kClueMcCoyWarnedIzo, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGrigorian, kClueMcCoyHelpedIzoIzoIsAReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGrigorian, kClueMcCoyHelpedDektora, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGrigorian, kClueMcCoyHelpedLucy, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGrigorian, kClueMcCoyHelpedGordo, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGrigorian, kClueMcCoyShotGuzza, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGrigorian, kClueMcCoyRetiredZuben, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGrigorian, kClueMcCoyRetiredLucy, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGrigorian, kClueMcCoyRetiredDektora, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGrigorian, kClueMcCoyRetiredGordo, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGrigorian, kClueMcCoyRetiredSadik, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGrigorian, kClueMcCoyRetiredLutherLance, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGrigorian, kClueMcCoyBetrayal, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGrigorian, kClueMcCoyKilledRunciter2, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGrigorian, kClueClovisOrdersMcCoysDeath, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGrigorian, kClueEarlyAttemptedToSeduceLucy, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGrigorian, kClueCrazylegsInterview1, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGrigorian, kClueCrazylegsInterview2, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGrigorian, kCluePowerSource, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGrigorian, kClueCrystalArrestedCrazylegs, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGrigorian, kClueSightingMcCoyRuncitersShop, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTransient, kClueMcCoysDescription, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTransient, kClueMcCoyIsKind, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTransient, kClueMcCoyHelpedLucy, 75, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTransient, kClueMcCoyRetiredZuben, 75, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTransient, kClueMcCoyRetiredLucy, 75, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTransient, kClueMcCoyRetiredLutherLance, 75, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTransient, kClueEarlyAttemptedToSeduceLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTransient, kClueHomelessManKid, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTransient, kClueFlaskOfAbsinthe, 100, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueMcCoyShotZubenInTheBack, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueMcCoyIsAnnoying, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueMcCoyIsKind, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueMcCoyIsInsane, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueMcCoyKilledRunciter1, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueMcCoysDescription, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueMcCoyIsABladeRunner, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueMcCoyLetZubenEscape, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueMcCoyWarnedIzo, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueMcCoyHelpedIzoIzoIsAReplicant, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueMcCoyHelpedGordo, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueMcCoyHelpedLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueMcCoyHelpedDektora, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueMcCoyShotGuzza, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueMcCoyRetiredZuben, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueMcCoyRetiredLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueMcCoyRetiredDektora, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueMcCoyRetiredSadik, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueMcCoyBetrayal, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueMcCoyKilledRunciter2, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueEarlyAttemptedToSeduceLucy, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueMcCoyIsStupid, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueHowieLeeInterview, 30, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueIzoInterview, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueGordoInterview1, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueGordoInterview2, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueEarlyQInterview, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueSuspectDektora, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueZubenInterview, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueGordoInterview3, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueDektoraInterview4, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueIzosStashRaided, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueGordoBlabs, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueDektoraConfession, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueDNATyrell, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueDNASebastian, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueDNAChew, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueDNAMoraji, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueDNAMarcus, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueCrazylegsInterview1, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueCrazylegsInterview2, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kCluePowerSource, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueCrystalArrestedCrazylegs, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLance, kClueSightingMcCoyRuncitersShop, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueShellCasings, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueMcCoyIsStupid, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueMcCoyIsAnnoying, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueMcCoyIsKind, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueMcCoyIsInsane, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueMcCoyRetiredZuben, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueMcCoyShotZubenInTheBack, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueMcCoyKilledRunciter1, 40, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueMcCoyLetZubenEscape, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueMcCoyWarnedIzo, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueMcCoyHelpedIzoIzoIsAReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueMcCoyHelpedDektora, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueMcCoyHelpedLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueMcCoyHelpedGordo, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueMcCoyShotGuzza, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueMcCoyRetiredLucy, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueMcCoyRetiredDektora, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueMcCoyRetiredSadik, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueMcCoyRetiredLutherLance, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueMcCoyKilledRunciter2, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueClovisOrdersMcCoysDeath, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueVKDektoraReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueVKBobGorskyReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueVKLutherLanceReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueVKGrigorianReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueVKIzoReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueVKCrazylegsReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueVKRunciterReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueVKEarlyQReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueOfficersStatement, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueSightingSadikBradbury, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueSightingMcCoyRuncitersShop, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueWeaponsOrderForm, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueHollowayInterview, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueRunciterConfession1, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueRunciterConfession2, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueWeaponsCache, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueBakersBadge, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueHoldensBadge, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueCar, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBulletBob, kClueMcCoyIsABladeRunner, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRunciter, kClueMcCoyIsAnnoying, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRunciter, kClueMcCoyIsKind, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRunciter, kClueMcCoyIsInsane, 95, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRunciter, kClueMcCoyLetZubenEscape, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRunciter, kClueMcCoyHelpedIzoIzoIsAReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRunciter, kClueMcCoyShotGuzza, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRunciter, kClueMcCoyRetiredZuben, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRunciter, kClueMcCoyRetiredLucy, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRunciter, kClueMcCoyRetiredDektora, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRunciter, kClueMcCoyRetiredSadik, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRunciter, kClueMcCoyRetiredLutherLance, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRunciter, kClueEarlyAttemptedToSeduceLucy, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueMcCoyShotZubenInTheBack, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueMcCoyIsAnnoying, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueMcCoyIsKind, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueMcCoyIsInsane, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueMcCoyKilledRunciter1, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueMcCoysDescription, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueMcCoyIsABladeRunner, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueMcCoyLetZubenEscape, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueMcCoyWarnedIzo, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueMcCoyHelpedIzoIzoIsAReplicant, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueMcCoyHelpedGordo, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueMcCoyHelpedLucy, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueMcCoyHelpedDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueMcCoyShotGuzza, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueMcCoyRetiredZuben, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueMcCoyRetiredLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueMcCoyRetiredDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueMcCoyRetiredSadik, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueMcCoyRetiredLutherLance, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueMcCoyKilledRunciter2, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueSuspectDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueEarlyAttemptedToSeduceLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueMcCoyIsStupid, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueGordoBlabs, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueDNATyrell, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueDNASebastian, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueDNAChew, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueDNAMoraji, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueDNALutherLance, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueDNAMarcus, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueCrazylegsInterview1, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueCrazylegsInterview2, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kCluePowerSource, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorInsectDealer, kClueSightingMcCoyRuncitersShop, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrellGuard, kClueMcCoyIsStupid, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrellGuard, kClueMcCoyIsAnnoying, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrellGuard, kClueMcCoyIsKind, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrellGuard, kClueMcCoyIsInsane, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrellGuard, kClueMcCoyRetiredZuben, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrellGuard, kClueMcCoyHelpedDektora, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrellGuard, kClueMcCoyHelpedLucy, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrellGuard, kClueMcCoyHelpedGordo, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrellGuard, kClueMcCoyShotGuzza, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrellGuard, kClueMcCoyRetiredLucy, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrellGuard, kClueMcCoyRetiredDektora, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrellGuard, kClueMcCoyRetiredSadik, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrellGuard, kClueMcCoyRetiredLutherLance, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrellGuard, kClueMcCoyKilledRunciter2, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrellGuard, kClueClovisOrdersMcCoysDeath, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrellGuard, kClueSightingSadikBradbury, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrellGuard, kClueSightingMcCoyRuncitersShop, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueMcCoyIsStupid, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueMcCoyIsAnnoying, 85, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueMcCoyIsKind, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueMcCoyIsInsane, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueMcCoyRetiredZuben, 100, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueMcCoyShotZubenInTheBack, 100, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueMcCoyKilledRunciter1, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueMcCoyLetZubenEscape, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueMcCoyWarnedIzo, 85, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueMcCoyHelpedIzoIzoIsAReplicant, 85, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueMcCoyHelpedDektora, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueMcCoyHelpedLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueMcCoyHelpedGordo, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueMcCoyShotGuzza, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueMcCoyRetiredLucy, 100, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueMcCoyRetiredDektora, 100, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueMcCoyRetiredLutherLance, 100, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueMcCoyBetrayal, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueMcCoyKilledRunciter2, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueClovisOrdersMcCoysDeath, 100, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueWeaponsOrderForm, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueHollowayInterview, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueRunciterConfession1, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueRunciterConfession2, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueMcCoyRetiredSadik, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueMcCoyIsABladeRunner, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kCluePowerSource, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueCrystalArrestedCrazylegs, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQ, kClueSightingMcCoyRuncitersShop, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorZuben, kClueMcCoyIsKind, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorZuben, kClueMcCoyShotZubenInTheBack, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorZuben, kClueMcCoyKilledRunciter1, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorZuben, kClueMcCoyKilledRunciter2, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorZuben, kClueMcCoyLetZubenEscape, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorZuben, kClueMcCoyWarnedIzo, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorZuben, kClueMcCoyHelpedIzoIzoIsAReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorZuben, kClueMcCoyHelpedDektora, 95, false, false, -1);
	Actor_Clue_Add_To_Database(kActorZuben, kClueMcCoyHelpedLucy, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorZuben, kClueMcCoyHelpedGordo, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorZuben, kClueMcCoyShotGuzza, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorZuben, kClueMcCoyRetiredLucy, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorZuben, kClueMcCoyRetiredDektora, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorZuben, kClueMcCoyRetiredSadik, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorZuben, kClueMcCoyRetiredLutherLance, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorZuben, kClueClovisOrdersMcCoysDeath, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorZuben, kClueMcCoyIsInsane, 95, false, false, -1);
	Actor_Clue_Add_To_Database(kActorZuben, kClueMcCoysDescription, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorZuben, kClueMcCoyIsABladeRunner, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorZuben, kClueMcCoyBetrayal, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorZuben, kClueEarlyAttemptedToSeduceLucy, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorZuben, kClueCrazylegsInterview1, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorZuben, kClueCrazylegsInterview2, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorZuben, kCluePowerSource, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorZuben, kClueCrystalArrestedCrazylegs, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorZuben, kClueSightingMcCoyRuncitersShop, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueMcCoyShotZubenInTheBack, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueMcCoyIsAnnoying, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueMcCoyIsKind, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueMcCoyIsInsane, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueMcCoyKilledRunciter1, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueMcCoysDescription, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueMcCoyIsABladeRunner, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueMcCoyLetZubenEscape, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueMcCoyWarnedIzo, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueMcCoyHelpedIzoIzoIsAReplicant, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueMcCoyHelpedGordo, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueMcCoyHelpedLucy, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueMcCoyHelpedDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueMcCoyShotGuzza, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueMcCoyRetiredZuben, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueMcCoyRetiredLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueMcCoyRetiredDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueMcCoyRetiredSadik, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueMcCoyRetiredLutherLance, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueMcCoyKilledRunciter2, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueSuspectDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueEarlyAttemptedToSeduceLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueMcCoyIsStupid, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueGordoBlabs, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueDNATyrell, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueDNASebastian, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueDNAChew, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueDNAMoraji, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueDNALutherLance, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueDNAMarcus, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueCrazylegsInterview1, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueCrazylegsInterview2, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kCluePowerSource, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHasan, kClueSightingMcCoyRuncitersShop, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMarcus, kClueMcCoyIsAnnoying, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMarcus, kClueMcCoyIsKind, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMarcus, kClueMcCoyIsInsane, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMarcus, kClueMcCoyKilledRunciter1, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMarcus, kClueMcCoysDescription, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMarcus, kClueMcCoyIsABladeRunner, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMarcus, kClueMcCoyLetZubenEscape, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMarcus, kClueMcCoyWarnedIzo, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMarcus, kClueMcCoyHelpedIzoIzoIsAReplicant, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMarcus, kClueMcCoyHelpedGordo, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMarcus, kClueMcCoyHelpedLucy, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMarcus, kClueMcCoyHelpedDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMarcus, kClueMcCoyShotGuzza, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMarcus, kClueMcCoyRetiredZuben, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMarcus, kClueMcCoyRetiredLucy, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMarcus, kClueMcCoyRetiredDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMarcus, kClueMcCoyRetiredSadik, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMarcus, kClueMcCoyKilledRunciter2, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMarcus, kClueMcCoyIsStupid, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMarcus, kClueHowieLeeInterview, 30, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMarcus, kClueDNATyrell, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMarcus, kClueDNASebastian, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMarcus, kClueDNAMoraji, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMarcus, kClueDNAMarcus, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMarcus, kClueCrazylegsInterview2, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMarcus, kCluePowerSource, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMarcus, kClueSightingMcCoyRuncitersShop, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueMcCoyShotZubenInTheBack, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueMcCoyIsAnnoying, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueMcCoyIsKind, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueMcCoyIsInsane, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueMcCoyKilledRunciter1, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueMcCoysDescription, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueMcCoyIsABladeRunner, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueMcCoyLetZubenEscape, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueMcCoyWarnedIzo, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueMcCoyHelpedIzoIzoIsAReplicant, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueMcCoyHelpedGordo, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueMcCoyHelpedLucy, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueMcCoyHelpedDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueMcCoyShotGuzza, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueMcCoyRetiredZuben, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueMcCoyRetiredLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueMcCoyRetiredDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueMcCoyRetiredSadik, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueMcCoyRetiredLutherLance, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueMcCoyKilledRunciter2, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueSuspectDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueEarlyAttemptedToSeduceLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueMcCoyIsStupid, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueGordoBlabs, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueDNATyrell, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueDNASebastian, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueDNAChew, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueDNAMoraji, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueDNALutherLance, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueDNAMarcus, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueCrazylegsInterview1, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueCrazylegsInterview2, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kCluePowerSource, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMia, kClueSightingMcCoyRuncitersShop, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerLeary, kClueChromeDebris, 20, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerLeary, kClueCrowdInterviewA, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerLeary, kClueCrowdInterviewB, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerLeary, kCluePaintTransfer, 25, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerLeary, kClueMcCoyIsKind, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerLeary, kClueMcCoyShotZubenInTheBack, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerLeary, kClueMcCoyKilledRunciter1, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerLeary, kClueMcCoyKilledRunciter2, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerLeary, kClueMcCoyLetZubenEscape, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerLeary, kClueMcCoyWarnedIzo, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerLeary, kClueMcCoyHelpedIzoIzoIsAReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerLeary, kClueMcCoyHelpedDektora, 95, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerLeary, kClueMcCoyHelpedLucy, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerLeary, kClueMcCoyHelpedGordo, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerLeary, kClueMcCoyShotGuzza, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerLeary, kClueMcCoyRetiredLucy, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerLeary, kClueMcCoyRetiredDektora, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerLeary, kClueMcCoyRetiredSadik, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerLeary, kClueMcCoyRetiredLutherLance, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerLeary, kClueMcCoyIsInsane, 95, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerLeary, kClueMcCoyIsStupid, 95, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSteele, kClueSightingSadikBradbury, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerLeary, kClueSightingMcCoyRuncitersShop, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerLeary, kClueWeaponsOrderForm, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerLeary, kClueHollowayInterview, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerLeary, kClueRunciterConfession1, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerLeary, kClueRunciterConfession2, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerGrayford, kClueMcCoyIsKind, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerGrayford, kClueMcCoyShotZubenInTheBack, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerGrayford, kClueMcCoyKilledRunciter1, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerGrayford, kClueMcCoyKilledRunciter2, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerGrayford, kClueMcCoyLetZubenEscape, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerGrayford, kClueMcCoyWarnedIzo, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerGrayford, kClueMcCoyHelpedIzoIzoIsAReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerGrayford, kClueMcCoyHelpedDektora, 95, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerGrayford, kClueMcCoyHelpedLucy, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerGrayford, kClueMcCoyHelpedGordo, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerGrayford, kClueMcCoyShotGuzza, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerGrayford, kClueMcCoyRetiredLucy, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerGrayford, kClueMcCoyRetiredDektora, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerGrayford, kClueMcCoyRetiredSadik, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerGrayford, kClueMcCoyRetiredLutherLance, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerGrayford, kClueMcCoyIsInsane, 95, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerGrayford, kClueMcCoyIsStupid, 95, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerGrayford, kClueCrowdInterviewB, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerGrayford, kClueCrowdInterviewA, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerGrayford, kClueChromeDebris, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerGrayford, kCluePaintTransfer, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerGrayford, kClueSightingSadikBradbury, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerGrayford, kClueSightingMcCoyRuncitersShop, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerGrayford, kClueWeaponsOrderForm, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerGrayford, kClueHollowayInterview, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerGrayford, kClueRunciterConfession1, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorOfficerGrayford, kClueRunciterConfession2, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHanoi, kClueMcCoyIsInsane, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHanoi, kClueMcCoysDescription, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHanoi, kClueMcCoyIsStupid, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueOfficersStatement, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueShellCasings, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueChopstickWrapper, 85, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueSushiMenu, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueMcCoyIsStupid, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueMcCoyIsAnnoying, 85, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueMcCoyIsKind, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueMcCoyIsInsane, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueMcCoyRetiredZuben, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueMcCoyShotZubenInTheBack, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueMcCoyKilledRunciter1, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueMcCoyLetZubenEscape, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueMcCoyWarnedIzo, 85, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueMcCoyHelpedIzoIzoIsAReplicant, 85, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueMcCoyHelpedDektora, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueMcCoyHelpedLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueMcCoyHelpedGordo, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueMcCoyShotGuzza, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueMcCoyRetiredLucy, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueMcCoyRetiredDektora, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueMcCoyRetiredSadik, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueMcCoyRetiredLutherLance, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueMcCoyBetrayal, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueMcCoyKilledRunciter2, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueClovisOrdersMcCoysDeath, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueVKDektoraReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueVKBobGorskyReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueVKLutherLanceReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueVKGrigorianReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueVKIzoReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueVKCrazylegsReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueVKRunciterReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueVKEarlyQReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueSightingSadikBradbury, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueSightingMcCoyRuncitersShop, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueWeaponsOrderForm, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueHollowayInterview, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueRunciterConfession1, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorBaker, kClueRunciterConfession2, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueMcCoyShotZubenInTheBack, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueMcCoyIsAnnoying, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueMcCoyIsKind, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueMcCoyIsInsane, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueMcCoyKilledRunciter1, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueMcCoysDescription, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueMcCoyIsABladeRunner, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueMcCoyLetZubenEscape, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueMcCoyWarnedIzo, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueMcCoyHelpedIzoIzoIsAReplicant, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueMcCoyHelpedGordo, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueMcCoyHelpedLucy, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueMcCoyHelpedDektora, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueMcCoyShotGuzza, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueMcCoyRetiredZuben, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueMcCoyRetiredLucy, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueMcCoyRetiredDektora, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueMcCoyRetiredSadik, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueMcCoyRetiredLutherLance, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueMcCoyKilledRunciter2, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueEarlyAttemptedToSeduceLucy, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueMcCoyIsStupid, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueHowieLeeInterview, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueIzoInterview, 40, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueGordoInterview1, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueGordoInterview2, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueEarlyQInterview, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueSuspectDektora, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueZubenInterview, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueGordoInterview3, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueDektoraInterview4, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueIzosStashRaided, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueGordoBlabs, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueDektoraConfession, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueDNATyrell, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueDNASebastian, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueDNAChew, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueDNAMoraji, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueDNALutherLance, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueDNAMarcus, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueCrazylegsInterview1, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueCrazylegsInterview2, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kCluePowerSource, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueCrystalArrestedCrazylegs, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorDeskClerk, kClueSightingMcCoyRuncitersShop, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHowieLee, kClueHowieLeeInterview, 85, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHowieLee, kClueFishLadyInterview, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHowieLee, kClueGordoInterview1, 85, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHowieLee, kClueCar, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHowieLee, kClueHomelessManKid, 85, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHowieLee, kClueMcCoyHelpedLucy, 75, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHowieLee, kClueMcCoyIsKind, 85, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHowieLee, kClueMcCoyKilledRunciter1, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHowieLee, kClueMcCoyKilledRunciter2, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHowieLee, kClueMcCoyHelpedIzoIzoIsAReplicant, 25, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHowieLee, kClueMcCoyHelpedGordo, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHowieLee, kClueMcCoyShotGuzza, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHowieLee, kClueMcCoyRetiredLucy, 75, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHowieLee, kClueMcCoyRetiredGordo, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHowieLee, kClueMcCoyRetiredZuben, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHowieLee, kClueEarlyAttemptedToSeduceLucy, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueMcCoyShotZubenInTheBack, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueMcCoyIsAnnoying, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueMcCoyIsKind, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueMcCoyIsInsane, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueMcCoyKilledRunciter1, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueMcCoysDescription, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueMcCoyIsABladeRunner, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueMcCoyLetZubenEscape, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueMcCoyWarnedIzo, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueMcCoyHelpedIzoIzoIsAReplicant, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueMcCoyHelpedGordo, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueMcCoyHelpedLucy, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueMcCoyHelpedDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueMcCoyShotGuzza, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueMcCoyRetiredZuben, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueMcCoyRetiredLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueMcCoyRetiredDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueMcCoyRetiredSadik, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueMcCoyRetiredLutherLance, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueMcCoyKilledRunciter2, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueSuspectDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueEarlyAttemptedToSeduceLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueMcCoyIsStupid, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueGordoBlabs, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueDNATyrell, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueDNASebastian, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueDNAChew, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueDNAMoraji, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueDNALutherLance, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueDNAMarcus, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueCrazylegsInterview1, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueCrazylegsInterview2, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kCluePowerSource, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorFishDealer, kClueSightingMcCoyRuncitersShop, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueGuzzaFramedMcCoy, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueVKDektoraReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueVKBobGorskyReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueVKLutherLanceReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueVKGrigorianReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueVKIzoReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueVKCrazylegsReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueVKRunciterReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueVKEarlyQReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueMcCoyIncept, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kCluePoliceWeaponUsed, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueMcCoysWeaponUsedonBob, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueMcCoyRecoveredHoldensBadge, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueMcCoyIsKind, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueMcCoyShotZubenInTheBack, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueMcCoyKilledRunciter1, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueMcCoyKilledRunciter2, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueMcCoyLetZubenEscape, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueMcCoyWarnedIzo, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueMcCoyHelpedIzoIzoIsAReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueMcCoyHelpedDektora, 95, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueMcCoyHelpedGordo, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueMcCoyShotGuzza, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueMcCoyRetiredLucy, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueMcCoyRetiredGordo, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueMcCoyRetiredDektora, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueMcCoyRetiredSadik, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueMcCoyRetiredLutherLance, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueMcCoyRetiredZuben, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueMcCoyIsInsane, 95, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueMcCoyIsStupid, 95, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueSightingSadikBradbury, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueSightingMcCoyRuncitersShop, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueWeaponsOrderForm, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueHollowayInterview, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueRunciterConfession1, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorKlein, kClueRunciterConfession2, 65, false, false, -1);
	if (_vm->_cutContent) {
		Actor_Clue_Add_To_Database(kActorKlein, kClueShellCasings, 65, false, false, -1);
		Actor_Clue_Add_To_Database(kActorKlein, kClueChromeDebris, 65, false, false, -1);
		Actor_Clue_Add_To_Database(kActorKlein, kClueOfficersStatement, 65, false, false, -1);
		Actor_Clue_Add_To_Database(kActorKlein, kCluePaintTransfer, 65, false, false, -1);
	}
	Actor_Clue_Add_To_Database(kActorMurray, kClueMcCoyShotZubenInTheBack, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueMcCoyIsAnnoying, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueMcCoyIsKind, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueMcCoyIsInsane, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueMcCoyKilledRunciter1, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueMcCoysDescription, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueMcCoyIsABladeRunner, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueMcCoyLetZubenEscape, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueMcCoyWarnedIzo, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueMcCoyHelpedIzoIzoIsAReplicant, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueMcCoyHelpedGordo, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueMcCoyHelpedLucy, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueMcCoyHelpedDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueMcCoyShotGuzza, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueMcCoyRetiredZuben, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueMcCoyRetiredLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueMcCoyRetiredDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueMcCoyRetiredSadik, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueMcCoyRetiredLutherLance, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueMcCoyKilledRunciter2, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueSuspectDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueEarlyAttemptedToSeduceLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueMcCoyIsStupid, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueGordoBlabs, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueDNATyrell, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueDNASebastian, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueDNAChew, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueDNAMoraji, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueDNALutherLance, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueDNAMarcus, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueCrazylegsInterview1, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueCrazylegsInterview2, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kCluePowerSource, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMurray, kClueSightingMcCoyRuncitersShop, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueMcCoyShotZubenInTheBack, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueMcCoyIsAnnoying, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueMcCoyIsKind, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueMcCoyIsInsane, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueMcCoyKilledRunciter1, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueMcCoysDescription, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueMcCoyIsABladeRunner, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueMcCoyLetZubenEscape, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueMcCoyWarnedIzo, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueMcCoyHelpedIzoIzoIsAReplicant, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueMcCoyHelpedGordo, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueMcCoyHelpedLucy, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueMcCoyHelpedDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueMcCoyShotGuzza, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueMcCoyRetiredZuben, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueMcCoyRetiredLucy, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueMcCoyRetiredDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueMcCoyRetiredSadik, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueMcCoyRetiredLutherLance, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueMcCoyBetrayal, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueMcCoyKilledRunciter2, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueClovisOrdersMcCoysDeath, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueSuspectDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueEarlyAttemptedToSeduceLucy, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueMcCoyIsStupid, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueHowieLeeInterview, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueIzoInterview, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueGordoInterview1, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueGordoInterview2, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueEarlyQInterview, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueZubenInterview, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueGordoInterview3, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueIzosStashRaided, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueGordoBlabs, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueDNATyrell, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueDNASebastian, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueDNAChew, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueDNAMoraji, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueDNALutherLance, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueDNAMarcus, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueCrazylegsInterview1, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueCrazylegsInterview2, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kCluePowerSource, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueCrystalArrestedCrazylegs, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHawkersBarkeep, kClueSightingMcCoyRuncitersShop, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueShellCasings, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueChopstickWrapper, 85, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueSushiMenu, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueMcCoyIsStupid, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueMcCoyIsAnnoying, 85, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueMcCoyIsKind, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueMcCoyIsInsane, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueMcCoyRetiredZuben, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueMcCoyShotZubenInTheBack, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueMcCoyKilledRunciter1, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueMcCoyLetZubenEscape, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueMcCoyWarnedIzo, 85, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueMcCoyHelpedIzoIzoIsAReplicant, 85, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueMcCoyHelpedDektora, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueMcCoyHelpedLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueMcCoyHelpedGordo, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueMcCoyShotGuzza, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueMcCoyRetiredLucy, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueMcCoyRetiredDektora, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueMcCoyRetiredSadik, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueMcCoyRetiredLutherLance, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueMcCoyBetrayal, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueMcCoyKilledRunciter2, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueClovisOrdersMcCoysDeath, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueVKDektoraReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueVKBobGorskyReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueVKLutherLanceReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueVKGrigorianReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueVKIzoReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueVKCrazylegsReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueVKRunciterReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueVKEarlyQReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueSightingSadikBradbury, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueSightingMcCoyRuncitersShop, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueWeaponsOrderForm, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueHollowayInterview, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueRunciterConfession1, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorHolloway, kClueRunciterConfession2, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueGuzzaFramedMcCoy, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueVKDektoraReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueVKBobGorskyReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueVKLutherLanceReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueVKGrigorianReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueVKIzoReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueVKCrazylegsReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueVKRunciterReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueVKEarlyQReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueMcCoyIncept, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kCluePoliceWeaponUsed, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueMcCoysWeaponUsedonBob, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueMcCoyRecoveredHoldensBadge, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueMcCoyHelpedLucy, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueMcCoyIsKind, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueMcCoyShotZubenInTheBack, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueMcCoyKilledRunciter1, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueMcCoyKilledRunciter2, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueMcCoyLetZubenEscape, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueMcCoyWarnedIzo, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueMcCoyHelpedIzoIzoIsAReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueMcCoyHelpedDektora, 95, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueMcCoyHelpedGordo, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueMcCoyShotGuzza, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueMcCoyRetiredLucy, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueMcCoyRetiredGordo, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueMcCoyRetiredDektora, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueMcCoyRetiredSadik, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueMcCoyRetiredLutherLance, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueMcCoyRetiredZuben, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueMcCoyIsInsane, 95, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueMcCoyIsStupid, 95, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueSightingSadikBradbury, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueSightingMcCoyRuncitersShop, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueWeaponsOrderForm, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueHollowayInterview, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueRunciterConfession1, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSergeantWalls, kClueRunciterConfession2, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMoraji, kClueMcCoyIsAnnoying, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMoraji, kClueMcCoyIsKind, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMoraji, kClueMcCoyIsInsane, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMoraji, kClueMcCoyKilledRunciter1, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMoraji, kClueMcCoysDescription, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMoraji, kClueMcCoyIsABladeRunner, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMoraji, kClueMcCoyLetZubenEscape, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMoraji, kClueMcCoyWarnedIzo, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMoraji, kClueMcCoyHelpedIzoIzoIsAReplicant, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMoraji, kClueMcCoyHelpedGordo, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMoraji, kClueMcCoyHelpedLucy, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMoraji, kClueMcCoyHelpedDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMoraji, kClueMcCoyShotGuzza, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMoraji, kClueMcCoyRetiredZuben, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMoraji, kClueMcCoyRetiredLucy, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMoraji, kClueMcCoyRetiredDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMoraji, kClueMcCoyRetiredSadik, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMoraji, kClueMcCoyKilledRunciter2, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMoraji, kClueMcCoyIsStupid, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMoraji, kClueHowieLeeInterview, 30, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMoraji, kClueDNATyrell, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMoraji, kClueDNASebastian, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMoraji, kClueDNAMoraji, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMoraji, kClueDNAMarcus, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMoraji, kClueCrazylegsInterview2, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMoraji, kCluePowerSource, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMoraji, kClueSightingMcCoyRuncitersShop, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueOfficersStatement, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueShellCasings, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueChopstickWrapper, 85, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueSushiMenu, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueMcCoyIsStupid, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueMcCoyIsAnnoying, 85, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueMcCoyIsKind, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueMcCoyIsInsane, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueMcCoyRetiredZuben, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueMcCoyShotZubenInTheBack, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueMcCoyKilledRunciter1, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueMcCoyLetZubenEscape, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueMcCoyWarnedIzo, 85, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueMcCoyHelpedIzoIzoIsAReplicant, 85, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueMcCoyHelpedDektora, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueMcCoyHelpedLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueMcCoyHelpedGordo, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueMcCoyShotGuzza, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueMcCoyRetiredLucy, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueMcCoyRetiredDektora, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueMcCoyRetiredSadik, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueMcCoyRetiredLutherLance, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueMcCoyBetrayal, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueMcCoyKilledRunciter2, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueClovisOrdersMcCoysDeath, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueVKDektoraReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueVKBobGorskyReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueVKLutherLanceReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueVKGrigorianReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueVKIzoReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueVKCrazylegsReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueVKRunciterReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueVKEarlyQReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueSightingSadikBradbury, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueSightingMcCoyRuncitersShop, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueWeaponsOrderForm, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueHollowayInterview, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueRunciterConfession1, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorPhotographer, kClueRunciterConfession2, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueMcCoyIsStupid, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueMcCoyIsAnnoying, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueMcCoyIsKind, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueMcCoyIsInsane, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueMcCoyRetiredZuben, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueMcCoyShotZubenInTheBack, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueMcCoyKilledRunciter1, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueMcCoyLetZubenEscape, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueMcCoyWarnedIzo, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueMcCoyHelpedIzoIzoIsAReplicant, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueMcCoyHelpedDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueMcCoyHelpedLucy, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueMcCoyHelpedGordo, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueMcCoyShotGuzza, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueMcCoyRetiredLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueMcCoyRetiredDektora, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueMcCoyRetiredLutherLance, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueMcCoyBetrayal, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueMcCoyKilledRunciter2, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueClovisOrdersMcCoysDeath, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueWeaponsOrderForm, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueHollowayInterview, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueRunciterConfession1, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueRunciterConfession2, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueMcCoyRetiredSadik, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueMcCoyIsABladeRunner, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kCluePowerSource, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueCrystalArrestedCrazylegs, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorEarlyQBartender, kClueSightingMcCoyRuncitersShop, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffyPatron, kClueMcCoyIsStupid, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffyPatron, kClueMcCoyIsAnnoying, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffyPatron, kClueMcCoyIsKind, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffyPatron, kClueMcCoyIsInsane, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffyPatron, kClueMcCoyKilledRunciter1, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffyPatron, kClueMcCoyKilledRunciter2, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffyPatron, kClueRunciterConfession1, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffyPatron, kClueRunciterConfession2, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffyPatron, kClueMcCoyIsABladeRunner, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrell, kClueMcCoyIsKind, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrell, kClueMcCoyShotZubenInTheBack, 40, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrell, kClueMcCoyKilledRunciter1, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrell, kClueMcCoyLetZubenEscape, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrell, kClueMcCoyWarnedIzo, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrell, kClueMcCoyHelpedIzoIzoIsAReplicant, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrell, kClueMcCoyHelpedDektora, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrell, kClueMcCoyHelpedLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrell, kClueMcCoyHelpedGordo, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrell, kClueMcCoyShotGuzza, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrell, kClueMcCoyRetiredLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrell, kClueMcCoyRetiredDektora, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrell, kClueMcCoyRetiredSadik, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrell, kClueMcCoyRetiredLutherLance, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrell, kClueClovisOrdersMcCoysDeath, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrell, kClueMcCoyIsInsane, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrell, kClueMcCoysDescription, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrell, kClueMcCoyIsABladeRunner, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrell, kClueMcCoyBetrayal, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrell, kClueEarlyAttemptedToSeduceLucy, 40, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrell, kCluePowerSource, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTyrell, kClueSightingMcCoyRuncitersShop, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorChew, kClueMcCoyIsAnnoying, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorChew, kClueMcCoyIsKind, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorChew, kClueMcCoyIsInsane, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorChew, kClueMcCoyKilledRunciter1, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorChew, kClueMcCoysDescription, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorChew, kClueMcCoyIsABladeRunner, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorChew, kClueMcCoyLetZubenEscape, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorChew, kClueMcCoyWarnedIzo, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorChew, kClueMcCoyHelpedIzoIzoIsAReplicant, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorChew, kClueMcCoyHelpedGordo, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorChew, kClueMcCoyHelpedLucy, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorChew, kClueMcCoyHelpedDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorChew, kClueMcCoyShotGuzza, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorChew, kClueMcCoyRetiredZuben, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorChew, kClueMcCoyRetiredLucy, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorChew, kClueMcCoyRetiredDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorChew, kClueMcCoyRetiredSadik, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorChew, kClueMcCoyKilledRunciter2, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorChew, kClueMcCoyIsStupid, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorChew, kClueHowieLeeInterview, 30, false, false, -1);
	Actor_Clue_Add_To_Database(kActorChew, kClueDNATyrell, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorChew, kClueDNASebastian, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorChew, kClueDNAMoraji, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorChew, kClueDNAMarcus, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorChew, kClueCrazylegsInterview2, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorChew, kCluePowerSource, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorChew, kClueSightingMcCoyRuncitersShop, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueGuzzaFramedMcCoy, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueVKDektoraReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueVKBobGorskyReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueVKLutherLanceReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueVKGrigorianReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueVKIzoReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueVKCrazylegsReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueVKRunciterReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueVKEarlyQReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueMcCoyIncept, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kCluePoliceWeaponUsed, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueMcCoysWeaponUsedonBob, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueMcCoyRecoveredHoldensBadge, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueCar, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueMcCoyHelpedLucy, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueMcCoyIsKind, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueMcCoyShotZubenInTheBack, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueMcCoyKilledRunciter1, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueMcCoyKilledRunciter2, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueMcCoyLetZubenEscape, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueMcCoyWarnedIzo, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueMcCoyHelpedIzoIzoIsAReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueMcCoyHelpedDektora, 95, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueMcCoyHelpedGordo, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueMcCoyShotGuzza, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueMcCoyRetiredLucy, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueMcCoyRetiredGordo, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueMcCoyRetiredDektora, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueMcCoyRetiredSadik, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueMcCoyRetiredLutherLance, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueMcCoyRetiredZuben, 90, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueClovisOrdersMcCoysDeath, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueMcCoyIsInsane, 95, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueMcCoyIsStupid, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueSightingSadikBradbury, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueSightingMcCoyRuncitersShop, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueWeaponsOrderForm, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueHollowayInterview, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueRunciterConfession1, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGaff, kClueRunciterConfession2, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueMcCoyIsStupid, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueMcCoyIsAnnoying, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueMcCoyIsKind, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueMcCoyIsInsane, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueMcCoyRetiredZuben, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueMcCoyShotZubenInTheBack, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueMcCoyKilledRunciter1, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueMcCoyLetZubenEscape, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueMcCoyWarnedIzo, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueMcCoyHelpedIzoIzoIsAReplicant, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueMcCoyHelpedDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueMcCoyHelpedLucy, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueMcCoyHelpedGordo, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueMcCoyShotGuzza, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueMcCoyRetiredLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueMcCoyRetiredDektora, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueMcCoyRetiredLutherLance, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueMcCoyBetrayal, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueMcCoyKilledRunciter2, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueClovisOrdersMcCoysDeath, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueWeaponsOrderForm, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueHollowayInterview, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueRunciterConfession1, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueRunciterConfession2, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueMcCoyRetiredSadik, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueMcCoyIsABladeRunner, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kCluePowerSource, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueCrystalArrestedCrazylegs, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorTaffy, kClueSightingMcCoyRuncitersShop, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueMcCoyRetiredZuben, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueMcCoyShotZubenInTheBack, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueMcCoyIsAnnoying, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueMcCoyIsKind, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueMcCoyIsInsane, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueMcCoyKilledRunciter1, 40, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueMcCoysDescription, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueMcCoyIsABladeRunner, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueMcCoyLetZubenEscape, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueMcCoyWarnedIzo, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueMcCoyHelpedIzoIzoIsAReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueMcCoyHelpedGordo, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueMcCoyHelpedLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueMcCoyHelpedDektora, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueMcCoyShotGuzza, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueMcCoyRetiredLucy, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueMcCoyRetiredDektora, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueMcCoyRetiredLutherLance, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueMcCoyBetrayal, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueMcCoyKilledRunciter2, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueClovisOrdersMcCoysDeath, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueEarlyAttemptedToSeduceLucy, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueMcCoyIsStupid, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueHowieLeeInterview, 30, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueIzoInterview, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueGordoInterview1, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueGordoInterview2, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueEarlyQInterview, 25, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueSuspectDektora, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueZubenInterview, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueGordoInterview3, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueDektoraInterview4, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueIzosStashRaided, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueGordoBlabs, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueDektoraConfession, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueDNATyrell, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueDNAChew, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueDNAMoraji, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueDNALutherLance, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueDNAMarcus, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueCrazylegsInterview1, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueCrazylegsInterview2, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kCluePowerSource, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueCrystalArrestedCrazylegs, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorSebastian, kClueSightingMcCoyRuncitersShop, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRachael, kClueMcCoyIsKind, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRachael, kClueMcCoyShotZubenInTheBack, 40, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRachael, kClueMcCoyLetZubenEscape, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRachael, kClueMcCoyWarnedIzo, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRachael, kClueMcCoyHelpedIzoIzoIsAReplicant, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRachael, kClueMcCoyHelpedDektora, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRachael, kClueMcCoyHelpedLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRachael, kClueMcCoyHelpedGordo, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRachael, kClueMcCoyShotGuzza, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRachael, kClueMcCoyRetiredLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRachael, kClueMcCoyRetiredDektora, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRachael, kClueMcCoyRetiredSadik, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRachael, kClueMcCoyRetiredLutherLance, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRachael, kClueClovisOrdersMcCoysDeath, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRachael, kClueMcCoyIsInsane, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRachael, kClueMcCoyIsABladeRunner, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRachael, kClueMcCoyBetrayal, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRachael, kClueEarlyAttemptedToSeduceLucy, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRachael, kCluePowerSource, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorRachael, kClueSightingMcCoyRuncitersShop, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGeneralDoll, kClueMcCoyIsAnnoying, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGeneralDoll, kClueMcCoyIsKind, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGeneralDoll, kClueMcCoyIsInsane, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGeneralDoll, kClueMcCoyIsStupid, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGeneralDoll, kClueMcCoyIsABladeRunner, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueMcCoyShotZubenInTheBack, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueMcCoyIsAnnoying, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueMcCoyIsKind, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueMcCoyIsInsane, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueMcCoyKilledRunciter1, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueMcCoysDescription, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueMcCoyIsABladeRunner, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueMcCoyLetZubenEscape, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueMcCoyWarnedIzo, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueMcCoyHelpedIzoIzoIsAReplicant, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueMcCoyHelpedGordo, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueMcCoyHelpedLucy, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueMcCoyHelpedDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueMcCoyShotGuzza, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueMcCoyRetiredZuben, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueMcCoyRetiredLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueMcCoyRetiredDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueMcCoyRetiredSadik, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueMcCoyRetiredLutherLance, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueMcCoyKilledRunciter2, 35, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueSuspectDektora, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueEarlyAttemptedToSeduceLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueMcCoyIsStupid, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueGordoBlabs, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueDNATyrell, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueDNASebastian, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueDNAChew, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueDNAMoraji, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueDNALutherLance, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueDNAMarcus, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueCrazylegsInterview1, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueCrazylegsInterview2, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kCluePowerSource, 50, false, false, -1);
	Actor_Clue_Add_To_Database(kActorIsabella, kClueSightingMcCoyRuncitersShop, 60, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueMcCoyIsStupid, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueMcCoyIsAnnoying, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueMcCoyIsKind, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueMcCoyIsInsane, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueMcCoyRetiredZuben, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueMcCoyShotZubenInTheBack, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueMcCoyKilledRunciter1, 40, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueMcCoyLetZubenEscape, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueMcCoyWarnedIzo, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueMcCoyHelpedIzoIzoIsAReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueMcCoyHelpedDektora, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueMcCoyHelpedLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueMcCoyHelpedGordo, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueMcCoyShotGuzza, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueMcCoyRetiredLucy, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueMcCoyRetiredDektora, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueMcCoyRetiredSadik, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueMcCoyRetiredLutherLance, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueMcCoyBetrayal, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueMcCoyKilledRunciter2, 45, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueClovisOrdersMcCoysDeath, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueVKDektoraReplicant, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueVKBobGorskyReplicant, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueVKLutherLanceReplicant, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueVKGrigorianReplicant, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueVKIzoReplicant, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueVKCrazylegsReplicant, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueVKRunciterReplicant, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueVKEarlyQReplicant, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueOfficersStatement, 40, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueSightingSadikBradbury, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueSightingMcCoyRuncitersShop, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueWeaponsOrderForm, 40, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueHollowayInterview, 55, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueRunciterConfession1, 40, false, false, -1);
	Actor_Clue_Add_To_Database(kActorLeon, kClueRunciterConfession2, 40, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMaggie, kClueMcCoyIsAnnoying, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMaggie, kClueMcCoyIsKind, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMaggie, kClueMcCoyIsInsane, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMaggie, kClueMcCoyIsStupid, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorMaggie, kClueMcCoyIsABladeRunner, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGenwalkerA, kClueMcCoyIsAnnoying, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGenwalkerA, kClueMcCoyIsKind, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGenwalkerA, kClueMcCoyIsInsane, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGenwalkerA, kClueMcCoyIsStupid, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGenwalkerA, kClueMcCoyIsABladeRunner, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGenwalkerB, kClueMcCoyIsAnnoying, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGenwalkerB, kClueMcCoyIsKind, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGenwalkerB, kClueMcCoyIsInsane, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGenwalkerB, kClueMcCoyIsStupid, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGenwalkerB, kClueMcCoyIsABladeRunner, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGenwalkerC, kClueMcCoyIsAnnoying, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGenwalkerC, kClueMcCoyIsKind, 70, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGenwalkerC, kClueMcCoyIsInsane, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGenwalkerC, kClueMcCoyIsStupid, 65, false, false, -1);
	Actor_Clue_Add_To_Database(kActorGenwalkerC, kClueMcCoyIsABladeRunner, 65, false, false, -1);
}

void InitScript::Init_World_Waypoints() {
	World_Waypoint_Set(  0, kSetDR01_DR02_DR04, -676.0f, -0.04f, -94.0f);
	World_Waypoint_Set(  1, kSetDR01_DR02_DR04, -807.0f, -0.04f, 109.0f);
	World_Waypoint_Set(  2, kSetPS05, 541.8f, 0.38f, -435.68f);
	World_Waypoint_Set(  3, kSetPS07, 561.01f, 0.34f, -606.67f);
	World_Waypoint_Set(  4, kSetPS10_PS11_PS12_PS13, -404.09f, -9.23f, 251.95f);
	World_Waypoint_Set(  5, kSetPS10_PS11_PS12_PS13, -99.0f, -9.23f, 690.0f);
	World_Waypoint_Set(  6, kSetPS10_PS11_PS12_PS13, -374.14f, -8.97f, 240.18f);
	World_Waypoint_Set(  7, kSetPS10_PS11_PS12_PS13, -766.02f, -8.82f, 271.44f);
	World_Waypoint_Set(  8, kSetPS10_PS11_PS12_PS13, -546.19f, -9.06f, 351.38f);
	World_Waypoint_Set(  9, kSetPS10_PS11_PS12_PS13, -522.66f, -8.6f, 1409.29f);
	World_Waypoint_Set( 10, kSetPS10_PS11_PS12_PS13, -324.21f, -9.01f, 1428.74f);
	World_Waypoint_Set( 11, kSetPS10_PS11_PS12_PS13, 23.72f, -8.87f, 1335.19f);
	World_Waypoint_Set( 12, kSetRC01, -132.0f, 6.09f, 91.0f);
	World_Waypoint_Set( 13, kSetDR01_DR02_DR04, 21.4f, 0.22f, -201.68f);
	World_Waypoint_Set( 14, kSetDR01_DR02_DR04, 164.44f, 0.29f, -265.69f);
	World_Waypoint_Set( 15, kSetDR01_DR02_DR04, 279.7f, 7.23f, -888.43f);
	World_Waypoint_Set( 16, kSetPS10_PS11_PS12_PS13, 41.35f, -8.98f, 556.2f);
	World_Waypoint_Set( 17, kSetPS10_PS11_PS12_PS13, -697.86f, -0.73f, 21.89f);
	World_Waypoint_Set( 18, kSetPS10_PS11_PS12_PS13, -678.17f, -0.77f, 1043.62f);
	World_Waypoint_Set( 19, kSetPS10_PS11_PS12_PS13, 116.89f, -0.74f, 1581.12f);
	World_Waypoint_Set( 20, kSetDR01_DR02_DR04, -312.92f, 0.17f, -345.2f);
	World_Waypoint_Set( 21, kSetDR01_DR02_DR04, -290.04f, 0.23f, -513.79f);
	World_Waypoint_Set( 22, kSetDR01_DR02_DR04, 6.97f, 0.54f, -759.56f);
	World_Waypoint_Set( 23, kSetDR01_DR02_DR04, 280.48f, 11.58f, -941.15f);
	World_Waypoint_Set( 24, kSetDR01_DR02_DR04, 231.14f, 7.14f, -688.96f);
	World_Waypoint_Set( 25, kSetDR01_DR02_DR04, 54.92f, 0.2f, -171.75f);
	World_Waypoint_Set( 26, kSetDR01_DR02_DR04, -56.77f, 0.18f, -166.99f);
	World_Waypoint_Set( 27, kSetDR01_DR02_DR04, -78.12f, 0.34f, -449.92f);
	World_Waypoint_Set( 28, kSetRC01, -30.0f, -625.51f, 366.15f);
	World_Waypoint_Set( 29, kSetRC01, -51.81f, -622.47f, 286.93f);
	World_Waypoint_Set( 30, kSetRC01, -320.58f, -625.53f, 301.58f);
	World_Waypoint_Set( 31, kSetPS07, 421.01f, 0.22f, -566.67f);
	World_Waypoint_Set( 32, kSetPS07, 336.0f, 0.22f, -520.0f);
	World_Waypoint_Set( 33, kSetFreeSlotA, 0.0f, 0.0f, 0.0f);
	World_Waypoint_Set( 34, kSetFreeSlotB, 0.0f, 0.0f, 0.0f);
	World_Waypoint_Set( 35, kSetFreeSlotC, 0.0f, 0.0f, 0.0f);
	World_Waypoint_Set( 36, kSetFreeSlotD, 0.0f, 0.0f, 0.0f);
	World_Waypoint_Set( 37, kSetFreeSlotE, 0.0f, 0.0f, 0.0f);
	World_Waypoint_Set( 38, kSetFreeSlotF, 0.0f, 0.0f, 0.0f);
	World_Waypoint_Set( 39, kSetFreeSlotG, 0.0f, 0.0f, 0.0f);
	World_Waypoint_Set( 40, kSetFreeSlotH, 0.0f, 0.0f, 0.0f);
	World_Waypoint_Set( 41, kSetFreeSlotI, 0.0f, 0.0f, 0.0f);
	World_Waypoint_Set( 42, kSetFreeSlotJ, 0.0f, 0.0f, 0.0f);
	World_Waypoint_Set( 43, kSetCT01_CT12, -427.0f, -6.5f, 1188.0f);
	World_Waypoint_Set( 44, kSetCT01_CT12, -255.2f, -6.5f, 455.2f);
	World_Waypoint_Set( 45, kSetCT02, -247.02f, -145.11f, 32.99f);
	World_Waypoint_Set( 46, kSetCT02, -154.83f, -145.11f, 9.39f);
	World_Waypoint_Set( 47, kSetCT03_CT04, -619.36f, -616.15f, 220.91f);
	World_Waypoint_Set( 48, kSetCT03_CT04, -82.86f, -621.3f, 769.03f);
	World_Waypoint_Set( 49, kSetCT06, -7.31f, -58.23f, 22.44f);
	World_Waypoint_Set( 50, kSetCT06, 132.16f, -58.23f, 767.0f);
	World_Waypoint_Set( 51, kSetCT03_CT04, -335.05f, -618.82f, 312.9f);
	World_Waypoint_Set( 52, kSetCT07, 189.7f, -58.23f, -4.72f);
	World_Waypoint_Set( 53, kSetCT01_CT12, -450.32f, -6.5f, 230.39f);
	World_Waypoint_Set( 54, kSetCT01_CT12, -70.04f, -6.5f, 150.17f);
	World_Waypoint_Set( 55, kSetPS07, 491.0f, 0.0f, -571.0f);
	World_Waypoint_Set( 56, kSetCT01_CT12, -221.68f, -6.5f, 150.15f);
	World_Waypoint_Set( 57, kSetRC01, -291.43f, -0.3f, 277.92f);
	World_Waypoint_Set( 58, kSetRC01, -272.91f, -0.3f, 369.1f);
	World_Waypoint_Set( 59, kSetDR01_DR02_DR04, -118.65f, 0.15f, -130.15f);
	World_Waypoint_Set( 60, kSetDR01_DR02_DR04, 22.27f, 0.15f, -69.81f);
	World_Waypoint_Set( 61, kSetRC02_RC51, -39.0f, -1238.0f, 108284.0f);
	World_Waypoint_Set( 62, kSetPS02, -11.0f, -40.0f, -45.0f);
	World_Waypoint_Set( 63, kSetCT03_CT04, -133.0f, -621.0f, 686.0f);
	World_Waypoint_Set( 64, kSetCT01_CT12, -360.0f, -6.13f, 380.0f);
	World_Waypoint_Set( 65, kSetPS05, 688.0f, 0.37f, -518.0f);
	World_Waypoint_Set( 66, kSetCT03_CT04, -83.0f, -621.0f, 627.0f);
	World_Waypoint_Set( 67, kSetCT01_CT12, -212.65f, -2.08f, 513.47f);
	World_Waypoint_Set( 68, kSetCT01_CT12, -219.43f, -2.08f, 584.8f);
	World_Waypoint_Set( 69, kSetCT01_CT12, -215.0f, -2.08f, 548.0f);
	// 70 - UNUSED
	World_Waypoint_Set( 71, kSetRC01, 210.0f, 5.55f, 146.19f);
	World_Waypoint_Set( 72, kSetRC01, -55.27f, 5.55f, 108.34f);
	World_Waypoint_Set( 73, kSetPS07, 338.75f, 0.22f, -612.0f);
	World_Waypoint_Set( 74, kSetPS07, 338.75f, 0.22f, -560.0f);
	World_Waypoint_Set( 75, kSetCT03_CT04, -138.45f, -621.3f, 778.52f);
	World_Waypoint_Set( 76, kSetPS03, -499.23f, -354.62f, -51.3f);
	World_Waypoint_Set( 77, kSetPS03, -903.0f, -354.62f, 676.0f);
	World_Waypoint_Set( 78, kSetPS03, -723.0f, -354.62f, -1272.0f);
	World_Waypoint_Set( 79, kSetPS09, 207.36f, 0.67f, -96.42f);
	World_Waypoint_Set( 80, kSetPS09, -134.43f, 0.43f, -180.46f);
	World_Waypoint_Set( 81, kSetPS09, -559.0f, 0.15f, -100.0f);
	World_Waypoint_Set( 82, kSetPS03, -1250.07f, -354.0f, -1186.9f);
	World_Waypoint_Set( 83, kSetRC02_RC51, -55.11f, -1238.89f, 107995.87f);
	World_Waypoint_Set( 84, kSetCT02, -161.62f, -145.11f, -53.73f);
	World_Waypoint_Set( 85, kSetCT02, -201.62f, -145.11f, -85.73f);
	World_Waypoint_Set( 86, kSetCT01_CT12, -171.55f, -2.08f, 361.01f);
	World_Waypoint_Set( 87, kSetCT01_CT12, -523.51f, -9.23f, 1384.76f);
	World_Waypoint_Set( 88, kSetCT01_CT12, -102.01f, -9.23f, 1375.38f);
	World_Waypoint_Set( 89, kSetRC02_RC51, 14.54f, -1238.89f, 108280.85f);
	World_Waypoint_Set( 90, kSetRC02_RC51, 37.59f, -1238.89f, 108449.29f);
	World_Waypoint_Set( 91, kSetRC02_RC51, 9.68f, -1238.89f, 108427.73f);
	World_Waypoint_Set( 92, kSetRC02_RC51, -153.29f, -1238.89f, 108473.52f);
	World_Waypoint_Set( 93, kSetRC02_RC51, -104.0f, -1238.89f, 108413.0f);
	World_Waypoint_Set( 94, kSetCT07, 302.32f, -58.23f, 35.14f);
	World_Waypoint_Set( 95, kSetDR05, 62.0f, 0.3f, 129.0f);
	World_Waypoint_Set( 96, kSetDR05, -134.63f, -0.3f, 171.41f);
	World_Waypoint_Set( 97, kSetDR01_DR02_DR04, -1135.0f, 6.98f, 441.0f);
	World_Waypoint_Set( 98, kSetDR01_DR02_DR04, -1015.0f, 7.18f, 354.75f);
	World_Waypoint_Set( 99, kSetDR01_DR02_DR04, -975.0f, -0.04f, 316.0f);
	World_Waypoint_Set(100, kSetCT01_CT12, -334.46f, -6.5f, 500.64f);
	World_Waypoint_Set(101, kSetDR01_DR02_DR04, -334.46f, -6.5f, 500.64f);
	World_Waypoint_Set(102, kSetRC02_RC51, 27.89f, -1238.89f, 108288.73f);
	World_Waypoint_Set(103, kSetRC04, 48.31f, 0.15f, 17.11f);
	World_Waypoint_Set(104, kSetRC04, 4.31f, 0.15f, -39.0f);
	World_Waypoint_Set(105, kSetCT03_CT04, -764.58f, -616.31f, 229.6f);
	World_Waypoint_Set(106, kSetCT01_CT12, -25.0f, -6.5f, 352.28f);
	World_Waypoint_Set(107, kSetRC04, -3.6f, -621.79f, 164.09f);
	World_Waypoint_Set(108, kSetRC04, 86.03f, -622.47f, 73.21f);
	World_Waypoint_Set(109, kSetDR01_DR02_DR04, -793.0f, -0.04f, 164.0f);
	World_Waypoint_Set(110, kSetDR01_DR02_DR04, -665.0f, -0.04f, 304.0f);
	World_Waypoint_Set(111, kSetDR01_DR02_DR04, -765.0f, -0.04f, 232.0f);
	World_Waypoint_Set(112, kSetDR01_DR02_DR04, -817.0f, -0.04f, 300.0f);
	World_Waypoint_Set(113, kSetDR01_DR02_DR04, -907.0f, -0.04f, 304.0f);
	World_Waypoint_Set(114, kSetBB01, -4.0f, 0.0f, 880.0f);
	World_Waypoint_Set(115, kSetBB01, 174.0f, 0.0f, 890.15f);
	World_Waypoint_Set(116, kSetBB01, 69.0f, 0.0f, 695.0f);
	World_Waypoint_Set(117, kSetBB01, 0.0f, 0.0f, 0.0f);
	World_Waypoint_Set(118, kSetCT05, -376.35f, -109.91f, 604.4f);
	World_Waypoint_Set(119, kSetCT05, -375.0f, -109.91f, 750.0f);
	World_Waypoint_Set(120, kSetAR01_AR02, -50.81f, 2.5f, 233.0f);
	World_Waypoint_Set(121, kSetAR01_AR02, -50.81f, 2.5f, 31.03f);
	World_Waypoint_Set(122, kSetAR01_AR02, 28.12f, 2.5f, 100.64f);
	World_Waypoint_Set(123, kSetCT01_CT12, -474.28f, -6.5f, 979.59f);
	World_Waypoint_Set(124, kSetMA01, 8.74f, 0.0f, -282.81f);
	World_Waypoint_Set(125, kSetMA01, 978.98f, 0.0f, 145.64f);
	World_Waypoint_Set(126, kSetMA01, 477.18f, 0.0f, -287.21f);
	World_Waypoint_Set(127, kSetBB11, 31.39f, -10.27f, -64.52f);
	World_Waypoint_Set(128, kSetBB11, 7.39f, -10.27f, -136.52f);
	World_Waypoint_Set(127, kSetBB11, 102.98f, -30.89f, -121.02f); // a bug? Overwrites previous co-ordinates (this is the final value). Note: The 127 waypoint is UNUSED!
	World_Waypoint_Set(128, kSetBB11, -20.0f, -30.89f, -121.02f);  // a bug? Overwrites previous co-ordinates (this is the final value). Note: The 128 waypoint is UNUSED!
	World_Waypoint_Set(129, kSetBB11, -136.61f, -10.27f, -136.52f);
	World_Waypoint_Set(130, kSetBB11, -36.61f, -10.27f, -136.52f);
	World_Waypoint_Set(131, kSetBB09, 435.45f, -9.0f, 166.0f);
	World_Waypoint_Set(132, kSetBB09, 619.45f, -9.0f, 234.0f);
	World_Waypoint_Set(133, kSetBB09, 619.45f, -9.0f, 270.0f);
	World_Waypoint_Set(134, kSetBB05, -80.59f, -60.31f, 256.35f);
	World_Waypoint_Set(135, kSetBB05, -48.0f, -60.31f, 183.0f);
	World_Waypoint_Set(136, kSetBB05, -24.59f, -60.31f, 64.35f);
	World_Waypoint_Set(137, kSetBB05, 99.41f, -60.31f, 4.35f);
	World_Waypoint_Set(138, kSetBB05, 99.41f, -60.34f, -115.65f);
	World_Waypoint_Set(139, kSetBB05, 147.41f, -60.34f, -115.65f);
	World_Waypoint_Set(140, kSetBB06_BB07, -43.88f, -0.04f, 172.95f);
	World_Waypoint_Set(141, kSetBB06_BB07, 78.36f, -0.04f, 80.79f);
	World_Waypoint_Set(142, kSetBB06_BB07, 81.74f, -0.04f, -94.0f);
	World_Waypoint_Set(143, kSetBB06_BB07, -118.26f, -0.04f, -94.04f);
	World_Waypoint_Set(144, kSetBB07, -654.56f, 252.59f, -1110.88f);
	World_Waypoint_Set(145, kSetBB07, -578.56f, 252.59f, -1010.88f);
	World_Waypoint_Set(146, kSetBB07, -470.56f, 252.59f, -1070.88f);
	World_Waypoint_Set(147, kSetBB07, -510.56f, 252.59f, -1006.88f);
	World_Waypoint_Set(148, kSetBB07, -646.56f, 252.59f, -1006.88f);
	World_Waypoint_Set(149, kSetHC01_HC02_HC03_HC04, 647.0f, 1.6f, -81.87f);
	World_Waypoint_Set(150, kSetUG02, -269.0f, 120.16f, -88.0f);
	World_Waypoint_Set(151, kSetUG02, -181.0f, 120.16f, -96.0f);
	World_Waypoint_Set(152, kSetUG02, -133.0f, 84.13f, -108.0f);
	World_Waypoint_Set(153, kSetUG02, -95.0f, 74.87f, -503.0f);
	World_Waypoint_Set(154, kSetRC03, -172.6f, 1.72f, 87.62f);
	World_Waypoint_Set(155, kSetAR01_AR02, -284.0f, 0.0f, 296.0f);
	World_Waypoint_Set(156, kSetAR01_AR02, -680.0f, 0.0f, -156.0f);
	World_Waypoint_Set(157, kSetAR01_AR02, -702.0f, 0.0f, -919.0f);
	World_Waypoint_Set(158, kSetAR01_AR02, 140.0f, 0.0f, -1233.0f);
	World_Waypoint_Set(159, kSetAR01_AR02, -228.0f, 0.0f, -92.0f);
	World_Waypoint_Set(160, kSetAR01_AR02, -274.0f, 0.0f, -627.0f);
	World_Waypoint_Set(161, kSetAR01_AR02, -329.27f, 0.0f, -1115.14f);
	World_Waypoint_Set(162, kSetHC01_HC02_HC03_HC04, 815.34f, 0.14f, 165.21f);
	World_Waypoint_Set(163, kSetHC01_HC02_HC03_HC04, -35.0f, 0.14f, 39.0f);
	World_Waypoint_Set(164, kSetHC01_HC02_HC03_HC04, -24.0f, 0.14f, -631.0f);
	World_Waypoint_Set(165, kSetHC01_HC02_HC03_HC04, -125.0f, 0.14f, -221.0f);
	World_Waypoint_Set(166, kSetHC01_HC02_HC03_HC04, 456.82f, 0.14f, 69.0f);
	World_Waypoint_Set(167, kSetRC03, -815.0f, -4.01f, 96.0f);
	World_Waypoint_Set(168, kSetRC03, -235.0f, 1.72f, 92.0f);
	World_Waypoint_Set(169, kSetRC03, 5.0f, 1.72f, 92.0f);
	World_Waypoint_Set(170, kSetRC03, 265.0f, 1.72f, 36.0f);
	World_Waypoint_Set(171, kSetRC03, -639.0f, 1.72f, -124.0f);
	World_Waypoint_Set(172, kSetHC01_HC02_HC03_HC04, -225.0f, 1.0f, 39.0f);
	World_Waypoint_Set(172, kSetHC01_HC02_HC03_HC04, -217.0f, 1.0f, 127.0f); // a bug? Overwrites previous co-ordinates (this is the final value). Note: The 172 waypoint is UNUSED!
	// 173 - UNUSED
	World_Waypoint_Set(174, kSetRC03, 326.96f, -4.01f, 383.16f);
	World_Waypoint_Set(175, kSetRC03, 264.43f, -4.01f, 313.73f);
	World_Waypoint_Set(176, kSetUG06, -78.43f, 0.0f, 269.98f);
	World_Waypoint_Set(177, kSetUG06, 19.0f, 0.0f, 269.98f);
	World_Waypoint_Set(178, kSetUG06, 91.0f, 0.0f, 137.6f);
	World_Waypoint_Set(179, kSetUG06, 40.14f, 0.0f, 276.62f);
	World_Waypoint_Set(180, kSetUG06, -71.86f, 0.0f, 276.62f);
	World_Waypoint_Set(181, kSetUG06, -112.56f, 0.0f, 228.03f);
	World_Waypoint_Set(182, kSetUG01, 143.45f, -50.13f, -12.22f);
	World_Waypoint_Set(183, kSetUG01, 199.45f, -50.13f, -1400.22f);
	World_Waypoint_Set(184, kSetUG01, -112.55f, -50.13f, -2360.22f);
	World_Waypoint_Set(185, kSetUG09, 115.0f, 156.94f, -310.0f);
	World_Waypoint_Set(186, kSetUG09, 153.0f, 156.94f, -294.0f);
	World_Waypoint_Set(187, kSetUG14, -295.0f, 12.97f, -148.0f);
	World_Waypoint_Set(188, kSetUG14, 157.0f, 129.0f, -504.0f);
	World_Waypoint_Set(189, kSetUG14, 229.0f, 186.04f, -24.0f);
	World_Waypoint_Set(190, kSetUG14, 157.0f, 186.04f, -24.0f);
	World_Waypoint_Set(191, kSetUG14, 157.0f, 128.92f, -148.0f);
	World_Waypoint_Set(192, kSetTB02_TB03, -136.19f, 0.0f, 1580.03f);
	World_Waypoint_Set(193, kSetTB02_TB03, -308.0f, -81.46f, 1466.0f);
	World_Waypoint_Set(194, kSetBB03, 82.26f, 60.16f, -124.35f);
	World_Waypoint_Set(195, kSetBB03, 226.1f, 60.16f, -139.84f);
	World_Waypoint_Set(196, kSetBB05, 39.41f, -60.31f, 308.35f);
	World_Waypoint_Set(197, kSetBB05, 99.41f, -60.31f, 220.35f);
	World_Waypoint_Set(198, kSetBB05, 267.41f, -60.31f, 180.35f);
	// 199 - UNUSED
	World_Waypoint_Set(200, kSetBB06_BB07, -44.46f, -0.04f, 177.4f);
	World_Waypoint_Set(201, kSetBB06_BB07, 137.0f, -0.04f, 17.0f);
	World_Waypoint_Set(202, kSetRC03, -610.0f, -4.01f, 237.11f);
	World_Waypoint_Set(203, kSetRC03, -368.96f, -4.01f, 237.11f);
	World_Waypoint_Set(204, kSetHC01_HC02_HC03_HC04, 19.0f, 0.14f, 83.0f);
	World_Waypoint_Set(205, kSetHC01_HC02_HC03_HC04, -58.36f, 0.14f, 4.4f);
	World_Waypoint_Set(206, kSetHC01_HC02_HC03_HC04, -18.11f, 0.14f, -669.45f);
	World_Waypoint_Set(207, kSetHC01_HC02_HC03_HC04, -18.11f, 0.14f, -669.45f);
	World_Waypoint_Set(208, kSetHC01_HC02_HC03_HC04, -162.25f, 0.14f, -511.93f);
	World_Waypoint_Set(209, kSetHC01_HC02_HC03_HC04, -128.25f, 0.14f, -322.0f);
	World_Waypoint_Set(210, kSetHC01_HC02_HC03_HC04, 714.48f, 0.14f, 14.92f);
	World_Waypoint_Set(211, kSetHC01_HC02_HC03_HC04, 23.0f, 0.14f, -1.0f);
	World_Waypoint_Set(212, kSetHC01_HC02_HC03_HC04, 28.47f, 0.14f, 3.8f);
	World_Waypoint_Set(213, kSetHC01_HC02_HC03_HC04, 36.47f, 0.14f, 55.89f);
	World_Waypoint_Set(214, kSetHC01_HC02_HC03_HC04, 155.75f, 0.14f, 54.0f);
	World_Waypoint_Set(215, kSetAR01_AR02, -70.0f, 0.0f, -647.0f);
	World_Waypoint_Set(216, kSetAR01_AR02, -270.01f, 0.0f, -441.68f);
	World_Waypoint_Set(217, kSetAR01_AR02, -209.98f, 0.0f, -483.05f);
	World_Waypoint_Set(218, kSetAR01_AR02, -428.08f, 0.0f, -110.16f);
	World_Waypoint_Set(219, kSetAR01_AR02, 256.0f, 0.0f, -298.08f);
	World_Waypoint_Set(220, kSetAR01_AR02, -187.18f, 0.0f, -298.08f);
	World_Waypoint_Set(221, kSetAR01_AR02, -428.08f, 0.0f, -110.16f);
	World_Waypoint_Set(222, kSetAR01_AR02, -466.0f, 0.0f, -635.0f);
	World_Waypoint_Set(223, kSetAR01_AR02, -382.0f, 0.0f, -1099.0f);
	World_Waypoint_Set(224, kSetAR01_AR02, -227.0f, 0.0f, -1333.0f);
	World_Waypoint_Set(225, kSetAR01_AR02, 140.88f, 0.0f, -1362.34f);
	World_Waypoint_Set(226, kSetAR01_AR02, -448.18f, 0.0f, -626.38f);
	World_Waypoint_Set(227, kSetAR01_AR02, -444.18f, 0.0f, -730.38f);
	World_Waypoint_Set(228, kSetBB01, -198.02f, 9.04f, 487.7f);
	World_Waypoint_Set(229, kSetBB01, -147.4f, 9.04f, 918.08f);
	World_Waypoint_Set(230, kSetBB01, -201.67f, 9.04f, 829.09f);
	World_Waypoint_Set(231, kSetBB01, -177.67f, 9.04f, 829.09f);
	World_Waypoint_Set(232, kSetCT01_CT12, -92.52f, -6.5f, 714.44f);
	World_Waypoint_Set(233, kSetCT01_CT12, -352.52f, -6.5f, 714.44f);
	World_Waypoint_Set(234, kSetCT01_CT12, -352.52f, -6.5f, 666.44f);
	World_Waypoint_Set(235, kSetCT01_CT12, -136.41f, -6.5f, 735.26f);
	World_Waypoint_Set(236, kSetCT01_CT12, -248.41f, -6.5f, 747.26f);
	World_Waypoint_Set(237, kSetCT01_CT12, -352.52f, -6.5f, 252.0f);
	World_Waypoint_Set(238, kSetCT01_CT12, -190.25f, -6.5f, 274.58f);
	World_Waypoint_Set(239, kSetCT11, -371.87f, 0.0f, 275.89f);
	World_Waypoint_Set(240, kSetCT11, -371.87f, 0.0f, -60.11f);
	World_Waypoint_Set(241, kSetCT11, 588.5f, 0.0f, 254.19f);
	World_Waypoint_Set(242, kSetCT11, 560.5f, 0.0f, 254.19f);
	World_Waypoint_Set(243, kSetDR01_DR02_DR04, -153.77f, -0.01f, -1037.98f);
	World_Waypoint_Set(244, kSetDR01_DR02_DR04, 398.23f, 6.98f, -1037.98f);
	World_Waypoint_Set(245, kSetDR01_DR02_DR04, 40.78f, 7.22f, -943.72f);
	World_Waypoint_Set(246, kSetDR01_DR02_DR04, 68.78f, -0.01f, -943.72f);
	World_Waypoint_Set(247, kSetDR01_DR02_DR04, 96.78f, -0.01f, -973.72f);
	World_Waypoint_Set(248, kSetPS03, -897.38f, -354.62f, 704.77f);
	World_Waypoint_Set(249, kSetPS03, -914.76f, -354.62f, -312.43f);
	World_Waypoint_Set(250, kSetPS03, -457.54f, -354.62f, -820.15f);
	World_Waypoint_Set(251, kSetPS05, 556.72f, 0.37f, -141.26f);
	World_Waypoint_Set(252, kSetPS05, 635.66f, 0.37f, -594.11f);
	World_Waypoint_Set(253, kSetPS09, 130.42f, 0.0f, -79.98f);
	World_Waypoint_Set(254, kSetPS09, -311.15f, 0.0f, -161.06f);
	World_Waypoint_Set(255, kSetPS09, -403.15f, 0.0f, -161.06f);
	World_Waypoint_Set(256, kSetPS09, -487.15f, 0.0f, -137.11f);
	World_Waypoint_Set(257, kSetPS09, -611.15f, 0.0f, -73.06f);
	World_Waypoint_Set(258, kSetHC01_HC02_HC03_HC04, 37.64f, 0.14f, -48.02f);
	World_Waypoint_Set(259, kSetHC01_HC02_HC03_HC04, 109.64f, 0.14f, 91.98f);
	World_Waypoint_Set(260, kSetHC01_HC02_HC03_HC04, -149.0f, 0.14f, 79.0f);
	World_Waypoint_Set(261, kSetHC01_HC02_HC03_HC04, -129.0f, 0.14f, -237.0f);
	World_Waypoint_Set(262, kSetHC01_HC02_HC03_HC04, -1.0f, 0.14f, -671.0f);
	World_Waypoint_Set(263, kSetPS04, -728.0f, -354.0f, 1090.0f);
	World_Waypoint_Set(264, kSetMA02_MA04, -8.41f, -144.0f, 343.0f);
	World_Waypoint_Set(265, kSetMA02_MA04, -20.81f, -144.0f, 450.0f);
	World_Waypoint_Set(266, kSetMA02_MA04, -200.0f, -144.0f, 206.0f);
	World_Waypoint_Set(267, kSetMA02_MA04, -17.0f, -144.0f, 178.0f);
	World_Waypoint_Set(268, kSetMA04, -7207.0f, 955.5f, 1852.75f);
	World_Waypoint_Set(269, kSetMA04, -7191.0f, 955.5f, 1700.75f);
	World_Waypoint_Set(270, kSetMA04, 7116.0f, 955.5f, 1871.0f);
	World_Waypoint_Set(271, kSetDR01_DR02_DR04, -1139.89f, -0.04f, 67.89f);
	World_Waypoint_Set(272, kSetDR01_DR02_DR04, -690.5f, -0.04f, -210.48f);
	World_Waypoint_Set(273, kSetDR01_DR02_DR04, -495.89f, -0.04f, -204.11f);
	World_Waypoint_Set(274, kSetRC01, -511.75f, 5.55f, 55.63f);
	World_Waypoint_Set(275, kSetRC01, 296.21f, 5.55f, 59.63f);
	World_Waypoint_Set(276, kSetTB06, -34.57f, 149.42f, -502.83f);
	World_Waypoint_Set(277, kSetTB06, 51.0f, 149.42f, -487.27f);
	World_Waypoint_Set(278, kSetTB06, 82.0f, 149.42f, -519.0f);
	World_Waypoint_Set(279, kSetTB06, 95.97f, 149.42f, -549.51f);
	World_Waypoint_Set(280, kSetTB06, -34.0f, 149.42f, -551.0f);
	World_Waypoint_Set(281, kSetDR01_DR02_DR04, -2060.99f, -0.04f, -234.8f);
	World_Waypoint_Set(282, kSetNR01, -346.69f, 31.55f, -1476.41f);
	World_Waypoint_Set(283, kSetNR01, -298.69f, 31.55f, -1476.41f);
	World_Waypoint_Set(284, kSetNR01, -298.69f, 31.55f, -1260.41f);
	World_Waypoint_Set(285, kSetNR01, -418.69f, 31.55f, -1260.41f);
	World_Waypoint_Set(286, kSetNR04, -104.24f, 0.0f, 183.16f);
	World_Waypoint_Set(287, kSetNR07, -254.0f, -73.5f, -41.0f);
	World_Waypoint_Set(288, kSetCT08_CT51_UG12, -125.14f, 0.02f, -176.76f);
	World_Waypoint_Set(289, kSetAR01_AR02, -871.15f, 0.0f, -1081.93f);
	World_Waypoint_Set(290, kSetAR01_AR02, -411.15f, 0.0f, -1117.93f);
	World_Waypoint_Set(291, kSetNR01, 225.31f, 31.665f, -572.41f);
	World_Waypoint_Set(292, kSetNR04, -127.0f, 0.0f, 178.0f);
	World_Waypoint_Set(293, kSetNR04, 5.0f, 0.0f, 342.0f);
	World_Waypoint_Set(294, kSetNR04, 173.0f, 0.0f, 226.0f);
	World_Waypoint_Set(295, kSetNR04, 13.0f, 0.0f, -50.0f);
	World_Waypoint_Set(296, kSetUG04, 168.0f, 11.87f, -987.0f);
	World_Waypoint_Set(297, kSetUG04, -178.5f, 23.73f, -2176.05f);
	World_Waypoint_Set(298, kSetUG09, -145.0f, 156.94f, -370.0f);
	World_Waypoint_Set(299, kSetUG09, -37.0f, 156.94f, -506.0f);
	World_Waypoint_Set(300, kSetUG09, 75.0f, 156.94f, -506.0f);
	World_Waypoint_Set(301, kSetUG10, 60.3f, 81.33f, -647.7f);
	World_Waypoint_Set(302, kSetUG10, -271.0f, 81.33f, -647.7f);
	World_Waypoint_Set(303, kSetUG10, -11.7f, 81.33f, -647.7f);
	World_Waypoint_Set(304, kSetUG10, 10.94f, 115.0f, 59.67f);
	World_Waypoint_Set(305, kSetUG10, 0.3f, 115.0f, 404.3f);
	World_Waypoint_Set(306, kSetUG10, -329.38f, 115.0f, -385.84f);
	World_Waypoint_Set(307, kSetUG12, 131.0f, -126.21f, -224.0f);
	World_Waypoint_Set(308, kSetUG12, 103.0f, -126.21f, 152.0f);
	World_Waypoint_Set(309, kSetUG12, 19.0f, -126.21f, 152.0f);
	World_Waypoint_Set(310, kSetUG12, 459.0f, -126.21f, 152.0f);
	World_Waypoint_Set(311, kSetUG12, -29.0f, -126.21f, 556.0f);
	World_Waypoint_Set(312, kSetUG14, -311.0f, 129.0f, -488.0f);
	World_Waypoint_Set(313, kSetBB11, 102.98f, -31.0f, -149.0f);
	World_Waypoint_Set(314, kSetBB11, 20.0f, -31.0f, -109.0f);
	World_Waypoint_Set(315, kSetBB11, -60.6f, -31.0f, -109.0f);
	World_Waypoint_Set(316, kSetBB11, 87.35f, -31.0f, 74.0f);
	World_Waypoint_Set(317, kSetBB11, 74.0f, -31.0f, 42.0f);
	World_Waypoint_Set(318, kSetBB11, 74.0f, -31.0f, 98.0f);
	World_Waypoint_Set(319, kSetBB11, 115.35f, -31.0f, 302.36f);
	World_Waypoint_Set(320, kSetBB11, 104.38f, -31.0f, 260.0f);
	World_Waypoint_Set(321, kSetBB11, 120.0f, -31.0f, 115.0f);
	World_Waypoint_Set(322, kSetNR04, 121.0f, 0.0f, -82.0f);
	World_Waypoint_Set(323, kSetHC01_HC02_HC03_HC04, 600.58f, 0.14f, 32.82f);
	// 324 - 335 are also set in Bryant (Sebastian's bear) AI script, same co-ordinates
	World_Waypoint_Set(324, kSetBB05, 267.0f, -60.3f, 203.0f);
	World_Waypoint_Set(325, kSetBB05, 84.0f, -60.3f, 337.0f);
	World_Waypoint_Set(326, kSetBB06_BB07, -36.0f, 0.0f, 185.0f);
	World_Waypoint_Set(327, kSetBB06_BB07, -166.0f, 0.0f, -103.0f);
	World_Waypoint_Set(328, kSetBB07, -556.0f, 252.59f, -1018.11f);
	World_Waypoint_Set(329, kSetBB07, -655.0f, 252.6f, -1012.0f);
	World_Waypoint_Set(330, kSetBB07, -657.0f, 253.0f, -1127.0f);
	World_Waypoint_Set(331, kSetBB12, 163.8f, 0.0f, 67.0f);
	World_Waypoint_Set(332, kSetBB06_BB07, -39.0f, 0.0f, 11.5f);
	World_Waypoint_Set(333, kSetBB12, -34.0f, 0.0f, 33.0f);
	World_Waypoint_Set(334, kSetBB05, 3.0f, -60.3f, -144.0f);
	World_Waypoint_Set(335, kSetBB12, -50.0f, 0.0f, 212.0f);
	World_Waypoint_Set(336, kSetNR07, -110.0f, -73.5f, -169.0f);
	World_Waypoint_Set(337, kSetNR07, -161.0f, -73.5f, -105.0f);
	World_Waypoint_Set(338, kSetNR07, -193.0f, -73.5f, -105.0f);
	World_Waypoint_Set(339, kSetUG07, 106.0f, -12.21f, -94.0f);
	World_Waypoint_Set(340, kSetUG07, 98.02f, -12.21f, -126.0f);
	World_Waypoint_Set(341, kSetUG07, 106.0f, -21.47f, -278.0f);
	World_Waypoint_Set(342, kSetUG07, 82.0f, -12.19f, -278.0f);
	World_Waypoint_Set(343, kSetDR01_DR02_DR04, -1847.0f, -0.04f, 82.0f);
	World_Waypoint_Set(344, kSetDR01_DR02_DR04, -1847.0f, -0.04f, -222.0f);
	World_Waypoint_Set(345, kSetDR01_DR02_DR04, -1147.0f, -0.04f, -198.0f);
	World_Waypoint_Set(346, kSetDR01_DR02_DR04, -667.0f, -0.04f, -125.0f);
	World_Waypoint_Set(347, kSetDR01_DR02_DR04, -471.0f, -0.04f, -110.0f);
	World_Waypoint_Set(348, kSetDR01_DR02_DR04, -403.0f, -0.04f, -110.0f);
	// 349 - kSetRC03 - Izo AI script
	World_Waypoint_Set(350, kSetNR01, -416.0f, -31.93f, -841.0f);
	World_Waypoint_Set(351, kSetCT09, 105.0f, 348.52f, 948.0f);
	World_Waypoint_Set(352, kSetCT11, -426.0f, 9.68f, -33.0f);
	World_Waypoint_Set(353, kSetCT11, -439.0f, 9.68f, -101.0f);
	World_Waypoint_Set(354, kSetNR04, 57.0f, 0.0f, 18.0f);
	World_Waypoint_Set(355, kSetNR04, 161.0f, 0.0f, 410.0f);
	World_Waypoint_Set(356, kSetNR02, 19.01f, -24.0f, 20.21f);
	World_Waypoint_Set(357, kSetNR02, 22.26f, 12.0f, -31.01f);
	World_Waypoint_Set(358, kSetNR04, 33.0f, 0.0f, 198.0f);
	World_Waypoint_Set(359, kSetNR04, 62.92f, 0.16f, 309.72f);
	// 360 - kSetHF05 - CrazyLegs AI script
	World_Waypoint_Set(361, kSetNR03, -185.0f, -70.19f, -1046.0f);
	World_Waypoint_Set(362, kSetNR03, -121.0f, -70.19f, -778.0f);
	World_Waypoint_Set(363, kSetNR03, -166.0f, -70.19f, -579.0f);
	World_Waypoint_Set(364, kSetNR03, -160.0f, -70.19f, -164.0f);
	World_Waypoint_Set(365, kSetNR03, 3.0f, -70.19f, -986.0f);
	World_Waypoint_Set(366, kSetNR02, -94.21f, 12.0f, -26.15f);
	World_Waypoint_Set(367, kSetNR02, -286.21f, -24.0f, 37.85f);
	World_Waypoint_Set(368, kSetUG16, 176.91f, -40.67f, 225.92f);
	World_Waypoint_Set(369, kSetNR01, -220.0f, 23.88f, -1437.0f);
	World_Waypoint_Set(370, kSetNR01, -392.0f, 31.55f, -1757.0f);
	World_Waypoint_Set(371, kSetHF03, 441.0f, 47.76f, -798.98f);
	World_Waypoint_Set(372, kSetHF03, 185.62f, 47.76f, -867.42f);
	World_Waypoint_Set(373, kSetHF03, 947.0f, 47.76f, -696.0f);
	World_Waypoint_Set(374, kSetUG18, -339.22f, 0.22f, -11.33f);
	World_Waypoint_Set(375, kSetNR02, -299.0f, -24.0f, 322.0f);
	World_Waypoint_Set(376, kSetNR02, -215.0f, -24.0f, 322.0f);
	World_Waypoint_Set(377, kSetHF03, 397.6f, 47.76f, -823.23f);
	World_Waypoint_Set(378, kSetHF03, 461.56f, 47.76f, -757.78f);
	World_Waypoint_Set(379, kSetTB07, -260.15f, 12.0f, -19.16f);
	World_Waypoint_Set(380, kSetHF02, 456.43f, 47.76f, -276.05f);
	World_Waypoint_Set(381, kSetRC03, -160.0f, -4.01f, 496.0f);
	World_Waypoint_Set(382, kSetRC03, 0.0f, 1.72f, 60.0f);
	World_Waypoint_Set(383, kSetRC03, 0.0f, 1.72f, -192.0f);
	World_Waypoint_Set(384, kSetRC03, 260.0f, 1.72f, 52.0f);
	World_Waypoint_Set(385, kSetCT11, 489.0f, 9.68f, 74.0f);
	World_Waypoint_Set(386, kSetCT11, -375.0f, 9.68f, 54.0f);
	World_Waypoint_Set(387, kSetCT11, -359.0f, 0.0f, 302.0f);
	World_Waypoint_Set(388, kSetBB01, 215.0f, 0.0f, -122.0f);
	World_Waypoint_Set(389, kSetBB01, -133.0f, 9.04f, 910.0f);
	World_Waypoint_Set(390, kSetDR01_DR02_DR04, -655.0f, 6.98f, -364.0f);
	World_Waypoint_Set(391, kSetDR01_DR02_DR04, -795.0f, 6.98f, -352.0f);
	World_Waypoint_Set(392, kSetDR01_DR02_DR04, -1103.0f, 6.98f, -384.0f);
	World_Waypoint_Set(393, kSetDR01_DR02_DR04, -1759.0f, -0.04f, 75.0f);
	World_Waypoint_Set(394, kSetMA07, 476.0f, -162.0f, 196.0f);
	World_Waypoint_Set(395, kSetMA07, 120.0f, -162.0f, 148.0f);
	World_Waypoint_Set(396, kSetMA07, 120.0f, -161.0f, -160.0f);
	World_Waypoint_Set(397, kSetMA07, 148.0f, -161.0f, -160.0f);
	World_Waypoint_Set(398, kSetNR01, 324.0f, 31.0f, -1316.0f);
	World_Waypoint_Set(399, kSetNR01, 236.0f, 31.0f, -1316.0f);
	World_Waypoint_Set(400, kSetNR01, 248.0f, 31.0f, -540.0f);
	World_Waypoint_Set(401, kSetNR01, -287.0f, 31.0f, -480.0f);
	World_Waypoint_Set(402, kSetNR01, -331.0f, 31.0f, -620.0f);
	World_Waypoint_Set(403, kSetNR01, -239.0f, 31.0f, -1436.0f);
	World_Waypoint_Set(404, kSetNR01, -411.0f, 31.0f, -1436.0f);
	World_Waypoint_Set(405, kSetUG01, 90.0f, -50.0f, -42.0f);
	World_Waypoint_Set(406, kSetUG01, -106.0f, -50.0f, -2358.0f);
	World_Waypoint_Set(407, kSetUG10, 0.0f, 81.02f, -512.0f);
	World_Waypoint_Set(408, kSetUG10, 0.0f, 1.15f, 400.0f);
	World_Waypoint_Set(409, kSetUG04, -48.0f, -1.74f, -983.0f);
	// 410 - UNUSED
	World_Waypoint_Set(411, kSetUG05, 80.0f, -16.72f, -4.0f);
	World_Waypoint_Set(412, kSetUG05, -48.0f, -11.0f, -352.0f);
	World_Waypoint_Set(413, kSetUG06, -109.0f, 0.0f, 285.0f);
	World_Waypoint_Set(414, kSetUG06, -109.0f, 0.0f, 125.0f);
	World_Waypoint_Set(415, kSetUG07, 198.0f, -12.0f, -282.0f);
	World_Waypoint_Set(416, kSetUG07, 90.0f, -12.0f, -274.0f);
	World_Waypoint_Set(417, kSetUG07, 10.0f, -12.0f, -282.0f);
	World_Waypoint_Set(418, kSetUG07, -106.0f, -12.0f, -746.0f);
	World_Waypoint_Set(419, kSetUG07, -59.0f, -12.0f, -614.0f);
	World_Waypoint_Set(420, kSetUG08, -496.0f, 0.0f, -168.0f);
	World_Waypoint_Set(421, kSetUG08, -341.0f, 0.0f, 248.0f);
	World_Waypoint_Set(422, kSetUG08, -348.0f, 0.0f, -36.0f);
	World_Waypoint_Set(423, kSetUG13, 60.0f, 52.0f, -544.0f);
	World_Waypoint_Set(424, kSetUG13, -552.0f, 141.0f, -1008.0f);
	World_Waypoint_Set(425, kSetUG14, 245.0f, 186.0f, -24.0f);
	World_Waypoint_Set(426, kSetUG14, -287.0f, 12.0f, -148.0f);
	World_Waypoint_Set(427, kSetUG18, -9.0f, 0.0f, 588.0f);
	World_Waypoint_Set(428, kSetUG18, -669.0f, 0.0f, 37.0f);
	World_Waypoint_Set(429, kSetNR05_NR08, -796.08f, 0.0f, -184.09f);
	World_Waypoint_Set(430, kSetMA07, -328.0f, -1.62f, 148.0f);
	World_Waypoint_Set(431, kSetUG06, 75.0f, 0.0f, -71.0f);
	World_Waypoint_Set(432, kSetUG06, 63.0f, 153.0f, -467.0f);
	World_Waypoint_Set(433, kSetUG09, 115.0f, 156.0f, -310.0f);
	World_Waypoint_Set(434, kSetUG09, -57.0f, 156.0f, -306.0f);
	World_Waypoint_Set(435, kSetUG09, -121.0f, 156.0f, -426.0f);
	World_Waypoint_Set(436, kSetUG18, -274.74f, 0.0f, 464.75f); // A bug? redundant? 436 is also set as a kSetUG18 waypoint in Sadik's AI script. Different co-ordinates
	World_Waypoint_Set(437, kSetHF05, 271.97f, 40.63f, 18.4f);
	World_Waypoint_Set(438, kSetHF05, 203.97f, 40.63f, 18.4f);
	World_Waypoint_Set(439, kSetNR05_NR08, -1273.27f, 0.32f, 126.92f);
	World_Waypoint_Set(440, kSetCT01_CT12, -453.0f, -6.5f, 1176.0f);
	World_Waypoint_Set(441, kSetCT01_CT12, -497.0f, -6.5f, 1080.0f);
	World_Waypoint_Set(442, kSetCT01_CT12, -623.0f, -6.5f, 787.0f);
	World_Waypoint_Set(443, kSetCT01_CT12, -436.0f, -6.5f, 765.0f);
	// 444 - kSetUG15 - Free Slot A (Rat) AI script
	World_Waypoint_Set(445, kSetNR04, 129.0f, 0.0f, 418.0f);
	World_Waypoint_Set(446, kSetUG04, 176.0f, 19.31f, -283.0f);
	World_Waypoint_Set(447, kSetUG04, 40.0f, -1.74f, -247.0f);
	World_Waypoint_Set(448, kSetUG04, 24.0f, -6.71f, -179.0f);
	World_Waypoint_Set(449, kSetUG04, 44.0f, -1.74f, 57.0f);
	World_Waypoint_Set(450, kSetUG01, -74.61f, -50.13f, -802.42f);
	World_Waypoint_Set(451, kSetUG01, 141.39f, -50.13f, -802.92f);
	World_Waypoint_Set(452, kSetHF06, -91.5f, 367.93f, 277.84f);
	World_Waypoint_Set(453, kSetHF06, 32.5f, 367.93f, 277.84f);
	World_Waypoint_Set(454, kSetHF06, 216.5f, 367.93f, 265.84f);
	World_Waypoint_Set(455, kSetHF06, 216.5f, 367.93f, 389.84f);
	World_Waypoint_Set(456, kSetNR11, -100.0f, 0.33f, -272.0f);
	World_Waypoint_Set(457, kSetUG05, 129.65f, 16.72f, -78.36f);
	World_Waypoint_Set(458, kSetUG05, 44.2f, -11.64f, -390.86f);
	World_Waypoint_Set(459, kSetUG05, 103.36f, -16.72f, -484.49f);
	World_Waypoint_Set(460, kSetUG06, 103.0f, 0.0f, 413.0f);
	World_Waypoint_Set(461, kSetUG06, 103.0f, 0.0f, 349.0f);
	World_Waypoint_Set(462, kSetNR11, -119.0f, 0.33f, 77.0f);
	// 463 - kSetUG01 or kSetUG04 or kSetUG07 or kSetUG09 or kSetUG12 - Free Slot A (Rat) AI script
	// 464 - kSetUG01 or kSetUG04 or kSetUG07 or kSetUG09 or kSetUG12 - Free Slot A (Rat) AI script
	// 465 - kSetUG01 or kSetUG04 or kSetUG07             or kSetUG12 - Free Slot A (Rat) AI script
	// 466 - kSetUG01 or kSetUG04 or kSetUG07 or kSetUG09 or kSetUG12 - Free Slot B (Rat) AI script
	World_Waypoint_Set(467, kSetNR05_NR08, -585.67f, 0.0f, 380.58f);
	World_Waypoint_Set(468, kSetMA07, -312.0f, -162.8f, 156.0f);
	World_Waypoint_Set(469, kSetMA07, 68.0f, -162.8f, 144.0f);
	World_Waypoint_Set(470, kSetMA07, 100.0f, -162.8f, -100.0f);
	World_Waypoint_Set(471, kSetMA07, 208.0f, -162.8f, -100.0f);
	World_Waypoint_Set(472, kSetMA07, -16.0f, -162.8f, -100.0f);
	World_Waypoint_Set(473, kSetDR01_DR02_DR04, -667.39f, -0.04f, -28.38f);
	World_Waypoint_Set(474, kSetDR01_DR02_DR04, -659.0f, 7.18f, -334.0f);
	World_Waypoint_Set(475, kSetDR01_DR02_DR04, -659.0f, -0.04f, 242.0f);
	World_Waypoint_Set(476, kSetDR01_DR02_DR04, -2327.0f, -0.04f, 142.0f);
	World_Waypoint_Set(477, kSetUG02, -97.24f, 84.13f, -69.94f);
	World_Waypoint_Set(478, kSetUG02, -97.24f, 74.87f, -509.94f);
	World_Waypoint_Set(479, kSetUG01, -134.0f, -50.13f, -250.41f);
	World_Waypoint_Set(480, kSetUG01, 17.01f, -50.13f, -2355.41f);
	World_Waypoint_Set(481, kSetUG10, -193.5f, 1.15f, 29.0f);
	World_Waypoint_Set(482, kSetUG10, -329.5f, 1.15f, 29.0f);
	World_Waypoint_Set(483, kSetUG10, -329.5f, 1.15f, -379.0f);
	// 484 - kSetNR01 or kSetNR05_NR08 or kSetHF01 or kSetHF03 - Izo AI script
	// 485 - kSetNR01 or kSetNR05_NR08 or kSetHF01 or kSetHF03 - Izo AI script
	// 486 - kSetNR02                              or kSetHF02 - Izo AI script
	// 487 - kSetNR02                              or kSetHF02 - Izo AI script
	World_Waypoint_Set(488, kSetUG01, 22.0f, -50.13f, -650.0f);
	World_Waypoint_Set(489, kSetUG01, -14.0f, -50.13f, -2354.0f);
	World_Waypoint_Set(490, kSetNR01, -360.0f, 31.55f, -1457.0f);
	World_Waypoint_Set(491, kSetNR01, 308.0f, 31.66f, -1457.0f);
	World_Waypoint_Set(492, kSetNR01, -72.0f, 23.88f, -1445.0f);
	World_Waypoint_Set(493, kSetNR01, 76.0f, 23.88f, -1333.0f);
	World_Waypoint_Set(494, kSetNR01, -236.0f, 31.55f, -337.0f);
	World_Waypoint_Set(495, kSetNR02, -275.0f, -24.0f, 42.0f);
	World_Waypoint_Set(496, kSetNR02, 185.0f, -24.0f, 42.0f);
	World_Waypoint_Set(497, kSetNR03, -250.0f, -70.19f, -639.0f);
	World_Waypoint_Set(498, kSetNR03, 454.0f, -70.19f, -667.0f);
	World_Waypoint_Set(499, kSetNR05_NR08, -573.43f, 0.0f, -635.5f);
	World_Waypoint_Set(500, kSetNR05_NR08, -625.43f, 0.0f, -635.5f);
	World_Waypoint_Set(501, kSetNR06, -215.08f, -71.88f, 150.86f);
	World_Waypoint_Set(502, kSetNR06, 60.92f, -71.88f, -29.14f);
	World_Waypoint_Set(503, kSetNR05_NR08, -1417.36f, 0.32f, 149.18f);
	World_Waypoint_Set(504, kSetNR05_NR08, -1512.0f, 0.32f, 323.0f);
	World_Waypoint_Set(505, kSetNR05_NR08, -1813.36f, 0.32f, 325.18f);
	World_Waypoint_Set(506, kSetNR10, -24.78f, 2.84f, -182.43f);
	World_Waypoint_Set(507, kSetNR10, -200.78f, 2.84f, -282.43f);
	World_Waypoint_Set(508, kSetHF01, 579.54f, -0.01f, -380.98f);
	World_Waypoint_Set(509, kSetHF01, 307.54f, 8.0f, -752.98f);
	World_Waypoint_Set(510, kSetHF01, 124.0f, 8.0f, -888.0f);
	World_Waypoint_Set(511, kSetHF01, 124.0f, 8.0f, -244.0f);
	World_Waypoint_Set(512, kSetHF02, -25.54f, 47.76f, -321.98f);
	World_Waypoint_Set(513, kSetHF02, 446.46f, 47.76f, -509.98f);
	World_Waypoint_Set(514, kSetHF03, 567.0f, 47.76f, -884.0f);
	World_Waypoint_Set(515, kSetHF03, 203.0f, 47.76f, -880.0f);
	World_Waypoint_Set(516, kSetHF05, -79.01f, 40.63f, 91.01f);
	World_Waypoint_Set(517, kSetHF04, 1246.62f, -0.31f, -171.02f);
	World_Waypoint_Set(518, kSetHF04, -72.89f, -0.31f, -154.77f);
	World_Waypoint_Set(519, kSetHF04, 285.88f, -0.31f, -134.49f);
	World_Waypoint_Set(520, kSetHF04, 231.31f, -0.31f, 266.36f);
	World_Waypoint_Set(521, kSetHF04, 482.02f, -0.31f, -661.24f);
	World_Waypoint_Set(522, kSetHF04, 1183.98f, -0.31f, -176.25f);
	World_Waypoint_Set(523, kSetHF04, -45.0f, -0.34f, -351.0f);
	// 524 - UNUSED
	// 525 - kSetKP02 - Free Slot A (Rat) AI script
	// 526 - kSetKP02 - Free Slot A (Rat) AI script
	// 527 - kSetKP02 - Free Slot B (Rat) AI script
	// 528 - kSetKP02 - Free Slot B (Rat) AI script
	// 529 - kSetKP02 - Free Slot B (Rat) AI script
	World_Waypoint_Set(530, kSetKP01, 36.79f, -12.2f, -534.54f);
	World_Waypoint_Set(531, kSetKP01, -279.21f, -12.2f, -594.54f);
	World_Waypoint_Set(532, kSetUG14, -76.51f, 129.0f, -748.49f);
	World_Waypoint_Set(533, kSetUG14, -48.51f, 129.0f, -676.49f);
	World_Waypoint_Set(534, kSetUG14, -176.51f, 129.0f, -504.49f);
	World_Waypoint_Set(535, kSetUG14, 111.49f, 129.0f, -504.49f);
	World_Waypoint_Set(536, kSetUG14, -296.51f, 12.97f, -300.49f);
	World_Waypoint_Set(537, kSetUG14, -220.51f, 12.97f, -184.49f);
	World_Waypoint_Set(538, kSetUG14, -40.51f, 12.97f, -148.49f);
	World_Waypoint_Set(539, kSetUG07, 190.0f, 12.0f, -282.0f);
	World_Waypoint_Set(540, kSetKP05_KP06, -934.24f, 0.0f, 807.77f);
	World_Waypoint_Set(541, kSetKP05_KP06, -1147.2f, 0.0f, 893.18f);
	World_Waypoint_Set(542, kSetKP05_KP06, -1098.4f, 8.26f, -312.12f);
	World_Waypoint_Set(543, kSetKP05_KP06, -1046.4f, 8.26f, -312.12f);
	World_Waypoint_Set(544, kSetUG01, 111.72f, -50.13f, -490.46f);
	World_Waypoint_Set(545, kSetUG01, -143.86f, 490.46f, -300.38f);
	World_Waypoint_Set(546, kSetNR04, 13.0f, 0.0f, 206.0f);
	// 547 - kSetUG01 or kSetUG04 or kSetUG07 or kSetUG09 or kSetUG12 - Free Slot B (Rat) AI script
	// 548 - kSetUG01 or kSetUG04 or kSetUG07             or kSetUG12 - Free Slot B (Rat) AI script
	World_Waypoint_Set(549, kSetNR04, -15.0f, 0.0f, 338.0f);
	World_Waypoint_Set(550, kSetKP05_KP06, -785.31f, 0.0f, -237.05f);
	World_Waypoint_Set(551, kSetKP05_KP06, -737.31f, 0.0f, -145.05f);
	// 552 - UNUSED
	// 553 - kSetRC03 - Izo AI script
	// 554 - kSetAR01_AR02 - Hasan AI script (new)
}

void InitScript::Init_SDB() {
	SDB_Set_Actor(kSuspectSadik, kActorSadik);
	SDB_Set_Sex(kSuspectSadik, 1);
	SDB_Add_MO_Clue(kSuspectSadik, kClueCrystalsCase);
	SDB_Add_MO_Clue(kSuspectSadik, kClueDetonatorWire);
	SDB_Add_MO_Clue(kSuspectSadik, kClueSadiksGun);
	SDB_Add_MO_Clue(kSuspectSadik, kCluePlasticExplosive);
	SDB_Add_Whereabouts_Clue(kSuspectSadik, kClueTyrellSecurity);
	SDB_Add_Whereabouts_Clue(kSuspectSadik, kClueKingstonKitchenBox1);
	SDB_Add_Whereabouts_Clue(kSuspectSadik, kClueDragonflyEarring);
	SDB_Add_Whereabouts_Clue(kSuspectSadik, kClueChewInterview);
	SDB_Add_Whereabouts_Clue(kSuspectSadik, kClueHomelessManInterview1);
	SDB_Add_Replicant_Clue(kSuspectSadik, kClueDetonatorWire);
	SDB_Add_Replicant_Clue(kSuspectSadik, kClueCrystalsCase);
	SDB_Add_Replicant_Clue(kSuspectSadik, kClueMorajiInterview);
	SDB_Add_Replicant_Clue(kSuspectSadik, kClueAttemptedFileAccess);
	SDB_Add_Replicant_Clue(kSuspectSadik, kClueExpertBomber);
	SDB_Add_Replicant_Clue(kSuspectSadik, kClueRachaelInterview);
	SDB_Add_Replicant_Clue(kSuspectSadik, kClueCrystalsCase);
	SDB_Add_Non_Replicant_Clue(kSuspectSadik, kClueStaggeredbyPunches);
	SDB_Add_Non_Replicant_Clue(kSuspectSadik, kClueIzosWarning);
	SDB_Add_Non_Replicant_Clue(kSuspectSadik, kClueAmateurBomber);
	SDB_Add_Other_Clue(kSuspectSadik, kClueGrigorianInterviewB1);
	SDB_Add_Other_Clue(kSuspectSadik, kClueGrigorianInterviewB2);
	SDB_Add_Other_Clue(kSuspectSadik, kClueAct2Ended);
	SDB_Add_Other_Clue(kSuspectSadik, kClueBombingSuspect);
	SDB_Add_Other_Clue(kSuspectSadik, kClueSadikAtMoonbus);
	SDB_Add_Identity_Clue(kSuspectSadik, kClueAct2Ended);
	SDB_Add_Photo_Clue(kSuspectSadik, kClueBombingSuspect, 31);
	SDB_Add_Photo_Clue(kSuspectSadik, kClueSadikAtMoonbus, 38);

	SDB_Set_Actor(kSuspectClovis, kActorClovis);
	SDB_Set_Sex(kSuspectClovis, 1);
	SDB_Add_MO_Clue(kSuspectClovis, kClueShellCasings);
	SDB_Add_MO_Clue(kSuspectClovis, kClueLabShellCasings);
	SDB_Add_Whereabouts_Clue(kSuspectClovis, kClueDispatchHitAndRun);
	SDB_Add_Whereabouts_Clue(kSuspectClovis, kClueRuncitersViewB);
	SDB_Add_Whereabouts_Clue(kSuspectClovis, kClueChewInterview);
	SDB_Add_Replicant_Clue(kSuspectClovis, kClueOfficersStatement);
	SDB_Add_Replicant_Clue(kSuspectClovis, kClueDoorForced2);
	if (_vm->_cutContent) {
		SDB_Add_Replicant_Clue(kSuspectClovis, kClueDoorForced1);
	}
	SDB_Add_Replicant_Clue(kSuspectClovis, kClueMorajiInterview);
	SDB_Add_Replicant_Clue(kSuspectClovis, kClueZubenTalksAboutLucy1);
	SDB_Add_Replicant_Clue(kSuspectClovis, kClueZubenTalksAboutLucy2);
	SDB_Add_Replicant_Clue(kSuspectClovis, kClueDektoraInterview4);
	SDB_Add_Other_Clue(kSuspectClovis, kClueAnimalMurderSuspect);
	SDB_Add_Other_Clue(kSuspectClovis, kClueGracefulFootprints);
	SDB_Add_Other_Clue(kSuspectClovis, kClueIzosWarning);
	SDB_Add_Other_Clue(kSuspectClovis, kClueAct2Ended);
	SDB_Add_Other_Clue(kSuspectClovis, kClueClovisAtMoonbus);
	SDB_Add_Other_Clue(kSuspectClovis, kClueAnimalMurderSuspect);
	SDB_Add_Other_Clue(kSuspectClovis, kClueIzosFriend);
	SDB_Add_Other_Clue(kSuspectClovis, kClueMilitaryBoots);
	SDB_Add_Identity_Clue(kSuspectClovis, kClueIzosWarning);
	SDB_Add_Identity_Clue(kSuspectClovis, kClueAct2Ended);
	SDB_Add_Identity_Clue(kSuspectClovis, kClueDektoraInterview4);
	SDB_Add_Photo_Clue(kSuspectClovis, kClueClovisAtMoonbus, 37);
	SDB_Add_Photo_Clue(kSuspectClovis, kClueAnimalMurderSuspect, 7);
	SDB_Add_Photo_Clue(kSuspectClovis, kClueIzosFriend, 25);
	SDB_Add_Photo_Clue(kSuspectClovis, kClueMilitaryBoots, 8);

	SDB_Set_Actor(kSuspectZuben, kActorZuben);
	SDB_Set_Sex(kSuspectZuben, 1);
	SDB_Add_MO_Clue(kSuspectZuben, kClueOfficersStatement);
	SDB_Add_MO_Clue(kSuspectZuben, kClueLabCorpses);
	SDB_Add_Whereabouts_Clue(kSuspectZuben, kClueDispatchHitAndRun);
	SDB_Add_Whereabouts_Clue(kSuspectZuben, kClueRuncitersViewB);
	SDB_Add_Replicant_Clue(kSuspectZuben, kClueOfficersStatement);
	SDB_Add_Replicant_Clue(kSuspectZuben, kClueDoorForced2);
	if (_vm->_cutContent) {
		SDB_Add_Replicant_Clue(kSuspectZuben, kClueDoorForced1);
	}
	SDB_Add_Replicant_Clue(kSuspectZuben, kClueHowieLeeInterview);
	SDB_Add_Replicant_Clue(kSuspectZuben, kClueZubenRunsAway);
	SDB_Add_Replicant_Clue(kSuspectZuben, kClueZubenSquadPhoto);
	SDB_Add_Replicant_Clue(kSuspectZuben, kClueZubenTalksAboutLucy1);
	SDB_Add_Replicant_Clue(kSuspectZuben, kClueZubenTalksAboutLucy2);
	SDB_Add_Non_Replicant_Clue(kSuspectZuben, kClueLimpingFootprints);
	SDB_Add_Non_Replicant_Clue(kSuspectZuben, kClueBigManLimping);
	SDB_Add_Non_Replicant_Clue(kSuspectZuben, kClueZubensMotive);
	SDB_Add_Other_Clue(kSuspectZuben, kClueCrowdInterviewA);
	SDB_Add_Other_Clue(kSuspectZuben, kClueZubenInterview);
	SDB_Add_Other_Clue(kSuspectZuben, kClueLucyInterview);
	SDB_Add_Identity_Clue(kSuspectZuben, kClueHowieLeeInterview);
	SDB_Add_Identity_Clue(kSuspectZuben, kClueZubenRunsAway);
	SDB_Add_Identity_Clue(kSuspectZuben, kClueZubenInterview);
	SDB_Add_Identity_Clue(kSuspectZuben, kClueLucyInterview);
	SDB_Add_Photo_Clue(kSuspectZuben, kClueZubenSquadPhoto, 33);

	SDB_Set_Actor(kSuspectLucy, kActorLucy);
	SDB_Set_Sex(kSuspectLucy, 0);
	SDB_Add_Whereabouts_Clue(kSuspectLucy, kClueChopstickWrapper);
	SDB_Add_Whereabouts_Clue(kSuspectLucy, kClueSushiMenu);
	SDB_Add_Whereabouts_Clue(kSuspectLucy, kClueReferenceLetter);
	SDB_Add_Whereabouts_Clue(kSuspectLucy, kClueRuncitersViewA);
	SDB_Add_Whereabouts_Clue(kSuspectLucy, kClueHysteriaToken);
	SDB_Add_Whereabouts_Clue(kSuspectLucy, kClueZubenInterview);
	SDB_Add_Replicant_Clue(kSuspectLucy, kClueRunciterInterviewA);
	SDB_Add_Replicant_Clue(kSuspectLucy, kClueRunciterInterviewB1);
	SDB_Add_Replicant_Clue(kSuspectLucy, kClueVKLucyReplicant);
	SDB_Add_Replicant_Clue(kSuspectLucy, kClueZubenTalksAboutLucy1);
	SDB_Add_Replicant_Clue(kSuspectLucy, kClueDektoraInterview4);
	SDB_Add_Replicant_Clue(kSuspectLucy, kClueRuncitersConfession1);
	SDB_Add_Non_Replicant_Clue(kSuspectLucy, kClueToyDog);
	SDB_Add_Non_Replicant_Clue(kSuspectLucy, kClueRagDoll);
	SDB_Add_Non_Replicant_Clue(kSuspectLucy, kClueCandy);
	SDB_Add_Non_Replicant_Clue(kSuspectLucy, kClueVKLucyHuman);
	SDB_Add_Non_Replicant_Clue(kSuspectLucy, kClueZubenTalksAboutLucy2);
	SDB_Add_Other_Clue(kSuspectLucy, kClueLucy);
	SDB_Add_Other_Clue(kSuspectLucy, kClueCrowdInterviewA);
	SDB_Add_Identity_Clue(kSuspectLucy, kClueRunciterInterviewA);
	SDB_Add_Identity_Clue(kSuspectLucy, kClueDektoraInterview4);
	SDB_Add_Photo_Clue(kSuspectLucy, kClueLucy, 5);

	SDB_Set_Actor(kSuspectDekora, kActorDektora);
	SDB_Add_MO_Clue(kSuspectDekora, kClueScorpions);
	SDB_Add_Replicant_Clue(kSuspectDekora, kClueVKDektoraReplicant);
	SDB_Add_Replicant_Clue(kSuspectDekora, kClueEarlyQInterview);
	SDB_Add_Replicant_Clue(kSuspectDekora, kClueDragonflyBelt);
	SDB_Add_Replicant_Clue(kSuspectDekora, kClueDektoraInterview4);
	SDB_Add_Non_Replicant_Clue(kSuspectDekora, kClueVKDektoraHuman);
	SDB_Add_Non_Replicant_Clue(kSuspectDekora, kClueDektoraInterview2);
	SDB_Add_Non_Replicant_Clue(kSuspectDekora, kClueDektoraInterview3);
	SDB_Add_Non_Replicant_Clue(kSuspectDekora, kClueDektorasCard);
	SDB_Add_Non_Replicant_Clue(kSuspectDekora, kClueDektoraInterview1);
	SDB_Add_Other_Clue(kSuspectDekora, kClueDragonflyBelt);
	SDB_Add_Other_Clue(kSuspectDekora, kClueWomanInAnimoidRow);
	SDB_Add_Other_Clue(kSuspectDekora, kClueChinaBar);
	SDB_Add_Other_Clue(kSuspectDekora, kClueCarRegistration1);
	SDB_Add_Identity_Clue(kSuspectDekora, kClueDektoraInterview2);
	SDB_Add_Identity_Clue(kSuspectDekora, kClueDektoraInterview3);
	SDB_Add_Identity_Clue(kSuspectDekora, kClueEarlyQInterview);
	SDB_Add_Photo_Clue(kSuspectDekora, kClueWomanInAnimoidRow, 21);
	SDB_Add_Photo_Clue(kSuspectDekora, kClueChinaBar, 19);

	SDB_Set_Actor(kSuspectGordo, kActorGordo);
	SDB_Set_Sex(kSuspectGordo, 1);
	SDB_Add_Whereabouts_Clue(kSuspectGordo, kClueStolenCheese);
	SDB_Add_Identity_Clue(kSuspectGordo, kClueGordoInterview1);
	SDB_Add_Identity_Clue(kSuspectGordo, kClueGordoInterview2);

	SDB_Set_Actor(kSuspectIzo, kActorIzo);
	SDB_Set_Sex(kSuspectIzo, 1);
	SDB_Add_Whereabouts_Clue(kSuspectIzo, kClueBobInterview1);
	SDB_Add_Whereabouts_Clue(kSuspectIzo, kClueBobInterview2);
	SDB_Add_Whereabouts_Clue(kSuspectIzo, kClueGrigorianInterviewB2);
	SDB_Add_Whereabouts_Clue(kSuspectIzo, kClueHomelessManInterview1);
	SDB_Add_Replicant_Clue(kSuspectIzo, kClueGogglesReplicantIssue);
	SDB_Add_Other_Clue(kSuspectIzo, kClueGrigorianInterviewB1);
	SDB_Add_Other_Clue(kSuspectIzo, kClueWeaponsCache);
	SDB_Add_Other_Clue(kSuspectIzo, kClueFolder);
	SDB_Add_Other_Clue(kSuspectIzo, kClueIzosStashRaided);
	SDB_Add_Other_Clue(kSuspectIzo, kClueIzo);
	SDB_Add_Other_Clue(kSuspectIzo, kCluePhotoOfMcCoy1);
	SDB_Add_Other_Clue(kSuspectIzo, kCluePhotoOfMcCoy2);
	SDB_Add_Other_Clue(kSuspectIzo, kClueRadiationGoggles);
	SDB_Add_Other_Clue(kSuspectIzo, kClueIzoInterview);
	SDB_Add_Identity_Clue(kSuspectIzo, kClueGrigorianInterviewB2);
	SDB_Add_Identity_Clue(kSuspectIzo, kClueBobInterview1);
	SDB_Add_Identity_Clue(kSuspectIzo, kClueBobInterview2);
	SDB_Add_Identity_Clue(kSuspectIzo, kCluePhotoOfMcCoy1);
	SDB_Add_Identity_Clue(kSuspectIzo, kCluePhotoOfMcCoy2);
	SDB_Add_Identity_Clue(kSuspectIzo, kClueRadiationGoggles);
	SDB_Add_Identity_Clue(kSuspectIzo, kClueIzoInterview);
	SDB_Add_Photo_Clue(kSuspectIzo, kClueIzo, 26);

	SDB_Set_Actor(kSuspectMcCoy, kActorMcCoy);
	SDB_Add_Replicant_Clue(kSuspectMcCoy, kClueMcCoyAtMoonbus);
	SDB_Add_Other_Clue(kSuspectMcCoy, kCluePhotoOfMcCoy1);
	SDB_Add_Other_Clue(kSuspectMcCoy, kCluePhotoOfMcCoy2);
	SDB_Add_Identity_Clue(kSuspectMcCoy, kClueMcCoyAtMoonbus);
	SDB_Add_Photo_Clue(kSuspectMcCoy, kClueMcCoyAtMoonbus, 36);
	SDB_Add_Photo_Clue(kSuspectMcCoy, kCluePhotoOfMcCoy1, 17);
	SDB_Add_Photo_Clue(kSuspectMcCoy, kCluePhotoOfMcCoy2, 18);

	SDB_Set_Actor(kSuspectGuzza, kActorGuzza);
	SDB_Add_Other_Clue(kSuspectGuzza, kClueGuzza);
	SDB_Add_Other_Clue(kSuspectGuzza, kClueFolder);
	SDB_Add_Other_Clue(kSuspectGuzza, kClueGuzzaFramedMcCoy);
	SDB_Add_Identity_Clue(kSuspectGuzza, kClueGuzza);
	SDB_Add_Identity_Clue(kSuspectGuzza, kClueGuzzaFramedMcCoy);
	SDB_Add_Identity_Clue(kSuspectGuzza, kClueFolder);
	SDB_Add_Photo_Clue(kSuspectGuzza, kClueGuzza, 27);
}

void InitScript::Init_CDB() {
	CDB_Set_Crime(kClueOfficersStatement, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueDoorForced1, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueDoorForced2, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueLimpingFootprints, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueGracefulFootprints, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueShellCasings, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueCandy, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueToyDog, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueChopstickWrapper, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueSushiMenu, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueLabCorpses, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueLabShellCasings, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueRuncitersVideo, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueReferenceLetter, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueCrowdInterviewA, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueCrowdInterviewB, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueRunciterInterviewA, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueRunciterInterviewB1, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueRunciterInterviewB2, kCrimeAnimalMurder);
	CDB_Set_Crime(kCluePaintTransfer, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueChromeDebris, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueRuncitersViewA, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueRuncitersViewB, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueCarColorAndMake, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueLicensePlate, kCrimeAnimalMurder);
	CDB_Set_Crime(kCluePartialLicenseNumber, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueLabPaintTransfer, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueAnimalMurderSuspect, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueMilitaryBoots, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueLucyInterview, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueCarRegistration1, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueCarRegistration2, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueCarRegistration3, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueZubenInterview, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueLucy, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueDragonflyAnklet, kCrimeAnimalMurder);
	CDB_Set_Crime(kClueZubenSquadPhoto, kCrimeAnimalMurder);

	CDB_Set_Crime(kCluePhoneCallGuzza, kCrimeEisendullerMurder);
	CDB_Set_Crime(kClueDragonflyEarring, kCrimeEisendullerMurder);
	CDB_Set_Crime(kClueTyrellSecurity, kCrimeEisendullerMurder);
	CDB_Set_Crime(kClueTyrellGuardInterview, kCrimeEisendullerMurder);
	CDB_Set_Crime(kClueDetonatorWire, kCrimeEisendullerMurder);
	CDB_Set_Crime(kClueVictimInformation, kCrimeEisendullerMurder);
	CDB_Set_Crime(kClueAttemptedFileAccess, kCrimeEisendullerMurder);
	CDB_Set_Crime(kClueKingstonKitchenBox1, kCrimeEisendullerMurder);
	CDB_Set_Crime(kClueTyrellSalesPamphletEntertainModel, kCrimeEisendullerMurder);
	CDB_Set_Crime(kClueTyrellSalesPamphletLolita, kCrimeEisendullerMurder);
	CDB_Set_Crime(kClueDogCollar1, kCrimeEisendullerMurder);
	CDB_Set_Crime(kClueRachaelInterview, kCrimeEisendullerMurder);
	CDB_Set_Crime(kClueTyrellInterview, kCrimeEisendullerMurder);
	CDB_Set_Crime(kClueBombingSuspect, kCrimeEisendullerMurder);
	CDB_Set_Crime(kClueDogCollar2, kCrimeEisendullerMurder);
	CDB_Set_Crime(kClueKingstonKitchenBox2, kCrimeEisendullerMurder);
	CDB_Set_Crime(kCluePlasticExplosive, kCrimeEisendullerMurder);
	CDB_Set_Crime(kClueTyrellSecurityPhoto, kCrimeEisendullerMurder);

	CDB_Set_Crime(kClueGaffsInformation, kCrimeMoonbusHijacking);
	CDB_Set_Crime(kClueMoonbus1, kCrimeMoonbusHijacking);
	CDB_Set_Crime(kClueMcCoyAtMoonbus, kCrimeMoonbusHijacking);
	CDB_Set_Crime(kClueClovisAtMoonbus, kCrimeMoonbusHijacking);
	CDB_Set_Crime(kClueSadikAtMoonbus, kCrimeMoonbusHijacking);
	CDB_Set_Crime(kClueVKLucyReplicant, kCrimeMoonbusHijacking);
	CDB_Set_Crime(kClueCrystalsCase, kCrimeMoonbusHijacking);
	CDB_Set_Crime(kCluePhoneCallCrystal, kCrimeMoonbusHijacking);

	CDB_Set_Crime(kClueCrimeSceneNotes, kCrimeFactoryBombing);
	CDB_Set_Crime(kClueGrigorianInterviewA, kCrimeFactoryBombing);
	CDB_Set_Crime(kClueGrigorianInterviewB1, kCrimeFactoryBombing);
	CDB_Set_Crime(kClueGrigorianInterviewB2, kCrimeFactoryBombing);

	CDB_Set_Crime(kClueMorajiInterview, kCrimeMorajiMurder);
	CDB_Set_Crime(kClueExpertBomber, kCrimeMorajiMurder);
	CDB_Set_Crime(kClueAmateurBomber, kCrimeMorajiMurder);

	CDB_Set_Crime(kClueWeaponsCache, kCrimeArmsDealing);
	CDB_Set_Crime(kClueFolder, kCrimeArmsDealing);
	CDB_Set_Crime(kClueIzosStashRaided, kCrimeArmsDealing);
	CDB_Set_Crime(kClueHomelessManInterview1, kCrimeArmsDealing);
	CDB_Set_Crime(kClueHomelessManInterview2, kCrimeArmsDealing);
	CDB_Set_Crime(kClueHomelessManKid, kCrimeArmsDealing);
	CDB_Set_Crime(kClueOriginalRequisitionForm, kCrimeArmsDealing);
	CDB_Set_Crime(kCluePoliceIssueWeapons, kCrimeArmsDealing);
	CDB_Set_Crime(kClueFolder, kCrimeArmsDealing);
	CDB_Set_Crime(kClueGuzzaFramedMcCoy, kCrimeArmsDealing);

	CDB_Set_Crime(kClueStaggeredbyPunches, kCrimeBradburyAssault);
	CDB_Set_Crime(kClueAct2Ended, kCrimeBradburyAssault);

	for (int i = 0; i != 288; ++i) {
		CDB_Set_Clue_Asset_Type(i, kClueTypeIntangible);
	}

	CDB_Set_Clue_Asset_Type(kClueOfficersStatement, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueDoorForced2, kClueTypeAudioRecording);
	if (_vm->_cutContent) {
		CDB_Set_Clue_Asset_Type(kClueDoorForced1, kClueTypeAudioRecording);
	}
	CDB_Set_Clue_Asset_Type(kClueLimpingFootprints, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueGracefulFootprints, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueShellCasings, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueCandy, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueToyDog, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueChopstickWrapper, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueSushiMenu, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueLabCorpses, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueLabShellCasings, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueRuncitersVideo, kClueTypeVideoClip);
	CDB_Set_Clue_Asset_Type(kClueLucy, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueDragonflyAnklet, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueReferenceLetter, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueCrowdInterviewA, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueCrowdInterviewB, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueZubenInterview, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueZubenSquadPhoto, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueBigManLimping, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueRunciterInterviewA, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueRunciterInterviewB1, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueRunciterInterviewB2, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueHowieLeeInterview, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kCluePaintTransfer, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueChromeDebris, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueRuncitersViewA, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueRuncitersViewB, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueCarColorAndMake, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kCluePartialLicenseNumber, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueBriefcase, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueGaffsInformation, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueCrystalVisitedRunciters, kClueTypeIntangible);
	CDB_Set_Clue_Asset_Type(kClueCrystalVisitedChinatown, kClueTypeIntangible);
	CDB_Set_Clue_Asset_Type(kClueWantedPoster, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueLicensePlate, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueLabPaintTransfer, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueDispatchHitAndRun, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueInceptShotRoy, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kCluePhoneCallGuzza, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueDragonflyEarring, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueTyrellSecurity, kClueTypeVideoClip);
	CDB_Set_Clue_Asset_Type(kClueTyrellGuardInterview, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueBombingSuspect, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueSadiksGun, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueDetonatorWire, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueVictimInformation, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueAttemptedFileAccess, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueCrystalsCase, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueKingstonKitchenBox1, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueTyrellSalesPamphletEntertainModel, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueTyrellSalesPamphletLolita, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kCluePeruvianLadyInterview, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueHasanInterview, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueBobInterview1, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueBobInterview2, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueIzoInterview, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueIzosWarning, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueRadiationGoggles, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueGogglesReplicantIssue, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueFishLadyInterview, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueDogCollar1, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueWeaponsCache, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueChewInterview, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueMorajiInterview, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueGordoInterview1, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueGordoInterview2, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueAnsweringMachineMessage, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueChessTable, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueStaggeredbyPunches, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueMaggieBracelet, kClueTypeObject);
	if (_vm->_cutContent) {
		CDB_Set_Clue_Asset_Type(kClueGarterSnake, kClueTypeObject);
		CDB_Set_Clue_Asset_Type(kClueGoldfish, kClueTypeObject);
		CDB_Set_Clue_Asset_Type(kClueSlug, kClueTypeObject);
	}
	CDB_Set_Clue_Asset_Type(kClueEnvelope, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueIzosFriend, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueChinaBarSecurityPhoto, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kCluePurchasedScorpions, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueWeaponsOrderForm, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueShippingForm, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueHysteriaToken, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueRagDoll, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueMoonbus1, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueCheese, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueDektorasDressingRoom, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueEarlyQsClub, kClueTypeVideoClip);
	CDB_Set_Clue_Asset_Type(kClueStrangeScale1, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueDektoraInterview1, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueDektoraInterview2, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueDektoraInterview3, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueDektorasCard, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueGrigoriansNote, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueCollectionReceipt, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueSpecialIngredient, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueStolenCheese, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueGordoInterview3, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueGordoConfession, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueGordosLighterReplicant, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueGordosLighterHuman, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueDektoraInterview4, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueHollowayInterview, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueBakersBadge, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueHoldensBadge, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueCarIdentified, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueCarRegistration1, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueCarRegistration2, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueCarRegistration3, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueCrazylegsInterview1, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueCrazylegsInterview2, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueLichenDogWrapper, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueRequisitionForm, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueScaryChair, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueIzosStashRaided, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueHomelessManInterview1, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueHomelessManInterview2, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueHomelessManKid, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueGuzzaFramedMcCoy, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueOriginalShippingForm, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueOriginalRequisitionForm, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueCandyWrapper, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueFlaskOfAbsinthe, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueDektoraConfession, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueRunciterConfession1, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueRunciterConfession2, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueLutherLanceInterview, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueMoonbus2, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueMoonbusCloseup, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kCluePhoneCallDektora1, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kCluePhoneCallDektora2, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kCluePhoneCallLucy1, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kCluePhoneCallLucy2, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kCluePhoneCallClovis, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kCluePhoneCallCrystal, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kCluePowerSource, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueBomb, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueCrimeSceneNotes, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueGrigorianInterviewA, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueGrigorianInterviewB1, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueGrigorianInterviewB2, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueDNATyrell, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueDNASebastian, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueDNAChew, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueDNAMoraji, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueDNALutherLance, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueDNAMarcus, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueAnimalMurderSuspect, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueMilitaryBoots, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueOuterDressingRoom, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kCluePhotoOfMcCoy1, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kCluePhotoOfMcCoy2, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueEarlyQAndLucy, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueClovisflowers, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueLucyWithDektora, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueWomanInAnimoidRow, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueScorpions, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueStrangeScale2, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueChinaBarSecurityCamera, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueIzo, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueGuzza, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueChinaBarSecurityDisc, kClueTypeVideoClip);
	CDB_Set_Clue_Asset_Type(kClueScorpionbox, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueTyrellSecurityPhoto, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueChinaBar, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kCluePlasticExplosive, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueDogCollar2, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueKingstonKitchenBox2, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueCrystalsCigarette, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueSpinnerKeys, kClueTypeObject);
	CDB_Set_Clue_Asset_Type(kClueExpertBomber, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueAmateurBomber, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueVKLucyReplicant, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueVKLucyHuman, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueVKDektoraReplicant, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueVKDektoraHuman, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueVKBobGorskyReplicant, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueVKBobGorskyHuman, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueVKGrigorianReplicant, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueVKGrigorianHuman, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueVKRunciterReplicant, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueVKRunciterHuman, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueLucyInterview, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueMoonbusReflection, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueMcCoyAtMoonbus, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueClovisAtMoonbus, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueSadikAtMoonbus, kClueTypePhotograph);
	CDB_Set_Clue_Asset_Type(kClueZubenTalksAboutLucy1, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueZubenTalksAboutLucy2, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueZubensMotive, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueRachaelInterview, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueTyrellInterview, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueRuncitersConfession1, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueEarlyInterviewA, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueEarlyInterviewB1, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueEarlyInterviewB2, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueCrazylegsInterview3, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueCrazylegGgrovels, kClueTypeAudioRecording);
	CDB_Set_Clue_Asset_Type(kClueFolder, kClueTypeObject);
}

void InitScript::Init_Spinner() {
	Spinner_Set_Selectable_Destination_Flag(kSpinnerDestinationPoliceStation,    true);
	Spinner_Set_Selectable_Destination_Flag(kSpinnerDestinationMcCoysApartment,  true);
	Spinner_Set_Selectable_Destination_Flag(kSpinnerDestinationRuncitersAnimals, true);
	Spinner_Set_Selectable_Destination_Flag(kSpinnerDestinationChinatown,        false);
	Spinner_Set_Selectable_Destination_Flag(kSpinnerDestinationAnimoidRow,       false);
	Spinner_Set_Selectable_Destination_Flag(kSpinnerDestinationTyrellBuilding,   false);
	Spinner_Set_Selectable_Destination_Flag(kSpinnerDestinationDNARow,           false);
	Spinner_Set_Selectable_Destination_Flag(kSpinnerDestinationBradburyBuilding, false);
	Spinner_Set_Selectable_Destination_Flag(kSpinnerDestinationNightclubRow,     false);
	Spinner_Set_Selectable_Destination_Flag(kSpinnerDestinationHysteriaHall,     false);
}

void InitScript::Init_Actor_Friendliness() {
	Actor_Set_Friendliness_To_Other(kActorSteele, kActorMcCoy, 65);
	Actor_Set_Friendliness_To_Other(kActorSteele, kActorGuzza, 60);
	Actor_Set_Friendliness_To_Other(kActorSteele, kActorGrigorian, 30);
	Actor_Set_Friendliness_To_Other(kActorSteele, kActorRunciter, 35);
	Actor_Set_Friendliness_To_Other(kActorSteele, kActorOfficerLeary, 65);
	Actor_Set_Friendliness_To_Other(kActorSteele, kActorOfficerGrayford, 65);
	Actor_Set_Friendliness_To_Other(kActorSteele, kActorHowieLee, 70);
	Actor_Set_Friendliness_To_Other(kActorSteele, kActorKlein, 65);
	Actor_Set_Friendliness_To_Other(kActorSteele, kActorSergeantWalls, 80);
	Actor_Set_Friendliness_To_Other(kActorSteele, kActorGaff, 65);
	Actor_Set_Friendliness_To_Other(kActorGordo, kActorSteele, 45);
	Actor_Set_Friendliness_To_Other(kActorGordo, kActorGuzza, 65);
	Actor_Set_Friendliness_To_Other(kActorGordo, kActorGrigorian, 70);
	Actor_Set_Friendliness_To_Other(kActorGordo, kActorTransient, 75);
	Actor_Set_Friendliness_To_Other(kActorGordo, kActorRunciter, 30);
	Actor_Set_Friendliness_To_Other(kActorGordo, kActorZuben, 80);
	Actor_Set_Friendliness_To_Other(kActorGordo, kActorOfficerLeary, 40);
	Actor_Set_Friendliness_To_Other(kActorGordo, kActorOfficerGrayford, 40);
	Actor_Set_Friendliness_To_Other(kActorGordo, kActorHowieLee, 70);
	Actor_Set_Friendliness_To_Other(kActorGordo, kActorGaff, 40);
	Actor_Set_Friendliness_To_Other(kActorGuzza, kActorSteele, 75);
	Actor_Set_Friendliness_To_Other(kActorGuzza, kActorGordo, 70);
	Actor_Set_Friendliness_To_Other(kActorGuzza, kActorGrigorian, 40);
	Actor_Set_Friendliness_To_Other(kActorGuzza, kActorTransient, 55);
	Actor_Set_Friendliness_To_Other(kActorGuzza, kActorRunciter, 40);
	Actor_Set_Friendliness_To_Other(kActorGuzza, kActorZuben, 45);
	Actor_Set_Friendliness_To_Other(kActorGuzza, kActorOfficerLeary, 55);
	Actor_Set_Friendliness_To_Other(kActorGuzza, kActorOfficerGrayford, 55);
	Actor_Set_Friendliness_To_Other(kActorGuzza, kActorHowieLee, 60);
	Actor_Set_Friendliness_To_Other(kActorGuzza, kActorKlein, 60);
	Actor_Set_Friendliness_To_Other(kActorGuzza, kActorSergeantWalls, 60);
	Actor_Set_Friendliness_To_Other(kActorGuzza, kActorGaff, 65);
	Actor_Set_Friendliness_To_Other(kActorClovis, kActorMcCoy, 63);
	Actor_Set_Friendliness_To_Other(kActorLucy, kActorMcCoy, 50);
	Actor_Set_Friendliness_To_Other(kActorGrigorian, kActorSteele, 30);
	Actor_Set_Friendliness_To_Other(kActorGrigorian, kActorGordo, 70);
	Actor_Set_Friendliness_To_Other(kActorGrigorian, kActorGuzza, 30);
	Actor_Set_Friendliness_To_Other(kActorGrigorian, kActorTransient, 55);
	Actor_Set_Friendliness_To_Other(kActorGrigorian, kActorRunciter, 60);
	Actor_Set_Friendliness_To_Other(kActorGrigorian, kActorZuben, 70);
	Actor_Set_Friendliness_To_Other(kActorGrigorian, kActorOfficerLeary, 30);
	Actor_Set_Friendliness_To_Other(kActorGrigorian, kActorOfficerGrayford, 30);
	Actor_Set_Friendliness_To_Other(kActorGrigorian, kActorHowieLee, 65);
	Actor_Set_Friendliness_To_Other(kActorGrigorian, kActorGaff, 30);
	Actor_Set_Friendliness_To_Other(kActorTransient, kActorGordo, 75);
	Actor_Set_Friendliness_To_Other(kActorTransient, kActorGuzza, 40);
	Actor_Set_Friendliness_To_Other(kActorTransient, kActorGrigorian, 35);
	Actor_Set_Friendliness_To_Other(kActorTransient, kActorRunciter, 40);
	Actor_Set_Friendliness_To_Other(kActorTransient, kActorZuben, 65);
	Actor_Set_Friendliness_To_Other(kActorTransient, kActorOfficerLeary, 40);
	Actor_Set_Friendliness_To_Other(kActorTransient, kActorOfficerGrayford, 40);
	Actor_Set_Friendliness_To_Other(kActorTransient, kActorHowieLee, 70);
	Actor_Set_Friendliness_To_Other(kActorTransient, kActorGaff, 45);
	Actor_Set_Friendliness_To_Other(kActorRunciter, kActorSteele, 55);
	Actor_Set_Friendliness_To_Other(kActorRunciter, kActorGordo, 40);
	Actor_Set_Friendliness_To_Other(kActorRunciter, kActorGuzza, 40);
	Actor_Set_Friendliness_To_Other(kActorRunciter, kActorGrigorian, 35);
	Actor_Set_Friendliness_To_Other(kActorRunciter, kActorTransient, 40);
	Actor_Set_Friendliness_To_Other(kActorRunciter, kActorZuben, 40);
	Actor_Set_Friendliness_To_Other(kActorRunciter, kActorOfficerLeary, 60);
	Actor_Set_Friendliness_To_Other(kActorRunciter, kActorOfficerGrayford, 60);
	Actor_Set_Friendliness_To_Other(kActorRunciter, kActorHowieLee, 65);
	Actor_Set_Friendliness_To_Other(kActorRunciter, kActorGaff, 65);
	Actor_Set_Friendliness_To_Other(kActorZuben, kActorSteele, 45);
	Actor_Set_Friendliness_To_Other(kActorZuben, kActorGordo, 90);
	Actor_Set_Friendliness_To_Other(kActorZuben, kActorGuzza, 55);
	Actor_Set_Friendliness_To_Other(kActorZuben, kActorGrigorian, 65);
	Actor_Set_Friendliness_To_Other(kActorZuben, kActorTransient, 60);
	Actor_Set_Friendliness_To_Other(kActorZuben, kActorRunciter, 35);
	Actor_Set_Friendliness_To_Other(kActorZuben, kActorOfficerLeary, 35);
	Actor_Set_Friendliness_To_Other(kActorZuben, kActorOfficerGrayford, 35);
	Actor_Set_Friendliness_To_Other(kActorZuben, kActorHowieLee, 60);
	Actor_Set_Friendliness_To_Other(kActorZuben, kActorGaff, 35);
	Actor_Set_Friendliness_To_Other(kActorOfficerLeary, kActorSteele, 75);
	Actor_Set_Friendliness_To_Other(kActorOfficerLeary, kActorGuzza, 75);
	Actor_Set_Friendliness_To_Other(kActorOfficerLeary, kActorGrigorian, 35);
	Actor_Set_Friendliness_To_Other(kActorOfficerLeary, kActorTransient, 45);
	Actor_Set_Friendliness_To_Other(kActorOfficerLeary, kActorRunciter, 40);
	Actor_Set_Friendliness_To_Other(kActorOfficerLeary, kActorOfficerGrayford, 80);
	Actor_Set_Friendliness_To_Other(kActorOfficerLeary, kActorHowieLee, 70);
	Actor_Set_Friendliness_To_Other(kActorOfficerLeary, kActorKlein, 85);
	Actor_Set_Friendliness_To_Other(kActorOfficerLeary, kActorSergeantWalls, 85);
	Actor_Set_Friendliness_To_Other(kActorOfficerLeary, kActorGaff, 85);
	Actor_Set_Friendliness_To_Other(kActorOfficerGrayford, kActorSteele, 75);
	Actor_Set_Friendliness_To_Other(kActorOfficerGrayford, kActorGuzza, 75);
	Actor_Set_Friendliness_To_Other(kActorOfficerGrayford, kActorGrigorian, 35);
	Actor_Set_Friendliness_To_Other(kActorOfficerGrayford, kActorTransient, 45);
	Actor_Set_Friendliness_To_Other(kActorOfficerGrayford, kActorRunciter, 40);
	Actor_Set_Friendliness_To_Other(kActorOfficerGrayford, kActorOfficerLeary, 80);
	Actor_Set_Friendliness_To_Other(kActorOfficerGrayford, kActorHowieLee, 65);
	Actor_Set_Friendliness_To_Other(kActorOfficerGrayford, kActorKlein, 80);
	Actor_Set_Friendliness_To_Other(kActorOfficerGrayford, kActorSergeantWalls, 85);
	Actor_Set_Friendliness_To_Other(kActorOfficerGrayford, kActorGaff, 85);
	Actor_Set_Friendliness_To_Other(kActorHowieLee, kActorMcCoy, 60);
	Actor_Set_Friendliness_To_Other(kActorHowieLee, kActorSteele, 60);
	Actor_Set_Friendliness_To_Other(kActorHowieLee, kActorGordo, 65);
	Actor_Set_Friendliness_To_Other(kActorHowieLee, kActorGuzza, 65);
	Actor_Set_Friendliness_To_Other(kActorHowieLee, kActorTransient, 65);
	Actor_Set_Friendliness_To_Other(kActorHowieLee, kActorRunciter, 40);
	Actor_Set_Friendliness_To_Other(kActorHowieLee, kActorZuben, 65);
	Actor_Set_Friendliness_To_Other(kActorHowieLee, kActorOfficerLeary, 70);
	Actor_Set_Friendliness_To_Other(kActorHowieLee, kActorOfficerGrayford, 70);
	Actor_Set_Friendliness_To_Other(kActorHowieLee, kActorGaff, 70);
	Actor_Set_Friendliness_To_Other(kActorKlein, kActorSteele, 70);
	Actor_Set_Friendliness_To_Other(kActorKlein, kActorGuzza, 75);
	Actor_Set_Friendliness_To_Other(kActorKlein, kActorGrigorian, 35);
	Actor_Set_Friendliness_To_Other(kActorKlein, kActorOfficerLeary, 70);
	Actor_Set_Friendliness_To_Other(kActorKlein, kActorOfficerGrayford, 70);
	Actor_Set_Friendliness_To_Other(kActorKlein, kActorSergeantWalls, 65);
	Actor_Set_Friendliness_To_Other(kActorKlein, kActorGaff, 70);
	Actor_Set_Friendliness_To_Other(kActorSergeantWalls, kActorMcCoy, 70);
	Actor_Set_Friendliness_To_Other(kActorSergeantWalls, kActorSteele, 70);
	Actor_Set_Friendliness_To_Other(kActorSergeantWalls, kActorGuzza, 80);
	Actor_Set_Friendliness_To_Other(kActorSergeantWalls, kActorGrigorian, 35);
	Actor_Set_Friendliness_To_Other(kActorSergeantWalls, kActorOfficerLeary, 70);
	Actor_Set_Friendliness_To_Other(kActorSergeantWalls, kActorOfficerGrayford, 70);
	Actor_Set_Friendliness_To_Other(kActorSergeantWalls, kActorKlein, 65);
	Actor_Set_Friendliness_To_Other(kActorSergeantWalls, kActorGaff, 70);
	Actor_Set_Friendliness_To_Other(kActorGaff, kActorSteele, 70);
	Actor_Set_Friendliness_To_Other(kActorGaff, kActorGuzza, 65);
	Actor_Set_Friendliness_To_Other(kActorGaff, kActorGrigorian, 35);
	Actor_Set_Friendliness_To_Other(kActorGaff, kActorRunciter, 35);
	Actor_Set_Friendliness_To_Other(kActorGaff, kActorOfficerLeary, 70);
	Actor_Set_Friendliness_To_Other(kActorGaff, kActorOfficerGrayford, 65);
	Actor_Set_Friendliness_To_Other(kActorGaff, kActorHowieLee, 80);
	Actor_Set_Friendliness_To_Other(kActorGaff, kActorKlein, 70);
	Actor_Set_Friendliness_To_Other(kActorGaff, kActorSergeantWalls, 70);
}

void InitScript::Init_Actor_Combat_Aggressiveness() {
	Actor_Set_Combat_Aggressiveness(kActorGuzza, 50);
	Actor_Set_Combat_Aggressiveness(kActorBulletBob, 50);
	Actor_Set_Combat_Aggressiveness(kActorTyrellGuard, 50);
	Actor_Set_Combat_Aggressiveness(kActorZuben, 90);
	Actor_Set_Combat_Aggressiveness(kActorGordo, 50);
	Actor_Set_Combat_Aggressiveness(kActorLucy, 0);
	Actor_Set_Combat_Aggressiveness(kActorIzo, 70);
	Actor_Set_Combat_Aggressiveness(kActorDektora, 60);
	Actor_Set_Combat_Aggressiveness(kActorSteele, 70);
	Actor_Set_Combat_Aggressiveness(kActorSadik, 80);
	Actor_Set_Combat_Aggressiveness(kActorClovis, 75);
	Actor_Set_Combat_Aggressiveness(kActorOfficerLeary, 70);
	Actor_Set_Combat_Aggressiveness(kActorOfficerGrayford, 60);
	Actor_Set_Combat_Aggressiveness(kActorMutant1, 40);
	Actor_Set_Combat_Aggressiveness(kActorMutant2, 30);
	Actor_Set_Combat_Aggressiveness(kActorMutant3, 30);
	Actor_Set_Combat_Aggressiveness(kActorFreeSlotA, 100);
}

void InitScript::Init_Actor_Honesty() {
	Actor_Set_Honesty(kActorZuben, 90);
}

void InitScript::Init_Actor_Intelligence() {
	Actor_Set_Intelligence(kActorZuben, 20);
	Actor_Set_Intelligence(kActorGordo, 70);
	Actor_Set_Intelligence(kActorLucy, 60);
	Actor_Set_Intelligence(kActorIzo, 75);
	Actor_Set_Intelligence(kActorDektora, 80);
	Actor_Set_Intelligence(kActorSteele, 80);
	Actor_Set_Intelligence(kActorSadik, 80);
	Actor_Set_Intelligence(kActorClovis, 100);
	Actor_Set_Intelligence(kActorOfficerLeary, 50);
	Actor_Set_Intelligence(kActorOfficerGrayford, 40);
	Actor_Set_Intelligence(kActorMutant1, 40);
	Actor_Set_Intelligence(kActorMutant2, 20);
	Actor_Set_Intelligence(kActorMutant3, 30);
	Actor_Set_Intelligence(kActorFreeSlotA, 10);
}

void InitScript::Init_Actor_Stability() {
	Actor_Set_Stability(kActorZuben, 35);
}

void InitScript::Init_Actor_Health() {
	Actor_Set_Health(kActorMcCoy, 50, 50);
	Actor_Set_Health(kActorGuzza, 50, 50);
	Actor_Set_Health(kActorCrazylegs, 50, 50);
	Actor_Set_Health(kActorLuther, 50, 50);
	Actor_Set_Health(kActorGrigorian, 50, 50);
	Actor_Set_Health(kActorTransient, 50, 50);
	Actor_Set_Health(kActorLance, 50, 50);
	Actor_Set_Health(kActorBulletBob, 50, 50);
	Actor_Set_Health(kActorRunciter, 50, 50);
	Actor_Set_Health(kActorInsectDealer, 50, 50);
	Actor_Set_Health(kActorTyrellGuard, 50, 50);
	Actor_Set_Health(kActorEarlyQ, 50, 50);
	Actor_Set_Health(kActorHasan, 50, 50);
	Actor_Set_Health(kActorMarcus, 50, 50);
	Actor_Set_Health(kActorMia, 50, 50);
	Actor_Set_Health(kActorHanoi, 50, 50);
	Actor_Set_Health(kActorBaker, 50, 50);
	Actor_Set_Health(kActorDeskClerk, 50, 50);
	Actor_Set_Health(kActorHowieLee, 50, 50);
	Actor_Set_Health(kActorFishDealer, 50, 50);
	Actor_Set_Health(kActorKlein, 50, 50);
	Actor_Set_Health(kActorMurray, 50, 50);
	Actor_Set_Health(kActorHawkersBarkeep, 50, 50);
	Actor_Set_Health(kActorHolloway, 50, 50);
	Actor_Set_Health(kActorSergeantWalls, 50, 50);
	Actor_Set_Health(kActorMoraji, 50, 50);
	Actor_Set_Health(kActorTheBard, 50, 50);
	Actor_Set_Health(kActorPhotographer, 50, 50);
	Actor_Set_Health(kActorDispatcher, 50, 50);
	Actor_Set_Health(kActorRajif, 50, 50);
	Actor_Set_Health(kActorGovernorKolvig, 50, 50);
	Actor_Set_Health(kActorEarlyQBartender, 50, 50);
	Actor_Set_Health(kActorHawkersParrot, 50, 50);
	Actor_Set_Health(kActorTaffyPatron, 50, 50);
	Actor_Set_Health(kActorLockupGuard, 50, 50);
	Actor_Set_Health(kActorTeenager, 50, 50);
	Actor_Set_Health(kActorHysteriaPatron1, 50, 50);
	Actor_Set_Health(kActorHysteriaPatron2, 50, 50);
	Actor_Set_Health(kActorHysteriaPatron3, 50, 50);
	Actor_Set_Health(kActorShoeshineMan, 50, 50);
	Actor_Set_Health(kActorTyrell, 50, 50);
	Actor_Set_Health(kActorChew, 50, 50);
	Actor_Set_Health(kActorGaff, 50, 50);
	Actor_Set_Health(kActorBryant, 50, 50);
	Actor_Set_Health(kActorTaffy, 50, 50);
	Actor_Set_Health(kActorSebastian, 50, 50);
	Actor_Set_Health(kActorRachael, 50, 50);
	Actor_Set_Health(kActorGeneralDoll, 50, 50);
	Actor_Set_Health(kActorIsabella, 50, 50);
	Actor_Set_Health(kActorLeon, 50, 50);
	Actor_Set_Health(kActorMaggie, 50, 50);
	Actor_Set_Health(kActorGenwalkerA, 50, 50);
	Actor_Set_Health(kActorGenwalkerB, 50, 50);
	Actor_Set_Health(kActorGenwalkerC, 50, 50);

	// The health of all who can be Replicants will again be set further below
	// (excluding LutherLance)
	Actor_Set_Health(kActorSteele, 60, 60);
	Actor_Set_Health(kActorGordo, 40, 40);
	Actor_Set_Health(kActorDektora, 60, 60);
	Actor_Set_Health(kActorClovis, 90, 90);
	Actor_Set_Health(kActorLucy, 20, 20);
	Actor_Set_Health(kActorIzo, 50, 50);
	Actor_Set_Health(kActorSadik, 60, 60);
	Actor_Set_Health(kActorZuben, 80, 80);
	Actor_Set_Health(kActorOfficerLeary, 40, 40);
	Actor_Set_Health(kActorOfficerGrayford, 50, 50);

	Actor_Set_Health(kActorMutant1, 30, 30);
	Actor_Set_Health(kActorMutant2, 50, 50);
	Actor_Set_Health(kActorMutant3, 20, 20);
	Actor_Set_Health(kActorFreeSlotA, 20, 20);
	Actor_Set_Health(kActorFreeSlotB, 20, 20);
	if (Game_Flag_Query(kFlagGordoIsReplicant)) {
		Actor_Set_Health(kActorGordo, 60, 60);
	}
	if (Game_Flag_Query(kFlagLucyIsReplicant)) {
		Actor_Set_Health(kActorLucy, 40, 40);
	}
	if (Game_Flag_Query(kFlagIzoIsReplicant)) {
		Actor_Set_Health(kActorIzo, 65, 65);
	}
	if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
		Actor_Set_Health(kActorDektora, 70, 70);
	}
	if (Game_Flag_Query(kFlagSadikIsReplicant)) {
		Actor_Set_Health(kActorSadik, 80, 80);
	}
}

void InitScript::Init_Combat_Cover_Waypoints() {
	Combat_Cover_Waypoint_Set_Data( 0,  0, kSetDR01_DR02_DR04, kSceneDR01, -603.0f,    0.0f,     32.0f);
	Combat_Cover_Waypoint_Set_Data( 1,  0, kSetDR01_DR02_DR04, kSceneDR01, -670.0f,    0.0f,    -24.0f);
	Combat_Cover_Waypoint_Set_Data( 2,  0, kSetDR01_DR02_DR04, kSceneDR01, -604.0f,    0.0f,    -96.0f);
	Combat_Cover_Waypoint_Set_Data( 3,  0, kSetDR01_DR02_DR04, kSceneDR01, -490.0f,    0.0f,    -20.0f);
	Combat_Cover_Waypoint_Set_Data( 4,  0, kSetDR01_DR02_DR04, kSceneDR01, -667.0f,    0.0f,   -164.0f);
	Combat_Cover_Waypoint_Set_Data( 5,  0, kSetDR01_DR02_DR04, kSceneDR01, -606.0f,    0.0f,   -222.0f);
	Combat_Cover_Waypoint_Set_Data( 6,  0, kSetDR01_DR02_DR04, kSceneDR01, -534.0f,    0.0f,   -170.0f);
	Combat_Cover_Waypoint_Set_Data( 7,  3, kSetNR01,           kSceneNR01,   76.06f,  23.83f, -1058.49f);
	Combat_Cover_Waypoint_Set_Data( 8,  3, kSetNR01,           kSceneNR01, -335.94f,  31.55f, -1406.49f);
	Combat_Cover_Waypoint_Set_Data( 9, 15, kSetHF06,           kSceneHF06,  286.0f,  367.93f,   330.0f);
	Combat_Cover_Waypoint_Set_Data(10, 15, kSetHF06,           kSceneHF06,  -58.0f,  367.93f,   294.0f);
	Combat_Cover_Waypoint_Set_Data(11,  4, kSetHF01,           kSceneHF01,  -30.0f,    8.0f,   -759.0f);
	Combat_Cover_Waypoint_Set_Data(12,  4, kSetHF01,           kSceneHF01,  -93.0f,    8.0f,   -693.0f);
	Combat_Cover_Waypoint_Set_Data(13,  4, kSetHF01,           kSceneHF01,   -6.0f,    8.0f,   -607.0f);
	Combat_Cover_Waypoint_Set_Data(14,  4, kSetHF01,           kSceneHF01,   78.0f,    8.0f,   -687.0f);
	Combat_Cover_Waypoint_Set_Data(15,  4, kSetHF01,           kSceneHF01,  262.0f,    8.0f,   -683.0f);
	Combat_Cover_Waypoint_Set_Data(16,  4, kSetHF01,           kSceneHF01,  334.0f,    8.0f,   -607.0f);
	Combat_Cover_Waypoint_Set_Data(17,  4, kSetHF01,           kSceneHF01,  426.0f,    8.0f,   -679.0f);
	Combat_Cover_Waypoint_Set_Data(18,  5, kSetCT01_CT12,      kSceneCT12, -318.0f,   -6.5f,   1117.0f);
	Combat_Cover_Waypoint_Set_Data(19,  5, kSetCT01_CT12,      kSceneCT12, -330.0f,   -6.5f,    981.0f);
	Combat_Cover_Waypoint_Set_Data(20,  5, kSetCT01_CT12,      kSceneCT12, -294.0f,   -6.5f,    725.0f);
	Combat_Cover_Waypoint_Set_Data(21, 16, kSetUG12,           kSceneUG12,  380.0f, -126.21f,   198.0f);
	Combat_Cover_Waypoint_Set_Data(22, 16, kSetUG12,           kSceneUG12,  364.0f, -126.21f,   -66.0f);
	Combat_Cover_Waypoint_Set_Data(23, 18, kSetRC03,           kSceneRC03,    8.0f,    1.72f,     8.0f);
	Combat_Cover_Waypoint_Set_Data(24, 18, kSetRC03,           kSceneRC03,  188.0f,    1.72f,    92.0f);
	Combat_Cover_Waypoint_Set_Data(25, 18, kSetRC03,           kSceneRC03,  216.0f,   -4.01f,   196.0f);
	Combat_Cover_Waypoint_Set_Data(26, 18, kSetRC03,           kSceneRC03, -420.0f,    1.72f,   100.0f);
	Combat_Cover_Waypoint_Set_Data(27, 18, kSetRC03,           kSceneRC03, -672.0f,    1.72f,   -44.0f);
	Combat_Cover_Waypoint_Set_Data(28, 10, kSetUG05,           kSceneUG05, -128.77f,  -5.21f,  -435.0f);
	Combat_Cover_Waypoint_Set_Data(29, 10, kSetUG05,           kSceneUG05,  -20.77f,   0.81f,  -199.0f);
	Combat_Cover_Waypoint_Set_Data(30, 10, kSetUG04,           kSceneUG04, -216.0f,   39.15f,  -819.0f);
	Combat_Cover_Waypoint_Set_Data(31, 10, kSetUG04,           kSceneUG04,  144.0f,   -1.74f, -1015.0f);
	Combat_Cover_Waypoint_Set_Data(32, 10, kSetUG04,           kSceneUG04,  152.0f,    4.14f,  -811.0f);
	Combat_Cover_Waypoint_Set_Data(33, 10, kSetUG06,           kSceneUG06, -121.0f,    0.0f,    -39.0f);
	Combat_Cover_Waypoint_Set_Data(34, 10, kSetUG07,           kSceneUG07,   86.0f,  -12.21f,  -278.0f);
	Combat_Cover_Waypoint_Set_Data(35, 20, kSetHF05,           kSceneHF05,  407.82f,  40.63f,    95.25f);
	Combat_Cover_Waypoint_Set_Data(36, 20, kSetHF05,           kSceneHF05,  357.81f,  40.63f,   387.9f);
}

void InitScript::Init_Combat_Flee_Waypoints() {
	Combat_Flee_Waypoint_Set_Data( 0,  4, kSetHF01,           kSceneHF01,   187.0f,     8.0f,   -197.0f,  -1);
	Combat_Flee_Waypoint_Set_Data( 1,  4, kSetHF01,           kSceneHF01,   454.0f,     8.0f,   -717.0f,  -1);
	Combat_Flee_Waypoint_Set_Data( 2,  4, kSetHF02,           kSceneHF02,   -10.87f,   47.76f,  -141.32f, -1);
	Combat_Flee_Waypoint_Set_Data( 3,  4, kSetHF02,           kSceneHF02,   466.0f,    47.76f,  -532.0f,  -1);
	Combat_Flee_Waypoint_Set_Data( 4,  4, kSetHF03,           kSceneHF03,   199.0f,    47.76f,  -880.0f,  -1);
	Combat_Flee_Waypoint_Set_Data( 5,  4, kSetHF03,           kSceneHF03,   594.0f,    47.76f, -1141.0f,  -1);
	Combat_Flee_Waypoint_Set_Data( 6,  4, kSetHF03,           kSceneHF03,   912.0f,    47.76f,  -447.0f,  -1);
	Combat_Flee_Waypoint_Set_Data( 7,  4, kSetHF03,           kSceneHF03,   492.0f,    47.76f,  -459.0f,  -1);
	Combat_Flee_Waypoint_Set_Data( 8,  1, kSetBB01,           kSceneBB01,   271.0f,     0.0f,   1038.0f,  -1);
	Combat_Flee_Waypoint_Set_Data( 9,  1, kSetBB01,           kSceneBB01,  -175.0f,     9.04f,     8.59f, -1);
	Combat_Flee_Waypoint_Set_Data(10,  1, kSetBB01,           kSceneBB01,  -343.05f,    9.04f,   260.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(11,  0, kSetDR01_DR02_DR04, kSceneDR01,  -443.0f,    -0.04f,  -180.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(12,  0, kSetDR01_DR02_DR04, kSceneDR02, -1485.0f,     6.98f,  -393.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(13,  0, kSetDR01_DR02_DR04, kSceneDR04,  -652.0f,     7.18f,   354.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(14,  6, kSetMA01,           kSceneMA01,    25.0f,     0.0f,   -314.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(15,  6, kSetMA01,           kSceneMA01,   980.0f,     0.0f,    189.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(16,  6, kSetMA01,           kSceneMA01,   601.0f,     0.0f,  -1641.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(17, 12, kSetUG07,           kSceneUG07,   218.0f,   -12.21f,  -290.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(18, 12, kSetUG07,           kSceneUG07,  -150.0f,   -12.21f,  -962.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(19, 13, kSetUG08,           kSceneUG08,  -524.0f,     0.0f,   -172.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(20, 13, kSetUG08,           kSceneUG08,  -427.0f,     0.0f,    199.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(21, 13, kSetUG08,           kSceneUG08,  -121.0f,     0.0f,   -145.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(22, 14, kSetUG10,           kSceneUG10,     2.0f,     1.15f,   412.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(23, 14, kSetUG10,           kSceneUG10,  -327.0f,     1.15f,  -384.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(24, 14, kSetUG10,           kSceneUG10,  -253.0f,    81.33f,  -620.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(25,  5, kSetCT05,           kSceneCT05,  -518.52f, -109.91f,   312.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(26,  5, kSetCT05,           kSceneCT05,   141.0f,  -109.91f,   452.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(27,  5, kSetCT11,           kSceneCT11,   516.0f,     0.56f,   779.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(28,  5, kSetCT11,           kSceneCT11,   388.0f,     9.68f,   190.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(29,  5, kSetCT11,           kSceneCT11,  -429.0f,     9.68f,  -115.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(30,  7, kSetPS14,           kScenePS14,  -997.0f,   508.14f,  -630.84f, -1);
	Combat_Flee_Waypoint_Set_Data(31,  7, kSetPS14,           kScenePS14,  -416.27f,  508.14f,  -574.84f, -1);
	Combat_Flee_Waypoint_Set_Data(32,  7, kSetPS14,           kScenePS14,  -416.0f,   508.14f,  -906.84f, -1);
	Combat_Flee_Waypoint_Set_Data(33,  7, kSetPS14,           kScenePS14, -1168.0f,   508.14f, -1666.84f, -1);
	Combat_Flee_Waypoint_Set_Data(34, 10, kSetUG06,           kSceneUG06,    75.0f,   153.0f,   -485.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(35, 10, kSetUG06,           kSceneUG06,   -18.0f,     0.0f,    321.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(36, 11, kSetUG01,           kSceneUG01,  -190.0f,   -50.13f,  -298.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(37, 11, kSetUG01,           kSceneUG01,   126.0f,   -50.13f,  -150.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(38, 11, kSetUG01,           kSceneUG01,   187.78f,  -50.13f, -1262.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(39, 11, kSetUG01,           kSceneUG01,   -20.22f,  -30.13f, -2338.3f,  -1);
	Combat_Flee_Waypoint_Set_Data(40,  3, kSetNR01,           kSceneNR01,  -416.0f,    31.55f,  -829.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(41,  3, kSetNR01,           kSceneNR01,  -412.0f,    31.55f, -1357.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(42,  3, kSetNR01,           kSceneNR01,  -208.0f,    23.0f,  -1693.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(43,  3, kSetNR01,           kSceneNR01,   -64.0f,    23.83f, -2097.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(44,  3, kSetNR01,           kSceneNR01,   320.0f,    23.83f, -1058.49f, -1);
	Combat_Flee_Waypoint_Set_Data(45,  3, kSetNR01,           kSceneNR01,   252.0f,    31.65f,  -674.49f, -1);
	Combat_Flee_Waypoint_Set_Data(46,  8, kSetKP02,           kSceneKP02,  -803.0f,  -615.49f,  2619.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(47,  8, kSetKP02,           kSceneKP02, -1027.0f,  -614.49f,  3151.24f, -1);
	Combat_Flee_Waypoint_Set_Data(48,  9, kSetKP05_KP06,      kSceneKP05, -1190.0f,     0.0f,    876.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(49,  9, kSetKP05_KP06,      kSceneKP05,  -687.0f,     0.0f,    910.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(50,  9, kSetKP05_KP06,      kSceneKP06,  -455.0f,     8.26f,  -453.26f, -1);
	Combat_Flee_Waypoint_Set_Data(51,  9, kSetKP05_KP06,      kSceneKP06, -1127.0f,     8.26f,  -705.26f, -1);
	Combat_Flee_Waypoint_Set_Data(52,  9, kSetKP05_KP06,      kSceneKP06, -1143.0f,     8.26f,  -261.26f, -1);
	Combat_Flee_Waypoint_Set_Data(53,  9, kSetKP05_KP06,      kSceneKP06,  -703.0f,     8.26f,  -157.26f, -1);
	Combat_Flee_Waypoint_Set_Data(54, 10, kSetUG04,           kSceneUG04,   166.0f,    11.87f,  -250.8f,  -1);
	Combat_Flee_Waypoint_Set_Data(55, 10, kSetUG04,           kSceneUG04,   158.0f,     4.14f,   -10.8f,  -1);
	Combat_Flee_Waypoint_Set_Data(56, 10, kSetUG04,           kSceneUG04,  -310.0f,    39.15f,  -822.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(57, 10, kSetUG04,           kSceneUG04,  -302.0f,    -1.74f, -5847.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(58, 10, kSetUG05,           kSceneUG05,     4.0f,     1.37f, -3684.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(59, 10, kSetUG05,           kSceneUG05,   146.28f,   -6.05f,  -135.93f, -1);
	Combat_Flee_Waypoint_Set_Data(60,  7, kSetMA07,           kSceneMA07,   212.0f,  -162.8f,   -108.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(61,  7, kSetMA07,           kSceneMA07,   -28.0f,  -162.8f,   -104.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(62,  7, kSetMA07,           kSceneMA07,   504.0f,  -162.8f,    224.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(63,  7, kSetMA07,           kSceneMA07,  -301.0f,  -162.8f,    275.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(64, 15, kSetHF06,           kSceneHF06,    98.0f,   367.93f,   -10.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(65, 15, kSetHF06,           kSceneHF06,  -206.0f,   367.69f,   386.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(66,  5, kSetCT01_CT12,      kSceneCT12,  -190.0f,    -6.5f,    789.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(67,  5, kSetCT01_CT12,      kSceneCT12,   123.0f,    -6.5f,   1002.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(68,  5, kSetCT01_CT12,      kSceneCT12,  -573.0f,    -6.5f,   1202.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(69, 16, kSetUG12,           kSceneUG12,   120.0f,  -126.21f,  -350.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(70, 16, kSetUG12,           kSceneUG12,   524.0f,  -126.21f,   158.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(71, 16, kSetUG12,           kSceneUG12,   276.0f,  -126.21f,   537.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(72, 17, kSetUG14,           kSceneUG14,  -334.7f,    12.97f,  -332.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(73, 18, kSetRC03,           kSceneRC03,  -672.0f,     1.72f,   -96.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(74, 18, kSetRC03,           kSceneRC03,  -552.0f,    -4.01f,   268.0f,  -1);
	Combat_Flee_Waypoint_Set_Data(75, 18, kSetRC03,           kSceneRC03,   293.06f,    1.72f,   112.25f, -1);
	Combat_Flee_Waypoint_Set_Data(76, 20, kSetHF05,           kSceneHF05,   435.13f,   37.18f,  -292.34f, -1);
}

void InitScript::Init_Shadows() {
	int list[] = {440, 37, 38, 83, 874};
	Disable_Shadows(list, 5);
}

} // End of namespace BladeRunner
