/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
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

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#include "hpl1/engine/game/Game.h"

#include "hpl1/engine/game/ScriptFuncs.h"
#include "hpl1/engine/game/Updater.h"
#include "hpl1/engine/graphics/Graphics.h"
#include "hpl1/engine/graphics/LowLevelGraphics.h"
#include "hpl1/engine/graphics/Renderer3D.h"
#include "hpl1/engine/input/Input.h"
#include "hpl1/engine/input/Mouse.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/system/LogicTimer.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/System.h"

#include "hpl1/engine/gui/Gui.h"

#include "hpl1/engine/game/low_level_game_setup.h"
#include "hpl1/engine/system/low_level_system.h"

#include "common/events.h"
#include "hpl1/hpl1.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// FPS COUNTER
//////////////////////////////////////////////////////////////////////////

cFPSCounter::cFPSCounter(LowLevelSystem *apLowLevelSystem) {
	mfFPS = 60;

	mlFramecounter = 0;
	mfFrametimestart = 0;
	mfFrametime = 0;

	mfUpdateRate = 1;

	mpLowLevelSystem = apLowLevelSystem;

	mfFrametimestart = ((float)GetApplicationTime()) / 1000.0f;
}

void cFPSCounter::AddFrame() {
	mlFramecounter++;

	mfFrametime = (((float)GetApplicationTime()) / 1000.0f) - mfFrametimestart;

	// update the timer
	if (mfFrametime >= mfUpdateRate) {
		mfFPS = ((float)mlFramecounter) / mfFrametime;
		mlFramecounter = 0;
		mfFrametimestart = ((float)GetApplicationTime()) / 1000.0f;
	}
}

//////////////////////////////////////////////////////////////////////////
// SETUP VAR CONTAINER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cSetupVarContainer::cSetupVarContainer() {
	msBlank = "";
}

//-----------------------------------------------------------------------

void cSetupVarContainer::AddString(const tString &asName, const tString &asValue) {
	Common::StableMap<tString, tString>::value_type val(asName, asValue);
	m_mapVars.insert(val);
}

void cSetupVarContainer::AddInt(const tString &asName, int alValue) {
	AddString(asName, cString::ToString(alValue));
}
void cSetupVarContainer::AddFloat(const tString &asName, float afValue) {
	AddString(asName, cString::ToString(afValue));
}
void cSetupVarContainer::AddBool(const tString &asName, bool abValue) {
	AddString(asName, abValue ? "true" : "false");
}

//-----------------------------------------------------------------------

const tString &cSetupVarContainer::GetString(const tString &asName) {
	Common::StableMap<tString, tString>::iterator it = m_mapVars.find(asName);
	if (it == m_mapVars.end())
		return msBlank;
	else
		return it->second;
}

