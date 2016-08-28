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

#ifndef ADL_HIRES4_H
#define ADL_HIRES4_H

#include "common/str.h"

#include "adl/adl_v3.h"

namespace Adl {

#define IDI_HR4_NUM_ROOMS 164
#define IDI_HR4_NUM_MESSAGES 255
#define IDI_HR4_NUM_VARS 40
#define IDI_HR4_NUM_ITEM_DESCS 44
#define IDI_HR4_NUM_ITEM_PICS 41
#define IDI_HR4_NUM_ITEM_OFFSETS 40

// Messages used outside of scripts
#define IDI_HR4_MSG_CANT_GO_THERE      110
#define IDI_HR4_MSG_DONT_UNDERSTAND    112
#define IDI_HR4_MSG_ITEM_DOESNT_MOVE   114
#define IDI_HR4_MSG_ITEM_NOT_HERE      115
#define IDI_HR4_MSG_THANKS_FOR_PLAYING 113

class HiRes4Engine : public AdlEngine_v3 {
public:
	~HiRes4Engine();

protected:
	HiRes4Engine(OSystem *syst, const AdlGameDescription *gd) :
			AdlEngine_v3(syst, gd),
			_boot(nullptr),
			_curDisk(0) { }

	// AdlEngine
	void init();
	void initGameState();
	void loadRoom(byte roomNr);
	Common::String formatVerbError(const Common::String &verb) const;
	Common::String formatNounError(const Common::String &verb, const Common::String &noun) const;

	Common::SeekableReadStream *createReadStream(DiskImage *disk, byte track, byte sector, byte offset = 0, byte size = 0) const;
	void loadCommonData();
	void insertDisk(byte diskNr);
	void rebindDisk();
	virtual const char *getDiskImageName(byte index) const = 0;

	DiskImage *_boot;
	byte _curDisk;
};

class HiRes4Engine_Atari : public HiRes4Engine {
public:
	HiRes4Engine_Atari(OSystem *syst, const AdlGameDescription *gd) : HiRes4Engine(syst, gd) { }

private:
	// AdlEngine_v2
	void adjustDataBlockPtr(byte &track, byte &sector, byte &offset, byte &size) const;

	// HiRes4Engine
	const char *getDiskImageName(byte index) const;
};

} // End of namespace Adl

#endif
