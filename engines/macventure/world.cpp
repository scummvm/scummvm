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

	Common::SeekableReadStream *saveGameRes = saveGameFile.readStream(saveGameFile.size());

	_saveGame = new SaveGame(_engine, saveGameRes);

	_objectConstants = new Container("Shadowgate II/Shadow Graphic");

	uint32 size = _objectConstants->getItemByteSize(2);
	char * ob1 = new char[size];
	_objectConstants->getItem(2, ob1);

	delete saveGameRes;
	saveGameFile.close();		
}


World::~World()	{
	
	if (_saveGame)
		delete _saveGame;

	if (_objectConstants)
		delete _objectConstants;
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
	_startGameFileName.replace(_startGameFileName.end(), _startGameFileName.end(), ".bin");

	return true;
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
