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

class AudioProcess :
	public Process {
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
		SampleInfo(Std::string &b, int32 shpnum, ObjId o, int32 c,
		           uint32 s, uint32 e, uint32 ps, uint16 v, int16 lv, int16 rv) :
			_sfxNum(-1), _priority(shpnum), _objId(o), _loops(0), _channel(c), _barked(b),
			_curSpeechStart(s), _curSpeechEnd(e), _pitchShift(ps), _volume(v),
			_lVol(lv), _rVol(rv) { }
	};

	Std::list<SampleInfo>   sample_info;

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

	static void ConCmd_listSFX(const Console::ArgvType &argv);
	static void ConCmd_stopSFX(const Console::ArgvType &argv);
	static void ConCmd_playSFX(const Console::ArgvType &argv);


	void run() override;

	void playSFX(int _sfxNum, int _priority, ObjId _objId, int _loops,
	             bool no_duplicates, uint32 _pitchShift,
	             uint16 _volume, int16 _lVol, int16 _rVol);

	void playSFX(int _sfxNum, int _priority, ObjId _objId, int _loops,
	             bool no_duplicates = false, uint32 _pitchShift = 0x10000,
	             uint16 _volume = 0x80) {
		playSFX(_sfxNum, _priority, _objId, _loops, no_duplicates, _pitchShift, _volume, -1, -1);
	}

	void stopSFX(int _sfxNum, ObjId _objId);
	bool isSFXPlaying(int _sfxNum);
	void setVolumeSFX(int _sfxNum, uint8 _volume);

	bool playSpeech(Std::string &_barked, int shapenum, ObjId _objId,
	                uint32 _pitchShift = 0x10000, uint16 _volume = 256);
	void stopSpeech(Std::string &_barked, int shapenum, ObjId _objId);
	bool isSpeechPlaying(Std::string &_barked, int shapenum);

	//! get length (in milliseconds) of speech
	uint32 getSpeechLength(Std::string &_barked, int shapenum) const;

	//! play a sample (without storing a SampleInfo)
	//! returns _channel sample is played on, or -1
	int playSample(AudioSample *sample, int _priority, int _loops,
	               uint32 _pitchShift = 0x10000, int16 _lVol = 256, int16 _rVol = 256);

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
	void saveData(ODataSource *ods) override;
	uint32 paused;

	//! play the next speech sample for the text in this SampleInfo
	//! note: si is reused if successful
	//! returns true if there was speech left to play, or false if finished
	bool continueSpeech(SampleInfo &si);

	bool calculateSoundVolume(ObjId _objId, int16 &_lVol, int16 &_rVol) const;

	static AudioProcess *_theAudioProcess;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
