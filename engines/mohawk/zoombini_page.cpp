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

#include "common/algorithm.h"

#include "mohawk/cursors.h"
#include "mohawk/resource.h"

#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_metaengine.h"
#include "mohawk/zoombini_page.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"
#include "mohawk/zoombini_text.h"

namespace Mohawk {

ZoombiniPage::ZoombiniPage(MohawkEngine_Zoombini *vm, ZoombiniPageCategory pageCategory, ZoombiniPageType pageType) : _vm(vm), _pageCategory(pageCategory), _pageType(pageType) {
	_pageStartFrameTime = _vm->_system->getMillis();
	_pageStartFrameCounter = _vm->getAnimationFrameCounter(_pageStartFrameTime);
	_currentFrameTime = _pageStartFrameTime;
	_currentFrameCounter = _pageStartFrameCounter;
	_lastFrameTime = _pageStartFrameTime;
	_lastFrameCounter = _pageStartFrameCounter;
}

ZoombiniPage::~ZoombiniPage() {
	restoreSnoidDragFidgets();
	clear();
}

uint8 ZoombiniPage::getScriptSoundPriority(ZmbResource resource) const {
	return lookupScriptSoundPriority(resource, getScriptSoundPriorityRanges());
}

Audio::Mixer::SoundType ZoombiniPage::getFeatureSoundType(const ZmbFeature *feature, ZmbResource soundRes) const {
	if (dynamic_cast<const ZmbSnoid *>(feature) && soundRes._archiveKind == ZmbResource::kSystem &&
		kSysResSound0100_ZoombiniVoiceBase <= soundRes._id && soundRes._id <= kSysResSound0499_ZoombiniVoiceLast)
		return Audio::Mixer::kSpeechSoundType;

	return Audio::Mixer::kSFXSoundType;
}

bool ZoombiniPage::debugDoPageCommand(int argc, const char **argv, Common::String &output) {
	if (1 < argc)
		output = Common::String::format("Page command '%s' is not available on the active page.\n", argv[1]);
	else
		output = "No page commands are available on the active page.\n";
	return true;
}

bool ZoombiniPage::debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output) {
	(void)argc;
	(void)argv;

	output = "The active page provides no built-in debug actions.\n";
	return true;
}

uint8 ZoombiniPage::lookupScriptSoundPriority(ZmbResource resource, const ZoombiniPage::ScriptSoundPriorityRanges &ranges) {
	for (uint i = 0; i < ranges.size(); i++) {
		if (ranges[i].first <= resource._id && resource._id <= ranges[i].last)
			return static_cast<uint8>(32 - i);
	}
	return 0;
}

ZoombiniPage::KeyboardNavDirection ZoombiniPage::getKeyboardNavDirection(const Common::KeyState &kbd) {
	switch (kbd.keycode) {
	case Common::KEYCODE_LEFT:
		return KBD_NAV_LEFT;
	case Common::KEYCODE_KP4:
		return (kbd.flags & Common::KBD_NUM) == 0 ? KBD_NAV_LEFT : KBD_NAV_NONE;
	case Common::KEYCODE_RIGHT:
		return KBD_NAV_RIGHT;
	case Common::KEYCODE_KP6:
		return (kbd.flags & Common::KBD_NUM) == 0 ? KBD_NAV_RIGHT : KBD_NAV_NONE;
	case Common::KEYCODE_UP:
		return KBD_NAV_UP;
	case Common::KEYCODE_KP8:
		return (kbd.flags & Common::KBD_NUM) == 0 ? KBD_NAV_UP : KBD_NAV_NONE;
	case Common::KEYCODE_DOWN:
		return KBD_NAV_DOWN;
	case Common::KEYCODE_KP2:
		return (kbd.flags & Common::KBD_NUM) == 0 ? KBD_NAV_DOWN : KBD_NAV_NONE;
	case Common::KEYCODE_PAGEUP:
		return KBD_NAV_PAGEUP;
	case Common::KEYCODE_KP9:
		return (kbd.flags & Common::KBD_NUM) == 0 ? KBD_NAV_PAGEUP : KBD_NAV_NONE;
	case Common::KEYCODE_PAGEDOWN:
		return KBD_NAV_PAGEDOWN;
	case Common::KEYCODE_KP3:
		return (kbd.flags & Common::KBD_NUM) == 0 ? KBD_NAV_PAGEDOWN : KBD_NAV_NONE;
	default:
		return KBD_NAV_NONE;
	}
}

void ZoombiniPage::onAnimFrame() {
	// Snoid animation advances inside its pre-render callback in @ref ZoombiniPage::renderFeatures().
	renderFeatures();
	checkCloseFeatures();
}

bool ZoombiniPage::isClosed() {
	return _isClosed;
}

void ZoombiniPage::close() {
	if (_isClosed)
		return;

	restoreSnoidDragFidgets();
	if (_pageCategory != ZoombiniPageCategory::kDialog)
		_vm->_sound->releaseAllLoadedSounds();
	onFadeOut();
	_isClosed = true;
}

void ZoombiniPage::closeForQuit() {
	restoreSnoidDragFidgets();
	// Run the active page's palette cleanup after the save-before-quit prompt.
	// Do not let an already-closed page suppress this shutdown-specific fade.
	if (_pageCategory != ZoombiniPageCategory::kDialog)
		_vm->_sound->releaseAllLoadedSounds();
	onFadeOut();
	_isClosed = true;
}

void ZoombiniPage::onFrame() {
	// A queued fade keeps the old page object alive after @ref ZoombiniPage::close().
	// Freeze the final composite instead of rendering cleanup-only runner state.
	if (_isClosed)
		return;

	_currentFrameTime = _vm->_system->getMillis();
	_currentFrameCounter = _vm->getAnimationFrameCounter(_currentFrameTime);

	onEveryFrame();

	if (_currentFrameCounter != _lastFrameCounter || _doForceRedraw) {
		if (_builtinDebugTextRestorePending) {
			addExternalDirtyRect(Common::Rect(0, 0, 310, 16));
			_builtinDebugTextRestorePending = false;
		}
		do {
			_forceRedrawPending |= _doForceRedraw;
			_doForceRedraw = false;
			onAnimFrame();
		} while (_doForceRedraw);

		_lastFrameTime = _currentFrameTime;
		_lastFrameCounter = _currentFrameCounter;
	}

	// Holding still is activity too; neither idle timer may accumulate during the drag.
	if (_snoidDragFidgetsSuppressed)
		_vm->resetFidgetActivity();
}

void ZoombiniPage::onModalFrame() {
	if (_isClosed)
		return;

	_currentFrameTime = _vm->_system->getMillis();
	_currentFrameCounter = _vm->getAnimationFrameCounter(_currentFrameTime);
}

void ZoombiniPage::openArchive(const Common::String &mhkName) {
	const Common::Path mhkPath = Common::Path(_vm->getArchiveRoot()).append(mhkName);

	MohawkArchive *mhk = new MohawkArchive();
	if (!mhk->openFile(mhkPath)) {
		delete mhk;
		error("page: cannot open or parse required resource file '%s'", mhkPath.toString().c_str());
		return;
	}

	_vm->addPageArchive(mhk);
}

void ZoombiniPage::openMidiArchive() {
	// The engine owns the selected v1.x MIDI archive for the whole game.
	// Pages retain this helper to ensure that a profile change takes effect at
	// their next open boundary without placing the archive in the page stack.
	_vm->loadMidiArchive();
}

ZmbEventHandleResult ZoombiniPage::handleLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	// Manual sticky mouse (Ctrl+J) and auto-sticky mouse (Ctrl+U) both route a
	// picked-up item through the click-to-pick / click-to-drop path, so a click
	// on an active (sticky) drag drops it.
	if ((_vm->_state->getEnableStickyMouse() || _vm->_state->getEnableAutoStickyMouse()) && hasStickyMouseDrag()) {
		// A direct touchscreen tap can report its simulated button-down at the
		// destination without a preceding mouse-move event. Apply the final
		// position and drop highlight first.
		onMouseMove(absPos, relPos);
		endStickyMouseDrag(absPos);
		return ZmbEventHandleResult::kConsumed;
	}
	return onLButtonDown(absPos, relPos);
}

ZmbEventHandleResult ZoombiniPage::handleLButtonUp(const Common::Point &absPos, const Common::Point &relPos) {
	if (hasStickyMouseDrag()) {
		// Manual sticky mouse (Ctrl+J): a picked-up item never drops on release.
		if (_vm->_state->getEnableStickyMouse())
			return ZmbEventHandleResult::kConsumed;

		// Auto-sticky mouse (Ctrl+U): convert a held drag to sticky on release,
		// unless rendering stalled during the current frame.
		// Compare elapsed frame ticks with the persisted threshold (30 frames, or about 0.5 seconds, by default).
		if (_vm->_state->getEnableAutoStickyMouse()) {
			const uint32 nowFrameCounter = _vm->getAnimationFrameCounter(_vm->_system->getMillis());
			const uint32 framesSinceRender =
				(_lastFrameCounter < nowFrameCounter) ? (nowFrameCounter - _lastFrameCounter) : 0;
			if (framesSinceRender < _vm->_state->getAutoStickyThreshold())
				return ZmbEventHandleResult::kConsumed;
		}
	}
	return onLButtonUp(absPos, relPos);
}

ZmbEventHandleResult ZoombiniPage::handleRButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	// A sticky drag keeps primary-click ownership even when the backend emits a
	// secondary click, so the click still completes the active drag.
	if ((_vm->_state->getEnableStickyMouse() || _vm->_state->getEnableAutoStickyMouse()) && hasStickyMouseDrag())
		return handleLButtonDown(absPos, relPos);

	const ZmbEventHandleResult result = onRButtonDown(absPos, relPos);
	if (result == ZmbEventHandleResult::kConsumed)
		return result;
	return handleLButtonDown(absPos, relPos);
}

ZmbEventHandleResult ZoombiniPage::handleRButtonUp(const Common::Point &absPos, const Common::Point &relPos) {
	const ZmbEventHandleResult result = onRButtonUp(absPos, relPos);
	if (result == ZmbEventHandleResult::kConsumed)
		return result;
	return handleLButtonUp(absPos, relPos);
}

ZmbEventHandleResult ZoombiniPage::onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	ZmbEventHandleResult result = ZmbEventHandleResult::kPassthrough;
	Common::Array<ZmbFeature *> eventList;
	buildSortedEventList(eventList);
	for (ZmbFeature *feature : eventList) {
		result = feature->onLButtonDown(this, absPos, relPos);
		if (result == ZmbEventHandleResult::kConsumed)
			break;
	}
	return result;
}

ZmbEventHandleResult ZoombiniPage::onLButtonUp(const Common::Point &absPos, const Common::Point &relPos) {
	ZmbEventHandleResult result = ZmbEventHandleResult::kPassthrough;
	Common::Array<ZmbFeature *> eventList;
	buildSortedEventList(eventList);
	for (ZmbFeature *feature : eventList) {
		result = feature->onLButtonUp(this, absPos, relPos);
		if (result == ZmbEventHandleResult::kConsumed)
			break;
	}
	return result;
}

ZmbEventHandleResult ZoombiniPage::onWheelUp(const Common::Point &absPos) {
	ZmbEventHandleResult result = ZmbEventHandleResult::kPassthrough;
	Common::Array<ZmbFeature *> eventList;
	buildSortedEventList(eventList);
	for (ZmbFeature *feature : eventList) {
		result = feature->onWheelUp(this, absPos);
		if (result == ZmbEventHandleResult::kConsumed)
			break;
	}
	return result;
}

ZmbEventHandleResult ZoombiniPage::onWheelDown(const Common::Point &absPos) {
	ZmbEventHandleResult result = ZmbEventHandleResult::kPassthrough;
	Common::Array<ZmbFeature *> eventList;
	buildSortedEventList(eventList);
	for (ZmbFeature *feature : eventList) {
		result = feature->onWheelDown(this, absPos);
		if (result == ZmbEventHandleResult::kConsumed)
			break;
	}
	return result;
}

ZmbEventHandleResult ZoombiniPage::onMouseMove(const Common::Point &absPos, const Common::Point &relPos) {
	ZmbEventHandleResult result = ZmbEventHandleResult::kPassthrough;
	Common::Array<ZmbFeature *> eventList;
	buildSortedEventList(eventList);
	for (ZmbFeature *feature : eventList) {
		result = feature->onMouseMove(this, absPos, relPos);
		if (result == ZmbEventHandleResult::kConsumed)
			break;
	}
	return result;
}

ZmbEventHandleResult ZoombiniPage::onKeyDown(const Common::KeyState &kbd, bool kbdRepeat) {
	ZmbEventHandleResult result = ZmbEventHandleResult::kPassthrough;
	Common::Array<ZmbFeature *> eventList;
	buildSortedEventList(eventList);
	for (ZmbFeature *feature : eventList) {
		result = feature->onKeyDown(this, kbd, kbdRepeat);
		if (result == ZmbEventHandleResult::kConsumed)
			break;
	}
	return result;
}

ZmbEventHandleResult ZoombiniPage::onKeyUp(const Common::KeyState &kbd, bool kbdRepeat) {
	ZmbEventHandleResult result = ZmbEventHandleResult::kPassthrough;
	Common::Array<ZmbFeature *> eventList;
	buildSortedEventList(eventList);
	for (ZmbFeature *feature : eventList) {
		result = feature->onKeyUp(this, kbd, kbdRepeat);
		if (result == ZmbEventHandleResult::kConsumed)
			break;
	}
	return result;
}

ZmbEventHandleResult ZoombiniPage::onImeComposition(const Common::ImeComposition &composition) {
	(void)composition;
	return ZmbEventHandleResult::kPassthrough;
}

ZmbEventHandleResult ZoombiniPage::onQuit() {
	// There is no per-feature quit callback.
	// Page subclasses can override this for custom cleanup.
	return ZmbEventHandleResult::kPassthrough;
}

ZmbFeature *ZoombiniPage::loadScrbFeature(ZmbResource imgResource, int16 scrbId, uint32 frameInterval, uint32 flags, const ZmbFeature::EventHooks &eventHooks) {
	return registerFeature(this, _scrbFeatures, imgResource, static_cast<uint16>(scrbId), scrbId, frameInterval, Common::Point(0, 0), flags, nullptr, eventHooks);
}

ZmbFeature *ZoombiniPage::loadScrbFeature(ZmbResource imgResource, int16 scrbId, uint32 frameInterval, const Common::Point &pointRef, uint32 flags, const ZmbFeature::EventHooks &eventHooks) {
	return registerFeature(this, _scrbFeatures, imgResource, static_cast<uint16>(scrbId), scrbId, frameInterval, pointRef, flags, nullptr, eventHooks);
}

ZmbFeature *ZoombiniPage::loadScrbFeature(ZmbResource imgResource, int16 scrbId, const Common::Array<ZmbHotspot> &hotspots, uint32 frameInterval, uint32 flags, const ZmbFeature::EventHooks &eventHooks) {
	return registerFeature(this, _scrbFeatures, imgResource, static_cast<uint16>(scrbId), scrbId, frameInterval, Common::Point(0, 0), flags, &hotspots, eventHooks);
}

ZmbFeature *ZoombiniPage::loadVirtualFeature(ZmbResource imgResource, uint16 runnerId, uint32 frameInterval, uint32 flags, const ZmbFeature::EventHooks &eventHooks) {
	return registerFeature(this, _scrbFeatures, imgResource, runnerId, 0, frameInterval, Common::Point(0, 0), flags, nullptr, eventHooks);
}

ZmbFeature *ZoombiniPage::registerFeature(ZoombiniPage *page, ZmbFeatureList<ZmbFeature> &featureList, ZmbResource imgRes, uint16 runnerId, int16 scrbId, uint32 frameInterval, const Common::Point &pointRef, uint32 flags, const Common::Array<ZmbHotspot> *virtualHotspots, const ZmbFeature::EventHooks &eventHooks) {
	// Assign a unique registration index to each runner.
	// Multiple runners may share the same SCRB resource ID.

	ZmbFeature *feature = new ZmbFeature(page->_vm, runnerId, scrbId, frameInterval, pointRef, flags, imgRes);
	featureList.insert(runnerId, feature);
	feature->setRegistrationIndex(page->_nextRegistrationIndex);
	page->_nextRegistrationIndex += 1;
	// A nonzero scrbId loads SCRB data onto the runner.
	// A zero ID creates a callback-only runner with no SCRB data.
	// The runner relies entirely on its pre-render and post-render callbacks for drawing.
	if (0 < scrbId) {
		page->_scrbImageResources[scrbId] = imgRes;
		const ZmbScriptDecoder::DecodedScrb *decodedScrb = page->getDecodedScrb(ZmbResource(imgRes._archiveKind, scrbId));
		if (!feature->setDecodedScrb(decodedScrb, scrbId)) {
			error("page: required SCRB %d is malformed", scrbId);
			return nullptr;
		}
		feature->setInitialScrbLoadPending(true);
	}
	if (virtualHotspots) {
		feature->setVirtualHotspots(*virtualHotspots);
	}

	feature->initValues();

	// Always register hooks for normal per-frame rendering
	feature->setEventHooks(eventHooks);

	// DRAW_ON_REG features run their initial pre-render during registration.
	// This materializes frame zero and its click coverage without drawing pixels.
	// Deferred flags then leave the dormant drop target hidden until it is activated.
	if (feature->hasFlag(ZmbFeature::FLAG_00002000_DRAW_ON_REG)) {
		feature->onPreRender(page);

		// Auto-populate a draw-on-reg slot when @ref ZmbFeature::FLAG_00002000_DRAW_ON_REG is set.
		// Store the runner and registration position as snap position.
		// Pages with custom snap offsets call @ref ZoombiniPage::setDrawOnRegSnapPosition() afterward to override it.
		if (page->_drawOnRegCount < kMaxDrawOnRegSlots) {
			page->_drawOnRegFeatures[page->_drawOnRegCount] = feature;
			page->_drawOnRegSnapPositions[page->_drawOnRegCount] = pointRef;
			page->_drawOnRegOccupancy[page->_drawOnRegCount] = 0;
			page->_drawOnRegCount += 1;
		}
	}

	return feature;
}

ZmbFeature *ZoombiniPage::loadSubFeature(ZmbFeature *parentFeature, ZmbResource imgResource, int16 scrbId) {
	uint32 flags = parentFeature->getFlags();
	if ((flags & ZmbFeature::FLAG_02000000_RANDOM_FRAME) != 0) {
		flags &= ~ZmbFeature::FLAG_02000000_RANDOM_FRAME;
	}

	// The sub-feature inherits the parent feature's frame interval and adjusted flags.
	ZmbFeature *subFeature = new ZmbFeature(_vm, static_cast<uint16>(scrbId), scrbId, parentFeature->getFrameInterval(), flags, imgResource);
	subFeature->setRegistrationIndex(_nextRegistrationIndex);
	_nextRegistrationIndex += 1;
	_scrbImageResources[scrbId] = imgResource;

	const ZmbScriptDecoder::DecodedScrb *decodedScrb = getDecodedScrb(ZmbResource(imgResource._archiveKind, scrbId));
	if (!subFeature->setDecodedScrb(decodedScrb, scrbId)) {
		error("page: required sub-feature SCRB %d is malformed", scrbId);
		return nullptr;
	}
	subFeature->setInitialScrbLoadPending(true);
	subFeature->initValues();
	subFeature->setEventHooks(ZmbFeature::EventHooks());

	parentFeature->setSubFeature(subFeature);

	return subFeature;
}

ZmbFeature *ZoombiniPage::createMainFeatureHead(uint32 flags) {
	ZmbFeature *head = new ZmbFeature(_vm, 0, 0, 0, flags, ZmbResource(ZmbResource::kPage, 0));
	head->initValues();
	_mainFeatureHeads.push_back(head);
	return head;
}

void ZoombiniPage::registerScrbImageGroup(ZmbResource imgResource, int16 scrbBaseId, uint16 scrbCount) {
	for (uint16 i = 0; i < scrbCount; i++)
		_scrbImageResources[static_cast<int16>(scrbBaseId + i)] = imgResource;
}

void ZoombiniPage::unloadScrbFeature(ZmbFeature *feature) {
	// The previous frame's pre-render already added the removed feature's old click rectangle.
	// The external dirty accumulator still contains it.
	// We add the feature's sortRect as an external dirty rect so the area gets background-restored next frame.
	const Common::Rect &oldRect = feature->getZSortRect();
	if (!oldRect.isEmpty())
		addExternalDirtyRect(oldRect);
	runnerOrderErase(feature);
	deregisterFeature(_scrbFeatures, feature);
}

