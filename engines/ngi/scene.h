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

#ifndef NGI_SCENE_H
#define NGI_SCENE_H

#include "common/ptr.h"
#include "ngi/objects.h"
#include "ngi/gfx.h"
#include "ngi/interaction.h"

namespace NGI {

class MessageQueue;
class GameProject;
class BehaviorController;
class MctlCompound;

class Scene : public Background {
 public:
	/** list items are owned */
	Common::Array<AniObject *> _aniObjects;

	// Includes non-owning references to roaming XML actors from other scenes.
	Common::Array<AniObject *> _renderableAnis;
	PicAniInfoList _roamingObjects;
	Common::Array<Entrance> _entrances;
	int16 _heroId = 0;
	Common::Array<MessageQueue *> _messageQueues;
	// XML queue templates are cloned when run and can be shared between scenes.
	Scene *_commonQueueScene = nullptr;
	// PtrList _faObjectList; // not used
	Common::ScopedPtr<Shadows> _shadows;
	Common::ScopedPtr<SoundList> _soundList;
	Common::ScopedPtr<BehaviorController> _behaviorController;
	Common::ScopedPtr<MctlCompound> _motionController;
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
	void render();
	void drawAndroid(Graphics::ManagedSurface &target) const;
	void drawContent(int minPri, int maxPri, bool drawBG);
	void updateScrolling();
	void updateScrolling2();

	/** Returns true if the scene may need redrawing. */
	bool update(int counterdiff);

	AniObject *getAniMan();
	AniObject *findAniObjectById(int obj, int a3);
	AniObject *getStaticANIObject1ByName(const Common::String &name, int a3);
	MessageQueue *findMessageQueue(int messageId);
	MessageQueue *getMessageQueueByName(const Common::String &name);

	void removeAni(AniObject *obj);
	/** takes ownership of the passed obj */
	void addAni(AniObject *obj, bool addList2);

	void setPictureObjectsFlag4();
	PictureObject *findPictureObjectById(int objId, int flags);
	PictureObject *getPictureObjectByName(const Common::String &name, int keyCode);
	void removePicture(PictureObject *obj);
	void preloadMovements(GameVar *var);

	AniObject *findAniObjectAtPos(int x, int y);
	PictureObject *findPictureObjectAtPos(int x, int y);
	int getPictureObjectIdAtPos(int x, int y);

	void initObjectCursors(const char *name);

	void stopAllSounds();

  private:
#if 0
	static bool compareObjPriority(const void *p1, const void *p2);
	void sortRenderables(Common::Array<AniObject *> &list, bool skipFirst = false);
	void sortRenderables(Common::Array<PictureObject *> &list, bool skipFirst = false);
#else
	template<typename T>
	void sortRenderables(Common::Array<T*> &list, uint startIndex = 0);
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
	void loadSceneXML(const GameProject &project);
};

class SceneTagList : public Common::List<SceneTag>, public CObject {
 public:
	bool load(MfcArchive &file) override;
};

} // End of namespace NGI

#endif /* NGI_SCENE_H */
