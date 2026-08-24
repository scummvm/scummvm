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

#ifndef HOLLYWOOD_SCENES_INTRO_SCENE9000_H
#define HOLLYWOOD_SCENES_INTRO_SCENE9000_H

#include "common/array.h"

#include "hollywood/graphics.h"
#include "hollywood/music.h"
#include "hollywood/resource.h"

namespace Hollywood {

class HollywoodEngine;

class Scene9000 {
public:
	Scene9000(HollywoodEngine *vm);

	bool play();

private:
	bool load();
	bool loadChunk(uint chunkIndex);
	bool runChunk();
	bool pollEvents();
	bool delay(uint32 millis);

	void resetChunkState();
	void presentFrame();

	void drawStripSpriteFrame(uint16 descriptorIndex);
	void restoreSpriteBackground(uint16 descriptorIndex);

	uint16 readUint16(uint offset) const;
	uint32 readUint32(uint offset) const;

	enum {
		kIntroChunkCount = 4,
		kIntroFrameDescriptorCount = 21,
		kIntroFrameDescriptorSize = 14,
		kAnimatedPaletteByteCount = 0xff * 3,
		kFrameStepMillis = 50,
		kPaletteStepMillis = 50,
		kHoldStepMillis = 1000,
		kHoldStepCount = 4
	};

	HollywoodEngine *_vm;
	MusicPlayer _music;
	ResourceChunkTable _chunkTable;
	Common::Array<byte> _paletteSource;
	Common::Array<byte> _paletteCurrent;
	Common::Array<byte> _resourceArena;
	IndexedSurfaceBuffer _frameDecodeBuffer;
	IndexedSurfaceBuffer _sceneFramebuffer;
	Graphics::ManagedSurface _screen;
	Palette6Bit _displayPalette;
	bool _skipRequested;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_INTRO_SCENE9000_H
