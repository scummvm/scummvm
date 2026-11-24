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

#ifndef NGI_SOUND_H
#define NGI_SOUND_H

#include "common/array.h"
#include "common/ptr.h"
#include "audio/mixer.h"

namespace NGI {

class Sound : public MemoryObject {
	int _id;
	int _flags = 0;
	byte *_soundData;
	Audio::SoundHandle *_handle;
	Common::Array<Audio::SoundHandle> _overlappingHandles;

public:
	enum { kEffect = 0, kSpeech = 1, kMusic = 2 };
	int16 _objectId;

	Sound();
	Sound(int id, const Common::Path &path, int flags);
	~Sound() override;

	virtual bool load(MfcArchive &file, NGIArchive *archive);
	bool load(MfcArchive &file) override { assert(0); return false; } // Disable base class
	void updateVolume();
	int getId() const { return _id; }
	int getFlags() const { return _flags; }
	Audio::SoundHandle *getHandle() const { return _handle; }
	bool isPlaying() const;

	void play(int loop, int creator = 0);
	void unload();
	int getVolume();
	void stop();
	void pause(bool paused);

	void setPanAndVolumeByStaticAni();
	void attenuate(int vol, int pan);
};

class SoundList : public CObject {
	Common::Array<Common::ScopedPtr<Sound>> _soundItems;
	Common::ScopedPtr<NGIArchive> _libHandle;

 public:
	virtual bool load(MfcArchive &file, const Common::Path &fname);
	bool load(MfcArchive &file) override { assert(0); return false; } // Disable base class
	bool loadFile(const Common::Path &fname, const Common::Path &libname);

	int getCount() { return _soundItems.size(); }
	Sound &getSoundByIndex(int idx) { return *_soundItems[idx]; }
	void addSound(int id, const Common::Path &path, int flags);
	void stopByCreator(int objectId);
	Sound *getSound(int id);
};

} // End of namespace NGI

#endif /* NGI_SOUND_H */
