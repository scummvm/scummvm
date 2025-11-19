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

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/file.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/framelimiter.h"
#include "graphics/paletteman.h"
#include "image/pcx.h"
#include "image/png.h"

#include "pelrock.h"
#include "pelrock/console.h"
#include "pelrock/detection.h"
#include "pelrock/fonts/small_font.h"
#include "pelrock/offsets.h"
#include "pelrock/pelrock.h"

namespace Pelrock {

PelrockEngine *g_engine;

PelrockEngine::PelrockEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
																				 _gameDescription(gameDesc), _randomSource("Pelrock") {
	g_engine = this;
	_chronoManager = new ChronoManager();
}

PelrockEngine::~PelrockEngine() {
	delete _screen;
	delete _chronoManager;
	for (int i = 0; i < 5; i++) {
		delete[] _cursorMasks[i];
	}
	for (int i = 0; i < kNumVerbIcons; i++) {
		delete[] _verbIcons[i];
	}

	delete[] _popUpBalloon;
	// if (_bgPopupBalloon)
	// 	delete[] _bgPopupBalloon;
	delete _smallFont;
	for (int i = 0; i < 4; i++) {

		// free all frame buffers
		for (int j = 0; j < walkingAnimLengths[i]; j++) {
			delete[] walkingAnimFrames[i][j];
		}

		// free the array of pointers
		delete[] walkingAnimFrames[i];
	}
}

uint32 PelrockEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String PelrockEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error PelrockEngine::run() {
	// Initialize 320x200 paletted graphics mode
	initGraphics(640, 400);
	_screen = new Graphics::Screen();

	// Set the engine's debugger console
	setDebugger(new Console());

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	// Simple event handling loop
	Common::Event e;
	Graphics::FrameLimiter limiter(g_system, 60);

	init();
	if (shouldPlayIntro == false) {
		stateGame = GAME;
	} else {
		stateGame = INTRO;
		playIntro();
	}

	while (!shouldQuit()) {
		_chronoManager->updateChrono();
		while (g_system->getEventManager()->pollEvent(e)) {
			if (e.type == Common::EVENT_KEYDOWN) {
				if (e.kbd.keycode == Common::KEYCODE_w) {
					isAlfredWalking = true;
					isAlfredTalking = false;
				} else if (e.kbd.keycode == Common::KEYCODE_t) {
					isAlfredWalking = false;
					isAlfredTalking = true;
				} else if (e.kbd.keycode == Common::KEYCODE_s) {
					isAlfredWalking = false;
					isAlfredTalking = false;
				}
			} else if (e.type == Common::EVENT_MOUSEMOVE) {
				mouseX = e.mouse.x;
				mouseY = e.mouse.y;
				// debug(3, "Mouse moved to (%d,%d)", mouseX, mouseY);
			} else if (e.type == Common::EVENT_LBUTTONDOWN) {
				_mouseDownTime = g_system->getMillis();
				_isMouseDown = true;
			} else if (e.type == Common::EVENT_LBUTTONUP) {
				if (_isMouseDown) {
					uint32 clickDuration = g_system->getMillis() - _mouseDownTime;
					if (clickDuration >= kLongClickDuration) {
						checkLongMouseClick(e.mouse.x, e.mouse.y);
					} else {
						checkMouseClick(e.mouse.x, e.mouse.y);
					}
					_isMouseDown = false;
				}
			}
		}
		checkMouseHover();
		frames();
		_screen->update();
		// limiter.delayBeforeSwap();
		// limiter.startFrame();
	}

	return Common::kNoError;
}

void PelrockEngine::init() {
	loadCursors();
	loadInteractionIcons();

	_compositeBuffer = new byte[640 * 400];
	_currentBackground = new byte[640 * 400];

	_smallFont = new SmallFont();
	_smallFont->load("ALFRED.4");
	_largeFont = new LargeFont();
	_largeFont->load("ALFRED.7");

	changeCursor(DEFAULT);
	CursorMan.showMouse(true);
	if (gameInitialized == false) {
		gameInitialized = true;
		loadAnims();
		setScreen(0, 2);
		// setScreen(2, 2);
		// setScreen(28, 0);
	}
}

void PelrockEngine::playIntro() {
}

void PelrockEngine::loadAnims() {
	loadAlfredAnims();
}

const int EXPECTED_SIZE = 640 * 400;
size_t rleDecompress(const uint8_t *data, size_t data_size, uint32_t offset, uint32_t size, uint8_t **out_data) {
	// Check for uncompressed markers
	if (size == 0x8000 || size == 0x6800) {
		*out_data = (uint8_t *)malloc(size);
		memcpy(*out_data, data + offset, size);
		return size;
	}

	// RLE compressed
	*out_data = (uint8_t *)malloc(EXPECTED_SIZE * 2); // Allocate enough space
	size_t result_size = 0;

	uint32_t pos = offset;
	uint32_t end = offset + size;

	while (pos + 2 <= end && pos + 2 <= data_size) {
		// Check for BUDA marker
		if (pos + 4 <= data_size &&
			data[pos] == 'B' && data[pos + 1] == 'U' &&
			data[pos + 2] == 'D' && data[pos + 3] == 'A') {
			break;
		}

		uint8_t count = data[pos];
		uint8_t value = data[pos + 1];

		for (int i = 0; i < count; i++) {
			(*out_data)[result_size++] = value;
		}

		pos += 2;
	}

	return result_size;
}

void PelrockEngine::getPalette(Common::File *roomFile, int roomOffset, byte *palette) {
	// get palette
	int paletteOffset = roomOffset + (11 * 8);
	roomFile->seek(paletteOffset, SEEK_SET);
	uint32 offset = roomFile->readUint32LE();
	uint32 size = roomFile->readUint32LE();

	roomFile->seek(offset, SEEK_SET);

	roomFile->read(palette, size);
	for (int i = 0; i < 256; i++) {
		palette[i * 3] = palette[i * 3] << 2;
		palette[i * 3 + 1] = palette[i * 3 + 1] << 2;
		palette[i * 3 + 2] = palette[i * 3 + 2] << 2;
	}
}

void PelrockEngine::getBackground(Common::File *roomFile, int roomOffset, byte *background) {
	roomFile->seek(0, SEEK_SET);
	// get screen
	size_t combined_size = 0;
	size_t uncompressed_size = 0;
	for (int pair_idx = 0; pair_idx < 8; pair_idx++) {
		uint32_t pair_offset = roomOffset + (pair_idx * 8);
		if (pair_offset + 8 > roomFile->size())
			continue;

		roomFile->seek(pair_offset, SEEK_SET);
		uint32_t offset = roomFile->readUint32LE();
		uint32_t size = roomFile->readUint32LE();
		uncompressed_size += size;

		if (offset > 0 && size > 0 && offset < roomFile->size()) {
			byte *data = new byte[size];
			roomFile->seek(offset, SEEK_SET);
			roomFile->read(data, size);
			uint8_t *block_data = NULL;
			size_t block_size = rleDecompress(data, size, 0, size, &block_data);

			memcpy(background + combined_size, block_data, block_size);
			combined_size += block_size + 1;
			free(block_data);
			delete[] data;
		}
	}
}

Common::Array<AnimSet> PelrockEngine::loadRoomAnimations(Common::File *roomFile, int roomOffset) {
	uint32_t pair_offset = roomOffset + (8 * 8);
	// debug("Sprite pair offset position: %d", pair_offset);
	roomFile->seek(pair_offset, SEEK_SET);
	uint32_t offset = roomFile->readUint32LE();
	uint32_t size = roomFile->readUint32LE();

	byte *data = new byte[size];
	roomFile->seek(offset, SEEK_SET);
	roomFile->read(data, size);

	unsigned char *pic = new byte[10000 * 10000];
	if (offset > 0 && size > 0) {
		rleDecompress(data, size, 0, size, &pic);
	} else {
		return Common::Array<AnimSet>();
	}
	Common::Array<AnimSet> anims = Common::Array<AnimSet>();
	uint32_t spriteEnd = offset + size;

	uint32_t pair10_offset_pos = roomOffset + (10 * 8);
	uint32_t metadata_start = spriteEnd + 108;
	uint32_t picOffset = 0;
	for (int i = 0; i < 7; i++) {
		uint32_t animOffset = metadata_start + (i * 44);
		byte *animData = new byte[44];
		roomFile->seek(animOffset, SEEK_SET);
		roomFile->read(animData, 44);
		AnimSet animSet;
		animSet.x = animData[0] | (animData[1] << 8);
		animSet.y = animData[2] | (animData[3] << 8);
		animSet.w = animData[4];
		animSet.h = animData[5];
		animSet.extra = animData[6];
		// roomFile->skip(1); // reserved
		animSet.numAnims = animData[8];
		animSet.spriteType = animData[33];
		animSet.actionFlags = animData[34];
		animSet.isDisabled = animData[38];
		if (animSet.numAnims == 0) {
			break;
		}
		animSet.animData = new Anim[animSet.numAnims];
		// debug("AnimSet %d has %d sub-anims, type %d, actionFlags %d, isDisabled? %d", i, animSet.numAnims, animSet.spriteType, animSet.actionFlags, animSet.isDisabled);
		int subAnimOffset = 10;
		for (int j = 0; j < animSet.numAnims; j++) {

			Anim anim;
			anim.x = animSet.x;
			anim.y = animSet.y;
			anim.w = animSet.w;
			anim.h = animSet.h;
			anim.curFrame = 0;

			anim.nframes = animData[subAnimOffset + j];
			anim.loopCount = animData[subAnimOffset + 4 + j];
			anim.speed = animData[subAnimOffset + 8 + j];
			anim.animData = new byte[anim.nframes];
			if (anim.w > 0 && anim.h > 0 && anim.nframes > 0) {
				uint32_t needed = anim.w * anim.h * anim.nframes;
				anim.animData = new byte[needed];
				Common::copy(pic + picOffset, pic + picOffset + needed, anim.animData);
				animSet.animData[j] = anim;
				// debug("  Anim %d-%d: x=%d y=%d w=%d h=%d nframes=%d loopCount=%d speed=%d", i, j, anim.x, anim.y, anim.w, anim.h, anim.nframes, anim.loopCount, anim.speed);
				picOffset += needed;
			} else {
				continue;
				debug("Anim %d-%d: invalid dimensions, skipping", i, j);
			}
			animSet.animData[j] = anim;
		}

		anims.push_back(animSet);

		// if (w > 0 && h > 0 && frames > 0) {
		// 	AnimSet anim;
		// 	anim.x = x;
		// 	anim.y = y;
		// 	anim.w = w;
		// 	anim.h = h;
		// 	anim.numAnims = frames;
		// 	uint32_t needed = anim.w * anim.h * anim.nframes;
		// 	anim.animData = new byte[needed];
		// 	Common::copy(pic + picOffset, pic + picOffset + needed, anim.animData);
		// 	picOffset += needed;
		// 	debug("Anim %d: x=%d y=%d w=%d h=%d nframes=%d", i, anim.x, anim.y, anim.w, anim.h, anim.nframes);
		// 	anims.push_back(anim);
		// }
	}
	return anims;
}

Common::List<WalkBox> PelrockEngine::loadWalkboxes(Common::File *roomFile, int roomOffset) {
	uint32_t pair10_offset_pos = roomOffset + (10 * 8);
	roomFile->seek(pair10_offset_pos, SEEK_SET);
	// roomFile->skip(4);
	uint32_t pair10_data_offset = roomFile->readUint32LE();
	uint32_t pair10_size = roomFile->readUint32LE();

	uint32_t walkbox_countOffset = pair10_data_offset + 0x213;
	roomFile->seek(walkbox_countOffset, SEEK_SET);
	byte walkbox_count = roomFile->readByte();
	debug("Walkbox count: %d", walkbox_count);
	uint32_t walkbox_offset = pair10_data_offset + 0x218;
	Common::List<WalkBox> walkboxes;
	for (int i = 0; i < walkbox_count; i++) {
		uint32_t box_offset = walkbox_offset + i * 9;
		roomFile->seek(box_offset, SEEK_SET);
		int16 x1 = roomFile->readSint16LE();
		int16 y1 = roomFile->readSint16LE();
		int16 w = roomFile->readSint16LE();
		int16 h = roomFile->readSint16LE();
		byte flags = roomFile->readByte();
		debug("Walkbox %d: x1=%d y1=%d w=%d h=%d", i, x1, y1, w, h);
		WalkBox box;
		box.x = x1;
		box.y = y1;
		box.w = w;
		box.h = h;
		box.flags = flags;
		walkboxes.push_back(box);
	}
	return walkboxes;
}

Common::Array<Description> PelrockEngine::loadRoomDescriptions(Common::File *roomFile, int roomOffset, uint32_t &outPos) {
	uint32_t pair12_offset_pos = roomOffset + (12 * 8);
	roomFile->seek(pair12_offset_pos, SEEK_SET);
	// roomFile->skip(4);
	uint32_t pair12_data_offset = roomFile->readUint32LE();
	uint32_t pair12_size = roomFile->readUint32LE();

	roomFile->seek(pair12_data_offset, SEEK_SET);
	byte *data = new byte[pair12_size];
	roomFile->read(data, pair12_size);
	Common::Array<Description> descriptions;
	uint32_t pos = 0;
	uint32_t lastDescPos = 0;
	while (pos < (pair12_size)) {
		// char *desc = new char[256];
		int desc_pos = 0;
		if (data[pos] == 0xFF) {
			Description description;
			description.itemId = data[++pos];
			pos += 2;
			description.index = data[pos++];
			description.text = "";
			// debug("Found description terminator");
			while (pos < (pair12_size) && data[pos] != 0xFD && pos < (pair12_size)) {
				// debug(" char: %c", data[pos]);
				if (data[pos] != 0x00) {
					description.text.append(1, (char)data[pos]);
				}
				// desc[desc_pos++] = (char)data[pos];
				// debug("Current desc: %s", desc);
				pos++;
			}
			debug("Found description for item %d index %d, text: %s", description.itemId, description.index, description.text.c_str());

			descriptions.push_back(description);
			lastDescPos = pos;
		}
		pos++;
	}
	debug("End of descriptions at position %d", pos);
	outPos = lastDescPos + 1;
	delete[] data;
	// for (Common::List<Common::String>::iterator i = descriptions.begin(); i != descriptions.end(); i++) {
	// 	debug("Room description: %s", i->c_str());
	// }
	return descriptions;
}

/**
 * def decode_byte(b):
	"""Decode a byte to character"""
	special = {
		0x80: 'ñ', 0x81: 'í', 0x82: '¡', 0x83: '¿', 0x84: 'ú',
		0x7B: 'á', 0x7C: 'é', 0x7D: 'í', 0x7E: 'ó', 0x7F: 'ú',
	}

	if b in special:
		return special[b]
	elif 0x20 <= b <= 0x7A:
		return chr(b)
	else:
		return f'[{b:02X}]'
 */
char32_t decodeByte(byte b) {
	if (b == 0x80) {
		return '\xA4';
	} else if (b == 0x81) {
		return '\xA1';
	} else if (b == 0x82) {
		return '\xAD';
	} else if (b == 0x83) {
		return '\xA8';
	} else if (b == 0x84) {
		return '\xA3';
	} else if (b == 0x7B) {
		return '\xA0';
	} else if (b == 0x7C) {
		return '\x82';
	} else if (b == 0x7D) {
		return '\xA1';
	} else if (b == 0x7E) {
		return '\xA2';
	} else if (b == 0x7F) {
		return '\xA3';
	} else if (b >= 0x20 && b <= 0x7A) {
		return (char)b;
	} else {
		// return string in format [XX]
		return '.';
	}
}

void PelrockEngine::talk() {
	if (_currentRoomConversations.size() == 0)
		return;
	int x = _currentRoomHotspots[0].x;
	int y = _currentRoomHotspots[0].y;
	debug("Say %s", _currentRoomConversations[0].text.c_str());
	// showDescription(_currentRoomConversations[0].text, x, y, _currentRoomConversations[0].speakerId);
	// for(int i = 0; i < _currentRoomConversations[0].choices.size(); i++) {
	// 	int idx = _currentRoomConversations.size() - 1 - i;
	// 	_smallFont->drawString(_screen, _currentRoomConversations[0].choices[idx].text.c_str(), 0, 400 - ((i + 1) * 12), 640, 14);
	// }
}

Common::String PelrockEngine::getControlName(byte b) {
	switch (b) {
	case 0xFD:
		return "END_LINE";
	case 0xFC:
		return "TEXT_TERM";
	case 0xFB:
		return "CHOICE";
	case 0xFA:
		return "SKIP";
	case 0xF9:
		return "PAGE_BREAK";
	case 0xF8:
		return "ACTION";
	case 0xF7:
		return "END_BRANCH";
	case 0xF6:
		return "LINE_CONT";
	case 0xF5:
		return "END_BRANCH_2";
	case 0xF4:
		return "END_CONV";
	case 0xF1:
		return "CHOICE_ALT";
	case 0xF0:
		return "GO_BACK";
	case 0xFE:
		return "END_BRANCH_3";
	case 0xEB:
		return "END_ALT";
	case 0xFF:
		return "DESC_START";
	case 0x08:
		return "SPEAKER";
	default:
		return Common::String::format("UNKNOWN(0x%02X)", b);
	}
}

Common::String PelrockEngine::cleanText(const Common::String &text) {
	Common::String cleaned = text;

	// Trim leading/trailing whitespace
	while (!cleaned.empty() && Common::isSpace(cleaned.firstChar())) {
		cleaned.deleteChar(0);
	}
	while (!cleaned.empty() && Common::isSpace(cleaned.lastChar())) {
		cleaned.deleteLastChar();
	}

	// Remove leading [XX][00] patterns
	while (!cleaned.empty() && cleaned.contains('[')) {
		uint idx = 0;
		for (uint i = 0; i < cleaned.size() && i < 15; i++) {
			if (cleaned[i] == '[') {
				idx = i;
				break;
			}
		}

		if (idx < 10) {
			int endIdx = -1;
			for (uint i = idx; i < cleaned.size() && i < idx + 10; i++) {
				if (cleaned[i] == ']') {
					endIdx = i;
					break;
				}
			}

			if (endIdx > (int)idx && endIdx < (int)idx + 10) {
				cleaned = cleaned.c_str() + endIdx + 1;
				// Trim leading whitespace again
				while (!cleaned.empty() && Common::isSpace(cleaned.firstChar())) {
					cleaned.deleteChar(0);
				}
			} else {
				break;
			}
		} else {
			break;
		}
	}

	// Remove single leading control characters
	if (cleaned.size() > 1) {
		byte first = (byte)cleaned[0];
		byte second = (byte)cleaned[1];

		if ((first == 'A' || first == 'H') &&
			(Common::isUpper(second) || second == 0x83 || second == 0x82 || second == '[')) {
			cleaned.deleteChar(0);
			while (!cleaned.empty() && Common::isSpace(cleaned.firstChar())) {
				cleaned.deleteChar(0);
			}
		} else if (strchr("#%')!+,.-\"*&$(/", first)) {
			cleaned.deleteChar(0);
			while (!cleaned.empty() && Common::isSpace(cleaned.firstChar())) {
				cleaned.deleteChar(0);
			}
		}
	}

	return cleaned;
}

Common::Array<ConversationElement> PelrockEngine::parseConversationElements(const byte *convData, uint32 size) {
	Common::Array<ConversationElement> elements;
	Common::HashMap<int, int> choiceIndices; // Track choice index occurrences
	uint32 pos = 0;

	// First pass: parse elements and track choice indices
	while (pos < size) {
		byte b = convData[pos];

		if (b == 0x08) { // SPEAKER
			pos++;
			if (pos < size) {
				byte speakerId = convData[pos];
				Common::String speaker = (speakerId == 0x0D) ? "ALFRED" : "NPC";
				pos++;

				// Read text
				Common::String text;
				while (pos < size && convData[pos] != 0x08 && convData[pos] != 0xFB &&
					   convData[pos] != 0xF1 && convData[pos] != 0xF8 && convData[pos] != 0xFD &&
					   convData[pos] != 0xFC && convData[pos] != 0xF4 && convData[pos] != 0xF7 &&
					   convData[pos] != 0xF5 && convData[pos] != 0xFE && convData[pos] != 0xEB &&
					   convData[pos] != 0xF0) {
					char32_t ch = decodeByte(convData[pos]);
					if (ch != '.') {
						text += ch;
					}
					pos++;
				}

				text = cleanText(text);
				if (!text.empty()) {
					ConversationElement elem;
					elem.type = ConversationElement::DIALOGUE;
					elem.speakerId = speakerId;
					elem.speaker = speaker;
					elem.text = text;
					elem.choiceIndex = -1;
					elements.push_back(elem);
				}
			}
		} else if (b == 0xFB || b == 0xF1) { // CHOICE marker
			pos++;
			int choiceIndex = -1;
			if (pos < size) {
				choiceIndex = convData[pos];
				// Track this choice index
				if (choiceIndices.contains(choiceIndex)) {
					choiceIndices[choiceIndex]++;
				} else {
					choiceIndices[choiceIndex] = 1;
				}
				pos++;
			}

			// Skip next 2 bytes (speaker marker)
			if (pos < size)
				pos++;
			if (pos < size)
				pos++;

			// Read text
			Common::String text;
			while (pos < size && convData[pos] != 0x08 && convData[pos] != 0xFB &&
				   convData[pos] != 0xF1 && convData[pos] != 0xF8 && convData[pos] != 0xFD &&
				   convData[pos] != 0xFC && convData[pos] != 0xF4 && convData[pos] != 0xF7 &&
				   convData[pos] != 0xF5 && convData[pos] != 0xFE && convData[pos] != 0xEB &&
				   convData[pos] != 0xF0) {
				char32_t ch = decodeByte(convData[pos]);
				if (ch != '.') {
					text += ch;
				}
				pos++;
			}

			text = cleanText(text);
			if (!text.empty()) {
				ConversationElement elem;
				elem.type = ConversationElement::CHOICE_MARKER;
				elem.text = text;
				elem.choiceIndex = choiceIndex;
				elements.push_back(elem);
			}
		} else if (b == 0xF8) { // ACTION
			pos += 3;
		} else if (b == 0xF4) { // END_CONV
			ConversationElement elem;
			elem.type = ConversationElement::END_CONV;
			elements.push_back(elem);
			pos++;
		} else if (b == 0xF7) { // END_BRANCH
			ConversationElement elem;
			elem.type = ConversationElement::END_BRANCH;
			elements.push_back(elem);
			pos++;
		} else if (b == 0xFD || b == 0xFC || b == 0xF5 || b == 0xFE || b == 0xEB || b == 0xF0) {
			pos++;
		} else {
			pos++;
		}
	}

	// Second pass: mark which indices are actual choices (appear multiple times)
	for (uint i = 0; i < elements.size(); i++) {
		if (elements[i].choiceIndex >= 0) {
			elements[i].isRealChoice = (choiceIndices[elements[i].choiceIndex] > 1);
		}
	}

	return elements;
}

Common::Array<ConversationNode> PelrockEngine::buildTreeStructure(const Common::Array<ConversationElement> &elements) {
	Common::Array<ConversationNode> roots;
	Common::Array<StackEntry> stack;
	ConversationNode *currentRoot = nullptr;
	uint i = 0;

	while (i < elements.size()) {
		const ConversationElement &elem = elements[i];

		if (elem.type == ConversationElement::DIALOGUE && elem.speaker == "NPC") {
			if (stack.empty()) {
				// New root conversation
				ConversationNode root;
				root.type = ConversationNode::ROOT;
				root.text = elem.text;
				roots.push_back(root);
				currentRoot = &roots[roots.size() - 1];
			} else {
				// NPC response within a branch
				ConversationNode *parent = stack[stack.size() - 1].node;
				ConversationNode response;
				response.type = ConversationNode::RESPONSE;
				response.speaker = "NPC";
				response.speakerId = elem.speakerId;
				response.text = elem.text;
				parent->responses.push_back(response);
			}
			i++;

		} else if (elem.type == ConversationElement::CHOICE_MARKER) {
			if (elem.isRealChoice) {
				// Real choice - player selects from menu
				ConversationNode choiceNode;
				choiceNode.type = ConversationNode::CHOICE;
				choiceNode.text = elem.text;
				choiceNode.choiceIndex = elem.choiceIndex;

				// Find where to attach this choice
				while (!stack.empty() && stack[stack.size() - 1].index >= elem.choiceIndex) {
					stack.pop_back();
				}

				if (!stack.empty()) {
					ConversationNode *parent = stack[stack.size() - 1].node;
					parent->subchoices.push_back(choiceNode);

					// Get pointer to the newly added choice
					ConversationNode *newChoice = &parent->subchoices[parent->subchoices.size() - 1];

					StackEntry entry;
					entry.node = newChoice;
					entry.index = elem.choiceIndex;
					stack.push_back(entry);
				} else {
					if (currentRoot) {
						currentRoot->choices.push_back(choiceNode);

						// Get pointer to the newly added choice
						ConversationNode *newChoice = &currentRoot->choices[currentRoot->choices.size() - 1];

						StackEntry entry;
						entry.node = newChoice;
						entry.index = elem.choiceIndex;
						stack.push_back(entry);
					}
				}
			} else {
				// Auto-dialogue - ALFRED just speaks
				if (!stack.empty()) {
					ConversationNode *parent = stack[stack.size() - 1].node;
					ConversationNode response;
					response.type = ConversationNode::RESPONSE;
					response.speaker = "ALFRED";
					response.speakerId = 0x0D;
					response.text = elem.text;
					parent->responses.push_back(response);
				}
			}
			i++;

		} else if (elem.type == ConversationElement::DIALOGUE && elem.speaker == "ALFRED") {
			if (!stack.empty()) {
				ConversationNode *parent = stack[stack.size() - 1].node;
				ConversationNode response;
				response.type = ConversationNode::RESPONSE;
				response.speaker = "ALFRED";
				response.text = elem.text;
				response.speakerId = 0x0D;
				parent->responses.push_back(response);
			}
			i++;

		} else if (elem.type == ConversationElement::END_CONV) {
			if (!stack.empty()) {
				stack[stack.size() - 1].node->terminated = true;
				stack.pop_back();
			}
			i++;

		} else if (elem.type == ConversationElement::END_BRANCH) {
			stack.clear();
			currentRoot = nullptr;
			i++;

		} else {
			i++;
		}
	}

	return roots;
}

Common::Array<ConversationNode> PelrockEngine::loadConversations(Common::File *roomFile, int roomOffset, uint32_t startPos) {

	debug("Loading conversations starting at position %d", startPos);

	uint32_t pair12_offset_pos = roomOffset + (12 * 8);
	roomFile->seek(pair12_offset_pos, SEEK_SET);
	uint32_t pair12_data_offset = roomFile->readUint32LE();
	uint32_t pair12_size = roomFile->readUint32LE();

	// startPos += 2;
	uint32_t conversation_start = pair12_data_offset + startPos;
	uint32_t conversation_size = pair12_size - startPos;

	roomFile->seek(conversation_start, SEEK_SET);
	byte *data = new byte[conversation_size];
	roomFile->read(data, conversation_size);

	Common::Array<ConversationElement> elements = parseConversationElements(data, conversation_size);
	Common::Array<ConversationNode> roots = buildTreeStructure(elements);
	return roots;

	// uint32_t i = 0;
	// int lineNum = 1;

	// while (i < conversation_size) {
	//     uint32_t lineStart = i;
	//     ConversationLine line;
	//     line.offset = startPos + lineStart;
	//     line.speaker = 0;

	//     // Read until we hit an end marker
	//     while (i < conversation_size) {
	//         byte b = data[i];

	//         if (b == 0x08) { // SPEAKER
	//             i++;
	//             if (i < conversation_size) {
	//                 line.speaker = data[i];
	//                 line.controlBytes.push_back(Common::String::format("SPEAKER(0x%02X)", line.speaker));
	//                 i++;
	//             }
	//             continue;
	//         } else if (isControlByte(b)) {
	//             line.controlBytes.push_back(Common::String::format("%s(0x%02X)",
	//                                                                 getControlName(b).c_str(), b));
	//             i++;

	//             // Check for end markers
	//             if (isTerminalByte(b)) {
	//                 break;
	//             }
	//         } else {
	//             // Regular text character
	//             if (b != 0x00) {
	//                 line.text += decodeByte(b);
	//             }
	//             i++;
	//         }
	//     }

	//     // Store raw bytes for this line
	//     for (uint32_t j = lineStart; j < i && j < conversation_size; j++) {
	//         line.rawBytes.push_back(data[j]);
	//     }

	//     // Only add line if it has content
	//     if (!line.text.empty() || line.controlBytes.size() > 0) {
	//         // debug("Line %d (offset %d): %s", lineNum, line.offset, line.text.c_str());
	//         if (line.controlBytes.size() > 0) {
	//             Common::String controls;
	//             for (uint k = 0; k < line.controlBytes.size(); k++) {
	//                 if (k > 0) controls += ", ";
	//                 controls += line.controlBytes[k];
	//             }
	//             // debug("  Controls: %s", controls.c_str());
	//         }
	//         conversations.push_back(line);
	//         lineNum++;
	//     }
	// }

	// delete[] data;
	// debug("Loaded %d conversation lines", conversations.size());
	// return conversations;
}

void PelrockEngine::loadRoomMetadata(Common::File *roomFile, int roomOffset) {
	uint32_t outPos = 0;

	Common::Array<Description> descriptions = loadRoomDescriptions(roomFile, roomOffset, outPos);
	debug("After decsriptions, position is %d", outPos);
	Common::Array<ConversationNode> roots = loadConversations(roomFile, roomOffset, outPos);
	for (int i = 0; i < roots.size(); i++) {
		if (roots[i].text.empty()) {
			continue;
		}
		debug("Conversation %d: %s", i, roots[i].text.c_str());
	}
	_currentRoomConversations = roots;

	Common::Array<AnimSet> anims = loadRoomAnimations(roomFile, roomOffset);

	Common::Array<HotSpot> hotspots;
	int count = 0;
	for (int i = 0; i < anims.size(); i++) {

		HotSpot thisHotspot;
		thisHotspot.x = anims[i].x;
		thisHotspot.y = anims[i].y;
		thisHotspot.w = anims[i].w;
		thisHotspot.h = anims[i].h;
		thisHotspot.extra = anims[i].extra;
		thisHotspot.type = anims[i].actionFlags;
		thisHotspot.isEnabled = !anims[i].isDisabled;
		hotspots.push_back(thisHotspot);
		count++;
	}

	Common::Array<HotSpot> staticHotspots = loadHotspots(roomFile, roomOffset);
	Common::List<Exit> exits = loadExits(roomFile, roomOffset);

	Common::List<WalkBox> walkboxes = loadWalkboxes(roomFile, roomOffset);

	debug("total descriptions = %d, anims = %d, hotspots = %d", descriptions.size(), anims.size(), staticHotspots.size());
	for (int i = 0; i < staticHotspots.size(); i++) {
		HotSpot hotspot = staticHotspots[i];
		hotspots.push_back(hotspot);
	}

	int walkboxCount = 0;

	_currentRoomAnims = anims;
	_currentRoomHotspots = hotspots;
	_currentRoomExits = exits;
	_currentRoomWalkboxes = walkboxes;
	_currentRoomDescriptions = descriptions;

	for (int i = 0; i < _currentRoomHotspots.size(); i++) {
		HotSpot hotspot = _currentRoomHotspots[i];
		// debug("Hotspot %d: x=%d y=%d w=%d h=%d type=%d enabled? %d extra=%d, desc=%s", i, hotspot.x, hotspot.y, hotspot.w, hotspot.h, hotspot.type, hotspot.isEnabled, hotspot.extra, _currentRoomDescriptions[i].text.c_str());
		_screen->drawLine(hotspot.x, hotspot.y, hotspot.x + hotspot.w, hotspot.y, 200 + i);
		_screen->drawLine(hotspot.x, hotspot.y + hotspot.h, hotspot.x + hotspot.w, hotspot.y + hotspot.h, 200 + i);
		_screen->drawLine(hotspot.x, hotspot.y, hotspot.x, hotspot.y + hotspot.h, 200 + i);
		_screen->drawLine(hotspot.x + hotspot.w, hotspot.y, hotspot.x + hotspot.w, hotspot.y + hotspot.h, 200 + i);
	}

	for (Common::List<Exit>::iterator i = _currentRoomExits.begin(); i != _currentRoomExits.end(); i++) {
		// debug("Exit: x=%d y=%d w=%d h=%d to room %d", i->x, i->y, i->w, i->h, i->targetRoom);
		// _screen->fillRect(Common::Rect(i->x, i->y, i->x + i->w, i->y + i->h), 255);
		// _screen->drawLine(i->x, i->y, i->x + i->w, i->y, 0);
		// _screen->drawLine(i->x, i->y + i->h, i->x + i->w, i->y + i->h, 0);
		// _screen->drawLine(i->x, i->y, i->x, i->y + i->h, 0);
		// _screen->drawLine(i->x + i->w, i->y, i->x + i->w, i->y + i->h);
	}
}

void PelrockEngine::loadCursors() {
	Common::File alfred7File;
	if (!alfred7File.open("ALFRED.7")) {
		error("Couldnt find file ALFRED.7");
	}
	for (int i = 0; i < 5; i++) {
		uint32_t cursorOffset = cursor_offsets[i];
		alfred7File.seek(cursorOffset);
		_cursorMasks[i] = new byte[kCursorSize];
		alfred7File.read(_cursorMasks[i], kCursorSize);
	}
	alfred7File.close();
}

void PelrockEngine::loadInteractionIcons() {
	Common::File alfred7File;
	if (!alfred7File.open("ALFRED.7")) {
		error("Couldnt find file ALFRED.7");
	}

	alfred7File.seek(kBalloonFramesOffset, SEEK_SET);

	uint32_t totalBalloonSize = kBalloonWidth * kBalloonHeight * kBalloonFrames;
	_popUpBalloon = new byte[totalBalloonSize];

	uint32_t compressedSize = kBalloonFramesSize;

	byte *raw = new byte[compressedSize];
	alfred7File.read(raw, compressedSize);
	rleDecompress(raw, compressedSize, 0, compressedSize, &_popUpBalloon);

	delete[] raw;

	alfred7File.close();
	Common::File alfred4File;
	if (!alfred4File.open("ALFRED.4")) {
		error("Couldnt find file ALFRED.4");
	}

	int iconSize = kVerbIconHeight * kVerbIconWidth;
	for (int i = 0; i < kNumVerbIcons; i++) {
		uint32_t iconOffset = i * iconSize;
		_verbIcons[i] = new byte[iconSize];
		alfred4File.read(_verbIcons[i], iconSize);
	}
	alfred4File.close();
}

Common::List<Exit> PelrockEngine::loadExits(Common::File *roomFile, int roomOffset) {
	Common::List<Exit> exits;
	uint32_t pair10_offset_pos = roomOffset + (10 * 8);
	roomFile->seek(pair10_offset_pos, SEEK_SET);
	uint32_t pair10_data_offset = roomFile->readUint32LE();
	uint32_t pair10_size = roomFile->readUint32LE();
	roomFile->seek(pair10_data_offset + 0x1BE, SEEK_SET);
	int exit_count = roomFile->readByte();
	roomFile->seek(pair10_data_offset + 0x1BF, SEEK_SET);
	for (int i = 0; i < exit_count; i++) {
		Exit exit;
		exit.targetRoom = roomFile->readUint16LE();
		exit.flags = roomFile->readByte();
		exit.x = roomFile->readUint16LE();
		exit.y = roomFile->readUint16LE();
		exit.w = roomFile->readByte();
		exit.h = roomFile->readByte();

		exit.targetX = roomFile->readUint16LE();
		exit.targetY = roomFile->readUint16LE();
		exit.dir = roomFile->readByte();
		exits.push_back(exit);
	}
	return exits;
}

Common::Array<HotSpot> PelrockEngine::loadHotspots(Common::File *roomFile, int roomOffset) {
	uint32_t pair10_offset_pos = roomOffset + (10 * 8);
	debug("Hotspot(10)  pair offset position: %d", pair10_offset_pos);
	roomFile->seek(pair10_offset_pos, SEEK_SET);
	uint32_t pair10_data_offset = roomFile->readUint32LE();
	uint32_t pair10_size = roomFile->readUint32LE();
	uint32_t count_offset = pair10_data_offset + 0x47a;
	roomFile->seek(count_offset, SEEK_SET);
	byte hotspot_count = roomFile->readByte();
	uint32_t hotspot_data_start = pair10_data_offset + 0x47c;
	Common::Array<HotSpot> hotspots;
	for (int i = 0; i < hotspot_count; i++) {
		uint32_t obj_offset = hotspot_data_start + i * 9;
		roomFile->seek(obj_offset, SEEK_SET);
		byte obj_bytes[9];
		roomFile->read(obj_bytes, 9);
		HotSpot spot;
		spot.type = obj_bytes[0];
		spot.x = obj_bytes[1] | (obj_bytes[2] << 8);
		spot.y = obj_bytes[3] | (obj_bytes[4] << 8);
		spot.w = obj_bytes[5];
		spot.h = obj_bytes[6];
		spot.extra = obj_bytes[7] | (obj_bytes[8] << 8);
		// debug("Hotspot %d: type=%d x=%d y=%d w=%d h=%d extra=%d", i, spot.type, spot.x, spot.y, spot.w, spot.h, spot.extra);
		hotspots.push_back(spot);
	}
	return hotspots;
	// uint32_t hover_areas_start = pair10_data_offset + 0x1BE;
	// roomFile->seek(hover_areas_start, SEEK_SET);
}

void extractSingleFrame(byte *source, byte *dest, int frameIndex, int frameWidth, int frameHeight) {
	for (int y = 0; y < frameHeight; y++) {
		for (int x = 0; x < frameWidth; x++) {
			unsigned int src_pos = (frameIndex * frameHeight * frameWidth) + (y * frameWidth) + x;
			dest[y * frameWidth + x] = source[src_pos];
		}
	}
}

void PelrockEngine::loadAlfredAnims() {
	Common::File alfred3;
	if (!alfred3.open(Common::Path("ALFRED.3"))) {
		error("Could not open ALFRED.3");
		return;
	}
	int alfred3Size = alfred3.size();
	unsigned char *bufferFile = (unsigned char *)malloc(alfred3Size);
	alfred3.seek(0, SEEK_SET);
	alfred3.read(bufferFile, alfred3Size);
	alfred3.close();

	int index = 0;
	int index3 = 0;
	uint32_t capacity = 3060 * 102;
	unsigned char *pic = new unsigned char[capacity];
	rleDecompress(bufferFile, alfred3Size, 0, alfred3Size, &pic);

	for (int i = 0; i < 4; i++) {
		standingAnimFrames[i] = new byte[kAlfredFrameWidth * kAlfredFrameHeight];
		int talkingFramesOffset = walkingAnimLengths[0] + walkingAnimLengths[1] + walkingAnimLengths[2] + walkingAnimLengths[3] + 4;

		int prevWalkingFrames = 0;
		int prevTalkingFrames = 0;

		for (int j = 0; j < i; j++) {
			prevWalkingFrames += walkingAnimLengths[j] + 1;
			prevTalkingFrames += talkingAnimLengths[j];
		}

		walkingAnimFrames[i] = new byte *[walkingAnimLengths[i]];

		int standingFrame = prevWalkingFrames;
		debug("Loading standing frame %d at index %d", i, standingFrame);
		extractSingleFrame(pic, standingAnimFrames[i], standingFrame, kAlfredFrameWidth, kAlfredFrameHeight);
		for (int j = 0; j < walkingAnimLengths[i]; j++) {

			walkingAnimFrames[i][j] = new byte[kAlfredFrameWidth * kAlfredFrameHeight];
			int walkingFrame = prevWalkingFrames + 1 + j;
			extractSingleFrame(pic, walkingAnimFrames[i][j], walkingFrame, kAlfredFrameWidth, kAlfredFrameHeight);
		}

		talkingAnimFrames[i] = new byte *[talkingAnimLengths[i]];

		int talkingStartFrame = talkingFramesOffset + prevTalkingFrames;
		for (int j = 0; j < talkingAnimLengths[i]; j++) {
			talkingAnimFrames[i][j] = new byte[kAlfredFrameWidth * kAlfredFrameHeight];
			int talkingFrame = talkingStartFrame + j;
			extractSingleFrame(pic, talkingAnimFrames[i][j], talkingFrame, kAlfredFrameWidth, kAlfredFrameHeight);
		}
	}
}

byte *PelrockEngine::grabBackgroundSlice(int x, int y, int w, int h) {
	byte *bg = new byte[w * h];
	for (int j = 0; j < w; j++) {
		for (int i = 0; i < h; i++) {
			int idx = i * w + j;
			if (y + i < 400 && x + j < 640) {
				*(bg + idx) = _currentBackground[(y + i) * 640 + (x + j)];
			}
		}
	}
	return bg;
}
void PelrockEngine::putBackgroundSlice(int x, int y, int w, int h, byte *slice) {
	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {
			int index = (j * w + i);
			if (x + i < 640 && y + j < 400)
				*(byte *)g_engine->_screen->getBasePtr(x + i, y + j) = slice[index];
		}
	}
}

