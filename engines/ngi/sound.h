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

#ifndef NGI_SOUND_H
#define NGI_SOUND_H

#include "common/array.h"
#include "common/ptr.h"

namespace Audio {
class SoundHandle;
}

namespace NGI {

class Sound : public MemoryObject {
	int _id;
	byte *_soundData;
	Audio::SoundHandle *_handle;

public:
	int16 _objectId;

public:
	Sound();
	~Sound() override;

	virtual bool load(MfcArchive &file, NGIArchive *archive);
	bool load(MfcArchive &file) override { assert(0); return false; } // Disable base class
	void updateVolume();
	int getId() const { return _id; }
	Audio::SoundHandle *getHandle() const { return _handle; }

	void play(int flag);
	void freeSound();
	int getVolume();
	void stop();

	void setPanAndVolumeByStaticAni();
	void setPanAndVolume(int vol, int pan);
};

class SoundList : public CObject {
	Common::Array<Sound> _soundItems;
	Common::ScopedPtr<NGIArchive> _libHandle;

 public:
	virtual bool load(MfcArchive &file, const Common::String &fname);
	bool load(MfcArchive &file) override { assert(0); return false; } // Disable base class
	bool loadFile(const Common::String &fname, const Common::String &libname);

	int getCount() { return _soundItems.size(); }
	Sound &getSoundByIndex(int idx) { return _soundItems[idx]; }
	Sound *getSoundItemById(int id);
};

} // End of namespace NGI

#endif /* NGI_SOUND_H */
