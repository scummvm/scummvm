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

#ifndef MOHAWK_SAVELOAD_H
#define MOHAWK_SAVELOAD_H

#include "common/serializer.h"
#include "common/savefile.h"
#include "common/str.h"
#include "common/memstream.h"

#include "engines/savestate.h"

namespace Mohawk {

class MohawkEngine_Riven;

enum {
	kCDSaveGameVersion = 0x00010000,
	kDVDSaveGameVersion = 0x00010100
};

struct RivenSaveMetadata {
	uint8 saveDay;
	uint8 saveMonth;
	uint16 saveYear;

	uint8 saveHour;
	uint8 saveMinute;

	uint32 totalPlayTime;

	Common::String saveDescription;

	RivenSaveMetadata();
	bool sync(Common::Serializer &s);
};

class RivenSaveLoad {
public:
	RivenSaveLoad(MohawkEngine_Riven*, Common::SaveFileManager*);
	~RivenSaveLoad();

	Common::StringArray generateSaveGameList();
	Common::Error loadGame(Common::String);
	Common::Error saveGame(Common::String);
	void deleteSave(Common::String);

	static SaveStateDescriptor querySaveMetaInfos(const Common::String &filename);
	static Common::String querySaveDescription(const Common::String &filename);

private:
	MohawkEngine_Riven *_vm;
	Common::SaveFileManager *_saveFileMan;

	Common::MemoryWriteStreamDynamic *genNAMESection();
	Common::MemoryWriteStreamDynamic *genMETASection(const Common::String &desc) const;
	Common::MemoryWriteStreamDynamic *genTHMBSection() const;
	Common::MemoryWriteStreamDynamic *genVARSSection();
	Common::MemoryWriteStreamDynamic *genVERSSection();
	Common::MemoryWriteStreamDynamic *genZIPSSection();
};

} // End of namespace Mohawk

#endif
