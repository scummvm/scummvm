/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef FULLPIPE_OBJECTS_H
#define FULLPIPE_OBJECTS_H

#include "fullpipe/utils.h"

namespace Fullpipe {

class MessageQueue;
class SceneTagList;

class GameProject : public CObject {
 public:
	int _field_4;
	char *_headerFilename;
	SceneTagList *_sceneTagList;
	int _field_10;

 public:
	GameProject();
	~GameProject();
	virtual bool load(MfcArchive &file);
};

struct PicAniInfo {
	int32 type;
	int16 objectId;
	int16 field_6;
	int32 field_8;
	int16 sceneId;
	int16 field_E;
	int32 ox;
	int32 oy;
	int32 priority;
	int16 staticsId;
	int16 movementId;
	int16 dynamicPhaseIndex;
	int16 flags;
	int32 field_24;
	int32 someDynamicPhaseIndex;

	bool load(MfcArchive &file);
};

class CMotionController;

class Sc2 : public CObject {
 public:
	int16 _sceneId;
	int16 _field_2;
	Scene *_scene;
	CMotionController *_motionController;
	int32 *_data1; // FIXME, could be a struct
	int _count1;
	PicAniInfo **_defPicAniInfos;
	int _defPicAniInfosCount;
	PicAniInfo **_picAniInfos;
	int _picAniInfosCount;
	int _isLoaded;
	EntranceInfo **_entranceData;
	int _entranceDataCount;

 public:
	Sc2();
	virtual bool load(MfcArchive &file);
};

typedef Common::Array<Sc2> Sc2Array;

union VarValue {
	float floatValue;
	int32 intValue;
	char *stringValue;
};

class CGameVar : public CObject {
 public:
	CGameVar *_nextVarObj;
	CGameVar *_prevVarObj;
	CGameVar *_parentVarObj;
	CGameVar *_subVars;
	CGameVar *_field_14;
	char *_varName;
	VarValue _value;
	int _varType;

 public:
	CGameVar();
	virtual bool load(MfcArchive &file);
	CGameVar *getSubVarByName(const char *name);
	bool setSubVarAsInt(const char *name, int value);
	int getSubVarAsInt(const char *name);
	CGameVar *addSubVarAsInt(const char *name, int value);
	bool addSubVar(CGameVar *subvar);
	int getSubVarsCount();
	CGameVar *getSubVarByIndex(int idx);
};

struct PreloadItem {
	int preloadId1;
	int preloadId2;
	int sceneId;
	int field_C;
};

class PreloadItems : public Common::Array<PreloadItem>, public CObject {
 public:
	virtual bool load(MfcArchive &file);
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_OBJECTS_H */
