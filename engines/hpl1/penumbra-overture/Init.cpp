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
 * This file is part of Penumbra Overture.
 */

#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/engine/graphics/Material.h"
#include "hpl1/penumbra-overture/ButtonHandler.h"
#include "hpl1/penumbra-overture/GameArea.h"
#include "hpl1/penumbra-overture/GameDamageArea.h"
#include "hpl1/penumbra-overture/GameEnemy.h"
#include "hpl1/penumbra-overture/GameForceArea.h"
#include "hpl1/penumbra-overture/GameItem.h"
#include "hpl1/penumbra-overture/GameLadder.h"
#include "hpl1/penumbra-overture/GameLamp.h"
#include "hpl1/penumbra-overture/GameLink.h"
#include "hpl1/penumbra-overture/GameLiquidArea.h"
#include "hpl1/penumbra-overture/GameMessageHandler.h"
#include "hpl1/penumbra-overture/GameObject.h"
#include "hpl1/penumbra-overture/GameSaveArea.h"
#include "hpl1/penumbra-overture/GameScripts.h"
#include "hpl1/penumbra-overture/GameStickArea.h"
#include "hpl1/penumbra-overture/GameSwingDoor.h"
#include "hpl1/penumbra-overture/MapHandler.h"
#include "hpl1/penumbra-overture/Player.h"

#include "hpl1/penumbra-overture/AttackHandler.h"
#include "hpl1/penumbra-overture/Credits.h"
#include "hpl1/penumbra-overture/DeathMenu.h"
#include "hpl1/penumbra-overture/DemoEndText.h"
#include "hpl1/penumbra-overture/EffectHandler.h"
#include "hpl1/penumbra-overture/FadeHandler.h"
#include "hpl1/penumbra-overture/GameMusicHandler.h"
#include "hpl1/penumbra-overture/GraphicsHelper.h"
#include "hpl1/penumbra-overture/Inventory.h"
#include "hpl1/penumbra-overture/MapLoadText.h"
#include "hpl1/penumbra-overture/Notebook.h"
#include "hpl1/penumbra-overture/NumericalPanel.h"
#include "hpl1/penumbra-overture/PlayerHands.h"
#include "hpl1/penumbra-overture/PreMenu.h"
#include "hpl1/penumbra-overture/RadioHandler.h"
#include "hpl1/penumbra-overture/SaveHandler.h"
#include "hpl1/penumbra-overture/TriggerHandler.h"

#include "hpl1/penumbra-overture/MainMenu.h"

#include "hpl1/penumbra-overture/IntroStory.h"

#include "hpl1/penumbra-overture/Version.h" // cool version .h that uses SVN revision #s

#include "common/config-manager.h"
#include "hpl1/graphics.h"
#include "hpl1/serialize.h"

// Global init...
cInit *gpInit;

bool CheckSupport(cInit *apInit) {
	return true;
}

// END CHECK SUPPORT
///////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

#define gsUserSettingsPath _W("")
#define gsDefaultSettingsPath _W("config/default_settings.cfg")
bool gbUsingUserSettings = false;

//-----------------------------------------------------------------------

cInit::cInit() : iUpdateable("Init") {
	mbDestroyGraphics = true;
	mbWeaponAttacking = false;
	mbResetCache = true;
	mlMaxSoundDataNum = 120;
	mlMaxPSDataNum = 12;
	mbShowCrossHair = false;

	gpInit = this;
}

//-----------------------------------------------------------------------

cInit::~cInit() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cInit::CreateHardCodedPS(iParticleEmitterData *apPE) {
	cParticleSystemData3D *pPS = hplNew(cParticleSystemData3D, (apPE->GetName(), mpGame->GetResources(), mpGame->GetGraphics()));
	pPS->AddEmitterData(apPE);
	mpGame->GetResources()->GetParticleManager()->AddData3D(pPS);
}

//-----------------------------------------------------------------------

