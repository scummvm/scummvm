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

#include "pelrock/room.h"
#include "pelrock/pelrock.h"
#include "pelrock/util.h"

namespace Pelrock {

RoomManager::RoomManager() {
}

RoomManager::~RoomManager() {
	// delete[] _currentRoomHotspots;
	// delete[] _currentRoomAnims;
	// delete[] _currentRoomExits;
	// delete[] _currentRoomWalkboxes;
	// delete[] _currentRoomDescriptions;
	// delete[] _currentRoomConversations;
}

void RoomManager::getPalette(Common::File *roomFile, int roomOffset, byte *palette) {
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

void RoomManager::getBackground(Common::File *roomFile, int roomOffset, byte *background) {
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

Common::Array<Exit> RoomManager::loadExits(Common::File *roomFile, int roomOffset) {
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

Common::Array<HotSpot> RoomManager::loadHotspots(Common::File *roomFile, int roomOffset) {
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
		HotSpot spot;
		spot.type = roomFile->readByte();
		spot.x = roomFile->readUint16LE();
		spot.y = roomFile->readUint16LE();
		spot.w = roomFile->readByte();
		spot.h = roomFile->readByte();
		spot.extra = roomFile->readUint16LE();
		debug("Hotspot %d: type=%d x=%d y=%d w=%d h=%d extra=%d", i, spot.type, spot.x, spot.y, spot.w, spot.h, spot.extra);
		hotspots.push_back(spot);
	}
	return hotspots;
	// uint32_t hover_areas_start = pair10_data_offset + 0x1BE;
	// roomFile->seek(hover_areas_start, SEEK_SET);
}

void RoomManager::loadRoomMetadata(Common::File *roomFile, int roomOffset) {
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
	for (int i = 0; i < anims.size(); i++) {

		HotSpot thisHotspot;
		thisHotspot.index = i;
		thisHotspot.x = anims[i].x;
		thisHotspot.y = anims[i].y;
		thisHotspot.w = anims[i].w;
		thisHotspot.h = anims[i].h;
		thisHotspot.extra = anims[i].extra;
		thisHotspot.type = anims[i].actionFlags;
		thisHotspot.isEnabled = !anims[i].isDisabled;
		hotspots.push_back(thisHotspot);
	}

	Common::Array<HotSpot> staticHotspots = loadHotspots(roomFile, roomOffset);
	Common::Array<Exit> exits = loadExits(roomFile, roomOffset);

	Common::Array<WalkBox> walkboxes = loadWalkboxes(roomFile, roomOffset);

	debug("total descriptions = %d, anims = %d, hotspots = %d", descriptions.size(), anims.size(), staticHotspots.size());
	for (int i = 0; i < staticHotspots.size(); i++) {
		HotSpot hotspot = staticHotspots[i];
		hotspot.index = anims.size() + i;
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
		drawRect(g_engine->_screen, hotspot.x, hotspot.y, hotspot.w, hotspot.h, 200 + i);
	}

	for (int i = 0; i < _currentRoomExits.size(); i++) {
		Exit exit = _currentRoomExits[i];
		// drawRect(_screen, exit.x, exit.y, exit.w, exit.h, 100 + i);
	}
}
Common::Array<AnimSet> RoomManager::loadRoomAnimations(Common::File *roomFile, int roomOffset) {
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
		animSet.index = i;
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

Common::Array<WalkBox> RoomManager::loadWalkboxes(Common::File *roomFile, int roomOffset) {
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

Common::Array<Description> RoomManager::loadRoomDescriptions(Common::File *roomFile, int roomOffset, uint32_t &outPos) {
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
		int desc_pos = 0;
		if (data[pos] == 0xFF) {
			Description description;
			description.itemId = data[pos + 1];
			pos += 3;
			description.index = data[pos++];
			description.text = "";
			// debug("Found description terminator");
			while (pos < (pair12_size) && data[pos] != 0xFD && pos < (pair12_size)) {
				// debug(" char: %c", data[pos]);
				if (data[pos] != 0x00) {
					description.text.append(1, (char)data[pos]);
				}
				if (data[pos] == 0xF8) {
					description.actionTrigger = data[pos + 1] | data[pos + 2] << 8;
					debug("Found action trigger: %d", description.actionTrigger);
					pos += 2;
					break;
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

void RoomManager::loadRoomTalkingAnimations(int roomNumber) {

	int headerIndex = roomNumber;
	uint32 offset = kTalkingAnimHeaderSize * headerIndex;

	TalkinAnimHeader talkHeader;
	Common::File talkFile;
	if (!talkFile.open("ALFRED.2")) {
		error("Couldnt find file ALFRED.2");
	}

	talkFile.seek(offset, SEEK_SET);

	talkHeader.spritePointer = talkFile.readUint32LE();
	talkFile.read(&talkHeader.unknown2, 3);
	talkHeader.offsetXAnimA = talkFile.readByte();
	talkHeader.offsetYAnimA = talkFile.readByte();
	talkHeader.wAnimA = talkFile.readByte();
	talkHeader.hAnimA = talkFile.readByte();
	talkFile.read(&talkHeader.unknown3, 2);
	talkHeader.numFramesAnimA = talkFile.readByte();
	talkFile.read(&talkHeader.unknown4, 5);

	talkHeader.offsetXAnimB = talkFile.readByte();
	talkHeader.offsetYAnimB = talkFile.readByte();
	talkHeader.wAnimB = talkFile.readByte();
	talkHeader.hAnimB = talkFile.readByte();
	talkFile.read(&talkHeader.unknown5, 2);
	talkHeader.numFramesAnimB = talkFile.readByte();
	talkFile.read(&talkHeader.unknown6, 29);
	debug("Talking anim header for room %d: spritePointer=%d, wA=%d, hA=%d, framesA=%d, wB=%d, hB=%d, framesB=%d", roomNumber, talkHeader.spritePointer, talkHeader.wAnimA, talkHeader.hAnimA, talkHeader.numFramesAnimA, talkHeader.wAnimB, talkHeader.hAnimB, talkHeader.numFramesAnimB);

	if (talkHeader.spritePointer == 0) {
		debug("No talking animation for room %d", roomNumber);
		talkFile.close();
		return;
	}

	// if(talkHeader.animA != nullptr) {
	// 	delete[] talkHeader.animA;
	// 	talkHeader.animA = nullptr;
	// }
	talkHeader.animA = new byte *[talkHeader.numFramesAnimA];

	byte *data = nullptr;
	int animASize = talkHeader.wAnimA * talkHeader.hAnimA * talkHeader.numFramesAnimA;
	byte *decompressed = nullptr;
	size_t dataSize = 0;
	readUntilBuda(&talkFile, talkHeader.spritePointer, data, dataSize);
	size_t decompressedSize = rleDecompress(data, dataSize, 0, dataSize, &decompressed);
	free(data);
	debug("Decompressed talking anim A size: %zu, decompressed size: %zu", dataSize, decompressedSize);
	for (int i = 0; i < talkHeader.numFramesAnimA; i++) {
		talkHeader.animA[i] = new byte[talkHeader.wAnimA * talkHeader.hAnimA];
		extractSingleFrame(decompressed, talkHeader.animA[i], i, talkHeader.wAnimA, talkHeader.hAnimA);

	}

	if (talkHeader.numFramesAnimB > 0) {
		// if(talkHeader.animA != nullptr) {
		// 	delete[] talkHeader.animA;
		// 	talkHeader.animA = nullptr;
		// }
		talkHeader.animB = new byte *[talkHeader.numFramesAnimB];
		for (int i = 0; i < talkHeader.numFramesAnimB; i++) {
			talkHeader.animB[i] = new byte[talkHeader.wAnimB * talkHeader.hAnimB];
			extractSingleFrame(decompressed + animASize, talkHeader.animB[i], i, talkHeader.wAnimB, talkHeader.hAnimB);
		}
	}
	free(decompressed);
	_talkingAnimHeader = talkHeader;

	talkFile.close();
}

Common::String RoomManager::getControlName(byte b) {
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

Common::String RoomManager::cleanText(const Common::String &text) {
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

Common::Array<ConversationElement> RoomManager::parseConversationElements(const byte *convData, uint32 size) {
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

Common::Array<ConversationNode> RoomManager::buildTreeStructure(const Common::Array<ConversationElement> &elements) {
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
				root.speaker = "NPC";
				root.speakerId = elem.speakerId;
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
				choiceNode.speaker = "ALFRED";
				choiceNode.speakerId = 0x0D; // Player
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

Common::Array<ConversationNode> RoomManager::loadConversations(Common::File *roomFile, int roomOffset, uint32_t startPos) {

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


} // End of namespace Pelrock
