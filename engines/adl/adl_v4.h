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

namespace Adl {

// Base track/sector for a region
struct RegionLocation {
	byte track;
	byte sector;
};

// Location of the 7 initial data blocks, relative to RegionLocation
struct RegionInitDataOffset {
	byte track;
	byte sector;
	byte offset;
	byte volume;
};

class AdlEngine_v4 : public AdlEngine_v3 {
public:
	~AdlEngine_v4() override;

protected:
	AdlEngine_v4(OSystem *syst, const AdlGameDescription *gd);

	// AdlEngine
	void setupOpcodeTables() override;
	void gameLoop() override;
	void loadState(Common::ReadStream &stream) override;
	void saveState(Common::WriteStream &stream) override;
	Common::String loadMessage(uint idx) const override;
	Common::String getItemDescription(const Item &item) const override;
	void switchRegion(byte region) override;
	void switchRoom(byte roomNr) override;

	// AdlEngine_v2
	void adjustDataBlockPtr(byte &track, byte &sector, byte &offset, byte &size) const override;

	enum RegionChunkType {
		kRegionChunkUnknown,
		kRegionChunkMessages,
		kRegionChunkGlobalPics,
		kRegionChunkVerbs,
		kRegionChunkNouns,
		kRegionChunkRooms,
		kRegionChunkRoomCmds,
		kRegionChunkGlobalCmds
	};

	void loadRegionLocations(Common::ReadStream &stream, uint regions);
	void loadRegionInitDataOffsets(Common::ReadStream &stream, uint regions);
	void initRegions(const byte *roomsPerRegion, uint regions);
	void fixupDiskOffset(byte &track, byte &sector) const;
	virtual RegionChunkType getRegionChunkType(const uint16 addr) const;
	void loadRegion(byte region);
	void loadItemPicIndex(Common::ReadStream &stream, uint items);
	void backupRoomState(byte room);
	virtual void initRoomState(RoomState &roomState) const;
	virtual byte restoreRoomState(byte room);
	void backupVars();
	void restoreVars();

	int o_isItemInRoom(ScriptEnv &e) override;
	virtual int o_isVarGT(ScriptEnv &e);
	int o_moveItem(ScriptEnv &e) override;
	virtual int o_setRegionToPrev(ScriptEnv &e);
	int o_moveAllItems(ScriptEnv &e) override;
	virtual int o_setRegion(ScriptEnv &e);
	int o_save(ScriptEnv &e) override;
	int o_restore(ScriptEnv &e) override;
	int o_restart(ScriptEnv &e) override;
	virtual int o_setRegionRoom(ScriptEnv &e);
	int o_setRoomPic(ScriptEnv &e) override;

	Common::Array<RegionLocation> _regionLocations;
	Common::Array<RegionInitDataOffset> _regionInitDataOffsets;
	Common::SeekableReadStream *_itemPicIndex;
};

} // End of namespace Adl

#endif
