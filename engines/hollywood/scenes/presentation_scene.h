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

#ifndef HOLLYWOOD_SCENES_PRESENTATION_SCENE_H
#define HOLLYWOOD_SCENES_PRESENTATION_SCENE_H

#include "common/array.h"

#include "hollywood/graphics.h"
#include "hollywood/scenes/animation_channels.h"
#include "hollywood/scenes/animation_player.h"
#include "hollywood/scenes/scene_resources.h"
#include "hollywood/scenes/speech_overlay.h"

namespace Hollywood {

class HollywoodEngine;

// Owns display, resources, timing, and skip plumbing shared by full-screen scenes.
class PresentationScene : public SceneAnimationPlayerDelegate {
protected:
	// Keeps fixed-duration cutscenes responsive while leaving frame updates to the scene.
	// Each successful beginFrame() must be followed by finishFrame().
	class TimedPresentationLoop {
	public:
		enum DurationMode {
			kUntilStopped
		};

		TimedPresentationLoop(PresentationScene &scene, uint32 durationMillis,
			uint32 maximumSliceMillis = 10, bool allowSkip = true);
		TimedPresentationLoop(PresentationScene &scene, DurationMode,
			uint32 maximumSliceMillis = 10, bool allowSkip = true);

		bool beginFrame();
		uint32 finishFrame();

	private:
		PresentationScene &_scene;
		uint32 _durationMillis;
		uint32 _maximumSliceMillis;
		uint32 _elapsedMillis;
		uint32 _sliceMillis;
		bool _allowSkip;
		bool _durationLimited;
	};

	PresentationScene(HollywoodEngine *vm, const char *debugName,
		uint32 sceneFramebufferSize = kSceneBufferByteCount,
		uint32 savedFramebufferSize = kSceneBufferByteCount);
	virtual ~PresentationScene() {}

	virtual void stopAudio() {}
	virtual void drawFrameOverlays();
	virtual uint presentRowOffset() const;
	virtual uint presentXOffset() const;
	virtual int subtitleViewportYOffset() const;
	bool animationPlaybackShouldStop() const override;
	void presentAnimationFrame() override;
	bool waitForAnimationFrame(uint32 millis, bool allowSkip) override;

	bool showAnchoredSubtitle(const Common::String &text, byte colorIndex,
		int centerX, int anchorBottomY,
		SpeechOverlayWrapStyle wrapStyle = kSpeechOverlayAdaptiveWrap);
	bool showAnchoredSubtitle(SpeechOverlay &overlay, const Common::String &text,
		byte colorIndex, int centerX, int anchorBottomY,
		SpeechOverlayWrapStyle wrapStyle = kSpeechOverlayAdaptiveWrap);
	bool showPositionedSubtitle(const Common::String &text, byte colorIndex,
		int centerX, int topY,
		SpeechOverlayWrapStyle wrapStyle = kSpeechOverlayAdaptiveWrap);
	bool showPositionedSubtitle(SpeechOverlay &overlay, const Common::String &text,
		byte colorIndex, int centerX, int topY,
		SpeechOverlayWrapStyle wrapStyle = kSpeechOverlayAdaptiveWrap);
	void clearSubtitle();
	void clearSubtitle(SpeechOverlay &overlay);
	bool loadFixedChunk(uint index, Common::Array<byte> &destination, uint fixedSize);
	bool loadFixedChunk(uint index, IndexedSurfaceBuffer &destination, uint fixedSize);
	bool loadVariableChunk(uint index, Common::Array<byte> &destination);
	bool loadChunkTo(uint index, Common::Array<byte> &destination,
		uint32 destinationOffset);
	bool loadArenaChunk(uint index);
	bool loadArenaChunk(uint archiveIndex, uint localChunkIndex);
	bool loadArenaChunkAlias(uint sourceIndex, uint aliasIndex, uint targetIndex);
	void drawResourceSpriteLayer(const ResourceSpriteLayer &layer);
	void drawLayerStack(SceneAnimationStratum stratum);

	void presentFrame();
	void presentFrame(uint rowOffset, uint xOffset);
	virtual bool pollEvents(bool allowSkip = true);
	bool delay(uint32 millis, bool allowSkip = true);
	bool consumeStepAdvanceRequest();

	bool revealSavedFramebufferWithCurtain(byte bandWidth = 0x14);
	bool clearSceneFramebufferWithCurtain(byte bandWidth = 0x14);
	void revealSavedFramebufferBand(uint sweepOffset, byte bandWidth, uint xOffset = 0);
	void clearSceneFramebufferBand(uint sweepOffset, byte bandWidth);
	void copySavedFramebufferRun(int y, int x, int width);
	void clearSceneFramebufferRun(int y, int x, int width);

	HollywoodEngine *_vm;
	const char *_debugName;
	SceneResources _resources;
	SceneLayerStack _sceneLayers;
	RealtimeAnimationTracks _realtimeAnimationTracks;
	SceneAnimationPlayer _animationPlayer;
	Common::Array<byte> _paletteCurrent;
	IndexedSurfaceBuffer _sceneFramebuffer;
	IndexedSurfaceBuffer _savedFramebuffer;
	Graphics::ManagedSurface _screen;
	Palette6Bit _displayPalette;
	SpeechOverlay _subtitle;
	bool _skipRequested;
	bool _stepAdvanceRequested;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PRESENTATION_SCENE_H
