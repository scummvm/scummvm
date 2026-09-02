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

#ifndef MOHAWK_ZOOMBINI_SCRIPTS_H
#define MOHAWK_ZOOMBINI_SCRIPTS_H

#include "audio/mixer.h"

#include "common/array.h"
#include "common/ptr.h"
#include "common/rect.h"
#include "common/stablemap.h"
#include "common/stack.h"
#include "common/substream.h"

#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_resource.h"
#include "mohawk/zoombini_state.h"

namespace Common {

class KeyState;

} // End of namespace Common

namespace Mohawk {

class MohawkEngine_Zoombini;
class ZoombiniPage;

/**
 * Represents NODE resource in .MHK archive, which defines a set of waypoints.
 * Zoombinis can move along the waypoints defined in a NODE resource.
 *
 * Structure:
 * - (UINT16_BE) number of points (times 4 = size of following data)
 * - (INT16_BE, INT16_BE) * numPoints: array of points (x, y)
 */
class ZmbNode {
public:
	/** Waypoint coordinates decoded from the NODE resource. */
	Common::Array<Common::Point> _waypoints;

	/**
	 * Paths from the companion PATH resource (same resource ID, different tag).
	 * Each path is up to 24 entries of 1-indexed waypoint indices (0 = empty slot).
	 * Paths define safe routes between waypoints; selects the path containing the destination-nearest waypoint,
	 * then walks the selected path.
	 */
	Common::Array<Common::Array<uint8>> _paths;

	/** Decode waypoint records from a NODE stream. */
	bool parseStream(Common::SeekableReadStream *stream);
	/** Decode path waypoint indices from a PATH stream. */
	bool parsePathStream(Common::SeekableReadStream *stream);
};

/**
 * Reader for the paired X/Y registration-table form of REGS.
 *
 * ID_REGS does not represenet a universal payload schema.
 * This class handles the common Zoombini REGS form consisting of two resources,
 * each containing a big-endian signed 16-bit axis table.
 * The first resource supplies X values and the second supplies Y values.
 * The usual page convention is consecutive resource IDs: base for X and base + 1 for Y.
 *
 * The first value is retained at offset index 0 as a reserved shape slot.
 * Shape lookups are one-based, while sub-image lookups are zero-based and add one
 * before indexing the stored table.
 * Callers subtract the resulting registration delta from the target position
 * when aligning a shape or hotspot.
 *
 * This reader must not be used for other REGS schemas.
 * For example, Maze 16600-series resources contain launcher headers and logical
 * cell records, while Maze 16000 contains packed screen points.
 * Maze 16501, 17000/17001, and 18000/18001 are also decoded by Maze-specific readers.
 */
class ZmbShapeOffsetRegs {
public:
	/** Registration deltas indexed by the one-based shape index. Index 0 is reserved. */
	Common::Array<Common::Point> _offsets;

	/** Load paired X and Y REGS axis resources into registration offsets. */
	void parseStreams(MohawkEngine_Zoombini *vm, ZmbResource::ArchiveKind archiveKind, int16 resIdX, int16 resIdY);

	/** Return the registration delta for a zero-based sub-image. */
	Common::Point getSubImageDelta(uint16 subImage) const;
	/** Return the registration delta for a one-based shape. */
	Common::Point getShapeDelta(uint16 shapeIdx) const;
	/** Return the registration delta for a hotspot's shape convention. */
	Common::Point getHotspotDelta(const ZmbHotspot &hotspot) const;

private:
	/**
	 * Decode one REGS axis stream into signed offsets.
	 *
	 * This method takes ownership of @p stream and deletes it after decoding.
	 */
	Common::Array<int16> parseStream(Common::SeekableReadStream *stream);
};

/** Represents a hotspot in a feature script (SCRB). */
struct ZmbHotspot {
public:
	enum ReservedId : uint16 {
		kIndexNone = UINT16_MAX,
		kShapeNone = 0,
		kLengthAuto = 0,
		kDrawnRectVirtual = UINT16_MAX,
	};
	/** 0-based index of the hotspot. */
	uint16 _hsId = kIndexNone;
	/** 1-based index of the shape. */
	int16 _shapeIdx = kShapeNone;
	/** 0-based index of the frame. */
	uint16 _frame = 0;
	/** X position of the hotspot. */
	int16 _x = 0;
	/** Y position of the hotspot. */
	int16 _y = 0;

	ZmbHotspot() {}
	/**
	 * Represents a physical hotspot entry from .MHK archive.
	 * @param hsId 0-based index
	 * @param shapeid 1-based shape index
	 * @param frame 0-based frame index
	 * @param x X position
	 * @param y Y position
	 */
	ZmbHotspot(uint16 hsId, int16 shapeid, uint16 frame, int16 x, int16 y) : _hsId(hsId), _shapeIdx(shapeid), _frame(frame), _x(x), _y(y) {}
	/**
	 * Represents a virtual hotspot entry from MapRects.
	 * @param hsId 0-based index
	 * @param shapeid 1-based shape index
	 * @param frame 0-based frame index
	 * @param rect Rectangle defining the hotspot area
	 */
	ZmbHotspot(uint16 hsId, int16 shapeid, uint16 frame, const Common::Rect &rect) : _hsId(hsId), _shapeIdx(shapeid), _frame(frame), _x(rect.left), _y(rect.top) {}

	/**
	 * Convert the one-based @ref ZmbHotspot::_shapeIdx to a zero-based subimage ID.
	 * @return 0-based subImage id
	 */
	uint16 getSubImageId() const { return _shapeIdx - 1; }

	/**
	 * Get the position of the hotspot.
	 * @return Position of the hotspot
	 */
	Common::Point getPos() const { return Common::Point(_x, _y); }

	/**
	 * Compute hash function to uniquely identify a hotspot in a Feature.
	 * @return uint32 hash value
	 */
	uint32 hash() const { return hash(_frame, _hsId); }

	/**
	 * Compute hash function to uniquely identify a hotspot in a Feature.
	 * @return uint32 hash value
	 */
	static uint32 hash(uint16 frame, uint16 hsIdx) { return (frame << 16) + hsIdx; }
};

/** Decoded contents of one SCRB or SCRS frame before runtime materialization. */
struct ZmbDecodedScriptFrame {
	/** All nonnegative shape entries in their authored order. */
	Common::Array<ZmbHotspot> hotspots;
	/** Raw sound resource ID attached to the frame terminator. */
	int16 soundResId = 0;
	/** Whether the frame terminator carried a sound resource ID. */
	bool hasSoundRes = false;
	/** Nonzero event code attached to the frame terminator. */
	uint8 eventCode = 0;
};

/** Decoder for the shared SCRB/SCRS frame stream format. */
class ZmbScriptDecoder {
public:
	/** Trait-layer ordering encoded in the second SCRS header word. */
	enum class TraitLayout : int16 {
		/** Reserved placeholder layout encoded as 0xFFFF (-1), with no layer mapping. */
		kInvalid = -1,
		/** Layer slots: feet, body, nose, eye, hair. */
		kSlotOrder00 = 0,
		/** Layer slots: feet, nose, body, eye, hair. */
		kSlotOrder01 = 1,
		/** Layer slots: body, eye, nose, feet, hair. */
		kSlotOrder02 = 2,
		/** Layer slots: body, feet, nose, eye, hair; used by the Fleens renderer. */
		kSlotOrder03 = 3,
	};

	/** Immutable decoded contents of one complete SCRB resource. */
	struct DecodedScrb {
		/** Frames in their authored order, including event-only frames. */
		Common::Array<ZmbDecodedScriptFrame> frames;
	};

	/** Immutable decoded contents of one complete SCRS resource. */
	struct DecodedScrs {
		/** Frames in their authored order, including event-only frames. */
		Common::Array<ZmbDecodedScriptFrame> frames;
		/** Trait-layer ordering read from the SCRS header. */
		TraitLayout traitLayout = TraitLayout::kSlotOrder00;
	};

	/**
	 * Resolve and decode an SCRB resource.
	 * Stream creation and lifetime remain internal to the decoder.
	 * @param decodedScrb Receives the complete immutable decoded representation.
	 * @return Whether the complete resource was valid.
	 */
	static bool decodeScrbResource(MohawkEngine_Zoombini *vm, ZmbResource resource, DecodedScrb &decodedScrb);
	/**
	 * Resolve and decode an SCRS resource.
	 * Stream creation and lifetime remain internal to the decoder.
	 * @param decodedScrs Receives the complete immutable decoded representation.
	 * @return Whether the complete resource was valid.
	 */
	static bool decodeScrsResource(MohawkEngine_Zoombini *vm, ZmbResource resource, DecodedScrs &decodedScrs);
	/**
	 * Decode frame records after an SCRB or SCRS header.
	 * The caller retains ownership of @p stream.
	 * @param resourceId Resource ID used in malformed-stream diagnostics.
	 * @param resourceType Four-character resource type used in diagnostics.
	 */
	static bool decodeScriptFrames(Common::SeekableReadStream *stream, uint16 frameCount, int16 resourceId,
								   const char *resourceType, Common::Array<ZmbDecodedScriptFrame> &frames);

private:
	/** Decode an already-open SCRB stream. The caller retains ownership of @p stream. */
	static bool decodeScrbStream(Common::SeekableReadStream *stream, int16 resourceId, DecodedScrb &decodedScrb);
	/** Decode an already-open SCRS stream. The caller retains ownership of @p stream. */
	static bool decodeScrsStream(Common::SeekableReadStream *stream, int16 resourceId, DecodedScrs &decodedScrs);
};

/** Mutable hotspots materialized for one cached script frame or owned by one virtual feature. */
struct ZmbHotspotGroup {
public:
	/** Stable feature runner ID that owns this materialized group. */
	uint16 _runnerId = 0;
	/** Frame index. 0-based, -1 means no frame is selected. */
	int32 _frameIdx = -1;

	ZmbHotspotGroup(uint16 runnerId, int32 frameIdx) : _runnerId(runnerId), _frameIdx(frameIdx) {}
	/** Release draw records and hotspot-owned resources. */
	~ZmbHotspotGroup();

	/**
	 * Get a snapshot copy of all hotspots in this group.
	 * @return Array of hotspots
	 */
	Common::Array<ZmbHotspot> copyHotspots() { return _hotspots; }
	/**
	 * Get the number of shapes in this group.
	 * @return Number of hotspots
	 */
	uint32 getHotspotCount() const { return _hotspots.size(); }
	/**
	 * Get a hotspot by its 0-based index.
	 * @param hsId 0-based index of the hotspot
	 * @return The hotspot at the given index
	 */
	ZmbHotspot &getHotspot(uint32 hsId);
	/**
	 * Get a hotspot by its 0-based index.
	 * @param hsId 0-based index of the hotspot
	 * @return The hotspot at the given index
	 */
	ZmbHotspot &operator[](uint32 hsId);
	/**
	 * Append a hotspot to the group.
	 * @param hs The hotspot to append
	 */
	void appendHotspot(const ZmbHotspot &hs);
	/**
	 * Set hotspots to the group.
	 * @param hotspots Array of hotspots to set
	 */
	void setHotspots(const Common::Array<ZmbHotspot> &hotspots);
	/** Clear all hotspots from the group. */
	void clear();
	/** Clear the embedded sound and event copied from a decoded script frame. */
	void clearScriptMetadata();

