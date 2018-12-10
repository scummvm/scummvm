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

#include "bladerunner/savefile.h"

#include "bladerunner/boundingbox.h"
#include "bladerunner/vector.h"

#include "common/rect.h"
#include "common/savefile.h"
#include "common/system.h"

#include "graphics/thumbnail.h"

namespace BladeRunner {

SaveStateList SaveFileManager::list(const Common::String &target) {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray files = saveFileMan->listSavefiles(target + ".###");

	SaveStateList saveList;
	for (Common::StringArray::const_iterator fileName = files.begin(); fileName != files.end(); ++fileName) {
		Common::InSaveFile *saveFile = saveFileMan->openForLoading(*fileName);
		if (saveFile == nullptr || saveFile->err()) {
			continue;
		}

		BladeRunner::SaveFileHeader header;
		readHeader(*saveFile, header);

		int slotNum = atoi(fileName->c_str() + fileName->size() - 3);
		saveList.push_back(SaveStateDescriptor(slotNum, header._name));
	}

	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());

	return saveList;
}

SaveStateDescriptor SaveFileManager::queryMetaInfos(const Common::String &target, int slot) {
	Common::String filename = Common::String::format("%s.%03d", target.c_str(), slot);
	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(filename);

	if (saveFile == nullptr || saveFile->err()) {
		return SaveStateDescriptor();
	}

	BladeRunner::SaveFileHeader header;
	if (!BladeRunner::SaveFileManager::readHeader(*saveFile, header, false)) {
		delete saveFile;
		return SaveStateDescriptor();
	}
	delete saveFile;

	SaveStateDescriptor desc(slot, header._name);
	desc.setThumbnail(header._thumbnail);
	desc.setSaveDate(header._year, header._month, header._day);
	desc.setSaveTime(header._hour, header._minute);
	return desc;
}

Common::InSaveFile *SaveFileManager::openForLoading(const Common::String &target, int slot) {
	Common::String filename = Common::String::format("%s.%03d", target.c_str(), slot);
	return g_system->getSavefileManager()->openForLoading(filename);
}

Common::OutSaveFile *SaveFileManager::openForSaving(const Common::String &target, int slot) {
	Common::String filename = Common::String::format("%s.%03d", target.c_str(), slot);
	return g_system->getSavefileManager()->openForSaving(filename);
}

void SaveFileManager::remove(const Common::String &target, int slot) {
	Common::String filename = Common::String::format("%s.%03d", target.c_str(), slot);
	g_system->getSavefileManager()->removeSavefile(filename);
}

bool SaveFileManager::readHeader(Common::SeekableReadStream &in, SaveFileHeader &header, bool skipThumbnail) {
	SaveFileReadStream s(in);

	if (s.readUint32BE() != kTag) {
		warning("No header found in save file");
		return false;
	}

	header._version = s.readByte();
	if (header._version != kVersion) {
		warning("Unsupported version of save file %u, supported is %u", header._version, kVersion);
		return false;
	}

	header._name = s.readStringSz(kNameLength);

	header._year   = s.readUint16LE();
	header._month  = s.readUint16LE();
	header._day    = s.readUint16LE();
	header._hour   = s.readUint16LE();
	header._minute = s.readUint16LE();

	header._thumbnail = nullptr;

	if (!skipThumbnail) {
		header._thumbnail = new Graphics::Surface(); // freed by ScummVM's smartptr

		int32 pos = s.pos();

		s.skip(4); //skip size;

		void *thumbnailData = new byte[kThumbnailSize]; // freed by ScummVM's smartptr
		s.read(thumbnailData, kThumbnailSize);

		// TODO: cleanup - remove magic constants
		header._thumbnail->init(80, 60, 160, thumbnailData, Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0));

		s.seek(pos);
	}

	return true;
}

bool SaveFileManager::writeHeader(Common::WriteStream &out, SaveFileHeader &header) {
	SaveFileWriteStream s(out);

	s.writeUint32BE(kTag);
	s.writeByte(kVersion);

	s.writeStringSz(header._name, kNameLength);

	TimeDate td;
	g_system->getTimeAndDate(td);
	s.writeUint16LE(td.tm_year + 1900);
	s.writeUint16LE(td.tm_mon + 1);
	s.writeUint16LE(td.tm_mday);
	s.writeUint16LE(td.tm_hour);
	s.writeUint16LE(td.tm_min);

	return true;
}

