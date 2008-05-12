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
 * $URL$
 * $Id$
 *
 */

#include "base/plugins.h"

#ifdef DYNAMIC_MODULES
#include "common/config-manager.h"
#include "common/fs.h"
#endif

// Plugin versioning

int pluginTypeVersions[PLUGIN_TYPE_MAX] = {
	PLUGIN_TYPE_ENGINE_VERSION,
};


// Abstract plugins

PluginType Plugin::getType() const {
	return _type;
}

const char *Plugin::getName() const {
	return _pluginObject->getName();
}

class StaticPlugin : public Plugin {
public:
	StaticPlugin(PluginObject *pluginobject, PluginType type) {
		assert(pluginobject);
		assert(type < PLUGIN_TYPE_MAX);
		_pluginObject = pluginobject;
		_type = type;
	}

	~StaticPlugin() {
		delete _pluginObject;
	}

	virtual bool loadPlugin()		{ return true; }
	virtual void unloadPlugin()		{}
};

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

		#if PLUGIN_ENABLED_STATIC(SCUMM)
		LINK_PLUGIN(SCUMM)
		#endif
		#if PLUGIN_ENABLED_STATIC(AGI)
		LINK_PLUGIN(AGI)
		#endif
		#if PLUGIN_ENABLED_STATIC(AGOS)
		LINK_PLUGIN(AGOS)
		#endif
		#if PLUGIN_ENABLED_STATIC(CINE)
		LINK_PLUGIN(CINE)
		#endif
		#if PLUGIN_ENABLED_STATIC(CRUISE)
		LINK_PLUGIN(CRUISE)
		#endif
		#if PLUGIN_ENABLED_STATIC(DRASCULA)
		LINK_PLUGIN(DRASCULA)
		#endif
		#if PLUGIN_ENABLED_STATIC(GOB)
		LINK_PLUGIN(GOB)
		#endif
		#if PLUGIN_ENABLED_STATIC(IGOR)
		LINK_PLUGIN(IGOR)
		#endif
		#if PLUGIN_ENABLED_STATIC(KYRA)
		LINK_PLUGIN(KYRA)
		#endif
		#if PLUGIN_ENABLED_STATIC(LURE)
		LINK_PLUGIN(LURE)
		#endif
		#if PLUGIN_ENABLED_STATIC(M4)
		LINK_PLUGIN(M4)
		#endif
		#if PLUGIN_ENABLED_STATIC(MADE)
		LINK_PLUGIN(MADE)
		#endif		
		#if PLUGIN_ENABLED_STATIC(PARALLACTION)
		LINK_PLUGIN(PARALLACTION)
		#endif
		#if PLUGIN_ENABLED_STATIC(QUEEN)
		LINK_PLUGIN(QUEEN)
		#endif
		#if PLUGIN_ENABLED_STATIC(SAGA)
		LINK_PLUGIN(SAGA)
		#endif
		#if PLUGIN_ENABLED_STATIC(SKY)
		LINK_PLUGIN(SKY)
		#endif
		#if PLUGIN_ENABLED_STATIC(SWORD1)
		LINK_PLUGIN(SWORD1)
		#endif
		#if PLUGIN_ENABLED_STATIC(SWORD2)
		LINK_PLUGIN(SWORD2)
		#endif
		#if PLUGIN_ENABLED_STATIC(TOUCHE)
		LINK_PLUGIN(TOUCHE)
		#endif

		return pl;
	}
};

#ifdef DYNAMIC_MODULES

PluginList FilePluginProvider::getPlugins() {
	PluginList pl;

	// Prepare the list of directories to search
	Common::StringList pluginDirs;

	// Add the default directories
	pluginDirs.push_back(".");
	pluginDirs.push_back("plugins");

	// Add the provider's custom directories
	addCustomDirectories(pluginDirs);

	// Add the user specified directory
	Common::String pluginsPath(ConfMan.get("pluginspath"));
	if (!pluginsPath.empty()) {
		FilesystemNode dir(pluginsPath);
		pluginDirs.push_back(dir.getPath());
	}

	Common::StringList::const_iterator d;
	for (d = pluginDirs.begin(); d != pluginDirs.end(); d++) {
		// Load all plugins.
		// Scan for all plugins in this directory
		FilesystemNode dir(*d);
		FSList files;
		if (!dir.getChildren(files, FilesystemNode::kListFilesOnly)) {
			debug(1, "Couldn't open plugin directory '%s'", d->c_str());
			continue;
		} else {
			debug(1, "Reading plugins from plugin directory '%s'", d->c_str());
		}

		for (FSList::const_iterator i = files.begin(); i != files.end(); ++i) {
			Common::String name(i->getName());
			if (name.hasPrefix(getPrefix()) && name.hasSuffix(getSuffix())) {
				pl.push_back(createPlugin(i->getPath()));
			}
		}
	}

	return pl;
}