	/** Store the sound resource attached to the group's terminator. */
	void assignSoundRes(ZmbResource soundRes) { _soundRes = soundRes; }
	bool hasAssignedSoundRes() const { return _soundRes.hasId(); }
	ZmbResource getAssignedSoundRes() const { return _soundRes; }

	/** Store the nonzero event byte attached to the group's terminator. */
	void assignEventCode(uint8 eventCode) { _eventCode = eventCode; }
	bool hasAssignedEventCode() const { return _eventCode != 0; }
	uint8 getAssignedEventCode() const { return _eventCode; }

	// Iterator
	typedef Common::Array<ZmbHotspot>::iterator ArrayIterator;
	typedef Common::Array<ZmbHotspot>::const_iterator ConstArrayIterator;
	ArrayIterator begin() { return _hotspots.begin(); }
	ArrayIterator end() { return _hotspots.end(); }
	ConstArrayIterator begin() const { return _hotspots.begin(); }
	ConstArrayIterator end() const { return _hotspots.end(); }

private:
	/** Hotspots belonging to this SCRB frame group. */
	Common::Array<ZmbHotspot> _hotspots;
	/** Sound resource attached to the group terminator. */
	ZmbResource _soundRes;
	/** Nonzero event code attached to the group terminator. */
	uint8 _eventCode = 0;
};

class ZmbFeature;
class ZmbDrawRecord {
public:
	/** Feature that produced this draw record. */
	ZmbFeature *_scrb = nullptr;
	/** Hotspot group that contains the drawn hotspot. */
	ZmbHotspotGroup *_hsGroup = nullptr;
	/** Hotspot data used for this draw record. */
	ZmbHotspot _hs;
	/** Screen rectangle occupied by the drawn shape. */
	Common::Rect _drawnRect;

	ZmbDrawRecord() {}
	ZmbDrawRecord(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, const ZmbHotspot &hs, const Common::Rect &drawnRect) : _scrb(feature), _hsGroup(hsGroup), _hs(hs), _drawnRect(drawnRect) {}

	/** Return whether the record no longer refers to a live feature/group. */
	bool isEmpty() { return !_scrb || !_hsGroup; }
	/** Return whether the record represents a virtual hit-test rectangle. */
	bool isVirtualZone() { return _hs._frame == ZmbHotspot::kDrawnRectVirtual; }
};

struct ZmbPreparedRenderHotspot {
	/** Transformed hotspot consumed by the default post-render path. */
	ZmbHotspot _hotspot;
	/** Image resource from which the transformed shape is drawn. */
	ZmbResource _resource;
	/** Weak source-group pointer. A retained reload frame detaches it before releasing the group. */
	ZmbHotspotGroup *_hsGroup = nullptr;
};

/**
 * Runtime feature runner and shared decoded-script backing.
 *
 * A feature keeps a stable runner identity while its current script, frame
 * groups, materialized draw records, event hooks, and sound policy change over
 * time. @ref ZoombiniPage owns the active registration and calls the feature's
 * pre-render and post-render callbacks; a feature owns any sub-feature chain it
 * creates, even when a child is temporarily registered in the page's active map.
 *
 * SCRB and SCRS share the same immutable decoded-frame backing and frame
 * materialization path. The semantic owner supplies resource-specific playback:
 * @ref ZmbSnoid owns Zoombini SCRS state, while puzzle pages may bind SCRS data
 * to a generic feature for custom actors. Loading a new SCRB onto an existing
 * feature replaces script data but preserves the runner identity, position,
 * hooks, and sound policy used by the page.
 */
class ZmbFeature {
public:
	/**
	 * Select how this runner dispatches embedded SCRB/SCRS sounds.
	 *
	 * Page-wide script sounds can share one render-pass priority queue where authored effects depend on arbitration.
	 * Unrelated feedback sounds may be dispatched immediately.
	 * Snoid runners use the priority queue by default so consecutive movement sounds replace earlier tails.
	 */
	enum class ScriptSoundPolicy : uint8 {
		/** Follow @ref ZoombiniPage::usesOriginalScriptSoundQueue(). */
		kInheritPage,
		/** Bypass page arbitration and start this runner's sounds immediately. */
		kImmediate,
		/** Add this runner's sounds to the render-pass priority arbitration. */
		kPriorityQueue,
		/** Add this runner's sounds with priority above every authored range. */
		kForcedPriorityQueue
	};

	enum Flag : uint32 {
		/**
		 * Simple static shapes.
		 * Zero type bits select a generic feature runner.
		 * @remarks Mutually exclusive with FLAG_00000001_TYPE_SNOID and FLAG_00000002_TYPE_GRIDWALKER.
		 */
		FLAG_00000000_TYPE_SHAPES = 0x00000000,
		/**
		 * Zoombini entity.
		 * In render sorting, goes into a separate entity render list.
		 * @remarks Mutually exclusive with FLAG_00000000_TYPE_SHAPES and FLAG_00000002_TYPE_GRIDWALKER.
		 */
		FLAG_00000001_TYPE_SNOID = 0x00000001,
		/**
		 * Grid-walker registration and runtime category bit.
		 *
		 * The flag indicates the use of @ref ZmbLillyGridWalker, but in effect only lilly uses that.
		 * The loader and owning page decide whether a runner is a @ref ZmbSnoid,
		 * a generic @ref ZmbFeature, or has separate @ref ZmbLillyGridWalker state.
		 *
		 * Sorted into the same render list as SNOID entities when it is the only flag set.
		 * @remarks Mutually exclusive with FLAG_00000000_TYPE_SHAPES and FLAG_00000001_TYPE_SNOID.
		 */
		FLAG_00000002_TYPE_GRIDWALKER = 0x00000002,
		/**
		 * Renders the feature last (topmost) in the render order.
		 * Features with this flag are appended to the end of the sorted render list.
		 * Also prevents automatic OVERLAY (0x4000000) force-set during render sorting.
		 * Used for buttons, dialogs, notification boxes, and page picker UI elements.
		 */
		FLAG_00001000_TOPMOST = 0x00001000,
		/**
		 * On registration, immediately triggers pre-render and records position.
		 * When CHAIN_SCRIPT chain completes, disables rendering.
		 */
		FLAG_00002000_DRAW_ON_REG = 0x00002000,
		/** When NOT set, getDrawnRect is called before rendering to allow rect invalidation. */
		FLAG_00004000_NO_DIRTY_MERGE = 0x00004000,
		/**
		 * Loop animation shapes continuously.
		 * In the render pipeline, features with this flag stay in the animation runner list
		 * and are NOT sorted into the normal/overlay/entity render lists.
		 */
		FLAG_00008000_LOOP_ANIM = 0x00008000,
		/**
		 * One-time render skip. Skips rendering once, then auto-clears itself.
		 * Resets frame index to 0 and hotspot index to 1 when triggered.
		 */
		FLAG_00010000_SKIP_ONCE = 0x00010000,
		/** Persistent render skip. Sets doRender = false without clearing the flag. */
		FLAG_00020000_SKIP_RENDER = 0x00020000,
		/**
		 * Reference other SCRB. Stores otherScriptId for chaining.
		 * At end-of-animation, loads the chained SCRB.
		 */
		FLAG_00040000_CHAIN_SCRIPT = 0x00040000,
		/**
		 * Animate the feature after some event is toggled.
		 * On initial load, rendering is disabled. Mainly used in easter eggs.
		 */
		FLAG_00080000_DEFER_ANIM = 0x00080000,
		/**
		 * Play-once animation. At end-of-animation, clears hotspot/shape data,
		 * and fires onHotspotShapeOrFrameFunc(-1) callback.
		 */
		FLAG_00100000_PLAY_ONCE = 0x00100000,
		/**
		 * Position delta mode.
		 * Copies hotspot pos to secondary position on load,
		 * then computes delta = feature position - secondary position and applies to all hotspot positions.
		 */
		FLAG_00800000_POS_DELTA = 0x00800000,
		/**
		 * Render the feature after some event is toggled.
		 * On initial load, rendering is disabled.
		 */
		FLAG_01000000_DEFER_RENDER = 0x01000000,
		/**
		 * Select random frame when rendering.
		 * If combined with CHAIN_SCRIPT, clears RANDOM_FRAME and negates script ID on load.
		 */
		FLAG_02000000_RANDOM_FRAME = 0x02000000,
		/**
		 * Draw to the overlay screen instead of the shape screen.
		 * In render sorting, features with this flag go into the overlay render list.
		 * Automatically force-set on normal features that lack FLAG_00001000_TOPMOST.
		 */
		FLAG_04000000_OVERLAY = 0x04000000,
		/** Allocates an RgnR RMap instance (size 0x90) for per-shape bounding rect tracking. */
		FLAG_08000000_REGION_TRACK = 0x08000000,
		/**
		 * Z-order sort protection: right edge.
		 * Prevents a feature from being sorted in front of existing features
		 * that extend further right. Features with this flag are appended to end
		 * of the render list and are not auto-assigned OVERLAY.
		 */
		FLAG_10000000_ZSORT_RIGHT = 0x10000000,
		/**
		 * Z-order sort protection: bottom edge.
		 * Prevents a feature from being sorted in front of existing features
		 * that extend further down.
		 */
		FLAG_20000000_ZSORT_BOTTOM = 0x20000000,
		/**
		 * Z-order sort protection: left edge.
		 * Prevents a feature from being sorted in front of existing features
		 * that extend further left.
		 */
		FLAG_40000000_ZSORT_LEFT = 0x40000000,
	};

