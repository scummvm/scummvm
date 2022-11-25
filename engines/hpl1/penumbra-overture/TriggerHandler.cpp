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

#include "hpl1/penumbra-overture/TriggerHandler.h"

#include "hpl1/algorithms.h"
#include "hpl1/penumbra-overture/GameEnemy.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/MapHandler.h"

//////////////////////////////////////////////////////////////////////////
// TRIGGER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameTrigger::cGameTrigger() : iEntity3D("") {
	SetRadius(1);
}

//-----------------------------------------------------------------------

void cGameTrigger::SetRadius(float afX) {
	mfRadius = afX;
	mBoundingVolume.SetSize(mfRadius * 2);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cTriggerHandler::cTriggerHandler(cInit *apInit) : iUpdateable("TriggerHandler") {
	mpInit = apInit;
}

//-----------------------------------------------------------------------

cTriggerHandler::~cTriggerHandler(void) {
	STLMapDeleteAll(m_mapTriggers);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameTrigger *cTriggerHandler::Add(cGameTrigger *apTrigger, eGameTriggerType aType,
								   const cVector3f &avLocalPos,
								   int alPrio, float afTime, float afRadius) {
	apTrigger->SetRadius(afRadius);
	apTrigger->mfTimeCount = afTime;
	apTrigger->mlPrio = alPrio;
	apTrigger->mType = aType;
	apTrigger->SetPosition(avLocalPos);

	m_mapTriggers.insert(tGameTriggerMap::value_type(alPrio, apTrigger));

	return apTrigger;
}

//-----------------------------------------------------------------------

void cTriggerHandler::OnStart() {
	mpMapHandler = mpInit->mpMapHandler;
}

//-----------------------------------------------------------------------

void cTriggerHandler::Update(float afTimeStep) {
	//////////////////////////////////////////////
	// Go through the enemies and hand them triggers
	tGameEnemyIterator EnemyIt = mpMapHandler->GetGameEnemyIterator();
	while (EnemyIt.HasNext()) {
		iGameEnemy *pEnemy = EnemyIt.Next();
		if (pEnemy->IsActive() == false || pEnemy->GetUsesTriggers() == false)
			continue;

		/////////////////////////////////////////
		// Check if it is time to update triggers.
		/*if(pEnemy->GetTriggerUpdateCount() >= pEnemy->GetTriggerUpdateRate())
		{
			pEnemy->SetTriggerUpdateCount(0);
		}
		else
		{
			pEnemy->SetTriggerUpdateCount(pEnemy->GetTriggerUpdateCount() + afTimeStep);
			continue;
		}*/

		/////////////////////////////////////
		// Go through triggers by priority
		tGameTriggerMapIt TriggerIt = m_mapTriggers.begin();
		for (; TriggerIt != m_mapTriggers.end(); ++TriggerIt) {
			cGameTrigger *pTrigger = TriggerIt->second;

			// Check if trigger is of right type
			if (!(pTrigger->GetType() & pEnemy->GetTriggerTypes())) {
				continue;
			}

			// Check if trigger is in reach
			if (cMath::PointBVCollision(pEnemy->GetPosition(), *pTrigger->GetBoundingVolume()) == false) {
				continue;
			}

			// Let enemy handle trigger, if false get next trigger else next enemy.
			if (pEnemy->HandleTrigger(pTrigger)) {
				break;
			}
		}
	}

	//////////////////////////////////
	// Go through triggers and remove when timer is out.
	auto const newEnd = Hpl1::removeIf(m_mapTriggers.begin(), m_mapTriggers.end(), [afTimeStep](tGameTriggerMap::value_type &pair) {
		pair.second->mfTimeCount -= afTimeStep;
		if (pair.second->mfTimeCount <= 0) {
			hplDelete(pair.second);
			return true;
		}
		return false;
	});
	m_mapTriggers.erase(newEnd, m_mapTriggers.end());
}

//-----------------------------------------------------------------------

void cTriggerHandler::Reset() {
}