void ZoombiniPage::loadScrbOntoFeature(ZmbFeature *feature, int16 newScrbId, bool scheduleRender,
									   bool invalidateCurrentCoverage) {
	// Page-level convenience wrapper.
	// Resolve the SCRB resource and delegate to @ref ZmbFeature::loadScrbData().
	if (!feature) {
		error("page: cannot load SCRB onto a null feature");
		return;
	}

	// A zero @p newScrbId reloads the runner's current resource ID.
	int16 scrbId = newScrbId;
	if (scrbId == 0)
		scrbId = feature->getScrbId();

	// A runtime load on a CHAIN_SCRIPT runner first preserves the current SCRB as its one-shot return target.
	// RANDOM_FRAME becomes a negative target so the replacement plays sequentially.
	// The previous SCRB resumes in random mode when the replacement finishes.
	// A nonzero target means a replacement is already active, so a repeated load is ignored.
	if (feature->hasFlag(ZmbFeature::FLAG_00040000_CHAIN_SCRIPT)) {
		if (feature->getChainedScrbId() != 0)
			return;

		int16 currentScrbId = feature->getScrbId();
		if (feature->hasFlag(ZmbFeature::FLAG_02000000_RANDOM_FRAME)) {
			feature->removeFlag(ZmbFeature::FLAG_02000000_RANDOM_FRAME);
			feature->setChainedScrbId(-currentScrbId);
		} else {
			feature->setChainedScrbId(currentScrbId);
		}
	}

	// Loading the new SCRB updates its resource ID immediately.
	// The runner can then select another tBMP/REGS group without changing identity.
	ZmbResource imageResource = feature->getResource();
	Common::HashMap<int16, ZmbResource>::iterator imageResourceIt = _scrbImageResources.find(scrbId);
	if (imageResourceIt != _scrbImageResources.end())
		imageResource = imageResourceIt->_value;

	const ZmbScriptDecoder::DecodedScrb *decodedScrb = getDecodedScrb(ZmbResource(imageResource._archiveKind, scrbId));

	if (!feature->getZSortRect().isEmpty()) {
		if (invalidateCurrentCoverage)
			markFeatureVisualCoverageDirty(feature, false);
		addExternalDirtyRect(feature->getZSortRect());
	}

	feature->setResource(imageResource);
	feature->setShapeOffsetRegs(nullptr);
	Common::HashMap<int16, ZmbShapeOffsetRegs *>::iterator shapeRegsIt = _shapeOffsetRegsMap.find(imageResource._id);
	if (shapeRegsIt != _shapeOffsetRegsMap.end())
		feature->setShapeOffsetRegs(shapeRegsIt->_value);

	feature->loadScrbData(decodedScrb, scrbId, scheduleRender);
}

void ZoombiniPage::playCurrentFrameSound(ZmbFeature *feature) {
	if (!feature)
		return;

	const int32 frameIdx = feature->getLastFrameIdx();
	ZmbResource resource;
	if (feature->getFrameSoundResource(frameIdx, resource)) {
		dispatchFeatureSound(feature, resource);
		feature->setLastSoundedFrameIdx(frameIdx);
	}
}

void ZoombiniPage::dispatchFeatureSound(ZmbFeature *feature, ZmbResource resource) {
	const Audio::Mixer::SoundType soundType = getFeatureSoundType(feature, resource);
	if (shouldQueueFeatureSound(feature)) {
		const bool forcePriority = feature &&
								   feature->getScriptSoundPolicy() ==
									   ZmbFeature::ScriptSoundPolicy::kForcedPriorityQueue;
		_vm->_sound->enqueueScriptSound(resource, soundType, forcePriority);
	} else {
		feature->playOwnedSound(resource, soundType);
	}
}

void ZoombiniPage::queueScriptSoundForNextRenderPass(ZmbResource resource, Audio::Mixer::SoundType soundType) {
	_vm->_sound->enqueueScriptSound(resource, soundType);
}

ZmbResource ZoombiniPage::resolveSnoidVoiceResource(int16 soundId) const {
	return ZmbResource(ZmbResource::kSystem, soundId);
}

void ZoombiniPage::attachSubFeature(ZmbFeature *subFeature) {
	// Guard against duplicate registration (e.g. user clicks before animation finishes).
	// The caller in zoombini_scripts.cpp already checks @ref ZmbFeature::isSubFeatureRunning(),
	// but double-check here as a safety net.
	if (subFeature->isSubFeatureRunning())
		return;

	// Insert without taking ownership - the parent feature still owns this pointer.
	// Duplicate scrbId keys are allowed (the list always appends).
	_subFeatures.insert(subFeature->getId(), subFeature);
}

void ZoombiniPage::collectDebugLoadedFeatures(Common::Array<ZmbLoadedFeatureInfo> &features) const {
	for (const ZmbFeature *feature : _scrbFeatures)
		features.push_back(ZmbLoadedFeatureInfo(feature, ZmbLoadedFeatureListKind::kScrb));
	for (const ZmbFeature *feature : _subFeatures)
		features.push_back(ZmbLoadedFeatureInfo(feature, ZmbLoadedFeatureListKind::kSub));
	for (const ZmbSnoid *snoid : _snoidMap)
		features.push_back(ZmbLoadedFeatureInfo(snoid, ZmbLoadedFeatureListKind::kSnoid));
}

void ZoombiniPage::deregisterFeature(ZmbFeatureList<ZmbFeature> &featureList, ZmbFeature *feature) {
	if (!feature) {
		error("page: cannot unload a null feature");
		return;
	}
	featureList.eraseByPtr(feature, feature->getId());
	delete feature;
}

void ZoombiniPage::loadNodePath(ZmbResource nodeResource) {
	ZmbNode *node = new ZmbNode();
	Common::SeekableReadStream *stream = _vm->getResource(ID_NODE, nodeResource);
	if (!node->parseStream(stream)) {
		delete node;
		error("page: required NODE %d is malformed", nodeResource._id);
		return;
	}
	// PATH and NODE share a resource number and are required together for routing.
	if (!_vm->hasResource(ID_PATH, nodeResource)) {
		delete node;
		error("page: required PATH %d is missing", nodeResource._id);
		return;
	}
	Common::SeekableReadStream *pathStream = _vm->getResource(ID_PATH, nodeResource);
	if (!node->parsePathStream(pathStream)) {
		delete node;
		error("page: required PATH %d is malformed", nodeResource._id);
		return;
	}
	_nodeMap[nodeResource._id] = node;
}

void ZoombiniPage::loadShapeOffsetRegs(ZmbResource::ArchiveKind archiveKind, int16 baseResId) {
	ZmbShapeOffsetRegs *regs = new ZmbShapeOffsetRegs();
	regs->parseStreams(_vm, archiveKind, baseResId, static_cast<int16>(baseResId + 1));
	_shapeOffsetRegsMap[baseResId] = regs;
}

/**
 * Categorize the feature into one of the 4 render groups based on its flags.
 * Check order matters.
 *
 * Classify @p feature into one of the four output lists.
 * @ref ZmbFeature::FLAG_00008000_LOOP_ANIM has the highest priority.
 * It overrides a pre-existing @ref ZmbFeature::FLAG_04000000_OVERLAY and the entity-type flags.
 * Most Snoids use only @ref ZmbFeature::FLAG_00000001_TYPE_SNOID and are depth-sorted.
 * Pages such as Caves add @ref ZmbFeature::FLAG_00008000_LOOP_ANIM after seating a Snoid
 * to preserve an explicit runner-list position.
 */
void ZoombiniPage::categorizeFeature(ZmbFeature *feature, Common::Array<ZmbFeature *> &loopAnimList, Common::Array<ZmbFeature *> &overlayList, Common::Array<ZmbFeature *> &normalList, Common::Array<ZmbFeature *> &entityList) {
	// Check order matters.
	// LOOP_ANIM takes precedence over the exact entity-type checks below.
	if (feature->hasFlag(ZmbFeature::FLAG_00008000_LOOP_ANIM)) {
		loopAnimList.push_back(feature);
	} else if (feature->hasFlag(ZmbFeature::FLAG_04000000_OVERLAY)) {
		// Preserve the current global runner-list order for overlay features.
		overlayList.push_back(feature);
	} else if (feature->getFlags() == ZmbFeature::FLAG_00000001_TYPE_SNOID || feature->getFlags() == ZmbFeature::FLAG_00000002_TYPE_GRIDWALKER) {
		// Only bare Snoid or Town-entity flags enter the entity list.
		// Snoids with additional flags enter the normal list.
		entityList.push_back(feature);
	} else {
		// Set OVERLAY on non-TOPMOST normal features.
		// On the next frame they enter the OVERLAY branch and retain their current position in the global runner list.
		if (!feature->hasFlag(ZmbFeature::FLAG_00001000_TOPMOST))
			feature->addFlag(ZmbFeature::FLAG_04000000_OVERLAY);
		normalList.push_back(feature);
	}
}

/**
 * Sort features by ascending click-rectangle bottom and left coordinates.
 *
 * Uses the current visual rectangle from @ref ZmbFeature::getZSortRect() for stable Z-ordering.
 * The method reads @ref ZmbFeature::_sortRect while preserving manual click zones separately.
 *
 * A topmost incoming feature always traverses to the current tail.
 * Existing topmost entries remain ordinary comparison nodes for later incoming features.
 */
void ZoombiniPage::insertionSortFeatures(Common::Array<ZmbFeature *> &list) {
	if (list.size() <= 1)
		return;

	Common::Array<ZmbFeature *> sorted;
	for (ZmbFeature *incoming : list) {
		uint32 insertPos = 0;
		if (incoming->hasFlag(ZmbFeature::FLAG_00001000_TOPMOST)) {
			insertPos = sorted.size();
		} else {
			const Common::Rect &incomingRect = incoming->getZSortRect();
			while (insertPos < sorted.size()) {
				const Common::Rect &existingRect = sorted[insertPos]->getZSortRect();
				if (incomingRect.bottom < existingRect.bottom ||
					(incomingRect.bottom == existingRect.bottom &&
					 incomingRect.left < existingRect.left))
					break;
				insertPos += 1;
			}
		}
		sorted.insert_at(insertPos, incoming);
	}
	list = sorted;
}

/**
 * Merge @p incomingList into @p existingList using the bottom and left coordinates as the sort key.
 * The existing list may already contain loop-animation, overlay, and previously merged entries.
 *
 * Skip leading @ref ZmbFeature::FLAG_00008000_LOOP_ANIM entries before scanning for the insertion point.
 * Append an incoming @ref ZmbFeature::FLAG_00001000_TOPMOST entry to the tail.
 * Insert ordinary incoming entries before an existing @ref ZmbFeature::FLAG_00001000_TOPMOST entry.
 * When sort keys match, apply the existing entry's ZSORT constraints.
 * A vertical gap always permits insertion; otherwise the left, right, and bottom constraints can block it.
 * Continue scanning from the existing node that follows the inserted entry because the incoming list is sorted.
 *
 * This runs first for normal features and then for entity features.
 */
void ZoombiniPage::mergeSortedListInto(Common::Array<ZmbFeature *> &existingList, const Common::Array<ZmbFeature *> &incomingList) {
	if (incomingList.empty())
		return;

	// Find scan start: skip past LOOP_ANIM entries in existing list.
	// Overlay items with higher sort keys draw after entities with lower sort keys.
	// This lets foreground foliage cover Snoid feet.
	// The dirty-region clip restricts drawing to changed pixels.
	// Pixels from non-dirty features persist on the shape screen from the previous frame.
	uint32 scanStart = 0;
	while (scanStart < existingList.size() &&
		   existingList[scanStart]->hasFlag(ZmbFeature::FLAG_00008000_LOOP_ANIM) &&
		   scanStart + 1 < existingList.size()) {
		scanStart += 1;
	}

	uint32 scanPos = scanStart;

	for (uint32 incomingIdx = 0; incomingIdx < incomingList.size(); incomingIdx++) {
		ZmbFeature *incoming = incomingList[incomingIdx];

		// TOPMOST incoming: append at tail.
		if (incoming->hasFlag(ZmbFeature::FLAG_00001000_TOPMOST)) {
			existingList.push_back(incoming);
			continue;
		}

		const Common::Rect &inRect = incoming->getZSortRect();
		uint32 insertPos = existingList.size(); // default: append at end
		bool found = false;

		for (uint32 listIdx = scanPos; listIdx < existingList.size(); listIdx++) {
			ZmbFeature *existing = existingList[listIdx];

			// TOPMOST existing: insert incoming before it.
			if (existing->hasFlag(ZmbFeature::FLAG_00001000_TOPMOST)) {
				insertPos = listIdx;
				found = true;
				break;
			}

			// The last existing entry is an unconditional append target.
			// No later sort-key or ZSORT constraint check applies.
			// This behavior is critical for Ferry seats.
			// A Snoid whose bounding box fits inside a seat would otherwise pass every ZSORT constraint.
			// It would then be inserted before the seat and rendered behind it.
			if (existingList.size() <= listIdx + 1) {
				// Append after the last entry; the insertion position retains the existing list length.
				found = true;
				break;
			}

			const Common::Rect &exRect = existing->getZSortRect();

			// Sort key comparison: incoming should go before existing?
			// Incoming.bottom < existing.bottom, or (equal bottom and incoming.left < existing.left).
			bool candidatePosition = inRect.bottom < exRect.bottom || (inRect.bottom == exRect.bottom && inRect.left < exRect.left);
			if (!candidatePosition)
				continue;

			// ZSORT constraint check on existing entry.
			uint32 exFlags = existing->getFlags();

			// No vertical overlap -> always allow.
			bool noVerticalOverlap = inRect.bottom < exRect.top;

			if (!noVerticalOverlap) {
				// Check each active ZSORT constraint; any violation blocks insertion.
				if ((exFlags & ZmbFeature::FLAG_40000000_ZSORT_LEFT) && inRect.left < exRect.left)
					continue; // ZSORT_LEFT violated
				if ((exFlags & ZmbFeature::FLAG_10000000_ZSORT_RIGHT) && exRect.right < inRect.right)
					continue; // ZSORT_RIGHT violated
				if ((exFlags & ZmbFeature::FLAG_20000000_ZSORT_BOTTOM) && inRect.top < exRect.top)
					continue; // ZSORT_BOTTOM violated
			}

			insertPos = listIdx;
			found = true;
			break;
		}

		existingList.insert_at(insertPos, incoming);

		// After insertion, remember the existing entry that received it.
		// The next incoming entry resumes scanning there.
		// If appended at end (not found), scanPos stays where it was.
		if (found) {
			scanPos = insertPos + 1; // +1 because we inserted before it, shifting it right
		}
	}
}

/**
 *
 * Build the final render list in this merge order:
 * 1. Partition into loopAnimList, overlayList, normalList, entityList
 * 2. Assemble combined = loopAnimList + overlayList (unsorted)
 * 3. Sort normalList -> merge into combined (with ZSORT constraints)
 * 4. Sort entityList -> merge into combined (with ZSORT constraints)
 *
 * Both merge passes interleave entries by sort key while respecting ZSORT protection flags on existing entries.
 * This means normal entries can be interleaved with overlay entries (not kept strictly separate).
 */
void ZoombiniPage::syncRunnerOrder() {
	// Drop entries whose features were deregistered (paranoia - the delete
	// paths call @ref ZoombiniPage::runnerOrderErase(), but a stale pointer here would be fatal).
	// Append any missing live features in registration-index order.
	// This preserves registration order at the tail of the runner list.
	Common::Array<ZmbFeature *> live;
	for (ZmbFeature *f : _scrbFeatures)
		live.push_back(f);
	for (ZmbFeature *f : _subFeatures)
		live.push_back(f);
	for (ZmbSnoid *s : _snoidMap)
		live.push_back(s);

	uint32 writeIdx = 0;
	for (uint32 runnerIdx = 0; runnerIdx < _runnerOrder.size(); runnerIdx++) {
		if (Common::find(live.begin(), live.end(), _runnerOrder[runnerIdx]) != live.end()) {
			_runnerOrder[writeIdx] = _runnerOrder[runnerIdx];
			writeIdx += 1;
		}
	}
	_runnerOrder.resize(writeIdx);

	Common::Array<ZmbFeature *> missing;
	for (ZmbFeature *f : live) {
		if (Common::find(_runnerOrder.begin(), _runnerOrder.end(), f) == _runnerOrder.end())
			missing.push_back(f);
	}
	// Insertion sort the additions by registration index (small N).
	for (uint32 missingIdx = 1; missingIdx < missing.size(); missingIdx++) {
		ZmbFeature *key = missing[missingIdx];
		int32 insertionIdx = static_cast<int32>(missingIdx) - 1;
		while (0 <= insertionIdx && key->getRegistrationIndex() < missing[insertionIdx]->getRegistrationIndex()) {
			missing[insertionIdx + 1] = missing[insertionIdx];
			insertionIdx -= 1;
		}
		missing[insertionIdx + 1] = key;
	}
	for (ZmbFeature *f : missing)
		_runnerOrder.push_back(f);
}

void ZoombiniPage::runnerOrderErase(ZmbFeature *feature) {
	for (uint32 i = 0; i < _runnerOrder.size(); i++) {
		if (_runnerOrder[i] == feature) {
			_runnerOrder.remove_at(i);
			return;
		}
	}
}

void ZoombiniPage::manualLinkBefore(ZmbFeature *feature, ZmbFeature *target) {
	// Detach, insert before parent.
	if (!feature || !target || feature == target)
		return;

	// Detaching a runner invalidates the coverage it occupies right now. A change that
	// only reorders draw depth still has to repaint that area with the new stacking,
	// so the footprint is reported before the list is touched.
	addExternalDirtyRect(feature->getClickRect());

	// Re-linking can occur before the first frame.
	// Synchronize the lazily populated runner-order array before looking up either endpoint.
	syncRunnerOrder();
	runnerOrderErase(feature);
	for (uint32 i = 0; i < _runnerOrder.size(); i++) {
		if (_runnerOrder[i] == target) {
			_runnerOrder.insert_at(i, feature);
			return;
		}
	}
	_runnerOrder.push_back(feature);
}

void ZoombiniPage::manualLinkAfter(ZmbFeature *feature, ZmbFeature *target) {
	// Detach, insert after parent.
	if (!feature || !target || feature == target)
		return;

	// See @ref ZoombiniPage::manualLinkBefore(): a detach invalidates the runner's footprint.
	addExternalDirtyRect(feature->getClickRect());

	// See @ref ZoombiniPage::manualLinkBefore():
	// pre-first-frame links must operate on the same complete runner list required by pre-first-frame linking.
	syncRunnerOrder();
	runnerOrderErase(feature);
	for (uint32 i = 0; i < _runnerOrder.size(); i++) {
		if (_runnerOrder[i] == target) {
			_runnerOrder.insert_at(i + 1, feature);
			return;
		}
	}
	_runnerOrder.push_back(feature);
}

void ZoombiniPage::manualLinkAtEnd(ZmbFeature *feature) {
	if (!feature)
		return;

	// See @ref ZoombiniPage::manualLinkBefore(): a detach invalidates the runner's footprint.
	addExternalDirtyRect(feature->getClickRect());

	syncRunnerOrder();
	runnerOrderErase(feature);
	_runnerOrder.push_back(feature);
}

void ZoombiniPage::setRunnerZSortEnabled(bool enabled) {
	if (!enabled)
		syncRunnerOrder();
	_manualZOrder = !enabled;
}

void ZoombiniPage::forceRunnerZSort() {
	// Run exactly one positional sort pass even while manual Z-order mode is active.
	// Callers use this after changing flags on runners that must be re-bucketed immediately,
	// where waiting for the next enabled sort would leave them in a stale bucket.
	const bool savedManualZOrder = _manualZOrder;
	_manualZOrder = false;
	Common::Array<ZmbFeature *> sorted;
	buildSortedRenderList(sorted);
	_manualZOrder = savedManualZOrder;
}

void ZoombiniPage::collectSnoidsInRunnerOrder(Common::Array<ZmbSnoid *> &snoids) const {
	snoids.clear();

	for (ZmbFeature *feature : _runnerOrder) {
		for (ZmbSnoid *snoid : _snoidMap) {
			if (feature == snoid) {
				snoids.push_back(snoid);
				break;
			}
		}
	}

	// Runners created after the last snapshot are linked at the global tail in
	// registration order. This also covers the first render.
	for (ZmbSnoid *snoid : _snoidMap) {
		if (Common::find(snoids.begin(), snoids.end(), snoid) == snoids.end())
			snoids.push_back(snoid);
	}
}

ZmbFeature *ZoombiniPage::findRunnerAtPoint(const Common::Point &pos, uint32 requiredFlags) {
	const uint32 kRunnerFlagMask = 0x007FFFFF;
	const uint32 maskedRequiredFlags = requiredFlags & kRunnerFlagMask;
	syncRunnerOrder();

	for (uint32 runnerIdx = _runnerOrder.size(); 0 < runnerIdx; runnerIdx--) {
		ZmbFeature *feature = _runnerOrder[runnerIdx - 1];
		if (!feature)
			continue;
		if ((feature->getFlags() & kRunnerFlagMask) != maskedRequiredFlags)
			continue;
		if (feature->isPointInClickRect(pos))
			return feature;
	}

	return nullptr;
}

int32 ZoombiniPage::squaredDistanceBetweenPoints(const Common::Point &first, const Common::Point &second) {
	const int32 dx = first.x - second.x;
	const int32 dy = first.y - second.y;
	return dx * dx + dy * dy;
}

void ZoombiniPage::assignIdleSnoidsToSlots(const Common::Point *slotPositions, int16 slotCount,
										   int32 distanceSquared, bool requirePackSnoid, Common::Array<uint16> &assignedRunnerIds) const {
	assignedRunnerIds.clear();
	if (!slotPositions || slotCount <= 0)
		return;

	for (int16 slotIdx = 0; slotIdx < slotCount; slotIdx++)
		assignedRunnerIds.push_back(0);

	Common::Array<ZmbSnoid *> candidates;
	collectSnoidsInRunnerOrder(candidates);
	for (int16 slotIdx = 0; slotIdx < slotCount; slotIdx++) {
		for (ZmbSnoid *candidate : candidates) {
			if (!candidate || !candidate->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID))
				continue;
			if (requirePackSnoid && !candidate->isPackSnoid())
				continue;

			const SnoidAnimState state = candidate->getAnimState();
			if (state != kSnoidAnimState000_Idle && state != kSnoidAnimState003_Flip &&
				state != kSnoidAnimState006_Fidget)
				continue;

			if (distanceSquared <= squaredDistanceBetweenPoints(candidate->getPointLoc(), slotPositions[slotIdx]))
				continue;

			bool alreadyAssigned = false;
			for (int16 priorSlotIdx = 0; priorSlotIdx < slotIdx; priorSlotIdx++) {
				if (assignedRunnerIds[priorSlotIdx] == candidate->getId()) {
					alreadyAssigned = true;
					break;
				}
			}
			if (alreadyAssigned)
				continue;

			assignedRunnerIds[slotIdx] = candidate->getId();
			break;
		}
	}
}

