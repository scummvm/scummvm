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

#include "common/debug.h"

#include "cryomni3d/atlantis/dialogs_manager.h"
#include "cryomni3d/atlantis/engine.h"

namespace CryOmni3D {
namespace Atlantis {

Atlantis_DialogsManager::Atlantis_DialogsManager(CryOmni3DEngine_Atlantis *engine) :
	_engine(engine) {
}

bool Atlantis_DialogsManager::play(const Common::String &sequence) {
	bool slowStop = false;
	return DialogsManager::play(sequence, slowStop);
}

void Atlantis_DialogsManager::executeShow(const Common::String &show) {
	debugC(2, kDebugScript, "Atlantis_DialogsManager::executeShow: %s", show.c_str());
}

void Atlantis_DialogsManager::playDialog(const Common::String &video,
        const Common::String &sound, const Common::String &text,
        const SubtitlesSettings &settings) {
	debugC(2, kDebugScript, "Atlantis_DialogsManager::playDialog: video=%s sound=%s",
	      video.c_str(), sound.c_str());
}

void Atlantis_DialogsManager::displayMessage(const Common::String &text) {
	debugC(2, kDebugScript, "Atlantis_DialogsManager::displayMessage: %s", text.c_str());
}

uint Atlantis_DialogsManager::askPlayerQuestions(const Common::String &video,
        const Common::StringArray &questions) {
	debugC(2, kDebugScript, "Atlantis_DialogsManager::askPlayerQuestions: video=%s", video.c_str());
	return 0;
}

} // namespace Atlantis
} // namespace CryOmni3D
