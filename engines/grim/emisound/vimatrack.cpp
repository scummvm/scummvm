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
#include "common/stream.h"
#include "common/mutex.h"
#include "common/textconsole.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"
#include "engines/grim/imuse/imuse_mcmp_mgr.h"
#include "engines/grim/emisound/vimatrack.h"

namespace Grim {

struct Region {
	int32 offset;		// offset of region
	int32 length;		// lenght of region
};

struct SoundDesc {
	uint16 freq;		// frequency
	byte channels;		// stereo or mono
	byte bits;			// 8, 12, 16
	int numRegions;		// number of Regions
	Region *region;
	bool endFlag;
	bool inUse;
	char name[32];
	McmpMgr *mcmpMgr;
	int type;
	int volGroupId;
	bool mcmpData;
	uint32 headerSize;
	Common::SeekableReadStream *inStream;
};
	
bool VimaTrack::isPlaying() {
	// FIXME: Actually clean up the data better 
	// (we don't currently handle the case where it isn't asked for through isPlaying, or deleted explicitly).
	if (_stream->endOfData()) {
		g_system->getMixer()->stopHandle(*_handle);
		return false;
	} else
		return true;
}
	
bool VimaTrack::openSound(Common::String voiceName, Common::SeekableReadStream *file) {
	_soundName = voiceName;
	_mcmp = new McmpMgr();
	_desc = new SoundDesc();
	_desc->inStream = file;
	_desc->mcmpData = true;
	_desc->mcmpMgr = _mcmp;
	int headerSize = 0;

	if(_mcmp->openSound(voiceName.c_str(), file, headerSize)) {
		parseSoundHeader(_desc, headerSize);

		_stream = Audio::makeQueuingAudioStream(_desc->freq, (false));
		g_system->getMixer()->playStream(Audio::Mixer::kMusicSoundType, _handle, _stream);

		playTrack();
		return true;
	} else {
		return false;
	}
}
void VimaTrack::parseSoundHeader(SoundDesc *sound, int &headerSize) {
	Common::SeekableReadStream *data = sound->inStream;
	
	uint32 tag = data->readUint32BE();
	if (tag == MKTAG('R','I','F','F')) {
		sound->endFlag = false;
		sound->region = new Region[1];
		sound->numRegions = 1;
		sound->region[0].offset = 0;
		data->seek(18, SEEK_CUR);
		sound->channels = data->readByte();
		data->readByte();
		sound->freq = data->readUint32LE();
		data->seek(6, SEEK_CUR);
		sound->bits = data->readByte();
		data->seek(5, SEEK_CUR);
		sound->region[0].length = data->readUint32LE();
		headerSize = 44;
	} else {
		assert(tag != MKTAG('i','M','U','S'));
		error("VimaTrack::parseSoundHeader() Unknown sound format");
	}
}

int32 VimaTrack::getDataFromRegion(SoundDesc *sound, int region, byte **buf, int32 offset, int32 size) {
	//assert(checkForProperHandle(sound));
	assert(buf && offset >= 0 && size >= 0);
	assert(region >= 0 && region < sound->numRegions);
	
	int32 region_offset = sound->region[region].offset;
	int32 region_length = sound->region[region].length;
	
	if (offset + size > region_length) {
		size = region_length - offset;
		sound->endFlag = true;
	} else {
		sound->endFlag = false;
	}
	
	if (sound->mcmpData) {
		size = sound->mcmpMgr->decompressSample(region_offset + offset, size, buf);
	} else {
		*buf = (byte *)malloc(sizeof(byte) * size);
		sound->inStream->seek(region_offset + offset + sound->headerSize, SEEK_SET);
		sound->inStream->read(*buf, size);
	}
	
	return size;
}
void VimaTrack::playTrack() {
	//Common::StackLock lock(_mutex);
	assert(_stream);
	byte *data = NULL;
	int32 result = 0;
	
	int32 curRegion = -1;
	int32 regionOffset = 0;
	int32 mixerFlags = Audio::FLAG_16BITS;
	
	curRegion++;
	
	int channels = _desc->channels;
	
	//int32 mixer_size = track->feedSize / _callbackFps;
	int32 mixer_size = _desc->freq * channels * 2;
	
	if (_stream->endOfData()) { // FIXME: Currently we just allocate a bunch here, try to find the correct size instead.
		mixer_size *= 8;
	}
	
	if (channels == 1)
		mixer_size &= ~1;
	if (channels == 2)
		mixer_size &= ~3;
	
	if (mixer_size == 0)
		return;
	
	do {
		result = getDataFromRegion(_desc, curRegion, &data, regionOffset, mixer_size);
		if (channels == 1) {
			result &= ~1;
		}
		if (channels == 2) {
			result &= ~3;
		}
		
		if (result > mixer_size)
			result = mixer_size;
		
		if (g_system->getMixer()->isReady()) {
			_stream->queueBuffer(data, result, DisposeAfterUse::YES, mixerFlags);
			regionOffset += result;
		} else
			delete[] data;
		
		if (curRegion >= 0 && curRegion < _desc->numRegions - 1) {
			curRegion++;
			
			if (!_stream) {
				return;
			}
		} else {
			warning("Out of regions");
			//return;
		}
		mixer_size -= result;
		assert(mixer_size >= 0);
	} while (mixer_size && !_desc->endFlag);
	if (g_system->getMixer()->isReady()) {
		//g_system->getMixer()->setChannelVolume(track->handle, track->getVol());
		//g_system->getMixer()->setChannelBalance(track->handle, track->getPan());
	}
}

VimaTrack::VimaTrack(Common::String soundName) {
	_handle = new Audio::SoundHandle();
	_file = NULL;
	setSoundName(soundName);
}

VimaTrack::~VimaTrack() {
	if (_handle)
		g_system->getMixer()->stopHandle(*_handle);
	delete _mcmp;
	
	if (_desc) {
		delete[] _desc->region;
	}
	
	delete _desc;
}

} // end of namespace Grim 