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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/usecode/intrinsics.h"
#include "ultima/ultima8/kernel/object.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/audio/sound_flex.h"
#include "ultima/ultima8/audio/speech_flex.h"
#include "ultima/ultima8/audio/audio_sample.h"
#include "ultima/ultima8/audio/audio_mixer.h"
#include "ultima/ultima8/audio/midi_player.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/world/camera_process.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_class stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(AudioProcess, Process)

AudioProcess *AudioProcess::_theAudioProcess = nullptr;

AudioProcess::AudioProcess(void) : _paused(0) {
	_theAudioProcess = this;
	_type = 1; // persistent
}

AudioProcess::~AudioProcess(void) {
	_theAudioProcess = nullptr;
}

bool AudioProcess::calculateSoundVolume(ObjId objId, int16 &lVol, int16 &rVol) const {
	Item *item = getItem(objId);
	if (!item) return false;

	// Need to get items relative coords from avatar

	int32 ax, ay, az, ix, iy, iz;
	CameraProcess::GetCameraLocation(ax, ay, az);
	item->getLocationAbsolute(ix, iy, iz);
	ix -= ax;
	iy -= ay;
	iz -= az;

	//
	// Convert to screenspace
	//
	// Note that this should also correct for Crusader too.
	//

	int x = (ix - iy) / 4;
	int y = (ix + iy) / 8 - iz;

	// Fall off over 350 pixels
	int limit = 350 * 350;
	int dist = limit - (x * x + y * y);
	if (dist < 0) dist = 0;
	dist = (dist * 256) / limit;

	int lbal = 160;
	int rbal = 160;

	if (x < 0) {
		if (x < -160) rbal = 0;
		else rbal = x + 160;
	} else if (x > 0) {
		if (x > 160) lbal = 0;
		else lbal = 160 - x;
	}

	lVol = (dist * lbal) / 160;
	rVol = (dist * rbal) / 160;

	return true;
}

void AudioProcess::run() {
	AudioMixer *mixer = AudioMixer::get_instance();

	// Update the channels
	Std::list<SampleInfo>::iterator it;
	for (it = _sampleInfo.begin(); it != _sampleInfo.end();) {
		bool finished = false;
		if (!mixer->isPlaying(it->_channel)) {
			if (it->_sfxNum == -1)
				finished = !continueSpeech(*it);
			else
				finished = true;
		}

		if (finished)
			it = _sampleInfo.erase(it);
		else {

			if (it->_sfxNum != -1 && it->_objId) {
				it->_lVol = 256;
				it->_rVol = 256;
				calculateSoundVolume(it->_objId, it->_lVol, it->_rVol);
			}
			mixer->setVolume(it->_channel, (it->_lVol * it->_volume) / 256, (it->_rVol * it->_volume) / 256);

			++it;
		}
	}
}

bool AudioProcess::continueSpeech(SampleInfo &si) {
	assert(si._sfxNum == -1);

	SpeechFlex *speechflex;
	speechflex = GameData::get_instance()->getSpeechFlex(si._priority);
	if (!speechflex) return false;

	if (si._curSpeechEnd >= si._barked.size()) return false;

	si._curSpeechStart = si._curSpeechEnd;
	int index = speechflex->getIndexForPhrase(si._barked,
	            si._curSpeechStart,
	            si._curSpeechEnd);
	if (!index) return false;

	AudioSample *sample = speechflex->getSample(index);
	if (!sample) return false;

	// hack to prevent playSample from deleting 'si'
	si._channel = -1;
	int channel = playSample(sample, 200, 0);
	if (channel == -1)
		return false;

	si._channel = channel;
	return true;
}


