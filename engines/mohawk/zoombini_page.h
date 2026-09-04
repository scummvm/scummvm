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

#ifndef MOHAWK_ZOOMBINI_PAGE_H
#define MOHAWK_ZOOMBINI_PAGE_H

#include "mohawk/zoombini_resource.h"

#include "common/array.h"
#include "common/events.h"
#include "common/hashmap.h"
#include "common/list.h"
#include "common/stablemap.h"
#include "common/stack.h"
#include "common/str.h"

#include "mohawk/resource.h"
#include "mohawk/zoombini_scripts.h"
#include "mohawk/zoombini_text.h"

namespace Mohawk {

struct ZmbStateActivePack;

/**
 * Ordered feature container with insertion-order iteration and linear lookup by uint16 ID.
 *
 * This container backs the page's separate standard-feature and Snoid registries.
 * The page combines active entries into @ref ZoombiniPage::_runnerOrder when it
 * needs one global lifecycle or render order.
 * Each runner has a unique registration ID and a separate SCRB resource ID,
 * so multiple runners may share one resource ID.
 * Collection sizes are small (<100), making a cache-friendly array with linear lookup appropriate.
 *
 * Duplicate keys are appended, and find returns the first matching entry.
 */
template<class T>
class ZmbFeatureList {
public:
	using iterator = typename Common::Array<T *>::iterator;
	using const_iterator = typename Common::Array<T *>::const_iterator;

	iterator begin() { return _items.begin(); }
	iterator end() { return _items.end(); }
	const_iterator begin() const { return _items.begin(); }
	const_iterator end() const { return _items.end(); }

	uint size() const { return _items.size(); }
	bool empty() const { return _items.empty(); }

	/** Insert a feature at the tail (= registered last = drawn last in
	 *  LOOP_ANIM bucket). Always succeeds; duplicate keys are allowed. */
	void insert(uint16 id, T *feature) {
		(void)id;

		_items.push_back(feature);
	}

	/** Lookup by ID (linear scan). Returns nullptr when not found. */
	T *find(uint16 id) const {
		for (const auto *item : _items) {
			if (item->getId() == id)
				return const_cast<T *>(item);
		}
		return nullptr;
	}

	/** Erase the first feature matching the given ID.
	 *  Returns the erased pointer (caller responsible for delete), or nullptr if not found.
	 *
	 *  @note Since multiple features can share the same ID, this method is only safe when
	 *  the specific instance does not matter or IDs are guaranteed to be unique. */
	T *erase(uint16 id) {
		for (uint i = 0; i < _items.size(); i++) {
			if (_items[i]->getId() == id) {
				T *ptr = _items[i];
				_items.remove_at(i);
				return ptr;
			}
		}
		return nullptr;
	}

	/** Erase a specific feature instance by pointer comparison.
	 *  Does not return the pointer as it's already provided.
	 *
	 *  This must be used instead of @ref erase() when removing a known instance from a list
	 *  that may contain duplicate IDs. Using @ref erase() in such cases can remove the wrong
	 *  instance, leaving the intended object in the list as a dangling pointer after deletion,
	 *  which leads to Use-After-Free crashes during render/event loops. */
	void eraseByPtr(T *feature, uint16 id) {
		(void)id;

		for (uint i = 0; i < _items.size(); i++) {
			if (_items[i] == feature) {
				_items.remove_at(i);
				return;
			}
		}
	}

	void clear() {
		_items.clear();
	}

private:
	Common::Array<T *> _items;
};

/**
 * Identifies the source collection of a @ref ZmbLoadedFeatureInfo entry.
 *
 * The debug feature list combines page SCRB features, SUB features, and
 * Snoid entries into one list while preserving the collection that supplied
 * each entry.
 */
enum class ZmbLoadedFeatureListKind : uint8 {
	/** An entry from the page SCRB feature list. */
	kScrb,
	/** An entry from the page SUB feature list. */
	kSub,
	/** An entry from the page Snoid map. */
	kSnoid
};

struct ZmbLoadedFeatureInfo {
	/** Construct an empty feature-list entry. */
	ZmbLoadedFeatureInfo() = default;
	/** Construct a feature-list entry with its owning registry kind. */
	ZmbLoadedFeatureInfo(const ZmbFeature *feature, ZmbLoadedFeatureListKind listKind) : _feature(feature), _listKind(listKind) {}

	/** Feature pointer exposed by the debug feature listing. */
	const ZmbFeature *_feature = nullptr;
	/** Registry from which @ref _feature was collected. */
	ZmbLoadedFeatureListKind _listKind = ZmbLoadedFeatureListKind::kScrb;
};

class MohawkEngine_Zoombini;
class MohawkSurface;

/**
 * Base owner for one active Zoombini page or modal page.
 *
 * The engine creates a concrete page and invokes @ref open(),
 * @ref setBackgroundMusic(), @ref setBackgroundBitmap(), @ref initStates(),
 * @ref loadFeatures(), and @ref initSfxGroupFlags(). It then performs any requested
 * fidget reset, invokes @ref initHelpPrompt(), and finishes synchronous setup through
 * @ref activatePage() before the first fade-in. The page owns its page-local archives,
 * feature runners, timing groups, input handlers, and transient state until @ref close() releases them.
 *
 * Standard SCRB runners, independently attached sub-features, and Snoid runners
 * use separate registries because their ownership and input rules differ.
 * @ref _runnerOrder is the flat active-runner order used by animation, event
 * dispatch, and rendering. Automatic Z sorting can rebuild that order; manual
 * links can preserve it explicitly. A parent feature owns its sub-features even
 * while the page temporarily registers those sub-features for independent
 * rendering.
 *
 * On an animation tick, @ref onAnimFrame() runs the pre-render pass for active
 * runners, dispatches frame sounds and events, prepares the render order, and
 * runs the post-render shape callbacks. @ref checkCloseFeatures() then detaches
 * runners that completed or were scheduled for removal.
 */
class ZoombiniPage {
public:
	/**
	 * Animation event codes passed to @ref ZoombiniPage::onFeatureAnimEvent().
	 *
	 * Subtract one from the raw byte in each 0xFFxx SCRB/SCRS frame terminator before dispatch.
	 * The result becomes the eventCode parameter.
	 *
	 * Framework-level codes (handled by the page before/during dispatch):
	 *   -1        End-of-animation (PLAY_ONCE / CHAIN_SCRIPT completion).
	 *             Dispatched to @ref ZoombiniPage::onFeatureAnimEvent() at the completion boundary.
	 *   200-239   Voice SFX intercepted by the framework.
	 *             These codes do not reach @ref ZoombiniPage::onFeatureAnimEvent().
	 *             They map through the voice-group table for Snoid voice samples.
	 *
	 * Shared conventions (used identically across multiple puzzle pages):
	 *   240-243   Pending snoid body arrangement override.
	 *             Arrangement index = eventCode - 239 (range 1-4).
	 *             Applied on the next event 0 toggle cycle.
	 *             Used by: xfer, tunnels.
	 *   250-253   Direct snoid body arrangement set.
	 *             Arrangement index = eventCode - 250 (range 0-3).
	 *             Used by: xfer, tunnels, smoke.
	 *
	 * All other values from 0 through 199 are page-specific.
	 * Each puzzle defines their meaning in its @ref ZoombiniPage::onFeatureAnimEvent() override.
	 */
	enum AnimateEvent : int16 {
		/** End-of-animation cycle. */
		kAnimEventM1_End = -1,
		/** First voice SFX code (intercepted). */
		kAnimEvent200_VoiceFirst = 200,
		/** Last voice SFX code (intercepted). */
		kAnimEvent239_VoiceLast = 239,
		/** First pending body arrangement code. */
		kAnimEvent240_BodyArrangePendFirst = 240,
		/** Last pending body arrangement code. */
		kAnimEvent243_BodyArrangePendLast = 243,
		/** First direct body arrangement code. */
		kAnimEvent250_BodyArrangeDirectFirst = 250,
		/** Last direct body arrangement code. */
		kAnimEvent253_BodyArrangeDirectLast = 253,
	};

	/**
	 * Numeric SND range used for script-sound priority lookup.
	 *
	 * @ref ZoombiniPage::lookupScriptSoundPriority() uses the first matching range
	 * to choose the priority of a SCRB or SCRS sound ID.
	 * Script-sound archive selection is independent: IDs below 1000 and IDs
	 * from 20000 use the system archive; other IDs use the page archive.
	 *
	 * - 32 is the highest priority, 0 is the lowest priority.
	 * - 0 is the default priority for all unregistered SND resources.
	 * - In same priority, the first registered range is considered first.
	 * - In same priority, the last queued sound becomes winner.
	 */
	struct ScriptSoundPriorityRange {
		/** First inclusive sound ID in the priority range. */
		int16 first;
		/** Last inclusive sound ID in the priority range. */
		int16 last;
	};

	/** Ordered script-sound priority ranges owned by one page. */
	typedef Common::Array<ScriptSoundPriorityRange> ScriptSoundPriorityRanges;

	/** Construct a page owner for one concrete page type. */
	ZoombiniPage(MohawkEngine_Zoombini *vm, ZoombiniPageCategory pageCategory, ZoombiniPageType pageType);
	/** Release page-owned features, archives, and transient callbacks. */
	virtual ~ZoombiniPage();

	/** Called when the page is opened. */
	virtual void open() {}
	/**
	 * Set the background music for the page.
	 * Called after @ref ZoombiniPage::open() and before @ref ZoombiniPage::loadFeatures().
	 */
	virtual void setBackgroundMusic() {}
	/**
	 * Set the background bitmap for the page.
	 * Mainly used for dialog implementations.
	 * Concrete pages retain ownership of archive selection, resource IDs, and
	 * release-specific background rules; similar IDs do not define a shared
	 * page descriptor.
	 * Called after @ref ZoombiniPage::open() and before @ref ZoombiniPage::loadFeatures().
	 */
	virtual void setBackgroundBitmap() {}
	/** Initialize page-specific state before loading features. */
	virtual void initStates() {}
	/**
	 * Load the features for the page.
	 * Called after @ref ZoombiniPage::open(), @ref ZoombiniPage::setBackgroundMusic(),
	 * @ref ZoombiniPage::setBackgroundBitmap(), and @ref ZoombiniPage::initStates().
	 */
	virtual void loadFeatures() = 0;
	/**
	 * Initialize the page-entry SFX selector after feature loading.
	 * Puzzle pages cache this operation so an original load-time consumer can request it earlier without advancing the page flag twice.
	 */
	virtual void initSfxGroupFlags() {}
	/**
	 * Return whether the engine should restart the Snoid fidget inactivity period after loading this page.
	 * Queried after @ref ZoombiniPage::initSfxGroupFlags() and before @ref ZoombiniPage::initHelpPrompt().
	 */
	virtual bool requiresFidgetActivityResetOnLoad() const { return false; }
	/**
	 * Select and store the page-owned F1 help prompt after feature loading and any requested fidget reset.
	 * Called immediately before @ref ZoombiniPage::activatePage().
	 */
	virtual void initHelpPrompt() {}
	/**
	 * Called when the page is closed.
	 * The base implementation clears loaded features and archives.
	 */
	virtual void close();
	/**
	 * Called in every ScummVM render frames.
	 * Handles @ref ZoombiniPage::onEveryFrame() and @ref ZoombiniPage::onAnimFrame().
	 */
	void onFrame();
	/** Advance the limited page-owned work retained while a modal dialog owns rendering and input. */
	virtual void onModalFrame();
	/**
	 * Called in every ScummVM render frames.
	 *
	 * Can reach over 60FPS when the game is running fast,
	 * so only use for non-animated logic that needs to be updated as fast as possible,
	 * e.g. mouse cursor movement.
	 */
	virtual void onEveryFrame() {};
	/**
	 * Called in every Zoombini animation frames, aka ticks (60TPS).
	 *
	 * Game logic tied to animation frames (e.g. ambient sound driver) should be updated and executed here.
	 */
	virtual void onAnimFrame();

