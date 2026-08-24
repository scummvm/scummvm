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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE6000_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE6000_H

#include "hollywood/scenes/chapter_intro_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene6000 : public ChapterIntroScene {
public:
	Scene6000(HollywoodEngine *vm);

private:
	const char *resourceArchiveName() const override;
	const char *musicArchiveName() const override;
	uint16 musicCueId() const override;
	uint16 nextState() const override;
	byte activeAudioChapterIndex() const override;
	uint sceneArenaFirstChunk() const override;
	uint sceneArenaLastChunk() const override;
	void adjustPaletteAfterLoad() override;
	void runPresentation() override;

	void drawAnimatedSpriteFrame(bool drawSprite);

	byte _spriteFrameIndex;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE6000_H
