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
#include "ultima/ultima8/usecode/uc_machine.h"
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
#include "ultima/ultima8/kernel/core_app.h"
#include "common/util.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_class stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(AudioProcess)

AudioProcess *AudioProcess::_theAudioProcess = nullptr;
const uint32 AudioProcess::PITCH_SHIFT_NONE = 0x10000;

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

	// Clip to expected range of 0-255
	lVol = CLIP(lVol, (int16)0, (int16)255);
	rVol = CLIP(rVol, (int16)0, (int16)255);

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

		if (it->_loops == -1) {
			// check if an ever-looping sfx for an item has left the
			// fast area.. if so we are "finished".
			Item *item = getItem(it->_objId);
			if (item && !item->hasFlags(Item::FLG_FASTAREA) && mixer->isPlaying(it->_channel)) {
				finished = true;
				mixer->stopSample(it->_channel);
			}
		}

		if (finished)
			it = _sampleInfo.erase(it);
		else {
			if (it->_sfxNum != -1 && it->_objId) {
				it->_lVol = 255;
				it->_rVol = 255;
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


void AudioProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeByte(static_cast<uint8>(_sampleInfo.size()));

	Std::list<SampleInfo>::iterator it;
	for (it = _sampleInfo.begin(); it != _sampleInfo.end(); ++it) {
		ws->writeUint16LE(it->_sfxNum);
		ws->writeUint16LE(it->_priority);
		ws->writeUint16LE(it->_objId);
		ws->writeUint16LE(it->_loops);
		ws->writeUint32LE(it->_pitchShift);
		ws->writeUint16LE(it->_volume);

		if (it->_sfxNum == -1) { // Speech
			ws->writeUint32LE(static_cast<uint32>(it->_barked.size()));
			ws->write(it->_barked.c_str(), static_cast<uint32>(it->_barked.size()));
		}
	}
}

bool AudioProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	uint32 count = rs->readByte();

	while (count--) {
		int16 sfxNum = rs->readUint16LE();
		int16 priority = rs->readUint16LE();
		int16 objId = rs->readUint16LE();
		int16 loops = rs->readUint16LE();
		uint32 pitchShift = rs->readUint32LE();
		uint16 volume = rs->readUint16LE();

		if (sfxNum != -1) { // SFX
			int16 lVol = 0;
			int16 rVol = 0;
			if (objId != 0) {
				lVol = 255;
				rVol = 255;
			}
			// Note: Small inconsistency for backward compatibility - reload ambient sounds as non-ambient.
			playSFX(sfxNum, priority, objId, loops, false, pitchShift, volume, lVol, rVol, false);
		} else {                // Speech
			uint32 slen = rs->readUint32LE();

			char *buf = new char[slen + 1];
			rs->read(buf, slen);
			buf[slen] = 0;
			Std::string text = buf;
			delete[] buf;

			playSpeech(text, priority, objId, pitchShift, volume);
		}
	}

	return true;
}

int AudioProcess::playSample(AudioSample *sample, int priority, int loops, uint32 pitchShift, int16 lVol, int16 rVol, bool ambient) {
	AudioMixer *mixer = AudioMixer::get_instance();
	int channel = mixer->playSample(sample, loops, priority, false, pitchShift, lVol, rVol, ambient);

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
                           int16 lVol, int16 rVol, bool ambient) {

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
					pout << "Sound " << sfxNum << " already playing on obj " << objId << Std::endl;
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
		lVol = 255;
		rVol = 255;
		if (objId) calculateSoundVolume(objId, lVol, rVol);
	}

	int channel = playSample(sample, priority, loops, pitchShift, (lVol * volume) / 256, (rVol * volume) / 256, ambient);
	if (channel == -1) return;

	// Update list
	_sampleInfo.push_back(SampleInfo(sfxNum, priority, objId, loops, channel, pitchShift, volume, lVol, rVol, ambient));
}

