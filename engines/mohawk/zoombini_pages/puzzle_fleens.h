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

#ifndef MOHAWK_ZOOMBINI_PAGES_PUZZLE_FLEENS_H
#define MOHAWK_ZOOMBINI_PAGES_PUZZLE_FLEENS_H

#include "mohawk/zoombini_pages/puzzle_base.h"

namespace Mohawk {

/**
 * Fleens! puzzle page (route 3, puzzle 1).
 *
 * The page has one visual Fleen for every loaded Zoombini.
 * Three Fleens occupy the exposed branch and are the puzzle targets.
 * Dropping the corresponding Zoombini on the highlighted lure point drives a coupled Snoid/Fleen escape state machine.
 * The Go button becomes available only after all three targets leave the branch.
 *
 */
class ZoombiniPuzzleFleens : public ZoombiniPuzzle {
public:
	/** Fleen-specific values in the shared hair, eyes, nose, and feet category order. */
	struct FleenTrait {
		/** Empty feature value shared with @ref ZmbTrait. */
		static constexpr byte TRAIT_NONE = ZmbTrait::TRAIT_NONE;

		/** Fleen hair feature (1-5 valid, 0 = none). */
		enum HairKind : byte {
			kHairPonytail = 1,
			kHairSpikey = 2,
			kHairBandana = 3,
			kHairMohawk = 4,
			kHairHornedHelmet = 5,
		};

		/** Fleen eye feature (1-5 valid, 0 = none). */
		enum EyeKind : byte {
			kEyeEyelashes = 1,
			kEyeBand = 2,
			kEyeThreeEyed = 3,
			kEyeSpectacles = 4,
			kEyeVisor = 5,
		};

		/** Fleen nose feature (1-5 valid, 0 = none). */
		enum NoseKind : byte {
			kNoseBlack = 1,
			kNoseBrown = 2,
			kNoseYellow = 3,
			kNoseCyan = 4,
			kNosePurple = 5,
		};

		/** Fleen feet feature (1-5 valid, 0 = none). */
		enum FeetKind : byte {
			kFeetBoots = 1,
			kFeetDressShoes = 2,
			kFeetSegway = 3,
			kFeetTreads = 4,
			kFeetRockets = 5,
		};

		byte _hair = TRAIT_NONE;
		byte _eyes = TRAIT_NONE;
		byte _nose = TRAIT_NONE;
		byte _feet = TRAIT_NONE;

		/** Access a Fleen feature by the shared category kind. */
		byte &operator[](ZmbTrait::TraitKind kind);
		/** Access a Fleen feature by the shared category kind. */
		const byte &operator[](ZmbTrait::TraitKind kind) const;
		/** Return the Fleen-specific name for one feature value. */
		static const char *debugTraitValueName(ZmbTrait::TraitKind kind, byte value);
		/** Format Fleen feature names in hair, eyes, nose, and feet order. */
		Common::String toStr() const;

	private:
		static constexpr const char *kTraitValueNames[4][5] = {
			{"Ponytail", "Spikey", "Bandana", "Mohawk", "HornedHelmet"},
			{"Eyelashes", "EyeBand", "ThreeEyed", "Spectacles", "Visor"},
			{"Black", "Brown", "Yellow", "Cyan", "Purple"},
			{"Boots", "DressShoes", "Segway", "Treads", "Rockets"}};
	};

	/** Spatial ownership code assigned to one Fleen runner. */
	enum FleenPositionCode : byte {
		// Position of the upper normal branch.
		kFleenPosCode00_UpperBranchFirst = 0,
		kFleenPosCode01_UpperBranchSecond = 1,
		kFleenPosCode02_UpperBranchThird = 2,
		kFleenPosCode03_UpperBranchFourth = 3,
		// Position of the middle normal branch.
		kFleenPosCode04_MiddleBranchFirst = 4,
		kFleenPosCode05_MiddleBranchSecond = 5,
		kFleenPosCode06_MiddleBranchThird = 6,
		kFleenPosCode07_MiddleBranchFourth = 7,
		kFleenPosCode08_MiddleBranchFifth = 8,
		kFleenPosCode09_MiddleBranchSixth = 9,
		kFleenPosCode10_MiddleBranchSeventh = 10,
		kFleenPosCode11_MiddleBranchEighth = 11,
		// Position of the lower normal branch.
		kFleenPosCode12_LowerBranchFirst = 12,
		kFleenPosCode13_LowerBranchSecond = 13,
		kFleenPosCode14_LowerBranchThird = 14,
		kFleenPosCode15_LowerBranchFourth = 15,
		kFleenPosCode16_LowerBranchFifth = 16,
		// Position of target branch, which has a beehive.
		kFleenPosCode17_TargetBranchFirst = 17,
		kFleenPosCode18_TargetBranchSecond = 18,
		kFleenPosCode19_TargetBranchLast = 19,
		/**
		 * Assigned by event 7 after an accepted submission when the paired Fleen
		 * leaves its branch to follow that Zoombini.
		 */
		kFleenPosCode20_LuredOffBranch = 20,
		/** 21-25: Unused upper bound accepted with the lured Fleen by event 135. */
		kFleenPosCode25_LuredRangeLast = 25
	};

