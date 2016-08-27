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

#ifndef ADL_ADL_V4_H
#define ADL_ADL_V4_H

#include "adl/adl_v3.h"

namespace Common {
class RandomSource;
}

struct DiskOffset {
	byte track;
	byte sector;
};

namespace Adl {

class AdlEngine_v4 : public AdlEngine_v3 {
public:
	virtual ~AdlEngine_v4() { }

protected:
	AdlEngine_v4(OSystem *syst, const AdlGameDescription *gd);

	// AdlEngine
	virtual void setupOpcodeTables();
	virtual Common::String loadMessage(uint idx) const;
	Common::String getItemDescription(const Item &item) const;

	// AdlEngine_v2
	virtual void adjustDataBlockPtr(byte &track, byte &sector, byte &offset, byte &size) const;

	void applyDiskOffset(byte &track, byte &sector) const;

	int o4_isVarGT(ScriptEnv &e);
	int o4_isItemInRoom(ScriptEnv &e);
	int o4_isNounNotInRoom(ScriptEnv &e);
	int o4_skipOneCommand(ScriptEnv &e);
	int o4_listInv(ScriptEnv &e);
	int o4_moveItem(ScriptEnv &e);
	int o4_dummy(ScriptEnv &e);
	int o4_setTextMode(ScriptEnv &e);
	int o4_setDisk(ScriptEnv &e);
	int o4_sound(ScriptEnv &e);

	byte _curDisk;
	Common::Array<DiskOffset> _diskOffsets;
};

} // End of namespace Adl

#endif
