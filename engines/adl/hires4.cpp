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

#include "common/system.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/file.h"
#include "common/stream.h"
#include "common/events.h"
#include "common/memstream.h"

#include "adl/adl_v3.h"
#include "adl/detection.h"
#include "adl/display_a2.h"
#include "adl/graphics.h"
#include "adl/disk.h"

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
	HiRes4Engine(OSystem *syst, const AdlGameDescription *gd) :
			AdlEngine_v3(syst, gd),
			_boot(nullptr) { _brokenRooms.push_back(121); }
	~HiRes4Engine() override;

private:
	// AdlEngine
	void runIntro() override;
	void init() override;
	void initGameState() override;

	void putSpace(uint x, uint y) const;
	void drawChar(byte c, Common::SeekableReadStream &shapeTable, Common::Point &pos) const;
	void drawText(const Common::String &str, Common::SeekableReadStream &shapeTable, const float ht, const float vt) const;

	void runIntroAdvise(Common::SeekableReadStream &menu);
	void runIntroLogo(Common::SeekableReadStream &ms2);
	void runIntroTitle(Common::SeekableReadStream &menu, Common::SeekableReadStream &ms2);
	void runIntroInstructions(Common::SeekableReadStream &instructions);
	void runIntroLoading(Common::SeekableReadStream &adventure);

	static const uint kClock = 1022727; // Apple II CPU clock rate

	DiskImage *_boot;
};

// TODO: It might be worth replacing this with a more generic variant that
// can be used in both hires4 and hires6
static Common::MemoryReadStream *readSkewedSectors(DiskImage *disk, byte track, byte sector, byte count) {
	const uint bytesPerSector = disk->getBytesPerSector();
	const uint sectorsPerTrack = disk->getSectorsPerTrack();
	const uint bufSize = count * bytesPerSector;
	byte *const buf = (byte *)malloc(bufSize);
	byte *p = buf;

	while (count-- != 0) {
		StreamPtr stream(disk->createReadStream(track, sector));
		stream->read(p, bytesPerSector);

		if (stream->err() || stream->eos())
			error("Error loading from disk image");

		p += bytesPerSector;
		sector += 5;
		sector %= sectorsPerTrack;
		if (sector == 0)
			++track;
	}

	return new Common::MemoryReadStream(buf, bufSize, DisposeAfterUse::YES);
}

static Common::MemoryReadStream *decodeData(Common::SeekableReadStream &stream, const uint startOffset, uint endOffset, const byte xorVal) {
	assert(stream.size() >= 0);

	uint streamSize(stream.size());

	if (endOffset > streamSize)
		endOffset = streamSize;

	byte *const buf = (byte *)malloc(streamSize);
	stream.read(buf, streamSize);

	if (stream.err() || stream.eos())
		error("Failed to read data for decoding");

	for (uint i = startOffset; i < endOffset; ++i)
		buf[i] ^= xorVal;

	return new Common::MemoryReadStream(buf, streamSize, DisposeAfterUse::YES);
}

HiRes4Engine::~HiRes4Engine() {
	delete _boot;
}

void HiRes4Engine::putSpace(uint x, uint y) const {
	if (shouldQuit())
		return;

	_display->moveCursorTo(Common::Point(x, y));
	_display->printChar(' ');
	_display->renderText();
	delay(2);
}

void HiRes4Engine::drawChar(byte c, Common::SeekableReadStream &shapeTable, Common::Point &pos) const {
	shapeTable.seek(0);
	byte entries = shapeTable.readByte();

	if (c >= entries)
		error("Character %d is not in the shape table", c);

	shapeTable.seek(c * 2 + 2);
	uint16 offset = shapeTable.readUint16LE();

	shapeTable.seek(offset);

	_graphics->drawShape(shapeTable, pos);
}

void HiRes4Engine::drawText(const Common::String &str, Common::SeekableReadStream &shapeTable, const float ht, const float vt) const {
	if (shouldQuit())
		return;

	Common::Point pos((int16)(ht * 7), (int16)(vt * 7.7f));

	drawChar(99, shapeTable, pos);

	for (uint i = 0; i < str.size(); ++i) {
		const byte c = str[i] - 32;

		drawChar(c, shapeTable, pos);
		drawChar(98, shapeTable, pos);

		_display->renderGraphics();
		delay(15);
	}
}

