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

#include "base/plugins.h"

#include "common/func.h"
#include "common/debug.h"
#include "common/config-manager.h"

#ifdef DYNAMIC_MODULES
#include "common/fs.h"
#endif

#include "base/detection/detection.h"

// Plugin versioning

int pluginTypeVersions[PLUGIN_TYPE_MAX] = {
	PLUGIN_TYPE_ENGINE_DETECTION_VERSION,
	PLUGIN_TYPE_ENGINE_VERSION,
	PLUGIN_TYPE_MUSIC_VERSION,
	PLUGIN_TYPE_DETECTION_VERSION,
};


// Abstract plugins

PluginType Plugin::getType() const {
	return _type;
}

const char *Plugin::getName() const {
	return _pluginObject->getName();
}

const char *Plugin::getEngineId() const {
	if (_type == PLUGIN_TYPE_ENGINE_DETECTION) {
		return _pluginObject->getEngineId();
	}

	return nullptr;
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

	virtual PluginList getPlugins() {
		PluginList pl;

		#define LINK_PLUGIN(ID) \
			extern PluginType g_##ID##_type; \
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
		#if defined(WIN32) && !defined(__SYMBIAN32__)
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
		#if defined(__amigaos4__)
		LINK_PLUGIN(CAMD)
		#endif
		#if defined(MACOSX)
		LINK_PLUGIN(COREAUDIO)
		#endif
		#ifdef USE_FLUIDSYNTH
		LINK_PLUGIN(FLUIDSYNTH)
		#endif
		#ifdef USE_MT32EMU
		LINK_PLUGIN(MT32)
		#endif
		#if defined(__ANDROID__)
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
		LINK_PLUGIN(TOWNS)
		LINK_PLUGIN(PC98)
		LINK_PLUGIN(SEGACD)
		#if defined(USE_TIMIDITY)
		LINK_PLUGIN(TIMIDITY)
		#endif
		#if defined(MACOSX)
		// Keep this at the end of the list - it takes a long time to enumerate
		// and is only for hardware midi devices
		LINK_PLUGIN(COREMIDI)
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
	pluginDirs.push_back(Common::FSNode("plugins"));

	// Add the provider's custom directories
	addCustomDirectories(pluginDirs);

	// Add the user specified directory
	Common::String pluginsPath(ConfMan.get("pluginspath"));
	if (!pluginsPath.empty())
		pluginDirs.push_back(Common::FSNode(pluginsPath));

	Common::FSList::const_iterator dir;
	for (dir = pluginDirs.begin(); dir != pluginDirs.end(); ++dir) {
		// Load all plugins.
		// Scan for all plugins in this directory
		Common::FSList files;
		if (!dir->getChildren(files, Common::FSNode::kListFilesOnly)) {
			debug(1, "Couldn't open plugin directory '%s'", dir->getPath().c_str());
			continue;
		} else {
			debug(1, "Reading plugins from plugin directory '%s'", dir->getPath().c_str());
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

PluginManager *PluginManager::_instance = NULL;

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

Plugin *PluginManager::getEngineFromMetaEngine(const Plugin *plugin) {
	assert(plugin->getType() == PLUGIN_TYPE_ENGINE_DETECTION);

	Plugin *enginePlugin = nullptr;
	bool found = false;

	// Use the engineID from MetaEngine for comparasion.
	Common::String metaEnginePluginName = plugin->getEngineId();
	PluginMan.loadFirstPlugin();
	do {
		PluginList pl = PluginMan.getPlugins(PLUGIN_TYPE_ENGINE);
		// Iterate over all engine plugins.
		for (PluginList::const_iterator itr = pl.begin(); itr != pl.end(); itr++) {
			// The getName() provides a name which is similiar to getEngineId.
			// Because engines are engines themselves, this function is simply named getName.
			Common::String enginePluginName((*itr)->getName());

			if (metaEnginePluginName.equalsIgnoreCase(enginePluginName)) {
				enginePlugin = (*itr);
				found = true;
				break;
			}
		}
	} while (!found && PluginMan.loadNextPlugin());

	if (enginePlugin) {
		debug(9, "MetaEngine: %s \t matched to \t Engine: %s", plugin->getName(), enginePlugin->getFileName());
		return enginePlugin;
	}

	debug(9, "MetaEngine: %s couldn't find a match for an engine plugin.", plugin->getName());
	return nullptr;
}

Plugin *PluginManager::getMetaEngineFromEngine(const Plugin *plugin) {
	assert(plugin->getType() == PLUGIN_TYPE_ENGINE);

	Plugin *metaEngine = nullptr;

	PluginList pl = PluginMan.getPlugins(PLUGIN_TYPE_ENGINE_DETECTION);

	// This will return a name of the Engine plugin, which will be identical to
	// a getEngineID from a relevant MetaEngine.
	Common::String enginePluginName(plugin->getName());

	for (PluginList::const_iterator itr = pl.begin(); itr != pl.end(); itr++) {
		Common::String metaEngineName = (*itr)->getEngineId();

		if (metaEngineName.equalsIgnoreCase(enginePluginName)) {
			metaEngine = (*itr);
			break;
		}
	}

	if (metaEngine) {
		debug(9, "Engine: %s matched to MetaEngine: %s", plugin->getFileName(), metaEngine->getName());
		return metaEngine;
	}

	debug(9, "Engine: %s couldn't find a match for an MetaEngine plugin.", plugin->getFileName());
	return nullptr;
}

/**
 * This should only be called once by main()
 **/
void PluginManagerUncached::init() {
	unloadAllPlugins();
	_allEnginePlugins.clear();
	ConfMan.setBool("always_run_fallback_detection_extern", false);

	unloadPluginsExcept(PLUGIN_TYPE_ENGINE, NULL, false); // empty the engine plugins

	Common::String detectPluginName = "detection";
#ifdef PLUGIN_SUFFIX
	detectPluginName += PLUGIN_SUFFIX;
#endif

	bool foundDetectPlugin = false;

	for (ProviderList::iterator pp = _providers.begin();
	                            pp != _providers.end();
	                            ++pp) {
		PluginList pl((*pp)->getPlugins());

		for (PluginList::iterator p = pl.begin(); p != pl.end(); ++p) {
			// This is a 'hack' based on the assumption that we have no sound
			// file plugins. Currently this is the case. If it changes, we
			// should find a fast way of detecting whether a plugin is a
			// music or an engine plugin.
			if (!foundDetectPlugin && (*pp)->isFilePluginProvider()) {
				Common::String pName = (*p)->getFileName();
				if (pName.hasSuffix(detectPluginName)) {
					_detectionPlugin = (*p);
					foundDetectPlugin = true;
					debug(9, "Detection plugin found!");
					continue;
				}
			}

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
			Common::String filename = (*domain)[engineId];

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
bool PluginManagerUncached::loadPluginByFileName(const Common::String &filename) {
	if (filename.empty())
		return false;

	unloadPluginsExcept(PLUGIN_TYPE_ENGINE, NULL, false);

	PluginList::iterator i;
	for (i = _allEnginePlugins.begin(); i != _allEnginePlugins.end(); ++i) {
		if (Common::String((*i)->getFileName()) == filename && (*i)->loadPlugin()) {
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
	if ((*_currentPlugin)->getFileName()) {
		if (!ConfMan.hasMiscDomain("engine_plugin_files"))
			ConfMan.addMiscDomain("engine_plugin_files");

		Common::ConfigManager::Domain *domain = ConfMan.getDomain("engine_plugin_files");
		assert(domain);
		(*domain)[engineId] = (*_currentPlugin)->getFileName();

		ConfMan.flushToDisk();
	}
}

void PluginManagerUncached::loadDetectionPlugin() {
	bool linkMetaEngines = false;

	if (_isDetectionLoaded) {
		debug(9, "Detection plugin is already loaded. Adding each available engines to the memory.");
		linkMetaEngines = true;
	} else {
		if (_detectionPlugin) {
			if (_detectionPlugin->loadPlugin()) {
				assert((_detectionPlugin)->getType() == PLUGIN_TYPE_DETECTION);

				linkMetaEngines = true;
				_isDetectionLoaded = true;
			} else {
				debug(9, "Detection plugin was not loaded correctly.");
				return;
			}
		} else {
			debug(9, "Detection plugin not found.");
			return;
		}
	}

	if (linkMetaEngines) {
		_pluginsInMem[PLUGIN_TYPE_ENGINE_DETECTION].clear();
		const Detection &detectionConnect = _detectionPlugin->get<Detection>();
		const PluginList &pl = detectionConnect.getPlugins();
		Common::for_each(pl.begin(), pl.end(), Common::bind1st(Common::mem_fun(&PluginManagerUncached::tryLoadPlugin), this));
	}

}

void PluginManagerUncached::unloadDetectionPlugin() {
	if (_isDetectionLoaded) {
		_pluginsInMem[PLUGIN_TYPE_ENGINE_DETECTION].clear();
		_detectionPlugin->unloadPlugin();
		_isDetectionLoaded = false;
	} else {
		debug(9, "Detection plugin is already unloaded.");
	}
}

void PluginManagerUncached::loadFirstPlugin() {
	unloadPluginsExcept(PLUGIN_TYPE_ENGINE, NULL, false);

	// let's try to find one we can load
	for (_currentPlugin = _allEnginePlugins.begin(); _currentPlugin != _allEnginePlugins.end(); ++_currentPlugin) {
		if ((*_currentPlugin)->loadPlugin()) {
			addToPluginsInMemList(*_currentPlugin);
			break;
		}
	}
}

bool PluginManagerUncached::loadNextPlugin() {
	unloadPluginsExcept(PLUGIN_TYPE_ENGINE, NULL, false);

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
		unloadPluginsExcept((PluginType)i, NULL);
}

void PluginManager::unloadPluginsExcept(PluginType type, const Plugin *plugin, bool deletePlugin /*=true*/) {
	Plugin *found = NULL;
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
	if (found != NULL) {
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

	PluginList::iterator pl = _pluginsInMem[plugin->getType()].begin();
	while (!found && pl != _pluginsInMem[plugin->getType()].end()) {
		if (!strcmp(plugin->getName(), (*pl)->getName())) {
			// Found a duplicated module. Replace the old one.
			found = true;
			delete *pl;
			*pl = plugin;
			debug(1, "Replaced the duplicated plugin: '%s'", plugin->getName());
		}
		pl++;
	}

	if (!found) {
		// If it provides a new module, just add it to the list of known plugins in memory.
		_pluginsInMem[plugin->getType()].push_back(plugin);
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
		const Plugin *p = EngineMan.findPlugin(engineId);
		if (p) {
			const MetaEngineDetection &engine = p->get<MetaEngineDetection>();

			PlainGameDescriptor pluginResult = engine.findGame(gameId.c_str());
			if (pluginResult.gameId) {
				results.push_back(QualifiedGameDescriptor(engine.getEngineId(), pluginResult));
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
	const PluginList &plugins = getPlugins();

	QualifiedGameList results;
	PluginList::const_iterator iter;

	for (iter = plugins.begin(); iter != plugins.end(); ++iter) {
		const MetaEngineDetection &engine = (*iter)->get<MetaEngineDetection>();
		PlainGameDescriptor pluginResult = engine.findGame(gameId.c_str());

		if (pluginResult.gameId) {
			results.push_back(QualifiedGameDescriptor(engine.getEngineId(), pluginResult));
		}
	}

	return results;
}

DetectionResults EngineManager::detectGames(const Common::FSList &fslist) const {
	DetectedGames candidates;
	PluginList plugins;
	PluginList::const_iterator iter;

	// MetaEngines are always loaded into memory, so, get them and
	// run detection for all of them.
	plugins = getPlugins(PLUGIN_TYPE_ENGINE_DETECTION);

	// Iterate over all known games and for each check if it might be
	// the game in the presented directory.
	for (iter = plugins.begin(); iter != plugins.end(); ++iter) {
		const MetaEngineDetection &metaEngine = (*iter)->get<MetaEngineDetection>();
		DetectedGames engineCandidates = metaEngine.detectGames(fslist);

		for (uint i = 0; i < engineCandidates.size(); i++) {
			engineCandidates[i].path = fslist.begin()->getParent().getPath();
			engineCandidates[i].shortPath = fslist.begin()->getParent().getDisplayName();
			candidates.push_back(engineCandidates[i]);
		}
	}

	return DetectionResults(candidates);
}

const PluginList &EngineManager::getPlugins(const PluginType fetchPluginType) const {
	return PluginManager::instance().getPlugins(fetchPluginType);
}

namespace {

void addStringToConf(const Common::String &key, const Common::String &value, const Common::String &domain) {
	if (!value.empty())
		ConfMan.set(key, value, domain);
}

} // End of anonymous namespace

Common::String EngineManager::createTargetForGame(const DetectedGame &game) {
	// The auto detector or the user made a choice.
	// Pick a domain name which does not yet exist (after all, we
	// are *adding* a game to the config, not replacing).
	Common::String domain = game.preferredTarget;

	assert(!domain.empty());
	if (ConfMan.hasGameDomain(domain)) {
		int suffixN = 1;
		Common::String gameid(domain);

		while (ConfMan.hasGameDomain(domain)) {
			domain = gameid + Common::String::format("-%d", suffixN);
			suffixN++;
		}
	}

	// Add the name domain
	ConfMan.addGameDomain(domain);

	// Copy all non-empty relevant values into the new domain
	addStringToConf("engineid", game.engineId, domain);
	addStringToConf("gameid", game.gameId, domain);
	addStringToConf("description", game.description, domain);
	addStringToConf("language", Common::getLanguageCode(game.language), domain);
	addStringToConf("platform", Common::getPlatformCode(game.platform), domain);
	addStringToConf("path", game.path, domain);
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

const Plugin *EngineManager::findLoadedPlugin(const Common::String &engineId) const {
	const PluginList &plugins = getPlugins();

	for (PluginList::const_iterator iter = plugins.begin(); iter != plugins.end(); iter++)
		if (engineId == (*iter)->get<MetaEngineDetection>().getEngineId())
			return *iter;

	return 0;
}

const Plugin *EngineManager::findPlugin(const Common::String &engineId) const {
	// First look for the game using the plugins in memory. This is critical
	// for calls coming from inside games
	const Plugin *plugin = findLoadedPlugin(engineId);
	if (plugin)
		return plugin;

	// Now look for the plugin using the engine ID. This is much faster than scanning plugin
	// by plugin
	if (PluginMan.loadPluginFromEngineId(engineId))  {
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

	return 0;
}

QualifiedGameDescriptor EngineManager::findTarget(const Common::String &target, const Plugin **plugin) const {
	// Ignore empty targets
	if (target.empty())
		return QualifiedGameDescriptor();

	// Lookup the domain. If we have no domain, fallback on the old function [ultra-deprecated].
	const Common::ConfigManager::Domain *domain = ConfMan.getDomain(target);
	if (!domain || !domain->contains("gameid") || !domain->contains("engineid"))
		return QualifiedGameDescriptor();

	// Look for the engine ID
	const Plugin *foundPlugin = findPlugin(domain->getVal("engineid"));
	if (!foundPlugin) {
		return QualifiedGameDescriptor();
	}

	// Make sure it does support the game ID
	const MetaEngineDetection &engine = foundPlugin->get<MetaEngineDetection>();
	PlainGameDescriptor desc = engine.findGame(domain->getVal("gameid").c_str());
	if (!desc.gameId) {
		return QualifiedGameDescriptor();
	}

	if (plugin)
		*plugin = foundPlugin;

	return QualifiedGameDescriptor(engine.getEngineId(), desc);
}

void EngineManager::upgradeTargetIfNecessary(const Common::String &target) const {
	Common::ConfigManager::Domain *domain = ConfMan.getDomain(target);
	assert(domain);

	if (!domain->contains("engineid")) {
		upgradeTargetForEngineId(target);
	} else {
		if (domain->getVal("engineid").equals("fullpipe")) {
			domain->setVal("engineid", "ngi");

			debug("Upgrading engineid from 'fullpipe' to 'ngi'");

			ConfMan.flushToDisk();
		}
	}
}

void EngineManager::upgradeTargetForEngineId(const Common::String &target) const {
	Common::ConfigManager::Domain *domain = ConfMan.getDomain(target);
	assert(domain);

	debug("Target '%s' lacks an engine ID, upgrading...", target.c_str());

	Common::String oldGameId = domain->getVal("gameid");
	Common::String path = domain->getVal("path");

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
	const Plugin *plugin = findPlugin(oldGameId);
	if (plugin) {
		// Run detection on the game path
		Common::FSNode dir(path);
		Common::FSList files;
		if (!dir.getChildren(files, Common::FSNode::kListAll)) {
			warning("Failed to access path '%s' when upgrading target '%s'", path.c_str(), target.c_str());
			return;
		}

		// Take the first detection entry
		const MetaEngineDetection &metaEngine = plugin->get<MetaEngineDetection>();
		DetectedGames candidates = metaEngine.detectGames(files);
		if (candidates.empty()) {
			warning("No games supported by the engine '%s' were found in path '%s' when upgrading target '%s'",
			        metaEngine.getEngineId(), path.c_str(), target.c_str());
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
