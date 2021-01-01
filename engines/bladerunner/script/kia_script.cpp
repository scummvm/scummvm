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

#include "bladerunner/script/kia_script.h"

#include "bladerunner/bladerunner.h"
#include "bladerunner/mouse.h"
#include "bladerunner/ui/kia.h"

namespace BladeRunner {

KIAScript::KIAScript(BladeRunnerEngine *vm) : ScriptBase(vm) {}

void KIAScript::playClueAssetScript(int notUsed, int clueId) {
	_vm->_kia->playerReset();
	_vm->_mouse->disable();
	SCRIPT_KIA_DLL_Play_Clue_Asset_Script(notUsed, clueId);
	_vm->_mouse->enable();
}

void KIAScript::SCRIPT_KIA_DLL_Play_Clue_Asset_Script(int notUsed, int clueId) {
	switch (clueId) {
	case kClueOfficersStatement:
		KIA_Play_Actor_Dialogue(kActorOfficerLeary, 40);
		break;
	case kClueDoorForced2:
		KIA_Play_Actor_Dialogue(kActorOfficerLeary, 0);
		break;
	case kClueDoorForced1:
		if (_vm->_cutContent || _vm->_gameVars[kVariableChapter] == 1) {
			// keep in mind, this line in only available in Act 1 (1.TLK) unless _vm->_cutContent is selected (provided that cut content now loads all TLKs)
			// Without cut content it will be silent in subsequent Acts
			KIA_Play_Actor_Dialogue(kActorVoiceOver, 1870);
		}
		break;
	case kClueLimpingFootprints:
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 1970);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 1980);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 1990);
		break;
	case kClueGracefulFootprints:
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 1970);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 1980);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 1990);
		break;
	case kClueShellCasings:
		KIA_Play_Slice_Model(kModelAnimationShellCasings);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 1960);
		break;
	case kClueCandy:
		KIA_Play_Slice_Model(kModelAnimationCandy);
		break;
	case kClueToyDog:
		KIA_Play_Slice_Model(kModelAnimationToyDog);
		break;
	case kClueChopstickWrapper:
		KIA_Play_Slice_Model(kModelAnimationChopstickWrapper);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2010);
		break;
	case kClueSushiMenu:
		KIA_Play_Photograph(6);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2020);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2030);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2040);
		break;
	case kClueLabCorpses:
		KIA_Play_Actor_Dialogue(kActorKlein, 140);
		// Similar fix to PS07:
		// quote 150 is *boop* in ENG version
		// it is redundant in DEU and FRA versions (identical to second half of quote 140)
		// it is required in ESP and ITA versions. It is the missing second half of quote 140.
		if (_vm->_cutContent
		    && (_vm->_language == Common::ES_ESP
		        || _vm->_language == Common::IT_ITA)
		) {
			KIA_Play_Actor_Dialogue(kActorKlein, 150);
		}
		break;
	case kClueLabShellCasings:
		KIA_Play_Actor_Dialogue(kActorKlein, 50);
		KIA_Play_Actor_Dialogue(kActorKlein, 60);
		KIA_Play_Actor_Dialogue(kActorKlein, 70);
		KIA_Play_Actor_Dialogue(kActorKlein, 80);
		KIA_Play_Actor_Dialogue(kActorKlein, 90);
		break;
	case kClueRuncitersVideo:
		KIA_Play_Slice_Model(kModelAnimationVideoDisc);
		break;
	case kClueLucy:
		KIA_Play_Photograph(5);
		break;
	case kClueDragonflyAnklet:
		KIA_Play_Photograph(4);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 4050);
		break;
	case kClueReferenceLetter:
		KIA_Play_Slice_Model(kModelAnimationReferenceLetter);
		KIA_Play_Actor_Dialogue(kActorRunciter, 280);
		KIA_Play_Actor_Dialogue(kActorRunciter, 290);
		break;
	case kClueCrowdInterviewA:
		if (_vm->_cutContent
		    && (_vm->_language == Common::ES_ESP
		        || _vm->_language == Common::IT_ITA)
		) {
			// Same fix as in RC01:
			// Quote 110 is the second half of the sentence about Lucy hanging around with Zuben ("a fat guy")
			// in ENG, DEU and FRA it is redundant, but it's needed in ESP and ITA
			KIA_Play_Actor_Dialogue(kActorOfficerLeary, 100);
			KIA_Play_Actor_Dialogue(kActorOfficerLeary, 110);
		} else {
			KIA_Play_Actor_Dialogue(kActorOfficerLeary, 100);
		}
		break;
	case kClueCrowdInterviewB:
		KIA_Play_Actor_Dialogue(kActorOfficerLeary, 120);
		KIA_Play_Actor_Dialogue(kActorOfficerLeary, 130);
		break;
	case kClueZubenInterview:
		KIA_Play_Actor_Dialogue(kActorMcCoy, 380);
		KIA_Play_Actor_Dialogue(kActorZuben, 30);
		KIA_Play_Actor_Dialogue(kActorZuben, 40);
		KIA_Play_Actor_Dialogue(kActorMcCoy, 410);
		KIA_Play_Actor_Dialogue(kActorZuben, 50);
		break;
	case kClueZubenSquadPhoto:
		KIA_Play_Photograph(33);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 350);
		break;
	case kClueBigManLimping:
		KIA_Play_Actor_Dialogue(kActorTransient, 10);
		break;
	case kClueRunciterInterviewA:
		KIA_Play_Actor_Dialogue(kActorRunciter, 40);
		KIA_Play_Actor_Dialogue(kActorRunciter, 50);
		KIA_Play_Actor_Dialogue(kActorMcCoy, 4565);
		KIA_Play_Actor_Dialogue(kActorRunciter, 60);
		break;
	case kClueRunciterInterviewB1:
		KIA_Play_Actor_Dialogue(kActorRunciter, 250);
		KIA_Play_Actor_Dialogue(kActorRunciter, 270);
		break;
	case kClueRunciterInterviewB2:
		KIA_Play_Actor_Dialogue(kActorRunciter, 260);
		KIA_Play_Actor_Dialogue(kActorRunciter, 270);
		break;
	case kClueHowieLeeInterview:
		KIA_Play_Actor_Dialogue(kActorMcCoy, 295);
		KIA_Play_Actor_Dialogue(kActorHowieLee, 90);
		KIA_Play_Actor_Dialogue(kActorHowieLee, 100);
		break;
	case kCluePaintTransfer:
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 1880);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 1890);
		break;
	case kClueChromeDebris:
		KIA_Play_Slice_Model(kModelAnimationChromeDebris);
		break;
	case kClueRuncitersViewA:
		KIA_Play_Photograph(11);
		break;
	case kClueRuncitersViewB:
		KIA_Play_Photograph(12);
		break;
	case kClueCarColorAndMake:
		KIA_Play_Photograph(10);
		break;
	case kCluePartialLicenseNumber:
		KIA_Play_Photograph(9);
		break;
	case kClueBriefcase:
		KIA_Play_Slice_Model(kModelAnimationBriefcase);
		KIA_Play_Actor_Dialogue(kActorMcCoy, 5870);
		KIA_Play_Actor_Dialogue(kActorGuzza, 810);
		KIA_Play_Actor_Dialogue(kActorGuzza, 820);
		break;
	case kClueGaffsInformation:
		KIA_Play_Actor_Dialogue(kActorGaff, 20);
		KIA_Play_Actor_Dialogue(kActorMcCoy, 680);
		KIA_Play_Actor_Dialogue(kActorGaff, 30);
		break;
	case kClueLicensePlate:
		KIA_Play_Slice_Model(kModelAnimationLicensePlate);
		break;
	case kClueLabPaintTransfer:
		KIA_Play_Actor_Dialogue(kActorKlein, 170);
		KIA_Play_Actor_Dialogue(kActorKlein, 180);
		KIA_Play_Actor_Dialogue(kActorKlein, 190);
		KIA_Play_Actor_Dialogue(kActorKlein, 200);
		break;
	case kClueDispatchHitAndRun:
		KIA_Play_Actor_Dialogue(kActorDispatcher, 90);
		KIA_Play_Actor_Dialogue(kActorDispatcher, 100);
		KIA_Play_Actor_Dialogue(kActorDispatcher, 110);
		KIA_Play_Actor_Dialogue(kActorDispatcher, 120);
		KIA_Play_Actor_Dialogue(kActorDispatcher, 130);
		KIA_Play_Actor_Dialogue(kActorDispatcher, 140);
		KIA_Play_Actor_Dialogue(kActorDispatcher, 150);
		break;
	case kCluePhoneCallGuzza:
		KIA_Play_Actor_Dialogue(kActorGuzza, 30);
		KIA_Play_Actor_Dialogue(kActorGuzza, 50);
		KIA_Play_Actor_Dialogue(kActorGuzza, 70);
		break;
	case kClueDragonflyEarring:
		KIA_Play_Slice_Model(kModelAnimationDragonflyEarring);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2140);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2150);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2160);
		break;
	case kClueTyrellSecurity:
		KIA_Play_Slice_Model(kModelAnimationVideoDisc);
		break;
	case kClueTyrellGuardInterview:
		KIA_Play_Actor_Dialogue(kActorMcCoy, 5140);
		KIA_Play_Actor_Dialogue(kActorTyrellGuard, 30);
		KIA_Play_Actor_Dialogue(kActorTyrellGuard, 40);
		break;
	case kClueBombingSuspect:
		KIA_Play_Photograph(31);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2140);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2150);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2160);
		break;
	case kClueDetonatorWire:
		KIA_Play_Slice_Model(kModelAnimationDetonatorWire);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2320);
		if (Game_Flag_Query(kFlagSadikIsReplicant)) {
			KIA_Play_Actor_Dialogue(kActorVoiceOver, 2330);
			KIA_Play_Actor_Dialogue(kActorVoiceOver, 2340);
		}
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2350);
		break;
	case kClueVictimInformation:
		KIA_Play_Actor_Dialogue(kActorTyrellGuard, 100);
		KIA_Play_Actor_Dialogue(kActorTyrellGuard, 110);
		KIA_Play_Actor_Dialogue(kActorTyrellGuard, 120);
		KIA_Play_Actor_Dialogue(kActorTyrellGuard, 130);
		break;
	case kClueAttemptedFileAccess:
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2170);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2180);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2190);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2200);
		break;
	case kClueCrystalsCase:
		KIA_Play_Actor_Dialogue(kActorSteele, 2230);
		KIA_Play_Actor_Dialogue(kActorSteele, 2260);
		KIA_Play_Actor_Dialogue(kActorSteele, 2270);
		KIA_Play_Actor_Dialogue(kActorSteele, 2280);
		break;
	case kClueKingstonKitchenBox1:
		KIA_Play_Slice_Model(kModelAnimationKingstonKitchenBox);
		if (Query_Difficulty_Level() == kGameDifficultyEasy) {
			KIA_Play_Actor_Dialogue(kActorVoiceOver, 4140);
		} else {
			KIA_Play_Actor_Dialogue(kActorVoiceOver, 4150);
		}
		break;
	case kClueTyrellSalesPamphletEntertainModel:
		KIA_Play_Slice_Model(kModelAnimationTyrellSalesPamphletKIA);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 4280);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 4290);
		break;
	case kClueTyrellSalesPamphletLolita:
		KIA_Play_Slice_Model(kModelAnimationTyrellSalesPamphletKIA);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 4280);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 4300);
		break;
	case kCluePeruvianLadyInterview:
		KIA_Play_Actor_Dialogue(kActorInsectDealer, 90);
		KIA_Play_Actor_Dialogue(kActorInsectDealer, 100);
		KIA_Play_Actor_Dialogue(kActorInsectDealer, 110);
		KIA_Play_Actor_Dialogue(kActorInsectDealer, 120);
		KIA_Play_Actor_Dialogue(kActorInsectDealer, 130);
		break;
	case kClueHasanInterview:
		KIA_Play_Actor_Dialogue(kActorHasan, 90);
		KIA_Play_Actor_Dialogue(kActorHasan, 100);
		break;
	case kClueBobInterview1:
		KIA_Play_Actor_Dialogue(kActorBulletBob, 320);
		KIA_Play_Actor_Dialogue(kActorBulletBob, 330);
		KIA_Play_Actor_Dialogue(kActorBulletBob, 340);
		KIA_Play_Actor_Dialogue(kActorBulletBob, 380);
		KIA_Play_Actor_Dialogue(kActorBulletBob, 390);
		KIA_Play_Actor_Dialogue(kActorBulletBob, 400);
		break;
	case kClueBobInterview2:
		KIA_Play_Actor_Dialogue(kActorBulletBob, 320);
		KIA_Play_Actor_Dialogue(kActorBulletBob, 330);
		KIA_Play_Actor_Dialogue(kActorBulletBob, 410);
		KIA_Play_Actor_Dialogue(kActorBulletBob, 420);
		KIA_Play_Actor_Dialogue(kActorBulletBob, 440);
		KIA_Play_Actor_Dialogue(kActorBulletBob, 450);
		break;
	case kClueIzoInterview:
		KIA_Play_Actor_Dialogue(kActorIzo, 210);
		KIA_Play_Actor_Dialogue(kActorIzo, 220);
		KIA_Play_Actor_Dialogue(kActorIzo, 240);
		KIA_Play_Actor_Dialogue(kActorIzo, 250);
		break;
	case kClueIzosWarning:
		KIA_Play_Actor_Dialogue(kActorIzo, 750);
		KIA_Play_Actor_Dialogue(kActorIzo, 760);
		KIA_Play_Actor_Dialogue(kActorMcCoy, 5500);
		KIA_Play_Actor_Dialogue(kActorIzo, 780);
		KIA_Play_Actor_Dialogue(kActorIzo, 790);
		break;
	case kClueRadiationGoggles:
		KIA_Play_Slice_Model(kModelAnimationRadiationGoggles);
		break;
	case kClueGogglesReplicantIssue:
		KIA_Play_Actor_Dialogue(kActorBulletBob, 560);
		KIA_Play_Actor_Dialogue(kActorBulletBob, 570);
		KIA_Play_Actor_Dialogue(kActorBulletBob, 580);
		break;
	case kClueFishLadyInterview:
		KIA_Play_Actor_Dialogue(kActorFishDealer, 120);
		KIA_Play_Actor_Dialogue(kActorFishDealer, 130);
		break;
	case kClueDogCollar1:
		KIA_Play_Slice_Model(kModelAnimationDogCollar);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 4160);
		break;
	case kClueWeaponsCache:
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2430);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2440);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2450);
		break;
	case kClueChewInterview:
		KIA_Play_Actor_Dialogue(kActorChew, 140);
		KIA_Play_Actor_Dialogue(kActorChew, 150);
		KIA_Play_Actor_Dialogue(kActorChew, 170);
		KIA_Play_Actor_Dialogue(kActorChew, 180);
		KIA_Play_Actor_Dialogue(kActorChew, 190);
		break;
	case kClueMorajiInterview:
		KIA_Play_Actor_Dialogue(kActorMoraji, 20);
		KIA_Play_Actor_Dialogue(kActorMoraji, 30);
		KIA_Play_Actor_Dialogue(kActorMoraji, 40);
		KIA_Play_Actor_Dialogue(kActorMoraji, 50);
		break;
	case kClueGordoInterview1:
		KIA_Play_Actor_Dialogue(kActorGordo, 1010);
		KIA_Play_Actor_Dialogue(kActorMcCoy, 6495);
		KIA_Play_Actor_Dialogue(kActorGordo, 1020);
		KIA_Play_Actor_Dialogue(kActorMcCoy, 6500);
		KIA_Play_Actor_Dialogue(kActorGordo, 1030);
		break;
	case kClueGordoInterview2:
		KIA_Play_Actor_Dialogue(kActorGordo, 1040);
		KIA_Play_Actor_Dialogue(kActorGordo, 1050);
		KIA_Play_Actor_Dialogue(kActorMcCoy, 6505);
		KIA_Play_Actor_Dialogue(kActorGordo, 1060);
		KIA_Play_Actor_Dialogue(kActorGordo, 1070);
		KIA_Play_Actor_Dialogue(kActorMcCoy, 6510);
		KIA_Play_Actor_Dialogue(kActorGordo, 1080);
		break;
	case kClueAnsweringMachineMessage:
		KIA_Play_Actor_Dialogue(kActorSebastian, 0);
		KIA_Play_Actor_Dialogue(kActorSebastian, 10);
		KIA_Play_Actor_Dialogue(kActorSebastian, 20);
		KIA_Play_Actor_Dialogue(kActorSebastian, 30);
		KIA_Play_Actor_Dialogue(kActorSebastian, 40);
		KIA_Play_Actor_Dialogue(kActorSebastian, 50);
		break;
	case kClueChessTable:
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 80);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 90);
		break;
	case kClueStaggeredbyPunches:
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 4370);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 4380);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 4390);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 4400);
		break;
	case kClueMaggieBracelet:
		KIA_Play_Slice_Model(kModelAnimationMaggieBracelet);
		if (_vm->_cutContent) {
			KIA_Play_Actor_Dialogue(kActorVoiceOver, 4030);
		}
		break;
	case kClueEnvelope:
		KIA_Play_Slice_Model(kModelAnimationEnvelope);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 850);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 860);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 870);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 880);
		break;
	case kClueIzosFriend:
		KIA_Play_Photograph(25);
		break;
	case kClueChinaBarSecurityPhoto:
		KIA_Play_Photograph(20);
		break;
	case kCluePurchasedScorpions:
		KIA_Play_Actor_Dialogue(kActorMcCoy, 220);
		KIA_Play_Actor_Dialogue(kActorInsectDealer, 320);
		KIA_Play_Actor_Dialogue(kActorMcCoy, 225);
		KIA_Play_Actor_Dialogue(kActorInsectDealer, 330);
		KIA_Play_Actor_Dialogue(kActorMcCoy, 230);
		KIA_Play_Actor_Dialogue(kActorInsectDealer, 340);
		break;
	case kClueWeaponsOrderForm:
		KIA_Play_Slice_Model(kModelAnimationWeaponsOrderForm);
		break;
	case kClueShippingForm:
		KIA_Play_Slice_Model(kModelAnimationWeaponsOrderForm);
		break;
	case kClueGuzzasCash:
		KIA_Play_Actor_Dialogue(kActorGuzza, 520);
		KIA_Play_Actor_Dialogue(kActorGuzza, 530);
		KIA_Play_Actor_Dialogue(kActorGuzza, 540);
		KIA_Play_Actor_Dialogue(kActorGuzza, 550);
		break;
	case kClueHysteriaToken:
		KIA_Play_Slice_Model(kModelAnimationHysteriaToken);
		break;
	case kClueRagDoll:
		KIA_Play_Slice_Model(kModelAnimationRagDoll);
		break;
	case kClueMoonbus1:
		KIA_Play_Photograph(34);
		break;
	case kClueCheese:
		KIA_Play_Slice_Model(kModelAnimationCheese);
		break;
	case kClueDektorasDressingRoom:
		KIA_Play_Photograph(16);
		break;
	case kClueEarlyQsClub:
		KIA_Play_Slice_Model(kModelAnimationVideoDisc);
		break;
	case kClueDragonflyCollection:
		KIA_Play_Actor_Dialogue(kActorInsectDealer, 290);
		KIA_Play_Actor_Dialogue(kActorInsectDealer, 300);
		break;
	case kClueDragonflyBelt:
		KIA_Play_Slice_Model(kModelAnimationDragonflyBelt);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 4050);
		break;
	case kClueEarlyQInterview:
		KIA_Play_Actor_Dialogue(kActorEarlyQ, 140);
		KIA_Play_Actor_Dialogue(kActorEarlyQ, 150);
		break;
	case kClueStrangeScale1:
		KIA_Play_Slice_Model(kModelAnimationStrangeScale);
		break;
	case kClueDektoraInterview1:
		KIA_Play_Actor_Dialogue(kActorDektora, 650);
		KIA_Play_Actor_Dialogue(kActorDektora, 660);
		KIA_Play_Actor_Dialogue(kActorMcCoy, 3665);
		KIA_Play_Actor_Dialogue(kActorDektora, 670);
		KIA_Play_Actor_Dialogue(kActorDektora, 680);
		KIA_Play_Actor_Dialogue(kActorDektora, 690);
		break;
	case kClueDektoraInterview2:
		KIA_Play_Actor_Dialogue(kActorDektora, 580);
		break;
	case kClueDektoraInterview3:
		KIA_Play_Actor_Dialogue(kActorMcCoy, 3600);
		KIA_Play_Actor_Dialogue(kActorDektora, 550);
		break;
	case kClueDektorasCard:
		KIA_Play_Slice_Model(kModelAnimationDektorasCard);
		break;
	case kClueGrigoriansNote:
		KIA_Play_Slice_Model(kModelAnimationGrigoriansNote);
		break;
	case kClueCollectionReceipt:
		KIA_Play_Slice_Model(kModelAnimationCollectionReceipt);
		break;
	case kClueSpecialIngredient:
		KIA_Play_Actor_Dialogue(kActorMurray, 210);
		KIA_Play_Actor_Dialogue(kActorMurray, 220);
		KIA_Play_Actor_Dialogue(kActorMia, 140);
		KIA_Play_Actor_Dialogue(kActorMurray, 230);
		break;
	case kClueStolenCheese:
		KIA_Play_Actor_Dialogue(kActorIsabella, 210);
		KIA_Play_Actor_Dialogue(kActorIsabella, 260);
		KIA_Play_Actor_Dialogue(kActorMcCoy, 1390);
		KIA_Play_Actor_Dialogue(kActorIsabella, 300);
		break;
	case kClueGordoInterview3:
		KIA_Play_Actor_Dialogue(kActorGordo, 450);
		KIA_Play_Actor_Dialogue(kActorMcCoy, 3280);
		break;
	case kClueGordoConfession:
		KIA_Play_Actor_Dialogue(kActorMcCoy, 3250);
		KIA_Play_Actor_Dialogue(kActorGordo, 540);
		KIA_Play_Actor_Dialogue(kActorGordo, 550);
		break;
	case kClueGordosLighterReplicant:
		KIA_Play_Slice_Model(kModelAnimationGordosLighterReplicant);
		if (_vm->_cutContent) {
			if (Actor_Clue_Query(kActorMcCoy, kClueZubenSquadPhoto)) {
				KIA_Play_Actor_Dialogue(kActorVoiceOver, 1450);
			} else {
				KIA_Play_Actor_Dialogue(kActorVoiceOver, 350);
			}
			KIA_Play_Actor_Dialogue(kActorVoiceOver, 1460);
			KIA_Play_Actor_Dialogue(kActorVoiceOver, 1470);
		} else {
			KIA_Play_Actor_Dialogue(kActorVoiceOver, 350);
		}
		break;
	case kClueGordosLighterHuman:
		KIA_Play_Slice_Model(kModelAnimationGordosLighterHuman);
		break;
	case kClueDektoraInterview4:
		KIA_Play_Actor_Dialogue(kActorMcCoy, 3860);
		KIA_Play_Actor_Dialogue(kActorDektora, 1030);
		KIA_Play_Actor_Dialogue(kActorDektora, 1040);
		KIA_Play_Actor_Dialogue(kActorMcCoy, 3865);
		KIA_Play_Actor_Dialogue(kActorDektora, 1050);
		KIA_Play_Actor_Dialogue(kActorDektora, 1060);
		break;
	case kClueHollowayInterview:
		KIA_Play_Actor_Dialogue(kActorHolloway, 0);
		KIA_Play_Actor_Dialogue(kActorHolloway, 10);
		break;
	case kClueBakersBadge:
		KIA_Play_Slice_Model(kModelAnimationBadge);
		break;
	case kClueHoldensBadge:
		KIA_Play_Slice_Model(kModelAnimationBadge);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 4420);
		break;
	case kClueCarIdentified:
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 3780);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 3790);
		break;
	case kClueCarRegistration1:
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 3800);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 3810);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 3820);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 3830);
		break;
	case kClueCarRegistration2:
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 3840);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 3850);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 3860);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 3870);
		break;
	case kClueCarRegistration3:
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 3880);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 3890);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 3900);
		break;
	case kClueCrazylegsInterview1:
		KIA_Play_Actor_Dialogue(kActorCrazylegs, 830);
		KIA_Play_Actor_Dialogue(kActorCrazylegs, 840);
		KIA_Play_Actor_Dialogue(kActorCrazylegs, 850);
		break;
	case kClueLichenDogWrapper:
		KIA_Play_Slice_Model(kModelAnimationLichenDogWrapper);
		break;
	case kClueRequisitionForm:
		KIA_Play_Slice_Model(kModelAnimationRequisitionForm);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 3930);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 3940);
		break;
	case kClueScaryChair:
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2550);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2560);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2570);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2580);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2590);
		break;
	case kClueIzosStashRaided:
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2470);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2480);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2490);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2500);
		break;
	case kClueHomelessManInterview1:
		KIA_Play_Actor_Dialogue(kActorMcCoy, 5615);
		KIA_Play_Actor_Dialogue(kActorTransient, 170);
		KIA_Play_Actor_Dialogue(kActorMcCoy, 5625);
		KIA_Play_Actor_Dialogue(kActorTransient, 180);
		KIA_Play_Actor_Dialogue(kActorMcCoy, 5630);
		KIA_Play_Actor_Dialogue(kActorTransient, 190);
		KIA_Play_Actor_Dialogue(kActorMcCoy, 5635);
		KIA_Play_Actor_Dialogue(kActorTransient, 200);
		break;
	case kClueHomelessManInterview2:
		KIA_Play_Actor_Dialogue(kActorMcCoy, 5640);
		KIA_Play_Actor_Dialogue(kActorTransient, 230);
		KIA_Play_Actor_Dialogue(kActorMcCoy, 5645);
		KIA_Play_Actor_Dialogue(kActorTransient, 240);
		KIA_Play_Actor_Dialogue(kActorTransient, 250);
		KIA_Play_Actor_Dialogue(kActorMcCoy, 5650);
		KIA_Play_Actor_Dialogue(kActorTransient, 260);
		break;
	case kClueHomelessManKid:
		KIA_Play_Actor_Dialogue(kActorTransient, 340);
		KIA_Play_Actor_Dialogue(kActorTransient, 350);
		KIA_Play_Actor_Dialogue(kActorTransient, 360);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2710);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2730);
		break;
	case kClueFolder:
		KIA_Play_Slice_Model(kModelAnimationFolder);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2740);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2750);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2760);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 2770);
		break;
	case kClueGuzzaFramedMcCoy:
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 3320);
		break;
	case kClueOriginalShippingForm:
		KIA_Play_Slice_Model(kModelAnimationOriginalShippingForm);
		break;
	case kClueOriginalRequisitionForm:
		KIA_Play_Slice_Model(kModelAnimationOriginalRequisitionForm);
		break;
	case kClueCandyWrapper:
		KIA_Play_Slice_Model(kModelAnimationCandyWrapper);
		break;
	case kClueFlaskOfAbsinthe:
		KIA_Play_Slice_Model(kModelAnimationFlaskOfAbsinthe);
		break;
	case kClueLutherLanceInterview:
		KIA_Play_Actor_Dialogue(kActorLuther, 240);
		KIA_Play_Actor_Dialogue(kActorLance, 200);
		KIA_Play_Actor_Dialogue(kActorLance, 210);
		KIA_Play_Actor_Dialogue(kActorLuther, 260);
		KIA_Play_Actor_Dialogue(kActorLuther, 270);
		break;
	case kCluePhoneCallDektora1:
		KIA_Play_Actor_Dialogue(kActorDektora, 360);
		KIA_Play_Actor_Dialogue(kActorDektora, 380);
		break;
	case kCluePhoneCallDektora2:
		KIA_Play_Actor_Dialogue(kActorMcCoy, 2505);
		KIA_Play_Actor_Dialogue(kActorDektora, 430);
		KIA_Play_Actor_Dialogue(kActorDektora, 440);
		KIA_Play_Actor_Dialogue(kActorMcCoy, 2530);
		KIA_Play_Actor_Dialogue(kActorDektora, 450);
		KIA_Play_Actor_Dialogue(kActorMcCoy, 2535);
		KIA_Play_Actor_Dialogue(kActorDektora, 460);
		KIA_Play_Actor_Dialogue(kActorDektora, 470);
		break;
	case kCluePhoneCallLucy1:
		KIA_Play_Actor_Dialogue(kActorLucy, 590);
		KIA_Play_Actor_Dialogue(kActorLucy, 630);
		break;
	case kCluePhoneCallLucy2:
		KIA_Play_Actor_Dialogue(kActorLucy, 540);
		KIA_Play_Actor_Dialogue(kActorLucy, 550);
		KIA_Play_Actor_Dialogue(kActorMcCoy, 2550);
		KIA_Play_Actor_Dialogue(kActorLucy, 560);
		break;
	case kCluePhoneCallClovis:
		KIA_Play_Actor_Dialogue(kActorClovis, 530);
		KIA_Play_Actor_Dialogue(kActorClovis, 540);
		break;
	case kCluePhoneCallCrystal:
		KIA_Play_Actor_Dialogue(kActorSteele, 700);
		KIA_Play_Actor_Dialogue(kActorSteele, 750);
		KIA_Play_Actor_Dialogue(kActorSteele, 760);
		break;
	case kCluePowerSource:
		KIA_Play_Slice_Model(kModelAnimationPowerSource);
		break;
	case kClueBomb:
		KIA_Play_Slice_Model(kModelAnimationBomb);
		break;
	case kClueDNATyrell:
	case kClueDNASebastian:
	case kClueDNAChew:
	case kClueDNAMoraji:
	case kClueDNALutherLance:
	case kClueDNAMarcus:
		{
			int dnaEvidences = Global_Variable_Query(kVariableDNAEvidence);
			if (dnaEvidences == 1) {
				KIA_Play_Slice_Model(kModelAnimationDNAEvidence01OutOf6);
			} else if (dnaEvidences == 2) {
				KIA_Play_Slice_Model(kModelAnimationDNAEvidence03OutOf6);
			} else if (dnaEvidences == 3) {
				KIA_Play_Slice_Model(kModelAnimationDNAEvidence04OutOf6);
			} else if (dnaEvidences >= 4) {
				KIA_Play_Slice_Model(kModelAnimationDNAEvidenceComplete);
			}
		}
		break;
	case kClueGarterSnake:
		KIA_Play_Slice_Model(kModelAnimationGarterSnake);
		break;
	case kClueSlug:
		KIA_Play_Slice_Model(kModelAnimationSlug);
		break;
	case kClueGoldfish:
		KIA_Play_Slice_Model(kModelAnimationGoldfish);
		break;
	case kClueZubenTalksAboutLucy1:
		KIA_Play_Actor_Dialogue(kActorZuben, 230);
		KIA_Play_Actor_Dialogue(kActorZuben, 240);
		break;
	case kClueZubenTalksAboutLucy2:
		KIA_Play_Actor_Dialogue(kActorZuben, 250);
		KIA_Play_Actor_Dialogue(kActorZuben, 260);
		break;
	case kClueZubensMotive:
		KIA_Play_Actor_Dialogue(kActorZuben, 280);
		KIA_Play_Actor_Dialogue(kActorMcCoy, 7350);
		KIA_Play_Actor_Dialogue(kActorZuben, 290);
		KIA_Play_Actor_Dialogue(kActorZuben, 300);
		KIA_Play_Actor_Dialogue(kActorZuben, 310);
		break;
	case kClueVKDektoraReplicant:
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 420);
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 430);
		break;
	case kClueVKDektoraHuman:
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 420);
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 440);
		break;
	case kClueVKBobGorskyReplicant:
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 420);
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 430);
		break;
	case kClueVKBobGorskyHuman:
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 420);
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 440);
		break;
	case kClueVKLutherLanceReplicant:
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 420);
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 430);
		break;
	case kClueVKLutherLanceHuman:
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 420);
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 440);
		break;
	case kClueVKGrigorianReplicant:
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 420);
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 430);
		break;
	case kClueVKGrigorianHuman:
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 420);
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 440);
		break;
	case kClueVKIzoReplicant:
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 420);
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 430);
		break;
	case kClueVKIzoHuman:
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 420);
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 440);
		break;
	case kClueVKCrazylegsReplicant:
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 420);
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 430);
		break;
	case kClueVKCrazylegsHuman:
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 420);
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 440);
		break;
	case kClueVKRunciterReplicant:
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 420);
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 430);
		break;
	case kClueVKRunciterHuman:
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 420);
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 440);
		break;
	case kClueVKEarlyQReplicant:
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 420);
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 430);
		break;
	case kClueVKEarlyQHuman:
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 420);
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 440);
		break;
	case kClueCrimeSceneNotes:
		KIA_Play_Actor_Dialogue(kActorSteele, 3310);
		KIA_Play_Actor_Dialogue(kActorSteele, 3320);
		if (_vm->_cutContent
		    && (_vm->_language == Common::ES_ESP
		        || _vm->_language == Common::IT_ITA)
		) {
			//
			// in ITA and ESP the 3340 quote is the second half of the sentence starting in previous quote (3330)
			KIA_Play_Actor_Dialogue(kActorSteele, 3330);
			KIA_Play_Actor_Dialogue(kActorSteele, 3340);
		} else if (_vm->_cutContent
		           && _vm->_language == Common::FR_FRA
		) {
			// in FRA the 3340 quote has the full sentence rendering the previous quote (3330) redundant
			// FRA (Restored Content) version needs only 3340
			KIA_Play_Actor_Dialogue(kActorSteele, 3340);
		} else {
			// ENG and DEU and non-restored content versions need only 3330
			// the 3340 quote is *BOOP* in the ENG and DEU versions
			KIA_Play_Actor_Dialogue(kActorSteele, 3330);
		}
		KIA_Play_Actor_Dialogue(kActorSteele, 3350);
		KIA_Play_Actor_Dialogue(kActorSteele, 3360);
		KIA_Play_Actor_Dialogue(kActorSteele, 3370);
		KIA_Play_Actor_Dialogue(kActorSteele, 3380);
		break;
	case kClueGrigorianInterviewA:
		KIA_Play_Actor_Dialogue(kActorSteele, 3390);
		KIA_Play_Actor_Dialogue(kActorSteele, 3400);
		KIA_Play_Actor_Dialogue(kActorSteele, 3410);
		// TODO this line of Grigorian is supposedly interrupted by Steele's following line
		//      maybe implement a way to not wait before the next line is played, similar to Actor_Says_With_Pause()
		//       (look into tick() for kia.cpp)
		KIA_Play_Actor_Dialogue(kActorGrigorian, 1260);
		KIA_Play_Actor_Dialogue(kActorSteele, 3420);
		KIA_Play_Actor_Dialogue(kActorSteele, 3430);
		KIA_Play_Actor_Dialogue(kActorSteele, 3440);
		KIA_Play_Actor_Dialogue(kActorGrigorian, 1270);
		KIA_Play_Actor_Dialogue(kActorSteele, 3450);
		KIA_Play_Actor_Dialogue(kActorSteele, 3460);
		KIA_Play_Actor_Dialogue(kActorGrigorian, 1280);
		KIA_Play_Actor_Dialogue(kActorSteele, 3470);
		KIA_Play_Actor_Dialogue(kActorGrigorian, 1300);
		KIA_Play_Actor_Dialogue(kActorGrigorian, 1310);
		if (_vm->_cutContent
		    && (_vm->_language == Common::ES_ESP
		        || _vm->_language == Common::IT_ITA)
		) {
			//
			// in ITA and ESP the 3490 quote is the second half of the sentence starting in previous quote (3480)
			KIA_Play_Actor_Dialogue(kActorSteele, 3480);
			KIA_Play_Actor_Dialogue(kActorSteele, 3490);
		} else {
			// the 3490 quote is *BOOP* in the ENG and DEU versions
			// the 3490 quote is also redundant in FRA version, since it's only the first half of the previous quote (3480)
			KIA_Play_Actor_Dialogue(kActorSteele, 3480);
		}
		KIA_Play_Actor_Dialogue(kActorSteele, 3500);
		KIA_Play_Actor_Dialogue(kActorGrigorian, 1320);
		KIA_Play_Actor_Dialogue(kActorGrigorian, 1330);
		KIA_Play_Actor_Dialogue(kActorSteele, 3510);
		KIA_Play_Actor_Dialogue(kActorGrigorian, 1340);
		KIA_Play_Actor_Dialogue(kActorSteele, 3520);
		KIA_Play_Actor_Dialogue(kActorGrigorian, 1350);
		KIA_Play_Actor_Dialogue(kActorSteele, 3530);
		KIA_Play_Actor_Dialogue(kActorSteele, 3540);
		break;
	case kClueGrigorianInterviewB1:
		// Izo is a Replicant
		KIA_Play_Actor_Dialogue(kActorSteele, 3550); // Describe them to me again.
		KIA_Play_Actor_Dialogue(kActorGrigorian, 1360); // Just the Rastafarian fellow. And that-- that friend of his. A slim blond man, Asian. With-- with a ponytail.
		KIA_Play_Actor_Dialogue(kActorGrigorian, 1370);
		if (_vm->_cutContent) {
			KIA_Play_Actor_Dialogue(kActorSteele, 3620); // Let me ask you a question.
			KIA_Play_Actor_Dialogue(kActorSteele, 3630);
			KIA_Play_Actor_Dialogue(kActorGrigorian, 1380);
			KIA_Play_Actor_Dialogue(kActorSteele, 3640);  // such as
			KIA_Play_Actor_Dialogue(kActorGrigorian, 1390);
		}
		KIA_Play_Actor_Dialogue(kActorSteele, 3560); // Bravo, Spencer.
		KIA_Play_Actor_Dialogue(kActorSteele, 3570);
		break;
	case kClueGrigorianInterviewB2:
		// Izo is a human
		KIA_Play_Actor_Dialogue(kActorSteele, 3580); // Describe them for me again.
		KIA_Play_Actor_Dialogue(kActorGrigorian, 1400); // There was that Rastafarian fellow and one of our ex members showed up.
		KIA_Play_Actor_Dialogue(kActorSteele, 3590);
		KIA_Play_Actor_Dialogue(kActorGrigorian, 1410); // A Japanese man named Izo.
		KIA_Play_Actor_Dialogue(kActorSteele, 3600);
		KIA_Play_Actor_Dialogue(kActorGrigorian, 1420); // We're a pacifist organization and we have rules
		KIA_Play_Actor_Dialogue(kActorGrigorian, 1430); // Izo refused to check in his samurai sword
		KIA_Play_Actor_Dialogue(kActorSteele, 3610);
		KIA_Play_Actor_Dialogue(kActorGrigorian, 1440);
		KIA_Play_Actor_Dialogue(kActorSteele, 3620); // Let me ask you a question.
		KIA_Play_Actor_Dialogue(kActorSteele, 3630);
		KIA_Play_Actor_Dialogue(kActorGrigorian, 1450);
		KIA_Play_Actor_Dialogue(kActorSteele, 3640);  // such as
		KIA_Play_Actor_Dialogue(kActorGrigorian, 1460);
		KIA_Play_Actor_Dialogue(kActorSteele, 3650);
		break;
	case kClueAnimalMurderSuspect:
		KIA_Play_Photograph(7);
		break;
	case kClueMilitaryBoots:
		KIA_Play_Photograph(8);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 4110);
		break;
	case kClueOuterDressingRoom:
		KIA_Play_Photograph(15);
		break;
	case kCluePhotoOfMcCoy1:
		KIA_Play_Photograph(17);
		break;
	case kCluePhotoOfMcCoy2:
		KIA_Play_Photograph(18);
		break;
	case kClueEarlyQAndLucy:
		KIA_Play_Photograph(1);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 4260);
		break;
	case kClueClovisflowers:
		KIA_Play_Photograph(3);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 4230);
		break;
	case kClueLucyWithDektora:
		KIA_Play_Photograph(2);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 4040);
		break;
	case kClueWomanInAnimoidRow:
		KIA_Play_Photograph(21);
		break;
	case kClueScorpions:
		KIA_Play_Photograph(22);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 4180);
		break;
	case kClueStrangeScale2:
		KIA_Play_Photograph(23);
		break;
	case kClueChinaBarSecurityCamera:
		KIA_Play_Photograph(24);
		break;
	case kClueIzo:
		KIA_Play_Photograph(26);
		break;
	case kClueGuzza:
		KIA_Play_Photograph(27);
		break;
	case kClueChinaBarSecurityDisc:
		KIA_Play_Slice_Model(kModelAnimationVideoDisc);
		break;
	case kClueScorpionbox:
		KIA_Play_Photograph(0);
		break;
	case kClueTyrellSecurityPhoto:
		KIA_Play_Photograph(28);
		break;
	case kClueChinaBar:
		KIA_Play_Photograph(19);
		break;
	case kCluePlasticExplosive:
		KIA_Play_Photograph(32);
		break;
	case kClueDogCollar2:
		KIA_Play_Photograph(30);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 4160);
		break;
	case kClueKingstonKitchenBox2:
		KIA_Play_Photograph(29);
		if (Query_Difficulty_Level() == kGameDifficultyEasy) {
			KIA_Play_Actor_Dialogue(kActorVoiceOver, 4140);
		} else {
			KIA_Play_Actor_Dialogue(kActorVoiceOver, 4150);
		}
		break;
	case kClueCrystalsCigarette:
		KIA_Play_Slice_Model(kModelAnimationCrystalsCigarette);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 1770);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 1150);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 1180);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 1190);
		break;
	case kClueSpinnerKeys:
		KIA_Play_Slice_Model(kModelAnimationSpinnerKeys);
		break;
	case kClueExpertBomber:
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 730);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 740);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 750);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 760);
		break;
	case kClueAmateurBomber:
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 670);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 680);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 700);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 710);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 720);
		break;
	case kClueVKLucyReplicant:
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 420);
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 430);
		break;
	case kClueVKLucyHuman:
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 420);
		KIA_Play_Actor_Dialogue(kActorAnsweringMachine, 440);
		break;
	case kClueLucyInterview:
		KIA_Play_Actor_Dialogue(kActorMcCoy, 1645);
		KIA_Play_Actor_Dialogue(kActorLucy, 240);
		KIA_Play_Actor_Dialogue(kActorLucy, 250);
		KIA_Play_Actor_Dialogue(kActorMcCoy, 1675);
		KIA_Play_Actor_Dialogue(kActorLucy, 260);
		KIA_Play_Actor_Dialogue(kActorLucy, 270);
		break;
	case kClueMoonbusReflection:
		KIA_Play_Photograph(35);
		break;
	case kClueMcCoyAtMoonbus:
		KIA_Play_Photograph(36);
		if (_vm->_cutContent) {
			if (Actor_Clue_Query(kActorMcCoy, kClueMoonbusReflection)) {
				KIA_Play_Actor_Dialogue(kActorVoiceOver, 4250);
			} else {
				KIA_Play_Actor_Dialogue(kActorVoiceOver, 4010);
				KIA_Play_Actor_Dialogue(kActorVoiceOver, 4020);
			}
		} else {
			// original re-uses the "That can't be me" from the ESPER
			KIA_Play_Actor_Dialogue(kActorVoiceOver, 4240);
		}
		break;
	case kClueClovisAtMoonbus:
		KIA_Play_Photograph(37);
		KIA_Play_Actor_Dialogue(kActorVoiceOver, 4220);
		break;
	case kClueSadikAtMoonbus:
		KIA_Play_Photograph(38);
		break;
	case kClueRachaelInterview:
		KIA_Play_Actor_Dialogue(kActorMcCoy, 5365);
		KIA_Play_Actor_Dialogue(kActorRachael, 600);
		KIA_Play_Actor_Dialogue(kActorMcCoy, 5370);
		KIA_Play_Actor_Dialogue(kActorRachael, 610);
		break;
	case kClueTyrellInterview:
		KIA_Play_Actor_Dialogue(kActorTyrell, 0);
		KIA_Play_Actor_Dialogue(kActorTyrell, 10);
		KIA_Play_Actor_Dialogue(kActorTyrell, 20);
		KIA_Play_Actor_Dialogue(kActorTyrell, 30);
		break;
	case kClueRuncitersConfession1:
		KIA_Play_Actor_Dialogue(kActorRunciter, 630);
		KIA_Play_Actor_Dialogue(kActorRunciter, 640);
		KIA_Play_Actor_Dialogue(kActorRunciter, 650);
		break;
	default:
		break;
	}
}

} // End of namespace BladeRunner
