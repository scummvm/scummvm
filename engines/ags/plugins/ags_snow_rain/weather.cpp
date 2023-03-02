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


void View::syncGame(Serializer &s) {
	s.syncAsInt(view);
	s.syncAsInt(loop);
	s.syncAsBool(is_default);

	// Pointer saved/loaded raw structure, which included bitmap pointer
	int dummy = 0;
	s.syncAsInt(dummy);
}

/*------------------------------------------------------------------*/

Weather::Weather(bool IsSnow, int32 &scrWidth, int32 &scrHeight, IAGSEngine *&engine) :
		_mIsSnow(IsSnow), _screenWidth(scrWidth), _screenHeight(scrHeight), _engine(engine) {
	Initialize();
}

void Weather::Update() {
	if (_mTargetAmount > _mAmount)
		_mAmount++;
	else if (_mTargetAmount < _mAmount)
		_mAmount--;

	if (!ReinitializeViews())
		return;

	int i;
	for (i = 0; i < _mAmount * 2; i++) {
		_mParticles[i].y += _mParticles[i].speed;
		_mParticles[i].x += _mWindSpeed;

		if (_mParticles[i].x < 0)
			_mParticles[i].x += _screenWidth;

		if (_mParticles[i].x > _screenWidth - 1)
			_mParticles[i].x -= _screenWidth;

		if (_mParticles[i].y > _mParticles[i].max_y) {
			_mParticles[i].y = -1 * (::AGS::g_vm->getRandomNumber(0x7fffffff) % _screenHeight);
			_mParticles[i].x = ::AGS::g_vm->getRandomNumber(0x7fffffff) % _screenWidth;
			_mParticles[i].alpha = ::AGS::g_vm->getRandomNumber(0x7fffffff) % _mDeltaAlpha + _mMinAlpha;
			_mParticles[i].speed = (float)(::AGS::g_vm->getRandomNumber(0x7fffffff) % _mDeltaFallSpeed + _mMinFallSpeed) / 50.0f;
			_mParticles[i].max_y = ::AGS::g_vm->getRandomNumber(0x7fffffff) % _mDeltaBaseline + _mTopBaseline;
		} else if ((_mParticles[i].y > 0) && (_mParticles[i].alpha > 0))
			_engine->BlitSpriteTranslucent(_mParticles[i].x, _mParticles[i].y, _mViews[_mParticles[i].kind_id].bitmap, _mParticles[i].alpha);
	}

	_engine->MarkRegionDirty(0, 0, _screenWidth, _screenHeight);
}

void Weather::UpdateWithDrift() {
	if (_mTargetAmount > _mAmount)
		_mAmount++;
	else if (_mTargetAmount < _mAmount)
		_mAmount--;

	if (!ReinitializeViews())
		return;

	int i, drift;
	for (i = 0; i < _mAmount * 2; i++) {
		_mParticles[i].y += _mParticles[i].speed;
		drift = _mParticles[i].drift * sin((float)(_mParticles[i].y +
		                                   _mParticles[i].drift_offset) * _mParticles[i].drift_speed * 2.0f * PI / 360.0f);

		if (signum(_mWindSpeed) == signum(drift))
			_mParticles[i].x += _mWindSpeed;
		else
			_mParticles[i].x += _mWindSpeed / 4;

		if (_mParticles[i].x < 0)
			_mParticles[i].x += _screenWidth;

		if (_mParticles[i].x > _screenWidth - 1)
			_mParticles[i].x -= _screenWidth;

		if (_mParticles[i].y > _mParticles[i].max_y) {
			_mParticles[i].y = -1 * (::AGS::g_vm->getRandomNumber(0x7fffffff) % _screenHeight);
			_mParticles[i].x = ::AGS::g_vm->getRandomNumber(0x7fffffff) % _screenWidth;
			_mParticles[i].alpha = ::AGS::g_vm->getRandomNumber(0x7fffffff) % _mDeltaAlpha + _mMinAlpha;
			_mParticles[i].speed = (float)(::AGS::g_vm->getRandomNumber(0x7fffffff) % _mDeltaFallSpeed + _mMinFallSpeed) / 50.0f;
			_mParticles[i].max_y = ::AGS::g_vm->getRandomNumber(0x7fffffff) % _mDeltaBaseline + _mTopBaseline;
			_mParticles[i].drift = ::AGS::g_vm->getRandomNumber(0x7fffffff) % _mDeltaDrift + _mMinDrift;
			_mParticles[i].drift_speed = (::AGS::g_vm->getRandomNumber(0x7fffffff) % _mDeltaDriftSpeed + _mMinDriftSpeed) / 50.0f;
		} else if ((_mParticles[i].y > 0) && (_mParticles[i].alpha > 0))
			_engine->BlitSpriteTranslucent(_mParticles[i].x + drift, _mParticles[i].y, _mViews[_mParticles[i].kind_id].bitmap, _mParticles[i].alpha);
	}

	_engine->MarkRegionDirty(0, 0, _screenWidth, _screenHeight);
}

