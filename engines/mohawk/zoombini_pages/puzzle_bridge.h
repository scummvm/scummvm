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

#ifndef MOHAWK_ZOOMBINI_PAGES_PUZZLE_BRIDGE_H
#define MOHAWK_ZOOMBINI_PAGES_PUZZLE_BRIDGE_H

#include "mohawk/zoombini_pages/puzzle_base.h"

namespace Mohawk {

/**
 * Allergic Cliffs puzzle page (@ref ZoombiniPageType::kBridge).
 * Route 1, Puzzle 1
 *
 * The bridge has two lanes.
 * The Allergic Cliffs sneeze at Zoombinis with certain traits.
 * The player must drag each Zoombini to the correct lane.
 */
class ZoombiniPuzzleBridge : public ZoombiniPuzzle {
public:
	/** Create the Allergic Cliffs puzzle page. */
	ZoombiniPuzzleBridge(MohawkEngine_Zoombini *vm);
	/** Release bridge runners, animations, and page-owned resources. */
	~ZoombiniPuzzleBridge() override;
	/** Static puzzle name used by debug answer descriptions. */
	static constexpr const char *kPageName = ZmbXferRouteInfo::kXferPageNameAllergicCliffs;
	static constexpr int kRouteNumber = 1;
	static constexpr int kRoutePuzzleIdx = 1;
	/** Return the puzzle name used by debug answer descriptions. */
	const char *getPageName() const override { return kPageName; }
	int getRouteNumber() const override { return kRouteNumber; }
	int getRoutePuzzleIdx() const override { return kRoutePuzzleIdx; }

	/** Generate the bridge rule and initialize the crossing state. */
	void open() override;
	/** Select the bridge puzzle music. */
	void setBackgroundMusic() override;
	/** Select the bridge background bitmap. */
	void setBackgroundBitmap() override;
	/** Initialize puzzle states and departure transfer source. */
	void initStates() override;
	/** Load bridge lanes, cliff, water, and Zoombini features. */
	void loadFeatures() override;
	/** Preserve the original absence of an Allergic Cliffs F1 help prompt. */
	void initHelpPrompt() override;
	/** Restart the global Snoid fidget inactivity period after Allergic Cliffs loads. */
	bool requiresFidgetActivityResetOnLoad() const override { return true; }

	/** Arbitrate every embedded Bridge sound through the original page-wide queue. */
	bool usesOriginalScriptSoundQueue() const override { return true; }
	/** Return the original Allergic Cliffs script-sound range priority. */
	const ZoombiniPage::ScriptSoundPriorityRanges &getScriptSoundPriorityRanges() const override;
	/** Advance queued crossings and reactions after the current render. */
	void onPostRenderFrame() override;
	/** Schedule Bridge celebrations after the shared ambient driver. */
	void onPostAmbientFrame() override;
	/** Process lane and cliff animation callbacks. */
	void onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) override;

