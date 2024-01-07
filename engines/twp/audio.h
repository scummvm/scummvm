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

namespace Twp {

class AudioChannel;
class SoundDefinition;

struct SoundId {
    int id;
    int objId;
    SoundDefinition* sndDef;
    Audio::Mixer::SoundType cat;
    AudioChannel* chan;
    float pan;
};

class SoundDefinition;
class SoundStream: public Common::SeekableReadStream {
public:
	void open(SoundDefinition* sndDef);

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
	SoundDefinition(const Common::String& name);

	void load();
	int getId() const { return _id; }
	Common::String getName() const { return _name; }

private:
    int _id;						// identifier for this sound
    Common::String _name;		    // name of the sound to load
    Common::Array<byte> _buffer;	// buffer containing the sound data
    bool _loaded;				    // indicates whether or not the sound buffer has been loaded
};

struct AudioSlot {
	Audio::SoundHandle handle;
	SoundDefinition *sndDef = nullptr;
	SoundStream stream;
	bool busy = false;
	float volume = 1.f;
	float fadeInTimeMs = 0.f;
	float fadeOutTimeMs = 0.f;
	int total = 0;
	int id = 0;
};

class AudioSystem {
public:
	int play(SoundDefinition* sndDef, Audio::Mixer::SoundType cat, int loopTimes = 0, float fadeInTimeMs = 0.f, float volume = 1.f, int objId = 0);

	bool playing(int id) const;
	bool playing(SoundDefinition* soundDef) const;

	void fadeOut(int id, float fadeTime);
	void stop(int id);

	void setMasterVolume(float vol);
	float getMasterVolume() const;
	void setVolume(int id, float vol);

	void update(float elapsed);

	Common::Array<SoundDefinition*> _soundDefs;

private:
	void updateVolume(AudioSlot* slot);
	AudioSlot* getFreeSlot();

private:
	AudioSlot _slots[32];
	float _masterVolume = 1.f;
};

} // End of namespace Twp

#endif // TWP_H
