/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef PEGASUS_ITEMS_BIOCHIPS_ARTHURCHIP_H
#define PEGASUS_ITEMS_BIOCHIPS_ARTHURCHIP_H

#include "pegasus/hotspot.h"
#include "pegasus/util.h"
#include "pegasus/items/biochips/biochipitem.h"

namespace Pegasus {

enum ArthurEvent {
	kArthurLoadedSavedGame,
	kArthurAttemptedLockedDoor,
	kArthurAttemptedLockedDoorAgain,
	kArthurDestroyedInventoryItem,
	kArthurDisabledAI,

	kArthurCaldoriaFinishedJoyride,
	kArthurCaldoriaSelectedStickyBuns,
	kArthurCaldoriaCreatedCornbread,
	kArthurCaldoriaDrankOJ,
	kArthurCaldoriaZoomedToLaundry,
	kArthurCaldoriaReachedToilet,
	kArthurCaldoriaReadPaper,
	kArthurCaldoriaChoseAgencyHairStyle,
	kArthurCaldoriaSawVacantApartment,
	kArthurCaldoriaLookThroughTelescope,
	kArthurCaldoriaRoofDoor,
	kArthurCaldoriaUsedCardBomb,
	kArthurCaldoriaBlownDoor,
	kArthurCaldoriaSawVoiceAnalysis,
	kArthurCaldoriaStunningSinclair,
	kArthurCaldoriaSeeRoofBomb,
	kArthurCaldoriaDisarmedNuke,

	kArthurTSAEnteredCave,
	kArthurTSASawAgent3,
	kArthurTSASawBust,
	kArthurTSAReachedJunction,
	kArthurTSAClickedRobot1,
	kArthurTSAClickedRobot2,
	kArthurTSAOpenTBPMonitor,
	kArthurTSASawJourneymanKey,
	kArthurTSASawBiochips,
	kArthurTSAUsedPegasus,
	kArthurTSAConfinedByBaldwin,
	kArthurTSARedirectedRobots,
	kArthurTSAUsedTurbolift,
	kArthurTSASawFirstOpMemMovie,
	kArthurTSAInPegasusNoVideo,
	kArthurTSASawBaldwinSayGo,
	kArthurTSALeaving,

	kArthurGoToPrehistoric,

	kArthurPrehistoricReachedJunction,
	kArthurPrehistoricSawBreaker,
	kArthurPrehistoricBreakerThrown,
	kArthurPrehistoricAtCliffEdge,
	kArthurPrehistoricSawEggs,
	kArthurPrehistoricZoomedToVault,
	kArthurPrehistoricAttemptedBridge,
	kArthurPrehistoricExtendedBridge,
	kArthurPrehistoricCrossedBridge,
	kArthurPrehistoricUnlockedVault,

	kArthurMarsReadyForKiosk,
	kArthurMarsLookedAtGuards,
	kArthurMarsZoomedToKeyCard,
	kArthurMarsTurnedOnTransport,
	kArthurMarsCantFillMask,
	kArthurMarsSawWelcomeVideos,
	kArthurMarsRobotThrownPlayerWithMask,
	kArthurMarsLeftPodNoCrowBar,
	kArthurMarsLookAtEmptyTracks,
	kArthurMarsEnteredReactor,
	kArthurMarsSawLockedPanel,
	kArthurMarsSawLockedPanelNoNitrogen,
	kArthurMarsUsedLiquidNitrogen,
	kArthurMarsFoundCardBomb,
	kArthurMarsSolvedReactorGame,
	kArthurMarsDeactivatedCardBomb,
	kArthurMarsExitedReactorWithCardBomb,
	kArthurMarsInAirlockNoOxygen,
	kArthurMarsMazeReachedJunction,
	kArthurMarsOxygen50Warning,
	kArthurMarsOxygen25Warning,
	kArthurMarsOxygen5Warning,
	kArthurMarsFoundBuckets,
	kArthurMarsApproachedBuckets,
	kArthurMarsEnteredGearRoom,
	kArthurMarsLookAtGears,
	kArthurMarsExitedGearRoom,
	kArthurMarsFoundNoShuttlePresent,
	kArthurMarsEnteredShuttle,
	kArthurMarsFoundDeadRobot,
	kArthurMarsRobotHeadOpen,