	/** Start a Zoombini drag or handle bridge control input. */
	ZmbEventHandleResult onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Update lane hover and active drag feedback. */
	ZmbEventHandleResult onMouseMove(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Resolve a dropped Zoombini into a lane or return position. */
	void endDrag(const Common::Point &dropPos) override;

protected:
	/** Allergic Cliffs actions accepted by the built-in debug console and keyboard dispatcher. */
	enum class BuiltinDebugAction {
		kInvalid,
		kAcceptance,
		kRoute
	};
	static constexpr const char *kBuiltinDebugActionAcceptance = "acceptance";
	static constexpr const char *kBuiltinDebugActionRoute = "route";
	/** Begin departure with only accepted lane occupants marked as occupied. */
	void onGoButtonActivated() override;
	/** Prepare a complete departure for the debug finish command. */
	void debugPrepareForDeparture() override;
	/** Describe the generated bridge toll rule for diagnostics. */
	Common::String debugGetAnswer() const override;
	/** Describe the last rejected Snoid trait and script selected by the bridge. */
	Common::String debugGetLastRejectRoute() const;
	/** Describe the Allergic Cliffs built-in debug actions. */
	Common::String debugGetBuiltinDebugCommandHelp() const override;
	/** Run one Allergic Cliffs built-in debug action. */
	bool debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output) override;
	/** Handle Allergic Cliffs built-in debug keys. */
	ZmbEventHandleResult onDebugKeyDown(const Common::KeyState &kbd) override;
	/** Parse one console-facing Allergic Cliffs action name. */
	static BuiltinDebugAction parseBuiltinDebugAction(const Common::String &action);
	/** Run one typed Allergic Cliffs built-in debug action. */
	bool runBuiltinDebugAction(BuiltinDebugAction action, Common::String &output);
	/** Report bridge mistake-budget usage for diagnostics. */
	ZmbChanceInfo debugGetChances() const override;
	/** Allergic Cliffs supports debugger chance adjustment. */
	bool debugCanSetChances() const override { return true; }
	/** Set bridge mistake-budget usage for diagnostics. */
	bool debugSetChances(int16 remaining) override;

	/**
	 * Build the trait toll table and select the winning combination.
	 *
	 * Fill @ref ZoombiniPuzzleBridge::_allergyRuleTraitKindCodes and @ref ZoombiniPuzzleBridge::_allergyRuleTraitValues.
	 */
	void buildTraitTollTable();

	/**
	 * Values accepted by one trait category in a Bridge rule.
	 * A zero second value represents a rule with one accepted value.
	 * Both values are zero when the category does not participate in the rule.
	 */
	struct TraitValueSet {
		/** First accepted trait value, or zero when the category is unused. */
		byte firstValue;
		/** Optional second accepted trait value, or zero for a one-value rule. */
		byte secondValue;

		/** Return whether this category has no accepted trait values. */
		bool isEmpty() const { return firstValue == 0 && secondValue == 0; }
	};

	/**
	 * One candidate Bridge toll rule.
	 * Each field stores the accepted values for its named trait category.
	 */
	struct TraitRuleDescriptor {
		/** Accepted hair trait values. */
		TraitValueSet hairValues = {};
		/** Accepted eye trait values. */
		TraitValueSet eyesValues = {};
		/** Accepted nose trait values. */
		TraitValueSet noseValues = {};
		/** Accepted foot trait values. */
		TraitValueSet feetValues = {};

		/**
		 * Create a rule that accepts one value for one trait category.
		 * @param traitKind Trait category to populate.
		 * @param traitValue Sole accepted value for @p traitKind.
		 * @return A descriptor with every other category empty.
		 */
		static TraitRuleDescriptor withFirstValue(ZmbTrait::TraitKind traitKind, byte traitValue);
		/**
		 * Return a copy with every first accepted value multiplied by @p scale.
		 * Second accepted values remain unchanged.
		 * @param scale Multiplier applied to each first accepted value.
		 * @return The scaled descriptor copy.
		 */
		TraitRuleDescriptor scaledFirstValues(byte scale) const;
		/**
		 * Add each first accepted value from @p other to this descriptor.
		 * Second accepted values remain unchanged.
		 * @param other Descriptor that supplies the addends.
		 */
		void addFirstValues(const TraitRuleDescriptor &other);
		/**
		 * Return the values accepted for one trait category.
		 * @param traitKind Trait category to inspect.
		 * @return The category's accepted-value set.
		 */
		TraitValueSet getValues(ZmbTrait::TraitKind traitKind) const;
		/**
		 * Set one accepted value and clear the optional second value.
		 * @param traitKind Trait category to update.
		 * @param traitValue Sole accepted value for @p traitKind.
		 */
		void setFirstValue(ZmbTrait::TraitKind traitKind, byte traitValue);
		/**
		 * Replace the values accepted for one trait category.
		 * @param traitKind Trait category to update.
		 * @param values Replacement accepted-value set.
		 */
		void setValues(ZmbTrait::TraitKind traitKind, const TraitValueSet &values);
	};