void Weather::syncGame(Serializer &s) {
	int saveVersion = SaveMagic;
	s.syncAsInt(saveVersion);

	if (s.isLoading() && (uint)saveVersion != SaveMagic) {
		_engine->AbortGame("ags_snowrain: bad save.");
		return;
	}

	// TODO: At some point check whether the original did a packed
	// structure for Weather, or if bools were padded to 4 bytes
	s.syncAsBool(_mIsSnow);
	s.syncAsInt(_mMinDrift);
	s.syncAsInt(_mMaxDrift);
	s.syncAsInt(_mDeltaDrift);
	s.syncAsInt(_mMinDriftSpeed);
	s.syncAsInt(_mMaxDriftSpeed);
	s.syncAsInt(_mDeltaDriftSpeed);
	s.syncAsInt(_mAmount);
	s.syncAsInt(_mTargetAmount);
	s.syncAsInt(_mMinAlpha);
	s.syncAsInt(_mMaxAlpha);
	s.syncAsInt(_mDeltaAlpha);
	s.syncAsFloat(_mWindSpeed);
	s.syncAsInt(_mTopBaseline);
	s.syncAsInt(_mBottomBaseline);
	s.syncAsInt(_mDeltaBaseline);
	s.syncAsInt(_mMinFallSpeed);
	s.syncAsInt(_mMaxFallSpeed);
	s.syncAsInt(_mDeltaFallSpeed);

	for (int i = 0; i < 5; ++i)
		_mViews[i].syncGame(s);

	if (s.isLoading())
		InitializeParticles();
}

bool Weather::ReinitializeViews() {
	if ((_mViews[4].view == -1) || (_mViews[4].loop == -1))
		return false;

	AGSViewFrame *view_frame = _engine->GetViewFrame(_mViews[4].view, _mViews[4].loop, 0);
	BITMAP *default_bitmap = _engine->GetSpriteGraphic(view_frame->pic);

	int i;
	for (i = 0; i < 5; i++) {
		if (_mViews[i].bitmap != nullptr) {
			if (_mViews[i].is_default)
				_mViews[i].bitmap = default_bitmap;
			else {
				view_frame = _engine->GetViewFrame(_mViews[i].view, _mViews[i].loop, 0);
				_mViews[i].bitmap = _engine->GetSpriteGraphic(view_frame->pic);
			}
		}
	}

	return true;
}

bool Weather::IsActive() {
	return (_mAmount > 0) || (_mTargetAmount != _mAmount);
}

void Weather::EnterRoom() {
	_mAmount = _mTargetAmount;

	// If baseline is not fixed, reset and clamp to the new screenHeight
	if (!_mBaselineFixed)
		ResetBaseline();
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
	ResetBaseline();

	if (_mIsSnow)
		SetFallSpeed(10, 70);
	else
		SetFallSpeed(100, 300);

	_mViewsInitialized = false;

	int i;
	for (i = 0; i < 5; i++) {
		_mViews[i].is_default = true;
		_mViews[i].view = -1;
		_mViews[i].loop = -1;
		_mViews[i].bitmap = nullptr;
	}

	SetAmount(0);
}

void Weather::InitializeParticles() {
	for (uint i = 0; i < ARRAYSIZE(_mParticles); i++) {
		_mParticles[i].clear();
		_mParticles[i].kind_id = ::AGS::g_vm->getRandomNumber(0x7fffffff) % 5;
		_mParticles[i].y = ::AGS::g_vm->getRandomNumber(0x7fffffff) % (_screenHeight * 2) - _screenHeight;
		_mParticles[i].x = ::AGS::g_vm->getRandomNumber(0x7fffffff) % _screenWidth;
		_mParticles[i].alpha = ::AGS::g_vm->getRandomNumber(0x7fffffff) % _mDeltaAlpha + _mMinAlpha;
		_mParticles[i].speed = (float)(::AGS::g_vm->getRandomNumber(0x7fffffff) % _mDeltaFallSpeed + _mMinFallSpeed) / 50.0f;
		_mParticles[i].max_y = ::AGS::g_vm->getRandomNumber(0x7fffffff) % _mDeltaBaseline + _mTopBaseline;
		_mParticles[i].drift = ::AGS::g_vm->getRandomNumber(0x7fffffff) % _mDeltaDrift + _mMinDrift;
		_mParticles[i].drift_speed = (::AGS::g_vm->getRandomNumber(0x7fffffff) % _mDeltaDriftSpeed + _mMinDriftSpeed) / 50.0f;
		_mParticles[i].drift_offset = ::AGS::g_vm->getRandomNumber(0x7fffffff) % 100;
	}
}

