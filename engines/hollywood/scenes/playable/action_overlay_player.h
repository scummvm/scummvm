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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_ACTION_OVERLAY_PLAYER_H
#define HOLLYWOOD_SCENES_PLAYABLE_ACTION_OVERLAY_PLAYER_H

#include "common/types.h"

#include "hollywood/scenes/animation_layers.h"

namespace Hollywood {

/**
 * Stores a temporary resource overlay outside the regular scene layer stack.
 *
 * PlayableScene draws it at the selected stratum. Actor-replacement playback
 * hides the active actor until finish() restores the previous state.
 */
class ActionOverlayPlayer {
public:
	ActionOverlayPlayer();

	void reset();
	bool beginActorReplacement(uint newChunkIndex, uint newDescriptorCount,
		const byte *frameMap, uint frameMapSize,
		SceneAnimationStratum drawStratum = kSceneAnimationActorReplacement,
		bool restoreBackground = false) {
		return begin(newChunkIndex, newDescriptorCount, frameMap, frameMapSize,
			drawStratum, true, restoreBackground);
	}
	void setFrame(uint frame);
	void finish(bool previousHideActiveActor);
	bool isVisible() const { return _layer.visible; }
	bool replacesActor() const { return isVisible() && _hideActiveActor; }

	ResourceSpriteLayer _layer;
	SceneAnimationStratum _stratum;
	bool _hideActiveActor;
	bool _restoreBackgroundBeforeDraw;

private:
	friend class PlayableScene;
	bool begin(uint newChunkIndex, uint newDescriptorCount, const byte *frameMap,
		uint frameMapSize, SceneAnimationStratum stratum, bool hideActiveActor,
		bool restoreBackground);
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_ACTION_OVERLAY_PLAYER_H
