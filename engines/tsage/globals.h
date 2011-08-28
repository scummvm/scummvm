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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TSAGE_GLOBALS_H
#define TSAGE_GLOBALS_H

#include "common/random.h"
#include "tsage/core.h"
#include "tsage/dialogs.h"
#include "tsage/scenes.h"
#include "tsage/events.h"
#include "tsage/sound.h"
#include "tsage/saveload.h"
#include "tsage/blue_force/blueforce_ui.h"

namespace TsAGE {

class Globals : public SavedObject {
private:
	static void dispatchSound(ASound *obj);
public:
	GfxSurface _screenSurface;
	GfxManager _gfxManagerInstance;
	Common::List<GfxManager *> _gfxManagers;
	SceneHandler *_sceneHandler;
	Game *_game;
	EventsClass _events;
	SceneManager _sceneManager;
	ScenePalette _scenePalette;
	SceneRegions _sceneRegions;
	SceneItemList _sceneItems;
	SceneObjectList _sceneObjectsInstance;
	SceneObjectList *_sceneObjects;
	SynchronizedList<SceneObjectList *> _sceneObjects_queue;
	SceneText _sceneText;
	int _gfxFontNumber;
	GfxColors _gfxColors;
	GfxColors _fontColors;
	byte _color1, _color2, _color3;
	SoundManager _soundManager;
	Common::Point _dialogCenter;
	WalkRegions _walkRegions;
	SynchronizedList<ASound *> _sounds;
	bool _flags[256];
	Player _player;
	ASound _soundHandler;
	InvObjectList *_inventory;
	Region _paneRegions[2];
	int _paneRefreshFlag[2];
	Common::Point _sceneOffset;
	Common::Point _prevSceneOffset;
	SceneObject *_scrollFollower;
	SequenceManager _sequenceManager;
	Common::RandomSource _randomSource;
	int _stripNum;
	int _gfxEdgeAdjust;
public:
	Globals();
	~Globals();

	void reset();
	void setFlag(int flagNum) {
		assert((flagNum >= 0) && (flagNum < MAX_FLAGS));
		_flags[flagNum] = true;
	}
	void clearFlag(int flagNum) {
		assert((flagNum >= 0) && (flagNum < MAX_FLAGS));
		_flags[flagNum] = false;
	}
	bool getFlag(int flagNum) const {
		assert((flagNum >= 0) && (flagNum < MAX_FLAGS));
		return _flags[flagNum];
	}