void AudioProcess::saveData(ODataSource *ods) {
	Process::saveData(ods);

	ods->write1(static_cast<uint8>(_sampleInfo.size()));

	Std::list<SampleInfo>::iterator it;
	for (it = _sampleInfo.begin(); it != _sampleInfo.end(); ++it) {
		ods->write2(it->_sfxNum);
		ods->write2(it->_priority);
		ods->write2(it->_objId);
		ods->write2(it->_loops);
		ods->write4(it->_pitchShift);
		ods->write2(it->_volume);

		if (it->_sfxNum == -1) { // Speech
			ods->write4(static_cast<uint32>(it->_barked.size()));
			ods->write(it->_barked.c_str(), static_cast<uint32>(it->_barked.size()));
		}
	}
}

bool AudioProcess::loadData(IDataSource *ids, uint32 version) {
	if (!Process::loadData(ids, version)) return false;

	uint32 count = ids->read1();

	while (count--) {
		int16 sfxNum = ids->read2();
		int16 priority = ids->read2();
		int16 objId = ids->read2();
		int16 loops = ids->read2();
		uint32 pitchShift = ids->read4();
		uint16 volume = ids->read2();

		if (sfxNum != -1) { // SFX
			int16 lVol = 0;
			int16 rVol = 0;
			if (objId != 0) {
				lVol = 256;
				rVol = 256;
			}
			playSFX(sfxNum, priority, objId, loops, false, pitchShift, volume, lVol, rVol);
		} else {                // Speech
			uint32 slen = ids->read4();

			char *buf = new char[slen + 1];
			ids->read(buf, slen);
			buf[slen] = 0;
			Std::string text = buf;
			delete[] buf;

			playSpeech(text, priority, objId, pitchShift, volume);
		}
	}

	return true;
}

int AudioProcess::playSample(AudioSample *sample, int priority, int loops, uint32 pitchShift, int16 lVol, int16 rVol) {
	AudioMixer *mixer = AudioMixer::get_instance();
	int channel = mixer->playSample(sample, loops, priority, false, pitchShift, lVol, rVol);

	if (channel == -1) return channel;

	// Erase old sample using channel (if any)
	Std::list<SampleInfo>::iterator it;
	for (it = _sampleInfo.begin(); it != _sampleInfo.end();) {
		if (it->_channel == channel) {
			it = _sampleInfo.erase(it);
		} else {
			++it;
		}
	}

	return channel;
}

void AudioProcess::playSFX(int sfxNum, int priority, ObjId objId, int loops,
                           bool no_duplicates, uint32 pitchShift, uint16 volume,
                           int16 lVol, int16 rVol) {

	SoundFlex *soundflx = GameData::get_instance()->getSoundFlex();

	AudioMixer *mixer = AudioMixer::get_instance();

	if (no_duplicates) {
		Std::list<SampleInfo>::iterator it;
		for (it = _sampleInfo.begin(); it != _sampleInfo.end();) {
			if (it->_sfxNum == sfxNum && it->_objId == objId &&
			        it->_loops == loops) {

				// Exactly the same (and playing) so just return
				//if (it->priority == priority)
				if (mixer->isPlaying(it->_channel)) {
					pout << "Sound already playing" << Std::endl;
					return;
				} else {
					it = _sampleInfo.erase(it);
					continue;
				}
			}

			++it;
		}
	}

	AudioSample *sample = soundflx->getSample(sfxNum);
	if (!sample) return;

	if (lVol == -1 || rVol == -1) {
		lVol = 256;
		rVol = 256;
		if (objId) calculateSoundVolume(objId, lVol, rVol);
	}

	int channel = playSample(sample, priority, loops, pitchShift, (lVol * volume) / 256, (rVol * volume) / 256);
	if (channel == -1) return;

	// Update list
	_sampleInfo.push_back(SampleInfo(sfxNum, priority, objId, loops, channel, pitchShift, volume, lVol, rVol));
}

void AudioProcess::stopSFX(int sfxNum, ObjId objId) {
	AudioMixer *mixer = AudioMixer::get_instance();

	Std::list<SampleInfo>::iterator it;
	for (it = _sampleInfo.begin(); it != _sampleInfo.end();) {
		if (it->_sfxNum == sfxNum && it->_objId == objId) {
			if (mixer->isPlaying(it->_channel)) mixer->stopSample(it->_channel);
			it = _sampleInfo.erase(it);
		} else {
			++it;
		}
	}
}

