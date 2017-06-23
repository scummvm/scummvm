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

#ifndef BLADERUNNER_SCRIPT_H
#define BLADERUNNER_SCRIPT_H

#include "common/str.h"

#include "bladerunner/bladerunner.h"

namespace BladeRunner {

enum Actors {
	kActorMcCoy = 0,
	kActorSteele = 1,
	kActorGordo = 2,
	kActorDektora = 3,
	kActorGuzza = 4,
	kActorClovis = 5,
	kActorLucy = 6,
	kActorIzo = 7,
	kActorSadik = 8,
	kActorCrazylegs = 9,
	kActorLuther = 10,
	kActorGrigorian = 11,
	kActorTransient = 12,
	kActorLance = 13,
	kActorBulletBob = 14,
	kActorRunciter = 15,
	kActorInsectDealer = 16,
	kActorTyrellGuard = 17,
	kActorEarlyQ = 18,
	kActorZuben = 19,
	kActorHasan = 20,
	kActorMarcus = 21,
	kActorMia = 22,
	kActorOfficerLeary = 23,
	kActorOfficerGrayford = 24,
	kActorHanoi = 25,
	kActorBaker = 26,
	kActorDeskClerk = 27,
	kActorHowieLee = 28,
	kActorFishDealer = 29,
	kActorKlein = 30,
	kActorMurray = 31,
	kActorHawkersBarkeep = 32,
	kActorHolloway = 33,
	kActorSergeantWalls = 34,
	kActorMoraji = 35,
	kActorTheBard = 36,
	kActorPhotographer = 37,
	kActorDispatcher = 38,
	kActorAnsweringMachine = 39,
	kActorRajif = 40,
	kActorGovernorKolvig = 41,
	kActorEarlyQBartender = 42,
	kActorHawkersParrot = 43,
	kActorTaffyPatron = 44,
	kActorLockupGuard = 45,
	kActorTeenager = 46,
	kActorHysteriaPatron1 = 47,
	kActorHysteriaPatron2 = 48,
	kActorHysteriaPatron3 = 49,
	kActorShoeshineMan = 50,
	kActorTyrell = 51,
	kActorChew = 52,
	kActorGaff = 53,
	kActorBryant = 54,
	kActorTaffy = 55,
	kActorSebastian = 56,
	kActorRachael = 57,
	kActorGeneralDoll = 58,
	kActorIsabella = 59,
	kActorBlimpGuy = 60,
	kActorNewscaster = 61,
	kActorLeon = 62,
	kActorMaleAnnouncer = 63,
	kActorFreeSlotA = 64,
	kActorFreeSlotB = 65,
	kActorMaggie = 66,
	kActorGenwalkerA = 67,
	kActorGenwalkerB = 68,
	kActorGenwalkerC = 69,
	kActorMutant1 = 70,
	kActorMutant2 = 71,
	kActorMutant3 = 72,
	kActorVoiceOver = 99
};

enum Clues {
	kClueOfficersStatement = 0,
	kClueDoorForced1 = 1,
	kClueDoorForced2 = 2,
	kClueLimpingFootprints = 3,
	kClueGracefulFootprints = 4,
	kClueShellCasings = 5,
	kClueCandy = 6,
	kClueToyDog = 7,
	kClueChopstickWrapper = 8,
	kClueSushiMenu = 9,
	kClueLabCorpses = 10,
	kClueLabShellCasings = 11,
	kClueRuncitersVideo = 12,
	kClueLucy = 13,
	kClueDragonflyAnklet = 14,
	kClueReferenceLetter = 15,
	kClueCrowdInterviewA = 16,
	kClueCrowdInterviewB = 17,
	kClueZubenRunsAway = 18,
	kClueZubenInterview = 19,
	kClueZuben = 20,
	kClueBigManLimping = 21,
	kClueRunciterInterviewA = 22,
	kClueRunciterInterviewB1 = 23,
	kClueRunciterInterviewB2 = 24,
	kClueHowieLeeInterview = 25,
	kCluePaintTransfer = 26,
	kClueChromeDebris = 27,
	kClueRuncitersViewA = 28,
	kClueRuncitersViewB = 29,
	kClueCarColorAndMake = 30,
	kCluePartialLicenseNumber = 31,
	kClueBriefcase = 32,
	kClueGaffsInformation = 33,
	kClueCrystalVisitedRunciters = 34,
	kClueCrystalVisitedChinatown = 35,
	kClueWantedPoster = 36,
	kClueLicensePlate = 37,
	kClueLicensePlateMatch = 38,
	kClueLabPaintTransfer = 39,
	kClueDispatchHitAndRun = 40,
	kClueInceptShotRoy = 41,
	kClueInceptShotsLeon = 42,
	kCluePhoneCallGuzza = 43,
	kClueDragonflyEarring = 44,
	kClueTyrellSecurity = 45,
	kClueTyrellGuardInterview = 46,
	kClueBombingSuspect = 47,
	kClueSadiksGun = 48,
	kClueDetonatorWire = 49,
	kClueVictimInformation = 50,
	kClueAttemptedFileAccess = 51,
	kClueCrystalsCase = 52,
	kClueKingstonKitchenBox1 = 53,
	kClueTyrellSalesPamphlet1 = 54,
	kClueTyrellSalesPamphlet2 = 55,
	kCluePeruvianLadyInterview = 56,
	kClueHasanInterview = 57,
	kClueBobInterview1 = 58,
	kClueBobInterview2 = 59,
	kClueIzoInterview = 60,
	kClueIzosWarning = 61,
	kClueRadiationGoggles = 62,
	kClueGogglesReplicantIssue = 63,
	kClueFishLadyInterview = 64,
	kClueDogCollar1 = 65,
	kClueWeaponsCache = 66,
	kClueChewInterview = 67,
	kClueMorajiInterview = 68,
	kClueGordoInterview1 = 69,
	kClueGordoInterview2 = 70,
	kClueAnsweringMachineMessage = 71,
	kClueChessTable = 72,
	kClueSightingSadikBradbury = 73,
	kClueStaggeredbyPunches = 74,
	kClueMaggieBracelet = 75,
	kClueEnvelope = 76,
	kClueIzosFriend = 77,
	kClueChinaBarSecurityPhoto = 78,
	kCluePurchasedScorpions = 79,
	kClueWeaponsOrderForm = 80,
	kClueShippingForm = 81,
	kClueGuzzasCash = 82,
	kCluePoliceIssueWeapons = 83,
	kClueHysteriaToken = 84,
	kClueRagDoll = 85,
	kClueMoonbus1 = 86,
	kClueCheese = 87,
	kClueDektorasDressingRoom = 88,
	kClueEarlyQsClub = 89,
	kClueDragonflyCollection = 90,
	kClueDragonflyBelt = 91,
	kClueEarlyQInterview = 92,
	kClueStrangeScale1 = 93,
	kClueDektoraInterview1 = 94,
	kClueSuspectDektora = 95,
	kClueDektoraInterview2 = 96,
	kClueDektoraInterview3 = 97,
	kClueDektorasCard = 98,
	kClueGrigoriansNote = 99,
	kClueCollectionReceipt = 100,
	kClueSpecialIngredient = 101,
	kClueStolenCheese = 102,
	kClueGordoInterview3 = 103,
	kClueGordoConfession = 104,
	kClueGordosLighter1 = 105,
	kClueGordosLighter2 = 106,
	kClueDektoraInterview4 = 107,
	kClueHollowayInterview = 108,
	kClueBakersBadge = 109,
	kClueHoldensBadge = 110,
	kClueCar = 111,
	kClueCarIdentified = 112,
	kClueCarRegistration1 = 113,
	kClueCarRegistration2 = 114,
	kClueCarRegistration3 = 115,
	kClueCrazylegsInterview1 = 116,
	kClueCrazylegsInterview2 = 117,
	kClueLichenDogWrapper = 118,
	kClueRequisitionForm = 119,
	kClueScaryChair = 120,
	kClueIzosStashRaided = 121,
	kClueHomelessManInterview1 = 122,
	kClueHomelessManInterview2 = 123,
	kClueHomelessManKid = 124,
	kClueFolder = 125,
	kClueGuzzaFramedMcCoy = 126,
	kClueOriginalShippingForm = 127,
	kClueOriginalRequisitionForm = 128,
	kClueCandyWrapper = 129,
	kClueGordoBlabs = 130,
	kClueFlaskOfAbsinthe = 131,
	kClueGuzzaAgreesToMeet = 132,
	kClueDektoraConfession = 133,
	kClueRunciterConfession1 = 134,
	kClueRunciterConfession2 = 135,
	kClueLutherLanceInterview = 136,
	kClueMoonbus2 = 137,
	kClueMoonbusCloseup = 138,
	kCluePhoneCallDektora1 = 139,
	kCluePhoneCallDektora2 = 140,
	kCluePhoneCallLucy1 = 141,
	kCluePhoneCallLucy2 = 142,
	kCluePhoneCallClovis = 143,
	kCluePhoneCallCrystal = 144,
	kCluePowerSource = 145,
	kClueBomb = 146,
	kClueDNATyrell = 147,
	kClueDNASebastian = 148,
	kClueDNAChew = 149,
	kClueDNAMoraji = 150,
	kClueDNALutherLance = 151,
	kClueDNAMarcus = 152,
	kClueGarterSnake = 153,
	kClueSlug = 154,
	kClueGoldfish = 155,
	kClueZubenTalksAboutLucy1 = 156,
	kClueZubenTalksAboutLucy2 = 157,
	kClueZubensMotive = 158,
	kClueSightingBulletBob = 159,
	kClueSightingClovis = 160,
	kClueSightingDektora = 161,
	kClueVKDektoraReplicant = 162,
	kClueVKDektoraHuman = 163,
	kClueVKBobGorskyReplicant = 164,
	kClueVKBobGorskyHuman = 165,
	kClueVKLutherLanceReplicant = 166,
	kClueVKLutherLanceHuman = 167,
	kClueVKGrigorianReplicant = 168,
	kClueVKGrigorianHuman = 169,
	kClueVKIzoReplicant = 170,
	kClueVKIzoHuman = 171,
	kClueVKCrazylegsReplicant = 172,
	kClueVKCrazylegsHuman = 173,
	kClueVKRunciterReplicant = 174,
	kClueVKRunciterHuman = 175,
	kClueVKEarlyQReplicant = 176,
	kClueVKEarlyQHuman = 177,
	kClueCrimeSceneNotes = 178,
	kClueGrigorianInterviewA = 179,
	kClueGrigorianInterviewB1 = 180,
	kClueGrigorianInterviewB2 = 181,
	kClueLabAnalysisGoldChain = 182,
	kClueSightingZuben = 183,
	kClueCrystalRetiredZuben = 184,
	kClueCrystalRetiredGordo = 185,
	kClueSightingGordo = 186,
	kClueCrystalRetiredIzo = 187,
	kClueClovisIncept = 188,
	kClueDektoraIncept = 189,
	kClueLucyIncept = 190,
	kClueGordoIncept = 191,
	kClueIzoIncept = 192,
	kClueSadikIncept = 193,
	kClueZubenIncept = 194,
	kClueMcCoyIncept = 195,
	kClueWarRecordsGordoFrizz = 196,
	kCluePoliceWeaponUsed = 197,
	kClueMcCoysWeaponUsedonBob = 198,
	kClueBobRobbed = 199,
	kClueBobShotInSelfDefense = 200,
	kClueBobShotInColdBlood = 201,
	kClueMcCoyRecoveredHoldensBadge = 202,
	kClueCrystalTestedBulletBob = 203,
	kClueCrystalRetiredBob = 204,
	kClueCrystalTestedCrazylegs = 205,
	kClueCrystalRetiredCrazylegs = 206,
	kClueCrystalArrestedCrazylegs = 207,
	kClueCrystalTestedRunciter = 208,
	kClueCrystalRetiredRunciter1 = 209,
	kClueCrystalRetiredRunciter2 = 210,
	kClueSightingMcCoyRuncitersShop = 211,
	kClueMcCoyKilledRunciter1 = 212,
	kClueMcCoysDescription = 213,
	kClueMcCoyIsABladeRunner = 214,
	kClueMcCoyLetZubenEscape = 215,
	kClueMcCoyWarnedIzo = 216,
	kClueMcCoyHelpedIzoIzoIsAReplicant = 217,
	kClueMcCoyHelpedDektora = 218,
	kClueMcCoyHelpedLucy = 219,
	kClueMcCoyHelpedGordo = 220,
	kClueMcCoyShotGuzza = 221,
	kClueMcCoyRetiredZuben = 222,
	kClueMcCoyRetiredLucy = 223,
	kClueMcCoyRetiredDektora = 224,
	kClueMcCoyRetiredGordo = 225,
	kClueMcCoyRetiredSadik = 226,
	kClueMcCoyShotZubenintheback = 227,
	kClueMcCoyRetiredLutherLance = 228,
	kClueMcCoyBetrayal = 229,
	kClueMcCoyKilledRunciter2 = 230,
	kClueClovisOrdersMcCoysDeath = 231,
	kClueEarlyAttemptedToSeduceLucy = 232,
	kClueCarWasStolen = 233,
	kClueGrigoriansResponse1 = 234,
	kClueGrigoriansResponse2 = 235,
	kClueCrazysInvolvement = 236,
	kClueGrigoriansResources = 237,
	kClueMcCoyPulledAGun = 238,
	kClueMcCoyIsStupid = 239,
	kClueMcCoyIsAnnoying = 240,
	kClueMcCoyIsKind = 241,
	kClueMcCoyIsInsane = 242,
	kClueAnimalMurderSuspect = 243,
	kClueMilitaryBoots = 244,
	kClueOuterDressingRoom = 245,
	kCluePhotoOfMcCoy1 = 246,
	kCluePhotoOfMcCoy2 = 247,
	kClueEarlyQAndLucy = 248,
	kClueClovisflowers = 249,
	kClueLucyWithDektora = 250,
	kClueWomanInAnimoidRow = 251,
	kClueScorpions = 252,
	kClueStrangeScale2 = 253,
	kClueChinaBarSecurityCamera = 254,
	kClueIzo = 255,
	kClueGuzza = 256,
	kClueChinaBarSecurityDisc = 257,
	kClueScorpionbox = 258,
	kClueTyrellSecurityPhoto = 259,
	kClueChinaBar = 260,
	kCluePlasticExplosive = 261,
	kClueDogCollar2 = 262,
	kClueKingstonKitchenBox2 = 263,
	kClueCrystalsCigarette = 264,
	kClueSpinnerKeys = 265,
	kClueAct2Ended = 266,
	kClueAct3Ended = 267,
	kClueAct4Ended = 268,
	kClueExpertBomber = 269,
	kClueAmateurBomber = 270,
	kClueVKLucyReplicant = 271,
	kClueVKLucyHuman = 272,
	kClueLucyInterview = 273,
	kClueMoonbusReflection = 274,
	kClueMcCoyAtMoonbus = 275,
	kClueClovisAtMoonbus = 276,
	kClueSadikAtMoonbus = 277,
	kClueRachaelInterview = 278,
	kClueTyrellInterview = 279,
	kClueRuncitersConfession1 = 280,
	kClueRuncitersConfession2 = 281,
	kClueRuncitersConfession3 = 282,
	kClueEarlyInterviewA = 283,
	kClueEarlyInterviewB1 = 284,
	kClueEarlyInterviewB2 = 285,
	kClueCrazylegsInterview3 = 286,
	kClueCrazylegGgrovels = 287
};

enum ClueTypes {
	kClueTypePhotograph  = 0,
	kClueTypeVideoClip = 1,
	kClueTypeAudioRecording = 2,
	kClueTypeObject = 3
};

enum Crimes {
	kCrimeAnimalMurder = 0,
	kCrimeEisendullerMurder = 1,
	kCrimeArmsDealing = 2,
	kCrimeMorajiMurder = 3,
	kCrimeBradburyAssault = 4,
	kCrimeFactoryBombing = 5,
	kCrimeBobMurder = 6,
	kCrimeRunciterMurder = 7,
	kCrimeMoonbusHijacking = 8
};

enum SpinnerDestinations {
	kSpinnerDestinationPoliceStation = 0,
	kSpinnerDestinationMcCoysApartment = 1,
	kSpinnerDestinationRuncitersAnimals = 2,
	kSpinnerDestinationChinatown = 3,
	kSpinnerDestinationAnimoidRow = 4,
	kSpinnerDestinationTyrellBuilding = 5,
	kSpinnerDestinationDNARow = 6,
	kSpinnerDestinationBradburyBuilding = 7,
	kSpinnerDestinationNightclubRow = 8,
	kSpinnerDestinationHysteriaHall = 9
};

class BladeRunnerEngine;

class ScriptBase {
protected:
	BladeRunnerEngine *_vm;

public:
	ScriptBase(BladeRunnerEngine *vm) : _vm(vm) {}
	virtual ~ScriptBase() {}

protected:
	void Preload(int animationId);
	void Actor_Put_In_Set(int actorId, int set);
	void Actor_Set_At_XYZ(int actorId, float x, float y, float z, int direction);
	void Actor_Set_At_Waypoint(int actorId, int waypointId, int angle);
	bool Region_Check(int left, int top, int right, int down);
	bool Object_Query_Click(const char *objectName1, const char *objectName2);
	void Object_Do_Ground_Click();
	bool Object_Mark_For_Hot_Mouse(const char *objectName);
	void Actor_Face_Actor(int actorId, int otherActorId, bool animate);
	void Actor_Face_Object(int actorId, const char *objectName, bool animate);
	void Actor_Face_Item(int actorId, int itemId, bool animate);
	void Actor_Face_Waypoint(int actorId, int waypointId, bool animate);
	void Actor_Face_XYZ(int actorId, float x, float y, float z, bool animate);
	void Actor_Face_Current_Camera(int actorId, bool animate);
	void Actor_Face_Heading(int actorId, int heading, bool animate);
	int Actor_Query_Friendliness_To_Other(int actorId, int otherActorId);
	void Actor_Modify_Friendliness_To_Other(int actorId, int otherActorId, signed int change);
	void Actor_Set_Friendliness_To_Other(int actorId, int otherActorId, int friendliness);
	void Actor_Set_Honesty(int actorId, int honesty);
	void Actor_Set_Intelligence(int actorId, int intelligence);
	void Actor_Set_Stability(int actorId, int stability);
	void Actor_Set_Combat_Aggressiveness(int actorId, int combatAggressiveness);
	int Actor_Query_Current_HP(int actorId);
	int Actor_Query_Max_HP(int actorId);
	int Actor_Query_Combat_Aggressiveness(int actorId);
	int Actor_Query_Honesty(int actorId);
	int Actor_Query_Intelligence(int actorId);
	int Actor_Query_Stability(int actorId);
	void Actor_Modify_Current_HP(int actorId, signed int change);
	void Actor_Modify_Max_HP(int actorId, signed int change);
	void Actor_Modify_Combat_Aggressiveness(int actorId, signed int change);
	void Actor_Modify_Honesty(int actorId, signed int change);
	void Actor_Modify_Intelligence(int actorId, signed int change);
	void Actor_Modify_Stability(int actorId, signed int change);
	void Actor_Set_Flag_Damage_Anim_If_Moving(int actorId, bool value);
	bool Actor_Query_Flag_Damage_Anim_If_Moving(int actorId);
	void Actor_Combat_AI_Hit_Attempt(int actorId);
	void Non_Player_Actor_Combat_Mode_On(int actorId, int a2, int a3, int otherActorId, int a5, int animationModeCombatIdle, int animationModeCombatWalk, int animationModeCombatRun, int a9, int a10, int a11, int a12, int a13, int a14);
	void Non_Player_Actor_Combat_Mode_Off(int actorId);
	void Actor_Set_Health(int actorId, int hp, int maxHp);
	void Actor_Set_Targetable(int actorId, bool targetable);
	void Actor_Says(int actorId, int sentenceId, int animationMode);
	void Actor_Says_With_Pause(int actorId, int sentenceId, float pause, int animationMode);
	void Actor_Voice_Over(int sentenceId, int actorId);
	void Actor_Start_Speech_Sample(int actorId, int sentenceId);
	void Actor_Start_Voice_Over_Sample(int sentenceId);
	int Actor_Query_Which_Set_In(int actorId);
	bool Actor_Query_Is_In_Current_Set(int actorId);
	bool Actor_Query_In_Set(int actorId, int setId);
	int Actor_Query_Inch_Distance_From_Actor(int actorId, int otherActorId);
	int Actor_Query_Inch_Distance_From_Waypoint(int actorId, int waypointId);
	bool Actor_Query_In_Between_Two_Actors(int actorId, int otherActor1Id, int otherActor2Id);
	void Actor_Set_Goal_Number(int actorId, int goalNumber);
	int Actor_Query_Goal_Number(int actorId);
	void Actor_Query_XYZ(int actorId, float *x, float *y, float *z);
	int Actor_Query_Facing_1024(int actorId);
	void Actor_Set_Frame_Rate_FPS(int actorId, int fps);
	int Slice_Animation_Query_Number_Of_Frames(int animationId);
	void Actor_Change_Animation_Mode(int actorId, int animationMode);
	int Actor_Query_Animation_Mode(int actorId);
	bool Loop_Actor_Walk_To_Actor(int actorId, int otherActorId, int distance, int a4, bool running);
	bool Loop_Actor_Walk_To_Item(int actorId, int itemId, int a3, int a4, bool run);
	bool Loop_Actor_Walk_To_Scene_Object(int actorId, const char *objectName, int distance, bool a4, bool run);
	bool Loop_Actor_Walk_To_Waypoint(int actorId, int waypointId, int a3, int a4, bool run);
	bool Loop_Actor_Walk_To_XYZ(int actorId, float x, float y, float z, int a4, int a5, bool run, int a7);
	void Async_Actor_Walk_To_Waypoint(int actorId, int waypointId, int a3, int run);
	void Async_Actor_Walk_To_XYZ(int actorId, float x, float y, float z, int a5, bool run);
	void Actor_Force_Stop_Walking(int actorId);
	bool Loop_Actor_Travel_Stairs(int actorId, int a2, int a3, int a4);
	bool Loop_Actor_Travel_Ladder(int actorId, int a2, int a3, int a4);
	void Actor_Clue_Add_To_Database(int actorId, int clueId, int weight, bool clueAcquired, bool unknownFlag, int fromActorId);
	void Actor_Clue_Acquire(int actorId, int clueId, bool unknownFlag, int fromActorId);
	void Actor_Clue_Lose(int actorId, int clueId);
	bool Actor_Clue_Query(int actorId, int clueId);
	void Actor_Clues_Transfer_New_To_Mainframe(int actorId);
	void Actor_Clues_Transfer_New_From_Mainframe(int actorId);
	void Actor_Set_Invisible(int actorId, bool isInvisible);
	void Actor_Set_Immunity_To_Obstacles(int actorId, bool isImmune);
	void Item_Add_To_World(int itemId, int animationId, int setId, float x, float y, float z, signed int facing, int height, int width, bool isTargetable, bool isObstacle, bool isPoliceMazeEnemy, bool updateOnly);
	void Item_Remove_From_World(int itemId);
	void Item_Spin_In_World(int itemId);
	void Item_Flag_As_Target(int itemId);
	void Item_Flag_As_Non_Target(int itemId);
	void Item_Pickup_Spin_Effect(int a1, int a2, int a3);
	int Animation_Open();
	int Animation_Close();
	int Animation_Start();
	int Animation_Stop();
	int Animation_Skip_To_Frame();
	void Delay(int miliseconds);
	void Player_Loses_Control();
	void Player_Gains_Control();
	void Player_Set_Combat_Mode(bool activate);
	bool Player_Query_Combat_Mode();
	void Player_Set_Combat_Mode_Access(bool enable);
	int Player_Query_Current_Set();
	int Player_Query_Current_Scene();
	int Player_Query_Agenda();
	void Player_Set_Agenda(int agenda);
	int Query_Difficulty_Level();
	void Game_Flag_Set(int flag);
	void Game_Flag_Reset(int flag);
	bool Game_Flag_Query(int flag);
	void Set_Enter(int setId, int sceneId);
	void Chapter_Enter(int chapter, int setId, int sceneId);
	int Global_Variable_Set(int, int);
	int Global_Variable_Reset(int);
	int Global_Variable_Query(int);
	int Global_Variable_Increment(int, int);
	int Global_Variable_Decrement(int, int);
	int Random_Query(int min, int max);
	void Sound_Play(int id, int volume, int panFrom, int panTo, int priority);
	void Sound_Play_Speech_Line(int actorId, int speechId, int a3, int a4, int a5);
	void Sound_Left_Footstep_Walk(int actorId);
	void Sound_Right_Footstep_Walk(int actorId);
	void Sound_Left_Footstep_Run(int actorId);
	void Sound_Right_Footstep_Run(int actorId);
	// Sound_Walk_Shuffle_Stop
	void Footstep_Sounds_Set(int index, int value);
	void Footstep_Sound_Override_On(int footstepSoundOverride);
	void Footstep_Sound_Override_Off();
	bool Music_Play(int a1, int a2, int a3, int a4, int a5, int a6, int a7);
	void Music_Adjust(int a1, int a2, int a3);
	void Music_Stop(int a1);
	bool Music_Is_Playing();
	void Overlay_Play(const char *overlay, int a2, int a3, int a4, int a5);
	void Overlay_Remove(const char *overlay);
	void Scene_Loop_Set_Default(int);
	void Scene_Loop_Start_Special(int, int, int);
	void Outtake_Play(int id, int noLocalization = false, int container = -1);
	void Ambient_Sounds_Add_Sound(int id, int time1, int time2, int volume1, int volume2, int pan1begin, int pan1end, int pan2begin, int pan2end, int priority, int unk);
	void Ambient_Sounds_Remove_Sound(int id, bool a2);
	void Ambient_Sounds_Add_Speech_Sound(int id, int unk1, int time1, int time2, int volume1, int volume2, int pan1begin, int pan1end, int pan2begin, int pan2end, int priority, int unk2);
	// Ambient_Sounds_Remove_Speech_Sound
	int Ambient_Sounds_Play_Sound(int a1, int a2, int a3, int a4, int a5);
	// Ambient_Sounds_Play_Speech_Sound
	void Ambient_Sounds_Remove_All_Non_Looping_Sounds(int time);
	void Ambient_Sounds_Add_Looping_Sound(int id, int volume, int pan, int fadeInTime);
	void Ambient_Sounds_Adjust_Looping_Sound(int id, int panBegin, int panEnd, int a4);
	void Ambient_Sounds_Remove_Looping_Sound(int id, bool a2);
	void Ambient_Sounds_Remove_All_Looping_Sounds(int time);
	void Setup_Scene_Information(float actorX, float actorY, float actorZ, int actorFacing);
	bool Dialogue_Menu_Appear(int x, int y);
	bool Dialogue_Menu_Disappear();
	bool Dialogue_Menu_Clear_List();
	bool Dialogue_Menu_Add_To_List(int answer);
	bool Dialogue_Menu_Add_DONE_To_List(int answer);
	// Dialogue_Menu_Add_To_List_Never_Repeat_Once_Selected
	bool DM_Add_To_List(int answer, int a2, int a3, int a4);
	bool DM_Add_To_List_Never_Repeat_Once_Selected(int answer, int a2, int a3, int a4);
	void Dialogue_Menu_Remove_From_List(int answer);
	int Dialogue_Menu_Query_Input();
	int Dialogue_Menu_Query_List_Size();
	void Scene_Exit_Add_2D_Exit(int index, int left, int top, int right, int down, int type);
	void Scene_Exit_Remove(int index);
	void Scene_Exits_Disable();
	void Scene_Exits_Enable();
	void Scene_2D_Region_Add(int index, int left, int top, int right, int down);
	void Scene_2D_Region_Remove(int index);
	void World_Waypoint_Set(int waypointId, int setId, float x, float y, float z);
	// World_Waypoint_Reset
	float World_Waypoint_Query_X(int waypointId);
	float World_Waypoint_Query_Y(int waypointId);
	float World_Waypoint_Query_Z(int waypointId);
	void Combat_Cover_Waypoint_Set_Data(int combatCoverId, int a2, int setId, int a4, float x, float y, float z);
	void Combat_Flee_Waypoint_Set_Data(int combatFleeWaypointId, int a2, int setId, int a4, float x, float y, float z, int a8);
	void Police_Maze_Target_Track_Add(int itemId, float startX, float startY, float startZ, float endX, float endY, float endZ, int steps, signed int data[], bool a10);
	// Police_Maze_Query_Score
	// Police_Maze_Zero_Score
	// Police_Maze_Increment_Score
	// Police_Maze_Decrement_Score
	// Police_Maze_Set_Score
	void Police_Maze_Set_Pause_State(int a1);
	void CDB_Set_Crime(int clueId, int crimeId);
	void CDB_Set_Clue_Asset_Type(int clueId, int assetType);
	void SDB_Set_Actor(int suspectId, int actorId);
	bool SDB_Add_Photo_Clue(int suspectId, int clueId, int shapeId);
	void SDB_Set_Name(int suspectId);
	void SDB_Set_Sex(int suspectId, int sex);
	bool SDB_Add_Identity_Clue(int suspectId, int clueId);
	bool SDB_Add_MO_Clue(int suspectId, int clueId);
	bool SDB_Add_Whereabouts_Clue(int suspectId, int clueId);
	bool SDB_Add_Replicant_Clue(int suspectId, int clueId);
	bool SDB_Add_Non_Replicant_Clue(int suspectId, int clueId);
	bool SDB_Add_Other_Clue(int suspectId, int clueId);
	void Spinner_Set_Selectable_Destination_Flag(int a1, int a2);
	// Spinner_Query_Selectable_Destination_Flag
	int Spinner_Interface_Choose_Dest(int a1, int a2);
	void ESPER_Flag_To_Activate();
	bool Voight_Kampff_Activate(int a1, int a2);
	int Elevator_Activate(int elevator);
	void View_Score_Board();
	// Query_Score
	void Set_Score(int a0, int a1);
	void Give_McCoy_Ammo(int ammoType, int ammo);
	void Assign_Player_Gun_Hit_Sounds(int row, int soundId1, int soundId2, int soundId3);
	void Assign_Player_Gun_Miss_Sounds(int row, int soundId1, int soundId2, int soundId3);
	void Disable_Shadows(int animationsIdsList[], int listSize);
	bool Query_System_Currently_Loading_Game();
	void Actor_Retired_Here(int actorId, int width, int height, int retired, int retiredByActorId);
	void Clickable_Object(const char *objectName);
	void Unclickable_Object(const char *objectName);
	void Obstacle_Object(const char *objectName, bool updateWalkpath);
	void Unobstacle_Object(const char *objectName, bool updateWalkpath);
	void Obstacle_Flag_All_Objects(bool isObstacle);
	void Combat_Target_Object(const char *objectName);
	void Un_Combat_Target_Object(const char *objectName);
	void Set_Fade_Color(float r, float g, float b);
	void Set_Fade_Density(float density);
	void Set_Fog_Color(const char* fogName, float r, float g, float b);
	void Set_Fog_Density(const char* fogName, float density);
	void ADQ_Flush();
	void ADQ_Add(int actorId, int sentenceId, int animationMode);
	void ADQ_Add_Pause(int delay);
	bool Game_Over();
	void Autosave_Game(int textId);
	void I_Sez(const char *str);