void HiRes4Engine::runIntroAdvise(Common::SeekableReadStream &menu) {
	Common::StringArray backupText;
	backupText.push_back(readStringAt(menu, 0x659, '"'));
	backupText.push_back(readStringAt(menu, 0x682, '"'));
	backupText.push_back(readStringAt(menu, 0x6a9, '"'));
	backupText.push_back(readStringAt(menu, 0x6c6, '"'));

	_display->setMode(Display::kModeText);

	for (uint x = 2; x <= 36; ++x)
		putSpace(x, 2);

	for (uint y = 3; y <= 20; ++y) {
		putSpace(2, y);
		putSpace(36, y);
	}

	for (uint x = 2; x <= 36; ++x)
		putSpace(x, 20);

	for (uint x = 0; x <= 38; ++x)
		putSpace(x, 0);

	for (uint y = 1; y <= 21; ++y) {
		putSpace(0, y);
		putSpace(38, y);
	}

	for (uint x = 0; x <= 38; ++x)
		putSpace(x, 22);

	int y = 7;

	for (uint i = 0; i < backupText.size(); ++i) {
		uint x = 0;

		do {
			if (shouldQuit())
				return;

			++x;

			Common::String left = backupText[i];
			left.erase(x, Common::String::npos);
			Common::String right = backupText[i];
			right.erase(0, right.size() - x);

			_display->moveCursorTo(Common::Point(19 - x, y));
			_display->printAsciiString(left);
			_display->moveCursorTo(Common::Point(19, y));
			_display->printAsciiString(right);
			_display->renderText();
			delay(35);
		} while (x != backupText[i].size() / 2);

		if (i == 2)
			y = 18;
		else
			y += 2;
	}

	Common::String cursor = readStringAt(menu, 0x781, '"');

	uint cursorIdx = 0;
	while (!shouldQuit()) {
		Common::Event event;
		if (pollEvent(event)) {
			if (event.type == Common::EVENT_KEYDOWN)
				break;
		}

		_display->moveCursorTo(Common::Point(32, 18));
		_display->printChar(_display->asciiToNative(cursor[cursorIdx]));
		_display->renderText();
		g_system->delayMillis(25);
		cursorIdx = (cursorIdx + 1) % cursor.size();
	}
}

void HiRes4Engine::runIntroLogo(Common::SeekableReadStream &ms2) {
	Display_A2 *display = static_cast<Display_A2 *>(_display);
	const uint width = display->getGfxWidth();
	const uint height = display->getGfxHeight();
	const uint pitch = display->getGfxPitch();

	display->clear(0x00);
	display->setMode(Display::kModeGraphics);
	byte *logo = new byte[pitch * height];
	display->loadFrameBuffer(ms2, logo);

	for (uint x = 0; x < width; ++x) {
		for (uint y = 0; y < height; ++y) {
			const byte p = logo[y * pitch + x / 7];
			display->setPixelBit(Common::Point(x, y), p);
			if (x % 7 == 6)
				display->setPixelPalette(Common::Point(x, y), p);
		}
		display->renderGraphics();

		if (shouldQuit()) {
			delete[] logo;
			return;
		}

		delay(7);
	}

	delete[] logo;

	for (uint i = 38; i != 0; --i) {
		Common::Point p;

		for (p.y = 1; p.y < (int)height; ++p.y)
			for (p.x = 0; p.x < (int)width; p.x += 7)
				display->setPixelByte(Common::Point(p.x, p.y - 1), display->getPixelByte(p));

		display->renderGraphics();

		Tones tone;
		tone.push_back(Tone(kClock / 2.0 / ((i * 4 + 1) * 10.0 + 10.0), 12.5));
		playTones(tone, false, false);

		if (shouldQuit())
			return;
	}
}

