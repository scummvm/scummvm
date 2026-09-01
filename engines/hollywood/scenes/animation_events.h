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

#ifndef HOLLYWOOD_SCENES_ANIMATION_EVENTS_H
#define HOLLYWOOD_SCENES_ANIMATION_EVENTS_H

#include "common/array.h"
#include "common/types.h"

namespace Hollywood {

// Ordered actions dispatched after an animation installs the selected frame.
// Referenced maps and commit targets must remain valid during synchronous playback.
struct AnimationFrameEvent {
	enum Type {
		kFramebufferPatch,
		kResourcePatch,
		kSound,
		kLoopingSound,
		kResidentSound,
		kStopSound,
		kSecondarySpeech,
		kStartedSecondarySpeech,
		kPrimarySpeech,
		kActorPose,
		kActorPath,
		kLayerFrame,
		kLayerFrameMap,
		kLayerReset,
		kLayerVisibility,
		kStateCommit,
		kInvalidatePalette,
		kFadeFromBlack,
		kFadeToBlack,
		kCustomHook
	};
	// Sound events target the main effect player unless an ambient slot is selected.
	enum {
		kMainSoundSlot = 0xff
	};

	typedef void (*CommitFunction)(void *target, uint32 value);

	AnimationFrameEvent(int newFrame, Type newType) :
			frame(newFrame),
			everyFrame(false),
			type(newType),
			selector(0),
			resourceChunk(0),
			soundId(0),
			soundVolumePercent(100),
			soundSlot(kMainSoundSlot),
			speechRow(0),
			speechFrame(0),
			speechId(0),
			speechCenterX(0),
			speechTopY(0),
			speechRed(0),
			speechGreen(0),
			speechBlue(0),
			actorX(0),
			actorY(0),
			actorFacing(0),
			actorCel(0),
			actorPathFrameMillis(0),
			layerId(0),
			layerFrame(0),
			layerFrameMap(nullptr),
			layerFrameMapSize(0),
			layerFrameMapFirstFrame(0),
			showMappedLayer(false),
			layerVisible(false),
			commitTarget(nullptr),
			commitValue(0),
			commitFunction(nullptr),
			hookId(0) {
	}

	bool matches(uint currentFrame) const {
		return everyFrame || (frame >= 0 && (uint)frame == currentFrame);
	}

	int frame;
	bool everyFrame;
	Type type;
	byte selector;
	uint resourceChunk;
	uint16 soundId;
	byte soundVolumePercent;
	byte soundSlot;
	uint16 speechRow;
	byte speechFrame;
	byte speechId;
	uint16 speechCenterX;
	uint16 speechTopY;
	byte speechRed;
	byte speechGreen;
	byte speechBlue;
	int actorX;
	int actorY;
	byte actorFacing;
	byte actorCel;
	uint32 actorPathFrameMillis;
	uint layerId;
	byte layerFrame;
	const byte *layerFrameMap;
	uint layerFrameMapSize;
	int layerFrameMapFirstFrame;
	bool showMappedLayer;
	bool layerVisible;
	void *commitTarget;
	uint32 commitValue;
	CommitFunction commitFunction;
	byte hookId;
};

class AnimationFrameEvents {
public:
	void addFramebufferPatch(int frame, byte selector) {
		AnimationFrameEvent event(frame, AnimationFrameEvent::kFramebufferPatch);
		event.selector = selector;
		_events.push_back(event);
	}

	void addResourcePatch(int frame, uint chunkIndex) {
		AnimationFrameEvent event(frame, AnimationFrameEvent::kResourcePatch);
		event.resourceChunk = chunkIndex;
		_events.push_back(event);
	}

	void addSound(int frame, uint16 soundId, byte volumePercent, bool looping,
			byte slotIndex = AnimationFrameEvent::kMainSoundSlot) {
		AnimationFrameEvent event(frame, looping ? AnimationFrameEvent::kLoopingSound : AnimationFrameEvent::kSound);
		event.soundId = soundId;
		event.soundVolumePercent = volumePercent;
		event.soundSlot = slotIndex;
		_events.push_back(event);
	}

	void addStopSound(int frame) {
		_events.push_back(AnimationFrameEvent(frame, AnimationFrameEvent::kStopSound));
	}

	void addResidentSound(int frame, byte soundId, byte volumePercent) {
		AnimationFrameEvent event(frame, AnimationFrameEvent::kResidentSound);
		event.soundId = soundId;
		event.soundVolumePercent = volumePercent;
		_events.push_back(event);
	}

