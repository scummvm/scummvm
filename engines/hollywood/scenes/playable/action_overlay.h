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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_ACTION_OVERLAY_H
#define HOLLYWOOD_SCENES_PLAYABLE_ACTION_OVERLAY_H

#include "common/types.h"

namespace Hollywood {

// How a temporary overlay sequence affects the main actor while it is playing.
enum ActionOverlayActorVisibility {
	kActionOverlayKeepActiveActorVisibility,
	kActionOverlayShowActiveActor,
	kActionOverlayHideActiveActor
};

// Optional behavior for action overlays: clipping, state patches, sounds, hooks.
struct ActionOverlayOptions {
	ActionOverlayOptions() :
		firstFrame(0),
		endFrame(0),
		actorVisibility(kActionOverlayKeepActiveActorVisibility),
		redrawAtEnd(true),
		statePatchFrame(-1),
		statePatchSelector(0),
		soundFrame(-1),
		soundId(0),
		soundVolumePercent(100),
		hookFrame(-1),
		hookId(0) {
	}

	uint firstFrame;
	uint endFrame;
	ActionOverlayActorVisibility actorVisibility;
	bool redrawAtEnd;
	int statePatchFrame;
	byte statePatchSelector;
	int soundFrame;
	byte soundId;
	byte soundVolumePercent;
	int hookFrame;
	byte hookId;
};

} // End of namespace Hollywood

#endif