	void AI_Countdown_Timer_Start(int actorId, signed int timer, int seconds);
	void AI_Countdown_Timer_Reset(int actorId, int timer);
	void AI_Movement_Track_Unpause(int actorId);
	void AI_Movement_Track_Pause(int actorId);
	void AI_Movement_Track_Repeat(int actorId);
	void AI_Movement_Track_Append_Run_With_Facing(int actorId, int waypointId, int delay, int angle);
	void AI_Movement_Track_Append_With_Facing(int actorId, int waypointId, int delay, int angle);
	void AI_Movement_Track_Append_Run(int actorId, int waypointId, int delay);
	void AI_Movement_Track_Append(int actorId, int waypointId, int delay);
	void AI_Movement_Track_Flush(int actorId);

	void ESPER_Add_Photo(const char* fileName, int a2, int a3);
	void ESPER_Define_Special_Region(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11, int a12, int a13, const char *name);

	void KIA_Play_Actor_Dialogue(int a1, int a2);
	void KIA_Play_Slice_Model(int a1);
	void KIA_Play_Photograph(int a1);

	void VK_Play_Speech_Line(int actorIndex, int a2, float a3);
	void VK_Add_Question(int a1, int a2, int a3);
	void VK_Subject_Reacts(int a1, int a2, int a3, int a4);
	void VK_Eye_Animates(int a1);
};

} // End of namespace BladeRunner

#endif
