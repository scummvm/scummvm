/*

This is not the AGS SnowRain plugin by Scorpiorus (http://www.bigbluecup.com/yabb/index.php?topic=25665.0),
but a workalike plugin created by JJS for the AGS engine PSP port.

*/

#include "core/platform.h"

#if AGS_PLATFORM_OS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma warning(disable : 4244)
#endif

#if !defined(BUILTIN_PLUGINS)
#define THIS_IS_THE_PLUGIN
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef PSP_VERSION
#include <pspsdk.h>
#include <pspmath.h>
#define sin(x) vfpu_sinf(x)
#endif

#include "plugin/agsplugin.h"

#if defined(BUILTIN_PLUGINS)
namespace ags_snowrain {
#endif

//#define DEBUG
//#define AGS_SNOWRAIN_DLL_SAVEGAME_COMPATIBILITY

#define signum(x) ((x > 0) ? 1 : -1)

const unsigned int Magic = 0xCAFE0000;
const unsigned int Version = 2;
const unsigned int SaveMagic = Magic + Version;
const float PI = 3.14159265f;

int screen_width = 320;
int screen_height = 200;
int screen_color_depth = 32;

IAGSEngine *engine;


typedef struct {
	int view;
	int loop;
	bool is_default;
	BITMAP *bitmap;
} view_t;


typedef struct {
	float x;
	float y;
	int alpha;
	float speed;
	int max_y;
	int kind_id;
	int drift;
	float drift_speed;
	float drift_offset;
} drop_t;


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

	drop_t mParticles[2000];
	view_t mViews[5];

	bool mViewsInitialized;
};


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
			mParticles[i].x += screen_width;

		if (mParticles[i].x > screen_width - 1)
			mParticles[i].x -= screen_width;

		if (mParticles[i].y > mParticles[i].max_y) {
			mParticles[i].y = -1 * (rand() % screen_height);
			mParticles[i].x = rand() % screen_width;
			mParticles[i].alpha = rand() % mDeltaAlpha + mMinAlpha;
			mParticles[i].speed = (float)(rand() % mDeltaFallSpeed + mMinFallSpeed) / 50.0f;
			mParticles[i].max_y = rand() % mDeltaBaseline + mTopBaseline;
		} else if ((mParticles[i].y > 0) && (mParticles[i].alpha > 0))
			engine->BlitSpriteTranslucent(mParticles[i].x, mParticles[i].y, mViews[mParticles[i].kind_id].bitmap, mParticles[i].alpha);
	}

	engine->MarkRegionDirty(0, 0, screen_width, screen_height);
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
		drift = mParticles[i].drift * sin((float)(mParticles[i].y + mParticles[i].drift_offset) * mParticles[i].drift_speed * 2.0f * PI / 360.0f);

		if (signum(mWindSpeed) == signum(drift))
			mParticles[i].x += mWindSpeed;
		else
			mParticles[i].x += mWindSpeed / 4;

		if (mParticles[i].x < 0)
			mParticles[i].x += screen_width;

		if (mParticles[i].x > screen_width - 1)
			mParticles[i].x -= screen_width;

		if (mParticles[i].y > mParticles[i].max_y) {
			mParticles[i].y = -1 * (rand() % screen_height);
			mParticles[i].x = rand() % screen_width;
			mParticles[i].alpha = rand() % mDeltaAlpha + mMinAlpha;
			mParticles[i].speed = (float)(rand() % mDeltaFallSpeed + mMinFallSpeed) / 50.0f;
			mParticles[i].max_y = rand() % mDeltaBaseline + mTopBaseline;
			mParticles[i].drift = rand() % mDeltaDrift + mMinDrift;
			mParticles[i].drift_speed = (rand() % mDeltaDriftSpeed + mMinDriftSpeed) / 50.0f;
		} else if ((mParticles[i].y > 0) && (mParticles[i].alpha > 0))
			engine->BlitSpriteTranslucent(mParticles[i].x + drift, mParticles[i].y, mViews[mParticles[i].kind_id].bitmap, mParticles[i].alpha);
	}

	engine->MarkRegionDirty(0, 0, screen_width, screen_height);
}

static size_t engineFileRead(void *ptr, size_t size, size_t count, long fileHandle) {
	auto totalBytes = engine->FRead(ptr, size * count, fileHandle);
	return totalBytes / size;
}

static size_t engineFileWrite(const void *ptr, size_t size, size_t count, long fileHandle) {
	auto totalBytes = engine->FWrite(const_cast<void *>(ptr), size * count, fileHandle);
	return totalBytes / size;
}