	/** Runtime state for one visual Fleen and its coupled Zoombini. */
	struct FleenCreature {
		/** Callback requested when the current Fleen animation finishes. */
		enum Callback : byte {
			/** No completion callback is pending. */
			kCallbackNone,
			/** Dispatch the movement completion path. */
			kCallbackMovement
		};

		/**
		 * Fixed branch-seat coordinate assigned from page REGS 5000 or 5001 when this Fleen is created.
		 * SCRS movement updates @ref ZoombiniPuzzleFleens::FleenCreature::renderAnchor; this original seat assignment does not change.
		 */
		Common::Point assignedBranchPos;
		/** Current registration-space anchor used to place the Fleen shapes. */
		Common::Point renderAnchor;
		/** Offset from the active SCRS anchor to @ref renderAnchor. */
		Common::Point scriptAnchorOffset;
		/** ID of the Zoombini paired with this Fleen. */
		uint16 pairedSnoidId = 0;
		/** Feature used to render and receive events for this creature. */
		ZmbFeature *feature = nullptr;
		/** Page-transformed Fleen features. */
		FleenTrait trait;
		/** Authored spatial ownership code. */
		FleenPositionCode posCode = kFleenPosCode00_UpperBranchFirst;
		/** Pair state: 0 is available, 1 has been submitted, and 2 has dispatched its escape action. */
		byte boardingState = 0;
		/** Trait-layout code selected for the current Fleen animation. */
		ZmbScriptDecoder::TraitLayout traitLayout = ZmbScriptDecoder::TraitLayout::kSlotOrder00;
		/** Zero-based SCRS frame published on the next due animation tick. */
		int32 nextScrsFrameIdx = 0;
		/** Per-Fleen fidget counter initialized by RNG and advanced on idle ticks. */
		byte fidgetCounter = 0;
		/** Absolute page frame at which the next idle or SCRS animation tick is due. */
		uint32 nextTickFrame = 0;
		/** Whether the creature participates in rendering and updates. */
		bool active = false;
		/** Whether an action-specific SCRS is currently playing. */
		bool animating = false;
		/** Whether the creature is facing the left side of the branch. */
		bool facingLeft = false;
		/** Whether this Fleen is one of the three puzzle targets. */
		bool isTarget = false;
		/** Completion callback selected for the current animation. */
		Callback callback = kCallbackNone;
		/** Whether the callback should run when playback reaches its end. */
		bool callbackOnComplete = false;

		/** Format the assigned branch-seat coordinate and traits like @ref ZmbSnoid::toStr(). */
		Common::String toStr() const {
			return Common::String::format("Fleen    (%3d, %3d): %s",
										  assignedBranchPos.x, assignedBranchPos.y, trait.toStr().c_str());
		}
	};

	/** Create the Fleens puzzle page. */
	ZoombiniPuzzleFleens(MohawkEngine_Zoombini *vm);
	/** Release Fleen creatures, coupled Snoids, and page resources. */
	~ZoombiniPuzzleFleens() override;
	/** Static puzzle name used by debug answer descriptions. */
	static constexpr const char *kPageName = ZmbXferRouteInfo::kXferPageNameFleens;
	static constexpr int kRouteNumber = 3;
	static constexpr int kRoutePuzzleIdx = 1;
	/** Return the puzzle name used by debug answer descriptions. */
	const char *getPageName() const override { return kPageName; }
	int getRouteNumber() const override { return kRouteNumber; }
	int getRoutePuzzleIdx() const override { return kRoutePuzzleIdx; }

