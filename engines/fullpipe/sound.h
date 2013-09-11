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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef FULLPIPE_SOUND_H
#define FULLPIPE_SOUND_H

namespace Fullpipe {

class Sound : public MemoryObject {
	int _id;
	char *_description;
	int16 _objectId;
	int16 _field_32;
	int _directSoundBuffer;
	int _directSoundBuffers[7];
	byte *_soundData;

  public:
	Sound();
	bool load(MfcArchive &file, NGIArchive *archive);
	void updateVolume();

	void setPanAndVolumeByStaticAni();
};

class SoundList : public CObject {
	Sound **_soundItems;
	int _soundItemsCount;
	NGIArchive *_libHandle;

 public:
	SoundList();
	bool load(MfcArchive &file, char *fname);
	bool loadFile(const char *fname, char *libname);

	int getCount() { return _soundItemsCount; }
	Sound *getSoundByIndex(int idx) { return _soundItems[idx]; }
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_SOUND_H */
