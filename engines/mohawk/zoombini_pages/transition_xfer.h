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

#ifndef MOHAWK_ZOOMBINI_PAGES_TRANSITION_XFER_H
#define MOHAWK_ZOOMBINI_PAGES_TRANSITION_XFER_H

#include "mohawk/zoombini_pages/transition_base.h"
#include "mohawk/zoombini_resource.h"
#include "mohawk/zoombini_state.h"

#include "common/array.h"
#include "graphics/surface.h"

namespace Mohawk {

/**
 * Route transfer transition page.
 *
 * Xfer selects one of the six route-specific scenes, loads the associated
 * background and environment features, and walks the active pack through the
 * scene. The source page and destination page are derived from the saved
 * journey state; completion waits for the authored animation/audio hand-off
 * before closing and constructing the next page.
 */
class ZoombiniTransitionXfer : public ZoombiniTransition {
public:
	/** Create the transfer page. */
	ZoombiniTransitionXfer(MohawkEngine_Zoombini *vm);
	/** Release transfer features, sounds, and route-path surfaces. */
	~ZoombiniTransitionXfer() override;

	/** Determine the route scene and initialize transfer state. */
	void open() override;
	/** Select the route-specific transfer background. */
	void setBackgroundBitmap() override;
	/** Load the environment, route-path, and walking Zoombini features. */
	void loadFeatures() override;
	/** Return the original route-transfer script-sound range priority. */
	const ZoombiniPage::ScriptSoundPriorityRanges &getScriptSoundPriorityRanges() const override;
	/** Render first, then advance transfer scheduling and auto-close checks. */
	void onAnimFrame() override;
	/** Finish the transfer and release route-path resources. */
	void close() override;
	/** Process route-scene animation callbacks. */
	void onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) override;
	/** Handle pointer skip/advance input. */
	ZmbEventHandleResult onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Handle keyboard skip/advance input. */
	ZmbEventHandleResult onKeyDown(const Common::KeyState &kbd, bool kbdRepeat) override;

protected:
	/** Render the Town-count overlay during the destination-town transfer. */
	ZmbRenderResult xfer5TownCount_render(ZmbFeature *feature);
	/** Apply the Town-count overlay after its feature has been composited. */
	void xfer5TownCount_onPostRender(ZmbFeature *feature);
	/** Rebuild the Town-count feature after its displayed population changes. */
	void reloadXfer5TownCount();
	/** Activate the one-shot environmental features for the Town transfer. */
	void activateXfer5EnvironmentalFeatures();
	/** Materialize the walkers that appear between the route and Town. */
	void activateMidRouteWalkers();

	// Constants
	/** Page-local resources grouped by transfer route and resource family. */
	enum PageResourceId : int16 {
		// Background resources, grouped by XferSceneId.
		kResBackground1000_Route1_BigBadHungry = 1000,
		kResBackground2000_Route2_WhosBayou = 2000,
		kResBackground3000_Route3_DeepDarkForest = 3000,
		kResBackground4000_Route4_MountainOfDespair = 4000,
		kResBackground5000_Route0_FromIsle = 5000,
		kResBackground6000_Route5_ToTown = 6000,

		// Page shape archive resources, grouped by XferSceneId.
		kResShapes1100_Route1_BigBadHungry = 1100,
		kResShapes2100_Route2_WhosBayou = 2100,
		kResShapes3100_Route3_DeepDarkForest = 3100,
		kResShapes4100_Route4_MountainOfDespair = 4100,
		kResShapes5100_Route0_FromIsle = 5100,
		kResShapes6100_Route5_ToTown = 6100,

		// SCRB resources, grouped by XferSceneId.
		kResScrb1100_Route1_MainOverlay = 1100,
		kResScrb1101_Route1_ForegroundEdge1 = 1101,
		kResScrb1102_Route1_ForegroundEdge2 = 1102,
		kResScrb1200_Route1_RoutePath = 1200,
		kResScrb2100_Route2_MainOverlay = 2100,
		kResScrb2101_Route2_ForegroundEdge1 = 2101,
		kResScrb2102_Route2_ForegroundEdge2 = 2102,
		kResScrb2200_Route2_RoutePath = 2200,
		kResScrb3100_Route3_MainOverlay = 3100,
		kResScrb3101_Route3_ForegroundEdge1 = 3101,
		kResScrb3102_Route3_ForegroundEdge2 = 3102,
		kResScrb3200_Route3_RoutePath = 3200,
		kResScrb4100_Route4_MainOverlay = 4100,
		kResScrb4101_Route4_ForegroundEdge1 = 4101,
		kResScrb4102_Route4_ForegroundEdge2 = 4102,
		kResScrb4200_Route4_RoutePath = 4200,
		kResScrb5100_Route0_DockRock = 5100,
		kResScrb5101_Route0_DockForeground = 5101,
		kResScrb5102_Route0_EventDirtFallLeft = 5102,
		kResScrb5103_Route0_EventDirtFallRight = 5103,
		kResScrb5104_Route0_RandomBirdFlight1 = 5104,
		kResScrb5105_Route0_RandomBirdFlight2 = 5105,
		kResScrb5106_Route0_RandomBirdFlight3 = 5106,
		kResScrb5107_Route0_RandomBirdFlight4 = 5107,
		kResScrb5108_Route0_DirtCollapse = 5108,
		kResScrb6100_Route5_Foreground1 = 6100,
		kResScrb6101_Route5_Foreground2 = 6101,
		kResScrb6102_Route5_Foreground3 = 6102,
		kResScrb6103_Route5_Foreground4 = 6103,
		kResScrb6104_Route5_MidBackground = 6104,
		kResScrb6105_Route5_TownCount = 6105,
		kResScrb6106_Route5_BirdFlyby = 6106,
		kResScrb6107_Route5_BeeFlight = 6107,
		kResScrb6108_Route5_FarBackgroundAnimation = 6108,

