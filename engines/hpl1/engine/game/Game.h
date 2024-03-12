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

#ifndef HPL_GAME_H
#define HPL_GAME_H

#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

class cUpdater;
class LowLevelGameSetup;
class LowLevelSystem;
class cLogicTimer;

class cSystem;
class cInput;
class cResources;
class cGraphics;
class cScene;
class cSound;
class cPhysics;
class cAI;
class cGui;

class cFPSCounter {
public:
	cFPSCounter(LowLevelSystem *apLowLevelSystem);

	void AddFrame();

	float mfFPS;
	float mfUpdateRate;

private:
	LowLevelSystem *mpLowLevelSystem;
	int mlFramecounter;
	float mfFrametimestart;
	float mfFrametime;
};

//---------------------------------------------------

class cSetupVarContainer {
public:
	cSetupVarContainer();

	void AddString(const tString &asName, const tString &asValue);

	void AddInt(const tString &asName, int alValue);
	void AddFloat(const tString &asName, float afValue);
	void AddBool(const tString &asName, bool abValue);

	const tString &GetString(const tString &asName);

	float GetFloat(const tString &asName, float afDefault);
	int GetInt(const tString &asName, int alDefault);
	bool GetBool(const tString &asName, bool abDefault);

private:
	Common::StableMap<tString, tString> m_mapVars;
	tString msBlank;
};

//---------------------------------------------------

class cGame {
public:
	cGame(LowLevelGameSetup *apGameSetup, cSetupVarContainer &aVars);
	cGame(LowLevelGameSetup *apGameSetup, int alWidth, int alHeight, int alBpp, bool abFullscreen,
		  unsigned int alUpdateRate = 60, int alMultisampling = 0);
	~cGame();

private:
	void GameInit(LowLevelGameSetup *apGameSetup, cSetupVarContainer &aVars);

public:
	/**
	 * Starts the game loop. To make stuff run they must be added as updatables..
	 */
	void Run();
	/**
	 * Exists the game.
	 * \todo is this a good way to do it? Should game be global. If so, make a singleton.
	 */
	void Exit();

	/**
	 *
	 * \return A pointer to Scene
	 */
	cScene *GetScene();
	/**
	 *
	 * \return A pointer to Resources
	 */
	cResources *GetResources();

	/**
	 *
	 * \return A pointer to the Updater
	 */
	cUpdater *GetUpdater();

	/**
	 *
	 * \return A pointer to the System
	 */
	cSystem *GetSystem();

	/**
	 *
	 * \return A pointer to the Input
	 */
	cInput *GetInput();

	/**
	 *
	 * \return A pointer to the Graphics
	 */
	cGraphics *GetGraphics();
	/**
	 *
	 * \return A pointer to the Sound
	 */
	cSound *GetSound();
	/**
	 *
	 * \return A pointer to the Physics
	 */
	cPhysics *GetPhysics();

	/**
	 *
	 * \return A pointer to the AI
	 */
	cAI *GetAI();

	/**
	 *
	 * \return A pointer to the Gui
	 */
	cGui *GetGui();

	void ResetLogicTimer();
	void SetUpdatesPerSec(int alUpdatesPerSec);
	int GetUpdatesPerSec();
	float GetStepSize();

	cLogicTimer *GetLogicTimer() { return mpLogicTimer; }

	float GetFPS();

	void SetFPSUpdateRate(float afSec);
	float GetFPSUpdateRate();

	float GetFrameTime() { return mfFrameTime; }

	float GetUpdateTime() { return mfUpdateTime; }

	double GetGameTime() { return mfGameTime; }

	void SetLimitFPS(bool abX) { mbLimitFPS = abX; }
	bool GetLimitFPS() { return mbLimitFPS; }

private:
	bool mbGameIsDone;

	float mfFrameTime;

	float mfUpdateTime;

	double mfGameTime;

	LowLevelGameSetup *mpGameSetup;
	cUpdater *mpUpdater;
	cLogicTimer *mpLogicTimer;

	cFPSCounter *mpFPSCounter;

	bool mbLimitFPS;

	// Modules that Game connnect to:
	cResources *mpResources;
	cSystem *mpSystem;
	cInput *mpInput;
	cGraphics *mpGraphics;
	cScene *mpScene;
	cSound *mpSound;
	cPhysics *mpPhysics;
	cAI *mpAI;
	cGui *mpGui;
};

} // namespace hpl

#endif // HPL_GAME_H
