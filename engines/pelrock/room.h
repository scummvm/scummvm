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
static const int unpickableHotspotExtras[] = {
	308, // lamppost cable
	65, // objects in shop
	66,
	67,
	68,
	69,
	70,
	71,
	72,
	73,
	74,
	6,
	7,
	316, // wires
	357, // mailbox should pick a different hotspot
};

class RoomManager {
public:
	RoomManager();
	~RoomManager();
	void loadRoomMetadata(Common::File *roomFile, int roomNumber);
	Common::Array<HotSpot> unifyHotspots(Common::Array<Pelrock::Sprite> &anims, Common::Array<Pelrock::HotSpot> &staticHotspots);
	void loadRoomTalkingAnimations(int roomNumber);
	void getPalette(Common::File *roomFile, int roomOffset, byte *palette);
	void getBackground(Common::File *roomFile, int roomOffset, byte *background);

	/** Methods to modify room data at runtime **/
	void addSticker(int stickerId, bool persist = true);
	void addStickerToRoom(byte room, int stickerId);
	void onlyPersistSticker(byte room, int stickerId);
	void removeSticker(int index);
	void removeSticker(byte room, int index);
	bool hasSticker(int index);
	bool hasSticker(byte room, int index);
	void changeExit(int index, bool enabled, bool persist = true);
	void changeExit(byte room, int index, bool enabled, bool persist = true);
	void disableExit(int index, bool persist = true);
	void disableExit(byte room, int index, bool persist = true);
	void enableExit(int index, bool persist = true);
	void enableExit(byte room, int index, bool persist = true);
	void changeWalkBox(WalkBox walkbox);
	void changeWalkbox(byte room, WalkBox walkbox);
	void changeHotSpot(HotSpot hotspot);
	void changeHotspot(byte room, HotSpot hotspot);
	void disableSprite(byte roomNumber, int spriteIndex, bool persist = true);
	void enableSprite(int spriteIndex, int zOrder, bool persist = true);
	void enableSprite(byte roomNumber, int spriteIndex, int zOrder, bool persist = true);
	/**
	 * Utility function to enable or disable a hotspot, with an option to persist the change.
	 */
	void enableHotspot(HotSpot *hotspot, bool persist = true);
	void enableHotspot(byte room, HotSpot *hotspot, bool persist = true);
	void disableHotspot(HotSpot *hotspot, bool persist = true);
	void disableHotspot(byte room, HotSpot *hotspot, bool persist = true);
	void moveHotspot(HotSpot *hotspot, int16 newX, int16 newY, bool persist = true);
	void setActionMask(HotSpot *hotspot, byte actionMask);
	void moveHotspot(byte room, HotSpot *hotspot, int16 newX, int16 newY, bool persist = true);
	void addWalkbox(WalkBox walkbox);
	void addWalkbox(byte room, WalkBox walkbox);
	void applyDisabledChoices(byte roomNumber, byte *conversationData, size_t conversationDataSize);
	void applyDisabledChoice(ResetEntry entry, byte *conversationData, size_t conversationDataSize);
	void addDisabledChoice(ChoiceOption choice);
	bool isPickableByExtra(uint16 extra) {
		int size = sizeof(unpickableHotspotExtras) / sizeof(unpickableHotspotExtras[0]);
		for (int i = 0; i < size; i++) {
			if (extra == unpickableHotspotExtras[i])
				return false;
		}
		return true;
	}
	Sprite *findSpriteByIndex(byte index);
	HotSpot *findHotspotByIndex(byte index);
	HotSpot *findHotspotByExtra(uint16 extra);
	PaletteAnim *getPaletteAnimForRoom(int roomNumber);

	Common::String getRoomName(int roomNumber) {
		if (roomNumber >= 0 && (uint)roomNumber < _roomNames.size()) {
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

	TalkingAnims _talkingAnimHeader;
	ScalingParams _scaleParams;
	byte *_pixelsShadows = nullptr;
	byte _roomPalette[768];
	byte paletteRemaps[4][256];
	byte _musicTrack = 0;
	Common::Array<byte> _roomSfx;
	PaletteAnim *_currentPaletteAnim = nullptr;
	byte *_conversationData = nullptr;
	size_t _conversationDataSize = 0;
	Common::Array<Sticker> _roomStickers;
	uint32 _conversationOffset;

private:
	void init();
	void loadAnimationPixelData(Common::File *roomFile, int roomOffset, byte *&buffer, size_t &outSize);
	Common::Array<Sprite> loadRoomAnimations(byte *pixelData, size_t pixelDataSize, byte *data, size_t size);
	Common::Array<HotSpot> loadHotspots(byte *data, size_t size);
	Common::Array<Exit> loadExits(byte *data, size_t size);
	ScalingParams loadScalingParams(byte *data, size_t size);
	Common::Array<WalkBox> loadWalkboxes(byte *data, size_t size);
	uint32 loadDescriptions(byte *pair12data, size_t pair12size, Common::Array<Description> &outDescriptions);
	void loadConversationData(byte *pair12data, size_t pair12size, uint32 startPos, size_t &outConversationDataSize, byte *&outConversationData);
	void resetConversationStates(byte roomNumber, byte *conversationData, size_t conversationDataSize);
	void resetMetadataDefaults(byte room, byte *&data, size_t size);

	byte *loadShadowMap(int roomNumber);
	void loadRemaps(int roomNumber);
	Common::StringArray loadRoomNames();
	byte loadMusicTrackForRoom(Common::File *roomFile, int roomOffset);
	Common::Array<byte> loadRoomSfx(Common::File *roomFile, int roomOffset);

	Common::StringArray _roomNames;
	byte *_resetData = nullptr;
};

} // End of namespace Pelrock

#endif
