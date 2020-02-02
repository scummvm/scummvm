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

#ifndef ULTIMA8_AUDIO_AUDIOPROCESS_H
#define ULTIMA8_AUDIO_AUDIOPROCESS_H

#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/usecode/intrinsics.h"
#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/string.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {
namespace Pentagram {
class AudioSample;
}

class AudioProcess :
	public Process {
	struct SampleInfo {
		int32       sfxnum;
		int32       priority;
		ObjId       objid;
		int32       loops;
		int32       channel;
		Std::string barked;
		uint32      curspeech_start, curspeech_end;
		uint32      pitch_shift;    // 0x10000 is normal
		uint16      volume;         // 0-256
		int16       lvol;
		int16       rvol;

		SampleInfo() : sfxnum(-1) { }
		SampleInfo(int32 s, int32 p, ObjId o, int32 l, int32 c, uint32 ps, uint16 v, int16 lv, int16 rv) :
			sfxnum(s), priority(p), objid(o), loops(l), channel(c),
			pitch_shift(ps), volume(v), lvol(lv), rvol(rv) { }
		SampleInfo(Std::string &b, int32 shpnum, ObjId o, int32 c,
		           uint32 s, uint32 e, uint32 ps, uint16 v, int16 lv, int16 rv) :
			sfxnum(-1), priority(shpnum), objid(o), loops(0), channel(c), barked(b),
			curspeech_start(s), curspeech_end(e), pitch_shift(ps), volume(v),
			lvol(lv), rvol(rv) { }
	};

	Std::list<SampleInfo>   sample_info;

public:
	// p_dynamic_class stuff
	ENABLE_RUNTIME_CLASSTYPE()

	AudioProcess(void);
	virtual ~AudioProcess(void);

	//! Get the current instance of the Audio Processes
	static AudioProcess *get_instance() {
		return the_audio_process;
	}

	INTRINSIC(I_playSFX);
	INTRINSIC(I_playAmbientSFX);
	INTRINSIC(I_isSFXPlaying);
	INTRINSIC(I_setVolumeSFX);
	INTRINSIC(I_stopSFX);

	static void ConCmd_listSFX(const Console::ArgvType &argv);
	static void ConCmd_stopSFX(const Console::ArgvType &argv);
	static void ConCmd_playSFX(const Console::ArgvType &argv);


	virtual void run() override;

	void playSFX(int sfxnum, int priority, ObjId objid, int loops,
	             bool no_duplicates, uint32 pitch_shift,
	             uint16 volume, int16 lvol, int16 rvol);

	void playSFX(int sfxnum, int priority, ObjId objid, int loops,
	             bool no_duplicates = false, uint32 pitch_shift = 0x10000,
	             uint16 volume = 0x80) {
		playSFX(sfxnum, priority, objid, loops, no_duplicates, pitch_shift, volume, -1, -1);
	}

	void stopSFX(int sfxnum, ObjId objid);
	bool isSFXPlaying(int sfxnum);
	void setVolumeSFX(int sfxnum, uint8 volume);

	bool playSpeech(Std::string &barked, int shapenum, ObjId objid,
	                uint32 pitch_shift = 0x10000, uint16 volume = 256);
	void stopSpeech(Std::string &barked, int shapenum, ObjId objid);
	bool isSpeechPlaying(Std::string &barked, int shapenum);

	//! get length (in milliseconds) of speech
	uint32 getSpeechLength(Std::string &barked, int shapenum) const;

	//! play a sample (without storing a SampleInfo)
	//! returns channel sample is played on, or -1
	int playSample(Pentagram::AudioSample *sample, int priority, int loops,
	               uint32 pitch_shift = 0x10000, int16 lvol = 256, int16 rvol = 256);

	//! pause all currently playing samples
	void pauseAllSamples();
	//! unpause all currently playing samples
	void unpauseAllSamples();

	//! stop all samples except speech
	void stopAllExceptSpeech();

	// AudioProcess::playSound console command
	//static void ConCmd_playSound(const Console::ArgvType &argv);

	bool loadData(IDataSource *ids, uint32 version);

private:
	virtual void saveData(ODataSource *ods) override;
	uint32 paused;

	//! play the next speech sample for the text in this SampleInfo
	//! note: si is reused if successful
	//! returns true if there was speech left to play, or false if finished
	bool continueSpeech(SampleInfo &si);

	bool calculateSoundVolume(ObjId objid, int16 &lvol, int16 &rvol) const;

	static AudioProcess    *the_audio_process;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
