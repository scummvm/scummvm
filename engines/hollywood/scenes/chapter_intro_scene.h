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

#ifndef HOLLYWOOD_SCENES_CHAPTER_INTRO_SCENE_H
#define HOLLYWOOD_SCENES_CHAPTER_INTRO_SCENE_H

#include "common/array.h"

#include "hollywood/graphics.h"
#include "hollywood/music.h"
#include "hollywood/resource.h"

namespace Hollywood {

class HollywoodEngine;

class ChapterIntroScene {
public:
	virtual ~ChapterIntroScene() {}

	bool play();

protected:
	ChapterIntroScene(HollywoodEngine *vm, const char *debugName);

	virtual const char *resourceArchiveName() const = 0;
	virtual const char *musicArchiveName() const = 0;
	virtual uint16 musicCueId() const = 0;
	virtual uint16 nextState() const = 0;
	virtual byte activeAudioChapterIndex() const = 0;
	virtual uint sceneArenaFirstChunk() const = 0;
	virtual uint sceneArenaLastChunk() const = 0;
	virtual uint16 sceneViewportXOffset() const;
	virtual void initializeChapterState();
	virtual void adjustPaletteAfterLoad();
	virtual void drawInitialFrame();
	virtual void runPresentation() = 0;

	bool load();
	bool loadChunk(uint index, Common::Array<byte> &destination, uint fixedSize);
	bool loadChunk(uint index, IndexedSurfaceBuffer &destination, uint fixedSize);
	bool loadArenaChunk(uint index);
	void fadeInPalette();
	void fadeOutPalette();
	void presentFrame();
	void rotatePaletteRange(uint firstIndex, uint lastIndex);
	bool delay(uint32 millis);
	bool pollEvents();
	void drawClipFrameDelta(uint chunkIndex, uint tableEntryCount, byte frameIndex);

	enum {
		kResourceChunkCount = 40,
		kFrameBufferSize = 0x78000
	};

	HollywoodEngine *_vm;
	MusicPlayer _music;
	ResourceChunkTable _chunkTable;
	uint32 _resourceChunkOffsets[kResourceChunkCount];
	uint32 _resourceArenaCursor;
	Common::Array<byte> _paletteResource;
	Common::Array<byte> _paletteCurrent;
	IndexedSurfaceBuffer _baseFramebuffer;
	IndexedSurfaceBuffer _sceneFramebuffer;
	Common::Array<byte> _resourceArena;
	Graphics::ManagedSurface _screen;
	Palette6Bit _displayPalette;
	const char *_debugName;
	bool _skipRequested;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_CHAPTER_INTRO_SCENE_H
