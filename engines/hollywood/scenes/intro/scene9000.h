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

#include "hollywood/music.h"
#include "hollywood/resource.h"
#include "hollywood/scenes/presentation_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene9000 : public PresentationScene {
public:
	Scene9000(HollywoodEngine *vm);

	bool play();

private:
	bool load();
	bool loadChunk(uint chunkIndex);
	bool runChunk();
	void stopAudio() override;

	void resetChunkState();
	void drawAnimationFrame(uint16 descriptorIndex);

	enum {
		kIntroChunkCount = 4,
		kIntroFrameDescriptorCount = 21,
		kAnimatedPaletteByteCount = 0xff * 3,
		kFrameStepMillis = 50,
		kPaletteStepMillis = 50,
		kHoldStepMillis = 1000,
		kHoldStepCount = 4
	};

	MusicPlayer _music;
	ResourceChunkTable _chunkTable;
	Common::Array<byte> _paletteSource;
	Common::Array<byte> _resourceArena;
	IndexedSurfaceBuffer _frameDecodeBuffer;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_INTRO_SCENE9000_H
