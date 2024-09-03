/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/system.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/file.h"
#include "common/stream.h"

#include "adl/adl_v2.h"
#include "adl/display_a2.h"
#include "adl/graphics.h"
#include "adl/disk.h"

namespace Adl {

class HiResBaseEngine : public AdlEngine_v2 {
public:
	HiResBaseEngine(OSystem *syst, const AdlGameDescription *gd, const byte numRooms, const byte numMsgs, const byte numItemPics);

private:
	// AdlEngine
	void init() override;
	void initGameState() override;

	const byte _numRooms, _numMsgs, _numItemPics;
};

HiResBaseEngine::HiResBaseEngine(OSystem *syst, const AdlGameDescription *gd, const byte numRooms, const byte numMsgs, const byte numItemPics) :
		AdlEngine_v2(syst, gd),
		_numRooms(numRooms),
		_numMsgs(numMsgs),
		_numItemPics(numItemPics) {

	_messageIds.cantGoThere = 110;
	_messageIds.dontUnderstand = 112;
	_messageIds.itemDoesntMove = 114;
	_messageIds.itemNotHere = 115;
	_messageIds.thanksForPlaying = 113;
}

void HiResBaseEngine::init() {
	_graphics = new GraphicsMan_v2<Display_A2>(*static_cast<Display_A2 *>(_display));

	_disk = new Common::DiskImage();
	if (!_disk->open(getDiskImageName(0)))
		error("Failed to open disk image '%s'", getDiskImageName(0).toString(Common::Path::kNativeSeparator).c_str());

	_disk->setSectorLimit(13);

	Common::StreamPtr stream(_disk->createReadStream(0x1f, 0x2, 0x00, 4));
	loadMessages(*stream, _numMsgs);

	stream.reset(_disk->createReadStream(0x19, 0x0, 0x00, 25, 13));
	Common::StringArray exeStrings;
	extractExeStrings(*stream, 0x1566, exeStrings);
	mapExeStrings(exeStrings);

	// Heuristic to test for early versions that differ slightly
	// Later versions have two additional strings for "INIT DISK"
	const bool oldEngine = exeStrings.size() < 13;

	if (!oldEngine) {
		stream.reset(_disk->createReadStream(0x19, 0x7, 0xd7));
		_strings_v2.time = readString(*stream, 0xff);
	}

	// Load global picture data
	stream.reset(_disk->createReadStream(0x19, 0xa, 0x80, 0));
	loadPictures(*stream);

	// Load item picture data
	stream.reset(_disk->createReadStream(0x1e, 0x9, 0x05));
	loadItemPictures(*stream, _numItemPics);

	// Load commands from executable
	stream.reset(_disk->createReadStream(0x1d, 0x7, 0x00, 4));
	readCommands(*stream, _roomCommands);

	stream.reset(_disk->createReadStream((oldEngine ? 0x19 : 0x1f), 0x7, 0x00, 3));
	readCommands(*stream, _globalCommands);

	// Load dropped item offsets
	stream.reset(_disk->createReadStream(0x1b, 0x4, 0x15));
	loadDroppedItemOffsets(*stream, 16);

	// Load verbs
	stream.reset(_disk->createReadStream(0x19, 0x0, 0x00, 3));
	loadWords(*stream, _verbs, _priVerbs);

	// Load nouns
	stream.reset(_disk->createReadStream(0x22, 0x2, 0x00, 7));
	loadWords(*stream, _nouns, _priNouns);
}

void HiResBaseEngine::initGameState() {
	_state.vars.resize(40);

	Common::StreamPtr stream(_disk->createReadStream(0x21, 0x5, 0x0e, 7));
	loadRooms(*stream, _numRooms);

	stream.reset(_disk->createReadStream(0x21, 0x0, 0x00, 2));
	loadItems(*stream);
}

class HiRes2Engine : public HiResBaseEngine {
public:
	HiRes2Engine(OSystem *syst, const AdlGameDescription *gd);

private:
	// AdlEngine
	void runIntro() override;
};

HiRes2Engine::HiRes2Engine(OSystem *syst, const AdlGameDescription *gd) :
		HiResBaseEngine(syst, gd, 135, 255, 38) {

	_messageIds.cantGoThere = 123;
	_messageIds.dontUnderstand = 19;
	_messageIds.itemDoesntMove = 242;
	_messageIds.itemNotHere = 4;
	_messageIds.thanksForPlaying = 239;
}

void HiRes2Engine::runIntro() {
	// Only the Green Valley version has a title screen
	if (_disk->getSectorsPerTrack() != 16)
		return;

	Common::StreamPtr stream(_disk->createReadStream(0x00, 0xd, 0x17, 1));

	_display->setMode(Display::kModeText);

	Common::String str = readString(*stream);

	if (stream->eos() || stream->err())
		error("Error reading disk image");

	_display->printString(str);
	delay(2000);
}

class HiRes3Engine : public HiResBaseEngine {
public:
	HiRes3Engine(OSystem *syst, const AdlGameDescription *gd);
};

HiRes3Engine::HiRes3Engine(OSystem *syst, const AdlGameDescription *gd) :
		HiResBaseEngine(syst, gd, 138, 255, 36) {

	const byte brokenRooms[] = { 18, 24, 54, 98, 102, 108 };

	for (int i = 0; i < ARRAYSIZE(brokenRooms); ++i)
		_brokenRooms.push_back(brokenRooms[i]);
}

Engine *HiRes2Engine_create(OSystem *syst, const AdlGameDescription *gd) {
	return new HiRes2Engine(syst, gd);
}

Engine *HiRes0Engine_create(OSystem *syst, const AdlGameDescription *gd) {
	return new HiResBaseEngine(syst, gd, 43, 142, 2);
}

Engine *HiRes3Engine_create(OSystem *syst, const AdlGameDescription *gd) {
	return new HiRes3Engine(syst, gd);
}

} // End of namespace Adl
