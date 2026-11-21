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

#ifndef HOLLYWOOD_RUNAWAY_SAVELOAD_H
#define HOLLYWOOD_RUNAWAY_SAVELOAD_H

#include "common/serializer.h"
#include "hollywood/runaway/detection.h"

namespace Hollywood {

// Persistent state survives scene changes. Saves are made between actions;
// resources, pointers and animation clocks are rebuilt when restoring it.
struct RunawaySaveState {
	struct Hospital {
		byte glassTaken = 0, pillsTaken = 0, sheetTaken = 0, bagTaken = 0, exitMapRead = 0;
		byte markerTaken = 0, alcohol = 0;
		byte window = 0, chartReplaced = 0, chartMoved = 0, decoy = 0, attacked = 0, medicineRead = 0;
		byte storeroomVisited = 0, syringesSeen = 0, sprayTaken = 0, book = 0, blankChart = 0, headTaken = 0, pillowsTaken = 0;
	} hospital;

	struct Museum {
		byte susan = 0, restoration = 0, laserUpgraded = 0, laserSeen = 0, shelvesSeen = 0;
		byte susanMet = 0, labVisited = 0, shelfBooksSeen = 0, safeUsed = 0;
		byte officeVisited = 0, exhibitionVisited = 0, phonePlanExplained = 0;
		byte willyMet = 0, batteryRequested = 0, coffeeMachine = 0, coffeeSeen = 0;
		byte basketSeen = 0, needsPackage = 0, needsGroundCoffee = 0, willyInLab = 0;
		byte keypad = 0, sawCode = 0, phoneUsed = 0, voiceLockSeen = 0;
		byte chamberOpen = 0, maskTaken = 0, keyTaken = 0, ladleTaken = 0;
		byte cabinetSeen = 0, scannerSeen = 0, nitrogenSeen = 0, microscopeSeen = 0;
		byte talcSeen = 0, varnishSeen = 0, mayanObjectsSeen = 0, coffeeBeansTaken = 0;
		byte buttonSeen = 0, ownVoiceTried = 0, analysisDoorSeen = 0, leftLabOnce = 0, keySeen = 0;
		byte exhibitSeen[3] = {};
		uint16 keypadDigits = 0;
		uint32 chamberTime = 0;
	} museum;

	struct MuseumItems {
		byte businessCardExamined = 0;
		byte restoredCrucifix = 0, voiceRecorded = 0, coffee = 0, frozenBattery = 0;
		byte rubyRemoved = 0, recorderBattery = 0, batteryInLadle = 0, packageFilled = 0;
	} museumItems;

	struct Desert {
		byte visited[10] = {};
		byte escapedCabin = 0, hatchSeen = 0, bucketTaken = 0;
		byte helmetTaken = 0, beltTaken = 0, beltSeen = 0, gun = 0, gunSeen = 0;
		byte mariolaHelped = 0, ballTaken = 0, tanningOil = 0, lulaHelped = 0;
		byte busPosterSeen = 0, lulaCalled = 0, lulaRefused = 0;
		byte dynamitePlaced = 0, oilSeen = 0;
		byte antsDistracted = 0, dynamite = 0, shedTried = 0, trainEntered = 0;
		byte barrel = 0, screwSeen = 0, screwTaken = 0, bucket = 0, trainDoorOpen = 0, powderNeeded = 0;
		byte barnDoor = 0, footrest = 0, guard = 0, insideBarn = 0, beerSeen = 0;
		byte peanutButter = 0, motorcycleSeen = 0, footrestSeen = 0;
		byte lulaDistractedGuard = 0, guardWeaknessKnown = 0;
		byte busDoor = 0, lipstickShelf = 0, keyTaken = 0, glassesTaken = 0;
		byte vacuum = 0, crank = 0, necklacesSeen = 0, ringsSeen = 0;
		byte fridgeOpened = 0, ventCleaned = 0, ventSeen = 0, needle = 0, fridgeSeen = 0;
		byte carlaMet = 0, carlaStory = 0, planKnown = 0;
		uint32 carlaTopics = 0, lulaTopics = 0;
	} desert;

	struct DesertItems {
		byte antTrickKnown = 0;
		byte helmet = 0, lipstick = 0, lipsticks = 0, belt = 0, ball = 0;
		byte glasses = 0, pill = 0, bucket = 0;
		byte gunReady = 0, bombReady = 0, helicopterReady = 0;
	} desertItems;