// Helper function for transparent blitting
void drawSpriteToBuffer(byte *buffer, int bufferWidth,
						byte *sprite, int x, int y,
						int width, int height,
						int transparentColor) {
	for (int py = 0; py < height; py++) {
		for (int px = 0; px < width; px++) {
			int srcIdx = py * width + px;
			byte pixel = sprite[srcIdx];

			if (pixel != transparentColor) {
				int destX = x + px;
				int destY = y + py;

				if (destX >= 0 && destX < 640 &&
					destY >= 0 && destY < 400) {
					buffer[destY * bufferWidth + destX] = pixel;
				}
			}
		}
	}
}

Common::List<VerbIcons> PelrockEngine::populateActionsMenu(HotSpot *hotspot) {
	Common::List<VerbIcons> verbs;
	debug("Populating actions menu for hotspot type %d", hotspot->type);
	verbs.push_back(LOOK);

	if (hotspot->type & 1) {
		debug("Hotspot allows OPEN action");
		verbs.push_back(OPEN);
	}
	if (hotspot->type & 2) {
		debug("Hotspot allows CLOSE action");
		verbs.push_back(CLOSE);
	}
	if (hotspot->type & 4) {
		debug("Hotspot allows UNKNOWN action");
		verbs.push_back(UNKNOWN);
	}
	if (hotspot->type & 8) {
		debug("Hotspot allows PICKUP action");
		verbs.push_back(PICKUP);
	}
	if (hotspot->type & 16) {
		debug("Hotspot allows TALK action");
		verbs.push_back(TALK);
	}
	if (hotspot->type & 32) {
		debug("Hotspot allows WALK action");
		verbs.push_back(PUSH);
	}
	if (hotspot->type & 128) {
		debug("Hotspot allows PULL action");
		verbs.push_back(PULL);
	}
	return verbs;
}

