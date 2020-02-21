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

AudioProcess *AudioProcess::_theAudioProcess = 0;

AudioProcess::AudioProcess(void) : paused(0) {
	_theAudioProcess = this;
	_type = 1; // persistent
}

AudioProcess::~AudioProcess(void) {
	_theAudioProcess = 0;
}

bool AudioProcess::calculateSoundVolume(ObjId _objId, int16 &_lVol, int16 &_rVol) const {
	Item *item = getItem(_objId);
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

	_lVol = (dist * lbal) / 160;
	_rVol = (dist * rbal) / 160;

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
	int _channel = playSample(sample, 200, 0);
	if (_channel == -1)
		return false;

	si._channel = _channel;
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
		int16 _sfxNum = ids->read2();
		int16 _priority = ids->read2();
		int16 _objId = ids->read2();
		int16 _loops = ids->read2();
		uint32 _pitchShift = ids->read4();
		uint16 _volume = ids->read2();

		if (_sfxNum != -1) { // SFX
			int16 _lVol = 0;
			int16 _rVol = 0;
			if (_objId != 0) {
				_lVol = 256;
				_rVol = 256;
			}
			playSFX(_sfxNum, _priority, _objId, _loops, false, _pitchShift, _volume, _lVol, _rVol);
		} else {                // Speech
			uint32 slen = ids->read4();

			char *buf = new char[slen + 1];
			ids->read(buf, slen);
			buf[slen] = 0;
			Std::string text = buf;
			delete[] buf;

			playSpeech(text, _priority, _objId, _pitchShift, _volume);
		}
	}

	return true;
}

int AudioProcess::playSample(AudioSample *sample, int _priority, int _loops, uint32 _pitchShift, int16 _lVol, int16 _rVol) {
	AudioMixer *mixer = AudioMixer::get_instance();
	int _channel = mixer->playSample(sample, _loops, _priority, false, _pitchShift, _lVol, _rVol);

	if (_channel == -1) return _channel;

	// Erase old sample using _channel (if any)
	Std::list<SampleInfo>::iterator it;
	for (it = _sampleInfo.begin(); it != _sampleInfo.end();) {
		if (it->_channel == _channel) {
			it = _sampleInfo.erase(it);
		} else {
			++it;
		}
	}

	return _channel;
}

