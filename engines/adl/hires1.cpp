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
#include "common/memstream.h"
#include "common/ptr.h"

#include "adl/adl.h"
#include "adl/graphics.h"
#include "adl/display_a2.h"

namespace Adl {

#define IDS_HR1_EXE_0    "AUTO LOAD OBJ"
#define IDS_HR1_EXE_1    "ADVENTURE"
#define IDS_HR1_MESSAGES "MESSAGES"

#define IDI_HR1_NUM_ROOMS         41
#define IDI_HR1_NUM_PICS          97
#define IDI_HR1_NUM_VARS          20
#define IDI_HR1_NUM_ITEM_OFFSETS  21
#define IDI_HR1_NUM_MESSAGES     168

// Messages used outside of scripts
#define IDI_HR1_MSG_CANT_GO_THERE      137
#define IDI_HR1_MSG_DONT_UNDERSTAND     37
#define IDI_HR1_MSG_ITEM_DOESNT_MOVE   151
#define IDI_HR1_MSG_ITEM_NOT_HERE      152
#define IDI_HR1_MSG_THANKS_FOR_PLAYING 140
#define IDI_HR1_MSG_DONT_HAVE_IT       127
#define IDI_HR1_MSG_GETTING_DARK         7

#define IDI_HR1_OFS_PD_TEXT_0    0x005d
#define IDI_HR1_OFS_PD_TEXT_1    0x012b
#define IDI_HR1_OFS_PD_TEXT_2    0x016d
#define IDI_HR1_OFS_PD_TEXT_3    0x0259

#define IDI_HR1_OFS_ITEMS        0x0100
#define IDI_HR1_OFS_ROOMS        0x050a
#define IDI_HR1_OFS_PICS         0x4b03
#define IDI_HR1_OFS_CMDS_0       0x3c00
#define IDI_HR1_OFS_CMDS_1       0x3d00
#define IDI_HR1_OFS_MSGS         0x4d00

#define IDI_HR1_OFS_ITEM_OFFSETS 0x68ff
#define IDI_HR1_OFS_SHAPES       0x4f00

class HiRes1Engine : public AdlEngine {
public:
	HiRes1Engine(OSystem *syst, const AdlGameDescription *gd) :
			AdlEngine(syst, gd),
			_files(nullptr),
			_messageDelay(true) { }
	~HiRes1Engine() override { delete _files; }

protected:
	// AdlEngine
	void runIntro() override;
	void init() override;
	void initGameState() override;
	void restartGame();
	void printString(const Common::String &str) override;
	Common::String loadMessage(uint idx) const override;
	void printMessage(uint idx) override;
	void drawItems() override;
	void drawItem(Item &item, const Common::Point &pos) override;
	void loadRoom(byte roomNr) override;
	void showRoom() override;

	void showInstructions(Common::SeekableReadStream &stream);
	void wordWrap(Common::String &str) const;

	Common::Files *_files;
	Common::File _exe;
	Common::Array<Common::DataBlockPtr> _corners;
	Common::Array<byte> _roomDesc;
	bool _messageDelay;

