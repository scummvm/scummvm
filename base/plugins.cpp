/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "backends/fs/fs.h"
#include "base/gameDetector.h"
#include "base/plugins.h"
#include "base/engine.h"
#include "common/util.h"

/** Type of factory functions which make new Engine objects. */
typedef Engine *(*EngineFactory)(GameDetector *detector, OSystem *syst);

typedef const char *(*NameFunc)();
typedef GameList (*TargetListFunc)();
typedef GameList (*DetectFunc)(const FSList &fslist);


#ifdef DYNAMIC_MODULES

#ifdef UNIX
#include <dlfcn.h>
#else
#error No support for loading plugins on non-unix systems at this point!
#endif

#endif


#pragma mark -


GameSettings Plugin::findGame(const char *gameName) const {
	// Find the GameSettings for this game
	assert(gameName);
	GameList games = getSupportedGames();
	GameSettings result = {NULL, NULL, MDT_NONE, 0, NULL};
	for (GameList::Iterator g = games.begin(); g != games.end(); ++g) {
		if (!scumm_stricmp(g->gameName, gameName)) {
			result = *g;
			break;
		}
	}
	return result;
}


#pragma mark -


class StaticPlugin : public Plugin {
	const char *_name;
	EngineFactory _ef;
	DetectFunc _df;
	GameList _games;
public:
	StaticPlugin(const char *name, GameList games, EngineFactory ef, DetectFunc df)
		: _name(name), _ef(ef), _df(df), _games(games) {
	}

	const char *getName() const					{ return _name; }

	Engine *createInstance(GameDetector *detector, OSystem *syst) const {
		return (*_ef)(detector, syst);
	}

	GameList getSupportedGames() const { return _games; }
	GameList detectGames(const FSList &fslist) const {
		return (*_df)(fslist);
	}
};


#pragma mark -


#ifdef DYNAMIC_MODULES

class DynamicPlugin : public Plugin {
	void *_dlHandle;
	Common::String _filename;

	Common::String _name;
	EngineFactory _ef;
	DetectFunc _df;
	GameList _games;
	
	void *findSymbol(const char *symbol);

public:
	DynamicPlugin(const char *filename)
		: _dlHandle(0), _filename(filename), _ef(0), _df(0), _games() {}
	
	const char *getName() const					{ return _name.c_str(); }

	Engine *createInstance(GameDetector *detector, OSystem *syst) const {
		assert(_ef);
		return (*_ef)(detector, syst);
	}

	GameList getSupportedGames() const { return _games; }
	GameList detectGames(const FSList &fslist) const {
		assert(_df);
		return (*_df)(fslist);
	}

	bool loadPlugin();
	void unloadPlugin();
};

void *DynamicPlugin::findSymbol(const char *symbol) {
#ifdef UNIX
#ifdef MACOSX
	// Need to prepend underscore on Mac OS X
	char buffer[256];
	buffer[0] = '_';
	strcpy(buffer + 1, symbol);
	symbol = buffer;
#endif
	void *func = dlsym(_dlHandle, symbol);
	if (!func)
		warning("Failed loading symbold '%s' from plugin '%s' (%s)", symbol, _filename.c_str(), dlerror());
	return func;
#else
#error TODO
#endif
}

bool DynamicPlugin::loadPlugin() {
	assert(!_dlHandle);
	_dlHandle = dlopen(_filename.c_str(), RTLD_LAZY);
	
	if (!_dlHandle) {
		warning("Failed loading plugin '%s' (%s)", _filename.c_str(), dlerror());
		return false;
	}
	
	// Query the plugin's name
	NameFunc nameFunc = (NameFunc)findSymbol("PLUGIN_name");
	if (!nameFunc) {
		unloadPlugin();
		return false;
	}
	_name = nameFunc();
	
	// Query the plugin for the targets it supports
	TargetListFunc gameListFunc = (TargetListFunc)findSymbol("PLUGIN_getSupportedGames");
	if (!gameListFunc) {
		unloadPlugin();
		return false;
	}
	_games = gameListFunc();
	
	// Retrieve the factory function
	_ef = (EngineFactory)findSymbol("PLUGIN_createEngine");
	if (!_ef) {
		unloadPlugin();
		return false;
	}
	
	// Retrieve the detector function
	_df = (DetectFunc)findSymbol("PLUGIN_detectGames");
	if (!_df) {
		unloadPlugin();
		return false;
	}

	return true;
}

void DynamicPlugin::unloadPlugin() {
	if (_dlHandle) {
		if (dlclose(_dlHandle) != 0)
			warning("Failed unloading plugin '%s' (%s)", _filename.c_str(), dlerror());
	}
}

#endif	// DYNAMIC_MODULES

#pragma mark -


PluginManager::PluginManager() {
}

PluginManager::~PluginManager() {
	// Explicitly unload all loaded plugins
	unloadPlugins();
}

void PluginManager::loadPlugins() {
#ifdef DYNAMIC_MODULES
	// Load dynamic plugins
	// TODO... this is right now just a nasty hack. 
	// This should search one or multiple directories for all plugins it can
	// find (to this end, we maybe should use a special prefix/suffix; e.g.
	// instead of libscumm.so, use scumm.engine or scumm.plugin etc.).
	//
	// The list of directories to search could be e.g.:
	// User specified (via config file), ".", "./plugins", "$(prefix)/lib".
	//
	// We also need to add code which ensures what we are looking at is
	// a) a ScummVM engine and b) matches the version of the executable.
	// Hence one more symbol should be exported by plugins which returns
	// the "ABI" version the plugin was built for, and we can compare that
	// to the ABI version of the executable.
	#define LOAD_MODULE(name, NAME) \
		tryLoadPlugin(new DynamicPlugin("scumm/lib" name ".so"));
#else
	// "Loader" for the static plugins
	#define LOAD_MODULE(name, NAME) \
		tryLoadPlugin(new StaticPlugin(name, Engine_##NAME##_gameList(), Engine_##NAME##_create, Engine_##NAME##_detectGames));
#endif

	// Load all plugins.
	// Right now the list is hardcoded. On the long run, of course it should
	// automatically be determined.
#ifndef DISABLE_SCUMM
	LOAD_MODULE("scumm", SCUMM);
#endif

#ifndef DISABLE_SIMON
	LOAD_MODULE("simon", SIMON);
#endif

#ifndef DISABLE_SKY
	LOAD_MODULE("sky", SKY);
#endif

#ifndef DISABLE_SWORD2
	LOAD_MODULE("sword2", SWORD2);
#endif

#ifndef DISABLE_QUEEN
	LOAD_MODULE("queen", QUEEN);
#endif
}

void PluginManager::unloadPlugins() {
	int i;
	for (i = 0; i < _plugins.size(); i++) {
		_plugins[i]->unloadPlugin();
		delete _plugins[i];
	}
	_plugins.clear();
}

bool PluginManager::tryLoadPlugin(Plugin *plugin) {
	assert(plugin);
	// Try to load the plugin
	if (plugin->loadPlugin()) {
		// If succesful, add it to the list of known plugins and return.
		_plugins.push_back(plugin);
		return true;
	} else {
		// Failed to load the plugin
		delete plugin;
		return false;
	}
}