void HiRes4Engine::runIntroTitle(Common::SeekableReadStream &menu, Common::SeekableReadStream &ms2) {
	ms2.seek(0x2290);
	StreamPtr shapeTable(ms2.readStream(0x450));
	if (ms2.err() || ms2.eos())
		error("Failed to read shape table");

	Common::String titleString(readStringAt(menu, 0x1f5, '"'));
	drawText(titleString, *shapeTable, 4.0f, 22.5f);

	titleString = readStringAt(menu, 0x22b, '"');
	drawText(titleString, *shapeTable, 5.0f, 24.0f);

	// Draw "TM" with lines
	_graphics->drawLine(Common::Point(200, 170), Common::Point(200, 174), 0x7f);
	_graphics->drawLine(Common::Point(198, 170), Common::Point(202, 170), 0x7f);
	_display->renderGraphics();
	delay(7);
	_graphics->drawLine(Common::Point(204, 170), Common::Point(204, 174), 0x7f);
	_graphics->drawLine(Common::Point(204, 170), Common::Point(207, 173), 0x7f);
	_graphics->drawLine(Common::Point(207, 173), Common::Point(209, 170), 0x7f);
	_graphics->drawLine(Common::Point(209, 170), Common::Point(209, 174), 0x7f);
	_display->renderGraphics();
	delay(7);

	titleString = readStringAt(menu, 0x46c);
	drawText(titleString, *shapeTable, 20.0f - titleString.size() / 2.0f, 10.6f);

	titleString = readStringAt(menu, 0x490);
	drawText(titleString, *shapeTable, 20.0f - titleString.size() / 2.0f, 11.8f);

	Common::StringArray menuStrings;
	menuStrings.push_back(readStringAt(menu, 0x515));
	menuStrings.push_back(readStringAt(menu, 0x52b));

	for (uint i = 0; i < menuStrings.size(); ++i)
		drawText(Common::String::format("%d) ", i + 1) + menuStrings[i], *shapeTable, 12.5f, 14.0f + i * 1.2f);

	titleString = readStringAt(menu, 0x355, '"');
	drawText(titleString, *shapeTable, 12.5f, 14.0f + menuStrings.size() * 1.2f + 2.0f);
}

void HiRes4Engine::runIntroInstructions(Common::SeekableReadStream &instructions) {
	Common::String line;
	Common::String pressKey(readStringAt(instructions, 0xad6, '"'));
	instructions.seek(0);

	_display->home();
	_display->setMode(Display::kModeText);

	// Search for PRINT commands in tokenized BASIC
	while (1) {
		char c;

		do {
			c = instructions.readByte();

			if (instructions.err() || instructions.eos())
				error("Error reading instructions file");

			// GOSUB (calls "press any key" routine)
			if (c == (char)0xb0) {
				_display->moveCursorTo(Common::Point(6, 23));
				_display->printAsciiString(pressKey);
				inputKey();

				if (shouldQuit())
					return;

				_display->home();
			}
		} while (c != (char)0xba); // PRINT

		uint quotes = 0;
		while (1) {
			c = instructions.readByte();

			if (instructions.err() || instructions.eos())
				error("Error reading instructions file");

			if (c == '"') {
				++quotes;
				continue;
			}

			if (c == 0)
				break;

			if (quotes == 1)
				line += c;
			else if (c == ':') // Separator
				break;
			else if (c == '4') // CTRL-D before "RUN MENU"
				return;
		};

		line += '\r';
		_display->printAsciiString(line);
		line.clear();
	}
}

void HiRes4Engine::runIntroLoading(Common::SeekableReadStream &adventure) {
	_display->home();
	_display->setMode(Display::kModeText);

	const uint kStrings = 4;
	const uint kStringLen = 39;
	char text[kStrings][kStringLen];

	adventure.seek(0x2eb);

	if (adventure.read(text, sizeof(text)) < sizeof(text))
		error("Failed to read loading screen text");

	const uint yPos[kStrings] = { 2, 19, 8, 22 };

	for (uint  i = 0; i < kStrings; ++i) {
		_display->moveCursorTo(Common::Point(0, yPos[i]));
		_display->printString(Common::String(text[i], kStringLen));
	}

	delay(4000);
}

void HiRes4Engine::runIntro() {
	Common::ScopedPtr<Files_AppleDOS> files(new Files_AppleDOS());
	files->open(getDiskImageName(0));

	while (!shouldQuit()) {
		StreamPtr menu(files->createReadStream("MENU"));
		runIntroAdvise(*menu);

		if (shouldQuit())
			return;

		StreamPtr ms2(files->createReadStream("MS2"));
		runIntroLogo(*ms2);

		if (shouldQuit())
			return;

		_graphics->setBounds(Common::Rect(280, 192));
		runIntroTitle(*menu, *ms2);
		_graphics->setBounds(Common::Rect(280, 160));

		while (1) {
			char key = inputKey();

			if (shouldQuit())
				return;

			if (key == _display->asciiToNative('1')) {
				StreamPtr instructions(files->createReadStream("INSTRUCTIONS"));
				runIntroInstructions(*instructions);
				break;
			} else if (key == _display->asciiToNative('2')) {
				StreamPtr adventure(files->createReadStream("THE ADVENTURE"));
				runIntroLoading(*adventure);
				return;
			}
		};
	}
}