void PelrockEngine::frames() {

	if (_chronoManager->_gameTick) {

		memcpy(_compositeBuffer, _currentBackground, 640 * 400);

		debug("Game tick!");
		for (int i = 0; i < _currentRoomAnims.size(); i++) {
			// debug("Processing animation set %d, numAnims %d", num, i->numAnims);

			int x = _currentRoomAnims[i].animData[_currentRoomAnims[i].curAnimIndex].x;
			int y = _currentRoomAnims[i].animData[_currentRoomAnims[i].curAnimIndex].y;
			int w = _currentRoomAnims[i].animData[_currentRoomAnims[i].curAnimIndex].w;
			int h = _currentRoomAnims[i].animData[_currentRoomAnims[i].curAnimIndex].h;

			int frameSize = _currentRoomAnims[i].animData[_currentRoomAnims[i].curAnimIndex].w * _currentRoomAnims[i].animData[_currentRoomAnims[i].curAnimIndex].h;
			int curFrame = _currentRoomAnims[i].animData[_currentRoomAnims[i].curAnimIndex].curFrame;
			byte *frame = new byte[frameSize];
			Common::copy(_currentRoomAnims[i].animData[_currentRoomAnims[i].curAnimIndex].animData + (curFrame * _currentRoomAnims[i].animData[_currentRoomAnims[i].curAnimIndex].h * _currentRoomAnims[i].animData[_currentRoomAnims[i].curAnimIndex].w), _currentRoomAnims[i].animData[_currentRoomAnims[i].curAnimIndex].animData + (curFrame * _currentRoomAnims[i].animData[_currentRoomAnims[i].curAnimIndex].h * _currentRoomAnims[i].animData[_currentRoomAnims[i].curAnimIndex].w) + (frameSize), frame);
			// debug("Current frame %d of %d", curFrame, i->animData[j].nframes);

			// byte *bg = grabBackgroundSlice(x, y, w, h);
			// putBackgroundSlice(x, y, w, h, bg);

				drawSpriteToBuffer(_compositeBuffer, 640, frame, _currentRoomAnims[i].x, _currentRoomAnims[i].y, _currentRoomAnims[i].w, _currentRoomAnims[i].h, 255);
				// for (int y = 0; y < i->h; y++) {
				// 	for (int x = 0; x < i->w; x++) {

				// 		unsigned int src_pos = (y * i->w) + x;
				// 		int xPos = i->x + x;
				// 		int yPos = i->y + y;
				// 		if (frame[src_pos] != 255 && xPos > 0 && yPos > 0 && xPos < 640 && yPos < 400)
				// 			_screen->setPixel(xPos, yPos, frame[src_pos]);
				// 	}
				// }
			if (_currentRoomAnims[i].animData[_currentRoomAnims[i].curAnimIndex].elpapsedFrames == _currentRoomAnims[i].animData[_currentRoomAnims[i].curAnimIndex].speed) {
				_currentRoomAnims[i].animData[_currentRoomAnims[i].curAnimIndex].elpapsedFrames = 0;
				if (_currentRoomAnims[i].animData[_currentRoomAnims[i].curAnimIndex].curFrame < _currentRoomAnims[i].animData[_currentRoomAnims[i].curAnimIndex].nframes - 1) {
					_currentRoomAnims[i].animData[_currentRoomAnims[i].curAnimIndex].curFrame++;
				} else {
					if (_currentRoomAnims[i].animData[_currentRoomAnims[i].curAnimIndex].curLoop < _currentRoomAnims[i].animData[_currentRoomAnims[i].curAnimIndex].loopCount - 1) {
						_currentRoomAnims[i].animData[_currentRoomAnims[i].curAnimIndex].curFrame = 0;
						_currentRoomAnims[i].animData[_currentRoomAnims[i].curAnimIndex].curLoop++;
					} else {
						_currentRoomAnims[i].animData[_currentRoomAnims[i].curAnimIndex].curFrame = 0;
						_currentRoomAnims[i].animData[_currentRoomAnims[i].curAnimIndex].curLoop = 0;
						if (_currentRoomAnims[i].curAnimIndex < _currentRoomAnims[i].numAnims - 1) {
							_currentRoomAnims[i].curAnimIndex++;
						} else {
							_currentRoomAnims[i].curAnimIndex = 0;
						}
					}
				}
			}
			else {
				_currentRoomAnims[i].animData[_currentRoomAnims[i].curAnimIndex].elpapsedFrames++;
			}
		}

		// if (_bgAlfred != nullptr) {
		// 	putBackgroundSlice(xAlfred, yAlfred, kAlfredFrameWidth, kAlfredFrameHeight, _bgAlfred);
		// 	delete[] _bgAlfred;
		// 	_bgAlfred = nullptr;
		// }
		// _bgAlfred = grabBackgroundSlice(xAlfred, yAlfred, kAlfredFrameWidth, kAlfredFrameHeight);

		if (isAlfredWalking) {
			debug("Drawing walking frame %d for direction %d", curAlfredFrame, dirAlfred);
			drawAlfred(walkingAnimFrames[dirAlfred][curAlfredFrame]);

			if (curAlfredFrame < walkingAnimLengths[dirAlfred] - 1) {
				curAlfredFrame++;
			} else {
				curAlfredFrame = 0;
			}
			debug("CurAlfredFrame from walking is now %d", curAlfredFrame);
		} else if (isAlfredTalking) {
			drawAlfred(talkingAnimFrames[dirAlfred][curAlfredFrame]);

			if (curAlfredFrame < talkingAnimLengths[dirAlfred] - 1) {
				curAlfredFrame++;
			} else {
				curAlfredFrame = 0;
			}
			debug("CurAlfredFrame from talking is now %d", curAlfredFrame);
		} else {
			drawAlfred(standingAnimFrames[dirAlfred]);
		}
		if (_displayPopup) {

			// byte *bgDialog = new byte[kBalloonWidth * kBalloonHeight];
			// for (int j = 0; j < kBalloonWidth; j++) {
			// 	for (int i = 0; i < kBalloonHeight; i++) {
			// 		int idx = i * kBalloonWidth + j;
			// 		if (_popupY + i < 400 && _popupX + j < 640) {
			// 			*(bgDialog + idx) = _currentBackground[(_popupY + i) * 640 + (_popupX + j)];
			// 		}
			// 	}
			// }
			// if (_bgPopupBalloon != nullptr) {
			// 	putBackgroundSlice(_popupX, _popupY, kBalloonWidth, kBalloonHeight, _bgPopupBalloon);
			// }
			showActionBalloon(_popupX, _popupY, _currentPopupFrame);
			if (_currentPopupFrame < 3) {
				_currentPopupFrame++;
			} else
				_currentPopupFrame = 0;
		}
		// int walkboxCount = 0;
		// for (Common::List<WalkBox>::iterator i = _currentRoomWalkboxes.begin(); i != _currentRoomWalkboxes.end(); i++) {
		// 	// _screen->fillRect(Common::Rect(i->x, i->y, i->x + i->w, i->y + i->h), 255);
		// 	_screen->drawLine(i->x, i->y, i->x + i->w, i->y, 0 + walkboxCount);
		// 	_screen->drawLine(i->x, i->y + i->h, i->x + i->w, i->y + i->h, 0 + walkboxCount);
		// 	_screen->drawLine(i->x, i->y, i->x, i->y + i->h, 0 + walkboxCount);
		// 	_screen->drawLine(i->x + i->w, i->y, i->x + i->w, i->y + i->h, 0 + walkboxCount);
		// 	walkboxCount++;
		// }
		memcpy(_screen->getPixels(), _compositeBuffer, 640 * 400);
		_screen->markAllDirty();
		// _screen->update();
	}
}

