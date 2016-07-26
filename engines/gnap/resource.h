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

#ifndef GNAP_RESOURCE_H
#define GNAP_RESOURCE_H

#include "common/array.h"
#include "common/events.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/substream.h"
#include "common/system.h"

#include "graphics/surface.h"

#include "gnap/datarchive.h"

namespace Gnap {

enum {
	kResTypeSprite		= 0,
	kResTypeBitmap		= 1,
	kResTypeSound		= 2,
	kResTypeSequence	= 3
};

struct SequenceFrame {
	int16 _duration;
	bool _isScaled;
	Common::Rect _rect;
	int32 _spriteId;
	int32 _soundId;
	void loadFromStream(Common::MemoryReadStream &stream);
};

struct SequenceAnimation {
	int32 _additionalDelay;
	int16 _framesCount;
	int16 _maxTotalDuration;
	SequenceFrame *frames;

	SequenceAnimation() : frames(nullptr), _additionalDelay(0), _framesCount(0), _maxTotalDuration(0) {}
	~SequenceAnimation() { delete[] frames; }
	void loadFromStream(Common::MemoryReadStream &stream);
};

class SequenceResource {
public:
	SequenceResource(byte *data, uint32 size);
	~SequenceResource();
public:
	int32 _sequenceId;
	int32 _defaultId;
	int32 _sequenceId2;
	uint32 _defaultId2;
	uint32 _flags;
	int32 _totalDuration;
	int16 _xOffs;
	int16 _yOffs;
	int32 _animationsCount;
	SequenceAnimation *_animations;
};

class SpriteResource {
public:
	SpriteResource(byte *data, uint32 size);
	~SpriteResource();
public:
	byte *_data;
	byte *_pixels;
	uint32 *_palette;
	int16 _width, _height;
	uint16 _unknownVal1;
	uint16 _unknownVal2;
	bool _transparent;
	uint16 _colorsCount;
};

class SoundResource {
public:
	SoundResource(byte *data, uint32 size);
	~SoundResource();
public:
	byte *_data;
	uint32 _size;
};

template <class ResourceClass, int ResourceType, bool FreeAfterLoad>
class ResourceCacheTemplate {
public:

	ResourceCacheTemplate(DatManager *dat) : _dat(dat) {
	}

	~ResourceCacheTemplate() {
	}

	ResourceClass *get(int resourceId) {
		Resource *resource = find(resourceId);
		if (!resource) {
			debug(9, "Loading resource type %d with ID %08X from disk", ResourceType, resourceId);
			resource = new Resource(load(resourceId));
			_cache[resourceId] = resource;
		} else {
			debug(9, "Resource type %d with ID %08X was in cache", ResourceType, resourceId);
		}
		resource->_isLocked = true;
		return resource->_obj;
	}

	void release(int resourceId) {
		Resource *resource = find(resourceId);
		if (resource)
			resource->_isLocked = false;
	}

	void purge(bool force = false) {
		for (CacheMapIterator it = _cache.begin(); it != _cache.end(); ++it) {
			Resource *resource = it->_value;
			if (force || !resource->_isLocked) {
				delete resource;
				_cache.erase(it);
			}
		}
	}

protected:

	struct Resource {
		ResourceClass *_obj;
		bool _isLocked;
		Resource(ResourceClass *obj) : _obj(obj), _isLocked(false) {}
		~Resource() { delete _obj; }
	};

	typedef Common::HashMap<int, Resource *> CacheMap;
	typedef typename CacheMap::iterator CacheMapIterator;

	DatManager *_dat;
	CacheMap _cache;

	Resource *find(int resourceId) {
		CacheMapIterator it = _cache.find(resourceId);
		if (it != _cache.end())
			return it->_value;
		return nullptr;
	}

	ResourceClass *load(int resourceId) {
		if (_dat->getResourceType(resourceId) != ResourceType)
			error("ResourceCache::load() Wrong resource type: Expected %d, got %d", ResourceType, _dat->getResourceType(resourceId));

		byte *resourceData = _dat->loadResource(resourceId);
		uint32 resourceSize = _dat->getResourceSize(resourceId);
		ResourceClass *obj = new ResourceClass(resourceData, resourceSize);
		if (FreeAfterLoad)
			delete[] resourceData;
		return obj;
	}

};

typedef ResourceCacheTemplate<SpriteResource, kResTypeSprite, false> SpriteCache;
typedef ResourceCacheTemplate<SoundResource, kResTypeSound, false> SoundCache;
typedef ResourceCacheTemplate<SequenceResource, kResTypeSequence, true> SequenceCache;

} // End of namespace Gnap

#endif // GNAP_RESOURCE_H
