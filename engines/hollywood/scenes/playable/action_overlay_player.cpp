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
		_layer(),
		_stratum(kSceneAnimationInFrontOfActors),
		_hideActiveActor(false),
		_restoreBackgroundBeforeDraw(false) {
}

void ActionOverlayPlayer::reset() {
	_layer.visible = false;
	_stratum = kSceneAnimationInFrontOfActors;
	_hideActiveActor = false;
	_restoreBackgroundBeforeDraw = false;
}

bool ActionOverlayPlayer::begin(uint overlayChunkIndex, uint overlayDescriptorCount,
		const byte *frameMap, uint frameMapSize, SceneAnimationStratum newStratum,
		bool newHideActiveActor, bool restoreBackground) {
	const bool previousHideActiveActor = _hideActiveActor;
	_stratum = newStratum;
	_layer.configure(overlayChunkIndex, (uint16)overlayDescriptorCount, frameMap, frameMapSize);
	_layer.visible = true;
	_hideActiveActor = newHideActiveActor;
	_restoreBackgroundBeforeDraw = restoreBackground;
	return previousHideActiveActor;
}

void ActionOverlayPlayer::setFrame(uint frame) {
	_layer.setFrame((byte)frame);
}

void ActionOverlayPlayer::finish(bool previousHideActiveActor) {
	_layer.visible = false;
	_stratum = kSceneAnimationInFrontOfActors;
	_hideActiveActor = previousHideActiveActor;
	_restoreBackgroundBeforeDraw = false;
}

} // End of namespace Hollywood