	/** All pairs of distinct trait values used by Level 2 rules. */
	static constexpr TraitValueSet kLevel2AlternativeValueTable[10]{
		{1, 2},
		{1, 3},
		{1, 4},
		{1, 5},
		{2, 3},
		{2, 4},
		{2, 5},
		{3, 4},
		{3, 5},
		{4, 5},
	};
	/** Unit descriptors for the first varying trait category in each Level 3 group. */
	static const TraitRuleDescriptor kLevel3BaseTable[6];
	/** Unit descriptors for the second varying trait category in each Level 3 group. */
	static const TraitRuleDescriptor kLevel3StepTable[6];
	/** Trait category omitted from each Level 4 three-category rule group. */
	static constexpr ZmbTrait::TraitKind kLevel4ExcludedTraitTable[4]{
		ZmbTrait::kTraitHair,
		ZmbTrait::kTraitEyes,
		ZmbTrait::kTraitNose,
		ZmbTrait::kTraitFeet,
	};

	/** One-based Bridge lane identity carried by the pending-drop and active-crossing state. */
	enum class BridgeLane : int16 {
		/** No lane has been selected. */
		kNone00 = 0,
		/** Upper bridge lane. */
		kUpper01 = 1,
		/** Lower bridge lane. */
		kLower02 = 2
	};

	/** Return the trait category in the original toll-rule selection order: feet, nose, eyes, hair. */
	static ZmbTrait::TraitKind traitKindFromRuleSlot(int16 slot);
	/** Return the one-based page code used by the Bridge requirement arrays. */
	static int16 requirementTraitKindCode(ZmbTrait::TraitKind traitKind);
	/** Match a Bridge rule candidate against canonical Zoombini traits. */
	static bool traitMatchesDescriptor(const ZmbTrait &trait, const TraitRuleDescriptor &descriptor, bool matchSecondValues);
	/** Convert a selected Bridge lane to its zero-based position and DRAW_ON_REG slot index. */
	static int16 bridgeLaneToIdx(BridgeLane lane);
	/** Run the post-ambient non-repeating celebration scheduler. */
	void scheduleCelebration();
	/**
	 * Test whether a bridge lane rejects a Zoombini's traits.
	 *
	 * @param trait The Zoombini's traits.
	 * @param targetLane Lane being tested.
	 * @return true if the selected lane rejects the Zoombini.
	 */
	bool testTraitMatch(const ZmbTrait &trait, BridgeLane targetLane) const;
	/**
	 * Collect traits from all active pack Zoombinis.
	 * @param outTraits Output array of traits in hair/eyes/nose/feet order.
	 * @return Number of entries written.
	 */
	int16 collectZmbTraits(Common::Array<ZmbTrait> &outTraits) const;

