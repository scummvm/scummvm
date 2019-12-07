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

#include "ultima8/misc/pent_include.h"
#include "ultima8/audio/audio_process.h"
#include "ultima8/usecode/intrinsics.h"
#include "ultima8/kernel/object.h"

#include "ultima8/games/game_data.h"
#include "ultima8/audio/sound_flex.h"
#include "ultima8/audio/speech_flex.h"
#include "ultima8/audio/audio_sample.h"
#include "ultima8/audio/audio_mixer.h"

#include "ultima8/world/get_object.h"
#include "ultima8/world/item.h"
#include "ultima8/world/camera_process.h"

#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"

namespace Ultima8 {

using Pentagram::AudioSample;
using Pentagram::AudioMixer;

// p_dynamic_class stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(AudioProcess, Process)

AudioProcess *AudioProcess::the_audio_process = 0;

AudioProcess::AudioProcess(void) : paused(0) {
	the_audio_process = this;
	type = 1; // persistent
}

AudioProcess::~AudioProcess(void) {
	the_audio_process = 0;
}

bool AudioProcess::calculateSoundVolume(ObjId objid, int16 &lvol, int16 &rvol) const {
	Item *item = getItem(objid);
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

	lvol = (dist * lbal) / 160;
	rvol = (dist * rbal) / 160;

	return true;
}

void AudioProcess::run() {
	AudioMixer *mixer = AudioMixer::get_instance();

	// Update the channels
	std::list<SampleInfo>::iterator it;
	for (it = sample_info.begin(); it != sample_info.end();) {
		bool finished = false;
		if (!mixer->isPlaying(it->channel)) {
			if (it->sfxnum == -1)
				finished = !continueSpeech(*it);
			else
				finished = true;
		}

		if (finished)
			it = sample_info.erase(it);
		else {

			if (it->sfxnum != -1 && it->objid) {
				it->lvol = 256;
				it->rvol = 256;
				calculateSoundVolume(it->objid, it->lvol, it->rvol);
			}
			mixer->setVolume(it->channel, (it->lvol * it->volume) / 256, (it->rvol * it->volume) / 256);

			++it;
		}
	}
}

bool AudioProcess::continueSpeech(SampleInfo &si) {
	assert(si.sfxnum == -1);

	SpeechFlex *speechflex;
	speechflex = GameData::get_instance()->getSpeechFlex(si.priority);
	if (!speechflex) return false;

	if (si.curspeech_end >= si.barked.size()) return false;

	si.curspeech_start = si.curspeech_end;
	int index = speechflex->getIndexForPhrase(si.barked,
	            si.curspeech_start,
	            si.curspeech_end);
	if (!index) return false;

	AudioSample *sample = speechflex->getSample(index);
	if (!sample) return false;

	// hack to prevent playSample from deleting 'si'
	si.channel = -1;
	int channel = playSample(sample, 200, 0);
	if (channel == -1)
		return false;

	si.channel = channel;
	return true;
}


void AudioProcess::saveData(ODataSource *ods) {
	Process::saveData(ods);

	ods->write1(static_cast<uint8>(sample_info.size()));

	std::list<SampleInfo>::iterator it;
	for (it = sample_info.begin(); it != sample_info.end(); ++it) {
		ods->write2(it->sfxnum);
		ods->write2(it->priority);
		ods->write2(it->objid);
		ods->write2(it->loops);
		ods->write4(it->pitch_shift);
		ods->write2(it->volume);

		if (it->sfxnum == -1) { // Speech
			ods->write4(static_cast<uint32>(it->barked.size()));
			ods->write(it->barked.c_str(), static_cast<uint32>(it->barked.size()));
		}
	}
}

bool AudioProcess::loadData(IDataSource *ids, uint32 version) {
	if (!Process::loadData(ids, version)) return false;

	uint32 count = ids->read1();

	while (count--) {
		int16 sfxnum = ids->read2();
		int16 priority = ids->read2();
		int16 objid = ids->read2();
		int16 loops = ids->read2();
		uint32 pitch_shift = ids->read4();
		uint16 volume = ids->read2();

		if (sfxnum != -1) { // SFX
			int16 lvol = 0;
			int16 rvol = 0;
			if (objid != 0) {
				lvol = 256;
				rvol = 256;
			}
			playSFX(sfxnum, priority, objid, loops, false, pitch_shift, volume, lvol, rvol);
		} else {                // Speech
			uint32 slen = ids->read4();

			char *buf = new char[slen + 1];
			ids->read(buf, slen);
			buf[slen] = 0;
			std::string text = buf;
			delete[] buf;

			playSpeech(text, priority, objid, pitch_shift, volume);
		}
	}

	return true;
}

int AudioProcess::playSample(AudioSample *sample, int priority, int loops, uint32 pitch_shift, int16 lvol, int16 rvol) {
	AudioMixer *mixer = AudioMixer::get_instance();
	int channel = mixer->playSample(sample, loops, priority, false, pitch_shift, lvol, rvol);

	if (channel == -1) return channel;

	// Erase old sample using channel (if any)
	std::list<SampleInfo>::iterator it;
	for (it = sample_info.begin(); it != sample_info.end();) {
		if (it->channel == channel) {
			it = sample_info.erase(it);
		} else {
			++it;
		}
	}

	return channel;
}

void AudioProcess::playSFX(int sfxnum, int priority, ObjId objid, int loops,
                           bool no_duplicates, uint32 pitch_shift, uint16 volume,
                           int16 lvol, int16 rvol) {
	//con->Printf("playSFX(%i, %i, 0x%X, %i)\n", sfxnum, priority, objid, loops);

	SoundFlex *soundflx = GameData::get_instance()->getSoundFlex();

	AudioMixer *mixer = AudioMixer::get_instance();

	if (no_duplicates) {
		std::list<SampleInfo>::iterator it;
		for (it = sample_info.begin(); it != sample_info.end();) {
			if (it->sfxnum == sfxnum && it->objid == objid &&
			        it->loops == loops) {

				// Exactly the same (and playing) so just return
				//if (it->priority == priority)
				if (mixer->isPlaying(it->channel)) {
					pout << "Sound already playing" << std::endl;
					return;
				} else {
					it = sample_info.erase(it);
					continue;
				}
			}

			++it;
		}
	}

	AudioSample *sample = soundflx->getSample(sfxnum);
	if (!sample) return;

	if (lvol == -1 || rvol == -1) {
		lvol = 256;
		rvol = 256;
		if (objid) calculateSoundVolume(objid, lvol, rvol);
	}

	int channel = playSample(sample, priority, loops, pitch_shift, (lvol * volume) / 256, (rvol * volume) / 256);
	if (channel == -1) return;

	// Update list
	sample_info.push_back(SampleInfo(sfxnum, priority, objid, loops, channel, pitch_shift, volume, lvol, rvol));
}

void AudioProcess::stopSFX(int sfxnum, ObjId objid) {
	//con->Printf("stopSFX(%i, 0x%X)\n", sfxnum, objid);

	AudioMixer *mixer = AudioMixer::get_instance();

	std::list<SampleInfo>::iterator it;
	for (it = sample_info.begin(); it != sample_info.end();) {
		if (it->sfxnum == sfxnum && it->objid == objid) {
			if (mixer->isPlaying(it->channel)) mixer->stopSample(it->channel);
			it = sample_info.erase(it);
		} else {
			++it;
		}
	}
}

bool AudioProcess::isSFXPlaying(int sfxnum) {
	//con->Printf("isSFXPlaying(%i)\n", sfxnum);

	std::list<SampleInfo>::iterator it;
	for (it = sample_info.begin(); it != sample_info.end(); ++it) {
		if (it->sfxnum == sfxnum)
			return true;
	}

	return false;
}

void AudioProcess::setVolumeSFX(int sfxnum, uint8 volume) {
	//con->Printf("setVolumeSFX(%i, %i)\n", sfxnum, volume);
	AudioMixer *mixer = AudioMixer::get_instance();

	std::list<SampleInfo>::iterator it;
	for (it = sample_info.begin(); it != sample_info.end(); ++it) {
		if (it->sfxnum == sfxnum && it->sfxnum != -1) {
			it->volume = volume;

			int lvol = 256, rvol = 256;
			if (it->objid) calculateSoundVolume(it->objid, it->lvol, it->rvol);
			mixer->setVolume(it->channel, (lvol * it->volume) / 256, (rvol * it->volume) / 256);
		}
	}
}

//
// Speech
//

bool AudioProcess::playSpeech(std::string &barked, int shapenum, ObjId objid, uint32 pitch_shift, uint16 volume) {
	SpeechFlex *speechflex = GameData::get_instance()->getSpeechFlex(shapenum);

	if (!speechflex) return false;

	AudioMixer *mixer = AudioMixer::get_instance();

	std::list<SampleInfo>::iterator it;
	for (it = sample_info.begin(); it != sample_info.end();) {

		if (it->sfxnum == -1 && it->barked == barked &&
		        it->priority == shapenum && it->objid == objid) {

			if (mixer->isPlaying(it->channel)) {
				pout << "Speech already playing" << std::endl;
				return true;
			} else {
				it = sample_info.erase(it);
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

	int channel = playSample(sample, 200, 0, pitch_shift, volume, volume);

	if (channel == -1) return false;

	// Update list
	sample_info.push_back(SampleInfo(barked, shapenum, objid, channel,
	                                 speech_start, speech_end, pitch_shift, volume, 256, 256));

	return true;
}

uint32 AudioProcess::getSpeechLength(std::string &barked, int shapenum) const {
	SpeechFlex *speechflex = GameData::get_instance()->getSpeechFlex(shapenum);
	if (!speechflex) return 0;

	return speechflex->getSpeechLength(barked);
}


void AudioProcess::stopSpeech(std::string &barked, int shapenum, ObjId objid) {
	AudioMixer *mixer = AudioMixer::get_instance();

	std::list<SampleInfo>::iterator it;
	for (it = sample_info.begin(); it != sample_info.end();) {
		if (it->sfxnum == -1 && it->priority == shapenum &&
		        it->objid == objid && it->barked == barked) {
			if (mixer->isPlaying(it->channel)) mixer->stopSample(it->channel);
			it = sample_info.erase(it);
		} else {
			++it;
		}
	}
}

bool AudioProcess::isSpeechPlaying(std::string &barked, int shapenum) {
	std::list<SampleInfo>::iterator it;
	for (it = sample_info.begin(); it != sample_info.end(); ++it) {
		if (it->sfxnum == -1 && it->priority == shapenum &&
		        it->barked == barked) {
			return true;
		}
	}

	return false;
}

void AudioProcess::pauseAllSamples() {
	paused++;
	if (paused != 1) return;

	AudioMixer *mixer = AudioMixer::get_instance();

	std::list<SampleInfo>::iterator it;
	for (it = sample_info.begin(); it != sample_info.end();) {
		if (mixer->isPlaying(it->channel)) {
			mixer->setPaused(it->channel, true);
			++it;
		} else {
			it = sample_info.erase(it);
		}

	}

}

void AudioProcess::unpauseAllSamples() {
	paused--;
	if (paused != 0) return;

	AudioMixer *mixer = AudioMixer::get_instance();

	std::list<SampleInfo>::iterator it;
	for (it = sample_info.begin(); it != sample_info.end();) {
		if (mixer->isPlaying(it->channel)) {
			mixer->setPaused(it->channel, false);
			++it;
		} else {
			it = sample_info.erase(it);
		}

	}

}

void AudioProcess::stopAllExceptSpeech() {
	AudioMixer *mixer = AudioMixer::get_instance();

	std::list<SampleInfo>::iterator it;
	for (it = sample_info.begin(); it != sample_info.end();) {
		if (it->barked.empty()) {
			if (mixer->isPlaying(it->channel)) mixer->stopSample(it->channel);
			it = sample_info.erase(it);
		} else {
			++it;
		}
	}
}

//
// Intrinsics
//

uint32 AudioProcess::I_playSFX(const uint8 *args, unsigned int argsize) {
	ARG_SINT16(sfxnum);

	int16 priority = 0x60;
	if (argsize >= 4) {
		ARG_SINT16(priority_);
		priority = priority_;
	}

	ObjId objid = 0;
	if (argsize == 6) {
		ARG_OBJID(objid_);
		objid = objid_;
	}

	AudioProcess *ap = AudioProcess::get_instance();
	if (ap) ap->playSFX(sfxnum, priority, objid, 0);
	else perr << "Error: No AudioProcess" << std::endl;

	return 0;
}

uint32 AudioProcess::I_playAmbientSFX(const uint8 *args, unsigned int argsize) {
	ARG_SINT16(sfxnum);

	int16 priority = 0x60;
	if (argsize >= 4) {
		ARG_SINT16(priority_);
		priority = priority_;
	}

	ObjId objid = 0;
	if (argsize == 6) {
		ARG_OBJID(objid_);
		objid = objid_;
	}

//	con->Printf("playAmbientSFX(%i, %i, 0x%X)\n", sfxnum, priority, objID);
	AudioProcess *ap = AudioProcess::get_instance();
	if (ap) ap->playSFX(sfxnum, priority, objid, -1, true);
	else perr << "Error: No AudioProcess" << std::endl;

	return 0;
}

uint32 AudioProcess::I_isSFXPlaying(const uint8 *args, unsigned int argsize) {
	ARG_SINT16(sfxnum);

	AudioProcess *ap = AudioProcess::get_instance();
	if (ap) return ap->isSFXPlaying(sfxnum);
	else perr << "Error: No AudioProcess" << std::endl;
	return 0;
}

uint32 AudioProcess::I_setVolumeSFX(const uint8 *args, unsigned int /*argsize*/) {
	// Sets volume for last played instances of sfxnum (???)
	ARG_SINT16(sfxnum);
	ARG_UINT8(volume);

	AudioProcess *ap = AudioProcess::get_instance();
	if (ap) ap->setVolumeSFX(sfxnum, volume);
	else perr << "Error: No AudioProcess" << std::endl;

	return 0;
}

uint32 AudioProcess::I_stopSFX(const uint8 *args, unsigned int argsize) {
	ARG_SINT16(sfxnum);

	ObjId objid = 0;
	if (argsize == 4) {
		ARG_OBJID(objid_);
		objid = objid_;
	}

	AudioProcess *ap = AudioProcess::get_instance();
	if (ap) ap->stopSFX(sfxnum, objid);
	else perr << "Error: No AudioProcess" << std::endl;

	return 0;
}

// static
void AudioProcess::ConCmd_listSFX(const Console::ArgvType &argv) {
	AudioProcess *ap = AudioProcess::get_instance();
	if (!ap) {
		perr << "Error: No AudioProcess" << std::endl;
		return;
	}

	std::list<SampleInfo>::iterator it;
	for (it = ap->sample_info.begin(); it != ap->sample_info.end(); ++it) {
		pout.Print("Sample: num %d, obj %d, loop %d, prio %d",
		            it->sfxnum, it->objid, it->loops, it->priority);
		if (!it->barked.empty()) {
			pout << ", speech: \"" << it->barked.substr(it->curspeech_start, it->curspeech_end - it->curspeech_start) << "\"";
		}
		pout << std::endl;
	}
}

// static
void AudioProcess::ConCmd_stopSFX(const Console::ArgvType &argv) {
	AudioProcess *ap = AudioProcess::get_instance();
	if (!ap) {
		perr << "Error: No AudioProcess" << std::endl;
		return;
	}

	if (argv.size() < 2) {
		pout << "usage: stopSFX <sfxnum> [<objid>]" << std::endl;
		return;
	}

	int sfxnum = static_cast<int>(strtol(argv[1].c_str(), 0, 0));
	ObjId objid = (argv.size() >= 3) ? static_cast<ObjId>(strtol(argv[2].c_str(), 0, 0)) : 0;

	ap->stopSFX(sfxnum, objid);
}

// static
void AudioProcess::ConCmd_playSFX(const Console::ArgvType &argv) {
	AudioProcess *ap = AudioProcess::get_instance();
	if (!ap) {
		perr << "Error: No AudioProcess" << std::endl;
		return;
	}

	if (argv.size() < 2) {
		pout << "usage: playSFX <sfxnum>" << std::endl;
		return;
	}

	int sfxnum = static_cast<int>(strtol(argv[1].c_str(), 0, 0));

	ap->playSFX(sfxnum, 0x60, 0, 0);
}

} // End of namespace Ultima8