void PelrockEngine::drawAlfred(byte *buf) {
	// for (uint32_t y = 0; y < kAlfredFrameHeight; y++) {
	// 	for (uint32_t x = 0; x < kAlfredFrameWidth; x++) {
	// 		unsigned int src_pos = (y * kAlfredFrameWidth) + x;
	// 		// debug("Xpos = %d, yPos=%d", x + xAlfred, y + yAlfred);
	// 		if (buf[src_pos] != 255 && x + xAlfred >= 0 && y + yAlfred >= 0 && x + xAlfred < 640 && y + yAlfred < 400)
	// 			_screen->setPixel(x + xAlfred, y + yAlfred, buf[src_pos]);
	// 	}
	// }
	drawSpriteToBuffer(_compositeBuffer, 640, buf, xAlfred, yAlfred, kAlfredFrameWidth, kAlfredFrameHeight, 255);
}

void PelrockEngine::checkLongMouseClick(int x, int y) {
	int hotspotIndex = isHotspotUnder(mouseX, mouseY);
	if (hotspotIndex != -1) {
		// _popupX = hotspot->x;
		// _popupY = hotspot->y;
		// if (_bgPopupBalloon != nullptr) {
		// 	putBackgroundSlice(_popupX, _popupY, kBalloonWidth, kBalloonHeight, _bgPopupBalloon);
		// 	delete[] _bgPopupBalloon;
		// }
		_popupX = x - kBalloonWidth / 2;
		if (_popupX < 0)
			_popupX = 0;
		if (_popupX + kBalloonWidth > 640) {
			_popupX -= 640 - (_popupX + kBalloonWidth);
		}

		_popupY = y - kBalloonHeight;
		if (_popupY < 0) {
			_popupY = 0;
		}
		_displayPopup = true;
		_currentPopupFrame = 0;
		_currentHotspot = &_currentRoomHotspots[hotspotIndex];
		debug("Current hotspot type: %d", _currentHotspot->type);
		// _bgPopupBalloon = grabBackgroundSlice(_popupX, _popupY, kBalloonWidth, kBalloonHeight);
	}
}

