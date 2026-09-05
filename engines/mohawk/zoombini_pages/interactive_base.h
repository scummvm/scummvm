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

#ifndef MOHAWK_ZOOMBINI_PAGES_INTERACTIVE_BASE_H
#define MOHAWK_ZOOMBINI_PAGES_INTERACTIVE_BASE_H

#include "audio/mixer.h"
#include "mohawk/zoombini_page.h"

namespace Mohawk {

/** Snapshot of a puzzle's chance budget for the debug "puzzle chances" command. */
struct ZmbChanceInfo {
	/**
	 * Classification of how a page counts the player's "chances", reported by the
	 * debug "puzzle chances" command.
	 */
	enum class ZmbChanceType {
		/** Page does not represent a puzzle chance budget. */
		kNone, // Page is not a puzzle (transitions, resting, rodmap, shelters).
		/** Puzzle does not use the generic mistake or submit chance model. */
		kAmorphous, // A puzzle with an untracked or page-specific resource (Lilly, Smoke, Maze).
		/** Puzzle permits unlimited attempts. */
		kInfinite, // A puzzle with unlimited tries (Ferry, Slides).
		/** One chance is consumed by each submitted answer. */
		kSubmit, // One chance consumed per submit (Fleens, Net).
		/** One chance is consumed by each mistake. */
		kMistake // One chance consumed per mistake (Bridge, Caves, Pizza, Hotel, Tunnels).
	};

	/** Classification used by debug chance reporting. */
	ZmbChanceType type = ZmbChanceType::kNone;
	/**
	 * Per-visit maximum budget granted at puzzle start; -1 when unbounded or
	 * not tracked by the current implementation.
	 */
	int16 opportunities = -1;
	/** Chances consumed so far; -1 when not tracked. */
	int16 used = -1;
	/**
	 * Page-specific event that consumes one chance.
	 * A null event uses the generic "mistake" or "submit" label.
	 */
	const char *unitName = nullptr;

	ZmbChanceInfo() = default;
	/** Construct an unbounded snapshot with the supplied classification. */
	ZmbChanceInfo(ZmbChanceType t) : type(t) {}
	/** Construct a fully populated chance snapshot. */
	ZmbChanceInfo(ZmbChanceType t, int16 opps, int16 u, const char *unit = nullptr)
		: type(t), opportunities(opps), used(u), unitName(unit) {}

	/** Remaining chances, or -1 when unbounded/unknown. */
	int16 chancesLeft() const {
		return (0 <= opportunities && 0 <= used) ? static_cast<int16>(opportunities - used) : -1;
	}

	/** Return the stable textual name used by the debug command. */
	static const char *typeName(ZmbChanceType t) {
		switch (t) {
		case ZmbChanceType::kNone:
			return "none";
		case ZmbChanceType::kAmorphous:
			return "amorphous";
		case ZmbChanceType::kInfinite:
			return "infinite";
		case ZmbChanceType::kSubmit:
			return "submit";
		case ZmbChanceType::kMistake:
			return "mistake";
		default:
			return "?";
		}
	}
};

/**
 * Base class for pages that accept pointer/keyboard interaction.
 *
 * This layer owns the shared input and presentation infrastructure used by
 * puzzles and shelters: Go/Map/Help controls, notification boxes, sticky and
 * continuous buttons, Zoombini drag-and-drop, departure sequencing, and the
 * debug command hooks. Concrete pages supply their own feature callbacks and
 * implement only the page-specific acceptance, storage, or puzzle policy.
 *
 * A Go action is deliberately two-phase. The page arms a departure, the
 * animation frame callback waits for the departure sound and walk-off state,
 * and @ref ZoombiniInteractive::executeDeparture() performs the actual page
 * transition. This keeps input handling and resource cleanup consistent
 * across page types.
 */
class ZoombiniInteractive : public ZoombiniPage {
public:
	/** Create an interactive page for @p pageType. */
	ZoombiniInteractive(MohawkEngine_Zoombini *vm, ZoombiniPageType pageType);
	/** Release shared interactive features and pending audio. */
	~ZoombiniInteractive() override;