	/** Activate the page and enable input processing. */
	virtual void activatePage() { _pageActive = true; }

	/** Deactivate the page and disable input processing. */
	virtual void deactivatePage() { _pageActive = false; }
	/** Return whether this page permits the game-owned Save/Load dialog. */
	virtual bool canOpenSaveLoadDialog() const { return false; }

	/** Show a localized warning overlay for the requested duration. */
	void showWarningBox(const Common::U32String &text, uint32 durationSeconds = DEFAULT_WARNING_BOX_SHOW_SECONDS);

	/** Start the page fade-in transition. */
	void onFadeIn();
	/** Start the page fade-out transition. */
	void onFadeOut();

	/** Return whether the page has completed its close transition. */
	virtual bool isClosed();
	/** Close the page immediately as part of application shutdown. */
	void closeForQuit();

	/** Open a page-owned archive and push it onto the archive stack. */
	void openArchive(const Common::String &name);
	/**
	 * Ensure that the v1.x shared MIDI background-music archive is open.
	 *
	 * v1.x hybrid Mac/PC discs ship two device-profile archives with the SAME
	 * tMID resource IDs come from MIDIMPC.MHK (Windows/MPC) or MIDIMAC.MHK (Macintosh).
	 * This helper picks MIDIMAC.MHK when the "use_mac_midi" option is set and MIDIMPC.MHK otherwise.
	 * Callers must still gate this on a v1.x check (TLC v2.0 has no MIDI).
	 */
	void openMidiArchive();
	ZoombiniPageType getPageType() const { return _pageType; }
	ZoombiniPageCategory getPageCategory() const { return _pageCategory; }
	/**
	 * Handle a page-specific debugger command from the console's @c page command.
	 *
	 * @param argc Number of arguments, including the registered command at @p argv[0].
	 * @param argv Registered command, page subcommand, and its arguments.
	 * @param output Diagnostic text to print after the handler returns.
	 * @return True when the command was invalid or unsupported; false when it succeeded.
	 */
	virtual bool debugDoPageCommand(int argc, const char **argv, Common::String &output);
	/** Handle a built-in page-local debug action from @c page @c builtin_debug. */
	virtual bool debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output);
	/**
	 * Return the active page's debugger subcommands for the top-level page help.
	 *
	 * The returned text should contain one or more indented command synopsis and
	 * description lines. Pages without page-specific commands return an empty string.
	 */
	virtual Common::String debugGetPageCommandHelp() const { return Common::String(); }
	/** Return the built-in page-local debug actions exposed through @c page @c builtin_debug. */
	virtual Common::String debugGetBuiltinDebugCommandHelp() const { return Common::String(); }
	/**
	 * Return the current page's registered priority for a queued SND resource.
	 *
	 * Priority data is available independently of @ref usesOriginalScriptSoundQueue().
	 */
	uint8 getScriptSoundPriority(ZmbResource resource) const;
	void scheduleForceRedraw() { _doForceRedraw = true; }
	/**
	 * Queue a back-screen mutation for restoration into the persistent shape screen on the next render.
	 * @ref ZoombiniGraphics calls this automatically for every back-screen draw operation.
	 */
	void invalidateBackgroundRect(const Common::Rect &rect);

