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

#include "ngi/ngi.h"

#include "ngi/objects.h"
#include "ngi/behavior.h"
#include "ngi/ngiarchive.h"
#include "ngi/statics.h"
#include "ngi/messages.h"
#include "ngi/motion.h"
#include "ngi/gameloader.h"

#include "ngi/constants.h"

#include "common/algorithm.h"
#include "graphics/surface.h"

namespace NGI {

Scene *NGIEngine::findScene(int sceneId) {
	return _gameProject->findScene(sceneId);
}

Scene *GameProject::findScene(int sceneId) {
	for (SceneTag &tag : *_sceneTagList) {
		if (tag._sceneId != sceneId)
			continue;
		if (!tag._scene) {
			if (g_nmi->getPlatform() == Common::kPlatformAndroid)
				tag.loadSceneXML(*this);
			else
				tag.loadScene();
		}
		return tag._scene;
	}
	return nullptr;
}

bool SceneTagList::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "SceneTagList::load()");

	int numEntries = file.readUint16LE();

	for (int i = 0; i < numEntries; i++) {
		push_back(SceneTag());
		back().load(file);
	}

	return true;
}

SceneTag::SceneTag() :
	_scene(nullptr),
	_sceneId(0) {}

SceneTag::~SceneTag() {
	delete _scene;
}

bool SceneTag::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "SceneTag::load()");

	_sceneId = file.readUint16LE();

	_tag = file.readPascalString();

	debugC(6, kDebugLoading, "sceneId: %d  tag: %s", _sceneId, _tag.c_str());

	return true;
}

void SceneTag::loadScene() {
	Common::Path archname = genFileName(0, _sceneId, "nl");

	Common::Archive *arch = makeNGIArchive(archname);

	Common::Path fname = genFileName(0, _sceneId, "sc");

	Common::ScopedPtr<Common::SeekableReadStream> file(arch->createReadStreamForMember(fname));

	delete _scene;
	_scene = new Scene();

	MfcArchive archive(file.get());

	_scene->load(archive);

	if (_scene->_shadows)
		_scene->_shadows->init();

	g_nmi->_currArchive = nullptr;
}

Scene::Scene() : _sceneId(0), _field_BC(0) {}

Scene::~Scene() {
	// _faObjlist is not used

	for (uint i = 0; i < _messageQueues.size(); i++)
		delete _messageQueues[i];

	_messageQueues.clear();

	for (uint i = 0; i < _aniObjects.size(); i++)
		delete _aniObjects[i];

	_aniObjects.clear();

	g_nmi->_globalPalette = &g_nmi->_defaultPalette;

	// delete _field_BC;
}