int PelrockEngine::isHotspotUnder(int x, int y) {

	for (size_t i = 0; i < _currentRoomHotspots.size(); i++) {
		HotSpot hotspot = _currentRoomHotspots[i];
		if (hotspot.isEnabled &&
			mouseX >= hotspot.x && mouseX <= (hotspot.x + hotspot.w) &&
			mouseY >= hotspot.y && mouseY <= (hotspot.y + hotspot.h)) {
			return i;
		}
	}
	return -1;
}

Exit *PelrockEngine::isExitUnder(int x, int y) {
	for (Common::List<Exit>::iterator i = _currentRoomExits.begin(); i != _currentRoomExits.end(); i++) {
		if (x >= i->x && x <= (i->x + i->w) &&
			y >= i->y && y <= (i->y + i->h)) {
			return &(*i);
		}
	}
	return nullptr;
}

void PelrockEngine::showActionBalloon(int posx, int posy, int curFrame) {

	drawSpriteToBuffer(_compositeBuffer, 640, _popUpBalloon + (curFrame * kBalloonHeight * kBalloonWidth), posx, posy, kBalloonWidth, kBalloonHeight, 255);
	Common::List<VerbIcons> availableActions = populateActionsMenu(_currentHotspot);

	// for (Common::List<VerbIcons>::iterator i = availableActions.begin(); i != availableActions.end(); i++) {
	// 	debug("Verb icon to show: %d", *i);
	// }

	drawSpriteToBuffer(_compositeBuffer, 640, _verbIcons[LOOK], posx + 20, posy + 20, kVerbIconWidth, kVerbIconHeight, 1);
	for (Common::List<VerbIcons>::iterator i = availableActions.begin(); i != availableActions.end(); i++) {
		VerbIcons verb = *i;
		int index = 0;
		for (Common::List<VerbIcons>::iterator j = availableActions.begin(); j != i; j++) {
			index++;
		}
		drawSpriteToBuffer(_compositeBuffer, 640, _verbIcons[verb], posx + 20 + (index * (kVerbIconWidth + 2)), posy + 20, kVerbIconWidth, kVerbIconHeight, 1);
	}
}

