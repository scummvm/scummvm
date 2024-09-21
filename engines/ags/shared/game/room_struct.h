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

//
// RoomStruct, a class describing initial room data.
//
// Because of the imperfect implementation there is inconsistency in how
// this data is interpreted at the runtime.
// Some of that data is never supposed to be changed at runtime. Another
// may be changed, but these changes are lost as soon as room is unloaded.
// The changes that must remain in memory are kept as separate classes:
// see RoomStatus, RoomObject etc.
//
// Partially this is because same class was used for both engine and editor,
// while runtime code was not available for the editor.
//
// This is also the reason why some classes here are named with the "Info"
// postfix. For example, RoomObjectInfo is the initial object data, and
// there is also RoomObject runtime-only class for mutable data.
//
// [ivan-mogilko] In my opinion, eventually there should be only one room class
// and one class per room entity, regardless of whether code is shared with
// the editor or not. But that would require extensive refactor/rewrite of
// the engine code, and savegame read/write code.
//
//=============================================================================

#ifndef AGS_SHARED_GAME_ROOM_INFO_H
#define AGS_SHARED_GAME_ROOM_INFO_H

#include "common/std/memory.h"
#include "ags/lib/allegro.h" // RGB
#include "ags/shared/ac/common_defines.h"
#include "ags/shared/game/interactions.h"
#include "ags/shared/util/geometry.h"
#include "ags/shared/util/string.h"

namespace AGS3 {

struct ccScript;
struct SpriteInfo;
typedef std::shared_ptr<ccScript> PScript;

// TODO: move the following enums under AGS::Shared namespace
// later, when more engine source is put in AGS namespace and
// refactored.

// Room's area mask type
enum RoomAreaMask {
	kRoomAreaNone = 0,
	kRoomAreaHotspot,
	kRoomAreaWalkBehind,
	kRoomAreaWalkable,
	kRoomAreaRegion
};

// Room's audio volume modifier
enum RoomVolumeMod {
	kRoomVolumeQuietest = -3,
	kRoomVolumeQuieter = -2,
	kRoomVolumeQuiet = -1,
	kRoomVolumeNormal = 0,
	kRoomVolumeLoud = 1,
	kRoomVolumeLouder = 2,
	kRoomVolumeLoudest = 3,
	// These two options are only settable at runtime by SetMusicVolume()
	kRoomVolumeExtra1 = 4,
	kRoomVolumeExtra2 = 5,

	kRoomVolumeMin = kRoomVolumeQuietest,
	kRoomVolumeMax = kRoomVolumeExtra2,
};

// Extended room boolean options
enum RoomFlags {
	kRoomFlag_BkgFrameLocked = 0x01
};

// Flag tells that walkable area does not have continious zoom
#define NOT_VECTOR_SCALED  -10000
// Flags tells that room is not linked to particular game ID
#define NO_GAME_ID_IN_ROOM_FILE 16325

#define MAX_ROOM_BGFRAMES  5   // max number of frames in animating bg scene

#define MAX_ROOM_HOTSPOTS  50  // v2.62: 20 -> 30; v2.8: -> 50
#define MAX_ROOM_OBJECTS_v300 40 // for some legacy logic support
#define MAX_ROOM_OBJECTS   256 // v3.6.0: 40 -> 256 (now limited by room format)
#define MAX_ROOM_REGIONS   16
#define MAX_WALK_AREAS     16
#define MAX_WALK_BEHINDS   16

#define MAX_MESSAGES       100


namespace AGS {
namespace Shared {

class Bitmap;
class Stream;

typedef std::shared_ptr<Bitmap> PBitmap;

// Various room options
struct RoomOptions {
	// Index of the startup music in the room
	// this is a deprecated option, used before 3.2.* with old audio API.
	int  StartupMusic;
	// If saving and loading game is disabled in the room;
	// this is a deprecated option that affects only built-in save/load dialogs
	bool SaveLoadDisabled;
	// If player character is turned off in the room
	bool PlayerCharOff;
	// Apply player character's normal view when entering this room
	int  PlayerView;
	// Room's music volume modifier
	RoomVolumeMod MusicVolume;
	// A collection of RoomFlags
	int  Flags;