	/** Generate the target traits and initialize Fleen state. */
	void open() override;
	/** Select the Fleens puzzle music. */
	void setBackgroundMusic() override;
	/** Select the Fleens background bitmap. */
	void setBackgroundBitmap() override;
	/** Initialize puzzle states and departure transfer source. */
	void initStates() override;
	/** Load branch, lure, Fleen, and Zoombini features. */
	void loadFeatures() override;
	/** Select the Fleens F1 prompt without changing the authored setup RNG order. */
	void initHelpPrompt() override;
	/** Restart the global Snoid fidget inactivity period after Fleens loads. */
	bool requiresFidgetActivityResetOnLoad() const override { return true; }

protected:
	/** Fleens actions accepted by the built-in debug console and keyboard dispatcher. */
	enum class BuiltinDebugAction {
		kInvalid,
		kLevel
	};
	static constexpr const char *kBuiltinDebugActionLevel = "level";
	/** Advance Fleen movement and coupled escape state machines after rendering. */
	void onPostRenderFrame() override;
	/** Begin departure after all target Fleens have escaped. */
	void onGoButtonActivated() override;
	/** Keep the Fleen walk and sound gates in control of the departure state machine. */
	void updateDepartureState() override;
	/** Continue the Fleen controller while the departure sound is active. */
	bool runsControllerDuringDeparture() const override { return true; }
	/** Describe the target trait assignments for diagnostics. */
	Common::String debugGetAnswer() const override;
	/** Describe the Fleens built-in debug actions. */
	Common::String debugGetBuiltinDebugCommandHelp() const override;
	/** Run one Fleens built-in debug action. */
	bool debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output) override;
	/** Handle Fleens built-in debug keys. */
	ZmbEventHandleResult onDebugKeyDown(const Common::KeyState &kbd) override;
	/** Parse one console-facing Fleens action name. */
	static BuiltinDebugAction parseBuiltinDebugAction(const Common::String &action);
	/** Run one typed Fleens built-in debug action. */
	bool runBuiltinDebugAction(BuiltinDebugAction action, Common::String &output);
	/** Report Fleens submit-based chance usage for diagnostics. */
	ZmbChanceInfo debugGetChances() const override;
	/** Fleens supports debugger chance adjustment. */
	bool debugCanSetChances() const override { return true; }
	/** Set Fleens submit-based chance usage for diagnostics. */
	bool debugSetChances(int16 remaining) override;
	/** Process Fleen and Snoid animation callbacks. */
	void onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) override;
	/** Start or update a Fleen drag/drop interaction. */
	ZmbEventHandleResult onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Use the original script-sound queue for Fleen playback. */
	bool usesOriginalScriptSoundQueue() const override { return true; }
	/** Return Fleen's release-specific script-sound priority ranges. */
	const ZoombiniPage::ScriptSoundPriorityRanges &getScriptSoundPriorityRanges() const override;
	/** Return Speech for authored Zoombini and Fleen vocal reactions while retaining movement sounds as SFX. */
	Audio::Mixer::SoundType getFeatureSoundType(const ZmbFeature *feature, ZmbResource sndRes) const override;
	/** Resolve a page-local Zoombini reaction voice sound ID. */
	ZmbResource resolveSnoidVoiceResource(int16 soundId) const override;
	/** Correct the optional Fleens tree-descent composite poses. */
	void adjustSnoidScriptHotspotForRender(const ZmbSnoid *snoid, ZmbHotspot &hotspot) const override;