Common::Point ZoombiniPage::findRandomEmptySnoidSlotPosition(const Common::Point *slotPositions,
															 int16 slotCount, int32 distanceSquared, bool scanForwardWhenRandomHigh) const {
	if (!slotPositions || slotCount <= 0)
		return Common::Point();

	Common::Array<uint16> assignedRunnerIds;
	assignIdleSnoidsToSlots(slotPositions, slotCount, distanceSquared, false, assignedRunnerIds);

	int16 selectedSlotIdx = -1;
	const bool scanForward = (50 < _vm->_rnd->getRandomNumber(1, 100)) == scanForwardWhenRandomHigh;
	if (scanForward) {
		for (int16 slotIdx = 0; selectedSlotIdx < 0 && slotIdx < slotCount; slotIdx++) {
			if (assignedRunnerIds[slotIdx] == 0)
				selectedSlotIdx = slotIdx;
		}
	} else {
		for (int16 slotIdx = slotCount - 1; selectedSlotIdx < 0 && 0 <= slotIdx; slotIdx--) {
			if (assignedRunnerIds[slotIdx] == 0)
				selectedSlotIdx = slotIdx;
		}
	}

	if (selectedSlotIdx < 0)
		selectedSlotIdx = 0;
	return slotPositions[selectedSlotIdx];
}

void ZoombiniPage::buildSortedRenderList(Common::Array<ZmbFeature *> &outList) {
	// Manual Z-order mode performs no positional sorting.
	// Render in the current global linked-list order.
	// Drag and manual-link operations have already established the required order.
	if (_manualZOrder) {
		syncRunnerOrder();
		outList = _runnerOrder;
		return;
	}

	Common::Array<ZmbFeature *> loopAnimList, overlayList, normalList, entityList;

	// Step 1: Categorize features into render buckets.
	// Walks the one global runner list.
	// Keep that exact input order so LOOP_ANIM and OVERLAY runners retain explicit drag/re-link mutations.
	syncRunnerOrder();
	for (ZmbFeature *feature : _runnerOrder)
		categorizeFeature(feature, loopAnimList, overlayList, normalList, entityList);

	// Step 2: Assemble combined list = loopAnim + overlay.
	outList.clear();
	for (ZmbFeature *feature : loopAnimList)
		outList.push_back(feature);
	for (ZmbFeature *feature : overlayList)
		outList.push_back(feature);

	// Step 3: Sort normalList (newly categorized features), merge into combined.
	// Merge(sorted, combined)
	insertionSortFeatures(normalList);
	mergeSortedListInto(outList, normalList);

	// Step 4: Sort entityList, merge into combined.
	// Merge(sorted, combined)
	insertionSortFeatures(entityList);
	mergeSortedListInto(outList, entityList);

	// Store the sorted result as the current global runner order.
	_runnerOrder = outList;
}

void ZoombiniPage::buildSortedEventList(Common::Array<ZmbFeature *> &outList) {
	// Event dispatch needs ALL features (including OVERLAY) for correct
	// hit-testing.  We skip the OVERLAY cache here and build the list
	// from scratch, treating OVERLAY features as normal.
	Common::Array<ZmbFeature *> loopAnimList, normalList, entityList;

	for (ZmbFeature *f : _scrbFeatures) {
		if (f->hasFlag(ZmbFeature::FLAG_00008000_LOOP_ANIM))
			loopAnimList.push_back(f);
		else if (f->getFlags() == ZmbFeature::FLAG_00000001_TYPE_SNOID || f->getFlags() == ZmbFeature::FLAG_00000002_TYPE_GRIDWALKER)
			entityList.push_back(f);
		else
			normalList.push_back(f);
	}

	outList.clear();
	for (ZmbFeature *feature : loopAnimList)
		outList.push_back(feature);

	insertionSortFeatures(normalList);
	mergeSortedListInto(outList, normalList);

	insertionSortFeatures(entityList);
	mergeSortedListInto(outList, entityList);
}

bool ZoombiniPage::addDirtyRect(const Common::Rect &rect) {
	Common::Rect clipped = rect;
	clipped.clip(Common::Rect(0, 0, 640, 480));
	if (clipped.isEmpty())
		return false;

	return ZoombiniGraphics::mergeRectIntoRegion(_dirtyRects, clipped);
}

void ZoombiniPage::markFeatureVisualCoverageDirty(ZmbFeature *feature, bool expandRenderClip) {
	if (feature->hasVisualRectConstraint()) {
		const Common::Rect &rect = feature->getVisualRectConstraint();
		if (addDirtyRect(rect) && expandRenderClip)
			_vm->_gfx->addRenderClipRect(rect);
		return;
	}

	bool added = false;

	if (feature->hasFlag(ZmbFeature::FLAG_08000000_REGION_TRACK) && feature->hasDrawRecords()) {
		Common::Array<Common::Rect> rects;
		feature->collectDrawRecordRects(rects);

		for (uint32 i = 0; i < rects.size(); i++) {
			if (addDirtyRect(rects[i])) {
				added = true;
				if (expandRenderClip)
					_vm->_gfx->addRenderClipRect(rects[i]);
			}
		}
	}

	if (!added) {
		const Common::Rect &rect = feature->getZSortRect();
		if (addDirtyRect(rect) && expandRenderClip)
			_vm->_gfx->addRenderClipRect(rect);
	}
}

void ZoombiniPage::markPreparedFeatureVisualCoverageDirty(ZmbFeature *feature, bool expandRenderClip) {
	if (feature->hasVisualRectConstraint()) {
		const Common::Rect &rect = feature->getVisualRectConstraint();
		addDirtyRect(rect);
		if (expandRenderClip)
			_vm->_gfx->addRenderClipRect(rect);
		return;
	}

	if (feature->hasFlag(ZmbFeature::FLAG_08000000_REGION_TRACK) &&
		feature->hasPreparedVisualRects()) {
		Common::Array<Common::Rect> rects;
		feature->collectPreparedVisualRects(rects);
		for (uint32 i = 0; i < rects.size(); i++) {
			addDirtyRect(rects[i]);
			if (expandRenderClip)
				_vm->_gfx->addRenderClipRect(rects[i]);
		}
		return;
	}

	const Common::Rect &rect = feature->getZSortRect();
	addDirtyRect(rect);
	if (expandRenderClip)
		_vm->_gfx->addRenderClipRect(rect);
}

void ZoombiniPage::unregisterFeatureTimingGroup(ZmbFeature *feature) {
	if (!feature)
		return;

	uint32 groupIdx = 0;
	while (groupIdx < _featureTimingGroups.size()) {
		FeatureTimingGroup &group = _featureTimingGroups[groupIdx];
		bool containsFeature = false;
		for (uint32 memberIdx = 0; memberIdx < group._members.size(); memberIdx++) {
			if (group._members[memberIdx] == feature) {
				containsFeature = true;
				break;
			}
		}
		if (!containsFeature) {
			groupIdx += 1;
			continue;
		}

		for (uint32 memberIdx = 0; memberIdx < group._members.size(); memberIdx++)
			group._members[memberIdx]->clearSharedFrameTimingResult();
		_featureTimingGroups.remove_at(groupIdx);
	}
}

void ZoombiniPage::registerFeatureTimingGroup(ZmbFeature *first, ZmbFeature *second) {
	if (!first || !second || first == second)
		return;

	// Clear either runner's previous slot membership first.
	unregisterFeatureTimingGroup(first);
	unregisterFeatureTimingGroup(second);

	FeatureTimingGroup group;
	group._members.push_back(first);
	group._members.push_back(second);
	_featureTimingGroups.push_back(group);
}

void ZoombiniPage::registerFeatureTimingFollower(ZmbFeature *owner, ZmbFeature *follower) {
	if (!owner || !follower || owner == follower)
		return;

	registerFeatureTimingGroup(owner, follower);
	FeatureTimingGroup &group = _featureTimingGroups.back();
	if (group._members.size() == 2 &&
		group._members[0] == owner && group._members[1] == follower) {
		group._runtimeOwner = owner;
		// A follower may be linked from the owner's current frame callback.
		// Preserve that due result so the follower consumes its synchronously
		// materialized frame zero in the same render pass.
		group._cachedTimingState = owner->isFrameTimingReady() ? 1 : 0;
	}
}

void ZoombiniPage::registerAlternatingFeatureTimingGroup(ZmbFeature *first, ZmbFeature *second) {
	if (!first || !second || first == second)
		return;
	registerFeatureTimingGroup(first, second);
	FeatureTimingGroup &group = _featureTimingGroups.back();
	if (group._members.size() == 2 &&
		group._members[0] == first && group._members[1] == second)
		group._alternatePhases = true;
}

void ZoombiniPage::joinFeatureTimingGroup(ZmbFeature *groupMember, ZmbFeature *newMember) {
	if (!groupMember || !newMember || groupMember == newMember)
		return;

	// Join the new member to the timing group that already contains groupMember.
	// The shared group slot coordinates frame timing; it is not an SCRS start-frame index.
	for (uint32 groupIdx = 0; groupIdx < _featureTimingGroups.size(); groupIdx++) {
		FeatureTimingGroup &group = _featureTimingGroups[groupIdx];
		bool containsGroupMember = false;
		bool containsNewMember = false;
		for (uint32 memberIdx = 0; memberIdx < group._members.size(); memberIdx++) {
			containsGroupMember |= group._members[memberIdx] == groupMember;
			containsNewMember |= group._members[memberIdx] == newMember;
		}
		if (!containsGroupMember)
			continue;
		if (!containsNewMember)
			group._members.push_back(newMember);
		return;
	}

	// A standalone runner still owns a timing slot that a newly started follower can share.
	// The owner has already reached the callback that starts the follower, so preserve
	// the current due decision for a follower reached later in this render pass.
	registerFeatureTimingFollower(groupMember, newMember);
}

void ZoombiniPage::setFeatureTimingGroupScriptSoundPolicy(ZmbFeature *groupMember, ZmbFeature::ScriptSoundPolicy policy) {
	if (!groupMember)
		return;

	for (uint32 groupIdx = 0; groupIdx < _featureTimingGroups.size(); groupIdx++) {
		FeatureTimingGroup &group = _featureTimingGroups[groupIdx];
		for (uint32 memberIdx = 0; memberIdx < group._members.size(); memberIdx++) {
			if (group._members[memberIdx] != groupMember)
				continue;

			for (uint32 policyMemberIdx = 0;
				 policyMemberIdx < group._members.size(); policyMemberIdx++) {
				group._members[policyMemberIdx]->setScriptSoundPolicy(policy);
			}
			return;
		}
	}

	groupMember->setScriptSoundPolicy(policy);
}

void ZoombiniPage::prepareFeatureTimingGroups() {
	for (ZmbFeature *feature : _scrbFeatures)
		feature->clearSharedFrameTimingResult();
	for (ZmbFeature *feature : _subFeatures)
		feature->clearSharedFrameTimingResult();
	for (ZmbSnoid *snoid : _snoidMap)
		snoid->clearSharedFrameTimingResult();

	for (uint32 groupIdx = 0; groupIdx < _featureTimingGroups.size(); groupIdx++) {
		FeatureTimingGroup &group = _featureTimingGroups[groupIdx];
		if (group._runtimeOwner && !group._runtimeOwner->isRenderActivated()) {
			group._runtimeOwner = nullptr;
			group._cachedTimingState = 0;
		}
	}
}

void ZoombiniPage::prepareFeatureTimingResult(ZmbFeature *feature) {
	if (!feature || !feature->isRenderActivated())
		return;

	for (uint32 groupIdx = 0; groupIdx < _featureTimingGroups.size(); groupIdx++) {
		FeatureTimingGroup &group = _featureTimingGroups[groupIdx];
		if (Common::find(group._members.begin(), group._members.end(), feature) == group._members.end())
			continue;

		if (!group._runtimeOwner) {
			group._runtimeOwner = feature;
			group._cachedTimingState = 0;
		}

		bool timingReady = false;
		if (!group._alternatePhases) {
			if (group._runtimeOwner == feature)
				group._cachedTimingState = feature->isAnimationTimerDue(_currentFrameCounter) ? 1 : 0;
			timingReady = group._cachedTimingState != 0;
		} else if (group._runtimeOwner == feature) {
			if (group._cachedTimingState == 0) {
				timingReady = feature->isAnimationTimerDue(_currentFrameCounter);
				group._cachedTimingState = timingReady ? 1 : 0;
			}
		} else if (group._cachedTimingState == 1) {
			group._cachedTimingState = 2;
		} else if (group._cachedTimingState == 2) {
			group._cachedTimingState = 0;
			timingReady = true;
		}

		feature->setSharedFrameTimingResult(timingReady);
		return;
	}
}

void ZoombiniPage::addExternalDirtyRect(const Common::Rect &rect) {
	if (rect.isEmpty())
		return;

	Common::Rect clipped = rect;
	clipped.clip(Common::Rect(0, 0, 640, 480));
	if (clipped.isEmpty())
		return;

	// Preserve the exact union represented by the source dirty region.
	// Replacing intersecting rectangles with a bounding box would restore unchanged corner pixels on the next frame.
	ZoombiniGraphics::mergeRectIntoRegion(_externalDirtyRects, clipped);
}

void ZoombiniPage::invalidateBackgroundRect(const Common::Rect &rect) {
	// Merge external background damage before restoring the background into the persistent shape screen.
	// Keep it separate from graphics output damage.
	addExternalDirtyRect(rect);
}

bool ZoombiniPage::transformSnoidHotspotForRender(const ZmbSnoid *snoid, ZmbHotspot &hs, uint8 snoidLayerShift, ZmbResource &snoidShapeRes) const {
	if (hs._shapeIdx == ZmbHotspot::kShapeNone)
		return false;

	adjustSnoidScriptHotspotForRender(snoid, hs);

	if (snoid->hasFlag(ZmbFeature::FLAG_00800000_POS_DELTA)) {
		const Common::Point &posDelta = snoid->getPosDelta();
		hs._x += posDelta.x;
		hs._y += posDelta.y;
	}

	snoidShapeRes = snoid->getResource();
	hs._x += snoid->getScrsRenderOffset().x;
	hs._y += snoid->getScrsRenderOffset().y;

	if (0 < hs._shapeIdx) {
		if (!snoid->hasCombinedShapeIndices())
			hs._shapeIdx += snoid->getBodyLayerBaseOffset(hs._hsId, snoidLayerShift);

		if (0 < hs._shapeIdx) {
			if (snoid->isFacingLeft())
				hs._shapeIdx = static_cast<int16>(2 * hs._shapeIdx);
			else
				hs._shapeIdx = static_cast<int16>(2 * hs._shapeIdx - 1);

			ZmbShapeOffsetRegs *activeRegs = nullptr;
			if (snoid->getAnimState() == kSnoidAnimState009_ScriptNormal &&
				!snoid->_useSmallShapeRegs) {
				// State 9 NORMAL: pair with tBMP 3100 + REGS 102/103.
				activeRegs = _vm->_snoidScriptShapeRegs;
				snoidShapeRes = ZmbResource(ZmbResource::kSystem, 3100);
			} else if (snoid->_useSmallShapeRegs) {
				activeRegs = _vm->_smallSnoidShapeOffsetRegs;
			} else {
				activeRegs = _vm->_snoidShapeOffsetRegs;
			}
			if (activeRegs) {
				const Common::Point delta = activeRegs->getShapeDelta(hs._shapeIdx);
				hs._x -= delta.x;
				hs._y -= delta.y;
			}
		}
	}

	if (hs._shapeIdx < 1)
		return false;

	// A zero or out-of-range shape terminates both coverage accumulation and the Snoid layer-blit walk.
	if (_vm->_gfx->getShapeCount(snoidShapeRes) < static_cast<uint32>(hs._shapeIdx))
		return false;

	return true;
}

void ZoombiniPage::prepareSnoidVisualCoverage(ZmbSnoid *snoid, bool cacheFrame) {
	if (!snoid)
		return;
	snoid->clearPreparedVisualRects();
	if (!snoid->isRenderActivated())
		return;

	// Hotel event 15 activates the post-render clip after the current frame is materialized.
	// Apply its clickRect intersection on the next Snoid frame.
	snoid->applyPendingVisualRectConstraint();

	ZmbHotspotGroup *hsGroup = snoid->getCurrentScriptVisualFrame();
	if (!hsGroup) {
		snoid->setPreparedVisualRects(Common::Array<Common::Rect>());
		return;
	}

	Common::Array<ZmbHotspot> hotspots = hsGroup->copyHotspots();

	uint8 snoidLayerShift = 0;
	if (!snoid->hasCombinedShapeIndices() &&
		snoid->getAnimState() == kSnoidAnimState009_ScriptNormal) {
		if (!hotspots.empty() && 18 < hotspots[0]._shapeIdx)
			snoidLayerShift = 1;
	}
	Common::Rect sortRect;
	Common::Array<ZmbPreparedRenderHotspot> preparedHotspots;
	Common::Array<Common::Rect> preparedVisualRects;
	bool hasSortRect = false;

	for (uint32 hotspotIdx = 0; hotspotIdx < hotspots.size(); hotspotIdx++) {
		ZmbHotspot hs = hotspots[hotspotIdx];
		ZmbResource snoidShapeRes;
		// Coverage accumulation and drawing stop at the first zero or invalid layer.
		// The fixed draw-record array is sentinel-terminated, not sparse.
		if (!transformSnoidHotspotForRender(snoid, hs, snoidLayerShift, snoidShapeRes))
			break;

		if (cacheFrame) {
			ZmbPreparedRenderHotspot prepared;
			prepared._hotspot = hs;
			prepared._resource = snoidShapeRes;
			prepared._hsGroup = hsGroup;
			preparedHotspots.push_back(prepared);
		}

		Common::Rect shapeSize = _vm->_gfx->getShapeSize(snoidShapeRes, static_cast<uint16>(hs._shapeIdx));
		Common::Rect drawnRect(hs._x, hs._y, hs._x + shapeSize.width(), hs._y + shapeSize.height());
		if (hasSortRect)
			sortRect.extend(drawnRect);
		else {
			sortRect = drawnRect;
			hasSortRect = true;
		}

		drawnRect.clip(Common::Rect(0, 0, ZoombiniGraphics::kScreenWidth, ZoombiniGraphics::kScreenHeight));
		drawnRect = snoid->constrainVisualRect(drawnRect);
		if (drawnRect.isEmpty())
			continue;
		preparedVisualRects.push_back(drawnRect);
	}

	if (!hasSortRect) {
		snoid->setSortRect(Common::Rect());
		snoid->setRenderedClickRect(Common::Rect());
		snoid->setPreparedVisualRects(preparedVisualRects);
		if (cacheFrame)
			snoid->setPreparedRenderHotspots(preparedHotspots);
		return;
	}

	sortRect = snoid->constrainVisualRect(sortRect);
	snoid->setSortRect(sortRect);
	snoid->setRenderedClickRect(sortRect);
	snoid->setPreparedVisualRects(preparedVisualRects);
	if (cacheFrame)
		snoid->setPreparedRenderHotspots(preparedHotspots);
}