float cSetupVarContainer::GetFloat(const tString &asName, float afDefault) {
	const tString &sVal = GetString(asName);
	if (sVal == "")
		return afDefault;
	else
		return cString::ToFloat(sVal.c_str(), afDefault);
}
int cSetupVarContainer::GetInt(const tString &asName, int alDefault) {
	const tString &sVal = GetString(asName);
	if (sVal == "")
		return alDefault;
	else
		return cString::ToInt(sVal.c_str(), alDefault);
}
bool cSetupVarContainer::GetBool(const tString &asName, bool abDefault) {
	const tString &sVal = GetString(asName);
	if (sVal == "")
		return abDefault;
	else
		return cString::ToBool(sVal.c_str(), abDefault);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGame::cGame(LowLevelGameSetup *apGameSetup, cSetupVarContainer &aVars) {
	GameInit(apGameSetup, aVars);
}

//-----------------------------------------------------------------------

cGame::cGame(LowLevelGameSetup *apGameSetup, int alWidth, int alHeight, int alBpp, bool abFullscreen,
			 unsigned int alUpdateRate, int alMultisampling) {
	cSetupVarContainer Vars;
	Vars.AddInt("ScreenWidth", alWidth);
	Vars.AddInt("ScreenHeight", alHeight);
	Vars.AddInt("ScreenBpp", alBpp);
	Vars.AddBool("Fullscreen", abFullscreen);
	Vars.AddInt("Multisampling", alMultisampling);
	Vars.AddInt("LogicUpdateRate", alUpdateRate);
	GameInit(apGameSetup, Vars);
}

//-----------------------------------------------------------------------

void cGame::GameInit(LowLevelGameSetup *apGameSetup, cSetupVarContainer &aVars) {
	mpGameSetup = apGameSetup;

	Log("Creating Engine Modules\n");
	Log("--------------------------------------------------------\n");

	// Create the modules that game connects to and init them!
	Log(" Creating graphics module\n");
	mpGraphics = mpGameSetup->createGraphics();

	Log(" Creating system module\n");
	mpSystem = mpGameSetup->createSystem();

	Log(" Creating resource module\n");
	mpResources = mpGameSetup->createResources(mpGraphics);

	Log(" Creating input module\n");
	mpInput = mpGameSetup->createInput(mpGraphics);

	Log(" Creating sound module\n");
	mpSound = mpGameSetup->createSound();

	Log(" Creating physics module\n");
	mpPhysics = mpGameSetup->createPhysics();

	Log(" Creating ai module\n");
	mpAI = mpGameSetup->createAi();

	Log(" Creating gui module\n");
	mpGui = hplNew(cGui, ());

	Log(" Creating scene module\n");
	mpScene = mpGameSetup->createScene(mpGraphics, mpResources, mpSound, mpPhysics, mpSystem, mpAI);

	Log("--------------------------------------------------------\n\n");

	// Init the resources
	mpResources->Init(mpGraphics, mpSystem, mpSound, mpScene, mpGui);

	// Init the graphics
	mpGraphics->Init(aVars.GetInt("ScreenWidth", 800),
					 aVars.GetInt("ScreenHeight", 600),
					 aVars.GetInt("ScreenBpp", 32),
					 aVars.GetBool("Fullscreen", false),
					 aVars.GetInt("Multisampling", 0),
					 aVars.GetString("WindowCaption"),
					 mpResources);

	// Init Sound
	mpSound->Init(mpResources, aVars.GetBool("UseSoundHardware", true),
				  aVars.GetBool("ForceGeneric", false),
				  aVars.GetBool("UseEnvironmentalAudio", false),
				  aVars.GetInt("MaxSoundChannels", 32),
				  aVars.GetInt("StreamUpdateFreq", 10),
				  aVars.GetBool("UseSoundThreading", true),
				  aVars.GetBool("UseVoiceManagement", true),
				  aVars.GetInt("MaxMonoChannelsHint", 0),
				  aVars.GetInt("MaxStereoChannelsHint", 0),
				  aVars.GetInt("StreamBufferSize", 4096),
				  aVars.GetInt("StreamBufferCount", 8),
				  aVars.GetBool("LowLevelSoundLogging", false),
				  aVars.GetString("DeviceName"));

	// Init physics
	mpPhysics->Init(mpResources);

	// Init AI
	mpAI->Init();

	// Init Gui
	mpGui->Init(mpResources, mpGraphics, mpSound, mpScene);

	Log("Initializing Game Module\n");
	Log("--------------------------------------------------------\n");
	// Create the updatehandler
	Log(" Adding engine updates\n");
	mpUpdater = hplNew(cUpdater, (mpSystem->GetLowLevel()));

	// Add some loaded modules to the updater
	mpUpdater->AddGlobalUpdate(mpInput);
	mpUpdater->AddGlobalUpdate(mpPhysics);
	mpUpdater->AddGlobalUpdate(mpScene);
	mpUpdater->AddGlobalUpdate(mpSound);
	mpUpdater->AddGlobalUpdate(mpAI);
	mpUpdater->AddGlobalUpdate(mpGui);
	mpUpdater->AddGlobalUpdate(mpResources);

	// Setup the "default" updater container
	mpUpdater->AddContainer("Default");
	mpUpdater->SetContainer("Default");

	// Create the logic timer.
	mpLogicTimer = mpSystem->CreateLogicTimer(aVars.GetInt("LogicUpdateRate", 800));

	// Init some standard script funcs
	Log(" Initializing script functions\n");
	cScriptFuncs::Init(mpGraphics, mpResources, mpSystem, mpInput, mpScene, mpSound, this);

	// Since game is not done:
	mbGameIsDone = false;

	mfUpdateTime = 0;
	mfGameTime = 0;

	mbLimitFPS = true;

	mpFPSCounter = hplNew(cFPSCounter, (mpSystem->GetLowLevel()));
	Log("--------------------------------------------------------\n\n");

	Log("User Initialization\n");
	Log("--------------------------------------------------------\n");
}

//-----------------------------------------------------------------------

cGame::~cGame() {
	Log("--------------------------------------------------------\n\n");

	hplDelete(mpLogicTimer);
	hplDelete(mpFPSCounter);

	hplDelete(mpUpdater);

	hplDelete(mpGui);
	hplDelete(mpScene);
	hplDelete(mpInput);
	hplDelete(mpSound);
	hplDelete(mpGraphics);
	hplDelete(mpResources);
	hplDelete(mpPhysics);
	hplDelete(mpAI);
	hplDelete(mpSystem);

	Log(" Deleting game setup provided by user\n");
	hplDelete(mpGameSetup);

	Log("HPL Exit was successful!\n");
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHOD
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

int glClearUpdateCheck = 0;
void cGame::Run() {
	double fNumOfTimes = 0;
	double fMediumTime = 0;

	mpUpdater->OnStart();

	mpLogicTimer->Reset();

	// Loop the game... fix the var...
	unsigned long lTempTime = GetApplicationTime();

	mfFrameTime = 0;
	unsigned long lTempFrameTime = GetApplicationTime();

	bool mbIsUpdated = true;

	while (!mbGameIsDone && !g_engine->shouldQuit()) {
		//////////////////////////
		// Update logic.
		while (mpLogicTimer->WantUpdate() && !mbGameIsDone) {
			unsigned int lUpdateTime = GetApplicationTime();

			mpUpdater->Update(GetStepSize());

			unsigned int lDeltaTime = GetApplicationTime() - lUpdateTime;
			mfUpdateTime = (float)(lDeltaTime) / 1000.0f;

			mbIsUpdated = true;

			glClearUpdateCheck++;

			mfGameTime += GetStepSize();
		}
		mpLogicTimer->EndUpdateLoop();

		// If not making a single rendering is better to use gpu and
		// cpu at the same time and make query checks etc after logic update.
		// If any delete has occured in the update this might crash. so skip it for now.
		/*if(mbRenderOnce==false)	{
			mpGraphics->GetRenderer3D()->FetchOcclusionQueries();
			mpUpdater->OnPostBufferSwap();
		}*/

		// Draw graphics!
		if (mbIsUpdated)
			mpScene->UpdateRenderList(mfFrameTime);
		if (mbLimitFPS == false || mbIsUpdated) {
			mbIsUpdated = false;

			// Get the the from the last frame.
			mfFrameTime = ((float)(GetApplicationTime() - lTempFrameTime)) / 1000;
			lTempFrameTime = GetApplicationTime();

			// Draw this frame
			// unsigned long lFTime = GetApplicationTime();
			mpUpdater->OnDraw();
			mpScene->Render(mpUpdater, mfFrameTime);

			// Update fps counter.
			mpFPSCounter->AddFrame();

			// Update the screen.
			mpGraphics->GetLowLevel()->SwapBuffers();
			mpGraphics->GetRenderer3D()->FetchOcclusionQueries();
			mpUpdater->OnPostBufferSwap();

			fNumOfTimes++;
		}
	}
	Log("--------------------------------------------------------\n\n");

	Log("Statistics\n");
	Log("--------------------------------------------------------\n");

	unsigned long lTime = GetApplicationTime() - lTempTime;
	fMediumTime = fNumOfTimes / (((double)lTime) / 1000);

	Log(" Medium framerate: %f\n", fMediumTime);
	Log("--------------------------------------------------------\n\n");

	Log("User Exit\n");
	Log("--------------------------------------------------------\n");

	mpUpdater->OnExit();
}
//-----------------------------------------------------------------------

void cGame::Exit() {
	mbGameIsDone = true;
}
//-----------------------------------------------------------------------

void cGame::ResetLogicTimer() {
	mpLogicTimer->Reset();
}

void cGame::SetUpdatesPerSec(int alUpdatesPerSec) {
	mpLogicTimer->SetUpdatesPerSec(alUpdatesPerSec);
}

int cGame::GetUpdatesPerSec() {
	return mpLogicTimer->GetUpdatesPerSec();
}

float cGame::GetStepSize() {
	return mpLogicTimer->GetStepSize();
}

//-----------------------------------------------------------------------

cScene *cGame::GetScene() {
	return mpScene;
}

//-----------------------------------------------------------------------

cResources *cGame::GetResources() {
	return mpResources;
}

//-----------------------------------------------------------------------

cGraphics *cGame::GetGraphics() {
	return mpGraphics;
}

//-----------------------------------------------------------------------

cSystem *cGame::GetSystem() {
	return mpSystem;
}

//-----------------------------------------------------------------------

cInput *cGame::GetInput() {
	return mpInput;
}

//-----------------------------------------------------------------------

cSound *cGame::GetSound() {
	return mpSound;
}

//-----------------------------------------------------------------------

cPhysics *cGame::GetPhysics() {
	return mpPhysics;
}

//-----------------------------------------------------------------------

cAI *cGame::GetAI() {
	return mpAI;
}

//-----------------------------------------------------------------------

cGui *cGame::GetGui() {
	return mpGui;
}

//-----------------------------------------------------------------------

cUpdater *cGame::GetUpdater() {
	return mpUpdater;
}

float cGame::GetFPS() {
	return mpFPSCounter->mfFPS;
}

//-----------------------------------------------------------------------

void cGame::SetFPSUpdateRate(float afSec) {
	mpFPSCounter->mfUpdateRate = afSec;
}
float cGame::GetFPSUpdateRate() {
	return mpFPSCounter->mfUpdateRate;
}

//-----------------------------------------------------------------------
} // namespace hpl
