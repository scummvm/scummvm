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

#include "base/plugins.h"

#include "common/func.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/config-manager.h"

#ifdef DYNAMIC_MODULES
#include "common/fs.h"
#endif

#include "base/detection/detection.h"

#include "engines/advancedDetector.h"

// Plugin versioning

const int pluginTypeVersions[PLUGIN_TYPE_MAX] = {
	PLUGIN_TYPE_ENGINE_DETECTION_VERSION,
	PLUGIN_TYPE_ENGINE_VERSION,
	PLUGIN_TYPE_MUSIC_VERSION,
	PLUGIN_TYPE_DETECTION_VERSION,
	PLUGIN_TYPE_SCALER_VERSION,
};


// Abstract plugins

PluginType Plugin::getType() const {
	return _type;
}

const char *Plugin::getName() const {
	return _pluginObject->getName();
}

StaticPlugin::StaticPlugin(PluginObject *pluginobject, PluginType type) {
	assert(pluginobject);
	assert(type < PLUGIN_TYPE_MAX);
	_pluginObject = pluginobject;
	_type = type;
}

StaticPlugin::~StaticPlugin() {
	delete _pluginObject;
}

bool StaticPlugin::loadPlugin()		{ return true; }
void StaticPlugin::unloadPlugin()	{}

class StaticPluginProvider : public PluginProvider {
public:
	StaticPluginProvider() {
	}

	~StaticPluginProvider() {
	}