private:
	/** Selects one of the authored custom Fleen SCRS animation families. */
	enum class FleenActionCode : int16 {
		/** Alias of the default SCRS 4000 pose; the current state machine does not produce this code. */
		kDefaultAnimationAlias01 = 1,
		/** Select the default SCRS 4000 pose as the non-fidget idle outcome. */
		kIdlePose02 = 2,
		/** Play the SCRS 4001 idle fidget. */
		kIdleFidget03 = 3,
		/** Select SCRS 4002; the current state machine does not produce this code. */
		kAlternateAnimation04 = 4,
		/** Start the selected Fleen's boarding movement with SCRS 4003. */
		kStartBoarding05 = 5,
		/** Move the selected Fleen from its branch toward the escape hand-off point. */
		kApproachEscapePoint06 = 6,
		/** React the remaining target Fleens after the first target match. */
		kReactAfterFirstTargetMatch07 = 7,
		/** React the remaining target Fleens after the second target match. */
		kReactAfterSecondTargetMatch08 = 8,
		/** Send the opening-lure Fleen toward the initial exit target. */
		kLeaveAfterInitialLure09 = 9,
		/** Send a submitted Fleen toward the ordinary escape target. */
		kLeaveWithSubmittedPair10 = 10,
		/** Send the final matched Fleen toward the completed-puzzle exit target. */
		kLeaveWithFinalMatchedPair11 = 11,
		/** Advance a retained Fleen during queued-pair departure processing. */
		kAdvanceQueuedPair12 = 12,
		/** Send the paired Fleen after the oldest full-queue pair is rejected. */
		kFollowFullQueueEviction13 = 13,
		/** Move a Fleen in the selected escape range offscreen. */
		kLeavePage14 = 14
	};

	/** Result of advancing one page-owned Fleen SCRS clock. */
	enum class FleenScrsAdvanceResult : byte {
		/** The Fleen's next animation deadline has not arrived. */
		kWaiting00 = 0,
		/** A new authored SCRS frame became current. */
		kFrameReady01 = 1,
		/** The current multi-frame SCRS reached its declared frame count. */
		kPlaybackComplete02 = 2
	};

	/** Select the authored Fleen SCRS root used to align raw frame coordinates. */
	enum class FleenScrsAnchorSelection : byte {
		/** Use the first hotspot in authored frame zero. */
		kFirstFrame00 = 0,
		/** Scan backward for the last frame whose first hotspot has a positive shape. */
		kLastAnchoredFrame01 = 1
	};

	/** Page-local Fleen, branch, lure, and Zoombini animation resources. */
	enum PageResourceId : int16 {
		kResBackground300 = 300,

		// Terrain bitmaps
		kResBitmapTerrain500 = 500,

		// Shape bitmaps
		kResBitmapShape0300_MainFeature = 300,
		kResBitmapShape0400_FeatureDetail = 400,
		kResBitmapShape1000_FeatureGroup0 = 1000,
		kResBitmapShape1100_FeatureGroup1 = 1100,
		kResBitmapShape1200_FeatureGroup2 = 1200,
		kResBitmapShape4000_FleenCreature = 4000,

		kResRegs4000_FleenRegistration = 4000,
		kResRegs4001_FleenRegistrationAlt = 4001,
		kResRegs5000_SpecialPositions = 5000,
		kResRegs5001_NormalPositions = 5001,

		kResScrb1000_MainFeatureBase = 1000,
		kResScrb1004_ExitBase = 1004,
		kResScrb1100_SpecialFeature = 1100,
		kResScrb1200_HideSpotBase = 1200,

		/** Default SCRS animation used before an action-specific sequence is selected. */
		kResScrs4000_FleenAnimationBase = 4000,
		kResScrs4001_FleenAnimation = 4001,
		kResScrs4002_FleenAnimation = 4002,
		kResScrs4003_FleenAnimation = 4003,
		kResScrs4020_Action7Base = 4020,
		kResScrs4021_Action8Base = 4021,
		kResScrs4025_Action9FeetBase = 4025,
		kResScrs4030_Action10FeetBase = 4030,
		kResScrs4035_Action11FeetBase = 4035,
		kResScrs4040_Action12FeetBase = 4040,
		kResScrs4045_Action13FeetBase = 4045,
		kResScrs4050_Action14FeetBase = 4050,
		kResScrs4056_Action14Position12 = 4056,
		kResScrs4057_Action14Position4 = 4057,
		kResScrs4058_Action14Position0 = 4058,
		kResScrs6000_RejectBase = 6000,
		kResScrs7000_NormalBase = 7000,
		kResScrs7005_MismatchCompleteBase = 7005,
		kResScrs7010_Action3 = 7010,
		kResScrs7011_Action4FeetBase = 7011,
		kResScrs7016_Event7016FeetBase = 7016,
		kResScrs7021_Event7021FeetBase = 7021,
		kResScrs7026_Event8FeetBase = 7026,
		kResScrs7027_Event8FeetSkates = 7027,
		kResScrs7030_Event8FeetLast = 7030,
		kResScrs7031_Event5FeetBase = 7031,
		kResScrs7036_Event1FeetBase = 7036,
		kResScrs7041_Event2FeetBase = 7041,

		// Sound resources.
		kResSound1800_ZoombiniVoiceBase = 1800,
		kResSound1814_ZoombiniVoiceLast = 1814,
		kResSound1899_ZoombiniVoiceRegistrationLast = 1899,
		kResSound4000_FleenMovementBase = 4000,
		kResSound4018_FleenMovementLast = 4018,
		kResSound4100_FleenVoiceBase = 4100,
		kResSound4124_FleenVoiceLast = 4124,

		// Sound ranges used by getScriptSoundPriority().
		kResSoundRange0099_PageSingle = 99,
		kResSoundRange0175_PageBase = 175,
		kResSoundRange0199_PageLast = 199,
		kResSoundRange0300_PageBase = 300,
		kResSoundRange0324_PageLast = 324,
		kResSoundRange0375_PageBase = 375,
		kResSoundRange0399_PageLast = 399,
		kResSoundRange0425_PageBase = 425,
		kResSoundRange0499_PageLast = 499,
		kResSoundRange1000_PuzzleSfxBase = 1000,
		kResSoundRange1002_PuzzleSfxLast = 1002,
		kResSoundRange1200_SpecialSfx = 1200,
		kResSoundRange1800_ZoombiniVoiceBase = 1800,
		kResSoundRange1899_ZoombiniVoiceRegistrationLast = 1899,
		kResSoundRange4099_PageLast = 4099,
		kResSoundRange4100_FleenVoiceBase = 4100,
		kResSoundRange4199_FleenVoiceRegistrationLast = 4199,
		kResSoundRange7000_AmbientBase = 7000,
		kResSoundRange7099_AmbientLast = 7099,
	};

	/** Event codes emitted while a Fleen pursues its assigned Snoid or departs. */
	enum FleenPursuitEventCode : int16 {
		kFleenPursuitEventCode000_ToggleFacing = 0,
		kFleenPursuitEventCode001_LoadOverlay = 1,
		kFleenPursuitEventCode002_LinkToSnoid = 2,
		kFleenPursuitEventCode060_StartFinalAction = 60,
		kFleenPursuitEventCode131_ArmDeparture = 131,
		kFleenPursuitEventCode136_ShowBoardingSnoid = 136,
		kFleenPursuitEventCode137_Deactivate = 137,
		kFleenPursuitEventCode140_LinkBeforeSnoid = 140,
		kFleenPursuitEventCode218_PlayReactionSound = 218
	};

	/** Event codes emitted while Fleens escape through the departure sequence. */
	enum FleenEscapeEventCode : int16 {
		kFleenEscapeEventCode000_ToggleFacing = 0,
		kFleenEscapeEventCode004_StartBoarding = 4,
		kFleenEscapeEventCode005_MoveToEscapePoint = 5,
		kFleenEscapeEventCode006_ProcessMismatchGroup = 6,
		kFleenEscapeEventCode007_MoveToExit = 7,
		kFleenEscapeEventCode008_LinkToEscapeOverlay = 8,
		kFleenEscapeEventCode009_LinkToFleen = 9,
		kFleenEscapeEventCode028_LinkToEscapeOverlayLate = 28,
		kFleenEscapeEventCode030_SpawnExit = 30,
		kFleenEscapeEventCode132_StartQueuedDeparture = 132,
		kFleenEscapeEventCode133_EscapeRangeNear = 133,
		kFleenEscapeEventCode134_EscapeRangeMiddle = 134,
		kFleenEscapeEventCode135_EscapeRangeFar = 135,
		kFleenEscapeEventCode137_DeactivateSnoid = 137
	};

	/**
	 * Selects the Snoid SCRS to start.
	 *
	 * @ref ZoombiniPuzzleFleens::mapEventToScrsId resolves this page-local
	 * selector to an SCRS resource.
	 * It differs from an animation callback eventCode emitted by a running SCRS.
	 */
	enum class SnoidScriptEventType : int16 {
		/** Feet-specific SCRS 7036-7040 starts the page-opening lure. */
		kInitialLure = 1,
		/** Starts the submitted Snoid/Fleen pair escape SCRS. */
		kSubmittedPairEscape = 2,
		/** Fixed SCRS 7010 follows the full-queue lead-in after pair relinking. */
		kFullQueueEvictionPairTransition = 3,
		/** Feet-specific SCRS 7011-7015 finishes the full-queue eviction. */
		kFullQueueEvictionHideSnoid = 4,
		/** Feet-specific SCRS 7031-7035 plays an idle Snoid fidget. */
		kIdleFidget = 5,
		/** Feet-specific SCRS 7026-7030 drives the final Go entry and next round. */
		kQueuedDepartureRoundTail = 8,
		/** Feet-specific SCRS 6000-6004 rejects the full-queue Snoid. */
		kFullQueueEvictionReject = 9,
		/** Feet-specific SCRS 7016-7020 advances the oldest full-queue pair. */
		kFullQueueEvictionLeadIn = 7016,
		/** Feet-specific SCRS 7021-7025 advances a non-tail Go entry. */
		kQueuedDepartureRoundFollower = 7021,
	};

	/**
	 * Next state after a Snoid SCRS animation event.
	 *
	 * Full-queue eviction callbacks run lead-in, pair-transition, rejection,
	 * and hide states in that order.
	 * The queued-departure tail callback separately schedules another Go round.
	 */
	enum class SnoidCallback {
		kNone,
		/** Handles the lure of one successfully submitted Snoid/Fleen pair. */
		kSubmitSnoidLureFleenPair,
		/** Completion of SCRS 7016-7020 starts the pair-transition SCRS 7010. */
		kFullQueueEvictionLeadIn,
		/** Completion of SCRS 7010 starts the rejection SCRS 6000-6004. */
		kFullQueueEvictionPairTransition,
		/** SCRS 6000-6004 starts @ref FleenActionCode::kFollowFullQueueEviction13 before SCRS 7011-7015. */
		kFullQueueEvictionRejectAndStartFleenAction,
		/** SCRS 7026-7030 on the final Go entry schedules the next round. */
		kQueuedDepartureRoundTail,
		/** Completion of SCRS 7011-7015 hides the Snoid evicted from the full queue. */
		kFullQueueEvictionHideSnoid
	};

	/** Generate the trait assignment used by the target Fleens. */
	void buildZmbTraitSetup();
	/** Create visual Fleen creatures and bind them to loaded Snoids. */
	void spawnFleenCreatures();
	/** Read a Fleen position table from a REGS resource. */
	void readFleenPositionRegs(int16 regsResId, Common::Array<Common::Point> &positions);
	/** Bind one Fleen SCRS and initialize its page-owned playback state. */
	void loadFleenCreatureScrs(FleenCreature &creature, int16 scrsResourceId, FleenCreature::Callback callback,
							   const Common::Point *targetPos = nullptr, bool callbackOnComplete = false);
	/** Find the authored first or final root hotspot used to align one Fleen SCRS. */
	static bool findFleenScrsAnchor(const ZmbScriptDecoder::DecodedScrs &decodedScrs, Common::Point &anchor,
									FleenScrsAnchorSelection selection);
	void startFleenAction(FleenCreature &creature, FleenActionCode actionCode, FleenCreature::Callback callback = FleenCreature::kCallbackNone,
						  const Common::Point *targetPos = nullptr, bool callbackOnComplete = false);
	/** Resolve @p actionCode to the corresponding Fleen SCRS resource. */
	int16 resolveFleenActionResource(FleenActionCode actionCode, const FleenCreature &creature) const;
	/** Return the shape-layer offset for a Fleen trait layout. */
	int16 getFleenBodyLayerOffset(const FleenCreature &creature, int layer) const;
	/** Find the Fleen coupled to a Snoid ID. */
	FleenCreature *findFleenBySnoid(uint16 snoidId);
	/** Find the const Fleen coupled to a Snoid ID. */
	const FleenCreature *findFleenBySnoid(uint16 snoidId) const;
	/** Find the Fleen that owns a feature runner. */
	FleenCreature *findFleenByFeature(const ZmbFeature *feature);
	/** Count active Snoids currently loaded on the page. */
	int16 countLoadedSnoids() const;
	/** Prepare Fleen hotspots before custom rendering. */
	bool fleenCreature_preRender(ZmbFeature *feature);
	/** Render one Fleen creature from its cached state. */
	ZmbRenderResult fleenCreature_render(ZmbFeature *feature);
	/** Advance one Fleen's frame and callback state. */
	void tickFleenCreature(FleenCreature &creature);
	/** Advance the active Fleen SCRS when its page-owned deadline becomes due. */
	FleenScrsAdvanceResult advanceFleenScrsFrame(FleenCreature &creature);
	/**
	 * Resolve one raw Fleen SCRS hotspot to its final tBMP 4000 shape and draw position.
	 * Applies the trait-layer offset and facing-pair selection to the raw shape,
	 * then subtracts the script-anchor and REGS registration offsets from its coordinates.
	 *
	 * @param creature Fleen state supplying traits, facing, and anchor translation.
	 * @param hotspot Raw SCRS hotspot to transform.
	 * @param shapeId Receives the one-based tBMP 4000 shape ID.
	 * @param drawPos Receives the screen position after anchor and registration corrections.
	 * @return Whether the hotspot resolves to a positive shape that should be drawn.
	 */
	bool resolveFleenRenderHotspot(const FleenCreature &creature, const ZmbHotspot &hotspot,
								   uint16 &shapeId, Common::Point &drawPos) const;
	/** Recompute the Fleen feature's transformed dirty and hit rectangles. */
	void updateFleenVisualCoverage(FleenCreature &creature);
	/** Draw one Fleen's transformed body and overlays. */
	void renderFleenCreature(const FleenCreature &creature);
	/** Process a Fleen movement animation event. */
	void processFleenMovementEvent(FleenCreature &creature, int16 eventCode);

	/** Map a Snoid script selector to the corresponding SCRS ID. */
	int16 mapEventToScrsId(SnoidScriptEventType scriptEventType, const ZmbSnoid *snoid) const;
	/** Start a Snoid script and register its page-local state-machine callback. */
	void startSnoidScript(ZmbSnoid &snoid, SnoidScriptEventType scriptEventType, SnoidCallback callback,
						  ZmbScrsCompletionMode completionMode = ZmbScrsCompletionMode::kReturnToIdle, const Common::Point *initPos = nullptr);
	/** Return the callback currently attached to a Snoid. */
	SnoidCallback getSnoidCallback(const ZmbSnoid &snoid) const;
	/** Attach a state-machine callback to a Snoid's active script. */
	void setSnoidCallback(const ZmbSnoid &snoid, SnoidCallback callback);
	/** Process an escape-range callback emitted by the departure script. */
	void processFleenEscapeRange(int16 eventCode);
	/**
	 * Process the page-opening lure or a successfully submitted pair's escape.
	 *
	 * Full-queue eviction uses @ref ZoombiniPuzzleFleens::processDepartureEvent.
	 */
	void processInitialLureOrSubmittedPairEscapeEvent(ZmbSnoid &snoid, int16 eventCode);
	/** Process the full-queue eviction or Go departure callback for one Snoid. */
	void processDepartureEvent(ZmbSnoid &snoid, SnoidCallback callback, int16 eventCode);
	/**
	 * Start one Go-triggered queued departure round.
	 *
	 * Followers use SCRS 7021-7025.
	 * The tail uses SCRS 7026-7030 and arms the next round at event 131.
	 */
	void processQueuedDepartureRound();
	/** Start the page-opening type-1 lure script before player input. */
	void startInitialLureScript();
	/** Start the coupled Snoid/Fleen boarding animation. */
	void startBoardingAnimation();
	/** Start boarding feedback for a pair that is already in the departure queue. */
	void beginBoardingAnimation(ZmbSnoid &snoid, int16 fleenIndex, bool appendQueue);
	/** Mark all target Fleens escaped and enable the page departure. */
	void onFleenEscapeComplete();
	/** Spawn the feature used by the final escape animation. */
	void spawnFleenEscapeFeature();
	/** Resolve the current pointer drop into a Fleen target slot. */
	void endDrag(const Common::Point &dropPos) override;
	/** Resolve a drop directly onto a numbered Fleen slot. */
	void endDrag(int16 targetSlot);
	/** Shift the pending departure queue after one Snoid leaves. */
	void shiftDepartureQueue();

	/** Fixed page positions assigned to occupied Zoombinis in pack order. */
	static constexpr Common::Point kPackSnoidPositions[16]{
		Common::Point(238, 368),
		Common::Point(185, 417),
		Common::Point(155, 448),
		Common::Point(197, 396),
		Common::Point(160, 357),
		Common::Point(164, 384),
		Common::Point(150, 416),
		Common::Point(116, 357),
		Common::Point(130, 386),
		Common::Point(109, 418),
		Common::Point(117, 448),
		Common::Point(74, 348),
		Common::Point(89, 384),
		Common::Point(67, 418),
		Common::Point(76, 450),
		Common::Point(56, 379),
	};
	/**
	 * Page Help hit rectangle. It is instance-owned because Common::Rect
	 * requires runtime construction and ScummVM prohibits global C++ constructors.
	 */
	const Common::Rect _helpButtonRect = Common::Rect(560, 441, 599, 478);
	/** Active lure position used for Fleen matching. */
	static constexpr Common::Point kFleenLurePosition = Common::Point(438, 357);
	/** Initial lure position before the branch animation begins. */
	static constexpr Common::Point kFleenLureInitPosition = Common::Point(236, 395);
	/** Resource tBMP 4000 shape offsets for hair values 1 through 5; index 0 has no shape. */
	static constexpr int16 kFleenHairShapeOffsets[6]{
		0,
		185,
		203,
		221,
		239,
		257,
	};
	/** Resource tBMP 4000 shape offsets for eye values 1 through 5; index 0 has no shape. */
	static constexpr int16 kFleenEyeShapeOffsets[6]{
		0,
		90,
		109,
		128,
		147,
		166,
	};
	/** Resource tBMP 4000 shape offsets for nose values 1 through 5; index 0 has no shape. */
	static constexpr int16 kFleenNoseShapeOffsets[6]{
		0,
		15,
		30,
		45,
		60,
		75,
	};
	/** Resource tBMP 4000 shape offsets for feet values 1 through 5; index 0 has no shape. */
	static constexpr int16 kFleenFeetShapeOffsets[6]{
		0,
		275,
		290,
		305,
		328,
		347,
	};

	/** Whether all three target Fleens have been matched; Go input has a separate opening/departure gate. */
	bool _allTargetFleensMatched = false;
	/** Whether the opening sequence has released Go input and normal idle-fidget progression. */
	bool _goInputEnabled = false;
	/** Whether the player may select and submit a Snoid-Fleen pair after the opening lure. */
	bool _pairSelectionInputEnabled = true;
	/**
	 * Whether the next submitted Snoid-Fleen pair may start boarding.
	 * Pair selection remains available while this scheduler gate is disabled.
	 */
	bool _boardingStartEnabled = true;
	/** Whether escape overlay 1200 has been activated for the submitted-pair script path. */
	bool _escapeOverlayLoaded = false;
	/** Whether Snoid event 4 has armed the link to escape overlay 1202 for consumption at event 0. */
	bool _snoidRaftOverlayLinkPending = false;
	/** Whether Snoid event 7 has armed the paired Fleen link to escape overlay 1202. */
	bool _fleenRaftOverlayLinkPending = false;
	/** Whether Go or the SCRS 7026-7030 tail has requested the next submitted-pair departure round. */
	bool _departureRoundPending = false;
	/** Whether event 132 may process the most recently submitted pair and any full-queue eviction. */
	bool _submittedPairQueueEventPending = false;
	/** Whether the full-queue eviction lead-in script has completed. */
	bool _fullQueueEvictionLeadInComplete = false;
	/** Whether the full-queue eviction pair-transition script has completed. */
	bool _fullQueueEvictionPairTransitionComplete = false;
	/** Whether the full-queue eviction rejection script has completed. */
	bool _fullQueueEvictionRejectScriptComplete = false;
	/** Number of target pairs already matched, from zero through three. */
	int16 _matchedTargetCount = 0;
	/** One-based occupied-pack ordinals of remaining target Snoids; zero means matched or absent. */
	int16 _targetSnoidOrdinals[3] = {0, 0, 0};
	/** Serialized value rotations, from one through five, applied to each source Zoombini trait. */
	byte _traitValueRotations[4] = {0, 0, 0, 0};
	/** Runtime destination @ref ZmbTrait::TraitKind for each source Zoombini trait. */
	ZmbTrait::TraitKind _traitDestinationKinds[4] = {ZmbTrait::kTraitHair, ZmbTrait::kTraitEyes, ZmbTrait::kTraitNose, ZmbTrait::kTraitFeet};
	/** Zoombini IDs for up to seven accepted lure submissions; a seventh submission evicts the oldest pair. */
	uint16 _submittedSnoidQueue[7] = {};
	/** Fleen indices paired one-to-one with @ref _submittedSnoidQueue. */
	int16 _submittedFleenQueue[7] = {};
	/** Number of accepted pairs currently retained in the submission queue. */
	int16 _submittedPairCount = 0;
	/** Zoombini ID accepted for the next boarding sequence, or zero when none is pending. */
	uint16 _pendingBoardingSnoidId = 0;
	/** Fleen index accepted for the next boarding sequence, or -1 when none is pending. */
	int16 _pendingBoardingFleenIndex = -1;
	/** Zoombini ID owned by the active boarding sequence. */
	uint16 _boardingSnoidId = 0;
	/** Fleen index owned by the active boarding sequence. */
	int16 _boardingFleenIndex = -1;
	/** Zoombini ID of the oldest pair selected for full-queue eviction. */
	uint16 _evictionSnoidId = 0;
	/** Fleen index of the oldest pair selected for full-queue eviction. */
	int16 _evictionFleenIndex = -1;
	/** Number of idle Zoombinis to select for one post-escape fidget pass. */
	int16 _idleSnoidFidgetTargetCount = 0;
	// Fleens owns the celebration candidate set and completion path.
	// Matching timer fields elsewhere do not define a reusable state machine.
	/** Number of idle Zoombini fidget scripts started in the current pass. */
	int16 _idleSnoidFidgetStartedCount = 0;
	/** Page frame of the last idle Zoombini fidget selection. */
	uint32 _idleSnoidFidgetLastSelectionFrame = 0;
	/** Minimum page-frame interval between idle Zoombini fidget selections. */
	uint32 _idleSnoidFidgetSelectionInterval = 60;
	/** Non-repeating random pool state used to select idle Zoombinis. */
	uint32 _idleSnoidFidgetPoolState = 0;
	/** Number of Zoombini walk animations expected in the active Go departure round. */
	int16 _departureWalkPendingCount = 0;
	/** Number of Zoombini walk animations completed in the active Go departure round. */
	int16 _departureWalkCompletedCount = 0;
	/** Fleen index currently being dragged, or -1. */
	int16 _draggedFleenIndex = -1;
	/** Completion callback selected for each Zoombini ID from 10000 through 10015. */
	SnoidCallback _snoidScriptCallbacks[16] = {};

	/** SCRB 1000-1003 feature that displays target-match progress. */
	ZmbFeature *_matchProgressFeature = nullptr;
	/** Random SCRB 1004-1006 feature spawned for the bee and final escape. */
	ZmbFeature *_escapeExitFeature = nullptr;
	/** SCRB 1200-1206 overlay features used by the submitted-pair escape sequence. */
	ZmbFeature *_escapeOverlayFeatures[7] = {};
	/** Fleen creature state indexed by visual slot. */
	FleenCreature _fleenCreatures[16];
	/** Number of valid entries in @ref _fleenCreatures. */
	int16 _activeFleenCount = 0;
	/** Authored target Fleen branch-seat coordinates loaded from page REGS 5000. */
	Common::Array<Common::Point> _targetBranchPositions;
	/** Authored non-target Fleen branch-seat coordinates loaded from page REGS 5001. */
	Common::Array<Common::Point> _normalBranchPositions;
	/** Registration points for Fleen body shapes. */
	ZmbShapeOffsetRegs _fleenShapeRegs;
};

} // End of namespace Mohawk

#endif