	/** Advance shared button, drag, notification, and departure state. */
	void onAnimFrame() override;
	/** Keep the ambient scheduler active while a modal dialog freezes all other puzzle work. */
	void onModalFrame() override;
	/** Dispatch keyboard input to the active page controls. */
	ZmbEventHandleResult onKeyDown(const Common::KeyState &kbd, bool kbdRepeat) override;
	/** Dispatch pointer motion to the active page controls and drag state. */
	ZmbEventHandleResult onMouseMove(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Finish a Snoid drag or dispatch the release to the page features. */
	ZmbEventHandleResult onLButtonUp(const Common::Point &absPos, const Common::Point &relPos) override;

	// [*] Notification Box
	/** Show a short notification that expires after its brief display cycle. */
	void showNotiBoxShort(const Common::U32String &ustr);
	/** Hide the short notification immediately. */
	void hideNotiBoxShort();
	/** Show a long notification with caller-supplied text. */
	void showNotiBoxLong(const Common::U32String &ustr);
	/** Show a long notification that remains until another owner replaces it. */
	void showNotiBoxLongPersistent(const Common::U32String &ustr);
	/** Show a localized long notification identified by @p textKey. */
	void showNotiBoxLong(ZoombiniText::Key textKey);

	/**
	 * Debug: finish the current puzzle and depart.
	 * Called from the console @c puzzle finish command.
	 * Ferry, Net, Maze, Caves, and Slides use their page-specific completion/Go handling. Other puzzles
	 * serialize every runner as accepted and close directly into Xfer.
	 */
	void debugForceFinish();
	/**
	 * Override to set any page-specific state needed for a forced debug departure.
	 * Default implementation does nothing.
	 */
	virtual void debugPrepareForDeparture() {}
	/**
	 * Override to return a human-readable description of the current puzzle rule/answer.
	 * Default implementation returns "(no answer required)".
	 */
	virtual Common::String debugGetAnswer() const { return "(no answer required)"; }
	/**
	 * Override to describe how the page counts the player's chances, for the debug "puzzle chances" command.
	 * @ref ZmbChanceInfo::ZmbChanceType::kNone marks a non-puzzle page.
	 */
	virtual ZmbChanceInfo debugGetChances() const { return ZmbChanceInfo(); }
	/**
	 * Return whether the active page supports the explicit chance-set command.
	 * The default is false because a chance report does not imply that its
	 * underlying state can be changed by the debugger.
	 */
	virtual bool debugCanSetChances() const { return false; }
	/**
	 * Set the number of remaining finite chances or page-specific resource uses for the debug command.
	 * Triggers a logic or animation chained with the page's normal chance-consumption behavior.
	 * @return True when the active page applied the requested value.
	 */
	virtual bool debugSetChances(int16 remaining) {
		(void)remaining;
		return false;
	}
	/**
	 * Override to append page-specific finite budgets to the debug
	 * "puzzle chances" output. Default: no additional budget.
	 */
	virtual Common::String debugGetChanceDetails() const { return Common::String(); }
	/** Describe the shared built-in debug actions exposed by the debugger. */
	Common::String debugGetGlobalDebugCommandHelp() const;
	/** Run one shared built-in debug action from the debugger console. */
	bool debugDoGlobalDebugCommand(int argc, const char **argv, Common::String &output);
	/** Return whether the page supplies an F1 help sound for debug replay. */
	bool debugCanReplayActiveHelpSound() const { return _activeHelpSoundId.hasId(); }
	/** Replay the page-owned F1 help sound for the page builtin_debug command. */
	virtual bool debugReplayActiveHelpSound();

protected:
	/** Handle a page-owned built-in debug key before shared retail shortcuts. */
	virtual ZmbEventHandleResult onDebugKeyDown(const Common::KeyState &kbd) {
		(void)kbd;
		return ZmbEventHandleResult::kPassthrough;
	}
	/** Handle a shared built-in debug key after page and retail shortcuts. */
	ZmbEventHandleResult onGlobalDebugKeyDown(const Common::KeyState &kbd);
	/** Shared actions accepted by the built-in debug console and keyboard dispatcher. */
	enum class GlobalDebugAction {
		kInvalid,
		kEnable,
		kDisable,
		kStepOff,
		kStep,
		kResetFidget,
		kPaths,
		kSound,
		kPalette,
		kMergeRect,
		kUnlockRoutes,
		kRunnerRectsAll,
		kRunnerRectsFiltered,
		kRunnerIdsAll,
		kRunnerIdsFiltered,
		kRunnerDebug,
		kForceParty,
		kDragCoords
	};
	/** Console-facing names for the shared built-in debug actions. */
	static constexpr const char *kGlobalDebugActionEnable = "enable";
	static constexpr const char *kGlobalDebugActionDisable = "disable";
	static constexpr const char *kGlobalDebugActionStepOff = "step-off";
	static constexpr const char *kGlobalDebugActionStep = "step";
	static constexpr const char *kGlobalDebugActionResetFidget = "reset-fidget";
	static constexpr const char *kGlobalDebugActionPaths = "paths";
	static constexpr const char *kGlobalDebugActionSound = "sound";
	static constexpr const char *kGlobalDebugActionPalette = "palette";
	static constexpr const char *kGlobalDebugActionMergeRect = "merge-rect";
	static constexpr const char *kGlobalDebugActionUnlockRoutes = "unlock-routes";
	static constexpr const char *kGlobalDebugActionRunnerRectsAll = "runner-rects-all";
	static constexpr const char *kGlobalDebugActionRunnerRectsFiltered = "runner-rects-filtered";
	static constexpr const char *kGlobalDebugActionRunnerIdsAll = "runner-ids-all";
	static constexpr const char *kGlobalDebugActionRunnerIdsFiltered = "runner-ids-filtered";
	static constexpr const char *kGlobalDebugActionRunnerDebug = "runner-debug";
	static constexpr const char *kGlobalDebugActionForceParty = "force-party";
	static constexpr const char *kGlobalDebugActionDragCoords = "drag-coords";
	/** Parse one console-facing shared built-in debug action name. */
	static GlobalDebugAction parseGlobalDebugAction(const Common::String &action);
	/** Run one typed shared built-in debug action. */
	bool runGlobalDebugAction(GlobalDebugAction action, Common::String &output);
	/** Draw one ASCII message through the original 310 by 16 debug strip. */
	void showBuiltinDebugText(const Common::String &text, bool waitForInput = false);
	/** Refresh the drag NotiBox from the current Ctrl+R display mode. */
	void updateBuiltinDragNotification();
	/** Sort walk-in runners by their destination position. */
	static bool compareWalkerPositions(ZmbSnoid *left, ZmbSnoid *right);

	/** Notify the page that a Snoid has entered drag mode. */
	void onSnoidDragStarted(ZmbSnoid *snoid) override;
	/** Notify the page that a Snoid has left drag mode. */
	void onSnoidDragEnded(ZmbSnoid *snoid) override;
	bool hasStickyMouseDrag() const override { return isDragging(); }
	void endStickyMouseDrag(const Common::Point &dropPos) override { endDrag(dropPos); }
	virtual void endDrag(const Common::Point &dropPos) {
		(void)dropPos;
	}

	/**
	 * State for a button whose action continues while it is held.
	 * e.g. storage scroll buttons
	 *
	 * Storage scroll buttons use this state to distinguish normal, hover, and
	 * pressed shapes while the owning page repeats the associated action.
	 */
	class ContinuousButtonState {
	public:
		/** Whether the button participates in hit testing and rendering. */
		bool _enabled = false;
		/** Whether the button is currently held down. */
		bool _pressed = false;
		/** Resting hotspot index used for hit testing. */
		uint16 _hsNormalIdx = ZmbHotspot::kIndexNone;
		/** Pressed hotspot index used while the button is held. */
		uint16 _hsPressedIdx = ZmbHotspot::kIndexNone;
		/** Resting shape ID. */
		uint16 _shapeNormalId = ZmbHotspot::kShapeNone;
		/** Pressed shape ID. */
		uint16 _shapePressedId = ZmbHotspot::kShapeNone;
		/** Optional hover shape ID. */
		uint16 _shapeHoverId = ZmbHotspot::kShapeNone;
		/** Whether the pointer is currently over the button. */
		bool _isHovered = false;

		/** Construct a disabled button with no assigned shapes. */
		ContinuousButtonState() = default;
		/** Release the button state. */
		virtual ~ContinuousButtonState() = default;
		/** Construct an enabled button with normal and pressed hotspots/shapes. */
		ContinuousButtonState(uint16 hsNormalIdx, uint16 hsPressedIdx, uint16 normalShapeId, uint16 pressedShapeId)
			: _enabled(true), _hsNormalIdx(hsNormalIdx), _hsPressedIdx(hsPressedIdx), _shapeNormalId(normalShapeId), _shapePressedId(pressedShapeId) {
		}

		/** Assign the shape used while the pointer hovers the button. */
		void setHoverState(uint16 hoverShapeId);
		/** Return true when a dedicated hover shape is configured. */
		bool hasHoverState() const;
		/** Set hover state and report whether the visual state changed. */
		bool setHovered(bool hovered);
		/** Mark the button as held. */
		void press();
		/** Mark the button as released. */
		void release();
	};
	/** Select continuous-button shapes for the current frame and press state. */
	void continuousButton_selectShapes(ZmbFeature *feature, Common::Array<ZmbHotspot> &hotspots, Common::StableMap<uint32, ContinuousButtonState> &contButtonStateMap, uint16 pressedDeltaX = 0, uint16 pressedDeltaY = 0);

	// [*] Three Buttons
	/** Configure the primary Go button rectangle and normal/pressed shapes. */
	void setGoButton(const Common::Rect &rect, uint16 shapeDisabledId, uint16 shapeEnabledId, uint16 shapePressedId);
	/** Configure the secondary Go button rectangle and normal/pressed shapes. */
	void setSecondGoButton(const Common::Rect &rect, uint16 shapeDisabledId, uint16 shapeEnabledId, uint16 shapePressedId);
	/** Configure the Map button rectangle and normal/pressed shapes. */
	void setMapButton(const Common::Rect &rect, uint16 shapeNormalId, uint16 shapePressedId);
	/** Configure the Help button rectangle and load its feature. */
	void setHelpButton(const Common::Rect &rect);

	/** Return the primary Go shape for the requested enabled state. */
	uint16 getGoButtonNormalShapeId(bool isPressable) {
		if (isPressable)
			return _goButtonShapeEnabledId;
		else
			return _goButtonShapeDisabledId;
	}
	/** Return the secondary Go shape for the requested enabled state. */
	uint16 getSecondGoButtonNormalShapeId(bool isPressable) {
		if (isPressable)
			return _secondGoButtonShapeEnabledId;
		else
			return _secondGoButtonShapeDisabledId;
	}

	/**
	 * Load the shared Go/Map feature from @p bitmapResId.
	 *
	 * @param separateBitmapResources True when shape N is stored as the image
	 * resource `bitmapResId + N - 1` instead of one multi-shape tBMP.
	 */
	void loadGoMapButtonsFeature(int16 bitmapResId, bool separateBitmapResources = false);
	/** Load the shared Help button feature. */
	void loadHelpButtonFeature();
	/** Build navigation and Help hotspots for a page-owned callback runner. */
	void buildEmbeddedControlHotspots(int16 bitmapResId, Common::Array<ZmbHotspot> &goMapHotspots, Common::Array<ZmbHotspot> &helpHotspots, bool separateBitmapResources = false);
	/** Bind navigation and Help visual invalidation to a page-owned callback runner. */
	void bindEmbeddedControlFeature(ZmbFeature *feature);
	/** Configure a TLC hover shape when the release provides that bitmap. */
	void setTlcButtonHoverIfPresent(ButtonState &buttonState, uint16 hoverShapeId, const ZmbResource &bitmapRes);
	/** Configure TLC-specific Go and Map hover resources. */
	void configureTlcGoMapButtonHover(int16 bitmapResId);
	/** Update TLC Go/Map hover state from the current pointer position. */
	void updateTlcButtonHover(const Common::Point &absPos);

	/** Render Go/Map controls when each button uses a separate bitmap resource. */
	ZmbRenderResult goMapButtons_renderSeparateBitmaps(ZmbFeature *feature);
	/** Adjust Go/Map hotspot rectangles for their separate bitmap layout. */
	void goMapButtons_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** Record the rendered Go/Map button coverage. */
	void goMapButtons_onPostRender(ZmbFeature *feature);
	/** Apply the selected Go/Map button action. */
	void goMapButtons_onButtonAction(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs);
	/** Handle a pointer press on a Go/Map control. */
	ZmbEventHandleResult goMapButtons_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);

