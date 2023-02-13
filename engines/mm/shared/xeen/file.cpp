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

#include "mm/shared/xeen/file.h"
#ifdef ENABLE_XEEN
#include "mm/xeen/files.h"
#include "mm/xeen/xeen.h"
#endif

namespace MM {
namespace Shared {
namespace Xeen {

File::File(const Common::String &filename) {
	File::open(filename);
}

File::File(const Common::String &filename, Common::Archive &archive) {
	File::open(filename, archive);
}

#ifdef ENABLE_XEEN
File::File(const Common::String &filename, int ccMode) {
	File::open(filename, ccMode);
}
#endif

bool File::open(const Common::Path &filename) {
#ifdef ENABLE_XEEN
	MM::Xeen::XeenEngine *engine = dynamic_cast<MM::Xeen::XeenEngine *>(g_engine);

	if (engine) {
		MM::Xeen::FileManager &fm = *engine->_files;

		if (!fm._currentSave || !Common::File::open(filename, *fm._currentSave)) {
			if (!fm._currentArchive || !Common::File::open(filename, *fm._currentArchive)) {
				// Could not find in current archive, so try intro.cc or in folder
				if (!Common::File::open(filename))
					error("Could not open file - %s", filename.toString().c_str());
			}
		}
	} else {
		if (!Common::File::open(filename))
			error("Could not open file - %s", filename.toString().c_str());
	}
#else
	if (!Common::File::open(filename))
		error("Could not open file - %s", filename.toString().c_str());
#endif

	return true;
}

bool File::open(const Common::Path &filename, Common::Archive &archive) {
	if (!Common::File::open(filename, archive))
		error("Could not open file - %s", filename.toString().c_str());
	return true;
}

#ifdef ENABLE_XEEN
bool File::open(const Common::String &filename, int ccMode) {
	MM::Xeen::XeenEngine *engine = dynamic_cast<MM::Xeen::XeenEngine *>(g_engine);
	assert(engine);

	MM::Xeen::FileManager &fm = *engine->_files;
	int oldNum = fm._ccNum;

	fm.setGameCc(ccMode);
	if (File::exists(filename, *fm._currentArchive))
		File::open(filename, *fm._currentArchive);
	else
		File::open(filename);

	fm.setGameCc(oldNum);

	return true;
}

void File::setCurrentArchive(int ccMode) {
	MM::Xeen::XeenEngine *engine = dynamic_cast<MM::Xeen::XeenEngine *>(g_engine);
	assert(engine);
	MM::Xeen::FileManager &fm = *engine->_files;

	switch (ccMode) {
	case 0:
		fm._currentArchive = fm._xeenCc;
		fm._currentSave = fm._xeenSave;
		break;

	case 1:
		fm._currentArchive = fm._darkCc;
		fm._currentSave = fm._darkSave;
		break;

	case 2:
		fm._currentArchive = fm._introCc;
		fm._currentSave = nullptr;
		break;

	default:
		break;
	}

	assert(fm._currentArchive);
}
#endif

Common::String File::readString() {
	Common::String result;
	char c;

	while (pos() < size() && (c = (char)readByte()) != '\0')
		result += c;

	return result;
}

bool File::exists(const Common::String &filename) {
#ifdef ENABLE_XEEN
	MM::Xeen::XeenEngine *engine = dynamic_cast<MM::Xeen::XeenEngine *>(g_engine);

	if (engine) {
		MM::Xeen::FileManager &fm = *engine->_files;

		if (!fm._currentSave || !fm._currentSave->hasFile(filename)) {
			if (!fm._currentArchive->hasFile(filename)) {
				// Could not find in current archive, so try intro.cc or in folder
				return Common::File::exists(filename);
			}
		}

		return true;
	} else {
		return Common::File::exists(filename);
	}
#else
	return Common::File::exists(filename);
#endif
}

#ifdef ENABLE_XEEN
bool File::exists(const Common::String &filename, int ccMode) {
	MM::Xeen::XeenEngine *engine = dynamic_cast<MM::Xeen::XeenEngine *>(g_engine);
	assert(engine);
	MM::Xeen::FileManager &fm = *engine->_files;
	int oldNum = fm._ccNum;

	fm.setGameCc(ccMode);
	bool result = exists(filename);
	fm.setGameCc(oldNum);

	return result;
}
#endif

bool File::exists(const Common::String &filename, Common::Archive &archive) {
	return archive.hasFile(filename);
}

void File::syncBitFlags(Common::Serializer &s, bool *startP, bool *endP) {
	byte data = 0;

	int bitCounter = 0;
	for (bool *p = startP; p < endP; ++p, bitCounter = (bitCounter + 1) % 8) {
		if (bitCounter == 0) {
			if (s.isLoading() || p != startP)
				s.syncAsByte(data);

			if (s.isSaving())
				data = 0;
		}

		if (s.isLoading())
			*p = ((data >> bitCounter) & 1) != 0;
		else if (*p)
			data |= 1 << bitCounter;
	}

	if (s.isSaving())
		s.syncAsByte(data);
}

} // namespace Xeen
} // namespace Shared
} // namespace MM
