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

namespace Fullpipe {

class CObject {
 public:
	virtual bool load(MfcArchive &in) { return true; }
	virtual ~CObject() {}

	bool loadFile(const char *fname);
};

class CObList : public Common::List<CObject>, public CObject {
 public:
	virtual bool load(MfcArchive &file);
};

class MemoryObject {
	//CObject obj;
	int filename;
	int field_8;
	int field_C;
	int field_10;
	char field_14;
	char field_15;
	char field_16;
	char field_17;
	int data;
	int dataSize;
	int flags;
	int libHandle;
};

class CObArray : public Common::Array<CObject>, public CObject {
 public:
	virtual bool load(MfcArchive &file);
};

struct CNode {
	CNode *pNext;
	CNode *pPrev;
	void *data;
};

typedef Common::Array<void *> CPtrList;

class SceneTag : public CObject {
 public:
	int _field_4;
	char *_tag;
	int _scene;
	int16 _sceneId;
	int16 _field_12;

 public:
	SceneTag();
	~SceneTag();

	virtual bool load(MfcArchive &file);
};

class SceneTagList : public Common::List<SceneTag>, public CObject {
 public:
	virtual bool load(MfcArchive &file);
};

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

class CMotionController : public CObject {
	int _field_4;
	int _isEnabled;

 public:
	CMotionController() : _isEnabled(1) {}
	virtual bool load(MfcArchive &file);
};

class CMctlCompoundArray : public Common::Array<CObject>, public CObject {
 public:
	virtual bool load(MfcArchive &file);
};

class CMctlConnectionPointsArray : public Common::Array<CObject>, public CObject {
 public:
	virtual bool load(MfcArchive &file);
};

class CMctlCompoundArrayItem : public CObject {
  int _motionControllerObj;
  int _movGraphReactObj;
  CMctlConnectionPointsArray _connectionPoints;
  int _field_20;
  int _field_24;
  int _field_28;

 public:
	CMctlCompoundArrayItem() : _movGraphReactObj(0), _motionControllerObj(0) {}
};

class CMctlCompound : public CMotionController {
	CMctlCompoundArray _motionControllers;

 public:
	virtual bool load(MfcArchive &file);
};

class Unk2 : public CObject {
  int _items;
  int _count;

 public:
	Unk2() : _items(0), _count(0) {}
};

class CMovGraph : public CMotionController {
  CObList _nodes;
  CObList _links;
  int _field_44;
  int _items;
  int _itemsCount;
  int (*_callback1)(int, int, int);
  Unk2 _unk2;

 public:
	CMovGraph();
	virtual bool load(MfcArchive &file);
};

class ShadowsItemArray : public Common::Array<CObject>, public CObject {
 public:
	virtual bool load(MfcArchive &file);
};

class Background {
	CPtrList list;
	int stringObj;
	int x;
	int y;
	int16 messageQueueId;
	int colorMemoryObj;
	int bigPictureArray1Count;
	int bigPictureArray2Count;
	int bigPictureArray;
};

class Shadows {
	//CObject obj;
	int sceneId;
	int staticAniObjectId;
	int movementId;
	ShadowsItemArray items;
};

class Scene {
	Background bg;
	CPtrList staticANIObjectList1;
	CPtrList staticANIObjectList2;
	CPtrList messageQueueList;
	CPtrList faObjectList;
	Shadows *shadows;
	int soundList;
	int16 sceneId;
	int stringObj;
	int field_BC;
	int libHandle;
};

class Sc2 : public CObject {
	int16 _sceneId;
	int16 _field_2;
	Scene *_scene;
	CMotionController *_motionController;
	int _data1;
	int _count1;
	int _defPicAniInfos;
	int _defPicAniInfosCount;
	int _picAniInfos;
	int _picAniInfosCount;
	int _isLoaded;
	int _entranceData;
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
};

class InventoryPoolItem {
 public:
	int16 _id;
	int16 _pictureObjectNormalId;
	int16 _pictureObjectId1;
	int16 _pictureObjectMouseInsideId;
	int16 _pictureObjectId3;
	int16 _field_A;
	int _field_C;
	int _obj;
	int _flags;
};

typedef Common::Array<InventoryPoolItem> InventoryPoolItems;

class CInventory : public CObject {
	int16 _sceneId;
	int16 _field_6;
	InventoryPoolItems _itemsPool;

 public:
	CInventory() { _sceneId = 0; }
	virtual bool load(MfcArchive &file);
};

struct InventoryItem {
	int16 itemId;
	int16 count;
};

typedef Common::Array<InventoryItem> InventoryItems;

class InventoryIcon {
	int pictureObjectNormal;
	int pictureObjectMouseInside;
	int pictureObject3;
	int x1;
	int y1;
	int x2;
	int y2;
	int16 inventoryItemId;
	int16 field_1E;
	int isSelected;
	int isMouseInside;
};

typedef Common::Array<InventoryIcon> InventoryIcons;

class Picture {
	MemoryObject obj;
	Common::Rect rect;
	int convertedBitmap;
	int x;
	int y;
	int field_44;
	int width;
	int height;
	int bitmap;
	int field_54;
	int memoryObject2;
	int alpha;
	int paletteData;
};

class BigPicture {
	Picture pic;
};

class CInventory2 : public CObject {
	CInventory _inventory;
	InventoryItems _inventoryItems;
	InventoryIcons _inventoryIcons;
	int _selectedId;
	int _field_48;
	int _isInventoryOut;
	int _isLocked;
	int _topOffset;
	Scene *_sceneObj;
	BigPicture *_picture;

 public:
	CInventory2();
	bool loadPartial(MfcArchive &file);
	virtual bool load(MfcArchive &file);
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
	CInventory2 _inventory;
	Sc2Array _sc2array;
	void *_sceneSwitcher;
	void *_preloadCallback;
	void *_readSavegameCallback;
	int16 _field_F8;
	int16 _field_FA;
	PreloadItems _preloadItems;
	CGameVar *_gameVar;
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
