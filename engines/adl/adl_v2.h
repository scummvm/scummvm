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

#ifndef ADL_ADL_V2_H
#define ADL_ADL_V2_H

#include "adl/adl.h"

namespace Adl {

class AdlEngine_v2 : public AdlEngine {
public:
	~AdlEngine_v2() override;

protected:
	AdlEngine_v2(OSystem *syst, const AdlGameDescription *gd);

	// AdlEngine
	void setupOpcodeTables() override;
	void initState() override;
	byte roomArg(byte room) const override;
	void advanceClock() override;
	void printString(const Common::String &str) override;
	Common::String loadMessage(uint idx) const override;
	void drawItems() override;
	void drawItem(Item &item, const Common::Point &pos) override;
	void loadRoom(byte roomNr) override;
	void showRoom() override;
	void takeItem(byte noun) override;

	// Engine
	bool canSaveGameStateCurrently() override;

	void insertDisk(byte volume);
	virtual DataBlockPtr readDataBlockPtr(Common::ReadStream &f) const;
	virtual void adjustDataBlockPtr(byte &track, byte &sector, byte &offset, byte &size) const { }
	void loadItems(Common::ReadStream &stream);
	void loadRooms(Common::ReadStream &stream, byte count);
	void loadMessages(Common::ReadStream &stream, byte count);
	void loadPictures(Common::ReadStream &stream);
	void loadItemPictures(Common::ReadStream &stream, byte count);
	virtual bool isInventoryFull() { return false; }
	int askForSlot(const Common::String &question);

	void checkTextOverflow(char c);
	void handleTextOverflow();

	virtual int o_isFirstTime(ScriptEnv &e);
	virtual int o_isRandomGT(ScriptEnv &e);
	virtual int o_isNounNotInRoom(ScriptEnv &e);
	virtual int o_isCarryingSomething(ScriptEnv &e);

	int o_moveItem(ScriptEnv &e) override;
	int o_setCurPic(ScriptEnv &e) override;
	int o_setPic(ScriptEnv &e) override;
	virtual int o_moveAllItems(ScriptEnv &e);
	int o_save(ScriptEnv &e) override;
	int o_restore(ScriptEnv &e) override ;
	int o_placeItem(ScriptEnv &e) override;
	virtual int o_tellTime(ScriptEnv &e);
	virtual int o_setRoomFromVar(ScriptEnv &e);
	virtual int o_initDisk(ScriptEnv &e);

	struct {
		Common::String time;
		Common::String saveInsert, saveReplace;
		Common::String restoreInsert, restoreReplace;
	} _strings_v2;

	uint _maxLines;
	DiskImage *_disk;
	byte _currentVolume;
	Common::Array<DataBlockPtr> _itemPics;
	bool _itemRemoved;
	byte _roomOnScreen, _picOnScreen, _itemsOnScreen;
	Common::Array<byte> _brokenRooms;
};

} // End of namespace Adl

#endif
