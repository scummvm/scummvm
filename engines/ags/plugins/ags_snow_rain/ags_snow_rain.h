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

#ifndef AGS_PLUGINS_AGS_SNOW_RAIN_AGS_SNOW_RAIN_H
#define AGS_PLUGINS_AGS_SNOW_RAIN_AGS_SNOW_RAIN_H

#include "ags/plugins/dll.h"
#include "ags/plugins/ags_snow_rain/weather.h"

namespace AGS3 {
namespace Plugins {
namespace AGSSnowRain {

extern IAGSEngine *_engine;
extern int _screenWidth;
extern int _screenHeight;
extern int _screenColorDepth;

/**
 * This is not the original AGS SnowRain plugin, but a workalike
 * plugin created for the AGS engine PSP port.
 */
class AGSSnowRain : public DLL {
private:
	static Weather *_rain;
	static Weather *_snow;
private:
	static const char *AGS_GetPluginName();
	static void AGS_EngineStartup(IAGSEngine *lpEngine);
	static void AGS_EngineShutdown();
	static int AGS_EngineOnEvent(int event, int data);

private:
	static void srSetWindSpeed(int value);
	static void srSetBaseline(int top, int bottom);
	static void srSetSnowDriftRange(int min_value, int max_value);
	static void srSetSnowDriftSpeed(int min_value, int max_value);
	static void srChangeSnowAmount(int amount);
	static void srSetSnowView(int kind_id, int event, int view, int loop);
	static void srSetSnowDefaultView(int view, int loop);
	static void srSetSnowTransparency(int min_value, int max_value);
	static void srSetSnowWindSpeed(int value);
	static void srSetSnowBaseline(int top, int bottom);
	static void srSetSnowAmount(int amount);
	static void srSetSnowFallSpeed(int min_value, int max_value);
	static void srSetRainDriftRange(int min_value, int max_value);
	static void srSetRainDriftSpeed(int min_value, int max_value);
	static void srChangeRainAmount(int amount);
	static void srSetRainView(int kind_id, int event, int view, int loop);
	static void srSetRainDefaultView(int view, int loop);
	static void srSetRainTransparency(int min_value, int max_value);
	static void srSetRainWindSpeed(int value);
	static void srSetRainBaseline(int top, int bottom);
	static void srSetRainAmount(int amount);
	static void srSetRainFallSpeed(int min_value, int max_value);
public:
	AGSSnowRain();
};

} // namespace AGSSnowRain
} // namespace Plugins
} // namespace AGS3

#endif