static int getIntConfig(const char *name, const int defaultVal) {
	if (ConfMan.hasKey(name))
		return ConfMan.getInt(name);
	return defaultVal;
}

static bool getBoolConfig(const char *name, const bool defaultVal) {
	if (ConfMan.hasKey(name))
		return ConfMan.getBool(name);
	return defaultVal;
}

static tString getStringConfig(const char *name, const tString &defaultVal) {
	if (ConfMan.hasKey(name))
		return ConfMan.get(name);
	return defaultVal;
}

bool cInit::Init(tString saveToLoad) {
	Hpl1::serializeInit();
	Hpl1::engineSerializeInit();
	Hpl1::penumbraOvertureSerializeInit();
	cSoundEntity::initGlobalCallbackList();
	cSerializeClass::initSaveClassesMap();

	SetWindowCaption("Penumbra Loading...");

	// MAIN INIT /////////////////////

	mpConfig = new cConfigFile(_W(""));
	mpGameConfig = new cConfigFile(_W("config/game.cfg"));
	mpGameConfig->Load();

	mvScreenSize.x = getIntConfig("screen-width", 800);
	mvScreenSize.y = getIntConfig("screen-height", 600);
	mbFullScreen = ConfMan.getBool("fullscreen");
	mbLogResources = false;
	mbDebugInteraction = false;

	mbSubtitles = ConfMan.getBool("subtitles");
	mbSimpleWeaponSwing = getBoolConfig("simple_weapon_swing", false);
	mbDisablePersonalNotes = getBoolConfig("disable_personal_notes", false);
	mbAllowQuickSave = getBoolConfig("allow_quick_save", false);
	_allowInputUnlock = getBoolConfig("allow_input_unlock", false);
	mbFlashItems = getBoolConfig("flash_items", true);
	mbShowCrossHair = getBoolConfig("show_crosshair", false);

	mbSimpleSwingInOptions = false;

	msGlobalScriptFile = getStringConfig("global_script", "global_script.hps");
	msLanguageFile = "English.lang"; // TODO: replace with proper language support
	mDifficulty = static_cast<eGameDifficulty>(getIntConfig("difficulty", eGameDifficulty_Normal));

	msStartMap = getStringConfig("starting_map", "level00_01_boat_cabin.dae");
	msStartLink = getStringConfig("starting_map_position", "link01");
	mlFSAA = getIntConfig("fsaa", 0);
	mbPostEffects = getBoolConfig("post_effects", true);
	if (Hpl1::areShadersAvailable())
		iMaterial::SetQuality(static_cast<eMaterialQuality>(getIntConfig("shader_quality", eMaterialQuality_High)));
	else
		iMaterial::SetQuality(eMaterialQuality_VeryLow);

	mPhysicsAccuracy = static_cast<ePhysicsAccuracy>(getIntConfig("physics_accuracy", ePhysicsAccuracy_High));
	mfPhysicsUpdatesPerSec = static_cast<float>(getIntConfig("physics_updates_per_second", 60));

	mlMaxSoundChannels = 32;
	mbUseSoundHardware = false;
	mlStreamUpdateFreq = 10;
	mbUseSoundThreading = true;
	mlMaxMonoChannelsHint = 0;
	mlMaxStereoChannelsHint = 0;
	msDeviceName = "NULL";

	iGpuProgram::SetLogDebugInformation(true);
	iResourceBase::SetLogCreateAndDelete(mbLogResources);

	cSetupVarContainer Vars;
	Vars.AddInt("ScreenWidth", mvScreenSize.x);
	Vars.AddInt("ScreenHeight", mvScreenSize.y);
	Vars.AddInt("ScreenBpp", 32);
	Vars.AddBool("Fullscreen", mbFullScreen);
	Vars.AddInt("Multisampling", mlFSAA);
	Vars.AddInt("LogicUpdateRate", 60);
	Vars.AddBool("UseSoundHardware", mbUseSoundHardware);
	Vars.AddBool("ForceGeneric", false);
	Vars.AddInt("MaxSoundChannels", mlMaxSoundChannels);
	Vars.AddInt("StreamUpdateFreq", mlStreamUpdateFreq);
	Vars.AddBool("UseSoundThreading", mbUseSoundThreading);
	Vars.AddBool("UseVoiceManagement", true);
	Vars.AddInt("MaxMonoChannelsHint", mlMaxMonoChannelsHint);
	Vars.AddInt("MaxStereoChannelsHint", mlMaxStereoChannelsHint);
	Vars.AddInt("StreamBufferSize", 64);
	Vars.AddInt("StreamBufferCount", 4);
	Vars.AddString("DeviceName", "NULL");
	Vars.AddString("WindowCaption", "Penumbra Loading...");

	Vars.AddBool("LowLevelSoundLogging", false);

	LowLevelGameSetup *pSetUp = NULL;

	pSetUp = hplNew(LowLevelGameSetup, ());
	mpGame = hplNew(cGame, (pSetUp, Vars));

#ifdef TIMELIMIT // TODO: supporting demo
	CheckTimeLimit();
#endif

	// Make sure hardware is really used.
	mbUseSoundHardware = mpGame->GetSound()->GetLowLevel()->IsHardwareAccelerated();

	mbShowPreMenu = getBoolConfig("show_pre_menu", true);
	mbShowIntro = getBoolConfig("show_intro", true);
	mbShowMenu = getBoolConfig("show_menu", true);

	mfMaxPhysicsTimeStep = 1.0f / mfPhysicsUpdatesPerSec;

	cMath::Randomize();

	// RESOURCE INIT /////////////////////
	mpGame->GetResources()->LoadResourceDirsFile("resources.cfg");

	// LANGUAGE ////////////////////////////////
	mpGame->GetResources()->SetLanguageFile(msLanguageFile);

	Log("Initializing " PRODUCT_NAME "\n  Version\t" PRODUCT_VERSION "\n  Date\t" PRODUCT_DATE "\n");

	// Add loaders
	mpGame->GetResources()->AddEntity3DLoader(hplNew(cEntityLoader_GameObject, ("Object", this)));
	mpGame->GetResources()->AddEntity3DLoader(hplNew(cEntityLoader_GameItem, ("Item", this)));
	mpGame->GetResources()->AddEntity3DLoader(hplNew(cEntityLoader_GameSwingDoor, ("SwingDoor", this)));
	mpGame->GetResources()->AddEntity3DLoader(hplNew(cEntityLoader_GameLamp, ("Lamp", this)));

	mpGame->GetResources()->AddEntity3DLoader(hplNew(cEntityLoader_GameEnemy, ("Enemy", this)));

	mpGame->GetResources()->AddArea3DLoader(hplNew(cAreaLoader_GameArea, ("script", this)));
	mpGame->GetResources()->AddArea3DLoader(hplNew(cAreaLoader_GameLink, ("link", this)));

#ifndef DEMO_VERSION
	mpGame->GetResources()->AddArea3DLoader(hplNew(cAreaLoader_GameSaveArea, ("save", this)));
	mpGame->GetResources()->AddArea3DLoader(hplNew(cAreaLoader_GameLadder, ("ladder", this)));
	mpGame->GetResources()->AddArea3DLoader(hplNew(cAreaLoader_GameDamageArea, ("damage", this)));
	mpGame->GetResources()->AddArea3DLoader(hplNew(cAreaLoader_GameForceArea, ("force", this)));
	mpGame->GetResources()->AddArea3DLoader(hplNew(cAreaLoader_GameLiquidArea, ("liquid", this)));
	mpGame->GetResources()->AddArea3DLoader(hplNew(cAreaLoader_GameStickArea, ("stick", this)));
#endif

	/// FIRST LOADING SCREEN ////////////////////////////////////
	mpGraphicsHelper = hplNew(cGraphicsHelper, (this));
	mpGraphicsHelper->DrawLoadingScreen("");

	// SOUND ////////////////////////////////
	mpGame->GetSound()->GetLowLevel()->SetVolume(1);

	// PHYSICS INIT /////////////////////
	mpGame->GetPhysics()->LoadSurfaceData("materials.cfg");

	// EARLY GAME INIT /////////////////////
	mpEffectHandler = hplNew(cEffectHandler, (this));

	// GRAPHICS INIT ////////////////////
	mpGame->GetGraphics()->GetRendererPostEffects()->SetActive(mbPostEffects);
	mpGame->GetGraphics()->GetRendererPostEffects()->SetBloomActive(getBoolConfig("bloom", true));
	mpGame->GetGraphics()->GetRendererPostEffects()->SetBloomSpread(6);

	mpGame->GetGraphics()->GetRendererPostEffects()->SetMotionBlurActive(getBoolConfig("motion_blur", false));
	mpGame->GetGraphics()->GetRendererPostEffects()->SetMotionBlurAmount(static_cast<float>(getIntConfig("motion_blur_amount", 300)) / 1000.f);

	mpGame->GetGraphics()->GetRenderer3D()->SetRefractionUsed(getBoolConfig("refractions", false));

	mpEffectHandler->GetDepthOfField()->SetDisabled(!getBoolConfig("depth_of_field", true));

	mpGame->GetResources()->GetMaterialManager()->SetTextureSizeLevel(getIntConfig("texture_size_level", 0));
	mpGame->GetResources()->GetMaterialManager()->SetTextureFilter(static_cast<eTextureFilter>(getIntConfig("texture_filter", eTextureFilter_Bilinear)));
	mpGame->GetResources()->GetMaterialManager()->SetTextureAnisotropy(1.0f);

	mpGame->GetGraphics()->GetLowLevel()->SetGammaCorrection(static_cast<float>(getIntConfig("gamma", 1000)) / 1000.f);

	mpGame->GetGraphics()->GetRenderer3D()->SetShowShadows(static_cast<eRendererShowShadows>(getIntConfig("shadows", eRendererShowShadows_All)));

	mpGame->SetLimitFPS(getBoolConfig("limit_fps", true));

	// BASE GAME INIT /////////////////////
	mpMusicHandler = hplNew(cGameMusicHandler, (this));
	mpPlayerHands = hplNew(cPlayerHands, (this));
	mpButtonHandler = hplNew(cButtonHandler, (this));
	mpMapHandler = hplNew(cMapHandler, (this));
	mpGameMessageHandler = hplNew(cGameMessageHandler, (this));
	mpRadioHandler = hplNew(cRadioHandler, (this));
	mpInventory = hplNew(cInventory, (this));
	mpFadeHandler = hplNew(cFadeHandler, (this));
	mpSaveHandler = hplNew(cSaveHandler, (this));
	mpTriggerHandler = hplNew(cTriggerHandler, (this));
	mpAttackHandler = hplNew(cAttackHandler, (this));
	mpNotebook = hplNew(cNotebook, (this));
	mpNumericalPanel = hplNew(cNumericalPanel, (this));
	mpDeathMenu = hplNew(cDeathMenu, (this));
	mpPlayer = hplNew(cPlayer, (this));
	mpMapLoadText = hplNew(cMapLoadText, (this));
	mpPreMenu = hplNew(cPreMenu, (this));
	mpCredits = hplNew(cCredits, (this));
	mpDemoEndText = hplNew(cDemoEndText, (this));

	mpIntroStory = hplNew(cIntroStory, (this));

	mpMainMenu = hplNew(cMainMenu, (this));

	// UPDATE STATES INIT /////////////////////
	// Add to the global state
	mpGame->GetUpdater()->AddGlobalUpdate(mpButtonHandler);
	mpGame->GetUpdater()->AddGlobalUpdate(mpSaveHandler);

	// Add to the default state
	mpGame->GetUpdater()->AddUpdate("Default", mpButtonHandler);
	mpGame->GetUpdater()->AddUpdate("Default", mpPlayer);
	mpGame->GetUpdater()->AddUpdate("Default", mpPlayerHands);
	mpGame->GetUpdater()->AddUpdate("Default", mpMusicHandler);
	mpGame->GetUpdater()->AddUpdate("Default", mpMapHandler);
	mpGame->GetUpdater()->AddUpdate("Default", mpGameMessageHandler);
	mpGame->GetUpdater()->AddUpdate("Default", mpRadioHandler);
	mpGame->GetUpdater()->AddUpdate("Default", mpInventory);
	mpGame->GetUpdater()->AddUpdate("Default", mpFadeHandler);
	mpGame->GetUpdater()->AddUpdate("Default", mpEffectHandler);
	mpGame->GetUpdater()->AddUpdate("Default", mpTriggerHandler);
	mpGame->GetUpdater()->AddUpdate("Default", mpAttackHandler);
	mpGame->GetUpdater()->AddUpdate("Default", mpNotebook);
	mpGame->GetUpdater()->AddUpdate("Default", mpNumericalPanel);
	mpGame->GetUpdater()->AddUpdate("Default", mpDeathMenu);
	mpGame->GetUpdater()->AddUpdate("Default", this); // need this last because of the init script.

	// Add to intro state
	mpGame->GetUpdater()->AddContainer("Intro");
	mpGame->GetUpdater()->AddUpdate("Intro", mpIntroStory);

	// Add to main menu state
	mpGame->GetUpdater()->AddContainer("MainMenu");
	mpGame->GetUpdater()->AddUpdate("MainMenu", mpMainMenu);

	// Add to map load text state
	mpGame->GetUpdater()->AddContainer("MapLoadText");
	mpGame->GetUpdater()->AddUpdate("MapLoadText", mpMapLoadText);

	// Add to pre menu state
	mpGame->GetUpdater()->AddContainer("PreMenu");
	mpGame->GetUpdater()->AddUpdate("PreMenu", mpPreMenu);

	// Add to credits text state
	mpGame->GetUpdater()->AddContainer("Credits");
	mpGame->GetUpdater()->AddUpdate("Credits", mpCredits);

	// Add to demo end text state
	mpGame->GetUpdater()->AddContainer("DemoEndText");
	mpGame->GetUpdater()->AddUpdate("DemoEndText", mpDemoEndText);

	mpGame->GetUpdater()->SetContainer("Default");

	// SCRIPT INIT /////////////////////
	cGameScripts::Init();

	// MISC INIT ///////////////////////
	mpDefaultFont = mpGame->GetResources()->GetFontManager()->CreateFontData("verdana.fnt");

	// GLOBAL SCRIPT ////////////////////////////
	mpGlobalScript = mpGame->GetResources()->GetScriptManager()->CreateScript(msGlobalScriptFile);
	if (mpGlobalScript) {
		mpGlobalScript->Run("OnInit()");
	}

	if (saveToLoad == "") {
		if (mbShowPreMenu) {
			mpPreMenu->SetActive(true);
		} else if (mbShowMenu) {
			mpMainMenu->SetActive(true);
		} else {
			mpMapHandler->Load(msStartMap, msStartLink);
		}
	} else {
		mpGame->GetInput()->GetLowLevel()->BeginInputUpdate(); // prevents the game from becoming unresponsive
		mpSaveHandler->LoadGameFromFile(cString::To16Char(saveToLoad), false);
		mpGame->GetInput()->GetLowLevel()->EndInputUpdate(); // clears the event queue
	}

	SetWindowCaption("Penumbra");

	return true;
}

