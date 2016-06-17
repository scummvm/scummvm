#include "macventure/world.h"

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
		res = _saveGame->getGroups()[attrID][objID];
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

bool MacVenture::World::isObjActive(ObjID obj) {
	return false;
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

const Common::Array<AttributeGroup>& MacVenture::SaveGame::getGroups() {
	return _groups;
}

const AttributeGroup * SaveGame::getGroup(uint32 groupID) {
	assert(groupID < _groups.size());
	return &(_groups[groupID]);
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
