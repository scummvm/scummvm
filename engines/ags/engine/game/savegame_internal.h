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

#ifndef AGS_ENGINE_GAME_SAVEGAME_INTERNAL_H
#define AGS_ENGINE_GAME_SAVEGAME_INTERNAL_H

#include "ags/lib/std/memory.h"
#include "ags/lib/std/vector.h"
#include "ags/shared/ac/common_defines.h"
#include "ags/shared/game/room_struct.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/media/audio/audio_defines.h"

namespace AGS3 {
namespace AGS {
namespace Engine {

using AGS::Shared::Bitmap;

typedef std::shared_ptr<Bitmap> PBitmap;

// PreservedParams keeps old values of particular gameplay
// parameters that are saved before the save restoration
// and either applied or compared to new values after
// loading save data
struct PreservedParams {
	// Whether speech and audio packages available
	bool SpeechVOX = false;
	bool MusicVOX = false;
	// Game options, to preserve ones that must not change at runtime
	int GameOptions[GameSetupStructBase::MAX_OPTIONS]{};
	// Script global data sizes
	size_t GlScDataSize = 0u;
	std::vector<size_t> ScMdDataSize;

	PreservedParams();
};

enum GameViewCamFlags {
	kSvgGameAutoRoomView = 0x01
};

enum CameraSaveFlags {
	kSvgCamPosLocked = 0x01
};

enum ViewportSaveFlags {
	kSvgViewportVisible = 0x01
};

// RestoredData keeps certain temporary data to help with
// the restoration process
struct RestoredData {
	int                     FPS;
	// Unserialized bitmaps for dynamic surfaces
	std::vector<Bitmap *>    DynamicSurfaces;
	// Scripts global data
	struct ScriptData {
		std::vector<char>	Data;
		size_t              Len;

		ScriptData();
	};
	ScriptData              GlobalScript;
	std::vector<ScriptData> ScriptModules;
	// Room data (has to be be preserved until room is loaded)
	PBitmap                 RoomBkgScene[MAX_ROOM_BGFRAMES];
	int16_t                 RoomLightLevels[MAX_ROOM_REGIONS];
	int32_t                 RoomTintLevels[MAX_ROOM_REGIONS];
	int16_t                 RoomZoomLevels1[MAX_WALK_AREAS + 1];
	int16_t                 RoomZoomLevels2[MAX_WALK_AREAS + 1];
	RoomVolumeMod           RoomVolume;
	// Mouse cursor parameters
	int                     CursorID;
	int                     CursorMode;
	// General audio
	struct ChannelInfo {
		int ClipID = -1;
		int Pos = 0;
		int Priority = 0;
		int Repeat = 0;
		int Vol = 0;
		int VolAsPercent = 0;
		int Pan = 0;
		int Speed = 0;
		// since version 1
		int XSource = -1;
		int YSource = -1;
		int MaxDist = 0;
	};
	ChannelInfo             AudioChans[TOTAL_AUDIO_CHANNELS];
	// Ambient sounds
	int                     DoAmbient[MAX_GAME_CHANNELS];
	// Viewport and camera data, has to be preserved and applied only after
	// room gets loaded, because we must clamp these to room parameters.
	struct ViewportData {
		int ID = -1;
		int Flags = 0;
		int Left = 0;
		int Top = 0;
		int Width = 0;
		int Height = 0;
		int ZOrder = 0;
		int CamID = -1;
	};
	struct CameraData {
		int ID = -1;
		int Flags = 0;
		int Left = 0;
		int Top = 0;
		int Width = 0;
		int Height = 0;
	};
	std::vector<ViewportData> Viewports;
	std::vector<CameraData> Cameras;
	int32_t Camera0_Flags = 0; // flags for primary camera, when data is read in legacy order

	RestoredData();
};

} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif
