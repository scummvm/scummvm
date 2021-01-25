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

#include "ags/lib/allegro.h"
#include "ags/plugins/ags_snow_rain/weather.h"
#include "ags/plugins/ags_snow_rain/ags_snow_rain.h"
#include "ags/ags.h"

namespace AGS3 {
namespace Plugins {
namespace AGSSnowRain {

#define signum(x) ((x > 0) ? 1 : -1)

const unsigned int Magic = 0xCAFE0000;
const unsigned int Version = 2;
const unsigned int SaveMagic = Magic + Version;
const float PI = 3.14159265f;

Weather::Weather() {
	mIsSnow = false;
	Initialize();
}

Weather::Weather(bool IsSnow) {
	mIsSnow = IsSnow;
	Initialize();
}

Weather::~Weather() = default;

void Weather::Update() {
	if (mTargetAmount > mAmount)
		mAmount++;
	else if (mTargetAmount < mAmount)
		mAmount--;

	if (!ReinitializeViews())
		return;

	int i;
	for (i = 0; i < mAmount * 2; i++) {
		mParticles[i].y += mParticles[i].speed;
		mParticles[i].x += mWindSpeed;

		if (mParticles[i].x < 0)
			mParticles[i].x += _screenWidth;

		if (mParticles[i].x > _screenWidth - 1)
			mParticles[i].x -= _screenWidth;

		if (mParticles[i].y > mParticles[i].max_y) {
			mParticles[i].y = -1 * (::AGS::g_vm->getRandomNumber(0x7fffffff) % _screenHeight);
			mParticles[i].x = ::AGS::g_vm->getRandomNumber(0x7fffffff) % _screenWidth;
			mParticles[i].alpha = ::AGS::g_vm->getRandomNumber(0x7fffffff) % mDeltaAlpha + mMinAlpha;
			mParticles[i].speed = (float)(::AGS::g_vm->getRandomNumber(0x7fffffff) % mDeltaFallSpeed + mMinFallSpeed) / 50.0f;
			mParticles[i].max_y = ::AGS::g_vm->getRandomNumber(0x7fffffff) % mDeltaBaseline + mTopBaseline;
		} else if ((mParticles[i].y > 0) && (mParticles[i].alpha > 0))
			_engine->BlitSpriteTranslucent(mParticles[i].x, mParticles[i].y, mViews[mParticles[i].kind_id].bitmap, mParticles[i].alpha);
	}

	_engine->MarkRegionDirty(0, 0, _screenWidth, _screenHeight);
}

void Weather::UpdateWithDrift() {
	if (mTargetAmount > mAmount)
		mAmount++;
	else if (mTargetAmount < mAmount)
		mAmount--;

	if (!ReinitializeViews())
		return;

	int i, drift;
	for (i = 0; i < mAmount * 2; i++) {
		mParticles[i].y += mParticles[i].speed;
		drift = mParticles[i].drift * sin((float)(mParticles[i].y +
			mParticles[i].drift_offset) * mParticles[i].drift_speed * 2.0f * PI / 360.0f);

		if (signum(mWindSpeed) == signum(drift))
			mParticles[i].x += mWindSpeed;
		else
			mParticles[i].x += mWindSpeed / 4;

		if (mParticles[i].x < 0)
			mParticles[i].x += _screenWidth;

		if (mParticles[i].x > _screenWidth - 1)
			mParticles[i].x -= _screenWidth;

		if (mParticles[i].y > mParticles[i].max_y) {
			mParticles[i].y = -1 * (::AGS::g_vm->getRandomNumber(0x7fffffff) % _screenHeight);
			mParticles[i].x = ::AGS::g_vm->getRandomNumber(0x7fffffff) % _screenWidth;
			mParticles[i].alpha = ::AGS::g_vm->getRandomNumber(0x7fffffff) % mDeltaAlpha + mMinAlpha;
			mParticles[i].speed = (float)(::AGS::g_vm->getRandomNumber(0x7fffffff) % mDeltaFallSpeed + mMinFallSpeed) / 50.0f;
			mParticles[i].max_y = ::AGS::g_vm->getRandomNumber(0x7fffffff) % mDeltaBaseline + mTopBaseline;
			mParticles[i].drift = ::AGS::g_vm->getRandomNumber(0x7fffffff) % mDeltaDrift + mMinDrift;
			mParticles[i].drift_speed = (::AGS::g_vm->getRandomNumber(0x7fffffff) % mDeltaDriftSpeed + mMinDriftSpeed) / 50.0f;
		} else if ((mParticles[i].y > 0) && (mParticles[i].alpha > 0))
			_engine->BlitSpriteTranslucent(mParticles[i].x + drift, mParticles[i].y, mViews[mParticles[i].kind_id].bitmap, mParticles[i].alpha);
	}

	_engine->MarkRegionDirty(0, 0, _screenWidth, _screenHeight);
}

static size_t engineFileRead(void *ptr, size_t size, size_t count, long fileHandle) {
	auto totalBytes = _engine->FRead(ptr, size * count, fileHandle);
	return totalBytes / size;
}

static size_t engineFileWrite(const void *ptr, size_t size, size_t count, long fileHandle) {
	auto totalBytes = _engine->FWrite(const_cast<void *>(ptr), size * count, fileHandle);
	return totalBytes / size;
}

void Weather::RestoreGame(long file) {
	unsigned int SaveVersion = 0;
	engineFileRead(&SaveVersion, sizeof(SaveVersion), 1, file);

	if (SaveVersion != SaveMagic) {
		_engine->AbortGame("ags_snowrain: bad save.");
	}

	// Current version
	engineFileRead(&mIsSnow, 4, 1, file);
	engineFileRead(&mMinDrift, 4, 1, file);
	engineFileRead(&mMaxDrift, 4, 1, file);
	engineFileRead(&mDeltaDrift, 4, 1, file);
	engineFileRead(&mMinDriftSpeed, 4, 1, file);
	engineFileRead(&mMaxDriftSpeed, 4, 1, file);
	engineFileRead(&mDeltaDriftSpeed, 4, 1, file);
	engineFileRead(&mAmount, 4, 1, file);
	engineFileRead(&mTargetAmount, 4, 1, file);
	engineFileRead(&mMinAlpha, 4, 1, file);
	engineFileRead(&mMaxAlpha, 4, 1, file);
	engineFileRead(&mDeltaAlpha, 4, 1, file);
	engineFileRead(&mWindSpeed, 4, 1, file);
	engineFileRead(&mTopBaseline, 4, 1, file);
	engineFileRead(&mBottomBaseline, 4, 1, file);
	engineFileRead(&mDeltaBaseline, 4, 1, file);
	engineFileRead(&mMinFallSpeed, 4, 1, file);
	engineFileRead(&mMaxFallSpeed, 4, 1, file);
	engineFileRead(&mDeltaFallSpeed, 4, 1, file);
	engineFileRead(mViews, sizeof(View) * 5, 1, file);

	InitializeParticles();
}

void Weather::SaveGame(long file) {
	engineFileWrite(&SaveMagic, sizeof(SaveMagic), 1, file);

	engineFileWrite(&mIsSnow, 4, 1, file);
	engineFileWrite(&mMinDrift, 4, 1, file);
	engineFileWrite(&mMaxDrift, 4, 1, file);
	engineFileWrite(&mDeltaDrift, 4, 1, file);
	engineFileWrite(&mMinDriftSpeed, 4, 1, file);
	engineFileWrite(&mMaxDriftSpeed, 4, 1, file);
	engineFileWrite(&mDeltaDriftSpeed, 4, 1, file);
	engineFileWrite(&mAmount, 4, 1, file);
	engineFileWrite(&mTargetAmount, 4, 1, file);
	engineFileWrite(&mMinAlpha, 4, 1, file);
	engineFileWrite(&mMaxAlpha, 4, 1, file);
	engineFileWrite(&mDeltaAlpha, 4, 1, file);
	engineFileWrite(&mWindSpeed, 4, 1, file);
	engineFileWrite(&mTopBaseline, 4, 1, file);
	engineFileWrite(&mBottomBaseline, 4, 1, file);
	engineFileWrite(&mDeltaBaseline, 4, 1, file);
	engineFileWrite(&mMinFallSpeed, 4, 1, file);
	engineFileWrite(&mMaxFallSpeed, 4, 1, file);
	engineFileWrite(&mDeltaFallSpeed, 4, 1, file);
	engineFileWrite(mViews, sizeof(View) * 5, 1, file);
}

bool Weather::ReinitializeViews() {
	if ((mViews[4].view == -1) || (mViews[4].loop == -1))
		return false;

	AGSViewFrame *view_frame = _engine->GetViewFrame(mViews[4].view, mViews[4].loop, 0);
	BITMAP *default_bitmap = _engine->GetSpriteGraphic(view_frame->pic);

	int i;
	for (i = 0; i < 5; i++) {
		if (mViews[i].bitmap != nullptr) {
			if (mViews[i].is_default)
				mViews[i].bitmap = default_bitmap;
			else {
				view_frame = _engine->GetViewFrame(mViews[i].view, mViews[i].loop, 0);
				mViews[i].bitmap = _engine->GetSpriteGraphic(view_frame->pic);
			}
		}
	}

	return true;
}

bool Weather::IsActive() {
	return (mAmount > 0) || (mTargetAmount != mAmount);
}

void Weather::EnterRoom() {
	mAmount = mTargetAmount;
}

void Weather::ClipToRange(int &variable, int min, int max) {
	if (variable < min)
		variable = min;

	if (variable > max)
		variable = max;
}

void Weather::Initialize() {
	SetDriftRange(10, 100);
	SetDriftSpeed(10, 120);

	SetTransparency(0, 0);
	SetWindSpeed(0);
	SetBaseline(0, 200);

	if (mIsSnow)
		SetFallSpeed(10, 70);
	else
		SetFallSpeed(100, 300);

	mViewsInitialized = false;

	int i;
	for (i = 0; i < 5; i++) {
		mViews[i].is_default = true;
		mViews[i].view = -1;
		mViews[i].loop = -1;
		mViews[i].bitmap = nullptr;
	}

	SetAmount(0);
}

void Weather::InitializeParticles() {
	memset(mParticles, 0, sizeof(Drop) * 2000);
	int i;
	for (i = 0; i < 2000; i++) {
		mParticles[i].kind_id = ::AGS::g_vm->getRandomNumber(0x7fffffff) % 5;
		mParticles[i].y = ::AGS::g_vm->getRandomNumber(0x7fffffff) % (_screenHeight * 2) - _screenHeight;
		mParticles[i].x = ::AGS::g_vm->getRandomNumber(0x7fffffff) % _screenWidth;
		mParticles[i].alpha = ::AGS::g_vm->getRandomNumber(0x7fffffff) % mDeltaAlpha + mMinAlpha;
		mParticles[i].speed = (float)(::AGS::g_vm->getRandomNumber(0x7fffffff) % mDeltaFallSpeed + mMinFallSpeed) / 50.0f;
		mParticles[i].max_y = ::AGS::g_vm->getRandomNumber(0x7fffffff) % mDeltaBaseline + mTopBaseline;
		mParticles[i].drift = ::AGS::g_vm->getRandomNumber(0x7fffffff) % mDeltaDrift + mMinDrift;
		mParticles[i].drift_speed = (::AGS::g_vm->getRandomNumber(0x7fffffff) % mDeltaDriftSpeed + mMinDriftSpeed) / 50.0f;
		mParticles[i].drift_offset = ::AGS::g_vm->getRandomNumber(0x7fffffff) % 100;
	}
}

void Weather::SetDriftRange(int min_value, int max_value) {
#ifdef DEBUG
	char buffer[200];
	sprintf(buffer, "%d %s %d %d\n", (int)mIsSnow, "SetDriftRange", min_value, max_value);
	_engine->PrintDebugConsole(buffer);
#endif

	ClipToRange(min_value, 0, 100);
	ClipToRange(max_value, 0, 100);

	if (min_value > max_value)
		min_value = max_value;

	mMinDrift = min_value / 2;
	mMaxDrift = max_value / 2;
	mDeltaDrift = mMaxDrift - mMinDrift;

	if (mDeltaDrift == 0)
		mDeltaDrift = 1;
}

void Weather::SetDriftSpeed(int min_value, int max_value) {
#ifdef DEBUG
	char buffer[200];
	sprintf(buffer, "%d %s %d %d\n", (int)mIsSnow, "SetDriftSpeed", min_value, max_value);
	_engine->PrintDebugConsole(buffer);
#endif

	ClipToRange(min_value, 0, 200);
	ClipToRange(max_value, 0, 200);

	if (min_value > max_value)
		min_value = max_value;

	mMinDriftSpeed = min_value;
	mMaxDriftSpeed = max_value;
	mDeltaDriftSpeed = mMaxDriftSpeed - mMinDriftSpeed;

	if (mDeltaDriftSpeed == 0)
		mDeltaDriftSpeed = 1;
}

void Weather::ChangeAmount(int amount) {
#ifdef DEBUG
	char buffer[200];
	sprintf(buffer, "%d %s %d\n", (int)mIsSnow, "ChangeAmount", amount);
	_engine->PrintDebugConsole(buffer);
#endif

	ClipToRange(amount, 0, 1000);

	mTargetAmount = amount;
}

void Weather::SetView(int kind_id, int event, int view, int loop) {
#ifdef DEBUG
	char buffer[200];
	sprintf(buffer, "%d %s %d %d %d %d\n", (int)mIsSnow, "SetView", kind_id, event, view, loop);
	_engine->PrintDebugConsole(buffer);
#endif

	AGSViewFrame *view_frame = _engine->GetViewFrame(view, loop, 0);
	mViews[kind_id].bitmap = _engine->GetSpriteGraphic(view_frame->pic);
	mViews[kind_id].is_default = false;
	mViews[kind_id].view = view;
	mViews[kind_id].loop = loop;

	if (!mViewsInitialized)
		SetDefaultView(view, loop);
}

void Weather::SetDefaultView(int view, int loop) {
#ifdef DEBUG
	char buffer[200];
	sprintf(buffer, "%d %s %d %d\n", (int)mIsSnow, "SetDefaultView", view, loop);
	_engine->PrintDebugConsole(buffer);
#endif

	AGSViewFrame *view_frame = _engine->GetViewFrame(view, loop, 0);
	BITMAP *bitmap = _engine->GetSpriteGraphic(view_frame->pic);

	mViewsInitialized = true;

	int i;
	for (i = 0; i < 5; i++) {
		if (mViews[i].is_default) {
			mViews[i].view = view;
			mViews[i].loop = loop;
			mViews[i].bitmap = bitmap;
		}
	}
}

void Weather::SetTransparency(int min_value, int max_value) {
#ifdef DEBUG
	char buffer[200];
	sprintf(buffer, "%d %s %d %d\n", (int)mIsSnow, "SetTransparency", min_value, max_value);
	_engine->PrintDebugConsole(buffer);
#endif

	ClipToRange(min_value, 0, 100);
	ClipToRange(max_value, 0, 100);

	if (min_value > max_value)
		min_value = max_value;

	mMinAlpha = 255 - floor((float)max_value * 2.55f + 0.5f);
	mMaxAlpha = 255 - floor((float)min_value * 2.55f + 0.5f);
	mDeltaAlpha = mMaxAlpha - mMinAlpha;

	if (mDeltaAlpha == 0)
		mDeltaAlpha = 1;

	int i;
	for (i = 0; i < 2000; i++)
		mParticles[i].alpha = ::AGS::g_vm->getRandomNumber(0x7fffffff) % mDeltaAlpha + mMinAlpha;
}

void Weather::SetWindSpeed(int value) {
#ifdef DEBUG
	char buffer[200];
	sprintf(buffer, "%d %s %d\n", (int)mIsSnow, "SetWindSpeed", value);
	_engine->PrintDebugConsole(buffer);
#endif

	ClipToRange(value, -200, 200);

	mWindSpeed = (float)value / 20.0f;
}

void Weather::SetBaseline(int top, int bottom) {
#ifdef DEBUG
	char buffer[200];
	sprintf(buffer, "%d %s %d %d\n", (int)mIsSnow, "SetBaseline", top, bottom);
	_engine->PrintDebugConsole(buffer);
#endif

	if (_screenHeight > 0) {
		ClipToRange(top, 0, _screenHeight);
		ClipToRange(bottom, 0, _screenHeight);
	}

	if (top > bottom)
		top = bottom;

	mTopBaseline = top;
	mBottomBaseline = bottom;
	mDeltaBaseline = mBottomBaseline - mTopBaseline;

	if (mDeltaBaseline == 0)
		mDeltaBaseline = 1;
}

void Weather::SetAmount(int amount) {
#ifdef DEBUG
	char buffer[200];
	sprintf(buffer, "%d %s %d\n", (int)mIsSnow, "SetAmount", amount);
	_engine->PrintDebugConsole(buffer);
#endif

	ClipToRange(amount, 0, 1000);

	mAmount = mTargetAmount = amount;

	InitializeParticles();
}

void Weather::SetFallSpeed(int min_value, int max_value) {
#ifdef DEBUG
	char buffer[200];
	sprintf(buffer, "%d %s %d %d\n", (int)mIsSnow, "SetFallSpeed", min_value, max_value);
	_engine->PrintDebugConsole(buffer);
#endif

	ClipToRange(min_value, 0, 1000);
	ClipToRange(max_value, 0, 1000);

	if (min_value > max_value)
		min_value = max_value;

	mMinFallSpeed = min_value;
	mMaxFallSpeed = max_value;
	mDeltaFallSpeed = mMaxFallSpeed - mMinFallSpeed;

	if (mDeltaFallSpeed == 0)
		mDeltaFallSpeed = 1;
}

} // namespace AGSSnowRain
} // namespace Plugins
} // namespace AGS3
