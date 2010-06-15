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
	bool scanned;
	const Common::FSNode *resourceFile;
	int volume_number;
	ResourceSource *associated_map;
	uint32 audioCompressionType;
	int32 *audioCompressionOffsetMapping;
	Common::MacResManager *_macResMan;

protected:
	ResourceSource(ResSourceType type, const Common::String &name);
public:
	virtual ~ResourceSource();

	ResSourceType getSourceType() const { return _sourceType; }
	const Common::String &getLocationName() const { return _name; }
};

class DirectoryResourceSource : public ResourceSource {
public:
	DirectoryResourceSource(const Common::String &name) : ResourceSource(kSourceDirectory, name) {}
};

class PatchResourceSource : public ResourceSource {
public:
	PatchResourceSource(const Common::String &name) : ResourceSource(kSourcePatch, name) {}
};

class VolumeResourceSource : public ResourceSource {
public:
	VolumeResourceSource(const Common::String &name) : ResourceSource(kSourceVolume, name) {}
};

class ExtMapResourceSource : public ResourceSource {
public:
	ExtMapResourceSource(const Common::String &name) : ResourceSource(kSourceExtMap, name) {}
};

class IntMapResourceSource : public ResourceSource {
public:
	IntMapResourceSource(const Common::String &name) : ResourceSource(kSourceIntMap, name) {}
};

class AudioVolumeResourceSource : public ResourceSource {
public:
	AudioVolumeResourceSource(const Common::String &name) : ResourceSource(kSourceAudioVolume, name) {}
};

class ExtAudioMapResourceSource : public ResourceSource {
public:
	ExtAudioMapResourceSource(const Common::String &name) : ResourceSource(kSourceExtAudioMap, name) {}
};

class WaveResourceSource : public ResourceSource {
public:
	WaveResourceSource(const Common::String &name) : ResourceSource(kSourceWave, name) {}
};

class MacResourceForkResourceSource : public ResourceSource {
public:
	MacResourceForkResourceSource(const Common::String &name) : ResourceSource(kSourceMacResourceFork, name) {}
};

} // End of namespace Sci

#endif // SCI_RESOURCE_INTERN_H
