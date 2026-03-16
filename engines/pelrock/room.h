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

static const int kRoomStructSize = 104;
static const int kTalkingAnimHeaderSize = 55;
static const int kNumSfxPerRoom = 9;
static const int unpickableHotspotExtras[] = {
	308, // lamppost cable
	65,  // objects in shop
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
	91, // mud and stone should only be picked under certain conditions!
	92};

static const uint32 stickerOffsets[137] = {
	0x000000, 0x00005B, 0x0000B6, 0x000298, 0x00047A, 0x0023C8, 0x004316, 0x004376,
	0x005119, 0x005EBC, 0x0083ED, 0x008529, 0x0092C4, 0x00A3AA, 0x00B490, 0x00B6A6,
	0x00C05A, 0x00CA0E, 0x00D3D0, 0x00D46E, 0x00F036, 0x00FB8F, 0x00FC55, 0x0119D7,
	0x013759, 0x01391F, 0x014A9D, 0x015C1B, 0x017601, 0x018FE7, 0x019048, 0x0190A9,
	0x01910A, 0x0197F4, 0x019EDE, 0x01A7EC, 0x01B0FA, 0x01B8C4, 0x01C644, 0x01D83A,
	0x01E104, 0x01E8C6, 0x01F45D, 0x01FBBB, 0x02011D, 0x02052F, 0x020A95, 0x020E5B,
	0x0210B3, 0x0216E6, 0x021D5E, 0x0233A3, 0x0249E8, 0x025777, 0x026506, 0x028E2B,
	0x02B82F, 0x02C9D7, 0x02E4CA, 0x02FFBD, 0x03234A, 0x0346D7, 0x036A83, 0x038E2F,
	0x03B18D, 0x03D4EB, 0x03DEC9, 0x03F813, 0x04115D, 0x045303, 0x0494A9, 0x04955F,
	0x049615, 0x0496CB, 0x0499E1, 0x049EC7, 0x04A023, 0x04A447, 0x04BA6D, 0x04BFA1,
	0x04CE33, 0x04CF09, 0x04DB3B, 0x052885, 0x0575CF, 0x05775B, 0x057D79, 0x058397,
	0x058969, 0x058F50, 0x05A9DB, 0x05C561, 0x05C72E, 0x05C8FB, 0x05EAC1, 0x060C87,
	0x060D19, 0x060E62, 0x061039, 0x0613C2, 0x061764, 0x061847, 0x062535, 0x062D4B,
	0x064F11, 0x0670D7, 0x067381, 0x0675A9, 0x0677EF, 0x067A98, 0x067DDE, 0x068115,
	0x0684E3, 0x068A76, 0x068F30, 0x0693C8, 0x0696AD, 0x06C2C9, 0x06C84D, 0x07095D,
	0x071854, 0x07274B, 0x073642, 0x074539, 0x075454, 0x0791DA, 0x07CF60, 0x07E4AB,
	0x07ECED, 0x07F52F, 0x07FD71, 0x080591, 0x080B24, 0x080B84, 0x080F39, 0x0812F5,
	0x0816B1
};

#define PERSIST_TEMP 1
#define PERSIST_PERM 2
#define PERSIST_BOTH 3

class RoomManager {
public:
	RoomManager();
	~RoomManager();
	void clearAnims();
	void loadRoomMetadata(Common::File *roomFile, int roomNumber);
	/**
	 * Passer by animations are animations of characters that merely traverse the scene as ambient
	 */
	RoomPasserBys *loadPasserByAnims(int roomNumber);
	/**
	 * Treats sprites and hotspots the same.
	 */
	Common::Array<HotSpot> unifyHotspots(Common::Array<Pelrock::Sprite> &anims, Common::Array<Pelrock::HotSpot> &staticHotspots);
	void loadRoomTalkingAnimations(int roomNumber);
	void getPalette(Common::File *roomFile, int roomOffset, byte *palette);
	void getBackground(Common::File *roomFile, int roomOffset, byte *background);
	void loadWaterPaletteRemap();

	/** Methods to modify room data at runtime **/
	void addSticker(int stickerId, int persist = PERSIST_BOTH);
	void addStickerToRoom(byte room, int stickerId, int persist = PERSIST_BOTH);