	/**
	 * Called when a SCRB/SCRS feature fires a non-voice event code during animation.
	 *
	 * Dispatches non-negative SCRS event codes on every frame that contains them.
	 * Dispatches @ref ZoombiniPage::kAnimEventM1_End at most once per @ref ZmbFeature::activateAnimate() cycle.
	 * The one-shot latch suppresses further end events until the next activation.
	 *
	 * @param feature The feature that fired the event.
	 * @param eventCode The adjusted event code (raw - 1).
	 * See @ref ZoombiniPage::AnimateEvent.
	 */
	virtual void onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) {
		(void)feature;
		(void)eventCode;
	}

	// [*] Page-level Event Handlers
	/** Route a left-button press through dialogs, features, and page logic. */
	ZmbEventHandleResult handleLButtonDown(const Common::Point &absPos, const Common::Point &relPos);
	/** Route a left-button release through dialogs, features, and page logic. */
	ZmbEventHandleResult handleLButtonUp(const Common::Point &absPos, const Common::Point &relPos);
	/** Route a right-button press through a page override, then the primary-click path. */
	ZmbEventHandleResult handleRButtonDown(const Common::Point &absPos, const Common::Point &relPos);
	/** Route a right-button release through a page override, then the primary-click path. */
	ZmbEventHandleResult handleRButtonUp(const Common::Point &absPos, const Common::Point &relPos);
	/** Handle a page-specific left-button press. */
	virtual ZmbEventHandleResult onLButtonDown(const Common::Point &absPos, const Common::Point &relPos);
	/** Handle a page-specific left-button release. */
	virtual ZmbEventHandleResult onLButtonUp(const Common::Point &absPos, const Common::Point &relPos);
	/** Optionally override the default right-button press fallback. */
	virtual ZmbEventHandleResult onRButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
		(void)absPos;
		(void)relPos;
		return ZmbEventHandleResult::kPassthrough;
	}
	/** Optionally override the default right-button release fallback. */
	virtual ZmbEventHandleResult onRButtonUp(const Common::Point &absPos, const Common::Point &relPos) {
		(void)absPos;
		(void)relPos;
		return ZmbEventHandleResult::kPassthrough;
	}
	/** Handle an upward mouse-wheel event. */
	virtual ZmbEventHandleResult onWheelUp(const Common::Point &absPos);
	/** Handle a downward mouse-wheel event. */
	virtual ZmbEventHandleResult onWheelDown(const Common::Point &absPos);
	/** Handle page-local pointer movement. */
	virtual ZmbEventHandleResult onMouseMove(const Common::Point &absPos, const Common::Point &relPos);
	/** Clear cached page-local hover cursor state before modal input takes ownership. */
	virtual void clearHoverCursor() {}
	/** Handle a page-local key press. */
	virtual ZmbEventHandleResult onKeyDown(const Common::KeyState &kbd, bool kbdRepeat);
	/** Handle a page-local key release. */
	virtual ZmbEventHandleResult onKeyUp(const Common::KeyState &kbd, bool kbdRepeat);
	/**
	 * Update or cancel transient IME composition owned by this page.
	 *
	 * @param composition Native composition state or cancellation.
	 */
	virtual ZmbEventHandleResult onImeComposition(const Common::ImeComposition &composition);
	/** Return true while this page owns native IME composition. */
	virtual bool wantsImeComposition() const { return false; }
	/** Handle a quit request before the page is destroyed. */
	virtual ZmbEventHandleResult onQuit();

	// [*] Feature Script (SCRB: Map Object)
	/**
	 * Register a feature runner and, for a nonzero SCRB ID, load its initial SCRB data.
	 *
	 * @param imgResource Image resource used by SCRB shapes or a callback renderer.
	 * @param scrbId Initial SCRB resource ID, or zero for a callback-only runner.
	 * @param frameInterval Number of animation frames between normal SCRB updates.
	 * @param flags Initial @ref ZmbFeature flags.
	 * @param eventHooks Optional owner callbacks, including the post-render drawing callback.
	 */
	ZmbFeature *loadScrbFeature(ZmbResource imgResource, int16 scrbId, uint32 frameInterval, uint32 flags, const ZmbFeature::EventHooks &eventHooks = ZmbFeature::EventHooks());
	/** Register a SCRB runner with an initial position. */
	ZmbFeature *loadScrbFeature(ZmbResource imgResource, int16 scrbId, uint32 frameInterval, const Common::Point &point, uint32 flags, const ZmbFeature::EventHooks &eventHooks = ZmbFeature::EventHooks());
	/**
	 * Register a SCRB runner whose identity is not an initial SCRB resource id.
	 * Used for runners allocated before their SCRB data is assigned through @ref ZoombiniPage::loadScrbOntoFeature().
	 */
	ZmbFeature *loadVirtualFeature(ZmbResource imgResource, uint16 runnerId, uint32 frameInterval, uint32 flags, const ZmbFeature::EventHooks &eventHooks = ZmbFeature::EventHooks());
	/**
	 * Load a callback-only SCRB feature with virtual hotspots.
	 * Creates a callback-only runner that never loads SCRB data.
	 * Stores the runner in @ref ZoombiniPage::_scrbFeatures with a zero SCRB ID.
	 * Uses @p hotspots for click detection instead of SCRB-embedded hotspot groups.
	 */
	ZmbFeature *loadScrbFeature(ZmbResource imgResource, int16 scrbId, const Common::Array<ZmbHotspot> &hotspots, uint32 frameInterval, uint32 flags, const ZmbFeature::EventHooks &eventHooks = ZmbFeature::EventHooks());
	/** Register a child feature attached to @p parentFeature. */
	ZmbFeature *loadSubFeature(ZmbFeature *parentFeature, ZmbResource imgResource, int16 scrbId);
	/**
	 * Create an unregistered chain-head feature for @ref ZoombiniPage::loadSubFeature().
	 * The bare @ref ZmbFeature stores inherited flags and the frame interval without creating a visible runner.
	 */
	ZmbFeature *createMainFeatureHead(uint32 flags);
	/**
	 * Register the image resource shared by a contiguous SCRB group.
	 * This records resource ownership without creating or parsing feature runners.
	 *
	 * @param imgResource Image resource used by every SCRB in the group.
	 * @param scrbBaseId First SCRB resource ID in the group.
	 * @param scrbCount Number of consecutive SCRB resource IDs in the group.
	 */
	void registerScrbImageGroup(ZmbResource imgResource, int16 scrbBaseId, uint16 scrbCount);
	/** Unregister a SCRB runner and release its owned feature data. */
	void unloadScrbFeature(ZmbFeature *feature);
	/**
	 * Swap the SCRB data on an already-registered feature.
	 * Load or reload SCRB resource data onto an existing feature runner without destroying or recreating it.
	 * CHAIN_SCRIPT runners preserve their current SCRB as a return target;
	 * repeated swaps are ignored while that target is pending.
	 *
	 * Preserves the feature's identity (map key), flags, event hooks, and position reference.
	 * Resets animation state (frame index, sound index, render timers) and re-runs @ref ZmbFeature::initValues().
	 *
	 * @param feature The target feature (must already be registered).
	 * @param newScrbId SCRB resource ID to load, or zero to reload the feature's current SCRB.
	 * @param scheduleRender Assigns the runner render gate after the swap.
	 * @param invalidateCurrentCoverage Adds the runner's old coverage to the current render pass.
	 * The old coverage is still queued for the next render pass when this is false.
	 */
	void loadScrbOntoFeature(ZmbFeature *feature, int16 newScrbId, bool scheduleRender = true,
							 bool invalidateCurrentCoverage = true);
	/** Play the sound attached to the feature's current SCRB/SCRS frame immediately. */
	void playCurrentFrameSound(ZmbFeature *feature);
	/**
	 * Dispatch a non-embedded sound using the owning feature's effective policy.
	 * Prefer this over a page-wide opt-in when the effect belongs to one runner.
	 */
	void dispatchFeatureSound(ZmbFeature *feature, ZmbResource resource);
	/**
	 * Explicitly enqueue a script sound that has no feature owner. The pending
	 * candidate enables arbitration for the next render pass by itself.
	 * @param resource Resolved page or system SND resource.
	 * @param soundType Mixer volume category retained by the selected winner.
	 */
	void queueScriptSoundForNextRenderPass(ZmbResource resource, Audio::Mixer::SoundType soundType = Audio::Mixer::kSFXSoundType);
	/**
	 * Register a sub-feature (already owned by a parent feature) into the page's active scrb feature map
	 * so that it is rendered independently. The page does NOT take ownership; the parent feature retains it.
	 * When the sub-feature's animation cycle ends, @ref ZoombiniPage::checkCloseFeatures() detaches it.
	 * @param subFeature The sub-feature to register
	 */
	void attachSubFeature(ZmbFeature *subFeature);

	uint32 getCurrentFrameCounter() const { return _currentFrameCounter; }
	/**
	 * Collect feature runners currently registered in the page render loop.
	 * Chain-head objects are deliberately excluded because they only own
	 * inactive sub-feature chains and are not runtime runners themselves.
	 */
	void collectDebugLoadedFeatures(Common::Array<ZmbLoadedFeatureInfo> &features) const;

	/** Run the complete feature pre-render, sort, draw, and post-render pass. */
	void renderFeatures();
	/** Draw a 310 by 16 debug text strip immediately, optionally restoring it on the next render pass. */
	void drawBuiltinDebugText(const Common::String &text, bool restoreOnNextFrame = false);
	/** Pause page progression until the next keyboard or mouse-button input is observed. */
	void waitForBuiltinDebugInput() { _builtinDebugInputWaitActive = true; }
	/** Return whether a blocking built-in debug strip owns page progression. */
	bool isBuiltinDebugInputWaitActive() const { return _builtinDebugInputWaitActive; }
	/** Release the blocking built-in debug strip without consuming the observed input. */
	void releaseBuiltinDebugInputWait() { _builtinDebugInputWaitActive = false; }
	/** Fill and outline one built-in page debug panel. */
	void drawBuiltinDebugPanel(const Common::Rect &rect);
	/** Draw centered text inside one built-in page debug panel cell. */
	void drawBuiltinDebugPanelText(const Common::String &text, const Common::Rect &rect);
	/** Draw one trait-value sprite used by the Bridge and Tunnels debug displays. */
	void drawBuiltinDebugTraitSprite(const Common::Point &anchor, byte traitKindCode, byte traitValue);
	/** Draw the next NODE/PATH diagnostic view immediately. */
	void drawBuiltinNodePaths();
	/** Draw the original 32 by 8 palette-index grid immediately. */
	void drawBuiltinPaletteGrid();
	/** Draw original runner click-rectangle diagnostics immediately. */
	void drawBuiltinRunnerClickRects(bool filterSnoids, bool showIds, int16 targetPosition);
	/** Return the number of runners in the original global-list order. */
	uint32 getBuiltinDebugRunnerCount();
	/** Detach features that completed or were scheduled for removal. */
	void checkCloseFeatures();

	/**
	 * Run the complete Snoid pre-render callback.
	 * @ref ZoombiniPage::renderFeatures() calls this before Z-sorting instead of @ref ZoombiniPage::preRenderFeature().
	 */
	void preRenderSnoid(ZmbSnoid *snoid);

	/**
	 * Pre-render pass for a single standard feature: animation logic.
	 * Called for non-Snoid features BEFORE Z-sorting.
	 * Handles frame selection, end-of-cycle events, per-frame flag checks, and sound dispatch.
	 */
	void preRenderFeature(ZmbFeature *feature);

	/**
	 * Post-render pass for a single feature: shape blitting.
	 * Called in Z-sorted order AFTER pre-render pass.
	 * Only blits shapes and computes sort rects.
	 */
	ZmbRenderResult blitShapes(ZmbFeature *feature);
	/** Blit a feature with the page's active Color Assist remap. */
	ZmbRenderResult blitShapesWithColorAssist(ZmbFeature *feature);
	/** Blit a feature using an explicitly selected palette remap mode. */
	ZmbRenderResult blitShapesInternal(ZmbFeature *feature, ZoombiniGraphics::PaletteRemapMode remapColorAssistPalette);

	/** Select the SCRB frame that should be rendered at the current frame counter. */
	int32 selectRenderFrame(ZmbFeature *feature);

	/**
	 * Render a stored Zoombini (idle pose, right-facing) at the given screen position
	 * using the given traits, and return the bounding rect of the rendered shapes.
	 *
	 * @param screenKind Target screen buffer.
	 * @param trait Trait data (hair/eye/nose/feet) to use for the idle pose.
	 * @param pos Screen position of the snoid anchor point.
	 * @return Bounding rect of all drawn shapes, or an invalid Rect if nothing was drawn.
	 */
	Common::Rect renderStoredSnoid(ZoombiniGraphics::ScreenKind screenKind, const ZmbTrait &trait, const Common::Point &pos);

	/** Release all page-owned resources and reset registries. */
	void clear();
	/** Release registered SCRB features. */
	void clearScrbFeatures();
	/** Release sub-features owned by registered parent features. */
	void clearSubFeatures();
	/** Release unregistered feature chain heads. */
	void clearMainFeatureHeads();
	/** Release registered Snoid runners. */
	void clearSnoids();
	/** Release cached shape registration offset tables. */
	void clearRegs();
	/** Release cached NODE path data. */
	void clearNode();

	// [*] SCRS: Snoid Script (Zoombini)
	/** Load a Snoid feature at its resource-defined initial position. */
	ZmbSnoid *loadSnoid(ZmbResource imgResource, int16 scrsId, uint32 flags, const ZmbFeature::EventHooks &eventHooks = ZmbFeature::EventHooks());
	/** Load a Snoid feature at an explicit screen position. */
	ZmbSnoid *loadSnoid(ZmbResource imgResource, int16 scrsId, const Common::Point &point, uint32 flags, const ZmbFeature::EventHooks &eventHooks = ZmbFeature::EventHooks());
	/** Load an active-pack Snoid at an explicit screen position. */
	ZmbSnoid *loadSnoidFromPack(uint16 snoidId, const Common::Point &point, uint32 flags, const ZmbFeature::EventHooks &eventHooks = ZmbFeature::EventHooks());
	/** Return the synthetic runner ID for a page-local dynamic pack Snoid index. */
	uint16 getDynamicPackSnoidId(uint16 snoidIdx) const;
	/** Reserve and return the next synthetic runner ID for a dynamic pack Snoid. */
	uint16 allocateDynamicPackSnoidId();

	/**
	 * Materialize pack entries as registered Snoid runners, then clear the
	 * serialized entries from @p pack.
	 *
	 * Runner registration order is determined at this call site. Callers must
	 * finish every serialized-pack read first and must not move materialization
	 * across feature registrations that establish page layering or event order.
	 */
	int16 loadSnoidsFromPack(ZmbStateActivePack &pack,
							 const Common::Point *occupiedPositions,
							 uint16 occupiedPositionCount,
							 bool loadNonOccupied,
							 uint16 firstSnoidId,
							 Common::Array<ZmbSnoid *> *loadedSnoids = nullptr);
	/** Materialize pack entries using the shared page-local dynamic Snoid ID sequence. */
	int16 loadDynamicSnoidsFromPack(ZmbStateActivePack &pack,
									const Common::Point *occupiedPositions,
									uint16 occupiedPositionCount,
									bool loadNonOccupied,
									Common::Array<ZmbSnoid *> *loadedSnoids = nullptr);

	/** Schedule materialized pack Snoids for rendering and input. */
	void schedulePackSnoids(bool activateRender, bool occupied);

	/**
	 * @param saveMode true for the save-dialog snapshot path. This preserves
	 * runner state and marks every entry occupied except on the two basecamps.
	 */
	void saveSnoidsToPack(bool saveMode = false);
	/** Count the real active-pack snoids currently materialized on this page. */
	int16 getPackSnoidCount() const;
	/**
	 * Load a SCRB-based Snoid.
	 * Identical to @ref ZoombiniPage::loadSnoid() but reads an SCRB resource instead of SCRS.
	 * Use this for town inhabitants (SCRB 4000-4007).
	 * They share animation scripts with the SCRB sub-features but are registered as independent Snoids.
	 * @p snoidId is used as the map key and must be unique.
	 * @p scrbId is the SCRB resource to parse.
	 */
	ZmbSnoid *loadSnoidFromScrb(ZmbResource imgResource, uint16 snoidId, int16 scrbId, const Common::Point &point, uint32 flags, const ZmbFeature::EventHooks &eventHooks = ZmbFeature::EventHooks());
	/** Unregister the Snoid associated with a runner ID. */
	void unloadSnoid(uint16 runnerId);
	/** Find the first registered Snoid feature with @p runnerId. */
	ZmbSnoid *getSnoid(uint16 runnerId) const;
	/** Return the Snoid for @p runnerId only when it is in the exact idle state. */
	ZmbSnoid *getIdleSnoid(uint16 runnerId) const;

	/**
	 * Return true if any stationary snoid (idle/flip/fidget) other than @p self
	 * is within @p distSquared squared-pixel distance of @p pt.
	 * Threshold is the raw squared distance; comparison is strict less-than,
	 * and only states 0/3/6 (idle/flip/fidget) are checked.
	 */
	bool isPointOccupiedByOtherSnoid(const ZmbSnoid *self, const Common::Point &pt, int32 distSquared) const;

	/**
	 * Return the squared pixel distance between two screen-space points.
	 * The result intentionally omits the square root because callers compare it
	 * with squared thresholds.
	 */
	static int32 squaredDistanceBetweenPoints(const Common::Point &first, const Common::Point &second);

	// [*] Snoid Drag Helpers
	/**
	 * Begin dragging a snoid: set drag animation and hide mouse cursor.
	 * Sets @ref kSnoidAnimState005_Drag; the cursor is hidden so the Snoid sprite acts as the cursor.
	 * Called by all interactive pages when starting a drag operation.
	 */
	void beginSnoidDrag(ZmbSnoid *snoid);

	/**
	 * End dragging a snoid: restore flags and mouse cursor visibility.
	 * Bitmask restored from saved value; cursor restored after drag loop.
	 * The caller is responsible for setting the final animation state (idle, arrive, etc.) and position.
	 */
	void endSnoidDrag(ZmbSnoid *snoid);

	// [*] Terrain Barrier Bitmap
	/**
	 * Load a terrain barrier bitmap (tBMP resource) that defines walkable areas.
	 * These bitmaps act as masks used to determine if Zoombinis or other objects can be dropped.
	 * The bitmap is 160x120 (screen / 4), 8bpp. Pixel value 1 = walkable.
	 * @param resId tBMP resource ID from the page archive (e.g. 100, 500, 1600).
	 */
	void loadTerrainBitmap(int16 resId);

	/** Return whether this page has a terrain barrier bitmap loaded. */
	bool hasTerrainBitmap() const { return _terrainBitmap != nullptr; }

	/** Mask the current composite screen outside this page's walkable terrain with black. */
	bool applyTerrainMaskToShapeScreen();

	/**
	 * Check if a screen position is on walkable terrain.
	 * Maps screen coords to bitmap coords by dividing by 4, then checks pixel == 1.
	 * If no terrain bitmap is loaded, returns false (no terrain = invalid drop).
	 */
	bool isTerrainWalkable(int16 x, int16 y) const;

	/**
	 * Validate a snoid drop against the terrain barrier bitmap and adjust
	 * position to avoid collisions with idle snoids.
	 *
	 * Flow:
	 * 1.
	 * If no terrain loaded, returns false (drop invalid).
	 * 2.
	 * Checks pixel at (snoidX/4, snoidY/4); pixel != 1 -> returns false.
	 * 3.
	 * Checks collision with idle snoids (threshold 36 sq dist approx 6px).
	 * 4.
	 * If colliding, finds a non-colliding position via random offset scan.
	 * 5.
	 * Updates snoid position and returns true.
	 *
	 * Pages with terrain: BC2(100), Bridge(1600), Caves(100), Ferry(100),
	 * Fleens(500), Hotel(100), Maze2(100), Pizza(100), Slides(100),
	 * Smoke(100), Tunnels(100).
	 * Pages WITHOUT terrain: BC1, Picker, Town - always returns false.
	 */
	bool validateTerrainDrop(ZmbSnoid *snoid);

	/**
	 * Enter the shared post-drop arrival state for an exact destination.
	 * Stores the destination, selects common image 1, and enters @ref kSnoidAnimState004_Arrive.
	 * The arrival state settles the Snoid facing right for every puzzle.
	 */
	void settleSnoidAtTarget(ZmbSnoid *snoid, const Common::Point &target);

	/**
	 * Complete the generic free-terrain branch of beginDragFeatureRunner.
	 * Target either the validated release point or the pickup point, select common image 1, face right,
	 * and enter Snoid Arrive state 4.
	 * @return True when the resolved destination exactly matches the raw release point.
	 */
	bool settleSnoidAfterTerrainDrop(ZmbSnoid *snoid, const Common::Point &pickupPos);

	/**
	 * Find a non-colliding position for a snoid by scanning random x-offsets.
	 * When called, tries up to 20 random x-offset positions (4 * random(-5,5) pixels) at the same y.
	 * Keeps the first non-colliding candidate or retains @p origin if every candidate collides.
	 */
	Common::Point findNonCollidingPosition(const ZmbSnoid *self, const Common::Point &origin, int32 distSquared) const;
	/**
	 * Find a non-colliding position inside a fixed rectangle using the shared 5x4 return grid.
	 * The first free candidate is returned; if every candidate collides, the current animation destination is retained.
	 */
	Common::Point findNonCollidingPosition(const ZmbSnoid *self, const Common::Rect &rect, int32 distSquared) const;

	/**
	 * Free the terrain barrier bitmap.
	 * Called by @ref ZoombiniPage::clear().
	 */
	void clearTerrainBitmap();

	// [*] Draw-on-Region Slot System
	// Track seat and pedestal features that serve as Snoid drop targets.
	// @ref ZoombiniPage::registerFeature() populates the slots for @ref ZmbFeature::FLAG_00002000_DRAW_ON_REG runners.
	// Pages with custom seat loading may also populate them directly.
	/** Maximum number of draw-on-reg slots available to a page. */
	static const int16 kMaxDrawOnRegSlots = 125;

	/** Number of registered draw-on-reg slots. */
	int16 _drawOnRegCount = 0;

	/**
	 * Exact seat/pedestal runner instances.
	 * SCRB resource IDs are not unique when several seats use the same script.
	 */
	ZmbFeature *_drawOnRegFeatures[kMaxDrawOnRegSlots] = {};

	/** Snap positions for drop targets. */
	Common::Point _drawOnRegSnapPositions[kMaxDrawOnRegSlots];

	/** Occupancy: 0 = empty, else = feature ID of seated snoid. */
	uint16 _drawOnRegOccupancy[kMaxDrawOnRegSlots] = {};

	/**
	 * Register a draw-on-reg slot manually (for pages with custom layout parsing).
	 * Returns the 0-based slot index.
	 */
	int16 registerDrawOnRegSlot(ZmbFeature *runner, const Common::Point &snapPos);

	/** Override the snap position of an existing slot. */
	void setDrawOnRegSnapPosition(int16 slotIdx, const Common::Point &pos);

	/** Get the occupant feature ID of a slot (0 = empty). */
	uint16 getDrawOnRegOccupant(int16 slotIdx) const;

	/** Set the occupant of a slot. */
	void setDrawOnRegOccupant(int16 slotIdx, uint16 occupantId);

	/** Clear a slot's occupant. */
	void clearDrawOnRegOccupant(int16 slotIdx);

	/** Find the slot index occupied by a given feature, or -1 if not found. */
	int16 findDrawOnRegSlotByOccupant(uint16 occupantId) const;

	/**
	 * Hit-test a point against draw-on-reg snap positions using a zone rect.
	 * Returns 0-based slot index of the first empty slot within the zone, or -1.
	 * @param pos Point to test (typically snoid position during drag).
	 * @param zoneRadius Half-size of the test rect.
	 * @param emptyOnly If true, skip occupied slots (default drag behavior).
	 */
	int16 hitTestDrawOnRegSlot(const Common::Point &pos, int16 zoneRadius, bool emptyOnly = true) const;

	/**
	 * Reset all draw-on-reg slots.
	 * Called by @ref ZoombiniPage::clear().
	 */
	void resetDrawOnRegSlots();

	// [*] Constant
	enum SystemResourceId : int16 {
		kSysResShapeBitmap0001_Dialog = 1,
		kSysResShapeBitmap0020_Credits = 20,
		kSysResShapeBitmap3000_SnoidNormal = 3000,
		kSysResShapeBitmap3001_NotiBox = 3001,
		kSysResShapeBitmap3100_SnoidCatapult = 3100,
		kSysResShapeBitmap3200_SnoidSmall = 3200,

		kSysResScrb0001_DialogOptionsFrame = 1,
		kSysResScrb0002_DialogOptionsSmallButtons = 2,
		kSysResScrb0003_DialogOptionsBigButtons = 3,
		kSysResScrb0004_DialogLoad = 4,
		kSysResScrb0005_DialogLoad = 5,
		kSysResScrb0006_DialogLoad = 6,
		kSysResScrb0007_DialogSave = 7,
		kSysResScrb0008_DialogSave = 8,
		kSysResScrb0009_DialogSave = 9,
		kSysResScrb0010_DialogMsgBox = 10,
		kSysResScrb0011_DialogMsgBox = 11,
		kSysResScrb0012_Dialog = 12,
		kSysResScrb0013_Dialog = 13,
		kSysResScrb0014_Dialog = 14,
		kSysResScrb0015_Dialog = 15,
		kSysResScrb0016_Dialog = 16,
		kSysResScrb0017_DialogHelp = 17,
		kSysResScrb0018_Dialog = 18,
		kSysResScrb0020_DialogCredits = 20,

		// Sound resources.
		kSysResSound0100_ZoombiniVoiceBase = 100,
		kSysResSound0499_ZoombiniVoiceLast = 499,
		kSysResSound0900_Ambient = 900,
		kSysResSound0901_Ambient = 901,
		kSysResSound0902_Ambient = 902,
		kSysResSound0903_Ambient = 903,
		kSysResSound0904_Ambient = 904,
		kSysResSound0905_Ambient = 905,
		kSysResSound0906_Ambient = 906,
		kSysResSound0907_Ambient = 907,
		kSysResSound0908_Ambient = 908,
		kSysResSound0909_Ambient = 909,
		kSysResSound0910_Ambient = 910,
		kSysResSound0911_Ambient = 911,
		kSysResSound0912_Ambient = 912,
		kSysResSound0913_Ambient = 913,
		kSysResSound0914_Ambient = 914,
		kSysResSound0915_Ambient = 915,
		kSysResSound0916_Ambient = 916,
		kSysResSound0917_Ambient = 917,
		kSysResSound0918_Ambient = 918,
		kSysResSound0919_Ambient = 919,
		kSysResSound0920_Ambient = 920,
		kSysResSound0921_Ambient = 921,
		kSysResSound0922_Ambient = 922,
		kSysResSound0923_Ambient = 923,
		kSysResSound0924_Ambient = 924,
		kSysResSound0925_Ambient = 925,
		kSysResSound0926_Ambient = 926,
		kSysResSound0927_Ambient = 927,
		kSysResSound0928_Ambient = 928,
		kSysResSound0929_Ambient = 929,
		kSysResSound0930_Ambient = 930,
		kSysResSound0931_Ambient = 931,
		kSysResSound0932_Ambient = 932,
		kSysResSound0933_Ambient = 933,
		kSysResSound0934_Ambient = 934,
		kSysResSound0935_Ambient = 935,
		kSysResSound0936_Ambient = 936,
		kSysResSound0937_Ambient = 937,
		kSysResSound0938_Ambient = 938,
		kSysResSound0939_Ambient = 939,
		kSysResSound0940_Ambient = 940,
		kSysResSound0941_Ambient = 941,
		kSysResSound0942_Ambient = 942,
		kSysResSound0943_Ambient = 943,
		kSysResSound0944_Ambient = 944,
		kSysResSound0996_DepartSFX = 996,
		kSysResSound0996_ButtonSFX = 996,
		kSysResSound0996_Go = 996,
		kSysResSound0997_ArriveSFX = 997,
		kSysResSound0997_Intro = 997,
		kSysResSound0997_IntroMove = 997,
		kSysResSound0997_MoveSFX = 997,
		kSysResSound0999_ButtonSFX = 999,
		kSysResSound20000_Base = 20000,
		kSysResSound20000_Route4_SmokeVoiceA = 20000,
		kSysResSound20001_Route4_SmokeVoiceB = 20001,
		kSysResSound20002_Route4_SmokeHard = 20002,
		kSysResSound20003_Route4_SmokeNoVoice = 20003,
		kSysResSound20004_Route4_MazeVoiceA = 20004,
		kSysResSound20005_Route4_MazeVoiceB = 20005,
		kSysResSound20006_Route4_MazeNoVoice = 20006,
		kSysResSound20007_Route1_TunnelsVoiceA = 20007,
		kSysResSound20008_Route1_TunnelsVoiceB = 20008,
		kSysResSound20009_Route1_TunnelsNoVoice = 20009,
		kSysResSound20010_Route1_PizzaVoiceA = 20010,
		kSysResSound20011_Route1_PizzaVoiceB = 20011,
		kSysResSound20012_Route1_PizzaNoVoice = 20012,
		kSysResSound20013_Route2_FerryVoiceA = 20013,
		kSysResSound20014_Route2_FerryVoiceB = 20014,
		kSysResSound20015_Route2_FerryHard = 20015,
		kSysResSound20016_Route2_FerryNoVoice = 20016,
		kSysResSound20017_Route2_LillyVoiceA = 20017,
		kSysResSound20018_Route2_LillyVoiceB = 20018,
		kSysResSound20019_Route2_LillyHard = 20019,
		kSysResSound20020_Route2_LillyNoVoice = 20020,
		kSysResSound20021_Route2_SlidesVoiceA = 20021,
		kSysResSound20022_Route2_SlidesVoiceB = 20022,
		kSysResSound20023_Route2_SlidesVoiceC = 20023,
		kSysResSound20024_Route2_SlidesNoVoice = 20024,
		kSysResSound20025_Route3_FleensVoiceA = 20025,
		kSysResSound20026_Route3_FleensVoiceB = 20026,
		kSysResSound20027_Route3_FleensHard = 20027,
		kSysResSound20028_Route3_FleensNoVoice = 20028,
		kSysResSound20029_Route3_HotelVoiceA = 20029,
		kSysResSound20030_Route3_HotelVoiceB = 20030,
		kSysResSound20031_Route3_HotelNoVoice = 20031,
		kSysResSound20032_Route3_NetVoiceA = 20032,
		kSysResSound20033_Route3_NetVoiceB = 20033,
		kSysResSound20034_Route3_NetNoVoice = 20034,
		kSysResSound20035_Route4_CavesVoiceA = 20035,
		kSysResSound20036_Route4_CavesVoiceB = 20036,
		kSysResSound20037_Route4_CavesNoVoice = 20037,
		kSysResSound20042_PickerAfterVideoVoice = 20042,
		kSysResSound20043_PickerAfterVideoVoice = 20043,
		kSysResSound20044_PickerAfterVideoVoice = 20044,
		kSysResSound20045_PartialSuccessBase = 20045,
		kSysResSound20045_RejectBase = 20045,
		kSysResSound20048_PartialSuccessLast = 20048,
		kSysResSound20048_PartialSuccessVariant3 = 20048,
		kSysResSound20048_RejectLast = 20048,
		kSysResSound20049_ArriveBC1Voice = 20049,
		kSysResSound20050_ArriveBC1Voice = 20050,
		kSysResSound20051_ArriveBC1Voice = 20051,
		kSysResSound20052_ArriveBC1Voice = 20052,
		kSysResSound20053_ArriveBC1Voice = 20053,
		kSysResSound20054_ArriveBC1Voice = 20054,
		kSysResSound20055_CompletionBase = 20055,
		kSysResSound20063_CompletionLast = 20063,
		kSysResSound20064_F1Replay = 20064,
		kSysResSound20065_CavesHelp = 20065,
		kSysResSound20066_HelpBase = 20066,
		kSysResSound20067_HelpLast = 20067,
		kSysResSound20068_Help = 20068,
		kSysResSound20069_HelpEasy = 20069,
		kSysResSound20070_HelpHard = 20070,
		kSysResSound20071_HelpEasy = 20071,
		kSysResSound20072_HelpHard = 20072,
		kSysResSound20073_FerryHelpEasy = 20073,
		kSysResSound20074_FerryHelpHard = 20074,
		kSysResSound20075_HelpEasy = 20075,
		kSysResSound20076_HelpLevel2 = 20076,
		kSysResSound20077_HelpHard = 20077,
		kSysResSound20078_F1Replay = 20078,
		kSysResSound20079_HelpEasy = 20079,
		kSysResSound20080_HelpHard = 20080,
		kSysResSound20081_Help = 20081,
		kSysResSound20082_BC2Voice1 = 20082,
		kSysResSound20083_BC2Voice4 = 20083,
		kSysResSound20084_BC2Voice2 = 20084,
		kSysResSound20085_BC2Voice3 = 20085,
		kSysResSound20086_Voice = 20086,
		kSysResSound20087_Voice = 20087,
		kSysResSound20088_Voice = 20088,

		// Town ambient voice tracks.
		kSysResSound20089_Ambient = 20089,
		kSysResSound20090_Ambient = 20090,
		kSysResSound20091_Ambient = 20091,
		kSysResSound20092_Ambient = 20092,
		kSysResSound20093_Ambient = 20093,
		kSysResSound20094_Route0_BridgeVoiceA = 20094,
		kSysResSound20095_Route0_BridgeVoiceB = 20095,
		kSysResSound20096_Route0_BridgeVoiceC = 20096,
		kSysResSound20097_Route0_BridgeVoiceD = 20097,
		kSysResSound20098_Route0_BridgeHard = 20098,
		kSysResSound20099_Route0_BridgeNoVoice = 20099,
		kSysResSound20100_Route5_TownVoice = 20100,
		kSysResSound20101_Route5_TownVoice = 20101,
		kSysResSound20102_Route5_TownVoice = 20102,
		kSysResSound20103_Route5_TownNoVoice = 20103,
		kSysResSound20104_TownBGM = 20104,

		// Sound ranges used by getScriptSoundPriority().
		kSysResSoundRange0900_AmbientBase = 900,
		kSysResSoundRange0944_AmbientLast = 944,
		kSysResSoundRange0996_DepartSFX = 996,
		kSysResSoundRange0997_ArriveSFX = 997,
		kSysResSoundRange20000_Base = 20000,
		kSysResSoundRange29999_Last = 29999,

		// Help text resources.
		kSysResStrl1300_HelpPicker = 1300,
		kSysResStrl1400_HelpBasecamp1 = 1400,
		kSysResStrl1500_HelpBasecamp2 = 1500,
		kSysResStrl1600_HelpTown = 1600,
		kSysResStrl1700_HelpBridge = 1700,
		kSysResStrl1800_HelpTunnels = 1800,
		kSysResStrl1900_HelpPizza = 1900,
		kSysResStrl2000_HelpFerry = 2000,
		kSysResStrl2100_HelpLilly = 2100,
		kSysResStrl2200_HelpSlides = 2200,
		kSysResStrl2300_HelpFleens = 2300,
		kSysResStrl2400_HelpHotel = 2400,
		kSysResStrl2500_HelpNet = 2500,
		kSysResStrl2600_HelpCaves = 2600,
		kSysResStrl2700_HelpSmoke = 2700,
		kSysResStrl2800_HelpMaze = 2800,
		kSysResStrl2900_HelpPickerUpdate = 2900,

		// v1.11KR only - Snoid names (30000-30006).
		kSysResStrl30000_ZoombiniNames = 30000,
		kSysResStrl30001_ZoombiniNames = 30001,
		kSysResStrl30002_ZoombiniNames = 30002,
		kSysResStrl30003_ZoombiniNames = 30003,
		kSysResStrl30004_ZoombiniNames = 30004,
		kSysResStrl30005_ZoombiniNames = 30005,
		kSysResStrl30006_ZoombiniNames = 30006,
	};

	enum SystemShapeId {
		kSystemShape0001_01_OptionsDialog = 1,
		kSystemShape0001_02_OptionsFrame = 2,
		kSystemShape0001_03_OptionsRedButtonNormal = 3,
		kSystemShape0001_04_OptionsRedButtonPressed = 4,
		kSystemShape0001_05_OptionsOnButtonNormal = 5,
		kSystemShape0001_06_OptionsOffButtonNormal = 6,
		kSystemShape0001_07_OptionsOnButtonPressed = 7,
		kSystemShape0001_08_OptionsOffButtonPressed = 8,
		kSystemShape0001_09_ShortGreenButtonNormal = 9,
		kSystemShape0001_10_ShortGreenButtonPressed = 10,
		kSystemShape0001_11_SaveLoadListFrame = 11,
		kSystemShape0001_12_LongGreenButtonNormal = 12,
		kSystemShape0001_13_LongGreenButtonPressed = 13,
		kSystemShape0001_14_LongRedButtonNormal = 14,
		kSystemShape0001_15_LongRedButtonPressed = 15,
		kSystemShape0001_16_SaveLoadScrollUpButtonNormal = 16,
		kSystemShape0001_17_SaveLoadScrollUpButtonPressed = 17,
		kSystemShape0001_18_SaveLoadScrollDownButtonNormal = 18,
		kSystemShape0001_19_SaveLoadScrollDownButtonPressed = 19,
		kSystemShape0001_20_SaveLoadInputFrame = 20,
		kSystemShape0001_21_ModalDialog = 21,
		kSystemShape0001_22_OptionsToggleLegendOn = 22,
		kSystemShape0001_23_OptionsToggleLegendOff = 23,
		kSystemShape0001_24_HelpButtonNormal = 24,
		kSystemShape0001_25_HelpButtonPressed = 25,
		kSystemShape0001_26_HelpDialogPrevButtonNormal = 26,
		kSystemShape0001_27_HelpDialogPrevButtonPressed = 27,
		kSystemShape0001_28_HelpDialogNextButtonNormal = 28,
		kSystemShape0001_29_HelpDialogNextButtonPressed = 29,
		kSystemShape0001_39_HelpButtonHover = 39,

		kSystemShape3001_01_NotiBoxShort = 1,
		kSystemShape3001_02_NotiBoxLong = 2,
	};

	// [*] NODE: Waypoint Paths for Snoid Animation
	/**
	 * Return the ZmbNode for the given resource id, or nullptr if not loaded.
	 * Snoid animation uses this during @ref kSnoidAnimState007_Depart to build a waypoint path.
	 */
	ZmbNode *getNode(int16 nodeId) const {
		auto it = _nodeMap.find(nodeId);
		return (it != _nodeMap.end()) ? it->_value : nullptr;
	}

	/**
	 * Return the first loaded ZmbNode (pages typically have at most one),
	 * or nullptr if no nodes are loaded.
	 */
	ZmbNode *getFirstNode() const {
		if (_nodeMap.empty())
			return nullptr;
		return _nodeMap.begin()->_value;
	}

	/** Public so @ref ZmbSnoid can consult it during departure routing. */
	bool isSnoidNodePathDisabled() const { return _snoidNodePathDisabled; }
	void setSnoidNodePathDisabled(bool v) { _snoidNodePathDisabled = v; }

	/**
	 * Return an immutable SCRB decode from the cache that owns its archive namespace.
	 * Page resources are retained by this page; system resources are retained by the engine.
	 * Successful decodes are cached lazily and failed decodes are not cached.
	 */
	const ZmbScriptDecoder::DecodedScrb *getDecodedScrb(ZmbResource resource);
	/**
	 * Return an immutable SCRS decode from the cache that owns its archive namespace.
	 * Page resources are retained by this page; system resources are retained by the engine.
	 * Successful decodes are cached lazily and failed decodes are not cached.
	 */
	const ZmbScriptDecoder::DecodedScrs *getDecodedScrs(ZmbResource resource);

	/**
	 * Register a Snoid SCRS pool group.
	 * Call order is significant.
	 * The first registered group is group 0 (NORMAL pool, Snoid render state 9, tBMP 3100 + NORMAL body-layer tables);
	 * the SECOND is "group 1" (REJECT pool, state 8, tBMP 3000 + general body-layer tables).
	 * At most two groups are tracked.
	 */
	void registerScrsGroup(int16 baseId, uint16 count);

	/**
	 * Resolve the playback mode for a Snoid SCRS ID.
	 * Registered group 1 selects REJECT state 8; group 0 and unregistered IDs
	 * select NORMAL state 9, so pages never hardcode the render state.
	 */
	ZmbScrsPlaybackMode resolveScrsRejectState(int16 scrsId) const;

	/**
	 * Load a page/system SCRS and start snoid playback, auto-selecting REJECT
	 * (state 8) vs NORMAL (state 9) from the registered SCRS groups. Shared
	 * entry point that removes per-call hardcoding of the render state.
	 * @param scrsResource Archive-qualified SCRS resource to play.
	 * @param completionMode Whether the Snoid returns to idle or becomes hidden after the SCRS finishes.
	 * @param suppressVoiceEvents Ignore voice markers while retaining other script events.
	 * @return true on success (resource found and playback started).
	 */
	bool startSnoidScrs(ZmbSnoid *snoid, ZmbResource scrsResource, ZmbScrsCompletionMode completionMode = ZmbScrsCompletionMode::kReturnToIdle,
						const Common::Point *endPos = nullptr, bool suppressVoiceEvents = false);
	/** Notify this page after a Snoid's state-7 path walk reaches its final target. */
	virtual void onSnoidWalkCompleted(ZmbSnoid *snoid) {
		(void)snoid;
	}