	/** Adjust the Help button hotspot geometry. */
	void helpButton_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** Record the rendered Help button coverage. */
	void helpButton_onPostRender(ZmbFeature *feature);
	/** Apply the Help button animation state. */
	void helpButton_onPostAnimation(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs);
	/** Handle a pointer press on the Help control. */
	ZmbEventHandleResult helpButton_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);

	enum ThreeButtonHotspotIdx {
		kHotspotGoButtonNormal = 0,
		kHotspotSecondGoButtonNormal = 1,
		kHotspotMapButtonNormal = 2,
		kHotspotGoButtonPressed = 3,
		kHotspotSecondGoButtonPressed = 4,
		kHotspotMapButtonPressed = 5,

		kHotspotHelpButtonNormal = 0,
		kHotspotHelpButtonPressed = 1,
	};

	enum ThreeButtonIdx {
		kThreeButtons_Go = 0,
		kThreeButtons_SecondGo,
		kThreeButtons_Map,
		kThreeButtons_Help,
	};

	enum ThreeButtonBitmapResId {
		kBasecamp1ButtonBitmapResId = 2100,
		kBasecamp2ButtonBitmapResId = 9000,
	};

	// Z1-20U/TLC v2.0 release only: hover shapes are yellow-outline variants
	// absent from the 1.x Broderbund/Korean releases.
	enum ThreeButtonShapeIdx {
		kShapeBasecamp1GoRouteUpButtonNormal = 1,
		kShapeBasecamp1GoRouteUpButtonPressed = 2,
		kShapeBasecamp1GoRouteDownButtonNormal = 3,
		kShapeBasecamp1GoRouteDownButtonPressed = 4,
		kShapeBasecamp1GoRouteUpButtonHover = 17,
		kShapeBasecamp1GoRouteDownButtonHover = 18,
		kShapeBasecamp1MapButtonHover = 19,
		kShapeBasecamp2GoButtonNormal = 1,
		kShapeBasecamp2GoButtonPressed = 2,
		kShapeBasecamp2MapButtonNormal = 5,
		kShapeBasecamp2MapButtonPressed = 6,
		kShapeBasecamp2GoButtonHover = 17,
		kShapeBasecamp2MapButtonHover = 19,

