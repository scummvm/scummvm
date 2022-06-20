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

#include "hpl1/penumbra-overture/GameMusicHandler.h"

#include "hpl1/penumbra-overture/CharacterMove.h"
#include "hpl1/penumbra-overture/GameEnemy.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/MapHandler.h"
#include "hpl1/penumbra-overture/Player.h"
#include "hpl1/penumbra-overture/SaveHandler.h"

//////////////////////////////////////////////////////////////////////////
// TRIGGER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameMusic::cGameMusic() {
	Reset();
}

//-----------------------------------------------------------------------

void cGameMusic::Reset() {
	msFile = "";
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameMusicHandler::cGameMusicHandler(cInit *apInit) : iUpdateable("GameMusicHandler") {
	mpInit = apInit;
	mpMusicHandler = mpInit->mpGame->GetSound()->GetMusicHandler();

	mlMaxPrio = 10;

	mvGameMusic.resize(mlMaxPrio + 1);

	Reset();
}

//-----------------------------------------------------------------------

cGameMusicHandler::~cGameMusicHandler(void) {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameMusicHandler::OnStart() {
}

//-----------------------------------------------------------------------

void cGameMusicHandler::Update(float afTimeStep) {
	///////////////////////////////////
	// Check if close by music should be played.

	// Music not playing
	if (mbAttackPlaying == false) {
		if (mbEnemyClosePlaying == false) {
			tString sMusic = "";
			int lHighestPrio = -1;
			tGameEnemyIterator enemyIt = mpInit->mpMapHandler->GetGameEnemyIterator();
			while (enemyIt.HasNext()) {
				iGameEnemy *pEnemy = enemyIt.Next();

				if (pEnemy->GetHealth() <= 0 || pEnemy->IsActive() == false)
					continue;

				float fDist = cMath::Vector3Dist(pEnemy->GetMover()->GetCharBody()->GetFeetPosition(),
												 mpInit->mpPlayer->GetCharacterBody()->GetFeetPosition());

				if (fDist < pEnemy->GetCloseMusicStartDist()) {
					if (pEnemy->GetCloseMusic() != "" && (sMusic == "" || lHighestPrio < pEnemy->GetCloseMusicPrio())) {
						sMusic = pEnemy->GetCloseMusic();
						lHighestPrio = pEnemy->GetCloseMusicPrio();
					}
				}
			}

			if (sMusic != "") {
				mfEnemyGoneCount = 0;
				mfEnemyCloseCount += afTimeStep;
				if (mfEnemyCloseCount > 2.0f) {
					Play(sMusic, true, 1, 0.25f, 5);
					mbEnemyClosePlaying = true;
				}
			}
		}
		// Music playing
		else {
			bool bFound;
			tGameEnemyIterator enemyIt = mpInit->mpMapHandler->GetGameEnemyIterator();
			while (enemyIt.HasNext()) {
				iGameEnemy *pEnemy = enemyIt.Next();

				if (pEnemy->GetHealth() <= 0 || pEnemy->IsActive() == false)
					continue;

				float fDist = cMath::Vector3Dist(pEnemy->GetMover()->GetCharBody()->GetFeetPosition(),
												 mpInit->mpPlayer->GetCharacterBody()->GetFeetPosition());

				if (fDist < pEnemy->GetCloseMusicStopDist()) {
					bFound = true;
				}
			}

			if (bFound == false) {
				mfEnemyCloseCount = 0;
				mfEnemyGoneCount += afTimeStep;
				if (mfEnemyGoneCount > 6.0f) {
					Stop(0.2f, 5);
					mbEnemyClosePlaying = false;
				}
			}
		}
	}

	///////////////////////////////////
	// Check if attack music should be played.
	if (mbAttackPlaying == false && m_setAttackers.empty() == false) {
		mfAttackStopCount = 0;
		mfAttackPlayCount += afTimeStep;

		if (mfAttackPlayCount > 1.5f) {
			//////////////////////
			// Go through attackers and get music with highest priority
			tString sMusic = "";
			int lMaxPrio = -9999;

			tAttackerSetIt it = m_setAttackers.begin();
			for (; it != m_setAttackers.end(); ++it) {
				iGameEnemy *pEnemy = *it;

				if (pEnemy->GetAttackMusic() != "" && pEnemy->GetAttackMusicPrio() > lMaxPrio) {
					sMusic = pEnemy->GetAttackMusic();
					lMaxPrio = pEnemy->GetAttackMusicPrio();
				}
			}

			//////////////////////
			// Play music if found
			if (sMusic != "") {
				if (mbEnemyClosePlaying) {
					mbEnemyClosePlaying = false;
					Stop(0.2f, 5);
				}

				Play(sMusic, true, 1, 1.3f, 9);
				mbAttackPlaying = true;
				// Log("Play!");
				mfAttackPlayCount = 0;
			}
		}
	} else if (mbAttackPlaying && m_setAttackers.empty()) {
		mfAttackPlayCount = 0;
		mfAttackStopCount += afTimeStep;

		if (mfAttackStopCount > 1.2f) {
			mbAttackPlaying = false;
			Stop(0.2f, 9);
			// Log("Stop!");
			mfAttackStopCount = 0;
		}
	}

	///////////////////////////////////
	// Check if current song is over
	if (mlCurrentMaxPrio >= 0) {
		if (mpMusicHandler->GetCurrentSong() == NULL) {
			mvGameMusic[mlCurrentMaxPrio].msFile = "";
			mlCurrentMaxPrio = -1;
			PlayHighestPriority();
		}
	}
}

//-----------------------------------------------------------------------

void cGameMusicHandler::Reset() {
	for (size_t i = 0; i < mvGameMusic.size(); ++i) {
		mvGameMusic[i].Reset();
	}

	mlCurrentMaxPrio = -1;

	mbAttackPlaying = false;
	m_setAttackers.clear();

	mfAttackPlayCount = 0;
	mfAttackStopCount = 0;

	mbEnemyClosePlaying = false;
	mfEnemyCloseCount = 0;
	mfEnemyGoneCount = 0;
}

//-----------------------------------------------------------------------

void cGameMusicHandler::Play(const tString &asFile, bool abLoop, float afVolume, float afFade, int alPrio) {
	if (alPrio > mlMaxPrio)
		alPrio = mlMaxPrio;

	if (mvGameMusic[alPrio].msFile == asFile)
		return;

	// If higher or equal to song playing, start playing
	if (mlCurrentMaxPrio <= alPrio) {
		mpMusicHandler->Play(asFile, afVolume, afFade, abLoop);
		mlCurrentMaxPrio = alPrio;
	}

	// Set game music properites
	mvGameMusic[alPrio].mbLoop = abLoop;
	mvGameMusic[alPrio].mfVolume = afVolume;
	mvGameMusic[alPrio].msFile = asFile;
}

//-----------------------------------------------------------------------

void cGameMusicHandler::Stop(float afFade, int alPrio) {
	if (alPrio > mlMaxPrio)
		alPrio = mlMaxPrio;

	// Check if there is any song playing at this prio
	if (mvGameMusic[alPrio].msFile != "") {
		mvGameMusic[alPrio].msFile = "";

		// Check the current song playing is this
		if (alPrio == mlCurrentMaxPrio) {
			mpMusicHandler->Stop(afFade);

			// Play next song in priority if there is any.
			mlCurrentMaxPrio = -1;
			PlayHighestPriority();
		}
	}
}

//-----------------------------------------------------------------------

void cGameMusicHandler::AddAttacker(iGameEnemy *apEntity) {
	m_setAttackers.insert(apEntity);
}

//-----------------------------------------------------------------------

void cGameMusicHandler::RemoveAttacker(iGameEnemy *apEntity) {
	m_setAttackers.erase(apEntity);
}

bool cGameMusicHandler::AttackerExist(iGameEnemy *apEntity) {
	tAttackerSetIt it = m_setAttackers.find(apEntity);
	if (it == m_setAttackers.end())
		return false;

	return true;
}

//-----------------------------------------------------------------------

void cGameMusicHandler::OnWorldLoad() {
	m_setAttackers.clear();

	// Update(2.0f);
}

void cGameMusicHandler::OnWorldExit() {
	m_setAttackers.clear();

	for (int i = 0; i < 60; ++i)
		Update(0.1f);
}

//-----------------------------------------------------------------------

void cGameMusicHandler::LoadFromGlobal(cGameMusicHandler_GlobalSave *apSave) {
	mlCurrentMaxPrio = apSave->mlCurrentMaxPrio;
	mbEnemyClosePlaying = apSave->mbEnemyClosePlaying;
	mbAttackPlaying = apSave->mbAttackPlaying;

	for (size_t i = 0; i < mvGameMusic.size(); ++i) {
		mvGameMusic[i].msFile = apSave->mvGameMusic[i].msFile;
		mvGameMusic[i].mfVolume = apSave->mvGameMusic[i].mfVolume;
		mvGameMusic[i].mbLoop = apSave->mvGameMusic[i].mbLoop;
	}
}

//-----------------------------------------------------------------------

void cGameMusicHandler::SaveToGlobal(cGameMusicHandler_GlobalSave *apSave) {
	apSave->mlCurrentMaxPrio = mlCurrentMaxPrio;
	apSave->mbEnemyClosePlaying = mbEnemyClosePlaying;
	apSave->mbAttackPlaying = mbAttackPlaying;

	apSave->mvGameMusic.Resize(mvGameMusic.size());
	for (size_t i = 0; i < mvGameMusic.size(); ++i) {
		apSave->mvGameMusic[i].msFile = mvGameMusic[i].msFile;
		apSave->mvGameMusic[i].mfVolume = mvGameMusic[i].mfVolume;
		apSave->mvGameMusic[i].mbLoop = mvGameMusic[i].mbLoop;
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameMusicHandler::PlayHighestPriority() {
	for (int i = mlMaxPrio; i >= 0; --i) {
		if (mvGameMusic[i].msFile != "") {
			// If looped, play
			if (mvGameMusic[i].mbLoop) {
				mpMusicHandler->Play(mvGameMusic[i].msFile,
									 mvGameMusic[i].mfVolume,
									 0.3f, true);
				mlCurrentMaxPrio = (int)i;
				break;
			}
			// If not looped remove
			else {
				mvGameMusic[i].msFile = "";
			}
		}
	}
}

//-----------------------------------------------------------------------
