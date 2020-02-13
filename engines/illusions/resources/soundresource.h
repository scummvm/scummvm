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

#ifndef ILLUSIONS_SOUNDRESOURCE_H
#define ILLUSIONS_SOUNDRESOURCE_H

#include "illusions/graphics.h"
#include "illusions/resourcesystem.h"

namespace Illusions {

class IllusionsEngine;

class SoundGroupResourceLoader : public BaseResourceLoader {
public:
	SoundGroupResourceLoader(IllusionsEngine *vm) : _vm(vm) {}
	~SoundGroupResourceLoader() override {}
	void load(Resource *resource) override;
	bool isFlag(int flag) override;
protected:
	IllusionsEngine *_vm;
};

struct SoundEffect {
	uint32 _soundEffectId;
	bool _looping;
	int16 _field6;
	int16 _volume;
	int16 _frequency;
	void load(Common::SeekableReadStream &stream);
};

class SoundGroupResource {
public:
	SoundGroupResource();
	~SoundGroupResource();
	void load(byte *data, uint32 dataSize);
public:
	uint _soundEffectsCount;
	SoundEffect *_soundEffects;
};

class SoundGroupInstance : public ResourceInstance {
public:
	SoundGroupInstance(IllusionsEngine *vm);
	void load(Resource *resource) override;
	void unload() override;
public:
	IllusionsEngine *_vm;
	SoundGroupResource *_soundGroupResource;
	uint32 _resId;
};

} // End of namespace Illusions

#endif // ILLUSIONS_SOUNDRESOURCE_H
