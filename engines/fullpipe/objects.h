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
};

class GameProject {
	CObject obj;
	int field_4;
	char *headerFilename;
	SceneTagList *sceneTagList;
	void *field_10;
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
	bool loadFile(char *fname);

 private:
	CObject obj;
	GameProject *gameProject;
	CInteractionController *interactionController;
	int field_C;
	int field_10;
	int field_14;
	int field_18;
	int field_1C;
	int field_20;
	int field_24;
	int field_28;
	int field_2C;
	CInputController inputController;
	int inventory;
	int field_7C;
	int field_80;
	int field_84;
	int field_88;
	int field_8C;
	int field_90;
	int field_94;
	int field_98;
	int field_9C;
	int field_A0;
	int field_A4;
	int field_A8;
	int field_AC;
	int field_B0;
	int field_B4;
	int field_B8;
	int field_BC;
	int field_C0;
	int field_C4;
	int field_C8;
	int field_CC;
	int field_D0;
	int field_D4;
	Sc2Array sc2array;
	void *sceneSwitcher;
	void *preloadCallback;
	void *readSavegameCallback;
	int16 field_F8;
	int16 field_FA;
	CObArray preloadItems;
	CGameVar *gameVar;
	char *gameName;
	ExCommand exCommand;
	int updateCounter;
	int preloadId1;
	int preloadId2;
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_OBJECTS_H */