	/**
	 * Represents a full SCRB, which is responsible for drawing shapes and executing event hooks.
	 * @param vm Pointer to the @ref MohawkEngine_Zoombini instance
	 * @param runnerId Stable runtime runner identifier
	 * @param scrbId The SCRB identifier
	 * @param frameInterval The frame interval
	 * @param flags Feature flags
	 * @param imgResource The archive-qualified shape image resource
	 */
	ZmbFeature(MohawkEngine_Zoombini *vm, uint16 runnerId, int16 scrbId, uint32 frameInterval, uint32 flags, ZmbResource imgResource);
	/**
	 * Represents a full SCRB with a point of reference.
	 * @param vm Pointer to the @ref MohawkEngine_Zoombini instance
	 * @param runnerId Stable runtime runner identifier
	 * @param scrbId The SCRB identifier
	 * @param frameInterval The frame interval
	 * @param pointRef The point of reference
	 * @param flags Feature flags
	 * @param imgResource The archive-qualified shape image resource
	 */
	ZmbFeature(MohawkEngine_Zoombini *vm, uint16 runnerId, int16 scrbId, uint32 frameInterval, const Common::Point &pointRef, uint32 flags, ZmbResource imgResource);
	/**
	 * Represents a virtual SCRB with shapeImageId assigned, which is used to draw MapRect shapes.
	 * @param vm Pointer to the @ref MohawkEngine_Zoombini instance
	 * @param runnerId Stable runtime runner identifier
	 * @param scrbId The SCRB identifier
	 * @param flags Feature flags
	 * @param imgResource The archive-qualified shape image resource
	 */
	ZmbFeature(MohawkEngine_Zoombini *vm, uint16 runnerId, int16 scrbId, uint32 flags, ZmbResource imgResource);
	/**
	 * Represents an empty virtual SCRB, which is used to run event hooks.
	 * It also serves as a base constructor for @ref ZmbSnoid.
	 * @param vm Pointer to the @ref MohawkEngine_Zoombini instance
	 * @param runnerId Stable runtime runner identifier
	 * @param scrbId The SCRB identifier
	 * @param flags Feature flags
	 */
	ZmbFeature(MohawkEngine_Zoombini *vm, uint16 runnerId, int16 scrbId, uint32 flags);
	/** Release the feature, its hotspot groups, and owned SCRB data. */
	virtual ~ZmbFeature();

public:
	/** Initialize decoded script state and render bookkeeping. */
	void initValues();