SaveFileWriteStream::SaveFileWriteStream(Common::WriteStream &s) : _s(s) {}

void SaveFileWriteStream::debug(char *p) {
	write(p, strlen(p) + 1);
}

void SaveFileWriteStream::padBytes(int count) {
	for (int i = 0; i < count; ++i) {
		writeByte(0);
	}
}

void SaveFileWriteStream::writeInt(int v) {
	writeUint32LE(v);
}

void SaveFileWriteStream::writeFloat(float v) {
	writeFloatLE(v);
}

void SaveFileWriteStream::writeBool(bool v) {
	writeUint32LE(v);
}

void SaveFileWriteStream::writeStringSz(const Common::String &s, int sz) {
	assert(s.size() < (uint)sz);
	write(s.begin(), s.size());
	padBytes((uint)sz - s.size());
}

void SaveFileWriteStream::writeVector2(const Vector2 &v) {
	writeFloatLE(v.x);
	writeFloatLE(v.y);
}

void SaveFileWriteStream::writeVector3(const Vector3 &v) {
	writeFloatLE(v.x);
	writeFloatLE(v.y);
	writeFloatLE(v.z);
}

void SaveFileWriteStream::writeRect(const Common::Rect &v) {
	writeUint32LE(v.left);
	writeUint32LE(v.top);
	writeUint32LE(v.right);
	writeUint32LE(v.bottom);
}

void SaveFileWriteStream::writeBoundingBox(const BoundingBox &v, bool serialized) {
	float x0, y0, z0, x1, y1, z1;

	v.getXYZ(&x0, &y0, &z0, &x1, &y1, &z1);
	writeFloatLE(x0);
	writeFloatLE(y0);
	writeFloatLE(z0);
	writeFloatLE(x1);
	writeFloatLE(y1);
	writeFloatLE(z1);

	// Bounding boxes have a lot of extra data that's never actually used
	int count = serialized ? 96 : 64;
	for (int i = 0; i < count; ++i) {
		writeFloatLE(0.0f);
	}
}

SaveFileReadStream::SaveFileReadStream(Common::SeekableReadStream &s) : _s(s) {}

int SaveFileReadStream::readInt() {
	return readUint32LE();
}

float SaveFileReadStream::readFloat() {
	return readFloatLE();
}

bool SaveFileReadStream::readBool() {
	return readUint32LE();
}

Common::String SaveFileReadStream::readStringSz(int sz) {
	char *buf = new char[sz];
	read(buf, sz);
	Common::String result = buf;
	delete[] buf;
	return result;
}

Vector2 SaveFileReadStream::readVector2() {
	Vector2 result;
	result.x = readFloatLE();
	result.y = readFloatLE();
	return result;
}

Vector3 SaveFileReadStream::readVector3() {
	Vector3 result;
	result.x = readFloatLE();
	result.y = readFloatLE();
	result.z = readFloatLE();
	return result;
}

Common::Rect SaveFileReadStream::readRect() {
	Common::Rect result;
	result.left = readUint32LE();
	result.top = readUint32LE();
	result.right = readUint32LE();
	result.bottom = readUint32LE();
	return result;
}

BoundingBox SaveFileReadStream::readBoundingBox(bool serialized) {
	float x0, y0, z0, x1, y1, z1;

	x0 = readFloatLE();
	y0 = readFloatLE();
	z0 = readFloatLE();

	x1 = readFloatLE();
	y1 = readFloatLE();
	z1 = readFloatLE();

	// Bounding boxes have a lot of extra data that's never actually used, and there two formats for storing bounding boxes.
	int count = serialized ? 96 : 64;
	for (int i = 0; i < count; ++i) {
		readFloatLE();
	}

	return BoundingBox(x0, y0, z0, x1, y1, z1);
}



} // End of namespace BladeRunner
