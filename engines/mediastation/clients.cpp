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

#include "mediastation/clients.h"

namespace MediaStation {

bool DeviceOwner::attemptToReadFromStream(Chunk &chunk, uint sectionType) {
	bool handledParam = true;
	switch (sectionType) {
	case kDeviceOwnerAllowMultipleSounds:
		_allowMultipleSounds = chunk.readTypedByte();
		break;

	case kDeviceOwnerAllowMultipleStreams:
		_allowMultipleStreams = chunk.readTypedByte();
		break;

	default:
		handledParam = false;
	}

	return handledParam;
}

void Document::readStartupInformation(Chunk &chunk) {
	DocumentSectionType sectionType = static_cast<DocumentSectionType>(chunk.readTypedUint16());
	switch (sectionType) {
	case kDocumentEntryScreen:
		_entryScreenId = chunk.readTypedUint16();
		break;

	default:
		error("%s: Unhandled section type 0x%x", __func__, static_cast<uint>(sectionType));
	}
}

bool Document::attemptToReadFromStream(Chunk &chunk, uint sectionType) {
	bool handledParam = true;
	switch (sectionType) {
	case kDocumentContextLoadComplete: {
		uint contextId = chunk.readTypedUint16();
		warning("STUB: readContextLoadCompleteFromStream %d", contextId);
		break;
	}

	case kDocumentStartupInformation:
		readStartupInformation(chunk);
		break;

	default:
		handledParam = false;
	}

	return handledParam;
}

} // End of namespace MediaStation
