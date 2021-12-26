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

#include "common/stream.h"
#include "common/archive.h"

#include "engines/grim/update/packfile.h"
#include "engines/grim/update/mscab.h"
#include "engines/grim/update/lang_filter.h"
#include "engines/grim/update/update.h"
#include "engines/grim/grim.h"

namespace Grim {

Common::Archive *loadUpdateArchive(Common::SeekableReadStream *data) {
	Common::SeekableReadStream *updStream = new PackFile(data);
	Common::Archive *cab = new MsCabinet(updStream);
	Common::Archive *update = new LangFilter(cab, g_grim->getGameLanguage());

	Common::ArchiveMemberList list;
	if (update->listMembers(list) == 0) {
		delete update;
		return nullptr;
	} else
		return update;

}

} // end of namespace Grim