void ZoombiniPage::prepareFeatureVisualCoverage(ZmbFeature *feature, const Common::Point *materializedPosDelta,
												bool preservePreparedTerminatorFrame) {
	// Snoids and custom actor owners prepare their own transformed coverage.
	if (!feature)
		return;
	if (feature->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID) || feature->managesOwnVisualCoverage())
		return;
	if (!feature->isRenderActivated() && feature->hasFlag(ZmbFeature::FLAG_01000000_DEFER_RENDER))
		return;

	// A terminator-only group dispatches its event without replacing the
	// original runner's persistent materialized hotspot array. Keep the exact
	// prepared payload: rebuilding the preceding raw group would apply state
	// changes made by the event to a frame that was already transformed.
	ZmbHotspotGroup *exactGroup = feature->getHotspotGroupExact(feature->getLastFrameIdx());
	if (preservePreparedTerminatorFrame && exactGroup && exactGroup->getHotspotCount() == 0 &&
		feature->hasPreparedRenderHotspots())
		return;

	feature->clearPreparedVisualRects();

	feature->clearPreparedRenderHotspots();

	// Features with FLAG_01000000_DEFER_RENDER and rendering disabled do not draw, hence have no clickRect.
	// Inactive runners without DEFER_RENDER keep drawing their frozen materialized frame.
	// They therefore still need visual coverage.
	ZmbHotspotGroup *hsGroup = feature->getCurrentScriptVisualFrame();
	if (!hsGroup) {
		feature->setPreparedVisualRects(Common::Array<Common::Rect>());
		feature->setPreparedRenderHotspots(Common::Array<ZmbPreparedRenderHotspot>());
		return;
	}

	// Materialize POS_DELTA before the shape callback, then apply REGS offsets.
	// Post-render consumes this frozen payload without re-running page callbacks.
	// Then union rectangles from @ref ZoombiniGraphics::getShapeSize().
	// Use @ref ZoombiniGraphics::getShapeSize() instead of @ref ZoombiniGraphics::drawShape().
	// This keeps pixels untouched before Z-sort.
	Common::Array<ZmbHotspot> hotspots = hsGroup->copyHotspots();
	if (feature->hasFlag(ZmbFeature::FLAG_00800000_POS_DELTA)) {
		const Common::Point posDelta = materializedPosDelta ? *materializedPosDelta : feature->getPosDelta();
		for (uint32 i = 0; i < hotspots.size(); i++) {
			hotspots[i]._x += posDelta.x;
			hotspots[i]._y += posDelta.y;
		}
	}
	feature->onPreRenderShape(this, hsGroup, hotspots);

	ZmbShapeOffsetRegs *shapeRegs = feature->getShapeOffsetRegs();
	if (shapeRegs) {
		for (uint32 hotspotIdx = 0; hotspotIdx < hotspots.size(); hotspotIdx++) {
			ZmbHotspot &hs = hotspots[hotspotIdx];
			if (hs._shapeIdx != ZmbHotspot::kShapeNone) {
				const Common::Point delta = shapeRegs->getShapeDelta(hs._shapeIdx);
				hs._x -= delta.x;
				hs._y -= delta.y;
			}
		}
	}

	Common::Rect sortRect;
	Common::Array<ZmbPreparedRenderHotspot> preparedHotspots;
	Common::Array<Common::Rect> preparedVisualRects;
	bool hasSortRect = false;
	for (uint32 hotspotIdx = 0; hotspotIdx < hotspots.size(); hotspotIdx++) {
		ZmbHotspot hs = hotspots[hotspotIdx];
		if (hs._shapeIdx == ZmbHotspot::kShapeNone)
			continue;

		ZmbPreparedRenderHotspot prepared;
		prepared._hotspot = hs;
		prepared._resource = feature->getResource();
		prepared._hsGroup = hsGroup;
		preparedHotspots.push_back(prepared);

		Common::Rect shapeSize = _vm->_gfx->getShapeSize(feature->getResource(), static_cast<uint16>(hs._shapeIdx));
		Common::Rect drawnRect(hs._x, hs._y, hs._x + shapeSize.width(), hs._y + shapeSize.height());
		drawnRect.clip(Common::Rect(0, 0, ZoombiniGraphics::kScreenWidth, ZoombiniGraphics::kScreenHeight));
		drawnRect = feature->constrainVisualRect(drawnRect);
		if (drawnRect.isEmpty())
			continue;
		preparedVisualRects.push_back(drawnRect);

		if (hasSortRect) {
			sortRect.extend(drawnRect);
		} else {
			sortRect = drawnRect;
			hasSortRect = true;
		}
	}

	feature->setPreparedVisualRects(preparedVisualRects);
	feature->setPreparedRenderHotspots(preparedHotspots);
	if (hasSortRect)
		feature->setSortRect(sortRect);
	else
		feature->setSortRect(Common::Rect());
}

void ZoombiniPage::renderFeatures() {
	// Dirty-rect rendering architecture:
	//
	// The renderer maintains a persistent shape screen as its composite buffer.
	// It does not clear that buffer each frame.
	// Only dirty regions where features changed receive background restoration and redraw.
	// Non-dirty pixels persist from the previous frame's composite.
	//
	// Pipeline:
	//   1. Merge external dirty accumulator into main dirty region
	//   2. PreRender: animation logic + merge OLD visual coverage into dirty
	//   3. Z-sort features
	//   4. Restore background ONLY in dirty region
	//   5. Set render clip to dirty region rects
	//   6. For each Z-sorted feature: merge NEW visual coverage if dirty,
	//      draw
	//   7. Release render clip region
	//
	// The render clip is a union of dirty rectangles, not their bounding box.
	// Each draw operation is clipped against each rectangle's intersection.

	ZmbBuiltinDebugRuntimeState &builtinDebug = _vm->_builtinDebug;
	if (builtinDebug._stepMode) {
		if (!builtinDebug._stepAdvanceRequested)
			return;
		builtinDebug._stepAdvanceRequested = false;
	}

	// Page compatibility mode, feature-level opt-ins, and ownerless explicit
	// candidates can each activate render-pass arbitration.
	const bool scriptSoundQueueActive = shouldProcessScriptSoundQueue();
	if (scriptSoundQueueActive)
		_vm->_sound->beginScriptSoundFrame();

	// Step 1: The dirty region is reset at the END of the previous pass, not here.
	// Damage reported after a pass has finished - an arrival handler, a queue drain, a
	// mouse-up - therefore stays in the region and is honoured by this pass, instead of
	// being discarded before anything restores the background underneath it.

	// Step 2: Merge the external dirty accumulator while preserving disjoint rectangles.
	for (const Common::Rect &rect : _externalDirtyRects)
		addDirtyRect(rect);
	_externalDirtyRects.clear();

	// Step 3: Force redraw: entire screen is dirty (initial frame, page change, etc.)
	if (_forceRedrawPending) {
		addDirtyRect(Common::Rect(0, 0, 640, 480));
		_forceRedrawPending = false;
	}

	// Shared slots retain their runtime owner and cached timing state across passes.
	prepareFeatureTimingGroups();

	// Pass 1 follows the current global runner order, matching the post-render list owner.
	// A timing slot is evaluated immediately before each member reaches its pre-render callback.
	// Set @ref ZmbFeature::_needsRedraw on animating features.
	// Merge their old visual coverage into the dirty region.
	syncRunnerOrder();
	ZmbFeature *preRenderFeature = _runnerOrder.empty() ? nullptr : _runnerOrder.front();
	while (preRenderFeature) {
		const bool rematerializationRequested = preRenderFeature->consumeVisualRematerializationRequest();
		const uint32 preparedRenderGeneration = preRenderFeature->getPreparedRenderGeneration();
		if (rematerializationRequested)
			markFeatureVisualCoverageDirty(preRenderFeature, false);

		prepareFeatureTimingResult(preRenderFeature);
		preRenderFeature->onPreRender(this);

		// Static UI runners retain their last transformed frame after their SCRB
		// render timer is deactivated. Refresh only an explicitly requested runner.
		// A normally due pre-render may already have materialized this frame.
		if (rematerializationRequested &&
			preparedRenderGeneration == preRenderFeature->getPreparedRenderGeneration())
			// A semantic owner explicitly requested this refresh after changing position
			// or UI state. Rebuild the preceding shape-bearing group even when the script
			// is parked on a terminator-only frame.
			prepareFeatureVisualCoverage(preRenderFeature, nullptr, false);

		if (rematerializationRequested)
			markPreparedFeatureVisualCoverageDirty(preRenderFeature, false);

		// A pre-render callback may re-link runners. Follow the current runner's
		// updated successor, as the original linked-list traversal does.
		// This also keeps array insertions and erasures from invalidating an iterator.
		uint32 runnerIdx = 0;
		while (runnerIdx < _runnerOrder.size() && _runnerOrder[runnerIdx] != preRenderFeature)
			runnerIdx += 1;
		preRenderFeature =
			runnerIdx + 1 < _runnerOrder.size() ? _runnerOrder[runnerIdx + 1] : nullptr;
	}

	// Rebuild Snoid click rectangles and draw records before Z-sort and background restoration.
	// Compute the current visual bounds with the same hotspot, body-layer, and REGS transform.
	// @ref ZoombiniPage::blitShapes() uses that transform when drawing.
	// Every Snoid frame remains frozen between dedicated timer fires.
	// Live position, facing, or callback state can already describe the next frame.
	for (ZmbSnoid *s : _snoidMap) {
		if (!s->hasPreparedRenderHotspots())
			prepareSnoidVisualCoverage(s, true);
	}

	// Z-sort: partition and sort feature runners
	Common::Array<ZmbFeature *> renderList;
	buildSortedRenderList(renderList);

	Common::Array<Common::Rect> initialDirtyRects = _dirtyRects;

	// The original Ctrl+Z mode clears the persistent composite to palette 14
	// before restoring only the current dirty region from the background.
	if (builtinDebug._runnerBackdropMode)
		_vm->_gfx->fillArea(ZoombiniGraphics::kShapeScreen, ZoombiniGraphics::kColor0E_VeryLightGray);

	// Step 4: restore the background in each dirty rectangle.
	for (const Common::Rect &dirtyRect : initialDirtyRects) {
		if (!dirtyRect.isEmpty())
			_vm->_gfx->copyBackToShapeScreen(dirtyRect);
	}

	// Set render clip to the list of dirty rects.  All drawing is confined
	// to the precise union of these rects - non-dirty features' previous-frame
	// pixels persist on the shape-screen.
	// An empty dirty region suppresses every post-render write.
	// It is distinct from an unrestricted, cleared render clip.
	_vm->_gfx->setRenderClipRects(initialDirtyRects);

	// Pass 2: Post-render - draw shapes in Z-sorted order
	//
	// Pre-render computes the new visual coverage from hotspot metadata and REGS shape sizes.
	// Post-render merges that coverage into dirty state before drawing,
	// so the clip always covers the feature's new area.
	//
	// Standard SCRB/Snoid runners have their current coverage prepared before this
	// loop, separately from the previous DrawRecords. REGION_TRACK runners merge
	// per-shape rectangles; ordinary runners merge their aggregate visual rectangle.
	// That materialized region is authoritative; individual draw-call bounds must
	// not widen it.
	// Custom callbacks still need draw-call tracking because they have no predictable current RMap.
	//
	// CRITICAL: features MUST draw through the clip.
	// Clearing the clip would let dirty features paint outside the dirty region onto the persistent shapeScreen,
	// causing dialog remnants and Z-ordering corruption.
	for (ZmbFeature *feature : renderList) {
		const bool featureNeedsRedraw = feature->needsRedraw();
		const bool hasPreparedVisualCoverage =
			feature->usesDefaultRenderFunc() && feature->hasPreparedVisualRects();
		if (featureNeedsRedraw) {
			if (hasPreparedVisualCoverage) {
				// Merge newly materialized coverage using the runner's tracking mode.
				markPreparedFeatureVisualCoverageDirty(feature, true);
			} else {
				// Custom callbacks have no predictable new coverage. Keep their old
				// visual region active, then let their draw calls expand it below.
				markFeatureVisualCoverageDirty(feature, true);
			}
		}

		// Consume this frame's request before rendering. A render callback may
		// request another redraw, which must survive for the next frame.
		feature->setNeedsRedraw(false);

		Common::Array<Common::Rect> savedRenderClipRects;
		if (feature->hasVisualRectConstraint()) {
			// Constrain the active render clip to this feature's visual region.
			savedRenderClipRects = _vm->_gfx->getRenderClipRects();
			Common::Array<Common::Rect> constrainedRenderClipRects;
			for (const Common::Rect &renderClipRect : savedRenderClipRects) {
				Common::Rect constrained = renderClipRect;
				constrained.clip(feature->getVisualRectConstraint());
				if (!constrained.isEmpty())
					constrainedRenderClipRects.push_back(constrained);
			}
			_vm->_gfx->setRenderClipRects(constrainedRenderClipRects);
		}

		const bool expandTrackedDirtyClip = featureNeedsRedraw &&
											!feature->usesDefaultRenderFunc() && !feature->hasVisualRectConstraint();
		_vm->_gfx->beginDirtyRectTracking(expandTrackedDirtyClip);
		ZmbRenderResult renderResult = feature->onPostRender(this);
		Common::Rect drawnRect = _vm->_gfx->endDirtyRectTracking();
		if (feature->hasVisualRectConstraint())
			_vm->_gfx->setRenderClipRects(savedRenderClipRects);

		if (!hasPreparedVisualCoverage && featureNeedsRedraw &&
			renderResult == ZmbRenderResult::kRendered && !drawnRect.isEmpty()) {
			feature->setSortRect(drawnRect);
			if (!feature->hasExplicitClickRect() || feature->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID))
				feature->setRenderedClickRect(drawnRect);
		}

		if (featureNeedsRedraw && !hasPreparedVisualCoverage) {
			// A custom renderer's new region only becomes known after its calls.
			// Keep that region active for subsequent higher-Z features.
			markFeatureVisualCoverageDirty(feature, true);
		}
	}

	// Release clip region.
	_vm->_gfx->clearRenderClipRect();

	const bool scriptSoundQueueActiveAtEnd = shouldProcessScriptSoundQueue();
	if (scriptSoundQueueActive || scriptSoundQueueActiveAtEnd) {
		// A callback may opt a feature in and enqueue its first candidate during
		// pre-render. Prune a finished retained winner before selecting it.
		if (!scriptSoundQueueActive)
			_vm->_sound->beginScriptSoundFrame();
		_vm->_sound->flushScriptSoundFrame(*this);
	}

	// Reset the dirty region only once everything this pass drew has been presented.
	// Anything reported after this point belongs to the next pass, which is what lets
	// out-of-pass damage survive long enough to be restored. See Step 1 above.
	_dirtyRects.clear();
}

static void drawBuiltinHollowRect(ZoombiniGraphics *gfx, const Common::Rect &rect, uint32 color) {
	if (rect.isEmpty())
		return;
	const ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;
	gfx->drawLine(screenKind, Common::Point(rect.left, rect.top), Common::Point(rect.right - 1, rect.top), color);
	gfx->drawLine(screenKind, Common::Point(rect.right - 1, rect.top), Common::Point(rect.right - 1, rect.bottom - 1), color);
	gfx->drawLine(screenKind, Common::Point(rect.right - 1, rect.bottom - 1), Common::Point(rect.left, rect.bottom - 1), color);
	gfx->drawLine(screenKind, Common::Point(rect.left, rect.bottom - 1), Common::Point(rect.left, rect.top), color);
}

static ZoombiniGraphics::TextConf getBuiltinDebugTextConf() {
	ZoombiniGraphics::TextConf textConf;
	textConf._fontUsage = ZoombiniFontUsage::kFontText;
	textConf._textPalette = ZoombiniGraphics::kColor2D_Black;
	textConf._hAlign = Graphics::kTextAlignCenter;
	textConf._vAlign = Graphics::kTextAlignCenter;
	textConf._wordWrap = false;
	return textConf;
}

void ZoombiniPage::drawBuiltinDebugText(const Common::String &text, bool restoreOnNextFrame) {
	const Common::Rect rect(0, 0, 310, 16);
	_vm->_gfx->fillArea(ZoombiniGraphics::kShapeScreen, rect, ZoombiniGraphics::kColor0A_White);
	_vm->_gfx->drawText(ZoombiniGraphics::kShapeScreen, Common::U32String(text.c_str()), rect, getBuiltinDebugTextConf());
	_vm->_gfx->flushScreens();
	_vm->_system->updateScreen();

	// Non-blocking direct draws remain until normal page damage replaces them.
	// A blocking draw is restored immediately before the first render pass after input resumes.
	// A new direct draw during that release event replaces the pending message instead.
	_builtinDebugTextRestorePending = restoreOnNextFrame;
}

void ZoombiniPage::drawBuiltinDebugPanel(const Common::Rect &rect) {
	_vm->_gfx->fillArea(ZoombiniGraphics::kShapeScreen, rect, ZoombiniGraphics::kColor0E_VeryLightGray);
	drawBuiltinHollowRect(_vm->_gfx, rect, ZoombiniGraphics::kColor0B_VeryDarkGray);
}

void ZoombiniPage::drawBuiltinDebugPanelText(const Common::String &text, const Common::Rect &rect) {
	ZoombiniGraphics::TextConf textConf = getBuiltinDebugTextConf();
	textConf._textPalette = ZoombiniGraphics::kColor0B_VeryDarkGray;
	_vm->_gfx->drawText(ZoombiniGraphics::kShapeScreen, Common::U32String(text.c_str()), rect, textConf);
}

void ZoombiniPage::drawBuiltinDebugTraitSprite(const Common::Point &anchor, byte traitKindCode, byte traitValue) {
	static constexpr uint16 kFeetTable[6] = {0, 191, 246, 335, 360, 411};
	static constexpr uint16 kNoseTable[6] = {0, 171, 175, 179, 183, 187};
	static constexpr uint16 kEyeTable[6] = {0, 91, 107, 123, 139, 155};
	static constexpr uint16 kHairTable[6] = {0, 11, 27, 43, 59, 75};

	if (traitKindCode < 1 || 4 < traitKindCode || traitValue < 1 || 5 < traitValue)
		return;

	uint16 shapeBase = 0;
	switch (traitKindCode) {
	case 1:
		shapeBase = kHairTable[traitValue];
		break;
	case 2:
		shapeBase = kEyeTable[traitValue];
		break;
	case 3:
		shapeBase = kNoseTable[traitValue];
		break;
	case 4:
		shapeBase = kFeetTable[traitValue];
		break;
	default:
		return;
	}

	const uint16 shapeId = static_cast<uint16>(2 * shapeBase + 1);
	Common::Point drawPos = anchor;
	if (_vm->_snoidShapeOffsetRegs) {
		const Common::Point delta = _vm->_snoidShapeOffsetRegs->getShapeDelta(shapeId);
		drawPos.x -= delta.x;
		drawPos.y -= delta.y;
	}
	_vm->_gfx->drawShape(ZoombiniGraphics::kShapeScreen,
						 ZmbResource(ZmbResource::kSystem, kSysResShapeBitmap3000_SnoidNormal), shapeId, drawPos);
}

void ZoombiniPage::drawBuiltinNodePaths() {
	if (_nodeMap.empty())
		return;

	ZmbBuiltinDebugRuntimeState &debugState = _vm->_builtinDebug;
	const ZmbNode *node = _nodeMap.begin()->_value;
	const ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;
	ZoombiniGraphics::TextConf textConf = getBuiltinDebugTextConf();
	textConf._textPalette = ZoombiniGraphics::kColor0B_VeryDarkGray;

	for (uint32 waypointIdx = 0; waypointIdx < node->_waypoints.size(); waypointIdx++) {
		const Common::Point &point = node->_waypoints[waypointIdx];
		const Common::Rect rect(point.x - 10, point.y - 10, point.x + 10, point.y + 10);
		_vm->_gfx->fillArea(screenKind, rect, ZoombiniGraphics::kColor0E_VeryLightGray);
		drawBuiltinHollowRect(_vm->_gfx, rect, ZoombiniGraphics::kColor0B_VeryDarkGray);
		_vm->_gfx->drawText(screenKind, Common::U32String::format("%u", waypointIdx + 1), rect, textConf);
	}

	if (!node->_paths.empty()) {
		const uint16 pathIdx = debugState._pathCycleIndex % node->_paths.size();
		const uint32 pathColor = pathIdx + 33;
		const Common::Array<uint8> &path = node->_paths[pathIdx];
		Common::Point previous;
		bool hasPrevious = false;
		for (uint8 waypointId : path) {
			if (waypointId == 0)
				break;
			if (node->_waypoints.size() < waypointId)
				continue;
			const Common::Point &point = node->_waypoints[waypointId - 1];
			if (hasPrevious)
				_vm->_gfx->drawLine(screenKind, previous, point, pathColor);
			const Common::Rect rect(point.x - 10, point.y - 10, point.x + 10, point.y + 10);
			_vm->_gfx->fillArea(screenKind, rect, pathColor);
			drawBuiltinHollowRect(_vm->_gfx, rect, ZoombiniGraphics::kColor0B_VeryDarkGray);
			_vm->_gfx->drawText(screenKind, Common::U32String::format("%u", waypointId), rect, textConf);
			previous = point;
			hasPrevious = true;
		}
		debugState._pathCycleIndex += 1;
	}
}

void ZoombiniPage::drawBuiltinPaletteGrid() {
	const ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;
	for (uint16 paletteIdx = 0; paletteIdx < 256; paletteIdx++) {
		const int16 left = static_cast<int16>(paletteIdx % 32) * 8;
		const int16 top = static_cast<int16>(paletteIdx / 32) * 8;
		_vm->_gfx->fillArea(screenKind, Common::Rect(left, top, left + 8, top + 8), paletteIdx);
	}
}

uint32 ZoombiniPage::getBuiltinDebugRunnerCount() {
	syncRunnerOrder();
	return _runnerOrder.size() + kBuiltinDebugOriginalRootRunnerCount;
}

void ZoombiniPage::drawBuiltinRunnerClickRects(bool filterSnoids, bool showIds, int16 targetPosition) {
	syncRunnerOrder();
	ZoombiniGraphics::TextConf textConf = getBuiltinDebugTextConf();
	textConf._textPalette = ZoombiniGraphics::kColor0B_VeryDarkGray;

	for (uint32 runnerIdx = 0; runnerIdx < _runnerOrder.size(); runnerIdx++) {
		const int16 position = static_cast<int16>(runnerIdx + kBuiltinDebugOriginalRootRunnerCount + 1);
		if (targetPosition != 0 && position != targetPosition)
			continue;

		ZmbFeature *feature = _runnerOrder[runnerIdx];
		if (filterSnoids && (feature->getFlags() & 0x0F) != ZmbFeature::FLAG_00000001_TYPE_SNOID)
			continue;
		const Common::Rect rect = feature->getClickRect();
		if (rect.isEmpty())
			continue;

		_vm->_gfx->fillArea(ZoombiniGraphics::kShapeScreen, rect, ZoombiniGraphics::kColor0E_VeryLightGray);
		drawBuiltinHollowRect(_vm->_gfx, rect, ZoombiniGraphics::kColor0B_VeryDarkGray);

		Common::String label;
		if (filterSnoids) {
			const ZmbSnoid *snoid = dynamic_cast<const ZmbSnoid *>(feature);
			label = snoid && snoid->_packIsOccupied ? "+" : "-";
			if (showIds)
				label += Common::String::format("%u", feature->getId());
		} else if (showIds) {
			label = Common::String::format("%u", feature->getId());
		} else {
			label = Common::String::format("%c%u", feature->isRenderActivated() ? '+' : '-', position);
			if (feature->getId() == UINT16_MAX)
				label += Common::String::format(":%u", _runnerOrder.size() + kBuiltinDebugOriginalRootRunnerCount);
		}
		_vm->_gfx->drawText(ZoombiniGraphics::kShapeScreen, Common::U32String(label.c_str()), rect, textConf);
	}
}

