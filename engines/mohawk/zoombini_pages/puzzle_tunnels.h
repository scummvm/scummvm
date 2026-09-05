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

#ifndef MOHAWK_ZOOMBINI_PAGES_PUZZLE_TUNNELS_H
#define MOHAWK_ZOOMBINI_PAGES_PUZZLE_TUNNELS_H

#include "mohawk/zoombini_pages/puzzle_base.h"

namespace Mohawk {

/**
 * Stone Cold Caves puzzle page (@ref ZoombiniPageType::kTunnels).
 * Route 1, Puzzle 2
 *
 * Players must guide Zoombinis through one of four tunnels, by matching their traits to the correct tunnel entrances.
 *
 * Difficulty levels:
 * Level 1: 4 physical lanes, one single-trait split (e.g., "has blue eyes")
 * Level 2: 4 tunnels active, two single-trait guards
 * Level 3: 4 tunnels active, two dual-trait guards (OR within category)
 * Level 4: 4 tunnels active, two cross-category dual-trait guards (OR)
 *
 */
class ZoombiniPuzzleTunnels : public ZoombiniPuzzle {
public:
	/** Create the Stone Cold Caves puzzle page. */
	ZoombiniPuzzleTunnels(MohawkEngine_Zoombini *vm);
	/** Release tunnel gates, guards, and Zoombini features. */
	~ZoombiniPuzzleTunnels() override;
	/** Static puzzle name used by debug answer descriptions. */
	static constexpr const char *kPageName = ZmbXferRouteInfo::kXferPageNameStoneColdCaves;
	static constexpr int kRouteNumber = 1;
	static constexpr int kRoutePuzzleIdx = 2;
	/** Return the puzzle name used by debug answer descriptions. */
	const char *getPageName() const override { return kPageName; }
	int getRouteNumber() const override { return kRouteNumber; }
	int getRoutePuzzleIdx() const override { return kRoutePuzzleIdx; }

	/** Generate tunnel guards and initialize the active pack. */
	void open() override;
	/** Select the Stone Cold Caves background bitmap. */
	void setBackgroundBitmap() override;
	/** Initialize page-specific state before loading features. */
	void initStates() override;
	/** Load tunnel gates, hints, guards, and Zoombini features. */
	void loadFeatures() override;
	/** Restart the global Snoid fidget inactivity period after Stone Cold Caves loads. */
	bool requiresFidgetActivityResetOnLoad() const override { return true; }
	/** Select one authored Stone Cold Caves F1 prompt after the fidget reset. */
	void initHelpPrompt() override;
	/** Schedule the entry actor and first idle deadline before enabling page input. */
	void activatePage() override;
	/** Return the original Stone Cold Caves script-sound range priority. */
	const ZoombiniPage::ScriptSoundPriorityRanges &getScriptSoundPriorityRanges() const override;
	/** Classify the four Rock Guards' localized dialogue separately from Tunnels effects. */
	Audio::Mixer::SoundType getFeatureSoundType(const ZmbFeature *feature, ZmbResource resource) const override;
	/** Advance tunnel entry, rejection, and departure state after rendering. */
	void onPostRenderFrame() override;
	/** Schedule tunnel celebrations after the shared ambient driver. */
	void onPostAmbientFrame() override;
	/** Process tunnel guard and Snoid animation callbacks. */
	void onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) override;

protected:
	/** Tunnels actions accepted by the built-in debug console and keyboard dispatcher. */
	enum class BuiltinDebugAction {
		kInvalid,
		kAcceptance,
		kCycleCrystal,
		kCycleFerrous,
		kCycleIgno,
		kCycleOnyx,
		kCelebrationPlusFour,
		kIdle,
		kEndGame,
		kAdvance,
		kEntry,
		kEntryFirstVisit
	};
	static constexpr const char *kBuiltinDebugActionAcceptance = "acceptance";
	static constexpr const char *kBuiltinDebugActionCycleCrystal = "cycle-crystal";
	static constexpr const char *kBuiltinDebugActionCycleFerrous = "cycle-ferrous";
	static constexpr const char *kBuiltinDebugActionCycleIgno = "cycle-igno";
	static constexpr const char *kBuiltinDebugActionCycleOnyx = "cycle-onyx";
	static constexpr const char *kBuiltinDebugActionCelebrationPlusFour = "celebration-plus-four";
	static constexpr const char *kBuiltinDebugActionIdle = "idle";
	static constexpr const char *kBuiltinDebugActionEndGame = "end-game";
	static constexpr const char *kBuiltinDebugActionAdvance = "advance";
	static constexpr const char *kBuiltinDebugActionEntry = "entry";
	static constexpr const char *kBuiltinDebugActionEntryFirstVisit = "entry-first-visit";
	/** Continue the selected gate farewell while a Go departure is active. */
	bool runsControllerDuringDeparture() const override { return true; }
	/** Stop ordinary ambient selection once departure has begun. */
	bool runsAmbientDuringDeparture() const override { return false; }
	/** Depart with Zoombinis that passed their tunnel guards. */
	void onGoButtonActivated() override;
	/** Describe the generated tunnel guards for diagnostics. */
	Common::String debugGetAnswer() const override;
	/** Report tunnel mistake-budget usage for diagnostics. */
	ZmbChanceInfo debugGetChances() const override;
	/** Stone Cold Caves supports debugger chance adjustment. */
	bool debugCanSetChances() const override { return true; }
	/** Set tunnel mistake-budget usage for diagnostics. */
	bool debugSetChances(int16 remaining) override;
	/** Run a Tunnels built-in debug action. */
	bool debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output) override;
	/** Describe Tunnels built-in debug actions. */
	Common::String debugGetBuiltinDebugCommandHelp() const override;
	/** Handle the Tunnels built-in debug keys. */
	ZmbEventHandleResult onDebugKeyDown(const Common::KeyState &kbd) override;
	/** Start a tunnel drop or handle a page control click. */
	ZmbEventHandleResult onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Resolve the selected tunnel for the current drag. */
	void endDrag(const Common::Point &dropPos) override;

