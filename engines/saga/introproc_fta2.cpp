/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// "Faery Tale Adventure II: Halls of the Dead" Intro sequence scene procedures

#include "saga/saga.h"
#include "saga/scene.h"

#include "sound/mixer.h"
#include "graphics/video/smk_player.h"

namespace Saga {

class MoviePlayerSMK : Graphics::SMKPlayer {
protected:
	virtual void setPalette(byte *pal);
public:
	MoviePlayerSMK(Audio::Mixer *mixer);
	virtual ~MoviePlayerSMK(void);
};

MoviePlayerSMK::MoviePlayerSMK(Audio::Mixer *mixer)
	: SMKPlayer(mixer) {
	debug(0, "Creating SMK cutscene player");
}

MoviePlayerSMK::~MoviePlayerSMK(void) {
	closeFile();
}

void MoviePlayerSMK::setPalette(byte *pal) {
	// TODO	
}

int Scene::FTA2StartProc() {
	// STUB

	MoviePlayerSMK *smkPlayer = new MoviePlayerSMK(_vm->_mixer);
	
	delete smkPlayer;

	_vm->quitGame();

	return !SUCCESS;
}

} // End of namespace Saga