	// Button rendering callback
	/** Prepare the Bridge lane button hotspots. */
	void bridgeButtons_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** Handle a pointer press on a Bridge control. */
	ZmbEventHandleResult bridgeButtons_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);

	// Bridge visual rendering callback
	/** Render the Bridge visual overlay and lane segments. */
	ZmbRenderResult bridgeVisuals_render(ZmbFeature *feature);
	/** Record Bridge visual coverage after rendering. */
	void bridgeVisuals_postRender(ZmbFeature *feature);
	/** Reload SCRB animation data on an existing feature. */
	void reloadScrbAnimation(uint16 featureId, int16 newScrbId);
	/** Rebuild the cliff-gate runner at the current peg-loss stage. */
	void refreshPegDisplay();
	/** Find the reject return seat inside the saved lane-start rectangle. */
	Common::Point findRejectReturnPosition(ZmbSnoid *snoid);

	/**
	 * Bridge interpretation of @ref ZmbSnoid::_runnerStatus.
	 * The same byte has different meanings on other pages; keep this enum page-local.
	 */
	enum SnoidRunnerStatus : byte {
		/** Available on the input bank for another submission. */
		kSnoidRunnerStatus00_Available = 0,
		/** Accepted and traversing the bridge. */
		kSnoidRunnerStatus01_Crossing = 1,
		/** Settled on the far side and no longer draggable. */
		kSnoidRunnerStatus02_Arrived = 2
	};

	/**
	 * Event codes from crossing Snoid SCRS playback.
	 * The raw SCRS terminator is adjusted before dispatch.
	 */
	enum LaneEventCode : int16 {
		kLaneEventCode001_LowerStart = 1,
		kLaneEventCode002_UpperStart = 2,
		kLaneEventCode003_LowerArrival = 3,
		kLaneEventCode004_LowerStart = 4,
		kLaneEventCode005_UpperStart = 5,
		kLaneEventCode006_UpperArrival = 6,
		kLaneEventCode010_StartRejectTraitScript = 10,
		kLaneEventCode020_FinishRejectTraitScript = 20
	};

	/** Event codes from cliff entrance SCRB playback. */
	enum EntranceEventCode : int16 {
		kEntranceEventCode000_Start = 0,
		kEntranceEventCode001_ShowHairRule = 1,
		kEntranceEventCode002_ShowEyesRule = 2,
		kEntranceEventCode003_ShowNoseRule = 3,
		kEntranceEventCode004_ShowFeetRule = 4,
		kEntranceEventCode005_ShowHairRuleAlternate = 5,
		kEntranceEventCode006_ShowFeetRuleAlternate = 6,
		kEntranceEventCode100_ShowWaterSplash = 100,
		kEntranceEventCode101_RestoreWaterOverlay = 101
	};

	/** Cliff reaction event retained until the rejected Snoid starts its throw script. */
	enum class RejectReactionCode : int16 {
		/** No cliff reaction is waiting to be consumed. */
		kNone00 = 0,
		/** Primary hair-rule event, which selects the default reject script group. */
		kPrimaryHairRule01 = 1,
		/** Eye-rule event, which selects the eye reject script group. */
		kEyesRule02 = 2,
		/** Nose-rule event, which selects the nose reject script group. */
		kNoseRule03 = 3,
		/** Foot-rule event, which selects the foot reject script group. */
		kFeetRule04 = 4,
		/** Alternate hair-rule event, which selects the hair reject script group. */
		kAlternateHairRule05 = 5,
		/** Alternate foot-rule event, which selects the default reject script group. */
		kAlternateFeetRule06 = 6,
		/** No reject reaction has been recorded by the built-in diagnostic. */
		kNotRecordedMinus01 = -1
	};

	/** Process a lane crossing step callback. */
	void processLaneStepEvent(ZmbFeature *snoidFeature, int16 stepCode);
	/** Start the authored reject throw script for a Snoid. */
	bool startRejectThrowScript(ZmbSnoid *snoid);
	/** Process cliff entrance events from SCRB animation playback. */
	void processEntranceEvent(int16 eventId, ZmbFeature *eventSource);
	/** Count accepted or crossing pack Snoids. */
	int16 countAcceptedPackSnoids() const;
	/** Find an idle pack snoid, optionally preferring a specific ID. */
	ZmbSnoid *findIdlePackSnoid(uint16 preferredId = 0);
	/**
	 * Return true if a lane's shared DRAW_ON_REG seat can store another queued Zoombini.
	 * Starting the stored entry remains subject to the bridge reuse delay and active reject gate.
	 */
	bool canAcceptDropOnLane(BridgeLane lane) const;
	/**
	 * Before Go departure, mark only accepted/right-bank Zoombinis as occupied.
	 * Shared cleanup then routes the non-occupied left-bank Zoombinis back to
	 * the route's resting pack.
	 */
	void markAcceptedSnoidsForDeparture();
	/** Return the drag boundary for the left, pre-crossing interaction side of the chasm. */
	const Common::Rect &getDragConstraintRect() const override;

	/** Page-local bridge backgrounds, lanes, animations, and sounds. */
	enum PageResourceId : int16 {
		// Background
		kResBackground1000 = 1000,

		// Shape bitmaps
		kResBitmapShape1100 = 1100,
		kResBitmapShape1200 = 1200,
		kResBitmapShape1300 = 1300,
		kResBitmapShape1400_GoMapButtons = 1400,

		// Terrain bitmaps
		kResBitmapTerrain1600 = 1600,

		// SCRB features - overlays
		kResScrb1100_Main = 1100,
		kResScrb1101_Overlay = 1101,
		kResScrb1102_Overlay = 1102,
		kResScrb1103_Overlay = 1103, // special: water overlay
		kResScrb1104_Overlay = 1104,
		kResScrb1105_Overlay = 1105, // cliff gate
		// SCRB features - SHPL (shapes loaded separately)
		kResScrb1106_Water = 1106, // 0x452
		// SCRB features - cliff/gate animations
		kResScrb1200_CliffLane1 = 1200, // 0x4B0
		kResScrb1201_CliffLane2 = 1201, // 0x4B1
		kResScrb1202_CliffGate = 1202,  // 0x4B2
		kResScrb1208_CliffGateRejectBase = 1208,
		kResScrb1214_CliffRejectLane2 = 1214,
		kResScrb1215_CliffGateRejectBase = 1215,
		kResScrb1221_CliffEntrance = 1221,
		kResScrb1222_CliffRejectLane1 = 1222,
		kResScrb1223_CliffGateRejectAltBase = 1223,
		kResScrb1229_CliffGateRejectBase = 1229,
		kResScrb1235_CliffGateEntrance = 1235,
		kResScrb1236_WaterSplash = 1236,
		kResScrb1237_CliffGateRejectBase = 1237,
		kResScrb1243_CliffGateRejectAltBase = 1243,
		kResScrb1248_CliffLaneLast = 1248,
		// SCRB features - bridge segments
		kResScrb1300_Segment0 = 1300,
		kResScrb1301_Segment1 = 1301,

		// SCRS snoid scripts - reject pool
		kResScrs1000_RejectBase = 1000,
		kResScrs1004_RejectHair = 1004,
		kResScrs1008_RejectNose = 1008,
		kResScrs1012_RejectEyes = 1012,
		kResScrs1016_RejectDefault = 1016,
		// SCRS snoid scripts - normal pool
		kResScrs2000_NormalBase = 2000,
		kResScrs2005_NormalBase = 2005,
		kResScrs2010_RejectBase = 2010,
		kResScrs2015_NormalBase = 2015,
		kResScrs2019_CelebrationBase = 2019,
		kBridgeRejectScrsCount = 20,
		kBridgeNormalScrsCount = 25,

		// Sound ranges used by getScriptSoundPriority().
		kResSoundRange0175_PageRangeBase = 175,
		kResSoundRange0199_PageRangeLast = 199,
		kResSoundRange1000_SystemRangeBase = 1000,
		kResSoundRange1099_SystemRangeLast = 1099,
		kResSoundRange1200_SystemRangeBase = 1200,
		kResSoundRange1201_SystemRangeLast = 1201,
		kResSoundRange1202_SystemRangeBase = 1202,
		kResSoundRange1213_SystemRangeLast = 1213,
		kResSoundRange1214_SystemRangeBase = 1214,
		kResSoundRange1215_SystemRangeLast = 1215,
		kResSoundRange1216_SystemRangeBase = 1216,
		kResSoundRange1226_SystemRangeLast = 1226,
	};

	/**
	 * Fixed waiting positions on the left, pre-crossing side of the chasm.
	 * Pack Snoids start here, and a removed pending lane drop returns to an empty position in this table.
	 */
	static constexpr Common::Point kLeftBankWaitingPositions[16]{
		Common::Point(176, 304),
		Common::Point(169, 327),
		Common::Point(144, 283),
		Common::Point(147, 355),
		Common::Point(124, 318),
		Common::Point(119, 379),
		Common::Point(108, 284),
		Common::Point(99, 345),
		Common::Point(88, 414),
		Common::Point(69, 262),
		Common::Point(79, 303),
		Common::Point(78, 370),
		Common::Point(61, 346),
		Common::Point(45, 301),
		Common::Point(36, 359),
		Common::Point(30, 404),
	};

	/** Upper- and lower-lane staging positions at the left-bank bridge entrances before crossing starts. */
	static constexpr Common::Point kLaneEntryPositions[2]{
		Common::Point(116, 104),
		Common::Point(128, 203),
	};

	/** Right-bank arrival positions for Snoids that finish the upper lane, in stacking order. */
	static constexpr Common::Point kUpperLaneArrivalPositions[16]{
		Common::Point(618, 45),
		Common::Point(582, 49),
		Common::Point(552, 36),
		Common::Point(524, 32),
		Common::Point(493, 25),
		Common::Point(464, 27),
		Common::Point(422, 36),
		Common::Point(618, 86),
		Common::Point(588, 81),
		Common::Point(556, 76),
		Common::Point(615, 129),
		Common::Point(580, 122),
		Common::Point(550, 116),
		Common::Point(522, 112),
		Common::Point(493, 106),
		Common::Point(530, 69),
	};

	/** Right-bank arrival positions for Snoids that finish the lower lane, in stacking order. */
	static constexpr Common::Point kLowerLaneArrivalPositions[16]{
		Common::Point(615, 342),
		Common::Point(590, 332),
		Common::Point(579, 303),
		Common::Point(549, 290),
		Common::Point(522, 281),
		Common::Point(492, 271),
		Common::Point(621, 314),
		Common::Point(602, 283),
		Common::Point(573, 267),
		Common::Point(533, 248),
		Common::Point(622, 257),
		Common::Point(596, 242),
		Common::Point(561, 235),
		Common::Point(621, 197),
		Common::Point(594, 187),
		Common::Point(566, 178),
	};

	/** Drag boundary covering the left, pre-crossing interaction side of the chasm. */
	const Common::Rect _dragConstraintRect = Common::Rect(0, 0, 280, 480);
	/** Upper- and lower-lane regions on the left bank used to return a rejected Snoid. */
	const Common::Rect _rejectReturnRegions[2] = {
		Common::Rect(10, 50, 57, 105),
		Common::Rect(10, 165, 65, 212),
	};

	// --- Puzzle State ---
	/** Number of entries in the cliff-allergy rule. */
	byte _allergyRuleEntryCount = 0;
	/** One-based trait-kind codes for the cliff-allergy rule (1=hair, 2=eyes, 3=nose, 4=feet). */
	byte _allergyRuleTraitKindCodes[5] = {};
	/** Trait values paired with @ref ZoombiniPuzzleBridge::_allergyRuleTraitKindCodes. */
	byte _allergyRuleTraitValues[5] = {};
	/** Whether the upper lane accepts Snoids that match at least one allergy-rule entry. */
	bool _matchingTraitsUseUpperLane = false;
	/** Whether Go is enabled after the first accepted lane arrival. */
	bool _goButtonEnabled = false;
	/** Failed crossing/peg-drop stage (0-6 max). */
	int16 _failedCrossingCount = 0;
	/** Number of Snoids whose crossing scripts have started but not completed logically. */
	int16 _activeCrossingCount = 0;
	/**
	 * Whether the oldest pending lane drop may start crossing the bridge.
	 * Player lane drops remain available while this queue-start gate is disabled.
	 */
	bool _queuedCrossingStartEnabled = true;
	/** Rejection result for the crossing most recently removed from the pending-drop queue. */
	bool _activeCrossingRejected = false;
	/** Lane used by the active crossing, or none before the first crossing starts. */
	BridgeLane _activeCrossingLane = BridgeLane::kNone00;
	/** Number of queued lane drops that have not started crossing (0-2). */
	int16 _pendingDropCount = 0;
	/** Lanes selected by the two-entry pending-drop FIFO. */
	BridgeLane _pendingDropLanes[2] = {};
	/** Pack Snoid IDs for the two-entry pending-drop FIFO. */
	int16 _pendingDropSnoidIds[2] = {};
	/** Whether the current drag removed the oldest pending drop rather than the second entry. */
	bool _dragRemovedOldestPendingDrop = false;
	/** Cached allergy-rule rejection results for the two-entry pending-drop FIFO. */
	bool _pendingDropRejected[2] = {false, false};
	/** Arrived Snoid IDs in upper-lane stacking order. */
	int16 _upperLaneArrivedSnoidIds[16] = {};
	/** Arrived Snoid IDs in lower-lane stacking order. */
	int16 _lowerLaneArrivedSnoidIds[16] = {};
	/** Number of Snoids committed to upper-lane arrival positions. */
	int16 _upperLaneArrivalCount = 0;
	/** Number of Snoids committed to lower-lane arrival positions. */
	int16 _lowerLaneArrivalCount = 0;
	/** Active reject-throw SCRS ID, or -1 before that phase has started. */
	int16 _activeRejectScrsId = -1;
	/** Cliff reaction waiting to select the reject-throw SCRS. */
	RejectReactionCode _pendingRejectReactionCode = RejectReactionCode::kNone00;
	/** Cliff-lane feature ID waiting for deferred reject-animation setup. */
	uint16 _pendingCliffLaneFeatureId = 0;
	/** Whether the player may pick up the rejected Snoid before its automatic return completes. */
	bool _rejectedSnoidPickupEnabled = false;
	/** Whether the sixth-rejection cliff collapse must be installed after the current render pass. */
	bool _terminalCliffTransitionPending = false;
	/** Number of celebration SCRSes requested by the arrival thresholds. */
	int16 _celebrationTargetCount = 0;
	/** Number of requested celebration SCRSes that have started. */
	int16 _celebrationStartedCount = 0;
	/** Absolute frame counter recorded at the most recent celebration attempt. */
	uint32 _lastCelebrationFrame = 0;
	/** Minimum frame interval between celebration attempts. */
	uint32 _celebrationFrameInterval = 120;
	/** Bit mask of pack indices already selected by the non-repeating celebration pool. */
	uint32 _celebrationUsedSnoidMask = 0;
	/**
	 * Pack Snoid ID currently being rejected, or 0 when none is active.
	 *
	 * Every pack Snoid dispatches through @ref ZoombiniPuzzleBridge::onFeatureAnimEvent(),
	 * including arrived Snoids playing celebration SCRSes.
	 * Tracking the designated crossing Snoid prevents an unrelated end event
	 * from applying the reject return-to-bank path.
	 */
	uint16 _rejectCrossingSnoidId = 0;
	/** Last cliff reaction shown by the built-in R debug key. */
	RejectReactionCode _lastRejectReactionCode = RejectReactionCode::kNotRecordedMinus01;
	/** Last rejected Snoid SCRS ID shown by the built-in R debug key. */
	int16 _lastRejectSnoidScrsId = -1;
	/** Absolute frame counter recorded when the most recent crossing started. */
	uint32 _lastCrossingStartFrame = 0;

	// --- Feature handles ---
	/** Feature IDs for the reusable cliff animation runners. */
	int16 _cliffLane1FeatureId = 0; // SCRB 1200
	int16 _cliffLane2FeatureId = 0; // SCRB 1201
	int16 _cliffGateFeatureId = 0;  // SCRB 1202
	int16 _cliffMainFeatureId = 0;  // SCRB 1105 cliff-gate overlay
	/** Feature ID for the SCRB 1103 water overlay runner. */
	int16 _waterOverlayFeatureId = 0;
};

} // End of namespace Mohawk

#endif