void ZoombiniPage::checkCloseFeatures() {
	ZmbFeatureList<ZmbFeature> *deleteLists[1] = {
		&_scrbFeatures,
	};

	for (uint32 i = 0; i < ARRAYSIZE(deleteLists); i++) {
		ZmbFeatureList<ZmbFeature> *listPtr = deleteLists[i];

		Common::Array<ZmbFeature *> deletePtrs;
		for (ZmbFeature *f : *listPtr) {
			if (f->isCloseScheduled())
				deletePtrs.push_back(f);
		}
		for (ZmbFeature *f : deletePtrs) {
			const Common::Rect &oldRect = f->getZSortRect();
			if (!oldRect.isEmpty())
				addExternalDirtyRect(oldRect);
			unregisterFeatureTimingGroup(f);
			runnerOrderErase(f);
			deregisterFeature(*listPtr, f);
		}
	}

	// Detach sub-features from @ref ZoombiniPage::_subFeatures without deleting them.
	// The parent feature owns each pointer.
	Common::Array<ZmbFeature *> detachPtrs;
	for (ZmbFeature *f : _subFeatures) {
		if (f->isDetachScheduled())
			detachPtrs.push_back(f);
	}
	for (ZmbFeature *subFeature : detachPtrs) {
		_subFeatures.eraseByPtr(subFeature, subFeature->getId());
		subFeature->clearDetach();
		subFeature->setSubFeatureRunning(false);
	}
}

bool ZoombiniPage::shouldQueueFeatureSound(const ZmbFeature *feature) const {
	if (feature) {
		switch (feature->getScriptSoundPolicy()) {
		case ZmbFeature::ScriptSoundPolicy::kImmediate:
			return false;
		case ZmbFeature::ScriptSoundPolicy::kPriorityQueue:
		case ZmbFeature::ScriptSoundPolicy::kForcedPriorityQueue:
			return true;
		case ZmbFeature::ScriptSoundPolicy::kInheritPage:
			break;
		}
	}
	return usesOriginalScriptSoundQueue();
}

bool ZoombiniPage::hasPriorityQueueFeature() const {
	for (const ZmbFeature *feature : _scrbFeatures) {
		if (feature->getScriptSoundPolicy() == ZmbFeature::ScriptSoundPolicy::kPriorityQueue ||
			feature->getScriptSoundPolicy() == ZmbFeature::ScriptSoundPolicy::kForcedPriorityQueue)
			return true;
	}
	for (const ZmbFeature *feature : _subFeatures) {
		if (feature->getScriptSoundPolicy() == ZmbFeature::ScriptSoundPolicy::kPriorityQueue ||
			feature->getScriptSoundPolicy() == ZmbFeature::ScriptSoundPolicy::kForcedPriorityQueue)
			return true;
	}
	for (const ZmbSnoid *snoid : _snoidMap) {
		if (snoid->getScriptSoundPolicy() == ZmbFeature::ScriptSoundPolicy::kPriorityQueue ||
			snoid->getScriptSoundPolicy() == ZmbFeature::ScriptSoundPolicy::kForcedPriorityQueue)
			return true;
	}
	return false;
}

bool ZoombiniPage::shouldProcessScriptSoundQueue() const {
	return usesOriginalScriptSoundQueue() ||
		   hasPriorityQueueFeature() ||
		   _vm->_sound->hasQueuedScriptSoundThisFrame();
}

void ZoombiniPage::dispatchFeatureFrameSound(ZmbFeature *feature, int32 frameIdx) {
	ZmbResource sndRes;
	if (feature->getFrameSoundResource(frameIdx, sndRes))
		dispatchFeatureSound(feature, sndRes);
}

void ZoombiniPage::dispatchFeatureFrameSoundAndEvent(ZmbFeature *feature, int32 frameIdx) {
	if (!feature->isAnimationCycleRunning() || frameIdx <= feature->getLastSoundedFrameIdx())
		return;

	feature->setLastSoundedFrameIdx(frameIdx);
	dispatchFeatureFrameSound(feature, frameIdx);

	// Process SCRS event codes (0xFFxx frame terminators where xx != 0).
	// Gates ordinary callbacks on onHotspotShapeOrFrameFunc, while voice events use a separate table.
	// @ref ZmbFeature::_animEndCallbackFired records whether the callback has been consumed.
	uint8 eventCode = 0;
	if (!feature->getFrameEventCode(frameIdx, eventCode) || feature->hasAnimEndCallbackFired())
		return;
	const uint8 adjustedCode = eventCode - 1;
	ZmbSnoid *snoid = dynamic_cast<ZmbSnoid *>(feature);
	if (kAnimEvent200_VoiceFirst <= adjustedCode && adjustedCode <= kAnimEvent239_VoiceLast && snoid) {
		if (!snoid->allowsScrsVoiceEvents())
			return;

		static constexpr int16 kVoiceGroupMap[18] = {
			8, 6, 7, 10, 2, 12, 1, 9,
			0, 4, 5, 3, 11, 13, 14, 15, 16, 17};

		const uint8 voiceIdx = adjustedCode - kAnimEvent200_VoiceFirst;
		const int16 voiceGroup = (voiceIdx < ARRAYSIZE(kVoiceGroupMap)) ? kVoiceGroupMap[voiceIdx] : 0;
		if (voiceGroup != 0) {
			const int16 sndResId = snoid->getVoiceResId(voiceGroup);
			if (0 < sndResId) {
				debug(5, "snoid: event code %u -> voice group %d -> SND %d", eventCode, voiceGroup, sndResId);
				const ZmbResource voiceResource = resolveSnoidVoiceResource(sndResId);
				dispatchFeatureSound(feature, voiceResource);
			}
		}
	} else {
		debug(5, "snoid: event code %u dispatched (adjusted %d)", eventCode, adjustedCode);
		onFeatureAnimEvent(feature, static_cast<int16>(adjustedCode));
	}
}

void ZoombiniPage::preRenderSnoid(ZmbSnoid *snoid) {
	// Keep the old-coverage snapshot, timer and state update, frame materialization,
	// and changed flag in one pre-render transaction.
	if (!snoid || !snoid->isRenderActivated())
		return;

	const bool hadVisualRectConstraint = snoid->hasVisualRectConstraint();
	Common::Rect oldVisualRectConstraint;
	Common::Array<Common::Rect> oldDrawRecordRects;
	Common::Rect oldZSortRect;
	// Snapshot the displayed frame before the tick can replace its script data
	// or enter another animation state. Apply the damage only after the timer
	// gate confirms that a visual transition actually occurred.
	if (hadVisualRectConstraint) {
		oldVisualRectConstraint = snoid->getVisualRectConstraint();
	} else {
		if (snoid->hasFlag(ZmbFeature::FLAG_08000000_REGION_TRACK) &&
			snoid->hasDrawRecords())
			snoid->collectDrawRecordRects(oldDrawRecordRects);
		oldZSortRect = snoid->getZSortRect();
	}

	if (!snoid->onSnoidAnimTick(this))
		return;

	// Merge the previous visual coverage captured before the tick replaced it.
	// Shape swaps can change opaque pixels while retaining the same registered bounds.
	if (hadVisualRectConstraint) {
		addDirtyRect(oldVisualRectConstraint);
	} else {
		bool addedOldCoverage = false;
		for (uint32 i = 0; i < oldDrawRecordRects.size(); i++) {
			if (addDirtyRect(oldDrawRecordRects[i]))
				addedOldCoverage = true;
		}
		if (!addedOldCoverage)
			addDirtyRect(oldZSortRect);
	}

	snoid->setNeedsRedraw(true);

	// Cache the rebuilt click rectangle before event callbacks and Z-sort;
	// @ref ZoombiniPage::blitShapes() consumes the prepared frame during post-render.
	prepareSnoidVisualCoverage(snoid, true);
	dispatchFeatureFrameSoundAndEvent(snoid, snoid->getLastFrameIdx());
}

/**
 * Pre-render pass for a single standard feature: animation logic.
 * Called for non-Snoid features before Z-sorting.
 *
 * Order of operations:
 * 1. Frame selection (advance animation via incremental ++)
 * 2. End-of-cycle handling (CHAIN_SCRIPT, PLAY_ONCE)
 * End-of-cycle handling and frame advancement are mutually exclusive.
 * @ref ZmbFeature::defaultSelectRenderFrame() advances past @ref ZmbFeature::_frameIdxMax to signal the cycle boundary.
 * The handler then resets @ref ZmbFeature::_lastFrameIdx to zero.
 * 3. Sound dispatch uses the final @ref ZmbFeature::_lastFrameIdx after any reset.
 * 4. Per-frame flag checks (SKIP_RENDER, SKIP_ONCE)
 */
void ZoombiniPage::preRenderFeature(ZmbFeature *feature) {
	// A new runner begins with rendering enabled and a click rectangle.
	// Its first standard pre-render loads SCRB frame zero, then applies deferred flags.
	// Post-render keeps drawing the frozen frame even when DEFER_ANIM disables animation.
	// DEFER_RENDER suppresses that drawing.
	if (feature->isInitialScrbLoadPending()) {
		feature->setInitialScrbLoadPending(false);
		feature->setLastFrameIdx(0);
		// Initial registration materializes frame zero synchronously and consumes
		// the loaded-frame hold. Runtime SCRB replacement retains its first hold.
		feature->clearFirstFrameAdvanceHold();
		prepareFeatureVisualCoverage(feature);
		feature->scheduleNextRenderFrame(_currentFrameCounter);

		if (feature->getMaxFrameIdx() < 1 ||
			feature->hasFlag(ZmbFeature::FLAG_00020000_SKIP_RENDER) || feature->hasFlag(ZmbFeature::FLAG_00080000_DEFER_ANIM))
			feature->deactivateRender();

		if (feature->hasFlag(ZmbFeature::FLAG_01000000_DEFER_RENDER)) {
			feature->deactivateRender();
		} else {
			feature->setNeedsRedraw(true);
		}
		// A running animation cycle begins its sound and event traversal at frame zero.
		// Dormant runners only materialize their initial visual state in this pass.
		if (!feature->hasFlag(ZmbFeature::FLAG_01000000_DEFER_RENDER))
			dispatchFeatureFrameSoundAndEvent(feature, feature->getLastFrameIdx());
		return;
	}

	// Early return when render-enabled state=0.
	// Hotspot positions are not updated; @ref ZmbFeature::_lastFrameIdx stays at its last processed value.
	// Post-render still draws non-DEFER_RENDER features through the @ref ZoombiniPage::blitShapes() gate.
	if (!feature->isRenderActivated())
		return;

	// 1. Frame selection - advance animation state
	// Advance the group frame index.
	// @ref ZmbFeature::defaultSelectRenderFrame() increments @ref ZmbFeature::_lastFrameIdx.
	// The cycle ends when @ref ZmbFeature::_lastFrameIdx passes @ref ZmbFeature::_frameIdxMax.
	// @ref ZmbFeature::isEndOfAnimationCycle() reports that result below.
	// It also sets @ref ZmbFeature::_frameTimingReady.
	int32 frameIdx = feature->onSelectRenderFrame(this);
	// Store the result because custom frame-selection hooks may not call @ref ZmbFeature::setLastFrameIdx() themselves.
	feature->setLastFrameIdx(frameIdx);

	// Timing gate.
	// The next-render deadline determines frame readiness.
	// The paired hotspot timing-group system may modulate that deadline.
	// Without paired slots, it reduces to next render deadline <= currentTime.
	// When timing is not ready, return here - no end-of-cycle,
	// no dirty rect merge, no sound dispatch, no flag checks.
	if (!feature->isFrameTimingReady())
		return;

	// Dirty rect merge (dirty-rect flag = 1).
	// Merge the feature's current, about-to-be-replaced visual coverage into the dirty region.
	// This repaints the area occupied by the previous visual during the current frame.
	// @ref ZmbFeature::FLAG_00004000_NO_DIRTY_MERGE skips the merge.
	// Use it when the old and new rectangles are identical, as in in-place animations.
	if (!feature->hasFlag(ZmbFeature::FLAG_00004000_NO_DIRTY_MERGE))
		markFeatureVisualCoverageDirty(feature, false);
	feature->setNeedsRedraw(true);
	Common::Point materializedPosDelta = feature->getPosDelta();

	ZmbSnoid *snoid = dynamic_cast<ZmbSnoid *>(feature);
	if (snoid) {
		const SnoidAnimState st = snoid->getAnimState();
		if (st == kSnoidAnimState008_ScriptReject || st == kSnoidAnimState009_ScriptNormal)
			prepareSnoidVisualCoverage(snoid, true);
	}

	if (feature->isAnimateActivated()) {
		// 2. End-of-cycle handling
		// End-of-cycle fires after the final declared script frame.
		// @ref ZmbFeature::defaultSelectRenderFrame() advances @ref ZmbFeature::_lastFrameIdx.
		// The index passes @ref ZmbFeature::_frameIdxMax after the previous tick displays the last valid frame.
		bool didChainScript = false;

		if (feature->isEndOfAnimationCycle()) {

			// Swap SCRB data on the same feature
			if (feature->hasFlag(ZmbFeature::FLAG_00040000_CHAIN_SCRIPT)) {
				int16 chainedId = feature->getChainedScrbId();
				if (chainedId != 0) {
					feature->setChainedScrbId(0);
					if (0 <= chainedId) {
						loadScrbOntoFeature(feature, static_cast<uint16>(chainedId));
					} else {
						// Negative = negate and set RANDOM_FRAME
						loadScrbOntoFeature(feature, static_cast<uint16>(-chainedId));
						feature->addFlag(ZmbFeature::FLAG_02000000_RANDOM_FRAME);
					}
					// The nested load above may synthesize a reverse target.
					// Discard it after consuming this chain.
					// This prevents the restored SCRB from ping-ponging back automatically.
					feature->setChainedScrbId(0);
					// Disable render after chain
					if (feature->hasFlag(ZmbFeature::FLAG_00002000_DRAW_ON_REG))
						feature->deactivateRender();
				}
				// Disable render
				if (feature->getMaxFrameIdx() < 1)
					feature->deactivateRender();
				// Restart frame and sound-event traversal at the first group.
				feature->setLastFrameIdx(0);
				feature->setLastSoundedFrameIdx(-1);
				didChainScript = true;
			}

			// Stop rendering at end of cycle
			if (feature->hasFlag(ZmbFeature::FLAG_00100000_PLAY_ONCE)) {
				feature->deactivateRender();
				// Post-render draws frozen records even while rendering is disabled,
				// unless @ref ZmbFeature::FLAG_00004000_DEFER_RENDER is set.
				// PLAY_ONCE reaches every declared group before PLAY_ONCE completes.
				// Empty terminator groups preserve the preceding draw records.
				// An explicit zero-shape group clears them.
				// @ref ZmbFeature::getHotspotGroup() models that fallback.
				// Retain the final declared frame instead of jumping back to the last positive shape.
				// Hotel 7501 relies on its late zero-shape group to remove Ulla after she exits the screen.
				feature->setLastFrameIdx(feature->getMaxFrameIdx());
				// Fire the callback and return early only if CHAIN_SCRIPT did not run (runner 1).
				// One-shot: onHotspotShapeOrFrameFunc cleared to 0 after firing.
				if (!didChainScript) {
					if (!feature->hasAnimEndCallbackFired()) {
						// Save the SCRB load generation before firing the callback.
						// If the callback loads a new SCRB, @ref ZmbFeature::loadScrbData() increments the generation.
						// It also resets @ref ZmbFeature::_animEndCallbackFired to false.
						// Do not mark the fresh SCRB's callback as fired.
						uint32 genBefore = feature->getScrbLoadGeneration();
						onFeatureAnimEvent(feature, kAnimEventM1_End);
						if (feature->getScrbLoadGeneration() == genBefore)
							feature->markAnimEndCallbackFired();
					}
					return;
				}
			} else if (!didChainScript) {
				// Loop from beginning.
				// SCRS playback states 8 and 9 use @ref ZmbSnoid::onSnoidAnimTick() for their frame lifecycle.
				// That method advances through the final frame.
				// @ref ZmbFeature::getFrameCount() supplies the limit.
				// The script end is dispatched afterward.
				// Resetting @ref ZmbFeature::_lastFrameIdx here would loop playback.
				// It would alternate between frame zero and the final shape-bearing frame.
				// The Snoid would never reach trailing terminator-only frames that carry the case-2 chain event.
				// SCRS 1900, 1904, and 1906 carry that event in frame 24's 0xFF03 terminator.
				if (snoid) {
					SnoidAnimState st = snoid->getAnimState();
					if (st == kSnoidAnimState008_ScriptReject || st == kSnoidAnimState009_ScriptNormal) {
						// Snoid SCRS state machine owns lifecycle: skip reset.
					} else {
						feature->setLastFrameIdx(0);
						feature->setLastSoundedFrameIdx(-1);
					}
				} else {
					feature->setLastFrameIdx(0);
					feature->setLastSoundedFrameIdx(-1);
				}
			}

			// Detach independently running sub-features.
			if (feature->isSubFeatureRunning())
				feature->scheduleDetach();
		}

		// 3. Sound dispatch - fire sounds/events for newly reached frames.
		// Re-read @ref ZmbFeature::_lastFrameIdx because end-of-cycle handling may have reset it to zero.
		frameIdx = feature->getLastFrameIdx();
		materializedPosDelta = feature->getPosDelta();
		dispatchFeatureFrameSoundAndEvent(feature, frameIdx);

		// 3b. Deferred -1 callback for CHAIN_SCRIPT
		// The deferred -1 callback fires at the end of the hotspot frame walk,
		// after processing frame 0 event codes of the chained SCRB.
		// One-shot: onHotspotShapeOrFrameFunc is cleared to 0 after firing.
		if (didChainScript && !feature->hasAnimEndCallbackFired()) {
			uint32 genBefore = feature->getScrbLoadGeneration();
			onFeatureAnimEvent(feature, kAnimEventM1_End);
			if (feature->getScrbLoadGeneration() == genBefore)
				feature->markAnimEndCallbackFired();
		}
	}

	// 4. Per-frame flag checks
	// These run AFTER end-of-cycle, BEFORE hotspot/shape processing.
	if (feature->hasFlag(ZmbFeature::FLAG_00020000_SKIP_RENDER))
		feature->deactivateRender();

	if (feature->hasFlag(ZmbFeature::FLAG_00010000_SKIP_ONCE)) {
		feature->removeFlag(ZmbFeature::FLAG_00010000_SKIP_ONCE);
		feature->setLastFrameIdx(0);
		feature->deactivateRender();
	}

	if (!snoid)
		prepareFeatureVisualCoverage(feature, &materializedPosDelta);
}

/**
 * Post-render pass for a single feature: shape blitting only.
 * Called in Z-sorted order after the pre-render pass.
 * Consumes the transformed frame prepared during pre-render, blits its shapes,
 * and updates draw records without invoking the shape hook again.
 */
ZmbRenderResult ZoombiniPage::blitShapes(ZmbFeature *feature) {
	return blitShapes(feature, ZoombiniGraphics::kPaletteRemapNone);
}