bool Scene::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "Scene::load()");

	Background::load(file);

	_sceneId = file.readUint16LE();

	_sceneName = file.readPascalString();
	debug(0, "scene: <%s> %d", transCyrillic(_sceneName), _sceneId);

	int count = file.readUint16LE();
	debugC(7, kDebugLoading, "scene.ani: %d", count);

	for (int i = 0; i < count; i++) {
		int aniNum = file.readUint16LE();
		Common::Path aniname = genFileName(0, aniNum, "ani");

		Common::SeekableReadStream *f = g_nmi->_currArchive->createReadStreamForMember(aniname);

		AniObject *ani = new AniObject();

		MfcArchive archive(f);

		ani->load(archive);
		ani->_sceneId = _sceneId;

		_aniObjects.push_back(ani);

		delete f;
	}

	count = file.readUint16LE();
	debugC(7, kDebugLoading, "scene.mq: %d", count);

	for (int i = 0; i < count; i++) {
		int qNum = file.readUint16LE();
		Common::Path qname = genFileName(0, qNum, "qu");

		Common::SeekableReadStream *f = g_nmi->_currArchive->createReadStreamForMember(qname);
		MfcArchive archive(f);

		archive.readUint16LE(); // Skip 2 bytes

		MessageQueue *mq = new MessageQueue();

		mq->load(archive);

		_messageQueues.push_back(mq);

		delete f;
	}

	count = file.readUint16LE();
	debugC(7, kDebugLoading, "scene.fa: %d", count);

	for (int i = 0; i < count; i++) {
		// There are no .FA files
		assert(0);
	}

	_libHandle.reset(g_nmi->_currArchive);

	if (_pictureObjects.size() > 0 && !_bgname.empty()) {
		char fname[260];

		Common::strlcpy(fname, _bgname.c_str(), 260);
		Common::strlcpy(strrchr(fname, '.') + 1, "col", 260);

		Common::ScopedPtr<MemoryObject> col(new MemoryObject());
		col->loadFile(fname);
		if (col->getDataSize()) {
			assert(col->getDataSize() == 256 * sizeof(uint32));
			const byte *data = col->getData();
			for (int i = 0; i < 256; ++i) {
				_palette.pal[i] = READ_LE_UINT32(data);
				data += sizeof(uint32);
			}
			_palette.size = 256;
		}
	}

	Common::Path shdname = genFileName(0, _sceneId, "shd");

	Shadows *shd = new Shadows();

	if (shd->loadFile(shdname))
		_shadows.reset(shd);
	else
		delete shd;

	Common::Path slsname = genFileName(0, _sceneId, "sls");

	if (g_nmi->_soundEnabled) {
		_soundList.reset(new SoundList());

		if (g_nmi->_flgSoundList) {
			Common::Path nlname = genFileName(17, _sceneId, "nl");

			_soundList->loadFile(slsname, nlname);
		} else {
			_soundList->loadFile(slsname, nullptr);
		}
	}

	initStaticANIObjects();

	if (file.size() - file.pos() > 0)
		error("Scene::load  (%d bytes left)", (int)(file.size() - file.pos()));

	return true;
}

void Scene::initStaticANIObjects() {
	for (uint i = 0; i < _aniObjects.size(); i++)
		_aniObjects[i]->initMovements();
}

void Scene::init() {
	_x = 0;
	_y = 0;

	g_nmi->_sceneRect.moveTo(0, 0);

	for (uint i = 0; i < _pictureObjects.size(); i++)
		_pictureObjects[i]->clearFlags();

	for (uint i = 0; i < _aniObjects.size(); i++)
		_aniObjects[i]->clearFlags();

	if (_renderableAnis.size() != _aniObjects.size()) {
		_renderableAnis.clear();

		for (uint i = 0; i < _aniObjects.size(); i++)
			_renderableAnis.push_back(_aniObjects[i]);
	}
}

AniObject *Scene::getAniMan() {
	AniObject *aniMan = findAniObjectById(ANI_MAN, -1);

	removeAni(aniMan);

	return aniMan;
}

AniObject *Scene::findAniObjectById(int obj, int a3) {
	const auto &objects = _roamingObjects.empty() ? _aniObjects : _renderableAnis;
	for (AniObject *ani : objects) {
		if (ani->_id == obj && (a3 == -1 || ani->_copyId == a3))
			return ani;
	}
	return nullptr;
}

AniObject *Scene::getStaticANIObject1ByName(const Common::String &name, int a3) {
	for (uint i = 0; i < _aniObjects.size(); i++) {
		if ((_aniObjects[i]->_title == name) && (a3 == -1 || _aniObjects[i]->_copyId == a3))
			return _aniObjects[i];
	}

	return nullptr;
}

void Scene::removeAni(AniObject *obj) {
	for (uint i = 0; i < _aniObjects.size(); i++)
		if (_aniObjects[i] == obj) {
			_aniObjects.remove_at(i);
			break;
		}

	for (uint i = 0; i < _renderableAnis.size(); i++)
		if (_renderableAnis[i] == obj) {
			_renderableAnis.remove_at(i);
			break;
		}
}

