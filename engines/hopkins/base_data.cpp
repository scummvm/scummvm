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

// Resource loaders and fixed-point lookup tables for Hopkins WBASE.

#include "hopkins/base_data.h"

#include "common/algorithm.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/stream.h"
#include "common/util.h"

namespace Hopkins {

namespace {

struct BitmapResource {
	uint16 id;
	const char *filename;
};

static const BitmapResource kWallResources[] = {
	{ 1,  "MUR1.BBM" },
	{ 2,  "MUR2.BBM" },
	{ 3,  "MUR3.BBM" },
	{ 4,  "PLAFOND.BBM" },
	{ 5,  "SOL.BBM" },
	{ 6,  "MUR4.BBM" },
	{ 7,  "MUR5.BBM" },
	{ 8,  "MUR6.BBM" },
	{ 9,  "MUR7.BBM" },
	{ 10, "MUR8.BBM" },
	{ 11, "MUR9.BBM" },
	{ 12, "MUR10.BBM" },
	{ 13, "MUR11.BBM" },
	{ 14, "MUR12.BBM" },
	{ 15, "MUR13.BBM" },
	{ 16, "PORTE3.BBM" },
	{ 59, "MURS.BBM" },
	{ 60, "PORTE2.BBM" },
	{ 62, "PORTE2.BBM" }
};

static const BitmapResource kObjectResources[] = {
	{ 1,  "H1.BBM" },
	{ 2,  "H2.BBM" },
	{ 3,  "H3.BBM" },
	{ 4,  "H4.BBM" },
	{ 5,  "TIRE1.BBM" },
	{ 6,  "TIRE2.BBM" },
	{ 7,  "FUITE1.BBM" },
	{ 8,  "FUITE2.BBM" },
	{ 9,  "FUITE3.BBM" },
	{ 10, "FUITE4.BBM" },
	{ 11, "MF1.BBM" },
	{ 12, "MF2.BBM" },
	{ 13, "MF3.BBM" },
	{ 14, "MF4.BBM" },
	{ 15, "MF5.BBM" },
	{ 16, "MF6.BBM" }
};

static const char *const kRequiredResources[] = {
	"BASE.MAP", "BASE.PAL", "INFO.DAT", "MAIN.SPR", "BASEFONT.SPR",
	"MUR1.BBM", "MUR2.BBM", "MUR3.BBM", "PLAFOND.BBM", "SOL.BBM",
	"MUR4.BBM", "MUR5.BBM", "MUR6.BBM", "MUR7.BBM", "MUR8.BBM",
	"MUR9.BBM", "MUR10.BBM", "MUR11.BBM", "MUR12.BBM", "MUR13.BBM",
	"PORTE3.BBM", "MURS.BBM", "PORTE2.BBM",
	"H1.BBM", "H2.BBM", "H3.BBM", "H4.BBM", "TIRE1.BBM", "TIRE2.BBM",
	"FUITE1.BBM", "FUITE2.BBM", "FUITE3.BBM", "FUITE4.BBM",
	"MF1.BBM", "MF2.BBM", "MF3.BBM", "MF4.BBM", "MF5.BBM", "MF6.BBM"
};

static const char *const kAudioResources[] = {
	"SOUND54.WAV", "SOUND40.WAV", "SOUND53.WAV", "SOUND55.WAV"
};

static const uint32 kTagFORM = MKTAG('F', 'O', 'R', 'M');
static const uint32 kTagPBM = MKTAG('P', 'B', 'M', ' ');
static const uint32 kTagBMHD = MKTAG('B', 'M', 'H', 'D');
static const uint32 kTagBODY = MKTAG('B', 'O', 'D', 'Y');

static bool appendMissing(const char *filename, Common::String *missingResources) {
	if (Common::File::exists(Common::Path(filename)))
		return false;
	if (missingResources) {
		if (!missingResources->empty())
			*missingResources += ", ";
		*missingResources += filename;
	}
	return true;
}

static bool readExact(Common::SeekableReadStream &stream, void *dest, uint32 size) {
	return size == 0 || stream.read(dest, size) == size;
}

static bool decodeByteRun1(const Common::Array<byte> &source, uint32 expectedSize, Common::Array<byte> &dest) {
	dest.clear();
	dest.reserve(expectedSize);
	uint sourcePos = 0;
	while (sourcePos < source.size() && dest.size() < expectedSize) {
		const int8 control = (int8)source[sourcePos++];
		if (control >= 0) {
			const uint count = (uint)control + 1;
			if (sourcePos + count > source.size() || dest.size() + count > expectedSize)
				return false;
			for (uint i = 0; i < count; ++i)
				dest.push_back(source[sourcePos++]);
		} else if (control != -128) {
			const uint count = (uint)(1 - control);
			if (sourcePos >= source.size() || dest.size() + count > expectedSize)
				return false;
			const byte value = source[sourcePos++];
			for (uint i = 0; i < count; ++i)
				dest.push_back(value);
		}
	}
	return dest.size() == expectedSize;
}

} // End of anonymous namespace

BaseData::BaseData() {
	Common::fill(_map, _map + ARRAYSIZE(_map), 0);
	Common::fill(_objectMap, _objectMap + ARRAYSIZE(_objectMap), 0);
	Common::fill(_palette, _palette + ARRAYSIZE(_palette), 0);
	Common::fill(_viewCos, _viewCos + ARRAYSIZE(_viewCos), 0);
	Common::fill(_floorCos, _floorCos + ARRAYSIZE(_floorCos), 0);
	Common::fill(_distanceHeight, _distanceHeight + ARRAYSIZE(_distanceHeight), 0);
	Common::fill(_adjust, _adjust + ARRAYSIZE(_adjust), 0);
	_wallBitmaps.resize(256);
	_objectBitmaps.resize(256);
}

bool BaseData::hasRequiredResources(Common::String *missingResources) {
	bool missing = false;
	if (missingResources)
		missingResources->clear();
	for (uint i = 0; i < ARRAYSIZE(kRequiredResources); ++i)
		missing |= appendMissing(kRequiredResources[i], missingResources);
	return !missing;
}

void BaseData::appendAudioResourceReport(Common::String &missingResources) {
	for (uint i = 0; i < ARRAYSIZE(kAudioResources); ++i)
		appendMissing(kAudioResources[i], &missingResources);
}

bool BaseData::load(Common::String &errorMessage) {
	errorMessage.clear();
	if (!loadMap(errorMessage) || !loadPalette(errorMessage) || !loadInfo(errorMessage) ||
			!loadBitmaps(errorMessage))
		return false;
	buildDerivedTables();
	return true;
}

bool BaseData::loadMap(Common::String &errorMessage) {
	Common::File file;
	if (!file.open(Common::Path("BASE.MAP"))) {
		errorMessage = "BASE.MAP not found";
		return false;
	}
	if (file.size() != kBaseMapCellCount * 4) {
		errorMessage = Common::String::format("BASE.MAP has unexpected size %u", (uint)file.size());
		return false;
	}
	for (int i = 0; i < kBaseMapCellCount; ++i)
		_map[i] = file.readUint16LE();
	for (int i = 0; i < kBaseMapCellCount; ++i)
		_objectMap[i] = file.readUint16LE();
	if (file.err()) {
		errorMessage = "BASE.MAP is truncated";
		return false;
	}
	return true;
}

bool BaseData::loadPalette(Common::String &errorMessage) {
	Common::File file;
	if (!file.open(Common::Path("BASE.PAL"))) {
		errorMessage = "BASE.PAL not found";
		return false;
	}
	if (file.size() != sizeof(_palette) || !readExact(file, _palette, sizeof(_palette))) {
		errorMessage = "BASE.PAL must contain exactly 768 bytes";
		return false;
	}
	return true;
}

bool BaseData::loadInfo(Common::String &errorMessage) {
	Common::File file;
	if (!file.open(Common::Path("INFO.DAT"))) {
		errorMessage = "INFO.DAT not found";
		return false;
	}
	const uint32 expected = 7 * kBaseAngleCount * sizeof(int32);
	if (file.size() != expected) {
		errorMessage = Common::String::format("INFO.DAT has size %u; expected %u", (uint)file.size(), expected);
		return false;
	}
	for (uint table = 0; table < ARRAYSIZE(_trig); ++table) {
		_trig[table].resize(kBaseAngleCount);
		for (int angle = 0; angle < kBaseAngleCount; ++angle)
			_trig[table][angle] = file.readSint32LE();
	}
	return !file.err();
}

bool BaseData::loadBitmaps(Common::String &errorMessage) {
	for (uint i = 0; i < ARRAYSIZE(kWallResources); ++i) {
		if (!loadBbm(Common::Path(kWallResources[i].filename), _wallBitmaps[kWallResources[i].id], errorMessage))
			return false;
	}
	for (uint i = 0; i < ARRAYSIZE(kObjectResources); ++i) {
		if (!loadBbm(Common::Path(kObjectResources[i].filename), _objectBitmaps[kObjectResources[i].id], errorMessage))
			return false;
	}
	if (!loadSpr(Common::Path("MAIN.SPR"), _weaponFrames, errorMessage))
		return false;
	if (!loadSpr(Common::Path("BASEFONT.SPR"), _fontFrames, errorMessage))
		return false;
	if (_weaponFrames.size() != 2 || _fontFrames.size() != 10) {
		errorMessage = "Unexpected MAIN.SPR or BASEFONT.SPR frame count";
		return false;
	}
	return true;
}

bool BaseData::loadBbm(const Common::Path &filename, BaseBitmap &bitmap, Common::String &errorMessage) {
	Common::File file;
	if (!file.open(filename)) {
		errorMessage = Common::String::format("%s not found", filename.toString(Common::Path::kNativeSeparator).c_str());
		return false;
	}
	if (file.size() < 12 || file.readUint32BE() != kTagFORM) {
		errorMessage = Common::String::format("%s is not an IFF FORM", filename.toString().c_str());
		return false;
	}
	const uint32 formSize = file.readUint32BE();
	if (file.readUint32BE() != kTagPBM) {
		errorMessage = Common::String::format("%s is not an IFF PBM", filename.toString().c_str());
		return false;
	}

	uint16 width = 0;
	uint16 height = 0;
	byte compression = 0xff;
	Common::Array<byte> body;
	const uint32 formEnd = MIN((uint32)file.size(), formSize + 8);
	while ((uint32)file.pos() + 8 <= formEnd) {
		const uint32 tag = file.readUint32BE();
		const uint32 chunkSize = file.readUint32BE();
		if ((uint32)file.pos() + chunkSize > formEnd) {
			errorMessage = Common::String::format("%s contains a truncated IFF chunk", filename.toString().c_str());
			return false;
		}
		if (tag == kTagBMHD) {
			if (chunkSize < 20) {
				errorMessage = Common::String::format("%s has a short BMHD", filename.toString().c_str());
				return false;
			}
			width = file.readUint16BE();
			height = file.readUint16BE();
			file.skip(6);
			compression = file.readByte();
			file.skip(chunkSize - 11);
		} else if (tag == kTagBODY) {
			body.resize(chunkSize);
			if (!readExact(file, body.begin(), chunkSize)) {
				errorMessage = Common::String::format("%s has a truncated BODY", filename.toString().c_str());
				return false;
			}
		} else {
			file.skip(chunkSize);
		}
		if (chunkSize & 1)
			file.skip(1);
	}

	if (width != 64 || height != 64 || body.empty()) {
		errorMessage = Common::String::format("%s is not a 64x64 ACK bitmap", filename.toString().c_str());
		return false;
	}
	Common::Array<byte> rowMajor;
	const uint32 expected = (uint32)width * height;
	if (compression == 0) {
		if (body.size() < expected) {
			errorMessage = Common::String::format("%s has a short uncompressed BODY", filename.toString().c_str());
			return false;
		}
		rowMajor.resize(expected);
		Common::copy(body.begin(), body.begin() + expected, rowMajor.begin());
	} else if (compression == 1) {
		if (!decodeByteRun1(body, expected, rowMajor)) {
			errorMessage = Common::String::format("%s has invalid ByteRun1 data", filename.toString().c_str());
			return false;
		}
	} else {
		errorMessage = Common::String::format("%s uses unsupported IFF compression %u", filename.toString().c_str(), compression);
		return false;
	}

	// AckReadiff transposes PBM scanlines into contiguous texture columns.
	bitmap.width = width;
	bitmap.height = height;
	bitmap.columnMajor = true;
	bitmap.pixels.resize(expected);
	bitmap.blankColumns.resize(width);
	for (uint x = 0; x < width; ++x) {
		bool blank = true;
		for (uint y = 0; y < height; ++y) {
			const byte pixel = rowMajor[y * width + x];
			bitmap.pixels[x * height + y] = pixel;
			blank &= pixel == 0;
		}
		bitmap.blankColumns[x] = blank ? 1 : 0;
	}
	return true;
}

bool BaseData::loadSpr(const Common::Path &filename, Common::Array<BaseBitmap> &frames, Common::String &errorMessage) {
	Common::File file;
	if (!file.open(filename)) {
		errorMessage = Common::String::format("%s not found", filename.toString().c_str());
		return false;
	}
	frames.clear();
	while (file.pos() < file.size()) {
		if (file.size() - file.pos() < 6) {
			errorMessage = Common::String::format("%s has a partial sprite header", filename.toString().c_str());
			return false;
		}
		const uint16 pixelCount = file.readUint16LE();
		const uint16 width = file.readUint16LE();
		const uint16 height = file.readUint16LE();
		if (!width || !height || pixelCount != (uint32)width * height || file.size() - file.pos() < pixelCount) {
			errorMessage = Common::String::format("%s has an invalid sprite frame", filename.toString().c_str());
			return false;
		}
		BaseBitmap frame;
		frame.width = width;
		frame.height = height;
		frame.columnMajor = false;
		frame.pixels.resize(pixelCount);
		if (!readExact(file, frame.pixels.begin(), pixelCount))
			return false;
		frames.push_back(frame);
	}
	return !frames.empty();
}

void BaseData::buildDerivedTables() {
	// The public ACK code repairs tangent singularities; Hopkins uses the same
	// layout with quarter turns at 480, 960 and 1440.
	for (int angle = kBaseQuarterTurn; angle <= kBaseThreeQuarterTurn; angle += kBaseQuarterTurn) {
		_trig[2][angle] = _trig[2][angle + 1];
		_trig[3][angle] = _trig[3][angle + 1];
	}

	_xNext.resize(kBaseAngleCount);
	_yNext.resize(kBaseAngleCount);
	for (int angle = 0; angle < kBaseAngleCount; ++angle) {
		// ACK performs these multiplies in 32-bit Watcom arithmetic.  The
		// tangent entries adjacent to a singularity intentionally wrap; use an
		// unsigned shift so the modern C++ port has the same defined result.
		_yNext[angle] = (int32)((uint32)_trig[2][angle] << kBaseCellShift);
		_xNext[angle] = (int32)((uint32)_trig[3][angle] << kBaseCellShift);
	}

	int cameraAngle = kBaseAngleHalfFov;
	int direction = -1;
	for (int column = 0; column < kBaseViewWidth; ++column) {
		_viewCos[column] = _trig[6][cameraAngle];
		_floorCos[column] = _trig[4][cameraAngle] >> 6;
		cameraAngle += direction;
		if (cameraAngle <= 0) {
			cameraAngle = -cameraAngle;
			direction = -direction;
		}
	}
	_floorCos[kBaseViewWidth] = _floorCos[kBaseViewWidth - 1];

	// Preserve the two differently scaled inverse tables used by ACK rays.
	for (int angle = 0; angle < kBaseAngleCount; ++angle) {
		_trig[4][angle] >>= 4;
		_trig[5][angle] >>= 6;
	}

	const int32 heightScale = kBaseCellSize * 128;
	_distanceHeight[0] = kBaseMaximumWallHeight;
	_adjust[0] = 4194304L / heightScale;
	for (int distance = 1; distance < kBaseMaximumDistance; ++distance) {
		int height = heightScale / distance;
		if (heightScale - height * distance > distance / 2)
			++height;
		height = CLIP(height, kBaseMinimumWallHeight, kBaseMaximumWallHeight);
		_distanceHeight[distance] = height;
		_adjust[distance] = 2097152L / height;
	}
	_distanceHeight[kBaseMaximumDistance] = kBaseMinimumWallHeight;
	_adjust[kBaseMaximumDistance] = _adjust[kBaseMaximumDistance - 1];
}

uint16 BaseData::mapCodeAt(int mapPos) const {
	return mapPos >= 0 && mapPos < kBaseMapCellCount ? _map[mapPos] : 0;
}

uint16 BaseData::mapCodeXY(int x, int y) const {
	return x >= 0 && x < kBaseMapWidth && y >= 0 && y < kBaseMapHeight ? _map[baseMapIndex(x, y)] : 0;
}

uint16 BaseData::objectCodeAt(int mapPos) const {
	return mapPos >= 0 && mapPos < kBaseMapCellCount ? _objectMap[mapPos] : 0;
}

const BaseBitmap &BaseData::wallBitmap(uint id) const {
	static const BaseBitmap empty;
	return id < _wallBitmaps.size() ? _wallBitmaps[id] : empty;
}

const BaseBitmap &BaseData::objectBitmap(uint id) const {
	static const BaseBitmap empty;
	return id < _objectBitmaps.size() ? _objectBitmaps[id] : empty;
}

const BaseBitmap &BaseData::weaponFrame(uint id) const {
	static const BaseBitmap empty;
	return id < _weaponFrames.size() ? _weaponFrames[id] : empty;
}

const BaseBitmap &BaseData::fontFrame(uint id) const {
	static const BaseBitmap empty;
	return id < _fontFrames.size() ? _fontFrames[id] : empty;
}

int32 BaseData::sinQ16(int angle) const { return _trig[0][normalizeBaseAngle(angle)]; }
int32 BaseData::cosQ16(int angle) const { return _trig[1][normalizeBaseAngle(angle)]; }
int32 BaseData::longTanQ16(int angle) const { return _trig[2][normalizeBaseAngle(angle)]; }
int32 BaseData::longInvTanQ16(int angle) const { return _trig[3][normalizeBaseAngle(angle)]; }
int32 BaseData::invCos(int angle) const { return _trig[4][normalizeBaseAngle(angle)]; }
int32 BaseData::invSin(int angle) const { return _trig[5][normalizeBaseAngle(angle)]; }
int32 BaseData::longCosQ16(int angle) const { return _trig[6][normalizeBaseAngle(angle)]; }
int32 BaseData::xNextQ16(int angle) const { return _xNext[normalizeBaseAngle(angle)]; }
int32 BaseData::yNextQ16(int angle) const { return _yNext[normalizeBaseAngle(angle)]; }
int32 BaseData::viewCosQ16(int column) const { return _viewCos[CLIP(column, 0, kBaseViewWidth - 1)]; }
int32 BaseData::floorCos(int column) const { return _floorCos[CLIP(column, 0, kBaseViewWidth)]; }
int16 BaseData::distanceHeight(int distance) const { return _distanceHeight[CLIP(distance, 0, kBaseMaximumDistance)]; }
int32 BaseData::adjustTable(int distance) const { return _adjust[CLIP(distance, 0, kBaseMaximumDistance)]; }

} // End of namespace Hopkins
