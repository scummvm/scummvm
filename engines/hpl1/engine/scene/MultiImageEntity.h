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

#ifndef HPL_MULTI_IMAGE_ENTITY_H
#define HPL_MULTI_IMAGE_ENTITY_H

#include "hpl1/engine/scene/ImageEntity.h"
#include "common/stablemap.h"

namespace hpl {

class cScene;
class cNode2D;

class cMultiImagePart {
public:
	tFlag mlId;
	int mlActiveEntity;
	tImageEntityVec mvEntity;

	unsigned int mlPrio;
	unsigned int mlNextAnimPrio;
	tString msNextAnim;
	bool mbSyncFrame;
};

typedef Common::StableMap<unsigned int, cMultiImagePart> tMultiImagePartMap;
typedef tMultiImagePartMap::iterator tMultiImagePartMapIt;

class cMultiImageEntity {
public:
	cMultiImageEntity(class cScene *apScene, cNode2D *apNode);
	~cMultiImageEntity();

	bool Add(tString asName, tString asFile, cVector3f avLocalPos, tFlag alPartId);
	bool Add(tString asName, cImageEntity *apEntity, cVector3f avLocalPos, tFlag alPartId);

	void SetFlipH(bool abX);

	void SetAlpha(float afX);
	void Flash(float afX);

	void SetActive(bool abX);
	bool GetActive() { return mbActive; }

	bool PlayAnim(const tString &asName, tFlag alParts, unsigned int alPrio, bool abLoop = true, bool abSyncFrame = false);
	void UpdateAnim();

	void SetAnimPaused(bool abX);
	bool GetAnimPaused() { return mbAnimPaused; }

	cImageEntity *GetEntity(int alPartId);

private:
	bool mbActive;
	float mfAlpha;
	bool mbAnimPaused;

	cScene *mpScene;
	cNode2D *mpNode;

	tMultiImagePartMap m_mapEntityParts;

	bool mbFlipH;
};

} // namespace hpl

#endif // HPL_MULTI_IMAGE_ENTITY_H