const char* FilePluginProvider::getPrefix() const {
#ifdef PLUGIN_PREFIX
	return PLUGIN_PREFIX;
#else
	return "";
#endif
}

const char* FilePluginProvider::getSuffix() const {
#ifdef PLUGIN_SUFFIX
	return PLUGIN_SUFFIX;
#else
	return "";
#endif
}

void FilePluginProvider::addCustomDirectories(Common::StringList &dirs) const {
#ifdef PLUGIN_DIRECTORY
	dirs.push_back(PLUGIN_DIRECTORY);
#endif
}

#endif // DYNAMIC_MODULES

#pragma mark -

DECLARE_SINGLETON(PluginManager);

PluginManager::PluginManager() {
	// Always add the static plugin provider.
	addPluginProvider(new StaticPluginProvider());
}

PluginManager::~PluginManager() {
	// Explicitly unload all loaded plugins
	unloadPlugins();

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

void PluginManager::loadPlugins() {
	for (ProviderList::iterator pp = _providers.begin();
	                            pp != _providers.end();
	                            ++pp) {
		PluginList pl((**pp).getPlugins());
		for (PluginList::iterator plugin = pl.begin(); plugin != pl.end(); ++plugin) {
			tryLoadPlugin(*plugin);
		}
	}

}

void PluginManager::unloadPlugins() {
	unloadPluginsExcept(NULL);
}

void PluginManager::unloadPluginsExcept(const Plugin *plugin) {
	Plugin *found = NULL;
	for (PluginList::iterator p = _plugins.begin(); p != _plugins.end(); ++p) {
		if (*p == plugin) {
			found = *p;
		} else {
			(**p).unloadPlugin();
			delete *p;
		}
	}
	_plugins.clear();
	if (found != NULL) {
		_plugins.push_back(found);
	}
}

bool PluginManager::tryLoadPlugin(Plugin *plugin) {
	assert(plugin);
	// Try to load the plugin
	if (plugin->loadPlugin()) {
		// If successful, add it to the list of known plugins and return.
		_plugins.push_back(plugin);

		// TODO/FIXME: We should perform some additional checks here:
		// * Check for some kind of "API version" (possibly derived from the
		//   SVN tree revision?)
		// * If two plugins provide the same engine, we should only load one.
		//   To detect this situation, we could just compare the plugin name.
		//   To handle it, simply prefer modules loaded earlier to those coming.
		//   Or vice versa... to be determined... :-)

		return true;
	} else {
		// Failed to load the plugin
		delete plugin;
		return false;
	}
}


// Engine plugins

#include "engines/metaengine.h"

const char *EnginePlugin::getCopyright() const {
	return ((MetaEngine*)_pluginObject)->getCopyright();
}

PluginError EnginePlugin::createInstance(OSystem *syst, Engine **engine) const {
	return ((MetaEngine*)_pluginObject)->createInstance(syst, engine);
}

GameList EnginePlugin::getSupportedGames() const {
	return ((MetaEngine*)_pluginObject)->getSupportedGames();
}

GameDescriptor EnginePlugin::findGame(const char *gameid) const {
	return ((MetaEngine*)_pluginObject)->findGame(gameid);
}

GameList EnginePlugin::detectGames(const FSList &fslist) const {
	return ((MetaEngine*)_pluginObject)->detectGames(fslist);
}

SaveStateList EnginePlugin::listSaves(const char *target) const {
	return ((MetaEngine*)_pluginObject)->listSaves(target);
}

DECLARE_SINGLETON(EngineManager);

GameDescriptor EngineManager::findGame(const Common::String &gameName, const EnginePlugin **plugin) const {
	// Find the GameDescriptor for this target
	const EnginePluginList &plugins = getPlugins();
	GameDescriptor result;

	if (plugin)
		*plugin = 0;

	EnginePluginList::const_iterator iter = plugins.begin();
	for (iter = plugins.begin(); iter != plugins.end(); ++iter) {
		result = (*iter)->findGame(gameName.c_str());
		if (!result.gameid().empty()) {
			if (plugin)
				*plugin = *iter;
			break;
		}
	}
	return result;
}

GameList EngineManager::detectGames(const FSList &fslist) const {
	GameList candidates;

	const EnginePluginList &plugins = getPlugins();

	// Iterate over all known games and for each check if it might be
	// the game in the presented directory.
	EnginePluginList::const_iterator iter;
	for (iter = plugins.begin(); iter != plugins.end(); ++iter) {
		candidates.push_back((*iter)->detectGames(fslist));
	}

	return candidates;
}

const EnginePluginList &EngineManager::getPlugins() const {
	return (const EnginePluginList&)PluginManager::instance().getPlugins();
}
