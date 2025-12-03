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
#ifndef PELROCK_ROOM_H
#define PELROCK_ROOM_H

#include "common/array.h"
#include "common/file.h"
#include "common/scummsys.h"

#include "pelrock/types.h"

namespace Pelrock {

class RoomManager {
public:
	RoomManager();
	~RoomManager();
	void loadRoomMetadata(Common::File *roomFile, int roomNumber);
	void loadRoomTalkingAnimations(int roomNumber);
	void getPalette(Common::File *roomFile, int roomOffset, byte *palette);
	void getBackground(Common::File *roomFile, int roomOffset, byte *background);
	Common::String getRoomName(int roomNumber) {
		if (roomNumber >= 0 && roomNumber < _roomNames.size()) {
			return _roomNames[roomNumber];
		}
		return "Unknown Room";
	}

	Common::Array<HotSpot> _currentRoomHotspots;
	Common::Array<AnimSet> _currentRoomAnims;
	Common::Array<Exit> _currentRoomExits;
	Common::Array<WalkBox> _currentRoomWalkboxes;
	Common::Array<Description> _currentRoomDescriptions;
	Common::Array<ConversationNode> _currentRoomConversations;
	TalkingAnimHeader _talkingAnimHeader;
	ScalingParams _scaleParams;
	byte *_pixelsShadows = nullptr;
	byte _roomPalette[768];
	byte alfredRemap[256];
	byte overlayRemap[256];
	Common::Array<Common::String> _roomNames;
	byte _musicTrack = 0;
	Common::Array<int> _roomSfx;

private:
	Common::Array<AnimSet> loadRoomAnimations(Common::File *roomFile, int roomOffset);
	Common::Array<HotSpot> loadHotspots(Common::File *roomFile, int roomOffset);
	Common::Array<Exit> loadExits(Common::File *roomFile, int roomOffset);
	Common::Array<WalkBox> loadWalkboxes(Common::File *roomFile, int roomOffset);
	Common::Array<Description> loadRoomDescriptions(Common::File *roomFile, int roomOffset, uint32_t &outPos);

	Common::String getControlName(byte b);
	Common::String cleanText(const Common::String &text);
	Common::Array<ConversationElement> parseConversationElements(const byte *convData, uint32 size);
	Common::Array<ConversationNode> buildTreeStructure(const Common::Array<ConversationElement> &elements);
	Common::Array<ConversationNode> loadConversations(Common::File *roomFile, int roomOffset, uint32_t startPos);
	ScalingParams loadScalingParams(Common::File *roomFile, int roomOffset);
	byte *loadShadowMap(int roomNumber);
	void loadRemaps(int roomNumber);
	Common::Array<Common::String> loadRoomNames();
	byte loadMusicTrackForRoom(Common::File *roomFile, int roomOffset);
	Common::Array<int> loadRoomSfx(Common::File *roomFile, int roomOffset);
};

} // End of namespace Pelrock

#endif