		kShapeGoButtonNormal = 2,
		kShapeGoButtonPressed = 3,
		kShapeMapButtonNormal = 5,
		kShapeMapButtonPressed = 6,
		kShapeGoButtonHover = 7,
		kShapeMapButtonHover = 8,
		kShapePickerGoButtonNormal = 9,
		kShapePickerGoButtonPressed = 10,
		kShapePickerMapButtonNormal = 11,
		kShapePickerMapButtonPressed = 12,
		kShapePickerGoButtonHover = 17,
		kShapePickerMapButtonHover = 18,
	};

	// [*] Notification Box
	/** Number of 60 FPS frames for the long notification box. */
	constexpr static uint32 NOTIBOX_LONG_SHOW_FRAME_DURATION = 90;
	/** Show a short or long localized notification box with the requested lifetime. */
	void showNotiBox(const Common::U32String &ustr, bool isNotiBoxLong, bool persistent);

	enum NotiBoxHotspotIdx {
		kHotspotNotiBoxShort = 0,
		kHotspotNotiBoxLong = 1,
	};

	/** Select the short or long notification-box shape. */
	void notiBox_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** Draw notification text after the notification-box shape. */
	void notiBox_onPostRender(ZmbFeature *feature);

	// [*] StickyButtonState - Helper for sticky button handling.
	// Pressing toggles @ref ZoombiniInteractive::StickyButtonState::_isStuck immediately without a frame animation.
	/**
	 * State for a button that toggles and remains selected after release.
	 *
	 * Pressing toggles @ref ZoombiniInteractive::StickyButtonState::_isStuck
	 * immediately without waiting for a frame animation.
	 */
	class StickyButtonState {
	public:
		/** Whether the button is available for interaction. */
		bool _enabled = false;
		/** Sound played when changing from unselected to selected. */
		ZmbResource _pressSoundId;
		/** Sound played when changing from selected to unselected. */
		ZmbResource _releaseSoundId;
		/** Optional localized label key associated with the button. */
		ZoombiniText::Key _textKey = ZoombiniText::kNone;
		/** Hotspot shown at rest. */
		uint16 _hsNormalId = ZmbHotspot::kIndexNone;
		/** Hotspot retained for press hit testing. */
		uint16 _hsPressedId = ZmbHotspot::kIndexNone;
		/** Shape shown when the button is not selected. */
		uint16 _normalShapeIdx = ZmbHotspot::kShapeNone;
		/** Shape shown when the button is selected. */
		uint16 _pressedShapeIdx = ZmbHotspot::kShapeNone;
		/** Whether the button is currently selected. */
		bool _isStuck = false;

		StickyButtonState() = default;
		~StickyButtonState() = default;
		StickyButtonState(ZmbResource pressSoundId, uint16 hsNormalIdx, uint16 hsPressedIdx, uint16 normalShapeIdx, uint16 pressedShapeIdx)
			: _enabled(true), _pressSoundId(pressSoundId), _releaseSoundId(pressSoundId), _hsNormalId(hsNormalIdx), _hsPressedId(hsPressedIdx), _normalShapeIdx(normalShapeIdx), _pressedShapeIdx(pressedShapeIdx) {}
		StickyButtonState(ZmbResource pressSoundId, ZmbResource releaseSoundId, uint16 hsNormalIdx, uint16 hsPressedIdx, uint16 normalShapeIdx, uint16 pressedShapeIdx)
			: _enabled(true), _pressSoundId(pressSoundId), _releaseSoundId(releaseSoundId), _hsNormalId(hsNormalIdx), _hsPressedId(hsPressedIdx), _normalShapeIdx(normalShapeIdx), _pressedShapeIdx(pressedShapeIdx) {}
		StickyButtonState(ZoombiniText::Key textKey, ZmbResource pressSoundId, uint16 hsNormalIdx, uint16 hsPressedIdx, uint16 normalShapeIdx, uint16 pressedShapeIdx)
			: _enabled(true), _textKey(textKey), _pressSoundId(pressSoundId), _releaseSoundId(pressSoundId), _hsNormalId(hsNormalIdx), _hsPressedId(hsPressedIdx), _normalShapeIdx(normalShapeIdx), _pressedShapeIdx(pressedShapeIdx) {}
		StickyButtonState(ZoombiniText::Key textKey, ZmbResource pressSoundId, ZmbResource releaseSoundId, uint16 hsNormalIdx, uint16 hsPressedIdx, uint16 normalShapeIdx, uint16 pressedShapeIdx)
			: _enabled(true), _textKey(textKey), _pressSoundId(pressSoundId), _releaseSoundId(releaseSoundId), _hsNormalId(hsNormalIdx), _hsPressedId(hsPressedIdx), _normalShapeIdx(normalShapeIdx), _pressedShapeIdx(pressedShapeIdx) {}

		/** Return true when the toggle is in its selected state. */
		bool isStuck() const { return _isStuck; }

		/** Toggle state and play the configured press/release sound. */
		void toggle(MohawkEngine_Zoombini *vm);
		/** Restore the unselected state without playing a sound. */
		void reset() { _isStuck = false; }
	};