protected:
	/** Whether the page is currently active and accepting input. */
	bool _pageActive = false;

	/** Select the Color Assist palette remap for one Snoid hotspot. */
	virtual ZoombiniGraphics::PaletteRemapMode getColorAssistPaletteRemap(
		const ZmbFeature *feature, const ZmbHotspot &hotspot, ZmbResource resource) const {
		(void)feature;
		(void)hotspot;
		(void)resource;
		return ZoombiniGraphics::kPaletteRemapNone;
	}
	/** Return whether this page keeps drag capture after the pointer leaves a feature. */
	virtual bool hasStickyMouseDrag() const { return false; }
	/** End a page-specific sticky drag. */
	virtual void endStickyMouseDrag(const Common::Point &dropPos) {
		(void)dropPos;
	}
	/**
	 * Compatibility default for runners whose policy is @ref ZmbFeature::ScriptSoundPolicy::kInheritPage.
	 * New isolated dependencies should prefer feature-level policy.
	 */
	virtual bool usesOriginalScriptSoundQueue() const { return false; }
	/**
	 * Return the effective sound ranges in the original registration order.
	 *
	 * The original shared puzzle dispatch automatically installs SND 996-997 at priority 32 before most puzzle initializers append their ranges.
	 * ScummVM has no inherited mutable range table, so each puzzle override hardcodes the resulting order, including that shared range.
	 * Ferry is the exception: its initializer clears the inherited table and re-registers SND 996-997 later in its page-owned order.
	 */
	virtual const ScriptSoundPriorityRanges &getScriptSoundPriorityRanges() const {
		static const ScriptSoundPriorityRanges kNoRanges;
		return kNoRanges;
	}
	/** Return the mixer volume category for a sound submitted by an authored feature. */
	virtual Audio::Mixer::SoundType getFeatureSoundType(const ZmbFeature *feature, ZmbResource sndRes) const;
	/** Resolve a priority from ranges listed in descending registration order. */
	static uint8 lookupScriptSoundPriority(ZmbResource resource, const ScriptSoundPriorityRanges &ranges);
	/** Resolve a sound ID through ranges listed in original registration order. */
	virtual ZmbResource resolveSnoidVoiceResource(int16 soundId) const;
	/** Apply an optional page-specific correction to a raw Snoid SCRS hotspot. */
	virtual void adjustSnoidScriptHotspotForRender(const ZmbSnoid *snoid, ZmbHotspot &hotspot) const {
		(void)snoid;
		(void)hotspot;
	}
	/** Map a Snoid voice sound ID to its owning archive and resource address. */
	enum KeyboardNavDirection {
		KBD_NAV_NONE,
		KBD_NAV_LEFT,
		KBD_NAV_RIGHT,
		KBD_NAV_UP,
		KBD_NAV_DOWN,
		KBD_NAV_PAGEUP,
		KBD_NAV_PAGEDOWN
	};

	/** Convert a key event into a page-level navigation direction. */
	static KeyboardNavDirection getKeyboardNavDirection(const Common::KeyState &kbd);

	/** Notify the page that a Snoid drag has started. */
	virtual void onSnoidDragStarted(ZmbSnoid *snoid) {
		(void)snoid;
	}
	/** Notify the page that a Snoid drag has ended. */
	virtual void onSnoidDragEnded(ZmbSnoid *snoid) {
		(void)snoid;
	}

	/** Engine that owns this page. */
	MohawkEngine_Zoombini *_vm;

	/** Category used for page lifecycle and archive cleanup. */
	ZoombiniPageCategory _pageCategory;
	/** Concrete page type used for routing and saved-state lookup. */
	ZoombiniPageType _pageType;
	/** Whether page transitions use the standard fade effect. */
	bool _useFadeEffect = true;

	/** Registered top-level SCRB feature runners. */
	ZmbFeatureList<ZmbFeature> _scrbFeatures;
	/** Chain-head features from @ref ZoombiniPage::createMainFeatureHead(), not in any feature map. */
	Common::Array<ZmbFeature *> _mainFeatureHeads;
	/** Sub-features temporarily running independently (e.g. FLAG_00040000_CHAIN_SCRIPT). */
	ZmbFeatureList<ZmbFeature> _subFeatures;
	/** Registered Snoid runners keyed by their synthetic or serialized IDs. */
	ZmbFeatureList<ZmbSnoid> _snoidMap;
	/** Image resource selected for each registered or initially loaded SCRB ID. */
	Common::HashMap<int16, ZmbResource> _scrbImageResources;

	/** Monotonic counter for feature registration order tracking. */
	uint32 _nextRegistrationIndex = 0;
	/** Next index in the synthetic 60000-based ID range used by dynamic pack Snoids. */
	/** Next index in the synthetic dynamic-pack Snoid ID sequence. */
	uint16 _nextDynamicPackSnoidIdx = 0;
	/** Cached paired X/Y shape registration offsets keyed by image resource ID. */
	Common::HashMap<int16, ZmbShapeOffsetRegs *> _shapeOffsetRegsMap;
	/** Cached NODE path resources keyed by image resource ID. */
	Common::HashMap<int16, ZmbNode *> _nodeMap;
	/** Stable page-lifetime decoded page SCRB resources keyed by their raw 16-bit ID. */
	Common::StableMap<int16, ZmbScriptDecoder::DecodedScrb> _decodedScrbCache;
	/** Stable page-lifetime decoded page SCRS resources keyed by their raw 16-bit ID. */
	Common::StableMap<int16, ZmbScriptDecoder::DecodedScrs> _decodedScrsCache;

	/** Terrain barrier bitmap for walkability checks. Null if not loaded. Not owned; cached by GraphicsManager. */
	MohawkSurface *_terrainBitmap = nullptr;

	/**
	 * Snoid SCRS pool group registry.
	 * Index 0 = NORMAL pool (state 9), index 1 = REJECT pool (state 8).
	 * @ref ZoombiniPage::_scrsGroupNum stores the number of registered groups.
	 */
	int16 _scrsGroupBase[2] = {0, 0};
	/** Number of entries in each normal and reject SCRS pool. */
	uint16 _scrsGroupCount[2] = {0, 0};
	/** Number of registered SCRS pool groups. */
	int _scrsGroupNum = 0;

	/**
	 * Saved flag bitmask of the dragged Snoid, restored by @ref ZoombiniPage::endSnoidDrag().
	 * @ref ZoombiniPage::beginSnoidDrag() captures it first.
	 * It then adds @ref ZmbFeature::FLAG_00001000_TOPMOST and @ref ZmbFeature::FLAG_04000000_OVERLAY.
	 */
	uint32 _dragSavedSnoidFlags = 0;
	/**
	 * Saved Snoid animation interval while dragging.
	 * @ref ZoombiniPage::beginSnoidDrag() changes the interval to 3 while the Snoid is held.
	 * @ref ZoombiniPage::endSnoidDrag() restores the saved value on release.
	 */
	uint32 _dragSavedSnoidFrameInterval = 6;

	/** A held Snoid suspends new idle fidgets without cancelling animations already playing. */
	bool _snoidDragFidgetsSuppressed = false;
	uint16 _dragSavedFidgetThreshold = 64;

	/** Runner-list neighbors captured before the dragged Snoid is moved to the global tail. */
	ZmbFeature *_dragSavedRunnerPrevious = nullptr;
	/** Next runner captured before the dragged Snoid is moved to the global tail. */
	ZmbFeature *_dragSavedRunnerNext = nullptr;

	/** Wall-clock timestamp captured when the page starts. */
	uint32 _pageStartFrameTime = 0;
	/** Animation-frame counter captured when the page starts. */
	uint32 _pageStartFrameCounter = 0;
	/** Current wall-clock timestamp used by page timing. */
	uint32 _currentFrameTime = 0;
	/** Current 60 Hz animation-frame counter. */
	uint32 _currentFrameCounter = 0;
	/** Previous wall-clock timestamp used to advance page timing. */
	uint32 _lastFrameTime = 0;
	/** Previous animation-frame counter used to detect frame advancement. */
	uint32 _lastFrameCounter = 0;
	/** Requests a complete redraw on the next render pass. */
	bool _doForceRedraw = true;
	/** Whether the forced redraw has already been consumed by the renderer. */
	bool _forceRedrawPending = false;

	// Keep dirty coverage as a non-overlapping rectangle decomposition
	// so overlapping inputs never fill unchanged corners of their bounding box.
	/** Non-overlapping screen regions that need background restoration. */
	Common::Array<Common::Rect> _dirtyRects;
	/** Merge a rectangle into the page's non-overlapping dirty-region set. */
	bool addDirtyRect(const Common::Rect &rect);
	/** Return whether a feature's frame sound should enter the script queue. */
	bool shouldQueueFeatureSound(const ZmbFeature *feature) const;
	/** Return whether any feature currently owns the priority sound queue. */
	bool hasPriorityQueueFeature() const;
	/** Return whether the page should process queued script sounds this frame. */
	bool shouldProcessScriptSoundQueue() const;
	/** Dispatch a frame sound for a feature without a non-voice event. */
	void dispatchFeatureFrameSound(ZmbFeature *feature, int32 frameIdx);
	/** Dispatch a frame sound and its associated animation event for an active cycle. */
	void dispatchFeatureFrameSoundAndEvent(ZmbFeature *feature, int32 frameIdx);
	/**
	 * Merge the feature's stored visual coverage into the dirty region.
	 * Before preRender replaces hotspot state, this is the old on-screen
	 * coverage that must be erased by background restore. After rendering, it
	 * is the new coverage that must expand the active dirty clip.
	 */
	void markFeatureVisualCoverageDirty(ZmbFeature *feature, bool expandRenderClip);
	/**
	 * Merge current materialized coverage using aggregate or REGION_TRACK mode,
	 * rather than the previous frame's DrawRecords.
	 */
	void markPreparedFeatureVisualCoverageDirty(ZmbFeature *feature, bool expandRenderClip);
	/** Apply Snoid trait offsets and page remaps to one render hotspot. */
	bool transformSnoidHotspotForRender(const ZmbSnoid *snoid, ZmbHotspot &hs, uint8 snoidLayerShift, ZmbResource &snoidShapeRes) const;
	/** Cache the current Snoid visual coverage for dirty-region tracking. */
	void prepareSnoidVisualCoverage(ZmbSnoid *snoid, bool cacheFrame);

	/**
	 * The first active member to reach the slot becomes its runtime owner.
	 * Normal peers consume the owner's cached timing decision.
	 * Alternating groups use the original two-phase handoff.
	 */
	struct FeatureTimingGroup {
		/** Features synchronized by one timing decision. */
		Common::Array<ZmbFeature *> _members;
		/** Feature that currently owns the cached timing decision. */
		ZmbFeature *_runtimeOwner = nullptr;
		/** Cached timing result shared by the group. */
		uint8 _cachedTimingState = 0;
		/** Whether the group alternates timing phases between its members. */
		bool _alternatePhases = false;
	};
	/** Timing groups used to synchronize paired feature runners. */
	Common::Array<FeatureTimingGroup> _featureTimingGroups;
	/** Register two features as a synchronized timing group. */
	void registerFeatureTimingGroup(ZmbFeature *first, ZmbFeature *second);
	/**
	 * Register a timing group whose owner has already reached its current frame event.
	 * The follower waits for the owner's next cached timing decision.
	 */
	void registerFeatureTimingFollower(ZmbFeature *owner, ZmbFeature *follower);
	/** Register two features with alternating timing phases. */
	void registerAlternatingFeatureTimingGroup(ZmbFeature *first, ZmbFeature *second);
	/** Join an existing timing group, or make a standalone @p groupMember the runtime owner. */
	void joinFeatureTimingGroup(ZmbFeature *groupMember, ZmbFeature *newMember);
	void setFeatureTimingGroupScriptSoundPolicy(ZmbFeature *groupMember,
												ZmbFeature::ScriptSoundPolicy policy);
	/** Remove a feature from its timing group. */
	void unregisterFeatureTimingGroup(ZmbFeature *feature);
	/** Prepare timing decisions for all registered groups. */
	void prepareFeatureTimingGroups();
	/** Apply the prepared group timing decision to one feature. */
	void prepareFeatureTimingResult(ZmbFeature *feature);

	/**
	 * Materialize a non-Snoid SCRB frame after its timing gate fires.
	 * Apply POS_DELTA, the page shape hook, and REGS once, then store the transformed hotspots and visual coverage.
	 * @ref ZoombiniPage::blitShapes() consumes that payload without rebuilding it.
	 *
	 * @param materializedPosDelta Optional pre-event POS_DELTA captured for the current frame.
	 * @param preservePreparedTerminatorFrame Keep the already transformed payload on event-only frames.
	 */
	void prepareFeatureVisualCoverage(ZmbFeature *feature, const Common::Point *materializedPosDelta = nullptr,
									  bool preservePreparedTerminatorFrame = true);

	/**
	 * Collects dirty rectangles from feature loads, SCRB swaps, and other external sources between render frames.
	 *
	 * The collection must remain a region rather than a single bounding rectangle.
	 * Each RMap is merged without filling the space between disjoint rectangles.
	 */
	Common::Array<Common::Rect> _externalDirtyRects;
	/** Add a dirty region produced outside the normal feature pass. */
	void addExternalDirtyRect(const Common::Rect &rect);

	/**
	 * Allocate and initialize a registered feature runner.
	 *
	 * The supplied @p flags are copied unchanged to @ref ZmbFeature.
	 * In particular, this registration path never infers or adds
	 * @ref ZmbFeature::FLAG_01000000_DEFER_RENDER.
	 * A runner starts render-active by default, and @ref ZmbFeature::initValues()
	 * derives every render-state change from its supplied flags.
	 *
	 * A physical nonzero @p scrbId parses SCRB data and schedules its initial
	 * materialization pass before initialization.
	 * A zero @p scrbId, or a virtual runner, has no initial SCRB-load transition.
	 * With no flag that deactivates rendering, its custom render callback remains active
	 * for the post-render pass in @ref ZoombiniPage::renderFeatures().
	 */
	static ZmbFeature *registerFeature(ZoombiniPage *page, ZmbFeatureList<ZmbFeature> &featureList, ZmbResource imgResource, uint16 runnerId, int16 scrbId, uint32 frameInterval, const Common::Point &point, uint32 flags, const Common::Array<ZmbHotspot> *virtualHotspots, const ZmbFeature::EventHooks &eventHooks = ZmbFeature::EventHooks());
	/** Remove a feature from a registry and release its registration state. */
	static void deregisterFeature(ZmbFeatureList<ZmbFeature> &featureList, ZmbFeature *feature);

	/** Load NODE and PATH data for an archive-qualified resource. */
	void loadNodePath(ZmbResource nodeRes);
	/** Load paired X/Y REGS shape-registration offsets for an image resource. */
	void loadShapeOffsetRegs(ZmbResource::ArchiveKind archiveKind, int16 imgResource);

	/** Number of frames used by generic button press animations. */
	static constexpr uint32 BUTTON_PRESS_ANIMATION_FRAMES = 4;
	/** Default warning-box duration in seconds. */
	static constexpr uint32 DEFAULT_WARNING_BOX_SHOW_SECONDS = 4;
	/** Outer border palette used by warning boxes. */
	static constexpr uint32 WARNING_BOX_OUTER_COLOR = ZoombiniGraphics::kColor29_Brown;
	/** Inner border palette used by warning boxes. */
	static constexpr uint32 WARNING_BOX_INNER_COLOR = ZoombiniGraphics::kColor27_Red;
	/** Fill palette used by warning boxes. */
	static constexpr uint32 WARNING_BOX_FILL_COLOR = ZoombiniGraphics::kColor2B_Yellow;
	/** Text palette used by warning boxes. */
	static constexpr uint32 WARNING_BOX_TEXT_COLOR = ZoombiniGraphics::kColor2D_Black;
	/** Authored screen rectangle for the warning overlay. */
	const Common::Rect _warningBoxRect = Common::Rect(0x0178, 0x000C, 0x0274, 0x0048);
	/** Current warning text displayed by the warning overlay. */
	Common::U32String _warningBoxText;
	/** Animation frame at which the warning overlay is hidden. */
	uint32 _warningBoxShowUntilFrame = 0;
	/** Feature runner used to render the warning overlay. */
	ZmbFeature *_warningBoxFeature = nullptr;
	/** Prepare warning-box hotspots and visibility. */
	bool warningBox_preRender(ZmbFeature *feature);
	/** Render warning-box text after its background shapes. */
	void warningBox_onPostRender(ZmbFeature *feature);

	/** AnimateState - Helper for press-animation and toggle-animation handling */
	class AnimateState {
	public:
		/** Whether the state should dispatch a post-animation callback. */
		bool _firePostAnimationEvent = false;
		/** Frame at which the animation began. */
		uint32 _animationStartFrame = 0;
		/** Number of frames required to complete the animation. */
		uint32 _animationFrameCount = BUTTON_PRESS_ANIMATION_FRAMES;
		/** Sound resource played when the animation is triggered. */
		ZmbResource _pressSoundId;

		/** Construct an inactive animation state. */
		AnimateState() = default;
		AnimateState(ZmbResource pressSoundId) : _pressSoundId(pressSoundId) {}
		virtual ~AnimateState() = default;

		/** Start the animation at @p frameCounter. */
		void animate(uint32 frameCounter) {
			_animationStartFrame = frameCounter;
		}
		/** Return whether the animation has started and is still tracked. */
		bool isAnimating() const { return _animationStartFrame != 0; }
		/** Return whether the animation duration has elapsed. */
		bool isAnimationDone(uint32 frameCounter) const {
			return isAnimating() && _animationFrameCount <= frameCounter - _animationStartFrame;
		}
		/** Set the duration used by subsequent animation checks. */
		void setAnimateFrameCount(uint32 animFrameCount) { _animationFrameCount = animFrameCount; }
		/** Clear the active animation and post-animation latch. */
		virtual void reset() {
			_firePostAnimationEvent = false;
			_animationStartFrame = 0;
		}

	protected:
		// Subclasses should expose animation triggering through a typed method, such as a press operation.
		// Re-declare @ref ZoombiniPage::AnimateState::animate() as private in each subclass to enforce this.
	};

	// [*] ButtonState - Helper for press-animation-fire Button Handling
	class ButtonState : public AnimateState {
	public:
		/** Whether the button's hotspot should be drawn. */
		bool _drawEnabled = false;
		/** Localized text key rendered over the button. */
		ZoombiniText::Key _textKey = ZoombiniText::kNone;
		/** Normal and pressed hotspot IDs. */
		uint16 _hsNormalId = ZmbHotspot::kIndexNone;
		/** Pressed hotspot ID. */
		uint16 _hsPressedId = ZmbHotspot::kIndexNone;
		/** Normal, pressed, and hover shape IDs. */
		uint16 _shapeNormalIdx = ZmbHotspot::kShapeNone;
		/** Pressed shape ID. */
		uint16 _shapePressedIdx = ZmbHotspot::kShapeNone;
		/** Optional hover shape ID. */
		uint16 _shapeHoverIdx = ZmbHotspot::kShapeNone;
		/** Whether pointer hover is currently active. */
		bool _isHovered = false;

		/** Whether the button is disabled independently of draw state. */
		bool _isPressDisabled = false;
		/** Disabled-state shape ID, or no shape when unsupported. */
		uint16 _shapeDisabledIdx = ZmbHotspot::kShapeNone;

		/** Construct an empty button state. */
		ButtonState() = default;
		~ButtonState() override = default;
		ButtonState(ZoombiniText::Key textKey, ZmbResource pressSoundId, uint16 hsNormalId, uint16 hsPressedId, uint16 normalShapeId, uint16 pressedShapeId)
			: AnimateState(pressSoundId), _drawEnabled(true), _textKey(textKey), _hsNormalId(hsNormalId), _hsPressedId(hsPressedId), _shapeNormalIdx(normalShapeId), _shapePressedIdx(pressedShapeId) {}
		ButtonState(ZmbResource pressSoundId, uint16 hsNormalId, uint16 hsPressedId, uint16 normalShapeId, uint16 pressedShapeId)
			: AnimateState(pressSoundId), _drawEnabled(true), _hsNormalId(hsNormalId), _hsPressedId(hsPressedId), _shapeNormalIdx(normalShapeId), _shapePressedIdx(pressedShapeId) {}

		/** Set the shape used while the button is disabled. */
		void setDisabledState(uint16 disabledShapeId) {
			_shapeDisabledIdx = disabledShapeId;
		}
		/** Return whether a disabled shape has been configured. */
		bool hasDisabledState() const {
			return _shapeDisabledIdx != ZmbHotspot::kShapeNone;
		}
		/** Set the optional hover shape. */
		void setHoverState(uint16 hoverShapeId) {
			_shapeHoverIdx = hoverShapeId;
		}
		/** Return whether a hover shape has been configured. */
		bool hasHoverState() const {
			return _shapeHoverIdx != ZmbHotspot::kShapeNone;
		}
		/** Update hover state and return whether it changed. */
		bool setHovered(bool hovered) {
			if (_isHovered == hovered)
				return false;
			_isHovered = hovered;
			return true;
		}
		/** Start the button press animation and its optional sound. */
		void press(MohawkEngine_Zoombini *vm, ZmbFeature *feature, uint32 frameCounter);

	private:
		using AnimateState::animate;
	};
	typedef void (ZoombiniPage::*OnButtonActionFunc)(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs);
	typedef Common::Rect (ZoombiniPage::*ButtonGetRectFunc)(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs, const Common::Rect &drawnRect);
	/** Select normal, pressed, disabled, or hover shapes for button hotspots. */
	void genericButton_selectShapes(ZmbFeature *feature, Common::Array<ZmbHotspot> &hotspots, Common::StableMap<uint32, ButtonState> &buttonStateMap, uint16 pressedDeltaX = 0, uint16 pressedDeltaY = 0, bool validateHoverShapeCount = true);
	/** Render localized labels using the button draw records. */
	void genericButton_textRender(ZmbFeature *feature, Common::StableMap<uint32, ButtonState> &buttonStateMap, Graphics::TextAlign textAlign = Graphics::kTextAlignLeft, int16 normalDeltaY = 0, int16 pressedDeltaY = 0);
	/** Render localized labels using explicit text configuration. */
	void genericButton_textRender(ZmbFeature *feature, Common::StableMap<uint32, ButtonState> &buttonStateMap, const ZoombiniGraphics::TextConf &tc, int16 normalDeltaY = 0, int16 pressedDeltaY = 0);
	/** Render localized labels using a page-provided rectangle callback. */
	void genericButton_textRender(ZmbFeature *feature, Common::StableMap<uint32, ButtonState> &buttonStateMap, ButtonGetRectFunc getRectFunc, const ZoombiniGraphics::TextConf &tc);
	/** Update button hover states from the absolute pointer position. */
	void genericButton_updateHoverState(ZmbFeature *feature, const Common::Point &absPos, Common::StableMap<uint32, ButtonState> &buttonStateMap, const Common::HashMap<uint32, Common::Rect> &buttonRectMap);
	/** Dispatch completed button animations to the owning page callback. */
	void genericButton_action(ZmbFeature *feature, Common::StableMap<uint32, ButtonState> &buttonStateMap, OnButtonActionFunc onButtonActionFunc);
	/** Handle a button press using hotspot rectangles. */
	ZmbEventHandleResult genericButton_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, Common::StableMap<uint32, ButtonState> &buttonStateMap, OnButtonActionFunc onButtonActionFunc = nullptr);
	/** Handle a button press using an explicit rectangle map. */
	ZmbEventHandleResult genericButton_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, Common::StableMap<uint32, ButtonState> &buttonStateMap, const Common::HashMap<uint32, Common::Rect> &buttonRectMap, OnButtonActionFunc onButtonActionFunc = nullptr);
	/** Handle a button press using a page-provided rectangle callback. */
	ZmbEventHandleResult genericButton_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, Common::StableMap<uint32, ButtonState> &buttonStateMap, ButtonGetRectFunc getRectFunc, OnButtonActionFunc onButtonActionFunc = nullptr);

	// [*] ToggleButtonState - Helper for Toggle Button Handling
	class ToggleButtonState : public AnimateState {
	public:
		/** Whether the toggle is available to the player. */
		bool _enabled = false;
		/** Localized text key rendered over the toggle. */
		ZoombiniText::Key _textKey = ZoombiniText::kNone;
		/** Normal and pressed hotspot IDs. */
		uint16 _hsNormalId = ZmbHotspot::kIndexNone;
		/** Pressed hotspot ID. */
		uint16 _hsPressedId = ZmbHotspot::kIndexNone;
		/** On-state normal and pressed shape IDs. */
		uint16 _onNormalShapeIdx = ZmbHotspot::kShapeNone;
		/** On-state pressed shape ID. */
		uint16 _onPressedShapeIdx = ZmbHotspot::kShapeNone;
		/** Off-state normal and pressed shape IDs. */
		uint16 _offNormalShapeIdx = ZmbHotspot::kShapeNone;
		/** Off-state pressed shape ID. */
		uint16 _offPressedShapeIdx = ZmbHotspot::kShapeNone;
		/** Current toggle value; new toggles default to on. */
		bool _toggleState = true; // On by default

		/** Construct a toggle in its default on state. */
		ToggleButtonState() = default;
		~ToggleButtonState() override = default;
		ToggleButtonState(ZoombiniText::Key textKey, ZmbResource pressSoundId, uint16 hsNormalIdx, uint16 hsPressedIdx, uint16 onNormalShapeIdx, uint16 onPressedShapeIdx, uint16 offNormalShapeIdx, uint16 offPressedShapeIdx)
			: AnimateState(pressSoundId), _enabled(true), _textKey(textKey), _hsNormalId(hsNormalIdx), _hsPressedId(hsPressedIdx), _onNormalShapeIdx(onNormalShapeIdx), _onPressedShapeIdx(onPressedShapeIdx), _offNormalShapeIdx(offNormalShapeIdx), _offPressedShapeIdx(offPressedShapeIdx) {}
		ToggleButtonState(ZmbResource pressSoundId, uint16 hsNormalIdx, uint16 hsPressedIdx, uint16 onNormalShapeIdx, uint16 onPressedShapeIdx, uint16 offNormalShapeIdx, uint16 offPressedShapeIdx)
			: AnimateState(pressSoundId), _enabled(true), _hsNormalId(hsNormalIdx), _hsPressedId(hsPressedIdx), _onNormalShapeIdx(onNormalShapeIdx), _onPressedShapeIdx(onPressedShapeIdx), _offNormalShapeIdx(offNormalShapeIdx), _offPressedShapeIdx(offPressedShapeIdx) {}

		/** Start the toggle press animation and its optional sound. */
		void press(MohawkEngine_Zoombini *vm, ZmbFeature *feature, uint32 frameCounter);
		/** Restore the default on state and clear animation state. */
		void reset() override {
			_toggleState = true;
			AnimateState::reset();
		}

	private:
		using AnimateState::animate;
	};

	typedef void (ZoombiniPage::*OnToggleButtonPostAnimationFunc)(ZmbFeature *feature, uint32 bsIdx, ToggleButtonState &bs);
	typedef Common::Rect (ZoombiniPage::*ToggleButtonGetRectFunc)(ZmbFeature *feature, uint32 bsIdx, ToggleButtonState &bs, const Common::Rect &drawnRect);
	/** Select shapes for the current on/off toggle states. */
	void genericToggleButton_selectShapes(ZmbFeature *feature, Common::Array<ZmbHotspot> &hotspots, Common::StableMap<uint32, ToggleButtonState> &buttonStateMap, uint16 pressedDeltaX = 0, uint16 pressedDeltaY = 0);
	/** Render toggle labels with the default text alignment. */
	void genericToggleButton_textRender(ZmbFeature *feature, Common::StableMap<uint32, ToggleButtonState> &buttonStateMap, Graphics::TextAlign textAlign = Graphics::kTextAlignLeft);
	/** Render toggle labels using explicit text configuration. */
	void genericToggleButton_textRender(ZmbFeature *feature, Common::StableMap<uint32, ToggleButtonState> &buttonStateMap, const ZoombiniGraphics::TextConf &tc);
	/** Render toggle labels using a page-provided rectangle callback. */
	void genericToggleButton_textRender(ZmbFeature *feature, Common::StableMap<uint32, ToggleButtonState> &buttonStateMap, ToggleButtonGetRectFunc getRectFunc, const ZoombiniGraphics::TextConf &tc);
	/** Run post-animation callbacks for toggles that finished pressing. */
	void genericToggleButton_postAnimation(ZmbFeature *feature, Common::StableMap<uint32, ToggleButtonState> &buttonStateMap, OnToggleButtonPostAnimationFunc onButtonActionFunc);
	/** Handle a toggle press using hotspot rectangles. */
	ZmbEventHandleResult genericToggleButton_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, Common::StableMap<uint32, ToggleButtonState> &buttonStateMap);
	/** Handle a toggle press using an explicit rectangle map. */
	ZmbEventHandleResult genericToggleButton_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, Common::StableMap<uint32, ToggleButtonState> &buttonStateMap, const Common::HashMap<uint32, Common::Rect> &buttonRectMap);
	/** Handle a toggle press using a page-provided rectangle callback. */
	ZmbEventHandleResult genericToggleButton_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, Common::StableMap<uint32, ToggleButtonState> &buttonStateMap, ToggleButtonGetRectFunc getRectFunc);

