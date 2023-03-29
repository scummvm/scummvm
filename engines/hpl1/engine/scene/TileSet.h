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

#ifndef HPL_TILESET_H
#define HPL_TILESET_H

#include "common/array.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/resources/ResourceBase.h"
#include "hpl1/engine/scene/TileData.h"
#include "hpl1/engine/system/SystemTypes.h"

class TiXmlElement;

namespace hpl {

#define kMaxTileFrameWidth (9)

typedef Common::Array<iTileData *> tTileDataVec;
typedef tTileDataVec::iterator tTileDataVecIt;

class cResources;

class cTileSet : public iResourceBase {
public:
	cTileSet(tString asName, cGraphics *apGraphics, cResources *apResources);
	~cTileSet();

	// resource stuff:
	bool reload() { return false; }
	void unload() {}
	void destroy() {}

	void Add(iTileData *apData);
	iTileData *Get(int alNum);

	bool CreateFromFile(const tString &asFile);

private:
	float mfTileSize;
	tTileDataVec mvData;
	cResources *mpResources;
	cGraphics *mpGraphics;

	int mlNum;
	int mvImageHandle[eMaterialTexture_LastEnum];
	cVector2l mvFrameSize;

	bool LoadData(TiXmlElement *pElement);
	void GetTileNum(TiXmlElement *apElement);
};

} // namespace hpl

#endif // HPL_TILESET_H