void PelrockEngine::checkMouseClick(int x, int y) {

	_displayPopup = false;
	_currentHotspot = nullptr;
	// if (_bgPopupBalloon != nullptr) {
	// 	putBackgroundSlice(_popupX, _popupY, kBalloonWidth, kBalloonHeight, _bgPopupBalloon);
	// 	delete[] _bgPopupBalloon;
	// 	_bgPopupBalloon = nullptr;
	// }

	Common::Point walkTarget = calculateWalkTarget(mouseX, mouseY);

	Exit *exit = isExitAtPoint(walkTarget.x, walkTarget.y);

	if (exit != nullptr) {
		xAlfred = exit->targetX;
		yAlfred = exit->targetY - kAlfredFrameHeight;
		setScreen(exit->targetRoom, exit->dir);
	}

	int hotspotIndex = isHotspotUnder(mouseX, mouseY);
	if (hotspotIndex != -1) {
		talk();
		debug("Hotspot clicked: %d", _currentRoomHotspots[hotspotIndex].extra);
		// showDescription(_currentRoomDescriptions[hotspotIndex].text.c_str(), xAlfred, yAlfred, 13);
		// changeCursor(HOTSPOT);
	}
}

void PelrockEngine::changeCursor(Cursor cursor) {
	CursorMan.replaceCursor(_cursorMasks[cursor], kCursorWidth, kCursorHeight, 0, 0, 255);
}