void HiRes4Engine::init() {
	_graphics = new GraphicsMan_v2<Display_A2>(*static_cast<Display_A2 *>(_display));

	_boot = new DiskImage();
	if (!_boot->open(getDiskImageName(0)))
		error("Failed to open disk image '%s'", getDiskImageName(0).c_str());

	insertDisk(1);

	StreamPtr stream(readSkewedSectors(_boot, 0x05, 0x6, 1));
	_strings.verbError = readStringAt(*stream, 0x4f);
	_strings.nounError = readStringAt(*stream, 0x8e);
	_strings.enterCommand = readStringAt(*stream, 0xbc);

	stream.reset(readSkewedSectors(_boot, 0x05, 0x3, 1));
	stream->skip(0xd7);
	_strings_v2.time = readString(*stream, 0xff);

	stream.reset(readSkewedSectors(_boot, 0x05, 0x7, 2));
	_strings.lineFeeds = readStringAt(*stream, 0xf8);

	stream.reset(readSkewedSectors(_boot, 0x06, 0xf, 3));
	_strings_v2.saveInsert = readStringAt(*stream, 0x5f);
	_strings_v2.saveReplace = readStringAt(*stream, 0xe5);
	_strings_v2.restoreInsert = readStringAt(*stream, 0x132);
	_strings_v2.restoreReplace = readStringAt(*stream, 0x1c2);
	_strings.playAgain = readStringAt(*stream, 0x225);

	_messageIds.cantGoThere = IDI_HR4_MSG_CANT_GO_THERE;
	_messageIds.dontUnderstand = IDI_HR4_MSG_DONT_UNDERSTAND;
	_messageIds.itemDoesntMove = IDI_HR4_MSG_ITEM_DOESNT_MOVE;
	_messageIds.itemNotHere = IDI_HR4_MSG_ITEM_NOT_HERE;
	_messageIds.thanksForPlaying = IDI_HR4_MSG_THANKS_FOR_PLAYING;

	stream.reset(readSkewedSectors(_boot, 0x0a, 0x0, 5));
	loadMessages(*stream, IDI_HR4_NUM_MESSAGES);

	stream.reset(readSkewedSectors(_boot, 0x05, 0x2, 1));
	stream->skip(0x80);
	loadPictures(*stream);

	stream.reset(readSkewedSectors(_boot, 0x09, 0x2, 1));
	stream->skip(0x05);
	loadItemPictures(*stream, IDI_HR4_NUM_ITEM_PICS);

	stream.reset(readSkewedSectors(_boot, 0x04, 0x0, 3));
	stream->skip(0x15);
	loadItemDescriptions(*stream, IDI_HR4_NUM_ITEM_DESCS);

	stream.reset(readSkewedSectors(_boot, 0x08, 0x2, 6));
	stream->skip(0xa5);
	readCommands(*stream, _roomCommands);

	stream.reset(readSkewedSectors(_boot, 0x04, 0xc, 4));
	stream.reset(decodeData(*stream, 0x218, 0x318, 0xee));
	readCommands(*stream, _globalCommands);

	stream.reset(readSkewedSectors(_boot, 0x06, 0x6, 1));
	stream->skip(0x15);
	loadDroppedItemOffsets(*stream, IDI_HR4_NUM_ITEM_OFFSETS);

	stream.reset(readSkewedSectors(_boot, 0x05, 0x0, 4));
	loadWords(*stream, _verbs, _priVerbs);

	stream.reset(readSkewedSectors(_boot, 0x0b, 0xb, 7));
	loadWords(*stream, _nouns, _priNouns);
}

void HiRes4Engine::initGameState() {
	_state.vars.resize(IDI_HR4_NUM_VARS);

	StreamPtr stream(readSkewedSectors(_boot, 0x0b, 0x9, 10));
	stream->skip(0x0e);
	loadRooms(*stream, IDI_HR4_NUM_ROOMS);

	stream.reset(readSkewedSectors(_boot, 0x0b, 0x0, 13));
	stream.reset(decodeData(*stream, 0x43, 0x143, 0x91));
	loadItems(*stream);
}

