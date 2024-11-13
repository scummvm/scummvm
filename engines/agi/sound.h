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

#ifndef AGI_SOUND_H
#define AGI_SOUND_H

namespace Audio {
class Mixer;
class SoundHandle;
}

namespace Agi {

#define SOUND_EMU_NONE  0
#define SOUND_EMU_PC    1
#define SOUND_EMU_PCJR  2
#define SOUND_EMU_MAC   3
#define SOUND_EMU_AMIGA 4
#define SOUND_EMU_APPLE2 5
#define SOUND_EMU_APPLE2GS 6
#define SOUND_EMU_COCO3 7
#define SOUND_EMU_MIDI 8

/**
 * AGI sound resource types.
 * These values are the first 16-bit LE words of each resource's header,
 * except for AGIv1, Apple II, and CoCo3, which do not have headers.
 */
enum AgiSoundEmuType {
	AGI_SOUND_SAMPLE    = 0x0001,
	AGI_SOUND_MIDI      = 0x0002,
	AGI_SOUND_4CHN      = 0x0008,
	AGI_SOUND_APPLE2    = 0xffff,
	AGI_SOUND_COCO3     = 0xfffe
};

class SoundMgr;

class SoundGen {
public:
	SoundGen(AgiBase *vm, Audio::Mixer *pMixer);
	virtual ~SoundGen();

	virtual void play(int resnum) = 0;
	virtual void stop() = 0;

	AgiBase *_vm;

	Audio::Mixer *_mixer;
	Audio::SoundHandle *_soundHandle;

	uint32 _sampleRate;
};

/**
 * AGI sound resource structure.
 */
class AgiSound {
public:
	AgiSound(byte resourceNr, byte *data, uint32 length, uint16 type) :
		_resourceNr(resourceNr),
		_data(data),
		_length(length),
		_type(type),
		_isPlaying(false) {}

	virtual ~AgiSound()      { free(_data); }

	virtual void play()      { _isPlaying = true; }
	virtual void stop()      { _isPlaying = false; }
	virtual bool isPlaying() { return _isPlaying; }
	byte *getData()          { return _data; }
	uint32 getLength()       { return _length; }
	virtual uint16 type()    { return _type; }
	virtual bool isValid()   { return true; }

	/**
	 * A named constructor for creating different types of AgiSound objects
	 * from a raw sound resource.
	 */
	static AgiSound *createFromRawResource(uint8 *data, uint32 len, int resnum, int soundemu, bool isAgiV1);

protected:
	byte _resourceNr;
	byte *_data;
	uint32 _length;
	uint16 _type;
	bool _isPlaying;
};

class PCjrSound : public AgiSound {
public:
	PCjrSound(byte resourceNr, byte *data, uint32 length, uint16 type);
	const uint8 *getVoicePointer(uint voiceNum);
};

class SoundMgr {

public:
	SoundMgr(AgiBase *agi, Audio::Mixer *pMixer);
	~SoundMgr();

	void unloadSound(int);
	void startSound(int, int);
	void stopSound();

	void soundIsFinished();
	bool isPlaying() const { return _playingSound != -1; }

private:
	int _endflag;
	AgiBase *_vm;

	SoundGen *_soundGen;

	int _playingSound;
};

} // End of namespace Agi

#endif /* AGI_SOUND_H */