void AudioProcess::stopSFX(int sfxNum, ObjId objId) {
	AudioMixer *mixer = AudioMixer::get_instance();

	Std::list<SampleInfo>::iterator it;
	for (it = _sampleInfo.begin(); it != _sampleInfo.end();) {
		if ((sfxNum == -1 || it->_sfxNum == sfxNum)
			 && it->_objId == objId) {
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

bool AudioProcess::isSFXPlayingForObject(int sfxNum, ObjId objId) {
	Std::list<SampleInfo>::iterator it;
	for (it = _sampleInfo.begin(); it != _sampleInfo.end(); ++it) {
		if (it->_sfxNum == sfxNum && (objId == it->_objId))
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
	                                 speech_start, speech_end, pitchShift, volume, 256, 256, false));

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
	ARG_SINT16(sfxNum);

	int16 priority = 0x60;
	if (argsize >= 4) {
		ARG_SINT16(priority_);
		priority = priority_;
	}

	ObjId objId = 0;
	if (argsize == 6) {
		ARG_OBJID(objectId);
		objId = objectId;
	}

	AudioProcess *ap = AudioProcess::get_instance();
	if (ap) ap->playSFX(sfxNum, priority, objId, 0);
	else perr << "Error: No AudioProcess" << Std::endl;

	return 0;
}

uint32 AudioProcess::I_playAmbientSFX(const uint8 *args, unsigned int argsize) {
	ARG_SINT16(sfxNum);

	int16 priority = 0x60;
	if (argsize >= 4) {
		ARG_SINT16(priority_);
		priority = priority_;
	}

	ObjId objId = 0;
	if (argsize == 6) {
		ARG_OBJID(objectId);
		objId = objectId;
	}

	AudioProcess *ap = AudioProcess::get_instance();
	if (ap) ap->playSFX(sfxNum, priority, objId, -1, true);
	else perr << "Error: No AudioProcess" << Std::endl;

	return 0;
}

uint32 AudioProcess::I_playSFXCru(const uint8 *args, unsigned int argsize) {
	ARG_ITEM_FROM_PTR(item)
	ARG_SINT16(sfxNum);

	if (!item) {
		warning("I_playSFXCru: Couldn't get item %d", id_item);
	} else {
		AudioProcess *ap = AudioProcess::get_instance();
		if (ap) {
			// Crusader stops any existing item sounds before starting the next.
			ap->stopSFX(item->getObjId(), -1);
			ap->playSFX(sfxNum, 0x10, item->getObjId(), 0, true, PITCH_SHIFT_NONE, 0x80, false);
		} else {
			warning("I_playSFXCru Error: No AudioProcess");
		}
	}
	return 0;
}


uint32 AudioProcess::I_playAmbientSFXCru(const uint8 *args, unsigned int argsize) {
	// Similar to I_playAmbientSFX, but the params are different.
	ARG_ITEM_FROM_PTR(item)
	ARG_SINT16(sfxNum);

	if (!item) {
		warning("I_playAmbientSFXCru: Couldn't get item %d", id_item);
	} else {
		AudioProcess *ap = AudioProcess::get_instance();
		if (ap)
			ap->playSFX(sfxNum, 0x10, item->getObjId(), -1, true, PITCH_SHIFT_NONE, 0xff, true);
		else
			warning("I_playAmbientSFXCru Error: No AudioProcess");
	}
	return 0;
}

uint32 AudioProcess::I_isSFXPlaying(const uint8 *args, unsigned int argsize) {
	ARG_SINT16(sfxNum);

	AudioProcess *ap = AudioProcess::get_instance();
	if (ap) return ap->isSFXPlaying(sfxNum);
	else perr << "Error: No AudioProcess" << Std::endl;
	return 0;
}

uint32 AudioProcess::I_isSFXPlayingForObject(const uint8 *args, unsigned int argsize) {
	ARG_ITEM_FROM_PTR(item)
	ARG_SINT16(sfxNum);

	if (!item) {
		warning("I_isSFXPlayingForObject: Couldn't get item");
	} else {
		AudioProcess *ap = AudioProcess::get_instance();
		if (ap)
			return ap->isSFXPlayingForObject(sfxNum, item->getObjId());
		else
			warning("I_isSFXPlayingForObject Error: No AudioProcess");
	}
	return 0;
}

uint32 AudioProcess::I_setVolumeSFX(const uint8 *args, unsigned int /*argsize*/) {
	// Sets volume for last played instances of sfxNum (???)
	ARG_SINT16(sfxNum);
	ARG_UINT8(volume);

	AudioProcess *ap = AudioProcess::get_instance();
	if (ap) ap->setVolumeSFX(sfxNum, volume);
	else perr << "Error: No AudioProcess" << Std::endl;

	return 0;
}

uint32 AudioProcess::I_stopSFX(const uint8 *args, unsigned int argsize) {
	ARG_SINT16(sfxNum);

	ObjId objId = 0;
	if (argsize == 4) {
		ARG_OBJID(objectId);
		objId = objectId;
	}

	AudioProcess *ap = AudioProcess::get_instance();
	if (ap) ap->stopSFX(sfxNum, objId);
	else perr << "Error: No AudioProcess" << Std::endl;

	return 0;
}

uint32 AudioProcess::I_stopSFXCru(const uint8 *args, unsigned int argsize) {
	int16 sfxNum = -1;
	ARG_ITEM_FROM_PTR(item);

	if (!item) {
		perr << "Invalid item in I_stopSFXCru";
		return 0;
	}

	if (argsize == 6) {
		ARG_SINT16(sfxNumber);
		sfxNum = sfxNumber;
	}

	AudioProcess *ap = AudioProcess::get_instance();
	if (ap) ap->stopSFX(sfxNum, item->getObjId());
	else perr << "Error: No AudioProcess" << Std::endl;

	return 0;
}

uint32 AudioProcess::I_stopAllSFX(const uint8 * /*args*/, unsigned int /*argsize*/) {
	AudioProcess *ap = AudioProcess::get_instance();
	// Not *exactly* the same, but close enough for this intrinsic.
	if (ap) ap->stopAllExceptSpeech();
	else perr << "Error: No AudioProcess" << Std::endl;

	return 0;
}

} // End of namespace Ultima8
} // End of namespace Ultima
