/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "common/mutex.h"
#include "common/textconsole.h"
#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "audio/decoders/mp3.h"
#include "engines/grim/resource.h"
#include "engines/grim/emisound/mp3track.h"

namespace Grim {

void MP3Track::parseRIFFHeader(Common::SeekableReadStream *data) {
	uint32 tag = data->readUint32BE();
	if (tag == MKTAG('R','I','F','F')) {
		_endFlag = false;
		data->seek(18, SEEK_CUR);
		_channels = data->readByte();
		data->readByte();
		_freq = data->readUint32LE();
		data->seek(6, SEEK_CUR);
		_bits = data->readByte();
		data->seek(5, SEEK_CUR);
		_regionLength = data->readUint32LE();
		_headerSize = 44;
	} else {
		error("Unknown file header");
	}
}

MP3Track::MP3Track(Audio::Mixer::SoundType soundType) {
	_soundType = soundType;
}

MP3Track::~MP3Track() {
	stop();
}
	
bool MP3Track::openSound(Common::String soundName, Common::SeekableReadStream *file) {
#ifndef USE_MAD
	return false;
#else
	if (!file) {
		warning("Stream for %s not open", soundName.c_str());
		//return false;
	}
	_soundName = soundName;
	parseRIFFHeader(file);
	_stream = Audio::makeLoopingAudioStream(Audio::makeMP3Stream(file, DisposeAfterUse::YES), 0);
	_handle = new Audio::SoundHandle();
	return true;
#endif
}

} // end of namespace Grim 