void Weather::SetDriftRange(int min_value, int max_value) {
	ClipToRange(min_value, 0, 100);
	ClipToRange(max_value, 0, 100);

	if (min_value > max_value)
		min_value = max_value;

	_mMinDrift = min_value / 2;
	_mMaxDrift = max_value / 2;
	_mDeltaDrift = _mMaxDrift - _mMinDrift;

	if (_mDeltaDrift == 0)
		_mDeltaDrift = 1;
}

void Weather::SetDriftSpeed(int min_value, int max_value) {
	ClipToRange(min_value, 0, 200);
	ClipToRange(max_value, 0, 200);

	if (min_value > max_value)
		min_value = max_value;

	_mMinDriftSpeed = min_value;
	_mMaxDriftSpeed = max_value;
	_mDeltaDriftSpeed = _mMaxDriftSpeed - _mMinDriftSpeed;

	if (_mDeltaDriftSpeed == 0)
		_mDeltaDriftSpeed = 1;
}

void Weather::ChangeAmount(int amount) {
	ClipToRange(amount, 0, 1000);

	_mTargetAmount = amount;
}

void Weather::SetView(int kind_id, int event, int view, int loop) {
	AGSViewFrame *view_frame = _engine->GetViewFrame(view, loop, 0);
	_mViews[kind_id].bitmap = _engine->GetSpriteGraphic(view_frame->pic);
	_mViews[kind_id].is_default = false;
	_mViews[kind_id].view = view;
	_mViews[kind_id].loop = loop;

	if (!_mViewsInitialized)
		SetDefaultView(view, loop);
}

void Weather::SetDefaultView(int view, int loop) {
	AGSViewFrame *view_frame = _engine->GetViewFrame(view, loop, 0);
	BITMAP *bitmap = _engine->GetSpriteGraphic(view_frame->pic);

	_mViewsInitialized = true;

	int i;
	for (i = 0; i < 5; i++) {
		if (_mViews[i].is_default) {
			_mViews[i].view = view;
			_mViews[i].loop = loop;
			_mViews[i].bitmap = bitmap;
		}
	}
}

void Weather::SetTransparency(int min_value, int max_value) {
	ClipToRange(min_value, 0, 100);
	ClipToRange(max_value, 0, 100);

	if (min_value > max_value)
		min_value = max_value;

	_mMinAlpha = 255 - floor((float)max_value * 2.55f + 0.5f);
	_mMaxAlpha = 255 - floor((float)min_value * 2.55f + 0.5f);
	_mDeltaAlpha = _mMaxAlpha - _mMinAlpha;

	if (_mDeltaAlpha == 0)
		_mDeltaAlpha = 1;

	int i;
	for (i = 0; i < 2000; i++)
		_mParticles[i].alpha = ::AGS::g_vm->getRandomNumber(0x7fffffff) % _mDeltaAlpha + _mMinAlpha;
}

void Weather::SetWindSpeed(int value) {
	ClipToRange(value, -200, 200);

	_mWindSpeed = (float)value / 20.0f;
}

void Weather::SetBaseline(int top, int bottom) {
	if (_screenHeight > 0) {
		ClipToRange(top, 0, _screenHeight);
		ClipToRange(bottom, 0, _screenHeight);
	}

	if (top > bottom)
		top = bottom;

	_mTopBaseline = top;
	_mBottomBaseline = bottom;
	_mDeltaBaseline = _mBottomBaseline - _mTopBaseline;

	if (_mDeltaBaseline == 0)
		_mDeltaBaseline = 1;

	_mBaselineFixed = true;
}

void Weather::ResetBaseline() {
	_mTopBaseline = 0;
	_mBottomBaseline = _screenHeight;
	_mDeltaBaseline = _screenHeight;
	_mBaselineFixed = false;
}

void Weather::SetAmount(int amount) {
	ClipToRange(amount, 0, 1000);

	_mAmount = _mTargetAmount = amount;

	InitializeParticles();
}

void Weather::SetFallSpeed(int min_value, int max_value) {
	ClipToRange(min_value, 0, 1000);
	ClipToRange(max_value, 0, 1000);

	if (min_value > max_value)
		min_value = max_value;

	_mMinFallSpeed = min_value;
	_mMaxFallSpeed = max_value;
	_mDeltaFallSpeed = _mMaxFallSpeed - _mMinFallSpeed;

	if (_mDeltaFallSpeed == 0)
		_mDeltaFallSpeed = 1;
}

} // namespace AGSSnowRain
} // namespace Plugins
} // namespace AGS3