	// -- Render callback typedefs --------------------------------------
	// @ref ZmbFeature::EventHooks::_preRenderFunc gates the standard pre-render logic.
	// @ref ZmbFeature::EventHooks::_selectRenderFrameFunc selects the frame for that pass.
	// @ref ZmbFeature::EventHooks::_preRenderShapeFunc adjusts parsed hotspots before drawing.
	// @ref ZmbFeature::EventHooks::_renderFunc draws the feature.
	// @ref ZmbFeature::EventHooks::_postRenderFunc runs after drawing.
	// @ref ZoombiniPage::onFeatureAnimEvent() dispatches animation events.
	// @ref ZmbFeature::_animEndCallbackFired tracks the one-shot end-event state.
	typedef bool (ZoombiniPage::*OnPreRenderFunc)(ZmbFeature *feature);
	typedef int32 (ZoombiniPage::*OnSelectRenderFrameFunc)(ZmbFeature *feature);
	typedef void (ZoombiniPage::*OnPreRenderShapeFunc)(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	typedef ZmbRenderResult (ZoombiniPage::*OnRenderFunc)(ZmbFeature *feature);
	typedef void (ZoombiniPage::*OnPostRenderFunc)(ZmbFeature *feature);

	// -- Input event callback typedefs ---------------------------------
	// Per-feature input hooks are used by the page-level event dispatcher.
	typedef ZmbEventHandleResult (ZoombiniPage::*OnMouseMoveFunc)(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);
	typedef ZmbEventHandleResult (ZoombiniPage::*OnLButtonDownFunc)(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);
	typedef ZmbEventHandleResult (ZoombiniPage::*OnLButtonUpFunc)(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);
	typedef ZmbEventHandleResult (ZoombiniPage::*OnKeyDownFunc)(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat);
	typedef ZmbEventHandleResult (ZoombiniPage::*OnKeyUpFunc)(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat);
	typedef ZmbEventHandleResult (ZoombiniPage::*OnWheelUpFunc)(ZmbFeature *feature, const Common::Point &absPos);
	typedef ZmbEventHandleResult (ZoombiniPage::*OnWheelDownFunc)(ZmbFeature *feature, const Common::Point &absPos);

	/** Event hooks for the feature script. */
	struct EventHooks {
		// Render hooks ------------------------------------------------
		/** Gate the standard pre-render path. */
		OnPreRenderFunc _preRenderFunc = nullptr;
		/** Select the frame for the current render pass. */
		OnSelectRenderFrameFunc _selectRenderFrameFunc = nullptr;
		/** Per-hotspot-group shape preprocessing callback. */
		OnPreRenderShapeFunc _preRenderShapeFunc = nullptr;
		/** Shape blitting */
		OnRenderFunc _renderFunc = nullptr;
		/** Additional processing after shape blitting. */
		OnPostRenderFunc _postRenderFunc = nullptr;

		// -- Input event hooks (ScummVM extensions) --------------------
		/** Feature-level pointer-motion callback. */
		OnMouseMoveFunc _mouseMoveFunc = nullptr;
		/** Feature-level left-button-down callback. */
		OnLButtonDownFunc _lButtonDownFunc = nullptr;
		/** Feature-level left-button-up callback. */
		OnLButtonUpFunc _lButtonUpFunc = nullptr;
		/** Feature-level key-down callback. */
		OnKeyDownFunc _keyDownFunc = nullptr;
		/** Feature-level key-up callback. */
		OnKeyUpFunc _keyUpFunc = nullptr;
		/** Feature-level wheel-up callback. */
		OnWheelUpFunc _wheelUpFunc = nullptr;
		/** Feature-level wheel-down callback. */
		OnWheelDownFunc _wheelDownFunc = nullptr;

		// -- Render hook setters ---------------------------------------

		/**
		 * Boolean gate that runs before standard pre-render logic.
		 * Return false to skip @ref ZoombiniPage::preRenderFeature() entirely.
		 */
		void setPreRenderFunc(OnPreRenderFunc preRenderFunc) { _preRenderFunc = preRenderFunc; }
		/**
		 * Frame selection hook.
		 * The default performs the standard frame advance.
		 */
		void setSelectRenderFrameFunc(OnSelectRenderFrameFunc onSelectRenderFrameFunc) { _selectRenderFrameFunc = onSelectRenderFrameFunc; }
		/** Called per-frame after hotspot data is parsed, before shape rendering. */
		void setPreRenderShapeFunc(OnPreRenderShapeFunc preRenderShapeFunc) { _preRenderShapeFunc = preRenderShapeFunc; }
		/**
		 * Shape blitting.
		 * Use @ref ZoombiniPage::blitShapes() when no custom render callback is installed.
		 */
		void setRenderFunc(OnRenderFunc renderFunc) { _renderFunc = renderFunc; }
		/** Additional processing after renderFunc completes successfully. */
		void setPostRenderFunc(OnPostRenderFunc postRenderFunc) { _postRenderFunc = postRenderFunc; }

		// -- Input event hook setters (ScummVM extensions) -------------

		void setMouseMoveFunc(OnMouseMoveFunc mouseMoveFunc) { _mouseMoveFunc = mouseMoveFunc; }
		void setLButtonDownFunc(OnLButtonDownFunc lButtonDownFunc) { _lButtonDownFunc = lButtonDownFunc; }
		void setLButtonUpFunc(OnLButtonUpFunc lButtonUpFunc) { _lButtonUpFunc = lButtonUpFunc; }
		void setKeyDownFunc(OnKeyDownFunc keyDownFunc) { _keyDownFunc = keyDownFunc; }
		void setKeyUpFunc(OnKeyUpFunc keyUpFunc) { _keyUpFunc = keyUpFunc; }
		void setWheelUpFunc(OnWheelUpFunc wheelUpFunc) { _wheelUpFunc = wheelUpFunc; }
		void setWheelDownFunc(OnWheelDownFunc wheelDownFunc) { _wheelDownFunc = wheelDownFunc; }
	};

	/**
	 * Set event hooks for the feature script.
	 * @param hooks The event hooks to set
	 */
	void setEventHooks(const EventHooks &hooks);

	/**
	 * Set the pre-render shape callback on an existing feature.
	 * Used when a callback needs to be set after feature construction.
	 * @param func The pre-render shape callback
	 */
	void setPreRenderShapeFunc(OnPreRenderShapeFunc func) { _eventHooks._preRenderShapeFunc = func; }

	/**
	 * Pre-render pass: animation logic.
	 * Run before Z-sort.
	 * Call the custom pre-render callback.
	 * Advance frame selection and handle the CHAIN_SCRIPT or PLAY_ONCE end of cycle.
	 * Check the SKIP_RENDER and SKIP_ONCE flags, then dispatch sound.
	 * @param page The page context.
	 */
	void onPreRender(ZoombiniPage *page);

	/**
	 * Post-render pass: shape blitting + custom postRender callback.
	 * Run after Z-sort.
	 * Blit shapes to the screen and call the custom post-render callback.
	 * @param page The page to render to.
	 * @return @ref ZmbRenderResult::kRendered if shapes were drawn,
	 * or @ref ZmbRenderResult::kSkipped if rendering is deactivated.
	 */
	ZmbRenderResult onPostRender(ZoombiniPage *page);

	/**
	 * Invoke the select-render-frame event hook.
	 * @param page The page to handle the event
	 * @return The frame index to render
	 */
	int32 onSelectRenderFrame(ZoombiniPage *page);
	/**
	 * Invoke the pre-render-shape event hook.
	 * @param page The page to handle the event
	 * @param hsGroup The hotspot group to handle the event
	 * @param hotspots The hotspots to handle the event
	 */
	void onPreRenderShape(ZoombiniPage *page, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/**
	 * Invoke the mouse move event hook.
	 * @param page The page to handle the event
	 */
	ZmbEventHandleResult onMouseMove(ZoombiniPage *page, const Common::Point &absPos, const Common::Point &relPos);
	/**
	 * Invoke the mouse left button down event hook.
	 * @param page The page to handle the event
	 */
	ZmbEventHandleResult onLButtonDown(ZoombiniPage *page, const Common::Point &absPos, const Common::Point &relPos);
	/**
	 * Invoke the mouse left button up event hook.
	 * @param page The page to handle the event
	 */
	ZmbEventHandleResult onLButtonUp(ZoombiniPage *page, const Common::Point &absPos, const Common::Point &relPos);
	/**
	 * Invoke the key down event hook.
	 * @param page The page to handle the event
	 */
	ZmbEventHandleResult onKeyDown(ZoombiniPage *page, const Common::KeyState &kbd, bool kbdRepeat);
	/**
	 * Invoke the key up event hook.
	 * @param page The page to handle the event
	 */
	ZmbEventHandleResult onKeyUp(ZoombiniPage *page, const Common::KeyState &kbd, bool kbdRepeat);
	/**
	 * Invoke the mouse wheel up event hook.
	 * @param page The page to handle the event
	 */
	ZmbEventHandleResult onWheelUp(ZoombiniPage *page, const Common::Point &absPos);
	/**
	 * Invoke the mouse wheel down event hook.
	 * @param page The page to handle the event
	 */
	ZmbEventHandleResult onWheelDown(ZoombiniPage *page, const Common::Point &absPos);

	/*
	### SCRB and SCRS resource formats

	Both resource types use the same frame stream decoded by @ref ZmbScriptDecoder::decodeScriptFrames().
	Their headers differ by the SCRS-only trait-layout word. A feature binds the cached immutable
	frame array and materializes only the frame currently needed by rendering or callbacks.

	SCRB_Header {
		uint16BE  numFrames       // Number of animation frames (1-203)
	}

	SCRS_Header {
		uint16BE  numFrames       // Header word 0: number of animation frames (1-203)
		uint16BE  traitLayout     // Header word 1: trait-layer slot arrangement (see below)
	}

	The SCRB frame stream starts at byte offset 2.
	The SCRS header is exactly two 16-bit big-endian words (4 bytes).
	Its frame stream starts at byte offset 4.
	SCRS has no initial-X or initial-Y header words.

	// Repeated numFrames times in both resource types:
	Frame[] {
		Entry[] {
			sint16BE  shapeId   // If >= 0: shape index. If < 0: frame terminator.
			sint16BE  x         // X position (only if shapeId >= 0)
			sint16BE  y         // Y position (only if shapeId >= 0)
		}
		// Shared terminator semantics:
		//   0xFF00 (-256): plain end of frame
		//   0xFFxx (-255 to -1): end of frame; low byte is the frame event code
		//   0xFExx and below (< -256): end of frame with the same event-code rule,
		//                              followed by an extra sint16BE sound resource ID
	}

	### SCRS trait-layout field values

	For @ref ZmbSnoid, the registered SCRS group selects @ref SnoidAnimState::kSnoidAnimState009_ScriptNormal or
	@ref SnoidAnimState::kSnoidAnimState008_ScriptReject, including the shape archive and trait-table family.
	The trait-layout word independently maps trait bases to the first five script layers.

	| Value  | Layer slots 0 through 4 | Consumer |
	|--------|-------------------------|----------|
	| 0      | feet, body (zero base), nose, eye, hair | Snoid and Fleen |
	| 1      | feet, nose, body (zero base), eye, hair | Snoid and Fleen |
	| 2      | body (zero base), eye, nose, feet, hair | Snoid and Fleen |
	| 3      | body (zero base), feet, nose, eye, hair | Fleen custom renderer |
	| 0xFFFF | Reserved sentinel; no layer mapping | Non-played placeholder SCRS |

	@ref ZmbSnoid::getBodyLayerBaseOffset() implements layouts 0 through 2.
	The Fleens page renderer implements layouts 0 through 3 for Fleen trait tables.
	The number of nonnegative entries in a frame is variable and consumer-defined;
	it is not encoded by the trait-layout word.
	*/

	/** Bind one cached decoded SCRB as this runner's initial immutable script data. */
	bool setDecodedScrb(const ZmbScriptDecoder::DecodedScrb *decodedScrb, int16 scrbId);
	/**
	 * Swap the SCRB data on this feature.
	 *
	 * This path clears existing script data, binds a cached decoded SCRB,
	 * resets animation state and reruns @ref ZmbFeature::initValues().
	 * It preserves @ref ZmbFeature::_id, flags, callbacks,
	 * and position unless @ref ZmbFeature::FLAG_00800000_POS_DELTA recalculates the position.
	 * It updates @ref ZmbFeature::_scrbId while preserving the runner identity.
	 *
	 * @param decodedScrb Cached immutable SCRB resource owned by the page or engine.
	 * @param scrbId Resource ID of the SCRB data being loaded.
	 * @param scheduleRender Whether to activate rendering after the swap.
	 */
	void loadScrbData(const ZmbScriptDecoder::DecodedScrb *decodedScrb, int16 scrbId, bool scheduleRender = true);
	/**
	 * Bind one cached decoded SCRS as this runner's immutable script data.
	 * The semantic owner retains the trait layout and controls frame timing,
	 * completion, anchoring, and actor-specific rendering.
	 */
	bool setDecodedScrs(const ZmbScriptDecoder::DecodedScrs *decodedScrs);
	/** Return a frame's embedded event code when present. */
	bool getFrameEventCode(int32 frameIdx, uint8 &eventCode) const;
	/** Return a frame's resolved embedded sound resource when present. */
	bool getFrameSoundResource(int32 frameIdx, ZmbResource &resource) const;
	/** Play an immediate sound and retain its mixer handle under this feature. */
	bool playOwnedSound(ZmbResource resource, Audio::Mixer::SoundType soundType = Audio::Mixer::kSFXSoundType) const;
	/** Play a frame's embedded SFX with duplicate-SND suppression. */
	bool playFrameSound(int32 frameIdx) const;
	/**
	 * Enqueue a frame's embedded SFX for the render-frame priority pass.
	 * @param forcePriority Select this candidate above every authored page range.
	 */
	bool enqueueFrameSound(int32 frameIdx, bool forcePriority = false) const;
	/**
	 * Set this runner's script-sound policy. Linked sub-features inherit the
	 * explicit policy so one authored feature chain cannot change dispatch mode
	 * merely because its active runner changes.
	 */
	void setScriptSoundPolicy(ScriptSoundPolicy policy);
	ScriptSoundPolicy getScriptSoundPolicy() const { return _scriptSoundPolicy; }
	/** Stop embedded SFX instances started by this script. */
	void stopFrameSounds() const;
	/**
	 * Set virtual hotspots for the virtual feature script.
	 * @param hotspots The hotspots to set
	 */
	void setVirtualHotspots(const Common::Array<ZmbHotspot> &hotspots);

	/** Stable runner identity used by page feature maps. */
	uint16 getId() const { return _id; }
	/** Current SCRB resource ID, or zero while no SCRB is bound. */
	int16 getScrbId() const { return _scrbId; }
	/** Set the current SCRB resource ID for a runner created without initial SCRB data. */
	void setScrbId(int16 scrbId) { _scrbId = scrbId; }
	ZmbResource getResource() const { return _imgResource; }
	void setResource(ZmbResource res) { _imgResource = res; }

	/**
	 * Set per-tBMP REGS for shape registration-point offsets.
	 * @param regs Weak (non-owning) pointer to the REGS data
	 */
	void setShapeOffsetRegs(ZmbShapeOffsetRegs *regs) { _shapeRegs = regs; }
	ZmbShapeOffsetRegs *getShapeOffsetRegs() const { return _shapeRegs; }

	/** Return the effective hotspot group for a frame, with previous-frame fallback. */
	ZmbHotspotGroup *getHotspotGroup(int32 frameid);
	/** Return the current materialized visual frame, including previous-pose fallback. */
	ZmbHotspotGroup *getCurrentScriptVisualFrame();
	/**
	 * Materialize one exact cached script frame without visual fallback.
	 * Virtual-hotspot features return their feature-owned group.
	 */
	ZmbHotspotGroup *getHotspotGroupExact(int32 frameid) const;
	/** Return one immutable raw frame from the active cached SCRB or SCRS. */
	const ZmbDecodedScriptFrame *getDecodedScriptFrame(int32 frameIdx) const;
	/** Return the total number of hotspots across all parsed frames. */
	uint32 getHotspotTotalCount() const;
	/** Return the number of distinct hotspot IDs in the script. */
	uint16 getHotspotIdCount() const;

	/** Return the authored cached-script frame count or the number of virtual groups. */
	uint32 getFrameCount() const { return _activeDecodedFrames ? _activeDecodedFrames->size() : _virtualFrameMap.size(); }
	int32 getMaxFrameIdx() { return _frameIdxMax; }
	/**
	 * Highest frame index that contains a positive-shape hotspot.
	 * Used by @ref ZmbFeature::FLAG_00100000_PLAY_ONCE to settle on a visible frame
	 * instead of a trailing terminator-only frame.
	 * It may differ from @ref ZmbFeature::getMaxFrameIdx() when a SCRS or SCRB pads with empty terminator frames.
	 * For example, Ferry SCRS 1900 declares 25 frames but only frames 0-10 carry shapes.
	 */
	int32 getLastShapeFrameIdx() const { return _lastShapeFrameIdx; }
	int32 getLastFrameIdx() const { return _lastFrameIdx; }
	void setLastFrameIdx(int32 frameIdx) { _lastFrameIdx = frameIdx; }
	/** Consume the loaded-SCRB frame-zero hold after synchronous materialization. */
	void clearFirstFrameAdvanceHold() { _skipFirstAdvance = false; }
	/** Select the next standard SCRB frame using the page frame counter. */
	int32 defaultSelectRenderFrame(uint32 currentFrameCounter);
	uint32 getFrameInterval() const { return _frameInterval; }
	void setFrameInterval(uint32 interval) { _frameInterval = interval; }
	Common::Point getPointLoc() const { return _pointLoc; }
	void setPointLoc(const Common::Point &pointLoc) { _pointLoc = pointLoc; }
	/**
	 * Set the POS_DELTA reference point independently of the live runner position.
	 * SCRB data initializes the reference point.
	 * Town overwrites it with the feature position for inhabitant and celebration runners before scrolling.
	 */
	void setPointRef(const Common::Point &pointRef) { _pointRef = pointRef; }
	/** Return the POS_DELTA offset from the immutable reference point. */
	Common::Point getPosDelta() const;

	Common::Rect getClickRect() const { return _clickRect; }
	/** Install a click rectangle owned by page logic rather than rendered SCRB geometry. */
	void setClickRect(const Common::Rect &rect) {
		_hasClickRect = true;
		_hasExplicitClickRect = true;
		_clickRect = rect;
	}
	/** Replace the aggregate click rectangle rebuilt from the current rendered shapes. */
	void setRenderedClickRect(const Common::Rect &rect) {
		_hasClickRect = true;
		_clickRect = rect;
	}
	/** Invalidate renderer-owned click coverage while preserving an explicit page-owned rectangle. */
	void invalidateRenderedClickRect() {
		if (!_hasExplicitClickRect)
			_hasClickRect = false;
	}
	bool hasClickRect() const { return _hasClickRect; }
	/** Return whether page logic owns a fixed click rectangle for this runner. */
	bool hasExplicitClickRect() const { return _hasExplicitClickRect; }
	bool isPointInClickRect(const Common::Point &absPos) const {
		return _hasClickRect && _clickRect.contains(absPos);
	}

	/** Clear parsed script data, draw records, and transient playback state. */
	void clear();

	/** Store the rectangle produced for one frame and hotspot. */
	ZmbDrawRecord *setDrawRecord(ZmbHotspotGroup *hsGroup, const ZmbHotspot &hs, const Common::Rect &drawnRect);
	/** Find the draw record for a frame and hotspot index. */
	ZmbDrawRecord *getDrawRecord(uint16 frame, uint16 hsIdx);
	/** Remove one frame/hotspot draw record. */
	void eraseDrawRecord(uint16 frame, uint16 hsIdx);
	/** Remove all materialized draw records. */
	void clearDrawRecords();
	bool hasDrawRecords() const { return !_drawnRecordMap.empty(); }
	/** Append all current draw-record rectangles to @p rects. */
	void collectDrawRecordRects(Common::Array<Common::Rect> &rects) const;
	void setPreparedVisualRects(const Common::Array<Common::Rect> &rects) {
		_preparedVisualRects = rects;
		_preparedVisualRectsValid = true;
	}
	void clearPreparedVisualRects() {
		_preparedVisualRects.clear();
		_preparedVisualRectsValid = false;
	}
	bool hasPreparedVisualRects() const { return _preparedVisualRectsValid; }
	void collectPreparedVisualRects(Common::Array<Common::Rect> &rects) const {
		rects.push_back(_preparedVisualRects);
	}
	void setPreparedRenderHotspots(const Common::Array<ZmbPreparedRenderHotspot> &hotspots) {
		_preparedRenderHotspots = hotspots;
		_preparedRenderFrameValid = true;
		_preparedRenderGeneration += 1;
	}
	const Common::Array<ZmbPreparedRenderHotspot> &getPreparedRenderHotspots() const {
		return _preparedRenderHotspots;
	}
	bool hasPreparedRenderHotspots() const { return _preparedRenderFrameValid; }
	uint32 getPreparedRenderGeneration() const { return _preparedRenderGeneration; }
	void clearPreparedRenderHotspots() {
		_preparedRenderHotspots.clear();
		_preparedRenderFrameValid = false;
	}
	/** True only when the complete post-render path is the predictable default shape blit. */
	bool usesDefaultRenderFunc() const {
		return _usesDefaultRenderFunc && !_eventHooks._postRenderFunc;
	}
	/** Set whether the semantic owner computes this runner's transformed visual coverage. */
	void setManagesOwnVisualCoverage(bool managesCoverage) { _managesOwnVisualCoverage = managesCoverage; }
	/** Return whether the semantic owner supplies transformed visual coverage. */
	bool managesOwnVisualCoverage() const { return _managesOwnVisualCoverage; }

	/**
	 * Activate a post-render clip and queue its geometry update.
	 * Clipping starts immediately, while clickRect intersection begins on the following Snoid timer tick.
	 */
	void queueVisualRectConstraint(const Common::Rect &rect) {
		_visualRectConstraint = rect;
		_hasVisualRectConstraint = true;
		_pendingVisualRectConstraint = rect;
		_hasPendingVisualRectConstraint = true;
	}
	/** Remove the current and pending visual rectangle constraint. */
	void clearVisualRectConstraint() {
		_hasVisualRectConstraint = false;
		_hasPendingVisualRectConstraint = false;
		_visualRectConstraintAffectsGeometry = false;
	}
	void applyPendingVisualRectConstraint() {
		if (!_hasPendingVisualRectConstraint)
			return;
		_visualRectConstraint = _pendingVisualRectConstraint;
		_visualRectConstraintAffectsGeometry = true;
		_hasPendingVisualRectConstraint = false;
	}
	Common::Rect constrainVisualRect(const Common::Rect &rect) const {
		if (!_visualRectConstraintAffectsGeometry)
			return rect;
		Common::Rect constrained = rect;
		constrained.clip(_visualRectConstraint);
		return constrained;
	}
	bool hasVisualRectConstraint() const { return _hasVisualRectConstraint; }
	const Common::Rect &getVisualRectConstraint() const { return _visualRectConstraint; }
	/** Find the topmost draw record containing an absolute point. */
	ZmbDrawRecord *findDrawRecordAtPoint(const Common::Point &absPos);
	/** Collect every draw record containing an absolute point. */
	void findDrawRecordsAtPoint(const Common::Point &absPos, Common::Array<ZmbDrawRecord *> &foundRecords);
	/** Find a draw record by one hotspot ID. */
	ZmbDrawRecord *findDrawRecordByHotspotIdx(uint16 hsIdx);
	/** Find a draw record matching either of two hotspot IDs. */
	ZmbDrawRecord *findDrawRecordByHotspotIdx(uint16 hsIdx1, uint16 hsIdx2);
	/** Find a draw record matching any ID in @p hsIdxArr. */
	ZmbDrawRecord *findDrawRecordByHotspotIdx(Common::Array<uint16> hsIdxArr);
	/** Find a draw record by one shape ID. */
	ZmbDrawRecord *findDrawRecordByShapeId(uint16 shapeId);
	/** Find a draw record matching either of two shape IDs. */
	ZmbDrawRecord *findDrawRecordByShapeId(uint16 shapeId1, uint16 shapeId2);
	/** Find a draw record matching any ID in @p shapeIdArr. */
	ZmbDrawRecord *findDrawRecordByShapeId(Common::Array<uint16> shapeIdArr);

	bool isCloseScheduled() const { return _isCloseScheduled; }
	void scheduleClose() { _isCloseScheduled = true; }

	/** Activate a feature that is currently owned by a parent chain. */
	void activateSubFeature();

	void activateRender() { _isRenderActivated = true; }
	void deactivateRender() { _isRenderActivated = false; }
	bool isRenderActivated() const { return _isRenderActivated; }
	void setInitialScrbLoadPending(bool pending) { _initialScrbLoadPending = pending; }
	bool isInitialScrbLoadPending() const { return _initialScrbLoadPending; }
	/** Enable standard animation and clear the end-event latch. */
	void activateAnimate();
	/** Disable standard animation for this runner. */
	void deactivateAnimate();
	/** Install a page callback that selects the render frame. */
	void setSelectRenderFrameFunc(OnSelectRenderFrameFunc func);

	// Dirty-rect tracking for the render pipeline.
	bool needsRedraw() const { return _needsRedraw; }
	void setNeedsRedraw(bool v) { _needsRedraw = v; }
	/**
	 * Request a new transformed visual frame for state-driven static runners.
	 *
	 * The persistent compositor normally reuses the last materialized frame while
	 * a runner is inactive. The semantic owner calls this when its UI state changes.
	 */
	void requestVisualRematerialization() {
		_visualRematerializationRequested = true;
		_needsRedraw = true;
	}
	/** Consume and clear a pending semantic-owner rematerialization request. */
	bool consumeVisualRematerializationRequest() {
		const bool requested = _visualRematerializationRequested;
		_visualRematerializationRequested = false;
		return requested;
	}

	/**
	 * Suppress repeated @ref ZoombiniPage::kAnimEventM1_End dispatches after the first end-of-cycle callback.
	 * @ref ZmbFeature::activateAnimate() clears this flag for the next animation cycle.
	 */
	bool hasAnimEndCallbackFired() const { return _animEndCallbackFired; }
	/** Mark the one-shot end-of-cycle callback as already dispatched. */
	void markAnimEndCallbackFired() { _animEndCallbackFired = true; }
	uint32 getScrbLoadGeneration() const { return _scrbLoadGeneration; }

	/**
	 * True when the next render deadline passed on this tick.
	 * @ref ZmbFeature::defaultSelectRenderFrame() sets this value.
	 * @ref ZoombiniPage::preRenderFeature() uses it to gate the complete standard animation pass.
	 * Snoids advance through @ref ZmbSnoid::onSnoidAnimTick() and do not consume this state.
	 */
	bool isFrameTimingReady() const { return _frameTimingReady; }
	virtual bool isAnimationTimerDue(uint32 currentFrameCounter) const {
		return _nextRenderFrame <= currentFrameCounter;
	}
	/** Store the timing result shared by a feature timing group. */
	void setSharedFrameTimingResult(bool ready) {
		_hasSharedFrameTimingResult = true;
		_sharedFrameTimingResult = ready;
	}
	/** Clear the shared timing result before the next frame. */
	void clearSharedFrameTimingResult() { _hasSharedFrameTimingResult = false; }
	/** Return the shared timing result or calculate this feature's deadline. */
	bool getFrameTimingResult(uint32 currentFrameCounter) const {
		return _hasSharedFrameTimingResult ? _sharedFrameTimingResult : isAnimationTimerDue(currentFrameCounter);
	}
	/** Schedule the next standard SCRB pass from the current frame counter. */
	void scheduleNextRenderFrame(uint32 currentFrameCounter) { _nextRenderFrame = currentFrameCounter + _frameInterval; }
	/** Reset next render deadline to 0 so the timing gate passes on the next tick. */
	void resetNextRenderFrame() { _nextRenderFrame = 0; }
	/**
	 * Check if this feature should be animated.
	 * @return True if this feature is being animated in animation frame
	 */
	bool isAnimateActivated() const;
	/** Schedule this feature to animate for specific frames. */
	void scheduleAnimateForFrames(uint16 animateFrames);
	/**
	 * Check if an animation cycle is running based on the current frame counter.
	 * @param currentFrameCounter The current frame counter to check against
	 * @return True if an animation cycle is currently running, false otherwise
	 */
	bool isAnimationCycleRunning() const;
	/** Return whether the current frame is the end of the active animation cycle. */
	bool isEndOfAnimationCycle() const;

	int32 getLastSoundedFrameIdx() const { return _lastSoundedFrameIdx; }
	void setLastSoundedFrameIdx(int32 frameIdx) { _lastSoundedFrameIdx = frameIdx; }

	// [*] Flags
	uint32 getFlags() const { return _flags; }
	bool hasFlag(Flag flag) const { return (_flags & flag) != 0; }
	void setFlags(uint32 flags) { _flags = flags; }
	void addFlag(Flag flag) { _flags |= static_cast<uint32>(flag); }
	void removeFlag(Flag flag) { _flags &= ~flag; }
	const Common::Rect &getSortRect() const { return _sortRect; }
	void setSortRect(const Common::Rect &rect) { _sortRect = rect; }

	uint32 getRegistrationIndex() const { return _registrationIndex; }
	void setRegistrationIndex(uint32 registrationIdx) { _registrationIndex = registrationIdx; }

	/**
	 * Get the rectangle used for Z-sorting and dirty invalidation.
	 * @ref ZmbFeature::_sortRect carries current visual bounds,
	 * while @ref ZmbFeature::_clickRect remains available for manual hit zones and as the first-draw fallback.
	 */
	const Common::Rect &getZSortRect() const { return !_sortRect.isEmpty() ? _sortRect : _clickRect; }

	/**
	 * Set the SCRB ID to chain to at end-of-animation-cycle (CHAIN_SCRIPT).
	 * 0 = No target.
	 * Negative = also set RANDOM_FRAME on load.
	 * Cleared automatically after the swap in @ref ZoombiniPage::preRenderFeature().
	 */
	void setChainedScrbId(int16 id) { _chainedScrbId = id; }
	int16 getChainedScrbId() const { return _chainedScrbId; }

	/**
	 * Link another subFeature to this one, to be used for FLAG_00040000_CHAIN_SCRIPT behaviour.
	 * The subFeature's lifetime is managed internally; this is an owning reference.
	 * @param sub The feature to link
	 */
	void setSubFeature(ZmbFeature *subFeature);
	/** Get the feature linked through @ref ZmbFeature::setSubFeature(), or nullptr if none. */
	ZmbFeature *getSubFeature() const { return _refSubFeature; }
	/**
	 * Run the subFeature's render function if it exists.
	 * "Run" means registering the subFeature to the page's active feature list.
	 */
	void runSubFeature(ZoombiniPage *page);

	/** Return true after @ref ZmbFeature::runSubFeature() registers this sub-feature. */
	bool isSubFeatureRunning() const { return _isSubFeatureRunning; }
	/** Set whether this sub-feature is currently registered in the page's active feature list. */
	void setSubFeatureRunning(bool v) { _isSubFeatureRunning = v; }
	/**
	 * Schedule this sub-feature to be detached (removed from the page's feature list without being deleted).
	 * The parent feature retains ownership of the sub-feature.
	 */
	void scheduleDetach() { _isDetachScheduled = true; }
	/** Returns true if this sub-feature is scheduled to be detached from the page's feature list. */
	bool isDetachScheduled() const { return _isDetachScheduled; }
	/** Clear the detach schedule flag after detachment has occurred. */
	void clearDetach() { _isDetachScheduled = false; }

protected:
	/** Bind a stable cached frame array and derive this runner's frame bounds. */
	void bindDecodedFrames(const Common::Array<ZmbDecodedScriptFrame> *frames);
	/** Copy one cached frame into the runner's reusable materialized group. */
	ZmbHotspotGroup *materializeDecodedFrame(int32 frameIdx) const;

private:
	/** Resolve an embedded sound ID to its explicit archive-qualified resource. */
	ZmbResource resolveSoundId(int16 soundResId) const;

	/** Engine instance that owns this feature. */
	MohawkEngine_Zoombini *_vm;
	/** Stable runner identity, separate from the currently loaded SCRB resource. */
	uint16 _id = 0;
	/** Current SCRB resource ID, or zero while no SCRB is bound. */
	int16 _scrbId = 0;

	/**
	 * SCRB ID to chain to at end-of-animation-cycle when FLAG_00040000_CHAIN_SCRIPT is set.
	 * 0 = No chain target.
	 * Negative = set RANDOM_FRAME on load.
	 */
	int16 _chainedScrbId = 0;

	/** Parent feature that owns this sub-feature, when applicable. */
	ZmbFeature *_refSubFeature = nullptr;
	/** Sound-priority policy inherited by this feature's script playback. */
	ScriptSoundPolicy _scriptSoundPolicy = ScriptSoundPolicy::kInheritPage;

	/** True while @ref ZmbFeature::runSubFeature() has this sub-feature registered in the active SCRB map. */
	bool _isSubFeatureRunning = false;
	/**
	 * True when @ref ZoombiniPage::checkCloseFeatures() should remove this sub-feature from the page map
	 * without being deleted (the parent feature still owns the pointer).
	 */
	bool _isDetachScheduled = false;

	/** Feature-owned groups used only by callback and common-image virtual runners. */
	Common::HashMap<int32, ZmbHotspotGroup *> _virtualFrameMap;
	/** Stable cached SCRB or SCRS frame array owned by the active page or engine. */
	const Common::Array<ZmbDecodedScriptFrame> *_activeDecodedFrames = nullptr;
	/** Reusable mutable copy of one cached script frame for rendering and callbacks. */
	mutable ZmbHotspotGroup *_materializedScriptFrame = nullptr;
	/** Latest mixer handle per immediate SFX resource started by this runner. */
	mutable Common::HashMap<int16, Audio::SoundHandle> _frameSoundHandles;
	/** key: shape id, value: ZmbDrawRecord */
	Common::StableMap<uint32, ZmbDrawRecord *> _drawnRecordMap;
	/** Current frame coverage computed before post-render; distinct from old DrawRecords. */
	Common::Array<Common::Rect> _preparedVisualRects;
	/** Whether @ref _preparedVisualRects contains current-frame coverage. */
	bool _preparedVisualRectsValid = false;
	/**
	 * Complete transformed frame consumed by the default post-render callback.
	 * Validity is separate because an explicitly empty materialized frame must persist.
	 */
	Common::Array<ZmbPreparedRenderHotspot> _preparedRenderHotspots;
	/** Whether @ref _preparedRenderHotspots is valid for post-render. */
	bool _preparedRenderFrameValid = false;
	/** Generation number incremented whenever prepared hotspots are replaced. */
	uint32 _preparedRenderGeneration = 0;

	/** Optional clip applied to visual and hit-test coverage. */
	Common::Rect _visualRectConstraint;
	/** Next clip waiting for the next Snoid geometry tick. */
	Common::Rect _pendingVisualRectConstraint;
	/** Whether the current visual rectangle constraint is active. */
	bool _hasVisualRectConstraint = false;
	/** Whether a replacement constraint is waiting for the next geometry tick. */
	bool _hasPendingVisualRectConstraint = false;
	/** Whether the current constraint changes geometry and hit testing. */
	bool _visualRectConstraintAffectsGeometry = false;

	/** Current click rectangle used for hit testing. */
	Common::Rect _clickRect;
	/** Whether @ref ZmbFeature::_clickRect contains valid frame coverage. */
	bool _hasClickRect = false;
	/** Whether page logic owns @ref ZmbFeature::_clickRect instead of SCRB rendering. */
	bool _hasExplicitClickRect = false;

	// [*] Registered Informations
	/** Interval of frames between two consequent render timings. */
	uint32 _frameInterval = 0;
	/** Feature flags controlling ownership, rendering, animation, and input. */
	uint32 _flags = 0;
	/** Stable registration order used by rendering and event sorting. */
	uint32 _registrationIndex = 0;
	/** Image resource used by this feature's shapes. */
	ZmbResource _imgResource;
	/**
	 * Per-tBMP shape registration-point offsets loaded for this feature's image resource.
	 * @ref ZoombiniPage::blitShapes() subtracts these offsets after @ref ZmbFeature::onPreRenderShape().
	 * This is a weak pointer whose lifetime is managed by @ref ZoombiniPage::_regsMap.
	 */
	ZmbShapeOffsetRegs *_shapeRegs = nullptr;
	/**
	 * (FLAG_00800000_POS_DELTA or FLAG_00000001_TYPE_SNOID only)
	 * The position of the feature script, which can be changed when animating.
	 */
	Common::Point _pointLoc;
	/**
	 * (FLAG_00800000_POS_DELTA or FLAG_00000001_TYPE_SNOID only)
	 * The position set when the feature script is created, which can be used as a immutable reference.
	 */
	Common::Point _pointRef;

	// [*] Frame controls for animation
	/** Current selected frame index. */
	int32 _lastFrameIdx = 0;
	/** Highest parsed frame index in the script. */
	int32 _frameIdxMax = 0;
	/** Highest frame containing a positive-shape hotspot; see @ref ZmbFeature::getLastShapeFrameIdx(). */
	int32 _lastShapeFrameIdx = 0;
	/** Last frame whose embedded sound was dispatched. */
	int32 _lastSoundedFrameIdx = -1;
	/**
	 * Absolute frame counter at which the next animation advance is allowed.
	 * @ref ZmbFeature::defaultSelectRenderFrame() compares it with the current frame counter.
	 */
	uint32 _nextRenderFrame = 0;
	/**
	 * Set after SCRB load to hold frame zero for the first later due pass.
	 * Initial registration consumes it during synchronous frame-zero materialization.
	 * Runtime loads clear it after the skip in @ref ZmbFeature::defaultSelectRenderFrame().
	 */
	bool _skipFirstAdvance = false;
	/**
	 * Result of the frame-deadline gate.
	 * It controls the complete pre-render body: end-of-cycle handling, frame advance, flag checks, hotspot walking,
	 * and sound dispatch.
	 * Set by @ref ZmbFeature::defaultSelectRenderFrame() and checked by @ref ZoombiniPage::preRenderFeature().
	 */
	bool _frameTimingReady = true;
	/** Shared hotspot-slot timing decision prepared by @ref ZoombiniPage. */
	bool _hasSharedFrameTimingResult = false;
	/** Shared frame-deadline result prepared by the page. */
	bool _sharedFrameTimingResult = false;

	// [*] Z-sort rect: bounding box of all shapes drawn in the previous frame.
	// @ref ZoombiniPage::blitShapes() updates it for @ref ZoombiniPage::renderFeatures() sorting.
	/** Bounding rectangle used for positional Z sorting and dirty invalidation. */
	Common::Rect _sortRect;

	// [*] State controls
	/** Whether the feature is scheduled for close and removal. */
	bool _isCloseScheduled = false;
	/** Whether animation processing is active. */
	bool _isAnimateActivated = false;
	/** Whether rendering is active. */
	bool _isRenderActivated = true;
	/**
	 * Tracks the one-time first-pre-render transition.
	 * Frame zero is materialized before DEFER_ANIM or DEFER_RENDER can make the runner dormant.
	 */
	bool _initialScrbLoadPending = false;
	/**
	 * Set when @ref ZoombiniPage::preRenderFeature() advances the animation.
	 * Cleared after @ref ZoombiniPage::renderFeatures().
	 * While set, the feature's visual coverage joins the dirty region and its shapes are redrawn.
	 */
	bool _needsRedraw = false;
	/** Pending semantic-owner refresh of a static runner's transformed visual frame. */
	bool _visualRematerializationRequested = false;
	/**
	 * Tracks whether the one-shot end-of-cycle event has fired.
	 * Reset by @ref ZmbFeature::activateAnimate().
	 */
	bool _animEndCallbackFired = false;

	/**
	 * Generation counter incremented by @ref ZmbFeature::loadScrbData().
	 * @ref ZoombiniPage::preRenderFeature() uses it
	 * to detect a SCRB loaded during the end callback and avoid marking the fresh animation complete.
	 */
	uint32 _scrbLoadGeneration = 0;

	// [*] Callbacks
	/** Whether the default page render callback is still in use. */
	bool _usesDefaultRenderFunc = true;
	/** Whether the semantic owner computes custom transformed visual coverage. */
	bool _managesOwnVisualCoverage = false;
	/** Page callbacks attached to this feature runner. */
	EventHooks _eventHooks;
};

/** Animation state for a @ref ZmbSnoid. */
enum SnoidAnimState : uint8 {
	/** Idle: periodically rolls fidget chance (10%) */
	kSnoidAnimState000_Idle = 0,
	/** Turn-around right: post-arrival facing flip (right->left), then idle */
	kSnoidAnimState001_TurnLeft = 1,
	/** Turn-around left: post-arrival facing flip (left->right), then idle */
	kSnoidAnimState002_TurnRight = 2,
	/** Flipping: swaps shape layers for 6 frames */
	kSnoidAnimState003_Flip = 3,
	/** Arriving: moves to target, then transitions to idle */
	kSnoidAnimState004_Arrive = 4,
	/** Being dragged by cursor */
	kSnoidAnimState005_Drag = 5,
	/** Playing fidget animation */
	kSnoidAnimState006_Fidget = 6,
	/** Departing: starts path-walking animation */
	kSnoidAnimState007_Depart = 7,
	/** Playing a "reject" response script (e.g., Ferry rejecting flight). */
	kSnoidAnimState008_ScriptReject = 8,
	/** Playing a standard event or dialogue script. */
	kSnoidAnimState009_ScriptNormal = 9,
	/** Arrival motion: moves to target with a specific animation */
	kSnoidAnimState010_ArrivalMotion = 10,
	/** Path-walking: following NODE waypoints toward destination */
	kSnoidAnimState112_Path = 112,
};

/** Behavior applied when a Snoid SCRS reaches its final frame. */
enum class ZmbScrsCompletionMode : byte {
	/** Return the Snoid to its visible idle pose at the final script position. */
	kReturnToIdle,
	/** Hide the Snoid at the final script position. */
	kHide
};

/** Snoid rendering state used while a SCRS is playing. */
enum class ZmbScrsPlaybackMode : byte {
	/** Use the standard-event state 9 body-layer renderer. */
	kNormal,
	/** Use the rejection state 8 body-layer renderer. */
	kReject
};

/** A Zoombini runner with layered SCRS animation support. */
class ZmbSnoid : public ZmbFeature {
public:
	/** Construct a Snoid feature for an active-pack or page runner. */
	ZmbSnoid(MohawkEngine_Zoombini *vm, uint16 snoidId, uint32 flags);
	/** Release Snoid-specific animation and trait state. */
	~ZmbSnoid() override;

	/**
	 * Resolve a cached SCRS resource and bind its decoded frames and trait layout.
	 * SCRS uses an exact two-word header: uint16BE frame count followed by a
	 * uint16BE trait-layout field. Frame data begins immediately afterward.
	 */
	bool loadScrsResource(ZmbResource resource);

	/**
	 * Return the current position and four traits as one debug line.
	 * @param showPosition Whether to include the current position.
	 */
	Common::String toStr(bool showPosition = true) const;

	// --- Animation state machine ---

	/**
	 * Set the Snoid animation state and initialize the state-specific frame data.
	 * Resets frame counters and configures the animation for the given state.
	 * @param state The new animation state
	 * @param pos Optional position override (e.g. target position for walking)
	 */
	void setAnimState(SnoidAnimState state, const Common::Point *pos = nullptr);
	/**
	 * Start the six-tick idle trait-highlight flash selected by a hair/eye/nose/feet bitmask.
	 * Selected layers alternate with the orange tBMP 3000 subshapes 851 through 890.
	 * Bits zero through three correspond to serialized trait order.
	 */
	void startTraitHighlight(uint16 traitMask);

	/**
	 * Advance the Snoid animation state machine once during the pre-render phase.
	 * Updates walking positions, handles idle fidget rolls, and advances SCRS playback.
	 * @param page The page owning this snoid (for frame counter, NODE access, etc.)
	 * @return true if the snoid's visuals changed and need re-rendering
	 */
	bool onSnoidAnimTick(ZoombiniPage *page);
	/**
	 * Reset the dedicated Snoid animation deadline.
	 * @ref ZoombiniPage::beginSnoidDrag() calls this before changing the temporary drag interval to 3.
	 */
	void resetNextAnimFrame() { _nextAnimFrame = 0; }

	SnoidAnimState getAnimState() const { return _animState; }
	bool isFacingLeft() const { return _isFacingLeft; }
	void setFacingLeft(bool facingLeft) { _isFacingLeft = facingLeft; }
	/** 0=Front, 1=right-facing common pose, 2=variant.*/
	void setCommonImageIndex(uint8 imageIndex) { _shapeImageIdx = imageIndex; }

	/** Set holding animation phase for feet cycling. */
	void setHoldingAnimPhase(uint16 phase) { _holdingAnimPhase = phase; }
	uint16 getHoldingAnimPhase() const { return _holdingAnimPhase; }

	/**
	 * Build virtual hotspots for the generic common-image pose family.
	 * Raw shapes 1, 2, and 3 correspond to SCRS 100, 101, and 102.
	 */
	void setupCommonImageHotspots(uint16 rawShape, bool useSmallShapeRegs);

	/**
	 * Set up virtual hotspots for the idle pose from the Snoid's current traits.
	 * Uses the reversed lookup tables for five values of each body trait.
	 * Call this after assigning @ref ZmbSnoid::_trait and before the first render frame.
	 * Raw shape 2 is the right-facing SCRS 101 idle pose.
	 */
	void setupIdleHotspots();

	/**
	 * Build virtual hotspots for the SMALL-scale idle pose (XFER FromIsle scene).
	 * Uses compact body-part index tables paired with the small-snoid SHPL resource 3200 in the system archive.
	 * Also enables @ref ZmbSnoid::_useSmallShapeRegs so the renderer uses the small REGS offsets.
	 */
	void setupSmallIdleHotspots();

	/**
	 * Rebuild hotspots for the current generic common-image pose.
	 * Used by arrival and turn states, which keep @ref ZmbSnoid::_shapeImageIdx.
	 */
	void setupCurrentCommonImageHotspots();

	/** True when this snoid uses the small-scale SHPL/REGS (resource 3200, XFER_0 only). */
	bool _useSmallShapeRegs = false;

	/**
	 * Start SCRS script playback on this Snoid.
	 * Resolves and binds the cached resource, then computes the secondary-position anchor offset.
	 * Configures frame advancement for states 8 and 9.
	 *
	 * @param resource Archive-qualified SCRS resource to play.
	 * @param completionMode Whether the Snoid returns to idle or becomes hidden after the SCRS finishes.
	 * @param playbackMode Whether playback uses the NORMAL state 9 or REJECT state 8 renderer.
	 * @param initPos Optional anchor override.
	 * When non-null, the method finds the last positive-shape anchor and aligns that frame to @p initPos.
	 * Ferry landing SCRS use this to end at the target instead of starting there.
	 * @param suppressVoiceEvents Suppress 200..239 voice opcodes for page uses with intentionally silent animation.
	 * @return Whether the resource was decoded and playback started.
	 */
	bool startScrsPlayback(ZmbResource resource, ZmbScrsCompletionMode completionMode,
						   ZmbScrsPlaybackMode playbackMode = ZmbScrsPlaybackMode::kReject,
						   const Common::Point *initPos = nullptr, bool suppressVoiceEvents = false);
	bool allowsScrsVoiceEvents() const { return !_scrsVoiceEventsSuppressed; }
	/** Set whether completion of the current SCRS dispatches the page-owned end event. */
	void setScrsEndEventEnabled(bool enabled) { _scrsEndEventEnabled = enabled; }
	/** Return the resource ID of the active SCRS, or zero when unspecified. */
	int16 getActiveScrsId() const { return _activeScrsId; }

	/**
	 * Clean up after SCRS playback finishes and clear the select-render-frame hook.
	 * The automatic completion path keeps pointLoc at the current SCRS position for the handoff.
	 */
	void finishScrsPlayback(bool restorePosition = false);

	/**
	 * Return the translation applied to raw SCRS hotspot coordinates while rendering.
	 * Stores this separately as -secondary position while feature position tracks the current visible root.
	 */
	Common::Point getScrsRenderOffset() const {
		if (_animState == kSnoidAnimState008_ScriptReject || _animState == kSnoidAnimState009_ScriptNormal)
			return _scrsRenderOffset;
		return getPointLoc();
	}

	/**
	 * Returns true if @ref ZmbSnoid::setupIdleHotspots() synthesized the hotspot data.
	 * Such data already contains the combined trait-table and raw-shape value in @ref ZmbHotspot::_shapeIdx.
	 * Returns false for SCRS-parsed Snoids that still need @ref ZmbSnoid::getBodyLayerBaseOffset() before mirroring.
	 */
	bool hasCombinedShapeIndices() const { return _usesVirtualHotspots; }

	/**
	 * Return the trait-based base offset to add to a raw SCRS shape for a body layer.
	 * Trait layout 0 is:
	 * layer 0 = feet, layer 1 = body-center (0), layer 2 = nose, layer 3 = eye, layer 4 = hair
	 *
	 * @ref SnoidAnimState::kSnoidAnimState009_ScriptNormal normally uses the script-specific trait tables
	 * for every trait layout. Other animation states use the general trait tables.
	 *
	 * @param layer Zero-based body-part layer index matching @ref ZmbHotspot::_hsId.
	 * @param layerShift Shift used when a NORMAL frame's first raw shape exceeds
	 * 18, so layer 0 gets no trait offset and layers 1-5 map to slots 0-4.
	 */
	int16 getBodyLayerBaseOffset(uint8 layer, uint8 layerShift = 0) const;

	/**
	 * Compute the voice SFX resource ID for this snoid, given a voice group index.
	 *
	 * @param voiceGroup Voice group 0-17 (mapped from SCRS event codes 200-217).
	 * @return SND resource ID to play, or 0 if suppressed.
	 */
	int16 getVoiceResId(int16 voiceGroup) const;

	/**
	 * Update virtual hotspots for the current walk animation phase using live SCRS data.
	 * Selects the correct directional SCRS (from SCRS 105-129) for this snoid's feet type
	 * and the current movement direction bucket, then applies trait base offsets.
	 * @param page      The owning page, which routes the system SCRS lookup to the engine cache.
	 * @param dirBucket Direction bucket 0-4 (from movement slope: 0=steeply down, 4=steeply up).
	 * @param phase     Raw walk phase counter (wrapped with % frameCount inside).
	 */
	void updateWalkHotspots(ZoombiniPage *page, int dirBucket, int phase);

	/**
	 * Update virtual hotspots for the current fidget animation frame using SCRS data.
	 * Selects from SCRS 130-137 (set 0) or SCRS 138-145 (set 1) for this snoid's traits.
	 * Empty frames are skipped to preserve the current pose.
	 * @param page The owning page, which routes the system SCRS lookup to the engine cache.
	 * @param fidgetSet 0 = normal shape-image set, 1 = flipped set.
	 * @param variant Random variant 0-7.
	 * @param frameIdx  Current animation frame index (0-based, not wrapped here).
	 */
	void updateFidgetHotspots(ZoombiniPage *page, int fidgetSet, int variant, int frameIdx);

	/**
	 * Update virtual hotspots for the holding (drag) animation using SCRS data.
	 * Selects from SCRS 146-150 (one per feet type 1-5) based on this snoid's feet trait.
	 * @ref ZmbSnoid::_holdingAnimPhase selects the frame and cycles through the available frames.
	 * @param page The owning page, which routes the system SCRS lookup to the engine cache.
	 */
	void updateHoldingHotspots(ZoombiniPage *page);

	/** Return the current walk or arrival target. */
	Common::Point getAnimTargetPos() const { return _animTargetPos; }
	/** Set the walk or arrival target. */
	void setAnimTargetPos(const Common::Point &pos) { _animTargetPos = pos; }

	/** Return the horizontal walk speed per animation tick. */
	int16 getAnimSpeedX() const { return _animSpeedX; }
	/** Return the vertical walk speed per animation tick. */
	int16 getAnimSpeedY() const { return _animSpeedY; }
	/** Set both components of the walk speed. */
	void setAnimSpeed(int16 speedX, int16 speedY) {
		_animSpeedX = speedX;
		_animSpeedY = speedY;
	}

	/**
	 * Set up a walk to the given target position.
	 * Sets @ref ZmbSnoid::_animTargetPos and enters @ref kSnoidAnimState007_Depart,
	 * which will initialize NODE/PATH routing or a direct fallback with dynamic speed.
	 *
	 * @param target The destination position to walk toward.
	 * @param page Optional owning page.
	 * When supplied, materialize the first walking frame synchronously.
	 */
	void initWalkToTarget(const Common::Point &target, ZoombiniPage *page = nullptr);

	/**
	 * Set up the Picker repack direct-walk exception.
	 *
	 * Capture the one-shot direct-walk decision on this runner.
	 * Consume it when @ref kSnoidAnimState007_Depart initializes the route.
	 */
	void initDirectWalkToTarget(const Common::Point &target, ZoombiniPage *page = nullptr);

	/**
	 * Set @ref ZmbSnoid::_delayUntilFrame.
	 * This path uses the same deadline field for ordinary animation timing and staggered starts,
	 * so replacing the delay must also replace the next timer fire.
	 */
	void setDelayUntilFrame(uint32 frame) {
		_delayUntilFrame = frame;
		_nextAnimFrame = frame;
	}

	/** Return whether the first animation tick is still gated by an absolute deadline. */
	bool hasDeferredAnimationStart() const {
		return _delayUntilFrame != 0;
	}
	/** Return whether the Snoid's absolute animation deadline has arrived. */
	bool isAnimationTimerDue(uint32 currentFrameCounter) const override {
		return _nextAnimFrame <= currentFrameCounter;
	}

	/**
	 * Override the trait layout.
	 * Changes which trait tables map to the five layer slots in @ref ZmbSnoid::getBodyLayerBaseOffset().
	 * Values: 0=(feet,body,nose,eye,hair), 1=(feet,nose,body,eye,hair),
	 * 2=(body,eye,nose,feet,hair).
	 */
	/** Set the trait-layer arrangement used by virtual and SCRS rendering. */
	void setTraitLayout(ZmbScriptDecoder::TraitLayout traitLayout) { _traitLayout = traitLayout; }
	/** Return the active trait-layer arrangement. */
	ZmbScriptDecoder::TraitLayout getTraitLayout() const { return _traitLayout; }

	/**
	 * Per-snoid SCRS animation cycle counter.
	 * Incremented by the page's @ref ZoombiniPage::onFeatureAnimEvent() when event code 0 fires to toggle facing.
	 * This counter is meaningful only during SCRS playback.
	 */
	uint8 _scrsAnimCycleCount = 0;

	/** Trait values that define this Zoombini's layered body. */
	ZmbTrait _trait;
	/** Localized display name associated with this Zoombini. */
	Common::U32String _name;

	/**
	 * Tracks whether this snoid occupies a pedestal slot (true)
	 * or is a non-occupied entry sitting at an arbitrary position (false).
	 * A page sets this when it materializes a saved-pack runner.
	 * @ref ZoombiniPage::saveSnoidsToPack() uses it to rebuild the active pack with the correct occupancy flags.
	 */
	bool _packIsOccupied = false;

	/**
	 * True only for a real Zoombini runner materialized from a saved pack.
	 * Pack runners and temporary SCRS animation runners share @ref ZoombiniPage::_snoidMap,
	 * so ID ranges cannot distinguish them; several pools also use IDs above 10000.
	 */
	bool isPackSnoid() const { return _isPackSnoid; }
	void markAsPackSnoid() {
		_isPackSnoid = true;
		// Initialize every active-pack runner with common image 1 and facing right.
		_shapeImageIdx = 1;
		_isFacingLeft = false;
	}
	/** Seed the idle-fidget phase assigned when an active-pack runner is materialized. */
	void setIdleTickCounter(uint8 idleTickCounter) { _idleTickCounter = idleTickCounter; }

	/**
	 * Raw auxiliary state owned and interpreted by the current page.
	 * Pages that use this byte define a local SnoidRunnerStatus enum.
	 * This byte does not represent animation state or serialized pack occupancy.
	 */
	byte _runnerStatus = 0;

private:
	/** Convert a route delta into per-axis walk speeds. */
	static void calcPathSpeed(int16 dx, int16 dy, int16 &speedX, int16 &speedY);
	/** Convert a route delta into one of the five walk direction buckets. */
	static int computeWalkDirBucket(int16 dx, int16 dy);

	/** Advance to the next NODE/PATH checkpoint when the current one is reached. */
	bool advancePathSubTarget(ZoombiniPage *page, bool forceHotspotUpdate = false);
	/** Synchronize the feature position with the current SCRS root hotspot. */
	void syncScrsPointLoc();
	/** Advance the shared idle-fidget tail used by idle and post-arrival turn states. */
	void processIdleFidget(ZoombiniPage *page, bool &needsRedraw);

	/** Engine instance that owns this Snoid. */
	MohawkEngine_Zoombini *_vm;
	/** Runtime runner ID used by the page feature registry. */
	uint16 _id = 0;
	/** Whether this runner came from serialized active-pack data. */
	bool _isPackSnoid = false;

	/**
	 * Trait-layer arrangement from the second SCRS word.
	 * @ref ZmbSnoid supports layouts 0 through 2.
	 * Layout 3 belongs to the Fleen renderer, and 0xFFFF (-1) is a reserved sentinel.
	 */
	ZmbScriptDecoder::TraitLayout _traitLayout = ZmbScriptDecoder::TraitLayout::kInvalid;

	// --- Animation state fields ---
	/** Current Snoid animation state-machine state. */
	SnoidAnimState _animState = kSnoidAnimState000_Idle;
	/** Whether the rendered Snoid faces left. */
	bool _isFacingLeft = false;

	/** Target position for walking/arriving animations. */
	Common::Point _animTargetPos;
	/** Walk speed per tick (X component). */
	int16 _animSpeedX = 0;
	/** Walk speed per tick (Y component). */
	int16 _animSpeedY = 0;
	/** Counter for idle fidget timing (incremented each idle tick). */
	uint8 _idleTickCounter = 0;
	/**
	 * Set when @ref ZmbSnoid::setAnimState() enters idle state.
	 * Cleared and redraw-marked on the first idle animation tick.
	 * This is separate from @ref ZmbSnoid::_fidgetValue, which selects the fidget variant.
	 */
	bool _needsIdleRedraw = false;
	/** Counter for flip animation (0..6). Swaps layers each tick. */
	int16 _flipCounter = 0;
	/**
	 * Combined pre-mirror shape indices for flip animation.
	 * Computed from trait-specific shape categories (425+hair, 430+eye, 435+feet, 440+nose)
	 * on flip entry, then swapped with main hotspot shapes each tick for 6 ticks.
	 * The renderer maps the selected categories to tBMP 3000 subshapes 851 through 890.
	 */
	int16 _flipShadowShapes[5] = {0, 0, 0, 0, 0};
	/** Random fidget variant value (0..7), selecting SCRS 130-137 or 138-145. */
	int16 _fidgetValue = 0;
	/**
	 * This field tracks the animation image state.
	 * 0 = front/center common image, 1 = right-facing idle/normal animated state,
	 * 2 = flipped or variant state selected by @ref kSnoidAnimState003_Flip.
	 * This value selects fidget set A for 1 (SCRS 130-137) or set B for 2 (SCRS 138-145).
	 */
	uint8 _shapeImageIdx = 0;
	/**
	 * Holding animation phase counter for feet cycling animation.
	 * @ref ZmbSnoid::onSnoidAnimTick() advances this counter each tick in drag state.
	 * When phase >= frameCount, resets to 2 and loops (or 0 for small snoid mode).
	 * This creates the "dangling feet" animation while snoid is held.
	 */
	uint16 _holdingAnimPhase = 0;
	/** Current NODE/PATH route index. */
	int16 _pathRouteIdx = -1;
	/** Next PATH slot to read. */
	int16 _pathSlotIdx = -1;
	/** PATH slot increment, +1 or -1. */
	int16 _pathWalkDir = 0;
	/** Current checkpoint or final destination. */
	Common::Point _pathSubTarget;
	/** One-shot direct-routing request consumed by the next state-7 route setup. */
	bool _skipNodePathOnNextDepart = false;
	/**
	 * True when @ref ZmbHotspot::_shapeIdx values already include the trait-base offset.
	 * @ref ZmbSnoid::setupIdleHotspots() and @ref ZmbSnoid::updateWalkHotspots() set this for virtual hotspots.
	 * It remains false for SCRS-parsed Snoids.
	 */
	bool _usesVirtualHotspots = false;
	/**
	 * Walk animation cycle phase, stored as a raw counter and wrapped
	 * by the frame count in @ref ZmbSnoid::updateWalkHotspots().
	 */
	int _walkPhase = 0;
	/** Current walk direction bucket 0-4 (0=slope-down, 2=horizontal, 4=slope-up). */
	int _walkDirBucket = 2;
	/**
	 * Saved @ref ZmbFeature::_pointLoc from before SCRS playback.
	 * Only restored by callers that explicitly ask for pre-SCRS position restoration.
	 */
	Common::Point _scrsOrigPointLoc;
	/**
	 * Translation added to raw SCRS hotspot coordinates while rendering.
	 * Kept separate from pointLoc because scripted frames update pointLoc to the current visible root
	 * before callbacks are dispatched.
	 */
	Common::Point _scrsRenderOffset;
	/**
	 * Completion behavior for SCRS playback in states 8 and 9.
	 * @ref ZmbSnoid::startScrsPlayback() configures this behavior.
	 * @ref ZoombiniTransitionXfer uses it to hide celebration-animated Snoids after their scripts complete.
	 */
	ZmbScrsCompletionMode _scrsCompletionMode = ZmbScrsCompletionMode::kReturnToIdle;
	/** Whether voice opcodes are suppressed during active SCRS playback. */
	bool _scrsVoiceEventsSuppressed = false;
	/** Whether completion of the current SCRS dispatches the page-owned end event. */
	bool _scrsEndEventEnabled = true;
	/**
	 * Renders SCRS frame 0 immediately before the timer-driven state machine advances.
	 * Keep the first timer tick from skipping directly to frame 1.
	 */
	bool _scrsJustStarted = false;
	/** SCRS resource currently driving this Snoid, or zero when inactive. */
	int16 _activeScrsId = 0;
	/**
	 * Time-based animation deadline.
	 * Animation fires when @ref ZoombiniPage::getCurrentFrameCounter() reaches @ref ZmbSnoid::_nextAnimFrame.
	 * The next deadline is the current frame counter plus @ref ZmbFeature::getFrameInterval().
	 *
	 * The deadline uses the selected animation clock,
	 * making the interval wall-clock based regardless of the actual render rate.
	 * Zero makes the first tick fire immediately.
	 */
	uint32 _nextAnimFrame = 0;
	/**
	 * Deferred start frame counter.
	 * A non-zero value suppresses ticking and rendering.
	 * The deadline clears when @ref ZoombiniPage::getCurrentFrameCounter() reaches @ref ZmbSnoid::_delayUntilFrame.
	 * Used by the Shift+dice "generate all" path to stagger Snoid entries.
	 */
	uint32 _delayUntilFrame = 0;
};

} // End of namespace Mohawk

#endif