	void removeSticker(int stickerId);
	void removeStickerFromRoom(byte room, int stickerId);

	bool hasSticker(int index);
	bool hasSticker(byte room, int index);

	void changeExit(byte index, bool enabled, int persist = PERSIST_BOTH);
	void changeExit(byte room, byte index, bool enabled, int persist = PERSIST_BOTH);

	void disableExit(byte index, int persist = PERSIST_BOTH);
	void disableExit(byte room, byte index, int persist = PERSIST_BOTH);
	void enableExit(byte index, int persist = PERSIST_BOTH);
	void enableExit(byte room, byte index, int persist = PERSIST_BOTH);

	void changeWalkBox(WalkBox walkbox, int persist = PERSIST_BOTH);
	void changeWalkbox(byte room, WalkBox walkbox, int persist = PERSIST_BOTH);

	void changeHotSpot(HotSpot hotspot, int persist = PERSIST_BOTH);
	void changeHotspot(byte room, HotSpot hotspot, int persist = PERSIST_BOTH);

	void disableSprite(byte spriteIndex, int persist = PERSIST_BOTH);
	void disableSprite(byte roomNumber, byte spriteIndex, int persist = PERSIST_BOTH);
	void enableSprite(byte spriteIndex, byte zOrder, int persist = PERSIST_BOTH);
	void enableSprite(byte roomNumber, byte spriteIndex, byte zOrder, int persist = PERSIST_BOTH);
	/**
	 * Utility function to enable or disable a hotspot, with an option to persist the change.
	 */
	void enableHotspot(HotSpot *hotspot, int persist = PERSIST_BOTH);
	void enableHotspot(byte room, HotSpot *hotspot, int persist = PERSIST_BOTH);

	void disableHotspot(HotSpot *hotspot, int persist = PERSIST_BOTH);
	void disableHotspot(byte room, HotSpot *hotspot, int persist = PERSIST_BOTH);

	void moveHotspot(HotSpot *hotspot, int16 newX, int16 newY, int persist = PERSIST_BOTH);
	void moveHotspot(byte room, HotSpot *hotspot, int16 newX, int16 newY, int persist = PERSIST_BOTH);
	void setActionMask(HotSpot *hotspot, byte actionMask, int persist = PERSIST_BOTH);

	void addWalkbox(WalkBox walkbox, int persist = PERSIST_BOTH);
	void addWalkbox(byte room, WalkBox walkbox, int persist = PERSIST_BOTH);

	void applyDisabledChoices(byte roomNumber, byte *conversationData, size_t conversationDataSize);
	void applyDisabledChoice(ResetEntry entry, byte *conversationData, size_t conversationDataSize);
	void addDisabledChoice(ChoiceOption choice);

	/**
	 * Will apply the default "take item with given extra" handler if returns true
	 */
	bool isPickableByExtra(uint16 extra) {
		if (extra > 112)
			return false;
		int size = sizeof(unpickableHotspotExtras) / sizeof(unpickableHotspotExtras[0]);
		for (int i = 0; i < size; i++) {
			if (extra == unpickableHotspotExtras[i])
				return false;
		}
		return true;
	}

	Sprite *findSpriteByIndex(byte index);
	Sprite *findSpriteByExtra(int16 extra);
	HotSpot *findHotspotByIndex(byte index);
	HotSpot *findHotspotByExtra(uint16 extra);
	PaletteAnim *getPaletteAnimForRoom(int roomNumber);

	byte _currentRoomNumber = 0;
	int _prevRoomNumber = -1;
	Common::Array<HotSpot> _currentRoomHotspots;
	Common::Array<Sprite> _currentRoomAnims;
	Common::Array<Exit> _currentRoomExits;
	Common::Array<WalkBox> _currentRoomWalkboxes;
	Common::Array<Description> _currentRoomDescriptions;

	TalkingAnims _talkingAnims;
	ScalingParams _scaleParams;
	byte *_pixelsShadows = nullptr;
	byte _roomPalette[768];
	byte _paletteRemaps[5][256];
	byte _musicTrack = 0;
	Common::Array<byte> _roomSfx;
	PaletteAnim *_currentPaletteAnim = nullptr;
	RoomPasserBys *_passerByAnims = nullptr;
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

	byte *_resetData = nullptr;
};

} // End of namespace Pelrock

#endif
