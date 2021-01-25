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

#ifndef AGS_PLUGINS_AGS_SNOW_RAIN_WEATHER_H
#define AGS_PLUGINS_AGS_SNOW_RAIN_WEATHER_H

#include "ags/plugins/dll.h"

namespace AGS3 {
namespace Plugins {
namespace AGSSnowRain {

struct View {
	int view = 0;
	int loop = 0;
	bool is_default = false;
	BITMAP *bitmap = nullptr;
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
};

class Weather {
public:
	Weather();
	Weather(bool IsSnow);
	~Weather();

	void Initialize();
	void InitializeParticles();

	void RestoreGame(long file);
	void SaveGame(long file);
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

private:
	void ClipToRange(int &variable, int min, int max);

	bool mIsSnow;

	int mMinDrift;
	int mMaxDrift;
	int mDeltaDrift;

	int mMinDriftSpeed;
	int mMaxDriftSpeed;
	int mDeltaDriftSpeed;

	int mAmount;
	int mTargetAmount;

	int mMinAlpha;
	int mMaxAlpha;
	int mDeltaAlpha;

	float mWindSpeed;

	int mTopBaseline;
	int mBottomBaseline;
	int mDeltaBaseline;

	int mMinFallSpeed;
	int mMaxFallSpeed;
	int mDeltaFallSpeed;

	Drop mParticles[2000];
	View mViews[5];

	bool mViewsInitialized;
};

} // namespace AGSSnowRain
} // namespace Plugins
} // namespace AGS3

#endif
