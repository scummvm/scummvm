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
 * $URL$
 * $Id$
 *
 */

#ifndef SCI_RESOURCE_INTERN_H
#define SCI_RESOURCE_INTERN_H

#include "sci/resource.h"

namespace Common {
	class MacResManager;
}

namespace Sci {

enum ResSourceType {
	kSourceDirectory = 0,
	kSourcePatch,
	kSourceVolume,
	kSourceExtMap,
	kSourceIntMap,
	kSourceAudioVolume,
	kSourceExtAudioMap,
	kSourceWave,
	kSourceMacResourceFork
};


class ResourceSource {
protected:
	const ResSourceType _sourceType;
	const Common::String _name;

public:
	bool _scanned;
	const Common::FSNode * const _resourceFile;
	const int _volumeNumber;

protected:
	ResourceSource(ResSourceType type, const Common::String &name, int volNum = 0, const Common::FSNode *resFile = 0);
public:
	virtual ~ResourceSource();

	ResSourceType getSourceType() const { return _sourceType; }
	const Common::String &getLocationName() const { return _name; }

	// Auxiliary method, used by loadResource implementations.
	Common::SeekableReadStream *getVolumeFile(Resource *res);

	/**
	 * TODO: Document this
	 */
	virtual ResourceSource *findVolume(ResourceSource *map, int volNum) {
		return NULL;
	};

	/**
	 * Scan this source for TODO.
	 */
	virtual void scanSource() {}

	/**
	 * Load a resource.
	 */
	virtual void loadResource(Resource *res);

	virtual uint32 getAudioCompressionType() const { return 0; }

};

class DirectoryResourceSource : public ResourceSource {
public:
	DirectoryResourceSource(const Common::String &name) : ResourceSource(kSourceDirectory, name) {}

	virtual void scanSource();
};

class PatchResourceSource : public ResourceSource {
public:
	PatchResourceSource(const Common::String &name) : ResourceSource(kSourcePatch, name) {}

	virtual void loadResource(Resource *res);
};

class VolumeResourceSource : public ResourceSource {
protected:
	ResourceSource * const _associatedMap;

public:
	VolumeResourceSource(const Common::String &name, ResourceSource *map, int volNum, ResSourceType type = kSourceVolume)
		: ResourceSource(type, name, volNum), _associatedMap(map) {
	}

	VolumeResourceSource(const Common::String &name, ResourceSource *map, int volNum, const Common::FSNode *resFile)
		: ResourceSource(kSourceVolume, name, volNum, resFile), _associatedMap(map) {
	}

	virtual ResourceSource *findVolume(ResourceSource *map, int volNum) {
		if (_associatedMap == map && _volumeNumber == volNum)
			return this;
		return NULL;
	};
};

class ExtMapResourceSource : public ResourceSource {
public:
	ExtMapResourceSource(const Common::String &name, int volNum, const Common::FSNode *resFile = 0)
		: ResourceSource(kSourceExtMap, name, volNum, resFile) {
	}

	virtual void scanSource();
};

class IntMapResourceSource : public ResourceSource {
public:
	IntMapResourceSource(const Common::String &name, int volNum)
		: ResourceSource(kSourceIntMap, name, volNum) {
	}

	virtual void scanSource();
};

class AudioVolumeResourceSource : public VolumeResourceSource {
protected:
	uint32 _audioCompressionType;
	int32 *_audioCompressionOffsetMapping;

public:
	AudioVolumeResourceSource(const Common::String &name, ResourceSource *map, int volNum);

	virtual void loadResource(Resource *res);

	virtual uint32 getAudioCompressionType() const;
};

class ExtAudioMapResourceSource : public ResourceSource {
public:
	ExtAudioMapResourceSource(const Common::String &name, int volNum)
		: ResourceSource(kSourceExtAudioMap, name, volNum) {
	}

	virtual void scanSource();
};

class WaveResourceSource : public ResourceSource {
public:
	WaveResourceSource(const Common::String &name) : ResourceSource(kSourceWave, name) {}

	virtual void loadResource(Resource *res);
};

/**
 * Reads SCI1.1+ resources from a Mac resource fork.
 */
class MacResourceForkResourceSource : public ResourceSource {
protected:
	Common::MacResManager *_macResMan;

public:
	MacResourceForkResourceSource(const Common::String &name, int volNum);
	~MacResourceForkResourceSource();

	virtual void scanSource();

	virtual void loadResource(Resource *res);
};

} // End of namespace Sci

#endif // SCI_RESOURCE_INTERN_H