class HiRes4Engine_Atari : public AdlEngine_v3 {
public:
	HiRes4Engine_Atari(OSystem *syst, const AdlGameDescription *gd) :
			AdlEngine_v3(syst, gd),
			_boot(nullptr),
			_curDisk(0) { _brokenRooms.push_back(121); }
	~HiRes4Engine_Atari() override;

private:
	// AdlEngine
	void init() override;
	void initGameState() override;
	void loadRoom(byte roomNr) override;
	Common::String formatVerbError(const Common::String &verb) const override;
	Common::String formatNounError(const Common::String &verb, const Common::String &noun) const override;

	// AdlEngine_v2
	void adjustDataBlockPtr(byte &track, byte &sector, byte &offset, byte &size) const override;

	Common::SeekableReadStream *createReadStream(DiskImage *disk, byte track, byte sector, byte offset = 0, byte size = 0) const;
	void loadCommonData();
	void insertDisk(byte diskNr);
	void rebindDisk();

	DiskImage *_boot;
	byte _curDisk;
};

static const char *const atariDisks[] = { "ULYS1A.XFD", "ULYS1B.XFD", "ULYS2C.XFD" };

HiRes4Engine_Atari::~HiRes4Engine_Atari() {
	delete _boot;
}

void HiRes4Engine_Atari::init() {
	_graphics = new GraphicsMan_v2<Display_A2>(*static_cast<Display_A2 *>(_display));

	_boot = new DiskImage();
	if (!_boot->open(atariDisks[0]))
		error("Failed to open disk image '%s'", atariDisks[0]);

	insertDisk(1);
	loadCommonData();

	StreamPtr stream(createReadStream(_boot, 0x06, 0x2));
	_strings.verbError = readStringAt(*stream, 0x4f);
	_strings.nounError = readStringAt(*stream, 0x83);
	_strings.enterCommand = readStringAt(*stream, 0xa6);

	stream.reset(createReadStream(_boot, 0x05, 0xb, 0xd7));
	_strings_v2.time = readString(*stream, 0xff);

	stream.reset(createReadStream(_boot, 0x06, 0x7, 0x00, 2));
	_strings_v2.saveInsert = readStringAt(*stream, 0x62);
	_strings_v2.saveReplace = readStringAt(*stream, 0xdd);
	_strings_v2.restoreInsert = readStringAt(*stream, 0x12a);
	_strings_v2.restoreReplace = readStringAt(*stream, 0x1b8);
	_strings.playAgain = readStringAt(*stream, 0x21b);
	// TODO: restart sequence has "insert side a/b" strings

	_messageIds.cantGoThere = IDI_HR4_MSG_CANT_GO_THERE;
	_messageIds.dontUnderstand = IDI_HR4_MSG_DONT_UNDERSTAND;
	_messageIds.itemDoesntMove = IDI_HR4_MSG_ITEM_DOESNT_MOVE;
	_messageIds.itemNotHere = IDI_HR4_MSG_ITEM_NOT_HERE;
	_messageIds.thanksForPlaying = IDI_HR4_MSG_THANKS_FOR_PLAYING;

	stream.reset(createReadStream(_boot, 0x06, 0xd, 0x12, 2));
	loadItemDescriptions(*stream, IDI_HR4_NUM_ITEM_DESCS);

	stream.reset(createReadStream(_boot, 0x07, 0x1, 0xf4));
	loadDroppedItemOffsets(*stream, IDI_HR4_NUM_ITEM_OFFSETS);

	stream.reset(createReadStream(_boot, 0x08, 0xe, 0xa5, 5));
	readCommands(*stream, _roomCommands);

	stream.reset(createReadStream(_boot, 0x0a, 0x9, 0x00, 3));
	readCommands(*stream, _globalCommands);

	stream.reset(createReadStream(_boot, 0x05, 0x4, 0x00, 3));
	loadWords(*stream, _verbs, _priVerbs);

	stream.reset(createReadStream(_boot, 0x03, 0xb, 0x00, 6));
	loadWords(*stream, _nouns, _priNouns);
}

void HiRes4Engine_Atari::loadRoom(byte roomNr) {
	if (roomNr >= 59 && roomNr < 113) {
		if (_curDisk != 2) {
			insertDisk(2);
			rebindDisk();
		}
	} else if (_curDisk != 1) {
		insertDisk(1);
		rebindDisk();
	}

	AdlEngine_v3::loadRoom(roomNr);
}