void Scene::addAni(AniObject *obj, bool addList2) {
	// WORKAROUND: This is used for making sure that the objects
	// with same priority do not get swapped during drawing
	obj->_cnum = _renderableAnis.size() + 1;

	if (obj->_copyId)
		obj->assignCopyId(&_aniObjects);

	_aniObjects.push_back(obj);

	if (addList2) {
		if (!obj->_copyId)
			obj->clearFlags();

		_renderableAnis.push_back(obj);
	}
}

void Scene::setPictureObjectsFlag4() {
	for (uint i = 0; i < _pictureObjects.size(); i++) {
		_pictureObjects[i]->_flags |= 4;
	}
}

void Scene::stopAllSounds() {
	for (int i = 0; i < _soundList->getCount(); i++)
		_soundList->getSoundByIndex(i).stop();
}

PictureObject *Scene::findPictureObjectById(int objId, int flags) {
	for (uint i = g_nmi->getPlatform() == Common::kPlatformAndroid ? 0 : 1; i < _pictureObjects.size(); i++) {
		if (_pictureObjects[i]->_id == objId && _pictureObjects[i]->_copyId == flags)
			return _pictureObjects[i];
	}

	return nullptr;
}

PictureObject *Scene::getPictureObjectByName(const Common::String &objName, int flags) {
	for (uint i = 0; i < _pictureObjects.size(); i++) {
		if ((_pictureObjects[i]->_title == objName) && (_pictureObjects[i]->_copyId == flags || flags == -1))
			return _pictureObjects[i];
	}

	return nullptr;
}

void Scene::removePicture(PictureObject *obj) {
	for (uint i = 0; i < _pictureObjects.size(); i++) {
		if (_pictureObjects[i] == obj) {
			_pictureObjects.remove_at(i);
			delete obj;

			return;
		}
	}
}

MessageQueue *Scene::findMessageQueue(int messageId) {
	for (uint i = 0; i < _messageQueues.size(); i++)
		if (_messageQueues[i]->_queueId == messageId)
			return _messageQueues[i];

	return _commonQueueScene ? _commonQueueScene->findMessageQueue(messageId) : nullptr;
}

MessageQueue *Scene::getMessageQueueByName(const Common::String &name) {
	for (uint i = 0; i < _messageQueues.size(); i++)
		if (_messageQueues[i]->_title == name)
			return _messageQueues[i];

	return nullptr;
}

void Scene::preloadMovements(GameVar *var) {
	GameVar *preload = var->getSubVarByName("PRELOAD");
	if (!preload)
		return;

	for (GameVar *i = preload->_subVars; i; i = i->_nextVarObj) {
		AniObject *ani = getStaticANIObject1ByName(i->_varName, -1);

		if (ani) {
			GameVar *subVars = i->_subVars;

			if (subVars) {
				for (;subVars; subVars = subVars->_nextVarObj) {
					Movement *mov = ani->getMovementByName(subVars->_varName);

					if (mov)
						mov->load();
				}
			} else {
				ani->loadMovements();
			}
		}
	}
}

void Scene::initObjectCursors(const char *varname) {
	GameVar *cursorsVar = g_nmi->getGameLoaderGameVar()->getSubVarByName(varname)->getSubVarByName("CURSORS");

	if (!cursorsVar || !cursorsVar->_subVars)
		return;

	int maxId = 0;
	int minId = 0xffff;

	for (GameVar *sub = cursorsVar->_subVars; sub; sub = sub->_nextVarObj) {
		GameObject *obj = getPictureObjectByName(sub->_varName, -1);

		if (obj || (obj = getStaticANIObject1ByName(sub->_varName, -1)) != nullptr) {
			if (obj->_id < minId)
				minId = obj->_id;
			if (obj->_id > maxId)
				maxId = obj->_id;
		}
	}

	g_nmi->_minCursorId = minId;
	g_nmi->_maxCursorId = maxId;

	g_nmi->_objectIdCursors.resize(maxId - minId + 1);

	for (GameVar *sub = cursorsVar->_subVars; sub; sub = sub->_nextVarObj) {
		GameObject *obj = getPictureObjectByName(sub->_varName, -1);

		if (!obj)
			obj = getStaticANIObject1ByName(sub->_varName, -1);

		PictureObject *pic = getGameLoaderInventory()->getScene()->getPictureObjectByName(sub->_value.stringValue, -1);

		if (obj && pic)
			g_nmi->_objectIdCursors[obj->_id - minId] = pic->_id;
	}
}

