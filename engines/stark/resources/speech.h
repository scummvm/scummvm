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

#ifndef STARK_RESOURCES_SPEECH_H
#define STARK_RESOURCES_SPEECH_H

#include "common/str.h"

#include "engines/stark/resources/resource.h"

namespace Stark {

class XRCReadStream;

namespace Resources {

class Sound;

/**
 * Speech resource
 *
 * Speech resources are used to define dialog lines.
 * A Speech resource contains text for a character and references
 * a Sound resource for the dubbing.
 */
class Speech : public Resource {
public:
	static const ResourceType::Type TYPE = ResourceType::kSpeech;

	Speech(Resource *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Speech();

	// Resource API
	void readData(XRCReadStream *stream) override;
	void onExitLocation() override;
	void onPreDestroy() override;


	/** Obtain the text associated to the speech line */
	Common::String getPhrase() const;

	/** Play the voice over */
	void playSound();

	/** Return true if the speech is playing */
	bool isPlaying();

	/** Stop the speech if it is playing */
	void stop();

protected:
	void printData() override;

	Common::String _phrase;
	uint32 _character;

	Sound *_soundResource;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_SPEECH_H
