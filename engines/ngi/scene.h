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

#ifndef NGI_SCENE_H
#define NGI_SCENE_H

#include "common/ptr.h"
#include "ngi/gfx.h"

namespace NGI {

class MessageQueue;

class Scene : public Background {
 public:
	/** list items are owned */
	Common::Array<StaticANIObject *> _staticANIObjectList1;

	Common::Array<StaticANIObject *> _staticANIObjectList2;
	Common::Array<MessageQueue *> _messageQueueList;
	// PtrList _faObjectList; // not used
	Common::ScopedPtr<Shadows> _shadows;
	Common::ScopedPtr<SoundList> _soundList;
	int16 _sceneId;
	Common::String _sceneName;
	int _field_BC;
	Common::ScopedPtr<NGIArchive> _libHandle;

  public:
	Scene();
	~Scene() override;

	bool load(MfcArchive &file) override;

	void initStaticANIObjects();
	void init();
	void draw();
	void drawContent(int minPri, int maxPri, bool drawBG);
	void updateScrolling();
	void updateScrolling2();

	void update(int counterdiff);

	StaticANIObject *getAniMan();
	StaticANIObject *getStaticANIObject1ById(int obj, int a3);
	StaticANIObject *getStaticANIObject1ByName(const Common::String &name, int a3);
	MessageQueue *getMessageQueueById(int messageId);
	MessageQueue *getMessageQueueByName(const Common::String &name);

	void deleteStaticANIObject(StaticANIObject *obj);
	/** takes ownership of the passed obj */
	void addStaticANIObject(StaticANIObject *obj, bool addList2);

	void setPictureObjectsFlag4();
	PictureObject *getPictureObjectById(int objId, int flags);
	PictureObject *getPictureObjectByName(const Common::String &name, int keyCode);
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
	Common::String _tag;
	/** owned, but cannot use ScopedPtr because this object must be copyable */
	Scene *_scene;
	int16 _sceneId;

 public:
	SceneTag();
	~SceneTag() override;

	bool load(MfcArchive &file) override;
	void loadScene();
};

class SceneTagList : public Common::List<SceneTag>, public CObject {
 public:
	bool load(MfcArchive &file) override;
};

} // End of namespace NGI

#endif /* NGI_SCENE_H */
