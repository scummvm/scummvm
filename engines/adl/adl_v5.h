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

#ifndef ADL_ADL_V5_H
#define ADL_ADL_V5_H

#include "adl/adl_v3.h"

namespace Common {
class RandomSource;
}

struct DiskOffset {
	byte track;
	byte sector;
};

namespace Adl {

// FIXME: Subclass _v4 when it is done
class AdlEngine_v5 : public AdlEngine_v3 {
public:
	virtual ~AdlEngine_v5() { }

protected:
	AdlEngine_v5(OSystem *syst, const AdlGameDescription *gd);

	// AdlEngine
	virtual void setupOpcodeTables();
	virtual Common::String loadMessage(uint idx) const;
	Common::String getItemDescription(const Item &item) const;

	// AdlEngine_v2
	virtual void adjustDataBlockPtr(byte &track, byte &sector, byte &offset, byte &size) const;

	void applyDiskOffset(byte &track, byte &sector) const;

	int o5_isVarGT(ScriptEnv &e);
	int o5_isItemInRoom(ScriptEnv &e);
	int o5_isNounNotInRoom(ScriptEnv &e);
	int o5_skipOneCommand(ScriptEnv &e);
	int o5_listInv(ScriptEnv &e);
	int o5_moveItem(ScriptEnv &e);
	int o5_dummy(ScriptEnv &e);
	int o5_setTextMode(ScriptEnv &e);
	int o5_setDisk(ScriptEnv &e);
	int o5_sound(ScriptEnv &e);

	byte _curDisk;
	Common::Array<DiskOffset> _diskOffsets;
};

} // End of namespace Adl

#endif