#if 0
bool Scene::compareObjPriority(const void *p1, const void *p2) {
	if (((const GameObject *)p1)->_z > ((const GameObject *)p2)->_z)
		return true;

	if (((const GameObject *)p1)->_z == ((const GameObject *)p2)->_z)
		if (((const GameObject *)p1)->_cnum > ((const GameObject *)p2)->_cnum)
			return true;

	return false;
}

void Scene::sortRenderables(Common::Array<AniObject *> &list, bool skipFirst) {
	if (skipFirst) {
		Common::Array<AniObject *>::iterator s = list.begin();

		++s;

		Common::sort(s, list.end(), Scene::compareObjPriority);
	} else {
		Common::sort(list.begin(), list.end(), Scene::compareObjPriority);
	}
}

void Scene::sortRenderables(Common::Array<PictureObject *> &list, bool skipFirst) {
	if (skipFirst) {
		Common::Array<PictureObject *>::iterator s = list.begin();

		++s;

		Common::sort(s, list.end(), Scene::compareObjPriority);
	} else {
		Common::sort(list.begin(), list.end(), Scene::compareObjPriority);
	}
}
#else
template<typename T>
void Scene::sortRenderables(Common::Array<T *> &list, uint startIndex) {
	if (list.size() > startIndex) {
		int lastIndex = list.size() - 1;
		bool changed;
		do {
			changed = false;
			T *refElement = list[startIndex];
			for (int i = startIndex; i < lastIndex; i++) {
				T *curElement = list[i + 1];
				if (curElement->_z > refElement->_z) {
					// Push refElement down the list
					list.remove_at(i);
					list.insert_at(i + 1, refElement);
					changed = true;
				} else
					refElement = curElement;
			}
			lastIndex--;
		} while (changed);
	}
}
#endif

void Scene::render() {
	debugC(6, kDebugDrawing, ">>>>> Scene::render()");
	updateScrolling();

	// Clean previous stuff
	g_nmi->_backgroundSurface.fillRect(Common::Rect(0, 0, 800, 600), 0);

	drawContent(60000, 0, true);

	sortRenderables(_renderableAnis);

	for (uint i = 0; i < _renderableAnis.size(); i++) {
		_renderableAnis[i]->draw2();
	}

	int priority = -1;
	for (uint i = 0; i < _renderableAnis.size(); i++) {
		drawContent(_renderableAnis[i]->_z, priority, false);

		_renderableAnis[i]->render();

		priority = _renderableAnis[i]->_z;
	}

	drawContent(-1, priority, false);
}

void Scene::updateScrolling() {
	if (_messageQueueId && !_x && !_y) {
		MessageQueue *mq = g_nmi->_globalMessageQueueList->getMessageQueueById(_messageQueueId);

		if (mq)
			mq->update();

		_messageQueueId = 0;
	}

	// Might happen very early in the game
	if (!_pictureObjects.size())
		return;

	if (_x || _y) {
		int offsetX = 0;
		int offsetY = 0;

		if (_x < 0) {
			if (!g_nmi->_sceneRect.left && !(_pictureObjects[0]->_flags & 2))
				_x = 0;

			if (_x <= -g_nmi->_scrollSpeed) {
				offsetX = -g_nmi->_scrollSpeed;
				_x += g_nmi->_scrollSpeed;
			}
		} else if (_x >= g_nmi->_scrollSpeed) {
			offsetX = g_nmi->_scrollSpeed;
			_x -= g_nmi->_scrollSpeed;
		} else {
			_x = 0;
		}

		if (_y > 0) {
			offsetY = g_nmi->_scrollSpeed;
			_y -= g_nmi->_scrollSpeed;
		}

		if (_y < 0) {
			offsetY -= g_nmi->_scrollSpeed;
			_y += g_nmi->_scrollSpeed;
		}

		g_nmi->_sceneRect.translate(offsetX, offsetY);
	}

	updateScrolling2();
}