	kArthurWSCRemovedDart,
	kArthurWSCPoisonedDuringGame,
	kArthurWSCFailedMolecule,
	kArthurWSCDesignedAntidote,
	kArthurWSCSawAresHologram,
	kArthurWSCLookAtMorphExperiment,
	kArthurWSCStartMorphExperiment,
	kArthurWSCSawMorphExperiment,
	kArthurWSCLeftLabNoKeyOrCanisters,
	kArthurWSCAtOppositeDoor,
	kArthurWSCReadyForMap,
	kArthurWSCAttemptedLockedDoor,
	kArthurWSCSawSinclairDoor,
	kArthurWSCSawSinclairDoorNoKey,
	kArthurWSCAttemptedSinclairDoorNoKey,
	kArthurWSCZoomedToSnake,
	kArthurWSCActivatedComputer,
	kArthurWSCZoomedToSinclairMessages,
	kArthurWSCPlayedEasterEggMessage,
	kArthurWSCGotMachineGun,
	kArthurWSCSeenNerd,
	kArthurWSCSawBrokenDoor,
	kArthurWSCSawBrokenDoorNoCrowBar,
	kArthurWSCUsedCrowBar,
	kArthurWSCDidPlasmaDodge,
	kArthurWSCEnteredAuditorium,
	kArthurWSCSawSinclairLecture,
	kArthurWSCEnteredPassage,
	kArthurWSCInPassage,
	kArthurWSCExitedPassage,
	kArthurWSCSawCatwalkDoor,
	kArthurWSCRobotHeadOpen,

	kArthurNoradAtSecurityMonitor,
	kArthurNoradSawFillingStation,
	kArthurNoradSawIntakeWarning,
	kArthurNoradDidntFillCanisters,
	kArthurNoradSawUnconsciousOperator,
	kArthurNoradAttemptedLockedDoor,
	kArthurNoradAttemptedLockedDoorAgain,
	kArthurNoradReachedPressureDoor,
	kArthurNoradSawSubMessage,
	kArthurNoradSawClawMonitor,
	kArthurNoradPlayedWithClaw,
	kArthurNoradEnteredSub,
	kArthurNoradExitedSub,
	kArthurNoradApproachedDamagedDoor,
	kArthurNoradAtRetScanNoBiochip,
	kArthurNoradStartGlobeGame,
	kArthurNoradSelectedIncorrectSilo,
	kArthurNoradFinishedGlobeGame,
	kArthurNoradThreatenedByRobot,
	kArthurNoradBeatRobotWithClaw,
	kArthurNoradRobotHeadOpen,

	kNumArthurFlags
};

class ArthurChip : public BiochipItem {
public:
	ArthurChip(const ItemID, const NeighborhoodID, const RoomID, const DirectionConstant);
	virtual ~ArthurChip();

	void select();

	void setUpArthurChip();
	void activateArthurHotspots();
	void clickInArthurHotspot(HotSpotID);
	void playArthurMovie(const Common::String &);
	bool playArthurMovieForEvent(const Common::String &, ArthurEvent event);

protected:
	Hotspot _arthurWisdomHotspot;
	Hotspot _chattyArthurHotspot;
	Hotspot _chattyAIHotspot;
	Hotspot _arthurHeadHotspot;
	Common::String _lastArthurMovie;
};

class ArthurManager : public Common::Singleton<ArthurManager> {
public:
	ArthurManager() { resetArthurState(); }

	void resetArthurState();

protected:
	friend class Common::Singleton<SingletonBaseType>;
	friend class ArthurChip;

private:
	FlagsArray<byte, kNumArthurFlags> _arthurFlags;
};

extern ArthurChip *g_arthurChip;

} // End of namespace Pegasus

#define Arthur (::Pegasus::ArthurManager::instance())

#endif