	struct Town {
		byte arrived = 0, streetVisited = 0, strapSeen = 0;
		byte sushiMet = 0, mineMapKnown = 0;
		byte bankVisited = 0, bankShelves = 0, bankStampTaken = 0, basementKnown = 0;
		byte bodySeen = 0, doctorKnown = 0, firewoodTaken = 0, cellOpen = 0;
		byte bagSearched = 0, bucketSeen = 0, bottleSeen = 0, cellRefusals = 0;
		byte safeSeen = 0, oscarTobacco = 0, stethoscopeTaken = 0, linimentTaken = 0;
		byte locomotiveVisited = 0, locomotiveKey = 0, waterTank = 0;
		byte cabVisited = 0, cabExited = 0, boiler = 0;
		byte steamLever = 0, pressureWheel = 0, drainSeen = 0;
		byte craterSeen = 0;
		byte mineVisited = 0, oilCan = 0, rockMoved = 0;
		byte houseVisited = 0, stone = 0, vessel = 0;
		byte oscarMet = 0, oscarHelpRefused = 0, rawTobaccoRefused = 0;
		byte saloonEntranceVisited = 0, saloonDoorKnown = 0, saloonVisited = 0;
		byte flowerPotTaken = 0, gardeningShed = 0, tobaccoLeavesTaken = 0;
		byte artichokeSeen = 0, carnivorousPlantSeen = 0;
		byte craterVisited = 0, joshuaMet = 0, campLeft = 0;
		byte strapDelivered = 0, wrenchDelivered = 0, wrenchNeeded = 0, fuelNeeded = 0;
		byte machineKnown = 0, rotorKnown = 0, batteryKnown = 0, helmetKnown = 0, dimensionalDoorKnown = 0;
		byte strapIdea = 0;
		byte sandingBlock = 0, wrenchThrown = 0, wrenchFound = 0, wrenchInTroughSeen = 0;
		byte kevinAway = 0, craterFloorVisited = 0, joshuaGone = 0, helmetTaken = 0;
		byte kevinMet = 0, catapult = 0, statueLowered = 0, blowtorchTaken = 0;
		byte sculptureTraded = 0, craneKnown = 0, craneTried = 0, wrenchPermission = 0;
		byte fuelTradeKnown = 0, waterPermission = 0, waterRequested = 0, hopiDiscussed = 0;
		byte roughSculptureRefused = 0, polishedSculptureRefused = 0, oscarStrengthKnown = 0;
		byte basementVisited = 0, bankImpactSeen = 0;
		byte safeOpened = 0, stethoscopeTried = 0, safeDialVisited = 0;
		byte safeDial = 69, safeProgress = 0;
		byte machinePowered = 0, melodyTried = 0, campingGearTaken = 0;
	} town;

	struct TownItems {
		byte strap = 0; // Loose, needs joining, joined.
		byte sculpture = 0; // Rough, sanded, decorated with amber.
		byte oilCanFull = 0, bottleContents = 0;
		byte tube30 = 0, tube50 = 0; // Contents in units of ten cubic centimeters.
		byte mortar = 0, mortarIdea = 0;
	} townItems;

	struct Hopi {
		byte cliffVisited = 0, skeletonSeen = 0, nail = 0, timber = 0;
		byte femurTaken = 0, rockTried = 0;
		byte ginaFound = 0, villageLevel = 0, rope = 0, legSplinted = 0;
		byte cryptVisited = 0;
		byte canyonVisited = 0, cryptEntranceVisited = 0, keyholeBlockedKnown = 0, bushSeen = 0;
		byte cryptDoor = 0; // Sand-blocked, cleared, open, sealed after visiting Wupuchim.
		byte statueVisited = 0, tomahawk = 0, leftCrossSeen = 0, rightCrossSeen = 0;
	} hopi;

	struct HopiItems {
		byte toolHandle = 0;
	} hopiItems;

	struct Finale {
		byte doritaMet = 0, mediumDiscussed = 0, mediumTests = 0, seanceComplete = 0;
		byte oscarAway = 0, gangsters = 0, moneyFound = 0, nunsHabit = 0;
		byte trailerFound = 0; // Unknown, directions from Saturn, visited.
		byte recordingPlan = 0, recordings = 0, ginaUpdates = 0, ginaVisitPending = 0;
		byte gangstersMet = 0, gangsterBag = 0; // Bag: absent, present, examined, taken.
		byte scalpelNeeded = 0, streetVisited = 0, sheriffThanked = 0, sushiVisited = 0;
		byte bankResearch = 0; // Not started, requested, Brian went away, results ready.
		byte voicePlanPending = 0, voiceEditing = 0, departure = 0;
		byte rutgerMet = 0, rutgerTranceDiscussed = 0, rutgerBrewDiscussed = 0, rutgerPlantHint = 0;
		byte rutgerBrew = 0; // Not started, brewing, consumed, discussed afterward.
		byte saloonVisited = 0;
		byte locomotiveBadge = 0; // In the boiler, on the ground, examined, taken.
		byte saturnMet = 0, craterVisited = 0;
		byte hotelVisited = 0, rifle = 0, pokerTaken = 0, bellRings = 0;
		byte sushiTruthTold = 0, hackerKnown = 0, trailerDiscussed = 0, gangstersDiscussed = 0;
		byte bagDelivered = 0; // Not delivered, waiting for Sushi, examined.
		byte movieFanKnown = 0;
		byte trailerVisited = 0, trailerDoor = 0, trailerDoorTried = 0, bankBrochureTaken = 0;
		byte trailerWarnings = 0, trailerInteriorVisited = 0;
		byte ginaVisited = 0, ginaMediumDiscussed = 0;
	} finale;

	struct FinaleItems {
		byte scalpel = 0; // Cold, hot, cooled after cutting the pods.
		byte heatNeeded = 0, podsOpened = 0;
	} finaleItems;

	uint16 scene = 5030, x = 623, y = 447, camera = 111;
	byte facing = 3, lidOpen = 0, unplugged = 0, windowClean = 0;
	byte coolerMoved = 0, coolerInspected = 0, peeked = 0, ice = 0, hatch = 0;
	byte pickedUp[20] = {}, examined[20] = {};
	byte itemCount = 0, items[32] = {}, selected = 0, cloth = 0;
	byte inventoryOpen = 0, verb = 0;
	uint32 randomSeed = 0, playTime = 0;
	byte game = kRunawayCabinDemo;
	uint16 cameraY = 0;
	byte upperFloor = 1, books = 0, recorder = 0, moneyIdentified = 0, safeOpen = 0;
	byte varnishTaken = 0, talcTaken = 0, coffeeTaken = 0, keyCount = 0;
	byte water = 0;
	byte markerInk = 0, syringeFilled = 0, chartWritten = 0, headWig = 0;

	byte chapter() const;
	bool valid() const;
	bool read(Common::SeekableReadStream &stream);
	bool write(Common::WriteStream &stream);

private:
	void sync(Common::Serializer &s);
};

} // End of namespace Hollywood

#endif