ZmbRenderResult ZoombiniPage::blitShapes(ZmbFeature *feature, ZoombiniGraphics::PaletteRemapMode remapColorAssistPalette) {
	ZmbSnoid *snoid = dynamic_cast<ZmbSnoid *>(feature);

	// Skip a normal feature only when rendering is inactive and DEFER_RENDER is set.
	// Features without DEFER_RENDER always draw from their last pre-render position.
	//
	// Snoids use a stricter callback contract that checks only their render-enabled state.
	// This contract does not include the DEFER_RENDER bitmask exception.
	// Town changes walker type flags after registration without replacing this callback.
	// Use the @ref ZmbSnoid type, not its current bitmask, to preserve the callback choice.
	if (snoid) {
		if (!feature->isRenderActivated())
			return ZmbRenderResult::kSkipped;
	} else {
		if (!feature->isRenderActivated() && feature->hasFlag(ZmbFeature::FLAG_01000000_DEFER_RENDER))
			return ZmbRenderResult::kSkipped;
	}

	// Use frame index computed during preRender pass
	int32 frameIdx = feature->getLastFrameIdx();

	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;

	if (feature->hasPreparedRenderHotspots()) {
		const Common::Array<ZmbPreparedRenderHotspot> &preparedHotspots = feature->getPreparedRenderHotspots();
		feature->clearDrawRecords();

		Common::Rect sortRect;
		bool hasSortRect = false;
		for (uint32 i = 0; i < preparedHotspots.size(); i++) {
			ZmbHotspot hs = preparedHotspots[i]._hotspot;
			bool clearBeforeRender = false;
			ZoombiniGraphics::PaletteRemapMode shapeRemap = remapColorAssistPalette;
			if (shapeRemap == ZoombiniGraphics::kPaletteRemapNone)
				shapeRemap = getColorAssistPaletteRemap(feature, hs, preparedHotspots[i]._resource);
			Common::Rect drawnRect = _vm->_gfx->drawShape(screenKind, preparedHotspots[i]._resource, &hs, clearBeforeRender, shapeRemap);
			drawnRect = feature->constrainVisualRect(drawnRect);
			if (drawnRect.isEmpty())
				continue;

			if (preparedHotspots[i]._hsGroup)
				feature->setDrawRecord(preparedHotspots[i]._hsGroup, hs, drawnRect);

			if (hasSortRect) {
				sortRect.extend(drawnRect);
			} else {
				sortRect = drawnRect;
				hasSortRect = true;
			}
		}

		if (hasSortRect) {
			feature->setSortRect(sortRect);
			if (!feature->hasExplicitClickRect() || snoid)
				feature->setRenderedClickRect(sortRect);
		} else {
			feature->setSortRect(Common::Rect());
			if (snoid)
				feature->setRenderedClickRect(Common::Rect());
		}

		return ZmbRenderResult::kRendered;
	}

	// Render sprites (Shape)
	ZmbHotspotGroup *hsGroup = feature->getHotspotGroup(frameIdx);
	if (!hsGroup)
		return ZmbRenderResult::kRendered;

	// Copy the hotspots because the shape pre-render hook may modify them.
	Common::Array<ZmbHotspot> hotspots = hsGroup->copyHotspots();
	if (!snoid && feature->hasFlag(ZmbFeature::FLAG_00800000_POS_DELTA)) {
		const Common::Point &posDelta = feature->getPosDelta();
		for (uint32 i = 0; i < hotspots.size(); i++) {
			hotspots[i]._x += posDelta.x;
			hotspots[i]._y += posDelta.y;
		}
	}
	feature->onPreRenderShape(this, hsGroup, hotspots);

	// Apply per-tBMP REGS offsets after @ref ZmbFeature::onPreRenderShape(),
	// which may have remapped @ref ZmbHotspot::_shapeIdx.
	ZmbShapeOffsetRegs *shapeRegs = snoid ? nullptr : feature->getShapeOffsetRegs();
	if (shapeRegs) {
		for (uint32 hotspotIdx = 0; hotspotIdx < hotspots.size(); hotspotIdx++) {
			ZmbHotspot &hs = hotspots[hotspotIdx];
			if (hs._shapeIdx != ZmbHotspot::kShapeNone) {
				const Common::Point delta = shapeRegs->getShapeDelta(hs._shapeIdx);
				hs._x -= delta.x;
				hs._y -= delta.y;
			}
		}
	}

	// Draw shapes to screen
	feature->clearDrawRecords();
	Common::Rect sortRect;
	bool hasSortRect = false;

	// Apply the SCRS base-shape layer shift for normal scripted Snoids.
	uint8 snoidLayerShift = 0;
	if (snoid) {
		if (!snoid->hasCombinedShapeIndices() &&
			snoid->getAnimState() == kSnoidAnimState009_ScriptNormal) {
			if (!hotspots.empty() && 18 < hotspots[0]._shapeIdx)
				snoidLayerShift = 1;
		}
	}

	for (uint32 hotspotIdx = 0; hotspotIdx < hotspots.size(); hotspotIdx++) {
		ZmbHotspot &hs = hotspots[hotspotIdx];
		ZmbResource shapeRes = feature->getResource();
		if (snoid) {
			if (!transformSnoidHotspotForRender(snoid, hs, snoidLayerShift, shapeRes))
				break;
		} else {
			if (hs._shapeIdx == ZmbHotspot::kShapeNone)
				continue;
		}

		bool clearBeforeRender = false;
		ZoombiniGraphics::PaletteRemapMode shapeRemap = remapColorAssistPalette;
		if (shapeRemap == ZoombiniGraphics::kPaletteRemapNone)
			shapeRemap = getColorAssistPaletteRemap(feature, hs, shapeRes);
		Common::Rect drawnRect = _vm->_gfx->drawShape(screenKind, shapeRes, &hs, clearBeforeRender, shapeRemap);
		drawnRect = feature->constrainVisualRect(drawnRect);
		if (drawnRect.isEmpty())
			continue;

		feature->setDrawRecord(hsGroup, hs, drawnRect);

		if (hasSortRect) {
			sortRect.extend(drawnRect);
		} else {
			sortRect = drawnRect;
			hasSortRect = true;
		}
	}
	if (hasSortRect) {
		feature->setSortRect(sortRect);
		// Snoids rebuild their click rectangle each frame to follow the rendered bounding box during movement.
		// Standard SCRB runners also rebuild their aggregate rectangle from each transformed shape pass.
		// Features with an explicit page-owned rectangle retain that manual hit-test zone.
		// @ref ZmbFeature::_sortRect stores the current visual rectangle for dirty invalidation and Z-sort.
		if (!feature->hasExplicitClickRect() || snoid)
			feature->setRenderedClickRect(sortRect);
	} else if (snoid) {
		feature->setSortRect(Common::Rect());
		feature->setRenderedClickRect(Common::Rect());
	}

	return ZmbRenderResult::kRendered;
}

int32 ZoombiniPage::selectRenderFrame(ZmbFeature *feature) {
	return feature->defaultSelectRenderFrame(_currentFrameCounter);
}

Common::Rect ZoombiniPage::renderStoredSnoid(ZoombiniGraphics::ScreenKind screenKind, const ZmbTrait &trait, const Common::Point &pos) {
	// Build the five-hotspot idle pose produced by @ref ZmbSnoid::setupIdleHotspots().
	// Then apply the @ref ZoombiniPage::blitShapes() transform: feature position offset, right-facing mirror (2*idx-1),
	// and REGS registration-point correction.
	static constexpr uint16 kFeetTable[6] = {0, 191, 246, 335, 360, 411};
	static constexpr uint16 kNoseTable[6] = {0, 171, 175, 179, 183, 187};
	static constexpr uint16 kEyeTable[6] = {0, 91, 107, 123, 139, 155};
	static constexpr uint16 kHairTable[6] = {0, 11, 27, 43, 59, 75};
	static constexpr uint16 kRawShapeIdle = 2;

	uint8 feet = CLIP<uint8>(trait._feet, 1, 5);
	uint8 nose = CLIP<uint8>(trait._nose, 1, 5);
	uint8 eye = CLIP<uint8>(trait._eyes, 1, 5);
	uint8 hair = CLIP<uint8>(trait._hair, 1, 5);

	// Combined shape indices add each trait offset to the raw idle shape.
	const uint16 combined[5] = {
		static_cast<uint16>(kFeetTable[feet] + kRawShapeIdle), // slot 0: feet
		static_cast<uint16>(0 + kRawShapeIdle),                // slot 1: body anchor
		static_cast<uint16>(kNoseTable[nose] + kRawShapeIdle), // slot 2: nose
		static_cast<uint16>(kEyeTable[eye] + kRawShapeIdle),   // slot 3: eye
		static_cast<uint16>(kHairTable[hair] + kRawShapeIdle), // slot 4: hair
	};

	ZmbResource snoidRes(ZmbResource::kSystem, 3000);
	Common::Rect sortRect;
	bool hasSortRect = false;

	for (int layer = 0; layer < 5; layer++) {
		uint16 shapeIdx = combined[layer];
		if (shapeIdx == ZmbHotspot::kShapeNone)
			continue;

		int16 sx = pos.x;
		int16 sy = pos.y;

		// Map the combined trait-offset index to its right-facing shape.
		uint16 finalShape = 2 * shapeIdx - 1;

		// Apply REGS registration-point correction
		if (_vm->_snoidShapeOffsetRegs) {
			const Common::Point delta = _vm->_snoidShapeOffsetRegs->getShapeDelta(finalShape);
			sx -= static_cast<int16>(delta.x);
			sy -= static_cast<int16>(delta.y);
		}

		ZmbHotspot hs(layer, finalShape, 0, sx, sy);
		const Common::Rect &drawnRect = _vm->_gfx->drawShape(screenKind, snoidRes, &hs, false);

		if (hasSortRect)
			sortRect.extend(drawnRect);
		else {
			sortRect = drawnRect;
			hasSortRect = true;
		}
	}

	return sortRect;
}

void ZoombiniPage::clear() {
	_featureTimingGroups.clear();
	_scrbImageResources.clear();
	clearSubFeatures();
	clearScrbFeatures();
	clearMainFeatureHeads();
	clearSnoids();
	clearRegs();
	clearNode();
	clearTerrainBitmap();
	resetDrawOnRegSlots();
	_runnerOrder.clear();
}

void ZoombiniPage::clearScrbFeatures() {
	for (ZmbFeature *f : _scrbFeatures) {
		unregisterFeatureTimingGroup(f);
		delete f;
	}
	_scrbFeatures.clear();
}

void ZoombiniPage::clearMainFeatureHeads() {
	for (uint i = 0; i < _mainFeatureHeads.size(); i++) {
		delete _mainFeatureHeads[i];
	}
	_mainFeatureHeads.clear();
}

void ZoombiniPage::clearSubFeatures() {
	// Sub-features are owned by their parent features - only clear the map, do NOT delete.
	for (ZmbFeature *f : _subFeatures) {
		unregisterFeatureTimingGroup(f);
		f->setSubFeatureRunning(false);
		f->clearDetach();
	}
	_subFeatures.clear();
}

void ZoombiniPage::clearSnoids() {
	for (ZmbSnoid *s : _snoidMap) {
		unregisterFeatureTimingGroup(s);
		runnerOrderErase(s);
		delete s;
	}
	_snoidMap.clear();
}

ZmbSnoid *ZoombiniPage::loadSnoid(ZmbResource imgResource, int16 scrsId, uint32 flags, const ZmbFeature::EventHooks &eventHooks) {
	return loadSnoid(imgResource, scrsId, Common::Point(0, 0), flags, eventHooks);
}

ZmbSnoid *ZoombiniPage::loadSnoid(ZmbResource imgResource, int16 scrsId, const Common::Point &point, uint32 flags, const ZmbFeature::EventHooks &eventHooks) {
	const uint16 runnerId = static_cast<uint16>(scrsId);
	if (_snoidMap.find(runnerId)) {
		error("page: duplicate snoid id %d", scrsId);
		return nullptr;
	}

	ZmbSnoid *snoid = new ZmbSnoid(_vm, runnerId, flags);
	_snoidMap.insert(runnerId, snoid);
	snoid->setRegistrationIndex(_nextRegistrationIndex);
	_nextRegistrationIndex += 1;

	snoid->setPointLoc(point);
	snoid->setResource(imgResource);

	if (!snoid->loadScrsResource(ZmbResource(imgResource._archiveKind, scrsId))) {
		error("page: required SCRS %d is malformed", scrsId);
		return nullptr;
	}

	snoid->initValues();
	snoid->setEventHooks(eventHooks);

	return snoid;
}

ZmbSnoid *ZoombiniPage::loadSnoidFromPack(uint16 snoidId, const Common::Point &point, uint32 flags, const ZmbFeature::EventHooks &eventHooks) {
	if (_snoidMap.find(snoidId)) {
		error("page: duplicate snoid id %u", snoidId);
		return nullptr;
	}

	ZmbSnoid *snoid = new ZmbSnoid(_vm, snoidId, flags);
	snoid->markAsPackSnoid();
	_snoidMap.insert(snoidId, snoid);
	snoid->setRegistrationIndex(_nextRegistrationIndex);
	_nextRegistrationIndex += 1;

	snoid->setPointLoc(point);
	// Set the animation destination equal to the feature position at load time.
	// This is used as the sort key (sorts by animation destination.x).
	snoid->setAnimTargetPos(point);
	// Seed the z-sort rect so the very first frame sorts correctly by position.
	// @ref ZoombiniPage::blitShapes() overwrites this with the actual bounding box each frame.
	snoid->setSortRect(Common::Rect(point.x, point.y, point.x + 1, point.y + 1));
	// Pack snoids use the global system shapes (ZOOMBINI.MHK tBMP 3000).
	// @ref ZmbSnoid::setupIdleHotspots() builds hotspot data programmatically from traits.
	snoid->setResource(ZmbResource(ZmbResource::kSystem, 3000));

	// No SCRS resource parsing - traits/name are set by the caller from pack data

	snoid->initValues();
	snoid->setEventHooks(eventHooks);

	return snoid;
}

uint16 ZoombiniPage::getDynamicPackSnoidId(uint16 snoidIdx) const {
	static constexpr uint16 kDynamicPackSnoidIdBase = 60000;
	return kDynamicPackSnoidIdBase + snoidIdx;
}

uint16 ZoombiniPage::allocateDynamicPackSnoidId() {
	const uint16 snoidId = getDynamicPackSnoidId(_nextDynamicPackSnoidIdx);
	_nextDynamicPackSnoidIdx += 1;
	return snoidId;
}

int16 ZoombiniPage::loadSnoidsFromPack(ZmbStateActivePack &pack,
									   const Common::Point *occupiedPositions,
									   uint16 occupiedPositionCount,
									   bool loadNonOccupied,
									   uint16 firstSnoidId,
									   Common::Array<ZmbSnoid *> *loadedSnoids) {
	int16 loadedCount = 0;
	int16 totalPackRunnerCount = 0;
	uint16 occupiedPositionIdx = 0;
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		if (*it && (*it)->isPackSnoid())
			totalPackRunnerCount += 1;
	}

	if (loadedSnoids)
		loadedSnoids->clear();

	for (int16 runnerIdx = 0; runnerIdx < pack.getPackZmbCount() &&
							  runnerIdx < pack.getEntryCapacity() &&
							  totalPackRunnerCount < pack.getEntryCapacity();
		 runnerIdx++) {
		ZmbStateActiveEntry &entry = pack.getEntry(runnerIdx);
		const bool isOccupied = entry.getIsOccupied();
		if (isOccupied && pack.getSkipOccupiedEntries())
			continue;
		if (!isOccupied && (!loadNonOccupied || pack.getSkipUnoccupiedEntries()))
			continue;

		Common::Point position;
		if (isOccupied) {
			if (occupiedPositionIdx < occupiedPositionCount)
				position = occupiedPositions[occupiedPositionIdx];
			else
				position = Common::Point();
			occupiedPositionIdx += 1;
		} else {
			position = entry.getPos();
		}

		// Every active-pack entry that reaches registration receives an independent
		// idle phase, even on pages that temporarily disable fidget animation.
		const uint8 idleTickCounter = static_cast<uint8>(_vm->_rnd->getRandomNumber(0, 64));
		ZmbSnoid *snoid = loadSnoidFromPack(firstSnoidId + loadedCount, position, ZmbFeature::FLAG_00000001_TYPE_SNOID);
		if (snoid) {
			snoid->setIdleTickCounter(idleTickCounter);
			snoid->_trait = entry.getTraits();
			snoid->_name = entry.getU32Name(_vm);
			snoid->_packIsOccupied = isOccupied;
			snoid->setupIdleHotspots();
			if (loadedSnoids)
				loadedSnoids->push_back(snoid);
		}
		loadedCount += 1;
		totalPackRunnerCount += 1;
	}

	// This is the destructive boundary of the ownership transfer.
	// Serialized entries now live only in registered Snoid runners.
	pack.clearEntries();
	return loadedCount;
}

int16 ZoombiniPage::loadDynamicSnoidsFromPack(ZmbStateActivePack &pack,
											  const Common::Point *occupiedPositions,
											  uint16 occupiedPositionCount,
											  bool loadNonOccupied,
											  Common::Array<ZmbSnoid *> *loadedSnoids) {
	const int16 loadedCount = loadSnoidsFromPack(pack, occupiedPositions, occupiedPositionCount,
												 loadNonOccupied, getDynamicPackSnoidId(_nextDynamicPackSnoidIdx),
												 loadedSnoids);
	_nextDynamicPackSnoidIdx += loadedCount;
	return loadedCount;
}

void ZoombiniPage::schedulePackSnoids(bool activateRender, bool occupied) {
	uint16 occupiedCount = 0;
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		ZmbSnoid *snoid = *it;
		if (!snoid || !snoid->isPackSnoid())
			continue;

		if (activateRender)
			snoid->activateRender();

		if (occupied && snoid->isRenderActivated() && occupiedCount < 20) {
			snoid->_packIsOccupied = true;
			occupiedCount += 1;
		} else {
			snoid->_packIsOccupied = false;
		}
	}
}

void ZoombiniPage::saveSnoidsToPack(bool saveMode) {
	// Two-pass: occupied snoids first, then non-occupied.
	ZmbStateFile &f = _vm->_state->getCurrentState();

	if (!saveMode)
		_vm->setArrivalTurnDirection(ArrivalTurnDirection::kRight);

	// Reset pack-entry filtering flags.
	f._zmbPackActive.setSkipOccupiedEntries(false);
	f._zmbPackActive.setSkipUnoccupiedEntries(false);

	// Reactivate hidden runners and clear their occupancy so they are serialized by the non-occupied pass.
	// An already-visible runner keeps its normalized boolean occupancy state.
	if (!saveMode) {
		for (ZmbFeatureList<ZmbSnoid>::iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
			ZmbSnoid *snoid = *it;
			if (!snoid->isPackSnoid())
				continue;
			if (!snoid->isRenderActivated()) {
				snoid->activateRender();
				snoid->_packIsOccupied = false;
			}
		}
	}

	// Save mode makes both passes accept every runner and forces occupied=1.
	// BC1 and BC2 override save mode and retain the occupied/non-occupied split.
	const bool forceOccupied = saveMode && _pageType != ZoombiniPageType::kBasecamp1 &&
							   _pageType != ZoombiniPageType::kBasecamp2;
	f._zmbPackActive.clearEntries();
	int16 serializedCount = 0;
	for (ZmbFeatureList<ZmbSnoid>::iterator it = _snoidMap.begin(); it != _snoidMap.end() &&
																	serializedCount < f._zmbPackActive.getEntryCapacity();
		 it++) {
		if (*it && (*it)->isPackSnoid())
			serializedCount += 1;
	}
	int16 destIdx = 0;
	for (int pass = 0; pass < 2 && destIdx < f._zmbPackActive.getEntryCapacity(); pass++) {
		const bool wantOccupied = pass == 0;
		for (ZmbFeatureList<ZmbSnoid>::iterator it = _snoidMap.begin(); it != _snoidMap.end() &&
																		destIdx < f._zmbPackActive.getEntryCapacity();
			 it++) {
			ZmbSnoid *snoid = *it;
			if (!snoid->isPackSnoid() || (!forceOccupied && snoid->_packIsOccupied != wantOccupied))
				continue;

			ZmbStateActiveEntry entry;
			entry.setTraits(snoid->_trait);
			entry.setPos(snoid->getPointLoc());
			entry.setIsOccupied(forceOccupied || wantOccupied);
			entry.setU32Name(_vm, snoid->_name);

			bool wroteEntry;
			if (forceOccupied && pass == 1)
				wroteEntry = f._zmbPackActive.writeEntryAt(destIdx, entry);
			else
				wroteEntry = f._zmbPackActive.appendEntry(entry);
			if (!wroteEntry)
				break;
			destIdx += 1;
		}
	}

	// In save mode, both passes accept every runner.
	// The unused tail receives duplicate entries, while the meaningful count remains the runner count.
}

int16 ZoombiniPage::getPackSnoidCount() const {
	int16 count = 0;
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		if (*it && (*it)->isPackSnoid())
			count += 1;
	}
	return count;
}

ZmbSnoid *ZoombiniPage::loadSnoidFromScrb(ZmbResource imgResource, uint16 snoidId, int16 scrbId, const Common::Point &point, uint32 flags, const ZmbFeature::EventHooks &eventHooks) {
	if (_snoidMap.find(snoidId)) {
		error("page: duplicate snoid id %u", snoidId);
		return nullptr;
	}

	ZmbSnoid *snoid = new ZmbSnoid(_vm, snoidId, flags);
	snoid->setScrbId(scrbId);
	_snoidMap.insert(snoidId, snoid);
	snoid->setRegistrationIndex(_nextRegistrationIndex);
	_nextRegistrationIndex += 1;

	snoid->setPointLoc(point);
	snoid->setSortRect(Common::Rect(point.x, point.y, point.x + 1, point.y + 1));
	snoid->setResource(imgResource);

	// Inhabitants use cached SCRB data rather than SCRS playback.
	const ZmbScriptDecoder::DecodedScrb *decodedScrb = getDecodedScrb(ZmbResource(imgResource._archiveKind, scrbId));
	if (!snoid->setDecodedScrb(decodedScrb, scrbId)) {
		error("page: required Snoid SCRB %d is malformed", scrbId);
		return nullptr;
	}

	snoid->initValues();
	snoid->setEventHooks(eventHooks);

	return snoid;
}

void ZoombiniPage::unloadSnoid(uint16 runnerId) {
	ZmbSnoid *snoid = _snoidMap.erase(runnerId);
	if (!snoid)
		return;
	unregisterFeatureTimingGroup(snoid);
	runnerOrderErase(snoid);
	delete snoid;
}

ZmbSnoid *ZoombiniPage::getSnoid(uint16 runnerId) const {
	return _snoidMap.find(runnerId);
}

ZmbSnoid *ZoombiniPage::getIdleSnoid(uint16 runnerId) const {
	ZmbSnoid *snoid = getSnoid(runnerId);
	if (snoid && snoid->getAnimState() == kSnoidAnimState000_Idle)
		return snoid;
	return nullptr;
}