bool AudioProcess::isSFXPlaying(int sfxNum) {
	Std::list<SampleInfo>::iterator it;
	for (it = _sampleInfo.begin(); it != _sampleInfo.end(); ++it) {
		if (it->_sfxNum == sfxNum)
			return true;
	}

	return false;
}

void AudioProcess::setVolumeSFX(int sfxNum, uint8 volume) {
	AudioMixer *mixer = AudioMixer::get_instance();

	Std::list<SampleInfo>::iterator it;
	for (it = _sampleInfo.begin(); it != _sampleInfo.end(); ++it) {
		if (it->_sfxNum == sfxNum && it->_sfxNum != -1) {
			it->_volume = volume;

			int lVol = 256, _rVol = 256;
			if (it->_objId) calculateSoundVolume(it->_objId, it->_lVol, it->_rVol);
			mixer->setVolume(it->_channel, (lVol * it->_volume) / 256, (_rVol * it->_volume) / 256);
		}
	}
}

//
// Speech
//

bool AudioProcess::playSpeech(const Std::string &barked, int shapeNum, ObjId objId, uint32 pitchShift, uint16 volume) {
	SpeechFlex *speechflex = GameData::get_instance()->getSpeechFlex(shapeNum);

	if (!speechflex) return false;

	AudioMixer *mixer = AudioMixer::get_instance();

	Std::list<SampleInfo>::iterator it;
	for (it = _sampleInfo.begin(); it != _sampleInfo.end();) {

		if (it->_sfxNum == -1 && it->_barked == barked &&
		        it->_priority == shapeNum && it->_objId == objId) {

			if (mixer->isPlaying(it->_channel)) {
				pout << "Speech already playing" << Std::endl;
				return true;
			} else {
				it = _sampleInfo.erase(it);
				continue;
			}
		}

		++it;
	}

	uint32 speech_start = 0;
	uint32 speech_end;
	int index = speechflex->getIndexForPhrase(barked, speech_start, speech_end);
	if (!index) return false;

	AudioSample *sample = speechflex->getSample(index);
	if (!sample) return false;

	int channel = playSample(sample, 200, 0, pitchShift, volume, volume);

	if (channel == -1) return false;

	// Update list
	_sampleInfo.push_back(SampleInfo(barked, shapeNum, objId, channel,
	                                 speech_start, speech_end, pitchShift, volume, 256, 256));

	return true;
}

uint32 AudioProcess::getSpeechLength(const Std::string &barked, int shapenum) const {
	SpeechFlex *speechflex = GameData::get_instance()->getSpeechFlex(shapenum);
	if (!speechflex) return 0;

	return speechflex->getSpeechLength(barked);
}


void AudioProcess::stopSpeech(const Std::string &barked, int shapenum, ObjId objId) {
	AudioMixer *mixer = AudioMixer::get_instance();

	Std::list<SampleInfo>::iterator it;
	for (it = _sampleInfo.begin(); it != _sampleInfo.end();) {
		if (it->_sfxNum == -1 && it->_priority == shapenum &&
		        it->_objId == objId && it->_barked == barked) {
			if (mixer->isPlaying(it->_channel)) mixer->stopSample(it->_channel);
			it = _sampleInfo.erase(it);
		} else {
			++it;
		}
	}
}

bool AudioProcess::isSpeechPlaying(const Std::string &barked, int shapeNum) {
	Std::list<SampleInfo>::iterator it;
	for (it = _sampleInfo.begin(); it != _sampleInfo.end(); ++it) {
		if (it->_sfxNum == -1 && it->_priority == shapeNum &&
		        it->_barked == barked) {
			return true;
		}
	}

	return false;
}

