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

#include "common/file.h"
#include "common/config-manager.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/wave.h"
#include "audio/decoders/vorbis.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/te/te_core.h"
#include "tetraedge/te/te_sound_manager.h"
#include "tetraedge/te/te_music.h"

namespace Tetraedge {

TeSoundManager::TeSoundManager() {
}

void TeSoundManager::playFreeSound(const Common::Path &path) {
	playFreeSound(path, 1.0f, "sfx");
}

void TeSoundManager::playFreeSound(const Common::Path &path, float vol, const Common::String &channel) {
	TeCore *core = g_engine->getCore();
	TetraedgeFSNode sndNode = core->findFile(path);

	if (!sndNode.isReadable()) {
		warning("TeSoundManager::playFreeSound: couldn't open %s", sndNode.toString().c_str());
		return;
	}

	Common::SeekableReadStream *streamfile = sndNode.createReadStream();
	if (!streamfile) {
		warning("TeSoundManager::playFreeSound: couldn't open %s", sndNode.toString().c_str());
		return;
	}

	Common::String fileName = path.baseName();

	Audio::AudioStream *stream;
	if (fileName.contains(".wav"))
		stream = Audio::makeWAVStream(streamfile, DisposeAfterUse::YES);
	else if (fileName.contains(".ogg"))
		stream = Audio::makeVorbisStream(streamfile, DisposeAfterUse::YES);
	else
		error("Unsupported audio file type %s", fileName.c_str());

	byte bvol = round(vol * 255.0);
	int channelId = channel.hash();
	Audio::Mixer *mixer = g_system->getMixer();
	if (!_handles.contains(channel))
		_handles.setVal(channel, Audio::SoundHandle());
	else
		mixer->stopHandle(_handles[channel]);
	mixer->playStream(Audio::Mixer::kMusicSoundType, &_handles[channel], stream, channelId, bvol);
}

void TeSoundManager::stopFreeSound(const Common::String &name) {
	if (!_handles.contains(name))
		return;
	Audio::Mixer *mixer = g_system->getMixer();
	mixer->stopHandle(_handles.getVal(name));
	_handles.erase(name);
}

void TeSoundManager::setChannelVolume(const Common::String &channel, float vol) {
	if (channel == "dialog") {
		ConfMan.setInt("speech_volume", (int)(vol * 255));
	} else if (channel == "music") {
		ConfMan.setInt("music_volume", (int)(vol * 255));
	} else {
		ConfMan.setInt("sfx_volume", (int)(vol * 255));
	}
	g_engine->syncSoundSettings();
}

float TeSoundManager::getChannelVolume(const Common::String &channel) {
	if (channel == "dialog") {
		return ConfMan.getInt("speech_volume") / 255.0f;
	} else if (channel == "music") {
		return ConfMan.getInt("music_volume") / 255.0f;
	} else {
		return ConfMan.getInt("sfx_volume") / 255.0f;
	}
}

void TeSoundManager::update() {
	// Take a copy in case the list changes as we iterate.
	Common::Array<TeMusic *> musics = _musics;
	for (TeMusic *m : musics) {
		m->update();
	}
}


} // end namespace Tetraedge