void AudioProcess::playSFX(int _sfxNum, int _priority, ObjId _objId, int _loops,
                           bool no_duplicates, uint32 _pitchShift, uint16 _volume,
                           int16 _lVol, int16 _rVol) {
	//con->Printf("playSFX(%i, %i, 0x%X, %i)\n", _sfxNum, _priority, _objId, _loops);

	SoundFlex *soundflx = GameData::get_instance()->getSoundFlex();

	AudioMixer *mixer = AudioMixer::get_instance();

	if (no_duplicates) {
		Std::list<SampleInfo>::iterator it;
		for (it = _sampleInfo.begin(); it != _sampleInfo.end();) {
			if (it->_sfxNum == _sfxNum && it->_objId == _objId &&
			        it->_loops == _loops) {

				// Exactly the same (and playing) so just return
				//if (it->_priority == _priority)
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

	AudioSample *sample = soundflx->getSample(_sfxNum);
	if (!sample) return;

	if (_lVol == -1 || _rVol == -1) {
		_lVol = 256;
		_rVol = 256;
		if (_objId) calculateSoundVolume(_objId, _lVol, _rVol);
	}

	int _channel = playSample(sample, _priority, _loops, _pitchShift, (_lVol * _volume) / 256, (_rVol * _volume) / 256);
	if (_channel == -1) return;

	// Update list
	_sampleInfo.push_back(SampleInfo(_sfxNum, _priority, _objId, _loops, _channel, _pitchShift, _volume, _lVol, _rVol));
}

void AudioProcess::stopSFX(int _sfxNum, ObjId _objId) {
	//con->Printf("stopSFX(%i, 0x%X)\n", _sfxNum, _objId);

	AudioMixer *mixer = AudioMixer::get_instance();

	Std::list<SampleInfo>::iterator it;
	for (it = _sampleInfo.begin(); it != _sampleInfo.end();) {
		if (it->_sfxNum == _sfxNum && it->_objId == _objId) {
			if (mixer->isPlaying(it->_channel)) mixer->stopSample(it->_channel);
			it = _sampleInfo.erase(it);
		} else {
			++it;
		}
	}
}

bool AudioProcess::isSFXPlaying(int _sfxNum) {
	//con->Printf("isSFXPlaying(%i)\n", _sfxNum);

	Std::list<SampleInfo>::iterator it;
	for (it = _sampleInfo.begin(); it != _sampleInfo.end(); ++it) {
		if (it->_sfxNum == _sfxNum)
			return true;
	}

	return false;
}

void AudioProcess::setVolumeSFX(int _sfxNum, uint8 _volume) {
	//con->Printf("setVolumeSFX(%i, %i)\n", _sfxNum, _volume);
	AudioMixer *mixer = AudioMixer::get_instance();

	Std::list<SampleInfo>::iterator it;
	for (it = _sampleInfo.begin(); it != _sampleInfo.end(); ++it) {
		if (it->_sfxNum == _sfxNum && it->_sfxNum != -1) {
			it->_volume = _volume;

			int _lVol = 256, _rVol = 256;
			if (it->_objId) calculateSoundVolume(it->_objId, it->_lVol, it->_rVol);
			mixer->setVolume(it->_channel, (_lVol * it->_volume) / 256, (_rVol * it->_volume) / 256);
		}
	}
}

//
// Speech
//

bool AudioProcess::playSpeech(Std::string &_barked, int shapenum, ObjId _objId, uint32 _pitchShift, uint16 _volume) {
	SpeechFlex *speechflex = GameData::get_instance()->getSpeechFlex(shapenum);

	if (!speechflex) return false;

	AudioMixer *mixer = AudioMixer::get_instance();

	Std::list<SampleInfo>::iterator it;
	for (it = _sampleInfo.begin(); it != _sampleInfo.end();) {

		if (it->_sfxNum == -1 && it->_barked == _barked &&
		        it->_priority == shapenum && it->_objId == _objId) {

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
	int index = speechflex->getIndexForPhrase(_barked, speech_start, speech_end);
	if (!index) return false;

	AudioSample *sample = speechflex->getSample(index);
	if (!sample) return false;

	int _channel = playSample(sample, 200, 0, _pitchShift, _volume, _volume);

	if (_channel == -1) return false;

	// Update list
	_sampleInfo.push_back(SampleInfo(_barked, shapenum, _objId, _channel,
	                                 speech_start, speech_end, _pitchShift, _volume, 256, 256));

	return true;
}

uint32 AudioProcess::getSpeechLength(Std::string &_barked, int shapenum) const {
	SpeechFlex *speechflex = GameData::get_instance()->getSpeechFlex(shapenum);
	if (!speechflex) return 0;

	return speechflex->getSpeechLength(_barked);
}


void AudioProcess::stopSpeech(Std::string &_barked, int shapenum, ObjId _objId) {
	AudioMixer *mixer = AudioMixer::get_instance();

	Std::list<SampleInfo>::iterator it;
	for (it = _sampleInfo.begin(); it != _sampleInfo.end();) {
		if (it->_sfxNum == -1 && it->_priority == shapenum &&
		        it->_objId == _objId && it->_barked == _barked) {
			if (mixer->isPlaying(it->_channel)) mixer->stopSample(it->_channel);
			it = _sampleInfo.erase(it);
		} else {
			++it;
		}
	}
}

bool AudioProcess::isSpeechPlaying(Std::string &_barked, int shapenum) {
	Std::list<SampleInfo>::iterator it;
	for (it = _sampleInfo.begin(); it != _sampleInfo.end(); ++it) {
		if (it->_sfxNum == -1 && it->_priority == shapenum &&
		        it->_barked == _barked) {
			return true;
		}
	}

	return false;
}

void AudioProcess::pauseAllSamples() {
	paused++;
	if (paused != 1) return;

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
	paused--;
	if (paused != 0) return;

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

	ObjId _objId = 0;
	if (argsize == 6) {
		ARG_OBJID(objid_);
		_objId = objid_;
	}

	AudioProcess *ap = AudioProcess::get_instance();
	if (ap) ap->playSFX(_sfxNum, _priority, _objId, 0);
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

	ObjId _objId = 0;
	if (argsize == 6) {
		ARG_OBJID(objid_);
		_objId = objid_;
	}

//	con->Printf("playAmbientSFX(%i, %i, 0x%X)\n", _sfxNum, _priority, objID);
	AudioProcess *ap = AudioProcess::get_instance();
	if (ap) ap->playSFX(_sfxNum, _priority, _objId, -1, true);
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

	ObjId _objId = 0;
	if (argsize == 4) {
		ARG_OBJID(objid_);
		_objId = objid_;
	}

	AudioProcess *ap = AudioProcess::get_instance();
	if (ap) ap->stopSFX(_sfxNum, _objId);
	else perr << "Error: No AudioProcess" << Std::endl;

	return 0;
}

} // End of namespace Ultima8
} // End of namespace Ultima