void AudioProcess::pauseAllSamples() {
	_paused++;
	if (_paused != 1) return;

	AudioMixer *mixer = AudioMixer::get_instance();

	Std::list<SampleInfo>::iterator it;
	for (it = _sampleInfo.begin(); it != _sampleInfo.end();) {
		if (mixer->isPlaying(it->_channel)) {
			mixer->setPaused(it->_channel, true);
			++it;
		} else {
			it = _sampleInfo.erase(it);
		}

	}

}

void AudioProcess::unpauseAllSamples() {
	_paused--;
	if (_paused != 0) return;

	AudioMixer *mixer = AudioMixer::get_instance();

	Std::list<SampleInfo>::iterator it;
	for (it = _sampleInfo.begin(); it != _sampleInfo.end();) {
		if (mixer->isPlaying(it->_channel)) {
			mixer->setPaused(it->_channel, false);
			++it;
		} else {
			it = _sampleInfo.erase(it);
		}

	}

}

void AudioProcess::stopAllExceptSpeech() {
	AudioMixer *mixer = AudioMixer::get_instance();

	Std::list<SampleInfo>::iterator it;
	for (it = _sampleInfo.begin(); it != _sampleInfo.end();) {
		if (it->_barked.empty()) {
			if (mixer->isPlaying(it->_channel)) mixer->stopSample(it->_channel);
			it = _sampleInfo.erase(it);
		} else {
			++it;
		}
	}
}

//
// Intrinsics
//

uint32 AudioProcess::I_playSFX(const uint8 *args, unsigned int argsize) {
	ARG_SINT16(_sfxNum);

	int16 _priority = 0x60;
	if (argsize >= 4) {
		ARG_SINT16(priority_);
		_priority = priority_;
	}

	ObjId objId = 0;
	if (argsize == 6) {
		ARG_OBJID(objectId);
		objId = objectId;
	}

	AudioProcess *ap = AudioProcess::get_instance();
	if (ap) ap->playSFX(_sfxNum, _priority, objId, 0);
	else perr << "Error: No AudioProcess" << Std::endl;

	return 0;
}

uint32 AudioProcess::I_playAmbientSFX(const uint8 *args, unsigned int argsize) {
	ARG_SINT16(_sfxNum);

	int16 _priority = 0x60;
	if (argsize >= 4) {
		ARG_SINT16(priority_);
		_priority = priority_;
	}

	ObjId objId = 0;
	if (argsize == 6) {
		ARG_OBJID(objectId);
		objId = objectId;
	}

	AudioProcess *ap = AudioProcess::get_instance();
	if (ap) ap->playSFX(_sfxNum, _priority, objId, -1, true);
	else perr << "Error: No AudioProcess" << Std::endl;

	return 0;
}

uint32 AudioProcess::I_isSFXPlaying(const uint8 *args, unsigned int argsize) {
	ARG_SINT16(_sfxNum);

	AudioProcess *ap = AudioProcess::get_instance();
	if (ap) return ap->isSFXPlaying(_sfxNum);
	else perr << "Error: No AudioProcess" << Std::endl;
	return 0;
}

uint32 AudioProcess::I_setVolumeSFX(const uint8 *args, unsigned int /*argsize*/) {
	// Sets _volume for last played instances of _sfxNum (???)
	ARG_SINT16(_sfxNum);
	ARG_UINT8(_volume);

	AudioProcess *ap = AudioProcess::get_instance();
	if (ap) ap->setVolumeSFX(_sfxNum, _volume);
	else perr << "Error: No AudioProcess" << Std::endl;

	return 0;
}

uint32 AudioProcess::I_stopSFX(const uint8 *args, unsigned int argsize) {
	ARG_SINT16(_sfxNum);

	ObjId objId = 0;
	if (argsize == 4) {
		ARG_OBJID(objectId);
		objId = objectId;
	}

	AudioProcess *ap = AudioProcess::get_instance();
	if (ap) ap->stopSFX(_sfxNum, objId);
	else perr << "Error: No AudioProcess" << Std::endl;

	return 0;
}

} // End of namespace Ultima8
} // End of namespace Ultima