void PelrockEngine::checkMouseHover() {
	bool isSomethingUnder = false;

	// Calculate walk target first (before checking anything else)
	Common::Point walkTarget = calculateWalkTarget(mouseX, mouseY);

	// Check if walk target hits any exit
	bool exitDetected = false;
	Exit *exit = isExitAtPoint(walkTarget.x, walkTarget.y);
	if (exit != nullptr) {
		exitDetected = true;
	}

	int hotspotIndex = isHotspotUnder(mouseX, mouseY);
	if (hotspotIndex != -1) {
		isSomethingUnder = true;
	}
	// Exit *exit = isExitUnder(mouseX, mouseY);
	// if (exit != nullptr) {
	// 	isSomethingUnder = true;
	// 	changeCursor(EXIT);
	// }
	if (isSomethingUnder && exitDetected) {
		changeCursor(COMBINATION);
	} else if (isSomethingUnder) {
		changeCursor(HOTSPOT);
	} else if (exitDetected) {
		changeCursor(EXIT);
	} else {
		changeCursor(DEFAULT);
	}
}

Common::Point PelrockEngine::calculateWalkTarget(int mouseX, int mouseY) {
	// Starting point for pathfinding
	int sourceX = mouseX;
	int sourceY = mouseY;

	// TODO: If hovering over a sprite/hotspot, adjust source point to sprite center
	// For now, just use mouse position

	// Find nearest walkable point in walkboxes
	uint32 minDistance = 0xFFFFFFFF;
	Common::Point bestTarget(sourceX, sourceY);

	for (Common::List<WalkBox>::iterator it = _currentRoomWalkboxes.begin();
		 it != _currentRoomWalkboxes.end(); ++it) {

		// Calculate distance from source point to this walkbox (Manhattan distance)
		int dx = 0;
		int dy = 0;

		// Calculate horizontal distance
		if (sourceX < it->x) {
			dx = it->x - sourceX;
		} else if (sourceX > it->x + it->w) {
			dx = sourceX - (it->x + it->w);
		}
		// else: sourceX is inside walkbox horizontally, dx = 0

		// Calculate vertical distance
		if (sourceY < it->y) {
			dy = it->y - sourceY;
		} else if (sourceY > it->y + it->h) {
			dy = sourceY - (it->y + it->h);
		}
		// else: sourceY is inside walkbox vertically, dy = 0

		uint32 distance = dx + dy;

		if (distance < minDistance) {
			minDistance = distance;

			// Calculate target point (nearest point on walkbox to source)
			int targetX = sourceX;
			int targetY = sourceY;

			if (sourceX < it->x) {
				targetX = it->x;
			} else if (sourceX > it->x + it->w) {
				targetX = it->x + it->w;
			}

			if (sourceY < it->y) {
				targetY = it->y;
			} else if (sourceY > it->y + it->h) {
				targetY = it->y + it->h;
			}

			bestTarget.x = targetX;
			bestTarget.y = targetY;
		}
	}

	return bestTarget;
}