	typedef void (ZoombiniInteractive::*OnStickyButtonActionFunc)(ZmbFeature *feature, uint32 bsIdx, StickyButtonState &bs);
	/** Select sticky-button shapes for the current selection state. */
	void genericStickyButton_selectShapes(ZmbFeature *feature, Common::Array<ZmbHotspot> &hotspots, Common::StableMap<uint32, StickyButtonState> &buttonStateMap);
	/** Handle a sticky-button click using hotspot geometry. */
	ZmbEventHandleResult genericStickyButton_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, Common::StableMap<uint32, StickyButtonState> &buttonStateMap, OnStickyButtonActionFunc onActionFunc = nullptr);
	/** Handle a sticky-button click using explicit button rectangles. */
	ZmbEventHandleResult genericStickyButton_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, Common::StableMap<uint32, StickyButtonState> &buttonStateMap, const Common::HashMap<uint32, Common::Rect> &buttonRectMap, OnStickyButtonActionFunc onActionFunc = nullptr);

	/** Enable or disable both the primary and secondary Go buttons. */
	void setGoButtonsEnabled(bool enabled);

	/**
	 * Complete a pending departure instead of processing an ordinary click.
	 * @ref ZoombiniInteractive::_departureState identifies this state.
	 */
	ZmbEventHandleResult onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) override;

	/**
	 * Called when the Go button is activated.
	 * The default implementation plays SND 996 and starts the common departure state machine.
	 * @ref ZoombiniInteractive::onAnimFrame() polls @ref ZoombiniInteractive::isDepartSfxDone().
	 * It calls @ref ZoombiniInteractive::executeDeparture() after the poll reports completion.
	 *
	 * Overrides set @ref ZoombiniInteractive::_departXferSrcSiPage, perform page-specific work,
	 * and then call @ref ZoombiniInteractive::onGoButtonActivated().
	 */
	virtual void onGoButtonActivated();
	/** Handle a click on a disabled primary Go button. */
	virtual void onDisabledGoButtonActivated();

	/**
	 * Called when the secondary Go button is activated.
	 * The default implementation plays the departure SFX and starts the common departure state machine.
	 */
	virtual void onSecondGoButtonActivated();
	/** Handle a click on a disabled secondary Go button. */
	virtual void onDisabledSecondGoButtonActivated();

	// [*] Departure state machine
	/** States of the page departure sequence. */
	enum class ZmbDepartureState {
		/** Idle state; no departure in progress. */
		kIdle,
		/** Go button triggered; preparing for departure. */
		kTriggered,
		/** Departure animation currently playing (input usually locked). */
		kAnimating,
		/** Sequence completed; awaiting final page transition. */
		kCompleted
	};

	/**
	 * Xfer source page for the standard departure.
	 * Set before calling @ref ZoombiniInteractive::onGoButtonActivated().
	 */
	ZmbSrcPageKind _departXferSrcSiPage = ZmbSrcPageKind::kMinus1;

	/** Current state of the page departure sequence. */
	ZmbDepartureState _departureState = ZmbDepartureState::kIdle;
	/** Return true while the departure animation or its completion hand-off is pending. */
	bool isDeparturePending() const {
		return _departureState == ZmbDepartureState::kAnimating || _departureState == ZmbDepartureState::kCompleted;
	}
	/** Return true for every non-idle departure phase, including a page-specific trigger phase. */
	bool isDepartureActive() const { return _departureState != ZmbDepartureState::kIdle; }
	/**
	 * Advance the common SFX-driven departure after the page render.
	 * Pages with authored completion gates override this and advance @ref ZoombiniInteractive::_departureState themselves.
	 */
	virtual void updateDepartureState();
	/** Return true when the page controller continues during an active departure. */
	virtual bool runsControllerDuringDeparture() const { return false; }
	/** Return true when the shared ambient driver continues after departure-time page controller work. */
	virtual bool runsAmbientDuringDeparture() const { return runsControllerDuringDeparture(); }
	/**
	 * Run page-owned controller work after the current render and before ambient audio selection.
	 * Pages whose original controller consumes render callbacks in the same frame override this hook.
	 */
	virtual void onPostRenderFrame() {}
	/** Run page-owned controller work after the shared ambient driver. */
	virtual void onPostAmbientFrame() {}
	/** True while a debug finish is forcing every active runner to be accepted. */
	bool _debugFinishAllAccepted = false;
	/**
	 * True during the asynchronous Go-button press animation.
	 * This adaptation must prevent other page or feature clicks from interleaving with the authored synchronous delay.
	 */
	bool _goButtonPressPending = false;
	/**
	 * One click received while the asynchronous Go press is still rendering.
	 * Defer it until the transition is armed so it becomes a departure skip,
	 * matching the interaction boundary of the authored synchronous delay.
	 */
	bool _deferredGoClickPending = false;
	/** Absolute position of a click deferred during Go-button animation. */
	Common::Point _deferredGoClickAbsPos;
	/** Page-relative position of a click deferred during Go-button animation. */
	Common::Point _deferredGoClickRelPos;

	/**
	 * Execute the page departure transition.
	 * Called by @ref ZoombiniInteractive::onAnimFrame() when departure SFX finishes.
	 * Default: saves snoid runners back to active pack, routes non-occupied snoids to the resting pack,
	 * selects the Xfer source and destination, and closes the page.
	 * Override for custom departure logic (e.g. BC1/BC2 save pack state).
	 */
	virtual void executeDeparture();

	/**
	 * Route non-occupied failed Snoids from the active pack to the appropriate resting pack.
	 * The current route selects BC0, BC1, or BC2.
	 * Container puzzles (Pizza/Slides/Net/Maze) route to BC0/BC1/BC2.
	 */
	void routeNonOccupiedToRestingPack();

	// [*] Departure walk-off animation infrastructure
	// Shared by Bridge, BC1, BC2, Tunnels, Pizza, Net.

	/**
	 * Start the shared departure walk-off animation.
	 * Iterates idle snoids, sets walk-to-target with staggered timing.
	 *
	 *
	 * @param target Screen position all snoids walk toward (typically off-screen).
	 * @param stagger Frames between each successive snoid starting its walk (default 45).
	 */
	void startDepartWalkAnimation(const Common::Point &target, uint32 stagger = 45);

	/**
	 * Play the departure SFX and track its handle for completion polling.
	 * @param systemSoundId  System resource ID of the departure SFX (default 996).
	 */
	void playDepartSfx(int16 systemSoundId = kSysResSound0996_DepartSFX);

	/**
	 * Check whether the departure SFX has finished playing.
	 * Returns true immediately if no SFX handle was captured.
	 */
	bool isDepartSfxDone() const;

	/** Sound handle of the departure SFX, for completion polling. */
	/** Mixer handle of the common departure sound. */
	Audio::SoundHandle _departSfxHandle;
	/** Whether @ref _departSfxHandle contains a valid departure sound. */
	bool _hasDepartSfxHandle = false;

	/** Called when the Map button is activated. */
	virtual void onMapButtonActivated();
	/** Optional per-page confirmation before transitioning to RodMap. */
	virtual bool confirmMapTransition();
	/** Optional per-page save-state cleanup before transitioning to RodMap. */
	virtual void saveStateBeforeMapTransition();

	// [*] Snoid drag-and-drop infrastructure
	// Common state and helpers shared by all interactive pages.

	/** The snoid currently being dragged, or nullptr. */
	ZmbSnoid *_draggedSnoid = nullptr;

	/** Drag offset from snoid origin to mouse click point. */
	Common::Point _dragOffset;

	/** Initial Snoid position before dragging started. */
	Common::Point _dragOrigPos;

	/** Previous mouse X position during drag, for computing movement direction. */
	int16 _dragPrevMouseX = 0;

	/**
	 * Full fixed-resolution screen rectangle used as the default drag limit.
	 * It is instance-owned because Common::Rect requires runtime construction
	 * and ScummVM prohibits global C++ constructors.
	 */
	const Common::Rect _defaultDragConstraint = Common::Rect(0, 0, 639, 479);

	/** Whether a drag is currently active. */
	bool isDragging() const { return _draggedSnoid != nullptr; }

	/**
	 * Find the frontmost bare Snoid under the cursor point.
	 * The default implementation walks @ref ZoombiniPage::_runnerOrder from front to back
	 * and tests runners whose low feature flags contain only the Snoid type.
	 * It uses each matching runner's aggregate click rectangle.
	 * Pages can override this method to add filtering.
	 */
	virtual ZmbSnoid *findSnoidAtPoint(const Common::Point &pos);
	/**
	 * Find the frontmost Snoid in an exact low-feature-flag class.
	 * This preserves generic runner occlusion: a matching non-Snoid runner blocks
	 * Snoids behind it instead of being skipped by a Snoid-only container scan.
	 */
	ZmbSnoid *findSnoidAtPoint(const Common::Point &pos, uint32 requiredFlags);

	/**
	 * Begin a Snoid drag operation.
	 * Stores the dragged Snoid, initial position, and cursor offset,
	 * then calls @ref ZoombiniPage::beginSnoidDrag() to set the drag animation and hide the cursor.
	 */
	void startSnoidDrag(ZmbSnoid *snoid, const Common::Point &mousePos);

	/**
	 * End a Snoid drag operation.
	 * Clears the current dragged Snoid and calls @ref ZoombiniPage::endSnoidDrag() to restore the cursor.
	 * Returns the formerly dragged Snoid for drop evaluation.
	 */
	ZmbSnoid *finishSnoidDrag();
	/** Return whether the active drag keeps the Snoid at its pickup position. */
	virtual bool isSnoidDragPositionFixed() const { return false; }

	/**
	 * Return the screen-space constraint rect for drag movement.
	 * Default: full screen (0,0,639,479). Pages override for smaller areas
	 * (e.g. Bridge uses left bank only).
	 */
	virtual const Common::Rect &getDragConstraintRect() const;

	// [*] Draw-on-Region Drop-Target Interaction

	/**
	 * Click zone radius for draw-on-reg drop detection.
	 * Page initialization may override the default value of 15.
	 */
	int16 _clickZoneRadius = 15;

	/** First draw-on-reg slot inside the Snoid pickup-root square, or -1. */
	int16 _dragSourceDrawOnRegSlot = -1;

	/** Currently highlighted draw-on-reg slot during drag, or -1. */
	int16 _dragHighlightSlot = -1;

	/**
	 * Update draw-on-reg seat highlighting during drag.
	 * @ref ZoombiniInteractive::onMouseMove() calls this method
	 * to highlight empty slots within the zone radius and clear the highlight when leaving.
	 */
	void updateDrawOnRegHighlight();

	/** Clear any active draw-on-reg highlight (e.g. on drag end). */
	void clearDrawOnRegHighlight();

	/**
	 * In More Action mode, keep the first 75% of the loaded pack idle at their pedestal positions.
	 * Give the last 25% a walk-in animation from x=-50 and the destination Y plus @p initialYOffset.
	 * Less Action mode or a page-specific transient gate keeps every snoid static.
	 */
	void layoutStaticAndWalkIn(int16 initialYOffset, bool suppressWalkIn);

	/**
	 * Sort walk-in snoids by destination X and assign staggered next render deadline values
	 * so they enter the scene sequentially.
	 */
	void assignStaggeredWalkDelays(uint32 startOffset, uint32 stagger);

	/** Return whether @p pos lies inside any configured navigation or help button. */
	bool isPointInControlButtonRect(const Common::Point &pos) const;