		// SCRS resources, grouped by XferSceneId.
		kResScrs5199_Route0_WalkBase = 5199,
		kResScrs6199_Route5_WalkBase = 6199,
	};

	enum class XferSceneId : uint16 {
		kScene0_FromIsle = 0,
		kScene1_BigBadHungry = 1,
		kScene2_WhosBayou = 2,
		kScene3_DeepDarkForest = 3,
		kScene4_MountainOfDespair = 4,
		kScene5_ToTown = 5,
	};

	/**
	 * Immutable route-title rectangles are instance members because Common::Rect
	 * requires runtime construction and ScummVM prohibits global C++ constructors.
	 */
	const Common::Rect _routeTextRects[4] = {
		Common::Rect(43, 54, 226, 107),
		Common::Rect(371, 33, 613, 65),
		Common::Rect(127, 29, 299, 81),
		Common::Rect(135, 29, 323, 82),
	};

	// Route determination
	/** Derive the transfer route and destination page from saved journey state. */
	void computeXferRoute();

	// Sound selection
	/** Return the numeric ID of the route-specific system PCM resource with optional Narrator speech. */
	int16 selectXferSoundId() const;

	// Skip-to-completion logic
	/** Complete the transfer scene while preserving its destination state. */
	void skipToCompletion();

	// Xfer state set by @ref ZoombiniTransitionXfer::computeXferRoute().
	/** Selected XFER scene view, using the XFER scene index values. */
	XferSceneId _xferView = XferSceneId::kScene0_FromIsle;
	/** Destination page constructed after the transfer closes. */
	ZoombiniPageType _nextPageType = ZoombiniPageType::kBridge;
	/** Background resource selected for the current route view. */
	int16 _xferBackgroundResId = kResBackground5000_Route0_FromIsle;
	/** Shape archive resource selected for the current route view. */
	int16 _xferShapesId = kResShapes5100_Route0_FromIsle;
	/** Number of main environment SCRBs to load */
	uint16 _xferScrbCount = 9;

	// Completion tracking for auto-close.
	/** Absolute frame counter for timer-based auto-close (all views: +300 frames) */
	uint32 _closureFrame = 0;
	/** Exact system SND being played for the close-wait-for-sound check. */
	ZmbResource _xferSoundRes;
	/** Use small-scale snoid shapes (XFER_0 only; drives resource 3200 + small tables) */
	bool _useSmallSnoids = false;

	// SCRS periodic trigger state (XFER_0 and XFER_5 only)
	/** Absolute frame counter for next SCRS trigger event */
	uint32 _scrsNextTriggerFrame = 0;
	/** Index of next snoid to trigger (0..snoidCount-1) */
	uint16 _scrsTriggerIdx = 0;
	/** True once the first snoid trigger has fired (XFER_0: enables env SCRB branch) */
	bool _scrsTriggerPhase1 = false;
	/** Total snoids loaded for this XFER (for trigger indexing) */
	uint16 _xferSnoidCount = 0;
	/** SCRS resource base for feet-trait offset (XFER_0: 5199, XFER_5: 6199) */
	int16 _scrsResIdBase = kResScrs5199_Route0_WalkBase;
	/** XFER_5 sign count, seeded from stored town count and incremented by SCRS event 50. */
	int16 _xfer5DisplayedTownCount = 0;
	/** Render and bake SCRB 6105 only when its displayed population changes. */
	bool _xfer5TownCountNeedsBake = false;
	/** Deferred SCRB 6105 reload requested by a population event. */
	bool _xfer5TownCountRefreshPending = false;
	/** XFER_5 SCRB 6106 bird and SCRB 6107 bee one-shot runners. */
	ZmbFeature *_xfer5ForegroundFeatures[2] = {nullptr, nullptr};
	/** Defer the XFER_5 environmental one-shots until their initial static render pass completes. */
	bool _xfer5ForegroundActivationPending = false;

