#include "macventure/world.h"
#include "macventure/macventure.h"

#include "common/file.h"

namespace MacVenture {

World::World(MacVentureEngine *engine, Common::MacResManager *resMan)  {
	_resourceManager = resMan;	
	_engine = engine;

	if (!loadStartGameFileName())
		error("Could not load initial game configuration");

	Common::File saveGameFile;
	if (!saveGameFile.open(_startGameFileName)) 
		error("Could not load initial game configuration");

	debug("Loading save game state from %s", _startGameFileName.c_str());
	Common::SeekableReadStream *saveGameRes = saveGameFile.readStream(saveGameFile.size());

	_saveGame = new SaveGame(_engine, saveGameRes);	
	_objectConstants = new Container(_engine->getFilePath(kObjectPathID).c_str());
	calculateObjectRelations();
	
	_gameText = new Container("Shadowgate II/Shadow Text");

	ObjID tid = (ObjID)1;
	TextAsset test = TextAsset(tid, _gameText, _engine->isOldText(), _engine->getDecodingHuffman());

	delete saveGameRes;
	saveGameFile.close();		
}


World::~World()	{
	
	if (_saveGame)
		delete _saveGame;

	if (_objectConstants)
		delete _objectConstants;
}


uint32 World::getObjAttr(ObjID objID, uint32 attrID) {
	uint32 res;
	uint32 index = _engine->getGlobalSettings().attrIndices[attrID];
	if (!(index & 0x80)) { // It's not a constant
		res = _saveGame->getAttr(objID, index);
	} else {
		Common::SeekableReadStream *objStream = _objectConstants->getItem(objID);
		index &= 0x7F;
		objStream->skip((index * 2) - 1);
		res = objStream->readUint16BE();
	}
	res &= _engine->getGlobalSettings().attrMasks[attrID];
	res >>= _engine->getGlobalSettings().attrShifts[attrID];
	debug(11, "Attribute %x from object %x is %x", attrID, objID, res);
	return res;
}

void World::setObjAttr(ObjID objID, uint32 attrID, Attribute value) {
	if (attrID == kAttrPosX || attrID == kAttrPosY) {}
		// Round to scale
	
	if (attrID == kAttrParentObject) 
		setParent(objID, value);
	
	if (attrID < kAttrOtherDoor)
		_engine->enqueueObject(objID);

	uint32 idx = _engine->getGlobalSettings().attrIndices[attrID];
	value <<= _engine->getGlobalSettings().attrShifts[attrID];
	value &= _engine->getGlobalSettings().attrMasks[attrID];
	Attribute oldVal = _saveGame->getAttr(objID, idx);
	oldVal &= ~_engine->getGlobalSettings().attrMasks[attrID];
	_saveGame->setAttr(idx, objID, (value | oldVal));
	_engine->gameChanged();
}

bool MacVenture::World::isObjActive(ObjID obj) {
	return false;
}

Common::Array<ObjID> World::getFamily(ObjID objID, bool recursive) {
	Common::Array<ObjID> res;
	res.push_back(objID);
	res.push_back(getChildren(objID, recursive));
	return res;
}

Common::Array<ObjID> World::getChildren(ObjID objID, bool recursive) {
	Common::Array<ObjID> res;
	ObjID child = _relations[objID * 2];
	while (child) {
		res.push_back(child);
		if (!recursive)
			res.push_back(getChildren(child, false));
		child = _relations[child * 2 + 1];
	}
	return Common::Array<ObjID>();
}

WindowReference World::getObjWindow(ObjID objID) {
	switch (objID) {
	case 0xfffc: return kExitsWindow;
	case 0xfffd: return kSelfWindow;
	case 0xfffe: return kOutConsoleWindow;
	case 0xffff: return kCommandsWindow;
	}

	return findObjWindow(objID);
}

WindowReference World::findObjWindow(ObjID objID) {
	return kMainGameWindow;
}

Attribute World::getGlobal(uint32 attrID) {
	return _saveGame->getGlobals()[attrID];
}

void World::setGlobal(uint32 attrID, Attribute value) {
	_saveGame->setGlobal(attrID, value);
}

void World::updateObj(ObjID objID) {
	WindowReference win;
	if (getObjAttr(1, kAttrParentObject) == objID) {
		win = kMainGameWindow;
	} else {
		win = getObjWindow(objID);
	}
	if (win) {
		//focusObjWin(objID);
		_engine->runObjQueue();
		//_engine->updateWindow(win);
	}
}

void World::captureChildren(ObjID objID) {
}

void World::releaseChildren(ObjID objID) {
}

Common::String World::getText(ObjID objID) {
	TextAsset text = TextAsset(objID, _gameText, _engine->isOldText(), _engine->getDecodingHuffman());
	
	return *text.decode();
}


bool World::loadStartGameFileName() {
	Common::SeekableReadStream *res;

	res = _resourceManager->getResource(MKTAG('S', 'T', 'R', ' '), kStartGameFilenameID);
	if (!res)
		return false;

	byte length = res->readByte();
	char *fileName = new char[length + 1];
	res->read(fileName, length);
	fileName[length] = '\0';
	_startGameFileName = Common::String(fileName, length);

	return true;
}

void World::calculateObjectRelations() {
	ObjID val, next;
	uint32 numObjs = _engine->getGlobalSettings().numObjects;
	const AttributeGroup &parents = *_saveGame->getGroup(0);
	for (uint i = 0; i < numObjs * 2; i++) {
		_relations.push_back(0);
	}
	for (uint i = numObjs - 1; i > 0; i--) {
		val = parents[i];
		next = _relations[val * 2];
		if (next) { _relations[i * 2 + 1] = next; }
		_relations[val * 2] = i;
	}
}

void World::setParent(ObjID child, ObjID newParent) {
	ObjID old = _saveGame->getAttr(child, kAttrParentObject);
	if (newParent == child)
		return;

	ObjID oldNdx = old * 2;
	old = _relations[oldNdx];
	while (old != child) {
		oldNdx = (old * 2) + 1;
		old = _relations[oldNdx];
	}
	_relations[oldNdx] = _relations[(old * 2) + 1];
	oldNdx = newParent * 2;
	old = _relations[oldNdx];
	while (old && old <= child) {
		oldNdx = (old * 2) + 1;
		old = _relations[oldNdx];
	}
	_relations[child * 2 + 1] = old;
	_relations[oldNdx] = child;
}

// SaveGame
SaveGame::SaveGame(MacVentureEngine *engine, Common::SeekableReadStream *res) {
	_groups = Common::Array<AttributeGroup>();
	loadGroups(engine, res);
	_globals = Common::Array<uint16>();
	loadGlobals(engine, res);
	_text = Common::String();
	loadText(engine, res);
}

SaveGame::~SaveGame() {
}


Attribute SaveGame::getAttr(ObjID objID, uint32 attrID) {
	return _groups[attrID][objID];
}

void SaveGame::setAttr(uint32 attrID, ObjID objID, Attribute value) {
	_groups[attrID][objID] = value;
}

const Common::Array<AttributeGroup>& MacVenture::SaveGame::getGroups() {
	return _groups;
}

const AttributeGroup * SaveGame::getGroup(uint32 groupID) {
	assert(groupID < _groups.size());
	return &(_groups[groupID]);
}

void SaveGame::setGlobal(uint32 attrID, Attribute value) {
	_globals[attrID] = value;
}

const Common::Array<uint16>& MacVenture::SaveGame::getGlobals() {
	return _globals;
}

const Common::String & MacVenture::SaveGame::getText() {
	return _text;
}

void SaveGame::loadGroups(MacVentureEngine *engine, Common::SeekableReadStream * res) {
	GlobalSettings settings = engine->getGlobalSettings();
	for (int i = 0; i < settings.numGroups; ++i) {
		AttributeGroup g;
		for (int j = 0; j < settings.numObjects; ++j)
			g.push_back(res->readUint16BE());
		
		_groups.push_back(g);
	}
}

void SaveGame::loadGlobals(MacVentureEngine *engine, Common::SeekableReadStream * res) {
	GlobalSettings settings = engine->getGlobalSettings();
	for (int i = 0; i < settings.numGlobals; ++i) {
		_globals.push_back(res->readUint16BE());
	}
}

void SaveGame::loadText(MacVentureEngine *engine, Common::SeekableReadStream * res) {
	_text = "Placeholder Console Text";
}


} // End of namespace MacVenture
