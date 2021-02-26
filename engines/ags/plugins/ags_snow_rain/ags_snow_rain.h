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

#include "ags/plugins/plugin_base.h"
#include "ags/plugins/ags_snow_rain/weather.h"

namespace AGS3 {
namespace Plugins {
namespace AGSSnowRain {

extern IAGSEngine *_engine;
extern int32 _screenWidth;
extern int32 _screenHeight;
extern int32 _screenColorDepth;

/**
 * This is not the original AGS SnowRain plugin, but a workalike
 * plugin created for the AGS engine PSP port.
 */
class AGSSnowRain : public PluginBase {
private:
	static Weather *_rain;
	static Weather *_snow;
private:
	static const char *AGS_GetPluginName();
	static void AGS_EngineStartup(IAGSEngine *lpEngine);
	static void AGS_EngineShutdown();
	static NumberPtr AGS_EngineOnEvent(int event, NumberPtr data);

private:
	static void srSetWindSpeed(const ScriptMethodParams &params);
	static void srSetBaseline(const ScriptMethodParams &params);
	static void srSetSnowDriftRange(const ScriptMethodParams &params);
	static void srSetSnowDriftSpeed(const ScriptMethodParams &params);
	static void srChangeSnowAmount(const ScriptMethodParams &params);
	static void srSetSnowView(const ScriptMethodParams &params);
	static void srSetSnowDefaultView(const ScriptMethodParams &params);
	static void srSetSnowTransparency(const ScriptMethodParams &params);
	static void srSetSnowWindSpeed(const ScriptMethodParams &params);
	static void srSetSnowBaseline(const ScriptMethodParams &params);
	static void srSetSnowAmount(const ScriptMethodParams &params);
	static void srSetSnowFallSpeed(const ScriptMethodParams &params);
	static void srSetRainDriftRange(const ScriptMethodParams &params);
	static void srSetRainDriftSpeed(const ScriptMethodParams &params);
	static void srChangeRainAmount(const ScriptMethodParams &params);
	static void srSetRainView(const ScriptMethodParams &params);
	static void srSetRainDefaultView(const ScriptMethodParams &params);
	static void srSetRainTransparency(const ScriptMethodParams &params);
	static void srSetRainWindSpeed(const ScriptMethodParams &params);
	static void srSetRainBaseline(const ScriptMethodParams &params);
	static void srSetRainAmount(const ScriptMethodParams &params);
	static void srSetRainFallSpeed(const ScriptMethodParams &params);
public:
	AGSSnowRain();
};

} // namespace AGSSnowRain
} // namespace Plugins
} // namespace AGS3

#endif
