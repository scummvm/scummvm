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

#ifndef ILLUSIONS_MIDIRESOURCE_H
#define ILLUSIONS_MIDIRESOURCE_H

#include "illusions/graphics.h"
#include "illusions/resourcesystem.h"

namespace Illusions {

class IllusionsEngine;

class MidiGroupResourceLoader : public BaseResourceLoader {
public:
	MidiGroupResourceLoader(IllusionsEngine *vm) : _vm(vm) {}
	~MidiGroupResourceLoader() override {}
	void load(Resource *resource) override;
	bool isFlag(int flag) override;
protected:
	IllusionsEngine *_vm;
};

struct MidiMusic {
	uint32 _musicId;
	bool _looping;
	void load(Common::SeekableReadStream &stream);
};

class MidiGroupResource {
public:
	MidiGroupResource();
	~MidiGroupResource();
	void load(byte *data, uint32 dataSize);
public:
	uint _midiMusicCount;
	MidiMusic *_midiMusic;
};

class MidiGroupInstance : public ResourceInstance {
public:
	MidiGroupInstance(IllusionsEngine *vm);
	void load(Resource *resource) override;
	void unload() override;
public:
	IllusionsEngine *_vm;	
	MidiGroupResource *_midiGroupResource;
	uint32 _resId;
};

} // End of namespace Illusions

#endif // ILLUSIONS_SOUNDRESOURCE_H
