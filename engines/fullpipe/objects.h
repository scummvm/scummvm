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
	int vmt;
};

class CObList {
	int vmt;
	int m_pNodeHead;
	int m_pNodeTail;
	int m_nCount;
	int m_pNodeFree;
	int m_pBlocks;
	int m_nBlockSize;
};

class CObArray {
	CObject obj;
	int m_pData;
	int m_nSize;
	int m_nMaxSize;
	int m_nGrowBy;
};

class CNode {
	int pNext;
	CNode *pPrev;
	void *data;
};

class CPtrList {
	CObject obj;
	CNode *m_pNodeHead;
	int m_pNodeTail;
	int m_nCount;
	int m_pNodeFree;
	int m_pBlocks;
	int m_nBlockSize;
};

class SceneTagList {
	CPtrList list;

 public:
	SceneTagList(CFile &file);
};

class GameProject {
	CObject _obj;
	int _field_4;
	char *_headerFilename;
	SceneTagList *_sceneTagList;
	int _field_10;

 public:
	GameProject(CFile &file);
	~GameProject();
};

class CInteractionController {
	CObject obj;
	CObList interactions;
	int16 field_20;
	char gap_22[2];
	int flag24;
};

class CInputControllerItemArray {
	CObArray objs;
};

class CInputController {
	CObject obj;
	int flag;
	int flags;
	int cursorHandle;
	int hCursor;
	int field_14;
	int cursorId;
	int cursorIndex;
	CInputControllerItemArray cursorsArray;
	int cursorDrawX;
	int cursorDrawY;
	int cursorDrawWidth;
	int cursorDrawHeight;
	int cursorItemPicture;
};

class Sc2Array {
	CObArray objs;
};

union VarValue {
  int floatValue;
  int intValue;
  int stringValue;
};

class Message {
	CObject obj;
	int messageKind;
	int16 parentId;
	char gap_A[2];
	int x;
	int y;
	int field_14;
	int sceneClickX;
	int sceneClickY;
	int field_20;
	int field_24;
	int param28;
	int field_2C;
	int field_30;
	int field_34;
};

class ExCommand {
	Message msg;
	int messageNum;
	int field_3C;
	int flags;
	int parId;
};

class CGameVar {
	CObject obj;
	CGameVar *nextVarObj;
	CGameVar *prevVarObj;
	CGameVar *parentVarObj;
	CGameVar *subVars;
	int field_14;
	char *stringObj;
	VarValue value;
	int varType;
};

class CGameLoader {
 public:
	bool loadFile(const char *fname);
	~CGameLoader();

 private:
	CObject _obj;
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
	int _inventory;
	int _field_7C;
	int _field_80;
	int _field_84;
	int _field_88;
	int _field_8C;
	int _field_90;
	int _field_94;
	int _field_98;
	int _field_9C;
	int _field_A0;
	int _field_A4;
	int _field_A8;
	int _field_AC;
	int _field_B0;
	int _field_B4;
	int _field_B8;
	int _field_BC;
	int _field_C0;
	int _field_C4;
	int _field_C8;
	int _field_CC;
	int _field_D0;
	int _field_D4;
	Sc2Array _sc2array;
	void *_sceneSwitcher;
	void *_preloadCallback;
	void *_readSavegameCallback;
	int16 _field_F8;
	int16 _field_FA;
	CObArray _preloadItems;
	CGameVar *gameVar;
	char *_gameName;
	ExCommand _exCommand;
	int _updateCounter;
	int _preloadId1;
	int _preloadId2;
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_OBJECTS_H */

