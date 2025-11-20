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

void drawRect(Graphics::ManagedSurface *surface, int x, int y, int w, int h, byte color) {
	// debug("Drawing rect at (%d,%d) w=%d h=%d color=%d", x, y, w, h, color);
	surface->drawLine(x, y, x + w, y, color);
	surface->drawLine(x, y + h, x + w, y + h, color);
	surface->drawLine(x, y, x, y + h, color);
	surface->drawLine(x + w, y, x + w, y + h, color);
}
Common::Array<Common::Array<Common::String> > wordWrap(Common::String text);
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
		setScreen(0, 1);
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
	}
	return anims;
}

Common::Array<WalkBox> PelrockEngine::loadWalkboxes(Common::File *roomFile, int roomOffset) {
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
	Common::Array<WalkBox> walkboxes;
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
	Common::Array<Exit> exits = loadExits(roomFile, roomOffset);

	Common::Array<WalkBox> walkboxes = loadWalkboxes(roomFile, roomOffset);

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
		drawRect(_screen, hotspot.x, hotspot.y, hotspot.w, hotspot.h, 200 + i);
	}

	for (int i = 0; i < _currentRoomExits.size(); i++) {
		Exit exit = _currentRoomExits[i];
		drawRect(_screen, exit.x, exit.y, exit.w, exit.h, 100 + i);
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

Common::Array<Exit> PelrockEngine::loadExits(Common::File *roomFile, int roomOffset) {
	Common::Array<Exit> exits;
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

		// debug("Game tick!");
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

			drawSpriteToBuffer(_compositeBuffer, 640, frame, _currentRoomAnims[i].x, _currentRoomAnims[i].y, _currentRoomAnims[i].w, _currentRoomAnims[i].h, 255);

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
			} else {
				_currentRoomAnims[i].animData[_currentRoomAnims[i].curAnimIndex].elpapsedFrames++;
			}
		}

		if (isAlfredWalking) {

			MovementStep step = _currentContext.movement_buffer[_current_step];
			debug("Alfred step: distance_x=%d, distance_y=%d", step.distance_x, step.distance_y);

			if (step.distance_x > 0) {
				if (step.flags & MOVE_RIGHT) {
					dirAlfred = 0;
					xAlfred += MIN((uint16_t)6, step.distance_x);
				}
				if (step.flags & MOVE_LEFT) {
					dirAlfred = 1;
					xAlfred -= MIN((uint16_t)6, step.distance_x);
				}
			}
			if (step.distance_y > 0) {
				if (step.flags & MOVE_DOWN) {
					dirAlfred = 2;
					yAlfred += MIN((uint16_t)6, step.distance_y);
				}
				if (step.flags & MOVE_UP) {
					dirAlfred = 3;
					yAlfred -= MIN((uint16_t)6, step.distance_y);
				}
			}

			if (step.distance_x > 0)
				step.distance_x -= MIN((uint16_t)6, step.distance_x);

			if (step.distance_y > 0)
				step.distance_y -= MIN((uint16_t)6, step.distance_y);

			debug("Alfred position after step: x=%d, y=%d, step distance_x=%d, step distance_y=%d", xAlfred, yAlfred, step.distance_x, step.distance_y);
			if (step.distance_x <= 0 && step.distance_y <= 0) {
				debug("Alfred completed step %d", _current_step);
				_current_step++;
				if (_current_step >= _currentContext.movement_count) {
					debug("Alfred reached his walk target.");
					_current_step = 0;
					isAlfredWalking = false;
				}
			} else {
				_currentContext.movement_buffer[_current_step] = step;
			}

			Exit *exit = isExitUnder(xAlfred, yAlfred);

			if (exit != nullptr) {
				xAlfred = exit->targetX;
				yAlfred = exit->targetY;
				setScreen(exit->targetRoom, exit->dir);
			}

			debug("Drawing walking frame %d for direction %d", curAlfredFrame, dirAlfred);

			if (curAlfredFrame >= walkingAnimLengths[dirAlfred]) {
				curAlfredFrame = 0;
			}

			drawAlfred(walkingAnimFrames[dirAlfred][curAlfredFrame]);
			curAlfredFrame++;

		} else if (isAlfredTalking) {
			if (curAlfredFrame >= talkingAnimLengths[dirAlfred] - 1) {
				curAlfredFrame = 0;
			}
			drawAlfred(talkingAnimFrames[dirAlfred][curAlfredFrame]);
			curAlfredFrame++;
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

		memcpy(_screen->getPixels(), _compositeBuffer, 640 * 400);

		if (!_currentTextPages.empty()) {
			// debug("Will render text, _chronoManager->_textTtl=%d", _chronoManager->_textTtl);
			if (_chronoManager->_textTtl > 0) {
				renderText(_currentTextPages[_currentTextPageIndex], _textColor);
			} else if (_currentTextPageIndex < _currentTextPages.size() - 1) {
				_currentTextPageIndex++;

				int totalChars = 0;
				for (int i = 0; i < _currentTextPages[_currentTextPageIndex].size(); i++) {
					totalChars += _currentTextPages[_currentTextPageIndex][i].size();
				}
				_chronoManager->_textTtl = totalChars * kTextCharDisplayTime;
			} else {
				_currentTextPages.clear();
				_currentTextPageIndex = 0;
				isAlfredTalking = false;
			}
		}

		// debug("Drawing walkboxes..., %d, _currentRoomWalkboxes.size()=%d",  _currentRoomWalkboxes.size(), _currentRoomWalkboxes.size());
		for (int i = 0; i < _currentRoomWalkboxes.size(); i++) {
			// debug("Drawing walkbox %d", i);
			WalkBox box = _currentRoomWalkboxes[i];
			drawRect(_screen, box.x, box.y, box.w, box.h, 150 + i);
		}
		if (_curWalkTarget.x < 640 && _curWalkTarget.y < 400 && _curWalkTarget.x >= 0 && _curWalkTarget.y >= 0) {
			_screen->setPixel(_curWalkTarget.x, _curWalkTarget.y, 100);
			if (_curWalkTarget.x - 1 > 0 && _curWalkTarget.y - 1 > 0)
				_screen->setPixel(_curWalkTarget.x - 1, _curWalkTarget.y - 1, 100);
			if (_curWalkTarget.x - 1 > 0 && _curWalkTarget.y + 1 < 400)
				_screen->setPixel(_curWalkTarget.x - 1, _curWalkTarget.y + 1, 100);
			if (_curWalkTarget.x + 1 < 640 && _curWalkTarget.y - 1 > 0)
				_screen->setPixel(_curWalkTarget.x + 1, _curWalkTarget.y - 1, 100);
			if (_curWalkTarget.x + 1 < 640 && _curWalkTarget.y + 1 < 400)
				_screen->setPixel(_curWalkTarget.x + 1, _curWalkTarget.y + 1, 100);
			if (_curWalkTarget.x - 2 > 0)
				_screen->setPixel(_curWalkTarget.x - 2, _curWalkTarget.y, 100);
			if (_curWalkTarget.x + 2 < 640)
				_screen->setPixel(_curWalkTarget.x + 2, _curWalkTarget.y, 100);
			if (_curWalkTarget.y - 2 > 0)
				_screen->setPixel(_curWalkTarget.x, _curWalkTarget.y - 2, 100);
			if (_curWalkTarget.y + 2 < 400)
				_screen->setPixel(_curWalkTarget.x, _curWalkTarget.y + 2, 100);
		}
		_screen->markAllDirty();
		// _screen->update();
	}
}