	PluginList getPlugins() override {
		PluginList pl;

		#define LINK_PLUGIN(ID) \
			extern const PluginType g_##ID##_type; \
			extern PluginObject *g_##ID##_getObject(); \
			pl.push_back(new StaticPlugin(g_##ID##_getObject(), g_##ID##_type));

		// "Loader" for the static plugins.
		// Iterate over all registered (static) plugins and load them.

		// Engine plugins
		#include "engines/plugins_table.h"

		#ifdef DETECTION_STATIC
		// Engine-detection plugins are included if we don't use uncached plugins.
		#include "engines/detection_table.h"
		#endif

		// Music plugins
		// TODO: Use defines to disable or enable each MIDI driver as a
		// static/dynamic plugin, like it's done for the engines
		LINK_PLUGIN(AUTO)
		LINK_PLUGIN(NULL)
		#if defined(WIN32)
		LINK_PLUGIN(WINDOWS)
		#endif
		#if defined(USE_ALSA)
		LINK_PLUGIN(ALSA)
		#endif
		#if defined(USE_SEQ_MIDI)
		LINK_PLUGIN(SEQ)
		#endif
		#if defined(USE_SNDIO)
		LINK_PLUGIN(SNDIO)
		#endif
		#if defined(__MINT__)
		LINK_PLUGIN(STMIDI)
		#endif
		#if defined(IRIX)
		LINK_PLUGIN(DMEDIA)
		#endif
		#if defined(__amigaos4__) || defined(__MORPHOS__)
		LINK_PLUGIN(CAMD)
		#endif
		#if defined(RISCOS)
		LINK_PLUGIN(RISCOS)
		#endif
		#if defined(MACOSX)
		LINK_PLUGIN(COREAUDIO)
		LINK_PLUGIN(COREMIDI)
		#endif
		#ifdef USE_FLUIDSYNTH
		LINK_PLUGIN(FLUIDSYNTH)
		#endif
		#ifdef USE_MT32EMU
		LINK_PLUGIN(MT32)
		#endif
		#if defined(USE_SONIVOX)
		LINK_PLUGIN(EAS)
		#endif
		LINK_PLUGIN(ADLIB)
		LINK_PLUGIN(PCSPK)
		LINK_PLUGIN(PCJR)
		LINK_PLUGIN(CMS)
		#ifndef DISABLE_SID
		LINK_PLUGIN(C64)
		#endif
		LINK_PLUGIN(AMIGA)
		LINK_PLUGIN(APPLEIIGS)
		LINK_PLUGIN(MACINTOSH)
		LINK_PLUGIN(TOWNS)
		LINK_PLUGIN(PC98)
		LINK_PLUGIN(SEGACD)
		#if defined(USE_TIMIDITY)
		LINK_PLUGIN(TIMIDITY)
		#endif

		// Scaler plugins
		LINK_PLUGIN(NORMAL)
#ifdef USE_SCALERS
#ifdef USE_HQ_SCALERS
		LINK_PLUGIN(HQ)
#endif
#ifdef USE_EDGE_SCALERS
		LINK_PLUGIN(EDGE)
#endif
		LINK_PLUGIN(ADVMAME)
		LINK_PLUGIN(SAI)
		LINK_PLUGIN(SUPERSAI)
		LINK_PLUGIN(SUPEREAGLE)
		LINK_PLUGIN(PM)
		LINK_PLUGIN(DOTMATRIX)
		LINK_PLUGIN(TV)
#endif

		return pl;
	}
};

#ifdef DYNAMIC_MODULES

PluginList FilePluginProvider::getPlugins() {
	PluginList pl;

	// Prepare the list of directories to search
	Common::FSList pluginDirs;

	// Add the default directories
	#ifndef WIN32
	pluginDirs.push_back(Common::FSNode("."));
	#endif
	#ifndef PSP2
	pluginDirs.push_back(Common::FSNode("plugins"));
	#endif

	// Add the provider's custom directories
	addCustomDirectories(pluginDirs);

	// Add the user specified directory
	Common::Path pluginsPath(ConfMan.getPath("pluginspath"));
	if (!pluginsPath.empty())
		pluginDirs.push_back(Common::FSNode(pluginsPath));

	Common::FSList::const_iterator dir;
	for (dir = pluginDirs.begin(); dir != pluginDirs.end(); ++dir) {
		// Load all plugins.
		// Scan for all plugins in this directory
		Common::FSList files;
		if (!dir->getChildren(files, Common::FSNode::kListFilesOnly)) {
			debug(1, "Couldn't open plugin directory '%s'", dir->getPath().toString().c_str());
			continue;
		} else {
			debug(1, "Reading plugins from plugin directory '%s'", dir->getPath().toString().c_str());
		}

		for (Common::FSList::const_iterator i = files.begin(); i != files.end(); ++i) {
			if (isPluginFilename(*i)) {
				pl.push_back(createPlugin(*i));
			}
		}
	}

	return pl;
}

bool FilePluginProvider::isPluginFilename(const Common::FSNode &node) const {
	Common::String filename = node.getName();

#ifdef PLUGIN_PREFIX
	// Check the plugin prefix
	if (!filename.hasPrefix(PLUGIN_PREFIX))
		return false;
#endif

#ifdef PLUGIN_SUFFIX
	// Check the plugin suffix
	if (!filename.hasSuffix(PLUGIN_SUFFIX))
		return false;
#endif

	return true;
}

void FilePluginProvider::addCustomDirectories(Common::FSList &dirs) const {
#ifdef PLUGIN_DIRECTORY
	dirs.push_back(Common::FSNode(PLUGIN_DIRECTORY));
#endif
}

#endif // DYNAMIC_MODULES

#pragma mark -

PluginManager *PluginManager::_instance = nullptr;

PluginManager &PluginManager::instance() {
	if (_instance)
		return *_instance;

#if defined(UNCACHED_PLUGINS) && defined(DYNAMIC_MODULES)
		_instance = new PluginManagerUncached();
#else
		_instance = new PluginManager();
#endif
	return *_instance;
}

PluginManager::PluginManager() {
	// Always add the static plugin provider.
	addPluginProvider(new StaticPluginProvider());
}

PluginManager::~PluginManager() {
	// Explicitly unload all loaded plugins
	unloadAllPlugins();

	// Delete the plugin providers
	for (ProviderList::iterator pp = _providers.begin();
	                            pp != _providers.end();
	                            ++pp) {
		delete *pp;
	}
}

void PluginManager::addPluginProvider(PluginProvider *pp) {
	_providers.push_back(pp);
}

PluginManagerUncached::~PluginManagerUncached() {
	// Unload from memory all engine plugins without deleting them
	// They are also referenced from _allEnginePlugins which we clean up here
	unloadPluginsExcept(PLUGIN_TYPE_ENGINE, nullptr, false);

	for (PluginList::iterator p = _allEnginePlugins.begin(); p != _allEnginePlugins.end(); ++p) {
		delete *p;
	}
	_allEnginePlugins.clear();

	// Explicitly unload the detection plugin
	unloadDetectionPlugin();
}

/**
 * This should only be called once by main()
 **/
void PluginManagerUncached::init() {
	ConfMan.setBool("always_run_fallback_detection_extern", false);

	unloadPluginsExcept(PLUGIN_TYPE_ENGINE, nullptr, false); // empty the engine plugins

#ifndef DETECTION_STATIC
	Common::String detectPluginName = "detection";
#ifdef PLUGIN_SUFFIX
	detectPluginName += PLUGIN_SUFFIX;
#endif

	bool foundDetectPlugin = false;
#endif

	for (ProviderList::iterator pp = _providers.begin();
	                            pp != _providers.end();
	                            ++pp) {
		PluginList pl((*pp)->getPlugins());

		for (PluginList::iterator p = pl.begin(); p != pl.end(); ++p) {
			// This is a 'hack' based on the assumption that we have no sound
			// file plugins. Currently this is the case. If it changes, we
			// should find a fast way of detecting whether a plugin is a
			// music or an engine plugin.
#ifndef DETECTION_STATIC
			if (!foundDetectPlugin && (*pp)->isFilePluginProvider()) {
				Common::Path pName = (*p)->getFileName();
				if (pName.baseName().hasSuffixIgnoreCase(detectPluginName)) {
					_detectionPlugin = (*p);
					foundDetectPlugin = true;
					debug(9, "Detection plugin found!");
					continue;
				}
			}
#endif

			if ((*pp)->isFilePluginProvider()) {
				_allEnginePlugins.push_back(*p);
			} else if ((*p)->loadPlugin()) { // and this is the proper method
				if ((*p)->getType() == PLUGIN_TYPE_ENGINE) {
					(*p)->unloadPlugin();
					_allEnginePlugins.push_back(*p);
				} else {	// add non-engine plugins to the 'in-memory' list
							// these won't ever get unloaded
					addToPluginsInMemList(*p);
				}
			}
 		}
 	}
}

/**
 * Try to load the plugin by searching in the ConfigManager for a matching
 * engine ID under the domain 'engine_plugin_files'.
 **/
bool PluginManagerUncached::loadPluginFromEngineId(const Common::String &engineId) {
	Common::ConfigManager::Domain *domain = ConfMan.getDomain("engine_plugin_files");

	if (domain) {
		if (domain->contains(engineId)) {
			Common::Path filename(Common::Path::fromConfig((*domain)[engineId]));

			if (loadPluginByFileName(filename)) {
				return true;
			}
		}
	}
	// Check for a plugin with the same name as the engine before starting
	// to scan all plugins
	Common::String tentativeEnginePluginFilename = engineId;
#ifdef PLUGIN_SUFFIX
	tentativeEnginePluginFilename += PLUGIN_SUFFIX;
#endif
	for (PluginList::iterator p = _allEnginePlugins.begin(); p != _allEnginePlugins.end(); ++p) {
		Common::Path filename = (*p)->getFileName();
		if (filename.baseName().hasSuffixIgnoreCase(tentativeEnginePluginFilename)) {
			if (loadPluginByFileName(filename)) {
				return true;
			}
		}
	}
	return false;
}

/**
 * Load a plugin with a filename taken from ConfigManager.
 **/
bool PluginManagerUncached::loadPluginByFileName(const Common::Path &filename) {
	if (filename.empty())
		return false;

	unloadPluginsExcept(PLUGIN_TYPE_ENGINE, nullptr, false);

	PluginList::iterator i;
	for (i = _allEnginePlugins.begin(); i != _allEnginePlugins.end(); ++i) {
		if ((*i)->getFileName() == filename && (*i)->loadPlugin()) {
			addToPluginsInMemList(*i);
			_currentPlugin = i;
			return true;
		}
	}
	return false;
}

/**
 * Update the config manager with a plugin file name that we found can handle
 * the engine.
 **/
void PluginManagerUncached::updateConfigWithFileName(const Common::String &engineId) {
	// Check if we have a filename for the current plugin
	if (!(*_currentPlugin)->getFileName().empty()) {
		if (!ConfMan.hasMiscDomain("engine_plugin_files"))
			ConfMan.addMiscDomain("engine_plugin_files");

		Common::ConfigManager::Domain *domain = ConfMan.getDomain("engine_plugin_files");
		assert(domain);
		(*domain).setVal(engineId, (*_currentPlugin)->getFileName().toConfig());

		ConfMan.flushToDisk();
	}
}

#ifndef DETECTION_STATIC
void PluginManagerUncached::loadDetectionPlugin() {
	if (_isDetectionLoaded) {
		debug(9, "Detection plugin is already loaded. Adding each available engines to the memory.");
		return;
	}

	if (!_detectionPlugin) {
		debug(9, "Detection plugin not found.");
		return;
	}

	// Unload all leftover engines before reloading the detection plugin
	unloadPluginsExcept(PLUGIN_TYPE_ENGINE, nullptr, false);

	if (!_detectionPlugin->loadPlugin()) {
		debug(9, "Detection plugin was not loaded correctly.");
		return;
	}

	assert((_detectionPlugin)->getType() == PLUGIN_TYPE_DETECTION);

	_pluginsInMem[PLUGIN_TYPE_ENGINE_DETECTION].clear();
	const Detection &detectionConnect = _detectionPlugin->get<Detection>();
	const PluginList &pl = detectionConnect.getPlugins();
	Common::for_each(pl.begin(), pl.end(), Common::bind1st(Common::mem_fun(&PluginManagerUncached::tryLoadPlugin), this));

	_isDetectionLoaded = true;
}

void PluginManagerUncached::unloadDetectionPlugin() {
	if (!_isDetectionLoaded) {
		debug(9, "Detection plugin is already unloaded.");
		return;
	}

	unloadPluginsExcept(PLUGIN_TYPE_ENGINE_DETECTION, nullptr, true);
	_detectionPlugin->unloadPlugin();
	_isDetectionLoaded = false;
}
#endif

void PluginManagerUncached::loadFirstPlugin() {
	unloadPluginsExcept(PLUGIN_TYPE_ENGINE, nullptr, false);

	// let's try to find one we can load
	for (_currentPlugin = _allEnginePlugins.begin(); _currentPlugin != _allEnginePlugins.end(); ++_currentPlugin) {
		if ((*_currentPlugin)->loadPlugin()) {
			addToPluginsInMemList(*_currentPlugin);
			break;
		}
	}
}

bool PluginManagerUncached::loadNextPlugin() {
	unloadPluginsExcept(PLUGIN_TYPE_ENGINE, nullptr, false);

	if (!_currentPlugin || _currentPlugin == _allEnginePlugins.end())
		return false;

	for (++_currentPlugin; _currentPlugin != _allEnginePlugins.end(); ++_currentPlugin) {
		if ((*_currentPlugin)->loadPlugin()) {
			addToPluginsInMemList(*_currentPlugin);
			return true;
		}
	}
	return false; // no more in list
}

/**
 * Used by only the cached plugin manager. The uncached manager can only have
 * one plugin in memory at a time.
 **/
void PluginManager::loadAllPlugins() {
	for (ProviderList::iterator pp = _providers.begin();
	                            pp != _providers.end();
	                            ++pp) {
		PluginList pl((*pp)->getPlugins());
		Common::for_each(pl.begin(), pl.end(), Common::bind1st(Common::mem_fun(&PluginManager::tryLoadPlugin), this));
	}

#ifndef DETECTION_STATIC
	/*
	 * When detection is dynamic, loading above only gets us a PLUGIN_TYPE_DETECTION plugin
	 * We must register all plugins linked in it in order to use them
	 */
	PluginList dpl = getPlugins(PLUGIN_TYPE_DETECTION);
	_pluginsInMem[PLUGIN_TYPE_ENGINE_DETECTION].clear();
	for (PluginList::iterator it = dpl.begin();
	                            it != dpl.end();
	                            ++it) {
		const Detection &detectionConnect = (*it)->get<Detection>();
		const PluginList &pl = detectionConnect.getPlugins();
		Common::for_each(pl.begin(), pl.end(), Common::bind1st(Common::mem_fun(&PluginManager::tryLoadPlugin), this));
	}
#endif
}

void PluginManager::loadAllPluginsOfType(PluginType type) {
	for (ProviderList::iterator pp = _providers.begin();
	                            pp != _providers.end();
	                            ++pp) {
		PluginList pl((*pp)->getPlugins());
		for (PluginList::iterator p = pl.begin();
				                  p != pl.end();
								  ++p) {
			if ((*p)->loadPlugin()) {
				if ((*p)->getType() == type) {
					addToPluginsInMemList((*p));
				} else {
					// Plugin is wrong type
					(*p)->unloadPlugin();
					delete (*p);
				}
			} else {
				// Plugin did not load
				delete (*p);
			}
		}
	}
}

void PluginManager::unloadAllPlugins() {
	for (int i = 0; i < PLUGIN_TYPE_MAX; i++)
		unloadPluginsExcept((PluginType)i, nullptr);
}

void PluginManager::unloadPluginsExcept(PluginType type, const Plugin *plugin, bool deletePlugin /*=true*/) {
	Plugin *found = nullptr;
	for (PluginList::iterator p = _pluginsInMem[type].begin(); p != _pluginsInMem[type].end(); ++p) {
		if (*p == plugin) {
			found = *p;
		} else {
			(*p)->unloadPlugin();
			if (deletePlugin) {
				delete *p;
			}
		}
	}
	_pluginsInMem[type].clear();
	if (found != nullptr) {
		_pluginsInMem[type].push_back(found);
	}
}

/*
 * Used only by the cached plugin manager since it deletes the plugin.
 */
bool PluginManager::tryLoadPlugin(Plugin *plugin) {
	assert(plugin);
	// Try to load the plugin
	if (plugin->loadPlugin()) {
		addToPluginsInMemList(plugin);
		return true;
	} else {
		// Failed to load the plugin
		delete plugin;
		return false;
	}
}

/**
 * Add to the list of plugins loaded in memory.
 */
void PluginManager::addToPluginsInMemList(Plugin *plugin) {
	bool found = false;
	// The plugin is valid, see if it provides the same module as an
	// already loaded one and should replace it.

	PluginList &list = _pluginsInMem[plugin->getType()];
	PluginList::iterator pl = list.begin();
	while (!found && pl != list.end()) {
		if (!strcmp(plugin->getName(), (*pl)->getName())) {
			// Found a duplicated module. Replace the old one.
			found = true;
			(*pl)->unloadPlugin();
			delete *pl;
			*pl = plugin;
			debug(1, "Replaced the duplicated plugin: '%s'", plugin->getName());
		}
		pl++;
	}

	if (!found) {
		// If it provides a new module, just add it to the list of known plugins in memory.
		list.push_back(plugin);
	}
}

// Engine plugins

#include "engines/metaengine.h"

namespace Common {
DECLARE_SINGLETON(EngineManager);
}

/**
 * This function works for both cached and uncached PluginManagers.
 * For the cached version, most of the logic here will short circuit.
 *
 * For the uncached version, we first try to find the plugin using the engineId
 * and only if we can't find it there, we loop through the plugins.
 **/
QualifiedGameList EngineManager::findGamesMatching(const Common::String &engineId, const Common::String &gameId) const {
	QualifiedGameList results;

	if (!engineId.empty()) {
		// If we got an engine name, look for THE game only in that engine
		const Plugin *p = EngineMan.findDetectionPlugin(engineId);
		if (p) {
			const MetaEngineDetection &engine = p->get<MetaEngineDetection>();
			DebugMan.addAllDebugChannels(engine.getDebugChannels());

			PlainGameDescriptor pluginResult = engine.findGame(gameId.c_str());
			if (pluginResult.gameId) {
				results.push_back(QualifiedGameDescriptor(engine.getName(), pluginResult));
			}
		}
	} else {
		// This is a slow path, we have to scan the list of plugins
		PluginMan.loadFirstPlugin();
		do {
			results.push_back(findGameInLoadedPlugins(gameId));
		} while (PluginMan.loadNextPlugin());
	}

	return results;
}

/**
 * Find the game within the plugins loaded in memory
 **/
QualifiedGameList EngineManager::findGameInLoadedPlugins(const Common::String &gameId) const {
	// Find the GameDescriptor for this target
	const PluginList &plugins = getPlugins(PLUGIN_TYPE_ENGINE_DETECTION);

	QualifiedGameList results;
	PluginList::const_iterator iter;

	for (iter = plugins.begin(); iter != plugins.end(); ++iter) {
		const MetaEngineDetection &engine = (*iter)->get<MetaEngineDetection>();
		DebugMan.addAllDebugChannels(engine.getDebugChannels());
		PlainGameDescriptor pluginResult = engine.findGame(gameId.c_str());

		if (pluginResult.gameId) {
			results.push_back(QualifiedGameDescriptor(engine.getName(), pluginResult));
		}
	}

	return results;
}

DetectionResults EngineManager::detectGames(const Common::FSList &fslist, uint32 skipADFlags, bool skipIncomplete) {
	DetectedGames candidates;
	PluginList plugins;
	PluginList::const_iterator iter;

	// MetaEngines are always loaded into memory, so, get them and
	// run detection for all of them.
	plugins = getPlugins(PLUGIN_TYPE_ENGINE_DETECTION);

	// Clear md5 cache before each detection starts, just in case.
	ADCacheMan.clear();

	// Iterate over all known games and for each check if it might be
	// the game in the presented directory.
	for (iter = plugins.begin(); iter != plugins.end(); ++iter) {
		MetaEngineDetection &metaEngine = (*iter)->get<MetaEngineDetection>();
		// set the debug flags
		DebugMan.addAllDebugChannels(metaEngine.getDebugChannels());
		DetectedGames engineCandidates = metaEngine.detectGames(fslist, skipADFlags, skipIncomplete);

		for (uint i = 0; i < engineCandidates.size(); i++) {
			engineCandidates[i].path = fslist.begin()->getParent().getPath();
			engineCandidates[i].shortPath = fslist.begin()->getParent().getDisplayName();
			candidates.push_back(engineCandidates[i]);
		}
	}

	// Close all archives that were opened during detection
	ADCacheMan.clearArchives();

	return DetectionResults(candidates);
}

const PluginList &EngineManager::getPlugins(const PluginType fetchPluginType) const {
	return PluginManager::instance().getPlugins(fetchPluginType);
}

static void addStringToConf(const Common::String &key, const Common::String &value, const Common::String &domain) {
	if (!value.empty())
		ConfMan.set(key, value, domain);
}

static void addPathToConf(const Common::String &key, const Common::Path &value, const Common::String &domain) {
	if (!value.empty())
		ConfMan.setPath(key, value, domain);
}

Common::String EngineManager::generateUniqueDomain(const Common::String gameId) {
	Common::String domainName(gameId);
	int suffixN = 1;
	while (ConfMan.hasGameDomain(domainName)) {
		domainName = gameId + Common::String::format("-%d", suffixN++);
	}
	return domainName;
}

Common::String EngineManager::createTargetForGame(const DetectedGame &game) {
	// The auto detector or the user made a choice.
	// Pick a domain name which does not yet exist (after all, we
	// are *adding* a game to the config, not replacing).
	Common::String domain = game.preferredTarget;

	assert(!domain.empty());
	domain = generateUniqueDomain(domain);

	// Add the name domain
	ConfMan.addGameDomain(domain);

	// Copy all non-empty relevant values into the new domain
	addStringToConf("engineid", game.engineId, domain);
	addStringToConf("gameid", game.gameId, domain);
	addStringToConf("description", game.description, domain);
	addStringToConf("language", Common::getLanguageCode(game.language), domain);
	addStringToConf("platform", Common::getPlatformCode(game.platform), domain);
	addPathToConf("path", game.path, domain);
	addStringToConf("extra", game.extra, domain);
	addStringToConf("guioptions", game.getGUIOptions(), domain);

	// Add any extra configuration keys
	for (Common::StringMap::iterator i = game._extraConfigEntries.begin();
			i != game._extraConfigEntries.end(); ++i)
		addStringToConf((*i)._key, (*i)._value, domain);

	// TODO: Setting the description field here has the drawback
	// that the user does never notice when we upgrade our descriptions.
	// It might be nice to leave this field empty, and only set it to
	// a value when the user edits the description string.
	// However, at this point, that's impractical. Once we have a method
	// to query all backends for the proper & full description of a given
	// game target, we can change this (currently, you can only query
	// for the generic gameid description; it's not possible to obtain
	// a description which contains extended information like language, etc.).

	return domain;
}

const Plugin *EngineManager::findDetectionPlugin(const Common::String &engineId) const {
	const PluginList &plugins = getPlugins(PLUGIN_TYPE_ENGINE_DETECTION);

	for (PluginList::const_iterator iter = plugins.begin(); iter != plugins.end(); iter++)
		if (engineId == (*iter)->get<MetaEngineDetection>().getName())
			return *iter;

	return nullptr;
}

const Plugin *PluginManager::findLoadedPlugin(const Common::String &engineId) {
	const PluginList &plugins = getPlugins(PLUGIN_TYPE_ENGINE);

	for (PluginList::const_iterator iter = plugins.begin(); iter != plugins.end(); iter++)
		if (engineId == (*iter)->get<MetaEngine>().getName())
			return *iter;

	return nullptr;
}

const Plugin *PluginManager::findEnginePlugin(const Common::String &engineId) {
	// First look for the game using the plugins in memory. This is critical
	// for calls coming from inside games
	const Plugin *plugin = findLoadedPlugin(engineId);
	if (plugin)
		return plugin;

	// Now look for the plugin using the engine ID. This is much faster than scanning plugin
	// by plugin
	if (loadPluginFromEngineId(engineId))  {
		plugin = findLoadedPlugin(engineId);
		if (plugin)
			return plugin;
	}

	// We failed to find it using the engine ID. Scan the list of plugins
	PluginMan.loadFirstPlugin();
	do {
		plugin = findLoadedPlugin(engineId);
		if (plugin) {
			// Update with new plugin file name
			PluginMan.updateConfigWithFileName(engineId);
			return plugin;
		}
	} while (PluginMan.loadNextPlugin());

	return nullptr;
}

QualifiedGameDescriptor EngineManager::findTarget(const Common::String &target) const {
	// Ignore empty targets
	if (target.empty())
		return QualifiedGameDescriptor();

	// Lookup the domain. If we have no domain, fallback on the old function [ultra-deprecated].
	const Common::ConfigManager::Domain *domain = ConfMan.getDomain(target);
	if (!domain || !domain->contains("gameid") || !domain->contains("engineid"))
		return QualifiedGameDescriptor();

	// Look for the engine ID
	const Plugin *foundPlugin = findDetectionPlugin(domain->getVal("engineid"));
	if (!foundPlugin) {
		return QualifiedGameDescriptor();
	}

	// Make sure it does support the game ID
	const MetaEngineDetection &engine = foundPlugin->get<MetaEngineDetection>();
	DebugMan.addAllDebugChannels(engine.getDebugChannels());
	PlainGameDescriptor desc = engine.findGame(domain->getVal("gameid").c_str());
	if (!desc.gameId) {
		return QualifiedGameDescriptor();
	}

	return QualifiedGameDescriptor(engine.getName(), desc);
}

void EngineManager::upgradeTargetIfNecessary(const Common::String &target) const {
	Common::ConfigManager::Domain *domain = ConfMan.getDomain(target);
	assert(domain);

	if (!domain->contains("engineid")) {
		upgradeTargetForEngineId(target);
	} else {
		Common::String engineId = domain->getVal("engineid");

		if (engineId.equals("fullpipe")) {
			domain->setVal("engineid", "ngi");

			debug("Upgrading engineid from 'fullpipe' to 'ngi'");

			ConfMan.flushToDisk();

		} else if (engineId.equals("xeen")) {
			domain->setVal("engineid", "mm");

			debug("Upgrading engineid from 'xeen' to 'mm'");
		}
	}
}

void EngineManager::upgradeTargetForEngineId(const Common::String &target) const {
	Common::ConfigManager::Domain *domain = ConfMan.getDomain(target);
	assert(domain);

	debug("Target '%s' lacks an engine ID, upgrading...", target.c_str());

	Common::String oldGameId = domain->getVal("gameid");
	Common::Path path = Common::Path::fromConfig(domain->getVal("path"));

	// At this point the game ID and game path must be known
	if (oldGameId.empty()) {
		warning("The game ID is required to upgrade target '%s'", target.c_str());
		return;
	}
	if (path.empty()) {
		warning("The game path is required to upgrade target '%s'", target.c_str());
		return;
	}

	// Game descriptor for the upgraded target
	Common::String engineId;
	Common::String newGameId;

	// First, try to update entries for engines that previously used the "single id" system
	// Search for an engine whose ID is the game ID
	const Plugin *plugin = findDetectionPlugin(oldGameId);
	if (plugin) {
		// Run detection on the game path
		Common::FSNode dir(path);
		Common::FSList files;
		if (!dir.getChildren(files, Common::FSNode::kListAll)) {
			warning("Failed to access path '%s' when upgrading target '%s'", path.toString(Common::Path::kNativeSeparator).c_str(), target.c_str());
			return;
		}

		// Take the first detection entry
		MetaEngineDetection &metaEngine = plugin->get<MetaEngineDetection>();
		// set debug flags before call detectGames
		DebugMan.addAllDebugChannels(metaEngine.getDebugChannels());
		// Clear md5 cache before detection starts
		ADCacheMan.clear();
		DetectedGames candidates = metaEngine.detectGames(files);
		if (candidates.empty()) {
			warning("No games supported by the engine '%s' were found in path '%s' when upgrading target '%s'",
			        metaEngine.getName(), path.toString(Common::Path::kNativeSeparator).c_str(), target.c_str());
			return;
		}

		engineId = candidates[0].engineId;
		newGameId = candidates[0].gameId;
	}

	// Next, try to find an engine with the game ID in its supported games list
	if (engineId.empty()) {
		QualifiedGameList candidates = findGamesMatching("", oldGameId);
		if (candidates.size() > 1) {
			warning("Multiple matching engines were found when upgrading target '%s'", target.c_str());
			return;
		} else if (!candidates.empty()) {
			engineId = candidates[0].engineId;
			newGameId = candidates[0].gameId;
		}
	}

	if (engineId.empty() || newGameId.empty()) {
		warning("No matching engine was found when upgrading target '%s'", target.c_str());
		return;
	}

	domain->setVal("engineid", engineId);
	domain->setVal("gameid", newGameId);

	// Save a backup of the pre-upgrade gameId to the config file
	if (newGameId != oldGameId) {
		domain->setVal("oldgameid", oldGameId);
	}

	debug("Upgrade complete (engine ID '%s', game ID '%s')", engineId.c_str(), newGameId.c_str());

	ConfMan.flushToDisk();
}

// Music plugins

#include "audio/musicplugin.h"

namespace Common {
DECLARE_SINGLETON(MusicManager);
}

const PluginList &MusicManager::getPlugins() const {
	return PluginManager::instance().getPlugins(PLUGIN_TYPE_MUSIC);
}

// Scaler plugins

#include "graphics/scalerplugin.h"

namespace Common {
DECLARE_SINGLETON(ScalerManager);
}

const PluginList &ScalerManager::getPlugins() const {
	return PluginManager::instance().getPlugins(PLUGIN_TYPE_SCALER);
}

uint ScalerManager::getMaxExtraPixels() const {
	uint maxPixels = 0;
	PluginList plugins = getPlugins();
	PluginList::iterator i = plugins.begin();
	for (; i != plugins.end(); ++i) {
		uint n = (*i)->get<ScalerPluginObject>().extraPixels();
		if (n > maxPixels) {
			maxPixels = n;
		}
	}
	return maxPixels;
}

Plugin *ScalerManager::findScalerPlugin(const char *name) const {
	const PluginList &plugins = getPlugins();
	for (PluginList::const_iterator i = plugins.begin(); i != plugins.end(); ++i) {
		if (!strcmp((*i)->get<ScalerPluginObject>().getName(), name)) {
			return *i;
		}
	}

	return nullptr;
}

uint ScalerManager::findScalerPluginIndex(const char *name) const {
	const PluginList &plugins = getPlugins();
	uint index = 0;

	for (PluginList::const_iterator i = plugins.begin(); i != plugins.end(); ++i) {
		if (!strcmp((*i)->get<ScalerPluginObject>().getName(), name)) {
			return index;
		}
		index++;
	}

	return 0;
}

struct LegacyGraphicsMode {
	const char *oldName;
	const char *newName;
	uint factor;
};

// Table for using old names for scalers in the configuration
// to keep compatibiblity with old config files.
static const LegacyGraphicsMode s_legacyGraphicsModes[] = {
	{ "1x", "normal", 1 },
	{ "2x", "normal", 2 },
	{ "3x", "normal", 3 },
	{ "normal1x", "normal", 1 },
	{ "normal2x", "normal", 2 },
	{ "normal3x", "normal", 3 },
	{ "normal4x", "normal", 4 },
	{ "hq2x", "hq", 2 },
	{ "hq3x", "hq", 3 },
	{ "edge2x", "edge", 2 },
	{ "edge3x", "edge", 3 },
	{ "advmame2x", "advmame", 2 },
	{ "advmame3x", "advmame", 3 },
	{ "advmame4x", "advmame", 4 },
	{ "2xsai", "sai", 2 },
	{ "sai2x", "sai", 2 },
	{ "super2xsai", "supersai", 2 },
	{ "supersai2x", "supersai", 2 },
	{ "supereagle", "supereagle", 2 },
	{ "supereagle2x", "supereagle", 2 },
	{ "pm2x", "pm", 2 },
	{ "dotmatrix", "dotmatrix", 2 },
	{ "dotmatrix2x", "dotmatrix", 2 },
	{ "tv2x", "tv", 2 }
};

bool ScalerManager::isOldGraphicsSetting(const Common::String &gfxMode) {
	for (uint i = 0; i < ARRAYSIZE(s_legacyGraphicsModes); ++i) {
		if (gfxMode == s_legacyGraphicsModes[i].oldName) {
			return true;
		}
	}
	return false;
}

void ScalerManager::updateOldSettings() {
	// Search for legacy gfx_mode and replace it
	if (ConfMan.hasKey("gfx_mode")) {
		Common::String gfxMode(ConfMan.get("gfx_mode"));
		for (uint i = 0; i < ARRAYSIZE(s_legacyGraphicsModes); ++i) {
			if (gfxMode == s_legacyGraphicsModes[i].oldName) {
				ConfMan.set("scaler", s_legacyGraphicsModes[i].newName);
				ConfMan.setInt("scale_factor", s_legacyGraphicsModes[i].factor);
				break;
			}
		}
	}

	// Look in all game domains as well
	for (Common::ConfigManager::DomainMap::iterator domain = ConfMan.beginGameDomains(); domain != ConfMan.endGameDomains(); ++domain) {
		if (domain->_value.contains("gfx_mode")) {
			Common::String gfxMode(domain->_value.getVal("gfx_mode"));
			for (uint i = 0; i < ARRAYSIZE(s_legacyGraphicsModes); ++i) {
				if (gfxMode == s_legacyGraphicsModes[i].oldName) {
					warning("%s: %s -> %s@%dx", domain->_value.getDomainComment().c_str(), s_legacyGraphicsModes[i].oldName, s_legacyGraphicsModes[i].newName, s_legacyGraphicsModes[i].factor);
					domain->_value.setVal("scaler", s_legacyGraphicsModes[i].newName);
					domain->_value.setVal("scale_factor", Common::String::format("%i", s_legacyGraphicsModes[i].factor));
					domain->_value.erase("gfx_mode");
					break;
				}
			}
		}
	}
}
