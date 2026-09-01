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

namespace Hollywood {

class HollywoodEngine;

// Owns the display and skip lifecycle shared by non-interactive full-screen scenes.
class PresentationScene {
protected:
	// Keeps fixed-duration cutscenes responsive while leaving frame updates to the scene.
	// Each successful beginFrame() must be followed by finishFrame().
	class TimedPresentationLoop {
	public:
		TimedPresentationLoop(PresentationScene &scene, uint32 durationMillis,
			uint32 maximumSliceMillis = 10);

		bool beginFrame();
		uint32 finishFrame();

	private:
		PresentationScene &_scene;
		uint32 _durationMillis;
		uint32 _maximumSliceMillis;
		uint32 _elapsedMillis;
		uint32 _sliceMillis;
	};

	PresentationScene(HollywoodEngine *vm, const char *debugName,
		uint32 sceneFramebufferSize = kFrameBufferSize,
		uint32 savedFramebufferSize = kFrameBufferSize);
	virtual ~PresentationScene() {}

	virtual void stopAudio() {}
	virtual void drawFrameOverlays() {}
	virtual uint presentRowOffset() const;
	virtual uint presentXOffset() const;

	void presentFrame();
	void presentFrame(uint rowOffset, uint xOffset);
	bool pollEvents();
	bool delay(uint32 millis);

	bool revealSavedFramebufferWithCurtain(byte bandWidth = 0x14);
	bool clearSceneFramebufferWithCurtain(byte bandWidth = 0x14);
	void revealSavedFramebufferBand(uint sweepOffset, byte bandWidth, uint xOffset = 0);
	void clearSceneFramebufferBand(uint sweepOffset, byte bandWidth);
	void copySavedFramebufferRun(int y, int x, int width);
	void clearSceneFramebufferRun(int y, int x, int width);

	enum {
		kFrameBufferSize = 0x78000
	};

	HollywoodEngine *_vm;
	const char *_debugName;
	Common::Array<byte> _paletteCurrent;
	IndexedSurfaceBuffer _sceneFramebuffer;
	IndexedSurfaceBuffer _savedFramebuffer;
	Graphics::ManagedSurface _screen;
	Palette6Bit _displayPalette;
	bool _skipRequested;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PRESENTATION_SCENE_H
