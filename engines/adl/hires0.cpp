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

#include "common/textconsole.h"

#include "adl/adl_v2.h"
#include "adl/graphics.h"
#include "adl/disk.h"

namespace Adl {

#define IDS_HR0_DISK_IMAGE "MISSION.NIB"

#define IDI_HR0_NUM_ROOMS 43
#define IDI_HR0_NUM_MESSAGES 142
#define IDI_HR0_NUM_VARS 40
#define IDI_HR0_NUM_ITEM_PICS 2
#define IDI_HR0_NUM_ITEM_OFFSETS 16

// Messages used outside of scripts
#define IDI_HR0_MSG_CANT_GO_THERE      110
#define IDI_HR0_MSG_DONT_UNDERSTAND    112
#define IDI_HR0_MSG_ITEM_DOESNT_MOVE   114
#define IDI_HR0_MSG_ITEM_NOT_HERE      115
#define IDI_HR0_MSG_THANKS_FOR_PLAYING 113

class HiRes0Engine : public AdlEngine_v2 {
public:
	HiRes0Engine(OSystem *syst, const AdlGameDescription *gd) :
			AdlEngine_v2(syst, gd) { }
	~HiRes0Engine() { }

private:
	// AdlEngine
	void init();
	void initGameState();
};

void HiRes0Engine::init() {
	_graphics = new GraphicsMan_v2(*_display);

	_disk = new DiskImage();
	if (!_disk->open(IDS_HR0_DISK_IMAGE))
		error("Failed to open disk image '" IDS_HR0_DISK_IMAGE "'");

	_disk->setSectorLimit(13);

	// TODO: all these strings/offsets/etc are the same as hires2

	StreamPtr stream(_disk->createReadStream(0x1f, 0x2, 0x00, 2));
	loadMessages(*stream, IDI_HR0_NUM_MESSAGES);

	// Read parser messages
	stream.reset(_disk->createReadStream(0x1a, 0x1));
	_strings.verbError = readStringAt(*stream, 0x4f);
	_strings.nounError = readStringAt(*stream, 0x8e);
	_strings.enterCommand = readStringAt(*stream, 0xbc);

	// Read time string
	stream.reset(_disk->createReadStream(0x19, 0x7, 0xd7));
	_strings_v2.time = readString(*stream, 0xff);

	// Read line feeds
	stream.reset(_disk->createReadStream(0x19, 0xb, 0xf8, 1));
	_strings.lineFeeds = readString(*stream);

	// Read opcode strings
	stream.reset(_disk->createReadStream(0x1a, 0x6, 0x00, 2));
	_strings_v2.saveInsert = readStringAt(*stream, 0x5f);
	_strings_v2.saveReplace = readStringAt(*stream, 0xe5);
	_strings_v2.restoreInsert = readStringAt(*stream, 0x132);
	_strings_v2.restoreReplace = readStringAt(*stream, 0x1c2);
	_strings.playAgain = readStringAt(*stream, 0x225);
	_strings.pressReturn = readStringAt(*stream, 0x25f);

	_messageIds.cantGoThere = IDI_HR0_MSG_CANT_GO_THERE;
	_messageIds.dontUnderstand = IDI_HR0_MSG_DONT_UNDERSTAND;
	_messageIds.itemDoesntMove = IDI_HR0_MSG_ITEM_DOESNT_MOVE;
	_messageIds.itemNotHere = IDI_HR0_MSG_ITEM_NOT_HERE;
	_messageIds.thanksForPlaying = IDI_HR0_MSG_THANKS_FOR_PLAYING;

	// Load global picture data
	stream.reset(_disk->createReadStream(0x19, 0xa, 0x80, 0));
	loadPictures(*stream);

	// Load item picture data
	stream.reset(_disk->createReadStream(0x1e, 0x9, 0x05));
	loadItemPictures(*stream, IDI_HR0_NUM_ITEM_PICS);

	// Load commands from executable
	stream.reset(_disk->createReadStream(0x1d, 0x7, 0x00, 2));
	readCommands(*stream, _roomCommands);

	stream.reset(_disk->createReadStream(0x1f, 0x7, 0x00, 3));
	readCommands(*stream, _globalCommands);

	// Load dropped item offsets
	stream.reset(_disk->createReadStream(0x1b, 0x4, 0x15));
	loadDroppedItemOffsets(*stream, IDI_HR0_NUM_ITEM_OFFSETS);

	// Load verbs
	stream.reset(_disk->createReadStream(0x19, 0x0, 0x00, 3));
	loadWords(*stream, _verbs, _priVerbs);

	// Load nouns
	stream.reset(_disk->createReadStream(0x22, 0x2, 0x00, 2));
	loadWords(*stream, _nouns, _priNouns);
}

void HiRes0Engine::initGameState() {
	_state.vars.resize(IDI_HR0_NUM_VARS);

	StreamPtr stream(_disk->createReadStream(0x21, 0x5, 0x0e, 2));
	loadRooms(*stream, IDI_HR0_NUM_ROOMS);

	stream.reset(_disk->createReadStream(0x21, 0x0));
	loadItems(*stream);
}

Engine *HiRes0Engine_create(OSystem *syst, const AdlGameDescription *gd) {
	return new HiRes0Engine(syst, gd);
}

} // End of namespace Adl
