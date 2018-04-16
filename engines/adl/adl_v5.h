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

#include "adl/adl_v4.h"

namespace Adl {

class AdlEngine_v5 : public AdlEngine_v4 {
public:
	virtual ~AdlEngine_v5() { }

protected:
	AdlEngine_v5(OSystem *syst, const AdlGameDescription *gd);

	// AdlEngine_v4
	virtual RegionChunkType getRegionChunkType(const uint16 addr) const;
	virtual void initRoomState(RoomState &roomState) const;
	virtual byte restoreRoomState(byte room);

	int o5_isNounNotInRoom(ScriptEnv &e);
	int o5_abortScript(ScriptEnv &e);
	int o5_dummy(ScriptEnv &e);
	int o5_setTextMode(ScriptEnv &e);
	int o5_setRegionRoom(ScriptEnv &e);
	int o5_setRoomPic(ScriptEnv &e);
};

} // End of namespace Adl

#endif
