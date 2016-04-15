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

#ifndef ADL_HIRES6_H
#define ADL_HIRES6_H

#include "common/str.h"

#include "adl/adl_v3.h"
#include "adl/disk.h"

namespace Common {
class ReadStream;
class Point;
}

namespace Adl {

#define IDI_HR6_NUM_ROOMS 35
#define IDI_HR6_NUM_MESSAGES 256
#define IDI_HR6_NUM_VARS 40
#define IDI_HR6_NUM_ITEM_DESCS 15
#define IDI_HR6_NUM_ITEM_PICS 15
#define IDI_HR6_NUM_ITEM_OFFSETS 16

// Messages used outside of scripts
#define IDI_HR6_MSG_CANT_GO_THERE      249
#define IDI_HR6_MSG_DONT_UNDERSTAND    247
#define IDI_HR6_MSG_ITEM_DOESNT_MOVE   253
#define IDI_HR6_MSG_ITEM_NOT_HERE      254
#define IDI_HR6_MSG_THANKS_FOR_PLAYING 252

struct DiskDataDesc {
	byte track;
	byte sector;
	byte offset;
	byte volume;
};

class HiRes6Engine : public AdlEngine_v3 {
public:
	HiRes6Engine(OSystem *syst, const AdlGameDescription *gd) :
			AdlEngine_v3(syst, gd),
			_boot(nullptr),
			_currVerb(0),
			_currNoun(0) {
	}

	~HiRes6Engine() { delete _boot; }

private:
	// AdlEngine
	void runIntro() const;
	void init();
	void initGameState();
	void printRoomDescription();
	void showRoom();

	// AdlEngine_v2
	void printString(const Common::String &str);
	void applyDataBlockOffset(byte &track, byte &sector) const;

	void loadDisk(byte disk);

	DiskImage_DSK *_boot;
	byte _currVerb, _currNoun;
	Common::Array<DiskDataDesc> _diskDataDesc;
};

} // End of namespace Adl

#endif
