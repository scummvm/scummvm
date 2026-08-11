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

#include "audio/softsynth/pcspk.h"
#include "glk/pc_speaker.h"

namespace Glk {

PCSpeaker::PCSpeaker() {
	_speaker = new Audio::PCSpeaker();
	_speaker->init();
}

PCSpeaker::~PCSpeaker() {
	delete _speaker;
}

void PCSpeaker::speakerOn(int16 frequency, int32 length) {
	_speaker->play(Audio::PCSpeaker::kWaveFormSquare, frequency, length);
}

void PCSpeaker::speakerOff() {
	_speaker->stop();
}

void PCSpeaker::onUpdate(uint32 millis) {
	if (_speaker->isPlaying())
		_speaker->stop(millis);
}

} // End of namespace Glk