	void addSecondarySpeech(int frame, uint16 rowIndex, byte frameIndex, byte speechId) {
		AnimationFrameEvent event(frame, AnimationFrameEvent::kSecondarySpeech);
		event.speechRow = rowIndex;
		event.speechFrame = frameIndex;
		event.speechId = speechId;
		_events.push_back(event);
	}

	void addStartedSecondarySpeech(int frame, uint16 rowIndex, byte frameIndex) {
		AnimationFrameEvent event(frame, AnimationFrameEvent::kStartedSecondarySpeech);
		event.speechRow = rowIndex;
		event.speechFrame = frameIndex;
		_events.push_back(event);
	}

	void addPrimarySpeech(int frame, uint16 rowIndex, byte frameIndex,
			uint16 centerX, uint16 topY, byte red, byte green, byte blue) {
		AnimationFrameEvent event(frame, AnimationFrameEvent::kPrimarySpeech);
		event.speechRow = rowIndex;
		event.speechFrame = frameIndex;
		event.speechCenterX = centerX;
		event.speechTopY = topY;
		event.speechRed = red;
		event.speechGreen = green;
		event.speechBlue = blue;
		_events.push_back(event);
	}

	void addActorPose(int frame, int x, int y, byte facing, byte cel) {
		AnimationFrameEvent event(frame, AnimationFrameEvent::kActorPose);
		event.actorX = x;
		event.actorY = y;
		event.actorFacing = facing;
		event.actorCel = cel;
		_events.push_back(event);
	}

	void addActorPath(int frame, int targetX, int targetY, byte finalFacing,
			byte finalCel, uint32 frameMillis) {
		AnimationFrameEvent event(frame, AnimationFrameEvent::kActorPath);
		event.actorX = targetX;
		event.actorY = targetY;
		event.actorFacing = finalFacing;
		event.actorCel = finalCel;
		event.actorPathFrameMillis = frameMillis;
		_events.push_back(event);
	}

	void addLayerFrame(int frame, uint layerId, byte layerFrame) {
		AnimationFrameEvent event(frame, AnimationFrameEvent::kLayerFrame);
		event.layerId = layerId;
		event.layerFrame = layerFrame;
		_events.push_back(event);
	}

	void addLayerFrameMap(uint layerId, const byte *mappedFrames, uint mappedFrameCount,
			int firstPlaybackFrame, bool showLayer) {
		AnimationFrameEvent event(-1, AnimationFrameEvent::kLayerFrameMap);
		event.everyFrame = true;
		event.layerId = layerId;
		event.layerFrameMap = mappedFrames;
		event.layerFrameMapSize = mappedFrameCount;
		event.layerFrameMapFirstFrame = firstPlaybackFrame;
		event.showMappedLayer = showLayer;
		_events.push_back(event);
	}

	void addLayerVisibility(int frame, uint layerId, bool visible) {
		AnimationFrameEvent event(frame, AnimationFrameEvent::kLayerVisibility);
		event.layerId = layerId;
		event.layerVisible = visible;
		_events.push_back(event);
	}

	void addLayerReset(int frame, uint layerId, byte layerFrame) {
		AnimationFrameEvent event(frame, AnimationFrameEvent::kLayerReset);
		event.layerId = layerId;
		event.layerFrame = layerFrame;
		_events.push_back(event);
	}

	template<class T, class V>
	void addStateCommit(int frame, T &target, const V &value) {
		static_assert(sizeof(T) <= sizeof(uint32), "Animation frame state value is too large");
		AnimationFrameEvent event(frame, AnimationFrameEvent::kStateCommit);
		event.commitTarget = &target;
		event.commitValue = static_cast<uint32>(static_cast<T>(value));
		event.commitFunction = &commit<T>;
		_events.push_back(event);
	}

	void addPaletteInvalidation(int frame) {
		_events.push_back(AnimationFrameEvent(frame, AnimationFrameEvent::kInvalidatePalette));
	}

	void addPaletteFade(int frame, bool fromBlack) {
		_events.push_back(AnimationFrameEvent(frame, fromBlack ?
			AnimationFrameEvent::kFadeFromBlack : AnimationFrameEvent::kFadeToBlack));
	}

	void addCustomHook(int frame, byte hookId, bool everyFrame = false) {
		if (hookId == 0)
			return;
		AnimationFrameEvent event(frame, AnimationFrameEvent::kCustomHook);
		event.everyFrame = everyFrame;
		event.hookId = hookId;
		_events.push_back(event);
	}

	const Common::Array<AnimationFrameEvent> &entries() const {
		return _events;
	}

private:
	template<class T>
	static void commit(void *target, uint32 value) {
		*static_cast<T *>(target) = static_cast<T>(value);
	}