	RoomOptions();
};

// Single room background frame
struct RoomBgFrame {
	PBitmap     Graphic;
	// Palette is only valid in 8-bit games
	RGB         Palette[256];
	// Tells if this frame should keep previous frame palette instead of using its own
	bool        IsPaletteShared;

	RoomBgFrame();
};

// Describes room edges (coordinates of four edges)
struct RoomEdges {
	int32_t Left;
	int32_t Right;
	int32_t Top;
	int32_t Bottom;

	RoomEdges();
	RoomEdges(int l, int r, int t, int b);
};

// Room hotspot description
struct RoomHotspot {
	String      Name;
	String      ScriptName;
	// Custom properties
	StringIMap  Properties;
	// Old-style interactions
	PInteraction Interaction;
	// Event script links
	PInteractionScripts EventHandlers;

	// Player will automatically walk here when interacting with hotspot
	Point       WalkTo;
};

// Room object description
struct RoomObjectInfo {
	int32_t         Room;
	int32_t         X;
	int32_t         Y;
	int32_t         Sprite;
	bool            IsOn;
	// Object's z-order in the room, or -1 (use Y)
	int32_t         Baseline;
	int32_t         Flags;
	String          Name;
	String          ScriptName;
	// Custom properties
	StringIMap      Properties;
	// Old-style interactions
	PInteraction    Interaction;
	// Event script links
	PInteractionScripts EventHandlers;

	RoomObjectInfo();
};

// Room region description
struct RoomRegion {
	// Light level (-100 -> +100) or Tint luminance (0 - 255)
	int32_t         Light;
	// Tint setting (R-B-G-S)
	int32_t         Tint;
	// Custom properties
	StringIMap      Properties;
	// Old-style interactions
	PInteraction    Interaction;
	// Event script links
	PInteractionScripts EventHandlers;

	RoomRegion();
};

// Walkable area description
struct WalkArea {
	// Apply player character's normal view on this area
	int32_t     CharacterView;
	// Character's scaling (-100 -> +100 %)
	// General scaling, or scaling at the farthest point
	int32_t     ScalingFar;
	// Scaling at the nearest point, or NOT_VECTOR_SCALED for uniform scaling
	int32_t     ScalingNear;
	// Optional override for player character view
	int32_t     PlayerView;
	// Top and bottom Y of the area
	int32_t     Top;
	int32_t     Bottom;

	WalkArea();
};

// Walk-behind description
struct WalkBehind {
	// Object's z-order in the room
	int32_t Baseline;

	WalkBehind();
};

// Room messages

#define MSG_DISPLAYNEXT 0x01 // supercedes using alt-200 at end of message
#define MSG_TIMELIMIT   0x02

struct MessageInfo {
	int8    DisplayAs; // 0 - std display window, >=1 - as character's speech
	int8    Flags; // combination of MSG_xxx flags

	MessageInfo();
};


// Room's legacy resolution type
enum RoomResolutionType {
	kRoomRealRes = 0, // room should always be treated as-is
	kRoomLoRes = 1, // created for low-resolution game
	kRoomHiRes = 2 // created for high-resolution game
};


//
// Description of a single room.
// This class contains initial room data. Some of it may still be modified
// at the runtime, but then these changes get lost as soon as room is unloaded.
//
class RoomStruct {
public:
	RoomStruct();
	~RoomStruct();

	// Gets if room should adjust its base size depending on game's resolution
	inline bool IsRelativeRes() const {
		return _resolution != kRoomRealRes;
	}
	// Gets if room belongs to high resolution
	inline bool IsLegacyHiRes() const {
		return _resolution == kRoomHiRes;
	}
	// Gets legacy resolution type
	inline RoomResolutionType GetResolutionType() const {
		return _resolution;
	}

	// Releases room resources
	void            Free();
	// Release room messages and scripts correspondingly. These two functions are needed
	// at very specific occasion when only part of the room resources has to be freed.
	void            FreeMessages();
	void            FreeScripts();
	// Init default room state
	void            InitDefaults();
	// Set legacy resolution type
	void            SetResolution(RoomResolutionType type);

