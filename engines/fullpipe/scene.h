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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef FULLPIPE_SCENE_H
#define FULLPIPE_SCENE_H

#include "fullpipe/gfx.h"

namespace Fullpipe {

class MessageQueue;

class Scene : public Background {
 public:
	Common::Array<StaticANIObject *> _staticANIObjectList1;
	Common::Array<StaticANIObject *> _staticANIObjectList2;
	Common::Array<MessageQueue *> _messageQueueList;
	// PtrList _faObjectList; // not used
	Shadows *_shadows;
	SoundList *_soundList;
	int16 _sceneId;
	char *_sceneName;
	int _field_BC;
	NGIArchive *_libHandle;

  public:
	Scene();
	virtual ~Scene();

	virtual bool load(MfcArchive &file);

	void initStaticANIObjects();
	void init();
	void draw();
	void drawContent(int minPri, int maxPri, bool drawBG);
	void updateScrolling();
	void updateScrolling2();

	void update(int counterdiff);

	StaticANIObject *getAniMan();
	StaticANIObject *getStaticANIObject1ById(int obj, int a3);
	StaticANIObject *getStaticANIObject1ByName(char *name, int a3);
	MessageQueue *getMessageQueueById(int messageId);
	MessageQueue *getMessageQueueByName(char *name);

	void deleteStaticANIObject(StaticANIObject *obj);
	void addStaticANIObject(StaticANIObject *obj, bool addList2);

	void setPictureObjectsFlag4();
	PictureObject *getPictureObjectById(int objId, int flags);
	PictureObject *getPictureObjectByName(const char *name, int keyCode);
	void deletePictureObject(PictureObject *obj);
	void preloadMovements(GameVar *var);

	StaticANIObject *getStaticANIObjectAtPos(int x, int y);
	PictureObject *getPictureObjectAtPos(int x, int y);
	int getPictureObjectIdAtPos(int x, int y);

	void initObjectCursors(const char *name);

	void stopAllSounds();

  private:
#if 0
	static bool compareObjPriority(const void *p1, const void *p2);
	void objectList_sortByPriority(Common::Array<StaticANIObject *> &list, bool skipFirst = false);
	void objectList_sortByPriority(Common::Array<PictureObject *> &list, bool skipFirst = false);
#else
	template<typename T>
	void objectList_sortByPriority(Common::Array<T*> &list, uint startIndex = 0);
#endif
};

class SceneTag : public CObject {
 public:
	CObject *_field_4;
	char *_tag;
	Scene *_scene;
	int16 _sceneId;

 public:
	SceneTag();
	~SceneTag();

	virtual bool load(MfcArchive &file);
	void loadScene();
};

class SceneTagList : public Common::List<SceneTag>, public CObject {
 public:
	virtual bool load(MfcArchive &file);
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_SCENE_H */
