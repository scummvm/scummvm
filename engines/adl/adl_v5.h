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
	~AdlEngine_v5() override { }

protected:
	AdlEngine_v5(OSystem *syst, const AdlGameDescription *gd);

	// AdlEngine
	void setupOpcodeTables() override;

	// AdlEngine_v4
	RegionChunkType getRegionChunkType(const uint16 addr) const override;
	void initRoomState(RoomState &roomState) const override;
	byte restoreRoomState(byte room) override;

	int o_isNounNotInRoom(ScriptEnv &e) override;
	virtual int o_abortScript(ScriptEnv &e);
	virtual int o_dummy(ScriptEnv &e);
	virtual int o_setTextMode(ScriptEnv &e);
	int o_setRegionRoom(ScriptEnv &e) override;
	int o_setRoomPic(ScriptEnv &e) override;
};

} // End of namespace Adl

#endif
