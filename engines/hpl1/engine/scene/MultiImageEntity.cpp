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

#include "hpl1/engine/scene/MultiImageEntity.h"

#include "hpl1/debug.h"
#include "hpl1/engine/scene/Node2D.h"
#include "hpl1/engine/scene/Scene.h"
#include "hpl1/engine/scene/World2D.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMultiImageEntity::cMultiImageEntity(class cScene *apScene, cNode2D *apNode) {
	mpScene = apScene;
	mpNode = apNode;

	mbFlipH = false;
	mbActive = true;
	mfAlpha = 1;
	mbAnimPaused = false;
}

//-----------------------------------------------------------------------

cMultiImageEntity::~cMultiImageEntity() {
	tMultiImagePartMapIt PartIt = m_mapEntityParts.begin();
	while (PartIt != m_mapEntityParts.end()) {
		for (int i = 0; i < (int)PartIt->second.mvEntity.size(); i++) {
			/*Destroy entity*/ // Not really needed since the map will do it..
		}

		PartIt++;
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------
bool cMultiImageEntity::Add(tString asName, tString asFile, cVector3f avLocalPos, tFlag alPartId) {
	cImageEntity *pEntity = mpScene->GetWorld2D()->CreateImageEntity(asName, asFile);
	if (pEntity == NULL) {
		error("Couldn't load image entity '%s'", asFile.c_str());
		return false;
	}

	return Add(asName, pEntity, avLocalPos, alPartId);
}

//-----------------------------------------------------------------------

bool cMultiImageEntity::Add(tString asName, cImageEntity *apEntity, cVector3f avLocalPos, tFlag alPartId) {
	apEntity->SetPosition(avLocalPos);
	mpNode->AddEntity(apEntity);

	tMultiImagePartMapIt PartIt = m_mapEntityParts.find(alPartId);
	if (PartIt == m_mapEntityParts.end()) {
		cMultiImagePart ImagePart;
		ImagePart.mlActiveEntity = 0;
		ImagePart.mlId = alPartId;

		ImagePart.mlPrio = 0;
		ImagePart.mlNextAnimPrio = 0;
		ImagePart.msNextAnim = "";
		ImagePart.mbSyncFrame = false;

		m_mapEntityParts.insert(tMultiImagePartMap::value_type(alPartId, ImagePart));
		PartIt = m_mapEntityParts.find(alPartId);
	}

	PartIt->second.mvEntity.push_back(apEntity);

	return true;
}
//-----------------------------------------------------------------------

cImageEntity *cMultiImageEntity::GetEntity(int alPartId) {
	tMultiImagePartMapIt PartIt = m_mapEntityParts.find(alPartId);
	if (PartIt == m_mapEntityParts.end())
		return NULL;

	return PartIt->second.mvEntity[PartIt->second.mlActiveEntity];
}

//-----------------------------------------------------------------------

bool cMultiImageEntity::PlayAnim(const tString &asName, tFlag alParts, unsigned int alPrio, bool abLoop, bool abSyncFrame) {
	tMultiImagePartMapIt PartIt = m_mapEntityParts.begin();
	while (PartIt != m_mapEntityParts.end()) {
		// See if the part is among the parts to get the animation.
		if ((PartIt->second.mlId & alParts) == 0) {
			PartIt++;
			continue;
		}

		cImageEntity *pEntity = PartIt->second.mvEntity[PartIt->second.mlActiveEntity];
		if (pEntity->GetCurrentAnimation() == asName) {
			PartIt++;
			continue;
		}

		// Check if the prio is lower than current last animation, if so
		// check if this animation should be played after and continue.
		if (PartIt->second.mlPrio > alPrio) {
			if (PartIt->second.mlNextAnimPrio <= alPrio) {
				PartIt->second.msNextAnim = asName;
			}
			PartIt++;
			continue;
		}

		PartIt->second.msNextAnim = pEntity->GetCurrentAnimation();

		pEntity->SetAnimation(asName, abLoop);

		PartIt->second.mlPrio = alPrio;
		PartIt->second.mbSyncFrame = abSyncFrame;

		PartIt++;
	}

	return true;
}

//-----------------------------------------------------------------------

void cMultiImageEntity::UpdateAnim() {
	tMultiImagePartMapIt PartIt = m_mapEntityParts.begin();
	while (PartIt != m_mapEntityParts.end()) {
		cImageEntity *pEntity = PartIt->second.mvEntity[PartIt->second.mlActiveEntity];

		if (pEntity->AnimationIsPlaying() == false) {
			if (PartIt->second.msNextAnim != "") {
				pEntity->SetAnimation(PartIt->second.msNextAnim);

				// If chosen, synch with the frame of another entity playing the animation.
				if (PartIt->second.mbSyncFrame) {
					tMultiImagePartMapIt it = m_mapEntityParts.begin();
					while (it != m_mapEntityParts.end()) {
						cImageEntity *pTemp = it->second.mvEntity[it->second.mlActiveEntity];

						if (pTemp->GetCurrentAnimation() == PartIt->second.msNextAnim) {
							pEntity->SetFrameNum(pTemp->GetFrameNum());
							break;
						}
						it++;
					}
				}

				PartIt->second.msNextAnim = "";
				PartIt->second.mlPrio = PartIt->second.mlNextAnimPrio;
				PartIt->second.mlNextAnimPrio = 0;
				PartIt->second.mbSyncFrame = false;
			}
		}

		PartIt++;
	}
}
//-----------------------------------------------------------------------

void cMultiImageEntity::SetFlipH(bool abX) {
	if (mbFlipH == abX)
		return;

	mbFlipH = abX;

	tMultiImagePartMapIt PartIt = m_mapEntityParts.begin();
	while (PartIt != m_mapEntityParts.end()) {
		for (int i = 0; i < (int)PartIt->second.mvEntity.size(); i++) {
			PartIt->second.mvEntity[i]->SetFlipH(mbFlipH);
			cVector3f vPos = PartIt->second.mvEntity[i]->GetLocalPosition();

			PartIt->second.mvEntity[i]->SetPosition(cVector3f(-vPos.x, vPos.y, vPos.z));
		}

		PartIt++;
	}
}
//-----------------------------------------------------------------------

void cMultiImageEntity::SetActive(bool abX) {
	if (mbActive == abX)
		return;

	mbActive = abX;

	tMultiImagePartMapIt PartIt = m_mapEntityParts.begin();
	while (PartIt != m_mapEntityParts.end()) {
		for (int i = 0; i < (int)PartIt->second.mvEntity.size(); i++) {
			PartIt->second.mvEntity[i]->SetActive(mbActive);
		}

		PartIt++;
	}
}

//-----------------------------------------------------------------------

void cMultiImageEntity::SetAlpha(float afX) {
	if (mfAlpha == afX)
		return;

	mfAlpha = afX;

	tMultiImagePartMapIt PartIt = m_mapEntityParts.begin();
	while (PartIt != m_mapEntityParts.end()) {
		for (int i = 0; i < (int)PartIt->second.mvEntity.size(); i++) {
			PartIt->second.mvEntity[i]->SetAlpha(afX);
		}

		PartIt++;
	}
}

//-----------------------------------------------------------------------

void cMultiImageEntity::SetAnimPaused(bool abX) {
	if (mbAnimPaused == abX)
		return;

	mbAnimPaused = abX;

	tMultiImagePartMapIt PartIt = m_mapEntityParts.begin();
	while (PartIt != m_mapEntityParts.end()) {
		for (int i = 0; i < (int)PartIt->second.mvEntity.size(); i++) {
			PartIt->second.mvEntity[i]->SetAnimationPaused(abX);
		}

		PartIt++;
	}
}

//-----------------------------------------------------------------------

void cMultiImageEntity::Flash(float afX) {
	tMultiImagePartMapIt PartIt = m_mapEntityParts.begin();
	while (PartIt != m_mapEntityParts.end()) {
		for (int i = 0; i < (int)PartIt->second.mvEntity.size(); i++) {
			PartIt->second.mvEntity[i]->Flash(afX);
		}

		PartIt++;
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
} // namespace hpl
