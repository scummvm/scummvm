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

#ifndef HOLLYWOOD_SCENES_INTRO_SCENE9160_H
#define HOLLYWOOD_SCENES_INTRO_SCENE9160_H

#include "hollywood/music.h"
#include "hollywood/scenes/intro/intro_resource_set.h"
#include "hollywood/scenes/intro/intro_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene9160 : public IntroSceneBase {
public:
	Scene9160(HollywoodEngine *vm);

	bool play();

private:
	bool load();
	bool loadVariableChunk(uint index, Common::Array<byte> &destination);
	bool loadChunk(uint index, Common::Array<byte> &destination, uint fixedSize);
	bool loadArenaChunk(uint index);
	void sanitizePanel(Common::Array<byte> &panel);
	void copyPanelToFramebuffer(const Common::Array<byte> &panel, int yOffset);
	void drawOverlayChunk(uint chunkIndex, int yOffset);
	void buildInitialFrame();
	void prepareNextPair(uint currentChunk);
	void scrollCreditsPanel();
	void fadeInPalette();
	void fadeOutPalette();
	bool waitBeforeScroll();
	bool waitForMusicEnd();
	uint presentRowOffset() const override;
	void stopAudio() override;

	IntroResourceSet _resources;
	MusicPlayer *_music;
	Common::Array<byte> _paletteResource;
	Common::Array<byte> _panelA;
	Common::Array<byte> _panelB;
	uint _rowOffset;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_INTRO_SCENE9160_H
