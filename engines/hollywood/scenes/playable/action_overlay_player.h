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

#include "hollywood/scenes/playable/action_overlay.h"
#include "hollywood/scenes/playable/animation_layers.h"

namespace Hollywood {

// Tracks a temporary scene overlay and its active-actor visibility rule.
class ActionOverlayPlayer {
public:
	ActionOverlayPlayer();

	void reset();
	bool applyActorVisibility(ActionOverlayActorVisibility actorVisibility);
	void begin(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize);
	void setFrame(uint frame);
	void finish(bool previousHideActiveActor);

	bool visible;
	ResourceSpriteLayer layer;
	byte chunkIndex;
	byte descriptorCount;
	byte frameIndex;
	bool hideActiveActor;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_ACTION_OVERLAY_PLAYER_H