void Weather::RestoreGame(long file) {
	unsigned int SaveVersion = 0;
	engineFileRead(&SaveVersion, sizeof(SaveVersion), 1, file);

	if (SaveVersion != SaveMagic) {
		engine->AbortGame("ags_snowrain: bad save.");
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
	engineFileRead(mViews, sizeof(view_t) * 5, 1, file);

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
	engineFileWrite(mViews, sizeof(view_t) * 5, 1, file);
}


bool Weather::ReinitializeViews() {
	if ((mViews[4].view == -1) || (mViews[4].loop == -1))
		return false;

	AGSViewFrame *view_frame = engine->GetViewFrame(mViews[4].view, mViews[4].loop, 0);
	BITMAP *default_bitmap = engine->GetSpriteGraphic(view_frame->pic);

	int i;
	for (i = 0; i < 5; i++) {
		if (mViews[i].bitmap != nullptr) {
			if (mViews[i].is_default)
				mViews[i].bitmap = default_bitmap;
			else {
				view_frame = engine->GetViewFrame(mViews[i].view, mViews[i].loop, 0);
				mViews[i].bitmap = engine->GetSpriteGraphic(view_frame->pic);
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
	memset(mParticles, 0, sizeof(drop_t) * 2000);
	int i;
	for (i = 0; i < 2000; i++) {
		mParticles[i].kind_id = rand() % 5;
		mParticles[i].y = rand() % (screen_height * 2) - screen_height;
		mParticles[i].x = rand() % screen_width;
		mParticles[i].alpha = rand() % mDeltaAlpha + mMinAlpha;
		mParticles[i].speed = (float)(rand() % mDeltaFallSpeed + mMinFallSpeed) / 50.0f;
		mParticles[i].max_y = rand() % mDeltaBaseline + mTopBaseline;
		mParticles[i].drift = rand() % mDeltaDrift + mMinDrift;
		mParticles[i].drift_speed = (rand() % mDeltaDriftSpeed + mMinDriftSpeed) / 50.0f;
		mParticles[i].drift_offset = rand() % 100;
	}
}


void Weather::SetDriftRange(int min_value, int max_value) {
#ifdef DEBUG
	char buffer[200];
	sprintf(buffer, "%d %s %d %d\n", (int)mIsSnow, "SetDriftRange", min_value, max_value);
	engine->PrintDebugConsole(buffer);
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
	engine->PrintDebugConsole(buffer);
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
	engine->PrintDebugConsole(buffer);
#endif

	ClipToRange(amount, 0, 1000);

	mTargetAmount = amount;
}


void Weather::SetView(int kind_id, int event, int view, int loop) {
#ifdef DEBUG
	char buffer[200];
	sprintf(buffer, "%d %s %d %d %d %d\n", (int)mIsSnow, "SetView", kind_id, event, view, loop);
	engine->PrintDebugConsole(buffer);
#endif

	AGSViewFrame *view_frame = engine->GetViewFrame(view, loop, 0);
	mViews[kind_id].bitmap = engine->GetSpriteGraphic(view_frame->pic);
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
	engine->PrintDebugConsole(buffer);
#endif

	AGSViewFrame *view_frame = engine->GetViewFrame(view, loop, 0);
	BITMAP *bitmap = engine->GetSpriteGraphic(view_frame->pic);

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
	engine->PrintDebugConsole(buffer);
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
		mParticles[i].alpha = rand() % mDeltaAlpha + mMinAlpha;
}


void Weather::SetWindSpeed(int value) {
#ifdef DEBUG
	char buffer[200];
	sprintf(buffer, "%d %s %d\n", (int)mIsSnow, "SetWindSpeed", value);
	engine->PrintDebugConsole(buffer);
#endif

	ClipToRange(value, -200, 200);

	mWindSpeed = (float)value / 20.0f;
}


void Weather::SetBaseline(int top, int bottom) {
#ifdef DEBUG
	char buffer[200];
	sprintf(buffer, "%d %s %d %d\n", (int)mIsSnow, "SetBaseline", top, bottom);
	engine->PrintDebugConsole(buffer);
#endif

	if (screen_height > 0) {
		ClipToRange(top, 0, screen_height);
		ClipToRange(bottom, 0, screen_height);
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
	engine->PrintDebugConsole(buffer);
#endif

	ClipToRange(amount, 0, 1000);

	mAmount = mTargetAmount = amount;

	InitializeParticles();
}


void Weather::SetFallSpeed(int min_value, int max_value) {
#ifdef DEBUG
	char buffer[200];
	sprintf(buffer, "%d %s %d %d\n", (int)mIsSnow, "SetFallSpeed", min_value, max_value);
	engine->PrintDebugConsole(buffer);
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



Weather *rain;
Weather *snow;




// ********************************************
// ************  AGS Interface  ***************
// ********************************************

void srSetWindSpeed(int value) {
	snow->SetWindSpeed(value);
	rain->SetWindSpeed(value);
}

void srSetBaseline(int top, int bottom) {
	snow->SetBaseline(top, bottom);
	rain->SetBaseline(top, bottom);
}

void srSetSnowDriftRange(int min_value, int max_value) {
	snow->SetDriftRange(min_value, max_value);
}

void srSetSnowDriftSpeed(int min_value, int max_value) {
	snow->SetDriftSpeed(min_value, max_value);
}

void srChangeSnowAmount(int amount) {
	snow->ChangeAmount(amount);
}

void srSetSnowView(int kind_id, int event, int view, int loop) {
	snow->SetView(kind_id, event, view, loop);
}

void srSetSnowDefaultView(int view, int loop) {
	snow->SetDefaultView(view, loop);
}

void srSetSnowTransparency(int min_value, int max_value) {
	snow->SetTransparency(min_value, max_value);
}

void srSetSnowWindSpeed(int value) {
	snow->SetWindSpeed(value);
}

void srSetSnowBaseline(int top, int bottom) {
	snow->SetBaseline(top, bottom);
}

void srSetSnowAmount(int amount) {
	snow->SetAmount(amount);
}

void srSetSnowFallSpeed(int min_value, int max_value) {
	snow->SetFallSpeed(min_value, max_value);
}

void srSetRainDriftRange(int min_value, int max_value) {
	rain->SetDriftRange(min_value, max_value);
}

void srSetRainDriftSpeed(int min_value, int max_value) {
	rain->SetDriftSpeed(min_value, max_value);
}

void srChangeRainAmount(int amount) {
	rain->ChangeAmount(amount);
}

void srSetRainView(int kind_id, int event, int view, int loop) {
	rain->SetView(kind_id, event, view, loop);
}

void srSetRainDefaultView(int view, int loop) {
	rain->SetDefaultView(view, loop);
}

void srSetRainTransparency(int min_value, int max_value) {
	rain->SetTransparency(min_value, max_value);
}

void srSetRainWindSpeed(int value) {
	rain->SetWindSpeed(value);
}

void srSetRainBaseline(int top, int bottom) {
	rain->SetBaseline(top, bottom);
}

void srSetRainAmount(int amount) {
	rain->SetAmount(amount);
}

void srSetRainFallSpeed(int min_value, int max_value) {
	rain->SetFallSpeed(min_value, max_value);
}

void AGS_EngineStartup(IAGSEngine *lpEngine) {
	engine = lpEngine;

	if (engine->version < 13)
		engine->AbortGame("Engine interface is too old, need newer version of AGS.");

	engine->RegisterScriptFunction("srSetSnowDriftRange", (void *)&srSetSnowDriftRange);
	engine->RegisterScriptFunction("srSetSnowDriftSpeed", (void *)&srSetSnowDriftSpeed);
	engine->RegisterScriptFunction("srSetSnowFallSpeed", (void *)&srSetSnowFallSpeed);
	engine->RegisterScriptFunction("srChangeSnowAmount", (void *)&srChangeSnowAmount);
	engine->RegisterScriptFunction("srSetSnowBaseline", (void *)&srSetSnowBaseline);
	engine->RegisterScriptFunction("srSetSnowTransparency", (void *)&srSetSnowTransparency);
	engine->RegisterScriptFunction("srSetSnowDefaultView", (void *)&srSetSnowDefaultView);
	engine->RegisterScriptFunction("srSetSnowWindSpeed", (void *)&srSetSnowWindSpeed);
	engine->RegisterScriptFunction("srSetSnowAmount", (void *)&srSetSnowAmount);
	engine->RegisterScriptFunction("srSetSnowView", (void *)&srSetSnowView);

	engine->RegisterScriptFunction("srSetRainDriftRange", (void *)&srSetRainDriftRange);
	engine->RegisterScriptFunction("srSetRainDriftSpeed", (void *)&srSetRainDriftSpeed);
	engine->RegisterScriptFunction("srSetRainFallSpeed", (void *)&srSetRainFallSpeed);
	engine->RegisterScriptFunction("srChangeRainAmount", (void *)&srChangeRainAmount);
	engine->RegisterScriptFunction("srSetRainBaseline", (void *)&srSetRainBaseline);
	engine->RegisterScriptFunction("srSetRainTransparency", (void *)&srSetRainTransparency);
	engine->RegisterScriptFunction("srSetRainDefaultView", (void *)&srSetRainDefaultView);
	engine->RegisterScriptFunction("srSetRainWindSpeed", (void *)&srSetRainWindSpeed);
	engine->RegisterScriptFunction("srSetRainAmount", (void *)&srSetRainAmount);
	engine->RegisterScriptFunction("srSetRainView", (void *)&srSetRainView);

	engine->RegisterScriptFunction("srSetWindSpeed", (void *)&srSetWindSpeed);
	engine->RegisterScriptFunction("srSetBaseline", (void *)&srSetBaseline);

	engine->RequestEventHook(AGSE_PREGUIDRAW);
	engine->RequestEventHook(AGSE_PRESCREENDRAW);
	engine->RequestEventHook(AGSE_ENTERROOM);
	engine->RequestEventHook(AGSE_SAVEGAME);
	engine->RequestEventHook(AGSE_RESTOREGAME);

	rain = new Weather;
	snow = new Weather(true);
}

void AGS_EngineShutdown() {
	delete rain;
	delete snow;
}

int AGS_EngineOnEvent(int event, int data) {
	if (event == AGSE_PREGUIDRAW) {
		if (rain->IsActive())
			rain->Update();

		if (snow->IsActive())
			snow->UpdateWithDrift();
	} else if (event == AGSE_ENTERROOM) {
		rain->EnterRoom();
		snow->EnterRoom();
	} else if (event == AGSE_RESTOREGAME) {
		rain->RestoreGame(data);
		snow->RestoreGame(data);
	} else if (event == AGSE_SAVEGAME) {
		rain->SaveGame(data);
		snow->SaveGame(data);
	} else if (event == AGSE_PRESCREENDRAW) {
		// Get screen size once here
		engine->GetScreenDimensions(&screen_width, &screen_height, &screen_color_depth);
		engine->UnrequestEventHook(AGSE_PRESCREENDRAW);
	}

	return 0;
}

int AGS_EngineDebugHook(const char *scriptName, int lineNum, int reserved) {
	return 0;
}

void AGS_EngineInitGfx(const char *driverID, void *data) {
}



#if AGS_PLATFORM_OS_WINDOWS && !defined(BUILTIN_PLUGINS)

// ********************************************
// ***********  Editor Interface  *************
// ********************************************

const char *scriptHeader =
    "import void srSetSnowDriftSpeed(int, int);\r\n"
    "import void srSetSnowDriftRange(int, int);\r\n"
    "import void srSetSnowFallSpeed(int, int);\r\n"
    "import void srChangeSnowAmount(int);\r\n"
    "import void srSetSnowBaseline(int, int);\r\n"
    "import void srChangeRainAmount(int);\r\n"
    "import void srSetRainView(int, int, int, int);\r\n"
    "import void srSetRainTransparency(int, int);\r\n"
    "import void srSetSnowTransparency(int, int);\r\n"
    "import void srSetSnowDefaultView(int, int);\r\n"
    "import void srSetRainDefaultView(int, int);\r\n"
    "import void srSetRainWindSpeed(int);\r\n"
    "import void srSetSnowWindSpeed(int);\r\n"
    "import void srSetWindSpeed(int);\r\n"
    "import void srSetRainBaseline(int, int);\r\n"
    "import void srSetBaseline(int, int);\r\n"
    "import void srSetSnowAmount(int);\r\n"
    "import void srSetRainAmount(int);\r\n"
    "import void srSetRainFallSpeed(int, int);\r\n"
    "import void srSetSnowView(int, int, int, int);\r\n";


IAGSEditor *editor;


LPCSTR AGS_GetPluginName(void) {
	// Return the plugin description
	return "Snow/Rain plugin recreation";
}

int  AGS_EditorStartup(IAGSEditor *lpEditor) {
	// User has checked the plugin to use it in their game

	// If it's an earlier version than what we need, abort.
	if (lpEditor->version < 1)
		return -1;

	editor = lpEditor;
	editor->RegisterScriptHeader(scriptHeader);

	// Return 0 to indicate success
	return 0;
}

void AGS_EditorShutdown() {
	// User has un-checked the plugin from their game
	editor->UnregisterScriptHeader(scriptHeader);
}

void AGS_EditorProperties(HWND parent) {
	// User has chosen to view the Properties of the plugin
	// We could load up an options dialog or something here instead
	MessageBoxA(parent, "Snow/Rain plugin recreation by JJS", "About", MB_OK | MB_ICONINFORMATION);
}

int AGS_EditorSaveGame(char *buffer, int bufsize) {
	// We don't want to save any persistent data
	return 0;
}

void AGS_EditorLoadGame(char *buffer, int bufsize) {
	// Nothing to load for this plugin
}

#endif


#if defined(BUILTIN_PLUGINS)
} // namespace ags_snowrain
#endif