void Scene::updateScrolling2() {
	if (_pictureObjects.size()) {
		int offsetY = 0;
		int offsetX = 0;

		const Dims dims = _pictureObjects[0]->getFullSize();
		const int flags = _pictureObjects[0]->_flags;

		if (g_nmi->_sceneRect.left < 0 && !(flags & 2))
			offsetX = -g_nmi->_sceneRect.left;

		if (g_nmi->_sceneRect.top < 0 && !(flags & 0x20))
			offsetY = -g_nmi->_sceneRect.top;

		if (g_nmi->_sceneRect.right > dims.x - 1 && g_nmi->_sceneRect.left > 0 && !(flags & 2))
			offsetX = dims.x - g_nmi->_sceneRect.right - 1;

		if (g_nmi->_sceneRect.bottom > dims.y - 1 && g_nmi->_sceneRect.top > 0 && !(flags & 0x20))
			offsetY = dims.y - g_nmi->_sceneRect.bottom - 1;

		g_nmi->_sceneRect.translate(offsetX, offsetY);
	}
}

AniObject *Scene::findAniObjectAtPos(int x, int y) {
	AniObject *res = nullptr;

	for (uint i = 0; i < _aniObjects.size(); i++) {
		AniObject *p = _aniObjects[i];

		if ((p->_objectFlags & 0x100) && (p->_flags & 4) &&
				p->hit(x, y) &&
				(!res || res->_z > p->_z))
			res = p;
	}

	return res;
}

PictureObject *Scene::findPictureObjectAtPos(int x, int y) {
	PictureObject *res = nullptr;

	for (uint i = 0; i < _pictureObjects.size(); i++) {
		PictureObject *p = _pictureObjects[i];
		if ((p->_objectFlags & 0x100) && (p->_flags & 4) &&
				p->hit(x, y) &&
				(!res || res->_z >= p->_z))
			res = p;
	}

	return res;
}

int Scene::getPictureObjectIdAtPos(int x, int y) {
	PictureObject *resp = nullptr;
	int res = 0;

	for (uint i = 0; i < _pictureObjects.size(); i++) {
		PictureObject *p = _pictureObjects[i];
		if ((p->_objectFlags & 0x100) && (p->_flags & 4) &&
				p->hit(x, y) &&
				(!res || resp->_z >= p->_z)) {
			resp = p;
			res = p->_id;
		}
	}

	return res;
}

bool Scene::update(int counterdiff) {
	debugC(6, kDebugDrawing, "Scene::update(%d)", counterdiff);

	bool changed = false;
	for (uint i = 0; i < _renderableAnis.size(); i++)
		changed |= _renderableAnis[i]->update(counterdiff);
	return changed;
}

