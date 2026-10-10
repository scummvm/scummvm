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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE3000_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE3000_H

#include "hollywood/scenes/chapter_intro_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene3000 : public ChapterIntroScene {
public:
	Scene3000(HollywoodEngine *vm);

private:
	const char *resourceArchiveName() const override;
	const char *musicArchiveName() const override;
	uint16 musicCueId() const override;
	uint16 nextState() const override;
	byte activeAudioChapterIndex() const override;
	uint sceneArenaFirstChunk() const override;
	uint sceneArenaLastChunk() const override;
	uint16 sceneViewportXOffset() const override;
	void drawInitialFrame() override;
	void runPresentation() override;

	void drawPresentationFrame(bool largeDirty, bool smallDirty, int previousClipFrame);

	byte _largeFrame;
	byte _smallFrame;
	byte _clipFrame;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE3000_H
