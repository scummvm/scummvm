/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/mutex.h"
#include "common/textconsole.h"
#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "engines/grim/resource.h"
#include "engines/grim/emi/sound/codecs/scx.h"
#include "engines/grim/emi/sound/scxtrack.h"

namespace Grim {

SCXTrack::SCXTrack(Audio::Mixer::SoundType soundType) {
	_soundType = soundType;
}

SCXTrack::~SCXTrack() {
	stop();
}

bool SCXTrack::openSound(const Common::String &soundName, Common::SeekableReadStream *file, const Audio::Timestamp *start) {
	_soundName = soundName;
	Audio::RewindableAudioStream *scxStream = makeSCXStream(file, DisposeAfterUse::YES);
	if (_soundType == Audio::Mixer::kMusicSoundType)
		_stream = Audio::makeLoopingAudioStream(scxStream, 0);
	else
		_stream = scxStream;
	_handle = new Audio::SoundHandle();
	return true;
}

bool SCXTrack::isPlaying() {
	if (!_handle)
		return false;

	return g_system->getMixer()->isSoundHandleActive(*_handle);
}

} // end of namespace Grim