	// SCRB animation callback state (XFER_0 and XFER_5 only)
	// Handle SCRS event codes during playback.

	/**
	 * Snoid SCRS completion counter.
	 * Incremented on event code 26 (animation complete). When >4 (5 snoids done),
	 * the final env SCRB is activated to trigger page transition.
	 * Set to -1 to disable further counting after final activation.
	 */
	int16 _completionCounter = 0;

	/**
	 * Pending body arrangement override.
	 * Set by event codes 240-243 (value = eventCode - 239, so 1-4).
	 * Applied on the next event code 0 (facing toggle) as arrangement (value - 1).
	 * 0 = no pending override.
	 */
	uint16 _bodyArrangementOverride = 0;

	/**
	 * SCRB IDs of the 4 env animation runners (XFER_0 only).
	 * Loaded SCRBs 5104-5107, in their original four-entry runner table order.
	 * @ref ZoombiniTransitionXfer::onAnimFrame() activates them randomly when the SCRS timer fires.
	 * Each runner can be selected again after its transient PLAY_ONCE cycle becomes inactive.
	 */
	int16 _envScrbIds[4] = {0, 0, 0, 0};

	/**
	 * SCRB ID of the one-shot env animation runner (XFER_0 only).
	 * Loaded SCRB 5108.
	 * Activated once when the random branch selects it;
	 * @ref ZoombiniTransitionXfer::_envOneShotAvailable prevents reuse.
	 */
	int16 _envOneShotScrbId = 0;
	/** Whether the one-shot environment animation can still be activated. */
	bool _envOneShotAvailable = false;

	/**
	 * Z-link target SCRB ID.
	 * XFER_0: 5100 (dock rock overlay), XFER_5: 6104 (mid-background), else 0.
	 *
	 * This page disables positional z-sorting through @ref ZoombiniPage::setRunnerZSortEnabled().
	 * Render order therefore follows registration order, and explicit re-links persist.
	 * Event 0 cycle 2 for XFER_0 links the walking Snoid after this runner, in front of the rock.
	 * Event 26 links it before this runner again, behind the rock.
	 */
	int16 _linkTargetScrbId = 0;

	/**
	 * Final env SCRB ID.
	 * Activated after @ref ZoombiniTransitionXfer::_completionCounter exceeds four.
	 */
	int16 _finalEnvScrbId = 0;

	/**
	 * One-shot trigger flags for events 10-11 (XFER_0 only).
	 * Initialized to true, cleared after activation.
	 * [0] = SCRB 5102 dirt fall (event 10), [1] = SCRB 5103 dirt fall (event 11).
	 */
	bool _envEventTriggerFlags[2] = {false, false};

	/**
	 * SCRB ID for event 50 activation (XFER_5 only: 6105).
	 * Runner for SCRB 6105 (town count display).
	 */
	int16 _xfer5EventScrbId = 0;

	// -----------------------------------------------------------------------
	// Route Path Flood-Fill State (XFER_1-4 only)
	// Post-render flood-fill animation.
	// -----------------------------------------------------------------------

	/**
	 * Route Path animation counter.
	 * Increments by 7 per frame, wraps at 1000. Controls flood-fill expansion rate.
	 */
	uint32 _routePathCounter = 0;

	/**
	 * Frame-interval gate for route path flood-fill.
	 * Each interval marks the route path dirty.
	 * Post-render then performs one flood-fill step.
	 * The gate advances @ref ZoombiniTransitionXfer::_routePathNextFrame by the feature interval.
	 */
	uint32 _routePathNextFrame = 0;

	/**
	 * Route band position.
	 * 1-4 based on which crossing within the current route.
	 * Used for shape selection and seed index.
	 */
	uint16 _routePathBand = 1;

	/**
	 * Route color level.
	 * 1-4 based on the puzzle difficulty / route progression level.
	 * Determines the flood-fill color string ("10/.", "3210", "5432", "7654").
	 * Separate from @ref ZoombiniTransitionXfer::_routePathLevel because band position and color level differ.
	 * The first traversal always uses level 1 colors for all bands.
	 */
	uint16 _routePathColorLevel = 1;

	/** Pointer to the route path overlay feature for callback. */
	ZmbFeature *_routePathFeature = nullptr;

	/**
	 * Mid-route walkers in registration order.
	 * The first static composite renders them idle before the array is traversed in reverse.
	 */
	ZmbSnoid *_midRouteWalkers[16] = {};
	/** Number of valid entries in @ref _midRouteWalkers. */
	uint16 _midRouteWalkerCount = 0;
	/** Whether mid-route walkers still need activation after the first composite. */
	bool _midRouteWalkerActivationPending = false;
	/** Whether the initial composite containing inactive walkers has rendered. */
	bool _midRouteInitialCompositeRendered = false;

