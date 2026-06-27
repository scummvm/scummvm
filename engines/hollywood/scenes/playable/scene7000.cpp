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

#include "hollywood/scenes/playable/scene7000.h"

#include "common/debug.h"
#include "common/path.h"

#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kChapter7MusicArchiveName = "RESOURCE.M07";
const uint16 kScene7000MusicCueId = 0x000c;
const byte kScene7000MusicVolumePercent = 10;

Scene7000::Scene7000(HollywoodEngine *vm) :
		_vm(vm) {
}

bool Scene7000::play() {
	GameplayState &state = _vm->gameState();
	state.initializeForState7000();

	MusicPlayer *music = _vm->gameplayMusic();
	music->setArchive(Common::Path(kChapter7MusicArchiveName));
	music->playMusicCue(kScene7000MusicCueId, kScene7000MusicVolumePercent);

	debugC(1, kDebugScene, "Scene 7000 initialized gameplay bootstrap; next state=0x%04x", state.mainFlowStateId);
	return true;
}

} // End of namespace Hollywood