Exit *PelrockEngine::isExitAtPoint(int x, int y) {
	for (Common::List<Exit>::iterator i = _currentRoomExits.begin();
		 i != _currentRoomExits.end(); ++i) {
		// Check if point is inside exit trigger rectangle
		if (x >= i->x && x <= (i->x + i->w) &&
			y >= i->y && y <= (i->y + i->h)) {
			return &(*i);
		}
	}
	return nullptr;
}

void PelrockEngine::showDescription(Common::String text, int x, int y, byte color) {
	Common::Rect rect = _largeFont->getBoundingBox(text.c_str());
	if (x + 2 + rect.width() > 640) {
		x = 640 - rect.width() - 2;
	}
	if (y + 2 + rect.height() > 400) {
		y = 400 - rect.height();
	}
	if (x - 2 < 0) {
		x = 2;
	}
	if (y - 2 < 0) {
		y = 2;
	}

	x = 2;
	y = 2;
	// if (_bgText != nullptr) {
	// 	putBackgroundSlice(x, y, 640, 400, _bgText);
	// 	delete[] _bgText;
	// }
	int16 w = MIN(rect.width(), (int16)(640 - x));
	int16 h = MIN(rect.height(), (int16)(400 - y));
	debug("grabbing bg slice at (%d,%d) w=%d h=%d", x, y, w, h);

	// _bgText = grabBackgroundSlice(x, y, 640, 400);
	_largeFont->drawString(_screen, text.c_str(), x - 1, y, 640, 0); // Left
	_largeFont->drawString(_screen, text.c_str(), x - 2, y, 640, 0); // Left
	_largeFont->drawString(_screen, text.c_str(), x + 1, y, 640, 0); // Right
	_largeFont->drawString(_screen, text.c_str(), x + 2, y, 640, 0); // Right
	_largeFont->drawString(_screen, text.c_str(), x, y - 1, 640, 0); // Top
	_largeFont->drawString(_screen, text.c_str(), x, y - 2, 640, 0); // Top
	_largeFont->drawString(_screen, text.c_str(), x, y + 1, 640, 0); // Bottom
	_largeFont->drawString(_screen, text.c_str(), x, y + 2, 640, 0); // Bottom

	// Draw main text on top
	_largeFont->drawString(_screen, text.c_str(), x, y, 640, color);
}

void PelrockEngine::setScreen(int number, int dir) {

	Common::File roomFile;
	if (!roomFile.open(Common::Path("ALFRED.1"))) {
		error("Could not open ALFRED.1");
		return;
	}
	dirAlfred = dir;

	int roomOffset = number * kRoomStructSize;
	curAlfredFrame = 0;
	byte *palette = new byte[256 * 3];
	getPalette(&roomFile, roomOffset, palette);

	int paletteOffset = roomOffset + (11 * 8);
	roomFile.seek(paletteOffset, SEEK_SET);
	uint32 offset = roomFile.readUint32LE();

	g_system->getPaletteManager()->setPalette(palette, 0, 256);

	byte *background = new byte[640 * 400];
	getBackground(&roomFile, roomOffset, background);
	if (_currentBackground != nullptr)
		delete[] _currentBackground;
	_currentBackground = new byte[640 * 400];
	Common::copy(background, background + 640 * 400, _currentBackground);
	for (int i = 0; i < 640; i++) {
		for (int j = 0; j < 400; j++) {
			_screen->setPixel(i, j, background[j * 640 + i]);
		}
	}

	loadRoomMetadata(&roomFile, roomOffset);

	_screen->markAllDirty();
	roomFile.close();
	delete[] background;
	delete[] palette;
}

Common::Error PelrockEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

} // End of namespace Pelrock
