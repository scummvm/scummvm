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
#include "common/util.h"

int pluginTypeVersions[PLUGIN_TYPE_MAX] = {
	PLUGIN_TYPE_ENGINE_VERSION,
};

PluginType Plugin::getType() const {
	return _type;
}

const char *Plugin::getName() const {
	return _pluginObject->getName();
}

const char *Plugin::getCopyright() const {
	return ((MetaEngine*)_pluginObject)->getCopyright();
}

PluginError Plugin::createInstance(OSystem *syst, Engine **engine) const {
	return ((MetaEngine*)_pluginObject)->createInstance(syst, engine);
}

GameList Plugin::getSupportedGames() const {
	return ((MetaEngine*)_pluginObject)->getSupportedGames();
}

GameDescriptor Plugin::findGame(const char *gameid) const {
	return ((MetaEngine*)_pluginObject)->findGame(gameid);
}

GameList Plugin::detectGames(const FSList &fslist) const {
	return ((MetaEngine*)_pluginObject)->detectGames(fslist);
}

SaveStateList Plugin::listSaves(const char *target) const {
	return ((MetaEngine*)_pluginObject)->listSaves(target);
}


#ifndef DYNAMIC_MODULES
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

		#ifndef DISABLE_SCUMM
		LINK_PLUGIN(SCUMM)
		#endif
		#ifndef DISABLE_AGI
		LINK_PLUGIN(AGI)
		#endif
		#ifndef DISABLE_AGOS
		LINK_PLUGIN(AGOS)
		#endif
		#ifndef DISABLE_CINE
		LINK_PLUGIN(CINE)
		#endif
		#ifndef DISABLE_CRUISE
		LINK_PLUGIN(CRUISE)
		#endif
		#ifndef DISABLE_DRASCULA
		LINK_PLUGIN(DRASCULA)
		#endif
		#ifndef DISABLE_GOB
		LINK_PLUGIN(GOB)
		#endif
		#ifndef DISABLE_IGOR
		LINK_PLUGIN(IGOR)
		#endif
		#ifndef DISABLE_KYRA
		LINK_PLUGIN(KYRA)
		#endif
		#ifndef DISABLE_LURE
		LINK_PLUGIN(LURE)
		#endif
		#ifndef DISABLE_M4
		LINK_PLUGIN(M4)
		#endif
		#ifndef DISABLE_MADE
		LINK_PLUGIN(MADE)
		#endif		
		#ifndef DISABLE_PARALLACTION
		LINK_PLUGIN(PARALLACTION)
		#endif
		#ifndef DISABLE_QUEEN
		LINK_PLUGIN(QUEEN)
		#endif
		#ifndef DISABLE_SAGA
		LINK_PLUGIN(SAGA)
		#endif
		#ifndef DISABLE_SKY
		LINK_PLUGIN(SKY)
		#endif
		#ifndef DISABLE_SWORD1
		LINK_PLUGIN(SWORD1)
		#endif
		#ifndef DISABLE_SWORD2
		LINK_PLUGIN(SWORD2)
		#endif
		#ifndef DISABLE_TOUCHE
		LINK_PLUGIN(TOUCHE)
		#endif

		return pl;
	}
};

#else

PluginList FilePluginProvider::getPlugins() {
	PluginList pl;

	// Prepare the list of directories to search
	Common::StringList pluginDirs;
	// TODO: Add the user specified directory (via config file)
	pluginDirs.push_back(".");
	pluginDirs.push_back("plugins");

	// Add the provider's custom directories
	addCustomDirectories(pluginDirs);

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

#endif

#pragma mark -

DECLARE_SINGLETON(PluginManager);

PluginManager::PluginManager() {
#ifndef DYNAMIC_MODULES
	// Add the static plugin provider if we do not build with dynamic
	// plugins.
	addPluginProvider(new StaticPluginProvider());
#endif
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

GameList PluginManager::detectGames(const FSList &fslist) const {
	GameList candidates;

	// Iterate over all known games and for each check if it might be
	// the game in the presented directory.
	PluginList::const_iterator iter;
	for (iter = _plugins.begin(); iter != _plugins.end(); ++iter) {
		candidates.push_back((*iter)->detectGames(fslist));
	}

	return candidates;
}
