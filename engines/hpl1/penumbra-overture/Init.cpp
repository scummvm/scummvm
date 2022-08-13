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
#include "hpl1/penumbra-overture/HapticGameCamera.h"
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

// Global init...
cInit *gpInit;

///////////////////////////////////////////////
// BEGIN TIME LIMIT

//#define TIMELIMIT

#ifdef TIMELIMIT
float gfNoHealthLeftMin = 19;
float gfNoHealthLeftMax = 26;
void CheckTimeLimit() {
	iLowLevelSystem *pLowlevelSystem = gpInit->mpGame->GetSystem()->GetLowLevel();
	cDate date = pLowlevelSystem->GetDate();

	if (date.month_day < gfNoHealthLeftMin ||
		date.month_day > gfNoHealthLeftMax ||
		date.year != 2007 || date.month != 0 ||
		pLowlevelSystem->FileExists("c:\\WINDOWS\\ocregx.dat")) {
		if (pLowlevelSystem->FileExists("c:\\WINDOWS\\ocregx.dat") == false) {
			FILE *pFile = fopen("c:\\WINDOWS\\ocregx.dat", "wb");

			for (int i = 0; i < 937; ++i) {
				unsigned char c = (char)cMath::RandRectl(0, 255);
				fwrite(&c, 1, 1, pFile);
			}
		}

		exit(0);
	}
}
#endif

// END TIME LIMIT
///////////////////////////////////////////////

///////////////////////////////////////////////
// BEGIN CHECK SUPPORT

