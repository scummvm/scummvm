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
Weather *_rain;
Weather *_snow;
private:
const char *AGS_GetPluginName();
void AGS_EngineStartup(IAGSEngine *lpEngine);
void AGS_EngineShutdown();
int64 AGS_EngineOnEvent(int event, NumberPtr data);

private:
void srSetWindSpeed(ScriptMethodParams &params);
void srSetBaseline(ScriptMethodParams &params);
void srSetSnowDriftRange(ScriptMethodParams &params);
void srSetSnowDriftSpeed(ScriptMethodParams &params);
void srChangeSnowAmount(ScriptMethodParams &params);
void srSetSnowView(ScriptMethodParams &params);
void srSetSnowDefaultView(ScriptMethodParams &params);
void srSetSnowTransparency(ScriptMethodParams &params);
void srSetSnowWindSpeed(ScriptMethodParams &params);
void srSetSnowBaseline(ScriptMethodParams &params);
void srSetSnowAmount(ScriptMethodParams &params);
void srSetSnowFallSpeed(ScriptMethodParams &params);
void srSetRainDriftRange(ScriptMethodParams &params);
void srSetRainDriftSpeed(ScriptMethodParams &params);
void srChangeRainAmount(ScriptMethodParams &params);
void srSetRainView(ScriptMethodParams &params);
void srSetRainDefaultView(ScriptMethodParams &params);
void srSetRainTransparency(ScriptMethodParams &params);
void srSetRainWindSpeed(ScriptMethodParams &params);
void srSetRainBaseline(ScriptMethodParams &params);
void srSetRainAmount(ScriptMethodParams &params);
void srSetRainFallSpeed(ScriptMethodParams &params);
public:
AGSSnowRain();
};

} // namespace AGSSnowRain
} // namespace Plugins
} // namespace AGS3

#endif
