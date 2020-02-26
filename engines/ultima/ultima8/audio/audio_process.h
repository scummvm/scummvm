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

class AudioSample;

class AudioProcess : public Process {
public:
	struct SampleInfo {
		int32       _sfxNum;
		int32       _priority;
		ObjId       _objId;
		int32       _loops;
		int32       _channel;
		Std::string _barked;
		uint32      _curSpeechStart, _curSpeechEnd;
		uint32      _pitchShift;    // 0x10000 is normal
		uint16      _volume;         // 0-256
		int16       _lVol;
		int16       _rVol;

		SampleInfo() : _sfxNum(-1) { }
		SampleInfo(int32 s, int32 p, ObjId o, int32 l, int32 c, uint32 ps, uint16 v, int16 lv, int16 rv) :
			_sfxNum(s), _priority(p), _objId(o), _loops(l), _channel(c),
			_pitchShift(ps), _volume(v), _lVol(lv), _rVol(rv) { }
		SampleInfo(const Std::string &b, int32 shpnum, ObjId o, int32 c,
				   uint32 s, uint32 e, uint32 ps, uint16 v, int16 lv, int16 rv) :
			_sfxNum(-1), _priority(shpnum), _objId(o), _loops(0), _channel(c), _barked(b),
			_curSpeechStart(s), _curSpeechEnd(e), _pitchShift(ps), _volume(v),
			_lVol(lv), _rVol(rv) { }
	};

	Std::list<SampleInfo> _sampleInfo;
public:
	// p_dynamic_class stuff
	ENABLE_RUNTIME_CLASSTYPE()

	AudioProcess(void);
	~AudioProcess(void) override;

	//! Get the current instance of the Audio Processes
	static AudioProcess *get_instance() {
		return _theAudioProcess;
	}

	INTRINSIC(I_playSFX);
	INTRINSIC(I_playAmbientSFX);
	INTRINSIC(I_isSFXPlaying);
	INTRINSIC(I_setVolumeSFX);
	INTRINSIC(I_stopSFX);

	void run() override;

	void playSFX(int sfxNum, int priority, ObjId objId, int loops,
				 bool no_duplicates, uint32 pitchShift,
				 uint16 volume, int16 lVol, int16 rVol);

	void playSFX(int sfxNum, int priority, ObjId objId, int loops,
				 bool no_duplicates = false, uint32 pitchShift = 0x10000,
				 uint16 volume = 0x80) {
		playSFX(sfxNum, priority, objId, loops, no_duplicates, pitchShift, volume, -1, -1);
	}

	void stopSFX(int sfxNum, ObjId objId);
	bool isSFXPlaying(int sfxNum);
	void setVolumeSFX(int sfxNum, uint8 volume);

	bool playSpeech(const Std::string &barked, int shapenum, ObjId objId,
					uint32 pitchShift = 0x10000, uint16 volume = 256);
	void stopSpeech(const Std::string &barked, int shapenum, ObjId objId);
	bool isSpeechPlaying(const Std::string &barked, int shapenum);

	//! get length (in milliseconds) of speech
	uint32 getSpeechLength(const Std::string &barked, int shapenum) const;

	//! play a sample (without storing a SampleInfo)
	//! returns channel sample is played on, or -1
	int playSample(AudioSample *sample, int priority, int loops,
				   uint32 pitchShift = 0x10000, int16 lVol = 256, int16 rVol = 256);

	//! pause all currently playing samples
	void pauseAllSamples();
	//! unpause all currently playing samples
	void unpauseAllSamples();

	//! stop all samples except speech
	void stopAllExceptSpeech();

	bool loadData(IDataSource *ids, uint32 version);

private:
	void saveData(ODataSource *ods) override;
	uint32 _paused;

	//! play the next speech sample for the text in this SampleInfo
	//! note: si is reused if successful
	//! returns true if there was speech left to play, or false if finished
	bool continueSpeech(SampleInfo &si);

	bool calculateSoundVolume(ObjId objId, int16 &lVol, int16 &rVol) const;

	static AudioProcess *_theAudioProcess;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
