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

#ifndef TWP_AUDIO_H
#define TWP_AUDIO_H

#include "common/str.h"
#include "common/array.h"
#include "common/stream.h"
#include "audio/mixer.h"
#include "twp/ggpack.h"

namespace Audio {

class SeekableAudioStream;
}

namespace Twp {

enum {
	NUM_AUDIO_SLOTS = 32
};

class AudioChannel;
class SoundDefinition;

class SoundDefinition;
class SoundStream : public Common::SeekableReadStream {
public:
	void open(Common::SharedPtr<SoundDefinition> sndDef);

	virtual uint32 read(void *dataPtr, uint32 dataSize) override;
	virtual bool eos() const override;

	virtual int64 pos() const override;
	virtual int64 size() const override;
	virtual bool seek(int64 offset, int whence = SEEK_SET) override;

private:
	MemStream _stream;
};

class SoundDefinition {
public:
	friend class SoundStream;

public:
	SoundDefinition(const Common::String &name);

	void load();
	int getId() const { return _id; }
	Common::String getName() const { return _name; }

private:
	int _id;                     // identifier for this sound
	Common::String _name;        // name of the sound to load
	Common::Array<byte> _buffer; // buffer containing the sound data
	bool _loaded = false;        // indicates whether or not the sound buffer has been loaded
};

struct AudioSlot {
	Audio::SoundHandle handle;                                         // handle returned when this sound has been played
	Common::SharedPtr<SoundDefinition> sndDef;                         // sound definition associated to this slot
	SoundStream stream;                                                // audio stream
	bool busy = false;                                                 // is sound active
	float volume = 1.f;                                                // actual volume for this slot
	float fadeInTimeMs = 0.f;                                          // fade-in time in milliseconds
	float fadeOutTimeMs = 0.f;                                         // fade-out time in milliseconds
	int total = 0;                                                     // duration of the sound in milliseconds
	int id = 0;                                                        // unique sound ID
	int objId = 0;                                                     // object ID or 0 if none
	int loopTimes = 0;                                                 // specified number of times to loop
	Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType; // sound type: plain, music, sfx, speech
};

class AudioSystem {
public:
	int play(Common::SharedPtr<SoundDefinition> sndDef, Audio::Mixer::SoundType cat, int loopTimes = 0, float fadeInTimeMs = 0.f, float volume = 1.f, int objId = 0);

	bool playing(int id) const;
	bool playing(Common::SharedPtr<SoundDefinition> soundDef) const;

	void fadeOut(int id, float fadeTime);
	void stop(int id);

	void setMasterVolume(float vol);
	float getMasterVolume() const;
	void setVolume(int id, float vol);

	int getElapsed(int id) const;
	int getDuration(int id) const;

	void update(float elapsed);

	Common::Array<Common::SharedPtr<SoundDefinition> > _soundDefs;
	AudioSlot _slots[NUM_AUDIO_SLOTS];
	Common::SharedPtr<SoundDefinition> _soundHover; // not used yet, should be used in the GUI

private:
	void updateVolume(AudioSlot *slot);
	AudioSlot *getFreeSlot();

private:
	float _masterVolume = 1.f;
};

} // End of namespace Twp

#endif // TWP_H