void PelrockEngine::renderText(Common::Array<Common::String> lines, int color) {
	if (color == ALFRED_COLOR) {
		int baseX = xAlfred;
		int baseY = yAlfred - kAlfredFrameHeight - 10;
		int maxW = 0;
		for (size_t i = 0; i < lines.size(); i++) {
			Common::Rect r = _largeFont->getBoundingBox(lines[i]);
			if (r.width() > maxW) {
				maxW = r.width();
			}
		}
		int lineSize = lines.size();
		for (size_t i = 0; i < lines.size(); i++) {
			int textX = baseX - (maxW / 2);
			int textY = baseY - (lineSize * 20) + (i * 20);
			drawText(lines[i], textX, textY, maxW, color);
		}
	}
}

void PelrockEngine::drawAlfred(byte *buf) {

	drawSpriteToBuffer(_compositeBuffer, 640, buf, xAlfred, yAlfred - kAlfredFrameHeight, kAlfredFrameWidth, kAlfredFrameHeight, 255);
}

void PelrockEngine::checkLongMouseClick(int x, int y) {
	int hotspotIndex = isHotspotUnder(mouseX, mouseY);
	if (hotspotIndex != -1) {

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
	for (int i = 0; i < _currentRoomExits.size(); i++) {
		Exit exit = _currentRoomExits[i];
		if (x >= exit.x && x <= (exit.x + exit.w) &&
			y >= exit.y && y <= (exit.y + exit.h)) {
			return &(_currentRoomExits[i]);
		}
	}
	return nullptr;
}

void PelrockEngine::showActionBalloon(int posx, int posy, int curFrame) {

	drawSpriteToBuffer(_compositeBuffer, 640, _popUpBalloon + (curFrame * kBalloonHeight * kBalloonWidth), posx, posy, kBalloonWidth, kBalloonHeight, 255);
	Common::List<VerbIcons> availableActions = populateActionsMenu(_currentHotspot);

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

void PelrockEngine::walkTo(int x, int y) {
	isAlfredWalking = true;
	curAlfredFrame = 0;

	PathContext context = {NULL, NULL, NULL, 0, 0, 0};

	pathFind(x, y, &context);
	debug("\nPath Information:\n");
	debug("================\n");

	debug("Walkbox path (%d boxes): ", context.path_length);
	for (int i = 0; i < context.path_length && context.path_buffer[i] != PATH_END; i++) {
		debug("%d ", context.path_buffer[i]);
	}

	debug("Movement steps (%d steps):\n", context.movement_count);
	for (int i = 0; i < context.movement_count; i++) {
		MovementStep *step = &context.movement_buffer[i];
		debug("  Step %d: ", i);

		if (step->flags & MOVE_RIGHT)
			debug("RIGHT ");
		if (step->flags & MOVE_LEFT)
			debug("LEFT ");
		if (step->flags & MOVE_DOWN)
			debug("DOWN ");
		if (step->flags & MOVE_UP)
			debug("UP ");

		debug("(dx=%d, dy=%d)\n", step->distance_x, step->distance_y);
	}

	debug("\nCompressed path (%d bytes): ", context.compressed_length);
	for (int i = 0; i < context.compressed_length; i++) {
		debug("%02X ", context.compressed_path[i]);
	}

	// if (x > xAlfred) {
	// 	dirAlfred = RIGHT;
	// } else if (x < xAlfred) {
	// 	dirAlfred = LEFT;
	// } else if (y < yAlfred) {
	// 	dirAlfred = UP;
	// } else if (y > yAlfred) {
	// 	dirAlfred = DOWN;
	// }
	debug("Setting Alfred to walk towards (%d, %d) from (%d, %d) in direction %d", x, y, xAlfred, yAlfred, dirAlfred);
	_currentContext = context;
	debug("Path find complete, movement count: %d", _currentContext.movement_count);
}

bool PelrockEngine::pathFind(int x, int y, PathContext *context) {

	if (context->path_buffer == NULL) {
		context->path_buffer = (uint8_t *)malloc(MAX_PATH_LENGTH);
	}
	if (context->movement_buffer == NULL) {
		context->movement_buffer = (MovementStep *)malloc(MAX_MOVEMENT_STEPS * sizeof(MovementStep));
	}
	// if (context->compressed_path == NULL) {
	//     context->compressed_path = (uint8_t*)malloc(MAX_COMPRESSED_PATH);
	// }

	int startX = xAlfred;
	int startY = yAlfred;
	Common::Point target = calculateWalkTarget(x, y);
	x = target.x;
	y = target.y;
	debug("Startx= %d, starty= %d, destx= %d, desty= %d", startX, startY, x, y);

	uint8_t start_box = find_walkbox_for_point(startX, startY);
	uint8_t dest_box = find_walkbox_for_point(x, y);

	debug("Pathfinding from (%d, %d) in box %d to (%d, %d) in box %d\n",
		  startX, startY, start_box,
		  x, y, dest_box);
	// Check if both points are in valid walkboxes
	if (start_box == 0xFF || dest_box == 0xFF) {
		debug("Error: Start or destination not in any walkbox\n");
		return false;
	}
	// Special case: same walkbox
	if (start_box == dest_box) {
		// Generate direct movement
		MovementStep direct_step;
		direct_step.flags = 0;
		if (startX < x) {
			direct_step.distance_x = x - startX;
			direct_step.flags |= MOVE_RIGHT;
		} else {
			direct_step.distance_x = startX - x;
			direct_step.flags |= MOVE_LEFT;
		}

		if (startY < y) {
			direct_step.distance_y = y - startY;
			direct_step.flags |= MOVE_DOWN;
		} else {
			direct_step.distance_y = startY - y;
			direct_step.flags |= MOVE_UP;
		}

		context->movement_buffer[0] = direct_step;
		context->movement_count = 1;
	} else {
		// Build walkbox path
		context->path_length = build_walkbox_path(start_box, dest_box,
												  context->path_buffer);

		if (context->path_length == 0) {
			debug("Error: No path found\n");
			return false;
		}

		// Generate movement steps
		context->movement_count = generate_movement_steps(
			context->path_buffer,
			context->path_length,
			startX, startY,
			x, y,
			context->movement_buffer);
	}
	return true;
}

/**
 * Calculate movement needed to reach a target within a walkbox
 */
void calculate_movement_to_target(uint16_t current_x, uint16_t current_y,
								  uint16_t target_x, uint16_t target_y,
								  WalkBox *box,
								  MovementStep *step) {
	step->flags = 0;
	step->distance_x = 0;
	step->distance_y = 0;

	// Calculate horizontal movement
	if (current_x < box->x) {
		// Need to move right to enter walkbox
		step->distance_x = box->x - current_x;
		step->flags |= MOVE_RIGHT;
	} else if (current_x > box->x + box->w) {
		// Need to move left to enter walkbox
		step->distance_x = current_x - (box->x + box->w);
		step->flags |= MOVE_LEFT;
	}

	// Calculate vertical movement
	if (current_y < box->y) {
		// Need to move down to enter walkbox
		step->distance_y = box->y - current_y;
		step->flags |= MOVE_DOWN;
	} else if (current_y > box->y + box->h) {
		// Need to move up to enter walkbox
		step->distance_y = current_y - (box->y + box->h);
		step->flags |= MOVE_UP;
	}
}

/**
 * Generate movement steps from walkbox path
 * Returns: number of movement steps generated
 */
uint16_t PelrockEngine::generate_movement_steps(uint8_t *path_buffer,
												uint16_t path_length,
												uint16_t start_x, uint16_t start_y,
												uint16_t dest_x, uint16_t dest_y,
												MovementStep *movement_buffer) {
	uint16_t current_x = start_x;
	uint16_t current_y = start_y;
	uint16_t movement_index = 0;

	// Generate movements for each walkbox in path
	for (uint16_t i = 0; i < path_length && path_buffer[i] != PATH_END; i++) {
		uint8_t box_index = path_buffer[i];
		WalkBox *box = &_currentRoomWalkboxes[box_index];

		MovementStep step;
		calculate_movement_to_target(current_x, current_y,
									 dest_x, dest_y,
									 box, &step);

		if (step.distance_x > 0 || step.distance_y > 0) {
			movement_buffer[movement_index++] = step;

			// Update current position
			if (step.flags & MOVE_RIGHT) {
				current_x = box->x;
			} else if (step.flags & MOVE_LEFT) {
				current_x = box->x + box->w;
			}

			if (step.flags & MOVE_DOWN) {
				current_y = box->y;
			} else if (step.flags & MOVE_UP) {
				current_y = box->y + box->h;
			}
		}
	}

	// Final movement to exact destination
	MovementStep final_step;
	final_step.flags = 0;

	if (current_x < dest_x) {
		final_step.distance_x = dest_x - current_x;
		final_step.flags |= MOVE_RIGHT;
	} else if (current_x > dest_x) {
		final_step.distance_x = current_x - dest_x;
		final_step.flags |= MOVE_LEFT;
	} else {
		final_step.distance_x = 0;
	}

	if (current_y < dest_y) {
		final_step.distance_y = dest_y - current_y;
		final_step.flags |= MOVE_DOWN;
	} else if (current_y > dest_y) {
		final_step.distance_y = current_y - dest_y;
		final_step.flags |= MOVE_UP;
	} else {
		final_step.distance_y = 0;
	}

	if (final_step.distance_x > 0 || final_step.distance_y > 0) {
		movement_buffer[movement_index++] = final_step;
	}

	return movement_index;
}

uint16_t PelrockEngine::build_walkbox_path(
	uint8_t start_box,
	uint8_t dest_box,
	uint8_t *path_buffer) {
	uint16_t path_index = 0;
	uint8_t current_box = start_box;

	// Initialize path with start walkbox
	path_buffer[path_index++] = start_box;

	// Clear visited flags
	clear_visited_flags();

	// Breadth-first search through walkboxes
	while (current_box != dest_box && path_index < MAX_PATH_LENGTH - 1) {
		uint8_t next_box = get_adjacent_walkbox(current_box);

		if (next_box == 0xFF) {
			// Dead end - backtrack
			if (path_index > 1) {
				path_index--;
				current_box = path_buffer[path_index - 1];
			} else {
				// No path exists
				return 0;
			}
		} else if (next_box == dest_box) {
			// Found destination
			path_buffer[path_index++] = dest_box;
			break;
		} else {
			// Continue searching
			path_buffer[path_index++] = next_box;
			current_box = next_box;
		}
	}

	// Terminate path
	path_buffer[path_index] = PATH_END;

	return path_index;
}

void PelrockEngine::clear_visited_flags() {
	for (int i = 0; i < _currentRoomWalkboxes.size(); i++) {
		_currentRoomWalkboxes[i].flags = 0;
	}
}

/**
 * Check if two walkboxes overlap or touch (are adjacent)
 */
bool walkboxes_adjacent(WalkBox *box1, WalkBox *box2) {
	uint16_t box1_x_max = box1->x + box1->w;
	uint16_t box1_y_max = box1->y + box1->h;
	uint16_t box2_x_max = box2->x + box2->w;
	uint16_t box2_y_max = box2->y + box2->h;

	// Check if X ranges overlap
	bool x_overlap = (box1->x <= box2_x_max) && (box2->x <= box1_x_max);

	// Check if Y ranges overlap
	bool y_overlap = (box1->y <= box2_y_max) && (box2->y <= box1_y_max);

	return x_overlap && y_overlap;
}

uint8_t PelrockEngine::get_adjacent_walkbox(uint8_t current_box_index) {
	WalkBox *current_box = &_currentRoomWalkboxes[current_box_index];

	// Mark current walkbox as visited
	current_box->flags = 0x01;

	// Search for adjacent unvisited walkbox
	for (uint8_t i = 0; i < _currentRoomWalkboxes.size(); i++) {
		// Skip current walkbox
		if (i == current_box_index) {
			continue;
		}

		// Skip already visited walkboxes
		if (_currentRoomWalkboxes[i].flags == 0x01) {
			continue;
		}

		// Check if walkboxes are adjacent
		if (walkboxes_adjacent(current_box, &_currentRoomWalkboxes[i])) {
			return i;
		}
	}

	return 0xFF; // No adjacent walkbox found
}

bool PelrockEngine::point_in_walkbox(WalkBox *box, uint16_t x, uint16_t y) {
	return (x >= box->x &&
			x <= box->x + box->w &&
			y >= box->y &&
			y <= box->y + box->h);
}

uint8_t PelrockEngine::find_walkbox_for_point(uint16_t x, uint16_t y) {
	for (uint8_t i = 0; i < _currentRoomWalkboxes.size(); i++) {
		if (point_in_walkbox(&_currentRoomWalkboxes[i], x, y)) {
			return i;
		}
	}
	return 0xFF; // Not found
}

void PelrockEngine::checkMouseClick(int x, int y) {

	_displayPopup = false;
	_currentHotspot = nullptr;

	Common::Point walkTarget = calculateWalkTarget(mouseX, mouseY);
	_curWalkTarget = walkTarget;
	// debug("Calculated walk target at (%d, %d)", walkTarget.x, walkTarget.y);
	// Exit *exit = isExitUnder(walkTarget.x, walkTarget.y);

	/*if (exit != nullptr) {
		xAlfred = exit->targetX;
		yAlfred = exit->targetY;
		setScreen(exit->targetRoom, exit->dir);
	} else {*/
	/*	} */

	int hotspotIndex = isHotspotUnder(mouseX, mouseY);
	if (hotspotIndex != -1) {
		sayAlfred(_currentRoomDescriptions[hotspotIndex].text);
		debug("Hotspot clicked: %d", _currentRoomHotspots[hotspotIndex].extra);
	} else {
		walkTo(walkTarget.x, walkTarget.y);
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
	Exit *exit = isExitUnder(walkTarget.x, walkTarget.y);
	if (exit != nullptr) {
		exitDetected = true;
	}

	int hotspotIndex = isHotspotUnder(mouseX, mouseY);
	if (hotspotIndex != -1) {
		isSomethingUnder = true;
	}

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

	// for (Common::List<WalkBox>::iterator it = _currentRoomWalkboxes.begin();
	//  it != _currentRoomWalkboxes.end(); ++it) {
	for (size_t i = 0; i < _currentRoomWalkboxes.size(); i++) {

		// Calculate distance from source point to this walkbox (Manhattan distance)
		int dx = 0;
		int dy = 0;

		// Calculate horizontal distance
		if (sourceX < _currentRoomWalkboxes[i].x) {
			dx = _currentRoomWalkboxes[i].x - sourceX;
		} else if (sourceX > _currentRoomWalkboxes[i].x + _currentRoomWalkboxes[i].w) {
			dx = sourceX - (_currentRoomWalkboxes[i].x + _currentRoomWalkboxes[i].w);
		}
		// else: sourceX is inside walkbox horizontally, dx = 0

		// Calculate vertical distance
		if (sourceY < _currentRoomWalkboxes[i].y) {
			dy = _currentRoomWalkboxes[i].y - sourceY;
		} else if (sourceY > _currentRoomWalkboxes[i].y + _currentRoomWalkboxes[i].h) {
			dy = sourceY - (_currentRoomWalkboxes[i].y + _currentRoomWalkboxes[i].h);
		}
		// else: sourceY is inside walkbox vertically, dy = 0

		uint32 distance = dx + dy;

		if (distance < minDistance) {
			minDistance = distance;

			// Calculate target point (nearest point on walkbox to source)
			int targetX = sourceX;
			int targetY = sourceY;

			if (sourceX < _currentRoomWalkboxes[i].x) {
				targetX = _currentRoomWalkboxes[i].x;
			} else if (sourceX > _currentRoomWalkboxes[i].x + _currentRoomWalkboxes[i].w) {
				targetX = _currentRoomWalkboxes[i].x + _currentRoomWalkboxes[i].w;
			}

			if (sourceY < _currentRoomWalkboxes[i].y) {
				targetY = _currentRoomWalkboxes[i].y;
			} else if (sourceY > _currentRoomWalkboxes[i].y + _currentRoomWalkboxes[i].h) {
				targetY = _currentRoomWalkboxes[i].y + _currentRoomWalkboxes[i].h;
			}

			bestTarget.x = targetX;
			bestTarget.y = targetY;
		}
	}

	return bestTarget;
}

void PelrockEngine::drawText(Common::String text, int x, int y, int w, byte color) {
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

	_largeFont->drawString(_screen, text.c_str(), x - 1, y, w, 0, Graphics::kTextAlignCenter); // Left
	// _largeFont->drawString(_screen, text.c_str(), x - 2, y, 640, 0); // Left
	_largeFont->drawString(_screen, text.c_str(), x + 1, y, w, 0, Graphics::kTextAlignCenter); // Right
	// _largeFont->drawString(_screen, text.c_str(), x + 2, y, 640, 0); // Right
	_largeFont->drawString(_screen, text.c_str(), x, y - 1, w, 0, Graphics::kTextAlignCenter); // Top
	// _largeFont->drawString(_screen, text.c_str(), x, y - 2, 640, 0); // Top
	_largeFont->drawString(_screen, text.c_str(), x, y + 1, w, 0, Graphics::kTextAlignCenter); // Bottom
	// _largeFont->drawString(_screen, text.c_str(), x, y + 2, 640, 0); // Bottom

	// Draw main text on top
	_largeFont->drawString(_screen, text.c_str(), x, y, w, color, Graphics::kTextAlignCenter);
}

void PelrockEngine::sayAlfred(Common::String text) {
	isAlfredTalking = true;
	curAlfredFrame = 0;
	debug("Alfred says: %s", text.c_str());
	_currentTextPages = wordWrap(text);
	_textColor = 13;
	int totalChars = 0;
	for (int i = 0; i < _currentTextPages[0].size(); i++) {
		totalChars += _currentTextPages[0][i].size();
	}
	_chronoManager->_textTtl = totalChars * kTextCharDisplayTime;
}
bool isEndMarker(char char_byte) {
	return char_byte == CHAR_END_MARKER_1 || char_byte == CHAR_END_MARKER_2 || char_byte == CHAR_END_MARKER_3 || char_byte == CHAR_END_MARKER_4;
}

int calculateWordLength(Common::String text, int startPos, bool &isEnd) {
	// return word_length, is_end
	int wordLength = 0;
	int pos = startPos;
	while (pos < text.size()) {
		char char_byte = text[pos];
		if (char_byte == CHAR_SPACE || isEndMarker(char_byte)) {
			break;
		}
		wordLength++;
		pos++;
	}
	// Check if we hit an end marker
	if (pos < text.size() && isEndMarker(text[pos])) {
		isEnd = true;
	}
	// Count ALL trailing spaces as part of this word
	if (pos < text.size() && !isEnd) {
		if (text[pos] == CHAR_END_MARKER_3) { // 0xF8 (-8) special case
			wordLength += 3;
		} else {
			// Count all consecutive spaces
			while (pos < text.size() && text[pos] == CHAR_SPACE) {
				wordLength++;
				pos++;
			}
		}
	}
	return wordLength;
}

Common::String joinStrings(const Common::Array<Common::String> &strings, const Common::String &separator) {
	Common::String result;
	for (uint i = 0; i < strings.size(); i++) {
		result += strings[i];
		if (i < strings.size() - 1)
			result += separator;
	}
	return result;
}

Common::Array<Common::Array<Common::String> > wordWrap(Common::String text) {

	Common::Array<Common::Array<Common::String> > pages;
	Common::Array<Common::String> currentPage;
	Common::Array<Common::String> currentLine;
	int charsRemaining = MAX_CHARS_PER_LINE;
	int position = 0;
	int currentLineNum = 0;
	while (position < text.size()) {
		bool isEnd = false;
		int wordLength = calculateWordLength(text, position, isEnd);
		// # Extract the word (including trailing spaces)
		// word = text[position:position + word_length].decode('latin-1', errors='replace')
		Common::String word = text.substr(position, wordLength).decode(Common::kLatin1);
		// # Key decision: if word_length > chars_remaining, wrap to next line
		if (wordLength > charsRemaining) {
			// Word is longer than the entire line - need to split
			currentPage.push_back(joinStrings(currentLine, " "));
			currentLine.clear();
			charsRemaining = MAX_CHARS_PER_LINE;
			currentLineNum++;

			if (currentLineNum >= MAX_LINES) {
				pages.push_back(currentPage);
				currentPage.clear();
				currentLineNum = 0;
			}
		}
		// Add word to current line
		currentLine.push_back(word);
		charsRemaining -= wordLength;

		if (charsRemaining == 0 && isEnd) {
			Common::String lineText = joinStrings(currentLine, "");
			while (lineText.lastChar() == CHAR_SPACE) {
				lineText = lineText.substr(0, lineText.size() - 1);
			}
			int trailingSpaces = currentLine.size() - lineText.size();
			if (trailingSpaces > 0) {
				currentPage.push_back(lineText);
				//  current_line = [' ' * trailing_spaces]
				Common::String currentLine(trailingSpaces, ' ');
				charsRemaining = MAX_CHARS_PER_LINE - trailingSpaces;
				currentLineNum += 1;

				if (currentLineNum >= MAX_LINES) {
					pages.push_back(currentPage);
					currentPage.clear();
					currentLineNum = 0;
				}
			}
		}

		position += wordLength;
		if (isEnd) {
			// End of sentence/paragraph/page
			break;
		}
	}
	if (currentLine.empty() == false) {
		Common::String lineText = joinStrings(currentLine, "");
		while (lineText.lastChar() == CHAR_SPACE) {
			lineText = lineText.substr(0, lineText.size() - 1);
		}
		currentPage.push_back(lineText);
	}
	if (currentPage.empty() == false) {
		pages.push_back(currentPage);
	}
	debug("Word wrap produced %d pages", pages.size());
	for (int i = 0; i < pages.size(); i++) {
		debug(" Page %d:", i);
		for (int j = 0; j < pages[i].size(); j++) {
			debug("   Line %d: %s", j, pages[i][j].c_str());
		}
	}
	return pages;
}

void PelrockEngine::setScreen(int number, int dir) {

	Common::File roomFile;
	if (!roomFile.open(Common::Path("ALFRED.1"))) {
		error("Could not open ALFRED.1");
		return;
	}
	dirAlfred = dir;
	isAlfredWalking = false;
	isAlfredTalking = false;
	_current_step = 0;
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
