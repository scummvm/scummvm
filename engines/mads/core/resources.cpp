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

#include "common/scummsys.h"
#include "common/archive.h"
#include "common/textconsole.h"
#include "mads/mads.h"
#include "mads/nebular/nebular.h"
#include "mads/core/resources.h"
#include "mads/core/hag.h"

namespace MADS {

void Resources::init(RexNebularEngine *vm) {
	SearchMan.add("HAG", new HagArchive(vm->getGameID(), vm->isDemo()));
}

Common::Path Resources::formatName(RESPREFIX resType, int id, const Common::String &ext) {
	Common::String result = "*";

	if (resType == 3 && !id) {
		id = id / 100;
	}

	if (!ext.empty()) {
		switch (resType) {
		case RESPREFIX_GL:
			result += "GL000";
			break;
		case RESPREFIX_SC:
			result += Common::String::format("SC%.3d", id);
			break;
		case RESPREFIX_RM:
			result += Common::String::format("RM%.3d", id);
			break;
		default:
			break;
		}

		result += ext;
	}

	return Common::Path(result);
}

Common::Path Resources::formatName(int prefix, char asciiCh, int id, EXTTYPE extType,
		const Common::String &suffix) {
	Common::String result;
	if (prefix <= 0) {
		result = "*";
	} else {
		result = Common::String::format("%s%.3d",
			(prefix < 100) ? "*SC" : "*RM", prefix);
	}

	result += Common::String::format("%c", asciiCh);
	if (id >= 0)
		result += Common::String::format("%d", id);
	if (!suffix.empty())
		result += suffix;

	switch (extType) {
	case EXT_SS:
		result += ".SS";
		break;
	case EXT_AA:
		result += ".AA";
		break;
	case EXT_DAT:
		result += ".DAT";
		break;
	case EXT_HH:
		result += ".HH";
		break;
	case EXT_ART:
		result += ".ART";
		break;
	case EXT_INT:
		result += ".INT";
		break;
	default:
		break;
	}

	return Common::Path(result);
}

Common::Path Resources::formatResource(const Common::String &resName,
		const Common::String &hagFilename) {
//	int v1 = 0, v2 = 0;

	if (resName.hasPrefix("*")) {
		// Resource file specified
		error("TODO: formatResource");
	} else {
		// File outside of hag file
		return Common::Path(resName);
	}
}

Common::Path Resources::formatAAName(int idx) {
	return formatName(0, 'I', idx, EXT_AA, "");
}

/*------------------------------------------------------------------------*/

void File::openFile(const Common::Path &filename) {
	if (!Common::File::open(filename))
		error("Could not open file - %s", filename.toString().c_str());
}

/*------------------------------------------------------------------------*/

void SynchronizedList::synchronize(Common::Serializer &s) {
	int v = 0;
	int count = size();
	s.syncAsUint16LE(count);

	if (s.isSaving()) {
		for (int idx = 0; idx < count; ++idx) {
			v = (*this)[idx];
			s.syncAsSint32LE(v);
		}
	} else {
		clear();
		reserve(count);
		for (int idx = 0; idx < count; ++idx) {
			s.syncAsSint32LE(v);
			push_back(v);
		}
	}
}

} // namespace MADS