bool ZoombiniPage::isPointOccupiedByOtherSnoid(const ZmbSnoid *self, const Common::Point &pt, int32 distSquared) const {
	// Only states 0 (idle), 3 (flip), and 6 (fidget) are stationary collision candidates.
	// The threshold is a squared distance and comparison is strict.
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		const ZmbSnoid *other = *it;
		if (other == self)
			continue;
		// Only check stationary snoids (idle / flip / fidget)
		SnoidAnimState st = other->getAnimState();
		if (st != kSnoidAnimState000_Idle && st != kSnoidAnimState003_Flip && st != kSnoidAnimState006_Fidget)
			continue;
		const Common::Point &opos = other->getPointLoc();
		if (squaredDistanceBetweenPoints(opos, pt) < distSquared)
			return true;
	}
	return false;
}

void ZoombiniPage::beginSnoidDrag(ZmbSnoid *snoid) {
	assert(!_snoidDragFidgetsSuppressed);
	_dragSavedFidgetThreshold = _vm->_fidgetThreshold;
	_snoidDragFidgetsSuppressed = true;
	_vm->_fidgetThreshold = 0;

	// Queue the previously materialized click rectangle before replacing the pose.
	addExternalDirtyRect(snoid->getClickRect());

	// Save the current bitmask, then add TOPMOST and OVERLAY.
	// TOPMOST (0x1000) sends this incoming tail runner to the end of the sorted list.
	// OVERLAY (0x4000000) routes it into the overlay render bucket.
	_dragSavedSnoidFlags = snoid->getFlags();
	_dragSavedSnoidFrameInterval = snoid->getFrameInterval();
	snoid->addFlag(ZmbFeature::FLAG_00001000_TOPMOST);
	snoid->addFlag(ZmbFeature::FLAG_04000000_OVERLAY);
	// Move the dragged runner to the global-list tail.
	// Automatic Z-sort may place it back by depth after the drag flags are restored.
	syncRunnerOrder();
	_dragSavedRunnerPrevious = nullptr;
	_dragSavedRunnerNext = nullptr;
	for (uint32 runnerIdx = 0; runnerIdx < _runnerOrder.size(); runnerIdx++) {
		if (_runnerOrder[runnerIdx] != snoid)
			continue;
		if (0 < runnerIdx)
			_dragSavedRunnerPrevious = _runnerOrder[runnerIdx - 1];
		if (runnerIdx + 1 < _runnerOrder.size())
			_dragSavedRunnerNext = _runnerOrder[runnerIdx + 1];
		break;
	}
	runnerOrderErase(snoid);
	_runnerOrder.push_back(snoid);

	// Direct drag resets the animation deadline and temporarily halves the normal interval from 6 to 3.
	snoid->resetNextAnimFrame();
	snoid->setFrameInterval(3);
	snoid->clearPreparedRenderHotspots();
	snoid->setAnimState(kSnoidAnimState005_Drag);

	// Materialize the holding frame before the drag loop starts.
	// Normal Snoids use SCRS 146-150.
	// @ref ZmbSnoid::setAnimState() has already rebuilt the common-image pose for the small-body branch.
	// Preserve this frame until the interval-3 timer replaces it.
	if (!snoid->_useSmallShapeRegs)
		snoid->updateHoldingHotspots(this);
	prepareSnoidVisualCoverage(snoid, true);
	addExternalDirtyRect(snoid->getClickRect());

	_vm->_cursor->hideCursor();
	onSnoidDragStarted(snoid);
}

void ZoombiniPage::endSnoidDrag(ZmbSnoid *snoid) {
	// Queue the final materialized holding footprint before the caller selects its drop or return state.
	addExternalDirtyRect(snoid->getClickRect());
	snoid->clearPreparedRenderHotspots();
	snoid->setNeedsRedraw(true);

	// Restore the saved bitmask exactly.
	// Do not restore the runner's former linked-list position.
	snoid->setFlags(_dragSavedSnoidFlags);
	snoid->setFrameInterval(_dragSavedSnoidFrameInterval);
	restoreSnoidDragFidgets();

	_vm->_cursor->showCursor();
	onSnoidDragEnded(snoid);
}

void ZoombiniPage::restoreSnoidDragFidgets() {
	if (!_snoidDragFidgetsSuppressed)
		return;
	_vm->_fidgetThreshold = _dragSavedFidgetThreshold;
	_snoidDragFidgetsSuppressed = false;
}

void ZoombiniPage::restoreSnoidPreDragRunnerOrder(ZmbSnoid *snoid) {
	if (!snoid)
		return;

	syncRunnerOrder();
	runnerOrderErase(snoid);

	if (_dragSavedRunnerPrevious) {
		for (uint32 runnerIdx = 0; runnerIdx < _runnerOrder.size(); runnerIdx++) {
			if (_runnerOrder[runnerIdx] == _dragSavedRunnerPrevious) {
				_runnerOrder.insert_at(runnerIdx + 1, snoid);
				_dragSavedRunnerPrevious = nullptr;
				_dragSavedRunnerNext = nullptr;
				return;
			}
		}
	}

	if (_dragSavedRunnerNext) {
		for (uint32 runnerIdx = 0; runnerIdx < _runnerOrder.size(); runnerIdx++) {
			if (_runnerOrder[runnerIdx] == _dragSavedRunnerNext) {
				_runnerOrder.insert_at(runnerIdx, snoid);
				_dragSavedRunnerPrevious = nullptr;
				_dragSavedRunnerNext = nullptr;
				return;
			}
		}
	}

	_runnerOrder.push_back(snoid);
	_dragSavedRunnerPrevious = nullptr;
	_dragSavedRunnerNext = nullptr;
}

void ZoombiniPage::loadTerrainBitmap(int16 resId) {
	clearTerrainBitmap();

	// Load the tBMP Terrain resource.
	// The bitmap is 160x120 (screen / 4), 8bpp. Pixel value 1 = walkable.
	// The surface is cached by GraphicsManager; we just store a pointer.
	_terrainBitmap = _vm->_gfx->findImage(ZmbResource(ZmbResource::kPage, resId));
	if (_terrainBitmap) {
		const Graphics::Surface *surface = _terrainBitmap->getSurface();
		debug(3, "page: loaded terrain barrier bitmap: %dx%d (resource id %d)", surface->w, surface->h, resId);
	}
}

bool ZoombiniPage::isTerrainWalkable(int16 x, int16 y) const {
	if (!_terrainBitmap)
		return false; // No terrain loaded = position invalid

	const Graphics::Surface *surface = _terrainBitmap->getSurface();
	int16 terrainX = x / 4;
	int16 terrainY = y / 4;

	// Clamp to bitmap bounds
	terrainX = CLIP<int16>(terrainX, 0, surface->w - 1);
	terrainY = CLIP<int16>(terrainY, 0, surface->h - 1);

	const byte *pixels = reinterpret_cast<const byte *>(surface->getBasePtr(terrainX, terrainY));
	return *pixels == 1;
}

bool ZoombiniPage::validateTerrainDrop(ZmbSnoid *snoid) {
	// Check terrain walkability and then run the stationary-Snoid collision scan.
	// The collision scan runs even for blocked terrain so its random-number
	// consumption stays identical to the shared drag controller.

	static constexpr int32 kTerrainCollisionThreshold = 36; // ~6px radius

	const Common::Point pos = snoid->getPointLoc();
	const bool terrainValid = isTerrainWalkable(pos.x, pos.y);

	if (isPointOccupiedByOtherSnoid(snoid, pos, kTerrainCollisionThreshold)) {
		const Common::Point adjusted = findNonCollidingPosition(snoid, pos, kTerrainCollisionThreshold);
		// A blocked-terrain result discards the candidate and returns to the
		// pickup point, but it still performs the bounded candidate scan above.
		if (terrainValid)
			snoid->setPointLoc(adjusted);
	}

	return terrainValid;
}

void ZoombiniPage::settleSnoidAtTarget(ZmbSnoid *snoid, const Common::Point &target) {
	if (!snoid)
		return;

	// State 4 performs a one-tick position handoff and settles facing right.
	snoid->setAnimTargetPos(target);
	snoid->setCommonImageIndex(1);
	snoid->setFacingLeft(false);
	snoid->setAnimState(kSnoidAnimState004_Arrive);
}

bool ZoombiniPage::settleSnoidAfterTerrainDrop(ZmbSnoid *snoid, const Common::Point &pickupPos) {
	if (!snoid)
		return false;

	const Common::Point releasePos = snoid->getPointLoc();
	Common::Point target = pickupPos;
	if (validateTerrainDrop(snoid))
		target = snoid->getPointLoc();

	settleSnoidAtTarget(snoid, target);
	return target == releasePos;
}

Common::Point ZoombiniPage::findNonCollidingPosition(const ZmbSnoid *self, const Common::Point &origin, int32 distSquared) const {
	// Scans a 5x4 grid pattern (20 iterations). Each candidate:
	//   x = origin.x + 4 * random(-5, 5),  y = origin.y
	// Keep the first non-colliding position. If all collide, retain the original destination.

	// 5 columns x 4 rows = 20 attempts
	for (int row = 1; row <= 4; row++) {
		for (int col = 1; col <= 5; col++) {
			// Random x offset in [-20, +20] in steps of 4.
			int16 randOffset = static_cast<int16>(4 * (_vm->_rnd->getRandomNumber(10) - 5));
			Common::Point candidate(origin.x + randOffset, origin.y);

			// Clamp to screen bounds
			candidate.x = CLIP<int16>(candidate.x, 0, 640);
			candidate.y = CLIP<int16>(candidate.y, 0, 480);

			if (!isPointOccupiedByOtherSnoid(self, candidate, distSquared))
				return candidate;
		}
	}

	return origin;
}

Common::Point ZoombiniPage::findNonCollidingPosition(const ZmbSnoid *self, const Common::Rect &rect, int32 distSquared) const {
	const int16 width = rect.width();
	const int16 height = rect.height();
	const int16 halfCellWidth = width / 10;
	const Common::Point originalTarget = self->getAnimTargetPos();

	for (int16 row = 1; row <= 4; row++) {
		for (int16 col = 1; col <= 5; col++) {
			Common::Point candidate(rect.left + col * width / 5 + _vm->_rnd->getRandomNumber(0, 5), rect.top + row * height / 4);
			if ((row & 1) == 0)
				candidate.x += halfCellWidth;
			candidate.x = CLIP<int16>(candidate.x, 0, 640);
			candidate.y = CLIP<int16>(candidate.y, 0, 480);
			if (!isPointOccupiedByOtherSnoid(self, candidate, distSquared))
				return candidate;
		}
	}

	return originalTarget;
}

void ZoombiniPage::clearTerrainBitmap() {
	// Not owned by us - cached by GraphicsManager, freed on archive clear.
	_terrainBitmap = nullptr;
}

bool ZoombiniPage::applyTerrainMaskToShapeScreen() {
	if (!_terrainBitmap)
		return false;

	const Graphics::Surface *surface = _terrainBitmap->getSurface();
	if (!surface || surface->w <= 0 || surface->h <= 0)
		return false;

	const int screenWidth = ZoombiniGraphics::kScreenWidth;
	const int screenHeight = ZoombiniGraphics::kScreenHeight;
	const int terrainCellWidth = 4;
	const int terrainCellCountX = (screenWidth + terrainCellWidth - 1) / terrainCellWidth;
	const int terrainCellCountY = (screenHeight + terrainCellWidth - 1) / terrainCellWidth;

	for (int terrainY = 0; terrainY < terrainCellCountY; terrainY++) {
		const int sourceY = MIN<int>(terrainY, surface->h - 1);
		int maskedStartX = -1;

		for (int terrainX = 0; terrainX <= terrainCellCountX; terrainX++) {
			bool masked = false;
			if (terrainX < terrainCellCountX) {
				const int sourceX = MIN<int>(terrainX, surface->w - 1);
				const byte *pixel = reinterpret_cast<const byte *>(surface->getBasePtr(sourceX, sourceY));
				masked = *pixel != 1;
			}

			if (masked && maskedStartX < 0) {
				maskedStartX = terrainX;
			} else if (!masked && 0 <= maskedStartX) {
				const Common::Rect maskedRect(maskedStartX * terrainCellWidth, terrainY * terrainCellWidth,
											  MIN<int>(terrainX * terrainCellWidth, screenWidth),
											  MIN<int>((terrainY + 1) * terrainCellWidth, screenHeight));
				_vm->_gfx->fillArea(ZoombiniGraphics::kShapeScreen, maskedRect, ZoombiniGraphics::kColor2D_Black);
				maskedStartX = -1;
			}
		}
	}

	return true;
}

// ---------------------------------------------------------------------------
// Draw-on-Region Slot System
// ---------------------------------------------------------------------------

void ZoombiniPage::setDrawOnRegSnapPosition(int16 slotIdx, const Common::Point &pos) {
	assert(0 <= slotIdx && slotIdx < _drawOnRegCount);
	_drawOnRegSnapPositions[slotIdx] = pos;
}

uint16 ZoombiniPage::getDrawOnRegOccupant(int16 slotIdx) const {
	assert(0 <= slotIdx && slotIdx < _drawOnRegCount);
	return _drawOnRegOccupancy[slotIdx];
}

void ZoombiniPage::setDrawOnRegOccupant(int16 slotIdx, uint16 occupantId) {
	assert(0 <= slotIdx && slotIdx < _drawOnRegCount);
	_drawOnRegOccupancy[slotIdx] = occupantId;
}

void ZoombiniPage::clearDrawOnRegOccupant(int16 slotIdx) {
	assert(0 <= slotIdx && slotIdx < _drawOnRegCount);
	_drawOnRegOccupancy[slotIdx] = 0;
}

int16 ZoombiniPage::findDrawOnRegSlotByOccupant(uint16 occupantId) const {
	for (int16 i = 0; i < _drawOnRegCount; i++) {
		if (_drawOnRegOccupancy[i] == occupantId)
			return i;
	}
	return -1;
}

int16 ZoombiniPage::hitTestDrawOnRegSlot(const Common::Point &pos, int16 zoneRadius, bool emptyOnly) const {
	// Build a rectangle centered on @p pos with @p zoneRadius, then test each slot's snap position.
	Common::Rect zoneRect(pos.x - zoneRadius, pos.y - zoneRadius,
						  pos.x + zoneRadius, pos.y + zoneRadius);
	for (int16 i = 0; i < _drawOnRegCount; i++) {
		if (emptyOnly && _drawOnRegOccupancy[i] != 0)
			continue;
		if (zoneRect.contains(_drawOnRegSnapPositions[i].x, _drawOnRegSnapPositions[i].y))
			return i;
	}
	return -1;
}

void ZoombiniPage::resetDrawOnRegSlots() {
	// Reset the count.
	for (int16 i = 0; i < kMaxDrawOnRegSlots; i++) {
		_drawOnRegFeatures[i] = nullptr;
		_drawOnRegOccupancy[i] = 0;
	}
	_drawOnRegCount = 0;
}

const ZmbScriptDecoder::DecodedScrb *ZoombiniPage::getDecodedScrb(ZmbResource scrbRes) {
	if (scrbRes._archiveKind == ZmbResource::kSystem)
		return _vm->getSystemDecodedScrb(scrbRes._id);

	Common::StableMap<int16, ZmbScriptDecoder::DecodedScrb>::iterator it = _decodedScrbCache.find(scrbRes._id);
	if (it != _decodedScrbCache.end())
		return &it->second;

	ZmbScriptDecoder::DecodedScrb decodedScrb;
	if (!ZmbScriptDecoder::decodeScrbResource(_vm, scrbRes, decodedScrb))
		return nullptr;

	ZmbScriptDecoder::DecodedScrb &cachedScrb = _decodedScrbCache[scrbRes._id];
	cachedScrb = decodedScrb;
	return &cachedScrb;
}

const ZmbScriptDecoder::DecodedScrs *ZoombiniPage::getDecodedScrs(ZmbResource scrsRes) {
	if (scrsRes._archiveKind == ZmbResource::kSystem)
		return _vm->getSystemDecodedScrs(scrsRes._id);

	Common::StableMap<int16, ZmbScriptDecoder::DecodedScrs>::iterator it = _decodedScrsCache.find(scrsRes._id);
	if (it != _decodedScrsCache.end())
		return &it->second;

	ZmbScriptDecoder::DecodedScrs decodedScrs;
	if (!ZmbScriptDecoder::decodeScrsResource(_vm, scrsRes, decodedScrs))
		return nullptr;

	ZmbScriptDecoder::DecodedScrs &cachedScrs = _decodedScrsCache[scrsRes._id];
	cachedScrs = decodedScrs;
	return &cachedScrs;
}

void ZoombiniPage::registerScrsGroup(int16 baseId, uint16 count) {
	// Groups are filled in call order.
	// The first registered group becomes group 0 (NORMAL, state 9).
	// The second becomes group 1 (REJECT, state 8).
	// At most two groups are allowed.
	if (2 <= _scrsGroupNum)
		return;
	_scrsGroupBase[_scrsGroupNum] = baseId;
	_scrsGroupCount[_scrsGroupNum] = count;
	_scrsGroupNum += 1;
}

ZmbScrsPlaybackMode ZoombiniPage::resolveScrsRejectState(int16 scrsId) const {
	// The SCRS ID's owning group selects the render state.
	// Group 1 uses reject script state 8.
	// Group 0 and unregistered IDs use normal script state 9.
	for (int scriptIdx = 0; scriptIdx < _scrsGroupNum; scriptIdx++) {
		if (_scrsGroupBase[scriptIdx] <= scrsId &&
			scrsId < static_cast<int>(_scrsGroupBase[scriptIdx]) + _scrsGroupCount[scriptIdx])
			return scriptIdx == 1 ? ZmbScrsPlaybackMode::kReject : ZmbScrsPlaybackMode::kNormal;
	}
	return ZmbScrsPlaybackMode::kNormal;
}

bool ZoombiniPage::startSnoidScrs(ZmbSnoid *snoid, ZmbResource scrsResource, ZmbScrsCompletionMode completionMode,
								  const Common::Point *endPos, bool suppressVoiceEvents) {
	if (!snoid)
		return false;

	// Preserve both sides of an immediate SCRS replacement.
	// The start can occur between render passes or from a frame callback,
	// so queue each rectangle in both dirty accumulators.
	const Common::Rect oldClickRect = snoid->getClickRect();
	addDirtyRect(oldClickRect);
	addExternalDirtyRect(oldClickRect);
	if (snoid->hasPreparedVisualRects()) {
		Common::Array<Common::Rect> oldPreparedVisualRects;
		snoid->collectPreparedVisualRects(oldPreparedVisualRects);
		for (uint32 i = 0; i < oldPreparedVisualRects.size(); i++) {
			addDirtyRect(oldPreparedVisualRects[i]);
			addExternalDirtyRect(oldPreparedVisualRects[i]);
		}
	}

	// State 8 vs 9 comes from the registered SCRS group, never a hardcoded flag.
	if (!snoid->startScrsPlayback(scrsResource, completionMode, resolveScrsRejectState(scrsResource._id), endPos, suppressVoiceEvents))
		return false;
	prepareSnoidVisualCoverage(snoid, true);
	const Common::Rect &newClickRect = snoid->getClickRect();
	addDirtyRect(newClickRect);
	addExternalDirtyRect(newClickRect);
	snoid->setNeedsRedraw(true);
	return true;
}

void ZoombiniPage::clearRegs() {
	for (Common::HashMap<int16, ZmbShapeOffsetRegs *>::iterator it = _shapeOffsetRegsMap.begin(); it != _shapeOffsetRegsMap.end(); it++) {
		ZmbShapeOffsetRegs *regs = it->_value;
		delete regs;
	}
	_shapeOffsetRegsMap.clear();
}

void ZoombiniPage::clearNode() {
	for (Common::HashMap<int16, ZmbNode *>::iterator it = _nodeMap.begin(); it != _nodeMap.end(); it++) {
		ZmbNode *node = it->_value;
		delete node;
	}
	_nodeMap.clear();
}

void ZoombiniPage::onFadeIn() {
	if (!_useFadeEffect)
		return;

	// The v2.0US/TLC release does not use normal page enter/leave fades.
	if (_vm->isVersionFamilyTlcV2())
		return;

	// v1.x releases use 500 ms when no frame time is supplied.
	_vm->_gfx->queueFadeEffect(ZoombiniGraphics::kFadeIn, 500);
}

void ZoombiniPage::onFadeOut() {
	if (!_useFadeEffect)
		return;

	// The v2.0US/TLC release does not use normal page enter/leave fades.
	if (_vm->isVersionFamilyTlcV2())
		return;

	// v1.x releases use the same 500 ms default.
	_vm->_gfx->queueFadeEffect(ZoombiniGraphics::kFadeOut, 500);
}

