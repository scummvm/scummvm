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
#include "fullpipe/inventory.h"
#include "fullpipe/gfx.h"
#include "fullpipe/scene.h"

namespace Fullpipe {

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

class MessageQueue : public CObject {
	int _id;
	int _flags;
	char *_stringObj;
	int16 _dataId;
	int16 _field_12;
	int _field_14;
	CPtrList _exCommands;
	int _counter;
	int _field_38;
	int _isFinished;
	int _parId;
	int _flag1;

 public:
	MessageQueue();
	virtual bool load(MfcArchive &file);
};

class CInteraction : public CObject {
	int16 _objectId1;
	int16 _objectId2;
	int16 _objectId3;
	int16 _staticsId1;
	int16 _staticsId2;
	int16 _field_E;
	int _objectState1;
	int _objectState2;
	int _xOffs;
	int _yOffs;
	MessageQueue *_messageQueue;
	int _sceneId;
	int _field_28;
	int _flags;
	char *_stringObj;

 public:
	CInteraction();
	virtual bool load(MfcArchive &file);
};

class CInteractionController : public CObject {
	CObList _interactions;
	int16 _field_20;
	int _flag24;

 public:
	CInteractionController() : _field_20(0), _flag24(1) {}

	virtual bool load(MfcArchive &file);
};

class CInputControllerItemArray {
	CObArray objs;
};

class CInputController {
	//CObject obj;
	int _flag;
	int _flags;
	int _cursorHandle;
	int _hCursor;
	int _field_14;
	int _cursorId;
	int _cursorIndex;
	CInputControllerItemArray _cursorsArray;
	int _cursorDrawX;
	int _cursorDrawY;
	int _cursorDrawWidth;
	int _cursorDrawHeight;
	int _cursorItemPicture;

 public:
	CInputController();
};

struct PicAniInfo {
	int32 type;
	int16 objectId;
	int16 field_6;
	int32 field_8;
	int16 field_C;
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

struct EntranceInfo {
	int32 sceneId;
	int32 field_4;
	int32 messageQueueId;
	byte gap_C[292]; // FIXME
	int32 field_130;

	bool load(MfcArchive &file);
};

class CMotionController;

class Sc2 : public CObject {
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

class Sc2Array : public Common::Array<Sc2> {
};

union VarValue {
	float floatValue;
	int intValue;
	char *stringValue;
};

class Message {
 public:
	int _messageKind;
	int16 _parentId;
	int _x;
	int _y;
	int _field_14;
	int _sceneClickX;
	int _sceneClickY;
	int _field_20;
	int _field_24;
	int _param28;
	int _field_2C;
	int _field_30;
	int _field_34;

 public:
	Message();
};

class ExCommand : public CObject {
	Message _msg;
	int _messageNum;
	int _field_3C;
	int _flags;
	int _parId;

 public:
	ExCommand();

	virtual bool load(MfcArchive &file);
};

class CGameVar : public CObject {
 public:
	CGameVar *_nextVarObj;
	CGameVar *_prevVarObj;
	CGameVar *_parentVarObj;
	CGameVar *_subVars;
	CGameVar *_field_14;
	char *_stringObj;
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

class CGameLoader : public CObject {
 public:
	CGameLoader();
	virtual ~CGameLoader();

	virtual bool load(MfcArchive &file);

	CGameVar *_gameVar;
	CInventory2 _inventory;

 private:
	GameProject *_gameProject;
	CInteractionController *_interactionController;
	int _field_C;
	int _field_10;
	int _field_14;
	int _field_18;
	int _field_1C;
	int _field_20;
	int _field_24;
	int _field_28;
	int _field_2C;
	CInputController _inputController;
	Sc2Array _sc2array;
	void *_sceneSwitcher;
	void *_preloadCallback;
	void *_readSavegameCallback;
	int16 _field_F8;
	int16 _field_FA;
	PreloadItems _preloadItems;
	char *_gameName;
	ExCommand _exCommand;
	int _updateCounter;
	int _preloadId1;
	int _preloadId2;
};

class CObjstateCommand : public CObject {
	ExCommand _cmd;
	char *_stringObj;
	int _value;

 public:
	CObjstateCommand();
	virtual bool load(MfcArchive &file);
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_OBJECTS_H */