	// Gets bitmap of particular mask layer
	Bitmap *GetMask(RoomAreaMask mask) const;
	// Gets mask's scale relative to the room's background size
	float   GetMaskScale(RoomAreaMask mask) const;

	// TODO: see later whether it may be more convenient to move these to the Region class instead.
	// Gets if the given region has light level set
	bool HasRegionLightLevel(int id) const;
	// Gets if the given region has a tint set
	bool HasRegionTint(int id) const;
	// Gets region's light level in -100 to 100 range value; returns 0 (default level) if region's tint is set
	int  GetRegionLightLevel(int id) const;
	// Gets region's tint luminance in 0 to 100 range value; returns 0 if region's light level is set
	int  GetRegionTintLuminance(int id) const;

	// TODO: all members are currently public because they are used everywhere; hide them later
public:
	// Game's unique ID, corresponds to GameSetupStructBase::uniqueid.
	// If this field has a valid value and does not match actual game's id,
	// then engine will refuse to start this room.
	// May be set to NO_GAME_ID_IN_ROOM_FILE to let it run within any game.
	int32_t                 GameID;
	// Loaded room file's data version. This value may be used to know when
	// the room must have behavior specific to certain version of AGS.
	int32_t                 DataVersion;

	// Room region masks resolution. Defines the relation between room and mask units.
	// Mask point is calculated as roompt / MaskResolution. Must be >= 1.
	int32_t                 MaskResolution;
	// Size of the room, in logical coordinates (= pixels)
	int32_t                 Width;
	int32_t                 Height;
	// Primary room palette (8-bit games)
	RGB                     Palette[256];

	// Basic room options
	RoomOptions             Options;

	// Background frames
	int32_t                 BackgroundBPP; // bytes per pixel
	size_t                  BgFrameCount;
	RoomBgFrame             BgFrames[MAX_ROOM_BGFRAMES];
	// Speed at which background frames are changing, 0 - no auto animation
	int32_t                 BgAnimSpeed;
	// Edges
	RoomEdges               Edges;
	// Region masks
	PBitmap                 HotspotMask;
	PBitmap                 RegionMask;
	PBitmap                 WalkAreaMask;
	PBitmap                 WalkBehindMask;
	// Room entities
	size_t                  HotspotCount;
	RoomHotspot             Hotspots[MAX_ROOM_HOTSPOTS];
	std::vector<RoomObjectInfo> Objects;
	size_t                  RegionCount;
	RoomRegion              Regions[MAX_ROOM_REGIONS];
	size_t                  WalkAreaCount;
	WalkArea                WalkAreas[MAX_WALK_AREAS];
	size_t                  WalkBehindCount;
	WalkBehind              WalkBehinds[MAX_WALK_BEHINDS];

	// Old numbered room messages (used with DisplayMessage, etc)
	size_t                  MessageCount;
	String                  Messages[MAX_MESSAGES];
	MessageInfo             MessageInfos[MAX_MESSAGES];

	// Custom properties
	StringIMap              Properties;
	// Old-style interactions
	InterVarVector          LocalVariables;
	PInteraction            Interaction;
	// Event script links
	PInteractionScripts     EventHandlers;
	// Compiled room script
	PScript                 CompiledScript;
	// Various extended options with string values, meta-data etc
	StringMap               StrOptions;

private:
	// Room's legacy resolution type, defines relation room and game's resolution
	RoomResolutionType      _resolution;
};


// Loads new room data into the given RoomStruct object
void load_room(const String &filename, RoomStruct *room, bool game_is_hires, const std::vector<SpriteInfo> &sprinfos);
// Checks if it's necessary and upscales low-res room backgrounds and masks for the high resolution game
// NOTE: it does not upscale object coordinates, because that is usually done when the room is loaded
void UpscaleRoomBackground(RoomStruct *room, bool game_is_hires);
// Ensures that all existing room masks match room background size and
// MaskResolution property, resizes mask bitmaps if necessary.
void FixRoomMasks(RoomStruct *room);
// Adjusts bitmap size if necessary and returns either new or old bitmap.
PBitmap FixBitmap(PBitmap bmp, int dst_width, int dst_height);

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
