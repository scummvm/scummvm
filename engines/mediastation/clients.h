/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MEDIASTATION_CLIENTS_H
#define MEDIASTATION_CLIENTS_H

#include "mediastation/datafile.h"

namespace MediaStation {

class ParameterClient {
public:
	ParameterClient() {};
	virtual ~ParameterClient() {};

	virtual bool attemptToReadFromStream(Chunk &chunk, uint sectionType) = 0;
};

enum DeviceOwnerSectionType {
	kDeviceOwnerAllowMultipleSounds = 0x35,
	kDeviceOwnerAllowMultipleStreams = 0x36,
};

class DeviceOwner : public ParameterClient {
public:
	virtual bool attemptToReadFromStream(Chunk &chunk, uint sectionType) override;

	bool _allowMultipleSounds = false;
	bool _allowMultipleStreams = false;
};

enum DocumentSectionType {
	kDocumentContextLoadComplete = 0x10,
	kDocumentStartupInformation = 0x2e,
	kDocumentEntryScreen = 0x2f,
};

class Document : public ParameterClient {
public:
	virtual bool attemptToReadFromStream(Chunk &chunk, uint sectionType) override;
	void readStartupInformation(Chunk &chunk);

	uint _entryScreenId = 0;
};

} // End of namespace MediaStation

#endif
