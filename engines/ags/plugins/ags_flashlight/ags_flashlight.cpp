/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * of the License, or(at your option) any later version.
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

#include "ags/lib/allegro.h"
#include "ags/plugins/ags_flashlight/ags_flashlight.h"
#include "ags/shared/core/platform.h"
#include "common/str.h"

namespace AGS3 {
namespace Plugins {
namespace AGSFlashlight {

const uint32 Magic = 0xBABE0000;
const uint32 Version = 2;
const uint32 SaveMagic = Magic + Version;

const char *AGSFlashlight::AGS_GetPluginName() {
	return "Flashlight plugin recreation";
}

void AGSFlashlight::AGS_EngineStartup(IAGSEngine *engine) {
	PluginBase::AGS_EngineStartup(engine);

	if (_engine->version < 13)
		_engine->AbortGame("Engine interface is too old, need newer version of AGS.");

	SCRIPT_METHOD(SetFlashlightTint, AGSFlashlight::SetFlashlightTint);
	SCRIPT_METHOD(GetFlashlightTintRed, AGSFlashlight::GetFlashlightTintRed);
	SCRIPT_METHOD(GetFlashlightTintGreen, AGSFlashlight::GetFlashlightTintGreen);
	SCRIPT_METHOD(GetFlashlightTintBlue, AGSFlashlight::GetFlashlightTintBlue);

	SCRIPT_METHOD(GetFlashlightMinLightLevel, AGSFlashlight::GetFlashlightMinLightLevel);
	SCRIPT_METHOD(GetFlashlightMaxLightLevel, AGSFlashlight::GetFlashlightMaxLightLevel);

	SCRIPT_METHOD(SetFlashlightDarkness, AGSFlashlight::SetFlashlightDarkness);
	SCRIPT_METHOD(GetFlashlightDarkness, AGSFlashlight::GetFlashlightDarkness);
	SCRIPT_METHOD(SetFlashlightDarknessSize, AGSFlashlight::SetFlashlightDarknessSize);
	SCRIPT_METHOD(GetFlashlightDarknessSize, AGSFlashlight::GetFlashlightDarknessSize);

	SCRIPT_METHOD(SetFlashlightBrightness, AGSFlashlight::SetFlashlightBrightness);
	SCRIPT_METHOD(GetFlashlightBrightness, AGSFlashlight::GetFlashlightBrightness);
	SCRIPT_METHOD(SetFlashlightBrightnessSize, AGSFlashlight::SetFlashlightBrightnessSize);
	SCRIPT_METHOD(GetFlashlightBrightnessSize, AGSFlashlight::GetFlashlightBrightnessSize);

	SCRIPT_METHOD(SetFlashlightPosition, AGSFlashlight::SetFlashlightPosition);
	SCRIPT_METHOD(GetFlashlightPositionX, AGSFlashlight::GetFlashlightPositionX);
	SCRIPT_METHOD(GetFlashlightPositionY, AGSFlashlight::GetFlashlightPositionY);


	SCRIPT_METHOD(SetFlashlightFollowMouse, AGSFlashlight::SetFlashlightFollowMouse);
	SCRIPT_METHOD(GetFlashlightFollowMouse, AGSFlashlight::GetFlashlightFollowMouse);

	SCRIPT_METHOD(SetFlashlightFollowCharacter, AGSFlashlight::SetFlashlightFollowCharacter);
	SCRIPT_METHOD(GetFlashlightFollowCharacter, AGSFlashlight::GetFlashlightFollowCharacter);
	SCRIPT_METHOD(GetFlashlightCharacterDX, AGSFlashlight::GetFlashlightCharacterDX);
	SCRIPT_METHOD(GetFlashlightCharacterDY, AGSFlashlight::GetFlashlightCharacterDY);
	SCRIPT_METHOD(GetFlashlightCharacterHorz, AGSFlashlight::GetFlashlightCharacterHorz);
	SCRIPT_METHOD(GetFlashlightCharacterVert, AGSFlashlight::GetFlashlightCharacterVert);

	SCRIPT_METHOD(SetFlashlightMask, AGSFlashlight::SetFlashlightMask);
	SCRIPT_METHOD(GetFlashlightMask, AGSFlashlight::GetFlashlightMask);

	_engine->RequestEventHook(AGSE_PREGUIDRAW);
	_engine->RequestEventHook(AGSE_PRESCREENDRAW);
	_engine->RequestEventHook(AGSE_SAVEGAME);
	_engine->RequestEventHook(AGSE_RESTOREGAME);
}

int64 AGSFlashlight::AGS_EngineOnEvent(int event, NumberPtr data) {
	if (event == AGSE_PREGUIDRAW) {
		Update();
	} else if (event == AGSE_RESTOREGAME) {
		Serializer s(_engine, data, true);
		syncGame(s);
	} else if (event == AGSE_SAVEGAME) {
		Serializer s(_engine, data, false);
		syncGame(s);
	} else if (event == AGSE_PRESCREENDRAW) {
		// Get screen size once here.
		_engine->GetScreenDimensions(&screen_width, &screen_height, &screen_color_depth);

		// TODO: There's no reliable way to figure out if a game is running in legacy upscale mode from the
		// plugin interface, so for now let's just play it conservatively and check it per-game
		AGSGameInfo *gameInfo = new AGSGameInfo;
		gameInfo->Version = 26;
		_engine->GetGameInfo(gameInfo);
		if (gameInfo->UniqueId == 1050154255 || // MMD, MMM04, MMM13, MMM28, MMM46, MMM56, MMM57, MMM68, MMM78, MMMD9, MMMH5
			gameInfo->UniqueId == 1161197869)   // MMM70
			g_ScaleFactor = (screen_width > 320) ? 2 : 1;

		delete gameInfo;
		_engine->UnrequestEventHook(AGSE_PRESCREENDRAW);
	}

	return 0;
}

void AGSFlashlight::syncGame(Serializer &s) {
	uint32 SaveVersion = SaveMagic;
	s.syncAsInt(SaveVersion);

	if (s.isLoading() && SaveVersion != SaveMagic) {
		// The real AGSFlashlight, or at least the one included with
		// Maniac Mansion Deluxe, doesn't persist any fields.
		// So in such a case, revert the 4 bytes and skip everything else
		s.unreadInt();

	} else {
		s.syncAsInt(g_RedTint);
		s.syncAsInt(g_GreenTint);
		s.syncAsInt(g_BlueTint);

		s.syncAsInt(g_DarknessLightLevel);
		s.syncAsInt(g_BrightnessLightLevel);
		s.syncAsInt(g_DarknessSize);
		s.syncAsInt(g_DarknessDiameter);
		s.syncAsInt(g_BrightnessSize);

		s.syncAsInt(g_FlashlightX);
		s.syncAsInt(g_FlashlightY);

		s.syncAsInt(g_FlashlightFollowMouse);

		s.syncAsInt(g_FollowCharacterId);
		s.syncAsInt(g_FollowCharacterDx);
		s.syncAsInt(g_FollowCharacterDy);
		s.syncAsInt(g_FollowCharacterHorz);
		s.syncAsInt(g_FollowCharacterVert);

		if (s.isLoading()) {
			if (g_FollowCharacterId != 0)
				g_FollowCharacter = _engine->GetCharacter(g_FollowCharacterId);

			g_BitmapMustBeUpdated = true;
		}
	}
}

void AGSFlashlight::SetFlashlightTint(ScriptMethodParams &params) {
	PARAMS3(int, RedTint, int, GreenTint, int, BlueTint);
	ClipToRange(RedTint, -31, 31);
	ClipToRange(GreenTint, -31, 31);
	ClipToRange(BlueTint, -31, 31);

	if ((RedTint != g_RedTint) || (GreenTint != g_GreenTint) || (BlueTint != g_BlueTint))
		g_BitmapMustBeUpdated = true;

	g_RedTint = RedTint;
	g_GreenTint = GreenTint;
	g_BlueTint = BlueTint;
}

void AGSFlashlight::GetFlashlightTintRed(ScriptMethodParams &params) {
	params._result = g_RedTint;
}

void AGSFlashlight::GetFlashlightTintGreen(ScriptMethodParams &params) {
	params._result = g_GreenTint;
}

void AGSFlashlight::GetFlashlightTintBlue(ScriptMethodParams &params) {
	params._result = g_BlueTint;
}

void AGSFlashlight::GetFlashlightMinLightLevel(ScriptMethodParams &params) {
	params._result = 0;
}

void AGSFlashlight::GetFlashlightMaxLightLevel(ScriptMethodParams &params) {
	params._result = 100;
}

void AGSFlashlight::SetFlashlightDarkness(ScriptMethodParams &params) {
	PARAMS1(int, LightLevel);
	ClipToRange(LightLevel, 0, 100);

	if (LightLevel != g_DarknessLightLevel) {
		g_BitmapMustBeUpdated = true;
		g_DarknessLightLevel = LightLevel;

		if (g_DarknessLightLevel > g_BrightnessLightLevel)
			g_BrightnessLightLevel = g_DarknessLightLevel;
	}
}

void AGSFlashlight::GetFlashlightDarkness(ScriptMethodParams &params) {
	params._result = g_DarknessLightLevel;
}

void AGSFlashlight::SetFlashlightDarknessSize(ScriptMethodParams &params) {
	PARAMS1(int, Size);
	if (Size * g_ScaleFactor != g_DarknessSize) {
		g_BitmapMustBeUpdated = true;
		g_DarknessSize = Size * g_ScaleFactor;
		g_DarknessDiameter = g_DarknessSize * 2;

		if (g_BrightnessSize > g_DarknessSize) {
			ScriptMethodParams p(g_DarknessSize / g_ScaleFactor);
			SetFlashlightBrightnessSize(p);
		}
	}
}

void AGSFlashlight::GetFlashlightDarknessSize(ScriptMethodParams &params) {
	params._result = (g_DarknessSize / g_ScaleFactor);
}


void AGSFlashlight::SetFlashlightBrightness(ScriptMethodParams &params) {
	PARAMS1(int, LightLevel);
	ClipToRange(LightLevel, 0, 100);

	if (LightLevel != g_BrightnessLightLevel) {
		g_BitmapMustBeUpdated = true;
		g_BrightnessLightLevel = LightLevel;

		if (g_BrightnessLightLevel < g_DarknessLightLevel)
			g_DarknessLightLevel = g_BrightnessLightLevel;
	}
}

void AGSFlashlight::GetFlashlightBrightness(ScriptMethodParams &params) {
	params._result = g_BrightnessLightLevel;
}

void AGSFlashlight::SetFlashlightBrightnessSize(ScriptMethodParams &params) {
	PARAMS1(int, Size);
	if (Size * g_ScaleFactor != g_BrightnessSize) {
		g_BitmapMustBeUpdated = true;
		g_BrightnessSize = Size * g_ScaleFactor;

		if (g_DarknessSize < g_BrightnessSize) {
			ScriptMethodParams p(g_BrightnessSize / g_ScaleFactor);
			SetFlashlightDarknessSize(p);
		}
	}
}

void AGSFlashlight::GetFlashlightBrightnessSize(ScriptMethodParams &params) {
	params._result = g_BrightnessSize / g_ScaleFactor;
}

void AGSFlashlight::SetFlashlightPosition(ScriptMethodParams &params) {
	PARAMS2(int, X, int, Y);
	g_FlashlightX = X;
	g_FlashlightY = Y;
}

void AGSFlashlight::GetFlashlightPositionX(ScriptMethodParams &params) {
	params._result = g_FlashlightX;
}

void AGSFlashlight::GetFlashlightPositionY(ScriptMethodParams &params) {
	params._result = g_FlashlightY;
}

void AGSFlashlight::SetFlashlightFollowMouse(ScriptMethodParams &params) {
	PARAMS1(int, OnOff);
	g_FlashlightFollowMouse = (OnOff != 0);
}

void AGSFlashlight::GetFlashlightFollowMouse(ScriptMethodParams &params) {
	params._result = g_FlashlightFollowMouse ? 1 : 0;
}

void AGSFlashlight::SetFlashlightFollowCharacter(ScriptMethodParams &params) {
	PARAMS5(int, CharacterId, int, dx, int, dy, int, horz, int, vert);
	g_FollowCharacterId = CharacterId;
	g_FollowCharacterDx = dx;
	g_FollowCharacterDy = dy;
	g_FollowCharacterHorz = horz;
	g_FollowCharacterVert = vert;

	g_FollowCharacter = _engine->GetCharacter(CharacterId);
}

void AGSFlashlight::GetFlashlightFollowCharacter(ScriptMethodParams &params) {
	params._result = g_FollowCharacterId;
}

void AGSFlashlight::GetFlashlightCharacterDX(ScriptMethodParams &params) {
	params._result = g_FollowCharacterDx;
}

void AGSFlashlight::GetFlashlightCharacterDY(ScriptMethodParams &params) {
	params._result = g_FollowCharacterDy;
}

void AGSFlashlight::GetFlashlightCharacterHorz(ScriptMethodParams &params) {
	params._result = g_FollowCharacterHorz;
}

void AGSFlashlight::GetFlashlightCharacterVert(ScriptMethodParams &params) {
	params._result = g_FollowCharacterVert;
}

void AGSFlashlight::SetFlashlightMask(ScriptMethodParams &params) {
	//PARAMS1(int, SpriteSlot);
	// Not implemented.
}

void AGSFlashlight::GetFlashlightMask(ScriptMethodParams &params) {
	params._result = 0;
}

} // namespace AGSFlashlight
} // namespace Plugins
} // namespace AGS3
