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

#ifndef BLADERUNNER_GAME_CONSTANTS_H
#define BLADERUNNER_GAME_CONSTANTS_H

namespace BladeRunner {

enum Actors {
	kActorMcCoy            =  0,
	kActorSteele           =  1,
	kActorGordo            =  2,
	kActorDektora          =  3,
	kActorGuzza            =  4,
	kActorClovis           =  5,
	kActorLucy             =  6,
	kActorIzo              =  7,
	kActorSadik            =  8,
	kActorCrazylegs        =  9,
	kActorLuther           = 10,
	kActorGrigorian        = 11,
	kActorTransient        = 12, // Homeless
	kActorLance            = 13,
	kActorBulletBob        = 14,
	kActorRunciter         = 15,
	kActorInsectDealer     = 16,
	kActorTyrellGuard      = 17,
	kActorEarlyQ           = 18,
	kActorZuben            = 19,
	kActorHasan            = 20,
	kActorMarcus           = 21,
	kActorMia              = 22,
	kActorOfficerLeary     = 23,
	kActorOfficerGrayford  = 24,
	kActorHanoi            = 25,
	kActorBaker            = 26,
	kActorDeskClerk        = 27,
	kActorHowieLee         = 28,
	kActorFishDealer       = 29,
	kActorKlein            = 30,
	kActorMurray           = 31,
	kActorHawkersBarkeep   = 32,
	kActorHolloway         = 33,
	kActorSergeantWalls    = 34,
	kActorMoraji           = 35,
	kActorTheBard          = 36,
	kActorPhotographer     = 37,
	kActorDispatcher       = 38,
	kActorAnsweringMachine = 39,
	kActorRajif            = 40,
	kActorGovernorKolvig   = 41,
	kActorEarlyQBartender  = 42,
	kActorHawkersParrot    = 43,
	kActorTaffyPatron      = 44,
	kActorLockupGuard      = 45,
	kActorTeenager         = 46,
	kActorHysteriaPatron1  = 47,
	kActorHysteriaPatron2  = 48,
	kActorHysteriaPatron3  = 49,
	kActorShoeshineMan     = 50,
	kActorTyrell           = 51,
	kActorChew             = 52,
	kActorGaff             = 53,
	kActorBryant           = 54,
	kActorTaffy            = 55,
	kActorSebastian        = 56,
	kActorRachael          = 57,
	kActorGeneralDoll      = 58,
	kActorIsabella         = 59,
	kActorBlimpGuy         = 60,
	kActorNewscaster       = 61,
	kActorLeon             = 62,
	kActorMaleAnnouncer    = 63,
	kActorFreeSlotA        = 64, // Rat
	kActorFreeSlotB        = 65,
	kActorMaggie           = 66,
	kActorGenwalkerA       = 67,
	kActorGenwalkerB       = 68,
	kActorGenwalkerC       = 69,
	kActorMutant1          = 70,
	kActorMutant2          = 71,
	kActorMutant3          = 72,
	kActorVoiceOver        = 99
};

enum Clues {
	kClueOfficersStatement                 =   0,
	kClueDoorForced1                       =   1,
	kClueDoorForced2                       =   2,
	kClueLimpingFootprints                 =   3,
	kClueGracefulFootprints                =   4,
	kClueShellCasings                      =   5,
	kClueCandy                             =   6,
	kClueToyDog                            =   7,
	kClueChopstickWrapper                  =   8,
	kClueSushiMenu                         =   9,
	kClueLabCorpses                        =  10,
	kClueLabShellCasings                   =  11,
	kClueRuncitersVideo                    =  12,
	kClueLucy                              =  13,
	kClueDragonflyAnklet                   =  14,
	kClueReferenceLetter                   =  15,
	kClueCrowdInterviewA                   =  16,
	kClueCrowdInterviewB                   =  17,
	kClueZubenRunsAway                     =  18,
	kClueZubenInterview                    =  19,
	kClueZubenSquadPhoto                   =  20,
	kClueBigManLimping                     =  21,
	kClueRunciterInterviewA                =  22,
	kClueRunciterInterviewB1               =  23,
	kClueRunciterInterviewB2               =  24,
	kClueHowieLeeInterview                 =  25,
	kCluePaintTransfer                     =  26,
	kClueChromeDebris                      =  27,
	kClueRuncitersViewA                    =  28,
	kClueRuncitersViewB                    =  29,
	kClueCarColorAndMake                   =  30,
	kCluePartialLicenseNumber              =  31,
	kClueBriefcase                         =  32,
	kClueGaffsInformation                  =  33,
	kClueCrystalVisitedRunciters           =  34,
	kClueCrystalVisitedChinatown           =  35,
	kClueWantedPoster                      =  36,
	kClueLicensePlate                      =  37,
	kClueLicensePlateMatch                 =  38,
	kClueLabPaintTransfer                  =  39,
	kClueDispatchHitAndRun                 =  40, // is now acquired in restored Cut Content
	kClueInceptShotRoy                     =  41,
	kClueInceptShotsLeon                   =  42,
	kCluePhoneCallGuzza                    =  43,
	kClueDragonflyEarring                  =  44,
	kClueTyrellSecurity                    =  45,
	kClueTyrellGuardInterview              =  46,
	kClueBombingSuspect                    =  47,
	kClueSadiksGun                         =  48,
	kClueDetonatorWire                     =  49,
	kClueVictimInformation                 =  50,
	kClueAttemptedFileAccess               =  51,
	kClueCrystalsCase                      =  52,
	kClueKingstonKitchenBox1               =  53,
	kClueTyrellSalesPamphletEntertainModel =  54,
	kClueTyrellSalesPamphletLolita         =  55,
	kCluePeruvianLadyInterview             =  56,
	kClueHasanInterview                    =  57,
	kClueBobInterview1                     =  58,
	kClueBobInterview2                     =  59,
	kClueIzoInterview                      =  60,
	kClueIzosWarning                       =  61,
	kClueRadiationGoggles                  =  62,
	kClueGogglesReplicantIssue             =  63,
	kClueFishLadyInterview                 =  64,
	kClueDogCollar1                        =  65,
	kClueWeaponsCache                      =  66,
	kClueChewInterview                     =  67,
	kClueMorajiInterview                   =  68,
	kClueGordoInterview1                   =  69,
	kClueGordoInterview2                   =  70,
	kClueAnsweringMachineMessage           =  71,
	kClueChessTable                        =  72,
	kClueSightingSadikBradbury             =  73,
	kClueStaggeredbyPunches                =  74,
	kClueMaggieBracelet                    =  75,
	kClueEnvelope                          =  76,
	kClueIzosFriend                        =  77,
	kClueChinaBarSecurityPhoto             =  78,
	kCluePurchasedScorpions                =  79,
	kClueWeaponsOrderForm                  =  80,
	kClueShippingForm                      =  81,
	kClueGuzzasCash                        =  82,
	kCluePoliceIssueWeapons                =  83,
	kClueHysteriaToken                     =  84,
	kClueRagDoll                           =  85,
	kClueMoonbus1                          =  86,
	kClueCheese                            =  87,
	kClueDektorasDressingRoom              =  88,
	kClueEarlyQsClub                       =  89,
	kClueDragonflyCollection               =  90,
	kClueDragonflyBelt                     =  91,
	kClueEarlyQInterview                   =  92,
	kClueStrangeScale1                     =  93,
	kClueDektoraInterview1                 =  94,
	kClueSuspectDektora                    =  95,
	kClueDektoraInterview2                 =  96,
	kClueDektoraInterview3                 =  97,
	kClueDektorasCard                      =  98,
	kClueGrigoriansNote                    =  99,
	kClueCollectionReceipt                 = 100,
	kClueSpecialIngredient                 = 101,
	kClueStolenCheese                      = 102,
	kClueGordoInterview3                   = 103,
	kClueGordoConfession                   = 104,
	kClueGordosLighterReplicant            = 105,
	kClueGordosLighterHuman                = 106,
	kClueDektoraInterview4                 = 107,
	kClueHollowayInterview                 = 108,
	kClueBakersBadge                       = 109,
	kClueHoldensBadge                      = 110,
	kClueCar                               = 111,
	kClueCarIdentified                     = 112,
	kClueCarRegistration1                  = 113,
	kClueCarRegistration2                  = 114,
	kClueCarRegistration3                  = 115,
	kClueCrazylegsInterview1               = 116,
	kClueCrazylegsInterview2               = 117,
	kClueLichenDogWrapper                  = 118,
	kClueRequisitionForm                   = 119,
	kClueScaryChair                        = 120,
	kClueIzosStashRaided                   = 121,
	kClueHomelessManInterview1             = 122,
	kClueHomelessManInterview2             = 123,
	kClueHomelessManKid                    = 124,
	kClueFolder                            = 125,
	kClueGuzzaFramedMcCoy                  = 126,
	kClueOriginalShippingForm              = 127,
	kClueOriginalRequisitionForm           = 128,
	kClueCandyWrapper                      = 129,
	kClueGordoBlabs                        = 130,
	kClueFlaskOfAbsinthe                   = 131,
	kClueGuzzaAgreesToMeet                 = 132,
	kClueDektoraConfession                 = 133,
	kClueRunciterConfession1               = 134,
	kClueRunciterConfession2               = 135,
	kClueLutherLanceInterview              = 136,
	kClueMoonbus2                          = 137,
	kClueMoonbusCloseup                    = 138,
	kCluePhoneCallDektora1                 = 139,
	kCluePhoneCallDektora2                 = 140,
	kCluePhoneCallLucy1                    = 141,
	kCluePhoneCallLucy2                    = 142,
	kCluePhoneCallClovis                   = 143,
	kCluePhoneCallCrystal                  = 144,
	kCluePowerSource                       = 145,
	kClueBomb                              = 146,
	kClueDNATyrell                         = 147,
	kClueDNASebastian                      = 148,
	kClueDNAChew                           = 149,
	kClueDNAMoraji                         = 150,
	kClueDNALutherLance                    = 151,
	kClueDNAMarcus                         = 152,
	kClueGarterSnake                       = 153,
	kClueSlug                              = 154,
	kClueGoldfish                          = 155,
	kClueZubenTalksAboutLucy1              = 156,
	kClueZubenTalksAboutLucy2              = 157,
	kClueZubensMotive                      = 158,
	kClueSightingBulletBob                 = 159,
	kClueSightingClovis                    = 160,
	kClueSightingDektora                   = 161,
	kClueVKDektoraReplicant                = 162,
	kClueVKDektoraHuman                    = 163,
	kClueVKBobGorskyReplicant              = 164,
	kClueVKBobGorskyHuman                  = 165,
	kClueVKLutherLanceReplicant            = 166,
	kClueVKLutherLanceHuman                = 167,
	kClueVKGrigorianReplicant              = 168,
	kClueVKGrigorianHuman                  = 169,
	kClueVKIzoReplicant                    = 170,
	kClueVKIzoHuman                        = 171,
	kClueVKCrazylegsReplicant              = 172,
	kClueVKCrazylegsHuman                  = 173,
	kClueVKRunciterReplicant               = 174,
	kClueVKRunciterHuman                   = 175,
	kClueVKEarlyQReplicant                 = 176,
	kClueVKEarlyQHuman                     = 177,
	kClueCrimeSceneNotes                   = 178,
	kClueGrigorianInterviewA               = 179,
	kClueGrigorianInterviewB1              = 180,
	kClueGrigorianInterviewB2              = 181,
	kClueLabAnalysisGoldChain              = 182,
	kClueSightingZuben                     = 183,
	kClueCrystalRetiredZuben               = 184,
	kClueCrystalRetiredGordo               = 185,
	kClueSightingGordo                     = 186,
	kClueCrystalRetiredIzo                 = 187,
	kClueClovisIncept                      = 188,
	kClueDektoraIncept                     = 189,
	kClueLucyIncept                        = 190,
	kClueGordoIncept                       = 191,
	kClueIzoIncept                         = 192,
	kClueSadikIncept                       = 193,
	kClueZubenIncept                       = 194,
	kClueMcCoyIncept                       = 195,
	kClueWarRecordsGordoFrizz              = 196,
	kCluePoliceWeaponUsed                  = 197,
	kClueMcCoysWeaponUsedonBob             = 198,
	kClueBobRobbed                         = 199,
	kClueBobShotInSelfDefense              = 200,
	kClueBobShotInColdBlood                = 201,
	kClueMcCoyRecoveredHoldensBadge        = 202,
	kClueCrystalTestedBulletBob            = 203,
	kClueCrystalRetiredBob                 = 204,
	kClueCrystalTestedCrazylegs            = 205,
	kClueCrystalRetiredCrazylegs           = 206,
	kClueCrystalArrestedCrazylegs          = 207,
	kClueCrystalTestedRunciter             = 208,
	kClueCrystalRetiredRunciter1           = 209,
	kClueCrystalRetiredRunciter2           = 210,
	kClueSightingMcCoyRuncitersShop        = 211,
	kClueMcCoyKilledRunciter1              = 212,
	kClueMcCoysDescription                 = 213,
	kClueMcCoyIsABladeRunner               = 214,
	kClueMcCoyLetZubenEscape               = 215,
	kClueMcCoyWarnedIzo                    = 216,
	kClueMcCoyHelpedIzoIzoIsAReplicant     = 217,
	kClueMcCoyHelpedDektora                = 218,
	kClueMcCoyHelpedLucy                   = 219,
	kClueMcCoyHelpedGordo                  = 220,
	kClueMcCoyShotGuzza                    = 221,
	kClueMcCoyRetiredZuben                 = 222,
	kClueMcCoyRetiredLucy                  = 223,
	kClueMcCoyRetiredDektora               = 224,
	kClueMcCoyRetiredGordo                 = 225,
	kClueMcCoyRetiredSadik                 = 226,
	kClueMcCoyShotZubenInTheBack           = 227,
	kClueMcCoyRetiredLutherLance           = 228,
	kClueMcCoyBetrayal                     = 229,
	kClueMcCoyKilledRunciter2              = 230,
	kClueClovisOrdersMcCoysDeath           = 231,
	kClueEarlyAttemptedToSeduceLucy        = 232,
	kClueCarWasStolen                      = 233,
	kClueGrigoriansResponse1               = 234,
	kClueGrigoriansResponse2               = 235,
	kClueCrazysInvolvement                 = 236,
	kClueGrigoriansResources               = 237,
	kClueMcCoyPulledAGun                   = 238,
	kClueMcCoyIsStupid                     = 239,
	kClueMcCoyIsAnnoying                   = 240,
	kClueMcCoyIsKind                       = 241,
	kClueMcCoyIsInsane                     = 242,
	kClueAnimalMurderSuspect               = 243,
	kClueMilitaryBoots                     = 244,
	kClueOuterDressingRoom                 = 245,
	kCluePhotoOfMcCoy1                     = 246,
	kCluePhotoOfMcCoy2                     = 247,
	kClueEarlyQAndLucy                     = 248,
	kClueClovisflowers                     = 249,
	kClueLucyWithDektora                   = 250,
	kClueWomanInAnimoidRow                 = 251,
	kClueScorpions                         = 252,
	kClueStrangeScale2                     = 253,
	kClueChinaBarSecurityCamera            = 254,
	kClueIzo                               = 255,
	kClueGuzza                             = 256,
	kClueChinaBarSecurityDisc              = 257,
	kClueScorpionbox                       = 258,
	kClueTyrellSecurityPhoto               = 259,
	kClueChinaBar                          = 260,
	kCluePlasticExplosive                  = 261,
	kClueDogCollar2                        = 262,
	kClueKingstonKitchenBox2               = 263, // ESPER hard-copy
	kClueCrystalsCigarette                 = 264,
	kClueSpinnerKeys                       = 265,
	kClueAct2Ended                         = 266, // is acquired but never checked. Has no type and seems like a placeholder
	kClueAct3Ended                         = 267, // unused
	kClueAct4Ended                         = 268, // unused
	kClueExpertBomber                      = 269,
	kClueAmateurBomber                     = 270,
	kClueVKLucyReplicant                   = 271,
	kClueVKLucyHuman                       = 272,
	kClueLucyInterview                     = 273,
	kClueMoonbusReflection                 = 274,
	kClueMcCoyAtMoonbus                    = 275,
	kClueClovisAtMoonbus                   = 276,
	kClueSadikAtMoonbus                    = 277,
	kClueRachaelInterview                  = 278,
	kClueTyrellInterview                   = 279,
	kClueRuncitersConfession1              = 280,
	kClueRuncitersConfession2              = 281,
	kClueRuncitersConfession3              = 282,
	kClueEarlyInterviewA                   = 283,
	kClueEarlyInterviewB1                  = 284,
	kClueEarlyInterviewB2                  = 285,
	kClueCrazylegsInterview3               = 286,
	kClueCrazylegGgrovels                  = 287
};

enum ClueTypes {
	kClueTypeIntangible     = -1,
	kClueTypePhotograph     =  0,
	kClueTypeVideoClip      =  1,
	kClueTypeAudioRecording =  2,
	kClueTypeObject         =  3
};

enum Suspects {
	kSuspectSadik           = 0,
	kSuspectClovis          = 1,
	kSuspectZuben           = 2,
	kSuspectLucy            = 3,
	kSuspectDekora          = 4,
	kSuspectGordo           = 5,
	kSuspectIzo             = 6,
	kSuspectMcCoy           = 7,
	kSuspectGuzza           = 8
};

enum Crimes {
	kCrimeAnimalMurder      = 0,
	kCrimeEisendullerMurder = 1,
	kCrimeArmsDealing       = 2,
	kCrimeMorajiMurder      = 3,
	kCrimeBradburyAssault   = 4,
	kCrimeFactoryBombing    = 5,
	kCrimeBobMurder         = 6,
	kCrimeRunciterMurder    = 7,
	kCrimeMoonbusHijacking  = 8
};

enum SpinnerDestinations {
	kSpinnerDestinationPoliceStation    = 0,
	kSpinnerDestinationMcCoysApartment  = 1,
	kSpinnerDestinationRuncitersAnimals = 2,
	kSpinnerDestinationChinatown        = 3,
	kSpinnerDestinationAnimoidRow       = 4,
	kSpinnerDestinationTyrellBuilding   = 5,
	kSpinnerDestinationDNARow           = 6,
	kSpinnerDestinationBradburyBuilding = 7,
	kSpinnerDestinationNightclubRow     = 8,
	kSpinnerDestinationHysteriaHall     = 9
};

enum Flags {
	kFlagNotUsed0                             =   0, // is never checked
	kFlagRC02Entered                          =   1,
	kFlagGamePlayedInRestoredContentMode      =   2, // Re-purposed. Original: 2 is never used
	kFlagRC01GotOfficersStatement             =   3,
	kFlagRC02Left                             =   4,
	kFlagMcCoyCommentsOnMurderedAnimals       =   5, // Re-purposed. Original: 5 is never used
	kFlagRC02RunciterInterview                =   6,
	kFlagCT01TalkToHowieAboutDeadZuben        =   7, // Re-purposed. Original: 7 is never used
	kFlagRC01toRC02                           =   8,
	kFlagRC02toRC01                           =   9,
	kFlagDR01toDR04                           =  10,
	kFlagDR04toDR01                           =  11,
	kFlagPS07KleinTalkShellCasings            =  12,
	kFlagPS07KleinTalkOfficersStatement       =  13,
	kFlagPS10toPS11                           =  14,
	kFlagPS11toPS10                           =  15,
	kFlagPS11toPS12                           =  16,
	kFlagPS12toPS11                           =  17, // is never checked
	kFlagPS12toPS13                           =  18,
	kFlagPS13toPS12                           =  19,
	kFlagRC02RunciterVKChosen                 =  20, // Re-purposed. Original: 20 is never used
	kFlagPS13toPS05                           =  21,
	kFlagPS02toPS05                           =  22,
	kFlagPS06toPS05                           =  23,
	kFlagIntroPlayed                          =  24,
	kFlagCT01Visited                          =  25,
	kFlagCT01McCoyTalkedToHowieLee            =  26,
	kFlagCT01Evidence1Linked                  =  27,
	kFlagCT01Evidence2Linked                  =  28,
	kFlagCT01ZubenLeft                        =  29,
	kFlagCT01TalkToHowieAfterZubenMissing     =  30, // New, it is now set/unset to restore cut content there. Original: is never set
	kFlagCT01ZubenGone                        =  31,
	kFlagCT01GordoTalk                        =  32,
	kFlagMA02toMA06                           =  33,
	kFlagMA06ToMA02                           =  34,
	kFlagMA02ToMA04                           =  35,
	kFlagMA04ToMA02                           =  36,
	kFlagMA01toMA06                           =  37,
	kFlagMA06toMA01                           =  38,
	kFlagPS04toPS03                           =  39,
	kFlagZubenRetired                         =  40,
	kFlagZubenSpared                          =  41,
	kFlagPS03toPS04                           =  42,
	kFlagPS15Entered                          =  43,
	kFlagIzoIsReplicant                       =  44,
	kFlagGordoIsReplicant                     =  45,
	kFlagLucyIsReplicant                      =  46,
	kFlagDektoraIsReplicant                   =  47,
	kFlagSadikIsReplicant                     =  48,
	kFlagPS09GrigorianTalk1                   =  49,
	kFlagPS09GrigorianVKChosen                =  50, // Re-purposed. Original: 50 is never used
	kFlagGrigorianDislikeMcCoy                =  51,
	kFlagRC02McCoyCommentsOnVideoScreens      =  52, // Re-purposed. Original: 52 is never used
	kFlagPS09GrigorianDialogue                =  53,
	kFlagPS09GrigorianTalk2                   =  54,
	kFlagGrigorianArrested                    =  55,
	kFlagPS09CrazylegsGrigorianTalk           =  56,
	kFlagMA07toMA06                           =  57,
	kFlagMA06toMA07                           =  58,
	kFlagCT02ZubenTalk                        =  59,
	kFlagMA02MaggieIntroduced                 =  60,
	kFlagChapter1Ended                        =  61,
	kFlagMA04toMA05                           =  62,
	kFlagMA05toMA04                           =  63,
	kFlagGaffApproachedMcCoyAboutZuben        =  64,
	// 65 is never used
	// 66 is never used
	// 67 is never used
	kFlagCT02toCT01                           =  68,
	kFlagCT02toCT03                           =  69,
	kFlagCT03toCT02                           =  70,
	kFlagCT03toCT01                           =  71,
	kFlagCT03toCT04                           =  72,
	kFlagCT04toCT03                           =  73,
	kFlagCT04toCT05                           =  74,
	kFlagCT05toCT04                           =  75,
	kFlagCT05toCT12                           =  76,
	kFlagCT05toCT06                           =  77,
	kFlagCT06toCT05                           =  78,
	kFlagCT06toCT08                           =  79,
	kFlagCT08toCT06                           =  80, // is never checked
	kFlagCT08toCT09                           =  81,
	kFlagCT09toCT10                           =  82, // is never checked
	kFlagCT09toCT11                           =  83, // is never checked
	kFlagCT09toCT08                           =  84, // is never checked
	kFlagCT10toCT09                           =  85,
	kFlagCT11toCT12                           =  86,
	// 87 is never used
	kFlagCT12toCT01                           =  88,
	kFlagCT12toCT03                           =  89,
	kFlagCT12toCT05                           =  90,
	kFlagCT12toCT11                           =  91,
	// 92 is never used
	// 93 is never used
	kFlagCT05WarehouseOpen                    =  94,
	kFlagTB05toTB02                           =  95,
	kFlagTB02toTB05                           =  96, // is never checked
	kFlagTB05toTB06                           =  97, // is never checked
	kFlagTB06toTB05                           =  98,
	kFlagTB05MonitorIntro                     =  99,
	kFlagTB05MonitorUnlockAttempt             = 100,
	kFlagTB05MonitorDone                      = 101,
	kFlagTB06Visited                          = 102,
	kFlagNotUsed103                           = 103, // is never set
	kFlagPS07KleinTalkPaintTransfer           = 104,
	kFlagPS07KleinTalkChromeDebris            = 105,
	kFlagHC04IsabellaTalk                     = 106,
	kFlagHC04toRC03                           = 107,
	kFlagRC03toHC04                           = 108,
	kFlagHC04toHC02                           = 109,
	kFlagHC02toHC04                           = 110,
	kFlagPS07KleinTalkClues                   = 111,
	kFlagMcCoyCommentsOnAnimoids              = 112, // Re-purposed. Original: 112 is never used
	kFlagScorpionsInAR02                      = 113, // Re-purposed. Original: 113 is never used
	kFlagRC03toRC01                           = 114,
	kFlagRC01toRC03                           = 115,
	kFlagRC03toAR02                           = 116,
	kFlagAR02toRC03                           = 117,
	kFlagRC03toUG01                           = 118,
	kFlagUG01toRC03                           = 119,
	kFlagRC03toRC04                           = 120, // is never checked
	kFlagRC04toRC03                           = 121,
	// 122 is never used
	kFlagCT01toCT12                           = 123,
	// 122 is never used
	kFlagMcCoyHasShellCasings                 = 125,
	kFlagMcCoyHasOfficersStatement            = 126,
	kFlagMcCoyHasPaintTransfer                = 127,
	kFlagMcCoyHasChromeDebris                 = 128,
	kFlagCT01ZubenMcCoyCheck                  = 129,
	kFlagPS02toPS01                           = 130,
	kFlagPS02toPS07                           = 131,
	kFlagPS02toPS03                           = 132,
	kFlagPS02toPS09                           = 133,
	kFlagPS03toPS14                           = 134,
	kFlagPS14toPS03                           = 135,
	kFlagPS05toPS06                           = 136,
	kFlagCT04HomelessTalk                     = 137,
	kFlagPS07KleinInsulted                    = 138,
	kFlagMcCoyTalkedToBulletBobAboutHasan     = 139, // Re-purposed. Original: 139 is never used
	// 140 is never used
	kFlagRC51Available                        = 141,
	kFlagNotUsed142                           = 142, // is never checked
	// 143 is never used
	kFlagCT07toCT06                           = 144,
	kFlagCT06ZubenPhoto                       = 145,
	kFlagChapter1Ending                       = 146,
	kFlagRC51ChopstickWrapperTaken            = 147,
	kFlagRC51CandyTaken                       = 148,
	kFlagRC51ToyDogTaken                      = 149,
	kFlagNotUsed150                           = 150, // has no use
	kFlagRC03UnlockedToUG01                   = 151,
	kFlagUG15toUG16a                          = 152,
	kFlagUG16toUG15a                          = 153,
	kFlagTB02toTB03                           = 154, // is never checked
	kFlagTB03toTB02                           = 155,
	kFlagPS07KleinInsultedTalk                = 156,
	kFlagSergeantWallsMazeInstructions        = 157, // Re-purposed. Original: 157 is never used
	kFlagSergeantWallsBuzzInDone              = 158,
	kFlagPS04GuzzaTalkZubenRetired            = 159,
	kFlagPS04GuzzaTalkZubenEscaped            = 160,
	kFlagPS04GuzzaTalk1                       = 161,
	kFlagPS04GuzzaTalk2                       = 162,
	kFlagRC01ChromeDebrisTaken                = 163,
	kFlagIzoArrested                          = 164,
	kFlagCrazylegsArrested                    = 165,
	kFlagPS09CrazylegsTalk1                   = 166,
	kFlagPS09IzoTalk1                         = 167,
	kFlagPS09IzoTalk2                         = 168,
	kFlagCT04HomelessKilledByMcCoy            = 169,
	kFlagCT04HomelessBodyInDumpster           = 170,
	kFlagCT04HomelessBodyFound                = 171,
	kFlagCT04HomelessBodyThrownAway           = 172,
	kFlagCT04HomelessBodyInDumpsterNotChecked = 173, // is never checked
	kFlagCT04LicensePlaceFound                = 174,
	kFlagPS09CrazylegsTalk2                   = 175,
	kFlagMcCoyInChinaTown                     = 176,
	kFlagMcCoyInDNARow                        = 177,
	kFlagMcCoyInPoliceStation                 = 178,
	kFlagMcCoyInMcCoyApartment                = 179,
	kFlagMcCoyInAnimoidRow                    = 180,
	kFlagMcCoyInNightclubRow                  = 181,
	kFlagMcCoyInRunciters                     = 182,
	kFlagSteeleInRunciters                    = 183,
	kFlagSteeleInChinaTown                    = 184,
	kFlagSteeleInPoliceStation                = 185,
	kFlagRC01PoliceDone                       = 186,
	kFlagRC02RunciterTalk1                    = 187,
	// 188 is never used
	// 189 is never used
	kFlagRC02ShellCasingsTaken                = 190,
	// 191 is never used
	kFlagCT01BoughtHowieLeeFood               = 192,
	// 193 is never used
	// 194 is never used
	// 195 is never used
	kFlagKleinAnimation1                      = 196,
	kFlagKleinAnimation2                      = 197,
	kFlagKleinAnimation3                      = 198,
	kFlagOfficerLearyTakingNotes              = 199,
	// 200 is never used
	// 201 is never used
	// 202 is never used
	// 203 is never used
	kFlagPS15toPS05                           = 204,
	// 205 is never used
	kFlagSergeantWallsBuzzInRequest           = 206, // is never set
	// 207 is never used
	// 208 is never used
	// 209 is never used
	kFlagCT02ZubenFled                        = 210,
	kFlagPS09Entered                          = 211,
	kFlagNotUsed212                           = 212, // is never checked
	// 213 is never used
	// 214 is never used
	// 215 is never used
	kFlagBB09toBB10a                          = 216,
	// 217 is never used
	kFlagBB08toBB09                           = 218,
	kFlagBB09toBB08                           = 219,
	kFlagBB09toBB10b                          = 220,
	kFlagBB10toBB09                           = 221,
	kFlagBB10toBB11                           = 222, // is never checked
	kFlagBB11toBB10                           = 223,
	kFlagDR01toDR02                           = 224,
	kFlagDR02toDR01                           = 225,
	kFlagDR02toDR03                           = 226,
	kFlagDR03toDR02                           = 227,
	kFlagDR04toDR05                           = 228, // is never checked
	kFlagDR05toDR04                           = 229,
	kFlagDR04toDR06                           = 230,
	kFlagDR06toDR04                           = 231,
	kFlagNotUsed232                           = 232,
	// 233 is never used
	kFlagCT02toCT01walk                       = 234,
	// 235 is never used
	kFlagMcCoyAnimation1                      = 236,
	// 237 is never used
	// 238 is never used
	kFlagHowieLeeAnimation1                   = 239,
	// 240 is never used
	// 241 is never used
	// 242 is never used
	// 243 is never used
	// 244 is never used
	// 245 is never used
	kFlagWrongInvestigation                   = 246, // Re-purposed. Original: 246 is never used
	kFlagArrivedFromSpinner1                  = 247,
	kFlagSpinnerAtCT01                        = 248,
	kFlagSpinnerAtRC01                        = 249,
	kFlagSpinnerAtMA01                        = 250,
	kFlagSpinnerAtPS01                        = 251,
	kFlagSpinnerAtAR01                        = 252,
	kFlagSpinnerAtDR01                        = 253,
	kFlagSpinnerAtBB01                        = 254,
	kFlagSpinnerAtNR01                        = 255,
	kFlagSpinnerAtHF01                        = 256,
	kFlagMcCoyInHysteriaHall                  = 257,
	kFlagMcCoyInBradburyBuilding              = 258,
	kFlagMcCoyInUnderground                   = 259,
	// 260 is never used
	kFlagMcCoyInTyrellBuilding                = 261,
	kFlagBB01toBB02                           = 262,
	kFlagBB02toBB01                           = 263,
	kFlagBB01toDR02                           = 264,
	kFlagDR02toBB01                           = 265,
	kFlagDR05JustExploded                     = 266,
	kFlagDR03ChewTalk1                        = 267,
	kFlagDR06JesterActive                     = 268,
	kFlagDR05BombActivated                    = 269,
	kFlagDR05ChainShot                        = 270,
	kFlagDR05BombWillExplode                  = 271,
	kFlagDR05BombExploded                     = 272,
	kFlagArrivedFromSpinner2                  = 273,
	kFlagDR05MorajiTalk                       = 274,
	// 275 is never used
	kFlagNotUsed276                           = 276, // is never set
	// 277 is never used
	kFlagDR06KeyboardChecked                  = 278,
	// 279 is never used
	kFlagDR06VidphoneChecked                  = 280,
	kFlagBB03toBB02                           = 281,
	kFlagBB02toBB03                           = 282, // is never checked
	kFlagBB03toBB04                           = 283,
	kFlagBB04toBB03                           = 284,
	kFlagBB03toBB05                           = 285, // is never checked
	kFlagBB05toBB03                           = 286,
	kFlagRC04Entered                          = 287,
	// 288 is never used
	kFlagRC04McCoyShotBob                     = 289,
	kFlagRC04BobTalk1                         = 290,
	// 291 is never used
	kFlagRC04BobTalk2                         = 292,
	kFlagCT02PotTipped                        = 293,
	kFlagCT12GaffSpinner                      = 294,
	kFlagRC04BobShootMcCoy                    = 295,
	kFlagRC04McCoyCombatMode                  = 296,
	kFlagBB05toBB06                           = 297,
	kFlagBB06toBB05                           = 298,
	kFlagBB05toBB07                           = 299,
	kFlagBB07toBB05                           = 300,
	kFlagBB05toBB12                           = 301, // is never checked
	kFlagBB12toBB05                           = 302,
	kFlagRC04McCoyWarned                      = 303,
	kFlagCT11toCT09                           = 304, // is never checked
	kFlagRC04BobTalkAmmo                      = 305,
	kFlagNotUsed306                           = 306, // is never set
	kFlagSpinnerAtTB02                        = 307,
	kFlagHF01toHF02                           = 308,
	kFlagHF02toHF01                           = 309,
	kFlagHF01toHF03                           = 310,
	kFlagHF03toHF01                           = 311,
	kFlagHF01toHF05                           = 312,
	kFlagHF05toHF01                           = 313,
	kFlagUG01toUG02                           = 314,
	kFlagUG02toUG01                           = 315,
	kFlagUG01toUG10                           = 316,
	kFlagUG10toUG01                           = 317,
	kFlagUG02toHC03                           = 318,
	kFlagHC03toUG02                           = 319,
	kFlagAR01toAR02                           = 320,
	kFlagAR02toAR01                           = 321,
	kFlagAR01toHC01                           = 322,
	kFlagHC01toAR01                           = 323,
	kFlagUG01SteamOff                         = 324,
	kFlagCT04BodyDumped                       = 325,
	kFlagBB01Available                        = 326,
	// 327 is never used
	kFlagAR01FishDealerTalk                   = 328,
	kFlagAR02InsectDealerInterviewed          = 329,
	kFlagAR02HassanTalk                       = 330,
	kFlagAR02ScorpionsChecked                 = 331,
	kFlagBB02toBB04                           = 332,
	kFlagBB04toBB02                           = 333,
	kFlagUG03toUG04                           = 334, // is never checked
	kFlagUG04toUG03                           = 335,
	kFlagUG03toUG10                           = 336,
	kFlagUG10toUG03                           = 337,
	kFlagUG04toUG05                           = 338, // is never checked
	kFlagUG05toUG04                           = 339,
	kFlagUG04toUG06                           = 340,
	kFlagUG06toUG04                           = 341,
	kFlagUG06toNR01                           = 342,
	kFlagNR01toUG06                           = 343,
	kFlagUG12toUG14                           = 344,
	kFlagUG14toUG12                           = 345, // is never checked
	kFlagUG14toUG10                           = 346,
	kFlagUG10toUG14                           = 347, // is never checked
	kFlagUG14toUG19                           = 348,
	kFlagUG19toUG14                           = 349,
	kFlagUG15toUG13                           = 350,
	kFlagUG13toUG15                           = 351,
	kFlagUG15toUG17                           = 352,
	kFlagUG17toUG15                           = 353,
	kFlagUG15toUG16b                          = 354, // is never checked
	kFlagUG16toUG15b                          = 355,
	kFlagUG19toMA07                           = 356,
	kFlagMA07toUG19                           = 357,
	kFlagHF07toHF05                           = 358,
	kFlagHF05toHF07                           = 359,
	kFlagHF07toUG06                           = 360,
	kFlagUG06toHF07                           = 361,
	kFlagBB07toBB06                           = 362,
	kFlagBB06toBB07                           = 363,
	kFlagBB07toBB12                           = 364,
	kFlagBB12toBB07                           = 365,
	kFlagAR02InsectDealerTalk                 = 366,
	kFlagAR02StungByScorpion                  = 367,
	kFlagHF07Hole                             = 368,
	kFlagHF05Hole                             = 369,
	kFlagNotUsed370                           = 370, // is never checked
	// 371 is never used
	// 372 is never used
	kFlagKP02Available                        = 373,
	kFlagAR02DektoraBoughtScorpions           = 374,
	kFlagBB11SadikPunchedMcCoy                = 375,
	// 376 is never used
	kFlagHF01MurrayMiaIntro                   = 377,
	kFlagDirectorsCut                         = 378, // Designer's Cut (KIA options) flag!
	kFlagCT08toCT51                           = 379, // is never checked
	kFlagCT51toCT08                           = 380,
	// 381 is never used
	kFlagHF01MurrayMiaTalk                    = 382,
	kFlagNotUsed383                           = 383,
	kFlagHC01toHC02                           = 384,
	kFlagHC02toHC01                           = 385,
	kFlagHC01toHC03                           = 386, // is never checked
	kFlagHC03toHC01                           = 387,
	kFlagHC03TrapDoorOpen                     = 388,
	// 389 is never used
	// 390 is never used
	kFlagBB09SadikRun                         = 391,
	kFlagRC01McCoyAndOfficerLearyTalking      = 392,
	kFlagBB06toBB51                           = 393, // is never checked
	kFlagBB51toBB06a                          = 394,
	kFlagBB51toBB06b                          = 395,
	kFlagBB07ElectricityOn                    = 396,
	kFlagRC01McCoyAndOfficerLearyArtMetaphor  = 397,
	kFlagBB07PrinterChecked                   = 398,
	kFlagGeneralDollShot                      = 399,
	kFlagHC01IzoTalk1                         = 400,
	kFlagHC01IzoTalk2                         = 401,
	kFlagHC03Available                        = 402,
	kFlagHC03CageOpen                         = 403,
	kFlagHC02HawkersBarkeepIntroduction       = 404,
	kFlagHC02HawkersBarkeepBraceletTalk       = 405,
	kFlagUnused406                            = 406, // is never checked
	kFlagUnused407                            = 407, // is never checked
	kFlagMcCoyCommentsOnStatues               = 408, // Re-purposed. Original: 408 is never used
	kFlagMA01GaffApproachMcCoy                = 409,
	kFlagBB06AndroidDestroyed                 = 410,
	kFlagKP02toUG12                           = 411,
	kFlagUG12toKP02                           = 412, // is never checked
	kFlagKP02toKP01                           = 413, // is never checked
	kFlagKP01toKP02                           = 414,
	kFlagKP01toKP04                           = 415, // is never checked
	kFlagKP04toKP01                           = 416,
	kFlagKP01toKP03                           = 417,
	kFlagKP03toKP01                           = 418,
	kFlagKP03toKP05                           = 419,
	kFlagKP05toKP03                           = 420,
	kFlagKP03BombActive                       = 421,
	kFlagKP03BombExploded                     = 422,
	kFlagUG07toUG10                           = 423,
	kFlagUG10toUG07                           = 424, // is never checked
	kFlagUG07toUG08                           = 425, // is never checked
	kFlagUG08toUG07                           = 426,
	kFlagUG07toUG09                           = 427, // is never checked
	kFlagUG09ToUG07                           = 428,
	kFlagUG08toUG13                           = 429,
	kFlagUG13toUG08                           = 430,
	kFlagUG08ElevatorUp                       = 431,
	kFlagUG09toCT12                           = 432,
	kFlagCT12ToUG09                           = 433,
	kFlagUG13toUG18                           = 434, // is never checked
	kFlagUG18toUG13                           = 435,
	kFlagUG13CallElevator                     = 436,
	kFlagNR04toNR03                           = 437,
	kFlagNR03toNR04                           = 438,
	kFlagNR06toNR08                           = 439,
	kFlagNR08toNR06                           = 440, // is never checked
	kFlagNR06toNR07                           = 441,
	kFlagNR07toNR06                           = 442,
	kFlagGenericWalkerWaiting                 = 443,
	kFlagIzoShot                              = 444,
	// 445 is never used
	// 446 is never used
	kFlagTB03toUG17                           = 447,
	kFlagUG17toTB03                           = 448,
	kFlagUG02WeaponsChecked                   = 449,
	kFlagTB02ElevatorToTB05                   = 450,
	kFlagTB05Entered                          = 451,
	// 452 is never used
	kFlagTB02GuardTalk1                       = 453,
	// 454 is never used
	kFlagTB02GuardTalk2                       = 455,
	kFlagTB02SteeleTalk                       = 456,
	kFlagTB02SteeleEnter                      = 457,
	// 458 is never used
	// 459 is never used
	kFlagSteeleWalkingAround                  = 460,
	kFlagMaggieHasBomb                        = 461,
	kFlagGuzzaIsMovingAround                  = 462,
	kFlagHC01GuzzaWalk                        = 463,
	kFlagHC01GuzzaPrepare                     = 464,
	kFlagMcCoyArrested                        = 465,
	kFlagBB10Shelf1Available                  = 466,
	kFlagBB10Shelf2Available                  = 467,
	kFlagBB10Shelf3Available                  = 468,
	kFlagBB10Shelf4Available                  = 469,
	kFlagBB10Shelf5Available                  = 470,
	kFlagGaffChapter2Started                  = 471,
	// 472 is never used
	// 473 is never used
	kFlagUG10GateOpen                         = 474,
	kFlagNR09toNR10                           = 475,
	kFlagNR10toNR09                           = 476,
	kFlagNR11toNR10                           = 477, // is never checked
	kFlagMcCoyAtPS03                          = 478, // has no use
	kFlagMcCoyInHawkersCircle                 = 479,
	// 480 is never used
	// 481 is never used
	kFlagSteeleSmoking                        = 482,
	kFlagTB06Introduction                     = 483,
	kFlagKP03BombDisarmed                     = 484,
	kFlagTB06Photographer                     = 485,
	kFlagDNARowAvailable                      = 486,
	kFlagKIAPrivacyAddon                      = 487,
	// 488 is never used
	kFlagDektoraChapter2Started               = 489,
	kFlagEarlyQStartedChapter1                = 490,
	kFlagEarlyQStartedChapter2                = 491,
	kFlagCT04HomelessTrashFinish              = 492,
	kFlagBB02ElevatorDown                     = 493,
	kFlagBB02Entered                          = 494,
	kFlagBB05Entered                          = 495,
	kFlagBB08Entered                          = 496,
	kFlagBB12Entered                          = 497,
	kFlagUG02FromUG01                         = 498,
	kFlagUG02Interactive                      = 499,
	// 500 is never used
	// 501 is never used
	// 502 is never used
	// 503 is never used
	kFlagAR02DektoraWillBuyScorpions          = 504,
	kFlagDR03ChewTalkExplosion                = 505,
	kFlagBB08toBB12                           = 506,
	kFlagBB12toBB08                           = 507, // is never checked
	kFlagNR07Entered                          = 508,
	kFlagBB11SadikFight                       = 509,
	kFlagNotUsed510                           = 510, // is never set
	kFlagDR05ExplodedEntered                  = 511,
	// 512 is never used
	// 513 is never used
	// 514 is never used
	kFlagDR05ViewExplosion                    = 515,
	kFlagCT07ZubenAttack                      = 516,
	// 517 is never used
	// 518 is never used
	kFlagTB06DogCollarTaken                   = 519,
	kFlagTB06KitchenBoxTaken                  = 520,
	kFlagHC03TrapDoorOpened                   = 521,
	kFlagUG05TalkAboutTunnel                  = 522,
	kFlagMutantsActive                        = 523,
	kFlagUG06Chapter4Started                  = 524,
	kFlagCT10Entered                          = 525,
	// 526 is never used
	kFlagHF02toHF03                           = 527,
	kFlagHF03toHF02                           = 528,
	kFlagHF05toHF06                           = 529, // is never checked
	kFlagHF06toHF05                           = 530,
	kFlagCT11toDR01                           = 531,
	kFlagNR01toNR02                           = 532,
	kFlagNR02toNR01                           = 533,
	kFlagNR03toNR01                           = 534,
	kFlagNR01toNR03                           = 535,
	kFlagNR03toNR05                           = 536,
	kFlagNR05toNR03                           = 537,
	kFlagCT09Entered                          = 538,
	kFlagCT09LeonInterrupted                  = 539,
	kFlagCT09DeskClerkTalk                    = 540,
	// 541 is never used
	kFlagClovisChapter4Started                = 542,
	kFlagGordoTalk1                           = 543,
	kFlagGordoTalk2                           = 544,
	kFlagNotUsed545                           = 545, // is never set
	kFlagNR05toNR08                           = 546,
	kFlagNR08toNR05                           = 547,
	kFlagDR06MannequinHeadOpen                = 548,
	kFlagTB03Entered                          = 549,
	kFlagMcCoyTiedDown                        = 550,
	kFlagUG16toDR06                           = 551, // is never checked
	kFlagDR06toUG16                           = 552,
	kFlagUG13Entered                          = 553,
	kFlagUG13HomelessTalk1                    = 554,
	// 555 is never used
	kFlagUG16LutherLanceTalk1                 = 556,
	kFlagNotUsed557                           = 557, // is never checked
	kFlagDR01toCT11                           = 558,
	kFlagHF06SteelInterruption                = 559,
	kFlagLutherLanceIsReplicant               = 560,
	kFlagNR02GordoLeaveLighter                = 561,
	kFlagHF05CrazyLegsTalk1                   = 562,
	kFlagHF05CrazyLegsTalk2                   = 563,
	kFlagEarlyQStartedChapter3                = 564,
	kFlagNotUsed565                           = 565, // has no use
	kFlagHF03toHF04                           = 566, // is never checked
	kFlagHF04toHF03                           = 567,
	kFlagUG16ComputerOff                      = 568,
	kFlagNR04EarlyQWalkedIn                   = 569,
	// 570 is never used
	// 571 is never used
	// 572 is never used
	kFlagNR03Entered                          = 573,
	kFlagKP04toKP05                           = 574,
	kFlagKP05toKP04                           = 575,
	kFlagKP05toKP06                           = 576,
	kFlagKP06toKP05                           = 577, // has no use
	kFlagKP06toKP07                           = 578,
	kFlagKP07toKP06                           = 579,
	// 580 is never used
	// 581 is never used
	kFlagKP07BusActive                        = 582, // is never set
	// 583 is never used
	kFlagHF04DoorsClosed                      = 584,
	kFlagHF04CloseDoors                       = 585,
	kFlagHF04OpenDoors                        = 586,
	kFlagUG15LutherLanceStartedDying          = 587,
	kFlagNR05BartenderTalk1                   = 588,
	kFlagNR05BartenderTalk2                   = 589,
	kFlagNR05EarlyQTalk                       = 590,
	kFlagDektoraRanAway                       = 591,
	kFlagGordoRanAway                         = 592,
	kFlagLucyRanAway                          = 593,
	kFlagNR02GordoJumpDown                    = 594,
	kFlagLutherLanceAreDead                   = 595,
	kFlagUG16PulledGun                        = 596,
	kFlagUG16FolderFound                      = 597,
	kFlagUG07ClovisCaughtMcCoy                = 598,
	kFlagKIAPrivacyAddonIntro                 = 599,
	kFlagUG16LutherLanceTalkReplicants1       = 600,
	kFlagUG16LutherLanceTalkReplicants2       = 601,
	// 602 is never used
	kFlagSteeleAimingAtGordo                  = 603,
	kFlagNR03McCoyThrownOut                   = 604,
	kFlagNR04DiscFound                        = 605,
	kFlagNR04EarlyQStungByScorpions           = 606,
	kFlagMcCoyRetiredHuman                    = 607,
	kFlagTB07toTB02                           = 608,
	kFlagNR04McCoyAimedAtEarlyQ               = 609,
	kFlagUG08Entered                          = 610,
	kFlagNR03HanoiTalk                        = 611,
	kFlagTB07RachaelTalk                      = 612,
	kFlagHF03LucyTalk                         = 613,
	kFlagNR08toNR09                           = 614,
	kFlagNR09toNR08                           = 615,
	kFlagMcCoyCapturedByHolloway              = 616,
	kFlagSpinnerMissing                       = 617,
	// 618 is never used
	// 619 is never used
	kFlagNR08Available                        = 620,
	// 621 is never used
	kFlagNR08TouchedDektora                   = 622,
	kFlagUG07Empty                            = 623,
	// 624 is never used
	kFlagTB07TyrellMeeting                    = 625,
	// 626 is never used
	kFlagNR01McCoyIsDrugged                   = 627,
	// 628 is never used
	kFlagUnpauseGenWalkers                    = 629,
	kFlagMutantsPaused                        = 630,
	kFlagRatWalkingAround                     = 631,
	kFlagNR01DektoraFall                      = 632,
	kFlagNR11DektoraBurning                   = 633,
	kFlagNR11BurnedUp                         = 634,
	kFlagNR11SteeleShoot                      = 635,
	kFlagNR08Faded                            = 636,
	kFlagMcCoyCommentsOnOldProjector          = 637, // Re-purposed. Original: 637 is never used
	kFlagNR07McCoyIsCop                       = 638,
	kFlagMcCoyCommentsOnEyeOfEyeworks         = 639, // Re-purposed. Original: 639 is never used
	kFlagNR10CameraDestroyed                  = 640,
	kFlagNR10toNR11                           = 641, // is never checked
	kFlagNR10McCoyBlinded                     = 642,
	kFlagSteeleKnowsBulletBobIsDead           = 643,
	kFlagNotUsed644                           = 644, // is never checked
	kFlagCT11DogWrapperTaken                  = 645,
	kFlagSteeleDead                           = 646, // is never checked
	kFlagMA04McCoySleeping                    = 647,
	kFlagMA07GaffTalk                         = 648,
	kFlagMA04PhoneMessageFromClovis           = 649,
	kFlagMA04PhoneMessageFromLucy             = 650,
	kFlagNR08McCoyWatchingShow                = 651,
	kFlagCrazylegsArrestedTalk                = 652,
	kFlagMcCoyIsHelpingReplicants             = 653,
	kFlagMA02Chapter5Started                  = 654,
	kFlagMA02RajifTalk                        = 655,
	kFlagUG02RagiationGooglesTaken            = 656,
	kFlagNotUsed657                           = 657, // is never set
	kFlagKP07Entered                          = 658,
	kFlagNR11BreakWindow                      = 659,
	kFlagDNARowAvailableTalk                  = 660,
	kFlagTB07ShadeDown                        = 661, // is never set
	kFlagHF01TalkToLovedOne                   = 662,
	kFlagHF05PoliceArrived                    = 663,
	kFlagUG07PoliceLeave                      = 664,
	kFlagUG19Available                        = 665,
	kFlagMcCoyFreedOfAccusations              = 666,
	// 667 is never used
	// 668 is never used
	kFlagKP02DispatchOnToxicKipple            = 669, // Re-purposed. Original: 669 is never used
	kFlagCallWithGuzza                        = 670,
	kFlagUG18GuzzaScene                       = 671,
	kFlagMA07toPS14                           = 672,
	kFlagPS14toMA07                           = 673,
	kFlagKP01Entered                          = 674,
	kFlagUG04DispatchOnHoodooRats             = 675, // Re-purposed. Original: 675 is never used
	kFlagUG15RatShot                          = 676,
	kFlagUG15BridgeWillBreak                  = 677,
	kFlagChapter2Intro                        = 678,
	kFlagChapter3Intro                        = 679,
	kFlagChapter4Intro                        = 680,
	kFlagMcCoyCommentsOnHoodooRats            = 681, // Re-purposed. Original: 681 is never used
	kFlagUG15BridgeBroken                     = 682,
	kFlagUG05TalkToPolice                     = 683,
	kFlagHF05PoliceAttacked                   = 684,
	kFlagClovisLyingDown                      = 685,
	kFlagNotUsed686                           = 686,
	kFlagMcCoyCommentsOnFans                  = 687, // Re-purposed. Original: 687 is never used
	kFlagPS05TV0                              = 688,
	kFlagPS05TV1                              = 689,
	kFlagPS05TV2                              = 690,
	kFlagPS05TV3                              = 691,
	kFlagPS05TV4                              = 692,
	kFlagUG03DeadHomeless                     = 693,
	kFlagUG14DeadHomeless                     = 694,
	kFlagNR01VisitedFirstTimeWithSpinner      = 695, // Re-purposed. Original: 695 is never used
	kFlagKP07ReplicantsAttackMcCoy            = 696,
	kFlagKP07McCoyPulledGun                   = 697,
	kFlagUG15LanceLuthorTrade                 = 698,
	kFlagTalkToZuben                          = 699,
	kFlagBB01VisitedFirstTimeWithSpinner      = 700, // Re-purposed. Original: 700 is never used
	kFlagMcCoyAttackedLucy                    = 701,
	kFlagBulletBobDead                        = 702,
	kFlagUG18BriefcaseTaken                   = 703,
	kFlagRC02EnteredChapter4                  = 704,
	kFlagRC02RunciterTalkWithGun              = 705,
	kFlagRC02RunciterTalk2                    = 706,
	kFlagTB06PhotographTalk1                  = 707,
	kFlagUG02AmmoTaken                        = 708,
	kFlagRC51Discovered                       = 709,
	kFlagAR01Entered                          = 710,
	kFlagMA04WatchedTV                        = 711,
	kFlagMcCoyShotAtZuben                     = 712,
	kFlagDR04McCoyShotMoraji                  = 713,
	kFlagMcCoyAttackedReplicants              = 714,
	kFlagDR06UnlockedToUG16                   = 715,
	kFlagUG13HomelessLayingdown               = 716,
	kFlagRC04BobTalk3                         = 717,
	kflagPS01toPS02                           = 718,
	kFlagCT02McCoyFell                        = 719,
	kFlagCT02McCoyCombatReady                 = 720,
	kFlagCT02McCoyShouldCommentOnDumpedSoup   = 721, // Now: used for cut content for McCoy's monologue if he fails to dodge the soup, Original: was set but not used
	kFlagChapter4Starting                     = 722,
	kFlagZubenBountyPaid                      = 723,
	kFlagUG15BridgeBreaks                     = 724,
	kFlagCT11DogWrapperAvailable              = 725,
	kFlagAR02Entered                          = 726, // Now: never checked (superseded by new kFlagScorpionsInAR02). Original: used to track if McCoy visited AR02 in order to remove scorpion's cage item if Dektora buys it
	kFlagPS04WeaponsOrderForm                 = 727,
	kFlagDR01Visited                          = 728, // Re-purposed. Original: 728 is never used
	kFlagNR08DektoraShow                      = 729
};

enum Variables {
	kVariableChapter                       =  1,
	kVariableChinyen                       =  2,
	kVariableGameVersion                   =  3, // Re-purposed. Original: not used, so it would have value of 0 in the classic version
	kVariableUG18CountUpForNextTrainAction =  4, // Re-purposed. Original: not used
	kVariableUG18StateOfTrains             =  5, // Re-purposed. Original: not used
	kVariableUG18StateOfGuzzaCorpse        =  6, // Re-purposed. Original: not used
	// variables 7 - 8 are not used
	kVariablePoliceMazeScore               =  9,
	kVariablePoliceMazePS10TargetCounter   = 10,
	kVariablePoliceMazePS11TargetCounter   = 11,
	kVariablePoliceMazePS12TargetCounter   = 12,
	kVariablePoliceMazePS13TargetCounter   = 13,
	kVariableMcCoyEvidenceMissed           = 14,
	// variable 15 has no use
	kVariableFishDealerBanterTalk          = 16, // Re-purposed. Original: not used
	kVariableInsectDealerBanterTalk        = 17, // Re-purposed. Original: not used
	kVariableHasanBanterTalk               = 18, // Re-purposed. Original: not used
	kVariableIzoShot                       = 19, // has no use
	// variables 20 - 23 are not used
	kVariableBobShot                       = 24, // has no use
	kVariableGeneralDollShot               = 25, // has no use
	// variables 26 - 28 are not used
	kVariableLutherLanceShot               = 29,
	// variables 30 - 31 are not used
	kVariableGenericWalkerAModel           = 32, // values: 0 1 2 3 4 5 (6 7 8 9 for restored content)
	kVariableGenericWalkerBModel           = 33, // values: 0 1 2 3 4 5 (6 7 8 9 for restored content)
	kVariableGenericWalkerCModel           = 34, // values: 0 1 2 3 4 5 (6 7 8 9 for restored content)
	kVariableGenericWalkerConfig           = 35, // Re-purposed. Original: has no real use
	kVariableBB10ShelvesAvailable          = 36,
	kVariableWalkLoopActor                 = 37,
	kVariableWalkLoopRun                   = 38,
	kVariableDR06MannequinHeadOpened       = 39,
	kVariableHollowayArrest                = 40, // values: 1 dektora, 2 gordo, 3 lucy
	kVariableGordosJoke                    = 41,
	kVariableMcCoyDrinks                   = 42, // is never checked
	kVariableHanoiNR05Warnings             = 43,
	kVariableHanoiNR04Warnings             = 44,
	kVariableAffectionTowards              = 45, // values: 0 none, 1 steele, 2 dektora, 3 lucy
	kVariableGunPulledInFrontOfSebastian   = 46,
	kVariableNR01GetUpCounter              = 47,
	kVariableDNAEvidence                   = 48,
	kVariableCorruptedGuzzaEvidence        = 49,
	kVariableTaffyLewisMusic               = 50,
	kVariableReplicantsSurvivorsAtMoonbus  = 51,
	kVariableNextTvNews                    = 52,
	kVariableEarlyQFrontMusic              = 53,
	kVariableEarlyQBackMusic               = 54
};

enum Outtakes {
	kOuttakeIntro             =  0,
	kOuttakeMovieA            =  1,
	kOuttakeMovieB1           =  2,
	kOuttakeMovieB2           =  3,
	kOuttakeMovieB3           =  4,
	kOuttakeMovieB4           =  5,
	kOuttakeMovieB5           =  6,
	kOuttakeMovieC1           =  7,
	kOuttakeMovieC2           =  8,
	kOuttakeMovieC3           =  9,
	kOuttakeMovieD            = 10,
	kOuttakeInterrogation     = 11,
	kOuttakeEnd1A             = 12,
	kOuttakeEnd1B             = 13,
	kOuttakeEnd1C             = 14,
	kOuttakeEnd1D             = 15,
	kOuttakeEnd1E             = 16,
	kOuttakeEnd1F             = 17,
	kOuttakeEnd2              = 18,
	kOuttakeEnd3              = 19,
	kOuttakeEnd4A             = 20,
	kOuttakeEnd4B             = 21,
	kOuttakeEnd4C             = 22,
	kOuttakeEnd4D             = 23,
	kOuttakeEnd5              = 24,
	kOuttakeEnd6              = 25,
	kOuttakeEnd7              = 26,
	kOuttakeTyrellBuildingFly = 27,
	kOuttakeWestwood          = 28,
	kOuttakeFlyThrough        = 29, // Act 1 - Original: unused - has no sound
	kOuttakeAway1             = 30, // Act 2, 3
	kOuttakeAway2             = 31, // Act 1
	kOuttakeAscent            = 32, // Act 1, 4, 5 - Original: unused
	kOuttakeDescent           = 33, // Act 1, 4, 5
	kOuttakeInside1           = 34, // Act 1
	kOuttakeInside2           = 35, // Act 1, 2, 3
	kOuttakeTowards1          = 36, // Act 1
	kOuttakeTowards2          = 37, // Act 1
	kOuttakeTowards3          = 38, // Act 1, 2, 3
	kOuttakeRachel            = 39,
	kOuttakeDektora           = 40,
	kOuttakeBladeRunner       = 41
};

enum AnimationModes {
	kAnimationModeIdle            =  0,
	kAnimationModeWalk            =  1,
	kAnimationModeRun             =  2,
	kAnimationModeTalk            =  3,
	kAnimationModeCombatIdle      =  4,
	kAnimationModeCombatAim       =  5,
	kAnimationModeCombatAttack    =  6,
	kAnimationModeCombatWalk      =  7,
	kAnimationModeCombatRun       =  8,
	// 9 - 19 various talk modes
	kAnimationModeDodge           = 20,
	kAnimationModeHit             = 21,
	kAnimationModeCombatHit       = 22,
	// 23 give / take away
	// 24 - 25 not used
	// 26 ???
	// 27 - 28 not used
	// 29 stand up
	// 30 - 37 not used
	// 38 ???
	// 39 not used
	// 40 McCoy throws body
	kAnimationModeSpinnerGetIn    = 41,
	kAnimationModeSpinnerGetOut   = 42,
	// 43 - taking photo/using cellphone
	kAnimationModeWalkUp          = 44,
	kAnimationModeWalkDown        = 45,
	kAnimationModeCombatWalkUp    = 46,
	kAnimationModeCombatWalkDown  = 47,
	kAnimationModeDie             = 48,
	kAnimationModeCombatDie       = 49,
	// 50 Luther, Tyrell Guard ???
	// 51 Steele, Maggie ???
	kAnimationModeFeeding         = 52,
	kAnimationModeSit             = 53, // more like sitting than sitting down
	// 54 Clovis, Maggie ???
	// 55 transient, Tyrell Guard, Maggie
	// 56 Maggie ???
	// 57 Maggie ???
	// 58 - 60 not used
	// 61 Guzza ???
	// 62 Sadik ???
	// 63 Sadik ???
	kAnimationModeClimbUp         = 64,
	kAnimationModeClimbDown       = 65,
	kAnimationModeCombatClimbUp   = 66,
	kAnimationModeCombatClimbDown = 67
	// 68 McCoy getting kicked
	// 69 not used
	// 70 Dektora ???
	// 71 Dektora ???
	// 72 Desk Clerk, Leon ???
	// 73 EarlyQ ???
	// 74 EarlyQ ???
	// 75 McCoy drinking ???
	// 76 EarlyQ ???
	// 77 not used
	// 78 Hanoi ???
	// 79 Dektora ???
	// 80 Gordo ???
	// 81 - 82 not used
	// 83 Gordo ???
	// 84 Gordo ???
	// 85 McCoy, EarlyQ - sitdown
	// 86 - 87 not used
	// 88 ???
	// 89 Homeless ???
};

enum SceneLoopMode {
	kSceneLoopModeNone        = -1,
	kSceneLoopModeLoseControl =  0,
	kSceneLoopModeChangeSet   =  1,
	kSceneLoopModeOnce        =  2,
	kSceneLoopModeSpinner     =  3
};

enum Scenes {
	kSceneAR01 =   0, // Animoid Row - Fish dealer
	kSceneAR02 =   1, // Animoid Row - Insect dealer
	kSceneBB01 =   2, // Bradbury building - Outside
	kSceneBB02 =   3, // Bradbury building - Inside
	kSceneBB03 =   4, // Bradbury building - Entrance
	kSceneBB04 =   5, // Bradbury building - Elevator
	kSceneBB05 =   6, // Bradbury building - Entry hall
	kSceneBB06 =   7, // Bradbury building - Billiard room - front
	kSceneBB07 =   8, // Bradbury building - Sebastian's office
	kSceneBB08 =   9, // Bradbury Building - Bathroom
	kSceneBB09 =  10, // Bradbury Building - Above bathroom
	kSceneBB10 =  11, // Bradbury Building - Bookcase
	kSceneBB11 =  12, // Bradbury Building - Roof
	kSceneCT01 =  13, // Chinatown - Howie Lee's Restaurant
	kSceneCT02 =  14, // Chinatown - Restaurant - Kitchen
	kSceneCT03 =  15, // Chinatown - Back alley - Restaurant
	kSceneCT04 =  16, // Chinatown - Back alley - Dumpster
	kSceneCT05 =  17, // Chinatown - Warehouse - Inside
	kSceneCT06 =  18, // Chinatown - Passage - front
	kSceneCT07 =  19, // Chinatown - Passage - back
	kSceneCT08 =  20, // Chinatown - Yukon Hotel - Backroom
	kSceneCT09 =  21, // Chinatown - Yukon Hotel - Lobby
	kSceneCT10 =  22, // Chinatown - Yukon Hotel - Room
	kSceneCT11 =  23, // Chinatown - Yukon Hotel - Outside
	kSceneCT12 =  24, // Chinatown - Warehouse - Outside
	kSceneDR01 =  25, // DNA Row - Street
	kSceneDR02 =  26, // DNA Row - Eyeworld - Outside
	kSceneDR03 =  27, // DNA Row - Eyeworld - Inside
	kSceneDR04 =  28, // DNA Row - Dermo design - Outside
	kSceneDR05 =  29, // DNA Row - Dermo design - Inside
	kSceneDR06 =  30, // DNA Row - Twin's office
	kSceneHC01 =  31, // Hawker's Circle - Green pawn - Outside
	kSceneHC02 =  32, // Hawker's Circle - China bar
	kSceneHC03 =  33, // Hawker's Circle - Green pawn - Inside
	kSceneHF01 =  34, // Hysteria Hall - Outside
	kSceneHF02 =  35, // Hysteria Hall - Hall of Mirrors - Exit
	kSceneHF03 =  36, // Hysteria Hall - Hysteria Arcade
	kSceneHF04 =  37, // Hysteria Hall - Hall of Mirrors
	kSceneHF05 =  38, // Hysteria Hall - Crazy Legs Larry Autos - Inside
	kSceneHF06 =  39, // Hysteria Hall - Crazy Legs Larry Autos - Root
	kSceneHF07 =  40, // Hysteria Hall - Crazy Legs Larry Autos - Underground
	kSceneKP01 =  41, // Kipple - Rubble
	kSceneKP02 =  42, // Kipple - Entry / Gate
	kSceneKP03 =  43, // Kipple - Bomb
	kSceneKP04 =  44, // Kipple - Huge pillar
	kSceneKP05 =  45, // Kipple - Moonbus - Far
	kSceneKP06 =  46, // Kipple - Moonbus - Near
	kSceneKP07 =  47, // Kipple - Moonbus - Inside
	kSceneMA01 =  48, // McCoy's Apartment - Roof
	kSceneMA02 =  49, // McCoy's Apartment - Living room
	kSceneMA04 =  50, // McCoy's Apartment - Sleeping room
	kSceneMA05 =  51, // McCoy's Apartment - Balcony
	kSceneMA06 =  52, // McCoy's Apartment - Elevator
	kSceneMA07 =  53, // McCoy's Apartment - Ground floor
	kSceneNR01 =  54, // Nightclub Row - Outside
	kSceneNR02 =  55, // Nightclub Row - Taffy Lewis'
	kSceneNR03 =  56, // Nightclub Row - Early Q's - Main area
	kSceneNR04 =  57, // Nightclub Row - Early Q's - Office
	kSceneNR05 =  58, // Nightclub Row - Early Q's - VIP area
	kSceneNR06 =  59, // Nightclub Row - Early Q's - Empty dressing room
	kSceneNR07 =  60, // Nightclub Row - Early Q's - Dektora's dressing room
	kSceneNR08 =  61, // Nightclub Row - Early Q's - Stage
	kSceneNR09 =  62, // Nightclub Row - Early Q's - Balcony
	kSceneNR10 =  63, // Nightclub Row - Early Q's - Projector room
	kSceneNR11 =  64, // Nightclub Row - Early Q's - Loft
	kScenePS01 =  65, // Police Station - Roof
	kScenePS02 =  66, // Police Station - Elevator
	kScenePS03 =  67, // Police Station - Ground floor
	kScenePS04 =  68, // Police Station - Guzza's Office
	kScenePS05 =  69, // Police Station - Mainframe & Shooting range hallway
	kScenePS06 =  70, // Police Station - ESPER room
	kScenePS07 =  71, // Police Station - Laboratory
	kScenePS09 =  72, // Police Station - Lockup
	kScenePS10 =  73, // Police Station - Shooting range 1
	kScenePS11 =  74, // Police Station - Shooting range 2
	kScenePS12 =  75, // Police Station - Shooting range 3
	kScenePS13 =  76, // Police Station - Shooting range 4
	kScenePS14 =  77, // Police Station - Outside
	kSceneRC01 =  78, // Runciter - Outside
	kSceneRC02 =  79, // Runciter - Inside
	kSceneRC03 =  80, // Bullet Bob's Runner Surplus - Outside
	kSceneRC04 =  81, // Bullet Bob's Runner Surplus - Inside
	kSceneTB02 =  82, // Tyrell Building - Reception
	kSceneTB03 =  83, // Tyrell Building - Reception back
	kSceneTB05 =  84, // Tyrell Building - Grav Test Lab - Outside
	kSceneTB06 =  85, // Tyrell Building - Grav Test Lab - Inside
	kSceneUG01 =  86, // Underground - Under RC03
	kSceneUG02 =  87, // Underground - Under HC03
	kSceneUG03 =  88, // Underground - Chair
	kSceneUG04 =  89, // Underground - Rails with crash - start
	kSceneUG05 =  90, // Underground - Under HF07 - Rails with cars
	kSceneUG06 =  91, // Underground - Under NR01 - Metro entrance
	kSceneUG07 =  92, // Underground - Pipe
	kSceneUG08 =  93, // Underground - Elevator
	kSceneUG09 =  94, // Underground - Behind CT12
	kSceneUG10 =  95, // Underground - Moving bridge
	kSceneUG12 =  96, // Underground - Gate
	kSceneUG13 =  97, // Underground - Homeless' living room
	kSceneUG14 =  98, // Underground - Crossroad
	kSceneUG15 =  99, // Underground - Bridge with rat
	kSceneUG16 = 100, // Underground - Under DR06
	kSceneUG17 = 101, // Underground - Under TB03
	kSceneUG18 = 102, // Underground - Pit
	kSceneUG19 = 103, // Underground - Under MA07
	kSceneBB51 = 104, // Bradbury Building - Billiard room - Back
	kSceneCT51 = 105, // Chinatown - Yukon Hotel - Backroom - back
	kSceneHC04 = 106, // Hawker's Circle - Kingston kitchen
	kSceneRC51 = 107, // Runciter - Lucy's desk
	kSceneTB07 = 108, // Tyrell Building - Tyrell's office
	kScenePS15 = 119, // Police Station - Armory
	kSceneBB12 = 120  // Bradbury Building - Monkey room
};

enum Sets {
	kSetAR01_AR02           =   0,
	kSetBB02_BB04_BB06_BB51 =   1,
	kSetBB06_BB07           =   2, //BB06
	kSetBB07                =   3,
	kSetCT01_CT12           =   4,
	kSetCT03_CT04           =   5,
	kSetCT08_CT51_UG12      =   6, //UG12
	kSetDR01_DR02_DR04      =   7,
	kSetHC01_HC02_HC03_HC04 =   8,
	kSetKP05_KP06           =   9,
	kSetMA02_MA04           =  10, //MA04
	kSetNR02                =  11,
	kSetNR04                =  12,
	kSetNR05_NR08           =  13,
	kSetPS10_PS11_PS12_PS13 =  14,
	kSetPS05                =  15,
	kSetRC02_RC51           =  16,
	kSetTB02_TB03           =  17,
	kSetTB07                =  18,
	kSetUG16                =  19,
	kSetBB01                =  20,
	kSetBB03                =  21,
	kSetBB05                =  22,
	kSetBB08                =  23,
	kSetBB09                =  24,
	kSetBB10                =  25,
	kSetBB11                =  26,
	kSetCT02                =  27,
	kSetCT05                =  28,
	kSetCT06                =  29,
	kSetCT07                =  30,
	kSetCT09                =  31,
	kSetCT10                =  32,
	kSetCT11                =  33,
	kSetDR03                =  34,
	kSetDR05                =  35,
	kSetDR06                =  36,
	kSetHF01                =  37,
	kSetHF02                =  38,
	kSetHF03                =  39,
	kSetHF04                =  40,
	kSetHF05                =  41,
	kSetHF06                =  42,
	kSetHF07                =  43,
	kSetKP01                =  44,
	kSetKP02                =  45,
	kSetKP03                =  46,
	kSetKP04                =  47,
	kSetKP07                =  48,
	kSetMA01                =  49,
	kSetMA04                =  50,
	kSetMA05                =  51,
	kSetMA06                =  52,
	kSetMA07                =  53,
	kSetNR01                =  54,
	kSetNR03                =  55,
	kSetNR06                =  56,
	kSetNR07                =  57,
	kSetNR09                =  58,
	kSetNR10                =  59,
	kSetNR11                =  60,
	kSetPS01                =  61,
	kSetPS02                =  62,
	kSetPS03                =  63,
	kSetPS04                =  64,
	kSetPS06                =  65,
	kSetPS07                =  66,
	kSetPS09                =  67,
	kSetPS14                =  68,
	kSetRC01                =  69,
	kSetRC03                =  70,
	kSetRC04                =  71,
	kSetTB05                =  72,
	kSetTB06                =  73,
	kSetUG01                =  74,
	kSetUG02                =  75,
	kSetUG03                =  76,
	kSetUG04                =  77,
	kSetUG05                =  78,
	kSetUG06                =  79,
	kSetUG07                =  80,
	kSetUG08                =  81,
	kSetUG09                =  82,
	kSetUG10                =  83,
	kSetUG12                =  84,
	kSetUG13                =  85,
	kSetUG14                =  86,
	kSetUG15                =  87,
	kSetUG17                =  88,
	kSetUG18                =  89,
	kSetUG19                =  90,
	kSetFreeSlotA           =  91,
	kSetFreeSlotB           =  92,
	kSetFreeSlotC           =  93,
	kSetFreeSlotD           =  94,
	kSetFreeSlotE           =  95,
	kSetFreeSlotF           =  96,
	kSetFreeSlotG           =  97,
	kSetFreeSlotH           =  98,
	kSetFreeSlotI           =  99,
	kSetFreeSlotJ           = 100,
	kSetPS15                = 101,
	kSetBB12                = 102
};

enum GameItems {
	kItemPS10Target1            =   0,
	kItemPS10Target2            =   1,
	kItemPS10Target3            =   2,
	kItemPS10Target4            =   3,
	kItemPS10Target5            =   4,
	kItemPS10Target6            =   5,
	kItemPS10Target7            =   6,
	kItemPS10Target8            =   7,
	kItemPS10Target9            =   8,
	kItemPS11Target1            =   9,
	kItemPS11Target2            =  10,
	kItemPS11Target3            =  11,
	kItemPS11Target4            =  12,
	kItemPS11Target5            =  13,
	kItemPS11Target6            =  14,
	kItemPS11Target7            =  15,
	kItemPS11Target8            =  16,
	kItemPS11Target9            =  17,
	kItemPS11Target10           =  18,
	kItemPS11Target11           =  19,
	kItemPS11Target12           =  20,
	kItemPS11Target13           =  21,
	kItemPS11Target14           =  22,
	kItemPS11Target15           =  23,
	// 24 is never used
	// 25 is never used
	// 26 is never used
	kItemPS11Target16           =  27,
	// 28 is never used
	kItemPS12Target1            =  29,
	kItemPS12Target2            =  30,
	kItemPS12Target3            =  31,
	kItemPS12Target4            =  32,
	kItemPS12Target5            =  33,
	kItemPS12Target6            =  34,
	kItemPS12Target7            =  35,
	kItemPS12Target8            =  36,
	kItemPS12Target9            =  37,
	kItemPS12Target10           =  38,
	kItemPS12Target11           =  39,
	kItemPS12Target12           =  40,
	kItemPS12Target13           =  41,
	kItemPS12Target14           =  42,
	kItemPS12Target15           =  43,
	kItemPS12Target16           =  44,
	kItemPS12Target17           =  45,
	kItemPS13Target1            =  46,
	kItemPS13Target2            =  47,
	kItemPS13Target3            =  48,
	kItemPS13Target4            =  49,
	kItemPS13Target5            =  50,
	kItemPS13Target6            =  51,
	kItemPS13Target7            =  52,
	kItemPS13Target8            =  53,
	kItemPS13Target9            =  54,
	kItemPS13Target10           =  55,
	kItemPS13Target11           =  56,
	kItemPS13Target12           =  57,
	kItemPS13Target13           =  58,
	// 59 is never used
	// 60 is never used
	// 61 is never used
	kItemPS13Target14           =  62,
	kItemPS13Target15           =  63,
	// 64 is never used
	// 65 is never used
	kItemChromeDebris           =  66,
	// 67 is never used
	// 68 is never used
	// 69 is never used
	// 70 is never used
	// 71 is never used
	// 72 is never used
	// 73 is never used
	// 74 is never used
	// 75 is never used
	// 76 is never used
	kItemDragonflyEarring       =  76,
	kItemBB06ControlBox         =  77,
	kItemBomb                   =  78,
	kItemCandy                  =  79,
	kItemCheese                 =  81,
	kItemChopstickWrapper       =  82,
	kItemDNATyrell              =  83,
	kItemDogCollar              =  84,
	kItemRagDoll                =  85,
	// 86 is never used
	// 87 is never used
	kItemRadiationGoogles       =  88,
	kItemGordosLighterReplicant =  89,
	kItemGordosLighterHuman     =  90,
	kItemBriefcase              =  91,
	// 92 is never used
	// 93 is never used
	// 94 is never used
	// 95 is never used
	// 96 is never used
	// 97 is never used
	kItemToyDog                 =  98,
	// 99 is never used
	kItemShellCasingA           = 100,
	kItemShellCasingB           = 101,
	kItemShellCasingC           = 102,
	kItemDeadDogA               = 103,
	kItemDeadDogB               = 104,
	kItemDeadDogC               = 105,
	kItemScorpions              = 106,
	kItemCamera                 = 107,
	kItemKitchenBox             = 108,
	kItemChair                  = 109,
	kItemWeaponsCrate           = 110,
	kItemWeaponsOrderForm       = 111,
	// 112 is never used
	// 113 is never used
	// 114 is never used
	kItemDogWrapper             = 115,
	// 116 is never used
	// 117 is never used
	kItemPowerSource            = 118,
	kItemTyrellSalesPamphlet    = 119,
	kItemMoonbusPhoto           = 120,
	kItemGreenPawnLock          = 121,
	kItemChain                  = 122
};

enum GameModelAnimations {
	//   0 -  53: McCoy animations
	kModelAnimationMcCoyWithGunIdle              =   0,
	kModelAnimationMcCoyWithGunGotHitRight       =   1,
	kModelAnimationMcCoyWithGunGotHitLeft        =   2,
	kModelAnimationMcCoyWithGunWalking           =   3,
	kModelAnimationMcCoyWithGunRunning           =   4,
	kModelAnimationMcCoyWithGunShotDead          =   5,
	kModelAnimationMcCoyWithGunClimbStairsUp     =   6,
	kModelAnimationMcCoyWithGunClimbStairsDown   =   7,
	kModelAnimationMcCoyWithGunUnholsterGun      =   8,
	kModelAnimationMcCoyWithGunHolsterGun        =   9,
	kModelAnimationMcCoyWithGunAiming            =  10,
	kModelAnimationMcCoyWithGunStopAimResumeIdle =  11,
	kModelAnimationMcCoyWithGunShooting          =  12,
	kModelAnimationMcCoyWalking                  =  13,
	kModelAnimationMcCoyRunning                  =  14,
	kModelAnimationMcCoyClimbStairsUp            =  15,
	kModelAnimationMcCoyClimbStairsDown          =  16,
	kModelAnimationMcCoyGotHitRight              =  17,
	kModelAnimationMcCoyFallsOnHisBack           =  18,
	kModelAnimationMcCoyIdle                     =  19,
	kModelAnimationMcCoyProtestingTalk           =  20,
	kModelAnimationMcCoyScratchHeadTalk          =  21,
	kModelAnimationMcCoyPointingTalk             =  22,
	kModelAnimationMcCoyUpsetTalk                =  23,
	kModelAnimationMcCoyDismissiveTalk           =  24,
	kModelAnimationMcCoyScratchEarTalk           =  25,
	kModelAnimationMcCoyHandsOnWaistTalk         =  26,
	kModelAnimationMcCoyScratchEarLongerTalk     =  27,
	kModelAnimationMcCoyDodgeAndDrawGun          =  28,
	kModelAnimationMcCoyLeaningOver              =  29,
	kModelAnimationMcCoyThrowsBeggarInTrash      =  30,
	kModelAnimationMcCoyDiesInAgony              =  31,
	kModelAnimationMcCoyGivesFromPocket          =  32,
	kModelAnimationMcCoyLeaningOverSearching     =  33,
	kModelAnimationMcCoyLeaningOverResumeIdle    =  34,
	kModelAnimationMcCoyEntersSpinner            =  35,
	kModelAnimationMcCoyExitsSpinner             =  36,
	kModelAnimationMcCoyClimbsLadderUp           =  37,
	kModelAnimationMcCoyClimbsLadderDown         =  38,
	kModelAnimationMcCoyRecoversFromPassingOut   =  39,
	kModelAnimationMcCoyGiveMovement             =  40,
	kModelAnimationMcCoySittingToUseConsole      =  41,
	kModelAnimationMcCoyWithGunGrabbedByArm0     =  42,
	kModelAnimationMcCoyWithGunGrabbedByArm1     =  43,
	kModelAnimationMcCoyWithGunGrabbedByArmHurt  =  44,
	kModelAnimationMcCoyWithGunGrabbedByArmFreed =  45,
	kModelAnimationMcCoyTiedInChairIdle          =  46,
	kModelAnimationMcCoyTiedInChairMoving        =  47,
	kModelAnimationMcCoyTiedInChairFreed         =  48,
	kModelAnimationMcCoyStartled                 =  49,
	kModelAnimationMcCoyCrouchingDown            =  50,
	kModelAnimationMcCoyCrouchedIdle             =  51,
	kModelAnimationMcCoyCrouchedGetsUp           =  52,
	kModelAnimationMcCoyDrinkingBooze            =  53,
	//  54 -  92: Steele animations
	//  93 - 133: Gordo animations
	// 134 - 171: Dektora animations
	kModelAnimationDektoraCombatIdle             = 134,
	kModelAnimationDektoraCombatWalkingA         = 135,
	kModelAnimationDektoraCombatWalkingB         = 136,
	kModelAnimationDektoraCombatGotHitRight      = 137,
	kModelAnimationDektoraCombatGotHitLeft       = 138,
	kModelAnimationDektoraCombatBegin            = 139, // assumes fighting pose
	kModelAnimationDektoraCombatEnd              = 140, // exits combat mode
	kModelAnimationDektoraCombatLegAttack        = 141,
	kModelAnimationDektoraCombatPunchAttack      = 142,
	kModelAnimationDektoraWalking                = 143,
	kModelAnimationDektoraRunning                = 144, // fast walking
	kModelAnimationDektoraClimbStairsUp          = 145,
	kModelAnimationDektoraClimbStairsDown        = 146,
	kModelAnimationDektoraFrontShoveMove         = 147,
	kModelAnimationDektoraBackDodgeMove          = 148,
	kModelAnimationDektoraFallsDead              = 149,
	kModelAnimationDektoraSittingIdle            = 150,
	kModelAnimationDektoraSittingShootingGun     = 151, // UNUSED?
	kModelAnimationDektoraSittingSubtleTalking   = 152,
	kModelAnimationDektoraSittingIntenseTalking  = 153,
	kModelAnimationDektoraSittingPullingGunOut   = 154,
	kModelAnimationDektoraSittingHoldingGun      = 155,
	kModelAnimationDektoraStandingIdle           = 156, // slow nod left right, could be talking too
	kModelAnimationDektoraStandingTalkGestureA   = 157, // dismissive / questioning
	kModelAnimationDektoraStandingNodShort       = 158, // could be talking too
	kModelAnimationDektoraStandingTalkAgreeing   = 159,
	kModelAnimationDektoraStandingTalkGestureB   = 160, // mellow
	kModelAnimationDektoraStandingTalkGestureC   = 161, // move both hands
	kModelAnimationDektoraStandingTalkGestureD   = 162, // appreciative
	kModelAnimationDektoraInFlamesA              = 163,
	kModelAnimationDektoraInFlamesGotHit         = 164, // UNUSED?
	kModelAnimationDektoraInFlamesStartFalling   = 165,
	kModelAnimationDektoraInFlamesB              = 166,
	kModelAnimationDektoraInFlamesEndFalling     = 167,
	kModelAnimationDektoraDancingA               = 168,
	kModelAnimationDektoraDancingB               = 169,
	kModelAnimationDektoraDancingC               = 170,
	kModelAnimationDektoraDancingFinale          = 171,
	// 172 - 207: Guzza animations
	// 208 - 252: Clovis animations
	// 253 - 276: Lucy animations
	// 277 - 311: Izo animations
	// 312 - 345: Sadik animations
	// 316 - 359: Twins/Luther animations
	// 360 - 387: EarlyQ animations
	// 388 - 421: Zuben animations
	// 422 - 437: Generic walker A/B/C animations (with/without umbrella, walking/still)
	// 438 - 439: Dancer model animations (unused?)
	kModelAnimationBulletBobsTrackingGun         = 440,
	kModelAnimationMaleTargetEmptyHandsActive    = 441,
	kModelAnimationMaleTargetEmptyHandsDead      = 442, // 441+1
	kModelAnimationMaleTargetWithGunActive       = 443,
	kModelAnimationMaleTargetWithGunDead         = 444, // 443+1
	kModelAnimationMaleTargetWithShotgunActive   = 445,
	kModelAnimationMaleTargetWithShotgunDead     = 446, // 445+1
	kModelAnimationFemaleTargetWithBabyActive    = 447,
	kModelAnimationFemaleTargetWithBabyDead      = 448, // 447+1
	kModelAnimationFemaleTargetWithGunActive     = 449,
	kModelAnimationFemaleTargetWithGunDead       = 450, // 449+1
	//       451: T-Pose Security Guard (unused?)
	// 452 - 469: CrazyLegs animations
	// 470 - 486: Grigorian animations
	// 487 - 505: Transient/Homeless animations
	// 506 - 525: Bullet Bob animations
	// 526 - 544: Runciter animations
	// 545 - 554: Insect Dealer animations
	// 555 - 565: Tyrell Guard animations
	// 566 - 570: Mia animations
	// 571 - 604: Officer Leary animations
	// 605 - 641: Officer Grayford animations
	// 642 - 660: Hanoi animations
	// 661 - 670: Desk Clerk (Yukon) animations
	// 671 - 681: Howie Lee animations
	// 682 - 687: Fish Dealer animations
	// 688 - 697: Dino Klein animations
	kModelAnimationKleinWalking                    = 688,
	kModelAnimationKleinStandingIdle               = 689,
	kModelAnimationKleinTalkScratchBackOfHead      = 690,
	kModelAnimationKleinWorkingOnInstruments       = 691,
	kModelAnimationKleinTalkSmallLeftHandMove      = 692,
	kModelAnimationKleinTalkRightHandTouchFace     = 693,
	kModelAnimationKleinTalkWideHandMotion         = 694,
	kModelAnimationKleinTalkSuggestOrAsk           = 695,
	kModelAnimationKleinTalkDismissive             = 696,
	kModelAnimationKleinTalkRaisingBothHands       = 697,
	// 698 - 704: Murray animations
	// 705 - 715: Hawker's Barkeep animations
	// 716 - 721: Holloway animations
	// 722 - 731: Sergeant Walls animations
	// 732 - 743: Moraji animations
	// 744 - 750: Photographer animations
	//       751: Rajif animations
	// 752 - 757: EarlyQ Bartender animations
	// 758 - 764: Shoeshine Man animations (UNUSED)
	// 765 - 772: Tyrell animations
	// 773 - 787: Chew animations
	// 788 - 804: Gaff animations
	// 805 - 808: Bear "Bryant" (Sebastian's toy) animations
	// 809 - 821: Sebastian animations
	// 822 - 832: Rachael animations
	kModelAnimationRachaelWalking                  = 822,
	kModelAnimationRachaelIdle                     = 823,
	kModelAnimationRachaelIdleOscilate             = 824, // UNUSED
	kModelAnimationRachaelTalkSoftNod              = 825,
	kModelAnimationRachaelTalkNodToLeft            = 826,
	kModelAnimationRachaelTalkSuggestWithNodToLeft = 827,
	kModelAnimationRachaelTalkIndiffWithNodToLeft  = 828,
	kModelAnimationRachaelTalkOfferPointing        = 829,
	kModelAnimationRachaelTalkHaltMovement         = 830,
	kModelAnimationRachaelTalkHandOnChest          = 831,
	kModelAnimationRachaelTalkHandWaveToRight      = 832,
	// 833 - 837: "General" (Sebastian's toy) animations
	// 838 - 845: Mama Isabella animations
	// 846 - 856: Leon animations
	// 857 - 862: Rat (Free Slot A/B) animations
	// 863 - 876: Maggie animations
	// 877 - 884: Hysteria Patron1 (dancer) animations
	kModelAnimationHysteriaPatron1DanceStandingUpSemiSitAndUp = 877,
	kModelAnimationHysteriaPatron1DanceStandingUpLeftMotion   = 878,
	kModelAnimationHysteriaPatron1DanceStandingUpStowingMoney = 879, // original UNUSED
	kModelAnimationHysteriaPatron1DanceSplitsDuckAndDown      = 880,
	kModelAnimationHysteriaPatron1DanceSplitsSemiUpAndDown    = 881,
	kModelAnimationHysteriaPatron1DanceSplitsBackAndForth     = 882,
	kModelAnimationHysteriaPatron1DanceStandingUpToSplits     = 883,
	kModelAnimationHysteriaPatron1DanceSplitsToStandingUp     = 884,
	// 885 - 892: Hysteria Patron2 (dancer) animations
	kModelAnimationHysteriaPatron2DanceHandsBellyMotion       = 885, // most used
	kModelAnimationHysteriaPatron2DanceHandsUpLeftMotion      = 886,
	kModelAnimationHysteriaPatron2DanceHandsUpSitAndUp        = 887,
	kModelAnimationHysteriaPatron2DanceHandsDownHipsSwirl     = 888,
	kModelAnimationHysteriaPatron2DanceHandsDownLegSwirl      = 889,
	kModelAnimationHysteriaPatron2DanceHandsDownLeanBackForth = 890,
	kModelAnimationHysteriaPatron2DanceHandsUpToHandsDown     = 891,
	kModelAnimationHysteriaPatron2DanceHandsDownToHandsUp     = 892,
	// 893 - 900: Mutant 1 animations
	// 901 - 907: Mutant 2 animations
	// 908 - 917: Mutant 3 animations
	// 918 - 919: Taffy Patron (Gordo's hostage) animations
	// 920 - 930: Hasan animations
	kModelAnimationHasanTalkWipeFaceLeftHand01    = 920, // UNUSED
	kModelAnimationHasanIdleSlightMovement        = 921,
	kModelAnimationHasanTalkSuggest               = 922,
	kModelAnimationHasanTakingABiteSnakeMove      = 923,
	kModelAnimationHasanTalkPointingRightHand     = 924, // UNUSED
	kModelAnimationHasanTalkMovingBothHands       = 925,
	kModelAnimationHasanTalkLeftRightLeftGesture  = 926,
	kModelAnimationHasanTalkRaiseHandSnakeMove    = 927,
	kModelAnimationHasanTalkMovingBothHandsAndNod = 928,
	kModelAnimationHasanTalkWipeFaceLeftHand02    = 929, // (almost) identical to 920 (same number of frames too)
	kModelAnimationHasanTalkUpset                 = 930,
	// 931 - 996: Item animations
	kModelAnimationBadge                       = 931,
	kModelAnimationBomb                        = 932,
	kModelAnimationCandy                       = 933,
	kModelAnimationCandyWrapper                = 934,
	kModelAnimationDektorasCard                = 935,
	kModelAnimationCheese                      = 936,
	kModelAnimationChopstickWrapper            = 937,
	kModelAnimationChromeDebris                = 938,
	kModelAnimationDragonflyBelt               = 939,
	kModelAnimationDragonflyEarring            = 940,
	kModelAnimationDNADataDisc                 = 941,
	kModelAnimationDogCollar                   = 942,
	kModelAnimationRagDoll                     = 943,
	kModelAnimationEnvelope                    = 944,
	kModelAnimationFlaskOfAbsinthe             = 945,
	kModelAnimationFolderInKIA                 = 946,
	kModelAnimationGoldfish                    = 947,
	kModelAnimationFolder                      = 948,
//	kModelAnimationLetter                      = 949, // UNUSED?
	kModelAnimationGarterSnake                 = 950,
	kModelAnimationLichenDogWrapper            = 951,
	kModelAnimationLicensePlate                = 952,
	kModelAnimationGordosLighterReplicant      = 953,
	kModelAnimationGordosLighterHuman          = 954,
	kModelAnimationKingstonKitchenBox          = 955,
	kModelAnimationMaggieBracelet              = 956,
	kModelAnimationGrigoriansNote              = 957,
	kModelAnimationOriginalRequisitionForm     = 958,
	kModelAnimationOriginalShippingForm        = 959,
	kModelAnimationPowerSource                 = 960,
	kModelAnimationCollectionReceipt           = 961,
	kModelAnimationRequisitionForm             = 962,
	kModelAnimationRadiationGoggles            = 963,
	kModelAnimationReferenceLetter             = 964,
	kModelAnimationWeaponsOrderForm            = 965,
	kModelAnimationShellCasings                = 966,
	kModelAnimationSlug                        = 967,
//	kModelAnimationKnife                       = 968, // UNUSED - 3 frames - a knife? - spins badly
	kModelAnimationStrangeScale                = 969,
	kModelAnimationHysteriaToken               = 970,
	kModelAnimationToyDog                      = 971,
	kModelAnimationTyrellSalesPamphlet         = 972,
	kModelAnimationTyrellSalesPamphletKIA      = 973,
	kModelAnimationDetonatorWire               = 974,
	kModelAnimationVideoDisc                   = 975,
	kModelAnimationCageOfScorpions             = 976,
	kModelAnimationIzoCamera                   = 977,
	kModelAnimationDeadDogA                    = 978,
	kModelAnimationDeadDogB                    = 979,
	kModelAnimationDeadDogC                    = 980,
	kModelAnimationMarcusDead                  = 981,
	kModelAnimationYukonHotelChair             = 982,
	kModelAnimationWeaponsCrate                = 983,
	kModelAnimationPhoto                       = 984,
	kModelAnimationCrystalsCigarette           = 985,
	kModelAnimationSpinnerKeys                 = 986,
	kModelAnimationBriefcase                   = 987,
	kModelAnimationDNAEvidence01OutOf6         = 988,
//	kModelAnimationDNAEvidence02OutOf6         = 989, // UNUSED - actual 2 parts of DNA
	kModelAnimationDNAEvidence03OutOf6         = 990, // used for two parts found
	kModelAnimationDNAEvidence04OutOf6         = 991, // used for three parts found
//	kModelAnimationDNAEvidence05OutOf6         = 992, // UNUSED - actual 5 parts of DNA
	kModelAnimationDNAEvidenceComplete         = 993,
//	kModelAnimationAmmoType00                  = 994, // UNUSED - simple bullet
	kModelAnimationAmmoType01                  = 995, // from Bullet Bob's
	kModelAnimationAmmoType02                  = 996  // from Izo stash (Act 4)
};

enum Elevators {
	kElevatorMA = 1,
	kElevatorPS = 2
};

// enum SceneObjectOffset {
// 	kSceneObjectActorIdStart  = 0,
// 	kSceneObjectActorIdEnd    = kSceneObjectActorIdStart  +  73,
// 	kSceneObjectItemIdStart   = kSceneObjectActorIdEnd    +   1,
// 	kSceneObjectItemIdEnd     = kSceneObjectItemIdStart   + 123,
// 	kSceneObjectObjectIdStart = kSceneObjectItemIdEnd     +   1,
// 	kSceneObjectObjectIdEnd   = kSceneObjectObjectIdStart +  95
// };

enum SceneObjectOffset {
	kSceneObjectOffsetActors  =   0,
	kSceneObjectOffsetItems   =  74,
	kSceneObjectOffsetObjects = 198
};

enum ActorCombatStates {
	kActorCombatStateIdle                 = 0,
	kActorCombatStateCover                = 1,
	kActorCombatStateApproachCloseAttack  = 2,
	kActorCombatStateUncover              = 3,
	kActorCombatStateAim                  = 4,
	kActorCombatStateRangedAttack         = 5,
	kActorCombatStateCloseAttack          = 6,
	kActorCombatStateFlee                 = 7,
	kActorCombatStateApproachRangedAttack = 8
};

enum PoliceMazeTrackInstruction {
	kPMTIActivate        = -26, // args: variableId, maxValue
	kPMTILeave           = -25, // args: -
	kPMTIShoot           = -24, // args: soundId, notUsed
	kPMTIEnemyReset      = -23, // args: otherItemId
	kPMTIEnemySet        = -22, // args: otherItemId
	kPMTIFlagReset       = -21, // args: flagId
	kPMTIFlagSet         = -20, // args: flagId
	kPMTIVariableDec     = -19, // args: variableId
	kPMTIVariableInc     = -18, // args: variableId, maxValue
	kPMTIVariableReset   = -17, // args: variableId
	kPMTIVariableSet     = -16, // args: variableId, value
	kPMTITargetSet       = -15, // args: otherItemId, value
	kPMTIPausedReset1of3 = -14, // args: otherItemId1, otherItemId2, otherItemId3
	kPMTIPausedReset1of2 = -13, // args: otherItemId1, otherItemId2
	kPMTIPausedSet       = -12, // args: otherItemId
	kPMTIPausedReset     = -11, // args: otherItemId
	kPMTIPlaySound       = -10, // args: soundId, volume
	kPMTIObstacleReset   =  -9, // args: otherItemId
	kPMTIObstacleSet     =  -8, // args: otherItemId
	kPMTIWaitRandom      =  -7, // args: min, max
	kPMTIRotate          =  -6, // args: target, delta
	kPMTIFacing          =  -5, // args: angle
	kPMTIRestart         =  -4, // args: -
	kPMTIWait            =  -3, // args: time
	kPMTIMove            =  -2, // args: target
	kPMTIPosition        =  -1, // args: index
	kPMTI26              =   0
};

enum PlayerAgenda {
	kPlayerAgendaPolite     = 0,
	kPlayerAgendaNormal     = 1,
	kPlayerAgendaSurly      = 2,
	kPlayerAgendaErratic    = 3,
	kPlayerAgendaUserChoice = 4
};

enum AffectionTowards {
	kAffectionTowardsNone    = 0,
	kAffectionTowardsSteele  = 1,
	kAffectionTowardsDektora = 2,
	kAffectionTowardsLucy    = 3
};

/*
 * Common goals:
 *   0 -  99 - first chapter
 * 100 - 199 - second chapter
 * 200 - 299 - third chapter
 * 300 - 399 - fourth chapter
 * 400 - 499 - fifth chapter
 * 599       - dead / gone
 */

enum GoalMcCoy {
	kGoalMcCoyDefault              =   0,
	kGoalMcCoyDodge                =   1,
	kGoalMcCoyBB11GetUp            = 100,
	kGoalMcCoyBB11PrepareToRunAway = 101,
	kGoalMcCoyBB11RunAway          = 102,
	kGoalMcCoyBB11GetCaught        = 103,
	kGoalMcCoyNRxxSitAtTable       = 200,
	kGoalMcCoyNRxxStandUp          = 201,
	kGoalMcCoyNR01ThrownOut        = 210,
	kGoalMcCoyNR01GetUp            = 211,
	kGoalMcCoyNR01LayDrugged       = 212,
	kGoalMcCoyNR04Drink            = 220,
	kGoalMcCoyNR04PassOut          = 221,
	kGoalMcCoyNR11Shoot            = 230,
	kGoalMcCoyNR10Fall             = 231,
	kGoalMcCoyUG07Caught           = 301,
	kGoalMcCoyUG07BrokenFinger     = 302,
	kGoalMcCoyUG07Released         = 303,
	kGoalMcCoyCallWithGuzza        = 350,
	kGoalMcCoyUG15Fall             = 390,
	kGoalMcCoyUG15Die              = 391,
	kGoalMcCoyStartChapter5        = 400,
	kGoalMcCoyArrested             = 500,
	kGoalMcCoyGone                 = 599
};

enum GoalSteele {
	// chapter 1
	kGoalSteeleDefault                     =   0,
	kGoalSteeleGoToRC01                    =   1,
	kGoalSteeleGoToRC02                    =   2,
	kGoalSteeleGoToFreeSlotC1              =   3,
	kGoalSteeleGoToFreeSlotG1              =   4,
	kGoalSteeleGoToCT01                    =   5,
	kGoalSteeleGoToFreeSlotC2              =   6,
	kGoalSteeleGoToFreeSlotG2              =   7,
	kGoalSteeleGoToPoliceShootingRange     =   8,
	kGoalSteeleGoToPS02                    =   9,
	kGoalSteeleGoToFreeSlotG3              =  10,
	kGoalSteeleInterviewGrigorian          =  11,
	// chapter 2
	kGoalSteeleApprehendIzo                = 100,
	kGoalSteeleGoToTB02                    = 110,
	kGoalSteeleLeaveTB02                   = 111,
	kGoalSteeleGoToPoliceStation           = 112,
	kGoalSteeleWalkAround                  = 113,
	kGoalSteeleShootIzo                    = 120,
	kGoalSteeleArrestIzo                   = 121,
	kGoalSteeleIzoBlockedByMcCoy           = 125,
	kGoalSteeleLeaveRC03                   = 130,
	kGoalSteeleWalkAroundRestart           = 190,
	// chapter 3
	kGoalSteeleStartChapter3               = 205,
	kGoalSteeleNR11StartWaiting            = 210,
	kGoalSteeleNR11StopWaiting             = 211,
	kGoalSteeleNR11Enter                   = 212,
	kGoalSteeleNR11Entered                 = 213,
	kGoalSteeleNR11Decide                  = 214,
	kGoalSteeleNR11Shoot                   = 215,
	kGoalSteeleNR11StopShooting            = 216,
	kGoalSteeleNR01WaitForMcCoy            = 230,
	kGoalSteeleNR01GoToNR08                = 231,
	kGoalSteeleNR01GoToNR02                = 232,
	kGoalSteeleNR01GoToHF03                = 233,
	kGoalSteeleHF03McCoyChasingLucy        = 234,
	kGoalSteeleNR08WalkOut                 = 235,
	kGoalSteeleNR10Wait                    = 236,
	kGoalSteeleHF02ConfrontLucy            = 240,
	kGoalSteeleHF02ShootLucy               = 241,
	kGoalSteeleHF02LucyShotBySteele        = 242,
	kGoalSteeleHF02LucyRanAway             = 243,
	kGoalSteeleHF02LucyShotByMcCoy         = 244,
	kGoalSteeleHF02LucyLostByMcCoy         = 245,
	kGoalSteeleHFxxGoToSpinner             = 246,
	kGoalSteeleNR01ConfrontGordo           = 250,
	kGoalSteeleNR01TalkToGordo             = 251,
	kGoalSteeleNR01ShootGordo              = 252,
	kGoalSteeleNR01ShootMcCoy              = 255,
	kGoalSteeleNR01McCoyShotGordo          = 258,
	kGoalSteeleNR01McCoyShotGun            = 260,
	kGoalSteeleNR01PrepareTalkAboutShotGun = 261,
	kGoalSteeleNR01TalkAboutShotGun        = 262,
	kGoalSteeleNR01ShotByMcCoy             = 270,
	kGoalSteeleNR01PrepareShotByMcCoy      = 271,
	kGoalSteeleNRxxGoToSpinner             = 275,
	kGoalSteeleTalkAboutMissingSpinner     = 280,
	kGoalSteeleImmediatelyStartChapter4    = 285,
	kGoalSteeleNR01StartChapter4           = 290,
	kGoalSteeleHF01StartChapter4           = 291,
	// chapter 4
	kGoalSteeleStartChapter4               = 300,
	// chapter 5
	kGoalSteeleStartChapter5               = 400,
	kGoalSteeleHF06Attack                  = 402, // has no use
	// kGoalSteeleKP03WillShootMcCoy = 410,
	kGoalSteeleKP03Walk                    = 411,
	kGoalSteeleKP03StopWalking             = 412,
	kGoalSteeleKP03Leave                   = 413,
	kGoalSteeleKP03Exploded                = 415,
	kGoalSteeleKP03Dying                   = 416,
	kGoalSteeleKP03ShootMcCoy              = 418,
	kGoalSteeleKP03Dead                    = 419,
	kGoalSteeleKP01Wait                    = 420,
	kGoalSteeleKP01TalkToMcCoy             = 421,
	kGoalSteeleKP01Leave                   = 422,
	kGoalSteeleKP01Left                    = 423,
	kGoalSteeleKP05Enter                   = 430,
	kGoalSteeleKP05Leave                   = 431,
	kGoalSteeleKP06Enter                   = 432,
	kGoalSteeleKP06Leave                   = 433,
	kGoalSteeleWaitingForEnd               = 499,
	kGoalSteeleGone                        = 599
};

enum GoalDektora {
	kGoalDektoraDefault                      =   0,
	// chapter 1
	kGoalDektoraStartWalkingAround           = 100,
	kGoalDektoraWalkAroundAsReplicant        = 101,
	kGoalDektoraWalkAroundAsHuman            = 102,
	kGoalDektoraStopWalkingAround            = 103,
	// chapter 2
	kGoalDektoraStartChapter3                = 199,
	kGoalDektoraNR07Sit                      = 200,
	kGoalDektoraNR08Dance                    = 210,
	kGoalDektoraNR08Leave                    = 211,
	kGoalDektoraNR08ReadyToRun               = 245,
	kGoalDektoraNR08GoToNR10                 = 246,
	kGoalDektoraNR10AttackMcCoy              = 247,
	kGoalDektoraNR11Hiding                   = 250,
	kGoalDektoraNR11WalkAway                 = 260,
	kGoalDektoraNR11PrepareBurning           = 269,
	kGoalDektoraNR11Burning                  = 270,
	kGoalDektoraNR11BurningGoToMcCoy         = 271,
	kGoalDektoraNR11BurningGoToWindow        = 272,
	kGoalDektoraNR11PrepareFallThroughWindow = 273,
	kGoalDektoraNR11FallThroughWindow        = 274,
	kGoalDektoraNR11BurningFallToNR10        = 279,
	kGoalDektoraNR11RanAway                  = 290,
	kGoalDektoraNR07RanAway                  = 295,
	kGoalDektoraKP07Wait                     = 513, // new goal when in moonbus, Act 5
	kGoalDektoraGone                         = 599
};

enum GoalGordo {
	// chapter 1
	kGoalGordoDefault                   =   0,
	kGoalGordoCT01Leave                 =   1,
	kGoalGordoCT01Left                  =   2,
	kGoalGordoCT05WalkThrough           =   3,
	kGoalGordoCT05Leave                 =   4,
	kGoalGordoCT05Left                  =   5,
	kGoalGordoCT01StandUp               =  90,
	kGoalGordoCT01WalkAway              =  91,
	kGoalGordoCT01BidFarewellToHowieLee =  92,
	kGoalGordoCT01WalkToHowieLee        =  93,
	// chapter 2
	kGoalGordoWalkAround                = 100,
	kGoalGordoGoToDNARow                = 101,
	kGoalGordoGoToChinaTown             = 102,
	kGoalGordoGoToFreeSlotHAGJ          = 103,
	kGoalGordoGoToFreeSlotAH            = 104,
	// chapter 3
	kGoalGordoStartChapter3             = 200,
	kGoalGordoNR02WaitAtBar             = 201,
	kGoalGordoNR02GoToPodium            = 202,
	kGoalGordoNR02WaitForMcCoy          = 204,
	kGoalGordoNR02NextAct               = 205,
	kGoalGordoNR02TellJoke1             = 206,
	kGoalGordoNR02TellJoke2             = 207,
	kGoalGordoNR02TellJoke3             = 208,
	kGoalGordoNR02TalkAboutMcCoy        = 210,
	kGoalGordoNR02WaitAtPodium          = 211,
	kGoalGordoNR02TalkToMcCoy           = 215,
	kGoalGordoNR02RunAway1              = 220,
	kGoalGordoNR02RunAway2              = 221,
	kGoalGordoNR02RunAway3              = 222,
	kGoalGordoGoToFreeSlotGAG           = 225,
	kGoalGordoNR01WaitAndAttack         = 230,
	kGoalGordoNR01Attack                = 231,
	kGoalGordoNR01WaitAndGiveUp         = 240,
	kGoalGordoNR01GiveUp                = 241,
	kGoalGordoNR01TalkToMcCoy           = 242,
	kGoalGordoNR01RunAway               = 243,
	kGoalGordoNR01WaitAndTakeHostage    = 250,
	kGoalGordoNR01ReleaseHostage        = 251,
	kGoalGordoNR01HostageShot           = 254,
	kGoalGordoNR01HostageDie            = 255,
	kGoalGordoNR01Arrested              = 260,
	kGoalGordoNR01RanAway               = 280,
	kGoalGordoNR01Die                   = 299,
	kGoalGordoKP07Wait                  = 513, // new goal when in moonbus, Act 5
	kGoalGordoGone                      = 599
};

enum GoalGuzza {

	kGoalGuzzaDefault                 =   0, // added goal
	kGoalGuzzaLeftOffice              = 100,
	kGoalGuzzaGoToHawkersCircle1      = 101,
	kGoalGuzzaAtOffice                = 102,
	kGoalGuzzaGoToHawkersCircle2      = 103,
	kGoalGuzzaGoToFreeSlotB           = 104,
	kGoalGuzzaGoToFreeSlotG           = 105,
	kGoalGuzzaWasAtNR03               = 200,
	kGoalGuzzaSitAtNR03               = 201,
	kGoalGuzzaUG18Wait                = 300,
	kGoalGuzzaUG18Target              = 301,
	kGoalGuzzaUG18WillGetShotBySadik  = 302,
	kGoalGuzzaUG18HitByMcCoy          = 303,
	kGoalGuzzaUG18MissedByMcCoy       = 304,
	kGoalGuzzaUG18ShotByMcCoy         = 305,
	kGoalGuzzaUG18ShootMcCoy          = 306,
	kGoalGuzzaUG18FallDown            = 307,
	kGoalGuzzaUG18ShotBySadik         = 390,
	kGoalGuzzaGone                    = 599
};

enum GoalClovis {
	kGoalClovisDefault                   = 100,
	kGoalClovisBB11WalkToMcCoy           = 101,
	kGoalClovisBB11StopSadik             = 102,
	kGoalClovisBB11TalkWithSadik         = 103,
	kGoalClovisBB11PrepareTalkToMcCoy    = 104, // bug? this is not triggered when player skips dialogue too fast
	kGoalClovisBB11TalkToMcCoy           = 105, // ends Chapter 2
	// chapter 4
	kGoalClovisUG18Wait                  = 300,
	kGoalClovisUG18SadikWillShootGuzza   = 301,
	kGoalClovisUG18SadikIsShootingGuzza  = 302,
	kGoalClovisUG18GuzzaDied             = 303,
	kGoalClovisUG18Leave                 = 310,
	kGoalClovisStartChapter3             = 350,
	kGoalClovisStartChapter4             = 400,
	kGoalClovisUG07ChaseMcCoy            = 401,
	kGoalClovisUG07KillMcCoy             = 402,
	// chapter 5
	kGoalClovisStartChapter5             = 500,
	kGoalClovisDecide                    = 510,
	kGoalClovisKP06Wait                  = 511, // this immediately reset to kGoalClovisKP07Wait by Sadik at start of chapter 5
	kGoalClovisKP06TalkToMcCoy           = 512, // never triggered
	kGoalClovisKP07Wait                  = 513,
	kGoalClovisKP07TalkToMcCoy           = 514,
	kGoalClovisKP07SayFinalWords         = 515,
	kGoalClovisKP07FlyAway               = 516, // and game over
	kGoalClovisKP07ReplicantsAttackMcCoy = 517,
	kGoalClovisKP07LayDown               = 518,
	kGoalClovisGone                      = 599
};

enum GoalLucy {
	kGoalLucyDefault              =   0,
	// chapter 3
	kGoalLucyMoveAround           = 200,
	kGoalLucyWillReturnToHF03     = 201,
	kGoalLucyGoToHF03             = 205,
	kGoalLucyHF03RunOutPhase1     = 210,
	kGoalLucyHF03RunOutPhase2     = 211,
	kGoalLucyHF03RunToHF041       = 212,
	kGoalLucyHF03RunToHF042       = 213,
	kGoalLucyHF03RunAwayWithHelp1 = 214,
	kGoalLucyHF03RunAwayWithHelp2 = 215,
	kGoalLucyGoToFreeSlotGAG      = 220,
	kGoalLucyGoToFreeSlotGAHJ     = 225,
	kGoalLucyHF04Start            = 230,
	kGoalLucyHF04TalkToMcCoy      = 232,
	kGoalLucyHF04Run1             = 233,
	kGoalLucyHF04Run2             = 234,
	kGoalLucyHF04WaitForMcCoy1    = 235,
	kGoalLucyHF04Run3             = 236,
	kGoalLucyHF04WaitForMcCoy2    = 237,
	kGoalLucyHF04Run4             = 238,
	kGoalLucyHF04WalkAway         = 239,
	kGoalLucyReturnToHF03         = 250,
	kGoalLucyHF03RanAway          = 299,
	// chapter 4
	kGoalLucyStartChapter4        = 300,
	kGoalLucyUG01Wait             = 310,
	kGoalLucyUG01VoightKampff     = 311,
	kGoalLucyUG01RunAway          = 312,
	kGoalLucyKP07Wait             = 513, // new goal when in moonbus, Act 5
	kGoalLucyGone                 = 599
};

enum GoalIzo {
	kGoalIzoDefault         =   0,
	kGoalIzoPrepareCamera   =   1,
	kGoalIzoTakePhoto       =   2,
	kGoalIzoRunToUG02       =   3,
	kGoalIzoStopRunning     = 100,
	kGoalIzoRunToRC03       = 101,
	kGoalIzoWaitingAtRC03   = 102,
	kGoalIzoEscape          = 103, // how can this happen? can mccoy catch izo in ug02?
	kGoalIzoRC03Walk        = 110,
	kGoalIzoRC03Run         = 111,
	kGoalIzoRC03RunAway     = 114,
	kGoalIzoGetArrested     = 120,
	kGoalIzoGotArrested     = 180,
	kGoalIzoRC03RanAwayDone = 181, // new - added for bug fix of Izo getting stuck at RC03
	kGoalIzoGoToHC01        = 150,
	kGoalIzoGoToHC03        = 155,
	kGoalIzoDieHidden       = 198,
	kGoalIzoDie             = 199,
	kGoalIzoKP07Wait        = 513, // new goal when in moonbus, Act 5
	kGoalIzoGone            = 599
};

enum GoalSadik {
	kGoalSadikDefault                       = 100,
	kGoalSadikRunFromBB09                   = 101,
	kGoalSadikBB11Wait                      = 102,
	kGoalSadikBB11ThrowMcCoy                = 103,
	kGoalSadikBB11CatchMcCoy                = 104,
	kGoalSadikBB11KnockOutMcCoy             = 105,
	kGoalSadikBB11KickMcCoy                 = 106,
	kGoalSadikBB11TalkWithClovis            = 107,
	kGoalSadikUG18Wait                      = 300,
	kGoalSadikUG18Move                      = 301,
	kGoalSadikUG18Decide                    = 302,
	// 303, 304 and 305 are never set or used
	kGoalSadikUG18WillShootMcCoy            = 306,
	kGoalSadikUG18PrepareShootMcCoy         = 307,
	kGoalSadikUG18ShootMcCoy                = 308,
	kGoalSadikUG18Leave                     = 310,
	kGoalSadikKP06NeedsReactorCoreFromMcCoy = 416,
	kGoalSadikGone                          = 599
};

enum GoalLuther {
	kGoalLutherDefault           = 400,
	kGoalLutherMoveAround        = 401,
	kGoalLutherMoveAroundRestart = 402,
	kGoalLutherStop              = 403,
	kGoalLutherShot              = 494,
	kGoalLutherDyingStarted      = 495,
	kGoalLutherDyingWait         = 496,
	kGoalLutherDyingCheck        = 497,
	kGoalLutherDie               = 498,
	kGoalLutherDead              = 499,
	kGoalLutherKP07Wait          = 513, // new goal when in moonbus, Act 5
	kGoalLutherGone              = 599
};

enum GoalHowieLee {
	kGoalHowieLeeDefault              =   0,
	kGoalHowieLeeMovesInDiner01       =   1,
	kGoalHowieLeeMovesInDiner02       =   2,
	kGoalHowieLeeMovesInDiner03       =   3,
	kGoalHowieLeeGoesToCT04GarbageBin =   4,
	kGoalHowieLeeGoesToFreeSlotH      =   5,
	kGoalHowieLeeMovesInDiner06       =   6, // Unused goal?
	kGoalHowieLeeStopMoving           =  50,
	kGoalHowieLeeGoesToFreeSlotC      = 100
};

enum GoalTransient {
	kGoalTransientDefault   = 0,
	kGoalTransientCT04Leave = 2
};

enum GoalBulletBob {
	kGoalBulletBobDefault       =  0,
	kGoalBulletBobWarningMcCoy  =  1,
	kGoalBulletBobShootMcCoy    =  2,
	kGoalBulletBobWillShotMcCoy =  3, // has no use
	kGoalBulletBobDead          =  4,
	kGoalBulletBobShotMcCoy     =  6,
	kGoalBulletBobGone          = 99
};

// applies to all generic walkers
enum GoalGenericWalker {
	kGoalGenwalkerDefault             =   0, // setup walker model and path
	kGoalGenwalkerMoving              =   1,
	kGoalGenwalkerABulletBobsTrackGun = 200  // only kActorGenWalkerA will change goal to this
};

enum GoalRachael {
	kGoalRachaelDefault                        =   0,
	kGoalRachaelLeavesAfterTyrellMeeting       = 200,
	kGoalRachaelShouldBeOutsideMcCoysAct3      = 211, // added Rachael goal
	kGoalRachaelShouldBeInElevatorMcCoysAct3   = 212, // added Rachael goal
	kGoalRachaelIsOutsideMcCoysBuildingAct3    = 300,
	kGoalRachaelIsInsideMcCoysElevatorAct3     = 302, // added Rachael goal
	kGoalRachaelIsInsideElevatorStartTalkAct3  = 303, // added Rachael goal
	kGoalRachaelIsOutWalksToPoliceHQAct3       = 305,
	kGoalRachaelIsOutResumesWalkToPoliceHQAct3 = 306,
	kGoalRachaelIsOutFleeingToPoliceHQAct3     = 307, // added Rachael goal
	kGoalRachaelAtEndOfAct3IfNotMetWithMcCoy   = 399, // added Rachael goal
	kGoalRachaelAtEndOfAct3IfMetWithMcCoy      = 400,
	kGoalRachaelIsInsideMcCoysElevatorAct4     = 402, // added Rachael goal
	kGoalRachaelIsInsideElevatorStartTalkAct4  = 403, // added Rachael goal
	kGoalRachaelIsOutsideMcCoysBuildingAct4    = 404, // added Rachael goal
	kGoalRachaelIsOutWalksToPoliceHQAct4       = 405, // added Rachael goal
	kGoalRachaelAtEndOfAct4                    = 499  // added Rachael goal
};

enum GoalRunciter {
	kGoalRunciterDefault        =   0,
	kGoalRunciterRC02WalkAround =   1,
	kGoalRunciterGoToFreeSlotGH =   2,
	kGoalRunciterRC02Wait       = 300,
	kGoalRunciterDead           = 599
};

enum GoalTyrellGuard {
	kGoalTyrellGuardSleeping             = 300,
	kGoalTyrellGuardWakeUpAndArrestMcCoy = 301,
	kGoalTyrellGuardWakeUp               = 302,
	kGoalTyrellGuardArrestMcCoy          = 303,
	kGoalTyrellGuardWait                 = 304
};

enum GoalEarlyQ {
	// cut feature? goals 0 - 200 has no use as EarlyQ is walking around NR which is not accessible
	kGoalEarlyQWalkAround           = 200,
	kGoalEarlyQNR04Enter            = 201,
	kGoalEarlyQNR04Talk1            = 202,
	kGoalEarlyQNR04GoToBar          = 203,
	kGoalEarlyQNR04PourDrink        = 204,
	kGoalEarlyQNR04GoToMcCoy        = 205,
	kGoalEarlyQNR04McCoyPulledGun   = 206,
	kGoalEarlyQNR04SitDown          = 207,
	kGoalEarlyQNR04ScorpionsCheck   = 208,
	kGoalEarlyQNR04Talk2            = 209,
	kGoalEarlyQNR04StungByScorpions = 210,
	kGoalEarlyQNR04WaitForPulledGun = 211,
	kGoalEarlyQNR04TakeDisk         = 212,
	kGoalEarlyQNR04Talk3            = 213,
	kGoalEarlyQNR04AskForDisk       = 214,
	kGoalEarlyQNR04HandDrink        = 215,
	kGoalEarlyQNR04GetShot          = 216,
	kGoalEarlyQNR04Leave            = 217,
	kGoalEarlyQNR05Wait             = 220,
	kGoalEarlyQNR05WillLeave        = 221,
	kGoalEarlyQNR05Leave            = 222,
	kGoalEarlyQNR05AnnouceDektora   = 223,
	kGoalEarlyQNR05UnlockNR08       = 224,
	kGoalEarlyQNR05UnlockedNR08     = 225,
	kGoalEarlyQNR05TalkingToMcCoy   = 229,
	kGoalEarlyQNR04Wait             = 230
};

enum GoalZuben {
	kGoalZubenDefault             =   0,
	kGoalZubenCT01WalkToCT02      =   1,
	kGoalZubenCT02RunToFreeSlotG  =   2,
	kGoalZubenCT07Spared          =   4,
	kGoalZubenCT07RunToFreeSlotA  =   5,
	kGoalZubenDie                 =   6,
	kGoalZubenSpared              =   7,
	kGoalZubenCT02PushPot         =   8,
	kGoalZubenCT02RunToDoor       =   9,
	kGoalZubenCT02OpenDoor        =  10,
	kGoalZubenCT06JumpDown        =  11,
	kGoalZubenCT06AttackMcCoy     =  12,
	kGoalZubenCT06HideAtFreeSlotA =  13,
	kGoalZubenCT02PotDodgeCheck   =  14,
	kGoalZubenFled                =  20,
	kGoalZubenMA01AttackMcCoy     =  21,
	kGoalZubenDiesInChapter1      =  99,
	kGoalZubenKP07Wait            = 513, // new goal when in moonbus, Act 5
	kGoalZubenGone                = 599
};

enum GoalHasan {
	kGoalHasanDefault                         =   0,
	kGoalHasanIsAway                          = 300, // original final goal for Hasan
	kGoalHasanIsWalkingAroundIsAtAR02         = 301, // new goal for restored content
	kGoalHasanIsWalkingAroundStayAwayFromAR02 = 302, // new goal for restored content
	kGoalHasanIsWalkingAroundStaysAtAR02      = 303, // new goal for restored content
	kGoalHasanIsWalkingAroundIsAway           = 304  // new goal for restored content
};

enum GoalOfficerLeary {
	kGoalOfficerLearyDefault                          =   0,
	kGoalOfficerLearyRC01WalkToCrowd                  =   1,
#if BLADERUNNER_ORIGINAL_BUGS
	kGoalOfficerLearyRC01CrowdInterrogation           =   2,
#else
	kGoalOfficerLearyRC01CrowdInterrogation           =   2,
	kGoalOfficerLearyRC01ResumeWalkToCrowd            =   4, // added OfficerLeary goal
#endif // BLADERUNNER_ORIGINAL_BUGS
	kGoalOfficerLearyPoliceDoneFromRC01               =   3,
	kGoalOfficerLearyEndOfAct1                        =  99,
	kGoalOfficerLearyVisitsBulletBob                  = 102, // un-triggered
	kGoalOfficerLearyStartOfAct4                      = 300,
	kGoalOfficerLearyHuntingAroundAct4                = 305,
	kGoalOfficerLearyPrepareToHuntAroundAct4          = 306,
	kGoalOfficerLearyBlockingUG07                     = 307,
	// 308 - 309 un-triggered?
	kGoalOfficerLearyAttackMcCoyAct4                  = 310,
	kGoalOfficerLearyStartOfAct5                      = 400,
	kGoalOfficerLearyDummyGoalAct5                    = 410,
	kGoalOfficerLearyPoliceWait120SecondsToAttackHF05 = 420,
	kGoalOfficerLearyPoliceWait60SecondsToAttackHF05  = 425,
	kGoalOfficerLearyPoliceAboutToAttackHF05          = 430,
	kGoalOfficerLearyDead                             = 599
};


enum GoalOfficerGrayford {
	kGoalOfficerGrayfordDefault                       =   0,
	kGoalOfficerGrayfordWalksInPS03a                  =   1,
	kGoalOfficerGrayfordWalksInPS03b                  =   2,
	kGoalOfficerGrayfordWalksInPS03c                  =   3,
	kGoalOfficerGrayfordWalksInPS03d                  =   4,
	kGoalOfficerGrayfordWalksInFreeSlotC              =   5,
	kGoalOfficerGrayfordWalksInPS09a                  =   6,
	kGoalOfficerGrayfordWalksInPS09b                  =   7,
	kGoalOfficerGrayfordWalksInPS09c                  =   8,
	kGoalOfficerGrayfordWalksInPS03e                  =   9,
	kGoalOfficerGrayfordPrepareToRestartWalkAround    =  10,
	kGoalOfficerGrayfordStopAndTalk1                  =  99, // this is used temporarily to make him stop and talk
	kGoalOfficerGrayfordArrivesToDR04                 = 101,
	kGoalOfficerGrayfordArrivedAtDR04                 = 102,
	kGoalOfficerGrayfordTalkToMcCoyAndReportAtDR04    = 103,
	kGoalOfficerGrayfordPatrolsAtDR04a                = 104,
	kGoalOfficerGrayfordPatrolsAtDR04b                = 105,
	kGoalOfficerGrayfordStopPatrolToTalkToMcCoyAtDR04 = 106,
	kGoalOfficerGrayfordLeavesWithMorajiCorpseDR04    = 110,
	kGoalOfficerGrayfordStopAndTalkDR04               = 199, // this is used temporarily to make him stop and talk while at DR04 (Moraji's death scene)
	kGoalOfficerGrayfordStartOfAct4                   = 300,
	kGoalOfficerGrayfordHuntingAroundAct4             = 305,
	kGoalOfficerGrayfordPrepareToHuntAroundAct4       = 306,
	kGoalOfficerGrayfordBlockingUG07                  = 307,
	kGoalOfficerGrayfordArrestsMcCoyAct4CT12          = 308,
	kGoalOfficerGrayfordAttackMcCoyAct4               = 310,
	kGoalOfficerGrayfordArrestMcCoyInTB03Act4         = 399, // TB02_TB03
	kGoalOfficerGrayfordStartOfAct5                   = 400,
	kGoalOfficerGrayfordDummyGoalAct5                 = 410,
	kGoalOfficerGrayfordPoliceAboutToAttackHF05       = 430,
	kGoalOfficerGrayfordDead                          = 599
};

enum GoalHanoi {
	kGoalHanoiDefault                 = 200,
	kGoalHanoiResetTimer              = 201,
	kGoalHanoiNR07TalkToMcCoy         = 202,
	kGoalHanoiNR07GrabMcCoy           = 203,
	kGoalHanoiNR03GoToDefaultPosition = 210,
	kGoalHanoiNR03GoToSwivelTable     = 211,
	kGoalHanoiNR03GoToOfficeDoor      = 212,
	kGoalHanoiNR03GoToDancer          = 213,
	kGoalHanoiNR03StartGuarding       = 215,
	kGoalHanoiThrowOutMcCoy           = 220,
	kGoalHanoiNR08WatchShow           = 230,
	kGoalHanoiNR08Leave               = 235,
	kGoalHanoiNR08Left                = 236,
	kGoalHanoiNR04Enter               = 240,
	kGoalHanoiNR04ShootMcCoy          = 241
};

enum GoalDeskClerk {
	kGoalDeskClerkDefault           =   0,
	kGoalDeskClerkKnockedOut        =   1,
	kGoalDeskClerkRecovered         =   2,
	kGoalDeskClerkGone              = 400
};

enum GoalKlein {
	kGoalKleinDefault                 =   0,
	kGoalKleinMovingInLab01           =   1,
	kGoalKleinMovingInLab02           =   2,
	kGoalKleinGotoLabSpeaker          =   3,
	kGoalKleinIsAnnoyedByMcCoyInit    =   4,
	kGoalKleinIsAnnoyedByMcCoy01      =   5,
	kGoalKleinIsAnnoyedByMcCoy02      =   6,
	kGoalKleinIsAnnoyedByMcCoyFinal   =   7,
	kGoalKleinIsAnnoyedByMcCoyPreInit =   8, // new goal
	kGoalKleinAwayAtEndOfActOne       = 100,
	kGoalKleinAwayAtEndOfActThree     = 299  // new goal for restored content purposes
};

enum GoalHolloway {
	kGoalHollowayDefault             =   0,
	kGoalHollowayGoToNR07            = 240,
	kGoalHollowayGoToNR02            = 241,
	kGoalHollowayGoToHF03            = 242,
	kGoalHollowayApproachMcCoy       = 250,
	kGoalHollowayTalkToMcCoy         = 251,
	kGoalHollowayKnockOutMcCoy       = 255,
	kGoalHollowayPrepareCaptureMcCoy = 256,
	kGoalHollowayCaptureMcCoy        = 257
};

enum GoalMoraji {
	kGoalMorajiDefault    =  0,
	kGoalMorajiShot       =  5,
	kGoalMorajiFreed      = 10,
	kGoalMorajiRunOut     = 11,
	kGoalMorajiGetUp      = 18,
	kGoalMorajiScream     = 19,
	kGoalMorajiJump       = 20,
	kGoalMorajiLayDown    = 21,
	kGoalMorajiDie        = 22,
	kGoalMorajiDead       = 23,
	kGoalMorajiChooseFate = 30,
	kGoalMorajiPerished   = 99
};

enum GoalGaff {
	kGoalGaffDefault             =   0,
	kGoalGaffCT12WaitForMcCoy    =   1,
	kGoalGaffCT12GoToSpinner     =   2,
	kGoalGaffMA01ApproachMcCoy   =   3,
	kGoalGaffMA01Leave           =   4,
	kGoalGaffCT12Leave           =   5,
	kGoalGaffCT12FlyAway         =  10,
	kGoalGaffStartWalkingAround  = 100,
	kGoalGaffWalkAround          = 101,
	kGoalGaffRepeatWalkingAround = 102,
	kGoalGaffStartChapter4       = 299,
	kGoalGaffMA07Wait            = 300,
	kGoalGaffMA07TalkToMcCoy     = 301,
	kGoalGaffMA07Left            = 302,
	kGoalGaffMA07ShootMcCoy      = 303, // cannot be triggered
	kGoalGaffGone                = 499
};

enum GoalLeon {
	kGoalLeonDefault            = 0,
	kGoalLeonHoldingDeskClerk   = 1,
	kGoalLeonReleaseDeskClerk   = 2,
	kGoalLeonPrepareTalkToMcCoy = 3,
	kGoalLeonTalkToMcCoy        = 4,
	kGoalLeonApproachMcCoy      = 5, // there is no way how to trigger this path in the game
	kGoalLeonPunchMcCoy         = 6, // there is no way how to trigger this path in the game
	kGoalLeonLeave              = 7,
	kGoalLeonGone               = 8
};

enum GoalCrazyLegs {
	kGoalCrazyLegsDefault         =   0,
	kGoalCrazyLegsShotAndHit      =   1,
	kGoalCrazyLegsLeavesShowroom  =   2, // set either after McCoy re-holsters his gun, or if McCoy tells him to lie to Steele
	// goal 10 seems unused
	// goal 10 updates to 11 (also unused)
	kGoalCrazyLegsMcCoyDrewHisGun = 210, // When McCoy draws gun and Ch < 5
	kGoalCrazyLegsIsArrested      = 699
};

enum GoalFreeSlotA { // Rat A
	kGoalFreeSlotADefault            =   0,
	kGoalFreeSlotAUG15Wait           = 300,
	kGoalFreeSlotAUG15WalkOut        = 301,
	kGoalFreeSlotAUG15RunToOtherSide = 302,
	kGoalFreeSlotAUG15RunBack        = 303,
	kGoalFreeSlotAUG15Attack         = 304,
	kGoalFreeSlotAWalkAround         = 306,
	kGoalFreeSlotAWalkAroundRestart  = 307,
	kGoalFreeSlotAAttackMcCoy        = 308,
	kGoalFreeSlotAUG15Die            = 305,
	kGoalFreeSlotAUG15Fall           = 309,
	kGoalFreeSlotAUG15Prepare        = 310,
	kGoalFreeSlotAAct5Default        = 400,
	kGoalFreeSlotAAct5Prepare        = 405,
	kGoalFreeSlotAAct5KP02Attack     = 406,
	kGoalFreeSlotAGone               = 599
};

enum GoalFreeSlotB { // Rat B
	kGoalFreeSlotBAct4Default     = 300,
	kGoalFreeSlotBAct4WalkAround  = 301,
	kGoalFreeSlotBAct4AttackMcCoy = 302,
	kGoalFreeSlotBAct5Default     = 400,
	kGoalFreeSlotBAct5Prepare     = 405,
	kGoalFreeSlotBAct5KP02Attack  = 406,
	kGoalFreeSlotBGone            = 599
};

enum GoalMaggie {
	kGoalMaggieMA02Default        =   0,
	kGoalMaggieMA02WalkToEntrance =   1,
	kGoalMaggieMA02GetFed         =   3,
	kGoalMaggieMA02WalkToMcCoy    =   7,
	kGoalMaggieMA02Wait           =   8,
	kGoalMaggieMA02SitDown        =  10, // not used
	kGoalMaggieMA02Sleep          =  11, // not used
	kGoalMaggieKP05Wait           = 411,
	kGoalMaggieKP05McCoyEntred    = 412,
	kGoalMaggieKP05WalkToMcCoy    = 413,
	kGoalMaggieKP05WillExplode    = 414,
	kGoalMaggieKP05Explode        = 415,
	kGoalMaggieDead               = 599
};

enum ActorTimers {
	kActorTimerAIScriptCustomTask0 = 0,
	kActorTimerAIScriptCustomTask1 = 1,
	kActorTimerAIScriptCustomTask2 = 2,
	kActorTimerMovementTrack       = 3,
	kActorTimerClueExchange        = 4,
	kActorTimerAnimationFrame      = 5,
	kActorTimerRunningStaminaFPS   = 6
};

// Certain tracks are available at Frank Klepacki's website/portfolio for Blade Runner
// Those are noted with their "official" name in a side-comment here, as they appear at the website
// A few may not match the incremental number given in-game (eg kMusicGothic3 is "Gothic Club 2")
enum MusicTracks {
	kMusicArabLoop =  0, // Animoid Row track     (Not available at Frank Klepacki's website/portfolio for Blade Runner)
	kMusicBatl226M =  1, // "Battle Theme"
	kMusicBRBlues  =  2, // "Blade Runner Blues"
	kMusicKyoto    =  3, // "Etsuko Theme"
	kMusicOneTime  =  4, // "One More Time, Love" (Not available at Frank Klepacki's website/portfolio for Blade Runner)
	kMusicGothic3  =  5, // "Gothic Club 2"
	kMusicArkdFly1 =  6,
	kMusicArkDnce1 =  7,
	kMusicTaffy2   =  8, // "Taffy's Club 2"
	kMusicTaffy3   =  9,
	kMusicTaffy4   = 10,
	kMusicBeating1 = 11,
	kMusicCrysDie1 = 12,
	kMusicGothic1  = 13, // "Gothic Club"
	kMusicGothic2  = 14,
	kMusicStrip1   = 15,
	kMusicDkoDnce1 = 16,
	kMusicCredits  = 17, // "Blade Runner End Credits"
	kMusicMoraji   = 18,
	kMusicClovDie1 = 19,
	kMusicClovDies = 20,
	kMusicLoveSong = 21  // "Love Theme" (Lucy, Dektora, Subway drive ending)
};

enum SFXSounds {
	kSfxCROSLOCK =   0,
	kSfxSTEAM1   =   1,
	kSfxSPINNY1  =   2,
	kSfxFEMHURT1 =   3,  // used only 3 times; 2 of them are bugs
	kSfxFEMHURT2 =   4,
	kSfxGUNMISS1 =   5,  // unused
	kSfxGUNMISS2 =   6,  // unused
	kSfxGUNMISS3 =   7,  // unused
	kSfxGUNMISS4 =   8,  // unused
	kSfxHITTARG1 =   9,  // unused
	kSfxHITTARG2 =  10,  // unused
	kSfxHITTARG3 =  11,  // unused
	kSfxLGCAL1   =  12,
	kSfxLGCAL2   =  13,  // used only twice
	kSfxLGCAL3   =  14,
	kSfxMDCAL1   =  15,  // unused
	kSfxMDCAL2   =  16,  // unused
	kSfxRICO1    =  17,  // unused
	kSfxRICO2    =  18,  // unused
	kSfxRICO3    =  19,  // used only once
	kSfxRICOCHT1 =  20,  // unused
	kSfxRICOCHT2 =  21,  // unused
	kSfxRICOCHT3 =  22,  // unused
	kSfxRICOCHT4 =  23,  // unused
	kSfxRICOCHT5 =  24,  // unused
	kSfxSMCAL1   =  25,  // unused
	kSfxSMCAL2   =  26,  // unused
	kSfxSMCAL3   =  27,
	kSfxSMCAL4   =  28,  // unused
	kSfxTARGUP1  =  29,
	kSfxTARGUP2  =  30,  // unused
	kSfxTARGUP3  =  31,  // used only once
	kSfxTARGUP4  =  32,
	kSfxTARGUP5  =  33,
	kSfxTARGUP6  =  34,
	kSfxTARGUP7  =  35,  // unused
	kSfxTRGSPIN1 =  36,  // unused
	kSfxTRGSPIN2 =  37,  // unused
	kSfxTRGSPIN3 =  38,  // unused
	kSfxDRIPPY4  =  39,  // unused
	kSfxDRIPPY10 =  40,
	kSfxDRIPPY1  =  41,
	kSfxDRIPPY2  =  42,
	kSfxDRIPPY6  =  43,
	kSfxDRIPPY7  =  44,
	kSfxPSAMB6   =  45,  //           - Looping
	kSfxRTONE3   =  46,  //           - Looping
	kSfxSCANNER1 =  47,
	kSfxSCANNER2 =  48,
	kSfxSCANNER3 =  49,
	kSfxSCANNER4 =  50,
	kSfxSCANNER5 =  51,
	kSfxSCANNER6 =  52,
	kSfxSCANNER7 =  53,
	kSfxCTRAIN1  =  54,  //           - Looping
	kSfxCTAMBL1  =  55,  //           - Looping
	kSfxCTAMBR1  =  56,  //           - Looping
	kSfxSTEAM3   =  57,
	kSfxSTEAM6A  =  58,
	kSfxNEON5    =  59,
	kSfxNEON6    =  60,
	kSfxDISH1    =  61,
	kSfxDISH2    =  62,
	kSfxDISH3    =  63,
	kSfxDISH4    =  64,
	kSfxDISH5    =  65,  // unused
	kSfxSPIN1A   =  66,
	kSfxSPIN2A   =  67,
	kSfxSPIN2B   =  68,
	kSfxSPIN3A   =  69,
	kSfxBOOLOOP2 =  70,  //           - Looping
	kSfxBRBED5   =  71,  //           - Looping
	kSfxSWEEP2   =  72,
	kSfxSWEEP3   =  73,
	kSfxSWEEP4   =  74,
	kSfxWINDLOP8 =  75,  //           - Looping
	kSfxPETDEAD1 =  76,
	kSfxPETDEAD3 =  77,
	kSfxPETDEAD4 =  78,
	kSfxPETDEAD5 =  79,
	kSfxRCAMB1   =  80,  // unused    - Looping
	kSfxRCAMBR1  =  81,  //           - Looping
	kSfxRCCARBY1 =  82,
	kSfxRCCARBY2 =  83,
	kSfxRCCARBY3 =  84,
	kSfxRCRAIN1  =  85,  //           - Looping
	kSfxRCTALK1  =  86,  // unused    - Looping?
	kSfxSIREN2   =  87,
	kSfxRCTALK2  =  88,  // unused    - Looping?
	kSfxRCTALK3  =  89,  // unused    - Looping?
	kSfxPSDOOR1  =  90,
	kSfxPSDOOR2  =  91,
	kSfxPSPA6    =  92,
	kSfxPSPA7    =  93,
	kSfxPSPA8    =  94,
	kSfxBOILPOT2 =  95,  //           - Looping
	kSfxKTCHRAIN =  96,  //           - Looping
	kSfxNEON7    =  97,
	kSfxBIGFAN2  =  98,  //           - Looping
	kSfxROOFAIR1 =  99,  //           - Looping
	kSfxROOFRMB1 = 100,  //           - Looping
	kSfxROOFRAN1 = 101,  //           - Looping
	kSfxROOFLIT1 = 102,
	kSfxRAINAWN1 = 103,  //           - Looping
	kSfxAPRTAMB5 = 104,  //           - Looping
	kSfxCTRUNOFF = 105,  //           - Looping
	kSfxCTROOFL1 = 106,  //           - Looping
	kSfxCTROOFR1 = 107,  //           - Looping
	kSfxDRAMB4   = 108,  //           - Looping
	kSfxBRBED3   = 109,  //           - Looping
	kSfxRESPRTR1 = 110,  //           - Looping
	kSfxDRAMB5   = 111,  //           - Looping
	kSfxFACTAMB2 = 112,  //             Looping
	kSfxDRRAIN1  = 113,  // unused    - Looping
	kSfxSPINUP1  = 114,  // elevator sound
	kSfxSPINOPN3 = 115,  // unused
	kSfxSPINOPN4 = 116,
	kSfxCARUP3   = 117,
	kSfxCARDOWN3 = 118,
	kSfxSPINCLS1 = 119,
	kSfxSERVOU1  = 120,
	kSfxSERVOD1  = 121,  // used only twice; 1 is unused (shot doll "General")
	kSfxMTLHIT1  = 122,
	kSfxSPNBEEP9 = 123,
	kSfxPRISAMB3 = 124,  //           - Looping
	kSfxPRISSLM1 = 125,
	kSfxPRISSLM2 = 126,
	kSfxPRISSLM3 = 127,
	kSfxSPINAMB2 = 128,  // unused    - Looping?
	kSfxSPNAMB1  = 129,  // unused    - Looping?
	kSfxSPNBEEP2 = 130,
	kSfxSPNBEEP3 = 131,
	kSfxSPNBEEP4 = 132,
	kSfxSPNBEEP5 = 133,
	kSfxSPNBEEP6 = 134,
	kSfxSPNBEEP7 = 135,
	kSfxSPNBEEP8 = 136,
	kSfxPRISAMB1 = 137,  //           - Looping
	kSfxBRBED5X  = 138,  //           - Looping
	kSfxLABAMB1  = 139,  // unused    - Looping?
	kSfxLABAMB2  = 140,  // unused    - Looping?
	kSfxLABAMB3  = 141,  //           - Looping
	kSfxTUBES1   = 142,
	kSfxTUBES2   = 143,  // unused
	kSfxTUBES3   = 144,  // unused
	kSfxTUBES4   = 145,
	kSfxLABMISC1 = 146,
	kSfxLABMISC2 = 147,
	kSfxLABMISC3 = 148,
	kSfxLABMISC4 = 149,
	kSfxLABMISC5 = 150,
	kSfxLABMISC6 = 151,
	kSfxLABMISC7 = 152,  // used only once
	kSfxLABMISC8 = 153,  // used only once
	kSfxLABMISC9 = 154,  // used only once
	kSfxLABBUZZ1 = 155,
	kSfxPAGE1    = 156,
	kSfxPAGE2    = 157,
	kSfxPAGE3    = 158,
	kSfxUPTARG3  = 159,
	kSfxCEMENTL1 = 160,
	kSfxCEMENTL2 = 161,
	kSfxCEMENTL3 = 162,
	kSfxCEMENTL4 = 163,
	kSfxCEMENTL5 = 164,
	kSfxCEMENTR1 = 165,
	kSfxCEMENTR2 = 166,
	kSfxCEMENTR3 = 167,
	kSfxCEMENTR4 = 168,
	kSfxCEMENTR5 = 169,
	kSfxCEMWETL1 = 170,
	kSfxCEMWETL2 = 171,
	kSfxCEMWETL3 = 172,
	kSfxCEMWETL4 = 173,
	kSfxCEMWETL5 = 174,
	kSfxCEMWETR1 = 175,
	kSfxCEMWETR2 = 176,
	kSfxCEMWETR3 = 177,
	kSfxCEMWETR4 = 178,
	kSfxCEMWETR5 = 179,
	kSfxGARBAGE4 = 180,  // used only once
	kSfx67_0470R = 181,
	kSfx67_0480R = 182,
	kSfx67_0500R = 183,
	kSfx67_0540R = 184,
	kSfx67_0560R = 185,
	kSfx67_0870R = 186,
	kSfx67_0880R = 187,  // unused
	kSfx67_0900R = 188,
	kSfx67_0940R = 189,
	kSfx67_0960R = 190,
	kSfx67_1070R = 191,
	kSfx67_1080R = 192,
	kSfx67_1100R = 193,
	kSfx67_1140R = 194,
	kSfx67_1160R = 195,
	kSfxZUBWLK1  = 196,
	kSfxZUBWLK2  = 197,
	kSfxZUBWLK3  = 198,
	kSfxZUBWLK4  = 199,
	kSfxBIGPOT4  = 200,  // unused
	kSfxMTLDOOR2 = 201,
	kSfxPOTSPL4  = 202,  // unused
	kSfxPOTSPL5  = 203,
	kSfxBIGPOT3  = 204,  // used only twice
	kSfxCTDRONE1 = 205,
	kSfxZUBLAND1 = 206,
	kSfxZUBDEAD1 = 207,
	kSfxELDOORO2 = 208,
	kSfxELDOORC1 = 209,
	kSfxELEAMB3  = 210,  //           - Looping
	kSfxFOUNTAIN = 211,  //           - Looping
	kSfxBELLY1   = 212,
	kSfxBELLY2   = 213,
	kSfxBELLY3   = 214,
	kSfxBELLY4   = 215,
	kSfxBELLY5   = 216,
	kSfxHUMMER1  = 217,  //           - Looping and non-looping
	kSfxHUMMER2  = 218,
	kSfxHUMMER3  = 219,  //           - Looping
	kSfxHUMMER4  = 220,
	kSfxPUNCH1   = 221,
	kSfxKICK1    = 222,
	kSfxKICK2    = 223,
	kSfxBANG1    = 224,
	kSfxBANG2    = 225,
	kSfxBANG3    = 226,
	kSfxBANG4    = 227,
	kSfxBANG5    = 228,
	kSfxBANG6    = 229,
	kSfxGLOOP1   = 230,
	kSfxGLOOP2   = 231,  // unused
	kSfxGLOOP3   = 232,  // unused
	kSfxGLOOP4   = 233,
	kSfxPIPER1   = 234,
	kSfxSQUEAK1  = 235,
	kSfxTB5LOOP1 = 236,  //           - Looping
	kSfxTB5LOOP2 = 237,  //           - Looping
	kSfxHCBELL1  = 238,
	kSfxHCSING1  = 239,  // unused
	kSfxINDFLUT1 = 240,
	kSfxHCLOOP1  = 241,  //           - Looping
	kSfxHCANM8   = 242,
	kSfxHCANM2   = 243,
	kSfxHCANM3   = 244,
	kSfxHCANM4   = 245,
	kSfxHCANM5   = 246,
	kSfxHCANM6   = 247,
	kSfxHCANM7   = 248,
	kSfxHCANM1   = 249,
	kSfxDOGTOY3  = 250,  // unused
	kSfxBARAMB2  = 251,  // unused
	kSfxBARSFX1  = 252,
	kSfxBARSFX2  = 253,  // unused
	kSfxBARSFX3  = 254,
	kSfxBARSFX4  = 255,
	kSfxBARSFX5  = 256,
	kSfxBARSFX6  = 257,
	kSfxBARSFX7  = 258,
	kSfxCLINK1   = 259,
	kSfxCLINK2   = 260,
	kSfxCLINK3   = 261,
	kSfxCLINK4   = 262,
	kSfxDOGBARK1 = 263,
	kSfxDOGBARK3 = 264,
	kSfxDOGBITE1 = 265,  // unused
	kSfxDOGCRY1  = 266,  // unused
	kSfxDOGCRY2  = 267,  // unused
	kSfxDOGFAR1  = 268,  // unused
	kSfxDOGFAR2  = 269,  // unused
	kSfxDOGGUN1  = 270,  // unused
	kSfxDOGGUN2  = 271,  // unused
	kSfxDOGHURT1 = 272,
	kSfxDOGMAD1  = 273,  // unused
	kSfxDOGNEED1 = 274,  // unused
	kSfxDOGNEED2 = 275,  // unused
	kSfxDOGTAIL1 = 276,
	kSfxDOGTAIL2 = 277,  // unused
	kSfxDOGTOY1  = 278,  // unused
	kSfxDOGTOY2  = 279,  // unused
	kSfxBARAMB1  = 280,  //           - Looping
	kSfx14KBEEP1 = 281,
	kSfxTBBEEP1  = 282,  // unused
	kSfxTBDOOR1  = 283,
	kSfxTBDOOR2  = 284,  // unused
	kSfxTBLOOP1  = 285,  //           - Looping
	kSfxTRUCKBY1 = 286,
	kSfxCHEVBY1  = 287,
	kSfxFIREBD1  = 288,  //           - Looping
	kSfxGLASSY1  = 289,
	kSfxGLASSY2  = 290,
	kSfxBBDRIP1  = 291,
	kSfxBBDRIP2  = 292,
	kSfxBBDRIP3  = 293,
	kSfxBBDRIP4  = 294,
	kSfxBBDRIP5  = 295,
	kSfxRADIATR1 = 296,  // unused
	kSfxRADIATR2 = 297,
	kSfxRADIATR3 = 298,
	kSfxRADIATR4 = 299,
	kSfxJESTMOV1 = 300,
	kSfxINDXPLOD = 301,
	kSfxJESTMUS1 = 302,  // unused
	kSfxBBGRN1   = 303,
	kSfxBBGRN2   = 304,
	kSfxBBGRN3   = 305,
	kSfxBBMOVE1  = 306,
	kSfxBBMOVE2  = 307,
	kSfxBBMOVE3  = 308,
	kSfxHAUNT1   = 309,
	kSfxHAUNT2   = 310,
	kSfxMONKCYM1 = 311,
	kSfxCAMERA2  = 312,
	kSfxMONKEY1  = 313,
	kSfxMONKEY2  = 314,  // unused
	kSfxRUNAWAY1 = 315,
	kSfxTRPDOOR1 = 316,
	kSfxTRPDORO  = 317,  // unused
	kSfxTRPDORC  = 318,  // unused
	kSfxAUDLAFF1 = 319,
	kSfxAUDLAFF2 = 320,
	kSfxAUDLAFF3 = 321,
	kSfxAUDLAFF4 = 322,
	kSfxAUDLAFF5 = 323,
	kSfxAUDLAFF6 = 324,
	kSfxAUDLAFF7 = 325,
	kSfxAUDLAFF8 = 326,
	kSfxAUDLAFF9 = 327,
	kSfxCHAINLNK = 328,
	kSfxMAMASNG1 = 329,  //           - Looping
	kSfxROTIS2   = 330,  //           - Looping
	kSfxSTMLOOP7 = 331,  //           - Looping
	kSfxUGBED1   = 332,  //           - Looping
	kSfxUGBED2   = 333,  //           - Looping
	kSfxBIGFAN3  = 334,  // unused
	kSfxGETITEM1 = 335,
	kSfxFLORBUZZ = 336,  //           - Looping
	kSfxDESKBELL = 337,
	kSfxCURTAIN1 = 338,
	kSfxDRAWER1  = 339,
	kSfxARCBED1  = 340,  //           - Looping
	kSfxCIRCUS1  = 341,  //           - Looping
	kSfxFORTUNE1 = 342,  // unused (un-triggered)
	kSfxCARCREK1 = 343,
	kSfxCARLAND1 = 344,
	kSfxDORSLID1 = 345,  // used only twice
	kSfxDORSLID2 = 346,  // used only once
	kSfxKUNG1    = 347,  // unused
	kSfxLOWERN1  = 348,
	kSfxLOWERY1  = 349,
	kSfxMAGCHNK1 = 350,
	kSfxMAGDROP1 = 351,
	kSfxMAGMOVE1 = 352,
	kSfxMAGMOVE2 = 353,
	kSfxRAISEN1  = 354,
	kSfxRAISEY1  = 355,
	kSfxBUMSNOR1 = 356,
	kSfxBUMSNOR2 = 357,
	kSfxBUMSNOR3 = 358,
	kSfxSTONDOR1 = 359,
	kSfxSTONDOR2 = 360,
	kSfxSEXYAD2  = 361,
	kSfxMUSBLEED = 362,  //           - Looping
	kSfxSUNROOM1 = 363,
	kSfxSUNROOM2 = 364,
	kSfxSUNROOM3 = 365,
	kSfxSUNROOM4 = 366,
	kSfxSUBWAY1  = 367,
	kSfxYELL1M1  = 368,
	kSfxGRUNT1M1 = 369,
	kSfxGRUNT2M1 = 370,
	kSfxHURT1M1  = 371,
	kSfxCARGELE2 = 372,
	kSfxCARUP3B  = 373,
	kSfxCOLONY   = 374,
	kSfxTHNDER2  = 375,
	kSfxTHNDER3  = 376,
	kSfxTHNDER4  = 377,
	kSfxTHNDR1   = 378,
	kSfxTHNDR2   = 379,
	kSfxTHNDR3   = 380,
	kSfxRAIN10   = 381,  //           - Looping
	kSfxRAINALY1 = 382,  //           - Looping
	kSfxSKINBED1 = 383,  //           - Looping
	kSfxRUMLOOP1 = 384,  //           - Looping
	kSfxPHONE1   = 385,
	kSfxESPLOOP1 = 386,  //           - Looping
	kSfxESPLOOP2 = 387,  //           - Looping
	kSfxESPLOOP3 = 388,  //           - Looping
	kSfxSTEAMY1  = 389,
	kSfxSTEAMY2  = 390,
	kSfxSQUEAK2  = 391,
	kSfxSQUEAK3  = 392,
	kSfxSQUEAK4  = 393,  // used only once
	kSfxSQUEAK5  = 394,
	kSfxGRUNT1M2 = 395,
	kSfxGRUNT1M3 = 396,
	kSfxGRUNT2M2 = 397,
	kSfxGRUNT2M3 = 398,
	kSfxHURT1M2  = 399,
	kSfxHURT1M3  = 400,
	kSfxYELL1M2  = 401,
	kSfxYELL1M3  = 402,
	kSfxVIDFONE1 = 403,
	kSfxDRAIN1X  = 404,
	kSfxTOILET1  = 405,
	kSfxGARGLE1  = 406,
	kSfxWASH1    = 407,
	kSfxAPRTFAN1 = 408,  //           - Looping
	kSfxMA04VO1A = 409,  // unused Developer commentary for MA04 scene (McCoy's apartment)
	kSfxCT01VO1A = 410,  // unused Developer commentary for CT01 scene (Howie Lee's, Chinatown)
	kSfxHC01VO1A = 411,  // unused Developer commentary for HC01 scene (Hawker's Circle)
	kSfxELEBAD1  = 412,
	kSfxBR025_5A = 413,
	kSfxBR027_1P = 414,
	kSfxBR024_4B = 415,
	kSfxBR029_3A = 416,
	kSfxBR034_1A = 417,
	kSfxBR030_3A = 418,
	kSfxBR031_1P = 419,
	kSfxBR028_2A = 420,
	kSfxBR026_2A = 421,  // unused
	kSfxBR032_7B = 422,
	kSfxBR033_4B = 423,
	kSfxBR035_7B = 424,
	kSfxBR010_4A = 425,  // unused
	kSfxBR011_2A = 426,
	kSfxBR012_3B = 427,  // unused
	kSfxBR013_3D = 428,
	kSfxBR014_5A = 429,
	kSfxBR015_3C = 430,  // unused
	kSfxBR016_2B = 431,
	kSfxBR017_2A = 432,  // unused
	kSfxBR018_3E = 433,  // unused
	kSfxBBELE2   = 434,
	kSfxRATTY1   = 435,  // unused
	kSfxRATTY2   = 436,  // unused
	kSfxRATTY3   = 437,
	kSfxRATTY4   = 438,
	kSfxRATTY5   = 439,
	kSfxSCARY1   = 440,
	kSfxSCARY2   = 441,
	kSfxSCARY3   = 442,
	kSfxSCARY4   = 443,
	kSfxSCARY5   = 444,
	kSfxSCARY6   = 445,
	kSfxSCARY7   = 446,
	kSfxPNEUM5   = 447,  // used only once
	kSfxROBOTMV1 = 448,
	kSfxMCGUN1   = 449,
	kSfxMCGUN2   = 450,
	kSfxDEKGLAS1 = 451,  // used only once
	kSfx35MM     = 452,
	kSfx35MMBRK1 = 453,
	kSfx35MMGO1  = 454,  // unused
	kSfxBR027_3P = 455,
	kSfxBRWIND2  = 456,
	kSfxBUTN6    = 457,
	kSfxBR018_1P = 458,
	kSfxBR003_1A = 459,
	kSfxBRTARGET = 460,
	kSfxVKBEEP1  = 461,
	kSfxVKBEEP2  = 462,
	kSfxCAMCOP1  = 463,
	kSfxKPAMB1   = 464,  //           - Looping
	kSfxMANHOLE1 = 465,  // unused
	kSfxMETALL1  = 466,
	kSfxMETALL2  = 467,
	kSfxMETALL3  = 468,
	kSfxMETALL4  = 469,
	kSfxMETALL5  = 470,
	kSfxMETALR1  = 471,
	kSfxMETALR2  = 472,
	kSfxMETALR3  = 473,
	kSfxMETALR4  = 474,
	kSfxMETALR5  = 475,
	kSfxWOODL1   = 476,
	kSfxWOODL2   = 477,
	kSfxWOODL3   = 478,
	kSfxWOODL4   = 479,
	kSfxWOODL5   = 480,
	kSfxWOODR1   = 481,
	kSfxWOODR2   = 482,
	kSfxWOODR3   = 483,
	kSfxWOODR4   = 484,
	kSfxWOODR5   = 485,
	kSfxDIALOUT3 = 486,  // unused
	kSfxBIKEMIX4 = 487,
	kSfxCHAINBRK = 488,
	kSfxGUNAIM1  = 489,  // unused
	kSfxCAREXPL1 = 490,
	kSfxCRYEXPL1 = 491,
	kSfxSHOTCOK1 = 492,
	kSfxSHOTGUN1 = 493,
	kSfxDOGEXPL1 = 494,
	kSfxBEEP16   = 495,
	kSfxMECHAN1C = 496,
	kSfxMECHAN1  = 497,
	kSfxPANEL1   = 498,
	kSfxPANEL2   = 499,
	kSfxPANOPEN  = 500,
	kSfxELECTRO1 = 501,
	kSfxBEEP1    = 502,
	kSfxBUTN4P   = 503,
	kSfxBUTN4R   = 504,
	kSfxBUTN5P   = 505,
	kSfxBUTN5R   = 506,
	kSfxTEXT1    = 507,
	kSfxTEXT3    = 508,
	kSfxBEEP10   = 509,
	kSfxBEEP10A  = 510,
	kSfxBEEP15   = 511,
	kSfxMUSVOL8  = 512,
	kSfxELECBP1  = 513,
	kSfxCRZYEXPL = 514,  // unused
	kSfxELEBUTN1 = 515,
	kSfxELECLAB1 = 516,  //           - Looping
	kSfxGUNH1A   = 517,
	kSfxGUNH1B   = 518,
	kSfxGUNH1C   = 519,
	kSfxGUNH2A   = 520,
	kSfxGUNH2B   = 521,
	kSfxGUNH2C   = 522,
	kSfxGUNH3A   = 523,
	kSfxGUNH3B   = 524,
	kSfxGUNH3C   = 525,
	kSfxGUNM1A   = 526,
	kSfxGUNM1B   = 527,
	kSfxGUNM1C   = 528,
	kSfxGUNM2A   = 529,
	kSfxGUNM2B   = 530,
	kSfxGUNM2C   = 531,
	kSfxGUNM3A   = 532,
	kSfxGUNM3B   = 533,
	kSfxGUNM3C   = 534,
	kSfxGUNH1AR  = 535,  // unused
	kSfxGUNH1BR  = 536,  // unused
	kSfxGUNH1CR  = 537,  // unused
	kSfxGUNH2AR  = 538,  // unused
	kSfxGUNH2BR  = 539,  // unused
	kSfxGUNH2CR  = 540,  // unused
	kSfxGUNH3AR  = 541,  // unused
	kSfxGUNH3BR  = 542,  // unused
	kSfxGUNH3CR  = 543,  // unused
	kSfxGUNM1AR  = 544,  // unused
	kSfxGUNM1BR  = 545,  // unused
	kSfxGUNM1CR  = 546,  // unused
	kSfxGUNM2AR  = 547,  // unused
	kSfxGUNM2BR  = 548,  // unused
	kSfxGUNM2CR  = 549,  // unused
	kSfxGUNM3AR  = 550,  // unused
	kSfxGUNM3BR  = 551,  // unused
	kSfxGUNM3CR  = 552,  // unused
	kSfxGARBAGE  = 553,
	kSfxBELLTONE = 554,  // unused
	kSfxMALEHURT = 555,
	kSfxHOLSTER1 = 556,
	kSfxHEADHIT2 = 557,
	kSfxUGLEVER1 = 558,
	kSfxCOMPDWN4 = 559,
	kSfxDROPGLAS = 560,
	kSfxBRKFNGR1 = 561,
	kSfxBOMBFAIL = 562,
	kSfxBABYCRY2 = 563,
	kSfxBRKROPE1 = 564,
	kSfxKICKDOOR = 565,  // unused
	kSfxDEKCLAP1 = 566,
	kSfxWHISTLE1 = 567,
	kSfxWHISTLE2 = 568,
	kSfxWHISTLE3 = 569,
	kSfxFEMORG1  = 570,
	kSfxFEMORG2  = 571,
	kSfxFEMORG3  = 572,
	kSfxFEMORG4  = 573,
	kSfxDOORLOCK = 574,
	kSfxRIMSHOT1 = 575,
	kSfxRIMSHOT2 = 576,
	kSfxRIMSHOT3 = 577,
	kSfxRIMSHOT4 = 578,
	kSfxRIMSHOT5 = 579,
	kSfxGOTSHOT1 = 580,  // unused
	kSfxCOMEDY   = 581,
	kSfxDRUGOUT  = 582,
	kSfxPLANKDWN = 583,
	kSfxBANGDOOR = 584,  // unused
	kSfxCOMPBED1 = 585,  //           - Looping
	kSfxMOONBED2 = 586,  //           - Looping
	kSfxDATALOAD = 587,
	kSfxBEEPNEAT = 588,
	kSfxWINDLOOP = 589,  //           - Looping
	kSfxTBALARM  = 590,
	kSfxLIGHTON  = 591,
	kSfxCOMPON1  = 592,
	kSfxCHARMTL7 = 593,
	kSfxCHARMTL8 = 594,
	kSfxCHARMTL9 = 595,
	kSfxSHUTDOWN = 596
};

enum GameDifficultyLevel {
	kGameDifficultyEasy   = 0,
	kGameDifficultyMedium = 1,
	kGameDifficultyHard   = 2
};
} // End of namespace BladeRunner

#endif