protected:
	/**
	 * Preserve explicit runner order instead of positional Z-sorting.
	 * @ref ZoombiniPage::setRunnerZSortEnabled() enables or disables this mode.
	 * When enabled, @ref ZoombiniPage::buildSortedRenderList() returns @ref ZoombiniPage::_runnerOrder unchanged.
	 * Changes made through @ref ZoombiniPage::manualLinkBefore(), @ref ZoombiniPage::manualLinkAfter(),
	 * and @ref ZoombiniPage::manualLinkAtEnd() therefore persist across frames.
	 */
	bool _manualZOrder = false;

	/**
	 * When set, Snoid departure routing bypasses NODE/PATH and walks directly to the destination.
	 * Tunnels uses it for pack-seat and lane motion, then clears it before Go.
	 * Pizza enables it after the intro so delivery walks remain direct, then clears it on departure.
	 * Picker stores its shorter one-shot bypass on each shifted Snoid instead.
	 */
	bool _snoidNodePathDisabled = false;

	/** Re-link @p feature immediately before @p target, so it is drawn earlier and appears behind. */
	void manualLinkBefore(ZmbFeature *feature, ZmbFeature *target);
	/** Re-link @p feature immediately after @p target, so it is drawn later and appears in front. */
	void manualLinkAfter(ZmbFeature *feature, ZmbFeature *target);
	/** Detach @p feature and append it at the global runner-list tail. */
	void manualLinkAtEnd(ZmbFeature *feature);
	/**
	 * Restore the runner-list link captured before @p snoid was raised for dragging.
	 * Pages use this only when a non-positional render phase must not retain the temporary drag-tail link.
	 */
	void restoreSnoidPreDragRunnerOrder(ZmbSnoid *snoid);
	/** Enable or disable positional runner Z-sorting. */
	void setRunnerZSortEnabled(bool enabled);
	/**
	 * Run one positional sort pass immediately, even while manual Z-order mode is active.
	 * Use this after re-flagging runners that must change render bucket right away.
	 * The manual Z-order setting is left unchanged.
	 */
	void forceRunnerZSort();
	/** Return Snoids in the current global runner-list order. */
	void collectSnoidsInRunnerOrder(Common::Array<ZmbSnoid *> &snoids) const;
	/**
	 * Find the frontmost runner whose low feature flags exactly match @p requiredFlags.
	 * The search follows the current global runner order from tail to head and tests
	 * the aggregate click rectangle. High feature flags do not affect matching.
	 */
	ZmbFeature *findRunnerAtPoint(const Common::Point &pos, uint32 requiredFlags);
	/**
	 * Select an empty fixed seat using the shared idle-runner assignment contract.
	 * Each seat takes the first eligible runner in list order within the strict
	 * squared-distance threshold. The returned seat is selected by a random
	 * forward or reverse scan of the empty seats.
	 * @param scanForwardWhenRandomHigh Select the forward scan when the random
	 * value is above 50; false preserves the Picker/Smoke reverse convention.
	 */
	Common::Point findRandomEmptySnoidSlotPosition(const Common::Point *slotPositions,
												   int16 slotCount, int32 distanceSquared,
												   bool scanForwardWhenRandomHigh = true) const;
	/**
	 * Assign the first eligible idle Snoid in runner order to each nearby slot.
	 * States 0, 3, and 6 are eligible and each runner is assigned at most once.
	 * @param requirePackSnoid Restrict candidates to active-pack Snoids when true.
	 * @param assignedRunnerIds Receives one runner ID per slot, or zero when no candidate matches.
	 */
	void assignIdleSnoidsToSlots(const Common::Point *slotPositions, int16 slotCount,
								 int32 distanceSquared, bool requirePackSnoid,
								 Common::Array<uint16> &assignedRunnerIds) const;