void Scene::drawContent(int minPri, int maxPri, bool drawBg) {
	if (!_pictureObjects.size() && !_bigPictureXDim)
		return;

	if (_palette.size) {
		g_nmi->_globalPalette = &_palette;
	}

	debugC(1, kDebugDrawing, "Scene::drawContent(>%d, <%d, %d)", minPri, maxPri, drawBg);

#if 0
	if (_pictureObjects.size() > 2) { // We need to z-sort them
		sortRenderables(_pictureObjects, true);
	}
#else
	sortRenderables(_pictureObjects, 1);
#endif

	if (minPri == -1 && _pictureObjects.size())
		minPri = _pictureObjects.back()->_z - 1;

	if (maxPri == -1)
		maxPri = 60000;

	debugC(1, kDebugDrawing, "-> Scene::drawContent(>%d, <%d, %d)", minPri, maxPri, drawBg);

	Dims dims;

	debugC(1, kDebugDrawing, "_bigPict: %d objlist: %d", _bigPictureXDim, _pictureObjects.size());

	if (drawBg && _bigPictureXDim && _pictureObjects.size()) {

		dims = _bigPictureArray[0]->getDimensions();

		int width = dims.x;
		int height = dims.y;

		debugC(8, kDebugDrawing, "w: %d h:%d", width, height);

		dims = _pictureObjects[0]->getFullSize();

		debugC(8, kDebugDrawing, "w2: %d h2:%d", dims.x, dims.y);

		int bgStX = g_nmi->_sceneRect.left % dims.x;

		if (bgStX < 0)
			bgStX += dims.x;

		int bgNumX = bgStX / width;
		int bgOffsetX = bgStX % width;

		int bgStY = g_nmi->_sceneRect.top % dims.y;

		if (bgStY < 0)
			bgStY += dims.y;

		int bgNumY = bgStY / height;
		int bgOffsetY = bgStY % height;

		int bgPosX = g_nmi->_sceneRect.left - bgOffsetX;

		if (bgPosX < g_nmi->_sceneRect.right - 1) {
			for (;;) {
				uint v25 = bgNumY;
				for (int y = g_nmi->_sceneRect.top - bgOffsetY; y < g_nmi->_sceneRect.bottom - 1;) {
					BigPicture *v27 = getBigPicture(bgNumX, v25);
					v27->render(bgPosX, y, 0, 0);
					y += v27->getDimensions().y;
					v25++;

					if (v25 >= _bigPictureYDim) {
						if (!(_pictureObjects[0]->_flags & 0x20))
							break;
						v25 = 0;
					}
				}
				dims = getBigPicture(bgNumX, 0)->getDimensions();
				int oldx = dims.x + bgPosX;
				bgPosX += dims.x;
				bgNumX++;

				if (bgNumX >= (int)_bigPictureXDim) {
					if (!(_pictureObjects[0]->_flags & 0x2))
						break;
					bgNumX = 0;
				}
				if (oldx >= g_nmi->_sceneRect.right - 1)
					break;
			}
		}
	}


	for (uint i = 1; i < _pictureObjects.size(); i++) {
		PictureObject *obj = _pictureObjects[i];

		if (obj->_z < minPri || obj->_z >= maxPri)
			continue;

		int objX = obj->_x;
		int objY = obj->_y;

		debugC(8, kDebugDrawing, "obj: %d %d", objX, objY);

		dims = obj->getFullSize();

		int width = dims.x;
		int height = dims.y;

		if (obj->_flags & 8) {
			while (objX > g_nmi->_sceneRect.right) {
				objX -= width;
				obj->setPos(objX, objY);
			}
			for (int j = width + objX; width + objX < g_nmi->_sceneRect.left; j = width + objX) {
				objX = j;
				obj->setPos(j, objY);
			}
		}

		if (obj->_flags & 0x10) {
			while (objY > g_nmi->_sceneRect.bottom) {
				objY -= height;
				obj->setPos(objX, objY);
			}
			for (int j = objY + height; objY + height < g_nmi->_sceneRect.top; j = objY + height) {
				objY = j;
				obj->setPos(objX, j);
			}
		}
		if (obj->_flags & 4)
			obj->render();

		if (obj->_flags & 2) {
			if (objX > g_nmi->_sceneRect.left) {
				obj->setPos(objX - width, objY);
				obj->render();
				obj->setPos(objX, objY);
			}
			if (width + objX < g_nmi->_sceneRect.right) {
				obj->setPos(width + objX, objY);
				obj->render();
				obj->setPos(objX, objY);
			}
		}

		if (obj->_flags & 0x20) {
			if (objY > g_nmi->_sceneRect.top) {
				obj->setPos(objX, objY - height);
				obj->render();
				obj->setPos(objX, objY);
			}
			if (height + objY < g_nmi->_sceneRect.bottom) {
				obj->setPos(objX, height + objY);
				obj->render();
				obj->setPos(objX, objY);
			}
		}
	}
}

} // End of namespace NGI
