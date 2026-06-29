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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE7040_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE7040_H

#include "hollywood/scenes/playable/sue_playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene7040 : public SuePlayableScene {
public:
	Scene7040(HollywoodEngine *vm);

private:
	const char *resourceArchiveName() const override;
	uint sceneInitialRequiredChunkCount() const override;
	uint sceneArenaFirstChunk() const override;
	uint sceneArenaLastChunk() const override;
	uint sceneStageIndex() const override;
	const char *sceneDebugName() const override;
	uint16 sceneViewportXOffset() const override;
	bool shouldLoadPaletteAfterFrankensteinNote() const override;
	bool shouldConvertSavedFramebufferFF() const override;
	bool shouldRunExitSideEffectsAfterLoop() const override;
	bool usesG04PathRouteSpecialCase() const override;
	bool isMainFlowStateInScene(uint16 stateId) const override;
	bool hasCustomEntrySequence() const override;
	void runCustomEntrySequence() override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;

	void runJosephGuestListGreeting();
	void waitPreItemIdleSequence();
	bool shouldStopJosephGuestListGreeting();
};

} // End of namespace Hollywood

#endif