private:
	/** Restore the threshold on release or page teardown, including an already-disabled value. */
	void restoreSnoidDragFidgets();

	/** Partition a feature into render/event buckets from its flags. */
	static void categorizeFeature(ZmbFeature *feature, Common::Array<ZmbFeature *> &loopAnimList, Common::Array<ZmbFeature *> &overlayList, Common::Array<ZmbFeature *> &normalList, Common::Array<ZmbFeature *> &entityList);
	/** Sort one feature bucket by authored registration and Z order. */
	static void insertionSortFeatures(Common::Array<ZmbFeature *> &list);
	/** Merge an already sorted feature bucket into another sorted list. */
	static void mergeSortedListInto(Common::Array<ZmbFeature *> &existingList, const Common::Array<ZmbFeature *> &incomingList);
	/** Build the positional render order for the current frame. */
	void buildSortedRenderList(Common::Array<ZmbFeature *> &outList);
	/** Build the event-dispatch order for the current frame. */
	void buildSortedEventList(Common::Array<ZmbFeature *> &outList);
	/** Number of process-owned root runners omitted from @ref ZoombiniPage::_runnerOrder. */
	static constexpr uint32 kBuiltinDebugOriginalRootRunnerCount = 1;
	/** Synchronize the mutable runner order with the registered feature lists. */
	void syncRunnerOrder();
	/** Erase a feature from the explicit runner order. */
	void runnerOrderErase(ZmbFeature *feature);
	/** Whether a blocking built-in debug strip is waiting for the next input. */
	bool _builtinDebugInputWaitActive = false;
	/** Whether the first render pass after an input wait must restore the built-in debug strip. */
	bool _builtinDebugTextRestorePending = false;

	/**
	 * Current global runner-list order.
	 * @ref ZoombiniPage::manualLinkBefore(), @ref ZoombiniPage::manualLinkAfter(),
	 * and @ref ZoombiniPage::manualLinkAtEnd() mutate this list directly.
	 * Positional Z-sorting replaces this order.
	 * @ref ZoombiniPage::buildSortedRenderList() supplies the sorted result.
	 */
	Common::Array<ZmbFeature *> _runnerOrder;

	/** Whether @ref close() has completed page-owned cleanup. */
	bool _isClosed = false;
};

} // End of namespace Mohawk

#endif