private:
	/**
	 * Ambient system-sound pools used by @ref ZoombiniInteractive::runAmbientSoundDriver().
	 * Each pool is selected by the corresponding interactive page type.
	 */
	static constexpr int16 kAmbientPoolBC1[15] = {
		ZoombiniPage::kSysResSound0924_Ambient, ZoombiniPage::kSysResSound0933_Ambient,
		ZoombiniPage::kSysResSound0904_Ambient, ZoombiniPage::kSysResSound0905_Ambient,
		ZoombiniPage::kSysResSound0906_Ambient, ZoombiniPage::kSysResSound0925_Ambient,
		ZoombiniPage::kSysResSound0926_Ambient, ZoombiniPage::kSysResSound0927_Ambient,
		ZoombiniPage::kSysResSound0928_Ambient, ZoombiniPage::kSysResSound0929_Ambient,
		ZoombiniPage::kSysResSound0917_Ambient, ZoombiniPage::kSysResSound0918_Ambient,
		ZoombiniPage::kSysResSound0919_Ambient, ZoombiniPage::kSysResSound0920_Ambient,
		ZoombiniPage::kSysResSound0936_Ambient};
	static constexpr int16 kAmbientPoolBC2[10] = {
		ZoombiniPage::kSysResSound0924_Ambient, ZoombiniPage::kSysResSound0904_Ambient,
		ZoombiniPage::kSysResSound0905_Ambient, ZoombiniPage::kSysResSound0906_Ambient,
		ZoombiniPage::kSysResSound0933_Ambient, ZoombiniPage::kSysResSound0936_Ambient,
		ZoombiniPage::kSysResSound0917_Ambient, ZoombiniPage::kSysResSound0918_Ambient,
		ZoombiniPage::kSysResSound0919_Ambient, ZoombiniPage::kSysResSound0920_Ambient};
	static constexpr int16 kAmbientPoolBridge[9] = {
		ZoombiniPage::kSysResSound0924_Ambient, ZoombiniPage::kSysResSound0933_Ambient,
		ZoombiniPage::kSysResSound0904_Ambient, ZoombiniPage::kSysResSound0905_Ambient,
		ZoombiniPage::kSysResSound0906_Ambient, ZoombiniPage::kSysResSound0942_Ambient,
		ZoombiniPage::kSysResSound0943_Ambient, ZoombiniPage::kSysResSound0944_Ambient,
		ZoombiniPage::kSysResSound0902_Ambient};
	static constexpr int16 kAmbientPoolTunnels[9] = {
		ZoombiniPage::kSysResSound0911_Ambient, ZoombiniPage::kSysResSound0914_Ambient,
		ZoombiniPage::kSysResSound0915_Ambient, ZoombiniPage::kSysResSound0942_Ambient,
		ZoombiniPage::kSysResSound0943_Ambient, ZoombiniPage::kSysResSound0944_Ambient,
		ZoombiniPage::kSysResSound0904_Ambient, ZoombiniPage::kSysResSound0933_Ambient,
		ZoombiniPage::kSysResSound0934_Ambient};
	static constexpr int16 kAmbientPoolPizza[12] = {
		ZoombiniPage::kSysResSound0940_Ambient, ZoombiniPage::kSysResSound0941_Ambient,
		ZoombiniPage::kSysResSound0943_Ambient, ZoombiniPage::kSysResSound0921_Ambient,
		ZoombiniPage::kSysResSound0917_Ambient, ZoombiniPage::kSysResSound0918_Ambient,
		ZoombiniPage::kSysResSound0919_Ambient, ZoombiniPage::kSysResSound0920_Ambient,
		ZoombiniPage::kSysResSound0905_Ambient, ZoombiniPage::kSysResSound0911_Ambient,
		ZoombiniPage::kSysResSound0912_Ambient, ZoombiniPage::kSysResSound0916_Ambient};
	static constexpr int16 kAmbientPoolFerry[19] = {
		ZoombiniPage::kSysResSound0924_Ambient, ZoombiniPage::kSysResSound0933_Ambient,
		ZoombiniPage::kSysResSound0904_Ambient, ZoombiniPage::kSysResSound0905_Ambient,
		ZoombiniPage::kSysResSound0906_Ambient, ZoombiniPage::kSysResSound0937_Ambient,
		ZoombiniPage::kSysResSound0938_Ambient, ZoombiniPage::kSysResSound0939_Ambient,
		ZoombiniPage::kSysResSound0902_Ambient, ZoombiniPage::kSysResSound0903_Ambient,
		ZoombiniPage::kSysResSound0925_Ambient, ZoombiniPage::kSysResSound0926_Ambient,
		ZoombiniPage::kSysResSound0927_Ambient, ZoombiniPage::kSysResSound0928_Ambient,
		ZoombiniPage::kSysResSound0929_Ambient, ZoombiniPage::kSysResSound0917_Ambient,
		ZoombiniPage::kSysResSound0918_Ambient, ZoombiniPage::kSysResSound0919_Ambient,
		ZoombiniPage::kSysResSound0920_Ambient};
	static constexpr int16 kAmbientPoolLilly[20] = {
		ZoombiniPage::kSysResSound0930_Ambient, ZoombiniPage::kSysResSound0931_Ambient,
		ZoombiniPage::kSysResSound0932_Ambient, ZoombiniPage::kSysResSound0937_Ambient,
		ZoombiniPage::kSysResSound0938_Ambient, ZoombiniPage::kSysResSound0939_Ambient,
		ZoombiniPage::kSysResSound0925_Ambient, ZoombiniPage::kSysResSound0926_Ambient,
		ZoombiniPage::kSysResSound0927_Ambient, ZoombiniPage::kSysResSound0928_Ambient,
		ZoombiniPage::kSysResSound0929_Ambient, ZoombiniPage::kSysResSound0917_Ambient,
		ZoombiniPage::kSysResSound0918_Ambient, ZoombiniPage::kSysResSound0919_Ambient,
		ZoombiniPage::kSysResSound0920_Ambient, ZoombiniPage::kSysResSound0904_Ambient,
		ZoombiniPage::kSysResSound0905_Ambient, ZoombiniPage::kSysResSound0906_Ambient,
		ZoombiniPage::kSysResSound0924_Ambient, ZoombiniPage::kSysResSound0933_Ambient};
	static constexpr int16 kAmbientPoolSlides[13] = {
		ZoombiniPage::kSysResSound0911_Ambient, ZoombiniPage::kSysResSound0912_Ambient,
		ZoombiniPage::kSysResSound0913_Ambient, ZoombiniPage::kSysResSound0914_Ambient,
		ZoombiniPage::kSysResSound0921_Ambient, ZoombiniPage::kSysResSound0936_Ambient,
		ZoombiniPage::kSysResSound0904_Ambient, ZoombiniPage::kSysResSound0905_Ambient,
		ZoombiniPage::kSysResSound0906_Ambient, ZoombiniPage::kSysResSound0917_Ambient,
		ZoombiniPage::kSysResSound0918_Ambient, ZoombiniPage::kSysResSound0919_Ambient,
		ZoombiniPage::kSysResSound0920_Ambient};
	static constexpr int16 kAmbientPoolFleens[13] = {
		ZoombiniPage::kSysResSound0911_Ambient, ZoombiniPage::kSysResSound0912_Ambient,
		ZoombiniPage::kSysResSound0913_Ambient, ZoombiniPage::kSysResSound0914_Ambient,
		ZoombiniPage::kSysResSound0921_Ambient, ZoombiniPage::kSysResSound0936_Ambient,
		ZoombiniPage::kSysResSound0904_Ambient, ZoombiniPage::kSysResSound0905_Ambient,
		ZoombiniPage::kSysResSound0906_Ambient, ZoombiniPage::kSysResSound0917_Ambient,
		ZoombiniPage::kSysResSound0918_Ambient, ZoombiniPage::kSysResSound0919_Ambient,
		ZoombiniPage::kSysResSound0920_Ambient};
	static constexpr int16 kAmbientPoolNet[17] = {
		ZoombiniPage::kSysResSound0924_Ambient, ZoombiniPage::kSysResSound0933_Ambient,
		ZoombiniPage::kSysResSound0904_Ambient, ZoombiniPage::kSysResSound0905_Ambient,
		ZoombiniPage::kSysResSound0906_Ambient, ZoombiniPage::kSysResSound0931_Ambient,
		ZoombiniPage::kSysResSound0932_Ambient, ZoombiniPage::kSysResSound0938_Ambient,
		ZoombiniPage::kSysResSound0925_Ambient, ZoombiniPage::kSysResSound0926_Ambient,
		ZoombiniPage::kSysResSound0927_Ambient, ZoombiniPage::kSysResSound0928_Ambient,
		ZoombiniPage::kSysResSound0929_Ambient, ZoombiniPage::kSysResSound0917_Ambient,
		ZoombiniPage::kSysResSound0918_Ambient, ZoombiniPage::kSysResSound0919_Ambient,
		ZoombiniPage::kSysResSound0920_Ambient};
	static constexpr int16 kAmbientPoolCaves[10] = {
		ZoombiniPage::kSysResSound0922_Ambient, ZoombiniPage::kSysResSound0923_Ambient,
		ZoombiniPage::kSysResSound0935_Ambient, ZoombiniPage::kSysResSound0907_Ambient,
		ZoombiniPage::kSysResSound0908_Ambient, ZoombiniPage::kSysResSound0909_Ambient,
		ZoombiniPage::kSysResSound0900_Ambient, ZoombiniPage::kSysResSound0901_Ambient,
		ZoombiniPage::kSysResSound0934_Ambient, ZoombiniPage::kSysResSound0910_Ambient};
	static constexpr int16 kAmbientPoolSmoke[10] = {
		ZoombiniPage::kSysResSound0922_Ambient, ZoombiniPage::kSysResSound0923_Ambient,
		ZoombiniPage::kSysResSound0935_Ambient, ZoombiniPage::kSysResSound0907_Ambient,
		ZoombiniPage::kSysResSound0908_Ambient, ZoombiniPage::kSysResSound0909_Ambient,
		ZoombiniPage::kSysResSound0900_Ambient, ZoombiniPage::kSysResSound0901_Ambient,
		ZoombiniPage::kSysResSound0934_Ambient, ZoombiniPage::kSysResSound0910_Ambient};
	static constexpr int16 kAmbientPoolMaze[10] = {
		ZoombiniPage::kSysResSound0922_Ambient, ZoombiniPage::kSysResSound0923_Ambient,
		ZoombiniPage::kSysResSound0935_Ambient, ZoombiniPage::kSysResSound0907_Ambient,
		ZoombiniPage::kSysResSound0908_Ambient, ZoombiniPage::kSysResSound0909_Ambient,
		ZoombiniPage::kSysResSound0900_Ambient, ZoombiniPage::kSysResSound0901_Ambient,
		ZoombiniPage::kSysResSound0934_Ambient, ZoombiniPage::kSysResSound0910_Ambient};
	/** Schedule and play ambient sound resources during idle page frames. */
	void runAmbientSoundDriver();
	/** Frame counter when next ambient sound is due */
	uint32 _ambientNextPlayFrame = 0;
	/** Resource ID for Go/Map buttons bitmap */
	int16 _goMapBitmapResId = 0;
	/** Whether the active release stores each Go/Map shape in a separate bitmap. */
	bool _goMapButtonsUseSeparateBitmapResources = false;

	/** Hit rectangle for the primary Go button. */
	Common::Rect _goButtonRect;
	/** Disabled primary Go shape. */
	uint16 _goButtonShapeDisabledId = ZmbHotspot::kShapeNone;
	/** Enabled primary Go shape. */
	uint16 _goButtonShapeEnabledId = ZmbHotspot::kShapeNone;
	/** Pressed primary Go shape. */
	uint16 _goButtonShapePressedId = ZmbHotspot::kShapeNone;

	/** Hit rectangle for the secondary Go button. */
	Common::Rect _secondGoButtonRect;
	/** Disabled secondary Go shape. */
	uint16 _secondGoButtonShapeDisabledId = ZmbHotspot::kShapeNone;
	/** Enabled secondary Go shape. */
	uint16 _secondGoButtonShapeEnabledId = ZmbHotspot::kShapeNone;
	/** Pressed secondary Go shape. */
	uint16 _secondGoButtonShapePressedId = ZmbHotspot::kShapeNone;

	/** Hit rectangle for the Map button. */
	Common::Rect _mapButtonRect;
	/** Normal Map shape. */
	uint16 _mapButtonShapeNormalId = ZmbHotspot::kShapeNone;
	/** Pressed Map shape. */
	uint16 _mapButtonShapePressedId = ZmbHotspot::kShapeNone;

	/** Hit rectangle for the Help button. */
	Common::Rect _helpButtonRect;

	/** Per-button Go/Map visual and interaction state. */
	Common::StableMap<uint32, ButtonState> _goMapButtonStateMap;
	/** Per-button Help visual and interaction state. */
	Common::StableMap<uint32, ButtonState> _helpButtonStateMap;
	/** Explicit hit rectangles for releases with separate button bitmaps. */
	Common::HashMap<uint32, Common::Rect> _threeButtonRectMap;
	/** Materialized Go/Map hotspot set. */
	Common::Array<ZmbHotspot> _goMapButtonHotspots;
	/** Shared Go/Map feature runner. */
	ZmbFeature *_goMapButtonsFeature = nullptr;
	/** Shared Help feature runner. */
	ZmbFeature *_helpButtonFeature = nullptr;

	/** Short notification-box rectangle. */
	const Common::Rect _notiBoxShortRect = Common::Rect(0x0115, 0x01CA, 0x016C, 0x01DD);
	/** Long notification-box rectangle. */
	const Common::Rect _notiBoxLongRect = Common::Rect(0x0101, 0x01CA, 0x0183, 0x01DD);
	/** Frame at which the current notification expires. */
	uint32 _notiBoxShowUntilFrame = 0;
	/** Text currently displayed in the notification box. */
	Common::U32String _notiBoxText;
	/** Whether the current notification uses the long layout. */
	bool _isNotiBoxLong = false;
	/** Notification-box feature runner. */
	ZmbFeature *_notiBoxFeature = nullptr;

protected:
	// [*] Active Help Sound (F1 key replay)
	// Help voice resource replayed when F1 is pressed.
	/** Help voice resource replayed by the F1 shortcut. */
	ZmbResource _activeHelpSoundId;

private:
	/** Replay the current page's help voice when help audio is enabled. */
	void playActiveHelpSound();
};

} // End of namespace Mohawk

#endif
