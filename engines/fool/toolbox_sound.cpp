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

#include "audio/audiostream.h"
#include "audio/softsynth/pcspk.h"
#include "common/system.h"
#include "common/savefile.h"

#include "gui/filebrowser-dialog.h"

#include "fool/fool.h"
#include "fool/toolbox.h"

namespace Fool {

// FIXME: This sound code is incredibly basic, only providing
// an interface for the square wave synthesiser.

void Toolbox::StartSound(SynthPtr synthRec, uint32 numBytes, ProcPtr completionRtn) {
	if (!synthRec) {
		warning("Toolbox::startSound: record not found");
		return;
	}
	bool synch = completionRtn == -1;
	if (synthRec->mode == swMode) {
		SWSynthRec *rec = static_cast<SWSynthRec *>(synthRec);
		for (auto &it : rec->triplets) {
			_swSynth->playQueue(Audio::PCSpeaker::kWaveFormSquare, it.count == 0 ? 783360.0f : (783360.0f / it.count), it.duration*1000000/60, MIN(it.amplitude, (uint16)255));
		}
		if (synch) {
			while (_swSynth->isPlaying()) {
				Delay(1);
			}
		}
	} else {
		warning("Toolbox::startSound: unimplemented synth mode %d", synthRec->mode);
		return;
	}
}

void Toolbox::StopSound() {
	_swSynth->stop();
}

bool Toolbox::SoundDone() {
	return !_swSynth->isPlaying();
}

void Toolbox::SetSoundVol(int16 level) {
	warning("STUB: Toolbox::SetSoundVol");
}

int16 Toolbox::GetSoundVol() {
	warning("STUB: Toolbox::GetSoundVol");
	return 7;
}


} // namespace Fool
