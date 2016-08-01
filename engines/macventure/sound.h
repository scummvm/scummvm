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

#ifndef MACVENTURE_SOUND_H
#define MACVENTURE_SOUND_H

#include "macventure/macventure.h"
#include "macventure/container.h"

#include "common/file.h"
#include "common/hashmap.h"

namespace MacVenture {

enum SoundType {
	kSound10 = 0x10,
	kSound12 = 0x12,
	kSound18 = 0x18,
	kSound1a = 0x1a,
	kSound44 = 0x44,
	kSound78 = 0x78,
	kSound7e = 0x7e
};

class SoundAsset {

public:
	SoundAsset(Container *container, ObjID id);
	~SoundAsset();

	void play();
	uint32 getPlayLength();

private:

	void decode10(Common::SeekableReadStream *stream);
	void decode7e(Common::SeekableReadStream *stream);

private:

	Container *_container;
	ObjID _id;

	Common::Array<byte> _data;
	uint32 _length;
	uint32 _frequency;
};

class SoundManager {
public:
	SoundManager(MacVentureEngine *engine);
	~SoundManager();

	uint32 playSound(ObjID sound);

private:
	void ensureLoaded(ObjID sound);

private:

	Container *_container;
	Common::HashMap<ObjID, SoundAsset*> _assets;

};
} // End of namespace MacVenture

#endif