Common::String HiRes4Engine_Atari::formatVerbError(const Common::String &verb) const {
	Common::String err = _strings.verbError;
	for (uint i = 0; i < verb.size(); ++i)
		err.setChar(verb[i], i + 8);
	return err;
}

Common::String HiRes4Engine_Atari::formatNounError(const Common::String &verb, const Common::String &noun) const {
	Common::String err = _strings.nounError;
	for (uint i = 0; i < verb.size(); ++i)
		err.setChar(verb[i], i + 8);
	for (uint i = 0; i < noun.size(); ++i)
		err.setChar(noun[i], i + 19);
	return err;
}

void HiRes4Engine_Atari::insertDisk(byte diskNr) {
	if (_curDisk == diskNr)
		return;

	_curDisk = diskNr;

	delete _disk;
	_disk = new DiskImage();
	if (!_disk->open(atariDisks[diskNr]))
		error("Failed to open disk image '%s'", atariDisks[diskNr]);
}

void HiRes4Engine_Atari::rebindDisk() {
	// As room.data is bound to the DiskImage, we need to rebind them here
	// We cannot simply reload the rooms as that would reset their state

	// FIXME: Remove DataBlockPtr-DiskImage coupling?

	StreamPtr stream(createReadStream(_boot, 0x03, 0x1, 0x0e, 9));
	for (uint i = 0; i < IDI_HR4_NUM_ROOMS; ++i) {
		stream->skip(7);
		_state.rooms[i].data = readDataBlockPtr(*stream);
		stream->skip(3);
	}

	// Rebind data that is on both side B and C
	loadCommonData();
}

void HiRes4Engine_Atari::loadCommonData() {
	_messages.clear();
	StreamPtr stream(createReadStream(_boot, 0x0a, 0x4, 0x00, 3));
	loadMessages(*stream, IDI_HR4_NUM_MESSAGES);

	_pictures.clear();
	stream.reset(createReadStream(_boot, 0x05, 0xe, 0x80));
	loadPictures(*stream);

	_itemPics.clear();
	stream.reset(createReadStream(_boot, 0x09, 0xe, 0x05));
	loadItemPictures(*stream, IDI_HR4_NUM_ITEM_PICS);
}

void HiRes4Engine_Atari::initGameState() {
	_state.vars.resize(IDI_HR4_NUM_VARS);

	StreamPtr stream(createReadStream(_boot, 0x03, 0x1, 0x0e, 9));
	loadRooms(*stream, IDI_HR4_NUM_ROOMS);

	stream.reset(createReadStream(_boot, 0x02, 0xc, 0x00, 12));
	loadItems(*stream);

	// FIXME
	_display->moveCursorTo(Common::Point(0, 23));
}

Common::SeekableReadStream *HiRes4Engine_Atari::createReadStream(DiskImage *disk, byte track, byte sector, byte offset, byte size) const {
	adjustDataBlockPtr(track, sector, offset, size);
	return disk->createReadStream(track, sector, offset, size);
}

void HiRes4Engine_Atari::adjustDataBlockPtr(byte &track, byte &sector, byte &offset, byte &size) const {
	// Convert the Apple II disk offsets in the game, to Atari disk offsets
	uint sectorIndex = (track * 16 + sector + 1) << 1;

	// Atari uses 128 bytes per sector vs. 256 on the Apple II
	// Note that size indicates *additional* sectors to read after reading one sector
	size *= 2;

	if (offset >= 128) {
		// Offset in the second half of an Apple II sector, skip one sector and adjust offset
		++sectorIndex;
		offset -= 128;
	} else {
		// Offset in the first half of an Apple II sector, we need to read one additional sector
		++size;
	}

	// Compute track/sector for Atari's 18 sectors per track (sectorIndex is 1-based)
	track = (sectorIndex - 1) / 18;
	sector = (sectorIndex - 1) % 18;
}

Engine *HiRes4Engine_create(OSystem *syst, const AdlGameDescription *gd) {
	switch (getPlatform(*gd)) {
	case Common::kPlatformApple2:
		return new HiRes4Engine(syst, gd);
	case Common::kPlatformAtari8Bit:
		return new HiRes4Engine_Atari(syst, gd);
	default:
		error("Unsupported platform");
	}
}

} // End of namespace Adl
