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

#include "mohawk/resource.h"

#include "common/events.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "mohawk/zoombini.h"
#include "mohawk/zoombini_page.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_scripts.h"
#include "mohawk/zoombini_sound.h"

namespace Mohawk {

bool ZmbNode::parseStream(Common::SeekableReadStream *stream) {
	_waypoints.clear();
	if (!ZmbResource::hasBytes(stream, 2)) {
		warning("ZmbNode: missing or truncated NODE header");
		delete stream;
		return false;
	}

	const uint16 numPoints = stream->readUint16BE();
	const int64 expectedSize = 2 + static_cast<int64>(numPoints) * 4;
	if (numPoints == 0 || UINT8_MAX < numPoints || !ZmbResource::hasSize(stream, expectedSize, expectedSize)) {
		warning("ZmbNode: invalid waypoint count %u for a %lld-byte NODE resource", numPoints, static_cast<long long>(stream->size()));
		delete stream;
		return false;
	}

	for (uint16 waypointIdx = 0; waypointIdx < numPoints; waypointIdx++) {
		const int16 coordX = stream->readSint16BE();
		const int16 coordY = stream->readSint16BE();
		_waypoints.push_back(Common::Point(coordX, coordY));
	}
	if (stream->pos() != stream->size()) {
		warning("ZmbNode: trailing data after %u NODE waypoints", numPoints);
		_waypoints.clear();
		delete stream;
		return false;
	}

	delete stream;
	return true;
}

bool ZmbNode::parsePathStream(Common::SeekableReadStream *stream) {
	// PATH resource: uint16BE path count, then M * 24 bytes of one-based waypoint references; zero marks an empty slot.
	// Each path stores 24 one-byte waypoint references immediately after the count header.
	_paths.clear();
	if (!ZmbResource::hasBytes(stream, 2)) {
		warning("ZmbNode: missing or truncated PATH header");
		delete stream;
		return false;
	}

	const uint16 pathCount = stream->readUint16BE();
	const int64 expectedSize = 2 + static_cast<int64>(pathCount) * 24;
	if (!ZmbResource::hasSize(stream, expectedSize, expectedSize)) {
		warning("ZmbNode: invalid path count %u for a %lld-byte PATH resource", pathCount, static_cast<long long>(stream->size()));
		delete stream;
		return false;
	}
	_paths.resize(pathCount);
	for (uint16 pathIdx = 0; pathIdx < pathCount; pathIdx++) {
		_paths[pathIdx].resize(24);
		for (int pathByteIdx = 0; pathByteIdx < 24; pathByteIdx++) {
			const uint8 waypointReference = stream->readByte();
			if (_waypoints.size() < waypointReference) {
				warning("ZmbNode: PATH %u slot %d references missing waypoint %u", pathIdx, pathByteIdx, waypointReference);
				_paths.clear();
				delete stream;
				return false;
			}
			_paths[pathIdx][pathByteIdx] = waypointReference;
		}
	}
	if (stream->pos() != stream->size()) {
		warning("ZmbNode: trailing data after %u PATH records", pathCount);
		_paths.clear();
		delete stream;
		return false;
	}
	delete stream;
	return true;
}

Common::Array<int16> ZmbShapeOffsetRegs::parseStream(Common::SeekableReadStream *stream) {
	Common::Array<int16> coord;
	if (!stream || stream->size() < 0 || (stream->size() & 1) != 0) {
		warning("ZmbRegsOffset: missing or odd-sized REGS axis resource");
		delete stream;
		return coord;
	}

	// ZmbRegsOffset handles the paired-axis registration form of REGS.
	// Resource IDs with other page-specific schemas must be decoded by their owner.
	const uint32 entryCount = static_cast<uint32>(stream->size() / sizeof(int16));
	coord.reserve(entryCount);
	for (uint32 entryIndex = 0; entryIndex < entryCount; entryIndex++)
		coord.push_back(stream->readSint16BE());

	delete stream;
	return coord;
}

void ZmbShapeOffsetRegs::parseStreams(MohawkEngine_Zoombini *vm, ZmbResource::ArchiveKind archiveKind, int16 resIdX, int16 resIdY) {
	// The paired resources are independent signed-16 axis tables.
	// Do not use this helper for a single-table or structured REGS payload.
	// Opening two @ref Common::SeekableReadStream instances for the same resource makes them interfere,
	// so we need to read the two streams separately and combine them in memory.
	_offsets.clear();
	const Common::Array<int16> coordsX = parseStream(vm->getResource(ID_REGS, ZmbResource(archiveKind, resIdX)));
	const Common::Array<int16> coordsY = parseStream(vm->getResource(ID_REGS, ZmbResource(archiveKind, resIdY)));

	if (coordsX.empty() || coordsY.empty()) {
		error("ZmbRegsOffset: required REGS coordinate arrays must not be empty");
		return;
	}
	if (coordsX.size() != coordsY.size()) {
		error("ZmbRegsOffset: coordinate arrays X and Y have different sizes (%u vs %u)", coordsX.size(), coordsY.size());
		return;
	}

	for (uint32 i = 0; i < coordsX.size(); i++) {
		_offsets.push_back(Common::Point(coordsX[i], coordsY[i]));
	}
}

Common::Point ZmbShapeOffsetRegs::getSubImageDelta(uint16 subImage) const {
	// @p subImage is zero-based, while @ref ZmbRegsOffset::_offsets is one-based.
	if (_offsets.size() <= subImage + 1u) {
		error("ZmbRegsOffset: subimage %u has no registration offset", subImage);
		return Common::Point();
	}
	return _offsets[subImage + 1];
}

Common::Point ZmbShapeOffsetRegs::getShapeDelta(uint16 shapeIdx) const {
	// @p shapeIdx and @ref ZmbRegsOffset::_offsets are both one-based.
	if (_offsets.size() <= shapeIdx) {
		error("ZmbRegsOffset: shape %u has no registration offset", shapeIdx);
		return Common::Point();
	}
	return _offsets[shapeIdx];
}

Common::Point ZmbShapeOffsetRegs::getHotspotDelta(const ZmbHotspot &hotspot) const {
	return getShapeDelta(hotspot._shapeIdx);
}

bool ZmbScriptDecoder::decodeScrbResource(MohawkEngine_Zoombini *vm, ZmbResource resource, ZmbScriptDecoder::DecodedScrb &decodedScrb) {
	decodedScrb.frames.clear();
	if (!vm) {
		warning("ZmbScript: missing engine while resolving SCRB %d", resource._id);
		return false;
	}

	Common::SeekableReadStream *stream = vm->getResource(ID_SCRB, resource);
	if (!stream)
		return false;
	const bool decoded = decodeScrbStream(stream, resource._id, decodedScrb);
	delete stream;
	return decoded;
}

bool ZmbScriptDecoder::decodeScrbStream(Common::SeekableReadStream *stream, int16 resourceId, ZmbScriptDecoder::DecodedScrb &decodedScrb) {
	decodedScrb.frames.clear();
	if (!ZmbResource::hasBytes(stream, 2)) {
		warning("ZmbScript: missing or truncated SCRB %d resource", resourceId);
		return false;
	}

	const uint16 frameCount = stream->readUint16BE();
	if (frameCount < 1 || !ZmbResource::hasBytes(stream, static_cast<int64>(frameCount) * 2)) {
		warning("ZmbScript: invalid frame count %u in SCRB %d", frameCount, resourceId);
		return false;
	}

	return decodeScriptFrames(stream, frameCount, resourceId, "SCRB", decodedScrb.frames);
}

bool ZmbScriptDecoder::decodeScrsResource(MohawkEngine_Zoombini *vm, ZmbResource resource, ZmbScriptDecoder::DecodedScrs &decodedScrs) {
	decodedScrs.frames.clear();
	decodedScrs.traitLayout = ZmbScriptDecoder::TraitLayout::kSlotOrder00;
	if (!vm) {
		warning("ZmbScript: missing engine while resolving SCRS %d", resource._id);
		return false;
	}

	Common::SeekableReadStream *stream = vm->getResource(ID_SCRS, resource);
	if (!stream)
		return false;
	const bool decoded = decodeScrsStream(stream, resource._id, decodedScrs);
	delete stream;
	return decoded;
}

bool ZmbScriptDecoder::decodeScrsStream(Common::SeekableReadStream *stream, int16 resourceId, ZmbScriptDecoder::DecodedScrs &decodedScrs) {
	decodedScrs.traitLayout = ZmbScriptDecoder::TraitLayout::kSlotOrder00;
	decodedScrs.frames.clear();
	if (!ZmbResource::hasBytes(stream, 4)) {
		warning("ZmbScript: missing or truncated SCRS %d resource", resourceId);
		return false;
	}

	const uint16 frameCount = stream->readUint16BE();
	const uint16 rawTraitLayoutCode = stream->readUint16BE();
	if (frameCount < 1 || !ZmbResource::hasBytes(stream, static_cast<int64>(frameCount) * 2)) {
		warning("ZmbScript: invalid frame count %u in SCRS %d", frameCount, resourceId);
		return false;
	}
	if (static_cast<uint16>(ZmbScriptDecoder::TraitLayout::kSlotOrder03) < rawTraitLayoutCode && rawTraitLayoutCode != UINT16_MAX) {
		warning("ZmbScript: invalid trait layout %u in SCRS %d", rawTraitLayoutCode, resourceId);
		return false;
	}
	if (rawTraitLayoutCode == UINT16_MAX)
		decodedScrs.traitLayout = ZmbScriptDecoder::TraitLayout::kInvalid;
	else
		decodedScrs.traitLayout = static_cast<ZmbScriptDecoder::TraitLayout>(static_cast<int16>(rawTraitLayoutCode));

	return decodeScriptFrames(stream, frameCount, resourceId, "SCRS", decodedScrs.frames);
}

bool ZmbScriptDecoder::decodeScriptFrames(Common::SeekableReadStream *stream, uint16 frameCount, int16 resourceId,
										  const char *resourceType, Common::Array<ZmbDecodedScriptFrame> &frames) {
	frames.clear();
	if (!stream || frameCount == 0 || !ZmbResource::hasBytes(stream, static_cast<int64>(frameCount) * 2)) {
		warning("ZmbScript: invalid or truncated %s %d stream or frame count", resourceType, resourceId);
		return false;
	}

	for (uint16 frameIdx = 0; frameIdx < frameCount; frameIdx++) {
		ZmbDecodedScriptFrame frame;
		bool foundSentinel = false;
		for (uint32 hotspotIdx = 0;; hotspotIdx += 1) {
			if (!ZmbResource::hasBytes(stream, 2))
				break;
			const int16 shapeid = stream->readSint16BE();
			if (shapeid < 0) {
				// 0xFF00: end of frame
				// 0xFFxx: end of frame, with event code in low byte
				// 0xFExx: end of frame, with event code in low byte and
				//        extra int16 (sound resource id)
				if (shapeid < -256) {
					if (!ZmbResource::hasBytes(stream, 2)) {
						warning("ZmbScript: truncated sound terminator in frame %u (%s %d)", frameIdx, resourceType, resourceId);
						frames.clear();
						return false;
					}
					frame.soundResId = stream->readSint16BE();
					frame.hasSoundRes = 0 < frame.soundResId;
				}
				if ((shapeid & 0xFF) != 0)
					frame.eventCode = static_cast<uint8>(shapeid & 0xFF);
				foundSentinel = true;
				break;
			}

			if (!ZmbResource::hasBytes(stream, 4))
				break;
			if (UINT16_MAX <= hotspotIdx) {
				warning("ZmbScript: hotspot index exceeds the serialized uint16 ID domain in frame %u (%s %d)", frameIdx, resourceType, resourceId);
				frames.clear();
				return false;
			}
			const int16 x = stream->readSint16BE();
			const int16 y = stream->readSint16BE();
			frame.hotspots.push_back(ZmbHotspot(static_cast<uint16>(hotspotIdx), shapeid, frameIdx, x, y));
		}

		if (!foundSentinel) {
			warning("ZmbScript: missing frame terminator in frame %u (%s %d)", frameIdx, resourceType, resourceId);
			frames.clear();
			return false;
		}

		frames.push_back(frame);
	}
	if (stream->pos() != stream->size()) {
		warning("ZmbScript: trailing data after the final frame in %s %d", resourceType, resourceId);
		frames.clear();
		return false;
	}

	return true;
}

ZmbHotspotGroup::~ZmbHotspotGroup() {
	clear();
}

ZmbHotspot &ZmbHotspotGroup::getHotspot(uint32 hsId) {
	static ZmbHotspot fallbackHotspot;
	if (_hotspots.size() <= hsId) {
		error("ZmbHotspotGroup: hotspot %u is out of bounds (size %u)", hsId, _hotspots.size());
		return fallbackHotspot;
	}
	return _hotspots[hsId];
}

ZmbHotspot &ZmbHotspotGroup::operator[](uint32 hsId) {
	return getHotspot(hsId);
}

void ZmbHotspotGroup::appendHotspot(const ZmbHotspot &hs) {
	_hotspots.push_back(hs);
}

void ZmbHotspotGroup::setHotspots(const Common::Array<ZmbHotspot> &hotspots) {
	_hotspots = hotspots;
}

void ZmbHotspotGroup::clear() {
	_hotspots.clear();
}

void ZmbHotspotGroup::clearScriptMetadata() {
	_soundRes = ZmbResource();
	_eventCode = 0;
}

ZmbResource ZmbFeature::resolveSoundId(int16 soundResId) const {
	return _vm->resolveLegacySoundResource(soundResId);
}
ZmbFeature::ZmbFeature(MohawkEngine_Zoombini *vm, uint16 runnerId, int16 scrbId, uint32 frameInterval, uint32 flags, ZmbResource imgResource) : _vm(vm), _id(runnerId), _scrbId(scrbId), _frameInterval(frameInterval), _flags(flags), _imgResource(imgResource) {
}

ZmbFeature::ZmbFeature(MohawkEngine_Zoombini *vm, uint16 runnerId, int16 scrbId, uint32 frameInterval, const Common::Point &pointRef, uint32 flags, ZmbResource imgResource) : _vm(vm), _id(runnerId), _scrbId(scrbId), _frameInterval(frameInterval), _pointLoc(pointRef), _flags(flags), _imgResource(imgResource) {
}

ZmbFeature::ZmbFeature(MohawkEngine_Zoombini *vm, uint16 runnerId, int16 scrbId, uint32 flags, ZmbResource imgResource) : _vm(vm), _id(runnerId), _scrbId(scrbId), _flags(flags), _imgResource(imgResource) {
}

ZmbFeature::ZmbFeature(MohawkEngine_Zoombini *vm, uint16 runnerId, int16 scrbId, uint32 flags) : _vm(vm), _id(runnerId), _scrbId(scrbId), _flags(flags) {
}

ZmbFeature::~ZmbFeature() {
	clear();
}

void ZmbFeature::initValues() {
	// Handle flags
	if (hasFlag(ZmbFeature::FLAG_00008000_LOOP_ANIM) && 0 < _frameIdxMax) {
		activateRender();
		activateAnimate();
	}

	if (hasFlag(ZmbFeature::FLAG_02000000_RANDOM_FRAME) && 0 < _frameIdxMax) {
		activateRender();
		activateAnimate();
	}

	// PLAY_ONCE stops rendering only at the end of an animation cycle.
	// The feature renders normally from the start; animation advancement is controlled
	// by other flags (LOOP_ANIM activates it, DEFER_ANIM defers it).
	// Keep animation active here.
	// Calling @ref ZmbFeature::deactivateAnimate() would break the common LOOP_ANIM and PLAY_ONCE combination.
	// Bridge, Xfer, RodMap, and Basecamp 2 use that combination.
	if (hasFlag(ZmbFeature::FLAG_00100000_PLAY_ONCE)) {
		activateRender();
	}

	if (hasFlag(ZmbFeature::FLAG_00020000_SKIP_RENDER)) {
		deactivateRender();
	}

	// A newly registered runner materializes frame zero before DEFER_ANIM suppresses subsequent pre-render processing.
	// Runtime SCRB loads have no pending initial pass and apply the dormant state immediately.
	if (hasFlag(ZmbFeature::FLAG_00080000_DEFER_ANIM)) {
		deactivateAnimate();
		if (!_initialScrbLoadPending)
			deactivateRender();
	}

	if (hasFlag(ZmbFeature::FLAG_01000000_DEFER_RENDER) && !_initialScrbLoadPending) {
		deactivateRender();
	}

	// Initialize derived hotspot and animation state.
	if (0 < getFrameCount()) {
		_hasClickRect = false;
		_hasExplicitClickRect = false;
		_skipFirstAdvance = 1;
		if (hasFlag(ZmbFeature::FLAG_00800000_POS_DELTA)) {
			// @ref ZmbFeature::FLAG_00800000_POS_DELTA stores the first hotspot in @ref ZmbFeature::_pointRef.
			// @ref ZmbFeature::_pointLoc retains the requested layout position.
			// The feature's delta moves the hotspot there.
			ZmbHotspotGroup *firstGroup = getHotspotGroup(0);
			if (!firstGroup || firstGroup->getHotspotCount() == 0) {
				error("ZmbFeature: missing anchor hotspot for POS_DELTA SCRB %d", _scrbId);
				return;
			}
			_pointRef = firstGroup->getHotspot(0).getPos();
		}
	}
}

void ZmbFeature::setEventHooks(const EventHooks &hooks) {
	_eventHooks = hooks;
	_usesDefaultRenderFunc = !_eventHooks._renderFunc ||
							 _eventHooks._renderFunc == &ZoombiniPage::blitShapes;
	if (!_eventHooks._renderFunc)
		_eventHooks._renderFunc = &ZoombiniPage::blitShapes;
	if (!_eventHooks._selectRenderFrameFunc)
		_eventHooks._selectRenderFrameFunc = &ZoombiniPage::selectRenderFrame;
}

void ZmbFeature::onPreRender(ZoombiniPage *page) {
	if (!page)
		return;

	// Run the custom pre-render callback first, then continue through @ref ZoombiniPage::preRenderFeature().
	if (_eventHooks._preRenderFunc) {
		if ((page->*(_eventHooks._preRenderFunc))(this) == false)
			return;
	}

	// Snoids use their dedicated pre-render path.
	// Test the concrete runner type because Town removes the TYPE_SNOID flag without changing that behavior.
	ZmbSnoid *snoid = dynamic_cast<ZmbSnoid *>(this);
	if (snoid) {
		page->preRenderSnoid(snoid);
		return;
	}

	page->preRenderFeature(this);
}

ZmbRenderResult ZmbFeature::onPostRender(ZoombiniPage *page) {
	if (!page)
		return ZmbRenderResult::kSkipped;

	// Blit shapes through the selected render callback, then run the custom post-render callback.
	// Use @ref ZoombiniPage::blitShapes() when no custom render callback is installed.
	OnRenderFunc renderFunc = _eventHooks._renderFunc;
	if (!renderFunc)
		renderFunc = &ZoombiniPage::blitShapes;
	ZmbRenderResult ret = (page->*renderFunc)(this);
	if (ret == ZmbRenderResult::kRendered && _eventHooks._postRenderFunc)
		(page->*(_eventHooks._postRenderFunc))(this);
	return ret;
}

int32 ZmbFeature::onSelectRenderFrame(ZoombiniPage *page) {
	if (!_eventHooks._selectRenderFrameFunc || !page)
		return 0;
	return (page->*(_eventHooks._selectRenderFrameFunc))(this);
}

void ZmbFeature::onPreRenderShape(ZoombiniPage *page, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	if (!_eventHooks._preRenderShapeFunc || !page)
		return;
	(page->*(_eventHooks._preRenderShapeFunc))(this, hsGroup, hotspots);
}

ZmbEventHandleResult ZmbFeature::onMouseMove(ZoombiniPage *page, const Common::Point &absPos, const Common::Point &relPos) {
	if (!_eventHooks._mouseMoveFunc || !page)
		return ZmbEventHandleResult::kPassthrough;
	return (page->*(_eventHooks._mouseMoveFunc))(this, absPos, relPos);
}

ZmbEventHandleResult ZmbFeature::onLButtonDown(ZoombiniPage *page, const Common::Point &absPos, const Common::Point &relPos) {
	if (!_eventHooks._lButtonDownFunc || !page)
		return ZmbEventHandleResult::kPassthrough;
	return (page->*(_eventHooks._lButtonDownFunc))(this, absPos, relPos);
}

ZmbEventHandleResult ZmbFeature::onLButtonUp(ZoombiniPage *page, const Common::Point &absPos, const Common::Point &relPos) {
	if (!_eventHooks._lButtonUpFunc || !page)
		return ZmbEventHandleResult::kPassthrough;
	return (page->*(_eventHooks._lButtonUpFunc))(this, absPos, relPos);
}

ZmbEventHandleResult ZmbFeature::onKeyDown(ZoombiniPage *page, const Common::KeyState &kbd, bool kbdRepeat) {
	if (!_eventHooks._keyDownFunc || !page)
		return ZmbEventHandleResult::kPassthrough;
	return (page->*(_eventHooks._keyDownFunc))(this, kbd, kbdRepeat);
}

ZmbEventHandleResult ZmbFeature::onKeyUp(ZoombiniPage *page, const Common::KeyState &kbd, bool kbdRepeat) {
	if (!_eventHooks._keyUpFunc || !page)
		return ZmbEventHandleResult::kPassthrough;
	return (page->*(_eventHooks._keyUpFunc))(this, kbd, kbdRepeat);
}

ZmbEventHandleResult ZmbFeature::onWheelUp(ZoombiniPage *page, const Common::Point &absPos) {
	if (!_eventHooks._wheelUpFunc || !page)
		return ZmbEventHandleResult::kPassthrough;
	return (page->*(_eventHooks._wheelUpFunc))(this, absPos);
}

ZmbEventHandleResult ZmbFeature::onWheelDown(ZoombiniPage *page, const Common::Point &absPos) {
	if (!_eventHooks._wheelDownFunc || !page)
		return ZmbEventHandleResult::kPassthrough;
	return (page->*(_eventHooks._wheelDownFunc))(this, absPos);
}

bool ZmbFeature::setDecodedScrb(const ZmbScriptDecoder::DecodedScrb *decodedScrb, int16 scrbId) {
	if (!decodedScrb)
		return false;

	clear();
	_lastFrameIdx = 0;
	_frameIdxMax = 0;
	_lastShapeFrameIdx = 0;
	_lastSoundedFrameIdx = -1;
	bindDecodedFrames(&decodedScrb->frames);
	setScrbId(scrbId);
	return true;
}

void ZmbFeature::loadScrbData(const ZmbScriptDecoder::DecodedScrb *decodedScrb, int16 scrbId, bool scheduleRender) {
	// Swap SCRB data on an existing runner.
	// Clear existing script data and draw records, bind the cached SCRB, reset animation state,
	// and re-runs @ref ZmbFeature::initValues().
	// Preserve the runner identity, flags, callbacks, and position reference.

	// Increment the generation counter.
	// The PLAY_ONCE handler in @ref ZoombiniPage::preRenderFeature() uses this counter.
	// It detects a new SCRB loaded during the end callback.
	// This avoids a stale @ref ZmbFeature::markAnimEndCallbackFired().
	_scrbLoadGeneration += 1;
	const bool retainPreparedVisualRects = hasPreparedVisualRects();
	Common::Array<Common::Rect> retainedPreparedVisualRects;
	if (retainPreparedVisualRects)
		collectPreparedVisualRects(retainedPreparedVisualRects);

	const bool retainPreparedRenderFrame = hasPreparedRenderHotspots();
	Common::Array<ZmbPreparedRenderHotspot> retainedPreparedRenderHotspots;
	if (retainPreparedRenderFrame) {
		retainedPreparedRenderHotspots = getPreparedRenderHotspots();
		for (uint32 i = 0; i < retainedPreparedRenderHotspots.size(); i++)
			retainedPreparedRenderHotspots[i]._hsGroup = nullptr;
	}

	// Match a newly loaded runner's ownership boundary, including replacement of an existing chained sub-feature.
	clear();
	_initialScrbLoadPending = false;
	_visualRematerializationRequested = false;

	// Reset animation state
	_lastFrameIdx = 0;
	_frameIdxMax = 0;
	_lastShapeFrameIdx = 0;
	_lastSoundedFrameIdx = -1;
	_nextRenderFrame = 0;
	_frameTimingReady = true;

	_hasClickRect = false;
	_skipFirstAdvance = true;

	// Bind the cached immutable SCRB data. Rendering materializes only the selected frame.
	if (!decodedScrb) {
		error("ZmbFeature: required SCRB %d is malformed", scrbId);
		return;
	}
	bindDecodedFrames(&decodedScrb->frames);
	setScrbId(scrbId);

	// Re-initialize flag-dependent state
	initValues();

	// @p scheduleRender assigns the render state; it is not a true-only activation flag.
	// Callers pass false to install a dormant replacement SCRB without rendering, advancing, or dispatching events.
	// Mark the runner dirty immediately after another callback loads it.
	// This contributes coverage in the current post-render pass before the runner's next pre-render.
	_needsRedraw = true;
	if (scheduleRender) {
		activateRender();
		activateAnimate();
	} else {
		deactivateRender();
	}

	// A reload may occur after this runner's pre-render turn.
	// Keep its already materialized pixels for the current post-render pass.
	// The next pre-render replaces this detached payload with the new SCRB frame.
	if (retainPreparedVisualRects)
		setPreparedVisualRects(retainedPreparedVisualRects);
	if (retainPreparedRenderFrame)
		setPreparedRenderHotspots(retainedPreparedRenderHotspots);
}

bool ZmbFeature::setDecodedScrs(const ZmbScriptDecoder::DecodedScrs *decodedScrs) {
	if (!decodedScrs)
		return false;

	clear();
	_lastFrameIdx = 0;
	_frameIdxMax = 0;
	_lastShapeFrameIdx = 0;
	_lastSoundedFrameIdx = -1;
	bindDecodedFrames(&decodedScrs->frames);
	setScrbId(0);
	return true;
}

bool ZmbFeature::getFrameEventCode(int32 frameIdx, uint8 &eventCode) const {
	const ZmbDecodedScriptFrame *frame = getDecodedScriptFrame(frameIdx);
	if (frame) {
		if (frame->eventCode == 0)
			return false;
		eventCode = frame->eventCode;
		return true;
	}

	ZmbHotspotGroup *group = getHotspotGroupExact(frameIdx);
	if (!group || !group->hasAssignedEventCode())
		return false;
	eventCode = group->getAssignedEventCode();
	return true;
}

bool ZmbFeature::getFrameSoundResource(int32 frameIdx, ZmbResource &resource) const {
	const ZmbDecodedScriptFrame *frame = getDecodedScriptFrame(frameIdx);
	if (frame) {
		if (!frame->hasSoundRes)
			return false;
		resource = resolveSoundId(frame->soundResId);
		return true;
	}

	ZmbHotspotGroup *group = getHotspotGroupExact(frameIdx);
	if (!group || !group->hasAssignedSoundRes())
		return false;
	resource = group->getAssignedSoundRes();
	return true;
}

bool ZmbFeature::playOwnedSound(ZmbResource resource, Audio::Mixer::SoundType soundType) const {
	// Reuse an already playing SND instead of layering a second instance when another script reaches the same event.
	if (!_vm->_sound->isSoundPlaying(resource)) {
		Audio::SoundHandle *handle = _vm->_sound->playSound(resource, soundType);
		if (handle)
			_frameSoundHandles[resource._id] = *handle;
	}
	return true;
}

bool ZmbFeature::playFrameSound(int32 frameIdx) const {
	ZmbResource resource;
	if (!getFrameSoundResource(frameIdx, resource))
		return false;

	return playOwnedSound(resource);
}

bool ZmbFeature::enqueueFrameSound(int32 frameIdx, bool forcePriority) const {
	ZmbResource resource;
	if (!getFrameSoundResource(frameIdx, resource))
		return false;

	// A feature-level opt-in routes its SFX through the priority queue.
	// Immediate page feedback remains outside that arbitration.
	_vm->_sound->enqueueScriptSound(resource, Audio::Mixer::kSFXSoundType, forcePriority);
	return true;
}

void ZmbFeature::setScriptSoundPolicy(ScriptSoundPolicy policy) {
	_scriptSoundPolicy = policy;
	if (_refSubFeature)
		_refSubFeature->setScriptSoundPolicy(policy);
}

void ZmbFeature::setSubFeature(ZmbFeature *subFeature) {
	_refSubFeature = subFeature;
	if (_refSubFeature)
		_refSubFeature->setScriptSoundPolicy(_scriptSoundPolicy);
}

void ZmbFeature::stopFrameSounds() const {
	// The original engine has one active SND slot, but each runner owns its mixer instances here.
	// Stop only this runner's mixer instances so unrelated mixed SFX remain active.
	// Do not gate this on @ref Audio::Mixer::isSoundHandleActive().
	// A click may unload a runner before the mixer thread marks a new stream active.
	for (Common::HashMap<int16, Audio::SoundHandle>::iterator it = _frameSoundHandles.begin(); it != _frameSoundHandles.end(); it++)
		_vm->_mixer->stopHandle(it->_value);
	_frameSoundHandles.clear();
}

void ZmbFeature::bindDecodedFrames(const Common::Array<ZmbDecodedScriptFrame> *frames) {
	_activeDecodedFrames = frames;
	delete _materializedScriptFrame;
	_materializedScriptFrame = nullptr;
	_frameIdxMax = frames && !frames->empty() ? static_cast<int32>(frames->size()) - 1 : 0;
	_lastShapeFrameIdx = 0;

	if (!frames)
		return;

	for (uint32 frameIdx = 0; frameIdx < frames->size(); frameIdx++) {
		const Common::Array<ZmbHotspot> &hotspots = (*frames)[frameIdx].hotspots;
		for (uint32 hotspotIdx = 0; hotspotIdx < hotspots.size(); hotspotIdx++) {
			// @ref ZmbFeature::_lastShapeFrameIdx tracks the last frame with a positive-shape hotspot.
			// @ref ZmbFeature::FLAG_00100000_PLAY_ONCE freezes there so the runner settles on visible shapes
			// instead of a trailing terminator-only frame or a stale previous-frame fallback.
			if (0 < hotspots[hotspotIdx]._shapeIdx)
				_lastShapeFrameIdx = static_cast<int32>(frameIdx);
		}
	}
}

ZmbHotspotGroup *ZmbFeature::materializeDecodedFrame(int32 frameIdx) const {
	const ZmbDecodedScriptFrame *frame = getDecodedScriptFrame(frameIdx);
	if (!frame)
		return nullptr;

	if (!_materializedScriptFrame)
		_materializedScriptFrame = new ZmbHotspotGroup(_id, frameIdx);
	else
		_materializedScriptFrame->_frameIdx = frameIdx;

	_materializedScriptFrame->setHotspots(frame->hotspots);
	_materializedScriptFrame->clearScriptMetadata();
	if (frame->hasSoundRes)
		_materializedScriptFrame->assignSoundRes(resolveSoundId(frame->soundResId));
	if (frame->eventCode != 0)
		_materializedScriptFrame->assignEventCode(frame->eventCode);
	return _materializedScriptFrame;
}

const ZmbDecodedScriptFrame *ZmbFeature::getDecodedScriptFrame(int32 frameIdx) const {
	if (!_activeDecodedFrames || frameIdx < 0 || _activeDecodedFrames->size() <= static_cast<uint32>(frameIdx))
		return nullptr;
	return &(*_activeDecodedFrames)[frameIdx];
}

void ZmbFeature::setVirtualHotspots(const Common::Array<ZmbHotspot> &hotspots) {
	clear();

	ZmbHotspotGroup *hsGroup = new ZmbHotspotGroup(_id, 0);
	_virtualFrameMap[0] = hsGroup;
	_frameIdxMax = 0;
	_lastShapeFrameIdx = 0;
	_lastFrameIdx = 0;
	_isAnimateActivated = false;

	hsGroup->setHotspots(hotspots);

	debug(3, "scrb: set virtual feature with %u hotspots", hotspots.size());
}
ZmbHotspotGroup *ZmbFeature::getHotspotGroupExact(int32 frameId) const {
	if (frameId < 0)
		return nullptr;
	if (_activeDecodedFrames)
		return materializeDecodedFrame(frameId);
	Common::HashMap<int32, ZmbHotspotGroup *>::const_iterator it = _virtualFrameMap.find(frameId);
	if (it == _virtualFrameMap.end())
		return nullptr;
	return it->_value;
}

ZmbHotspotGroup *ZmbFeature::getHotspotGroup(int32 frameId) {
	if (frameId < 0)
		return nullptr;
	if (_activeDecodedFrames) {
		const int32 lastFrameIdx = MIN(frameId, _frameIdxMax);
		for (int32 visualFrameIdx = lastFrameIdx; 0 <= visualFrameIdx; visualFrameIdx--) {
			const ZmbDecodedScriptFrame *frame = getDecodedScriptFrame(visualFrameIdx);
			if (!frame || frame->hotspots.empty())
				continue;
			return materializeDecodedFrame(visualFrameIdx);
		}
		return nullptr;
	}

	ZmbHotspotGroup *hsGroup = nullptr;

	Common::HashMap<int32, ZmbHotspotGroup *>::iterator it = _virtualFrameMap.find(frameId);
	if (it != _virtualFrameMap.end())
		hsGroup = it->_value;

	// Keep materialized hotspots as persistent draw records.
	// A terminator-only frame dispatches its event without clearing them;
	// a frame beyond the script likewise leaves the last materialized shapes in place.
	// Preserve an explicit empty frame's events through @ref ZmbFeature::getHotspotGroupExact(),
	// but render the closest preceding non-empty frame here.
	if ((!hsGroup || hsGroup->getHotspotCount() == 0) && 0 < frameId) {
		const int32 fallbackStart = MIN(frameId - 1, _frameIdxMax);
		for (int32 altFrameId = fallbackStart; 0 <= altFrameId; altFrameId--) {
			it = _virtualFrameMap.find(altFrameId);
			if (it != _virtualFrameMap.end() && 0 < it->_value->getHotspotCount()) {
				hsGroup = it->_value;
				break;
			}
		}
	}

	if (!hsGroup)
		return nullptr;

	return hsGroup;
}

ZmbHotspotGroup *ZmbFeature::getCurrentScriptVisualFrame() {
	return getHotspotGroup(_lastFrameIdx);
}

uint32 ZmbFeature::getHotspotTotalCount() const {
	if (_activeDecodedFrames) {
		uint32 count = 0;
		for (uint32 frameIdx = 0; frameIdx < _activeDecodedFrames->size(); frameIdx++)
			count += (*_activeDecodedFrames)[frameIdx].hotspots.size();
		return count;
	}

	uint32 count = 0;
	Common::HashMap<int32, ZmbHotspotGroup *>::const_iterator it = _virtualFrameMap.begin();
	for (; it != _virtualFrameMap.end(); it++) {
		ZmbHotspotGroup *hsGroup = it->_value;
		count += hsGroup->getHotspotCount();
	}
	return count;
}

uint16 ZmbFeature::getHotspotIdCount() const {
	if (_activeDecodedFrames)
		return static_cast<uint16>(_activeDecodedFrames->size());
	return _virtualFrameMap.size();
}

int32 ZmbFeature::defaultSelectRenderFrame(uint32 currentFrameCounter) {
	// The next-render deadline gates the entire pre-render body.
	// @ref ZmbFeature::_frameTimingReady propagates the result to @ref ZoombiniPage::preRenderFeature().

	// RANDOM_FRAME: pick a random frame index each qualifying tick.
	if (hasFlag(ZmbFeature::FLAG_02000000_RANDOM_FRAME)) {
		_frameTimingReady = getFrameTimingResult(currentFrameCounter);
		if (_frameTimingReady) {
			_nextRenderFrame = currentFrameCounter + _frameInterval;
			_lastFrameIdx = _vm->_rnd->getRandomNumber(_frameIdxMax);
		}
		return _lastFrameIdx;
	}

	// DEFER_ANIM remains dormant until @ref ZmbFeature::activateAnimate() is called.
	if (hasFlag(ZmbFeature::FLAG_00080000_DEFER_ANIM) && !_isAnimateActivated)
		return 0;

	if (isAnimateActivated()) {
		// Apply the next-render deadline gate.
		_frameTimingReady = getFrameTimingResult(currentFrameCounter);
		if (_frameTimingReady) {
			_nextRenderFrame = currentFrameCounter + _frameInterval;

			// End-of-cycle checking and frame advancement are mutually exclusive.
			// @ref ZmbFeature::_skipFirstAdvance applies only before the cycle boundary.
			// The boundary occurs when @ref ZmbFeature::_lastFrameIdx reaches @ref ZmbFeature::_frameIdxMax.
			// At or past the maximum, advance once to signal the end to @ref ZoombiniPage::preRenderFeature().
			if (_lastFrameIdx < _frameIdxMax) {
				// Skip first frame advance after SCRB load
				if (_skipFirstAdvance) {
					_skipFirstAdvance = false;
				} else {
					_lastFrameIdx += 1;
				}
			} else {
				// Advance past @ref ZmbFeature::_frameIdxMax to trigger end-of-cycle detection.
				_lastFrameIdx += 1;
			}
		}
	}
	return _lastFrameIdx;
}

Common::Point ZmbFeature::getPosDelta() const {
	if (!hasFlag(ZmbFeature::FLAG_00800000_POS_DELTA))
		return Common::Point(0, 0);
	return _pointLoc - _pointRef;
}

ZmbDrawRecord *ZmbFeature::setDrawRecord(ZmbHotspotGroup *hsGroup, const ZmbHotspot &hs, const Common::Rect &drawnRect) {
	ZmbDrawRecord *record = new ZmbDrawRecord(this, hsGroup, hs, drawnRect);
	uint32 h = hs.hash();
	Common::StableMap<uint32, ZmbDrawRecord *>::iterator existing = _drawnRecordMap.find(h);
	if (existing != _drawnRecordMap.end())
		delete existing->second;
	_drawnRecordMap[h] = record;
	return record;
}

ZmbDrawRecord *ZmbFeature::getDrawRecord(uint16 frame, uint16 hsIdx) {
	uint32 hash = ZmbHotspot::hash(frame, hsIdx);
	Common::StableMap<uint32, ZmbDrawRecord *>::iterator it = _drawnRecordMap.find(hash);
	if (it == _drawnRecordMap.end())
		return nullptr;

	return it->second;
}

void ZmbFeature::eraseDrawRecord(uint16 frame, uint16 hsIdx) {
	uint32 hash = ZmbHotspot::hash(frame, hsIdx);
	Common::StableMap<uint32, ZmbDrawRecord *>::iterator it = _drawnRecordMap.find(hash);
	if (it == _drawnRecordMap.end())
		return;

	ZmbDrawRecord *record = it->second;
	_drawnRecordMap.erase(it);
	delete record;
}

void ZmbFeature::clearDrawRecords() {
	for (Common::StableMap<uint32, ZmbDrawRecord *>::iterator it = _drawnRecordMap.begin(); it != _drawnRecordMap.end(); it++) {
		delete it->second;
	}
	_drawnRecordMap.clear();
}

void ZmbFeature::collectDrawRecordRects(Common::Array<Common::Rect> &rects) const {
	for (Common::StableMap<uint32, ZmbDrawRecord *>::const_iterator it = _drawnRecordMap.begin(); it != _drawnRecordMap.end(); it++) {
		const Common::Rect &rect = it->second->_drawnRect;
		if (!rect.isEmpty())
			rects.push_back(rect);
	}
}

ZmbDrawRecord *ZmbFeature::findDrawRecordAtPoint(const Common::Point &absPos) {
	for (Common::StableMap<uint32, ZmbDrawRecord *>::iterator it = _drawnRecordMap.begin(); it != _drawnRecordMap.end(); it++) {
		ZmbDrawRecord *record = it->second;
		if (record->_drawnRect.contains(absPos))
			return record;
	}
	return nullptr;
}

void ZmbFeature::findDrawRecordsAtPoint(const Common::Point &absPos, Common::Array<ZmbDrawRecord *> &foundRecords) {
	for (Common::StableMap<uint32, ZmbDrawRecord *>::iterator it = _drawnRecordMap.begin(); it != _drawnRecordMap.end(); it++) {
		ZmbDrawRecord *record = it->second;
		if (record->_drawnRect.contains(absPos))
			foundRecords.push_back(record);
	}
}

ZmbDrawRecord *ZmbFeature::findDrawRecordByHotspotIdx(uint16 hsIdx) {
	for (Common::StableMap<uint32, ZmbDrawRecord *>::iterator it = _drawnRecordMap.begin(); it != _drawnRecordMap.end(); it++) {
		ZmbDrawRecord *record = it->second;
		if (record->_hs._hsId == hsIdx)
			return record;
	}
	return nullptr;
}

ZmbDrawRecord *ZmbFeature::findDrawRecordByHotspotIdx(uint16 hsIdx1, uint16 hsIdx2) {
	for (Common::StableMap<uint32, ZmbDrawRecord *>::iterator it = _drawnRecordMap.begin(); it != _drawnRecordMap.end(); it++) {
		ZmbDrawRecord *record = it->second;
		if (record->_hs._hsId == hsIdx1 || record->_hs._hsId == hsIdx2)
			return record;
	}
	return nullptr;
}

ZmbDrawRecord *ZmbFeature::findDrawRecordByHotspotIdx(Common::Array<uint16> hsIdxArr) {
	for (Common::StableMap<uint32, ZmbDrawRecord *>::iterator it = _drawnRecordMap.begin(); it != _drawnRecordMap.end(); it++) {
		ZmbDrawRecord *record = it->second;

		for (uint32 i = 0; i < hsIdxArr.size(); i++) {
			if (record->_hs._hsId == hsIdxArr[i])
				return record;
		}
	}
	return nullptr;
}

ZmbDrawRecord *ZmbFeature::findDrawRecordByShapeId(uint16 shapeId) {
	for (Common::StableMap<uint32, ZmbDrawRecord *>::iterator it = _drawnRecordMap.begin(); it != _drawnRecordMap.end(); it++) {
		ZmbDrawRecord *record = it->second;
		if (record->_hs._shapeIdx == shapeId)
			return record;
	}
	return nullptr;
}

ZmbDrawRecord *ZmbFeature::findDrawRecordByShapeId(uint16 shapeId1, uint16 shapeId2) {
	for (Common::StableMap<uint32, ZmbDrawRecord *>::iterator it = _drawnRecordMap.begin(); it != _drawnRecordMap.end(); it++) {
		ZmbDrawRecord *record = it->second;
		if (record->_hs._shapeIdx == shapeId1 || record->_hs._shapeIdx == shapeId2)
			return record;
	}
	return nullptr;
}

ZmbDrawRecord *ZmbFeature::findDrawRecordByShapeId(Common::Array<uint16> shapeIdArr) {
	for (Common::StableMap<uint32, ZmbDrawRecord *>::iterator it = _drawnRecordMap.begin(); it != _drawnRecordMap.end(); it++) {
		ZmbDrawRecord *record = it->second;

		for (uint32 shapeIdx = 0; shapeIdx < shapeIdArr.size(); shapeIdx++) {
			if (record->_hs._shapeIdx == shapeIdArr[shapeIdx])
				return record;
		}
	}
	return nullptr;
}

void ZmbFeature::activateSubFeature() {
	;
}

void ZmbFeature::clear() {
	clearDrawRecords();
	clearPreparedVisualRects();
	clearPreparedRenderHotspots();
	_visualRematerializationRequested = false;

	if (_refSubFeature) {
		delete _refSubFeature;
		_refSubFeature = nullptr;
	}

	for (Common::HashMap<int32, ZmbHotspotGroup *>::iterator it = _virtualFrameMap.begin(); it != _virtualFrameMap.end(); it++) {
		delete it->_value;
	}
	_virtualFrameMap.clear();
	_activeDecodedFrames = nullptr;
	delete _materializedScriptFrame;
	_materializedScriptFrame = nullptr;
}

void ZmbFeature::activateAnimate() {
	_isAnimateActivated = true;
	_animEndCallbackFired = false;
	_frameTimingReady = true;
	_lastSoundedFrameIdx = -1;
	scheduleAnimateForFrames(_frameIdxMax);
}

void ZmbFeature::deactivateAnimate() {
	_isAnimateActivated = false;
}

void ZmbFeature::setSelectRenderFrameFunc(OnSelectRenderFrameFunc func) {
	_eventHooks._selectRenderFrameFunc = func;
}

bool ZmbFeature::isAnimateActivated() const {
	return _isAnimateActivated;
}

void ZmbFeature::scheduleAnimateForFrames(uint16 frames) {
	// Start at first frame.
	_lastFrameIdx = 0;
	_frameIdxMax = frames;
}

bool ZmbFeature::isAnimationCycleRunning() const {
	if (!isAnimateActivated())
		return false;
	if (hasFlag(ZmbFeature::FLAG_00008000_LOOP_ANIM))
		return true;
	return _lastFrameIdx <= _frameIdxMax;
}

bool ZmbFeature::isEndOfAnimationCycle() const {
	if (!isAnimateActivated())
		return false;
	// End-of-cycle fires when the group frame index reaches the declared frame count.
	// @ref ZmbFeature::defaultSelectRenderFrame() advances @ref ZmbFeature::_lastFrameIdx.
	// Passing @ref ZmbFeature::_frameIdxMax signals the boundary.
	// The last valid frame is displayed on the previous tick, so the boundary fires one tick later.
	return _frameIdxMax < _lastFrameIdx;
}

void ZmbFeature::runSubFeature(ZoombiniPage *page) {
	assert(hasFlag(ZmbFeature::FLAG_00040000_CHAIN_SCRIPT));

	if (!_refSubFeature)
		return;

	// Do nothing if the sub-feature is already running in the page's feature list
	if (_refSubFeature->isSubFeatureRunning())
		return;

	// Reset and start the sub-feature's animation from the beginning
	_refSubFeature->activateRender();
	_refSubFeature->activateAnimate();

	// Register it into the page's active scrb feature map for independent rendering.
	// Ownership stays with this parent feature; the page will detach (not delete) it
	// once the animation cycle ends.
	_refSubFeature->setSubFeatureRunning(true);
	page->attachSubFeature(_refSubFeature);
}

ZmbSnoid::ZmbSnoid(MohawkEngine_Zoombini *vm, uint16 snoidId, uint32 flags) : _vm(vm), _id(snoidId), ZmbFeature(vm, snoidId, 0, flags) {
	assert(hasFlag(FLAG_00000001_TYPE_SNOID));
	setFrameInterval(6);
	setScriptSoundPolicy(ScriptSoundPolicy::kPriorityQueue);
}

ZmbSnoid::~ZmbSnoid() {
}

Common::String ZmbSnoid::toStr(bool showPosition) const {
	if (!showPosition)
		return Common::String::format("Zoombini: %s", _trait.toStr().c_str());

	const Common::Point point = getPointLoc();
	return Common::String::format("Zoombini (%3d, %3d): %s", point.x, point.y, _trait.toStr().c_str());
}

bool ZmbSnoid::loadScrsResource(ZmbResource resource) {
	if (!_vm)
		return false;

	const ZmbScriptDecoder::DecodedScrs *decodedScrs = nullptr;
	ZoombiniPage *page = _vm->getActivePage();
	if (resource._archiveKind == ZmbResource::kSystem)
		decodedScrs = _vm->getSystemDecodedScrs(resource._id);
	else if (page)
		decodedScrs = page->getDecodedScrs(resource);
	if (!setDecodedScrs(decodedScrs))
		return false;
	_traitLayout = decodedScrs->traitLayout;
	return true;
}

bool ZmbSnoid::startScrsPlayback(ZmbResource resource, ZmbScrsCompletionMode completionMode,
								 ZmbScrsPlaybackMode playbackMode, const Common::Point *initPos, bool suppressVoiceEvents) {
	// Save the pre-SCRS position for restoration when playback finishes.
	_scrsOrigPointLoc = getPointLoc();
	_scrsVoiceEventsSuppressed = suppressVoiceEvents;
	_scrsEndEventEnabled = true;
	clearPreparedRenderHotspots();

	// Bind SCRS data; @ref ZmbFeature::clear() removes prior idle or SCRS hotspot data.
	if (!loadScrsResource(resource)) {
		error("ZmbSnoid: required SCRS %d is malformed", resource._id);
		return false;
	}
	_activeScrsId = resource._id;

	// Parsed SCRS shape indices remain raw and therefore need @ref ZmbSnoid::getBodyLayerBaseOffset().
	_usesVirtualHotspots = false;

	// Without initPos, anchor the first positive hotspot in frame 0 to the current feature position.
	// With initPos, scan backward for the final frame with a positive hotspot and anchor that frame to initPos,
	// so the animation ends at the requested point.
	// Rendering subtracts the resulting anchor offset and the REGS correction from each SCRS coordinate.
	const ZmbDecodedScriptFrame *anchorFrame = nullptr;
	Common::Point anchorRefPoint = _scrsOrigPointLoc;
	_scrsRenderOffset = _scrsOrigPointLoc;
	if (initPos) {
		anchorRefPoint = *initPos;
		// Scan downward for the highest frame whose first hotspot has a positive shape.
		for (int32 frameId = static_cast<int32>(getFrameCount()) - 1; 0 <= frameId; frameId--) {
			const ZmbDecodedScriptFrame *candidate = getDecodedScriptFrame(frameId);
			if (!candidate || candidate->hotspots.empty())
				continue;
			if (0 < candidate->hotspots[0]._shapeIdx) {
				anchorFrame = candidate;
				break;
			}
		}
	}
	if (!anchorFrame) {
		// Without a final-frame anchor, use frame 0's first positive hotspot.
		anchorFrame = getDecodedScriptFrame(0);
	}
	if (anchorFrame) {
		for (uint32 hotspotIdx = 0; hotspotIdx < anchorFrame->hotspots.size(); hotspotIdx++) {
			const ZmbHotspot &hotspot = anchorFrame->hotspots[hotspotIdx];
			if (0 < hotspot._shapeIdx) {
				_scrsRenderOffset = Common::Point(anchorRefPoint.x - hotspot._x, anchorRefPoint.y - hotspot._y);
				break;
			}
		}
	}

	// Preserve the facing direction from before playback.
	// @p completionMode independently controls whether the Snoid is hidden when the SCRS ends.
	_scrsCompletionMode = completionMode;

	if (playbackMode == ZmbScrsPlaybackMode::kReject)
		_animState = kSnoidAnimState008_ScriptReject;
	else
		_animState = kSnoidAnimState009_ScriptNormal;
	_scrsAnimCycleCount = 0;
	_scrsJustStarted = true;
	// Enable rendering before the first frame; the previous script may have hidden this Snoid.
	activateRender();

	setLastFrameIdx(0);
	setLastSoundedFrameIdx(-1);
	syncScrsPointLoc();
	activateAnimate();
	return true;
}

void ZmbSnoid::syncScrsPointLoc() {
	const ZmbDecodedScriptFrame *frame = getDecodedScriptFrame(getLastFrameIdx());
	if (!frame || frame->hotspots.empty())
		return;

	// Update the feature position only when the first entry in the current SCRS frame has a positive shape ID.
	const ZmbHotspot &anchor = frame->hotspots[0];
	if (0 < anchor._shapeIdx) {
		setPointLoc(Common::Point(_scrsRenderOffset.x + anchor._x, _scrsRenderOffset.y + anchor._y));
	}
}

void ZmbSnoid::finishScrsPlayback(bool restorePosition) {
	if (restorePosition)
		setPointLoc(_scrsOrigPointLoc);
	clearPreparedRenderHotspots();
	_scrsRenderOffset = Common::Point(0, 0);
	_activeScrsId = 0;
	_scrsVoiceEventsSuppressed = false;
	deactivateAnimate();
}

void ZmbSnoid::setAnimState(SnoidAnimState state, const Common::Point *pos) {
	const SnoidAnimState previousState = _animState;

	// Clamp unknown states to idle
	if (kSnoidAnimState010_ArrivalMotion < state && state != kSnoidAnimState112_Path)
		state = kSnoidAnimState000_Idle;

	// Enable rendering for every state.
	// A hide-on-complete SCRS callback may start another state.
	// Tunnels does this with state 10 for an accepted Snoid.
	activateRender();

	// Reset walk animation cycle when entering a walking state
	if (state == kSnoidAnimState007_Depart) {
		_walkPhase = 0;
		// If departing and target is to the right, ensure facing right for first walk frame
		if (pos) {
			if (getPointLoc().x < pos->x)
				_isFacingLeft = false;
			else if (pos->x < getPointLoc().x)
				_isFacingLeft = true;
		}
	}

	// Entry from a non-idle state preserves @ref ZmbSnoid::_shapeImageIdx and the facing direction.
	// It then materializes that common-image pose.
	// SCRS completion uses this path.
	// A selector of 0 or 2 after a fidget or drag must not be forced to selector 1.
	// A transition while already idle normalizes the selector to 1 during idle or flip initialization.
	if (state == kSnoidAnimState000_Idle) {
		_needsIdleRedraw = true;
		if (previousState != kSnoidAnimState000_Idle) {
			setupCurrentCommonImageHotspots();
		} else {
			_shapeImageIdx = 1;
			if (_useSmallShapeRegs)
				setupSmallIdleHotspots();
			else
				setupIdleHotspots();
		}
	} else if (state == kSnoidAnimState004_Arrive) {
		// Rebuild draw records from the current common-image pose (SCRS 100/101/102).
		// The first post-drop frame must use the seated/common pose, not a dangling drag sub-pose left in the runner.
		_shapeImageIdx = 1;
		setupCurrentCommonImageHotspots();
	} else if (state == kSnoidAnimState005_Drag) {
		// Start normal holding SCRS 146-150 at @ref ZmbSnoid::_shapeImageIdx.
		// Small Snoids use the same selector for SCRS 100-102 and start their one-frame script at 0.
		_holdingAnimPhase = _useSmallShapeRegs ? 0 : CLIP<uint16>(_shapeImageIdx, 0, 2);
		if (_useSmallShapeRegs)
			setupCurrentCommonImageHotspots();
	} else if (state == kSnoidAnimState001_TurnLeft || state == kSnoidAnimState002_TurnRight ||
			   state == kSnoidAnimState010_ArrivalMotion) {
		// Rebuild draw records from the current common-image pose for states 1, 2, and 10.
		// Without this, the previous state's virtual hotspots remain until a later tick.
		// Ferry seat drops would retain their drag pose.
		// An accepted Tunnels Snoid would have no frame after its SCRS was cleared.
		setupCurrentCommonImageHotspots();
	}

	_animState = state;
	_flipCounter = 0;

	// Store trait-specific shadow shapes in @ref ZmbSnoid::_flipShadowShapes.
	// @ref ZmbSnoid::onSnoidAnimTick() swaps the main hotspot shapes with these shadows for six ticks.
	if (state == kSnoidAnimState003_Flip) {
		_shapeImageIdx = 1;
		// Layer order: 0=feet, 1=body, 2=nose, 3=eye, 4=hair.
		// Feet, nose, eye, and hair shadows use bases 435, 440, 430, and 425.
		// The body layer always copies its main shape.
		ZmbHotspotGroup *hsGroup = getHotspotGroup(0);
		if (hsGroup && 5 <= hsGroup->getHotspotCount()) {
			_flipShadowShapes[0] = static_cast<int16>(_trait._feet + 435);
			_flipShadowShapes[1] = hsGroup->getHotspot(1)._shapeIdx; // body: copy from main
			_flipShadowShapes[2] = static_cast<int16>(_trait._nose + 440);
			_flipShadowShapes[3] = static_cast<int16>(_trait._eyes + 430);
			_flipShadowShapes[4] = static_cast<int16>(_trait._hair + 425);
		}
	}

	if (pos)
		setPointLoc(*pos);

	// Reset frame counters for fresh animation playback
}

void ZmbSnoid::startTraitHighlight(uint16 traitMask) {
	if (_animState != kSnoidAnimState000_Idle)
		return;

	_shapeImageIdx = 1;
	ZmbHotspotGroup *hsGroup = getHotspotGroup(0);
	if (!hsGroup || hsGroup->getHotspotCount() < 5)
		return;

	for (uint16 layerIdx = 0; layerIdx < 5; layerIdx++)
		_flipShadowShapes[layerIdx] = hsGroup->getHotspot(layerIdx)._shapeIdx;

	// Virtual Snoid hotspots store combined pre-mirror indices.
	// The renderer maps these categories to the paired tBMP 3000 shapes 851-890.
	if ((traitMask & (1u << ZmbTrait::kTraitFeet)) != 0)
		_flipShadowShapes[0] = static_cast<int16>(_trait._feet + 435);
	if ((traitMask & (1u << ZmbTrait::kTraitNose)) != 0)
		_flipShadowShapes[2] = static_cast<int16>(_trait._nose + 440);
	if ((traitMask & (1u << ZmbTrait::kTraitEyes)) != 0)
		_flipShadowShapes[3] = static_cast<int16>(_trait._eyes + 430);
	if ((traitMask & (1u << ZmbTrait::kTraitHair)) != 0)
		_flipShadowShapes[4] = static_cast<int16>(_trait._hair + 425);

	_needsIdleRedraw = false;
	_flipCounter = 0;
	_animState = kSnoidAnimState003_Flip;
	activateAnimate();
	activateRender();
}

// Compute per-animation-interval walk/path speed from source to destination.
// Use a slope-based direction bucket: 0=up, 1=up-right, 2=right, 3=down-right, and 4=down.
// Select the dominant-axis speed from a fixed table.
// Compute the minor-axis speed as a proportional fraction.
// Speed values are raw pixels per animation tick.
// The time-based deadline advances by the runner's frame interval.
// The default interval is 6, while Bridge accepted crossings use 4 or 5; one default tick is about 102 ms.
void ZmbSnoid::calcPathSpeed(int16 dx, int16 dy, int16 &speedX, int16 &speedY) {
	// @p dy is current Y minus target Y, so a positive value means the target is above the current screen position.
	int slope;
	if (dx != 0) {
		slope = (static_cast<int>(dy) << 10) / ABS(dx);
	} else if (0 <= dy) {
		slope = 1410;
	} else {
		slope = -1410;
	}

	// Direction bucket + speed table
	// dir 0 (<= -1409): mostly upward         -> sx=5,  sy=-15
	// dir 1 (-1409..-333): steep up-right     -> sx=13, sy=-10
	// dir 2 (-332..331):   mostly horizontal  -> sx=16, sy=8
	// dir 3 (332..1408):   steep down-right   -> sx=13, sy=10
	// dir 4 (>= 1409):     mostly downward    -> sx=5,  sy=15
	int16 sx, sy;
	if (slope <= -1409) {
		sx = 5;
		sy = -15;
	} else if (slope <= -332) {
		sx = 13;
		sy = -10;
	} else if (slope < 332) {
		sx = 16;
		sy = 8;
	} else if (slope < 1409) {
		sx = 13;
		sy = 10;
	} else {
		sx = 5;
		sy = 15;
	}

	// Dominant-axis clamping: Divide the dominant distance by its template speed to obtain the segment duration.
	// Scale the minor axis to finish in the same number of frames.
	if (ABS(sy) <= ABS(sx)) {
		// X dominates
		speedX = sx;
		int scale = ABS(dx) / ABS(sx); // frames to cross dx at template rate
		speedY = (scale != 0) ? static_cast<int16>(dy / scale) : static_cast<int16>(dy);
		if (speedY == 0 && dy != 0)
			speedY = (0 < dy) ? 1 : -1;
	} else {
		// Y dominates
		speedY = sy;
		int scale = ABS(dy) / ABS(sy); // frames to cross dy at template rate
		speedX = (scale != 0) ? static_cast<int16>(dx / scale) : static_cast<int16>(dx);
		if (speedX == 0 && dx != 0)
			speedX = (0 < dx) ? 1 : -1;
	}

	// Speed values are raw pixels per animation interval, so no extra scaling is needed.
	speedX = ABS(speedX);
	if (speedY != 0) {
		int16 sgnY = (0 < speedY) ? 1 : -1;
		speedY = sgnY * ABS(speedY);
	}
}

void ZmbSnoid::initWalkToTarget(const Common::Point &target, ZoombiniPage *page) {
	// Enter the departure state.
	// It initializes waypoint routing, or a straight-line walk when NODE data is unavailable.
	// The @ref kSnoidAnimState007_Depart tick handler computes dynamic speed.
	setAnimState(kSnoidAnimState007_Depart);

	// Prime walk frame zero before returning.
	// Maze invokes this immediately after the exit SCRS is released,
	// so retaining the SCRS's last materialized frame until the next Snoid timer would cause a one-frame jump.
	// State 7 still selects the route and performs the first movement step on the normal animation tick.
	// Use direction bucket zero because the target has not yet been consumed.
	if (page) {
		_walkDirBucket = 0;
		updateWalkHotspots(page, _walkDirBucket, 0);
		setNeedsRedraw(true);
	}
	_animTargetPos = target;
}

void ZmbSnoid::initDirectWalkToTarget(const Common::Point &target, ZoombiniPage *page) {
	// Skip NODE/PATH once for a direct state-7 walk.
	_skipNodePathOnNextDepart = true;
	initWalkToTarget(target, page);
}

bool ZmbSnoid::advancePathSubTarget(ZoombiniPage *page, bool forceHotspotUpdate) {
	// Read one PATH slot, advance the route cursor,
	// then choose that checkpoint or the final seat solely by squared distance.
	// It does not apply snoid occupancy filtering.
	_pathSubTarget = _animTargetPos;

	ZmbNode *node = page ? page->getFirstNode() : nullptr;
	if (node && 0 <= _pathRouteIdx && _pathRouteIdx < static_cast<int16>(node->_paths.size()) && 0 <= _pathSlotIdx) {
		const Common::Array<uint8> &path = node->_paths[_pathRouteIdx];
		uint8 waypointRef = 0;
		if (_pathSlotIdx < static_cast<int16>(path.size()))
			waypointRef = path[_pathSlotIdx];
		_pathSlotIdx += _pathWalkDir;

		if (0 < waypointRef && waypointRef <= node->_waypoints.size()) {
			const Common::Point &waypoint = node->_waypoints[waypointRef - 1];
			const Common::Point curPos = getPointLoc();
			if (ZoombiniPage::squaredDistanceBetweenPoints(waypoint, curPos) <
				ZoombiniPage::squaredDistanceBetweenPoints(_animTargetPos, curPos))
				_pathSubTarget = waypoint;
		}
	}

	const Common::Point curPos = getPointLoc();
	int16 dx = _pathSubTarget.x - curPos.x;
	int16 dy = curPos.y - _pathSubTarget.y;
	if (dx == 0 && dy == 0)
		return false;

	int newBucket = computeWalkDirBucket(dx, dy);
	if (forceHotspotUpdate || newBucket != _walkDirBucket) {
		_walkDirBucket = newBucket;
		updateWalkHotspots(page, _walkDirBucket, _walkPhase);
	}
	calcPathSpeed(dx, dy, _animSpeedX, _animSpeedY);
	if (dx != 0)
		_isFacingLeft = (dx < 0);
	return true;
}

void ZmbSnoid::processIdleFidget(ZoombiniPage *page, bool &needsRedraw) {
	if (!_vm->_fidgetThreshold)
		return;

	// Compare the saved signed byte after incrementing the stored counter.
	const int8 previousCounter = static_cast<int8>(_idleTickCounter);
	_idleTickCounter += 1;
	if (previousCounter <= static_cast<int16>(_vm->_fidgetThreshold))
		return;

	// Expiry always restarts the counter, including in the small-body presentation.
	_idleTickCounter = 0;
	if (_useSmallShapeRegs || 10 <= _vm->_rnd->getRandomNumber(0, 100))
		return;

	// Shape-image selector 0 becomes set A, 1 remains set A, and 2 selects set B.
	if (_shapeImageIdx == 0)
		_shapeImageIdx = 1;
	_fidgetValue = _vm->_rnd->getRandomNumber(0, 7);
	setAnimState(kSnoidAnimState006_Fidget);

	// Voice RNG and the retained modulo-32 cadence advance only while the SFX gate is open.
	if (_vm->_fidgetThreshold && _vm->_state->getEnableSound()) {
		const int16 voiceGroup = 50 < _vm->_rnd->getRandomNumber(1, 100) ? 5 : 4;
		_vm->_fidgetVoiceCadenceCounter = (_vm->_fidgetVoiceCadenceCounter + 1) % 32;
		const int16 sndResId = getVoiceResId(voiceGroup);
		if (0 < sndResId)
			page->dispatchFeatureSound(this, ZmbResource(ZmbResource::kSystem, sndResId));
	}

	needsRedraw = true;
}

bool ZmbSnoid::onSnoidAnimTick(ZoombiniPage *page) {
	// Compare the next-render deadline against the selected animation-clock frame counter.
	// An interval of 6 is 100 ms at exact 60 FPS or 102 ms with the original 17 ms tick.
	if (_delayUntilFrame != 0) {
		if (page->getCurrentFrameCounter() < _delayUntilFrame)
			return false;
		// The delay gates animation ticks only.
		// Post-render still draws active runners, so staggered starts do not alter @ref ZmbFeature::_isRenderActivated.
		_delayUntilFrame = 0;
	}
	// Hidden snoids (render-enabled state=0) skip the entire animation state machine.
	// Hidden SCRS pool Snoids must stop ticking after script completion.
	// Otherwise, their idle counters continue with uninitialized traits.
	// The counters may then trigger the wrong fidget voice SFX.
	if (!isRenderActivated())
		return false;
	// Bridge accepted crossings use a random interval of 4 or 5; other Snoids keep the registration-time interval of 6.
	uint32 currentFrame = page->getCurrentFrameCounter();
	if (!getFrameTimingResult(currentFrame))
		return false;
	_nextAnimFrame = currentFrame + getFrameInterval();

	bool needsRedraw = false;

	switch (_animState) {
	case kSnoidAnimState000_Idle: // [*] Fidget check
		// On the first tick after re-entering idle, clear the pending redraw state.
		// @ref ZmbSnoid::_shapeImageIdx intentionally persists across idle ticks
		// so the selected fidget set depends on walk history.
		if (_needsIdleRedraw) {
			_needsIdleRedraw = false;
			needsRedraw = true;
		}
		processIdleFidget(page, needsRedraw);
		break;
	case kSnoidAnimState001_TurnLeft:
	case kSnoidAnimState002_TurnRight: {
		// Post-arrival turn-around animation.
		// Cycle @ref ZmbSnoid::_shapeImageIdx and @ref ZmbSnoid::_isFacingLeft before settling to idle.
		// State 1 enters facing right, flips to left, then idles.
		// State 2 enters facing left, flips to right, then idles.
		// Falls through to idle tick (fidget check) in the same tick.
		_idleTickCounter = 0;
		needsRedraw = true;

		if (_animState == kSnoidAnimState001_TurnLeft) {
			// State 1: settling condition = facing left
			if (_isFacingLeft) {
				_shapeImageIdx = 1;
				_animState = kSnoidAnimState000_Idle;
			} else {
				if (_shapeImageIdx == 2) {
					_shapeImageIdx = 1;
				} else {
					_shapeImageIdx = 0;
					_isFacingLeft = true;
				}
			}
		} else {
			// State 2: settling condition = facing right
			if (_isFacingLeft) {
				if (_shapeImageIdx == 2) {
					_shapeImageIdx = 1;
				} else {
					_shapeImageIdx = 0;
					_isFacingLeft = false;
				}
			} else {
				_shapeImageIdx = 1;
				_animState = kSnoidAnimState000_Idle;
			}
		}

		// Keep drawing the common-image set selected when the turn state began.
		// The stored selector and facing direction take effect when a later state transition
		// materializes a new common-image set.
		// Turn states continue through the shared idle/fidget tail in this tick.
		processIdleFidget(page, needsRedraw);
		break;
	}
	case kSnoidAnimState003_Flip: {
		// Swap the five shape-layer slots with the shadow slots on each tick.
		// Return to idle after six swaps.
		// Do not fall through to redraw.
		if (6 <= _flipCounter) {
			setAnimState(kSnoidAnimState000_Idle);
			_idleTickCounter = 0;
		} else {
			// Swap main hotspot shapes with shadow shapes
			ZmbHotspotGroup *hsGroup = getHotspotGroup(0);
			if (hsGroup && 5 <= hsGroup->getHotspotCount()) {
				for (int shapeIdx = 0; shapeIdx < 5; shapeIdx++) {
					int16 tmp = hsGroup->getHotspot(shapeIdx)._shapeIdx;
					hsGroup->getHotspot(shapeIdx)._shapeIdx = _flipShadowShapes[shapeIdx];
					_flipShadowShapes[shapeIdx] = tmp;
				}
			}
			_flipCounter += 1;
		}
		needsRedraw = true;
		break;
	}
	case kSnoidAnimState004_Arrive: {
		// Move the Snoid directly to its target, then enter the configured post-arrival turn state.
		needsRedraw = true;
		Common::Point pos = getPointLoc();
		if (pos == _animTargetPos) {
			_idleTickCounter = 0;
			// Enter the configured post-arrival state: idle, turn right, or turn left.
			setAnimState(_vm->_arrivalTurnState);
		} else {
			// Face right and copy the target coordinates to the current position.
			_isFacingLeft = false;
			setPointLoc(_animTargetPos);
		}
		break;
	}
	case kSnoidAnimState005_Drag: {
		// Position is set externally by mouse handler.
		// Small Town snoids use the common-image pose selected when drag began;
		// the feet-specific holding SCRS group is only used at normal scale.
		if (_useSmallShapeRegs) {
			needsRedraw = true;
			break;
		}
		// Use holding animation (SCRS 146-150) based on feet type.
		// Advance once per tick.
		// At the declared frame count, reset to frame 2 and loop; small Snoids reset to frame 0.
		// This cycles through all holding animation frames for feet animation.
		needsRedraw = true;
		if (page) {
			const uint8 feet = CLIP<uint8>(_trait._feet, 1, 5);
			const int16 scrsId = static_cast<int16>(145 + feet);
			const ZmbScriptDecoder::DecodedScrs *anim = page->getDecodedScrs(ZmbResource(ZmbResource::kSystem, scrsId));
			if (anim && !anim->frames.empty()) {
				// Advance each tick and wrap to frame 2, where the loop begins.
				if (anim->frames.size() <= _holdingAnimPhase) {
					_holdingAnimPhase = _useSmallShapeRegs ? 0 : 2;
				}
				updateHoldingHotspots(page);
				_holdingAnimPhase += 1;
			}
		}
		break;
	}
	case kSnoidAnimState006_Fidget: {
		// Play one SCRS fidget frame per tick, then return to idle at the declared frame count.
		// @ref ZmbSnoid::_shapeImageIdx selects set A for value 1 and set B for value 2.
		needsRedraw = true;
		if (page) {
			int fidgetSet = (2 <= _shapeImageIdx) ? 1 : 0;
			const int16 scrsId = static_cast<int16>((fidgetSet == 0 ? 130 : 138) + _fidgetValue);
			const ZmbScriptDecoder::DecodedScrs *anim = page->getDecodedScrs(ZmbResource(ZmbResource::kSystem, scrsId));
			if (anim && _flipCounter < static_cast<int32>(anim->frames.size())) {
				updateFidgetHotspots(page, fidgetSet, _fidgetValue, _flipCounter);
				_flipCounter += 1;
			} else {
				setAnimState(kSnoidAnimState000_Idle);
			}
		} else {
			// Fallback when page is unavailable: wait ~20 ticks.
			_flipCounter += 1;
			if (20 <= _flipCounter)
				setAnimState(kSnoidAnimState000_Idle);
		}
		break;
	}
	case kSnoidAnimState007_Depart: {
		// Select a PATH route and direction, compute segment speed, then transition to the path-walking state.
		// 1. Find the one-based waypoint nearest to the final destination.
		// 2. Among paths containing it, find the member nearest to the current position.
		// 3. Store the route, next slot, and direction.
		// The stepper reads PATH slots dynamically without precomputing or occupancy-filtering them.
		_pathRouteIdx = -1;
		_pathSlotIdx = -1;
		_pathWalkDir = 1;
		_pathSubTarget = _animTargetPos;
		const bool skipNodePath = _skipNodePathOnNextDepart || (page && page->isSnoidNodePathDisabled());
		_skipNodePathOnNextDepart = false;

		if (page) {
			ZmbNode *node = page->getFirstNode();
			if (node && !node->_waypoints.empty() && !node->_paths.empty() &&
				!skipNodePath) {
				const Common::Point curPos = getPointLoc();
				const Common::Array<Common::Point> &wps = node->_waypoints;

				uint8 destinationWaypointRef = 0;
				int32 minDestDist = 999999;
				for (uint32 entryIdx = 0; entryIdx < wps.size(); entryIdx++) {
					const int32 dist = ZoombiniPage::squaredDistanceBetweenPoints(wps[entryIdx], _animTargetPos);
					if (dist <= minDestDist) {
						minDestDist = dist;
						destinationWaypointRef = static_cast<uint8>(entryIdx + 1);
					}
				}

				int32 minCurDist = 999999;
				for (uint32 routeIdx = 0; routeIdx < node->_paths.size(); routeIdx++) {
					const Common::Array<uint8> &path = node->_paths[routeIdx];
					int16 destinationSlotIdx = -1;
					for (uint32 slotIdx = 0; slotIdx < path.size(); slotIdx++) {
						if (path[slotIdx] == destinationWaypointRef) {
							destinationSlotIdx = static_cast<int16>(slotIdx);
							break;
						}
					}
					if (destinationSlotIdx < 0)
						continue;

					for (uint32 slotIdx = 0; slotIdx < path.size(); slotIdx++) {
						uint8 waypointRef = path[slotIdx];
						if (waypointRef == 0 || wps.size() < waypointRef)
							continue;

						const Common::Point &waypoint = wps[waypointRef - 1];
						const int32 dist = ZoombiniPage::squaredDistanceBetweenPoints(waypoint, curPos);
						if (dist <= minCurDist) {
							minCurDist = dist;
							_pathRouteIdx = static_cast<int16>(routeIdx);
							_pathSlotIdx = static_cast<int16>(slotIdx + 1);
							_pathWalkDir = 1;
							if (destinationSlotIdx != 0 && destinationSlotIdx <= static_cast<int16>(slotIdx))
								_pathWalkDir = -1;
						}
					}
				}
			}
		}

		advancePathSubTarget(page, true);

		// Route initialization and the first movement step happen in the same tick.
		needsRedraw = true;
		_animState = kSnoidAnimState112_Path;

		{ // Fall through: apply first movement step in this same tick.
			Common::Point pos = getPointLoc();
			int16 dx = _pathSubTarget.x - pos.x;
			int16 dy = pos.y - _pathSubTarget.y;
			if (dx != 0 || dy != 0) {
				if (dx != 0) {
					int16 step = MIN<int16>(ABS(dx), ABS(_animSpeedX));
					pos.x += (0 < dx) ? step : -step;
					_isFacingLeft = (dx < 0);
				}
				if (dy != 0) {
					int16 step = MIN<int16>(ABS(dy), ABS(_animSpeedY));
					pos.y += (0 < dy) ? -step : step;
				}
				setPointLoc(pos);
			}
		}
		break;
	}
	case kSnoidAnimState112_Path: {
		// Move along NODE waypoints toward final destination.
		// The subtarget in @ref ZmbSnoid::_pathSubTarget advances through the selected PATH slots
		// until the final-seat distance shortcut applies.
		needsRedraw = true;
		Common::Point pos = getPointLoc();

		int16 dx = _pathSubTarget.x - pos.x;
		int16 dy = pos.y - _pathSubTarget.y;

		if (dx == 0 && dy == 0) {
			if (!advancePathSubTarget(page)) {
				// At the final destination, retain the last walking direction and enter the configured post-arrival state.
				_idleTickCounter = 0;
				setAnimState(_vm->_arrivalTurnState);
				if (0 < _vm->_walkersInProgress)
					_vm->_walkersInProgress -= 1;
				page->onSnoidWalkCompleted(this);
			}
		} else {
			// Speed and direction remain fixed for the current segment.
			// They are set on departure or arrival at a waypoint and remain constant between waypoints.
			// Recomputing direction from the shrinking distance each tick made the slope drift near waypoints.
			// Integer rounding exhausted one axis before the other.
			// The sprite then appeared to walk horizontally instead of diagonally.
			if (dx != 0) {
				int16 step = MIN<int16>(ABS(dx), ABS(_animSpeedX));
				pos.x += (0 < dx) ? step : -step;
				_isFacingLeft = (dx < 0);
			}
			if (dy != 0) {
				int16 step = MIN<int16>(ABS(dy), ABS(_animSpeedY));
				pos.y += (0 < dy) ? -step : step;
			}
			setPointLoc(pos);

			// Advance the walk animation once per interval.
			_walkPhase += 1;
			updateWalkHotspots(page, _walkDirBucket, _walkPhase);
		}
		break;
	}
	case kSnoidAnimState008_ScriptReject:
	case kSnoidAnimState009_ScriptNormal: {
		// Advance one SCRS frame per render-timer interval.
		// At the declared frame count, either hide the Snoid or return it to idle.
		// Use @ref ZmbFeature::getFrameCount() rather than @ref ZmbFeature::getMaxFrameIdx() for the terminal index.
		// The header count is authoritative, so the Snoid must traverse terminator-only trailing frames.
		// This includes Ferry SCRS 1900/1904/1906 frame 24, whose 0xFF03 terminator carries case 2.
		// @ref ZoombiniPage::preRenderFeature() dispatches each frame's event.
		const int32 lastFrame = static_cast<int32>(getFrameCount()) - 1;
		if (_scrsJustStarted) {
			_scrsJustStarted = false;
			needsRedraw = true;
			break;
		}
		if (getLastFrameIdx() < lastFrame) {
			setLastFrameIdx(getLastFrameIdx() + 1);
			syncScrsPointLoc();
			needsRedraw = true;
		} else {
			// SCRS animation finished.
			const bool dispatchEndEvent = _scrsEndEventEnabled;
			// kHide hides the Snoid without restoring its position.
			// Otherwise, return to idle at the current SCRS-driven position.
			if (_scrsCompletionMode == ZmbScrsCompletionMode::kHide) {
				finishScrsPlayback(false);
				deactivateRender();
				_animState = kSnoidAnimState000_Idle;
			} else {
				finishScrsPlayback(false);
				setAnimState(kSnoidAnimState000_Idle);
			}
			_scrsCompletionMode = ZmbScrsCompletionMode::kReturnToIdle;
			_scrsJustStarted = false;
			// Dispatch the one-shot animation-end event.
			if (page && dispatchEndEvent)
				page->onFeatureAnimEvent(this, ZoombiniPage::kAnimEventM1_End);
			needsRedraw = true;
		}
		break;
	}
	case kSnoidAnimState010_ArrivalMotion:
		// Enter the departure path, reset the direction bucket, prime walk frame zero,
		// and increment the walkers-in-progress counter.
		setAnimState(kSnoidAnimState007_Depart);
		_walkDirBucket = 0;
		updateWalkHotspots(page, _walkDirBucket, _walkPhase);
		_vm->_walkersInProgress += 1;
		needsRedraw = true;
		break;
	default:
		break;
	}

	return needsRedraw;
}

void ZmbSnoid::setupCommonImageHotspots(uint16 rawShape, bool useSmallShapeRegs) {
	// Use the generic common-image family from SCRS 100/101/102.
	// @p rawShape selects the body-part variant; trait tables synthesize the per-layer positions.
	static constexpr uint16 kFeetTable[6] = {0, 191, 246, 335, 360, 411};
	static constexpr uint16 kNoseTable[6] = {0, 171, 175, 179, 183, 187};
	static constexpr uint16 kEyeTable[6] = {0, 91, 107, 123, 139, 155};
	static constexpr uint16 kHairTable[6] = {0, 11, 27, 43, 59, 75};

	static constexpr uint16 kSmallFeetTable[6] = {0, 131, 174, 227, 235, 278};
	static constexpr uint16 kSmallNoseTable[6] = {0, 111, 115, 119, 123, 127};
	static constexpr uint16 kSmallEyeTable[6] = {0, 91, 95, 99, 103, 107};
	static constexpr uint16 kSmallHairTable[6] = {0, 11, 27, 43, 59, 75};

	const uint16 *feetTable = useSmallShapeRegs ? kSmallFeetTable : kFeetTable;
	const uint16 *noseTable = useSmallShapeRegs ? kSmallNoseTable : kNoseTable;
	const uint16 *eyeTable = useSmallShapeRegs ? kSmallEyeTable : kEyeTable;
	const uint16 *hairTable = useSmallShapeRegs ? kSmallHairTable : kHairTable;

	uint8 feet = (1 <= _trait._feet && _trait._feet <= 5) ? _trait._feet : 1;
	uint8 nose = (1 <= _trait._nose && _trait._nose <= 5) ? _trait._nose : 1;
	uint8 eye = (1 <= _trait._eyes && _trait._eyes <= 5) ? _trait._eyes : 1;
	uint8 hair = (1 <= _trait._hair && _trait._hair <= 5) ? _trait._hair : 1;

	rawShape = CLIP<uint16>(rawShape, 1, 3);
	_useSmallShapeRegs = useSmallShapeRegs;

	Common::Array<ZmbHotspot> hotspots;
	hotspots.push_back(ZmbHotspot(0, feetTable[feet] + rawShape, 0, 0, 0));
	hotspots.push_back(ZmbHotspot(1, 0 + rawShape, 0, 0, 0));
	hotspots.push_back(ZmbHotspot(2, noseTable[nose] + rawShape, 0, 0, 0));
	hotspots.push_back(ZmbHotspot(3, eyeTable[eye] + rawShape, 0, 0, 0));
	hotspots.push_back(ZmbHotspot(4, hairTable[hair] + rawShape, 0, 0, 0));

	_usesVirtualHotspots = true;
	setVirtualHotspots(hotspots);
}

void ZmbSnoid::setupCurrentCommonImageHotspots() {
	setupCommonImageHotspots(static_cast<uint16>(CLIP<int>(static_cast<int>(_shapeImageIdx), 0, 2) + 1), _useSmallShapeRegs);
}

void ZmbSnoid::setupIdleHotspots() {
	// Raw shape 2 is the seated idle pose for every body layer.
	// Raw shape 1 is the front-facing pose.
	static constexpr uint16 kRawShapeIdle = 2;
	setupCommonImageHotspots(kRawShapeIdle, false);
}

void ZmbSnoid::setupSmallIdleHotspots() {
	// Small Snoids use the same raw seated-idle shape.
	static constexpr uint16 kRawShapeIdle = 2;
	setupCommonImageHotspots(kRawShapeIdle, true);
}

int16 ZmbSnoid::getBodyLayerBaseOffset(uint8 layer, uint8 layerShift) const {
	// General trait tables
	static constexpr int16 kFeetTable[6] = {0, 191, 246, 335, 360, 411};
	static constexpr int16 kNoseTable[6] = {0, 171, 175, 179, 183, 187};
	static constexpr int16 kEyeTable[6] = {0, 91, 107, 123, 139, 155};
	static constexpr int16 kHairTable[6] = {0, 11, 27, 43, 59, 75};

	// NORMAL-specific trait tables
	static constexpr int16 kNormalFeetTable[6] = {0, 288, 306, 324, 342, 360};
	static constexpr int16 kNormalNoseTable[6] = {0, 198, 216, 234, 252, 270};
	static constexpr int16 kNormalEyeTable[6] = {0, 108, 126, 144, 162, 180};
	static constexpr int16 kNormalHairTable[6] = {0, 18, 72, 36, 54, 90};

	// Small-Snoid tables.
	// Used when general tables are swapped to small variants (XFER_0).
	static constexpr int16 kSmallFeetTable[6] = {0, 131, 174, 227, 235, 278};
	static constexpr int16 kSmallNoseTable[6] = {0, 111, 115, 119, 123, 127};
	static constexpr int16 kSmallEyeTable[6] = {0, 91, 95, 99, 103, 107};
	static constexpr int16 kSmallHairTable[6] = {0, 11, 27, 43, 59, 75};

	// Apply layerShift when a NORMAL frame's first raw shape exceeds 18.
	// The shift is independent of the number of visible layers.
	int effectiveLayer = static_cast<int>(layer) - static_cast<int>(layerShift);
	if (effectiveLayer < 0 || 4 < effectiveLayer)
		return 0;

	uint8 feet = (1 <= _trait._feet && _trait._feet <= 5) ? _trait._feet : 1;
	uint8 nose = (1 <= _trait._nose && _trait._nose <= 5) ? _trait._nose : 1;
	uint8 eye = (1 <= _trait._eyes && _trait._eyes <= 5) ? _trait._eyes : 1;
	uint8 hair = (1 <= _trait._hair && _trait._hair <= 5) ? _trait._hair : 1;

	// Select trait tables by animation state.
	// NORMAL scripts use their dedicated tables; other states use the general or small-Snoid tables.
	const int16 *feetTbl, *noseTbl, *eyeTbl, *hairTbl;
	if (_animState == kSnoidAnimState009_ScriptNormal) {
		feetTbl = kNormalFeetTable;
		noseTbl = kNormalNoseTable;
		eyeTbl = kNormalEyeTable;
		hairTbl = kNormalHairTable;
	} else if (_useSmallShapeRegs) {
		feetTbl = kSmallFeetTable;
		noseTbl = kSmallNoseTable;
		eyeTbl = kSmallEyeTable;
		hairTbl = kSmallHairTable;
	} else {
		feetTbl = kFeetTable;
		noseTbl = kNoseTable;
		eyeTbl = kEyeTable;
		hairTbl = kHairTable;
	}

	// Slot mapping depends on the SCRS trait layout.
	//   Trait layout 0: [feet, body(0), nose, eye, hair].
	//   Trait layout 1: [feet, nose, body(0), eye, hair].
	//   Trait layout 2: [body(0), eye, nose, feet, hair].
	switch (_traitLayout) {
	case ZmbScriptDecoder::TraitLayout::kSlotOrder01:
		switch (effectiveLayer) {
		case 0:
			return feetTbl[feet];
		case 1:
			return noseTbl[nose];
		case 2:
			return 0; // body
		case 3:
			return eyeTbl[eye];
		case 4:
			return hairTbl[hair];
		default:
			return 0;
		}
		break;
	case ZmbScriptDecoder::TraitLayout::kSlotOrder02:
		switch (effectiveLayer) {
		case 0:
			return 0; // body
		case 1:
			return eyeTbl[eye];
		case 2:
			return noseTbl[nose];
		case 3:
			return feetTbl[feet];
		case 4:
			return hairTbl[hair];
		default:
			return 0;
		}
		break;
	case ZmbScriptDecoder::TraitLayout::kSlotOrder00:
	default: // trait layout 0 (most common)
		switch (effectiveLayer) {
		case 0:
			return feetTbl[feet];
		case 1:
			return 0; // body
		case 2:
			return noseTbl[nose];
		case 3:
			return eyeTbl[eye];
		case 4:
			return hairTbl[hair];
		default:
			return 0;
		}
		break;
	}
}

int16 ZmbSnoid::getVoiceResId(int16 voiceGroup) const {
	// Map voice groups 0-17 to SND resource IDs.
	// Groups 0-15: base SND ID in steps of 25 (100, 125, ..., 475) + trait-based offset.
	// Group 16: random SND in range [1800, 1814].
	// Group 17: fixed SND resource 99.
	static constexpr int16 kVoiceGroupBase[16] = {
		100, 125, 150, 175, 200, 225, 250, 275,
		300, 325, 350, 375, 400, 475, 450, 425};

	int16 base = 0;
	bool applyTraitOffset = true;

	if (0 <= voiceGroup && voiceGroup <= 15) {
		base = kVoiceGroupBase[voiceGroup];
	} else if (voiceGroup == 16) {
		base = _vm->_rnd->getRandomNumber(1800, 1814);
		applyTraitOffset = false;
	} else if (voiceGroup == 17) {
		base = 99;
		applyTraitOffset = false;
	}

	if (base == 0)
		return 0;

	if (applyTraitOffset) {
		// Hair traits 1-3 and 4-5 select distinct voice-block offsets.
		uint8 hair = _trait._hair;
		switch (hair) {
		case 2:
			base += 5;
			break;
		case 3:
			base += 20;
			break;
		case 4:
			base += 15;
			break;
		case 5:
			base += 10;
			break;
		default:
			break; // hair 0, 1: no additional offset
		}
		// Add the nose-trait offset.
		base += static_cast<int16>(_trait._nose) - 1;
	}

	return base;
}

/** Compute a direction bucket from 0 through 4 for a movement vector.
 * The Y delta is current Y minus target Y, so positive values place the target above the Snoid on screen.
 * Slope thresholds are the fixed-point values (<<10 scale).
 */
int ZmbSnoid::computeWalkDirBucket(int16 dx, int16 dy) {
	int32 slope;
	if (dx != 0) {
		slope = (static_cast<int32>(dy) << 10) / ABS(dx);
	} else {
		slope = (0 <= dy) ? 1410 : -1410;
	}
	if (slope <= -1409)
		return 0;
	if (slope <= -332)
		return 1;
	if (slope < 332)
		return 2;
	if (slope < 1409)
		return 3;
	return 4;
}

void ZmbSnoid::updateWalkHotspots(ZoombiniPage *page, int dirBucket, int phase) {
	if (!page)
		return;

	// Normal-size body-part tables.
	static constexpr uint16 kFeetTable[6] = {0, 191, 246, 335, 360, 411};
	static constexpr uint16 kNoseTable[6] = {0, 171, 175, 179, 183, 187};
	static constexpr uint16 kEyeTable[6] = {0, 91, 107, 123, 139, 155};
	static constexpr uint16 kHairTable[6] = {0, 11, 27, 43, 59, 75};

	// Small-Snoid body-part tables.
	// Used when the snoid is walking with resource 3200 (XFER_0 picker-to-bridge).
	static constexpr uint16 kSmallFeetTable[6] = {0, 131, 174, 227, 235, 278};
	static constexpr uint16 kSmallNoseTable[6] = {0, 111, 115, 119, 123, 127};
	static constexpr uint16 kSmallEyeTable[6] = {0, 91, 95, 99, 103, 107};
	static constexpr uint16 kSmallHairTable[6] = {0, 11, 27, 43, 59, 75};

	const uint16 *feetTbl = _useSmallShapeRegs ? kSmallFeetTable : kFeetTable;
	const uint16 *noseTbl = _useSmallShapeRegs ? kSmallNoseTable : kNoseTable;
	const uint16 *eyeTbl = _useSmallShapeRegs ? kSmallEyeTable : kEyeTable;
	const uint16 *hairTbl = _useSmallShapeRegs ? kSmallHairTable : kHairTable;

	uint8 feet = CLIP<uint8>(_trait._feet, 1, 5);
	uint8 nose = CLIP<uint8>(_trait._nose, 1, 5);
	uint8 eye = CLIP<uint8>(_trait._eyes, 1, 5);
	uint8 hair = CLIP<uint8>(_trait._hair, 1, 5);

	const int16 scrsId = static_cast<int16>(100 + CLIP<int>(dirBucket, 0, 4) + 5 * feet);
	const ZmbScriptDecoder::DecodedScrs *anim = page->getDecodedScrs(ZmbResource(ZmbResource::kSystem, scrsId));
	if (!anim || anim->frames.empty())
		return;

	int frameIdx = 0;
	if (0 < phase && 1 < anim->frames.size())
		frameIdx = 1 + ((phase - 1) % static_cast<int>(anim->frames.size() - 1));

	const ZmbDecodedScriptFrame &frame = anim->frames[frameIdx];
	// Empty frame: keep current visual state
	if (frame.hotspots.empty())
		return;
	if (5 < frame.hotspots.size()) {
		error("ZmbSnoid: system walk SCRS %d frame %d has too many body layers", scrsId, frameIdx);
		return;
	}

	// The walk SCRS trait layout determines the body-layer slot layout.
	//   Trait layout 0: [feet, body(0), nose, eye, hair] for directions 0-2.
	//   Trait layout 1: [feet, nose, body(0), eye, hair] for directions 3-4.
	//   Trait layout 2: [body(0), eye, nose, feet, hair], unused by walk animations.
	int16 traitBase[5];
	if (anim->traitLayout == ZmbScriptDecoder::TraitLayout::kSlotOrder01) {
		traitBase[0] = static_cast<int16>(feetTbl[feet]);
		traitBase[1] = static_cast<int16>(noseTbl[nose]); // slot 1 = nose
		traitBase[2] = 0;                                 // slot 2 = body base
		traitBase[3] = static_cast<int16>(eyeTbl[eye]);
		traitBase[4] = static_cast<int16>(hairTbl[hair]);
	} else {
		traitBase[0] = static_cast<int16>(feetTbl[feet]);
		traitBase[1] = 0;                                 // slot 1 = body base
		traitBase[2] = static_cast<int16>(noseTbl[nose]); // slot 2 = nose
		traitBase[3] = static_cast<int16>(eyeTbl[eye]);
		traitBase[4] = static_cast<int16>(hairTbl[hair]);
	}

	Common::Array<ZmbHotspot> hotspots;
	for (uint32 shapeIdx = 0; shapeIdx < frame.hotspots.size(); shapeIdx++) {
		const ZmbHotspot &rawHotspot = frame.hotspots[shapeIdx];
		const int16 shape = 0 < rawHotspot._shapeIdx ? traitBase[shapeIdx] + rawHotspot._shapeIdx : 0;
		hotspots.push_back(ZmbHotspot(static_cast<uint16>(shapeIdx), shape, 0, rawHotspot._x, rawHotspot._y));
	}

	_usesVirtualHotspots = true;
	setVirtualHotspots(hotspots);
}

void ZmbSnoid::updateFidgetHotspots(ZoombiniPage *page, int fidgetSet, int variant, int frameIdx) {
	if (!page)
		return;

	static constexpr uint16 kFeetTable[6] = {0, 191, 246, 335, 360, 411};
	static constexpr uint16 kNoseTable[6] = {0, 171, 175, 179, 183, 187};
	static constexpr uint16 kEyeTable[6] = {0, 91, 107, 123, 139, 155};
	static constexpr uint16 kHairTable[6] = {0, 11, 27, 43, 59, 75};
	static constexpr uint16 kSmallFeetTable[6] = {0, 131, 174, 227, 235, 278};
	static constexpr uint16 kSmallNoseTable[6] = {0, 111, 115, 119, 123, 127};
	static constexpr uint16 kSmallEyeTable[6] = {0, 91, 95, 99, 103, 107};
	static constexpr uint16 kSmallHairTable[6] = {0, 11, 27, 43, 59, 75};

	const uint16 *feetTbl = _useSmallShapeRegs ? kSmallFeetTable : kFeetTable;
	const uint16 *noseTbl = _useSmallShapeRegs ? kSmallNoseTable : kNoseTable;
	const uint16 *eyeTbl = _useSmallShapeRegs ? kSmallEyeTable : kEyeTable;
	const uint16 *hairTbl = _useSmallShapeRegs ? kSmallHairTable : kHairTable;

	uint8 feet = CLIP<uint8>(_trait._feet, 1, 5);
	uint8 nose = CLIP<uint8>(_trait._nose, 1, 5);
	uint8 eye = CLIP<uint8>(_trait._eyes, 1, 5);
	uint8 hair = CLIP<uint8>(_trait._hair, 1, 5);

	const int16 scrsId = static_cast<int16>((CLIP<int>(fidgetSet, 0, 1) == 0 ? 130 : 138) + CLIP<int>(variant, 0, 7));
	const ZmbScriptDecoder::DecodedScrs *anim = page->getDecodedScrs(ZmbResource(ZmbResource::kSystem, scrsId));
	if (!anim || anim->frames.empty())
		return;

	const int materializedFrameIdx = frameIdx % static_cast<int>(anim->frames.size());
	const ZmbDecodedScriptFrame &frame = anim->frames[materializedFrameIdx];
	// Empty frame: keep current visual state
	if (frame.hotspots.empty())
		return;
	if (5 < frame.hotspots.size()) {
		error("ZmbSnoid: system fidget SCRS %d frame %d has too many body layers", scrsId, materializedFrameIdx);
		return;
	}

	// Fidget SCRSes all have variant=0; apply arrangement 0 unconditionally here too.
	int16 traitBase[5];
	traitBase[0] = static_cast<int16>(feetTbl[feet]);
	traitBase[1] = 0;
	traitBase[2] = static_cast<int16>(noseTbl[nose]);
	traitBase[3] = static_cast<int16>(eyeTbl[eye]);
	traitBase[4] = static_cast<int16>(hairTbl[hair]);

	Common::Array<ZmbHotspot> hotspots;
	for (uint32 shapeIdx = 0; shapeIdx < frame.hotspots.size(); shapeIdx++) {
		const ZmbHotspot &rawHotspot = frame.hotspots[shapeIdx];
		const int16 shape = 0 < rawHotspot._shapeIdx ? traitBase[shapeIdx] + rawHotspot._shapeIdx : 0;
		hotspots.push_back(ZmbHotspot(static_cast<uint16>(shapeIdx), shape, 0, rawHotspot._x, rawHotspot._y));
	}

	_usesVirtualHotspots = true;
	setVirtualHotspots(hotspots);
}

void ZmbSnoid::updateHoldingHotspots(ZoombiniPage *page) {
	if (!page)
		return;

	// Use SCRS 146-150, selected by feet type; feet type 1 selects SCRS 146.
	// @ref ZmbSnoid::_holdingAnimPhase advances each tick in @ref ZmbSnoid::onSnoidAnimTick().
	static constexpr uint16 kFeetTable[6] = {0, 191, 246, 335, 360, 411};
	static constexpr uint16 kNoseTable[6] = {0, 171, 175, 179, 183, 187};
	static constexpr uint16 kEyeTable[6] = {0, 91, 107, 123, 139, 155};
	static constexpr uint16 kHairTable[6] = {0, 11, 27, 43, 59, 75};
	static constexpr uint16 kSmallFeetTable[6] = {0, 131, 174, 227, 235, 278};
	static constexpr uint16 kSmallNoseTable[6] = {0, 111, 115, 119, 123, 127};
	static constexpr uint16 kSmallEyeTable[6] = {0, 91, 95, 99, 103, 107};
	static constexpr uint16 kSmallHairTable[6] = {0, 11, 27, 43, 59, 75};

	const uint16 *feetTbl = _useSmallShapeRegs ? kSmallFeetTable : kFeetTable;
	const uint16 *noseTbl = _useSmallShapeRegs ? kSmallNoseTable : kNoseTable;
	const uint16 *eyeTbl = _useSmallShapeRegs ? kSmallEyeTable : kEyeTable;
	const uint16 *hairTbl = _useSmallShapeRegs ? kSmallHairTable : kHairTable;

	uint8 feet = CLIP<uint8>(_trait._feet, 1, 5);
	uint8 nose = CLIP<uint8>(_trait._nose, 1, 5);
	uint8 eye = CLIP<uint8>(_trait._eyes, 1, 5);
	uint8 hair = CLIP<uint8>(_trait._hair, 1, 5);

	const int16 scrsId = static_cast<int16>(145 + feet);
	const ZmbScriptDecoder::DecodedScrs *anim = page->getDecodedScrs(ZmbResource(ZmbResource::kSystem, scrsId));
	if (!anim || anim->frames.empty())
		return;

	// Use @ref ZmbSnoid::_holdingAnimPhase for frame cycling during drag.
	// Advance each tick and wrap at the authored SCRS frame count.
	// Phase resets to 2 when looping (frames 0-1 are entry poses, 2+ are cycling).
	int frameIdx = CLIP<int>(static_cast<int>(_holdingAnimPhase), 0, static_cast<int>(anim->frames.size()) - 1);
	const ZmbDecodedScriptFrame &frame = anim->frames[frameIdx];

	// Empty frame: keep current visual state
	if (frame.hotspots.empty())
		return;
	if (5 < frame.hotspots.size()) {
		error("ZmbSnoid: system holding SCRS %d frame %d has too many body layers", scrsId, frameIdx);
		return;
	}

	// Holding SCRSes use variant=0 (normal arrangement).
	int16 traitBase[5];
	traitBase[0] = static_cast<int16>(feetTbl[feet]);
	traitBase[1] = 0;
	traitBase[2] = static_cast<int16>(noseTbl[nose]);
	traitBase[3] = static_cast<int16>(eyeTbl[eye]);
	traitBase[4] = static_cast<int16>(hairTbl[hair]);

	Common::Array<ZmbHotspot> hotspots;
	for (uint32 shapeIdx = 0; shapeIdx < frame.hotspots.size(); shapeIdx++) {
		const ZmbHotspot &rawHotspot = frame.hotspots[shapeIdx];
		const int16 shape = 0 < rawHotspot._shapeIdx ? traitBase[shapeIdx] + rawHotspot._shapeIdx : 0;
		hotspots.push_back(ZmbHotspot(static_cast<uint16>(shapeIdx), shape, 0, rawHotspot._x, rawHotspot._y));
	}

	_usesVirtualHotspots = true;
	setVirtualHotspots(hotspots);
}

} // End of namespace Mohawk