//-----------------------------------------------------------------------

void cInit::ResetGame(bool abRunInitScript) {
	mpGame->GetUpdater()->Reset();

	if (abRunInitScript) {
		if (mpGlobalScript) {
			mpGlobalScript->Run("OnInit()");
		}
	}

	mpGame->GetSound()->GetMusicHandler()->Stop(100);
	mpGame->GetSound()->GetSoundHandler()->StopAll(eSoundDest_All);
}

//-----------------------------------------------------------------------

void cInit::Run() {
	mpGame->Run();
}

//-----------------------------------------------------------------------

void cInit::OnStart() {
}

//-----------------------------------------------------------------------

void cInit::Update(float afTimeStep) {
	if (mpGlobalScript) {
		mpGlobalScript->Run("OnUpdate()");
	}
}

//-----------------------------------------------------------------------

void cInit::Reset() {
}

//-----------------------------------------------------------------------

void cInit::Exit() {
	// PLAYER EXIT /////////////////////
	// Log(" Exit Save Handler\n");
	// hplDelete( mpSaveHandler );

	Log(" Reset maphandler\n");
	mpMapHandler->Reset();

	Log(" Exit Player\n");
	// PLAYER EXIT /////////////////////
	hplDelete(mpPlayer);

	// BASE GAME EXIT //////////////////
	Log(" Exit Button Handler\n");
	hplDelete(mpButtonHandler);
	Log(" Exit Map Handler\n");
	hplDelete(mpMapHandler);
	Log(" Exit Game Message Handler\n");
	hplDelete(mpGameMessageHandler);
	Log(" Exit Radio Handler\n");
	hplDelete(mpRadioHandler);
	Log(" Exit Inventory\n");
	hplDelete(mpInventory);
	Log(" Exit Fade Handler\n");
	hplDelete(mpFadeHandler);
	Log(" Exit Save Handler\n");
	hplDelete(mpSaveHandler);
	Log(" Exit Trigger Handler\n");
	hplDelete(mpTriggerHandler);
	Log(" Exit Attack Handler\n");
	hplDelete(mpAttackHandler);
	Log(" Exit Notebook\n");
	hplDelete(mpNotebook);
	Log(" Exit Numerical panel\n");
	hplDelete(mpNumericalPanel);
	Log(" Exit Intro story\n");
	hplDelete(mpIntroStory);
	Log(" Exit Death menu\n");
	hplDelete(mpDeathMenu);
	Log(" Exit Graphics helper\n");
	hplDelete(mpGraphicsHelper);
	Log(" Exit Main menu\n");
	hplDelete(mpMainMenu);
	Log(" Exit Player hands\n");
	hplDelete(mpPlayerHands);
	Log(" Exit Music handler\n");
	hplDelete(mpMusicHandler);
	Log(" Exit Map Load Text\n");
	hplDelete(mpMapLoadText);
	Log(" Exit PreMenu\n");
	hplDelete(mpPreMenu);
	Log(" Exit Credits\n");
	hplDelete(mpCredits);
	Log(" Exit Demo end text\n");
	hplDelete(mpDemoEndText);

	cSoundEntity::finalizeGlobalCallbackList();
	cSerializeClass::finalizeSaveClassesMap();
	cGameScripts::finalize();

	Log(" Saving config\n");
	// Save engine stuff.
	ConfMan.setBool("bloom", mpGame->GetGraphics()->GetRendererPostEffects()->GetBloomActive());
	ConfMan.setBool("motion_blur", mpGame->GetGraphics()->GetRendererPostEffects()->GetMotionBlurActive());
	ConfMan.setBool("depth_of_field", !mpEffectHandler->GetDepthOfField()->IsDisabled());
	ConfMan.setInt("motion_blur_amount", static_cast<int>(mpGame->GetGraphics()->GetRendererPostEffects()->GetMotionBlurAmount() * 1000.f));
	ConfMan.setBool("refractions", mpGame->GetGraphics()->GetRenderer3D()->GetRefractionUsed());

	ConfMan.setInt("texture_size_level", mpGame->GetResources()->GetMaterialManager()->GetTextureSizeLevel());
	ConfMan.setInt("texture_filter", mpGame->GetResources()->GetMaterialManager()->GetTextureFilter());
	// mpConfig->SetFloat("Graphics", "TextureAnisotropy", mpGame->GetResources()->GetMaterialManager()->GetTextureAnisotropy());

	ConfMan.setInt("gamma", mpGame->GetGraphics()->GetLowLevel()->GetGammaCorrection() * 1000.f);

	ConfMan.setInt("fsaa", mlFSAA);
	ConfMan.setBool("post_effects", mbPostEffects);
	if (Hpl1::areShadersAvailable())
		ConfMan.setInt("shader_quality", iMaterial::GetQuality());
	ConfMan.setBool("limit_fps", mpGame->GetLimitFPS());

	ConfMan.setInt("shadows", mpGame->GetGraphics()->GetRenderer3D()->GetShowShadows());

	Log(" Exit Effect Handler\n");
	hplDelete(mpEffectHandler);

	Log(" Exit Game\n");

	// MAIN EXIT /////////////////////
	// Delete the game,  do this after all else is deleted.
	hplDelete(mpGame);

	Log(" Saving last config\n");
	// Save the stuff to the config file
	ConfMan.setInt("screen-width", mvScreenSize.x);
	ConfMan.setInt("screen-height", mvScreenSize.y);

	ConfMan.set("global_script", msGlobalScriptFile);

	ConfMan.setBool("simple_weapon_swing", mbSimpleWeaponSwing);
	ConfMan.setBool("disable_personal_notes", mbDisablePersonalNotes);
	ConfMan.setBool("allow_quick_save", mbAllowQuickSave);
	ConfMan.setBool("allow_input_unlock", _allowInputUnlock);
	ConfMan.setBool("flash_tems", mbFlashItems);
	ConfMan.setBool("show_crosshair", mbShowCrossHair);

	ConfMan.set("starting_map", msStartMap);
	ConfMan.set("starting_map_position", msStartLink);

	ConfMan.setInt("difficulty", mDifficulty);

	ConfMan.setInt("physics_accuracy", mPhysicsAccuracy);
	ConfMan.setInt("physics_updates_per_second", static_cast<int>(mfPhysicsUpdatesPerSec));

	ConfMan.setBool("show_pre_menu", mbShowPreMenu);
	ConfMan.setBool("show_menu", mbShowMenu);
	ConfMan.setBool("show_intro", mbShowIntro);

	hplDelete(mpConfig);

	hplDelete(mpGameConfig);
	ConfMan.flushToDisk();
}

//-----------------------------------------------------------------------

void cInit::RunScriptCommand(const tString &asCommand) {
	if (asCommand[0] == '@') {
		if (mpGlobalScript && mpGlobalScript->Run(cString::Sub(asCommand, 1)) == false) {
			Warning("Couldn't run command '%s' from global script\n", asCommand.c_str());
		}
	} else {
		cWorld3D *pWorld = mpGame->GetScene()->GetWorld3D();
		if (pWorld->GetScript()->Run(asCommand) == false) {
			Warning("Couldn't run command '%s' in map script file\n", asCommand.c_str());
		}
	}
}

//-----------------------------------------------------------------------
void cInit::PreloadSoundEntityData(const tString &asFile) {
	if (asFile == "")
		return;

	mpGame->GetResources()->GetSoundEntityManager()->Preload(asFile);
}

//-----------------------------------------------------------------------

void cInit::PreloadParticleSystem(const tString &asFile) {
	if (asFile == "")
		return;
	cParticleSystem3D *pPS = mpGame->GetResources()->GetParticleManager()->CreatePS3D(
		"", asFile, 1, cMatrixf::Identity);
	hplDelete(pPS);
}

//-----------------------------------------------------------------------