	/**
	 * Working pixel buffer for flood-fill (points into shape surface pixels).
	 * Set on first render call; dimensions match the path overlay shape.
	 */
	/** Pixel base of the route-path shape used by the flood-fill callback. */
	byte *_routePathPixels = nullptr;
	/** Width of the active route-path shape in pixels. */
	uint16 _routePathWidth = 0;
	/** Height of the active route-path shape in pixels. */
	uint16 _routePathHeight = 0;
	/** Bytes between rows in the active route-path shape. */
	uint32 _routePathPitch = 0;

	/** BFS queue for flood-fill expansion. */
	static constexpr int kRoutePathQueueSize = 24;
	bool _routePathQueueActive[kRoutePathQueueSize] = {};
	int16 _routePathQueueX[kRoutePathQueueSize] = {};
	int16 _routePathQueueY[kRoutePathQueueSize] = {};

	/** Flood-fill progress tracking. */
	uint32 _routePathTotalPixels = 0;
	/** Number of route-path pixels that remain to be flood-filled. */
	uint32 _routePathRemainingPixels = 0;

	/**
	 * Color values for flood-fill.
	 * @ref ZoombiniTransitionXfer::_routePathMark1 is the first intermediate color set during initialization.
	 * @ref ZoombiniTransitionXfer::_routePathMark2 is the second intermediate color set during initialization.
	 * @ref ZoombiniTransitionXfer::_routePathReplace1 is the first final color set during expansion.
	 * @ref ZoombiniTransitionXfer::_routePathReplace2 is the second final color set during expansion.
	 */
	byte _routePathMark1 = 0;
	/** Second intermediate palette value used by the route-path flood fill. */
	byte _routePathMark2 = 0;
	/** First replacement palette value written during flood-fill expansion. */
	byte _routePathReplace1 = 0;
	/** Second replacement palette value written during flood-fill expansion. */
	byte _routePathReplace2 = 0;

	// -----------------------------------------------------------------------
	// Route View Slot State (XFER_1-4 only)
	// Remaps main SCRB shapes based on puzzle completion to show completed bands in foreground colors.
	// -----------------------------------------------------------------------

	/**
	 * Per-puzzle completion level array.
	 * Built from game-state flags by @ref ZoombiniTransitionXfer::buildPuzzleCompletionArray().
	 * Index = ZmbSrcPageKind enum value (0-16).
	 * Values: -1 = current band (being animated), 0 = not completed, 1-4 = highest level.
	 */
	int8 _puzzleCompletionArr[17] = {};

	/**
	 * Route progress level for shape variant selection.
	 * @ref ZoombiniTransitionXfer::routeView_updateSlots() uses this level.
	 * It selects the shape variant for the current and cascading bands.
	 * 1 = uninitialized, 0-4 = level.
	 */
	int16 _routeProgressLevel = -1;

	/**
	 * Global route slot index for the current destination.
	 * Maps to a position in @ref ZoombiniTransitionXfer::kRouteViewSlotTable.
	 */
	int16 _routeSlotIndex = 0;

	// Route path methods
	/** Determine the route-path band represented by the current transfer. */
	void computeRoutePathBand();
	/** Determine the palette level used to animate the current route path. */
	void computeRoutePathColorLevel();
	/** Build the per-puzzle completion levels used by the route view. */
	void buildPuzzleCompletionArray();
	/** Read one puzzle's highest completed difficulty from saved state. */
	static uint16 readPuzzleLevelFlag(const ZmbStateFile &state, ZmbSrcPageKind siPage);
	/** Remap route-view slots to completed and active puzzle colors. */
	void routeView_updateSlots(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** Select the route-path band and prepare its flood-fill hotspots. */
	void routePath_selectBand(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** Render the current route-path flood-fill result. */
	ZmbRenderResult routePath_onPostRender(ZmbFeature *feature);
	/** Initialize the flood-fill grid and its intermediate palette markers. */
	void routePath_initGrid(int16 seedX, int16 seedY, byte mark1, byte mark2, byte replace1, byte replace2);
	/** Expand the route-path flood fill by the requested animation budget. */
	void routePath_expandFloodFill(uint32 counter);
	/** Reserve one flood-fill queue slot for a reachable pixel. */
	void routePath_reserveSlot(int16 y, int16 x, byte *pixel);

	/**
	 * Helper: activate a deferred env SCRB feature by ID.
	 *
	 * @param persistAfterPlay Keep drawing the frozen last frame after playback when true.
	 * When false, hide the feature again after playback and ignore re-triggers while it is still animating.
	 */
	void activateEnvScrb(int16 scrbId, bool persistAfterPlay = false);
};

} // End of namespace Mohawk

#endif
