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

#ifndef AGS_PLUGINS_AGS_SNOW_RAIN_WEATHER_H
#define AGS_PLUGINS_AGS_SNOW_RAIN_WEATHER_H

#include "ags/plugins/plugin_base.h"
#include "ags/plugins/serializer.h"

namespace AGS3 {
namespace Plugins {
namespace AGSSnowRain {

struct View {
	int view = 0;
	int loop = 0;
	bool is_default = false;
	BITMAP *bitmap = nullptr;

	void syncGame(Serializer &s);
};

struct Drop {
	float x = 0;
	float y = 0;
	int alpha = 0;
	float speed = 0;
	int max_y = 0;
	int kind_id = 0;
	int drift = 0;
	float drift_speed = 0;
	float drift_offset = 0;

	void clear() {
		x = 0;
		y = 0;
		alpha = 0;
		speed = 0;
		max_y = 0;
		kind_id = 0;
		drift = 0;
		drift_speed = 0;
		drift_offset = 0;
	}
};

class Weather {
private:
	void ClipToRange(int &variable, int min, int max);

	bool _mIsSnow;
	int32 &_screenWidth;
	int32 &_screenHeight;
	IAGSEngine *&_engine;

	int _mMinDrift = 0;
	int _mMaxDrift = 0;
	int _mDeltaDrift = 0;

	int _mMinDriftSpeed = 0;
	int _mMaxDriftSpeed = 0;
	int _mDeltaDriftSpeed = 0;

	int _mAmount = 0;
	int _mTargetAmount = 0;

	int _mMinAlpha = 0;
	int _mMaxAlpha = 0;
	int _mDeltaAlpha = 0;

	float _mWindSpeed = 0;

	int _mTopBaseline = 0;
	int _mBottomBaseline = 0;
	int _mDeltaBaseline = 0;

	int _mMinFallSpeed = 0;
	int _mMaxFallSpeed = 0;
	int _mDeltaFallSpeed = 0;

	Drop _mParticles[2000];
	View _mViews[5];

	bool _mViewsInitialized;

public:
	Weather(bool IsSnow, int32 &scrWidth, int32 &scrHeight, IAGSEngine *&engine);

	void Initialize();
	void InitializeParticles();

	void syncGame(Serializer &s);
	bool ReinitializeViews();

	bool IsActive();
	void Update();
	void UpdateWithDrift();
	void EnterRoom();

	void SetDriftRange(int min_value, int max_value);
	void SetDriftSpeed(int min_value, int max_value);
	void ChangeAmount(int amount);
	void SetView(int kind_id, int event, int view, int loop);
	void SetDefaultView(int view, int loop);
	void SetTransparency(int min_value, int max_value);
	void SetWindSpeed(int value);
	void SetBaseline(int top, int bottom);
	void SetAmount(int amount);
	void SetFallSpeed(int min_value, int max_value);
};

} // namespace AGSSnowRain
} // namespace Plugins
} // namespace AGS3

#endif
