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

#ifdef DYNAMIC_MODULES
#include "common/fs.h"
#endif

#include "engines/detection.h"

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

	virtual bool isDynamic() const override { return false; }
	virtual bool isLoaded() const override { return true; }
	virtual bool loadPlugin() override { return true; }
	virtual void unloadPlugin() override {}
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

	PluginList newPlugins=pp->getPlugins();
	PluginList::const_iterator i;
	for (i = newPlugins.begin(); i != newPlugins.end(); ++i) {
		Plugin *p = *i;
		if (!p->isLoaded())
			continue;
		_pluginsInMem[p->getType()].push_back(p);
	}
}

Plugin *PluginManager::getPluginByFileName(const Common::String &filename) const {
	if (filename.empty())
		return nullptr;

	PluginList allPlugins = getAllPlugins();

	PluginList::iterator i;
	for (i = allPlugins.begin(); i != allPlugins.end(); ++i) {
		if (Common::String((*i)->getFileName()) == filename)
			return *i;
	}
	return nullptr;
}

PluginList PluginManager::getAllPlugins() const {
	PluginList list;

	ProviderList::const_iterator i;
	for (i = _providers.begin(); i != _providers.end(); ++i) {
		list.push_back((*i)->getPlugins());
	}
	return list;
}

PluginList PluginManager::getAllPluginsOfType(PluginType type) const {
	PluginList allPlugins = getAllPlugins();
	PluginList filteredList;

	PluginList::const_iterator i;
	for (i = allPlugins.begin(); i != allPlugins.end(); ++i) {
		Plugin *plugin = *i;
		if (plugin->getType() != type)
			continue;
		filteredList.push_back(plugin);
	}
	return filteredList;
}

bool PluginManager::loadPlugin(Plugin *plugin) {
	assert(plugin);
	assert(!plugin->isLoaded());
	// Try to load the plugin
	if (plugin->loadPlugin()) {
		_pluginsInMem[plugin->getType()].push_back(plugin);
		return true;
	} else {
		// Failed to load the plugin
		return false;
	}
}

void PluginManager::unloadAllPlugins() {
	for (int i = 0; i < PLUGIN_TYPE_MAX; i++)
		unloadAllPluginsOfType((PluginType)i);
}

void PluginManager::unloadPlugin(Plugin *plugin) {
	assert(plugin);
	assert(plugin->isLoaded());
	if (!plugin->isDynamic())
		return;

	PluginList &list = _pluginsInMem[plugin->getType()];
	for (PluginList::iterator p = list.begin(); p != list.end(); ++p) {
		if (*p == plugin) {
			(*p)->unloadPlugin();
			p=list.erase(p);
		}
	}
}

void PluginManager::unloadAllPluginsOfType(PluginType type) {
	PluginList &list = _pluginsInMem[type];
	for (PluginList::iterator i = list.begin(); i != list.end(); ++i) {
		Plugin *p = *i;
		if (!p->isDynamic())
			continue;
		if (!p->isLoaded())
			continue;
		p->unloadPlugin();
		i = list.erase(i);
	}
}

void PluginManager::unloadAllPluginsOfTypeExcept(PluginType type, const Plugin *keep) {
	assert(keep);
	PluginList &list = _pluginsInMem[type];
	for (PluginList::iterator i = list.begin(); i != list.end(); ++i) {
		Plugin *p = *i;
		if (!p->isDynamic())
			continue;
		if (!p->isLoaded())
			continue;
		if (p == keep)
			continue;
		
		p->unloadPlugin();
		i = list.erase(i);
	}
}

// Music plugins

#include "audio/musicplugin.h"

namespace Common {
DECLARE_SINGLETON(MusicManager);
}

const PluginList &MusicManager::getMusicPlugins() const {
	return PluginManager::instance().getLoadedPluginsOfType(PLUGIN_TYPE_MUSIC);
}
