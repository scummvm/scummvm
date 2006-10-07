/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#include "common/stdafx.h"
#include "base/plugins.h"
#include "common/util.h"
#include "common/fs.h"

#ifdef DYNAMIC_MODULES
  #if defined(UNIX)
  #include "backends/plugins/posix/posix-provider.h"
  #elif defined(__DC__)
  #include "backends/plugins/dc/dc-provider.h"
  #elif defined(_WIN32)
  #include "backends/plugins/win32/win32-provider.h"
  #endif
#endif


void DetectedGame::updateDesc(const char *extra) {
	// TODO: The format used here (LANG/PLATFORM/EXTRA) is not set in stone.
	// We may want to change the order (PLATFORM/EXTRA/LANG, anybody?), or
	// the seperator (instead of '/' use ', ' or ' ').
	const bool hasCustomLanguage = (language != Common::UNK_LANG);
	const bool hasCustomPlatform = (platform != Common::kPlatformUnknown);
	const bool hasExtraDesc = (extra && extra[0]);

	// Adapt the description string if custom platform/language is set.
	if (hasCustomLanguage || hasCustomPlatform || hasExtraDesc) {
		description += " (";
		if (hasCustomLanguage)
			description += Common::getLanguageDescription(language);
		if (hasCustomPlatform) {
			if (hasCustomLanguage)
				description += "/";
			description += Common::getPlatformDescription(platform);
		}
		if (hasExtraDesc) {
			if (hasCustomPlatform || hasCustomLanguage)
				description += "/";
			description += extra;
		}
		description += ")";
	}
}


#pragma mark -

#ifndef DYNAMIC_MODULES
class StaticPlugin : public Plugin {
	PluginRegistrator *_plugin;
public:
	StaticPlugin(PluginRegistrator *plugin)
		: _plugin(plugin) {
		assert(_plugin);
	}
	
	~StaticPlugin() {
		delete _plugin;
	}

	virtual bool loadPlugin()		{ return true; }
	virtual void unloadPlugin()		{}

	const char *getName() const { return _plugin->_name; }
	const char *getCopyright() const { return _plugin->_copyright; }

	PluginError createInstance(OSystem *syst, Engine **engine) const {
		assert(_plugin->_ef);
		return (*_plugin->_ef)(syst, engine);
	}

	GameList getSupportedGames() const { return _plugin->_games; }

	GameDescriptor findGame(const char *gameid) const {
		assert(_plugin->_qf);
		return (*_plugin->_qf)(gameid);
	}

	DetectedGameList detectGames(const FSList &fslist) const {
		assert(_plugin->_df);
		return (*_plugin->_df)(fslist);
	}
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
			extern PluginRegistrator *g_##ID##_PluginReg; \
			extern void g_##ID##_PluginReg_alloc(); \
			g_##ID##_PluginReg_alloc(); \
			plugin = g_##ID##_PluginReg; \
			pl.push_back(new StaticPlugin(plugin));
	
		// "Loader" for the static plugins.
		// Iterate over all registered (static) plugins and load them.
		PluginRegistrator *plugin;
	
		#ifndef DISABLE_SCUMM
		LINK_PLUGIN(SCUMM)
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
		#ifndef DISABLE_AGOS
		LINK_PLUGIN(AGOS)
		#endif
		#ifndef DISABLE_QUEEN
		LINK_PLUGIN(QUEEN)
		#endif
		#ifndef DISABLE_SAGA
		LINK_PLUGIN(SAGA)
		#endif
		#ifndef DISABLE_KYRA
		LINK_PLUGIN(KYRA)
		#endif
		#ifndef DISABLE_GOB
		LINK_PLUGIN(GOB)
		#endif
		#ifndef DISABLE_LURE
		LINK_PLUGIN(LURE)
		#endif
		#ifndef DISABLE_CINE
		LINK_PLUGIN(CINE)
		#endif
		#ifndef DISABLE_AGI
		LINK_PLUGIN(AGI)
		#endif
	
	
		return pl;
	}
};


#endif

#pragma mark -

DECLARE_SINGLETON(PluginManager);

PluginManager::PluginManager() {

// FIXME: The following code should be moved to the backend specific code,
// usually into the main() function just before scummvm_main is called
#ifdef DYNAMIC_MODULES

#if defined(UNIX)
	addPluginProvider(new POSIXPluginProvider());
#elif defined(__DC__)
	addPluginProvider(new DCPluginProvider());
#elif defined(_WIN32)
	addPluginProvider(new Win32PluginProvider());
#else
#error No support for loading plugins on non-unix systems at this point!
#endif

#else
	addPluginProvider(new StaticPluginProvider());
#endif
}

PluginManager::~PluginManager() {
	// Explicitly unload all loaded plugins
	unloadPlugins();
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
	uint i;
	for (i = 0; i < _plugins.size(); i++) {
		if (_plugins[i] == plugin) {
			found = _plugins[i];
		} else {
			_plugins[i]->unloadPlugin();
			delete _plugins[i];
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

DetectedGameList PluginManager::detectGames(const FSList &fslist) const {
	DetectedGameList candidates;

	// Iterate over all known games and for each check if it might be
	// the game in the presented directory.
	PluginList::const_iterator iter;
	for (iter = _plugins.begin(); iter != _plugins.end(); ++iter) {
		candidates.push_back((*iter)->detectGames(fslist));
	}

	return candidates;
}
