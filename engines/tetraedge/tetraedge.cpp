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

#include "tetraedge/tetraedge.h"
#include "tetraedge/detection.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "common/savefile.h"
#include "engines/util.h"
#include "engines/dialogs.h"

#include "tetraedge/game/amerzone_game.h"
#include "tetraedge/game/syberia_game.h"
#include "tetraedge/game/application.h"
#include "tetraedge/game/character.h"
#include "tetraedge/te/te_core.h"
#include "tetraedge/te/te_renderer.h"
#include "tetraedge/te/te_resource_manager.h"
#include "tetraedge/te/te_lua_thread.h"
#include "tetraedge/te/te_sound_manager.h"
#include "tetraedge/te/te_input_mgr.h"
#include "tetraedge/te/te_particle.h"
#include "tetraedge/obb_archive.h"

namespace Tetraedge {

TetraedgeEngine *g_engine;

TetraedgeEngine::TetraedgeEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Tetraedge"), _resourceManager(nullptr),
	_core(nullptr),	_application(nullptr), _game(nullptr), _renderer(nullptr),
	_soundManager(nullptr), _inputMgr(nullptr), _gameType(kNone) {
	g_engine = this;
}

TetraedgeEngine::~TetraedgeEngine() {
	//TeObject::deleteNow();
	delete _core;
	delete _game;
	delete _application;
	delete _renderer;
	delete _soundManager;
	delete _resourceManager;
	delete _inputMgr;
	for (Common::Array<Common::Archive *>::iterator it = _rootArchives.begin(); it != _rootArchives.end(); it++)
		delete *it;
	Object3D::cleanup();
	Character::cleanup();
	TeAnimation::cleanup();
	TeLuaThread::cleanup();
	TeTimer::cleanup();
	TeObject::cleanup();
	TeParticle::cleanup();
}

/*static*/
Common::StringArray TetraedgeEngine::splitString(const Common::String &text, char c) {
	Common::StringArray values;

	Common::String str = text;
	size_t pos;
	while ((pos = str.findFirstOf(c)) != Common::String::npos) {
		values.push_back(Common::String(str.c_str(), pos));
		str = Common::String(str.c_str() + pos + 1);
	}

	values.push_back(str);
	return values;
}

Application *TetraedgeEngine::getApplication() {
	// created in run()
	return _application;
}

TeCore *TetraedgeEngine::getCore() {
	if (_core == nullptr)
		_core = new TeCore();
	return _core;
}

Game *TetraedgeEngine::getGame() {
	if (_game == nullptr) {
		if (gameIsAmerzone())
			_game = new AmerzoneGame();
		else
			_game = new SyberiaGame();
	}
	return _game;
}

TeSoundManager *TetraedgeEngine::getSoundManager() {
	if (_soundManager == nullptr)
		_soundManager = new TeSoundManager();
	return _soundManager;
}

TeRenderer *TetraedgeEngine::getRenderer() {
	// created in run()
	return _renderer;
}

TeResourceManager *TetraedgeEngine::getResourceManager() {
	if (_resourceManager == nullptr)
		_resourceManager = new TeResourceManager();
	return _resourceManager;
}

TeInputMgr *TetraedgeEngine::getInputMgr() {
	if (_inputMgr == nullptr)
		_inputMgr = new TeInputMgr();
	return _inputMgr;
}

uint32 TetraedgeEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String TetraedgeEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Language TetraedgeEngine::getGameLanguage() const {
	return _gameDescription->language;
}

Common::Platform TetraedgeEngine::getGamePlatform() const {
	return _gameDescription->platform;
}

bool TetraedgeEngine::isUtf8Release() const {
	return !!(_gameDescription->flags & GF_UTF8);
}

bool TetraedgeEngine::isGameDemo() const {
	return (_gameDescription->flags & ADGF_DEMO) != 0;
}

bool TetraedgeEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	return _game && _application && !_application->mainMenu().isEntered();
}

bool TetraedgeEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	return canSaveAutosaveCurrently() && !_application->isLockCursor();
}

bool TetraedgeEngine::canSaveAutosaveCurrently() {
	if (!_game || !_application)
		return false;

	bool sceneLoaded;

	if (gameIsAmerzone()) {
		AmerzoneGame *game = dynamic_cast<AmerzoneGame *>(_game);
		assert(game);
		sceneLoaded = (game->warpY() != nullptr);
	} else {
		sceneLoaded = !_game->currentScene().empty() && !_game->currentZone().empty();
	}

	return _game->running() && sceneLoaded;
}

Common::Error TetraedgeEngine::loadGameState(int slot) {
	// In case autosaves are on, do a save first before loading the new save
	saveAutosaveIfEnabled();

	Common::String saveStateName = getSaveStateName(slot);

	Common::InSaveFile *saveFile = getSaveFileManager()->openForLoading(saveStateName);

	if (!saveFile)
		return Common::kReadingFailed;

	// The game will reopen the file and do the loading, see Game::initLoadedBackupData
	getGame()->setLoadName(saveStateName);

	delete saveFile;
	return Common::kNoError;
}

