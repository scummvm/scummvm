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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE2000_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE2000_H

#include "common/random.h"

#include "hollywood/music.h"
#include "hollywood/scenes/chapter_intro_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene2000 : public ChapterIntroScene {
public:
	Scene2000(HollywoodEngine *vm);
	~Scene2000() override;

private:
	const char *resourceArchiveName() const override;
	const char *musicArchiveName() const override;
	uint16 musicCueId() const override;
	uint16 nextState() const override;
	byte activeAudioChapterIndex() const override;
	uint sceneArenaFirstChunk() const override;
	uint sceneArenaLastChunk() const override;
	void adjustPaletteAfterLoad() override;
	void drawInitialFrame() override;
	void runPresentation() override;

	void advanceSmallSprites();
	void drawPresentationFrame(byte previousClipMapIndex);

	Common::RandomSource _random;
	SoundBank0Player _presentationSound;
	byte _spriteStates[6];
	int8 _spriteDeltas[6];
	byte _clipMapIndex;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE2000_H
