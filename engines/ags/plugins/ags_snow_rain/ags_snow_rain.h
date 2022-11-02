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

#ifndef AGS_PLUGINS_AGS_SNOW_RAIN_AGS_SNOW_RAIN_H
#define AGS_PLUGINS_AGS_SNOW_RAIN_AGS_SNOW_RAIN_H

#include "ags/plugins/ags_plugin.h"
#include "ags/plugins/ags_snow_rain/weather.h"

namespace AGS3 {
namespace Plugins {
namespace AGSSnowRain {

/**
 * This is not the original AGS SnowRain plugin, but a workalike
 * plugin created for the AGS engine PSP port.
 */
class AGSSnowRain : public PluginBase {
	SCRIPT_HASH(AGSSnowRain)
private:
	int32 _screenWidth = 320;
	int32 _screenHeight = 200;
	int32 _screenColorDepth = 32;
	Weather _rain;
	Weather _snow;

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
	virtual ~AGSSnowRain() {}

	const char *AGS_GetPluginName() override;
	void AGS_EngineStartup(IAGSEngine *lpEngine) override;
	int64 AGS_EngineOnEvent(int event, NumberPtr data) override;
};

} // namespace AGSSnowRain
} // namespace Plugins
} // namespace AGS3

#endif