	Common::Array<AnimationFrameEvent> _events;
};

// Fluent frame events shared by resource overlays and layer-frame ranges.
template<class Spec>
class AnimationEventSpec {
public:
	Spec &patchAt(int frame, byte selector) {
		events.addFramebufferPatch(frame, selector);
		return self();
	}

	Spec &resourcePatchAt(int frame, uint resourceChunkIndex) {
		events.addResourcePatch(frame, resourceChunkIndex);
		return self();
	}

	Spec &soundAt(int frame, uint16 soundId, byte volumePercent = 100) {
		events.addSound(frame, soundId, volumePercent, false);
		return self();
	}

	Spec &loopingSoundAt(int frame, uint16 soundId, byte volumePercent = 100) {
		events.addSound(frame, soundId, volumePercent, true);
		return self();
	}

	Spec &stopSoundAt(int frame) {
		events.addStopSound(frame);
		return self();
	}

	Spec &ambientSoundAt(int frame, uint16 soundId, byte volumePercent = 100,
			byte slotIndex = 0) {
		events.addSound(frame, soundId, volumePercent, false, slotIndex);
		return self();
	}

	Spec &loopingAmbientSoundAt(int frame, uint16 soundId, byte volumePercent = 100,
			byte slotIndex = 0) {
		events.addSound(frame, soundId, volumePercent, true, slotIndex);
		return self();
	}

	Spec &residentSoundAt(int frame, byte soundId, byte volumePercent = 100) {
		events.addResidentSound(frame, soundId, volumePercent);
		return self();
	}

	Spec &secondarySpeechAt(int frame, uint16 rowIndex, byte frameIndex,
			byte speechId = 0) {
		events.addSecondarySpeech(frame, rowIndex, frameIndex, speechId);
		return self();
	}

	Spec &startSecondarySpeechAt(int frame, uint16 rowIndex, byte frameIndex) {
		events.addStartedSecondarySpeech(frame, rowIndex, frameIndex);
		return self();
	}

	Spec &primarySpeechAt(int frame, uint16 rowIndex, byte frameIndex,
			uint16 centerX, uint16 topY, byte red, byte green, byte blue) {
		events.addPrimarySpeech(frame, rowIndex, frameIndex, centerX, topY, red, green, blue);
		return self();
	}

	Spec &actorPoseAt(int frame, int x, int y, byte facing, byte cel = 0) {
		events.addActorPose(frame, x, y, facing, cel);
		return self();
	}

	// Starts a path that advances alongside the blocking animation.
	Spec &actorPathAt(int frame, int targetX, int targetY, byte finalFacing,
			byte finalCel, uint32 frameMillis) {
		events.addActorPath(frame, targetX, targetY, finalFacing, finalCel, frameMillis);
		return self();
	}

	Spec &layerFrameAt(int frame, uint layerId, byte layerFrame) {
		events.addLayerFrame(frame, layerId, layerFrame);
		return self();
	}

	Spec &mappedLayerFrames(uint layerId, const byte *mappedFrames,
			uint mappedFrameCount, int firstPlaybackFrame = 0) {
		events.addLayerFrameMap(layerId, mappedFrames, mappedFrameCount,
			firstPlaybackFrame, false);
		return self();
	}

	Spec &visibleMappedLayerFrames(uint layerId, const byte *mappedFrames,
			uint mappedFrameCount, int firstPlaybackFrame = 0) {
		events.addLayerFrameMap(layerId, mappedFrames, mappedFrameCount,
			firstPlaybackFrame, true);
		return self();
	}

	Spec &layerVisibleAt(int frame, uint layerId, bool visible) {
		events.addLayerVisibility(frame, layerId, visible);
		return self();
	}

	Spec &layerResetAt(int frame, uint layerId, byte layerFrame) {
		events.addLayerReset(frame, layerId, layerFrame);
		return self();
	}

	template<class T, class V>
	Spec &commitAt(int frame, T &target, const V &value) {
		events.addStateCommit(frame, target, value);
		return self();
	}

	Spec &invalidatePaletteAt(int frame) {
		events.addPaletteInvalidation(frame);
		return self();
	}

	Spec &fadeFromBlackAt(int frame) {
		events.addPaletteFade(frame, true);
		return self();
	}

	Spec &fadeToBlackAt(int frame) {
		events.addPaletteFade(frame, false);
		return self();
	}

	Spec &hookAt(int frame, byte hookId) {
		events.addCustomHook(frame, hookId);
		return self();
	}

	Spec &hookEveryFrame(byte hookId) {
		events.addCustomHook(-1, hookId, true);
		return self();
	}

	AnimationFrameEvents events;

private:
	Spec &self() {
		return static_cast<Spec &>(*this);
	}
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_ANIMATION_EVENTS_H
