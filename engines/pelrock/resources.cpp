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

#include "pelrock/resources.h"
#include "pelrock/pelrock.h"
#include "pelrock/util.h"

namespace Pelrock {

ResourceManager::ResourceManager() {
}

ResourceManager::~ResourceManager() {
}

Common::Array<Exit> ResourceManager::loadExits(Common::File *roomFile, int roomOffset) {
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

Common::Array<HotSpot> ResourceManager::loadHotspots(Common::File *roomFile, int roomOffset) {
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

void ResourceManager::loadRoomMetadata(Common::File *roomFile, int roomOffset) {
	uint32_t outPos = 0;

	Common::Array<Description> descriptions = loadRoomDescriptions(roomFile, roomOffset, outPos);
	debug("After decsriptions, position is %d", outPos);
	// Common::Array<ConversationNode> roots = loadConversations(roomFile, roomOffset, outPos);
	// for (int i = 0; i < roots.size(); i++) {
	// 	if (roots[i].text.empty()) {
	// 		continue;
	// 	}
	// 	debug("Conversation %d: %s", i, roots[i].text.c_str());
	// }
	// g_engine->_currentRoomConversations = roots;

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

	g_engine->_currentRoomAnims = anims;
	g_engine->_currentRoomHotspots = hotspots;
	g_engine->_currentRoomExits = exits;
	g_engine->_currentRoomWalkboxes = walkboxes;
	g_engine->_currentRoomDescriptions = descriptions;
	for (int i = 0; i < g_engine->_currentRoomHotspots.size(); i++) {
		HotSpot hotspot = g_engine->_currentRoomHotspots[i];
		drawRect(g_engine->_screen, hotspot.x, hotspot.y, hotspot.w, hotspot.h, 200 + i);
	}

	for (int i = 0; i < g_engine->_currentRoomExits.size(); i++) {
		Exit exit = g_engine->_currentRoomExits[i];
		// drawRect(_screen, exit.x, exit.y, exit.w, exit.h, 100 + i);
	}
}
Common::Array<AnimSet> ResourceManager::loadRoomAnimations(Common::File *roomFile, int roomOffset) {
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

Common::Array<WalkBox> ResourceManager::loadWalkboxes(Common::File *roomFile, int roomOffset) {
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

Common::Array<Description> ResourceManager::loadRoomDescriptions(Common::File *roomFile, int roomOffset, uint32_t &outPos) {
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

} // End of namespace Pelrock
