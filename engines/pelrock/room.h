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

static const int kNumSfxPerRoom = 8;

class RoomManager {
public:
	RoomManager();
	~RoomManager();
	void loadRoomMetadata(Common::File *roomFile, int roomNumber);
	void loadRoomTalkingAnimations(int roomNumber);
	void getPalette(Common::File *roomFile, int roomOffset, byte *palette);
	void getBackground(Common::File *roomFile, int roomOffset, byte *background);
	void addSticker(Sticker sticker);
	void removeSticker(int index);
	bool hasSticker(int index);
	HotSpot *findHotspotByExtra(uint16 extra);
	PaletteAnim *getPaletteAnimForRoom(int roomNumber);

	Common::String getRoomName(int roomNumber) {
		if (roomNumber >= 0 && roomNumber < _roomNames.size()) {
			return _roomNames[roomNumber];
		}
		return "Unknown Room";
	}

	byte _currentRoomNumber = 0;
	Common::Array<HotSpot> _currentRoomHotspots;
	Common::Array<Sprite> _currentRoomAnims;
	Common::Array<Exit> _currentRoomExits;
	Common::Array<WalkBox> _currentRoomWalkboxes;
	Common::Array<Description> _currentRoomDescriptions;

	TalkingAnimHeader _talkingAnimHeader;
	ScalingParams _scaleParams;
	byte *_pixelsShadows = nullptr;
	byte _roomPalette[768];
	byte alfredRemap[256];
	byte overlayRemap[256];
	byte _musicTrack = 0;
	Common::Array<byte> _roomSfx;
	PaletteAnim *_currentPaletteAnim = nullptr;
	Common::Array<Sticker> _currentRoomStickers;
	byte *_conversationData = nullptr;
	size_t _conversationDataSize = 0;

private:
	Common::Array<Sprite> loadRoomAnimations(Common::File *roomFile, int roomOffset);
	Common::Array<HotSpot> loadHotspots(Common::File *roomFile, int roomOffset);
	Common::Array<Exit> loadExits(Common::File *roomFile, int roomOffset);
	Common::Array<WalkBox> loadWalkboxes(Common::File *roomFile, int roomOffset);
	Common::Array<Description> loadRoomTexts(Common::File *roomFile, int roomOffset);

	ScalingParams loadScalingParams(Common::File *roomFile, int roomOffset);
	byte *loadShadowMap(int roomNumber);
	void loadRemaps(int roomNumber);
	Common::StringArray loadRoomNames();
	byte loadMusicTrackForRoom(Common::File *roomFile, int roomOffset);
	Common::Array<byte> loadRoomSfx(Common::File *roomFile, int roomOffset);

	Common::StringArray _roomNames;
};

} // End of namespace Pelrock

#endif