Common::Error TetraedgeEngine::loadGameStream(Common::SeekableReadStream *stream) {
	Common::Serializer s(stream, nullptr);
	Common::Error retval = syncGame(s);
	return retval;
}

void TetraedgeEngine::closeGameDialogs() {
	if (!_game)
		return;
	_game->closeDialogs();
}

void TetraedgeEngine::configureSearchPaths() {
	const Common::FSNode gameDataDir(ConfMan.getPath("path"));
	if (_gameDescription->platform == Common::kPlatformMacintosh) {
		SearchMan.addSubDirectoryMatching(gameDataDir, "Resources", 0, 6);
		_rootArchives.push_back(new Common::FSDirectory(gameDataDir.getChild("Resources"), 10));
	} else
		_rootArchives.push_back(new Common::FSDirectory(gameDataDir, 10));

	if (_gameDescription->platform == Common::Platform::kPlatformAndroid
	    && strlen(_gameDescription->filesDescriptions[0].fileName) > 4
	    && scumm_stricmp(_gameDescription->filesDescriptions[0].fileName + strlen(_gameDescription->filesDescriptions[0].fileName) - 4, ".obb") == 0) {
		ObbArchive *obb = ObbArchive::open(_gameDescription->filesDescriptions[0].fileName);
		_rootArchives.push_back(obb);
		SearchMan.add("obbarchive", obb, 0, false);
	}
}

int TetraedgeEngine::getDefaultScreenWidth() const {
	return gameIsAmerzone() ? 1280 : 800;
}

int TetraedgeEngine::getDefaultScreenHeight() const {
	return gameIsAmerzone() ? 800 : 600;
}

bool TetraedgeEngine::onKeyUp(const Common::KeyState &state) {
	switch (state.keycode) {
	case Common::KEYCODE_l:
		if (loadGameDialog())
			_game->initLoadedBackupData();
		break;
	case Common::KEYCODE_s:
		saveGameDialog();
		break;
	case Common::KEYCODE_ESCAPE:
		closeGameDialogs();
		break;
	default:
		break;
	}

	return false;
}

void TetraedgeEngine::registerConfigDefaults() {
	// The skips are mostly for debugging to jump straight to certain
	// things.  If they are all enabled you get into a new game as
	// soon as possible.
	ConfMan.registerDefault("skip_videos", false);
	ConfMan.registerDefault("skip_splash", false);
	ConfMan.registerDefault("skip_mainmenu", false);
	ConfMan.registerDefault("skip_confirm", false);

	ConfMan.registerDefault("disable_shadows", false);
	ConfMan.registerDefault("correct_movie_aspect", true);
	ConfMan.registerDefault("restore_scenes", false);
}


Common::Error TetraedgeEngine::run() {
	if (getGameId() == "syberia")
		_gameType = kSyberia;
	else if (getGameId() == "syberia2")
		_gameType = kSyberia2;
	else if (getGameId() == "amerzone")
		_gameType = kAmerzone;
	else
		error("Unknown game id %s", getGameId().c_str());

	registerConfigDefaults();

	configureSearchPaths();
	// from BasicOpenGLView::prepareOpenGL..
	_application = new Application();
	_renderer = TeRenderer::makeInstance();
	_renderer->init(getDefaultScreenWidth(), getDefaultScreenHeight());
	_renderer->reset();

	getInputMgr()->_keyUpSignal.add(this, &TetraedgeEngine::onKeyUp);

	// If a savegame was selected from the launcher, load it.
	// Should be before application->create() because it only
	// sets the game name to load inside the Game object.  It will
	// actually be loaded when the application is created.
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	_application->create();

	Common::Event e;

	while (!shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
			getInputMgr()->handleEvent(e);
		}

		_application->run();

		g_system->delayMillis(10);
	}

	// Ensure game has stopped.
	_game->leave(true);
	TeObject::deleteNow();
	_application->destroy();

	return Common::kNoError;
}

Common::Error TetraedgeEngine::syncGame(Common::Serializer &s) {
	return getGame()->syncGame(s);
}

void TetraedgeEngine::openConfigDialog() {
	GUI::ConfigDialog dlg;
	dlg.runModal();

	g_system->applyBackendSettings();
	applyGameSettings();
	syncSoundSettings();
}


Graphics::RendererType TetraedgeEngine::preferredRendererType() const {
	Common::String rendererConfig = ConfMan.get("renderer");
	Graphics::RendererType desiredRendererType = Graphics::Renderer::parseTypeCode(rendererConfig);
	uint32 availableRendererTypes = Graphics::Renderer::getAvailableTypes();

	availableRendererTypes &=
#if defined(USE_OPENGL_GAME)
			Graphics::kRendererTypeOpenGL |
#endif
#if defined(USE_OPENGL_SHADERS)
			Graphics::kRendererTypeOpenGLShaders |
#endif
#if defined(USE_TINYGL)
			Graphics::kRendererTypeTinyGL |
#endif
			0;

	Graphics::RendererType matchingRendererType = Graphics::Renderer::getBestMatchingType(desiredRendererType, availableRendererTypes);
	// Currently no difference between shaders and otherwise for this engine.
	if (matchingRendererType == Graphics::kRendererTypeOpenGLShaders)
		matchingRendererType = Graphics::kRendererTypeOpenGL;

	if (matchingRendererType == 0) {
		error("No supported renderer available.");
	}

	return matchingRendererType;
}