private:
	/** Parse one console-facing Tunnels action name. */
	static BuiltinDebugAction parseBuiltinDebugAction(const Common::String &action);
	/** Run one typed Tunnels keyboard-debug branch. */
	bool runBuiltinDebugAction(BuiltinDebugAction action, Common::String &output);
	/**
	 * Tunnels interpretation of @ref ZmbSnoid::_runnerStatus.
	 * The same byte has different meanings on other pages; keep this enum page-local.
	 */
	enum SnoidRunnerStatus : byte {
		/** Available in the input-side pool for another submission. */
		kSnoidRunnerStatus00_Available = 0,
		/** Accepted into a gate and removed from the input-side pool. */
		kSnoidRunnerStatus01_EnteredGate = 1
	};

	/** Page-local tunnel gates, guards, hints, and rejection resources. */
	enum PageResourceId : int16 {
		kResBackground300 = 300,

		// Terrain bitmaps
		kResBitmapTerrain100 = 100,

		// Shape bitmaps
		kResBitmapShape0400 = 400,
		kResBitmapShape4000_Gate0 = 4000,
		kResBitmapShape4200_Gate1 = 4200,
		kResBitmapShape4400_Gate2 = 4400,
		kResBitmapShape4600_Gate3 = 4600,
		kResBitmapShape5000 = 5000,
		kResBitmapShape6000 = 6000,
		kResBitmapShape7000 = 7000,
		kResBitmapShape9000 = 9000,

		kResNode1000_WalkNetwork = 1000,

		kResScrb4000_Reject0 = 4000,
		kResScrb4001_Reject1 = 4001,
		kResScrb4002_Reject2 = 4002,
		kResScrb4003_Reject3 = 4003,
		kResScrb4004_Reject4 = 4004,
		kResScrb4005_Reject5 = 4005,
		kResScrb4006_Reject6 = 4006,
		kResScrb4007_Reject7 = 4007,
		kResScrb4008_Reject8 = 4008,
		kResScrb4009_Reject9 = 4009,
		kResScrb4010_Hint0 = 4010,
		kResScrb4011_Hint1 = 4011,
		kResScrb4012_Hint2 = 4012,
		kResScrb4013_Hint3 = 4013,
		kResScrb4014_Hint4 = 4014,
		kResScrb4015_Hint5 = 4015,
		kResScrb4016_Hint6 = 4016,
		kResScrb4017_Hint7 = 4017,
		kResScrb4018_Hint8 = 4018,
		kResScrb4019_Hint9 = 4019,
		kResScrb4020_Hint10 = 4020,
		kResScrb4021_Reject = 4021,
		kResScrb4022_Reject = 4022,
		kResScrb4023_Reject = 4023,
		kResScrb4024_Reject = 4024,
		kResScrb4025_Reject = 4025,
		kResScrb4026_Reject = 4026,
		kResScrb4027_Reject = 4027,
		kResScrb4028_Reject = 4028,
		kResScrb4029_Reject = 4029,
		kResScrb4030_Reject = 4030,
		kResScrb4031_Advance = 4031,
		kResScrb4032_Advance = 4032,
		kResScrb4033_Advance = 4033,
		kResScrb4034_Advance = 4034,
		kResScrb4035_Advance = 4035,
		kResScrb4036_Advance = 4036,
		kResScrb4037_Advance = 4037,
		kResScrb4038_Advance = 4038,
		kResScrb4199_RejectRangeLast = 4199,
		kResScrb4200_Reject0 = 4200,
		kResScrb4201_Reject1 = 4201,
		kResScrb4202_Reject2 = 4202,
		kResScrb4203_Reject3 = 4203,
		kResScrb4204_Reject4 = 4204,
		kResScrb4205_Reject5 = 4205,
		kResScrb4206_Reject6 = 4206,
		kResScrb4207_Reject7 = 4207,
		kResScrb4208_Reject8 = 4208,
		kResScrb4209_Reject9 = 4209,
		kResScrb4210_Reject10 = 4210,
		kResScrb4211_Reject11 = 4211,
		kResScrb4212_Reject12 = 4212,
		kResScrb4213_Reject13 = 4213,
		kResScrb4214_Reject14 = 4214,
		kResScrb4215_Reject15 = 4215,
		kResScrb4216_Reject = 4216,
		kResScrb4217_Idle = 4217,
		kResScrb4218_Idle = 4218,
		kResScrb4219_Idle = 4219,
		kResScrb4220_Idle = 4220,
		kResScrb4221_Init = 4221,
		kResScrb4222_Reject = 4222,
		kResScrb4223_Reject = 4223,
		kResScrb4224_Reject = 4224,
		kResScrb4225_Advance = 4225,
		kResScrb4226_Advance = 4226,
		kResScrb4399_RejectRangeLast = 4399,
		kResScrb4400_Reject0 = 4400,
		kResScrb4401_Reject1 = 4401,
		kResScrb4402_Reject2 = 4402,
		kResScrb4403_Reject3 = 4403,
		kResScrb4404_Reject4 = 4404,
		kResScrb4405_Hint = 4405,
		kResScrb4406_Hint = 4406,
		kResScrb4407_Hint = 4407,
		kResScrb4408_Reject = 4408,
		kResScrb4409_Reject = 4409,
		kResScrb4410_Reject = 4410,
		kResScrb4411_Reject = 4411,
		kResScrb4412_Reject = 4412,
		kResScrb4413_Reject = 4413,
		kResScrb4414_Reject = 4414,
		kResScrb4415_Reject = 4415,
		kResScrb4416_Reject = 4416,
		kResScrb4417_Hint = 4417,
		kResScrb4418_Hint = 4418,
		kResScrb4419_Reject = 4419,
		kResScrb4420_Reject = 4420,
		kResScrb4421_Reject = 4421,
		kResScrb4422_Reject = 4422,
		kResScrb4423_Reject = 4423,
		kResScrb4599_RejectRangeLast = 4599,
		kResScrb4600_Reject0 = 4600,
		kResScrb4601_Reject1 = 4601,
		kResScrb4602_Reject2 = 4602,
		kResScrb4603_Reject3 = 4603,
		kResScrb4604_Reject4 = 4604,
		kResScrb4605_Reject5 = 4605,
		kResScrb4606_Reject6 = 4606,
		kResScrb4607_Reject7 = 4607,
		kResScrb4608_Reject8 = 4608,
		kResScrb4609_Reject9 = 4609,
		kResScrb4610_Reject = 4610,
		kResScrb4611_Reject = 4611,
		kResScrb4612_Reject = 4612,
		kResScrb4613_Reject = 4613,
		kResScrb4614_Reject = 4614,
		kResScrb4615_Reject = 4615,
		kResScrb4616_Reject = 4616,
		kResScrb4617_Reject = 4617,
		kResScrb4699_RejectRangeLast = 4699,
		kResScrb5000_TunnelEntryBase = 5000,
		kResScrb6000_Controller = 6000,
		kResScrb6004_DoorRejectBase = 6004,
		kResScrb7000_MainPath = 7000,
		kResScrb7001_RockfallBase = 7001,
		kResScrb9000_Feedback = 9000,
		kResScrb9001_FeedbackBase = 9001,

		kResScrs8000_RejectBase = 8000,
		kResScrs8496_WalkBase = 8496,
		kResScrs8500_NormalBase = 8500,
		kResScrs8519_RejectWalkBase = 8519,
		kResScrs8559_AcceptWalkBase = 8559,

		// Sound resources.
		kResSound4000_GateBase = 4000,
		kResSound4000_CrystalSpeechBase = 4000,
		kResSound4038_CrystalSpeechLast = 4038,
		kResSound4200_FerrousSpeechBase = 4200,
		kResSound4227_FerrousSpeechLast = 4227,
		kResSound4400_IgnoSpeechBase = 4400,
		kResSound4427_IgnoSpeechLast = 4427,
		kResSound4600_OnyxSpeechBase = 4600,
		kResSound4618_OnyxSpeechLast = 4618,
		kResSound4699_GateLast = 4699,
		kResSound4700_Rockfall4Remaining = 4700,
		kResSound4701_Rockfall3Remaining = 4701,
		kResSound4702_Rockfall2Remaining = 4702,
		kResSound4703_Rockfall1Remaining = 4703,
		kResSound7000_AmbientBase = 7000,
		kResSound7099_AmbientLast = 7099,

		// Sound ranges used by getScriptSoundPriority().
		kResSoundRange0099_PageSingle = 99,
		kResSoundRange0175_PageRangeBase = 175,
		kResSoundRange0199_PageRangeLast = 199,
		kResSoundRange0425_PageRangeBase = 425,
		kResSoundRange0499_PageRangeLast = 499,
		kResSoundRange4000_GateBase = 4000,
		kResSoundRange4699_GateLast = 4699,
		kResSoundRange4700_RockfallBase = 4700,
		kResSoundRange4799_RockfallLast = 4799,
		kResSoundRange6000_SystemRangeBase = 6000,
		kResSoundRange6099_SystemRangeLast = 6099,
		kResSoundRange7000_AmbientBase = 7000,
		kResSoundRange7099_AmbientLast = 7099,
		kResSoundRange8500_PageRangeBase = 8500,
		kResSoundRange8599_PageRangeLast = 8599,
	};

	/** Tunnel gate and Snoid animation callbacks. */
	enum TunnelEventCode : int16 {
		kTunnelEventCode000_ToggleSnoidFacing = 0,
		kTunnelEventCode010_StartRejectReturn = 10,
		kTunnelEventCode013_StartGateActor = 13
	};

	/** Materialize the active pack at the input-side cave seats. */
	void loadZoombinisFromPack();

	/** Generate tunnel rules based on difficulty level. */
	void generateRules();

	/** Level 1: Single trait rule. */
	void setupLevel1_singleTrait();
	/** Level 2: Dual guards, single trait each. */
	void setupLevel2_dualSingleTrait();
	/** Level 3: Dual guards, two conditions each (OR within same category). */
	void setupLevel3_dualDoubleTrait();
	/** Level 4: Dual guards, cross-category conditions (OR). */
	void setupLevel4_crossCategoryTrait();

	/**
	 * Evaluate if a Zoombini matches a tunnel rule for a given zone.
	 * @param snoid The Zoombini to evaluate
	 * @param dropZone The target tunnel zone (1-4)
	 * @param guardSideMatch [out] Whether the selected route uses guard A's matching side
	 * @return true if the Zoombini does NOT match the zone rule (rejection)
	 */
	bool evaluateRule(ZmbSnoid *snoid, int16 dropZone, bool &guardSideMatch);

	/** Find which tunnel zone a position corresponds to. @return 1-4 or 0. */
	int16 getDropZone(const Common::Point &pos);

	/** Build animation queue entry for a Zoombini placement. */
	void handleZoombiniPlacement(ZmbSnoid *snoid, int16 zone,
								 bool isRejection, bool guardAMatch);

	// ========================================
	// Static Data Tables
	// ========================================

	/** Authored input-side seat positions for the sixteen active-pack slots. */
	static constexpr Common::Point kSnoidPositions[16]{
		Common::Point(399, 402),
		Common::Point(367, 398),
		Common::Point(337, 397),
		Common::Point(306, 400),
		Common::Point(274, 400),
		Common::Point(240, 403),
		Common::Point(381, 424),
		Common::Point(351, 424),
		Common::Point(322, 428),
		Common::Point(292, 422),
		Common::Point(261, 426),
		Common::Point(371, 458),
		Common::Point(342, 459),
		Common::Point(310, 457),
		Common::Point(277, 457),
		Common::Point(245, 459),
	};
	/** Entry positions immediately before the four visible tunnel lanes. */
	static constexpr Common::Point kTunnelEntryPositions[4]{
		Common::Point(98, 424),
		Common::Point(178, 415),
		Common::Point(453, 421),
		Common::Point(533, 430),
	};
	/** Map visible tunnel-entry feature order to the corresponding gate-actor slots. */
	static constexpr int16 kGateActorSlotByEntry[4]{
		1,
		2,
		0,
		3,
	};
	/** Positions used by replay SCRS when a runner leaves a tunnel. */
	static constexpr Common::Point kScrsReplayPositions[4]{
		Common::Point(145, 455),
		Common::Point(210, 434),
		Common::Point(430, 434),
		Common::Point(476, 455),
	};
	/** Sixteen accepted-Snoid positions for each authored output lane. */
	static constexpr Common::Point kOutputLanePositions[4][16]{
		{
			// Gate 0
			Common::Point(277, 62),
			Common::Point(264, 63),
			Common::Point(247, 64),
			Common::Point(230, 66),
			Common::Point(274, 84),
			Common::Point(255, 86),
			Common::Point(236, 90),
			Common::Point(214, 92),
			Common::Point(273, 102),
			Common::Point(255, 104),
			Common::Point(235, 108),
			Common::Point(215, 112),
			Common::Point(258, 120),
			Common::Point(239, 128),
			Common::Point(220, 130),
			Common::Point(200, 133),
		},
		{
			// Gate 1
			Common::Point(403, 60),
			Common::Point(381, 61),
			Common::Point(362, 64),
			Common::Point(346, 69),
			Common::Point(412, 80),
			Common::Point(392, 84),
			Common::Point(372, 87),
			Common::Point(353, 93),
			Common::Point(414, 98),
			Common::Point(401, 103),
			Common::Point(382, 107),
			Common::Point(363, 110),
			Common::Point(415, 118),
			Common::Point(403, 121),
			Common::Point(387, 123),
			Common::Point(370, 127),
		},
		{
			// Gate 2
			Common::Point(288, 213),
			Common::Point(273, 219),
			Common::Point(257, 223),
			Common::Point(238, 226),
			Common::Point(222, 230),
			Common::Point(283, 235),
			Common::Point(268, 239),
			Common::Point(252, 245),
			Common::Point(237, 248),
			Common::Point(221, 252),
			Common::Point(287, 257),
			Common::Point(270, 260),
			Common::Point(253, 262),
			Common::Point(240, 265),
			Common::Point(220, 270),
			Common::Point(259, 280),
		},
		{
			// Gate 3
			Common::Point(414, 217),
			Common::Point(389, 223),
			Common::Point(373, 228),
			Common::Point(357, 233),
			Common::Point(415, 238),
			Common::Point(399, 247),
			Common::Point(382, 249),
			Common::Point(362, 255),
			Common::Point(419, 259),
			Common::Point(400, 263),
			Common::Point(381, 267),
			Common::Point(363, 276),
			Common::Point(420, 271),
			Common::Point(401, 278),
			Common::Point(387, 283),
			Common::Point(371, 268),
		},
	};
	/** The visible drop-zone order is A, C, D, B rather than the storage-table order A, B, C, D. */
	/** Convert visible drop-zone order to the gate/lane storage order. */
	static constexpr int16 kDropZoneToLane[4]{
		0,
		2,
		3,
		1,
	};
	/** Convert hover-data selector values to gate types. */
	static constexpr int16 kHoverDataToGateType[8]{
		0,
		1,
		0,
		2,
		2,
		3,
		1,
		3,
	};
	/** X origins used when spawning gate-side runners. */
	static constexpr int16 kSpawnOriginX[4]{
		141,
		198,
		426,
		479,
	};
	/** Radius of the authored tunnel drop-zone click test. */
	static const int16 kClickZoneRadius = 40;

	// =========================================================================
	// Feedback / Hint SCRB Pools
	// =========================================================================

	/**
	 * SCRB feedback and hint pools are selected by gate type and a non-repeating
	 * random index.
	 * Rejection pools are selected by gate type, while pools 2 and 4 also provide
	 * success-path feedback for specific gate layouts.
	 * The small and large hint pools are selected by the matched layout size.
	 * These collections are pools because each branch chooses an entry randomly
	 * without repeating it until that branch's selection cycle is exhausted.
	 */
	static constexpr int16 kRejectScrbPool0[10]{
		kResScrb4000_Reject0,
		kResScrb4001_Reject1,
		kResScrb4002_Reject2,
		kResScrb4003_Reject3,
		kResScrb4004_Reject4,
		kResScrb4005_Reject5,
		kResScrb4006_Reject6,
		kResScrb4007_Reject7,
		kResScrb4008_Reject8,
		kResScrb4009_Reject9,
	};
	static constexpr int16 kCorrectHintSmallScrbPool[11]{
		kResScrb4010_Hint0,
		kResScrb4011_Hint1,
		kResScrb4012_Hint2,
		kResScrb4013_Hint3,
		kResScrb4014_Hint4,
		kResScrb4015_Hint5,
		kResScrb4016_Hint6,
		kResScrb4017_Hint7,
		kResScrb4018_Hint8,
		kResScrb4019_Hint9,
		kResScrb4020_Hint10,
	};
	static constexpr int16 kRejectScrbPool1[8]{
		kResScrb4200_Reject0,
		kResScrb4201_Reject1,
		kResScrb4202_Reject2,
		kResScrb4203_Reject3,
		kResScrb4204_Reject4,
		kResScrb4205_Reject5,
		kResScrb4206_Reject6,
		kResScrb4207_Reject7,
	};
	static constexpr int16 kRejectScrbPool2[8]{
		kResScrb4208_Reject8,
		kResScrb4209_Reject9,
		kResScrb4210_Reject10,
		kResScrb4211_Reject11,
		kResScrb4212_Reject12,
		kResScrb4213_Reject13,
		kResScrb4214_Reject14,
		kResScrb4215_Reject15,
	};
	static constexpr int16 kRejectScrbPool3[7]{
		kResScrb4404_Reject4,
		kResScrb4403_Reject3,
		kResScrb4413_Reject,
		kResScrb4414_Reject,
		kResScrb4415_Reject,
		kResScrb4416_Reject,
		kResScrb4400_Reject0,
	};
	static constexpr int16 kRejectScrbPool4[6]{
		kResScrb4604_Reject4,
		kResScrb4605_Reject5,
		kResScrb4606_Reject6,
		kResScrb4607_Reject7,
		kResScrb4608_Reject8,
		kResScrb4609_Reject9,
	};
	static constexpr int16 kCorrectHintLargeScrbPool[6]{
		kResScrb4417_Hint,
		kResScrb4405_Hint,
		kResScrb4406_Hint,
		kResScrb4407_Hint,
		kResScrb4401_Reject1,
		kResScrb4418_Hint,
	};
	static constexpr int16 kRejectGateScrbPool1[4]{
		kResScrb4600_Reject0,
		kResScrb4601_Reject1,
		kResScrb4602_Reject2,
		kResScrb4603_Reject3,
	};

	// ========================================
	// Core State Variables
	// ========================================
	/** Number of Zoombinis that have completed an accepted tunnel entry. */
	int16 _enteredSnoidCount = 0;
	/** Difficulty-owned rejection budget remaining for the current visit. */
	int16 _remainingRejectChances = 0;
	/** Level-1 toggle selecting which mirrored entrance pair is blocked. */
	bool _level1BlockedPairToggle = false;
	/** Whether post-game celebration processing has begun. */
	bool _postGameStarted = false;
	/** Whether the main-path SCRB has completed for the current setup or post-game phase. */
	bool _postGameAnimCompleted = false;
	/**
	 * Whether the next queued Snoid approach or gate-actor step may start.
	 * Player drag and drop remains available while this queue-advance gate is disabled.
	 */
	bool _animationQueueAdvanceEnabled = true;
	/** Ordered setup states for gate, runner, and main-path materialization. */
	enum class SetupPhase : byte {
		/** No setup animation is pending. */
		kIdle00 = 0,
		/** The entry gate-actor sequence is pending. */
		kRunnersSelected01 = 1,
		/** The main-path SCRB must be materialized on the next frame. */
		kMainPathPending02 = 2,
		/** Initial page setup has completed. */
		kComplete03 = 3
	};
	/** Setup phase used while gates and runners are being materialized. */
	SetupPhase _setupPhase = SetupPhase::kIdle00;

	// ========================================
	// Rule System
	// ========================================

	struct TunnelGuardRule {
		/** Whether a trait match belongs to Crystal for guard A or Onyx for guard B. */
		bool traitMatchOnPrimarySide = false;
		/** Number of active trait conditions in this guard. */
		byte conditionCount = 0;
		/** Canonical trait categories tested by this guard. */
		ZmbTrait::TraitKind traitKinds[2] = {};
		/** Trait values tested by this guard. */
		byte traitValues[2] = {};
	};

	/**
	 * Values accepted by one trait category in a Tunnels rule.
	 * Level 2 and level 4 use only @ref firstValue as their one required value.
	 * Level 3 uses both fields as alternative values for the same category.
	 * Both fields are zero when the category does not participate in the rule.
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
	 * One candidate tunnel rule.
	 * Each field stores the accepted values for its named trait category.
	 * Level 2 populates exactly one field with one matching value.
	 * Level 3 populates exactly one field with two alternative matching values.
	 * Level 4 populates two fields with one matching value each.
	 */
	struct TraitRuleDescriptor {
		/** Accepted hair trait values. */
		TraitValueSet hairValues = {};
		/** Accepted eye trait values. */
		TraitValueSet eyesValues = {};
		/** Accepted nose trait values. */
		TraitValueSet noseValues = {};
		/** Accepted feet trait values. */
		TraitValueSet feetValues = {};

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

	/**
	 * One ordered pair of distinct trait categories used to generate Level 4 rules.
	 * The generator assigns its first value loop to @ref first and its second value loop to @ref second.
	 * Each pair therefore produces 25 descriptors: five values for @ref first times five values for @ref second.
	 * A generated descriptor accepts a Zoombini that matches either populated category; this pair does not represent a conjunction.
	 * The table order fixes the descriptor indices used by the existing candidate scoring and random tiebreaker.
	 */
	struct TraitKindPair {
		/** Category assigned the generator's first value. */
		ZmbTrait::TraitKind firstKind;
		/** Category assigned the generator's second value. */
		ZmbTrait::TraitKind secondKind;
	};
	/** Return the trait category in the original rule-selection order: feet, nose, eyes, hair. */
	static ZmbTrait::TraitKind traitKindFromRuleSlot(int16 slot);
	/** Match a level-2 rule descriptor against a Zoombini's traits. */
	static bool matchLevel2Descriptor(const ZmbTrait &trait, const TraitRuleDescriptor &descriptor);
	/** Match a level-3 rule descriptor against a Zoombini's traits. */
	static bool matchLevel3Descriptor(const ZmbTrait &trait, const TraitRuleDescriptor &descriptor);
	/** Match a level-4 rule descriptor against a Zoombini's traits. */
	static bool matchLevel4Descriptor(const ZmbTrait &trait, const TraitRuleDescriptor &descriptor);
	/** Decode a level-2 rule descriptor into a page-local guard. */
	static void descriptorToGuard(const TraitRuleDescriptor &descriptor, TunnelGuardRule &guard);
	/** Evaluate one decoded guard against a Zoombini's canonical trait record. */
	static bool evaluateGuard(const TunnelGuardRule &guard, const ZmbTrait &trait);
	/** Format one decoded guard for the built-in debug answer. */
	static Common::String formatGuardTraitDescription(const TunnelGuardRule &guard);

	/**
	 * Number of active guard axis in the generated rule set.
	 * Set to 1 in L1, and 2 in L2 ~ L4.
	 */
	int16 _guardAxisCount = 0;
	/** Generated guard data for the two sides of the tunnel layout. */
	TunnelGuardRule _guardRules[2];

	// ========================================
	// Per-Gate State
	// ========================================

	/** Consecutive correct-entry count for visible zones 1-4; element zero is unused. */
	int16 _zoneCorrectStreak[5] = {};
	/** Number of accepted Zoombinis occupying each authored output lane. */
	int16 _laneOccupancy[4] = {};

	/** Snoid runner IDs stored in each output lane's sixteen authored positions. */
	uint16 _laneSnoidIds[4][16] = {};

	/** Crystal rejections since the last accepted submission. */
	int16 _crystalRejectCountSinceSuccess = 0;

	/** Igno rejections since the last accepted submission. */
	int16 _ignoRejectCountSinceSuccess = 0;

	// ========================================
	// Animation Queue System
	// ========================================

	struct AnimQueueEntry {
		/** Snoid runner ID, or zero for an actor-only sequence. */
		uint16 snoidId = 0;
		/** Whether this entry is a rejected tunnel submission. */
		bool isRejection = false;
		/** Current step of the entry's primary/secondary gate-actor sequence. */
		int16 actorSequenceStep = 0;
		/** SCRS resource used for the approach to the selected tunnel. */
		int16 approachScrsId = 0;
		/** SCRS resource used for a rejection return. */
		int16 returnScrsId = 0;
		/** Primary gate-actor slot, or -1 when absent. */
		int16 primaryActorSlot = -1;
		/** Initial SCRB for the primary gate actor. */
		int16 primaryActorScrbId = 0;
		/** Follow-up SCRB for the primary gate actor. */
		int16 primaryFollowupScrbId = 0;
		/** Secondary gate-actor slot, or -1 when absent. */
		int16 secondaryActorSlot = -1;
		/** Initial SCRB for the secondary gate actor. */
		int16 secondaryActorScrbId = 0;
		/** Follow-up SCRB for the secondary gate actor. */
		int16 secondaryFollowupScrbId = 0;
		/** One-based visible drop zone associated with this entry. */
		int16 dropZone = 0;
	};

	/** Pending tunnel-entry and rejection animations in submission order. */
	AnimQueueEntry _animQueue[5];
	/** Number of valid entries currently stored in @ref _animQueue. */
	int16 _animQueueCount = 0;
	/** Whether the currently dragged Snoid was removed from the animation queue. */
	bool _draggedSnoidWasQueued = false;

	/** Gate-actor slot deferred until tracked gate and ambient sounds are idle. */
	int16 _deferredActorSlot = -1;
	/** SCRB resource to load on @ref _deferredActorSlot. */
	int16 _deferredActorScrbId = 0;
	/** Whether completion of the deferred actor owns the animation queue head. */
	bool _deferredActorPopsQueue = false;

	enum GateActorCompletionAction : byte {
		kGateActorCompletionNone00 = 0,
		kGateActorCompletionAdvanceQueue01 = 1,
		kGateActorCompletionDeferred02 = 2,
		kGateActorCompletionDeferredAndPop03 = 3,
		kGateActorCompletionPrimary04 = 4
	};

	/** Completion action associated with each gate actor. */
	GateActorCompletionAction _gateActorCompletionActions[4] = {};

	/** Gate-actor sequence selected by @ref queueGateActorSequence(). */
	enum class GateActorSequenceMode : byte {
		kIdle00 = 0,
		kEntry01 = 1,
		kPostGame02 = 2,
		kAdvance03 = 3
	};

	/** Append a submission to the tunnel animation queue. */
	void appendAnimQueueEntry(const AnimQueueEntry &entry);
	/** Remove the oldest submission from the tunnel animation queue. */
	void popAnimQueueEntry();
	/** Remove the queue entry belonging to @p runnerId. */
	int16 removeAnimQueueEntry(uint16 runnerId);
	/** Advance the front queue entry by one animation step. */
	void advanceAnimStep();
	/** Select and queue one gate-actor sequence for the requested presentation mode. */
	void queueGateActorSequence(GateActorSequenceMode mode);
	/** Cache active-pack seat occupancy before gate placement begins. */
	void collectPackSeatOccupancy();
	/** Select an unused active-pack seat for a returning runner. */
	int16 selectUnusedPackSeat();
	/** Place a runner in a gate slot, pushing existing entries when needed. */
	int16 assignSlotWithPush(int16 returnFromLeftSide);
	/** Spawn queued active-pack Zoombinis at their tunnel-entry positions. */
	int16 spawnPendingZoombinis();
	/** Play the ambient sound associated with the current tunnel state. */
	void playAmbientSound();
	/** Clear the gate feature render-suppression flag after an animation. */
	void clearGateRenderFlag();
	/** Map a gate SCRB ID to its actor image resource. */
	static int16 resolveGateActorImageResourceId(int16 scrbId);
	/** Start a gate actor and remember its queue completion action. */
	void playGateActor(int16 actorSlot, int16 scrbId, GateActorCompletionAction completion);
	/** Refresh sound playback for the currently active SCRB animation. */
	void refreshActiveScrbSound();

	/** Process a Snoid SCRS event emitted by tunnel movement. */
	void processSnoidAnimEvent(ZmbSnoid *snoid, int16 eventCode);
	/** Process a gate SCRB event emitted by a gate or feedback actor. */
	void processGateAnimEvent(ZmbFeature *feature, int16 eventCode);
	/** Play the selected low-chance rockfall SFX and one SCRB 7001-7004 visual variant. */
	void playRockfallWarningFeedback();
	/** Find an idle active-pack Snoid by serialized ID. */
	ZmbSnoid *findIdlePackSnoid(uint16 snoidId);

	// ========================================
	// Feature Runners
	// ========================================

	/** SCRB 9000 feature used as the first accepted-Snoid link anchor. */
	ZmbFeature *_laneLinkAnchorFeature = nullptr;
	/** Gate-entry feature runners for the four visible lanes. */
	ZmbFeature *_tunnelEntryFeatures[4] = {};
	/** SCRB 7001-7004 falling-rock overlay used for low-chance rejection warnings. */
	ZmbFeature *_rockfallWarningFeature = nullptr;
	/** Gate-actor features in Crystal, Onyx, Ferrous, and Igno slot order. */
	ZmbFeature *_gateActorFeatures[4] = {};
	/** Main route feature used for gate and path animation. */
	ZmbFeature *_mainPathFeature = nullptr;
	/** Shared current debug SCRB resource ID across the four gate actors. */
	int16 _builtinDebugScrbId = 0;

	// ========================================
	// Ambient/Idle Animation
	// ========================================
	// Timer storage is generic, but eligibility, pool state, and completion are
	// Tunnels-owned and must not be merged with another page's idle state machine.

	/** Required shared inactivity before another idle-actor sequence can be queued. */
	uint32 _idleActorDelay = 0;
	/** Number of celebration walkers requested by the current result. */
	int16 _celebrationTarget = 0;
	/** Number of celebration walkers already played. */
	int16 _celebrationsPlayed = 0;
	/** Frame at which celebration scheduling last ran. */
	uint32 _lastCelebrationFrame = 0;
	/** Delay between celebration walker spawns. */
	uint32 _celebrationInterval = 0;
	/** SND 4700-4703 selected when a rejection leaves four through one chances, or zero. */
	int16 _rockfallWarningSoundId = 0;
	/** Whether the selected rockfall warning SFX is currently blocking queue advancement. */
	bool _rockfallWarningSoundPlaying = false;
	/** Final accepted-entry narrator resource waiting for the audio boundary. */
	int16 _completionNarratorSoundId = 0;
	/** Exact active page SND selected from the gate or ambient SCRB ranges. */
	ZmbResource _activeGateOrAmbientSoundResource;
	/** Mixer handle for the active rockfall warning SFX. */
	Audio::SoundHandle _rockfallWarningSoundHandle;
	/** Whether @ref _rockfallWarningSoundHandle contains a valid handle. */
	bool _rockfallWarningSoundHandleValid = false;

	/** Non-repeating pool state for Crystal rejection actors. */
	uint32 _crystalRejectPoolState = 0;
	/** Non-repeating pool state for Crystal hint actors. */
	uint32 _crystalHintPoolState = 0;
	/** Non-repeating pool state for Ferrous rejection actors. */
	uint32 _ferrousRejectPoolState = 0;
	/** Non-repeating pool state for Ferrous accepted-entry actors. */
	uint32 _ferrousCorrectPoolState = 0;
	/** Non-repeating pool state for Igno rejection actors. */
	uint32 _ignoRejectPoolState = 0;
	/** Non-repeating pool state for Igno hint actors. */
	uint32 _ignoHintPoolState = 0;
	/** Non-repeating pool state for Onyx rejection actors. */
	uint32 _onyxRejectPoolState = 0;
	/** Non-repeating pool state for Onyx accepted-entry actors. */
	uint32 _onyxCorrectPoolState = 0;
	/** Non-repeating pool state for idle gate actors. */
	uint32 _idleActorPoolState = 0;
	/** Non-repeating pool state for page-entry gate actors. */
	uint32 _entryActorPoolState = 0;
	/** Non-repeating pool state for post-game gate actors. */
	uint32 _postGameActorPoolState = 0;
	/** Non-repeating pool state for advance actors while every pack Snoid is idle. */
	uint32 _allIdleAdvanceActorPoolState = 0;
	/** Non-repeating pool state for advance actors while the pack is partially distributed. */
	uint32 _partialAdvanceActorPoolState = 0;
	/** Non-repeating pool state for advance actors after Go becomes ready. */
	uint32 _goReadyAdvanceActorPoolState = 0;
	/** Non-repeating pool state for celebration Snoid selection. */
	uint32 _celebrationSnoidPoolState = 0;

	/** Snoid runner IDs assigned to the authored input-side seat order. */
	uint16 _packSeatSnoidIds[16] = {};
};

} // End of namespace Mohawk

#endif
