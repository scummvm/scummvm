/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_RESOURCES_SOUND_H
#define STARK_RESOURCES_SOUND_H

#include "common/str.h"

#include "engines/stark/resources/resource.h"

namespace Stark {

class XRCReadStream;

class Sound : public Resource {
public:
	static const ResourceType::Type TYPE = ResourceType::kSoundItem;

	enum {
		kSoundTypeVoice  = 0,
		kSoundTypeEffect = 1,
		kSoundTypeMusic  = 2
	};

	Sound(Resource *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Sound();

	// Resource API
	void readData(XRCReadStream *stream) override;

protected:
	void printData() override;

	Common::String _filename;
	Common::String _archiveName;
	uint32 _enabled;
	uint32 _looping;
	uint32 _field_64;
	uint32 _playUntilComplete;
	uint32 _maxDuration;
	uint32 _stockSoundType;
	Common::String _soundName;
	uint32 _field_6C;
	uint32 _soundType;
	uint32 _pan;
	float _volume;
};

} // End of namespace Stark

#endif // STARK_RESOURCES_SOUND_H
