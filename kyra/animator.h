/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef KYRAANIMATOR_H
#define KYRAANIMATOR_H

namespace Kyra {
class KyraEngine;
class Screen;

struct AnimObject {
	uint8 index;
	uint32 active;
	uint32 refreshFlag;
	uint32 bkgdChangeFlag;
	bool disable;
	uint32 flags;
	int16 drawY;
	uint8 *sceneAnimPtr;
	int16 animFrameNumber;
	uint8 *background;
	uint16 rectSize;
	int16 x1, y1;
	int16 x2, y2;
	uint16 width;
	uint16 height;
	uint16 width2;
	uint16 height2;
	AnimObject *nextAnimObject;
};

class ScreenAnimator {
public:
	ScreenAnimator(KyraEngine *vm, OSystem *system);
	virtual ~ScreenAnimator();

	operator bool() const { return _initOk; }

	void init(int actors, int items, int sprites);
	void close();

	AnimObject *objects() { return _screenObjects; }
	AnimObject *actors() { return _actors; }
	AnimObject *items() { return _items; }
	AnimObject *sprites() { return _sprites; }

	void initAnimStateList();
	void preserveAllBackgrounds();
	void flagAllObjectsForBkgdChange();
	void flagAllObjectsForRefresh();
	void restoreAllObjectBackgrounds();
	void preserveAnyChangedBackgrounds();
	virtual void prepDrawAllObjects();
	void copyChangedObjectsForward(int refreshFlag);

	void updateAllObjectShapes();
	void animRemoveGameItem(int index);
	void animAddGameItem(int index, uint16 sceneId);
	void animAddNPC(int character);
	void animRefreshNPC(int character);

	void clearQueue() { _objectQueue = 0; }
	void addObjectToQueue(AnimObject *object);
	void refreshObject(AnimObject *object);
	
	void makeBrandonFaceMouse();
	void setBrandonAnimSeqSize(int width, int height);
	void resetBrandonAnimSeqSize();
	void setCharacterDefaultFrame(int character);
	void setCharactersHeight();

	int16 fetchAnimWidth(const uint8 *shape, int16 mult);
	int16 fetchAnimHeight(const uint8 *shape, int16 mult);

	int _noDrawShapesFlag;
	bool _updateScreen;
	uint16 _brandonDrawFrame;
	int _brandonScaleX;
	int _brandonScaleY;

protected:
	KyraEngine *_vm;
	Screen *_screen;
	OSystem *_system;
	bool _initOk;

	AnimObject *_screenObjects;

	AnimObject *_actors;
	AnimObject *_items;
	AnimObject *_sprites;

	AnimObject *objectRemoveQueue(AnimObject *queue, AnimObject *rem);
	AnimObject *objectAddHead(AnimObject *queue, AnimObject *head);
	AnimObject *objectQueue(AnimObject *queue, AnimObject *add);

	void preserveOrRestoreBackground(AnimObject *obj, bool restore);

	AnimObject *_objectQueue;

	int _brandonAnimSeqSizeWidth;
	int _brandonAnimSeqSizeHeight;
	
};
} // end of namespace Kyra

#endif