bool CheckSupport(cInit *apInit) {
	return true;
#if 0
	iLowLevelGraphics *pLowLevelGraphics = apInit->mpGame->GetGraphics()->GetLowLevel();
	cInit *mpInit = apInit;
	// Vertex shader support.
	if (pLowLevelGraphics->GetCaps(eGraphicCaps_GL_VertexProgram) == 0) {
		apInit->msErrorMessage = kTranslate("StartUp", "Error_NoVertexShader") + _W("\n") +
								 kTranslate("StartUp", "ErrorAdd01") + _W("\n") +
								 kTranslate("StartUp", "ErrorAdd02");
		return false;
	}
	// Texture units
	if (pLowLevelGraphics->GetCaps(eGraphicCaps_MaxTextureImageUnits) <= 1) {
		apInit->msErrorMessage = kTranslate("StartUp", "Error_FewTextureUnits") + _W("\n") +
								 kTranslate("StartUp", "ErrorAdd01") + _W("\n") +
								 kTranslate("StartUp", "ErrorAdd02");
		return false;
	}

	Log("Checking Supported Profiles\n");
#define CG_CHECK(p)                \
	if (cgGLIsProfileSupported(p)) \
	Log("  Profile " #p " is supported\n")
	CG_CHECK(CG_PROFILE_VP20);
	CG_CHECK(CG_PROFILE_FP20);

	CG_CHECK(CG_PROFILE_VP30);
	CG_CHECK(CG_PROFILE_FP30);

	CG_CHECK(CG_PROFILE_VP40);
	CG_CHECK(CG_PROFILE_FP40);

	CG_CHECK(CG_PROFILE_ARBVP1);
	CG_CHECK(CG_PROFILE_ARBFP1);

	CG_CHECK(CG_PROFILE_GLSLV);
	CG_CHECK(CG_PROFILE_GLSLF);
	CG_CHECK(CG_PROFILE_GLSLC);
#undef CG_CHECK
#endif
	// Try compiling vertex shader
#if 0
	Log("Trying to load vertex program!\n");
	iGpuProgram *pTestVtxProg = pLowLevelGraphics->CreateGpuProgram("Test", eGpuProgramType_Vertex);
	if (pTestVtxProg->CreateFromFile("core/programs/Fallback01_Diffuse_Light_p1_vp.cg", "main") == false) {
		Log("Did not succeed!\n");
		if (iMaterial::GetQuality() != eMaterialQuality_VeryLow) {
			apInit->msErrorMessage = kTranslate("StartUp", "Error_BadVertexShader") + _W("\n") +
									 kTranslate("StartUp", "Error_BadVertexShader02") + _W("\n") +
									 kTranslate("StartUp", "Error_BadVertexShader03") + _W("\n") +
									 kTranslate("StartUp", "Error_BadVertexShader04") + _W("\n") +
									 kTranslate("StartUp", "ErrorAdd02");
			hplDelete(pTestVtxProg);

			mpInit->mpConfig->SetInt("Graphics", "ShaderQuality", eMaterialQuality_VeryLow);
			mpInit->mpConfig->Save();

			return false;
		}
	}
	Log("Success!\n");
	hplDelete(pTestVtxProg);
#endif
	return true;
}

// END CHECK SUPPORT
///////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

tWString gsUserSettingsPath = _W("");
tWString gsDefaultSettingsPath = _W("config/default_settings.cfg");
bool gbUsingUserSettings = false;

//-----------------------------------------------------------------------

cInit::cInit(void) : iUpdateable("Init") {
	mbDestroyGraphics = true;
	mbWeaponAttacking = false;
	mbResetCache = true;
	mlMaxSoundDataNum = 120;
	mlMaxPSDataNum = 12;
	mbShowCrossHair = false;
	mbHasHaptics = false;
	mbHasHapticsOnRestart = false;

	gpInit = this;
}

//-----------------------------------------------------------------------

cInit::~cInit(void) {
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

bool cInit::Init(tString asCommandLine) {
	/*if(asCommandLine != "")
	{
		Log("CommandLine: %s\n",asCommandLine.c_str());
		if(cSerialChecker::Validate(asCommandLine)<=0)
		{
			FatalError("Invalid serial number!\n");
		}
	}*/

	// iResourceBase::SetLogCreateAndDelete(true);
	SetWindowCaption("Penumbra Loading...");

	// PERSONAL DIR /////////////////////
	tWString sPersonalDir = GetSystemSpecialPath(eSystemPath_Personal);
	if (cString::GetLastCharW(sPersonalDir) != _W("/") &&
		cString::GetLastCharW(sPersonalDir) != _W("\\")) {
		sPersonalDir += _W("/");
	}

	// CREATE NEEDED DIRS /////////////////////
	gsUserSettingsPath = sPersonalDir + PERSONAL_RELATIVEROOT PERSONAL_RELATIVEGAME _W("settings.cfg");
#ifndef WIN32
	// For Mac OS X and Linux move the OLD Episode 1 folder to Penumbra/Overture and symlink to the old path
	if (FolderExists(sPersonalDir + PERSONAL_RELATIVEROOT _W("Penumbra Overture/Episode1")) && !IsFileLink(sPersonalDir + PERSONAL_RELATIVEROOT _W("Penumbra Overture/Episode1"))) {
		// Create the new folder
		if (!FolderExists(sPersonalDir + PERSONAL_RELATIVEROOT _W("Penumbra")))
			CreateFolder(sPersonalDir + PERSONAL_RELATIVEROOT _W("Penumbra"));
		// Move the Older Episode 1 to the new Overture
		RenameFile(sPersonalDir + PERSONAL_RELATIVEROOT _W("Penumbra Overture/Episode1"),
				   sPersonalDir + PERSONAL_RELATIVEROOT _W("Penumbra/Overture"));
		// Link back the old one to the new one
		LinkFile(sPersonalDir + PERSONAL_RELATIVEROOT _W("Penumbra/Overture"),
				 sPersonalDir + PERSONAL_RELATIVEROOT _W("Penumbra Overture/Episode1"));
	}
#endif

	tWString vDirs[] = {PERSONAL_RELATIVEPIECES // auto includes ,
							PERSONAL_RELATIVEROOT PERSONAL_RELATIVEGAME_PARENT,
						PERSONAL_RELATIVEROOT PERSONAL_RELATIVEGAME};
	int lDirNum = PERSONAL_RELATIVEPIECES_COUNT + 2;

	// Check if directories exist and if not create
	for (int i = 0; i < lDirNum; ++i) {
		tWString sDir = sPersonalDir + vDirs[i];
		if (FolderExists(sDir))
			continue;

		CreateFolder(sDir);
	}

	// MAIN INIT /////////////////////

	// Check for what settings file to use.
	if (FileExists(gsUserSettingsPath)) {
		mpConfig = hplNew(cConfigFile, (gsUserSettingsPath));
		gbUsingUserSettings = true;
	} else {
		mpConfig = hplNew(cConfigFile, (gsDefaultSettingsPath));
		gbUsingUserSettings = false;
	}

	// Load config file
	mpConfig->Load();

#ifdef LAST_INIT_OK
	// If last init was not okay, reset all settings.
	if (mpConfig->GetBool("Game", "LastInitOK", true) == false) {
		hplDelete(mpConfig);
		mpConfig = hplNew(cConfigFile, (gsDefaultSettingsPath));
		gbUsingUserSettings = false;
		CreateMessageBoxW(
			_W("Info"),
			_W("Game did not end properly last run, resetting configuration"));
	}
#endif

	// Init is not done, so we do not know if it is okay.
	if (gbUsingUserSettings) {
		mpConfig->SetBool("Game", "LastInitOK", false);
		mpConfig->Save();
	}

	mpGameConfig = hplNew(cConfigFile, (_W("config/game.cfg")));
	mpGameConfig->Load();

	mvScreenSize.x = mpConfig->GetInt("Screen", "Width", 800);
	mvScreenSize.y = mpConfig->GetInt("Screen", "Height", 600);
	mbFullScreen = mpConfig->GetBool("Screen", "FullScreen", true);
	mbVsync = mpConfig->GetBool("Screen", "Vsync", false);
	mbLogResources = mpConfig->GetBool("Debug", "LogResources", false);
	mbDebugInteraction = mpConfig->GetBool("Debug", "DebugInteraction", false);

	msWebPageOnExit = mpConfig->GetString("Demo", "WebPageOnExit", "http://www.Penumbra-Overture.com");

	mbSubtitles = mpConfig->GetBool("Game", "Subtitles", true);
	mbSimpleWeaponSwing = mpConfig->GetBool("Game", "SimpleWeaponSwing", false);
	mbDisablePersonalNotes = mpConfig->GetBool("Game", "DisablePersonalNotes", false);
	mbAllowQuickSave = mpConfig->GetBool("Game", "AllowQuickSave", false);
	mbFlashItems = mpConfig->GetBool("Game", "FlashItems", true);
	mbShowCrossHair = mpConfig->GetBool("Game", "ShowCrossHair", false);

	mbHapticsAvailable = mpConfig->GetBool("Haptics", "Available", false);
	if (mbHapticsAvailable) {
		mbHasHaptics = mpConfig->GetBool("Haptics", "Active", false);
		cHaptic::SetIsUsed(mbHasHaptics);
		mbHasHapticsOnRestart = mbHasHaptics;
	} else {
		mbHasHaptics = false;
	}
	mfHapticForceMul = mpConfig->GetFloat("Haptics", "ForceMul", 1.0f);
	mfHapticMoveScreenSpeedMul = mpConfig->GetFloat("Haptics", "MoveScreenSpeedMul", 1.0f);
	mfHapticScale = mpConfig->GetFloat("Haptics", "Scale", 0.04f);
	mfHapticProxyRadius = mpConfig->GetFloat("Haptics", "ProxyRadius", 0.019f);
	mfHapticOffsetZ = mpConfig->GetFloat("Haptics", "OffsetZ", 1.9f);
	mfHapticMaxInteractDist = mpConfig->GetFloat("Haptics", "HapticMaxInteractDist", 2);

	mbSimpleSwingInOptions = mpConfig->GetBool("Game", "SimpleSwingInOptions", mbHapticsAvailable ? true : false);

	msGlobalScriptFile = mpConfig->GetString("Map", "GlobalScript", "global_script.hps");
	msLanguageFile = mpConfig->GetString("Game", "LanguageFile", "english.lang");
	msCurrentUser = mpConfig->GetString("Game", "CurrentUser", "default");
	mDifficulty = (eGameDifficulty)mpConfig->GetInt("Game", "Difficulty", 1);

	msStartMap = mpConfig->GetString("Map", "File", "level00_01_boat_cabin.dae");
	msStartLink = mpConfig->GetString("Map", "StartPos", "link01");

	mlFSAA = mpConfig->GetInt("Graphics", "FSAA", 0);
	mbPostEffects = mpConfig->GetBool("Graphics", "PostEffects", true);
	iMaterial::SetQuality((eMaterialQuality)mpConfig->GetInt("Graphics", "ShaderQuality", eMaterialQuality_VeryLow));

	mPhysicsAccuracy = (ePhysicsAccuracy)mpConfig->GetInt("Physics", "Accuracy", ePhysicsAccuracy_High);
	mfPhysicsUpdatesPerSec = mpConfig->GetFloat("Physics", "UpdatesPerSec", 60.0f);

	mlMaxSoundChannels = mpConfig->GetInt("Sound", "MaxSoundChannels", 32);
	mbUseSoundHardware = mpConfig->GetBool("Sound", "UseSoundHardware", false);
	// mbForceGenericSoundDevice = mpConfig->GetBool("Sound", "ForceGeneric", false);
	mlStreamUpdateFreq = mpConfig->GetInt("Sound", "StreamUpdateFreq", 10);
	mbUseSoundThreading = mpConfig->GetBool("Sound", "UseThreading", true);
	// mbUseVoiceManagement = mpConfig->GetBool("Sound","UseVoiceManagement", true);
	mlMaxMonoChannelsHint = mpConfig->GetInt("Sound", "MaxMonoChannelsHint", 0);
	mlMaxStereoChannelsHint = mpConfig->GetInt("Sound", "MaxStereoChannelsHint", 0);
	// mlStreamBufferSize = mpConfig->GetInt("Sound", "StreamBufferSize", 64);
	// mlStreamBufferCount = mpConfig->GetInt("Sound", "StreamBufferCount", 4);
	msDeviceName = mpConfig->GetString("Sound", "DeviceName", "NULL");

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
	Vars.AddBool("ForceGeneric", mpConfig->GetBool("Sound", "ForceGeneric", false));
	Vars.AddInt("MaxSoundChannels", mlMaxSoundChannels);
	Vars.AddInt("StreamUpdateFreq", mlStreamUpdateFreq);
	Vars.AddBool("UseSoundThreading", mbUseSoundThreading);
	Vars.AddBool("UseVoiceManagement", mpConfig->GetBool("Sound", "UseVoiceManagement", true));
	Vars.AddInt("MaxMonoChannelsHint", mlMaxMonoChannelsHint);
	Vars.AddInt("MaxStereoChannelsHint", mlMaxStereoChannelsHint);
	Vars.AddInt("StreamBufferSize", mpConfig->GetInt("Sound", "StreamBufferSize", 64));
	Vars.AddInt("StreamBufferCount", mpConfig->GetInt("Sound", "StreamBufferCount", 4));
	Vars.AddString("DeviceName", mpConfig->GetString("Sound", "DeviceName", "NULL"));
	Vars.AddString("WindowCaption", "Penumbra Loading...");

	Vars.AddBool("LowLevelSoundLogging", mpConfig->GetBool("Sound", "LowLevelLogging", false));

	// Set CG Options
	//cCGProgram::SetFProfile(mpConfig->GetString("Graphics", "ForceFP", "AUTO"));
	//cCGProgram::SetVProfile(mpConfig->GetString("Graphics", "ForceVP", "AUTO"));

	LowLevelGameSetup *pSetUp = NULL;

	pSetUp = hplNew(LowLevelGameSetup, ());
	mpGame = hplNew(cGame, (pSetUp, Vars));

#ifdef TIMELIMIT
	CheckTimeLimit();
#endif

	// Make sure there really is haptic support!
	if (mbHasHaptics && cHaptic::GetIsUsed() == false) {
		CreateMessageBoxW(_W("Error!"), _W("No haptic support found. Mouse will be used instead!\n"));
		mbHasHaptics = false;
	}

	// Make sure hardware is really used.
	mbUseSoundHardware = mpGame->GetSound()->GetLowLevel()->IsHardwareAccelerated();

	mpGame->GetGraphics()->GetLowLevel()->SetVsyncActive(mbVsync);

	mbShowPreMenu = mpConfig->GetBool("Game", "ShowPreMenu", true);
	mbShowMenu = mpConfig->GetBool("Game", "ShowMenu", true);
	mbShowIntro = mpConfig->GetBool("Game", "ShowIntro", true);

	mfMaxPhysicsTimeStep = 1.0f / mfPhysicsUpdatesPerSec;

	cMath::Randomize();

	// RESOURCE INIT /////////////////////
	mpGame->GetResources()->LoadResourceDirsFile("resources.cfg");

	// LANGUAGE ////////////////////////////////
	mpGame->GetResources()->SetLanguageFile(msLanguageFile);

	Log("Initializing " PRODUCT_NAME "\n  Version\t" PRODUCT_VERSION "\n  Date\t" PRODUCT_DATE "\n");
	//////////////////////////////////////////////7
	// Check if computer supports game
	if (CheckSupport(this) == false)
		return false;

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
	mpGame->GetSound()->GetLowLevel()->SetVolume(mpConfig->GetFloat("Sound", "Volume", 1));

	// PHYSICS INIT /////////////////////
	mpGame->GetPhysics()->LoadSurfaceData("materials.cfg", mpGame->GetHaptic());

	// EARLY GAME INIT /////////////////////
	mpEffectHandler = hplNew(cEffectHandler, (this));

	// GRAPHICS INIT ////////////////////
	mpGame->GetGraphics()->GetRendererPostEffects()->SetActive(mbPostEffects);
	mpGame->GetGraphics()->GetRendererPostEffects()->SetBloomActive(mpConfig->GetBool("Graphics", "Bloom", true));
	mpGame->GetGraphics()->GetRendererPostEffects()->SetBloomSpread(6);

	mpGame->GetGraphics()->GetRendererPostEffects()->SetMotionBlurActive(mpConfig->GetBool("Graphics", "MotionBlur", false));
	mpGame->GetGraphics()->GetRendererPostEffects()->SetMotionBlurAmount(mpConfig->GetFloat("Graphics", "MotionBlurAmount", 0.3f));

	mpGame->GetGraphics()->GetRenderer3D()->SetRefractionUsed(mpConfig->GetBool("Graphics", "Refractions", false));

	mpEffectHandler->GetDepthOfField()->SetDisabled(!mpConfig->GetBool("Graphics", "DepthOfField", true));

	mpGame->GetResources()->GetMaterialManager()->SetTextureSizeLevel(mpConfig->GetInt("Graphics", "TextureSizeLevel", 0));
	mpGame->GetResources()->GetMaterialManager()->SetTextureFilter((eTextureFilter)mpConfig->GetInt("Graphics", "TextureFilter", 0));
	mpGame->GetResources()->GetMaterialManager()->SetTextureAnisotropy(mpConfig->GetFloat("Graphics", "TextureAnisotropy", 1.0f));

	mpGame->GetGraphics()->GetLowLevel()->SetGammaCorrection(mpConfig->GetFloat("Graphics", "Gamma", 1.0f));

	mpGame->GetGraphics()->GetRenderer3D()->SetShowShadows((eRendererShowShadows)mpConfig->GetInt("Graphics", "Shadows", 0));

	mpGame->SetLimitFPS(mpConfig->GetBool("Graphics", "LimitFPS", true));

	// HAPTIC INIT ////////////////////
	if (mbHasHaptics) {
		mpGame->GetHaptic()->GetLowLevel()->SetWorldScale(mfHapticScale);
		mpGame->GetHaptic()->GetLowLevel()->SetVirtualMousePosBounds(cVector2f(-60, -60),
																	 cVector2f(25, 25), cVector2f(800, 600));
		mpGame->GetHaptic()->GetLowLevel()->SetProxyRadius(mfHapticProxyRadius);
	}

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

	// mpGraphicsHelper->DrawLoadingScreen("");

	// mpIntroStory->SetActive(true);
	// mpCredits->SetActive(true);
	// mpDemoEndText->SetActive(true);

	// mpGame->SetRenderOnce(true);
	// mpGame->GetGraphics()->GetRenderer3D()->SetDebugFlags(eRendererDebugFlag_LogRendering);

	if (mbShowPreMenu) {
		mpPreMenu->SetActive(true);
	} else if (mbShowMenu) {
		mpMainMenu->SetActive(true);
	} else {
		mpMapHandler->Load(msStartMap, msStartLink);
	}

	if (gbUsingUserSettings) {
		mpConfig->SetBool("Game", "LastInitOK", true);
		mpConfig->Save();
	}

	//////////////////////////////////////////////////////////
	// Create newer settings file, if using default.
	if (gbUsingUserSettings == false) {
		if (mpConfig)
			hplDelete(mpConfig);
		mpConfig = hplNew(cConfigFile, (gsUserSettingsPath));
		gbUsingUserSettings = true;
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
	mpConfig->SetBool("Haptics", "Active", mbHasHapticsOnRestart);
	mpConfig->SetBool("Haptics", "Available", mbHapticsAvailable);
	mpConfig->SetFloat("Haptics", "ForceMul", mfHapticForceMul);
	mpConfig->SetFloat("Haptics", "MoveScreenSpeedMul", mfHapticMoveScreenSpeedMul);
	mpConfig->SetFloat("Haptics", "Scale", mfHapticScale);
	mpConfig->SetFloat("Haptics", "ProxyRadius", mfHapticProxyRadius);
	mpConfig->SetFloat("Haptics", "OffsetZ", mfHapticOffsetZ);
	mpConfig->SetFloat("Haptics", "HapticMaxInteractDist", mfHapticMaxInteractDist);
	mpConfig->SetFloat("Haptics", "ProxyRadius", mfHapticProxyRadius);
	mpConfig->SetFloat("Haptics", "OffsetZ", mfHapticOffsetZ);
	if (mbHasHaptics) {
		mpConfig->SetFloat("Haptics", "InteractModeCameraSpeed", mpPlayer->GetHapticCamera()->GetInteractModeCameraSpeed());
		mpConfig->SetFloat("Haptics", "ActionModeCameraSpeed", mpPlayer->GetHapticCamera()->GetActionModeCameraSpeed());
	}

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
	// Log(" Exit Game Scripts\n");
	// hplDelete( mpGameScripts );
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

	Log(" Saving config\n");
	// Save engine stuff.
	mpConfig->SetBool("Graphics", "Bloom", mpGame->GetGraphics()->GetRendererPostEffects()->GetBloomActive());
	mpConfig->SetBool("Graphics", "MotionBlur", mpGame->GetGraphics()->GetRendererPostEffects()->GetMotionBlurActive());
	mpConfig->SetFloat("Graphics", "MotionBlurAmount", mpGame->GetGraphics()->GetRendererPostEffects()->GetMotionBlurAmount());
	mpConfig->SetBool("Graphics", "DepthOfField", !mpEffectHandler->GetDepthOfField()->IsDisabled());
	mpConfig->GetBool("Graphics", "Refractions", mpGame->GetGraphics()->GetRenderer3D()->GetRefractionUsed());

	mpConfig->SetFloat("Sound", "Volume", mpGame->GetSound()->GetLowLevel()->GetVolume());
	mpConfig->SetBool("Sound", "UseSoundHardware", mbUseSoundHardware);
	mpConfig->SetInt("Sound", "MaxSoundChannels", mlMaxSoundChannels);
	mpConfig->SetInt("Sound", "StreamUpdateFreq", mlStreamUpdateFreq);
	mpConfig->SetBool("Sound", "UseThreading", mbUseSoundThreading);
	mpConfig->SetInt("Sound", "MaxMonoChannelsHint", mlMaxMonoChannelsHint);
	mpConfig->SetInt("Sound", "MaxStereoChannelsHint", mlMaxStereoChannelsHint);
	mpConfig->SetString("Sound", "DeviceName", msDeviceName);

	mpConfig->SetInt("Graphics", "TextureSizeLevel", mpGame->GetResources()->GetMaterialManager()->GetTextureSizeLevel());
	mpConfig->SetInt("Graphics", "TextureFilter", mpGame->GetResources()->GetMaterialManager()->GetTextureFilter());
	mpConfig->SetFloat("Graphics", "TextureAnisotropy", mpGame->GetResources()->GetMaterialManager()->GetTextureAnisotropy());

	mpConfig->SetFloat("Graphics", "Gamma", mpGame->GetGraphics()->GetLowLevel()->GetGammaCorrection());

	mpConfig->SetInt("Graphics", "FSAA", mlFSAA);
	mpConfig->SetBool("Graphics", "PostEffects", mbPostEffects);
	mpConfig->SetInt("Graphics", "ShaderQuality", iMaterial::GetQuality());
	mpConfig->SetBool("Graphics", "LimitFPS", mpGame->GetLimitFPS());

	mpConfig->SetInt("Graphics", "Shadows", mpGame->GetGraphics()->GetRenderer3D()->GetShowShadows());

	Log(" Exit Effect Handler\n");
	hplDelete(mpEffectHandler);

	Log(" Exit Game\n");

	// MAIN EXIT /////////////////////
	// Delete the game,  do this after all else is deleted.
	hplDelete(mpGame);

	Log(" Saving last config\n");
	// Save the stuff to the config file
	mpConfig->SetInt("Screen", "Width", mvScreenSize.x);
	mpConfig->SetInt("Screen", "Height", mvScreenSize.y);
	mpConfig->SetBool("Screen", "FullScreen", mbFullScreen);
	mpConfig->SetBool("Screen", "Vsync", mbVsync);

	mpConfig->SetString("Demo", "WebPageOnExit", msWebPageOnExit);

	mpConfig->SetString("Map", "GlobalScript", msGlobalScriptFile);

	mpConfig->SetBool("Game", "Subtitles", mbSubtitles);
	mpConfig->SetBool("Game", "SimpleWeaponSwing", mbSimpleWeaponSwing);
	mpConfig->SetBool("Game", "DisablePersonalNotes", mbDisablePersonalNotes);
	mpConfig->SetBool("Game", "AllowQuickSave", mbAllowQuickSave);
	mpConfig->SetBool("Game", "FlashItems", mbFlashItems);
	mpConfig->SetBool("Game", "ShowCrossHair", mbShowCrossHair);

	mpConfig->SetBool("Game", "SimpleSwingInOptions", mbSimpleSwingInOptions);

	mpConfig->SetString("Map", "File", msStartMap);
	mpConfig->SetString("Map", "StartPos", msStartLink);

	mpConfig->SetInt("Game", "Difficulty", mDifficulty);
	mpConfig->SetString("Game", "CurrentUser", msCurrentUser);
	mpConfig->SetString("Game", "LanguageFile", msLanguageFile);
	mpConfig->SetBool("Game", "ShowPreMenu", mbShowPreMenu);
	mpConfig->SetBool("Game", "ShowMenu", mbShowMenu);
	mpConfig->SetBool("Game", "ShowIntro", mbShowIntro);

	mpConfig->SetInt("Physics", "PhysicsAccuracy", mPhysicsAccuracy);
	mpConfig->SetFloat("Physics", "UpdatesPerSec", mfPhysicsUpdatesPerSec);

	mpConfig->SetBool("Debug", "LogResources", mbLogResources);

	mpConfig->Save();
	hplDelete(mpConfig);

	hplDelete(mpGameConfig);

#ifdef DEMO_VERSION
	if (msWebPageOnExit != "")
		OpenBrowserWindow(cString::To16Char(msWebPageOnExit));
#endif
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

	/*iSoundData *pSound = mpGame->GetResources()->GetSoundManager()->CreateSoundData(asFile,false);
	if(pSound){
		Warning("Couldn't preload sound '%s'\n",asFile.c_str());
	}*/
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
