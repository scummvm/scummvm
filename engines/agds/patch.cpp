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

#include "agds/patch.h"
#include "agds/resourceManager.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/stream.h"

namespace AGDS {

void ObjectPatch::load(Common::ReadStream &stream) {
	text = readString(stream);
	region = readString(stream);
	z = stream.readUint16LE();
}

void ObjectPatch::save(Common::WriteStream &stream) const {
	writeString(stream, text);
	writeString(stream, region);
	stream.writeUint16LE(z);
}

void Patch::load(Common::ReadStream &stream) {
	screenSaved = stream.readByte();
	screenRegionName = readString(stream);
	prevScreenName = readString(stream);
	debug("patch screen, valid: %d region: %s, prev: %s", screenSaved, screenRegionName.c_str(), prevScreenName.c_str());

	loadingType = static_cast<ScreenLoadingType>(stream.readUint32LE());
	characterPosition.x = stream.readSint32LE();
	characterPosition.y = stream.readSint32LE();
	characterDirection = stream.readSint32LE();
	characterPresent = stream.readUint32LE();
	debug("character %s at %u,%u with dir: %d", characterPresent ? "[present]" : "[absent]", characterPosition.x, characterPosition.y, characterDirection);
	uint object_count = stream.readUint32LE();
	debug("objects in this patch: %u", object_count);
	if (stream.read(palette, sizeof(palette)) != sizeof(palette)) {
		error("short read, can't read palette");
	}

	defaultMouseCursor = readString(stream);
	debug("default pointer name: %s", defaultMouseCursor.c_str());
	objects.clear();
	for (uint i = 0; i < object_count; ++i) {
		int flag = stream.readSint16LE();
		Common::String name = readString(stream);
		debug("patch object %s %d", name.c_str(), flag);
		objects.push_back(Object(name, flag));
	}
}

void Patch::save(Common::WriteStream &stream) {
	stream.writeByte(screenSaved);
	writeString(stream, screenRegionName);
	writeString(stream, prevScreenName);

	stream.writeUint32LE(static_cast<uint>(loadingType));
	stream.writeUint32LE(characterPosition.x);
	stream.writeUint32LE(characterPosition.y);
	stream.writeSint32LE(characterDirection);
	stream.writeUint32LE(characterPresent);

	stream.writeUint32LE(objects.size());
	if (stream.write(palette, sizeof(palette)) != sizeof(palette)) {
		error("short write, can't write palette");
	}

	writeString(stream, defaultMouseCursor);
	for (auto &object : objects) {
		stream.writeSint16LE(object.flag);
		writeString(stream, object.name);
	}
}

void Patch::setFlag(const Common::String &name, int flag) {
	for (auto &object : objects) {
		if (object.name == name) {
			object.flag = flag;
			return;
		}
	}
	objects.push_back({name, flag});
}

int Patch::getFlag(const Common::String &name) const {
	for (auto &object : objects) {
		if (object.name == name)
			return object.flag;
	}
	return 0;
}

int Patch::incRef(const Common::String &name) {
	for (auto &object : objects) {
		if (object.name == name) {
			return ++object.flag;
		}
	}
	objects.push_back({name, 1});
	return 1;
}

int Patch::decRef(const Common::String &name) {
	for (auto &object : objects) {
		if (object.name == name) {
			// this is original code lol
			object.flag = 0;
			return 0;
		}
	}
	objects.push_back({name, 0});
	return 0;
}

} // namespace AGDS
