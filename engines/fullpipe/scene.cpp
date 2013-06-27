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

#include "fullpipe/fullpipe.h"

#include "fullpipe/objects.h"
#include "fullpipe/ngiarchive.h"
#include "fullpipe/statics.h"

namespace Fullpipe {

Scene *FullpipeEngine::accessScene(int sceneId) {
	SceneTag *t = 0;

	for (SceneTagList::iterator s = _gameProject->_sceneTagList->begin(); s != _gameProject->_sceneTagList->end(); ++s) {
		if (s->_sceneId == sceneId) {
			t = &(*s);
			break;
		}
	}

	if (!t)
		return 0;

	if (!t->_scene) {
		t->loadScene();
	}

	return t->_scene;
}

bool SceneTagList::load(MfcArchive &file) {
	int numEntries = file.readUint16LE();

	for (int i = 0; i < numEntries; i++) {
		SceneTag *t = new SceneTag();
		t->load(file);
		push_back(*t);
	}

	return true;
}

SceneTag::SceneTag() {
	_field_4 = 0;
	_scene = 0;
}

bool SceneTag::load(MfcArchive &file) {
	_field_4 = 0;
	_scene = 0;

	_sceneId = file.readUint16LE();

	_tag = file.readPascalString();

	debug(6, "sceneId: %d  tag: %s", _sceneId, _tag);

	return true;
}

SceneTag::~SceneTag() {
	free(_tag);
}

void SceneTag::loadScene() {
	char *archname = genFileName(0, _sceneId, "nl");

	Common::Archive *arch = makeNGIArchive(archname);

	char *fname = genFileName(0, _sceneId, "sc");

	Common::SeekableReadStream *file = arch->createReadStreamForMember(fname);

	_scene = new Scene();

	MfcArchive archive(file);

	_scene->load(archive);

	delete file;

	g_fullpipe->_currArchive = 0;

	free(fname);
	free(archname);
}

Scene::Scene() {
	_sceneId = 0;
	_field_BC = 0;
	_shadows = 0;
	_soundList = 0;
}

bool Scene::load(MfcArchive &file) {
	Background::load(file);

	_sceneId = file.readUint16LE();
	
	_scstringObj = file.readPascalString();
	debug(0, "scene: <%s>", transCyrillic((byte *)_scstringObj));

	int count = file.readUint16LE();
	debug(7, "scene.ani: %d", count);

	for (int i = 0; i < count; i++) {
		int aniNum = file.readUint16LE();
		char *aniname = genFileName(0, aniNum, "ani");

		Common::SeekableReadStream *f = g_fullpipe->_currArchive->createReadStreamForMember(aniname);

		StaticANIObject *ani = new StaticANIObject();

		MfcArchive archive(f);

		ani->load(archive);
		ani->_sceneId = _sceneId;

		_staticANIObjectList1.push_back(ani);

		delete f;
		free(aniname);
	}

	count = file.readUint16LE();
	debug(7, "scene.mq: %d", count);

	for (int i = 0; i < count; i++) {
		int qNum = file.readUint16LE();
		char *qname = genFileName(0, qNum, "qu");

		Common::SeekableReadStream *f = g_fullpipe->_currArchive->createReadStreamForMember(qname);
		MfcArchive archive(f);

		archive.readUint16LE(); // Skip 2 bytes

		MessageQueue *mq = new MessageQueue();

		mq->load(archive);

		_messageQueueList.push_back(mq);

		delete f;
		free(qname);
	}

	count = file.readUint16LE();
	debug(7, "scene.fa: %d", count);

	for (int i = 0; i < count; i++) {
		// There are no .FA files
		assert(0);
	}

	_libHandle = g_fullpipe->_currArchive;

	if (_picObjList.size() > 0 && _stringObj && strlen(_stringObj) > 1) {
		char fname[260];

		strcpy(fname, _stringObj);
		strcpy(strrchr(fname, '.') + 1, "col");

		MemoryObject *col =  new MemoryObject();
		col->loadFile(fname);

		_colorMemoryObj = col;
	  
	}

	char *shdname = genFileName(0, _sceneId, "shd");

	Shadows *shd = new Shadows();

	if (shd->loadFile(shdname))
		_shadows = shd;

	free(shdname);

	char *slsname = genFileName(0, _sceneId, "sls");

	if (g_fullpipe->_soundEnabled) {
		_soundList = new SoundList();

		if (g_fullpipe->_flgSoundList) {
			char *nlname = genFileName(17, _sceneId, "nl");
		  
			_soundList->loadFile(slsname, nlname);

			free(nlname);
		} else {
			_soundList->loadFile(slsname, 0);
		}
	}

	free(slsname);

	initStaticANIObjects();

	warning("STUB: Scene::load  (%d bytes left)", file.size() - file.pos());

	return true;
}

void Scene::initStaticANIObjects() {
	warning("STUB: Scene::initStaticANIObjects");
}

} // End of namespace Fullpipe