	GfxManager &gfxManager() { return **_gfxManagers.begin(); }
	virtual Common::String getClassName() { return "Globals"; }
	virtual void synchronize(Serializer &s);
	void dispatchSounds();
};

extern Globals *_globals;

#define GLOBALS (*_globals)
#define BF_GLOBALS (*((::TsAGE::BlueForce::BlueForceGlobals *)_globals))

// Note: Currently this can't be part of the _globals structure, since it needs to be constructed
// prior to many of the fields in Globals execute their constructors
extern ResourceManager *_resourceManager;


namespace BlueForce {

using namespace TsAGE;

enum Bookmark {
	bNone,
	bStartOfGame, bCalledToDomesticViolence, bArrestedGreen, bLauraToParamedics,
	bBookedGreen, bStoppedFrankie, bBookedFrankie, bBookedFrankieEvidence,
	bEndOfWorkDayOne, bTalkedToGrannyAboutSkipsCard, bLyleStoppedBy, bEndDayOne,
	bInspectionDone, bCalledToDrunkStop, bArrestedDrunk, bEndDayTwo,
	bFlashBackOne, bFlashBackTwo, bFlashBackThree, bDroppedOffLyle, bEndDayThree,
	bDoneWithIsland, bDoneAtLyles, bEndDayFour, bInvestigateBoat, bFinishedWGreen,
	bAmbushed, bAmbushOver, bEndOfGame
};

enum Flag {
	JAKE_FILE_COPIED, gunClean, onBike, onDuty, fShowedIdToKate, fLateToMarina,
	fCalledBackup, fWithLyle, gunDrawn, fBackupArrived340, fBriefedBackup,
	fGotAllSkip340, fToldToLeave340, fBackupIn350, fNetInBoat, fForbesWaiting,
	fWithCarter, fTalkedToTony, fMugOnKate, takenWeasel, gotTrailer450,
	showEugeneNapkin, showRapEugene, fMgrCallsWeasel, fCarterMetLyle,
	fGunLoaded, fLoadedSpare, showEugeneID, fRandomShot350, examinedFile810,
	shownLyleCrate1, shownLyleRapsheet, shownLyleDisk, shownLylePO,
	fCanDrawGun, fGotAutoWeapon, fGotBulletsFromDash, fShotSuttersDesk,
	greenTaken, fLateToDrunkStop, didDrunk, fSearchedTruck, seenFolder,
	showMugAround, frankInJail, fTalkedCarterDay3, fDecryptedBluePrints,
	fTalkedToDrunkInCar, fToldLyleOfSchedule, fTalkedShooterNoBkup,
	fTalkedDriverNoBkup, fDriverOutOfTruck, readGreenRights, readFrankRights,
	talkedToHarrisAboutDrunk, unlockBoat, fShootGoon, fBlowUpGoon,
	fTalkedToBarry, fTalkedToLarry, fLeftTraceIn920, fLeftTraceIn900,
	fBackupAt340, fShotNicoIn910, fGotPointsForTktBook, fGotPointsForMCard,
	fShowedBluePrint, fGotPointsForPunch, fGotPointsForBox, fGotPointsForBank,
	fGotPointsForCombo, fGotPointsForCoin, fGotPointsForCPU, fGotPointsForBoots,
	fGotPointsForCrate, fGotPointsForBlackCord, fGotPointsForGeneratorPlug,
	fGotPointsForFuseBoxPlug, fGotPointsForStartGenerator, fGotPointsForLightsOn,
	fGotPointsForOpeningDoor, fGotPointsForClosingDoor, fGotPointsForLightsOff,
	fGotPointsForGeneratorOff, fGotPointsForCordOnForklift, fGotPointsForCuffingNico,
	fGotPointsForCuffingDA, fGotPointsForSearchingNico, fGotPointsForSearchingDA,
	fLeftTraceIn910, fBookedGreenEvidence, fGotPointsForCleaningGun,
	fGotPointsForMemo, fGotPointsForFBI, fTookTrailerAmmo, fAlertedGreen355,
	fGotGreen355fTalkedToGrannyDay3, shownFax, beenToJRDay2, shownLyleCrate1Day1,
	fLyleOnIsland, iWasAmbushed, fGangInCar, fArrivedAtGangStop, ticketVW,
	f1015Marina, fCan1015Marina, f1015Frankie, fCan1015Frankie, f1015Drunk,
	fCan1015Drunk, f1027Marina, fCan1027Marina, f1027Frankie, fCan1027Frankie,
	f1027Drunk, fCan1027Drunk, f1035Marina, fCan1035Marina, f1035Frankie,
	fCan1035Frankie, f1035Drunk, fCan1035Drunk, f1097Marina, fCan1097Marina,
	f1097Frankie, fCan1097Frankie, f1097Drunk, fCan1097Drunk, f1098Marina,
	fCan1098Marina, f1098Frankie, fCan1098Frankie, f1098Drunk, fCan1098Drunk,
	fCuffedFrankie, fGotPointsForTrapDog, fGotPointsForUnlockGate,
	fGotPointsForUnlockWarehouse, fGotPointsForLockWarehouse, fGotPointsForLockGate,
	fGotPointsForFreeDog, fGotPointsForWhistleDog, fGivenNapkin, fCan1004Marina,
	fCan1004Drunk, fHasLeftDrunk, fHasDrivenFromDrunk, fCrateOpen, fSawGuns,
	hookPoints
};

class BlueForceGlobals: public Globals {
public:
	ASoundExt _sound1, _sound2, _sound3;
	UIElements _uiElements;
	int _dayNumber;
	int _v4CEA4;
	int _v4CEA8;
	int _driveFromScene;
	int _driveToScene;
	int _v4CF9E;
	int _v4E238;
	int _v501FC;
	int _v51C42;
	int _v51C44;
	int _interfaceY;
	Bookmark _bookmark;
	int _mapLocationId;

	BlueForceGlobals();
	virtual Common::String getClassName() { return "BFGlobals"; }
	virtual void synchronize(Serializer &s);
};

} // End of namespace BlueForce

} // End of namespace TsAGE

#endif