Common::Error TetraedgeEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	Common::Error result = Engine::saveGameState(slot, desc, isAutosave);
	if (result.getCode() == Common::kNoError) {
		ConfMan.setInt("last_save_slot", slot);
		ConfMan.flushToDisk();
	}
	return result;
}

/*static*/
void TetraedgeEngine::getSavegameThumbnail(Graphics::Surface &thumb) {
	g_engine->getApplication()->getSavegameThumbnail(thumb);
}

bool TetraedgeFSNode::getChildren(TetraedgeFSList &fslist, Common::FSNode::ListMode mode, bool hidden) const {
	if (!_archive)
		return false;

	Common::Array<Common::String> tmpsublist;
	if(!_archive->getChildren(_archivePath, tmpsublist, (Common::Archive::ListMode)  mode, hidden))
		return false;
	fslist.clear();
	for(Common::Array<Common::String>::iterator it = tmpsublist.begin(); it != tmpsublist.end(); it++) {
		fslist.push_back(TetraedgeFSNode(_archive, _archivePath.join(*it)));
	}
	return true;
}

class SubPathArchive : public Common::Archive {
public:
	SubPathArchive(Common::Archive *archive, const Common::Path &prefix) : _archive(archive), _prefix(prefix) {}

	bool hasFile(const Common::Path &path) const override {
		return _archive && _archive->hasFile(_prefix.join(path));
	}

	bool isPathDirectory(const Common::Path &path) const override {
		return _archive && _archive->isPathDirectory(_prefix.join(path));
	}

	bool getChildren(const Common::Path &path, Common::Array<Common::String> &list, ListMode mode, bool hidden) const override {
		return _archive && getChildren(_prefix.join(path), list, mode, hidden);
	}

	int listMembers(Common::ArchiveMemberList &list) const override {
		Common::ArchiveMemberList tmpList;
		if (!_archive)
			return 0;
		_archive->listMembers(tmpList);
		Common::String prefixStr = _prefix.toString();
		if (!prefixStr.hasSuffix("/"))
			prefixStr += "/";
		int count = 0;
		for (Common::ArchiveMemberList::iterator it = tmpList.begin(); it != tmpList.end(); it++) {
			if ((*it)->getName().hasPrefix(prefixStr)) {
				list.push_back(*it);
				count++;
			}
		}
		return count;
	}

	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override {
		return _archive ? _archive->getMember(_prefix.join(path)) : nullptr;
	}

	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override {
		return _archive ? _archive->createReadStreamForMember(_prefix.join(path)) : nullptr;
	}

	char getPathSeparator() const override {
		return _archive ? _archive->getPathSeparator() : '/';
	}

private:
	Common::Archive *_archive;
	Common::Path _prefix;
};

void TetraedgeFSNode::maybeAddToSearchMan() const {
	const Common::String path = getPath().toString(Common::Path::kNativeSeparator);
	if (SearchMan.hasArchive(path))
		return;
	if (!_archivePath.empty())
		SearchMan.add(path, new SubPathArchive(_archive, _archivePath));
}

Common::SeekableReadStream *TetraedgeFSNode::createReadStream() const {
	return _archive ? _archive->createReadStreamForMember(_archivePath) : nullptr;
}

bool TetraedgeFSNode::isReadable() const {
	return _archive && _archive->hasFile(_archivePath);
}

bool TetraedgeFSNode::isDirectory() const {
	return _archive && _archive->isPathDirectory(_archivePath);
}

Common::Path TetraedgeFSNode::getPath() const {
	return _archivePath;
}

Common::String TetraedgeFSNode::toString() const {
	return _archivePath.toString(Common::Path::kNativeSeparator);
}

TetraedgeFSNode TetraedgeFSNode::getChild(const Common::Path &path) const {
	return TetraedgeFSNode(_archive, _archivePath.join(path));
}

bool TetraedgeFSNode::exists() const {
	return isDirectory() || isReadable();
}

bool TetraedgeFSNode::loadXML(Common::XMLParser &parser) const {
	return parser.loadStream(createReadStream(), _archivePath.toString());
}

Common::String TetraedgeFSNode::getName() const {
	return _archivePath.getLastComponent().toString();
}

bool TetraedgeFSNode::operator<(const TetraedgeFSNode& node) const {
	return getPath() < node.getPath();
}

int TetraedgeFSNode::getDepth() const {
	return _archivePath.splitComponents().size();
}

} // namespace Tetraedge
