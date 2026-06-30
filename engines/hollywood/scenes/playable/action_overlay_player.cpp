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

#include "hollywood/scenes/playable/action_overlay_player.h"

namespace Hollywood {

ActionOverlayPlayer::ActionOverlayPlayer() :
		visible(false),
		layer(),
		chunkIndex(0),
		descriptorCount(0),
		frameIndex(0),
		hideActiveActor(false) {
}

void ActionOverlayPlayer::reset() {
	visible = false;
	layer.visible = false;
	chunkIndex = 0;
	descriptorCount = 0;
	frameIndex = 0;
	hideActiveActor = false;
}

bool ActionOverlayPlayer::applyActorVisibility(ActionOverlayActorVisibility actorVisibility) {
	const bool previousHideActiveActor = hideActiveActor;
	if (actorVisibility == kActionOverlayShowActiveActor)
		hideActiveActor = false;
	else if (actorVisibility == kActionOverlayHideActiveActor)
		hideActiveActor = true;

	return previousHideActiveActor;
}

void ActionOverlayPlayer::begin(uint overlayChunkIndex, uint overlayDescriptorCount,
		const byte *frameMap, uint frameMapSize) {
	visible = true;
	chunkIndex = (byte)overlayChunkIndex;
	descriptorCount = (byte)overlayDescriptorCount;
	frameIndex = 0;
	layer.configure(overlayChunkIndex, (uint16)overlayDescriptorCount, frameMap, frameMapSize);
	layer.visible = true;
}

void ActionOverlayPlayer::setFrame(uint frame) {
	layer.setFrame((byte)frame);
	frameIndex = (byte)layer.descriptorIndex();
}

void ActionOverlayPlayer::finish(bool previousHideActiveActor) {
	visible = false;
	layer.visible = false;
	frameIndex = 0;
	hideActiveActor = previousHideActiveActor;
}

} // End of namespace Hollywood
