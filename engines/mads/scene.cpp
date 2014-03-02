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

#include "common/scummsys.h"
#include "mads/scene.h"
#include "mads/compression.h"
#include "mads/mads.h"
#include "mads/nebular/nebular_scenes.h"

namespace MADS {

Scene::Scene(MADSEngine *vm): _vm(vm), _spriteSlots(vm) {
	_priorSceneId = 0;
	_nextSceneId = 0;
	_currentSceneId = 0;
	_vocabBuffer = nullptr;
	_sceneLogic = nullptr;
	_sceneInfo = nullptr;
	_animFlag = false;
	_animVal1 = 0;
	_depthStyle = 0;

	_verbList.push_back(VerbInit(VERB_LOOK, 2, 0));
	_verbList.push_back(VerbInit(VERB_TAKE, 2, 0));
	_verbList.push_back(VerbInit(VERB_PUSH, 2, 0));
	_verbList.push_back(VerbInit(VERB_OPEN, 2, 0));
	_verbList.push_back(VerbInit(VERB_PUT, 1, -1));
	_verbList.push_back(VerbInit(VERB_TALKTO, 2, 0));
	_verbList.push_back(VerbInit(VERB_GIVE, 1, 2));
	_verbList.push_back(VerbInit(VERB_PULL, 2, 0));
	_verbList.push_back(VerbInit(VERB_CLOSE, 2, 0));
	_verbList.push_back(VerbInit(VERB_THROW, 1, 2));
}

Scene::~Scene() {
	delete[] _vocabBuffer;
	delete _sceneLogic;
	delete _sceneInfo;
}

void Scene::clearDynamicHotspots() {
	_dynamicHotspots.clear();
	_dynamicHotspotsChanged = false;
}

void Scene::clearVocab() {
	freeVocab();
	_activeVocabs.clear();
}

void Scene::freeVocab() {
	delete[] _vocabBuffer;
	_vocabBuffer = nullptr;
}

void Scene::addActiveVocab(int vocabId) {
	if (activeVocabIndexOf(vocabId) == -1) {
		assert(_activeVocabs.size() < 200);
		_activeVocabs.push_back(vocabId);
	}
}

int Scene::activeVocabIndexOf(int vocabId) {
	for (uint i = 0; i < _activeVocabs.size(); ++i) {
		if (_activeVocabs[i] == vocabId)
			return i;
	}

	return -1;
}

void Scene::clearSequenceList() {
	_sequences.clear();
}

void Scene::clearMessageList() {
	_messages.clear();
	_talkFont = "*FONTCONV.FF";
	_textSpacing  = -1;
}

void Scene::loadSceneLogic() {
	delete _sceneLogic;

	switch (_vm->getGameID()) {
	case GType_RexNebular:
		_sceneLogic = Nebular::SceneFactory::createScene(this);
		break;
	default:
		error("Unknown game");
	}
}

void Scene::loadScene(int sceneId, const Common::String &prefix, bool palFlag) {
	// Store the previously active scene number and set the new one
	_priorSceneId = _currentSceneId;
	_currentSceneId = sceneId;

	_v1 = 0;
	if (palFlag)
		_vm->_palette->resetGamePalette(18, 10);

	_spriteSlots.clear(false);
	_sequences.clear();
	_messages.clear();

	// TODO: palletteUsage reset?  setPalette(_nullPalette);
	_sceneInfo = SceneInfo::init(_vm);
	_sceneInfo->load(_currentSceneId, _v1, Common::String(), _vm->_game->_v2 ? 17 : 16,
		_depthSurface, _backgroundSurface);

	// Initialise palette animation for the scene
	initPaletteAnimation(_sceneInfo->_palAnimData, false);

	// Copy over nodes
	_nodes.clear();
	for (uint i = 0; i < _sceneInfo->_nodes.size(); ++i)
		_nodes.push_back(_sceneInfo->_nodes[i]);

	// Load hotspots
	loadHotspots();

	// Load vocab
	loadVocab();

	// Load palette usage
	_vm->_palette->_paletteUsage.load(1, 0xF);

	// Load interface
	int flags = _vm->_game->_v2 ? 0x4101 : 0x4100;
	if (!_vm->_textWindowStill)
		flags |= 0x200;
	_animation = Animation::init(_vm, this);
	
}

void Scene::loadHotspots() {
	File f(Resources::formatName(RESPREFIX_RM, _currentSceneId, ".HH"));
	MadsPack madsPack(&f);

	Common::SeekableReadStream *stream = madsPack.getItemStream(0);
	int count = stream->readUint16LE();
	delete stream;

	stream = madsPack.getItemStream(1);
	_hotspots.clear();
	for (int i = 0; i < count; ++i)
		_hotspots.push_back(Hotspot(*stream));

	delete stream;
	f.close();
}

void Scene::loadVocab() {
	// Add all the verbs to the active vocab list
	for (uint i = 0; i < _verbList.size(); ++i)
		addActiveVocab(_verbList[i]._id);

	// Load the vocabs for any object descriptions and custom actions
	for (uint objIndex = 0; objIndex < _vm->_game->_objects.size(); ++objIndex) {
		InventoryObject &io = _vm->_game->_objects[objIndex];
		addActiveVocab(io._descId);

		for (int vocabIndex = 0; vocabIndex <io._vocabCount; ++vocabIndex) {
			addActiveVocab(io._vocabList[vocabIndex]._vocabId);
		}
	}

	// Load scene hotspot list vocabs and verbs
	for (uint i = 0; i < _hotspots.size(); ++i) {
		addActiveVocab(_hotspots[i]._vocabId);
		if (_hotspots[i]._verbId)
			addActiveVocab(_hotspots[i]._verbId);
	}

	loadVocabStrings();
}

void Scene::loadVocabStrings() {
	freeVocab();
	File f("*VOCAB.DAT");

	char *textStrings = new char[f.size()];
	f.read(textStrings, f.size());

	for (uint strIndex = 0; strIndex < _activeVocabs.size(); ++strIndex) {
		const char *s = textStrings;
		for (int vocabIndex = 0; vocabIndex < _activeVocabs[strIndex]; ++vocabIndex)
			s += strlen(s) + 1;

		_vocabStrings.push_back(s);
	}

	delete[] textStrings;
	f.close();
}

void Scene::initPaletteAnimation(Common::Array<RGB4> &animData, bool animFlag) {
	// Initialise the animation palette and ticks list
	_animTicksList.clear();
	_animPalData.clear();

	for (uint i = 0; i < animData.size(); ++i) {
		_animTicksList.push_back(_vm->_events->getFrameCounter());
		_animPalData.push_back(animData[i]);
	}

	// Save the initial starting palette
	Common::copy(&_vm->_palette->_mainPalette[0], &_vm->_palette->_mainPalette[PALETTE_SIZE],
		&_vm->_palette->_savedPalette[0]);

	// Calculate total
	_animCount = 0;
	for (uint i = 0; i < _animPalData.size(); ++i)
		_animCount += _animPalData[i].r;

	_animVal1 = (_animCount > 16) ? 3 : 0;
	_animFlag = animFlag;
}

void Scene::free() {
	warning("TODO: Scene::free");
}

} // End of namespace MADS
