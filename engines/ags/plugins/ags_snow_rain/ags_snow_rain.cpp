/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or(at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ags/plugins/ags_snow_rain/ags_snow_rain.h"

namespace AGS3 {
namespace Plugins {
namespace AGSSnowRain {

IAGSEngine *_engine;
int _screenWidth;
int _screenHeight;
int _screenColorDepth;

Weather *AGSSnowRain::_rain;
Weather *AGSSnowRain::_snow;

AGSSnowRain::AGSSnowRain() : DLL() {
	_engine = nullptr;
	_screenWidth = 320;
	_screenHeight  = 200;
	_screenColorDepth = 32;

	DLL_METHOD(AGS_GetPluginName);
	DLL_METHOD(AGS_EngineStartup);
	DLL_METHOD(AGS_EngineShutdown);
	DLL_METHOD(AGS_EngineOnEvent);
}

const char *AGSSnowRain::AGS_GetPluginName() {
	// Return the plugin description
	return "Snow/Rain plugin recreation";
}

void AGSSnowRain::AGS_EngineStartup(IAGSEngine *engine) {
	_engine = engine;

	if (_engine->version < 13)
		_engine->AbortGame("Engine interface is too old, need newer version of AGS.");

	SCRIPT_METHOD(srSetSnowDriftRange);
	SCRIPT_METHOD(srSetSnowDriftSpeed);
	SCRIPT_METHOD(srSetSnowFallSpeed);
	SCRIPT_METHOD(srChangeSnowAmount);
	SCRIPT_METHOD(srSetSnowBaseline);
	SCRIPT_METHOD(srSetSnowTransparency);
	SCRIPT_METHOD(srSetSnowDefaultView);
	SCRIPT_METHOD(srSetSnowWindSpeed);
	SCRIPT_METHOD(srSetSnowAmount);
	SCRIPT_METHOD(srSetSnowView);

	SCRIPT_METHOD(srSetRainDriftRange);
	SCRIPT_METHOD(srSetRainDriftSpeed);
	SCRIPT_METHOD(srSetRainFallSpeed);
	SCRIPT_METHOD(srChangeRainAmount);
	SCRIPT_METHOD(srSetRainBaseline);
	SCRIPT_METHOD(srSetRainTransparency);
	SCRIPT_METHOD(srSetRainDefaultView);
	SCRIPT_METHOD(srSetRainWindSpeed);
	SCRIPT_METHOD(srSetRainAmount);
	SCRIPT_METHOD(srSetRainView);

	SCRIPT_METHOD(srSetWindSpeed);
	SCRIPT_METHOD(srSetBaseline);

	_engine->RequestEventHook(AGSE_PREGUIDRAW);
	_engine->RequestEventHook(AGSE_PRESCREENDRAW);
	_engine->RequestEventHook(AGSE_ENTERROOM);
	_engine->RequestEventHook(AGSE_SAVEGAME);
	_engine->RequestEventHook(AGSE_RESTOREGAME);

	_rain = new Weather();
	_snow = new Weather(true);
}

void AGSSnowRain::AGS_EngineShutdown() {
	delete _rain;
	delete _snow;
}

int AGSSnowRain::AGS_EngineOnEvent(int event, int data) {
	if (event == AGSE_PREGUIDRAW) {
		if (_rain->IsActive())
			_rain->Update();

		if (_snow->IsActive())
			_snow->UpdateWithDrift();
	} else if (event == AGSE_ENTERROOM) {
		_rain->EnterRoom();
		_snow->EnterRoom();
	} else if (event == AGSE_RESTOREGAME) {
		Serializer s(_engine, data, true);
		_rain->SyncGame(s);
		_snow->SyncGame(s);
	} else if (event == AGSE_SAVEGAME) {
		Serializer s(_engine, data, false);
		_rain->SyncGame(s);
		_snow->SyncGame(s);
	} else if (event == AGSE_PRESCREENDRAW) {
		// Get screen size once here
		_engine->GetScreenDimensions(&_screenWidth, &_screenHeight , &_screenColorDepth);
		_engine->UnrequestEventHook(AGSE_PRESCREENDRAW);
	}

	return 0;
}


void AGSSnowRain::srSetWindSpeed(int value) {
	_snow->SetWindSpeed(value);
	_rain->SetWindSpeed(value);
}

void AGSSnowRain::srSetBaseline(int top, int bottom) {
	_snow->SetBaseline(top, bottom);
	_rain->SetBaseline(top, bottom);
}

void AGSSnowRain::srSetSnowDriftRange(int min_value, int max_value) {
	_snow->SetDriftRange(min_value, max_value);
}

void AGSSnowRain::srSetSnowDriftSpeed(int min_value, int max_value) {
	_snow->SetDriftSpeed(min_value, max_value);
}

void AGSSnowRain::srChangeSnowAmount(int amount) {
	_snow->ChangeAmount(amount);
}

void AGSSnowRain::srSetSnowView(int kind_id, int event, int view, int loop) {
	_snow->SetView(kind_id, event, view, loop);
}

void AGSSnowRain::srSetSnowDefaultView(int view, int loop) {
	_snow->SetDefaultView(view, loop);
}

void AGSSnowRain::srSetSnowTransparency(int min_value, int max_value) {
	_snow->SetTransparency(min_value, max_value);
}

void AGSSnowRain::srSetSnowWindSpeed(int value) {
	_snow->SetWindSpeed(value);
}

void AGSSnowRain::srSetSnowBaseline(int top, int bottom) {
	_snow->SetBaseline(top, bottom);
}

void AGSSnowRain::srSetSnowAmount(int amount) {
	_snow->SetAmount(amount);
}

void AGSSnowRain::srSetSnowFallSpeed(int min_value, int max_value) {
	_snow->SetFallSpeed(min_value, max_value);
}

void AGSSnowRain::srSetRainDriftRange(int min_value, int max_value) {
	_rain->SetDriftRange(min_value, max_value);
}

void AGSSnowRain::srSetRainDriftSpeed(int min_value, int max_value) {
	_rain->SetDriftSpeed(min_value, max_value);
}

void AGSSnowRain::srChangeRainAmount(int amount) {
	_rain->ChangeAmount(amount);
}

void AGSSnowRain::srSetRainView(int kind_id, int event, int view, int loop) {
	_rain->SetView(kind_id, event, view, loop);
}

void AGSSnowRain::srSetRainDefaultView(int view, int loop) {
	_rain->SetDefaultView(view, loop);
}

void AGSSnowRain::srSetRainTransparency(int min_value, int max_value) {
	_rain->SetTransparency(min_value, max_value);
}

void AGSSnowRain::srSetRainWindSpeed(int value) {
	_rain->SetWindSpeed(value);
}

void AGSSnowRain::srSetRainBaseline(int top, int bottom) {
	_rain->SetBaseline(top, bottom);
}

void AGSSnowRain::srSetRainAmount(int amount) {
	_rain->SetAmount(amount);
}

void AGSSnowRain::srSetRainFallSpeed(int min_value, int max_value) {
	_rain->SetFallSpeed(min_value, max_value);
}

} // namespace AGSSnowRain
} // namespace Plugins
} // namespace AGS3