void ZoombiniPage::genericButton_selectShapes(ZmbFeature *feature, Common::Array<ZmbHotspot> &hotspots, Common::StableMap<uint32, ButtonState> &buttonStateMap, uint16 pressedDeltaX, uint16 pressedDeltaY, bool validateHoverShapeCount) {
	if (!feature) {
		error("page: button callback has no feature");
		return;
	}

	bool continueRedraw = false;
	for (Common::StableMap<uint32, ButtonState>::iterator it = buttonStateMap.begin(); it != buttonStateMap.end(); it++) {
		ButtonState &bs = it->second;

		if (!bs._drawEnabled)
			continue;
		if (hotspots.size() <= bs._hsNormalId || hotspots.size() <= bs._hsPressedId) {
			error("page: required button SCRB hotspot table is malformed");
			return;
		}

		ZmbHotspot &hsNormal = hotspots[bs._hsNormalId];
		ZmbHotspot &hsPressed = hotspots[bs._hsPressedId];
		const bool useHoverShape = (!bs.hasDisabledState() || !bs._isPressDisabled) && bs.hasHoverState() && bs._isHovered &&
								   (!validateHoverShapeCount || bs._shapeHoverIdx <= _vm->_gfx->getShapeCount(feature->getResource()));
		const bool hoverMasksPress = _vm->isVersionFamilyTlcV2() && useHoverShape;

		if (bs.hasDisabledState() && bs._isPressDisabled)
			hsNormal._shapeIdx = bs._shapeDisabledIdx;
		else if (useHoverShape)
			hsNormal._shapeIdx = bs._shapeHoverIdx;
		else
			hsNormal._shapeIdx = bs._shapeNormalIdx;

		bool disableNormalHotspot = false;
		if (bs.isAnimating()) {
			continueRedraw = true;
			uint32 elapsedFrames = _currentFrameCounter - bs._animationStartFrame;
			if (elapsedFrames < bs._animationFrameCount) {
				// TLC v2 hover redraw remains visually dominant while a button press is active.
				disableNormalHotspot = !hoverMasksPress && (elapsedFrames < bs._animationFrameCount - 1);
			} else {
				bs._animationStartFrame = 0;
				bs._firePostAnimationEvent = true;
			}
		}

		if (disableNormalHotspot) {
			hsNormal._shapeIdx = ZmbHotspot::kShapeNone;
			hsPressed._x += pressedDeltaX;
			hsPressed._y += pressedDeltaY;
		} else {
			hsPressed._shapeIdx = ZmbHotspot::kShapeNone;
		}
	}

	// Static UI SCRBs do not advance their own render timer. Keep the owning
	// feature dirty through the release frame and the resulting button action.
	if (continueRedraw)
		feature->requestVisualRematerialization();
}

void ZoombiniPage::genericButton_updateHoverState(ZmbFeature *feature, const Common::Point &absPos, Common::StableMap<uint32, ButtonState> &buttonStateMap, const Common::HashMap<uint32, Common::Rect> &buttonRectMap) {
	if (!feature) {
		error("page: button hover update requires a feature");
		return;
	}

	bool changed = false;
	Common::Rect dirtyRect = feature->getZSortRect();

	for (Common::StableMap<uint32, ButtonState>::iterator it = buttonStateMap.begin(); it != buttonStateMap.end(); it++) {
		uint32 bsIdx = it->first;
		ButtonState &bs = it->second;
		bool hovered = false;

		if (bs._drawEnabled && bs.hasHoverState() && (!bs.hasDisabledState() || !bs._isPressDisabled)) {
			Common::HashMap<uint32, Common::Rect>::const_iterator rit = buttonRectMap.find(bsIdx);
			if (rit != buttonRectMap.end()) {
				const Common::Rect &buttonRect = rit->_value;
				hovered = buttonRect.contains(absPos);
				if (dirtyRect.isEmpty())
					dirtyRect = buttonRect;
				else
					dirtyRect.extend(buttonRect);
			}
		}

		changed |= bs.setHovered(hovered);
	}

	if (!changed)
		return;

	if (!dirtyRect.isEmpty())
		addExternalDirtyRect(dirtyRect);
	feature->requestVisualRematerialization();
}

void ZoombiniPage::genericButton_textRender(ZmbFeature *feature, Common::StableMap<uint32, ButtonState> &buttonStateMap, Graphics::TextAlign textAlign, int16 normalDeltaY, int16 pressedDeltaY) {
	ZoombiniGraphics::TextConf tc;
	tc._hAlign = textAlign;
	tc._vAlign = textAlign == Graphics::kTextAlignCenter ? Graphics::kTextAlignCenter : Graphics::kTextAlignStart;
	tc._wordWrap = false;
	genericButton_textRender(feature, buttonStateMap, tc, normalDeltaY, pressedDeltaY);
}

void ZoombiniPage::genericButton_textRender(ZmbFeature *feature, Common::StableMap<uint32, ButtonState> &buttonStateMap, const ZoombiniGraphics::TextConf &tc, int16 normalDeltaY, int16 pressedDeltaY) {
	if (!feature) {
		error("page: button text render requires a feature");
		return;
	}

	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;

	for (Common::StableMap<uint32, ButtonState>::iterator it = buttonStateMap.begin(); it != buttonStateMap.end(); it++) {
		ButtonState &bs = it->second;

		if (!bs._drawEnabled)
			continue;

		ZmbDrawRecord *record = feature->findDrawRecordByHotspotIdx(bs._hsNormalId, bs._hsPressedId);
		if (record == nullptr)
			continue;
		Common::Rect textRect = record->_drawnRect;

		// Apply text offset when button is pressed
		if (bs.isAnimating()) {
			textRect.top += pressedDeltaY;
			textRect.bottom += pressedDeltaY;
		} else {
			textRect.top += normalDeltaY;
			textRect.bottom += normalDeltaY;
		}

		if (bs._textKey != ZoombiniText::kNone)
			_vm->_gfx->drawText(screenKind, bs._textKey, textRect, tc);
	}
}

void ZoombiniPage::genericButton_textRender(ZmbFeature *feature, Common::StableMap<uint32, ButtonState> &buttonStateMap, ButtonGetRectFunc textRectFunc, const ZoombiniGraphics::TextConf &tc) {
	if (!feature || !textRectFunc) {
		error("page: button text render requires a feature and rectangle callback");
		return;
	}

	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;

	for (Common::StableMap<uint32, ButtonState>::iterator it = buttonStateMap.begin(); it != buttonStateMap.end(); it++) {
		ButtonState &bs = it->second;

		if (!bs._drawEnabled)
			continue;

		ZmbDrawRecord *record = feature->findDrawRecordByHotspotIdx(bs._hsNormalId, bs._hsPressedId);
		if (record == nullptr)
			continue;
		Common::Rect textRect = (this->*textRectFunc)(feature, it->first, bs, record->_drawnRect);

		if (bs._textKey != ZoombiniText::kNone)
			_vm->_gfx->drawText(screenKind, bs._textKey, textRect, tc);
	}
}

void ZoombiniPage::genericButton_action(ZmbFeature *feature, Common::StableMap<uint32, ButtonState> &buttonStateMap, OnButtonActionFunc onPostAnimationFunc) {
	if (!feature || !onPostAnimationFunc) {
		error("page: button action requires a feature and action callback");
		return;
	}

	// [Post-Animation Events]
	for (Common::StableMap<uint32, ButtonState>::iterator it = buttonStateMap.begin(); it != buttonStateMap.end(); it++) {
		uint32 bsIdx = it->first;
		ButtonState &bs = it->second;

		if (!bs._drawEnabled)
			continue;

		if (bs.hasDisabledState() && bs._isPressDisabled)
			continue;

		if (!bs._firePostAnimationEvent)
			continue;
		bs._firePostAnimationEvent = false;

		// The action may change button state or other visuals owned by this
		// feature. Render that post-action state on the following frame.
		feature->requestVisualRematerialization();
		(this->*onPostAnimationFunc)(feature, bsIdx, bs);
	}
}

ZmbEventHandleResult ZoombiniPage::genericButton_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, Common::StableMap<uint32, ButtonState> &buttonStateMap, OnButtonActionFunc onButtonActionFunc) {
	if (!feature) {
		error("page: button input requires a feature");
		return ZmbEventHandleResult::kPassthrough;
	}

	ZmbDrawRecord *drawRecord = feature->findDrawRecordAtPoint(absPos);
	if (!drawRecord)
		return ZmbEventHandleResult::kPassthrough;

	for (Common::StableMap<uint32, ButtonState>::iterator it = buttonStateMap.begin(); it != buttonStateMap.end(); it++) {
		uint32 bsIdx = it->first;
		ButtonState &bs = it->second;

		if (!bs._drawEnabled)
			continue;

		// Find the pressed button
		if (drawRecord->_hs._hsId != bs._hsNormalId && drawRecord->_hs._hsId != bs._hsPressedId)
			continue;

		if (bs.hasDisabledState() && bs._isPressDisabled) {
			// Zoombini has some buttons that can be clicked even when they are disabled,
			// but they won't trigger the button press animation and will directly trigger the action event.
			// Ex) Go button on the PICKER page, when not enough zoombinis are selected
			if (onButtonActionFunc != nullptr)
				(this->*onButtonActionFunc)(feature, bsIdx, bs);
			return ZmbEventHandleResult::kConsumed;
		}

		bs.press(_vm, feature, _currentFrameCounter);
		return ZmbEventHandleResult::kConsumed;
	}

	return ZmbEventHandleResult::kPassthrough;
}

ZmbEventHandleResult ZoombiniPage::genericButton_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, Common::StableMap<uint32, ButtonState> &buttonStateMap, const Common::HashMap<uint32, Common::Rect> &buttonRectMap, OnButtonActionFunc onButtonActionFunc) {
	if (!feature) {
		error("page: button input requires a feature");
		return ZmbEventHandleResult::kPassthrough;
	}

	// Hit-test explicit button rectangles instead of drawn rectangles.
	for (Common::StableMap<uint32, ButtonState>::iterator it = buttonStateMap.begin(); it != buttonStateMap.end(); it++) {
		uint32 bsIdx = it->first;
		ButtonState &bs = it->second;

		if (!bs._drawEnabled)
			continue;

		Common::HashMap<uint32, Common::Rect>::const_iterator rit = buttonRectMap.find(bsIdx);
		if (rit == buttonRectMap.end())
			continue;
		const Common::Rect &buttonRect = rit->_value;
		if (!buttonRect.contains(absPos))
			continue;

		if (bs.hasDisabledState() && bs._isPressDisabled) {
			// Zoombini has some buttons that can be clicked even when they are disabled,
			// but they won't trigger the button press animation and will directly trigger the action event.
			// Ex) Go button on the PICKER page, when not enough zoombinis are selected
			if (onButtonActionFunc != nullptr)
				(this->*onButtonActionFunc)(feature, bsIdx, bs);
			return ZmbEventHandleResult::kConsumed;
		}

		bs.press(_vm, feature, _currentFrameCounter);
		return ZmbEventHandleResult::kConsumed;
	}

	return ZmbEventHandleResult::kPassthrough;
}

ZmbEventHandleResult ZoombiniPage::genericButton_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, Common::StableMap<uint32, ButtonState> &buttonStateMap, ButtonGetRectFunc getRectFunc, OnButtonActionFunc onButtonActionFunc) {
	if (!feature || !getRectFunc) {
		error("page: button input requires a feature and rectangle callback");
		return ZmbEventHandleResult::kPassthrough;
	}

	// Hit-test callback-provided button rectangles instead of drawn rectangles.
	for (Common::StableMap<uint32, ButtonState>::iterator it = buttonStateMap.begin(); it != buttonStateMap.end(); it++) {
		uint32 bsIdx = it->first;
		ButtonState &bs = it->second;

		if (!bs._drawEnabled)
			continue;

		ZmbDrawRecord *record = feature->findDrawRecordByHotspotIdx(bs._hsNormalId, bs._hsPressedId);
		if (!record)
			continue;

		Common::Rect buttonRect = (this->*getRectFunc)(feature, bsIdx, bs, record->_drawnRect);
		if (!buttonRect.contains(absPos))
			continue;

		if (bs.hasDisabledState() && bs._isPressDisabled) {
			// Zoombini has some buttons that can be clicked even when they are disabled,
			// but they won't trigger the button press animation and will directly trigger the action event.
			// Ex) Go button on the PICKER page, when not enough zoombinis are selected
			if (onButtonActionFunc != nullptr)
				(this->*onButtonActionFunc)(feature, bsIdx, bs);
			return ZmbEventHandleResult::kConsumed;
		}

		bs.press(_vm, feature, _currentFrameCounter);
		return ZmbEventHandleResult::kConsumed;
	}

	return ZmbEventHandleResult::kPassthrough;
}

void ZoombiniPage::genericToggleButton_selectShapes(ZmbFeature *feature, Common::Array<ZmbHotspot> &hotspots, Common::StableMap<uint32, ToggleButtonState> &buttonStateMap, uint16 pressedDeltaX, uint16 pressedDeltaY) {
	if (!feature) {
		error("page: toggle-button callback has no feature");
		return;
	}

	bool continueRedraw = false;
	for (Common::StableMap<uint32, ToggleButtonState>::iterator it = buttonStateMap.begin(); it != buttonStateMap.end(); it++) {
		ToggleButtonState &bs = it->second;

		if (!bs._enabled)
			continue;
		if (hotspots.size() <= bs._hsNormalId || hotspots.size() <= bs._hsPressedId) {
			error("page: required toggle-button SCRB hotspot table is malformed");
			return;
		}

		ZmbHotspot &hsNormal = hotspots[bs._hsNormalId];
		ZmbHotspot &hsPressed = hotspots[bs._hsPressedId];
		if (!bs._toggleState) {
			hsNormal._shapeIdx = bs._offNormalShapeIdx;
			hsPressed._shapeIdx = bs._offPressedShapeIdx;
		}

		bool disableNormalHotspot = false;
		if (bs.isAnimating()) {
			continueRedraw = true;
			uint32 elapsedFrames = _currentFrameCounter - bs._animationStartFrame;
			if (elapsedFrames < bs._animationFrameCount) {
				disableNormalHotspot = (elapsedFrames < bs._animationFrameCount - 1);
			} else {
				bs._animationStartFrame = 0;
				bs._firePostAnimationEvent = true;
			}
		}

		if (disableNormalHotspot) {
			hsNormal._shapeIdx = ZmbHotspot::kShapeNone;
			hsPressed._x += pressedDeltaX;
			hsPressed._y += pressedDeltaY;
		} else {
			hsPressed._shapeIdx = ZmbHotspot::kShapeNone;
		}
	}

	if (continueRedraw)
		feature->requestVisualRematerialization();
}

void ZoombiniPage::genericToggleButton_textRender(ZmbFeature *feature, Common::StableMap<uint32, ToggleButtonState> &buttonStateMap, Graphics::TextAlign textAlign) {
	ZoombiniGraphics::TextConf tc;
	tc._hAlign = textAlign;
	tc._vAlign = textAlign == Graphics::kTextAlignCenter ? Graphics::kTextAlignCenter : Graphics::kTextAlignStart;
	genericToggleButton_textRender(feature, buttonStateMap, tc);
}

void ZoombiniPage::genericToggleButton_textRender(ZmbFeature *feature, Common::StableMap<uint32, ToggleButtonState> &buttonStateMap, const ZoombiniGraphics::TextConf &tc) {
	if (!feature) {
		error("page: toggle-button text render requires a feature");
		return;
	}

	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;

	for (Common::StableMap<uint32, ToggleButtonState>::iterator it = buttonStateMap.begin(); it != buttonStateMap.end(); it++) {
		ToggleButtonState &bs = it->second;

		if (!bs._enabled)
			continue;

		ZmbDrawRecord *record = feature->findDrawRecordByHotspotIdx(bs._hsNormalId, bs._hsPressedId);
		if (record == nullptr)
			continue;

		if (bs._textKey != ZoombiniText::kNone)
			_vm->_gfx->drawText(screenKind, bs._textKey, record->_drawnRect, tc);
	}
}

void ZoombiniPage::genericToggleButton_textRender(ZmbFeature *feature, Common::StableMap<uint32, ToggleButtonState> &buttonStateMap, ToggleButtonGetRectFunc textRectFunc, const ZoombiniGraphics::TextConf &tc) {
	if (!feature || !textRectFunc) {
		error("page: toggle-button text render requires a feature and rectangle callback");
		return;
	}

	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;

	for (Common::StableMap<uint32, ToggleButtonState>::iterator it = buttonStateMap.begin(); it != buttonStateMap.end(); it++) {
		ToggleButtonState &bs = it->second;

		if (!bs._enabled)
			continue;

		ZmbDrawRecord *record = feature->findDrawRecordByHotspotIdx(bs._hsNormalId, bs._hsPressedId);
		if (record == nullptr)
			continue;
		Common::Rect textRect = (this->*textRectFunc)(feature, it->first, bs, record->_drawnRect);

		if (bs._textKey != ZoombiniText::kNone)
			_vm->_gfx->drawText(screenKind, bs._textKey, textRect, tc);
	}
}

void ZoombiniPage::genericToggleButton_postAnimation(ZmbFeature *feature, Common::StableMap<uint32, ToggleButtonState> &buttonStateMap, OnToggleButtonPostAnimationFunc onPostAnimationFunc) {
	if (!feature || !onPostAnimationFunc) {
		error("page: toggle-button post-animation requires a feature and action callback");
		return;
	}

	// [Post-Animation Events]
	for (Common::StableMap<uint32, ToggleButtonState>::iterator it = buttonStateMap.begin(); it != buttonStateMap.end(); it++) {
		ToggleButtonState &bs = it->second;

		if (!bs._enabled)
			continue;

		if (!bs._firePostAnimationEvent)
			continue;
		bs._firePostAnimationEvent = false;

		feature->requestVisualRematerialization();
		(this->*onPostAnimationFunc)(feature, it->first, bs);
	}
}

ZmbEventHandleResult ZoombiniPage::genericToggleButton_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, Common::StableMap<uint32, ToggleButtonState> &buttonStateMap) {
	if (!feature) {
		error("page: toggle-button input requires a feature");
		return ZmbEventHandleResult::kPassthrough;
	}

	ZmbDrawRecord *drawRecord = feature->findDrawRecordAtPoint(absPos);
	if (!drawRecord)
		return ZmbEventHandleResult::kPassthrough;

	for (Common::StableMap<uint32, ToggleButtonState>::iterator it = buttonStateMap.begin(); it != buttonStateMap.end(); it++) {
		ToggleButtonState &bs = it->second;

		if (!bs._enabled)
			continue;

		// Find the pressed button
		if (drawRecord->_hs._hsId != bs._hsNormalId && drawRecord->_hs._hsId != bs._hsPressedId)
			continue;

		bs.press(_vm, feature, _currentFrameCounter);
		return ZmbEventHandleResult::kConsumed;
	}

	return ZmbEventHandleResult::kPassthrough;
}

ZmbEventHandleResult ZoombiniPage::genericToggleButton_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, Common::StableMap<uint32, ToggleButtonState> &buttonStateMap, const Common::HashMap<uint32, Common::Rect> &buttonRectMap) {
	if (!feature) {
		error("page: toggle-button input requires a feature");
		return ZmbEventHandleResult::kPassthrough;
	}

	// Hit-test explicit button rectangles instead of drawn rectangles.
	for (Common::StableMap<uint32, ToggleButtonState>::iterator it = buttonStateMap.begin(); it != buttonStateMap.end(); it++) {
		uint32 bsIdx = it->first;
		ToggleButtonState &bs = it->second;

		if (!bs._enabled)
			continue;

		Common::HashMap<uint32, Common::Rect>::const_iterator rit = buttonRectMap.find(bsIdx);
		if (rit == buttonRectMap.end())
			continue;
		const Common::Rect &buttonRect = rit->_value;
		if (!buttonRect.contains(absPos))
			continue;

		bs.press(_vm, feature, _currentFrameCounter);
		return ZmbEventHandleResult::kConsumed;
	}

	return ZmbEventHandleResult::kPassthrough;
}

ZmbEventHandleResult ZoombiniPage::genericToggleButton_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, Common::StableMap<uint32, ToggleButtonState> &buttonStateMap, ToggleButtonGetRectFunc getRectFunc) {
	if (!feature || !getRectFunc) {
		error("page: toggle-button input requires a feature and rectangle callback");
		return ZmbEventHandleResult::kPassthrough;
	}

	// Hit-test callback-provided button rectangles instead of drawn rectangles.
	for (Common::StableMap<uint32, ToggleButtonState>::iterator it = buttonStateMap.begin(); it != buttonStateMap.end(); it++) {
		uint32 bsIdx = it->first;
		ToggleButtonState &bs = it->second;

		if (!bs._enabled)
			continue;

		ZmbDrawRecord *record = feature->findDrawRecordByHotspotIdx(bs._hsNormalId, bs._hsPressedId);
		if (record == nullptr)
			continue;

		Common::Rect buttonRect = (this->*getRectFunc)(feature, bsIdx, bs, record->_drawnRect);
		if (!buttonRect.contains(absPos))
			continue;

		bs.press(_vm, feature, _currentFrameCounter);
		return ZmbEventHandleResult::kConsumed;
	}

	return ZmbEventHandleResult::kPassthrough;
}

void ZoombiniPage::ButtonState::press(MohawkEngine_Zoombini *vm, ZmbFeature *feature, uint32 frameCounter) {
	if (!feature) {
		error("page: button press requires a feature");
		return;
	}
	animate(frameCounter);
	feature->requestVisualRematerialization();
	if (_pressSoundId.hasId())
		vm->_sound->playSound(_pressSoundId, Audio::Mixer::kSFXSoundType, false);
}

void ZoombiniPage::ToggleButtonState::press(MohawkEngine_Zoombini *vm, ZmbFeature *feature, uint32 frameCounter) {
	if (!feature) {
		error("page: toggle-button press requires a feature");
		return;
	}
	animate(frameCounter);
	feature->requestVisualRematerialization();
	if (_pressSoundId.hasId())
		vm->_sound->playSound(_pressSoundId, Audio::Mixer::kSFXSoundType, false);
}

} // End of namespace Mohawk