	struct {
		Common::String cantGoThere;
		Common::String dontHaveIt;
		Common::String dontUnderstand;
		Common::String gettingDark;
	} _gameStrings;
};

void HiRes1Engine::showInstructions(Common::SeekableReadStream &stream) {
	_display->setMode(Display::kModeText);

	for (;;) {
		byte opc = stream.readByte();

		if (opc != 0x20)
			error("Error reading instructions");

		uint16 addr = stream.readUint16BE();

		if (addr == 0x58fc) {
			// HOME
			_display->home();
		} else if (addr == 0x6ffd) {
			// GETLN1
			inputString();

			if (shouldQuit())
				return;
		} else {
			// We assume print string call (addr varies per game)
			Common::String str = readString(stream);

			if (stream.err() || stream.eos())
				error("Error reading instructions");

			// Ctrl-D signifies system command (main binary would be loaded here)
			size_t posChr4 = str.findFirstOf(_display->asciiToNative(4));

			if (posChr4 != str.npos) {
				_display->printString(str.substr(0, posChr4));
				return;
			}

			_display->printString(str);
		}
	}
}

void HiRes1Engine::runIntro() {
	Common::StreamPtr stream(_files->createReadStream(IDS_HR1_EXE_0));

	// Early version have no bitmap in 'AUTO LOAD OBJ'
	if (getGameVersion() >= GAME_VER_HR1_COARSE) {
		// Later binaries have a MIXEDON prepended to it, by skipping it
		// we can use the same offsets for both variants
		if (stream->readUint16BE() == 0xad53)
			stream.reset(_files->createReadStream(IDS_HR1_EXE_0, 3));

		stream->seek(0x1000);
		_display->setMode(Display::kModeGraphics);
		static_cast<Display_A2 *>(_display)->loadFrameBuffer(*stream);
		_display->renderGraphics();

		if (getGameVersion() == GAME_VER_HR1_PD) {
			// Only the PD version shows a title screen during the load
			delay(4000);

			if (shouldQuit())
				return;
		}
	}

	Common::String str;

	// Show the PD disclaimer for the PD release
	if (getGameVersion() == GAME_VER_HR1_PD) {
		// The P.D.E. version on the Roberta Williams Anthology has a renamed HELLO file
		const char *fileName = "MYSTERY.HELLO";

		if (!_files->exists(fileName))
			fileName = "HELLO";

		Common::StreamPtr basic(_files->createReadStream(fileName));

		_display->setMode(Display::kModeText);
		_display->home();

		str = readStringAt(*basic, IDI_HR1_OFS_PD_TEXT_0, '"');
		_display->printAsciiString(str + '\r');

		str = readStringAt(*basic, IDI_HR1_OFS_PD_TEXT_1, '"');
		_display->printAsciiString(str + "\r\r");

		str = readStringAt(*basic, IDI_HR1_OFS_PD_TEXT_2, '"');
		_display->printAsciiString(str + "\r\r");

		str = readStringAt(*basic, IDI_HR1_OFS_PD_TEXT_3, '"');
		_display->printAsciiString(str + '\r');

		inputKey();
		if (shouldQuit())
			return;
	}

	_display->setMode(Display::kModeMixed);

	str = readStringAt(*stream, 0xc);

	if (getGameVersion() >= GAME_VER_HR1_COARSE) {
		bool instructions = false;

		char keyInstr = 'I';
		char keyGame = 'G';

		if (getLanguage() == Common::FR_FRA) {
			keyInstr = 'M';
			keyGame = 'J';
		}

		while (1) {
			_display->printString(str);
			Common::String s = inputString();

			if (shouldQuit())
				break;

			if (s.empty())
				continue;

			if (s[0] == _display->asciiToNative(keyInstr)) {
				instructions = true;
				break;
			} else if (s[0] == _display->asciiToNative(keyGame)) {
				break;
			}
		}

		if (instructions) {
			stream->seek(0x5d);
			showInstructions(*stream);
			_display->printAsciiString("\r");
		}
	} else {
		// This version shows the last page during the loading of the game
		// We wait for a key instead (even though there's no prompt for that).
		stream->seek(3);
		showInstructions(*stream);
		inputString();
	}

	stream.reset(_files->createReadStream(IDS_HR1_EXE_1));
	stream->seek(0x1800);
	static_cast<Display_A2 *>(_display)->loadFrameBuffer(*stream);
	_display->renderGraphics();

	_display->setMode(Display::kModeMixed);

	if (getGameVersion() == GAME_VER_HR1_SIMI) {
		// The original waits for the key after initializing the state.
		// This causes it to also wait for a key on a blank screen when
		// a game is restarted. We only wait for a key here during the
		// intro.

		// This does mean we need to push out some extra line feeds to clear the screen
		_display->printString(_strings.lineFeeds);
		inputKey();
	}
}

void HiRes1Engine::init() {
	if (Common::File::exists("ADVENTURE")) {
		_files = new Common::Files_Plain();
	} else {
		Common::Files_AppleDOS *files = new Common::Files_AppleDOS();
		if (!files->open(getDiskImageName(0)))
			error("Failed to open '%s'", getDiskImageName(0).toString(Common::Path::kNativeSeparator).c_str());
		_files = files;
	}

	_graphics = new GraphicsMan_v1<Display_A2>(*static_cast<Display_A2 *>(_display));
	_display->moveCursorTo(Common::Point(0, 23)); // DOS 3.3

	Common::StreamPtr stream(_files->createReadStream(IDS_HR1_EXE_1));

	Common::StringArray exeStrings;
	extractExeStrings(*stream, 0x1576, exeStrings);

	if (exeStrings.size() != 18)
		error("Failed to load strings from executable");

	// Some messages have overrides inside the executable
	_gameStrings.cantGoThere = exeStrings[12];
	_gameStrings.dontHaveIt = exeStrings[13];
	_gameStrings.dontUnderstand = exeStrings[14];
	_gameStrings.gettingDark = exeStrings[15];

	// Load other strings from executable
	_strings.enterCommand = exeStrings[5];
	_strings.verbError = exeStrings[3];
	_strings.nounError = exeStrings[4];
	_strings.playAgain = exeStrings[8];
	_strings.pressReturn = exeStrings[10];
	_strings.lineFeeds = exeStrings[1];

	// Set message IDs
	_messageIds.cantGoThere = IDI_HR1_MSG_CANT_GO_THERE;
	_messageIds.dontUnderstand = IDI_HR1_MSG_DONT_UNDERSTAND;
	_messageIds.itemDoesntMove = IDI_HR1_MSG_ITEM_DOESNT_MOVE;
	_messageIds.itemNotHere = IDI_HR1_MSG_ITEM_NOT_HERE;
	_messageIds.thanksForPlaying = IDI_HR1_MSG_THANKS_FOR_PLAYING;

	if (getLanguage() == Common::FR_FRA) {
		_verbErrorPos = 15;
		_nounErrorPos = 31;
	}

	// Load message offsets
	stream->seek(IDI_HR1_OFS_MSGS);
	for (uint i = 0; i < IDI_HR1_NUM_MESSAGES; ++i)
		_messages.push_back(_files->getDataBlock(IDS_HR1_MESSAGES, stream->readUint16LE()));

	// The French version has 5 additional strings
	if (getLanguage() == Common::FR_FRA)
		for (uint i = 0; i < 5; ++i)
			_messages.push_back(_files->getDataBlock(IDS_HR1_MESSAGES, stream->readUint16LE()));

	// Load picture data from executable
	stream->seek(IDI_HR1_OFS_PICS);
	for (uint i = 1; i <= IDI_HR1_NUM_PICS; ++i) {
		byte block = stream->readByte();
		Common::String name = Common::String::format("BLOCK%i", block);
		uint16 offset = stream->readUint16LE();
		_pictures[i] = _files->getDataBlock(Common::Path(name), offset);
	}

	// Load commands from executable
	stream->seek(IDI_HR1_OFS_CMDS_1);
	readCommands(*stream, _roomCommands);

	stream->seek(IDI_HR1_OFS_CMDS_0);
	readCommands(*stream, _globalCommands);

	// Load dropped item offsets
	stream->seek(IDI_HR1_OFS_ITEM_OFFSETS);
	loadDroppedItemOffsets(*stream, IDI_HR1_NUM_ITEM_OFFSETS);

	// Load shapes
	stream->seek(IDI_HR1_OFS_SHAPES);
	uint16 cornersCount = stream->readUint16LE();
	for (uint i = 0; i < cornersCount; ++i)
		_corners.push_back(_files->getDataBlock(IDS_HR1_EXE_1, IDI_HR1_OFS_SHAPES + stream->readUint16LE()));

	if (stream->eos() || stream->err())
		error("Failed to read game data from '" IDS_HR1_EXE_1 "'");

	stream->seek(getLanguage() == Common::FR_FRA ? 0x900 : 0x3800);
	loadWords(*stream, _verbs, _priVerbs);

	stream->seek(0xf00);
	loadWords(*stream, _nouns, _priNouns);
}

void HiRes1Engine::initGameState() {
	_state.vars.resize(IDI_HR1_NUM_VARS);

	Common::StreamPtr stream(_files->createReadStream(IDS_HR1_EXE_1));

	// Load room data from executable
	_roomDesc.clear();
	stream->seek(IDI_HR1_OFS_ROOMS);
	for (uint i = 0; i < IDI_HR1_NUM_ROOMS; ++i) {
		Room room;
		stream->readByte();
		_roomDesc.push_back(stream->readByte());
		for (uint j = 0; j < 6; ++j)
			room.connections[j] = stream->readByte();
		room.picture = stream->readByte();
		room.curPicture = stream->readByte();
		_state.rooms.push_back(room);
	}

	// Load item data from executable
	stream->seek(IDI_HR1_OFS_ITEMS);
	byte id;
	while ((id = stream->readByte()) != 0xff) {
		Item item;

		item.id = id;
		item.noun = stream->readByte();
		item.room = stream->readByte();
		item.picture = stream->readByte();
		item.isShape = stream->readByte();
		item.position.x = stream->readByte();
		item.position.y = stream->readByte();
		item.state = stream->readByte();
		item.description = stream->readByte();

		stream->readByte();

		byte size = stream->readByte();

		for (uint i = 0; i < size; ++i)
			item.roomPictures.push_back(stream->readByte());

		_state.items.push_back(item);
	}
}

void HiRes1Engine::restartGame() {
	_display->printString(_strings.pressReturn);
	initState();
	_display->printAsciiString(_strings.lineFeeds);
}

void HiRes1Engine::printString(const Common::String &str) {
	Common::String wrap = str;
	wordWrap(wrap);
	_display->printString(wrap);

	if (_messageDelay)
		delay(getLanguage() == Common::FR_FRA ? 2900 : 2250);
}

Common::String HiRes1Engine::loadMessage(uint idx) const {
	const char returnChar = _display->asciiToNative('\r');
	Common::StreamPtr stream(_messages[idx]->createReadStream());
	return readString(*stream, returnChar) + returnChar;
}

void HiRes1Engine::printMessage(uint idx) {
	// In the English version, messages with hardcoded overrides don't delay
	// after printing. It's unclear if this is a bug or not. In most cases
	// the slow drawing of the room will give the player a chance to read
	// it. This isn't the case in ScummVM however, so we add a delay after
	// these messages.

	// In the French version, messages with hardcoded overrides delay
	// based on string length. This leads to overly long delays on longer
	// strings. This might be a bug, since other messages have a fixed
	// delay (that is slightly longer than the English version).
	// We've chosen to stick with fixed delays here as well.

	// NOTE: Later games wait for a key when the text window overflows and
	// don't use delays. It might be better to use that system for this game
	// as well.
	switch (idx) {
	case IDI_HR1_MSG_CANT_GO_THERE:
		_display->printString(_gameStrings.cantGoThere);
		break;
	case IDI_HR1_MSG_DONT_HAVE_IT:
		_display->printString(_gameStrings.dontHaveIt);
		break;
	case IDI_HR1_MSG_DONT_UNDERSTAND:
		_display->printString(_gameStrings.dontUnderstand);
		break;
	case IDI_HR1_MSG_GETTING_DARK:
		_display->printString(_gameStrings.gettingDark);
		break;
	default:
		return printString(loadMessage(idx));
	}

	delay(1500);
}

void HiRes1Engine::drawItems() {
	Common::List<Item>::iterator item;

	uint dropped = 0;

	for (item = _state.items.begin(); item != _state.items.end(); ++item) {
		// Skip items not in this room
		if (item->room != _state.room)
			continue;

		if (item->state == IDI_ITEM_DROPPED) {
			// Draw dropped item if in normal view
			if (getCurRoom().picture == getCurRoom().curPicture)
				drawItem(*item, _itemOffsets[dropped++]);
		} else {
			// Draw fixed item if current view is in the pic list
			Common::Array<byte>::const_iterator pic;

			for (pic = item->roomPictures.begin(); pic != item->roomPictures.end(); ++pic) {
				if (*pic == getCurRoom().curPicture) {
					drawItem(*item, item->position);
					break;
				}
			}
		}
	}
}

void HiRes1Engine::drawItem(Item &item, const Common::Point &pos) {
	if (item.isShape) {
		Common::StreamPtr stream(_corners[item.picture - 1]->createReadStream());
		Common::Point p(pos);
		_graphics->drawShape(*stream, p);
	} else
		drawPic(item.picture, pos);
}

void HiRes1Engine::loadRoom(byte roomNr) {
	_roomData.description = loadMessage(_roomDesc[_state.room - 1]);
}

void HiRes1Engine::showRoom() {
	_state.curPicture = getCurRoom().curPicture;
	_graphics->clearScreen();
	loadRoom(_state.room);

	if (!_state.isDark) {
		drawPic(getCurRoom().curPicture);
		drawItems();
	}

	_display->renderGraphics();
	_messageDelay = false;
	printString(_roomData.description);
	_messageDelay = true;
}

void HiRes1Engine::wordWrap(Common::String &str) const {
	uint end = 39;

	const char spaceChar = _display->asciiToNative(' ');
	const char returnChar = _display->asciiToNative('\r');

	while (1) {
		if (str.size() <= end)
			return;

		while (str[end] != spaceChar)
			--end;

		str.setChar(returnChar, end);
		end += 40;
	}
}

class HiRes1Engine_VF : public HiRes1Engine {
public:
	HiRes1Engine_VF(OSystem *syst, const AdlGameDescription *gd) :
			HiRes1Engine(syst, gd) { }

private:
	// AdlEngine
	void runIntro() override;
	void getInput(uint &verb, uint &noun) override;
};

void HiRes1Engine_VF::getInput(uint &verb, uint &noun) {
	// This version has a modified "parser"
	while (1) {
		_display->printString(_strings.enterCommand);
		const Common::String line = getLine();

		if (shouldQuit() || _isRestoring)
			return;

		uint index = 0;
		Common::String verbString = getWord(line, index);

		if (!_verbs.contains(verbString)) {
			// If the verb is not found and it looks like an imperative, try to build the infinitive
			const size_t ezPos = verbString.find("\xc5\xda"); // "EZ"

			bool found = false;

			if (ezPos != verbString.npos) {
				const char *suf[] = { "\xc5\xd2", "\xc9\xd2", "\xd2\xc5", nullptr }; // "ER", "IR", "RE"

				for (uint i = 0; suf[i]; ++i) {
					verbString.replace(ezPos, 2, suf[i]);
					if (_verbs.contains(verbString)) {
						found = true;
						break;
					}
				}
			}

			if (!found) {
				_display->printString(formatVerbError(verbString));
				continue;
			}
		}

		verb = _verbs[verbString];

		while (1) {
			// Go over all nouns to find one we know. At the end of the string,
			// it will always match the empty word (which is in the noun list).

			// The original has a code path to return a noun error here, but
			// it appears to be non-functional.
			const Common::String nounString = getWord(line, index);

			if (_nouns.contains(nounString)) {
				noun = _nouns[nounString];
				return;
			}
		}
	}
}

void HiRes1Engine_VF::runIntro() {
	Common::StreamPtr stream(_files->createReadStream(IDS_HR1_EXE_0));

	stream->seek(0x1000);

	// Title image is one padding byte short, so we first read it into a buffer
	const uint frameBufferSize = 0x2000;
	byte *const frameBuffer = (byte *)malloc(frameBufferSize);

	if (stream->read(frameBuffer, frameBufferSize - 1) < frameBufferSize - 1)
		error("Failed to read title image");

	// Set missing byte
	frameBuffer[frameBufferSize - 1] = 0;
	Common::MemoryReadStream frameBufferStream(frameBuffer, frameBufferSize, DisposeAfterUse::YES);

	_display->setMode(Display::kModeGraphics);
	static_cast<Display_A2 *>(_display)->loadFrameBuffer(frameBufferStream);
	_display->renderGraphics();

	_display->setMode(Display::kModeMixed);

	Common::String str = readStringAt(*stream, 0xf);

	while (1) {
		_display->printString(str);
		const char key = inputKey();

		if (shouldQuit())
			return;

		if (key == _display->asciiToNative('M')) {
			stream->seek(0x75);
			showInstructions(*stream);
			return;
		} else if (key == _display->asciiToNative('J')) {
			return;
		}
	}
}

Engine *HiRes1Engine_create(OSystem *syst, const AdlGameDescription *gd) {
	if (gd->version == GAME_VER_HR1_VF2)
		return new HiRes1Engine_VF(syst, gd);

	return new HiRes1Engine(syst, gd);
}

} // End of namespace Adl
