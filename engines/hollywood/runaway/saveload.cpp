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

#include "hollywood/runaway/saveload.h"
#include "hollywood/runaway/scene_data.h"

namespace Hollywood {

void RunawaySaveState::sync(Common::Serializer &s) {
	s.syncAsUint16LE(scene);
	s.syncAsUint16LE(x);
	s.syncAsUint16LE(y);
	s.syncAsUint16LE(camera);
	s.syncAsByte(facing);
	s.syncAsByte(lidOpen);
	s.syncAsByte(unplugged);
	s.syncAsByte(windowClean);
	s.syncAsByte(coolerMoved);
	s.syncAsByte(coolerInspected);
	s.syncAsByte(peeked);
	s.syncAsByte(ice);
	s.syncAsByte(hatch);
	s.syncBytes(pickedUp, sizeof(pickedUp));
	s.syncBytes(examined, sizeof(examined));
	s.syncAsByte(itemCount);
	s.syncBytes(items, 26);
	s.syncAsByte(selected);
	s.syncAsByte(cloth);
	s.syncAsByte(inventoryOpen);
	s.syncAsByte(verb);
	s.syncAsUint32LE(randomSeed);
	s.syncAsUint32LE(playTime);
	s.syncAsByte(game, 2);
	s.syncAsUint16LE(cameraY, 2);
	s.syncAsByte(upperFloor, 2);
	s.syncAsByte(books, 2);
	s.syncAsByte(recorder, 2);
	s.syncAsByte(moneyIdentified, 2);
	s.syncAsByte(safeOpen, 2);
	s.syncAsByte(varnishTaken, 2);
	s.syncAsByte(talcTaken, 2);
	s.syncAsByte(coffeeTaken, 2);
	s.syncAsByte(keyCount, 2);
	s.syncAsByte(hospital.glassTaken, 3);
	s.syncAsByte(hospital.pillsTaken, 3);
	s.syncAsByte(hospital.sheetTaken, 3);
	s.syncAsByte(hospital.bagTaken, 3);
	s.syncAsByte(hospital.exitMapRead, 3);
	s.syncAsByte(hospital.markerTaken, 4);
	s.syncAsByte(hospital.alcohol, 4);
	s.syncAsByte(water, 4);
	s.syncAsByte(hospital.window, 5);
	s.syncAsByte(hospital.chartReplaced, 5);
	s.syncAsByte(hospital.chartMoved, 5);
	s.syncAsByte(hospital.decoy, 5);
	s.syncAsByte(hospital.attacked, 5);
	s.syncAsByte(hospital.medicineRead, 5);
	s.syncAsByte(hospital.storeroomVisited, 5);
	s.syncAsByte(hospital.syringesSeen, 5);
	s.syncAsByte(hospital.sprayTaken, 5);
	s.syncAsByte(hospital.book, 5);
	s.syncAsByte(hospital.blankChart, 5);
	s.syncAsByte(hospital.headTaken, 5);
	s.syncAsByte(hospital.pillowsTaken, 5);
	s.syncAsByte(markerInk, 5);
	s.syncAsByte(syringeFilled, 5);
	s.syncAsByte(chartWritten, 5);
	s.syncAsByte(headWig, 5);
	s.syncAsByte(museum.susan, 6);
	s.syncAsByte(museum.restoration, 6);
	s.syncAsByte(museum.laserUpgraded, 6);
	s.syncAsByte(museum.laserSeen, 6);
	s.syncAsByte(museum.shelvesSeen, 6);
	s.syncAsByte(museum.susanMet, 6);
	s.syncAsByte(museum.labVisited, 6);
	s.syncAsByte(museum.shelfBooksSeen, 6);
	s.syncAsByte(museum.safeUsed, 6);
	s.syncAsByte(museum.willyMet, 6);
	s.syncAsByte(museum.batteryRequested, 6);
	s.syncAsByte(museum.coffeeMachine, 6);
	s.syncAsByte(museum.coffeeSeen, 6);
	s.syncAsByte(museum.basketSeen, 6);
	s.syncAsByte(museum.needsPackage, 6);
	s.syncAsByte(museum.needsGroundCoffee, 6);
	s.syncAsByte(museum.willyInLab, 6);
	s.syncAsByte(museum.keypad, 6);
	s.syncAsByte(museum.sawCode, 6);
	s.syncAsByte(museum.phoneUsed, 6);
	s.syncAsByte(museum.voiceLockSeen, 6);
	s.syncAsByte(museum.chamberOpen, 6);
	s.syncAsByte(museum.maskTaken, 6);
	s.syncAsByte(museum.keyTaken, 6);
	s.syncAsByte(museum.ladleTaken, 6);
	s.syncAsByte(museum.cabinetSeen, 6);
	s.syncAsByte(museum.scannerSeen, 6);
	s.syncAsByte(museum.nitrogenSeen, 6);
	s.syncAsByte(museum.microscopeSeen, 6);
	s.syncAsByte(museum.talcSeen, 6);
	s.syncAsByte(museum.varnishSeen, 6);
	s.syncAsByte(museum.mayanObjectsSeen, 6);
	s.syncAsByte(museum.coffeeBeansTaken, 6);
	s.syncAsByte(museum.buttonSeen, 6);
	s.syncAsByte(museum.ownVoiceTried, 6);
	s.syncAsByte(museum.analysisDoorSeen, 6);
	s.syncAsByte(museum.leftLabOnce, 6);
	s.syncAsByte(museum.keySeen, 6);
	s.syncBytes(museum.exhibitSeen, sizeof(museum.exhibitSeen), 6);
	s.syncAsUint16LE(museum.keypadDigits, 6);
	s.syncAsUint32LE(museum.chamberTime, 6);
	s.syncAsByte(museumItems.businessCardExamined, 6);
	s.syncAsByte(museumItems.restoredCrucifix, 6);
	s.syncAsByte(museumItems.voiceRecorded, 6);
	s.syncAsByte(museumItems.coffee, 6);
	s.syncAsByte(museumItems.frozenBattery, 6);
	s.syncAsByte(museumItems.rubyRemoved, 6);
	s.syncAsByte(museumItems.recorderBattery, 6);
	s.syncAsByte(museumItems.batteryInLadle, 6);
	s.syncAsByte(museumItems.packageFilled, 6);
	s.syncBytes(desert.visited, sizeof(desert.visited), 7);
	s.syncAsByte(desert.escapedCabin, 7);
	s.syncAsByte(desert.hatchSeen, 7);
	s.syncAsByte(desert.bucketTaken, 7);
	s.syncAsByte(desert.helmetTaken, 7);
	s.syncAsByte(desert.beltTaken, 7);
	s.syncAsByte(desert.beltSeen, 7);
	s.syncAsByte(desert.gun, 7);
	s.syncAsByte(desert.gunSeen, 7);
	s.syncAsByte(desert.mariolaHelped, 7);
	s.syncAsByte(desert.ballTaken, 7);
	s.syncAsByte(desert.tanningOil, 7);
	s.syncAsByte(desert.lulaHelped, 7);
	s.syncAsByte(desert.busPosterSeen, 7);
	s.syncAsByte(desert.lulaCalled, 7);
	s.syncAsByte(desert.lulaRefused, 7);
	s.syncAsByte(desert.dynamitePlaced, 7);
	s.syncAsByte(desert.oilSeen, 7);
	s.syncAsByte(desert.antsDistracted, 7);
	s.syncAsByte(desert.dynamite, 7);
	s.syncAsByte(desert.shedTried, 7);
	s.syncAsByte(desert.trainEntered, 7);
	s.syncAsByte(desert.barrel, 7);
	s.syncAsByte(desert.screwSeen, 7);
	s.syncAsByte(desert.screwTaken, 7);
	s.syncAsByte(desert.bucket, 7);
	s.syncAsByte(desert.trainDoorOpen, 7);
	s.syncAsByte(desert.powderNeeded, 7);
	s.syncAsByte(desert.barnDoor, 7);
	s.syncAsByte(desert.footrest, 7);
	s.syncAsByte(desert.guard, 7);
	s.syncAsByte(desert.insideBarn, 7);
	s.syncAsByte(desert.beerSeen, 7);
	s.syncAsByte(desert.peanutButter, 7);
	s.syncAsByte(desert.motorcycleSeen, 7);
	s.syncAsByte(desert.footrestSeen, 7);
	s.syncAsByte(desert.lulaDistractedGuard, 7);
	s.syncAsByte(desert.guardWeaknessKnown, 7);
	s.syncAsByte(desert.busDoor, 7);
	s.syncAsByte(desert.lipstickShelf, 7);
	s.syncAsByte(desert.keyTaken, 7);
	s.syncAsByte(desert.glassesTaken, 7);
	s.syncAsByte(desert.vacuum, 7);
	s.syncAsByte(desert.crank, 7);
	s.syncAsByte(desert.necklacesSeen, 7);
	s.syncAsByte(desert.ringsSeen, 7);
	s.syncAsByte(desert.fridgeOpened, 7);
	s.syncAsByte(desert.ventCleaned, 7);
	s.syncAsByte(desert.ventSeen, 7);
	s.syncAsByte(desert.needle, 7);
	s.syncAsByte(desert.fridgeSeen, 7);
	s.syncAsByte(desert.carlaMet, 7);
	s.syncAsByte(desert.carlaStory, 7);
	s.syncAsByte(desert.planKnown, 7);
	s.syncAsUint32LE(desert.carlaTopics, 7);
	s.syncAsUint32LE(desert.lulaTopics, 7);
	s.syncAsByte(desertItems.antTrickKnown, 7);
	s.syncAsByte(desertItems.helmet, 7);
	s.syncAsByte(desertItems.lipstick, 7);
	s.syncAsByte(desertItems.lipsticks, 7);
	s.syncAsByte(desertItems.belt, 7);
	s.syncAsByte(desertItems.ball, 7);
	s.syncAsByte(desertItems.glasses, 7);
	s.syncAsByte(desertItems.pill, 7);
	s.syncAsByte(desertItems.bucket, 7);
	s.syncAsByte(desertItems.gunReady, 7);
	s.syncAsByte(desertItems.bombReady, 7);
	s.syncAsByte(desertItems.helicopterReady, 7);
	s.syncAsByte(town.arrived, 8);
	s.syncAsByte(town.streetVisited, 8);
	s.syncAsByte(town.strapSeen, 8);
	s.syncAsByte(town.sushiMet, 8);
	s.syncAsByte(town.mineMapKnown, 8);
	// Preserve the original inventory layout when reading older saves.
	s.syncBytes(items + 26, sizeof(items) - 26, 9);
	s.syncAsByte(town.bankVisited, 9);
	s.syncAsByte(town.bankShelves, 9);
	s.syncAsByte(town.bankStampTaken, 9);
	s.syncAsByte(town.basementKnown, 9);
	s.syncAsByte(town.bodySeen, 9);
	s.syncAsByte(town.doctorKnown, 9);
	s.syncAsByte(town.firewoodTaken, 9);
	s.syncAsByte(town.cellOpen, 9);
	s.syncAsByte(town.bagSearched, 9);
	s.syncAsByte(town.bucketSeen, 9);
	s.syncAsByte(town.bottleSeen, 9);
	s.syncAsByte(town.cellRefusals, 9);
	s.syncAsByte(town.safeSeen, 9);
	s.syncAsByte(town.oscarTobacco, 9);
	s.syncAsByte(town.stethoscopeTaken, 9);
	s.syncAsByte(town.linimentTaken, 9);
	s.syncAsByte(town.locomotiveVisited, 10);
	s.syncAsByte(town.locomotiveKey, 10);
	s.syncAsByte(town.waterTank, 10);
	s.syncAsByte(town.cabVisited, 10);
	s.syncAsByte(town.cabExited, 10);
	s.syncAsByte(town.boiler, 10);
	s.syncAsByte(town.steamLever, 10);
	s.syncAsByte(town.pressureWheel, 10);
	s.syncAsByte(town.drainSeen, 10);
	s.syncAsByte(townItems.oilCanFull, 10);
	s.syncAsByte(townItems.bottleContents, 10);
	s.syncAsByte(town.craterSeen, 10);
	s.syncAsByte(town.mineVisited, 11);
	s.syncAsByte(town.oilCan, 11);
	s.syncAsByte(town.rockMoved, 11);
	s.syncAsByte(town.houseVisited, 11);
	s.syncAsByte(town.stone, 11);
	s.syncAsByte(town.vessel, 11);
	s.syncAsByte(town.oscarMet, 11);
	s.syncAsByte(town.oscarHelpRefused, 11);
	s.syncAsByte(town.rawTobaccoRefused, 11);
	s.syncAsByte(town.saloonEntranceVisited, 12);
	s.syncAsByte(town.saloonDoorKnown, 12);
	s.syncAsByte(town.saloonVisited, 12);
	s.syncAsByte(town.flowerPotTaken, 12);
	s.syncAsByte(town.gardeningShed, 12);
	s.syncAsByte(town.tobaccoLeavesTaken, 12);
	s.syncAsByte(town.artichokeSeen, 12);
	s.syncAsByte(town.carnivorousPlantSeen, 12);
	s.syncAsByte(town.craterVisited, 13);
	s.syncAsByte(town.joshuaMet, 13);
	s.syncAsByte(town.campLeft, 13);
	s.syncAsByte(town.strapDelivered, 13);
	s.syncAsByte(town.wrenchDelivered, 13);
	s.syncAsByte(town.wrenchNeeded, 13);
	s.syncAsByte(town.fuelNeeded, 13);
	s.syncAsByte(town.machineKnown, 13);
	s.syncAsByte(town.rotorKnown, 13);
	s.syncAsByte(town.batteryKnown, 13);
	s.syncAsByte(town.helmetKnown, 13);
	s.syncAsByte(town.dimensionalDoorKnown, 13);
	s.syncAsByte(town.strapIdea, 13);
	s.syncAsByte(townItems.strap, 13);
	s.syncAsByte(town.sandingBlock, 14);
	s.syncAsByte(town.wrenchThrown, 14);
	s.syncAsByte(town.wrenchFound, 14);
	s.syncAsByte(town.wrenchInTroughSeen, 14);
	s.syncAsByte(town.kevinAway, 14);
	s.syncAsByte(town.craterFloorVisited, 14);
	s.syncAsByte(town.joshuaGone, 14);
	s.syncAsByte(town.helmetTaken, 14);
	s.syncAsByte(town.kevinMet, 14);
	s.syncAsByte(town.catapult, 14);
	s.syncAsByte(town.statueLowered, 14);
	s.syncAsByte(town.blowtorchTaken, 14);
	s.syncAsByte(town.sculptureTraded, 14);
	s.syncAsByte(town.craneKnown, 14);
	s.syncAsByte(town.craneTried, 14);
	s.syncAsByte(town.wrenchPermission, 14);
	s.syncAsByte(town.fuelTradeKnown, 14);
	s.syncAsByte(town.waterPermission, 14);
	s.syncAsByte(town.waterRequested, 14);
	s.syncAsByte(town.hopiDiscussed, 14);
	s.syncAsByte(town.roughSculptureRefused, 14);
	s.syncAsByte(town.polishedSculptureRefused, 14);
	s.syncAsByte(town.oscarStrengthKnown, 14);
	s.syncAsByte(town.basementVisited, 14);
	s.syncAsByte(town.bankImpactSeen, 14);
	s.syncAsByte(town.safeOpened, 14);
	s.syncAsByte(town.stethoscopeTried, 14);
	s.syncAsByte(town.safeDialVisited, 14);
	s.syncAsByte(town.safeDial, 14);
	s.syncAsByte(town.safeProgress, 14);
	s.syncAsByte(town.machinePowered, 14);
	s.syncAsByte(town.melodyTried, 14);
	s.syncAsByte(town.campingGearTaken, 14);
	s.syncAsByte(townItems.sculpture, 14);
	s.syncAsByte(townItems.tube30, 14);
	s.syncAsByte(townItems.tube50, 14);
	s.syncAsByte(townItems.mortar, 14);
	s.syncAsByte(townItems.mortarIdea, 14);
	s.syncAsByte(hopi.cliffVisited, 15);
	s.syncAsByte(hopi.skeletonSeen, 15);
	s.syncAsByte(hopi.nail, 15);
	s.syncAsByte(hopi.timber, 15);
	s.syncAsByte(hopi.femurTaken, 15);
	s.syncAsByte(hopi.rockTried, 15);
	s.syncAsByte(hopi.ginaFound, 15);
	s.syncAsByte(hopi.villageLevel, 15);
	s.syncAsByte(hopi.rope, 15);
	s.syncAsByte(hopi.legSplinted, 15);
	s.syncAsByte(hopi.cryptVisited, 15);
	s.syncAsByte(hopiItems.toolHandle, 15);
	s.syncAsByte(hopi.canyonVisited, 16);
	s.syncAsByte(hopi.cryptEntranceVisited, 16);
	s.syncAsByte(hopi.keyholeBlockedKnown, 16);
	s.syncAsByte(hopi.bushSeen, 16);
	s.syncAsByte(hopi.cryptDoor, 16);
	s.syncAsByte(hopi.statueVisited, 17);
	s.syncAsByte(hopi.tomahawk, 17);
	s.syncAsByte(hopi.leftCrossSeen, 17);
	s.syncAsByte(hopi.rightCrossSeen, 17);
	s.syncAsByte(finale.doritaMet, 18);
	s.syncAsByte(finale.mediumDiscussed, 18);
	s.syncAsByte(finale.mediumTests, 18);
	s.syncAsByte(finale.seanceComplete, 18);
	s.syncAsByte(finale.oscarAway, 18);
	s.syncAsByte(finale.gangsters, 18);
	s.syncAsByte(finale.trailerFound, 18);
	s.syncAsByte(finale.moneyFound, 18);
	s.syncAsByte(finale.nunsHabit, 18);
	s.syncAsByte(finale.recordingPlan, 18);
	s.syncAsByte(finale.recordings, 18);
	s.syncAsByte(finale.ginaUpdates, 18);
	s.syncAsByte(finale.ginaVisitPending, 18);
	s.syncAsByte(finale.gangstersMet, 19);
	s.syncAsByte(finale.gangsterBag, 19);
	s.syncAsByte(finale.scalpelNeeded, 20);
	s.syncAsByte(finale.streetVisited, 20);
	s.syncAsByte(finale.sheriffThanked, 20);
	s.syncAsByte(finale.sushiVisited, 20);
	s.syncAsByte(finale.bankResearch, 20);
	s.syncAsByte(finale.voicePlanPending, 20);
	s.syncAsByte(finale.voiceEditing, 20);
	s.syncAsByte(finale.departure, 20);
	s.syncAsByte(finale.rutgerMet, 21);
	s.syncAsByte(finale.rutgerTranceDiscussed, 21);
	s.syncAsByte(finale.rutgerBrewDiscussed, 21);
	s.syncAsByte(finale.rutgerPlantHint, 21);
	s.syncAsByte(finale.rutgerBrew, 21);
	s.syncAsByte(finale.saloonVisited, 22);
	s.syncAsByte(finale.locomotiveBadge, 22);
	s.syncAsByte(finale.saturnMet, 23);
	s.syncAsByte(finale.craterVisited, 23);
	s.syncAsByte(finale.hotelVisited, 24);
	s.syncAsByte(finale.rifle, 24);
	s.syncAsByte(finale.pokerTaken, 24);
	s.syncAsByte(finale.bellRings, 24);
	s.syncAsByte(finale.sushiTruthTold, 24);
	s.syncAsByte(finale.hackerKnown, 24);
	s.syncAsByte(finale.trailerDiscussed, 24);
	s.syncAsByte(finale.gangstersDiscussed, 24);
	s.syncAsByte(finale.bagDelivered, 24);
	s.syncAsByte(finale.movieFanKnown, 24);
	s.syncAsByte(finaleItems.scalpel, 24);
	s.syncAsByte(finaleItems.heatNeeded, 24);
	s.syncAsByte(finaleItems.podsOpened, 24);
	s.syncAsByte(finale.trailerVisited, 25);
	s.syncAsByte(finale.trailerDoor, 25);
	s.syncAsByte(finale.trailerDoorTried, 25);
	s.syncAsByte(finale.bankBrochureTaken, 25);
	s.syncAsByte(finale.trailerWarnings, 25);
	s.syncAsByte(finale.trailerInteriorVisited, 25);
	s.syncAsByte(finale.ginaVisited, 26);
	s.syncAsByte(finale.ginaMediumDiscussed, 26);
	s.syncAsByte(museum.officeVisited, 27);
	s.syncAsByte(museum.exhibitionVisited, 27);
	s.syncAsByte(museum.phonePlanExplained, 27);
}

byte RunawaySaveState::chapter() const {
	switch (scene / 1000) {
	case 1: case 2: return 1;
	case 4: return 2;
	case 5: return 3;
	case 6: return 4;
	case 7: return 5;
	case 8: case 9: return 6;
	default: return 0;
	}
}

bool RunawaySaveState::valid() const {
	if (game > kRunawayFullGame || !chapter() || x >= 32768 || y >= 32768 ||
			camera >= 32768 || cameraY >= 32768 || facing >= 8 ||
			lidOpen > 1 || unplugged > 1 || windowClean > 1 || coolerMoved > 1 ||
			coolerInspected > 1 || peeked > 1 || ice > 2 || hatch > 3 ||
			itemCount > sizeof(items) || selected > sizeof(items) || cloth > 2 || inventoryOpen > 1 || verb > 2)
		return false;
	if ((game == kRunawayCabinDemo || (game == kRunawayFullGame && scene >= 5030 && scene <= 5032)) &&
			(scene < 5030 || scene > (game == kRunawayCabinDemo ? 5030 : 5032) || x >= 1376 || y >= 600 || camera > 352 || cameraY))
		return false;
	if (game == kRunawayFullGame && ((chapter() == 3 && (scene < 5030 || scene > 5032)) || chapter() >= 4)) {
		const RunawayRoom *room = findRunawayRoom(scene);
		if (!room || x >= room->width || y >= room->height || camera > room->width - 1024 || cameraY > room->height - 600)
			return false;
	}
	if (upperFloor > 1 || books > 2 || recorder > 2 || moneyIdentified > 1 || safeOpen > 1 ||
			varnishTaken > 1 || talcTaken > 1 || coffeeTaken > 1 || keyCount > 4)
		return false;
	if (game == kRunawayMuseumDemo || (game == kRunawayFullGame && chapter() == 2)) {
		const RunawayRoom *room = findRunawayRoom(scene);
		if (!room || scene < 4000 || x >= room->width || y >= room->height || camera > room->width - 1024 ||
				cameraY > room->height - 600)
			return false;
	}
	if (hospital.glassTaken > 1 || hospital.pillsTaken > 1 || hospital.sheetTaken > 1 ||
			hospital.bagTaken > 1 || hospital.exitMapRead > 1 || hospital.markerTaken > 1 || hospital.alcohol > 2 || water > 2)
		return false;
	if (hospital.window > 2 || hospital.chartReplaced > 1 || hospital.chartMoved > 1 ||
			hospital.decoy > 1 || hospital.attacked > 1 || hospital.medicineRead > 1 ||
			hospital.storeroomVisited > 1 || hospital.syringesSeen > 1 || hospital.sprayTaken > 1 ||
			hospital.book > 2 || hospital.blankChart > 2 || hospital.headTaken > 1 || hospital.pillowsTaken > 1 ||
			markerInk > 3 || syringeFilled > 1 || chartWritten > 1 || headWig > 1)
		return false;
	if (game == kRunawayFullGame && chapter() == 1) {
		const RunawayRoom *room = findRunawayRoom(scene);
		if (!room || (room->number != kHospitalBedroom && room->number != kHospitalStoreroom && room->number != kHospitalBathroom) ||
				x >= room->width || y >= room->height || cameraY ||
				camera < (room->number == kHospitalBedroom ? 156 : 0) || camera > room->width - 1024)
			return false;
	}
	if (museum.susan > 3 || museum.restoration > 2 || museum.laserUpgraded > 1 ||
			museum.laserSeen > 1 || museum.shelvesSeen > 1 || museum.susanMet > 1 ||
			museum.labVisited > 1 || museum.shelfBooksSeen > 1 || museum.safeUsed > 1 ||
			museum.officeVisited > 1 || museum.exhibitionVisited > 1 || museum.phonePlanExplained > 1 ||
			museum.willyMet > 2 || museum.batteryRequested > 1 || museum.coffeeMachine > 2 ||
			museum.coffeeSeen > 1 || museum.basketSeen > 1 || museum.needsPackage > 1 ||
			museum.needsGroundCoffee > 1 || museum.willyInLab > 2 || museum.keypad > 4 ||
			museum.sawCode > 2 || museum.phoneUsed > 1 || museum.voiceLockSeen > 1 ||
			museum.chamberOpen > 1 || museum.maskTaken > 1 || museum.keyTaken > 1 ||
			museum.ladleTaken > 1 || museum.cabinetSeen > 1 || museum.scannerSeen > 1 ||
			museum.nitrogenSeen > 1 || museum.microscopeSeen > 1 || museum.talcSeen > 1 ||
			museum.varnishSeen > 1 || museum.mayanObjectsSeen > 1 || museum.coffeeBeansTaken > 1 ||
			museum.buttonSeen > 1 || museum.ownVoiceTried > 1 || museum.analysisDoorSeen > 1 || museum.leftLabOnce > 1 || museum.keySeen > 1 ||
			museum.exhibitSeen[0] > 1 || museum.exhibitSeen[1] > 1 || museum.exhibitSeen[2] > 1 ||
			museum.keypadDigits > 9999 || museum.chamberTime > 30000 || museumItems.businessCardExamined > 1 ||
			museumItems.restoredCrucifix > 1 || museumItems.voiceRecorded > 1 || museumItems.coffee > 2 ||
			museumItems.frozenBattery > 1 || museumItems.rubyRemoved > 1 || museumItems.recorderBattery > 4 ||
			museumItems.batteryInLadle > 1 || museumItems.packageFilled > 1)
		return false;
	if (desert.escapedCabin > 1 || desert.hatchSeen > 1 || desert.bucketTaken > 1 ||
			desert.helmetTaken > 1 || desert.beltTaken > 1 || desert.beltSeen > 1 || desert.gun > 2 || desert.gunSeen > 1 ||
			desert.mariolaHelped > 1 || desert.ballTaken > 1 || desert.tanningOil > 2 || desert.lulaHelped > 1 ||
			desert.busPosterSeen > 1 || desert.lulaCalled > 1 || desert.lulaRefused > 1 ||
			desert.dynamitePlaced > 1 || desert.oilSeen > 1 || desert.antsDistracted > 1 || desert.dynamite > 2 ||
			desert.shedTried > 1 || desert.trainEntered > 1 || desert.barrel > 2 || desert.screwSeen > 1 ||
			desert.screwTaken > 1 || desert.bucket > 3 || desert.trainDoorOpen > 1 || desert.powderNeeded > 1 ||
			desert.barnDoor > 2 || desert.footrest > 2 || desert.guard > 2 || desert.insideBarn > 1 || desert.beerSeen > 1 ||
			desert.peanutButter > 2 || desert.motorcycleSeen > 1 || desert.footrestSeen > 1 ||
			desert.lulaDistractedGuard > 1 || desert.guardWeaknessKnown > 1 ||
			desert.busDoor > 3 || desert.lipstickShelf > 3 || desert.keyTaken > 1 || desert.glassesTaken > 1 ||
			desert.vacuum > 2 || desert.crank > 2 || desert.necklacesSeen > 1 || desert.ringsSeen > 1 ||
			desert.fridgeOpened > 1 || desert.ventCleaned > 1 || desert.ventSeen > 1 || desert.needle > 2 || desert.fridgeSeen > 1 ||
			desert.carlaMet > 1 || desert.carlaStory > 1 || desert.planKnown > 1 ||
			desertItems.antTrickKnown > 1 || desertItems.helmet > 4 || desertItems.lipstick > 1 || desertItems.lipsticks > 1 ||
			desertItems.belt > 2 || desertItems.ball > 2 || desertItems.glasses > 2 || desertItems.pill > 1 ||
			desertItems.bucket > 2 || desertItems.gunReady > 1 || desertItems.bombReady > 1 || desertItems.helicopterReady > 1)
		return false;
	for (uint i = 0; i < sizeof(desert.visited); ++i) {
		if (desert.visited[i] > 1)
			return false;
	}
	if ((coolerMoved && ice != 2) || ((hatch != 0) != (coolerMoved != 0)))
		return false;
	for (uint i = 0; i < sizeof(pickedUp); ++i) {
		if (pickedUp[i] > 1 || examined[i] > 1)
			return false;
	}
	bool owned[33] = {};
	if (town.sandingBlock > 2 || town.wrenchThrown > 2 || town.wrenchFound > 2 || town.wrenchInTroughSeen > 1 ||
			town.kevinAway > 1 || town.craterFloorVisited > 1 || town.joshuaGone > 1 || town.helmetTaken > 1 ||
			town.kevinMet > 1 || town.catapult > 3 || town.statueLowered > 1 || town.blowtorchTaken > 1 ||
			town.sculptureTraded > 1 || town.craneKnown > 1 || town.craneTried > 1 || town.wrenchPermission > 2 ||
			town.fuelTradeKnown > 1 || town.waterPermission > 1 || town.waterRequested > 1 || town.hopiDiscussed > 1 ||
			town.roughSculptureRefused > 1 || town.polishedSculptureRefused > 1 || town.oscarStrengthKnown > 1 || town.basementVisited > 1 ||
			town.bankImpactSeen > 1 || town.safeOpened > 1 || town.stethoscopeTried > 1 || town.safeDialVisited > 1 ||
			town.safeDial > 99 || town.safeProgress > 3 || town.machinePowered > 1 || town.melodyTried > 1 ||
			town.campingGearTaken > 1 || townItems.sculpture > 2 || townItems.tube30 > 3 || townItems.tube50 > 5 ||
			townItems.mortar > 1 || townItems.mortarIdea > 1) {
		return false;
	}
	if (town.craterVisited > 1 || town.joshuaMet > 1 || town.campLeft > 1 ||
			town.strapDelivered > 3 || town.wrenchDelivered > 2 || town.wrenchNeeded > 1 || town.fuelNeeded > 1 ||
			town.machineKnown > 1 || town.rotorKnown > 1 || town.batteryKnown > 1 || town.helmetKnown > 1 ||
			town.dimensionalDoorKnown > 1 || town.strapIdea > 1 || townItems.strap > 2) {
		return false;
	}
	if (town.saloonEntranceVisited > 1 || town.saloonDoorKnown > 1 || town.saloonVisited > 1 ||
			town.flowerPotTaken > 1 || town.gardeningShed > 2 || town.tobaccoLeavesTaken > 1 ||
			town.artichokeSeen > 1 || town.carnivorousPlantSeen > 1) {
		return false;
	}
	if (town.mineVisited > 1 || town.oilCan > 2 || town.rockMoved > 1 ||
			town.houseVisited > 1 || town.stone > 2 || town.vessel > 2 ||
			town.oscarMet > 1 || town.oscarHelpRefused > 1 || town.rawTobaccoRefused > 1) {
		return false;
	}
	if (town.locomotiveVisited > 1 || town.locomotiveKey > 3 || town.waterTank > 5 ||
			town.cabVisited > 1 || town.cabExited > 1 || town.boiler > 3 ||
			town.steamLever > 2 || town.pressureWheel > 2 || town.drainSeen > 1 ||
			townItems.oilCanFull > 1 || townItems.bottleContents > 2 || town.craterSeen > 1) {
		return false;
	}
	if (town.arrived > 1 || town.streetVisited > 1 || town.strapSeen > 2 || town.sushiMet > 1 || town.mineMapKnown > 1) {
		return false;
	}
	if (town.bankVisited > 2 || town.bankShelves > 2 || town.bankStampTaken > 1 || town.basementKnown > 1 ||
			town.bodySeen > 1 || town.doctorKnown > 1 || town.firewoodTaken > 1 || town.cellOpen > 1 ||
			town.bagSearched > 1 || town.bucketSeen > 1 || town.bottleSeen > 1 || town.cellRefusals > 2 ||
			town.safeSeen > 1 || town.oscarTobacco > 2 || town.stethoscopeTaken > 1 || town.linimentTaken > 1) {
		return false;
	}
	if (hopi.cliffVisited > 1 || hopi.skeletonSeen > 1 || hopi.nail > 2 || hopi.timber > 3 ||
			hopi.femurTaken > 1 || hopi.rockTried > 1 || hopi.ginaFound > 1 || hopi.villageLevel > 4 ||
			hopi.rope > 2 || hopi.legSplinted > 1 || hopi.cryptVisited > 1 || hopiItems.toolHandle > 1) {
		return false;
	}
	if (hopi.canyonVisited > 1 || hopi.cryptEntranceVisited > 1 || hopi.keyholeBlockedKnown > 1 ||
			hopi.bushSeen > 1 || hopi.cryptDoor > 3) {
		return false;
	}
	if (hopi.statueVisited > 1 || hopi.tomahawk > 2 || hopi.leftCrossSeen > 1 || hopi.rightCrossSeen > 1) {
		return false;
	}
	if (finale.doritaMet > 1 || finale.mediumDiscussed > 1 || finale.mediumTests > 3 || finale.seanceComplete > 1 ||
			finale.oscarAway > 1 || finale.gangsters > 2 || finale.trailerFound > 2 || finale.moneyFound > 3 ||
			finale.nunsHabit > 2 || finale.recordingPlan > 1 || finale.recordings > 3 || finale.ginaUpdates > 7 ||
			finale.ginaVisitPending > 1 || finale.gangstersMet > 1 || finale.gangsterBag > 3) {
		return false;
	}
	if (finale.scalpelNeeded > 1 || finale.streetVisited > 1 || finale.sheriffThanked > 1 || finale.sushiVisited > 1 ||
			finale.bankResearch > 3 || finale.voicePlanPending > 1 || finale.voiceEditing > 2 || finale.departure > 2) {
		return false;
	}
	if (finale.rutgerMet > 1 || finale.rutgerTranceDiscussed > 1 || finale.rutgerBrewDiscussed > 1 ||
			finale.rutgerPlantHint > 1 || finale.rutgerBrew > 3 || finale.saloonVisited > 1 ||
			finale.locomotiveBadge > 3 || finale.saturnMet > 1 || finale.craterVisited > 1) {
		return false;
	}
	if (finale.hotelVisited > 1 || finale.rifle > 2 || finale.pokerTaken > 1 || finale.bellRings > 4 ||
			finale.sushiTruthTold > 1 || finale.hackerKnown > 1 || finale.trailerDiscussed > 1 ||
			finale.gangstersDiscussed > 1 || finale.bagDelivered > 2 || finale.movieFanKnown > 1 ||
			finaleItems.scalpel > 2 || finaleItems.heatNeeded > 1 || finaleItems.podsOpened > 1) {
		return false;
	}
	if (finale.trailerVisited > 1 || finale.trailerDoor > 1 || finale.trailerDoorTried > 1 ||
			finale.bankBrochureTaken > 1 || finale.trailerWarnings > 2 || finale.trailerInteriorVisited > 1) {
		return false;
	}
	if (finale.ginaVisited > 1 || finale.ginaMediumDiscussed > 1) {
		return false;
	}
	for (uint i = 0; i < itemCount; ++i) {
		if (!items[i] || items[i] >= sizeof(owned) || owned[items[i]] ||
				items[i] > (chapter() == 6 ? 21 : chapter() == 5 ? 16 : chapter() == 4 ? 32 : chapter() == 3 ? 26 : 19))
			return false;
		owned[items[i]] = true;
	}
	return !selected || owned[selected];
}

bool RunawaySaveState::read(Common::SeekableReadStream &stream) {
	*this = RunawaySaveState();
	if (stream.readUint32BE() != MKTAG('R', 'U', 'N', 'A'))
		return false;
	const uint16 version = stream.readUint16LE();
	if (version < 1 || version > 27)
		return false;
	Common::Serializer s(&stream, nullptr);
	s.setVersion(version);
	sync(s);
	return !s.err() && !stream.eos() && valid();
}

bool RunawaySaveState::write(Common::WriteStream &stream) {
	if (!valid())
		return false;
	stream.writeUint32BE(MKTAG('R', 'U', 'N', 'A'));
	stream.writeUint16LE(27);
	Common::Serializer s(nullptr, &stream);
	s.setVersion(27);
	sync(s);
	return !s.err();
}

} // End of namespace Hollywood
